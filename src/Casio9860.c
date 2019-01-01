/*******************************************************************************
	xfer9860 - a Casio fx-9860G (SD) communication utility
	Copyright (C)
	  2007		Manuel Naranjo <naranjo.manuel@gmail.com>
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
#include <ctype.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <string.h>

#include "Casio9860.h"
#include "usbio.h"

struct libusb_device_handle* fx_getDeviceHandle() {
	int ret = 0;
	struct libusb_device_handle *usb_handle;

	struct libusb_context *ctx = NULL;//a libusb session
	int r = libusb_init(&ctx);
	if(r < 0){
	   printf("\nInit Error %d\n",r);
	   return NULL;
	}
	libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, 3);
	libusb_context *context = NULL;
    libusb_device **list = NULL;
    ssize_t count = 0;
	char ask;

    count = libusb_get_device_list(context, &list);
    if (count == 0) {
		printf("\nERR: No USB devices found!\n");
		return NULL;
	}

    for (size_t idx = 0; idx < count; ++idx) {
        libusb_device *device = list[idx];
        struct libusb_device_descriptor desc = {0};

        ret = libusb_get_device_descriptor(device, &desc);
		if (ret != 0) {
			printf("\nERR: libusb_get_device_descriptor(): %i\n", ret);
		}

		if (desc.idVendor == C9860_VENDOR_ID && desc.idProduct == C9860_PRODUCT_ID) {
			printf("Device found, use? (y/n)\n");
			scanf(" %c", &ask);
			if (tolower(ask) == 'y') {
				ret = libusb_open(device, &usb_handle);
			}
		}
    }

    libusb_free_device_list(list, count);
	if (usb_handle == NULL) {
		return NULL;
	}

	ret = libusb_set_configuration(usb_handle, 1);
	if (ret < 0) { /* needed on WIN32 */
		printf("\nERR: libusb_set_configuration(): %i\n", ret);
		libusb_close(usb_handle);
		return NULL;
	}

	ret = libusb_claim_interface(usb_handle, 0);
	if (ret < 0) {
		printf("\nERR: libusb_claim_interface(): %i\n", ret);
		libusb_close(usb_handle);
		return NULL;
	}

	return usb_handle;
}

void fx_releaseDeviceHandle(struct libusb_device_handle* usb_handle) {
	libusb_release_interface(usb_handle, 0);
	return;
}

int fx_initDevice(struct libusb_device_handle *usb_handle) {
	int ret;
	char* buffer;
	buffer = (char*)calloc(0x29, sizeof(char));

	ret = libusb_control_transfer(usb_handle, 0x80, 0x6, 0x100, 0, buffer, 0x12, 200);
	debug(1, buffer, ret);

	if (ret < 0) {
		fprintf(stderr, "\nERR: fx_initDevice(), 1'st control: %i \n", ret);
		goto exit;
	}

	ret = libusb_control_transfer(usb_handle, 0x80, 0x6, 0x200, 0, buffer, 0x29, 250);
	debug(1, buffer, ret);
	if (ret < 0) {
		fprintf(stderr, "\nERR: fx_initDevice(), 2'nd control: %i \n", ret);
		goto exit;
	}

	ret = libusb_control_transfer(usb_handle, 0x41, 0x1, 0x0, 0, buffer, 0x0, 250);
	debug(1, buffer, ret);
	if (ret < 0) {
		fprintf(stderr, "\nERR: fx_initDevice(), 3'rd control: %i \n", ret);
		goto exit;
	}
	ret = 0;
exit:
	free(buffer);
	return ret;
}

int fx_sendComplete(struct libusb_device_handle *usb_handle, char *buffer) {
	memcpy(buffer, "\x01\x30\x30\x30\x37\x30", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendVerify(struct libusb_device_handle *usb_handle, char *buffer, char *type) {
	/* Type: 0x05
	 * ST: 00 or 01*/
	memcpy(buffer, "\x05\x30\x30\x30", 4);
	if (type[1] == '1') {
		memcpy(buffer+2, "\x31", 1);
	}
	fx_appendChecksum(buffer, 4);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_doConnVer(struct libusb_device_handle *usb_handle) {
	char *buffer = (char*)calloc(6, sizeof(char));
	if (buffer == NULL) {printf("ERR: fx_doConnVer(): allocation failed.\n");}
	fx_sendVerify(usb_handle, buffer, "00");	/* sends connver for start of communication */
	ReadUSB(usb_handle, buffer, 6);
	if (fx_getPacketType(buffer) == T_POSITIVE) {/* assuming the positive response is 'plain' */
		free(buffer);
		return 0;
	} else {
		free(buffer);
		return 1;	/* failure */
	}
}

int fx_sendTerminate(struct libusb_device_handle *usb_handle, char *buffer) {
	/* Type: 0x18
	 * ST: 01 */
	memcpy(buffer, "\x18\x30\x31\x30\x36\x46", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendPositive(struct libusb_device_handle *usb_handle, char *buffer, char type) {
	/* Type: 0x06
	 * ST: given as argument */
	memcpy(buffer, "\x06\x30\x30\x30", 4);
	if (type == POSITIVE_OVERWRITE || type == POSITIVE_SYSINFO) {
		memcpy(buffer+2, &type, 1);
	}

	fx_appendChecksum(buffer, 4);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_getPacketType(char *buffer) {
	if (buffer != NULL && buffer[0] != 0x00)
		return buffer[0];
	else
		return -1;
}

int fx_sendNegative(struct libusb_device_handle *usb_handle, char *buffer, char type) {
	/* Type 0x05
	 * ST: given as argument */
	memcpy(buffer, "\x05\x30\x30\x30", 4);
	if (type >= 0x30 || type <= 0x36) {	/* from '0' and '6' */
		memcpy(buffer+2, &type, 1);
	}
	fx_appendChecksum(buffer, 4);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendChange_Direction(struct libusb_device_handle *usb_handle, char *buffer) {
	memcpy(buffer, "\x03\x30\x30\x30\x37\x30", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendFlash_Capacity_Request(struct libusb_device_handle *usb_handle, char *buffer, char *device) {
	short int len = strlen(device);
	memcpy(buffer, "\x01\x34\x42\x31", 4);
	sprintf(buffer+OFF_DS, "%04X", 24+len);
	memcpy(buffer+OFF_OW, "000000000000", 12); /* includes OW, DT and FS */
	sprintf(buffer+OFF_SD1, "00000000%02X00", len);	/* includes D1 - D6 */
	memcpy(buffer+OFF_D1, device, len);

	fx_appendChecksum(buffer, 32+len);

	return WriteUSB(usb_handle, buffer, 34+len);
}

int fx_sendMCSCapacityRequest(struct libusb_device_handle *usb_handle, char *buffer) {
	memcpy(buffer, "\x01\x32\x42\x30", 4);	/* ST is 42 */
	fx_appendChecksum(buffer, 4);

	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendFlashCollectGarbage(struct libusb_device_handle *usb_handle, char *buffer, char *device) {
	short int len = strlen(device);
	memcpy(buffer, "\x01\x35\x31\x31", 4);	/* ST 51 */
	sprintf(buffer+OFF_DS, "%04X", 24+len);
	memcpy(buffer+OFF_OW, "000000000000", 12);
	sprintf(buffer+OFF_SD1, "00000000%02X00", len);
	memcpy(buffer+OFF_D1, device, len);

	fx_appendChecksum(buffer, 32+len);

	return WriteUSB(usb_handle, buffer, 34+len);

}

int fx_getFlashCapacity(struct libusb_device_handle *usb_handle, char *device) {
	int freeSize = 0;
	char * buffer = (char*)calloc(40,sizeof(char));
		if (buffer == NULL) { printf("ERR: fx_getFlashCapacity: alloc error\n"); return -1; }

	fx_sendFlash_Capacity_Request(usb_handle, buffer, device);
	ReadUSB(usb_handle, buffer, 6);
		if (fx_getPacketType(buffer) != T_POSITIVE) { printf("ERR: fx_getFlashCapacity: no proper response\n"); return -1; }

	fx_sendChange_Direction(usb_handle, buffer);
	ReadUSB(usb_handle, buffer, 0x26);
		if (fx_getPacketType(buffer) != T_COMMAND) { printf("ERR: fx_getFlashCapacity: no returned command\n"); return -1; }

	freeSize = fx_asciiHexToInt(buffer+OFF_FS, 8); /* free size is returned in FS */

	fx_sendPositive(usb_handle, buffer, POSITIVE_NORMAL);
	ReadUSB(usb_handle, buffer, 6);

	free(buffer);

	return freeSize; /* converts 'hex-ascii' to int */
}

int fx_getMCSCapacity(struct libusb_device_handle *usb_handle) {
	int freeSize = 0;
	char * buffer = (char*)calloc(40,sizeof(char));
	fx_sendMCSCapacityRequest(usb_handle, buffer);
	ReadUSB(usb_handle, buffer, 6);
		if (fx_getPacketType(buffer) != T_POSITIVE) { printf("ERR: fx_getMCShCapacity: no proper response\n"); return -1; }
	fx_sendChange_Direction(usb_handle, buffer);

	ReadUSB(usb_handle, buffer, 0x26);
		if (fx_getPacketType(buffer) != T_COMMAND) { printf("ERR: fx_getMCSCapacity: no returned command\n"); return -1; }

	freeSize = fx_asciiHexToInt(buffer+OFF_FS, 8);

	fx_sendPositive(usb_handle, buffer, POSITIVE_NORMAL);
	ReadUSB(usb_handle, buffer, 6);

	free(buffer);

	return freeSize;
}

int fx_sendFlashFileTransmission(struct libusb_device_handle *usb_handle, char *buffer, int filesize, char *filename, char *device) {
	short int fnsize = strlen(filename);
	short int devsize = strlen(device);
	memcpy(buffer, "\x01\x34\x35\x31", 4); /* T, ST = 45, DF */
	sprintf(buffer+OFF_DS, "%04X", 24+fnsize+devsize);
	memcpy(buffer+OFF_OW, "0280", 4); /* OW, DT */
	sprintf(buffer+OFF_FS, "%08X", filesize); /* FS   8 byte*/
	sprintf(buffer+OFF_SD1, "00%02X0000%02X00", fnsize, devsize); /* SD1 - SD6, 12b*/
	sprintf(buffer+OFF_D1, "%s%s", filename, device);

	fx_appendChecksum(buffer, 32+fnsize+devsize);

	return WriteUSB(usb_handle, buffer, 34+fnsize+devsize);
}

int fx_sendFlashFileTransmissionRequest(struct libusb_device_handle *usb_handle, char* buffer, char* filename, char* device) {
	short int fnsize = strlen(filename);
	short int devsize = strlen(device);
	memcpy(buffer, "\x01\x34\x34\x31", 4); /* T, ST = 44, DF */
	sprintf(buffer+OFF_DS, "%04X", 24+fnsize+devsize);
	memcpy(buffer+OFF_OW, "0000", 4); /* OW, DT */
	sprintf(buffer+OFF_FS, "00000000"); /* FS   8 byte*/
	sprintf(buffer+OFF_SD1, "00%02X0000%02X00", fnsize, devsize); /* SD2 and SD5 are in use, no folder support yet */
	sprintf(buffer+OFF_D1, "%s%s", filename, device);

	fx_appendChecksum(buffer, 32+fnsize+devsize);

	return WriteUSB(usb_handle, buffer, 34+fnsize+devsize);
}

int fx_sendData(struct libusb_device_handle *usb_handle, char *buffer, char* subtype, int total, int number, char *data, int length) {
	memcpy(buffer, "\x02\x00\x00\x31", 4); /* T, DF, leaves a hole for ST */
	 memcpy(buffer+1, subtype, 2); /* ST */
	sprintf(buffer+OFF_DS, "%04X", 8+length); /* DS, 4 b */
	sprintf(buffer+OFF_TP, "%04X%04X", total, number);
	memcpy(buffer+OFF_DD, data, length);

	fx_appendChecksum(buffer, 16+length);
	return WriteUSB(usb_handle, buffer, 18+length);
}

int fx_appendChecksum(char *buffer, int length) {
	int i;
	char sum = 0;
	for (i = 1; i < length; i++) {
		sum += *(buffer+i);
	}
	sprintf(buffer+length, "%02X", ((~sum)+1) & 0xFF);
	/* The value appears sometimes as FFFFxx, where AND'ing it with 0xFF, you
	 * get the wanted one-byte 0000xx.. */
	return 0;
}

int fx_unescapeBytes(char *source, char*dest, int length) {
	int i = 0, j = 0;
	while (i < length) {
		if (source[i] == 0x5C) { /* if we find an escape character.. look at next byte */
			if (source[i+1] == 0x5C) { /* if it's another escape char, it is taken literally */
				dest[j] = source[i];
				i++;	/* and we skip the following byte */
				goto done;
			} else {
				dest[j] = source[i+1]-0x20; /* if its a usual byte, subtract 0x20 */
				i++;
				goto done;
			}
		}

		/* default copying */
		dest[j] = source[i];

	done:
		i++;
		j++;
	}

	return j;
}

int fx_escapeBytes(char *source, char *dest, int length) {
	int i = 0, j = 0;
	while(i < length) {
		if (source[i] < 0x20) {
			dest[j] = 0x5C;
			dest[j+1] = 0x20+source[i];
			j++;
			goto done;
		}

		if (source[i] == 0x5C) {
			dest[j] = 0x5C; dest[j+1] = 0x5C; j++; /* I might have to use 0x7C here */
			goto done;
		}
		/* default action */
		*(dest+j) = *(source+i);

	done:
		j++;
		i++;
	}
	return j; /* length of destination (new) buffer */
}

long int fx_asciiHexToInt(char *source, int length) {
	int value;
	char *tmp = (char*)calloc(length+1, sizeof(char));
	memcpy(tmp, source, length); tmp[length] = 0; /* using the extra byte for null-termination */
	value = strtol(tmp, NULL, 16);
	free(tmp);
	return value;
}
