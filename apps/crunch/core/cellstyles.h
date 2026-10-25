#pragma once
// The cell styles a workbook offers: the built-in ones (whether or not the file has them yet) and the file's own, with what each looks like for a gallery's swatch.

#include "sheet/styles.h"

#include <QString>
#include <QVector>

namespace jp::crunch {

struct StyleSample {
    QString name;
    bool inFile = false;       // the workbook has the style already
    int group = 0;             // 0 good, bad, and neutral; 1 data and model; 2 titles and headings; 3 themed; 4 number formats; 5 the file's own
    bool bold = false, italic = false, underline = false;
    quint32 textRgb = 0x000000;
    bool hasFill = false;
    quint32 fillRgb = 0xFFFFFF;
    bool hasBorder = false;
    quint32 borderRgb = 0x000000;
    qint32 numFmtId = 0;
};

// The group names a gallery shows over its styles, by group number.
QString styleGroupName(int group);
// Every style a gallery offers, the file's own first, then the built-in ones in the gallery's order (those the file has taking its definition). Colors are resolved against the workbook's
// theme.
QVector<StyleSample> styleSamples(const jp::office::sheet::Styles &styles);

} // namespace jp::crunch
