// Tests of the readers of JSON and XML data for Get Data: the lists a file offers, the columns and rows read, and the limits on files made to do harm. Every file is made here.
#include "sheethelpers.h"
#include "sheet/datasource.h"
#include "sheet/datasourcebook.h"
#include "sheet/datasteps.h"
#include "sheet/tables.h"

#include <QTemporaryDir>

#include "../testrunner.h"

#include <QtTest>

using namespace jp::office::sheet;

class SheetDataSourceTests : public QObject {
    Q_OBJECT

    static QString cell(const DataRows &t, int row, const QString &column)
    {
        const int c = t.header.indexOf(column);
        return c < 0 ? QStringLiteral("<no column>") : t.rows.value(row).value(c).text;
    }

    static DataRows peopleRows()
    {
        DataRows t;
        t.header = {QStringLiteral("Name"), QStringLiteral("Qty"), QStringLiteral("Day")};
        auto row = [](const char *name, const char *qty, const char *day) {
            return QVector<DataCell>{{QString::fromLatin1(name), name[0] ? 's' : char(0)}, {QString::fromLatin1(qty), qty[0] ? 'n' : char(0)}, {QString::fromLatin1(day), day[0] ? 's' : char(0)}};
        };
        t.rows = {row("pear", "10", "3/4/2024"), row("Apple", "2", "12/1/2023"), row("fig", "", ""), row("apple pie", "7.5", "1/31/2024"), row("Plum", "10", "2/29/2024")};
        return t;
    }
    static QStringList column(const DataRows &t, int c)
    {
        QStringList out;
        for (const auto &r : t.rows) out << r.value(c).text;
        return out;
    }


private Q_SLOTS:
    void jsonListsAreFound()
    {
        QString why;
        QCOMPARE(dataJsonLists("[{\"a\":1}]", DataLimits(), &why), QStringList{QStringLiteral("$")});
        const QStringList nested = dataJsonLists("{\"meta\":{\"n\":1},\"items\":[{\"a\":1}],\"more\":{\"rows\":[{\"b\":2}]}}", DataLimits(), &why);
        QCOMPARE(nested, (QStringList{QStringLiteral("$.items"), QStringLiteral("$.more.rows")}));
        QVERIFY(dataJsonLists("{\"x\":[1,2,3]}", DataLimits(), &why).isEmpty());
        QVERIFY(!why.isEmpty());
        QVERIFY(dataJsonLists("{not json", DataLimits(), &why).isEmpty());
        QVERIFY(why.contains(QLatin1String("not valid JSON")));
    }

    void jsonKeepsTheOrderOfTheKeysAndReadsEscapesAndHugeNumbersWhole()
    {
        DataRows t;
        QString why;
        QVERIFY2(dataReadJson("\xEF\xBB\xBF[{\"z\":1,\"a\":\"x\\ny\\u00e9\\\"\",\"m\":12345678901234567890,\"b\":-1.5e3}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(t.header, (QStringList{QStringLiteral("z"), QStringLiteral("a"), QStringLiteral("m"), QStringLiteral("b")}));
        QCOMPARE(cell(t, 0, QStringLiteral("a")), QStringLiteral("x\ny\u00e9\""));
        QCOMPARE(cell(t, 0, QStringLiteral("m")), QStringLiteral("12345678901234567890"));   // more digits than a cell keeps: text, so none are lost
        QCOMPARE(t.rows[0][2].kind, 's');
        QCOMPARE(t.rows[0][3].kind, 'n');
        QVERIFY(!dataReadJson("[{\"a\":01x}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why));
        QVERIFY(!dataReadJson("[{\"a\":1},]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why));
        QVERIFY(!dataReadJson("[{\"a\":1}] x", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why));
    }

    // Two different sources never share a column: a name that collides gets a number, and the load's notes say so.
    void jsonNamesThatCollideGetOwnColumns()
    {
        DataRows t;
        QString why;
        QVERIFY2(dataReadJson("[{\"a.b\":1,\"a\":{\"b\":2}}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(t.header, (QStringList{QStringLiteral("a.b"), QStringLiteral("a.b2")}));
        QCOMPARE(cell(t, 0, QStringLiteral("a.b")), QStringLiteral("1"));
        QCOMPARE(cell(t, 0, QStringLiteral("a.b2")), QStringLiteral("2"));
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("new name")));
        // the other way round, and in different rows
        QVERIFY(dataReadJson("[{\"a\":{\"b\":2},\"a.b\":1},{\"a.b\":3}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why));
        QCOMPARE(t.header, (QStringList{QStringLiteral("a.b"), QStringLiteral("a.b2")}));
        QCOMPARE(cell(t, 0, QStringLiteral("a.b")), QStringLiteral("2"));
        QCOMPARE(cell(t, 0, QStringLiteral("a.b2")), QStringLiteral("1"));
        QCOMPARE(cell(t, 1, QStringLiteral("a.b2")), QStringLiteral("3"));
        // names that differ only in case are one name to a table, so they are two columns here too
        QVERIFY(dataReadJson("[{\"Id\":1,\"id\":2}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why));
        QCOMPARE(t.header.size(), 2);
        QCOMPARE(cell(t, 0, QStringLiteral("Id")), QStringLiteral("1"));
        // no collision: no note
        QVERIFY(dataReadJson("[{\"a\":1,\"b\":2}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why));
        QVERIFY(t.notes.isEmpty());
    }

    void jsonRepeatedKeyKeepsTheLastValueAndSaysSo()
    {
        DataRows t;
        QString why;
        QVERIFY2(dataReadJson("[{\"a\":1,\"a\":2,\"b\":\"x\",\"b\":\"y\"}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(t.header, (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
        QCOMPARE(cell(t, 0, QStringLiteral("a")), QStringLiteral("2"));
        QCOMPARE(cell(t, 0, QStringLiteral("b")), QStringLiteral("y"));   // not "x; y"
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("repeats a name")));
        // an object with many keys (found by the table of keys) keeps the last value too
        QByteArray many = "[{";
        for (int i = 0; i < 20; ++i) many += "\"k" + QByteArray::number(i) + "\":" + QByteArray::number(i) + ",";
        many += "\"k3\":333,\"k19\":1919}]";
        QVERIFY(dataReadJson(many, QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why));
        QCOMPARE(t.header.size(), 20);
        QCOMPARE(cell(t, 0, QStringLiteral("k3")), QStringLiteral("333"));
        QCOMPARE(cell(t, 0, QStringLiteral("k19")), QStringLiteral("1919"));
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("repeats a name")));
    }

    void xmlPrefixesKeepColumnsApartAndTopLevelTextIsKept()
    {
        DataRows t;
        QString why;
        const QByteArray xml = "<r xmlns:x=\"urn:x\"><i id=\"2\" x:id=\"1\"><name>A</name><x:name>B</x:name></i><i id=\"4\" x:id=\"3\"><name>C</name><x:name>D</x:name></i></r>";
        QVERIFY2(dataReadXml(xml, QStringLiteral("/r/i"), 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(t.header, (QStringList{QStringLiteral("id"), QStringLiteral("x:id"), QStringLiteral("name"), QStringLiteral("x:name")}));
        QCOMPARE(cell(t, 0, QStringLiteral("id")), QStringLiteral("2"));
        QCOMPARE(cell(t, 0, QStringLiteral("x:id")), QStringLiteral("1"));
        QCOMPARE(cell(t, 1, QStringLiteral("x:name")), QStringLiteral("D"));
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("new name")));
        // the prefixed one first: it is the prefixed one that is named with its prefix
        QVERIFY(dataReadXml("<r xmlns:x=\"urn:x\"><i x:id=\"1\" id=\"2\"/><i x:id=\"3\" id=\"4\"/></r>", QStringLiteral("/r/i"), 0, DataLimits(), &t, &why));
        QCOMPARE(t.header, (QStringList{QStringLiteral("x:id"), QStringLiteral("id")}));
        QCOMPARE(cell(t, 0, QStringLiteral("x:id")), QStringLiteral("1"));
        QCOMPARE(cell(t, 0, QStringLiteral("id")), QStringLiteral("2"));
        // two prefixes for one address are one column
        QVERIFY(dataReadXml("<r xmlns:x=\"urn:x\" xmlns:y=\"urn:x\"><i x:id=\"1\"/><i y:id=\"2\"/></r>", QStringLiteral("/r/i"), 0, DataLimits(), &t, &why));
        QCOMPARE(t.header.size(), 1);
        QCOMPARE(cell(t, 1, QStringLiteral("id")), QStringLiteral("2"));
        // text straight inside a row element that also has elements is kept
        QVERIFY(dataReadXml("<r><i>t1<b>x</b>t2</i><i>u</i></r>", QStringLiteral("/r/i"), 0, DataLimits(), &t, &why));
        QCOMPARE(cell(t, 0, QStringLiteral("text")), QStringLiteral("t1t2"));
        QCOMPARE(cell(t, 0, QStringLiteral("b")), QStringLiteral("x"));
        QCOMPARE(cell(t, 1, QStringLiteral("text")), QStringLiteral("u"));
        QCOMPARE(t.header.size(), 2);
    }

    void jsonNumbersOutOfRangeAreKeptAsText()
    {
        DataRows t;
        QString why;
        QVERIFY2(dataReadJson("[{\"a\":1e400,\"b\":-1E-400,\"c\":5,\"d\":0e5,\"e\":1e308}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(cell(t, 0, QStringLiteral("a")), QStringLiteral("1e400"));
        QCOMPARE(cell(t, 0, QStringLiteral("b")), QStringLiteral("-1E-400"));
        QCOMPARE(t.rows[0][0].kind, 's');
        QCOMPARE(t.rows[0][1].kind, 's');
        QCOMPARE(t.rows[0][2].kind, 'n');
        QCOMPARE(t.rows[0][3].kind, 'n');   // zero written with an exponent is zero
        QCOMPARE(t.rows[0][4].kind, 'n');
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("Some numbers")));
        const QVector<char> kinds = dataColumnKinds(t, false);
        QCOMPARE(kinds[0], 't');   // text that looks like a number but does not fit one stays text
        QCOMPARE(kinds[1], 't');
        QCOMPARE(kinds[2], 'n');
    }

    void jsonFormsThatAreNotJsonAreRefusedWithAPosition()
    {
        DataRows t;
        QString why;
        for (const char *bad : {"[{\"a\":+1}]", "[{\"a\":.5}]", "[{\"a\":01}]", "[{\"a\":-}]", "[{\"a\":1.}]", "[{\"a\":1e}]", "[{\"a\":1e+}]", "[{\"a\":\"\\u12\"}]", "[{\"a\":\"\\u12g4\"}]", "[{\"a\":\"\\u+123\"}]", "[{\"a\":\"\\u 123\"}]", "[{\"a\":--1}]", "[{\"a\":0x10}]"}) {
            why.clear();
            QVERIFY2(!dataReadJson(bad, QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), bad);
            QVERIFY2(why.contains(QLatin1String("not valid JSON")) && why.contains(QLatin1String("near byte")), bad);
        }
        for (const char *good : {"[{\"a\":0}]", "[{\"a\":-0}]", "[{\"a\":0.5}]", "[{\"a\":10e+2}]", "[{\"a\":-1.25E-2}]", "[{\"a\":\"\\u00E9\\uD83D\\uDE00\"}]"})
            QVERIFY2(dataReadJson(good, QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), good);
        QCOMPARE(cell(t, 0, QStringLiteral("a")), QString::fromUtf8("\xC3\xA9\xF0\x9F\x98\x80"));
        QVERIFY(t.notes.isEmpty());
    }

    void jsonCharactersThatCannotBeReadAreReplacedAndSaid()
    {
        DataRows t;
        QString why;
        QVERIFY2(dataReadJson("[{\"a\":\"x\\ud800y\",\"b\":\"p\\udc00\\ud800q\"}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(cell(t, 0, QStringLiteral("a")), QStringLiteral("x\uFFFDy"));
        QCOMPARE(cell(t, 0, QStringLiteral("b")), QStringLiteral("p\uFFFD\uFFFDq"));
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("could not be read")));
        QVERIFY(dataReadJson("[{\"a\":\"x\xFF" "y\"}]", QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why));
        QCOMPARE(cell(t, 0, QStringLiteral("a")), QStringLiteral("x\uFFFDy"));
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("could not be read")));
    }

    void listsCutByALimitAreSaid()
    {
        DataRows t;
        QString why;
        QByteArray big = "[{\"v\":[";
        for (int i = 0; i < 10001; ++i) big += (i ? ",1" : "1");
        big += "]}]";
        QVERIFY2(dataReadJson(big, QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(cell(t, 0, QStringLiteral("v")).count(QLatin1Char(';')), 9999);   // 10,000 items
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("10,000")));
        QByteArray wide = "[{\"v\":[";
        for (int i = 0; i < 20; ++i) wide += (i ? ",1" : "1");
        wide += "]}]";
        QVERIFY(dataReadJson(wide, QStringLiteral("$"), ArrayMode::Columns, 0, DataLimits(), &t, &why));
        QCOMPARE(t.header.size(), 16);
        QVERIFY(t.notes.join(QLatin1Char(' ')).contains(QLatin1String("first 16 columns")));
        QVERIFY(dataReadJson("[{\"v\":[1,2,3]}]", QStringLiteral("$"), ArrayMode::Columns, 0, DataLimits(), &t, &why));
        QVERIFY(t.notes.isEmpty());
    }

    void jsonNestingAndNodeCountsAreBounded()
    {
        QString why;
        QByteArray deep;
        for (int i = 0; i < 100000; ++i) deep += "[";
        QVERIFY(dataJsonLists(deep, DataLimits(), &why).isEmpty());
        QVERIFY(why.contains(QLatin1String("deeply")));
        QByteArray many = "[{\"a\":[";
        for (int i = 0; i < 4100000; ++i) many += "1,";
        many += "1]}]";
        DataLimits big;
        big.maxBytes = qint64(64) << 20;
        QVERIFY(dataJsonLists(many, big, &why).isEmpty());
        QVERIFY(why.contains(QLatin1String("more values")));
    }

    void jsonObjectsBecomeRowsAndFlattenedColumns()
    {
        const QByteArray json = "[{\"id\":1,\"name\":\"Ann\",\"ok\":true,\"addr\":{\"city\":\"Oslo\",\"geo\":{\"lat\":59.9}},\"tags\":[\"x\",\"y\"]},"
                                "{\"id\":2,\"name\":null,\"extra\":\"late\",\"tags\":[]}]";
        DataRows t;
        QString why;
        QVERIFY2(dataReadJson(json, QStringLiteral("$"), ArrayMode::Join, 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(t.rows.size(), 2);
        QCOMPARE(cell(t, 0, QStringLiteral("name")), QStringLiteral("Ann"));
        QCOMPARE(cell(t, 0, QStringLiteral("addr.city")), QStringLiteral("Oslo"));
        QCOMPARE(cell(t, 0, QStringLiteral("addr.geo.lat")), QStringLiteral("59.9"));
        QCOMPARE(cell(t, 0, QStringLiteral("tags")), QStringLiteral("x; y"));
        QCOMPARE(t.rows[0][t.header.indexOf(QStringLiteral("ok"))].kind, 'b');
        QCOMPARE(t.rows[0][t.header.indexOf(QStringLiteral("id"))].kind, 'n');
        QCOMPARE(t.rows[1][t.header.indexOf(QStringLiteral("name"))].kind, char(0));
        QCOMPARE(cell(t, 1, QStringLiteral("extra")), QStringLiteral("late"));
        QCOMPARE(cell(t, 0, QStringLiteral("extra")), QString());
        QVERIFY(t.rows[0].size() == t.header.size() && t.rows[1].size() == t.header.size());
        DataRows cols;
        QVERIFY(dataReadJson(json, QStringLiteral("$"), ArrayMode::Columns, 0, DataLimits(), &cols, &why));
        QCOMPARE(cell(cols, 0, QStringLiteral("tags.2")), QStringLiteral("y"));
    }

    void jsonDeepObjectsAreKeptAsTextAndSaid()
    {
        DataLimits limits;
        limits.maxDepth = 2;
        DataRows t;
        QString why;
        QVERIFY(dataReadJson("[{\"a\":{\"b\":{\"c\":{\"d\":1}}}}]", QStringLiteral("$"), ArrayMode::Join, 0, limits, &t, &why));
        QCOMPARE(t.header, QStringList{QStringLiteral("a.b")});
        QCOMPARE(cell(t, 0, QStringLiteral("a.b")), QStringLiteral("{\"c\":{\"d\":1}}"));
        QVERIFY(!t.notes.isEmpty());
    }

    void jsonLimitsHoldBeforeWork()
    {
        DataLimits limits;
        limits.maxRows = 3;
        limits.maxCols = 2;
        limits.maxStringChars = 4;
        QByteArray json = "[";
        for (int i = 0; i < 10; ++i) json += QByteArray(i ? "," : "") + "{\"a\":\"abcdefgh\",\"b\":1,\"c\":2}";
        json += "]";
        DataRows t;
        QString why;
        QVERIFY(dataReadJson(json, QStringLiteral("$"), ArrayMode::Join, 0, limits, &t, &why));
        QCOMPARE(t.rows.size(), 3);
        QCOMPARE(t.header.size(), 2);
        QCOMPARE(cell(t, 0, QStringLiteral("a")), QStringLiteral("abcd"));
        QCOMPARE(t.notes.size(), 3);
        DataRows preview;
        QVERIFY(dataReadJson(json, QStringLiteral("$"), ArrayMode::Join, 2, DataLimits(), &preview, &why));
        QCOMPARE(preview.rows.size(), 2);
        limits.maxBytes = 10;
        QVERIFY(!dataReadJson(json, QStringLiteral("$"), ArrayMode::Join, 0, limits, &t, &why));
        QVERIFY(why.contains(QLatin1String("larger than")));
    }

    void xmlRepeatingItemsAreFoundAndRead()
    {
        const QByteArray xml = "<feed><title>T</title><entry id=\"1\"><name>Ann</name><geo><lat>5</lat></geo></entry>"
                               "<entry id=\"2\"><name>Bob</name><note>hi</note></entry></feed>";
        QString why;
        QCOMPARE(dataXmlLists(xml, DataLimits(), &why), QStringList{QStringLiteral("/feed/entry")});
        DataRows t;
        QVERIFY2(dataReadXml(xml, QStringLiteral("/feed/entry"), 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(t.rows.size(), 2);
        QCOMPARE(cell(t, 0, QStringLiteral("id")), QStringLiteral("1"));
        QCOMPARE(cell(t, 1, QStringLiteral("name")), QStringLiteral("Bob"));
        QCOMPARE(cell(t, 0, QStringLiteral("geo.lat")), QStringLiteral("5"));
        QCOMPARE(cell(t, 1, QStringLiteral("note")), QStringLiteral("hi"));
        QCOMPARE(cell(t, 0, QStringLiteral("note")), QString());
        DataRows preview;
        QVERIFY(dataReadXml(xml, QStringLiteral("/feed/entry"), 1, DataLimits(), &preview, &why));
        QCOMPARE(preview.rows.size(), 1);
        QVERIFY(dataXmlLists("<a><b/></a>", DataLimits(), &why).isEmpty());
        QVERIFY(!dataReadXml(xml, QStringLiteral("/feed/nothing"), 0, DataLimits(), &t, &why));
    }

    void xmlWithADocumentTypeOrEntitiesIsRefused()
    {
        const QByteArray bomb = "<!DOCTYPE a [<!ENTITY x \"aaaaaaaaaa\"><!ENTITY y \"&x;&x;&x;&x;&x;&x;&x;&x;\">]><a><b>&y;</b><b>&y;</b></a>";
        QString why;
        QVERIFY(dataXmlLists(bomb, DataLimits(), &why).isEmpty());
        QVERIFY(why.contains(QLatin1String("document type")));
        DataRows t;
        why.clear();
        QVERIFY(!dataReadXml(bomb, QStringLiteral("/a/b"), 0, DataLimits(), &t, &why));
        QVERIFY(why.contains(QLatin1String("document type")));
        QVERIFY(!dataReadXml("<a><b>x</b><b>y", QStringLiteral("/a/b"), 0, DataLimits(), &t, &why));
        QVERIFY(why.contains(QLatin1String("not valid XML")));
    }

    void xmlDepthAndSizeAreBounded()
    {
        DataLimits limits;
        limits.maxXmlDepth = 20;
        QByteArray xml;
        for (int i = 0; i < 100000; ++i) xml += "<a>";
        QString why;
        QVERIFY(dataXmlLists(xml, limits, &why).isEmpty());
        QVERIFY(why.contains(QLatin1String("deeply")));
        limits.maxBytes = 5;
        why.clear();
        QVERIFY(dataXmlLists("<a><b/><b/></a>", limits, &why).isEmpty());
        QVERIFY(why.contains(QLatin1String("larger than")));
    }

    void aConnectionIsKeptAsTextAndReadBack()
    {
        DataConnection c;
        c.kind = QStringLiteral("json");
        c.path = QStringLiteral("/tmp/a b&c=d/x%y.json");
        c.list = QStringLiteral("$.items");
        c.mode = ArrayMode::Columns;
        c.asText = true;
        DataConnection back;
        QVERIFY(DataConnection::parse(c.text(), &back));
        QCOMPARE(back.path, c.path);
        QCOMPARE(back.list, c.list);
        QVERIFY(back.mode == ArrayMode::Columns);
        QVERIFY(back.asText);
        QVERIFY(!DataConnection::parse(QStringLiteral("kind=json"), &back));
        QVERIFY(!DataConnection::parse(QStringLiteral("jo-data:1?kind=exe&path=x"), &back));
    }

    void aConnectionKeepsTheNamesOfTheFilesColumns()
    {
        DataConnection c;
        c.kind = QStringLiteral("json");
        c.path = QStringLiteral("/tmp/a b.json");
        c.columns = QStringList{QStringLiteral("a,b"), QStringLiteral("c&d=e"), QStringLiteral("\u00e9 f"), QStringLiteral("100%")};
        DataConnection back;
        QVERIFY(DataConnection::parse(c.text(), &back));
        QCOMPARE(back.columns, c.columns);
        c.columns.clear();
        QVERIFY(DataConnection::parse(c.text(), &back));
        QVERIFY(back.columns.isEmpty());
        // more names than fit are left out, and the link still reads
        for (int i = 0; i < 256; ++i) c.columns << QString(400, QLatin1Char('&')) + QString::number(i);
        QVERIFY(DataConnection::parse(c.text(), &back));
        QVERIFY(back.columns.isEmpty());
    }

    void onlyTheExtensionWithOurAddressesIsRead()
    {
        Table t;
        t.name = QStringLiteral("Table1");
        t.row1 = 0;
        t.col1 = 0;
        t.row2 = 2;
        t.col2 = 1;
        t.columns = QStringList{QStringLiteral("a"), QStringLiteral("b")};
        t.info.resize(2);
        t.info[0].id = 1;
        t.info[1].id = 2;
        t.id = 1;
        t.dataSource = QStringLiteral("jo-data:1?kind=json&path=p");
        const QByteArray part = writeTablePart(t, QByteArray());
        Table back;
        back.columns = t.columns;
        QVERIFY(readTableModel(part, &back));
        QCOMPARE(back.dataSource, t.dataSource);
        // the same local name under another namespace address, or in another extension, is another program's
        QByteArray otherNs = part;
        otherNs.replace("urn:jeffoffice:data-source", "urn:somebody:else");
        QVERIFY(readTableModel(otherNs, &back));
        QVERIFY(back.dataSource.isEmpty());
        QByteArray otherExt = part;
        otherExt.replace("{6A1F0C3E-5B7D-4E29-9C41-0D3B7A52E8F1}", "{00000000-0000-0000-0000-000000000000}");
        QVERIFY(readTableModel(otherExt, &back));
        QVERIFY(back.dataSource.isEmpty());
        // and a table written over a part holding that other program's element keeps it
        t.dataSource.clear();
        const QByteArray over = writeTablePart(t, otherExt);
        QVERIFY(over.contains("00000000-0000-0000-0000-000000000000"));
    }

    void theConnectionIsSavedInTheTablePartAndTheOtherExtensionsStay()
    {
        Table t;
        t.name = QStringLiteral("Table1");
        t.row1 = 0;
        t.col1 = 0;
        t.row2 = 2;
        t.col2 = 1;
        t.columns = QStringList{QStringLiteral("a"), QStringLiteral("b")};
        t.info.resize(2);
        t.info[0].id = 1;
        t.info[1].id = 2;
        t.id = 1;
        t.dataSource = QStringLiteral("jo-data:1?kind=xml&path=%2Ftmp%2Fx.xml&list=%2Fa%2Fb&mode=join&text=0");
        const QByteArray part = writeTablePart(t, QByteArray());
        QVERIFY(part.contains("dataSource"));
        Table back;
        back.columns = t.columns;
        QVERIFY(readTableModel(part, &back));
        QCOMPARE(back.dataSource, t.dataSource);
        // written over a part that has another program's extension: both stay; and the link can be taken away
        QByteArray other = part;
        other.replace("</table>", "</table>");
        other.replace("<ext ", "<ext ");
        t.dataSource.clear();
        const QByteArray without = writeTablePart(t, part);
        QVERIFY(!without.contains("dataSource"));
        QVERIFY(!without.contains("extLst"));
        const QByteArray foreign = "<?xml version=\"1.0\"?><table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"T\" displayName=\"T\" ref=\"A1:B3\"><tableColumns count=\"2\"><tableColumn id=\"1\" name=\"a\"/><tableColumn id=\"2\" name=\"b\"/></tableColumns><extLst><ext uri=\"{1}\"><x:keep xmlns:x=\"urn:x\"/></ext></extLst></table>";
        t.dataSource = QStringLiteral("jo-data:1?kind=json&path=p");
        const QByteArray both = writeTablePart(t, foreign);
        QVERIFY(both.contains("x:keep") && both.contains("dataSource"));
        t.dataSource.clear();
        const QByteArray kept = writeTablePart(t, both);
        QVERIFY(kept.contains("x:keep") && !kept.contains("dataSource"));
    }

    void columnKindsAreDetectedAndTextCanBeChosen()
    {
        DataRows t;
        t.header = QStringList{QStringLiteral("n"), QStringLiteral("id"), QStringLiteral("d"), QStringLiteral("b"), QStringLiteral("mixed"), QStringLiteral("empty")};
        auto row = [](const QStringList &v) {
            QVector<DataCell> r;
            for (const QString &x : v) r.push_back(DataCell{x, x.isEmpty() ? char(0) : 's'});
            return r;
        };
        t.rows << row({QStringLiteral("1.5"), QStringLiteral("007"), QStringLiteral("2024-02-29"), QStringLiteral("true"), QStringLiteral("1"), QString()});
        t.rows << row({QStringLiteral("-2e3"), QStringLiteral("010"), QStringLiteral("2023-12-01"), QStringLiteral("FALSE"), QStringLiteral("x"), QString()});
        QCOMPARE(dataColumnKinds(t, false), (QVector<char>{'n', 't', 'd', 'b', 't', 't'}));
        QCOMPARE(dataColumnKinds(t, true), (QVector<char>(6, 't')));
        t.rows << row({QStringLiteral("1"), QStringLiteral("5"), QStringLiteral("2023-02-30"), QStringLiteral("true"), QStringLiteral("1"), QString()});
        QCOMPARE(dataColumnKinds(t, false).at(2), 't');
    }

    void onlyAFileOnThisComputerIsRead()
    {
        for (const char *p : {"http://example.com/a.json", "https://x/y", "ftp://h/f", "\\\\server\\share\\a.json", "//server/share/a.json", "file:///etc/passwd"}) {
            QByteArray bytes;
            QString why;
            QVERIFY2(dataPathIsRemote(QString::fromLatin1(p)), p);
            QVERIFY(!dataReadFile(QString::fromLatin1(p), DataLimits(), nullptr, &bytes, &why));
            QVERIFY(why.contains(QLatin1String("on this computer")));
        }
        QVERIFY(!dataPathIsRemote(QStringLiteral("C:\\data\\a.json")));
        QVERIFY(!dataPathIsRemote(QStringLiteral("/home/u/a.json")));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QByteArray bytes;
        QString why;
        QVERIFY(!dataReadFile(dir.path(), DataLimits(), nullptr, &bytes, &why));
        QVERIFY(why.contains(QLatin1String("not a file")));
        QFile f(dir.filePath(QStringLiteral("a.json")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("[{\"a\":1}]");
        f.close();
        QVERIFY(dataReadFile(f.fileName(), DataLimits(), nullptr, &bytes, &why));
        QCOMPARE(bytes, QByteArray("[{\"a\":1}]"));
        DataLimits small;
        small.maxBytes = 3;
        QVERIFY(!dataReadFile(f.fileName(), small, nullptr, &bytes, &why));
        QVERIFY(why.contains(QLatin1String("larger than")));
        int asked = 0;
        QVERIFY(!dataReadFile(f.fileName(), DataLimits(), [&asked] { return ++asked > 0; }, &bytes, &why));
        QVERIFY(why.contains(QLatin1String("stopped")));
    }

    // The rows and columns made come from the cells that are there: one cell at the far corner of a sheet does not make a table the size of the limits.
    void aSparseSheetIsSizedFromItsCells()
    {
        auto str = [](const char *ref, const char *text) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(QLatin1String(ref), QLatin1String(text)); };
        const QString rows = QStringLiteral("<row r=\"1\">") + str("A1", "name") + QStringLiteral("</row><row r=\"2\">") + str("A2", "Ann") + QStringLiteral("</row><row r=\"1048576\"><c r=\"XFD1048576\"><v>1</v></c></row>");
        const QByteArray book = sheettest::buildXlsx({sheettest::worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList{QStringLiteral("Data")});
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile f(dir.filePath(QStringLiteral("sparse.xlsx")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(book);
        f.close();
        DataRows t;
        QString why;
        QVERIFY2(dataReadBook(f.fileName(), QStringLiteral("sheet:Data"), 0, DataLimits(), nullptr, &t, &why), qPrintable(why));
        // the cell in the corner is past the limits and is left out; what is made is as big as the cells read: one column, and no more rows than the cells that are there
        QCOMPARE(t.header.size(), 1);
        QCOMPARE(t.rows.size(), 1);
        QCOMPARE(t.rows[0].size(), 1);
        // inside the limits the size follows the cells too: the last cell is at column 3 and row 5
        const QString rows2 = QStringLiteral("<row r=\"1\">") + str("A1", "a") + str("B1", "b") + QStringLiteral("</row><row r=\"2\">") + str("A2", "x") + QStringLiteral("</row><row r=\"5\">") + str("C5", "z") + QStringLiteral("</row>");
        QFile g(dir.filePath(QStringLiteral("small.xlsx")));
        QVERIFY(g.open(QIODevice::WriteOnly));
        g.write(sheettest::buildXlsx({sheettest::worksheet(rows2)}, QByteArray(), QString(), QString(), QByteArray(), QStringList{QStringLiteral("Data")}, QStringLiteral("<definedName name=\"Wide\">Data!$A$1:$XFD$1000</definedName>")));
        g.close();
        QVERIFY(dataReadBook(g.fileName(), QStringLiteral("name:Wide"), 0, DataLimits(), nullptr, &t, &why));
        QCOMPARE(t.header.size(), 3);
        QCOMPARE(t.rows.size(), 4);
        QCOMPARE(t.rows[0].size(), 3);
    }

    void aSheetAndANamedRangeOfAnotherWorkbookAreRows()
    {
        auto str = [](const char *ref, const char *text) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(QLatin1String(ref), QLatin1String(text)); };
        const QString rows = QStringLiteral("<row r=\"1\">") + str("A1", "name") + str("B1", "n") + QStringLiteral("</row><row r=\"2\">") + str("A2", "Ann") + QStringLiteral("<c r=\"B2\"><v>5</v></c></row><row r=\"3\">")
                             + str("A3", "Bob") + QStringLiteral("<c r=\"B3\" t=\"b\"><v>1</v></c></row><row r=\"4\">") + str("A4", "=1+1") + QStringLiteral("</row>");
        const QByteArray book = sheettest::buildXlsx({sheettest::worksheet(rows), sheettest::worksheet(QString())}, QByteArray(), QString(), QString(), QByteArray(), QStringList{QStringLiteral("Data"), QStringLiteral("Empty")},
                                                    QStringLiteral("<definedName name=\"Block\">Data!$A$1:$B$2</definedName>"));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile f(dir.filePath(QStringLiteral("b.xlsx")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(book);
        f.close();
        QString why;
        const QStringList lists = dataBookLists(f.fileName(), DataLimits(), nullptr, &why);
        QCOMPARE(lists, (QStringList{QStringLiteral("sheet:Data"), QStringLiteral("sheet:Empty"), QStringLiteral("name:Block")}));
        DataRows t;
        QVERIFY2(dataReadBook(f.fileName(), QStringLiteral("sheet:Data"), 0, DataLimits(), nullptr, &t, &why), qPrintable(why));
        QCOMPARE(t.header, (QStringList{QStringLiteral("name"), QStringLiteral("n")}));
        QCOMPARE(t.rows.size(), 3);
        QCOMPARE(cell(t, 0, QStringLiteral("n")), QStringLiteral("5"));
        QCOMPARE(t.rows[0][1].kind, 'n');
        QCOMPARE(t.rows[1][1].kind, 'b');
        QCOMPARE(cell(t, 2, QStringLiteral("name")), QStringLiteral("=1+1"));
        DataRows named;
        QVERIFY2(dataReadBook(f.fileName(), QStringLiteral("name:Block"), 0, DataLimits(), nullptr, &named, &why), qPrintable(why));
        QCOMPARE(named.rows.size(), 1);
        DataRows preview;
        QVERIFY(dataReadBook(f.fileName(), QStringLiteral("sheet:Data"), 1, DataLimits(), nullptr, &preview, &why));
        QCOMPARE(preview.rows.size(), 1);
        QVERIFY(!dataReadBook(f.fileName(), QStringLiteral("sheet:Empty"), 0, DataLimits(), nullptr, &t, &why));
        QVERIFY(!dataReadBook(f.fileName(), QStringLiteral("sheet:Nope"), 0, DataLimits(), nullptr, &t, &why));
        QVERIFY(!dataReadBook(f.fileName(), QStringLiteral("sheet:Data"), 0, DataLimits(), [] { return true; }, &t, &why));
        QVERIFY(why.contains(QLatin1String("stopped")));
        QVERIFY(dataBookLists(QStringLiteral("https://x/y.xlsx"), DataLimits(), nullptr, &why).isEmpty());
        QVERIFY(why.contains(QLatin1String("on this computer")));
        QFile bad(dir.filePath(QStringLiteral("bad.xlsx")));
        QVERIFY(bad.open(QIODevice::WriteOnly));
        bad.write("not a workbook");
        bad.close();
        QVERIFY(dataBookLists(bad.fileName(), DataLimits(), nullptr, &why).isEmpty());
        QVERIFY(!why.isEmpty());
    }
    // ---- The steps of a query (datasteps.h)
    void everyStepKindDoesItsWork()
    {
        DataRows t = peopleRows();
        DataStep filter;
        filter.op = DataStep::Op::Filter;
        filter.col = QStringLiteral("qty");   // (any case)
        filter.cmp = QStringLiteral("ge");
        filter.value = QStringLiteral("7.5");
        DataStep sort;
        sort.op = DataStep::Op::Sort;
        sort.col = QStringLiteral("Name");
        DataStep rename;
        rename.op = DataStep::Op::Rename;
        rename.col = QStringLiteral("Qty");
        rename.to = QStringLiteral("Amount");
        DataStep keep;
        keep.op = DataStep::Op::Keep;
        keep.cols = {QStringLiteral("Amount"), QStringLiteral("Name")};
        const DataStepsRun run = applyDataSteps({filter, sort, rename, keep}, &t, nullptr);
        QVERIFY2(run.ok, qPrintable(run.why));
        QCOMPARE(t.header, (QStringList{QStringLiteral("Amount"), QStringLiteral("Name")}));
        QCOMPARE(column(t, 1), (QStringList{QStringLiteral("apple pie"), QStringLiteral("pear"), QStringLiteral("Plum")}));   // (the filter keeps 10, 7.5, 10; the sort ignores case)
        QCOMPARE(column(t, 0), (QStringList{QStringLiteral("7.5"), QStringLiteral("10"), QStringLiteral("10")}));
        // remove, the other way to choose; and the comparisons
        t = peopleRows();
        DataStep remove;
        remove.op = DataStep::Op::Remove;
        remove.cols = {QStringLiteral("Day")};
        QVERIFY(applyDataSteps({remove}, &t, nullptr).ok);
        QCOMPARE(t.header, (QStringList{QStringLiteral("Name"), QStringLiteral("Qty")}));
        auto count = [](const QString &col, const QString &cmp, const QString &value) {
            DataRows rows = peopleRows();
            DataStep f;
            f.op = DataStep::Op::Filter;
            f.col = col;
            f.cmp = cmp;
            f.value = value;
            return applyDataSteps({f}, &rows, nullptr).ok ? int(rows.rows.size()) : -1;
        };
        QCOMPARE(count(QStringLiteral("Name"), QStringLiteral("eq"), QStringLiteral("FIG")), 1);
        QCOMPARE(count(QStringLiteral("Name"), QStringLiteral("ne"), QStringLiteral("fig")), 4);
        QCOMPARE(count(QStringLiteral("Name"), QStringLiteral("contains"), QStringLiteral("APPLE")), 2);
        QCOMPARE(count(QStringLiteral("Name"), QStringLiteral("notcontains"), QStringLiteral("apple")), 3);
        QCOMPARE(count(QStringLiteral("Qty"), QStringLiteral("gt"), QStringLiteral("9")), 2);   // (as numbers: 10 is more than 9, not less as text)
        QCOMPARE(count(QStringLiteral("Qty"), QStringLiteral("lt"), QStringLiteral("9")), 2);
        QCOMPARE(count(QStringLiteral("Qty"), QStringLiteral("le"), QStringLiteral("10")), 4);   // (the blank is not less than anything)
        QCOMPARE(count(QStringLiteral("Qty"), QStringLiteral("blank"), QString()), 1);
        QCOMPARE(count(QStringLiteral("Qty"), QStringLiteral("notblank"), QString()), 4);
        QCOMPARE(count(QStringLiteral("Qty"), QStringLiteral("eq"), QStringLiteral("10.0")), 2);
        // a sort puts the blank last in both directions, numbers before text, and keeps the order of equals
        t = peopleRows();
        DataStep byQty;
        byQty.op = DataStep::Op::Sort;
        byQty.col = QStringLiteral("Qty");
        byQty.descending = true;
        QVERIFY(applyDataSteps({byQty}, &t, nullptr).ok);
        QCOMPARE(column(t, 0), (QStringList{QStringLiteral("pear"), QStringLiteral("Plum"), QStringLiteral("apple pie"), QStringLiteral("Apple"), QStringLiteral("fig")}));
    }

    void changingAColumnsKindReadsTheLocaleAndNamesWhatItCannot()
    {
        DataRows t;
        t.header = {QStringLiteral("Price"), QStringLiteral("Day")};
        t.rows = {{{QStringLiteral("1.234,50"), 's'}, {QStringLiteral("31/12/2023"), 's'}}, {{QString(), 0}, {QString(), 0}}, {{QStringLiteral(" 7 "), 's'}, {QStringLiteral("1.2.2024"), 's'}}};
        DataStep price;
        price.op = DataStep::Op::Kind;
        price.col = QStringLiteral("Price");
        price.kind = QStringLiteral("number");
        price.decimal = QStringLiteral(",");
        price.group = QStringLiteral(".");
        DataStep day;
        day.op = DataStep::Op::Kind;
        day.col = QStringLiteral("Day");
        day.kind = QStringLiteral("date");
        day.order = QStringLiteral("DMY");
        const DataStepsRun run = applyDataSteps({price, day}, &t, nullptr);
        QVERIFY2(run.ok, qPrintable(run.why));
        QCOMPARE(column(t, 0), (QStringList{QStringLiteral("1234.5"), QString(), QStringLiteral("7")}));
        QCOMPARE(column(t, 1), (QStringList{QStringLiteral("2023-12-31"), QString(), QStringLiteral("2024-02-01")}));
        QCOMPARE(run.forcedKinds, (QVector<char>{'n', 'd'}));
        // month first reads the same text as another day; a day that is not there is refused, in words that name the step, the row, and the value
        t.rows = {{{QStringLiteral("1"), 's'}, {QStringLiteral("2/30/2024"), 's'}}};
        day.order = QStringLiteral("MDY");
        const DataStepsRun bad = applyDataSteps({day}, &t, nullptr);
        QVERIFY(!bad.ok);
        QCOMPARE(bad.failedStep, 0);
        QVERIFY(bad.why.startsWith(QLatin1String("Step 1 (Change Day to date): The value \"2/30/2024\" in row 1")));
        // text keeps the text a number was read as
        t.rows = {{{QStringLiteral("007"), 'n'}, {QStringLiteral("x"), 's'}}};
        DataStep text;
        text.op = DataStep::Op::Kind;
        text.col = QStringLiteral("Price");
        text.kind = QStringLiteral("text");
        QVERIFY(applyDataSteps({text}, &t, nullptr).ok);
        QCOMPARE(t.rows[0][0].kind, 's');
    }

    void aStepThatCannotRunNamesItself()
    {
        DataRows t = peopleRows();
        DataStep keep;
        keep.op = DataStep::Op::Keep;
        keep.cols = {QStringLiteral("Name")};
        DataStep filter;
        filter.op = DataStep::Op::Filter;
        filter.col = QStringLiteral("Qty");   // (not there after the first step)
        filter.cmp = QStringLiteral("gt");
        filter.value = QStringLiteral("1");
        const DataStepsRun run = applyDataSteps({keep, filter}, &t, nullptr);
        QVERIFY(!run.ok);
        QCOMPARE(run.failedStep, 1);
        QCOMPARE(run.why, QStringLiteral("Step 2 (Keep rows where Qty is greater than 1): The column \"Qty\" is not there. The columns are: Name."));
        DataStep twice;
        twice.op = DataStep::Op::Keep;
        twice.cols = {QStringLiteral("Name"), QStringLiteral("name")};
        t = peopleRows();
        QVERIFY(applyDataSteps({twice}, &t, nullptr).why.contains(QLatin1String("named twice")));
        DataStep none;
        none.op = DataStep::Op::Remove;
        none.cols = {QStringLiteral("Name"), QStringLiteral("Qty"), QStringLiteral("Day")};
        t = peopleRows();
        QVERIFY(applyDataSteps({none}, &t, nullptr).why.contains(QLatin1String("no columns")));
        DataStep clash;
        clash.op = DataStep::Op::Rename;
        clash.col = QStringLiteral("Qty");
        clash.to = QStringLiteral("name");
        t = peopleRows();
        QVERIFY(applyDataSteps({clash}, &t, nullptr).why.contains(QLatin1String("already called")));
    }

    void theStepsLimitsAreCounted()
    {
        // the work: a limit smaller than the rows of one step stops it before the step runs; at the limit it runs
        DataRows t = peopleRows();
        DataStep filter;
        filter.op = DataStep::Op::Filter;
        filter.col = QStringLiteral("Name");
        filter.cmp = QStringLiteral("notblank");
        DataStepsRun run = applyDataSteps({filter, filter, filter}, &t, nullptr, 14);   // (5 rows each: 15 visits)
        QVERIFY(!run.ok);
        QCOMPARE(run.failedStep, 2);
        QCOMPARE(run.work, qint64(15));
        QVERIFY(run.why.contains(QLatin1String("too long")));
        t = peopleRows();
        QVERIFY(applyDataSteps({filter, filter, filter}, &t, nullptr, 15).ok);
        // the number of steps
        QVector<DataStep> many(kMaxDataSteps + 1, filter);
        QVERIFY(!applyDataSteps(many, &t, nullptr).ok);
        QVector<DataStep> back;
        QString why;
        QVERIFY(!parseDataSteps(dataStepsToJson(many), &back, &why));
        QVERIFY(why.contains(QLatin1String("more steps")));
        many.resize(kMaxDataSteps);
        QVERIFY(parseDataSteps(dataStepsToJson(many), &back, &why));
        QCOMPARE(back.size(), kMaxDataSteps);
        // a name or a value past its limit, and a column list past 256
        DataStep long1 = filter;
        long1.value = QString(kMaxDataStepChars + 1, QLatin1Char('x'));
        QVERIFY(!parseDataSteps(dataStepsToJson({long1}), &back, &why));
        DataStep wide;
        wide.op = DataStep::Op::Keep;
        for (int i = 0; i < 257; ++i) wide.cols << QStringLiteral("c%1").arg(i);
        QVERIFY(!parseDataSteps(dataStepsToJson({wide}), &back, &why));
        // cancel: asked as it goes, and the run says it was stopped
        t = peopleRows();
        int asked = 0;
        run = applyDataSteps({filter}, &t, [&] { ++asked; return true; });
        QVERIFY(!run.ok);
        QVERIFY(asked >= 1);
        QVERIFY(run.why.contains(QLatin1String("stopped")));
    }

    void stepsAreSavedInTheConnectionTextAndANewerOneIsKeptAsItWas()
    {
        DataConnection c;
        c.kind = QStringLiteral("json");
        c.path = QStringLiteral("/home/ann & bob/rows+1.json");
        c.name = QStringLiteral("Rows \"1\"");
        c.ranAt = 1795000000;
        c.rowCount = 12;
        DataStep f;
        f.op = DataStep::Op::Filter;
        f.col = QStringLiteral("a&b");
        f.cmp = QStringLiteral("contains");
        f.value = QStringLiteral("line\none\t\"two\" <x> % + \u2713");
        DataStep k;
        k.op = DataStep::Op::Kind;
        k.col = QStringLiteral("d");
        k.kind = QStringLiteral("date");
        k.order = QStringLiteral("YMD");
        k.label = QStringLiteral("Make dates");
        c.steps = {f, k};
        const QString text = c.text();
        QVERIFY(text.startsWith(QLatin1String("jo-data:2?")));
        DataConnection back;
        QVERIFY(DataConnection::parse(text, &back));
        QCOMPARE(back.steps, c.steps);
        QCOMPARE(back.name, c.name);
        QCOMPARE(back.ranAt, c.ranAt);
        QCOMPARE(back.rowCount, c.rowCount);
        QCOMPARE(back.path, c.path);
        QCOMPARE(back.text(), text);
        // through a table part
        Table t;
        t.name = QStringLiteral("T");
        t.row1 = 0;
        t.col1 = 0;
        t.row2 = 1;
        t.col2 = 0;
        t.columns = QStringList{QStringLiteral("a")};
        t.info.resize(1);
        t.info[0].id = 1;
        t.id = 1;
        t.dataSource = text;
        Table read;
        read.columns = t.columns;
        QVERIFY(readTableModel(writeTablePart(t, QByteArray()), &read));
        QCOMPARE(read.dataSource, text);
        // without steps, the text is the one of version 1, with no steps in it
        c.steps.clear();
        QVERIFY(c.text().startsWith(QLatin1String("jo-data:1?")));
        QVERIFY(!c.text().contains(QLatin1String("steps=")));
        // a step of a kind this version does not have: the query reads, can't run, and is written as it was
        const QString newer = QString(text).replace(QStringLiteral("%22filter%22"), QStringLiteral("%22pivot%22"));
        QVERIFY(newer != text);
        DataConnection future;
        QVERIFY(DataConnection::parse(newer, &future));
        QVERIFY(!future.stepsProblem.isEmpty());
        QVERIFY(future.stepsProblem.contains(QLatin1String("newer version")));
        QVERIFY(future.steps.isEmpty());
        QCOMPARE(future.text(), newer);
        // a later version of the text itself is not read at all (the table keeps the string)
        QVERIFY(!DataConnection::parse(QString(newer).replace(QStringLiteral("jo-data:2"), QStringLiteral("jo-data:3")), &future));
    }

    void aLongQueryTextIsKeptWholeAndNeverCut()
    {
        Table t;
        t.name = QStringLiteral("T");
        t.row2 = 1;
        t.columns = QStringList{QStringLiteral("a")};
        t.info.resize(1);
        t.info[0].id = 1;
        t.id = 1;
        t.dataSource = QStringLiteral("jo-data:2?kind=json&path=p&steps=") + QString(kMaxDataSourceChars - 40, QLatin1Char('a'));
        Table read;
        read.columns = t.columns;
        QVERIFY(readTableModel(writeTablePart(t, QByteArray()), &read));
        QCOMPARE(read.dataSource, t.dataSource);
        QVERIFY(read.dataSource.size() > 80000);
        // past the limit it is cut at the limit (a table made by the program refuses such a text before it is saved)
        t.dataSource += QString(1000, QLatin1Char('b'));
        QVERIFY(readTableModel(writeTablePart(t, QByteArray()), &read));
        QCOMPARE(read.dataSource.size(), kMaxDataSourceChars);
    }
};

JP_TEST_CLASS(SheetDataSourceTests)
#include "tst_sheet_datasource.moc"
