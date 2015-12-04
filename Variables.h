// implement variables using a c++ map of the variable number and a pointer to the variable structure
// this will allow for a dynamically allocated number of variables
// 2010-12-13 j.m.reneau

#pragma once

#include <map>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <QString>

#include "Error.h"
#include "DataElement.h"

#define VARIABLE_MAXARRAYELEMENTS 1048576
#define MAX_RECURSE_LEVELS	1048576

  
class VariableArrayPart
{
	public:
		int xdim;
		int ydim;
		int size;
		std::map<int,DataElement*> datamap;
};

class Variable
{
	public:
		Variable();
		~Variable();
		
		DataElement *data;
		VariableArrayPart *arr;
};

class Variables
{
	public:
		Variables(Error *);
		~Variables();
		//
		QString debug();
		void clear();
		void increaserecurse();
		void decreaserecurse();
		int getrecurse();
		//
		int type(int);
		//
		DataElement *getdata(int);
		void setdata(int, DataElement *);
 		//
		void arraydim(int, int, int, bool);
		DataElement* arraygetdata(int, int, int);
		void arraysetdata(int, int, int, DataElement *);
		//
		int arraysize(int);
		int arraysizex(int);
		int arraysizey(int);
		//
		void makeglobal(int);


	private:
		Error *error;
		int recurselevel;
		std::map<int, std::map<int,Variable*> > varmap;
		std::map<int, bool> globals;
		Variable* get(int);
		void clearvariable(Variable *);
		bool isglobal(int);

};
