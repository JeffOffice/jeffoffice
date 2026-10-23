#include "calc/numfmt.h"

#include "calc/numfmtnames.h"

#include <QByteArray>
#include <QStringView>

#include <algorithm>
#include <cmath>

#if defined(__SIZEOF_INT128__)
using u128 = unsigned __int128;
#else
using u128 = unsigned long long;   // (the fractions and the times of day are then exact only for the values real workbooks have)
#endif

namespace jp::calc {

namespace {

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// Decimal digits. A value is shown from its 15 significant digits as decimal text, rounded half away from zero, never from the binary value.

struct Dec {
    QByteArray digits;   // the significant digits: no leading and no trailing zeros; empty for zero
    int point = 0;       // the value is 0.DIGITS x 10^point
    bool zero() const { return digits.isEmpty(); }
};

void stripZeros(Dec *d)
{
    while (!d->digits.isEmpty() && d->digits.back() == '0') d->digits.chop(1);
    if (d->digits.isEmpty()) d->point = 0;
}

// The digits of |v| to 15 significant ones, the rest rounded half up (v must be finite).
void toDec(double v, Dec *d)
{
    d->digits.clear();
    d->point = 0;
    v = std::fabs(v);
    if (v == 0) return;
    // The exact decimal expansion of the binary value, to 21 digits; the rounding to 15 is done here, half up.
    const QByteArray s = QString::number(v, 'e', 20).toLatin1();
    const int e = int(s.indexOf('e'));
    if (e < 3) return;
    QByteArray digits;
    digits.reserve(22);
    digits.append(s[0]);
    digits.append(s.mid(2, e - 2));
    int exp = s.mid(e + 1).toInt();
    if (digits.size() > 15) {
        const bool up = digits[15] >= '5';
        digits.truncate(15);
        if (up) {
            int i = 14;
            while (i >= 0 && digits[i] == '9') digits[i--] = '0';
            if (i >= 0) {
                ++digits[i];
            } else {
                digits = QByteArray("1");
                ++exp;
            }
        }
    }
    d->digits = digits;
    d->point = exp + 1;
    stripZeros(d);
}

// Rounds to `decimals` places after the point (to tens, hundreds, ... for negative ones), half up.
void roundDec(Dec *d, int decimals)
{
    if (d->zero()) return;
    const int keep = d->point + decimals;
    if (keep >= d->digits.size()) return;
    if (keep < 0) {
        *d = Dec();
        return;
    }
    const bool up = d->digits[keep] >= '5';
    d->digits.truncate(keep);
    if (up) {
        int i = keep - 1;
        while (i >= 0 && d->digits[i] == '9') d->digits[i--] = '0';
        if (i >= 0) {
            ++d->digits[i];
        } else {
            d->digits = QByteArray("1");
            ++d->point;
        }
    }
    stripZeros(d);
}

// The digits before the point, as text.
QString intPart(const Dec &d)
{
    if (d.zero() || d.point <= 0) return QString();
    QString s;
    s.reserve(d.point);
    for (int i = 0; i < d.point; ++i) s += i < d.digits.size() ? QChar(d.digits[i]) : QLatin1Char('0');
    return s;
}

// The k-th digit after the point (counting from 0).
char fracDigit(const Dec &d, int k)
{
    const int i = d.point + k;
    return i < 0 || i >= d.digits.size() ? '0' : d.digits[i];
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// General: the 15 digits that a number has, in fixed or scientific notation as C's %.15G has it (scientific from 1E+15 and under 0.0001); in a width, the
// most digits that fit.

QString generalOf(const Dec &d, int sig)
{
    const int exp10 = d.point - 1;
    const QByteArray &g = d.digits;
    QString t;
    if (exp10 < -4 || exp10 >= sig) {
        t += QChar(g[0]);
        if (g.size() > 1) {
            t += QLatin1Char('.');
            t += QString::fromLatin1(g.constData() + 1, g.size() - 1);
        }
        t += QLatin1Char('E');
        t += exp10 < 0 ? QLatin1Char('-') : QLatin1Char('+');
        const int a = std::abs(exp10);
        if (a < 10) t += QLatin1Char('0');
        t += QString::number(a);
    } else if (exp10 >= 0) {
        for (int i = 0; i <= exp10; ++i) t += i < g.size() ? QChar(g[i]) : QLatin1Char('0');
        if (g.size() > exp10 + 1) {
            t += QLatin1Char('.');
            t += QString::fromLatin1(g.constData() + exp10 + 1, g.size() - exp10 - 1);
        }
    } else {
        t += QLatin1String("0.");
        t += QString(-exp10 - 1, QLatin1Char('0'));
        t += QString::fromLatin1(g);
    }
    return t;
}

bool generalText(const Dec &d0, bool negative, int maxChars, QString *out)
{
    if (d0.zero()) {
        *out = QStringLiteral("0");
        return maxChars <= 0 || maxChars >= 1;
    }
    for (int sig = 15; sig >= 1; --sig) {
        Dec d = d0;
        if (d.digits.size() > sig) roundDec(&d, sig - d.point);
        QString t = generalOf(d, sig);
        if (negative) t.prepend(QLatin1Char('-'));
        if (maxChars <= 0 || t.size() <= maxChars) {
            *out = t;
            return true;
        }
        if (maxChars <= 0) break;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// The parsed code.

enum class T : quint8 {
    Lit,        // text (s)
    Digit,      // 0 # ? (ch)
    Point,
    Comma,      // n: 0 not decided, 1 groups of three, 2 divides by 1,000
    Percent,
    Exp,        // ch is the E or e; n is 1 if a plus sign shows
    Slash,
    FixedDen,   // n is the denominator, w the number of digits it is written with
    Text,       // @
    Pad,        // _x (ch)
    Fill,       // *x (ch)
    General,
    Year,       // n letters
    Month,      // n letters
    Minute,     // n letters (a Month the neighbors made minutes)
    Day,
    Hour,
    Second,
    FracSec,    // n digits
    AmPm,       // ch is the first letter; n is 1 for A/P
    Elapsed,    // ch is h, m, or s; n the width
    Era,        // e or E with no sign
};

struct Tok {
    T kind = T::Lit;
    QChar ch;
    qint32 n = 0;
    qint32 w = 0;
    QString s;
};

enum class Op : quint8 { None, Lt, Le, Gt, Ge, Eq, Ne };

struct Cond {
    Op op = Op::None;
    double limit = 0;
};

bool matches(const Cond &c, double v)
{
    switch (c.op) {
    case Op::None: return true;
    case Op::Lt: return v < c.limit;
    case Op::Le: return v <= c.limit;
    case Op::Gt: return v > c.limit;
    case Op::Ge: return v >= c.limit;
    case Op::Eq: return v == c.limit;
    case Op::Ne: return v != c.limit;
    }
    return false;
}

// A condition that makes a section the negative one: its sign is then the section's to show.
bool isNegative(const Cond &c) { return (c.op == Op::Lt || c.op == Op::Le) && c.limit == 0; }

enum class Kind : quint8 { Number, Date, Text, General };

struct Section {
    QVector<Tok> toks;
    Cond cond;
    int color = 0;
    int lang = 0;
    Kind kind = Kind::Number;
    // Numbers.
    int nInt = 0, nFrac = 0;          // places before and after the point (of the mantissa, in scientific notation)
    int percents = 0, scale = 0;      // times 100 each, divisions by 1,000 each
    bool group = false;
    int pointAt = -1, expAt = -1, expDigits = 0;
    // Fractions.
    int slashAt = -1, numFrom = -1, denFrom = -1, denTo = -1, denPlaces = 0;
    qint64 fixedDen = 0;
    // Dates and times.
    bool hasAmPm = false, hasTime = false, hasDay = false, hasElapsed = false;
    int fracSec = 0;
};

struct Parsed {
    QString code;
    QVector<Section> secs;
    bool valid = true;
    bool general = false;     // the whole code is General
    int textSection = -1;
};

void addLit(Section *sec, QStringView text)
{
    if (text.isEmpty()) return;
    if (sec->toks.isEmpty() || sec->toks.back().kind != T::Lit) sec->toks.append(Tok());
    sec->toks.back().s += text;
}

void addTok(Section *sec, T kind, QChar ch = QChar(), int n = 0)
{
    Tok t;
    t.kind = kind;
    t.ch = ch;
    t.n = n;
    sec->toks.append(t);
}

bool parseHex(QStringView s, quint32 *out)
{
    if (s.isEmpty() || s.size() > 8) return false;
    quint32 v = 0;
    for (QChar c : s) {
        const ushort u = c.unicode();
        int d;
        if (u >= '0' && u <= '9') d = u - '0';
        else if (u >= 'a' && u <= 'f') d = u - 'a' + 10;
        else if (u >= 'A' && u <= 'F') d = u - 'A' + 10;
        else return false;
        v = v * 16 + quint32(d);
    }
    *out = v;
    return true;
}

bool tokenize(QStringView s, Section *sec, int *brackets, int depth);

// A bracket: a condition, a color, an elapsed time, a currency and locale; anything else is passed over. False for one that can't be read.
bool bracket(QStringView b, Section *sec, int *brackets, int depth)
{
    if (b.isEmpty()) return true;
    const ushort first = b[0].unicode();
    if (first == '<' || first == '>' || first == '=') {
        Op op = Op::Eq;
        int len = 1;
        if (b.startsWith(QLatin1String("<="))) { op = Op::Le; len = 2; }
        else if (b.startsWith(QLatin1String("<>"))) { op = Op::Ne; len = 2; }
        else if (b.startsWith(QLatin1String(">="))) { op = Op::Ge; len = 2; }
        else if (first == '<') op = Op::Lt;
        else if (first == '>') op = Op::Gt;
        bool ok = false;
        const double limit = b.mid(len).trimmed().toDouble(&ok);
        if (!ok || !std::isfinite(limit)) return false;
        if (sec->cond.op == Op::None) {
            sec->cond.op = op;
            sec->cond.limit = limit;
        }
        return true;
    }
    if (first == '$') {
        const QStringView rest = b.mid(1);
        const int dash = int(rest.lastIndexOf(QLatin1Char('-')));
        QStringView text = rest;
        quint32 lcid = 0;
        bool haveLcid = false;
        if (dash >= 0 && parseHex(rest.mid(dash + 1), &lcid)) {
            text = rest.left(dash);
            haveLcid = true;
        }
        addLit(sec, text);
        if (haveLcid) {
            const quint32 low = lcid & 0xFFFF;
            if (low == 0xF800) {
                if (depth > 0) return false;
                return tokenize(QStringLiteral("dddd, mmmm dd, yyyy"), sec, brackets, depth + 1);
            }
            if (low == 0xF400) {
                if (depth > 0) return false;
                return tokenize(QStringLiteral("h:mm:ss AM/PM"), sec, brackets, depth + 1);
            }
            sec->lang = formatLanguageOf(low);
        }
        return true;
    }
    const QString lower = b.toString().trimmed().toLower();
    static const char *const names[] = {"black", "white", "red", "green", "blue", "yellow", "magenta", "cyan"};
    for (int i = 0; i < 8; ++i)
        if (lower == QLatin1String(names[i])) {
            sec->color = i + 1;
            return true;
        }
    if (lower.startsWith(QLatin1String("color"))) {
        bool ok = false;
        const int n = QStringView(lower).mid(5).trimmed().toInt(&ok);
        if (ok && n >= 1 && n <= 56) sec->color = n;
        return true;
    }
    if (!lower.isEmpty() && lower.size() <= 8 && (lower[0] == QLatin1Char('h') || lower[0] == QLatin1Char('m') || lower[0] == QLatin1Char('s'))) {
        bool same = true;
        for (QChar c : lower)
            if (c != lower[0]) same = false;
        if (same) {
            addTok(sec, T::Elapsed, lower[0], int(lower.size()));
            return true;
        }
    }
    return true;   // [DBNum1], [Natural], and the like change nothing here
}

bool tokenize(QStringView s, Section *sec, int *brackets, int depth)
{
    const int n = int(s.size());
    int i = 0;
    while (i < n) {
        const QChar c = s[i];
        switch (c.unicode()) {
        case '"': {
            const int j = int(s.indexOf(QLatin1Char('"'), i + 1));
            if (j < 0) return false;
            addLit(sec, s.mid(i + 1, j - i - 1));
            i = j + 1;
            break;
        }
        case '\\':
            if (i + 1 >= n) return false;
            addLit(sec, s.mid(i + 1, 1));
            i += 2;
            break;
        case '_':
            if (i + 1 >= n) return false;
            addTok(sec, T::Pad, s[i + 1]);
            i += 2;
            break;
        case '*':
            if (i + 1 >= n) return false;
            addTok(sec, T::Fill, s[i + 1]);
            i += 2;
            break;
        case '[': {
            if (++*brackets > kMaxFormatBrackets) return false;
            const int j = int(s.indexOf(QLatin1Char(']'), i + 1));
            if (j < 0 || j - i > 300) return false;
            if (!bracket(s.mid(i + 1, j - i - 1), sec, brackets, depth)) return false;
            i = j + 1;
            break;
        }
        case '0':
        case '#':
        case '?':
            addTok(sec, T::Digit, c);
            ++i;
            break;
        case '.':
            addTok(sec, T::Point);
            ++i;
            break;
        case ',':
            addTok(sec, T::Comma);
            ++i;
            break;
        case '%':
            addTok(sec, T::Percent);
            ++i;
            break;
        case '@':
            addTok(sec, T::Text);
            ++i;
            break;
        case '/': {
            const bool afterDigit = !sec->toks.isEmpty() && sec->toks.back().kind == T::Digit;
            addTok(sec, T::Slash);
            ++i;
            // A denominator written as digits (not a 0 that is a place) is a fixed one.
            if (afterDigit && i < n && s[i].unicode() >= '1' && s[i].unicode() <= '9') {
                int j = i;
                qint64 v = 0;
                while (j < n && s[j].unicode() >= '0' && s[j].unicode() <= '9') {
                    if (j - i >= 9) return false;
                    v = v * 10 + (s[j].unicode() - '0');
                    ++j;
                }
                Tok t;
                t.kind = T::FixedDen;
                t.n = qint32(v);
                t.w = j - i;
                sec->toks.append(t);
                i = j;
            }
            break;
        }
        case 'E':
        case 'e':
            if (i + 1 < n && (s[i + 1] == QLatin1Char('+') || s[i + 1] == QLatin1Char('-'))) {
                addTok(sec, T::Exp, c, s[i + 1] == QLatin1Char('+') ? 1 : 0);
                i += 2;
            } else {
                int j = i;
                while (j < n && (s[j] == QLatin1Char('e') || s[j] == QLatin1Char('E'))) ++j;
                addTok(sec, T::Era, c, j - i);
                i = j;
            }
            break;
        case 'd': case 'D':
        case 'm': case 'M':
        case 'y': case 'Y':
        case 'h': case 'H':
        case 's': case 'S': {
            const ushort lower = ushort(c.toLower().unicode());
            int j = i;
            while (j < n && s[j].toLower().unicode() == lower) ++j;
            const T kind = lower == 'd' ? T::Day : lower == 'm' ? T::Month : lower == 'y' ? T::Year : lower == 'h' ? T::Hour : T::Second;
            addTok(sec, kind, c, std::min(j - i, 100));
            i = j;
            break;
        }
        case 'a':
        case 'A':
            if (s.mid(i, 5).compare(QLatin1String("am/pm"), Qt::CaseInsensitive) == 0) {
                addTok(sec, T::AmPm, c, 0);
                i += 5;
            } else if (s.mid(i, 3).compare(QLatin1String("a/p"), Qt::CaseInsensitive) == 0) {
                addTok(sec, T::AmPm, c, 1);
                i += 3;
            } else {
                addLit(sec, s.mid(i, 1));
                ++i;
            }
            break;
        case 'g':
        case 'G':
            if (s.mid(i, 7).compare(QLatin1String("general"), Qt::CaseInsensitive) == 0) {
                addTok(sec, T::General);
                i += 7;
            } else {
                addLit(sec, s.mid(i, 1));
                ++i;
            }
            break;
        default:
            addLit(sec, s.mid(i, 1));
            ++i;
            break;
        }
    }
    return true;
}

void toLiteral(Tok *t, const QString &text)
{
    t->kind = T::Lit;
    t->s = text;
    t->n = 0;
}

QString literalOf(const Tok &t)
{
    switch (t.kind) {
    case T::Digit: return QString(t.ch);
    case T::Point: return QStringLiteral(".");
    case T::Comma: return QStringLiteral(",");
    case T::Percent: return QStringLiteral("%");
    case T::Slash: return QStringLiteral("/");
    case T::Text: return QStringLiteral("@");
    case T::Exp: return QString(t.ch) + (t.n ? QLatin1Char('+') : QLatin1Char('-'));
    case T::FixedDen: return QString::number(t.n);
    case T::Era: return QString(t.ch).repeated(std::max(t.n, 1));
    default: return t.s;
    }
}

bool isDateTok(T k)
{
    return k == T::Year || k == T::Month || k == T::Day || k == T::Hour || k == T::Second || k == T::AmPm || k == T::Elapsed;
}

bool isPlain(T k) { return k == T::Lit || k == T::Pad || k == T::Fill || k == T::Comma; }

// What the section is, and what each token comes to there. False if the section can't be.
bool analyze(Section *s)
{
    QVector<Tok> &t = s->toks;
    bool date = false, general = false, text = false, digit = false;
    for (const Tok &k : t) {
        if (isDateTok(k.kind)) date = true;
        else if (k.kind == T::General) general = true;
        else if (k.kind == T::Text) text = true;
        else if (k.kind == T::Digit) digit = true;
    }
    if (date) s->kind = Kind::Date;
    else if (general) s->kind = Kind::General;
    else if (text && !digit) s->kind = Kind::Text;
    else s->kind = Kind::Number;

    if (s->kind == Kind::Date) {
        // Fractions of a second: a point and zeros right after the seconds. Everything else that is a place or a sign is text.
        QVector<Tok> out;
        out.reserve(t.size());
        for (int i = 0; i < t.size(); ++i) {
            Tok k = t[i];
            const bool afterSeconds = !out.isEmpty() && (out.back().kind == T::Second || (out.back().kind == T::Elapsed && out.back().ch == QLatin1Char('s')));
            if (k.kind == T::Point && afterSeconds) {
                int j = i + 1;
                while (j < t.size() && t[j].kind == T::Digit && t[j].ch == QLatin1Char('0')) ++j;
                if (j > i + 1) {
                    k.kind = T::FracSec;
                    k.n = std::min(j - i - 1, 100);
                    out.append(k);
                    i = j - 1;
                    continue;
                }
            }
            switch (k.kind) {
            case T::Digit: case T::Point: case T::Comma: case T::Percent: case T::Slash: case T::Text: case T::Exp: case T::FixedDen: case T::General: {
                const QString lit = literalOf(k);
                // A comma stays a comma for the parts of the code that look for one between times.
                toLiteral(&k, lit);
                break;
            }
            case T::Era:
                k.kind = T::Year;
                k.n = 4;
                break;
            default: break;
            }
            // Join text that follows text.
            if (k.kind == T::Lit && !out.isEmpty() && out.back().kind == T::Lit) out.back().s += k.s;
            else out.append(k);
        }
        t = out;
        // m and mm are minutes after an hour and before seconds.
        for (int i = 0; i < t.size(); ++i) {
            if (t[i].kind != T::Month || t[i].n > 2) continue;
            int p = i - 1;
            while (p >= 0 && isPlain(t[p].kind)) --p;
            int q = i + 1;
            while (q < t.size() && isPlain(t[q].kind)) ++q;
            const bool afterHour = p >= 0 && (t[p].kind == T::Hour || (t[p].kind == T::Elapsed && t[p].ch == QLatin1Char('h')));
            const bool beforeSecond = q < t.size() && (t[q].kind == T::Second || (t[q].kind == T::Elapsed && t[q].ch == QLatin1Char('s')));
            if (afterHour || beforeSecond) t[i].kind = T::Minute;
        }
        for (const Tok &k : t) {
            if (k.kind == T::AmPm) s->hasAmPm = true;
            if (k.kind == T::Hour || k.kind == T::Minute || k.kind == T::Second || k.kind == T::AmPm || k.kind == T::Elapsed || k.kind == T::FracSec) s->hasTime = true;
            if (k.kind == T::Year || k.kind == T::Month || k.kind == T::Day) s->hasDay = true;
            if (k.kind == T::Elapsed) s->hasElapsed = true;
            if (k.kind == T::FracSec) s->fracSec = std::max(s->fracSec, std::min(k.n, 3));
        }
        return true;
    }

    if (s->kind == Kind::General || s->kind == Kind::Text) {
        for (Tok &k : t) {
            switch (k.kind) {
            case T::Digit: case T::Point: case T::Comma: case T::Percent: case T::Slash: case T::Exp: case T::FixedDen: case T::Era: toLiteral(&k, literalOf(k)); break;
            case T::Text:
                if (s->kind == Kind::General) toLiteral(&k, QStringLiteral("@"));
                break;
            case T::Elapsed: toLiteral(&k, QString(k.ch)); break;
            default: break;
            }
        }
        return true;
    }

    // A number.
    for (Tok &k : t) {
        if (k.kind == T::Text) toLiteral(&k, QStringLiteral("@"));
        else if (k.kind == T::Elapsed) toLiteral(&k, QString(k.ch));
        else if (k.kind == T::Era) toLiteral(&k, literalOf(k));
    }
    const int n = int(t.size());
    QVector<int> digitsBefore(n + 1, 0);
    for (int i = 0; i < n; ++i) digitsBefore[i + 1] = digitsBefore[i] + (t[i].kind == T::Digit ? 1 : 0);
    auto digitsIn = [&](int from, int to) { return from >= to ? 0 : digitsBefore[to] - digitsBefore[from]; };
    if (digitsBefore[n] > kMaxFormatDigits) return false;

    // The exponent, if there are places before it and after it.
    for (int i = 0; i < n; ++i) {
        if (t[i].kind != T::Exp) continue;
        if (s->expAt < 0 && digitsBefore[i] > 0 && i + 1 < n && t[i + 1].kind == T::Digit) {
            s->expAt = i;
            int j = i + 1;
            while (j < n && t[j].kind == T::Digit) ++j;
            s->expDigits = j - i - 1;
        } else {
            toLiteral(&t[i], literalOf(t[i]));
        }
    }
    const int mantEnd = s->expAt >= 0 ? s->expAt : n;
    if (s->expAt >= 0) {
        // Places after the exponent's own are text.
        int j = s->expAt + 1 + s->expDigits;
        for (; j < n; ++j)
            if (t[j].kind == T::Digit) toLiteral(&t[j], QString(t[j].ch));
    }

    // A fraction: places, a slash, and places or a number.
    if (s->expAt < 0) {
        for (int i = 1; i + 1 < n; ++i) {
            if (t[i].kind != T::Slash || t[i - 1].kind != T::Digit || (t[i + 1].kind != T::Digit && t[i + 1].kind != T::FixedDen)) continue;
            s->slashAt = i;
            s->numFrom = i - 1;
            while (s->numFrom > 0 && t[s->numFrom - 1].kind == T::Digit) --s->numFrom;
            s->denFrom = i + 1;
            if (t[i + 1].kind == T::FixedDen) {
                s->denTo = i + 2;
                s->fixedDen = t[i + 1].n;
                s->denPlaces = t[i + 1].w;
                if (s->fixedDen <= 0) return false;
            } else {
                s->denTo = i + 1;
                while (s->denTo < n && t[s->denTo].kind == T::Digit) ++s->denTo;
                s->denPlaces = s->denTo - s->denFrom;
            }
            if (i - s->numFrom > 9 || s->denPlaces > 9) return false;
            break;
        }
    }
    for (int i = 0; i < n; ++i) {
        if (t[i].kind == T::FixedDen && s->slashAt + 1 != i) toLiteral(&t[i], QString::number(t[i].n));
        else if (t[i].kind == T::Slash && i != s->slashAt) toLiteral(&t[i], QStringLiteral("/"));
    }
    const bool fraction = s->slashAt >= 0;

    // The point, and the places on either side of it.
    int intEnd = mantEnd;
    if (fraction) {
        intEnd = s->numFrom;
        for (int i = 0; i < n; ++i)
            if (t[i].kind == T::Point) toLiteral(&t[i], QStringLiteral("."));
        s->nInt = digitsIn(0, s->numFrom);
    } else {
        for (int i = 0; i < mantEnd; ++i) {
            if (t[i].kind != T::Point) continue;
            if (s->pointAt < 0) s->pointAt = i;
            else toLiteral(&t[i], QStringLiteral("."));
        }
        if (s->pointAt >= 0) intEnd = s->pointAt;
        s->nInt = digitsIn(0, intEnd);
        if (s->pointAt >= 0) s->nFrac = digitsIn(s->pointAt + 1, mantEnd);
    }
    // A point after the exponent is text.
    for (int i = mantEnd; i < n; ++i)
        if (t[i].kind == T::Point) toLiteral(&t[i], QStringLiteral("."));

    // Commas between places group by three; commas after the last place divide by 1,000 each.
    for (int i = 0; i < n; ++i) {
        if (t[i].kind != T::Comma) continue;
        const bool before = digitsBefore[i] > 0;
        bool groupIt = false, scaleIt = false;
        if (i < intEnd) {
            const bool after = digitsIn(i + 1, intEnd) > 0;
            groupIt = before && after;
            scaleIt = before && !after;
        } else if (!fraction && s->pointAt >= 0 && i > s->pointAt && i < mantEnd) {
            scaleIt = before && digitsIn(i + 1, mantEnd) == 0;
        }
        if (groupIt) {
            t[i].n = 1;
            s->group = true;
        } else if (scaleIt) {
            t[i].n = 2;
            ++s->scale;
        } else {
            toLiteral(&t[i], QStringLiteral(","));
        }
    }
    for (const Tok &k : t)
        if (k.kind == T::Percent) ++s->percents;
    return true;
}

Parsed parseCode(const QString &code)
{
    Parsed p;
    p.code = code;
    auto invalid = [&p]() {
        p.valid = false;
        p.secs.clear();
        p.general = true;
        p.textSection = -1;
        return p;
    };
    if (code.size() > kMaxFormatCodeChars) return invalid();
    if (code.isEmpty()) {
        p.general = true;
        return p;
    }
    // Split at the semicolons that are not in quotes, in brackets, or after a backslash.
    QVector<QStringView> parts;
    {
        const QStringView all(code);
        int start = 0;
        bool quote = false, brack = false;
        for (int i = 0; i < code.size(); ++i) {
            const ushort u = code[i].unicode();
            if (quote) {
                if (u == '"') quote = false;
            } else if (brack) {
                if (u == ']') brack = false;
            } else if (u == '"') {
                quote = true;
            } else if (u == '[') {
                brack = true;
            } else if (u == '\\' || u == '_' || u == '*') {
                ++i;
            } else if (u == ';') {
                parts.append(all.mid(start, i - start));
                start = i + 1;
                if (parts.size() >= kMaxFormatSections) return invalid();
            }
        }
        parts.append(all.mid(start));
        if (parts.size() > kMaxFormatSections) return invalid();
    }
    for (const QStringView &part : parts) {
        Section s;
        int brackets = 0;
        if (!tokenize(part, &s, &brackets, 0) || !analyze(&s)) return invalid();
        p.secs.append(s);
    }
    if (p.secs.size() == 1 && p.secs[0].kind == Kind::General) {
        const Section &s = p.secs[0];
        bool alone = true;
        for (const Tok &k : s.toks)
            if (k.kind != T::General) alone = false;
        p.general = alone && s.cond.op == Op::None && s.color == 0;
    }
    if (p.secs.size() == 4) {
        p.textSection = 3;
    } else {
        for (int i = int(p.secs.size()) - 1; i >= 0; --i)
            if (p.secs[i].kind == Kind::Text) {
                p.textSection = i;
                break;
            }
    }
    return p;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// The text being made.

struct Out {
    QString text;
    int fillAt = -1;
    QChar fillChar;
    QVector<FormatPad> pads;
    bool over = false;

    void add(QChar c)
    {
        if (text.size() >= kMaxFormatOutputChars) over = true;
        else text += c;
    }
    void add(QStringView s)
    {
        if (text.size() + s.size() > kMaxFormatOutputChars) over = true;
        else text += s;
    }
    void pad(QChar like)
    {
        if (text.size() >= kMaxFormatOutputChars) {
            over = true;
            return;
        }
        FormatPad p;
        p.at = int(text.size());
        p.like = like;
        pads.append(p);
        text += QLatin1Char(' ');
    }
    void fill(QChar c)
    {
        if (fillAt < 0) {
            fillAt = int(text.size());
            fillChar = c;
        }
    }
};

// The tokens of [from, to) that need no value: text, pads, fills, signs.
void plainToken(const Tok &k, Out *o)
{
    switch (k.kind) {
    case T::Lit: o->add(k.s); break;
    case T::Pad: o->pad(k.ch); break;
    case T::Fill: o->fill(k.ch); break;
    case T::Percent: o->add(QLatin1Char('%')); break;
    default: break;
    }
}

// The integer places of tokens [from, to) filled from the right with `ints`, and then (after a point) the places for `frac`.
struct Places {
    const Section *s;
    QString ints;        // the integer digits (no leading zeros)
    const Dec *frac;     // for the digits after the point
    int lastNonZero;     // the last fractional place that holds a digit that is not 0, or -1
};

void renderPlaces(const Places &pl, int from, int to, Out *o)
{
    const Section &s = *pl.s;
    const int nInt = s.nInt;
    const int L = int(pl.ints.size());
    int intIdx = 0, fracIdx = 0;
    bool afterPoint = false, seen = false, spaced = false;
    auto digit = [&](QChar d, int r) {
        // Groups of three: a comma before the digit that has two digits after it.
        if (s.group && r > 0 && r % 3 == 2) {
            if (seen) o->add(QLatin1Char(','));
            else if (spaced) o->add(QLatin1Char(' '));
        }
        o->add(d);
        seen = true;
    };
    bool extrasDone = false;
    auto extras = [&]() {
        if (extrasDone) return;
        extrasDone = true;
        const int extra = L - nInt;
        for (int i = 0; i < extra; ++i) digit(pl.ints[i], L - 1 - i);
    };
    for (int i = from; i < to; ++i) {
        const Tok &k = s.toks[i];
        switch (k.kind) {
        case T::Point:
            if (!afterPoint && nInt == 0 && L > 0) extras();
            o->add(QLatin1Char('.'));
            afterPoint = true;
            break;
        case T::Digit:
            if (!afterPoint) {
                const int p = intIdx++;
                extras();
                const int at = L - (nInt - p);
                const int r = nInt - 1 - p;
                if (at >= 0) {
                    digit(pl.ints[at], r);
                } else if (k.ch == QLatin1Char('0')) {
                    digit(QLatin1Char('0'), r);
                } else if (k.ch == QLatin1Char('?')) {
                    if (s.group && r > 0 && r % 3 == 2 && !seen) o->add(QLatin1Char(' '));
                    o->add(QLatin1Char(' '));
                    spaced = true;
                }
            } else {
                const int j = fracIdx++;
                const QChar d = pl.frac ? QChar(fracDigit(*pl.frac, j)) : QLatin1Char('0');
                if (k.ch == QLatin1Char('0')) o->add(d);
                else if (j <= pl.lastNonZero) o->add(d);
                else if (k.ch == QLatin1Char('?')) o->add(QLatin1Char(' '));
            }
            break;
        default: plainToken(k, o); break;
        }
    }
}

int lastNonZeroFrac(const Dec &d, int nFrac)
{
    int last = -1;
    for (int j = 0; j < nFrac; ++j)
        if (fracDigit(d, j) != '0') last = j;
    return last;
}

// A decimal text plus one.
QString incremented(const QString &s)
{
    QString r = s.isEmpty() ? QStringLiteral("0") : s;
    int i = int(r.size()) - 1;
    while (i >= 0 && r[i] == QLatin1Char('9')) r[i--] = QLatin1Char('0');
    if (i >= 0) r[i] = QChar(r[i].unicode() + 1);
    else r.prepend(QLatin1Char('1'));
    return r;
}

// The best fraction of p/q (p >= 0) with a denominator of at most maxDen: the last convergent, or the semiconvergent after it, whichever is closer.
void bestFraction(u128 p, u128 q, u128 maxDen, u128 *num, u128 *den)
{
    if (p == 0 || q == 0) {
        *num = 0;
        *den = 1;
        return;
    }
    u128 u = p, v = q;
    u128 hPrev = 1, kPrev = 0;
    u128 hCur = u / v, kCur = 1;
    u128 r = u % v;
    u = v;
    v = r;
    while (v != 0) {
        const u128 a = u / v;
        // The next convergent is (a h + hPrev) / (a k + kPrev); does its denominator fit?
        if (kCur > maxDen || a > (maxDen - kPrev) / kCur) {
            const u128 j = kCur > maxDen ? 0 : (maxDen - kPrev) / kCur;
            if (j >= 1) {
                const u128 h2 = j * hCur + hPrev, k2 = j * kCur + kPrev;
                // Which is closer to p/q: hCur/kCur or h2/k2? Compare |p k - h q| / k.
                auto err = [&](u128 h, u128 k) { const u128 a1 = p * k, b1 = h * q; return a1 > b1 ? a1 - b1 : b1 - a1; };
                if (err(h2, k2) * kCur < err(hCur, kCur) * k2) {
                    hCur = h2;
                    kCur = k2;
                }
            }
            break;
        }
        const u128 h = a * hCur + hPrev, k = a * kCur + kPrev;
        hPrev = hCur;
        kPrev = kCur;
        hCur = h;
        kCur = k;
        r = u % v;
        u = v;
        v = r;
    }
    *num = hCur;
    *den = kCur;
}

QString u128Text(u128 v)
{
    if (v == 0) return QStringLiteral("0");
    QString s;
    while (v > 0) {
        s.prepend(QChar('0' + int(v % 10)));
        v /= 10;
    }
    return s;
}

// The places of a numerator (right-aligned; digits that no place is there for go in front) or of a denominator (left-aligned), filled from `digits`
// (empty to leave them blank): a 0 place with no digit is 0, a ? place is a space, a # place nothing.
void renderCount(const Section &s, int from, int to, const QString &digits, bool right, Out *o)
{
    const int np = to - from;
    const int L = int(digits.size());
    auto empty = [&](int p) {
        const QChar ch = s.toks[from + p].ch;
        if (ch == QLatin1Char('0')) o->add(QLatin1Char('0'));
        else if (ch == QLatin1Char('?')) o->add(QLatin1Char(' '));
    };
    if (right) {
        for (int i = 0; i < L - np; ++i) o->add(digits[i]);
        for (int p = 0; p < np; ++p) {
            const int at = L - (np - p);
            if (at >= 0) o->add(digits[at]);
            else empty(p);
        }
    } else {
        o->add(digits);
        for (int p = L; p < np; ++p) empty(p);
    }
}

void renderFraction(const Section &s, Dec d, Out *o)
{
    // A value under 1E-5 has no fraction to show, and its p/q would not fit the arithmetic below.
    if (!d.zero() && d.point <= 15 && int(d.digits.size()) - d.point > 20) d = Dec();
    // The value as p/q: the digits, with the point moved to the end.
    u128 p = 0, q = 1;
    bool big = d.point > 15;
    QString wholeText;
    if (!big) {
        const int len = int(d.digits.size());
        for (int i = 0; i < len; ++i) p = p * 10 + u128(d.digits[i] - '0');
        if (d.point >= len) {
            for (int i = len; i < d.point; ++i) p *= 10;
        } else {
            for (int i = 0; i < len - d.point; ++i) q *= 10;
        }
    }
    const bool improper = s.nInt == 0;
    QString ip;      // the whole part, when there are places for it
    u128 num = 0, den = 1;
    if (big) {
        ip = intPart(d);
        num = 0;
        den = 1;
        if (improper) wholeText = ip;
    } else if (improper) {
        if (d.point > 4) {
            wholeText = intPart(d);
            num = 0;
        } else if (s.fixedDen > 0) {
            den = u128(s.fixedDen);
            num = (2 * p * den + q) / (2 * q);
        } else {
            u128 maxDen = 1;
            for (int i = 0; i < s.denPlaces; ++i) maxDen *= 10;
            bestFraction(p, q, maxDen - 1, &num, &den);
        }
    } else {
        u128 whole = p / q;
        u128 f = p % q;
        ip = whole == 0 ? QString() : u128Text(whole);
        if (f != 0) {
            if (s.fixedDen > 0) {
                den = u128(s.fixedDen);
                num = (2 * f * den + q) / (2 * q);
            } else {
                u128 maxDen = 1;
                for (int i = 0; i < s.denPlaces; ++i) maxDen *= 10;
                bestFraction(f, q, maxDen - 1, &num, &den);
            }
            if (num >= den) {
                ip = incremented(ip);
                num = 0;
                den = s.fixedDen > 0 ? den : 1;
            }
        }
    }
    const bool zeroFraction = num == 0;
    if (!improper && ip.isEmpty() && zeroFraction) ip = QStringLiteral("0");
    // The places before the numerator.
    {
        Places pl;
        pl.s = &s;
        pl.ints = ip;
        pl.frac = nullptr;
        pl.lastNonZero = -1;
        renderPlaces(pl, 0, s.numFrom, o);
    }
    bool anyZeroPlace = false;
    for (int i = s.numFrom; i < s.slashAt; ++i)
        if (s.toks[i].ch == QLatin1Char('0')) anyZeroPlace = true;
    for (int i = s.denFrom; i < s.denTo && s.fixedDen == 0; ++i)
        if (s.toks[i].ch == QLatin1Char('0')) anyZeroPlace = true;
    // A whole number among places for its fraction leaves them blank.
    const bool blank = !improper && zeroFraction;
    const QString numText = blank ? QString() : improper && !wholeText.isEmpty() ? wholeText : u128Text(num);
    renderCount(s, s.numFrom, s.slashAt, numText, true, o);
    o->add(blank && !anyZeroPlace ? QLatin1Char(' ') : QLatin1Char('/'));
    if (s.fixedDen > 0) {
        if (blank) o->add(QString(s.denPlaces, QLatin1Char(' ')));
        else o->add(QString::number(qint64(den)));
    } else {
        renderCount(s, s.denFrom, s.denTo, blank ? QString() : u128Text(den), false, o);
    }
    for (int i = s.denTo; i < s.toks.size(); ++i) plainToken(s.toks[i], o);
}

// A number in a section of digit places: plain, scientific, or a fraction. `negativeShown` puts a minus first.
void renderNumber(const Section &s, Dec d, bool negativeShown, Out *o)
{
    if (negativeShown) o->add(QLatin1Char('-'));
    if (!d.zero()) {
        d.point += 2 * s.percents - 3 * s.scale;
        if (d.point > kMaxFormatOutputChars + 400 || d.point < -100000) {
            if (d.point > 0) {
                o->over = true;
                return;
            }
            d = Dec();
        }
    }
    if (s.slashAt >= 0) {
        renderFraction(s, d, o);
        return;
    }
    if (s.expAt >= 0) {
        Dec m = d;
        int e = 0;
        const int ni = std::max(s.nInt, 1);
        if (!m.zero()) {
            const int e0 = m.point - 1;
            e = s.nInt <= 1 ? e0 : (e0 >= 0 ? e0 / ni * ni : -((-e0 + ni - 1) / ni) * ni);
            if (s.nInt == 0) e = m.point;
            m.point -= e;
            roundDec(&m, s.nFrac);
            if (!m.zero() && m.point > std::max(s.nInt, 1)) {
                e += ni;
                m.point -= ni;
            }
        }
        Places pl;
        pl.s = &s;
        pl.ints = intPart(m);
        pl.frac = &m;
        pl.lastNonZero = lastNonZeroFrac(m, s.nFrac);
        renderPlaces(pl, 0, s.expAt, o);
        const Tok &x = s.toks[s.expAt];
        o->add(x.ch);
        if (e < 0) o->add(QLatin1Char('-'));
        else if (x.n) o->add(QLatin1Char('+'));
        QString digits = QString::number(std::abs(e));
        if (digits.size() < s.expDigits) digits.prepend(QString(s.expDigits - digits.size(), QLatin1Char('0')));
        o->add(digits);
        for (int i = s.expAt + 1 + s.expDigits; i < s.toks.size(); ++i) plainToken(s.toks[i], o);
        return;
    }
    roundDec(&d, s.nFrac);
    Places pl;
    pl.s = &s;
    pl.ints = intPart(d);
    pl.frac = &d;
    pl.lastNonZero = lastNonZeroFrac(d, s.nFrac);
    renderPlaces(pl, 0, int(s.toks.size()), o);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// Dates and times.

u128 pow10u(int k)
{
    u128 v = 1;
    for (int i = 0; i < k; ++i) v *= 10;
    return v;
}

bool renderDate(const Section &s, const Dec &d, DateSystem system, bool negativeShown, Out *o)
{
    // The days (the digits before the point) and the time (those after, as a number of 10^-fd seconds).
    if (d.point > 18) return false;
    const int fd = s.fracSec;
    u128 days = 0;
    for (int i = 0; i < d.point && i < d.digits.size(); ++i) days = days * 10 + u128(d.digits[i] - '0');
    if (d.point > d.digits.size())
        for (int i = int(d.digits.size()); i < d.point; ++i) days *= 10;
    u128 units = 0;
    const u128 perDay = u128(86400) * pow10u(fd);
    if (s.hasTime) {
        // The digits after the point as F / 10^k.
        u128 f = 0;
        int k = 0;
        if (d.point < d.digits.size()) {
            const int start = std::max(d.point, 0);
            for (int i = start; i < d.digits.size(); ++i) f = f * 10 + u128(d.digits[i] - '0');
            k = int(d.digits.size()) - d.point;   // leading zeros of the fraction count as places
        }
        if (k > 0 && k <= 30) {
            const u128 denom = pow10u(k);
            units = (2 * f * perDay + denom) / (2 * denom);
        }
        if (units >= perDay) {
            units -= perDay;
            ++days;
        }
    }
    const bool time1904 = system == DateSystem::Date1904;
    if (negativeShown) {
        // Before day 0 there are no dates; a time (without a date) shows with its sign in the 1904 system only.
        if (!time1904 || s.hasDay) return false;
        o->add(QLatin1Char('-'));
    }
    int year = 0, month = 0, day = 0, weekday = 0;
    if (s.hasDay) {
        const u128 last = time1904 ? 2957003 : 2958465;
        if (days > last) return false;
        const double serial = double(qint64(days));
        if (!time1904 && days == 0) {
            year = 1900;
            month = 1;
            day = 0;
        } else if (!serialToDate(serial, system, &year, &month, &day)) {
            return false;
        }
        const qint64 w = qint64(days);
        weekday = time1904 ? int((w + 5) % 7) : int(((w - 1) % 7 + 7) % 7);
    }
    const u128 perHour = u128(3600) * pow10u(fd), perMinute = u128(60) * pow10u(fd), perSecond = pow10u(fd);
    u128 total = 0;
    if (s.hasElapsed) {
        if (days > 100000000000ULL) return false;
        total = days * perDay + units;
    }
    for (const Tok &k : s.toks) {
        switch (k.kind) {
        case T::Year: {
            QString y = QString::number(year);
            if (k.n <= 2) o->add(QString::number(year % 100).rightJustified(2, QLatin1Char('0')));
            else o->add(y.rightJustified(4, QLatin1Char('0')));
            break;
        }
        case T::Month:
            if (k.n == 1) o->add(QString::number(month));
            else if (k.n == 2) o->add(QString::number(month).rightJustified(2, QLatin1Char('0')));
            else if (k.n == 3) o->add(formatMonthName(s.lang, month, 1));
            else if (k.n == 4) o->add(formatMonthName(s.lang, month, 0));
            else o->add(formatMonthName(s.lang, month, 0).left(1));
            break;
        case T::Day:
            if (k.n == 1) o->add(QString::number(day));
            else if (k.n == 2) o->add(QString::number(day).rightJustified(2, QLatin1Char('0')));
            else if (k.n == 3) o->add(formatDayName(s.lang, weekday, 1));
            else o->add(formatDayName(s.lang, weekday, 0));
            break;
        case T::Hour: {
            int h = int((units / perHour) % 24);
            if (s.hasAmPm) h = h % 12 == 0 ? 12 : h % 12;
            o->add(k.n == 1 ? QString::number(h) : QString::number(h).rightJustified(2, QLatin1Char('0')));
            break;
        }
        case T::Minute: {
            const int m = s.hasElapsed ? int((total / perMinute) % 60) : int((units / perMinute) % 60);
            o->add(k.n == 1 ? QString::number(m) : QString::number(m).rightJustified(2, QLatin1Char('0')));
            break;
        }
        case T::Second: {
            const int sec = s.hasElapsed ? int((total / perSecond) % 60) : int((units / perSecond) % 60);
            o->add(k.n == 1 ? QString::number(sec) : QString::number(sec).rightJustified(2, QLatin1Char('0')));
            break;
        }
        case T::FracSec: {
            const u128 base = s.hasElapsed ? total : units;
            const u128 sub = base % pow10u(fd);
            QString digits = fd > 0 ? u128Text(sub).rightJustified(fd, QLatin1Char('0')) : QString();
            if (k.n > fd) digits += QString(k.n - fd, QLatin1Char('0'));
            o->add(QLatin1Char('.'));
            o->add(digits);
            break;
        }
        case T::AmPm: {
            const bool pm = (units / perHour) % 24 >= 12;
            const bool lower = k.ch.isLower();
            QString t = k.n ? (pm ? QStringLiteral("P") : QStringLiteral("A")) : (pm ? QStringLiteral("PM") : QStringLiteral("AM"));
            o->add(lower ? t.toLower() : t);
            break;
        }
        case T::Elapsed: {
            const QChar u = k.ch.toLower();
            const u128 v = u == QLatin1Char('h') ? total / perHour : u == QLatin1Char('m') ? total / perMinute : total / perSecond;
            o->add(u128Text(v).rightJustified(k.n, QLatin1Char('0')));
            break;
        }
        default: plainToken(k, o); break;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// Choosing the section, and the whole of formatting.

struct Pick {
    int index = 0;
    bool dropSign = false;
};

bool pick(const Parsed &p, double v, Pick *out)
{
    const int count = int(p.secs.size());
    const int nn = count == 4 ? 3 : count;
    out->dropSign = false;
    if (nn <= 1) {
        out->index = 0;
        return true;
    }
    const Cond &c0 = p.secs[0].cond, &c1 = p.secs[1].cond;
    if (nn == 2) {
        if (c0.op == Op::None && c1.op == Op::None) {
            out->index = v < 0 ? 1 : 0;
            out->dropSign = out->index == 1;
        } else if (c0.op == Op::None) {
            out->index = matches(c1, v) ? 1 : 0;
            out->dropSign = out->index == 1 && isNegative(c1);
        } else if (matches(c0, v)) {
            out->index = 0;
            out->dropSign = isNegative(c0);
        } else if (c1.op == Op::None) {
            out->index = 1;
            out->dropSign = c0.op == Op::Ge && c0.limit == 0;
        } else if (matches(c1, v)) {
            out->index = 1;
            out->dropSign = isNegative(c1);
        } else {
            return false;
        }
        return true;
    }
    Cond e0 = c0, e1 = c1;
    if (e0.op == Op::None) { e0.op = Op::Gt; e0.limit = 0; }
    if (e1.op == Op::None) { e1.op = Op::Lt; e1.limit = 0; }
    if (matches(e0, v)) {
        out->index = 0;
        out->dropSign = isNegative(e0);
    } else if (matches(e1, v)) {
        out->index = 1;
        out->dropSign = isNegative(e1);
    } else {
        out->index = 2;
    }
    return true;
}

Formatted failedResult()
{
    Formatted f;
    f.failed = true;
    return f;
}

Formatted finish(Out *o, int color)
{
    if (o->over) return failedResult();
    Formatted f;
    f.text = o->text;
    f.color = color;
    f.fillAt = o->fillAt;
    f.fillChar = o->fillChar;
    f.pads = o->pads;
    return f;
}

Formatted generalFormatted(double v, int maxChars, bool negativeShown)
{
    Dec d;
    toDec(v, &d);
    QString t;
    if (!generalText(d, negativeShown, maxChars, &t)) return failedResult();
    Formatted f;
    f.text = t;
    return f;
}

Formatted formatValue(const Parsed &p, double v, DateSystem system, int generalChars)
{
    if (!std::isfinite(v)) return failedResult();
    if (p.general || p.secs.isEmpty()) return generalFormatted(v, generalChars, v < 0);
    Pick pk;
    if (!pick(p, v, &pk)) return failedResult();
    const Section &s = p.secs[pk.index];
    const bool negativeShown = v < 0 && !pk.dropSign;
    Out o;
    switch (s.kind) {
    case Kind::Text: {
        Formatted g = generalFormatted(v, generalChars, v < 0 && !pk.dropSign);
        g.color = s.color;
        return g;
    }
    case Kind::General: {
        Dec d;
        toDec(v, &d);
        QString num;
        if (!generalText(d, false, generalChars, &num)) return failedResult();
        if (negativeShown) o.add(QLatin1Char('-'));
        for (const Tok &k : s.toks) {
            if (k.kind == T::General) o.add(num);
            else plainToken(k, &o);
        }
        return finish(&o, s.color);
    }
    case Kind::Date: {
        Dec d;
        toDec(v, &d);
        if (!renderDate(s, d, system, negativeShown, &o)) return failedResult();
        return finish(&o, s.color);
    }
    case Kind::Number: {
        Dec d;
        toDec(v, &d);
        renderNumber(s, d, negativeShown, &o);
        return finish(&o, s.color);
    }
    }
    return failedResult();
}

Formatted formatTextValue(const Parsed &p, const QString &text)
{
    Formatted f;
    if (p.textSection < 0) {
        f.text = text;
        return f;
    }
    const Section &s = p.secs[p.textSection];
    Out o;
    for (const Tok &k : s.toks) {
        if (k.kind == T::Text) o.add(text);
        else plainToken(k, &o);
        if (o.over) return failedResult();
    }
    if (o.over) return failedResult();
    f.text = o.text;
    f.color = s.color;
    f.fillAt = o.fillAt;
    f.fillChar = o.fillChar;
    f.pads = o.pads;
    return f;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// The public parts.

struct NumberFormat::Data {
    Parsed parsed;
};

NumberFormat::NumberFormat() : d(std::make_shared<Data>())
{
    const_cast<Data *>(d.get())->parsed.general = true;
}

NumberFormat::NumberFormat(const QString &code) : d(std::make_shared<Data>())
{
    const_cast<Data *>(d.get())->parsed = parseCode(code);
}

NumberFormat NumberFormat::builtin(int id)
{
    const QString code = builtinFormatCode(id);
    return code.isEmpty() ? NumberFormat() : NumberFormat(code);
}

bool NumberFormat::isValid() const { return d->parsed.valid; }
const QString &NumberFormat::code() const { return d->parsed.code; }
bool NumberFormat::isGeneral() const { return d->parsed.general; }

FormatKind NumberFormat::kind() const
{
    const Parsed &p = d->parsed;
    if (p.general || p.secs.isEmpty()) return FormatKind::General;
    switch (p.secs[0].kind) {
    case Kind::Number: return FormatKind::Number;
    case Kind::Date: return FormatKind::Date;
    case Kind::Text: return FormatKind::Text;
    case Kind::General: return FormatKind::General;
    }
    return FormatKind::General;
}

Formatted NumberFormat::format(double value, DateSystem system, int generalChars) const { return formatValue(d->parsed, value, system, generalChars); }

Formatted NumberFormat::formatText(const QString &text) const
{
    if (d->parsed.secs.isEmpty()) {
        Formatted f;
        f.text = text;
        return f;
    }
    return formatTextValue(d->parsed, text);
}

quint32 formatColorRgb(int color)
{
    // The 56 colors after the first eight of the default palette.
    static const quint32 palette[56] = {
        0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080,
        0xC0C0C0, 0x808080, 0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF, 0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF,
        0x800080, 0x800000, 0x008080, 0x0000FF, 0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99, 0x3366FF, 0x33CCCC,
        0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696, 0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333,
    };
    return color >= 1 && color <= 56 ? palette[color - 1] : 0;
}

QString builtinFormatCode(int id)
{
    switch (id) {
    case 0: return QStringLiteral("General");
    case 1: return QStringLiteral("0");
    case 2: return QStringLiteral("0.00");
    case 3: return QStringLiteral("#,##0");
    case 4: return QStringLiteral("#,##0.00");
    case 5: return QStringLiteral("\"$\"#,##0_);\\(\"$\"#,##0\\)");
    case 6: return QStringLiteral("\"$\"#,##0_);[Red]\\(\"$\"#,##0\\)");
    case 7: return QStringLiteral("\"$\"#,##0.00_);\\(\"$\"#,##0.00\\)");
    case 8: return QStringLiteral("\"$\"#,##0.00_);[Red]\\(\"$\"#,##0.00\\)");
    case 9: return QStringLiteral("0%");
    case 10: return QStringLiteral("0.00%");
    case 11: return QStringLiteral("0.00E+00");
    case 12: return QStringLiteral("# ?/?");
    case 13: return QStringLiteral("# ?" "?/??");
    case 14: return QStringLiteral("m/d/yyyy");
    case 15: return QStringLiteral("d-mmm-yy");
    case 16: return QStringLiteral("d-mmm");
    case 17: return QStringLiteral("mmm-yy");
    case 18: return QStringLiteral("h:mm AM/PM");
    case 19: return QStringLiteral("h:mm:ss AM/PM");
    case 20: return QStringLiteral("h:mm");
    case 21: return QStringLiteral("h:mm:ss");
    case 22: return QStringLiteral("m/d/yyyy h:mm");
    case 27: return QStringLiteral("[$-404]e/m/d");
    case 28: return QStringLiteral("[$-404]e\"年\"m\"月\"d\"日\"");
    case 29: return QStringLiteral("[$-404]e\"年\"m\"月\"d\"日\"");
    case 30: return QStringLiteral("m/d/yy");
    case 31: return QStringLiteral("yyyy\"年\"m\"月\"d\"日\"");
    case 32: return QStringLiteral("hh\"時\"mm\"分\"");
    case 33: return QStringLiteral("hh\"時\"mm\"分\"ss\"秒\"");
    case 34: return QStringLiteral("AM/PM hh\"時\"mm\"分\"");
    case 35: return QStringLiteral("AM/PM hh\"時\"mm\"分\"ss\"秒\"");
    case 36: return QStringLiteral("[$-404]e/m/d");
    case 37: return QStringLiteral("#,##0 ;(#,##0)");
    case 38: return QStringLiteral("#,##0 ;[Red](#,##0)");
    case 39: return QStringLiteral("#,##0.00;(#,##0.00)");
    case 40: return QStringLiteral("#,##0.00;[Red](#,##0.00)");
    case 41: return QStringLiteral("_(* #,##0_);_(* \\(#,##0\\);_(* \"-\"_);_(@_)");
    case 42: return QStringLiteral("_(\"$\"* #,##0_);_(\"$\"* \\(#,##0\\);_(\"$\"* \"-\"_);_(@_)");
    case 43: return QStringLiteral("_(* #,##0.00_);_(* \\(#,##0.00\\);_(* \"-\"??_);_(@_)");
    case 44: return QStringLiteral("_(\"$\"* #,##0.00_);_(\"$\"* \\(#,##0.00\\);_(\"$\"* \"-\"??_);_(@_)");
    case 45: return QStringLiteral("mm:ss");
    case 46: return QStringLiteral("[h]:mm:ss");
    case 47: return QStringLiteral("mmss.0");
    case 48: return QStringLiteral("##0.0E+0");
    case 49: return QStringLiteral("@");
    case 50: return QStringLiteral("[$-404]e/m/d");
    case 51: return QStringLiteral("[$-404]e\"年\"m\"月\"d\"日\"");
    case 52: return QStringLiteral("AM/PM hh\"時\"mm\"分\"");
    case 53: return QStringLiteral("AM/PM hh\"時\"mm\"分\"ss\"秒\"");
    case 54: return QStringLiteral("[$-404]e\"年\"m\"月\"d\"日\"");
    case 55: return QStringLiteral("AM/PM hh\"時\"mm\"分\"");
    case 56: return QStringLiteral("AM/PM hh\"時\"mm\"分\"ss\"秒\"");
    case 57: return QStringLiteral("[$-404]e/m/d");
    case 58: return QStringLiteral("[$-404]e\"年\"m\"月\"d\"日\"");
    default: return QString();
    }
}

namespace {

// The digits of a number rounded to `decimals`, with the groups of three if asked: the text FIXED and DOLLAR make of a number.
bool fixedDigits(double value, int decimals, bool commas, QString *out, bool *negative)
{
    if (!std::isfinite(value) || decimals > kMaxFixedDecimals || decimals < -kMaxFixedDecimals) return false;
    Dec d;
    toDec(value, &d);
    roundDec(&d, decimals);
    *negative = value < 0 && !d.zero();
    QString ints = intPart(d);
    if (ints.isEmpty()) ints = QStringLiteral("0");
    QString t;
    for (int i = 0; i < ints.size(); ++i) {
        if (commas && i > 0 && (ints.size() - i) % 3 == 0) t += QLatin1Char(',');
        t += ints[i];
    }
    if (decimals > 0) {
        t += QLatin1Char('.');
        for (int j = 0; j < decimals; ++j) t += QLatin1Char(fracDigit(d, j));
    }
    *out = t;
    return true;
}

} // namespace

bool formatTextFunction(double value, const QString &code, DateSystem system, QString *out)
{
    if (!std::isfinite(value)) return false;
    if (code.isEmpty()) {
        out->clear();
        return true;
    }
    const NumberFormat f(code);
    if (!f.isValid()) return false;
    const Formatted r = f.format(value, system, 0);
    if (r.failed) return false;
    *out = r.text;
    return true;
}

bool formatFixed(double value, int decimals, bool noCommas, QString *out)
{
    QString t;
    bool negative = false;
    if (!fixedDigits(value, decimals, !noCommas, &t, &negative)) return false;
    *out = negative ? QLatin1Char('-') + t : t;
    return true;
}

bool formatDollar(double value, int decimals, QString *out)
{
    QString t;
    bool negative = false;
    if (!fixedDigits(value, decimals, true, &t, &negative)) return false;
    *out = negative ? QStringLiteral("($") + t + QLatin1Char(')') : QLatin1Char('$') + t;
    return true;
}

} // namespace jp::calc
