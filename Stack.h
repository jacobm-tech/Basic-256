#pragma once

enum b_type {T_INT, T_FLOAT, T_STRING, T_BOOL, T_ARRAY, T_STRARRAY, T_UNUSED};


class stackval
{
 public:
  stackval *next;
  b_type type;
  union {
    char *string;
    int intval;
    double floatval; 
  } value;
  stackval();
  ~stackval();
};


class Stack
{
 public:
  Stack();
  ~Stack();
  void push(char *);
  void push(int);
  void push(double);
  void swap();
  stackval *pop();
  int popint();
  double popfloat();
  char *popstring();
  
 private:
  stackval *top;
};