/** Copyright (C) 2006, Ian Paul Larsen.
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


#ifndef __BASICOUTPUT_H
#define __BASICOUTPUT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QPaintEvent>

#include "ViewWidgetIFace.h"

class BasicOutput : public QTextEdit, public ViewWidgetIFace
{
  Q_OBJECT;
 public:
  	BasicOutput();
 	
  	char *inputString;
 	void inputStart();
	// Toolbar 
 	virtual bool initToolBar(ToolBar *);

 public slots:
  	void getInput();
	void slotPrint(); 
 
 signals:
  void inputEntered(QString);

 protected:
  void keyPressEvent(QKeyEvent *);
 
 private:
  int startPos;
  bool gettingInput;
};


#endif
