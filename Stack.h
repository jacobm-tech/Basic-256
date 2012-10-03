#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


enum b_type {T_INT, T_FLOAT, T_STRING, T_BOOL, T_ARRAY, T_STRARRAY, T_UNUSED};



typedef struct
{
  b_type type;
  union {
    char *string;
    int intval;
    double floatval; 
  } value;
} stackval;


class Stack
{
 public:
  Stack();
  ~Stack();
  void push(char *);
  void push(int);
  void push(double);
  void swap();
  void topto2();
  int peekType();
  stackval *pop();
  int popint();
  double popfloat();
  char *popstring();
  int toint(stackval *);
  double tofloat(stackval *);
  char *tostring(stackval *);
  void clean(stackval *);
  void clear();

  static const int defaultFToAMask = 6;
  int fToAMask;

 private:
  static const int initialSize = 64;
  stackval *top;
  stackval *bottom;
  stackval *limit;
  void checkLimit();
};
