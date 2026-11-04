#include "core/textexport.h"

#include "core/writeprops.h"
#include "text/textprops.h"

#include <QStringConverter>
#include <QTextBlock>
#include <QTextFragment>

namespace jp::write {

QString visibleText(WriteDoc &doc)
{
    QString out;
    bool first = true;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        if (!first) out += QLatin1Char('\n');
        first = false;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const QTextCharFormat f = fr.charFormat();
            if (f.boolProperty(tp::HiddenText) || f.hasProperty(prop::TrackedDeleted) || f.hasProperty(prop::Atom)) continue;
            QString text = fr.text();
            text.remove(QChar::ObjectReplacementCharacter);
            text.replace(QChar::LineSeparator, QLatin1Char('\n'));
            text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
            out += text;
        }
    }
    return out;
}

QByteArray plainTextFile(WriteDoc &doc, TextEncoding encoding, LineEnds ends, int *lost)
{
    QString text = visibleText(doc);
    if (ends == LineEnds::Windows) text.replace(QLatin1Char('\n'), QStringLiteral("\r\n"));
    if (lost) *lost = 0;
    switch (encoding) {
    case TextEncoding::Utf8: return text.toUtf8();
    case TextEncoding::Utf8WithMark: return QByteArray("\xEF\xBB\xBF", 3) + text.toUtf8();
    case TextEncoding::Utf16: {
        QByteArray out("\xFF\xFE", 2);   // (the mark, then little-endian text)
        QStringEncoder encode(QStringConverter::Utf16LE, QStringConverter::Flag::Default);
        out += QByteArray(encode(text));
        return out;
    }
    case TextEncoding::Latin1:
    case TextEncoding::Ascii: {
        const int most = encoding == TextEncoding::Ascii ? 0x7F : 0xFF;
        QByteArray out;
        out.reserve(text.size());
        for (const QChar c : text) {
            if (c.unicode() <= most) {
                out += char(c.unicode());
            } else if (c.isLowSurrogate()) {
                continue;   // (the pair is one character: counted at its first half)
            } else {
                out += '?';
                if (lost) ++*lost;
            }
        }
        return out;
    }
    }
    return {};
}

} // namespace jp::write
