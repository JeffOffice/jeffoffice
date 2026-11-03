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
// One sentence on what an argument is, by its name (a few by the function's too, as LARGE's k); empty for a name that is not known. A test checks that every argument of every function has one.
QString argumentHelpOf(const QString &function, const QString &argument);
// The arguments as rows to fill: the ones that always are there (`fixedOptional` says which may be left out), then the ones that repeat as a group (number1 gives number1, number2, ...; SWITCH's
// value1 and result1 give value1, result1, value2, result2, ...; an odd argument at the end of SWITCH's is its default, not another value).
struct ArgumentPlan {
    QStringList fixed;
    QList<bool> fixedOptional;
    QStringList repeating;
    QString trailing;   // an argument that comes after the last repeat, as a box of its own (SWITCH's default); empty for most functions
};
ArgumentPlan argumentPlanOf(const FunctionDoc &doc);
// The name of an argument of a group at its place (0 is the first, which has the name as it is: number1; 1 gives number2).
QString repeatedName(const QString &name, int instance);

} // namespace jp::crunch
