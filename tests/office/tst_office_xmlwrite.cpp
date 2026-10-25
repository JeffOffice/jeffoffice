// Tests of libs/office: an element written out as XML.
#include "officehelpers.h"

class OfficeXmlWriteTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The DOM keeps an element's attributes in a hash, whose order is new in each process; what is written has them in a fixed order (namespace
    // declarations first, then by name), whatever order the file had.
    void attributesAreWrittenInNameOrder()
    {
        const QString root = QStringLiteral("<a:t xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ");
        const QString shuffled = root + QStringLiteral("zeta=\"1\" mu=\"2\" r:id=\"rId1\" alpha=\"3\" omega=\"4\" kappa=\"5\" beta=\"6\" theta=\"7\"><a:c y=\"1\" x=\"2\" w=\"3\"/></a:t>");
        QDomDocument dom;
        QVERIFY(jp::office::parseXml(shuffled.toUtf8(), &dom));
        const QString out = jp::office::standaloneXml(dom.documentElement());
        QCOMPARE(out, QStringLiteral("<a:t xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                                      "alpha=\"3\" beta=\"6\" kappa=\"5\" mu=\"2\" omega=\"4\" r:id=\"rId1\" theta=\"7\" zeta=\"1\"><a:c w=\"3\" x=\"2\" y=\"1\"/></a:t>"));
    }

    // What is written reads back as the same tree, with its text and escapes.
    void whatIsWrittenReadsBack()
    {
        const QString text = QStringLiteral("<p:a xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" q=\"x &amp; &quot;y&quot;\" b=\"1\"><p:b>1 &lt; 2 &amp; 3</p:b></p:a>");
        QDomDocument first, second;
        QVERIFY(jp::office::parseXml(text.toUtf8(), &first));
        const QString out = jp::office::standaloneXml(first.documentElement());
        QVERIFY(jp::office::parseXml(out.toUtf8(), &second));
        QCOMPARE(second.documentElement().attribute(QStringLiteral("q")), QStringLiteral("x & \"y\""));
        QCOMPARE(second.documentElement().firstChildElement().text(), QStringLiteral("1 < 2 & 3"));
        QCOMPARE(jp::office::standaloneXml(second.documentElement()), out);
    }
};

JP_TEST_CLASS(OfficeXmlWriteTests)
#include "tst_office_xmlwrite.moc"
