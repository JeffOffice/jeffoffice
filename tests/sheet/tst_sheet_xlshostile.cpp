// Tests of the reader of the old binary workbook format against files made to hurt it: every length, count, and index in a file is checked against the bytes there are and
// against the limits, nothing is made in the size a file claims before the bytes for it are seen, and nothing a file can deepen recurses. Every file is built here.
#include "xlshelpers.h"

#include "sheet/calculation.h"
#include "sheet/xlsxsave.h"

#include <random>

namespace {

// A small workbook that has a bit of everything the reader reads: used to be damaged.
Book fullBook()
{
    Book b;
    b.globals(format(164, QStringLiteral("0.000")) + font(QStringLiteral("Courier"), 12, 700) + xf() + palette({0x112233}));
    b.after(supbookInternal(2) + externSheet({{0, 0, 0}, {0, 1, 1}}) + name(QStringLiteral("Rate"), tRef3d(0, 0, 0, false, false)) + name(QStringLiteral("Twice"), tName(1) + tInt(2) + tOp(0x05))
            + sst({QStringLiteral("one"), QString::fromUtf16(u"two 中"), QStringLiteral("three")}, 40, {}, {{0, 6, 2, 0}, {}, {1, 0, 4, 9}}));
    QByteArray one = dimensions(10, 6) + defColWidth(9) + colinfo(1, 2, 3000) + rowRec(2, 300, 0x40) + number(0, 0, 1.5) + rk(0, 1, 30) + mulrk(1, 0, {30, 34, 38}) + labelSst(2, 0, 1)
        + label(2, 1, QStringLiteral("abc")) + boolErr(3, 0, 1, false) + boolErr(3, 1, 7, true) + blank(4, 0, 16) + mulblank(4, 1, 3, 16);
    one += formulaNumber(5, 0, tRef(0, 0) + tRef(1, 0) + tOp(0x03), 7) + formulaSpecial(5, 1, tStr(QStringLiteral("x")), 0) + stringRec(QStringLiteral("x"));
    one += formulaExp(6, 0, 6, 0, 1) + shrfmla(6, 7, 0, 0, tRefN(-1, 0) + tInt(1) + tOp(0x03)) + formulaExp(7, 0, 6, 0, 2);
    one += formulaExp(8, 0, 8, 0, 1) + arrayRec(8, 9, 0, 0, tArea(0, 0, 1, 0)) + formulaExp(9, 0, 8, 0, 2);
    one += formulaNumber(10, 0, tRef3d(1, 0, 0) + tName(2) + tOp(0x03), 0);
    one += formulaNumber(11, 0, tArray() + tInt(1) + tOp(0x03), 0, 15, arrayValues({{1.0, 2.0}}));
    one += mergedCells({{0, 0, 0, 1}}) + window2(0x08 | 0x02, 1, 1) + pane(1, 2, 3, 4) + scl(3, 2) + selection(3, 1, 1);
    b.sheet(QStringLiteral("One"), one);
    b.sheet(QStringLiteral("Two"), number(0, 0, 4) + dimensions(1, 1));
    return b;
}

// Reads whatever there is, as a window would (opens, reads every sheet, asks for every formula and string, and works the formulas out), and checks what must hold of every file,
// whatever is in it: it is refused with a reason, or opened with every count within its limit and every cell inside the old form's sheet, and a sheet that was not read whole says why.
// With `calculate`, the formulas are worked out, and a workbook that was read whole is saved as the window saves it: the file written is a package that reads back with the sheets and
// cells the workbook has. Gives what does not hold, in words; empty if all does.
QString useAll(const QByteArray &bytes, const Limits &limits = Limits(), bool calculate = true)
{
    Workbook wb;
    QString why;
    if (!wb.openBinary(bytes, &why, limits)) return why.isEmpty() ? QStringLiteral("refused with no reason") : QString();
    const WorkbookInfo &info = wb.info();
    if (info.sheets.size() > limits.sheets) return QStringLiteral("more sheets than the limit");
    if (info.sheets.isEmpty()) return QStringLiteral("opened with no sheets");
    if (info.definedNames.size() > limits.definedNames) return QStringLiteral("more names than the limit");
    if (wb.sharedStrings().strings.size() > limits.sharedStrings || wb.sharedStrings().chars > limits.sharedStringChars) return QStringLiteral("more strings than the limit");
    if (wb.styles().fonts.size() > limits.fonts + 1 || wb.styles().formats.size() > limits.cellXfs || wb.styles().numFmts.size() > limits.numFmts) return QStringLiteral("more styles than the limit");
    bool wholeBook = wb.sharedStringsError().isEmpty() && wb.stylesError().isEmpty();
    QVector<qint64> cellCounts;
    for (int s = 0; s < info.sheets.size(); ++s) {
        Sheet sheet;
        const bool whole = wb.readSheet(s, &sheet);
        wholeBook = wholeBook && whole;
        cellCounts.append(sheet.cells.cellCount());
        if (whole != sheet.error.isEmpty()) return QStringLiteral("a sheet's error and its verdict disagree");
        if (sheet.cells.cellCount() > limits.cellsPerSheet) return QStringLiteral("more cells than the limit");
        if (sheet.mergeCount > limits.merges || sheet.columns.size() > limits.columnSpans || sheet.rowSpecs.size() > limits.rowSpecs) return QStringLiteral("more merges, columns, or rows than the limit");
        if (sheet.stats.formulaWork > limits.formulaWork + 4096 * 64) return QStringLiteral("more formula work than the limit");
        QString problem;
        sheet.cells.forEach([&](const Cell &c) {
            if (!problem.isEmpty()) return;
            if (c.row > 65535 || c.col > 255) problem = QStringLiteral("a cell outside the old form's sheet");
            else if (c.hasFormula() && (c.formula < 0 || c.formula >= sheet.formulas.size())) problem = QStringLiteral("a formula cell with no formula");
            else if (c.hasFormula() && sheet.formulas[c.formula].text >= sheet.formulaTexts.size()) problem = QStringLiteral("a formula whose text is not there");
            else if (c.hasFormula()) (void)sheet.formulaText(c);
            (void)sheet.text(c, wb.sharedStrings());
        });
        if (!problem.isEmpty()) return problem;
    }
    if (calculate && wholeBook) {
        QByteArray written;
        QString saveWhy;
        if (!saveXlsx(wb, SaveChanges(), &written, &saveWhy)) return QStringLiteral("a workbook read whole is not saved: ") + saveWhy;
        Package pkg;
        Workbook back;
        if (!pkg.readLazy(written, &saveWhy) || !back.open(pkg, &saveWhy)) return QStringLiteral("the file written does not open: ") + saveWhy;
        if (back.info().sheets.size() != info.sheets.size()) return QStringLiteral("the file written has other sheets");
        for (int s = 0; s < info.sheets.size(); ++s) {
            Sheet again;
            if (!back.readSheet(s, &again) || again.cells.cellCount() != cellCounts[s]) return QStringLiteral("the file written has other cells in sheet %1").arg(s);
        }
    }
    if (calculate) {
        jp::office::sheet::CalcOptions options;
        options.keepApart = true;
        jp::office::sheet::Calculation calc(wb, options);
        calc.load();
        calc.recalculate(5000000);
    }
    return QString();
}

} // namespace

class SheetXlsHostileTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The book the other tests damage is itself read whole.
    void theModelBookIsRead()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openBook(fullBook(), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        QCOMPARE(s.stats.unreadFormulas, qint64(0));
        QVERIFY(s.cells.cellCount() > 15);
        QCOMPARE(wb.sharedStrings().strings.size(), 3);
        QCOMPARE(wb.info().definedNames.size(), 2);
    }

    // A file cut off at every length: it opens or is refused, and what opens is read without a crash.
    void aFileCutOffAnywhereIsReadOrRefused()
    {
        const QByteArray stream = fullBook().stream();
        for (qsizetype n = 0; n <= stream.size(); n += 3) {
            const QString problem = useAll(stream.left(n));
            QVERIFY2(problem.isEmpty(), qPrintable(QStringLiteral("the stream cut after %1 bytes: %2").arg(n).arg(problem)));
        }
        const QByteArray file = fullBook().file();
        for (qsizetype n = 0; n < file.size(); n += 61) {
            const QString problem = useAll(file.left(n), Limits(), false);
            QVERIFY2(problem.isEmpty(), qPrintable(QStringLiteral("the file cut after %1 bytes: %2").arg(n).arg(problem)));
        }
        // (whole, it is read whole)
        QVERIFY(useAll(stream).isEmpty());
        Workbook wb;
        QVERIFY(openBook(fullBook(), &wb));
        Sheet sheet;
        QVERIFY(wb.readSheet(0, &sheet));
        QVERIFY(sheet.error.isEmpty());
    }

    // Every byte of the stream changed, one at a time, to a few values that matter (0, 1, 0x7F, 0x80, 0xFF).
    void everyByteChangedInTurn()
    {
        const QByteArray stream = fullBook().stream();
        for (qsizetype i = 0; i < stream.size(); ++i) {
            for (int v : {0x00, 0x01, 0x7F, 0x80, 0xFF}) {
                QByteArray copy = stream;
                copy[i] = char(v);
                const QString problem = useAll(copy, Limits(), i % 7 == 0);
                QVERIFY2(problem.isEmpty(), qPrintable(QStringLiteral("byte %1 made 0x%2: %3").arg(i).arg(v, 2, 16, QLatin1Char('0')).arg(problem)));
            }
        }
    }

    // Random damage, with a fixed seed: bytes changed, pieces taken out, repeated, and moved, the record lengths set to extremes.
    void randomDamageIsSurvived()
    {
        const QByteArray stream = fullBook().stream();
        std::mt19937_64 rng(12345);
        for (int round = 0; round < 3000; ++round) {
            QByteArray copy = stream;
            const int edits = 1 + int(rng() % 6);
            for (int e = 0; e < edits; ++e) {
                const qsizetype at = qsizetype(rng() % quint64(copy.size()));
                switch (rng() % 5) {
                case 0: copy[at] = char(rng() & 0xFF); break;
                case 1: copy.remove(at, qsizetype(1 + rng() % 40)); break;
                case 2: copy.insert(at, copy.mid(qsizetype(rng() % quint64(copy.size())), qsizetype(1 + rng() % 60))); break;
                case 3:
                    if (at + 3 < copy.size()) {
                        copy[at + 2] = char(rng() & 0xFF);
                        copy[at + 3] = char(rng() & 0xFF);
                    }
                    break;
                default: copy.truncate(at + 1); break;
                }
                if (copy.isEmpty()) copy = stream;
            }
            const QString problem = useAll(copy, Limits(), round % 4 == 0);
            QVERIFY2(problem.isEmpty(), qPrintable(QStringLiteral("round %1: %2").arg(round).arg(problem)));
        }
    }

    // A string table that claims four billion strings and holds three, and one whose strings claim more characters than there are bytes.
    void aStringTableThatClaimsMoreThanItHolds()
    {
        QByteArray table = sst({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")});
        for (int i = 8; i < 12; ++i) table[i] = char(0xFF);   // the count of strings (after the record's header and the total of references)
        table[12] = char(0xFF);
        table[13] = char(0xFF);   // the characters of the first string
        Book b;
        b.sheet(QStringLiteral("S"), labelSst(0, 0, 1));
        b.after(table);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        QVERIFY(wb.sharedStrings().strings.size() <= 1);   // (what came before the damage stays)
        // a count of characters of 65,535 in a string of a few bytes
        Book c;
        c.sheet(QStringLiteral("S"), number(0, 0, 1));
        c.after(rec(0x00FC, W().u32(1).u32(1).u16(65535).u8(0).raw(QByteArray(10, 'x'))));
        Workbook wc;
        QVERIFY(openBook(c, &wc, &why));
        QCOMPARE(wc.sharedStrings().strings.size(), 0);
    }

    // The limits: a table of more strings than a limit, more cells in a sheet, more fonts, cell formats, names, sheets, merged ranges, rows and columns of their own,
    // shared formulas, and a shared formula that covers more than the limit.
    void theLimitsAreKept()
    {
        QString why;
        // strings
        QStringList many;
        for (int i = 0; i < 50; ++i) many << QString::number(i);
        Book b;
        b.sheet(QStringLiteral("S"), number(0, 0, 1));
        b.after(sst(many));
        Limits small;
        small.sharedStrings = 10;
        Workbook wb;
        QVERIFY(!openBook(b, &wb, &why, small));
        QVERIFY(!why.isEmpty());
        small = Limits();
        small.sharedStringChars = 20;
        QVERIFY(!openBook(b, &wb, &why, small));
        // cells in a sheet: a run of blanks with a look of their own
        Book c;
        c.sheet(QStringLiteral("S"), mulblank(0, 0, 200, 16));
        Limits cells;
        cells.cellsPerSheet = 100;
        QVERIFY(openBook(c, &wb, &why, cells));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY(!s.error.isEmpty());
        QCOMPARE(s.cells.cellCount(), qint64(100));
        Limits bytes;
        bytes.sheetBytes = 1000;
        QVERIFY(openBook(c, &wb, &why, bytes));
        QVERIFY(!wb.readSheet(0, &s));
        // fonts, formats, cell formats, names, sheets
        Book d;
        for (int i = 0; i < 20; ++i) d.globals(font(QStringLiteral("F"), 10) + xf() + format(200 + i, QStringLiteral("0")));
        d.sheet(QStringLiteral("S"), QByteArray());
        Limits lf;
        lf.fonts = 10;
        QVERIFY(!openBook(d, &wb, &why, lf));
        Limits lx;
        lx.cellXfs = 10;
        QVERIFY(!openBook(d, &wb, &why, lx));
        Limits ln;
        ln.numFmts = 10;
        QVERIFY(!openBook(d, &wb, &why, ln));
        Limits lc;
        lc.numFmtChars = 5;
        QVERIFY(!openBook(d, &wb, &why, lc));
        Book e;
        for (int i = 0; i < 20; ++i) e.sheet(QStringLiteral("S%1").arg(i), QByteArray());
        Limits ls;
        ls.sheets = 5;
        QVERIFY(!openBook(e, &wb, &why, ls));
        Book g;
        for (int i = 0; i < 20; ++i) g.after(name(QStringLiteral("N%1").arg(i), tInt(1)));
        g.sheet(QStringLiteral("S"), QByteArray());
        Limits lm;
        lm.definedNames = 5;
        QVERIFY(!openBook(g, &wb, &why, lm));
        // names that stand for more than a limit, all together: the later ones are not read
        Book h;
        for (int i = 0; i < 20; ++i) h.after(name(QStringLiteral("N%1").arg(i), tStr(QString(200, QLatin1Char('x')))));
        h.sheet(QStringLiteral("S"), QByteArray());
        Limits lh;
        lh.definedNameChars = 2100;   // (the names themselves take 2,040 of it, and what they stand for would take 4,040 more)
        QVERIFY2(openBook(h, &wb, &why, lh), qPrintable(why));
        qint64 total = 0;
        for (const DefinedName &n : wb.info().definedNames) total += n.ref.size();
        QVERIFY(total < 2500);
        QCOMPARE(wb.info().definedNames.last().ref, QStringLiteral("#REF!"));
        // merged ranges, rows and columns with sizes of their own
        Book m;
        QByteArray rows, cols, merges;
        for (int i = 0; i < 30; ++i) rows += rowRec(i, 400, 0x40) + colinfo(i, i, 3000);
        merges = mergedCells({{0, 0, 0, 1}, {1, 1, 0, 1}, {2, 2, 0, 1}, {3, 3, 0, 1}, {4, 4, 0, 1}, {5, 5, 0, 1}});
        m.sheet(QStringLiteral("S"), rows + merges);
        Limits lr;
        lr.rowSpecs = 10;
        QVERIFY(openBook(m, &wb, &why, lr));
        QVERIFY(!wb.readSheet(0, &s));
        Limits lcol;
        lcol.columnSpans = 10;
        QVERIFY(openBook(m, &wb, &why, lcol));
        QVERIFY(!wb.readSheet(0, &s));
        Limits lmg;
        lmg.merges = 3;
        QVERIFY(openBook(m, &wb, &why, lmg));
        QVERIFY(!wb.readSheet(0, &s));
        QCOMPARE(s.merges.size(), 3);
        // shared formulas
        Book sh;
        sh.sheet(QStringLiteral("S"), formulaExp(0, 0, 0, 0, 0) + shrfmla(0, 99, 0, 0, tRefN(0, 0)) + formulaExp(5, 5, 5, 5, 0) + shrfmla(5, 6, 5, 5, tRefN(0, 0)));
        Limits lsh;
        lsh.sharedRangeCells = 50;
        QVERIFY(openBook(sh, &wb, &why, lsh));
        QVERIFY(!wb.readSheet(0, &s));
        Limits lsc;
        lsc.sharedFormulas = 1;
        QVERIFY(openBook(sh, &wb, &why, lsc));
        QVERIFY(!wb.readSheet(0, &s));
        // the formulas, all together
        Book fm;
        QByteArray cellsF;
        for (int i = 0; i < 50; ++i) cellsF += formulaNumber(i, 0, tStr(QString(40, QLatin1Char('x'))), 0);
        fm.sheet(QStringLiteral("S"), cellsF);
        Limits lfm;
        lfm.expandedFormulaChars = 500;
        QVERIFY(openBook(fm, &wb, &why, lfm));
        QVERIFY(!wb.readSheet(0, &s));
    }

    // Lengths that lie: a formula whose token count is more than its bytes, a name, an array, a choice, a shared formula, a table; cells past the sheet's columns; sheets that
    // are not where they say, share a place, or overlap; a chart nested without end.
    void lengthsAndIndexesThatLie()
    {
        QString why;
        Workbook wb;
        Sheet s;
        // a formula that claims 60,000 bytes of tokens
        Book a;
        a.sheet(QStringLiteral("S"), rec(0x0006, W().u16(0).u16(0).u16(15).f64(1).u16(0).u32(0).u16(60000).raw(QByteArray(10, '\x1E'))) + number(1, 0, 5));
        QVERIFY(openBook(a, &wb, &why));
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A2")), QStringLiteral("n:5"));
        // a record whose length goes past the end of the stream
        Book b;
        b.sheet(QStringLiteral("S"), number(0, 0, 5) + W().u16(0x0203).u16(40000).u16(1).u16(1).bytes());
        QVERIFY(openBook(b, &wb, &why));
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:5"));
        // cells past the 256 columns of the sheet, and in the last row
        Book c;
        c.sheet(QStringLiteral("S"), number(0, 255, 1) + number(0, 256, 2) + number(0, 65535, 3) + number(65535, 0, 4) + number(65535, 255, 5) + blank(0, 300, 16) + mulblank(1, 250, 20, 16)
                + labelSst(0, 1000, 0) + boolErr(2, 70000 & 0xFFFF, 1, false));
        QVERIFY(openBook(c, &wb, &why));
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("IV1")), QStringLiteral("n:1"));
        QCOMPARE(shown(wb, s, QStringLiteral("A65536")), QStringLiteral("n:4"));
        QCOMPARE(shown(wb, s, QStringLiteral("IV65536")), QStringLiteral("n:5"));
        QVERIFY(s.cells.find(0, 256) == nullptr);
        QVERIFY(s.cells.find(1, 255) != nullptr);
        QVERIFY(s.cells.find(1, 256) == nullptr);
        // a sheet that is not where the workbook says it is
        QByteArray stream = Book().sheet(QStringLiteral("S"), number(0, 0, 1)).stream();
        Workbook w2;
        // (the offset of the only sheet is at the 4th byte of its record: set it to the middle of a record)
        const qsizetype boundAt = stream.indexOf(QByteArray("\x85\x00", 2));
        QVERIFY(boundAt > 0);
        QByteArray moved = stream;
        moved[boundAt + 4] = char(moved[boundAt + 4] + 1);
        QVERIFY(w2.openBinary(moved, &why));
        QVERIFY(!w2.readSheet(0, &s));
        QVERIFY(!s.error.isEmpty());
        // a place that is past the end
        moved = stream;
        moved[boundAt + 7] = char(0x7F);
        QVERIFY(w2.openBinary(moved, &why));
        QVERIFY(!w2.readSheet(0, &s));
        // two sheets at one place, and the second inside the first
        Book d;
        d.sheet(QStringLiteral("A"), number(0, 0, 1) + number(1, 0, 1)).sheet(QStringLiteral("B"), number(0, 0, 2));
        stream = d.stream();
        const qsizetype first = stream.indexOf(QByteArray("\x85\x00", 2));
        const qsizetype second = stream.indexOf(QByteArray("\x85\x00", 2), first + 4);
        QVERIFY(first > 0 && second > first);
        QByteArray same = stream;
        for (int k = 0; k < 4; ++k) same[second + 4 + k] = same[first + 4 + k];
        QVERIFY(!w2.openBinary(same, &why));
        QVERIFY2(why.contains(QLatin1String("share")), qPrintable(why));
        // a chart that is never closed, and one nested a hundred deep
        Book e;
        QByteArray nested;
        for (int i = 0; i < 100; ++i) nested += rec(0x0809, W().u16(0x600).u16(0x20).u16(1).u16(2).u32(0).u32(6));
        e.sheet(QStringLiteral("S"), number(0, 0, 1) + nested + number(1, 0, 2));
        QVERIFY(!openBook(e, &wb, &why));
        QVERIFY2(!why.isEmpty(), "a message");
        Book f;
        f.sheet(QStringLiteral("S"), number(0, 0, 1) + rec(0x0809, W().u16(0x600).u16(0x20).u16(1).u16(2).u32(0).u32(6)) + number(1, 0, 2));
        QVERIFY(openBook(f, &wb, &why));
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(shown(wb, s, QStringLiteral("A2")), QStringLiteral("none"));
        // names that claim more bytes than they have, a string table cut inside a count, a table record cut short, and a shared formula whose range is backwards
        Book g;
        QByteArray badName = name(QStringLiteral("N"), tInt(1));
        badName[6] = char(0xFF);
        badName[7] = char(0x7F);   // the length of its formula
        g.after(badName + rec(0x00FC, W().u8(1)) + rec(0x0017, W().u16(60000)) + rec(0x01AE, W().u16(60000).u16(5)));
        g.sheet(QStringLiteral("S"), rec(0x0236, W().u16(5)) + rec(0x04BC, W().u16(9).u16(1).u8(9).u8(1).u8(0).u8(1).u16(1).u8(0x24)) + rec(0x0221, W().u16(5)) + number(0, 0, 1));
        QVERIFY(openBook(g, &wb, &why));
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1"));
    }

    // A formula that is a chain of tens of thousands of terms, one nested as deep as its bytes allow, and one that is a name that stands for itself: read in a moment, with the
    // engine's own bounds on the rest.
    void deepAndLongFormulasAreRead()
    {
        QByteArray chain = tRef(0, 0);
        for (int i = 0; i < 1500; ++i) chain += tRef(0, 0) + tOp(0x03);
        QByteArray nest = tInt(1);
        for (int i = 0; i < 2000; ++i) nest = nest + tFuncVar(4, 1);
        QByteArray parens = tInt(1);
        for (int i = 0; i < 8000; ++i) parens += tOp(0x15);
        Book b;
        b.after(name(QStringLiteral("Self"), tName(1) + tInt(1) + tOp(0x03)));
        b.sheet(QStringLiteral("S"), number(0, 0, 1) + formulaNumber(1, 0, chain, 0) + formulaNumber(2, 0, nest, 0) + formulaNumber(3, 0, parens, 0) + formulaNumber(4, 0, tName(1), 0));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        jp::office::sheet::CalcOptions options;
        options.keepApart = true;
        jp::office::sheet::Calculation calc(wb, options);
        QVERIFY(calc.load());
        calc.recalculate();
        QCOMPARE(calc.cell(0, 1, 0).debug(), QStringLiteral("n:1501"));   // (the cell added to itself's neighbors: 1,501 times 1)
        QCOMPARE(calc.cell(0, 2, 0).debug(), QStringLiteral("n:0"));      // (calls nested two thousand deep: past the engine's bound, so the result the file stored stays)
        QCOMPARE(calc.cell(0, 3, 0).debug(), QStringLiteral("n:0"));      // (a number in eight thousand parentheses: the same)
        QCOMPARE(calc.cell(0, 4, 0).debug(), QStringLiteral("n:0"));      // (a name that stands for itself: kept apart, with the result the file stored)
        QVERIFY(!calc.isDirty());
    }
};

JP_TEST_CLASS(SheetXlsHostileTests)
#include "tst_sheet_xlshostile.moc"
