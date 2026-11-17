#include "io/pptsalvage.h"

#include "io/pptimport.h"
#include "office/cfbsalvage.h"

#include <QCoreApplication>
#include <QMap>
#include <QtEndian>

#include <algorithm>

namespace jp::deck {

namespace {

constexpr quint16 kDocument = 1000, kSlide = 1006, kNotes = 1008, kSlidePersist = 1011, kMainMaster = 1016, kSlideList = 4080, kUserEdit = 4085, kCurrentUser = 4086, kPersistDirectory = 6002;
constexpr quint32 kTokenPlain = 0xE391C05F, kTokenEncrypted = 0xF3D1C4DF;

quint16 u16(QByteArrayView b, qint64 o) { return o >= 0 && o + 2 <= b.size() ? qFromLittleEndian<quint16>(b.constData() + o) : 0; }
quint32 u32(QByteArrayView b, qint64 o) { return o >= 0 && o + 4 <= b.size() ? qFromLittleEndian<quint32>(b.constData() + o) : 0; }

thread_local qint64 g_walked = 0;

struct Top {
    qint64 off = 0;   // of the header
    quint16 type = 0, inst = 0;
    qint64 len = 0;
    qint64 end() const { return off + 8 + len; }
};

// The whole records at the top of a stream, one after another from its start, to where one does not fit or is of a kind no record is (a type outside 1000 to 11999).
QVector<Top> topRecords(QByteArrayView d, qint64 *end)
{
    QVector<Top> out;
    qint64 pos = 0;
    while (pos + 8 <= d.size() && out.size() < kSalvageMaxRecords) {
        Top t;
        t.off = pos;
        t.inst = u16(d, pos) >> 4;
        t.type = u16(d, pos + 2);
        t.len = u32(d, pos + 4);
        if (t.type < 1000 || t.type >= 12000 || t.len > d.size() - pos - 8) break;
        out << t;
        pos = t.end();
    }
    g_walked = std::max<qint64>(g_walked, out.size());
    if (end) *end = pos;
    return out;
}

QByteArray record(quint16 type, const QByteArray &data)
{
    QByteArray r(8, '\0');
    qToLittleEndian(quint16(0), r.data());
    qToLittleEndian(type, r.data() + 2);
    qToLittleEndian(quint32(data.size()), r.data() + 4);
    return r + data;
}

QByteArray le32(quint32 v)
{
    QByteArray b(4, '\0');
    qToLittleEndian(v, b.data());
    return b;
}

QByteArray currentUser(quint32 editOffset)
{
    // (the size of the fixed part, the mark, the edit, no name, the version of the file format, and 2 bytes unused)
    return record(kCurrentUser, le32(20) + le32(kTokenPlain) + le32(editOffset) + QByteArray(2, '\0') + QByteArray::fromHex("f4030300") + QByteArray(2, '\0'));
}

QByteArray userEdit(quint32 lastEdit, quint32 directory, quint32 document, quint32 seed)
{
    return record(kUserEdit, le32(0) + QByteArray::fromHex("f4030300") + le32(lastEdit) + le32(directory) + le32(document) + le32(seed) + QByteArray(4, '\0'));
}

// The slides, masters, and notes of the document record, in the order it lists them: the persist ids, from the slide list records of inst 0 (slides), 1 (masters), and 2 (notes).
void listedIds(QByteArrayView d, const Top &doc, QVector<quint32> ids[3])
{
    qint64 pos = doc.off + 8;
    int guard = 0;
    while (pos + 8 <= doc.end() && ++guard < kSalvageMaxRecords) {
        const quint16 type = u16(d, pos + 2);
        const qint64 len = u32(d, pos + 4);
        if (len > doc.end() - pos - 8) break;
        if (type == kSlideList) {
            const int inst = u16(d, pos) >> 4;
            qint64 q = pos + 8;
            while (q + 8 <= pos + 8 + len) {
                const qint64 l = u32(d, q + 4);
                if (l > pos + 8 + len - q - 8) break;
                if (u16(d, q + 2) == kSlidePersist && inst <= 2 && l >= 4) ids[inst] << u32(d, q + 8);
                q += 8 + l;
            }
        }
        pos += 8 + len;
    }
}

} // namespace

qint64 salvageRecordsWalkedForTest() { return g_walked; }

QByteArray salvagePresentation(const QByteArray &file, QStringList *notes, QString *error)
{
    using jp::office::CfbSalvage;
    using jp::office::CfbWish;
    // the presentation's stream starts with its document record (whose first child is the document's atom); it is whole as far as its records go
    CfbWish stream;
    stream.name = QStringLiteral("PowerPoint Document");
    stream.nameMatches = [](const QString &n) { return n.endsWith(QLatin1String(" Document"), Qt::CaseInsensitive) && !n.contains(QLatin1Char('/')); };
    stream.looksLike = [](QByteArrayView from, const QMap<QString, QByteArray> &) { return u16(from, 0) == 0x000F && u16(from, 2) == kDocument && u16(from, 10) == 1001; };
    stream.validBytes = [](QByteArrayView bytes, const QMap<QString, QByteArray> &) -> qint64 {
        qint64 end = 0;
        const QVector<Top> tops = topRecords(bytes, &end);
        for (const Top &t : tops)
            if (t.type == kDocument) return end;
        return 0;
    };
    CfbWish user;
    user.name = QStringLiteral("Current User");
    user.required = false;
    user.looksLike = [](QByteArrayView from, const QMap<QString, QByteArray> &) { return u16(from, 0) == 0 && u16(from, 2) == kCurrentUser && u32(from, 8) == 20; };
    user.validBytes = [](QByteArrayView bytes, const QMap<QString, QByteArray> &) -> qint64 {
        const quint32 token = u32(bytes, 12);
        return u16(bytes, 2) == kCurrentUser && u32(bytes, 8) == 20 && (token == kTokenPlain || token == kTokenEncrypted) ? qint64(8) + u32(bytes, 4) : 0;
    };
    CfbWish pictures;   // (no scan: its records can't be told from the pictures inside others)
    pictures.name = QStringLiteral("Pictures");
    pictures.required = false;

    CfbSalvage found;
    QString why;
    if (!jp::office::salvageCompoundFile(file, {user, stream, pictures}, &found, &why)) {
        // (a presentation protected with a password has its records enciphered, and none is found by its look)
        if (u32(found.streams.value(QStringLiteral("Current User")), 12) == kTokenEncrypted || found.directoryNames.contains(QStringLiteral("EncryptedPackage"), Qt::CaseInsensitive)) {
            if (error) *error = QCoreApplication::translate("jp::deck::PptSalvage", "The presentation is protected with a password, and can't be repaired.");
            return QByteArray();
        }
        if (error) *error = QCoreApplication::translate("jp::deck::PptSalvage", "No presentation could be found in the file: its first records are damaged or gone.");
        return QByteArray();
    }
    auto protectedFile = [&] {
        if (error) *error = QCoreApplication::translate("jp::deck::PptSalvage", "The presentation is protected with a password, and can't be repaired.");
        return QByteArray();
    };
    if (found.directoryNames.contains(QStringLiteral("EncryptedPackage"), Qt::CaseInsensitive)) return protectedFile();
    const QByteArray oldUser = found.streams.value(QStringLiteral("Current User"));
    if (u32(oldUser, 12) == kTokenEncrypted) return protectedFile();
    QStringList said = found.notes;

    auto opens = [&](const CfbSalvage &s, QString *because) { return importBinaryPresentation(jp::office::rebuildCompoundFile(s), because) != nullptr; };
    QString because;
    bool done = opens(found, &because);
    if (!done) {
        // the stream as far as its records go; the last edit in it that is whole, then the list of where the records are made again
        QByteArray d = found.streams.value(QStringLiteral("PowerPoint Document"));
        qint64 end = 0;
        const QVector<Top> tops = topRecords(d, &end);
        d.truncate(qsizetype(end));
        CfbSalvage fixed = found;
        fixed.streams.insert(QStringLiteral("PowerPoint Document"), d);
        // (an edit is whole when its list of places is, in the stream, a record of the right kind)
        QMap<qint64, Top> byOffset;
        for (const Top &t : tops) byOffset.insert(t.off, t);
        for (int i = int(tops.size()) - 1; i >= 0 && !done; --i) {
            const Top &t = tops[i];
            if (t.type != kUserEdit || t.len < 28) continue;
            const Top dir = byOffset.value(u32(d, t.off + 8 + 12), Top());
            if (dir.type != kPersistDirectory) continue;
            fixed.streams.insert(QStringLiteral("Current User"), currentUser(quint32(t.off)));
            if (opens(fixed, &because)) {
                done = true;
                found = fixed;
                said << QCoreApplication::translate("jp::deck::PptSalvage", "The end of the presentation was lost, so it was opened as it was at its last save that is whole.");
            }
        }
        if (!done) {
            // the places made again: the document record's list of slides, masters, and notes against the slide, master, and notes records, each in the order stored
            const Top *doc = nullptr;
            for (const Top &t : tops)
                if (t.type == kDocument) doc = &t;   // (the last: a later save writes it again)
            if (doc) {
                QVector<quint32> ids[3];
                listedIds(d, *doc, ids);
                QVector<qint64> offsets[3];
                for (const Top &t : tops) {
                    if (t.type == kSlide) offsets[0] << t.off;
                    else if (t.type == kMainMaster) offsets[1] << t.off;
                    else if (t.type == kNotes) offsets[2] << t.off;
                }
                QMap<quint32, quint32> at;
                quint32 top = 0;
                for (int k = 0; k < 3; ++k) {
                    for (quint32 id : std::as_const(ids[k])) top = std::max(top, id);
                    for (int i = 0; i < ids[k].size() && i < offsets[k].size(); ++i) at.insert(ids[k][i], quint32(offsets[k][i]));
                }
                const quint32 docId = at.contains(1) ? top + 1 : 1;
                at.insert(docId, quint32(doc->off));
                QByteArray directory;
                for (auto it = at.cbegin(); it != at.cend(); ++it) directory += le32((it.key() & 0xFFFFF) | (1u << 20)) + le32(it.value());
                const qint64 dirAt = d.size();
                d += record(kPersistDirectory, directory);
                const qint64 editAt = d.size();
                d += userEdit(0, quint32(dirAt), docId, top + 2);
                fixed.streams.insert(QStringLiteral("PowerPoint Document"), d);
                fixed.streams.insert(QStringLiteral("Current User"), currentUser(quint32(editAt)));
                if (opens(fixed, &because)) {
                    done = true;
                    found = fixed;
                    said << QCoreApplication::translate("jp::deck::PptSalvage", "The list that says where each slide is kept was lost, so the slides are in the order they are stored, which may not be the order they were in.");
                }
            }
        }
    }
    if (!done) {
        if (error) *error = because.isEmpty() ? QCoreApplication::translate("jp::deck::PptSalvage", "No slide of the presentation could be read.") : because;
        return QByteArray();
    }
    if (!found.streams.contains(QStringLiteral("Pictures")) && (!found.directoryRead || ((found.cutShort || found.tablesGuessed) && found.directoryNames.contains(QStringLiteral("Pictures"), Qt::CaseInsensitive))))
        said << QCoreApplication::translate("jp::deck::PptSalvage", "The pictures of the presentation, if it had any, could not be found.");
    if (notes) *notes = said;
    return jp::office::rebuildCompoundFile(found);
}

} // namespace jp::deck
