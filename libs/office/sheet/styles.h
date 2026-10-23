#pragma once
// What a workbook's styles part says about number formats: the custom formats by id, and which format each cell format (the `s` of a cell) shows its
// value with. The rest of the styles (fonts, fills, borders) is not read here.

#include <QHash>
#include <QString>
#include <QVector>

namespace jp::office::sheet {

struct Styles {
    QHash<int, QString> numFmts;    // the formats the file defines, by id
    QVector<qint32> cellXfs;        // the format id of each cell format, in order
    qint64 chars = 0;               // the characters of all the format codes

    // The id of the number format of a cell format (0, General, for one the file doesn't have).
    qint32 numFmtId(quint32 xf) const { return xf < quint32(cellXfs.size()) ? cellXfs[int(xf)] : 0; }
    // Its code: the file's own for an id it defines, else the built-in one of that id (calc/numfmt.h), else "General".
    QString formatCode(quint32 xf) const;
};

} // namespace jp::office::sheet
