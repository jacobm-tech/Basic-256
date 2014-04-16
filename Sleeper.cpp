/** Copyright (C) 2014, James Reneau.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation; either version 2 of the License, or
 **  (at your option) any later version.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License along
 **  with this program; if not, write to the Free Software Foundation, Inc.,
 **  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include "Sleeper.h"

void Sleeper::sleepMS(long int ms) {
// sleep ms miliseconds
#ifdef WIN32
		Sleep(ms);
#else
        int s=0;
		if (ms>=1000) {
			s = (ms/1000);
			ms %= 1000;
		}
		struct timespec tim, tim2;
        tim.tv_sec = s;
		tim.tv_nsec = ms * 1000000L;
		nanosleep(&tim, &tim2);
#endif
}

void Sleeper::sleepSeconds(double s) {
	long int ms = s * 1000L;
	sleepMS(ms);
}

	
