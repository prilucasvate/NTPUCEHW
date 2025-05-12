#include <string>
#include "Student.h"
class CEStudent : public Student{
	public:
		void setCPE(bool);
		bool getCPE() const;
		bool graduationRequirement() const;
	private:
		bool CPE;
};
