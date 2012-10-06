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




//No argument ops
#define OP_END   		0x00
#define OP_NOP   		0x01
#define OP_RETURN		0x02
#define OP_CONCAT		0x03

#define OP_EQUAL 		0x04
#define OP_NEQUAL		0x05
#define OP_GT    		0x06
#define OP_LT    		0x07
#define OP_GTE   		0x08
#define OP_LTE   		0x09
#define OP_AND   		0x0a
#define OP_NOT   		0x0b
#define OP_OR    		0x0c
#define OP_XOR   		0x0d

#define OP_INT   		0x0e
#define OP_STRING		0x0f

#define OP_ADD   		0x10
#define OP_SUB   		0x11
#define OP_MUL   		0x12
#define OP_DIV   		0x13
#define OP_EX	 		0x14
#define OP_NEGATE		0x15

#define OP_PRINT 		0x16
#define OP_PRINTN		0x17
#define OP_INPUT 		0x18
#define OP_KEY   		0x19

#define OP_PLOT  		0x1a
#define OP_RECT  		0x1b
#define OP_CIRCLE		0x1c
#define OP_LINE  		0x1d

#define OP_REFRESH		0x1e
#define OP_FASTGRAPHICS		0x1f
#define OP_CLS			0x20
#define OP_CLG			0x21
#define OP_GRAPHSIZE            0x22
#define OP_GRAPHWIDTH		0x23
#define OP_GRAPHHEIGHT		0x24

#define OP_SIN			0x28
#define OP_COS			0x29
#define OP_TAN			0x2a
#define OP_RAND			0x2b
#define OP_CEIL			0x2c
#define OP_FLOOR		0x2d
#define OP_ABS			0x2e

#define OP_PAUSE		0x2f
#define OP_POLY			0x30
#define OP_LENGTH		0x31
#define OP_MID			0x32
#define OP_INSTR		0x33
#define OP_INSTR_S		0x34
#define OP_INSTR_SC		0x35
#define OP_INSTRX		0x36
#define OP_INSTRX_S		0x37


#define OP_OPEN			0x38
#define OP_READ			0x39
#define OP_WRITE		0x3a
#define OP_CLOSE		0x3b
#define OP_RESET		0x3c

#define OP_INCREASERECURSE	0x3e
#define OP_DECREASERECURSE	0x3f

#define OP_SOUND		0x40

//Int argument ops
#define OP_GOTO          	0x41
#define OP_GOSUB         	0x42
#define OP_BRANCH        	0x43
#define OP_NUMASSIGN     	0x45
#define OP_STRINGASSIGN  	0x46
#define OP_ARRAYASSIGN   	0x47
#define OP_STRARRAYASSIGN	0x48
#define OP_PUSHVAR       	0x49
#define OP_PUSHINT       	0x4a
#define OP_DEREF         	0x4b
#define OP_FOR           	0x4c
#define OP_NEXT          	0x4d
#define OP_CURRLINE      	0x4e
#define OP_DIM           	0x4f
#define OP_DIMSTR        	0x50
#define OP_ONERROR         	0x51
#define OP_EXPLODESTR		0x52
#define OP_EXPLODESTR_C		0x53
#define OP_EXPLODE		0x54
#define OP_EXPLODE_C		0x55
#define OP_EXPLODEXSTR		0x56
#define OP_EXPLODEX		0x57
#define OP_IMPLODE		0x58
#define OP_GLOBAL		0x59


//2 Int argument ops
#define OP_ARRAYLISTASSIGN	0x60
#define OP_STRARRAYLISTASSIGN	0x61

//Float argument ops
#define OP_PUSHFLOAT		0x70

//String argument ops
#define OP_PUSHSTRING		0x80

// jmr
#define OP_ASC			0x90
#define OP_CHR			0x91
#define OP_FLOAT		0x92
#define OP_READLINE		0x93
#define OP_WRITELINE		0x94
#define OP_EOF			0x95
#define OP_MOD			0x96
#define OP_YEAR			0x97
#define OP_MONTH		0x98
#define OP_DAY			0x99
#define OP_HOUR			0x9a
#define OP_MINUTE		0x9b
#define OP_SECOND		0x9c
#define OP_SETCOLORRGB		0x9d
#define OP_TEXT			0x9e
#define OP_FONT			0x9f
#define OP_SAY			0xa0
#define OP_WAVPLAY		0xa1
#define OP_WAVSTOP		0xa2
#define OP_SEEK			0xa3
#define OP_SIZE			0xa4
#define OP_EXISTS		0xa5
#define OP_STAMP		0xa6	// stamp with 4 numbers x,y,scale,rotation and an array
#define OP_STAMP_LIST		0xa7	// stamp with x, y, and an immediate list
#define OP_STAMP_S_LIST		0xa8	// stamp with x, y, scale and an immediate list
#define OP_STAMP_SR_LIST	0xa9	// stamp with x, y, scale, and rotation and an immediate list
#define OP_POLY_LIST		0xaa
#define OP_MOUSEX		0xab
#define OP_MOUSEY		0xac
#define OP_MOUSEB		0xad
#define OP_CLICKCLEAR		0xae
#define OP_CLICKX		0xaf
#define OP_CLICKY		0xb0
#define OP_CLICKB		0xb1
#define OP_LEFT			0xb2
#define OP_RIGHT		0xb3
#define OP_UPPER		0xb4
#define OP_LOWER		0xb5
#define OP_ARRAYASSIGN2D   	0xb8
#define OP_STRARRAYASSIGN2D	0xb9
#define OP_DEREF2D		0xba
#define OP_SYSTEM		0xbb
#define OP_VOLUME		0xbc
#define OP_SOUND_ARRAY		0xbd
#define OP_SOUND_LIST		0xbe
#define OP_SETCOLORINT		0xbf
#define OP_RGB			0xc0
#define OP_PIXEL		0xc1
#define OP_GETCOLOR		0xc2
#define OP_ASIN			0xc3
#define OP_ACOS			0xc4
#define OP_ATAN			0xc5
#define OP_DEGREES		0xc6
#define OP_RADIANS		0xc7
#define OP_REDIM          	0xc8
#define OP_REDIMSTR        	0xc9
#define OP_REDIM2D          	0xca
#define OP_REDIMSTR2D        	0xcb
#define OP_ALEN 	       	0xcd
#define OP_ALENX 	       	0xce
#define OP_ALENY 	       	0xcf
#define OP_INTDIV 	       	0xd0
#define OP_LOG			0xd1
#define OP_LOGTEN 	       	0xd2
#define OP_GETSLICE 	       	0xd3
#define OP_PUTSLICE 	       	0xd4
#define OP_PUTSLICEMASK		0xd5
#define OP_IMGLOAD 	       	0xd6
#define OP_IMGLOAD_S 	       	0xd7
#define OP_IMGLOAD_SR 	       	0xd8
#define OP_SQR			0xd9
#define OP_EXP			0xda

// extended codes
#define OP_EXTENDED_0		0xe0
#define OP_EXTENDED_1		0xe1
#define OP_EXTENDED_2		0xe2
#define OP_EXTENDED_3		0xe3
#define OP_EXTENDED_4		0xe4
#define OP_EXTENDED_5		0xe5
#define OP_EXTENDED_6		0xe6
#define OP_EXTENDED_7		0xe7
#define OP_EXTENDED_8		0xe8
#define OP_EXTENDED_9		0xe9
#define OP_EXTENDED_A		0xea
#define OP_EXTENDED_B		0xeb
#define OP_EXTENDED_C		0xec
#define OP_EXTENDED_D		0xed
#define OP_EXTENDED_E		0xee
#define OP_EXTENDED_F		0xef

// stack manipulation and special
#define OP_STACKSWAP		0xf0
#define OP_STACKTOPTO2		0xf1
#define OP_ARGUMENTCOUNTTEST	0xf2
#define OP_THROWERROR		0xff


// extended opcodes (second byte)
// first group e0xx
#define OP_SPRITEDIM 	       	0x00
#define OP_SPRITELOAD 	       	0x01
#define OP_SPRITESLICE 	       	0x02
#define OP_SPRITEMOVE 	       	0x03
#define OP_SPRITEHIDE 	       	0x04
#define OP_SPRITESHOW 	       	0x05
#define OP_SPRITECOLLIDE	0x06
#define OP_SPRITEPLACE 	       	0x07
#define OP_SPRITEX 	       	0x08
#define OP_SPRITEY 	       	0x09
#define OP_SPRITEH 	       	0x0a
#define OP_SPRITEW 	       	0x0b
#define OP_SPRITEV	       	0x0c
#define OP_CHANGEDIR	       	0x0d
#define OP_CURRENTDIR	       	0x0e
#define OP_WAVWAIT		0x0f
#define OP_DECIMAL		0x10
#define OP_DBOPEN		0x11
#define OP_DBCLOSE		0x12
#define OP_DBEXECUTE		0x13
#define OP_DBOPENSET		0x14
#define OP_DBCLOSESET		0x15
#define OP_DBROW		0x16
#define OP_DBINT		0x17
#define OP_DBFLOAT		0x18
#define OP_DBSTRING		0x19
#define OP_LASTERROR		0x1a
#define OP_LASTERRORLINE	0x1b
#define OP_LASTERRORMESSAGE	0x1c
#define OP_LASTERROREXTRA	0x1d
#define OP_OFFERROR		0x1e
#define OP_NETLISTEN		0x1f
#define OP_NETCONNECT		0x20
#define OP_NETREAD		0x21
#define OP_NETWRITE		0x22
#define OP_NETCLOSE		0x23
#define OP_NETDATA		0x24
#define OP_NETADDRESS		0x25
#define OP_KILL			0x26
#define OP_MD5			0x27
#define OP_SETSETTING		0x28
#define OP_GETSETTING		0x29
#define OP_PORTIN		0x2a
#define OP_PORTOUT		0x2b
#define OP_BINARYOR		0x2c
#define OP_BINARYAND		0x2d
#define OP_BINARYNOT		0x2e
#define OP_IMGSAVE		0x2f
#define OP_DIR			0x30
#define OP_REPLACE		0x31
#define OP_REPLACE_C		0x32
#define OP_REPLACEX		0x33
#define OP_COUNT		0x34
#define OP_COUNT_C		0x35
#define OP_COUNTX		0x36
#define OP_OSTYPE		0x37
#define OP_MSEC			0x38
#define OP_EDITVISIBLE		0x39
#define OP_GRAPHVISIBLE		0x40
#define OP_OUTPUTVISIBLE	0x41
#define OP_TEXTWIDTH		0x42



