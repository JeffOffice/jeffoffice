// The core of objects placed between apps (libs/office/ole, libs/shell/app/embeddededit.h): the objects and their caps, and editing one in its own program through a temp file that is
// watched, handed back as new bytes and a picture on each save, and deleted at the end. Most tests run the looks by hand (pollNow with a long interval) and shrink the caps
// (setLimits); the tests that use the real caps say so.
#include "ole/embedobj.h"
#include "app/embeddededit.h"
#include "../testrunner.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#ifdef Q_OS_UNIX
#include <sys/time.h>
#include <unistd.h>
#endif

using namespace jp;
using namespace jp::office::ole;

namespace {

QByteArray eocd(quint16 count = 0, quint32 cdSize = 0, quint32 cdOff = 0)
{
    QByteArray e(22, '\0');
    uchar *p = reinterpret_cast<uchar *>(e.data());
    p[0] = 'P'; p[1] = 'K'; p[2] = 5; p[3] = 6;
    p[10] = count & 0xff; p[11] = count >> 8;
    for (int i = 0; i < 4; ++i) { p[12 + i] = (cdSize >> (8 * i)) & 0xff; p[16 + i] = (cdOff >> (8 * i)) & 0xff; }
    return e;
}

// A package of the shape the checks read: a local header's signature, the body, and an empty central directory.
QByteArray package(const QByteArray &body) { return QByteArray("PK\x03\x04", 4) + body + eocd(); }

// A package whose central directory has one entry for each size, each claiming to unpack to that many bytes.
QByteArray zipClaiming(const QList<quint32> &sizes)
{
    QByteArray z = QByteArray("PK\x03\x04", 4) + QByteArray(26, '\0');
    QByteArray cd;
    for (quint32 s : sizes) {
        QByteArray e(47, '\0');
        uchar *p = reinterpret_cast<uchar *>(e.data());
        p[0] = 'P'; p[1] = 'K'; p[2] = 1; p[3] = 2;
        for (int i = 0; i < 4; ++i) p[24 + i] = (s >> (8 * i)) & 0xff;
        p[28] = 1;   // a one-letter name
        p[46] = 'a';
        cd += e;
    }
    const quint32 off = quint32(z.size());
    return z + cd + eocd(quint16(sizes.size()), quint32(cd.size()), off);
}

QImage stubPicture(const QByteArray &bytes, qint64, qint64, int)
{
    QImage im(8, 8, QImage::Format_RGB32);
    im.fill(QColor::fromRgb(qChecksum(QByteArrayView(bytes)) | 0xff000000u));
    return im;
}

QByteArray sha(const QByteArray &b) { return QCryptographicHash::hash(b, QCryptographicHash::Sha256); }

bool writeFile(const QString &path, const QByteArray &bytes)
{
    QFile f(path);
    return f.open(QIODevice::WriteOnly | QIODevice::Truncate) && f.write(bytes) == bytes.size();
}

// A file of exactly `size` bytes that is a package.
bool writeSized(const QString &path, qint64 size)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write("PK\x03\x04", 4);
    if (!f.resize(size) || !f.seek(size - 22)) return false;
    return f.write(eocd()) == 22 && f.flush();
}

QByteArray pngOfSize(int w, int h)
{
    QByteArray out;
    QBuffer buf(&out);
    buf.open(QIODevice::WriteOnly);
    QImage im(w, h, QImage::Format_RGB32);
    im.fill(Qt::white);   // a new image holds whatever the memory held, and two encodings of it would differ in size
    im.save(&buf, "PNG");
    return out;
}

EmbeddedObject sheetObject(const QByteArray &body = "first")
{
    ObjectBudget budget;
    auto o = makeObject(Kind::Sheet, package(body), 3000000, 2000000, stubPicture, budget);
    return o ? *o : EmbeddedObject();
}

} // namespace

class PubEmbedObjectTests : public QObject {
    Q_OBJECT

    // An editor that does not open anything, looked at only by pollNow.
    static void prepare(EmbeddedEdit &ed, const QString &root, QString *opened = nullptr)
    {
        ed.setTempRoot(root);
        ed.setPollInterval(600000);
        ed.setPicture(Kind::Sheet, stubPicture);
        ed.setPicture(Kind::Page, stubPicture);
        ed.setOpener([opened](const QString &path, Kind) { if (opened) *opened = path; return true; });
    }
    static void looks(EmbeddedEdit &ed, int n) { for (int i = 0; i < n; ++i) ed.pollNow(); }

private Q_SLOTS:
    void cleanup() { resetLimits(); }

    // The table of program IDs turns a kind into its ID and back (with or without the version); a kind of ours has a package extension; any other ID is Other and not ours.
    void programIdsRoundTrip()
    {
        for (Kind k : {Kind::Sheet, Kind::Page}) {
            QVERIFY(!progIdOf(k).isEmpty());
            QVERIFY(!extensionOf(k).isEmpty());
            QCOMPARE(kindOfProgId(progIdOf(k)), k);
            QCOMPARE(kindOfProgId(progIdOf(k).section(QLatin1Char('.'), 0, 1)), k);   // the same ID without its version
        }
        QVERIFY(progIdOf(Kind::Sheet) != progIdOf(Kind::Page));
        QCOMPARE(kindOfProgId(QStringLiteral("Some.Other.1")), Kind::Other);
        QVERIFY(progIdOf(Kind::Other).isEmpty());
        ObjectBudget b;
        QVERIFY(!keepObject(QStringLiteral("Some.Other.1"), package("x"), {}, 1, 1, b)->isOurs);
        auto unversioned = keepObject(progIdOf(Kind::Page).section(QLatin1Char('.'), 0, 1), package("x"), {}, 1, 1, b);
        QVERIFY(unversioned->isOurs);
        QCOMPARE(unversioned->kind(), Kind::Page);
    }

    // An object of ours is made from its package and the callback's picture, drawn at 2x; no picture, no bytes, or too many bytes make nothing, and a package of exactly the cap is kept.
    void makingAnObjectAndItsCap()
    {
        Limits l;
        l.objectBytes = 1000;
        setLimits(l);
        int scaleSeen = 0;
        PictureFn draw = [&](const QByteArray &b, qint64 w, qint64 h, int scale) { scaleSeen = scale; return stubPicture(b, w, h, scale); };
        ObjectBudget budget;
        auto o = makeObject(Kind::Page, package("a"), 100, 200, draw, budget, QStringLiteral("embeddings/object1"));
        QVERIFY(o.has_value());
        QCOMPARE(scaleSeen, 2);
        QVERIFY(o->isOurs);
        QCOMPARE(o->progId, progIdOf(Kind::Page));
        QCOMPARE(o->widthEmu, qint64(100));
        QCOMPARE(o->origin, QStringLiteral("embeddings/object1"));
        QCOMPARE(QImage::fromData(o->previewPng, "PNG").width(), 8);
        QVERIFY(!makeObject(Kind::Page, package("a"), 1, 1, [](const QByteArray &, qint64, qint64, int) { return QImage(); }, budget));
        QVERIFY(!makeObject(Kind::Page, QByteArray(), 1, 1, draw, budget));
        QVERIFY(!makeObject(Kind::Other, package("a"), 1, 1, draw, budget));
        QVERIFY(!makeObject(Kind::Page, package("a"), 1, 1, nullptr, budget));
        QVERIFY(!makeObject(Kind::Page, package(QByteArray(1001 - 26, 'x')), 1, 1, draw, budget));
        auto big = makeObject(Kind::Page, package(QByteArray(1000 - 26, 'x')), 1, 1, draw, budget);
        QVERIFY(big.has_value());
        QCOMPARE(big->partBytes.size(), qsizetype(1000));
        QVERIFY(!keepObject(QStringLiteral("X.1"), QByteArray(1001, 'x'), {}, 1, 1, budget));
        QVERIFY(keepObject(QStringLiteral("X.1"), QByteArray(1000, 'x'), {}, 1, 1, budget).has_value());
        // A picture larger than the side cap is not made.
        QVERIFY(pngOf(QImage(8193, 1, QImage::Format_RGB32)).isEmpty());
        QVERIFY(!pngOf(QImage(8192, 1, QImage::Format_RGB32)).isEmpty());
    }

    // The size of an object is checked against the picture's side cap before the callback is called: a size of 10^13, zero, or negative never reaches it.
    void sizesAreCheckedBeforeTheCallback()
    {
        int calls = 0;
        PictureFn draw = [&](const QByteArray &b, qint64 w, qint64 h, int s) { ++calls; return stubPicture(b, w, h, s); };
        ObjectBudget budget;
        for (qint64 bad : {qint64(10000000000000LL), qint64(0), qint64(-5), std::numeric_limits<qint64>::max()}) {
            QVERIFY(!makeObject(Kind::Sheet, package("a"), bad, 1000, draw, budget));
            QVERIFY(!makeObject(Kind::Sheet, package("a"), 1000, bad, draw, budget));
            auto saved = readSave(Kind::Sheet, package("a"), bad, 1000, draw);   // a save is taken, but its picture is not drawn
            QVERIFY(saved.has_value() && saved->pictureStale);
        }
        QCOMPARE(calls, 0);
        // exactly 8192 pixels at 2x is 4096 points: accepted; one EMU more is refused
        const qint64 exact = 8192LL * kEmuPerPoint / kPictureScale;
        QVERIFY(makeObject(Kind::Sheet, package("a"), exact, 1000, draw, budget).has_value());
        QCOMPARE(calls, 1);
        QVERIFY(!makeObject(Kind::Sheet, package("a"), exact + 1, 1000, draw, budget));
        QCOMPARE(calls, 1);
    }

    // A picture read from a file is checked from its header: a PNG within the caps is kept; one that is not a PNG, has too long a side, or has too many bytes is left out, and the object stays.
    void aPictureReadFromAFileIsChecked()
    {
        const QByteArray good = pngOfSize(16, 16);
        QVERIFY(!good.isEmpty());
        ObjectBudget budget;
        auto keep = [&](const QByteArray &png) { return keepObject(QStringLiteral("X.1"), package("p"), png, 1, 1, budget); };
        QCOMPARE(keep(good)->previewPng, good);
        QVERIFY(keep(QByteArray("not a picture at all"))->previewPng.isEmpty());
        QVERIFY(keep(pngOfSize(8193, 1))->previewPng.isEmpty());
        QCOMPARE(keep(pngOfSize(8192, 1))->previewPng.size(), pngOfSize(8192, 1).size());
        QByteArray jpeg;
        QBuffer buf(&jpeg);
        buf.open(QIODevice::WriteOnly);
        QImage small(4, 4, QImage::Format_RGB32);
        small.fill(Qt::white);
        if (small.save(&buf, "JPEG")) QVERIFY(keep(jpeg)->previewPng.isEmpty());
        Limits l;
        l.pictureBytes = good.size();
        setLimits(l);
        QCOMPARE(keep(good)->previewPng, good);   // exactly the cap
        l.pictureBytes = good.size() - 1;
        setLimits(l);
        QVERIFY(keep(good)->previewPng.isEmpty());
        QVERIFY(keep(good).has_value());          // the object is kept without it
    }

    // The caps of a whole file (real values): 2,000 objects, 64 MB each, 256 MB together with their pictures, each built exactly to its cap and one more refused.
    void budgetCaps()
    {
        ObjectBudget count;
        for (int i = 0; i < 2000; ++i) QVERIFY(count.admit(1));
        QCOMPARE(count.count(), 2000);
        QVERIFY(!count.admit(1));
        QCOMPARE(count.count(), 2000);

        const qint64 mb64 = 64LL * 1024 * 1024;
        ObjectBudget total;
        for (int i = 0; i < 4; ++i) QVERIFY(total.admit(mb64));
        QCOMPARE(total.total(), 256LL * 1024 * 1024);
        QVERIFY(!total.admit(1));
        QCOMPARE(total.count(), 4);

        ObjectBudget pictures;   // pictures count in the total: three objects of 64 MB and 16 MB of picture each leave nothing for a fourth 64 MB
        QVERIFY(pictures.admit(mb64, 16LL * 1024 * 1024));
        QVERIFY(pictures.admit(mb64, 16LL * 1024 * 1024));
        QVERIFY(pictures.admit(mb64, 16LL * 1024 * 1024));
        QVERIFY(!pictures.admit(mb64, 16LL * 1024 * 1024));
        QVERIFY(pictures.admit(16LL * 1024 * 1024, 0));   // 256 - 240 = 16 MB left, exactly
        QVERIFY(!pictures.admit(1));
        ObjectBudget bigPicture;
        QVERIFY(!bigPicture.admit(1, 16LL * 1024 * 1024 + 1));

        ObjectBudget one;
        QVERIFY(!one.admit(mb64 + 1));
        QVERIFY(!one.admit(-1));
        QCOMPARE(one.count(), 0);
        QVERIFY(one.admit(mb64));
    }

    // The builders take the budget and refuse when it says no, counting nothing for a refusal; the picture's bytes are in the count.
    void theBuildersAskTheBudget()
    {
        const QByteArray pkg = package("body");
        const QByteArray png = pngOfSize(16, 16);
        Limits l;
        l.objects = 1;
        setLimits(l);
        int draws = 0;
        PictureFn counting = [&](const QByteArray &b, qint64 w, qint64 h, int s) { ++draws; return stubPicture(b, w, h, s); };
        ObjectBudget budget;
        QVERIFY(makeObject(Kind::Sheet, pkg, 1000, 1000, counting, budget).has_value());
        QCOMPARE(budget.count(), 1);
        QCOMPARE(draws, 1);
        const qint64 before = budget.total();
        QVERIFY(!makeObject(Kind::Sheet, pkg, 1000, 1000, counting, budget));
        QCOMPARE(draws, 1);   // a refused object is not drawn
        QVERIFY(!keepObject(QStringLiteral("X.1"), pkg, png, 1, 1, budget));
        QCOMPARE(budget.count(), 1);
        QCOMPARE(budget.total(), before);

        l = Limits();
        l.totalBytes = pkg.size() + png.size();
        setLimits(l);
        ObjectBudget exact;
        QVERIFY(keepObject(QStringLiteral("X.1"), pkg, png, 1, 1, exact).has_value());
        QCOMPARE(exact.total(), l.totalBytes);
        l.totalBytes -= 1;
        setLimits(l);
        ObjectBudget under;
        QVERIFY(!keepObject(QStringLiteral("X.1"), pkg, png, 1, 1, under));
    }

    // A program ID of ours with bytes that are not a safe package is kept as another kind (byte for byte) and is never opened.
    void ourProgIdWithJunkBytesIsNotOurs()
    {
        ObjectBudget budget;
        const QByteArray junk = "definitely not a zip file";
        auto o = keepObject(progIdOf(Kind::Sheet), junk, {}, 1000, 1000, budget);
        QVERIFY(o.has_value());
        QVERIFY(!o->isOurs);
        QCOMPARE(o->kind(), Kind::Other);
        QCOMPARE(o->progId, progIdOf(Kind::Sheet));
        QCOMPARE(o->partBytes, junk);
        QTemporaryDir root;
        EmbeddedEdit ed;
        int opened = 0;
        prepare(ed, root.path());
        ed.setOpener([&](const QString &, Kind) { ++opened; return true; });
        QObject owner;
        QCOMPARE(ed.start(&owner, QStringLiteral("a"), *o), 0);
        EmbeddedObject forged = *o;   // a window that sets the flag itself still cannot start it
        forged.isOurs = true;
        QCOMPARE(ed.start(&owner, QStringLiteral("a"), forged), 0);
        QCOMPARE(opened, 0);
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // The central directory is summed before anything reads the package: more than 1 GB unpacked (a hand-made zip claiming 10 GB) or more than 10,000 entries is refused, the caps exactly are kept.
    void aPackageThatUnpacksTooMuchIsRefused()
    {
        const quint32 gb = 1024u * 1024u * 1024u;
        QVERIFY(packageWithinLimits(zipClaiming({gb})));
        QVERIFY(!packageWithinLimits(zipClaiming({gb, 1})));
        QVERIFY(packageWithinLimits(zipClaiming({gb / 2, gb / 2})));
        const quint32 big = 3500000000u;
        QByteArray tenGb = zipClaiming({big, big, big});
        QVERIFY(looksLikePackage(tenGb));
        QVERIFY(!packageWithinLimits(tenGb));
        QVERIFY(!packageWithinLimits(zipClaiming({0xFFFFFFFFu})));   // the 64-bit form
        QVERIFY(packageWithinLimits(zipClaiming(QList<quint32>(10000, 0))));
        QVERIFY(!packageWithinLimits(zipClaiming(QList<quint32>(10001, 0))));
        QVERIFY(!packageWithinLimits(QByteArrayLiteral("PK\x03\x04 and nothing else")));
        int calls = 0;
        PictureFn draw = [&](const QByteArray &b, qint64 w, qint64 h, int s) { ++calls; return stubPicture(b, w, h, s); };
        ObjectBudget budget;
        QVERIFY(!makeObject(Kind::Sheet, tenGb, 1000, 1000, draw, budget));
        QVERIFY(!readSave(Kind::Sheet, tenGb, 1000, 1000, draw));
        QCOMPARE(calls, 0);
        auto kept = keepObject(progIdOf(Kind::Sheet), tenGb, {}, 1, 1, budget);
        QVERIFY(kept.has_value());
        QVERIFY(!kept->isOurs);
        QCOMPARE(kept->partBytes, tenGb);
    }

    // An object of another kind is kept byte for byte, with its picture, and cannot be edited.
    void anotherKindPassesThroughUnchanged()
    {
        QByteArray bytes;
        for (int i = 0; i < 5000; ++i) bytes.append(char(i * 7 + 3));
        QByteArray png = pngOfSize(8, 8);
        ObjectBudget budget;
        auto o = keepObject(QStringLiteral("Some.Other.1"), bytes, png, 10, 20, budget, QStringLiteral("embeddings/oleObject3.bin"));
        QVERIFY(o.has_value());
        QCOMPARE(sha(o->partBytes), sha(bytes));
        QCOMPARE(o->previewPng, png);
        QVERIFY(!o->isOurs);
        QVERIFY(!readSave(o->kind(), package("x"), 10, 20, stubPicture));
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        QCOMPARE(ed.start(&owner, QStringLiteral("k"), *o), 0);
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // Edit and save in the other program (the real timer): the bytes and the picture come back, two saves in a row give two updates, and closing deletes the temp folder.
    void editAndSaveGivesNewBytesAndPicture()
    {
        QTemporaryDir root;
        QString opened;
        EmbeddedEdit ed;
        prepare(ed, root.path(), &opened);
        ed.setPollInterval(10);
        QObject owner;
        EmbeddedObject first = sheetObject();
        int id = ed.start(&owner, QStringLiteral("k"), first);
        QVERIFY(id != 0);
        QVERIFY(opened.endsWith(QStringLiteral(".xlsx")));
        QCOMPARE(opened, ed.pathOf(id));
        QFile f(opened);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll(), first.partBytes);
        f.close();
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);

        QByteArray second = package("second save, longer");
        QVERIFY(writeFile(opened, second));
        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), id);
        QCOMPARE(spy.at(0).at(1).toByteArray(), second);
        QVERIFY(spy.at(0).at(2).toByteArray() != first.previewPng);
        QVERIFY(!spy.at(0).at(3).toBool());

        QByteArray third = package("third");
        QVERIFY(writeFile(opened, third));
        QTRY_COMPARE(spy.count(), 2);
        QCOMPARE(spy.at(1).at(1).toByteArray(), third);

        ed.close(id);
        QVERIFY(!ed.isOpen(id));
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // The save carries no object: the window puts the bytes and picture into its current object, so a resize made during the edit stays; a picture that could not be drawn is flagged and the old one kept.
    void aResizeDuringTheEditSurvivesTheSave()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        EmbeddedObject current = sheetObject();
        const QByteArray oldPng = current.previewPng;
        int id = ed.start(&owner, QStringLiteral("k"), current);
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);
        current.widthEmu = 7654321;   // the person resizes it while the other program has it open
        current.heightEmu = 1234567;
        QVERIFY(writeFile(ed.pathOf(id), package("saved")));
        looks(ed, 2);
        QCOMPARE(spy.count(), 1);
        SavedObject saved{spy.at(0).at(1).toByteArray(), spy.at(0).at(2).toByteArray(), spy.at(0).at(3).toBool()};
        applySave(current, saved);
        QCOMPARE(current.widthEmu, qint64(7654321));
        QCOMPARE(current.heightEmu, qint64(1234567));
        QCOMPARE(current.partBytes, package("saved"));
        QVERIFY(current.previewPng != oldPng);

        ed.setPicture(Kind::Sheet, [](const QByteArray &, qint64, qint64, int) { return QImage(); });
        const QByteArray keptPng = current.previewPng;
        QVERIFY(writeFile(ed.pathOf(id), package("saved again")));
        looks(ed, 2);
        QCOMPARE(spy.count(), 2);
        QVERIFY(spy.at(1).at(3).toBool());
        QVERIFY(spy.at(1).at(2).toByteArray().isEmpty());
        applySave(current, SavedObject{spy.at(1).at(1).toByteArray(), spy.at(1).at(2).toByteArray(), true});
        QCOMPARE(current.partBytes, package("saved again"));
        QCOMPARE(current.previewPng, keptPng);
    }

    // A save counts only when the file has held the same for two looks: one look is not enough, and a file written in two parts is read once, whole.
    void aTornSaveIsReadOnceWhole()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject());
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);
        const QByteArray whole = package(QByteArray(300, 'w'));
        QVERIFY(writeFile(ed.pathOf(id), whole.left(100)));   // the first part
        looks(ed, 1);
        QVERIFY(writeFile(ed.pathOf(id), whole));             // the rest, before the second look
        looks(ed, 1);
        QCOMPARE(spy.count(), 0);
        looks(ed, 1);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(1).toByteArray(), whole);
        looks(ed, 3);
        QCOMPARE(spy.count(), 1);
    }

    // A file rewritten with the same size and the same time is still seen, because the content is hashed.
    void aRewriteWithTheSameSizeAndTimeIsSeen()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject("aaaaa"));
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);
        const QString path = ed.pathOf(id);
        const QDateTime when = QFileInfo(path).lastModified();
        const QByteArray other = package("bbbbb");
        QCOMPARE(other.size(), sheetObject("aaaaa").partBytes.size());
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(f.write(other), qint64(other.size()));
        QVERIFY(f.flush());
        QVERIFY(f.setFileTime(when, QFileDevice::FileModificationTime));
        f.close();
        QCOMPARE(QFileInfo(path).lastModified(), when);
        looks(ed, 2);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(1).toByteArray(), other);
    }

    // The same bytes saved again (only the time changes) are no update, however many looks are taken; the next real change is one.
    void aSaveOfTheSameBytesIsNoUpdate()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        EmbeddedObject o = sheetObject();
        int id = ed.start(&owner, QStringLiteral("k"), o);
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);
        QSignalSpy refused(&ed, &EmbeddedEdit::saveRefused);
        QFile f(ed.pathOf(id));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(f.write(o.partBytes), qint64(o.partBytes.size()));
        QVERIFY(f.flush());
        QVERIFY(f.setFileTime(QDateTime::currentDateTime().addSecs(60), QFileDevice::FileModificationTime));
        f.close();
        looks(ed, 4);   // the file changed (its time) and held for several looks
        QCOMPARE(spy.count(), 0);
        QCOMPARE(refused.count(), 0);
        QVERIFY(writeFile(ed.pathOf(id), package("changed")));
        looks(ed, 2);
        QCOMPARE(spy.count(), 1);
    }

    // A save that is not a package is left out and the watch goes on; the next good save is taken.
    void aSaveThatIsNotAPackageIsRefused()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject());
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);
        QSignalSpy refused(&ed, &EmbeddedEdit::saveRefused);
        QVERIFY(writeFile(ed.pathOf(id), "not a package at all"));
        looks(ed, 2);
        QCOMPARE(refused.count(), 1);
        looks(ed, 3);
        QCOMPARE(refused.count(), 1);   // not again at each look
        QCOMPARE(spy.count(), 0);
        QVERIFY(ed.isOpen(id));
        QVERIFY(writeFile(ed.pathOf(id), zipClaiming({3500000000u, 3500000000u, 3500000000u})));
        looks(ed, 2);
        QCOMPARE(refused.count(), 2);
        QVERIFY(writeFile(ed.pathOf(id), package("good again")));
        looks(ed, 2);
        QCOMPARE(spy.count(), 1);
    }

    // A deleted temp file ends the watch (after two looks without it) and leaves no folder.
    void aDeletedFileEndsTheWatch()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject());
        QSignalSpy ended(&ed, &EmbeddedEdit::watchEnded);
        QVERIFY(QFile::remove(ed.pathOf(id)));
        looks(ed, 1);
        QCOMPARE(ended.count(), 0);   // may be a delete and put back
        looks(ed, 1);
        QCOMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(0).toInt(), id);
        QCOMPARE(ended.at(0).at(1).value<EmbeddedEdit::EndReason>(), EmbeddedEdit::EndReason::Deleted);
        QVERIFY(!ed.isOpen(id));
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // A file replaced by something that is not a file (a folder) ends the watch.
    void aReplacedFileEndsTheWatch()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject());
        QString path = ed.pathOf(id);
        QSignalSpy ended(&ed, &EmbeddedEdit::watchEnded);
        QVERIFY(QFile::remove(path));
        QVERIFY(QDir().mkdir(path));
        looks(ed, 1);
        QCOMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(1).value<EmbeddedEdit::EndReason>(), EmbeddedEdit::EndReason::Replaced);
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // The file may grow to the cap exactly and is taken; one byte more ends the watch (cap shrunk to 2,000 bytes).
    void aFileThatGrowsPastTheCapEndsTheWatch()
    {
        Limits l;
        l.objectBytes = 2000;
        setLimits(l);
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject());
        QString path = ed.pathOf(id);
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);
        QSignalSpy ended(&ed, &EmbeddedEdit::watchEnded);
        QVERIFY(writeSized(path, 2000));
        looks(ed, 2);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(1).toByteArray().size(), qsizetype(2000));
        QCOMPARE(ended.count(), 0);
        QVERIFY(writeSized(path, 2001));
        looks(ed, 1);
        QCOMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(1).value<EmbeddedEdit::EndReason>(), EmbeddedEdit::EndReason::TooLarge);
        QCOMPARE(spy.count(), 1);
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // The same with the real cap of 64 MB (a sparse file).
    void aFileThatGrowsPastTheRealCapEndsTheWatch()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject());
        QString path = ed.pathOf(id);
        const qint64 cap = 64LL * 1024 * 1024;
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);
        QSignalSpy ended(&ed, &EmbeddedEdit::watchEnded);
        QVERIFY(writeSized(path, cap));
        looks(ed, 2);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(1).toByteArray().size(), qsizetype(cap));
        QCOMPARE(ended.count(), 0);
        QVERIFY(writeSized(path, cap + 1));
        looks(ed, 1);
        QCOMPARE(ended.count(), 1);
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // A file that cannot be opened is tried again for several looks (the other program may hold it) and taken when it can; only a file that stays shut ends the watch as unreadable.
    void anUnreadableFileIsTriedAgain()
    {
#ifdef Q_OS_WIN
        QSKIP("permissions work differently here");
#endif
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject());
        const QString path = ed.pathOf(id);
        QSignalSpy spy(&ed, &EmbeddedEdit::objectSaved);
        QSignalSpy ended(&ed, &EmbeddedEdit::watchEnded);
        QVERIFY(writeFile(path, package("held then free")));
        QVERIFY(QFile::setPermissions(path, QFileDevice::Permissions()));
        if (QFile(path).open(QIODevice::ReadOnly)) QSKIP("this user can read a file with no permissions");
        looks(ed, EmbeddedEdit::kUnreadableLooks - 1);
        QCOMPARE(ended.count(), 0);
        QVERIFY(QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        looks(ed, 2);
        QCOMPARE(spy.count(), 1);
        QVERIFY(QFile::setPermissions(path, QFileDevice::Permissions()));
        looks(ed, EmbeddedEdit::kUnreadableLooks);
        QCOMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(1).value<EmbeddedEdit::EndReason>(), EmbeddedEdit::EndReason::Unreadable);
        QTRY_VERIFY(QDir(root.path()).isEmpty());
    }

    // The same owner and key start one edit; another key or owner starts another; each file has its own readable name; closing a document ends its edits, and so does destroying it.
    void editsBelongToTheirDocument()
    {
        QTemporaryDir root;
        EmbeddedEdit ed;
        int opened = 0;
        prepare(ed, root.path());
        ed.setOpener([&](const QString &, Kind) { ++opened; return true; });
        QObject docA, docB;
        EmbeddedObject o = sheetObject();
        int a1 = ed.start(&docA, QStringLiteral("one"), o);
        QVERIFY(a1);
        QCOMPARE(ed.start(&docA, QStringLiteral("one"), o), a1);
        QCOMPARE(opened, 1);
        int a2 = ed.start(&docA, QStringLiteral("two"), o);
        int b1 = ed.start(&docB, QStringLiteral("one"), o);
        QVERIFY(a2 && b1 && a2 != a1 && b1 != a1 && b1 != a2);
        QCOMPARE(ed.openCount(), 3);
        QSet<QString> names;
        for (int id : {a1, a2, b1}) {
            QString n = QFileInfo(ed.pathOf(id)).fileName();
            QVERIFY2(QRegularExpression(QStringLiteral("^Object \\d+\\.xlsx$")).match(n).hasMatch(), qPrintable(n));
            names.insert(n);
        }
        QCOMPARE(names.size(), 3);
        QCOMPARE(ed.start(nullptr, QStringLiteral("x"), o), 0);

        ed.closeFor(&docA);
        QVERIFY(!ed.isOpen(a1) && !ed.isOpen(a2) && ed.isOpen(b1));
        QCOMPARE(QDir(root.path()).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size(), 1);
        QObject *doomed = new QObject;
        int d = ed.start(doomed, QStringLiteral("k"), o);
        QVERIFY(ed.isOpen(d));
        delete doomed;
        QVERIFY(!ed.isOpen(d));
        QVERIFY(ed.isOpen(b1));
        QCOMPARE(QDir(root.path()).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size(), 1);
    }

    // At most 16 objects are edited at once; a program that cannot be started leaves no temp file; closeAll and destruction clear the folder.
    void tempFilesAreBoundedAndAlwaysDeleted()
    {
        QTemporaryDir root;
        QObject owner;
        {
            EmbeddedEdit ed;
            prepare(ed, root.path());
            ed.setOpener([](const QString &, Kind) { return false; });
            QCOMPARE(ed.start(&owner, QStringLiteral("x"), sheetObject()), 0);
            QVERIFY(QDir(root.path()).isEmpty());
            ed.setOpener([](const QString &, Kind) { return true; });
            QSet<int> ids;
            for (int i = 0; i < EmbeddedEdit::kMaxOpen; ++i) {
                int id = ed.start(&owner, QString::number(i), sheetObject());
                QVERIFY(id != 0);
                ids.insert(id);
            }
            QCOMPARE(ids.size(), EmbeddedEdit::kMaxOpen);
            QCOMPARE(ed.start(&owner, QStringLiteral("17"), sheetObject()), 0);
            QCOMPARE(ed.openCount(), EmbeddedEdit::kMaxOpen);
            QCOMPARE(QDir(root.path()).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size(), EmbeddedEdit::kMaxOpen);
            ed.close(*ids.begin());
            QVERIFY(ed.start(&owner, QStringLiteral("18"), sheetObject()) != 0);
            // destroyed here with 16 open
        }
        QVERIFY(QDir(root.path()).isEmpty());
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QVERIFY(ed.start(&owner, QStringLiteral("y"), sheetObject()));
        ed.closeAll();
        QCOMPARE(ed.openCount(), 0);
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // A folder that cannot be deleted when its edit ends (the other program holds it) is deleted at the next start, and at closeAll.
    void aFolderThatCannotBeDeletedIsTriedAgain()
    {
#ifdef Q_OS_WIN
        QSKIP("permissions work differently here");
#endif
        QTemporaryDir root;
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        int id = ed.start(&owner, QStringLiteral("k"), sheetObject());
        const QString dir = QFileInfo(ed.pathOf(id)).absolutePath();
        QVERIFY(QFile::setPermissions(dir, QFileDevice::ReadOwner | QFileDevice::ExeOwner));   // nothing in it can be removed
        if (QFileInfo(dir).isWritable()) {
            QFile::setPermissions(dir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
            QSKIP("this user can delete from a read-only folder");
        }
        ed.close(id);
        QVERIFY(QFileInfo::exists(dir));
        QVERIFY(QFile::setPermissions(dir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
        int next = ed.start(&owner, QStringLiteral("k2"), sheetObject());
        QVERIFY(next);
        QVERIFY(!QFileInfo::exists(dir));
        ed.closeAll();
        QVERIFY(QDir(root.path()).isEmpty());
    }

    // Folders a crash left behind are removed when they are older than a day and belong to this user; newer ones and other names stay. (The "old" folder is made old by the cutoff.)
    void staleFoldersAreSwept()
    {
        QTemporaryDir root;
        QDir r(root.path());
        QVERIFY(r.mkpath(QStringLiteral("jo-embed-AAAAAA")));
        QVERIFY(writeFile(r.filePath(QStringLiteral("jo-embed-AAAAAA/Object 1.xlsx")), package("x")));
        QVERIFY(r.mkpath(QStringLiteral("other-folder")));
        QCOMPARE(EmbeddedEdit::sweepStale(root.path()), 0);   // a new one stays
        QVERIFY(r.exists(QStringLiteral("jo-embed-AAAAAA")));
        QCOMPARE(EmbeddedEdit::sweepStale(root.path(), QDateTime::currentDateTime().addDays(1)), 1);   // two days later it is old
        QVERIFY(!r.exists(QStringLiteral("jo-embed-AAAAAA")));
        QVERIFY(r.exists(QStringLiteral("other-folder")));
    }

    // An old folder whose lock another running copy holds is not swept; an old unlocked one is.
    void aFolderWithAHeldLockSurvivesTheSweep()
    {
        QTemporaryDir root;
        QDir r(root.path());
        QVERIFY(r.mkpath(QStringLiteral("jo-embed-LIVE00")));
        QVERIFY(r.mkpath(QStringLiteral("jo-embed-DEAD00")));
        QLockFile held(r.filePath(QStringLiteral("jo-embed-LIVE00/lock")));
        QVERIFY(held.tryLock(0));
        const QDateTime later = QDateTime::currentDateTime().addDays(2);
        QCOMPARE(EmbeddedEdit::sweepStale(root.path(), later), 1);
        QVERIFY(r.exists(QStringLiteral("jo-embed-LIVE00")));
        QVERIFY(!r.exists(QStringLiteral("jo-embed-DEAD00")));
        held.unlock();
        QCOMPARE(EmbeddedEdit::sweepStale(root.path(), later), 1);
        QVERIFY(!r.exists(QStringLiteral("jo-embed-LIVE00")));
    }

    // The first start in a folder sweeps it: a folder of this user's, made two days old on disk, is gone after start.
    void startSweepsAnOldFolder()
    {
#ifndef Q_OS_UNIX
        QSKIP("sets a folder's time through the C library");
#else
        QTemporaryDir root;
        QDir r(root.path());
        QVERIFY(r.mkpath(QStringLiteral("jo-embed-OLDOLD")));
        QVERIFY(r.mkpath(QStringLiteral("jo-embed-NEWNEW")));
        struct timeval old[2];
        old[0].tv_sec = old[1].tv_sec = time(nullptr) - 2 * 24 * 3600;
        old[0].tv_usec = old[1].tv_usec = 0;
        QVERIFY(utimes(r.filePath(QStringLiteral("jo-embed-OLDOLD")).toLocal8Bit().constData(), old) == 0);
        EmbeddedEdit ed;
        prepare(ed, root.path());
        QObject owner;
        QVERIFY(ed.start(&owner, QStringLiteral("k"), sheetObject()));
        QVERIFY(!r.exists(QStringLiteral("jo-embed-OLDOLD")));
        QVERIFY(r.exists(QStringLiteral("jo-embed-NEWNEW")));
#endif
    }
};

JP_TEST_CLASS(PubEmbedObjectTests)
#include "tst_pub_embedobj.moc"
