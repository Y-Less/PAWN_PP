# __PAWN PP__

PAWN_PP is a library that defines pre-processor macros useful for performing compile-time maths and concatenations. 


# __Functions__



#### __PP_CHAIN()__

The PAWN pre-processor's ordering is very specific, and can make some
operations very tricky.  For example, this will not work:

```pawn
PP_ADD(PP_ADD(5,6),7)
```

That will try to add `7` to the string `PP_ADD(5,6)` and best-case scenario
will give the answer `117` (`5+6` summed, followed by `7`).  Getting the
pre-processor to do the inner sum before the outer sum can't be done in that
way since all macros are evaluated strictly left-to-right, not inner-to-
outer.

`PP_CHAIN` is a solution for this problem, for those macros explicitly
designed to support it.  It is a stack machine, that uses `$` as result
placeholders, and `POP` to get a previous answer and put it in to that
placeholder.  The example above would thus become:

```pawn
PP_CHAIN( \
	PP_ADD(5,6) \ // Pushes 11 on to the stack.
	PP_POP()    \ // Pops 11 and replaces the next "$" with it.
	PP_ADD($,7) \ // Pushes 18 on to the stack.
)
```

At the end of the chain, the complete stack contents are dumped out.  This:

```pawn
PP_CHAIN( \
	PP_ADD(5,6) \
	PP_ADD(5,7) \
)
```

Would give:

```pawn
(12)(11)
```

Since there are two items on the stack and they are both enclosed in
brackets automatically.  The brackets can be stripped with `PP_UNWRAP`:

```pawn
PP_CHAIN( \
	PP_ADD(5,6)  \
	PP_ADD(5,7)  \
	PP_UNWRAP($) \
)
```

Would give:

```pawn
12
```



#### __PP_UNWRAP()__

`PP_UNWRAP` can ONLY be used inside `PP_CHAIN`, unlike some other functions,
and thus it will automatically pop the top item off the stack, destroy the
remainder of the stack, and put the popped item back on without its brackets.
`PP_PRINT` is similar, but keeps the brackets.  In addition, both can take
additional parameters and will format accordingly:

```pawn
PP_CHAIN( \
	PP_ADD(5,6)  \
	PP_UNWRAP(answer = $;) \
)
```

Would give:

```pawn
answer = 11;
```

(To output an actual `$`, use `PP_DOLLAR<>`).



#### __PP_ADD() _[MINUS / SUB / LOG2 / POW2]___

`PP_ADD`, `PP_MINUS`, `PP_SUB`, `PP_LOG2`, and `PP_POW2` are all stand-alone
maths operations and will work without being in `PP_CHAIN`.  Thus, they will
not automatically pop symbols in lieu of `$` (this is a limitation I tried to
fix, but couldn't), so sadly they need an explicit `PP_POP_1`, `PP_POP_2`, or
`PP_POP_3` (for which there is currently no true need):

```pawn
PP_CHAIN( \
	PP_ADD(5,6)   \
	PP_ADD(40,80) \
	PP_POP_2()    \
	PP_SUB($,$)   \
)
```

Would give:

```pawn
(-109)
```

Results are popped from the bottom of the stack (last in, first out), and
pushed in reverse parameter order (as in PAWN), resulting in the code above
being equivalent to `(5 + 6) - (40 + 80)`.

It should also be noted that because the vast majority of the calculations
done in this way are done destructively in-place, there are no line-length
limitations to worry about (unless your equation itself is stupidly long).



#### __PP_TOKENISE()__

`PP_TOKENISE` is a synonym for `PP_UNWRAP`, meaning you can do:

```pawn
#define MY_MACRO_1(%0) %0 from MY_MACRO one
#define MY_MACRO_2(%0) %0 from MY_MACRO two
#define MY_MACRO_3(%0) %0 from MY_MACRO three

PP_CHAIN( \
	PP_ADD(1,2)               \
	PP_TOKENISE(MY_MACRO_$)   \
)(hi)
```

Would give:

```pawn
hi from MY_MACRO three
```

In that case, the entire chain would evaluate to exactly `MY_MACRO_3`, which
would then be parse and be seen to be preceeding another set of brackets,
thus matching and being expanded.  Again, this is different to doing `1+2`,
which would result in `MY_MACRO_1+2(hi)`, not `MY_MACRO_3(hi)`, and thus fail
to match even the first `MY_MACRO_1`.

As stated elsewhere, this technique only works on macros explicitly designed
for being used in `PP_CHAIN`, mainly those that use a `_THEN` variant.  To
write a standard value macro to work here would look like:

```pawn
#define MAX_PLAYERS  MAX_PLAYERS_() // Standard entry point - note `()`s.
#define MAX_PLAYERS_ MACRO(500)     // Define the true value.
```

That would (somewhat clunkily) enable:

```pawn
PP_CHAIN( \
	PP_MACRO(MAX_PLAYERS) \
	PP_POP()              \
	PP_ADD(1,$)           \
)
```

Someone using `MAX_PLAYERS` would get:

```pawn
MAX_PLAYERS
MAX_PLAYERS_()
MACRO(500)()
_PP_MACRO(500)
500
```

Someone using `PP_MACRO(MAX_PLAYERS)` inside `PP_CHAIN()` would get:

```pawn
PP_MACRO(MAX_PLAYERS)
MAX_PLAYERS_(_THEN)
MACRO(500)(_THEN)
_PP_MACRO_THEN(500)
_PP_MACRO(500)
```

The trick being that `PP_CHAIN` replaces all the functions inside it with
`_THEN` suffixed versions.  This is the only way I could find to let macros
use information calculated by other macros before they were evaluated.

I could not get this to work for constants as well as macros, so this will
not work:

```pawn
PP_CHAIN( \
	PP_MACRO(100)         \
	PP_POP()              \
	PP_ADD(1,$)           \
)
```

Instead use `PP_PUSH` (or `PP_CONST` in exactly the same way):

```pawn
PP_CHAIN( \
	PP_PUSH(100)          \
	PP_POP()              \
	PP_ADD(1,$)           \
)
```

This is slightly awkward, as it does mean that a macro like this will not
work in all generic cases:

```pawn
#define Dialog[%1](%2) \
	PP_CHAIN( \
		PP_MACRO(%1)       \
		PP_TOKENISE(@dlg$) \
	)(%2)
```

This will work:

```pawn
#define DIALOG_LOGIN_ID DIALOG_LOGIN_ID_()
#define DIALOG_LOGIN_ID_ MACRO(101)

Dialog[DIALOG_LOGIN_ID](playerid, dialogid, response)
{
}
```

This will not:

```pawn
Dialog[101](playerid, dialogid, response)
{
}
```

Sadly, you would need a separate version for the two cases, like
`DialogNumber` and `DialogMacro`.



#### __PP_ADD()__

Adds two numbers (each up to +/-256) together in the pre-processor.  This is
different to using the compiler, which would just output `7+8`, or some sort
of sum generation, which would give `7+1+1+1+1+1+1+1+1`.  The output is a
pure value of `15`, making it usable in other macros.  Strictly speaking, the
value ranges are not exactly +/-256 - anything between those values
(inclusive) are guaranteed to work; however, some other values in certain
positions may work if the result is still within +/-512.



#### __PP_SUB()__

Subtracts two numbers (each up to +/-256) together in the pre-processor.
This is different to using the compiler, which would just output `7-8`, or
some sort of sum generation, which would give `7-1-1-1-1-1-1-1-1`.  The
output is a pure value of `-1`, making it usable in other macros.  Strictly
speaking, the value ranges are not exactly +/-256 - anything between those
values (inclusive) are guaranteed to work; however, some other values in
certain positions may work if the result is still within +/-512.



#### __PP_MINUS()__

Return a number with a minus sign attached.  This is used internally when a
valid symbol representing a negative number is required, instead of a true
negative number that would not be a valid macro name.  Exposed here because
why not?



#### __Deferred operators__  
When you want, say, a bracket in the output but don't
want it to be matched by a macro use `PP_LEFT_BRACKET<>` instead.  That way
anything preceeding that will not see or match against a bracket, but once
that macro is reached it will be replaced with just `(`.



#### __PP_DROP()__ _[ID / ENCLOSE / TAG]_

* `PP_DROP` just removes itself and its parameter.
* `PP_ID` returns its parameter.
* `PP_ENCLOSE` returns its parameter wrapped in brackets.
* `PP_TAG` returns its parameter with a default tag override.




#### __PP_AMP()__

`true` when non-zero (truthy), `false` when zero (or falsy).



#### __PP_NOT()__

`false` when non-zero (truthy), `true` when zero (or falsy).




#### __PP_LOG2()__

Finds the log-2 base of a power of 2 number.  Only goes up to 2**73 due to
symbol length restrictions, but that is still vastly more than can be fit in
to 32-bit or 64-bit cells.



#### __PP_POW2()__

Finds 2 raised to the given power.  Resolves to just a number.  Can't do it
any other way, since there's no point supporting digit separators (because
the support range isn't large enough), and the result for negatives will be a
floating point number.




#### __PP_PREVENT_EXPANSION(A_MACRO(macro, parameters))__

Stops the specified macro expanding, so this code below will NOT give `4`:

```pawn
#define MY_MACRO() 4
PP_PREVENT_EXPANSION(MY_MACRO())
```

Note that this ONLY works on function-like macros, this will always expand:

```pawn
PP_PREVENT_EXPANSION(MAX_PLAYERS)
```





#### __PP_WARNING("Your warning message")__

Gives a compile-time warning.  It will be an unused symbol warning which
looks vaguely like the specified warning.  Something like:

```
Symbol is never used: WARNING_Your_warning_message
```

That should be enough for the user to go on.  Otherwise, they can look at the
given file and line number (which will be correct) and will see the more
useful code from the example (`PP_ERROR("Your error message")`).



#### __PP_ERROR("Your error message")__

Gives a compile-time error.  It will be an undefined symbol error which looks
vaguely like the specified error.  Something like:

```
Undefined symbol: ERROR_Your_error_message
```

That should be enough for the user to go on.  Otherwise, they can look at the
given file and line number (which will be correct) and will see the more
useful code from the example (`PP_ERROR("Your error message")`).



#### __PP_STRIP_SPACES(something goes here)__

Gives:

```pawn
somethinggoeshere
```

You probably want to combine this with `PP_TAG`.



#### __PP_REPLACE_SPACES(_)(something goes here)__

Gives:

```pawn
something_goes_here
```

The first parameter is the separator to replace spaces with.  You probably
want to combine this with `PP_TAG`.



#### __PP_PACKED()__

`1` when `"hello"` is a packed string, `0` when it isn't.  This could be because
the compiler is version 4.x (in which case `''hello''` is an unpacked string,
or because `#pragma pack 1` has been used, in which case `!"hello"` is a
packed string (opposite of default).



#### __PP_STRINGISE() & PP_STRINGIZE_PACKED()__

Gives a consistent stringize operator across multiple compiler versions.

This is not perfect.  The SA-MP PAWN branch reports its version as 3.2.3664,
which is `0x0302` in the `__Pawn` macro, but the real 3.2.3664 doesn't have
native string concatenation abilities so needs the same hack as older
versions; however, the two can't be distinguished (or maybe they can but I've
not figured out a way yet).  Thus, since the most likely use of 3.2 is for
SA-MP I'm assuming that.

In PAWN version 4, the strings changed `"string"` became default packed, and
unpacked became `''string''`, as opposed to `!"string"` and `"string"`
before (unless `#pragma pack 1` is used).  I also think this is when the
official compiler introduced `...` and `#` itself, since I can't find earlier
mentions, but my set of documentation is incomplete.  For this reason, I have
no implementation for the 3.3.xxxx branch.

Additionally, in the SA-MP version, this is legal:

```pawn
new str[] = "Hello " #world;
```

But in the 4.x branch, only macro parameters can be stringised, and must be
explicitly concatenated with `...`:

```pawn
#define CAT(%0,%1) %0 ... #%1
new str[] = CAT("Hello ", world);
```

This code unites the two by making everything macro parameters and always
having the `...` operator, even when it isn't required:

```pawn
new str[] = PP_STRINGISE("Hello " ... #world);
```

I'm not too sure how the explicit `#` there will fare, since that may result
in `##` in some compilers - fine for `SA-MP` but I don't know about 4.x.


# __Installation__

Simply install to your project:

```bash
sampctl package install Y-Less/PAWN_PP
```

Include in your code and begin using the library:

```pawn
#include <pp>
```
