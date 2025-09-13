#include <iostream>
#include "Student.h"
using namespace std;
Student::Student(string name)
	:studentName(name){
}
Student::Student(){
}
void  Student::printName() const{
    cout<<"Student's name is " <<getName();
}
string Student::getName() const{
	return studentName;
}
void Student::setName(string name) {
	studentName=name;
}