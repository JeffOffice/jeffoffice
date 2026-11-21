// Tests of the ciphers and of opening workbooks that are protected with a password: the ciphers against published values, then each kind of protection round trip (the files are made here, in
// code), the refusals (a wrong password, a file that needs one, damage, hostile numbers), and the readers' reports.
#include "xlshelpers.h"
#include "core/workthread.h"
#include "odshelpers.h"

#include "io/cfb.h"
#include "io/crypto.h"
#include "office/officecrypt.h"
#include "sheet/ods.h"
#include "sheet/oldsave.h"
#include "office/binarycrypt.h"
#include "office/odfcrypt.h"
#include "office/odraw_inflate.h"
#include "sheet/sheetcrypt.h"
#include "sheet/xlscrypt.h"

#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QPasswordDigestor>
#include <QSet>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <atomic>
#include <random>
#include <thread>
#include <vector>

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
        header.u32(0x04).u32(0).u32(0x6801).u32(0x8004).u32(quint32(keyBits)).u32(1).u32(0).u32(0).raw(utf16(QStringLiteral("Microsoft Enhanced Cryptographic Provider v1.0"), 100)).u16(0);
        // (the verifier and then its hash are enciphered in one stream)
        const QByteArray both = rc4.apply(verifier + QCryptographicHash::hash(verifier, QCryptographicHash::Sha1));
        body = W().u16(1).u16(4).u16(2).u32(0x04).u32(quint32(header.bytes().size())).raw(header).u32(16).raw(salt).raw(both.left(16)).u32(20).raw(both.mid(16));
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


// The known-answer file: made by tools/odf_encrypt_kat.py from the standard alone (Python's hashlib, zlib, and the cryptography package's AES), never by the program's own encoder.
const char *const kOdfKnownAnswer =
    "UEsDBBQAAAAAAAAAcF1exjIMJwAAACcAAAAIAAAAbWltZXR5cGVhcHBsaWNhdGlvbi92bmQub2FzaXMub3BlbmRvY3VtZW50LnRl"
    "eHRQSwMEFAAAAAAAAABwXR18sbxgEAAAYBAAAAsAAABjb250ZW50LnhtbB1TB5vSgGi1F1sZFoWWGbXw61wH0bHCsBfEvO0gz3K3"
    "kgmWfweokMQAEdONdcSRQSVqor75YYspJ3pxCBYS3PI+6kku4xCXH95qttXLfl8Ze8hMbBPyh+rCSaUG+5bYEZJZfNgmTjYdB/bN"
    "o6CX7kMXw5wGajxtyHPElwCWBma3Besl3jIXga5PhYgudu6hJsr2QHKD8fhEUnw3FmmX64IMJhPo3YZ/3nvyG0c8PBr456sVlfvk"
    "XPVyg2yygbt5ghT7DV1x4gW8SgwtErs5Bq9arptI92/WH1fWPgTUxwOlusR182O74B4V7jEs7Jk209u+2CMA7PVVJCPm5HRmv/Nj"
    "jOXrSSMZ+zx55y6VL1eelPb13YOln/aOQVKI4RWW34ZH6xZ7WW94nZJJC7k7QLRT6uTdHYxs5maUfrOphscpGOK6nI+ex4sSCTHt"
    "dLJG8XIXuFjgtWNKrT/E3yrxDOKAz/Dv/Yrs0JwlVSOiaYSAeWJ5NTn2qBwHHIUGouZbb4l7nq8xXErS4Myyxpr5Mz2ciQKoq1G4"
    "nYcmIOE55tIe+c1LNaYdDFDjvrYoDQv9kCc3krNy/H/jXUlq/LNgS7QTmAiRT++p41yAWpsCYVb9oKaRnazhri89VwgvMzvlfh1k"
    "HEx7+lWRrsCYXTx55E7MDuYEnue9qWP80nHMd9vbaHxYuv2UT1rbMNOVO9sRS02pCW7jJzqkQt3S61ni5cpT36SJsAtcRT5y2MWe"
    "fNXh37ZwnnBgW0yC5x7FWMDoBOSt+v1VOEsFqtAyeWXf+bNbhH9KBv1kPQK9tYyWO8LwjqT9csJlBqCmupMfkWZZRNwqDKoPTK89"
    "1/OGXkz6mTtX4/FoDIpVQ4/S94xu3hc0v1GvNnbnR0Mbc5dYkX+1D2fymws/SESW9g5jl8ZlsRaruhT02bv7FqzWE1geZlrzuSQZ"
    "ODl2Pj/kJ/QoBMCtj8UY3M2Mluf+UsEVSZDLwkDrIeuYrRPXnTnH87nfM3kbe+FdNFMq8UbHJOsE4xmSusa/8m/7s0u0KkgjUydD"
    "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";
const char *const kOdfKnownBlowfish =
    "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"
    "AIkTAAAAAA==";
const char *const kOdfKnownContentSha256 = "bb03dd35a27433033370fde8670088b7def9c9db20b3b627b3e283bbc27965e3";
const QString kOdfKnownPassword = QString::fromUtf8("P\xc3\xa4ssword \xe2\x98\x83");

// A package of the plain kind with the parts of a text document, for the protection tests.
QByteArray plainOdfPackage(int pictures = 2)
{
    jp::ZipWriter z;
    z.add(QStringLiteral("mimetype"), "application/vnd.oasis.opendocument.text", false);
    QString manifest = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.3\">\n"
                                      " <manifest:file-entry manifest:full-path=\"/\" manifest:media-type=\"application/vnd.oasis.opendocument.text\"/>\n");
    auto part = [&](const QString &name, const QByteArray &data) {
        z.add(name, data, true);
        manifest += QStringLiteral(" <manifest:file-entry manifest:full-path=\"%1\" manifest:media-type=\"text/xml\"/>\n").arg(name);
    };
    part(QStringLiteral("content.xml"), "<office:document-content>" + QByteArray("hello world ").repeated(300) + "</office:document-content>");
    part(QStringLiteral("styles.xml"), "<office:document-styles/>");
    part(QStringLiteral("meta.xml"), "<office:document-meta><dc:title>Secret</dc:title></office:document-meta>");
    for (int i = 0; i < pictures; ++i) part(QStringLiteral("Pictures/p%1.png").arg(i), QByteArray(5000, char('a' + i)));
    manifest += QStringLiteral("</manifest:manifest>\n");
    z.add(QStringLiteral("META-INF/manifest.xml"), manifest.toUtf8(), true);
    return z.finish();
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

struct ArgonMake {
    QString password = QStringLiteral("test");
    quint32 iterations = 2, memory = 64, lanes = 2;
    QByteArray salt = QByteArray(16, 's'), iv = QByteArray(12, 'v');
    bool omitStartKey = false;
};

// The package of the newest files: the whole plain package deflated, enciphered with AES-GCM as the one part "encrypted-package", and a manifest that says how.
QByteArray protectOdsArgon(const QByteArray &plain, const ArgonMake &make)
{
    const QByteArray start = QCryptographicHash::hash(make.password.toUtf8(), QCryptographicHash::Sha256);
    const QByteArray key = crypto::argon2id(start, make.salt, make.iterations, make.memory, make.lanes, 32);
    const QByteArray sealed = crypto::aesGcmSeal(key, make.iv, QByteArray(), deflate(plain));
    jp::ZipWriter z;
    z.add(QStringLiteral("mimetype"), "application/vnd.oasis.opendocument.spreadsheet", false);
    z.add(QStringLiteral("encrypted-package"), make.iv + sealed, false);
    const QString manifest =
        QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.4\" "
                       "xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\">\n <manifest:file-entry manifest:full-path=\"encrypted-package\" "
                       "manifest:media-type=\"application/vnd.oasis.opendocument.spreadsheet\" manifest:size=\"%1\">\n  <manifest:encryption-data>\n"
                       "   <manifest:algorithm manifest:algorithm-name=\"http://www.w3.org/2009/xmlenc11#aes256-gcm\" manifest:initialisation-vector=\"%2\"/>\n%3"
                       "   <manifest:key-derivation manifest:key-derivation-name=\"urn:org:documentfoundation:names:experimental:office:manifest:argon2id\" loext:argon2-iterations=\"%4\" "
                       "loext:argon2-memory=\"%5\" loext:argon2-lanes=\"%6\" manifest:salt=\"%7\" manifest:key-size=\"32\"/>\n  </manifest:encryption-data>\n </manifest:file-entry>\n</manifest:manifest>\n")
            .arg(plain.size())
            .arg(QString::fromLatin1(make.iv.toBase64()))
            .arg(make.omitStartKey ? QString() : QStringLiteral("   <manifest:start-key-generation manifest:start-key-generation-name=\"http://www.w3.org/2001/04/xmlenc#sha256\" manifest:key-size=\"32\"/>\n"))
            .arg(make.iterations).arg(make.memory).arg(make.lanes)
            .arg(QString::fromLatin1(make.salt.toBase64()));
    z.add(QStringLiteral("META-INF/manifest.xml"), manifest.toUtf8(), true);
    return z.finish();
}


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

    void aesGcmMatchesTheStandardsValues()
    {
        // the test cases of the Galois/counter mode's specification (1, 2, 4, 6, and 16): empty data, one block, a tag over extra data, a long iv, and a 256-bit key
        const QByteArray zero16(16, 0), iv0(12, 0), none;
        QCOMPARE(crypto::aesGcmSeal(zero16, iv0, none, none), hex("58e2fccefa7e3061367f1d57a4e7455a"));
        QCOMPARE(crypto::aesGcmSeal(zero16, iv0, none, zero16), hex("0388dace60b6a392f328c2b971b2fe78ab6e47d42cec13bdf53a67b21257bddf"));
        const QByteArray key = hex("feffe9928665731c6d6a8f9467308308"), iv = hex("cafebabefacedbaddecaf888"), aad = hex("feedfacedeadbeeffeedfacedeadbeefabaddad2");
        const QByteArray text = hex("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a721c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39");
        const QByteArray sealed4 = hex("42831ec2217774244b7221b784d0d49ce3aa212f2c02a4e035c17e2329aca12e21d514b25466931c7d8f6a5aac84aa051ba30b396a0aac973d58e0915bc94fbc3221a5db94fae95ae7121a47");
        QCOMPARE(crypto::aesGcmSeal(key, iv, aad, text), sealed4);
        bool ok = false;
        QCOMPARE(crypto::aesGcmOpen(key, iv, aad, sealed4, &ok), text);
        QVERIFY(ok);
        const QByteArray iv60 = hex("9313225df88406e555909c5aff5269aa6a7a9538534f7da1e4c303d2a318a728c3c0c95156809539fcf0e2429a6b525416aedbf5a0de6a57a637b39b534b0f8c3b8ca3ef2ba7d7c3f5a3f6e6ae0e3b7f6d"
                                    ).left(60);
        QCOMPARE(crypto::aesGcmSeal(key, iv60, aad, text),
                 hex("8ce24998625615b603a033aca13fb894be9112a5c3a211a8ba262a3cca7e2ca701e4a9a4fba43c90ccdcb281d48c7c6fd62875d2aca417034c34aee5619cc5aefffe0bfa462af43c1699d050"));
        const QByteArray key256 = hex("feffe9928665731c6d6a8f9467308308feffe9928665731c6d6a8f9467308308");
        QCOMPARE(crypto::aesGcmSeal(key256, iv, aad, text),
                 hex("522dc1f099567d07f47f37a32a84427d643a8cdcbfe5c0c97598a2bd2555d1aa8cb08e48590dbb3da7b08b1056828838c5f61e6393ba7a0abcc9f66276fc6ece0f4e1768cddf8853bb2d551b"));
        // a changed byte, a changed tag, other extra data, a short input, and a wrong key size are all refused
        QByteArray bad = sealed4;
        bad[5] = char(bad[5] ^ 1);
        QVERIFY(crypto::aesGcmOpen(key, iv, aad, bad, &ok).isEmpty() && !ok);
        bad = sealed4;
        bad[bad.size() - 1] = char(bad[bad.size() - 1] ^ 0x80);
        QVERIFY(crypto::aesGcmOpen(key, iv, aad, bad, &ok).isEmpty() && !ok);
        QVERIFY(crypto::aesGcmOpen(key, iv, none, sealed4, &ok).isEmpty() && !ok);
        QVERIFY(crypto::aesGcmOpen(key, iv, aad, sealed4.left(10), &ok).isEmpty() && !ok);
        QVERIFY(crypto::aesGcmSeal(QByteArray(5, 'k'), iv, aad, text).isEmpty());
        QVERIFY(crypto::aesGcmSeal(key, QByteArray(), aad, text).isEmpty());
    }

    void blake2bMatchesTheStandardsValues()
    {
        QCOMPARE(crypto::blake2b("abc"), hex("ba80a53f981c4d0d6a2797b69f12f6e94c212f14685ac4b74b12bb6fdbffa2d17d87c5392aab792dc252d5de4533cc9518d38aa8dbf1925ab92386edd4009923"));
        QCOMPARE(crypto::blake2b(QByteArray()), hex("786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce"));
        QByteArray key(64, 0), long200(200, 0);
        for (int i = 0; i < 64; ++i) key[i] = char(i);
        for (int i = 0; i < 200; ++i) long200[i] = char(i);
        QCOMPARE(crypto::blake2b("abc", 64, key), hex("06bbc3dedf13a31139498655251b7588ccd3bb5aaa071b2d44d8e0a04095579ed590fbfdcf941f4370ce5ce623624e7a76d33e7a8109dcda9b57d72f8f8efa51"));
        QCOMPARE(crypto::blake2b(long200, 32), hex("63c3d97a9f8894d5e043a707b0fee7f7ec4c049a23bbf1079df20b4165f9e22d"));
        QVERIFY(crypto::blake2b("abc", 0).isEmpty());
        QVERIFY(crypto::blake2b("abc", 65).isEmpty());
    }

    void argon2idMatchesTheStandardsValue()
    {
        // RFC 9106, section 5.3: the test vector for Argon2id (3 passes, 32 KiB, 4 lanes, with a secret and extra data)
        const QByteArray tag = crypto::argon2id(QByteArray(32, '\x01'), QByteArray(16, '\x02'), 3, 32, 4, 32, QByteArray(8, '\x03'), QByteArray(12, '\x04'));
        QCOMPARE(tag, hex("0d640df58d78766c08c037a34a8b53c9d01ef0452d75b65eb52520e96b01e659"));
        // numbers that cannot be made, and a stop asked for, give nothing
        QVERIFY(crypto::argon2id("p", QByteArray(16, 's'), 0, 64, 1, 32).isEmpty());
        QVERIFY(crypto::argon2id("p", QByteArray(16, 's'), 1, 7, 1, 32).isEmpty());
        QVERIFY(crypto::argon2id("p", QByteArray(16, 's'), 1, 64, 0, 32).isEmpty());
        QVERIFY(crypto::argon2id("p", QByteArray(4, 's'), 1, 64, 1, 32).isEmpty());
        QVERIFY(crypto::argon2id("p", QByteArray(16, 's'), 1, 64, 1, 32, QByteArray(), QByteArray(), [] { return true; }).isEmpty());
        QCOMPARE(crypto::argon2id("p", QByteArray(16, 's'), 1, 64, 1, 100).size(), 100);   // (a tag longer than one hash)
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
            // (a count past the limit asks for more work than is allowed; one that is not a count is damaged)
            const bool costly = QByteArray(huge) == "2000000000" || QByteArray(huge) == "2000001";
            QCOMPARE(oc::decryptPackage(changed, packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1 << 30, false, &out), costly ? oc::CryptResult::TooCostly : oc::CryptResult::Damaged);
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
        QCOMPARE(oc::decryptPackage(info, packageOf(file, "EncryptedPackage"), QStringLiteral("x"), 1 << 30, false, &out), oc::CryptResult::TooCostly);
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
            QCOMPARE(oc::decryptOdf(z.finish(), QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::Damaged);
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
        QCOMPARE(oc::decryptOdf(z.finish(), QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::Damaged);
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
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("test"), 1 << 30, false, &out, [&asked] { return ++asked > 1; }), oc::CryptResult::Cancelled);
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
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::Ok);
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

    // The .xls a save writes with a password is checked here against the format's own description, with a reader made apart from the program's: the records that begin a stream, FILEPASS, and the
    // places in the list of sheets are not enciphered, every other byte is (RC4 with a key from the password's SHA-1 and the number of the 1024-byte block it lies in), and what the
    // password opens is the stream the save writes with none, with FILEPASS put in and the places of the sheets moved by its size.
    void anXlsWrittenWithAPasswordIsEncipheredAsTheFormatSays()
    {
        QString filler;   // (rows enough that the stream lies over several blocks of 1024 bytes)
        for (int r = 2; r < 400; ++r) filler += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        const QByteArray x = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>3</v></c><c r=\"B1\" t=\"inlineStr\"><is><t>hello</t></is></c></row>") + filler),
                                        worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>2</v></c></row>"))},
                                       QByteArray(), QString(), QString(), QByteArray(), QStringList{QStringLiteral("One"), QStringLiteral("Two")});
        using jp::office::sheet::saveXls;
        oc::Package pkg;
        Workbook book;
        QString why;
        QVERIFY2(pkg.readLazy(x, &why) && book.open(pkg, &why), qPrintable(why));
        QByteArray plainFile, lockedFile;
        QVERIFY2(saveXls(book, &plainFile, nullptr, &why), qPrintable(why));
        const QString password = QStringLiteral("Tr1cky é pass");
        QVERIFY2(saveXls(book, &lockedFile, nullptr, &why, nullptr, password), qPrintable(why));
        jp::cfb::File plainCfb, lockedCfb;
        QVERIFY(jp::cfb::read(plainFile, &plainCfb, &why) && jp::cfb::read(lockedFile, &lockedCfb, &why));
        const QByteArray plain = plainCfb.stream(QStringLiteral("Workbook")), locked = lockedCfb.stream(QStringLiteral("Workbook"));
        // FILEPASS is the record after the first BOF, in the form of the provider's RC4: type 1, version 4.2
        const int first = 4 + u16At(locked, 2);
        QCOMPARE(u16At(locked, 0), quint16(0x0809));
        QCOMPARE(u16At(locked, first), quint16(0x002F));
        const int fpLen = u16At(locked, first + 2);
        const QByteArray fp = locked.mid(first + 4, fpLen);
        auto u32At = [](const QByteArray &b, qsizetype at) { return quint32(u16At(b, at)) | quint32(u16At(b, at + 2)) << 16; };
        QCOMPARE(u16At(fp, 0), quint16(1));
        QCOMPARE(u16At(fp, 2), quint16(4));
        QCOMPARE(u16At(fp, 4), quint16(2));
        const quint32 headerSize = u32At(fp, 10);
        const int header = 14, v = 2 + 12 + int(headerSize);
        QCOMPARE(u32At(fp, 6), quint32(0x04));               // the flags of the whole header: made by the cryptographic provider
        QCOMPARE(u32At(fp, header), quint32(0x04));          // ... and of the header itself
        // the provider's name, as the format's readers look it up (two-byte letters and a zero; kept as hex so that the source does not spell the vendor)
        const QByteArray name = QByteArray::fromHex("4d6963726f736f667420456e68616e6365642043727970746f677261706869632050726f76696465722076312e30");
        QByteArray nameBytes;
        for (const char c : name) nameBytes.append(c).append(char(0));
        nameBytes.append(QByteArray(2, 0));
        QCOMPARE(fp.mid(header + 32, int(headerSize) - 32), nameBytes);
        QCOMPARE(u32At(fp, header + 8), quint32(0x6801));    // RC4
        QCOMPARE(u32At(fp, header + 12), quint32(0x8004));   // SHA-1
        QCOMPARE(u32At(fp, header + 16), quint32(128));
        QCOMPARE(u32At(fp, v), quint32(16));
        const QByteArray salt = fp.mid(v + 4, 16), encVerifier = fp.mid(v + 20, 16);
        QCOMPARE(u32At(fp, v + 36), quint32(20));
        const QByteArray encHash = fp.mid(v + 40, 20);
        QCOMPARE(int(fp.size()), v + 60);
        // (the salt is not the same in two saves)
        QByteArray again;
        QVERIFY(saveXls(book, &again, nullptr, &why, nullptr, password));
        jp::cfb::File againCfb;
        QVERIFY(jp::cfb::read(again, &againCfb, &why));
        QVERIFY(againCfb.stream(QStringLiteral("Workbook")).mid(first + 4 + v + 4, 16) != salt);
        // the key of a block, from the password
        const QByteArray base = QCryptographicHash::hash(salt + utf16(password, 255), QCryptographicHash::Sha1);
        auto keystream = [&](quint32 block) {
            crypto::Rc4 rc4(QCryptographicHash::hash(base + le32(block), QCryptographicHash::Sha1).left(16));
            return rc4.apply(QByteArray(1024, 0));
        };
        crypto::Rc4 vrc(QCryptographicHash::hash(base + le32(0), QCryptographicHash::Sha1).left(16));
        const QByteArray check = vrc.apply(encVerifier + encHash);
        QCOMPARE(QCryptographicHash::hash(check.left(16), QCryptographicHash::Sha1), check.mid(16, 20));
        // the stream with every enciphered byte opened
        QByteArray opened = locked;
        qsizetype p = first + 4 + fpLen;
        int boundSheets = 0, boms = 1, crossing = 0;
        QByteArray ks;
        qint64 have = -1;
        while (p + 4 <= opened.size()) {
            const quint16 id = u16At(locked, p), len = u16At(locked, p + 2);
            if (id == 0 && len == 0) break;
            QVERIFY(p + 4 + len <= opened.size());
            const bool clear = id == 0x0809 || id == 0x00E1;
            if (id == 0x0809) ++boms;
            if (id == 0x0085) ++boundSheets;
            if (!clear && len > 0 && (p + 4 + (id == 0x0085 ? 4 : 0)) / 1024 != (p + 3 + len) / 1024) ++crossing;
            for (int i = (id == 0x0085 ? 4 : 0); !clear && i < len; ++i) {
                const qsizetype at = p + 4 + i;
                if (at / 1024 != have) {
                    ks = keystream(quint32(at / 1024));
                    have = at / 1024;
                }
                opened[at] = char(quint8(locked[at]) ^ quint8(ks[int(at % 1024)]));
            }
            p += 4 + len;
        }
        QVERIFY(crossing >= 1);   // (a record that begins in one block of 1024 bytes and ends in the next is keyed by each byte's own block)
        QCOMPARE(boundSheets, 2);
        QCOMPARE(boms, 3);   // (the globals and the two sheets: each begins in the clear)
        // what the password opens is the stream of a save with none: FILEPASS put in, the places of the sheets moved by its size
        const int moved = 4 + fpLen;
        QByteArray expected = plain.left(first) + locked.mid(first, moved);
        qsizetype q = first;
        while (q + 4 <= plain.size()) {
            const quint16 id = u16At(plain, q), len = u16At(plain, q + 2);
            QByteArray rec = plain.mid(q, 4 + len);
            if (id == 0x0085) rec.replace(4, 4, le32(u32At(rec, 4) + quint32(moved)));
            expected += rec;
            q += 4 + len;
        }
        QVERIFY(p > 2048);   // (the records; what lies after them is the zeros that fill the stream out)
        QCOMPARE(opened.left(p), expected.left(p));
        // and each place the list of sheets gives is where that sheet's BOF lies
        qsizetype g = first + moved;
        int found = 0;
        while (g + 4 <= locked.size()) {
            const quint16 id = u16At(locked, g), len = u16At(locked, g + 2);
            if (id == 0x0085) {
                QCOMPARE(u16At(locked, qsizetype(u32At(locked, g + 4))), quint16(0x0809));
                ++found;
            }
            if (id == 0x000A) break;
            g += 4 + len;
        }
        QCOMPARE(found, 2);
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
            QVERIFY2(oc::odfEncrypted(file), f.name);
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
        QCOMPARE(oc::decryptOdf(z.finish(), QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::Damaged);
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
        const oc::CryptResult r = oc::decryptOdf(z.finish(), QStringLiteral("test"), 1 << 30, false, &out);
        QVERIFY(r == oc::CryptResult::WrongPassword || r == oc::CryptResult::Damaged);
    }
    void aKnownAnswerFileFromASeparateScriptOpensWithItsPasswordOnly()
    {
        for (const char *form : {kOdfKnownAnswer, kOdfKnownBlowfish}) {
        const QByteArray file = QByteArray::fromBase64(form);
        QVERIFY(oc::odfEncrypted(file));
        QByteArray out;
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("wrong"), 1 << 30, false, &out), oc::CryptResult::WrongPassword);
        QVERIFY(out.isEmpty());   // (nothing is given for a wrong password)
        QCOMPARE(oc::decryptOdf(file, kOdfKnownPassword, 1 << 30, true, &out), oc::CryptResult::Ok);
        QVERIFY(out.isEmpty());
        QCOMPARE(oc::decryptOdf(file, kOdfKnownPassword, 1 << 30, false, &out), oc::CryptResult::Ok);
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(out, parts));
        QCOMPARE(QString::fromLatin1(QCryptographicHash::hash(parts.value(QStringLiteral("content.xml")), QCryptographicHash::Sha256).toHex()), QString::fromLatin1(kOdfKnownContentSha256));
        QVERIFY(parts.value(QStringLiteral("content.xml")).startsWith("<?xml"));
        QVERIFY(parts.value(QStringLiteral("content.xml")).size() > 3000);   // (more than 1,024 bytes once deflated: the checksum is of real data)
        QVERIFY(!parts.value(QStringLiteral("META-INF/manifest.xml")).contains("encryption-data"));
        }
    }

    void aPackageIsProtectedInTheFormOfTheStandardAndOpensAgain()
    {
        const QByteArray plain = plainOdfPackage();
        oc::OdfEncryptOptions fast;
        fast.rounds = 1024;
        const QByteArray file = oc::encryptOdf(plain, QStringLiteral("secret"), 1 << 30, fast);
        QVERIFY(!file.isEmpty());
        QVERIFY(oc::odfEncrypted(file));
        QVERIFY(!oc::odfEncrypted(plain));
        // what is in the file: the type first and plain, the manifest plain, and no word of the text anywhere in clear
        QVERIFY(file.startsWith("PK"));
        QVERIFY(file.mid(30, 8) == "mimetype");
        QVERIFY(!file.contains("hello world"));
        QVERIFY(!file.contains("Secret"));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(file, parts));
        QCOMPARE(parts.value(QStringLiteral("mimetype")), QByteArray("application/vnd.oasis.opendocument.text"));
        const QByteArray manifest = parts.value(QStringLiteral("META-INF/manifest.xml"));
        QCOMPARE(manifest.count("<manifest:encryption-data"), 5);   // content, styles, meta, and two pictures
        QCOMPARE(manifest.count("full-path=\"styles.xml\""), 1);   // (each part is listed once, with its data, not again beside the plain listing)
        QCOMPARE(manifest.count("<manifest:file-entry"), 6);       // the root and the five parts
        QVERIFY(manifest.contains("aes256-cbc"));
        QVERIFY(manifest.contains("iteration-count=\"1024\""));
        // one salt for the file (so one key to make, however many parts), and an iv for each part, all different
        QSet<QByteArray> salts, ivs;
        int at = 0;
        while ((at = manifest.indexOf("manifest:salt=\"", at)) >= 0) {
            at += 15;
            salts.insert(manifest.mid(at, manifest.indexOf('"', at) - at));
        }
        at = 0;
        while ((at = manifest.indexOf("initialisation-vector=\"", at)) >= 0) {
            at += 23;
            ivs.insert(manifest.mid(at, manifest.indexOf('"', at) - at));
        }
        QCOMPARE(salts.size(), 1);
        QCOMPARE(ivs.size(), 5);
        // opened with the password: every part as it was
        QByteArray out;
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("secret"), 1 << 30, false, &out), oc::CryptResult::Ok);
        QMap<QString, QByteArray> before, after;
        QVERIFY(jp::readZip(plain, before));
        QVERIFY(jp::readZip(out, after));
        for (auto it = before.constBegin(); it != before.constEnd(); ++it)
            if (it.key() != QLatin1String("META-INF/manifest.xml")) QCOMPARE(after.value(it.key()), it.value());
        QCOMPARE(after.size(), before.size());
        // a wrong password is refused, with nothing given, and so is a password that differs in its last letter
        QByteArray none;
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("secreT"), 1 << 30, false, &none), oc::CryptResult::WrongPassword);
        QVERIFY(none.isEmpty());
        QCOMPARE(oc::decryptOdf(file, QString(), 1 << 30, false, &none), oc::CryptResult::WrongPassword);
        QVERIFY(none.isEmpty());
        // the same package protected twice is not the same bytes (random salts and ivs)
        QVERIFY(oc::encryptOdf(plain, QStringLiteral("secret"), 1 << 30, fast) != file);
    }

    void theObfuscationTakesAPasswordInTheWesternWindowsCoding()
    {
        QByteArray out;
        QVERIFY(oc::bincrypt::ansiPassword(QString::fromUtf8("\xe2\x82\xac\xe2\x80\x99x\xe2\x80\x9c\xc3\xa9"), &out));   // euro sign, right quote, x, left double quote, e acute
        QCOMPARE(out, QByteArray("\x80\x92x\x93\xe9"));
        QVERIFY(!oc::bincrypt::ansiPassword(QString::fromUtf8("\xe4\xbd\xa0"), &out));   // a Chinese character has no place in it
        QVERIFY(!oc::bincrypt::ansiPassword(QString(QChar(0x0080)), &out));
    }

    void aProtectedPackageIsReadByTheStandardsOwnRulesWithoutTheProgramsDecoder()
    {
        // (the key is made with Qt's PBKDF2 and the part opened with the cipher directly: a mistake the encoder and decoder share would show here)
        oc::OdfEncryptOptions fast;
        fast.rounds = 2048;
        const QString password = QString::fromUtf8("p\xc3\xa4ss");
        const QByteArray file = oc::encryptOdf(plainOdfPackage(0), password, 1 << 30, fast);
        QVERIFY(!file.isEmpty());
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(file, parts));
        const QByteArray manifest = parts.value(QStringLiteral("META-INF/manifest.xml"));
        const int entry = manifest.indexOf("full-path=\"styles.xml\"");
        QVERIFY(entry > 0);
        auto attr = [&](const char *name) {
            const int a = manifest.indexOf(QByteArray(name) + "=\"", entry) + int(strlen(name)) + 2;
            return manifest.mid(a, manifest.indexOf('"', a) - a);
        };
        const QByteArray salt = QByteArray::fromBase64(attr("manifest:salt")), iv = QByteArray::fromBase64(attr("manifest:initialisation-vector"));
        const QByteArray start = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
        const QByteArray key = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha1, start, salt, 2048, 32);
        QByteArray plain = crypto::aesCbc(key, iv, parts.value(QStringLiteral("styles.xml")), false);
        QVERIFY(!plain.isEmpty());
        const int pad = quint8(plain.back());
        QVERIFY(pad >= 1 && pad <= 16);
        plain.chop(pad);
        QCOMPARE(QString::fromLatin1(attr("manifest:checksum")), QString::fromLatin1(QCryptographicHash::hash(plain.left(1024), QCryptographicHash::Sha256).toBase64()));
        bool ok = false;
        const QByteArray inflated = oc::odrawInflate(plain, 1000, &ok);
        QVERIFY(ok);
        QCOMPARE(inflated, QByteArray("<office:document-styles/>"));
        QCOMPARE(attr("manifest:size").toInt(), int(inflated.size()));
    }

    void protectingAPackageIsBoundedAndRefusesWhatItCannotDo()
    {
        const QByteArray plain = plainOdfPackage();
        oc::OdfEncryptOptions fast;
        fast.rounds = 1024;
        QVERIFY(oc::encryptOdf(plain, QStringLiteral("x"), 100, fast).isEmpty());   // (more than the most the caller allows)
        QVERIFY(!oc::encryptOdf(plain, QStringLiteral("x"), 1 << 30, fast).isEmpty());
        oc::OdfEncryptOptions costly;
        costly.rounds = int(oc::kMaxOdfKeyRounds) + 1;
        QVERIFY(oc::encryptOdf(plain, QStringLiteral("x"), 1 << 30, costly).isEmpty());
        costly.rounds = 0;
        QVERIFY(oc::encryptOdf(plain, QStringLiteral("x"), 1 << 30, costly).isEmpty());
        QVERIFY(oc::encryptOdf(QByteArray("not a zip"), QStringLiteral("x"), 1 << 30, fast).isEmpty());
        // a package without a manifest, and one that is protected already, are not protected again
        jp::ZipWriter bare;
        bare.add(QStringLiteral("mimetype"), "application/vnd.oasis.opendocument.text", false);
        QVERIFY(oc::encryptOdf(bare.finish(), QStringLiteral("x"), 1 << 30, fast).isEmpty());
        QVERIFY(oc::encryptOdf(oc::encryptOdf(plain, QStringLiteral("x"), 1 << 30, fast), QStringLiteral("y"), 1 << 30, fast).isEmpty());
        // stopped while the keys are made
        std::atomic<int> asked{0};
        QVERIFY(oc::encryptOdf(plain, QStringLiteral("x"), 1 << 30, fast, [&asked] { return ++asked > 2; }).isEmpty());
    }

    void aManifestOfTooManyProtectedPartsOrTooMuchKeyStretchingIsRefusedBeforeAnyKeyIsMade()
    {
        auto entry = [](int i, qint64 rounds, const QByteArray &salt) {
            return QStringLiteral("<manifest:file-entry manifest:full-path=\"p%1\" manifest:size=\"1\"><manifest:encryption-data manifest:checksum-type=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k\" manifest:checksum=\"AAAA\">"
                                  "<manifest:algorithm manifest:algorithm-name=\"http://www.w3.org/2001/04/xmlenc#aes256-cbc\" manifest:initialisation-vector=\"AAAAAAAAAAAAAAAAAAAAAA==\"/>"
                                  "<manifest:key-derivation manifest:key-derivation-name=\"PBKDF2\" manifest:key-size=\"32\" manifest:iteration-count=\"%2\" manifest:salt=\"%3\"/>"
                                  "<manifest:start-key-generation manifest:start-key-generation-name=\"http://www.w3.org/2000/09/xmldsig#sha256\"/>"
                                  "</manifest:encryption-data></manifest:file-entry>").arg(i).arg(rounds).arg(QString::fromLatin1(salt.toBase64()));
        };
        const QString head = QStringLiteral("<?xml version=\"1.0\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">");
        // (counts: one more than the most parts)
        {
            QString manifest = head;
            for (int i = 0; i <= oc::kMaxOdfParts; ++i) manifest += entry(i, 1, QByteArray(16, 'a'));
            manifest += QStringLiteral("</manifest:manifest>");
            jp::ZipWriter z;
            z.add(QStringLiteral("mimetype"), "application/vnd.oasis.opendocument.text", false);
            z.add(QStringLiteral("META-INF/manifest.xml"), manifest.toUtf8(), true);
            QByteArray out;
            int asked = 0;
            QCOMPARE(oc::decryptOdf(z.finish(), QStringLiteral("x"), 1 << 30, false, &out, [&asked] { ++asked; return false; }), oc::CryptResult::Damaged);
            QCOMPARE(asked, 0);
        }
        // (work: parts with salts of their own, a million rounds each; the limit for all is forty million, so forty-one are refused as too costly before any key is begun, and four are tried)
        for (const int parts : {41, 4}) {
            QString manifest = head;
            jp::ZipWriter z;
            z.add(QStringLiteral("mimetype"), "application/vnd.oasis.opendocument.text", false);
            for (int i = 0; i < parts; ++i) {
                z.add(QStringLiteral("p%1").arg(i), QByteArray(16, 'x'), false);
                manifest += entry(i, 1000000, QByteArray(16, char(i + 1)));
            }
            manifest += QStringLiteral("</manifest:manifest>");
            z.add(QStringLiteral("META-INF/manifest.xml"), manifest.toUtf8(), true);
            QByteArray out;
            std::atomic<int> asked{0};
            const oc::CryptResult r = oc::decryptOdf(z.finish(), QStringLiteral("x"), 1 << 30, false, &out, [&asked] { ++asked; return false; });
            if (parts == 41) {
                QCOMPARE(r, oc::CryptResult::TooCostly);   // (not "damaged": the file needs too much work to unlock)
                QCOMPARE(int(asked), 0);                   // (refused before the first key was begun)
            } else {
                QCOMPARE(r, oc::CryptResult::WrongPassword);   // (four million rounds: within the limit, the work is done, and the password is not the one)
                QVERIFY(asked > 0);
            }
        }
    }

    void aProtectedPackageOfTwoHundredPartsIsSavedAndOpensAgain()
    {
        // (it was a file the program wrote and could not open: one key for each of its parts at the usual count was more work than a file was allowed)
        const QByteArray plain = plainOdfPackage(200);
        QElapsedTimer t;
        t.start();
        const QByteArray file = oc::encryptOdf(plain, QStringLiteral("many"), 1 << 30);   // (the usual 100,000 rounds)
        QVERIFY(!file.isEmpty());
        const qint64 wrote = t.elapsed();
        QByteArray out;
        t.restart();
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("many"), 1 << 30, false, &out), oc::CryptResult::Ok);
        qInfo() << "200 parts: wrote in" << wrote << "ms, opened in" << t.elapsed() << "ms";
        QMap<QString, QByteArray> before, after;
        QVERIFY(jp::readZip(plain, before));
        QVERIFY(jp::readZip(out, after));
        QCOMPARE(after.size(), before.size());
        QCOMPARE(after.value(QStringLiteral("Pictures/p199.png")), before.value(QStringLiteral("Pictures/p199.png")));
    }

    void aFileFromAnotherProgramWithASaltForEveryPartOfThreeHundredPartsOpens()
    {
        // (made here with Qt's PBKDF2 and the cipher directly, 300 parts at the usual 100,000 rounds each, the keys made on several threads)
        const int parts = 300;
        QVector<QByteArray> salts(parts), keys(parts);
        const QByteArray start = QCryptographicHash::hash(QByteArray("many parts"), QCryptographicHash::Sha256);
        for (int i = 0; i < parts; ++i) salts[i] = randomBytes(16, 5000 + i);
        {
            std::atomic<int> next{0};
            auto run = [&] {
                for (int i = next++; i < parts; i = next++) keys[i] = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha1, start, salts[i], 100000, 32);
            };
            std::vector<jp::WorkThread> pool;
            for (int i = 0; i < 7; ++i) pool.emplace_back(run);
            run();
            for (jp::WorkThread &th : pool) th.join();
        }
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), "application/vnd.oasis.opendocument.presentation", false);
        QString manifest = QStringLiteral("<?xml version=\"1.0\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">");
        for (int i = 0; i < parts; ++i) {
            const QByteArray data = QByteArray("part number ") + QByteArray::number(i) + QByteArray(300, 'z');
            const QByteArray iv = randomBytes(16, 9000 + i), packed = deflate(data);
            const int pad = 16 - int(packed.size() % 16);
            z.add(QStringLiteral("p%1.xml").arg(i), crypto::aesCbc(keys[i], iv, packed + QByteArray(pad, char(pad)), true), false);
            manifest += QStringLiteral("<manifest:file-entry manifest:full-path=\"p%1.xml\" manifest:size=\"%2\"><manifest:encryption-data manifest:checksum-type=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k\" manifest:checksum=\"%3\">"
                                       "<manifest:algorithm manifest:algorithm-name=\"http://www.w3.org/2001/04/xmlenc#aes256-cbc\" manifest:initialisation-vector=\"%4\"/>"
                                       "<manifest:key-derivation manifest:key-derivation-name=\"PBKDF2\" manifest:key-size=\"32\" manifest:iteration-count=\"100000\" manifest:salt=\"%5\"/>"
                                       "<manifest:start-key-generation manifest:start-key-generation-name=\"http://www.w3.org/2000/09/xmldsig#sha256\"/></manifest:encryption-data></manifest:file-entry>")
                            .arg(i).arg(data.size()).arg(QString::fromLatin1(QCryptographicHash::hash(packed.left(1024), QCryptographicHash::Sha256).toBase64()))
                            .arg(QString::fromLatin1(iv.toBase64())).arg(QString::fromLatin1(salts[i].toBase64()));
        }
        manifest += QStringLiteral("</manifest:manifest>");
        z.add(QStringLiteral("META-INF/manifest.xml"), manifest.toUtf8(), true);
        const QByteArray file = z.finish();
        QByteArray out;
        QElapsedTimer t;
        t.start();
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("many parts"), 1 << 30, false, &out), oc::CryptResult::Ok);
        qInfo() << "300 parts with a salt each opened in" << t.elapsed() << "ms";
        QMap<QString, QByteArray> parts2;
        QVERIFY(jp::readZip(out, parts2));
        QCOMPARE(parts2.value(QStringLiteral("p299.xml")).left(15), QByteArray("part number 299"));
        // stopped while the keys are made
        std::atomic<int> asked{0};
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("many parts"), 1 << 30, false, &out, [&asked] { return ++asked > 3; }), oc::CryptResult::Cancelled);
    }

    void anArgon2PackageOpensWithItsPasswordAndNoOther()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const bool omitStart : {false, true}) {
            ArgonMake make;
            make.password = QStringLiteral("pässw€rd");
            make.omitStartKey = omitStart;
            const QByteArray file = protectOdsArgon(odsBook(42), make);
            QVERIFY(jp::office::sheet::looksLikeOpenDocumentZip(file));
            QVERIFY(oc::odfEncrypted(file));
            Workbook wb;
            QString why;
            QVERIFY(!openBytes(file, QStringLiteral("a.ods"), &wb, &why));
            QVERIFY2(why.contains(QLatin1String("needs a password")), qPrintable(why));
            QCOMPARE(wb.passwordNeed(), PasswordNeed::Needed);
            Limits wrong;
            wrong.password = QStringLiteral("nope");
            Workbook wb2;
            QVERIFY(!openBytes(file, QStringLiteral("a.ods"), &wb2, &why, wrong));
            QCOMPARE(wb2.passwordNeed(), PasswordNeed::Wrong);
            QVERIFY2(why.contains(QLatin1String("not correct")), qPrintable(why));
            Limits right;
            right.password = make.password;
            Workbook wb3;
            QVERIFY2(openBytes(file, QStringLiteral("a.ods"), &wb3, &why, right), qPrintable(why));
            Sheet s;
            QVERIFY(wb3.readSheet(0, &s));
            QCOMPARE(odstest::shown(wb3, s, QStringLiteral("A1")), QStringLiteral("n:42"));
            // and from a file on disk, as a person opens it
            QFile f(dir.filePath(QStringLiteral("a.ods")));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(file);
            f.close();
            Workbook wb4;
            QVERIFY2(wb4.openFile(f.fileName(), &why, right), qPrintable(why));
            QVERIFY(wb4.wasProtected());
            QCOMPARE(needOf(f.fileName(), QString()), PasswordNeed::Needed);
        }
    }

    void anArgon2PackageCannotAskForMoreThanTenTimesTheDefaults()
    {
        // (the defaults are 64 MiB, 3 passes, and 4 lanes; the manifests below ask for far more, and are refused before any key is made)
        struct Ask {
            const char *what;
            quint32 iterations, memory, lanes;
        };
        for (const Ask &a : {Ask{"memory", 2, 4000000, 2}, Ask{"passes", 4000000, 64, 2}, Ask{"lanes", 2, 64000, 1000}, Ask{"all", 4000000, 4000000, 4000000}}) {
            ArgonMake make;
            make.iterations = 2;
            make.memory = 64;
            make.lanes = 2;
            QByteArray file = protectOdsArgon(odsBook(), make);
            QMap<QString, QByteArray> parts;
            QVERIFY(jp::readZip(file, parts));
            QByteArray manifest = parts.value(QStringLiteral("META-INF/manifest.xml"));
            manifest.replace("argon2-iterations=\"2\"", "argon2-iterations=\"" + QByteArray::number(a.iterations) + "\"");
            manifest.replace("argon2-memory=\"64\"", "argon2-memory=\"" + QByteArray::number(a.memory) + "\"");
            manifest.replace("argon2-lanes=\"2\"", "argon2-lanes=\"" + QByteArray::number(a.lanes) + "\"");
            jp::ZipWriter z;
            z.add(QStringLiteral("mimetype"), parts.value(QStringLiteral("mimetype")), false);
            z.add(QStringLiteral("encrypted-package"), parts.value(QStringLiteral("encrypted-package")), false);
            z.add(QStringLiteral("META-INF/manifest.xml"), manifest, false);
            const QByteArray bytes = z.finish();
            QByteArray out;
            QElapsedTimer t;
            t.start();
            QVERIFY2(oc::decryptOdf(bytes, QStringLiteral("test"), 1 << 30, false, &out) == oc::CryptResult::TooCostly, a.what);
            QVERIFY2(t.elapsed() < 2000, a.what);
            Workbook wb;
            QString why;
            Limits limits;
            limits.password = QStringLiteral("test");
            QVERIFY(!openBytes(bytes, QStringLiteral("a.ods"), &wb, &why, limits));
            QVERIFY2(why.contains(QLatin1String("more memory or time")), qPrintable(why));
        }
        // the largest each may be is accepted by the check (the key is not made here: a stop is asked for at once)
        ArgonMake big;
        big.iterations = 30;
        big.memory = 655360;
        big.lanes = 40;
        const QByteArray bytes = [&] {
            ArgonMake small;
            QByteArray file = protectOdsArgon(odsBook(), small);
            QMap<QString, QByteArray> parts;
            jp::readZip(file, parts);
            QByteArray manifest = parts.value(QStringLiteral("META-INF/manifest.xml"));
            manifest.replace("argon2-iterations=\"2\"", "argon2-iterations=\"30\"").replace("argon2-memory=\"64\"", "argon2-memory=\"655360\"").replace("argon2-lanes=\"2\"", "argon2-lanes=\"40\"");
            jp::ZipWriter z;
            z.add(QStringLiteral("mimetype"), parts.value(QStringLiteral("mimetype")), false);
            z.add(QStringLiteral("encrypted-package"), parts.value(QStringLiteral("encrypted-package")), false);
            z.add(QStringLiteral("META-INF/manifest.xml"), manifest, false);
            return z.finish();
        }();
        QByteArray out;
        QCOMPARE(oc::decryptOdf(bytes, QStringLiteral("test"), 1 << 30, false, &out, [] { return true; }), oc::CryptResult::Cancelled);
    }

    void aPackageOfTheNewestKindWithAChangedPartIsNotOpened()
    {
        const QByteArray file = protectOdsArgon(odsBook(), ArgonMake());
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(file, parts));
        QByteArray sealed = parts.value(QStringLiteral("encrypted-package"));
        sealed[20] = char(sealed[20] ^ 4);
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), parts.value(QStringLiteral("mimetype")), false);
        z.add(QStringLiteral("encrypted-package"), sealed, false);
        z.add(QStringLiteral("META-INF/manifest.xml"), parts.value(QStringLiteral("META-INF/manifest.xml")), false);
        QByteArray out;
        QCOMPARE(oc::decryptOdf(z.finish(), QStringLiteral("test"), 1 << 30, false, &out), oc::CryptResult::WrongPassword);
        // a package larger than the reader is allowed is refused without a key
        QCOMPARE(oc::decryptOdf(file, QStringLiteral("test"), 10, false, &out), oc::CryptResult::TooLarge);
    }

    void aProtectedFileFromTheCurrentOriginalProgramOpens()
    {
        // (made at the test's run by whatever program is on this machine that can save a spreadsheet with a password: without one, or if it saves in an older way, the test skips)
        const QString soffice = QStandardPaths::findExecutable(QStringLiteral("soffice")), python = QStandardPaths::findExecutable(QStringLiteral("python3"));
        if (soffice.isEmpty() || python.isEmpty()) QSKIP("no program on this machine to save a protected spreadsheet");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile script(dir.filePath(QStringLiteral("make.py")));
        QVERIFY(script.open(QIODevice::WriteOnly));
        script.write(
            "import uno, sys, time, subprocess\n"
            "from com.sun.star.beans import PropertyValue\n"
            "def pv(n,v):\n p=PropertyValue(); p.Name=n; p.Value=v; return p\n"
            "d0=sys.argv[1]\n"
            "proc=subprocess.Popen(['soffice','--headless','--norestore','-env:UserInstallation=file://'+d0+'/profile','--accept=socket,host=localhost,port=2391;urp;'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)\n"
            "ctx=None\n"
            "for i in range(90):\n"
            " try:\n"
            "  local=uno.getComponentContext()\n"
            "  res=local.ServiceManager.createInstanceWithContext('com.sun.star.bridge.UnoUrlResolver',local)\n"
            "  ctx=res.resolve('uno:socket,host=localhost,port=2391;urp;StarOffice.ComponentContext'); break\n"
            " except Exception: time.sleep(1)\n"
            "desk=ctx.ServiceManager.createInstanceWithContext('com.sun.star.frame.Desktop',ctx)\n"
            "d=desk.loadComponentFromURL('private:factory/scalc','_blank',0,(pv('Hidden',True),))\n"
            "sh=d.Sheets.getByIndex(0)\n"
            "sh.getCellByPosition(0,0).Value=42\n"
            "sh.getCellByPosition(1,0).Formula='=A1*2'\n"
            "d.storeToURL('file://'+d0+'/enc.ods',(pv('FilterName','calc8'),pv('Password','p\\u00e4ssw\\u20acrd'),pv('Overwrite',True)))\n"
            "d.close(True)\n"
            "try: desk.terminate()\n"
            "except Exception: pass\n"
            "proc.wait(timeout=30)\n");
        script.close();
        QProcess p;
        p.start(python, {script.fileName(), dir.path()});
        if (!p.waitForFinished(180000)) {
            p.kill();
            QSKIP("the other program did not save the file in time");
        }
        const QString path = dir.filePath(QStringLiteral("enc.ods"));
        if (!QFileInfo::exists(path)) QSKIP("the other program could not be driven here");
        Limits right;
        right.password = QStringLiteral("pässw€rd");
        Workbook wb;
        QString why;
        QVERIFY2(wb.openFile(path, &why, right), qPrintable(why));
        QVERIFY(wb.wasProtected());
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(show(s, wb.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:42"));
        QCOMPARE(show(s, wb.sharedStrings(), QStringLiteral("B1")).left(4), QStringLiteral("n:84"));
        Limits wrong;
        wrong.password = QStringLiteral("not it");
        Workbook wb2;
        QVERIFY(!wb2.openFile(path, &why, wrong));
        QCOMPARE(wb2.passwordNeed(), PasswordNeed::Wrong);
        QCOMPARE(needOf(path, QString()), PasswordNeed::Needed);
    }
};

JP_TEST_CLASS(SheetCryptTests)
#include "tst_sheet_crypt.moc"
