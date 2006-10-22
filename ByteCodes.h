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
#define OP_END        0x00
#define OP_NOP        0x01
#define OP_RETURN     0x02
#define OP_CONCAT     0x03

#define OP_EQUAL      0x04
#define OP_NEQUAL     0x05
#define OP_GT         0x06
#define OP_LT         0x07
#define OP_GTE        0x08
#define OP_LTE        0x09
#define OP_AND        0x0a
#define OP_NOT        0x0b
#define OP_OR         0x0c
#define OP_XOR        0x0d

#define OP_INT        0x0e
#define OP_STRING     0x0f

#define OP_ADD        0x10
#define OP_SUB        0x11
#define OP_MUL        0x12
#define OP_DIV        0x13
#define OP_EXP        0x14
#define OP_NEGATE     0x15

#define OP_PRINT      0x16
#define OP_PRINTN     0x17
#define OP_INPUT      0x18
#define OP_KEY        0x19

#define OP_PLOT       0x1a
#define OP_RECT       0x1b
#define OP_CIRCLE     0x1c

#define OP_REFRESH      0x1d
#define OP_FASTGRAPHICS 0x1e
#define OP_CLS          0x1f
#define OP_CLG          0x20

#define OP_SIN  0x21
#define OP_COS  0x22
#define OP_TAN  0x23
#define OP_RAND 0x24
#define OP_CEIL  0x25
#define OP_FLOOR 0x26
#define OP_ABS   0x27


//Int argument ops
#define OP_GOTO           0x41
#define OP_GOSUB          0x42
#define OP_BRANCH         0x43
#define OP_SETCOLOR       0x44
#define OP_NUMASSIGN      0x45
#define OP_STRINGASSIGN   0x46
#define OP_ARRAYASSIGN    0x47
#define OP_STRARRAYASSIGN 0x48
#define OP_PUSHVAR        0x49
#define OP_PUSHINT        0x4a
#define OP_DEREF          0x4b
#define OP_FOR            0x4c
#define OP_NEXT           0x4d

//2 Int argument ops
#define OP_DIM      0x60
#define OP_DIMSTR   0x61


//Float argument ops
#define OP_PUSHFLOAT  0x70


//String argument ops
#define OP_PUSHSTRING 0x80




