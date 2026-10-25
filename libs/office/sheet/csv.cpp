#include "sheet/csv.h"

#include "calc/dates.h"
#include "calc/numfmt.h"
#include "sheet/cellref.h"
#include "sheet/entry.h"
#include "sheet/xlscodepage.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>

#include <algorithm>
#include <cstring>
#include <map>
#include <vector>

namespace jp::office::sheet {

namespace {

// ---- encodings

bool validUtf8(const char *p, qsizetype n)
{
    qsizetype i = 0;
    while (i < n) {
        const quint8 c = quint8(p[i]);
        if (c < 0x80) {
            ++i;
            continue;
        }
        int more = 0;
        quint32 min = 0;
        if (c >= 0xC2 && c <= 0xDF) {
            more = 1;
            min = 0x80;
        } else if (c >= 0xE0 && c <= 0xEF) {
            more = 2;
            min = 0x800;
        } else if (c >= 0xF0 && c <= 0xF4) {
            more = 3;
            min = 0x10000;
        } else {
            return false;
        }
        if (i + more >= n) return false;
        quint32 cp = c & (0x3F >> more);
        for (int k = 1; k <= more; ++k) {
            const quint8 d = quint8(p[i + k]);
            if ((d & 0xC0) != 0x80) return false;
            cp = (cp << 6) | (d & 0x3F);
        }
        if (cp < min || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
        i += more + 1;
    }
    return true;
}

// How much of a file is looked at to tell whether it is UTF-8: the first 32 MB, not ending in the middle of a character (a file that is UTF-8 so far is UTF-8; checking a file of a
// gigabyte to its end would take the time the first screen should not wait for).
qsizetype utf8Sample(const QByteArray &bytes)
{
    qsizetype n = std::min<qsizetype>(bytes.size(), qsizetype(32) << 20);
    if (n < bytes.size())
        while (n > 0 && (quint8(bytes[n]) & 0xC0) == 0x80) --n;
    return n;
}

QString decodeSample(const QByteArray &bytes, qsizetype from, qsizetype count, const CsvForm &form)
{
    count = std::min<qsizetype>(count, bytes.size() - from);
    if (count <= 0) return QString();
    switch (form.encoding) {
    case CsvEncoding::Utf8:
    case CsvEncoding::Utf8Bom: return QString::fromUtf8(bytes.constData() + from, count);
    case CsvEncoding::Utf16Le:
    case CsvEncoding::Utf16Be: {
        count &= ~qsizetype(1);
        QString s(count / 2, QChar());
        for (qsizetype i = 0; i < count / 2; ++i) {
            const quint8 a = quint8(bytes[from + 2 * i]), b = quint8(bytes[from + 2 * i + 1]);
            s[i] = QChar(form.encoding == CsvEncoding::Utf16Le ? char16_t(a | (b << 8)) : char16_t((a << 8) | b));
        }
        return s;
    }
    case CsvEncoding::Legacy: return xlsBytesToText(form.codePage, bytes.constData() + from, count);
    }
    return QString();
}

// ---- the delimiter

// The number of fields in each of the first lines of the text, with `delimiter` (quotes respected); and in how many of the lines a quote stood where it opens nothing (inside a field
// that did not begin with one), which a file read with the right delimiter has few of.
std::vector<int> fieldCounts(const QString &text, QChar delimiter, int maxRecords, int *strayRecords = nullptr)
{
    std::vector<int> counts;
    int fields = 1;
    bool any = false, stray = false;
    int strays = 0;
    // (as the reading of the fields does it: a quote opens a quoted field where a field starts, at the start of a record or after a delimiter, and is an ordinary mark anywhere else)
    enum State { FieldStart, Plain, Quoted, AfterQuote } state = FieldStart;
    for (qsizetype i = 0; i < text.size() && int(counts.size()) < maxRecords; ++i) {
        const QChar c = text[i];
        if (state == Quoted) {
            if (c == QLatin1Char('"')) {
                if (i + 1 < text.size() && text[i + 1] == QLatin1Char('"')) ++i;
                else state = AfterQuote;
            }
            continue;
        }
        if (c == delimiter) {
            ++fields;
            any = true;
            state = FieldStart;
        } else if (c == QLatin1Char('\n') || c == QLatin1Char('\r')) {
            if (c == QLatin1Char('\r') && i + 1 < text.size() && text[i + 1] == QLatin1Char('\n')) ++i;
            if (any) counts.push_back(fields);
            if (stray) ++strays;
            stray = false;
            fields = 1;
            any = false;
            state = FieldStart;
        } else if (c == QLatin1Char('"') && state == FieldStart) {
            state = Quoted;
            any = true;
        } else {
            if (c == QLatin1Char('"')) stray = true;
            any = true;
            if (state == FieldStart) state = Plain;
        }
    }
    if (stray) ++strays;
    if (strayRecords) *strayRecords = strays;
    if (any && int(counts.size()) < maxRecords) counts.push_back(fields);
    return counts;
}

QChar chooseDelimiter(const QString &text, const QString &fileName, const QLocale &locale)
{
    const bool tsv = fileName.endsWith(QLatin1String(".tsv"), Qt::CaseInsensitive);
    // (in the order they win a tie: the language's own first)
    QList<QChar> order;
    if (tsv) order << QLatin1Char('\t');
    if (locale.decimalPoint() == QLatin1String(",")) order << QLatin1Char(';') << QLatin1Char(',');
    else order << QLatin1Char(',') << QLatin1Char(';');
    order << QLatin1Char('\t') << QLatin1Char('|');
    QChar best = QLatin1Char(',');
    qint64 bestScore = 0;
    int bestStrays = 0;
    for (const QChar d : std::as_const(order)) {
        int strays = 0;
        const std::vector<int> counts = fieldCounts(text, d, 100, &strays);
        if (counts.empty()) continue;
        std::map<int, int> histogram;
        for (const int n : counts) ++histogram[n];
        int mode = 1, modeCount = 0;
        for (const auto &h : histogram)
            if (h.first > 1 && h.second > modeCount) {
                mode = h.first;
                modeCount = h.second;
            }
        if (mode < 2) continue;
        // a delimiter whose lines have the same number of fields wins; among those, the one that splits more
        // (and a delimiter with which a quote stands in the middle of fields fewer times is the more likely: a quoted field full of commas is no reason to split a line at its commas)
        const qint64 score = qint64(modeCount) * 1000 + std::min(mode, 999);
        if (bestScore == 0 || strays < bestStrays || (strays == bestStrays && score > bestScore)) {
            bestScore = score;
            bestStrays = strays;
            best = d;
        }
    }
    return best;
}

// ---- reading one field as a cell

bool allDigits(QStringView s)
{
    if (s.isEmpty()) return false;
    for (const QChar c : s)
        if (c.unicode() < '0' || c.unicode() > '9') return false;
    return true;
}

// TRUE and FALSE in the language of the person (the words the common spreadsheet programs write for them there), in the order of the languages' codes; English is read in every language.
struct BooleanWords {
    QLocale::Language language;
    const char *yes;
    const char *no;
};
const BooleanWords kBooleanWords[] = {
    {QLocale::German, "WAHR", "FALSCH"},          {QLocale::French, "VRAI", "FAUX"},           {QLocale::Spanish, "VERDADERO", "FALSO"},
    {QLocale::Catalan, "CERT", "FALS"},           {QLocale::Italian, "VERO", "FALSO"},         {QLocale::Dutch, "WAAR", "ONWAAR"},
    {QLocale::Portuguese, "VERDADEIRO", "FALSO"}, {QLocale::Swedish, "SANT", "FALSKT"},        {QLocale::Danish, "SAND", "FALSK"},
    {QLocale::NorwegianBokmal, "SANN", "USANN"},  {QLocale::NorwegianNynorsk, "SANN", "USANN"}, {QLocale::Finnish, "TOSI", "EPÄTOSI"},
    {QLocale::Polish, "PRAWDA", "FAŁSZ"},         {QLocale::Czech, "PRAVDA", "NEPRAVDA"},      {QLocale::Slovak, "PRAVDA", "NEPRAVDA"},
    {QLocale::Hungarian, "IGAZ", "HAMIS"},        {QLocale::Romanian, "ADEVĂRAT", "FALS"},     {QLocale::Russian, "ИСТИНА", "ЛОЖЬ"},
    {QLocale::Turkish, "DOĞRU", "YANLIŞ"},        {QLocale::Greek, "ΑΛΗΘΕΣ", "ΨΕΥΔΕΣ"}};

// The words for true and false in the person's language, folded to compare without regard to case; empty for a language that writes them in English.
void booleanWordsFor(const QLocale &locale, QString *yes, QString *no)
{
    yes->clear();
    no->clear();
    for (const BooleanWords &w : kBooleanWords)
        if (w.language == locale.language()) {
            *yes = QString::fromUtf8(w.yes).toCaseFolded();
            *no = QString::fromUtf8(w.no).toCaseFolded();
            return;
        }
}

// Whether a day and a month typed as a date have a year with them. A date typed with a year has three parts (numbers, or a number and the name of a month) or a number of three digits or
// more; two parts are a day and a month (3/4, 12.5, March 1, Dec 25, Sep-01), which the common program completes with the year of the day it is read on, and which are kept as typed because
// a date made of a text cannot be made a text again. The time that follows a date is left out of the count.
bool dateHasYear(const QString &typed, const QSet<QString> &months)
{
    QString date = typed.trimmed();
    const qsizetype colon = date.indexOf(QLatin1Char(':'));
    if (colon >= 0) {
        qsizetype hour = colon;
        while (hour > 0 && date[hour - 1].isDigit()) --hour;
        date.truncate(hour);
    }
    int parts = 0;
    bool longNumber = false;
    for (qsizetype i = 0; i < date.size();) {
        qsizetype j = i;
        if (date[i].isDigit()) {
            while (j < date.size() && date[j].isDigit()) ++j;
            ++parts;
            if (j - i >= 3) longNumber = true;
        } else if (date[i].isLetter()) {
            while (j < date.size() && date[j].isLetter()) ++j;
            if (months.contains(date.mid(i, j - i).toLower())) ++parts;
        } else {
            ++j;
        }
        i = j;
    }
    return longNumber || parts >= 3;
}

// A field that a number would damage: digits only, with a zero in front of more digits (007), or more than 15 of them (the 16th and later become zeros).
bool keepsAsText(QStringView field)
{
    QStringView s = field.trimmed();
    if (!s.isEmpty() && (s[0] == QLatin1Char('+') || s[0] == QLatin1Char('-'))) s = s.mid(1);
    if (!allDigits(s)) return false;
    return (s.size() > 1 && s[0] == QLatin1Char('0')) || s.size() > 15;
}

// The currency sign of a locale as readNumber (entry.cpp) takes it.
QString currencySignOf(const QLocale &l)
{
    const QString c = l.currencySymbol(QLocale::CurrencySymbol);
    return c.isEmpty() ? QStringLiteral("$") : c;
}

// Every number format code readEntry can ask for, in the person's locale (decimals up to 10: a typed number with more shows 10).
QStringList entryFormatCodes(const QLocale &locale)
{
    QStringList codes;
    auto zeros = [](int n) { return QString(n, QLatin1Char('0')); };
    const QString sign = currencySignOf(locale);
    for (int dec = 0; dec <= 10; ++dec) {
        codes << (dec == 0 ? QStringLiteral("0%") : QStringLiteral("0.") + zeros(dec) + QStringLiteral("%"));
        const QString digits = dec == 0 ? QStringLiteral("#,##0") : QStringLiteral("#,##0.") + zeros(std::max(2, dec));
        codes << digits;
        codes << QStringLiteral("\"%1\"%2").arg(sign, digits);
        codes << QStringLiteral("%2\" %1\"").arg(sign, digits);
    }
    codes << QStringLiteral("0.00E+00");
    const QStringList times = {QStringLiteral("h:mm AM/PM"), QStringLiteral("h:mm:ss AM/PM"), QStringLiteral("[h]:mm"), QStringLiteral("[h]:mm:ss"), QStringLiteral("h:mm:ss.0"),
                               QStringLiteral("h:mm:ss"), QStringLiteral("h:mm")};
    for (const QString &t : times) {
        codes << t;
        codes << QStringLiteral("m/d/yyyy ") + t;
    }
    codes << QStringLiteral("m/d/yyyy");
    return codes;
}

// The words (in lower case) that may stand in a number, a date, or a time as typed: the exponent, the markers of the morning and the afternoon, true and false, and the names of the
// months (the person's language and English). A field with any other word in it is text, which spares the slow reading of a date or a number for the many texts of a file.
QSet<QString> knownWordsOf(const QLocale &locale, QSet<QString> *months)
{
    QSet<QString> words = {QStringLiteral("e"), QStringLiteral("am"), QStringLiteral("pm"), QStringLiteral("a"), QStringLiteral("p"), QStringLiteral("m"), QStringLiteral("true"),
                           QStringLiteral("false"), QStringLiteral("t")};
    const QLocale english(QLocale::English, QLocale::UnitedStates);
    auto addWords = [&](const QString &text) {
        QString w;
        for (const QChar c : text) {
            if (c.isLetter()) {
                w += c.toLower();
            } else if (!w.isEmpty()) {
                words.insert(w);
                w.clear();
            }
        }
        if (!w.isEmpty()) words.insert(w);
    };
    for (const QLocale *l : {&locale, &english}) {
        for (int m = 1; m <= 12; ++m)
            for (const QLocale::FormatType t : {QLocale::LongFormat, QLocale::ShortFormat}) {
                const QString name = l->monthName(m, t).toLower();
                const QSet<QString> before = words;
                addWords(name);
                if (name.size() >= 3) words.insert(name.left(3));
                for (const QString &w : std::as_const(words))
                    if (!before.contains(w)) months->insert(w);
            }
        addWords(l->amText());
        addWords(l->pmText());
    }
    return words;
}

// ---- the scanner: fields and records out of pieces of text

class Sink {
public:
    virtual ~Sink() = default;
    // A field (`quoted` when it was in quotes) of the current record; false stops the scan.
    virtual bool field(quint32 col, QStringView text, bool quoted) = 0;
    // The record is over; false stops the scan.
    virtual bool endRecord() = 0;
};

class Scanner {
public:
    Scanner(const QString &delimiters, QChar quote, bool merge, Sink *sink, int maxField) : m_delims(delimiters), m_quote(quote), m_hasQuote(!quote.isNull()), m_merge(merge), m_sink(sink), m_max(maxField) {}
    // Takes the next piece of the text. False if the sink asked to stop.
    bool feed(QStringView s)
    {
        qsizetype i = 0;
        const qsizetype n = s.size();
        while (i < n) {
            const QChar c = s[i];
            if (m_crPending) {
                m_crPending = false;
                if (c == QLatin1Char('\n')) {
                    ++i;
                    continue;
                }
            }
            switch (m_state) {
            case FieldStart:
                if (isDelim(c)) {
                    if (m_merge && m_afterDelim) {
                        ++i;   // (several separators in a row count as one)
                        continue;
                    }
                    if (!endField(QStringView(), false)) return false;
                    m_afterDelim = true;
                    ++i;
                } else if (m_hasQuote && c == m_quote) {
                    m_afterDelim = false;
                    m_state = Quoted;
                    m_quoted = true;
                    m_buf.clear();
                    m_inBuf = true;
                    ++i;
                } else if (c == QLatin1Char('\r') || c == QLatin1Char('\n')) {
                    if (c == QLatin1Char('\r')) m_crPending = true;
                    ++i;
                    if (m_fields > 0 && !endField(QStringView(), false)) return false;
                    if (!endRecord()) return false;
                } else {
                    m_state = Plain;
                    m_inBuf = false;
                    m_buf.clear();
                    m_quoted = false;
                    m_afterDelim = false;
                }
                break;
            case Plain: {
                qsizetype j = i;
                while (j < n && !isDelim(s[j]) && s[j] != QLatin1Char('\r') && s[j] != QLatin1Char('\n')) ++j;
                if (!m_inBuf && j < n) {
                    // the whole field is in this piece: no copy
                    QStringView view = s.mid(i, j - i);
                    if (view.size() > m_max) {
                        view = view.left(m_max);
                        m_truncated = true;
                    }
                    if (!emit(view, false)) return false;
                } else {
                    appendLimited(s.mid(i, j - i));
                    m_inBuf = true;
                    if (j < n && !emit(QStringView(m_buf), false)) return false;
                }
                if (j < n) {
                    const QChar t = s[j];
                    i = j + 1;
                    if (!isDelim(t)) {
                        if (t == QLatin1Char('\r')) m_crPending = true;
                        if (!endRecord()) return false;
                    } else {
                        m_afterDelim = true;
                    }
                } else {
                    i = j;
                }
                break;
            }
            case Quoted: {
                qsizetype j = i;
                while (j < n && s[j] != m_quote) ++j;
                appendLimited(s.mid(i, j - i));
                if (j < n) {
                    m_state = QuotePending;
                    i = j + 1;
                } else {
                    i = j;
                }
                break;
            }
            case QuotePending:
                if (c == m_quote) {
                    appendLimited(QStringView(&c, 1));
                    m_state = Quoted;
                    ++i;
                } else {
                    m_state = AfterQuote;
                }
                break;
            case AfterQuote:
                if (isDelim(c)) {
                    if (!emitBuffered()) return false;
                    m_afterDelim = true;
                    ++i;
                } else if (c == QLatin1Char('\r') || c == QLatin1Char('\n')) {
                    if (c == QLatin1Char('\r')) m_crPending = true;
                    ++i;
                    if (!emitBuffered() || !endRecord()) return false;
                } else {
                    appendLimited(QStringView(&c, 1));
                    ++i;
                }
                break;
            }
        }
        return true;
    }

    // The end of the text.
    bool finish()
    {
        switch (m_state) {
        case Plain:
            if (!emit(QStringView(m_buf), false)) return false;
            return endRecord();
        case Quoted:
            m_unclosed = true;
            m_unclosedRow = m_row;
            [[fallthrough]];
        case QuotePending:
        case AfterQuote:
            if (!emitBuffered()) return false;
            return endRecord();
        case FieldStart:
            if (m_fields > 0) {
                if (!endField(QStringView(), false)) return false;
                return endRecord();
            }
            return true;
        }
        return true;
    }

    bool truncated() const { return m_truncated; }
    bool unclosedQuote() const { return m_unclosed; }
    qint64 unclosedRow() const { return m_unclosedRow; }

private:
    enum State { FieldStart, Plain, Quoted, QuotePending, AfterQuote };

    void appendLimited(QStringView part)
    {
        if (m_buf.size() >= m_max) {
            if (!part.isEmpty()) m_truncated = true;
            return;
        }
        const qsizetype room = m_max - m_buf.size();
        if (part.size() > room) {
            part = part.left(room);
            m_truncated = true;
        }
        m_buf.append(part);
    }

    bool emit(QStringView text, bool quoted)
    {
        const bool ok = m_sink->field(quint32(m_fields), text, quoted);
        ++m_fields;
        m_state = FieldStart;
        m_inBuf = false;
        return ok;
    }

    bool endField(QStringView text, bool quoted) { return emit(text, quoted); }

    bool emitBuffered()
    {
        if (m_buf.contains(QLatin1Char('\r'))) m_buf.replace(QLatin1String("\r\n"), QLatin1String("\n"));
        const bool ok = emit(QStringView(m_buf), true);
        m_buf.clear();
        return ok;
    }

    bool isDelim(QChar c) const { return m_delims.size() == 1 ? c == m_delims[0] : m_delims.contains(c); }

    bool endRecord()
    {
        m_fields = 0;
        m_afterDelim = false;
        m_state = FieldStart;
        ++m_row;
        return m_sink->endRecord();
    }

    QString m_delims;
    QChar m_quote;
    bool m_hasQuote = true, m_merge = false, m_afterDelim = false;
    Sink *m_sink;
    int m_max;
    State m_state = FieldStart;
    QString m_buf;
    bool m_inBuf = false, m_quoted = false, m_crPending = false, m_truncated = false, m_unclosed = false;
    qint64 m_fields = 0, m_row = 0, m_unclosedRow = 0;
};

// The bytes of the mark at the front of a file that the encoding has, if the file has them (a person may choose an encoding for a file that has no mark; what is not a mark is text).
qint64 markLength(CsvEncoding encoding, const QByteArray &bytes)
{
    switch (encoding) {
    case CsvEncoding::Utf8:
    case CsvEncoding::Utf8Bom: return bytes.startsWith("\xEF\xBB\xBF") ? 3 : 0;
    case CsvEncoding::Utf16Le: return bytes.startsWith("\xFF\xFE") ? 2 : 0;
    case CsvEncoding::Utf16Be: return bytes.startsWith("\xFE\xFF") ? 2 : 0;
    case CsvEncoding::Legacy: return 0;
    }
    return 0;
}

// ---- a date in the order a person chose for its column

// Three numbers with a slash, dash, dot, or space between (or eight digits together) read in the order "MDY", "DMY", or "YMD"; a year of two digits is 2000 to 2029 or 1930 to 1999.
bool dateInOrder(const QString &text, const QString &order, double *serial)
{
    static const QRegularExpression parts(QStringLiteral("^\\s*(\\d{1,4})[\\s/.\\-](\\d{1,2})[\\s/.\\-](\\d{1,4})\\s*$")), packed(QStringLiteral("^\\s*(\\d{8})\\s*$"));
    int a = 0, b = 0, c = 0;
    const QRegularExpressionMatch m = parts.match(text);
    if (m.hasMatch()) {
        a = m.captured(1).toInt();
        b = m.captured(2).toInt();
        c = m.captured(3).toInt();
    } else if (const QRegularExpressionMatch p = packed.match(text); p.hasMatch()) {
        const QString d = p.captured(1);
        if (order == QLatin1String("YMD")) {
            a = d.left(4).toInt();
            b = d.mid(4, 2).toInt();
            c = d.mid(6, 2).toInt();
        } else {
            a = d.left(2).toInt();
            b = d.mid(2, 2).toInt();
            c = d.mid(4, 4).toInt();
        }
    } else {
        return false;
    }
    int year = 0, month = 0, day = 0;
    if (order == QLatin1String("YMD")) {
        year = a;
        month = b;
        day = c;
    } else if (order == QLatin1String("DMY")) {
        day = a;
        month = b;
        year = c;
    } else {
        month = a;
        day = b;
        year = c;
    }
    if (year < 100) year += year < 30 ? 2000 : 1900;
    return jp::calc::dateToSerial(year, month, day, jp::calc::DateSystem::Date1900, serial);
}

// ---- the sheet's cells

class SheetSink : public Sink {
public:
    SheetSink(const QHash<QString, quint32> &formats, const QSet<QString> &words, const QSet<QString> &months, const QLocale &locale, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow,
              Sheet *out, const CsvImport &choices)
        : m_formats(formats), m_words(words), m_months(months), m_limits(limits), m_progress(progress), m_stopAfter(stopAfterRow), m_s(*out), m_choices(choices)
    {
        m_options.locale = locale;
        booleanWordsFor(locale, &m_true, &m_false);
        m_sign = currencySignOf(locale);
        for (const QChar c : m_sign)
            if (c.isLetter()) m_signHasLetters = true;
        // the columns left out: the ones after them are one to the left
        int skipped = 0;
        for (const CsvColumn &c : m_choices.columns) {
            m_shift.push_back(skipped);
            if (c.kind == CsvColumn::Kind::Skip) ++skipped;
        }
        m_shiftRest = skipped;
    }

    bool field(quint32 col, QStringView text, bool quoted) override
    {
        Q_UNUSED(quoted)
        if (m_rec < m_choices.firstRow) return true;   // (the rows before the first are left out)
        if (text.isEmpty()) return true;
        const CsvColumn kind = m_choices.columnAt(col);
        if (kind.kind == CsvColumn::Kind::Skip) return true;
        const quint32 shown = col - quint32(col < quint32(m_shift.size()) ? m_shift[int(col)] : m_shiftRest);
        if (shown >= kMaxColumns) {
            m_tooManyColumns = true;
            return true;
        }
        const qint64 row = m_rec - m_choices.firstRow;
        if (row >= qint64(kMaxRows)) {
            m_tooManyRows = true;
            return false;
        }
        if (kind.kind == CsvColumn::Kind::Text) {
            Cell c;
            c.row = quint32(row);
            c.col = quint16(shown);
            return addText(c, text.toString());
        }
        if (kind.kind == CsvColumn::Kind::Date) {
            Cell c;
            c.row = quint32(row);
            c.col = quint16(shown);
            double serial = 0;
            if (dateInOrder(text.toString(), kind.dateOrder, &serial)) {
                c.type = CellType::Number;
                c.number = serial;
                c.style = m_formats.value(QStringLiteral("yyyy-mm-dd"), 0);
                return add(c, 0);
            }
            return addText(c, text.toString());
        }
        return addField(quint32(row), shown, text);
    }

    bool endRecord() override
    {
        ++m_rec;
        if (m_stopAfter >= 0 && m_rec - m_choices.firstRow > m_stopAfter) {
            m_stoppedEarly = true;
            return false;
        }
        return !m_failed;
    }

    bool tooManyRows() const { return m_tooManyRows; }
    bool tooManyColumns() const { return m_tooManyColumns; }
    bool stoppedEarly() const { return m_stoppedEarly; }
    bool failed() const { return m_failed; }
    qint64 rows() const { return std::max<qint64>(0, m_rec - m_choices.firstRow); }
    qint64 heldBytes() const { return m_held; }

private:
    bool add(Cell c, qint64 extra)
    {
        if (++m_cells > m_limits.cellsPerSheet) {
            m_s.error = QCoreApplication::translate("jp::office::sheet::Csv", "The sheet has more cells than any real file has.");
            m_failed = true;
            return false;
        }
        m_held += 32 + extra;
        if (m_held > m_limits.sheetBytes) {
            m_s.error = QCoreApplication::translate("jp::office::sheet::Csv", "The sheet is larger than any real file's.");
            m_failed = true;
            return false;
        }
        m_s.cells.add(c);
        m_s.extentRows = std::max(m_s.extentRows, c.row + 1);
        m_s.extentCols = std::max(m_s.extentCols, quint32(c.col) + 1);
        if (m_progress && m_cells % 4096 == 0 && !m_progress(m_cells)) {
            m_s.error = QCoreApplication::translate("jp::office::sheet::Csv", "The read was stopped.");
            m_failed = true;
            return false;
        }
        return true;
    }

    bool addText(Cell c, const QString &text)
    {
        c.type = CellType::InlineString;
        c.number = double(m_s.texts.size());
        m_s.texts.append(text);
        m_s.stats.longestString = std::max<qint64>(m_s.stats.longestString, text.size());
        return add(c, text.size() * 2 + 32);
    }

    bool couldBeValue(QStringView field) const
    {
        if (field.front() == QLatin1Char('#')) return true;
        QString stripped;
        if (m_signHasLetters && field.contains(m_sign)) {
            stripped = field.toString();
            stripped.remove(m_sign);
            field = stripped;
        }
        bool digit = false, wordAfterDigit = false;
        QString word;
        auto known = [&] { return word.isEmpty() || m_words.contains(word); };
        // (a month's name joined to a number with nothing between, as a gene is named MARCH1, is no date; and a month's name is no word of the numbers: TRUE, FALSE, and an exponent are)
        auto joined = [&](bool numberFollows) { return !word.isEmpty() && m_months.contains(word) && word.size() > 2 && (wordAfterDigit || numberFollows); };
        QChar before;
        for (const QChar c : field) {
            if (c.isLetter()) {
                if (word.isEmpty()) wordAfterDigit = before.isDigit();
                word += c.toLower();
                if (word.size() > 12) return false;
            } else {
                if (!known() || joined(c.isDigit())) return false;
                word.clear();
                if (c.isDigit()) digit = true;
            }
            before = c;
        }
        if (!known() || joined(false)) return false;
        // (TRUE and FALSE are words and no digit; any other value has a digit)
        return digit || m_words.contains(field.trimmed().toString().toLower());
    }

    bool addField(quint32 row, quint32 col, QStringView field)
    {
        Cell c;
        c.row = row;
        c.col = quint16(col);
        // text that stays text: an apostrophe in front (kept as it is), and digits that a number would damage
        if (field.front() == QLatin1Char('\'') || keepsAsText(field)) return addText(c, field.toString());
        const bool formula = field.front() == QLatin1Char('=') && field.size() > 1;
        // true and false in the person's language (in English they are read by the typed entry)
        if (!m_true.isEmpty() && field.front().isLetter()) {
            const QStringView word = field.trimmed();
            if (word.size() == m_true.size() || word.size() == m_false.size()) {
                const QString folded = word.toString().toCaseFolded();
                if (folded == m_true || folded == m_false) {
                    c.type = CellType::Bool;
                    c.number = folded == m_true ? 1 : 0;
                    return add(c, 0);
                }
            }
        }
        if (!formula) {
            // (only what could be a number, a date, TRUE or FALSE, or an error is looked at; a text with a word of its own in it is text)
            if (field.size() > 64 || !couldBeValue(field)) return addText(c, field.toString());
        }
        const QString typed = field.toString();
        const EntryResult r = readEntry(typed, m_options);
        if (!r.ok) return addText(c, typed);
        switch (r.entry.kind) {
        case Entry::Kind::Number:
            // (a date is made of what was typed only when it has a year with it)
            if (r.entry.formatCode.startsWith(QLatin1String("m/d/yyyy")) && !dateHasYear(typed, m_months)) return addText(c, typed);
            c.type = CellType::Number;
            c.number = r.entry.number;
            if (!r.entry.formatCode.isEmpty()) c.style = m_formats.value(r.entry.formatCode, 0);
            return add(c, 0);
        case Entry::Kind::Bool:
            c.type = CellType::Bool;
            c.number = r.entry.number;
            return add(c, 0);
        case Entry::Kind::Error:
            c.type = CellType::Error;
            c.number = double(int(r.entry.error));
            return add(c, 0);
        case Entry::Kind::Formula: {
            if (!formula) return addText(c, typed);   // (a sign and more that reads as a formula is text here: -hello, +49 30 1234)
            c.flags |= HasFormula;
            m_s.formulaTexts.append(r.entry.text);
            Formula f;
            f.text = qint32(m_s.formulaTexts.size() - 1);
            m_s.formulas.append(f);
            c.formula = qint32(m_s.formulas.size() - 1);
            ++m_s.formulaCells;
            m_s.stats.longestFormula = std::max<qint64>(m_s.stats.longestFormula, r.entry.text.size());
            return add(c, r.entry.text.size() * 2 + 64);
        }
        case Entry::Kind::Text: return addText(c, r.entry.text);
        case Entry::Kind::Empty: return true;
        }
        return true;
    }

    const QHash<QString, quint32> &m_formats;
    const QSet<QString> &m_words;
    const QSet<QString> &m_months;
    QString m_sign;
    QString m_true, m_false;   // (the words for true and false in the person's language)
    bool m_signHasLetters = false;
    Limits m_limits;
    std::function<bool(qint64)> m_progress;
    qint64 m_stopAfter;
    Sheet &m_s;
    EntryOptions m_options;
    qint64 m_rec = 0, m_cells = 0, m_held = 0;
    CsvImport m_choices;
    QVector<int> m_shift;
    int m_shiftRest = 0;
    bool m_tooManyRows = false, m_tooManyColumns = false, m_stoppedEarly = false, m_failed = false;
};

// The sheet's name from the file's: the name without its ending, as a sheet may be named.
QString sheetNameOf(const QString &fileName)
{
    QString name = QFileInfo(fileName).completeBaseName();
    for (QChar &c : name)
        if (QStringLiteral("[]:*?/\\").contains(c) || c.unicode() < 32) c = QLatin1Char('_');
    name = name.left(31).trimmed();
    while (name.startsWith(QLatin1Char('\''))) name.remove(0, 1);
    while (name.endsWith(QLatin1Char('\''))) name.chop(1);
    return name.isEmpty() ? QStringLiteral("Sheet1") : name;
}

} // namespace

bool isDelimitedName(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix();
    return suffix.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0 || suffix.compare(QLatin1String("tsv"), Qt::CaseInsensitive) == 0
           || suffix.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0;
}

int legacyCodePageFor(const QLocale &locale)
{
    switch (locale.language()) {
    case QLocale::Japanese: return 932;
    case QLocale::Chinese: return locale.script() == QLocale::TraditionalChineseScript || locale.territory() == QLocale::Taiwan || locale.territory() == QLocale::HongKong ? 950 : 936;
    case QLocale::Korean: return 949;
    case QLocale::Russian:
    case QLocale::Ukrainian:
    case QLocale::Bulgarian:
    case QLocale::Belarusian:
    case QLocale::Macedonian:
    case QLocale::Kazakh:
    case QLocale::Mongolian: return 1251;
    case QLocale::Serbian: return locale.script() == QLocale::LatinScript ? 1250 : 1251;
    case QLocale::Greek: return 1253;
    case QLocale::Turkish:
    case QLocale::Azerbaijani: return 1254;
    case QLocale::Hebrew: return 1255;
    case QLocale::Arabic:
    case QLocale::Persian:
    case QLocale::Urdu: return 1256;
    case QLocale::Estonian:
    case QLocale::Latvian:
    case QLocale::Lithuanian: return 1257;
    case QLocale::Vietnamese: return 1258;
    case QLocale::Thai: return 874;
    case QLocale::Polish:
    case QLocale::Czech:
    case QLocale::Slovak:
    case QLocale::Hungarian:
    case QLocale::Slovenian:
    case QLocale::Croatian:
    case QLocale::Romanian:
    case QLocale::Albanian:
    case QLocale::Bosnian: return 1250;
    default: return 1252;
    }
}

CsvForm detectCsvForm(const QByteArray &bytes, const QString &fileName, const QLocale &locale)
{
    CsvForm form;
    qsizetype skip = 0;
    if (bytes.startsWith("\xEF\xBB\xBF")) {
        form.encoding = CsvEncoding::Utf8Bom;
        skip = 3;
    } else if (bytes.startsWith("\xFF\xFE")) {
        form.encoding = CsvEncoding::Utf16Le;
        skip = 2;
    } else if (bytes.startsWith("\xFE\xFF")) {
        form.encoding = CsvEncoding::Utf16Be;
        skip = 2;
    } else if (validUtf8(bytes.constData(), utf8Sample(bytes))) {
        form.encoding = CsvEncoding::Utf8;
    } else {
        form.encoding = CsvEncoding::Legacy;
        form.codePage = legacyCodePageFor(locale);
    }
    const QString sample = decodeSample(bytes, skip, 1 << 16, form);
    // `sep=;` on the first line
    if (sample.startsWith(QLatin1String("sep=")) && sample.size() >= 5) {
        const qsizetype end = sample.indexOf(QRegularExpression(QStringLiteral("[\r\n]")));
        if (end == 5 || (end < 0 && sample.size() == 5)) {
            form.delimiter = sample[4];
            form.sepLine = true;
            return form;
        }
    }
    form.delimiter = chooseDelimiter(sample, fileName, locale);
    return form;
}

std::shared_ptr<const CsvBook> CsvBook::open(const QByteArray &file, const QString &fileName, const Limits &limits, const QLocale &locale, WorkbookInfo *info, SharedStrings *shared,
                                              Styles *styles, QString *error)
{
    // (a zero byte in the first piece of a file that is not UTF-16: a binary file, not text)
    if (!file.startsWith("\xFF\xFE") && !file.startsWith("\xFE\xFF") && file.left(8192).contains('\0')) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Csv", "This file is not a text file of values separated by commas, semicolons, or tabs.");
        return nullptr;
    }
    std::shared_ptr<CsvBook> book(new CsvBook);
    book->m_data = file;
    book->m_locale = locale;
    book->m_words = knownWordsOf(locale, &book->m_months);
    book->m_form = detectCsvForm(file, fileName, locale);
    if (limits.csv) {
        book->m_import = *limits.csv;
        const CsvImport &c = book->m_import;
        if (!c.separators.isEmpty()) book->m_form.delimiter = c.separators[0];
        if (c.encodingSet) {
            book->m_form.encoding = c.encoding;
            book->m_form.codePage = c.codePage;
        }
        // (the choices are bounded: a list of columns is only so long)
        if (book->m_import.columns.size() > kMaxCsvColumnChoices) book->m_import.columns.resize(kMaxCsvColumnChoices);
        if (book->m_import.firstRow < 0) book->m_import.firstRow = 0;
    }
    qint64 skip = markLength(book->m_form.encoding, file);
    if (book->m_form.sepLine) {
        // (the line's end: the data begins after it)
        const bool wide = book->m_form.encoding == CsvEncoding::Utf16Le || book->m_form.encoding == CsvEncoding::Utf16Be;
        const qint64 unit = wide ? 2 : 1;
        qint64 at = skip + 5 * unit;
        auto unitAt = [&](qint64 p) -> int {
            if (p + unit > file.size()) return -1;
            if (!wide) return quint8(file[p]);
            const quint8 a = quint8(file[p]), b = quint8(file[p + 1]);
            return book->m_form.encoding == CsvEncoding::Utf16Le ? (a | (b << 8)) : ((a << 8) | b);
        };
        if (unitAt(at) == '\r') at += unit;
        if (unitAt(at) == '\n') at += unit;
        skip = std::min<qint64>(at, file.size());
    }
    book->m_skip = skip;
    // ---- the workbook: one sheet named for the file, and the formats a typed entry asks for
    *info = WorkbookInfo();
    SheetEntry sheet;
    sheet.name = sheetNameOf(fileName);
    sheet.sheetId = 1;
    info->sheets.append(sheet);
    *shared = SharedStrings();
    *styles = Styles();
    FontSpec font;
    font.name = QStringLiteral("Calibri");
    font.size = 11;
    styles->fonts.append(font);
    styles->fills.append(FillSpec());
    FillSpec gray;
    gray.pattern = PatternKind::Gray125;
    styles->fills.append(gray);
    styles->borders.append(BorderSpec());
    styles->formats.append(CellFormat());
    styles->cellXfs.append(0);
    int nextId = 164;
    QStringList codes = entryFormatCodes(locale);
    codes << QStringLiteral("yyyy-mm-dd");   // (the format of a column a person chose to read as dates)
    for (const QString &code : codes) {
        if (book->m_formats.contains(code)) continue;
        int id = -1;
        for (int b = 0; b <= 58; ++b)
            if (jp::calc::builtinFormatCode(b) == code) {
                id = b;
                break;
            }
        if (id < 0) {
            id = nextId++;
            styles->numFmts.insert(id, code);
            styles->chars += code.size();
        }
        CellFormat f;
        f.numFmtId = id;
        styles->formats.append(f);
        styles->cellXfs.append(id);
        book->m_formats.insert(code, quint32(styles->formats.size() - 1));
    }
    return book;
}

bool CsvBook::readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows, Sheet *out) const
{
    *out = Sheet();
    if (index != 0) {
        out->error = QCoreApplication::translate("jp::office::sheet::Csv", "There is no such sheet.");
        return false;
    }
    qint64 stopAfter = stopAfterRow;
    if (screenRows > 0 && (stopAfter < 0 || screenRows - 1 < stopAfter)) stopAfter = screenRows - 1;
    out->format.defaultRowHeight = 15;
    out->view.tabSelected = true;
    SheetSink sink(m_formats, m_words, m_months, m_locale, limits, progress, stopAfter, out, m_import);
    const QString separators = m_import.separators.isEmpty() ? QString(m_form.delimiter) : m_import.separators;
    Scanner scanner(separators, m_import.quote, m_import.mergeSeparators, &sink, kMaxFieldChars);
    bool whole = true;
    const qint64 total = m_data.size();
    qint64 pos = m_skip;
    constexpr qint64 kPiece = 1 << 20;
    const bool wide = m_form.encoding == CsvEncoding::Utf16Le || m_form.encoding == CsvEncoding::Utf16Be;
    while (pos < total) {
        qint64 end = std::min(total, pos + kPiece);
        QString piece;
        if (wide) {
            end = pos + ((end - pos) & ~qint64(1));
            if (end == pos) break;
            piece = decodeSample(m_data, pos, end - pos, m_form);
        } else if (m_form.encoding == CsvEncoding::Legacy) {
            // a piece ends at a line's end (the byte after a line break can't be the second byte of a pair in any of the pages)
            if (end < total) {
                qint64 cut = end;
                while (cut > pos && m_data[cut - 1] != '\n' && m_data[cut - 1] != '\r') --cut;
                if (cut > pos) end = cut;
            }
            piece = decodeSample(m_data, pos, end - pos, m_form);
        } else {
            // (not in the middle of a character)
            while (end < total && end > pos && (quint8(m_data[end]) & 0xC0) == 0x80) --end;
            if (end == pos) end = std::min(total, pos + kPiece);
            piece = QString::fromUtf8(m_data.constData() + pos, end - pos);
        }
        pos = end;
        if (!scanner.feed(piece)) {
            whole = false;
            break;
        }
    }
    if (whole) whole = scanner.finish();
    // (the scan ended before the file did: a limit, or the rows asked for)
    const bool stoppedByRequest = sink.stoppedEarly() && !sink.failed() && !sink.tooManyRows();
    if (!whole && stoppedByRequest) out->partial = true;
    out->cells.finish();
    if (out->extentRows > 0) {
        out->dimension.row1 = 0;
        out->dimension.col1 = 0;
        out->dimension.row2 = out->extentRows - 1;
        out->dimension.col2 = out->extentCols - 1;
        out->dimension.valid = true;
    }
    QStringList problems;
    if (!out->error.isEmpty()) problems << out->error;
    if (sink.tooManyRows()) problems << QCoreApplication::translate("jp::office::sheet::Csv", "This file has more rows than a sheet holds, which is 1,048,576. The rows after that were not read.");
    if (sink.tooManyColumns()) problems << QCoreApplication::translate("jp::office::sheet::Csv", "Some rows have more columns than a sheet holds, which is 16,384. The columns after that were not read.");
    if (!out->partial) {
        if (scanner.truncated()) problems << QCoreApplication::translate("jp::office::sheet::Csv", "Some text is longer than a cell holds, which is 32,767 characters, and was cut.");
        if (scanner.unclosedQuote()) problems << QCoreApplication::translate("jp::office::sheet::Csv", "A quote in row %1 is never closed, so everything after it is in one cell.").arg(scanner.unclosedRow() + 1);
    }
    out->error = problems.join(QLatin1Char(' '));
    out->stats.heldBytes = sink.heldBytes();
    return out->error.isEmpty();
}

namespace {

// The sink of a preview: the text of the first fields of the first rows, as the choices read them.
class PreviewSink : public Sink {
public:
    PreviewSink(const CsvImport &choices, QVector<QStringList> *rows, int *widest) : m_choices(choices), m_rows(rows), m_widest(widest) {}
    bool field(quint32 col, QStringView text, bool quoted) override
    {
        Q_UNUSED(quoted)
        if (m_rec < m_choices.firstRow) return true;
        if (col >= quint32(kCsvPreviewColumns)) return true;
        const qint64 row = m_rec - m_choices.firstRow;
        if (row >= kCsvPreviewRows) return false;
        while (m_rows->size() <= row) m_rows->append(QStringList());
        QStringList &fields = (*m_rows)[int(row)];
        while (fields.size() <= qsizetype(col)) fields.append(QString());
        QString shown = text.left(kCsvPreviewFieldChars).toString();
        const CsvColumn kind = m_choices.columnAt(col);
        if (kind.kind == CsvColumn::Kind::Date && !shown.isEmpty()) {
            double serial = 0;
            int y = 0, m = 0, d = 0;
            if (dateInOrder(text.toString(), kind.dateOrder, &serial) && jp::calc::serialToDate(serial, jp::calc::DateSystem::Date1900, &y, &m, &d))
                shown = QStringLiteral("%1-%2-%3").arg(y, 4, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(d, 2, 10, QLatin1Char('0'));
        }
        fields[int(col)] = shown;
        *m_widest = std::max(*m_widest, int(col) + 1);
        return true;
    }
    bool endRecord() override
    {
        ++m_rec;
        return m_rec - m_choices.firstRow < kCsvPreviewRows;
    }

private:
    CsvImport m_choices;
    QVector<QStringList> *m_rows;
    int *m_widest;
    qint64 m_rec = 0;
};

} // namespace

QVector<QStringList> csvPreview(const QByteArray &file, const QString &fileName, const CsvImport &choices, const QLocale &locale, CsvForm *formOut, int *columnsOut)
{
    QVector<QStringList> rows;
    int widest = 0;
    const QByteArray head = file.left(kCsvPreviewBytes);
    CsvForm form = detectCsvForm(head, fileName, locale);
    CsvImport c = choices;
    if (!c.separators.isEmpty()) form.delimiter = c.separators[0];
    if (c.encodingSet) {
        form.encoding = c.encoding;
        form.codePage = c.codePage;
    }
    if (c.firstRow < 0) c.firstRow = 0;
    if (formOut) *formOut = form;
    if (head.isEmpty() || (!head.startsWith("\xFF\xFE") && !head.startsWith("\xFE\xFF") && head.left(8192).contains('\0'))) {
        if (columnsOut) *columnsOut = 0;
        return rows;
    }
    qint64 skip = markLength(form.encoding, head);
    if (form.sepLine) {
        const qsizetype nl = head.indexOf('\n', skip);
        skip = nl < 0 ? head.size() : nl + 1;
    }
    const bool wide = form.encoding == CsvEncoding::Utf16Le || form.encoding == CsvEncoding::Utf16Be;
    qint64 count = head.size() - skip;
    if (wide) count &= ~qint64(1);
    QString text;
    if (wide || form.encoding == CsvEncoding::Legacy) text = decodeSample(head, skip, count, form);
    else text = QString::fromUtf8(head.constData() + skip, qsizetype(count));
    PreviewSink sink(c, &rows, &widest);
    const QString separators = c.separators.isEmpty() ? QString(form.delimiter) : c.separators;
    Scanner scanner(separators, c.quote, c.mergeSeparators, &sink, kCsvPreviewFieldChars * 4);
    if (scanner.feed(text)) scanner.finish();   // (the last row of a file cut short is shown as far as it goes)
    if (columnsOut) *columnsOut = widest;
    return rows;
}

bool Workbook::openDelimited(const QByteArray &file, const QString &path, QString *error, const Limits &limits)
{
    *this = Workbook();
    m_limits = limits;
    const auto book = CsvBook::open(file, path, limits, QLocale(), &m_info, &m_shared, &m_styles, error);
    if (!book) {
        *this = Workbook();
        return false;
    }
    m_foreign = book;
    nameBaseParts();
    return true;
}

} // namespace jp::office::sheet
