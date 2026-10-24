#pragma once
// The cell styles a spreadsheet program comes with ("Good", "Heading 1", "Accent2", "Percent"): their names, the number the file gives each (`builtinId`), and what each does to the
// font, fill, border, and number format of a cell. A workbook that lacks one gets it when it is first used (Workbook::styleNamed).

#include "sheet/styles.h"

#include <QStringList>

namespace jp::office::sheet {

struct BuiltinStyle {
    QString name;
    qint32 builtinId = -1;
    // The font: what differs from the workbook's own first font (its name, size, and kind of face stay unless said).
    bool bold = false, italic = false;
    Underline underline = Underline::None;
    double size = 0;              // 0 to keep the workbook's
    QString fontName, scheme;     // a typeface to use whatever the workbook has, and "major" or "minor" for the theme's own (a title is in the heading font)
    ColorSpec fontColor;
    FillSpec fill;
    BorderSpec border;
    qint32 numFmtId = 0;          // a number format built into the file form (percent 9, comma 43, ...)
    // Where it is in the gallery: 0 good, bad, and neutral; 1 data and model; 2 titles and headings; 3 themed styles; 4 number formats.
    int group = 0;
};

// The names of the built-in styles in the order a gallery shows them, with the group each is in (see BuiltinStyle::group).
QStringList builtinStyleNames();
// The definition of the built-in style with the name (any case); false for a name that is not one.
bool builtinStyle(const QString &name, BuiltinStyle *out);

} // namespace jp::office::sheet
