#include <iostream>
#include "Student.h"
using namespace std;
Student::Student(string name)
	:studentName(name){
}
Student::Student(double GPA)
	:GPA(GPA){
}
Student::~Student(){
	cout<<"A Student object is destroyed!\n";
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
double Student::getGPA() const{
	return GPA;
}
void Student::computeGPA(int numberOfAs, int numberOfBs, int numberOfCs, int numberOfDs, int numberOfFs) {
	GPA=(4.0*numberOfAs+3.0*numberOfBs+2.0*numberOfCs+1.0*numberOfDs)/(numberOfAs+numberOfBs+numberOfCs+numberOfDs+numberOfFs);
}
void  Student::printGPA() const{
    cout<<GPA;
}
