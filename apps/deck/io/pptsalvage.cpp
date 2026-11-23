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
thread_local int g_opens = 0;

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

// Leaves out the part of the presentation that lies across each of `joins`, places where pieces of the file were joined by guess: the innermost record whose bytes run across a join is given a
// type no reader acts on (a container holding it keeps its other records). One pass down the records with the joins in order: each record and each join is looked at once (`*steps` counts
// them). Returns how many records were left out.
int dropAcrossJoins(QByteArray &d, QVector<qint64> joins, qint64 *steps)
{
    std::sort(joins.begin(), joins.end());
    struct Frame {
        qint64 pos, to;
    };
    QVector<Frame> stack;
    stack.push_back({0, d.size()});
    int dropped = 0, ji = 0;
    while (!stack.isEmpty() && ji < joins.size()) {
        Frame &f = stack.last();
        if (f.pos + 8 > f.to) {
            stack.pop_back();
            continue;
        }
        const qint64 pos = f.pos, len = u32(d, pos + 4), end = pos + 8 + len;
        ++*steps;
        if (len > f.to - pos - 8) {   // (a record that does not fit: nothing more is read in this one)
            while (ji < joins.size() && joins[ji] < f.to) ++ji;
            stack.pop_back();
            continue;
        }
        while (ji < joins.size() && joins[ji] <= pos) ++ji;
        f.pos = end;
        if (ji >= joins.size() || joins[ji] >= end) continue;
        if ((u16(d, pos) & 0xF) == 0xF && joins[ji] >= pos + 8 && stack.size() < 64) {
            stack.push_back({pos + 8, end});
            continue;
        }
        qToLittleEndian(quint16(9999), d.data() + pos + 2);
        ++dropped;
        while (ji < joins.size() && joins[ji] < end) {
            ++ji;
            ++*steps;
        }
    }
    return dropped;
}

// Where the presentation stops fitting its own structure after the first place where pieces were joined by guess. The presentation's lists of where its parts are kept (the persist
// directories) name the offset of every document, slide, notes, and master record; after the join, such a record that begins at an offset no list names, and an offset a list names
// that is not where a record begins, do not fit. Returns the start of the first record that does not fit, -1 if all fit. `*slide` is the number of the slide the join falls in (0 before the first).
qint64 stopAfterJoin(QByteArrayView d, qint64 join, int *slide)
{
    qint64 end = 0;
    const QVector<Top> tops = topRecords(d, &end);
    QVector<qint64> named;
    for (const Top &t : tops) {
        if (t.type != kPersistDirectory) continue;
        for (qint64 pos = t.off + 8; pos + 4 <= t.end();) {
            const quint32 head = u32(d, pos);
            const qint64 count = head >> 20;
            pos += 4;
            for (qint64 i = 0; i < count && pos + 4 <= t.end(); ++i, pos += 4) named << qint64(u32(d, pos));
        }
    }
    std::sort(named.begin(), named.end());
    *slide = 0;
    for (const Top &t : tops)
        if (t.type == kSlide && t.off <= join) ++*slide;
    qint64 stop = -1;
    auto fits = [&](const Top &t) { return t.type != kDocument && t.type != kSlide && t.type != kNotes && t.type != kMainMaster ? true : std::binary_search(named.cbegin(), named.cend(), t.off); };
    for (const Top &t : tops)
        if (t.off >= join && !fits(t)) {
            stop = t.off;
            break;
        }
    // (a place a list names, inside the stream, that is not the start of a record: the record that runs across it)
    for (qint64 place : std::as_const(named)) {
        if (place < join || place >= end) continue;
        const auto it = std::upper_bound(tops.cbegin(), tops.cend(), place, [](qint64 v, const Top &t) { return v < t.off; });
        if (it == tops.cbegin()) continue;
        const Top &t = *(it - 1);
        if (t.off != place) {
            stop = stop < 0 ? t.off : std::min(stop, t.off);
            break;
        }
    }
    return stop;
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
int salvageOpensForTest() { return g_opens; }

QByteArray salvagePresentation(const QByteArray &file, QStringList *notes, QString *error, qint64 *joinSteps)
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
    // it ends well when its records are whole to the end (zeros after them are the padding of a sector) and the last is an edit record, as every save ends it
    // The last edit record must be the one the presentation's own "Current User" stream names (its offset is the 4 bytes at 16); with that stream gone, the end can't be confirmed.
    stream.endsWell = [](QByteArrayView bytes, const QMap<QString, QByteArray> &found) {
        CfbWish::End end;
        qint64 at = 0;
        const QVector<Top> tops = topRecords(bytes, &at);
        end.prefix = at;
        bool zeros = true;
        for (qint64 i = at; i < bytes.size() && zeros; ++i) zeros = bytes[qsizetype(i)] == 0;
        const QByteArray user = found.value(QStringLiteral("Current User"));
        if (!tops.isEmpty() && tops.last().type == kUserEdit && zeros && u16(user, 2) == kCurrentUser && qint64(u32(user, 16)) == tops.last().off) end.whole = at;
        return end;
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
    int dropped = 0, joinSlide = 0;
    bool joined = false;
    {
        QByteArray d = found.streams.value(QStringLiteral("PowerPoint Document"));
        const QVector<qint64> joins = found.joins.value(QStringLiteral("PowerPoint Document"));
        qint64 steps = 0;
        dropped = dropAcrossJoins(d, joins, &steps);
        if (joinSteps) *joinSteps = steps + joins.size();
        // (what follows the first join is kept only while it fits the presentation's own lists of where its parts are)
        if (!joins.isEmpty() && joins.first() < d.size()) {
            const qint64 stop = stopAfterJoin(d, joins.first(), &joinSlide);
            if (stop >= 0) d.truncate(qsizetype(stop));
            joined = joins.first() < d.size();
        }
        found.streams.insert(QStringLiteral("PowerPoint Document"), d);
    }

    g_opens = 0;
    auto opens = [&](const CfbSalvage &s, QString *because) {
        ++g_opens;
        return importBinaryPresentation(jp::office::rebuildCompoundFile(s), because) != nullptr; };
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
        int tried = 0;
        for (int i = int(tops.size()) - 1; i >= 0 && !done && tried < kSalvageMaxEditTries; --i) {
            const Top &t = tops[i];
            if (t.type != kUserEdit || t.len < 28) continue;
            // (only an edit whose list of places is a whole record of the stream is tried)
            const Top dir = byOffset.value(u32(d, t.off + 8 + 12), Top());
            if (dir.type != kPersistDirectory) continue;
            ++tried;
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
                    // (more records than the list names are older copies, left by saves that added to the file: the newest are the last)
                    const int skip = std::max<int>(0, offsets[k].size() - ids[k].size());
                    for (int i = 0; i < ids[k].size() && i + skip < offsets[k].size(); ++i) at.insert(ids[k][i], quint32(offsets[k][i + skip]));
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
                    said << QCoreApplication::translate("jp::deck::PptSalvage", "The list that says where each slide is kept was lost, so the slides are in the order they are stored, which may not be the order they were in, and a slide may be an older version of itself.");
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
    if (joined) {
        if (joinSlide > 0) said << QCoreApplication::translate("jp::deck::PptSalvage", "What follows a place where pieces of the file were joined by a guess was checked only to see that it is in the right form, not that it belongs here. It starts in slide %1.").arg(joinSlide);
        else said << QCoreApplication::translate("jp::deck::PptSalvage", "What follows a place where pieces of the file were joined by a guess was checked only to see that it is in the right form, not that it belongs here. It starts before the first slide.");
    }
    if (dropped > 0) said << QCoreApplication::translate("jp::deck::PptSalvage", "%n part of the presentation (a shape, a line of text, or a picture) was left out because it lay across a place where pieces of the file were joined by a guess.", nullptr, dropped);
    if (notes) *notes = said;
    return jp::office::rebuildCompoundFile(found);
}

} // namespace jp::deck
