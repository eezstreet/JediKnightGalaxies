// hide these nasty warnings

#ifdef _WIN32

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
#pragma warning(disable : 4201)
#pragma warning(disable : 4214)
#pragma warning(disable : 4244)		// conversion from double to float
#pragma warning(disable : 4284)		// return type not UDT
#pragma warning(disable : 4305)		// truncation from const double to float
#pragma warning(disable : 4310)		// cast truncates constant value
#pragma warning(disable : 4389)		// signed/unsigned mismatch
#pragma warning(disable : 4503)		// decorated name length truncated
//#pragma warning(disable:  4505)!!!remove these to reduce vm size!! // unreferenced local function has been removed
#pragma warning(disable : 4511)		//copy ctor could not be genned
#pragma warning(disable : 4512)		//assignment op could not be genned
#pragma warning(disable : 4514)		// unreffed inline removed
#pragma warning(disable : 4663)		// c++ lang change
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4710)		// not inlined
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
#pragma warning(disable : 4786)		//identifier was truncated

//rww (for vc.net, warning numbers changed apparently):
#pragma warning(disable : 4213)		//nonstandard extension used : cast on l-value
#pragma warning(disable : 4245)		//signed/unsigned mismatch

#pragma warning(disable : 4996)     //deprecated warning



// Time to enable some warnings that probably should be left in
//#pragma warning(2 : 4242)			// Return type does not match the variable type
#pragma warning(2 : 4062)			// Switch does not contain a default label when it should
#pragma warning(2 : 4191)			// Function pointer calling convention, args, or return type do not match assignment
#pragma warning(2 : 4287)			// Unsigned variable was compared against a signed value
#pragma warning(2 : 4289)			// Variable declared in for loop is used outside scope
#pragma warning(2 : 4296)			// Operator expression is always false
#pragma warning(2 : 4555)			// Expression with no effect
#pragma warning(2 : 4619)			// No warning exists for #pragma warning
#pragma warning(2 : 4710)			// Function was not inlined
#pragma warning(2 : 4711)			// Function was inlined


#endif