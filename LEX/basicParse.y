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

    extern int yylex();
    extern char *yytext;
    int yyerror(const char *);
    int errorcode;
    extern int column;
    extern int linenumber;

    char *byteCode = NULL;
    unsigned int byteOffset = 0;
    unsigned int oldByteOffset = 0;
    unsigned int listlen = 0;

    struct label 
    {
      char *name;
      int offset;
    };

    unsigned int branchAddr = 0;

    char *EMPTYSTR = "";
    char *symtable[SYMTABLESIZE];
    int labeltable[SYMTABLESIZE];
    int numsyms = 0;
    int numlabels = 0;
    unsigned int maxbyteoffset = 0;

    int
    basicParse(char *);

    void 
    clearLabelTable()
    {
      int j;
      for (j = 0; j < SYMTABLESIZE; j++)
	{
	  labeltable[j] = -1;
	}
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
%token PLOT CIRCLE RECT POLY LINE FASTGRAPHICS GRAPHSIZE REFRESH CLS CLG
%token IF THEN FOR TO STEP NEXT 
%token OPEN READ WRITE CLOSE RESET
%token GOTO GOSUB RETURN REM END SETCOLOR
%token GTE LTE NE
%token DIM NOP
%token TOINT TOSTRING LENGTH MID INSTR
%token CEIL FLOOR RAND SIN COS TAN ABS PI
%token AND OR XOR NOT
%token PAUSE SOUND
%token ASC CHR TOFLOAT READLINE WRITELINE BOOLEOF MOD
%token YEAR MONTH DAY HOUR MINUTE SECOND TEXT FONT
%token SAY
%token GRAPHWIDTH GRAPHHEIGHT
%token WAVPLAY WAVSTOP


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

%left '-'
%left '+'
%left '*'
%left MOD
%left '/'
%left '^'
%left AND 
%left OR 
%left XOR 
%right '='
%nonassoc UMINUS


%%

program: validline '\n'         
       | validline '\n' program 
;

validline: ifstmt       { addIntOp(OP_CURRLINE, linenumber); }
         | compoundstmt { addIntOp(OP_CURRLINE, linenumber); }
         | /*empty*/    { addIntOp(OP_CURRLINE, linenumber); }
         | LABEL        { labeltable[$1] = byteOffset; addIntOp(OP_CURRLINE, linenumber + 1); }
;

ifstmt: ifexpr THEN compoundstmt 
        { 
	  if (branchAddr) 
	    { 
	      unsigned int *temp = (void *) byteCode + branchAddr;
	      *temp = byteOffset; 
	      branchAddr = 0; 
	    } 
	}
;

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
	 | wavplaystmt
	 | wavstopstmt
;

dimstmt: DIM VARIABLE '(' floatexpr ')'  { addIntOp(OP_DIM, $2); }
       | DIM STRINGVAR '(' floatexpr ')' { addIntOp(OP_DIMSTR, $2); }
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

ifexpr: IF compoundboolexpr 
         { 
	   //if true, don't branch. If false, go to next line.
	   addOp(OP_BRANCH);
	   checkByteMem(sizeof(int));
	   branchAddr = byteOffset;
	   byteOffset += sizeof(int);
         }
;

compoundboolexpr: boolexpr
                | compoundboolexpr AND compoundboolexpr {addOp(OP_AND); }
                | compoundboolexpr OR compoundboolexpr { addOp(OP_OR); }
                | compoundboolexpr XOR compoundboolexpr { addOp(OP_XOR); }
                | NOT compoundboolexpr %prec UMINUS { addOp(OP_NOT); }
                | '(' compoundboolexpr ')'
;

boolexpr: stringexpr '=' stringexpr  { addOp(OP_EQUAL); } 
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
        | BOOLEOF                    { addOp(OP_EOF); }
;

strarrayassign: STRINGVAR '[' floatexpr ']' '=' stringexpr { addIntOp(OP_STRARRAYASSIGN, $1); }
           | STRINGVAR '=' immediatestrlist { addInt2Op(OP_STRARRAYLISTASSIGN, $1, listlen); listlen = 0; }
;

arrayassign: VARIABLE '[' floatexpr ']' '=' floatexpr { addIntOp(OP_ARRAYASSIGN, $1); }
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

colorstmt: SETCOLOR COLOR   { addIntOp(OP_SETCOLOR, $2); }
         | SETCOLOR '(' COLOR ')' { addIntOp(OP_SETCOLOR, $3); }
         | SETCOLOR floatexpr ',' floatexpr ',' floatexpr { addOp(OP_SETCOLORRGB); }
         | SETCOLOR '(' floatexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_SETCOLORRGB); }
;

soundstmt: SOUND '(' floatexpr ',' floatexpr ')' { addOp(OP_SOUND); }
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
;

fontstmt: FONT stringexpr ',' floatexpr ',' floatexpr { addOp(OP_FONT); }
          | FONT '(' stringexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_FONT); }
;
saystmt: SAY stringexpr { addOp(OP_SAY); }
         | SAY '(' stringexpr ')' { addOp(OP_SAY); }
         | SAY floatexpr  { addOp(OP_SAY); }
         | SAY '(' floatexpr ')' { addOp(OP_SAY); }
;

polystmt: POLY VARIABLE ',' floatexpr { addIntOp(OP_POLY, $2); }
          | POLY '(' VARIABLE ',' floatexpr ')' { addIntOp(OP_POLY, $3); }
          | POLY VARIABLE { addIntOp(OP_POLY, $2); }
          | POLY '(' VARIABLE ')' { addIntOp(OP_POLY, $3); }
          | POLY immediatelist { addIntOp(OP_POLY, listlen); listlen=0; }
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
		 | INPUT VARIABLE  { addOp(OP_INPUT); addIntOp(OP_NUMASSIGN, $2); }
		 | INPUT VARIABLE '[' floatexpr ']'  { addOp(OP_INPUT); addIntOp(OP_ARRAYASSIGN, $2); }
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
         | FLOAT   { addFloatOp(OP_PUSHFLOAT, $1); }
         | INTEGER { addIntOp(OP_PUSHINT, $1); }
         | KEY     { addOp(OP_KEY); }
         | VARIABLE '[' floatexpr ']' { addIntOp(OP_DEREF, $1); }
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
         | ABS '(' floatexpr ')' { addOp(OP_ABS); }
         | RAND { addOp(OP_RAND); }
         | PI { addFloatOp(OP_PUSHFLOAT, 3.14159265); }
         | YEAR { addOp(OP_YEAR); }
         | MONTH { addOp(OP_MONTH); }
         | DAY { addOp(OP_DAY); }
         | HOUR { addOp(OP_HOUR); }
         | MINUTE { addOp(OP_MINUTE); }
         | SECOND { addOp(OP_SECOND); }
         | GRAPHWIDTH { addOp(OP_GRAPHWIDTH); }
         | GRAPHHEIGHT { addOp(OP_GRAPHHEIGHT); }
;

stringlist: stringexpr { listlen = 1; }
          | stringexpr ',' stringlist { listlen++; }
;

stringexpr: stringexpr '+' stringexpr     { addOp(OP_CONCAT); }
          | floatexpr '+' stringexpr     { addOp(OP_CONCAT); }
          | stringexpr '+' floatexpr     { addOp(OP_CONCAT); }
          | STRING    { addStringOp(OP_PUSHSTRING, $1); }
          | STRINGVAR '[' floatexpr ']' { addIntOp(OP_DEREF, $1); }
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
          | MID '(' stringexpr ',' floatexpr ',' floatexpr ')' { addOp(OP_MID); }
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

