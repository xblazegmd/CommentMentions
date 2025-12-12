# Code styling
Different people have different code styling. I made this mod with my own code styling. So while I could leave anyone to style their code however they'd like, it's nicer if I add some code styling guidelines so the code becomes more consistently styled (plus I don't want .NET devs messing with my code, see [Scopes](#scopes) to see what I mean with that).

This should hopefully give you a better idea to what code styling you should use. Code not following the code styling guidelines should be modified to follow them.

## Files and directories
Files should be in camelCase (or PascalCase if it's a hook). Directories should be in nocase. .hpp should be used over .h. .cpp and .hpp pairs in `core` should be in their own directory, with the exception of `utils.cpp` and `utils.hpp`.

```
// Correct
src/
    core/
        this/
            this.cpp
            this.hpp
        that/
            that.cpp
            that.hpp
        utils.cpp
        utils.hpp

// Wrong
src/
    Core/
        This.cpp
        This.h
        That.cpp
        That.h
        utils.cpp
        utils.h
```

## Comments
###### *If you need to change your comment styling to adapt to these guidelines, idk what's wrong with you*

Single-line comments should have a 1-space padding applied to them
```cpp
// Yes
/* Yes */

//Nope
/*This is way too claustrophobic*/
```

Multi-line comments should at the very least have the 1-space padding applied to them, or start at a new line with an indent
```cpp
/*
    Yes, this is cool
*/
/* This is also
cool */

/*Nope this
is bad*/
```

## Variables and constants
`nullptr` should be used to define null pointers

```cpp
// Correct
Object* obj = nullptr;

// Wrong
Object* obj = 0;
Object* obj = NULL;
Object* obj = 0.0f;
```

Only one variable should be defined in a single line

```cpp
// Correct
int a = 1;
int b = 2;

// Wrong
int a = 1, b = 2;
```

Static variables should be prefixed with g_
```cpp
// Correct
static Object* g_obj;

// Wrong
static Object* s_obj;
static Object* obj;
```

## Scopes
THIS IS NOT C#. DO NOT DEFINE SCOPES IN A NEWLINE
```cpp
// Correct
void myFunction() {
   // Function stuff 
}

if (a == b) {
    doThis();
}

// WRONG
void myOtherFunction()
{
    // Function stuff
}

if (b == c)
{
    doThat();
}
```

## Classes
Classes *must* be in PascalCase
```cpp
// Correct
class MyAmazingClass {
    // ...
};

// Wrong
class myAmazingClass {
    // ...
};
```

If the class inherits a `CCNode` or similar, then the class name *must* be prefixed with CM:
```cpp
// Correct
class CMAmazingClass : public CCNode {
    // ...
};

// Wrong
class AmazingClass : public CCNode {
    // ...
};
```

Access specifiers should be on a different indentation than the class definition (4 spaces)
```cpp
// Correct (for me at least)
class AmazingClass {
    public:
        // Stuff
};

// Wrong
class AmazingClass {
public:
    // I prefer a different indentation
};

// Also wrong
class AmazingClass {
 public:
    // Who does this???
};
```

## Namespaces
Namespaces should be used whenever possible. Namespaces should be in camelCase.

```cpp
// Correct
namespace amazingNamespace {
    // Blah, blah, blah
}

// Wrong
namespace AmazingNamespace {
    // Blah, blah, blah
}
```

If conflicting with a Geode namespace, namespaces should be prefixed with CM and be in PascalCase.

```cpp
// Correct
namespace CMUtils {
    // Utils
}

// Wrong
namespace someOtherUtilNameIdk {
    // Utils
}
```

For logic in `core`, no namespaces should be nested unless necessary.

```cpp
// Correct
namespace namespaceLol {
    // ...
}

// Wrong
namespace rootNamespace {
    namespace namespaceLol {
        // ...
    }
}

// Also wrong
namespace rootNamespace::namespaceLol {
    // ...
}
```

> I'm too lazy, I'll add more later