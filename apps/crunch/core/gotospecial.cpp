#include "core/gotospecial.h"

#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QRegularExpression>

#include <algorithm>
#include <array>
#include <map>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

// "Sheet!ref", "'Sheet name'!ref": the sheet name and the rest; false if there is no sheet part.
bool splitSheet(const QString &text, QString *sheetName, QString *rest)
{
    if (text.startsWith(QLatin1Char('\''))) {
        QString name;
        qsizetype i = 1;
        for (; i < text.size(); ++i) {
            if (text.at(i) == QLatin1Char('\'')) {
                if (i + 1 < text.size() && text.at(i + 1) == QLatin1Char('\'')) {
                    name += QLatin1Char('\'');
                    ++i;
                    continue;
                }
                break;
            }
            name += text.at(i);
        }
        if (i + 1 < text.size() && text.at(i + 1) == QLatin1Char('!')) {
            *sheetName = name;
            *rest = text.mid(i + 2);
            return true;
        }
        return false;
    }
    const qsizetype bang = text.lastIndexOf(QLatin1Char('!'));
    if (bang <= 0) return false;
    *sheetName = text.left(bang);
    *rest = text.mid(bang + 1);
    return true;
}

int sheetNamed(const EditSession &session, const QString &name)
{
    const Workbook *book = session.workbook();
    if (!book) return -1;
    for (int i = 0; i < book->info().sheets.size(); ++i) {
        const SheetEntry &e = book->info().sheets[i];
        if (!e.deleted && e.name.compare(name, Qt::CaseInsensitive) == 0) return i;
    }
    return -1;
}

bool parseArea(const QString &text, GoToTarget *out)
{
    static const QRegularExpression columns(QStringLiteral("^\\$?([A-Za-z]{1,3}):\\$?([A-Za-z]{1,3})$")), rows(QStringLiteral("^\\$?(\\d{1,7}):\\$?(\\d{1,7})$"));
    CellRange range;
    if (const auto m = columns.match(text); m.hasMatch()) {
        quint32 r = 0, c1 = 0, c2 = 0;
        if (!parseCellRef(m.captured(1) + QLatin1Char('1'), &r, &c1) || !parseCellRef(m.captured(2) + QLatin1Char('1'), &r, &c2)) return false;
        out->range.valid = true;
        out->range.row1 = 0;
        out->range.row2 = kMaxRows - 1;
        out->range.col1 = std::min(c1, c2);
        out->range.col2 = std::max(c1, c2);
        out->wholeColumns = true;
        return true;
    }
    if (const auto m = rows.match(text); m.hasMatch()) {
        const qint64 a = m.captured(1).toLongLong() - 1, b = m.captured(2).toLongLong() - 1;
        if (a < 0 || b < 0 || a >= qint64(kMaxRows) || b >= qint64(kMaxRows)) return false;
        out->range.valid = true;
        out->range.row1 = quint32(std::min(a, b));
        out->range.row2 = quint32(std::max(a, b));
        out->range.col1 = 0;
        out->range.col2 = kMaxColumns - 1;
        out->wholeRows = true;
        return true;
    }
    if (parseRangeRef(text, &range)) {
        out->range = range;
        return true;
    }
    return false;
}

} // namespace

GoToTarget resolveGoTo(const EditSession &session, const QString &input, int sheet)
{
    GoToTarget t;
    t.sheet = sheet;
    QString text = input.trimmed();
    if (text.isEmpty()) {
        t.problem = QCoreApplication::translate("jp::crunch::GoTo", "Type a cell, a range such as B3:D9, or the name of a range.");
        return t;
    }
    QString name, rest;
    if (splitSheet(text, &name, &rest)) {
        const int found = sheetNamed(session, name);
        if (found < 0) {
            t.problem = QCoreApplication::translate("jp::crunch::GoTo", "There is no sheet called \"%1\" in this workbook.").arg(name);
            return t;
        }
        t.sheet = found;
        text = rest;
    }
    if (parseArea(text, &t)) {
        t.ok = true;
        return t;
    }
    // a defined name: of the sheet in view first, then of the workbook (the names as they are now: made, changed, or taken away since the file was read)
    const Workbook *book = session.workbook();
    const Calculation *namesOf = session.calculation();
    if (book && namesOf && t.sheet == sheet && text.size() <= 255) {
        QString best;
        bool found = false;
        for (const int scope : {sheet, -1}) {
            const int id = session.names().find(scope, text);
            if (id >= 0 && session.nameListed(id) && namesOf->nameText(scope, text, &best)) {
                found = true;
                break;
            }
        }
        if (found) {
            QString nameSheet, nameRest;
            GoToTarget inner;
            QString ref = best.trimmed();
            if (ref.startsWith(QLatin1Char('='))) ref.remove(0, 1);
            inner.sheet = sheet;
            if (splitSheet(ref, &nameSheet, &nameRest)) {
                inner.sheet = sheetNamed(session, nameSheet);
                ref = nameRest;
            }
            if (inner.sheet >= 0 && parseArea(ref, &inner)) {
                inner.ok = true;
                return inner;
            }
            t.problem = QCoreApplication::translate("jp::crunch::GoTo", "\"%1\" does not stand for a range of cells, so there is nothing to go to.").arg(text);
            return t;
        }
    }
    t.problem = QCoreApplication::translate("jp::crunch::GoTo", "\"%1\" is not a cell, a range, or a name in this workbook.").arg(input.trimmed().left(60));
    return t;
}

QVector<QPair<QString, QString>> goToNames(const EditSession &session, int sheet)
{
    QVector<QPair<QString, QString>> out;
    const Workbook *book = session.workbook();
    if (!book) return out;
    const Calculation *calc = session.calculation();
    if (!calc) return out;
    for (int id = 0; id < session.names().slots(); ++id) {
        const NameEntry &n = session.names().at(id);
        if (!session.nameListed(id) || n.hidden || (n.scope >= 0 && n.scope != sheet)) continue;
        QString ref;
        if (!calc->nameText(n.scope, n.name, &ref)) continue;
        GoToTarget probe;
        QString a, b;
        QString area = splitSheet(ref, &a, &b) ? b : ref;
        if (!parseArea(area, &probe)) continue;
        out.append({n.name, ref});
        if (out.size() >= 5000) break;
    }
    std::sort(out.begin(), out.end(), [](const QPair<QString, QString> &x, const QPair<QString, QString> &y) { return x.first.compare(y.first, Qt::CaseInsensitive) < 0; });
    return out;
}

CellRange currentRegion(const EditSession &session, int sheet, quint32 row, quint32 col, bool *truncated)
{
    if (truncated) *truncated = false;
    CellRange box;
    box.valid = true;
    box.row1 = box.row2 = row;
    box.col1 = box.col2 = col;
    const Calculation *calc = session.calculation();
    if (!calc || sheet < 0 || sheet >= calc->sheetCount()) return box;
    const Sheet &s = calc->sheet(sheet);
    auto filled = [&](const Cell &c) { return c.type != CellType::Empty || c.hasFormula(); };
    CellStore::RangeWalk walk;
    // whether any cell in the band (inclusive) has something
    auto anyIn = [&](qint64 r1, qint64 c1, qint64 r2, qint64 c2) {
        if (r1 < 0) r1 = 0;
        if (c1 < 0) c1 = 0;
        if (r2 >= qint64(kMaxRows)) r2 = kMaxRows - 1;
        if (c2 >= qint64(kMaxColumns)) c2 = kMaxColumns - 1;
        if (r1 > r2 || c1 > c2) return false;
        bool any = false;
        s.cells.forEachInRange(quint32(r1), quint32(c1), quint32(r2), quint32(c2), [&](quint32, quint32, const Cell &c) {
            if (filled(c)) any = true;
            return !any;
        }, &walk);
        return any;
    };
    // Each side is grown as far as it goes before the next is looked at (a side is looked at again if another has grown), and a side past the cells the sheet reaches is not looked at.
    auto over = [&] { return walk.stepped + walk.compared > kMaxSpecialWork; };
    bool grew = true;
    while (grew && !over()) {
        grew = false;
        while (box.row2 + 1 < s.extentRows && !over() && anyIn(qint64(box.row2) + 1, qint64(box.col1) - 1, qint64(box.row2) + 1, qint64(box.col2) + 1)) {
            ++box.row2;
            grew = true;
        }
        while (box.row1 > 0 && !over() && anyIn(qint64(box.row1) - 1, qint64(box.col1) - 1, qint64(box.row1) - 1, qint64(box.col2) + 1)) {
            --box.row1;
            grew = true;
        }
        while (box.col2 + 1 < s.extentCols && !over() && anyIn(qint64(box.row1) - 1, qint64(box.col2) + 1, qint64(box.row2) + 1, qint64(box.col2) + 1)) {
            ++box.col2;
            grew = true;
        }
        while (box.col1 > 0 && !over() && anyIn(qint64(box.row1) - 1, qint64(box.col1) - 1, qint64(box.row2) + 1, qint64(box.col1) - 1)) {
            --box.col1;
            grew = true;
        }
    }
    if (over() && truncated) *truncated = true;
    return box;
}

namespace {

// Collects cells (in order of row, then column) into ranges: the cells of a row make runs, and a run that sits exactly under a run of the same columns joins it.
class RangeBuilder {
public:
    explicit RangeBuilder(SpecialResult *out) : m_out(out) {}
    void add(quint32 row, quint32 col)
    {
        ++m_cells;
        if (m_haveRow && row != m_row) endRow();
        m_haveRow = true;
        m_row = row;
        if (!m_runs.empty() && m_runs.back().second + 1 == col) m_runs.back().second = col;
        else m_runs.push_back({col, col});
    }
    void finish()
    {
        endRow();
        for (const auto &open : m_open) push(open.second);
        m_open.clear();
        std::sort(m_out->ranges.begin(), m_out->ranges.end(), [](const CellRange &a, const CellRange &b) { return a.row1 != b.row1 ? a.row1 < b.row1 : a.col1 < b.col1; });
        m_out->cells = m_cells;
        m_out->truncated = m_out->truncated || m_overflow;
    }

private:
    void push(const CellRange &r)
    {
        if (m_out->ranges.size() >= kMaxSpecialRanges) {
            m_overflow = true;
            return;
        }
        m_out->ranges.append(r);
    }
    void endRow()
    {
        if (!m_haveRow) return;
        for (const auto &run : m_runs) {
            const std::pair<quint32, quint32> key{run.first, run.second};
            auto above = m_open.find(key);
            if (above != m_open.end() && above->second.row2 + 1 == m_row) {
                above->second.row2 = m_row;
                continue;
            }
            if (above != m_open.end()) push(above->second);
            CellRange r;
            r.valid = true;
            r.row1 = r.row2 = m_row;
            r.col1 = run.first;
            r.col2 = run.second;
            m_open[key] = r;
        }
        // (a range the row did not carry on is finished)
        for (auto it = m_open.begin(); it != m_open.end();) {
            if (it->second.row2 != m_row) {
                push(it->second);
                it = m_open.erase(it);
            } else {
                ++it;
            }
        }
        m_runs.clear();
        m_haveRow = false;
    }
    SpecialResult *m_out;
    bool m_haveRow = false;
    quint32 m_row = 0;
    std::vector<std::pair<quint32, quint32>> m_runs;
    std::map<std::pair<quint32, quint32>, CellRange> m_open;
    qint64 m_cells = 0;
    bool m_overflow = false;
};

bool resultKindMatches(const Value &v, const SpecialOptions &o)
{
    switch (v.kind()) {
    case Value::Kind::Number: return o.numbers;
    case Value::Kind::Text: return o.text;
    case Value::Kind::Bool: return o.logicals;
    case Value::Kind::Error: return o.errors;
    case Value::Kind::Empty: return o.text;   // (a formula with no text: its result is the empty text)
    case Value::Kind::Array: return false;
    }
    return false;
}

} // namespace

SpecialResult goToSpecial(const EditSession &session, int sheet, const CellRange &selection, quint32 activeRow, quint32 activeCol, const SpecialOptions &options)
{
    SpecialResult out;
    const Calculation *calc = session.calculation();
    if (!calc || sheet < 0 || sheet >= calc->sheetCount()) {
        out.problem = QCoreApplication::translate("jp::crunch::GoTo", "There are no cells here.");
        return out;
    }
    const Sheet &s = calc->sheet(sheet);
    const bool oneCell = !selection.valid || (selection.row1 == selection.row2 && selection.col1 == selection.col2);
    // the cells looked in: the selection, or the cells the sheet uses
    CellRange region;
    region.valid = true;
    region.row1 = region.col1 = 0;
    region.row2 = s.extentRows > 0 ? s.extentRows - 1 : 0;
    region.col2 = s.extentCols > 0 ? s.extentCols - 1 : 0;
    if (!oneCell) {
        region = selection;
        if (s.extentRows > 0 && options.kind != SpecialKind::Blanks && options.kind != SpecialKind::VisibleCells) region.row2 = std::min(region.row2, s.extentRows - 1);
    }
    auto single = [&](quint32 r, quint32 c) {
        CellRange one;
        one.valid = true;
        one.row1 = one.row2 = r;
        one.col1 = one.col2 = c;
        out.ranges.append(one);
        out.cells = 1;
    };
    auto none = [&] { out.problem = QCoreApplication::translate("jp::crunch::GoTo", "No cell matches."); };
    switch (options.kind) {
    case SpecialKind::LastCell:
        if (s.extentRows == 0) none();
        else single(s.extentRows - 1, s.extentCols - 1);
        return out;
    case SpecialKind::CurrentRegion: {
        const CellRange r = currentRegion(session, sheet, activeRow, activeCol, &out.truncated);
        out.ranges.append(r);
        out.cells = r.cellCount();
        return out;
    }
    case SpecialKind::ConditionalFormats:
    case SpecialKind::DataValidation: {
        const Validation *same = nullptr;
        if (options.kind == SpecialKind::DataValidation && options.sameRule) {
            same = calc->validationAt(sheet, activeRow, activeCol);
            if (!same) {
                out.problem = QCoreApplication::translate("jp::crunch::GoTo", "The cell with the cursor has no rule for what may be entered, so there are no cells that have the same rule.");
                return out;
            }
        }
        auto take = [&](const QVector<CellRange> &ranges) {
            for (const CellRange &r : ranges) {
                CellRange c = r;
                if (!oneCell) {
                    c.row1 = std::max(r.row1, selection.row1);
                    c.row2 = std::min(r.row2, selection.row2);
                    c.col1 = std::max(r.col1, selection.col1);
                    c.col2 = std::min(r.col2, selection.col2);
                    if (c.row1 > c.row2 || c.col1 > c.col2) continue;
                }
                if (out.ranges.size() >= kMaxSpecialRanges) {
                    out.truncated = true;
                    return;
                }
                out.ranges.append(c);
                out.cells += c.cellCount();
            }
        };
        if (options.kind == SpecialKind::ConditionalFormats) {
            for (const CondRule &rule : s.conditionalFormats) take(rule.ranges);
        } else {
            for (const Validation &rule : s.validations) {
                if (same && !(rule.type == same->type && rule.formula1 == same->formula1 && rule.formula2 == same->formula2 && rule.compare == same->compare)) continue;
                take(rule.ranges);
            }
        }
        if (out.ranges.isEmpty()) none();
        std::sort(out.ranges.begin(), out.ranges.end(), [](const CellRange &a, const CellRange &b) { return a.row1 != b.row1 ? a.row1 < b.row1 : a.col1 < b.col1; });
        return out;
    }
    case SpecialKind::VisibleCells: {
        // the rows and columns that show, in runs, then the blocks they make
        const quint32 r1 = region.row1, r2 = region.row2, c1 = region.col1, c2 = region.col2;
        QVector<QPair<quint32, quint32>> rowRuns, colRuns;
        quint32 start = r1;
        for (const RowSpec &spec : calc->rowSpecsIn(sheet, r1, r2)) {
            if (!spec.hidden) continue;
            if (spec.row > start) rowRuns.append({start, spec.row - 1});
            start = spec.row + 1;
        }
        if (start <= r2) rowRuns.append({start, r2});
        start = c1;
        for (const ColumnSpan &span : calc->columnRuns(sheet, c1, c2)) {
            if (!span.hidden) continue;
            const quint32 first = std::max(span.first, c1), last = std::min(span.last, c2);
            if (first > start) colRuns.append({start, first - 1});
            start = last + 1;
        }
        if (start <= c2) colRuns.append({start, c2});
        if (qint64(rowRuns.size()) * colRuns.size() > kMaxSpecialRanges) {
            out.truncated = true;
        }
        for (const auto &rr : rowRuns)
            for (const auto &cr : colRuns) {
                if (out.ranges.size() >= kMaxSpecialRanges) break;
                CellRange r;
                r.valid = true;
                r.row1 = rr.first;
                r.row2 = rr.second;
                r.col1 = cr.first;
                r.col2 = cr.second;
                out.ranges.append(r);
                out.cells += r.cellCount();
            }
        if (out.ranges.isEmpty()) none();
        return out;
    }
    case SpecialKind::Blanks:
    case SpecialKind::Constants:
    case SpecialKind::Formulas:
    case SpecialKind::Errors: break;
    }
    RangeBuilder builder(&out);
    qint64 work = 0;
    if (options.kind == SpecialKind::Blanks) {
        // the cells in the region that hold nothing: per row, the gaps between the cells that hold something
        quint32 row = region.row1;
        for (; row <= region.row2; ++row) {
            if (++work > kMaxSpecialWork) {
                out.truncated = true;
                break;
            }
            quint32 next = region.col1;
            s.cells.forEachInRange(row, region.col1, row, region.col2, [&](quint32, quint32 cellCol, const Cell &c) {
                ++work;
                if (c.type == CellType::Empty && !c.hasFormula()) return true;   // (a cell that is there and holds nothing is blank: it falls in the gap)
                for (quint32 col = next; col < cellCol; ++col) builder.add(row, col);
                next = cellCol + 1;
                return true;
            });
            if (work > kMaxSpecialWork) {
                out.truncated = true;
                break;
            }
            for (quint32 col = next; col <= region.col2; ++col) {
                builder.add(row, col);
                if (++work > kMaxSpecialWork) break;
            }
        }
        builder.finish();
        if (out.ranges.isEmpty()) none();
        return out;
    }
    s.cells.forEachInRange(region.row1, region.col1, region.row2, region.col2, [&](quint32 cellRow, quint32 cellCol, const Cell &c) {
        if (++work > kMaxSpecialWork) {
            out.truncated = true;
            return false;
        }
        const bool hasFormula = c.hasFormula() || (c.flags & ArrayMember);
        bool take = false;
        if (options.kind == SpecialKind::Constants) {
            if (!hasFormula && c.type != CellType::Empty) {
                const Value v = session.valueAt(sheet, cellRow, cellCol);
                take = resultKindMatches(v, options);
            }
        } else if (options.kind == SpecialKind::Formulas) {
            if (hasFormula) take = resultKindMatches(session.valueAt(sheet, cellRow, cellCol), options);
        } else if (options.kind == SpecialKind::Errors) {
            take = session.valueAt(sheet, cellRow, cellCol).isError();
        }
        if (take) builder.add(cellRow, cellCol);
        return true;
    });
    builder.finish();
    if (out.ranges.isEmpty()) none();
    return out;
}

} // namespace jp::crunch
