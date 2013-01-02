/** Copyright (C) 2010, J.M.Reneau.
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



#include <QMessageBox>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QToolBar>
#include <QLabel>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QAction>
#include <QObject>
#include <QIcon>

#ifndef PREFERENCESWINH

#define PREFERENCESWINH

class PreferencesWin : public QDialog
{
  Q_OBJECT;

 public:
	PreferencesWin(QWidget * parent);
	void closeEvent(QCloseEvent *);

private slots:
	void clickCancelButton();
	void clickSaveButton();

  
private:
	QLabel *passwordlabel;
	QLineEdit *passwordinput;
	QCheckBox *systemcheckbox;
	QCheckBox *settingcheckbox;
	QCheckBox *portcheckbox;
	QCheckBox *warningscheckbox;
	QPushButton *cancelbutton;
	QPushButton *savebutton;

};

#endif
