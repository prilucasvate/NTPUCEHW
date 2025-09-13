/*
 * Test program for Assignment 4
 *
 *  Created by Hung-Ta Pai
 */

#include <iostream>
#include "CEStudent.h"
using namespace std;

int main()
{
	Student s1("Albert Pai");

	s1.printName();
	cout << endl;

	CEStudent s2;

	s2.setName("Hung-Ta Pai");
	s2.computeGPA(10, 5, 4, 3, 1);

	s2.setCPE(1);
	if (s2.getCPE())
	{
		cout << s2.getName() << " passed in CPE and has GPA=" << s2.getGPA() << ".\n";
	}
	else
	{
		cout << s2.getName() << " failed in CPE and has GPA=" << s2.getGPA() << ".\n";
	}

	if (s2.graduationRequirement())
	{
		cout << "Happy Graduation!\n";
	}

	cout << endl;
}
