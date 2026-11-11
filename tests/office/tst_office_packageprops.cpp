// Tests of libs/office: the properties of a package (core, extended, and custom parts).
#include "officehelpers.h"
#include "office/packageprops.h"
#include "office/privacy.h"
#include "io/zip.h"
#include "io/zipindex.h"

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
        QVERIFY(!after.value(QStringLiteral("content.xml")).contains("dc:date"));   // (the time of a comment goes)
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

    // Only real attributes and elements are changed: text that looks like an attribute, in the body or in another attribute's value, is the person's words and stays; a name in CDATA or with
    // entities is found as the name it is.
    void removingPersonalInformationNeverPatchesBodyText()
    {
        const QByteArray body = "<w:document><w:p><w:r><w:t> w:date=\"2020\" w:author=\"X\" w:initials=\"Q\"</w:t></w:r><w:r><w:t w:note=' w:author=\"Y\"'>kept</w:t></w:r>"
                                "<w:ins w:id=\"1\" w:author=\"Ann Lee\" w:date=\"2026-01-02T03:04:05Z\"><w:r><w:t>new</w:t></w:r></w:ins></w:p></w:document>";
        jp::ZipWriter z;
        z.add(QStringLiteral("word/document.xml"), body, true);
        z.add(QStringLiteral("docProps/app.xml"), "<Properties><Company><![CDATA[Acme & Co]]></Company><Manager>Bo &amp; Ray</Manager><Words>3</Words></Properties>");
        z.add(QStringLiteral("docProps/core.xml"), "<cp:coreProperties><dc:creator>Ann &#x26; Bo</dc:creator><!-- <dc:creator>Secret</dc:creator> --></cp:coreProperties>");
        QMap<QString, QByteArray> after;
        QVERIFY(jp::readZip(removePersonalFromPackage(z.finish()), after));
        const QByteArray doc = after.value(QStringLiteral("word/document.xml"));
        QVERIFY2(doc.contains("<w:t> w:date=\"2020\" w:author=\"X\" w:initials=\"Q\"</w:t>"), doc.constData());
        QVERIFY2(doc.contains("<w:t w:note=' w:author=\"Y\"'>kept</w:t>"), doc.constData());
        QVERIFY2(doc.contains("<w:ins w:id=\"1\" w:author=\"Author\"><w:r>"), doc.constData());
        const QByteArray app = after.value(QStringLiteral("docProps/app.xml"));
        QVERIFY2(!app.contains("Acme") && !app.contains("Ray") && app.contains("<Words>3</Words>"), app.constData());
        const QByteArray core = after.value(QStringLiteral("docProps/core.xml"));
        QVERIFY2(!core.contains("Ann") && core.contains("<dc:creator></dc:creator>"), core.constData());
    }

    // A sheet's legacy note keeps the link to its thread (the author "tc={...}"), and its Excel-style name run goes with the name.
    void removingPersonalInformationKeepsThreadLinksAndTakesTheNameRunOutOfNotes()
    {
        const QByteArray comments =
            "<comments><authors><author>Ann &amp; Bo</author><author>tc={ABC-1}</author><author>Cy</author></authors><commentList>"
            "<comment ref=\"A1\" authorId=\"0\"><text><r><rPr><b/></rPr><t>Ann &amp; Bo:</t></r><r><t xml:space=\"preserve\">\nCheck this</t></r></text></comment>"
            "<comment ref=\"B1\" authorId=\"1\"><text><t>[Threaded comment]</t></text></comment>"
            "<comment ref=\"C1\" authorId=\"2\"><text><r><t>Someone:</t></r><r><t>stays</t></r></text></comment></commentList></comments>";
        jp::ZipWriter z;
        z.add(QStringLiteral("xl/comments1.xml"), comments, true);
        QMap<QString, QByteArray> after;
        QVERIFY(jp::readZip(removePersonalFromPackage(z.finish()), after));
        const QByteArray x = after.value(QStringLiteral("xl/comments1.xml"));
        QVERIFY2(x.contains("<author>tc={ABC-1}</author>"), x.constData());
        QVERIFY2(!x.contains("Ann &amp; Bo") && x.count("<author>Author</author>") == 2, x.constData());
        QVERIFY2(!x.contains("Ann &amp; Bo:") && x.contains("<comment ref=\"A1\" authorId=\"0\"><text><r><t xml:space=\"preserve\">\nCheck this</t></r></text></comment>"), x.constData());
        QVERIFY2(x.contains("<t>[Threaded comment]</t>") && x.contains("<r><t>Someone:</t></r><r><t>stays</t></r>"), x.constData());
    }

    // The same coverage in every kind of file: names out, and the times of comments and changes out.
    void removingPersonalInformationTakesTimesAndNamesOutOfEveryKindOfFile()
    {
        jp::ZipWriter z;
        z.add(QStringLiteral("xl/threadedComments/threadedComment1.xml"), "<ThreadedComments><threadedComment ref=\"A1\" dT=\"2026-01-02T03:04:05.00\" personId=\"{P}\" id=\"{I}\"><text>Hi</text></threadedComment></ThreadedComments>");
        z.add(QStringLiteral("xl/revisions/revisionHeaders.xml"), "<headers><header guid=\"{G}\" dateTime=\"2026-01-02T03:04:05Z\" maxSheetId=\"1\" userName=\"Ann Lee\" minRId=\"1\"/></headers>");
        z.add(QStringLiteral("xl/revisions/userNames.xml"), "<users count=\"1\"><userinfo guid=\"{G}\" name=\"Ann Lee\" id=\"1\" dateTime=\"2026-01-02T03:04:05Z\"/></users>");
        z.add(QStringLiteral("ppt/comments/comment1.xml"), "<p:cmLst><p:cm authorId=\"0\" dt=\"2026-01-02T03:04:05.000\" idx=\"1\"><p:text>Hi</p:text></p:cm></p:cmLst>");
        z.add(QStringLiteral("ppt/comments/modernComment_1.xml"), "<p188:cmLst><p188:cm id=\"{C}\" authorId=\"{A}\" created=\"2026-01-02T03:04:05.000\"><p188:txBody/></p188:cm></p188:cmLst>");
        z.add(QStringLiteral("ppt/authors.xml"), "<p188:authorLst><p188:author id=\"{A}\" name=\"Ann Lee\" initials=\"AL\" userId=\"ann@example.com\" providerId=\"AD\"/></p188:authorLst>");
        z.add(QStringLiteral("word/commentsExtensible.xml"), "<w16cex:commentsExtensible><w16cex:commentExtensible w16cex:paraId=\"1\" w16cex:dateUtc=\"2026-01-02T03:04:05Z\"/></w16cex:commentsExtensible>");
        z.add(QStringLiteral("meta.xml"), "<office:meta><meta:printed-by>Ann Lee</meta:printed-by><dc:date>2027-05-06T07:08:09</dc:date>"
                                        "<meta:user-defined meta:name=\"Company\">Acme</meta:user-defined><meta:user-defined meta:name=\"Manager\">Bo Ray</meta:user-defined><meta:user-defined meta:name=\"Project\">Apollo</meta:user-defined></office:meta>");
        z.add(QStringLiteral("content.xml"), "<office:text><office:annotation><dc:creator>Ann Lee</dc:creator><dc:date>2026-01-02T03:04:05</dc:date><text:p>Hi</text:p></office:annotation>"
                                            "<text:changed-region><office:change-info><dc:creator>Ann Lee</dc:creator><dc:date>2026-01-02T03:04:05</dc:date></office:change-info></text:changed-region></office:text>");
        QMap<QString, QByteArray> after;
        int parts = 0;
        QVERIFY(jp::readZip(removePersonalFromPackage(z.finish(), &parts), after));
        QCOMPARE(parts, 9);
        for (auto it = after.cbegin(); it != after.cend(); ++it) {
            const QByteArray &x = it.value();
            QVERIFY2(!x.contains("Ann Lee") && !x.contains("Acme") && !x.contains("Bo Ray") && !x.contains("ann@example.com") && !x.contains("2026-01-02T03:04:05"), qPrintable(it.key() + QStringLiteral(": ") + QString::fromUtf8(x)));
        }
        QVERIFY(!after.value(QStringLiteral("xl/threadedComments/threadedComment1.xml")).contains("dT="));
        QVERIFY(after.value(QStringLiteral("xl/threadedComments/threadedComment1.xml")).contains("personId=\"{P}\""));
        QVERIFY(after.value(QStringLiteral("meta.xml")).contains("<meta:user-defined meta:name=\"Project\">Apollo</meta:user-defined>"));   // (custom properties are left)
        QVERIFY(after.value(QStringLiteral("meta.xml")).contains("<dc:date>2027-05-06T07:08:09</dc:date>"));   // (the date the file was changed is the file's own: kept)
    }

    // What can't be done is said: a part that is not UTF-8 or can't be inflated, bytes that are not a package, and a hook that damages them. The file is still given back whole.
    void removingPersonalInformationSaysWhatItCouldNotDo()
    {
        QStringList problems;
        jp::ZipWriter z;
        z.add(QStringLiteral("docProps/core.xml"), QByteArray("\xFF\xFE<\0c\0", 7));
        const QByteArray utf16 = z.finish();
        QCOMPARE(removePersonalFromPackage(utf16, nullptr, &problems), utf16);
        QCOMPARE(problems.size(), 1);
        QVERIFY(problems[0].contains(QLatin1String("docProps/core.xml")));
        problems.clear();
        QCOMPARE(removePersonalFromPackage("not a zip", nullptr, &problems), QByteArray("not a zip"));
        QCOMPARE(problems.size(), 1);
        problems.clear();
        jp::ZipWriter y;
        y.add(QStringLiteral("docProps/app.xml"), "<Properties><Company>Acme</Company></Properties>", true);
        y.add(QStringLiteral("word/media/x.bin"), QByteArray(300, 'x'));
        const QByteArray fine = y.finish();
        setPackageScrubHookForTest([](const QByteArray &b) {
            QByteArray damaged = b;
            const qsizetype at = damaged.indexOf("docProps/app.xml") + 16;
            damaged[at + 2] = char(damaged[at + 2] ^ 0x55);   // (a byte of the part's own bytes)
            return damaged;
        });
        const QByteArray out = removePersonalFromPackage(fine, nullptr, &problems);
        setPackageScrubHookForTest({});
        QVERIFY(!problems.isEmpty());
        QVERIFY(!out.isEmpty());
        problems.clear();
        QVERIFY(!removePersonalFromPackage(fine, nullptr, &problems).isEmpty());
        QVERIFY(problems.isEmpty());
    }

    // The entries a scrub does not touch are copied bit for bit, in their order, with the first entry of an open document package stored.
    void removingPersonalInformationCopiesTheOtherEntriesBitForBit()
    {
        const QByteArray picture = [] {
            QByteArray b;
            for (int i = 0; i < 4000; ++i) b += char((i * 7) % 251);
            return b;
        }();
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), "application/vnd.oasis.opendocument.text");
        z.add(QStringLiteral("Pictures/a.png"), picture, true);
        z.add(QStringLiteral("meta.xml"), "<office:meta><dc:creator>Ann Lee</dc:creator></office:meta>", true);
        z.add(QStringLiteral("styles.xml"), "<office:styles/>", true);
        const QByteArray zip = z.finish();
        const QByteArray out = removePersonalFromPackage(zip);
        QVector<jp::ZipEntry> a, b;
        QVERIFY(jp::readZipDirectory(zip, &a));
        QVERIFY(jp::readZipDirectory(out, &b));
        QCOMPARE(b.size(), a.size());
        QCOMPARE(b[0].name, QStringLiteral("mimetype"));
        QCOMPARE(int(b[0].method), 0);
        for (int i = 0; i < a.size(); ++i) {
            QCOMPARE(b[i].name, a[i].name);
            if (a[i].name == QLatin1String("meta.xml")) continue;
            QCOMPARE(b[i].method, a[i].method);
            QCOMPARE(b[i].crc, a[i].crc);
            QCOMPARE(out.mid(int(b[i].dataAt), int(b[i].csize)), zip.mid(int(a[i].dataAt), int(a[i].csize)));
        }
    }

    // The ZIP writer copies an entry as it is: the long form for sizes of 4 GiB and more (a test stands in with the long records), and the other ZIP's flag bits kept but the data-descriptor bit.
    void aRawEntryKeepsItsFlagsAndCanBeInTheLongForm()
    {
        const QByteArray bytes("hello");
        jp::ZipWriter z;
        z.useLongRecords(true);
        z.addRaw(QStringLiteral("a.txt"), 0, jp::crc32(bytes), 5, 5, bytes.constData(), 0x0001 | 0x0006 | 0x0008);
        z.addRaw(QStringLiteral("b.txt"), 0, jp::crc32(bytes), 5, 5, bytes.constData());
        const QByteArray zip = z.finish();
        QVector<jp::ZipEntry> e;
        QVERIFY(jp::readZipDirectory(zip, &e));
        QCOMPARE(e.size(), 2);
        QCOMPARE(e[0].csize, qint64(5));
        QCOMPARE(e[0].usize, qint64(5));
        QCOMPARE(int(e[0].flags), 0x0001 | 0x0006 | 0x0800);
        QCOMPARE(int(e[1].flags), 0x0800);
        QCOMPARE(zip.mid(int(e[0].dataAt), 5), bytes);
    }
};

JP_TEST_CLASS(OfficePackagePropsTests)
#include "tst_office_packageprops.moc"
