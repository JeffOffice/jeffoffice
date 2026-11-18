// Pack and Go, the part every app shares (libs/shell/app/packandgo): the fonts that may travel, the links found in a package and moved inside it, the bounds on hostile links, and the caps.
#include "app/packandgo.h"
#include "core/fonts.h"
#include "core/sfnt.h"
#include "io/zip.h"
#include "../testrunner.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

using namespace jp;

namespace {

QByteArray bytesOf(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

QString makeFile(const QString &path, const QByteArray &data = QByteArray("data"))
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) f.write(data);
    return path;
}

QString xmlText(const QString &s) { return QString(s).replace(QLatin1Char('&'), QLatin1String("&amp;")); }

// A small package: a main part with fields, and external relationships to `targets` (pictures).
QByteArray makePackage(const QStringList &targets, const QStringList &fieldFiles = {})
{
    ZipWriter z;
    z.add(QStringLiteral("[Content_Types].xml"), "<?xml version=\"1.0\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"><Default Extension=\"xml\" ContentType=\"application/xml\"/></Types>");
    z.add(QStringLiteral("_rels/.rels"), "<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/></Relationships>");
    QString body = QStringLiteral("<w:document xmlns:w=\"w\"><w:body><w:p><w:r><w:rPr><w:rFonts w:ascii=\"Georgia\" w:hAnsi=\"Georgia\"/></w:rPr><w:t>x</w:t></w:r>");
    for (const QString &f : fieldFiles) {
        QString path = f;
        path.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
        body += QStringLiteral("<w:r><w:instrText xml:space=\"preserve\"> INCLUDEPICTURE \"%1\" \\d </w:instrText></w:r>").arg(xmlText(path));
    }
    body += QStringLiteral("</w:p></w:body></w:document>");
    z.add(QStringLiteral("word/document.xml"), body.toUtf8());
    QString rels = QStringLiteral("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">");
    int n = 1;
    for (const QString &t : targets)
        rels += QStringLiteral("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"%2\" TargetMode=\"External\"/>").arg(n++).arg(xmlText(t));
    rels += QStringLiteral("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"https://example.com/\" TargetMode=\"External\"/></Relationships>").arg(n);
    z.add(QStringLiteral("word/_rels/document.xml.rels"), rels.toUtf8());
    return z.finish();
}

class Source : public OpcPackSource {
public:
    Source(const QByteArray &package, const QString &path = QString()) : m_package(package), m_path(path) {}
    QString documentName() const override { return QStringLiteral("Report.docx"); }
    QString documentPath() const override { return m_path; }
    QByteArray savedBytes(QString *) override { return m_package; }

private:
    QByteArray m_package;
    QString m_path;
};

QMap<QString, QByteArray> entriesOf(const QString &zip)
{
    QMap<QString, QByteArray> out;
    readZip(bytesOf(zip), out);
    return out;
}

// A copy of a bundled font's file under another name with its embedding permissions set.
QString fontCopy(const QString &dir, const QString &name, quint16 fsType)
{
    QString source;
    for (const QString &d : bundledFontDirs())
        for (const QFileInfo &fi : QDir(d).entryInfoList({QStringLiteral("*.ttf")}, QDir::Files))
            if (source.isEmpty()) source = fi.absoluteFilePath();
    QByteArray font = bytesOf(source);
    const quint16 tables = quint16((uchar(font[4]) << 8) | uchar(font[5]));
    for (int i = 0; i < tables; ++i) {
        const int at = 12 + 16 * i;
        if (font.mid(at, 4) == "OS/2") {
            const quint32 offset = (quint32(uchar(font[at + 8])) << 24) | (quint32(uchar(font[at + 9])) << 16) | (quint32(uchar(font[at + 10])) << 8) | quint32(uchar(font[at + 11]));
            font[int(offset) + 8] = char(fsType >> 8);
            font[int(offset) + 9] = char(fsType & 0xFF);
        }
    }
    return makeFile(QDir(dir).filePath(name), font);
}

} // namespace

class ShellPackAndGoTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // The files a package links to are copied into it and the links, in the relationships and in the fields, point at the copies; what was not a file stays as it was.
    void linkedFilesComeInsideAndTheLinksFollow()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString a = makeFile(dir.filePath(QStringLiteral("outside/a.png")), "AAAA");
        const QString b = makeFile(dir.filePath(QStringLiteral("work/pics/b.png")), "BBBB");
        const QString c = makeFile(dir.filePath(QStringLiteral("outside/c d.jpg")), "CCCC");
        Source source(makePackage({a, QStringLiteral("pics/b.png")}, {c}), dir.filePath(QStringLiteral("work/Report.docx")));
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(r.packed(), 3);
        const auto entries = entriesOf(dir.filePath(QStringLiteral("out.zip")));
        QVERIFY(entries.contains(QStringLiteral("Report.docx")));
        QCOMPARE(entries.value(QStringLiteral("linked/a.png")), QByteArray("AAAA"));
        QCOMPARE(entries.value(QStringLiteral("linked/b.png")), QByteArray("BBBB"));
        QCOMPARE(entries.value(QStringLiteral("linked/c_d.jpg")), QByteArray("CCCC"));
        // opened from the package: every link is inside it and its file is there
        QString error;
        const QVector<PackLink> links = OpcPackSource::linksOf(entries.value(QStringLiteral("Report.docx")), &error);
        QCOMPARE(links.size(), 3);
        for (const PackLink &l : links) {
            QVERIFY2(l.target.startsWith(QLatin1String("linked/")), qPrintable(l.target));
            QVERIFY2(entries.contains(l.target), qPrintable(l.target));
        }
        QVERIFY(!entries.value(QStringLiteral("Report.docx")).contains(dir.path().toUtf8()));
        // (the web address was left alone)
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(entries.value(QStringLiteral("Report.docx")), parts));
        QVERIFY(parts.value(QStringLiteral("word/_rels/document.xml.rels")).contains("https://example.com/"));
        QVERIFY(parts.value(QStringLiteral("word/document.xml")).contains("INCLUDEPICTURE \"linked/c_d.jpg\" \\d"));
    }

    // The same package in a folder: the document, its links, and the files, side by side.
    void aFolderHoldsTheSameAndOpensFromWhereItIs()
    {
        QTemporaryDir dir;
        const QString a = makeFile(dir.filePath(QStringLiteral("elsewhere/a.png")), "AAAA");
        Source source(makePackage({a}));
        QDir().mkpath(dir.filePath(QStringLiteral("dest")));
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("dest")), PackMode::Folder);
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(QFileInfo(r.path).fileName(), QStringLiteral("Report"));
        QVERIFY(QFileInfo(r.path + QStringLiteral("/Report.docx")).isFile());
        QCOMPARE(bytesOf(r.path + QStringLiteral("/linked/a.png")), QByteArray("AAAA"));
        const QVector<PackLink> links = OpcPackSource::linksOf(bytesOf(r.path + QStringLiteral("/Report.docx")));
        QCOMPARE(links.size(), 1);
        QVERIFY(QFileInfo(r.path + QLatin1Char('/') + links[0].target).isFile());
        // a second time makes a second folder and never writes over the first
        const PackReport again = packAndGo(source, dir.filePath(QStringLiteral("dest")), PackMode::Folder);
        QVERIFY(again.ok);
        QVERIFY(again.path != r.path);
    }

    // A link that climbs out of the folder, an absolute link to something that is not a kind of file Pack and Go takes, a file on another computer, a link to the document itself, and a
    // program are refused and named; nothing of them is in the package, and every name in it is inside it.
    void hostileLinksAreRefused()
    {
        QTemporaryDir dir;
        const QString outside = makeFile(dir.filePath(QStringLiteral("outside/secret.png")), "SECRET");
        const QString plain = makeFile(dir.filePath(QStringLiteral("outside/passwd")), "root:x");
        const QString program = makeFile(dir.filePath(QStringLiteral("outside/run.exe")), "MZ");
        const QString self = makeFile(dir.filePath(QStringLiteral("work/Report.docx")), "doc");
        Q_UNUSED(outside);
        const QStringList targets = {QStringLiteral("../../etc/x"), QStringLiteral("../outside/secret.png"), QStringLiteral("pics/../../outside/secret.png"), plain, program,
                                     QStringLiteral("\\\\server\\share\\a.png"), QStringLiteral("//server/share/a.png"), QStringLiteral("Report.docx"), self, QStringLiteral("/etc/passwd"),
                                     QStringLiteral("file:///etc/shadow"), QStringLiteral("CON.png"), QStringLiteral("nothere.png")};
        Source source(makePackage(targets), self);
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(r.packed(), 0);
        QCOMPARE(r.items.size(), targets.size());
        for (const PackItem &i : r.items) QVERIFY2(i.status != PackItem::Packed && !i.why.isEmpty(), qPrintable(i.target));
        QCOMPARE(r.items[0].status, PackItem::Refused);   // ../../etc/x
        QCOMPARE(r.items[1].status, PackItem::Refused);   // the picture outside the folder
        QCOMPARE(r.items[7].why, QStringLiteral("The link is to the document itself."));
        QVERIFY(r.text().contains(QStringLiteral("../../etc/x")));
        const auto entries = entriesOf(dir.filePath(QStringLiteral("out.zip")));
        QCOMPARE(entries.size(), 1);   // the document, and nothing else
        for (auto it = entries.cbegin(); it != entries.cend(); ++it) {
            QVERIFY2(!it.key().contains(QLatin1String("..")) && !it.key().startsWith(QLatin1Char('/')) && !it.key().contains(QLatin1Char('\\')) && !it.key().contains(QLatin1Char(':')), qPrintable(it.key()));
            QVERIFY(!it.value().contains("SECRET") && !it.value().contains("root:x"));
        }
    }

    // A file given by its whole path is the usual link and is packed; one that comes to a name a link may not (a symbolic link to a file that is not a picture) is not.
    void anAbsolutePictureIsPackedButALinkToAnotherKindIsNot()
    {
        QTemporaryDir dir;
        const QString pic = makeFile(dir.filePath(QStringLiteral("p/pic.png")), "PIC");
        const QString secret = makeFile(dir.filePath(QStringLiteral("p/secret.txt")), "SECRET");
        const QString viaLink = dir.filePath(QStringLiteral("p/looks.png"));
        QVERIFY(QFile::link(secret, viaLink));
        Source source(makePackage({pic, viaLink}));
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY(r.ok);
        QCOMPARE(r.packed(), 1);
        QCOMPARE(r.items[0].status, PackItem::Packed);
        QCOMPARE(r.items[1].status, PackItem::Refused);
        QVERIFY(!entriesOf(dir.filePath(QStringLiteral("out.zip"))).contains(QStringLiteral("linked/looks.png")));
    }

    // Two files with one name get two names in the package; one file linked twice is there once.
    void namesAreUniqueAndAFileIsPackedOnce()
    {
        QTemporaryDir dir;
        const QString one = makeFile(dir.filePath(QStringLiteral("x/same.png")), "ONE");
        const QString two = makeFile(dir.filePath(QStringLiteral("y/same.png")), "TWO");
        Source source(makePackage({one, two, dir.filePath(QStringLiteral("x/../x/same.png"))}));
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY(r.ok);
        const auto entries = entriesOf(dir.filePath(QStringLiteral("out.zip")));
        QCOMPARE(entries.value(QStringLiteral("linked/same.png")), QByteArray("ONE"));
        QCOMPARE(entries.value(QStringLiteral("linked/same_2.png")), QByteArray("TWO"));
        QCOMPARE(entries.size(), 1 + 2);
        QCOMPARE(r.packed(), 3);
        QCOMPARE(r.packedBytes(), qint64(6));
    }

    // A document that links to a document that links back, and one that links to itself, are not followed: the linked document is copied once, as it is.
    void aLoopOfDocumentsIsNotFollowed()
    {
        QTemporaryDir dir;
        const QString aPath = dir.filePath(QStringLiteral("a/A.docx")), bPath = dir.filePath(QStringLiteral("b/B.docx"));
        makeFile(bPath, makePackage({aPath}, {bPath}));
        makeFile(aPath, makePackage({bPath}, {aPath}));
        // (INCLUDETEXT to B from A, and B's own links back to A and to itself)
        class Linking : public Source {
        public:
            using Source::Source;
            QVector<PackLink> links() override { return {{m_a, PackKind::Document}, {m_b, PackKind::Document}, {m_b, PackKind::Document}}; }
            QString m_a, m_b;
        } linking(makePackage({}, {}), aPath);
        linking.m_a = aPath;
        linking.m_b = bPath;
        const PackReport r = packAndGo(linking, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(r.packed(), 2);   // (both links to B)
        QCOMPARE(r.items[0].status, PackItem::Refused);
        QCOMPARE(entriesOf(dir.filePath(QStringLiteral("out.zip"))).size(), 1 + 1);   // A's copy and B once
    }

    // Past the first cap the person is asked, and nothing is saved on a no; past the second, files are left out whatever the answer.
    void theCapsHold()
    {
        QTemporaryDir dir;
        QStringList files;
        for (int i = 0; i < 5; ++i) files << makeFile(dir.filePath(QStringLiteral("f/%1.png").arg(i)), QByteArray(10, 'a' + i));
        PackLimits limits;
        limits.askFiles = 3;
        int asked = 0;
        QString question;
        // a no: nothing is written
        {
            Source source(makePackage(files));
            const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("no.zip")), PackMode::Zip, limits, [&](const QString &q) {
                ++asked;
                question = q;
                return false;
            });
            QVERIFY(!r.ok && r.cancelled);
            QCOMPARE(asked, 1);
            QVERIFY2(question.contains(QLatin1Char('5')), qPrintable(question));
            QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("no.zip"))));
        }
        // no one to ask: the same
        {
            Source source(makePackage(files));
            const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("none.zip")), PackMode::Zip, limits);
            QVERIFY(r.cancelled);
            QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("none.zip"))));
        }
        // a yes
        {
            Source source(makePackage(files));
            const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("yes.zip")), PackMode::Zip, limits, [&](const QString &) { return true; });
            QVERIFY2(r.ok, qPrintable(r.error));
            QCOMPARE(r.packed(), 5);
        }
        // the size
        {
            PackLimits bytes;
            bytes.askBytes = 25;
            int again = 0;
            Source source(makePackage(files));
            packAndGo(source, dir.filePath(QStringLiteral("size.zip")), PackMode::Zip, bytes, [&](const QString &) {
                ++again;
                return false;
            });
            QCOMPARE(again, 1);
        }
        // the second cap: exactly this many files, however it is answered
        {
            PackLimits most;
            most.askFiles = 0;
            most.mostFiles = 2;
            Source source(makePackage(files));
            const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("most.zip")), PackMode::Zip, most, [](const QString &) { return true; });
            QVERIFY(r.ok);
            QCOMPARE(r.packed(), 2);
            QCOMPARE(entriesOf(dir.filePath(QStringLiteral("most.zip"))).size(), 1 + 2);
            QVERIFY(r.text().contains(QStringLiteral("(3)")));
        }
        {
            PackLimits most;
            most.askBytes = 1000;
            most.mostBytes = 25;
            Source source(makePackage(files));
            const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("bytes.zip")), PackMode::Zip, most);
            QVERIFY(r.ok);
            QCOMPARE(r.packed(), 2);
            QCOMPARE(r.packedBytes(), qint64(20));
        }
    }

    // A font whose license forbids it travels nowhere and is named; one that allows it is packed.
    void fontsThatForbidItAreLeftOutAndNamed()
    {
        QTemporaryDir dir;
        fontCopy(dir.path(), QStringLiteral("Closed-Regular.ttf"), 2);     // restricted license
        fontCopy(dir.path(), QStringLiteral("Open-Regular.ttf"), 8);       // editable embedding
        fontCopy(dir.path(), QStringLiteral("Free-Regular.ttf"), 0);       // installable
        fontCopy(dir.path(), QStringLiteral("Preview-Regular.ttf"), 4);    // preview and print only
        fontCopy(dir.path(), QStringLiteral("Unused-Regular.ttf"), 0);
        const PackedFonts fonts = packFonts({QStringLiteral("Closed"), QStringLiteral("Open"), QStringLiteral("Free"), QStringLiteral("Preview"), QStringLiteral("Not Here")}, {dir.path()});
        QStringList names;
        for (const PackFontFile &f : fonts.files) names << f.name;
        names.sort();
        QCOMPARE(names, (QStringList{QStringLiteral("Free-Regular.ttf"), QStringLiteral("Open-Regular.ttf")}));
        QCOMPARE(fonts.leftOut.size(), 1);   // (the copies are all one font, so one family name)
        QVERIFY(fontsLeftOutText(fonts.leftOut).contains(fonts.leftOut[0]));
        QVERIFY(fontsLeftOutText({}).isEmpty());
        // in the package and its report
        class Using : public Source {
        public:
            using Source::Source;
            QStringList fontFamilies() override { return {QStringLiteral("Closed"), QStringLiteral("Free")}; }
        } using_(makePackage({}));
        const PackReport r = packAndGo(using_, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip, PackLimits(), PackAsk(), {dir.path()});
        QVERIFY(r.ok);
        QCOMPARE(r.fontsPacked, QStringList{QStringLiteral("Free-Regular.ttf")});
        QCOMPARE(r.fontsLeftOut.size(), 1);
        QVERIFY(r.text().contains(r.fontsLeftOut[0]));
        const auto entries = entriesOf(dir.filePath(QStringLiteral("out.zip")));
        QVERIFY(entries.contains(QStringLiteral("fonts/Free-Regular.ttf")));
        QVERIFY(!entries.contains(QStringLiteral("fonts/Closed-Regular.ttf")));
    }

    // The fonts a package names: runs, drawings, cells, and the theme's two (as a name); not a theme placeholder.
    void fontsAPackageNamesAreFound()
    {
        ZipWriter z;
        z.add(QStringLiteral("[Content_Types].xml"), "<Types/>");
        z.add(QStringLiteral("word/document.xml"), "<w:rPr><w:rFonts w:ascii=\"Georgia\" w:eastAsia=\"MS Mincho\"/></w:rPr>");
        z.add(QStringLiteral("ppt/slides/slide1.xml"), "<a:latin typeface=\"Anton\"/><a:latin typeface=\"+mn-lt\"/>");
        z.add(QStringLiteral("xl/styles.xml"), "<font><name val=\"Calibri\"/></font>");
        z.add(QStringLiteral("xl/charts/chart1.xml"), "<a:latin typeface=\"Tahoma\"/>");
        const QStringList fonts = OpcPackSource::fontsOf(z.finish());
        QCOMPARE(fonts, (QStringList{QStringLiteral("Georgia"), QStringLiteral("MS Mincho"), QStringLiteral("Anton"), QStringLiteral("Calibri"), QStringLiteral("Tahoma")}));
    }

    // A package that cannot be read is said so, not packed empty.
    void aDamagedPackageIsRefused()
    {
        QTemporaryDir dir;
        Source source(QByteArray("this is not a zip"));
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY(!r.ok);
        QVERIFY(!r.error.isEmpty());
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("out.zip"))));
    }
};

JP_TEST_CLASS(ShellPackAndGoTests)
#include "tst_shell_packandgo.moc"
