/*
 * Test program for Assignment 1
 *
 *  Created by Hung-Ta Pai
 */

#include <iostream>
#include "Student.h"
using namespace std;
int main()
{
	Student s1;
	Student s("Albert Pai");
	s.printName();

	s.setName("Hung-Ta Pai");
	cout << endl<< s.getName();

	return 0;
}
