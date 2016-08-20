## PW-Sat2 Code Style

This short document describes preferred style for the OBC C source files.

#### Indentation

PW-Sat2 source code uses 4 spaces for statement indentation. Having short indentation style should not be read as encouragement for writing code with deeply nested code blocks. When you need more than three indentation levels than you are doing something wrong or you are trying to put too much logic in single function. In both cases the code should be fixed so it can be easily read & understood.

Do not put multiple statements in single line. Such style is much more error prone:

    if (value > 50) return 1;
        return 0;

#### Breaking long lines

Since code is much more read then it is actually written the code style should be at first as much readable as possible.
Statements that are longer than 120 characters should be broken into multiple lines. Exceeding this limit is discouraged but it is allowed as long as it does not inhibit readability and breaking such long statement into multiple lines would have negative impact.

When statement is broken in to multiple lines each of the subsequent lines should be indented to the next level than level of the original statement beginning:

    static const int value = getBaseValue() +
        getScallingFactor() * getCoefficient() +
        getCoordinateCorrection();

The same rule also applies to strings:

    static const char* const string = "This is"
        "rather long string value that does "
        "not fit in to single line. ";


In case of function declaration/definitions when entire function prototype does not fit in single line place each function argument in separate line like this:

    static int CalculateLength(int baseValue,
        float scallingFactor,
        float roll,
        float pitch,
        float yaw
        );

Function call statements should be organized in a similar way to function declaration statements. Whenever they do not fit into single line break them down into multiple lines keeping each function argument in separate line:

    int length = CalculateLength(getBaseLenght(),
        getConfiguredScale(),
        getXRotation(),
        getYRotation(),
        getZRotation()
        );

When declaring enums the preferred way of formatting is to put each enumerator in separate line, however if they all can fit into single line and do not contain value initializers they may be put in single line:

    enum SomeEnumType
    {
        FIRST,
        SECOND,
        THIRD
    };

or:

    enum SomeEnumType {FIRST, SECOND, THIRD};

but **not**:

    enum SomeEnumType {FIRST = 10, SECOND = 20, THIRD = 30};

#### Braces

PW-Sat2 follows [BSD/Allman](https://en.wikipedia.org/wiki/Indent_style#Allman_style) style of putting braces in the source code with some additional guidelines.

The opening & closing braces should be put either in the same row or the same column:

    // opening & braces are indented in the same way
    if(someCondition())
    {
        DoSomething();
    }

    const int lookup[] = {8, 7, 6, 5, 4};       // also fine, opening & closing brace are placed in the same line.

    // This style is also correct.
    const int lookup[] =
    {
        8, 7, 6, 5, 4
    };

In order to reduce potential confusion due and/or accidental errors always use braces for conditional blocks and loops, as a result this:

    if (condition)
    {
        SomeAction();
    }

    while (condition)
    {
        Count();
    }

is preferred to this:

    if (condition)
        SomeAction();

    while (condition)
        Count();


Use braces also in case of empty compound blocks like this:

    while (someCondition()) {}

or like this:

    while (someCondition())
    {
    }


Also put an empty line after closing brace unless the next line also contains closing brace:

    void foo()
    {
        if (condition)
        {
            while(anotherCondition())
            {
            }

            foo();
        }
    }

#### Spacing

Spacing should be first and foremost used to separate logical parts of the statement from each other.
Following are the minimal guidelines for spacing the statements using single space character as a separator. Most of the time single character is enough, but there are cases when using more than one character would lead to much more readable code structure i.e. indenting array initializers, comments. In such cases users are encouraged to do so (within reason).

Use spaces after most of the C keywords with the exception of the:

    sizeof, typeof alingas, alignof, break, continue

Do not add spaces inside parenthesized/bracket expression, space it like this:

    while (condition)

    int array[10];
    int lookup[] = {1, 2};

instead of:

    while ( condition )

    int array[ 10 ];
    int lookup[ ] = {1, 2};


Put single space characters around following binary/tenary operators:

    =  +  -  <  >  *  /  %  |  &  ^  <=  >=  ==  !=  ?  : || && ^= += -= *= /= %=

But do not place them after following unary operators:

    &  *  +  -  ~  !  sizeof typeof alingas alignof ++ -- ,

Do not place space before ++/-- post-fix operators.

Do not place spaces around the member access operators:

    . ->

Do not put spaces before semicolon.

Do not put the space after function names:

    static int CalculateLength(int baseValue,
        float scallingFactor,
        float roll,
        float pitch,
        float yaw
        );

    int length = CalculateLength(getBaseLenght(),
        getConfiguredScale(),
        getXRotation(),
        getYRotation(),
        getZRotation()
        );

Do not add extra spaces around curly braces for array/structure initializers. Express it like this:

    int lookup[] = {1, 2};
    struct Result status = {true, 1.0};

instead of this:

    int lookup[] = { 1, 2 };
    struct Result status = { true, 1.0 };

#### Naming

Naming is one of the most important aspects, which cannot be ignored.

PW-Sat2 uses [Camel Case](https://en.wikipedia.org/wiki/CamelCase) naming convention for all declared/defined symbols with some exceptions.

Names should be descriptive and represent the purpose of the symbol, but **do not** go to the extremes and use names like *temporaryObjectCounterWithCorrection*. Too long names do not provide much value and only take long time to type. Try to avoid single/double character names except for special cases like temporaries or loop counters. Avoid using abbreviations.

**Do not** try to encode symbol type into its name, this so-called Hungarian notation does not provide much value and the compiler already knows the types of the symbols and can check them at compile time.

Macros are separate entities that do not exist on the semantic level therefore they are treated separately, their names should be all uppercase and they should use underscore character to separate words:

    #define UNREFERENCED_PARAMETER(x) ((void)(x))

Similarly enumerators should also be all uppercase:

    enum SomeEnum
    {
        FIRST,
        SECOND,
        THIRD
    };

All global/public symbols should be follow upper camel case convention (they should start with capital letter), all nonpublic symbols should follow lower camel case convention (should start with lower case letter).

Since C language does not provide any means of the name separation/segregation all public symbols that module is providing should be prefixed with module name while following the naming convention for the exported symbols, i.e. macro names should be prefixed with the uppercase module name and underscore:

for module Comm:

    #define COMM_PROTOCOL_VERSION 0x12

whereas functions/global variables should be prefixed with module name in upper camel case:

    void CommSendPacket(const struct Buffer* buffer);

#### Automatic Formatting

For the moment the Pw-Sat2 project is using [clang-format](http://clang.llvm.org/docs/ClangFormat.html) utility for automatic source code formatting. Its mostly complete configuration file that will automatically apply majority of the formatting rules described above is available [here](../.clang-format). Be advised, the minimum supported version of clang format is 3.7.

The source code formatting with the Clang-Format utility has been integrated into Pw-Sat2 build system. Almost every C/C++ module in the repository supports special build target for automatic module wide source code formatting. The build targets for source code formatting contain '.format' suffix i.e. for logger module the target that should be invoked to reformat all of its files would be:

    make logger.format

Alternatively the Clang-Format utility can be invoked manually on the limited set of files or single file if need be using following command (assuming that clang-format is available somewhere in your PATH):

    clang-format --style=file -i <list of files>

Where \<list of files\> is list of paths to files that should be reformatted.

Clang-Format is already in one way or the other integrated with most of the popular IDEs. The Qt Creator already has built-in support for it, all that has to be done it is enable it in the options menu. If you are using Eclipse you can use [CppStyle](https://github.com/wangzw/CppStyle) plugin to automatically format the code using the clang-format utility and above configuration file.
