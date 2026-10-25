// Tests that a sheet's page setup follows rows, columns, and cells inserted and deleted as the saved file does: the breaks, the print area, and the titles; and that the model and the
// saver agree for every kind of edit (the model's setup is what the file holds when it is reopened). Every workbook is built here.
#include "sheethelpers.h"

#include "sheet/pagefollow.h"
#include "sheet/xlsxsave.h"

namespace {

PageSetup setupWith(const QString &area, const QString &titles, const QString &sheet = QStringLiteral("Data"))
{
    PageSetup p;
    readPrintAreaText(area, sheet, &p);
    readPrintTitlesText(titles, sheet, &p);
    return p;
}

PageBreak breakAt(quint32 before)
{
    PageBreak b;
    b.before = before;
    b.max = 16383;
    return b;
}

QString areas(const PageSetup &p)
{
    QStringList l;
    for (const CellRange &r : p.printArea) l << rangeName(r);
    return l.join(QLatin1Char(' '));
}

QString breaks(const QVector<PageBreak> &list)
{
    QStringList l;
    for (const PageBreak &b : list) l << QString::number(b.before);
    return l.join(QLatin1Char(' '));
}

const QStringList kNames = {QStringLiteral("Data"), QStringLiteral("Other")};

} // namespace

class SheetPageFollowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Rows inserted above a print area move it, inside it make it larger, below it leave it; on another sheet they change nothing.
    void rowsInsertedMoveAndGrowThePrintArea()
    {
        PageSetup p = setupWith(QStringLiteral("Data!$A$5:$D$20"), QString());
        followStructEdit(&p, StructEdit::insertRows(0, 0, 2), 0, kNames);
        QCOMPARE(areas(p), QStringLiteral("A7:D22"));
        followStructEdit(&p, StructEdit::insertRows(0, 10, 3), 0, kNames);
        QCOMPARE(areas(p), QStringLiteral("A7:D25"));
        followStructEdit(&p, StructEdit::insertRows(0, 100, 3), 0, kNames);
        QCOMPARE(areas(p), QStringLiteral("A7:D25"));
        followStructEdit(&p, StructEdit::insertRows(1, 0, 5), 0, kNames);
        QCOMPARE(areas(p), QStringLiteral("A7:D25"));
        followStructEdit(&p, StructEdit::insertColumns(0, 1, 2), 0, kNames);
        QCOMPARE(areas(p), QStringLiteral("A7:F25"));
    }

    // Two breaks that a delete brings to one place are one break (set by hand if either was), not two saved at the same place.
    void breaksThatMeetAfterADeleteAreOne()
    {
        PageSetup p;
        PageBreak a = breakAt(10), b = breakAt(14);
        a.manual = false;
        b.manual = true;
        p.rowBreaks = {a, b, breakAt(30)};
        followStructEdit(&p, StructEdit::deleteRows(0, 10, 4), 0, kNames);
        QCOMPARE(breaks(p.rowBreaks), QStringLiteral("10 26"));
        QVERIFY(p.rowBreaks[0].manual);
        p.colBreaks = {breakAt(3), breakAt(5)};
        followStructEdit(&p, StructEdit::deleteColumns(0, 3, 2), 0, kNames);
        QCOMPARE(breaks(p.colBreaks), QStringLiteral("3"));
    }

    // Rows deleted from a print area shrink it; all of it deleted leaves none; one block of several can go.
    void rowsDeletedShrinkThePrintArea()
    {
        PageSetup p = setupWith(QStringLiteral("Data!$A$5:$D$20,Data!$F$30:$G$40"), QString());
        followStructEdit(&p, StructEdit::deleteRows(0, 9, 4), 0, kNames);
        QCOMPARE(areas(p), QStringLiteral("A5:D16 F26:G36"));
        followStructEdit(&p, StructEdit::deleteRows(0, 24, 20), 0, kNames);
        QCOMPARE(areas(p), QStringLiteral("A5:D16"));
        followStructEdit(&p, StructEdit::deleteRows(0, 0, 100), 0, kNames);
        QVERIFY(!p.hasPrintArea());
    }

    // The titles follow: a title row moves down, and one that is deleted is no title.
    void titlesFollowTheirRowsAndColumns()
    {
        PageSetup p = setupWith(QString(), QStringLiteral("Data!$A:$B,Data!$2:$3"));
        followStructEdit(&p, StructEdit::insertRows(0, 0, 1), 0, kNames);
        QCOMPARE(p.titles.row1, 2u);
        QCOMPARE(p.titles.row2, 3u);
        followStructEdit(&p, StructEdit::insertColumns(0, 0, 3), 0, kNames);
        QCOMPARE(p.titles.col1, 3u);
        QCOMPARE(p.titles.col2, 4u);
        followStructEdit(&p, StructEdit::deleteRows(0, 2, 2), 0, kNames);
        QVERIFY(!p.titles.hasRows);
        QVERIFY(p.titles.hasCols);
    }

    // A break moves with the row or column it is before; one before a row that is deleted is gone; one at the first of the rows deleted stays.
    void breaksMoveWithTheirRows()
    {
        PageSetup p;
        p.rowBreaks = {breakAt(10), breakAt(20), breakAt(30)};
        p.colBreaks = {breakAt(4)};
        followStructEdit(&p, StructEdit::insertRows(0, 15, 5), 0, kNames);
        QCOMPARE(breaks(p.rowBreaks), QStringLiteral("10 25 35"));
        followStructEdit(&p, StructEdit::deleteRows(0, 24, 3), 0, kNames);   // rows 24, 25, 26 go: the break before 25 goes with its row
        QCOMPARE(breaks(p.rowBreaks), QStringLiteral("10 32"));
        followStructEdit(&p, StructEdit::deleteRows(0, 10, 1), 0, kNames);   // the break at the first deleted row stays where it is
        QCOMPARE(breaks(p.rowBreaks), QStringLiteral("10 31"));
        followStructEdit(&p, StructEdit::insertColumns(0, 0, 2), 0, kNames);
        QCOMPARE(breaks(p.colBreaks), QStringLiteral("6"));
        followStructEdit(&p, StructEdit::insertRows(1, 0, 9), 0, kNames);   // another sheet
        QCOMPARE(breaks(p.rowBreaks), QStringLiteral("10 31"));
        // cells moved down or right move no break (as the file does not)
        followStructEdit(&p, StructEdit::insertCellsDown(0, CellRange{0, 0, 3, 3, true}), 0, kNames);
        QCOMPARE(breaks(p.rowBreaks), QStringLiteral("10 31"));
        // pushed off the end
        PageSetup q;
        q.rowBreaks = {breakAt(1048570)};
        followStructEdit(&q, StructEdit::insertRows(0, 0, 10), 0, kNames);
        QVERIFY(q.rowBreaks.isEmpty());
    }

    // The model and the saver agree: for each edit, the setup followed here is the setup the file holds when it is saved with the edit and read again.
    void theModelAndTheSaverAgree()
    {
        const QString head = QStringLiteral("<dimension ref=\"A1:D8\"/>");
        const QString after = QStringLiteral("<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/><pageSetup paperSize=\"9\"/>"
                                             "<rowBreaks count=\"3\" manualBreakCount=\"3\"><brk id=\"6\" max=\"16383\" man=\"1\"/><brk id=\"12\" max=\"16383\" man=\"1\"/><brk id=\"30\" max=\"16383\" man=\"1\"/></rowBreaks>"
                                             "<colBreaks count=\"2\" manualBreakCount=\"2\"><brk id=\"2\" max=\"1048575\" man=\"1\"/><brk id=\"5\" max=\"1048575\" man=\"1\"/></colBreaks>");
        const QByteArray sheet = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), after, head);
        const QByteArray other = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"));
        const QByteArray book = buildXlsx({sheet, other}, QByteArray(), QString(), QString(), QByteArray(), kNames,
                                          QStringLiteral("<definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">Data!$A$3:$D$20,Data!$F$25:$G$40</definedName>"
                                                         "<definedName name=\"_xlnm.Print_Titles\" localSheetId=\"0\">Data!$A:$B,Data!$2:$3</definedName>"
                                                         "<definedName name=\"_xlnm.Print_Area\" localSheetId=\"1\">Other!$A$1:$B$5</definedName>"));
        const std::vector<StructEdit> cases = {
            StructEdit::insertRows(0, 0, 2),  StructEdit::insertRows(0, 8, 3),  StructEdit::insertRows(0, 12, 1), StructEdit::insertRows(0, 100, 4), StructEdit::deleteRows(0, 0, 1),
            StructEdit::deleteRows(0, 2, 5),  StructEdit::deleteRows(0, 5, 8),  StructEdit::deleteRows(0, 11, 30), StructEdit::deleteRows(0, 1, 2),   StructEdit::insertColumns(0, 0, 1),
            StructEdit::insertColumns(0, 1, 2), StructEdit::insertColumns(0, 3, 2), StructEdit::deleteColumns(0, 0, 1), StructEdit::deleteColumns(0, 1, 2), StructEdit::deleteColumns(0, 3, 1),
            StructEdit::insertRows(1, 0, 3),  StructEdit::deleteRows(1, 0, 2),  StructEdit::insertCellsDown(0, CellRange{2, 0, 4, 3, true}), StructEdit::deleteCellsUp(0, CellRange{2, 0, 4, 3, true}),
            StructEdit::insertCellsRight(0, CellRange{2, 0, 4, 3, true}), StructEdit::deleteCellsLeft(0, CellRange{2, 0, 4, 3, true}),
        };
        Package package;
        QString why;
        QVERIFY2(package.readLazy(book, &why), qPrintable(why));
        Workbook wb;
        QVERIFY2(wb.open(package, &why), qPrintable(why));
        Sheet original;
        QVERIFY(wb.readSheet(0, &original));
        QVERIFY(original.page.hasPrintArea());
        int index = 0;
        for (const StructEdit &edit : cases) {
            SaveChanges changes;
            changes.edits = {edit};
            QByteArray out;
            QVERIFY2(saveXlsx(wb, changes, &out, &why), qPrintable(why));
            Package again;
            QVERIFY2(again.readLazy(out, &why), qPrintable(why));
            Workbook reopened;
            QVERIFY2(reopened.open(again, &why), qPrintable(why));
            Sheet saved;
            QVERIFY(reopened.readSheet(0, &saved));
            PageSetup model = original.page;
            followStructEdit(&model, edit, 0, kNames);
            QVERIFY2(saved.page == model, qPrintable(QStringLiteral("edit %1: file has areas [%2] rows [%3] cols [%4], model has [%5] [%6] [%7]")
                                                          .arg(index).arg(areas(saved.page), breaks(saved.page.rowBreaks), breaks(saved.page.colBreaks), areas(model), breaks(model.rowBreaks), breaks(model.colBreaks))));
            // (the setup of the other sheet is followed too)
            Sheet savedOther;
            QVERIFY(reopened.readSheet(1, &savedOther));
            Sheet originalOther;
            QVERIFY(wb.readSheet(1, &originalOther));
            PageSetup modelOther = originalOther.page;
            followStructEdit(&modelOther, edit, 1, kNames);
            QVERIFY2(savedOther.page == modelOther, qPrintable(QStringLiteral("edit %1 on the other sheet").arg(index)));
            ++index;
        }
    }
};

JP_TEST_CLASS(SheetPageFollowTests)
#include "tst_sheet_pagefollow.moc"
