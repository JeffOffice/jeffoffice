// Sort: the lines of a range (its rows, or its columns for a sort from left to right) put in the order their keys come to, as one step. The keys are read from the workbook a slice at a time,
// the order is worked out from the keys alone (so that it can be done on another thread, and given up), and the cells are put in the step on the window's thread, each line's formulas
// written for the place they go to, as if the line were copied there.
#include "core/editsession.h"

#include "sheet/cellref.h"
#include "sheet/refshift.h"

#include <QCoreApplication>
#include "core/collation.h"

#include <QCollator>
#include <QLocale>

#include <algorithm>
#include <numeric>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

// The most characters of a text that a key holds, and of all the keys' texts together.
constexpr int kMaxKeyChars = 256;
constexpr qint64 kMaxSortKeyChars = qint64(16) << 20;

QVector<QStringList> builtInSortLists()
{
    QVector<QStringList> lists;
    const QLocale english(QLocale::English, QLocale::UnitedStates);
    auto make = [](const QLocale &locale, QLocale::FormatType type, bool days) {
        QStringList names;
        if (days) {
            // (starting from Sunday)
            for (int d = 7, n = 0; n < 7; ++n, d = d % 7 + 1) names << locale.dayName(d, type);
        } else {
            for (int m = 1; m <= 12; ++m) names << locale.monthName(m, type);
        }
        return names;
    };
    for (const QLocale &locale : {english, QLocale()}) {
        for (bool days : {true, false})
            for (QLocale::FormatType type : {QLocale::LongFormat, QLocale::ShortFormat}) {
                const QStringList list = make(locale, type, days);
                if (!lists.contains(list)) lists.append(list);
            }
    }
    return lists;
}

struct EditSession::SortPlan {
    int sheet = 0;
    CellRange range, data;
    SortSpec spec;
    bool columns = false;           // the lines are columns
    std::vector<quint32> units;     // the lines sorted (rows, or columns), in order of place
    struct Key {
        quint8 cls = 4;             // 0 a number, 1 text, 2 true or false, 3 an error, 4 nothing
        bool match = false;         // a sort by color: the cell has the color
        qint32 listIndex = 0;       // a sort by a list: the place in it (the list's length for a value that is not in it)
        double number = 0;
        QString text;
    };
    std::vector<Key> keys;          // units.size() * levels, the keys of a line together
    size_t prepared = 0;            // lines whose keys are read
    qint64 keyChars = 0;
    std::vector<quint32> order;     // for each place, the line (its place in `units`) that goes there
    bool ordered = false;
    qint64 changesAtPlan = 0;
    int undoIndexAtPlan = 0;
    std::vector<QStringList> lists; // each level's list, folded to lower case
    // the step, made a slice at a time (see applySort)
    int stage = 0;                  // 0: the cells are read, 1: the changes are made, 2: the merged cells and the move of notes, 3: the step is put in
    size_t readAt = 0, buildAt = 0;
    std::vector<Calculation::CellContent> contents;
    Step step;
    bool pushing = false;
    std::shared_ptr<SlicedPush> push;
};

namespace {

QString folded(const QString &text) { return text.trimmed().toCaseFolded(); }

} // namespace

std::shared_ptr<EditSession::SortPlan> EditSession::planSort(int sheet, const CellRange &range, const SortSpec &spec, QString *why)
{
    auto refuse = [&](const QString &problem) {
        if (why) *why = problem;
        return std::shared_ptr<SortPlan>();
    };
    QString reason;
    if (!canEdit(&reason) || !canEditSheet(sheet, &reason)) return refuse(reason);
    if (!range.valid) return refuse(tr("Select the cells to sort."));
    if (spec.levels.isEmpty()) return refuse(tr("Choose what to sort by."));
    if (spec.levels.size() > kMaxSortLevels) return refuse(tr("A sort can have up to %1 levels.").arg(kMaxSortLevels));
    if (range.cellCount() > kMaxCellsPerEdit)
        return refuse(tr("That is more cells than JeffCrunch sorts at once (%1). Select fewer.").arg(QLocale().toString(kMaxCellsPerEdit)));
    auto plan = std::make_shared<SortPlan>();
    plan->sheet = sheet;
    plan->range = range;
    plan->spec = spec;
    plan->columns = spec.leftToRight;
    plan->data = range;
    const quint32 width = plan->columns ? range.row2 - range.row1 + 1 : range.col2 - range.col1 + 1;
    for (const SortLevel &level : spec.levels)
        if (level.key < 0 || quint32(level.key) >= width) return refuse(tr("The sort is by %1 that is not in the cells selected.").arg(plan->columns ? tr("a row") : tr("a column")));
    if (spec.hasHeader) {
        if (plan->columns) ++plan->data.col1;
        else ++plan->data.row1;
    }
    const quint32 firstUnit = plan->columns ? plan->data.col1 : plan->data.row1, lastUnit = plan->columns ? plan->data.col2 : plan->data.row2;
    if (plan->data.row1 > plan->data.row2 || plan->data.col1 > plan->data.col2 || firstUnit >= lastUnit)
        return refuse(plan->columns ? tr("There are no columns to put in order: the selection has one column, or only a header.") : tr("There are no rows to put in order: the selection has one row, or only a header."));
    const Calculation *calc = m_book->calc.get();
    const Sheet &s = calc->sheet(sheet);
    // the cells that cannot be moved
    QString bad;
    s.cells.forEachInRange(plan->data.row1, plan->data.col1, plan->data.row2, plan->data.col2, [&](quint32 cellRow, quint32 cellCol, const Cell &c) {
        if (c.flags & (ArrayFormula | ArrayMember | DataTable)) {
            bad = tr("%1 is part of an array or a data table, so the cells can't be moved. Nothing was sorted.").arg(cellName(cellRow, cellCol));
            return false;
        }
        if (s.isProtected && m_book->book->styles().format(c.style).locked) {
            bad = tr("%1 is locked, and its sheet is protected, so the cells can't be moved. Nothing was sorted.").arg(cellName(cellRow, cellCol));
            return false;
        }
        return true;
    });
    if (bad.isEmpty() && s.isProtected) {
        // (a cell that is not there is locked unless its row or column says it is not)
        for (quint32 row = plan->data.row1; row <= plan->data.row2 && bad.isEmpty(); ++row)
            for (quint32 col = plan->data.col1; col <= plan->data.col2; ++col)
                if (!s.cells.find(row, col) && isLocked(sheet, implicitStyle(sheet, row, col))) {
                    bad = tr("%1 is locked, and its sheet is protected, so the cells can't be moved. Nothing was sorted.").arg(cellName(row, col));
                    break;
                }
    }
    if (!bad.isEmpty()) return refuse(bad);
    if (QString notAllowed; !allows(sheet, Allow::Sort, &notAllowed)) return refuse(notAllowed);   // (a protected sheet that does not allow sorting)
    for (quint32 row = plan->data.row1; row <= plan->data.row2; ++row)
        for (quint32 col = plan->data.col1; col <= plan->data.col2; ++col)
            if (m_book->book->tableHeaderAt(sheet, row, col, nullptr) >= 0)
                return refuse(tr("%1 is in the first row of a table, which holds the names of its columns, so it can't be moved. Start the sort below it.").arg(cellName(row, col)));
    // merged cells: all the same size, each inside one line, and none cut by the edge
    qint64 mergeRows = -1, mergeCols = -1;
    for (const CellRange &m : calc->mergesOf(sheet)) {
        if (m.row2 < plan->data.row1 || m.row1 > plan->data.row2 || m.col2 < plan->data.col1 || m.col1 > plan->data.col2) continue;
        if (m.row1 < plan->data.row1 || m.row2 > plan->data.row2 || m.col1 < plan->data.col1 || m.col2 > plan->data.col2)
            return refuse(tr("Some merged cells reach past the edge of the cells to sort, so they can't be moved. Select cells that include all of them, or none."));
        const qint64 mr = qint64(m.row2) - m.row1 + 1, mc = qint64(m.col2) - m.col1 + 1;
        if (mergeRows >= 0 && (mr != mergeRows || mc != mergeCols))
            return refuse(tr("The merged cells are not all the same size, so they can't be put in order. Unmerge them, or sort cells that have none."));
        mergeRows = mr;
        mergeCols = mc;
        if ((plan->columns ? mc : mr) != 1)
            return refuse(plan->columns ? tr("Some merged cells span more than one column, so the columns can't be put in order. Unmerge them first.")
                                        : tr("Some merged cells span more than one row, so the rows can't be put in order. Unmerge them first."));
    }
    // (the rows a filter hides stay where they are, and the rows that show are put in order among themselves)
    std::vector<RowSpec> filtered;
    if (!plan->columns)
        for (const RowSpec &r : calc->rowSpecsIn(sheet, firstUnit, lastUnit))
            if (r.filtered) filtered.push_back(r);
    size_t skip = 0;
    for (quint32 u = firstUnit; u <= lastUnit; ++u) {
        while (skip < filtered.size() && filtered[skip].row < u) ++skip;
        if (skip < filtered.size() && filtered[skip].row == u) continue;
        plan->units.push_back(u);
    }
    if (plan->units.size() < 2) return refuse(tr("There is nothing to put in order: the filter shows fewer than two of these rows."));
    plan->keys.assign(plan->units.size() * size_t(spec.levels.size()), SortPlan::Key());
    for (const SortLevel &level : spec.levels) {
        QStringList list;
        for (const QString &item : level.list) list << folded(item);
        plan->lists.push_back(list);
    }
    plan->changesAtPlan = m_loader->changesMade();
    plan->undoIndexAtPlan = m_undo.index();
    return plan;
}

bool EditSession::prepareSort(SortPlan &plan, qint64 budget)
{
    const Calculation *calc = m_book->calc.get();
    const Sheet &s = calc->sheet(plan.sheet);
    const size_t levels = size_t(plan.spec.levels.size());
    qint64 spent = 0;
    for (; plan.prepared < plan.units.size(); ++plan.prepared) {
        if (budget > 0 && spent >= budget) return false;
        const quint32 unit = plan.units[plan.prepared];
        for (size_t l = 0; l < levels; ++l) {
            const SortLevel &level = plan.spec.levels[int(l)];
            const quint32 key = quint32(level.key);
            const quint32 row = plan.columns ? plan.range.row1 + key : unit, col = plan.columns ? unit : plan.range.col1 + key;
            SortPlan::Key &k = plan.keys[plan.prepared * levels + l];
            ++spent;
            if (level.by == SortLevel::By::Value) {
                const Value v = valueAt(plan.sheet, row, col);
                switch (v.kind()) {
                case Value::Kind::Number:
                    k.cls = 0;
                    k.number = v.number();
                    break;
                case Value::Kind::Text: {
                    k.cls = 1;
                    k.text = v.text().left(kMaxKeyChars);
                    plan.keyChars += k.text.size();
                    break;
                }
                case Value::Kind::Bool:
                    k.cls = 2;
                    k.number = v.boolean() ? 1 : 0;
                    break;
                case Value::Kind::Error:
                    k.cls = 3;
                    k.number = double(int(v.errorKind()));
                    break;
                default: k.cls = 4; break;
                }
                if (!level.list.isEmpty()) {
                    const QStringList &list = plan.lists[l];
                    k.listIndex = qint32(list.size());
                    if (k.cls == 1 || k.cls == 0) {
                        const QString text = k.cls == 1 ? folded(v.text()) : folded(jp::calc::numberToText(v.number()));
                        const int at = int(list.indexOf(text));
                        if (at >= 0) k.listIndex = at;
                    }
                }
            } else {
                const Cell *c = s.cells.find(row, col);
                ColorSpec color;
                if (c) {
                    const CellFormat &format = m_book->book->styles().format(c->style);
                    if (level.by == SortLevel::By::CellColor) {
                        const FillSpec &fill = m_book->book->styles().fill(format.fill);
                        if (fill.pattern != PatternKind::None) color = fill.fg;
                    } else {
                        color = m_book->book->styles().font(format.font).color;
                    }
                }
                if (color.kind == ColorSpec::Kind::Auto) color = ColorSpec();
                k.match = colorKey(color) == colorKey(level.color);
            }
        }
        if (plan.keyChars > kMaxSortKeyChars) {
            // (a hostile workbook of huge texts: the plan is given up by the caller seeing `prepared` stand still)
            plan.prepared = plan.units.size();
            plan.keys.clear();
            return true;
        }
    }
    return true;
}

bool EditSession::orderSort(SortPlan &plan, const std::atomic<bool> *cancel)
{
    if (plan.keys.empty() && !plan.units.empty()) return false;
    const size_t levels = size_t(plan.spec.levels.size());
    const size_t n = plan.units.size();
    const QCollator collator = sortCollator(plan.spec.caseSensitive);
    plan.order.resize(n);
    std::iota(plan.order.begin(), plan.order.end(), 0u);
    qint64 compared = 0;
    bool stopped = false;
    struct Stop {};
    auto less = [&](quint32 a, quint32 b) {
        if ((++compared & 1023) == 0 && cancel && cancel->load(std::memory_order_relaxed)) {
            stopped = true;
            throw Stop();
        }
        for (size_t l = 0; l < levels; ++l) {
            const SortLevel &level = plan.spec.levels[int(l)];
            const SortPlan::Key &x = plan.keys[a * levels + l], &y = plan.keys[b * levels + l];
            int c = 0;
            if (level.by != SortLevel::By::Value) {
                if (x.match != y.match) return level.onTop ? x.match : y.match;
                continue;
            }
            if (x.cls == 4 || y.cls == 4) {
                // (nothing in the cell goes last, whichever way the sort goes)
                if (x.cls == y.cls) continue;
                return y.cls == 4;
            }
            if (!level.list.isEmpty() && x.listIndex != y.listIndex) {
                // (values in the list go in its order, or the other way round; those that are not in it go after them, whichever way the sort goes)
                const qint32 size = qint32(plan.lists[l].size());
                if (x.listIndex == size || y.listIndex == size) return x.listIndex < y.listIndex;
                c = x.listIndex < y.listIndex ? -1 : 1;
            } else if (x.cls != y.cls) {
                c = x.cls < y.cls ? -1 : 1;
            } else if (x.cls == 1) {
                c = collator.compare(x.text, y.text);
            } else if (x.number != y.number) {
                c = x.number < y.number ? -1 : 1;
            }
            if (c == 0) continue;
            return level.descending ? c > 0 : c < 0;
        }
        return false;
    };
    try {
        std::stable_sort(plan.order.begin(), plan.order.end(), less);
    } catch (const Stop &) {
        plan.order.clear();
        plan.ordered = false;
        return false;
    }
    (void)stopped;
    plan.ordered = true;
    return true;
}

namespace {

bool sameMeaningOf(const Calculation::CellContent &a, const Calculation::CellContent &b)
{
    return a.exists == b.exists && a.isFormula == b.isFormula && a.formula == b.formula && a.value == b.value && a.style == b.style;
}

} // namespace

EditSession::Outcome EditSession::applySort(SortPlan &plan, qint64 budget)
{
    Outcome out;
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    out.range = plan.range;
    if (plan.stage == 4) return out;   // (it was made)
    const Calculation *calc = m_book->calc.get();
    const quint32 otherFirst = plan.columns ? plan.data.row1 : plan.data.col1, otherLast = plan.columns ? plan.data.row2 : plan.data.col2;
    const size_t n = plan.units.size(), width = size_t(otherLast - otherFirst) + 1;
    QString why;
    if (!plan.pushing) {
        if (!canEdit(&why) || !canEditSheet(plan.sheet, &why)) return refuse(why);
        if (!plan.ordered || plan.order.size() != plan.units.size()) return refuse(tr("The sort could not be finished. Try it again."));
        if (m_loader->changesMade() != plan.changesAtPlan || m_undo.index() != plan.undoIndexAtPlan) return refuse(tr("The cells were changed while the sort was being worked out, so it was not made. Sort again."));
    }
    // (nothing the slices do before the cells are put in changes the workbook: they only read it)
    qint64 spent = 0;
    auto full = [&] { return budget > 0 && spent >= budget; };
    if (plan.stage == 0 && plan.readAt == 0) {
        bool identity = true;
        for (size_t i = 0; i < n; ++i)
            if (plan.order[i] != i) {
                identity = false;
                break;
            }
        if (identity) {
            plan.stage = 4;
            return out;   // (already in order)
        }
        plan.contents.assign(n * width, Calculation::CellContent());
    }
    // every cell as it is, once
    if (plan.stage == 0) {
        for (; plan.readAt < n; ++plan.readAt) {
            if (full()) {
                out.pending = true;
                return out;
            }
            spent += qint64(width);
            for (size_t o = 0; o < width; ++o) {
                const quint32 row = plan.columns ? otherFirst + quint32(o) : plan.units[plan.readAt], col = plan.columns ? plan.units[plan.readAt] : otherFirst + quint32(o);
                plan.contents[plan.readAt * width + o] = calc->content(plan.sheet, row, col);
            }
        }
        plan.stage = 1;
    }
    // the changes
    if (plan.stage == 1) {
        const bool hasShared = !m_book->book->sharedStringsPart().isEmpty();
        for (; plan.buildAt < n; ++plan.buildAt) {
            if (full()) {
                out.pending = true;
                return out;
            }
            spent += qint64(width);
            const size_t i = plan.buildAt, from = plan.order[i];
            if (from == i) continue;
            for (size_t o = 0; o < width; ++o) {
                const quint32 other = otherFirst + quint32(o);
                CellChange ch;
                ch.sheet = plan.sheet;
                ch.row = plan.columns ? other : plan.units[i];
                ch.col = quint16(plan.columns ? plan.units[i] : other);
                ch.before = plan.contents[i * width + o];
                const Calculation::CellContent &src = plan.contents[from * width + o];
                if (!src.exists) {
                    ch.after = Calculation::CellContent();
                } else {
                    ch.after = src;
                    ch.after.arrayRange = CellRange();
                    ch.after.dataTable = false;
                    ch.after.editable = true;
                    if (src.isFormula) {
                        // (written for the place it goes to, as if it were copied there)
                        const qint64 d = qint64(plan.units[i]) - qint64(plan.units[from]);
                        ch.after.formula = offsetFormula(src.formula, plan.columns ? 0 : int(d), plan.columns ? int(d) : 0);
                        ch.after.hasStored = false;
                        ch.after.stored = Value();
                        ch.after.copyOf(src, plan.sheet, ch.row, ch.col, plan.columns ? 0 : d, plan.columns ? d : 0);   // (put takes the held text of the cell it came from, with the distance changed, when it can)
                    } else if (src.value.isText()) {
                        ch.after.sharedString = -1;
                        ch.textToShare = hasShared;
                    }
                }
                if (sameMeaningOf(ch.before, ch.after)) continue;
                plan.step.cells.push_back(std::move(ch));
            }
        }
        plan.contents.clear();
        plan.contents.shrink_to_fit();
        plan.stage = 2;
        if (budget > 0 && spent > 0) {
            out.pending = true;   // (the next part is a slice of its own)
            return out;
        }
    }
    // merged cells go with their lines, and so do the notes and links on the cells
    if (plan.stage == 2) {
        const QVector<CellRange> merges = calc->mergesOf(plan.sheet);
        std::vector<size_t> placeOf(n);   // the place each line goes to
        for (size_t i = 0; i < n; ++i) placeOf[plan.order[i]] = i;
        MergeEdit m;
        m.sheet = plan.sheet;
        m.before = merges;
        m.countBefore = calc->mergeCountOf(plan.sheet);
        m.after = merges;
        bool any = false;
        for (CellRange &r : m.after) {
            if (r.row2 < plan.data.row1 || r.row1 > plan.data.row2 || r.col2 < plan.data.col1 || r.col1 > plan.data.col2) continue;
            const quint32 unit = plan.columns ? r.col1 : r.row1;
            const auto at = std::lower_bound(plan.units.begin(), plan.units.end(), unit);
            if (at == plan.units.end() || *at != unit) continue;
            const quint32 to = plan.units[placeOf[size_t(at - plan.units.begin())]];
            if (to == unit) continue;
            if (plan.columns) r.col1 = r.col2 = to;
            else r.row1 = r.row2 = to;
            any = true;
        }
        m.countAfter = m.countBefore;
        if (any && m.countBefore == qint64(merges.size())) plan.step.merges.push_back(std::move(m));
        if (!plan.step.cells.empty()) {
            // what the file ties to a cell (its note, its link) is moved with it when the file is written: the line that was at place i is now at place j
            auto permute = std::make_shared<CellPermute>();
            permute->sheet = plan.sheet;
            permute->columns = plan.columns;
            permute->first = plan.units.front();
            permute->otherFirst = otherFirst;
            permute->otherLast = otherLast;
            permute->to.resize(size_t(plan.units.back() - plan.units.front()) + 1);
            std::iota(permute->to.begin(), permute->to.end(), permute->first);
            for (size_t i = 0; i < n; ++i) permute->to[plan.units[plan.order[i]] - permute->first] = plan.units[i];
            plan.step.permutes.push_back(std::move(permute));
        }
        plan.stage = 3;
        if (plan.step.cells.empty() && plan.step.merges.empty()) {
            plan.stage = 4;
            return out;
        }
        if (budget > 0) {
            out.pending = true;
            return out;
        }
    }
    // the step is put in (a slice at a time if there is a budget: one step on the undo stack in the end)
    if (budget <= 0) {
        plan.stage = 4;
        pushStep(std::move(plan.step), tr("Sorting"));
        return out;
    }
    if (!plan.push) {
        plan.pushing = true;
        plan.push = beginSlicedPush(std::move(plan.step), tr("Sorting"));
    }
    if (!continueSlicedPush(*plan.push, budget)) {
        out.pending = true;
        return out;
    }
    plan.stage = 4;
    plan.pushing = false;
    return out;
}

EditSession::Outcome EditSession::sortRange(int sheet, const CellRange &range, const SortSpec &spec)
{
    Outcome out;
    QString why;
    const std::shared_ptr<SortPlan> plan = planSort(sheet, range, spec, &why);
    if (!plan) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    prepareSort(*plan, 0);
    if (!orderSort(*plan)) {
        out.ok = false;
        out.problem = tr("The cells hold too much text to sort at once. Select fewer.");
        return out;
    }
    return applySort(*plan);
}

bool EditSession::looksLikeHeader(int sheet, const CellRange &range) const
{
    if (!range.valid || range.row2 <= range.row1 || !m_book || !m_book->calc) return false;
    const Calculation *calc = m_book->calc.get();
    int texts = 0;
    for (quint32 col = range.col1; col <= range.col2; ++col) {
        const Value top = valueAt(sheet, range.row1, col);
        const Cell *c = calc->sheet(sheet).cells.find(range.row1, col);
        if (!top.isText() && !top.isEmpty()) return false;
        if (top.isText() && c && c->hasFormula()) return false;
        if (top.isText()) ++texts;
    }
    if (texts == 0) return false;
    // something under a header that is not text
    for (quint32 col = range.col1; col <= range.col2; ++col) {
        const Value below = valueAt(sheet, range.row1 + 1, col);
        if (below.isNumber() || below.isBool()) return true;
    }
    return false;
}

std::vector<ColorSpec> EditSession::colorsIn(int sheet, const CellRange &range, bool font, bool *none, int limit) const
{
    std::vector<ColorSpec> out;
    if (none) *none = false;
    if (!range.valid || !m_book || !m_book->calc || !m_book->book) return out;
    const Sheet &s = m_book->calc->sheet(sheet);
    qint64 seen = 0;
    s.cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](quint32, quint32, const Cell &c) {
        if (++seen > kMaxCellsPerEdit) return false;
        const CellFormat &format = m_book->book->styles().format(c.style);
        ColorSpec color;
        if (font) {
            color = m_book->book->styles().font(format.font).color;
        } else {
            const FillSpec &fill = m_book->book->styles().fill(format.fill);
            if (fill.pattern != PatternKind::None) color = fill.fg;
        }
        if (color.kind == ColorSpec::Kind::Auto) color = ColorSpec();
        if (!color.isSet()) {
            if (none) *none = true;
            return true;
        }
        if (std::find(out.begin(), out.end(), color) == out.end() && int(out.size()) < limit) out.push_back(color);
        return true;
    });
    // (cells that are not there have no color)
    if (none && qint64(range.cellCount()) > seen) *none = true;
    return out;
}

} // namespace jp::crunch
