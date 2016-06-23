/*******************************************************************************
	xfer9860 - a Casio fx-9860G (SD) communication utility
	Copyright (C) 2007
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

#ifndef CONFIG_H
#define CONFIG_H

/* leave as-is, scons takes care of this */
#ifndef VERSION
  #define VERSION "SVN"
#endif

#ifndef WIN32
 #include <unistd.h>
 #define MSLEEP(s)	usleep((s)*1000)
#else
 #include <windows.h>
#define MSLEEP(s)	Sleep(s)
#endif

#endif
