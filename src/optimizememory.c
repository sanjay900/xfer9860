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

#include "Casio9860.h"

int optimizeMemory() {
	struct libusb_device_handle *usb_handle;
	char* buffer = (char*)calloc(0x40, sizeof(char)); /* Used for misc packet data */
	printf("[>] Setting up USB connection.. ");
	usb_handle = fx_getDeviceHandle();	/* initiates usb system */
	if (usb_handle == NULL) {
		printf(	"\n[E] A listening device could not be found.\n"
		      	"    Make sure it is receiving; press [ON], [MENU], [sin], [F2]\n");
		return 1;
	}
	if (fx_initDevice(usb_handle) < 0) {	/* does calculator-specific setup */
		printf("\n[E] Error initializing device.\n"); goto exit_release;
	}
	printf("Connected!\n");

	printf("[>] Verifying device.. ");
	if (fx_doConnVer(usb_handle) != 0) { printf("Failed.\n"); goto exit_release; }
	else { printf("Done!\n"); }

	if (buffer == NULL) { printf("[E] Memory allocation error.\n"); goto exit_release; }
	printf("[>] Requesting optimization of storage memory..\n");
	fx_sendFlashCollectGarbage(usb_handle, buffer, "fls0");
	fx_sendComplete(usb_handle, buffer);

	free(buffer);
exit_release:
	fx_releaseDeviceHandle(usb_handle);

	return 0;
}
