#pragma once
// A number format code as the model keeps it ("#,##0.00", "yyyy-mm-dd", "0.0%;[Red]-0.0%", "[h]:mm:ss") written as the data styles of an OpenDocument document (number:number-style,
// number:date-style, number:percentage-style, number:text-style, ...). A code of several sections (or with conditions) is a style for the last section and styles for the others, which
// the style names with style:map elements, as the other spreadsheet program writes them. The reader of odsstyles.cpp turns these back into the same code.

#include <QString>

namespace jp::office::sheet {

struct OdfDataStyles {
    QString xml;        // the elements, to go in office:automatic-styles
    QString mainName;   // the name of the style a cell names (empty for General)
};

// `baseName` is the name of the style (and the start of the names of the styles it maps to). False if the code uses something OpenDocument's data styles cannot say; then nothing is set.
bool odfDataStyles(const QString &code, const QString &baseName, OdfDataStyles *out);

} // namespace jp::office::sheet
