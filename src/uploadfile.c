/*******************************************************************************
	xfer9860 - a Casio fx-9860G (SD) communication utility
	Copyright (C)
	  2007-2014	Andreas Bertheussen <andreasmarcel@gmail.com>
	  2014		Bruno Leon Alata <brleoal@gmail.com>, libusb-1.0 port

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA  02110-1301, USA.
*******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>	/* stat()*/
#include <sys/stat.h>	/**/
#include <math.h>	/* ceil() */
#include <libusb-1.0/libusb.h>

#include "Casio9860.h"
#include "usbio.h"
#include "config.h"

int uploadFile(char* sourceFileName, char* destFileName, int throttleSetting) {
	int i, destFileNameLength, flashCapacity, packetCount;
	char *fData, *sData, *buffer;
	struct stat sourceFileStatus;
	struct libusb_device_handle *usb_handle;

	FILE *sourceFile = fopen(sourceFileName, "rb");
	if (sourceFile == NULL) {
		printf("[E] Unable to open file: %s\n", sourceFileName);
		return 1;
        }
	printf("[I] Found file: %s\n", sourceFileName);

	destFileNameLength = strlen(destFileName);
	if (destFileNameLength > 12) {
		printf(	"[E] The destination filename: %s\n"
			"     is too long. Filesystem only supports 12 characters.\n", destFileName);
		goto exit_closefile;
	}

	stat(sourceFileName, &sourceFileStatus);
	printf("[I] File size:  %i byte(s)\n", (int)sourceFileStatus.st_size);

	printf("[>] Setting up USB connection.. ");
	usb_handle = fx_getDeviceHandle();	/* initiates usb system */
	if (usb_handle == NULL) {
		printf(	"\n[E] A listening device could not be found.\n"
		      	"    Make sure it is receiving; press [ON], [MENU], [sin], [F2]\n");
		goto exit_closefile;
	}
	if (fx_initDevice(usb_handle) < 0) {	/* does calculator-specific setup */
		printf("\n[E] Error initializing device.\n"); goto exit_release;
	}
	printf("Connected!\n");

	printf("[>] Verifying device.. ");
	if (fx_doConnVer(usb_handle) != 0) { printf("Failed.\n"); goto exit_release; }
	else { printf("Done!\n"); }

	printf("[>] Requesting fls0 capacity.. ");
	flashCapacity = fx_getFlashCapacity(usb_handle, "fls0");
	if (flashCapacity < 0) { printf("\n[E] Error requesting capacity information.\n"); goto exit_release; }
	printf("%i byte(s) free.\n", flashCapacity);

	if (flashCapacity < sourceFileStatus.st_size) {
		printf("[E] There is not enough space to store your file.\n");
		goto exit_release;
	}
	
	packetCount = ceil(sourceFileStatus.st_size/MAX_DATA_PAYLOAD) + 1;
	printf("\n[>] Starting transfer of %s to fls0, %i b, %i packets..\n",
	       destFileName, (int)sourceFileStatus.st_size, packetCount);
	fData = (char*)calloc(MAX_DATA_PAYLOAD, sizeof(char));
	sData = (char*)calloc(MAX_DATA_PAYLOAD*2, sizeof(char));
	buffer = (char*)calloc((MAX_DATA_PAYLOAD*2)+18, sizeof(char));	/* work buffer */
	if (fData == NULL || sData == NULL || buffer == NULL) { printf("[E] Error allocating memory."); goto exit_unalloc; }

	fx_sendFlashFileTransmission(usb_handle, buffer, sourceFileStatus.st_size, destFileName, "fls0");
	ReadUSB(usb_handle, buffer, 6);
	if (fx_getPacketType(buffer) != T_POSITIVE) { printf("[E] Unable to start transfer.\n"); goto exit_unalloc; }

	/* main transfer loop */
	printf("[");
	for (i = 0; i < packetCount; i++) {
		int resendCount = 0, readBytes = 0, escapedBytes = 0;
		readBytes = fread(fData, 1, MAX_DATA_PAYLOAD, sourceFile);
		escapedBytes = fx_escapeBytes(fData, sData, readBytes);
	resend_data:
		MSLEEP(throttleSetting);
		if (resendCount > 2) {
			printf("[E] Errors encountered during transmission.\n");
			goto exit_unalloc;
		}
		resendCount++;
		fx_sendData(usb_handle, buffer, ST_FILE_TO_FLASH, packetCount, i+1, sData, escapedBytes);
		if (i % 4 == 0) { printf("#"); fflush(stdout); } /* indicates every 1kB */
		if (ReadUSBPost(usb_handle, buffer, 6) == 0) {
			printf("ERR: Got no response, retrying.\n");
			goto resend_data;
		}
		if (memcmp(buffer, "\x15\x30\x31", 3) == 0) {	/* ugly way */
			printf("ERR: Got retransmission request, retrying.\n");
			goto resend_data;
		}
	}
	printf("]\n[I] File transfer completed.\n\n");
	fx_sendComplete(usb_handle, buffer);
exit_unalloc:
	free(buffer);
	free(sData);
	free(fData);
exit_release:	fx_releaseDeviceHandle(usb_handle);
exit_closefile:	fclose(sourceFile);
	return 0;
}
