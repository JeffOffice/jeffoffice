// The protection of the old binary form; see xlscrypt.h.

#include "sheet/xlscrypt.h"

#include "io/crypto.h"
#include "office/binarycrypt.h"

#include <QStringEncoder>

#include <algorithm>
#include <array>

namespace jp::office::sheet {

using namespace bincrypt;

namespace {

constexpr quint16 kBof = 0x0809, kEof = 0x000A, kFilePass = 0x002F, kBoundSheet = 0x0085;
constexpr quint16 kRenamed = 0x0DFF;   // (a number the format does not use)

quint16 u16(const QByteArray &b, qsizetype at) { return quint16(quint8(b[at]) | quint8(b[at + 1]) << 8); }

// The records that are not enciphered: where a stream begins, FILEPASS itself, and the ones of a workbook shared between people.
bool clearRecord(quint16 id)
{
    switch (id) {
    case kBof: case kFilePass: case 0x0194 /*USREXCL*/: case 0x0195 /*FILELOCK*/: case 0x00E1 /*INTERFACEHDR*/: case 0x0196 /*RRDINFO*/: case 0x0138 /*RRDHEAD*/: return true;
    default: return false;
    }
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

// The bytes of the stream after `after` that the provider's and the 40-bit RC4 forms encipher, in place: every record but the ones in the clear, and but the first four bytes of a sheet's entry
// in the list of sheets (where the sheet begins). The same call enciphers and deciphers. False for a record that is not whole.
bool rc4Records(QByteArray *stream, qsizetype after, const Rc4Keys &keys)
{
    stream->detach();
    char *d = stream->data();
    const qsizetype size = stream->size();
    QByteArray keystream;
    Wiper wiper;
    wiper.add(&keystream);
    qint64 haveBlock = -1;
    qsizetype p = after;
    while (p + 4 <= size) {
        const quint16 id = u16(*stream, p), len = u16(*stream, p + 2);
        const qsizetype start = p + 4, end = start + len;
        if (end > size) return false;
        if (!clearRecord(id)) {
            qsizetype from = start;
            if (id == kBoundSheet) from = std::min<qsizetype>(end, start + 4);
            for (qsizetype q = from; q < end; ++q) {
                const qint64 block = q / 1024;
                if (block != haveBlock) {
                    QByteArray blockKey = keys.forBlock(quint32(block));
                    crypto::Rc4 rc4(blockKey);
                    crypto::wipe(blockKey);
                    crypto::wipe(keystream);
                    keystream = rc4.apply(QByteArray(1024, 0));
                    haveBlock = block;
                }
                d[q] = char(quint8(d[q]) ^ quint8(keystream[int(q % 1024)]));
            }
        }
        p = end;
    }
    return true;
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
    Wiper wiper;
    wiper.add(&keys.base);
    struct TableWipe {
        std::array<quint8, 16> *t;
        ~TableWipe() { crypto::secureZero(t->data(), t->size()); }
    } tableWipe{&table};
    if (xorForm) {
        const qsizetype at = biff8 ? 2 : 0;
        if (body.size() < at + 4) return CryptResult::Damaged;
        QByteArray pw;
        Wiper pwWiper;
        pwWiper.add(&pw);
        if (!ansiPassword(password, &pw) || pw.isEmpty()) return CryptResult::WrongPassword;
        const quint16 key = u16(body, at), verifier = u16(body, at + 2);
        const bool fits = ((xorVerifier(pw) ^ verifier) | (xorKey(pw) ^ key)) == 0;   // (no early exit)
        if (!fits) return CryptResult::WrongPassword;
        table = xorArray(pw, key);
    } else {
        const CryptResult r = rc4Setup(body, verifyOnly, password, &keys);
        if (r != CryptResult::Ok) return r;
    }
    if (verifyOnly) return CryptResult::Ok;
    // the records after FILEPASS, one at a time
    if (!xorForm) {
        if (!rc4Records(stream, after, keys)) return CryptResult::Damaged;
    } else {
        stream->detach();
        char *d = stream->data();
        const qsizetype size = stream->size();
        qsizetype p = after;
        while (p + 4 <= size) {
            const quint16 id = u16(*stream, p), len = u16(*stream, p + 2);
            const qsizetype start = p + 4, end = start + len;
            if (end > size) return CryptResult::Damaged;
            if (!clearRecord(id)) {
                qsizetype from = start;
                if (id == kBoundSheet) from = std::min<qsizetype>(end, start + 4);   // (where the sheet begins is not enciphered)
                for (qsizetype q = from; q < end; ++q) {
                    const quint8 c = quint8(d[q]) ^ table[size_t((start + len + (q - start)) % 16)];
                    d[q] = char(quint8((c << 3) | (c >> 5)));
                }
            }
            p = end;
        }
    }
    char *d = stream->data();
    d[fp.at] = char(kRenamed & 0xFF);
    d[fp.at + 1] = char(kRenamed >> 8);
    return CryptResult::Ok;
}

bool encryptBiff(QByteArray *stream, const QString &password, const QByteArray &salt, const QByteArray &verifier)
{
    if (password.isEmpty() || stream->size() < 8 || u16(*stream, 0) != kBof) return false;
    const qsizetype first = 4 + u16(*stream, 2);
    if (first + 4 > stream->size() || u16(*stream, 4) < 0x0600) return false;   // (the form is BIFF8)
    FilePass already;
    bool biff8;
    if (findFilePass(*stream, &already, &biff8)) return false;
    Rc4Keys keys;
    Wiper wiper;
    wiper.add(&keys.base);
    const QByteArray body = rc4CryptoApiHeader(password, &keys, salt, verifier);
    if (body.isEmpty() || body.size() > 0xFFFF) return false;
    QByteArray record;
    record.append(char(kFilePass & 0xFF)).append(char(kFilePass >> 8)).append(char(body.size() & 0xFF)).append(char(body.size() >> 8)).append(body);
    // FILEPASS goes in right after the first BOF; where each sheet begins (the list of sheets is in the globals, before the first EOF) moves by its size
    QByteArray out = stream->left(first) + record;
    out.reserve(stream->size() + record.size());
    qsizetype p = first;
    bool globals = true;
    while (p + 4 <= stream->size()) {
        const quint16 id = u16(*stream, p), len = u16(*stream, p + 2);
        if (p + 4 + len > stream->size()) return false;
        QByteArray rec = stream->mid(p, 4 + len);
        if (globals && id == kBoundSheet && len >= 4) {
            const quint32 at = quint32(u16(rec, 4)) | quint32(u16(rec, 6)) << 16;
            const quint32 moved = at + quint32(record.size());
            for (int i = 0; i < 4; ++i) rec[4 + i] = char((moved >> (8 * i)) & 0xFF);
        }
        if (id == kEof) globals = false;
        out += rec;
        p += 4 + len;
    }
    if (!rc4Records(&out, first + record.size(), keys)) return false;
    *stream = out;
    return true;
}

} // namespace jp::office::sheet
