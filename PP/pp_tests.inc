// Test generic addition.
#assert PP_ADD(10,20)   == 30
#assert PP_ADD(10,-20)  == -10
#assert PP_ADD(-10,20)  == 10
#assert PP_ADD(-10,-20) == -30

// Test generic subtraction.
#assert PP_SUB(10,20)   == -10
#assert PP_SUB(10,-20)  == 30
#assert PP_SUB(-10,20)  == -30
#assert PP_SUB(-10,-20) == 10

// Test the extreme ranges.
#assert PP_ADD(256,256)   == 512
#assert PP_ADD(256,-256)  == 0
#assert PP_ADD(-256,256)  == 0
#assert PP_ADD(-256,-256) == -512

#assert PP_SUB(256,256)   == 0
#assert PP_SUB(256,-256)  == 512
#assert PP_SUB(-256,256)  == -512
#assert PP_SUB(-256,-256) == 0

// Test the cases where the first value is greater than 256, and the answer is
// less than 513 (which is the technical range for which these operations are
// defined, but it is just simpler to say +/-256 for each operand.

#assert PP_ADD(400,100)   == 500
//#assert PP_ADD(400,-100)  == 300
//#assert PP_ADD(-400,100)  == -300
#assert PP_ADD(-400,-100) == -500

//#assert PP_SUB(400,100)   == 300
#assert PP_SUB(400,-100)  == 500
#assert PP_SUB(-400,100)  == -500
//#assert PP_SUB(-400,-100) == -300

#assert PP_NOT(0)
#assert !PP_NOT(-1)
#assert !PP_NOT(1)
#assert !PP_NOT(42)

#assert PP_LOG2(256) == 8
#assert PP_LOG2(65536) == 16

#assert PP_CHAIN( \
	PP_ADD(5,6)   \
	PP_ADD(5,6)   \
	PP_POP_2()    \
	PP_SUB($,$)   \
	PP_PRINT($)   \
) == 0

#assert PP_CHAIN( \
	PP_ADD(5,6)   \
	PP_ADD(5,5)   \
	PP_POP_2()    \
	PP_SUB($,$)   \
	PP_PRINT($)   \
) == 1

#assert PP_CHAIN( \
	PP_ADD(5,6)   \
	PP_ADD(4,0)   \
	PP_POP_2()    \
	PP_SUB($,$)   \
	PP_PRINT($)   \
) == 7

#assert PP_CHAIN( \
	PP_ADD(4,0)   \
	PP_ADD(5,5)   \
	PP_POP_2()    \
	PP_SUB($,$)   \
	PP_PRINT($)   \
) == -6

#assert PP_CHAIN(PP_ADD(5,6)PP_POP_1()PP_ID($)) == 11
#assert PP_CHAIN( \
	PP_ADD(5,6) \
	PP_ADD(7,8) \
	PP_POP_2()  \
	PP_ADD($,$) \
	PP_PRINT(PP_LBRACKET<>$PP_RBRACKET<>) \
) == (26)

#assert PP_CHAIN( \
	PP_ADD(5,-9)  \
	PP_POP1()     \
	PP_SUB($,2)   \
	PP_POP()      \
	PP_ENCLOSE($) \
) == (-6)

#assert PP_CHAIN( \
	PP_ADD(5,-9)  \
	PP_POP1()     \
	PP_SUB($,2)   \
	PP_POP()      \
	PP_MINUS($)   \
	PP_POP()      \
	PP_ID($)      \
) == 6

#assert PP_POW_2(9) == 512
#assert PP_POW_2(30) == 1073741824

#assert PP_CHAIN( \
	PP_ADD(1,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_POW2($)    \
	PP_POP()      \
	PP_ID($)      \
) == 512

#assert PP_CHAIN( \
	PP_ADD(5,6)   \
	PP_ADD(5,6)   \
	PP_POP_2()    \
	PP_SUB($,$)   \
) == 0

#assert PP_CHAIN( \
	PP_ADD(5,6)   \
	PP_ADD(5,5)   \
	PP_POP_2()    \
	PP_SUB($,$)   \
) == 1

#assert PP_CHAIN( \
	PP_ADD(5,6)   \
	PP_ADD(4,0)   \
	PP_POP_2()    \
	PP_SUB($,$)   \
) == 7

#assert PP_CHAIN( \
	PP_ADD(4,0)   \
	PP_ADD(5,5)   \
	PP_POP_2()    \
	PP_SUB($,$)   \
) == -6

#assert PP_CHAIN(PP_ADD(5,6)) == 11

#assert PP_CHAIN( \
	PP_ADD(5,6) \
	PP_ADD(7,8) \
	PP_POP_2()  \
	PP_ADD($,$) \
	PP_UNWRAP($) \
) == (26)

#assert PP_CHAIN( \
	PP_ADD(5,-9)  \
	PP_POP1()     \
	PP_SUB($,2)   \
	) == (-6)

#assert PP_CHAIN( \
	PP_ADD(5,-9)  \
	PP_POP1()     \
	PP_SUB($,2)   \
	PP_POP()      \
	PP_MINUS($)   \
	PP_UNWRAP($)   \
) == 6

#assert PP_CHAIN( \
	PP_ADD(1,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP()      \
	PP_POW2($)    \
) == 512

#assert PP_CHAIN( \
	PP_ADD(1,1)   \
	PP_ADD(1,1)   \
	PP_ADD(1,1)   \
	PP_ADD(1,1)   \
	PP_POP()      \
	PP_ADD($,1)   \
	PP_POP_2()    \
	PP_ADD($,$)   \
	PP_POP_2()    \
	PP_ADD($,$)   \
	PP_POP_2()    \
	PP_ADD($,$)   \
	PP_POP()      \
	PP_POW2($)    \
) == 512

#assert PP_CHAIN( \
	PP_LOG2(32)    \
	PP_POP()      \
	PP_ADD($,5)   \
) == 10

#assert PP_CHAIN( \
	PP_LOG2(32)    \
	PP_POP()      \
	PP_SUB(6,$)   \
) == 1

static stock const _PP_TEST_STRING_0U[] = PP_STRINGISE("Hello " ... "world");
static stock const _PP_TEST_STRING_1U[] = PP_STRINGISE("Hello " ... #world);
static stock const _PP_TEST_STRING_2U[] = PP_STRINGISE("Hello " ... world);
static stock const _PP_TEST_STRING_3U[] = PP_STRINGISE(#Hello ... "world");
static stock const _PP_TEST_STRING_4U[] = PP_STRINGISE(#Hello ... #world);
static stock const _PP_TEST_STRING_5U[] = PP_STRINGISE(#Hello ... world);
static stock const _PP_TEST_STRING_6U[] = PP_STRINGISE(Hello ... "world");
static stock const _PP_TEST_STRING_7U[] = PP_STRINGISE(Hello ... #world);
static stock const _PP_TEST_STRING_8U[] = PP_STRINGISE(Hello ... world);
static stock const _PP_TEST_STRING_9U[] = PP_STRINGISE();

static stock const _PP_TEST_STRING_0P[] = PP_STRINGISE_PACKED("Hello " ... "world");
static stock const _PP_TEST_STRING_1P[] = PP_STRINGISE_PACKED("Hello " ... #world);
static stock const _PP_TEST_STRING_2P[] = PP_STRINGISE_PACKED("Hello " ... world);
static stock const _PP_TEST_STRING_3P[] = PP_STRINGISE_PACKED(#Hello ... "world");
static stock const _PP_TEST_STRING_4P[] = PP_STRINGISE_PACKED(#Hello ... #world);
static stock const _PP_TEST_STRING_5P[] = PP_STRINGISE_PACKED(#Hello ... world);
static stock const _PP_TEST_STRING_6P[] = PP_STRINGISE_PACKED(Hello ... "world");
static stock const _PP_TEST_STRING_7P[] = PP_STRINGISE_PACKED(Hello ... #world);
static stock const _PP_TEST_STRING_8P[] = PP_STRINGISE_PACKED(Hello ... world);
static stock const _PP_TEST_STRING_9P[] = PP_STRINGISE_PACKED();

static stock _PP_TEST_PREVENT_EXPANSION(a, b, c)
{
	// Need to disable the recursion warnings here maybe.
	#define _PP_TEST_PREVENT_EXPANSION(%0,%1,%2) PP_ERROR("Macro was expanded")
	PP_PREVENT_EXPANSION(_PP_TEST_PREVENT_EXPANSION(a, b, c));
}

