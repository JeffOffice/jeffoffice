#include "office/cfbsalvage.h"

#include "io/cfb.h"

#include <QCoreApplication>
#include <QSet>
#include <QtEndian>

#include <algorithm>

namespace jp::office {

namespace {

constexpr quint32 kFree = 0xFFFFFFFF, kEnd = 0xFFFFFFFE;
// A scan looks at every 64th byte of the file: a stream kept in a container's mini stream starts on one of those, not on a sector.
constexpr qsizetype kScanStep = 64;

quint16 rd16(QByteArrayView b, qsizetype o) { return o >= 0 && o + 2 <= b.size() ? qFromLittleEndian<quint16>(b.constData() + o) : 0; }
quint32 rd32(QByteArrayView b, qsizetype o) { return o >= 0 && o + 4 <= b.size() ? qFromLittleEndian<quint32>(b.constData() + o) : 0; }
quint64 rd64(QByteArrayView b, qsizetype o) { return o >= 0 && o + 8 <= b.size() ? qFromLittleEndian<quint64>(b.constData() + o) : 0; }

bool allZero(QByteArrayView b)
{
    for (char c : b)
        if (c) return false;
    return true;
}

struct DirEntry {
    QString name;
    int type = 0;
    quint32 start = 0;
    quint64 size = 0;
    int id = 0;   // its place in the directory
};

// A 128-byte block that could be a directory entry: a name of whole UTF-16 letters ended by a zero, and a type that exists.
bool plausibleEntry(QByteArrayView e)
{
    const int nameLen = rd16(e, 64);
    const int type = quint8(e.size() > 66 ? e[66] : 0);
    return nameLen >= 2 && nameLen <= 64 && nameLen % 2 == 0 && (type == 1 || type == 2 || type == 5) && rd16(e, nameLen - 2) == 0;
}

// A sector that could be one of a directory's: every block of it is an entry or empty, and one is an entry.
bool looksLikeDirectory(QByteArrayView s)
{
    bool any = false;
    for (qsizetype o = 0; o + 128 <= s.size(); o += 128) {
        const QByteArrayView e = s.mid(o, 128);
        if (allZero(e)) continue;
        if (!plausibleEntry(e)) return false;
        any = true;
    }
    return any;
}

// The container read as far as it goes.
class Tolerant {
public:
    Tolerant(const QByteArray &file, const CfbSalvageLimits &limits) : m_file(file), m_lim(limits) {}

    bool guessed = false, cutShort = false;
    bool directoryRead = false;
    bool miniUnread = false;   // the header names a size of mini sector the format does not have
    QVector<DirEntry> entries;

    bool open()
    {
        const QByteArrayView h(m_file);
        if (m_file.size() < 512 || !m_file.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1"))) return false;
        const int shift = rd16(h, 30), miniShift = rd16(h, 32);
        if (shift != 9 && shift != 12) return false;
        m_ss = qsizetype(1) << shift;
        m_mss = 64;   // (the format's only size of a mini sector: any other makes the mini stream unreadable)
        const bool miniOk = miniShift == 6;
        m_cutoff = rd32(h, 56);
        m_sectors = m_file.size() > m_ss ? (m_file.size() - m_ss + m_ss - 1) / m_ss : 0;
        const quint32 numFat = rd32(h, 44), dirStart = rd32(h, 48), miniFatStart = rd32(h, 60), numMiniFat = rd32(h, 64), difat = rd32(h, 68), numDifat = rd32(h, 72);
        // the FAT: its sectors are listed in the header and then in a chain of DIFAT sectors, each once; one that is gone (cut off or zeroed) leaves its entries unknown
        QVector<quint32> listed;
        for (int i = 0; i < 109; ++i) listed << rd32(h, 76 + i * 4);
        QSet<quint32> seen;
        quint32 d = difat;
        for (quint32 k = 0; k < numDifat && int(k) < m_lim.maxDifatSectors && qint64(d) < m_sectors && !seen.contains(d); ++k) {
            seen.insert(d);
            const QByteArrayView s = sector(d);
            for (qsizetype i = 0; i + 4 < m_ss && i + 4 <= s.size(); i += 4) listed << rd32(s, i);
            d = rd32(s, m_ss - 4);
        }
        const qint64 perSector = m_ss / 4;
        const qint64 most = std::min<qint64>({qint64(numFat), qint64(listed.size()), m_sectors / perSector + 2});
        m_fat.fill(kFree, qsizetype(most * perSector));
        m_fatKnown.fill(0, m_fat.size());
        for (quint32 k : seen) m_structural << k;   // (the sectors of the DIFAT chain)
        seen.clear();
        for (qint64 i = 0; i < most; ++i) {
            const quint32 id = listed[qsizetype(i)];
            if (qint64(id) >= m_sectors || seen.contains(id)) continue;
            seen.insert(id);
            const QByteArrayView s = sector(id);
            if (s.size() < m_ss || allZero(s)) continue;
            m_structural << id;
            for (qint64 k = 0; k < perSector; ++k) {
                m_fat[qsizetype(i * perSector + k)] = rd32(s, qsizetype(k * 4));
                m_fatKnown[qsizetype(i * perSector + k)] = 1;
            }
        }
        readDirectory(dirStart);
        miniUnread = !miniOk;
        if (miniOk) readMini(miniFatStart, numMiniFat);
        buildOwners();
        return true;
    }

    // How a stream is read, in the order the caller tries them: through the tables as far as they go (to the directory's size), through the tables on past the directory's size to
    // their own end, and through the tables with each BROKEN link (one that is free, loops, or leads outside the file) replaced by the sector that follows it in the file; the last also
    // takes sectors the tables mark free, for want of anything else. A chain that ends with the tables' end marker ends there in every mode: nothing is read beyond it.
    enum class Mode { Tables, PastSize, Guess, LastResort };

    // True if the tables lead from the last sector the directory's size needs to another one (the stream holds more than the directory says).
    bool runsPastItsSize(const DirEntry &e)
    {
        if (e.size == 0 || e.size >= quint64(m_lim.maxStreamBytes) || e.size < m_cutoff) return false;
        const qint64 count = qint64((e.size + quint64(m_ss) - 1) / quint64(m_ss));
        quint32 cur = e.start;
        QSet<quint32> seen;
        for (qint64 i = 0; i < count; ++i) {
            if (qint64(cur) >= m_sectors || seen.contains(cur) || cur >= quint32(m_fat.size())) return false;
            seen.insert(cur);
            if (i + 1 < count) cur = m_fat[qsizetype(cur)];
        }
        const quint32 next = m_fat[qsizetype(cur)];
        return qint64(next) < m_sectors && !seen.contains(next);
    }

    // One reading of a stream, and what was done to make it.
    struct Reading {
        QByteArray bytes;
        QVector<qint64> joins;   // where a sector was joined to the one before it by guess (offsets in bytes)
        bool cutShort = false;   // the file ended inside the stream
        bool chainBroke = false; // the tables ended the stream before its size
        bool guessed = false;
        bool pastSize = false;   // the tables hold more than the directory's size
        bool usedFree = false;
        // as the file gives it: the tables and the directory agree and nothing was guessed
        bool clean() const { return !cutShort && !chainBroke && !guessed && !pastSize; }
    };

    Reading read(const DirEntry &e, Mode mode)
    {
        Reading r;
        if (e.size == 0 || e.size > quint64(m_lim.maxStreamBytes)) return r;
        const bool mini = e.size < m_cutoff;
        if (mini && !m_haveMini) return r;
        if (mode == Mode::PastSize && (mini || !runsPastItsSize(e))) return r;
        const QVector<quint32> &table = mini ? m_miniFat : m_fat;
        const QVector<char> &known = mini ? m_miniKnown : m_fatKnown;
        const QVector<qint32> &owner = mini ? m_miniOwner : m_owner;
        const qint64 limit = mini ? qint64(m_mini.size() / m_mss) : m_sectors;
        const qsizetype unit = mini ? m_mss : m_ss;
        const qint64 count = mode == Mode::PastSize ? std::min<qint64>(limit, m_lim.maxStreamBytes / unit) : qint64((e.size + quint64(unit) - 1) / quint64(unit));
        const bool allowFree = mode == Mode::LastResort;
        r.pastSize = mode == Mode::Tables && !mini && runsPastItsSize(e);
        // a sector a guess may take: in the file, not part of the container's own tables, not another stream's, and not marked free (unless nothing else is left)
        auto usable = [&](quint32 n) {
            if (qint64(n) >= limit) return false;
            if (n < quint32(owner.size()) && owner[qsizetype(n)] != -1 && owner[qsizetype(n)] != e.id) return false;
            if (n < quint32(known.size()) && known[qsizetype(n)]) {
                const quint32 v = table[qsizetype(n)];
                if (v == 0xFFFFFFFD || v == 0xFFFFFFFC) return false;
                if (v == kFree && !allowFree) return false;
            }
            return true;
        };
        QSet<quint32> seen;
        quint32 cur = e.start;
        qint64 pieces = 0;
        const qint64 mostPieces = std::min<qint64>(count, limit + 1);
        while (pieces < mostPieces) {
            if (qint64(cur) >= limit) {
                r.chainBroke = true;
                break;
            }
            if (seen.contains(cur)) {
                r.chainBroke = true;
                break;
            }
            seen.insert(cur);
            const QByteArrayView data = mini ? QByteArrayView(m_mini).mid(qsizetype(cur) * m_mss, m_mss) : sector(cur);
            if (data.size() < unit) r.cutShort = true;
            if (allowFree && cur < quint32(known.size()) && known[qsizetype(cur)] && table[qsizetype(cur)] == kFree) r.usedFree = true;
            r.bytes.append(data.toByteArray());
            ++pieces;
            if (pieces == count) break;
            const quint32 next = cur < quint32(table.size()) ? table[qsizetype(cur)] : kFree;
            const bool tableOk = qint64(next) < limit && !seen.contains(next);
            const bool wholeTables = mode == Mode::Tables || mode == Mode::PastSize;
            // (a table that was read and says the chain ends here: the stream ends here, whatever size the directory gives)
            if (!wholeTables && next == kEnd && cur < quint32(known.size()) && known[qsizetype(cur)]) {
                r.chainBroke = true;
                break;
            }
            if (wholeTables) {
                if (!tableOk) {
                    if (qint64(next) >= limit && next < 0xFFFFFFFC) r.cutShort = true;   // (a link to a place the file does not reach)
                    else if (mode == Mode::Tables) r.chainBroke = true;
                    break;
                }
                cur = next;
                continue;
            }
            const quint32 after = cur + 1;
            if (tableOk) {
                cur = next;
                continue;
            }
            if (!usable(after) || seen.contains(after)) {
                if (qint64(after) >= limit) r.cutShort = true;
                else r.chainBroke = true;
                break;
            }
            r.guessed = true;
            r.joins << qint64(pieces) * unit;
            cur = after;
        }
        if (mode != Mode::PastSize && quint64(r.bytes.size()) < e.size && pieces < count) r.chainBroke = r.chainBroke || !r.cutShort;
        if (mode != Mode::PastSize) r.bytes = r.bytes.left(qsizetype(e.size));
        if (mini && m_miniGuessed) {
            // (the container's mini stream was put together by guess: every mini sector may be joined wrongly)
            r.guessed = true;
            for (qint64 o = unit; o < r.bytes.size(); o += unit) r.joins << o;
        }
        while (!r.joins.isEmpty() && r.joins.last() >= r.bytes.size()) r.joins.removeLast();
        return r;
    }

private:
    QByteArrayView sector(quint32 n) const
    {
        const qint64 off = (qint64(n) + 1) * m_ss;
        if (off >= m_file.size()) return QByteArrayView();
        return QByteArrayView(m_file).mid(qsizetype(off), qsizetype(std::min<qint64>(m_ss, m_file.size() - off)));
    }

    // The sectors of a stream of `count` (the sectors it needs) from `start`, each at most once; where the table does not say the next (or says what can't be, or the end too soon, or
    // a sector already followed), the next is the one after.
    QVector<quint32> chain(quint32 start, qint64 count, const QVector<quint32> &table, qint64 limit, bool regular)
    {
        QVector<quint32> out;
        QSet<quint32> seen;
        quint32 cur = start;
        for (qint64 i = 0; i < count; ++i) {
            if (qint64(cur) >= limit) {
                if (regular) cutShort = true;
                break;
            }
            if (seen.contains(cur)) break;
            seen.insert(cur);
            out << cur;
            if (i + 1 == count) break;
            const quint32 next = cur < quint32(table.size()) ? table[qsizetype(cur)] : kFree;
            if (next == kEnd) break;   // (the tables say the chain ends here)
            if (qint64(next) < limit && !seen.contains(next)) {
                cur = next;
            } else {
                guessed = true;
                ++m_guesses;
                cur = cur + 1;
            }
        }
        return out;
    }

    void readDirectory(quint32 start)
    {
        QVector<quint32> sectors;
        QSet<quint32> seen;
        quint32 cur = start;
        while (qint64(cur) < m_sectors && sectors.size() < m_lim.maxDirSectors && !seen.contains(cur)) {
            seen.insert(cur);
            sectors << cur;
            const quint32 next = cur < quint32(m_fat.size()) ? m_fat[qsizetype(cur)] : kFree;
            if (next == kEnd) break;
            if (qint64(next) < m_sectors && !seen.contains(next)) {
                cur = next;
                continue;
            }
            // (the table does not say: the next sector is taken as the directory's while it looks like one)
            const quint32 after = cur + 1;
            if (qint64(after) < m_sectors && !seen.contains(after) && looksLikeDirectory(sector(after))) {
                guessed = true;
                ++m_guesses;
                cur = after;
            } else {
                break;
            }
        }
        for (quint32 n : sectors) {
            m_structural << n;
            const QByteArrayView s = sector(n);
            for (qsizetype o = 0; o + 128 <= s.size(); o += 128) {
                const QByteArrayView e = s.mid(o, 128);
                if (!plausibleEntry(e)) continue;
                DirEntry d;
                const int nameLen = rd16(e, 64);
                for (int c = 0; c < nameLen / 2 - 1; ++c) d.name += QChar(rd16(e, c * 2));
                d.type = quint8(e[66]);
                d.start = rd32(e, 116);
                d.size = m_ss == 512 ? rd32(e, 120) : rd64(e, 120);
                if (d.type == 5 && m_root.type != 5) m_root = d;
                d.id = int(entries.size());
                entries << d;
                directoryRead = true;
            }
        }
    }

    void readMini(quint32 miniFatStart, quint32 numMiniFat)
    {
        if (m_root.type != 5 || m_root.size == 0 || m_root.size > quint64(m_lim.maxStreamBytes)) return;
        // (the root's chain is the mini stream)
        m_mini.clear();
        const qint64 count = qint64((m_root.size + quint64(m_ss) - 1) / quint64(m_ss));
        const int guessesBefore = m_guesses;
        for (quint32 n : chain(m_root.start, count, m_fat, m_sectors, true)) m_mini.append(sector(n).toByteArray());
        m_mini = m_mini.left(qsizetype(m_root.size));
        if (m_mini.isEmpty()) return;
        // (the table of the mini stream: one sector of it is left unknown if it can't be read)
        const qint64 mcount = std::min<qint64>(numMiniFat, qint64(m_mini.size() / m_mss / (m_ss / 4)) + 2);
        for (quint32 n : chain(miniFatStart, mcount, m_fat, m_sectors, true)) {
            const QByteArrayView s = sector(n);
            const qsizetype before = m_miniFat.size();
            m_miniFat.resize(before + qsizetype(m_ss / 4));
            const bool read = s.size() >= m_ss && !allZero(s);
            m_miniFatRead << read;
            for (qsizetype k = 0; k < m_ss / 4; ++k) m_miniFat[before + k] = read ? rd32(s, k * 4) : kFree;
        }
        m_miniGuessed = m_guesses != guessesBefore;
        m_haveMini = true;
    }

    // Who holds each sector: nobody (-1), the container's own tables (-2), or the directory entry whose chain reaches it (by the tables as they read).
    void buildOwners()
    {
        m_owner.fill(-1, qsizetype(m_sectors));
        for (quint32 n : std::as_const(m_structural))
            if (qint64(n) < m_sectors) m_owner[qsizetype(n)] = -2;
        const qint64 minis = m_haveMini ? m_mini.size() / m_mss : 0;
        m_miniOwner.fill(-1, qsizetype(minis));
        m_miniKnown.fill(0, m_miniFat.size());
        for (qsizetype i = 0; i < m_miniFat.size(); ++i) m_miniKnown[i] = m_miniFatRead.value(i / qsizetype(m_ss / 4), false);
        for (const DirEntry &e : std::as_const(entries)) {
            if (e.size == 0 || (e.type != 2 && e.type != 5)) continue;
            const bool mini = e.type == 2 && e.size < m_cutoff;
            if (mini && !m_haveMini) continue;
            QVector<qint32> &owner = mini ? m_miniOwner : m_owner;
            const QVector<quint32> &table = mini ? m_miniFat : m_fat;
            const qint64 limit = mini ? minis : m_sectors;
            quint32 cur = e.start;
            for (qint64 steps = 0; steps <= limit && qint64(cur) < limit && owner[qsizetype(cur)] == -1; ++steps) {
                owner[qsizetype(cur)] = e.id;
                cur = cur < quint32(table.size()) ? table[qsizetype(cur)] : kFree;
            }
        }
    }

    const QByteArray &m_file;
    const CfbSalvageLimits &m_lim;
    qsizetype m_ss = 512, m_mss = 64;
    quint32 m_cutoff = 4096;
    qint64 m_sectors = 0;
    QVector<quint32> m_fat, m_miniFat, m_structural;
    QVector<char> m_fatKnown, m_miniKnown;
    QVector<qint32> m_owner, m_miniOwner;
    QVector<bool> m_miniFatRead;
    bool m_miniGuessed = false;
    int m_guesses = 0;
    DirEntry m_root;
    QByteArray m_mini;
    bool m_haveMini = false;
};

bool nameIs(const CfbWish &w, const QString &name)
{
    return w.nameMatches ? w.nameMatches(name) : name.compare(w.name, Qt::CaseInsensitive) == 0;
}

} // namespace

bool salvageCompoundFile(const QByteArray &file, const QVector<CfbWish> &wishes, CfbSalvage *out, QString *error, const CfbSalvageLimits &limits)
{
    *out = CfbSalvage();
    if (file.size() > limits.maxFileBytes) {
        if (error) *error = QCoreApplication::translate("jp::office::CfbSalvage", "The file is too large to repair.");
        return false;
    }
    if (wishes.size() > limits.maxWishes) {
        if (error) *error = QCoreApplication::translate("jp::office::CfbSalvage", "Too many parts of the file were asked for.");
        return false;
    }
    // 1. the container read as far as it goes: a sound one gives its streams as they are, and a damaged one what its directory and tables lead to
    Tolerant tolerant(file, limits);
    const bool opened = tolerant.open();
    // 2. the container as it is, for what its directory does not lead to (a table of sectors that reads, and a directory that does not)
    jp::cfb::File sound;
    bool soundTried = false, soundRead = false;
    QMap<QString, qint64> scanStart;
    qint64 total = 0;
    QStringList missing;
    // what the caller's checks may look at in all, and what the readings may make: a few times the file, so no number of candidates makes the work more
    const qint64 budget = 8 * qint64(file.size()) + (1 << 20), builtBudget = 16 * qint64(file.size()) + (1 << 20);
    qint64 work = budget, built = builtBudget;
    bool anyCut = false, anyGuessed = false, anyUnverified = false, anyFree = false, anyLaterEntry = false;
    for (const CfbWish &w : wishes) {
        QByteArray found;
        QVector<qint64> joins;
        bool fromDirectory = false;
        auto accept = [&](const QByteArray &data) {
            if (data.isEmpty() || data.size() > limits.maxStreamBytes || data.size() > work) return false;
            work -= data.size();
            return !w.validBytes || w.validBytes(QByteArrayView(data), out->streams) > 0;
        };
        // The entries of the directory named for this stream, the one named exactly first, at most maxEntryTries of them. The stream is read from ONE of them (the first that gives a
        // reading that ends well, else the first that gives any the caller accepts), never from several put together: see the order of readings in REPAIR.md and the header.
        auto fromTolerant = [&] {
            if (!opened) return;
            QVector<const DirEntry *> named;
            for (int exact = 1; exact >= 0; --exact)
                for (const DirEntry &e : std::as_const(tolerant.entries))
                    if (e.type == 2 && (e.name.compare(w.name, Qt::CaseInsensitive) == 0) == bool(exact) && nameIs(w, e.name)) named << &e;
            if (named.size() > limits.maxEntryTries) named.resize(limits.maxEntryTries);
            auto choose = [&](const Tolerant::Reading &r, Tolerant::Mode mode, qint64 keep, bool verified) {
                found = r.bytes.left(qsizetype(keep));
                joins.clear();
                for (qint64 j : r.joins)
                    if (j < keep) joins << j;
                fromDirectory = true;
                anyCut |= r.cutShort;
                anyGuessed |= !joins.isEmpty() || (r.guessed && r.joins.isEmpty());
                anyFree |= r.usedFree;
                anyUnverified |= !verified && !r.cutShort;
                if (!(mode == Tolerant::Mode::Tables && r.clean())) out->repaired = true;
                else fromDirectory = false;
            };
            struct Fallback {
                Tolerant::Reading reading;
                Tolerant::Mode mode = Tolerant::Mode::Tables;
                qint64 prefix = 0;
                bool set = false;
                int entry = 0;
            } fallback;
            int usedEntry = -1;
            for (int tier = 0; tier < 2 && found.isEmpty() && !fallback.set; ++tier) {
                if (tier == 1 && !w.endsWell) break;
                const QVector<Tolerant::Mode> modes = tier == 0 ? QVector<Tolerant::Mode>{Tolerant::Mode::Tables, Tolerant::Mode::PastSize, Tolerant::Mode::Guess}
                                                                : QVector<Tolerant::Mode>{Tolerant::Mode::LastResort};
                for (int i = 0; i < named.size() && found.isEmpty(); ++i) {
                    Fallback mine;
                    QVector<QByteArray> tried;
                    for (Tolerant::Mode mode : modes) {
                        if (built <= 0) break;
                        Tolerant::Reading r = tolerant.read(*named[i], mode);
                        if (r.bytes.isEmpty()) continue;
                        built -= r.bytes.size();
                        if (tried.contains(r.bytes)) continue;
                        tried << r.bytes;
                        if (!accept(r.bytes)) continue;
                        if (!w.endsWell) {
                            // (nothing to check it by: the tables and the directory as far as they agree)
                            if (mode == Tolerant::Mode::Tables) {
                                choose(r, mode, r.bytes.size(), true);
                                if (!r.clean() || quint64(r.bytes.size()) < named[i]->size) out->partial << w.name;
                            }
                            if (!found.isEmpty()) break;
                            continue;
                        }
                        if (mode == Tolerant::Mode::Tables && r.clean()) {
                            choose(r, mode, r.bytes.size(), true);
                            break;
                        }
                        const CfbWish::End end = w.endsWell(QByteArrayView(r.bytes), out->streams);
                        if (end.whole >= 0) {
                            choose(r, mode, std::min<qint64>(end.whole, r.bytes.size()), true);
                            break;
                        }
                        const qint64 prefix = std::clamp<qint64>(end.prefix, 0, r.bytes.size());
                        if (prefix > mine.prefix) {
                            mine.reading = r;
                            mine.mode = mode;
                            mine.prefix = prefix;
                            mine.set = true;
                            mine.entry = i;
                        }
                    }
                    if (!found.isEmpty()) usedEntry = i;
                    if (found.isEmpty() && mine.set && !fallback.set) fallback = mine;
                }
            }
            // no reading ends well: what can be checked of the first entry's best (and no more)
            if (found.isEmpty() && fallback.set) {
                const QByteArray head = fallback.reading.bytes.left(qsizetype(fallback.prefix));
                usedEntry = fallback.entry;
                if (head.size() < fallback.reading.bytes.size() && !accept(head)) {
                    choose(fallback.reading, fallback.mode, fallback.reading.bytes.size(), false);
                    anyLaterEntry |= usedEntry > 0;
                    return;
                }
                choose(fallback.reading, fallback.mode, fallback.prefix, false);
            }
            anyLaterEntry |= usedEntry > 0;   // (a later entry of the same name was used in place of the first)
        };
        auto fromSound = [&] {
            if (!soundTried) {
                soundTried = true;
                soundRead = jp::cfb::read(file, &sound);
            }
            if (!soundRead) return;
            QStringList named;
            for (int exact = 1; exact >= 0; --exact)
                for (const QString &path : sound.streamPaths())
                    if (!path.contains(QLatin1Char('/')) && (path.compare(w.name, Qt::CaseInsensitive) == 0) == bool(exact) && nameIs(w, path)) named << path;
            for (int i = 0; i < named.size() && i < limits.maxEntryTries && found.isEmpty(); ++i) {
                const QByteArray data = sound.stream(named[i]);
                if (!accept(data)) continue;
                found = data;
                anyLaterEntry |= i > 0;
            }
        };
        fromTolerant();
        if (found.isEmpty()) {
            fromSound();
            fromDirectory = false;
        }
        CfbSalvage::How how = fromDirectory ? CfbSalvage::How::Directory : CfbSalvage::How::Whole;
        if (found.isEmpty() && w.validBytes && w.looksLike) {
            // 3. a scan: every place the stream could start, as far as the caller's own check says it is
            int checks = 0;
            const QByteArrayView all(file);
            const qsizetype step = w.anyOffset ? 1 : kScanStep;
            for (qsizetype at = 0; at + 8 <= all.size() && checks < limits.maxChecks; at += step) {
                const QByteArrayView view = all.mid(at, qsizetype(std::min<qint64>(all.size() - at, limits.maxStreamBytes)));
                if (!w.looksLike(view, out->streams)) continue;
                if (view.size() > work) break;
                ++checks;
                work -= view.size();
                const qint64 ok = w.validBytes(view, out->streams);
                if (ok <= 0) continue;
                found = view.left(qsizetype(std::min<qint64>(ok, view.size()))).toByteArray();
                how = CfbSalvage::How::Scan;
                scanStart.insert(w.name, at);
                break;
            }
        }
        if (found.isEmpty()) {
            if (w.required) missing << w.name;
            if (opened)
                for (const DirEntry &e : std::as_const(tolerant.entries))
                    if (e.type == 2 && e.size > 0 && nameIs(w, e.name)) {   // (a stream of no bytes has nothing to lose)
                        out->lost << w.name;
                        break;
                    }
            continue;
        }
        total += found.size();
        if (total > limits.maxTotalBytes) {
            if (error) *error = QCoreApplication::translate("jp::office::CfbSalvage", "The file holds more than can be repaired.");
            return false;
        }
        out->streams.insert(w.name, found);
        out->how.insert(w.name, how);
        if (!joins.isEmpty()) out->joins.insert(w.name, joins);
    }
    // a stream found by a scan runs to where the next stream found that way starts
    if (scanStart.size() > 1)
        for (auto it = scanStart.cbegin(); it != scanStart.cend(); ++it) {
            qint64 next = file.size();
            for (auto other = scanStart.cbegin(); other != scanStart.cend(); ++other)
                if (other.value() > it.value()) next = std::min(next, other.value());
            QByteArray &s = out->streams[it.key()];
            if (s.size() > next - it.value()) s.truncate(qsizetype(next - it.value()));
        }
    if (opened) {
        out->cutShort = tolerant.cutShort || anyCut;
        out->tablesGuessed = tolerant.guessed || anyGuessed;
        out->directoryRead = tolerant.directoryRead;
        out->miniUnread = tolerant.miniUnread;
        for (const DirEntry &e : std::as_const(tolerant.entries))
            if (e.type == 2) out->directoryNames << e.name;
    }
    if (out->directoryNames.isEmpty() && soundRead) out->directoryNames = sound.streamPaths();
    bool byDirectory = false, byScan = false;
    for (auto it = out->how.cbegin(); it != out->how.cend(); ++it) {
        byDirectory |= it.value() == CfbSalvage::How::Directory;
        byScan |= it.value() == CfbSalvage::How::Scan;
    }
    if (byDirectory) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "The file was damaged, so its parts were found by following the lists inside it, as far as they could be read.");
    if (byScan) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "The list of the parts of the file was damaged or gone, so each part was found by searching the file for it.");
    if (out->tablesGuessed && (byDirectory || byScan)) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "Where the lists that say how the pieces of the file fit together were damaged, each piece was taken to follow the one before it in the file.");
    if (anyLaterEntry) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "Two parts of the file had the same name. The first could not be used, so a later one was used in its place.");
    if (out->repaired) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "Part of the file was stored in a confusing way, so it was read from the file's own lists of where its pieces are.");
    out->unverified = anyUnverified;
    out->usedFree = anyFree;
    out->bytesBuilt = builtBudget - built;
    out->bytesChecked = budget - work;
    if (anyUnverified) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "Some of the file could not be checked to see that it was complete, so only the part that could be checked was kept.");
    if (anyFree) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "Pieces of the file that its lists mark as unused were read, because nothing else could be found; they may hold old data.");
    if (out->miniUnread) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "The small parts of the file, which are kept together in one place, could not be read.");
    if (out->cutShort && (byDirectory || byScan)) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "The file was cut short; what came after the cut is lost.");
    if (!missing.isEmpty()) {
        if (error) *error = QCoreApplication::translate("jp::office::CfbSalvage", "No %1 could be found in the file.").arg(missing.join(QStringLiteral(", ")));
        return false;
    }
    return true;
}

QByteArray rebuildCompoundFile(const CfbSalvage &salvage)
{
    jp::cfb::File f;
    jp::cfb::Entry root;
    root.type = jp::cfb::Entry::Root;
    root.name = QStringLiteral("Root Entry");
    f.entries.push_back(root);
    for (auto it = salvage.streams.cbegin(); it != salvage.streams.cend(); ++it) {
        jp::cfb::Entry e;
        e.type = jp::cfb::Entry::Stream;
        e.name = it.key();
        e.data = it.value();
        f.entries[0].children.push_back(int(f.entries.size()));
        f.entries.push_back(e);
    }
    return jp::cfb::write(f);
}

} // namespace jp::office
