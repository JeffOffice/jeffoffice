#pragma once
// The fonts a part of a workbook names in its text: a rich run of a shared string (`rFont`), and the letters of a shape or a chart (`a:latin`, `a:ea`, `a:cs`, as `typeface`). Replace Fonts
// counts them and puts another name in their place, in the bytes of the part, so that the part keeps everything else as it was.

#include <QByteArray>
#include <QMap>
#include <QPair>
#include <QString>
#include <QVector>

namespace jp::office::sheet {

// Whether a part of the package may name fonts this way (the shared strings, the drawings, the charts, and the comments), by its name.
bool partNamesFontsInText(const QString &partName);
// Adds to `counts` each font the bytes name (one for each element; a name that is a theme reference, "+mn-lt", is none).
void countFontsInText(const QByteArray &xml, QMap<QString, qint64> *counts);
// The bytes with each name that is `from` (without regard to case) given as `to`; unchanged (and `changed` false) when none is.
QByteArray renameFontsInText(const QByteArray &xml, const QString &from, const QString &to, bool *changed = nullptr);

} // namespace jp::office::sheet
