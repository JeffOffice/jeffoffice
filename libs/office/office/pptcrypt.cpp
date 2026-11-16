// The protection of the old binary presentation; see pptcrypt.h.

#include "office/pptcrypt.h"

#include "io/cfb.h"
#include "io/crypto.h"
#include "office/binarycrypt.h"

#include <QHash>
#include <QMap>
#include <QSet>

#include <algorithm>

namespace jp::office::ppt {

using namespace bincrypt;

namespace {

constexpr quint16 kUserEditAtom = 4085, kPersistDirectoryAtom = 6002, kCryptSession10Container = 12052;
constexpr quint32 kEncryptedToken = 0xF3D1C4DF, kPlainToken = 0xE391C05F;
constexpr qint64 kMaxDescription = 8192;

quint16 u16(const QByteArray &b, qsizetype at) { return quint16(quint8(b[at]) | quint8(b[at + 1]) << 8); }
quint32 u32(const QByteArray &b, qsizetype at) { return quint32(u16(b, at)) | quint32(u16(b, at + 2)) << 16; }

// The record header at `pos` and its data, if both lie in the bytes.
bool recAt(const QByteArray &d, qint64 pos, quint16 *type, quint16 *inst, qint64 *len)
{
    if (pos < 0 || pos + 8 > d.size()) return false;
    const qint64 l = u32(d, pos + 4);
    if (pos + 8 + l > d.size()) return false;
    *type = u16(d, pos + 2);
    *inst = quint16(u16(d, pos) >> 4);
    *len = l;
    return true;
}

struct Edit {
    qint64 at = 0;   // where the record is
    quint32 lastEdit = 0, directory = 0, encryptRef = 0;
    bool hasRef = false;
};

bool readEdit(const QByteArray &d, qint64 pos, Edit *e)
{
    quint16 type, inst;
    qint64 len;
    if (!recAt(d, pos, &type, &inst, &len) || type != kUserEditAtom || len < 28) return false;
    e->at = pos;
    e->lastEdit = u32(d, pos + 8 + 8);
    e->directory = u32(d, pos + 8 + 12);
    e->hasRef = len >= 32;
    e->encryptRef = e->hasRef ? u32(d, pos + 8 + 28) : 0;
    return true;
}

struct Located {
    QByteArray user;
    QString streamName;
    QByteArray stream;
    qint64 currentEdit = 0;
    quint32 token = 0;
};

bool locate(const cfb::File &cf, Located *l)
{
    l->user = cf.stream(QStringLiteral("Current User"));
    quint16 type, inst;
    qint64 len;
    if (!recAt(l->user, 0, &type, &inst, &len) || type != 4086 || len < 16) return false;
    l->token = u32(l->user, 8 + 4);
    l->currentEdit = u32(l->user, 8 + 8);
    // The presentation's stream is named for the program that wrote it, ending " Document".
    for (const QString &path : cf.streamPaths()) {
        if (path.contains(QLatin1Char('/')) || !path.endsWith(QLatin1String(" Document"))) continue;
        l->streamName = path;
        l->stream = cf.stream(path);
        return true;
    }
    return false;
}

// The records of the Pictures stream that are pictures, and the fields of each that are enciphered on their own (each from block 0).
bool isBlip(quint16 type) { return type >= 0xF018 && type <= 0xF117; }

void cryptField(const Rc4Keys &keys, QByteArray *s, qint64 from, qint64 size)
{
    if (size <= 0) return;
    crypto::Rc4 rc4 = keys.cipher(0);
    rc4.apply(s->data() + from, qsizetype(size));
}

void cryptPictures(const Rc4Keys &keys, QByteArray *s, const std::function<bool()> &cancelled, bool *stopped)
{
    s->detach();
    qint64 pos = 0;
    int n = 0;
    while (pos + 8 <= s->size()) {
        if (cancelled && (++n & 0xFF) == 0 && cancelled()) {
            *stopped = true;
            return;
        }
        cryptField(keys, s, pos, 8);
        const quint16 type = u16(*s, pos + 2), inst = quint16(u16(*s, pos) >> 4);
        const qint64 len = u32(*s, pos + 4), end = pos + 8 + len;
        if (!isBlip(type) || end > s->size()) {
            // (not a picture, or one cut short: what is left is not read, and the header is put back as it was found)
            cryptField(keys, s, pos, 8);
            return;
        }
        qint64 at = pos + 8;
        const bool metafile = type >= 0xF01A && type <= 0xF01C;
        const qint64 uids = (inst & 1) ? 2 : 1, fixed = 16 * uids + (metafile ? 34 : 1);
        if (len < fixed) return;
        for (qint64 i = 0; i < uids; ++i, at += 16) cryptField(keys, s, at, 16);
        const qint64 head = metafile ? 34 : 1;
        cryptField(keys, s, at, head);
        at += head;
        cryptField(keys, s, at, end - at);
        pos = end;
    }
}

} // namespace

bool pptEncrypted(const QByteArray &file)
{
    if (file.size() < 512 || !file.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1"))) return false;
    cfb::File cf;
    if (!cfb::read(file, &cf)) return false;
    Located l;
    if (!locate(cf, &l)) return false;
    if (l.token == kEncryptedToken) return true;
    Edit e;
    return readEdit(l.stream, l.currentEdit, &e) && e.encryptRef != 0;
}

CryptResult decryptPpt(const QByteArray &file, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain, const std::function<bool()> &cancelled)
{
    if (file.size() > maxBytes) return CryptResult::TooLarge;
    cfb::File cf;
    if (!cfb::read(file, &cf)) return CryptResult::Damaged;
    Located l;
    if (!locate(cf, &l)) return CryptResult::Damaged;
    QByteArray pictures = cf.stream(QStringLiteral("Pictures"));
    if (qint64(l.stream.size()) + pictures.size() > maxBytes) return CryptResult::TooLarge;
    struct StreamWipe {
        QByteArray *a, *b;
        ~StreamWipe()
        {
            crypto::wipe(*a);
            crypto::wipe(*b);
        }
    } streamWipe{&l.stream, &pictures};   // (a wrong password, or a failure, leaves no deciphered bytes behind)

    // The edits, newest first: the newest names the description of the protection, and every directory lists persist objects.
    Edit newest;
    if (!readEdit(l.stream, l.currentEdit, &newest)) return CryptResult::Damaged;
    if (!newest.hasRef || newest.encryptRef == 0) return CryptResult::Damaged;
    QMap<qint64, quint32> objects;   // offset -> persist id
    QHash<quint32, qint64> newestOffset;
    QSet<qint64> directories, edits;
    qint64 entries = 0;
    const qint64 maxEntries = l.stream.size() / 4;
    Edit e = newest;
    for (;;) {
        if (edits.contains(e.at) || edits.size() >= kMaxPptEdits) return CryptResult::Damaged;
        edits.insert(e.at);
        quint16 type, inst;
        qint64 len;
        if (!directories.contains(e.directory)) {
            directories.insert(e.directory);
            if (!recAt(l.stream, e.directory, &type, &inst, &len) || type != kPersistDirectoryAtom) return CryptResult::Damaged;
            qint64 p = qint64(e.directory) + 8;
            const qint64 end = p + len;
            while (p + 4 <= end) {
                const quint32 v = u32(l.stream, p);
                p += 4;
                const quint32 first = v & 0xFFFFF, count = v >> 20;
                for (quint32 i = 0; i < count && p + 4 <= end; ++i, p += 4) {
                    if (++entries > maxEntries) return CryptResult::Damaged;
                    const qint64 at = u32(l.stream, p);
                    const auto it = objects.constFind(at);
                    if (it != objects.constEnd() && it.value() != first + i) return CryptResult::Damaged;   // (one record that two ids claim)
                    objects.insert(at, first + i);
                    if (!newestOffset.contains(first + i)) newestOffset.insert(first + i, at);
                }
            }
        }
        if (e.lastEdit == 0) break;
        Edit older;
        if (!readEdit(l.stream, e.lastEdit, &older)) return CryptResult::Damaged;
        e = older;
    }
    const auto descAt = newestOffset.constFind(newest.encryptRef);
    quint16 type, inst;
    qint64 len;
    if (descAt == newestOffset.constEnd() || !recAt(l.stream, descAt.value(), &type, &inst, &len) || type != kCryptSession10Container) return CryptResult::Damaged;
    if (len < 6 || len > kMaxDescription) return CryptResult::Damaged;
    // the description is not enciphered: the two bytes in front of it are what a workbook's record has there
    const QByteArray description = QByteArray(2, 0) + l.stream.mid(descAt.value() + 8, qsizetype(len));
    Rc4Keys keys;
    Wiper wiper;
    wiper.add(&keys.base);
    const CryptResult r = rc4Setup(description, verifyOnly, password, &keys);
    if (r != CryptResult::Ok) return r;
    if (!keys.cryptoApi) return CryptResult::Unsupported;   // (the 40-bit form is not one a presentation has)
    if (verifyOnly) return CryptResult::Ok;

    // Each persist object, in the order of the stream, which may not overlap another.
    qint64 previousEnd = 0, done = 0;
    l.stream.detach();
    for (auto it = objects.cbegin(); it != objects.cend(); ++it) {
        if (cancelled && (++done & 0xFF) == 0 && cancelled()) return CryptResult::Cancelled;
        const qint64 at = it.key();
        if (it.value() == newest.encryptRef) continue;   // (the description stays as it is)
        if (at < previousEnd || at + 8 > l.stream.size()) return CryptResult::Damaged;
        crypto::Rc4 rc4 = keys.cipher(it.value());
        rc4.apply(l.stream.data() + at, 8);
        const qint64 end = at + 8 + qint64(u32(l.stream, at + 4));
        if (end > l.stream.size()) return CryptResult::Damaged;
        rc4.apply(l.stream.data() + at + 8, qsizetype(end - at - 8));
        previousEnd = end;
    }
    bool stopped = false;
    cryptPictures(keys, &pictures, cancelled, &stopped);
    if (stopped) return CryptResult::Cancelled;

    // the file as it was: the mark of protection is off, and nothing names a description
    QByteArray user = l.user;
    user.replace(12, 4, QByteArray(4, 0));
    for (int i = 0; i < 4; ++i) user[12 + i] = char((kPlainToken >> (8 * i)) & 0xFF);
    for (int i = 0; i < 4; ++i) l.stream[newest.at + 8 + 28 + i] = 0;
    cf.setStream(QStringLiteral("Current User"), user);
    cf.setStream(l.streamName, l.stream);
    if (cf.find(QStringLiteral("Pictures")) >= 0) cf.setStream(QStringLiteral("Pictures"), pictures);
    *plain = cfb::write(cf);
    return plain->isEmpty() ? CryptResult::Damaged : CryptResult::Ok;
}

} // namespace jp::office::ppt
