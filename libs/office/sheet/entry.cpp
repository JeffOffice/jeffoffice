#include "sheet/entry.h"

#include "calc/formula.h"
#include "calc/functions.h"
#include "calc/numfmt.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QVector>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::office::sheet::Entry", text); }

bool isDigit(QChar c) { return c.unicode() >= '0' && c.unicode() <= '9'; }

bool isSpaceLike(QChar c) { return c.isSpace() || c.unicode() == 0x202F || c.unicode() == 0x00A0; }

// ---- numbers

struct NumberRead {
    double value = 0;
    bool negative = false;
    int decimals = 0;        // digits after the decimal mark as typed
    bool grouped = false;    // thousands separators were typed
    bool exponent = false;   // E notation
};

// The characters that stand for a decimal mark and a separator of thousands in this locale (a space of any kind for a group is "a space").
QChar decimalOf(const QLocale &l)
{
    const QString d = l.decimalPoint();
    return d.isEmpty() ? QLatin1Char('.') : d.front();
}

QChar groupOf(const QLocale &l)
{
    const QString g = l.groupSeparator();
    return g.isEmpty() ? QLatin1Char(',') : g.front();
}

// What was typed with the digits of another script (Arabic-Indic, Persian, Devanagari, and the rest that are decimal digits) as the digits of the keyboard, the percent sign of Arabic
// as "%", and without the marks that only set the direction of text. A text that is not a number is not changed by it: the caller keeps what was typed for that.
QString normalizeNative(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar c : s) {
        const ushort u = c.unicode();
        if (u == 0x200E || u == 0x200F || u == 0x061C) continue;
        if (u > 127 && c.isDigit() && c.digitValue() >= 0) {
            out += QChar(ushort('0' + c.digitValue()));
            continue;
        }
        out += u == 0x066A ? QChar('%') : c;
    }
    return out;
}

bool isApostrophe(QChar c) { return c == QLatin1Char('\'') || c == QChar(0x2019) || c == QChar(0x02BC); }

// A number written with digits, the locale's decimal mark and thousands separator, and an exponent: no sign, no sign of money or percent.
bool readUnsigned(const QString &s, const QLocale &locale, NumberRead *out)
{
    if (s.isEmpty()) return false;
    const QChar dec = decimalOf(locale), grp = groupOf(locale);
    const bool groupIsSpace = isSpaceLike(grp);
    QString intPart, fraction, exponent;
    int i = 0;
    int firstGroupLen = -1, current = 0;
    bool sawGroup = false;
    for (; i < s.size(); ++i) {
        const QChar c = s[i];
        if (isDigit(c)) {
            intPart += c;
            ++current;
        } else if (c == grp || (groupIsSpace && isSpaceLike(c)) || (isApostrophe(grp) && isApostrophe(c))) {
            if (current == 0) return false;
            if (!sawGroup) firstGroupLen = current;
            else if (current != 3) return false;
            sawGroup = true;
            current = 0;
        } else {
            break;
        }
    }
    if (sawGroup) {
        if (current != 3 || firstGroupLen < 1 || firstGroupLen > 3) return false;
    }
    if (i < s.size() && s[i] == dec) {
        ++i;
        for (; i < s.size() && isDigit(s[i]); ++i) fraction += s[i];
    }
    if (intPart.isEmpty() && fraction.isEmpty()) return false;
    bool exp = false;
    if (i < s.size() && (s[i] == QLatin1Char('e') || s[i] == QLatin1Char('E'))) {
        ++i;
        QString sign;
        if (i < s.size() && (s[i] == QLatin1Char('+') || s[i] == QLatin1Char('-'))) sign = s[i++];
        QString digits;
        for (; i < s.size() && isDigit(s[i]); ++i) digits += s[i];
        if (digits.isEmpty() || digits.size() > 4) return false;
        exponent = sign + digits;
        exp = true;
    }
    if (i != s.size()) return false;
    if (intPart.size() + fraction.size() > 400) return false;
    const QString text = (intPart.isEmpty() ? QStringLiteral("0") : intPart) + (fraction.isEmpty() ? QString() : QStringLiteral(".") + fraction) + (exp ? QStringLiteral("e") + exponent : QString());
    bool ok = false;
    const double v = QLocale::c().toDouble(text, &ok);
    if (!ok || !std::isfinite(v)) return false;
    out->value = v;
    out->decimals = int(fraction.size());
    out->grouped = sawGroup;
    out->exponent = exp;
    return true;
}

// The signs: a "-" or "+" in front (or the minus sign of Unicode), or the whole in parentheses.
bool stripSign(QString *s, bool *negative)
{
    *negative = false;
    if (s->size() >= 2 && s->front() == QLatin1Char('(') && s->back() == QLatin1Char(')')) {
        *negative = true;
        *s = s->mid(1, s->size() - 2).trimmed();
        return true;
    }
    if (!s->isEmpty() && (s->front() == QLatin1Char('-') || s->front() == QChar(0x2212))) {
        *negative = true;
        *s = s->mid(1).trimmed();
    } else if (!s->isEmpty() && s->front() == QLatin1Char('+')) {
        *s = s->mid(1).trimmed();
    }
    return true;
}

QString currencySign(const QLocale &l)
{
    const QString c = l.currencySymbol(QLocale::CurrencySymbol);
    return c.isEmpty() ? QStringLiteral("$") : c;
}

bool currencyBefore(const QLocale &l)
{
    const QString sign = currencySign(l);
    const QString probe = l.toCurrencyString(1.0, sign);
    const int at = probe.indexOf(sign), digit = probe.indexOf(QLatin1Char('1'));
    return at < 0 || digit < 0 || at < digit;
}

// "+5", "-1,234.50", "(5)", "5%", "$5", "$-5", "1.5E3": a number with the format that shows it as typed.
bool readNumber(const QString &typed, const QLocale &locale, double *value, QString *code)
{
    QString s = typed.trimmed();
    if (s.isEmpty()) return false;
    bool negative = false;
    stripSign(&s, &negative);
    bool percent = false, money = false, moneyAfter = false;
    // a percent sign after (a space may come before it)
    if (s.endsWith(QLatin1Char('%'))) {
        percent = true;
        s.chop(1);
        s = s.trimmed();
    }
    const QString sign = currencySign(locale);
    if (s.startsWith(sign)) {
        money = true;
        s = s.mid(sign.size()).trimmed();
    } else if (s.endsWith(sign)) {
        money = true;
        moneyAfter = true;
        s.chop(sign.size());
        s = s.trimmed();
    }
    if (money && percent) return false;
    // a sign after the money's, "$-5"
    bool negative2 = false;
    if (money && (s.startsWith(QLatin1Char('-')) || s.startsWith(QChar(0x2212)))) {
        negative2 = true;
        s = s.mid(1).trimmed();
    }
    NumberRead r;
    if (!readUnsigned(s, locale, &r)) return false;
    double v = r.value;
    if (negative != negative2 && (negative || negative2)) v = -v;
    if (percent) v /= 100;
    if (percent && std::fabs(r.value) > 1e300) return false;
    *value = v;
    if (money) {
        const QString digits = r.decimals == 0 ? QStringLiteral("#,##0") : QStringLiteral("#,##0.") + QString(std::max(2, r.decimals), QLatin1Char('0'));
        const bool before = !moneyAfter && currencyBefore(locale);
        *code = before ? QStringLiteral("\"%1\"%2").arg(sign, digits) : QStringLiteral("%2\" %1\"").arg(sign, digits);
    } else if (percent) {
        *code = r.decimals == 0 ? QStringLiteral("0%") : QStringLiteral("0.") + QString(r.decimals, QLatin1Char('0')) + QStringLiteral("%");
    } else if (r.exponent) {
        *code = QStringLiteral("0.00E+00");
    } else if (r.grouped) {
        *code = r.decimals == 0 ? QStringLiteral("#,##0") : QStringLiteral("#,##0.") + QString(std::max(2, r.decimals), QLatin1Char('0'));
    } else {
        code->clear();
    }
    return true;
}

// ---- dates and times

enum class Order { MDY, DMY, YMD };

Order orderOf(const QLocale &l)
{
    const QString f = l.dateFormat(QLocale::ShortFormat);
    int d = f.indexOf(QLatin1Char('d')), m = f.indexOf(QLatin1Char('M')), y = f.indexOf(QLatin1Char('y'));
    if (d < 0) d = 1000;
    if (m < 0) m = 1000;
    if (y < 0) y = 1000;
    if (y < d && y < m) return Order::YMD;
    return d < m ? Order::DMY : Order::MDY;
}

// A month's number from its name (the locale's, long or short, or English), 0 if it is none.
int monthFromName(const QString &word, const QLocale &locale)
{
    const QString w = word.toLower();
    if (w.size() < 3) return 0;
    static const QLocale english(QLocale::English, QLocale::UnitedStates);
    for (const QLocale *l : {&locale, &english})
        for (int m = 1; m <= 12; ++m)
            for (const QLocale::FormatType t : {QLocale::LongFormat, QLocale::ShortFormat}) {
                QString name = l->monthName(m, t).toLower();
                if (name == w) return m;
                if (name.endsWith(QLatin1Char('.')) && name.chopped(1) == w) return m;
                if (name.size() >= 3 && w == name.left(3) && t == QLocale::LongFormat && w.size() == 3) return m;
            }
    return 0;
}

struct Token {
    QString text;
    bool numeric = false;
};

// The words and numbers of a date as typed. Empty if it is no date's shape: two marks of "/", "-", or "." in a row, a "," between numbers (it comes after a month's name or
// not at all), or a start or end with a mark but a "." at the end (as "4.3." is written in German).
QVector<Token> tokens(const QString &s, bool dotSeparates)
{
    QVector<Token> out;
    QString current;
    bool numeric = false;
    auto flush = [&] {
        if (!current.isEmpty()) out.append({current, numeric});
        current.clear();
    };
    const bool hasLetters = std::any_of(s.begin(), s.end(), [](QChar c) { return c.isLetter(); });
    bool lastWasMark = true;   // (a date does not begin with a mark)
    for (const QChar c : s) {
        const bool mark = c == QLatin1Char('/') || c == QLatin1Char('-') || (dotSeparates && c == QLatin1Char('.'));
        if (c == QLatin1Char(',') && !hasLetters) return {};
        const bool sep = mark || c == QLatin1Char(',') || c.isSpace();
        if (sep) {
            if (mark && lastWasMark && !(c.isSpace())) return {};
            flush();
            if (mark) lastWasMark = true;
            else if (!c.isSpace()) lastWasMark = false;
            continue;
        }
        lastWasMark = false;
        const bool digit = isDigit(c);
        if (!current.isEmpty() && digit != numeric) flush();
        numeric = digit;
        current += c;
    }
    flush();
    return out;
}

int yearOf(const QString &digits)
{
    const int n = digits.toInt();
    if (digits.size() <= 2) return n < 30 ? 2000 + n : 1900 + n;
    return n;
}

bool readDate(const QString &typed, const EntryOptions &o, double *serial)
{
    const QString s = typed.trimmed();
    if (s.isEmpty() || !isDigit(s.front()) ) {
        // (a date may begin with a month's name)
        if (s.isEmpty() || !s.front().isLetter()) return false;
    }
    const bool dotSeparates = o.locale.dateFormat(QLocale::ShortFormat).contains(QLatin1Char('.'));
    const QVector<Token> t = tokens(s, dotSeparates);
    if (t.size() < 2 || t.size() > 3) return false;
    // anything but separators, digits, and a month's name is no date
    int year = 0, month = 0, day = 0;
    const int named = std::count_if(t.begin(), t.end(), [](const Token &x) { return !x.numeric; });
    if (named > 1) return false;
    for (const Token &x : t)
        if (x.numeric && x.text.size() > 4) return false;
    const QDate today = o.today.isValid() ? o.today : QDate::currentDate();
    if (named == 1) {
        int nameAt = -1;
        for (int i = 0; i < t.size(); ++i)
            if (!t[i].numeric) nameAt = i;
        month = monthFromName(t[nameAt].text, o.locale);
        if (month == 0) return false;
        QVector<QString> nums;
        for (int i = 0; i < t.size(); ++i)
            if (i != nameAt) nums.append(t[i].text);
        if (nums.size() == 1) {
            // "March 4" or "4 March": a day; "March 2024": the month of a year
            if (nums[0].size() >= 3) {
                year = nums[0].toInt();
                day = 1;
            } else {
                day = nums[0].toInt();
                year = today.year();
            }
        } else {
            const bool firstIsYear = nums[0].size() >= 3 || nums[0].toInt() > 31;
            const bool secondIsYear = nums[1].size() >= 3 || nums[1].toInt() > 31;
            if (firstIsYear && !secondIsYear) {
                year = nums[0].toInt();
                day = nums[1].toInt();
            } else {
                day = nums[0].toInt();
                year = yearOf(nums[1]);
                if (secondIsYear && !firstIsYear) year = nums[1].toInt();
            }
        }
    } else {
        QVector<QString> n;
        for (const Token &x : t) n.append(x.text);
        Order order = orderOf(o.locale);
        if (n[0].size() >= 3) order = Order::YMD;   // a year of four digits first is always the year first
        if (n.size() == 3) {
            if (order == Order::YMD) {
                year = yearOf(n[0]);
                month = n[1].toInt();
                day = n[2].toInt();
                if (n[0].size() < 3 && orderOf(o.locale) != Order::YMD) return false;
            } else if (order == Order::MDY) {
                month = n[0].toInt();
                day = n[1].toInt();
                year = yearOf(n[2]);
            } else {
                day = n[0].toInt();
                month = n[1].toInt();
                year = yearOf(n[2]);
            }
        } else {
            // two numbers: a day and a month of this year, or a year and a month
            if (n[0].size() >= 3) {
                year = n[0].toInt();
                month = n[1].toInt();
                day = 1;
            } else if (n[1].size() >= 3) {
                year = n[1].toInt();
                month = n[0].toInt();
                day = 1;
            } else {
                year = today.year();
                if (order == Order::DMY) {
                    day = n[0].toInt();
                    month = n[1].toInt();
                } else {
                    month = n[0].toInt();
                    day = n[1].toInt();
                }
            }
        }
    }
    const int minYear = o.dateSystem == jp::calc::DateSystem::Date1904 ? 1904 : 1900;
    if (year < minYear || year > 9999 || month < 1 || month > 12 || day < 1) return false;
    if (!QDate::isValid(year, month, day) && !(year == 1900 && month == 2 && day == 29 && o.dateSystem == jp::calc::DateSystem::Date1900)) return false;
    return jp::calc::dateToSerial(year, month, day, o.dateSystem, serial);
}

struct TimeRead {
    double fraction = 0;     // of a day (more than 1 for 25:30)
    bool ampm = false;
    bool seconds = false;
    bool fractionalSeconds = false;
    bool over24 = false;
};

bool readTime(const QString &typed, const QLocale &locale, TimeRead *out)
{
    QString s = typed.trimmed();
    if (s.isEmpty()) return false;
    // a marker for the afternoon or morning at the end
    int pm = -1;
    QString am = locale.amText().toUpper(), pmText = locale.pmText().toUpper();
    const QString upper = s.toUpper();
    auto endsWithMarker = [&](const QString &m) { return upper.endsWith(m) && s.size() > m.size(); };
    if (!am.isEmpty() && endsWithMarker(am) ) { pm = 0; s.chop(am.size()); }
    else if (!pmText.isEmpty() && endsWithMarker(pmText)) { pm = 1; s.chop(pmText.size()); }
    else if (endsWithMarker(QStringLiteral("AM"))) { pm = 0; s.chop(2); }
    else if (endsWithMarker(QStringLiteral("PM"))) { pm = 1; s.chop(2); }
    else if (endsWithMarker(QStringLiteral("A")) && s.size() > 1 && (isDigit(s[s.size() - 2]) || s[s.size() - 2].isSpace())) { pm = 0; s.chop(1); }
    else if (endsWithMarker(QStringLiteral("P")) && s.size() > 1 && (isDigit(s[s.size() - 2]) || s[s.size() - 2].isSpace())) { pm = 1; s.chop(1); }
    s = s.trimmed();
    const QStringList parts = s.split(QLatin1Char(':'));
    if (parts.size() > 3) return false;
    if (parts.size() == 1 && pm < 0) return false;   // "10" alone is a number; "10 PM" is a time
    // hours and minutes are digits; the seconds may have a decimal mark and more digits
    for (int i = 0; i < parts.size(); ++i) {
        const QString &p = parts[i];
        if (p.isEmpty()) return false;
        const bool seconds = i == 2;
        for (const QChar c : p)
            if (!isDigit(c) && !(seconds && (c == decimalOf(locale) || c == QLatin1Char('.') || c == QLatin1Char(',')))) return false;
    }
    bool ok = false;
    const int h = parts[0].toInt(&ok);
    if (!ok || parts[0].size() > 4) return false;
    int m = 0;
    double sec = 0;
    if (parts.size() >= 2) {
        if (parts[1].size() > 2) return false;
        m = parts[1].toInt(&ok);
        if (!ok || m > 59) return false;
    }
    if (parts.size() == 3) {
        QString secText = parts[2];
        const QChar dec = decimalOf(locale);
        if (secText.contains(dec)) out->fractionalSeconds = true;
        secText.replace(dec, QLatin1Char('.'));
        if (secText.contains(QLatin1Char(','))) secText.replace(QLatin1Char(','), QLatin1Char('.'));
        if (secText.contains(QLatin1Char('.'))) out->fractionalSeconds = true;
        sec = QLocale::c().toDouble(secText, &ok);
        if (!ok || sec < 0 || sec >= 60) return false;
        out->seconds = true;
    }
    int hours = h;
    if (pm >= 0) {
        if (h < 1 || h > 12) return false;
        hours = h % 12 + (pm == 1 ? 12 : 0);
        out->ampm = true;
    }
    out->over24 = hours >= 24;
    out->fraction = (hours * 3600.0 + m * 60.0 + sec) / 86400.0;
    return true;
}

QString timeCode(const TimeRead &t)
{
    if (t.ampm) return t.seconds ? QStringLiteral("h:mm:ss AM/PM") : QStringLiteral("h:mm AM/PM");
    if (t.over24) return t.seconds ? QStringLiteral("[h]:mm:ss") : QStringLiteral("[h]:mm");
    if (t.fractionalSeconds) return QStringLiteral("h:mm:ss.0");
    return t.seconds ? QStringLiteral("h:mm:ss") : QStringLiteral("h:mm");
}

bool readDateTime(const QString &typed, const EntryOptions &o, double *value, QString *code)
{
    const QString s = typed.trimmed();
    if (s.isEmpty()) return false;
    double serial = 0;
    TimeRead time;
    // a time alone
    if (s.contains(QLatin1Char(':')) || s.contains(QLatin1Char(' '))) {
        if (!s.contains(QLatin1Char(' ')) || !readDate(s, o, &serial)) {
            if (readTime(s, o.locale, &time)) {
                *value = time.fraction;
                *code = timeCode(time);
                return true;
            }
        }
    }
    if (readDate(s, o, &serial)) {
        *value = serial;
        *code = QStringLiteral("m/d/yyyy");
        return true;
    }
    // a date and a time: split before the first word that holds a ":" (or the hour of "10 PM")
    const QStringList words = s.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    for (int i = 1; i < words.size(); ++i) {
        const QString datePart = words.mid(0, i).join(QLatin1Char(' ')), timePart = words.mid(i).join(QLatin1Char(' '));
        TimeRead t;
        if (readDate(datePart, o, &serial) && readTime(timePart, o.locale, &t)) {
            *value = serial + t.fraction;
            QString base = QStringLiteral("m/d/yyyy ");
            *code = base + timeCode(t);
            return true;
        }
    }
    return false;
}

} // namespace

namespace {

// The functions that the file format has with the prefix _xlfn. in front: those added after its first release (a fixed list of the format's, whether or not this program has them: a
// formula that names one this program lacks is kept as the file has it).
bool needsFilePrefix(const QString &upperName)
{
    static const QSet<QString> names = [] {
        static const char *const list[] = {
            "ACOT", "ACOTH", "AGGREGATE", "ARABIC", "ARRAYTOTEXT", "BASE", "BETA.DIST", "BETA.INV", "BINOM.DIST", "BINOM.DIST.RANGE", "BINOM.INV", "BITAND", "BITLSHIFT", "BITOR", "BITRSHIFT",
            "BITXOR", "BYCOL", "BYROW", "CEILING.MATH", "CEILING.PRECISE", "CHISQ.DIST", "CHISQ.DIST.RT", "CHISQ.INV", "CHISQ.INV.RT", "CHISQ.TEST", "CHOOSECOLS", "CHOOSEROWS", "COMBINA",
            "CONCAT", "CONFIDENCE.NORM", "CONFIDENCE.T", "COT", "COTH", "COVARIANCE.P", "COVARIANCE.S", "CSC", "CSCH", "DAYS", "DECIMAL", "DROP", "ENCODEURL", "ERF.PRECISE", "ERFC.PRECISE",
            "EXPAND", "EXPON.DIST", "F.DIST", "F.DIST.RT", "F.INV", "F.INV.RT", "F.TEST", "FILTER", "FILTERXML", "FLOOR.MATH", "FLOOR.PRECISE", "FORECAST.ETS", "FORECAST.ETS.CONFINT",
            "FORECAST.ETS.SEASONALITY", "FORECAST.ETS.STAT", "FORECAST.LINEAR", "FORMULATEXT", "GAMMA", "GAMMA.DIST", "GAMMA.INV", "GAMMALN.PRECISE", "GAUSS", "GROUPBY", "HSTACK",
            "HYPGEOM.DIST", "IFNA", "IFS", "IMAGE", "IMCOSH", "IMCOT", "IMCSC", "IMCSCH", "IMSEC", "IMSECH", "IMSINH", "IMTAN", "ISFORMULA", "ISOMITTED", "ISOWEEKNUM", "LAMBDA", "LET",
            "LOGNORM.DIST", "LOGNORM.INV", "MAKEARRAY", "MAP", "MAXIFS", "MINIFS", "MODE.MULT", "MODE.SNGL", "MUNIT", "NEGBINOM.DIST", "NETWORKDAYS.INTL", "NORM.DIST", "NORM.INV",
            "NORM.S.DIST", "NORM.S.INV", "NUMBERVALUE", "PDURATION", "PERCENTILE.EXC", "PERCENTILE.INC", "PERCENTOF", "PERCENTRANK.EXC", "PERCENTRANK.INC", "PERMUTATIONA", "PHI", "PIVOTBY",
            "POISSON.DIST", "QUARTILE.EXC", "QUARTILE.INC", "QUERYSTRING", "RANDARRAY", "RANK.AVG", "RANK.EQ", "REDUCE", "REGEXEXTRACT", "REGEXREPLACE", "REGEXTEST", "RRI", "SCAN", "SEC",
            "SECH", "SEQUENCE", "SHEET", "SHEETS", "SINGLE", "SKEW.P", "SORT", "SORTBY", "STDEV.P", "STDEV.S", "SWITCH", "T.DIST", "T.DIST.2T", "T.DIST.RT", "T.INV", "T.INV.2T", "T.TEST",
            "TAKE", "TEXTAFTER", "TEXTBEFORE", "TEXTJOIN", "TEXTSPLIT", "TOCOL", "TOROW", "TRIMRANGE", "UNICHAR", "UNICODE", "UNIQUE", "VALUETOTEXT", "VAR.P", "VAR.S", "VSTACK", "WEBSERVICE",
            "WEIBULL.DIST", "WORKDAY.INTL", "WRAPCOLS", "WRAPROWS", "XLOOKUP", "XMATCH", "XOR", "Z.TEST"};
        QSet<QString> set;
        for (const char *name : list) set.insert(QLatin1String(name));
        return set;
    }();
    return names.contains(upperName);
}

bool identStartChar(QChar c) { return c.isLetter() || c == QLatin1Char('_'); }
bool identPartChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'); }

// The end of a text, a sheet's name, or a table's columns that begins at `i` (the index after its closing character).
int quotedEnd(const QString &s, int i)
{
    const int n = int(s.size());
    const QChar c = s[i];
    const QChar close = c == QLatin1Char('[') ? QLatin1Char(']') : c;
    int j = i + 1;
    while (j < n) {
        if (s[j] == close) {
            if (close != QLatin1Char(']') && j + 1 < n && s[j + 1] == close) {
                j += 2;
                continue;
            }
            break;
        }
        ++j;
    }
    return std::min(n, j + 1);
}

// The index of the ")" that closes the "(" at `open`, or -1; the arguments between are split at the commas that are not inside a text, a name, brackets, braces, or another call.
int callEnd(const QString &s, int open, QStringList *arguments)
{
    const int n = int(s.size());
    int depth = 0, start = open + 1;
    for (int i = open; i < n;) {
        const QChar c = s[i];
        if (c == QLatin1Char('"') || c == QLatin1Char('\'') || c == QLatin1Char('[')) {
            i = quotedEnd(s, i);
            continue;
        }
        if (c == QLatin1Char('(') || c == QLatin1Char('{')) ++depth;
        else if (c == QLatin1Char(')') || c == QLatin1Char('}')) {
            --depth;
            if (depth == 0) {
                if (arguments) arguments->append(s.mid(start, i - start));
                return c == QLatin1Char(')') ? i : -1;
            }
        } else if (c == QLatin1Char(',') && depth == 1 && arguments) {
            arguments->append(s.mid(start, i - start));
            start = i + 1;
        }
        ++i;
    }
    return -1;
}

// The names a LET or a LAMBDA gives (parameters) are written in a file with _xlpm. in front, where they are given and where they are used; `scope` holds those in force (lower case).
QString markParameters(const QString &s, const QStringList &scope)
{
    QString out;
    out.reserve(s.size() + 8);
    const int n = int(s.size());
    for (int i = 0; i < n;) {
        const QChar c = s[i];
        if (c == QLatin1Char('"') || c == QLatin1Char('\'') || c == QLatin1Char('[')) {
            const int end = quotedEnd(s, i);
            out += s.mid(i, end - i);
            i = end;
            continue;
        }
        if (identStartChar(c) && (i == 0 || !identPartChar(s[i - 1]))) {
            int j = i;
            while (j < n && identPartChar(s[j])) ++j;
            int k = j;
            while (k < n && s[k].isSpace()) ++k;
            const QString word = s.mid(i, j - i);
            const bool call = k < n && s[k] == QLatin1Char('(');
            const bool let = call && word.compare(QLatin1String("_xlfn.LET"), Qt::CaseInsensitive) == 0;
            const bool lambda = call && word.compare(QLatin1String("_xlfn.LAMBDA"), Qt::CaseInsensitive) == 0;
            QStringList arguments;
            const int end = (let || lambda) ? callEnd(s, k, &arguments) : -1;
            if (end >= 0 && !arguments.isEmpty()) {
                // the names: every other argument of a LET but the last; all but the last of a LAMBDA
                const int count = int(arguments.size());
                QStringList inner = scope;
                QVector<bool> isName(count, false);
                for (int a = 0; a + 1 < count; a += let ? 2 : 1) {
                    isName[a] = true;
                    QString name = arguments[a].trimmed();
                    if (name.startsWith(QLatin1String("_xlpm."), Qt::CaseInsensitive)) name = name.mid(6);
                    inner.append(name.toLower());
                }
                out += s.mid(i, k - i + 1);
                for (int a = 0; a < count; ++a) {
                    if (a) out += QLatin1Char(',');
                    if (isName[a]) {
                        const QString &arg = arguments[a];
                        int lead = 0, trail = int(arg.size());
                        while (lead < trail && arg[lead].isSpace()) ++lead;
                        while (trail > lead && arg[trail - 1].isSpace()) --trail;
                        QString name = arg.mid(lead, trail - lead);
                        if (!name.startsWith(QLatin1String("_xlpm."), Qt::CaseInsensitive)) name = QLatin1String("_xlpm.") + name;
                        out += arg.left(lead) + name + arg.mid(trail);
                    } else {
                        out += markParameters(arguments[a], inner);
                    }
                }
                out += QLatin1Char(')');
                i = end + 1;
                continue;
            }
            const bool after = k < n && s[k] == QLatin1Char('!');
            const bool before = i > 0 && s[i - 1] == QLatin1Char('!');
            if (!call && !after && !before && scope.contains(word.toLower())) out += QLatin1String("_xlpm.");
            out += word;
            i = j;
            continue;
        }
        out += c;
        ++i;
    }
    return out;
}

} // namespace

QString toFileFormula(const QString &typed)
{
    QString out;
    out.reserve(typed.size() + 8);
    const int n = typed.size();
    for (int i = 0; i < n;) {
        const QChar c = typed[i];
        if (c == QLatin1Char('"') || c == QLatin1Char('\'') || c == QLatin1Char('[')) {
            // a text, a sheet's name, or a table's columns: as it is, to its end (a doubled quote is one inside)
            const int end = quotedEnd(typed, i);
            out += typed.mid(i, end - i);
            i = end;
            continue;
        }
        if (identStartChar(c) && (i == 0 || !identPartChar(typed[i - 1]))) {
            int j = i;
            while (j < n && identPartChar(typed[j])) ++j;
            int k = j;
            while (k < n && typed[k].isSpace()) ++k;
            const QString word = typed.mid(i, j - i);
            if (k < n && typed[k] == QLatin1Char('(') && !word.startsWith(QLatin1String("_xl"), Qt::CaseInsensitive)) {
                const jp::calc::FunctionInfo *f = jp::calc::findFunction(word);
                const QString upper = f ? QString::fromLatin1(f->name) : word.toUpper();
                if (needsFilePrefix(upper)) {
                    // (the two that only a sheet has are marked so too)
                    out += upper == QLatin1String("FILTER") || upper == QLatin1String("SORT") ? QLatin1String("_xlfn._xlws.") : QLatin1String("_xlfn.");
                    out += upper;
                    i = j;
                    continue;
                }
                if (f) {
                    if (f->flags & jp::calc::kFnFuture) out += QLatin1String("_xlfn.");
                    out += upper;
                    i = j;
                    continue;
                }
            }
            out += word;
            i = j;
            continue;
        }
        out += c;
        ++i;
    }
    if (out.contains(QLatin1String("_xlfn.LET"), Qt::CaseInsensitive) || out.contains(QLatin1String("_xlfn.LAMBDA"), Qt::CaseInsensitive)) return markParameters(out, QStringList());
    return out;
}

namespace {

bool commaDecimal(const QLocale &locale) { return locale.decimalPoint() == QLatin1String(","); }

// The end (the index after) of what is copied as it is: a text, a name of a sheet, a bracket, or a pair of braces that begins at `i`.
int verbatimEnd(const QString &f, int i)
{
    const int n = int(f.size());
    const QChar c = f[i];
    if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
        int j = i + 1;
        while (j < n) {
            if (f[j] == c) {
                if (j + 1 < n && f[j + 1] == c) {
                    j += 2;
                    continue;
                }
                break;
            }
            ++j;
        }
        return std::min(n, j + 1);
    }
    if (c == QLatin1Char('[')) {
        int depth = 0;
        for (int j = i; j < n; ++j) {
            if (f[j] == QLatin1Char('\'') && j + 1 < n) {
                ++j;
                continue;
            }
            if (f[j] == QLatin1Char('[')) ++depth;
            else if (f[j] == QLatin1Char(']') && --depth == 0) return j + 1;
        }
        return n;
    }
    // braces (an array's constants): to the matching one, over the texts in it
    int depth = 0;
    for (int j = i; j < n;) {
        if (f[j] == QLatin1Char('"')) {
            j = verbatimEnd(f, j);
            continue;
        }
        if (f[j] == QLatin1Char('{')) ++depth;
        else if (f[j] == QLatin1Char('}') && --depth == 0) return j + 1;
        ++j;
    }
    return n;
}

} // namespace

QString formulaFromLocale(const QString &typed, const QLocale &locale)
{
    if (!commaDecimal(locale) || !(typed.contains(QLatin1Char(';')) || typed.contains(QLatin1Char(',')))) return typed;
    QString out;
    out.reserve(typed.size());
    for (int i = 0; i < typed.size();) {
        const QChar c = typed[i];
        if (c == QLatin1Char('"') || c == QLatin1Char('\'') || c == QLatin1Char('[') || c == QLatin1Char('{')) {
            const int end = verbatimEnd(typed, i);
            out += typed.mid(i, end - i);
            i = end;
            continue;
        }
        out += c == QLatin1Char(';') ? QLatin1Char(',') : c == QLatin1Char(',') ? QLatin1Char('.') : c;
        ++i;
    }
    return out;
}

QString formulaToLocale(const QString &formula, const QLocale &locale)
{
    if (!commaDecimal(locale) || !(formula.contains(QLatin1Char('.')) || formula.contains(QLatin1Char(',')))) return formula;
    auto digit = [](QChar c) { return c.unicode() >= '0' && c.unicode() <= '9'; };
    auto identStart = [](QChar c) { return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('\\'); };
    auto identPart = [](QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\'); };
    QString out;
    out.reserve(formula.size());
    const int n = int(formula.size());
    for (int i = 0; i < n;) {
        const QChar c = formula[i];
        if (c == QLatin1Char('"') || c == QLatin1Char('\'') || c == QLatin1Char('[') || c == QLatin1Char('{')) {
            const int end = verbatimEnd(formula, i);
            out += formula.mid(i, end - i);
            i = end;
            continue;
        }
        if (identStart(c)) {
            // a name, a function, a cell (its digits and dots are its own)
            int j = i;
            while (j < n && identPart(formula[j])) ++j;
            out += formula.mid(i, j - i);
            i = j;
            continue;
        }
        if (digit(c) || (c == QLatin1Char('.') && i + 1 < n && digit(formula[i + 1]))) {
            // a number: digits, a decimal point and digits, an exponent
            int j = i;
            while (j < n && digit(formula[j])) ++j;
            QString number = formula.mid(i, j - i);
            if (j < n && formula[j] == QLatin1Char('.')) {
                number += QLatin1Char(',');
                ++j;
                int k = j;
                while (k < n && digit(formula[k])) ++k;
                number += formula.mid(j, k - j);
                j = k;
            }
            if (j < n && (formula[j] == QLatin1Char('E') || formula[j] == QLatin1Char('e'))) {
                int k = j + 1;
                if (k < n && (formula[k] == QLatin1Char('+') || formula[k] == QLatin1Char('-'))) ++k;
                if (k < n && digit(formula[k])) {
                    while (k < n && digit(formula[k])) ++k;
                    number += formula.mid(j, k - j);
                    j = k;
                }
            }
            out += number;
            i = j;
            continue;
        }
        out += c == QLatin1Char(',') ? QLatin1Char(';') : c;
        ++i;
    }
    return out;
}

EntryResult readEntry(const QString &typedIn, const EntryOptions &options)
{
    EntryResult r;
    const QString &typed = typedIn;
    auto tooLong = [&](int limit, const QString &what) {
        r.ok = false;
        r.problem = what.arg(QLocale().toString(limit), QLocale().toString(typed.size()));
        return r;
    };
    if (typed.isEmpty()) {
        r.entry.kind = Entry::Kind::Empty;
        return r;
    }
    // a formula
    const bool signStart = typed.size() > 1 && (typed.front() == QLatin1Char('+') || typed.front() == QLatin1Char('-'));
    auto formula = [&](const QString &text) {
        if (text.size() > kMaxFormulaTypedChars)
            return tooLong(kMaxFormulaTypedChars, tr("A formula can hold up to %1 characters, and this one has %2. Shorten it, and try again."));
        jp::calc::ParseOptions po;
        const QString fileText = formulaFromLocale(text, options.locale);
        const jp::calc::ParseResult p = jp::calc::parseFormula(fileText, po);
        if (!p.ok()) {
            r.ok = false;
            r.problem = tr("The formula can't be read. %1").arg(p.error);
            r.problemAt = p.errorPos + (typed.startsWith(QLatin1Char('=')) ? 1 : 0);
            return r;
        }
        r.entry.kind = Entry::Kind::Formula;
        r.entry.text = toFileFormula(fileText);
        return r;
    };
    if (typed.front() == QLatin1Char('=') && typed.size() > 1) return formula(typed.mid(1));
    if (typed == QLatin1String("=")) {
        r.ok = false;
        r.problem = tr("A formula needs something after the \"=\".");
        r.problemAt = 1;
        return r;
    }
    if (options.textFormat) {
        if (typed.size() > kMaxCellChars) return tooLong(kMaxCellChars, tr("A cell can hold up to %1 characters, and this text has %2. Shorten it, or split it between cells."));
        r.entry.kind = Entry::Kind::Text;
        r.entry.text = typed.front() == QLatin1Char('\'') && typed.size() > 1 ? typed.mid(1) : typed;
        return r;
    }
    // text that is kept as text: an apostrophe in front
    if (typed.front() == QLatin1Char('\'') && typed.size() > 1) {
        const QString rest = typed.mid(1);
        if (rest.size() > kMaxCellChars) return tooLong(kMaxCellChars, tr("A cell can hold up to %1 characters, and this text has %2. Shorten it, or split it between cells."));
        r.entry.kind = Entry::Kind::Text;
        r.entry.text = rest;
        return r;
    }
    if (typed.size() > kMaxCellChars) return tooLong(kMaxCellChars, tr("A cell can hold up to %1 characters, and this text has %2. Shorten it, or split it between cells."));
    // a long text is not a number, a date, or a time
    if (typed.size() <= 64) {
        double v = 0;
        QString code;
        const QString plain = normalizeNative(typed);
        if (readNumber(plain, options.locale, &v, &code)) {
            r.entry.kind = Entry::Kind::Number;
            r.entry.number = v;
            r.entry.formatCode = code;
            return r;
        }
        if (readDateTime(plain, options, &v, &code)) {
            r.entry.kind = Entry::Kind::Number;
            r.entry.number = v;
            r.entry.formatCode = code;
            return r;
        }
        const QString trimmed = typed.trimmed();
        if (trimmed.compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0 || trimmed.compare(QLatin1String("FALSE"), Qt::CaseInsensitive) == 0) {
            r.entry.kind = Entry::Kind::Bool;
            r.entry.number = trimmed.compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0 ? 1 : 0;
            return r;
        }
        if (trimmed.startsWith(QLatin1Char('#'))) {
            bool known = false;
            const jp::calc::ErrorKind e = jp::calc::errorKindFromText(trimmed, &known);
            if (known) {
                r.entry.kind = Entry::Kind::Error;
                r.entry.error = e;
                return r;
            }
        }
    }
    // a formula that begins with a sign (a sign and more that is no formula is a problem, as it is in the programs people know: "- hello" is not text)
    if (signStart && typed.trimmed().size() > 1) return formula(typed);
    r.entry.kind = Entry::Kind::Text;
    r.entry.text = typed;
    return r;
}

namespace {

QString numberInLocale(double v, const QLocale &locale)
{
    QString t = jp::calc::numberToText(v);
    const QChar dec = decimalOf(locale);
    if (dec != QLatin1Char('.')) t.replace(QLatin1Char('.'), dec);
    return t;
}

QString shortDate(const QDate &d, const QLocale &locale)
{
    QString f = locale.dateFormat(QLocale::ShortFormat);
    if (f.contains(QLatin1String("yy")) && !f.contains(QLatin1String("yyyy"))) f.replace(QLatin1String("yy"), QLatin1String("yyyy"));
    return locale.toString(d, f);
}

QString timeOf(double fraction, const QLocale &locale)
{
    qint64 secs = qint64(std::llround(fraction * 86400.0));
    const int h = int(secs / 3600), m = int(secs / 60 % 60), s = int(secs % 60);
    const bool twelve = locale.timeFormat(QLocale::ShortFormat).contains(QLatin1String("AP"), Qt::CaseInsensitive);
    if (twelve) {
        const int h12 = h % 12 == 0 ? 12 : h % 12;
        return QStringLiteral("%1:%2:%3 %4").arg(h12).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')).arg(h % 24 >= 12 ? (locale.pmText().isEmpty() ? QStringLiteral("PM") : locale.pmText()) : (locale.amText().isEmpty() ? QStringLiteral("AM") : locale.amText()));
    }
    return QStringLiteral("%1:%2:%3").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'));
}

} // namespace

QString editTextOfValue(const jp::calc::Value &value, const QString &formatCode, const EntryOptions &options)
{
    using jp::calc::Value;
    switch (value.kind()) {
    case Value::Kind::Empty:
    case Value::Kind::Array: return {};
    case Value::Kind::Bool: return value.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return jp::calc::errorKindText(value.errorKind());
    case Value::Kind::Number: {
        const jp::calc::NumberFormat format(formatCode);
        const double v = value.number();
        if (format.kind() == jp::calc::FormatKind::Date && v >= 0 && v < 2958466) {
            const double whole = std::floor(v), frac = v - whole;
            QString out;
            bool dateOk = true;
            if (whole >= 1) {
                int y = 0, m = 0, d = 0;
                dateOk = jp::calc::serialToDate(whole, options.dateSystem, &y, &m, &d) && QDate::isValid(y, m, d);
                if (dateOk) out = shortDate(QDate(y, m, d), options.locale);
            }
            if (dateOk) {
                if (frac > 1e-9 || whole < 1) out += (out.isEmpty() ? QString() : QStringLiteral(" ")) + timeOf(frac, options.locale);
                return out;
            }
        }
        if (formatCode.contains(QLatin1Char('%')) && format.kind() == jp::calc::FormatKind::Number) return numberInLocale(jp::calc::roundTo15Digits(v * 100), options.locale) + QStringLiteral("%");
        return numberInLocale(v, options.locale);
    }
    case Value::Kind::Text: {
        const QString &t = value.text();
        if (options.textFormat || t.isEmpty()) return t;
        // a text that would be read as something else is typed with an apostrophe
        if (t.front() == QLatin1Char('\'') || t.front() == QLatin1Char('=')) return QStringLiteral("'") + t;
        if (t.size() <= 64 || t.front() == QLatin1Char('+') || t.front() == QLatin1Char('-')) {
            EntryOptions plain = options;
            plain.textFormat = false;
            const EntryResult r = readEntry(t, plain);
            if (!r.ok || r.entry.kind != Entry::Kind::Text) return QStringLiteral("'") + t;
        }
        return t;
    }
    }
    return {};
}

} // namespace jp::office::sheet
