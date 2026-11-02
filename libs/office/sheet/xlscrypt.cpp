// The protection of the old binary form; see xlscrypt.h.

#include "sheet/xlscrypt.h"

#include "io/crypto.h"

#include <QCryptographicHash>
#include <QStringEncoder>

#include <algorithm>
#include <array>

namespace jp::office::sheet {

namespace {

constexpr quint16 kBof = 0x0809, kEof = 0x000A, kFilePass = 0x002F, kBoundSheet = 0x0085;
constexpr quint16 kRenamed = 0x0DFF;   // (a number the format does not use)

quint16 u16(const QByteArray &b, qsizetype at) { return quint16(quint8(b[at]) | quint8(b[at + 1]) << 8); }
quint32 u32(const QByteArray &b, qsizetype at) { return quint32(u16(b, at)) | quint32(u16(b, at + 2)) << 16; }
QByteArray le32(quint32 v)
{
    QByteArray b(4, 0);
    for (int i = 0; i < 4; ++i) b[i] = char((v >> (8 * i)) & 0xFF);
    return b;
}

// The records that are not enciphered: where a stream begins, FILEPASS itself, and the ones of a workbook shared between people.
bool clearRecord(quint16 id)
{
    switch (id) {
    case kBof: case kFilePass: case 0x0194 /*USREXCL*/: case 0x0195 /*FILELOCK*/: case 0x00E1 /*INTERFACEHDR*/: case 0x0196 /*RRDINFO*/: case 0x0138 /*RRDHEAD*/: return true;
    default: return false;
    }
}

// ---- the obfuscation (XOR)

constexpr std::array<quint16, 15> kInitialCode = {0xE1F0, 0x1D0F, 0xCC9C, 0x84C0, 0x110C, 0x0E10, 0xF1CE, 0x313E, 0x1872, 0xE139, 0xD40F, 0x84F9, 0x280C, 0xA96A, 0x4EC3};
constexpr std::array<quint8, 15> kPad = {0xBB, 0xFF, 0xFF, 0xBA, 0xFF, 0xFF, 0xB9, 0x80, 0x00, 0xBE, 0x0F, 0x00, 0xBF, 0x0F, 0x00};

// The 105 numbers of the key's matrix: those of one place in the password, seven to a character, each the one before it shifted as a 16-bit check value is
// (polynomial 0x1021); the first of each row is the 8th step of the next row's first.
std::array<quint16, 105> xorMatrix()
{
    auto step = [](quint16 x) { return quint16((x << 1) ^ ((x & 0x8000) ? 0x1021 : 0)); };
    std::array<quint16, 105> m{};
    for (int row = 0; row < 15; ++row) {
        quint16 x = 1;
        for (int i = 0; i < 16 + 8 * (14 - row); ++i) x = step(x);
        for (int j = 0; j < 7; ++j) {
            m[size_t(row * 7 + j)] = x;
            x = step(x);
        }
    }
    return m;
}

QByteArray ansiPassword(const QString &password)
{
    QByteArray b;
    for (const QChar c : password.left(15)) b.append(c.unicode() < 256 ? char(c.unicode()) : '?');
    return b;
}

quint16 xorKey(const QByteArray &pw)
{
    static const std::array<quint16, 105> matrix = xorMatrix();
    if (pw.isEmpty()) return 0;
    quint16 key = kInitialCode[size_t(pw.size() - 1)];
    int element = 104;
    for (qsizetype i = pw.size() - 1; i >= 0; --i) {
        quint8 c = quint8(pw[i]);
        for (int j = 0; j < 7; ++j) {
            if (c & 0x40) key ^= matrix[size_t(element)];
            c = quint8(c << 1);
            --element;
        }
    }
    return key;
}

quint16 xorVerifier(const QByteArray &pw)
{
    quint16 v = 0;
    QByteArray all;
    all.append(char(pw.size()));
    all.append(pw);
    for (qsizetype i = all.size() - 1; i >= 0; --i) v = quint16((((v & 0x4000) ? 1 : 0) | ((v * 2) & 0x7FFF)) ^ quint8(all[i]));
    return quint16(v ^ 0xCE4B);
}

std::array<quint8, 16> xorArray(const QByteArray &pw, quint16 key)
{
    std::array<quint8, 16> out{};
    for (int p = 0; p < 16; ++p) {
        const quint8 keyByte = (p & 1) ? quint8(key >> 8) : quint8(key & 0xFF);
        const quint8 src = p < pw.size() ? quint8(pw[p]) : kPad[size_t(p - pw.size())];
        const quint8 x = quint8(src ^ keyByte);
        out[size_t(p)] = quint8((x >> 1) | (x << 7));
    }
    return out;
}

// ---- RC4

QByteArray utf16(const QString &text, int most)
{
    QByteArray b;
    for (const QChar c : text.left(most)) {
        b.append(char(c.unicode() & 0xFF));
        b.append(char(c.unicode() >> 8));
    }
    return b;
}

// How the keys of the blocks are made, for the two forms.
struct Rc4Keys {
    bool cryptoApi = false;
    QByteArray base;      // the hash that the block number is added to
    int keyBytes = 16;
    QByteArray forBlock(quint32 block) const
    {
        if (cryptoApi) {
            // (a key shorter than 128 bits is made up to 16 bytes with zeros, as the provider does: a 40-bit key is five bytes and eleven zeros)
            QByteArray key = QCryptographicHash::hash(base + le32(block), QCryptographicHash::Sha1).left(keyBytes);
            key.append(QByteArray(std::max(0, 16 - int(key.size())), 0));
            return key;
        }
        return QCryptographicHash::hash(base + le32(block), QCryptographicHash::Md5);
    }
};

// 0 fits the password; the keys are made either way
CryptResult rc4Setup(const QByteArray &fp, bool verifyOnly, const QString &password, Rc4Keys *keys)
{
    (void)verifyOnly;
    // wEncryptionType, then the version of the form
    if (fp.size() < 6) return CryptResult::Damaged;
    const quint16 major = u16(fp, 2), minor = u16(fp, 4);
    QByteArray salt, encVerifier, encHash;
    if (major == 1 && minor == 1) {
        if (fp.size() < 54) return CryptResult::Damaged;
        salt = fp.mid(6, 16);
        encVerifier = fp.mid(22, 16);
        encHash = fp.mid(38, 16);
        const QByteArray h0 = QCryptographicHash::hash(utf16(password, 16), QCryptographicHash::Md5);
        QByteArray rep;
        for (int i = 0; i < 16; ++i) rep.append(h0.left(5) + salt);
        keys->cryptoApi = false;
        keys->base = QCryptographicHash::hash(rep, QCryptographicHash::Md5).left(5);
        crypto::Rc4 rc4(keys->forBlock(0));
        const QByteArray plain = rc4.apply(encVerifier + encHash);
        return QCryptographicHash::hash(plain.left(16), QCryptographicHash::Md5) == plain.mid(16, 16) ? CryptResult::Ok : CryptResult::WrongPassword;
    }
    if ((major == 2 || major == 3 || major == 4) && minor == 2) {
        // the same structures as an encrypted package's standard form: a header (flags, the provider's, the cipher, the hash, the size of the key), then the verifier
        if (fp.size() < 2 + 12 + 32 + 4) return CryptResult::Damaged;
        const quint32 headerSize = u32(fp, 2 + 8);
        if (headerSize < 32 || headerSize > 4096 || qint64(2) + 12 + headerSize + 4 + 16 + 16 + 4 + 20 > fp.size()) return CryptResult::Damaged;
        const quint32 algId = u32(fp, 2 + 12 + 8), hashId = u32(fp, 2 + 12 + 12);
        quint32 keySize = u32(fp, 2 + 12 + 16);
        if ((algId != 0x6801 && algId != 0) || (hashId != 0x8004 && hashId != 0)) return CryptResult::Unsupported;
        if (keySize == 0) keySize = 40;
        if (keySize < 40 || keySize > 128 || keySize % 8) return CryptResult::Damaged;
        const qsizetype v = 2 + 12 + qsizetype(headerSize);
        if (u32(fp, v) != 16) return CryptResult::Damaged;
        salt = fp.mid(v + 4, 16);
        encVerifier = fp.mid(v + 20, 16);
        if (u32(fp, v + 36) != 20) return CryptResult::Damaged;
        encHash = fp.mid(v + 40, 20);
        keys->cryptoApi = true;
        keys->keyBytes = int(keySize / 8);
        keys->base = QCryptographicHash::hash(salt + utf16(password, 255), QCryptographicHash::Sha1);
        crypto::Rc4 rc4(keys->forBlock(0));
        const QByteArray plain = rc4.apply(encVerifier + encHash);
        return QCryptographicHash::hash(plain.left(16), QCryptographicHash::Sha1) == plain.mid(16, 20) ? CryptResult::Ok : CryptResult::WrongPassword;
    }
    return CryptResult::Unsupported;
}

struct FilePass {
    qsizetype at = -1;   // the offset of the record's header
    quint16 length = 0;
};

// The FILEPASS record among the first records, and whether the stream is of the form with a version above BIFF5 (the BOF says).
bool findFilePass(const QByteArray &s, FilePass *fp, bool *biff8)
{
    qsizetype p = 0;
    *biff8 = true;
    for (int n = 0; n < 200 && p + 4 <= s.size(); ++n) {
        const quint16 id = u16(s, p), len = u16(s, p + 2);
        if (p + 4 + len > s.size()) return false;
        if (n == 0 && id == kBof && len >= 2) *biff8 = u16(s, p + 4) >= 0x0600;
        if (id == kFilePass) {
            fp->at = p;
            fp->length = len;
            return true;
        }
        if (id == kEof) return false;
        p += 4 + len;
    }
    return false;
}

} // namespace

quint16 xorKeyForTest(const QByteArray &password) { return xorKey(password.left(15)); }
quint16 xorVerifierForTest(const QByteArray &password) { return xorVerifier(password.left(15)); }

bool biffEncrypted(const QByteArray &stream)
{
    FilePass fp;
    bool biff8;
    return findFilePass(stream, &fp, &biff8);
}

CryptResult decryptBiff(QByteArray *stream, const QString &password, bool verifyOnly)
{
    FilePass fp;
    bool biff8;
    if (!findFilePass(*stream, &fp, &biff8)) return CryptResult::Damaged;
    const QByteArray body = stream->mid(fp.at + 4, fp.length);
    const qsizetype after = fp.at + 4 + fp.length;
    bool xorForm = !biff8;
    if (biff8) {
        if (body.size() < 2) return CryptResult::Damaged;
        const quint16 type = u16(body, 0);
        if (type == 0) xorForm = true;
        else if (type != 1) return CryptResult::Unsupported;
    }
    std::array<quint8, 16> table{};
    Rc4Keys keys;
    if (xorForm) {
        const qsizetype at = biff8 ? 2 : 0;
        if (body.size() < at + 4) return CryptResult::Damaged;
        const QByteArray pw = ansiPassword(password);
        if (pw.isEmpty()) return CryptResult::WrongPassword;
        const quint16 key = u16(body, at), verifier = u16(body, at + 2);
        if (xorVerifier(pw) != verifier || xorKey(pw) != key) return CryptResult::WrongPassword;
        table = xorArray(pw, key);
    } else {
        const CryptResult r = rc4Setup(body, verifyOnly, password, &keys);
        if (r != CryptResult::Ok) return r;
    }
    if (verifyOnly) return CryptResult::Ok;
    // the records after FILEPASS, one at a time
    stream->detach();
    char *d = stream->data();
    const qsizetype size = stream->size();
    QByteArray blockKeystream;
    qint64 haveBlock = -1;
    qsizetype p = after;
    while (p + 4 <= size) {
        const quint16 id = u16(*stream, p), len = u16(*stream, p + 2);
        const qsizetype start = p + 4, end = start + len;
        if (end > size) return CryptResult::Damaged;
        if (!clearRecord(id)) {
            qsizetype from = start;
            if (id == kBoundSheet) from = std::min<qsizetype>(end, start + 4);   // (where the sheet begins is not enciphered)
            if (xorForm) {
                for (qsizetype q = from; q < end; ++q) {
                    const quint8 c = quint8(d[q]) ^ table[size_t((start + len + (q - start)) % 16)];
                    d[q] = char(quint8((c << 3) | (c >> 5)));
                }
            } else {
                for (qsizetype q = from; q < end; ++q) {
                    const qint64 block = q / 1024;
                    if (block != haveBlock) {
                        crypto::Rc4 rc4(keys.forBlock(quint32(block)));
                        blockKeystream = rc4.apply(QByteArray(1024, 0));
                        haveBlock = block;
                    }
                    d[q] = char(quint8(d[q]) ^ quint8(blockKeystream[int(q % 1024)]));
                }
            }
        }
        p = end;
    }
    d[fp.at] = char(kRenamed & 0xFF);
    d[fp.at + 1] = char(kRenamed >> 8);
    return CryptResult::Ok;
}

} // namespace jp::office::sheet
