#pragma once
// The functions of the formula language: a registry of what each is called, how many arguments it takes, what each argument is (a value, or a reference or
// array kept as it is), and whether its value changes with no cell changing (volatile). No GUI.
//
// The parameter string has one letter for each parameter, and the last letter stands for every further argument of a function that takes any number:
//   v  a value. A reference is read (a range by implicit intersection with the cell of the formula, or, in an array formula, element by element), and an array
//      gives its first element (in an array formula, a function of a value is made for each element). An argument left out is told apart.
//   r  a reference, an array, or a value, kept as it is (SUM, COUNT, VLOOKUP's table).
//   a  as r, and what is written in the argument is worked out as an array (SUMPRODUCT, INDEX's first argument, LOOKUP's vectors).
//   x  a reference only (ROW, COLUMN, ROWS, AREAS, OFFSET's first argument); anything else is #VALUE!.
//   n  as r, and the function sees the argument as it comes (IF, IFERROR, IFNA, CHOOSE).

#include "calc/value.h"

#include <QString>

namespace jp::calc {

class Call;

constexpr quint8 kFnVolatile = 1;   // NOW, TODAY, RAND, RANDBETWEEN, OFFSET, INDIRECT: the value changes with no cell changing
constexpr quint8 kFnLazy = 2;       // IF, IFERROR, IFNA, CHOOSE: the arguments not needed are not worked out
constexpr quint8 kFnFuture = 4;     // added after the first release of the format: a file writes it with the prefix _xlfn. (and without it, it is some other function of that name)

struct FunctionInfo {
    const char *name;           // upper case, without the prefix a newer function carries in a file
    quint8 minArgs;
    quint8 maxArgs;             // 255: any number
    const char *params;
    quint8 flags;
    Value (*impl)(Call &);
};

// The letter of the parameter that argument i goes with (the last letter stands for every further argument).
inline char paramClass(const FunctionInfo &f, int i)
{
    int n = 0;
    while (f.params[n]) ++n;
    return n == 0 ? 'r' : f.params[i < n ? i : n - 1];
}

// The function of a name (any case; a prefix _xlfn. or _xlws. is dropped), or null.
const FunctionInfo *findFunction(const QString &name);
// Its place in the registry, -1 for none; the other way round.
int functionIndex(const QString &name);
const FunctionInfo &functionAt(int index);
int functionCount();

} // namespace jp::calc
