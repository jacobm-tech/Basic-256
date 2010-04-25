#include "Stack.h"
#include <string>

stackval::stackval()
{
	next = NULL;
	type = T_UNUSED;
	value.floatval = 0;
}

stackval::~stackval()
{
	if (type == T_STRING && value.string != NULL)
	{
		free(value.string);
		value.string = NULL;
	}
}

Stack::Stack()
{
	top = NULL;
}

Stack::~Stack()
{
}

void
Stack::push(char *c)
{
	stackval *temp = new stackval;

	temp->next = NULL;
	temp->type = T_STRING;
	temp->value.string = strdup(c);
	if (top)
	{
		temp->next = top;
	}
	top = temp;
}

void
Stack::push(int i)
{
	stackval *temp = new stackval;

	temp->next = NULL;
	temp->type = T_INT;
	temp->value.intval = i;
	if (top)
	{
		temp->next = top;
	}
	top = temp;
}

void
Stack::push(double d)
{
	stackval *temp = new stackval;

	temp->next = NULL;
	temp->type = T_FLOAT;
	temp->value.floatval = d;
	if (top)
	{
		temp->next = top;
	}
	top = temp;
}

stackval *
Stack::pop()
{
	stackval *temp = top;
	if (top)
	{
		top = top->next;
	}
	return temp;
}

void Stack::swap()
{
	// swap top two elements
	stackval *one = top;
	top = top->next;
	stackval *two = top;
	top = top->next;
	one->next = top;
	two->next = one;
	top = two;
}

int Stack::popint()
{
	stackval *temp = top;
	int i=0;
	if (top) {
		top = top->next;
	}
	if (temp->type == T_INT) {
		i = temp->value.intval;
	}
	else if (temp->type == T_FLOAT) {
		i = (int) temp->value.floatval;
	}
	else if (temp->type == T_STRING)
	{
		i = (int) atoi(temp->value.string);
	}
	delete temp;
	return i;
}

double Stack::popfloat()
{
	stackval *temp = top;
	double f=0;
	if (top)
	{
		top = top->next;
	}
	if (temp->type == T_FLOAT) {
		f = temp->value.floatval;
	}
	else if (temp->type == T_INT)
	{
		f = (double) temp->value.intval;
	}
	else if (temp->type == T_STRING)
	{
		f = (double) atof(temp->value.string);
	}
	delete temp;
	return f;
}

char* Stack::popstring()
{
	// don't forget to free() the string returned by this function when you are dome with it
	char *s=NULL;
	stackval *temp = top;
	if (top)
	{
		top = top->next;
	}
	if (temp->type == T_STRING) {
		s = temp->value.string;
		temp->value.string = NULL;		// set to null so we don't destroy string when stack value is destructed
	}
	else if (temp->type == T_INT)
	{
		char buffer[64];
		sprintf(buffer, "%d", temp->value.intval);
		s = strdup(buffer);
	}
	else if (temp->type == T_FLOAT)
	{
		char buffer[64];
		sprintf(buffer, "%#lf", temp->value.floatval);
		// strip trailing zeros and decimal point
		while(buffer[strlen(buffer)-1]=='0') buffer[strlen(buffer)-1] = 0x00;
		if(buffer[strlen(buffer)-1]=='.') buffer[strlen(buffer)-1] = 0x00;
		// return
		s = strdup(buffer);
	}
	delete temp;
	return s;
}