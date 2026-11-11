// Tests of libs/office: the properties of a package (core, extended, and custom parts).
#include "officehelpers.h"
#include "office/packageprops.h"
#include "office/privacy.h"
#include "io/zip.h"

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
        const QByteArray xml = writeCustomProps({}, p);
        const PackageProps back = readPackageProps({}, {}, xml);
        QCOMPARE(back.custom.size(), 3);
        QCOMPARE(back.custom[0].value, QStringLiteral("Apollo <1>"));
        QCOMPARE(back.custom[1].type, QStringLiteral("i4"));
        QCOMPARE(back.custom[2].name, QStringLiteral("When"));
        QVERIFY(xml.contains("pid=\"4\""));
    }

    // A part with known, unknown, and vector properties, and one after them all: every one is read, and what is not modeled is written back as it was, with its place and format id.
    void propertiesNotModeledAreKeptAsTheyWere()
    {
        const QByteArray custom =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n"
            "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\" xmlns:x=\"urn:x\">"
            "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"2\" name=\"Project\"><vt:lpwstr>Apollo &amp; Co caf\xC3\xA9</vt:lpwstr></property>\n"
            "<property fmtid=\"{AAAAAAAA-2E9C-101B-9397-08002B2CF9AE}\" pid=\"7\" name=\"Sizes\"><vt:vector size=\"2\" baseType=\"variant\"><vt:variant><vt:i4>1</vt:i4></vt:variant><vt:variant><vt:lpstr>two</vt:lpstr></vt:variant></vt:vector></property>"
            "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"8\" name=\"Big\"><vt:i8>9007199254740993</vt:i8></property>"
            "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"9\" name=\"Money\"><vt:cy>12.5</vt:cy></property>"
            "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"10\" name=\"Made\"><vt:date>2026-01-01T00:00:00Z</vt:date></property>"
            "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"11\" name=\"Count\"><vt:i4>7</vt:i4></property>"
            "</Properties>";
        const PackageProps p = readPackageProps({}, {}, custom);
        QVERIFY(!p.truncated);
        QCOMPARE(p.custom.size(), 6);   // (nothing after the vector is lost)
        QCOMPARE(p.custom[0].value, QString::fromUtf8("Apollo & Co caf\xC3\xA9"));
        QCOMPARE(p.custom[0].pid, 2);
        QVERIFY(!p.custom[1].raw.isEmpty());
        QVERIFY(p.custom[1].raw.contains("<vt:variant><vt:i4>1</vt:i4></vt:variant>"));
        QCOMPARE(p.custom[1].pid, 7);
        QCOMPARE(p.custom[1].fmtid, QStringLiteral("{AAAAAAAA-2E9C-101B-9397-08002B2CF9AE}"));
        QVERIFY(!p.custom[2].raw.isEmpty());   // i8, cy, and date are not modeled
        QVERIFY(!p.custom[3].raw.isEmpty());
        QCOMPARE(p.custom[5].value, QStringLiteral("7"));
        QCOMPARE(p.custom[5].pid, 11);
        // written back: every property not modeled is there byte for byte, the others with the same place and format id, and the root as it was
        const QByteArray out = writeCustomProps(custom, p);
        for (int i = 1; i <= 4; ++i) QVERIFY2(out.contains(p.custom[i].raw), qPrintable(QString::number(i)));
        QVERIFY(out.startsWith(custom.left(custom.indexOf("<property"))));
        const PackageProps back = readPackageProps({}, {}, out);
        QVERIFY2(back.custom.size() == p.custom.size(), out.constData());
        QCOMPARE(back.custom, p.custom);
        // a new property takes a place above all the file uses
        PackageProps more = p;
        more.custom.push_back({QStringLiteral("New"), QStringLiteral("lpwstr"), QStringLiteral("n"), 0, QString(), QByteArray()});
        QVERIFY(writeCustomProps(custom, more).contains("pid=\"12\" name=\"New\""));
    }

    // A part with more in it than is read is left as it is, and says so.
    void aPartPastTheCapsIsLeftAlone()
    {
        QByteArray core = "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\"><dc:title>Old</dc:title>";
        for (int i = 0; i < 120000; ++i) core += "<cp:x/>";
        core += "</cp:coreProperties>";
        PackageProps p = readPackageProps(core, {}, {});
        QVERIFY(p.truncated);
        p.title = QStringLiteral("New");
        QCOMPARE(writeCoreProps(core, p), core);
        QCOMPARE(writeAppProps(core, p), core);
        QByteArray custom = "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">";
        for (int i = 0; i < 5100; ++i) custom += "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"" + QByteArray::number(i + 2) + "\" name=\"p" + QByteArray::number(i) + "\"><vt:i4>1</vt:i4></property>";
        custom += "</Properties>";
        const PackageProps c = readPackageProps({}, {}, custom);
        QVERIFY(c.truncated);
        QCOMPARE(c.custom.size(), 5000);
        QCOMPARE(writeCustomProps(custom, c), custom);
    }

    // A part that breaks part way keeps what it had before, and the elements this program does not manage are kept.
    void aBrokenPartKeepsWhatItCouldBeRead()
    {
        const QByteArray core = "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\">"
                                "<dc:language>en-US</dc:language><cp:contentStatus>Draft</cp:contentStatus><dc:title>Old</dc:title><dc:cre";
        PackageProps p;
        p.title = QStringLiteral("New");
        const QByteArray out = writeCoreProps(core, p);
        QCOMPARE(textOf(out, QStringLiteral("language")), QStringLiteral("en-US"));
        QCOMPARE(textOf(out, QStringLiteral("contentStatus")), QStringLiteral("Draft"));
        QCOMPARE(textOf(out, QStringLiteral("title")), QStringLiteral("New"));
        QXmlStreamReader r(out);
        while (!r.atEnd()) r.readNext();
        QVERIFY2(!r.hasError(), qPrintable(r.errorString()));
        // the extended part keeps its lists of headings and titles
        const QByteArray app = "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">"
                               "<HeadingPairs><vt:vector size=\"2\" baseType=\"variant\"><vt:variant><vt:lpstr>Sheets</vt:lpstr></vt:variant><vt:variant><vt:i4>1</vt:i4></vt:variant></vt:vector></HeadingPairs>"
                               "<TitlesOfParts><vt:vector size=\"1\" baseType=\"lpstr\"><vt:lpstr>Sheet1</vt:lpstr></vt:vector></TitlesOfParts><Company>Old</Company><Wor";
        p.company = QStringLiteral("Acme");
        const QByteArray appOut = writeAppProps(app, p);
        QVERIFY(appOut.contains("<HeadingPairs>"));
        QVERIFY(appOut.contains("<TitlesOfParts>"));
        QVERIFY(appOut.contains("Sheet1"));
        QCOMPARE(textOf(appOut, QStringLiteral("Company")), QStringLiteral("Acme"));
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

    // The names, initials, and times of changes come out of the parts that hold them, in each kind of package; every other entry is copied as it is, and what is not a ZIP comes back as it was.
    void removingPersonalInformationTakesNamesOutOfTheKnownPartsAndCopiesTheRest()
    {
        const QByteArray picture(5000, 'x');
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), "application/vnd.oasis.opendocument.text");
        z.add(QStringLiteral("word/document.xml"), "<w:ins w:id=\"1\" w:author=\"Ann Lee\" w:date=\"2026-01-02T03:04:05Z\"><w:t>Ann Lee wrote this</w:t></w:ins>", true);
        z.add(QStringLiteral("word/comments.xml"), "<w:comment w:id=\"0\" w:author=\"Ann Lee\" w:initials=\"AL\"><w:t>Hi</w:t></w:comment>", true);
        z.add(QStringLiteral("word/people.xml"), "<w15:people><w15:person w15:author=\"Ann Lee\"><w15:presenceInfo w15:providerId=\"AD\" w15:userId=\"S-1\"/></w15:person></w15:people>");
        z.add(QStringLiteral("ppt/commentAuthors.xml"), "<p:cmAuthorLst><p:cmAuthor id=\"0\" name=\"Ann Lee\" initials=\"AL\" lastIdx=\"1\" clrIdx=\"0\"/></p:cmAuthorLst>");
        z.add(QStringLiteral("xl/comments1.xml"), "<comments><authors><author>Ann Lee</author></authors></comments>");
        z.add(QStringLiteral("xl/persons/person.xml"), "<personList><person displayName=\"Ann Lee\" id=\"{1}\" userId=\"ann@example.com\" providerId=\"AD\"/></personList>");
        z.add(QStringLiteral("xl/worksheets/sheet1.xml"), "<worksheet><scenarios><scenario name=\"Best\" locked=\"1\" user=\"Ann Lee\" count=\"1\"/></scenarios></worksheet>");
        z.add(QStringLiteral("docProps/core.xml"), "<cp:coreProperties><dc:title>Kept</dc:title><dc:creator>Ann Lee</dc:creator><cp:lastModifiedBy>Ann Lee</cp:lastModifiedBy></cp:coreProperties>");
        z.add(QStringLiteral("docProps/app.xml"), "<Properties><Company>Acme</Company><Manager>Bo Ray</Manager><Words>3</Words></Properties>");
        z.add(QStringLiteral("meta.xml"), "<office:meta><meta:initial-creator>Ann Lee</meta:initial-creator><dc:creator>Ann Lee</dc:creator><dc:title>Kept</dc:title></office:meta>");
        z.add(QStringLiteral("content.xml"), "<office:annotation><dc:creator>Ann Lee</dc:creator><meta:creator-initials>AL</meta:creator-initials><dc:date>2026-01-02</dc:date></office:annotation>");
        z.add(QStringLiteral("word/media/image1.png"), picture, true);
        const QByteArray zip = z.finish();
        int parts = 0;
        const QByteArray clean = removePersonalFromPackage(zip, &parts);
        QCOMPARE(parts, 11);
        QMap<QString, QByteArray> before, after;
        QVERIFY(jp::readZip(zip, before));
        QVERIFY(jp::readZip(clean, after));
        QCOMPARE(after.keys(), before.keys());
        for (auto it = after.cbegin(); it != after.cend(); ++it) {
            QVERIFY2(!it.value().contains("Ann Lee") || it.key() == QLatin1String("word/document.xml"), qPrintable(it.key()));   // (the words a person typed are not names to take out)
            QVERIFY2(!it.value().contains("Acme") && !it.value().contains("Bo Ray") && !it.value().contains("ann@example.com") && !it.value().contains("S-1"), qPrintable(it.key()));
        }
        QVERIFY(!after.value(QStringLiteral("word/document.xml")).contains("w:author=\"Ann Lee\"") && !after.value(QStringLiteral("word/document.xml")).contains("w:date="));
        QVERIFY(after.value(QStringLiteral("word/document.xml")).contains("w:author=\"Author\""));
        QVERIFY(after.value(QStringLiteral("word/document.xml")).contains("<w:t>Ann Lee wrote this</w:t>"));
        QVERIFY(!after.value(QStringLiteral("word/comments.xml")).contains("w:initials"));
        QVERIFY(!after.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("user=") && after.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("locked=\"1\" count=\"1\""));
        QVERIFY(after.value(QStringLiteral("docProps/core.xml")).contains("<dc:title>Kept</dc:title>"));
        QVERIFY(after.value(QStringLiteral("docProps/app.xml")).contains("<Words>3</Words>"));
        QVERIFY(after.value(QStringLiteral("meta.xml")).contains("<dc:title>Kept</dc:title>"));
        QVERIFY(after.value(QStringLiteral("content.xml")).contains("<dc:date>2026-01-02</dc:date>"));
        QCOMPARE(after.value(QStringLiteral("word/media/image1.png")), picture);
        QCOMPARE(after.value(QStringLiteral("mimetype")), before.value(QStringLiteral("mimetype")));
        // A package with nothing to take out is given back as it was, and so is what is not a ZIP.
        jp::ZipWriter plain;
        plain.add(QStringLiteral("a.xml"), "<a/>", true);
        const QByteArray plainZip = plain.finish();
        QCOMPARE(removePersonalFromPackage(plainZip, &parts), plainZip);
        QCOMPARE(parts, 0);
        QCOMPARE(removePersonalFromPackage("not a zip", &parts), QByteArray("not a zip"));
        PackageProps names;
        names.creator = QStringLiteral("Ann");
        names.company = QStringLiteral("Acme");
        names.title = QStringLiteral("T");
        QCOMPARE(removePersonalNames(names), 2);
        QVERIFY(names.creator.isEmpty() && names.company.isEmpty());
        QCOMPARE(names.title, QStringLiteral("T"));
    }
};

JP_TEST_CLASS(OfficePackagePropsTests)
#include "tst_office_packageprops.moc"
