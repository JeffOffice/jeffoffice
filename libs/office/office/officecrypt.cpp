// The protection of packages; see officecrypt.h. The forms are those of the published specification of Office file encryption (agile: version 4.4; standard: 3.2 and 4.2).

#include "office/officecrypt.h"

#include "io/cfb.h"
#include "io/crypto.h"

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QXmlStreamReader>

#include <algorithm>

namespace jp::office {

QString defaultPassword() { return QStringLiteral("VelvetSweatshop"); }

namespace {

constexpr qint64 kSegment = 4096;

QByteArray utf16le(const QString &text)
{
    QByteArray out;
    out.reserve(text.size() * 2);
    for (const QChar c : text) {
        out.append(char(c.unicode() & 0xFF));
        out.append(char(c.unicode() >> 8));
    }
    return out;
}

QByteArray le32(quint32 v)
{
    QByteArray b(4, 0);
    for (int i = 0; i < 4; ++i) b[i] = char((v >> (8 * i)) & 0xFF);
    return b;
}

quint32 getLe32(const QByteArray &b, qsizetype at) { return quint32(quint8(b[at])) | quint32(quint8(b[at + 1])) << 8 | quint32(quint8(b[at + 2])) << 16 | quint32(quint8(b[at + 3])) << 24; }
quint64 getLe64(const QByteArray &b, qsizetype at) { return quint64(getLe32(b, at)) | quint64(getLe32(b, at + 4)) << 32; }

bool algorithmOf(const QString &name, QCryptographicHash::Algorithm *alg, int *size)
{
    const QString n = name.toUpper();
    if (n == QLatin1String("SHA1") || n == QLatin1String("SHA-1")) *alg = QCryptographicHash::Sha1, *size = 20;
    else if (n == QLatin1String("SHA256")) *alg = QCryptographicHash::Sha256, *size = 32;
    else if (n == QLatin1String("SHA384")) *alg = QCryptographicHash::Sha384, *size = 48;
    else if (n == QLatin1String("SHA512")) *alg = QCryptographicHash::Sha512, *size = 64;
    else return false;
    return true;
}

QByteArray fitTo(QByteArray b, int size)
{
    if (b.size() > size) b.truncate(size);
    else if (b.size() < size) b.append(QByteArray(size - b.size(), char(0x36)));
    return b;
}

// ---- agile

struct Encryptor {
    qint64 spinCount = 0;
    int saltSize = 0, blockSize = 0, keyBits = 0, hashSize = 0;
    QCryptographicHash::Algorithm hash = QCryptographicHash::Sha512;
    QByteArray salt, verifierInput, verifierValue, keyValue;
};

struct AgileInfo {
    int saltSize = 0, blockSize = 0, keyBits = 0, hashSize = 0;
    QCryptographicHash::Algorithm hash = QCryptographicHash::Sha512;
    QByteArray salt;
    QByteArray hmacKey, hmacValue;
    bool integrity = false;
    Encryptor password;
    bool havePassword = false;
};

QByteArray b64(const QXmlStreamAttributes &a, const char *name) { return QByteArray::fromBase64(a.value(QLatin1String(name)).toLatin1()); }
int num(const QXmlStreamAttributes &a, const char *name) { return a.value(QLatin1String(name)).toInt(); }

// 0 = fine, else the result to give
CryptResult readCipher(const QXmlStreamAttributes &a, int *saltSize, int *blockSize, int *keyBits, int *hashSize, QCryptographicHash::Algorithm *hash)
{
    if (a.value(QLatin1String("cipherAlgorithm")).compare(QLatin1String("AES"), Qt::CaseInsensitive) != 0 || a.value(QLatin1String("cipherChaining")) != QLatin1String("ChainingModeCBC"))
        return CryptResult::Unsupported;
    *saltSize = num(a, "saltSize");
    *blockSize = num(a, "blockSize");
    *keyBits = num(a, "keyBits");
    *hashSize = num(a, "hashSize");
    int expected = 0;
    if (!algorithmOf(a.value(QLatin1String("hashAlgorithm")).toString(), hash, &expected)) return CryptResult::Unsupported;
    if (*blockSize != 16 || (*keyBits != 128 && *keyBits != 192 && *keyBits != 256)) return CryptResult::Unsupported;
    if (*hashSize != expected || *saltSize < 1 || *saltSize > 1024) return CryptResult::Damaged;
    return CryptResult::Ok;
}

CryptResult parseAgile(const QByteArray &info, AgileInfo *out)
{
    QXmlStreamReader xml(QByteArrayView(info).mid(8));
    bool keyData = false;
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        const QXmlStreamAttributes a = xml.attributes();
        const QStringView name = xml.name();
        if (name == QLatin1String("keyData")) {
            const CryptResult r = readCipher(a, &out->saltSize, &out->blockSize, &out->keyBits, &out->hashSize, &out->hash);
            if (r != CryptResult::Ok) return r;
            out->salt = b64(a, "saltValue");
            keyData = true;
        } else if (name == QLatin1String("dataIntegrity")) {
            out->hmacKey = b64(a, "encryptedHmacKey");
            out->hmacValue = b64(a, "encryptedHmacValue");
            out->integrity = true;
        } else if (name == QLatin1String("encryptedKey") && !out->havePassword) {
            Encryptor &e = out->password;
            const CryptResult r = readCipher(a, &e.saltSize, &e.blockSize, &e.keyBits, &e.hashSize, &e.hash);
            if (r != CryptResult::Ok) return r;
            bool spinOk = false;
            e.spinCount = a.value(QLatin1String("spinCount")).toLongLong(&spinOk);
            if (!spinOk) return CryptResult::Damaged;
            e.salt = b64(a, "saltValue");
            e.verifierInput = b64(a, "encryptedVerifierHashInput");
            e.verifierValue = b64(a, "encryptedVerifierHashValue");
            e.keyValue = b64(a, "encryptedKeyValue");
            out->havePassword = true;
        }
    }
    if (xml.hasError() || !keyData) return CryptResult::Damaged;
    if (!out->havePassword) return CryptResult::Unsupported;   // (a key held by a certificate)
    const Encryptor &e = out->password;
    if (e.spinCount < 0 || e.spinCount > kMaxPackageSpinCount) return CryptResult::Damaged;
    if (out->salt.size() != out->saltSize || e.salt.size() != e.saltSize) return CryptResult::Damaged;
    const qsizetype keyBytes = e.keyBits / 8;
    if (e.verifierInput.isEmpty() || e.verifierInput.size() % 16 || e.verifierValue.isEmpty() || e.verifierValue.size() % 16 || e.keyValue.size() % 16 || e.keyValue.size() < keyBytes) return CryptResult::Damaged;
    if (e.verifierInput.size() < e.saltSize || e.verifierValue.size() < e.hashSize) return CryptResult::Damaged;
    if (out->integrity && (out->hmacKey.size() < out->hashSize || out->hmacKey.size() % 16 || out->hmacValue.size() < out->hashSize || out->hmacValue.size() % 16)) return CryptResult::Damaged;
    return CryptResult::Ok;
}

QByteArray hashWith(QCryptographicHash::Algorithm alg, const QByteArray &a, const QByteArray &b = QByteArray())
{
    QCryptographicHash h(alg);
    h.addData(a);
    if (!b.isEmpty()) h.addData(b);
    return h.result();
}

// The hash of the salt and the password, spun round `spin` times.
QByteArray spun(QCryptographicHash::Algorithm alg, const QByteArray &salt, const QString &password, qint64 spin, const std::function<bool()> &cancelled = nullptr, bool *stopped = nullptr)
{
    QByteArray pw = utf16le(password);
    QByteArray h = hashWith(alg, salt, pw);
    crypto::wipe(pw);
    QCryptographicHash round(alg);
    for (qint64 i = 0; i < spin; ++i) {
        if ((i & 0x3FFF) == 0 && cancelled && cancelled()) {
            if (stopped) *stopped = true;
            return QByteArray();
        }
        round.reset();
        round.addData(le32(quint32(i)));
        round.addData(h);
        h = round.result();
    }
    return h;
}

const QByteArray kBlockVerifierInput = QByteArray::fromHex("fea7d2763b4b9e79");
const QByteArray kBlockVerifierValue = QByteArray::fromHex("d7aa0f6d3061344e");
const QByteArray kBlockKeyValue = QByteArray::fromHex("146e0be7abacd0d6");
const QByteArray kBlockHmacKey = QByteArray::fromHex("5fb2ad010cb9e1f6");
const QByteArray kBlockHmacValue = QByteArray::fromHex("a0677f02b22c8433");

QByteArray agileStep(const AgileInfo &info, const QByteArray &h, const QByteArray &blockKey)
{
    return fitTo(hashWith(info.password.hash, h, blockKey), info.password.keyBits / 8);
}

// The secret key, or empty if the password is not the one.
QByteArray agileSecretKey(const AgileInfo &info, const QString &password, const std::function<bool()> &cancelled, bool *stopped)
{
    const Encryptor &e = info.password;
    QByteArray h = spun(e.hash, e.salt, password, e.spinCount, cancelled, stopped);
    if (*stopped) return QByteArray();
    const QByteArray iv = fitTo(e.salt, 16);
    QByteArray k1 = agileStep(info, h, kBlockVerifierInput), k2 = agileStep(info, h, kBlockVerifierValue), k3 = agileStep(info, h, kBlockKeyValue);
    crypto::wipe(h);
    QByteArray input = crypto::aesCbc(k1, iv, e.verifierInput, false);
    QByteArray value = crypto::aesCbc(k2, iv, e.verifierValue, false);
    QByteArray secret;
    if (!input.isEmpty() && !value.isEmpty() && hashWith(e.hash, input.left(e.saltSize)) == value.left(e.hashSize)) secret = crypto::aesCbc(k3, iv, e.keyValue, false).left(e.keyBits / 8);
    for (QByteArray *k : {&k1, &k2, &k3, &input, &value}) crypto::wipe(*k);
    return secret;
}

QByteArray segmentIv(const AgileInfo &info, quint32 index) { return fitTo(hashWith(info.hash, info.salt, le32(index)), 16); }

CryptResult decryptAgile(const AgileInfo &info, const QByteArray &package, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *zip, const std::function<bool()> &cancelled)
{
    bool stopped = false;
    QByteArray secret = agileSecretKey(info, password, cancelled, &stopped);
    if (stopped) return CryptResult::Cancelled;
    if (secret.isEmpty()) return CryptResult::WrongPassword;
    if (verifyOnly) {
        crypto::wipe(secret);
        return CryptResult::Ok;
    }
    if (package.size() < 8) {
        crypto::wipe(secret);
        return CryptResult::Damaged;
    }
    const quint64 size = getLe64(package, 0);
    const qint64 data = package.size() - 8;
    if (size > quint64(data)) {
        crypto::wipe(secret);
        return CryptResult::Damaged;
    }
    if (qint64(size) > maxBytes) {
        crypto::wipe(secret);
        return CryptResult::TooLarge;
    }
    // (the check that nothing was changed comes first: the whole stream is what the code covers)
    if (info.integrity) {
        const QByteArray salt = fitTo(hashWith(info.hash, info.salt, kBlockHmacKey), 16), saltV = fitTo(hashWith(info.hash, info.salt, kBlockHmacValue), 16);
        QByteArray key = crypto::aesCbc(secret, salt, info.hmacKey, false).left(info.hashSize), expected = crypto::aesCbc(secret, saltV, info.hmacValue, false).left(info.hashSize);
        const QByteArray actual = QMessageAuthenticationCode::hash(package, key, info.hash);
        crypto::wipe(key);
        if (actual != expected) {
            crypto::wipe(secret);
            return CryptResult::Damaged;
        }
    }
    const qint64 whole = data - data % 16;
    QByteArray out;
    out.reserve(qsizetype(whole));
    for (qint64 at = 0, index = 0; at < whole; at += kSegment, ++index) {
        const qint64 n = std::min(kSegment, whole - at);
        const QByteArray piece = crypto::aesCbc(secret, segmentIv(info, quint32(index)), QByteArray::fromRawData(package.constData() + 8 + at, qsizetype(n)), false);
        if (piece.isEmpty()) {
            crypto::wipe(secret);
            return CryptResult::Damaged;
        }
        out.append(piece);
    }
    crypto::wipe(secret);
    if (qint64(size) > out.size()) return CryptResult::Damaged;
    out.truncate(qsizetype(size));
    *zip = std::move(out);
    return CryptResult::Ok;
}

// ---- standard

QByteArray standardKey(const QByteArray &salt, const QString &password, int keyBytes)
{
    QByteArray pw = utf16le(password);
    QByteArray h = hashWith(QCryptographicHash::Sha1, salt, pw);
    crypto::wipe(pw);
    QCryptographicHash round(QCryptographicHash::Sha1);
    for (quint32 i = 0; i < 50000; ++i) {
        round.reset();
        round.addData(le32(i));
        round.addData(h);
        h = round.result();
    }
    const QByteArray fin = hashWith(QCryptographicHash::Sha1, h, le32(0));
    QByteArray a(64, char(0x36)), b(64, char(0x5C));
    for (int i = 0; i < fin.size(); ++i) {
        a[i] = char(a[i] ^ fin[i]);
        b[i] = char(b[i] ^ fin[i]);
    }
    QByteArray key = hashWith(QCryptographicHash::Sha1, a) + hashWith(QCryptographicHash::Sha1, b);
    crypto::wipe(a);
    crypto::wipe(b);
    crypto::wipe(h);
    key.truncate(keyBytes);
    return key;
}

CryptResult decryptStandard(const QByteArray &info, const QByteArray &package, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *zip)
{
    if (info.size() < 12 + 32 + 4) return CryptResult::Damaged;
    const quint32 headerSize = getLe32(info, 8);
    if (headerSize < 32 || headerSize > 4096 || qint64(12) + headerSize + 4 + 16 + 16 + 4 > info.size()) return CryptResult::Damaged;
    const quint32 flags = getLe32(info, 12), algId = getLe32(info, 20), hashId = getLe32(info, 24), keySize = getLe32(info, 28);
    // (only the cipher of AES; the form that names RC4 here was never used for packages)
    const bool aes = algId == 0x660E || algId == 0x660F || algId == 0x6610;
    if (!(flags & 0x20) || !aes || hashId != 0x8004) return CryptResult::Unsupported;
    const quint32 want = algId == 0x660E ? 128 : algId == 0x660F ? 192 : 256;
    if (keySize != want) return CryptResult::Damaged;
    qsizetype at = 12 + qsizetype(headerSize);
    const quint32 saltSize = getLe32(info, at);
    if (saltSize != 16 || qint64(at) + 4 + 16 + 16 + 4 + 32 > info.size()) return CryptResult::Damaged;
    const QByteArray salt = info.mid(at + 4, 16), encVerifier = info.mid(at + 20, 16);
    const quint32 hashSize = getLe32(info, at + 36);
    if (hashSize != 20) return CryptResult::Damaged;
    const QByteArray encHash = info.mid(at + 40, 32);
    QByteArray key = standardKey(salt, password, int(keySize / 8));
    const QByteArray verifier = crypto::aesEcb(key, encVerifier, false), hash = crypto::aesEcb(key, encHash, false);
    if (verifier.isEmpty() || hash.isEmpty() || hashWith(QCryptographicHash::Sha1, verifier) != hash.left(20)) {
        crypto::wipe(key);
        return CryptResult::WrongPassword;
    }
    if (verifyOnly) {
        crypto::wipe(key);
        return CryptResult::Ok;
    }
    if (package.size() < 8) {
        crypto::wipe(key);
        return CryptResult::Damaged;
    }
    const quint64 size = getLe64(package, 0);
    const qint64 data = package.size() - 8;
    if (size > quint64(data)) {
        crypto::wipe(key);
        return CryptResult::Damaged;
    }
    if (qint64(size) > maxBytes) {
        crypto::wipe(key);
        return CryptResult::TooLarge;
    }
    const qint64 whole = data - data % 16;
    QByteArray out = crypto::aesEcb(key, QByteArray::fromRawData(package.constData() + 8, qsizetype(whole)), false);
    crypto::wipe(key);
    if (qint64(size) > out.size()) return CryptResult::Damaged;
    out.truncate(qsizetype(size));
    *zip = std::move(out);
    return CryptResult::Ok;
}

QByteArray randomBytes(int n)
{
    QByteArray b(n, 0);
    QRandomGenerator *g = QRandomGenerator::system();
    for (int i = 0; i < n; ++i) b[i] = char(g->generate() & 0xFF);
    return b;
}

// A string as the data-space structures keep it: its length in bytes, the characters in two bytes each, and zeros to a whole number of four bytes.
QByteArray spaceString(const QString &text)
{
    QByteArray b = le32(quint32(text.size() * 2));
    b.append(utf16le(text));
    b.append(QByteArray((4 - b.size() % 4) % 4, 0));
    return b;
}

// The storage that tells a reader the package is enciphered by the strong encryption transform (the original program refuses a package without it).
void addDataSpaces(jp::cfb::File *f)
{
    auto add = [&](const QString &name, jp::cfb::Entry::Type type, const QByteArray &data) {
        jp::cfb::Entry e;
        e.name = name;
        e.type = type;
        e.data = data;
        f->entries.append(e);
        return int(f->entries.size() - 1);
    };
    const QByteArray version1 = le32(1), versions = version1 + version1 + version1;
    const int spaces = add(QStringLiteral("\u0006DataSpaces"), jp::cfb::Entry::Storage, QByteArray());
    const int version = add(QStringLiteral("Version"), jp::cfb::Entry::Stream, spaceString(QStringLiteral("Microsoft.Container.DataSpaces")) + versions);
    QByteArray entry = le32(1) + le32(0) + spaceString(QStringLiteral("EncryptedPackage")) + spaceString(QStringLiteral("StrongEncryptionDataSpace"));
    entry.prepend(le32(quint32(entry.size() + 4)));
    const int map = add(QStringLiteral("DataSpaceMap"), jp::cfb::Entry::Stream, le32(8) + le32(1) + entry);
    const int info = add(QStringLiteral("DataSpaceInfo"), jp::cfb::Entry::Storage, QByteArray());
    const int strong = add(QStringLiteral("StrongEncryptionDataSpace"), jp::cfb::Entry::Stream, le32(8) + le32(1) + spaceString(QStringLiteral("StrongEncryptionTransform")));
    const int transforms = add(QStringLiteral("TransformInfo"), jp::cfb::Entry::Storage, QByteArray());
    const int transform = add(QStringLiteral("StrongEncryptionTransform"), jp::cfb::Entry::Storage, QByteArray());
    const QByteArray id = spaceString(QStringLiteral("{FF9A3F03-56EF-4613-BDD5-5A41C1D07246}"));
    const int primary = add(QStringLiteral("\u0006Primary"), jp::cfb::Entry::Stream,
                            le32(quint32(8 + id.size())) + le32(1) + id + spaceString(QStringLiteral("Microsoft.Container.EncryptionTransform")) + versions + le32(0) + le32(0) + le32(0) + le32(4));
    f->entries[0].children.append(spaces);
    f->entries[spaces].children = {version, map, info, transforms};
    f->entries[info].children = {strong};
    f->entries[transforms].children = {transform};
    f->entries[transform].children = {primary};
}

QByteArray container(const QByteArray &info, const QByteArray &package, bool dataSpaces = false)
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
    if (dataSpaces) addDataSpaces(&f);
    return jp::cfb::write(f);
}

} // namespace

CryptResult decryptPackage(const QByteArray &info, const QByteArray &package, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *zip, const std::function<bool()> &cancelled)
{
    if (info.size() < 8) return CryptResult::Damaged;
    const quint16 major = quint16(quint8(info[0]) | quint8(info[1]) << 8), minor = quint16(quint8(info[2]) | quint8(info[3]) << 8);
    if (major == 4 && minor == 4) {
        AgileInfo agile;
        const CryptResult r = parseAgile(info, &agile);
        if (r != CryptResult::Ok) return r;
        return decryptAgile(agile, package, password, maxBytes, verifyOnly, zip, cancelled);
    }
    if ((major == 2 || major == 3 || major == 4) && minor == 2) return decryptStandard(info, package, password, maxBytes, verifyOnly, zip);
    return CryptResult::Unsupported;
}

QByteArray encryptPackage(const QByteArray &zip, const QString &password, const EncryptOptions &options)
{
    AgileInfo info;
    QCryptographicHash::Algorithm alg;
    int hashSize = 0;
    if (!algorithmOf(options.hash, &alg, &hashSize) || (options.keyBits != 128 && options.keyBits != 192 && options.keyBits != 256) || options.spinCount < 0 || options.spinCount > kMaxPackageSpinCount) return QByteArray();
    const int spinCount = options.spinCount;
    info.saltSize = 16, info.blockSize = 16, info.keyBits = options.keyBits, info.hashSize = hashSize, info.hash = alg;
    info.salt = randomBytes(16);
    Encryptor &e = info.password;
    e.spinCount = spinCount, e.saltSize = 16, e.blockSize = 16, e.keyBits = options.keyBits, e.hashSize = hashSize, e.hash = alg;
    e.salt = randomBytes(16);
    QByteArray secret = randomBytes(options.keyBits / 8);
    const QByteArray verifier = randomBytes(16);
    const QByteArray iv = fitTo(e.salt, 16);
    QByteArray h = spun(e.hash, e.salt, password, spinCount);
    e.verifierInput = crypto::aesCbc(agileStep(info, h, kBlockVerifierInput), iv, verifier, true);
    // (the hash is padded with zeros to a whole number of blocks)
    QByteArray verifierHash = hashWith(e.hash, verifier);
    verifierHash.append(QByteArray((16 - verifierHash.size() % 16) % 16, 0));
    e.verifierValue = crypto::aesCbc(agileStep(info, h, kBlockVerifierValue), iv, verifierHash, true);
    QByteArray keyValue = secret;
    keyValue.append(QByteArray((16 - keyValue.size() % 16) % 16, 0));
    e.keyValue = crypto::aesCbc(agileStep(info, h, kBlockKeyValue), iv, keyValue, true);
    crypto::wipe(h);
    // the package, in segments, with the size in front
    QByteArray body;
    const qint64 padded = (zip.size() + 15) / 16 * 16;
    QByteArray all = zip;
    all.append(QByteArray(int(padded - zip.size()), 0));
    for (qint64 at = 0, index = 0; at < padded; at += kSegment, ++index)
        body.append(crypto::aesCbc(secret, segmentIv(info, quint32(index)), all.mid(qsizetype(at), qsizetype(std::min(kSegment, padded - at))), true));
    QByteArray stream(8, 0);
    for (int i = 0; i < 8; ++i) stream[i] = char((quint64(zip.size()) >> (8 * i)) & 0xFF);
    stream.append(body);
    // the check of changes: a key of the hash's size, and the code of the whole stream
    QByteArray hmacKey = randomBytes(hashSize);
    const QByteArray hmac = QMessageAuthenticationCode::hash(stream, hmacKey, info.hash);
    hmacKey.append(QByteArray((16 - hmacKey.size() % 16) % 16, 0));
    QByteArray hmacPadded = hmac;
    hmacPadded.append(QByteArray((16 - hmacPadded.size() % 16) % 16, 0));
    const QByteArray encKey = crypto::aesCbc(secret, fitTo(hashWith(info.hash, info.salt, kBlockHmacKey), 16), hmacKey, true);
    const QByteArray encValue = crypto::aesCbc(secret, fitTo(hashWith(info.hash, info.salt, kBlockHmacValue), 16), hmacPadded, true);
    crypto::wipe(secret);
    const QByteArray bits = QByteArray::number(options.keyBits), size = QByteArray::number(hashSize), name = options.hash.toUpper().toLatin1();
    const QByteArray cipher = " saltSize=\"16\" blockSize=\"16\" keyBits=\"" + bits + "\" hashSize=\"" + size + "\" cipherAlgorithm=\"AES\" cipherChaining=\"ChainingModeCBC\" hashAlgorithm=\"" + name + "\"";
    QByteArray xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n"
                     "<encryption xmlns=\"http://schemas.microsoft.com/office/2006/encryption\" xmlns:p=\"http://schemas.microsoft.com/office/2006/keyEncryptor/password\" "
                     "xmlns:c=\"http://schemas.microsoft.com/office/2006/keyEncryptor/certificate\">"
                     "<keyData" + cipher + " saltValue=\"" + info.salt.toBase64() + "\"/><dataIntegrity encryptedHmacKey=\"" + encKey.toBase64() + "\" encryptedHmacValue=\"" + encValue.toBase64()
                     + "\"/><keyEncryptors><keyEncryptor uri=\"http://schemas.microsoft.com/office/2006/keyEncryptor/password\"><p:encryptedKey spinCount=\"" + QByteArray::number(spinCount) + "\""
                     + cipher + " saltValue=\"" + e.salt.toBase64() + "\" encryptedVerifierHashInput=\"" + e.verifierInput.toBase64() + "\" encryptedVerifierHashValue=\"" + e.verifierValue.toBase64()
                     + "\" encryptedKeyValue=\"" + e.keyValue.toBase64() + "\"/></keyEncryptor></keyEncryptors></encryption>";
    const QByteArray head = QByteArray::fromHex("0400040040000000");
    return container(head + xml, stream, true);
}

QByteArray encryptPackageStandard(const QByteArray &zip, const QString &password)
{
    const QByteArray salt = randomBytes(16), verifier = randomBytes(16);
    QByteArray key = standardKey(salt, password, 16);
    const QByteArray encVerifier = crypto::aesEcb(key, verifier, true), encHash = crypto::aesEcb(key, hashWith(QCryptographicHash::Sha1, verifier) + QByteArray(12, 0), true);
    QByteArray header;
    auto put = [&](quint32 v) { header.append(le32(v)); };
    put(0x24), put(0), put(0x660E), put(0x8004), put(128), put(0x18), put(0), put(0);
    header.append(utf16le(QStringLiteral("Microsoft Enhanced RSA and AES Cryptographic Provider")));
    header.append(QByteArray(2, 0));
    QByteArray info = QByteArray::fromHex("0300020024000000");
    info.append(le32(quint32(header.size())));
    info.append(header);
    info.append(le32(16));
    info.append(salt);
    info.append(encVerifier);
    info.append(le32(20));
    info.append(encHash);
    const qint64 padded = (zip.size() + 15) / 16 * 16;
    QByteArray all = zip;
    all.append(QByteArray(int(padded - zip.size()), 0));
    QByteArray stream(8, 0);
    for (int i = 0; i < 8; ++i) stream[i] = char((quint64(zip.size()) >> (8 * i)) & 0xFF);
    stream.append(crypto::aesEcb(key, all, true));
    crypto::wipe(key);
    return container(info, stream);
}

} // namespace jp::office
