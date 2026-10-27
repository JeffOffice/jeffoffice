// Tests of saving a workbook read from the older binary format: it is written as a package of the XML form (every sheet, the strings, the styles, the names), reads back the same,
// takes the changes of the window, and is the file the next save puts its changes in. Every workbook is built in memory by xlshelpers.
#include "xlshelpers.h"

#include "io/zip.h"
#include "sheet/xlsxsave.h"

#include <QHash>
#include <QSet>
#include <QXmlStreamReader>

#include <atomic>
#include <limits>

namespace {

// A workbook with a little of everything the writer has to keep: strings (shared, and BIFF's own), formulas of the kinds the reader keeps, errors and booleans, formats with
// fonts, fills, borders, and named styles, widths and heights, merged ranges, panes, names, and a sheet that is hidden.
Book richBook()
{
    Book b;
    b.noDefaultFormats();
    for (int i = 0; i < 5; ++i) b.globals(font(QStringLiteral("Arial"), 10));
    b.globals(font(QStringLiteral("Courier New"), 12.5, 700, true, true, 2, 10, 1));
    b.globals(format(164, QStringLiteral("0.000")) + format(165, QStringLiteral("#,##0 \"units\"")));
    b.globals(format(166, QStringLiteral("0.0\t\"a\nb\"")));   // (a format with a tab and a line break in it, which an attribute keeps only as character references)
    XfSpec first;
    first.style = true;
    first.format = 9;
    b.globals(xf(first));
    XfSpec styleXf;
    styleXf.style = true;
    XfSpec commaXf;   // place 3: the style the comma format is
    commaXf.style = true;
    commaXf.format = 4;
    for (int i = 1; i < 15; ++i) b.globals(xf(i == 3 ? commaXf : styleXf));
    XfSpec def;
    def.format = 10;
    b.globals(xf(def));
    XfSpec one;   // place 16
    one.font = 6;
    one.format = 164;
    one.halign = 2;
    one.valign = 0;
    one.wrap = true;
    one.rotation = 90;
    one.indent = 2;
    one.shrink = true;
    one.borders[0] = 1;
    one.borders[2] = 6;
    one.borderColors[0] = 10;
    one.borderColors[2] = 12;
    one.pattern = 1;
    one.fore = 13;
    one.back = 65;
    one.parent = 18;
    b.globals(xf(one));
    XfSpec two;   // place 17: a date, not locked
    two.format = 14;
    two.locked = false;
    b.globals(xf(two));
    XfSpec heading;   // place 18: a named style
    heading.style = true;
    heading.font = 6;
    b.globals(xf(heading));
    b.globals(styleBuiltin(0, 0) + styleBuiltin(3, 3) + styleNamed(18, QStringLiteral("Heading & <1>")));
    b.globals(palette({0x112233, 0xAABBCC}));
    // (the first text has two runs, the second font 6 above, bold and italic and struck out; the last has runs in the middle of its second line)
    b.after(sst({QStringLiteral("alpha"), QStringLiteral("  padded  "), QStringLiteral("a & b <c> \"d\""), QStringLiteral("Привет, мир"), QStringLiteral("line\nbreak")}, 0, {}, {{1, 6, 3, 0}, {}, {}, {0, 6, 6, 0}, {5, 6}}));
    b.after(name(QStringLiteral("Rate"), tRef3d(0, 0, 1)) + name(QStringLiteral("Local"), tRef3d(0, 1, 1), 1) + name(QStringLiteral("Quiet"), tRef3d(0, 2, 1), 0, -1, true));
    b.after(supbookInternal(2) + externSheet({{0, 0, 0}}));
    QByteArray cells = dimensions(12, 6) + defColWidth(10) + defaultRowHeight(0, 255);
    cells += colinfo(1, 2, 3072, 0, 0x02) + colinfo(4, 4, 2560, 16, 0x01);
    cells += rowRec(3, 400, 0x40) + rowRec(5, 255, 0x20);
    cells += number(0, 0, 1.5, 16) + number(0, 1, 45293, 17) + labelSst(0, 2, 0) + labelSst(0, 3, 1) + labelSst(0, 4, 2) + labelSst(0, 5, 3) + labelSst(1, 0, 4);
    cells += boolErr(1, 1, 1, false) + boolErr(1, 2, 7, true) + blank(1, 3, 16);
    cells += formulaNumber(2, 0, tRef(0, 0) + tInt(2) + tOp(0x05), 3.0);
    cells += formulaSpecial(2, 1, tStr(QStringLiteral("hi")), 0) + stringRec(QStringLiteral("hi"));
    // B4:B6 = A1*2 (one formula for the three)
    cells += number(3, 0, 1) + number(4, 0, 2) + number(5, 0, 3);
    const QByteArray times2 = tRefN(0, -1) + tInt(2) + tOp(0x05);
    cells += formulaExp(3, 1, 3, 1, 2) + shrfmla(3, 5, 1, 1, times2) + formulaExp(4, 1, 3, 1, 4) + formulaExp(5, 1, 3, 1, 6);
    // an array formula over C7:D8 and a data table
    const QByteArray times = tRef(3, 0) + tInt(2) + tOp(0x05);
    cells += formulaExp(6, 2, 6, 2, 2) + arrayRec(6, 7, 2, 3, times) + formulaExp(6, 3, 6, 2, 4) + formulaExp(7, 2, 6, 2, 6) + formulaExp(7, 3, 6, 2, 8);
    cells += formulaExp(9, 1, 9, 1, 10, 15, true) + tableRec(9, 10, 1, 2, 0, 0, 0) + formulaExp(9, 2, 9, 1, 11, 15, true) + formulaExp(10, 1, 9, 1, 12, 15, true)
        + formulaExp(10, 2, 9, 1, 13, 15, true);
    cells += formulaExp(9, 4, 9, 4, 10, 15, true) + tableRec(9, 10, 4, 5, 0x04, 0, 1, 0, 0) + formulaExp(9, 5, 9, 4, 11, 15, true) + formulaExp(10, 4, 9, 4, 12, 15, true) + formulaExp(10, 5, 9, 4, 13, 15, true);
    cells += formulaExp(9, 7, 9, 7, 10, 15, true) + tableRec(9, 10, 7, 8, 0x00, 0, 1, 0, 0) + formulaExp(9, 8, 9, 7, 11, 15, true) + formulaExp(10, 7, 9, 7, 12, 15, true) + formulaExp(10, 8, 9, 7, 13, 15, true);
    cells += mergedCells({{0, 0, 4, 5}});
    cells += window2(0x02 | 0x04 | 0x08 | 0x10 | 0x200, 2, 1) + pane(2, 3, 5, 4, 0) + scl(3, 2) + selection(3, 1, 1);
    b.sheet(QStringLiteral("Data & <more>"), cells);
    b.sheet(QStringLiteral("Second"), number(0, 0, 7) + protect(true));
    b.sheet(QStringLiteral("Hidden"), number(0, 0, 9), 1);
    // a sheet whose rows are hidden unless it lists them: the first and third have cells and no row of their own, so they stay hidden
    b.sheet(QStringLiteral("Zero"), defaultRowHeight(0x02, 255) + rowRec(1, 300, 0x40) + number(0, 0, 1) + number(1, 0, 2) + number(2, 0, 3));
    return b;
}

bool sameColor(const ColorSpec &a, const ColorSpec &b) { return a.kind == b.kind && a.rgb == b.rgb && a.index == b.index && a.tint == b.tint; }

// The reasons two sheets differ, or an empty string if they hold the same.
QString differences(const Workbook &a, const Sheet &x, const Workbook &b, const Sheet &y)
{
    QStringList out;
    if (x.cells.cellCount() != y.cells.cellCount()) out << QStringLiteral("cell count %1 vs %2").arg(x.cells.cellCount()).arg(y.cells.cellCount());
    x.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        const Cell *d = y.cells.find(row, col);
        const QString ref = cellName(row, col);
        if (!d) {
            out << ref + QStringLiteral(" missing");
            return;
        }
        if (c.type != d->type) out << ref + QStringLiteral(" type %1 vs %2").arg(int(c.type)).arg(int(d->type));
        if (c.type == CellType::Number || c.type == CellType::Bool || c.type == CellType::Error || c.type == CellType::Date) {
            if (c.number != d->number) out << ref + QStringLiteral(" number %1 vs %2").arg(c.number).arg(d->number);
        } else if (x.text(c, a.sharedStrings()) != y.text(*d, b.sharedStrings())) {
            out << ref + QStringLiteral(" text '%1' vs '%2'").arg(x.text(c, a.sharedStrings()), y.text(*d, b.sharedStrings()));
        }
        if (c.style != d->style) out << ref + QStringLiteral(" style %1 vs %2").arg(c.style).arg(d->style);
        const quint8 kept = HasFormula | ArrayFormula | DataTable;
        if ((c.flags & kept) != (d->flags & kept)) out << ref + QStringLiteral(" flags %1 vs %2").arg(c.flags & kept).arg(d->flags & kept);
        if (c.hasFormula() && d->hasFormula()) {
            if (x.formulaText(c) != y.formulaText(*d)) out << ref + QStringLiteral(" formula '%1' vs '%2'").arg(x.formulaText(c), y.formulaText(*d));
            if ((c.flags & (ArrayFormula | DataTable)) && rangeName(x.formulaRange(c)) != rangeName(y.formulaRange(*d)))
                out << ref + QStringLiteral(" range %1 vs %2").arg(rangeName(x.formulaRange(c)), rangeName(y.formulaRange(*d)));
        }
    });
    if (x.merges.size() != y.merges.size()) out << QStringLiteral("merges");
    else
        for (int i = 0; i < x.merges.size(); ++i)
            if (!(x.merges[i] == y.merges[i])) out << QStringLiteral("merge %1").arg(i);
    if (x.columns.size() != y.columns.size()) out << QStringLiteral("columns %1 vs %2").arg(x.columns.size()).arg(y.columns.size());
    else
        for (int i = 0; i < x.columns.size(); ++i) {
            const ColumnSpan &p = x.columns[i], &q = y.columns[i];
            if (p.first != q.first || p.last != q.last || p.hidden != q.hidden || p.style != q.style || std::abs(p.width - q.width) > 1e-6 || p.customWidth != q.customWidth)
                out << QStringLiteral("column %1").arg(i);
        }
    // the rows as they look: hidden or not, how tall, and the format their empty cells have (a sheet can hide the rows it does not list, so a row with no spec of its own is not the same
    // as one with a spec that says the defaults)
    QSet<quint32> rows;
    x.cells.forEach([&](quint32 cellRow, quint32, const Cell &c) { rows.insert(cellRow); });
    y.cells.forEach([&](quint32 cellRow, quint32, const Cell &c) { rows.insert(cellRow); });
    for (const RowSpec &r : x.rowSpecs) rows.insert(r.row);
    for (const RowSpec &r : y.rowSpecs) rows.insert(r.row);
    auto look = [](const Sheet &sh, quint32 row) {
        for (const RowSpec &r : sh.rowSpecs)
            if (r.row == row) return QStringLiteral("%1/%2/%3/%4").arg(r.hidden).arg(r.height >= 0 ? r.height : sh.format.defaultRowHeight).arg(r.customFormat).arg(r.customFormat ? r.style : 0);
        return QStringLiteral("%1/%2/0/0").arg(sh.format.zeroHeight).arg(sh.format.defaultRowHeight);
    };
    for (const quint32 row : rows)
        if (look(x, row) != look(y, row)) out << QStringLiteral("row %1: %2 vs %3").arg(row).arg(look(x, row), look(y, row));
    if (x.format.baseColWidth != y.format.baseColWidth || std::abs(x.format.defaultRowHeight - y.format.defaultRowHeight) > 1e-6) out << QStringLiteral("sheet format");
    const SheetView &v = x.view, &w = y.view;
    if (v.frozen != w.frozen || v.frozenRows != w.frozenRows || v.frozenCols != w.frozenCols || v.paneRow != w.paneRow || v.paneCol != w.paneCol || v.topRow != w.topRow || v.leftCol != w.leftCol
        || v.zoom != w.zoom || v.showGridLines != w.showGridLines || v.showHeadings != w.showHeadings || v.showZeros != w.showZeros || v.tabSelected != w.tabSelected
        || v.activeRow != w.activeRow || v.activeCol != w.activeCol)
        out << QStringLiteral("view");
    if (x.isProtected != y.isProtected) out << QStringLiteral("protected");
    return out.join(QStringLiteral("; "));
}

// The entries of a styles part that tie a cell format to a named style: the style formats (cellStyleXfs), the cell formats (cellXfs), and the names (cellStyles), each as its attributes.
struct StyleTies {
    QVector<QHash<QString, QString>> styleXfs, cellXfs, names;
};

StyleTies styleTies(const QByteArray &xml)
{
    StyleTies out;
    QXmlStreamReader r(xml);
    int where = 0;   // 1 cellStyleXfs, 2 cellXfs, 3 cellStyles
    while (!r.atEnd()) {
        const auto token = r.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const QString name = r.name().toString();
            if (name == QLatin1String("cellStyleXfs")) where = 1;
            else if (name == QLatin1String("cellXfs")) where = 2;
            else if (name == QLatin1String("cellStyles")) where = 3;
            else if ((name == QLatin1String("xf") && (where == 1 || where == 2)) || (name == QLatin1String("cellStyle") && where == 3)) {
                QHash<QString, QString> a;
                for (const QXmlStreamAttribute &at : r.attributes()) a.insert(at.name().toString(), at.value().toString());
                (where == 1 ? out.styleXfs : where == 2 ? out.cellXfs : out.names).append(a);
            }
        } else if (token == QXmlStreamReader::EndElement) {
            const QString name = r.name().toString();
            if (name == QLatin1String("cellStyleXfs") || name == QLatin1String("cellXfs") || name == QLatin1String("cellStyles")) where = 0;
        }
    }
    return out;
}

bool sameFont(const FontSpec &a, const FontSpec &b)
{
    return a.name == b.name && a.size == b.size && a.bold == b.bold && a.italic == b.italic && a.strike == b.strike && a.underline == b.underline && a.vertAlign == b.vertAlign && sameColor(a.color, b.color);
}

// The reasons the runs of formatting of two lists of strings differ, or an empty string if they are the same.
QString sameRuns(const SharedStrings &a, const SharedStrings &b)
{
    if (a.rich.size() != b.rich.size()) return QStringLiteral("%1 strings with runs vs %2").arg(a.rich.size()).arg(b.rich.size());
    for (auto it = a.rich.constBegin(); it != a.rich.constEnd(); ++it) {
        const RichRuns y = b.rich.value(it.key());
        if (it.value().size() != y.size()) return QStringLiteral("string %1: %2 runs vs %3").arg(it.key()).arg(it.value().size()).arg(y.size());
        for (int i = 0; i < y.size(); ++i) {
            const RichRun &p = it.value()[i], &q = y[i];
            if (p.start != q.start || (p.font == RichRun::kCellFont) != (q.font == RichRun::kCellFont)) return QStringLiteral("string %1: run %2 begins elsewhere").arg(it.key()).arg(i);
            if (p.font != RichRun::kCellFont && !sameFont(a.richFonts[int(p.font)], b.richFonts[int(q.font)])) return QStringLiteral("string %1: run %2 is in another font").arg(it.key()).arg(i);
        }
    }
    return QString();
}

} // namespace

class SheetXlsSaveTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The workbook as the XML form: it is a package that opens, with the sheets, names, strings, and styles of the old file, and every sheet holds what the old one did.
    void anOldWorkbookIsWrittenAsAPackageThatReadsBackTheSame()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openBook(richBook(), &wb, &why), qPrintable(why));
        QVERIFY(wb.isBinary());
        QVERIFY(!wb.package().isLazy());
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        Workbook back;
        Limits withRuns;
        withRuns.richText = true;
        QVERIFY2(back.open(pkg, &why, withRuns), qPrintable(why));
        QVERIFY2(back.sharedStringsError().isEmpty() && back.stylesError().isEmpty(), qPrintable(back.sharedStringsError() + back.stylesError()));
        // the workbook
        QCOMPARE(back.info().sheets.size(), wb.info().sheets.size());
        for (int i = 0; i < wb.info().sheets.size(); ++i) {
            QCOMPARE(back.info().sheets[i].name, wb.info().sheets[i].name);
            QVERIFY(back.info().sheets[i].state == wb.info().sheets[i].state);
        }
        QCOMPARE(back.info().definedNames.size(), wb.info().definedNames.size());
        for (int i = 0; i < wb.info().definedNames.size(); ++i) {
            QCOMPARE(back.info().definedNames[i].name, wb.info().definedNames[i].name);
            QCOMPARE(back.info().definedNames[i].ref, wb.info().definedNames[i].ref);
            QCOMPARE(back.info().definedNames[i].localSheet, wb.info().definedNames[i].localSheet);
            QCOMPARE(back.info().definedNames[i].hidden, wb.info().definedNames[i].hidden);
        }
        QVERIFY(back.info().dateSystem == wb.info().dateSystem);
        // the strings, in the same places
        QCOMPARE(back.sharedStrings().strings, wb.sharedStrings().strings);
        // the runs of formatting inside the strings: the same places, in the same fonts
        QVERIFY2(sameRuns(wb.sharedStrings(), back.sharedStrings()).isEmpty(), qPrintable(sameRuns(wb.sharedStrings(), back.sharedStrings())));
        QCOMPARE(back.sharedStrings().rich.size(), 3);
        // the styles, in the same places
        const Styles &s = wb.styles(), &t = back.styles();
        QCOMPARE(t.formats.size(), s.formats.size());
        for (int i = 0; i < s.formats.size(); ++i) {
            const CellFormat &p = s.formats[i], &q = t.formats[i];
            QVERIFY2(p.numFmtId == q.numFmtId && p.horizontal == q.horizontal && p.vertical == q.vertical && p.wrap == q.wrap && p.shrink == q.shrink && p.indent == q.indent
                         && p.rotation == q.rotation && p.locked == q.locked,
                     qPrintable(QStringLiteral("cell format %1").arg(i)));
            QCOMPARE(s.formatCode(quint32(i), QLocale::c()), t.formatCode(quint32(i), QLocale::c()));
            const FontSpec &f = s.font(p.font), &g = t.font(q.font);
            QVERIFY2(f.name == g.name && f.size == g.size && f.bold == g.bold && f.italic == g.italic && f.strike == g.strike && f.underline == g.underline && f.vertAlign == g.vertAlign
                         && s.resolve(f.color, 1) == t.resolve(g.color, 1),
                     qPrintable(QStringLiteral("font of format %1").arg(i)));
            const FillSpec &fa = s.fill(p.fill), &fb = t.fill(q.fill);
            QVERIFY2(fa.pattern == fb.pattern && s.resolve(fa.fg, 2) == t.resolve(fb.fg, 2) && s.resolve(fa.bg, 3) == t.resolve(fb.bg, 3), qPrintable(QStringLiteral("fill of format %1").arg(i)));
            const BorderSpec &ba = s.border(p.border), &bb = t.border(q.border);
            const BorderSide *sa[] = {&ba.left, &ba.right, &ba.top, &ba.bottom}, *sb[] = {&bb.left, &bb.right, &bb.top, &bb.bottom};
            for (int k = 0; k < 4; ++k)
                QVERIFY2(sa[k]->style == sb[k]->style && (sa[k]->style == BorderStyle::None || s.resolve(sa[k]->color, 4) == t.resolve(sb[k]->color, 4)),
                         qPrintable(QStringLiteral("border %1 of format %2").arg(k).arg(i)));
        }
        for (auto it = s.numFmts.constBegin(); it != s.numFmts.constEnd(); ++it) QCOMPARE(t.numFmts.value(it.key()), it.value());
        // the palette
        ColorSpec c8;
        c8.kind = ColorSpec::Kind::Indexed;
        c8.index = 8;
        QCOMPARE(t.resolve(c8, 0), quint32(0x112233));
        c8.index = 12;
        QCOMPARE(t.resolve(c8, 0), s.resolve(c8, 0));
        // the named styles are in the styles part, as it is checked in the test of them
        // each sheet
        for (int i = 0; i < wb.info().sheets.size(); ++i) {
            Sheet x, y;
            QVERIFY(wb.readSheet(i, &x));
            QVERIFY2(back.readSheet(i, &y), qPrintable(y.error));
            QVERIFY2(differences(wb, x, back, y).isEmpty(), qPrintable(QStringLiteral("sheet %1: ").arg(i) + differences(wb, x, back, y)));
        }
        // and what the first sheet has is the sort of thing the check is about
        Sheet first;
        QVERIFY(back.readSheet(0, &first));
        QVERIFY(first.cells.find(0, 5) && first.text(*first.cells.find(0, 5), back.sharedStrings()) == QStringLiteral("Привет, мир"));
        QVERIFY(first.cells.find(3, 1) && first.cells.find(3, 1)->hasFormula());
        QVERIFY(first.cells.find(6, 2) && (first.cells.find(6, 2)->flags & ArrayFormula));
        QVERIFY(first.cells.find(9, 1) && (first.cells.find(9, 1)->flags & DataTable));
        QVERIFY(first.mergeCount == 1);
        // the sheet that hides the rows it does not list: the rows it did not list are hidden in the file written, and the one it listed is not
        Sheet zero;
        QVERIFY(back.readSheet(3, &zero));
        QVERIFY(zero.format.zeroHeight);
        auto hidden = [&](quint32 row) {
            for (const RowSpec &r : zero.rowSpecs)
                if (r.row == row) return r.hidden;
            return zero.format.zeroHeight;
        };
        QVERIFY(hidden(0) && !hidden(1) && hidden(2));
    }

    // The parts the writer made are the parts the workbook names, so that the changes of the window find them.
    void theWorkbookNamesThePartsItWillBeWrittenWith()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openBook(richBook(), &wb, &why), qPrintable(why));
        QVERIFY(!wb.mainPart().isEmpty() && !wb.sharedStringsPart().isEmpty() && !wb.stylesPart().isEmpty());
        QCOMPARE(wb.sharedStringsInFile(), qint64(wb.sharedStrings().strings.size()));
        QCOMPARE(wb.cellFormatsInFile(), qint64(wb.styles().formats.size()));
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        QVERIFY(pkg.hasPart(wb.mainPart()));
        QVERIFY(pkg.hasPart(wb.sharedStringsPart()));
        QVERIFY(pkg.hasPart(wb.stylesPart()));
        QSet<QString> parts;
        for (const SheetEntry &e : wb.info().sheets) {
            QVERIFY2(pkg.hasPart(e.part), qPrintable(e.part));
            QVERIFY(!parts.contains(e.part.toLower()));
            parts.insert(e.part.toLower());
        }
        QCOMPARE(pkg.mainPart(), wb.mainPart());
    }

    // The changes of the window are put in the file written: a number, a text with a new shared string, a format, and the results of a formula that were worked out again.
    void theChangesAreWrittenInTheFileMade()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openBook(richBook(), &wb, &why), qPrintable(why));
        const qint64 strings = wb.sharedStrings().strings.size();
        const qint32 added = wb.addSharedString(QStringLiteral("brand new"));
        QCOMPARE(qint64(added), strings);
        const int fmt = wb.numberFormatId(QStringLiteral("0.0000%"));
        const quint32 style = wb.formatWithNumberFormat(0, fmt);
        SaveChanges changes;
        SavedCell n;
        n.row = 20;
        n.col = 1;
        n.style = style;
        n.value.kind = SavedValue::Kind::Number;
        n.value.number = 42;
        SavedCell t;
        t.row = 20;
        t.col = 2;
        t.value.kind = SavedValue::Kind::Text;
        t.value.text = QStringLiteral("brand new");
        t.value.sharedString = added;
        changes.sheets[0].cells = {n, t};
        SavedResult r;
        r.row = 2;
        r.col = 0;
        r.value.kind = SavedValue::Kind::Number;
        r.value.number = 99;
        changes.sheets[0].results = {r};
        changes.newSharedStrings = wb.addedSharedStrings();
        changes.sharedStringsBefore = wb.sharedStringsInFile();
        for (const Workbook::AddedNumberFormat &f : wb.addedNumberFormats()) changes.newNumberFormats.append({f.id, f.code});
        for (const Workbook::AddedCellFormat &f : wb.addedCellFormats()) changes.newCellFormats.append({f.base, f.format.numFmtId, true, f.format});
        changes.cellFormatsBefore = wb.cellFormatsInFile();
        QByteArray bytes;
        SaveReport report;
        QVERIFY2(saveXlsx(wb, changes, &bytes, &why, &report), qPrintable(why));
        QCOMPARE(report.sheetsRewritten, 1);
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        Workbook back;
        QVERIFY2(back.open(pkg, &why), qPrintable(why));
        Sheet s;
        QVERIFY(back.readSheet(0, &s));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("B21")), QStringLiteral("n:42"));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("C21")), QStringLiteral("s:brand new"));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("A3")), QStringLiteral("n:99"));   // (the formula's result; the formula is still there)
        QVERIFY(s.cells.find(2, 0)->hasFormula());
        QCOMPARE(int(back.styles().numFmtId(s.cells.find(20, 1)->style)), fmt);
        QCOMPARE(back.styles().numFmts.value(fmt), QStringLiteral("0.0000%"));
        QCOMPARE(back.styles().formats.size(), wb.styles().formats.size());
    }

    // After the first save the workbook is the package written: the next save puts only what changed since in that file, and keeps what the first put in.
    void afterTheFirstSaveTheNextOneIsPutInTheFileWritten()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openBook(richBook(), &wb, &why), qPrintable(why));
        SaveChanges first;
        SavedCell n;
        n.row = 20;
        n.col = 1;
        n.value.kind = SavedValue::Kind::Number;
        n.value.number = 42;
        first.sheets[0].cells = {n};
        QByteArray one;
        QVERIFY2(saveXlsx(wb, first, &one, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(one, &why), qPrintable(why));
        wb.savedTo(pkg, 0, 0, 0);
        QVERIFY(wb.package().isLazy());
        SaveChanges second;
        SavedCell m;
        m.row = 21;
        m.col = 1;
        m.value.kind = SavedValue::Kind::Number;
        m.value.number = 43;
        second.sheets[0].cells = {m};
        QByteArray two;
        SaveReport report;
        QVERIFY2(saveXlsx(wb, second, &two, &why, &report), qPrintable(why));
        QCOMPARE(report.sheetsRewritten, 1);
        QCOMPARE(report.partsChanged, 1);   // (only the sheet: the written file is the base now, and the other parts are copied as they are)
        Package pkg2;
        QVERIFY2(pkg2.readLazy(two, &why), qPrintable(why));
        Workbook back;
        QVERIFY2(back.open(pkg2, &why), qPrintable(why));
        Sheet s;
        QVERIFY(back.readSheet(0, &s));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("B21")), QStringLiteral("n:42"));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("B22")), QStringLiteral("n:43"));
        // the workbook still reads its sheets from the old file's records, so what the window holds does not change
        Sheet old;
        QVERIFY(wb.readSheet(0, &old));
        QCOMPARE(show(old, wb.sharedStrings(), QStringLiteral("B21")), QStringLiteral("none"));
    }

    // Making the base file looks at the stop (a save that is given up), and a workbook with nothing in it is written as one.
    void aSaveCanBeStoppedAndAnEmptyWorkbookIsWritten()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openBook(richBook(), &wb, &why), qPrintable(why));
        std::atomic<bool> stop{true};
        QByteArray bytes;
        QVERIFY(!saveXlsx(wb, SaveChanges(), &bytes, &why, nullptr, &stop));
        QVERIFY2(why.contains(QLatin1String("stopped")), qPrintable(why));
        Book empty;
        empty.sheet(QStringLiteral("S"), QByteArray());
        Workbook e;
        QVERIFY2(openBook(empty, &e, &why), qPrintable(why));
        QVERIFY2(saveXlsx(e, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        Workbook back;
        QVERIFY2(back.open(pkg, &why), qPrintable(why));
        QCOMPARE(back.info().sheets.size(), 1);
    }

    // The named styles are written as cell styles: each with its name, the built-in ones with their number, and the number of the style format that holds its look; every style format has
    // a name, and a cell format follows the style format of the style it follows (the default one when it follows an unnamed one). A cell that uses a format that follows a named style
    // still does after the file is read again.
    void namedStylesAreWrittenAsCellStyles()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openBook(richBook(), &wb, &why), qPrintable(why));
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        Workbook back;
        QVERIFY2(back.open(pkg, &why), qPrintable(why));
        const StyleTies t = styleTies(pkg.part(back.stylesPart()));
        const Styles &st = wb.styles();
        QCOMPARE(t.cellXfs.size(), st.formats.size());
        QCOMPARE(t.names.size(), 3);
        // each name, the built-in number it has, and the style format it names: the format the style has in the workbook
        auto nameOf = [&](const QString &name) -> QHash<QString, QString> {
            for (const QHash<QString, QString> &n : t.names)
                if (n.value(QStringLiteral("name")) == name) return n;
            return {};
        };
        struct Expect {
            QString name;
            QString builtin;
            int format;   // the place of the format the style has in the workbook
        };
        const Expect expected[] = {{QStringLiteral("Normal"), QStringLiteral("0"), 15}, {QStringLiteral("Comma"), QStringLiteral("3"), 3}, {QStringLiteral("Heading & <1>"), QString(), 18}};
        QSet<int> named;
        for (const Expect &e : expected) {
            const QHash<QString, QString> n = nameOf(e.name);
            QVERIFY2(!n.isEmpty(), qPrintable(e.name));
            QCOMPARE(n.value(QStringLiteral("builtinId")), e.builtin);
            bool ok = false;
            const int at = n.value(QStringLiteral("xfId")).toInt(&ok);
            QVERIFY2(ok && at >= 0 && at < t.styleXfs.size(), qPrintable(e.name));
            named.insert(at);
            const CellFormat &f = st.formats[e.format];
            QCOMPARE(t.styleXfs[at].value(QStringLiteral("numFmtId")).toInt(), int(f.numFmtId));
            QCOMPARE(t.styleXfs[at].value(QStringLiteral("fontId")).toUInt(), f.font);
        }
        // every style format is named by one name (the other program expects as many names as style formats)
        QCOMPARE(named.size(), 3);
        QCOMPARE(t.styleXfs.size(), 3);
        // a cell format follows a style format that has a name, and the one of the style it follows: the heading, the default one for a cell format that follows the first (normal) style
        const int heading = nameOf(QStringLiteral("Heading & <1>")).value(QStringLiteral("xfId")).toInt();
        const int normal = nameOf(QStringLiteral("Normal")).value(QStringLiteral("xfId")).toInt();
        for (int i = 0; i < t.cellXfs.size(); ++i) {
            bool ok = false;
            const int at = t.cellXfs[i].value(QStringLiteral("xfId")).toInt(&ok);
            QVERIFY2(ok && named.contains(at), qPrintable(QStringLiteral("cell format %1").arg(i)));
        }
        QCOMPARE(t.cellXfs[16].value(QStringLiteral("xfId")).toInt(), heading);   // (the format of the first cell below follows the heading)
        QCOMPARE(t.cellXfs[17].value(QStringLiteral("xfId")).toInt(), normal);
        QCOMPARE(t.cellXfs[0].value(QStringLiteral("xfId")).toInt(), normal);
        // the cell that uses it is in the same format as before, and so follows the heading
        Sheet s;
        QVERIFY(back.readSheet(0, &s));
        const Cell *a1 = sheettest::at(s, QStringLiteral("A1"));
        QVERIFY(a1);
        QCOMPARE(a1->style, quint32(16));
        QCOMPARE(t.cellXfs[int(a1->style)].value(QStringLiteral("xfId")).toInt(), heading);
        QCOMPARE(back.styles().format(a1->style).font, st.format(16).font);
    }

    // Bold and plain text in one cell survive the save: the written strings part has the two runs (the bold one with its font), the text is the same, and read again with the runs
    // asked for they are where they were. A text in a BIFF5 workbook, which is the cell's own, is written as a rich text of the cell.
    void boldAndPlainTextInOneCellSurviveTheSave()
    {
        Book b;
        b.globals(font(QStringLiteral("Arial"), 10, 700));   // (the file's font 6)
        b.sheet(QStringLiteral("S"), labelSst(0, 0, 0) + labelSst(1, 0, 1));
        b.after(sst({QStringLiteral("bold and plain"), QStringLiteral("only plain")}, 0, {}, {{0, 6, 4, 0}, {}}));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        const QByteArray strings = pkg.part(QStringLiteral("/xl/sharedStrings.xml"));
        QVERIFY2(strings.contains("<si><r><rPr><b/>"), strings.constData());
        QVERIFY2(strings.contains(">bold</t></r>") && strings.contains("> and plain</t></r>"), strings.constData());
        QVERIFY2(strings.contains("<si><t>only plain</t></si>"), strings.constData());
        Workbook back;
        Limits withRuns;
        withRuns.richText = true;
        QVERIFY2(back.open(pkg, &why, withRuns), qPrintable(why));
        const SharedStrings &sh = back.sharedStrings();
        QCOMPARE(sh.strings.value(0), QStringLiteral("bold and plain"));
        QCOMPARE(sh.rich.size(), 1);
        const RichRuns runs = sh.rich.value(0);
        QCOMPARE(runs.size(), 2);
        QCOMPARE(runs[0].start, quint32(0));
        QVERIFY(sh.richFonts[int(runs[0].font)].bold);
        QCOMPARE(runs[1].start, quint32(4));
        QVERIFY(!sh.richFonts[int(runs[1].font)].bold);
        // opened as it is by default, the strings are the same and no run is kept
        Workbook plain;
        QVERIFY2(plain.open(pkg, &why), qPrintable(why));
        QCOMPARE(plain.sharedStrings().strings, sh.strings);
        QVERIFY(plain.sharedStrings().rich.isEmpty());
        // BIFF5
        Book five(5);
        five.globals(font(QStringLiteral("Arial"), 10, 700, false, false, 0, 0x7FFF, 0, 5));
        five.sheet(QStringLiteral("S"), rstring(0, 0, QStringLiteral("bold and plain"), {{0, 6}, {4, 0}}));
        Workbook w5;
        QVERIFY2(openBook(five, &w5, &why), qPrintable(why));
        QVERIFY2(saveXlsx(w5, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg5;
        QVERIFY2(pkg5.readLazy(bytes, &why), qPrintable(why));
        const QByteArray sheet = pkg5.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY2(sheet.contains("<is><r><rPr>") && sheet.contains("<b/>") && sheet.contains(">bold</t></r>") && sheet.contains("> and plain</t></r></is>"), sheet.constData());
        Workbook back5;
        QVERIFY2(back5.open(pkg5, &why), qPrintable(why));
        Sheet s5;
        QVERIFY(back5.readSheet(0, &s5));
        QCOMPARE(show(s5, back5.sharedStrings(), QStringLiteral("A1")), QStringLiteral("i:bold and plain"));
    }

    // A table of one input is written as the way it reads (a row input has dtr, a column input has not) and reads back as it was, with two inputs as well.
    void aDataTableKeepsWhichWayItReadsThroughTheSave()
    {
        Book b;
        QByteArray cells;
        cells += formulaExp(1, 1, 1, 1, 10, 15, true) + tableRec(1, 2, 1, 2, 0x04, 2, 3, 0, 0) + formulaExp(1, 2, 1, 1, 11, 15, true) + formulaExp(2, 1, 1, 1, 12, 15, true) + formulaExp(2, 2, 1, 1, 13, 15, true);
        cells += formulaExp(1, 5, 1, 5, 10, 15, true) + tableRec(1, 2, 5, 6, 0x00, 2, 3, 0, 0) + formulaExp(1, 6, 1, 5, 11, 15, true) + formulaExp(2, 5, 1, 5, 12, 15, true) + formulaExp(2, 6, 1, 5, 13, 15, true);
        cells += formulaExp(1, 8, 1, 8, 10, 15, true) + tableRec(1, 2, 8, 9, 0x08, 2, 3, 3, 4) + formulaExp(1, 9, 1, 8, 11, 15, true) + formulaExp(2, 8, 1, 8, 12, 15, true) + formulaExp(2, 9, 1, 8, 13, 15, true);
        b.sheet(QStringLiteral("S"), cells);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        const QByteArray sheet = pkg.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY2(sheet.contains("<f t=\"dataTable\" ref=\"B2:C3\" dtr=\"1\" r1=\"D3\"/>"), sheet.constData());
        QVERIFY2(sheet.contains("<f t=\"dataTable\" ref=\"F2:G3\" r1=\"D3\"/>"), sheet.constData());
        QVERIFY2(sheet.contains("<f t=\"dataTable\" ref=\"I2:J3\" dt2D=\"1\" dtr=\"1\" r1=\"D3\" r2=\"E4\"/>"), sheet.constData());
        Workbook back;
        QVERIFY2(back.open(pkg, &why), qPrintable(why));
        Sheet s;
        QVERIFY(back.readSheet(0, &s));
        auto text = [&](const QString &ref) {
            const Cell *c = sheettest::at(s, ref);
            return c && c->hasFormula() ? s.formulaText(*c) : QStringLiteral("none");
        };
        QCOMPARE(text(QStringLiteral("B2")), QStringLiteral("TABLE(D3,)"));
        QCOMPARE(text(QStringLiteral("F2")), QStringLiteral("TABLE(,D3)"));
        QCOMPARE(text(QStringLiteral("I2")), QStringLiteral("TABLE(D3,E4)"));
    }

    // A number that is not one (the file's bytes can spell it) is written as an error: the XML form has no such number.
    void aNumberThatIsNotANumberIsWrittenAsAnError()
    {
        Book b;
        b.sheet(QStringLiteral("S"), number(0, 0, std::numeric_limits<double>::quiet_NaN()) + number(0, 1, std::numeric_limits<double>::infinity())
                    + number(0, 2, -std::numeric_limits<double>::infinity()) + number(0, 3, 2.5));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        QVERIFY(!pkg.part(QStringLiteral("/xl/worksheets/sheet1.xml")).contains("nan"));
        QVERIFY(!pkg.part(QStringLiteral("/xl/worksheets/sheet1.xml")).contains("inf"));
        Workbook back;
        QVERIFY2(back.open(pkg, &why), qPrintable(why));
        Sheet s;
        QVERIFY(back.readSheet(0, &s));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("A1")), QStringLiteral("e:#NUM!"));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("B1")), QStringLiteral("e:#NUM!"));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("C1")), QStringLiteral("e:#NUM!"));
        QCOMPARE(show(s, back.sharedStrings(), QStringLiteral("D1")), QStringLiteral("n:2.5"));
    }

    // Text that XML cannot hold as it is (control characters, a lone surrogate) and text that looks like the escape the format uses are written so that they read back as they were.
    void textThatXmlCannotHoldIsWrittenSoThatItReadsBackTheSame()
    {
        Book b;
        const QString odd = QStringLiteral("tab\there \x01 bell \x0B and _x0041_ done");
        b.after(sst({odd, QStringLiteral("_x005F_ kept")}));
        b.sheet(QStringLiteral("S"), labelSst(0, 0, 0) + labelSst(0, 1, 1));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        Workbook back;
        QVERIFY2(back.open(pkg, &why), qPrintable(why));
        QCOMPARE(back.sharedStrings().strings, wb.sharedStrings().strings);
    }
};

JP_TEST_CLASS(SheetXlsSaveTests)
#include "tst_sheet_xlssave.moc"
