#pragma once
// The text of the bundled width and kerning tables (resources/fontwidths.txt, fontkerning.txt), told apart line by line.

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

namespace jp {

struct FontWidthText {
    QVector<char16_t> chars;                 // the characters the widths are of (the first line)
    QHash<QString, QByteArray> widths;       // "family|style" -> the text of its widths
};

FontWidthText parseFontWidthText(const QByteArray &text);
QHash<QString, QByteArray> parseFontKerningText(const QByteArray &text);   // "family|style" -> the text of its pairs
QHash<QString, bool> parseFontFaceStyles(const QByteArray &text);          // lower-cased family -> whether it has a bold face

}
