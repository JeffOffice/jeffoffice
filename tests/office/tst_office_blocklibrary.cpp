// Tests of libs/base/io/blocklibrary: the library of saved blocks (the file, merging two writers, the limits on a hostile file).
#include "officehelpers.h"
#include "io/blocklibrary.h"
#include "io/zip.h"

#include <QCoreApplication>
#include <cstdio>
#include <QFile>
#include <QProcess>
#include <QThread>
#include <QSemaphore>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

using namespace jp;

namespace {

BlockEntry entryOf(const QString &name, const QByteArray &data, const QString &gallery = kBlocksGallery)
{
    BlockEntry e;
    e.info.name = name;
    e.info.gallery = gallery;
    e.info.format = QStringLiteral("docx");
    e.data = data;
    return e;
}

QString idOf(int n) { return QStringLiteral("%1").arg(n, 32, 16, QLatin1Char('0')); }

// A library file made by hand, so that a test can put in what no program would write.
QByteArray handMade(const QJsonArray &entries, const QMap<QString, QByteArray> &files, int version = 1)
{
    QJsonObject root;
    root.insert(QStringLiteral("version"), version);
    root.insert(QStringLiteral("entries"), entries);
    ZipWriter z;
    z.add(QStringLiteral("library.json"), QJsonDocument(root).toJson(QJsonDocument::Compact), true);
    for (auto it = files.cbegin(); it != files.cend(); ++it) z.add(it.key(), it.value(), false);
    return z.finish();
}

QJsonObject listing(const QString &id, const QString &name)
{
    return QJsonObject{{QStringLiteral("id"), id}, {QStringLiteral("name"), name}, {QStringLiteral("gallery"), kBlocksGallery}, {QStringLiteral("format"), QStringLiteral("docx")}};
}

bool put(const QString &path, const QByteArray &bytes)
{
    QFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(bytes) == bytes.size();
}

// Clears the test hooks of the library however a test ends.
struct HookGuard {
    ~HookGuard()
    {
        BlockLibrary::setLoaderHook(nullptr);
        BlockLibrary::setWriteStepHook(nullptr);
    }
};

QByteArray noiseOf(int size)
{
    QByteArray noise(size, 'a');
    for (int i = 0; i < noise.size(); ++i) noise[i] = char((quint32(i) * 7919u + quint32(i >> 3) * 31u) & 0xFFu);
    return noise;
}

// A library file with `count` entries of about half a megabyte each, so that it is read on a thread of its own.
void fillLarge(const QString &path, int count)
{
    const QByteArray noise = noiseOf(500000);
    BlockLibrary a(path);
    QString err;
    for (int i = 0; i < count; ++i) {
        QByteArray data = noise;
        data[0] = char(i);
        QVERIFY2(a.add(entryOf(QStringLiteral("Large %1").arg(i), data), nullptr, &err), qPrintable(err));
    }
    QVERIFY(QFileInfo(path).size() >= BlockLibrary::kBackgroundBytes);
}

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

} // namespace

class BlockLibraryTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void entriesComeBackFromTheFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("sub/library.jblocks"));
        BlockLibrary a(path);
        QString id, err;
        BlockEntry e = entryOf(QStringLiteral("Signature"), QByteArray("one"));
        e.info.category = QStringLiteral("Letters");
        e.info.description = QStringLiteral("Closing lines");
        e.info.insertAs = BlockInsert::Paragraph;
        QVERIFY2(a.add(e, &id, &err), qPrintable(err));
        QCOMPARE(id.size(), 32);
        BlockLibrary b(path);
        QVERIFY(b.load());
        QCOMPARE(b.list().size(), 1);
        const BlockInfo i = b.list().first();
        QCOMPARE(i.name, QStringLiteral("Signature"));
        QCOMPARE(i.category, QStringLiteral("Letters"));
        QCOMPARE(i.description, QStringLiteral("Closing lines"));
        QVERIFY(i.insertAs == BlockInsert::Paragraph);
        QCOMPARE(i.size, qint64(3));
        QCOMPARE(b.data(id), QByteArray("one"));
    }

    void namesAreUniquePerGallery()
    {
        QTemporaryDir dir;
        BlockLibrary a(dir.filePath(QStringLiteral("l.jblocks")));
        QString err;
        QVERIFY(a.add(entryOf(QStringLiteral("Hello"), "1"), nullptr, &err));
        QVERIFY(!a.add(entryOf(QStringLiteral("hello"), "2"), nullptr, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(a.add(entryOf(QStringLiteral("hello"), "3", kShortcutsGallery), nullptr, &err));
        QVERIFY(a.add(entryOf(QStringLiteral("HELLO"), "4"), nullptr, &err, true));
        QCOMPARE(a.list().size(), 2);
        QCOMPARE(a.data(a.findByName(kBlocksGallery, QStringLiteral("Hello"))->id), QByteArray("4"));
        QVERIFY(!a.add(entryOf(QStringLiteral("  "), "5"), nullptr, &err));
        QVERIFY(!a.add(entryOf(QStringLiteral("Empty"), QByteArray()), nullptr, &err));
        QVERIFY(!a.add(entryOf(QStringLiteral("Big"), QByteArray(int(BlockLibrary::kMostEntryBytes) + 1, 'x')), nullptr, &err));
    }

    void updateAndRemove()
    {
        QTemporaryDir dir;
        BlockLibrary a(dir.filePath(QStringLiteral("l.jblocks")));
        QString idA, idB, err;
        QVERIFY(a.add(entryOf(QStringLiteral("A"), "a"), &idA, &err));
        QVERIFY(a.add(entryOf(QStringLiteral("B"), "b"), &idB, &err));
        BlockInfo i = *a.find(idA);
        i.name = QStringLiteral("B");
        QVERIFY(!a.update(i, &err));   // (taken)
        i.name = QStringLiteral("Renamed");
        i.insertAs = BlockInsert::Page;
        QVERIFY(a.update(i, &err));
        BlockLibrary b(a.path());
        QVERIFY(b.load());
        QCOMPARE(b.find(idA)->name, QStringLiteral("Renamed"));
        QVERIFY(b.find(idA)->insertAs == BlockInsert::Page);
        QVERIFY(b.remove({idA}, &err));
        QCOMPARE(b.list().size(), 1);
        BlockLibrary c(a.path());
        QVERIFY(c.load());
        QCOMPARE(c.list().size(), 1);
        QVERIFY(c.data(idA).isEmpty());
        QCOMPARE(c.data(idB), QByteArray("b"));
    }

    // Two windows that loaded the file before either changed it: the second one's change is made to what the first one wrote.
    void twoWritersLoseNothing()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        BlockLibrary a(path), b(path);
        QVERIFY(a.load());
        QVERIFY(b.load());
        QString err, idA;
        QVERIFY(a.add(entryOf(QStringLiteral("From A"), "a"), &idA, &err));
        QVERIFY(b.add(entryOf(QStringLiteral("From B"), "b"), nullptr, &err));
        QCOMPARE(b.list().size(), 2);
        QVERIFY(a.remove({idA}, &err));   // (a removes what it added: b's stays)
        BlockLibrary c(path);
        QVERIFY(c.load());
        QCOMPARE(c.list().size(), 1);
        QCOMPARE(c.list().first().name, QStringLiteral("From B"));
    }

    void aFileThatIsNoLibraryIsPutAside()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        QVERIFY(put(path, QByteArray("this is not a zip file at all")));
        BlockLibrary a(path);
        QString err;
        QVERIFY(!a.load(&err));
        QVERIFY(!err.isEmpty());
        QCOMPARE(a.list().size(), 0);
        QVERIFY(a.add(entryOf(QStringLiteral("New"), "n"), nullptr, &err));
        QVERIFY(QFileInfo::exists(path + QStringLiteral(".damaged")));
        BlockLibrary b(path);
        QVERIFY(b.load());
        QCOMPARE(b.list().size(), 1);
    }

    void aNewerFileIsNotWrittenOver()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        const QByteArray bytes = handMade({listing(idOf(1), QStringLiteral("Future"))}, {{QStringLiteral("e/") + idOf(1) + QStringLiteral(".bin"), "x"}}, BlockLibrary::kVersion + 1);
        QVERIFY(put(path, bytes));
        BlockLibrary a(path);
        QVERIFY(a.load());
        QCOMPARE(a.list().size(), 1);
        QString err;
        QVERIFY(!a.add(entryOf(QStringLiteral("More"), "m"), nullptr, &err));
        QVERIFY(!a.remove({idOf(1)}, &err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll(), bytes);
    }

    // What a file can multiply is bounded where it is read: the count, a name's length, an id that is no id, an entry with no bytes or too many.
    void aHostileFileIsReadWithinItsLimits()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        QJsonArray list;
        QMap<QString, QByteArray> files;
        const int count = BlockLibrary::kMostEntries + 500;
        for (int i = 1; i <= count; ++i) {
            list.append(listing(idOf(i), QStringLiteral("Block %1").arg(i)));
            files.insert(QStringLiteral("e/") + idOf(i) + QStringLiteral(".bin"), "x");
        }
        QVERIFY(put(path, handMade(list, files)));
        BlockLibrary a(path);
        QVERIFY(a.load());
        QCOMPARE(a.list().size(), BlockLibrary::kMostEntries);

        QJsonArray odd;
        QMap<QString, QByteArray> oddFiles;
        odd.append(listing(QStringLiteral("../../evil"), QStringLiteral("Path")));
        oddFiles.insert(QStringLiteral("e/../../evil.bin"), "x");
        odd.append(listing(idOf(2), QString(5000, QLatin1Char('n'))));
        oddFiles.insert(QStringLiteral("e/") + idOf(2) + QStringLiteral(".bin"), "x");
        odd.append(listing(idOf(3), QStringLiteral("No bytes")));
        odd.append(listing(idOf(4), QStringLiteral("Too big")));
        oddFiles.insert(QStringLiteral("e/") + idOf(4) + QStringLiteral(".bin"), QByteArray(int(BlockLibrary::kMostEntryBytes) + 1, 'x'));
        odd.append(listing(idOf(5), QStringLiteral("Fine")));
        oddFiles.insert(QStringLiteral("e/") + idOf(5) + QStringLiteral(".bin"), "ok");
        odd.append(listing(idOf(5), QStringLiteral("Same id again")));
        odd.append(QJsonValue(7));
        QVERIFY(put(path, handMade(odd, oddFiles)));
        BlockLibrary b(path);
        QVERIFY(b.load());
        QCOMPARE(b.list().size(), 2);
        QCOMPARE(b.find(idOf(2))->name.size(), BlockLibrary::kMostNameChars);
        QCOMPARE(b.data(idOf(5)), QByteArray("ok"));
    }

    void aDeeplyNestedManifestIsRefused()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        ZipWriter z;
        z.add(QStringLiteral("library.json"), QByteArray(200000, '[') + QByteArray(200000, ']'), true);
        QVERIFY(put(path, z.finish()));
        BlockLibrary a(path);
        QString err;
        QVERIFY(!a.load(&err));
        QCOMPARE(a.list().size(), 0);
    }

    void exportAndImportCarryEntries()
    {
        QTemporaryDir dir;
        BlockLibrary a(dir.filePath(QStringLiteral("a.jblocks")));
        QString idA, err;
        QVERIFY(a.add(entryOf(QStringLiteral("One"), "1"), &idA, &err));
        QVERIFY(a.add(entryOf(QStringLiteral("Two"), "2"), nullptr, &err));
        QVERIFY(a.exportTo(dir.filePath(QStringLiteral("some.jblocks")), {idA}, &err));
        QVERIFY(a.exportTo(dir.filePath(QStringLiteral("all.jblocks")), {}, &err));
        BlockLibrary b(dir.filePath(QStringLiteral("b.jblocks")));
        QVERIFY(b.add(entryOf(QStringLiteral("two"), "mine"), nullptr, &err));
        int skipped = 0;
        QCOMPARE(b.importFrom(dir.filePath(QStringLiteral("all.jblocks")), &err, &skipped), 1);
        QCOMPARE(skipped, 1);
        QCOMPARE(b.list().size(), 2);
        QCOMPARE(b.data(b.findByName(kBlocksGallery, QStringLiteral("One"))->id), QByteArray("1"));
        QCOMPARE(b.data(b.findByName(kBlocksGallery, QStringLiteral("Two"))->id), QByteArray("mine"));
        QCOMPARE(b.importFrom(dir.filePath(QStringLiteral("nothing.jblocks")), &err), -1);
        QVERIFY(put(dir.filePath(QStringLiteral("junk.jblocks")), "junk"));
        QCOMPARE(b.importFrom(dir.filePath(QStringLiteral("junk.jblocks")), &err), -1);
        QCOMPARE(b.list().size(), 2);
    }

    // A large file is read on a thread of its own; the answer comes to the thread that asked.
    void aLargeFileLoadsOnAThread()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        fillLarge(path, 6);
        BlockLibrary b(path);
        int answers = 0;
        bool ok = false;
        b.loadInBackground(this, [&](bool good) {
            ++answers;
            ok = good;
        });
        QTRY_COMPARE(answers, 1);
        QVERIFY(ok);
        QVERIFY(!b.loading());
        QCOMPARE(b.list().size(), 6);
        // (A small file is answered at once, on the caller's own thread.)
        BlockLibrary c(dir.filePath(QStringLiteral("none.jblocks")));
        int small = 0;
        c.loadInBackground(this, [&](bool) { ++small; });
        QCOMPARE(small, 1);
    }

    // The window that asked is gone before the file is read: the library is not left "loading" for good, and the next window gets the file.
    void aLoadWhoseAskerIsGoneEnds()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        fillLarge(path, 6);
        HookGuard guard;
        QSemaphore go;
        BlockLibrary::setLoaderHook([&] { go.tryAcquire(1, 60000); });   // (holds the loader until the asker is gone; the wait only guards a hang)
        BlockLibrary b(path);
        QObject *asker = new QObject;
        int answers = 0;
        b.loadInBackground(asker, [&](bool) { ++answers; });
        QVERIFY(b.loading());
        delete asker;
        go.release();
        QTRY_VERIFY(!b.loading());
        QCOMPARE(answers, 0);
        BlockLibrary::setLoaderHook(nullptr);
        QObject next;
        int nextAnswers = 0;
        b.loadInBackground(&next, [&](bool) { ++nextAnswers; });
        QTRY_COMPARE(nextAnswers, 1);
        QCOMPARE(b.list().size(), 6);
    }

    // A change made while a load is on its way is not undone by the load's late answer.
    void aChangeBeatsALateLoad()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        fillLarge(path, 6);
        HookGuard guard;
        QSemaphore read;
        BlockLibrary::setLoaderHook([&] { read.release(); });
        BlockLibrary b(path);
        int answers = 0;
        b.loadInBackground(this, [&](bool) { ++answers; });
        QVERIFY(read.tryAcquire(1, 60000));   // (the loader has read the six entries and hands them over when this thread next handles events)
        QString err;
        QVERIFY2(b.add(entryOf(QStringLiteral("Seventh"), "7"), nullptr, &err), qPrintable(err));
        QVERIFY(!b.loading());
        QTRY_COMPARE(answers, 1);
        QCOMPARE(b.list().size(), 7);
        BlockLibrary again(path);
        QVERIFY(again.load());
        QCOMPARE(again.list().size(), 7);
    }

#ifndef Q_OS_WIN
    // A file that cannot be read is no proof that it is no library: the change is refused and the file is left exactly as it is.
    void anUnreadableLibraryIsNeverPutAside()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        {
            BlockLibrary a(path);
            QString err;
            QVERIFY(a.add(entryOf(QStringLiteral("Keep"), "k"), nullptr, &err));
        }
        const QByteArray before = fileBytes(path);
        QVERIFY(!before.isEmpty());
        QVERIFY(QFile::setPermissions(path, QFileDevice::Permissions()));
        if (QFile(path).open(QIODevice::ReadOnly)) {
            QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
            QSKIP("this account can read files with no permissions");
        }
        BlockLibrary b(path);
        QString err;
        QVERIFY(!b.add(entryOf(QStringLiteral("New"), "n"), nullptr, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(!QFileInfo::exists(path + QStringLiteral(".damaged")));
        QVERIFY(QFileInfo::exists(path));
        QVERIFY(QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        QCOMPARE(fileBytes(path), before);
        BlockLibrary c(path);
        QVERIFY(c.load());
        QCOMPARE(c.list().size(), 1);
    }

    // A folder the program may not write in is said so plainly, not as "another window".
    void aReadOnlyFolderIsSaidPlainly()
    {
        QTemporaryDir dir;
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("ro")));
        const QString folder = dir.filePath(QStringLiteral("ro"));
        QVERIFY(QFile::setPermissions(folder, QFileDevice::ReadOwner | QFileDevice::ExeOwner));
        if (QFileInfo(folder).isWritable()) {
            QFile::setPermissions(folder, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
            QSKIP("this account can write in read-only folders");
        }
        BlockLibrary a(folder + QStringLiteral("/l.jblocks"));
        QString err;
        const bool added = a.add(entryOf(QStringLiteral("New"), "n"), nullptr, &err);
        QFile::setPermissions(folder, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        QVERIFY(!added);
        QVERIFY2(err.contains(QLatin1String("does not let")) && !err.contains(QLatin1String("Another window")), qPrintable(err));
    }
#endif

    // A file that is no library is put aside under a number, and an earlier one that was put aside is never deleted.
    void putAsideFilesAreNeverDeleted()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        QString err;
        for (int round = 1; round <= 3; ++round) {
            QVERIFY(put(path, QByteArray("junk number ") + QByteArray::number(round)));
            BlockLibrary a(path);
            QVERIFY2(a.add(entryOf(QStringLiteral("Block %1").arg(round), "b"), nullptr, &err), qPrintable(err));
            QVERIFY(QFile::remove(path));   // (the next round starts from junk again)
        }
        QCOMPARE(fileBytes(path + QStringLiteral(".damaged")), QByteArray("junk number 1"));
        QCOMPARE(fileBytes(path + QStringLiteral(".damaged-2")), QByteArray("junk number 2"));
        QCOMPARE(fileBytes(path + QStringLiteral(".damaged-3")), QByteArray("junk number 3"));
        // (When every number is taken, the change is refused and nothing is touched.)
        for (int n = 4; n <= BlockLibrary::kMostAsideFiles; ++n) QVERIFY(put(path + QStringLiteral(".damaged-%1").arg(n), "old"));
        QVERIFY(put(path, QByteArray("the last junk")));
        BlockLibrary b(path);
        QVERIFY(!b.add(entryOf(QStringLiteral("Last"), "l"), nullptr, &err));
        QVERIFY(!err.isEmpty());
        QCOMPARE(fileBytes(path), QByteArray("the last junk"));
        QCOMPARE(fileBytes(path + QStringLiteral(".damaged")), QByteArray("junk number 1"));
    }

    // The manifest is the first part of the file, and a file with it last (an older version's) is read as before.
    void theManifestComesFirstAndOldFilesStillRead()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        BlockLibrary a(path);
        QString err;
        QVERIFY(a.add(entryOf(QStringLiteral("One"), "1"), nullptr, &err));
        const QVector<ZipScanEntry> parts = scanZip(fileBytes(path));
        QVERIFY(parts.size() == 2);
        QCOMPARE(parts.first().name, QStringLiteral("library.json"));

        QJsonObject root{{QStringLiteral("version"), 1}, {QStringLiteral("entries"), QJsonArray{listing(idOf(9), QStringLiteral("Old order"))}}};
        ZipWriter z;
        z.add(QStringLiteral("e/") + idOf(9) + QStringLiteral(".bin"), "old", false);
        z.add(QStringLiteral("library.json"), QJsonDocument(root).toJson(QJsonDocument::Compact), true);
        QVERIFY(put(path, z.finish()));
        BlockLibrary b(path);
        QVERIFY(b.load());
        QCOMPARE(b.list().size(), 1);
        QCOMPARE(b.data(idOf(9)), QByteArray("old"));
    }

    // Fields this version does not know, in the manifest and in an entry, are still there after it has written the file again.
    void unknownFieldsSurviveARewrite()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        QJsonObject entry = listing(idOf(1), QStringLiteral("Future"));
        entry.insert(QStringLiteral("color"), QStringLiteral("red"));
        QJsonObject root{{QStringLiteral("version"), 1}, {QStringLiteral("entries"), QJsonArray{entry}}, {QStringLiteral("sync"), QJsonObject{{QStringLiteral("id"), 7}}}};
        ZipWriter z;
        z.add(QStringLiteral("library.json"), QJsonDocument(root).toJson(QJsonDocument::Compact), true);
        z.add(QStringLiteral("e/") + idOf(1) + QStringLiteral(".bin"), "x", false);
        QVERIFY(put(path, z.finish()));
        BlockLibrary a(path);
        QString err;
        QVERIFY2(a.add(entryOf(QStringLiteral("More"), "m"), nullptr, &err), qPrintable(err));
        QJsonObject now;
        for (const ZipScanEntry &e : scanZip(fileBytes(path)))
            if (e.name == QLatin1String("library.json")) now = QJsonDocument::fromJson(e.data).object();
        QCOMPARE(now.value(QStringLiteral("sync")).toObject().value(QStringLiteral("id")).toInt(), 7);
        const QJsonArray list = now.value(QStringLiteral("entries")).toArray();
        QCOMPARE(list.size(), 2);
        QCOMPARE(list.first().toObject().value(QStringLiteral("color")).toString(), QStringLiteral("red"));
    }

    // The limit on one entry: exactly the most is taken, one byte more is refused, and the file is not touched by the refusal.
    void thePerEntryLimitIsKept()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        BlockLibrary a(path);
        QString err;
        QVERIFY2(a.add(entryOf(QStringLiteral("Most"), QByteArray(int(BlockLibrary::kMostEntryBytes), 'x')), nullptr, &err), qPrintable(err));
        const QByteArray before = fileBytes(path);
        QVERIFY(!a.add(entryOf(QStringLiteral("Over"), QByteArray(int(BlockLibrary::kMostEntryBytes) + 1, 'x')), nullptr, &err));
        QVERIFY(!err.isEmpty());
        QCOMPARE(fileBytes(path), before);
        BlockLibrary b(path);
        QVERIFY(b.load());
        QCOMPARE(b.list().size(), 1);
        QCOMPARE(b.list().first().size, BlockLibrary::kMostEntryBytes);
    }

    // The limit on all the bytes: a library filled to exactly the most reads back whole (the manifest does not count against it), and one more byte is refused. (The limits are small for the test:
    // the same code, 4 blocks of 1,000 bytes, not 32 of 8 MB.)
    void theTotalLimitIsKept()
    {
        struct Small {
            Small() { BlockLibrary::setLimitsForTest(100, 1000, 4000); }
            ~Small() { BlockLibrary::setLimitsForTest(); }
        } small;
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        const int each = 1000;
        const int count = 4;
        {
            const QByteArray block(each, 'x');
            QJsonArray list;
            QMap<QString, QByteArray> files;
            for (int i = 1; i < count; ++i) {
                list.append(listing(idOf(i), QStringLiteral("Block %1").arg(i)));
                files.insert(QStringLiteral("e/") + idOf(i) + QStringLiteral(".bin"), block);
            }
            QVERIFY(put(path, handMade(list, files)));
        }
        BlockLibrary a(path);
        QString err;
        QVERIFY2(a.add(entryOf(QStringLiteral("The last that fits"), QByteArray(each, 'y')), nullptr, &err), qPrintable(err));
        QCOMPARE(a.list().size(), count);
        const qint64 size = QFileInfo(path).size();
        const QDateTime stamp = QFileInfo(path).lastModified();
        {
            BlockLibrary b(path);
            QVERIFY2(b.load(&err), qPrintable(err));
            QCOMPARE(b.list().size(), count);
        }
        QVERIFY(!a.add(entryOf(QStringLiteral("One more"), "z"), nullptr, &err));
        QVERIFY(!err.isEmpty());
        QCOMPARE(QFileInfo(path).size(), size);
        QCOMPARE(QFileInfo(path).lastModified(), stamp);
    }

    // The limit on the manifest: names and descriptions that escape to many bytes fill it long before the entry count does; the add that would pass it is refused, and every earlier one reads back.
    // (A manifest of 64 KB, and at most 40 blocks: with the check off the 40th is added and the test fails at once.)
    void theManifestLimitIsKept()
    {
        struct Small {
            Small() { BlockLibrary::setLimitsForTest(40, 0, 0, qint64(64) << 10); }
            ~Small() { BlockLibrary::setLimitsForTest(); }
        } small;
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        BlockLibrary a(path);
        QString err;
        const QString heavy(BlockLibrary::kMostTextChars, QChar(1));   // (each is written as \u0001: six bytes)
        int added = 0;
        bool refused = false;
        for (int i = 0; i < 40; ++i) {
            BlockEntry e = entryOf(QStringLiteral("Block %1").arg(i), "x");
            e.info.category = heavy;
            e.info.description = heavy;
            if (!a.add(e, nullptr, &err)) {
                refused = true;
                break;
            }
            ++added;
        }
        QVERIFY(refused);
        QVERIFY(!err.isEmpty());
        QVERIFY(added >= 2 && added < 40);
        BlockLibrary b(path);
        QVERIFY2(b.load(&err), qPrintable(err));
        QCOMPARE(b.list().size(), added);
        QVERIFY(!QFileInfo::exists(path + QStringLiteral(".damaged")));
    }

    // Three programs that add at the same time lose none of each other's entries (real processes: the same test program, run as a helper).
    void writersInSeparateProcessesLoseNothing()
    {
        if (qgetenv("JP_BLOCKS_HELPER") == "writer") {
            BlockLibrary mine(qEnvironmentVariable("JP_BLOCKS_PATH"));
            const QString who = qEnvironmentVariable("JP_BLOCKS_WHO");
            for (int i = 0; i < 200; ++i) {
                // (A window that is told another is changing the blocks tries again, as a person is told to: on a slow machine three writers can keep one another waiting past the time one waits, and
                // on Windows the lock file that another writer is just taking away can refuse to open for a moment, which the library says as "does not let the program write". Each try is counted, and
                // a try that fails for any other reason ends the writer, with its reason left in a file for the test that started it: the output of a helper is lost on the build machines.)
                QString err;
                bool added = false;
                for (int tries = 0; tries < 2000 && !added; ++tries) {
                    err.clear();
                    added = mine.add(entryOf(QStringLiteral("%1-%2").arg(who).arg(i), QByteArray(300, 'w')), nullptr, &err);
                    if (added) break;
                    if (!err.contains(QStringLiteral("Another window")) && !err.contains(QStringLiteral("does not let the program write"))) break;
                    QThread::yieldCurrentThread();
                }
                if (!added) {
                    QFile why(qEnvironmentVariable("JP_BLOCKS_PATH") + QStringLiteral(".why-") + who);
                    if (why.open(QIODevice::WriteOnly)) why.write(QStringLiteral("entry %1: %2").arg(i).arg(err).toUtf8());
                }
                QVERIFY2(added, qPrintable(err));
            }
            return;
        }
        if (!qgetenv("JP_BLOCKS_HELPER").isEmpty()) return;
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        QProcess writers[3];
        for (int w = 0; w < 3; ++w) {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("JP_BLOCKS_HELPER"), QStringLiteral("writer"));
            env.insert(QStringLiteral("JP_BLOCKS_PATH"), path);
            env.insert(QStringLiteral("JP_BLOCKS_WHO"), QStringLiteral("w%1").arg(w));
            writers[w].setProcessEnvironment(env);
            writers[w].setProcessChannelMode(QProcess::MergedChannels);
            writers[w].start(QCoreApplication::applicationFilePath(), {QStringLiteral("--class"), QStringLiteral("BlockLibraryTests"), QStringLiteral("writersInSeparateProcessesLoseNothing")});
            QVERIFY(writers[w].waitForStarted(30000));
        }
        for (int w = 0; w < 3; ++w) {
            QVERIFY(writers[w].waitForFinished(1800000));   // (only so that a writer that hangs does not hang the test for ever: the writers' own tries are counted)
            QFile why(path + QStringLiteral(".why-w%1").arg(w));
            const QString reason = why.open(QIODevice::ReadOnly) ? QString::fromUtf8(why.readAll()) : QString::fromUtf8(writers[w].readAll().right(600));
            QVERIFY2(writers[w].exitCode() == 0, qPrintable(QStringLiteral("exit code %1, status %2: ").arg(writers[w].exitCode()).arg(int(writers[w].exitStatus())) + reason));
        }
        BlockLibrary all(path);
        QVERIFY(all.load());
        QCOMPARE(all.list().size(), 600);
        QVERIFY(!QFileInfo::exists(path + QStringLiteral(".damaged")));
    }

    // A program ended in the middle of writing the library, at counted points, leaves the old file as it was (the new one is made apart and put in place whole).
    void aWriterEndedMidWriteLeavesTheOldFile()
    {
        if (qgetenv("JP_BLOCKS_HELPER") == "stopper") {
            BlockLibrary mine(qEnvironmentVariable("JP_BLOCKS_PATH"));
            const int stopAt = qEnvironmentVariable("JP_BLOCKS_STOP_AT").toInt();
            QVERIFY(mine.load());
            BlockLibrary::setWriteStepHook([stopAt](int step) {
                if (step != stopAt) return;
                std::printf("AT %d\n", step);
                std::fflush(stdout);
                while (std::fgetc(stdin) != EOF) {}   // (until the parent ends this process)
            });
            HookGuard guard;
            QString err;
            QVERIFY2(mine.add(entryOf(QStringLiteral("Second"), QByteArray(3 << 20, 'b')), nullptr, &err), qPrintable(err));
            return;
        }
        if (!qgetenv("JP_BLOCKS_HELPER").isEmpty()) return;
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("l.jblocks"));
        {
            BlockLibrary first(path);
            QString err;
            QVERIFY(first.add(entryOf(QStringLiteral("First"), QByteArray(3 << 20, 'a')), nullptr, &err));
        }
        const QByteArray before = fileBytes(path);
        const auto run = [&](int stopAt, QProcess &child) {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("JP_BLOCKS_HELPER"), QStringLiteral("stopper"));
            env.insert(QStringLiteral("JP_BLOCKS_PATH"), path);
            env.insert(QStringLiteral("JP_BLOCKS_STOP_AT"), QString::number(stopAt));
            child.setProcessEnvironment(env);
            child.setProcessChannelMode(QProcess::MergedChannels);
            child.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--class"), QStringLiteral("BlockLibraryTests"), QStringLiteral("aWriterEndedMidWriteLeavesTheOldFile")});
            return child.waitForStarted(30000);
        };
        for (int stopAt : {1, 3, 5}) {
            QProcess child;
            QVERIFY(run(stopAt, child));
            QByteArray seen;
            const QByteArray want = "AT " + QByteArray::number(stopAt);
            for (int i = 0; i < 1200 && !seen.contains(want) && child.state() != QProcess::NotRunning; ++i) {   // (bounded by count: each wait is 100 ms at most and ends when there is output)
                child.waitForReadyRead(100);
                seen += child.readAll();
            }
            QVERIFY2(seen.contains(want), seen.constData());
            const qint64 pid = child.processId();
            QVERIFY(pid > 0);
            child.kill();   // (this process's own child)
            QVERIFY(child.waitForFinished(30000));
            QCOMPARE(fileBytes(path), before);
            BlockLibrary now(path);
            QVERIFY(now.load());
            QCOMPARE(now.list().size(), 1);
        }
        QProcess whole;
        QVERIFY(run(0, whole));   // (never stops: the new file is whole, and the lock the ended writers left does not hold it up)
        QVERIFY(whole.waitForFinished(120000));
        QCOMPARE(whole.exitCode(), 0);
        BlockLibrary after(path);
        QVERIFY(after.load());
        QCOMPARE(after.list().size(), 2);
    }

    void theSharedLibraryFollowsItsPath()
    {
        QTemporaryDir dir;
        BlockLibrary::setSharedPath(dir.filePath(QStringLiteral("shared.jblocks")));
        QString err;
        QVERIFY(BlockLibrary::shared().add(entryOf(QStringLiteral("Mine"), "m"), nullptr, &err));
        QCOMPARE(BlockLibrary::shared().path(), dir.filePath(QStringLiteral("shared.jblocks")));
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("shared.jblocks"))));
        BlockLibrary::setSharedPath(QString());
        QVERIFY(BlockLibrary::defaultPath().contains(QStringLiteral("Saved Blocks")));
        QVERIFY(BlockLibrary::shared().path() != dir.filePath(QStringLiteral("shared.jblocks")));
    }
};

JP_TEST_CLASS(BlockLibraryTests)
#include "tst_office_blocklibrary.moc"
