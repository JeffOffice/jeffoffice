// Pivot tables in an open workbook (see sheet/pivot.h and sheet/pivotbook.h): reading the data again (Refresh), changing where it is, changing the fields and options of a table, and
// clearing it. The cells of a table are plain cells of the sheet; a change works the table out again and puts the cells right, with the table's and the cache's new state, as one step of
// Undo. What a change leaves alone is left alone: the cells that are the same are not touched, and a table nobody changed is saved as the file has it.
#include "core/editsession.h"

#include "calc/numfmt.h"
#include "sheet/cellref.h"
#include "sheet/pivot.h"
#include "sheet/pivotbook.h"

#include <QCoreApplication>
#include <QRegularExpression>

#include <algorithm>
#include <map>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;

qint64 pivotCacheBytes(const PivotCache &c)
{
    qint64 n = 0;
    for (const PivotCacheField &f : c.fields) n += qint64(f.records.size()) * 4 + qint64(f.items.size()) * qint64(sizeof(PivotValue) + 16);
    return n;
}

namespace {

constexpr int kMaxPivotTablesRefreshed = 5000;

CellRange pageArea(const PivotTable &t, const CellRange &loc)
{
    CellRange r;
    const int n = int(t.pageFields.size());
    if (n == 0 || !loc.valid || loc.row1 < quint32(n) + 1) return r;
    r.valid = true;
    r.row1 = loc.row1 - 1 - quint32(n);
    r.row2 = loc.row1 - 2;
    r.col1 = loc.col1;
    r.col2 = loc.col1 + 1;
    return r;
}

bool overlap(const CellRange &a, const CellRange &b) { return a.valid && b.valid && !(a.row2 < b.row1 || a.row1 > b.row2 || a.col2 < b.col1 || a.col1 > b.col2); }

} // namespace

int EditSession::pivotAt(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->book) return -1;
    const PivotBook &pb = m_book->book->pivots();
    for (int i : pb.tablesOn(sheet)) {
        const CellRange loc = pb.locationOf(i);
        if (!loc.valid) continue;
        if (loc.contains(row, col)) return i;
        if (pb.table(i).table && pageArea(*pb.table(i).table, loc).contains(row, col)) return i;
    }
    return -1;
}

bool EditSession::pivotUsable(int table, QString *why) const
{
    if (!m_book || !m_book->book || table < 0) return false;
    PivotBook &pb = m_book->book->pivots();
    if (table >= pb.tableCount()) return false;
    const PivotTableEntry &e = pb.table(table);
    if (!e.table) {
        if (why) *why = tr("JeffCrunch can't read this pivot table.");
        return false;
    }
    if (e.cache < 0) {
        if (why) *why = tr("This pivot table has lost the data it is made from.");
        return false;
    }
    const PivotCacheEntry &c = pb.cache(e.cache);
    if (!c.cache) {
        if (why) *why = tr("JeffCrunch can't read the data this pivot table is made from (%1).").arg(c.why);
        return false;
    }
    if (!c.cache->unsupported.isEmpty()) {
        if (why) *why = tr("This pivot table uses %1, which JeffCrunch doesn't work out yet. It stays as it is.").arg(c.cache->unsupported);
        return false;
    }
    if (!e.table->unsupported.isEmpty()) {
        if (why) *why = tr("This pivot table uses %1, which JeffCrunch doesn't work out yet. It stays as it is.").arg(e.table->unsupported);
        return false;
    }
    for (const PivotDataField &d : e.table->dataFields)
        if (d.showDataAs != QLatin1String("normal")) {
            if (why) *why = tr("This pivot table shows a value as a percent, a difference, or a rank, which JeffCrunch doesn't work out yet. It stays as it is.");
            return false;
        }
    return true;
}

QString EditSession::pivotSourceText(int table) const
{
    if (!m_book || !m_book->book || table < 0 || table >= m_book->book->pivots().tableCount()) return QString();
    PivotBook &pb = m_book->book->pivots();
    const int ci = pb.table(table).cache;
    if (ci < 0) return QString();
    const PivotCacheEntry &c = pb.cache(ci);
    if (!c.sourceName.isEmpty()) return c.sourceName;
    if (c.source.valid && c.sourceSheet >= 0 && c.sourceSheet < m_book->book->info().sheets.size())
        return sheetNamePrefix(m_book->book->info().sheets[c.sourceSheet].name) + rangeName(c.source);
    return QString();
}

bool EditSession::pivotSourceRange(int cache, int *sheet, CellRange *range, QString *why, const SourceOver *over) const
{
    PivotBook &pb = m_book->book->pivots();
    PivotCacheEntry c = pb.cache(cache);
    if (over && over->cache == cache) {
        c.source = over->range;
        c.sourceSheet = over->sheet;
        c.sourceName = over->name;
    }
    const auto &sheets = m_book->book->info().sheets;
    auto usableSheet = [&](int s) { return s >= 0 && s < sheets.size() && !sheets[s].deleted && sheets[s].kind == SheetKind::Worksheet; };
    if (c.source.valid && c.sourceSheet >= 0 && c.sourceName.isEmpty()) {
        if (!usableSheet(c.sourceSheet)) {
            if (why) *why = tr("The sheet this pivot table is made from is not in the workbook any more.");
            return false;
        }
        *sheet = c.sourceSheet;
        *range = c.source;
        return true;
    }
    if (!c.sourceName.isEmpty()) {
        for (const Table &t : m_book->book->tables())
            if (t.name.compare(c.sourceName, Qt::CaseInsensitive) == 0) {
                *sheet = t.sheet;
                range->valid = true;
                range->row1 = t.row1;
                range->col1 = t.col1;
                range->row2 = t.totalsRows > 0 ? t.row2 - 1 : t.row2;
                range->col2 = t.col2;
                return usableSheet(t.sheet);
            }
        for (const DefinedName &n : m_book->book->info().definedNames) {
            if (n.name.compare(c.sourceName, Qt::CaseInsensitive) != 0) continue;
            // "Sheet1!$A$1:$D$10" or "'My Sheet'!$A$1:$D$10"
            const int bang = n.ref.lastIndexOf(QLatin1Char('!'));
            if (bang <= 0) break;
            QString sheetName = n.ref.left(bang);
            if (sheetName.startsWith(QLatin1Char('\'')) && sheetName.endsWith(QLatin1Char('\'')) && sheetName.size() > 1) sheetName = sheetName.mid(1, sheetName.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
            QString ref = n.ref.mid(bang + 1);
            ref.remove(QLatin1Char('$'));
            CellRange r;
            if (!parseRangeRef(ref, &r)) break;
            for (int s = 0; s < sheets.size(); ++s)
                if (!sheets[s].deleted && sheets[s].name.compare(sheetName, Qt::CaseInsensitive) == 0 && usableSheet(s)) {
                    *sheet = s;
                    *range = r;
                    return true;
                }
            break;
        }
    }
    if (why) *why = tr("JeffCrunch can't find the data this pivot table is made from.");
    return false;
}

namespace {

// The number format code of an id the file defines or the program knows.
QString codeOfFormat(const Styles &styles, int id)
{
    if (styles.numFmts.contains(id)) return styles.numFmts.value(id);
    return jp::calc::builtinFormatCode(id, QLocale(QLocale::English, QLocale::UnitedStates));
}

} // namespace

EditSession::Outcome EditSession::pivotStep(std::vector<PivotChange> changes, const QString &text, QStringList *skipped)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    Workbook &book = *m_book->book;
    PivotBook &pb = book.pivots();
    Step step;
    // the cells every table of the step covers now (so that two tables don't cover the same cell)
    std::vector<std::pair<int, CellRange>> placed;
    auto one = [&](PivotChange &ch) -> QString {
        const PivotTableEntry &entry = pb.table(ch.table);
        const int sheet = entry.sheet;
        if (!canEditSheet(sheet, &why) || !allows(sheet, Allow::Pivot, &why)) return why;
        const int ci = entry.cache;
        const std::shared_ptr<const PivotCache> cache = ch.cacheNow ? std::shared_ptr<const PivotCache>(ch.cacheNow) : pb.cache(ci).cache;
        if (!cache) return tr("JeffCrunch can't read the data this pivot table is made from.");
        const CellRange oldLoc = pb.locationOf(ch.table);
        if (!oldLoc.valid) return tr("JeffCrunch can't read this pivot table.");
        const PivotTable &old = *entry.table;
        const CellRange oldPage = pageArea(old, oldLoc);
        PivotOptions options;
        // (the format a number shows in is the one of the cell above that shows that very number; looked for once for each number in a column, and no farther than 512 lines up: past that, or with no
        // such cell, it is the field's own)
        auto formats = std::make_shared<std::map<std::pair<int, double>, int>>();
        options.format = [&, formats](double v, int id, int gridRow, int gridCol) {
            const auto key = std::make_pair(gridCol, v);
            auto known = formats->find(key);
            int use = id;
            if (known != formats->end()) {
                use = known->second;
            } else {
                int found = id;
                const qint64 top = std::max<qint64>(qint64(oldLoc.row1), qint64(oldLoc.row1) + gridRow - 512);
                for (qint64 r = qint64(oldLoc.row1) + gridRow; r >= top; --r) {
                    const Calculation::CellContent c = m_book->calc->content(sheet, quint32(r), oldLoc.col1 + quint32(gridCol));
                    if (c.exists && !c.isFormula && c.value.isNumber() && c.value.number() == v) {
                        found = book.styles().numFmtId(c.style);
                        break;
                    }
                }
                (*formats)[key] = found;
                use = found;
            }
            const QString code = codeOfFormat(book.styles(), use);
            QString shown;
            if (!jp::calc::formatTextFunction(v, code.isEmpty() ? QStringLiteral("General") : code, book.info().dateSystem, &shown)) return QString::number(v, 'g', 15);
            return shown;
        };
        PivotGrid grid;
        QString problem;
        if (!computePivot(*ch.now, *cache, &grid, &problem, options)) return tr("This pivot table can't be worked out: %1.").arg(problem);
        // (a table of more than 256 columns is not made: the original program does not finish updating one; a wider one that is there already is let be)
        if (int(grid.colItems.size()) > kPivotMaxWrittenColumnItems && grid.cols > int(oldLoc.col2 - oldLoc.col1 + 1))
            return tr("This pivot table would have %1 columns. Put a field with fewer items across, or on the rows.").arg(grid.cols);
        const int pages = int(ch.now->pageFields.size());
        if (pages > 0 && oldLoc.row1 < quint32(pages) + 1) return tr("There is no room above the pivot table for its filters. Move the table down first.");
        // (a table with nothing in it claims no more than it had, or its frame below)
        const bool emptyTable = ch.now->rowFields.isEmpty() && ch.now->colFields.isEmpty() && ch.now->dataFields.isEmpty();
        if (emptyTable) {
            const int rows = std::max(1, std::min<int>(grid.rows, int(oldLoc.row2 - oldLoc.row1 + 1))), cols = std::max(1, std::min<int>(grid.cols, int(oldLoc.col2 - oldLoc.col1 + 1)));
            grid.rows = rows;
            grid.cols = cols;
            grid.cells.resize(qsizetype(rows) * cols);
            grid.cells.fill(PivotCell());
        }
        ch.now->rowPageCount = pages;
        ch.now->colPageCount = pages > 0 ? 1 : 0;
        pivotApplyGrid(ch.now.get(), grid, oldLoc.row1, oldLoc.col1);
        CellRange newLoc;
        parseRangeRef(ch.now->ref, &newLoc);
        if (!newLoc.valid || newLoc.row2 >= 1048576 || newLoc.col2 >= 16384) return tr("The pivot table would go past the edge of the sheet.");
        const CellRange newPage = pageArea(*ch.now, newLoc);
        // the new area must be free: cells of the table's own, or empty
        auto freeOfOthers = [&](const CellRange &area) {
            if (!area.valid) return true;
            for (int other = 0; other < pb.tableCount(); ++other) {
                if (other == ch.table || pb.table(other).sheet != sheet) continue;
                const CellRange ol = pb.locationOf(other);
                if (overlap(area, ol) || (pb.table(other).table && overlap(area, pageArea(*pb.table(other).table, ol)))) return false;
            }
            for (const auto &p : placed)
                if (pb.table(p.first).sheet == sheet && overlap(area, p.second)) return false;
            bool free = true;
            m_book->calc->sheet(sheet).cells.forEachInRange(area.row1, area.col1, area.row2, area.col2, [&](quint32 r, quint32 c, const Cell &cell) {
                if (oldLoc.contains(r, c) || oldPage.contains(r, c)) return true;
                if (cell.hasFormula() || cell.type != CellType::Empty) {
                    free = false;
                    return false;
                }
                return true;
            });
            if (!free) return false;
            // merged cells, links, notes, rules of validation, and conditional formats under the part the table grows over are in the way too
            auto insideOld = [&](const CellRange &r) { return (oldLoc.valid && r.row1 >= oldLoc.row1 && r.row2 <= oldLoc.row2 && r.col1 >= oldLoc.col1 && r.col2 <= oldLoc.col2) || (oldPage.valid && r.row1 >= oldPage.row1 && r.row2 <= oldPage.row2 && r.col1 >= oldPage.col1 && r.col2 <= oldPage.col2); };
            auto meets = [&](const CellRange &r) {
                if (!overlap(area, r)) return false;
                CellRange i;
                i.valid = true;
                i.row1 = std::max(area.row1, r.row1);
                i.row2 = std::min(area.row2, r.row2);
                i.col1 = std::max(area.col1, r.col1);
                i.col2 = std::min(area.col2, r.col2);
                return !insideOld(i);
            };
            const Sheet &sh = m_book->calc->sheet(sheet);
            for (const CellRange &r : sh.merges) if (meets(r)) return false;
            for (const Validation &v : sh.validations) for (const CellRange &r : v.ranges) if (meets(r)) return false;
            for (const CondRule &c : sh.conditionalFormats) for (const CellRange &r : c.ranges) if (meets(r)) return false;
            for (const LinkSpec &l : m_anchors.links(sheet)) if (meets(l.range)) return false;
            for (const auto &n : m_anchors.notes(sheet)) {
                CellRange r;
                r.valid = true;
                r.row1 = r.row2 = quint32(n.first >> 32);
                r.col1 = r.col2 = quint32(n.first & 0xffffffffu);
                if (meets(r)) return false;
            }
            return true;
        };
        if (!freeOfOthers(newLoc) || !freeOfOthers(newPage))
            return tr("The pivot table would cover cells that have something in them (%1). Clear those cells, or move the pivot table, and try again.").arg(rangeName(newLoc));
        // (the original program draws the table with nothing in it as its empty frame, 14 lines of 7 columns, when there is room for it)
        if (emptyTable && ch.now->gridDropZones && (newLoc.row2 - newLoc.row1 + 1 != 14 || newLoc.col2 - newLoc.col1 + 1 != 7)) {
            CellRange frame = newLoc;
            frame.row2 = frame.row1 + 13;
            frame.col2 = frame.col1 + 6;
            if (frame.row2 < 1048576 && frame.col2 < 16384 && freeOfOthers(frame)) {
                grid.rows = 14;
                grid.cols = 7;
                grid.cells.resize(98);
                grid.cells.fill(PivotCell());
                pivotApplyGrid(ch.now.get(), grid, oldLoc.row1, oldLoc.col1);
                parseRangeRef(ch.now->ref, &newLoc);
            }
        }
        placed.push_back({ch.table, newLoc});
        if (qint64(newLoc.cellCount()) > kMaxCellsPerEdit * 40) return tr("The pivot table would be larger than JeffCrunch changes at once.");

        // the cells
        std::set<std::pair<quint32, quint32>> written;
        auto put = [&](quint32 row, quint32 col, const PivotCell &pc) {
            written.insert({row, col});
            CellChange cc;
            cc.sheet = sheet;
            cc.row = row;
            cc.col = quint16(col);
            cc.before = m_book->calc->content(sheet, row, col);
            cc.after = cc.before;
            const bool hadValue = cc.before.exists && (cc.before.isFormula || !cc.before.value.isEmpty());
            if (pc.kind == PivotCell::Empty) {
                if (!hadValue) return;
                cc.after.isFormula = false;
                cc.after.formula.clear();
                cc.after.value = Value();
                cc.after.sharedString = -1;
                cc.after.hasStored = false;
                if (cc.after.style == 0) cc.after.exists = false;
                step.cells.push_back(std::move(cc));
                return;
            }
            Value v;
            switch (pc.kind) {
            case PivotCell::Number: v = Value::number(pc.number); break;
            case PivotCell::Bool: v = Value::boolean(pc.number != 0); break;
            case PivotCell::Error: v = Value::error(jp::calc::errorKindFromText(pc.text)); break;
            default: v = Value::text(pc.text); break;
            }
            if (!cc.before.isFormula && cc.before.exists && cc.before.value == v) return;
            cc.after.exists = true;
            cc.after.isFormula = false;
            cc.after.formula.clear();
            cc.after.value = v;
            cc.after.sharedString = -1;
            cc.after.hasStored = false;
            if (pc.kind == PivotCell::Text) cc.textToShare = !book.sharedStringsPart().isEmpty();
            // (a cell that has no format of its own gets the one the table gives the values: a person's own format is kept)
            if (pc.numFmtId > 0 && (!cc.before.exists || cc.before.style == 0)) {
                const QString code = codeOfFormat(book.styles(), pc.numFmtId);
                if (!code.isEmpty() && code != QLatin1String("General")) cc.formatCode = code;
            }
            step.cells.push_back(std::move(cc));
        };
        for (int r = 0; r < grid.rows; ++r)
            for (int c = 0; c < grid.cols; ++c) put(newLoc.row1 + quint32(r), newLoc.col1 + quint32(c), grid.at(r, c));
        for (int i = 0; i < pages; ++i) {
            put(newPage.row1 + quint32(i), newPage.col1, grid.pageLabels[2 * i]);
            put(newPage.row1 + quint32(i), newPage.col1 + 1, grid.pageLabels[2 * i + 1]);
        }
        // what the table covered and does not cover now is cleared
        for (const CellRange &was : {oldLoc, oldPage}) {
            if (!was.valid) continue;
            std::vector<std::pair<quint32, quint32>> gone;
            m_book->calc->sheet(sheet).cells.forEachInRange(was.row1, was.col1, was.row2, was.col2, [&](quint32 r, quint32 c, const Cell &cell) {
                if ((cell.hasFormula() || cell.type != CellType::Empty) && !written.count({r, c})) gone.push_back({r, c});
                return true;
            });
            for (const auto &g : gone) put(g.first, g.second, PivotCell());
        }

        PivotEdit pe;
        pe.table = ch.table;
        pe.tableBefore = entry.table;
        pe.structureBefore = entry.structureChanged;
        pe.tableAfter = ch.now;
        pe.structureAfter = entry.structureChanged || ch.structure;
        if (ch.cacheNow) {
            const PivotCacheEntry &ce = pb.cache(ci);
            pe.cache = ci;
            pe.cacheBefore = ce.cache;
            pe.cacheAfter = ch.cacheNow;
            pe.sourceBefore = ce.source;
            pe.sheetBefore = ce.sourceSheet;
            pe.nameBefore = ce.sourceName;
            pe.sourceAfter = ch.source;
            pe.sheetAfter = ch.sourceSheet;
            pe.nameAfter = ch.sourceName;
        }
        step.pivots.push_back(std::move(pe));
        return QString();
    };
    QStringList failed;
    // (the tables made from one cache are done or left alone together: a table left as it is must not meet a cache that was read again)
    for (size_t i = 0; i < changes.size();) {
        size_t j = i + 1;
        while (j < changes.size() && changes[i].cache >= 0 && changes[j].cache == changes[i].cache) ++j;
        const size_t cells = step.cells.size(), pivots = step.pivots.size(), placedBefore = placed.size();
        QString problem;
        QString name;
        for (size_t k = i; k < j && problem.isEmpty(); ++k) {
            problem = one(changes[k]);
            if (!problem.isEmpty() && pb.table(changes[k].table).table) name = pb.table(changes[k].table).table->name;
        }
        if (!problem.isEmpty()) {
            if (!skipped) return refuse(problem);
            step.cells.resize(cells);
            step.pivots.resize(pivots);
            placed.resize(placedBefore);
            failed << (name.isEmpty() ? problem : name + QStringLiteral(": ") + problem);
        }
        i = j;
    }
    if (skipped) *skipped = failed;
    if (step.empty()) {
        if (!failed.isEmpty()) return refuse(failed.join(QLatin1Char('\n')));
        return out;
    }
    out = pushTableStep(std::move(step), text);
    if (!failed.isEmpty()) out.problem = tr("These pivot tables were not refreshed:\n%1").arg(failed.join(QLatin1Char('\n')));
    return out;
}

void EditSession::applyPivotEdits(Step *step, bool forward, Workbook &book)
{
    if (step->pivots.empty()) return;
    PivotBook &pb = book.pivots();
    for (const PivotEdit &e : step->pivots) {
        if (e.cache >= 0) {
            if (forward) pb.setCache(e.cache, e.cacheAfter, e.sourceAfter, e.sheetAfter, e.nameAfter);
            else pb.setCache(e.cache, e.cacheBefore, e.sourceBefore, e.sheetBefore, e.nameBefore);
        }
        pb.setTable(e.table, forward ? e.tableAfter : e.tableBefore, forward ? e.structureAfter : e.structureBefore);
    }
}

EditSession::Outcome EditSession::refreshPivotCaches(const std::vector<int> &caches, const QString &text, const SourceOver *over, bool partial)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    Workbook &book = *m_book->book;
    PivotBook &pb = book.pivots();
    std::vector<PivotChange> changes;
    QStringList notDone;
    for (int ci : caches) {
        const size_t changesBefore = changes.size();
        const QString problem = [&]() -> QString {
        const PivotCacheEntry &ce = pb.cache(ci);
        if (!ce.cache) return tr("JeffCrunch can't read the data this pivot table is made from.");
        if (!ce.cache->unsupported.isEmpty()) return tr("This pivot table uses %1, which JeffCrunch can't read again yet.").arg(ce.cache->unsupported);
        int sheet = -1;
        CellRange range;
        if (!pivotSourceRange(ci, &sheet, &range, &why, over)) return why;
        // the source: its first row names the fields
        std::vector<int> tables = pb.tablesOfCache(ci);
        PivotSource ps;
        ps.row1 = range.row1;
        ps.col1 = range.col1;
        ps.row2 = range.row2;
        ps.col2 = range.col2;
        QHash<quint32, bool> dateStyle;
        const Sheet &src = m_book->calc->sheet(sheet);
        ps.cell = [&](quint32 r, quint32 c) {
            PivotValue pv;
            const Value v = valueAt(sheet, r, c);
            switch (v.kind()) {
            case Value::Kind::Empty: break;
            case Value::Kind::Number: {
                const Cell *cell = src.cells.find(r, c);
                bool date = false;
                if (cell) {
                    auto it = dateStyle.find(cell->style);
                    if (it == dateStyle.end()) it = dateStyle.insert(cell->style, formatOfStyle(cell->style).kind() == jp::calc::FormatKind::Date);
                    date = it.value();
                }
                pv.kind = date ? PivotValue::Date : PivotValue::Number;
                pv.number = v.number();
                break;
            }
            case Value::Kind::Text: pv.kind = PivotValue::Text; pv.text = v.text(); break;
            case Value::Kind::Bool: pv.kind = PivotValue::Bool; pv.number = v.boolean() ? 1 : 0; break;
            case Value::Kind::Error: pv.kind = PivotValue::Error; pv.text = jp::calc::errorKindText(v.errorKind()); break;
            case Value::Kind::Array: break;
            }
            return pv;
        };
        for (int t : tables) {
            const PivotTableEntry &te = pb.table(t);
            if (!te.table) return tr("JeffCrunch can't read a pivot table made from this data.");
            for (int f : te.table->rowFields) if (f >= 0) ps.axisFields.insert(f);
            for (int f : te.table->colFields) if (f >= 0) ps.axisFields.insert(f);
            for (const PivotPageField &p : te.table->pageFields) ps.axisFields.insert(p.fld);
        }
        // (the format of the first cell of data of each column is the field's)
        for (quint32 c = range.col1; c <= range.col2; ++c) {
            const Calculation::CellContent first = m_book->calc->content(sheet, range.row1 + 1, c);
            ps.numFmtIds.append(first.exists ? int(book.styles().numFmtId(first.style)) : 0);
            if (range.col2 - range.col1 > quint32(kPivotMaxFields)) break;
        }
        auto fresh = std::make_shared<PivotCache>();
        QString problem;
        if (!buildPivotCache(ps, *ce.cache, fresh.get(), &problem)) return tr("The data can't be read again: %1.").arg(problem);
        fresh->sourceType = ce.cache->sourceType;
        for (int t : tables) {
            PivotChange ch;
            ch.table = t;
            ch.now = std::make_shared<PivotTable>(*pb.table(t).table);
            matchPivotItems(ch.now.get(), *ce.cache, *fresh);
            ch.cache = ci;
            ch.cacheNow = fresh;
            ch.source = over && over->cache == ci ? (over->name.isEmpty() ? over->range : CellRange()) : (ce.sourceName.isEmpty() ? range : CellRange());
            ch.sourceSheet = over && over->cache == ci ? (over->name.isEmpty() ? over->sheet : -1) : (ce.sourceName.isEmpty() ? sheet : -1);
            ch.sourceName = over && over->cache == ci ? over->name : ce.sourceName;
            // a lone value has no place of its own among the rows and columns; fields that came or went move the places the part's other elements name
            pivotNormalizeLayout(ch.now.get());
            bool sameFields = fresh->fields.size() == ce.cache->fields.size();
            for (int f = 0; sameFields && f < fresh->fromField.size(); ++f) sameFields = fresh->fromField[f] == f;
            ch.structure = !sameFields;
            changes.push_back(std::move(ch));
        }
        return QString();
        }();
        if (problem.isEmpty()) continue;
        changes.resize(changesBefore);
        if (!partial) return refuse(problem);
        const auto tables = pb.tablesOfCache(ci);
        const QString name = !tables.empty() && pb.table(tables.front()).table ? pb.table(tables.front()).table->name : QString();
        notDone << (name.isEmpty() ? problem : name + QStringLiteral(": ") + problem);
    }
    if (changes.empty()) return notDone.isEmpty() ? out : refuse(notDone.join(QLatin1Char('\n')));
    QStringList skipped;
    Outcome result = pivotStep(std::move(changes), text, partial ? &skipped : nullptr);
    notDone << skipped;
    if (result.ok && !notDone.isEmpty()) result.problem = tr("These pivot tables were not refreshed:\n%1").arg(notDone.join(QLatin1Char('\n')));
    return result;
}

EditSession::Outcome EditSession::refreshPivot(int table)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) {
        out.ok = false;
        out.problem = why.isEmpty() ? tr("There is no pivot table here.") : why;
        return out;
    }
    return refreshPivotCaches({m_book->book->pivots().table(table).cache}, tr("Refresh Pivot Table"));
}

EditSession::Outcome EditSession::refreshAllPivots()
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    PivotBook &pb = m_book->book->pivots();
    std::vector<int> caches;
    for (int t = 0; t < pb.tableCount() && int(caches.size()) < kMaxPivotTablesRefreshed; ++t) {
        if (!pivotUsable(t)) continue;
        const int ci = pb.table(t).cache;
        if (std::find(caches.begin(), caches.end(), ci) == caches.end()) caches.push_back(ci);
    }
    if (caches.empty()) {
        out.ok = false;
        out.problem = tr("There are no pivot tables in this workbook that JeffCrunch can read again.");
        return out;
    }
    return refreshPivotCaches(caches, tr("Refresh All"), nullptr, true);
}

EditSession::Outcome EditSession::changePivotSource(int table, const QString &where)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    if (table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) return refuse(why.isEmpty() ? tr("There is no pivot table here.") : why);
    PivotBook &pb = m_book->book->pivots();
    const int ci = pb.table(table).cache;
    const PivotCacheEntry &ce = pb.cache(ci);
    // what the person typed: "Sheet1!A1:D100", "A1:D100", the name of a table, or a defined name
    QString text = where.trimmed();
    if (text.isEmpty()) return refuse(tr("Type the cells the data is in, for example Sheet1!A1:D100, or the name of a table."));
    int sheet = ce.sourceSheet >= 0 ? ce.sourceSheet : pb.table(table).sheet;
    CellRange range;
    QString name;
    const int bang = text.lastIndexOf(QLatin1Char('!'));
    QString refPart = text;
    if (bang > 0) {
        QString sheetName = text.left(bang);
        if (sheetName.startsWith(QLatin1Char('\'')) && sheetName.endsWith(QLatin1Char('\'')) && sheetName.size() > 1) sheetName = sheetName.mid(1, sheetName.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
        sheet = -1;
        const auto &sheets = m_book->book->info().sheets;
        for (int s = 0; s < sheets.size(); ++s)
            if (!sheets[s].deleted && sheets[s].kind == SheetKind::Worksheet && sheets[s].name.compare(sheetName, Qt::CaseInsensitive) == 0) sheet = s;
        if (sheet < 0) return refuse(tr("There is no sheet called \"%1\".").arg(sheetName));
        refPart = text.mid(bang + 1);
    }
    refPart.remove(QLatin1Char('$'));
    if (!parseRangeRef(refPart, &range) || range.row1 == range.row2) {
        // a table's name, or a name
        bool found = false;
        for (const Table &t : m_book->book->tables())
            if (t.name.compare(text, Qt::CaseInsensitive) == 0) {
                name = t.name;
                found = true;
            }
        if (!found)
            for (const DefinedName &n : m_book->book->info().definedNames)
                if (n.name.compare(text, Qt::CaseInsensitive) == 0) {
                    name = n.name;
                    found = true;
                }
        if (!found) return refuse(tr("\"%1\" is not a range of cells, a table, or a name. Type the cells the data is in, with its first row of names, for example Sheet1!A1:D100.").arg(text));
    }
    SourceOver over;
    over.cache = ci;
    over.sheet = name.isEmpty() ? sheet : -1;
    over.range = name.isEmpty() ? range : CellRange();
    over.name = name;
    return refreshPivotCaches({ci}, tr("Change Pivot Table Data Source"), &over);
}

EditSession::Outcome EditSession::setPivot(int table, const PivotTable &changed, bool structureChanged, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) {
        out.ok = false;
        out.problem = why.isEmpty() ? tr("There is no pivot table here.") : why;
        return out;
    }
    PivotBook &pb = m_book->book->pivots();
    PivotChange ch;
    ch.table = table;
    ch.now = std::make_shared<PivotTable>(changed);
    pivotRemapDataFields(*pb.table(table).table, ch.now.get());
    pivotSyncRoles(ch.now.get());
    pivotNormalizeLayout(ch.now.get());
    ch.structure = structureChanged;
    // a field put on an axis or a page needs its items shared in the cache and listed in the table
    const int ci = pb.table(table).cache;
    const PivotCacheEntry &ce = pb.cache(ci);
    auto cache = std::make_shared<PivotCache>(*ce.cache);
    if (pivotPrepareFields(ch.now.get(), cache.get())) {
        ch.cache = ci;
        ch.cacheNow = cache;
        ch.source = ce.source;
        ch.sourceSheet = ce.sourceSheet;
        ch.sourceName = ce.sourceName;
    }
    return pivotStep({ch}, text);
}

EditSession::Outcome EditSession::clearPivot(int table, bool filtersOnly)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) {
        out.ok = false;
        out.problem = why.isEmpty() ? tr("There is no pivot table here.") : why;
        return out;
    }
    PivotTable t = *m_book->book->pivots().table(table).table;
    if (filtersOnly) {
        for (PivotPageField &p : t.pageFields) p.item = -1;
        for (PivotField &f : t.fields)
            for (PivotTableItem &it : f.items) it.hidden = false;
    } else {
        t.rowFields.clear();
        t.colFields.clear();
        t.pageFields.clear();
        t.dataFields.clear();
        t.dataOnRows = false;
        for (PivotField &f : t.fields) {
            for (PivotTableItem &it : f.items) it.hidden = false;
        }
        pivotSyncRoles(&t);
    }
    PivotChange ch;
    ch.table = table;
    ch.now = std::make_shared<PivotTable>(std::move(t));
    ch.structure = !filtersOnly;
    return pivotStep({ch}, filtersOnly ? tr("Clear Filters") : tr("Clear Pivot Table"));
}

} // namespace jp::crunch
