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


#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <qglobal.h>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QScrollArea>
#include <QClipboard>

#include "BasicWidget.h"
#include "BasicOutput.h"
#include "BasicEdit.h"
#include "BasicGraph.h"
#include "VariableWin.h"
#include "DocumentationWin.h"
#include "PreferencesWin.h"
#include "RunController.h"
#include "EditSyntaxHighlighter.h"
#include "Settings.h"
#include "DockWidget.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
  	MainWindow(QWidget * parent, Qt::WindowFlags f, QString localestring);
	~MainWindow();
	void loadCustomizations();
	void saveCustomizations();
	void ifGuiStateRun();
	void ifGuiStateClose();
	void closeEvent(QCloseEvent *);
	void setGuiState(int);
    void setEnabledEditorButtons(bool val);
    void setRunState(int);
    

	// Main IU Widgets
	BasicWidget * editwinwgt;
	BasicWidget * outwinwgt;
	BasicWidget * graphwinwgt;
	BasicWidget * varwinwgt;

	// file menu and choices
	QMenu * filemenu;
	QAction * newact;
	QAction * openact;
	QAction * saveact;
	QAction * saveasact;
	QAction * printact;
	QAction *recentact[SETTINGSGROUPHISTN]; 
	QAction * exitact;

	// edit menu and choices
	QMenu * editmenu;
	QAction *undoact;
	QAction *redoact;
	QAction *cutact;
	QAction *copyact;
	QAction *pasteact;
	QAction *selectallact;
	QAction *findact;
	QAction *findagain;
	QAction *replaceact;
	QAction *beautifyact;
	QAction *prefact;

	// view menu
	QMenu *viewmenu;
	QAction *editWinVisibleAct;
	QAction *outWinVisibleAct;
	QAction *graphWinVisibleAct;
	QAction *variableWinVisibleAct;
	QAction *editWhitespaceAct;
	QAction *graphGridVisibleAct;
	QAction *fontact;
	QAction *maintbaract;
	QAction *texttbaract;
	QAction *graphtbaract;

	// run menu
	QMenu *runmenu;
	QAction * runact;
	QAction * debugact;
	QAction * stepact;
	QAction * bpact;
    QAction * stopact;
    QAction * clearbreakpointsact;

    // help menu
    QAction * helpthis;

    RunController *rc;
    EditSyntaxHighlighter * editsyntax;

	QString localecode;
	QLocale *locale;
    bool undoButtonValue;
    bool redoButtonValue;
	
public slots:
  void updateStatusBar(QString);
  void updateWindowTitle(QString);

private:

	DockWidget * editdock;
	DockWidget * outdock;
	DockWidget * graphdock;
	QScrollArea * graphscroll;
	DockWidget * vardock;

	QToolBar *maintbar;
    
	// SEE GUISTATE* Constants
	int guiState;

	// void pointer to the run controller
	// can't specify type because of circular reference
	//void *rcvoidpointer;		

private slots:
	void updateRecent();
	void about();
    void updatePasteButton();
    void updateCopyCutButtons(bool);
    void slotUndoAvailable(bool val);
    void slotRedoAvailable(bool val);
    void checkGraphMenuVisible();
    void checkOutMenuVisible();
    void checkVarMenuVisible();
};

#endif
