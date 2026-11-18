// Pack and Go, the part every app shares (libs/shell/app/packandgo): the fonts that may travel, the links found in a package and moved inside it, the bounds on hostile links, and the caps.
#include "app/packandgo.h"
#include "core/fonts.h"
#include "core/sfnt.h"
#include "io/zip.h"
#include "../testrunner.h"

#include <QDir>
#include <QFile>
#include <QUrl>
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
using ExtraRels = QVector<QPair<QString, QString>>;   // (relationship type's last word, external target)
QByteArray makePackage(const QStringList &targets, const QStringList &fieldFiles = {}, const ExtraRels &extraRels = {}, const QMap<QString, QByteArray> &extraParts = {})
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
    for (const auto &e : extraRels)
        rels += QStringLiteral("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/%2\" Target=\"%3\" TargetMode=\"External\"/>").arg(n++).arg(e.first, xmlText(e.second));
    rels += QStringLiteral("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"https://example.com/\" TargetMode=\"External\"/></Relationships>").arg(n);
    z.add(QStringLiteral("word/_rels/document.xml.rels"), rels.toUtf8());
    for (auto it = extraParts.cbegin(); it != extraParts.cend(); ++it) z.add(it.key(), it.value());
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

// The stats and caps put back as they were.
struct Hooks {
    Hooks() { packStats() = PackStats(); }
    ~Hooks()
    {
        packStats() = PackStats();
        packReadCaps() = PackReadCaps();
    }
};

// A symbolic link to a file or folder; false where the system makes none (Windows makes a shortcut file).
bool makeSymlink(const QString &target, const QString &link)
{
    QDir().mkpath(QFileInfo(link).absolutePath());
    return QFile::link(target, link) && QFileInfo(link).isSymLink();
}

QStringList namesIn(const QString &dir) { return QDir(dir).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden); }

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
                                     QStringLiteral("file:///etc/shadow"), QStringLiteral("CON.png"), QStringLiteral("nothere.png"), QStringLiteral("%2e%2e/outside/secret.png"),
                                     QStringLiteral("%2E%2E%2Foutside%2Fsecret.png"), QStringLiteral("file://host/x"), QStringLiteral("file://host/share/secret.png"), QStringLiteral("C:\\Users\\x\\secret.png"),
                                     QStringLiteral("C:/Users/x/secret.png"), QStringLiteral("..\\outside\\secret.png"), QStringLiteral("pics\\..\\..\\outside\\secret.png"),
                                     QStringLiteral("\\\\server\\share\\secret.png"), QStringLiteral("/..//outside/../outside/secret.png.exe")};
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

    // A picture, movie, or sound given by its whole path is the usual link and is packed. A document, workbook, or object a document names by a whole path that leaves its folder is not (a
    // hostile file would pull in the person's private files); nor is a link that comes to a name a link may not (a symbolic link to a file that is not a picture).
    void anAbsolutePictureIsPackedButALinkToAnotherKindIsNot()
    {
        QTemporaryDir dir;
        const QString pic = makeFile(dir.filePath(QStringLiteral("p/pic.png")), "PIC");
        const QString secret = makeFile(dir.filePath(QStringLiteral("p/secret.txt")), "SECRET");
        const QString viaLink = dir.filePath(QStringLiteral("p/looks.png"));
        QVERIFY(makeSymlink(secret, viaLink));
        Source source(makePackage({pic, viaLink}));
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY(r.ok);
        QCOMPARE(r.packed(), 1);
        QCOMPARE(r.items[0].status, PackItem::Packed);
        QCOMPARE(r.items[1].status, PackItem::Refused);
        QVERIFY(!entriesOf(dir.filePath(QStringLiteral("out.zip"))).contains(QStringLiteral("linked/looks.png")));
        // (a movie and a sound by their whole paths travel as pictures do)
        const QString movie = makeFile(dir.filePath(QStringLiteral("p/clip.mp4")), "MOVIE"), sound = makeFile(dir.filePath(QStringLiteral("p/tune.wav")), "SOUND");
        Source media(makePackage({}, {}, {{QStringLiteral("video"), movie}, {QStringLiteral("audio"), sound}}));
        const PackReport m = packAndGo(media, dir.filePath(QStringLiteral("media.zip")), PackMode::Zip);
        QVERIFY(m.ok);
        QCOMPARE(m.packed(), 2);
    }

    // A document, workbook, or object linked from outside the document's folder is refused and listed; one inside the folder is packed.
    void aLinkedDocumentOutsideTheFolderIsRefused()
    {
        QTemporaryDir dir;
        const QString notes = makeFile(dir.filePath(QStringLiteral("home/notes.txt")), "PRIVATE NOTES");
        const QString book = makeFile(dir.filePath(QStringLiteral("home/budget.xlsx")), "PRIVATE BOOK");
        const QString pdf = makeFile(dir.filePath(QStringLiteral("home/scan.pdf")), "PRIVATE PDF");
        makeFile(dir.filePath(QStringLiteral("work/inside.txt")), "OK TEXT");
        makeFile(dir.filePath(QStringLiteral("work/sub/inside.csv")), "OK CSV");
        const QString self = makeFile(dir.filePath(QStringLiteral("work/Report.docx")), "doc");
        ExtraRels rels = {{QStringLiteral("oleObject"), notes}, {QStringLiteral("subDocument"), makeFile(dir.filePath(QStringLiteral("home/other.docx")), "PRIVATE DOC")}, {QStringLiteral("package"), pdf}, {QStringLiteral("externalLinkPath"), book},
                          {QStringLiteral("oleObject"), QUrl::fromLocalFile(notes).toString()}, {QStringLiteral("oleObject"), QStringLiteral("../home/notes.txt")},
                          {QStringLiteral("oleObject"), QStringLiteral("inside.txt")}, {QStringLiteral("subDocument"), QStringLiteral("sub/inside.csv")}};
        Source source(makePackage({}, {}, rels), self);
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(r.items.size(), 8);
        for (int i = 0; i < 6; ++i) {
            QCOMPARE(r.items[i].status, PackItem::Refused);
            QVERIFY2(!r.items[i].why.isEmpty(), qPrintable(r.items[i].target));
        }
        QCOMPARE(r.items[6].status, PackItem::Packed);
        QCOMPARE(r.items[7].status, PackItem::Packed);
        QCOMPARE(r.packed(), 2);
        QVERIFY(r.text().contains(QStringLiteral("notes.txt")));
        const auto entries = entriesOf(dir.filePath(QStringLiteral("out.zip")));
        QCOMPARE(entries.size(), 3);
        for (auto it = entries.cbegin(); it != entries.cend(); ++it) QVERIFY2(!it.value().contains("PRIVATE"), qPrintable(it.key()));
        // (a document that is not saved has no folder: a whole path to a document is refused the same)
        Source unsaved(makePackage({}, {}, {{QStringLiteral("oleObject"), notes}}));
        const PackReport u = packAndGo(unsaved, dir.filePath(QStringLiteral("unsaved.zip")), PackMode::Zip);
        QVERIFY(u.ok);
        QCOMPARE(u.packed(), 0);
    }

    // A link through a symbolic link to a folder is judged by where it really comes to: a relative link, or a whole path, that goes out of the document's folder through one is refused.
    void aSymbolicLinkCannotCarryALinkOutOfTheFolder()
    {
        QTemporaryDir dir;
        makeFile(dir.filePath(QStringLiteral("private/secret.png")), "SECRET PNG");
        makeFile(dir.filePath(QStringLiteral("private/secret.txt")), "SECRET TXT");
        makeFile(dir.filePath(QStringLiteral("work/real/ok.png")), "OK");
        const QString self = makeFile(dir.filePath(QStringLiteral("work/Report.docx")), "doc");
        if (!makeSymlink(dir.filePath(QStringLiteral("private")), dir.filePath(QStringLiteral("work/pics"))) || !QFileInfo(dir.filePath(QStringLiteral("work/pics/secret.png"))).exists())
            QSKIP("this system makes no symbolic links to folders");
        const ExtraRels rels = {{QStringLiteral("oleObject"), dir.filePath(QStringLiteral("work/pics/secret.txt"))}, {QStringLiteral("oleObject"), QStringLiteral("pics/secret.txt")}};
        Source source(makePackage({QStringLiteral("pics/secret.png"), dir.filePath(QStringLiteral("work/pics/secret.png")), QStringLiteral("real/ok.png")}, {}, rels), self);
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(r.items.size(), 5);
        QCOMPARE(r.items[0].status, PackItem::Refused);   // a relative link through the symbolic link
        QCOMPARE(r.items[2].status, PackItem::Packed);    // a real folder inside is fine
        QCOMPARE(r.items[3].status, PackItem::Refused);   // an object by a whole path through it
        QCOMPARE(r.items[4].status, PackItem::Refused);   // an object by a relative path through it
        QCOMPARE(r.packed() - (r.items[1].status == PackItem::Packed ? 1 : 0), 1);
        for (const PackItem &i : r.items)
            if (i.status == PackItem::Refused) QVERIFY2(!i.why.isEmpty(), qPrintable(i.target));
        const auto entries = entriesOf(dir.filePath(QStringLiteral("out.zip")));
        for (auto it = entries.cbegin(); it != entries.cend(); ++it) QVERIFY2(it.key() == QLatin1String("linked/secret.png") || !it.value().contains("SECRET TXT"), qPrintable(it.key()));
        QVERIFY(!entries.contains(QStringLiteral("linked/secret.txt")));
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
        const QString aPath = dir.filePath(QStringLiteral("a/A.docx")), bPath = dir.filePath(QStringLiteral("a/B.docx"));
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

    // A part that does not inflate (damaged) fails the whole Pack and Go with words, in a ZIP and in a folder, and nothing is made.
    void aDamagedPartFailsThePackAndGo()
    {
        QTemporaryDir dir;
        const QString a = makeFile(dir.filePath(QStringLiteral("elsewhere/a.png")), "AAAA");
        ZipWriter z;
        z.add(QStringLiteral("[Content_Types].xml"), "<Types/>");
        z.add(QStringLiteral("word/document.xml"), "<w:document/>");
        z.add(QStringLiteral("word/_rels/document.xml.rels"),
              QStringLiteral("<Relationships><Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"%1\" TargetMode=\"External\"/></Relationships>").arg(a).toUtf8());
        z.add(QStringLiteral("word/media/blob.bin"), QByteArray(5000, 'q'), true);
        QByteArray package = z.finish();
        const qsizetype at = package.indexOf("word/media/blob.bin");
        QVERIFY(at > 0);
        for (int i = 2; i < 8; ++i) package[at + 19 + i] = char(package[at + 19 + i] ^ 0x55);   // (inside the deflated bytes)
        Source zipSource(package);
        const PackReport r = packAndGo(zipSource, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY(!r.ok);
        QVERIFY2(r.error.contains(QStringLiteral("blob.bin")), qPrintable(r.error));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("out.zip"))));
        QDir().mkpath(dir.filePath(QStringLiteral("dest")));
        Source folderSource(package);
        QVERIFY(!packAndGo(folderSource, dir.filePath(QStringLiteral("dest")), PackMode::Folder).ok);
        QVERIFY(namesIn(dir.filePath(QStringLiteral("dest"))).isEmpty());
        // (a part over the cap is the same)
        packReadCaps().part = 1000;
        Hooks hooks;
        Source bigSource(makePackage({a}, {}, {}, {{QStringLiteral("word/media/big.bin"), QByteArray(3000, 'x')}}));
        const PackReport big = packAndGo(bigSource, dir.filePath(QStringLiteral("big.zip")), PackMode::Zip);
        QVERIFY(!big.ok);
        QVERIFY2(big.error.contains(QStringLiteral("big.bin")), qPrintable(big.error));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("big.zip"))));
    }

    // The caps on reading a package hold at their numbers (made small here): the parts, one part, and all of them; a package at a cap is read.
    void theReadCapsHold()
    {
        QTemporaryDir dir;
        const PackReadCaps defaults;
        QCOMPARE(defaults.entries, 10000);
        QCOMPARE(defaults.part, qint64(512) << 20);
        QCOMPARE(defaults.total, qint64(2) << 30);
        Hooks hooks;
        auto tryPack = [&](const QByteArray &package) {
            Source source(package);
            return packAndGo(source, dir.filePath(QStringLiteral("caps.zip")), PackMode::Zip).ok;
        };
        const QByteArray four = makePackage({});
        packReadCaps().entries = 3;
        QVERIFY(!tryPack(four));
        packReadCaps().entries = 4;
        QVERIFY(tryPack(four));
        packReadCaps() = PackReadCaps();
        packReadCaps().part = 1000;
        QVERIFY(!tryPack(makePackage({}, {}, {}, {{QStringLiteral("word/media/one.bin"), QByteArray(1001, 'a')}})));
        QVERIFY(tryPack(makePackage({}, {}, {}, {{QStringLiteral("word/media/one.bin"), QByteArray(1000, 'a')}})));
        packReadCaps() = PackReadCaps();
        const QByteArray three = makePackage({}, {}, {}, {{QStringLiteral("a.bin"), QByteArray(900, 'a')}, {QStringLiteral("b.bin"), QByteArray(900, 'b')}, {QStringLiteral("c.bin"), QByteArray(900, 'c')}});
        packReadCaps().total = 2000;
        QVERIFY(!tryPack(three));
        packReadCaps().total = 20000;
        QVERIFY(tryPack(three));
    }

    // The packages are held a part at a time: the largest part held is the largest part, never the sum; a linked file is read in pieces; and a package with nothing to move is read once and
    // not made again.
    void memoryIsOnePartAndOnePieceOfAFile()
    {
        QTemporaryDir dir;
        const QString big = dir.filePath(QStringLiteral("elsewhere/big.png"));
        {
            QDir().mkpath(QFileInfo(big).absolutePath());
            QFile f(big);
            QVERIFY(f.open(QIODevice::WriteOnly));
            for (int i = 0; i < 5; ++i) f.write(QByteArray(1 << 20, char('a' + i)));
        }
        const QMap<QString, QByteArray> parts = {{QStringLiteral("word/media/p1.bin"), QByteArray(2 << 20, 'a')}, {QStringLiteral("word/media/p2.bin"), QByteArray(2 << 20, 'b')},
                                                 {QStringLiteral("word/media/p3.bin"), QByteArray(2 << 20, 'c')}};
        Hooks hooks;
        Source source(makePackage({big}, {}, {}, parts));
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY2(r.ok, qPrintable(r.error));
        const PackStats stats = packStats();
        QVERIFY2(stats.largestPartHeld >= (2 << 20) && stats.largestPartHeld < (3 << 20), qPrintable(QString::number(stats.largestPartHeld)));
        QVERIFY2(stats.largestFileChunk > 0 && stats.largestFileChunk <= (1 << 20), qPrintable(QString::number(stats.largestFileChunk)));
        QCOMPARE(stats.zipReads, 2);   // (found, then the links moved)
        QCOMPARE(entriesOf(dir.filePath(QStringLiteral("out.zip"))).value(QStringLiteral("linked/big.png")).size(), 5 << 20);
        QCOMPARE(bytesOf(dir.filePath(QStringLiteral("out.zip"))).contains(QByteArray(1 << 20, 'e')), true);
        // nothing to move: the ZIP is read once, and its bytes go out as they came
        packStats() = PackStats();
        Source plain(makePackage({}, {}, {}, parts));
        QVERIFY(packAndGo(plain, dir.filePath(QStringLiteral("plain.zip")), PackMode::Zip).ok);
        QCOMPARE(packStats().zipReads, 1);
        QCOMPARE(packStats().rewriteSlices, qint64(0));
        // a folder too
        packStats() = PackStats();
        QDir().mkpath(dir.filePath(QStringLiteral("dest")));
        Source folder(makePackage({big}, {}, {}, parts));
        const PackReport f = packAndGo(folder, dir.filePath(QStringLiteral("dest")), PackMode::Folder);
        QVERIFY(f.ok);
        QVERIFY(packStats().largestFileChunk > 0 && packStats().largestFileChunk <= (1 << 20));
        QCOMPARE(QFileInfo(f.path + QStringLiteral("/linked/big.png")).size(), qint64(5 << 20));
    }

    // A font file the program cannot read (a collection, a damaged one, one too large to inspect) is packed as before, not left out under a license message; the first folder that has a
    // font that is packed is the one used, and a folder that has only fonts left out does not end the search.
    void fontsTheProgramCannotInspectArePackedAsBefore()
    {
        QTemporaryDir dir;
        QDir().mkpath(dir.filePath(QStringLiteral("a")));
        QDir().mkpath(dir.filePath(QStringLiteral("b")));
        makeFile(dir.filePath(QStringLiteral("a/Collection-Regular.ttf")), QByteArray("ttcf") + QByteArray(2000, '\0'));
        makeFile(dir.filePath(QStringLiteral("a/Broken-Regular.otf")), QByteArray("this is not a font at all, not even close"));
        makeFile(dir.filePath(QStringLiteral("a/Empty-Regular.ttf")), QByteArray());
        PackedFonts fonts = packFonts({QStringLiteral("Collection"), QStringLiteral("Broken"), QStringLiteral("Empty")}, {dir.filePath(QStringLiteral("a"))});
        QStringList names;
        for (const PackFontFile &f : fonts.files) names << f.name;
        names.sort();
        QCOMPARE(names, (QStringList{QStringLiteral("Broken-Regular.otf"), QStringLiteral("Collection-Regular.ttf"), QStringLiteral("Empty-Regular.ttf")}));
        QVERIFY(fonts.leftOut.isEmpty());
        // a folder with only a font whose license forbids it: the next folder is searched, as it was
        fontCopy(dir.filePath(QStringLiteral("a")), QStringLiteral("Closed-Regular.ttf"), 2);
        fontCopy(dir.filePath(QStringLiteral("b")), QStringLiteral("Closed-Bold.ttf"), 0);
        fonts = packFonts({QStringLiteral("Closed")}, {dir.filePath(QStringLiteral("a")), dir.filePath(QStringLiteral("b"))});
        QCOMPARE(fonts.files.size(), 1);
        QCOMPARE(fonts.files[0].name, QStringLiteral("Closed-Bold.ttf"));
        // and a folder that has a font that is packed ends it
        fontCopy(dir.filePath(QStringLiteral("a")), QStringLiteral("Closed-Italic.ttf"), 0);
        fonts = packFonts({QStringLiteral("Closed")}, {dir.filePath(QStringLiteral("a")), dir.filePath(QStringLiteral("b"))});
        QCOMPARE(fonts.files.size(), 1);
        QCOMPARE(fonts.files[0].name, QStringLiteral("Closed-Italic.ttf"));
    }

    // Moving the links of a part is one forward pass: 20,000 links come out right, and the pieces put together are about two for each (the work done is counted, not timed).
    void twentyThousandLinksAreMovedInOnePass()
    {
        const int count = 20000;
        QByteArray rels = "<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
        QHash<QString, QString> moved;
        for (int i = 0; i < count; ++i) {
            rels += "<Relationship Id=\"r" + QByteArray::number(i) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"pics/picture-" + QByteArray::number(i)
                    + ".png\" TargetMode=\"External\"/>";
            moved.insert(QStringLiteral("pics/picture-%1.png").arg(i), QStringLiteral("linked/p%1.png").arg(i));
        }
        rels += "</Relationships>";
        ZipWriter z;
        z.add(QStringLiteral("[Content_Types].xml"), "<Types/>");
        z.add(QStringLiteral("word/_rels/document.xml.rels"), rels);
        const QByteArray package = z.finish();
        Hooks hooks;
        QString error;
        const QByteArray out = OpcPackSource::withLinks(package, moved, &error);
        QVERIFY2(!out.isEmpty(), qPrintable(error));
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(out, parts));
        const QByteArray part = parts.value(QStringLiteral("word/_rels/document.xml.rels"));
        QCOMPARE(part.count("Target=\"linked/p"), count);
        QVERIFY(part.contains("Target=\"linked/p19999.png\""));
        QVERIFY(part.contains("Id=\"r0\"") && !part.contains("pics/"));
        QVERIFY2(packStats().rewriteSlices <= 2 * count + 4 && packStats().rewriteSlices >= count, qPrintable(QString::number(packStats().rewriteSlices)));
        QVERIFY2(packStats().rewriteBytesIn == rels.size(), qPrintable(QString::number(packStats().rewriteBytesIn)));
    }

    // A folder is made whole or not at all: when a file changes while it is copied, no folder (not even a half-made one) is left, and a ZIP is not written either.
    void aFailedFolderLeavesNothing()
    {
        QTemporaryDir dir;
        const QString a = makeFile(dir.filePath(QStringLiteral("elsewhere/a.png")), "AAAA");
        const QString b = makeFile(dir.filePath(QStringLiteral("elsewhere/b.png")), "BBBB");
        class Growing : public Source {
        public:
            using Source::Source;
            bool saveCopy(const QHash<QString, QString> &moved, QByteArray *bytes, QString *error) override
            {
                QFile f(m_file);
                if (f.open(QIODevice::Append)) f.write("MORE THAN IT SAID");
                return Source::saveCopy(moved, bytes, error);
            }
            QString m_file;
        };
        QDir().mkpath(dir.filePath(QStringLiteral("dest")));
        Growing folder(makePackage({a, b}));
        folder.m_file = b;
        const PackReport r = packAndGo(folder, dir.filePath(QStringLiteral("dest")), PackMode::Folder);
        QVERIFY(!r.ok);
        QVERIFY(!r.error.isEmpty());
        QVERIFY2(namesIn(dir.filePath(QStringLiteral("dest"))).isEmpty(), qPrintable(namesIn(dir.filePath(QStringLiteral("dest"))).join(QLatin1Char('|'))));
        Growing zip(makePackage({a, b}));
        zip.m_file = b;
        QVERIFY(!packAndGo(zip, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip).ok);
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("out.zip"))));
    }

    // A field whose instruction is split across several runs is found and moved (and a second field is not joined to the first).
    void anIncludeFieldSplitAcrossRunsIsFoundAndMoved()
    {
        QTemporaryDir dir;
        const QString a = makeFile(dir.filePath(QStringLiteral("elsewhere/a.png")), "AAAA");
        const QString t = makeFile(dir.filePath(QStringLiteral("elsewhere/t.docx")), "TTTT");
        ZipWriter z;
        z.add(QStringLiteral("[Content_Types].xml"), "<Types/>");
        const QString document = QStringLiteral(
            "<w:document xmlns:w=\"w\"><w:body><w:p>"
            "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>"
            "<w:r><w:instrText xml:space=\"preserve\"> INCLUDEPIC</w:instrText></w:r><w:r><w:instrText>TURE \"</w:instrText></w:r><w:r><w:instrText>%1\" \\d </w:instrText></w:r>"
            "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"
            "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> INCLUDETEXT %2 </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"
            "</w:p></w:body></w:document>").arg(a, t);
        z.add(QStringLiteral("word/document.xml"), document.toUtf8());
        const QByteArray package = z.finish();
        QCOMPARE(OpcPackSource::linksOf(package).size(), 2);
        Source source(package, dir.filePath(QStringLiteral("work/Report.docx")));
        makeFile(dir.filePath(QStringLiteral("work/Report.docx")), "x");
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(r.packed(), 1);   // (the picture; the text file's document link is outside the folder)
        QCOMPARE(r.items[0].target, a);
        QCOMPARE(r.items[0].status, PackItem::Packed);
        QCOMPARE(r.items[1].status, PackItem::Refused);
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(entriesOf(dir.filePath(QStringLiteral("out.zip"))).value(QStringLiteral("Report.docx")), parts));
        QString joined;
        QRegularExpression run(QStringLiteral("<w:instrText[^>]*>([^<]*)</w:instrText>"));
        for (auto it = run.globalMatch(QString::fromUtf8(parts.value(QStringLiteral("word/document.xml")))); it.hasNext();) joined += it.next().captured(1);
        QVERIFY2(joined.contains(QStringLiteral("INCLUDEPICTURE \"linked/a.png\"")), qPrintable(joined));
        QVERIFY(!joined.contains(a));
    }

    // The names are checked as exact: two files that differ only in case get two names that differ by more than case.
    void namesThatDifferOnlyInCaseGetTwoNames()
    {
        QTemporaryDir dir;
        const QString upper = makeFile(dir.filePath(QStringLiteral("x/A.png")), "UPPER");
        const QString lower = makeFile(dir.filePath(QStringLiteral("y/a.png")), "LOWER");
        Source source(makePackage({upper, lower}));
        const PackReport r = packAndGo(source, dir.filePath(QStringLiteral("out.zip")), PackMode::Zip);
        QVERIFY(r.ok);
        QCOMPARE(r.packed(), 2);
        QVERIFY(r.items[0].packedAs.toLower() != r.items[1].packedAs.toLower());
        const auto entries = entriesOf(dir.filePath(QStringLiteral("out.zip")));
        QSet<QString> lowered;
        for (auto it = entries.cbegin(); it != entries.cend(); ++it) lowered.insert(it.key().toLower());
        QCOMPARE(lowered.size(), entries.size());
        QCOMPARE(entries.value(r.items[0].packedAs), QByteArray("UPPER"));
        QCOMPARE(entries.value(r.items[1].packedAs), QByteArray("LOWER"));
    }

    // The defaults: 2,000 files and 1 GB at most, asking at 200 files and 256 MB; a file that would pass the 1 GB is left out and named, and a 2,001st file the same.
    void theDefaultCapsAre2000FilesAndOneGigabyte()
    {
        const PackLimits limits;
        QCOMPARE(limits.askFiles, 200);
        QCOMPARE(limits.askBytes, qint64(256) << 20);
        QCOMPARE(limits.mostFiles, 2000);
        QCOMPARE(limits.mostBytes, qint64(1) << 30);
        QTemporaryDir dir;
        // 2,001 small files: 2,000 are taken
        QStringList files;
        QDir().mkpath(dir.filePath(QStringLiteral("many")));
        for (int i = 0; i < 2001; ++i) files << makeFile(dir.filePath(QStringLiteral("many/f%1.png").arg(i)), QByteArray(4, 'a'));
        Source many(makePackage(files));
        const PackReport r = packAndGo(many, dir.filePath(QStringLiteral("many.zip")), PackMode::Zip, PackLimits(), [](const QString &) { return true; });
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(r.packed(), 2000);
        QCOMPARE(r.items.last().status, PackItem::Refused);
        // two files of 600 MB (sparse, never read: the person is asked and says no): the second would pass 1 GB
        QStringList big;
        for (const char *name : {"big1.png", "big2.png"}) {
            QFile f(dir.filePath(QString::fromLatin1(name)));
            if (!f.open(QIODevice::WriteOnly) || !f.resize(qint64(600) << 20)) QSKIP("this folder cannot hold a large sparse file");
            big << f.fileName();
        }
        QString question;
        Source two(makePackage(big));
        const PackReport q = packAndGo(two, dir.filePath(QStringLiteral("two.zip")), PackMode::Zip, PackLimits(), [&](const QString &text) {
            question = text;
            return false;
        });
        QVERIFY(q.cancelled);
        QVERIFY2(question.contains(QStringLiteral("1 linked files, 600.0 MB")), qPrintable(question));
        QCOMPARE(q.items[0].status, PackItem::Packed);
        QCOMPARE(q.items[1].status, PackItem::Refused);
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
