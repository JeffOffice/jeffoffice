// Tests of libs/base/io/blocklibrary: the library of saved blocks (the file, merging two writers, the limits on a hostile file).
#include "officehelpers.h"
#include "io/blocklibrary.h"
#include "io/zip.h"

#include <QFile>
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
        QByteArray noise(500000, 'a');
        for (int i = 0; i < noise.size(); ++i) noise[i] = char((i * 7919 + (i >> 3) * 31) & 0xFF);
        BlockLibrary a(path);
        QString err;
        for (int i = 0; i < 6; ++i) {
            QByteArray data = noise;
            data[0] = char(i);
            QVERIFY(a.add(entryOf(QStringLiteral("Large %1").arg(i), data), nullptr, &err));
        }
        QVERIFY(QFileInfo(path).size() >= BlockLibrary::kBackgroundBytes);
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
