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

#ifndef USEQSOUND

#include "BasicMediaPlayer.h"

void BasicMediaPlayer::loadFile(QString file) {
    // blocking load adapted from http://qt-project.org/wiki/seek_in_sound_file
    setMedia(QUrl::fromLocalFile(QFileInfo(file).absoluteFilePath()));
    if(!isSeekable()) {
		QEventLoop loop;
		QTimer timer;
		timer.setSingleShot(true);
		timer.setInterval(2000);
		loop.connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()) );
        loop.connect(this, SIGNAL(seekableChanged(bool)), &loop, SLOT(quit()));
		loop.exec();
	}
}

int BasicMediaPlayer::state() {
    // media state 0-stop, 1-play, 2-pause
    // this eventually needs to be removed and the base class state() should work
	//
	// with QT5.1 it has been observed that
	// some media files do not report stop at the end of the file
	// and continue to report playing forever
	// if playing check of the position increases after a short sleep time
	
	int s;
	qint64 starttime, endtime;
	Sleeper *sleeper = new Sleeper();
    s = QMediaPlayer::state();
	if (s==QMediaPlayer::PlayingState) {
        starttime = QMediaPlayer::position();
		sleeper->sleepMS(30);
        endtime = QMediaPlayer::position();
		if (starttime==endtime) s = QMediaPlayer::StoppedState; // stopped
	}
	return(s);
}
		
void BasicMediaPlayer::wait() {
	// wait for the media file to complete
	Sleeper *sleeper = new Sleeper();
    while(state()==QMediaPlayer::PlayingState) {
		sleeper->sleepMS(300);
	}
}

#endif

