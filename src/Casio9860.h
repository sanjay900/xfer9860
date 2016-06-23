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

#ifndef CASIO_9860_H
#define CASIO_9860_H

#include <libusb-1.0/libusb.h>

#define C9860_VENDOR_ID		0x07CF
#define C9860_PRODUCT_ID	0x6101

/* OFFSETS
   for all packets: */
#define OFF_T	0	/* for clarity */
#define OFF_ST	1
#define OFF_DF	3
#define OFF_DS	4

/* for DATA packets: */
#define OFF_TP	8
#define OFF_PN	12
#define OFF_DD	16

/* for COMMAND packets: */
#define OFF_OW		8
#define OFF_DT		10
#define OFF_FS		12
#define OFF_SD1		20
#define OFF_SD2		22
#define OFF_SD3		24
#define OFF_SD4		26
#define OFF_SD5		28
#define OFF_SD6		30
#define OFF_D1		32
/* END OFFSETS */

/* Types (subtypes actually) used for specifying what packet does.
 * the positive and negative subtypes only have 1 effective byte. */
#define POSITIVE_NORMAL		'0'
#define POSITIVE_OVERWRITE	'1'
#define POSITIVE_SYSINFO	'2'

#define NEGATIVE_NORMAL		'0'
#define NEGATIVE_RETRANSMIT	'1'
#define NEGATIVE_FILEEXISTS	'2'
#define NEGATIVE_NOOVERWRITE	'3'
#define NEGATIVE_OVERWRITEERR	'4'
#define NEGATIVE_MEMFULL	'5'
#define NEGATIVE_IDENTIFY	'6'

#define T_POSITIVE	0x06
#define T_NEGATIVE	0x15
#define T_COMMAND	0x01
#define T_DATA		0x02
#define T_CHANGE	0x03
#define T_VERIFY	0x05

#define ST_FILE_TO_FLASH	"\x34\x35"

#define MAX_DATA_PAYLOAD 256

struct libusb_device_handle *fx_getDeviceHandle();
int fx_initDevice(struct libusb_device_handle *usb_handle);
void fx_releaseDeviceHandle(struct libusb_device_handle*);

/* routine functions */
int fx_doConnVer(struct libusb_device_handle*);
int fx_getFlashCapacity(struct libusb_device_handle*, char*);
int fx_getMCSCapacity(struct libusb_device_handle*);
/* packet functions */
int fx_sendComplete(struct libusb_device_handle*, char*);
int fx_sendVerify(struct libusb_device_handle*, char*, char*);
int fx_sendTerminate(struct libusb_device_handle*, char*);
int fx_sendPositive(struct libusb_device_handle*, char*, char);
int fx_sendNegative(struct libusb_device_handle*, char*, char);
int fx_sendChange_Direction(struct libusb_device_handle*, char*);
int fx_sendFlash_Capacity_Request(struct libusb_device_handle*, char*, char*);
int fx_sendFlashCollectGarbage(struct libusb_device_handle*, char*, char*);
int fx_sendMCSCapacityRequest(struct libusb_device_handle*, char*);
int fx_sendFlashFileTransmission(struct libusb_device_handle*, char*, int, char*, char*);
int fx_sendFlashFileTransmissionRequest(struct libusb_device_handle*, char*, char*, char*);
int fx_sendData(struct libusb_device_handle*, char*, char*, int, int, char*, int);

int fx_getPacketType(char*);

/* Service functions */
int fx_appendChecksum(char*, int);
int fx_escapeBytes(char*, char*, int);
int fx_unescapeBytes(char *, char*, int);
long int fx_asciiHexToInt(char *, int);

#endif
