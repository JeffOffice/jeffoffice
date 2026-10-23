#include "calc/value.h"

#include "calc/budget.h"
#include "calc/numfmt.h"

#include <QByteArray>

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

const char *const kErrorTexts[] = {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A", "#GETTING_DATA", "#SPILL!", "#CALC!", "#FIELD!", "#BLOCKED!", "#CONNECT!", "#EXTERNAL!"};
constexpr int kKnownErrors = int(sizeof(kErrorTexts) / sizeof(kErrorTexts[0]));

const Value &naValue()
{
    static const Value v = Value::error(ErrorKind::NA);
    return v;
}

} // namespace

QString errorKindText(ErrorKind error)
{
    const int i = int(error);
    return i >= 0 && i < kKnownErrors ? QString::fromLatin1(kErrorTexts[i]) : QStringLiteral("#UNKNOWN!");
}

ErrorKind errorKindFromText(QStringView text, bool *known)
{
    for (int i = 0; i < kKnownErrors; ++i)
        if (text.compare(QLatin1String(kErrorTexts[i]), Qt::CaseInsensitive) == 0) {
            if (known) *known = true;
            return ErrorKind(i);
        }
    if (known) *known = false;
    return ErrorKind::Unknown;
}

ArrayData::ArrayData(const ArrayData &other) : rows(other.rows), cols(other.cols), cells(other.cells) { budget::arrayMade(qint64(cells.size())); }

ArrayData::~ArrayData() { budget::arrayFreed(qint64(cells.size())); }

Value Value::array(int rows, int cols)
{
    Value r;
    if (rows < 1) rows = 1;
    if (cols < 1) cols = 1;
    if (qint64(rows) * qint64(cols) > kMaxArrayCells) return r;
    if (!budget::mayMake(qint64(rows) * qint64(cols))) return r;
    r.m_kind = Kind::Array;
    r.m_array = std::make_shared<ArrayData>();
    r.m_array->rows = rows;
    r.m_array->cols = cols;
    r.m_array->cells.resize(size_t(rows) * size_t(cols));
    budget::arrayMade(qint64(r.m_array->cells.size()));
    return r;
}

int Value::rows() const { return m_kind == Kind::Array ? m_array->rows : 1; }
int Value::cols() const { return m_kind == Kind::Array ? m_array->cols : 1; }

const Value &Value::at(int row, int col) const
{
    if (m_kind != Kind::Array) return *this;
    if (row < 0 || col < 0 || row >= m_array->rows || col >= m_array->cols) return naValue();
    return m_array->cells[size_t(row) * size_t(m_array->cols) + size_t(col)];
}

void Value::set(int row, int col, const Value &v)
{
    if (m_kind != Kind::Array || row < 0 || col < 0 || row >= m_array->rows || col >= m_array->cols) return;
    if (m_array.use_count() > 1) {
        budget::mayMake(qint64(m_array->cells.size()));   // (past the budget this notes it: the copy is made all the same, and the evaluation stopped at its next step)
        m_array = std::make_shared<ArrayData>(*m_array);
    }
    m_array->cells[size_t(row) * size_t(m_array->cols) + size_t(col)] = v;
}

QString Value::debug() const
{
    switch (m_kind) {
    case Kind::Empty: return QStringLiteral("-");
    case Kind::Number: return QStringLiteral("n:") + QString::number(m_num, 'g', 15);
    case Kind::Text: return QStringLiteral("s:") + m_text;
    case Kind::Bool: return QStringLiteral("b:") + QString::number(m_num != 0 ? 1 : 0);
    case Kind::Error: return QStringLiteral("e:") + errorKindText(m_error);
    case Kind::Array: {
        QString s = QStringLiteral("a:%1x%2[").arg(m_array->rows).arg(m_array->cols);
        for (int r = 0; r < m_array->rows; ++r) {
            if (r) s += QLatin1Char(';');
            for (int c = 0; c < m_array->cols; ++c) {
                if (c) s += QLatin1Char(',');
                s += at(r, c).debug();
            }
        }
        return s + QLatin1Char(']');
    }
    }
    return {};
}

bool Value::operator==(const Value &o) const
{
    if (m_kind != o.m_kind) return false;
    switch (m_kind) {
    case Kind::Empty: return true;
    case Kind::Number:
    case Kind::Bool: return m_num == o.m_num;
    case Kind::Text: return m_text == o.m_text;
    case Kind::Error: return m_error == o.m_error;
    case Kind::Array:
        if (m_array == o.m_array) return true;
        if (m_array->rows != o.m_array->rows || m_array->cols != o.m_array->cols) return false;
        return m_array->cells == o.m_array->cells;
    }
    return false;
}

// ---- the rules

namespace {

const NumberFormat &generalFormat()
{
    static const NumberFormat f;
    return f;
}

bool isDigit(QChar c) { return c.unicode() >= '0' && c.unicode() <= '9'; }

bool isSpace(QChar c) { return c == QLatin1Char(' ') || c == QLatin1Char('\t') || c == QLatin1Char('\n') || c == QLatin1Char('\r'); }

// A number as VALUE reads one: [sign] [$] digits [. digits] [E sign digits] [%], or in parentheses for a negative. The groups of three with "," are only allowed
// in the whole part, the first of one to three digits and the rest of three.
bool parsePlainNumber(QStringView s, double *out)
{
    qsizetype i = 0;
    const qsizetype n = s.size();
    bool negative = false;
    if (n >= 2 && s[0] == QLatin1Char('(') && s[n - 1] == QLatin1Char(')')) {
        negative = true;
        s = s.mid(1, n - 2);
        if (s.isEmpty() || s[0] == QLatin1Char('-') || s[0] == QLatin1Char('+') || isSpace(s[0])) return false;
    }
    const qsizetype m = s.size();
    bool signSeen = false;
    if (i < m && (s[i] == QLatin1Char('-') || s[i] == QLatin1Char('+'))) {
        negative = negative != (s[i] == QLatin1Char('-'));
        signSeen = true;
        ++i;
    }
    if (i < m && s[i] == QLatin1Char('$')) {
        ++i;
        if (!signSeen && i < m && (s[i] == QLatin1Char('-') || s[i] == QLatin1Char('+'))) {
            negative = negative != (s[i] == QLatin1Char('-'));
            ++i;
        }
    }
    QByteArray digits;
    digits.reserve(int(m));
    int groupDigits = 0, groups = 0;
    bool anyDigit = false, comma = false;
    for (; i < m; ++i) {
        const QChar c = s[i];
        if (isDigit(c)) {
            digits.append(char(c.unicode()));
            ++groupDigits;
            anyDigit = true;
        } else if (c == QLatin1Char(',')) {
            // a group ends here: the first has 1 to 3 digits, the next exactly 3
            if (groupDigits == 0 || (groups > 0 && groupDigits != 3) || (groups == 0 && groupDigits > 3)) return false;
            if (i + 1 >= m || !isDigit(s[i + 1])) return false;
            comma = true;
            ++groups;
            groupDigits = 0;
        } else {
            break;
        }
    }
    if (comma && groupDigits != 3) return false;
    if (i < m && s[i] == QLatin1Char('.')) {
        digits.append('.');
        ++i;
        while (i < m && isDigit(s[i])) {
            digits.append(char(s[i].unicode()));
            anyDigit = true;
            ++i;
        }
    }
    if (!anyDigit) return false;
    if (i < m && (s[i] == QLatin1Char('e') || s[i] == QLatin1Char('E'))) {
        qsizetype j = i + 1;
        QByteArray exp = "e";
        if (j < m && (s[j] == QLatin1Char('+') || s[j] == QLatin1Char('-'))) exp.append(char(s[j++].unicode()));
        const qsizetype d0 = j;
        while (j < m && isDigit(s[j])) exp.append(char(s[j++].unicode()));
        if (j == d0 || j - d0 > 4) return false;
        digits += exp;
        i = j;
    }
    bool percent = false;
    if (i < m && s[i] == QLatin1Char('%')) {
        percent = true;
        ++i;
    }
    if (i != m) return false;
    bool ok = false;
    double v = QString::fromLatin1(digits).toDouble(&ok);
    if (!ok || !std::isfinite(v)) return false;
    if (percent) v /= 100;
    *out = negative ? -v : v;
    return true;
}

// A time of day: h:mm, h:mm:ss, h:mm:ss.fff, with AM or PM (or A or P) after it, as a part of a day.
bool parseTimeOfDay(QStringView s, double *out)
{
    s = s.trimmed();
    qsizetype i = 0;
    const qsizetype n = s.size();
    auto number = [&](int *v, int maxDigits) {
        int count = 0;
        *v = 0;
        while (i < n && isDigit(s[i]) && count < maxDigits) {
            *v = *v * 10 + (s[i].unicode() - '0');
            ++i;
            ++count;
        }
        return count > 0;
    };
    int h = 0, m = 0, sec = 0;
    if (!number(&h, 5) || i >= n || s[i] != QLatin1Char(':')) return false;
    ++i;
    if (!number(&m, 2)) return false;
    double frac = 0;
    if (i < n && s[i] == QLatin1Char(':')) {
        ++i;
        if (!number(&sec, 2)) return false;
        if (i < n && s[i] == QLatin1Char('.')) {
            ++i;
            double scale = 0.1;
            bool any = false;
            while (i < n && isDigit(s[i])) {
                frac += scale * (s[i].unicode() - '0');
                scale /= 10;
                ++i;
                any = true;
            }
            if (!any) return false;
        }
    }
    while (i < n && isSpace(s[i])) ++i;
    int meridian = 0;   // 1 AM, 2 PM
    if (i < n) {
        const QChar c = s[i].toUpper();
        if (c == QLatin1Char('A') || c == QLatin1Char('P')) {
            meridian = c == QLatin1Char('A') ? 1 : 2;
            ++i;
            if (i < n && s[i].toUpper() == QLatin1Char('M')) ++i;
        }
    }
    if (i != n) return false;
    if (m > 59 || sec > 59) return false;
    if (meridian) {
        if (h < 1 || h > 12) return false;
        h = h % 12 + (meridian == 2 ? 12 : 0);
    }
    *out = (double(h) * 3600.0 + double(m) * 60.0 + double(sec) + frac) / 86400.0;
    return true;
}

struct CollationKey {
    static quint32 of(QChar c)
    {
        const QChar f = c.toCaseFolded();
        const ushort u = f.unicode();
        if (isDigit(f)) return (1u << 24) | u;
        if (f.isLetter()) return (2u << 24) | u;
        return u;
    }
};

} // namespace

QString numberToText(double v)
{
    const Formatted f = generalFormat().format(v);
    return f.failed ? QString::number(v, 'g', 15) : f.text;
}

bool textToNumber(const QString &text, DateSystem system, double *out)
{
    qsizetype a = 0, b = text.size();
    while (a < b && isSpace(text[a])) ++a;
    while (b > a && isSpace(text[b - 1])) --b;
    if (a == b) return false;
    const QStringView s = QStringView(text).mid(a, b - a);
    if (parsePlainNumber(s, out)) return true;
    // a time of day alone
    double t = 0;
    if (parseTimeOfDay(s, &t)) {
        *out = t;
        return true;
    }
    // a date, and a time after it
    const qsizetype colon = s.indexOf(QLatin1Char(':'));
    if (colon > 0) {
        qsizetype start = colon;
        while (start > 0 && isDigit(s[start - 1])) --start;
        if (start == 0 || !parseTimeOfDay(s.mid(start), &t)) return false;
        double d = 0;
        if (!dateValueOf(s.left(start).trimmed().toString(), system, &d)) return false;
        *out = d + t;
        return true;
    }
    double d = 0;
    if (dateValueOf(s.toString(), system, &d)) {
        *out = d;
        return true;
    }
    return false;
}

namespace {

// A text taken apart for comparing: the keys of its letters, digits, and symbols, and after each the accents (marks) that follow it.
struct Decomposed {
    std::vector<quint32> keys;
    std::vector<QString> marks;
};

Decomposed decompose(const QString &s)
{
    Decomposed d;
    const QString n = s.normalized(QString::NormalizationForm_D);
    for (const QChar ch : n) {
        const QChar::Category cat = ch.category();
        const bool mark = cat == QChar::Mark_NonSpacing || cat == QChar::Mark_SpacingCombining || cat == QChar::Mark_Enclosing;
        if (mark && !d.keys.empty()) {
            d.marks.back() += ch;
            continue;
        }
        d.keys.push_back(CollationKey::of(ch));
        d.marks.emplace_back();
    }
    return d;
}

// The order of two texts with a character that is not ASCII, where the characters of the first difference are: the keys of the characters without their accents
// first, and the accents (from the left) if the keys are the same.
int compareDecomposed(const QString &a, const QString &b)
{
    const Decomposed x = decompose(a), y = decompose(b);
    const size_t n = std::min(x.keys.size(), y.keys.size());
    for (size_t i = 0; i < n; ++i)
        if (x.keys[i] != y.keys[i]) return x.keys[i] < y.keys[i] ? -1 : 1;
    if (x.keys.size() != y.keys.size()) return x.keys.size() < y.keys.size() ? -1 : 1;
    for (size_t i = 0; i < n; ++i)
        if (x.marks[i] != y.marks[i]) return x.marks[i] < y.marks[i] ? -1 : 1;
    return 0;
}

} // namespace

int compareText(const QString &a, const QString &b)
{
    const qsizetype n = std::min(a.size(), b.size());
    qsizetype i = 0;
    int order = 0;
    for (; i < n; ++i) {
        if (a[i] == b[i]) continue;
        // (letters with accents: the keys of the letters first, and a text that is not ASCII is compared whole)
        if (a[i].unicode() > 127 || b[i].unicode() > 127) {
            budget::addWork(qint64((a.size() + b.size()) / 32));
            return compareDecomposed(a, b);
        }
        const quint32 x = CollationKey::of(a[i]), y = CollationKey::of(b[i]);
        if (x != y) {
            order = x < y ? -1 : 1;
            break;
        }
    }
    if (i == n) order = a.size() == b.size() ? 0 : (a.size() < b.size() ? -1 : 1);
    // comparing takes as long as the texts agree: a step for each 32 characters, counted with the next charge of the evaluation
    if (i >= 32) budget::addWork(qint64(i / 32));
    return order;
}

double roundTo15Digits(double v)
{
    if (v == 0 || !std::isfinite(v)) return v;
    return QString::number(v, 'e', 14).toDouble();
}

bool sameTo15Digits(double a, double b)
{
    if (a == b) return true;
    if (!std::isfinite(a) || !std::isfinite(b)) return false;
    const double scale = std::max(std::fabs(a), std::fabs(b));
    // 15 digits keep a relative error of 5e-15 at the most, so numbers that differ more than twice that cannot round to the same one
    if (std::fabs(a - b) > scale * 2e-14) return false;
    return roundTo15Digits(a) == roundTo15Digits(b);
}

int compareValues(const Value &a, const Value &b)
{
    auto rank = [](const Value &v, const Value &other) {
        switch (v.kind()) {
        case Value::Kind::Number: return 0;
        case Value::Kind::Text: return 1;
        case Value::Kind::Bool: return 2;
        default: break;
        }
        // an empty value is of the kind of the one it is compared with
        switch (other.kind()) {
        case Value::Kind::Text: return 1;
        case Value::Kind::Bool: return 2;
        default: return 0;
        }
    };
    const int ra = rank(a, b), rb = rank(b, a);
    if (ra != rb) return ra < rb ? -1 : 1;
    if (ra == 0) {
        const double x = a.number(), y = b.number();
        if (sameTo15Digits(x, y)) return 0;
        return x < y ? -1 : 1;
    }
    if (ra == 1) return compareText(a.isText() ? a.text() : QString(), b.isText() ? b.text() : QString());
    const bool x = a.boolean(), y = b.boolean();
    return x == y ? 0 : (x ? 1 : -1);
}

} // namespace jp::calc
