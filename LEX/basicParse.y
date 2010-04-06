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


%{


#ifdef __cplusplus
  extern "C" {
#endif
    
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "../ByteCodes.h"

    #define SYMTABLESIZE 2000
    #define IFTABLESIZE 1000

    extern int yylex();
    extern char *yytext;
    int yyerror(const char *);
    int errorcode;
    extern int column;
    extern int linenumber;

    char *byteCode = NULL;
    unsigned int byteOffset = 0;
	unsigned int lastLineOffset = 0;	// store the byte offset for the end of the last line - use in loops
    unsigned int oldByteOffset = 0;
    unsigned int listlen = 0;

    struct label 
    {
      char *name;
      int offset;
    };

    char *EMPTYSTR = "";
    char *symtable[SYMTABLESIZE];
    int labeltable[SYMTABLESIZE];
    int numsyms = 0;
    int numlabels = 0;
    unsigned int maxbyteoffset = 0;
	
	// array to hold stack of if statement branch locations
	// that need to have final jump location added to them
	unsigned int iftable[IFTABLESIZE];
	unsigned int numifs = 0;

    int
    basicParse(char *);

	void clearIfTable()
	{
		int j;
		for (j = 0; j < IFTABLESIZE; j++)
		{
			iftable[j] = -1;
		}
		numifs = 0;
    }

    void 
    clearLabelTable()
    {
      int j;
      for (j = 0; j < SYMTABLESIZE; j++)
	{
	  labeltable[j] = -1;
	}
	numlabels = 0;
    }

    void
    clearSymbolTable()
    {
      int j;
      if (numsyms == 0)
	{
	  for (j = 0; j < SYMTABLESIZE; j++)
	    {
	      symtable[j] = 0;
	    }
	}
      for (j = 0; j < numsyms; j++)
	{
	  if (symtable[j])
	    {
	      free(symtable[j]);
	    }
	  symtable[j] = 0;
	}
      numsyms = 0;
    }

    int 
    getSymbol(char *name)
    {
      int i;
      for (i = 0; i < numsyms; i++)
	{
	  if (symtable[i] && !strcmp(name, symtable[i]))
	    return i;
	}
      return -1;
    }

    int
    newSymbol(char *name) 
    {
      symtable[numsyms] = name;
      numsyms++;
      return numsyms - 1;
    }


    int
    newByteCode(unsigned int size) 
    {
      if (byteCode)
	{
	  free(byteCode);
	}
      maxbyteoffset = 1024;
      byteCode = malloc(maxbyteoffset);

      if (byteCode)
	{
	  memset(byteCode, 0, maxbyteoffset);
	  byteOffset = 0;
	  return 0;
	}
      
      return -1;
    }
    
    void 
    checkByteMem(unsigned int addedbytes)
    {
      if (byteOffset + addedbytes + 1 >= maxbyteoffset)
	{
	  maxbyteoffset += maxbyteoffset + addedbytes + 32;
	  byteCode = realloc(byteCode, maxbyteoffset);
	  memset(byteCode + byteOffset, 0, maxbyteoffset - byteOffset);
	}
    }

    void
    addOp(char op)
    {
      checkByteMem(sizeof(char));
      byteCode[byteOffset] = op;
      byteOffset++;
    }

	unsigned int addInt(int data) {
	  // add an integer to the bytecode at the current location
	  // return starting location of the integer - so we can write to it later
      unsigned int holdOffset = byteOffset;
	  int *temp;
	  checkByteMem(sizeof(int));
      temp = (void *) byteCode + byteOffset;
      *temp = data;
      byteOffset += sizeof(int);
	  return holdOffset;
	}
	
    void 
    addIntOp(char op, int data)
    {
      checkByteMem(sizeof(char) + sizeof(int));
      int *temp;
      byteCode[byteOffset] = op;
      byteOffset++;
      
      temp = (void *) byteCode + byteOffset;
      *temp = data;
      byteOffset += sizeof(int);
    }

    void 
    addInt2Op(char op, int data1, int data2)
    {
      checkByteMem(sizeof(char) + 2 * sizeof(int));
      int *temp;
      byteCode[byteOffset] = op;
      byteOffset++;
      
      temp = (void *) byteCode + byteOffset;
      temp[0] = data1;
      temp[1] = data2;
      byteOffset += 2 * sizeof(int);
    }

    void 
    addFloatOp(char op, double data)
    {
      checkByteMem(sizeof(char) + sizeof(double));
      double *temp;
      byteCode[byteOffset] = op;
      byteOffset++;
      
      temp = (void *) byteCode + byteOffset;
      *temp = data;
      byteOffset += sizeof(double);
    }

    void 
    addStringOp(char op, char *data)
    {
      int len = strlen(data) + 1;
      checkByteMem(sizeof(char) + len);
      double *temp;
      byteCode[byteOffset] = op;
      byteOffset++;
      
      temp = (void *) byteCode + byteOffset;
      strncpy((char *) byteCode + byteOffset, data, len);
      byteOffset += len;
    }


#ifdef __cplusplus
  }
#endif

%}

%token PRINT INPUT KEY 
%token PIXEL RGB PLOT CIRCLE RECT POLY STAMP LINE FASTGRAPHICS GRAPHSIZE REFRESH CLS CLG
%token IF THEN ELSE ENDIF WHILE ENDWHILE DO UNTIL FOR TO STEP NEXT 
%token OPEN READ WRITE CLOSE RESET
%token GOTO GOSUB RETURN REM END SETCOLOR
%token GTE LTE NE
%token DIM NOP
%token TOINT TOSTRING LENGTH MID LEFT RIGHT UPPER LOWER INSTR
%token CEIL FLOOR RAND SIN COS TAN ASIN ACOS ATAN ABS PI
%token AND OR XOR NOT
%token PAUSE SOUND
%token ASC CHR TOFLOAT READLINE WRITELINE BOOLEOF MOD
%token YEAR MONTH DAY HOUR MINUTE SECOND TEXT FONT
%token SAY SYSTEM
%token VOLUME
%token GRAPHWIDTH GRAPHHEIGHT
%token WAVPLAY WAVSTOP
%token SIZE SEEK EXISTS
%token BOOLTRUE BOOLFALSE
%token MOUSEX MOUSEY MOUSEB
%token CLICKCLEAR CLICKX CLICKY CLICKB
%token GETCOLOR
%token CLEAR BLACK WHITE RED DARKRED GREEN DARKGREEN BLUE DARKBLUE
%token CYAN DARKCYAN PURPLE DARKPURPLE YELLOW DARKYELLOW
%token ORANGE DARKORANGE GREY DARKGREY

%union 
{
  int number;
  double floatnum;
  char *string;
}

%token <number> LINENUM
%token <number> INTEGER
%token <floatnum> FLOAT 
%token <string> STRING
%token <number> VARIABLE
%token <number> STRINGVAR
%token <string> NEWVAR
%token <number> COLOR
%token <number> LABEL

%type <floatnum> floatexpr
%type <string> stringexpr

%left XOR 
%left OR 
%left AND 
%nonassoc NOT
%left '<' LTE '>' GTE '=' NE
%left '-' '+'
%left MOD
%left '*' '/'
%nonassoc UMINUS
%left '^'



%%

program: validline '\n'         
       | validline '\n' program 
;

validline: compoundifstmt       { lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber); }
         | ifstmt  { lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber); }
         | elsestmt   { lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber); }
         | endifstmt    { lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber); }
         | whilestmt   { 
			// push to iftable the byte location of the end of the last stmt (top of loop)
			iftable[numifs] = lastLineOffset;
			numifs++;
			lastLineOffset = byteOffset; 
			addIntOp(OP_CURRLINE, linenumber);
			}
         | endwhilestmt    { lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber); }
         | dostmt   { 
			// push to iftable the byte location of the end of the last stmt (top of loop)
			iftable[numifs] = lastLineOffset;
			numifs++;
			lastLineOffset = byteOffset; 
			addIntOp(OP_CURRLINE, linenumber);
			}
         | untilstmt    { lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber); }
         | compoundstmt { lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber); }
         | /*empty*/    { lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber); }
         | LABEL        { labeltable[$1] = byteOffset; lastLineOffset = byteOffset; addIntOp(OP_CURRLINE, linenumber + 1); }
;

compoundifstmt: ifexpr THEN compoundstmt 
        { 
	  // if there is an if branch or jump on the iftable stack get where it is
	  // in the bytecode array and then put the current bytecode address there
	  // - so we can jump over code
	  if (numifs>0) 
	    { 
		  numifs--;
	      unsigned int *temp = (void *) byteCode + iftable[numifs];
	      *temp = byteOffset; 
	    } 
	}
;

ifstmt: ifexpr THEN 
	{
		// there is nothing to do with a multi line if (ifexp handles it)
	}
;

elsestmt: ELSE 
	{ 
		// on else create a jump point to the endif
		addIntOp(OP_PUSHINT, 0);	// false - always jump before else to endif
		addOp(OP_BRANCH);
		unsigned int elsegototemp = addInt(0);
		// resolve the false jump on the if to the current location
		if (numifs>0) 
			{ 
				numifs--;
				unsigned int *temp = (void *) byteCode + iftable[numifs];
				*temp = byteOffset; 
			} 
		// now add the elsegoto jump to the iftable
		iftable[numifs] = elsegototemp;
		numifs++;
	}
;

endifexpr:	ENDIF|
	END IF;
	
endifstmt: endifexpr 
	{ 
		// if there is an if branch or jump on the iftable stack get where it is
		// in the bytecode array and then put the current bytecode address there
		// - so we can jump over code
		if (numifs>0) 
			{ 
				numifs--;
				unsigned int *temp = (void *) byteCode + iftable[numifs];
				*temp = byteOffset; 
			} 
	}
;

whilestmt: WHILE floatexpr 
         { 
		 // create temp 
	   //if true, don't branch. If false, go to next line do the loop.
	   addOp(OP_BRANCH);
	   // after branch add a placeholder for the final end of the loop
	   // it will be resolved in the endwhile statement, push the
	   // location of this location on the iftable
	   iftable[numifs] = addInt(0);
	   numifs++;
         }
;

endwhileexpr:	ENDWHILE|
	END WHILE;
	
endwhilestmt: endwhileexpr 
	{ 
		// there should be two bytecode locations.  the TOP is the
		// location to jump to at the top of the loopthe , TOP-1 is the location
		// the exit jump needs to be written back to jump point on WHILE
		if (numifs>1) 
			{ 
				addIntOp(OP_PUSHINT, 0);	// false - always jump back to the beginning
				addIntOp(OP_BRANCH, iftable[numifs-1]);
				// resolve the false jump on the while to the current location
				unsigned int *temp = (void *) byteCode + iftable[numifs-2];
				*temp = byteOffset; 
				numifs-=2;
			} 
	}
;

dostmt: DO 
         { 
		 // need nothing done at top of a do
         }
;


untilstmt: UNTIL floatexpr 
         { 
		 // create temp 
	   //if If false, go to to the corresponding do.
		if (numifs>0) 
			{ 
				addIntOp(OP_BRANCH, iftable[numifs-1]);
				numifs--;
			} 
         }

compoundstmt: statement | compoundstmt ':' statement
;

statement: gotostmt
         | gosubstmt
         | returnstmt
         | printstmt
         | plotstmt
         | circlestmt
         | rectstmt
         | polystmt
         | stampstmt
         | linestmt
         | numassign
         | stringassign
         | forstmt
         | nextstmt
         | colorstmt
         | inputstmt
         | endstmt
         | clearstmt
         | refreshstmt
         | fastgraphicsstmt
         | graphsizestmt
         | dimstmt
         | pausestmt
         | arrayassign
         | strarrayassign
         | openstmt
         | writestmt
         | writelinestmt
         | closestmt
         | resetstmt
         | soundstmt
         | textstmt
	 | fontstmt
	 | saystmt
	 | systemstmt
	 | volumestmt
	 | wavplaystmt
	 | wavstopstmt
	 	| seekstmt
	 	| clickclearstmt
;

dimstmt: DIM VARIABLE floatexpr { addIntOp(OP_DIM, $2); }		// parens added by single floatexpr
       | DIM STRINGVAR floatexpr { addIntOp(OP_DIMSTR, $2); }		// parens added by single floatexpr
       | DIM VARIABLE '(' floatexpr ',' floatexpr ')' { addIntOp(OP_DIM2D, $2); }
       | DIM STRINGVAR '(' floatexpr ',' floatexpr ')' { addIntOp(OP_DIMSTR2D, $2); }
;

pausestmt: PAUSE floatexpr { addOp(OP_PAUSE); }
;

clearstmt: CLS { addOp(OP_CLS); }
         | CLG { addOp(OP_CLG); } 
;

fastgraphicsstmt: FASTGRAPHICS { addOp(OP_FASTGRAPHICS); }
;

graphsizestmt: GRAPHSIZE floatexpr ',' floatexpr { addOp(OP_GRAPHSIZE); }
             | GRAPHSIZE '(' floatexpr ',' floatexpr ')' { addOp(OP_GRAPHSIZE); }
;

refreshstmt: REFRESH { addOp(OP_REFRESH); }
;

endstmt: END { addOp(OP_END); }
;

ifexpr: IF floatexpr 
         { 
	   //if true, don't branch. If false, go to next line.
	   addOp(OP_BRANCH);
	   // after branch add a placeholder for the final end of the if
	   // it will be resolved in the if/else/endif statement, push the
	   // location of this location on the iftable
	   checkByteMem(sizeof(int));
	   iftable[numifs] = byteOffset;
	   numifs++;
	   byteOffset += sizeof(int);
         }
;

strarrayassign: STRINGVAR '[' floatexpr ']' '=' stringexpr { addIntOp(OP_STRARRAYASSIGN, $1); }
	| STRINGVAR '[' floatexpr ',' floatexpr ']' '=' stringexpr { addIntOp(OP_STRARRAYASSIGN2D, $1); }
        | STRINGVAR '=' immediatestrlist { addInt2Op(OP_STRARRAYLISTASSIGN, $1, listlen); listlen = 0; }
;

arrayassign: VARIABLE '[' floatexpr ']' '=' floatexpr { addIntOp(OP_ARRAYASSIGN, $1); }
	| VARIABLE '[' floatexpr ',' floatexpr ']' '=' floatexpr { addIntOp(OP_ARRAYASSIGN2D, $1); }
        | VARIABLE '=' immediatelist { addInt2Op(OP_ARRAYLISTASSIGN, $1, listlen); listlen = 0; }
;


numassign: VARIABLE '=' floatexpr { addIntOp(OP_NUMASSIGN, $1); }
;

stringassign: STRINGVAR '=' stringexpr { addIntOp(OP_STRINGASSIGN, $1); }
;

forstmt: FOR VARIABLE '=' floatexpr TO floatexpr 
          { 
	    addIntOp(OP_PUSHINT, 1); //step
	    addIntOp(OP_FOR, $2);
	  }
       | FOR VARIABLE '=' floatexpr TO floatexpr STEP floatexpr
          { 
	    addIntOp(OP_FOR, $2);
	  }
;


nextstmt: NEXT VARIABLE { addIntOp(OP_NEXT, $2); }
;

gotostmt: GOTO VARIABLE     { addIntOp(OP_GOTO, $2); }
;

gosubstmt: GOSUB VARIABLE   { addIntOp(OP_GOSUB, $2); }
;

returnstmt: RETURN          { addOp(OP_RETURN); }
;

colorstmt: SETCOLOR floatexpr ',' floatexpr ',' floatexpr { addOp(OP_SETCOLORRGB); }
         | SETCOLOR '(' floatexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_SETCOLORRGB); }
         | SETCOLOR floatexpr { addOp(OP_SETCOLORINT); }
         | SETCOLOR '(' floatexpr  ')' { addOp(OP_SETCOLORINT); }
;

soundstmt: SOUND VARIABLE { addIntOp(OP_SOUND_ARRAY, $2); }
         | SOUND '(' VARIABLE ')' { addIntOp(OP_SOUND_ARRAY, $3); }
         | SOUND immediatelist { addIntOp(OP_SOUND_LIST, listlen); listlen=0; }
		 | SOUND '(' floatexpr ',' floatexpr ')' { addOp(OP_SOUND); }
         | SOUND floatexpr ',' floatexpr         { addOp(OP_SOUND); }
;

plotstmt: PLOT floatexpr ',' floatexpr { addOp(OP_PLOT); }
        | PLOT '(' floatexpr ',' floatexpr ')' { addOp(OP_PLOT); }
;

linestmt: LINE floatexpr ',' floatexpr ',' floatexpr ',' floatexpr { addOp(OP_LINE); }
        | LINE '(' floatexpr ',' floatexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_LINE); }
;


circlestmt: CIRCLE floatexpr ',' floatexpr ',' floatexpr { addOp(OP_CIRCLE); }
          | CIRCLE '(' floatexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_CIRCLE); }
;

rectstmt: RECT floatexpr ',' floatexpr ',' floatexpr ',' floatexpr { addOp(OP_RECT); }
          | RECT '(' floatexpr ',' floatexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_RECT); }
;

textstmt: TEXT floatexpr ',' floatexpr ',' stringexpr { addOp(OP_TEXT); }
          | TEXT '(' floatexpr ',' floatexpr ',' stringexpr ')' { addOp(OP_TEXT); }
		  | TEXT floatexpr ',' floatexpr ',' floatexpr { addOp(OP_TEXT); }
          | TEXT '(' floatexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_TEXT); }
;

fontstmt: FONT stringexpr ',' floatexpr ',' floatexpr { addOp(OP_FONT); }
          | FONT '(' stringexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_FONT); }
;

saystmt: SAY stringexpr { addOp(OP_SAY); }
         | SAY '(' stringexpr ')' { addOp(OP_SAY); }
         | SAY floatexpr  { addOp(OP_SAY); } 		// parens added by single floatexpr
;

systemstmt: SYSTEM stringexpr { addOp(OP_SYSTEM); }
         | SYSTEM '(' stringexpr ')' { addOp(OP_SYSTEM); }
;

volumestmt: VOLUME floatexpr { addOp(OP_VOLUME); } 		// parens added by single floatexpr
;

polystmt: POLY VARIABLE { addIntOp(OP_POLY, $2); }
          | POLY '(' VARIABLE ')' { addIntOp(OP_POLY, $3); }
          | POLY immediatelist { addIntOp(OP_POLY_LIST, listlen); listlen=0; }
;

stampstmt: STAMP floatexpr ',' floatexpr ',' floatexpr ',' VARIABLE { addFloatOp(OP_PUSHFLOAT, 0); addIntOp(OP_STAMP, $8); }
        | STAMP '(' floatexpr ',' floatexpr ',' floatexpr ',' VARIABLE ')' { addFloatOp(OP_PUSHFLOAT, 0); addIntOp(OP_STAMP, $9); }
        | STAMP floatexpr ',' floatexpr ',' floatexpr ',' immediatelist { addIntOp(OP_STAMP_S_LIST, listlen); listlen=0; }
	| STAMP floatexpr ',' floatexpr ',' VARIABLE { addFloatOp(OP_PUSHFLOAT, 1); addFloatOp(OP_PUSHFLOAT, 0); addIntOp(OP_STAMP, $6); }
        | STAMP '(' floatexpr ',' floatexpr ',' VARIABLE ')' { addFloatOp(OP_PUSHFLOAT, 1); addFloatOp(OP_PUSHFLOAT, 0); addIntOp(OP_STAMP, $7); }
        | STAMP floatexpr ',' floatexpr ',' immediatelist { addIntOp(OP_STAMP_LIST, listlen); listlen=0; }
	| STAMP floatexpr ',' floatexpr ','  floatexpr ',' floatexpr ',' VARIABLE { addIntOp(OP_STAMP, $10); }
        | STAMP '(' floatexpr ',' floatexpr ',' floatexpr ',' floatexpr ',' VARIABLE ')' { addIntOp(OP_STAMP, $11); }
        | STAMP floatexpr ',' floatexpr ',' floatexpr ',' floatexpr ',' immediatelist { addIntOp(OP_STAMP_SR_LIST, listlen); listlen=0; }
;

openstmt: OPEN '(' stringexpr ')' { addOp(OP_OPEN); } 
        | OPEN stringexpr         { addOp(OP_OPEN); }
;

writestmt: WRITE '(' stringexpr ')' { addOp(OP_WRITE); }
         | WRITE stringexpr         { addOp(OP_WRITE); }
;

writelinestmt: WRITELINE '(' stringexpr ')' { addOp(OP_WRITELINE); }
         | WRITELINE stringexpr         { addOp(OP_WRITELINE); }
;

closestmt: CLOSE         { addOp(OP_CLOSE); }
         | CLOSE '(' ')' { addOp(OP_CLOSE); }
;

resetstmt: RESET         { addOp(OP_RESET); }
         | RESET '(' ')' { addOp(OP_RESET); }
;

inputstmt: inputexpr ',' STRINGVAR  { addIntOp(OP_STRINGASSIGN, $3); }
         | inputexpr ',' STRINGVAR '[' floatexpr ']'  { addOp(OP_STACKSWAP); addIntOp(OP_STRARRAYASSIGN, $3); }
         | inputexpr ',' VARIABLE  { addIntOp(OP_NUMASSIGN, $3); }
         | inputexpr ',' VARIABLE '[' floatexpr ']'  { addOp(OP_STACKSWAP); addIntOp(OP_ARRAYASSIGN, $3); }
		 | INPUT STRINGVAR  { addOp(OP_INPUT); addIntOp(OP_STRINGASSIGN, $2); }
		 | INPUT STRINGVAR '[' floatexpr ']'  { addOp(OP_INPUT); addIntOp(OP_STRARRAYASSIGN, $2); }
		 | INPUT STRINGVAR '[' floatexpr ',' floatexpr ']'  { addOp(OP_INPUT); addIntOp(OP_STRARRAYASSIGN2D, $2); }
		 | INPUT VARIABLE  { addOp(OP_INPUT); addIntOp(OP_NUMASSIGN, $2); }
		 | INPUT VARIABLE '[' floatexpr ']'  { addOp(OP_INPUT); addIntOp(OP_ARRAYASSIGN, $2); }
		 | INPUT VARIABLE '[' floatexpr ',' floatexpr ']'  { addOp(OP_INPUT); addIntOp(OP_ARRAYASSIGN2D, $2); }
;

inputexpr: INPUT stringexpr { addOp(OP_PRINT);  addOp(OP_INPUT); }
         | INPUT '(' stringexpr ')' { addOp(OP_PRINT);  addOp(OP_INPUT); }
;

printstmt: PRINT { addStringOp(OP_PUSHSTRING, ""); addOp(OP_PRINTN); }
	 | PRINT stringexpr { addOp(OP_PRINTN); }
         | PRINT '(' stringexpr ')' { addOp(OP_PRINTN); }
         | PRINT floatexpr  { addOp(OP_PRINTN); }
         | PRINT stringexpr ';' { addOp(OP_PRINT); }
         | PRINT '(' stringexpr ')' ';' { addOp(OP_PRINT); }
         | PRINT floatexpr  ';' { addOp(OP_PRINT); }
;

wavplaystmt: WAVPLAY stringexpr  {addOp(OP_WAVPLAY);  }
         | WAVPLAY '(' stringexpr ')' { addOp(OP_WAVPLAY); }
;

wavstopstmt: WAVSTOP         { addOp(OP_WAVSTOP); }
         | WAVSTOP '(' ')' { addOp(OP_WAVSTOP); }
;

seekstmt: SEEK floatexpr  {addOp(OP_SEEK);  } 		// parens added by single floatexpr
;

clickclearstmt: CLICKCLEAR  {addOp(OP_CLICKCLEAR);  }
         | CLICKCLEAR '(' ')' { addOp(OP_CLICKCLEAR); }
;

immediatestrlist: '{' stringlist '}'
;

immediatelist: '{' floatlist '}'
;

floatlist: floatexpr { listlen = 1; }
         | floatexpr ',' floatlist { listlen++; }
;

floatexpr: '(' floatexpr ')' { $$ = $2; }
         | floatexpr '+' floatexpr { addOp(OP_ADD); }
         | floatexpr '-' floatexpr { addOp(OP_SUB); }
         | floatexpr '*' floatexpr { addOp(OP_MUL); }
         | floatexpr MOD floatexpr { addOp(OP_MOD); }
         | floatexpr '/' floatexpr { addOp(OP_DIV); }
         | floatexpr '^' floatexpr { addOp(OP_EXP); }
         | '-' FLOAT %prec UMINUS  { addFloatOp(OP_PUSHFLOAT, -$2); }
         | '-' INTEGER %prec UMINUS { addIntOp(OP_PUSHINT, -$2); }
         | '-' VARIABLE %prec UMINUS 
           { 
	     if ($2 < 0)
	       {
		 return -1;
	       }
	     else
	       {
		 addIntOp(OP_PUSHVAR, $2);
		 addOp(OP_NEGATE);
	       }
	   }
       | floatexpr AND floatexpr {addOp(OP_AND); }
       | floatexpr OR floatexpr { addOp(OP_OR); }
       | floatexpr XOR floatexpr { addOp(OP_XOR); }
       | NOT floatexpr %prec UMINUS { addOp(OP_NOT); }
        | stringexpr '=' stringexpr  { addOp(OP_EQUAL); } 
        | stringexpr NE stringexpr   { addOp(OP_NEQUAL); }
        | stringexpr '<' stringexpr    { addOp(OP_LT); }
        | stringexpr '>' stringexpr    { addOp(OP_GT); }
        | stringexpr GTE stringexpr    { addOp(OP_GTE); }
        | stringexpr LTE stringexpr    { addOp(OP_LTE); }
		| floatexpr '=' floatexpr    { addOp(OP_EQUAL); }
        | floatexpr NE floatexpr     { addOp(OP_NEQUAL); }
        | floatexpr '<' floatexpr    { addOp(OP_LT); }
        | floatexpr '>' floatexpr    { addOp(OP_GT); }
        | floatexpr GTE floatexpr    { addOp(OP_GTE); }
        | floatexpr LTE floatexpr    { addOp(OP_LTE); }
         | FLOAT   { addFloatOp(OP_PUSHFLOAT, $1); }
         | INTEGER { addIntOp(OP_PUSHINT, $1); }
         | KEY     { addOp(OP_KEY); }
         | VARIABLE '[' floatexpr ']' { addIntOp(OP_DEREF, $1); }
         | VARIABLE '[' floatexpr ',' floatexpr ']' { addIntOp(OP_DEREF2D, $1); }
         | VARIABLE 
           { 
	     if ($1 < 0)
	       {
		 return -1;
	       }
	     else
	       {
		 addIntOp(OP_PUSHVAR, $1);
	       }
	   }
         | TOINT '(' floatexpr ')' { addOp(OP_INT); }
         | TOINT '(' stringexpr ')' { addOp(OP_INT); }
         | TOFLOAT '(' floatexpr ')' { addOp(OP_FLOAT); }
         | TOFLOAT '(' stringexpr ')' { addOp(OP_FLOAT); }
         | LENGTH '(' stringexpr ')' { addOp(OP_LENGTH); }
         | ASC '(' stringexpr ')' { addOp(OP_ASC); }
         | INSTR '(' stringexpr ',' stringexpr ')' { addOp(OP_INSTR); }
         | CEIL '(' floatexpr ')' { addOp(OP_CEIL); }
         | FLOOR '(' floatexpr ')' { addOp(OP_FLOOR); }
         | SIN '(' floatexpr ')' { addOp(OP_SIN); }
         | COS '(' floatexpr ')' { addOp(OP_COS); }
         | TAN '(' floatexpr ')' { addOp(OP_TAN); }
         | ASIN '(' floatexpr ')' { addOp(OP_ASIN); }
         | ACOS '(' floatexpr ')' { addOp(OP_ACOS); }
         | ATAN '(' floatexpr ')' { addOp(OP_ATAN); }
         | ABS '(' floatexpr ')' { addOp(OP_ABS); }
         | RAND { addOp(OP_RAND); }
         | PI { addFloatOp(OP_PUSHFLOAT, 3.14159265); }
         | BOOLTRUE { addIntOp(OP_PUSHINT, 1); }
         | BOOLFALSE { addIntOp(OP_PUSHINT, 0); }
         | BOOLEOF                    { addOp(OP_EOF); }
         | EXISTS '(' stringexpr ')' { addOp(OP_EXISTS); }
         | YEAR { addOp(OP_YEAR); }
         | MONTH { addOp(OP_MONTH); }
         | DAY { addOp(OP_DAY); }
         | HOUR { addOp(OP_HOUR); }
         | MINUTE { addOp(OP_MINUTE); }
         | SECOND { addOp(OP_SECOND); }
         | GRAPHWIDTH { addOp(OP_GRAPHWIDTH); }
         | GRAPHHEIGHT { addOp(OP_GRAPHHEIGHT); }
         | SIZE { addOp(OP_SIZE); }
         | MOUSEX { addOp(OP_MOUSEX); }
         | MOUSEY { addOp(OP_MOUSEY); }
         | MOUSEB { addOp(OP_MOUSEB); }
         | CLICKX { addOp(OP_CLICKX); }
         | CLICKY { addOp(OP_CLICKY); }
         | CLICKB { addOp(OP_CLICKB); }
         | CLEAR { addIntOp(OP_PUSHINT, 0xffffff); }
         | BLACK { addIntOp(OP_PUSHINT, 0x000000); }
         | WHITE { addIntOp(OP_PUSHINT, 0xf8f8f8); }
         | RED { addIntOp(OP_PUSHINT, 0xff0000); }
         | DARKRED { addIntOp(OP_PUSHINT, 0x800000); }
         | GREEN { addIntOp(OP_PUSHINT, 0x00ff00); }
         | DARKGREEN { addIntOp(OP_PUSHINT, 0x008000); }
         | BLUE { addIntOp(OP_PUSHINT, 0x0000ff); }
         | DARKBLUE { addIntOp(OP_PUSHINT, 0x000080); }
         | CYAN { addIntOp(OP_PUSHINT, 0x00ffff); }
         | DARKCYAN { addIntOp(OP_PUSHINT, 0x008080); }
         | PURPLE { addIntOp(OP_PUSHINT, 0xff00ff); }
         | DARKPURPLE { addIntOp(OP_PUSHINT, 0x800080); }
         | YELLOW { addIntOp(OP_PUSHINT, 0xffff00); }
         | DARKYELLOW { addIntOp(OP_PUSHINT, 0x808000); }
         | ORANGE { addIntOp(OP_PUSHINT, 0xff6600); }
         | DARKORANGE { addIntOp(OP_PUSHINT, 0xaa3300); }
         | GREY { addIntOp(OP_PUSHINT, 0xa4a4a4); }
         | DARKGREY { addIntOp(OP_PUSHINT, 0x808080); }
		 | PIXEL '(' floatexpr ',' floatexpr ')' { addOp(OP_PIXEL); }
		 | RGB '(' floatexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_RGB); }
		 | GETCOLOR { addOp(OP_GETCOLOR); }
		 
  ;

stringlist: stringexpr { listlen = 1; }
          | stringexpr ',' stringlist { listlen++; }
;

stringexpr: stringexpr '+' stringexpr     { addOp(OP_CONCAT); }
          | floatexpr '+' stringexpr     { addOp(OP_CONCAT); }
          | stringexpr '+' floatexpr     { addOp(OP_CONCAT); }
          | STRING    { addStringOp(OP_PUSHSTRING, $1); }
          | STRINGVAR '[' floatexpr ']' { addIntOp(OP_DEREF, $1); }
          | STRINGVAR '[' floatexpr ',' floatexpr ']' { addIntOp(OP_DEREF2D, $1); }
          | STRINGVAR 
            { 
	      if ($1 < 0)
		{
		  return -1;
		}
	      else
		{
		  addIntOp(OP_PUSHVAR, $1);
		}
	    }
          | CHR '(' floatexpr ')' { addOp(OP_CHR); }
          | TOSTRING '(' floatexpr ')' { addOp(OP_STRING); }
          | UPPER '(' stringexpr ')' { addOp(OP_UPPER); }
          | LOWER '(' stringexpr ')' { addOp(OP_LOWER); }
          | MID '(' stringexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_MID); }
          | LEFT '(' stringexpr ',' floatexpr ')' { addOp(OP_LEFT); }
          | RIGHT '(' stringexpr ',' floatexpr ')' { addOp(OP_RIGHT); }
          | READ '(' ')' { addOp(OP_READ); }
          | READ { addOp(OP_READ); }
          | READLINE '(' ')' { addOp(OP_READLINE); }
          | READLINE { addOp(OP_READLINE); }
;

%%

int
yyerror(const char *msg) {
  errorcode = -1;
  if (yytext[0] == '\n') { linenumber--; }	// error happened on previous line
  return -1;
}

