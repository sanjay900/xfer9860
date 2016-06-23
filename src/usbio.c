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

#include "usbio.h"
#include "config.h"

#include <stdio.h>
#include <libusb-1.0/libusb.h>

void debug(int input, char* array, int len){
#ifdef SNOOP
	unsigned char temp;
	int i, j, line = 0;

	if (input) { fprintf(stderr, "<< "); }
	else { fprintf(stderr, ">> "); }

	for (i = 0 ; i < len ; i++){
		temp = (unsigned char) array[i];

		if (i % LEN_LINE == 0 && i != 0){
			fprintf(stderr, "\t");
			for (j = line; j < line + LEN_LINE; j++){
				char u = (unsigned char) array[j];
				if (isprint(u)) { fprintf(stderr, "%c", u); }
				else { fprintf(stderr, "."); }
			}

			line = i;
			fprintf(stderr,"\n");
			if (input) { fprintf(stderr, "<< "); }
			else { fprintf(stderr, ">> "); }
		}

		fprintf(stderr,"%02X ", temp);
	}

	if (i % LEN_LINE != 0)
		for (j = 0 ; j < (int)(LEN_LINE-(i % LEN_LINE)); j++)
			fprintf(stderr,"   ");

	fprintf(stderr, "\t");
	for (j = line; j < len; j++){
		temp = (short unsigned int) array[j];
		if (temp > 31)
			fprintf(stderr, "%c", temp);
		else
			fprintf(stderr, ".");
	}

	fprintf(stderr,"\n\n");
#endif /* SNOOP */
}
int ReadUSBPost(struct libusb_device_handle *usb_handle, char *buffer, int length) {
	int ret = 0;
	int BytesRead = 0;

	ret = libusb_bulk_transfer(usb_handle, 0x82, buffer, length, &BytesRead, USB_READ_TIMEOUT);
	if (ret < 0) { printf("ERR: ReadUSB(): Could not read: %i\n", ret); }
	debug(1, buffer, BytesRead);
	return BytesRead;
}
int ReadUSB(struct libusb_device_handle *usb_handle, char *buffer, int length) {
	int ret = 0;
	int BytesRead = 0;

	ret = libusb_bulk_transfer(usb_handle, 0x82, buffer, length, &BytesRead, USB_READ_TIMEOUT);
	if (fx_getPacketType(buffer) != 0x05) {
	    ret = libusb_bulk_transfer(usb_handle, 0x82, buffer, length, &BytesRead, USB_READ_TIMEOUT);
	}
	if (ret < 0) { printf("ERR: ReadUSB(): Could not read: %i\n", ret); }
	debug(1, buffer, BytesRead);
	return BytesRead;
}
int WriteUSB(struct libusb_device_handle *usb_handle, char *buffer, int length) {
	int ret = 0;
	int BytesWritten = 0;

	ret = libusb_bulk_transfer(usb_handle, 0x1, buffer, length, &BytesWritten, USB_WRITE_TIMEOUT);
	if (ret < 0) { printf("ERR: WriteUSB: Could not write: %i\n", ret); }
	debug(0, buffer, BytesWritten);
	return BytesWritten;
}
