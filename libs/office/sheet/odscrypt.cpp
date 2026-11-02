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
        const bool aes = p.algorithm.contains(QLatin1String("aes"), Qt::CaseInsensitive), blowfish = p.algorithm.contains(QLatin1String("blowfish"), Qt::CaseInsensitive);
        if ((!aes && !blowfish) || (!p.derivation.isEmpty() && p.derivation.compare(QLatin1String("PBKDF2"), Qt::CaseInsensitive) != 0)) {
            result = CryptResult::Unsupported;
            break;
        }
        // every size the manifest gives is checked before any work is done for it
        int keySize = p.keySize;
        if (keySize <= 0) keySize = !aes ? 16 : p.algorithm.contains(QLatin1String("128")) ? 16 : p.algorithm.contains(QLatin1String("192")) ? 24 : 32;
        const bool keyFits = aes ? (keySize == 16 || keySize == 24 || keySize == 32) : (keySize >= 4 && keySize <= 56);
        if (!keyFits || p.rounds < 1 || p.rounds > kMaxOdsKeyRounds || p.salt.isEmpty() || p.salt.size() > 1024 || p.iv.size() != (aes ? 16 : 8) || p.size < 0 || p.size > jp::kMaxZipEntryBytes) {
            result = CryptResult::Damaged;
            break;
        }
        QCryptographicHash::Algorithm checksum;
        if (!checksumHash(p.checksumType, &checksum) || p.checksum.isEmpty()) {
            result = CryptResult::Unsupported;
            break;
        }
        // (a manifest that names no start key is from the oldest files, which hashed the password with SHA-1, and from some that used SHA-256 and did not say so: both are tried)
        QVector<QCryptographicHash::Algorithm> starts;
        if (p.startKey.isEmpty()) starts = {QCryptographicHash::Sha1, QCryptographicHash::Sha256};
        else starts = {startHash(p.startKey)};
        QByteArray plainBytes;
        bool matched = false, broke = false;
        for (const QCryptographicHash::Algorithm start : starts) {
            const QByteArray cacheKey = p.salt + QByteArray::number(p.rounds) + ':' + QByteArray::number(keySize) + ':' + QByteArray::number(int(start));
            QByteArray key = keys.value(cacheKey);
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
                keys.insert(cacheKey, key);
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
