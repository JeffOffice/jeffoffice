// Tests of the readers of JSON and XML data for Get Data: the lists a file offers, the columns and rows read, and the limits on files made to do harm. Every file is made here.
#include "sheet/datasource.h"

#include "../testrunner.h"

#include <QtTest>

using namespace jp::office::sheet;

class SheetDataSourceTests : public QObject {
    Q_OBJECT

    static QString cell(const DataTable &t, int row, const QString &column)
    {
        const int c = t.header.indexOf(column);
        return c < 0 ? QStringLiteral("<no column>") : t.rows.value(row).value(c).text;
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

    void jsonObjectsBecomeRowsAndFlattenedColumns()
    {
        const QByteArray json = "[{\"id\":1,\"name\":\"Ann\",\"ok\":true,\"addr\":{\"city\":\"Oslo\",\"geo\":{\"lat\":59.9}},\"tags\":[\"x\",\"y\"]},"
                                "{\"id\":2,\"name\":null,\"extra\":\"late\",\"tags\":[]}]";
        DataTable t;
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
        DataTable cols;
        QVERIFY(dataReadJson(json, QStringLiteral("$"), ArrayMode::Columns, 0, DataLimits(), &cols, &why));
        QCOMPARE(cell(cols, 0, QStringLiteral("tags.2")), QStringLiteral("y"));
    }

    void jsonDeepObjectsAreKeptAsTextAndSaid()
    {
        DataLimits limits;
        limits.maxDepth = 2;
        DataTable t;
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
        DataTable t;
        QString why;
        QVERIFY(dataReadJson(json, QStringLiteral("$"), ArrayMode::Join, 0, limits, &t, &why));
        QCOMPARE(t.rows.size(), 3);
        QCOMPARE(t.header.size(), 2);
        QCOMPARE(cell(t, 0, QStringLiteral("a")), QStringLiteral("abcd"));
        QCOMPARE(t.notes.size(), 3);
        DataTable preview;
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
        DataTable t;
        QVERIFY2(dataReadXml(xml, QStringLiteral("/feed/entry"), 0, DataLimits(), &t, &why), qPrintable(why));
        QCOMPARE(t.rows.size(), 2);
        QCOMPARE(cell(t, 0, QStringLiteral("id")), QStringLiteral("1"));
        QCOMPARE(cell(t, 1, QStringLiteral("name")), QStringLiteral("Bob"));
        QCOMPARE(cell(t, 0, QStringLiteral("geo.lat")), QStringLiteral("5"));
        QCOMPARE(cell(t, 1, QStringLiteral("note")), QStringLiteral("hi"));
        QCOMPARE(cell(t, 0, QStringLiteral("note")), QString());
        DataTable preview;
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
        DataTable t;
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
};

JP_TEST_CLASS(SheetDataSourceTests)
#include "tst_sheet_datasource.moc"
