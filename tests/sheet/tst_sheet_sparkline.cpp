// Tests of the sparklines of a sheet as the file has them: read from a workbook built here, changed, saved by saveXlsx, and read again. Every workbook is built in memory.
#include "sheethelpers.h"

#include "io/zip.h"
#include "sheet/sparkline.h"
#include "sheet/xlsxbase.h"
#include "sheet/xlsxsave.h"

using namespace jp;

namespace {

QMap<QString, QByteArray> entriesOf(const QByteArray &zip)
{
    QMap<QString, QByteArray> parts;
    jp::readZip(zip, parts);
    return parts;
}

const QString kRows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>3</v></c><c r=\"C1\"><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><v>4</v></c></row>");

const char *kSparks =
    "<extLst><ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">"
    "<x14:sparklineGroups xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\">"
    "<x14:sparklineGroup lineWeight=\"1.5\" type=\"column\" displayEmptyCellsAs=\"gap\" markers=\"1\" high=\"1\" negative=\"1\" displayXAxis=\"1\" minAxisType=\"custom\" manualMin=\"-2\" xmlns:xr2=\"urn:xr2\" xr2:uid=\"{AAAAAAAA-0000-0000-0000-000000000001}\">"
    "<x14:colorSeries rgb=\"FF376092\"/><x14:colorNegative theme=\"5\" tint=\"-0.5\"/><x14:colorAxis rgb=\"FF000000\"/><x14:colorMarkers rgb=\"FFD00000\"/><x14:colorFirst rgb=\"FFD00000\"/>"
    "<x14:colorLast rgb=\"FFD00000\"/><x14:colorHigh rgb=\"FF00AA00\"/><x14:colorLow rgb=\"FFD00000\"/><x14:sparklines>"
    "<x14:sparkline><xm:f>Sheet1!A1:C1</xm:f><xm:sqref>E1</xm:sqref></x14:sparkline><x14:sparkline><xm:f>Sheet1!A1:A2</xm:f><xm:sqref>E2</xm:sqref></x14:sparkline>"
    "</x14:sparklines></x14:sparklineGroup>"
    "<x14:sparklineGroup type=\"stacked\"><x14:colorSeries rgb=\"FF376092\"/><x14:sparklines><x14:sparkline><xm:f>Sheet1!A1:C1</xm:f><xm:sqref>F1</xm:sqref></x14:sparkline></x14:sparklines></x14:sparklineGroup>"
    "</x14:sparklineGroups></ext><ext uri=\"{FFFFFFFF-0000-0000-0000-000000000000}\"><other/></ext></extLst>";

struct Opened {
    QByteArray bytes;
    Package package;
    Workbook book;
    bool ok = false;
    QString why;
};

std::unique_ptr<Opened> open(const QByteArray &bytes)
{
    auto o = std::make_unique<Opened>();
    o->bytes = bytes;
    o->ok = o->package.readLazy(o->bytes, &o->why) && o->book.open(o->package, &o->why);
    return o;
}

std::unique_ptr<Opened> saved(Opened &from, const jp::office::sheet::SaveChanges &changes, QString *why)
{
    QByteArray out;
    if (!jp::office::sheet::saveXlsx(from.book, changes, &out, why)) return nullptr;
    auto o = open(out);
    return o->ok ? std::move(o) : nullptr;
}

} // namespace

using namespace jp::office::sheet;

class SheetSparklineTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void aSheetsSparklinesAreRead()
    {
        auto o = open(buildXlsx({worksheet(kRows, QString::fromLatin1(kSparks))}));
        QVERIFY2(o->ok, qPrintable(o->why));
        Sheet sheet;
        QVERIFY(o->book.readSheet(0, &sheet));
        QCOMPARE(int(sheet.sparklineGroups.size()), 2);
        const SparklineGroup &g = sheet.sparklineGroups[0];
        QCOMPARE(int(g.type), int(SparklineGroup::Type::Column));
        QCOMPARE(int(g.empty), int(SparklineGroup::Empty::Gap));
        QCOMPARE(g.lineWeight, 1.5);
        QVERIFY(g.markers && g.high && g.negative && g.showAxis && !g.low && !g.first && !g.last);
        QCOMPARE(int(g.minAxis), int(SparklineGroup::Axis::Custom));
        QCOMPARE(g.manualMin, -2.0);
        QCOMPARE(int(g.maxAxis), int(SparklineGroup::Axis::Individual));
        QCOMPARE(g.series.rgb, quint32(0x376092));
        QCOMPARE(int(g.negativeColor.kind), int(ColorSpec::Kind::Theme));
        QCOMPARE(g.negativeColor.index, 5);
        QCOMPARE(g.negativeColor.tint, -0.5);
        QCOMPARE(g.highColor.rgb, quint32(0x00AA00));
        QVERIFY(g.otherAttributes.contains("xr2:uid"));
        QCOMPARE(int(g.sparklines.size()), 2);
        QCOMPARE(g.sparklines[1].row, quint32(1));
        QCOMPARE(g.sparklines[1].col, quint32(4));
        QCOMPARE(g.sparklines[1].source, QStringLiteral("Sheet1!A1:A2"));
        QCOMPARE(int(sheet.sparklineGroups[1].type), int(SparklineGroup::Type::WinLoss));
        QCOMPARE(int(sheet.sparklineGroups[1].empty), int(SparklineGroup::Empty::Zero));
    }

    void aSheetWithNoSparklinesHasNone()
    {
        auto o = open(buildXlsx({worksheet(kRows, QString())}));
        Sheet sheet;
        QVERIFY(o->book.readSheet(0, &sheet));
        QVERIFY(sheet.sparklineGroups.isEmpty());
    }

    void groupsAreWrittenInTheFormatsOrderAndReadBack()
    {
        auto o = open(buildXlsx({worksheet(kRows, QString::fromLatin1(kSparks))}));
        Sheet sheet;
        QVERIFY(o->book.readSheet(0, &sheet));
        const QByteArray xml = sparklineExtensionXml(sheet.sparklineGroups);
        QVERIFY(xml.startsWith("<ext "));
        // colors in the order of the schema, then the sparklines
        const QList<const char *> order = {"<x14:colorSeries", "<x14:colorNegative", "<x14:colorAxis", "<x14:colorMarkers", "<x14:colorFirst", "<x14:colorLast", "<x14:colorHigh", "<x14:colorLow", "<x14:sparklines>"};
        qsizetype at = 0;
        for (const char *name : order) {
            const qsizetype next = xml.indexOf(name, at);
            QVERIFY2(next >= at, name);
            at = next;
        }
        auto again = open(buildXlsx({worksheet(kRows, "<extLst>" + QString::fromUtf8(xml) + "</extLst>")}));
        Sheet back;
        QVERIFY(again->book.readSheet(0, &back));
        QVERIFY(back.sparklineGroups == sheet.sparklineGroups);
    }

    void aSavedSheetKeepsTheOtherExtensionsAndTakesTheChangedSparklines()
    {
        auto o = open(buildXlsx({worksheet(kRows, QString::fromLatin1(kSparks))}));
        QVERIFY(o->ok);
        Sheet sheet;
        QVERIFY(o->book.readSheet(0, &sheet));
        SaveChanges changes;
        changes.sheets[0].replaceSparklines = true;
        changes.sheets[0].sparklines = sheet.sparklineGroups;
        changes.sheets[0].sparklines.removeLast();
        changes.sheets[0].sparklines[0].high = false;
        changes.sheets[0].sparklines[0].low = true;
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const QByteArray xml = entriesOf(again->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QCOMPARE(xml.count("<x14:sparklineGroup "), 1);
        QVERIFY(xml.contains("<other/>"));
        QCOMPARE(xml.count("<extLst>"), 1);
        Sheet back;
        QVERIFY(again->book.readSheet(0, &back));
        QCOMPARE(int(back.sparklineGroups.size()), 1);
        QVERIFY(back.sparklineGroups[0].low && !back.sparklineGroups[0].high);
        // taking them all away leaves the other extension, and then nothing leaves no list
        SaveChanges none;
        none.sheets[0].replaceSparklines = true;
        auto cleared = saved(*o, none, &why);
        QVERIFY2(cleared, qPrintable(why));
        const QByteArray bare = entriesOf(cleared->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(!bare.contains("sparkline"));
        QVERIFY(bare.contains("<other/>"));
        auto only = open(buildXlsx({worksheet(kRows, QStringLiteral("<extLst><ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\"><x14:sparklineGroups/></ext></extLst>"))}));
        auto empty = saved(*only, none, &why);
        QVERIFY2(empty, qPrintable(why));
        QVERIFY(!entriesOf(empty->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("extLst"));
    }

    void aSheetThatHadNoneIsGivenAnExtensionList()
    {
        auto o = open(buildXlsx({worksheet(kRows, QStringLiteral("<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>"))}));
        QVERIFY(o->ok);
        SparklineGroup g;
        Sparkline s;
        s.row = 0;
        s.col = 3;
        s.source = QStringLiteral("Sheet1!A1:C1");
        g.sparklines.append(s);
        g.type = SparklineGroup::Type::WinLoss;
        SaveChanges changes;
        changes.sheets[0].replaceSparklines = true;
        changes.sheets[0].sparklines = {g};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const QByteArray xml = entriesOf(again->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(xml.indexOf("<pageMargins") < xml.indexOf("<extLst>"));
        QVERIFY(xml.indexOf("</extLst>") < xml.indexOf("</worksheet>"));
        Sheet back;
        QVERIFY(again->book.readSheet(0, &back));
        QCOMPARE(int(back.sparklineGroups.size()), 1);
        QCOMPARE(int(back.sparklineGroups[0].type), int(SparklineGroup::Type::WinLoss));
        QCOMPARE(back.sparklineGroups[0].sparklines[0].col, quint32(3));
    }

    // The workbook has only so many sparklines and characters of sources in all its sheets (a sheet read again counts once), and what does not fit is left out and said.
    void aWorkbookHoldsOnlySoManySparklines()
    {
        auto readWith = [&](const std::shared_ptr<SparklineBudget> &budget, int sheetIndex) {
            QByteArray xml = worksheet(kRows, QString::fromLatin1(kSparks));
            QBuffer buf(&xml);
            buf.open(QIODevice::ReadOnly);
            ReadOptions o;
            o.sparklineBudget = budget;
            o.sparklineSheet = sheetIndex;
            Sheet sheet;
            [&] { QVERIFY(readSheetPart(&buf, o, &sheet)); }();
            return sheet;
        };
        // (the file has 3 sparklines) room for one more in the workbook: one is kept, and the sheet says it left the rest
        auto budget = std::make_shared<SparklineBudget>();
        budget->set(9, 0, kMaxWorkbookSparklines - 1);
        Sheet sheet = readWith(budget, 0);
        int kept = 0;
        for (const SparklineGroup &g : sheet.sparklineGroups) kept += int(g.sparklines.size());
        QCOMPARE(kept, 1);
        QVERIFY(sheet.sparklinesLossy);
        // room for the characters of none of them
        auto chars = std::make_shared<SparklineBudget>();
        chars->set(9, kMaxWorkbookSparklineChars - 5, 0);
        sheet = readWith(chars, 0);
        QVERIFY(sheet.sparklineGroups.isEmpty());
        QVERIFY(sheet.sparklinesLossy);
        // enough room for exactly what the sheet has, read twice: it counts once
        auto exact = std::make_shared<SparklineBudget>();
        exact->set(9, 0, kMaxWorkbookSparklines - 3);
        for (int again = 0; again < 3; ++again) {
            sheet = readWith(exact, 0);
            int n = 0;
            for (const SparklineGroup &g : sheet.sparklineGroups) n += int(g.sparklines.size());
            QCOMPARE(n, 3);
            QVERIFY(!sheet.sparklinesLossy);
        }
    }

    // What a sparkline part holds that the model has no place for is not cut down: the sheet's sparklines are shown and flagged, and a range of cells for one sparkline is not made its first cell.
    void whatTheModelCannotHoldMakesTheSheetsSparklinesLossy()
    {
        auto lossy = [&](const QString &groupAttrs, const QString &inside, const QString &sqref = QStringLiteral("E1")) {
            const QString ext = QStringLiteral("<extLst><ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">"
                                               "<x14:sparklineGroups xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\"><x14:sparklineGroup %1>%2<x14:sparklines><x14:sparkline><xm:f>Sheet1!A1:C1</xm:f>"
                                               "<xm:sqref>%3</xm:sqref></x14:sparkline></x14:sparklines></x14:sparklineGroup></x14:sparklineGroups></ext></extLst>").arg(groupAttrs, inside, sqref);
            auto o = open(buildXlsx({worksheet(kRows, ext)}));
            Sheet sheet;
            [&] { QVERIFY(o->book.readSheet(0, &sheet)); }();
            return sheet;
        };
        Sheet plain = lossy(QString(), QStringLiteral("<x14:colorSeries rgb=\"FF376092\"/>"));
        QVERIFY(!plain.sparklinesLossy);
        QCOMPARE(int(plain.sparklineGroups.size()), 1);
        // a range of cells for one sparkline
        Sheet many = lossy(QString(), QString(), QStringLiteral("E1:E3"));
        QVERIFY(many.sparklinesLossy);
        QVERIFY(many.sparklineGroups.isEmpty());   // (it is not cut to E1)
        // an element that is not known, and a tint on a color that is not a theme color
        QVERIFY(lossy(QString(), QStringLiteral("<x14:colorSeries rgb=\"FF376092\"/><x14:somethingNew a=\"1\"/>")).sparklinesLossy);
        QVERIFY(lossy(QString(), QStringLiteral("<x14:colorSeries rgb=\"FF376092\" tint=\"0.4\"/>")).sparklinesLossy);
        // a theme color's tint is kept, and an attribute that is not known is kept as it is
        Sheet kept = lossy(QStringLiteral("customThing=\"7\""), QStringLiteral("<x14:colorSeries theme=\"4\" tint=\"-0.25\"/>"));
        QVERIFY(!kept.sparklinesLossy);
        QCOMPARE(kept.sparklineGroups[0].series.tint, -0.25);
        QVERIFY(sparklineExtensionXml(kept.sparklineGroups).contains("customThing=\"7\""));
        QVERIFY(sparklineExtensionXml(kept.sparklineGroups).contains("tint=\"-0.25\""));
    }

    // A file with two extensions for sparklines has the groups in the first and the second taken out; a group that uses a prefix the file declared higher up is written with its own declaration.
    void twoExtensionsAreOneAndAPrefixIsDeclared()
    {
        const QString one = QString::fromLatin1(kSparks);
        const QString twoExts = QStringLiteral("<extLst><ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">"
                                               "<x14:sparklineGroups xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\"><x14:sparklineGroup><x14:sparklines><x14:sparkline><xm:f>Sheet1!A1:C1</xm:f>"
                                               "<xm:sqref>E1</xm:sqref></x14:sparkline></x14:sparklines></x14:sparklineGroup></x14:sparklineGroups></ext>"
                                               "<ext uri=\"{FFFFFFFF-0000-0000-0000-000000000000}\"><other/></ext>"
                                               "<ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">"
                                               "<x14:sparklineGroups xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\"><x14:sparklineGroup type=\"column\"><x14:sparklines><x14:sparkline><xm:f>Sheet1!A2:C2</xm:f>"
                                               "<xm:sqref>E2</xm:sqref></x14:sparkline></x14:sparklines></x14:sparklineGroup></x14:sparklineGroups></ext></extLst>");
        auto o = open(buildXlsx({worksheet(kRows, twoExts)}));
        Sheet sheet;
        QVERIFY(o->book.readSheet(0, &sheet));
        QCOMPARE(int(sheet.sparklineGroups.size()), 2);
        SaveChanges changes;
        changes.sheets[0].replaceSparklines = true;
        changes.sheets[0].sparklines = sheet.sparklineGroups;
        changes.sheets[0].sparklines.removeLast();
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const QByteArray xml = entriesOf(again->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QCOMPARE(xml.count("{05C60535-1F16-4fd2-B633-F4F36F0B64E0}"), 1);
        QVERIFY(xml.contains("<other/>"));
        QCOMPARE(xml.count("<x14:sparklineGroup"), 2);   // (the open tag of the group, and its close)
        Sheet back;
        QVERIFY(again->book.readSheet(0, &back));
        QCOMPARE(int(back.sparklineGroups.size()), 1);
        // a prefix declared on an element above the group
        const QString up = QString::fromLatin1(kSparks).replace(QStringLiteral(" xmlns:xr2=\"urn:xr2\""), QString()).replace(QStringLiteral("<x14:sparklineGroups xmlns:xm="), QStringLiteral("<x14:sparklineGroups xmlns:xr2=\"urn:xr2\" xmlns:xm="));
        auto o2 = open(buildXlsx({worksheet(kRows, up)}));
        Sheet s2;
        QVERIFY(o2->book.readSheet(0, &s2));
        QVERIFY(s2.sparklineGroups[0].otherAttributes.contains("xmlns:xr2=\"urn:xr2\""));
        QVERIFY(s2.sparklineGroups[0].otherAttributes.contains("xr2:uid"));
        const QByteArray skeleton = newSheetSkeleton(s2);
        QCOMPARE(skeleton.count("xmlns:xr2="), 1);
        // and the skeleton is a part that reads: the same groups come back
        auto o3 = open(buildXlsx({skeleton}));
        QVERIFY2(o3->ok, qPrintable(o3->why));
        Sheet s3;
        QVERIFY(o3->book.readSheet(0, &s3));
        QCOMPARE(int(s3.sparklineGroups.size()), 2);
    }

    // A sheet's name with a character outside the first plane of Unicode (an emoji) is in a source as it was, once.
    void aSourceWithAnEmojiIsWrittenWhole()
    {
        SparklineGroup g;
        Sparkline s;
        s.row = 0;
        s.col = 4;
        s.source = QString::fromUtf8("'Data \xF0\x9F\x98\x80'!A1:C1");
        g.sparklines.append(s);
        g.dateRange = QString::fromUtf8("'Data \xF0\x9F\x98\x80'!A5:C5 & <\"x\">");
        const QByteArray xml = sparklineExtensionXml({g});
        QVERIFY(xml.contains("Data \xF0\x9F\x98\x80'!A1:C1"));
        QVERIFY(!xml.contains("\xEF\xBF\xBD"));   // (no replacement character)
        auto o = open(buildXlsx({worksheet(kRows, "<extLst>" + QString::fromUtf8(xml) + "</extLst>")}));
        Sheet back;
        QVERIFY(o->book.readSheet(0, &back));
        QCOMPARE(back.sparklineGroups[0].sparklines[0].source, s.source);
        QCOMPARE(back.sparklineGroups[0].dateRange, g.dateRange);
    }
};

JP_TEST_CLASS(SheetSparklineTests)
#include "tst_sheet_sparkline.moc"
