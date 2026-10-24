// Tests of the conversion of a formula in the OpenDocument form to the text the model keeps: references with their sheets, the separators of arguments and arrays, the
// union and intersection of references, the names of functions that carry a prefix, and what cannot be converted.
#include "sheethelpers.h"

#include "calc/formula.h"
#include "sheet/odsformula.h"

class SheetOdsFormulaTests : public QObject {
    Q_OBJECT

    static QString convert(const QString &odf, bool *ok = nullptr)
    {
        bool good = false;
        const QString text = odfFormulaToText(odf, &good);
        if (ok) *ok = good;
        return text;
    }

private Q_SLOTS:
    // A cell, a range, an absolute reference, a whole column and a whole row, and the error a deleted reference leaves.
    void referencesAreWrittenWithoutBrackets()
    {
        QCOMPARE(convert(QStringLiteral("of:=SUM([.A1:.B2])")), QStringLiteral("SUM(A1:B2)"));
        QCOMPARE(convert(QStringLiteral("of:=[.A1]+[.B1]*2")), QStringLiteral("A1+B1*2"));
        QCOMPARE(convert(QStringLiteral("of:=[.$A$1]+[.B$2]")), QStringLiteral("$A$1+B$2"));
        QCOMPARE(convert(QStringLiteral("of:=SUM([.A:.A])")), QStringLiteral("SUM(A:A)"));
        QCOMPARE(convert(QStringLiteral("of:=SUM([.1:.1])")), QStringLiteral("SUM(1:1)"));
        QCOMPARE(convert(QStringLiteral("of:=[.A1]+[.#REF!]")), QStringLiteral("A1+#REF!"));
        QCOMPARE(convert(QStringLiteral("of:=#N/A")), QStringLiteral("#N/A"));
    }

    // The sheet is before a dot, perhaps in quotes (a name that needs them keeps them); the second end of a range may leave the sheet out; two sheets are a range through them.
    void referencesToOtherSheetsKeepTheirSheet()
    {
        QCOMPARE(convert(QStringLiteral("of:=[$Sheet2.A1]")), QStringLiteral("Sheet2!A1"));
        QCOMPARE(convert(QStringLiteral("of:=[$'My Sheet'.A1:.B2]")), QStringLiteral("'My Sheet'!A1:B2"));
        QCOMPARE(convert(QStringLiteral("of:=[$'It''s'.A1]")), QStringLiteral("'It''s'!A1"));
        QCOMPARE(convert(QStringLiteral("of:=[$Sheet1.$A$1:$Sheet1.B2]")), QStringLiteral("Sheet1!$A$1:B2"));
        QCOMPARE(convert(QStringLiteral("of:=SUM([$Sheet1.A1:$Sheet3.A1])")), QStringLiteral("SUM(Sheet1:Sheet3!A1)"));
        QCOMPARE(convert(QStringLiteral("of:=[$A1.B2]")), QStringLiteral("'A1'!B2"));   // (a sheet named like a cell needs its quotes)
    }

    // Arguments are separated by semicolons; texts are copied as they are, their quotes doubled and their semicolons kept; spaces outside texts go.
    void separatorsAndTextsAreConverted()
    {
        QCOMPARE(convert(QStringLiteral("of:=IF([.A1]>0;\"yes\";\"no\")")), QStringLiteral("IF(A1>0,\"yes\",\"no\")"));
        QCOMPARE(convert(QStringLiteral("of:=\"a;b\"\"c\"&[.A1]")), QStringLiteral("\"a;b\"\"c\"&A1"));
        QCOMPARE(convert(QStringLiteral("of:= SUM( [.A1] ; [.A2] )")), QStringLiteral("SUM(A1,A2)"));
        QCOMPARE(convert(QStringLiteral("of:=IF([.A1]<>[.B1];1;0)")), QStringLiteral("IF(A1<>B1,1,0)"));
        QCOMPARE(convert(QStringLiteral("of:=-[.A1]%")), QStringLiteral("-A1%"));
        QCOMPARE(convert(QStringLiteral("of:=1.5E+3+.5")), QStringLiteral("1.5E+3+.5"));
    }

    // The columns of an array constant are separated by semicolons and its rows by vertical bars.
    void arrayConstantsUseTheirOwnSeparators()
    {
        QCOMPARE(convert(QStringLiteral("of:=SUM({1;2|3;4})")), QStringLiteral("SUM({1,2;3,4})"));
        QCOMPARE(convert(QStringLiteral("of:=MATCH(\"b\";{\"a\";\"b;c\"};0)")), QStringLiteral("MATCH(\"b\",{\"a\",\"b;c\"},0)"));
    }

    // The union of references is a tilde and their intersection an exclamation mark.
    void unionAndIntersection()
    {
        QCOMPARE(convert(QStringLiteral("of:=SUM([.A1:.A3]~[.C1:.C3])")), QStringLiteral("SUM((A1:A3,C1:C3))"));
        QCOMPARE(convert(QStringLiteral("of:=SUM([.A1]~[.B1]~[.C1];5)")), QStringLiteral("SUM((A1,B1,C1),5)"));
        QCOMPARE(convert(QStringLiteral("of:=SUM([.A1:.C3]![.B2:.D4])")), QStringLiteral("SUM(A1:C3 B2:D4)"));
        QCOMPARE(convert(QStringLiteral("of:=SUM(([.A1]~[.B1]))")), QStringLiteral("SUM(((A1,B1)))"));
    }

    // Functions: the prefixes the format puts before names it has outside the standard come off, and the ones the .xlsx writes with its own prefix get it.
    void namesOfFunctionsAreTheModels()
    {
        QCOMPARE(convert(QStringLiteral("of:=COM.MICROSOFT.STDEV.S([.A1:.A9])")), QStringLiteral("_xlfn.STDEV.S(A1:A9)"));
        QCOMPARE(convert(QStringLiteral("of:=LEGACY.NORMSDIST([.A1])")), QStringLiteral("NORMSDIST(A1)"));
        QCOMPARE(convert(QStringLiteral("of:=ORG.OPENOFFICE.EASTERSUNDAY([.A1])")), QStringLiteral("EASTERSUNDAY(A1)"));
        QCOMPARE(convert(QStringLiteral("of:=TRUE()")), QStringLiteral("TRUE()"));
        QCOMPARE(convert(QStringLiteral("of:=sum([.A1])")), QStringLiteral("SUM(A1)"));
        QCOMPARE(convert(QStringLiteral("of:=SUM(Prices)*Tax")), QStringLiteral("SUM(Prices)*Tax"));
    }

    // The namespace of the formula: of and oooc are the OpenDocument forms, msoxl is the other format's own text (as it is), none is the form without a namespace, and others are
    // not converted.
    void theNamespaceIsRead()
    {
        bool ok = false;
        QCOMPARE(convert(QStringLiteral("oooc:=SUM([.A1])"), &ok), QStringLiteral("SUM(A1)"));
        QVERIFY(ok);
        QCOMPARE(convert(QStringLiteral("=SUM([.A1])"), &ok), QStringLiteral("SUM(A1)"));
        QVERIFY(ok);
        QCOMPARE(convert(QStringLiteral("SUM([.A1])"), &ok), QStringLiteral("SUM(A1)"));
        QVERIFY(ok);
        QCOMPARE(convert(QStringLiteral("msoxl:=SUM(A1:B2)"), &ok), QStringLiteral("SUM(A1:B2)"));
        QVERIFY(ok);
        convert(QStringLiteral("foo:=1"), &ok);
        QVERIFY(!ok);
    }

    // What cannot be converted gives no text: another file's cells, a bracket never closed, a character no formula has, an empty formula, a call never closed.
    void whatCannotBeConvertedIsRefused()
    {
        bool ok = true;
        convert(QStringLiteral("of:=[ 'file:///C:/x.ods'#$Sheet1.A1]"), &ok);
        QVERIFY(!ok);
        ok = true;
        convert(QStringLiteral("of:=[.A1"), &ok);
        QVERIFY(!ok);
        ok = true;
        convert(QStringLiteral("of:=SUM([.A1]"), &ok);
        QVERIFY(!ok);
        ok = true;
        convert(QStringLiteral("of:=[.A1]@"), &ok);
        QVERIFY(!ok);
        ok = true;
        convert(QStringLiteral("of:="), &ok);
        QVERIFY(!ok);
        ok = true;
        convert(QStringLiteral("of:=\"never closed"), &ok);
        QVERIFY(!ok);
        ok = true;
        convert(QStringLiteral("of:=[.ZZZZ1]"), &ok);
        QVERIFY(!ok);
    }

    // What is converted is a formula the engine reads.
    void convertedFormulasParse()
    {
        const QStringList formulas = {QStringLiteral("of:=SUM([.A1:.B2];[$Sheet2.C3])"), QStringLiteral("of:=IF(AND([.A1]>1;[.B1]<2);\"x\";[.C1])"),
                                      QStringLiteral("of:=SUMPRODUCT(([.A1:.A9]=1)*([.B1:.B9]))"), QStringLiteral("of:=VLOOKUP([.A1];[$Data.A1:.C9];2;0)")};
        for (const QString &f : formulas) {
            bool ok = false;
            const QString text = convert(f, &ok);
            QVERIFY2(ok, qPrintable(f));
            QVERIFY2(jp::calc::parseFormula(text, jp::calc::ParseOptions()).ok(), qPrintable(text));
        }
    }

    // The address of a named range: the sheet before a dot, the second end of the range perhaps without a sheet.
    void namedRangeAddressesAreConverted()
    {
        bool ok = false;
        QCOMPARE(odfRangeToText(QStringLiteral("$Sheet1.$A$1:.$B$2"), &ok), QStringLiteral("Sheet1!$A$1:$B$2"));
        QVERIFY(ok);
        QCOMPARE(odfRangeToText(QStringLiteral("$'My Sheet'.A1"), &ok), QStringLiteral("'My Sheet'!A1"));
        QVERIFY(ok);
        QCOMPARE(odfRangeToText(QStringLiteral("$Sheet1.A1:$Sheet1.B2"), &ok), QStringLiteral("Sheet1!A1:B2"));
        QVERIFY(ok);
        odfRangeToText(QStringLiteral("not a range"), &ok);
        QVERIFY(!ok);
        odfRangeToText(QString(), &ok);
        QVERIFY(!ok);
    }

    void cellAddressesAreRead()
    {
        quint32 row = 9, col = 9;
        QVERIFY(odfCellOf(QStringLiteral("$B$3"), &row, &col));
        QCOMPARE(row, 2u);
        QCOMPARE(col, 1u);
        QVERIFY(odfCellOf(QStringLiteral("XFD1048576"), &row, &col));
        QCOMPARE(col, 16383u);
        QVERIFY(!odfCellOf(QStringLiteral("XFE1"), &row, &col));
        QVERIFY(!odfCellOf(QStringLiteral("A0"), &row, &col));
        QVERIFY(!odfCellOf(QStringLiteral("A"), &row, &col));
    }

    // A formula of any length and nesting is converted in a time proportional to its length (deep nesting is refused by the engine's parser, not by a crash here).
    void hostileFormulasAreBounded()
    {
        QString deep = QStringLiteral("of:=");
        for (int i = 0; i < 100000; ++i) deep += QLatin1Char('(');
        bool ok = true;
        convert(deep, &ok);
        QVERIFY(!ok);
        QString many = QStringLiteral("of:=1");
        for (int i = 0; i < 100000; ++i) many += QStringLiteral("+[.A1]");
        convert(many, &ok);   // (parsed or refused; the point is that it ends)
        QString unions = QStringLiteral("of:=SUM([.A1]");
        for (int i = 0; i < 20000; ++i) unions += QStringLiteral("~[.B1]");
        unions += QLatin1Char(')');
        convert(unions, &ok);
        QString junk;
        for (int i = 0; i < 20000; ++i) junk += QStringLiteral("[$'x");
        convert(QStringLiteral("of:=") + junk, &ok);
        QVERIFY(!ok);
    }
};

JP_TEST_CLASS(SheetOdsFormulaTests)
#include "tst_sheet_odsformula.moc"
