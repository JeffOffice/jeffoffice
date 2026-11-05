// Pivot tables in an open workbook (see sheet/pivot.h and sheet/pivotbook.h): reading the data again (Refresh), changing where it is, changing the fields and options of a table, and
// clearing it. The cells of a table are plain cells of the sheet; a change works the table out again and puts the cells right, with the table's and the cache's new state, as one step of
// Undo. What a change leaves alone is left alone: the cells that are the same are not touched, and a table nobody changed is saved as the file has it.
#include "core/editsession.h"
#include "sheet/entry.h"

#include "calc/numfmt.h"
#include "sheet/cellref.h"
#include "sheet/pivot.h"
#include "sheet/pivotbook.h"
#include "sheet/pivotformula.h"

#include <QDate>
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
    for (const PivotCacheField &f : c.fields) n += qint64(f.records.size() + f.rawRecords.size()) * 4 + qint64(f.items.size() + f.rawItems.size()) * qint64(sizeof(PivotValue) + 16);
    return n;
}

namespace {

constexpr int kMaxPivotTablesRefreshed = 5000;
// The cells of source data read again when a file opens, all the tables together (a table of 100,000 rows and 20 columns is 2 million; real files have a few tables): the rest wait for Refresh.
constexpr qint64 kMaxPivotCellsRefreshedOnOpen = 20000000;
qint64 gRefreshOnOpenLimit = 0;

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
        if (d.showDataAs != QLatin1String("normal") && d.showDataAs != QLatin1String("difference") && d.showDataAs != QLatin1String("percent") && d.showDataAs != QLatin1String("percentDiff") &&
            d.showDataAs != QLatin1String("runTotal") && d.showDataAs != QLatin1String("percentOfRow") && d.showDataAs != QLatin1String("percentOfCol") && d.showDataAs != QLatin1String("percentOfTotal") &&
            d.showDataAs != QLatin1String("index") && d.showDataAs != QLatin1String("rankAscending") && d.showDataAs != QLatin1String("rankDescending") && d.showDataAs != QLatin1String("percentOfParentRow") &&
            d.showDataAs != QLatin1String("percentOfParentCol")) {
            if (why) *why = tr("This pivot table shows a value as something that JeffCrunch doesn't work out yet. It stays as it is.");
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

bool EditSession::pivotNameRange(const QString &nameText, int *sheet, CellRange *range) const
{
    const auto &sheets = m_book->book->info().sheets;
    auto usableSheet = [&](int s) { return s >= 0 && s < sheets.size() && !sheets[s].deleted && sheets[s].kind == SheetKind::Worksheet; };
    for (const Table &t : m_book->book->tables())
        if (t.name.compare(nameText, Qt::CaseInsensitive) == 0) {
            *sheet = t.sheet;
            range->valid = true;
            range->row1 = t.row1;
            range->col1 = t.col1;
            range->row2 = t.totalsRows > 0 ? t.row2 - 1 : t.row2;
            range->col2 = t.col2;
            return usableSheet(t.sheet);
        }
    for (const DefinedName &n : m_book->book->info().definedNames) {
        if (n.name.compare(nameText, Qt::CaseInsensitive) != 0) continue;
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
    return false;
}

bool EditSession::parsePivotSource(const QString &where, int defaultSheet, int *sheet, CellRange *range, QString *name, QString *why) const
{
    const QString text = where.trimmed();
    if (text.isEmpty()) {
        *why = tr("Type the cells the data is in, for example Sheet1!A1:D100, or the name of a table.");
        return false;
    }
    *sheet = defaultSheet;
    *name = QString();
    const int bang = text.lastIndexOf(QLatin1Char('!'));
    QString refPart = text;
    if (bang > 0) {
        QString sheetName = text.left(bang);
        if (sheetName.startsWith(QLatin1Char('\'')) && sheetName.endsWith(QLatin1Char('\'')) && sheetName.size() > 1) sheetName = sheetName.mid(1, sheetName.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
        *sheet = -1;
        const auto &sheets = m_book->book->info().sheets;
        for (int s = 0; s < sheets.size(); ++s)
            if (!sheets[s].deleted && sheets[s].kind == SheetKind::Worksheet && sheets[s].name.compare(sheetName, Qt::CaseInsensitive) == 0) *sheet = s;
        if (*sheet < 0) {
            *why = tr("There is no sheet called \"%1\".").arg(sheetName);
            return false;
        }
        refPart = text.mid(bang + 1);
    }
    refPart.remove(QLatin1Char('$'));
    // whole columns (A:F) or whole rows (1:50): the cells they hold, from the top (or the left), as far as the sheet is used
    {
        static const QRegularExpression wholeColumns(QStringLiteral("^([A-Za-z]{1,3}):([A-Za-z]{1,3})$")), wholeRows(QStringLiteral("^([0-9]{1,7}):([0-9]{1,7})$"));
        quint32 usedRows = 0, usedCols = 0;
        if (*sheet >= 0 && *sheet < m_book->calc->sheetCount()) m_book->calc->usedExtent(*sheet, &usedRows, &usedCols);
        CellRange whole;
        if (const auto m = wholeColumns.match(refPart); m.hasMatch() && parseRangeRef(m.captured(1) + QStringLiteral("1:") + m.captured(2) + QStringLiteral("1"), &whole)) {
            if (usedRows == 0) {
                *why = tr("There is nothing in those columns.");
                return false;
            }
            whole.row1 = 0;
            whole.row2 = usedRows - 1;
            *range = whole;
            return true;
        }
        if (const auto m = wholeRows.match(refPart); m.hasMatch() && parseRangeRef(QStringLiteral("A") + m.captured(1) + QLatin1Char(':') + QStringLiteral("A") + m.captured(2), &whole)) {
            if (usedCols == 0) {
                *why = tr("There is nothing in those rows.");
                return false;
            }
            whole.col1 = 0;
            whole.col2 = usedCols - 1;
            *range = whole;
            if (range->row1 == range->row2) {
                *why = tr("The data needs a row of names and at least one row of data under it. Type more than one row, for example Sheet1!A1:D100.");
                return false;
            }
            return true;
        }
    }
    if (parseRangeRef(refPart, range) && range->row1 == range->row2) {
        *why = tr("The data needs a row of names and at least one row of data under it. Type more than one row, for example Sheet1!A1:D100.");
        return false;
    }
    if (!parseRangeRef(refPart, range)) {
        // a table's name, or a name
        QString found;
        for (const Table &t : m_book->book->tables())
            if (t.name.compare(text, Qt::CaseInsensitive) == 0) found = t.name;
        if (found.isEmpty())
            for (const DefinedName &n : m_book->book->info().definedNames)
                if (n.name.compare(text, Qt::CaseInsensitive) == 0) found = n.name;
        if (found.isEmpty()) {
            *why = tr("\"%1\" is not a range of cells, a table, or a name. Type the cells the data is in, with its first row of names, for example Sheet1!A1:D100.").arg(text);
            return false;
        }
        *name = found;
    }
    return true;
}

bool EditSession::pivotSourceRange(int cache, int *sheet, CellRange *range, QString *why, const SourceOver *over) const
{
    PivotBook &pb = m_book->book->pivots();
    PivotCacheEntry c = pb.cache(cache);
    if (over && over->cache == cache) {
        c.source = over->range;
        c.sourceSheet = over->sheet;
        c.sourceName = over->name;
        c.external = false;
    }
    if (c.external) {
        if (why) *why = tr("The data of this pivot table is in another workbook, which JeffCrunch does not open, so it can't be read again here. The table keeps the values the file has. To read it from this workbook, use Change Data Source.");
        return false;
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
        if (pivotNameRange(c.sourceName, sheet, range)) return true;
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
        const quint32 originRow = ch.moveRow >= 0 ? quint32(ch.moveRow) : oldLoc.row1, originCol = ch.moveCol >= 0 ? quint32(ch.moveCol) : oldLoc.col1;
        const PivotTable &old = *entry.table;
        const CellRange oldPage = pageArea(old, oldLoc);
        PivotOptions options;
        // (the format a number shows in is the one of the cell above that shows that very number; looked for once for each number in a column, and no farther than 512 lines up: past that, or with no
        // such cell, it is the field's own)
        auto formats = std::make_shared<std::map<std::pair<int, double>, int>>();
        // (and the looking is bounded as a whole, so that many different numbers over many lines don't each look a long way up: past the budget a number has the field's own format)
        auto budget = std::make_shared<int>(20000);
        options.format = [&, formats, budget](double v, int id, int gridRow, int gridCol) {
            const auto key = std::make_pair(gridCol, v);
            auto known = formats->find(key);
            int use = id;
            if (known != formats->end()) {
                use = known->second;
            } else {
                int found = id;
                const qint64 top = std::max<qint64>(qint64(oldLoc.row1), qint64(oldLoc.row1) + gridRow - 512);
                for (qint64 r = qint64(oldLoc.row1) + gridRow; r >= top && *budget > 0; --r) {
                    --*budget;
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
        const int pages = int(ch.now->pageFields.size());
        if (pages > 0 && originRow < quint32(pages) + 1) return tr("There is no room above the pivot table for its filters. Move the table down first.");
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
        pivotApplyGrid(ch.now.get(), grid, originRow, originCol);
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
            if (ch.cellsMoved) return true;   // (a move of the table moved its cells there already, after looking at what was in the way)
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
                pivotApplyGrid(ch.now.get(), grid, originRow, originCol);
                parseRangeRef(ch.now->ref, &newLoc);
            }
        }
        placed.push_back({ch.table, newLoc});
        if (qint64(newLoc.cellCount()) > kMaxCellsPerEdit * 40) return tr("The pivot table would be larger than JeffCrunch changes at once.");

        // the cells
        bool formatChosen = false;
        for (int i = 0; i < ch.now->dataFields.size(); ++i) {
            const PivotDataField &nowField = ch.now->dataFields[i];
            if (!nowField.numFmtCode.isEmpty()) formatChosen = true;
            else if (i < old.dataFields.size() && old.dataFields[i].numFmtId != nowField.numFmtId) formatChosen = true;
        }
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
            // (a value's number format that was just chosen is the cells' own, whatever they had)
            QString forced;
            if (pc.isValue && formatChosen) {
                if (pc.numFmtId <= -2 && -2 - pc.numFmtId < ch.now->dataFields.size()) forced = ch.now->dataFields[-2 - pc.numFmtId].numFmtCode;
                else if (pc.numFmtId > 0) forced = codeOfFormat(book.styles(), pc.numFmtId);
            }
            const bool sameValue = !cc.before.isFormula && cc.before.exists && cc.before.value == v;
            if (sameValue && forced.isEmpty()) return;
            if (sameValue) {
                if (codeOfFormat(book.styles(), book.styles().numFmtId(cc.before.style)) == forced || (forced == QLatin1String("General") && book.styles().numFmtId(cc.before.style) == 0)) return;
                cc.formatCode = forced;
                cc.styleOnly = true;
                step.cells.push_back(std::move(cc));
                return;
            }
            cc.after.exists = true;
            cc.after.isFormula = false;
            cc.after.formula.clear();
            cc.after.value = v;
            cc.after.sharedString = -1;
            cc.after.hasStored = false;
            if (pc.kind == PivotCell::Text) cc.textToShare = !book.sharedStringsPart().isEmpty();
            // (a cell that has no format of its own gets the one the table gives the values: a person's own format is kept)
            if (!forced.isEmpty()) {
                cc.formatCode = forced;
            } else if (pc.numFmtId > 0 && (!cc.before.exists || cc.before.style == 0)) {
                const QString code = codeOfFormat(book.styles(), pc.numFmtId);
                if (!code.isEmpty() && code != QLatin1String("General")) cc.formatCode = code;
            } else if (pc.numFmtId <= -2 && -2 - pc.numFmtId < ch.now->dataFields.size() && (!cc.before.exists || cc.before.style == 0)) {
                cc.formatCode = ch.now->dataFields[-2 - pc.numFmtId].numFmtCode;
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
        for (const PivotDataField &df : ch.now->dataFields)
            if (!df.numFmtCode.isEmpty()) pe.formatCodes = true;
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

EditSession::Outcome EditSession::createPivot(const QString &source, int sheet, quint32 row, quint32 col, int *made)
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
    // the data
    int srcSheet = -1;
    CellRange range;
    QString name;
    if (!parsePivotSource(source, sheet >= 0 ? sheet : 0, &srcSheet, &range, &name, &why)) return refuse(why);
    if (!name.isEmpty() && !pivotNameRange(name, &srcSheet, &range)) return refuse(tr("JeffCrunch can't find the cells \"%1\" stands for.").arg(name));
    const auto &sheets = book.info().sheets;
    if (srcSheet < 0 || srcSheet >= sheets.size() || sheets[srcSheet].deleted || sheets[srcSheet].kind != SheetKind::Worksheet) return refuse(tr("The data must be on a sheet of cells."));
    if (range.row2 <= range.row1) return refuse(tr("The data needs a row of names and at least one row of data under it."));
    if (range.col2 - range.col1 + 1 > quint32(kPivotMaxFields) || range.row2 - range.row1 > quint32(kPivotMaxRecords)) return refuse(tr("There is too much data for a pivot table."));
    // (the cells are read on this thread, in about a second for each million: a bound keeps a mistake from holding the window for long)
    if (qint64(range.row2 - range.row1) * qint64(range.col2 - range.col1 + 1) > kPivotMaxCreateCells) return refuse(tr("There is too much data to make a pivot table of in one step (more than %1 million cells). Use fewer rows or columns.").arg(kPivotMaxCreateCells / 1000000));
    PivotSource ps;
    fillPivotSource(&ps, srcSheet, range);
    auto cache = std::make_shared<PivotCache>();
    if (!buildPivotCache(ps, PivotCache(), cache.get(), &why)) return refuse(tr("The data can't be read: %1.").arg(why));
    cache->sourceType = QStringLiteral("worksheet");
    // where it goes: a cell of a sheet (checked now), or a sheet of its own (made last: a refusal leaves no sheet behind)
    int target = sheet;
    if (target < 0) {
        row = 2;
        col = 0;
    } else if (target >= sheets.size() || sheets[target].deleted || sheets[target].kind != SheetKind::Worksheet) {
        return refuse(tr("Pick a sheet of cells for the pivot table."));
    } else if (!canEditSheet(target, &why) || !allows(target, Allow::Pivot, &why)) {
        return refuse(why);
    }
    // the cells the empty table covers (three columns of eighteen lines, as the original program makes it): free of other tables, of cells that hold anything, and of the data
    CellRange area;
    area.valid = true;
    area.row1 = row;
    area.col1 = col;
    area.row2 = row + 17;
    area.col2 = col + 2;
    if (area.row2 >= 1048576 || area.col2 >= 16384) return refuse(tr("There is no room for a pivot table there."));
    if (target >= 0) {
        if (target == srcSheet && !(area.row2 < range.row1 || area.row1 > range.row2 || area.col2 < range.col1 || area.col1 > range.col2))
            return refuse(tr("The pivot table would cover some of the data it is made from (%1). Pick a place that is clear of the data.").arg(rangeName(range)));
        for (int t = 0; t < pb.tableCount(); ++t) {
            if (pb.table(t).sheet != target) continue;
            const CellRange other = pb.locationOf(t);
            if (!other.valid || (other.row2 < area.row1 || other.row1 > area.row2 || other.col2 < area.col1 || other.col1 > area.col2)) continue;
            return refuse(tr("There is a pivot table there already. Pick a cell that is clear of it."));
        }
        if (!cellsEmpty(target, area) || !tableFree(target, area, -1, &why)) return refuse(why.isEmpty() ? tr("There is something in the cells a pivot table would cover (%1). Pick a clear place.").arg(rangeName(area)) : why);
    }
    // a name no table of the workbook has
    QSet<QString> taken;
    for (int t = 0; t < pb.tableCount(); ++t)
        if (pb.table(t).table) taken.insert(pb.table(t).table->name.toLower());
    QString tableName;
    for (int n = 1;; ++n) {
        tableName = tr("Pivot Table %1").arg(n);
        if (!taken.contains(tableName.toLower())) break;
    }
    // the table: made as the original program makes an empty one (in the compact form, the style it gives), and read back so that it is the model's
    QString fields;
    for (int f = 0; f < cache->fields.size(); ++f) fields += QStringLiteral("<pivotField showAll=\"0\"/>");
    const QString ns = QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main");
    const QString tableXml = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotTableDefinition xmlns=\"%1\" name=\"%2\" cacheId=\"@CACHEID@\" applyNumberFormats=\"0\" applyBorderFormats=\"0\" "
                                            "applyFontFormats=\"0\" applyPatternFormats=\"0\" applyAlignmentFormats=\"0\" applyWidthHeightFormats=\"1\" dataCaption=\"Values\" updatedVersion=\"6\" minRefreshableVersion=\"3\" "
                                            "useAutoFormatting=\"1\" itemPrintTitles=\"1\" createdVersion=\"6\" indent=\"0\" outline=\"1\" outlineData=\"1\" multipleFieldFilters=\"0\"><location ref=\"%3\" firstHeaderRow=\"1\" "
                                            "firstDataRow=\"1\" firstDataCol=\"1\"/><pivotFields count=\"%4\">%5</pivotFields><pivotTableStyleInfo name=\"PivotStyleLight16\" showRowHeaders=\"1\" showColHeaders=\"1\" "
                                            "showRowStripes=\"0\" showColStripes=\"0\" showLastColumn=\"1\"/></pivotTableDefinition>")
                                .arg(ns, tableName, rangeName(area)).arg(cache->fields.size()).arg(fields);
    auto table = std::make_shared<PivotTable>();
    {
        QByteArray readable = tableXml.toUtf8();
        readable.replace("@CACHEID@", "0");
        if (!readPivotTable(readable, table.get(), &why)) return refuse(tr("The pivot table could not be made: %1.").arg(why));
    }
    const QString definitionXml = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotCacheDefinition xmlns=\"%1\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                                                  "r:id=\"rId1\" refreshedBy=\"JeffCrunch\" refreshedDate=\"0\" createdVersion=\"6\" refreshedVersion=\"6\" minRefreshableVersion=\"3\" recordCount=\"0\"><cacheSource type=\"worksheet\">"
                                                  "<worksheetSource ref=\"A1:A2\" sheet=\"Sheet1\"/></cacheSource><cacheFields count=\"0\"/></pivotCacheDefinition>")
                                      .arg(ns);
    // nothing is refused after this: the sheet of its own is made, and the table put on it
    if (target < 0) {
        QString newName;
        for (int n = 1;; ++n) {
            newName = tr("Pivot Table %1").arg(n);
            bool taken = false;
            for (const SheetEntry &se : sheets)
                if (!se.deleted && se.name.compare(newName, Qt::CaseInsensitive) == 0) taken = true;
            if (!taken) break;
        }
        const Outcome inserted = insertSheet(int(sheets.size()), newName, &target);
        if (!inserted.ok) return inserted;
    }
    Step step;
    PivotEdit pe;
    pe.created = true;
    pe.newCache = true;
    pe.cache = pb.cacheCount();
    pe.table = pb.tableCount();
    pe.sheet = target;
    pe.tableAfter = table;
    pe.cacheAfter = cache;
    pe.sourceAfter = name.isEmpty() ? range : CellRange();
    pe.sheetAfter = name.isEmpty() ? srcSheet : -1;
    pe.nameAfter = name;
    pe.tableCreated = tableXml.toUtf8();
    pe.cacheCreated = definitionXml.toUtf8();
    step.pivots.push_back(std::move(pe));
    const int index = pb.tableCount();
    out = pushTableStep(std::move(step), tr("Create Pivot Table"));
    if (out.ok && made) *made = index;
    return out;
}

QString EditSession::pivotDataFormula(int sheet, quint32 row, quint32 col, int formulaSheet) const
{
    if (!m_book || !m_book->book) return QString();
    PivotBook::View view;
    if (!m_book->book->pivots().viewAt(sheet, row, col, &view) || !view.where.valid || row < view.where.row1 || col < view.where.col1) return QString();
    QString data;
    std::vector<std::pair<QString, PivotValue>> items;
    if (!pivotGetDataArguments(*view.table, *view.cache, *view.grid, int(row - view.where.row1), int(col - view.where.col1), &data, &items)) return QString();
    auto quoted = [](const QString &t) { return QStringLiteral("\"") + QString(t).replace(QLatin1Char('"'), QLatin1String("\"\"")) + QStringLiteral("\""); };
    QString text = QStringLiteral("=GETPIVOTDATA(") + quoted(data) + QLatin1Char(',');
    if (sheet != formulaSheet) text += sheetNamePrefix(m_book->book->info().sheets[sheet].name);
    text += QStringLiteral("$") + columnName(view.where.col1) + QStringLiteral("$") + QString::number(view.where.row1 + 1);
    for (const auto &[field, item] : items) {
        text += QLatin1Char(',') + quoted(field) + QLatin1Char(',');
        switch (item.kind) {
        case PivotValue::Number:
        case PivotValue::Date: text += QString::number(item.number, 'g', 15); break;
        case PivotValue::Bool: text += item.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE"); break;
        case PivotValue::Missing: text += quoted(QStringLiteral("(blank)")); break;
        default: text += quoted(item.text); break;
        }
    }
    // (as typed in the person's locale: a semicolon between the arguments and a comma in a number where it writes them so)
    return formulaToLocale(text + QLatin1Char(')'), locale());
}

bool EditSession::cellsHoldAnything(int sheet, quint32 row, quint32 col) const
{
    const Cell *c = m_book->calc->sheet(sheet).cells.find(row, col);
    return c && (c->hasFormula() || c->type != CellType::Empty);
}

void EditSession::applyPivotEdits(Step *step, bool forward, Workbook &book)
{
    if (step->pivots.empty()) return;
    PivotBook &pb = book.pivots();
    for (PivotEdit &e : step->pivots) {
        if (forward && e.formatCodes && e.tableAfter) {
            // (the formats the values were given by their codes get their ids now, with the styles held still)
            auto resolved = std::make_shared<PivotTable>(*e.tableAfter);
            for (PivotDataField &df : resolved->dataFields)
                if (!df.numFmtCode.isEmpty()) {
                    df.numFmtId = int(book.numberFormatId(df.numFmtCode));
                    df.numFmtCode.clear();
                }
            e.tableAfter = resolved;
            e.formatCodes = false;
        }
        if (e.created) {
            if (forward) {
                if (e.newCache) pb.addCache(e.cacheAfter, e.sourceAfter, e.sheetAfter, e.nameAfter, e.cacheCreated, e.sheetAfter >= 0 && e.sheetAfter < book.info().sheets.size() ? book.info().sheets[e.sheetAfter].name : QString());
                pb.addTable(e.sheet, e.tableAfter, e.cache, e.tableCreated);
            } else {
                pb.removeLastTable();
                if (e.newCache) pb.removeLastCache();
            }
            // (the cells the table covers read differently now: a GETPIVOTDATA that names them is worked out again, whether the table came or went)
            CellRange at;
            if (e.tableAfter && parseRangeRef(e.tableAfter->ref, &at) && m_book->calc && e.sheet >= 0 && e.sheet < m_book->calc->sheetCount()) {
                Calculation::BulkEdit bulk(m_book->calc.get());
                for (quint32 r = at.row1; r <= at.row2; ++r)
                    for (quint32 c = at.col1; c <= at.col2; ++c)
                        if (!cellsHoldAnything(e.sheet, r, c)) m_book->calc->setValue(e.sheet, r, c, Value(), Calculation::Marking::Now);
            }
            continue;
        }
        if (e.cache >= 0) {
            if (forward) pb.setCache(e.cache, e.cacheAfter, e.sourceAfter, e.sheetAfter, e.nameAfter);
            else pb.setCache(e.cache, e.cacheBefore, e.sourceBefore, e.sheetBefore, e.nameBefore);
        }
        pb.setTable(e.table, forward ? e.tableAfter : e.tableBefore, forward ? e.structureAfter : e.structureBefore);
    }
}

void EditSession::fillPivotSource(PivotSource *ps, int sheet, const CellRange &range)
{
    const Workbook &book = *m_book->book;
    ps->row1 = range.row1;
    ps->col1 = range.col1;
    ps->row2 = range.row2;
    ps->col2 = range.col2;
    ps->dates = book.info().dateSystem;
    auto dateStyle = std::make_shared<QHash<quint32, bool>>();
    const Sheet *src = &m_book->calc->sheet(sheet);
    ps->cell = [this, sheet, dateStyle, src](quint32 r, quint32 c) {
        PivotValue pv;
        const Value v = valueAt(sheet, r, c);
        switch (v.kind()) {
        case Value::Kind::Empty: break;
        case Value::Kind::Number: {
            const Cell *cell = src->cells.find(r, c);
            bool date = false;
            if (cell) {
                auto it = dateStyle->find(cell->style);
                if (it == dateStyle->end()) it = dateStyle->insert(cell->style, formatOfStyle(cell->style).kind() == jp::calc::FormatKind::Date);
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
    // (the format of the first cell of data of each column is the field's)
    for (quint32 c = range.col1; c <= range.col2; ++c) {
        const Calculation::CellContent first = m_book->calc->content(sheet, range.row1 + 1, c);
        ps->numFmtIds.append(first.exists ? int(book.styles().numFmtId(first.style)) : 0);
        if (range.col2 - range.col1 > quint32(kPivotMaxFields)) break;
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
        fillPivotSource(&ps, sheet, range);
        for (int t : tables) {
            const PivotTableEntry &te = pb.table(t);
            if (!te.table) return tr("JeffCrunch can't read a pivot table made from this data.");
            for (int f : te.table->rowFields) if (f >= 0) ps.axisFields.insert(f);
            for (int f : te.table->colFields) if (f >= 0) ps.axisFields.insert(f);
            for (const PivotPageField &p : te.table->pageFields) ps.axisFields.insert(p.fld);
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

bool EditSession::isPivotValue(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->book) return false;
    PivotBook::View view;
    if (!m_book->book->pivots().viewAt(sheet, row, col, &view) || !view.where.valid || row < view.where.row1 || col < view.where.col1 || !view.grid) return false;
    const int r = int(row - view.where.row1), c = int(col - view.where.col1);
    return r < view.grid->rows && c < view.grid->cols && view.grid->at(r, c).isValue && view.grid->at(r, c).kind != PivotCell::Empty;
}

EditSession::Outcome EditSession::showPivotDetail(int sheet, quint32 row, quint32 col, int *made)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &m) {
        out.ok = false;
        out.problem = m;
        return out;
    };
    if (!canEdit(&why)) return refuse(why);
    PivotBook::View view;
    if (!m_book->book->pivots().viewAt(sheet, row, col, &view) || !view.where.valid || !view.table || !view.cache || !view.grid) return refuse(tr("JeffCrunch can't read this pivot table."));
    const int gr = int(row - view.where.row1), gc = int(col - view.where.col1);
    QVector<qint64> records;
    // (the most rows one step shows, from the fields that have values of their own: the scan stops there)
    qint64 shownFields = 0;
    for (const PivotCacheField &cf : view.cache->fields)
        if (cf.group.databaseField) ++shownFields;
    const qint64 limit = shownFields > 0 ? kMaxCellsPerEdit * 40 / shownFields - 1 : 0;
    bool tooMany = false;
    if (row < view.where.row1 || col < view.where.col1 || !pivotDetailRecords(*view.table, *view.cache, *view.grid, gr, gc, &records, limit, &tooMany)) return refuse(tr("Click a value of the pivot table, not a label."));
    if (tooMany) return refuse(tr("There are too many rows behind this value to show at once."));
    const PivotCache &cache = *view.cache;
    const int nf = int(cache.fields.size());
    // (the fields that have values of their own: not the group of another field)
    QVector<int> shown;
    for (int f = 0; f < nf; ++f)
        if (cache.fields[f].group.databaseField) shown.append(f);
    if (records.isEmpty()) return refuse(tr("There are no rows behind this value."));
    const auto &sheets = m_book->book->info().sheets;
    QString name;
    for (int n = 1;; ++n) {
        name = tr("Detail %1").arg(n);
        bool taken = false;
        for (const SheetEntry &se : sheets)
            if (!se.deleted && se.name.compare(name, Qt::CaseInsensitive) == 0) taken = true;
        if (!taken) break;
    }
    int target = -1;
    const Outcome inserted = insertSheet(int(sheets.size()), name, &target);
    if (!inserted.ok) return inserted;
    Step step;
    auto put = [&](quint32 r, quint32 c, const Value &v, bool text, const QString &format) {
        CellChange cc;
        cc.sheet = target;
        cc.row = r;
        cc.col = quint16(c);
        cc.before = m_book->calc->content(target, r, c);
        cc.after = cc.before;
        cc.after.exists = true;
        cc.after.isFormula = false;
        cc.after.formula.clear();
        cc.after.value = v;
        cc.after.sharedString = -1;
        cc.after.hasStored = false;
        cc.textToShare = text && !m_book->book->sharedStringsPart().isEmpty();
        cc.formatCode = format;
        step.cells.push_back(std::move(cc));
    };
    for (int k = 0; k < shown.size(); ++k) put(0, quint32(k), Value::text(cache.fields[shown[k]].name), true, QString());
    for (qsizetype i = 0; i < records.size(); ++i)
        for (int k = 0; k < shown.size(); ++k) {
            const PivotCacheField &f = cache.fields[shown[k]];
            const PivotValue &v = f.storedItems()[f.storedRecords()[records[i]]];
            const quint32 r = quint32(i) + 1, c = quint32(k);
            switch (v.kind) {
            case PivotValue::Missing: break;
            case PivotValue::Number: put(r, c, Value::number(v.number), false, QString()); break;
            case PivotValue::Date: put(r, c, Value::number(v.number), false, QStringLiteral("m/d/yyyy")); break;
            case PivotValue::Bool: put(r, c, Value::boolean(v.number != 0), false, QString()); break;
            case PivotValue::Error: put(r, c, Value::error(jp::calc::errorKindFromText(v.text)), false, QString()); break;
            case PivotValue::Text: put(r, c, Value::text(v.text), true, QString()); break;
            }
        }
    out = pushTableStep(std::move(step), tr("Show Details"));
    if (out.ok && made) *made = target;
    return out;
}

EditSession::Outcome EditSession::listPivotFormulas(int table, int *made)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &m) {
        out.ok = false;
        out.problem = m;
        return out;
    };
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) return refuse(why.isEmpty() ? tr("There is no pivot table here.") : why);
    PivotBook &pb = m_book->book->pivots();
    const int ci = pb.table(table).cache;
    const PivotCache &cache = *pb.cache(ci).cache;
    QVector<int> calc;
    for (int f = 0; f < cache.fields.size(); ++f)
        if (cache.fields[f].isCalculated()) calc.append(f);
    if (calc.isEmpty()) return refuse(tr("This pivot table has no calculated fields."));
    const auto &sheets = m_book->book->info().sheets;
    QString name;
    for (int n = 1;; ++n) {
        name = tr("Formulas %1").arg(n);
        bool taken = false;
        for (const SheetEntry &se : sheets)
            if (!se.deleted && se.name.compare(name, Qt::CaseInsensitive) == 0) taken = true;
        if (!taken) break;
    }
    int target = -1;
    const Outcome inserted = insertSheet(int(sheets.size()), name, &target);
    if (!inserted.ok) return inserted;
    Step step;
    auto put = [&](quint32 r, quint32 c, const QString &text) {
        CellChange cc;
        cc.sheet = target;
        cc.row = r;
        cc.col = quint16(c);
        cc.before = m_book->calc->content(target, r, c);
        cc.after = cc.before;
        cc.after.exists = true;
        cc.after.isFormula = false;
        cc.after.formula.clear();
        cc.after.value = Value::text(text);
        cc.after.sharedString = -1;
        cc.after.hasStored = false;
        cc.textToShare = !m_book->book->sharedStringsPart().isEmpty();
        step.cells.push_back(std::move(cc));
    };
    put(0, 0, tr("Calculated Field"));
    put(0, 1, tr("Formula"));
    for (int k = 0; k < calc.size(); ++k) {
        put(quint32(k) + 1, 0, cache.fields[calc[k]].name);
        put(quint32(k) + 1, 1, QLatin1Char('=') + cache.fields[calc[k]].formula);
    }
    out = pushTableStep(std::move(step), tr("List Formulas"));
    if (out.ok && made) *made = target;
    return out;
}

// The words of an item of a cache field, as a sheet name or a comparison between two caches needs them.
static QString pivotItemWords(const PivotCacheField &f, int x)
{
    const QVector<PivotValue> &items = f.storedItems();
    if (x < 0 || x >= items.size()) return QString();
    const PivotValue &v = items[x];
    switch (v.kind) {
    case PivotValue::Text: return v.text;
    case PivotValue::Number: return QString::number(v.number, 'g', 15);
    case PivotValue::Bool: return v.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case PivotValue::Date: return QDate(1899, 12, 30).addDays(int(v.number)).toString(Qt::ISODate);
    case PivotValue::Error: return v.text;
    case PivotValue::Missing: break;
    }
    return QString();
}

EditSession::Outcome EditSession::showReportFilterPages(int table, int pageField, int *firstSheet)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &m) {
        out.ok = false;
        out.problem = m;
        return out;
    };
    if (!canEdit(&why) || !m_book || !m_book->book || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) return refuse(why.isEmpty() ? tr("There is no pivot table here.") : why);
    PivotBook &pb = m_book->book->pivots();
    const std::shared_ptr<const PivotTable> original = pb.table(table).table;
    const int originalCache = pb.table(table).cache;
    if (!original || originalCache < 0 || pageField < 0 || pageField >= original->pageFields.size()) return refuse(tr("This pivot table has no filter to show pages for."));
    const std::shared_ptr<const PivotCache> cache = pb.cache(originalCache).cache;
    const int fld = original->pageFields[pageField].fld;
    if (!cache || fld < 0 || fld >= original->fields.size() || fld >= cache->fields.size()) return refuse(tr("This pivot table has no filter to show pages for."));
    const CellRange at = pb.locationOf(table);
    const QString source = pivotSourceText(table);
    if (!at.valid || source.isEmpty()) return refuse(tr("JeffCrunch can't find where this pivot table is, or the data it is made from."));
    QStringList labels;
    for (const PivotTableItem &it : original->fields[fld].items)
        if (it.t.isEmpty() && it.x >= 0 && !it.hidden) labels << pivotItemWords(cache->fields[fld], it.x);
    if (labels.isEmpty()) return refuse(tr("The filter has no items to make pages of."));
    if (labels.size() > 100) return refuse(tr("The filter has %1 items. Pages are made for at most 100.").arg(labels.size()));
    const int startIndex = m_undo.index();
    m_undo.beginMacro(tr("Show Report Filter Pages"));
    Outcome step;
    int first = -1;
    for (const QString &label : std::as_const(labels)) {
        // a sheet name that is allowed (up to 31 letters, none of the marks a sheet name cannot hold) and not taken
        QString name = label.trimmed();
        for (QChar &c : name)
            if (QStringLiteral("[]:*?/\\").contains(c)) c = QLatin1Char('_');
        name = name.left(31).trimmed();
        if (name.isEmpty()) name = tr("Blank");
        const auto &sheets = m_book->book->info().sheets;
        QString unique = name;
        for (int n = 2;; ++n) {
            bool taken = false;
            for (const SheetEntry &se : sheets)
                if (!se.deleted && se.name.compare(unique, Qt::CaseInsensitive) == 0) taken = true;
            if (!taken) break;
            const QString tail = QStringLiteral(" (%1)").arg(n);
            unique = name.left(31 - tail.size()) + tail;
        }
        int target = -1;
        step = insertSheet(int(sheets.size()), unique, &target);
        if (!step.ok) break;
        if (first < 0) first = target;
        int made = -1;
        step = createPivot(source, target, at.row1, at.col1, &made);
        if (!step.ok) break;
        // (the copy has the roles, the functions, and the look of the table; its items are those of its own cache, so the filter's item and the hidden ones are found there by their words)
        PivotTable copy = *original;
        copy.name = pb.table(made).table->name;
        for (PivotField &pf : copy.fields) {
            pf.items.clear();
            pf.hasItems = false;
        }
        step = setPivot(made, copy, true, tr("Show Report Filter Pages"));
        if (!step.ok) break;
        const PivotTable &built = *pb.table(made).table;
        const PivotCache &newCache = *pb.cache(pb.table(made).cache).cache;
        PivotTable again = built;
        auto positionOf = [&](int field, const QString &words) {
            for (int k = 0; k < again.fields[field].items.size(); ++k) {
                const PivotTableItem &it = again.fields[field].items[k];
                if (it.t.isEmpty() && it.x >= 0 && pivotItemWords(newCache.fields[field], it.x) == words) return k;
            }
            return -1;
        };
        const int place = fld < again.fields.size() ? positionOf(fld, label) : -1;
        if (place < 0) {
            step.ok = false;
            step.problem = tr("The item \"%1\" was not found in the copy of the data.").arg(label);
            break;
        }
        for (PivotPageField &p : again.pageFields)
            if (p.fld == original->pageFields[pageField].fld) p.item = place;
        for (int field = 0; field < original->fields.size() && field < again.fields.size(); ++field)
            for (const PivotTableItem &it : original->fields[field].items) {
                if (!it.hidden || it.x < 0) continue;
                const int k = positionOf(field, pivotItemWords(cache->fields[field], it.x));
                if (k >= 0) again.fields[field].items[k].hidden = true;
            }
        step = setPivot(made, again, false, tr("Show Report Filter Pages"));
        if (!step.ok) break;
    }
    m_undo.endMacro();
    if (!step.ok) {
        while (m_undo.index() > startIndex) m_undo.undo();
        return step;
    }
    if (firstSheet) *firstSheet = first;
    return out;
}

void EditSession::setRefreshOnOpenLimit(qint64 cells)
{
    gRefreshOnOpenLimit = cells;
}

EditSession::Outcome EditSession::refreshPivotsOnOpen()
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !m_book || !m_book->book) return out;
    PivotBook &pb = m_book->book->pivots();
    std::vector<int> caches;
    qint64 cells = 0;
    int left = 0;
    for (int t = 0; t < pb.tableCount(); ++t) {
        const int ci = pb.table(t).cache;
        if (ci < 0 || !pb.cache(ci).cache || !pb.cache(ci).cache->refreshOnLoad || !pivotUsable(t)) continue;
        if (std::find(caches.begin(), caches.end(), ci) != caches.end()) continue;
        const PivotCache &pc = *pb.cache(ci).cache;
        const qint64 size = pc.fields.isEmpty() ? 0 : qint64(pc.fields[0].records.size()) * pc.fields.size();
        if (int(caches.size()) >= kMaxPivotTablesRefreshed || cells + size > (gRefreshOnOpenLimit > 0 ? gRefreshOnOpenLimit : kMaxPivotCellsRefreshedOnOpen)) {
            ++left;
            continue;
        }
        cells += size;
        caches.push_back(ci);
    }
    if (caches.empty() && left == 0) return out;
    if (!caches.empty()) out = refreshPivotCaches(caches, tr("Refresh"), nullptr, true);
    if (left > 0) {
        const QString note = tr("%n pivot table(s) were not read again when the file opened: there is too much data. Use Refresh All.", nullptr, left);
        out.problem = out.problem.isEmpty() ? note : out.problem + QLatin1Char('\n') + note;
    }
    return out;
}

EditSession::Outcome EditSession::groupPivotField(int table, int field, const QStringList &levels, double start, double end, double step, bool autoStart, bool autoEnd)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &m) {
        out.ok = false;
        out.problem = m;
        return out;
    };
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) return refuse(why.isEmpty() ? tr("There is no pivot table here.") : why);
    PivotBook &pb = m_book->book->pivots();
    const int ci = pb.table(table).cache;
    const PivotCacheEntry &ce = pb.cache(ci);
    if (pb.tablesOfCache(ci).size() > 1) return refuse(tr("Another pivot table is made from the same data. A group is a part of the data, so it can't be made while two tables share it."));
    const PivotTable &old = *pb.table(table).table;
    if (field < 0 || field >= old.fields.size() || field >= ce.cache->fields.size()) return refuse(tr("There is no such field."));
    if (!old.rowFields.contains(field) && !old.colFields.contains(field)) return refuse(tr("Put the field in the rows or the columns first."));
    auto cache = std::make_shared<PivotCache>(*ce.cache);
    QVector<int> added;
    if (!pivotGroupField(cache.get(), field, levels, start, end, step, autoStart, autoEnd, m_book->book->info().dateSystem, &added, &why))
        return refuse(tr("These can't be grouped: %1.").arg(why));
    PivotChange ch;
    ch.table = table;
    ch.now = std::make_shared<PivotTable>(old);
    PivotTable &t = *ch.now;
    // (the fields added to the cache are added to the table, off its axes; a field whose values are now groups lists its items again)
    while (t.fields.size() < cache->fields.size()) t.fields.append(PivotField());
    t.fields[field].items.clear();
    t.fields[field].hasItems = false;
    t.fields[field].sortByData = -1;
    QVector<int> &axis = t.rowFields.contains(field) ? t.rowFields : t.colFields;
    int at = int(axis.indexOf(field));
    for (int k = 1; k < added.size(); ++k) {   // (the coarsest goes first)
        axis.insert(at, added[k]);
    }
    for (int k = 1; k < added.size(); ++k) t.fields[added[k]].axis = QString();
    pivotSyncRoles(&t);
    pivotNormalizeLayout(&t);
    pivotPrepareFields(&t, cache.get());
    ch.structure = true;
    ch.cache = ci;
    ch.cacheNow = cache;
    ch.source = ce.source;
    ch.sourceSheet = ce.sourceSheet;
    ch.sourceName = ce.sourceName;
    return pivotStep({ch}, tr("Group Field"));
}

EditSession::Outcome EditSession::ungroupPivotField(int table, int field)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &m) {
        out.ok = false;
        out.problem = m;
        return out;
    };
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) return refuse(why.isEmpty() ? tr("There is no pivot table here.") : why);
    PivotBook &pb = m_book->book->pivots();
    const int ci = pb.table(table).cache;
    const PivotCacheEntry &ce = pb.cache(ci);
    if (pb.tablesOfCache(ci).size() > 1) return refuse(tr("Another pivot table is made from the same data, so the group can't be taken away from this one alone."));
    if (field < 0 || field >= ce.cache->fields.size() || ce.cache->fields[field].group.kind == PivotFieldGroup::None) return refuse(tr("This field is not grouped."));
    auto cache = std::make_shared<PivotCache>(*ce.cache);
    const int base = cache->fields[field].group.base >= 0 ? cache->fields[field].group.base : field;
    QVector<int> removed;
    if (!pivotUngroupField(cache.get(), field, &removed)) return refuse(tr("This field is not grouped."));
    PivotChange ch;
    ch.table = table;
    ch.now = std::make_shared<PivotTable>(*pb.table(table).table);
    PivotTable &t = *ch.now;
    pivotRemoveFields(&t, removed);
    int placed = base;
    for (int r : removed)
        if (r < base) --placed;
    if (placed >= 0 && placed < t.fields.size()) {
        t.fields[placed].items.clear();
        t.fields[placed].hasItems = false;
        t.fields[placed].sortByData = -1;
    }
    pivotSyncRoles(&t);
    pivotNormalizeLayout(&t);
    pivotPrepareFields(&t, cache.get());
    ch.structure = true;
    ch.cache = ci;
    ch.cacheNow = cache;
    ch.source = ce.source;
    ch.sourceSheet = ce.sourceSheet;
    ch.sourceName = ce.sourceName;
    return pivotStep({ch}, tr("Ungroup Field"));
}

EditSession::Outcome EditSession::setPivotCalculatedField(int table, int field, const QString &name, const QString &formula)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &m) {
        out.ok = false;
        out.problem = m;
        return out;
    };
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) return refuse(why.isEmpty() ? tr("There is no pivot table here.") : why);
    PivotBook &pb = m_book->book->pivots();
    const int ci = pb.table(table).cache;
    const PivotCacheEntry &ce = pb.cache(ci);
    if (pb.tablesOfCache(ci).size() > 1) return refuse(tr("Another pivot table is made from the same data. A calculated field is a part of the data, so it can't be made while two tables share it."));
    const PivotTable &old = *pb.table(table).table;
    const QString use = name.trimmed();
    if (use.isEmpty()) return refuse(tr("Give the field a name."));
    if (field >= 0 && (field >= ce.cache->fields.size() || field >= old.fields.size() || !ce.cache->fields[field].isCalculated())) return refuse(tr("This is not a calculated field."));
    for (int f = 0; f < ce.cache->fields.size(); ++f)
        if (f != field && ce.cache->fields[f].name.compare(use, Qt::CaseInsensitive) == 0) return refuse(tr("A field is already called \"%1\". Choose another name.").arg(use));
    for (int f = 0; f < old.fields.size(); ++f)
        if (f != field && old.fields[f].name.compare(use, Qt::CaseInsensitive) == 0) return refuse(tr("A field is already called \"%1\". Choose another name.").arg(use));
    QString problem;
    const auto parsed = jp::office::sheet::PivotFormula::parse(formula, &problem);
    if (!parsed) return refuse(tr("The formula can't be read: %1.").arg(problem));
    for (const QString &n : parsed->names()) {
        bool found = false;
        for (int f = 0; f < ce.cache->fields.size() && !found; ++f)
            found = f != field && ce.cache->fields[f].name.compare(n, Qt::CaseInsensitive) == 0;
        if (!found) return refuse(tr("The formula names \"%1\", which is not a field of this table.").arg(n));
    }
    QString text = formula.trimmed();
    if (text.startsWith(QLatin1Char('='))) text = text.mid(1).trimmed();
    auto cache = std::make_shared<PivotCache>(*ce.cache);
    PivotChange ch;
    ch.table = table;
    ch.now = std::make_shared<PivotTable>(old);
    PivotTable &t = *ch.now;
    if (field < 0) {
        PivotCacheField cf;
        cf.name = use;
        cf.formula = text;
        cf.group.databaseField = false;
        cache->fields.append(cf);
        PivotField pf;
        pf.calculated = true;
        pf.defaultSubtotal = false;
        while (t.fields.size() < cache->fields.size() - 1) t.fields.append(PivotField());
        t.fields.append(pf);
        PivotDataField d;
        d.fld = int(cache->fields.size()) - 1;
        d.subtotal = QStringLiteral("sum");
        d.name = pivotDataFieldName(t, *cache, d.fld, d.subtotal);
        t.dataFields.append(d);
    } else {
        PivotCacheField &cf = cache->fields[field];
        const QString was = cf.name;
        cf.name = use;
        cf.formula = text;
        // a formula that names this field by its old name follows the new one
        if (was.compare(use, Qt::CaseInsensitive) != 0)
            for (PivotCacheField &o : cache->fields)
                if (o.isCalculated() && &o != &cf) {
                    const auto p = jp::office::sheet::PivotFormula::parse(o.formula, nullptr);
                    if (p && p->names().contains(was, Qt::CaseInsensitive)) return refuse(tr("Another calculated field uses the name \"%1\". Change that formula first.").arg(was));
                }
    }
    pivotSyncRoles(&t);
    pivotNormalizeLayout(&t);
    pivotPrepareFields(&t, cache.get());
    pivotKeepNumericFlags(cache.get());
    ch.structure = field < 0;
    ch.cache = ci;
    ch.cacheNow = cache;
    ch.source = ce.source;
    ch.sourceSheet = ce.sourceSheet;
    ch.sourceName = ce.sourceName;
    return pivotStep({ch}, field < 0 ? tr("Insert Calculated Field") : tr("Change Calculated Field"));
}

EditSession::Outcome EditSession::deletePivotCalculatedField(int table, int field)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &m) {
        out.ok = false;
        out.problem = m;
        return out;
    };
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) return refuse(why.isEmpty() ? tr("There is no pivot table here.") : why);
    PivotBook &pb = m_book->book->pivots();
    const int ci = pb.table(table).cache;
    const PivotCacheEntry &ce = pb.cache(ci);
    if (pb.tablesOfCache(ci).size() > 1) return refuse(tr("Another pivot table is made from the same data, so the field can't be taken away from this one alone."));
    if (field < 0 || field >= ce.cache->fields.size() || !ce.cache->fields[field].isCalculated()) return refuse(tr("This is not a calculated field."));
    const QString gone = ce.cache->fields[field].name;
    for (int f = 0; f < ce.cache->fields.size(); ++f) {
        if (f == field || !ce.cache->fields[f].isCalculated()) continue;
        const auto p = jp::office::sheet::PivotFormula::parse(ce.cache->fields[f].formula, nullptr);
        if (p && p->names().contains(gone, Qt::CaseInsensitive)) return refuse(tr("The calculated field \"%1\" uses this one. Delete or change it first.").arg(ce.cache->fields[f].name));
    }
    auto cache = std::make_shared<PivotCache>(*ce.cache);
    cache->fields.removeAt(field);
    PivotChange ch;
    ch.table = table;
    ch.now = std::make_shared<PivotTable>(*pb.table(table).table);
    PivotTable &t = *ch.now;
    pivotRemoveFields(&t, {field});
    pivotSyncRoles(&t);
    pivotNormalizeLayout(&t);
    pivotPrepareFields(&t, cache.get());
    ch.structure = true;
    ch.cache = ci;
    ch.cacheNow = cache;
    ch.source = ce.source;
    ch.sourceSheet = ce.sourceSheet;
    ch.sourceName = ce.sourceName;
    return pivotStep({ch}, tr("Delete Calculated Field"));
}

EditSession::Outcome EditSession::drillPivotItem(int sheet, quint32 row, quint32 col, bool open)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    PivotBook::View view;
    if (!m_book || !m_book->book || !m_book->book->pivots().viewAt(sheet, row, col, &view) || !view.where.valid || !view.grid || !view.table) return refuse(tr("Click the name of an item in a pivot table first."));
    const PivotGrid &g = *view.grid;
    const int gridRow = int(qint64(row) - view.where.row1), gridCol = int(qint64(col) - view.where.col1);
    const int ri = gridRow - g.firstDataRow;
    if (gridCol < 0 || gridCol >= g.firstDataCol || ri < 0 || ri >= g.rowItems.size()) return refuse(tr("Click the name of an item in the rows of a pivot table first."));
    const PivotAxisItem &line = g.rowItems[ri];
    const PivotTable &t = *view.table;
    if (!line.t.isEmpty() || line.labelOnly || line.x.isEmpty()) return refuse(tr("Click the name of an item in the rows of a pivot table first."));
    // (in the compact form every level is in the first column, the line's last item; otherwise the column says which level the cell names, if the line holds it)
    const bool oneColumn = t.compact && !t.outline;
    const int level = oneColumn ? line.r + int(line.x.size()) - 1 : gridCol;
    if (!oneColumn && (level < line.r || level >= line.r + int(line.x.size()))) return refuse(tr("Click the name of an item in the rows of a pivot table first."));
    const int place = oneColumn ? line.x.back() : line.x[level - line.r];
    if (level < 0 || level >= t.rowFields.size()) return refuse(tr("Click the name of an item in the rows of a pivot table first."));
    if (level + 1 >= t.rowFields.size()) return refuse(tr("This item has nothing under it."));
    const int field = t.rowFields[level];
    if (field < 0 || field >= t.fields.size() || place < 0 || place >= t.fields[field].items.size() || !t.fields[field].items[place].t.isEmpty()) return refuse(tr("JeffCrunch can't find this item."));
    if (t.fields[field].items[place].showDetails == open) return out;
    PivotTable changed = t;
    changed.fields[field].items[place].showDetails = open;
    return setPivot(view.index, changed, true, open ? tr("Drill Down") : tr("Drill Up"));
}

EditSession::Outcome EditSession::movePivot(int table, quint32 row, quint32 col)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) {
        out.ok = false;
        out.problem = why.isEmpty() ? tr("There is no pivot table here.") : why;
        return out;
    }
    PivotBook &pb = m_book->book->pivots();
    const CellRange at = pb.locationOf(table);
    if (at.valid && at.row1 == row && at.col1 == col) return out;
    if (row >= 1048576 || col >= 16384) {
        out.ok = false;
        out.problem = tr("The pivot table would go past the edge of the sheet.");
        return out;
    }
    PivotChange ch;
    ch.table = table;
    ch.now = std::make_shared<PivotTable>(*pb.table(table).table);
    ch.moveRow = row;
    ch.moveCol = col;
    if (!at.valid) return pivotStep({ch}, tr("Move Pivot Table"));
    // The cells (the filters above the table too) are moved as cut cells are, so the formulas that point at them follow; then the table is drawn at its new place, which finds its cells there.
    CellRange box = at;
    const CellRange page = pageArea(*pb.table(table).table, at);
    if (page.valid) {
        box.row1 = std::min(box.row1, page.row1);
        box.col1 = std::min(box.col1, page.col1);
        box.row2 = std::max(box.row2, page.row2);
        box.col2 = std::max(box.col2, page.col2);
    }
    const qint64 dRow = qint64(row) - at.row1, dCol = qint64(col) - at.col1;
    if (qint64(box.row1) + dRow < 0 || qint64(box.col1) + dCol < 0 || qint64(box.row2) + dRow >= 1048576 || qint64(box.col2) + dCol >= 16384) {
        out.ok = false;
        out.problem = tr("The pivot table would go past the edge of the sheet.");
        return out;
    }
    const int sheet = pb.table(table).sheet;
    CellRange target = box;
    target.row1 = quint32(qint64(box.row1) + dRow);
    target.row2 = quint32(qint64(box.row2) + dRow);
    target.col1 = quint32(qint64(box.col1) + dCol);
    target.col2 = quint32(qint64(box.col2) + dCol);
    // (what is in the way is looked for first: a move of cells writes over what is there)
    bool free = true;
    m_book->calc->sheet(sheet).cells.forEachInRange(target.row1, target.col1, target.row2, target.col2, [&](quint32 r, quint32 c, const Cell &cell) {
        if (box.contains(r, c)) return true;
        if (cell.hasFormula() || cell.type != CellType::Empty) {
            free = false;
            return false;
        }
        return true;
    });
    // (and anything else that sits there: a table, another pivot table, merged cells, the sheet's filter; none of it is moved or covered, so it is found before the cells are touched)
    QString inTheWay;
    auto meets = [&](const CellRange &r) { return r.valid && !(r.row2 < target.row1 || r.row1 > target.row2 || r.col2 < target.col1 || r.col1 > target.col2); };
    for (const Table &tb : m_book->book->tables())
        if (tb.sheet == sheet && !(tb.row2 < target.row1 || tb.row1 > target.row2 || tb.col2 < target.col1 || tb.col1 > target.col2)) inTheWay = tr("There is a table in the way (%1).").arg(tb.name);
    for (int other = 0; other < pb.tableCount() && inTheWay.isEmpty(); ++other) {
        if (other == table || pb.table(other).sheet != sheet) continue;
        const CellRange ol = pb.locationOf(other);
        if (meets(ol) || (pb.table(other).table && meets(pageArea(*pb.table(other).table, ol)))) inTheWay = tr("There is another pivot table in the way.");
    }
    if (inTheWay.isEmpty())
        for (const CellRange &m : m_book->calc->mergesOf(sheet))
            if (meets(m) && !box.contains(m.row1, m.col1)) inTheWay = tr("There are merged cells in the way (%1).").arg(rangeName(m));
    if (inTheWay.isEmpty() && meets(m_book->calc->autoFilterOf(sheet).ref)) inTheWay = tr("The sheet's filter is in the way.");
    if (!inTheWay.isEmpty()) {
        out.ok = false;
        out.problem = inTheWay + QLatin1Char(' ') + tr("Move the pivot table somewhere else, or clear those cells first.");
        return out;
    }
    if (!free) {
        out.ok = false;
        out.problem = tr("The pivot table would cover cells that have something in them (%1). Clear those cells, or move the pivot table, and try again.").arg(rangeName(target));
        return out;
    }
    const int startIndex = m_undo.index();
    m_undo.beginMacro(tr("Move Pivot Table"));
    m_movingPivot = true;
    Outcome step = moveCells(sheet, box, sheet, target.row1, target.col1);
    m_movingPivot = false;
    ch.cellsMoved = true;
    if (step.ok) step = pivotStep({ch}, tr("Move Pivot Table"));
    m_undo.endMacro();
    if (!step.ok) {
        while (m_undo.index() > startIndex) m_undo.undo();
        return step;
    }
    return out;
}

EditSession::Outcome EditSession::setPivotRefreshOnOpen(int table, bool on)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || table < 0 || table >= m_book->book->pivots().tableCount() || !pivotUsable(table, &why)) {
        out.ok = false;
        out.problem = why.isEmpty() ? tr("There is no pivot table here.") : why;
        return out;
    }
    PivotBook &pb = m_book->book->pivots();
    const int ci = pb.table(table).cache;
    const PivotCacheEntry &ce = pb.cache(ci);
    if (ce.cache->refreshOnLoad == on) return out;
    PivotChange ch;
    ch.table = table;
    ch.now = std::make_shared<PivotTable>(*pb.table(table).table);
    auto cache = std::make_shared<PivotCache>(*ce.cache);
    cache->refreshOnLoad = on;
    ch.cache = ci;
    ch.cacheNow = cache;
    ch.source = ce.source;
    ch.sourceSheet = ce.sourceSheet;
    ch.sourceName = ce.sourceName;
    return pivotStep({ch}, tr("Pivot Table Options"));
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
    int sheet = ce.sourceSheet >= 0 ? ce.sourceSheet : pb.table(table).sheet;
    CellRange range;
    QString name;
    if (!parsePivotSource(where, sheet, &sheet, &range, &name, &why)) return refuse(why);
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
    std::shared_ptr<PivotCache> cache;
    if (pivotNeedsSharedItems(*ch.now, *ce.cache)) cache = std::make_shared<PivotCache>(*ce.cache);   // (the records are copied only when a field gets its shared items)
    else {
        // (the table still gets its lists of items, which need no copy of the records)
        PivotCache none;
        none.fields = ce.cache->fields;
        pivotPrepareFields(ch.now.get(), &none);
    }
    if (cache && pivotPrepareFields(ch.now.get(), cache.get())) {
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
