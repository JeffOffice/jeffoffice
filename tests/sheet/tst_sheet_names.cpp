// Tests of the defined names of a calculation (what uses a name follows it when it is renamed, changed, or taken away), of the rules a name must keep, of going round circles, and of the
// workbook part written with names and calculation settings changed. Every workbook is built in memory.
#include "sheethelpers.h"

#include "sheet/calculation.h"
#include "sheet/namerefs.h"
#include "sheet/xlsxsave.h"

#include <memory>

namespace {

QString cellXml(const QString &ref, const QString &formula, const QString &value = QString())
{
    return QStringLiteral("<c r=\"%1\"><f>%2</f>%3</c>").arg(ref, formula.toHtmlEscaped(), value.isEmpty() ? QString() : QStringLiteral("<v>%1</v>").arg(value));
}
QString constant(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }
QString rowXml(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool open(const QByteArray &zip)
    {
        QString why;
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.goRoundCircles = true;
        calc.reset(new Calculation(book, options));
        return calc->load() && calc->recalculate().finished;
    }
    QString at(const QString &ref, int sheet = 0) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return calc->cell(sheet, r, c).debug();
    }
    QString formula(const QString &ref, int sheet = 0) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return calc->content(sheet, r, c).formula;
    }
    bool settle() { return calc->recalculate().finished; }
    bool circular(const QString &ref, int sheet = 0) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return calc->isCircular(sheet, r, c);
    }
};

QByteArray nameBook(const QString &calcPr = QString())
{
    const QString one = rowXml(1, cellXml("A1", "Rate*2") + constant("C1", "5")) + rowXml(2, constant("C2", "7"));
    const QString two = rowXml(1, cellXml("A1", "Rate+1") + constant("C1", "100"));
    return buildXlsx({worksheet(one), worksheet(two)}, QByteArray(), QString(), calcPr, QByteArray(), QStringList(), QStringLiteral("<definedName name=\"Rate\">Sheet1!$C$1</definedName>"));
}

} // namespace

class SheetNameTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void aNameThatIsNotAllowedSaysWhy()
    {
        for (const char *good : {"Rate", "_hidden", "Total.Rate_2", "\\path", "Tax_2024", "ABC"}) QVERIFY2(nameProblem(QString::fromLatin1(good)).isEmpty(), good);
        for (const char *bad : {"", "1abc", "my name", "A1", "XFD1048576", "R1C1", "R", "c", "RC", "R2", "a-b", "x/y"}) QVERIFY2(!nameProblem(QString::fromLatin1(bad)).isEmpty(), bad);
        QVERIFY(!nameProblem(QString(256, QLatin1Char('a'))).isEmpty());
        // a name that begins as an address in the other style does (R, a number, then more; C and a number, then more; RC, a number, then more) is one the other program will not open the file for
        for (const char *bad : {"R1x", "C1x", "R1Cx", "R1C1x", "R1C1_", "R2D2", "C3PO", "RC1x", "r1c1.a"}) QVERIFY2(!nameProblem(QString::fromLatin1(bad)).isEmpty(), bad);
        for (const char *good : {"Rx", "Cx", "RCx", "RRC", "Rate2", "Cost1", "_R1x", "R_1x"}) QVERIFY2(nameProblem(QString::fromLatin1(good)).isEmpty(), good);
        QVERIFY(nameProblem(QString(255, QLatin1Char('a'))).isEmpty());
        QVERIFY(nameProblem(QStringLiteral("my name")).contains(QStringLiteral("space")));
    }

    void renamingFollowsUsesOfANameAndNothingElse()
    {
        const QString f = QStringLiteral("SUM(Rate*2,Data!Rate,\"Rate\",Rate(1),Table1[Rate],$A$1,'Rate sheet'!A1)+rate");
        bool changed = false;
        QCOMPARE(renameNameInFormula(f, QStringLiteral("Rate"), QStringLiteral("Tax"), nullptr, &changed), QStringLiteral("SUM(Tax*2,Data!Tax,\"Rate\",Rate(1),Table1[Rate],$A$1,'Rate sheet'!A1)+Tax"));
        QVERIFY(changed);
        // only the uses with no sheet in front, or only those with one
        QCOMPARE(renameNameInFormula(f, QStringLiteral("Rate"), QStringLiteral("Tax"), [](const QString &s) { return s.isEmpty(); }),
                 QStringLiteral("SUM(Tax*2,Data!Rate,\"Rate\",Rate(1),Table1[Rate],$A$1,'Rate sheet'!A1)+Tax"));
        QCOMPARE(renameNameInFormula(f, QStringLiteral("Rate"), QStringLiteral("Tax"), [](const QString &s) { return s == QLatin1String("Data"); }),
                 QStringLiteral("SUM(Rate*2,Data!Tax,\"Rate\",Rate(1),Table1[Rate],$A$1,'Rate sheet'!A1)+rate"));
        // part of an address is not a name, whatever the name is
        QCOMPARE(renameNameInFormula(QStringLiteral("$A$1+A:A+B2"), QStringLiteral("A"), QStringLiteral("Z"), nullptr, &changed), QStringLiteral("$A$1+A:A+B2"));
        QVERIFY(!changed);
        QVERIFY(formulaUsesName(f, QStringLiteral("RATE")));
        QVERIFY(!formulaUsesName(QStringLiteral("SUM(Rates)+\"Rate\""), QStringLiteral("Rate")));
    }

    void aNameGivenAnotherTextOrTakenAwayWorksItsFormulasOutAgain()
    {
        Book b;
        QVERIFY(b.open(nameBook()));
        QCOMPARE(b.at("A1"), QStringLiteral("n:10"));
        QString text;
        QVERIFY(b.calc->nameText(-1, QStringLiteral("rate"), &text));
        QCOMPARE(text, QStringLiteral("Sheet1!$C$1"));
        QVERIFY(!b.calc->nameText(0, QStringLiteral("Rate"), nullptr));
        b.calc->setNameText(-1, QStringLiteral("Rate"), QStringLiteral("Sheet1!$C$2"));
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:14"));
        b.calc->setNameText(-1, QStringLiteral("Rate"), QString());
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("e:#NAME?"));
        QVERIFY(!b.calc->nameText(-1, QStringLiteral("Rate"), nullptr));
        // a name made
        b.calc->setNameText(-1, QStringLiteral("RATE"), QStringLiteral("Sheet1!$C$1"));
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:10"));
    }

    void aRenamedNameIsFollowedByTheFormulasThatUseItAndUndoPutsThemBack()
    {
        Book b;
        QVERIFY(b.open(nameBook()));
        Calculation::SheetRewrite rec;
        b.calc->renameName(-1, QStringLiteral("Rate"), QStringLiteral("Tax"), &rec);
        QVERIFY(b.settle());
        QCOMPARE(b.formula("A1"), QStringLiteral("Tax*2"));
        QCOMPARE(b.formula("A1", 1), QStringLiteral("Tax+1"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:6"));
        QVERIFY(b.calc->nameText(-1, QStringLiteral("Tax"), nullptr));
        QVERIFY(!b.calc->nameText(-1, QStringLiteral("Rate"), nullptr));
        QCOMPARE(rec.rewritten.size(), size_t(2));
        b.calc->undoRenameName(-1, QStringLiteral("Rate"), QStringLiteral("Tax"), rec);
        QVERIFY(b.settle());
        QCOMPARE(b.formula("A1"), QStringLiteral("Rate*2"));
        QCOMPARE(b.formula("A1", 1), QStringLiteral("Rate+1"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:10"));
        QVERIFY(b.calc->nameText(-1, QStringLiteral("Rate"), nullptr));
        QVERIFY(!b.calc->nameText(-1, QStringLiteral("Tax"), nullptr));
    }

    void aSheetsOwnNameHidesTheWorkbooksOfTheSameTextAndIsNotRenamedWithIt()
    {
        const QString one = rowXml(1, cellXml("A1", "Rate*2") + constant("C1", "5"));
        const QString two = rowXml(1, cellXml("A1", "Rate+1") + cellXml("B1", "Sheet1!Rate+1") + constant("C1", "100"));
        const QByteArray file = buildXlsx({worksheet(one), worksheet(two)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(),
                                          QStringLiteral("<definedName name=\"Rate\">Sheet1!$C$1</definedName><definedName name=\"Rate\" localSheetId=\"1\">Sheet2!$C$1</definedName>"));
        Book b;
        QVERIFY(b.open(file));
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:101"));   // the sheet's own
        QCOMPARE(b.at("A1"), QStringLiteral("n:10"));
        Calculation::SheetRewrite rec;
        b.calc->renameName(-1, QStringLiteral("Rate"), QStringLiteral("Tax"), &rec);   // the workbook's
        QVERIFY(b.settle());
        QCOMPARE(b.formula("A1"), QStringLiteral("Tax*2"));
        QCOMPARE(b.formula("A1", 1), QStringLiteral("Rate+1"));   // (Sheet2 reads its own name)
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:101"));
        QVERIFY(b.calc->nameText(1, QStringLiteral("Rate"), nullptr));
        // the sheet's own, renamed: Sheet2's formula follows, and the one that names the sheet in front of it too (Sheet1 has none of that name now)
        Calculation::SheetRewrite rec2;
        b.calc->renameName(1, QStringLiteral("Rate"), QStringLiteral("Own"), &rec2);
        QVERIFY(b.settle());
        QCOMPARE(b.formula("A1", 1), QStringLiteral("Own+1"));
        QCOMPARE(b.formula("B1", 1), QStringLiteral("Sheet1!Rate+1"));   // (Sheet1!Rate is the workbook's name, now Tax: not found, so it was not Sheet1's own)
    }

    void aRulesFormulaFollowsARenamedName()
    {
        const QString one = rowXml(1, constant("A1", "5") + constant("C1", "5"));
        const QByteArray file = buildXlsx({worksheet(one, QStringLiteral("<dataValidations count=\"1\"><dataValidation type=\"whole\" operator=\"lessThan\" sqref=\"A1\"><formula1>Rate</formula1></dataValidation></dataValidations>"))},
                                          QByteArray(), QString(), QString(), QByteArray(), QStringList(), QStringLiteral("<definedName name=\"Rate\">Sheet1!$C$1</definedName>"));
        Book b;
        QVERIFY(b.open(file));
        QCOMPARE(b.calc->sheet(0).validations.size(), 1);
        Calculation::SheetRewrite rec;
        b.calc->renameName(-1, QStringLiteral("Rate"), QStringLiteral("Tax"), &rec);
        QCOMPARE(b.calc->sheet(0).validations[0].formula1, QStringLiteral("Tax"));
        b.calc->undoRenameName(-1, QStringLiteral("Rate"), QStringLiteral("Tax"), rec);
        QCOMPARE(b.calc->sheet(0).validations[0].formula1, QStringLiteral("Rate"));
    }

    // ---- circles
    void aCircleHasTheResultZeroAndIsListedInTheOrderOfTheCells()
    {
        const QString rows = rowXml(1, cellXml("A1", "B1+1") + cellXml("B1", "A1*0.5")) + rowXml(3, cellXml("A3", "1+1"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QVERIFY(b.calc->isCircular(0, 0, 0));
        QVERIFY(!b.calc->isCircular(0, 2, 0));
        std::vector<Calculation::CircularCell> cells;
        QCOMPARE(b.calc->circularCells(&cells, 100), qint64(2));
        QCOMPARE(cells.size(), size_t(2));
        QCOMPARE(cells[0].col, 0u);
        QCOMPARE(cells[1].col, 1u);
        // cut to what is asked for, and the number still the whole
        cells.clear();
        QCOMPARE(b.calc->circularCells(&cells, 1), qint64(2));
        QCOMPARE(cells.size(), size_t(1));
        QCOMPARE(b.at("A1"), QStringLiteral("n:0"));
    }

    void aCircleIsBrokenByChangingACellInIt()
    {
        const QString rows = rowXml(1, cellXml("A1", "B1+1") + cellXml("B1", "A1*2") + cellXml("C1", "A1+B1"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QCOMPARE(b.calc->circularCells(nullptr, 0), qint64(2));
        QVERIFY(b.calc->setValue(0, 0, 1, Value::number(3)));   // B1 is a number now
        QVERIFY(b.settle());
        QCOMPARE(b.calc->circularCells(nullptr, 0), qint64(0));
        QVERIFY(!b.circular("A1"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:4"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:7"));
        // made again
        QVERIFY(b.calc->setFormula(0, 0, 1, QStringLiteral("A1*2")));
        QVERIFY(b.settle());
        QCOMPARE(b.calc->circularCells(nullptr, 0), qint64(2));
    }

    void aWorkbookThatGoesRoundCirclesWorksThemOutUntilTheyAreStillOrTheRoundsAreUsed()
    {
        // A1 = B1+1 and B1 = A1*0.5, worked out in that order from nothing: after one round 1 and 0.5, after two 1.5 and 0.75, after three 1.75 and 0.875 (and at the end 2 and 1)
        const QString rows = rowXml(1, cellXml("A1", "B1+1") + cellXml("B1", "A1*0.5")) + rowXml(3, cellXml("C3", "A1+B1"));
        Book three;
        QVERIFY(three.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"3\" iterateDelta=\"0.0000001\""))));
        QVERIFY(three.calc->iterating());
        QCOMPARE(three.calc->iterationCount(), 3);
        QCOMPARE(three.at("A1"), QStringLiteral("n:1.75"));
        QCOMPARE(three.at("B1"), QStringLiteral("n:0.875"));
        QCOMPARE(three.at("C3"), QStringLiteral("n:2.625"));   // (what reads a circle reads its values)
        QVERIFY(!three.calc->isCircular(0, 0, 0));
        std::vector<Calculation::CircularCell> cells;
        QCOMPARE(three.calc->circularCells(&cells, 10), qint64(0));
        // stopped by the change: after the second round no value changed by as much as 0.6 (0.5, and 0.25)
        Book still;
        QVERIFY(still.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"100\" iterateDelta=\"0.6\""))));
        QCOMPARE(still.at("A1"), QStringLiteral("n:1.5"));
        QCOMPARE(still.at("B1"), QStringLiteral("n:0.75"));
        // a hundred rounds with a small change: near 2 and 1
        Book many;
        QVERIFY(many.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"100\" iterateDelta=\"0.0001\""))));
        QVERIFY(std::fabs(many.calc->cell(0, 0, 0).number() - 2) < 0.001);
        QVERIFY(std::fabs(many.calc->cell(0, 0, 1).number() - 1) < 0.001);
    }

    void aCircleBeginsFromTheResultsTheFileStoredAndStaysWhereItIsWhenItIsStill()
    {
        // the file's own results are the fixed point (A1 = B1+1 = 2 and B1 = A1*0.5 = 1): one round changes nothing, and the values are the file's exactly
        const QString rows = rowXml(1, cellXml("A1", "B1+1", "2") + cellXml("B1", "A1*0.5", "1"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"100\" iterateDelta=\"0.0001\""))));
        QCOMPARE(b.at("A1"), QStringLiteral("n:2"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:1"));
        // and with results that are not (a file from a program that stopped early), it goes on from them: after one round 2.5 and 1.25 from 3 and 2
        const QString early = rowXml(1, cellXml("A1", "B1+1", "3") + cellXml("B1", "A1*0.5", "2"));
        Book c;
        QVERIFY(c.open(buildXlsx({worksheet(early)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"1\" iterateDelta=\"0.0001\""))));
        QCOMPARE(c.at("A1"), QStringLiteral("n:3"));     // (B1 was 2, so A1 = 3)
        QCOMPARE(c.at("B1"), QStringLiteral("n:1.5"));   // (and B1 = A1 * 0.5)
    }

    void turningIterationOnAndOffWorksTheCircleOutAgain()
    {
        const QString rows = rowXml(1, cellXml("A1", "B1+1") + cellXml("B1", "A1*0.5"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QCOMPARE(b.at("A1"), QStringLiteral("n:0"));
        b.calc->setIteration(true, 3, 1e-9);
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.75"));
        QVERIFY(!b.calc->isCircular(0, 0, 0));
        b.calc->setIteration(false, 3, 1e-9);
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:0"));
        QVERIFY(b.calc->isCircular(0, 0, 0));
    }

    void aCircleOfAThousandFormulasIsGoneRoundWithinTheWorkItIsGiven()
    {
        // 1000 formulas each reading the next, the last reading the first: the rounds are the most there are, each of 1000 formulas
        QString rows;
        for (int i = 1; i <= 1000; ++i) rows += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("A%1+1").arg(i % 1000 + 1)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"5\" iterateDelta=\"0\""))));
        // (a round of values that keep growing is still a round: five rounds, the first formula the first of each; A1 reads A2, which was worked out in the round before)
        QVERIFY(b.calc->cell(0, 0, 0).isNumber());
        QVERIFY(b.calc->cell(0, 0, 0).number() > 0);
        QVERIFY(b.calc->cell(0, 0, 0).number() <= 5 * 1000);
    }

    // ---- one sheet at a time
    void oneSheetIsWorkedOutAndWhatWaitsOnTheOthersStaysWaitingUntilTheNextRecalculation()
    {
        for (const bool later : {false, true}) {
            const QByteArray one = worksheet(rowXml(1, constant("A1", "1") + cellXml("B1", "A1*2")));
            const QByteArray two = worksheet(rowXml(1, constant("A1", "10") + cellXml("B1", "A1*3") + cellXml("C1", "Sheet1!B1+1")));
            Book b;
            QVERIFY(b.open(buildXlsx({one, two})));
            QCOMPARE(b.at("B1"), QStringLiteral("n:2"));
            QCOMPARE(b.at("B1", 1), QStringLiteral("n:30"));
            const auto marking = later ? Calculation::Marking::Later : Calculation::Marking::Now;
            QVERIFY(b.calc->setValue(0, 0, 0, Value::number(5), marking));
            QVERIFY(b.calc->setValue(1, 0, 0, Value::number(20), marking));
            b.calc->invalidateSheet(0);
            QVERIFY(b.settle());
            QCOMPARE(b.at("B1"), QStringLiteral("n:10"));          // (the sheet asked for)
            QCOMPARE(b.at("B1", 1), QStringLiteral("n:30"));       // (the other sheet still has what it had)
            QCOMPARE(b.at("C1", 1), QStringLiteral("n:3"));        // (it reads the first sheet's B1, which changed, and waits too)
            QVERIFY(b.calc->isDirty());                             // (and something waits)
            QVERIFY(b.settle());                                    // (the next recalculation works out what waited)
            QCOMPARE(b.at("B1", 1), QStringLiteral("n:60"));
            QCOMPARE(b.at("C1", 1), QStringLiteral("n:11"));
            QVERIFY(!b.calc->isDirty());
        }
    }

    // ---- review fixes
    void aFormulaTypedWhileTheRestWaitsIsWorkedOutAtOnceAndWhatReadsItIsNot()
    {
        const QString rows = rowXml(1, constant("A1", "1") + cellXml("B1", "A1*2") + cellXml("C1", "B1+1"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(5)));        // A1 changed: B1 and C1 are stale until the next recalculation
        QVERIFY(b.calc->setFormula(0, 0, 3, QStringLiteral("A1*3")));   // D1 typed
        QVERIFY(b.calc->setFormula(0, 0, 4, QStringLiteral("D1+B1")));  // E1 typed: reads D1 (fresh) and B1 (stale)
        qint64 slices = 0;
        while (!b.calc->recalculateFresh().finished) QVERIFY(++slices < 1000);
        QCOMPARE(b.at("D1"), QStringLiteral("n:15"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:25"));      // (B1 is worked out, as E1 needs it)
        QCOMPARE(b.at("B1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:3"));       // (nothing that was typed reads C1: it waits)
        QVERIFY(b.calc->isDirty());
        QVERIFY(b.settle());
        QCOMPARE(b.at("C1"), QStringLiteral("n:11"));
    }

    void oneSheetWorkedOutLeavesNothingWaitingWhenNoOtherSheetReadsIt()
    {
        const QString rows = rowXml(1, constant("A1", "1") + cellXml("B1", "A1*2"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(5)));
        b.calc->invalidateSheet(0);
        QVERIFY(b.settle());
        QCOMPARE(b.at("B1"), QStringLiteral("n:10"));
        QVERIFY(!b.calc->isDirty());
    }

    void aCircleWhoseFirstRoundMovesNothingByMuchKeepsTheStoredResults()
    {
        // stored 2.00005 and 1.00002: a round gives 2.00002 and 1.00001, which differ by less than 0.0001, so the file's numbers stand
        const QString rows = rowXml(1, cellXml("A1", "B1+1", "2.00005") + cellXml("B1", "A1*0.5", "1.00002"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"100\" iterateDelta=\"0.0001\""))));
        QCOMPARE(b.at("A1"), QStringLiteral("n:2.00005"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:1.00002"));
        // one that moves a value by more goes on
        const QString far = rowXml(1, cellXml("A1", "B1+1", "5") + cellXml("B1", "A1*0.5", "1"));
        Book c;
        QVERIFY(c.open(buildXlsx({worksheet(far)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"100\" iterateDelta=\"0.0001\""))));
        QVERIFY(std::fabs(c.calc->cell(0, 0, 0).number() - 2) < 0.001);
    }

    // ---- circles of a workbook that goes round them (iteration on): the values of the file stand while nothing they read changes, and a change is gone round from them, never worked out once as an ordinary
    // formula (a read of a formula not worked out yet as empty made errors that a circle fed itself for ever)
    static QByteArray circleBook(const QString &rows, bool iterate = true)
    {
        return buildXlsx({worksheet(rows)}, QByteArray(), QString(), iterate ? QStringLiteral("iterate=\"1\" iterateCount=\"100\" iterateDelta=\"0.00001\"") : QString());
    }
    // A1 = IF(B1=0, 0, 1/C1) + 1 and B1 = A1*0 + 3 + D1*0 are a circle; C1 = F1*2 is outside it (F1 = 2.5). The first working out of the circle reads B1 as empty and never reads C1; going round from the file's
    // results reads C1, which may not be worked out yet. The file has A1 = 1.2, B1 = 3, C1 = 5, and E1 = A1+B1 (a reader).
    static QString circleRows(const QString &c1 = cellXml("C1", "F1*2", "5"))
    {
        return rowXml(1, cellXml("A1", "IF(B1=0,0,1/C1)+1", "1.2") + cellXml("B1", "A1*0+3+D1*0", "3") + c1 + constant("D1", "0") + cellXml("E1", "A1+B1", "4.2") + constant("F1", "2.5"));
    }
    // The edit of the review's trigger: D1, which B1 reads, and F1, which C1 reads, are changed together, so that the circle is worked out before C1 (the readers of what was changed last come first in the order).
    static void changeBoth(Book &b, double d1, double f1)
    {
        QVERIFY(b.calc->setValue(0, 0, 5, Value::number(f1), Calculation::Marking::Later));
        QVERIFY(b.calc->setValue(0, 0, 3, Value::number(d1), Calculation::Marking::Later));
        QVERIFY(b.settle());
    }
    void aCircleThatReadsAFormulaOutsideItOnlyOnceItIsGoneRoundFromTheFilesResultsStaysAsTheFileHasIt()
    {
        Book b;
        QVERIFY(b.open(circleBook(circleRows())));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.2"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:3"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:5"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:4.2"));   // (what reads the circle reads its values)
    }
    // What the circle read of a formula that was not worked out yet is put right when that formula is: F1 5 makes C1 10 after the circle was gone round with 5, and the circle goes round again to 1.1.
    void aFormulaTheCircleReadBeforeItWasWorkedOutAndIsAnotherValueIsReadAgain()
    {
        Book b;
        QVERIFY(b.open(circleBook(circleRows())));
        changeBoth(b, 1, 5);
        QCOMPARE(b.at("C1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.1"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:3"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:4.1"));
        // and back: the circle and its reader are as the file had them
        changeBoth(b, 0, 2.5);
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.2"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:4.2"));
    }
    // The formula outside is a member of another circle that is worked out after the first: G1 = C1*0+1 and C1 = IF(G1=0,0,F1*2).
    void aSecondCircleThatFeedsTheFirstAndIsWorkedOutAfterItIsReadAgain()
    {
        const QString rows = circleRows(cellXml("C1", "IF(G1=0,0,F1*2)", "5") + cellXml("G1", "C1*0+1", "1"));
        Book b;
        QVERIFY(b.open(circleBook(rows)));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.2"));
        changeBoth(b, 1, 5);
        QCOMPARE(b.at("C1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("G1"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.1"));
    }
    // A formula the file stored no result for, outside the circle: it is read as the empty cell it is until it is worked out, and the circle is gone round again, not left with the error that 1/empty made.
    void aFormulaOutsideACircleWithNoStoredResultIsReadAgainWhenItIsWorkedOut()
    {
        Book b;
        QVERIFY(b.open(circleBook(circleRows(QStringLiteral("<c r=\"C1\"><f>F1*2</f></c>")))));
        QCOMPARE(b.at("C1"), QStringLiteral("n:5"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.2"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:3"));
    }
    // The formula outside is a cell of an array: it is read by its place in the array, not as the array's first cell.
    void aCellOfAnArrayTheCircleReadBeforeItWasWorkedOutIsReadByItsPlace()
    {
        const QString rows = rowXml(1, cellXml("A1", "IF(B1=0,0,1/C2)+1", "1.125") + cellXml("B1", "A1*0+3+D1*0", "3") + constant("D1", "0") + constant("F1", "2.5")
                                           + QStringLiteral("<c r=\"C1\"><f t=\"array\" ref=\"C1:C2\">F1:F2*2+D1*0</f><v>5</v></c>"))
                             + rowXml(2, constant("C2", "8") + constant("F2", "4"));
        Book b;
        QVERIFY(b.open(circleBook(rows)));
        QCOMPARE(b.at("C2"), QStringLiteral("n:8"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.125"));
        // D1 is changed: the circle reads it, and so does the array (+D1*0), which is worked out again after the circle and is what it was, so that nothing corrects what the circle read of it
        const qint64 before = b.calc->evaluations();
        QVERIFY(b.calc->setValue(0, 0, 3, Value::number(1), Calculation::Marking::Later));
        QVERIFY(b.settle());
        // (what the circle read of the array is what the array is: nothing corrects it, and the circle is gone round once: 9 formulas worked out, where reading the array whole and correcting it makes 21)
        QVERIFY2(b.calc->evaluations() - before <= 12, qPrintable(QString::number(b.calc->evaluations() - before)));
        QCOMPARE(b.at("C2"), QStringLiteral("n:8"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.125"));   // (1/C2 + 1: the second cell of the array, 8)
        // and with the array changed too, the circle is read again
        QVERIFY(b.calc->setValue(0, 1, 5, Value::number(2), Calculation::Marking::Later));
        QVERIFY(b.calc->setValue(0, 0, 3, Value::number(0), Calculation::Marking::Later));
        QVERIFY(b.settle());
        QCOMPARE(b.at("C2"), QStringLiteral("n:4"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.25"));
    }
    // With iteration off a circle gives 0 and is marked as one, as it did (a round is gone only when the workbook asks for it).
    void aCircleOfAWorkbookWithIterationOffGivesZeroAsItDid()
    {
        Book b;
        QVERIFY(b.open(circleBook(circleRows(), false)));
        QVERIFY(!b.calc->iterating());
        QCOMPARE(b.at("A1"), QStringLiteral("n:0"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:0"));
        QVERIFY(b.calc->isCircular(0, 0, 0));
        changeBoth(b, 1, 5);
        QCOMPARE(b.at("C1"), QStringLiteral("n:10"));
        QVERIFY(b.calc->isCircular(0, 0, 0));
    }
    // Guards (they passed before the change of the round, and say what a circle is to give): an edit of a cell nothing reads leaves the circle alone, an edit of what it reads is gone round from what it had, put back it
    // is as it was, and one that breaks the circle makes it an ordinary formula.
    void guardEditsOfACircleAndOfWhatItReadsGoRoundFromTheValuesItHad()
    {
        Book b;
        QVERIFY(b.open(circleBook(circleRows())));
        QVERIFY(b.calc->setValue(0, 5, 5, Value::number(9)));
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.2"));
        QVERIFY(b.calc->setFormula(0, 0, 2, QStringLiteral("2+2")));
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.25"));
        QVERIFY(b.calc->setFormula(0, 0, 2, QStringLiteral("F1*2")));
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.2"));
        QVERIFY(b.calc->setValue(0, 0, 1, Value::number(4)));   // B1 is a number: no circle
        QVERIFY(b.settle());
        QVERIFY(!b.calc->isCircular(0, 0, 0));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.2"));
        QVERIFY(b.calc->setFormula(0, 0, 1, QStringLiteral("A1*0+3+D1*0")));   // and the circle is made again
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:1.2"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:3"));
    }
    void guardACircleThatConvergesIsGoneRoundFromTheFilesResultsWhenAnInputOfItChanges()
    {
        // A1 = B1+C1 and B1 = A1*0.5, with C1 = 1 (the file's results are the fixed point 2 and 1) and E1 reading A1
        const QString rows = rowXml(1, cellXml("A1", "B1+C1", "2") + cellXml("B1", "A1*0.5", "1") + constant("C1", "1") + cellXml("E1", "A1*10", "20"));
        Book b;
        QVERIFY(b.open(circleBook(rows)));
        QCOMPARE(b.at("A1"), QStringLiteral("n:2"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:20"));
        QVERIFY(b.calc->setValue(0, 0, 2, Value::number(3)));   // an input of the circle: it goes round to A1 = 6, B1 = 3
        QVERIFY(b.settle());
        QVERIFY(std::fabs(b.calc->cell(0, 0, 0).number() - 6) < 0.001);
        QVERIFY(std::fabs(b.calc->cell(0, 0, 1).number() - 3) < 0.001);
        QVERIFY(std::fabs(b.calc->cell(0, 0, 4).number() - 60) < 0.01);
        QVERIFY(b.calc->setValue(0, 0, 2, Value::number(1)));   // put back
        QVERIFY(b.settle());
        QVERIFY(std::fabs(b.calc->cell(0, 0, 0).number() - 2) < 0.001);
        QVERIFY(std::fabs(b.calc->cell(0, 0, 4).number() - 20) < 0.01);
    }

    void aChangeOfANameReadsAgainOnlyTheFormulasThatMentionIt()
    {
        QString rows = rowXml(1, constant("C1", "5") + cellXml("A1", "Rate*2") + cellXml("B1", "Twice+1") + cellXml("D1", "Other*3"));
        for (int i = 2; i <= 300; ++i) rows += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("C1+%1").arg(i)));
        const QByteArray file = buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(),
                                          QStringLiteral("<definedName name=\"Rate\">Sheet1!$C$1</definedName><definedName name=\"Twice\">Rate*2</definedName><definedName name=\"Other\">7</definedName>"));
        Book b;
        QVERIFY(b.open(file));
        const qint64 before = b.calc->nameRebuilds();
        b.calc->setNameText(-1, QStringLiteral("Unused"), QStringLiteral("Sheet1!$C$1"));   // a name nothing uses
        QCOMPARE(b.calc->nameRebuilds() - before, qint64(0));
        QVERIFY(!b.calc->isDirty());   // (and no recalculation was begun: what changes by itself is not worked out for it)
        b.calc->setNameText(-1, QStringLiteral("rate"), QStringLiteral("Sheet1!$C$1+1"));    // used by A1, and by Twice, which B1 uses
        QCOMPARE(b.calc->nameRebuilds() - before, qint64(2));
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:12"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:13"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:21"));
    }

    void circlesAreKeptAsTheyAreFoundAndNotLookedForInEveryFormula()
    {
        QString rows = rowXml(1, cellXml("A1", "B1+1") + cellXml("B1", "A1*2"));
        for (int i = 2; i <= 5000; ++i) rows += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("%1+1").arg(i)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        const qint64 before = b.calc->circleWork();
        for (int k = 0; k < 3; ++k) {
            QVERIFY(b.calc->setFormula(0, 7, 3, QStringLiteral("%1+1").arg(k)));
            QVERIFY(b.settle());
            QCOMPARE(b.calc->circularCells(nullptr, 0), qint64(2));
        }
        QVERIFY2(b.calc->circleWork() - before <= 12, qPrintable(QString::number(b.calc->circleWork() - before)));
    }

    void namesOfAnotherWorkbookAreNotNamesOfThisOne()
    {
        QCOMPARE(renameNameInFormula(QStringLiteral("[1]!Total+Total+[1]Sheet1!Total"), QStringLiteral("Total"), QStringLiteral("Sum"), nullptr),
                 QStringLiteral("[1]!Total+Sum+[1]Sheet1!Total"));
        QVERIFY(!formulaUsesName(QStringLiteral("[1]!Total*2"), QStringLiteral("Total")));
        QVERIFY(formulaUsesName(QStringLiteral("[1]!Other*Total"), QStringLiteral("Total")));
        QVERIFY(!formulaUsesName(QStringLiteral("Table1[Total]"), QStringLiteral("Total")));
    }

    void theProgramsOwnNamesAreRefused()
    {
        for (const char *bad : {"_xlnm.Print_Area", "_XLNM.x", "_xlnm._FilterDatabase"}) QVERIFY2(!nameProblem(QString::fromLatin1(bad)).isEmpty(), bad);
        QVERIFY(nameProblem(QStringLiteral("_xlnmx")).isEmpty());
    }

    void patchingTheListOfNamesCostsWhatTheListAndTheChangesCost()
    {
        auto work = [](int n) {
            QByteArray xml = "<workbook><sheets/><definedNames>";
            std::vector<NameChange> changes;
            for (int i = 0; i < n; ++i) {
                xml += "<definedName name=\"N" + QByteArray::number(i) + "\">S!$A$1</definedName>";
                NameChange c;
                c.hadOld = true;
                c.oldName = QStringLiteral("n%1").arg(i);
                c.name = QStringLiteral("N%1").arg(i);
                c.text = QStringLiteral("S!$A$2");
                changes.push_back(c);
            }
            xml += "</definedNames></workbook>";
            const qint64 before = patchDefinedNamesWork();
            QByteArray out;
            const bool ok = patchDefinedNames(xml, changes, QVector<int>{0}, &out);
            return ok && out.contains("S!$A$2") ? patchDefinedNamesWork() - before : qint64(-1);
        };
        const qint64 small = work(10000), big = work(20000);
        QVERIFY(small > 0 && big > 0);
        QVERIFY2(big <= small * 21 / 10, qPrintable(QStringLiteral("%1 for 10,000 and %2 for 20,000").arg(small).arg(big)));
    }

    // ---- the workbook part
    void definedNamesAreChangedInPlaceAndTheRestIsLeftAsItWas()
    {
        const QByteArray xml = "<workbook xmlns=\"x\"><sheets><sheet name=\"S\" sheetId=\"1\"/></sheets><definedNames>"
                               "<definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">S!$A$1:$B$2</definedName>"
                               "<definedName name=\"Rate\"  comment=\"old\">S!$C$1</definedName>"
                               "<definedName name=\"Gone\">S!$D$1</definedName>"
                               "<definedName name=\"Same\" hidden=\"1\">S!$E$1</definedName>"
                               "</definedNames><calcPr calcId=\"1\"/></workbook>";
        std::vector<NameChange> ch;
        NameChange rate;
        rate.hadOld = true;
        rate.oldName = QStringLiteral("rate");
        rate.name = QStringLiteral("Tax");
        rate.text = QStringLiteral("S!$C$2");
        rate.comment = QStringLiteral("a <b> & \"c\"");
        ch.push_back(rate);
        NameChange gone;
        gone.hadOld = true;
        gone.oldName = QStringLiteral("Gone");
        gone.remove = true;
        ch.push_back(gone);
        NameChange same;
        same.hadOld = true;
        same.oldName = QStringLiteral("Same");
        same.name = QStringLiteral("Same");
        same.text = QStringLiteral("S!$E$1");
        same.hidden = true;
        ch.push_back(same);
        NameChange made;
        made.name = QStringLiteral("Mine");
        made.text = QStringLiteral("S!$F$1");
        made.place = 0;
        ch.push_back(made);
        QByteArray out;
        QVERIFY(patchDefinedNames(xml, ch, QVector<int>{0}, &out));
        QCOMPARE(out, QByteArray("<workbook xmlns=\"x\"><sheets><sheet name=\"S\" sheetId=\"1\"/></sheets><definedNames>"
                                 "<definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">S!$A$1:$B$2</definedName>"
                                 "<definedName name=\"Tax\"  comment=\"a &lt;b&gt; &amp; &quot;c&quot;\">S!$C$2</definedName>"
                                 "<definedName name=\"Same\" hidden=\"1\">S!$E$1</definedName>"
                                 "<definedName name=\"Mine\" localSheetId=\"0\">S!$F$1</definedName></definedNames><calcPr calcId=\"1\"/></workbook>"));
        // nothing changed: the bytes are the same
        QByteArray same2;
        QVERIFY(patchDefinedNames(xml, std::vector<NameChange>{same}, QVector<int>{0}, &same2));
        QCOMPARE(same2, xml);
        // the last one taken away: the list goes
        const QByteArray one = "<workbook><sheets/><definedNames><definedName name=\"A\">S!$A$1</definedName></definedNames></workbook>";
        NameChange a;
        a.hadOld = true;
        a.oldName = QStringLiteral("A");
        a.remove = true;
        QByteArray none;
        QVERIFY(patchDefinedNames(one, std::vector<NameChange>{a}, QVector<int>{0}, &none));
        QCOMPARE(none, QByteArray("<workbook><sheets/></workbook>"));
        // and a name made where there is no list: after the sheets
        QByteArray made2;
        QVERIFY(patchDefinedNames("<workbook><sheets/><calcPr/></workbook>", std::vector<NameChange>{made}, QVector<int>{0}, &made2));
        QCOMPARE(made2, QByteArray("<workbook><sheets/><definedNames><definedName name=\"Mine\" localSheetId=\"0\">S!$F$1</definedName></definedNames><calcPr/></workbook>"));
        // a name of a sheet is found by the place the sheet has in the order of the tabs
        const QByteArray scoped = "<workbook><sheets/><definedNames><definedName name=\"X\" localSheetId=\"1\">S!$A$1</definedName></definedNames></workbook>";
        NameChange x;
        x.hadOld = true;
        x.oldName = QStringLiteral("X");
        x.oldPlace = 5;   // (the sheet that is second in the tabs)
        x.name = QStringLiteral("X");
        x.place = 5;
        x.text = QStringLiteral("S!$A$2");
        QByteArray moved;
        QVERIFY(patchDefinedNames(scoped, std::vector<NameChange>{x}, QVector<int>{3, 5}, &moved));
        QCOMPARE(moved, QByteArray("<workbook><sheets/><definedNames><definedName name=\"X\" localSheetId=\"1\">S!$A$2</definedName></definedNames></workbook>"));
    }

    void theCalculationSettingsAreWrittenInTheCalcPr()
    {
        QByteArray out;
        QVERIFY(patchCalcPr("<workbook><sheets/><calcPr calcId=\"191029\" fullCalcOnLoad=\"1\"/></workbook>", CalcMode::Manual, true, 50, 0.01, &out));
        QCOMPARE(out, QByteArray("<workbook><sheets/><calcPr calcId=\"191029\" fullCalcOnLoad=\"1\" calcMode=\"manual\" iterate=\"1\" iterateCount=\"50\" iterateDelta=\"0.01\"/></workbook>"));
        QByteArray back;
        QVERIFY(patchCalcPr(out, CalcMode::Auto, false, 50, 0.01, &back));
        QCOMPARE(back, QByteArray("<workbook><sheets/><calcPr calcId=\"191029\" fullCalcOnLoad=\"1\" iterateCount=\"50\" iterateDelta=\"0.01\"/></workbook>"));
        QByteArray same;
        QVERIFY(patchCalcPr("<workbook><sheets/><calcPr calcId=\"1\"/></workbook>", CalcMode::Auto, false, 100, 0.001, &same));
        QCOMPARE(same, QByteArray("<workbook><sheets/><calcPr calcId=\"1\"/></workbook>"));
        QByteArray made;
        QVERIFY(patchCalcPr("<workbook><sheets/><definedNames><definedName name=\"A\">1</definedName></definedNames></workbook>", CalcMode::AutoNoTable, false, 100, 0.001, &made));
        QCOMPARE(made, QByteArray("<workbook><sheets/><definedNames><definedName name=\"A\">1</definedName></definedNames><calcPr calcMode=\"autoNoTable\"/></workbook>"));
    }

    void theCalculationSettingsOfAFileAreRead()
    {
        Book b;
        QVERIFY(b.open(nameBook(QStringLiteral("calcMode=\"manual\" iterate=\"1\" iterateCount=\"17\" iterateDelta=\"0.25\""))));
        QCOMPARE(int(b.book.info().calcMode), int(CalcMode::Manual));
        QVERIFY(b.book.info().iterate);
        QCOMPARE(b.book.info().iterateCount, 17);
        QCOMPARE(b.book.info().iterateDelta, 0.25);
        Book d;
        QVERIFY(d.open(nameBook()));
        QCOMPARE(d.book.info().iterateCount, 100);
        QCOMPARE(d.book.info().iterateDelta, 0.001);
        QVERIFY(!d.book.info().calcUnfinished);
        Book u;
        QVERIFY(u.open(nameBook(QStringLiteral("calcCompleted=\"0\""))));
        QVERIFY(u.book.info().calcUnfinished);
        Book f;
        QVERIFY(f.open(nameBook(QStringLiteral("calcCompleted=\"1\""))));
        QVERIFY(!f.book.info().calcUnfinished);
    }
};

JP_TEST_CLASS(SheetNameTests)
#include "tst_sheet_names.moc"
