#pragma once
// The call of a function in a formula being typed: which one the cursor is in (the innermost whose name the engine knows), where it begins and ends, and its arguments as written. The Function
// Arguments dialog is begun from it, and puts its text back in its place. No GUI.

#include <QChar>
#include <QString>
#include <QStringList>

namespace jp::crunch {

struct FunctionCall {
    bool found = false;
    QString name;            // as written
    int start = 0;           // where the name begins
    int end = 0;             // after the closing bracket (the end of the text for a call that is not closed yet)
    bool closed = true;
    QStringList arguments;   // each as written, trimmed; none for "()" (a text in quotes that is not closed is closed)
    QStringList raw;         // the same with the spaces as they were typed
};

// The call the cursor is in, between its brackets (just after the opening one up to just before the closing one); `separator` stands between arguments. Quotes, names of sheets in quotes, braces,
// and square brackets are not looked into, and a bracket of a name that is no function (a group in "(1+2)*3") is passed over.
FunctionCall functionCallAt(const QString &text, int cursor, QChar separator);
// "NAME(a,b)": the arguments joined by the separator; empty ones at the end are left off, but the first `keep` are kept (the ones the call had: VLOOKUP(A1,B:C,2,) keeps its last comma).
QString callText(const QString &name, const QStringList &arguments, QChar separator, int keep = 0);

} // namespace jp::crunch
