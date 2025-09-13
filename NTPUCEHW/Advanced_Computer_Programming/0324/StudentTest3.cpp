/*
 * StudentTest.cpp
 *
 *  Created on: 2018�~12��5��
 *      Author: Hung-Ta Pai
 */

#include<iostream>
#include "Student.h"
using namespace std;

int main()
{
	Student* s1Ptr = new Student(3.5);
	Student* s2Ptr = new Student(3.3);

	s1Ptr->setName("Hung-Ta Pai");
	s2Ptr->setName("Albert Pai");
	cout << s1Ptr->getName() << "'s GPA is " << s1Ptr->getGPA() << "." << endl;
	cout << s2Ptr->getName() << "'s GPA is " << s2Ptr->getGPA() << "." << endl;
	
	if (*s1Ptr < *s2Ptr)
	{
		cout << s2Ptr->getName() << " has higher GPA." << endl;
	}
	else if (*s2Ptr < *s1Ptr)
	{
		cout << s1Ptr->getName() << " has higher GPA." << endl;
	} else
	{
		cout << "Same GPA!" << endl;
	}

	cout << "Their average GPA is " << (*s1Ptr+*s2Ptr)/2 << "." << endl;
}
