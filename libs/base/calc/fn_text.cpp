// The text functions: cutting, finding, joining, changing case, and turning values to text and text to values.
#include "calc/machine.h"
#include "calc/numfmt.h"

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

// Windows-1252 for the codes 128 to 159, which are no Latin-1 characters.
const ushort kCp1252[32] = {0x20AC, 0x81, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x8D, 0x017D, 0x8F,
                            0x90, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x9D, 0x017E, 0x0178};

Value textResult(const QString &s)
{
    if (s.size() > kMaxValueTextChars) return Value::error(ErrorKind::Value);
    return Value::text(s);
}

// ---- cutting

Value fnLen(Call &c)
{
    QString s;
    if (!c.text(0, &s)) return c.failure();
    return Value::number(double(s.size()));
}

Value fnLeft(Call &c)
{
    QString s;
    double n;
    if (!c.text(0, &s) || !c.numberOr(1, 1, &n)) return c.failure();
    n = std::trunc(n);
    if (n < 0) return c.error(ErrorKind::Value);
    return Value::text(s.left(int(std::min(n, 1e9))));
}

Value fnRight(Call &c)
{
    QString s;
    double n;
    if (!c.text(0, &s) || !c.numberOr(1, 1, &n)) return c.failure();
    n = std::trunc(n);
    if (n < 0) return c.error(ErrorKind::Value);
    return Value::text(s.right(int(std::min(n, 1e9))));
}

Value fnMid(Call &c)
{
    QString s;
    double start, n;
    if (!c.text(0, &s) || !c.number(1, &start) || !c.number(2, &n)) return c.failure();
    start = std::trunc(start);
    n = std::trunc(n);
    if (start < 1 || n < 0) return c.error(ErrorKind::Value);
    if (start > double(s.size())) return Value::text(QString());
    return Value::text(s.mid(int(start) - 1, int(std::min(n, 1e9))));
}

Value fnReplace(Call &c)
{
    QString s, with;
    double start, n;
    if (!c.text(0, &s) || !c.number(1, &start) || !c.number(2, &n) || !c.text(3, &with)) return c.failure();
    start = std::trunc(start);
    n = std::trunc(n);
    if (start < 1 || n < 0) return c.error(ErrorKind::Value);
    const int from = int(std::min(start - 1, double(s.size())));
    return textResult(s.left(from) + with + s.mid(int(std::min(double(from) + n, double(s.size())))));
}

Value fnTrim(Call &c)
{
    QString s;
    if (!c.text(0, &s)) return c.failure();
    QString out;
    out.reserve(s.size());
    bool pendingSpace = false;
    for (const QChar ch : s) {
        if (ch == QLatin1Char(' ')) {
            pendingSpace = !out.isEmpty();
        } else {
            if (pendingSpace) out += QLatin1Char(' ');
            pendingSpace = false;
            out += ch;
        }
    }
    return Value::text(out);
}

Value fnClean(Call &c)
{
    QString s;
    if (!c.text(0, &s)) return c.failure();
    QString out;
    for (const QChar ch : s)
        if (ch.unicode() >= 32) out += ch;
    return Value::text(out);
}

Value fnRept(Call &c)
{
    QString s;
    double n;
    if (!c.text(0, &s) || !c.number(1, &n)) return c.failure();
    n = std::trunc(n);
    if (n < 0) return c.error(ErrorKind::Value);
    if (s.isEmpty() || n == 0) return Value::text(QString());
    if (double(s.size()) * n > kMaxValueTextChars) return c.error(ErrorKind::Value);
    return Value::text(s.repeated(int(n)));
}

Value fnSubstitute(Call &c)
{
    QString s, from, to;
    if (!c.text(0, &s) || !c.text(1, &from) || !c.text(2, &to)) return c.failure();
    double instance = 0;
    const bool one = c.argc() > 3 && !c.missing(3);
    if (one) {
        if (!c.number(3, &instance)) return c.failure();
        instance = std::trunc(instance);
        if (instance < 1) return c.error(ErrorKind::Value);
    }
    if (from.isEmpty()) return Value::text(s);
    QString out;
    int pos = 0, found = 0;
    for (;;) {
        const int at = s.indexOf(from, pos, Qt::CaseSensitive);
        if (at < 0) break;
        ++found;
        out += s.mid(pos, at - pos);
        if (!one || double(found) == instance) out += to;
        else out += from;
        pos = at + from.size();
        if (one && double(found) == instance) break;
    }
    out += s.mid(pos);
    return textResult(out);
}

// ---- finding

Value findText(Call &c, bool caseSensitive)
{
    QString what, within;
    double start;
    if (!c.text(0, &what) || !c.text(1, &within) || !c.numberOr(2, 1, &start)) return c.failure();
    start = std::trunc(start);
    if (start < 1 || start > double(within.size()) + (what.isEmpty() ? 1 : 0)) return c.error(ErrorKind::Value);
    const int from = int(start) - 1;
    if (what.isEmpty()) return Value::number(double(from + 1));
    if (caseSensitive) {
        const int at = within.indexOf(what, from, Qt::CaseSensitive);
        return at < 0 ? c.error(ErrorKind::Value) : Value::number(double(at + 1));
    }
    if (!hasWildcards(what)) {
        const int at = within.indexOf(what, from, Qt::CaseInsensitive);
        return at < 0 ? c.error(ErrorKind::Value) : Value::number(double(at + 1));
    }
    for (int i = from; i <= within.size(); ++i)
        if (wildcardMatch(within, i, what, 0, true)) return Value::number(double(i + 1));
    return c.error(ErrorKind::Value);
}
Value fnFind(Call &c) { return findText(c, true); }
Value fnSearch(Call &c) { return findText(c, false); }

Value fnExact(Call &c)
{
    QString a, b;
    if (!c.text(0, &a) || !c.text(1, &b)) return c.failure();
    return Value::boolean(a == b);
}

// ---- joining and changing case

Value fnConcatenate(Call &c)
{
    QString out;
    for (int i = 0; i < c.argc(); ++i) {
        QString s;
        if (!c.text(i, &s)) return c.failure();
        out += s;
        if (out.size() > kMaxValueTextChars) return c.error(ErrorKind::Value);
    }
    return Value::text(out);
}

Value fnConcat(Call &c)
{
    QString out;
    Value error;
    for (int i = 0; i < c.argc(); ++i) {
        bool ok = true;
        const bool done = c.each(i, [&](const Value &v, bool) {
            QString s;
            ErrorKind e = ErrorKind::Value;
            if (!coerceText(v, &s, &e)) {
                error = Value::error(e);
                ok = false;
                return false;
            }
            out += s;
            return out.size() <= kMaxValueTextChars;
        });
        if (!done) return ok ? c.error(ErrorKind::Value) : error;
    }
    return Value::text(out);
}

Value fnTextJoin(Call &c)
{
    QString delimiter;
    bool ignoreEmpty;
    if (!c.text(0, &delimiter) || !c.boolean(1, &ignoreEmpty)) return c.failure();
    QString out;
    bool first = true;
    Value error;
    for (int i = 2; i < c.argc(); ++i) {
        bool ok = true;
        const bool done = c.each(i, [&](const Value &v, bool) {
            QString s;
            ErrorKind e = ErrorKind::Value;
            if (!coerceText(v, &s, &e)) {
                error = Value::error(e);
                ok = false;
                return false;
            }
            if (s.isEmpty() && ignoreEmpty) return true;
            if (!first) out += delimiter;
            first = false;
            out += s;
            return out.size() <= kMaxValueTextChars;
        });
        if (!done) return ok ? c.error(ErrorKind::Value) : error;
    }
    return Value::text(out);
}

Value fnUpper(Call &c)
{
    QString s;
    if (!c.text(0, &s)) return c.failure();
    return Value::text(s.toUpper());
}

Value fnLower(Call &c)
{
    QString s;
    if (!c.text(0, &s)) return c.failure();
    return Value::text(s.toLower());
}

Value fnProper(Call &c)
{
    QString s;
    if (!c.text(0, &s)) return c.failure();
    bool afterLetter = false;
    for (QChar &ch : s) {
        if (ch.isLetter()) {
            ch = afterLetter ? ch.toLower() : ch.toUpper();
            afterLetter = true;
        } else {
            afterLetter = false;
        }
    }
    return Value::text(s);
}

Value fnHyperlink(Call &c)
{
    const Value link = c.scalar(0);
    if (c.argc() > 1 && !c.missing(1)) return c.scalar(1);
    return link;
}

Value fnT(Call &c)
{
    const Value v = c.scalar(0);
    if (v.isText() || v.isError()) return v;
    return Value::text(QString());
}

Value fnChar(Call &c)
{
    double n;
    if (!c.number(0, &n)) return c.failure();
    n = std::trunc(n);
    if (n < 1 || n > 255) return c.error(ErrorKind::Value);
    const int k = int(n);
    return Value::text(QString(QChar(k >= 128 && k < 160 ? kCp1252[k - 128] : ushort(k))));
}

Value fnCode(Call &c)
{
    QString s;
    if (!c.text(0, &s)) return c.failure();
    if (s.isEmpty()) return c.error(ErrorKind::Value);
    const ushort u = s[0].unicode();
    if (u < 128 || (u >= 160 && u < 256)) return Value::number(u);
    for (int i = 0; i < 32; ++i)
        if (kCp1252[i] == u) return Value::number(128 + i);
    return Value::number(63);
}

// ---- values and text

Value fnValue(Call &c)
{
    const Value v = c.scalar(0);
    switch (v.kind()) {
    case Value::Kind::Number: return v;
    case Value::Kind::Empty: return Value::number(0);
    case Value::Kind::Error: return v;
    case Value::Kind::Text: {
        double x;
        if (textToNumber(v.text(), c.context().dateSystem, &x)) return Value::number(x);
        return c.error(ErrorKind::Value);
    }
    default: return c.error(ErrorKind::Value);
    }
}

Value fnText(Call &c)
{
    const Value v = c.scalar(0);
    QString code;
    if (!c.text(1, &code)) return c.failure();
    if (v.isError()) return v;
    if (v.isBool()) return Value::text(v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE"));
    double x = 0;
    if (v.isText()) {
        if (!textToNumber(v.text(), c.context().dateSystem, &x)) {
            const NumberFormat f(code);
            if (!f.isValid()) return c.error(ErrorKind::Value);
            const Formatted r = f.formatText(v.text());
            return r.failed ? c.error(ErrorKind::Value) : Value::text(r.text);
        }
    } else if (v.isNumber()) {
        x = v.number();
    }
    QString out;
    if (!formatTextFunction(x, code, c.context().dateSystem, &out)) return c.error(ErrorKind::Value);
    return textResult(out);
}

Value fnFixed(Call &c)
{
    double x, d;
    bool noCommas;
    if (!c.number(0, &x) || !c.numberOr(1, 2, &d) || !c.booleanOr(2, false, &noCommas)) return c.failure();
    QString out;
    if (!formatFixed(x, int(std::max(-1000.0, std::min(1000.0, std::trunc(d)))), noCommas, &out)) return c.error(ErrorKind::Value);
    return textResult(out);
}

Value fnDollar(Call &c)
{
    double x, d;
    if (!c.number(0, &x) || !c.numberOr(1, 2, &d)) return c.failure();
    QString out;
    if (!formatDollar(x, int(std::max(-1000.0, std::min(1000.0, std::trunc(d)))), &out)) return c.error(ErrorKind::Value);
    return textResult(out);
}

} // namespace

void registerTextFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"LEN", 1, 1, "v", 0, fnLen});
    r.push_back({"LEFT", 1, 2, "vv", 0, fnLeft});
    r.push_back({"RIGHT", 1, 2, "vv", 0, fnRight});
    r.push_back({"MID", 3, 3, "vvv", 0, fnMid});
    r.push_back({"REPLACE", 4, 4, "vvvv", 0, fnReplace});
    r.push_back({"TRIM", 1, 1, "v", 0, fnTrim});
    r.push_back({"CLEAN", 1, 1, "v", 0, fnClean});
    r.push_back({"REPT", 2, 2, "vv", 0, fnRept});
    r.push_back({"SUBSTITUTE", 3, 4, "vvvv", 0, fnSubstitute});
    r.push_back({"FIND", 2, 3, "vvv", 0, fnFind});
    r.push_back({"SEARCH", 2, 3, "vvv", 0, fnSearch});
    r.push_back({"EXACT", 2, 2, "vv", 0, fnExact});
    r.push_back({"CONCATENATE", 1, 255, "v", 0, fnConcatenate});
    r.push_back({"CONCAT", 1, 255, "r", kFnFuture, fnConcat});
    r.push_back({"TEXTJOIN", 3, 255, "vvr", kFnFuture, fnTextJoin});
    r.push_back({"UPPER", 1, 1, "v", 0, fnUpper});
    r.push_back({"LOWER", 1, 1, "v", 0, fnLower});
    r.push_back({"PROPER", 1, 1, "v", 0, fnProper});
    r.push_back({"HYPERLINK", 1, 2, "vv", 0, fnHyperlink});
    r.push_back({"T", 1, 1, "v", 0, fnT});
    r.push_back({"CHAR", 1, 1, "v", 0, fnChar});
    r.push_back({"CODE", 1, 1, "v", 0, fnCode});
    r.push_back({"VALUE", 1, 1, "v", 0, fnValue});
    r.push_back({"TEXT", 2, 2, "vv", 0, fnText});
    r.push_back({"FIXED", 1, 3, "vvv", 0, fnFixed});
    r.push_back({"DOLLAR", 1, 2, "vv", 0, fnDollar});
}

} // namespace jp::calc
