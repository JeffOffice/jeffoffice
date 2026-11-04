// Tests of libs/office: the properties of a package (core, extended, and custom parts).
#include "officehelpers.h"
#include "office/packageprops.h"

#include <QTimeZone>
#include <QXmlStreamReader>

using namespace jp::office;

namespace {

// The text of the first element called `local` (anywhere), "" if none.
QString textOf(const QByteArray &xml, const QString &local)
{
    QXmlStreamReader r(xml);
    while (!r.atEnd())
        if (r.readNext() == QXmlStreamReader::StartElement && r.name() == local) return r.readElementText();
    return {};
}

const char *kCore =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
    "xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
    "<dc:title>Old</dc:title><dc:creator>Ann</dc:creator><cp:contentStatus>Draft</cp:contentStatus>"
    "<dcterms:created xsi:type=\"dcterms:W3CDTF\">2026-01-02T03:04:05Z</dcterms:created></cp:coreProperties>";

} // namespace

class OfficePackagePropsTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void readsTheThreeParts()
    {
        const QByteArray app = "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\"><Company>Acme</Company><Words>12</Words><Pages>2</Pages></Properties>";
        const QByteArray custom = "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">"
                                  "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"2\" name=\"Project\"><vt:lpwstr>Apollo</vt:lpwstr></property>"
                                  "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"3\" name=\"_MarkAsFinal\"><vt:bool>true</vt:bool></property></Properties>";
        const PackageProps p = readPackageProps(kCore, app, custom);
        QCOMPARE(p.title, QStringLiteral("Old"));
        QCOMPARE(p.creator, QStringLiteral("Ann"));
        QCOMPARE(p.created, QDateTime(QDate(2026, 1, 2), QTime(3, 4, 5), QTimeZone::UTC));
        QCOMPARE(p.company, QStringLiteral("Acme"));
        QCOMPARE(p.words, 12);
        QCOMPARE(p.pages, 2);
        QCOMPARE(p.lines, -1);
        QCOMPARE(p.custom.size(), 2);
        QCOMPARE(p.custom[0].value, QStringLiteral("Apollo"));
        QVERIFY(isMarkedFinal(p.custom));
    }
    void writingKeepsWhatItDoesNotManage()
    {
        PackageProps p = readPackageProps(kCore, {}, {});
        p.title = QStringLiteral("New & <better>");
        p.keywords = QStringLiteral("a, b");
        p.creator.clear();
        const QByteArray out = writeCoreProps(kCore, p);
        QCOMPARE(textOf(out, QStringLiteral("title")), QStringLiteral("New & <better>"));
        QCOMPARE(textOf(out, QStringLiteral("keywords")), QStringLiteral("a, b"));
        QCOMPARE(textOf(out, QStringLiteral("contentStatus")), QStringLiteral("Draft"));
        QVERIFY(textOf(out, QStringLiteral("creator")).isEmpty());
        const PackageProps back = readPackageProps(out, {}, {});
        QCOMPARE(back.created, p.created);
        QCOMPARE(back.title, p.title);
        // (the namespaces stay declared once, as the part had them)
        QCOMPARE(out.count("xmlns:dc="), 1);
    }
    void makesAPartFromNothing()
    {
        PackageProps p;
        p.title = QStringLiteral("T");
        p.modified = QDateTime(QDate(2026, 3, 4), QTime(5, 6, 7), QTimeZone::UTC);
        p.company = QStringLiteral("Co");
        p.words = 5;
        const QByteArray core = writeCoreProps({}, p), app = writeAppProps({}, p);
        const PackageProps back = readPackageProps(core, app, {});
        QCOMPARE(back.title, QStringLiteral("T"));
        QCOMPARE(back.modified, p.modified);
        QCOMPARE(back.company, QStringLiteral("Co"));
        QCOMPARE(back.words, 5);
        QCOMPARE(back.pages, -1);
        QVERIFY(core.contains("coreProperties"));
        QVERIFY(core.contains("2026-03-04T05:06:07Z"));
    }
    void aCountOfMinusOneIsLeftAlone()
    {
        const QByteArray app = "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\"><Application>JeffCrunch</Application><Words>12</Words><Lines>9</Lines></Properties>";
        PackageProps p;
        p.words = 40;
        p.company = QStringLiteral("Acme");
        const QByteArray out = writeAppProps(app, p);
        QCOMPARE(textOf(out, QStringLiteral("Words")), QStringLiteral("40"));
        QCOMPARE(textOf(out, QStringLiteral("Lines")), QStringLiteral("9"));
        QCOMPARE(textOf(out, QStringLiteral("Application")), QStringLiteral("JeffCrunch"));
        QCOMPARE(textOf(out, QStringLiteral("Company")), QStringLiteral("Acme"));
    }
    void customPropertiesRoundTrip()
    {
        PackageProps p;
        p.custom = {{QStringLiteral("Project"), QStringLiteral("lpwstr"), QStringLiteral("Apollo <1>")}, {QStringLiteral("Count"), QStringLiteral("i4"), QStringLiteral("7")},
                    {QStringLiteral("Project"), QStringLiteral("lpwstr"), QStringLiteral("again")}, {QString(), QStringLiteral("lpwstr"), QStringLiteral("no name")},
                    {QStringLiteral("Odd"), QStringLiteral("blob"), QStringLiteral("x")}, {QStringLiteral("When"), QStringLiteral("filetime"), QStringLiteral("2026-01-01T00:00:00Z")}};
        const QByteArray xml = writeCustomProps(p);
        const PackageProps back = readPackageProps({}, {}, xml);
        QCOMPARE(back.custom.size(), 3);
        QCOMPARE(back.custom[0].value, QStringLiteral("Apollo <1>"));
        QCOMPARE(back.custom[1].type, QStringLiteral("i4"));
        QCOMPARE(back.custom[2].name, QStringLiteral("When"));
        QVERIFY(xml.contains("pid=\"4\""));
    }
    void markAsFinal()
    {
        QVector<CustomProperty> c = {{QStringLiteral("A"), QStringLiteral("lpwstr"), QStringLiteral("x")}};
        QVERIFY(!isMarkedFinal(c));
        c = withMarkAsFinal(c, true);
        QVERIFY(isMarkedFinal(c));
        QCOMPARE(c.size(), 2);
        c = withMarkAsFinal(withMarkAsFinal(c, true), false);
        QVERIFY(!isMarkedFinal(c));
        QCOMPARE(c.size(), 1);
    }
    void damagedPartsAreRemade()
    {
        PackageProps p;
        p.title = QStringLiteral("T");
        const QByteArray out = writeCoreProps("<not xml", p);
        QCOMPARE(textOf(out, QStringLiteral("title")), QStringLiteral("T"));
        const QByteArray other = writeCoreProps("<other xmlns=\"urn:x\"><a/></other>", p);
        QVERIFY(other.contains("coreProperties"));
    }
};

JP_TEST_CLASS(OfficePackagePropsTests)
#include "tst_office_packageprops.moc"
