#if defined _INC_preproc
	#endinput
#endif
#define _INC_preproc

/**
 *               PP_CHAIN()
 * 
 * The PAWN pre-processor's ordering is very specific, and can make some
 * operations very tricky.  For example, this will not work:
 * 
 *     PP_ADD(PP_ADD(5,6),7)
 * 
 * That will try to add `7` to the string `PP_ADD(5,6)` and best-case scenario
 * will give the answer `117` (`5+6` summed, followed by `7`).  Getting the
 * pre-processor to do the inner sum before the outer sum can't be done in that
 * way since all macros are evaluated strictly left-to-right, not inner-to-
 * outer.
 * 
 * `PP_CHAIN` is a solution for this problem, for those macros explicitly
 * designed to support it.  It is a stack machine, that uses `$` as result
 * placeholders, and `POP` to get a previous answer and put it in to that
 * placeholder.  The example above would thus become:
 * 
 *     PP_CHAIN( \
 *         PP_ADD(5,6) \ // Pushes 11 on to the stack.
 *         PP_POP()    \ // Pops 11 and replaces the next "$" with it.
 *         PP_ADD($,7) \ // Pushes 18 on to the stack.
 *     )
 * 
 * At the end of the chain, the complete stack contents are dumped out.  This:
 * 
 *     PP_CHAIN( \
 *         PP_ADD(5,6) \
 *         PP_ADD(5,7) \
 *     )
 * 
 * Would give:
 * 
 *     (12)(11)
 * 
 * Since there are two items on the stack and they are both enclosed in
 * brackets automatically.  The brackets can be stripped with `PP_UNWRAP`:
 * 
 *     PP_CHAIN( \
 *         PP_ADD(5,6)  \
 *         PP_ADD(5,7)  \
 *         PP_UNWRAP($) \
 *     )
 * 
 * Would give:
 * 
 *     12
 * 
 * `PP_UNWRAP` can ONLY be used inside `PP_CHAIN`, unlike some other functions,
 * and thus it will automatically pop the top item off the stack, destroy the
 * remainder of the stack, and put the popped item back on without its brackets.
 * `PP_PRINT` is similar, but keeps the brackets.  In addition, both can take
 * additional parameters and will format accordingly:
 * 
 *     PP_CHAIN( \
 *         PP_ADD(5,6)  \
 *         PP_UNWRAP(answer = $;) \
 *     )
 * 
 * Would give:
 * 
 *     answer = 11;
 * 
 * (To output an actual `$`, use `PP_DOLLAR<>`).
 * 
 * `PP_ADD`, `PP_MINUS`, `PP_SUB`, `PP_LOG2`, and `PP_POW2` are all stand-alone
 * maths operations and will work without being in `PP_CHAIN`.  Thus, they will
 * not automatically pop symbols in lieu of `$` (this is a limitation I tried to
 * fix, but couldn't), so sadly they need an explicit `PP_POP_1`, `PP_POP_2`, or
 * `PP_POP_3` (for which there is currently no true need):
 * 
 *     PP_CHAIN( \
 *         PP_ADD(5,6)   \
 *         PP_ADD(40,80) \
 *         PP_POP_2()    \
 *         PP_SUB($,$)   \
 *     )
 * 
 * Would give:
 * 
 *     (-109)
 * 
 * Results are popped from the bottom of the stack (last in, first out), and
 * pushed in reverse parameter order (as in PAWN), resulting in the code above
 * being equivalent to `(5 + 6) - (40 + 80)`.
 * 
 * `PP_TOKENISE` is a synonym for `PP_UNWRAP`, meaning you can do:
 * 
 *     #define MY_MACRO_1(%0) %0 from MY_MACRO one
 *     #define MY_MACRO_2(%0) %0 from MY_MACRO two
 *     #define MY_MACRO_3(%0) %0 from MY_MACRO three
 *     
 *     PP_CHAIN( \
 *         PP_ADD(1,2)               \
 *         PP_TOKENISE(MY_MACRO_$)   \
 *     )(hi)
 * 
 * Would give:
 * 
 *     hi from MY_MACRO three
 * 
 * In that case, the entire chain would evaluate to exactly `MY_MACRO_3`, which
 * would then be parse and be seen to be preceeding another set of brackets,
 * thus matching and being expanded.  Again, this is different to doing `1+2`,
 * which would result in `MY_MACRO_1+2(hi)`, not `MY_MACRO_3(hi)`, and thus fail
 * to match even the first `MY_MACRO_1`.
 * 
 * As stated elsewhere, this technique only works on macros explicitly designed
 * for being used in `PP_CHAIN`, mainly those that use a `_THEN` variant.  To
 * write a standard value macro to work here would look like:
 * 
 *     #define MAX_PLAYERS        _MAX_PLAYERS()      // Standard entry point.
 *     #define MAX_PLAYERS_THEN() _MAX_PLAYERS(_THEN) // Chain entry point.
 *     #define _MAX_PLAYERS(%8)   PP_VALUE%8(500)     // Define the true value.
 * 
 * That would (somewhat clunkily) enable:
 * 
 *     PP_CHAIN( \
 *         MAX_PLAYERS() \
 *         PP_POP()      \ // Synonym for `PP_POP1` and `PP_POP_1`
 *         PP_ADD(1,$)   \
 *     )
 * 
 * Someone using `MAX_PLAYERS` would get:
 * 
 *     MAX_PLAYERS
 *     _MAX_PLAYERS()
 *     PP_VALUE(500)
 *     500
 * 
 * Someone using `MAX_PLAYERS()` would get:
 * 
 *     MAX_PLAYERS()
 *     _MAX_PLAYERS()()
 *     PP_VALUE(500)()
 *     500()
 * 
 * Someone using `MAX_PLAYERS()` inside `PP_CHAIN()` would get:
 * 
 *     MAX_PLAYERS()
 *     MAX_PLAYERS_THEN()
 *     _MAX_PLAYERS(_THEN)
 *     PP_VALUE_THEN(500)
 *     PP_PUSH_THEN(500)
 * 
 * The trick being that `PP_CHAIN` replaces all the functions inside it with
 * `_THEN` suffixed versions.  This is the only way I could find to let macros
 * use information calculated by other macros before they were evaluated.
 * 
 * It should also be noted that because the vast majority of the calculations
 * done in this way are done destructively in-place, there are no line-length
 * limitations to worry about (unless your equation itself is stupidly long).
**/
#define PP_CHAIN(%0(%1)%2) %0_THEN(%1)%2_PP_DONE()$()
#define _PP_DONE_THEN()$(%0) %0

#define PP_PUSH_THEN(%0)%1(%2)%3$(%4) %1_THEN(%2)%3$((%0)%4)
#define PP_POP_THEN()%0(%1$%2)%3$((%4)%5) %0_THEN(%1%4%2)%3$(%5)
#define PP_POP1_THEN()%0(%1$%2)%3$((%4)%5) %0_THEN(%1%4%2)%3$(%5)
#define PP_POP2_THEN()%0(%1$%2$%3)%4$((%5)(%6)%7) %0_THEN(%1%6%2%5%3)%4$(%7)
#define PP_POP3_THEN()%0(%1$%2$%3$%4)%5$((%6)(%7)(%8)%9) %0_THEN(%1%8%2%7%3%6%4)%5$(%9)
#define PP_POP_1_THEN PP_POP1_THEN
#define PP_POP_2_THEN PP_POP2_THEN
#define PP_POP_3_THEN PP_POP3_THEN
#define PP_PRINT_THEN(%0$%1)%2(%3)%4$((%5)%6) %2_THEN(%3)%4$((%0%5%1))
#define PP_UNWRAP_THEN(%0$%1)%2(%3)%4$((%5)%6) %2_THEN(%3)%4$(%0%5%1)
#define PP_TOKENISE_THEN(%8$%9)%2(%3)%4$((%5)%6) %2_THEN(%3)%4$(%8%5%9%6)
#define PP_TOKENIZE_THEN(%8$%9)%2(%3)%4$((%5)%6) %2_THEN(%3)%4$(%8%5%9%6)

#define PP_THEN.%0(%1)%2(%3)%4$(%5) %0(%1)%2_THEN(%3)%4$(%5)
#define PP_VALUE(%0) %0
#define PP_VALUE_THEN(%0) PP_PUSH_THEN(%0)

/**
 *               PP_ADD(7,8)
 *               PP_ADD(7,-43)
 *               PP_ADD(200,200)
 *               PP_ADD(-8,50)
 * 
 * Adds two numbers (each up to +/-256) together in the pre-processor.  This is
 * different to using the compiler, which would just output `7+8`, or some sort
 * of sum generation, which would give `7+1+1+1+1+1+1+1+1`.  The output is a
 * pure value of `15`, making it usable in other macros.  Strictly speaking, the
 * value ranges are not exactly +/-256 - anything between those values
 * (inclusive) are guaranteed to work; however, some other values in certain
 * positions may work if the result is still within +/-512.
**/
#define PP_ADD(%0,%1) _PP_STEP_%1(ADD,%0)()
#define PP_ADD_THEN(%0,%1) _PP_STEP_%1(ADD,%0)(_THEN)

/**
 *               PP_SUB(7,8)
 *               PP_SUB(7,-43)
 *               PP_SUB(200,200)
 *               PP_SUB(-8,50)
 * 
 * Subtracts two numbers (each up to +/-256) together in the pre-processor.
 * This is different to using the compiler, which would just output `7-8`, or
 * some sort of sum generation, which would give `7-1-1-1-1-1-1-1-1`.  The
 * output is a pure value of `-1`, making it usable in other macros.  Strictly
 * speaking, the value ranges are not exactly +/-256 - anything between those
 * values (inclusive) are guaranteed to work; however, some other values in
 * certain positions may work if the result is still within +/-512.
**/
#define PP_SUB(%0,%1) _PP_STEP_%1(SUB,%0)()
#define PP_SUB_THEN(%0,%1) _PP_STEP_%1(SUB,%0)(_THEN)

/**
 *               PP_MINUS(5)
 * 
 * Return a number with a minus sign attached.  This is used internally when a
 * valid symbol representing a negative number is required, instead of a true
 * negative number that would not be a valid macro name.  Exposed here because
 * why not?
**/
#define PP_MINUS(%0) _PP_MINUS_%0()
#define PP_MINUS_THEN(%0) _PP_MINUS_%0(_THEN)

/**
 * Deferred operators.  When you want, say, a bracket in the output but don't
 * want it to be matched by a macro use `PP_LEFT_BRACKET<>` instead.  That way
 * anything preceeding that will not see or match against a bracket, but once
 * that macro is reached it will be replaced with just `(`.
**/
#define PP_LEFT_BRACKET<> (
#define PP_RIGHT_BRACKET<> )
#define PP_LEFT_BRACE<> {
#define PP_RIGHT_BRACE<> }
#define PP_LEFT_SQUARE_BRACKET<> [
#define PP_RIGHT_SQUARE_BRACKET<> ]
#define PP_LEFT_ANGLE_BRACKET<> <
#define PP_RIGHT_ANGLE_BRACKET<> >
#define PP_LBRACKET<> (
#define PP_RBRACKET<> )
#define PP_LBRACE<> {
#define PP_RBRACE<> }
#define PP_LSQUARE<> [
#define PP_RSQUARE<> ]
#define PP_LANGLE<> <
#define PP_RANGLE<> >
#define PP_NULL<>
#define PP_COMMA<> ,
#define PP_SEMICOLON<> ;
#define PP_DOLLAR<> $
#define PP_COLON<> :
#define PP_SPACE<> PP_NULL<> PP_NULL<>

/**
 *               PP_DROP(x)
 *               PP_ID(x)
 *               PP_ENCLOSE(x)
 *               PP_TAG(x)
 * 
 * Useful helper functions:
 * 
 *   `PP_DROP` just removes itself and its parameter.
 *   `PP_ID` returns its parameter.
 *   `PP_ENCLOSE` returns its parameter wrapped in brackets.
 *   `PP_TAG` returns its parameter with a default tag override.
**/
#define PP_DROP(%0)
#define PP_ID(%0)      %0
#define PP_ENCLOSE(%0) (%0)
#define PP_TAG(%0)     (_:(%0))

#define PP_DROP_THEN    PP_THEN.PP_DROP
#define PP_ID_THEN      PP_THEN.PP_ID
#define PP_ENCLOSE_THEN PP_THEN.PP_ENCLOSE
#define PP_TAG_THEN     PP_THEN.PP_TAG

/**
 *               PP_AMP(5)
 * 
 * `true` when non-zero (truthy), `false` when zero (or falsy).
**/
#define PP_AMP(%0)          (bool:_PP_AMP_%0:true)
#define _PP_AMP_0:true)     false)
#define _PP_AMP_false:true) false)

/**
 *               PP_NOT(5)
 * 
 * `false` when non-zero (truthy), `true` when zero (or falsy).
**/
#define PP_NOT(%0)           (bool:_PP_NOT_%0:false)
#define _PP_NOT_0:false)     true)
#define _PP_NOT_false:false) true)
#define _PP_NOT_%0)          false)

/**
 *               PP_LOG2(32)
 *               PP_LOG2(0.25)
 *               PP_LOG_2(65,536)
 *               PP_LOG_2(268'435'456)
 * 
 * Finds the log-2 base of a power of 2 number.  Only goes up to 2**73 due to
 * symbol length restrictions, but that is still vastly more than can be fit in
 * to 32-bit or 64-bit cells.  Takes numbers as plain, comma thousands separated
 * and apostrophe thousands separated.  Not dot separated, sorry Europe.
 * 
 * Can resolve to just a number or a less accurate, but more flexible, tagged
 * number.  Define `PP_LOG2_SEPARATORS` to get to use `,` and `'`, but not use
 * it in a chain.
**/
#if defined PP_LOG2_SEPARATORS
	#define PP_LOG2(%0) (_:PP_LC:PP_LA:_PP_LOG2_%0)
	#define PP_LC:%0,%1) PP_LC:%0%1)
	#define PP_LA:%0'%1) PP_LA:%0%1)
#else
	#define PP_LOG2(%0) _PP_LOG2_%0()
	#define _PP_LOG2_0. _PP_LOG2_m
#endif
#define PP_LOG2_THEN(%0) _PP_LOG2_%0(_THEN)
#define PP_LOG_2 PP_LOG2
#define PP_LOG_2_THEN PP_LOG2_THEN

/**
 *               PP_POW2(32)
 *               PP_POW_2(5)
 *               PP_POW2(-2)
 * 
 * Finds 2 raised to the given power.  Resolves to just a number.  Can't do it
 * any other way, since there's no point supporting digit separators (because
 * the support range isn't large enough), and the result for negatives will be a
 * floating point number.
**/
#define PP_POW2(%0) _PP_POW2_%0()
#define PP_POW2_THEN(%0) _PP_POW2_%0(_THEN)
#define _PP_POW2_- _PP_POW2_m
#define PP_POW_2 PP_POW2

/**
 *               PP_PREVENT_EXPANSION(A_MACRO(macro, parameters))
 * 
 * Stops the specified macro expanding, so this code below will NOT give `4`:
 * 
 *     #define MY_MACRO() 4
 *     PP_PREVENT_EXPANSION(MY_MACRO())
 * 
 * Note that this ONLY works on function-like macros, this will always expand:
 * 
 *     PP_PREVENT_EXPANSION(MAX_PLAYERS)
 * 
**/
#define PP_PREVENT_EXPANSION(%0(%1)) %0 PP_LBRACKET<>%1 PP_RBRACKET<>

/**
 *               PP_WARNING("Your warning message")
 * 
 * Gives a compile-time warning.  It will be an unused symbol warning which
 * looks vaguely like the specified warning.  Something like:
 * 
 *     Symbol is never used: WARNING_Your_warning_message
 * 
 * That should be enough for the user to go on.  Otherwise, they can look at the
 * given file and line number (which will be correct) and will see the more
 * useful code from the example (`PP_ERROR("Your error message")`).
**/
#define PP_WARNING("%0") static stock PP_REPLACE_SPACES(_)(WARNING_%0)

/**
 *               PP_ERROR("Your error message")
 * 
 * Gives a compile-time error.  It will be an undefined symbol error which looks
 * vaguely like the specified error.  Something like:
 * 
 *     Undefined symbol: ERROR_Your_error_message
 * 
 * That should be enough for the user to go on.  Otherwise, they can look at the
 * given file and line number (which will be correct) and will see the more
 * useful code from the example (`PP_ERROR("Your error message")`).
**/
#define PP_ERROR("%0") PP_REPLACE_SPACES(_)(ERROR_%0)

/**
 *               PP_STRIP_SPACES(something goes here)
 * 
 * Gives:
 * 
 *     somethinggoeshere
 * 
 * You probably want to combine this with `PP_TAG`.
**/
#define PP_STRIP_SPACES(%0) (_PP_STRIP_SPACES:%0)
#define _PP_STRIP_SPACES:%0\32;%1) _PP_STRIP_SPACES:%0)

/**
 *               PP_REPLACE_SPACES(_)(something goes here)
 * 
 * Gives:
 * 
 *     something_goes_here
 * 
 * The first parameter is the separator to replace spaces with.  You probably
 * want to combine this with `PP_TAG`.
**/
#define PP_REPLACE_SPACES(%8)(%0) (_PP_REPLACE_SPACES:PP_DROP(%8)%0)
#define _PP_REPLACE_SPACES:PP_DROP(%8)%0\32;%1) _PP_REPLACE_SPACES:PP_DROP(%8)%0%8%1)

/**
 *               PP_PACKED
 * 
 * 1 when `"hello"` is a packed string, 0 when it isn't.  This could be because
 * the compiler is version 4.x (in which case `''hello''` is an unpacked string,
 * or because `#pragma pack 1` has been used, in which case `!"hello"` is a
 * packed string (opposite of default).
**/

// Only way to determine if `#pragma pack 1` is in use.  Longer string to not
// result in an array of just one cell.
static stock const _PP_PACKED_STRING[] = "PACKED";

#if sizeof (_PP_PACKED_STRING) == 2
	#define PP_PACKED 1
#elseif sizeof (_PP_PACKED_STRING) == 7
	#define PP_PACKED 0
#else
	#error Indeterminate packed string state.
#endif

/**
 *               PP_STRINGISE(your ... string ... here)
 *               PP_STRINGIZE(your ... string ... here)
 * 
 *               PP_STRINGISE_PACKED(your ... packed ... string ... here)
 *               PP_STRINGIZE_PACKED(your ... packed ... string ... here)
 * 
 * Gives a consistent stringize operator across multiple compiler versions.
 * 
 * This is not perfect.  The SA-MP PAWN branch reports its version as 3.2.3664,
 * which is `0x0302` in the `__Pawn` macro, but the real 3.2.3664 doesn't have
 * native string concatenation abilities so needs the same hack as older
 * versions; however, the two can't be distinguished (or maybe they can but I've
 * not figured out a way yet).  Thus, since the most likely use of 3.2 is for
 * SA-MP I'm assuming that.
 * 
 * In PAWN version 4, the strings changed `"string"` became default packed, and
 * unpacked became `''string''`, as opposed to `!"string"` and `"string"`
 * before (unless `#pragma pack 1` is used).  I also think this is when the
 * official compiler introduced `...` and `#` itself, since I can't find earlier
 * mentions, but my set of documentation is incomplete.  For this reason, I have
 * no implementation for the 3.3.xxxx branch.
 * 
 * Additionally, in the SA-MP version, this is legal:
 * 
 *     new str[] = "Hello " #world;
 * 
 * But in the 4.x branch, only macro parameters can be stringised, and must be
 * explicitly concatenated with `...`:
 * 
 *     #define CAT(%0,%1) %0 ... #%1
 *     new str[] = CAT("Hello ", world);
 * 
 * This code unites the two by making everything macro parameters and always
 * having the `...` operator, even when it isn't required:
 * 
 *     new str[] = PP_STRINGISE("Hello " ... #world);
 * 
 * I'm not too sure how the explicit `#` there will fare, since that may result
 * in `##` in some compilers - fine for `SA-MP` but I don't know about 4.x.
**/

#if __Pawn >= 0x0400
	// Use the `...`s, which should already be included.  Also convert `#"x" to
	// "x" and the wrong quotes to the correct ones.
	#define PP_STRINGISE(%0) (_:_PP_SS1:_PP_SS2:''''...#%0)
	#define PP_STRINGISE_PACKED(%0) (_:_PP_SS3:_PP_SS4:""...#%0)
	#define _PP_SS1:%0"%1"%2)   _PP_SS1:%0''%1''%2)
	#define _PP_SS2:%0#''%2)    _PP_SS2:%0''%2)
	#define _PP_SS3:%0''%1''%2) _PP_SS3:%0"%1"%2)
	#define _PP_SS4:%0#"%2)     _PP_SS4:%0"%2)
#elseif __Pawn >= 0x0302
	// SA-MP compiler and forks (Zeex, Fast/Russian, etc.)
	#if PP_PACKED
		#define PP_STRINGISE(%0) (_:_PP_SS:!""#%0)
		#define PP_STRINGISE_PACKED(%0) (_:_PP_SS:""#%0)
	#else
		#define PP_STRINGISE(%0) (_:_PP_SS:""#%0)
		#define PP_STRINGISE_PACKED(%0) (_:_PP_SS:!""#%0)
	#endif
	#define _PP_SS:%0...%1) _PP_SS:%0#%1)
#else
	// Old compilers.  Had a bug where macros would be expanded inside strings,
	// which is what we want, but wouldn't search for patterns inside them,
	// which is also what we want.  Change `#x...` to `"x"...`, then `"x"..."y"`
	// `"xy"`, but ignore `#"x"...`
	#if PP_PACKED
		#define PP_STRINGISE(%0) (_:_PP_SS:_PP_SS1:_PP_SS2:_PP_SS3:_PP_SS4:!#%0...)
		#define PP_STRINGISE_PACKED(%0) (_:_PP_SS:_PP_SS1:_PP_SS2:_PP_SS3:_PP_SS4:#%0...)
	#else
		#define PP_STRINGISE(%0) (_:_PP_SS:_PP_SS1:_PP_SS2:_PP_SS3:_PP_SS4:#%0...)
		#define PP_STRINGISE_PACKED(%0) (_:_PP_SS:_PP_SS1:_PP_SS2:_PP_SS3:_PP_SS4:!#%0...)
	#endif
	#define _PP_SS:%0##           %0#
	#define _PP_SS1:%0#%1...      _PP_SS:_PP_SS1:%0"%1"...#
	#define _PP_SS2:%0""%1""      _PP_SS2:%0"%1"
	#define _PP_SS3:%0"%1"..."%2" _PP_SS3:%0"%1%2"
	#define _PP_SS4:%0...#        %0
#endif

#define PP_STRINGIZE PP_STRINGISE
#define PP_STRINGIZE_PACKED PP_STRINGISE_PACKED

// Internal details.

// Convert addition of negative numbers to subtraction, and vice-versa.
#define _PP_STEP_-%0(%9,%1) _PP_INVERT_%9(%0,%1)
#define _PP_INVERT_ADD(%0,%1) _PP_STEP_%0(SUB,%1)
#define _PP_INVERT_SUB(%0,%1) _PP_STEP_%0(ADD,%1)
#define _PP_SUB_-%0(%1) _PP_SUB_PP_MINUS_%0(%1)
#define _PP_ADD_-%0(%1) _PP_ADD_PP_MINUS_%0(%1)

// A negative negative is positive.
#define PP_MINUS_- PP_MINUS_PP_MINUS_

// Include the other macros.
#if !defined PP_MAX_MATHS
	#define PP_MAX_MATHS 256
#endif

#include "PP/pp_log_2.inc"
#include "PP/pp_pow_2.inc"

#include "PP/pp_step_max_256.inc"
#include "PP/pp_add_max_256.inc"
#include "PP/pp_sub_max_256.inc"
#include "PP/pp_minus_max_256.inc"
#if PP_MAX_MATHS > 256
	#include "PP/pp_step_max_512.inc"
	#include "PP/pp_add_max_512.inc"
	#include "PP/pp_sub_max_512.inc"
	#include "PP/pp_minus_max_512.inc"
#endif
#if PP_MAX_MATHS > 512
	#include "PP/pp_step_max_1024.inc"
	#include "PP/pp_add_max_1024.inc"
	#include "PP/pp_sub_max_1024.inc"
	#include "PP/pp_minus_max_1024.inc"
#endif

#include "PP/pp_tests.inc"

