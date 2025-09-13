#include <string>
class Student{
	public:
		explicit Student(std::string);
		explicit Student();
		std::string getName() const;
		void setName(std::string);
		void  printName() const;
	private:
		std::string studentName;
};