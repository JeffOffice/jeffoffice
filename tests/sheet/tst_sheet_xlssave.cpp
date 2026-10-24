// Tests of saving a workbook read from the older binary format: it is written as a package of the XML form (every sheet, the strings, the styles, the names), reads back the same,
// takes the changes of the window, and is the file the next save puts its changes in. Every workbook is built in memory by xlshelpers.
#include "xlshelpers.h"

#include "io/zip.h"
#include "sheet/xlsxsave.h"

#include <QSet>

#include <atomic>

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
    XfSpec first;
    first.style = true;
    first.format = 9;
    b.globals(xf(first));
    XfSpec styleXf;
    styleXf.style = true;
    for (int i = 1; i < 15; ++i) b.globals(xf(styleXf));
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
    b.globals(styleBuiltin(0, 0) + styleNamed(18, QStringLiteral("Heading & <1>")));
    b.globals(palette({0x112233, 0xAABBCC}));
    b.after(sst({QStringLiteral("alpha"), QStringLiteral("  padded  "), QStringLiteral("a & b <c> \"d\""), QStringLiteral("Привет, мир"), QStringLiteral("line\nbreak")}));
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
    cells += mergedCells({{0, 0, 4, 5}});
    cells += window2(0x02 | 0x04 | 0x08 | 0x10 | 0x200, 2, 1) + pane(2, 3, 5, 4, 0) + scl(3, 2) + selection(3, 1, 1);
    b.sheet(QStringLiteral("Data & <more>"), cells);
    b.sheet(QStringLiteral("Second"), number(0, 0, 7) + protect(true));
    b.sheet(QStringLiteral("Hidden"), number(0, 0, 9), 1);
    return b;
}

bool sameColor(const ColorSpec &a, const ColorSpec &b) { return a.kind == b.kind && a.rgb == b.rgb && a.index == b.index && a.tint == b.tint; }

// The reasons two sheets differ, or an empty string if they hold the same.
QString differences(const Workbook &a, const Sheet &x, const Workbook &b, const Sheet &y)
{
    QStringList out;
    if (x.cells.cellCount() != y.cells.cellCount()) out << QStringLiteral("cell count %1 vs %2").arg(x.cells.cellCount()).arg(y.cells.cellCount());
    x.cells.forEach([&](const Cell &c) {
        const Cell *d = y.cells.find(c.row, c.col);
        const QString ref = cellName(c.row, c.col);
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
    if (x.rowSpecs.size() != y.rowSpecs.size()) out << QStringLiteral("rows %1 vs %2").arg(x.rowSpecs.size()).arg(y.rowSpecs.size());
    else
        for (int i = 0; i < x.rowSpecs.size(); ++i) {
            const RowSpec &p = x.rowSpecs[i], &q = y.rowSpecs[i];
            if (p.row != q.row || p.hidden != q.hidden || p.style != q.style || p.customFormat != q.customFormat || std::abs(p.height - q.height) > 1e-6 || p.customHeight != q.customHeight)
                out << QStringLiteral("row %1").arg(i);
        }
    if (x.format.baseColWidth != y.format.baseColWidth || std::abs(x.format.defaultRowHeight - y.format.defaultRowHeight) > 1e-6) out << QStringLiteral("sheet format");
    const SheetView &v = x.view, &w = y.view;
    if (v.frozen != w.frozen || v.frozenRows != w.frozenRows || v.frozenCols != w.frozenCols || v.paneRow != w.paneRow || v.paneCol != w.paneCol || v.topRow != w.topRow || v.leftCol != w.leftCol
        || v.zoom != w.zoom || v.showGridLines != w.showGridLines || v.showHeadings != w.showHeadings || v.showZeros != w.showZeros || v.tabSelected != w.tabSelected
        || v.activeRow != w.activeRow || v.activeCol != w.activeCol)
        out << QStringLiteral("view");
    if (x.isProtected != y.isProtected) out << QStringLiteral("protected");
    return out.join(QStringLiteral("; "));
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
        QVERIFY2(back.open(pkg, &why), qPrintable(why));
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
        // the named styles are in the styles part
        const QByteArray styles = pkg.part(back.stylesPart());
        QVERIFY2(styles.contains("Heading &amp; &lt;1&gt;"), styles.constData());
        QVERIFY(styles.contains("<cellStyles"));
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
        for (const Workbook::AddedCellFormat &f : wb.addedCellFormats()) changes.newCellFormats.append({f.base, f.numFmtId});
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
