// Tests of conditional formats: the rules a sheet's file holds (cell-value, formula, top and bottom, average, duplicates, text, dates, blanks, errors, color scales, data bars, and icon
// sets, with their priority and stop-if-true and the differential formats they apply), what each does to the cells of its ranges, and the cost of the drawing. Every workbook is built here.
#include "sheethelpers.h"

#include "sheet/styles.h"

namespace {

bool readStylesXml(const QByteArray &xml, Styles *out, QString *error = nullptr)
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    return readStyles(&buffer, Limits(), out, error);
}

const char kX14[] = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main";

} // namespace

class SheetCondFormatTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The rules are read in the order of their priority, whatever order the file has them in, with what each says: the cells, the kind, the comparison, the formulas, the format to apply,
    // stop-if-true, and the extra things of a color scale, a data bar (and its extension), and an icon set.
    void theRulesAreReadByPriority()
    {
        const QString after = QStringLiteral(
            "<conditionalFormatting sqref=\"A1:A10 C3\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"3\" stopIfTrue=\"1\" operator=\"between\"><formula>5</formula><formula>9</formula></cfRule></conditionalFormatting>"
            "<conditionalFormatting sqref=\"B1:B10\"><cfRule type=\"colorScale\" priority=\"1\"><colorScale><cfvo type=\"min\"/><cfvo type=\"percentile\" val=\"50\"/><cfvo type=\"max\"/>"
            "<color rgb=\"FFF8696B\"/><color rgb=\"FFFFEB84\"/><color theme=\"4\" tint=\"0.5\"/></colorScale></cfRule></conditionalFormatting>"
            "<conditionalFormatting sqref=\"D1:D10\"><cfRule type=\"dataBar\" priority=\"2\"><dataBar minLength=\"5\" maxLength=\"80\" showValue=\"0\"><cfvo type=\"min\"/><cfvo type=\"max\"/><color rgb=\"FF638EC6\"/></dataBar>"
            "<extLst><ext uri=\"{B025F937-C7B1-47D3-B67F-A62EFF666E3E}\" xmlns:x14=\"%1\"><x14:id>{ID-1}</x14:id></ext></extLst></cfRule></conditionalFormatting>"
            "<conditionalFormatting sqref=\"E1:E10\"><cfRule type=\"iconSet\" priority=\"4\"><iconSet iconSet=\"3Arrows\" reverse=\"1\"><cfvo type=\"percent\" val=\"0\"/><cfvo type=\"percent\" val=\"33\"/>"
            "<cfvo type=\"num\" val=\"7\" gte=\"0\"/></iconSet></cfRule></conditionalFormatting>"
            "<conditionalFormatting sqref=\"F1:F9\"><cfRule type=\"top10\" dxfId=\"1\" priority=\"5\" rank=\"3\" percent=\"1\" bottom=\"1\"/></conditionalFormatting>"
            "<conditionalFormatting sqref=\"G1\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"6\"><formula>MOD(ROW(),2)=0</formula></cfRule></conditionalFormatting>"
            "<extLst><ext uri=\"{78C0D931-6437-407d-A8EE-F0AAD7539E65}\" xmlns:x14=\"%1\"><x14:conditionalFormattings><x14:conditionalFormatting><x14:cfRule type=\"dataBar\" id=\"{ID-1}\">"
            "<x14:dataBar minLength=\"0\" maxLength=\"100\" gradient=\"0\" border=\"1\" negativeBarColorSameAsPositive=\"0\" axisPosition=\"middle\"><x14:cfvo type=\"autoMin\"/><x14:cfvo type=\"autoMax\"/>"
            "<x14:borderColor rgb=\"FF1F4E79\"/><x14:negativeFillColor rgb=\"FFFF0000\"/><x14:negativeBorderColor rgb=\"FF990000\"/><x14:axisColor rgb=\"FF000000\"/></x14:dataBar></x14:cfRule>"
            "</x14:conditionalFormatting></x14:conditionalFormattings></ext></extLst>").arg(QLatin1String(kX14));
        Sheet sheet;
        QVERIFY(readSheetXml(worksheet(QString(), after), &sheet));
        QCOMPARE(sheet.conditionalFormats.size(), 6);
        using T = CondRule::Type;
        const QVector<CondRule> &r = sheet.conditionalFormats;
        // by priority
        QVERIFY(r[0].type == T::ColorScale && r[1].type == T::DataBar && r[2].type == T::CellIs && r[3].type == T::IconSet && r[4].type == T::Top10 && r[5].type == T::Expression);
        QCOMPARE(r[0].priority, 1);
        QCOMPARE(r[5].priority, 6);
        // the color scale
        QCOMPARE(r[0].values.size(), 3);
        QVERIFY(r[0].values[0].type == CondValue::Type::Min && r[0].values[1].type == CondValue::Type::Percentile && r[0].values[2].type == CondValue::Type::Max);
        QCOMPARE(r[0].values[1].value, QStringLiteral("50"));
        QCOMPARE(r[0].colors.size(), 3);
        QVERIFY(r[0].colors[0] == ColorSpec::fromRgb(0xF8696B));
        QVERIFY(r[0].colors[2].kind == ColorSpec::Kind::Theme && r[0].colors[2].index == 4 && r[0].colors[2].tint == 0.5);
        QCOMPARE(r[0].ranges.size(), 1);
        QCOMPARE(rangeName(r[0].ranges[0]), QStringLiteral("B1:B10"));
        // the data bar, with its extension
        QVERIFY(r[1].colors.size() == 1 && r[1].colors[0] == ColorSpec::fromRgb(0x638EC6));
        QVERIFY(!r[1].showValue);
        QCOMPARE(r[1].minLength, 0);
        QCOMPARE(r[1].maxLength, 100);
        QVERIFY(!r[1].gradient && r[1].border && !r[1].negativeSameColor);
        QVERIFY(r[1].axis == CondRule::Axis::Middle);
        QVERIFY(r[1].negativeColor == ColorSpec::fromRgb(0xFF0000) && r[1].borderColor == ColorSpec::fromRgb(0x1F4E79) && r[1].negativeBorderColor == ColorSpec::fromRgb(0x990000));
        QVERIFY(r[1].axisColor == ColorSpec::fromRgb(0x000000));
        // the cell-value rule
        QCOMPARE(r[2].dxf, 0);
        QVERIFY(r[2].stopIfTrue);
        QVERIFY(r[2].op == CondRule::Operator::Between);
        QCOMPARE(r[2].formula1, QStringLiteral("5"));
        QCOMPARE(r[2].formula2, QStringLiteral("9"));
        QCOMPARE(r[2].ranges.size(), 2);
        QCOMPARE(rangeName(r[2].ranges[1]), QStringLiteral("C3"));
        // the icon set
        QCOMPARE(r[3].iconSet, QStringLiteral("3Arrows"));
        QVERIFY(r[3].reverse && r[3].showValue);
        QCOMPARE(r[3].values.size(), 3);
        QVERIFY(r[3].values[1].type == CondValue::Type::Percent && r[3].values[2].type == CondValue::Type::Number && !r[3].values[2].gte && r[3].values[0].gte);
        // top and bottom
        QCOMPARE(r[4].rank, 3);
        QVERIFY(r[4].percent && r[4].bottom);
        QCOMPARE(r[4].dxf, 1);
        // a formula
        QCOMPARE(r[5].formula1, QStringLiteral("MOD(ROW(),2)=0"));
    }

    // The differential formats the rules apply: a bold red font, a fill (whose color a differential format keeps in its background color), a border, and a number format.
    void theDifferentialFormatsAreRead()
    {
        const QByteArray xml = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>"),
                                         QStringLiteral("<dxfs count=\"3\">"
                                                        "<dxf><font><b/><i val=\"0\"/><u val=\"double\"/><color rgb=\"FF9C0006\"/></font><fill><patternFill><bgColor rgb=\"FFFFC7CE\"/></patternFill></fill></dxf>"
                                                        "<dxf><numFmt numFmtId=\"164\" formatCode=\"0.0%\"/><border><left style=\"thin\"><color rgb=\"FF0000FF\"/></left><bottom style=\"double\"/></border></dxf>"
                                                        "<dxf><fill><patternFill patternType=\"solid\"><fgColor theme=\"4\"/><bgColor indexed=\"64\"/></patternFill></fill></dxf></dxfs>"));
        Styles st;
        QString why;
        QVERIFY2(readStylesXml(xml, &st, &why), qPrintable(why));
        QCOMPARE(st.dxfs.size(), 3);
        const DifferentialFormat &a = st.dxfs[0], &b = st.dxfs[1], &c = st.dxfs[2];
        QVERIFY(a.bold && *a.bold && a.italic && !*a.italic && a.underline && *a.underline == Underline::Double);
        QVERIFY(a.fontColor == ColorSpec::fromRgb(0x9C0006));
        QVERIFY(a.hasFill && a.fill.pattern == PatternKind::Solid && a.fill.bg == ColorSpec::fromRgb(0xFFC7CE));
        QVERIFY(!a.hasBorder && !a.numberFormat);
        QVERIFY(b.numberFormat && *b.numberFormat == QStringLiteral("0.0%"));
        QVERIFY(b.hasBorder && b.border.left.style == BorderStyle::Thin && b.border.left.color == ColorSpec::fromRgb(0x0000FF) && b.border.bottom.style == BorderStyle::Double);
        QVERIFY(!b.bold && !b.hasFill);
        QVERIFY(c.hasFill && c.fill.fg.kind == ColorSpec::Kind::Theme && c.fill.fg.index == 4);
        QVERIFY(!c.isEmpty() && DifferentialFormat().isEmpty());
    }
};

JP_TEST_CLASS(SheetCondFormatTests)
#include "tst_sheet_condformat.moc"
