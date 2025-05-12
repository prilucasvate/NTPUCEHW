#include <string>
class Student{
	public:
		explicit Student(std::string);
		explicit Student(double=0.0);
		~Student();
		std::string getName() const;
		void setName(std::string);
		double getGPA() const;
		void computeGPA(int, int, int, int, int);
		void  printName() const;
		void printGPA() const;
	private:
		std::string studentName;
		double GPA;
};