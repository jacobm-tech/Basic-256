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



#include <QSettings>

#ifndef SETTINGSH
	#define SETTINGSH
	#define SETTINGSORG "BASIC-256 Consortium"
	#define SETTINGSAPP "BASIC-256 IDE"
	#define SETTINGSPORTABLEINI	"BASIC256_IDE.ini"

	// main window
	#define SETTINGSVISIBLE "Main/Visible"
	#define SETTINGSFLOATVISIBLE "Main/Visible"
	#define SETTINGSPOS "Main/Pos"
	#define SETTINGSDEFAULT_X 100
	#define SETTINGSDEFAULT_Y 100
	#define SETTINGSSIZE "Main/Size"
	#define SETTINGSDEFAULT_W 800
	#define SETTINGSDEFAULT_H 600
    #define SETTINGSFONT "Main/Font"
    #define SETTINGSTOOLBAR "Main/Toolbar"
    #define SETTINGSTOOLBARDEFAULT true
    #define SETTINGSFONTDEFAULT "DejaVu Sans Mono,11,-1,5,50,0,0,0,0,0"

    #define SETTINGSEDITWHITESPACE "Edit/Whitespace"
    #define SETTINGSEDITWHITESPACEDEFAULT false
    
    #define SETTINGSEDITVISIBLE "EditDock/Visible"
    #define SETTINGSEDITVISIBLEDEFAULT true

    #define SETTINGSOUTVISIBLE "OutDock/Visible"
    #define SETTINGSOUTVISIBLEDEFAULT true
    #define SETTINGSOUTFLOAT "OutDock/Float"
    #define SETTINGSOUTPOS "OutDock/Pos"
	#define SETTINGSOUTDEFAULT_X 100
	#define SETTINGSOUTDEFAULT_Y 100
    #define SETTINGSOUTSIZE "OutDock/Size"
	#define SETTINGSOUTDEFAULT_W 400
	#define SETTINGSOUTDEFAULT_H 400
    #define SETTINGSOUTTOOLBAR "OutDock/Toolbar"
    #define SETTINGSOUTTOOLBARDEFAULT true

    #define SETTINGSGRAPHVISIBLE "GraphDock/Visible"
    #define SETTINGSGRAPHVISIBLEDEFAULT true
    #define SETTINGSGRAPHFLOAT "GraphDock/Float"
    #define SETTINGSGRAPHPOS "GraphDock/Pos"
	#define SETTINGSGRAPHDEFAULT_X 100
	#define SETTINGSGRAPHDEFAULT_Y 100
    #define SETTINGSGRAPHSIZE "GraphDock/Size"
	#define SETTINGSGRAPHDEFAULT_W 400
	#define SETTINGSGRAPHDEFAULT_H 400
    #define SETTINGSGRAPHTOOLBAR "GraphDock/Toolbar"
    #define SETTINGSGRAPHTOOLBARDEFAULT true
    #define SETTINGSGRAPHGRIDLINES "GraphDock/GridLines"
    #define SETTINGSGRAPHGRIDLINESDEFAUT false
    
    #define SETTINGSVARVISIBLE "VarDock/Visible"
    #define SETTINGSVARVISIBLEDEFAULT true
    #define SETTINGSVARFLOAT "VarDock/Float"
    #define SETTINGSVARPOS "VarDock/Pos"
	#define SETTINGSVARDEFAULT_X 100
	#define SETTINGSVARDEFAULT_Y 100
    #define SETTINGSVARSIZE "VarDock/Size"
	#define SETTINGSVARDEFAULT_W 400
	#define SETTINGSVARDEFAULT_H 400
    
	// other IDE preferences
	#define SETTINGSIDESAVEONRUN "IDE/SaveOnRun"
	#define SETTINGSIDESAVEONRUNDEFAULT false
	
	
	// documentation window
	#define SETTINGSDOCSIZE "Doc/Size"
	#define SETTINGSDOCPOS "Doc/Pos"
	
	// preferences window
	#define SETTINGSPREFPOS "Pref/Pos"
	#define SETTINGSPREFPASSWORD "Pref/Password"

	// Replace window
	#define SETTINGSREPLACEPOS "Replace/Pos"
	#define SETTINGSREPLACEFROM "Replace/From"
	#define SETTINGSREPLACETO "Replace/To"
    #define SETTINGSREPLACECASE "Replace/Case"
    #define SETTINGSREPLACECASEDEFAULT false
    #define SETTINGSREPLACEWORDS "Replace/Words"
    #define SETTINGSREPLACEWORDSDEFAULT false
    #define SETTINGSREPLACEBACK "Replace/Back"
	#define SETTINGSREPLACEBACKDEFAULT false

	// permissions
	#define SETTINGSALLOWSYSTEM "Allow/System"
	#define SETTINGSALLOWSYSTEMDEFAULT true
	#define SETTINGSALLOWSETTING "Allow/Setting"
	#define SETTINGSALLOWSETTINGDEFAULT true
	#define SETTINGSALLOWPORT "Allow/Port"
	#define SETTINGSALLOWPORTDEFAULT true

	// error reporting options
	#define SETTINGSERRORNONE 0
	#define SETTINGSERRORWARN 1
	#define SETTINGSERROR 2

	// user settings
	#define SETTINGSTYPECONV "Runtime/TypeConv"
	#define SETTINGSTYPECONVDEFAULT SETTINGSERRORNONE
	#define SETTINGSVARNOTASSIGNED "Runtime/VNA"
	#define SETTINGSVARNOTASSIGNEDDEFAULT SETTINGSERROR
	#define SETTINGSDEBUGSPEED "Runtime/DebugSpeed"
	#define SETTINGSDEBUGSPEEDDEFAULT 10
	#define SETTINGSDEBUGSPEEDMIN 1
	#define SETTINGSDEBUGSPEEDMAX 2000
	#define SETTINGSDECDIGS "Runtime/DecDigs"
	#define SETTINGSDECDIGSDEFAULT 12
	#define SETTINGSDECDIGSMIN 9
	#define SETTINGSDECDIGSMAX 14
	#define SETTINGSFLOATTAIL "Runtime/FloatTail"
	#define SETTINGSFLOATTAILDEFAULT true
	
	
	
	
	// espeak language settings
	#define SETTINGSESPEAKVOICE "eSpeak/Voice"
	#define SETTINGSESPEAKVOICEDEFAULT "default"
	
	// printersettings
	#define SETTINGSPRINTERPRINTER "Printer/Printer"
	#define SETTINGSPRINTERPDFFILE "Printer/PDFFile"
	#define SETTINGSPRINTERPAPER "Printer/Paper"
	#define SETTINGSPRINTERPAPERDEFAULT 2
	#define SETTINGSPRINTERRESOLUTION "Printer/Resolution"
	#define SETTINGSPRINTERRESOLUTIONDEFAULT 0
	#define SETTINGSPRINTERORIENT "Printer/Orient"
	#define SETTINGSPRINTERORIENTDEFAULT 0

	// store history of files as SaveHistory/0 ... SaveHistory/8 
	#define SETTINGSGROUPHIST "SaveHistory"
	#define SETTINGSGROUPHISTN 9

	// store user settings (setsetting/getsetting) in seperate group
	#define SETTINGSGROUPUSER "UserSettings"


    // You need an SETTINGS; statement when you are using settings in a function
    // this defines a QSettings variable named "setings" for your use
    #ifdef WIN32PORTABLE
		#include <QCoreApplication>
        #define SETTINGS QSettings settings( QCoreApplication::applicationDirPath() + "/../../Data/settings/"  + SETTINGSPORTABLEINI, QSettings::IniFormat );
    #else
        #define SETTINGS QSettings settings(SETTINGSORG, SETTINGSAPP);
    #endif


#endif
