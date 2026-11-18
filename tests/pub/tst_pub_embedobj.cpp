// The core of objects placed between apps (libs/office/ole, libs/shell/app/embeddededit.h): the objects and their caps, and editing one in its own program through a temp file that is
// watched, handed back as a whole object on each save, and deleted at the end.
#include "ole/embedobj.h"
#include "app/embeddededit.h"
#include "../testrunner.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

using namespace jp;
using namespace jp::office::ole;

Q_DECLARE_METATYPE(jp::office::ole::EmbeddedObject)

namespace {

QByteArray package(const QByteArray &body) { return QByteArray("PK\x03\x04", 4) + body; }

QImage stubPicture(Kind, const QByteArray &bytes, qint64, qint64, int)
{
    QImage im(8, 8, QImage::Format_RGB32);
    im.fill(QColor::fromRgb(qChecksum(bytes) | 0xff000000u));
    return im;
}

QByteArray sha(const QByteArray &b) { return QCryptographicHash::hash(b, QCryptographicHash::Sha256); }

bool writeFile(const QString &path, const QByteArray &bytes)
{
    QFile f(path);
    return f.open(QIODevice::WriteOnly | QIODevice::Truncate) && f.write(bytes) == bytes.size();
}

} // namespace

class PubEmbedObjectTests : public QObject {
    Q_OBJECT

    // A file of exactly `size` bytes that begins as a package.
    static bool writeSized(const QString &path, qint64 size)
    {
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
        f.write("PK\x03\x04", 4);
        return f.resize(size) && f.size() == size;
    }

    static EmbeddedObject sheetObject(const QByteArray &body = "first")
    {
        auto o = makeObject(Kind::Sheet, package(body), 3000000, 2000000, stubPicture);
        return o ? *o : EmbeddedObject();
    }

private Q_SLOTS:
    void initTestCase() { qRegisterMetaType<EmbeddedObject>(); }

    // The table of program IDs turns a kind into its ID and back; a kind of ours has a package extension; any other ID is Other and not ours.
    void programIdsRoundTrip()
    {
        for (Kind k : {Kind::Sheet, Kind::Page}) {
            QVERIFY(!progIdOf(k).isEmpty());
            QVERIFY(!extensionOf(k).isEmpty());
            QCOMPARE(kindOfProgId(progIdOf(k)), k);
        }
        QVERIFY(progIdOf(Kind::Sheet) != progIdOf(Kind::Page));
        QCOMPARE(kindOfProgId(QStringLiteral("Some.Other.1")), Kind::Other);
        QVERIFY(progIdOf(Kind::Other).isEmpty());
        QVERIFY(!keepObject(QStringLiteral("Some.Other.1"), "x", {}, 1, 1)->isOurs);
        QVERIFY(keepObject(progIdOf(Kind::Page), "x", {}, 1, 1)->isOurs);
    }

    // An object of ours is made from its package and the callback's picture, drawn at 2x; no picture, no bytes, or too many bytes make nothing, and a package of exactly the cap is kept.
    void makingAnObjectAndItsCap()
    {
        int scaleSeen = 0;
        PictureFn draw = [&](Kind k, const QByteArray &b, qint64 w, qint64 h, int scale) { scaleSeen = scale; return stubPicture(k, b, w, h, scale); };
        auto o = makeObject(Kind::Page, package("a"), 100, 200, draw, QStringLiteral("embeddings/object1"));
        QVERIFY(o.has_value());
        QCOMPARE(scaleSeen, 2);
        QVERIFY(o->isOurs);
        QCOMPARE(o->progId, progIdOf(Kind::Page));
        QCOMPARE(o->widthEmu, qint64(100));
        QCOMPARE(o->origin, QStringLiteral("embeddings/object1"));
        QVERIFY(QImage::fromData(o->previewPng, "PNG").width() == 8);
        QVERIFY(!makeObject(Kind::Page, package("a"), 1, 1, [](Kind, const QByteArray &, qint64, qint64, int) { return QImage(); }));
        QVERIFY(!makeObject(Kind::Page, QByteArray(), 1, 1, draw));
        QVERIFY(!makeObject(Kind::Other, package("a"), 1, 1, draw));
        QVERIFY(!makeObject(Kind::Page, package("a"), 1, 1, nullptr));
        QVERIFY(!makeObject(Kind::Page, QByteArray(int(kMaxObjectBytes) + 1, 'x'), 1, 1, draw));
        auto big = makeObject(Kind::Page, QByteArray(int(kMaxObjectBytes), 'x'), 1, 1, draw);
        QVERIFY(big.has_value());
        QCOMPARE(big->partBytes.size(), qsizetype(kMaxObjectBytes));
        QVERIFY(!keepObject(QStringLiteral("X.1"), QByteArray(int(kMaxObjectBytes) + 1, 'x'), {}, 1, 1));
        QVERIFY(keepObject(QStringLiteral("X.1"), QByteArray(int(kMaxObjectBytes), 'x'), {}, 1, 1).has_value());
        // A picture larger than the side cap is not kept.
        QVERIFY(pngOf(QImage(kMaxPictureSide + 1, 1, QImage::Format_RGB32)).isEmpty());
        QVERIFY(!pngOf(QImage(kMaxPictureSide, 1, QImage::Format_RGB32)).isEmpty());
    }

    // The caps of a whole file: 2,000 objects, 64 MB each, 256 MB together; each is built exactly to its cap and one more is refused, and a refusal counts nothing.
    void budgetCaps()
    {
        ObjectBudget count;
        for (int i = 0; i < kMaxObjects; ++i) QVERIFY(count.admit(1));
        QCOMPARE(count.count(), kMaxObjects);
        QVERIFY(!count.admit(1));
        QCOMPARE(count.count(), kMaxObjects);

        ObjectBudget total;
        for (int i = 0; i < 4; ++i) QVERIFY(total.admit(kMaxObjectBytes));
        QCOMPARE(total.total(), kMaxTotalBytes);
        QVERIFY(!total.admit(1));
        QCOMPARE(total.total(), kMaxTotalBytes);
        QCOMPARE(total.count(), 4);

        ObjectBudget one;
        QVERIFY(!one.admit(kMaxObjectBytes + 1));
        QVERIFY(!one.admit(-1));
        QCOMPARE(one.count(), 0);
        QVERIFY(one.admit(kMaxObjectBytes));
    }

    // An object of another kind is kept byte for byte, with its picture, and cannot be edited or have its bytes changed.
    void anotherKindPassesThroughUnchanged()
    {
        QByteArray bytes;
        for (int i = 0; i < 5000; ++i) bytes.append(char(i * 7 + 3));
        QByteArray png = pngOf(stubPicture(Kind::Other, bytes, 0, 0, 2));
        auto o = keepObject(QStringLiteral("Some.Other.1"), bytes, png, 10, 20, QStringLiteral("embeddings/oleObject3.bin"));
        QVERIFY(o.has_value());
        QCOMPARE(sha(o->partBytes), sha(bytes));
        QCOMPARE(o->previewPng, png);
        QVERIFY(!o->isOurs);
        QVERIFY(!withNewBytes(*o, package("x"), stubPicture));
        QTemporaryDir root;
        EmbeddedEdit ed;
        ed.setTempRoot(root.path());
        ed.setOpener([](const QString &, Kind) { return true; });
        QCOMPARE(ed.start(*o), 0);
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // Edit and save in the other program: the bytes and the picture change in one object; two saves in a row give two updates; closing deletes the temp file.
    void editAndSaveGivesNewBytesAndPicture()
    {
        QTemporaryDir root;
        QString opened;
        EmbeddedEdit ed;
        ed.setTempRoot(root.path());
        ed.setPollInterval(10);
        ed.setPicture(stubPicture);
        ed.setOpener([&](const QString &path, Kind k) { opened = path; return k == Kind::Sheet; });
        EmbeddedObject first = sheetObject();
        int id = ed.start(first);
        QVERIFY(id != 0);
        QVERIFY(opened.endsWith(QStringLiteral(".xlsx")));
        QCOMPARE(opened, ed.pathOf(id));
        QFile f(opened);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll(), first.partBytes);
        f.close();
        QSignalSpy spy(&ed, &EmbeddedEdit::objectUpdated);

        QByteArray second = package("second save, longer");
        QVERIFY(writeFile(opened, second));
        QTRY_COMPARE(spy.count(), 1);
        EmbeddedObject got = qvariant_cast<EmbeddedObject>(spy.at(0).at(1));
        QCOMPARE(spy.at(0).at(0).toInt(), id);
        QCOMPARE(got.partBytes, second);
        QVERIFY(got.previewPng != first.previewPng);
        QVERIFY(got.isOurs);
        QCOMPARE(got.widthEmu, first.widthEmu);

        QByteArray third = package("third");
        QVERIFY(writeFile(opened, third));
        QTRY_COMPARE(spy.count(), 2);
        QCOMPARE(qvariant_cast<EmbeddedObject>(spy.at(1).at(1)).partBytes, third);

        ed.close(id);
        QVERIFY(!ed.isOpen(id));
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // A save of the same bytes is not an update.
    void aSaveOfTheSameBytesIsNoUpdate()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        ed.setTempRoot(root.path());
        ed.setPollInterval(10);
        ed.setOpener([](const QString &, Kind) { return true; });
        EmbeddedObject o = sheetObject();
        int id = ed.start(o);
        QVERIFY(id);
        QSignalSpy spy(&ed, &EmbeddedEdit::objectUpdated);
        QVERIFY(writeFile(ed.pathOf(id), o.partBytes));
        QSignalSpy refused(&ed, &EmbeddedEdit::saveRefused);
        QVERIFY(writeFile(ed.pathOf(id), package("changed")));
        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(refused.count(), 0);
    }

    // A save that is not a package is left out and the watch goes on; the next good save is taken.
    void aSaveThatIsNotAPackageIsRefused()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        ed.setTempRoot(root.path());
        ed.setPollInterval(10);
        ed.setOpener([](const QString &, Kind) { return true; });
        int id = ed.start(sheetObject());
        QSignalSpy spy(&ed, &EmbeddedEdit::objectUpdated);
        QSignalSpy refused(&ed, &EmbeddedEdit::saveRefused);
        QVERIFY(writeFile(ed.pathOf(id), "not a package at all"));
        QTRY_COMPARE(refused.count(), 1);
        QCOMPARE(spy.count(), 0);
        QVERIFY(ed.isOpen(id));
        QVERIFY(writeFile(ed.pathOf(id), package("good again")));
        QTRY_COMPARE(spy.count(), 1);
    }

    // A deleted temp file ends the watch and leaves no folder.
    void aDeletedFileEndsTheWatch()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        ed.setTempRoot(root.path());
        ed.setPollInterval(10);
        ed.setOpener([](const QString &, Kind) { return true; });
        int id = ed.start(sheetObject());
        QSignalSpy ended(&ed, &EmbeddedEdit::watchEnded);
        QVERIFY(QFile::remove(ed.pathOf(id)));
        QTRY_COMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(0).toInt(), id);
        QCOMPARE(ended.at(0).at(1).toInt(), int(EmbeddedEdit::EndReason::Deleted));
        QVERIFY(!ed.isOpen(id));
        QTRY_VERIFY(QDir(root.path()).isEmpty());
    }

    // A file replaced by something that is not a file (a folder) ends the watch.
    void aReplacedFileEndsTheWatch()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        ed.setTempRoot(root.path());
        ed.setPollInterval(10);
        ed.setOpener([](const QString &, Kind) { return true; });
        int id = ed.start(sheetObject());
        QString path = ed.pathOf(id);
        QSignalSpy ended(&ed, &EmbeddedEdit::watchEnded);
        QVERIFY(QFile::remove(path));
        QVERIFY(QDir().mkdir(path));
        QTRY_COMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(1).toInt(), int(EmbeddedEdit::EndReason::Replaced));
        QTRY_VERIFY(QDir(root.path()).isEmpty());
    }

    // The file may grow to the object cap exactly and is taken; one byte more ends the watch.
    void aFileThatGrowsPastTheCapEndsTheWatch()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        ed.setTempRoot(root.path());
        ed.setPollInterval(10);
        ed.setOpener([](const QString &, Kind) { return true; });
        int id = ed.start(sheetObject());
        QString path = ed.pathOf(id);
        QSignalSpy spy(&ed, &EmbeddedEdit::objectUpdated);
        QSignalSpy ended(&ed, &EmbeddedEdit::watchEnded);
        QVERIFY(writeSized(path, kMaxObjectBytes));
        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(qvariant_cast<EmbeddedObject>(spy.at(0).at(1)).partBytes.size(), qsizetype(kMaxObjectBytes));
        QCOMPARE(ended.count(), 0);
        QVERIFY(writeSized(path, kMaxObjectBytes + 1));
        QTRY_COMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(1).toInt(), int(EmbeddedEdit::EndReason::TooLarge));
        QCOMPARE(spy.count(), 1);
        QTRY_VERIFY(QDir(root.path()).isEmpty());
    }

    // At most 16 objects are edited at once; a program that cannot be started leaves no temp file; closeAll and destruction clear the folder.
    void tempFilesAreBoundedAndAlwaysDeleted()
    {
        QTemporaryDir root;
        {
            EmbeddedEdit ed;
            ed.setTempRoot(root.path());
            ed.setOpener([](const QString &, Kind) { return false; });
            QCOMPARE(ed.start(sheetObject()), 0);
            QVERIFY(QDir(root.path()).isEmpty());
            ed.setOpener([](const QString &, Kind) { return true; });
            QSet<int> ids;
            for (int i = 0; i < EmbeddedEdit::kMaxOpen; ++i) {
                int id = ed.start(sheetObject());
                QVERIFY(id != 0);
                ids.insert(id);
            }
            QCOMPARE(ids.size(), EmbeddedEdit::kMaxOpen);
            QCOMPARE(ed.start(sheetObject()), 0);
            QCOMPARE(ed.openCount(), EmbeddedEdit::kMaxOpen);
            QCOMPARE(QDir(root.path()).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size(), EmbeddedEdit::kMaxOpen);
            ed.close(*ids.begin());
            QVERIFY(ed.start(sheetObject()) != 0);
            // destroyed here with 16 open
        }
        QVERIFY(QDir(root.path()).isEmpty());
        EmbeddedEdit ed;
        ed.setTempRoot(root.path());
        ed.setOpener([](const QString &, Kind) { return true; });
        QVERIFY(ed.start(sheetObject()));
        ed.closeAll();
        QCOMPARE(ed.openCount(), 0);
        QVERIFY(QDir(root.path()).isEmpty());
    }
};

JP_TEST_CLASS(PubEmbedObjectTests)
#include "tst_pub_embedobj.moc"
