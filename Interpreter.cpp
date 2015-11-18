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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cmath>
#include <string>
#include <errno.h>

#ifdef WIN32
#include <winsock.h>
#include <windows.h>

typedef int socklen_t;

// for parallel port operations
HINSTANCE inpout32dll = NULL;
typedef unsigned char (CALLBACK* InpOut32InpType)(short int);
typedef void (CALLBACK* InpOut32OutType)(short int, unsigned char);
InpOut32InpType Inp32 = NULL;
InpOut32OutType Out32 = NULL;
#else
// unix, mac, android
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <arpa/inet.h>
#include <net/if.h>
#ifndef ANDROID
#include <ifaddrs.h>
#endif
#include <unistd.h>
#endif


#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QTime>
#include <QMutex>
#include <QWaitCondition>
#include <QSerialPort>
#include <QCoreApplication>


#include <QtWidgets/QMessageBox>

using namespace std;

#include "LEX/basicParse.tab.h"
#include "WordCodes.h"
#include "CompileErrors.h"
#include "Interpreter.h"
#include "md5.h"
#include "Settings.h"
#include "Sound.h"


extern QMutex *mymutex;
extern QMutex *mydebugmutex;
extern QWaitCondition *waitCond;
extern QWaitCondition *waitDebugCond;

extern int currentKey;

extern BasicGraph *graphwin;

extern "C" {
    extern int basicParse(char *);
    extern int labeltable[];
    extern int linenumber;			// linenumber being LEXd
    extern int column;				// column on line being LEXd
    extern char* lexingfilename;	// current included file name being LEXd

    extern int numparsewarnings;
    extern int newWordCode();
    extern int bytesToFullWords(int size);
    extern int *wordCode;
    extern unsigned int wordOffset;
    extern unsigned int maxwordoffset;
    extern char *symtable[];
    extern int numsyms;

    // arrays to return warnings from compiler
    // defined in basicParse.y
    extern int parsewarningtable[];
    extern int parsewarningtablelinenumber[];
    extern int parsewarningtablecolumn[];
    extern char *parsewarningtablelexingfilename[];
}

Interpreter::Interpreter() {
    fastgraphics = false;
    directorypointer=NULL;
    status = R_STOPPED;
    printing = false;
	sleeper = new Sleeper();
    for (int t=0; t<NUMSOCKETS; t++) netsockfd[t]=-1;
    // on a windows box start winsock
#ifdef WIN32
    //
    // initialize the winsock network library
    WSAData wsaData;
    int nCode;
    if ((nCode = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0) {
        emit(outputReady(tr("ERROR - Unable to initialize Winsock library.\n")));
    }
    //
    // initialize the inpout32 dll
    inpout32dll  = LoadLibrary(L"inpout32.dll");
    if (inpout32dll==NULL) {
        emit(outputReady(tr("ERROR - Unable to find inpout32.dll - direct port I/O disabled.\n")));
    } else {
        Inp32 = (InpOut32InpType) GetProcAddress(inpout32dll, "Inp32");
        if (Inp32==NULL) {
            emit(outputReady(tr("ERROR - Unable to find Inp32 in inpout32.dll - direct port I/O disabled.\n")));
        }
        Out32 = (InpOut32OutType) GetProcAddress(inpout32dll, "Out32");
        if (Inp32==NULL) {
            emit(outputReady(tr("ERROR - Unable to find Out32 in inpout32.dll - direct port I/O disabled.\n")));
        }
    }
#endif
    //
    // initialize pointers used for database recordsets (querries)
    for (int t=0; t<NUMDBCONN; t++) {
        for (int u=0; u<NUMDBSET; u++) {
            dbSet[t][u] = NULL;
        }
    }

}

Interpreter::~Interpreter() {
    // on a windows box stop winsock
#ifdef WIN32
    WSACleanup();
#endif
	delete sleeper;
}


int Interpreter::optype(int op) {
    // use constantants found in WordCodes.h
    return (OPTYPE_MASK & op) ;
}

QString Interpreter::opname(int op) {
    // used to convert opcode number in debuginfo to opcode name
    if (op==OP_END) return QString("OP_END");
    else if (op==OP_NOP) return QString("OP_NOP");
    else if (op==OP_RETURN) return QString("OP_RETURN");
    else if (op==OP_CONCAT) return QString("OP_CONCAT");
    else if (op==OP_EQUAL) return QString("OP_EQUAL");
    else if (op==OP_NEQUAL) return QString("OP_NEQUAL");
    else if (op==OP_GT) return QString("OP_GT");
    else if (op==OP_LT) return QString("OP_LT");
    else if (op==OP_GTE) return QString("OP_GTE");
    else if (op==OP_LTE) return QString("OP_LTE");
    else if (op==OP_AND) return QString("OP_AND");
    else if (op==OP_NOT) return QString("OP_NOT");
    else if (op==OP_OR) return QString("OP_OR");
    else if (op==OP_XOR) return QString("OP_XOR");
    else if (op==OP_INT) return QString("OP_INT");
    else if (op==OP_STRING) return QString("OP_STRING");
    else if (op==OP_ADD) return QString("OP_ADD");
    else if (op==OP_SUB) return QString("OP_SUB");
    else if (op==OP_MUL) return QString("OP_MUL");
    else if (op==OP_DIV) return QString("OP_DIV");
    else if (op==OP_EX) return QString("OP_EX");
    else if (op==OP_NEGATE) return QString("OP_NEGATE");
    else if (op==OP_PRINT) return QString("OP_PRINT");
    else if (op==OP_PRINTN) return QString("OP_PRINTN");
    else if (op==OP_INPUT) return QString("OP_INPUT");
    else if (op==OP_KEY) return QString("OP_KEY");
    else if (op==OP_PLOT) return QString("OP_PLOT");
    else if (op==OP_RECT) return QString("OP_RECT");
    else if (op==OP_CIRCLE) return QString("OP_CIRCLE");
    else if (op==OP_LINE) return QString("OP_LINE");
    else if (op==OP_REFRESH) return QString("OP_REFRESH");
    else if (op==OP_FASTGRAPHICS) return QString("OP_FASTGRAPHICS");
    else if (op==OP_CLS) return QString("OP_CLS");
    else if (op==OP_CLG) return QString("OP_CLG");
    else if (op==OP_GRAPHSIZE) return QString("OP_GRAPHSIZE");
    else if (op==OP_GRAPHWIDTH) return QString("OP_GRAPHWIDTH");
    else if (op==OP_GRAPHHEIGHT) return QString("OP_GRAPHHEIGHT");
    else if (op==OP_SIN) return QString("OP_SIN");
    else if (op==OP_COS) return QString("OP_COS");
    else if (op==OP_TAN) return QString("OP_TAN");
    else if (op==OP_RAND) return QString("OP_RAND");
    else if (op==OP_CEIL) return QString("OP_CEIL");
    else if (op==OP_FLOOR) return QString("OP_FLOOR");
    else if (op==OP_ABS) return QString("OP_ABS");
    else if (op==OP_PAUSE) return QString("OP_PAUSE");
    else if (op==OP_LENGTH) return QString("OP_LENGTH");
    else if (op==OP_MID) return QString("OP_MID");
    else if (op==OP_MIDX) return QString("OP_MIDX");
    else if (op==OP_INSTR) return QString("OP_INSTR");
    else if (op==OP_INSTRX) return QString("OP_INSTRX");
    else if (op==OP_OPEN) return QString("OP_OPEN");
    else if (op==OP_READ) return QString("OP_READ");
    else if (op==OP_WRITE) return QString("OP_WRITE");
    else if (op==OP_CLOSE) return QString("OP_CLOSE");
    else if (op==OP_RESET) return QString("OP_RESET");
    else if (op==OP_INCREASERECURSE) return QString("OP_INCREASERECURSE");
    else if (op==OP_DECREASERECURSE) return QString("OP_DECREASERECURSE");
    else if (op==OP_ASC) return QString("OP_ASC");
    else if (op==OP_CHR) return QString("OP_CHR");
    else if (op==OP_FLOAT) return QString("OP_FLOAT");
    else if (op==OP_READLINE) return QString("OP_READLINE");
    else if (op==OP_EOF) return QString("OP_EOF");
    else if (op==OP_MOD) return QString("OP_MOD");
    else if (op==OP_YEAR) return QString("OP_YEAR");
    else if (op==OP_MONTH) return QString("OP_MONTH");
    else if (op==OP_DAY) return QString("OP_DAY");
    else if (op==OP_HOUR) return QString("OP_HOUR");
    else if (op==OP_MINUTE) return QString("OP_MINUTE");
    else if (op==OP_SECOND) return QString("OP_SECOND");
    else if (op==OP_MOUSEX) return QString("OP_MOUSEX");
    else if (op==OP_MOUSEY) return QString("OP_MOUSEY");
    else if (op==OP_MOUSEB) return QString("OP_MOUSEB");
    else if (op==OP_CLICKCLEAR) return QString("OP_CLICKCLEAR");
    else if (op==OP_CLICKX) return QString("OP_CLICKX");
    else if (op==OP_CLICKY) return QString("OP_CLICKY");
    else if (op==OP_CLICKB) return QString("OP_CLICKB");
    else if (op==OP_TEXT) return QString("OP_TEXT");
    else if (op==OP_FONT) return QString("OP_FONT");
    else if (op==OP_SAY) return QString("OP_SAY");
    else if (op==OP_WAVPAUSE) return QString("OP_WAVPAUSE");
    else if (op==OP_WAVPLAY) return QString("OP_WAVPLAY");
    else if (op==OP_WAVSTOP) return QString("OP_WAVSTOP");
    else if (op==OP_SEEK) return QString("OP_SEEK");
    else if (op==OP_SIZE) return QString("OP_SIZE");
    else if (op==OP_EXISTS) return QString("OP_EXISTS");
    else if (op==OP_LEFT) return QString("OP_LEFT");
    else if (op==OP_RIGHT) return QString("OP_RIGHT");
    else if (op==OP_UPPER) return QString("OP_UPPER");
    else if (op==OP_LOWER) return QString("OP_LOWER");
    else if (op==OP_SYSTEM) return QString("OP_SYSTEM");
    else if (op==OP_VOLUME) return QString("OP_VOLUME");
    else if (op==OP_SETCOLOR) return QString("OP_SETCOLOR");
    else if (op==OP_RGB) return QString("OP_RGB");
    else if (op==OP_PIXEL) return QString("OP_PIXEL");
    else if (op==OP_GETCOLOR) return QString("OP_GETCOLOR");
    else if (op==OP_ASIN) return QString("OP_ASIN");
    else if (op==OP_ACOS) return QString("OP_ACOS");
    else if (op==OP_ATAN) return QString("OP_ATAN");
    else if (op==OP_DEGREES) return QString("OP_DEGREES");
    else if (op==OP_RADIANS) return QString("OP_RADIANS");
    else if (op==OP_INTDIV) return QString("OP_INTDIV");
    else if (op==OP_LOG) return QString("OP_LOG");
    else if (op==OP_LOGTEN) return QString("OP_LOGTEN");
    else if (op==OP_GETSLICE) return QString("OP_GETSLICE");
    else if (op==OP_PUTSLICE) return QString("OP_PUTSLICE");
    else if (op==OP_PUTSLICEMASK) return QString("OP_PUTSLICEMASK");
    else if (op==OP_IMGLOAD) return QString("OP_IMGLOAD");
    else if (op==OP_SQR) return QString("OP_SQR");
    else if (op==OP_EXP) return QString("OP_EXP");
    else if (op==OP_ARGUMENTCOUNTTEST) return QString("OP_ARGUMENTCOUNTTEST");
    else if (op==OP_THROWERROR) return QString("OP_THROWERROR");
    else if (op==OP_READBYTE) return QString("OP_READBYTE");
    else if (op==OP_WRITEBYTE) return QString("OP_WRITEBYTE");
    else if (op==OP_STACKSWAP) return QString("OP_STACKSWAP");
    else if (op==OP_STACKTOPTO2) return QString("OP_STACKTOPTO2");
    else if (op==OP_STACKDUP) return QString("OP_STACKDUP");
    else if (op==OP_STACKDUP2) return QString("OP_STACKDUP2");
    else if (op==OP_STACKSWAP2) return QString("OP_STACKSWAP2");
    else if (op==OP_GOTO) return QString("OP_GOTO");
    else if (op==OP_GOSUB) return QString("OP_GOSUB");
    else if (op==OP_BRANCH) return QString("OP_BRANCH");
    else if (op==OP_ASSIGN) return QString("OP_ASSIGN");
    else if (op==OP_ARRAYASSIGN) return QString("OP_ARRAYASSIGN");
    else if (op==OP_PUSHVAR) return QString("OP_PUSHVAR");
    else if (op==OP_PUSHINT) return QString("OP_PUSHINT");
    else if (op==OP_FOR) return QString("OP_FOR");
    else if (op==OP_NEXT) return QString("OP_NEXT");
    else if (op==OP_CURRLINE) return QString("OP_CURRLINE");
    else if (op==OP_DIM) return QString("OP_DIM");
    else if (op==OP_DIMSTR) return QString("OP_DIMSTR");
    else if (op==OP_ONERRORGOSUB) return QString("OP_ONERRORGOSUB");
    else if (op==OP_ONERRORCATCH) return QString("OP_ONERRORCATCH");
    else if (op==OP_EXPLODESTR) return QString("OP_EXPLODESTR");
    else if (op==OP_EXPLODE) return QString("OP_EXPLODE");
    else if (op==OP_EXPLODEXSTR) return QString("OP_EXPLODEXSTR");
    else if (op==OP_EXPLODEX) return QString("OP_EXPLODEX");
    else if (op==OP_IMPLODE) return QString("OP_IMPLODE");
    else if (op==OP_GLOBAL) return QString("OP_GLOBAL");
    else if (op==OP_STAMP_LIST) return QString("OP_STAMP_LIST");
    else if (op==OP_STAMP_S_LIST) return QString("OP_STAMP_S_LIST");
    else if (op==OP_STAMP_SR_LIST) return QString("OP_STAMP_SR_LIST");
    else if (op==OP_POLY_LIST) return QString("OP_POLY_LIST");
    else if (op==OP_WRITELINE) return QString("OP_WRITELINE");
    else if (op==OP_SOUND_LIST) return QString("OP_SOUND_LIST");
    else if (op==OP_DEREF) return QString("OP_DEREF");
    else if (op==OP_REDIM) return QString("OP_REDIM");
    else if (op==OP_REDIMSTR) return QString("OP_REDIMSTR");
    else if (op==OP_ALEN) return QString("OP_ALEN");
    else if (op==OP_ALENX) return QString("OP_ALENX");
    else if (op==OP_ALENY) return QString("OP_ALENY");
    else if (op==OP_PUSHVARREF) return QString("OP_PUSHVARREF");
    else if (op==OP_VARREFASSIGN) return QString("OP_VARREFASSIGN");
    else if (op==OP_VARREFSTRASSIGN) return QString("OP_VARREFSTRASSIGN");
    else if (op==OP_ARRAYLISTASSIGN) return QString("OP_ARRAYLISTASSIGN");
    else if (op==OP_STRARRAYLISTASSIGN) return QString("OP_STRARRAYLISTASSIGN");
    else if (op==OP_ARRAY2STACK) return QString("OP_ARRAY2STACK");
    else if (op==OP_PUSHFLOAT) return QString("OP_PUSHFLOAT");
    else if (op==OP_PUSHSTRING) return QString("OP_PUSHSTRING");
    else if (op==OP_INCLUDEFILE) return QString("OP_INCLUDEFILE");
    else if (op==OP_SPRITEPOLY_LIST) return QString("OP_SPRITEPOLY");
    else if (op==OP_SPRITEDIM) return QString("OP_SPRITEDIM");
    else if (op==OP_SPRITELOAD) return QString("OP_SPRITELOAD");
    else if (op==OP_SPRITESLICE) return QString("OP_SPRITESLICE");
    else if (op==OP_SPRITEMOVE) return QString("OP_SPRITEMOVE");
    else if (op==OP_SPRITEHIDE) return QString("OP_SPRITEHIDE");
    else if (op==OP_SPRITESHOW) return QString("OP_SPRITESHOW");
    else if (op==OP_SPRITECOLLIDE) return QString("OP_SPRITECOLLIDE");
    else if (op==OP_SPRITEPLACE) return QString("OP_SPRITEPLACE");
    else if (op==OP_SPRITEX) return QString("OP_SPRITEX");
    else if (op==OP_SPRITEY) return QString("OP_SPRITEY");
    else if (op==OP_SPRITEH) return QString("OP_SPRITEH");
    else if (op==OP_SPRITEW) return QString("OP_SPRITEW");
    else if (op==OP_SPRITEV) return QString("OP_SPRITEV");
    else if (op==OP_CHANGEDIR) return QString("OP_CHANGEDIR");
    else if (op==OP_CURRENTDIR) return QString("OP_CURRENTDIR");
    else if (op==OP_DBOPEN) return QString("OP_DBOPEN");
    else if (op==OP_DBCLOSE) return QString("OP_DBCLOSE");
    else if (op==OP_DBEXECUTE) return QString("OP_DBEXECUTE");
    else if (op==OP_DBOPENSET) return QString("OP_DBOPENSET");
    else if (op==OP_DBCLOSESET) return QString("OP_DBCLOSESET");
    else if (op==OP_DBROW) return QString("OP_DBROW");
    else if (op==OP_DBINT) return QString("OP_DBINT");
    else if (op==OP_DBFLOAT) return QString("OP_DBFLOAT");
    else if (op==OP_DBSTRING) return QString("OP_DBSTRING");
    else if (op==OP_LASTERROR) return QString("OP_LASTERROR");
    else if (op==OP_LASTERRORLINE) return QString("OP_LASTERRORLINE");
    else if (op==OP_LASTERRORMESSAGE) return QString("OP_LASTERRORMESSAGE");
    else if (op==OP_LASTERROREXTRA) return QString("OP_LASTERROREXTRA");
    else if (op==OP_OFFERROR) return QString("OP_OFFERROR");
    else if (op==OP_NETLISTEN) return QString("OP_NETLISTEN");
    else if (op==OP_NETCONNECT) return QString("OP_NETCONNECT");
    else if (op==OP_NETREAD) return QString("OP_NETREAD");
    else if (op==OP_NETWRITE) return QString("OP_NETWRITE");
    else if (op==OP_NETCLOSE) return QString("OP_NETCLOSE");
    else if (op==OP_NETDATA) return QString("OP_NETDATA");
    else if (op==OP_NETADDRESS) return QString("OP_NETADDRESS");
    else if (op==OP_KILL) return QString("OP_KILL");
    else if (op==OP_MD5) return QString("OP_MD5");
    else if (op==OP_SETSETTING) return QString("OP_SETSETTING");
    else if (op==OP_GETSETTING) return QString("OP_GETSETTING");
    else if (op==OP_PORTIN) return QString("OP_PORTIN");
    else if (op==OP_PORTOUT) return QString("OP_PORTOUT");
    else if (op==OP_BINARYOR) return QString("OP_BINARYOR");
    else if (op==OP_BINARYAND) return QString("OP_BINARYAND");
    else if (op==OP_BINARYNOT) return QString("OP_BINARYNOT");
    else if (op==OP_IMGSAVE) return QString("OP_IMGSAVE");
    else if (op==OP_DIR) return QString("OP_DIR");
    else if (op==OP_REPLACE) return QString("OP_REPLACE");
    else if (op==OP_REPLACEX) return QString("OP_REPLACEX");
    else if (op==OP_COUNT) return QString("OP_COUNT");
    else if (op==OP_COUNTX) return QString("OP_COUNTX");
    else if (op==OP_OSTYPE) return QString("OP_OSTYPE");
    else if (op==OP_MSEC) return QString("OP_MSEC");
    else if (op==OP_EDITVISIBLE) return QString("OP_EDITVISIBLE");
    else if (op==OP_GRAPHVISIBLE) return QString("OP_GRAPHVISIBLE");
    else if (op==OP_OUTPUTVISIBLE) return QString("OP_OUTPUTVISIBLE");
    else if (op==OP_TEXTHEIGHT) return QString("OP_TEXTHEIGHT");
    else if (op==OP_TEXTWIDTH) return QString("OP_TEXTWIDTH");
    else if (op==OP_SPRITER) return QString("OP_SPRITER");
    else if (op==OP_SPRITES) return QString("OP_SPRITES");
    else if (op==OP_FREEFILE) return QString("OP_FREEFILE");
    else if (op==OP_FREENET) return QString("OP_FREENET");
    else if (op==OP_FREEDB) return QString("OP_FREEDB");
    else if (op==OP_FREEDBSET) return QString("OP_FREEDBSET");
    else if (op==OP_DBINTS) return QString("OP_DBINTS");
    else if (op==OP_DBFLOATS) return QString("OP_DBFLOATS");
    else if (op==OP_DBSTRINGS) return QString("OP_DBSTRINGS");
    else if (op==OP_DBNULL) return QString("OP_DBNULL");
    else if (op==OP_DBNULLS) return QString("OP_DBNULLS");
    else if (op==OP_ARC) return QString("OP_ARC");
    else if (op==OP_CHORD) return QString("OP_CHORD");
    else if (op==OP_PIE) return QString("OP_PIE");
    else if (op==OP_PENWIDTH) return QString("OP_PENWIDTH");
    else if (op==OP_GETPENWIDTH) return QString("OP_GETPENWIDTH");
    else if (op==OP_GETBRUSHCOLOR) return QString("OP_GETBRUSHCOLOR");
    else if (op==OP_ALERT) return QString("OP_ALERT");
    else if (op==OP_CONFIRM) return QString("OP_CONFIRM");
    else if (op==OP_PROMPT) return QString("OP_PROMPT");
    else if (op==OP_FROMRADIX) return QString("OP_FROMRADIX");
    else if (op==OP_TORADIX) return QString("OP_TORADIX");
    else if (op==OP_PRINTERPAGE) return QString("OP_PRINTERPAGE");
    else if (op==OP_PRINTEROFF) return QString("OP_PRINTERON");
    else if (op==OP_PRINTERON) return QString("OP_PRINTEROFF");
    else if (op==OP_DEBUGINFO) return QString("OP_DEBUGINFO");
    else if (op==OP_WAVLENGTH) return QString("OP_WAVLENGTH");
    else if (op==OP_WAVPOS) return QString("OP_WAVPOS");
    else if (op==OP_WAVSEEK) return QString("OP_WAVSEEK");
    else if (op==OP_WAVSTATE) return QString("OP_WAVSTATE");
    else if (op==OP_REGEXMINIMAL) return QString("OP_REGEXMINIMAL");
    else if (op==OP_OPENSERIAL) return QString("OP_OPENSERIAL");
    else return QString("OP_UNKNOWN");
}


void Interpreter::printError(int e, QString message) {
    QString msg;
    if (e<WARNING_START) {
        msg = tr("ERROR");
    } else {
        msg = tr("WARNING");
    }
    if (currentIncludeFile!="") {
        msg += tr(" in included file ") + currentIncludeFile;
    }
    msg += tr(" on line ") + QString::number(currentLine) + tr(": ") + getErrorMessage(e);
    if (message!="") msg+= " " + message;
    msg += ".\n";
    emit(outputReady(msg));
}

QString Interpreter::getErrorMessage(int e) {
    QString errormessage("");
    switch(e) {
        case ERROR_NOSUCHLABEL:
            errormessage = tr("No such label %VARNAME%");
            break;
        case ERROR_FOR1:
            errormessage = tr("Illegal FOR -- start number > end number");
            break;
        case ERROR_FOR2 :
            errormessage = tr("Illegal FOR -- start number < end number");
            break;
        case ERROR_NEXTNOFOR:
            errormessage = tr("Next without FOR");
            break;
        case ERROR_FILENUMBER:
            errormessage = tr("Invalid File Number");
            break;
        case ERROR_FILEOPEN:
            errormessage = tr("Unable to open file");
            break;
        case ERROR_FILENOTOPEN:
            errormessage = tr("File not open");
            break;
        case ERROR_FILEWRITE:
            errormessage = tr("Unable to write to file");
            break;
        case ERROR_FILERESET:
            errormessage = tr("Unable to reset file");
            break;
        case ERROR_ARRAYSIZELARGE:
            errormessage = tr("Array %VARNAME% dimension too large");
            break;
        case ERROR_ARRAYSIZESMALL:
            errormessage = tr("Array %VARNAME% dimension too small");
            break;
        case ERROR_NOSUCHVARIABLE:
            errormessage = tr("Unknown variable %VARNAME%");
            break;
        case ERROR_NOTARRAY:
            errormessage = tr("Variable %VARNAME% is not an array");
            break;
        case ERROR_NOTSTRINGARRAY:
            errormessage = tr("Variable %VARNAME% is not a string array");
            break;
        case ERROR_ARRAYINDEX:
            errormessage = tr("Array %VARNAME% index out of bounds");
            break;
        case ERROR_STRNEGLEN:
            errormessage = tr("Substring length less that zero");
            break;
        case ERROR_STRSTART:
            errormessage = tr("Starting position less than zero");
            break;
        case ERROR_NONNUMERIC:
            errormessage = tr("Non-numeric value in numeric expression");
            break;
        case ERROR_RGB:
            errormessage = tr("RGB Color values must be in the range of 0 to 255");
            break;
        case ERROR_PUTBITFORMAT:
            errormessage = tr("String input to putbit incorrect");
            break;
        case ERROR_POLYARRAY:
            errormessage = tr("Argument not an array for poly()/stamp()");
            break;
        case ERROR_POLYPOINTS:
            errormessage = tr("Not enough points in array for poly()/stamp()");
            break;
        case ERROR_IMAGEFILE:
            errormessage = tr("Unable to load image file");
            break;
        case ERROR_SPRITENUMBER:
            errormessage = tr("Sprite number out of range");
            break;
        case ERROR_SPRITENA:
            errormessage = tr("Sprite has not been assigned");
            break;
        case ERROR_SPRITESLICE:
            errormessage = tr("Unable to slice image");
            break;
        case ERROR_FOLDER:
            errormessage = tr("Invalid directory name");
            break;
        case ERROR_INFINITY:
            errormessage = tr("Operation returned infinity");
            break;
        case ERROR_DBOPEN:
            errormessage = tr("Unable to open SQLITE database");
            break;
        case ERROR_DBQUERY:
            errormessage = tr("Database query error (message follows)");
            break;
        case ERROR_DBNOTOPEN:
            errormessage = tr("Database must be opened first");
            break;
        case ERROR_DBCOLNO:
            errormessage = tr("Column number out of range or column name not in data set");
            break;
        case ERROR_DBNOTSET:
            errormessage = tr("Record set must be opened first");
            break;
        case ERROR_NETSOCK:
            errormessage = tr("Error opening network socket");
            break;
        case ERROR_NETHOST:
            errormessage = tr("Error finding network host");
            break;
        case ERROR_NETCONN:
            errormessage = tr("Unable to connect to network host");
            break;
        case ERROR_NETREAD:
            errormessage = tr("Unable to read from network connection");
            break;
        case ERROR_NETNONE:
            errormessage = tr("Network connection has not been opened");
            break;
        case ERROR_NETWRITE:
            errormessage = tr("Unable to write to network connection");
            break;
        case ERROR_NETSOCKOPT:
            errormessage = tr("Unable to set network socket options");
            break;
        case ERROR_NETBIND:
            errormessage = tr("Unable to bind network socket");
            break;
        case ERROR_NETACCEPT:
            errormessage = tr("Unable to accept network connection");
            break;
        case ERROR_NETSOCKNUMBER:
            errormessage = tr("Invalid Socket Number");
            break;
        case ERROR_PERMISSION:
            errormessage = tr("You do not have permission to use this statement/function");
            break;
        case ERROR_IMAGESAVETYPE:
            errormessage = tr("Invalid image save type");
            break;
        case ERROR_ARGUMENTCOUNT:
            errormessage = tr("Number of arguments passed does not match FUNCTION/SUBROUTINE definition");
            break;
        case ERROR_MAXRECURSE:
            errormessage = tr("Maximum levels of recursion exceeded");
            break;
        case ERROR_DIVZERO:
            errormessage = tr("Division by zero");
            break;
        case ERROR_BYREF:
            errormessage = tr("Function/Subroutine expecting variable reference in call");
            break;
        case ERROR_BYREFTYPE:
            errormessage = tr("Function/Subroutine variable incorrect reference type in call");
            break;
        case ERROR_FREEFILE:
            errormessage = tr("There are no free file numbers to allocate");
            break;
        case ERROR_FREENET:
            errormessage = tr("There are no free network connections to allocate");
            break;
        case ERROR_FREEDB:
            errormessage = tr("There are no free database connections to allocate");
            break;
        case ERROR_DBCONNNUMBER:
            errormessage = tr("Invalid Database Connection Number");
            break;
        case ERROR_FREEDBSET:
            errormessage = tr("There are no free data sets to allocate for that database connection");
            break;
        case ERROR_DBSETNUMBER:
            errormessage = tr("Invalid data set number");
            break;
        case ERROR_DBNOTSETROW:
            errormessage = tr("You must advance the data set using DBROW before you can read data from it");
            break;
        case ERROR_PENWIDTH:
            errormessage = tr("Drawing pen width must be a non-negative number");
            break;
        case ERROR_COLORNUMBER:
            errormessage = tr("Color values must be in the range of -1 to 16,777,215");
            break;
        case ERROR_ARRAYINDEXMISSING:
            errormessage = tr("Array variable %VARNAME% has no value without an index");
            break;
        case ERROR_IMAGESCALE:
            errormessage = tr("Image scale must be greater than or equal to zero");
            break;
        case ERROR_FONTSIZE:
            errormessage = tr("Font size, in points, must be greater than or equal to zero");
            break;
        case ERROR_FONTWEIGHT:
            errormessage = tr("Font weight must be greater than or equal to zero");
            break;
        case ERROR_RADIXSTRING:
            errormessage = tr("Unable to convert radix string back to a decimal number");
            break;
        case ERROR_RADIX:
            errormessage = tr("Radix conversion base muse be between 2 and 36");
            break;
        case ERROR_LOGRANGE:
            errormessage = tr("Unable to calculate the logarithm or root of a negative number");
            break;
        case ERROR_STRINGMAXLEN:
            errormessage = tr("String exceeds maximum length of 16,777,216 characters");
            break;
        case ERROR_STACKUNDERFLOW:
            errormessage = tr("Stack Underflow Error");
            break;
        case ERROR_NOTANUMBER:
            errormessage = tr("Mathematical operation returned an undefined value");
            break;
        case ERROR_PRINTERNOTON:
            errormessage = tr("Printer is not on");
            break;
        case ERROR_PRINTERNOTOFF:
            errormessage = tr("Printing is already on");
            break;
        case ERROR_PRINTEROPEN:
            errormessage = tr("Unable to open printer");
            break;
        case ERROR_TYPECONV:
            errormessage = tr("Unable to convert string to number");
            break;
        case ERROR_WAVFILEFORMAT:
            errormessage = tr("Media file does not exist or in an unsupported format");
            break;
        case ERROR_FILEOPERATION:
            errormessage = tr("Can not perform that operation on a Serial Port");
            break;
        case ERROR_SERIALPARAMETER:
            errormessage = tr("Invalid serial port parameter");
            break;
        case ERROR_VARNOTASSIGNED:
            errormessage = tr("Variable has not been assigned a value.");
            break;


        // put ERROR new messages here
        case ERROR_NOTIMPLEMENTED:
            errormessage = tr("Feature not implemented in this environment");
            break;
        // warnings
        case WARNING_TYPECONV:
            errormessage = tr("Unable to convert string to number, zero used");
            break;
        case WARNING_WAVNOTSEEKABLE:
            errormessage = tr("Media file is not seekable");
            break;
        case WARNING_WAVNODURATION:
            errormessage = tr("Duration is not available for media file");
            break;
        case WARNING_VARNOTASSIGNED:
            errormessage = tr("Variable has not been assigned a value.");
            break;


        // put WARNING new messages here
        default:
            errormessage = tr("User thrown error number");

    }
    if (errormessage.contains(QString("%VARNAME%"),Qt::CaseInsensitive)) {
        if (symtable[errorvarnum]) {
            errormessage.replace(QString("%VARNAME%"),QString(symtable[errorvarnum]),Qt::CaseInsensitive);
        }
    }
    return errormessage;
}

int Interpreter::netSockClose(int fd) {
    // tidy up a network socket and return NULL to assign to the
    // fd variable to mark as closed as closed
    // call  f = netSockClose(f);
    if(fd>=0) {
#ifdef WIN32
        closesocket(fd);
#else
        close(fd);
#endif
    }
    return(-1);
}

void
Interpreter::setInputReady() {
    status = R_INPUTREADY;
}

bool
Interpreter::isAwaitingInput() {
    if (status == R_INPUT) {
        return true;
    }
    return false;
}

bool
Interpreter::isRunning() {
    if (status != R_STOPPED) {
        return true;
    }
    return false;
}


bool
Interpreter::isStopped() {
    if (status == R_STOPPED) {
        return true;
    }
    return false;
}


void Interpreter::clearsprites() {
    // cleanup sprites - release images and deallocate the space
    int i;
    if (nsprites>0) {
        for(i=0; i<nsprites; i++) {
            if (sprites[i].image) {
                delete sprites[i].image;
                sprites[i].image = NULL;
            }
            if (sprites[i].underimage) {
                delete sprites[i].underimage;
                sprites[i].underimage = NULL;
            }
        }
        free(sprites);
        sprites = NULL;
        nsprites = 0;
    }
}

void Interpreter::spriteundraw(int n) {
    // undraw all visible sprites >= n
    int x, y, i;
    QPainter *ian;
    ian = new QPainter(graphwin->image);
    i = nsprites-1;
    while(i>=n) {
        if (sprites[i].underimage && sprites[i].visible) {
            x = sprites[i].x - (sprites[i].underimage->width()/2);
            y = sprites[i].y - (sprites[i].underimage->height()/2);
            ian->setCompositionMode(QPainter::CompositionMode_Clear);
            ian->drawImage(x, y, *(sprites[i].underimage));
            ian->setCompositionMode(QPainter::CompositionMode_SourceOver);
            ian->drawImage(x, y, *(sprites[i].underimage));
        }
        i--;
    }
    ian->end();
    delete ian;
}

void Interpreter::spriteredraw(int n) {
    int x, y, i;
    // redraw all sprites n to nsprites-1
    i = n;
    while(i<nsprites) {
        if (sprites[i].image && sprites[i].visible) {
			QPainter *ian;
			ian = new QPainter(graphwin->image);
            if (sprites[i].r==0 && sprites[i].s==1) {
                if (sprites[i].underimage) {
                    delete sprites[i].underimage;
                    sprites[i].underimage = NULL;
                }
                if (sprites[i].image->width()>0 and sprites[i].image->height()>0) {
                    x = sprites[i].x - (sprites[i].image->width()/2);
                    y = sprites[i].y - (sprites[i].image->height()/2);
                    sprites[i].underimage = new QImage(graphwin->image->copy(x, y, sprites[i].image->width(), sprites[i].image->height()));
                    ian->drawImage(x, y, *(sprites[i].image));
                }
            } else {
                QTransform transform = QTransform().translate(0,0).rotateRadians(sprites[i].r).scale(sprites[i].s,sprites[i].s);;
                QImage rotated = sprites[i].image->transformed(transform);
                if (sprites[i].underimage) {
                    delete sprites[i].underimage;
                    sprites[i].underimage = NULL;
                }
                if (rotated.width()>0 and rotated.height()>0) {
                    x = sprites[i].x - (rotated.width()/2);
                    y = sprites[i].y - (rotated.height()/2);
                    sprites[i].underimage = new QImage(graphwin->image->copy(x, y, rotated.width(), rotated.height()));
                    ian->drawImage(x, y, rotated);
                }
            }
            ian->end();
            delete ian;
        }
        i++;
    }
}

bool Interpreter::spritecollide(int n1, int n2) {
    int top1, bottom1, left1, right1;
    int top2, bottom2, left2, right2;

    if (n1==n2) return true;											// cant collide with itself
    if (!sprites[n1].visible || !sprites[n2].visible) return false; 	// cant collide if invisible
    if (!sprites[n1].image || !sprites[n2].image) return false;			// cant collide if not initialized

    left1 = (int) (sprites[n1].x - (double) sprites[n1].image->width()*sprites[n1].s/2.0);
    left2 = (int) (sprites[n2].x - (double) sprites[n2].image->width()*sprites[n2].s/2.0);;
    right1 = left1 + sprites[n1].image->width()*sprites[n1].s;
    right2 = left2 + sprites[n2].image->width()*sprites[n2].s;
    top1 = (int) (sprites[n1].y - (double) sprites[n1].image->height()*sprites[n1].s/2.0);
    top2 = (int) (sprites[n2].y - (double) sprites[n2].image->height()*sprites[n2].s/2.0);
    bottom1 = top1 + sprites[n1].image->height()*sprites[n1].s;
    bottom2 = top2 + sprites[n2].image->height()*sprites[n2].s;

    if (bottom1<top2) return false;
    if (top1>bottom2) return false;
    if (right1<left2) return false;
    if (left1>right2) return false;
    return true;
}

int
Interpreter::compileProgram(char *code) {
    variables.clear();
    if (newWordCode() < 0) {
        return -1;
    }


    int result = basicParse(code);
    //
    // display warnings from compile and free the lexing file name string
    for(int i=0; i<numparsewarnings; i++) {
        QString msg = tr("COMPILE WARNING");
        if (strlen(parsewarningtablelexingfilename[i])!=0) {
            msg += tr(" in included file ") + QString(parsewarningtablelexingfilename[i]);
        } else {
            emit(goToLine(parsewarningtablelinenumber[i]));
        }
        msg += tr(" on line ") + QString::number(parsewarningtablelinenumber[i]) + tr(": ");
        switch(parsewarningtable[i]) {
            case COMPWARNING_MAXIMUMWARNINGS:
                msg += tr("The maximum number of compiler warnings have been displayed");
                break;
            case COMPWARNING_DEPRECATED_FORM:
                msg += tr("Statement format has been deprecated. It is recommended that you reauthor");
                break;

            default:
                msg += tr("Unknown compiler warning #") + QString::number(parsewarningtable[i]);
        }
        msg += tr(".\n");
        emit(outputReady(msg));
        //
        free(parsewarningtablelexingfilename[i]);
    }
    //
    // now display fatal error if there is one
    if (result != COMPERR_NONE)	{
        QString msg = tr("COMPILE ERROR");
        if (strlen(lexingfilename)!=0) {
            msg += tr(" in included file ") + QString(lexingfilename);
        } else {
            emit(goToLine(linenumber));
        }
        msg += tr(" on line ") + QString::number(linenumber) + tr(": ");
        switch(result) {
            case COMPERR_ASSIGNS2N:
                msg += tr("Error assigning a string to a numeric variable");
                break;
            case COMPERR_ASSIGNN2S:
                msg += tr("Error assigning a number to a string variable");
                break;
            case COMPERR_FUNCTIONGOTO:
                msg += tr("You may not define a label or use a GOTO or GOSUB statement in a FUNCTION/SUBROUTINE declaration");
                break;
            case COMPERR_GLOBALNOTHERE:
                msg += tr("You may not define GLOBAL variable(s) inside an IF, loop, TRY, CATCH, or FUNCTION/SUBROUTINE");
                break;
            case COMPERR_FUNCTIONNOTHERE:
                msg += tr("You may not define a FUNCTION/SUBROUTINE inside an IF, loop, TRY, CATCH, or other FUNCTION/SUBROUTINE");
                break;
            case COMPERR_ENDFUNCTION:
                msg += tr("END FUNCTION/SUBROUTINE without matching FUNCTION/SUBROUTINE");
                break;
            case COMPERR_FUNCTIONNOEND:
                msg += tr("FUNCTION/SUBROUTINE without matching END FUNCTION/SUBROUTINE statement");
                break;
            case COMPERR_FORNOEND:
                msg += tr("FOR without matching NEXT statement");
                break;
            case COMPERR_WHILENOEND:
                msg += tr("WHILE without matching END WHILE statement");
                break;
            case COMPERR_DONOEND:
                msg += tr("DO without matching UNTIL statement");
                break;
            case COMPERR_ELSENOEND:
                msg += tr("ELSE without matching END IF or END CASE statement");
                break;
            case COMPERR_IFNOEND:
                msg += tr("IF without matching END IF or ELSE statement");
                break;
            case COMPERR_UNTIL:
                msg += tr("UNTIL without matching DO");
                break;
            case COMPERR_ENDWHILE:
                msg += tr("END WHILE without matching WHILE");
                break;
            case COMPERR_ELSE:
                msg += tr("ELSE without matching IF");
                break;
            case COMPERR_ENDIF:
                msg += tr("END IF without matching IF");
                break;
            case COMPERR_NEXT:
                msg += tr("NEXT without matching FOR");
                break;
            case COMPERR_RETURNVALUE:
                msg += tr("RETURN with a value is only valid inside a FUNCTION");
                break;
            case COMPERR_RETURNTYPE:
                msg += tr("RETURN value type is not the same as FUNCTION");
                break;
            case COMPERR_CONTINUEDO:
                msg += tr("CONTINUE DO without matching DO");
                break;
            case COMPERR_CONTINUEFOR:
                msg += tr("CONTINUE DO without matching DO");
                break;
            case COMPERR_CONTINUEWHILE:
                msg += tr("CONTINUE WHILE without matching WHILE");
                break;
            case COMPERR_EXITDO:
                msg += tr("EXIT DO without matching DO");
                break;
            case COMPERR_EXITFOR:
                msg += tr("EXIT FOR without matching FOR");
                break;
            case COMPERR_EXITWHILE:
                msg += tr("EXIT WHILE without matching WHILE");
                break;
            case COMPERR_INCLUDEFILE:
                msg += tr("Unable to open INCLUDE file");
                break;
            case COMPERR_INCLUDEDEPTH:
                msg += tr("Maximum depth of INCLUDE files");
                break;
            case COMPERR_TRYNOEND:
                msg += tr("TRY without matching CATCH statement");
                break;
            case COMPERR_CATCH:
                msg += tr("CATCH whthout matching TRY statement");
                break;
            case COMPERR_CATCHNOEND:
                msg += tr("CATCH whthout matching ENDTRY statement");
                break;
            case COMPERR_ENDTRY:
                msg += tr("ENDTRY whthout matching CATCH statement");
                break;
            case COMPERR_NOTINTRYCATCH:
                msg += tr("You may not define an ONERROR or an OFFERROR in a TRY/CACTCH declaration");
                break;
            case COMPERR_ENDBEGINCASE:
                msg += tr("CASE whthout matching BEGIN CASE statement");
                break;
            case COMPERR_ENDENDCASEBEGIN:
                msg += tr("END CASE without matching BEGIN CASE statement");
                break;
            case COMPERR_ENDENDCASE:
                msg += tr("END CASE whthout matching CASE statement");
                break;
            case COMPERR_BEGINCASENOEND:
                msg += tr("BEGIN CASE whthout matching END CASE statement");
                break;
            case COMPERR_CASENOEND:
                msg += tr("CASE whthout next CASE or matching END CASE statement");
                break;

            default:
                if(column==0) {
                    msg += tr("Syntax error around beginning line");
                } else {
                    msg += tr("Syntax error around character ") + QString::number(column);
                }
        }
        msg += tr(".\n");
        emit(outputReady(msg));
        return -1;
    }

    // for debugging - dump the wordcode as hex
    //op = wordCode;
    //currentLine = 1;
    //while (op <= wordCode + wordOffset)
    //{
    //	emit(outputReady("off=" + QString::number(op-wordCode,16) + " w=" + QString::number(*op,16) + "\n"));
    //	op++;
    //}

    currentLine = 1;
    return 0;
}

void
Interpreter::initialize() {
    SETTINGS;

    op = wordCode;
    callstack = NULL;
    onerrorstack = NULL;
    forstack = NULL;
    fastgraphics = false;
    drawingpen = QPen(Qt::black);
    drawingbrush = QBrush(Qt::black, Qt::SolidPattern);
    status = R_RUNNING;
    once = true;
    currentLine = 1;
    currentIncludeFile = QString("");
    emit(mainWindowsResize(1, 300, 300));
    fontfamily = QString("");
    fontpoint = 0;
    fontweight = 0;
    nsprites = 0;
    printing = false;
    regexMinimal = false;
    runtimer.start();
    // clickclear mouse status
    graphwin->clickX = 0;
    graphwin->clickY = 0;
    graphwin->clickB = 0;
    // initialize files to NULL (closed)
    filehandle = (QIODevice**) malloc(NUMFILES * sizeof(QIODevice*));
    filehandletype = (int*) malloc(NUMFILES * sizeof(int));
    for (int t=0; t<NUMFILES; t++) {
        filehandle[t] = NULL;
        filehandletype[t] = 0;
    }
    // initialize network sockets to closed (-1)
    for (int t=0; t<NUMSOCKETS; t++) {
        netsockfd[t] = netSockClose(netsockfd[t]);
    }
    // initialize databse connections
    // by closing any that were previously open
    for (int t=0; t<NUMDBCONN; t++) {
        closeDatabase(t);
    }


    // set settings
    stack.settypeconverror(settings.value(SETTINGSTYPECONV, SETTINGSTYPECONVDEFAULT).toInt());
    stack.setdecimaldigits(settings.value(SETTINGSDECDIGS, SETTINGSDECDIGSDEFAULT).toInt());

}


void
Interpreter::cleanup() {
    // called by run() once the run is terminated
    //
    // Clean up stack
    stack.clear();
    // Clean up variables
    variables.clear();
    // Clean up sprites
    clearsprites();
    // Clean up, for frames, etc.
    if (wordCode) {
        free(wordCode);
        wordCode = NULL;
    }
    // close open files (set to NULL if closed)
    for (int t=0; t<NUMFILES; t++) {
        if (filehandle[t]) {
            filehandle[t]->close();
            filehandle[t] = NULL;
			filehandletype[t] = 0;
        }
    }
    // close open network connections (set to -1 of closed)
    for (int t=0; t<NUMSOCKETS; t++) {
        if (netsockfd[t]) netsockfd[t] = netSockClose(netsockfd[t]);
    }
    // close open database connections and record sets
    for (int t=0; t<NUMDBCONN; t++) {
        closeDatabase(t);
    }
    // close the currently open folder
    if(directorypointer != NULL) {
        closedir(directorypointer);
        directorypointer = NULL;
    }
    // close a print document if it is open
    if (printing) {
        printing = false;
        printdocumentpainter->end();
        delete printdocumentpainter;
    }
}

void Interpreter::closeDatabase(int t) {
    // cleanup database and all of its sets
    QString dbconnection = "DBCONNECTION" + QString::number(t);
    QSqlDatabase db = QSqlDatabase::database(dbconnection);
    if (db.isValid()) {
        for (int u=0; u<NUMDBSET; u++) {
            if (dbSet[t][u]) {
                dbSet[t][u]->clear();
                delete dbSet[t][u];
                dbSet[t][u] = NULL;
            }
        }
        db.close();
        QSqlDatabase::removeDatabase(dbconnection);
    }
}

void
Interpreter::runPaused() {
    if (status == R_PAUSED) {
        status = oldstatus;
    } else {
        oldstatus = status;
        status = R_PAUSED;
    }
}

void
Interpreter::runResumed() {
    runPaused();
}

void
Interpreter::runHalted() {
    status = R_STOPPED;
}


void
Interpreter::run() {
    errornum = ERROR_NONE;
    errormessage = "";
    lasterrornum = ERROR_NONE;
    lasterrormessage = "";
    lasterrorline = 0;
    onerrorstack = NULL;
	if (debugMode!=0) {			// highlight first line correctly in debugging mode
		emit(seekLine(2));
		emit(seekLine(1));
	}
    while (status != R_STOPPED && execByteCode() >= 0) {} //continue
    status = R_STOPPED;
    cleanup(); // cleanup the variables, databases, files, stack and everything
    emit(stopRun());
}


void
Interpreter::inputEntered(QString text) {
    if (status!=R_STOPPED) {
        inputString = text;
        status = R_INPUTREADY;
    }
}


void
Interpreter::waitForGraphics() {
    // wait for graphics operation to complete
    mymutex->lock();
    emit(goutputReady());
    waitCond->wait(mymutex);
    mymutex->unlock();
}


int
Interpreter::execByteCode() {
	int opcode;
	SETTINGS;

    if (status == R_INPUTREADY) {
        stack.pushstring(inputString);
        status = R_RUNNING;
        return 0;
    } else if (status == R_PAUSED) {
        sleep(1);
        return 0;
    } else if (status == R_INPUT) {
        return 0;
    }

    // if errnum is set then handle the last thrown error
    if (stack.error()!=ERROR_NONE) {
        errornum = stack.error();
        stack.clearerror();
    }
    if (errornum!=ERROR_NONE) {
        lasterrornum = errornum;
        lasterrormessage = errormessage;
        lasterrorline = currentLine;
        errornum = ERROR_NONE;
        errormessage = "";
        if(onerrorstack && lasterrornum > 0) {
            // progess call to subroutine for error handling
            // or jump to the catch label
            if (onerrorstack->onerrorgosub) {
                frame *temp = new frame;
                temp->returnAddr = op;
                temp->next = callstack;
                callstack = temp;
            }
            op = wordCode + onerrorstack->onerroraddress;
            return 0;
        } else {
            // no error handler defined or FATAL error - display message
            printError(lasterrornum, lasterrormessage);
            // if error number less than the start of warnings then
            // highlight the current line AND die
            if (lasterrornum<WARNING_START) {
                if (currentIncludeFile!="") emit(goToLine(currentLine));
                return -1;
            }
        }
    }

    //emit(outputReady("off=" + QString::number(op-wordCode,16) + " op=" + QString::number(*op,16) + " stack=" + stack.debug() + "\n"));


    opcode = *op;
    op++;

    switch (optype(opcode)) {

        case OPTYPE_VARIABLE: {
            //
            // OPCODES with an variable number following in the wordCcode go in this switch
            // int i is the symbol/variable number
            //
            int i = *op;
            op++;

            switch(opcode) {

                case OP_FOR: {
                    
                    forframe *temp = new forframe;
                    double step = stack.popfloat();
                    double endnum = stack.popfloat();
                    double startnum = stack.popfloat();

                    temp->next = forstack;
                    temp->variable = i;
                    temp->recurselevel = variables.getrecurse();

                    variables.set(i, T_FLOAT, startnum, NULL);

                    if(debugMode != 0) {
                        emit(varAssignment(variables.getrecurse(),QString(symtable[i]), QString::number(variables.get(i)->getfloat()), -1, -1));
                    }

                    temp->endNum = endnum;
                    temp->step = step;
                    temp->returnAddr = op;
                    forstack = temp;
                    
                    if (temp->step > 0 && variables.get(i)->getfloat() > temp->endNum) {
                        errornum = ERROR_FOR1;
                    } else if (temp->step < 0 && variables.get(i)->getfloat() < temp->endNum) {
                        errornum = ERROR_FOR2;
                    }
                }
                break;

                case OP_NEXT: {
                    forframe *temp = forstack;
                    
                    while (temp && temp->variable != (unsigned int ) i) {
                        temp = temp->next;
                    }

                    if (!temp) {
                        errornum = ERROR_NEXTNOFOR;
                    } else {

                        double val = variables.get(i)->getfloat();
                        val += temp->step;
                        variables.set(i, T_FLOAT, val, NULL);

                        if(debugMode != 0) {
                            emit(varAssignment(variables.getrecurse(),QString(symtable[i]), QString::number(variables.get(i)->getfloat()), -1, -1));
                        }

                        if (temp->step > 0 && stack.compareFloats(val, temp->endNum)!=1) {
                            op = temp->returnAddr;
                        } else if (temp->step < 0 && stack.compareFloats(val, temp->endNum)!=-1) {
                            op = temp->returnAddr;
                        } else {
                            forstack = temp->next;
                            delete temp;
                        }
                    }

                }
                break;
                case OP_DIM:
                case OP_REDIM:
                case OP_DIMSTR:
                case OP_REDIMSTR: {
                    int ydim = stack.popint();
                    int xdim = stack.popint();
                    if (ydim<=0) ydim=1; // need to dimension as 1d
                    if (opcode==OP_DIM || opcode == OP_REDIM) {
                        variables.arraydim(T_ARRAY, i, xdim, ydim, opcode == OP_REDIM);
                    } else {
                        variables.arraydim(T_STRARRAY, i, xdim, ydim, opcode == OP_REDIMSTR);
                    }
                    if (variables.error()==ERROR_NONE) {
                        if(debugMode != 0) {
                            emit(varAssignment(variables.getrecurse(),QString(symtable[i]), NULL, xdim, ydim));
                        }
                    } else {
                        errornum = variables.error();
                        errorvarnum = variables.errorvarnum();
                    }
                }
                break;

                case OP_ALEN:
                case OP_ALENX:
                case OP_ALENY: {
                    // return array lengths
                    switch(opcode) {
                        case OP_ALEN:
                            stack.pushint(variables.arraysize(i));
                            break;
                        case OP_ALENX:
                            stack.pushint(variables.arraysizex(i));
                            break;
                        case OP_ALENY:
                            stack.pushint(variables.arraysizey(i));
                            break;
                    }
                    if (variables.error()!=ERROR_NONE) {
                        errornum = variables.error();
                        errorvarnum = variables.errorvarnum();
                        stack.pushint(0);	// unknown
                    }
                }
                break;

                case OP_EXPLODE:
                case OP_EXPLODESTR:
                case OP_EXPLODEX:
                case OP_EXPLODEXSTR: {
                    // unicode safe explode a string to an array function
                    bool ok;
                   	Qt::CaseSensitivity casesens = Qt::CaseSensitive;

                    if (opcode!=OP_EXPLODEX && opcode!=OP_EXPLODEXSTR ) {
						// 0 sensitive (default) - opposite of QT
						casesens = (stack.popfloat()==0?Qt::CaseSensitive:Qt::CaseInsensitive);
                    }
                    QString qneedle = stack.popstring();
                    QString qhaystack = stack.popstring();

                    QStringList list;
                    if(opcode==OP_EXPLODE || opcode==OP_EXPLODESTR) {
                        list = qhaystack.split(qneedle, QString::KeepEmptyParts , casesens);
                    } else {
						QRegExp expr = QRegExp(qneedle);
						expr.setMinimal(regexMinimal);
                        list = qhaystack.split(expr, QString::KeepEmptyParts);
                    }

                    if(opcode==OP_EXPLODESTR || opcode==OP_EXPLODEXSTR) {
                        // create a string array
                        if (variables.arraysize(i)!=list.size()) variables.arraydim(T_STRARRAY, i, list.size(), 1, false);
                    } else {
                        // create a numeric array
                        if (variables.arraysize(i)!=list.size()) variables.arraydim(T_ARRAY, i, list.size(), 1, false);
                    }
                    if (variables.error()==ERROR_NONE) {
                        if(debugMode != 0) {
                            emit(varAssignment(variables.getrecurse(),QString(symtable[i]), NULL, list.size(), 1));
                        }

                        for(int x=0; x<list.size(); x++) {
                            if(opcode==OP_EXPLODESTR || opcode==OP_EXPLODEXSTR) {
                                // fill the string array
                                variables.arrayset(i, x, 0, T_STRING, 0, list.at(x));
                                if (variables.error()==ERROR_NONE) {
                                    if(debugMode != 0) {
                                        emit(varAssignment(variables.getrecurse(),QString(symtable[i]), variables.arrayget(i, x, 0)->getstring(), x, 0));
                                    }
                                } else {
                                    errornum = variables.error();
                                    errorvarnum = variables.errorvarnum();
                                }
                            } else {
                                // fill the numeric array
                                variables.arrayset(i, x, 0, T_FLOAT, list.at(x).toDouble(&ok), NULL);
                                if (variables.error()==ERROR_NONE) {
                                    if(debugMode != 0) {
                                        emit(varAssignment(variables.getrecurse(),QString(symtable[i]), QString::number(variables.arrayget(i, x, 0)->getfloat()), x, 0));
                                    }
                                } else {
                                    errornum = variables.error();
                                    errorvarnum = variables.errorvarnum();
                                }
                            }
                        }
                    } else {
                        errornum = variables.error();
                        errorvarnum = variables.errorvarnum();
                    }
                }
                break;

                case OP_IMPLODE: {

                    QString qdelim = stack.popstring();

                    QString stuff = "";

                    if (variables.get(i)->type == T_STRARRAY || variables.get(i)->type == T_ARRAY) {
                        int kount = variables.arraysize(i);
                        for(int n=0; n<kount; n++) {
                            if (n>0) stuff.append(qdelim);
                            if (variables.get(i)->type == T_STRARRAY) {
                                stuff.append(variables.arrayget(i, n, 0)->getstring());
                            } else {
                                stack.pushfloat(variables.arrayget(i, n, 0)->getfloat());
                                stuff.append(stack.popstring());
                            }
                        }
                    } else {
                        errornum = ERROR_NOTARRAY;
                        errorvarnum = i;
                    }
                    stack.pushstring(stuff);
                }
                break;

                case OP_GLOBAL: {
                    // make a variable number a global variable
                    variables.makeglobal(i);
                }
                break;

                case OP_ARRAYASSIGN: {
					// assign a float value to an array element
					// assumes that arrays are always two dimensional (if 1d then y=1)
                    DataElement *e = stack.popelement();
                    int yindex = stack.popint();
                    int xindex = stack.popint();

                    variables.arrayset(i, xindex, yindex, e->type, e->floatval, e->stringval);
                    delete(e);
                    
                    if (variables.error()==ERROR_NONE) {
                        if(debugMode != 0) {
							DataElement *v = variables.arrayget(i, xindex, yindex);
							emit(varAssignment(variables.getrecurse(),QString(symtable[i]), v->getstring(), xindex, yindex));
						}
                    } else {
                        errornum = variables.error();
                        errorvarnum = variables.errorvarnum();
                    }
                }
                break;


                case OP_DEREF: {
					// get a value from an array and push it to the stack
					// assumes that arrays are always two dimensional (if 1d then y=1)
					int yindex = stack.popint();
					int xindex = stack.popint();
					Variable *v = variables.get(i);
					
					if (v->type == T_ARRAY || v->type == T_STRARRAY) {
						stack.pushelement(variables.arrayget(i, xindex, yindex));
					} else {
						errornum = ERROR_NOTARRAY;
						errorvarnum = i;
						stack.pushint(0);
					}
				}
				break;

                case OP_PUSHVAR: {
					Variable *v = variables.get(i);
					if (v) {
						if (v->type == T_ARRAY || v->type == T_STRARRAY) {
							errornum = ERROR_ARRAYINDEXMISSING;
							errorvarnum = i;
							stack.pushint(0);
						} else {
							stack.pushelement(v);
						}
					} else {
                        errornum = ERROR_NOSUCHVARIABLE;
                        errorvarnum = i;
                        stack.pushint(0);
                    }
                }
                break;

                case OP_PUSHVARREF: {
					Variable *v = variables.get(i);
					if (v->type==T_STRING || v->type==T_STRARRAY) {
						stack.pushelement(T_VARREFSTR, i, NULL);
					} else {
						stack.pushelement(T_VARREF, i, NULL);
					}
				}
				break;

                case OP_ASSIGN: {
                    DataElement *e = stack.popelement();
                    variables.set(i, e->type, e->floatval, e->stringval);
                    delete(e);

                    if(debugMode != 0) {
						Variable *v = variables.get(i);
						emit(varAssignment(variables.getrecurse(),QString(symtable[i]), v->getstring(), -1, -1));
                    }
                }
                break;

                case OP_VARREFASSIGN: {
                    // assign a numeric variable reference
                    int type = stack.peekType();
                    int value = stack.popint();
                    if (type==T_VARREF) {
                        variables.set(i, T_VARREF, value, NULL);
                    } else {
                        if (type==T_VARREFSTR) {
                            errornum = ERROR_BYREFTYPE;
                        } else {
                            errornum = ERROR_BYREF;
                        }
                    }
                }
                break;

                case OP_VARREFSTRASSIGN: {
                    // assign a string variable reference
                    int type = stack.peekType();
                    int value = stack.popint();
                    if (type==T_VARREFSTR) {
                        variables.set(i, T_VARREFSTR, value, NULL);
                    } else {
                        if (type==T_VARREF) {
                            errornum = ERROR_BYREFTYPE;
                        } else {
                            errornum = ERROR_BYREF;
                        }
                    }
                }
                break;

                case OP_ARRAY2STACK: {
                    // Push all of the elements of an array to the stack and then push the length to the stack
                    // expects one integer - variable number

                    Variable *v = variables.get(i);
                    if (v->type == T_ARRAY || v->type == T_STRARRAY) {
                        int n = v->arr->size;
                        for (int j = 0; j < n; j++) {
							DataElement *av = variables.arrayget(i, j, 0);
							stack.pushelement(av);
                        }
                        stack.pushint(n);
                    } else {
                        errornum = ERROR_POLYARRAY;
                        stack.pushint(0);
                    }
                }
                break;


            }
        }
        break;

        case OPTYPE_INT: {
            //
            // OPCODES with an integer following in the wordCcode go in this switch
            // int i is the number extracted from the wordCode
            //
            int i = *op;  // integer for opcodes of this type
            op++;
            switch(opcode) {
				
				case OP_CURRLINE: {
					// opcode currentline is compound and includes level and line number
					int includelevel = i / 0x1000000;
					currentLine = i % 0x1000000;
					if (debugMode != 0) {
						if (includelevel==0) {
							// do debug for the main program not included parts
							// edit needs to eventually have tabs for includes and tracing and debugging
							// would go three dimensional - but not right now
							emit(seekLine(currentLine));
							if ((debugMode==1) || (debugMode==2 && debugBreakPoints->contains(currentLine-1))) {
								// wait for button if we are stepping or if we are at a break point
								mydebugmutex->lock();
								waitDebugCond->wait(mydebugmutex);
								mydebugmutex->unlock();
							} else {
								// when debugging to breakpoint slow execution down so that the
								// trace on the screen keeps caught up
								sleeper->sleepMS(settings.value(SETTINGSDEBUGSPEED, SETTINGSDEBUGSPEEDDEFAULT).toInt());
							}
						}
					}
				}
				break;

				case OP_PUSHINT: {
					stack.pushint(i);
				}
				break;

                case OP_ARRAYLISTASSIGN: {

                    int items = stack.popint();
                    
                    if (variables.arraysize(i)!=items) variables.arraydim(T_ARRAY, i, items, 1, false);
                    if(errornum==ERROR_NONE) {
                        if(debugMode != 0 && errornum==ERROR_NONE) {
                            emit(varAssignment(variables.getrecurse(),QString(symtable[i]), NULL, items, 1));
                        }

                        for (int index = items - 1; index >= 0 && errornum==ERROR_NONE; index--) {
                            double one = stack.popfloat();
							variables.arrayset(i, index, 0, T_FLOAT, one, NULL);
                            if (variables.error()==ERROR_NONE) {
                                if(debugMode != 0) {
                                    emit(varAssignment(variables.getrecurse(),QString(symtable[i]), QString::number(variables.arrayget(i, index, 0)->getfloat()), index, 0));
                                }
                            } else {
                                errornum = variables.error();
                                errorvarnum = variables.errorvarnum();
                                break;  // exit for
                            }
                        }
                    } else {
                        errornum = variables.error();
                        errorvarnum = variables.errorvarnum();
                    }

                }
                break;

                case OP_STRARRAYLISTASSIGN: {

                    int items = stack.popint();

                    if (variables.arraysize(i)!=items) variables.arraydim(T_STRARRAY, i, items, 1, false);

                    if(errornum==ERROR_NONE) {
                        if(debugMode != 0) {
                            emit(varAssignment(variables.getrecurse(),QString(symtable[i]), NULL, items, 1));
                        }

                        for (int index = items - 1; index >= 0 && errornum==ERROR_NONE; index--) {
                            QString q = stack.popstring();
                            variables.arrayset(i, index, 0, T_STRING, 0, q);
                            if (variables.error()==ERROR_NONE) {
                                if(debugMode != 0) {
                                     emit(varAssignment(variables.getrecurse(),QString(symtable[i]), variables.arrayget(i, index, 0)->getstring(), index, 0));
                                }
                            } else {
                                errornum = variables.error();
                                errorvarnum = variables.errorvarnum();
                                break;  // exit for
                            }
                        }
                    } else {
                        errornum = variables.error();
                        errorvarnum = variables.errorvarnum();
                    }
                }
                break;

            }
        }
        break;

        case OPTYPE_FLOAT: {
            //
            // OPCODES with an double following in the wordCcode go in this switch
            // double d is the number extracted from the wordCode
            //
            double *d = (double *) op;
            op += bytesToFullWords(sizeof(double));
            switch(opcode) {

                case OP_PUSHFLOAT: {
                    stack.pushfloat(*d);
                }
                break;

            }
        }
        break;

        case OPTYPE_STRING: {
            //
            // OPCODES with a string in the wordCcode go in this switch
            // int len will be set to the length and
            // QString s will be the string extracted from the wordCode
            //
            int len = strlen((char *) op) + 1;
            QString s = QString::fromUtf8((char *) op);
            op += bytesToFullWords(len);

            switch(opcode) {
                case OP_PUSHSTRING: {
                    // push string from compiled bytecode
                    stack.pushstring(s);
                }
                break;

                case OP_INCLUDEFILE: {
                    // current include file name save for runtime error codes
                    // set to "" in the byte code when returning to main program
                    currentIncludeFile = s;
                }
                break;
            }
        }
        break;

        case OPTYPE_LABEL: {
            //
            // OPCODES with an integer wordCode label/symbol number go here
            // the symbol is looked up in the labeltable and changed to an array location
            // and stored in i BEFORE the OPCODES are executed
            //
            // int i is the new execution location
            //
            int i = *op;    // label/symbol number
            op++;

            // lookup address for the label
            if (labeltable[i] >=0) {
                i = labeltable[i];
            } else {
                errorvarnum = i;
                printError(ERROR_NOSUCHLABEL,"");
                return -1;
            }

            switch(opcode) {
                case OP_GOTO: {
                    op = wordCode + i;
                }
                break;

                case OP_BRANCH: {
                    // goto if true
                    int val = stack.popint();

                    if (val == 0) { // go to next line on false, otherwise execute rest of line.
                        op = wordCode + i;
                    }
                }
                break;

                case OP_GOSUB: {
                    // setup return
                    frame *temp = new frame;
                    temp->returnAddr = op;
                    temp->next = callstack;
                    callstack = temp;
                    // do jump
                    op = wordCode + i;
                }
                break;

                case OP_ONERRORGOSUB: {
                    // setup onerror frame and put on top of onerrorstack
                    onerrorframe *temp = new onerrorframe;
                    temp->onerroraddress = i;
                    temp->onerrorgosub = true;
                    temp->next = onerrorstack;
                    onerrorstack = temp;
                }
                break;

                case OP_ONERRORCATCH: {
                    // setup onerror frame and put on top of onerrorstack
                    onerrorframe *temp = new onerrorframe;
                    temp->onerroraddress = i;
                    temp->onerrorgosub = false;
                    temp->next = onerrorstack;
                    onerrorstack = temp;
                }
                break;

            }
        }
        break;

        case OPTYPE_NONE: {
            switch(opcode) {
                case OP_NOP:
                    break;

                case OP_END: {
                    return -1;
                }
                break;


                case OP_RETURN: {
                    frame *temp = callstack;
                    if (temp) {
                        op = temp->returnAddr;
                        callstack = temp->next;
                    } else {
                        return -1;
                    }
                    delete temp;
                }
                break;



                case OP_OPEN: {
                    int type = stack.popint();	// 0 text 1 binary
                    QString name = stack.popstring();
                    int fn = stack.popint();

                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                    } else {
                        // close file number if open
                        if (filehandle[fn] != NULL) {
                            filehandle[fn]->close();
                            filehandle[fn] = NULL;
                        }
                        // create filehandle
                        filehandle[fn] = new QFile(name);
                        if (filehandle[fn] == NULL) {
                            errornum = ERROR_FILEOPEN;
                        } else {
					        filehandletype[fn] = type;
                            if (type==0) {
                                // text file (type 0)
                                if (!filehandle[fn]->open(QIODevice::ReadWrite | QIODevice::Text)) {
                                    errornum = ERROR_FILEOPEN;
                                }
                            } else {
                                // binary file (type 1)
                                if (!filehandle[fn]->open(QIODevice::ReadWrite)) {
                                    errornum = ERROR_FILEOPEN;
                                }
                            }
                        }
                    }
                }
                break;

               case OP_OPENSERIAL: {
                    int flow = stack.popint();
                    int parity = stack.popint();
                    int stop = stack.popint();
                    int data = stack.popint();
                    int baud = stack.popint();
                    QString name = stack.popstring();
                    int fn = stack.popint();
#ifdef ANDROID
                    errornum = ERROR_NOTIMPLEMENTED;
# else
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                    } else {
                        // close file number if open
                        if (filehandle[fn] != NULL) {
                            filehandle[fn]->close();
                            filehandle[fn] = NULL;
                        }
                        // create file filehandle
                        QSerialPort *p = new QSerialPort();
                        if (p == NULL) {
                            errornum = ERROR_FILEOPEN;
                        } else {
							p->setPortName(name);
							p->setReadBufferSize(SERIALREADBUFFERSIZE);
							if (errornum==ERROR_NONE) {
								if (!p->open(QIODevice::ReadWrite)) {
									errornum = ERROR_FILEOPEN;
								} else {
									// successful open
									filehandle[fn] = p;
							        filehandletype[fn] = 2;
							        // set the parameters
									if (!p->setBaudRate(baud)) errornum = ERROR_SERIALPARAMETER;
									switch (data) {
										case 5:
											if(!p->setDataBits(QSerialPort::Data5)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 6:
											if(!p->setDataBits(QSerialPort::Data6)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 7:
											if(!p->setDataBits(QSerialPort::Data7)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 8:
											if(!p->setDataBits(QSerialPort::Data8)) errornum = ERROR_SERIALPARAMETER;
											break;
										default: errornum = ERROR_SERIALPARAMETER;
									}
									switch (stop) {
										case 1:
											if(!p->setStopBits(QSerialPort::OneStop)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 2:
											if(!p->setStopBits(QSerialPort::TwoStop)) errornum = ERROR_SERIALPARAMETER;
											break;
										default: errornum = ERROR_SERIALPARAMETER;
									}
									switch (parity) {
										case 0:
											if(!p->setParity(QSerialPort::NoParity)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 1:
											if(!p->setParity(QSerialPort::OddParity)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 2:
											if(!p->setParity(QSerialPort::EvenParity)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 3:
											if(!p->setParity(QSerialPort::SpaceParity)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 4:
											if(!p->setParity(QSerialPort::MarkParity)) errornum = ERROR_SERIALPARAMETER;
											break;
										default:
											errornum = ERROR_SERIALPARAMETER;
									}
									switch (flow) {
										case 0:
											if(!p->setFlowControl(QSerialPort::NoFlowControl)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 1: 
											if(!p->setFlowControl(QSerialPort::HardwareControl)) errornum = ERROR_SERIALPARAMETER;
											break;
										case 2: 
											if(!p->setFlowControl(QSerialPort::SoftwareControl)) errornum = ERROR_SERIALPARAMETER;
											break;
										default:
											errornum = ERROR_SERIALPARAMETER;
									}
								}
							}
                        }
                    }
#endif
                }
                break;


                case OP_READ: {
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                        stack.pushint(0);
                    } else {
                        char c = ' ';
                        if (filehandle[fn] == NULL) {
                            errornum = ERROR_FILENOTOPEN;
                            stack.pushint(0);
                        } else {
                            int maxsize = 256;
                            char * strarray = (char *) malloc(maxsize);
                            memset(strarray, 0, maxsize);
                            // get the first char - Remove leading whitespace
                            do {
								filehandle[fn]->waitForReadyRead(FILEREADTIMEOUT);
								if (!filehandle[fn]->getChar(&c)) {
                                    stack.pushstring(QString::fromUtf8(strarray));
                                    free(strarray);
                                    return 0;
                                }
							} while (c == ' ' || c == '\t' || c == '\n');
                            // read token - we already have the first char
                            int offset = 0;
							// get next letter until we crap-out or get white space
							do {
								strarray[offset] = c;
                                offset++;
                                // grow the buffer if we need to
                                if (offset+2 >= maxsize) {
                                    maxsize *= 2;
                                    strarray = (char *) realloc(strarray, maxsize);
                                    memset(strarray + offset, 0, maxsize - offset);
                                }
								// get next char
								filehandle[fn]->waitForReadyRead(FILEREADTIMEOUT);
								if (!filehandle[fn]->getChar(&c)) {
									// no more to get - finish the string and push to stack
									strarray[offset] = 0;
                                    stack.pushstring(QString::fromUtf8(strarray));
                                    free(strarray);
                                    return 0;  //nextop
                                }
                            } while (c != ' ' && c != '\t' && c != '\n');
							// found a delimiter - finish the string and push to stack
							strarray[offset] = 0;
                            stack.pushstring(QString::fromUtf8(strarray));
                            free(strarray);
                            return 0;	// nextop
                        }
                    }
                }
                break;


                case OP_READLINE: {
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                        stack.pushint(0);
                    } else {

                        if (filehandle[fn] == NULL) {
                            errornum = ERROR_FILENOTOPEN;
                            stack.pushint(0);
                        } else {
                            //read entire line
							filehandle[fn]->waitForReadyRead(FILEREADTIMEOUT);
                            stack.pushstring(filehandle[fn]->readLine());
                        }
                    }
                }
                break;

                case OP_READBYTE: {
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                        stack.pushint(0);
                    } else {
                        char c = ' ';
                        filehandle[fn]->waitForReadyRead(FILEREADTIMEOUT);
                        if (filehandle[fn]->getChar(&c)) {
                            stack.pushint((int) (unsigned char) c);
                        } else {
                            stack.pushint((int) -1);
                        }
                    }
                }
                break;

                case OP_EOF: {
                    //return true to eof if error is returned
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                        stack.pushint(1);
                    } else {
                        if (filehandle[fn] == NULL) {
                            errornum = ERROR_FILENOTOPEN;
                            stack.pushint(1);
                        } else {
                            switch (filehandletype[fn]) {
								case 0:
								case 1:
									// normal file eof
									if (filehandle[fn]->atEnd()) {
										stack.pushint(1);
									} else {
										stack.pushint(0);
									}
									break;
								case 2:
									// serial
									QCoreApplication::processEvents();
									if (filehandle[fn]->bytesAvailable()==0) {
										stack.pushint(1);
									} else {
										stack.pushint(0);
									}
									break;
							}
                        }
                    }
                }
                break;


                case OP_WRITE:
                case OP_WRITELINE: {
                    QString temp = stack.popstring();
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                    } else {
                        int error = 0;
                        if (filehandle[fn] == NULL) {
                            errornum = ERROR_FILENOTOPEN;
                        } else {
                            if (filehandle[fn]->isSequential()) {
								// sequential file (serialPort)
								error = filehandle[fn]->write(temp.toUtf8().data());
								if (opcode == OP_WRITELINE) {
									error = filehandle[fn]->putChar('\n');
								}
								filehandle[fn]->waitForBytesWritten(FILEWRITETIMEOUT);
							} else {
								quint64 oldPos;
								// advance to the end but save where we are so we can go back
								oldPos = filehandle[fn]->pos();
								filehandle[fn]->seek(filehandle[fn]->size());
								error = filehandle[fn]->write(temp.toUtf8().data());
								if (opcode == OP_WRITELINE) {
									error = filehandle[fn]->putChar('\n');
								}
								// after append go back to the original location
								filehandle[fn]->seek(oldPos);
							}
                        }
                        if (error == -1) {
                            errornum = ERROR_FILEWRITE;
                        }
                    }
                }
                break;

                case OP_WRITEBYTE: {
                    int n = stack.popint();
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                    } else {
                        int error = 0;
                        if (filehandle[fn] == NULL) {
                            errornum = ERROR_FILENOTOPEN;
                        } else {
                            error = filehandle[fn]->putChar((unsigned char) n);
							if (filehandle[fn]->isSequential()) filehandle[fn]->waitForBytesWritten(FILEWRITETIMEOUT);
							if (error == -1) {
                                errornum = ERROR_FILEWRITE;
                            }
                        }
                    }
                }
                break;


                case OP_CLOSE: {
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                    } else {
                        if (filehandle[fn] == NULL) {
                            errornum = ERROR_FILENOTOPEN;
                        } else {
                            filehandle[fn]->close();
                            filehandle[fn] = NULL;
                        }
                    }
                }
                break;


                case OP_RESET: {
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                    } else {
                        if (filehandle[fn] == NULL) {
                            errornum = ERROR_FILENOTOPEN;
                        } else {
                            if (filehandle[fn]->isSequential()) {
								errornum = ERROR_FILEOPERATION;
 							} else {
								switch (filehandletype[fn]) {
									case 0:
										// text mode file (close and reopen)
										filehandle[fn]->close();
										if (!filehandle[fn]->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
											errornum = ERROR_FILERESET;
										}
										break;
									case 1:
										// binary mode file
										filehandle[fn]->close();
										if (!filehandle[fn]->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
											errornum = ERROR_FILERESET;
										}
										break;
									case 2:
										// serial pot
										errornum = ERROR_FILEOPERATION;
										break;
								}
                            }
                        }
                    }
                }
                break;

                case OP_SIZE: {
                    // push the current open file size on the stack
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMFILES) {
                        errornum = ERROR_FILENUMBER;
                        stack.pushint(0);
                    } else {
						int size = 0;
						if (filehandle[fn] == NULL) {
							errornum = ERROR_FILENOTOPEN;
						} else {
							switch(filehandletype[fn]) {
								case 0:
								case 1:
									// normal file
									size = filehandle[fn]->size();
									break;
								case 2:
									// serial
									QCoreApplication::processEvents();
									size = filehandle[fn]->bytesAvailable();
									break;
							}
						}
						stack.pushint(size);
                    }
                }
                break;

                case OP_EXISTS: {
                    // push a 1 if file exists else zero

                    QString filename = stack.popstring();
                    if (QFile::exists(filename)) {
                        stack.pushint(1);
                    } else {
                        stack.pushint(0);
                    }
                }
                break;

				case OP_SEEK: {
					// move file pointer to a specific loaction in file
					long pos = stack.popint();
					int fn = stack.popint();
					if (fn<0||fn>=NUMFILES) {
						errornum = ERROR_FILENUMBER;
					} else {
						if (filehandle[fn] == NULL) {
							errornum = ERROR_FILENOTOPEN;
						} else {
							switch(filehandletype[fn]) {
								case 0:
								case 1:
									// normal file
									filehandle[fn]->seek(pos);
									break;
								case 2:
									// serial
									errornum = ERROR_FILEOPERATION;
									break;
							}
						}
					}
				}
				break;

				case OP_INT: {
					// bigger integer safe (trim floating point off of a float)
					double val = stack.popfloat();
					double intpart;
					val = modf(val + (val>0?BASIC256EPSILON:-BASIC256EPSILON), &intpart);
					stack.pushfloat(intpart);
				}
				break;


				case OP_FLOAT: {
					double val = stack.popfloat();
					stack.pushfloat(val);
				}
				break;

				case OP_STRING: {
					stack.pushstring(stack.popstring());
				}
				break;

				case OP_RAND: {
					double r = 1.0;
					double ra;
					double rx;
					if (once) {
						int ms = 999 + QTime::currentTime().msec();
						once = false;
						srand(time(NULL) * ms);
					}
					while(r == 1.0) {
						ra = (double) rand() * (double) RAND_MAX + (double) rand();
						rx = (double) RAND_MAX * (double) RAND_MAX + (double) RAND_MAX + 1.0;
						r = ra/rx;
					}
					stack.pushfloat(r);
				}
				break;

				case OP_PAUSE: {
					double val = stack.popfloat();
					if (val > 0) {
						sleeper->sleepSeconds(val);
					}
				}
				break;

				case OP_LENGTH: {
					// unicode length
					stack.pushint(stack.popstring().length());
				}
				break;


				case OP_MID: {
					// unicode safe mid string
					int len = stack.popint();
					int pos = stack.popint();
					QString qtemp = stack.popstring();

					if ((len < 0)) {
						errornum = ERROR_STRNEGLEN;
						stack.pushint(0);
					} else {
						if ((pos < 1)) {
							errornum = ERROR_STRSTART;
							stack.pushint(0);
						} else {
							if ((pos < 1) || (pos > (int) qtemp.length())) {
								stack.pushstring(QString(""));
							} else {
								stack.pushstring(qtemp.mid(pos-1,len));
							}
						}
					}
				}
				break;

               case OP_MIDX: {
					// regex section string (MID regeX)
					int start = stack.popint();
					QRegExp expr = QRegExp(stack.popstring());
					expr.setMinimal(regexMinimal);
					QString qtemp = stack.popstring();


					if(start < 1) {
						errornum = ERROR_STRSTART;
						stack.pushstring(QString(""));
					} else {
						int pos;
						if (start==1) {
							pos = expr.indexIn(qtemp);
						} else {
							pos = expr.indexIn(qtemp.mid(start-1));
						}
						if (pos==-1) {
							// did not find it - return ""
							stack.pushstring(QString(""));
						} else {
							QStringList stuff = expr.capturedTexts();
							stack.pushstring(stuff[0]);
						}
					}
				}
				break;


                case OP_LEFT:
                case OP_RIGHT: {
                    // unicode safe left/right string
                    int len = stack.popint();
                    QString qtemp = stack.popstring();

                    if (len < 0) {
                        errornum = ERROR_STRNEGLEN;
                        stack.pushint(0);
                    } else {
                        switch(opcode) {
                            case OP_LEFT:
                                stack.pushstring(qtemp.left(len));
                                break;
                            case OP_RIGHT:
                                stack.pushstring(qtemp.right(len));
                                break;
                        }
                    }
                }
                break;


                case OP_UPPER: {
                    stack.pushstring(stack.popstring().toUpper());
                }
                break;

                case OP_LOWER: {
                    stack.pushstring(stack.popstring().toLower());
                }
                break;

                case OP_ASC: {
                    // unicode character sequence - return 16 bit number representing character
                    QString qs = stack.popstring();
                    stack.pushint((int) qs[0].unicode());
                }
                break;


                case OP_CHR: {
                    // convert a single unicode character sequence to string in utf8
                    int code = stack.popint();
                    QChar temp[2];
                    temp[0] = (QChar) code;
                    temp[1] = (QChar) 0;
                    QString qs = QString(temp,1);
                    stack.pushstring(qs);
                    qs = QString::null;
                }
                break;


                case OP_INSTR: {
                    // unicode safe instr function

                   // 0 sensitive (default) - opposite of QT
                    Qt::CaseSensitivity casesens = (stack.popint()==0?Qt::CaseSensitive:Qt::CaseInsensitive);
                    int start = stack.popint();
                    QString qstr = stack.popstring();
                    QString qhay = stack.popstring();

                    int pos = 0;
                    if(start < 1) {
                        errornum = ERROR_STRSTART;
                    } else {
						if (start <= qhay.length()) {
							pos = qhay.indexOf(qstr, start-1, casesens)+1;
						}
                    }
                    stack.pushint(pos);
                }
                break;
                
                
				case OP_INSTRX: {
					// regex instr
					int start = stack.popint();
					QRegExp expr = QRegExp(stack.popstring());
					expr.setMinimal(regexMinimal);
					QString qtemp = stack.popstring();
					
					int pos=0;

					if(start < 1) {
						errornum = ERROR_STRSTART;
					} else {
						pos = expr.indexIn(qtemp,start-1)+1;
					}
					stack.pushint(pos);
				}
				break;

                case OP_SIN:
                case OP_COS:
                case OP_TAN:
                case OP_ASIN:
                case OP_ACOS:
                case OP_ATAN:
                case OP_CEIL:
                case OP_FLOOR:
                case OP_ABS:
                case OP_DEGREES:
                case OP_RADIANS:
                case OP_LOG:
                case OP_LOGTEN:
                case OP_SQR:
                case OP_EXP: {

                    double val = stack.popfloat();
                    switch (opcode) {
                        case OP_SIN:
                            stack.pushfloat(sin(val));
                            break;
                        case OP_COS:
                            stack.pushfloat(cos(val));
                            break;
                        case OP_TAN:
                            val = tan(val);
                            if (isinf(val)) {
                                errornum = ERROR_INFINITY;
                                stack.pushint(0);
                            } else {
                                stack.pushfloat(val);
                            }
                            break;
                        case OP_ASIN:
                            stack.pushfloat(asin(val));
                            break;
                        case OP_ACOS:
                            stack.pushfloat(acos(val));
                            break;
                        case OP_ATAN:
                            stack.pushfloat(atan(val));
                            break;
                        case OP_CEIL:
                            stack.pushint(ceil(val));
                            break;
                        case OP_FLOOR:
                            stack.pushint(floor(val));
                            break;
                        case OP_ABS:
                            if (val < 0) {
                                val = -val;
                            }
                            stack.pushfloat(val);
                            break;
                        case OP_DEGREES:
                            stack.pushfloat(val * 180 / M_PI);
                            break;
                        case OP_RADIANS:
                            stack.pushfloat(val * M_PI / 180);
                            break;
                        case OP_LOG:
                            if (val<0) {
                                errornum = ERROR_LOGRANGE;
                                stack.pushint(0);
                            } else {
                                stack.pushfloat(log(val));
                            }
                            break;
                        case OP_LOGTEN:
                            if (val<0) {
                                errornum = ERROR_LOGRANGE;
                                stack.pushint(0);
                            } else {
                                stack.pushfloat(log10(val));
                            }
                            break;
                        case OP_SQR:
                            if (val<0) {
                                errornum = ERROR_LOGRANGE;
                                stack.pushint(0);
                            } else {
                                stack.pushfloat(sqrt(val));
                            }
                            break;
                        case OP_EXP:
                            val = exp(val);
                            if (isinf(val)) {
                                errornum = ERROR_INFINITY;
                                stack.pushint(0);
                            } else {
                                stack.pushfloat(val);
                            }
                            break;
                    }
                }
                break;


                case OP_ADD: {
                    double oneval = stack.popfloat();
                    double twoval = stack.popfloat();
                    double ans = twoval + oneval;
                    if (isinf(ans)) {
                        errornum = ERROR_INFINITY;
                        stack.pushint(0);
                    } else {
                        stack.pushfloat(ans);
                    }
                    break;
                }

                case OP_SUB: {
                    double oneval = stack.popfloat();
                    double twoval = stack.popfloat();
                    double ans = twoval - oneval;
                    if (isinf(ans)) {
                        errornum = ERROR_INFINITY;
                        stack.pushint(0);
                    } else {
                        stack.pushfloat(ans);
                    }
                    break;
                }

                case OP_MUL: {
                    double oneval = stack.popfloat();
                    double twoval = stack.popfloat();
                    double ans = twoval * oneval;
                    if (isinf(ans)) {
                        errornum = ERROR_INFINITY;
                        stack.pushint(0);
                    } else {
                        stack.pushfloat(ans);
                    }
                    break;
                }

                case OP_MOD: {
                    double oneval = stack.popfloat();
                    double twoval = stack.popfloat();
                    if (oneval==0) {
                        errornum = ERROR_DIVZERO;
                        stack.pushint(0);
                    } else {
                        double ans = fmod(twoval, oneval);
                        if (isinf(ans)) {
                            errornum = ERROR_INFINITY;
                            stack.pushint(0);
                        } else {
                            stack.pushfloat(ans);
                        }
                    }
                    break;
                }

                case OP_DIV: {
                    double oneval = stack.popfloat();
                    double twoval = stack.popfloat();
                    if (oneval==0) {
                        errornum = ERROR_DIVZERO;
                        stack.pushint(0);
                    } else {
                        double ans = twoval / oneval;
                        if (isinf(ans)) {
                            errornum = ERROR_INFINITY;
                            stack.pushint(0);
                        } else {
                            stack.pushfloat(ans);
                        }
                    }
                    break;
                }

                case OP_INTDIV: {
                    double oneval = stack.popfloat();
                    double twoval = stack.popfloat();
                    if (oneval==0) {
                        errornum = ERROR_DIVZERO;
                        stack.pushint(0);
                    } else {
                        double intpart;
                        modf(twoval /oneval, &intpart);
                        if (isinf(intpart)) {
                            errornum = ERROR_INFINITY;
                            stack.pushint(0);
                        } else {
                            stack.pushfloat(intpart);
                        }
                    }
                    break;
                }

                case OP_EX: {
                    double oneval = stack.popfloat();
                    double twoval = stack.popfloat();
                    double ans = pow(twoval, oneval);
                    if (std::isnan(ans)) {
                        errornum = ERROR_NOTANUMBER;
                        stack.pushint(0);
                    } else {
                        if (isinf(ans)) {
                            errornum = ERROR_INFINITY;
                            stack.pushint(0);
                        } else {
                            stack.pushfloat(ans);
                        }
                    }
                    break;
                }

                case OP_AND: {
                    int one = stack.popint();
                    int two = stack.popint();
                    if (one && two) {
                        stack.pushint(1);
                    } else {
                        stack.pushint(0);
                    }
                }
                break;

                case OP_OR: {
                    int one = stack.popint();
                    int two = stack.popint();
                    if (one || two)	{
                        stack.pushint(1);
                    } else {
                        stack.pushint(0);
                    }
                }
                break;

                case OP_XOR: {
                    int one = stack.popint();
                    int two = stack.popint();
                    if (!(one && two) && (one || two)) {
                        stack.pushint(1);
                    } else {
                        stack.pushint(0);
                    }
                }
                break;

                case OP_NOT: {
                    int temp = stack.popint();
                    if (temp) {
                        stack.pushint(0);
                    } else {
                        stack.pushint(1);
                    }
                }
                break;

                case OP_NEGATE: {
                    double n = stack.popfloat();
                    stack.pushfloat(n * -1);
                }
                break;

                case OP_EQUAL: {
                    int ans = stack.compareTopTwo()==0;
                    stack.pushint(ans);
                }
                break;

                case OP_NEQUAL: {
                    int ans = stack.compareTopTwo()!=0;
                    stack.pushint(ans);
                }
                break;

                case OP_GT: {
                    int ans = stack.compareTopTwo()==1;
                    stack.pushint(ans);

                }
                break;

                case OP_LTE: {
                    int ans = stack.compareTopTwo()!=1;
                    stack.pushint(ans);
                }
                break;

                case OP_LT: {
                    int ans = stack.compareTopTwo()==-1;
                    stack.pushint(ans);
                }
                break;

                case OP_GTE: {
                    int ans = stack.compareTopTwo()!=-1;
                    stack.pushint(ans);
                }
                break;

                case OP_SOUND_LIST: {
                    // play an immediate list of sounds

                    int length = stack.popint();

                    int* freqdur;
                    freqdur = (int*) malloc(length * sizeof(int));

                    for (int j = length-1; j >=0; j--) {
                        freqdur[j] = stack.popint();
                    }

                    sound.playSounds(length / 2 , freqdur);
                    free(freqdur);
                }
                break;


                case OP_VOLUME: {
                    // set the wave output height (volume 0-10)
                    int volume = stack.popint();
                    if (volume<0) volume = 0;
                    if (volume>10) volume = 10;
                    sound.setVolume(volume);
                }
                break;

                case OP_SAY: {
                    mymutex->lock();
                    emit(speakWords(stack.popstring()));
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                }
                break;

                case OP_SYSTEM: {
                    QString temp = stack.popstring();

                    if(settings.value(SETTINGSALLOWSYSTEM, SETTINGSALLOWSYSTEMDEFAULT).toBool()) {
                        mymutex->lock();
                        emit(executeSystem(temp));
                        waitCond->wait(mymutex);
                        mymutex->unlock();
                    } else {
                        errornum = ERROR_PERMISSION;
                    }
                }
                break;

                case OP_WAVPLAY: {
					QString file = stack.popstring();
#ifdef USEQSOUND
					if(file.compare("")!=0) {
						mymutex->lock();
						emit(playWAV(file));
						waitCond->wait(mymutex);
						mymutex->unlock();
					} else {
						errornum = ERROR_NOTIMPLEMENTED;
					}
#else
					if(file.compare("")!=0) {
						// load new file and start playback
						mediaplayer = new BasicMediaPlayer();
						mediaplayer->loadFile(file);
						mediaplayer->play();
						if (mediaplayer->error()!=0) {
							errornum = ERROR_WAVFILEFORMAT;
						}
					} else {
						// start playing an existing mediaplayer
						if (mediaplayer) {
							mediaplayer->play();
						} else {
							errornum = ERROR_WAVNOTOPEN;
						}
					}
#endif
                }
                break;

                case OP_WAVSTOP: {
#ifdef USEQSOUND
					mymutex->lock();
					emit(stopWAV());
					waitCond->wait(mymutex);
					mymutex->unlock();
#else
					if(mediaplayer) {
                        mediaplayer->stop();
					} else {
						errornum = ERROR_WAVNOTOPEN;
					}
#endif
                }
                break;

                case OP_SETCOLOR: {
                    unsigned int brushval = stack.popfloat();
                    unsigned int penval = stack.popfloat();

                    drawingpen.setColor(QColor::fromRgba((QRgb) penval));
                    drawingbrush.setColor(QColor::fromRgba((QRgb) brushval));
                }
                break;

                case OP_RGB: {
                    int aval = stack.popint();
                    int bval = stack.popint();
                    int gval = stack.popint();
                    int rval = stack.popint();
                    if (rval < 0 || rval > 255 || gval < 0 || gval > 255 || bval < 0 || bval > 255 || aval < 0 || aval > 255) {
                        errornum = ERROR_RGB;
                    } else {
                        stack.pushfloat( (unsigned int) QColor(rval,gval,bval,aval).rgba());
                    }
                }
                break;

                case OP_PIXEL: {
                    int y = stack.popint();
                    int x = stack.popint();
                    QRgb rgb = graphwin->image->pixel(x,y);
                    stack.pushfloat((unsigned int) rgb);
                }
                break;

                case OP_GETCOLOR: {
                    stack.pushfloat((unsigned int) drawingpen.color().rgba());
                }
                break;

                case OP_GETSLICE: {
                    // slice format is 4 digit HEX width, 4 digit HEX height,
                    // and (w*h)*8 digit HEX RGB for each pixel of slice
                    int h = stack.popint();
                    int w = stack.popint();
                    int y = stack.popint();
                    int x = stack.popint();
                    if (h*w > (STRINGMAXLEN -8)/8) {
                        errornum = ERROR_STRINGMAXLEN;
                        stack.pushint(0);
                    } else {
                        QString qs;
                        QRgb rgb;
                        int tw, th;
                        qs.append(QString::number(w,16).rightJustified(4,'0'));
                        qs.append(QString::number(h,16).rightJustified(4,'0'));
                        for(th=0; th<h; th++) {
                            for(tw=0; tw<w; tw++) {
                                rgb = graphwin->image->pixel(x+tw,y+th);
                                qs.append(QString::number(rgb,16).rightJustified(8,'0'));
                            }
                        }
                        stack.pushstring(qs);
                    }
                }
                break;

                case OP_PUTSLICE:
                case OP_PUTSLICEMASK: {
                    QRgb mask = 0x00;	// default mask transparent - nothing gets masked
                    if (opcode == OP_PUTSLICEMASK) mask = stack.popint();
                    QString imagedata = stack.popstring();
                    int y = stack.popint();
                    int x = stack.popint();
                    bool ok;
                    QRgb rgb, lastrgb = 0x00;
                    int th, tw;
                    int offset = 0; // location in qstring to get next hex number

                    int w = imagedata.mid(offset,4).toInt(&ok, 16);
                    offset+=4;
                    if (ok) {
                        int h = imagedata.mid(offset,4).toInt(&ok, 16);
                        offset+=4;
                        if (ok) {

                            QPainter *ian;
                            if (printing) {
                                ian = printdocumentpainter;
                            } else {
                                ian = new QPainter(graphwin->image);
                            }

                            ian->setPen(lastrgb);
                            for(th=0; th<h && ok; th++) {
                                for(tw=0; tw<w && ok; tw++) {
                                    rgb = imagedata.mid(offset, 8).toUInt(&ok, 16);
                                    offset+=8;
                                    if (ok && rgb != mask) {
                                        if (rgb!=lastrgb) {
                                            ian->setPen(rgb);
                                            lastrgb = rgb;
                                        }
                                        ian->drawPoint(x + tw, y + th);
                                    }
                                }
                            }
                            if (!printing) {
                                ian->end();
                                delete ian;
                                if (!fastgraphics) waitForGraphics();
                            }
                        }
                    }
                    if (!ok) {
                        errornum = ERROR_PUTBITFORMAT;
                    }
                }
                break;

                case OP_LINE: {
                    int y1val = stack.popint();
                    int x1val = stack.popint();
                    int y0val = stack.popint();
                    int x0val = stack.popint();

                    QPainter *ian;
                    if (printing) {
                        ian = printdocumentpainter;
                    } else {
                        ian = new QPainter(graphwin->image);
                    }

                    ian->setPen(drawingpen);
                    ian->setBrush(drawingbrush);
                    if (drawingpen.color()==QColor(0,0,0,0) && drawingbrush.color()==QColor(0,0,0,0) ) {
                        ian->setCompositionMode(QPainter::CompositionMode_Clear);
                    }
                    if (x1val >= 0 && y1val >= 0) {
                        ian->drawLine(x0val, y0val, x1val, y1val);
                     }

                    if (!printing) {
                        ian->end();
                        delete ian;
                        if (!fastgraphics) waitForGraphics();
                    }
                }
                break;


                case OP_RECT: {
                    int y1val = stack.popint();
                    int x1val = stack.popint();
                    int y0val = stack.popint();
                    int x0val = stack.popint();

                    QPainter *ian;
                    if (printing) {
                        ian = printdocumentpainter;
                    } else {
                        ian = new QPainter(graphwin->image);
                    }

                    if(x1val<0) {
                        x0val+=x1val;
                        x1val*=-1;
                    }
                    if(y1val<0) {
                        y0val+=y1val;
                        y1val*=-1;
                    }

                    ian->setBrush(drawingbrush);
                    ian->setPen(drawingpen);
                    if (drawingpen.color()==QColor(0,0,0,0) && drawingbrush.color()==QColor(0,0,0,0) ) {
                        ian->setCompositionMode(QPainter::CompositionMode_Clear);
                    }

                    if (x1val > 1 && y1val > 1) {
                        ian->drawRect(x0val, y0val, x1val-1, y1val-1);
                    } else if (x1val==1 && y1val==1) {
                        // rect 1x1 is actually a point
                        ian->drawPoint(x0val, y0val);
                    } else if (x1val==1 && y1val!=0) {
                        // rect 1xn is actually a line
                        ian->drawLine(x0val, y0val, x0val, y0val+y1val);
                    } else if (x1val!=0 && y1val==1) {
                        // rect nx1 is actually a line
                        ian->drawLine(x0val, y0val, x0val + x1val, y0val);
                    }

                    if (!printing) {
                        ian->end();
                        delete ian;
                        if (!fastgraphics) waitForGraphics();
                    }
                }
                break;


                case OP_POLY_LIST: {
                    // doing a polygon from an immediate list
                    // i is a pointer to the length of the list

                    int numbers = stack.popint();

                    int pairs = numbers / 2;
                    if (pairs < 3) {
                        errornum = ERROR_POLYPOINTS;
                    } else {
                        QPointF *points = new QPointF[pairs];
                        for (int j = 0; j < pairs; j++) {
                            int ypoint = stack.popint();
                            int xpoint = stack.popint();
                            points[j].setX(xpoint);
                            points[j].setY(ypoint);
                        }

                        QPainter *poly;
                        if (printing) {
                            poly = printdocumentpainter;
                        } else {
                            poly = new QPainter(graphwin->image);
                        }

                        poly->setPen(drawingpen);
                        poly->setBrush(drawingbrush);
                        if (drawingpen.color()==QColor(0,0,0,0) && drawingbrush.color()==QColor(0,0,0,0) ) {
                            poly->setCompositionMode(QPainter::CompositionMode_Clear);
                        }
                        poly->drawPolygon(points, pairs);

                        if(!printing) {
                            poly->end();
                            delete poly;
                            if (!fastgraphics) waitForGraphics();
                        }

                        delete points;
                    }
                }
                break;

                case OP_STAMP_LIST:
                case OP_STAMP_S_LIST:
                case OP_STAMP_SR_LIST: {
                    // special type of poly where x,y,scale, are given first and
                    // the ploy is sized and loacted - so we can move them easy
                    // doing a polygon from an immediate list
                    // i is a pointer to the length of the list
                    // pulling from stack so points are reversed 0=y, 1=x...  in list

                    double rotate=0;			// defaule rotation to 0 radians
                    double scale=1;			// default scale to full size (1x)

                    double tx, ty, savetx;		// used in scaling and rotating

                    // pop the immediate list to uncover the location and scale
                    int llist = stack.popint();
                    double *list = (double *) calloc(llist, sizeof(double));
                    for(int j = llist; j>0 ; j--) {
                        list[j-1] = stack.popfloat();
                    }

                    if (opcode==OP_STAMP_SR_LIST) rotate = stack.popfloat();
                    if (opcode==OP_STAMP_SR_LIST || opcode==OP_STAMP_S_LIST) scale = stack.popfloat();
                    int y = stack.popint();
                    int x = stack.popint();

                    if (scale<0) {
                        errornum = ERROR_IMAGESCALE;
                    } else {
                        int pairs = llist / 2;
                        if (pairs < 3) {
                            errornum = ERROR_POLYPOINTS;
                        } else {

                            QPointF *points = new QPointF[pairs];
                            for (int j = 0; j < pairs; j++) {
                                tx = scale * list[(j*2)];
                                ty = scale * list[(j*2)+1];
                                if (rotate!=0) {
                                    savetx = tx;
                                    tx = cos(rotate) * tx - sin(rotate) * ty;
                                    ty = cos(rotate) * ty + sin(rotate) * savetx;
                                }
                                points[j].setX(tx + x);
                                points[j].setY(ty + y);
                            }

                            QPainter *poly;
                            if (printing) {
                                poly = printdocumentpainter;
                            } else {
                                poly = new QPainter(graphwin->image);
                            }

                            poly->setPen(drawingpen);
                            poly->setBrush(drawingbrush);
                            if (drawingpen.color()==QColor(0,0,0,0) && drawingbrush.color()==QColor(0,0,0,0) ) {
                                poly->setCompositionMode(QPainter::CompositionMode_Clear);
                            }
                            poly->drawPolygon(points, pairs);

                            if (!printing) {
                                poly->end();
                                delete poly;
                                if (!fastgraphics) waitForGraphics();
                            }

                            delete points;
                        }
                    }

                    free(list);
                }
                break;


                case OP_CIRCLE: {
                    int rval = stack.popint();
                    int yval = stack.popint();
                    int xval = stack.popint();

                    QPainter *ian;
                    if (printing) {
                        ian = printdocumentpainter;
                    } else {
                        ian = new QPainter(graphwin->image);
                    }

                    ian->setPen(drawingpen);
                    ian->setBrush(drawingbrush);
                    if (drawingpen.color()==QColor(0,0,0,0) && drawingbrush.color()==QColor(0,0,0,0) ) {
                        ian->setCompositionMode(QPainter::CompositionMode_Clear);
                    }
                    ian->drawEllipse(xval - rval, yval - rval, 2 * rval, 2 * rval);

                    if(!printing) {
                        ian->end();
                        delete ian;
                        if (!fastgraphics) waitForGraphics();
                    }
                }
                break;

                case OP_IMGLOAD: {
                    // Image Load - with scale and rotate

                    // pop the filename to uncover the location and scale
                    QString file = stack.popstring();

                    double rotate = stack.popfloat();
                    double scale = stack.popfloat();
                    double y = stack.popint();
                    double x = stack.popint();

                    if (scale<0) {
                        errornum = ERROR_IMAGESCALE;
                    } else {
                        QImage i(file);
                        if(i.isNull()) {
                            errornum = ERROR_IMAGEFILE;
                        } else {
                            QPainter *ian;
                            if (printing) {
                                ian = printdocumentpainter;
                            } else {
                                ian = new QPainter(graphwin->image);
                            }

                            if (rotate != 0 || scale != 1) {
                                QTransform transform = QTransform().translate(0,0).rotateRadians(rotate).scale(scale, scale);
                                i = i.transformed(transform);
                            }
                            if (i.width() != 0 && i.height() != 0) {
                                ian->drawImage((int)(x - .5 * i.width()), (int)(y - .5 * i.height()), i);
                            }
                            if (!printing) {
                                ian->end();
                                delete ian;
                                if (!fastgraphics) waitForGraphics();
                            }
                        }
                    }
                }
                break;

                case OP_TEXT: {
                    QString txt = stack.popstring();
                    int y0val = stack.popint();
                    int x0val = stack.popint();

                    QPainter *ian;
                    if (printing) {
                        ian = printdocumentpainter;
                    } else {
                        ian = new QPainter(graphwin->image);
                    }

                    ian->setPen(drawingpen);
                    ian->setBrush(drawingbrush);
                    if (drawingpen.color()==QColor(0,0,0,0) && drawingbrush.color()==QColor(0,0,0,0) ) {
                        ian->setCompositionMode(QPainter::CompositionMode_Clear);
                    }
                    if(!fontfamily.isEmpty()) {
                        ian->setFont(QFont(fontfamily, fontpoint, fontweight));
                    }
                    ian->drawText(x0val, y0val+(QFontMetrics(ian->font()).ascent()), txt);

                    if (!printing) {
                        ian->end();
                        delete ian;
                        if (!fastgraphics) waitForGraphics();
                    }
                }
                break;


                case OP_FONT: {
                    int weight = stack.popint();
                    int point = stack.popint();
                    QString family = stack.popstring();
                    if (point<0) {
                        errornum = ERROR_FONTSIZE;
                    } else {
                        if (weight<0) {
                            errornum = ERROR_FONTWEIGHT;
                        } else {
                            fontpoint = point;
                            fontweight = weight;
                            fontfamily = family;
                        }
                    }
                }
                break;

                case OP_CLS: {
                    mymutex->lock();
                    emit(outputClear());
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                }
                break;

                case OP_CLG: {
                    graphwin->image->fill(QColor(0,0,0,0));
                    if (!fastgraphics) waitForGraphics();
                }
                break;

                case OP_PLOT: {
                    int oneval = stack.popint();
                    int twoval = stack.popint();

                    QPainter *ian;
                    if (printing) {
                        ian = printdocumentpainter;
                    } else {
                        ian = new QPainter(graphwin->image);
                    }

                    ian->setPen(drawingpen);
                    if (drawingpen.color()==QColor(0,0,0,0)) {
                        ian->setCompositionMode(QPainter::CompositionMode_Clear);
                    }
                    ian->drawPoint(twoval, oneval);

                    if (!printing) {
                        ian->end();
                        delete ian;
                        if (!fastgraphics) waitForGraphics();
                    }
                }
                break;

                case OP_FASTGRAPHICS: {
                    fastgraphics = true;
                    emit(fastGraphics());
                }
                break;

                case OP_GRAPHSIZE: {
                    int width = 300, height = 300;
                    int oneval = stack.popint();
                    int twoval = stack.popint();
                    if (oneval>0) height = oneval;
                    if (twoval>0) width = twoval;
                    if (width > 0 && height > 0) {
                        mymutex->lock();
                        emit(mainWindowsResize(1, width, height));
                        waitCond->wait(mymutex);
                        mymutex->unlock();
                    }
                }
                break;

                case OP_GRAPHWIDTH: {
                    if (printing) {
                        stack.pushint(printdocument->width());
                    } else {
                        stack.pushint((int) graphwin->image->width());
                    }
                }
                break;

                case OP_GRAPHHEIGHT: {
                    if (printing) {
                        stack.pushint(printdocument->height());
                    } else {
                        stack.pushint((int) graphwin->image->height());
                    }
                }
                break;

                case OP_REFRESH: {
                    mymutex->lock();
                    emit(goutputReady());
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                }
                break;

                case OP_INPUT: {
                    QString prompt = stack.popstring();
                    if (prompt.length()>0) {
                        mymutex->lock();
                        emit(outputReady(prompt));
                        waitCond->wait(mymutex);
                        mymutex->unlock();
                    }
#ifdef ANDROID
                    // input statement on android with popup keyboard
                    // problmatic so use a popup dialog box but display
                    // to output like it was really done there

                    mymutex->lock();
                    if (prompt.length()==0) prompt = "?";
                    emit(dialogPrompt(prompt,""));
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                    //
                    mymutex->lock();
                    emit(outputReady(returnString+"\n"));
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                    waitForGraphics();
                    //
                    stack.pushstring(returnString);
#else
                    // use the input status of interperter and get
                    // input from BasicOutput
                    // input is pushed by the emit back to the interperter
                    status = R_INPUT;
                    mymutex->lock();
                    emit(getInput());
                    waitCond->wait(mymutex);
                    mymutex->unlock();
#endif
                }
                break;

                case OP_KEY: {
#ifdef ANDROID
                    errornum = ERROR_NOTIMPLEMENTED;
                    stack.pushint(0);
#else
                    mymutex->lock();
                    stack.pushint(currentKey);
                    currentKey = 0;
                    mymutex->unlock();
#endif
                }
                break;

                case OP_PRINT:
                case OP_PRINTN: {
                    QString p = stack.popstring();
                    if (opcode == OP_PRINTN) {
                        p += "\n";
                    }
                    mymutex->lock();
                    emit(outputReady(p));
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                }
                break;

                case OP_CONCAT: {
                    QString one = stack.popstring();
                    QString two = stack.popstring();
                    unsigned int l = one.length() + two.length();
                    if (l<=STRINGMAXLEN) {
                        stack.pushstring(two + one);
                    } else {
                        errornum = ERROR_STRINGMAXLEN;
                        stack.pushint(0);
                    }
                }
                break;

                case OP_YEAR:
                case OP_MONTH:
                case OP_DAY:
                case OP_HOUR:
                case OP_MINUTE:
                case OP_SECOND: {
                    time_t rawtime;
                    struct tm * timeinfo;

                    time ( &rawtime );
                    timeinfo = localtime ( &rawtime );

                    switch (opcode) {
                        case OP_YEAR:
                            stack.pushint(timeinfo->tm_year + 1900);
                            break;
                        case OP_MONTH:
                            stack.pushint(timeinfo->tm_mon);
                            break;
                        case OP_DAY:
                            stack.pushint(timeinfo->tm_mday);
                            break;
                        case OP_HOUR:
                            stack.pushint(timeinfo->tm_hour);
                            break;
                        case OP_MINUTE:
                            stack.pushint(timeinfo->tm_min);
                            break;
                        case OP_SECOND:
                            stack.pushint(timeinfo->tm_sec);
                            break;
                    }
                }
                break;

                case OP_MOUSEX: {
                    stack.pushint((int) graphwin->mouseX);
                }
                break;

                case OP_MOUSEY: {
                    stack.pushint((int) graphwin->mouseY);
                }
                break;

                case OP_MOUSEB: {
                    stack.pushint((int) graphwin->mouseB);
                }
                break;

                case OP_CLICKCLEAR: {
                    graphwin->clickX = 0;
                    graphwin->clickY = 0;
                    graphwin->clickB = 0;
                }
                break;

                case OP_CLICKX: {
                    stack.pushint((int) graphwin->clickX);
                }
                break;

                case OP_CLICKY: {
                    stack.pushint((int) graphwin->clickY);
                }
                break;

                case OP_CLICKB: {
                    stack.pushint((int) graphwin->clickB);
                }
                break;

                case OP_INCREASERECURSE:
                    // increase recursion level in variable hash
                {
                    variables.increaserecurse();
                    if (variables.error()!=ERROR_NONE) {
                        errornum = variables.error();
                        errorvarnum = variables.errorvarnum();
                    }
                }
                break;

                case OP_DECREASERECURSE:
                    // decrease recursion level in variable hash
                    // and pop any unfinished for statements off of forstack
                {
                    while (forstack&&forstack->recurselevel == variables.getrecurse()) {
                        forframe *temp = new forframe;
                        temp = forstack;
                        forstack = temp->next;
                        delete temp;
                    }

                    if(debugMode != 0) {
                        // remove variables from variable window when we return back
                        emit(varAssignment(variables.getrecurse(),QString(""), QString(""), -1, -1));
                    }

                    variables.decreaserecurse();


                    if (variables.error()!=ERROR_NONE) {
                        errornum = variables.error();
                        errorvarnum = variables.errorvarnum();
                    }
                }
                break;

                case OP_SPRITEPOLY_LIST: {
                    // create a sprite from a polygon from an immediate list on the stack
                    // first stack element is the number of points*2 to pull from the stack
                    int maxx=0, maxy=0;
                    int x,y;

                    //emit(outputReady("begining of spritepoly=**" + stack.debug() + "**\n"));

                    int i = stack.popint(); // number of numbers pushed on stack from list or array

                    int pairs = i / 2;
                    if (pairs < 3) {
                        errornum = ERROR_POLYPOINTS;
                    } else {
                        QPointF *points = new QPointF[pairs];
                        for (int j = 0; j < pairs; j++) {
                            y = stack.popint();
                            x = stack.popint();
                            points[j].setX(x);
                            points[j].setY(y);
                            if (x>maxx) maxx=x;
                            if (y>maxy) maxy=y;
                        }

                        int n = stack.popint(); // sprite number
                        if(n < 0 || n >=nsprites) {
                            errornum = ERROR_SPRITENUMBER;
                        } else {
                            spriteundraw(n);
                            if (sprites[n].image) {
                                // free previous image before replacing
                                delete sprites[n].image;
                                sprites[n].image = NULL;
                            }
                            sprites[n].image = new QImage(maxx+1,maxy+1,QImage::Format_ARGB32);
                            sprites[n].image->fill(Qt::transparent);
                            QPainter *poly = new QPainter(sprites[n].image);
                            poly->setPen(drawingpen);
                            poly->setBrush(drawingbrush);
                            if (drawingpen.color()==QColor(0,0,0,0) && drawingbrush.color()==QColor(0,0,0,0) ) {
                                poly->setCompositionMode(QPainter::CompositionMode_Clear);
                            }
                            poly->drawPolygon(points, pairs);
                            poly->end();
							delete poly;
							
                            if (sprites[n].underimage) {
                                // free previous underimage before replacing
                                delete sprites[n].underimage;
                                sprites[n].underimage = NULL;
                            }
                            sprites[n].visible = false;
                            spriteredraw(n);
                        }
                        delete points;
                    }
                }
                break;




                case OP_SPRITEDIM: {
                    int n = stack.popint();
                    // deallocate existing sprites
                    clearsprites();
                    // create new ones that are not visible, active, and are at origin
                    if (n > 0) {
                        sprites = (sprite*) malloc(sizeof(sprite) * n);
                        nsprites = n;
                        while (n>0) {
                            n--;
                            sprites[n].image = NULL;
                            sprites[n].underimage = NULL;
                            sprites[n].visible = false;
                            sprites[n].x = 0;
                            sprites[n].y = 0;
                            sprites[n].r = 0;
                            sprites[n].s = 1;
                        }
                    }
                }
                break;

                case OP_SPRITELOAD: {

                    QString file = stack.popstring();
                    int n = stack.popint();

                    if(n < 0 || n >=nsprites) {
                        errornum = ERROR_SPRITENUMBER;
                    } else {
                        spriteundraw(n);
                        if (sprites[n].image) {
                            // free previous image before replacing
                            delete sprites[n].image;
                            sprites[n].image = NULL;
                        }
                        sprites[n].image = new QImage(file);
                        if(sprites[n].image->isNull()) {
                            errornum = ERROR_IMAGEFILE;
                            sprites[n].image = NULL;
                        } else {
                            if (sprites[n].underimage) {
                                // free previous underimage before replacing
                                delete sprites[n].underimage;
                                sprites[n].underimage = NULL;
                            }
                            sprites[n].visible = false;
                        }
                        spriteredraw(n);
                    }
                }
                break;

                case OP_SPRITESLICE: {

                    int h = stack.popint();
                    int w = stack.popint();
                    int y = stack.popint();
                    int x = stack.popint();
                    int n = stack.popint();

                    if(n < 0 || n >=nsprites) {
                        errornum = ERROR_SPRITENUMBER;
                    } else {
                        spriteundraw(n);
                        if (sprites[n].image) {
                            // free previous image before replacing
                            delete sprites[n].image;
                            sprites[n].image = NULL;
                        }
                        sprites[n].image = new QImage(graphwin->image->copy(x, y, w, h));
                        if(sprites[n].image->isNull()) {
                            errornum = ERROR_SPRITESLICE;
                            sprites[n].image = NULL;
                        } else {
                            if (sprites[n].underimage) {
                                // free previous underimage before replacing
                                delete sprites[n].underimage;
                                sprites[n].underimage = NULL;
                            }
                            sprites[n].visible = false;
                        }
                        spriteredraw(n);
                    }
                }
                break;

                case OP_SPRITEMOVE:
                case OP_SPRITEPLACE: {

                    double r = stack.popfloat();
                    double s = stack.popfloat();
                    double y = stack.popfloat();
                    double x = stack.popfloat();
                    int n = stack.popint();

                    if(n < 0 || n >=nsprites) {
                        errornum = ERROR_SPRITENUMBER;
                    } else {
                        if (s<0) {
                            errornum = ERROR_IMAGESCALE;
                        } else {
                            if(!sprites[n].image) {
                                errornum = ERROR_SPRITENA;
                            } else {

                                spriteundraw(n);
                                if (opcode==OP_SPRITEMOVE) {
                                    x += sprites[n].x;
                                    y += sprites[n].y;
                                    s += sprites[n].s;
                                    r += sprites[n].r;
                                    if (x >= (int) graphwin->image->width()) x = (double) graphwin->image->width();
                                    if (x < 0) x = 0;
                                    if (y >= (int) graphwin->image->height()) y = (double) graphwin->image->height();
                                    if (y < 0) y = 0;
                                    if (s < 0) s = 0;
                                }
                                sprites[n].x = x;
                                sprites[n].y = y;
                                sprites[n].s = s;
                                sprites[n].r = r;
                                spriteredraw(n);

                                if (!fastgraphics) waitForGraphics();
                            }
                        }
                    }
                }
                break;

                case OP_SPRITEHIDE:
                case OP_SPRITESHOW: {

                    int n = stack.popint();
                    bool vis = opcode==OP_SPRITESHOW;

                    if(n < 0 || n >=nsprites) {
                        errornum = ERROR_SPRITENUMBER;
                    } else {
                        if(!sprites[n].image) {
                            errornum = ERROR_SPRITENA;
                        } else {
                            if (sprites[n].visible != vis) {
                                spriteundraw(n);
                                sprites[n].visible = vis;
                                spriteredraw(n);
                                if (!fastgraphics) waitForGraphics();
                            }
                        }
                    }
                }
                break;

                case OP_SPRITECOLLIDE: {

                    int n1 = stack.popint();
                    int n2 = stack.popint();

                    if(n1 < 0 || n1 >=nsprites || n2 < 0 || n2 >=nsprites) {
                        errornum = ERROR_SPRITENUMBER;
                    } else {
                        if(!sprites[n1].image || !sprites[n2].image) {
                            errornum = ERROR_SPRITENA;
                        } else {
                            stack.pushint(spritecollide(n1, n2));
                        }
                    }
                }
                break;

                case OP_SPRITEX:
                case OP_SPRITEY:
                case OP_SPRITEH:
                case OP_SPRITEW:
                case OP_SPRITEV:
                case OP_SPRITER:
                case OP_SPRITES: {

                    int n = stack.popint();

                    if(n < 0 || n >=nsprites) {
                        errornum = ERROR_SPRITENUMBER;
                        stack.pushint(0);
                    } else {
                        if(!sprites[n].image) {
                            errornum = ERROR_SPRITENA;
                            stack.pushint(0);
                        } else {
                            if (opcode==OP_SPRITEX) stack.pushfloat(sprites[n].x);
                            if (opcode==OP_SPRITEY) stack.pushfloat(sprites[n].y);
                            if (opcode==OP_SPRITEH) stack.pushint(sprites[n].image->height());
                            if (opcode==OP_SPRITEW) stack.pushint(sprites[n].image->width());
                            if (opcode==OP_SPRITEV) stack.pushint(sprites[n].visible?1:0);
                            if (opcode==OP_SPRITER) stack.pushfloat(sprites[n].r);
                            if (opcode==OP_SPRITES) stack.pushfloat(sprites[n].s);
                        }
                    }
                }
                break;


                case OP_CHANGEDIR: {
                    QString file = stack.popstring();
                    if(!QDir::setCurrent(file)) {
                        errornum = ERROR_FOLDER;
                    }
                }
                break;

                case OP_CURRENTDIR: {
                    stack.pushstring(QDir::currentPath());
                }
                break;

                case OP_WAVWAIT: {
#ifdef USEQSOUND
					mymutex->lock();
					emit(waitWAV());
					waitCond->wait(mymutex);
					mymutex->unlock();
#else
					if(mediaplayer) {
						mediaplayer->wait();
					} else {
						errornum = ERROR_WAVNOTOPEN;
					}
#endif
                }
                break;

                case OP_DBOPEN: {
                    // open database connection
                    QString file = stack.popstring();
                    int n = stack.popint();
                    if (n<0||n>=NUMDBCONN) {
                        errornum = ERROR_DBCONNNUMBER;
                    } else {
                        closeDatabase(n);
                        QString dbconnection = "DBCONNECTION" + QString::number(n);
                        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",dbconnection);
                        db.setDatabaseName(file);
                        bool ok = db.open();
                        if (!ok) {
                            errornum = ERROR_DBOPEN;
                            closeDatabase(n);
                        }
                    }
                }
                break;

                case OP_DBCLOSE: {
                    int n = stack.popint();
                    if (n<0||n>=NUMDBCONN) {
                        errornum = ERROR_DBCONNNUMBER;
                    } else {
                        closeDatabase(n);
                    }
                }
                break;

                case OP_DBEXECUTE: {
                    // execute a statement on the database
                    QString stmt = stack.popstring();
                    int n = stack.popint();
                    if (n<0||n>=NUMDBCONN) {
                        errornum = ERROR_DBCONNNUMBER;
                    } else {
                        QString dbconnection = "DBCONNECTION" + QString::number(n);
                        QSqlDatabase db = QSqlDatabase::database(dbconnection);
                        if(db.isValid()) {
                            QSqlQuery *q = new QSqlQuery(db);
                            bool ok = q->exec(stmt);
                            if (!ok) {
                                errornum = ERROR_DBQUERY;
                                errormessage = q->lastError().databaseText();
                            }
                            delete q;
                        } else {
                            errornum = ERROR_DBNOTOPEN;
                        }
                    }
                }
                break;

                case OP_DBOPENSET: {
                    // open recordset
                    QString stmt = stack.popstring();
                    int set = stack.popint();
                    int n = stack.popint();
                    if (n<0||n>=NUMDBCONN) {
                        errornum = ERROR_DBCONNNUMBER;
                    } else {
                        if (set<0||set>=NUMDBSET) {
                            errornum = ERROR_DBSETNUMBER;
                        } else {
                            QString dbconnection = "DBCONNECTION" + QString::number(n);
                            QSqlDatabase db = QSqlDatabase::database(dbconnection);
                            if(db.isValid()) {
                                if (dbSet[n][set]) {
                                    dbSet[n][set]->clear();
                                    delete dbSet[n][set];
                                    dbSet[n][set] = NULL;
                                }
                                dbSet[n][set] = new QSqlQuery(db);
                                bool ok = dbSet[n][set]->exec(stmt);
                                if (!ok) {
                                    errornum = ERROR_DBQUERY;
                                    errormessage = dbSet[n][set]->lastError().databaseText();
                                }
                            } else {
                                errornum = ERROR_DBNOTOPEN;
                            }
                        }
                    }
                }
                break;

                case OP_DBCLOSESET: {
                    int set = stack.popint();
                    int n = stack.popint();
                    if (n<0||n>=NUMDBCONN) {
                        errornum = ERROR_DBCONNNUMBER;
                    } else {
                        if (set<0||set>=NUMDBSET) {
                            errornum = ERROR_DBSETNUMBER;
                        } else {
                            if (dbSet[n][set]) {
                                dbSet[n][set]->clear();
                                delete dbSet[n][set];
                                dbSet[n][set] = NULL;
                            } else {
                                errornum = ERROR_DBNOTSET;
                            }
                        }
                    }
                }
                break;

                case OP_DBROW: {
                    int set = stack.popint();
                    int n = stack.popint();
                    if (n<0||n>=NUMDBCONN) {
                        errornum = ERROR_DBCONNNUMBER;
                    } else {
                        if (set<0||set>=NUMDBSET) {
                            errornum = ERROR_DBSETNUMBER;
                        } else {
                            if (dbSet[n][set]) {
                                // return true if we move to a new row else false
                                stack.pushint(dbSet[n][set]->next());
                            } else {
                                errornum = ERROR_DBNOTSET;
                            }
                        }
                    }
                }
                break;

                case OP_DBINT:
                case OP_DBINTS:
                case OP_DBFLOAT:
                case OP_DBFLOATS:
                case OP_DBNULL:
                case OP_DBNULLS:
                case OP_DBSTRING:
                case OP_DBSTRINGS: {

                    int col = -1, set, n;
                    QString colname;
                    // get a column data (integer)
                    if (opcode == OP_DBINTS || opcode == OP_DBFLOATS || opcode == OP_DBNULLS || opcode == OP_DBSTRINGS) {
                        colname = stack.popstring();
                    } else {
                        col = stack.popint();
                    }
                    set = stack.popint();
                    n = stack.popint();
                    if (n<0||n>=NUMDBCONN) {
                        errornum = ERROR_DBCONNNUMBER;
                    } else {
                        if (set<0||set>=NUMDBSET) {
                            errornum = ERROR_DBSETNUMBER;
                        } else {
                            if (!dbSet[n][set]->isActive()) {
                                errornum = ERROR_DBNOTSET;
                            } else {
                                if (!dbSet[n][set]->isValid()) {
                                    errornum = ERROR_DBNOTSETROW;
                                } else {
                                    if (opcode == OP_DBINTS || opcode == OP_DBFLOATS || opcode == OP_DBNULLS || opcode == OP_DBSTRINGS) {
                                        col = dbSet[n][set]->record().indexOf(colname);
                                    }
                                    if (col < 0 || col >= dbSet[n][set]->record().count()) {
                                        errornum = ERROR_DBCOLNO;
                                    } else {
                                        switch(opcode) {
                                            case OP_DBINT:
                                            case OP_DBINTS:
                                                stack.pushint(dbSet[n][set]->record().value(col).toInt());
                                                break;
                                            case OP_DBFLOAT:
                                            case OP_DBFLOATS:
                                                stack.pushfloat(dbSet[n][set]->record().value(col).toDouble());
                                                break;
                                            case OP_DBNULL:
                                            case OP_DBNULLS:
                                                stack.pushint(dbSet[n][set]->record().value(col).isNull());
                                                break;
                                            case OP_DBSTRING:
                                            case OP_DBSTRINGS:
                                                stack.pushstring(dbSet[n][set]->record().value(col).toString());
                                                break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                break;

                case OP_LASTERROR: {
                    stack.pushint(lasterrornum);
                }
                break;

                case OP_LASTERRORLINE: {
                    stack.pushint(lasterrorline);
                }
                break;

                case OP_LASTERROREXTRA: {
                    stack.pushstring(lasterrormessage);
                }
                break;

                case OP_LASTERRORMESSAGE: {
                    stack.pushstring(getErrorMessage(lasterrornum));
                }
                break;

                case OP_OFFERROR: {
                    // pop a trap off of the trap stack
                    onerrorframe *temp = onerrorstack;
                    if (temp) {
                        onerrorstack = temp->next;
                    }
                }
                break;

                case OP_NETLISTEN: {
                    int tempsockfd;
                    struct sockaddr_in serv_addr, cli_addr;
                    socklen_t clilen;

                    int port = stack.popint();
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMSOCKETS) {
                        errornum = ERROR_NETSOCKNUMBER;
                    } else {
                        if (netsockfd[fn] >= 0) {
                            netsockfd[fn] = netSockClose(netsockfd[fn]);
                        }

                        // SOCK_DGRAM = UDP  SOCK_STREAM = TCP
                        tempsockfd = socket(AF_INET, SOCK_STREAM, 0);
                        if (tempsockfd < 0) {
                            errornum = ERROR_NETSOCK;
                            errormessage = strerror(errno);
                        } else {
                            int optval = 1;
                            if (setsockopt(tempsockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(int))) {
                                errornum = ERROR_NETSOCKOPT;
                                errormessage = strerror(errno);
                                tempsockfd = netSockClose(tempsockfd);
                            } else {
                                memset((char *) &serv_addr, 0, sizeof(serv_addr));
                                serv_addr.sin_family = AF_INET;
                                serv_addr.sin_addr.s_addr = INADDR_ANY;
                                serv_addr.sin_port = htons(port);
                                if (bind(tempsockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
                                    errornum = ERROR_NETBIND;
                                    errormessage = strerror(errno);
                                    tempsockfd = netSockClose(tempsockfd);
                                } else {
                                    listen(tempsockfd,5);
                                    clilen = sizeof(cli_addr);
                                    netsockfd[fn] = accept(tempsockfd, (struct sockaddr *) &cli_addr, &clilen);
                                    if (netsockfd[fn] < 0) {
                                        errornum = ERROR_NETACCEPT;
                                        errormessage = strerror(errno);
                                    }
                                    tempsockfd = netSockClose(tempsockfd);
                                }
                            }
                        }
                    }
                }
                break;

                case OP_NETCONNECT: {

                    struct sockaddr_in serv_addr;
                    struct hostent *server;

                    int port = stack.popint();
                    QString address = stack.popstring();
                    int fn = stack.popint();

                    if (fn<0||fn>=NUMSOCKETS) {
                        errornum = ERROR_NETSOCKNUMBER;
                    } else {

                        if (netsockfd[fn] >= 0) {
                            netsockfd[fn] = netSockClose(netsockfd[fn]);
                        }

                        netsockfd[fn] = socket(AF_INET, SOCK_STREAM, 0);
                        if (netsockfd[fn] < 0) {
                            errornum = ERROR_NETSOCK;
                            errormessage = strerror(errno);
                        } else {

                            server = gethostbyname(address.toUtf8().data());
                            if (server == NULL) {
                                errornum = ERROR_NETHOST;
                                errormessage = strerror(errno);
                                netsockfd[fn] = netSockClose(netsockfd[fn]);
                            } else {
                                memset((char *) &serv_addr, 0, sizeof(serv_addr));
                                serv_addr.sin_family = AF_INET;
                                memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
                                serv_addr.sin_port = htons(port);
                                if (::connect(netsockfd[fn],(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
                                    errornum = ERROR_NETCONN;
                                    errormessage = strerror(errno);
                                    netsockfd[fn] = netSockClose(netsockfd[fn]);
                                }
                            }
                        }
                    }
                }
                break;

                case OP_NETREAD: {
                    int MAXSIZE = 2048;
                    int n;
                    char * strarray = (char *) malloc(MAXSIZE);

                    int fn = stack.popint();
                    if (fn<0||fn>=NUMSOCKETS) {
                        errornum = ERROR_NETSOCKNUMBER;
                        stack.pushint(0);
                    } else {
                        if (netsockfd[fn] < 0) {
                            errornum = ERROR_NETNONE;
                            stack.pushint(0);
                        } else {
                            memset(strarray, 0, MAXSIZE);
                            n = recv(netsockfd[fn],strarray,MAXSIZE-1,0);
                            if (n < 0) {
                                errornum = ERROR_NETREAD;
                                errormessage = strerror(errno);
                                stack.pushint(0);
                            } else {
                                stack.pushstring(QString::fromUtf8(strarray));
                            }
                        }
                    }
                    free(strarray);
                }
                break;

                case OP_NETWRITE: {
                    QString data = stack.popstring();
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMSOCKETS) {
                        errornum = ERROR_NETSOCKNUMBER;
                    } else {
                        if (netsockfd[fn]<0) {
                            errornum = ERROR_NETNONE;
                        } else {
                            int n = send(netsockfd[fn],data.toUtf8().data(),data.length(),0);
                            if (n < 0) {
                                errornum = ERROR_NETWRITE;
                                errormessage = strerror(errno);
                            }
                        }
                    }
                }
                break;

                case OP_NETCLOSE: {
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMSOCKETS) {
                        errornum = ERROR_NETSOCKNUMBER;
                    } else {
                        if (netsockfd[fn]<0) {
                            errornum = ERROR_NETNONE;
                        } else {
                            netsockfd[fn] = netSockClose(netsockfd[fn]);
                        }
                    }
                }
                break;

                case OP_NETDATA: {
                    // push 1 if there is data to read on network connection
                    // wait 1 ms for each poll
                    int fn = stack.popint();
                    if (fn<0||fn>=NUMSOCKETS) {
                        errornum = ERROR_NETSOCKNUMBER;
                    } else {
#ifdef WIN32
                        unsigned long n;
                        if (ioctlsocket(netsockfd[fn], FIONREAD, &n)!=0) {
                            stack.pushint(0);
                        } else {
                            if (n==0L) {
                                stack.pushint(0);
                            } else {
                                stack.pushint(1);
                            }
                        }
#else
                        struct pollfd p[1];
                        p[0].fd = netsockfd[fn];
                        p[0].events = POLLIN | POLLPRI;
                        if(poll(p, 1, 1)<0) {
                            stack.pushint(0);
                        } else {
                            if (p[0].revents & POLLIN || p[0].revents & POLLPRI) {
                                stack.pushint(1);
                            } else {
                                stack.pushint(0);
                            }
                        }
#endif
                    }
                }
                break;

                case OP_NETADDRESS: {
                    // get first non "lo" ip4 address
#ifdef WIN32
                    char szHostname[100];
                    HOSTENT *pHostEnt;
                    int nAdapter = 0;
                    struct sockaddr_in sAddr;
                    gethostname( szHostname, sizeof( szHostname ));
                    pHostEnt = gethostbyname( szHostname );
                    memcpy ( &sAddr.sin_addr.s_addr, pHostEnt->h_addr_list[nAdapter], pHostEnt->h_length);
                    stack.pushstring(QString::fromUtf8(inet_ntoa(sAddr.sin_addr)));
#else
#ifdef ANDROID
                    errornum = ERROR_NOTIMPLEMENTED;
#else
                    bool good = false;
                    struct ifaddrs *myaddrs, *ifa;
                    void *in_addr;
                    char buf[64];
                    if(getifaddrs(&myaddrs) != 0) {
                        errornum = ERROR_NETNONE;
                    } else {
                        for (ifa = myaddrs; ifa != NULL && !good; ifa = ifa->ifa_next) {
                            if (ifa->ifa_addr == NULL) continue;
                            if (!(ifa->ifa_flags & IFF_UP)) continue;
                            if (ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, "lo") !=0 ) {
                                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                                in_addr = &s4->sin_addr;
                                if (inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf))) {
                                    stack.pushstring(QString::fromUtf8(buf));
                                    good = true;
                                }
                            }
                        }
                        freeifaddrs(myaddrs);
                    }
                    if (!good) {
                        // on error give local loopback
                        stack.pushstring(QString("127.0.0.1"));
                    }
#endif
#endif
                }
                break;

                case OP_KILL: {
                    QString name = stack.popstring();
                    if(!QFile::remove(name)) {
                        errornum = ERROR_FILEOPEN;
                    }
                }
                break;

                case OP_MD5: {
                    QString stuff = stack.popstring();
                    stack.pushstring(MD5(stuff.toUtf8().data()).hexdigest());
                }
                break;

                case OP_SETSETTING: {
                    QString stuff = stack.popstring();
                    QString key = stack.popstring();
                    QString app = stack.popstring();
                    if(settings.value(SETTINGSALLOWSETTING, SETTINGSALLOWSETTINGDEFAULT).toBool()) {
                        settings.beginGroup(SETTINGSGROUPUSER);
                        settings.beginGroup(app);
                        settings.setValue(key, stuff);
                        settings.endGroup();
                        settings.endGroup();
                    } else {
                        errornum = ERROR_PERMISSION;
                    }
                }
                break;

                case OP_GETSETTING: {
                    QString key = stack.popstring();
                    QString app = stack.popstring();
                    if(settings.value(SETTINGSALLOWSETTING, SETTINGSALLOWSETTINGDEFAULT).toBool()) {
                        if(app==QString("SYSTEM")) {
							stack.pushstring(settings.value(key, "").toString());
						} else {
							settings.beginGroup(SETTINGSGROUPUSER);
							settings.beginGroup(app);
							stack.pushstring(settings.value(key, "").toString());
							settings.endGroup();
							settings.endGroup();
						}
					} else {
						errornum = ERROR_PERMISSION;
						stack.pushint(0);
					}
				}
                break;


                case OP_PORTOUT: {
                    int data = stack.popint();
                    int port = stack.popint();
                    if(settings.value(SETTINGSALLOWPORT, SETTINGSALLOWPORTDEFAULT).toBool()) {
#ifdef WIN32
#ifdef WIN32PORTABLE
                        errornum = ERROR_NOTIMPLEMENTED;
# else
                        if (Out32==NULL) {
                            errornum = ERROR_NOTIMPLEMENTED;
                        } else {
                            Out32(port, data);
                        }
#endif
#else
                        errornum = ERROR_NOTIMPLEMENTED;
#endif
                    } else {
                        errornum = ERROR_PERMISSION;
                    }
                }
                break;

                case OP_PORTIN: {
                    int data=0;
                    int port = stack.popint();
                    if(settings.value(SETTINGSALLOWPORT, SETTINGSALLOWPORTDEFAULT).toBool()) {
#ifdef WIN32
#ifdef WIN32PORTABLE
                        errornum = ERROR_NOTIMPLEMENTED;
# else
                        if (Inp32==NULL) {
                            errornum = ERROR_NOTIMPLEMENTED;
                        } else {
                            data = Inp32(port);
                        }
#endif
#else
                        errornum = ERROR_NOTIMPLEMENTED;
#endif
                    } else {
                        errornum = ERROR_PERMISSION;
                    }
                    stack.pushint(data);
                }
                break;

                case OP_BINARYOR: {
                    unsigned long a = stack.popint();
                    unsigned long b = stack.popint();
                    stack.pushfloat(a|b);
                }
                break;

                case OP_BINARYAND: {
                    unsigned long a = stack.popint();
                    unsigned long b = stack.popint();
                    stack.pushfloat(a&b);
                }
                break;

                case OP_BINARYNOT: {
                    unsigned long a = stack.popint();
                    stack.pushfloat(~a);
                }
                break;

                case OP_IMGSAVE: {
                    // Image Save - Save image
                    QString type = stack.popstring();
                    QString file = stack.popstring();
                    QStringList validtypes;
                    validtypes << "BMP" << "bmp" << "JPG" << "jpg" << "JPEG" << "jpeg" << "PNG" << "png";
                    if (validtypes.indexOf(type)!=-1) {
                        graphwin->image->save(file, type.toUtf8().data());
                    } else {
                        errornum = ERROR_IMAGESAVETYPE;
                    }
                }
                break;

                case OP_DIR: {
                    // Get next directory entry - id path send start a new folder else get next file name
                    // return "" if we have no names on list - skippimg . and ..
                    QString folder = stack.popstring();
                    if (folder.length()>0) {
                        if(directorypointer != NULL) {
                            closedir(directorypointer);
                            directorypointer = NULL;
                        }
                        directorypointer = opendir( folder.toUtf8().data() );
                    }
                    if (directorypointer != NULL) {
                        struct dirent *dirp;
                        dirp = readdir(directorypointer);
                        while(dirp != NULL && dirp->d_name[0]=='.') dirp = readdir(directorypointer);
                        if (dirp) {
                            stack.pushstring(QString::fromUtf8(dirp->d_name));
                        } else {
                            stack.pushstring(QString(""));
                            closedir(directorypointer);
                            directorypointer = NULL;
                        }
                    } else {
                        errornum = ERROR_FOLDER;
                        stack.pushint(0);
                    }
                }
                break;

                case OP_REPLACE: {
                    // unicode safe replace function

                   // 0 sensitive (default) - opposite of QT
                    Qt::CaseSensitivity casesens = (stack.popint()==0?Qt::CaseSensitive:Qt::CaseInsensitive);

                    QString qto = stack.popstring();
                    QString qfrom = stack.popstring();
                    QString qhaystack = stack.popstring();

                    stack.pushstring(qhaystack.replace(qfrom, qto, casesens));
                }
                break;

                case OP_REPLACEX: {
                    // regex replace function

                    QString qto = stack.popstring();
                    QRegExp expr = QRegExp(stack.popstring());
					expr.setMinimal(regexMinimal);
					QString qhaystack = stack.popstring();

					stack.pushstring(qhaystack.replace(expr, qto));
                }
                break;

                case OP_COUNT: {
                    // unicode safe count function

                    // 0 sensitive (default) - opposite of QT
                    Qt::CaseSensitivity casesens = (stack.popint()==0?Qt::CaseSensitive:Qt::CaseInsensitive);
                    
                    QString qneedle = stack.popstring();
                    QString qhaystack = stack.popstring();

					stack.pushint((int) (qhaystack.count(qneedle, casesens)));
                }
                break;

                case OP_COUNTX: {
                    // regex count function

                    QRegExp expr = QRegExp(stack.popstring());
					expr.setMinimal(regexMinimal);
                    QString qhaystack = stack.popstring();

                    stack.pushint((int) (qhaystack.count(expr)));
                }
                break;

                case OP_OSTYPE: {
                    // Return type of OS this compile was for
                    int os = -1;
#ifdef WIN32
                    os = 0;
#endif
#ifdef LINUX
                    os = 1;
#endif
#ifdef MACX
                    os = 2;
#endif
#ifdef ANDROID
                    os = 3;
#endif
                    stack.pushint(os);
                }
                break;

                case OP_MSEC: {
                    // Return number of milliseconds the BASIC256 program has been running
                    stack.pushint((int) (runtimer.elapsed()));
                }
                break;

                case OP_EDITVISIBLE:
                case OP_GRAPHVISIBLE:
                case OP_OUTPUTVISIBLE: {
                    int show = stack.popint();
                    if (opcode==OP_EDITVISIBLE) emit(mainWindowsVisible(0,show!=0));
                    if (opcode==OP_GRAPHVISIBLE) emit(mainWindowsVisible(1,show!=0));
                    if (opcode==OP_OUTPUTVISIBLE) emit(mainWindowsVisible(2,show!=0));
                }
                break;

                case OP_REGEXMINIMAL: {
					// set the regular expression minimal flag (true = not greedy)
                    regexMinimal = stack.popint()!=0;
                }
                break;

                case OP_TEXTHEIGHT:
                case OP_TEXTWIDTH: {
                    // return the number of pixels the font requires for diaplay
                    // a string is required for width but not for height
                    int v = 0;

                    QPainter *ian;
                    if (printing) {
                        ian = printdocumentpainter;
                    } else {
                        ian = new QPainter(graphwin->image);
                    }

                    if(!fontfamily.isEmpty()) {
                        ian->setFont(QFont(fontfamily, fontpoint, fontweight));
                    }

                    if (opcode==OP_TEXTWIDTH) {
                        QString txt = stack.popstring();
                        v = QFontMetrics(ian->font()).width(txt);
                    }
                    if (opcode==OP_TEXTHEIGHT) v = QFontMetrics(ian->font()).height();

                    if (!printing) {
                        ian->end();
                        delete ian;
                    }
                    stack.pushint((int) (v));
                }
                break;

                case OP_FREEFILE: {
                    // return the next free file number - throw error if not free files
                    int f=-1;
                    for (int t=0; (t<NUMFILES)&&(f==-1); t++) {
                        if (!filehandle[t]) f = t;
                    }
                    if (f==-1) {
                        errornum = ERROR_FREEFILE;
                        stack.pushint(0);
                    } else {
                        stack.pushint(f);
                    }
                }
                break;

                case OP_FREENET: {
                    // return the next free network socket number - throw error if not free sockets
                    int f=-1;
                    for (int t=0; (t<NUMSOCKETS)&&(f==-1); t++) {
                        if (netsockfd[t]==-1) f = t;
                    }
                    if (f==-1) {
                        errornum = ERROR_FREENET;
                        stack.pushint(0);
                    } else {
                        stack.pushint(f);
                    }
                }
                break;

                case OP_FREEDB: {
                    // return the next free databsae number - throw error if none free
                    int f=-1;
                    for (int t=0; (t<NUMDBCONN)&&(f==-1); t++) {
                        QString dbconnection = "DBCONNECTION" + QString::number(t);
                        QSqlDatabase db = QSqlDatabase::database(dbconnection);
                        if (!db.isValid()) f = t;
                    }
                    if (f==-1) {
                        errornum = ERROR_FREEDB;
                        stack.pushint(0);
                    } else {
                        stack.pushint(f);
                    }
                }
                break;

                case OP_FREEDBSET: {
                    // return the next free set for a database - throw error if none free
                    int n = stack.popint();
                    int f=-1;
                    if (n<0||n>=NUMDBCONN) {
                        errornum = ERROR_DBCONNNUMBER;
                        stack.pushint(0);
                    } else {
                        for (int t=0; (t<NUMDBSET)&&(f==-1); t++) {
                            if (!dbSet[n][t]) f = t;
                        }
                        if (f==-1) {
                            errornum = ERROR_FREEDBSET;
                            stack.pushint(0);
                        } else {
                            stack.pushint(f);
                        }
                    }
                }
                break;

                case OP_ARC:
                case OP_CHORD:
                case OP_PIE: {
                    double angwval = stack.popfloat();
                    double startval = stack.popfloat();
                    int hval = stack.popint();
                    int wval = stack.popint();
                    int yval = stack.popint();
                    int xval = stack.popint();

                    // degrees * 16
                    int s = (int) (startval * 360 * 16 / 2 / M_PI);
                    int aw = (int) (angwval * 360 * 16 / 2 / M_PI);
                    // transform to clockwise from 12'oclock
                    s = 1440-s-aw;

                    QPainter *ian;
                    if (printing) {
                        ian = printdocumentpainter;
                    } else {
                        ian = new QPainter(graphwin->image);
                    }

                    ian->setPen(drawingpen);
                    ian->setBrush(drawingbrush);
                    if (drawingpen.color()==QColor(0,0,0,0) && drawingbrush.color()==QColor(0,0,0,0) ) {
                        ian->setCompositionMode(QPainter::CompositionMode_Clear);
                    }
                    if(opcode==OP_ARC) {
                        ian->drawArc(xval, yval, wval, hval, s, aw);
                    }
                    if(opcode==OP_CHORD) {
                        ian->drawChord(xval, yval, wval, hval, s, aw);
                    }
                    if(opcode==OP_PIE) {
                        ian->drawPie(xval, yval, wval, hval, s, aw);
                    }

                    if (!printing) {
                        ian->end();
                        delete ian;
                        if (!fastgraphics) waitForGraphics();
                    }
                }
                break;

                case OP_PENWIDTH: {
                    double a = stack.popfloat();
                    if (a<0) {
                        errornum = ERROR_PENWIDTH;
                    } else {
                        drawingpen.setWidthF(a);
                    }
                }
                break;

                case OP_GETPENWIDTH: {
                    stack.pushfloat((double) (drawingpen.widthF()));
                }
                break;

                case OP_GETBRUSHCOLOR: {
                    stack.pushfloat((unsigned int) drawingbrush.color().rgba());
                }
                break;

                case OP_ALERT: {
                    QString temp = stack.popstring();
                    mymutex->lock();
                    emit(dialogAlert(temp));
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                    waitForGraphics();
                }
                break;

                case OP_CONFIRM: {
                    int dflt = stack.popint();
                    QString temp = stack.popstring();
                    mymutex->lock();
                    emit(dialogConfirm(temp,dflt));
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                    stack.pushint(returnInt);
                    waitForGraphics();
                }
                break;

                case OP_PROMPT: {
                    QString dflt = stack.popstring();
                    QString msg = stack.popstring();
                    mymutex->lock();
                    emit(dialogPrompt(msg,dflt));
                    waitCond->wait(mymutex);
                    mymutex->unlock();
                    stack.pushstring(returnString);
                    waitForGraphics();
                }
                break;

                case OP_FROMRADIX: {
                    bool ok;
                    unsigned long dec;
                    int base = stack.popint();
                    QString n = stack.popstring();
                    if (base>=2 && base <=36) {
                        dec = n.toULong(&ok, base);
                        if (ok) {
                            stack.pushfloat(dec);
                        } else {
                            errornum = ERROR_RADIXSTRING;
                            stack.pushfloat(0);
                        }
                    } else {
                        errornum = ERROR_RADIX;
                        stack.pushfloat(0);
                    }
                }
                break;

                case OP_TORADIX: {
                    int base = stack.popint();
                    unsigned long n = stack.popfloat();
                    if (base>=2 && base <=36) {
                        QString out;
                        out.setNum(n, base);
                        stack.pushstring(out);
                    } else {
                        errornum = ERROR_RADIX;
                        stack.pushfloat(0);
                    }
                }
                break;

                case OP_PRINTEROFF: {
                    if (printing) {
                        printing = false;
                        printdocumentpainter->end();
                        delete printdocumentpainter;
                        delete printdocument;
                    } else {
                        errornum = ERROR_PRINTERNOTON;
                    }
                }
                break;

                case OP_PRINTERON: {
                    if (printing) {
                        errornum = ERROR_PRINTERNOTOFF;
                    } else {
                        int resolution = settings.value(SETTINGSPRINTERRESOLUTION, SETTINGSPRINTERRESOLUTIONDEFAULT).toInt();
                        int printer = settings.value(SETTINGSPRINTERPRINTER, 0).toInt();
                        int paper = settings.value(SETTINGSPRINTERPAPER, SETTINGSPRINTERPAPERDEFAULT).toInt();
                        if (printer==-1) {
                            // pdf printer
                            printdocument = new QPrinter((QPrinter::PrinterMode) resolution);
                            printdocument->setOutputFormat(QPrinter::PdfFormat);
                            printdocument->setOutputFileName(settings.value(SETTINGSPRINTERPDFFILE, "./output.pdf").toString());

                        } else {
                            // system printer
                            QList<QPrinterInfo> printerList=QPrinterInfo::availablePrinters();
                            if (printer>=printerList.count()) printer = 0;
                            printdocument = new QPrinter(printerList[printer], (QPrinter::PrinterMode) resolution);
                        }
                        if (printdocument) {
                            printdocument->setPaperSize((QPrinter::PaperSize) paper);
                            printdocument->setOrientation((QPrinter::Orientation)settings.value(SETTINGSPRINTERORIENT, SETTINGSPRINTERORIENTDEFAULT).toInt());
                            printdocumentpainter = new QPainter();
                            if (!printdocumentpainter->begin(printdocument)) {
                                errornum = ERROR_PRINTEROPEN;
                            } else {
                                printing = true;
                            }
                        } else {
                            errornum = 99999;
                        }
                    }
                }
                break;

                case OP_PRINTERPAGE: {
                    if (printing) {
                        printdocument->newPage();
                    } else {
                        errornum = ERROR_PRINTERNOTON;
                    }
                }
                break;

                case OP_PRINTERCANCEL: {
                    if (printing) {
                        printing = false;
                        printdocumentpainter->end();
                        delete printdocumentpainter;
                        printdocument->abort();
                        delete printdocument;
                    } else {
                        errornum = ERROR_PRINTERNOTON;
                    }
                }
                break;

                case OP_DEBUGINFO: {
                    // get info about BASIC256 runtime and return as a string
                    // put totally undocumented stuff HERE
                    // NOT FOR HUMANS TO USE
                    int what = stack.popint();
                    switch (what) {
                        case 1:
                            // stack height and content
                            mymutex->lock();
                            emit(outputReady(stack.debug()));
                            waitCond->wait(mymutex);
                            mymutex->unlock();
                            stack.pushint(stack.height());
                            break;
                        case 2:
                            // type of top stack element
                            stack.pushint(stack.peekType());
                            break;
                        case 3:
                            // number of symbols - display them to output area
							{
								for(int i=0; i<numsyms; i++) {
									mymutex->lock();
									emit(outputReady(QString("SYM %1 %2 LOC %3\n").arg(i).arg(symtable[i],-32).arg(labeltable[i],8,16,QChar('0'))));
									waitCond->wait(mymutex);
									mymutex->unlock();
								}
								stack.pushint(numsyms);
							}
							break;
                        case 4:
                            // dump the program object code
							{
								int *o = wordCode;
								while (o <= wordCode + wordOffset) {
									mymutex->lock();
									unsigned int offset = o-wordCode;
									int currentop = *o;
									o++;
									if (optype(currentop) == OPTYPE_NONE)	{
										emit(outputReady(QString("%1 %2\n").arg(offset,8,16,QChar('0')).arg(opname(currentop),-20)));
									} else if (optype(currentop) == OPTYPE_INT) {
										//op has one Int arg
										emit(outputReady(QString("%1 %2 %3\n").arg(offset,8,16,QChar('0')).arg(opname(currentop),-20).arg((int) *o)));
										o++;
									} else if (optype(currentop) == OPTYPE_VARIABLE) {
										//op has one Int arg
										emit(outputReady(QString("%1 %2 %3\n").arg(offset,8,16,QChar('0')).arg(opname(currentop),-20).arg(symtable[(int) *o])));
										o++;
									} else if (optype(currentop) == OPTYPE_LABEL) {
										//op has one Int arg (label - lookup the address from the labeltable
										emit(outputReady(QString("%1 %2 %3 %4\n").arg(offset,8,16,QChar('0')).arg(opname(currentop),-20).arg(labeltable[(int) *o],8,16,QChar('0')).arg(symtable[(int) *o])));
										o++;
									} else if (optype(currentop) == OPTYPE_FLOAT) {
										// op has a single double arg
										emit(outputReady(QString("%1 %2 %3\n").arg(offset,8,16,QChar('0')).arg(opname(currentop),-20).arg((double) *o)));
										o += bytesToFullWords(sizeof(double));
									} else if (optype(currentop) == OPTYPE_STRING) {
										// op has a single null terminated String arg
										emit(outputReady(QString("%1 %2 \"%3\"\n").arg(offset,8,16,QChar('0')).arg(opname(currentop),-20).arg((char*) o)));
										int len = bytesToFullWords(strlen((char*) o) + 1);
										o += len;
									}
									waitCond->wait(mymutex);
									mymutex->unlock();
								}
							}
							stack.pushint(0);
							break;
                        case 5:
                            // dump the variables
							{
								mymutex->lock();
								emit(outputReady(variables.debug()));
								waitCond->wait(mymutex);
								mymutex->unlock();
							}
							stack.pushint(0);
							break;

                        default:
                            stack.pushstring("");
                    }
                }
                break;

                case OP_STACKSWAP: {
                    // swap the top of the stack
                    // 0, 1, 2, 3...  becomes 1, 0, 2, 3...
                    stack.swap();
                }
                break;

                case OP_STACKSWAP2: {
                    // swap the top two pairs of the stack
                    // 0, 1, 2, 3...  becomes 2,3, 0,1...
                    stack.swap2();
                }
                break;

                case OP_STACKDUP: {
                    // duplicate top stack entry
                    stack.dup();
                }
                break;

                case OP_STACKDUP2: {
                    // duplicate top 2 stack entries
                    stack.dup2();
                }
                break;

                case OP_STACKTOPTO2: {
                    // move the top of the stack under the next two
                    // 0, 1, 2, 3...  becomes 1, 2, 0, 3...
                    stack.topto2();
                }
                break;

                case OP_ARGUMENTCOUNTTEST: {
                    // Throw error if stack does not have enough values
                    // used to check if functions and subroutines have the proper number
                    // of datas on the stack to fill the parameters
                    int a = stack.popint();
                    if (stack.height()<a) errornum = ERROR_ARGUMENTCOUNT;
                    //printf("ac args=%i stack=%i\n",a,stack.height());
                }
                break;

                case OP_THROWERROR: {
                    // Throw a user defined error number
                    int fn = stack.popint();
                    errornum = fn;
                }
                break;

                case OP_WAVLENGTH: {
					double d = 0.0;
#ifdef USEQSOUND
                    errornum = ERROR_NOTIMPLEMENTED;
#else
					if (mediaplayer) {
						if ((d = mediaplayer->length())==0)
							errornum = WARNING_WAVNODURATION;
					} else {
						errornum = ERROR_WAVNOTOPEN;
					}
#endif
					stack.pushfloat(d);
                }
                break;

                case OP_WAVPAUSE: {
#ifdef USEQSOUND
                    errornum = ERROR_NOTIMPLEMENTED;
#else
					if (mediaplayer) {
						mediaplayer->pause();
					} else {
						errornum = ERROR_WAVNOTOPEN;
					}
#endif
                }
                break;

                case OP_WAVPOS: {
					double p = 0.0;
#ifdef USEQSOUND
                    errornum = ERROR_NOTIMPLEMENTED;
#else
					if (mediaplayer) {
						p = mediaplayer->position();
					} else {
						errornum = ERROR_WAVNOTOPEN;
					}
#endif
					stack.pushfloat(p);
                }
                break;

                case OP_WAVSEEK: {
#ifdef USEQSOUND
                    errornum = ERROR_NOTIMPLEMENTED;
#else
					double pos = stack.popfloat();
					if (mediaplayer) {
						if (!mediaplayer->seek(pos)) {
							errornum = WARNING_WAVNOTSEEKABLE;
						}
					} else {
						errornum = ERROR_WAVNOTOPEN;
					}
#endif
                }
                break;

                case OP_WAVSTATE: {
					int s = 0;
#ifdef USEQSOUND
                    errornum = ERROR_NOTIMPLEMENTED;
#else
					if (mediaplayer) {
						s = mediaplayer->state();
					} else {
						errornum = ERROR_WAVNOTOPEN;
					}
#endif
					stack.pushint(s);
				}
                break;



                    // insert additional OPTYPE_NONE operations here



            }
        }
        break;

        default: {
            //emit(outputReady("optype=" + QString::number(optype(currentop)) + " op=" + QString::number(currentop,16) + "\n"));
            emit(outputReady(tr("Error in bytecode during label referencing at line ") + QString::number(currentLine) + ".\n"));
            return -1;
        }
        break;
    }

    return 0;
}
