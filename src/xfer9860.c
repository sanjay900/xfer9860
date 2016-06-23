/*******************************************************************************
	xfer9860 - a Casio fx-9860G (SD) communication utility
	Copyright (C) 2007-2014
		Andreas Bertheussen <andreasmarcel@gmail.com>

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
#include <unistd.h>
#include <stdlib.h>

#include "config.h"

#include "uploadfile.h"
#include "downloadfile.h"
#include "getinfo.h"
#include "optimizememory.h"

void displayHelp();
void displayAbout();

int main(int argc, char *argv[]) {
	int opt;

	/* parameters */
	char *sourceFileName = NULL;
	int throttleSetting = 0;

	/* operations */
	int uploadFileFlag = 0;
	int downloadFileFlag = 0;

	printf("--- xfer9860 %s  Copyright (C) 2007-2014 Andreas Bertheussen, Manuel Naranjo and Bruno L. Alata..\n", VERSION);

	while ((opt = getopt(argc, argv, "t:u:d:ioha")) != -1) {
		switch(opt) {
			case 't':
				throttleSetting = atoi(optarg);
				break;
			case 'h': break;
			case 'u':
				sourceFileName = optarg;
				uploadFileFlag = 1;
				break;
			case 'd':
				sourceFileName = optarg;
				downloadFileFlag = 1;
				break;
			case 'i':
				getInfo(throttleSetting);
				return 0;
			case 'o':
				optimizeMemory();
				return 0;
			case 'a':
				displayAbout();
				return 0;
			default:
				return 0;
		}
	}

	if ((uploadFileFlag || downloadFileFlag) && optind >= argc) {
		printf("You must specify a destination file name.\n");
		return 1;
	}

	if (uploadFileFlag) { uploadFile(sourceFileName, argv[optind], throttleSetting);	return 0; }
	if (downloadFileFlag) { downloadFile(sourceFileName, argv[optind], throttleSetting);	return 0; }

	displayHelp(); /* default action if none of the above are catched */
	return 0;
}

void displayHelp() {
	printf(	"--- a Casio fx-9860G (SD) communication utility.\n"
		"Usage: xfer9860 <action> destname [-t throttle]\n"
		"Calculator actions:\n"
		" -u srcname\tUpload file `srcname' from disk to `destname' on device.\n"
		" -d srcname\tDownload file `srcname' from device to `destname' on disk.\n"
		"\t\tNote that this is significantly slower than uploading.\n"
		" -i\t\tShows information about the connected calculator.\n"
		" -o\t\tOptimize the storage memory (garbage collection).\n"
		"\n");
	printf(	"Parameters:\n"
		" -t value\tThrottle setting. The value specifies the delay in ms between\n"
		"\t\tpackets. Default is 0. Try increasing this in case of problems.\n"
		"\n"
		"Other:\n"
		" -h\t\tDisplay this help message.\n"
		" -a\t\tDisplay info about xfer9860 and its licensing.\n"
		"\n");
}

void displayAbout() {
	printf(	"--- a Casio fx-9860G (SD) communication utility.\n"
		"This program is free software; you can redistribute it and/or\n"
		"modify it under the terms of the GNU General Public License\n"
		"as published by the Free Software Foundation; either version 2\n"
		"of the License, or (at your option) any later version.\n"
		"\n");
	printf(	"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program. If not, see <http://www.gnu.org/licenses/>.\n"
		"\n");
}
