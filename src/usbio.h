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


#ifndef USBIO_H
#define USBIO_H

#include <libusb-1.0/libusb.h>

#define LEN_LINE 16

#define USB_WRITE_TIMEOUT 300
#define USB_READ_TIMEOUT 600

void debug(int input, char* array, int len);
int ReadUSB(struct libusb_device_handle *usb_handle, char* buffer, int length);
int ReadUSBPost(struct libusb_device_handle *usb_handle, char* buffer, int length);
int WriteUSB(struct libusb_device_handle *usb_handle, char* buffer, int length);

#endif
