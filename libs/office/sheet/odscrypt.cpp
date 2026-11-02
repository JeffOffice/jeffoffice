// The protection of OpenDocument packages; see odscrypt.h.

#include "sheet/odscrypt.h"

#include "io/crypto.h"
#include "io/zip.h"
#include "io/zipindex.h"
#include "office/odraw_inflate.h"

#include <QCryptographicHash>
#include <QHash>
#include <QMessageAuthenticationCode>
#include <QRegularExpression>
#include <QXmlStreamReader>

#include <algorithm>

namespace jp::office::sheet {

namespace {

struct Protection {
    qint64 size = -1;
    QString checksumType, algorithm, startKey, derivation;
    QByteArray checksum, iv, salt;
    qint64 rounds = 0;
    int keySize = 0;
    qint64 argonIterations = 0, argonMemory = 0, argonLanes = 0;   // (the newest files: Argon2id, with the memory in KiB)
};

// What the manifest says of each part that has encryption-data, by the part's name.
bool readManifest(const QByteArray &zip, const QVector<jp::ZipEntry> &entries, QHash<QString, Protection> *out)
{
    const jp::ZipEntry *manifest = nullptr;
    for (const jp::ZipEntry &e : entries)
        if (e.name == QLatin1String("META-INF/manifest.xml") || e.name == QLatin1String("/META-INF/manifest.xml")) manifest = &e;
    if (!manifest || manifest->usize > (qint64(8) << 20)) return false;
    QByteArray xml;
    if (!jp::inflateZipEntry(zip, *manifest, &xml)) return false;
    QXmlStreamReader r(xml);
    QString path;
    qint64 size = -1;
    Protection *current = nullptr;
    QString currentPath;
    QHash<QString, Protection> found;
    while (!r.atEnd()) {
        r.readNext();
        if (r.isEndElement() && r.name() == QLatin1String("file-entry")) {
            current = nullptr;
            continue;
        }
        if (!r.isStartElement()) continue;
        const QXmlStreamAttributes a = r.attributes();
        // (the manifest's attributes carry its prefix: they are found by their local names)
        auto attr = [&](const char *name) {
            for (const QXmlStreamAttribute &at : a)
                if (at.name() == QLatin1String(name)) return at.value().toString();
            return QString();
        };
        auto has = [&](const char *name) {
            for (const QXmlStreamAttribute &at : a)
                if (at.name() == QLatin1String(name)) return true;
            return false;
        };
        if (r.name() == QLatin1String("file-entry")) {
            currentPath = attr("full-path");
            size = has("size") ? attr("size").toLongLong() : -1;
            current = nullptr;
        } else if (r.name() == QLatin1String("encryption-data")) {
            current = &found[currentPath];
            current->size = size;
            current->checksumType = attr("checksum-type");
            current->checksum = QByteArray::fromBase64(attr("checksum").toLatin1());
        } else if (current && r.name() == QLatin1String("algorithm")) {
            current->algorithm = attr("algorithm-name");
            current->iv = QByteArray::fromBase64(attr("initialisation-vector").toLatin1());
        } else if (current && r.name() == QLatin1String("key-derivation")) {
            current->derivation = attr("key-derivation-name");
            current->salt = QByteArray::fromBase64(attr("salt").toLatin1());
            current->rounds = attr("iteration-count").toLongLong();
            current->keySize = attr("key-size").toInt();
            current->argonIterations = attr("argon2-iterations").toLongLong();
            current->argonMemory = attr("argon2-memory").toLongLong();
            current->argonLanes = attr("argon2-lanes").toLongLong();
        } else if (current && r.name() == QLatin1String("start-key-generation")) {
            current->startKey = attr("start-key-generation-name");
        }
    }
    if (r.hasError()) return false;
    *out = found;
    return true;
}

// PBKDF2 with HMAC-SHA-1 (RFC 8018), in pieces that ask `cancelled` now and then; empty if it was stopped.
QByteArray pbkdf2Sha1(const QByteArray &password, const QByteArray &salt, qint64 rounds, int length, const std::function<bool()> &cancelled)
{
    QMessageAuthenticationCode mac(QCryptographicHash::Sha1, password);
    struct ClearKey {
        QMessageAuthenticationCode &m;
        ~ClearKey() { m.setKey(QByteArray()); }   // (the code's own copy of the password goes with it)
    } clearKey{mac};
    QByteArray out;
    for (quint32 block = 1; out.size() < length; ++block) {
        QByteArray index(4, 0);
        index[0] = char(block >> 24);
        index[1] = char(block >> 16);
        index[2] = char(block >> 8);
        index[3] = char(block);
        mac.reset();
        mac.addData(salt);
        mac.addData(index);
        QByteArray u = mac.result(), t = u;
        for (qint64 i = 1; i < rounds; ++i) {
            if ((i & 0x1FFF) == 0 && cancelled && cancelled()) {
                crypto::wipe(u);
                crypto::wipe(t);
                crypto::wipe(out);
                return QByteArray();
            }
            mac.reset();
            mac.addData(u);
            u = mac.result();
            for (int k = 0; k < t.size(); ++k) t[k] = char(t[k] ^ u[k]);
        }
        out.append(t);
        crypto::wipe(u);
        crypto::wipe(t);
    }
    out.truncate(length);
    return out;
}

bool isArgon(const Protection &p) { return p.derivation.contains(QLatin1String("argon2id"), Qt::CaseInsensitive); }

QString cleanName(const QString &n) { return n.startsWith(QLatin1Char('/')) ? n.mid(1) : n; }

QCryptographicHash::Algorithm startHash(const QString &name)
{
    const QString n = name.toLower();
    if (n.contains(QLatin1String("sha256"))) return QCryptographicHash::Sha256;
    if (n.contains(QLatin1String("sha512"))) return QCryptographicHash::Sha512;
    return QCryptographicHash::Sha1;   // (also when the manifest says nothing: the oldest files)
}

bool checksumHash(const QString &type, QCryptographicHash::Algorithm *alg)
{
    const QString t = type.toLower();
    if (t.contains(QLatin1String("sha256"))) *alg = QCryptographicHash::Sha256;
    else if (t.contains(QLatin1String("sha512"))) *alg = QCryptographicHash::Sha512;
    else if (t.contains(QLatin1String("sha1"))) *alg = QCryptographicHash::Sha1;
    else return false;
    return true;
}


// The newest files: the whole package, deflated, then enciphered as the one part "encrypted-package" (AES-GCM, the key made by Argon2id from the SHA-256 of the password). The part is the 12 bytes
// of the iv (the manifest has them too), the enciphered bytes, and the 16 of the tag.
CryptResult decryptArgonPackage(const QByteArray &zip, const QVector<jp::ZipEntry> &entries, const Protection &p, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain,
                                const std::function<bool()> &cancelled)
{
    const jp::ZipEntry *part = nullptr;
    for (const jp::ZipEntry &e : entries)
        if (cleanName(e.name) == QLatin1String("encrypted-package")) part = &e;
    if (!part || part->dataAt < 0 || part->csize < 12 + 16 || part->dataAt + part->csize > zip.size()) return CryptResult::Damaged;
    if (p.size > maxBytes) return CryptResult::TooLarge;
    QByteArray sealed;
    if (!jp::inflateZipEntry(zip, *part, &sealed) || sealed.size() < 12 + 16 || sealed.left(12) != p.iv) return CryptResult::Damaged;
    QByteArray pw = password.toUtf8();
    QByteArray start = QCryptographicHash::hash(pw, startHash(p.startKey.isEmpty() ? QStringLiteral("sha256") : p.startKey));
    crypto::wipe(pw);
    int keySize = p.keySize > 0 ? p.keySize : 32;
    QByteArray key = crypto::argon2id(start, p.salt, quint32(p.argonIterations), quint32(p.argonMemory), quint32(p.argonLanes), keySize, QByteArray(), QByteArray(), cancelled);
    crypto::wipe(start);
    if (key.isEmpty()) return cancelled && cancelled() ? CryptResult::Cancelled : CryptResult::Damaged;
    bool ok = false;
    QByteArray packed = crypto::aesGcmOpen(key, p.iv, QByteArray(), sealed.mid(12), &ok);
    crypto::wipe(key);
    if (!ok) return CryptResult::WrongPassword;
    if (verifyOnly) {
        crypto::wipe(packed);
        return CryptResult::Ok;
    }
    bool inflated = false;
    const QByteArray inner = jp::office::odrawInflate(packed, p.size, &inflated);
    crypto::wipe(packed);
    if (!inflated || inner.size() != p.size) return CryptResult::Damaged;
    *plain = inner;
    return CryptResult::Ok;
}

} // namespace

bool odsEncrypted(const QByteArray &zip)
{
    QVector<jp::ZipEntry> entries;
    if (!jp::readZipDirectory(zip, &entries)) return false;
    QHash<QString, Protection> found;
    return readManifest(zip, entries, &found) && !found.isEmpty();
}

CryptResult decryptOds(const QByteArray &zip, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain, const std::function<bool()> &cancelled)
{
    QVector<jp::ZipEntry> entries;
    if (!jp::readZipDirectory(zip, &entries)) return CryptResult::Damaged;
    QHash<QString, Protection> protection;
    if (!readManifest(zip, entries, &protection) || protection.isEmpty()) return CryptResult::Damaged;
    // Every part's sizes and counts are checked, all of them, before any key is made for any: a manifest with one bad part costs no work.
    auto checkPart = [](const Protection &p, bool *aes, int *keySize) {
        if (isArgon(p)) {
            *aes = true;
            const bool gcm = p.algorithm.contains(QLatin1String("aes"), Qt::CaseInsensitive) && p.algorithm.contains(QLatin1String("gcm"), Qt::CaseInsensitive);
            if (!gcm) return CryptResult::Unsupported;
            *keySize = p.keySize > 0 ? p.keySize : p.algorithm.contains(QLatin1String("128")) ? 16 : p.algorithm.contains(QLatin1String("192")) ? 24 : 32;
            if (p.argonIterations > kMaxOdsArgonIterations || p.argonMemory > kMaxOdsArgonMemoryKiB || p.argonLanes > kMaxOdsArgonLanes) return CryptResult::TooCostly;
            if (p.argonIterations < 1 || p.argonLanes < 1 || p.argonMemory < 8 * p.argonLanes) return CryptResult::Damaged;
            if ((*keySize != 16 && *keySize != 24 && *keySize != 32) || p.salt.size() < 8 || p.salt.size() > 1024 || p.iv.size() != 12 || p.size < 0 || p.size > jp::kMaxZipEntryBytes)
                return CryptResult::Damaged;
            return CryptResult::Ok;
        }
        if (p.algorithm.contains(QLatin1String("gcm"), Qt::CaseInsensitive)) return CryptResult::Unsupported;   // (that cipher is read only with the key stretching it comes with)
        *aes = p.algorithm.contains(QLatin1String("aes"), Qt::CaseInsensitive);
        const bool blowfish = p.algorithm.contains(QLatin1String("blowfish"), Qt::CaseInsensitive);
        if ((!*aes && !blowfish) || (!p.derivation.isEmpty() && p.derivation.compare(QLatin1String("PBKDF2"), Qt::CaseInsensitive) != 0)) return CryptResult::Unsupported;
        QCryptographicHash::Algorithm checksum;
        if (!checksumHash(p.checksumType, &checksum) || p.checksum.isEmpty()) return CryptResult::Unsupported;
        *keySize = p.keySize;
        if (*keySize <= 0) *keySize = !*aes ? 16 : p.algorithm.contains(QLatin1String("128")) ? 16 : p.algorithm.contains(QLatin1String("192")) ? 24 : 32;
        const bool keyFits = *aes ? (*keySize == 16 || *keySize == 24 || *keySize == 32) : (*keySize >= 4 && *keySize <= 56);
        if (!keyFits || p.rounds < 1 || p.rounds > kMaxOdsKeyRounds || p.salt.isEmpty() || p.salt.size() > 1024 || p.iv.size() != (*aes ? 16 : 8) || p.size < 0 || p.size > jp::kMaxZipEntryBytes)
            return CryptResult::Damaged;
        return CryptResult::Ok;
    };
    qint64 roundsOfAll = 0;
    for (auto it = protection.constBegin(); it != protection.constEnd(); ++it) {
        bool aes = false;
        int keySize = 0;
        const CryptResult r = checkPart(it.value(), &aes, &keySize);
        if (r != CryptResult::Ok) return r;
        roundsOfAll += it.value().rounds;   // (each is within the limit: the sum of some thousands of them cannot overflow)
        if (roundsOfAll > kMaxOdsKeyRounds * 64) return CryptResult::Damaged;
    }
    for (const Protection &p : std::as_const(protection))
        if (isArgon(p)) return protection.size() == 1 && protection.contains(QStringLiteral("encrypted-package")) ? decryptArgonPackage(zip, entries, protection.constBegin().value(), password, maxBytes, verifyOnly, plain, cancelled) : CryptResult::Unsupported;
    QByteArray pw = password.toUtf8();
    qint64 rounds = 0, total = 0;
    QHash<QByteArray, QByteArray> keys;   // by the salt, the rounds, the size, and the start hash: one stretch for parts that share them
    jp::ZipWriter out;
    CryptResult result = CryptResult::Ok;
    for (const jp::ZipEntry &e : entries) {
        const QString name = cleanName(e.name);
        const auto found = protection.constFind(name);
        if (e.dataAt < 0 || e.csize < 0 || e.dataAt + e.csize > zip.size()) {
            result = CryptResult::Damaged;
            break;
        }
        if (found == protection.constEnd()) {
            // (the manifest is read as the plain package's: with no part named as enciphered)
            if (!verifyOnly && name == QLatin1String("META-INF/manifest.xml")) {
                QByteArray xml;
                if (!jp::inflateZipEntry(zip, e, &xml)) {
                    result = CryptResult::Damaged;
                    break;
                }
                static const QRegularExpression enc(QStringLiteral("<(\\w+:)?encryption-data\\b[\\s\\S]*?</(\\w+:)?encryption-data>"));
                out.add(e.name, QString::fromUtf8(xml).remove(enc).toUtf8(), false);
                continue;
            }
            if (!verifyOnly) out.addRaw(e.name, e.method, e.crc, quint32(e.csize), quint32(e.usize), zip.constData() + e.dataAt);
            continue;
        }
        const Protection &p = *found;
        // (a part with no bytes is a part with none)
        if (p.size == 0 || e.csize == 0) {
            if (!verifyOnly) out.add(e.name, QByteArray(), false);
            continue;
        }
        bool aes = false;
        int keySize = 0;
        if (checkPart(p, &aes, &keySize) != CryptResult::Ok) {   // (checked above: only a part that was not in the list of the manifest's can reach this)
            result = CryptResult::Damaged;
            break;
        }
        QCryptographicHash::Algorithm checksum;
        checksumHash(p.checksumType, &checksum);
        // (a manifest that names no start key is from the oldest files, which hashed the password with SHA-1, and from some that used SHA-256 and did not say so: both are tried)
        QVector<QCryptographicHash::Algorithm> starts;
        if (p.startKey.isEmpty()) starts = {QCryptographicHash::Sha1, QCryptographicHash::Sha256};
        else starts = {startHash(p.startKey)};
        QByteArray plainBytes;
        struct Wipe {
            QByteArray &b;
            ~Wipe() { crypto::wipe(b); }
        } wipePlain{plainBytes};   // (what was deciphered is not left behind once the part is done with)
        bool matched = false, broke = false;
        for (const QCryptographicHash::Algorithm start : starts) {
            const QByteArray cacheKey = p.salt + QByteArray::number(p.rounds) + ':' + QByteArray::number(keySize) + ':' + QByteArray::number(int(start));
            // (the key is held in the cache only, and used from there: no second copy of it is left to be wiped)
            QByteArray &key = keys[cacheKey];
            if (key.isEmpty()) {
                // (each part was checked against the limit above, so the sum cannot overflow)
                rounds += p.rounds;
                if (rounds > kMaxOdsKeyRounds) {
                    result = CryptResult::Damaged;
                    broke = true;
                    break;
                }
                QByteArray startKey = QCryptographicHash::hash(pw, start);
                key = pbkdf2Sha1(startKey, p.salt, p.rounds, keySize, cancelled);
                crypto::wipe(startKey);
                if (key.isEmpty()) {
                    result = CryptResult::Cancelled;
                    broke = true;
                    break;
                }
            }
            QByteArray data = QByteArray::fromRawData(zip.constData() + e.dataAt, qsizetype(e.csize));
            if (aes) {
                if (p.iv.size() != 16 || e.csize % 16) {
                    result = CryptResult::Damaged;
                    broke = true;
                    break;
                }
                plainBytes = crypto::aesCbc(key, p.iv, data, false);
                // (what is padded to the block is trimmed: the padding's last byte says how much)
                if (!plainBytes.isEmpty()) {
                    const int pad = quint8(plainBytes.back());
                    if (pad >= 1 && pad <= 16 && pad <= plainBytes.size()) plainBytes.chop(pad);
                }
            } else {
                plainBytes = crypto::blowfishCfb(key, p.iv, data, false);
            }
            if (plainBytes.isEmpty() && e.csize > 0) {
                result = CryptResult::Damaged;
                broke = true;
                break;
            }
            if (crypto::equal(QCryptographicHash::hash(QByteArrayView(plainBytes).left(1024), checksum), p.checksum)) {
                matched = true;
                break;
            }
        }
        if (broke) break;
        if (!matched) {
            result = CryptResult::WrongPassword;
            break;
        }
        if (verifyOnly) break;   // (the first part is the test of the password)
        total += p.size;
        if (total > maxBytes) {
            result = CryptResult::TooLarge;
            break;
        }
        bool ok = false;
        const QByteArray inflated = jp::office::odrawInflate(plainBytes, p.size, &ok);
        if (!ok || inflated.size() != p.size) {
            result = CryptResult::Damaged;
            break;
        }
        out.add(e.name, inflated, false);
    }
    crypto::wipe(pw);
    for (QByteArray &k : keys) crypto::wipe(k);
    if (result == CryptResult::Ok && !verifyOnly) *plain = out.finish();
    return result;
}

} // namespace jp::office::sheet
