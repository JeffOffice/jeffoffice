// Formatting the cells: fonts, fills, borders, number formats, alignment, protection, named styles, merged cells, and clearing. One step each, made of the cells whose formats changed (as they
// were and as they are), the formats of whole columns and rows, and the merged ranges. See EditSession::formatCells.
#include "core/editsession.h"

#include <QCoreApplication>

#include <algorithm>
#include <map>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

// The format the cells of a block that hold nothing have: their row's when the row has one, else their column's, else the first.
struct Implicit {
    quint32 row1 = 0, col1 = 0;
    std::vector<quint32> rows, cols;
    quint32 at(quint32 row, quint32 col) const
    {
        if (row >= row1 && row - row1 < rows.size() && rows[row - row1]) return rows[row - row1];
        if (col >= col1 && col - col1 < cols.size()) return cols[col - col1];
        return 0;
    }
};

Implicit implicitOf(const Calculation &calc, int sheet, quint32 row1, quint32 row2, quint32 col1, quint32 col2)
{
    Implicit out;
    out.row1 = row1;
    out.col1 = col1;
    if (row2 - row1 < 200000) {
        out.rows.assign(size_t(row2 - row1) + 1, 0);
        for (const RowSpec &r : calc.rowSpecsIn(sheet, row1, row2))
            if (r.customFormat && r.row >= row1 && r.row <= row2) out.rows[r.row - row1] = r.style;
    }
    out.cols.assign(size_t(col2 - col1) + 1, 0);
    for (const ColumnSpan &c : calc.columnRuns(sheet, col1, col2))
        for (quint32 col = std::max(c.first, col1); col <= std::min(c.last, col2); ++col) out.cols[col - col1] = c.style;
    return out;
}

} // namespace

EditSession::Outcome EditSession::restyle(int sheet, const std::vector<RestyleTarget> &targets, const QString &text, const SizeBuilder &sizes)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (targets.empty() && !sizes) return out;
    Step step;
    {
        // (the formats are made with the worker held back: it reads the workbook's styles)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        Calculation *calc = hold->calculation();
        Workbook &book = *m_book->book;
        book.clearFormatLimit();
        quint32 row1 = targets.empty() ? 0 : targets.front().row, row2 = row1, col1 = targets.empty() ? 0 : targets.front().col, col2 = col1;
        for (const RestyleTarget &t : targets) {
            row1 = std::min(row1, t.row);
            row2 = std::max(row2, t.row);
            col1 = std::min(col1, t.col);
            col2 = std::max(col2, t.col);
        }
        const Implicit implicitStyles = implicitOf(*calc, sheet, row1, row2, col1, col2);
        std::map<std::pair<quint32, const FormatChange *>, quint32> made;
        step.styles.reserve(targets.size());
        for (const RestyleTarget &t : targets) {
            const Calculation::CellContent before = calc->content(sheet, t.row, t.col);
            const quint32 base = before.exists ? before.style : implicitStyles.at(t.row, t.col);
            if (isLocked(sheet, base)) {
                out.ok = false;
                out.problem = lockedMessage();
                return out;
            }
            quint32 style = 0;
            if (t.change) {
                const auto key = std::make_pair(base, t.change);
                const auto found = made.find(key);
                if (found != made.end()) style = found->second;
                else made[key] = style = book.formatWith(base, *t.change);
            }
            if (style == base) continue;
            if (!before.exists && style == 0) continue;
            StyleCell ch;
            ch.sheet = sheet;
            ch.row = t.row;
            ch.col = t.col;
            ch.before = before.exists ? before.style : 0;
            ch.after = style;
            step.styles.push_back(ch);
        }
        if (sizes) sizes(book, *calc, &step);
        if (book.formatLimitHit()) {
            // (a format that could not be made would have been left out of the step without a word)
            out.ok = false;
            out.problem = tr("This workbook has as many different formats as a workbook can hold (%1 cell formats). Give the cells a format the workbook already has, or clear the formats of some cells, and try again.")
                              .arg(QLocale().toString(jp::office::sheet::kMaxCellFormatsWritten));
            book.clearFormatLimit();
            return out;
        }
    }
    if (step.empty()) return out;
    pushStep(std::move(step), text);
    return out;
}

EditSession::Outcome EditSession::restyleRange(int sheet, const CellRange &range, const std::shared_ptr<const FormatChange> &change, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!range.valid || range.row2 >= kMaxRows || range.col2 >= kMaxColumns) {
        out.ok = false;
        out.problem = tr("Those are not cells of the sheet.");
        return out;
    }
    out.range = range;
    const bool wholeColumns = range.row1 == 0 && range.row2 == kMaxRows - 1;
    const bool wholeRows = !wholeColumns && range.col1 == 0 && range.col2 == kMaxColumns - 1;
    const Calculation *calc = m_book->calc.get();
    std::vector<RestyleTarget> targets;
    if (!wholeColumns && !wholeRows) {
        const qint64 count = (qint64(range.row2) - range.row1 + 1) * (qint64(range.col2) - range.col1 + 1);
        if (count > kMaxStyledCells && change) {
            out.ok = false;
            out.problem = tr("That is more cells than JeffCrunch formats at once (%1). Select whole columns or rows, or fewer cells.").arg(QLocale().toString(kMaxStyledCells));
            return out;
        }
        if (change) {
            targets.reserve(size_t(count));
            for (quint32 r = range.row1; r <= range.row2; ++r)
                for (quint32 c = range.col1; c <= range.col2; ++c) targets.push_back({r, c, change.get()});
        } else {
            // (the cells that are there: one that is not has no format of its own to clear)
            qint64 seen = 0;
            calc->sheet(sheet).cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](const Cell &cell) {
                if (++seen > kMaxStyledCells) return false;
                targets.push_back({cell.row, cell.col, nullptr});
                return true;
            });
            if (seen > kMaxStyledCells) {
                out.ok = false;
                out.problem = tr("That is more cells than JeffCrunch clears the formats of at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxStyledCells));
                return out;
            }
        }
        return restyle(sheet, targets, text);
    }
    // whole columns or rows: the cells that have formats of their own are changed, and the columns' (rows') own format is
    qint64 seen = 0;
    calc->sheet(sheet).cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](const Cell &cell) {
        if (++seen > kMaxStyledCells) return false;
        if (cell.style != 0 || change) targets.push_back({cell.row, cell.col, change.get()});
        return true;
    });
    if (seen > kMaxStyledCells) {
        out.ok = false;
        out.problem = tr("That is more cells than JeffCrunch formats at once (%1). Select fewer columns or rows.").arg(QLocale().toString(kMaxStyledCells));
        return out;
    }
    if (wholeRows && qint64(range.row2) - range.row1 + 1 > kMaxRowsPerResize) {
        out.ok = false;
        out.problem = tr("That is more rows than JeffCrunch formats at once (%1). Select fewer rows.").arg(QLocale().toString(kMaxRowsPerResize));
        return out;
    }
    // (the cells' formats and the columns' or rows' own are made in the same step)
    auto sizes = [=](Workbook &book, const Calculation &c, Step *step) {
        SizeEdit e;
        e.sheet = sheet;
        auto styled = [&](quint32 base) { return change ? book.formatWith(base, *change) : 0u; };
        if (wholeColumns) {
            e.columns = true;
            e.first = range.col1;
            e.last = range.col2;
            e.columnsBefore = c.columnRuns(sheet, range.col1, range.col2);
            quint32 pos = range.col1;
            auto emit = [&](quint32 a, quint32 b, const ColumnSpan *base) {
                ColumnSpan col;
                if (base) col = *base;
                col.first = a;
                col.last = b;
                col.style = styled(col.style);
                if (base || col.width >= 0 || col.hidden || col.style != 0) e.columnsAfter.push_back(col);
            };
            for (const ColumnSpan &r : e.columnsBefore) {
                if (r.first > pos) emit(pos, r.first - 1, nullptr);
                emit(r.first, r.last, &r);
                pos = r.last + 1;
            }
            if (pos <= range.col2) emit(pos, range.col2, nullptr);
        } else {
            e.columns = false;
            e.first = range.row1;
            e.last = range.row2;
            e.rowsBefore = c.rowSpecsIn(sheet, range.row1, range.row2);
            size_t at = 0;
            for (quint32 r = range.row1; r <= range.row2; ++r) {
                RowSpec spec;
                if (at < e.rowsBefore.size() && e.rowsBefore[at].row == r) spec = e.rowsBefore[at++];
                else spec.row = r;
                const quint32 now = styled(spec.customFormat ? spec.style : 0);
                spec.style = now;
                spec.customFormat = now != 0;
                // (a row with nothing to say is not listed)
                if (spec.height >= 0 || spec.hidden || spec.customFormat || spec.customHeight) e.rowsAfter.push_back(spec);
            }
        }
        step->sizes.push_back(std::move(e));
    };
    return restyle(sheet, targets, text, sizes);
}

EditSession::Outcome EditSession::formatCells(int sheet, const CellRange &range, const FormatChange &change, const QString &text)
{
    return restyleRange(sheet, range, std::make_shared<FormatChange>(change), text);
}

EditSession::Outcome EditSession::applyStyle(int sheet, const CellRange &range, quint32 style, const QString &text)
{
    FormatChange c;
    c.namedStyle = style;
    return formatCells(sheet, range, c, text);
}

EditSession::Outcome EditSession::applyStyleNamed(int sheet, const CellRange &range, const QString &name, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    qint32 place = -1;
    {
        // (the style is made with the worker held back: it reads the workbook's styles)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        place = m_book->book->styleNamed(name);
    }
    if (place < 0) {
        out.ok = false;
        out.problem = tr("This workbook has no place to keep the style \"%1\".").arg(name);
        return out;
    }
    return applyStyle(sheet, range, quint32(place), text);
}

EditSession::Outcome EditSession::clearFormats(int sheet, const CellRange &range)
{
    return restyleRange(sheet, range, nullptr, tr("Clearing formats"));
}

EditSession::Outcome EditSession::setBorders(int sheet, const CellRange &range, int edges, const BorderSide &line, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!range.valid || range.row2 >= kMaxRows || range.col2 >= kMaxColumns) {
        out.ok = false;
        out.problem = tr("Those are not cells of the sheet.");
        return out;
    }
    out.range = range;
    CellRange r = range;
    // (a selection of whole columns or rows gets lines in its used cells only)
    const Sheet &s = m_book->calc->sheet(sheet);
    if (r.row2 == kMaxRows - 1) r.row2 = std::min<quint32>(r.row2, std::max(r.row1, s.extentRows > 0 ? s.extentRows - 1 : 0));
    if (r.col2 == kMaxColumns - 1) r.col2 = std::min<quint32>(r.col2, std::max(r.col1, s.extentCols > 0 ? s.extentCols - 1 : 0));
    const qint64 count = (qint64(r.row2) - r.row1 + 1) * (qint64(r.col2) - r.col1 + 1);
    if (count > kMaxCellsPerEdit) {
        out.ok = false;
        out.problem = tr("That is more cells than JeffCrunch draws borders around at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    std::map<std::pair<quint32, quint32>, std::shared_ptr<FormatChange>> changes;
    auto at = [&](quint32 row, quint32 col) -> FormatChange & {
        auto &p = changes[{row, col}];
        if (!p) p = std::make_shared<FormatChange>();
        return *p;
    };
    for (quint32 row = r.row1; row <= r.row2; ++row)
        for (quint32 col = r.col1; col <= r.col2; ++col) {
            const bool first = col == r.col1, last = col == r.col2, top = row == r.row1, bottom = row == r.row2;
            FormatChange *c = nullptr;
            auto change = [&]() -> FormatChange & { return c ? *c : *(c = &at(row, col)); };
            if (first ? (edges & EdgeLeft) : (edges & EdgeInsideVertical)) change().left = line;
            if (last ? (edges & EdgeRight) : (edges & EdgeInsideVertical)) change().right = line;
            if (top ? (edges & EdgeTop) : (edges & EdgeInsideHorizontal)) change().top = line;
            if (bottom ? (edges & EdgeBottom) : (edges & EdgeInsideHorizontal)) change().bottom = line;
            if (edges & EdgeDiagonalUp) {
                change().diagonal = line;
                change().diagonalUp = line.style != BorderStyle::None;
            }
            if (edges & EdgeDiagonalDown) {
                change().diagonal = line;
                change().diagonalDown = line.style != BorderStyle::None;
            }
            // the cells beside the block meet its outer edges: the line is on their side too
            if (first && col > 0 && (edges & EdgeLeft)) at(row, col - 1).right = line;
            if (last && col + 1 < kMaxColumns && (edges & EdgeRight)) at(row, col + 1).left = line;
            if (top && row > 0 && (edges & EdgeTop)) at(row - 1, col).bottom = line;
            if (bottom && row + 1 < kMaxRows && (edges & EdgeBottom)) at(row + 1, col).top = line;
        }
    std::vector<RestyleTarget> targets;
    targets.reserve(changes.size());
    for (const auto &[place, change] : changes) targets.push_back({place.first, place.second, change.get()});
    Outcome result = restyle(sheet, targets, text);
    result.range = range;
    return result;
}

EditSession::Outcome EditSession::clearAll(int sheet, const CellRange &range)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!range.valid) return out;
    // the contents: Clear's step (the cells' formats stay in it), then the formats are cleared too, in one step
    const Calculation *calc = m_book->calc.get();
    const Sheet &s = calc->sheet(sheet);
    std::vector<CellChange> changes;
    bool array = false, locked = false, many = false;
    s.cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](const Cell &c) {
        const Calculation::CellContent before = calc->content(sheet, c.row, c.col);
        if (!before.editable) {
            array = true;
            return false;
        }
        if (isLocked(sheet, before.style)) {
            locked = true;
            return false;
        }
        if (m_book->book->tableHeaderAt(sheet, c.row, c.col, nullptr) >= 0 && (before.isFormula || !before.value.isEmpty())) {
            locked = false;
        }
        if (qint64(changes.size()) >= kMaxCellsPerEdit) {
            many = true;
            return false;
        }
        CellChange ch;
        ch.sheet = sheet;
        ch.row = c.row;
        ch.col = c.col;
        ch.before = before;
        ch.after = Calculation::CellContent();   // nothing: no value, no formula, the first format
        changes.push_back(std::move(ch));
        return true;
    });
    if (array || locked || many) {
        out.ok = false;
        out.problem = array ? tr("You can't change part of an array or a data table.") : locked ? lockedMessage()
                                                                                                : tr("That is more cells than JeffCrunch clears at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    Step step;
    step.cells = std::move(changes);
    // whole columns and rows lose their formats too
    if (range.row1 == 0 && range.row2 == kMaxRows - 1) {
        SizeEdit e;
        e.sheet = sheet;
        e.columns = true;
        e.first = range.col1;
        e.last = range.col2;
        e.columnsBefore = calc->columnRuns(sheet, range.col1, range.col2);
        for (ColumnSpan c : e.columnsBefore) {
            c.style = 0;
            if (c.width >= 0 || c.hidden) e.columnsAfter.push_back(c);
        }
        step.sizes.push_back(std::move(e));
    } else if (range.col1 == 0 && range.col2 == kMaxColumns - 1 && qint64(range.row2) - range.row1 < kMaxRowsPerResize) {
        SizeEdit e;
        e.sheet = sheet;
        e.columns = false;
        e.first = range.row1;
        e.last = range.row2;
        e.rowsBefore = calc->rowSpecsIn(sheet, range.row1, range.row2);
        for (RowSpec r : e.rowsBefore) {
            r.style = 0;
            r.customFormat = false;
            if (r.height >= 0 || r.hidden || r.customHeight) e.rowsAfter.push_back(r);
        }
        step.sizes.push_back(std::move(e));
    }
    if (step.empty()) return out;
    pushStep(std::move(step), tr("Clearing all"));
    return out;
}

EditSession::Outcome EditSession::mergeCells(int sheet, const CellRange &range, MergeKind kind, bool discard)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) return refuse(why);
    if (!range.valid || range.row2 >= kMaxRows || range.col2 >= kMaxColumns) return refuse(tr("Those are not cells of the sheet."));
    out.range = range;
    Calculation *calcEdit = m_book->calc.get();
    const Calculation *calc = calcEdit;
    const Sheet &s = calc->sheet(sheet);
    if (s.isProtected) return refuse(tr("This sheet is protected, so cells can't be merged or unmerged on it. Take the protection off in the program that put it on."));
    if (calc->mergeCountOf(sheet) != qint64(calc->mergesOf(sheet).size())) return refuse(tr("This sheet has more merged cells than JeffCrunch changes. Merge and unmerge them in the program that made it."));
    const QVector<CellRange> before = calc->mergesOf(sheet);
    auto meets = [](const CellRange &a, const CellRange &b) { return !(a.row2 < b.row1 || a.row1 > b.row2 || a.col2 < b.col1 || a.col1 > b.col2); };
    QVector<CellRange> after;
    std::vector<CellRange> blocks;
    if (kind == MergeKind::Unmerge) {
        for (const CellRange &m : before)
            if (!meets(m, range)) after.append(m);
    } else {
        if (range.row1 == range.row2 && range.col1 == range.col2) return out;   // (one cell)
        if (kind == MergeKind::Across) {
            if (range.col1 == range.col2) return out;
            for (quint32 r = range.row1; r <= range.row2; ++r) {
                CellRange one = range;
                one.row1 = one.row2 = r;
                blocks.push_back(one);
            }
        } else {
            blocks.push_back(range);
        }
        if (qint64(blocks.size()) > kMaxCellsPerEdit) return refuse(tr("That is more rows than JeffCrunch merges at once. Select fewer rows."));
        for (const CellRange &m : before) {
            bool taken = false;
            for (const CellRange &b : blocks) taken = taken || meets(m, b);
            if (!taken) after.append(m);
        }
        for (const CellRange &b : blocks) after.append(b);
        if (after.size() > kMaxMerges) return refuse(tr("That would make more merged cells than JeffCrunch makes."));
    }
    // the cells that lose their contents (all but the first of each block)
    Step step;
    bool needAsk = false;
    qint64 emptied = 0;
    for (const CellRange &b : blocks) {
        qint64 seen = 0;
        bool bad = false;
        s.cells.forEachInRange(b.row1, b.col1, b.row2, b.col2, [&](const Cell &c) {
            if (++seen > kMaxCellsPerEdit) return false;
            const Calculation::CellContent content = calc->content(sheet, c.row, c.col);
            if (!content.editable) {
                bad = true;
                return false;
            }
            if (c.row == b.row1 && c.col == b.col1) return true;
            if (!content.isFormula && content.value.isEmpty()) return true;
            needAsk = true;
            if (isLocked(sheet, content.style)) {
                bad = true;
                return false;
            }
            if (++emptied > kMaxCellsPerEdit) return false;
            if (!discard) return true;
            CellChange ch;
            ch.sheet = sheet;
            ch.row = c.row;
            ch.col = c.col;
            ch.before = content;
            ch.after = content;
            ch.after.isFormula = false;
            ch.after.formula.clear();
            ch.after.value = Value();
            ch.after.sharedString = -1;
            step.cells.push_back(std::move(ch));
            return true;
        });
        if (bad) return refuse(tr("You can't merge part of an array or a data table, or a locked cell."));
    }
    if (needAsk && !discard) {
        out.ok = false;
        out.confirm = true;
        out.problem = tr("When cells are joined into one, only what the cell at the top left holds is kept, and what the others hold is lost. Join them anyway?");
        return out;
    }
    MergeEdit m;
    m.sheet = sheet;
    m.before = before;
    m.after = after;
    m.countBefore = calc->mergeCountOf(sheet);
    m.countAfter = after.size();
    if (!(m.after == m.before)) step.merges.push_back(std::move(m));
    if (kind == MergeKind::MergeAndCenter) {
        // the merged cell is centered
        auto change = std::make_shared<FormatChange>();
        change->horizontal = HAlign::Center;
        std::vector<RestyleTarget> targets;
        for (const CellRange &b : blocks) targets.push_back({b.row1, b.col1, change.get()});
        // (the format is made first, then the contents and the merges go in the same step)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
        Workbook &book = *m_book->book;
        for (const RestyleTarget &t : targets) {
            const Calculation::CellContent content = hold->calculation()->content(sheet, t.row, t.col);
            const quint32 style = book.formatWith(content.style, *t.change);
            if (style == content.style && content.exists) continue;
            bool known = false;
            for (CellChange &ch : step.cells)
                if (ch.row == t.row && ch.col == t.col) {
                    ch.after.style = style;
                    known = true;
                }
            if (known) continue;
            CellChange ch;
            ch.sheet = sheet;
            ch.row = t.row;
            ch.col = quint16(t.col);
            ch.before = content;
            ch.after = content;
            ch.after.exists = true;
            ch.after.style = style;
            ch.styleOnly = true;
            step.cells.push_back(std::move(ch));
        }
    }
    if (step.empty()) return out;
    pushStep(std::move(step), kind == MergeKind::Unmerge ? tr("Unmerging cells") : kind == MergeKind::Across ? tr("Merging across") : kind == MergeKind::MergeAndCenter ? tr("Merging and centering") : tr("Merging cells"));
    return out;
}

} // namespace jp::crunch
