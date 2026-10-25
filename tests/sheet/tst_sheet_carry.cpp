// Tests of what a copy of a sheet takes from the file of the sheet it was copied from (the parts the program keeps no model of: protection, filter, links, page setup, breaks), read from the
// head and the tail of the part without reading the cells, moved by the edits made to rows and columns, and put in the XML the copy begins as; and of the outline of rows and the look of
// data bars, which a copy has from the model. Every sheet is built here.
#include "sheethelpers.h"

#include "sheet/structsave.h"
#include "sheet/xlsxbase.h"
#include "sheet/xlsxsave.h"

#include <QBuffer>
#include <QRegularExpression>

using namespace jp;

namespace {

QByteArray sheetWith(const QString &rows, const QString &after, const QString &head = QString()) { return worksheet(rows, after, head); }

SheetExtras extrasOf(const QByteArray &xml, bool *ok = nullptr)
{
    QByteArray copy = xml;
    QBuffer in(&copy);
    in.open(QIODevice::ReadOnly);
    SheetExtras x;
    QString why;
    const bool done = readSheetExtras(&in, &x, &why);
    if (ok) *ok = done;
    return x;
}

} // namespace

class SheetCarryTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The parts a copy takes are found at the head and the tail of the part, in the groups the format puts them in; the cells, the view, the sizes, the rules, and the merged ranges (which the
    // model has) are not taken, and nor is the name of a part of the package (a printer's settings).
    void theExtrasAreReadFromTheHeadAndTheTail()
    {
        const QByteArray xml = sheetWith(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"),
                                         QStringLiteral("<sheetProtection sheet=\"1\"/><autoFilter ref=\"A1:B5\"/><mergeCells count=\"1\"><mergeCell ref=\"A1:B1\"/></mergeCells>"
                                                        "<conditionalFormatting sqref=\"A1\"><cfRule type=\"expression\" priority=\"1\"><formula>TRUE</formula></cfRule></conditionalFormatting>"
                                                        "<hyperlinks><hyperlink ref=\"A1\" r:id=\"rId1\"/></hyperlinks><pageMargins left=\"1\" right=\"1\" top=\"1\" bottom=\"1\" header=\"0\" footer=\"0\"/>"
                                                        "<pageSetup orientation=\"landscape\" r:id=\"rId2\"/><drawing r:id=\"rId3\"/><tableParts count=\"0\"/>"),
                                         QStringLiteral("<sheetPr codeName=\"S\"><tabColor rgb=\"FFFF0000\"/><outlinePr summaryBelow=\"0\"/></sheetPr><dimension ref=\"A1\"/>"
                                                        "<sheetViews><sheetView workbookViewId=\"0\"/></sheetViews><sheetFormatPr defaultRowHeight=\"15\"/>"));
        bool ok = false;
        const SheetExtras x = extrasOf(xml, &ok);
        QVERIFY(ok && x.found);
        QCOMPARE(x.sheetPr, QByteArray("<outlinePr summaryBelow=\"0\"/>"));   // (the tab color is the model's, and the code name is the file's)
        QVERIFY2(x.afterData.contains("<sheetProtection sheet=\"1\"/>") && x.afterData.contains("<autoFilter ref=\"A1:B5\"/>"), x.afterData.constData());
        QVERIFY(!x.afterData.contains("mergeCells") && !x.afterData.contains("conditionalFormatting"));
        QVERIFY2(x.tail.contains("<hyperlink ref=\"A1\" r:id=\"rId1\"/>") && x.tail.contains("<pageMargins left=\"1\""), x.tail.constData());
        QVERIFY2(x.tail.contains("<pageSetup orientation=\"landscape\"/>"), x.tail.constData());   // (no r:id)
        QVERIFY(!x.tail.contains("drawing") && !x.tail.contains("tableParts") && !x.tail.contains("mergeCells"));
        QVERIFY(x.namespaces.contains("xmlns:r="));
    }

    // A sheet of a million cells is not read for its extras: the data is passed over in chunks, however long, an end tag cut by the edge of a chunk is found, and a string that holds the
    // text of an end tag (written with entities) is not one.
    void aLongSheetIsPassedOverWithoutBeingKept()
    {
        QString rows;
        for (int r = 1; r <= 60000; ++r)
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>&lt;/sheetData&gt; %1</t></is></c></row>").arg(r);
        const QByteArray xml = sheetWith(rows, QStringLiteral("<autoFilter ref=\"A1:A60000\"/><pageMargins left=\"2\" right=\"2\" top=\"2\" bottom=\"2\" header=\"0\" footer=\"0\"/>"));
        QVERIFY(xml.size() > 4000000);
        bool ok = false;
        const SheetExtras x = extrasOf(xml, &ok);
        QVERIFY(ok);
        QCOMPARE(x.afterData, QByteArray("<autoFilter ref=\"A1:A60000\"/>"));
        QVERIFY(x.tail.contains("left=\"2\""));
        // a part cut short is refused, not guessed
        QByteArray cut = xml.left(xml.size() / 2);
        QBuffer in(&cut);
        in.open(QIODevice::ReadOnly);
        SheetExtras none;
        QString why;
        QVERIFY(!readSheetExtras(&in, &none, &why));
        QVERIFY(!why.isEmpty());
    }

    // The extras follow the edits made to the rows and columns of the cells: a filter, a link, a protected range, a break, and an ignored error name cells.
    void theExtrasFollowTheEdits()
    {
        const QByteArray xml = sheetWith(QString(), QStringLiteral("<sheetProtection sheet=\"1\"/><protectedRanges><protectedRange sqref=\"B2:C3\" name=\"r\"/></protectedRanges><autoFilter ref=\"A1:C6\"/>"
                                                                    "<hyperlinks><hyperlink ref=\"A2\" location=\"Sheet1!A1\"/></hyperlinks><rowBreaks count=\"1\"><brk id=\"3\" max=\"16383\"/></rowBreaks>"
                                                                    "<ignoredErrors><ignoredError sqref=\"D4\" numberStoredAsText=\"1\"/></ignoredErrors>"));
        SheetExtras x = extrasOf(xml);
        MoveContext ctx;
        ctx.sheetNames = QStringList({QStringLiteral("Sheet1")});
        ctx.edits.push_back(StructEdit::insertRows(0, 0, 2));
        QString why;
        QVERIFY2(moveSheetExtras(&x, ctx, 0, &why, nullptr), qPrintable(why));
        QVERIFY2(x.afterData.contains("<autoFilter ref=\"A3:C8\"/>") && x.afterData.contains("sqref=\"B4:C5\""), x.afterData.constData());
        QVERIFY2(x.tail.contains("<hyperlink ref=\"A4\"") && x.tail.contains("<brk id=\"5\"") && x.tail.contains("sqref=\"D6\""), x.tail.constData());
        // the protection is not changed by an edit, and an edit to another sheet changes nothing
        QVERIFY(x.afterData.contains("<sheetProtection sheet=\"1\"/>"));
        SheetExtras y = extrasOf(xml);
        MoveContext other;
        other.sheetNames = QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet2")});
        other.edits.push_back(StructEdit::insertRows(1, 0, 5));
        QVERIFY(moveSheetExtras(&y, other, 0, &why, nullptr));
        QVERIFY(y.afterData.contains("<autoFilter ref=\"A1:C6\"/>"));
    }

    // The skeleton a copy begins as has the extras in the places the format puts them (after the data, then the rules, the links and the page setup in place of the default margins, the
    // extensions last), the namespaces they use, the outline properties after the tab color, and the protection a model says it has even when the file gave none.
    void theSkeletonHasTheExtrasInTheirPlaces()
    {
        Sheet sheet;
        sheet.tabColor = ColorSpec::fromRgb(0x00B050);
        sheet.isProtected = true;
        SheetExtras x;
        x.namespaces = " xmlns:x14ac=\"urn:test\"";
        x.sheetPr = "<outlinePr summaryBelow=\"0\"/>";
        x.afterData = "<autoFilter ref=\"A1:B2\"/>";
        x.tail = "<hyperlinks><hyperlink ref=\"A1\" location=\"B2\"/></hyperlinks><pageMargins left=\"1\" right=\"1\" top=\"1\" bottom=\"1\" header=\"0\" footer=\"0\"/><pageSetup orientation=\"landscape\"/>";
        const QByteArray out = skeletonWithExtras(newSheetSkeleton(sheet), x, true);
        Sheet read;
        QVERIFY2(readSheetXml(out, &read), out.constData());
        QVERIFY(read.isProtected);
        QVERIFY(read.tabColor == ColorSpec::fromRgb(0x00B050));
        auto at = [&](const char *s) { return out.indexOf(s); };
        QVERIFY(at("<tabColor") < at("<outlinePr") && at("<outlinePr") < at("</sheetPr>") && at("</sheetPr>") < at("<sheetViews"));
        QVERIFY(at("<sheetData/>") < at("<sheetProtection") && at("<sheetProtection") < at("<autoFilter") && at("<autoFilter") < at("<hyperlinks") && at("<hyperlinks") < at("<pageMargins left=\"1\""));
        QVERIFY(at("<pageMargins left=\"1\"") < at("<pageSetup") && at("<pageSetup") < at("</worksheet>"));
        QVERIFY(!out.contains("left=\"0.7\""));   // (the default margins are the file's own, when the file has none)
        QVERIFY(out.contains("xmlns:x14ac=\"urn:test\""));
        // no extras, a sheet that is not protected: the skeleton as it was
        Sheet plain;
        QCOMPARE(skeletonWithExtras(newSheetSkeleton(plain), SheetExtras(), false), newSheetSkeleton(plain));
    }

    // The look of a data bar (flat or gradient, bordered, where its axis is, its direction, the colors of negative values and the border and the axis) is written as the format's extension and
    // read back: a copy's bars look as the sheet's did. A plain bar needs no extension.
    void theLookOfADataBarIsWrittenAndRead()
    {
        CondRule bar;
        bar.type = CondRule::Type::DataBar;
        bar.ranges.append(CellRange{0, 1, 4, 1, true});
        bar.priority = 1;
        bar.values = {CondValue{CondValue::Type::Min, QString(), true}, CondValue{CondValue::Type::Max, QString(), true}};
        bar.colors = {ColorSpec::fromRgb(0x638EC6)};
        bar.gradient = false;
        bar.border = true;
        bar.negativeSameColor = false;
        bar.axis = CondRule::Axis::Middle;
        bar.rightToLeft = true;
        bar.minLength = 0;
        bar.maxLength = 100;
        bar.showValue = false;
        bar.borderColor = ColorSpec::fromRgb(0x112233);
        bar.negativeColor = ColorSpec::fromRgb(0xFF0000);
        bar.negativeBorderColor = ColorSpec::fromRgb(0xAA0000);
        bar.axisColor = ColorSpec::fromRgb(0x000001);
        CondRule plain = bar;
        plain.ranges = {CellRange{0, 2, 4, 2, true}};
        plain.priority = 2;
        plain.gradient = true;
        plain.border = false;
        plain.axis = CondRule::Axis::Automatic;
        plain.rightToLeft = false;
        plain.minLength = 10;
        plain.maxLength = 90;
        plain.showValue = true;
        plain.borderColor = plain.negativeColor = plain.negativeBorderColor = plain.axisColor = ColorSpec();
        Sheet sheet;
        sheet.conditionalFormats = {bar, plain};
        const QByteArray skeleton = newSheetSkeleton(sheet);
        QCOMPARE(skeleton.count("x14:dataBar "), 1);   // (the plain one has none)
        QCOMPARE(skeleton.count("<x14:cfRule"), 1);
        Sheet back;
        QVERIFY2(readSheetXml(skeleton, &back), skeleton.constData());
        QCOMPARE(back.conditionalFormats.size(), 2);
        const CondRule &b = back.conditionalFormats[0];
        QVERIFY(!b.gradient && b.border && b.axis == CondRule::Axis::Middle && b.rightToLeft && !b.showValue);
        QCOMPARE(b.minLength, 0);
        QCOMPARE(b.maxLength, 100);
        QVERIFY(b.borderColor == ColorSpec::fromRgb(0x112233) && b.negativeColor == ColorSpec::fromRgb(0xFF0000));
        QVERIFY(b.negativeBorderColor == ColorSpec::fromRgb(0xAA0000) && b.axisColor == ColorSpec::fromRgb(0x000001));
        const CondRule &p = back.conditionalFormats[1];
        QVERIFY(p.gradient && !p.border && p.axis == CondRule::Axis::Automatic && !p.rightToLeft && p.showValue);
    }

    // A row's outline level and whether its group is collapsed, and the deepest levels the sheet says, are read, and a sheet made from them (a copy) writes them again, in its rows and in
    // the format of its sheet.
    void theOutlineOfRowsIsReadAndWritten()
    {
        const QByteArray xml = sheetWith(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\" outlineLevel=\"1\"><c r=\"A2\"><v>2</v></c></row>"
                                                        "<row r=\"3\" outlineLevel=\"3\" hidden=\"1\"/><row r=\"4\" collapsed=\"1\"/><row r=\"5\" outlineLevel=\"9\"/>"),
                                         QString(), QStringLiteral("<sheetFormatPr defaultRowHeight=\"15\" outlineLevelRow=\"3\" outlineLevelCol=\"2\"/>"));
        Sheet sheet;
        QVERIFY(readSheetXml(xml, &sheet));
        QCOMPARE(sheet.format.outlineLevelRow, quint8(3));
        QCOMPARE(sheet.format.outlineLevelCol, quint8(2));
        QCOMPARE(sheet.rowSpecs.size(), 4);
        QCOMPARE(sheet.rowSpecs[0].row, quint32(1));
        QCOMPARE(sheet.rowSpecs[0].outline, quint8(1));
        QCOMPARE(sheet.rowSpecs[1].outline, quint8(3));
        QVERIFY(sheet.rowSpecs[1].hidden);
        QVERIFY(sheet.rowSpecs[2].collapsed && sheet.rowSpecs[2].outline == 0);
        QCOMPARE(sheet.rowSpecs[3].outline, quint8(7));   // (a level past the most is the most)
        // written again by the patcher, as a sheet that is made from the model
        SheetChanges c;
        c.skeleton = newSheetSkeleton(sheet);
        for (const RowSpec &spec : sheet.rowSpecs) {
            RowProps p;
            p.row = spec.row;
            p.hidden = spec.hidden;
            p.outlineKnown = true;
            p.outline = spec.outline;
            p.collapsed = spec.collapsed;
            c.rows.push_back(p);
        }
        QByteArray skeleton = c.skeleton;
        QBuffer in(&skeleton);
        in.open(QIODevice::ReadOnly);
        class Collect : public PartSink {
        public:
            QByteArray data;
            void write(const char *d, qsizetype n) override { data.append(d, n); }
        } out;
        QString why;
        QVERIFY2(patchSheet(&in, &out, c, true, &why), qPrintable(why));
        Sheet again;
        QVERIFY2(readSheetXml(out.data, &again), out.data.constData());
        QCOMPARE(again.format.outlineLevelRow, quint8(3));
        QCOMPARE(again.format.outlineLevelCol, quint8(2));
        QCOMPARE(again.rowSpecs.size(), sheet.rowSpecs.size());
        for (int i = 0; i < again.rowSpecs.size(); ++i) {
            QCOMPARE(again.rowSpecs[i].row, sheet.rowSpecs[i].row);
            QCOMPARE(again.rowSpecs[i].outline, sheet.rowSpecs[i].outline);
            QCOMPARE(again.rowSpecs[i].collapsed, sheet.rowSpecs[i].collapsed);
        }
    }

    // What the extensions of a sheet's file hold that the program neither shows nor writes into a copy is noted by the reader (sparklines, rules of conditional formatting of the extended
    // kind); a data bar's extension is read into the rule and is not among them, and nor are the drop-down lists that read other sheets (they are rules of validation like the rest).
    void whatTheExtensionsHoldThatIsNotKeptIsNoted()
    {
        const QString x14 = QStringLiteral("xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\"");
        auto sheetOf = [&](const QString &inside) { return sheetWith(QString(), QStringLiteral("<extLst><ext %1 uri=\"{x}\">%2</ext></extLst>").arg(x14, inside)); };
        Sheet s;
        QVERIFY(readSheetXml(sheetOf(QStringLiteral("<x14:sparklineGroups/>")), &s));
        QCOMPARE(s.notKept, QStringList({QStringLiteral("sparklines")}));
        QVERIFY(readSheetXml(sheetOf(QStringLiteral("<x14:dataValidations count=\"0\"/>")), &s));
        QVERIFY(s.notKept.isEmpty());
        QVERIFY(readSheetXml(sheetOf(QStringLiteral("<x14:conditionalFormattings><x14:conditionalFormatting><x14:cfRule type=\"iconSet\" id=\"{1}\"/><xm:sqref>A1</xm:sqref></x14:conditionalFormatting></x14:conditionalFormattings>")), &s));
        QCOMPARE(s.notKept, QStringList({QStringLiteral("extendedRules")}));
        QVERIFY(readSheetXml(sheetOf(QStringLiteral("<x14:conditionalFormattings><x14:conditionalFormatting><x14:cfRule type=\"dataBar\" id=\"{1}\"><x14:dataBar/></x14:cfRule><xm:sqref>A1</xm:sqref></x14:conditionalFormatting></x14:conditionalFormattings>")), &s));
        QVERIFY(s.notKept.isEmpty());
        QVERIFY(readSheetXml(sheetOf(QStringLiteral("<x14:somethingElse/>")), &s));
        QCOMPARE(s.notKept, QStringList({QStringLiteral("otherExtensions")}));
    }
};

JP_TEST_CLASS(SheetCarryTests)
#include "tst_sheet_carry.moc"
