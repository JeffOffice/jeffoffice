#include "core/textexport.h"

#include "core/writeprops.h"
#include "sheet/csv.h"
#include "sheet/xlscodepage.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QLocale>
#include <QStringConverter>
#include <QTextCursor>
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

namespace {

const char *qtEncoderNameOf(int codePage)
{
    return codePage == 932 ? "Shift_JIS" : codePage == 936 ? "GBK" : codePage == 949 ? "EUC-KR" : codePage == 950 ? "Big5" : nullptr;
}

} // namespace

const QVector<TextCodePage> &textCodePages()
{
#define LABEL(x) QT_TRANSLATE_NOOP("jp::write::TextFile", x)
    static const QVector<TextCodePage> pages = {
        {1252, LABEL("Western European (Windows 1252)")}, {1250, LABEL("Central European (Windows 1250)")}, {1251, LABEL("Cyrillic (Windows 1251)")},
        {1253, LABEL("Greek (Windows 1253)")},            {1254, LABEL("Turkish (Windows 1254)")},          {1255, LABEL("Hebrew (Windows 1255)")},
        {1256, LABEL("Arabic (Windows 1256)")},           {1257, LABEL("Baltic (Windows 1257)")},           {1258, LABEL("Vietnamese (Windows 1258)")},
        {874, LABEL("Thai (Windows 874)")},               {28591, LABEL("Western European (ISO 8859-1)")},  {28592, LABEL("Central European (ISO 8859-2)")},
        {28593, LABEL("South European (ISO 8859-3)")},    {28594, LABEL("Baltic (ISO 8859-4)")},            {28595, LABEL("Cyrillic (ISO 8859-5)")},
        {28596, LABEL("Arabic (ISO 8859-6)")},            {28597, LABEL("Greek (ISO 8859-7)")},             {28598, LABEL("Hebrew (ISO 8859-8)")},
        {28599, LABEL("Turkish (ISO 8859-9)")},           {28605, LABEL("Western European (ISO 8859-15)")}, {437, LABEL("United States (DOS 437)")},
        {850, LABEL("Western European (DOS 850)")},       {852, LABEL("Central European (DOS 852)")},       {866, LABEL("Cyrillic (DOS 866)")},
        {10000, LABEL("Mac Roman")},                      {932, LABEL("Japanese (Shift JIS)")},             {936, LABEL("Chinese, simplified (GBK)")},
        {949, LABEL("Korean (EUC-KR)")},                  {950, LABEL("Chinese, traditional (Big5)")}};
#undef LABEL
    return pages;
}

bool canWriteCodePage(int codePage)
{
    QByteArray ignored;
    if (office::sheet::xlsTextToBytes(codePage, QString(), &ignored)) return true;
    const char *name = qtEncoderNameOf(codePage);
    return name && QStringEncoder(name).isValid();
}

bool looksLikePlainText(const QByteArray &bytes)
{
    if (bytes.startsWith("PK") || bytes.startsWith("%PDF") || bytes.startsWith("{\\rtf") || bytes.startsWith("\xD0\xCF\x11\xE0")) return false;
    if (bytes.startsWith("\xFF\xFE") || bytes.startsWith("\xFE\xFF")) return true;
    return !bytes.left(8192).contains('\0');
}

TextForm detectTextForm(const QByteArray &bytes, int fallbackPage)
{
    TextForm form;
    QString probe;
    if (bytes.startsWith("\xEF\xBB\xBF")) {
        form.encoding = TextEncoding::Utf8WithMark;
    } else if (bytes.startsWith("\xFF\xFE")) {
        form.encoding = TextEncoding::Utf16;
    } else if (bytes.startsWith("\xFE\xFF")) {
        form.encoding = TextEncoding::Utf16Be;
    } else {
        QStringDecoder utf8(QStringConverter::Utf8, QStringConverter::Flag::Stateless);
        const QString text = utf8(bytes);
        bool ascii = true;
        for (const char c : bytes) ascii = ascii && (uchar(c) < 0x80);
        if (ascii || !utf8.hasError()) {
            form.encoding = TextEncoding::Utf8;
        } else {
            form.encoding = TextEncoding::CodePage;
            form.codePage = fallbackPage > 0 ? fallbackPage : office::sheet::legacyCodePageFor(QLocale::system());
        }
    }
    // The line ends are the first line's own.
    for (qsizetype i = 0; i < bytes.size(); ++i) {
        if (form.encoding == TextEncoding::Utf16 || form.encoding == TextEncoding::Utf16Be) break;
        if (bytes[i] == '\n') { form.ends = LineEnds::Unix; break; }
        if (bytes[i] == '\r') { form.ends = i + 1 < bytes.size() && bytes[i + 1] == '\n' ? LineEnds::Windows : LineEnds::ClassicMac; break; }
    }
    if (form.encoding == TextEncoding::Utf16 || form.encoding == TextEncoding::Utf16Be) {
        // (the 16-bit units, either way round, from after the mark)
        const bool big = form.encoding == TextEncoding::Utf16Be;
        auto unit = [&](qsizetype i) { return big ? (uchar(bytes[2 * i]) << 8) | uchar(bytes[2 * i + 1]) : (uchar(bytes[2 * i + 1]) << 8) | uchar(bytes[2 * i]); };
        const qsizetype units = bytes.size() / 2;
        for (qsizetype i = 1; i < units; ++i) {
            const int u = unit(i);
            if (u == '\n') { form.ends = LineEnds::Unix; break; }
            if (u == '\r') { form.ends = i + 1 < units && unit(i + 1) == '\n' ? LineEnds::Windows : LineEnds::ClassicMac; break; }
        }
    }
    return form;
}

QString decodeTextFile(const QByteArray &bytes, const TextForm &form)
{
    QString text;
    switch (form.encoding) {
    case TextEncoding::Utf8:
    case TextEncoding::Utf8WithMark: {
        QByteArray body = bytes;
        if (body.startsWith("\xEF\xBB\xBF")) body.remove(0, 3);
        text = QString::fromUtf8(body);
        break;
    }
    case TextEncoding::Utf16:
    case TextEncoding::Utf16Be: {
        QByteArray body = bytes;
        bool big = form.encoding == TextEncoding::Utf16Be;
        if (body.startsWith("\xFF\xFE")) body.remove(0, 2), big = false;
        else if (body.startsWith("\xFE\xFF")) body.remove(0, 2), big = true;
        QStringDecoder decode(big ? QStringConverter::Utf16BE : QStringConverter::Utf16LE);
        text = decode(body);
        break;
    }
    case TextEncoding::Latin1: text = QString::fromLatin1(bytes); break;
    case TextEncoding::Ascii: text = QString::fromLatin1(bytes); break;
    case TextEncoding::CodePage: text = office::sheet::xlsBytesToText(form.codePage, bytes.constData(), bytes.size()); break;
    }
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    return text;
}

QByteArray plainTextFile(WriteDoc &doc, TextEncoding encoding, LineEnds ends, int *lost, int codePage)
{
    QString text = visibleText(doc);
    if (ends == LineEnds::Windows) text.replace(QLatin1Char('\n'), QStringLiteral("\r\n"));
    else if (ends == LineEnds::ClassicMac) text.replace(QLatin1Char('\n'), QLatin1Char('\r'));
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
    case TextEncoding::Utf16Be: {
        QByteArray out("\xFE\xFF", 2);
        QStringEncoder encode(QStringConverter::Utf16BE, QStringConverter::Flag::Default);
        out += QByteArray(encode(text));
        return out;
    }
    case TextEncoding::CodePage: {
        QByteArray out;
        qsizetype replaced = 0;
        if (office::sheet::xlsTextToBytes(codePage, text, &out, &replaced)) {
            if (lost) *lost = int(replaced);
            return out;
        }
        // (a double-byte page: the system's or Qt's own conversion; a character it has no code for becomes a question mark and is counted)
        const char *name = qtEncoderNameOf(codePage);
        QStringEncoder encode(name ? name : "UTF-8");
        if (!encode.isValid()) return text.toUtf8();
        const QByteArray whole = encode(text);
        if (lost) *lost = int(text.count(QLatin1Char('?')) < whole.count('?') ? whole.count('?') - text.count(QLatin1Char('?')) : 0);
        return whole;
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

std::unique_ptr<WriteDoc> WriteDoc::fromText(const QString &text)
{
    std::unique_ptr<WriteDoc> doc = blank();
    QTextDocument *story = doc->story();
    story->setUndoRedoEnabled(false);
    QTextCursor c(story);
    c.insertText(text);
    story->setUndoRedoEnabled(true);
    story->setModified(false);
    return doc;
}

} // namespace jp::write
