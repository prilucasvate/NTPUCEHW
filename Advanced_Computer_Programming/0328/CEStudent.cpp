#include <iostream>
#include "CEStudent.h"
using namespace std;

bool CEStudent::getCPE() const{
	return CPE;
}
void CEStudent::setCPE(bool c) {
	CPE=c;
}
bool CEStudent::graduationRequirement() const{
    return (CPE==1)&&(getGPA()>=2.0);
}
