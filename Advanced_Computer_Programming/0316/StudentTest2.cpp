/*
 * Test program for Assignment 2
 *
 *  Created by Hung-Ta Pai
 */

#include <iostream>
#include <string>
#include "Student.h"
using namespace std;

int main()
{
	Student t1(3.5);
	Student t2;

	t1.setName("John");
	t2.setName("Mary");

	cout << t1.getName() << "'s GPA is " << t1.getGPA() << "." << endl;
	cout << t2.getName() << "'s GPA is " << t2.getGPA() << "." << endl;

	t2.computeGPA(2,2,3,4,2);
	cout << t2.getName() << "'s GPA: ";
	t2.printGPA();
	cout << endl;

	return 0;
}


