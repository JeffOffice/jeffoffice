// The protection of the old binary presentation; see pptcrypt.h.

#include "office/pptcrypt.h"

#include "io/cfb.h"
#include "io/crypto.h"
#include "office/binarycrypt.h"

#include <QHash>
#include <QMap>
#include <QSet>
#include <QVector>

#include <algorithm>

namespace jp::office::ppt {

using namespace bincrypt;

namespace {

constexpr quint16 kUserEditAtom = 4085, kPersistDirectoryAtom = 6002, kCryptSession10Container = 12052;
constexpr quint32 kEncryptedToken = 0xF3D1C4DF, kPlainToken = 0xE391C05F;
constexpr int kMaxIdsAtOneOffset = 8;

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

// What the Current User stream says: the header token and where the newest edit is. False if it is not that stream.
bool readCurrentUser(const QByteArray &user, quint32 *token, qint64 *currentEdit)
{
    quint16 type, inst;
    qint64 len;
    if (!recAt(user, 0, &type, &inst, &len) || type != 4086 || len < 16) return false;
    *token = u32(user, 8 + 4);
    *currentEdit = u32(user, 8 + 8);
    return true;
}

// The index of the presentation's stream, named for the program that wrote it, ending " Document".
int presentationStream(const cfb::File &cf)
{
    for (const QString &path : cf.streamPaths()) {
        if (path.contains(QLatin1Char('/')) || !path.endsWith(QLatin1String(" Document"))) continue;
        return cf.find(path);
    }
    return -1;
}

// The records of the Pictures stream that are pictures, and the fields of each that are enciphered on their own (each from block 0).
bool isBlip(quint16 type) { return type >= 0xF018 && type <= 0xF117; }

void cryptField(const Rc4Keys &keys, QByteArray *s, qint64 from, qint64 size)
{
    if (size <= 0) return;
    crypto::Rc4 rc4 = keys.cipher(0);
    rc4.apply(s->data() + from, qsizetype(size));
}

bool cryptPictures(const Rc4Keys &keys, QByteArray *s, const std::function<bool()> &cancelled)
{
    s->detach();
    qint64 pos = 0;
    while (pos + 8 <= s->size()) {
        if (cancelled && cancelled()) return false;
        cryptField(keys, s, pos, 8);
        const quint16 type = u16(*s, pos + 2), inst = quint16(u16(*s, pos) >> 4);
        const qint64 len = u32(*s, pos + 4), end = pos + 8 + len;
        qint64 at = pos + 8;
        const bool metafile = type >= 0xF01A && type <= 0xF01C;
        const qint64 uids = (inst & 1) ? 2 : 1, head = metafile ? 34 : 1;
        if (!isBlip(type) || end > s->size() || len < 16 * uids + head) {
            // (not a picture, or one cut short: what is left is not read, and the header is put back as it was found)
            cryptField(keys, s, pos, 8);
            return true;
        }
        for (qint64 i = 0; i < uids; ++i, at += 16) cryptField(keys, s, at, 16);
        cryptField(keys, s, at, head);
        at += head;
        cryptField(keys, s, at, end - at);
        pos = end;
    }
    return true;
}

// A record that could be the start of a persist object: a type in the range the format uses, whose data lies in the stream.
bool looksLikeObject(const QByteArray &header, qint64 at, qint64 streamSize)
{
    const quint16 type = u16(header, 2);
    return (quint8(header[0]) & 0xF) <= 0xF && type >= 1000 && type <= 13000 && at + 8 + qint64(u32(header, 4)) <= streamSize;
}

struct Ranges {
    QMap<qint64, qint64> byStart;   // start -> end
    // False if [start, end) overlaps a range already held (the same range again is not an overlap: one atom named twice).
    bool add(qint64 start, qint64 end)
    {
        auto next = byStart.lowerBound(start);
        if (next != byStart.end() && next.key() == start) return next.value() == end;
        if (next != byStart.end() && next.key() < end) return false;
        if (next != byStart.begin()) {
            auto before = std::prev(next);
            if (before.value() > start) return false;
        }
        byStart.insert(start, end);
        return true;
    }
};

} // namespace

bool pptEncrypted(const QByteArray &file, bool *isPpt)
{
    if (isPpt) *isPpt = false;
    if (file.size() < 512 || !file.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1"))) return false;
    // (only the sectors of the one small stream are read: a document or a workbook has none, and a presentation that is not protected is read whole only by its own reader)
    const QByteArray user = cfb::readStream(file, QStringLiteral("Current User"));
    quint32 token;
    qint64 currentEdit;
    if (user.isEmpty() || !readCurrentUser(user, &token, &currentEdit)) return false;
    if (isPpt) *isPpt = true;
    return token == kEncryptedToken;
}

CryptResult decryptPpt(const QByteArray &file, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain, const std::function<bool()> &cancelled)
{
    if (file.size() > maxBytes) return CryptResult::TooLarge;
    cfb::File cf;
    if (!cfb::read(file, &cf)) return CryptResult::Damaged;
    // (the streams are worked on where they are held, not copied: the file, its parts, and the result are the three copies at the most)
    const int streamIndex = presentationStream(cf), picturesIndex = cf.find(QStringLiteral("Pictures"));
    QByteArray user = cf.stream(QStringLiteral("Current User"));
    quint32 token;
    qint64 currentEdit;
    if (streamIndex < 0 || !readCurrentUser(user, &token, &currentEdit)) return CryptResult::Damaged;
    QByteArray &stream = cf.entries[streamIndex].data;
    QByteArray empty;
    QByteArray &pictures = picturesIndex >= 0 ? cf.entries[picturesIndex].data : empty;
    if (qint64(stream.size()) + pictures.size() > maxBytes) return CryptResult::TooLarge;
    struct StreamWipe {
        QByteArray *a, *b;
        ~StreamWipe()
        {
            crypto::wipe(*a);
            crypto::wipe(*b);
        }
    } streamWipe{&stream, &pictures};   // (a wrong password, or a failure, leaves no deciphered bytes behind)

    // The edits, newest first: the newest names the description of the protection, and every directory lists persist objects. All of it is read before the password is used, so it is
    // bounded here: the edits (kMaxPptEdits), the words of the directories all together (an eighth of the stream's bytes, so that a stream that is mostly directory is refused), and the
    // atoms may not overlap.
    Edit newest;
    if (!readEdit(stream, currentEdit, &newest)) return CryptResult::Damaged;
    if (!newest.hasRef || newest.encryptRef == 0) return CryptResult::Damaged;
    QMap<qint64, QVector<quint32>> objects;   // offset -> the persist ids that name it
    QHash<quint32, qint64> newestOffset;
    QSet<qint64> directories;
    QVector<qint64> editsWithRef;
    Ranges atoms;
    qint64 words = 0, edits = 0;
    const qint64 maxWords = stream.size() / 8 + 16;
    Edit e = newest;
    for (;;) {
        if (cancelled && cancelled()) return CryptResult::Cancelled;
        if (++edits > kMaxPptEdits) return CryptResult::Damaged;
        if (!atoms.add(e.at, e.at + 8 + u32(stream, e.at + 4))) return CryptResult::Damaged;
        if (e.hasRef && e.encryptRef != 0) editsWithRef << e.at;
        quint16 type, inst;
        qint64 len;
        if (!directories.contains(e.directory)) {
            directories.insert(e.directory);
            if (!recAt(stream, e.directory, &type, &inst, &len) || type != kPersistDirectoryAtom) return CryptResult::Damaged;
            if (!atoms.add(e.directory, e.directory + 8 + len)) return CryptResult::Damaged;
            QSet<quint32> ids;   // (an id is listed once in a directory)
            qint64 p = qint64(e.directory) + 8;
            const qint64 end = p + len;
            while (p + 4 <= end) {
                if (++words > maxWords || ((words & 0x3FF) == 0 && cancelled && cancelled())) return words > maxWords ? CryptResult::Damaged : CryptResult::Cancelled;
                const quint32 v = u32(stream, p);
                p += 4;
                const quint32 first = v & 0xFFFFF, count = v >> 20;
                for (quint32 i = 0; i < count && p + 4 <= end; ++i, p += 4) {
                    if (++words > maxWords) return CryptResult::Damaged;
                    if (ids.contains(first + i)) return CryptResult::Damaged;
                    ids.insert(first + i);
                    const qint64 at = u32(stream, p);
                    QVector<quint32> &named = objects[at];
                    if (!named.contains(first + i)) {
                        if (named.size() >= kMaxIdsAtOneOffset) return CryptResult::Damaged;
                        named << first + i;
                    }
                    if (!newestOffset.contains(first + i)) newestOffset.insert(first + i, at);
                }
            }
        }
        if (e.lastEdit == 0) break;
        Edit older;
        if (!readEdit(stream, e.lastEdit, &older)) return CryptResult::Damaged;
        e = older;
    }
    const auto descAt = newestOffset.constFind(newest.encryptRef);
    quint16 type, inst;
    qint64 len;
    if (descAt == newestOffset.constEnd() || !recAt(stream, descAt.value(), &type, &inst, &len) || type != kCryptSession10Container) return CryptResult::Damaged;
    if (len < 6 || len > kMaxDescription) return CryptResult::Damaged;
    // the description is not enciphered: the two bytes in front of it are what a workbook's record has there
    const QByteArray description = QByteArray(2, 0) + stream.mid(descAt.value() + 8, qsizetype(len));
    Rc4Keys keys;
    Wiper wiper;
    wiper.add(&keys.base);
    CryptResult r = rc4Setup(description, verifyOnly, password, &keys);
    if (r != CryptResult::Ok && r != CryptResult::WrongPassword) return r;
    // (the first form of the 40-bit kind, which a workbook has, is not one a presentation has: that is said before a password is asked for)
    if (!keys.cryptoApi) return CryptResult::Unsupported;
    if (r != CryptResult::Ok) return r;
    if (verifyOnly) return CryptResult::Ok;

    // Each persist object, in the order of the stream, which may not overlap another. An offset that several ids name (freed ids may point at the start of the stream) is read with
    // the id whose key gives a header that makes sense.
    qint64 previousEnd = 0;
    stream.detach();
    for (auto it = objects.cbegin(); it != objects.cend(); ++it) {
        if (cancelled && cancelled()) return CryptResult::Cancelled;
        const qint64 at = it.key();
        if (it.value().contains(newest.encryptRef)) continue;   // (the description stays as it is)
        if (at < previousEnd || at + 8 > stream.size()) return CryptResult::Damaged;
        quint32 id = it.value().first();
        if (it.value().size() > 1) {
            bool found = false;
            QVector<quint32> candidates = it.value();
            std::sort(candidates.begin(), candidates.end());
            for (quint32 c : candidates) {
                QByteArray header = stream.mid(at, 8);
                keys.cipher(c).apply(header.data(), 8);
                if (looksLikeObject(header, at, stream.size())) {
                    id = c;
                    found = true;
                    break;
                }
            }
            if (!found) return CryptResult::Damaged;
        }
        crypto::Rc4 rc4 = keys.cipher(id);
        rc4.apply(stream.data() + at, 8);
        const qint64 end = at + 8 + qint64(u32(stream, at + 4));
        if (end > stream.size()) return CryptResult::Damaged;
        rc4.apply(stream.data() + at + 8, qsizetype(end - at - 8));
        previousEnd = end;
    }
    if (!cryptPictures(keys, &pictures, cancelled)) return CryptResult::Cancelled;

    // the file as it was: the mark of protection is off, and no edit names a description
    for (int i = 0; i < 4; ++i) user[12 + i] = char((kPlainToken >> (8 * i)) & 0xFF);
    for (const qint64 at : editsWithRef)
        for (int i = 0; i < 4; ++i) stream[at + 8 + 28 + i] = 0;
    cf.setStream(QStringLiteral("Current User"), user);
    *plain = cfb::write(cf);
    return plain->isEmpty() ? CryptResult::Damaged : CryptResult::Ok;
}

} // namespace jp::office::ppt
