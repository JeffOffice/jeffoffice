#pragma once
// The functions a BIFF formula names by number: the name each number stands for and how many arguments a call through the fixed-argument token carries.

#include <QString>

namespace jp::office::sheet {

struct XlsFunction {
    const char *name;   // null for a number no function has
    qint8 fixedArgs;    // arguments of a call with no count of its own; -1 for a function that is always called with a count
};

// The function of that number (0 to 0x7FFF); a record with a null name if there is none.
XlsFunction xlsFunction(int index);

} // namespace jp::office::sheet
