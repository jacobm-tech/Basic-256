; BASIC256.nsi
; Modification History
; date...... programmer... description...
; 2008-09-01 j.m.reneau    original coding

!include nsDialogs.nsh

Var VERSION
Var VERSIONDATE
var customDialog
var customLabel0
var customLabel1
var customImage
var customImageHandle

Function .onInit
  StrCpy $VERSION "0.9.6p"
  StrCpy $VERSIONDATE "2010-06-14"
FunctionEnd

Function customPage

	nsDialogs::Create /NOUNLOAD 1018
	Pop $customDialog

	${If} $customDialog == error
		Abort
	${EndIf}

	${NSD_CreateBitmap} 0 0 100% 100% ""
	Pop $customImage
	${NSD_SetImage} $customImage resources\images\basic256.bmp $customImageHandle

	${NSD_CreateLabel} 50 0 80% 10% "BASIC256 $VERSION ($VERSIONDATE)"
	Pop $customLabel0
	${NSD_CreateLabel} 0 50 100% 80% "This installer will load BASIC256.  Previous versions will be overwritten and any saved files will be preserved."
	Pop $customLabel1

	nsDialogs::Show
FunctionEnd


; The name of the installer
Name "BASIC256 $VERSION ($VERSIONDATE)"

; The file to write
OutFile "BASIC256-$VERSION_Win32_Install.exe"

; The default installation directory
InstallDir $PROGRAMFILES\BASIC256

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\BASIC256" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page custom customPage "" ": BASIC256 Welcome"
Page license
LicenseData "license.txt"
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "BASIC256"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File .\release\BASIC256.exe
  File .\release\mingwm10.dll
  File .\release\QtCore4.dll
  File .\release\QtGui4.dll
  File .\release\QtNetwork4.dll
  File .\release\QtWebKit4.dll
  File ChangeLog
  File CONTRIBUTORS
  File license.txt
  File /r /x ".svn" Examples 
  File /r /x ".svn" ..\doc\help 
  
  SetOutPath $INSTDIR\Translations 
  File .\Translations\*.qm

 ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\BASIC256 "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BASIC256" "DisplayName" "BASIC-256"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BASIC256" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BASIC256" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BASIC256" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  SetOutPath $INSTDIR 
  CreateDirectory "$SMPROGRAMS\BASIC256"
  CreateShortCut "$SMPROGRAMS\BASIC256\BASIC256.lnk" "$INSTDIR\BASIC256.exe" "" "$INSTDIR\BASIC256.exe" 0
  CreateShortCut "$SMPROGRAMS\BASIC256\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BASIC256"
  DeleteRegKey HKLM SOFTWARE\BASIC256

  ; Remove files and uninstaller
  Delete $INSTDIR\BASIC256.exe
  Delete $INSTDIR\mingwm10.dll
  Delete $INSTDIR\QtCore4.dll
  Delete $INSTDIR\QtGui4.dll
  Delete $INSTDIR\ChangeLog
  Delete $INSTDIR\CONTRIBUTORS
  Delete $INSTDIR\license.txt
  RMDir /r $INSTDIR\Examples
  RMDir /r $INSTDIR\Translations
  RMDir /r $INSTDIR\help
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\BASIC256\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\BASIC256"
  RMDir "$INSTDIR"

SectionEnd
