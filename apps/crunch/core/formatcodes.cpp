#include "core/formatcodes.h"

#include "calc/numfmt.h"

#include <QCoreApplication>
#include <QRegularExpression>

namespace jp::crunch {

namespace {

QString tr(const char *s) { return QCoreApplication::translate("jp::crunch::FormatCodes", s); }

// The code with what is not a format's own taken out: quoted text, an escaped character, a bracket (color, condition, locale), a pad or a fill. What is left says what kind it is.
QString skeleton(const QString &code)
{
    QString out;
    for (int i = 0; i < code.size(); ++i) {
        const QChar c = code[i];
        if (c == QLatin1Char('"')) {
            const int end = code.indexOf(QLatin1Char('"'), i + 1);
            i = end < 0 ? code.size() : end;
            out += QLatin1Char('"');   // (a mark that there was a text)
        } else if (c == QLatin1Char('\\') || c == QLatin1Char('_') || c == QLatin1Char('*')) {
            if (c == QLatin1Char('*')) out += QLatin1Char('*');
            ++i;
        } else if (c == QLatin1Char('[')) {
            const int end = code.indexOf(QLatin1Char(']'), i + 1);
            const QString inside = end < 0 ? QString() : code.mid(i + 1, end - i - 1);
            // (an elapsed time, [h] [m] [s], is a time; the others are not part of what the format shows)
            if (inside.size() <= 2 && !inside.isEmpty() && QString::fromLatin1("hmsHMS").contains(inside[0]) && inside.count(inside[0]) == inside.size()) out += inside;
            else if (inside.startsWith(QLatin1Char('$'))) out += QLatin1Char('$');
            i = end < 0 ? code.size() : end;
        } else {
            out += c;
        }
    }
    return out;
}

QString firstSection(const QString &code)
{
    bool quoted = false;
    for (int i = 0; i < code.size(); ++i) {
        const QChar c = code[i];
        if (c == QLatin1Char('"')) quoted = !quoted;
        else if (c == QLatin1Char('\\') || c == QLatin1Char('_')) ++i;
        else if (c == QLatin1Char(';') && !quoted) return code.left(i);
    }
    return code;
}

bool hasDateOrTime(const QString &skel, bool *date, bool *time)
{
    // (outside text and brackets: a d or y is a date; an h or s, and AM/PM, a time; an m is a month in a date, and a minute where an h or an s is and no d or y)
    QString s = skel;
    s.remove(QStringLiteral("General"), Qt::CaseInsensitive);
    bool d = false, t = false, m = false;
    for (int i = 0; i < s.size(); ++i) {
        const QChar c = s[i].toLower();
        if (c == QLatin1Char('d') || c == QLatin1Char('y')) d = true;
        else if (c == QLatin1Char('h') || c == QLatin1Char('s')) t = true;
        else if (c == QLatin1Char('m')) m = true;
        else if (c == QLatin1Char('a') && s.mid(i).startsWith(QLatin1String("AM/PM"), Qt::CaseInsensitive)) {
            t = true;
            i += 4;
        }
    }
    *date = d || (m && !t);
    *time = t && !*date;
    return *date || t;
}

QString decimalsOf(int n) { return n > 0 ? QLatin1Char('.') + QString(n, QLatin1Char('0')) : QString(); }

QString symbolText(const QString &symbol)
{
    QString q = symbol;
    q.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + q + QLatin1Char('"');
}

// The positive pattern of Number and Currency: the digits, and the symbol before or after.
QString positivePattern(const NumberOptions &o, bool currency)
{
    const QString digits = (o.thousands ? QStringLiteral("#,##0") : QStringLiteral("0")) + decimalsOf(o.decimals);
    if (!currency || o.symbol.isEmpty()) return digits;
    return o.symbolFirst ? symbolText(o.symbol) + digits : digits + QStringLiteral("\\ ") + symbolText(o.symbol);
}

QString numberLike(const NumberOptions &o, bool currency)
{
    const QString p = positivePattern(o, currency);
    switch (o.negative) {
    case 1: return p + QStringLiteral(";[Red]-") + p;
    case 2: return p + QStringLiteral("_);\\(") + p + QStringLiteral("\\)");
    case 3: return p + QStringLiteral("_);[Red]\\(") + p + QStringLiteral("\\)");
    default: return p;
    }
}

QString accounting(const NumberOptions &o)
{
    const QString digits = QStringLiteral("#,##0") + decimalsOf(o.decimals);
    const QString dash = QStringLiteral("\"-\"") + QString(o.decimals, QLatin1Char('?'));
    QString lead, tail;
    if (!o.symbol.isEmpty()) (o.symbolFirst ? lead : tail) = symbolText(o.symbol);
    const QString pre = QStringLiteral("_(") + lead + QStringLiteral("* ");
    const QString post = tail.isEmpty() ? QString() : QStringLiteral(" ") + tail;
    return pre + digits + post + QStringLiteral("_);") + pre + QStringLiteral("\\(") + digits + QStringLiteral("\\)") + post + QStringLiteral(";") + pre + dash + post + QStringLiteral("_);_(@_)");
}

} // namespace

QString categoryName(NumberCategory c)
{
    switch (c) {
    case NumberCategory::General: return tr("General");
    case NumberCategory::Number: return tr("Number");
    case NumberCategory::Currency: return tr("Currency");
    case NumberCategory::Accounting: return tr("Accounting");
    case NumberCategory::Date: return tr("Date");
    case NumberCategory::Time: return tr("Time");
    case NumberCategory::Percentage: return tr("Percentage");
    case NumberCategory::Fraction: return tr("Fraction");
    case NumberCategory::Scientific: return tr("Scientific");
    case NumberCategory::Text: return tr("Text");
    case NumberCategory::Special: return tr("Special");
    case NumberCategory::Custom: return tr("Custom");
    }
    return QString();
}

QString codeFor(NumberCategory category, const NumberOptions &o)
{
    switch (category) {
    case NumberCategory::General: return QStringLiteral("General");
    case NumberCategory::Number: return numberLike(o, false);
    case NumberCategory::Currency: return numberLike(o, true);
    case NumberCategory::Accounting: return accounting(o);
    case NumberCategory::Percentage: return QStringLiteral("0") + decimalsOf(o.decimals) + QLatin1Char('%');
    case NumberCategory::Scientific: return QStringLiteral("0") + decimalsOf(o.decimals) + QStringLiteral("E+00");
    case NumberCategory::Text: return QStringLiteral("@");
    default: return QString();
    }
}

bool optionsOf(const QString &code, NumberCategory *category, NumberOptions *options)
{
    NumberOptions o;
    const QString first = firstSection(code);
    // the places after the decimal mark: the placeholders after the first "." outside text
    const QString skel = skeleton(first);
    const int dot = skel.indexOf(QLatin1Char('.'));
    o.decimals = 0;
    if (dot >= 0)
        for (int i = dot + 1; i < skel.size() && (skel[i] == QLatin1Char('0') || skel[i] == QLatin1Char('#') || skel[i] == QLatin1Char('?')); ++i) ++o.decimals;
    o.thousands = skel.contains(QStringLiteral("#,##0")) || skel.contains(QStringLiteral("0,000"));
    // the symbol: a quoted text next to the digits (the first one that is not a plain dash), or [$...] or a bare symbol character
    static const QRegularExpression quote(QStringLiteral("\"((?:[^\"]|\"\")*)\""));
    QString symbol;
    bool first1 = true;
    const auto m = quote.match(first);
    if (m.hasMatch() && m.captured(1) != QLatin1String("-")) {
        symbol = m.captured(1);
        symbol.replace(QStringLiteral("\"\""), QStringLiteral("\""));
        const int at = m.capturedStart();
        const int digitAt = first.indexOf(QRegularExpression(QStringLiteral("[0#?]")));
        first1 = digitAt < 0 || at < digitAt;
    }
    o.symbol = symbol;
    o.symbolFirst = first1;
    // the negative style from the other sections
    const QString rest = code.mid(first.size());
    o.negative = 0;
    if (rest.startsWith(QLatin1Char(';'))) {
        if (rest.contains(QStringLiteral("[Red]\\("))) o.negative = 3;
        else if (rest.contains(QStringLiteral("\\("))) o.negative = 2;
        else if (rest.contains(QStringLiteral("[Red]"))) o.negative = 1;
    }
    const NumberCategory cat = categoryOf(code);
    if (category) *category = cat;
    if (options) *options = o;
    switch (cat) {
    case NumberCategory::Number:
    case NumberCategory::Currency:
    case NumberCategory::Accounting:
    case NumberCategory::Percentage:
    case NumberCategory::Scientific:
        // (the options make exactly this code, or it is not one the dialog makes)
        return codeFor(cat, o) == code;
    default: return false;
    }
}

NumberCategory categoryOf(const QString &code)
{
    const QString trimmed = code.trimmed();
    if (trimmed.isEmpty() || trimmed.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0) return NumberCategory::General;
    if (trimmed == QLatin1String("@")) return NumberCategory::Text;
    for (const FormatType &t : typesFor(NumberCategory::Special, QLocale::c()))
        if (t.code == trimmed) return NumberCategory::Special;
    // (a condition on a part, [<0], makes a format of the person's own)
    if (trimmed.contains(QRegularExpression(QStringLiteral("\\[[<>=]")))) return NumberCategory::Custom;
    const QString first = firstSection(trimmed);
    const QString skel = skeleton(first);
    bool date = false, time = false;
    if (hasDateOrTime(skel, &date, &time)) return date ? NumberCategory::Date : NumberCategory::Time;
    // an accounting code pads the symbol and the number apart with a fill
    if (skel.contains(QLatin1Char('*')) && skel.contains(QStringLiteral("#,##0"))) return NumberCategory::Accounting;
    if (skel.contains(QLatin1Char('%'))) return NumberCategory::Percentage;
    if (skel.contains(QStringLiteral("E+"), Qt::CaseInsensitive) || skel.contains(QStringLiteral("E-"), Qt::CaseInsensitive)) return NumberCategory::Scientific;
    if (skel.contains(QRegularExpression(QStringLiteral("[?#0]/[?#0\\d]")))) return NumberCategory::Fraction;
    if (skel.contains(QLatin1Char('@'))) return NumberCategory::Custom;
    if (skel.contains(QRegularExpression(QStringLiteral("[0#?]")))) {
        // a currency: a symbol (anywhere, in quotes or not) or a short quoted text next to the digits; any longer text is the person's own
        static const QRegularExpression symbols(QStringLiteral("[$\u20AC\u00A3\u00A5\u20B9\u20A9]"));
        if (symbols.match(first).hasMatch()) return NumberCategory::Currency;
        static const QRegularExpression word(QStringLiteral("\"([^\"]+)\""));
        const auto m = word.match(first);
        if (m.hasMatch() && m.captured(1).size() <= 3 && m.captured(1) != QLatin1String("-") && !skel.contains(QLatin1Char('*'))) return NumberCategory::Currency;
    }
    if (skel.contains(QRegularExpression(QStringLiteral("^[#,0]*0(\\.[0#?]+)?$"))) || skel.contains(QRegularExpression(QStringLiteral("^#,##0(\\.[0#?]+)?$")))) return NumberCategory::Number;
    return NumberCategory::Custom;
}

QVector<FormatType> typesFor(NumberCategory category, const QLocale &locale)
{
    QVector<FormatType> t;
    switch (category) {
    case NumberCategory::Date:
        t = {{jp::calc::shortDateFormatCode(locale), tr("Short date")},
             {QStringLiteral("dddd, mmmm d, yyyy"), tr("Long date")},
             {QStringLiteral("d-mmm-yy"), QString()},
             {QStringLiteral("d-mmm"), QString()},
             {QStringLiteral("mmm-yy"), QString()},
             {QStringLiteral("mmmm d, yyyy"), QString()},
             {QStringLiteral("mmmm-yy"), QString()},
             {QStringLiteral("m/d"), QString()},
             {QStringLiteral("mm-dd-yy"), QString()},
             {QStringLiteral("yyyy-mm-dd"), tr("ISO date")},
             {QStringLiteral("dd/mm/yyyy"), QString()},
             {QStringLiteral("d mmmm yyyy"), QString()},
             {QStringLiteral("m/d/yyyy h:mm"), tr("Date and time")},
             {QStringLiteral("yyyy-mm-dd h:mm"), QString()}};
        break;
    case NumberCategory::Time:
        t = {{QStringLiteral("h:mm"), QString()},
             {QStringLiteral("h:mm:ss"), QString()},
             {QStringLiteral("h:mm AM/PM"), QString()},
             {QStringLiteral("h:mm:ss AM/PM"), QString()},
             {QStringLiteral("[h]:mm:ss"), tr("Elapsed hours")},
             {QStringLiteral("[h]:mm"), QString()},
             {QStringLiteral("mm:ss"), QString()},
             {QStringLiteral("mm:ss.0"), QString()}};
        break;
    case NumberCategory::Fraction:
        t = {{QStringLiteral("# ?/?"), tr("Up to one digit")},
             {QStringLiteral("# ??/??"), tr("Up to two digits")},
             {QStringLiteral("# ???/???"), tr("Up to three digits")},
             {QStringLiteral("# ?/2"), tr("As halves")},
             {QStringLiteral("# ?/4"), tr("As quarters")},
             {QStringLiteral("# ?/8"), tr("As eighths")},
             {QStringLiteral("# ??/16"), tr("As sixteenths")},
             {QStringLiteral("# ?/10"), tr("As tenths")},
             {QStringLiteral("# ??/100"), tr("As hundredths")}};
        break;
    case NumberCategory::Special:
        t = {{QStringLiteral("00000"), tr("ZIP code")},
             {QStringLiteral("00000\\-0000"), tr("ZIP code + 4")},
             {QStringLiteral("[<=9999999]###\\-####;\\(###\\) ###\\-####"), tr("Phone number")},
             {QStringLiteral("000\\-00\\-0000"), tr("Social security number")}};
        break;
    case NumberCategory::Custom:
        t = {{QStringLiteral("General"), QString()},
             {QStringLiteral("0"), QString()},
             {QStringLiteral("0.00"), QString()},
             {QStringLiteral("#,##0"), QString()},
             {QStringLiteral("#,##0.00"), QString()},
             {QStringLiteral("0%"), QString()},
             {QStringLiteral("0.00%"), QString()},
             {QStringLiteral("0.00E+00"), QString()},
             {QStringLiteral("# ?/?"), QString()},
             {QStringLiteral("@"), QString()},
             {QStringLiteral("0.00;[Red]-0.00"), QString()},
             {QStringLiteral("#,##0.00_);\\(#,##0.00\\)"), QString()},
             {QStringLiteral("[Red][<0]0;[Blue][>0]0;0"), QString()},
             {QStringLiteral("\"Total: \"#,##0.00"), QString()},
             {QStringLiteral("000000"), QString()},
             {jp::calc::shortDateFormatCode(locale), QString()},
             {QStringLiteral("h:mm:ss"), QString()}};
        break;
    default: break;
    }
    return t;
}

int negativeStyles() { return 4; }

QString withDecimals(const QString &code, int delta)
{
    if (delta == 0) return code;
    const QString trimmed = code.trimmed();
    if (trimmed.isEmpty() || trimmed.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0) return delta > 0 ? QStringLiteral("0.0") : QStringLiteral("General");
    const NumberCategory category = categoryOf(trimmed);
    if (category == NumberCategory::Date || category == NumberCategory::Time || category == NumberCategory::Text || category == NumberCategory::Fraction || category == NumberCategory::Special) return code;
    // every section that holds a number gets a place more or less: the run of placeholders at the end of its digits (after the "." when it has one, else after its last 0 or #)
    QStringList sections;
    QString current;
    bool quoted = false;
    for (int i = 0; i < trimmed.size(); ++i) {
        const QChar c = trimmed[i];
        if (c == QLatin1Char('"')) quoted = !quoted;
        if (!quoted && (c == QLatin1Char('\\') || c == QLatin1Char('_')) && i + 1 < trimmed.size()) {
            current += c;
            current += trimmed[++i];
            continue;
        }
        if (c == QLatin1Char(';') && !quoted) {
            sections << current;
            current.clear();
        } else {
            current += c;
        }
    }
    sections << current;
    bool changed = false;
    for (QString &s : sections) {
        // find the digits outside quotes and brackets: positions of the first '.' and of the last placeholder
        int dot = -1, lastDigit = -1, firstDigit = -1;
        bool q = false, bracket = false;
        for (int i = 0; i < s.size(); ++i) {
            const QChar c = s[i];
            if (c == QLatin1Char('"')) q = !q;
            else if (q) continue;
            else if (c == QLatin1Char('\\') || c == QLatin1Char('_') || c == QLatin1Char('*')) ++i;
            else if (c == QLatin1Char('[')) bracket = true;
            else if (c == QLatin1Char(']')) bracket = false;
            else if (!bracket && (c == QLatin1Char('0') || c == QLatin1Char('#') || c == QLatin1Char('?'))) {
                if (firstDigit < 0) firstDigit = i;
                lastDigit = i;
            } else if (!bracket && c == QLatin1Char('.') && dot < 0 && firstDigit >= 0) dot = i;
        }
        if (lastDigit < 0) continue;
        // where the decimals end: the run of placeholders that follows the dot
        int end = lastDigit;
        if (dot >= 0) {
            int j = dot + 1;
            while (j < s.size() && (s[j] == QLatin1Char('0') || s[j] == QLatin1Char('#') || s[j] == QLatin1Char('?'))) ++j;
            end = j - 1;
        }
        if (delta > 0) {
            if (dot < 0) s.insert(end + 1, QStringLiteral(".0"));
            else s.insert(end + 1, QLatin1Char('0'));
            changed = true;
        } else if (dot >= 0) {
            if (end == dot + 1) s.remove(dot, 2);
            else s.remove(end, 1);
            changed = true;
        }
    }
    if (!changed) return code;
    return sections.join(QLatin1Char(';'));
}

QString currencySymbolOf(const QLocale &locale, bool *first)
{
    const QString symbol = locale.currencySymbol(QLocale::CurrencySymbol);
    const QString sample = locale.toCurrencyString(1234.5, symbol);
    if (first) *first = sample.startsWith(symbol) || sample.startsWith(QLatin1Char('-') + symbol);
    return symbol;
}

QString currencyCode(const QLocale &locale)
{
    NumberOptions o;
    o.decimals = 2;
    o.thousands = true;
    o.symbol = currencySymbolOf(locale, &o.symbolFirst);
    if (o.symbol.isEmpty()) o.symbol = QStringLiteral("$");
    o.negative = 0;
    return codeFor(NumberCategory::Currency, o);
}

QString commaCode()
{
    NumberOptions o;
    o.decimals = 2;
    return codeFor(NumberCategory::Accounting, o);
}

QString percentCode()
{
    NumberOptions o;
    o.decimals = 0;
    return codeFor(NumberCategory::Percentage, o);
}

} // namespace jp::crunch
