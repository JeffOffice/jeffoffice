#include "calc/machine.h"

#include <algorithm>
#include <cmath>

namespace jp::calc {

bool coerceNumber(const Value &v, DateSystem system, double *out, ErrorKind *error)
{
    switch (v.kind()) {
    case Value::Kind::Number:
    case Value::Kind::Bool: *out = v.number(); return true;
    case Value::Kind::Empty: *out = 0; return true;
    case Value::Kind::Text:
        if (textToNumber(v.text(), system, out)) return true;
        *error = ErrorKind::Value;
        return false;
    case Value::Kind::Error: *error = v.errorKind(); return false;
    case Value::Kind::Array: *error = ErrorKind::Value; return false;
    }
    *error = ErrorKind::Value;
    return false;
}

bool coerceText(const Value &v, QString *out, ErrorKind *error)
{
    switch (v.kind()) {
    case Value::Kind::Number: *out = numberToText(v.number()); return true;
    case Value::Kind::Bool: *out = v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE"); return true;
    case Value::Kind::Empty: out->clear(); return true;
    case Value::Kind::Text: *out = v.text(); return true;
    case Value::Kind::Error: *error = v.errorKind(); return false;
    case Value::Kind::Array: *error = ErrorKind::Value; return false;
    }
    return false;
}

bool coerceBool(const Value &v, bool *out, ErrorKind *error)
{
    switch (v.kind()) {
    case Value::Kind::Number:
    case Value::Kind::Bool: *out = v.number() != 0; return true;
    case Value::Kind::Empty: *out = false; return true;
    case Value::Kind::Text:
        if (v.text().compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0) {
            *out = true;
            return true;
        }
        if (v.text().compare(QLatin1String("FALSE"), Qt::CaseInsensitive) == 0) {
            *out = false;
            return true;
        }
        *error = ErrorKind::Value;
        return false;
    case Value::Kind::Error: *error = v.errorKind(); return false;
    case Value::Kind::Array: *error = ErrorKind::Value; return false;
    }
    return false;
}

bool hasWildcards(const QString &s)
{
    for (const QChar ch : s)
        if (ch == QLatin1Char('?') || ch == QLatin1Char('*') || ch == QLatin1Char('~')) return true;
    return false;
}

bool wildcardMatch(const QString &text, int ti, const QString &pat, int pi, bool prefix)
{
    const int n = int(text.size()), m = int(pat.size());
    if (m - pi > 255) return false;   // (a criterion or a pattern of more than 255 characters is no pattern)
    int starP = -1, starT = -1;
    qint64 steps = 0;
    bool result = false;
    for (;;) {
        ++steps;
        if (pi < m) {
            const QChar p = pat[pi];
            if (p == QLatin1Char('*')) {
                while (pi < m && pat[pi] == QLatin1Char('*')) ++pi;
                if (pi == m) {
                    result = true;
                    break;
                }
                starP = pi;
                starT = ti;
                continue;
            }
            int advance = 1;
            bool any = false;
            QChar want = p;
            if (p == QLatin1Char('?')) {
                any = true;
            } else if (p == QLatin1Char('~') && pi + 1 < m) {
                want = pat[pi + 1];
                advance = 2;
            }
            if (ti < n && (any || text[ti].toCaseFolded() == want.toCaseFolded())) {
                ++ti;
                pi += advance;
                continue;
            }
        } else if (prefix || ti == n) {
            result = true;
            break;
        }
        // no match here: the last "*" takes one more character of the text
        if (starP < 0 || starT >= n) break;
        ++starT;
        ti = starT;
        pi = starP;
    }
    budget::addWork(steps);
    return result;
}

double powerOf(double x, double y)
{
    if (x == 0) return y > 0 ? 0.0 : INFINITY;
    const bool whole = y == std::floor(y);
    if (x < 0 && !whole) return NAN;
    const double base = std::fabs(x);
    double r;
    if (whole && std::fabs(y) <= 2147483648.0) {
        double acc = 1, b = base;
        qint64 n = qint64(std::fabs(y));
        while (n) {
            if (n & 1) acc *= b;
            b *= b;
            n >>= 1;
        }
        r = y < 0 ? 1 / acc : acc;
    } else {
        r = std::exp(y * std::log(base));
    }
    return x < 0 && whole && std::fmod(y, 2.0) != 0 ? -r : r;
}

Value Table::at(qint64 row, qint64 col) const
{
    if (m_isRef) {
        if (row < 0 || col < 0 || row >= m_fullRows || col >= m_fullCols) return Value();
        return m_machine->cell(m_area.sheet1, quint32(qint64(m_area.row1) + row), quint32(qint64(m_area.col1) + col));
    }
    if (row < 0 || col < 0 || row >= m_fullRows || col >= m_fullCols) return Value();
    return m_array.at(int(row), int(col));
}

qint64 Table::nextFilled(bool column, qint64 i, qint64 last) const
{
    if (i > last) return last + 1;
    if (!m_isRef) {
        for (; i <= last; ++i)
            if (!(column ? m_array.at(int(i), 0) : m_array.at(0, int(i))).isEmpty()) return i;
        return last + 1;
    }
    qint64 found = last + 1;
    const RefArea &a = m_area;
    auto take = [&](quint32 row, quint32 col, const Value &v) {
        if (v.isEmpty()) return true;
        found = column ? qint64(row) - qint64(a.row1) : qint64(col) - qint64(a.col1);
        return false;
    };
    if (column) m_machine->source().visit(a.sheet1, quint32(qint64(a.row1) + i), a.col1, quint32(qint64(a.row1) + last), a.col1, take);
    else m_machine->source().visit(a.sheet1, a.row1, quint32(qint64(a.col1) + i), a.row1, quint32(qint64(a.col1) + last), take);
    return found;
}

qint64 Table::previousFilled(bool column, qint64 i, qint64 first) const
{
    if (i < first) return first - 1;
    if (!m_isRef) {
        for (; i >= first; --i)
            if (!(column ? m_array.at(int(i), 0) : m_array.at(0, int(i))).isEmpty()) return i;
        return first - 1;
    }
    // the last place that holds something is the greatest x for which there is one from x to i: halve over x
    if (nextFilled(column, first, i) > i) return first - 1;
    qint64 lo = first, hi = i;
    while (lo < hi) {
        const qint64 mid = lo + (hi - lo + 1) / 2;
        if (nextFilled(column, mid, i) <= i) lo = mid;
        else hi = mid - 1;
    }
    return lo;
}

const EvalContext &Call::context() const { return m.context(); }

Value Call::scalar(int i)
{
    if (i >= argc()) return Value();
    return m.scalar(m_args[size_t(i)], false);
}

bool Call::number(int i, double *out)
{
    ErrorKind e = ErrorKind::Value;
    if (coerceNumber(scalar(i), m.context().dateSystem, out, &e)) return true;
    m_failure = Value::error(e);
    return false;
}

bool Call::integer(int i, qint64 *out)
{
    double x;
    if (!number(i, &x)) return false;
    x = std::trunc(x);
    if (!(std::fabs(x) <= 9007199254740992.0)) {
        m_failure = Value::error(ErrorKind::Num);
        return false;
    }
    *out = qint64(x);
    return true;
}

bool Call::text(int i, QString *out)
{
    ErrorKind e = ErrorKind::Value;
    if (coerceText(scalar(i), out, &e)) return true;
    m_failure = Value::error(e);
    return false;
}

bool Call::boolean(int i, bool *out)
{
    ErrorKind e = ErrorKind::Value;
    if (coerceBool(scalar(i), out, &e)) return true;
    m_failure = Value::error(e);
    return false;
}

bool Call::each(int i, const std::function<bool(const Value &, bool)> &f)
{
    if (i >= argc()) return true;
    const Item &it = m_args[size_t(i)];
    if (it.kind == Item::Kind::Missing) return f(Value(), true);
    if (it.kind == Item::Kind::Value && !it.value.isArray()) return f(it.value, true);
    return eachAt(i, [&](const Value &v, quint32, quint32) { return f(v, false); });
}

bool Call::eachAt(int i, const std::function<bool(const Value &, quint32, quint32)> &f)
{
    return eachCell(i, [&](const Value &v, int, quint32 row, quint32 col) { return f(v, row, col); });
}

bool Call::eachCell(int i, const std::function<bool(const Value &, int, quint32, quint32)> &f)
{
    if (i >= argc()) return true;
    const Item &it = m_args[size_t(i)];
    switch (it.kind) {
    case Item::Kind::Missing: return true;
    case Item::Kind::Value:
        if (it.value.isArray()) {
            for (int r = 0; r < it.value.rows(); ++r)
                for (int c = 0; c < it.value.cols(); ++c) {
                    const Value &v = it.value.at(r, c);
                    if (v.isEmpty()) continue;
                    if (!m.charge(1) || !f(v, -1, quint32(r), quint32(c))) return false;
                }
            return true;
        }
        return f(it.value, -1, 0, 0);
    case Item::Kind::Ref:
        for (int a = 0; a < it.areaCount(); ++a) {
            const RefArea &area = it.areaAt(a);
            for (int s = area.sheet1; s <= area.sheet2; ++s) {
                bool go = true;
                m.source().visit(s, area.row1, area.col1, area.row2, area.col2, [&](quint32 r, quint32 c, const Value &v) {
                    if (!m.charge(1)) {
                        go = false;
                        return false;
                    }
                    if (v.isEmpty()) return true;
                    if (!f(v, s, r, c)) {
                        go = false;
                        return false;
                    }
                    return true;
                });
                if (!go) return false;
            }
        }
        return true;
    }
    return true;
}

bool Call::table(int i, Table *t, bool clip)
{
    m_failure = Value::error(ErrorKind::Value);
    if (i >= argc() || m_args[size_t(i)].isMissing()) return false;
    const Item &it = m_args[size_t(i)];
    t->m_machine = &m;
    if (it.isRef()) {
        if (it.more || it.area.sheet1 != it.area.sheet2) return false;
        t->m_isRef = true;
        t->m_area = it.area;
        t->m_fullRows = t->m_rows = it.area.rows();
        t->m_fullCols = t->m_cols = it.area.cols();
        if (clip) {
            quint32 usedRows = 0, usedCols = 0;
            m.usedExtent(it.area.sheet1, &usedRows, &usedCols);
            t->m_rows = std::min<qint64>(t->m_fullRows, std::max<qint64>(0, qint64(usedRows) - qint64(it.area.row1)));
            t->m_cols = std::min<qint64>(t->m_fullCols, std::max<qint64>(0, qint64(usedCols) - qint64(it.area.col1)));
        }
        return true;
    }
    const Value &v = it.value;
    if (v.isError()) {
        m_failure = v;
        return false;
    }
    t->m_isRef = false;
    t->m_array = v;
    t->m_fullRows = t->m_rows = v.rows();
    t->m_fullCols = t->m_cols = v.cols();
    return true;
}

bool Call::array(int i, Value *out)
{
    if (i >= argc()) return false;
    const Item &it = m_args[size_t(i)];
    *out = it.isRef() ? m.toArray(it) : it.kind == Item::Kind::Value ? it.value : Value();
    if (out->isError()) {
        m_failure = *out;
        return false;
    }
    return true;
}

} // namespace jp::calc
