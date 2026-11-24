#include "core/fonttables.h"
#include "core/textlines.h"

namespace jp {

// (The columns are split at tabs and a tab at a line's end is an empty last column: Verdana's bold has an empty one, to have no kerning of its own.)
FontWidthText parseFontWidthText(const QByteArray &text)
{
    FontWidthText t;
    for (const QByteArray &line : textResourceLines(text, true)) {
        if (line.isEmpty() || line.startsWith('#')) continue;
        const QList<QByteArray> p = line.split('\t');
        if (t.chars.isEmpty()) {
            for (const QByteArray &c : line.split(',')) t.chars << char16_t(c.toUInt());
            continue;
        }
        if (p.size() < 3) continue;
        t.widths.insert(QString::fromUtf8(p[0]).toLower() + QLatin1Char('|') + QString::fromLatin1(p[1]), p[2]);
    }
    return t;
}

QHash<QString, QByteArray> parseFontKerningText(const QByteArray &text)
{
    QHash<QString, QByteArray> pairs;
    for (const QByteArray &line : textResourceLines(text, true)) {
        if (line.isEmpty() || line.startsWith('#')) continue;
        const QList<QByteArray> p = line.split('\t');
        if (p.size() < 3) continue;
        pairs.insert(QString::fromUtf8(p[0]).toLower() + QLatin1Char('|') + QString::fromLatin1(p[1]), p[2]);
    }
    return pairs;
}

QHash<QString, bool> parseFontFaceStyles(const QByteArray &text)
{
    QHash<QString, bool> t;
    for (const QByteArray &line : textResourceLines(text, true)) {
        const QList<QByteArray> p = line.split('\t');
        if (line.startsWith('#') || p.size() < 3) continue;
        bool &bold = t[QString::fromUtf8(p[0]).toLower()];
        bold |= (p[1].toInt() & 1) != 0;
    }
    return t;
}

}
