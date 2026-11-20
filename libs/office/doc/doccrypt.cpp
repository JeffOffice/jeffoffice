// The protection of the old binary document; see doccrypt.h.

#include "doc/doccrypt.h"

#include "io/cfb.h"
#include "io/crypto.h"
#include "office/binarycrypt.h"

#include <algorithm>

namespace jp::office::doc {

using namespace bincrypt;

namespace {

constexpr quint16 kEncrypted = 0x0100, kObfuscated = 0x8000;
constexpr int kBlock = 512;
constexpr quint16 kFirstBinary97 = 0x00C1;   // (the number of the 1997 form; older headers are not read)

quint16 u16(const QByteArray &b, qsizetype at) { return quint16(quint8(b[at]) | quint8(b[at + 1]) << 8); }
quint32 u32(const QByteArray &b, qsizetype at) { return quint32(u16(b, at)) | quint32(u16(b, at + 2)) << 16; }

struct Header {
    quint16 flags = 0;
    quint32 key = 0;
    QString table;
};

bool readHeader(const QByteArray &wd, Header *h)
{
    if (wd.size() < kDocPlainPrefix || u16(wd, 0) != 0xA5EC || u16(wd, 2) < kFirstBinary97) return false;
    h->flags = u16(wd, 10);
    h->key = u32(wd, 14);
    h->table = (h->flags & 0x0200) ? QStringLiteral("1Table") : QStringLiteral("0Table");
    return (h->flags & kEncrypted) != 0;
}

// RC4 over the bytes of a stream from `from` on, by blocks of 512 counted from the start of the stream; the bytes before `from` are not touched.
bool rc4Stream(QByteArray *s, qsizetype from, const Rc4Keys &keys, const std::function<bool()> &cancelled)
{
    s->detach();
    char *d = s->data();
    const qsizetype size = s->size();
    for (qsizetype block = from / kBlock; block * kBlock < size; ++block) {
        if (cancelled && (block & 0xFF) == 0 && cancelled()) return false;
        QByteArray key = keys.forBlock(quint32(block));
        crypto::Rc4 rc4(key);
        crypto::wipe(key);
        const qsizetype start = block * kBlock, end = std::min<qsizetype>(size, start + kBlock);
        if (start < from) rc4.skip(from - start);   // (the keystream runs on over the bytes that are not enciphered)
        rc4.apply(d + std::max(start, from), end - std::max(start, from));
    }
    return true;
}

// NOTE (open question, left as it is on purpose): the specification for the document format's obfuscation is method 2 of MS-OFFCRYPTO, whose data transformation (section 2.3.7.6) is a
// plain XOR with the 16-byte array that leaves a byte alone when it or the result is zero, with no rotation. This is method 1's transformation (section 2.3.7.3: XOR, then a rotation by
// five bits, the workbook format's), and the array and the verifier below are method 1's too (sections 2.3.7.1, 2.3.7.2; method 2's are 2.3.7.4, 2.3.7.5). The code differs from the
// specification and was not changed: there is no real protected file of this kind to settle which is right, and a change made from the text alone could break files that open today. The
// workbook format's code follows its own specification and has a real-file test.
void xorStream(QByteArray *s, qsizetype from, const std::array<quint8, 16> &table)
{
    s->detach();
    char *d = s->data();
    for (qsizetype q = from; q < s->size(); ++q) {
        const quint8 c = quint8(d[q]) ^ table[size_t(q % 16)];
        d[q] = char(quint8((c << 3) | (c >> 5)));
    }
}

} // namespace

bool docEncrypted(const QByteArray &file)
{
    if (file.size() < 512 || !file.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1"))) return false;
    cfb::File cf;
    if (!cfb::read(file, &cf)) return false;
    Header h;
    return readHeader(cf.stream(QStringLiteral("WordDocument")), &h);
}

CryptResult decryptDoc(const QByteArray &file, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain, const std::function<bool()> &cancelled)
{
    if (file.size() > maxBytes) return CryptResult::TooLarge;
    cfb::File cf;
    if (!cfb::read(file, &cf)) return CryptResult::Damaged;
    QByteArray wd = cf.stream(QStringLiteral("WordDocument"));
    Header h;
    if (!readHeader(wd, &h)) return CryptResult::Damaged;
    QByteArray table = cf.stream(h.table), data = cf.stream(QStringLiteral("Data"));
    if (qint64(wd.size()) + table.size() + data.size() > maxBytes) return CryptResult::TooLarge;
    struct StreamWipe {
        QByteArray *a, *b, *c;
        ~StreamWipe()
        {
            crypto::wipe(*a);
            crypto::wipe(*b);
            crypto::wipe(*c);
        }
    } streamWipe{&wd, &table, &data};   // (a wrong password, or a failure, leaves no deciphered bytes behind)

    qsizetype tableFrom = 0;
    if (h.flags & kObfuscated) {
        QByteArray pw;
        Wiper pwWiper;
        pwWiper.add(&pw);
        if (!ansiPassword(password, &pw) || pw.isEmpty()) return CryptResult::WrongPassword;
        // (The verifier and the key are method 1's, sections 2.3.7.1 and 2.3.7.2, not method 2's, sections 2.3.7.4 and 2.3.7.5, which the document format's specification names: see xorStream.)
        const quint16 key = quint16(h.key & 0xFFFF), verifier = quint16(h.key >> 16);
        if (((xorVerifier(pw) ^ verifier) | (xorKey(pw) ^ key)) != 0) return CryptResult::WrongPassword;
        if (verifyOnly) return CryptResult::Ok;
        std::array<quint8, 16> t = xorArray(pw, key);
        xorStream(&wd, kDocPlainPrefix, t);
        xorStream(&table, 0, t);
        xorStream(&data, 0, t);
        crypto::secureZero(t.data(), t.size());
    } else {
        // the description of the protection is at the start of the table stream, as long as the header says (the 2 bytes in front of it are what a workbook's record has there)
        const quint32 headerSize = h.key;
        if (headerSize < 4 + 48 || headerSize > 4096 || headerSize > quint32(table.size())) return CryptResult::Damaged;
        const QByteArray description = QByteArray(2, 0) + table.left(qsizetype(headerSize));
        Rc4Keys keys;
        Wiper wiper;
        wiper.add(&keys.base);
        const CryptResult r = rc4Setup(description, verifyOnly, password, &keys);
        if (r != CryptResult::Ok) return r;
        if (verifyOnly) return CryptResult::Ok;
        tableFrom = qsizetype(headerSize);
        if (!rc4Stream(&wd, kDocPlainPrefix, keys, cancelled) || !rc4Stream(&table, tableFrom, keys, cancelled) || !rc4Stream(&data, 0, keys, cancelled)) return CryptResult::Cancelled;
    }
    // the file as it was: the mark of protection is off, and the key is gone from the header
    wd[10] = char((h.flags & ~(kEncrypted | kObfuscated)) & 0xFF);
    wd[11] = char(((h.flags & ~(kEncrypted | kObfuscated)) >> 8) & 0xFF);
    for (int i = 0; i < 4; ++i) wd[14 + i] = 0;
    cf.setStream(QStringLiteral("WordDocument"), wd);
    cf.setStream(h.table, table);
    if (cf.find(QStringLiteral("Data")) >= 0) cf.setStream(QStringLiteral("Data"), data);
    *plain = cfb::write(cf);
    return plain->isEmpty() ? CryptResult::Damaged : CryptResult::Ok;
}

} // namespace jp::office::doc
