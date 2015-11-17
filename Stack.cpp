#include "Stack.h"
#include "Types.h"
#include <string>

// be doubly sure that when a stack element (DataElement) is popped from
// the stack and NOT PUT BACK it MUST be deleted to keep from causing a memory
// leak!!!!  - 2013-03-25 j.m.reneau

Stack::Stack() {
    errornumber = ERROR_NONE;
    typeconverror = SETTINGSTYPECONVDEFAULT;
    decimaldigits = SETTINGSDECDIGSDEFAULT;
}

Stack::~Stack() {
    clear();
}


void Stack::settypeconverror(int e) {
// TypeConvError	0- report no errors
// 					1 - report problems as warning
// 					2 - report problems as an error
	typeconverror = e;
}


void Stack::setdecimaldigits(int e) {
// DecimalDigits	when converting a double to a string how many decinal
//					digits will we display default 12 maximum should not
//					exceed 15, 14 to be safe
	decimaldigits = e;
}

void
Stack::clear() {
    DataElement *ele;
    while(!stacklist.empty()) {
        ele = stacklist.front();
        stacklist.pop_front();
        delete(ele);
    }
    errornumber = ERROR_NONE;
}

QString Stack::debug() {
    // return a string representing the stack
    QString s("");
    DataElement *ele;
    for (std::list<DataElement*>::iterator it = stacklist.begin(); it != stacklist.end(); it++) {
        ele = *it;
        if(ele->type==T_FLOAT) s += "float(" + QString::number(ele->floatval) + ") ";
        if(ele->type==T_STRING) s += "string(" + ele->stringval + ") ";
        if(ele->type==T_ARRAY) s += "array=" + QString::number(ele->floatval) + " ";
        if(ele->type==T_STRARRAY) s += "strarray=" + QString::number(ele->floatval) + " ";
        if(ele->type==T_UNUSED) s += "unused ";
        if(ele->type==T_VARREF) s += "varref=" + QString::number(ele->floatval) + " ";
        if(ele->type==T_VARREFSTR) s += "varrefstr=" + QString::number(ele->floatval) + " ";
    }
    return s;
}

int Stack::height() {
    // return the height of the stack in elements
    // magic of pointer math returns number of elements
    return stacklist.size();
}

int Stack::error() {
    // return error
    return errornumber;
}

void Stack::clearerror() {
    // clear error
    errornumber = ERROR_NONE;
}

void Stack::pushelement(b_type type, double floatval, QString stringval) {
	// create a new data element and push to stack
    DataElement *ele = new DataElement(type, floatval, stringval);
    stacklist.push_front(ele);
}

void Stack::pushelement(DataElement *source) {
	// create a new data element and push to stack
    DataElement *ele = new DataElement(source);
    stacklist.push_front(ele);
}


void
Stack::pushstring(QString string) {
    pushelement(T_STRING, 0, string);
}

void
Stack::pushint(int i) {
    pushelement(T_FLOAT, i, NULL);
}

void
Stack::pushfloat(double d) {
    pushelement(T_FLOAT, d, NULL);
}

int Stack::peekType() {
    if (stacklist.empty()) {
        errornumber = ERROR_STACKUNDERFLOW;
        return T_FLOAT;
    } else {
        DataElement *ele = stacklist.front();
        return ele->type;
    }
}

DataElement *Stack::popelement() {
    // pop an element but if there is not one on the stack
    // pop a zero and set the error to underflow
    DataElement *e;
    if (stacklist.empty()) {
        errornumber = ERROR_STACKUNDERFLOW;
        pushint(0);
    }
    e = stacklist.front();
    stacklist.pop_front();
    return e;
}

void Stack::swap2() {
    // swap top two pairs of elements
    DataElement *zero = popelement();
    DataElement *one = popelement();
    DataElement *two = popelement();
    DataElement *three = popelement();

    stacklist.push_front(one);
    stacklist.push_front(zero);
    stacklist.push_front(three);
    stacklist.push_front(two);
}

void Stack::swap() {
    // swap top two elements
    DataElement *zero = popelement();
    DataElement *one = popelement();
    stacklist.push_front(zero);
    stacklist.push_front(one);
}

void
Stack::topto2() {
    // move the top of the stack under the next two
    // 0, 1, 2, 3...  becomes 1, 2, 0, 3...
    DataElement *zero = popelement();
    DataElement *one = popelement();
    DataElement *two = popelement();
    stacklist.push_front(zero);
    stacklist.push_front(two);
    stacklist.push_front(one);
}

void Stack::dup() {
    // make copy of top
    DataElement *zero = popelement();
    DataElement *ele = new DataElement(zero);
    stacklist.push_front(ele);
    stacklist.push_front(zero);
}

void Stack::dup2() {
    DataElement *zero = popelement();
    DataElement *one = popelement();
    stacklist.push_front(one);
    stacklist.push_front(zero);
    // make copies of one and zero to dup
    DataElement *ele = new DataElement(one);
    stacklist.push_front(ele);
    ele = new DataElement(zero);
    stacklist.push_front(ele);
}

int Stack::popint() {
	bool ok;
    DataElement *top=popelement();
	int i = top->getint(&ok);
	if(!ok) {
		if (typeconverror==SETTINGSTYPECONVWARN) errornumber = WARNING_TYPECONV;
		if (typeconverror==SETTINGSTYPECONVERROR) errornumber = ERROR_TYPECONV;
	}
    delete(top);
    return i;
}

double Stack::popfloat() {
	bool ok;
    DataElement *top=popelement();
	double f = top->getfloat(&ok);
 	if(!ok) {
		if (typeconverror==SETTINGSTYPECONVWARN) errornumber = WARNING_TYPECONV;
		if (typeconverror==SETTINGSTYPECONVERROR) errornumber = ERROR_TYPECONV;
	}
   delete(top);
   return f;
}

QString Stack::popstring() {
    DataElement *top=popelement();
    QString s = top->getstring(NULL, decimaldigits);
    delete(top);
    return s;
}

int Stack::compareFloats(double one, double two) {
    // return 1 if one>two  0 if one==two or -1 if one<two
    // USE FOR ALL COMPARISON WITH NUMBERS
    // used a small number (epsilon) to make sure that
    // decimal precission errors are ignored
    if (fabs(one - two)<=BASIC256EPSILON) return 0;
    else if (one < two) return -1;
    else return 1;
}

int Stack::compareTopTwo() {
    // complex compare logic - compare two stack types with each other
    // return 1 if one>two  0 if one==two or -1 if one<two
    //
    DataElement *two = popelement();
    DataElement *one = popelement();
    if (one->type == T_STRING || two->type == T_STRING) {
        // one or both strings - [compare them as strings] strcmp
        QString stwo = two->getstring(NULL,decimaldigits);
        QString sone = one->getstring(NULL,decimaldigits);
        int ans = sone.compare(stwo);
        if (ans==0) return 0;
        else if (ans<0) return -1;
        else return 1;
    } else {
        // anything else - compare them as doubles
        double ftwo = two->getfloat();
        double fone = one->getfloat();
        return compareFloats(fone, ftwo);
    }
}
