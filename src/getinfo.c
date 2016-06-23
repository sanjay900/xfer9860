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
#include <libusb-1.0/libusb.h>

#include "Casio9860.h"
#include "config.h"

int getInfo(int throttleSetting) {
	struct libusb_device_handle *usb_handle;
	int mcsCapacity, flashCapacity;
	char* buffer = (char*)calloc(142, sizeof(char));
	printf("[>] Setting up USB connection.. ");
	usb_handle = fx_getDeviceHandle();	/* initiates usb system */
	if (usb_handle == NULL) {
		printf(	"\n[E] A listening device could not be found.\n"
		      	"    Make sure it is receiving; press [ON], [MENU], [sin], [F2]\n");
		goto exit_unalloc;
	}
	if (fx_initDevice(usb_handle) < 0) {	/* does calculator-specific setup */
		printf("\n[E] Error initializing device.\n");
	}
	printf("Connected!\n");
	printf("[>] Verifying device.. ");
	if (fx_doConnVer(usb_handle) != 0) { printf("Failed.\n"); goto exit_release; }
	else { printf("Done!\n"); }

	MSLEEP(throttleSetting);
	printf("[I] Main memory:");
	mcsCapacity = fx_getMCSCapacity(usb_handle);
	if (mcsCapacity < 0) { printf("[E] Error requesting MCS capacity information.\n"); goto exit_release; }
	printf("\t%i%% available.\n", (mcsCapacity*100)/62928 );

	MSLEEP(throttleSetting);
	printf("[I] Storage memory:");
	flashCapacity = fx_getFlashCapacity(usb_handle, "fls0");
	if (flashCapacity < 0) { printf("\n[E] Error requesting flash capacity information.\n"); goto exit_release; }
	printf("\t%i%% available.\n", (flashCapacity*100)/1572864);

	MSLEEP(throttleSetting);
	printf("[>] Closing connection.. ");
	fx_sendComplete(usb_handle, buffer);
	printf("Done!\n");
exit_release:
	fx_releaseDeviceHandle(usb_handle);
exit_unalloc:
	free(buffer);
	return 0;
}
