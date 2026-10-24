// Tests of editing cells in a calculation: the content of a cell taken and put back whole (for undo), the search for the formulas that read an edited cell left to the recalculation
// (so that an edit costs what moving the cell does, however many formulas read it), a cell's format, texts in the shared strings, and the workbook's additions to its shared
// strings and styles. Counted work only. Every workbook is built in memory.
#include "sheethelpers.h"

#include "sheet/calculation.h"

#include <atomic>
#include <memory>

namespace {

QString cellXml(const QString &ref, const QString &formula, const QString &value = QString())
{
    return QStringLiteral("<c r=\"%1\"><f>%2</f>%3</c>").arg(ref, formula.toHtmlEscaped(), value.isEmpty() ? QString() : QStringLiteral("<v>%1</v>").arg(value.toHtmlEscaped()));
}

QString constant(const QString &ref, const QString &value, int style = 0)
{
    return QStringLiteral("<c r=\"%1\"%3><v>%2</v></c>").arg(ref, value, style ? QStringLiteral(" s=\"%1\"").arg(style) : QString());
}

QString rowXml(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool open(const QByteArray &zip)
    {
        QString why;
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        calc.reset(new Calculation(book));
        return calc->load();
    }
    static quint32 row(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return r;
    }
    static quint32 col(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return c;
    }
    QString at(const QString &ref, int sheet = 0) const { return calc->cell(sheet, row(ref), col(ref)).debug(); }
    Calculation::CellContent content(const QString &ref) const { return calc->content(0, row(ref), col(ref)); }
    void finish() { while (!calc->recalculate().finished) {} }
};

} // namespace

class SheetEditTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // An edit that is left to the recalculation does not look for the formulas that read the cell: it costs what putting the cell in its block does, and the search is done, a chunk
    // at a time, by the recalculation.
    void anEditLeavesTheSearchForReadersToTheRecalculation()
    {
        QString rows = rowXml(1, constant("A1", "1"));
        for (int i = 2; i <= 50; ++i) rows += rowXml(i, cellXml(QStringLiteral("B%1").arg(i), QStringLiteral("A1+%1").arg(i)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        b.finish();
        QCOMPARE(b.at("B10"), QStringLiteral("n:11"));
        QVERIFY(!b.calc->isDirty());
        const qint64 queried = b.calc->graph().queryWork();
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(100), Calculation::Marking::Later));
        QCOMPARE(b.calc->graph().queryWork(), queried);   // nothing was looked for
        QCOMPARE(b.calc->dirtyCount(), qint64(0));
        QVERIFY(b.calc->isDirty());                       // but the recalculation has it to do
        QCOMPARE(b.at("A1"), QStringLiteral("n:100"));    // the cell has its value at once
        QCOMPARE(b.at("B10"), QStringLiteral("n:11"));    // and what reads it the old one, until the recalculation
        b.finish();
        QVERIFY(b.calc->graph().queryWork() > queried);
        QCOMPARE(b.at("B10"), QStringLiteral("n:110"));
        QCOMPARE(b.at("B50"), QStringLiteral("n:150"));
        QVERIFY(!b.calc->isDirty());
        // the same with the search done at once (as before)
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(7)));
        QCOMPARE(b.calc->dirtyCount(), qint64(49));
        b.finish();
        QCOMPARE(b.at("B10"), QStringLiteral("n:17"));
    }

    // Many edits one after another, with no recalculation between: each is the cell put in, and the one recalculation after them works out everything the edits touched, once.
    void manyEditsAreWorkedOutByOneRecalculation()
    {
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rowXml(1, constant("A1", "1") + constant("B1", "2") + cellXml("C1", "A1+B1")) + rowXml(2, cellXml("C2", "C1*2")))})));
        b.finish();
        QCOMPARE(b.at("C2"), QStringLiteral("n:6"));
        const Calculation::Progress none = [&] {
            b.calc->setValue(0, 0, 0, Value::number(10), Calculation::Marking::Later);
            b.calc->setValue(0, 0, 1, Value::number(20), Calculation::Marking::Later);
            b.calc->setValue(0, 0, 0, Value::number(100), Calculation::Marking::Later);
            Calculation::Progress p = b.calc->recalculate();
            return p;
        }();
        QVERIFY(none.finished);
        QCOMPARE(none.evaluated, qint64(2));   // C1 and C2, once each
        QCOMPARE(b.at("C1"), QStringLiteral("n:120"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:240"));
    }

    // An edit made while a recalculation is part way: what it had begun is worked out again, the edit is in, and the result is that of a recalculation from the start.
    void anEditInTheMiddleOfARecalculationIsAbsorbed()
    {
        QString rows = rowXml(1, constant("A1", "1"));
        for (int i = 2; i <= 3000; ++i) rows += rowXml(i, cellXml(QStringLiteral("B%1").arg(i), QStringLiteral("A1*%1").arg(i)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QVERIFY(!b.calc->recalculate(2000).finished);   // part way
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(3), Calculation::Marking::Later));
        b.finish();
        QCOMPARE(b.at("B3000"), QStringLiteral("n:9000"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:6"));
        QCOMPARE(b.calc->dirtyCount(), qint64(0));
        // and again, with the edit made while the marking of the readers is part way
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(4), Calculation::Marking::Later));
        QVERIFY(!b.calc->recalculate(1).finished);
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(5), Calculation::Marking::Later));
        b.finish();
        QCOMPARE(b.at("B3000"), QStringLiteral("n:15000"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:10"));
    }

    // A cell that very many formulas read: the edit does not look for them (no search, no marking); the recalculation does, in slices, and the slices that work them out are each
    // bounded by the work they are given.
    void aCellReadByManyFormulasCostsTheEditNothing()
    {
        const int n = 20000;
        QString rows = rowXml(1, constant("A1", "1"));
        for (int i = 2; i <= n; ++i) rows += rowXml(i, cellXml(QStringLiteral("B%1").arg(i), QStringLiteral("A$1+%1").arg(i)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        b.finish();
        const qint64 queried = b.calc->graph().queryWork();
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(5), Calculation::Marking::Later));
        QCOMPARE(b.calc->graph().queryWork(), queried);
        QCOMPARE(b.calc->dirtyCount(), qint64(0));
        // the slices that follow: each stays near the work it is given, but the one that makes the search for the readers of the cell (one step, as small as it can be made: what the graph gave is
        // one query of that many); the marking of what it found, and the asking of the graph about each of the cells found (which read nothing here), go a chunk at a time
        std::vector<qint64> jobs;
        qint64 slices = 0, evaluated = 0;
        for (;;) {
            const Calculation::Progress p = b.calc->recalculate(500);
            ++slices;
            jobs.push_back(p.jobItems);
            QVERIFY2(p.work <= 500 + 1024, qPrintable(QString::number(p.work)));
            evaluated += p.evaluated;
            if (p.finished) break;
            QVERIFY(slices < 100000);
        }
        std::sort(jobs.begin(), jobs.end());
        QVERIFY2(slices > 10, qPrintable(QString::number(slices)));
        QVERIFY2(jobs.back() >= n - 1, qPrintable(QString::number(jobs.back())));                          // the search is one step
        QVERIFY2(jobs[jobs.size() - 2] <= 500 + 1024, qPrintable(QString::number(jobs[jobs.size() - 2])));   // and no other slice goes over what it was given
        QCOMPARE(evaluated, qint64(n - 1));
        QCOMPARE(b.at("B20000"), QStringLiteral("n:20005"));
        QVERIFY(b.calc->graph().queryWork() >= queried + n - 1);
    }

    // A recalculation stopped while it marks what reads an edited cell, and then settled at once (what the window's hold does), has taken in hand what the stop found twice: the second
    // taking must not drop the first's list, or the formulas the first had marked stay marked and are never worked out (nothing that is worked out reads them, so nothing pulls them in).
    void aStopThatIsSettledAtOnceKeepsWhatTheFirstStopTookInHand()
    {
        const int n = 3000;
        QString rows = rowXml(1, constant("A1", "1"));
        for (int i = 1; i <= n; ++i) rows += rowXml(i + 1, cellXml(QStringLiteral("B%1").arg(i + 1), QStringLiteral("A$1*%1").arg(i)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        b.finish();
        QCOMPARE(b.at("B3001"), QStringLiteral("n:3000"));
        std::atomic<bool> stop{false};
        for (int round = 0; round < 6; ++round) {
            QVERIFY(b.calc->setValue(0, 0, 0, Value::number(5 + round), Calculation::Marking::Later));
            // part way through marking what reads A1 (some are marked, not all)
            for (int slice = 0; slice < 2 + round % 2; ++slice) QVERIFY(!b.calc->recalculate(300).finished);
            QVERIFY(b.calc->dirtyCount() > 0 && b.calc->dirtyCount() < n);
            stop = true;
            QVERIFY(b.calc->recalculate(0, &stop).cancelled);
            b.calc->settle();
            stop = false;
            b.finish();
            QCOMPARE(b.at("B2"), QStringLiteral("n:%1").arg(5 + round));
            QCOMPARE(b.at("B100"), QStringLiteral("n:%1").arg((5 + round) * 99));
            QCOMPARE(b.at("B3001"), QStringLiteral("n:%1").arg((5 + round) * 3000));
        }
        // the same where the settling comes with the next edit (every edit settles what is left): all that was marked is worked out
        for (int round = 0; round < 6; ++round) {
            QVERIFY(b.calc->setValue(0, 0, 0, Value::number(20 + round), Calculation::Marking::Later));
            for (int slice = 0; slice < 2 + round % 2; ++slice) QVERIFY(!b.calc->recalculate(300).finished);
            stop = true;
            QVERIFY(b.calc->recalculate(0, &stop).cancelled);
            stop = false;
            QVERIFY(b.calc->setValue(0, 3, 5, Value::number(1), Calculation::Marking::Later));
        }
        b.finish();
        QCOMPARE(b.at("B2"), QStringLiteral("n:25"));
        QCOMPARE(b.at("B3001"), QStringLiteral("n:75000"));
        QCOMPARE(b.calc->dirtyCount(), qint64(0));
    }

    // What a cell holds is taken whole and put back whole: a number, a text, a formula, a cell with only a format, one that is not there; the formulas that read it follow.
    void aCellsContentIsTakenAndPutBackWhole()
    {
        const QByteArray ss = sharedStringsXml({QStringLiteral("<t>shared</t>")});
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rowXml(1, constant("A1", "5", 3) + QStringLiteral("<c r=\"B1\" t=\"s\"><v>0</v></c>") + cellXml("C1", "A1*2") + QStringLiteral("<c r=\"D1\" s=\"4\"/>")
                                                        + QStringLiteral("<c r=\"E1\" t=\"inlineStr\"><is><t>inline</t></is></c>") + QStringLiteral("<c r=\"F1\" t=\"b\"><v>1</v></c>")
                                                        + QStringLiteral("<c r=\"G1\" t=\"e\"><v>#N/A</v></c>")) + rowXml(2, cellXml("A2", "C1+1")))},
                                  ss)));
        b.finish();
        struct Case {
            const char *ref;
            bool formula;
            QString text;
        };
        const Calculation::CellContent a1 = b.content("A1"), b1 = b.content("B1"), c1 = b.content("C1"), d1 = b.content("D1"), e1 = b.content("E1"), f1 = b.content("F1"), g1 = b.content("G1"), z9 = b.content("Z9");
        QVERIFY(a1.exists && !a1.isFormula && a1.value == Value::number(5) && a1.style == 3);
        QVERIFY(b1.exists && b1.value == Value::text(QStringLiteral("shared")) && b1.sharedString == 0);
        QVERIFY(c1.exists && c1.isFormula && c1.formula == QStringLiteral("A1*2"));
        QVERIFY(d1.exists && !d1.isFormula && d1.value.isEmpty() && d1.style == 4);
        QVERIFY(e1.exists && e1.value == Value::text(QStringLiteral("inline")));
        QVERIFY(f1.exists && f1.value == Value::boolean(true));
        QVERIFY(g1.exists && g1.value == Value::error(ErrorKind::NA));
        QVERIFY(!z9.exists);
        // change them all, then put them back
        QVERIFY(b.calc->put(0, 0, 0, Calculation::CellContent{true, true, true, QStringLiteral("100"), Value(), -1, 0}, Calculation::Marking::Later));
        QVERIFY(b.calc->put(0, 0, 2, Calculation::CellContent{true, true, false, QString(), Value::text(QStringLiteral("now text")), -1, 9}, Calculation::Marking::Later));
        QVERIFY(b.calc->put(0, 0, 3, Calculation::CellContent(), Calculation::Marking::Later));
        QVERIFY(b.calc->put(0, 8, 25, Calculation::CellContent{true, true, false, QString(), Value::number(1), -1, 0}, Calculation::Marking::Later));
        b.finish();
        QCOMPARE(b.at("A1"), QStringLiteral("n:100"));
        QCOMPARE(b.at("C1"), QStringLiteral("s:now text"));
        QCOMPARE(b.at("A2"), QStringLiteral("e:#VALUE!"));
        QVERIFY(!b.content("D1").exists);
        for (const auto &pair : {std::make_pair(QStringLiteral("A1"), a1), std::make_pair(QStringLiteral("C1"), c1), std::make_pair(QStringLiteral("D1"), d1)}) {
            const QString ref = pair.first;
            QVERIFY(b.calc->put(0, Book::row(ref), Book::col(ref), pair.second, Calculation::Marking::Later));
        }
        QVERIFY(b.calc->put(0, 8, 25, z9, Calculation::Marking::Later));
        b.finish();
        QCOMPARE(b.at("A1"), QStringLiteral("n:5"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:11"));
        QVERIFY(b.content("A1") == a1);
        QVERIFY(b.content("C1") == c1);
        QVERIFY(b.content("D1") == d1);
        QVERIFY(!b.content("Z9").exists);
        QVERIFY(!b.calc->content(0, 8, 25).exists);
    }

    // A text put in the shared strings is the cell's by its place; the cell's format is changed alone, and a cell with nothing and the first format goes.
    void textsAndFormatsAreChangedOnTheirOwn()
    {
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rowXml(1, constant("A1", "5", 2) + constant("B1", "6")))}, sharedStringsXml({QStringLiteral("<t>first</t>")}))));
        b.finish();
        const qint32 place = b.book.addSharedString(QStringLiteral("typed"));
        QCOMPARE(place, 1);
        QCOMPARE(b.book.addSharedString(QStringLiteral("typed")), 1);   // (the same text is the same place)
        QVERIFY(b.calc->setValue(0, 0, 1, Value::text(QStringLiteral("typed")), Calculation::Marking::Later, place));
        QCOMPARE(b.at("B1"), QStringLiteral("s:typed"));
        QCOMPARE(b.calc->sheet(0).cells.find(0, 1)->type, CellType::SharedString);
        QCOMPARE(b.calc->sheet(0).texts.size(), 0);   // (no text kept in the sheet)
        QCOMPARE(b.content("B1").sharedString, 1);
        QCOMPARE(b.book.addedSharedStrings(), QStringList({QStringLiteral("typed")}));
        // the format
        QVERIFY(b.calc->setStyle(0, 0, 0, 7));
        QCOMPARE(b.content("A1").style, 7u);
        QCOMPARE(b.at("A1"), QStringLiteral("n:5"));
        QVERIFY(b.calc->setStyle(0, 20, 20, 3));
        QVERIFY(b.content("U21").exists);
        QCOMPARE(b.content("U21").style, 3u);
        QVERIFY(b.calc->setStyle(0, 20, 20, 0));
        QVERIFY(!b.content("U21").exists);
        // (the extent of the sheet grows with a cell put outside it)
        QVERIFY(b.calc->setValue(0, 99, 5, Value::number(1), Calculation::Marking::Later));
        QCOMPARE(b.calc->sheet(0).extentRows, 100u);
        QCOMPARE(b.calc->sheet(0).extentCols, 21u);   // (the format of U21 reached that far, and went; the extent does not shrink)
    }

    // A formula with a function the engine lacks keeps the result the file stored until something it reads changes; then it has #NAME?, and what reads it is worked out from that. One
    // typed in a cell has #NAME? at once.
    void aFormulaWithAFunctionTheEngineLacksHasNameWhenWhatItReadsChanges()
    {
        const QString rows = rowXml(1, constant("A1", "1") + cellXml("B1", "NOSUCHFUNCTION(A1)", "42") + cellXml("C1", "B1+1") + cellXml("D1", "NOSUCHFUNCTION(5)", "7"));
        for (const auto marking : {Calculation::Marking::Later, Calculation::Marking::Now}) {
            Book b;
            CalcOptions options;
            options.keepApart = true;
            options.workOutWhatHasNoResult = true;
            QString why;
            QVERIFY(b.package.readLazy(buildXlsx({worksheet(rows)}), &why) && b.book.open(b.package, &why));
            b.calc.reset(new Calculation(b.book, options));
            QVERIFY(b.calc->load());
            b.finish();
            QCOMPARE(b.at("B1"), QStringLiteral("n:42"));      // as the file stored it
            QCOMPARE(b.at("C1"), QStringLiteral("n:43"));      // and what reads it is worked out from that
            QVERIFY(b.calc->setValue(0, 0, 3, Value::number(9), marking));    // D1 is read by nothing, and reads nothing: left alone
            b.finish();
            QCOMPARE(b.at("B1"), QStringLiteral("n:42"));
            QVERIFY(b.calc->setValue(0, 0, 0, Value::number(2), marking));
            b.finish();
            QCOMPARE(b.at("B1"), QStringLiteral("e:#NAME?"));
            QCOMPARE(b.at("C1"), QStringLiteral("e:#NAME?"));
            QVERIFY(b.calc->isTainted(0, 0, 1));
            QVERIFY(b.calc->setValue(0, 0, 0, Value::number(3), marking));      // (changed again: nothing more to say)
            b.finish();
            QCOMPARE(b.at("B1"), QStringLiteral("e:#NAME?"));
            // one typed has the error at once, and what reads it
            QVERIFY(b.calc->setFormula(0, 5, 0, QStringLiteral("NOSUCHFUNCTION(1)"), marking));
            QVERIFY(b.calc->setFormula(0, 5, 1, QStringLiteral("A6&\"x\""), marking));
            b.finish();
            QCOMPARE(b.at("A6"), QStringLiteral("e:#NAME?"));
            QCOMPARE(b.at("B6"), QStringLiteral("e:#NAME?"));
            QCOMPARE(b.calc->keptCount() >= 2, true);
        }
    }

    // The cells of an array and a data table are not changed alone.
    void aCellOfAnArrayIsNotChangedAlone()
    {
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rowXml(1, constant("A1", "1") + constant("B1", "2")) + rowXml(2, QStringLiteral("<c r=\"C1\"><f t=\"array\" ref=\"C1:C2\">A1:B1*2</f><v>2</v></c>")))})));
        b.finish();
        QVERIFY(!b.content("C1").editable);
        QVERIFY(!b.calc->setValue(0, 0, 2, Value::number(1), Calculation::Marking::Later));
        QVERIFY(!b.calc->setFormula(0, 0, 2, QStringLiteral("1"), Calculation::Marking::Later));
        QVERIFY(!b.calc->put(0, 0, 2, Calculation::CellContent{true, true, false, QString(), Value::number(1), -1, 0}));
    }

    // The workbook's additions: texts at the end of the shared strings, a number format, and a cell format that is an old one with another number format (made once).
    void theWorkbookTakesAdditionsToItsStringsAndStyles()
    {
        const QByteArray styles = stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"0.0&quot; kg&quot;\"/>"), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\"><alignment horizontal=\"center\"/></xf>"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rowXml(1, constant("A1", "5", 1)))}, sharedStringsXml({QStringLiteral("<t>one</t>")}), QString(), QString(), styles)));
        QCOMPARE(b.book.sharedStringsInFile(), qint64(1));
        QCOMPARE(b.book.cellFormatsInFile(), qint64(2));
        // number formats: built-in ones by their code, the file's own by theirs, a new one a new id
        QCOMPARE(b.book.numberFormatId(QStringLiteral("0%")), 9);
        QCOMPARE(b.book.numberFormatId(QStringLiteral("m/d/yyyy")), 14);
        QCOMPARE(b.book.numberFormatId(QStringLiteral("0.0\" kg\"")), 164);
        QCOMPARE(b.book.numberFormatId(QStringLiteral("0.000%")), 165);
        QCOMPARE(b.book.numberFormatId(QStringLiteral("0.000%")), 165);
        QCOMPARE(b.book.styles().numFmts.value(165), QStringLiteral("0.000%"));
        // cell formats
        const quint32 pct = b.book.formatWithNumberFormat(0, 9);
        QCOMPARE(pct, 2u);
        QCOMPARE(b.book.formatWithNumberFormat(0, 9), 2u);        // (made once)
        QCOMPARE(b.book.formatWithNumberFormat(0, 0), 0u);        // (already so)
        const quint32 centered = b.book.formatWithNumberFormat(1, 9);
        QCOMPARE(centered, 3u);
        QCOMPARE(b.book.styles().formats[3].numFmtId, 9);
        QCOMPARE(b.book.styles().numFmtId(3), 9);
        QCOMPARE(b.book.addedCellFormats().size(), 2);
        QCOMPARE(b.book.addedCellFormats()[1].base, 1u);
        QCOMPARE(b.book.addedNumberFormats().size(), 1);
        // after a save the additions are in the file
        b.book.addSharedString(QStringLiteral("x"));
        b.book.savedTo(b.package, 1, 1, 1);
        QCOMPARE(b.book.sharedStringsInFile(), qint64(2));
        QCOMPARE(b.book.cellFormatsInFile(), qint64(3));
        QCOMPARE(b.book.addedCellFormats().size(), 1);
        QCOMPARE(b.book.formatWithNumberFormat(1, 9), 3u);        // (still the one that was made)
        QCOMPARE(b.book.addedSharedStrings().size(), 0);
    }
};

JP_TEST_CLASS(SheetEditTests)
#include "tst_sheet_edit.moc"
