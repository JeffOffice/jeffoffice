#pragma once
// What the Pick a Function dialog says about the functions of the formula language: for each one the engine has, its kind, the names of its arguments (an optional one is in brackets, and
// "..." stands for as many more as the function takes), and a sentence on what it does. The list is the engine's: a test compares them, so no function is named here that the engine lacks.

#include <QString>
#include <QStringList>

namespace jp::crunch {

enum class FunctionCategory { Financial, DateTime, MathTrig, Statistical, LookupReference, Text, Logical, Information, Database };

struct FunctionDoc {
    const char *name;           // upper case
    FunctionCategory category;
    const char *args;           // "number1, [number2], ..."
    const char *summary;        // marked for translation: use summaryOf
};

// All of them, in the order they are listed; their number.
const FunctionDoc *functionDocs(int *count);
// The one of a name (any case), or null.
const FunctionDoc *docOf(const QString &name);
QString categoryName(FunctionCategory category);
QString summaryOf(const FunctionDoc &doc);
// The argument names in order ("[number2]" is optional, "..." is more of the last).
QStringList argumentsOf(const FunctionDoc &doc);
// "SUM(number1, ...)".
QString signatureOf(const FunctionDoc &doc);

} // namespace jp::crunch
