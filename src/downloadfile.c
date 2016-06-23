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

#include "Casio9860.h"
#include "usbio.h"
#include "config.h"

int readPacket(struct libusb_device_handle *usb_handle, char *buffer) {
	/* This basically works by receiving until all data has been read, because
	 * the calculator sometimes writes its packets in chunks. This makes downloading
	 * quite slow, as we have to have a minimum read timeout to make sure packets are
	 * read correctly, but at the same time allows short waiting if no data is ready. */
	int pos = 0, read = 0;
	int transferred = 0; /*output location for the number of bytes actually transferred*/
	do {
		read = libusb_bulk_transfer(usb_handle, 0x82, buffer+pos, ((MAX_DATA_PAYLOAD*2)+18)-pos, &transferred, 40);
		/*read = ReadUSB(usb_handle, buffer+pos, ((MAX_DATA_PAYLOAD*2)+18)-pos);*/
      	        pos += transferred;
	} while (read == 0);

	return pos;
}

int downloadFile(char* sourceFileName, char* destFileName, int throttleSetting) {
	struct libusb_device_handle *usb_handle;
	char *buffer, *fData;
	int fileSize = 0;
	/* The following are only used in the main receive loop (see below) */
	int i=0, dataLength=0, unescapedSize=0, totalCount=0, packetCount=0, packetLength=0, dataWritten=0;
	FILE *destFile = fopen(destFileName, "w");
	if (destFile == NULL) { printf("[E] Cannot open file for writing.\n"); return 1; }

	buffer =(char*)calloc((MAX_DATA_PAYLOAD*2)+18, sizeof(char));
	fData =	(char*)calloc((MAX_DATA_PAYLOAD*2)+18, sizeof(char));

	if (fData == NULL || buffer == NULL) { printf("[E] Error allocating memory\n"); goto exit_unalloc; }

	printf("[>] Setting up USB connection.. ");
	usb_handle = fx_getDeviceHandle();	/* initiates usb system */
	if (usb_handle == NULL) {
		printf(	"\n[E] A listening device could not be found.\n"
		      	"    Make sure it is receiving; press [ON], [MENU], [sin], [F2]\n");
		goto exit_unalloc;
	}
	if (fx_initDevice(usb_handle) < 0) {	/* does calculator-specific setup */
		printf("\n[E] Error initializing device.\n"); goto exit_release;
	}
	printf("Connected!\n");

	printf("[>] Verifying device.. ");
	if (fx_doConnVer(usb_handle) != 0) { printf("Failed.\n"); goto exit_release; }
	else { printf("Done!\n"); }

	fx_sendFlashFileTransmissionRequest(usb_handle, buffer, sourceFileName, "fls0");
	ReadUSB(usb_handle, buffer, 6);
	if (fx_getPacketType(buffer) == T_NEGATIVE) { printf("[E] The file could not be found on the device.\n"); goto exit_release;}

	fx_sendChange_Direction(usb_handle, buffer);
	ReadUSB(usb_handle, buffer, 0x40);
	if (fx_getPacketType( buffer) != T_COMMAND) { printf("[E] Did not receive expected transmission.\n"); goto exit_release; }

	/* Read filesize from offset 12, 8 bytes */
	fx_sendPositive(usb_handle, buffer, POSITIVE_NORMAL);

	fileSize = fx_asciiHexToInt(buffer+OFF_FS, 8);
	printf("\n[I] Got file transmission request, filesize is %i bytes.\n", fileSize);
	
	/* main receive loop */
	printf("[");
	while(1) { /* receive loop */
		packetLength = readPacket(usb_handle, buffer);
		if (fx_getPacketType(buffer) != T_DATA) { printf("[E] Did not receive expected data packet.\n"); goto exit_release; }
		/* the checksum has to be controlled */
		dataLength = fx_asciiHexToInt(buffer+OFF_DS, 4);
		dataLength -= 8; /* subtract the space for PN and TP to get length of DD field */

		unescapedSize = fx_unescapeBytes(buffer+OFF_DD, fData, dataLength); /* unescaped data to fData*/

		dataWritten += fwrite(fData, 1, unescapedSize, destFile);
		fx_sendPositive(usb_handle, buffer, POSITIVE_NORMAL);
		totalCount = fx_asciiHexToInt( buffer+OFF_TP, 4);
		packetCount = fx_asciiHexToInt( buffer+OFF_PN, 4);
		if (i % 4 == 0) { printf("#"); fflush(stdout); } /* indicates every 1kB */
		i++;
		if (packetCount == totalCount) { /* if this was last packet */
			break;
		}
		MSLEEP(throttleSetting);
	}

	ReadUSB(usb_handle, buffer, 6);

	printf("]\n[I] File download completed.\n\n"); fflush(stdout);
	fx_sendComplete(usb_handle, buffer);
exit_release:
	fx_releaseDeviceHandle(usb_handle);
exit_unalloc:
	free(fData);
	free(buffer);
	fclose(destFile);


	return 0;
}
