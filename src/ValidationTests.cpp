#include "Validation.h"
#include <assert.h>

struct Test 
{ 
	int x = 0; 
	std::string s = "aa"; 
};

VALIDATE_BEGIN(Test)
	VALIDATE(x).EqualTo(1);
	VALIDATE(s).Length<More>(2);
VALIDATE_END()

VALIDATE_BEGIN_EX(Test, Command)
	VALIDATE(x).NonZero();
	VALIDATE(s).Length<Equal>(1);
VALIDATE_END()

void validationTest()
{
	using namespace validation;

	{
		Test test;

		ValidationResult result = ValidationTraits<Test>::validate(test);

		// Issue in field 'x' - #1 
		assert(std::get<0>(result) == 0 + 1);
		// Issue with validator #1 'EqualTo'
		assert(std::get<1>(result) == 0 + 1);

		test.x = 1;

		result = ValidationTraits<Test>::validate(test);

		// Issue in field 'str' - #2 
		assert(std::get<0>(result) == 0 + 2);
		// Issue with validator #1 'Length<More>'
		assert(std::get<1>(result) == 0 + 1);
	}

	{
		Test test;

		ValidationResult result = ValidationTraits<Test, Command>::validate(test);

		// Issue in field #1 'x'
		assert(std::get<0>(result) == 0 + 1);
		// Issue with validator #1 'NonZero'
		assert(std::get<1>(result) == 0 + 1);

		test.x = 1;

		result = ValidationTraits<Test>::validate(test);

		// Issue in field 'str' #2 
		assert(std::get<0>(result) == 0 + 2);
		// Issue with validator #1 'Length<Equal>'
		assert(std::get<1>(result) == 0 + 1);
	}
}
