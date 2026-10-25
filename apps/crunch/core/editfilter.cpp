// Filter: the conditions of a sheet's filter (the values a column lets through, one or two comparisons, the top or bottom, the dates of a period, above or below the average, or a color)
// worked out for the rows under its buttons a slice at a time, and put in as one step with the rows they hide. The rows a filter hides are kept apart from those a person hid.
#include "core/editsession.h"

#include "calc/dates.h"
#include "calc/numfmt.h"
#include "sheet/cellref.h"
#include "sheet/textmatch.h"

#include <QCoreApplication>
#include <QDate>
#include <QSet>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

bool sameSpec(const RowSpec &a, const RowSpec &b)
{
    return a.row == b.row && a.height == b.height && a.hidden == b.hidden && a.customHeight == b.customHeight && a.customFormat == b.customFormat && a.style == b.style && a.outline == b.outline
           && a.collapsed == b.collapsed && a.hand == b.hand && a.filtered == b.filtered;
}

// The number a condition's value is, if it is one (written the way the file writes numbers).
bool numberOf(const QString &text, double *out)
{
    bool ok = false;
    const double v = text.trimmed().toDouble(&ok);
    if (!ok || !std::isfinite(v)) return false;
    *out = v;
    return true;
}

} // namespace

struct EditSession::FilterPlan {
    int sheet = 0;
    AutoFilter before, after;
    quint32 firstRow = 0, lastRow = 0;       // the rows under the buttons
    // one per column that has a condition
    struct Column {
        FilterColumn fc;
        quint32 col = 0;                     // the column in the sheet
        QSet<QString> values;                // Values: what shows in the cells let through (folded)
        std::vector<TextMatcher> matchers;   // Custom: each condition's text, as a pattern for the whole cell
        std::vector<double> numbers;         // Custom: the number of each condition, when it has one (else the matcher is used)
        std::vector<bool> isNumber;
        double lo = 0, hi = 0;               // Dynamic: the serial numbers that are let through (lo to before hi), or a threshold
        bool useRange = false, threshold = false, above = true, atLeast = false;
        ColorSpec wanted;
        bool needNumbers = false;
        std::vector<double> collected;       // the numbers of the column, for the top and the average
    };
    std::vector<Column> columns;
    std::vector<quint8> hide;                // for each row from firstRow: 1 if the conditions hide it
    quint32 next = 0;                        // the next row to look at
    int phase = 0;                           // 0: the numbers some conditions need are collected; 1: the rows are worked out
    size_t collecting = 0;
    bool done = false;
    qint64 changesAtPlan = 0;
    int undoIndexAtPlan = 0;
    double today = 0;
    int today_y = 0, today_m = 0, today_d = 0;
    DateSystem system = DateSystem::Date1900;
};

ColorSpec EditSession::colorOfCondition(const FilterColumn &c) const
{
    if (c.color.isSet()) return c.color;
    if (!m_book || !m_book->book || c.dxf < 0) return ColorSpec();
    const QVector<DifferentialFormat> &dxfs = m_book->book->styles().dxfs;
    if (c.dxf >= dxfs.size()) return ColorSpec();
    const DifferentialFormat &d = dxfs[c.dxf];
    if (c.fontColor) return d.fontColor;
    return d.fill.bg.isSet() ? d.fill.bg : d.fill.fg;
}

bool EditSession::rowFilteredOut(int sheet, quint32 row) const
{
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount()) return false;
    for (const RowSpec &r : m_book->calc->rowSpecsIn(sheet, row, row))
        if (r.row == row) return r.filtered;
    return false;
}

AutoFilter EditSession::filterWith(int sheet, quint32 column, const FilterColumn &condition) const
{
    AutoFilter f = m_book->calc->autoFilterOf(sheet);
    FilterColumn c = condition;
    c.column = column;
    for (int i = 0; i < f.columns.size(); ++i)
        if (f.columns[i].column == column) {
            c.hideButton = f.columns[i].hideButton;
            if (c.kind == FilterColumn::Kind::None && !c.hideButton) f.columns.remove(i);
            else f.columns[i] = c;
            return f;
        }
    if (c.kind != FilterColumn::Kind::None) f.columns.append(c);
    return f;
}

AutoFilter EditSession::filterCleared(int sheet) const
{
    AutoFilter f = m_book->calc->autoFilterOf(sheet);
    QVector<FilterColumn> kept;
    for (FilterColumn c : f.columns) {
        if (!c.hideButton) continue;
        c = FilterColumn{c};
        c.kind = FilterColumn::Kind::None;
        c.values.clear();
        c.dates.clear();
        c.conditions.clear();
        c.dxf = -1;
        c.color = ColorSpec();
        kept.append(c);
    }
    f.columns = kept;
    return f;
}

std::shared_ptr<EditSession::FilterPlan> EditSession::planFilter(int sheet, const AutoFilter &after, QString *why)
{
    auto refuse = [&](const QString &problem) {
        if (why) *why = problem;
        return std::shared_ptr<FilterPlan>();
    };
    QString reason;
    if (!canEdit(&reason) || !canEditSheet(sheet, &reason)) return refuse(reason);
    auto plan = std::make_shared<FilterPlan>();
    plan->sheet = sheet;
    plan->after = after;
    plan->before = m_book->calc->autoFilterOf(sheet);
    plan->system = m_book->book->info().dateSystem;
    // the rows to look at are those of the filter now and those it had (rows it hid are shown again if the range moves)
    const CellRange now = after.ref, was = plan->before.ref;
    quint32 first = ~quint32(0), last = 0;
    for (const CellRange &r : {now, was})
        if (r.valid) {
            first = std::min(first, r.row1 + 1);
            last = std::max(last, r.row2);
        }
    if (first == ~quint32(0)) {
        plan->done = true;
        return plan;
    }
    if (first > last) {
        plan->firstRow = first;
        plan->lastRow = first - 1;
        plan->done = true;
        return plan;
    }
    if (qint64(last) - first + 1 > kMaxFilterRows)
        return refuse(tr("The filter covers more rows than JeffCrunch filters at once (%1). Filter a smaller block of cells.").arg(QLocale().toString(kMaxFilterRows)));
    plan->firstRow = first;
    plan->lastRow = last;
    plan->hide.assign(size_t(last - first) + 1, 0);
    plan->next = first;
    if (after.ref.valid) {
        QDate d = QDate::currentDate();
        if (m_today.isValid()) d = m_today;
        double serial = 0;
        jp::calc::dateToSerial(d.year(), d.month(), d.day(), plan->system, &serial);
        plan->today = serial;
        plan->today_y = d.year();
        plan->today_m = d.month();
        plan->today_d = d.day();
        for (const FilterColumn &fc : after.columns) {
            if (fc.kind == FilterColumn::Kind::None) continue;
            if (fc.column > after.ref.col2 - after.ref.col1) return refuse(tr("A condition of the filter is for a column that is not in its range."));
            FilterPlan::Column c;
            c.fc = fc;
            c.col = after.ref.col1 + fc.column;
            switch (fc.kind) {
            case FilterColumn::Kind::Values:
                for (const QString &v : fc.values) c.values.insert(v.toCaseFolded());
                break;
            case FilterColumn::Kind::Custom:
                for (const FilterColumn::Condition &cond : fc.conditions) {
                    TextPattern p;
                    p.text = cond.value;
                    p.wholeCell = true;
                    c.matchers.emplace_back(p);
                    double n = 0;
                    c.numbers.push_back(0);
                    c.isNumber.push_back(numberOf(cond.value, &n));
                    if (c.isNumber.back()) c.numbers.back() = n;
                }
                if (fc.conditions.isEmpty()) return refuse(tr("A custom condition needs at least one comparison."));
                break;
            case FilterColumn::Kind::Top10:
                c.needNumbers = true;
                break;
            case FilterColumn::Kind::Dynamic: {
                const QString t = fc.dynamic;
                if (t == QLatin1String("aboveAverage") || t == QLatin1String("belowAverage")) {
                    c.needNumbers = true;
                    c.above = t == QLatin1String("aboveAverage");
                } else {
                    c.useRange = true;
                }
                break;
            }
            case FilterColumn::Kind::Color:
                c.wanted = colorOfCondition(fc);
                break;
            case FilterColumn::Kind::None: break;
            }
            plan->columns.push_back(std::move(c));
        }
        // the periods that are asked for, as serial numbers of days
        auto serialOf = [&](const QDate &d) {
            double s = 0;
            jp::calc::dateToSerial(d.year(), d.month(), d.day(), plan->system, &s);
            return s;
        };
        const QDate today = QDate(plan->today_y, plan->today_m, plan->today_d);
        for (FilterPlan::Column &c : plan->columns) {
            if (!c.useRange) continue;
            const QString t = c.fc.dynamic;
            QDate from, to;   // from the first day to the day after the last
            auto weekStart = [&](const QDate &d) { return d.addDays(-(d.dayOfWeek() % 7)); };   // (weeks begin on Sunday)
            auto monthStart = [&](int offset) { return QDate(today.year(), today.month(), 1).addMonths(offset); };
            auto quarterStart = [&](int offset) { return QDate(today.year(), ((today.month() - 1) / 3) * 3 + 1, 1).addMonths(offset * 3); };
            auto yearStart = [&](int offset) { return QDate(today.year() + offset, 1, 1); };
            if (t == QLatin1String("today")) { from = today; to = today.addDays(1); }
            else if (t == QLatin1String("yesterday")) { from = today.addDays(-1); to = today; }
            else if (t == QLatin1String("tomorrow")) { from = today.addDays(1); to = today.addDays(2); }
            else if (t == QLatin1String("thisWeek")) { from = weekStart(today); to = from.addDays(7); }
            else if (t == QLatin1String("lastWeek")) { from = weekStart(today).addDays(-7); to = from.addDays(7); }
            else if (t == QLatin1String("nextWeek")) { from = weekStart(today).addDays(7); to = from.addDays(7); }
            else if (t == QLatin1String("thisMonth")) { from = monthStart(0); to = from.addMonths(1); }
            else if (t == QLatin1String("lastMonth")) { from = monthStart(-1); to = from.addMonths(1); }
            else if (t == QLatin1String("nextMonth")) { from = monthStart(1); to = from.addMonths(1); }
            else if (t == QLatin1String("thisQuarter")) { from = quarterStart(0); to = from.addMonths(3); }
            else if (t == QLatin1String("lastQuarter")) { from = quarterStart(-1); to = from.addMonths(3); }
            else if (t == QLatin1String("nextQuarter")) { from = quarterStart(1); to = from.addMonths(3); }
            else if (t == QLatin1String("thisYear")) { from = yearStart(0); to = yearStart(1); }
            else if (t == QLatin1String("lastYear")) { from = yearStart(-1); to = yearStart(0); }
            else if (t == QLatin1String("nextYear")) { from = yearStart(1); to = yearStart(2); }
            else if (t == QLatin1String("yearToDate")) { from = yearStart(0); to = today.addDays(1); }
            else if ((t.size() == 2 && t.at(0) == QLatin1Char('Q') && t.at(1) >= QLatin1Char('1') && t.at(1) <= QLatin1Char('4')) || (t.startsWith(QLatin1Char('M')) && t.size() <= 3)) {
                c.useRange = false;   // (a quarter or a month of any year: told by the month of each date)
            } else {
                return refuse(tr("The filter has a kind of condition that JeffCrunch does not know (%1).").arg(t.left(30)));
            }
            if (from.isValid()) {
                c.lo = serialOf(from);
                c.hi = serialOf(to);
            }
        }
    }
    plan->changesAtPlan = m_loader->changesMade();
    plan->undoIndexAtPlan = m_undo.index();
    // no condition: nothing hides a row
    plan->phase = 1;
    for (size_t i = 0; i < plan->columns.size(); ++i)
        if (plan->columns[i].needNumbers) {
            plan->phase = 0;
            break;
        }
    return plan;
}

namespace {

int monthOfSerial(double serial, DateSystem system)
{
    int y = 0, m = 0, d = 0;
    return jp::calc::serialToDate(std::floor(serial), system, &y, &m, &d) ? m : 0;
}

} // namespace

bool EditSession::prepareFilter(FilterPlan &plan, qint64 budget)
{
    if (plan.done) return true;
    const Calculation *calc = m_book->calc.get();
    const Sheet &s = calc->sheet(plan.sheet);
    qint64 spent = 0;
    // phase 0: the numbers of the columns whose condition is about all the values (the top, the average)
    while (plan.phase == 0) {
        while (plan.collecting < plan.columns.size() && !plan.columns[plan.collecting].needNumbers) ++plan.collecting;
        if (plan.collecting >= plan.columns.size()) {
            plan.phase = 1;
            plan.next = plan.firstRow;
            break;
        }
        FilterPlan::Column &c = plan.columns[plan.collecting];
        for (; plan.next <= plan.lastRow; ++plan.next) {
            if (budget > 0 && spent >= budget) return false;
            ++spent;
            const Cell *cell = s.cells.find(plan.next, c.col);
            if (!cell) continue;
            const Value v = valueAt(plan.sheet, plan.next, c.col);
            if (v.isNumber()) c.collected.push_back(v.number());
            if (plan.next == 0xFFFFFFFFu) break;
        }
        // (this column's numbers are all in: its threshold)
        std::vector<double> &n = c.collected;
        if (c.fc.kind == FilterColumn::Kind::Dynamic) {
            double sum = 0;
            for (double v : n) sum += v;
            c.lo = n.empty() ? 0 : sum / double(n.size());
            c.threshold = true;
        } else if (c.fc.kind == FilterColumn::Kind::Top10) {
            std::sort(n.begin(), n.end());
            qint64 take = c.fc.percent ? qint64(std::floor(double(n.size()) * c.fc.count / 100.0)) : qint64(std::floor(c.fc.count));
            take = std::clamp<qint64>(take, n.empty() ? 0 : 1, qint64(n.size()));
            if (n.empty()) {
                c.threshold = true;
                c.atLeast = true;
                c.lo = std::numeric_limits<double>::infinity();
            } else {
                c.threshold = true;
                c.lo = c.fc.top ? n[n.size() - size_t(take)] : n[size_t(take) - 1];
            }
        }
        n.clear();
        n.shrink_to_fit();
        ++plan.collecting;
        plan.next = plan.firstRow;
    }
    // phase 1: the rows
    const qint64 rowsLeft = qint64(plan.lastRow) - plan.next + 1;
    (void)rowsLeft;
    for (; plan.next <= plan.lastRow && plan.after.ref.valid; ++plan.next) {
        if (budget > 0 && spent >= budget) return false;
        const quint32 row = plan.next;
        bool hidden = false;
        for (const FilterPlan::Column &c : plan.columns) {
            ++spent;
            const Cell *cell = s.cells.find(row, c.col);
            const Value v = cell ? valueAt(plan.sheet, row, c.col) : Value();
            bool pass = true;
            switch (c.fc.kind) {
            case FilterColumn::Kind::Values: {
                if (v.isEmpty()) {
                    pass = c.fc.blank;
                    break;
                }
                const QString shownText = displayText(plan.sheet, row, c.col);
                pass = c.values.contains(shownText.toCaseFolded());
                if (!pass && v.isNumber() && !c.fc.dates.isEmpty()) {
                    int y = 0, m = 0, d = 0;
                    if (jp::calc::serialToDate(std::floor(v.number()), plan.system, &y, &m, &d)) {
                        const double frac = v.number() - std::floor(v.number());
                        const int secs = int(std::lround(frac * 86400.0)) % 86400;
                        const int hh = secs / 3600, mm = (secs / 60) % 60, ss = secs % 60;
                        for (const FilterColumn::DateGroup &g : c.fc.dates) {
                            using U = FilterColumn::DateGroup::Unit;
                            if (g.year != y) continue;
                            if (g.unit == U::Year) { pass = true; break; }
                            if (g.month != m) continue;
                            if (g.unit == U::Month) { pass = true; break; }
                            if (g.day != d) continue;
                            if (g.unit == U::Day) { pass = true; break; }
                            if (g.hour != hh) continue;
                            if (g.unit == U::Hour) { pass = true; break; }
                            if (g.minute != mm) continue;
                            if (g.unit == U::Minute) { pass = true; break; }
                            if (g.second == ss) { pass = true; break; }
                        }
                    }
                }
                break;
            }
            case FilterColumn::Kind::Custom: {
                auto one = [&](size_t i) {
                    const FilterColumn::Condition &cond = c.fc.conditions[int(i)];
                    using O = FilterColumn::Op;
                    if (v.isNumber() && c.isNumber[i]) {
                        const double x = v.number(), y = c.numbers[i];
                        switch (cond.op) {
                        case O::Equal: return x == y;
                        case O::NotEqual: return x != y;
                        case O::Greater: return x > y;
                        case O::GreaterEqual: return x >= y;
                        case O::Less: return x < y;
                        case O::LessEqual: return x <= y;
                        }
                    }
                    // text: equal and not equal by the whole text and its wildcards; the order by the letters
                    const QString t = v.isEmpty() ? QString() : displayText(plan.sheet, row, c.col);
                    if (cond.op == O::Equal) return !t.isEmpty() ? c.matchers[i].matches(t) : cond.value.isEmpty();
                    if (cond.op == O::NotEqual) return !t.isEmpty() ? !c.matchers[i].matches(t) : !cond.value.isEmpty();
                    if (v.isEmpty()) return false;
                    if (v.isNumber() != c.isNumber[i]) return cond.op == O::Greater || cond.op == O::GreaterEqual ? v.isText() : !v.isText();   // (text is above numbers)
                    const int cmp = QString::compare(t, cond.value, Qt::CaseInsensitive);
                    switch (cond.op) {
                    case O::Greater: return cmp > 0;
                    case O::GreaterEqual: return cmp >= 0;
                    case O::Less: return cmp < 0;
                    case O::LessEqual: return cmp <= 0;
                    default: return false;
                    }
                };
                bool result = one(0);
                if (c.fc.conditions.size() > 1) result = c.fc.both ? (result && one(1)) : (result || one(1));
                pass = result;
                break;
            }
            case FilterColumn::Kind::Top10:
                pass = v.isNumber() && c.threshold && (c.fc.top ? v.number() >= c.lo : v.number() <= c.lo);
                break;
            case FilterColumn::Kind::Dynamic: {
                if (!v.isNumber()) {
                    pass = false;
                    break;
                }
                if (c.threshold) {
                    pass = c.above ? v.number() > c.lo : v.number() < c.lo;
                } else if (c.useRange) {
                    pass = v.number() >= c.lo && v.number() < c.hi;
                } else {
                    const QString t = c.fc.dynamic;
                    const int month = monthOfSerial(v.number(), plan.system);
                    if (t.startsWith(QLatin1Char('Q'))) pass = month > 0 && (month - 1) / 3 + 1 == t.mid(1).toInt();
                    else pass = month > 0 && month == t.mid(1).toInt();
                }
                break;
            }
            case FilterColumn::Kind::Color: {
                ColorSpec color;
                if (cell) {
                    const CellFormat &format = m_book->book->styles().format(cell->style);
                    if (c.fc.fontColor) {
                        color = m_book->book->styles().font(format.font).color;
                    } else {
                        const FillSpec &fill = m_book->book->styles().fill(format.fill);
                        if (fill.pattern != PatternKind::None) color = fill.fg;
                    }
                    if (color.kind == ColorSpec::Kind::Auto) color = ColorSpec();
                }
                pass = color == c.wanted;
                break;
            }
            case FilterColumn::Kind::None: break;
            }
            if (!pass) {
                hidden = true;
                break;
            }
        }
        plan.hide[row - plan.firstRow] = hidden ? 1 : 0;
        if (row == 0xFFFFFFFFu) break;
    }
    plan.done = true;
    return true;
}

EditSession::Outcome EditSession::applyFilter(FilterPlan &plan, const QString &text)
{
    Outcome out;
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    QString why;
    if (!canEdit(&why) || !canEditSheet(plan.sheet, &why)) return refuse(why);
    if (!plan.done) return refuse(tr("The filter could not be finished. Try it again."));
    if (m_loader->changesMade() != plan.changesAtPlan || m_undo.index() != plan.undoIndexAtPlan)
        return refuse(tr("The cells were changed while the filter was being worked out, so it was not applied. Filter again."));
    Step step;
    if (!(plan.after == plan.before) || !(plan.after.ref == plan.before.ref)) {
        FilterEdit e;
        e.sheet = plan.sheet;
        e.before = plan.before;
        e.after = plan.after;
        step.filters.push_back(e);
    }
    if (plan.lastRow >= plan.firstRow) {
        SizeEdit e;
        e.sheet = plan.sheet;
        e.columns = false;
        e.first = plan.firstRow;
        e.last = plan.lastRow;
        e.rowsBefore = m_book->calc->rowSpecsIn(plan.sheet, plan.firstRow, plan.lastRow);
        size_t at = 0;
        bool anyChange = false;
        const AutoFilter &now = plan.after;
        for (quint32 row = plan.firstRow; row <= plan.lastRow; ++row) {
            const bool has = at < e.rowsBefore.size() && e.rowsBefore[at].row == row;
            RowSpec before;
            if (has) before = e.rowsBefore[at++];
            before.row = row;
            // (a row that is hidden and is not the filter's was hidden by hand, whatever read the sheet)
            before.hand = before.hand || (before.hidden && !before.filtered);
            RowSpec r = before;
            // (rows under the buttons that the conditions hide; any other row is not the filter's)
            const bool inFilter = now.ref.valid && row > now.ref.row1 && row <= now.ref.row2;
            r.filtered = inFilter && plan.hide[row - plan.firstRow] != 0;
            r.hidden = r.hand || r.filtered;
            if (!sameSpec(before, r)) anyChange = true;
            if (has || r.height >= 0 || r.hidden || r.customFormat) e.rowsAfter.push_back(r);
            if (row == 0xFFFFFFFFu) break;
        }
        if (anyChange) step.sizes.push_back(std::move(e));
    }
    if (step.empty()) return out;
    pushStep(std::move(step), text);
    return out;
}

EditSession::Outcome EditSession::setFilter(int sheet, const AutoFilter &after, const QString &text)
{
    Outcome out;
    QString why;
    const std::shared_ptr<FilterPlan> plan = planFilter(sheet, after, &why);
    if (!plan) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    prepareFilter(*plan, 0);
    return applyFilter(*plan, text);
}

} // namespace jp::crunch
