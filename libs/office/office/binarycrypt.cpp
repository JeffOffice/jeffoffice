// The keys of the old binary forms of protection; see binarycrypt.h.

#include "office/binarycrypt.h"

#include "io/crypto.h"

#include <QCryptographicHash>
#include <QRandomGenerator>

#include <algorithm>
#include <array>

namespace jp::office::bincrypt {

namespace {

quint16 u16(const QByteArray &b, qsizetype at) { return quint16(quint8(b[at]) | quint8(b[at + 1]) << 8); }
quint32 u32(const QByteArray &b, qsizetype at) { return quint32(u16(b, at)) | quint32(u16(b, at + 2)) << 16; }
QByteArray le32(quint32 v)
{
    QByteArray b(4, 0);
    for (int i = 0; i < 4; ++i) b[i] = char((v >> (8 * i)) & 0xFF);
    return b;
}

constexpr std::array<quint16, 15> kInitialCode = {0xE1F0, 0x1D0F, 0xCC9C, 0x84C0, 0x110C, 0x0E10, 0xF1CE, 0x313E, 0x1872, 0xE139, 0xD40F, 0x84F9, 0x280C, 0xA96A, 0x4EC3};
constexpr std::array<quint8, 15> kPad = {0xBB, 0xFF, 0xFF, 0xBA, 0xFF, 0xFF, 0xB9, 0x80, 0x00, 0xBE, 0x0F, 0x00, 0xBF, 0x0F, 0x00};

} // namespace

// ---- the obfuscation (XOR)

// The 105 numbers of the key's matrix: those of one place in the password, seven to a character, each the one before it shifted as a 16-bit check value is
// (polynomial 0x1021); the first of each row is the 8th step of the next row's first.
static std::array<quint16, 105> xorMatrix()
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

// The password as the obfuscation takes it: the first 15 characters, one byte each, in the Western Windows coding (the euro sign and the curly quotes have places in it, in 0x80 to 0x9F).
// A password with a letter outside that coding has no such form, and is not the one (it is not turned into another password).
bool ansiPassword(const QString &password, QByteArray *out)
{
    static const ushort kWest[32] = {0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
                                     0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178};
    out->clear();
    for (const QChar c : password.left(15)) {
        const ushort u = c.unicode();
        if (u < 0x80 || (u >= 0xA0 && u <= 0xFF)) {
            out->append(char(u));
            continue;
        }
        int found = -1;
        for (int i = 0; i < 32; ++i)
            if (kWest[i] == u) found = i;
        if (found < 0) return false;
        out->append(char(0x80 + found));
    }
    return true;
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

static QByteArray utf16Impl(const QString &text, int most)
{
    QByteArray b;
    for (const QChar c : text.left(most)) {
        b.append(char(c.unicode() & 0xFF));
        b.append(char(c.unicode() >> 8));
    }
    return b;
}

Wiper::~Wiper()
{
    for (int i = 0; i < count; ++i) crypto::wipe(*items[i]);
}

QByteArray Rc4Keys::forBlock(quint32 block) const
{
    if (cryptoApi) {
        // (a key shorter than 128 bits is made up to 16 bytes with zeros, as the provider does: a 40-bit key is five bytes and eleven zeros)
        QByteArray key = QCryptographicHash::hash(base + le32(block), QCryptographicHash::Sha1).left(keyBytes);
        key.append(QByteArray(std::max(0, 16 - int(key.size())), 0));
        return key;
    }
    return QCryptographicHash::hash(base + le32(block), QCryptographicHash::Md5);
}

crypto::Rc4 Rc4Keys::cipher(quint32 block) const
{
    QByteArray key = forBlock(block);
    crypto::Rc4 rc4(key);
    crypto::wipe(key);
    return rc4;
}

// The hash a form made by the provider takes its block keys from: SHA-1 of the salt and the password as two-byte letters.
static QByteArray cryptoApiBase(const QByteArray &salt, const QString &password)
{
    QByteArray typed = utf16Impl(password, 255), seeded = salt + typed;
    const QByteArray base = QCryptographicHash::hash(seeded, QCryptographicHash::Sha1);
    crypto::wipe(typed);
    crypto::wipe(seeded);
    return base;
}

CryptResult rc4Setup(const QByteArray &fp, bool verifyOnly, const QString &password, Rc4Keys *keys)
{
    (void)verifyOnly;
    // wEncryptionType, then the version of the form
    if (fp.size() < 6) return CryptResult::Damaged;
    const quint16 major = u16(fp, 2), minor = u16(fp, 4);
    QByteArray salt, encVerifier, encHash, h0, rep, plain, derived, typed, seeded;
    Wiper wiper;
    wiper.add(&typed);
    wiper.add(&seeded);
    wiper.add(&h0);
    wiper.add(&rep);
    wiper.add(&plain);
    wiper.add(&derived);
    if (major == 1 && minor == 1) {
        if (fp.size() < 54) return CryptResult::Damaged;
        salt = fp.mid(6, 16);
        encVerifier = fp.mid(22, 16);
        encHash = fp.mid(38, 16);
        typed = utf16Impl(password, 16);
        h0 = QCryptographicHash::hash(typed, QCryptographicHash::Md5);
        for (int i = 0; i < 16; ++i) rep.append(h0.left(5) + salt);
        keys->cryptoApi = false;
        keys->base = QCryptographicHash::hash(rep, QCryptographicHash::Md5).left(5);
        derived = keys->forBlock(0);
        crypto::Rc4 rc4(derived);
        plain = rc4.apply(encVerifier + encHash);
        return crypto::equal(QCryptographicHash::hash(plain.left(16), QCryptographicHash::Md5), plain.mid(16, 16)) ? CryptResult::Ok : CryptResult::WrongPassword;
    }
    if ((major == 2 || major == 3 || major == 4) && minor == 2) {
        // the same structures as an encrypted package's standard form: a header (flags, the provider's, the cipher, the hash, the size of the key), then the verifier
        if (fp.size() < 2 + 12 + 32 + 4) return CryptResult::Damaged;
        const quint32 headerSize = u32(fp, 2 + 8);
        if (headerSize < 32 || headerSize > 4096 || qint64(2) + 12 + headerSize + 4 + 16 + 16 + 4 + 20 > fp.size()) return CryptResult::Damaged;
        const quint32 algId = u32(fp, 2 + 12 + 8), hashId = u32(fp, 2 + 12 + 12);
        // the flags the form relies on: made by the cryptographic provider (0x04), and not the AES kind (0x20)
        const quint32 headerFlags = u32(fp, 2 + 12);
        if (!(headerFlags & 0x04) || (headerFlags & 0x20)) return CryptResult::Unsupported;
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
        keys->base = cryptoApiBase(salt, password);
        derived = keys->forBlock(0);
        crypto::Rc4 rc4(derived);
        plain = rc4.apply(encVerifier + encHash);
        return crypto::equal(QCryptographicHash::hash(plain.left(16), QCryptographicHash::Sha1), plain.mid(16, 20)) ? CryptResult::Ok : CryptResult::WrongPassword;
    }
    return CryptResult::Unsupported;
}

static QByteArray randomBytes16()
{
    QByteArray b(16, 0);
    QRandomGenerator *g = QRandomGenerator::system();
    for (int i = 0; i < 16; i += 4) {
        const quint32 r = g->generate();
        for (int j = 0; j < 4; ++j) b[i + j] = char((r >> (8 * j)) & 0xFF);
    }
    return b;
}

QByteArray rc4CryptoApiHeader(const QString &password, Rc4Keys *keys, const QByteArray &saltIn, const QByteArray &verifierIn)
{
    if (password.isEmpty()) return QByteArray();
    const QByteArray salt = saltIn.size() == 16 ? saltIn : randomBytes16();
    QByteArray verifier = verifierIn.size() == 16 ? verifierIn : randomBytes16(), both, key0;
    Wiper wiper;
    wiper.add(&key0);
    wiper.add(&verifier);
    wiper.add(&both);
    keys->cryptoApi = true;
    keys->keyBytes = 16;
    keys->base = cryptoApiBase(salt, password);
    // (the check value and its hash are enciphered in one run of the block-0 key, as the reader opens them)
    key0 = keys->forBlock(0);
    crypto::Rc4 rc4(key0);
    both = rc4.apply(verifier + QCryptographicHash::hash(verifier, QCryptographicHash::Sha1));
    // the provider name the format's readers look up; a file with any other name is refused (kept as hex so that the source does not spell the vendor)
    static const char kProviderHex[] = "4d6963726f736f667420456e68616e6365642043727970746f677261706869632050726f76696465722076312e30";
    QByteArray provider = QByteArray::fromHex(kProviderHex);
    QByteArray header;
    auto put32 = [](QByteArray *b, quint32 v) { b->append(le32(v)); };
    put32(&header, 0x04);            // the flags: made by the cryptographic provider
    put32(&header, 0);               // the size of what is added to the header
    put32(&header, 0x6801);          // RC4
    put32(&header, 0x8004);          // SHA-1
    put32(&header, 128);             // the bits of the key
    put32(&header, 1);               // the provider's kind
    put32(&header, 0);
    put32(&header, 0);
    for (const char c : provider) header.append(c).append(char(0));   // (two-byte letters, then a zero)
    header.append(QByteArray(2, 0));
    QByteArray body;
    body.append(char(1)).append(char(0));    // the kind of protection: RC4
    body.append(char(4)).append(char(0));    // the version
    body.append(char(2)).append(char(0));
    put32(&body, 0x04);                      // the flags of the whole header
    put32(&body, quint32(header.size()));
    body.append(header);
    put32(&body, 16);
    body.append(salt);
    body.append(both.left(16));
    put32(&body, 20);
    body.append(both.mid(16, 20));
    return body;
}

} // namespace jp::office::bincrypt
