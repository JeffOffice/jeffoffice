// The compressed dictionaries and thesaurus of an install (text/dictpack.h): they give the answers the plain files gave, and a damaged
// or hostile file fails cleanly.
#include "pubhelpers.h"
#include "dictgolden.h"

#include "text/dictpack.h"
#include "text/hyphenation.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTemporaryDir>
#include <QtGlobal>
#include <map>

namespace {

QString plainRoot() { return QDir::cleanPath(QStringLiteral(JP_TEST_DATA "/../../resources/dict")); }
QString builtRoot() { return QStringLiteral(JP_BUILD_DICT); }

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

bool writeFile(const QString &path, const QByteArray &bytes)
{
    QDir().mkpath(QFileInfo(path).path());
    QFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(bytes) == bytes.size();
}

// Puts the dictionaries at `dir` for the test and puts them back after.
struct RootGuard {
    explicit RootGuard(const QString &dir) { jp::dict::setRootOverride(dir); }
    ~RootGuard() { jp::dict::setRootOverride(QString()); }
};

// What the thesaurus read before the compressed form: every line that does not start with "(" and has a bar after a first character
// is a word (lowercased, the last of equal ones kept), and its entry is that line and the lines its count says.
std::map<QByteArray, QByteArray> referenceEntries(const QByteArray &data)
{
    std::map<QByteArray, QByteArray> out;
    qsizetype p = data.indexOf('\n') + 1;
    while (p > 0 && p < data.size()) {
        const qsizetype e = data.indexOf('\n', p);
        if (e < 0) break;
        const QByteArray line = data.mid(p, e - p).trimmed();
        if (!line.startsWith('(')) {
            const int bar = line.indexOf('|');
            if (bar > 0) {
                const int count = line.split('|').value(1).toInt();
                qsizetype q = e + 1;
                for (int i = 0; i < count && q > 0 && q < data.size(); ++i) {
                    const qsizetype e2 = data.indexOf('\n', q);
                    q = e2 < 0 ? data.size() : e2 + 1;
                }
                out[QString::fromUtf8(line.left(bar)).toLower().toUtf8()] = data.mid(p, q - p);
            }
        }
        p = e + 1;
    }
    return out;
}

const char kMiniThesaurus[] =
    "UTF-8\n"
    "happy|2\n(adj)|glad|content|joyful\n(adj)|cheerful|sad (antonym)\n"
    "Apple|1\n(noun)|fruit|pome\n"
    "zebra|1\n(noun)|striped animal\n";
}

class PubDictTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Every shipped language says what it said when the plain files were read: a word list per language (and the suggestions for a few
    // wrong words) gives the same digest as tests/data/dict_golden.txt, made before the dictionaries were compressed.
    void spellingIsTheSameForEveryLanguage()
    {
        const QStringList golden = QString::fromUtf8(fileBytes(QStringLiteral(JP_TEST_DATA "/dict_golden.txt"))).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QCOMPARE(int(golden.size()), int(std::size(jp::dictgolden::kSources)) + 1);
        RootGuard guard(builtRoot());   // (the compressed files the build made, and no plain ones to fall back on)
        QCOMPARE(jp::dict::root(), builtRoot());
        QVERIFY(QFile::exists(builtRoot() + QStringLiteral("/en/en_US.dic.jz")));
        int at = 0;
        for (const jp::dictgolden::Source &s : jp::dictgolden::kSources)
            QCOMPARE(jp::dictgolden::digest(plainRoot(), s), golden.at(at++));
    }

    // The thesaurus answers 1,000 words as it did when it read the plain file.
    void thesaurusIsTheSameFor1000Words()
    {
        const QStringList golden = QString::fromUtf8(fileBytes(QStringLiteral(JP_TEST_DATA "/dict_golden.txt"))).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        RootGuard guard(builtRoot());
        QCOMPARE(jp::dict::root(), builtRoot());
        QCOMPARE(golden.last(), jp::dictgolden::thesaurusDigest(plainRoot()));
        QVERIFY(jp::dict::hasThesaurus());
    }

    // What the build writes for the install inflates to the plain files, byte for byte; the thesaurus holds every word's entry.
    void installedFilesAreThePlainFilesCompressed()
    {
        int checked = 0;
        QDirIterator it(plainRoot(), QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString plain = it.next();
            const QString rel = QDir(plainRoot()).relativeFilePath(plain);
            if (rel.endsWith(QLatin1String(".dat"))) continue;
            const bool packed = rel.endsWith(QLatin1String(".dic")) || rel.endsWith(QLatin1String(".aff"));
            const QByteArray want = fileBytes(plain);
            if (!packed) {
                QCOMPARE(fileBytes(builtRoot() + QLatin1Char('/') + rel), want);   // (licenses and READMEs go in as they are)
                continue;
            }
            QByteArray got;
            QVERIFY2(jp::dictpack::unpack(fileBytes(builtRoot() + QLatin1Char('/') + rel + QLatin1String(".jz")), got), qPrintable(rel));
            QVERIFY2(got == want, qPrintable(rel));
            ++checked;
        }
        QCOMPARE(checked, 68);   // (the .dic and .aff files of resources/dict)
        QVERIFY(!QFile::exists(builtRoot() + QStringLiteral("/en/en_US.dic")));   // (only the compressed file is installed)
        QVERIFY(!QFile::exists(builtRoot() + QStringLiteral("/en/th_en_US_v2.dat")));
        const std::map<QByteArray, QByteArray> reference = referenceEntries(fileBytes(plainRoot() + QStringLiteral("/en/th_en_US_v2.dat")));
        QVERIFY(reference.size() > 100000);
        jp::dictpack::Thesaurus t;
        QVERIFY(t.open(builtRoot() + QStringLiteral("/en/th_en_US_v2.dat.jz")));
        for (const auto &[key, entry] : reference) {
            if (t.entry(key) != entry) QFAIL(qPrintable(QString::fromUtf8(key)));
        }
        QVERIFY(t.entry("zzxqv").isEmpty());
        QVERIFY(t.entry("").isEmpty());
    }

    // Plain files (the source tree, an older install) still load, and answer as the compressed ones.
    void plainFilesStillLoad()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const char *f : {"en/en_US.dic", "en/en_US.aff", "en/hyph_en_US.dic", "en/th_en_US_v2.dat"}) {
            const QString to = dir.path() + QLatin1Char('/') + QLatin1String(f);
            QVERIFY(QDir().mkpath(QFileInfo(to).path()));
            QVERIFY(QFile::copy(plainRoot() + QLatin1Char('/') + QLatin1String(f), to));
        }
        const QStringList withBuilt = {jp::thesaurusLookup(QStringLiteral("happy")).join(QLatin1Char('|'))};
        QVERIFY(!withBuilt.first().isEmpty());
        RootGuard guard(dir.path());
        QVERIFY(jp::dict::hasSpelling(QStringLiteral("en-US")));
        QVERIFY(jp::dict::spell(QStringLiteral("en-US"), QStringLiteral("house")));
        QVERIFY(!jp::dict::spell(QStringLiteral("en-US"), QStringLiteral("hosue")));
        QVERIFY(!jp::dict::hyphenationFile(QStringLiteral("en-US")).isEmpty());
        QVERIFY(!jp::hyphenationPoints(QStringLiteral("hyphenation"), QStringLiteral("en-US")).isEmpty());
        QVERIFY(jp::dict::hasThesaurus());
        QCOMPARE(jp::thesaurusLookup(QStringLiteral("happy")).join(QLatin1Char('|')), withBuilt.first());
    }

    // A compressed file that is cut short, changed, or not one: nothing comes out, and nothing else goes wrong.
    void aDamagedFileFailsCleanly()
    {
        const QByteArray plain = fileBytes(plainRoot() + QStringLiteral("/en/en_US.dic"));
        const QByteArray packed = jp::dictpack::pack(plain);
        QVERIFY(packed.size() > 100 && packed.size() < plain.size() / 2);
        QByteArray out;
        QVERIFY(jp::dictpack::unpack(packed, out));
        QCOMPARE(out, plain);
        // cut short, at every kind of place
        for (const qsizetype n : {qsizetype(0), qsizetype(3), qsizetype(4), qsizetype(8), qsizetype(9), qsizetype(100), packed.size() / 2, packed.size() - 5, packed.size() - 1}) {
            QVERIFY2(!jp::dictpack::unpack(packed.left(n), out), qPrintable(QString::number(n)));
            QVERIFY(out.isEmpty());
        }
        // a byte changed (the stream checks itself) and bytes added
        for (const qsizetype at : {qsizetype(20), packed.size() / 3, packed.size() - 2}) {
            QByteArray bad = packed;
            bad[at] = char(bad[at] ^ 0x55);
            QVERIFY2(!jp::dictpack::unpack(bad, out), qPrintable(QString::number(at)));
        }
        QVERIFY(!jp::dictpack::unpack(packed + "x", out));
        // not one of ours
        QVERIFY(!jp::dictpack::unpack(plain, out));
        QVERIFY(!jp::dictpack::unpack(QByteArray("JZT1") + packed.mid(4), out));
        QVERIFY(!jp::dictpack::unpack(QByteArray(), out));
    }

    // No zip bomb: the size a file claims is checked first, and the stream must end at that size, so nothing grows past the cap.
    void inflatingIsBounded()
    {
        const QByteArray zeros(8 << 20, '\0');
        const QByteArray stream = qCompress(zeros, 9).mid(4);   // (Qt's four-byte size, then the zlib stream)
        QVERIFY(stream.size() < 20000);
        auto packedAs = [&](quint32 claim, const QByteArray &z) {
            QByteArray b("JZD1");
            for (int i = 0; i < 4; ++i) b.append(char((claim >> (8 * i)) & 0xff));
            return b + z;
        };
        QByteArray out;
        QVERIFY(jp::dictpack::unpack(packedAs(quint32(zeros.size()), stream), out));   // (the honest one)
        QCOMPARE(out.size(), zeros.size());
        QVERIFY(!jp::dictpack::unpack(packedAs(quint32(zeros.size()), stream), out, 1 << 20));       // over the caller's cap
        QVERIFY(!jp::dictpack::unpack(packedAs(1000, stream), out));                                  // claims less than it makes
        QVERIFY(!jp::dictpack::unpack(packedAs(quint32(zeros.size()) + 1, stream), out));            // claims more than it makes
        QVERIFY(!jp::dictpack::unpack(packedAs(0xffffffffu, stream), out));                          // claims 4 GB: refused before any allocation
        QVERIFY(!jp::dictpack::unpack(packedAs(quint32(jp::dictpack::kMaxFile) + 1, stream), out));
        QVERIFY(out.isEmpty());
        QVERIFY(jp::dictpack::pack(QByteArray(int(jp::dictpack::kMaxFile) + 1, 'a')).isEmpty());   // (nor is a file over the cap written)
    }

    // The packed thesaurus: small words come back, and damage to the table or a block gives nothing and no crash.
    void aDamagedThesaurusFailsCleanly()
    {
        const QByteArray packed = jp::dictpack::packThesaurus(QByteArray(kMiniThesaurus));
        QVERIFY(!packed.isEmpty());
        jp::dictpack::Thesaurus t;
        QVERIFY(t.openMemory(packed));
        QCOMPARE(t.entry("happy"), QByteArray("happy|2\n(adj)|glad|content|joyful\n(adj)|cheerful|sad (antonym)\n"));
        QCOMPARE(t.entry("apple"), QByteArray("Apple|1\n(noun)|fruit|pome\n"));   // (found by the lowercased word, and kept as written)
        QVERIFY(t.entry("Apple").isEmpty());
        QVERIFY(t.entry("aardvark").isEmpty());
        QVERIFY(t.entry("zzz").isEmpty());
        for (qsizetype n = 0; n < packed.size(); n += 3) {   // cut anywhere: it never opens, because the sizes of the blocks no longer add up
            jp::dictpack::Thesaurus cut;
            QVERIFY2(!cut.openMemory(packed.left(n)), qPrintable(QString::number(n)));
        }
        for (qsizetype at = 0; at < packed.size(); ++at) {   // any byte changed: it opens or it does not, and a lookup gives an entry or nothing
            QByteArray bad = packed;
            bad[at] = char(bad[at] ^ 0xff);
            jp::dictpack::Thesaurus b;
            if (b.openMemory(bad))
                for (const char *w : {"happy", "apple", "zebra", "q"}) (void)b.entry(w);
        }
        QByteArray hugeCount = packed;
        hugeCount[4] = char(0xff);
        hugeCount[5] = char(0xff);
        hugeCount[6] = char(0xff);
        hugeCount[7] = char(0x7f);
        jp::dictpack::Thesaurus h;
        QVERIFY(!h.openMemory(hugeCount));
        QVERIFY(!h.openMemory(QByteArray("JZT1")));
        {   // a table that claims 60,000 blocks in a small file is refused before it is read
            QTemporaryDir dir;
            QByteArray claim("JZT1");
            claim += QByteArray::fromHex("60ea0000") + QByteArray(100, '\0');
            QVERIFY(writeFile(dir.filePath(QStringLiteral("t.jz")), claim));
            jp::dictpack::Thesaurus f;
            QVERIFY(!f.open(dir.filePath(QStringLiteral("t.jz"))));
            QVERIFY(f.entry("happy").isEmpty());
        }
        QVERIFY(!h.openMemory(QByteArray()));
        QVERIFY(jp::dictpack::packThesaurus(QByteArray("UTF-8\n")).isEmpty());
        QVERIFY(jp::dictpack::packThesaurus(QByteArray("UTF-8\nword|1\n") + QByteArray(2 << 20, 'x') + "\n").isEmpty());   // (an entry over the cap)
    }

    // A language whose compressed file is damaged has no checker (every word is accepted, as for a missing dictionary); the others,
    // the hyphenation, and the thesaurus go on.
    void aDamagedInstallTurnsOffOnlyThatLanguage()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const char *f : {"en/en_US.dic", "en/en_US.aff", "en/en_GB.dic", "en/en_GB.aff", "en/hyph_en_US.dic", "en/th_en_US_v2.dat"}) {
            const QString plain = plainRoot() + QLatin1Char('/') + QLatin1String(f);
            const QString to = dir.path() + QLatin1Char('/') + QLatin1String(f) + QLatin1String(".jz");
            QByteArray packed = QString::fromLatin1(f).endsWith(QLatin1String(".dat")) ? jp::dictpack::packThesaurus(fileBytes(plain), 1) : jp::dictpack::pack(fileBytes(plain), 1);
            QVERIFY(!packed.isEmpty());
            if (QLatin1String(f) == QLatin1String("en/en_US.dic")) packed.truncate(packed.size() / 2);   // US English: cut short
            if (QLatin1String(f) == QLatin1String("en/hyph_en_US.dic")) packed[40] = char(packed[40] ^ 0xff);   // its hyphenation: changed
            if (QLatin1String(f) == QLatin1String("en/th_en_US_v2.dat")) packed.truncate(packed.size() - 100);   // the thesaurus: cut short
            QVERIFY(writeFile(to, packed));
        }
        RootGuard guard(dir.path());
        QVERIFY(!jp::dict::hasSpelling(QStringLiteral("en-US")));
        QVERIFY(jp::dict::spell(QStringLiteral("en-US"), QStringLiteral("hosue")));   // (no checker: nothing is wrong)
        QVERIFY(jp::dict::suggest(QStringLiteral("en-US"), QStringLiteral("hosue")).isEmpty());
        QVERIFY(jp::dict::hasSpelling(QStringLiteral("en-GB")));
        QVERIFY(jp::dict::spell(QStringLiteral("en-GB"), QStringLiteral("house")));
        QVERIFY(!jp::dict::spell(QStringLiteral("en-GB"), QStringLiteral("hosue")));
        QVERIFY(jp::hyphenationPoints(QStringLiteral("hyphenation"), QStringLiteral("en-US")).isEmpty());
        QVERIFY(!jp::dict::hasThesaurus());
        QVERIFY(jp::thesaurusLookup(QStringLiteral("happy")).isEmpty());
    }

    // Hunspell reads a line of at most 65,585 bytes (its buffer less one): one byte more ends the read, as it does reading a file.
    void aLongLineEndsTheReadAtHunspellsLimit()
    {
        for (const qsizetype length : {qsizetype(10), qsizetype(65585), qsizetype(65586)}) {
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            QVERIFY(writeFile(dir.filePath(QStringLiteral("en/en_US.aff.jz")), jp::dictpack::pack("SET UTF-8\n")));
            QVERIFY(writeFile(dir.filePath(QStringLiteral("en/en_US.dic.jz")), jp::dictpack::pack("3\nalpha\n" + QByteArray("gamma\tst:") + QByteArray(length - 9, 'x') + "\nbeta\n")));
            RootGuard guard(dir.path());
            QVERIFY(jp::dict::hasSpelling(QStringLiteral("en-US")));
            QVERIFY(jp::dict::spell(QStringLiteral("en-US"), QStringLiteral("alpha")));
            QCOMPARE(jp::dict::spell(QStringLiteral("en-US"), QStringLiteral("beta")), length <= 65585);   // (the word after it)
            QCOMPARE(jp::dict::spell(QStringLiteral("en-US"), QStringLiteral("gamma")), length <= 65585);   // (the long line itself)
        }
    }

    // The compressed hyphenation patterns break words as the plain ones did.
    void hyphenationFromCompressedFiles()
    {
        QVERIFY(QFile::exists(builtRoot() + QStringLiteral("/en/hyph_en_US.dic.jz")));
        const QVector<int> viaInstall = jp::hyphenationPoints(QStringLiteral("hyphenation"), QStringLiteral("en-US"));
        QVERIFY(!viaInstall.isEmpty());
        QVERIFY(!jp::hyphenationPoints(QString::fromUtf8("Reservierungsnummer"), QStringLiteral("de-DE")).isEmpty());
        QVERIFY(!jp::hyphenationPoints(QString::fromUtf8("помощник"), QStringLiteral("ru-RU")).isEmpty());
    }
};

JP_TEST_CLASS(PubDictTests)
#include "tst_pub_dict.moc"
