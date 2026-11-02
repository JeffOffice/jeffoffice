// Tests of the ciphers and of opening workbooks that are protected with a password: the ciphers against published values, then each kind of protection round trip (the files are made here, in
// code), the refusals (a wrong password, a file that needs one, damage, hostile numbers), and the readers' reports.
#include "xlshelpers.h"
#include "odshelpers.h"

#include "io/cfb.h"
#include "io/crypto.h"
#include "office/officecrypt.h"
#include "sheet/odscrypt.h"
#include "sheet/sheetcrypt.h"
#include "sheet/xlscrypt.h"

#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QPasswordDigestor>
#include <QTemporaryDir>

#include <random>

using namespace xlstest;
namespace crypto = jp::crypto;
namespace oc = jp::office;

namespace {

// Why a file was not opened with the password (None: it opened, or was refused for another reason).
PasswordNeed needOf(const QString &path, const QString &password)
{
    Workbook wb;
    Limits limits;
    limits.password = password;
    QString why;
    (void)wb.openFile(path, &why, limits);
    return wb.passwordNeed();
}

QByteArray hex(const char *h) { return QByteArray::fromHex(h); }

QByteArray le32(quint32 v) { return W().u32(v); }

QByteArray randomBytes(int n, unsigned seed)
{
    std::mt19937 gen(seed);
    QByteArray b(n, 0);
    for (char &c : b) c = char(gen() & 0xFF);
    return b;
}

// ---- a workbook of each form, with a cell whose value says it was read

QByteArray xlsxBook(int value = 7)
{
    return buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>%1</v></c></row>").arg(value))});
}

QByteArray odsBook(int value = 42) { return buildOds(contentXml(table(QStringLiteral("Sheet1"), columns(1) + row(floatCell(value))))); }

Book biffBook(int value = 5)
{
    Book b;
    b.sheet(QStringLiteral("One"), dimensions(1, 1) + number(0, 0, value) + label(1, 0, QStringLiteral("hello")));
    return b;
}

// ---- protecting a BIFF stream, as the file's writer does (the other side of oc::sheet::decryptBiff, made here from the format's description)

QByteArray utf16(const QString &t, int most)
{
    QByteArray b;
    for (const QChar c : t.left(most)) b.append(char(c.unicode() & 0xFF)).append(char(c.unicode() >> 8));
    return b;
}

quint16 u16At(const QByteArray &b, qsizetype at) { return quint16(quint8(b[at]) | quint8(b[at + 1]) << 8); }

enum class Biff { Xor, Rc4, CryptoApi };

QByteArray protectBiff(const QByteArray &stream, Biff kind, const QString &password, int keyBits = 128)
{
    const int first = 4 + u16At(stream, 2);   // the BOF
    QByteArray body;
    quint8 xorTable[16] = {};
    QByteArray base;
    int keyBytes = 16;
    const QByteArray salt = hex("00112233445566778899aabbccddeeff"), verifier = hex("0f1e2d3c4b5a69788796a5b4c3d2e1f0");
    auto blockKey = [&](quint32 block) {
        if (kind == Biff::Rc4) return QCryptographicHash::hash(base + le32(block), QCryptographicHash::Md5);
        // (a key of fewer than 128 bits is padded with zeros to 16 bytes)
        QByteArray key = QCryptographicHash::hash(base + le32(block), QCryptographicHash::Sha1).left(keyBytes);
        return key + QByteArray(16 - key.size(), 0);
    };
    if (kind == Biff::Xor) {
        QByteArray pw;
        for (const QChar c : password.left(15)) pw.append(char(c.unicode()));
        const quint16 key = oc::sheet::xorKeyForTest(pw), check = oc::sheet::xorVerifierForTest(pw);
        body = W().u16(0).u16(key).u16(check);
        const quint8 pad[15] = {0xBB, 0xFF, 0xFF, 0xBA, 0xFF, 0xFF, 0xB9, 0x80, 0x00, 0xBE, 0x0F, 0x00, 0xBF, 0x0F, 0x00};
        for (int p = 0; p < 16; ++p) {
            const quint8 k = (p & 1) ? quint8(key >> 8) : quint8(key & 0xFF);
            const quint8 x = quint8((p < pw.size() ? quint8(pw[p]) : pad[p - pw.size()]) ^ k);
            xorTable[p] = quint8((x >> 1) | (x << 7));
        }
    } else if (kind == Biff::Rc4) {
        const QByteArray h0 = QCryptographicHash::hash(utf16(password, 16), QCryptographicHash::Md5);
        QByteArray rep;
        for (int i = 0; i < 16; ++i) rep += h0.left(5) + salt;
        base = QCryptographicHash::hash(rep, QCryptographicHash::Md5).left(5);
        crypto::Rc4 rc4(blockKey(0));
        body = W().u16(1).u16(1).u16(1).raw(salt).raw(rc4.apply(verifier + QCryptographicHash::hash(verifier, QCryptographicHash::Md5)));
    } else {
        keyBytes = keyBits / 8;
        base = QCryptographicHash::hash(salt + utf16(password, 255), QCryptographicHash::Sha1);
        crypto::Rc4 rc4(blockKey(0));
        W header;
        header.u32(0x24).u32(0).u32(0x6801).u32(0x8004).u32(quint32(keyBits)).u32(1).u32(0).u32(0).raw(utf16(QStringLiteral("Microsoft Enhanced Cryptographic Provider v1.0"), 100)).u16(0);
        // (the verifier and then its hash are enciphered in one stream)
        const QByteArray both = rc4.apply(verifier + QCryptographicHash::hash(verifier, QCryptographicHash::Sha1));
        body = W().u16(1).u16(4).u16(2).u32(0x24).u32(quint32(header.bytes().size())).raw(header).u32(16).raw(salt).raw(both.left(16)).u32(20).raw(both.mid(16));
    }
    const QByteArray filePass = rec(0x2F, body);
    QByteArray out = stream.left(first) + filePass;
    // the records after the BOF: the sheets' places move with the record put in
    qsizetype p = first;
    QByteArray rest;
    while (p + 4 <= stream.size()) {
        const quint16 id = u16At(stream, p), len = u16At(stream, p + 2);
        QByteArray data = stream.mid(p + 4, len);
        if (id == 0x85 && data.size() >= 4) {
            const quint32 at = quint32(u16At(data, 0)) | quint32(u16At(data, 2)) << 16;
            data.replace(0, 4, le32(at + quint32(filePass.size())));
        }
        rest += W().u16(id).u16(len).raw(data).bytes();
        p += 4 + len;
    }
    out += rest;
    // enciphering: every record after FILEPASS, but the ones that begin a stream, with the first four bytes of a sheet's entry left
    qsizetype q = first + filePass.size();
    QByteArray keystream;
    qint64 have = -1;
    while (q + 4 <= out.size()) {
        const quint16 id = u16At(out, q), len = u16At(out, q + 2);
        const qsizetype start = q + 4;
        if (id != 0x809 && id != 0xE1) {
            for (qsizetype i = (id == 0x85 ? 4 : 0); i < len; ++i) {
                char &c = out[start + i];
                if (kind == Biff::Xor) {
                    // the writer: rotate right by 3, then xor with the table (the reader xors, then rotates left by 3)
                    const quint8 k = xorTable[(start + len + i) % 16];
                    const quint8 plain = quint8(c);
                    c = char(quint8((plain >> 3) | (plain << 5)) ^ k);
                } else {
                    const qint64 block = (start + i) / 1024;
                    if (block != have) {
                        crypto::Rc4 rc4(blockKey(quint32(block)));
                        keystream = rc4.apply(QByteArray(1024, 0));
                        have = block;
                    }
                    c = char(quint8(c) ^ quint8(keystream[int((start + i) % 1024)]));
                }
            }
        }
        q = start + len;
    }
    return out;
}

QByteArray compound(const QByteArray &stream)
{
    jp::cfb::File f;
    f.entries.resize(2);
    f.entries[0].type = jp::cfb::Entry::Root;
    f.entries[0].name = QStringLiteral("Root Entry");
    f.entries[0].children = {1};
    f.entries[1].name = QStringLiteral("Workbook");
    f.entries[1].data = stream;
    return jp::cfb::write(f);
}

QByteArray packageOf(const QByteArray &protectedFile, const char *name)
{
    jp::cfb::File f;
    QString why;
    if (!jp::cfb::read(protectedFile, &f, &why)) return QByteArray();
    return f.stream(QString::fromLatin1(name));
}

QByteArray containerOf(const QByteArray &info, const QByteArray &package)
{
    jp::cfb::File f;
    f.entries.resize(3);
    f.entries[0].type = jp::cfb::Entry::Root;
    f.entries[0].name = QStringLiteral("Root Entry");
    f.entries[0].children = {1, 2};
    f.entries[1].name = QStringLiteral("EncryptionInfo");
    f.entries[1].data = info;
    f.entries[2].name = QStringLiteral("EncryptedPackage");
    f.entries[2].data = package;
    return jp::cfb::write(f);
}

// ---- protecting an OpenDocument package

// Raw deflate of the bytes (the zlib stream of qCompress without its header and its check value).
QByteArray deflate(const QByteArray &data)
{
    const QByteArray z = qCompress(data, 9);   // 4 bytes of size, the 2 of the zlib header, the data, 4 of check value
    return z.mid(6, z.size() - 10);
}

struct OdsMake;
int blowfishIv(const OdsMake &make);

struct OdsMake {
    bool blowfish = false;
    bool sha256 = true;
    qint64 rounds = 1024;
    QString password = QStringLiteral("test");
    bool omitStartKey = false;
};

QByteArray protectOds(const QByteArray &plain, const OdsMake &make)
{
    QMap<QString, QByteArray> parts;
    if (!jp::readZip(plain, parts)) return QByteArray();
    QString manifest = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">\n"
                                      " <manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.spreadsheet\" manifest:full-path=\"/\"/>\n");
    jp::ZipWriter z;
    z.add(QStringLiteral("mimetype"), parts.value(QStringLiteral("mimetype")), false);
    int n = 0;
    for (const QString &name : {QStringLiteral("content.xml"), QStringLiteral("styles.xml"), QStringLiteral("settings.xml")}) {
        if (!parts.contains(name)) continue;
        const QByteArray data = parts.value(name);
        const QByteArray salt = randomBytes(16, 100 + n), iv = randomBytes(blowfishIv(make), 200 + n);
        ++n;
        QByteArray start = QCryptographicHash::hash(make.password.toUtf8(), make.sha256 ? QCryptographicHash::Sha256 : QCryptographicHash::Sha1);
        const int keySize = make.blowfish ? 16 : 32;
        const QByteArray key = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha1, start, salt, quint64(make.rounds), quint64(keySize));
        QByteArray packed = deflate(data), enc;
        const QByteArray check = QCryptographicHash::hash(packed.left(1024), make.sha256 && !make.blowfish ? QCryptographicHash::Sha256 : QCryptographicHash::Sha1);
        if (make.blowfish) {
            enc = crypto::blowfishCfb(key, iv, packed, true);
        } else {
            const int pad = 16 - int(packed.size() % 16);
            QByteArray padded = packed + QByteArray(pad, char(pad));
            enc = crypto::aesCbc(key, iv, padded, true);
        }
        z.add(name, enc, false);
        manifest += QStringLiteral(" <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"%1\" manifest:size=\"%2\">\n  <manifest:encryption-data manifest:checksum-type=\"%3\" manifest:checksum=\"%4\">\n"
                                   "   <manifest:algorithm manifest:algorithm-name=\"%5\" manifest:initialisation-vector=\"%6\"/>\n"
                                   "   <manifest:key-derivation manifest:key-derivation-name=\"PBKDF2\" manifest:key-size=\"%7\" manifest:iteration-count=\"%8\" manifest:salt=\"%9\"/>\n")
                        .arg(name).arg(data.size())
                        .arg(make.blowfish ? QStringLiteral("SHA1/1K") : make.sha256 ? QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k") : QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha1-1k"))
                        .arg(QString::fromLatin1(check.toBase64()))
                        .arg(make.blowfish ? QStringLiteral("Blowfish CFB") : QStringLiteral("http://www.w3.org/2001/04/xmlenc#aes256-cbc"))
                        .arg(QString::fromLatin1(iv.toBase64())).arg(keySize).arg(make.rounds).arg(QString::fromLatin1(salt.toBase64()));
        if (!make.omitStartKey)
            manifest += QStringLiteral("   <manifest:start-key-generation manifest:start-key-generation-name=\"%1\" manifest:key-size=\"%2\"/>\n")
                            .arg(make.sha256 ? QStringLiteral("http://www.w3.org/2000/09/xmldsig#sha256") : QStringLiteral("SHA1")).arg(make.sha256 ? 32 : 20);
        manifest += QStringLiteral("  </manifest:encryption-data>\n </manifest:file-entry>\n");
    }
    manifest += QStringLiteral("</manifest:manifest>\n");
    z.add(QStringLiteral("META-INF/manifest.xml"), manifest.toUtf8(), true);
    return z.finish();
}

int blowfishIv(const OdsMake &make) { return make.blowfish ? 8 : 16; }

} // namespace

class SheetCryptTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- the ciphers, against the values their standards publish

    void aesMatchesTheStandardsValues()
    {
        const QByteArray plain = hex("00112233445566778899aabbccddeeff");
        const QByteArray k128 = hex("000102030405060708090a0b0c0d0e0f"), k192 = hex("000102030405060708090a0b0c0d0e0f1011121314151617"),
                         k256 = hex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
        QCOMPARE(crypto::aesEcb(k128, plain, true), hex("69c4e0d86a7b0430d8cdb78070b4c55a"));
        QCOMPARE(crypto::aesEcb(k192, plain, true), hex("dda97ca4864cdfe06eaf70a0ec0d7191"));
        QCOMPARE(crypto::aesEcb(k256, plain, true), hex("8ea2b7ca516745bfeafc49904b496089"));
        QCOMPARE(crypto::aesEcb(k128, hex("69c4e0d86a7b0430d8cdb78070b4c55a"), false), plain);
        QCOMPARE(crypto::aesEcb(k192, hex("dda97ca4864cdfe06eaf70a0ec0d7191"), false), plain);
        QCOMPARE(crypto::aesEcb(k256, hex("8ea2b7ca516745bfeafc49904b496089"), false), plain);
        // chaining: the first two blocks of the standard's example for AES-128 in CBC mode
        const QByteArray key = hex("2b7e151628aed2a6abf7158809cf4f3c"), iv = hex("000102030405060708090a0b0c0d0e0f");
        const QByteArray text = hex("6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e51"), sealed = hex("7649abac8119b246cee98e9b12e9197d5086cb9b507219ee95db113a917678b2");
        QCOMPARE(crypto::aesCbc(key, iv, text, true), sealed);
        QCOMPARE(crypto::aesCbc(key, iv, sealed, false), text);
    }

    void aesRoundTripsAndRefusesWhatItCannotDo()
    {
        const QByteArray data = randomBytes(16 * 33, 1);
        for (const int bits : {128, 192, 256}) {
            const QByteArray key = randomBytes(bits / 8, 2), iv = randomBytes(16, 3);
            QCOMPARE(crypto::aesCbc(key, iv, crypto::aesCbc(key, iv, data, true), false), data);
            QCOMPARE(crypto::aesEcb(key, crypto::aesEcb(key, data, true), false), data);
            QVERIFY(crypto::aesCbc(key, iv, data, true) != crypto::aesEcb(key, data, true));
        }
        QVERIFY(crypto::aesEcb(QByteArray(15, 'k'), QByteArray(16, 'x'), true).isEmpty());
        QVERIFY(crypto::aesEcb(QByteArray(16, 'k'), QByteArray(17, 'x'), true).isEmpty());
        QVERIFY(crypto::aesCbc(QByteArray(16, 'k'), QByteArray(15, 'i'), QByteArray(16, 'x'), true).isEmpty());
    }

    void rc4MatchesTheStandardsValues()
    {
        // (RFC 6229: the first bytes of the stream of a key, and the same used on zeros)
        QCOMPARE(crypto::Rc4(hex("0102030405")).apply(QByteArray(16, 0)), hex("b2396305f03dc027ccc3524a0a1118a8"));
        QCOMPARE(crypto::Rc4(hex("0102030405060708")).apply(QByteArray(16, 0)), hex("97ab8a1bf0afb96132f2f67258da15a8"));
        QCOMPARE(crypto::Rc4(hex("0102030405060708090a0b0c0d0e0f10")).apply(QByteArray(16, 0)), hex("9ac7cc9a609d1ef7b2932899cde41b97"));
        // (the classic example: the key "Key" and the text "Plaintext")
        QCOMPARE(crypto::Rc4("Key").apply(QByteArray("Plaintext")), hex("bbf316e8d940af0ad3"));
        // a skip is the bytes used and not seen
        crypto::Rc4 a(hex("0102030405")), b(hex("0102030405"));
        a.skip(7);
        QByteArray zeros(20, 0);
        const QByteArray whole = b.apply(zeros);
        QCOMPARE(a.apply(QByteArray(13, 0)), whole.mid(7));
    }

    void blowfishMatchesThePublishedValues()
    {
        QCOMPARE(crypto::blowfishEcbBlock(hex("0000000000000000"), hex("0000000000000000"), true), hex("4ef997456198dd78"));
        QCOMPARE(crypto::blowfishEcbBlock(hex("ffffffffffffffff"), hex("ffffffffffffffff"), true), hex("51866fd5b85ecb8a"));
        QCOMPARE(crypto::blowfishEcbBlock(hex("3000000000000000"), hex("1000000000000001"), true), hex("7d856f9a613063f2"));
        QCOMPARE(crypto::blowfishEcbBlock(hex("3000000000000000"), hex("7d856f9a613063f2"), false), hex("1000000000000001"));
        // (cipher feedback with a whole block fed back, the last block short: a value made by an independent implementation)
        const QByteArray key = hex("0123456789abcdeff0e1d2c3b4a59687"), iv = hex("fedcba9876543210"), text = "The quick brown fox jumps over the lazy dog!";
        const QByteArray sealed = hex("846c44b6c0666189138fbfbb9125eeb22ed762fc4360800298216262c4024429215324bcb792b0c63108d09c");
        QCOMPARE(crypto::blowfishCfb(key, iv, text, true), sealed);
        QCOMPARE(crypto::blowfishCfb(key, iv, sealed, false), text);
        QVERIFY(crypto::blowfishCfb(QByteArray(3, 'k'), iv, text, true).isEmpty());
        QVERIFY(crypto::blowfishCfb(key, QByteArray(7, 'i'), text, true).isEmpty());
    }

    void aSecretIsOverwrittenWhenWiped()
    {
        QByteArray secret(64, 'p');
        const char *where = secret.constData();
        QByteArray other = secret;   // (a second reference: the wipe works on its own copy)
        crypto::wipe(secret);
        QVERIFY(secret.isEmpty());
        QVERIFY(where != nullptr);
        QCOMPARE(other, QByteArray(64, 'p'));
        QString text = QStringLiteral("hunter2");
        crypto::wipe(text);
        QVERIFY(text.isEmpty());
    }

    // ---- protected packages

    void everyAgileForm_opensWithItsPassword()
    {
        const QByteArray zip = xlsxBook();
        for (const int bits : {128, 192, 256})
            for (const QString &hash : {QStringLiteral("SHA1"), QStringLiteral("SHA256"), QStringLiteral("SHA384"), QStringLiteral("SHA512")}) {
                oc::EncryptOptions options;
                options.keyBits = bits;
                options.hash = hash;
                options.spinCount = 50;
                const QByteArray file = oc::encryptPackage(zip, QStringLiteral("päss wörd"), options);
                QVERIFY2(!file.isEmpty(), qPrintable(hash));
                QByteArray out;
                QString why;
                QCOMPARE(oc::decryptPackage(packageOf(file, "EncryptionInfo"), packageOf(file, "EncryptedPackage"), QStringLiteral("päss wörd"), 1 << 30, false, &out), oc::CryptResult::Ok);
                QCOMPARE(out, zip);
                QCOMPARE(oc::decryptPackage(packageOf(file, "EncryptionInfo"), packageOf(file, "EncryptedPackage"), QStringLiteral("other"), 1 << 30, false, &out), oc::CryptResult::WrongPassword);
            }
    }

    void aPackageOfSeveralSegmentsIsWholeAfterTheRoundTrip()
    {
        const QByteArray zip = randomBytes(4096 * 3 + 123, 7);
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray file = oc::encryptPackage(zip, QStringLiteral("x"), options);
        QByteArray out;
        QCOMPARE(oc::decryptPackage(packageOf(file, "EncryptionInfo"), packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Ok);
        QCOMPARE(out, zip);
        // (the same package refused when it would come to more than the caller allows)
        QCOMPARE(oc::decryptPackage(packageOf(file, "EncryptionInfo"), packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1000, false, &out), oc::CryptResult::TooLarge);
    }

    void aChangedAgilePackageIsDamagedAndNotWrong()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray file = oc::encryptPackage(randomBytes(9000, 8), QStringLiteral("x"), options);
        const QByteArray info = packageOf(file, "EncryptionInfo");
        QByteArray package = packageOf(file, "EncryptedPackage");
        package[5000] = char(package[5000] ^ 1);
        QByteArray out;
        QCOMPARE(oc::decryptPackage(info, package, QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Damaged);
        // (the check is of the stream whole: its size field too)
        package = packageOf(file, "EncryptedPackage");
        package[0] = char(package[0] ^ 1);
        QCOMPARE(oc::decryptPackage(info, package, QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Damaged);
    }

    void theStandardFormOpensWithItsPassword()
    {
        const QByteArray zip = xlsxBook();
        const QByteArray file = oc::encryptPackageStandard(zip, QStringLiteral("secret"));
        QByteArray out;
        QCOMPARE(oc::decryptPackage(packageOf(file, "EncryptionInfo"), packageOf(file, "EncryptedPackage"), QStringLiteral("secret"), 1 << 30, false, &out), oc::CryptResult::Ok);
        QCOMPARE(out, zip);
        QCOMPARE(oc::decryptPackage(packageOf(file, "EncryptionInfo"), packageOf(file, "EncryptedPackage"), QStringLiteral("Secret"), 1 << 30, false, &out), oc::CryptResult::WrongPassword);
    }

    void aPackageThatIsCutOrAskedForTooMuchIsRefusedQuickly()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray agile = oc::encryptPackage(randomBytes(5000, 9), QStringLiteral("x"), options);
        const QByteArray standard = oc::encryptPackageStandard(randomBytes(5000, 9), QStringLiteral("x"));
        for (const QByteArray &file : {agile, standard}) {
            const QByteArray info = packageOf(file, "EncryptionInfo"), package = packageOf(file, "EncryptedPackage");
            QByteArray out;
            // every length of the information, and of the package
            for (qsizetype n = 0; n < info.size(); n += 7) {
                const oc::CryptResult r = oc::decryptPackage(info.left(n), package, QStringLiteral("x"), 1 << 30, false, &out);
                QVERIFY2(r != oc::CryptResult::Ok, qPrintable(QString::number(n)));
            }
            for (qsizetype n = 0; n < package.size(); n += 997) QVERIFY(oc::decryptPackage(info, package.left(n), QStringLiteral("x"), 1 << 30, false, &out) != oc::CryptResult::Ok);
            // a size that the stream cannot hold
            QByteArray big = package;
            big[7] = char(0x7F);
            QVERIFY(oc::decryptPackage(info, big, QStringLiteral("x"), 1 << 30, false, &out) != oc::CryptResult::Ok);
        }
    }

    void aHugeSpinCountCannotKeepTheReaderBusy()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray file = oc::encryptPackage(randomBytes(100, 9), QStringLiteral("x"), options);
        QByteArray info = packageOf(file, "EncryptionInfo");
        QVERIFY(info.contains("spinCount=\"10\""));
        for (const char *huge : {"2000000000", "2000001", "99999999999999999999", "-5"}) {
            QByteArray changed = info;
            changed.replace("spinCount=\"10\"", QByteArray("spinCount=\"") + huge + "\"");
            QElapsedTimer t;
            t.start();
            QByteArray out;
            QCOMPARE(oc::decryptPackage(changed, packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Damaged);
            QVERIFY2(t.elapsed() < 2000, huge);
        }
        // (a made-up cipher, a hash that is not one, and a key of a size no cipher has)
        for (const QPair<const char *, const char *> &change : {QPair<const char *, const char *>{"cipherAlgorithm=\"AES\"", "cipherAlgorithm=\"RC2\""}, {"hashAlgorithm=\"SHA512\"", "hashAlgorithm=\"MD2\""},
                                                              {"keyBits=\"256\"", "keyBits=\"250\""}, {"blockSize=\"16\"", "blockSize=\"8\""}, {"saltSize=\"16\"", "saltSize=\"0\""}}) {
            QByteArray changed = info;
            changed.replace(change.first, change.second);
            QByteArray out;
            QVERIFY2(oc::decryptPackage(changed, packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1 << 30, false, &out) != oc::CryptResult::Ok, change.second);
        }
    }

    // A package written here has the structures that tell a reader how it is enciphered, of the sizes the format gives them.
    void aWrittenPackageHasTheDataSpaces()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray file = oc::encryptPackage(randomBytes(500, 4), QStringLiteral("x"), options);
        jp::cfb::File f;
        QString why;
        QVERIFY2(jp::cfb::read(file, &f, &why), qPrintable(why));
        const QString spaces = QStringLiteral("\u0006DataSpaces/");
        QCOMPARE(f.stream(spaces + QStringLiteral("Version")), hex("3c000000") + QByteArray("M\0i\0c\0r\0o\0s\0o\0f\0t\0.\0C\0o\0n\0t\0a\0i\0n\0e\0r\0.\0D\0a\0t\0a\0S\0p\0a\0c\0e\0s\0", 60) + hex("010000000100000001000000"));
        const QByteArray map = f.stream(spaces + QStringLiteral("DataSpaceMap"));
        QCOMPARE(map.size(), 8 + 104);
        QCOMPARE(map.left(16), hex("08000000010000006800000001000000"));
        QVERIFY(map.contains(QByteArray("E\0n\0c\0r\0y\0p\0t\0e\0d\0P\0a\0c\0k\0a\0g\0e\0", 32)));
        QCOMPARE(f.stream(spaces + QStringLiteral("DataSpaceInfo/StrongEncryptionDataSpace")).size(), 8 + 4 + 52);
        const QByteArray primary = f.stream(spaces + QStringLiteral("TransformInfo/StrongEncryptionTransform/\u0006Primary"));
        QCOMPARE(primary.size(), 200);
        QCOMPARE(primary.left(12), hex("580000000100000000004c00").left(8) + hex("4c000000"));
        QVERIFY(primary.contains(QByteArray("{\0F\0F\0" "9\0A\0" "3\0F\0" "0\0" "3\0", 18)));
        QCOMPARE(primary.right(4), hex("04000000"));
        // (the package is still read back)
        QByteArray out;
        QCOMPARE(oc::decryptPackage(f.stream(QStringLiteral("EncryptionInfo")), f.stream(QStringLiteral("EncryptedPackage")), QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Ok);
    }

    // The key of a long spin is made in pieces: a stop is seen at once, and a file that asks for more than the limit is refused with no work.
    void theKeyIsMadeInPiecesAndCanBeStopped()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray file = oc::encryptPackage(randomBytes(100, 9), QStringLiteral("x"), options);
        QByteArray info = packageOf(file, "EncryptionInfo");
        info.replace("spinCount=\"10\"", "spinCount=\"1500000\"");
        QElapsedTimer t;
        t.start();
        int asked = 0;
        QByteArray out;
        QCOMPARE(oc::decryptPackage(info, packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1 << 30, false, &out, [&asked] { return ++asked > 1; }), oc::CryptResult::Cancelled);
        QVERIFY(t.elapsed() < 1000);
        QVERIFY(asked >= 2);
        info.replace("spinCount=\"1500000\"", "spinCount=\"2000001\"");
        QCOMPARE(oc::decryptPackage(info, packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Damaged);
    }

    // ---- what the review asked for

    // Every size an OpenDocument manifest gives, of every part, is checked before any key is made: a bad one in the last part is refused with no work done for the first. The refusal is
    // the specific one (damaged), and quick.
    void aManifestWithSizesNoCipherHasIsRefusedBeforeAnyWork()
    {
        OdsMake blowfish;
        blowfish.blowfish = true;
        blowfish.sha256 = false;
        blowfish.rounds = 1500000;   // (a key for the first part would take a second or more)
        OdsMake aesMake;
        aesMake.rounds = 1500000;
        const QByteArray bf = protectOds(odsBook(), blowfish), aes = protectOds(odsBook(), aesMake);
        struct Change {
            const QByteArray *file;
            QByteArray from, to;
            const char *name;
        };
        const QByteArray salt = "manifest:salt=\"";
        const QVector<Change> changes = {
            {&bf, "manifest:key-size=\"16\"", "manifest:key-size=\"2000000000\"", "blowfish key of 2 GB"},
            {&bf, "manifest:key-size=\"16\"", "manifest:key-size=\"3\"", "blowfish key of 3 bytes"},
            {&bf, "manifest:key-size=\"16\"", "manifest:key-size=\"57\"", "blowfish key of 57 bytes"},
            {&aes, "manifest:key-size=\"32\" manifest:iteration", "manifest:key-size=\"7\" manifest:iteration", "aes key of 7 bytes"},
            {&aes, "manifest:iteration-count=\"1500000\"", "manifest:iteration-count=\"0\"", "no rounds"},
            {&aes, "manifest:iteration-count=\"1500000\"", "manifest:iteration-count=\"9223372036854775807\"", "rounds of LLONG_MAX"},
            {&bf, "manifest:iteration-count=\"1500000\"", "manifest:iteration-count=\"-4\"", "negative rounds"},
            {&aes, salt, salt + QByteArray(2000, 'A'), "a salt of 1500 bytes"},
        };
        for (const Change &c : changes) {
            // (the change is made in the manifest's last part only: the parts before it are well formed, and would each take a key)
            QMap<QString, QByteArray> parts;
            QVERIFY(jp::readZip(*c.file, parts));
            QByteArray manifest = parts.value(QStringLiteral("META-INF/manifest.xml"));
            const qsizetype at = manifest.lastIndexOf(c.from);
            QVERIFY2(at > 0 && manifest.indexOf(c.from) < at, c.name);
            manifest.replace(at, c.from.size(), c.to);
            jp::ZipWriter z;
            z.add(QStringLiteral("mimetype"), parts.value(QStringLiteral("mimetype")), false);
            for (auto it = parts.constBegin(); it != parts.constEnd(); ++it)
                if (it.key() != QLatin1String("mimetype")) z.add(it.key(), it.key() == QLatin1String("META-INF/manifest.xml") ? manifest : it.value(), false);
            QElapsedTimer t;
            t.start();
            QByteArray out;
            QCOMPARE(oc::sheet::decryptOds(z.finish(), QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::Damaged);
            QVERIFY2(t.elapsed() < 300, c.name);
        }
    }

    // Two parts, the first with a fair number of rounds and the second with the largest number there is.
    void aSecondPartWithAnEnormousNumberOfRoundsIsRefusedAtOnce()
    {
        OdsMake make;
        make.rounds = 1024;
        const QByteArray file = protectOds(odsBook(), make);
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(file, parts));
        QByteArray manifest = parts.value(QStringLiteral("META-INF/manifest.xml"));
        const QByteArray from = "manifest:iteration-count=\"1024\"";
        const qsizetype first = manifest.indexOf(from), second = manifest.indexOf(from, first + 1);
        QVERIFY(first > 0 && second > first);
        manifest.replace(second, from.size(), "manifest:iteration-count=\"9223372036854775807\"");
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), parts.value(QStringLiteral("mimetype")), false);
        for (auto it = parts.constBegin(); it != parts.constEnd(); ++it)
            if (it.key() != QLatin1String("mimetype")) z.add(it.key(), it.key() == QLatin1String("META-INF/manifest.xml") ? manifest : it.value(), false);
        QByteArray out;
        QCOMPARE(oc::sheet::decryptOds(z.finish(), QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::Damaged);
    }

    // The digest that tells a protected file from one another program changed is of the bytes the reader read; a file that is not protected has none.
    void aProtectedFilesDigestIsOfTheBytesRead()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        oc::EncryptOptions options;
        options.spinCount = 20;
        struct Case {
            const char *name;
            QByteArray bytes;
            QString password;
        };
        const QVector<Case> cases = {{"a.xlsx", oc::encryptPackage(xlsxBook(), QStringLiteral("pw"), options), QStringLiteral("pw")},
                                     {"b.xls", compound(protectBiff(biffBook().stream(), Biff::Rc4, QStringLiteral("pw"))), QStringLiteral("pw")},
                                     {"c.ods", protectOds(odsBook(), OdsMake()), QStringLiteral("test")},
                                     {"d.xlsx", xlsxBook(), QString()}};
        for (const Case &c : cases) {
            QFile f(dir.filePath(QString::fromLatin1(c.name)));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(c.bytes);
            f.close();
            Limits limits;
            limits.password = c.password;
            Workbook wb;
            QString why;
            QVERIFY2(wb.openFile(f.fileName(), &why, limits), c.name);
            if (c.password.isEmpty()) QVERIFY2(wb.fileDigest().isEmpty(), c.name);
            else QCOMPARE(wb.fileDigest(), QCryptographicHash::hash(c.bytes, QCryptographicHash::Sha256));
        }
    }

    void twoByteStringsAreComparedWholeAndByLength()
    {
        QVERIFY(crypto::equal(QByteArray("abcdef"), QByteArray("abcdef")));
        QVERIFY(crypto::equal(QByteArray(), QByteArray()));
        QVERIFY(!crypto::equal(QByteArray("abcdef"), QByteArray("abcdeg")));
        QVERIFY(!crypto::equal(QByteArray("abcdef"), QByteArray("Abcdef")));
        QVERIFY(!crypto::equal(QByteArray("abcdef"), QByteArray("abcde")));   // (a prefix is not the same)
        QVERIFY(!crypto::equal(QByteArray("abcde"), QByteArray("abcdef")));
        QVERIFY(!crypto::equal(QByteArray(), QByteArray("a")));
        QVERIFY(!crypto::equal(QByteArray(1, '\0'), QByteArray()));
    }

    // The obfuscation of the oldest files can only hold a password of the Western (Latin-1) letters: another password is said not to be the one, not turned into question marks.
    void aPasswordOfOtherLettersDoesNotMatchTheOldObfuscation()
    {
        const QByteArray stream = protectBiff(biffBook().stream(), Biff::Xor, QStringLiteral("abc"));
        for (const QString &typed : {QStringLiteral("\u0430\u0431\u0432"), QStringLiteral("abc\u0430"), QStringLiteral("\u4e2d")}) {
            Limits limits;
            limits.password = typed;
            Workbook wb;
            QString why;
            QVERIFY(!wb.openBinary(compound(stream), &why, limits));
            QCOMPARE(wb.passwordNeed(), PasswordNeed::Wrong);
        }
        // (even a password that would turn into the same question marks is not the one)
        const QByteArray marks = protectBiff(biffBook().stream(), Biff::Xor, QStringLiteral("???"));
        Limits cyrillic;
        cyrillic.password = QStringLiteral("\u0430\u0431\u0432");
        Workbook marked;
        QString text;
        QVERIFY(!marked.openBinary(compound(marks), &text, cyrillic));
        QCOMPARE(marked.passwordNeed(), PasswordNeed::Wrong);
        // (a password that has letters of the 40-bit form's own kind: any letters, as two bytes each)
        const QString greek = QStringLiteral("\u03b1\u03b2\u03b3");
        const QByteArray rc4 = protectBiff(biffBook().stream(), Biff::Rc4, greek);
        Limits limits;
        limits.password = greek;
        Workbook wb;
        QString why;
        QVERIFY2(wb.openBinary(compound(rc4), &why, limits), qPrintable(why));
    }

    // A stop is seen while an OpenDocument key is made, as it is for a package's.
    void anOpenDocumentKeyCanBeStopped()
    {
        OdsMake make;
        make.rounds = 1500000;
        const QByteArray file = protectOds(odsBook(), make);
        QElapsedTimer t;
        t.start();
        int asked = 0;
        QByteArray out;
        QCOMPARE(oc::sheet::decryptOds(file, QStringLiteral("test"), 1 << 30, false, &out, [&asked] { return ++asked > 1; }), oc::CryptResult::Cancelled);
        QVERIFY(t.elapsed() < 2000);
    }

    // The key the reader makes is the standard's: PBKDF2 with HMAC-SHA-1 (Qt's own, which gives RFC 6070's value for 4096 rounds, is what the file here was made with).
    void theOpenDocumentKeyIsPbkdf2()
    {
        OdsMake make;
        make.rounds = 4096;
        QCOMPARE(QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha1, "password", "salt", 4096, 20), hex("4b007901b765489abead49d926f721d065a429c1"));
        const QByteArray file = protectOds(odsBook(), make);
        QByteArray out;
        QCOMPARE(oc::sheet::decryptOds(file, QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::Ok);
    }

    void aPackageWithNoCheckOfChangesIsDamaged()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray file = oc::encryptPackage(randomBytes(100, 9), QStringLiteral("x"), options);
        QByteArray info = packageOf(file, "EncryptionInfo");
        const qsizetype from = info.indexOf("<dataIntegrity"), to = info.indexOf("/>", from) + 2;
        QVERIFY(from > 0 && to > from);
        info.remove(from, to - from);
        QByteArray out;
        QCOMPARE(oc::decryptPackage(info, packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Damaged);
    }

    void aKeyOfOneSizeAndHashOfAnotherAreRefused()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray file = oc::encryptPackage(randomBytes(100, 9), QStringLiteral("x"), options);
        const QByteArray info = packageOf(file, "EncryptionInfo"), package = packageOf(file, "EncryptedPackage");
        // (the second element that names them is the key encryptor's)
        auto changeSecond = [&](const QByteArray &from, const QByteArray &to, QByteArray changed = QByteArray()) {
            if (changed.isEmpty()) changed = info;
            const qsizetype first = changed.indexOf(from), second = changed.indexOf(from, first + 1);
            [&] { QVERIFY(first > 0 && second > first); }();
            changed.replace(second, from.size(), to);
            return changed;
        };
        QByteArray out;
        QCOMPARE(oc::decryptPackage(changeSecond("keyBits=\"256\"", "keyBits=\"128\""), package, QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Damaged);
        QCOMPARE(oc::decryptPackage(changeSecond("hashSize=\"64\"", "hashSize=\"32\"", changeSecond("hashAlgorithm=\"SHA512\"", "hashAlgorithm=\"SHA256\"")), package, QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::Unsupported);
    }

    // The password that every program knows is tried first, whatever password was typed.
    void theDefaultPasswordIsTriedWhateverWasTyped()
    {
        oc::EncryptOptions options;
        options.spinCount = 20;
        const QByteArray file = oc::encryptPackage(xlsxBook(), oc::defaultPassword(), options);
        Limits limits;
        limits.password = QStringLiteral("something else");
        Workbook wb;
        QString why;
        QVERIFY2(wb.openBinary(file, &why, limits), qPrintable(why));
        QVERIFY(wb.wasProtected());
        QVERIFY(wb.openedWithDefaultPassword());
        // a file with a password of its own is not "default only"
        Limits right;
        right.password = QStringLiteral("mine");
        Workbook mine;
        QVERIFY(mine.openBinary(oc::encryptPackage(xlsxBook(), QStringLiteral("mine"), options), &why, right));
        QVERIFY(mine.wasProtected());
        QVERIFY(!mine.openedWithDefaultPassword());
        // and the same for the old binary form
        const QByteArray old = compound(protectBiff(biffBook().stream(), Biff::Rc4, oc::defaultPassword()));
        Workbook oldBook;
        QVERIFY(oldBook.openBinary(old, &why, limits));
        QVERIFY(oldBook.openedWithDefaultPassword());
    }

    // ---- real files of other programs, when the collections are there (never copied into the repository)

    void realFilesOfOtherProgramsOpen_data()
    {
        QTest::addColumn<QString>("path");
        QTest::addColumn<QString>("password");
        QTest::addColumn<QString>("cell");
        QTest::addColumn<QString>("expected");
        // (the collections of other programs' files are where JO_CORPORA says; without it, the rows skip)
        const QString root = qEnvironmentVariable("JO_CORPORA");
        const QString poi = root + QStringLiteral("/poi/test-data/spreadsheet/"), lo = root + QStringLiteral("/libreoffice/sc/qa/unit/data/ods/");
        QTest::newRow("xls, 40-bit RC4") << poi + QStringLiteral("password.xls") << QStringLiteral("password") << QStringLiteral("D5") << QStringLiteral("s:A ZIP bomb is a variant");
        QTest::newRow("xls, obfuscation (XOR)") << poi + QStringLiteral("xor-encryption-abc.xls") << QStringLiteral("abc") << QStringLiteral("A2") << QStringLiteral("n:2");
        QTest::newRow("xls, type 4") << poi + QStringLiteral("35897-type4.xls") << QStringLiteral("freedom") << QStringLiteral("A1") << QStringLiteral("s:hello there!");
        QTest::newRow("xlsx, agile") << poi + QStringLiteral("protected_passtika.xlsx") << QStringLiteral("tika") << QStringLiteral("A1") << QStringLiteral("s:This is an Encrypted");
        QTest::newRow("xlsx, default password") << poi + QStringLiteral("58616.xlsx") << QString() << QStringLiteral("A1") << QStringLiteral("s:EPQ Manual Template");
        QTest::newRow("ods, AES") << lo + QStringLiteral("password.ods") << QStringLiteral("test") << QString() << QString();
        QTest::newRow("ods, Blowfish") << lo + QStringLiteral("passwordOld.ods") << QStringLiteral("test") << QStringLiteral("A1") << QStringLiteral("n:2");
    }

    void realFilesOfOtherProgramsOpen()
    {
        QFETCH(QString, path);
        QFETCH(QString, password);
        QFETCH(QString, cell);
        QFETCH(QString, expected);
        if (qEnvironmentVariableIsEmpty("JO_CORPORA") || !QFileInfo::exists(path)) QSKIP("set JO_CORPORA to the folder of the collections of other programs' files");
        Workbook wb;
        Limits limits;
        limits.password = password;
        QString why;
        QVERIFY2(wb.openFile(path, &why, limits), qPrintable(why));
        QVERIFY(wb.wasProtected());
        if (!password.isEmpty()) {
            Workbook wrong;
            Limits bad;
            bad.password = QStringLiteral("not it");
            QVERIFY(!wrong.openFile(path, &why, bad));
            QCOMPARE(wrong.passwordNeed(), PasswordNeed::Wrong);
        }
        if (cell.isEmpty()) return;   // (a document with no cell to look at: that it opens is the check)
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY2(show(s, wb.sharedStrings(), cell).startsWith(expected) || (wb.isBinary() && xlstest::shown(wb, s, cell).startsWith(expected.mid(0, 2) == QLatin1String("s:") ? QStringLiteral("i:") + expected.mid(2) : expected)),
                 qPrintable(show(s, wb.sharedStrings(), cell)));
    }

    // Files the original program wrote with passwords (made by this program first), when JO_ENC_SAMPLES names the folder they were brought back to.
    void filesTheOriginalProgramWroteOpen()
    {
        const QString dir = qEnvironmentVariable("JO_ENC_SAMPLES");
        if (dir.isEmpty() || !QDir(dir).exists()) QSKIP("set JO_ENC_SAMPLES to the folder of the files the original program re-saved");
        // (the passwords the files were made with)
        const QMap<QString, QString> passwords = {{QStringLiteral("agile256-sha512.xlsx"), QStringLiteral("Sesame-42")},
                                                  {QStringLiteral("agile256-sha512-unicode.xlsx"), QStringLiteral("p\u00e4ssw\u00f6rd \u00f1 1")},
                                                  {QStringLiteral("agile128-sha1.xlsx"), QStringLiteral("abc123")},
                                                  {QStringLiteral("agile256-sha256.xlsx"), QStringLiteral("Quiet River")}};
        const QStringList files = QDir(dir).entryList({QStringLiteral("*.xlsx")}, QDir::Files);
        if (files.isEmpty()) QSKIP("no files were brought back");
        int opened = 0;
        for (const QString &name : files) {
            // (re-saved files carry the name of the file they came from, after a prefix)
            QString original = name;
            if (original.startsWith(QLatin1String("resaved-"))) original = original.mid(8);
            if (!passwords.contains(original)) continue;
            Workbook wb;
            Limits limits;
            limits.password = passwords.value(original);
            QString why;
            QVERIFY2(wb.openFile(QDir(dir).filePath(name), &why, limits), qPrintable(name + QLatin1String(": ") + why));
            Sheet s;
            QVERIFY(wb.readSheet(0, &s));
            QVERIFY2(show(s, wb.sharedStrings(), QStringLiteral("A1")) == QLatin1String("s:Item"), qPrintable(name));
            ++opened;
        }
        QVERIFY(opened > 0);
    }

    void thePasswordAloneCanBeCheckedWithoutTheBulkWork()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray file = oc::encryptPackage(randomBytes(5000, 9), QStringLiteral("x"), options);
        const QByteArray info = packageOf(file, "EncryptionInfo");
        QCOMPARE(oc::decryptPackage(info, QByteArray(), QStringLiteral("x"), 0, true, nullptr), oc::CryptResult::Ok);
        QCOMPARE(oc::decryptPackage(info, QByteArray(), QStringLiteral("y"), 0, true, nullptr), oc::CryptResult::WrongPassword);
    }

    // ---- a workbook as the program opens it

    void anEncryptedXlsxOpensWithThePasswordAndSaysWhyItDidNot()
    {
        oc::EncryptOptions options;
        options.spinCount = 50;
        const QByteArray file = oc::encryptPackage(xlsxBook(), QStringLiteral("open sesame"), options);
        {
            Workbook wb;
            QString why;
            QVERIFY(!wb.openBinary(file, &why));
            QVERIFY2(why.contains(QLatin1String("needs a password")), qPrintable(why));
            QCOMPARE(wb.passwordNeed(), PasswordNeed::Needed);
        }
        {
            Limits limits;
            limits.password = QStringLiteral("wrong");
            Workbook wb;
            QString why;
            QVERIFY(!wb.openBinary(file, &why, limits));
            QVERIFY2(why.contains(QLatin1String("password is not correct")), qPrintable(why));
            QCOMPARE(wb.passwordNeed(), PasswordNeed::Wrong);
        }
        Limits limits;
        limits.password = QStringLiteral("open sesame");
        Workbook wb;
        QString why;
        QVERIFY2(wb.openBinary(file, &why, limits), qPrintable(why));
        QCOMPARE(wb.passwordNeed(), PasswordNeed::None);
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(show(s, wb.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:7"));
    }

    void aFileProtectedOnlyAgainstChangesOpensWithNoPassword()
    {
        oc::EncryptOptions options;
        options.spinCount = 50;
        const QByteArray file = oc::encryptPackage(xlsxBook(), oc::defaultPassword(), options);
        Workbook wb;
        QString why;
        QVERIFY2(wb.openBinary(file, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(show(s, wb.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:7"));
    }

    void anEncryptedPackageOfAnotherKindIsStillRefusedAsThatKind()
    {
        oc::EncryptOptions options;
        options.spinCount = 10;
        const QByteArray zip = buildOds(contentXml(table(QStringLiteral("T"), row(floatCell(1)))));
        const QByteArray file = oc::encryptPackage(zip, QStringLiteral("x"), options);
        Limits limits;
        limits.password = QStringLiteral("x");
        Workbook wb;
        QString why;
        QVERIFY(!wb.openBinary(file, &why, limits));
        QVERIFY2(!why.isEmpty(), "a reason");
        QCOMPARE(wb.passwordNeed(), PasswordNeed::None);
    }

    void aProtectedFileOfTheFutureIsSaidToBeProtectedInAWayNotRead()
    {
        // (a version of the information that no program of this one's kind has been told of)
        const QByteArray file = containerOf(QByteArray(64, '\x04'), QByteArray(4096, '\x05'));
        Workbook wb;
        QString why;
        QVERIFY(!wb.openBinary(file, &why));
        QVERIFY2(why.contains(QLatin1String("protected")), qPrintable(why));
        QCOMPARE(wb.passwordNeed(), PasswordNeed::None);
    }

    void theReaderTellsWhatAFileNeeds()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        oc::EncryptOptions options;
        options.spinCount = 50;
        auto write = [&](const QString &name, const QByteArray &bytes) {
            QFile f(dir.filePath(name));
            if (f.open(QIODevice::WriteOnly)) f.write(bytes);
            return f.fileName();
        };
        const QString xlsx = write(QStringLiteral("a.xlsx"), oc::encryptPackage(xlsxBook(), QStringLiteral("pw"), options));
        const QString plain = write(QStringLiteral("b.xlsx"), xlsxBook());
        const QString xls = write(QStringLiteral("c.xls"), compound(protectBiff(biffBook().stream(), Biff::Rc4, QStringLiteral("pw"))));
        const QString ods = write(QStringLiteral("d.ods"), protectOds(odsBook(), OdsMake()));
        for (const QString &path : {xlsx, xls}) {
            QCOMPARE(needOf(path, QString()), PasswordNeed::Needed);
            QCOMPARE(needOf(path, QStringLiteral("no")), PasswordNeed::Wrong);
            QCOMPARE(needOf(path, QStringLiteral("pw")), PasswordNeed::None);
        }
        QCOMPARE(needOf(ods, QString()), PasswordNeed::Needed);
        QCOMPARE(needOf(ods, QStringLiteral("no")), PasswordNeed::Wrong);
        QCOMPARE(needOf(ods, QStringLiteral("test")), PasswordNeed::None);
        QCOMPARE(needOf(plain, QString()), PasswordNeed::None);
        QCOMPARE(needOf(dir.filePath(QStringLiteral("missing.xlsx")), QString()), PasswordNeed::None);
    }

    // ---- the old binary form

    void theObfuscationsNumbersAreTheOnesTheFormatsExampleHas()
    {
        // (a published file's record for the password "abc": its key and its check value)
        QCOMPARE(oc::sheet::xorKeyForTest("abc"), quint16(0x514A));
        QCOMPARE(oc::sheet::xorVerifierForTest("abc"), quint16(0xCC1A));
    }

    void everyBinaryFormOpensWithItsPasswordAndNoOther()
    {
        const struct {
            Biff kind;
            const char *name;
            int keyBits;
        } forms[] = {{Biff::Xor, "xor", 0}, {Biff::Rc4, "rc4", 0}, {Biff::CryptoApi, "cryptoapi 128", 128}, {Biff::CryptoApi, "cryptoapi 40", 40}};
        for (const auto &form : forms) {
            const Book book = biffBook(5);
            const QByteArray stream = protectBiff(book.stream(), form.kind, QStringLiteral("hunter2"), form.keyBits ? form.keyBits : 128);
            QVERIFY2(stream != book.stream(), form.name);
            for (const bool wholeFile : {true, false}) {
                const QByteArray bytes = wholeFile ? compound(stream) : stream;
                Workbook wb;
                QString why;
                QVERIFY2(!wb.openBinary(bytes, &why), form.name);
                QCOMPARE(wb.passwordNeed(), PasswordNeed::Needed);
                Limits wrong;
                wrong.password = QStringLiteral("hunter3");
                Workbook wb2;
                QVERIFY2(!wb2.openBinary(bytes, &why, wrong), form.name);
                QCOMPARE(wb2.passwordNeed(), PasswordNeed::Wrong);
                Limits right;
                right.password = QStringLiteral("hunter2");
                Workbook wb3;
                QVERIFY2(wb3.openBinary(bytes, &why, right), qPrintable(QStringLiteral("%1: %2").arg(QLatin1String(form.name), why)));
                QCOMPARE(wb3.info().sheets.size(), 1);
                QCOMPARE(wb3.info().sheets[0].name, QStringLiteral("One"));
                Sheet s;
                QVERIFY(wb3.readSheet(0, &s));
                QCOMPARE(xlstest::shown(wb3, s, QStringLiteral("A1")), QStringLiteral("n:5"));
                QCOMPARE(xlstest::shown(wb3, s, QStringLiteral("A2")), QStringLiteral("i:hello"));
            }
        }
    }

    void aBinaryFileWithTheDefaultPasswordOpensWithNoneGiven()
    {
        for (const Biff kind : {Biff::Xor, Biff::Rc4, Biff::CryptoApi}) {
            const QByteArray stream = protectBiff(biffBook(9).stream(), kind, oc::defaultPassword());
            Workbook wb;
            QString why;
            QVERIFY2(wb.openBinary(compound(stream), &why), qPrintable(why));
            Sheet s;
            QVERIFY(wb.readSheet(0, &s));
            QCOMPARE(xlstest::shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:9"));
        }
    }

    void aProtectedBinaryFileCutAnywhereIsRefusedOrRead()
    {
        for (const Biff kind : {Biff::Xor, Biff::Rc4, Biff::CryptoApi}) {
            const QByteArray stream = protectBiff(biffBook().stream(), kind, QStringLiteral("pw"));
            Limits limits;
            limits.password = QStringLiteral("pw");
            for (qsizetype n = 0; n <= stream.size(); n += 3) {
                Workbook wb;
                QString why;
                (void)wb.openBinary(stream.left(n), &why, limits);   // (no crash, no hang)
            }
            // and damaged in its FILEPASS record, every byte of it changed in turn
            for (qsizetype i = 4; i < 4 + 4 + 60 && i < stream.size(); ++i) {
                QByteArray changed = stream;
                changed[24 + i % 40] = char(changed[24 + i % 40] ^ 0x55);
                Workbook wb;
                QString why;
                (void)wb.openBinary(changed, &why, limits);
            }
        }
    }

    void aBinaryFilesSheetsKeepTheirPlaces()
    {
        // (the bytes that say where each sheet begins are not enciphered, and the key follows the place of every byte: a workbook of several sheets, across a block boundary)
        Book b;
        QByteArray many;
        for (int i = 0; i < 600; ++i) many += number(i, 0, i) + number(i, 1, i * 2);
        b.sheet(QStringLiteral("A"), dimensions(600, 2) + many);
        b.sheet(QStringLiteral("B"), dimensions(1, 1) + number(0, 0, 77));
        for (const Biff kind : {Biff::Xor, Biff::Rc4, Biff::CryptoApi}) {
            Limits limits;
            limits.password = QStringLiteral("pw");
            Workbook wb;
            QString why;
            QVERIFY2(wb.openBinary(compound(protectBiff(b.stream(), kind, QStringLiteral("pw"))), &why, limits), qPrintable(why));
            Sheet a, bb;
            QVERIFY(wb.readSheet(0, &a));
            QVERIFY(wb.readSheet(1, &bb));
            QCOMPARE(xlstest::shown(wb, a, QStringLiteral("B600")), QStringLiteral("n:1198"));
            QCOMPARE(xlstest::shown(wb, bb, QStringLiteral("A1")), QStringLiteral("n:77"));
        }
    }

    // ---- OpenDocument

    void everyOpenDocumentFormOpensWithItsPassword()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        struct Form {
            const char *name;
            bool blowfish, sha256, omitStart;
        };
        for (const Form &f : {Form{"aes256 sha256", false, true, false}, Form{"aes256 sha1", false, false, false}, Form{"blowfish", true, false, false}, Form{"aes256, no start key named", false, false, true}}) {
            OdsMake make;
            make.blowfish = f.blowfish;
            make.sha256 = f.sha256;
            make.omitStartKey = f.omitStart;
            const QByteArray file = protectOds(odsBook(42), make);
            QVERIFY2(oc::sheet::odsEncrypted(file), f.name);
            Workbook wb;
            QString why;
            QVERIFY2(!openBytes(file, QStringLiteral("a.ods"), &wb, &why), f.name);
            QVERIFY2(why.contains(QLatin1String("needs a password")), qPrintable(why));
            QCOMPARE(wb.passwordNeed(), PasswordNeed::Needed);
            Limits wrong;
            wrong.password = QStringLiteral("nope");
            Workbook wb2;
            QVERIFY2(!openBytes(file, QStringLiteral("a.ods"), &wb2, &why, wrong), f.name);
            QCOMPARE(wb2.passwordNeed(), PasswordNeed::Wrong);
            Limits right;
            right.password = QStringLiteral("test");
            Workbook wb3;
            QVERIFY2(openBytes(file, QStringLiteral("a.ods"), &wb3, &why, right), qPrintable(QStringLiteral("%1: %2").arg(QLatin1String(f.name), why)));
            Sheet s;
            QVERIFY(wb3.readSheet(0, &s));
            QCOMPARE(odstest::shown(wb3, s, QStringLiteral("A1")), QStringLiteral("n:42"));
        }
    }

    void anOpenDocumentPackageWithManyPartsCannotAskForUnlimitedKeyWork()
    {
        OdsMake make;
        make.rounds = 1024;
        QByteArray file = protectOds(odsBook(), make);
        // the manifest says ten million rounds for each of its parts
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(file, parts));
        QByteArray manifest = parts.value(QStringLiteral("META-INF/manifest.xml"));
        manifest.replace("iteration-count=\"1024\"", "iteration-count=\"9000000\"");
        jp::ZipWriter z;
        for (auto it = parts.constBegin(); it != parts.constEnd(); ++it) z.add(it.key(), it.key() == QLatin1String("META-INF/manifest.xml") ? manifest : it.value(), false);
        QElapsedTimer t;
        t.start();
        QByteArray out;
        QCOMPARE(oc::sheet::decryptOds(z.finish(), QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::Damaged);
        QVERIFY(t.elapsed() < 5000);
    }

    void anOpenDocumentPartChangedAfterProtectionIsNotTakenForAWrongPassword()
    {
        const QByteArray file = protectOds(odsBook(), OdsMake());
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(file, parts));
        QByteArray content = parts.value(QStringLiteral("content.xml"));
        content[3] = char(content[3] ^ 8);
        parts[QStringLiteral("content.xml")] = content;
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), parts.value(QStringLiteral("mimetype")), false);
        for (auto it = parts.constBegin(); it != parts.constEnd(); ++it)
            if (it.key() != QLatin1String("mimetype")) z.add(it.key(), it.value(), false);
        QByteArray out;
        // (a part that does not hold what the checksum says is, to the reader, a password that is not the one: the message says "not correct", and a person can try again)
        const oc::CryptResult r = oc::sheet::decryptOds(z.finish(), QStringLiteral("test"), 1 << 30, false, &out);
        QVERIFY(r == oc::CryptResult::WrongPassword || r == oc::CryptResult::Damaged);
    }
};

JP_TEST_CLASS(SheetCryptTests)
#include "tst_sheet_crypt.moc"
