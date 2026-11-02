// The yardstick of the pivot tables (docs/features/jeffcrunch.md, PivotTable Analyze):
//   jcrunchtool pivot-check <file> [-v]   works out every pivot table of the workbook from its stored cache and compares the cells with those the file stored
//                                         where the table is: one line of tab-separated fields for each table: `pivot-check`, the path, the sheet, the table's name,
//                                         the result (ok, differs, or unsupported: and why), the cells compared, the cells that differ, and the first difference.
//                                         -v also prints the first rows of both grids of a table that differs.

#include "calc/numfmt.h"
#include "sheet/cellref.h"
#include "sheet/pivot.h"
#include "sheet/sheetreader.h"
#include "sheet/styles.h"

#include <QElapsedTimer>
#include <QTextStream>

#include <cmath>

using namespace jp::office;
using namespace jp::office::sheet;

namespace jp::crunchtool {

namespace {

QString show(const PivotCell &c)
{
    switch (c.kind) {
    case PivotCell::Empty: return QStringLiteral("·");
    case PivotCell::Number: return QString::number(c.number, 'g', 12);
    case PivotCell::Bool: return c.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case PivotCell::Text:
    case PivotCell::Error: return QLatin1Char('"') + c.text + QLatin1Char('"');
    }
    return QString();
}

PivotCell stored(const Sheet &sh, const SharedStrings &shared, quint32 row, quint32 col)
{
    PivotCell pc;
    const Cell *c = sh.cells.find(row, col);
    if (!c) return pc;
    switch (c->type) {
    case CellType::Empty: break;
    case CellType::Number:
    case CellType::Date: pc.kind = PivotCell::Number; pc.number = c->number; break;
    case CellType::SharedString:
    case CellType::InlineString:
    case CellType::FormulaString: pc.kind = PivotCell::Text; pc.text = sh.text(*c, shared); break;
    case CellType::Bool: pc.kind = PivotCell::Bool; pc.number = c->number; break;
    case CellType::Error: pc.kind = PivotCell::Error; pc.text = errorText(CellError(quint8(c->number))); break;
    }
    return pc;
}

bool same(const PivotCell &a, const PivotCell &b)
{
    if (a.kind != b.kind) {
        // (an error the file stored as text, or text that is an error)
        if ((a.kind == PivotCell::Text || a.kind == PivotCell::Error) && (b.kind == PivotCell::Text || b.kind == PivotCell::Error)) return a.text == b.text;
        return false;
    }
    switch (a.kind) {
    case PivotCell::Empty: return true;
    case PivotCell::Number: {
        const double m = std::max(std::abs(a.number), std::abs(b.number));
        return std::abs(a.number - b.number) <= 1e-9 * m || std::abs(a.number - b.number) <= 1e-12;
    }
    case PivotCell::Bool: return (a.number != 0) == (b.number != 0);
    default: return a.text == b.text;
    }
}

PivotValue valueOfStored(const Sheet &sh, const SharedStrings &shared, const Styles &styles, QHash<quint32, bool> *dateStyle, quint32 row, quint32 col)
{
    PivotValue v;
    const Cell *c = sh.cells.find(row, col);
    if (!c) return v;
    switch (c->type) {
    case CellType::Empty: break;
    case CellType::Number:
    case CellType::Date: {
        auto it = dateStyle->find(c->style);
        if (it == dateStyle->end()) {
            const int id = styles.numFmtId(c->style);
            const QString code = styles.numFmts.contains(id) ? styles.numFmts.value(id) : jp::calc::builtinFormatCode(id, QLocale(QLocale::English, QLocale::UnitedStates));
            it = dateStyle->insert(c->style, jp::calc::NumberFormat(code).kind() == jp::calc::FormatKind::Date);
        }
        v.kind = it.value() ? PivotValue::Date : PivotValue::Number;
        v.number = c->number;
        break;
    }
    case CellType::SharedString:
    case CellType::InlineString:
    case CellType::FormulaString: v.kind = PivotValue::Text; v.text = sh.text(*c, shared); break;
    case CellType::Bool: v.kind = PivotValue::Bool; v.number = c->number; break;
    case CellType::Error: v.kind = PivotValue::Error; v.text = errorText(CellError(quint8(c->number))); break;
    }
    return v;
}

} // namespace

int pivotRefreshMain(const QStringList &args)
{
    if (args.size() != 1) return -1;
    QTextStream out(stdout);
    Workbook book;
    QString why;
    if (!book.openFile(args[0], &why)) {
        out << "pivot-refresh\t" << args[0] << "\t\tnot opened: " << why << "\n";
        return 1;
    }
    const Package &pkg = book.package();
    QHash<QString, QString> doneCache;   // (a cache is read again once: the tables on it are all worked out from it)
    for (int s = 0; s < book.info().sheets.size(); ++s) {
        const SheetEntry &entry = book.info().sheets[s];
        if (entry.deleted || entry.kind != SheetKind::Worksheet || entry.part.isEmpty()) continue;
        const auto rels = pkg.relationshipsOfType(entry.part, QStringLiteral("/pivotTable"));
        if (rels.isEmpty()) continue;
        Sheet sh;
        book.readSheet(s, &sh);
        for (const Relationship &rel : rels) {
            const QString tablePart = resolvePartName(entry.part, rel.target);
            auto line = [&](const QString &name, const QString &result, const QString &detail, qint64 ms = 0) {
                out << "pivot-refresh\t" << args[0] << '\t' << entry.name << '\t' << name << '\t' << result << '\t' << ms << '\t' << detail << "\n";
            };
            PivotTable table;
            if (!readPivotTable(pkg.part(tablePart), &table, &why)) {
                line(tablePart, QStringLiteral("unsupported"), why);
                continue;
            }
            QString defPart, recPart;
            for (const Relationship &cr : pkg.relationshipsOfType(tablePart, QStringLiteral("/pivotCacheDefinition"))) defPart = resolvePartName(tablePart, cr.target);
            for (const Relationship &cr : pkg.relationshipsOfType(defPart, QStringLiteral("/pivotCacheRecords"))) recPart = resolvePartName(defPart, cr.target);
            PivotCache cache;
            if (defPart.isEmpty() || !readPivotCache(pkg.part(defPart), recPart.isEmpty() ? QByteArray() : pkg.part(recPart), book.info().dateSystem, &cache, &why)) {
                line(table.name, QStringLiteral("unsupported"), QStringLiteral("cache: ") + why);
                continue;
            }
            if (cache.sourceType != QLatin1String("worksheet") || cache.ref.isEmpty() || cache.sheet.isEmpty()) {
                line(table.name, QStringLiteral("unsupported"), QStringLiteral("source is a name or a table"));
                continue;
            }
            int srcSheet = -1;
            for (int i = 0; i < book.info().sheets.size(); ++i)
                if (!book.info().sheets[i].deleted && book.info().sheets[i].name.compare(cache.sheet, Qt::CaseInsensitive) == 0) srcSheet = i;
            CellRange src;
            if (srcSheet < 0 || !parseRangeRef(cache.ref, &src)) {
                line(table.name, QStringLiteral("unsupported"), QStringLiteral("source sheet or range not found"));
                continue;
            }
            Sheet source;
            book.readSheet(srcSheet, &source);
            QHash<quint32, bool> dateStyle;
            PivotSource ps;
            ps.row1 = src.row1;
            ps.col1 = src.col1;
            ps.row2 = src.row2;
            ps.col2 = src.col2;
            ps.cell = [&](quint32 r, quint32 c) { return valueOfStored(source, book.sharedStrings(), book.styles(), &dateStyle, r, c); };
            for (int f : table.rowFields) if (f >= 0) ps.axisFields.insert(f);
            for (int f : table.colFields) if (f >= 0) ps.axisFields.insert(f);
            for (const PivotPageField &p : table.pageFields) ps.axisFields.insert(p.fld);
            QElapsedTimer timer;
            timer.start();
            PivotCache fresh;
            if (!buildPivotCache(ps, cache, &fresh, &why)) {
                line(table.name, QStringLiteral("refused"), why);
                continue;
            }
            PivotTable refreshed = table;
            matchPivotItems(&refreshed, cache, fresh);
            PivotGrid grid, before;
            PivotOptions options;
            if (!computePivot(table, cache, &before, &why, options) || !computePivot(refreshed, fresh, &grid, &why, options)) {
                line(table.name, QStringLiteral("unsupported"), why);
                continue;
            }
            const qint64 ms = timer.elapsed();
            // 1. the data has not changed: the refreshed table shows what the table did
            QString detail;
            bool ok = true, unchanged = before.rows == grid.rows && before.cols == grid.cols;
            if (!unchanged) detail = QStringLiteral("the data changed since the last refresh: shape %1x%2 now %3x%4").arg(before.rows).arg(before.cols).arg(grid.rows).arg(grid.cols);
            else
                for (int i = 0; i < before.cells.size() && unchanged; ++i)
                    if (!same(before.cells[i], grid.cells[i])) {
                        unchanged = false;
                        detail = QStringLiteral("the data changed since the last refresh: cell %1,%2").arg(i / grid.cols).arg(i % grid.cols);
                    }
            // 2. what is written is read back to the same table
            if (ok) {
                CellRange loc;
                parseRangeRef(table.ref, &loc);
                PivotTable placed = refreshed;
                pivotApplyGrid(&placed, grid, loc.row1, loc.col1);
                const QByteArray tableXml = writePivotTable(pkg.part(tablePart), placed, false);
                const QByteArray defXml = writePivotCacheDefinition(pkg.part(defPart), fresh, fresh.recordCount, 45000.5, book.info().dateSystem);
                const QByteArray recXml = recPart.isEmpty() ? QByteArray() : writePivotCacheRecords(fresh, book.info().dateSystem);
                PivotTable t2;
                PivotCache c2;
                PivotGrid g2;
                QString shape;
                if (!tableXml.isEmpty() && !defXml.isEmpty() && !pivotCheckShape(tableXml, defXml, recXml, &shape)) {
                    ok = false;
                    detail = QStringLiteral("shape: ") + shape;
                } else if (tableXml.isEmpty() || defXml.isEmpty()) {
                    ok = false;
                    detail = QStringLiteral("a part was not written");
                } else if (!readPivotTable(tableXml, &t2, &why) || !readPivotCache(defXml, recXml, book.info().dateSystem, &c2, &why)) {
                    ok = false;
                    detail = QStringLiteral("not read back: ") + why;
                } else if (!computePivot(t2, c2, &g2, &why, options)) {
                    ok = false;
                    detail = QStringLiteral("worked out again: ") + why;
                } else if (g2.rows != grid.rows || g2.cols != grid.cols) {
                    ok = false;
                    detail = QStringLiteral("shape after read back");
                } else {
                    for (int i = 0; i < g2.cells.size() && ok; ++i)
                        if (!same(g2.cells[i], grid.cells[i])) {
                            ok = false;
                            detail = QStringLiteral("cell %1,%2 differs after read back").arg(i / grid.cols).arg(i % grid.cols);
                        }
                    // the lines the table wrote are the lines worked out
                    if (ok && (t2.rowItems.size() != grid.rowItems.size() || t2.colItems.size() != grid.colItems.size())) {
                        ok = false;
                        detail = QStringLiteral("lines after read back");
                    }
                }
            }
            line(table.name, !ok ? QStringLiteral("bad") : (unchanged ? QStringLiteral("ok") : QStringLiteral("changed")), detail, ms);
        }
    }
    return 0;
}

// `pivot-shape <file>`: every pivot table of the workbook with its cache and records, checked as the schema has them (pivotCheckShape): `pivot-shape`, the path, the table, and ok or what is wrong.
int pivotShapeMain(const QStringList &args)
{
    if (args.size() != 1) return -1;
    QTextStream out(stdout);
    Package pkg;
    QString why;
    if (!pkg.readLazyFile(args[0], &why)) {
        out << "pivot-shape\t" << args[0] << "\t\tnot opened: " << why << "\n";
        return 1;
    }
    for (const QString &part : pkg.partNames()) {
        if (!part.contains(QLatin1String("/pivotTables/pivotTable")) || part.contains(QLatin1String("_rels")) || !part.endsWith(QLatin1String(".xml"))) continue;
        QString def, rec;
        for (const Relationship &cr : pkg.relationshipsOfType(part, QStringLiteral("/pivotCacheDefinition"))) def = resolvePartName(part, cr.target);
        for (const Relationship &cr : pkg.relationshipsOfType(def, QStringLiteral("/pivotCacheRecords"))) rec = resolvePartName(def, cr.target);
        QString problem;
        const bool ok = !def.isEmpty() && pivotCheckShape(pkg.part(part), pkg.part(def), rec.isEmpty() ? QByteArray() : pkg.part(rec), &problem);
        out << "pivot-shape\t" << args[0] << '\t' << part << '\t' << (ok ? QStringLiteral("ok") : (def.isEmpty() ? QStringLiteral("no cache") : problem)) << "\n";
    }
    return 0;
}

int pivotCheckMain(const QStringList &args)
{
    if (args.isEmpty() || args.size() > 2) return -1;
    const bool verbose = args.size() == 2 && args[1] == QLatin1String("-v");
    QTextStream out(stdout);
    Workbook book;
    QString why;
    if (!book.openFile(args[0], &why)) {
        out << "pivot-check\t" << args[0] << "\t\t\tnot opened: " << why << "\n";
        return 1;
    }
    const Package &pkg = book.package();
    for (int s = 0; s < book.info().sheets.size(); ++s) {
        const SheetEntry &entry = book.info().sheets[s];
        if (entry.deleted || entry.kind != SheetKind::Worksheet || entry.part.isEmpty()) continue;
        const auto rels = pkg.relationshipsOfType(entry.part, QStringLiteral("/pivotTable"));
        if (rels.isEmpty()) continue;
        Sheet sh;
        book.readSheet(s, &sh);
        for (const Relationship &rel : rels) {
            const QString tablePart = resolvePartName(entry.part, rel.target);
            auto line = [&](const QString &name, const QString &result, qint64 cells, qint64 diffs, const QString &first) {
                out << "pivot-check\t" << args[0] << '\t' << entry.name << '\t' << name << '\t' << result << '\t' << cells << '\t' << diffs << '\t' << first << "\n";
            };
            PivotTable table;
            if (!readPivotTable(pkg.part(tablePart), &table, &why)) {
                line(tablePart, QStringLiteral("unsupported: ") + why, 0, 0, QString());
                continue;
            }
            QString defPart, recPart;
            for (const Relationship &cr : pkg.relationshipsOfType(tablePart, QStringLiteral("/pivotCacheDefinition")))
                defPart = resolvePartName(tablePart, cr.target);
            for (const Relationship &cr : pkg.relationshipsOfType(defPart, QStringLiteral("/pivotCacheRecords")))
                recPart = resolvePartName(defPart, cr.target);
            PivotCache cache;
            if (defPart.isEmpty() || !readPivotCache(pkg.part(defPart), recPart.isEmpty() ? QByteArray() : pkg.part(recPart), book.info().dateSystem, &cache, &why)) {
                line(table.name, QStringLiteral("unsupported: cache: ") + (defPart.isEmpty() ? QStringLiteral("none") : why), 0, 0, QString());
                continue;
            }
            PivotGrid grid;
            PivotOptions options;
            CellRange where;
            parseRangeRef(table.ref, &where);
            options.format = [&](double v, int id, int gridRow, int gridCol) {
                // (the number format of the cell above that shows this very number, else the field's)
                int use = id;
                for (qint64 r = qint64(where.row1) + gridRow; r >= where.row1 && r >= 0; --r) {
                    const Cell *cell = sh.cells.find(quint32(r), where.col1 + quint32(gridCol));
                    if (cell && (cell->type == CellType::Number || cell->type == CellType::Date) && cell->number == v) {
                        use = book.styles().numFmtId(cell->style);
                        break;
                    }
                }
                const QString code = book.styles().numFmts.contains(use) ? book.styles().numFmts.value(use) : jp::calc::builtinFormatCode(use, QLocale(QLocale::English, QLocale::UnitedStates));
                QString text;
                if (!jp::calc::formatTextFunction(v, code.isEmpty() ? QStringLiteral("General") : code, book.info().dateSystem, &text)) return QString::number(v, 'g', 15);
                return text;
            };
            if (!computePivot(table, cache, &grid, &why, options)) {
                line(table.name, QStringLiteral("unsupported: ") + why, 0, 0, QString());
                continue;
            }
            CellRange loc;
            parseRangeRef(table.ref, &loc);
            qint64 cells = 0, diffs = 0, firstRow = -1;
            QString first;
            // the shape the file has
            const qint64 fr = qint64(loc.row2) - loc.row1 + 1, fc = qint64(loc.col2) - loc.col1 + 1;
            auto note = [&](const QString &m) {
                if (first.isEmpty()) first = m;
            };
            if (fr != grid.rows || fc != grid.cols) {
                ++diffs;
                note(QStringLiteral("shape: stored %1x%2, worked out %3x%4").arg(fr).arg(fc).arg(grid.rows).arg(grid.cols));
            }
            QString lines;
            if (table.firstDataRow != grid.firstDataRow || table.firstDataCol != grid.firstDataCol) lines += QStringLiteral(" location");
            const qint64 rows = std::min<qint64>(fr, grid.rows), cols = std::min<qint64>(fc, grid.cols);
            for (qint64 r = 0; r < rows; ++r)
                for (qint64 c = 0; c < cols; ++c) {
                    ++cells;
                    const PivotCell a = stored(sh, book.sharedStrings(), loc.row1 + quint32(r), loc.col1 + quint32(c));
                    const PivotCell &b = grid.at(int(r), int(c));
                    if (!same(a, b)) {
                        ++diffs;
                        if (firstRow < 0) firstRow = r;
                        note(QStringLiteral("%1: stored %2, worked out %3").arg(cellName(loc.row1 + quint32(r), loc.col1 + quint32(c)), show(a), show(b)));
                    }
                }
            // the lines of the axes the file stored
            auto sameLines = [](const QVector<PivotAxisItem> &a, const QVector<PivotAxisItem> &b) {
                if (a.size() != b.size()) return false;
                for (int i = 0; i < a.size(); ++i)
                    if (a[i].t != b[i].t || a[i].r != b[i].r || a[i].i != b[i].i || a[i].x != b[i].x) return false;
                return true;
            };
            if (!sameLines(table.rowItems, grid.rowItems)) lines += QStringLiteral(" rowItems");
            if (!sameLines(table.colItems, grid.colItems)) lines += QStringLiteral(" colItems");
            line(table.name, diffs == 0 ? (lines.isEmpty() ? QStringLiteral("ok") : QStringLiteral("ok (lines differ:") + lines + QLatin1Char(')')) : QStringLiteral("differs"), cells, diffs, first);
            if (verbose && diffs > 0) {
                const qint64 from = std::max<qint64>(0, firstRow - 3);
                for (qint64 r = from; r < std::min<qint64>(std::max<qint64>(fr, grid.rows), from + 24); ++r) {
                    QStringList a, b;
                    for (qint64 c = 0; c < std::min<qint64>(std::max<qint64>(fc, grid.cols), 8); ++c) {
                        a << (r < fr && c < fc ? show(stored(sh, book.sharedStrings(), loc.row1 + quint32(r), loc.col1 + quint32(c))) : QStringLiteral("-"));
                        b << (r < grid.rows && c < grid.cols ? show(grid.at(int(r), int(c))) : QStringLiteral("-"));
                    }
                    out << "  S " << a.join(QStringLiteral(" | ")) << "\n  W " << b.join(QStringLiteral(" | ")) << "\n";
                }
            }
        }
    }
    return 0;
}

} // namespace jp::crunchtool
