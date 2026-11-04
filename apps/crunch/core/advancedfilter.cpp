#include "core/advancedfilter.h"

#include "calc/value.h"

#include <QCoreApplication>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

constexpr qint64 kMaxFilterWork = 4000000;   // (cells looked at)

AdvancedFilterResult refuse(const QString &why)
{
    AdvancedFilterResult r;
    r.ok = false;
    r.problem = why;
    return r;
}

struct Condition {
    enum class Op { Begins, Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual } op = Op::Begins;
    bool numeric = false;
    double number = 0;
    QString text;      // folded
    bool blank = false;
    QRegularExpression pattern;   // for text with ? or *
    bool wild = false;
};

bool readNumber(const QString &text, double *out)
{
    bool ok = false;
    double v = QLocale().toDouble(text.trimmed(), &ok);
    if (!ok) v = QLocale::c().toDouble(text.trimmed(), &ok);
    if (ok) *out = v;
    return ok;
}

Condition parseCondition(const QString &typed, const Value &value)
{
    Condition c;
    if (value.isNumber() && !typed.startsWith(QLatin1Char('='))) {
        c.op = Condition::Op::Equal;
        c.numeric = true;
        c.number = value.number();
        return c;
    }
    QString t = typed;
    struct { const char *sign; Condition::Op op; } signs[] = {{"<=", Condition::Op::LessEqual}, {">=", Condition::Op::GreaterEqual}, {"<>", Condition::Op::NotEqual}, {"<", Condition::Op::Less}, {">", Condition::Op::Greater}, {"=", Condition::Op::Equal}};
    bool signed_ = false;
    for (const auto &s : signs)
        if (t.startsWith(QLatin1String(s.sign))) {
            c.op = s.op;
            t = t.mid(int(strlen(s.sign)));
            signed_ = true;
            break;
        }
    if (signed_ && t.isEmpty()) {
        c.blank = true;
        return c;
    }
    double n = 0;
    if (signed_ && readNumber(t, &n)) {
        c.numeric = true;
        c.number = n;
        return c;
    }
    c.text = t.toCaseFolded();
    c.wild = c.text.contains(QLatin1Char('?')) || c.text.contains(QLatin1Char('*'));
    if (c.wild) {
        QString re;
        for (const QChar ch : c.text) re += ch == QLatin1Char('?') ? QStringLiteral(".") : ch == QLatin1Char('*') ? QStringLiteral(".*") : QRegularExpression::escape(QString(ch));
        c.pattern = QRegularExpression(QLatin1Char('^') + re + (c.op == Condition::Op::Begins ? QString() : QStringLiteral("$")), QRegularExpression::DotMatchesEverythingOption);
    }
    return c;
}

bool meets(const Condition &c, const Value &v)
{
    if (c.blank) return c.op == Condition::Op::NotEqual ? !v.isEmpty() : v.isEmpty();
    if (c.numeric) {
        if (!v.isNumber()) return c.op == Condition::Op::NotEqual;
        const double x = v.number();
        switch (c.op) {
        case Condition::Op::Equal: return x == c.number;
        case Condition::Op::NotEqual: return x != c.number;
        case Condition::Op::Less: return x < c.number;
        case Condition::Op::LessEqual: return x <= c.number;
        case Condition::Op::Greater: return x > c.number;
        case Condition::Op::GreaterEqual: return x >= c.number;
        default: return false;
        }
    }
    if (!v.isText()) return c.op == Condition::Op::NotEqual;
    const QString s = v.text().toCaseFolded();
    const bool same = c.wild ? c.pattern.match(s).hasMatch() : (c.op == Condition::Op::Begins ? s.startsWith(c.text) : s == c.text);
    switch (c.op) {
    case Condition::Op::Begins:
    case Condition::Op::Equal: return same;
    case Condition::Op::NotEqual: return !same;
    case Condition::Op::Less: return s < c.text;
    case Condition::Op::LessEqual: return s <= c.text;
    case Condition::Op::Greater: return s > c.text;
    case Condition::Op::GreaterEqual: return s >= c.text;
    }
    return false;
}

QString keyOfRow(const EditSession &s, int sheet, quint32 row, const CellRange &list)
{
    QString key;
    for (quint32 col = list.col1; col <= list.col2; ++col) {
        const Value v = s.valueAt(sheet, row, col);
        key += (v.isText() ? v.text().toCaseFolded() : v.isNumber() ? jp::calc::numberToText(v.number()) : s.editText(sheet, row, col)) + QLatin1Char('\x1f');
    }
    return key;
}

} // namespace

AdvancedFilterResult advancedFilterRows(const EditSession &session, int sheet, const AdvancedFilterSpec &spec)
{
    const CellRange &list = spec.list, &crit = spec.criteria;
    if (!list.valid || list.row2 <= list.row1) return refuse(QCoreApplication::translate("jp::crunch::AdvancedFilter", "The list needs a heading row and at least one row under it."));
    if (!crit.valid || crit.row2 < crit.row1) return refuse(QCoreApplication::translate("jp::crunch::AdvancedFilter", "The conditions need a heading row. Write the headings of the columns to test, and the conditions under them."));
    if (qint64(list.row2 - list.row1 + 1) * (list.col2 - list.col1 + 1) + qint64(crit.row2 - crit.row1 + 1) * (crit.col2 - crit.col1 + 1) > kMaxFilterWork)
        return refuse(QCoreApplication::translate("jp::crunch::AdvancedFilter", "The list is too large to filter at once."));
    // the list column each condition column tests
    std::vector<int> listColumn(crit.col2 - crit.col1 + 1, -1);
    for (quint32 cc = crit.col1; cc <= crit.col2; ++cc) {
        const Value head = session.valueAt(sheet, crit.row1, cc);
        const QString name = head.isText() ? head.text().trimmed().toCaseFolded() : QString();
        if (name.isEmpty()) continue;
        for (quint32 lc = list.col1; lc <= list.col2; ++lc) {
            const Value h = session.valueAt(sheet, list.row1, lc);
            if (h.isText() && h.text().trimmed().toCaseFolded() == name) {
                listColumn[cc - crit.col1] = int(lc);
                break;
            }
        }
    }
    // the conditions of each row of conditions (a row with none lets every row through)
    struct Test {
        quint32 column;
        Condition cond;
    };
    std::vector<std::vector<Test>> rowsOfTests;
    bool anyOpen = crit.row2 == crit.row1;
    for (quint32 r = crit.row1 + 1; r <= crit.row2; ++r) {
        std::vector<Test> tests;
        for (quint32 cc = crit.col1; cc <= crit.col2; ++cc) {
            const Value v = session.valueAt(sheet, r, cc);
            const QString typed = session.editText(sheet, r, cc);
            if (v.isEmpty() && typed.isEmpty()) continue;
            if (listColumn[cc - crit.col1] < 0) continue;
            tests.push_back({quint32(listColumn[cc - crit.col1]), parseCondition(typed, v)});
        }
        if (tests.empty()) anyOpen = true;
        rowsOfTests.push_back(std::move(tests));
    }
    AdvancedFilterResult out;
    QSet<QString> seen;
    for (quint32 row = list.row1 + 1; row <= list.row2; ++row) {
        bool pass = anyOpen;
        for (size_t t = 0; t < rowsOfTests.size() && !pass; ++t) {
            bool all = true;
            for (const Test &test : rowsOfTests[t])
                if (!meets(test.cond, session.valueAt(sheet, row, test.column))) {
                    all = false;
                    break;
                }
            pass = all && !rowsOfTests[t].empty();
        }
        if (!pass) continue;
        if (spec.unique) {
            const QString key = keyOfRow(session, sheet, row, list);
            if (seen.contains(key)) continue;
            seen.insert(key);
        }
        out.rows.push_back(row);
    }
    return out;
}

} // namespace jp::crunch
