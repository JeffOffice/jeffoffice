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
    QVector<DirEntry> entries;

    bool open()
    {
        const QByteArrayView h(m_file);
        if (m_file.size() < 512 || !m_file.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1"))) return false;
        const int shift = rd16(h, 30), miniShift = rd16(h, 32);
        if ((shift != 9 && shift != 12) || miniShift < 1 || miniShift > shift) return false;
        m_ss = qsizetype(1) << shift;
        m_mss = qsizetype(1) << miniShift;
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
        seen.clear();
        for (qint64 i = 0; i < most; ++i) {
            const quint32 id = listed[qsizetype(i)];
            if (qint64(id) >= m_sectors || seen.contains(id)) continue;
            seen.insert(id);
            const QByteArrayView s = sector(id);
            if (s.size() < m_ss || allZero(s)) continue;
            for (qint64 k = 0; k < perSector; ++k) m_fat[qsizetype(i * perSector + k)] = rd32(s, qsizetype(k * 4));
        }
        readDirectory(dirStart);
        readMini(miniFatStart, numMiniFat);
        return true;
    }

    // The bytes of a stream, `size` of them from `start`: through the tables where they are known, else as the next sector after the last. `contiguous` takes no table as known.
    QByteArray stream(const DirEntry &e, bool contiguous)
    {
        if (e.size == 0 || e.size > quint64(m_lim.maxStreamBytes)) return QByteArray();
        QByteArray out;
        if (e.size < m_cutoff) {
            if (!m_haveMini) return QByteArray();
            const qint64 count = qint64((e.size + quint64(m_mss) - 1) / quint64(m_mss));
            const QVector<quint32> c = chain(e.start, count, contiguous ? QVector<quint32>() : m_miniFat, m_mini.size() / m_mss, false);
            for (quint32 m : c) out += m_mini.mid(qsizetype(m) * m_mss, m_mss);
        } else {
            const qint64 count = qint64((e.size + quint64(m_ss) - 1) / quint64(m_ss));
            const QVector<quint32> c = chain(e.start, count, contiguous ? QVector<quint32>() : m_fat, m_sectors, true);
            out.reserve(qsizetype(c.size()) * m_ss);
            for (quint32 n : c) out.append(sector(n).toByteArray());
        }
        return out.left(qsizetype(e.size));
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
            if (qint64(next) < limit && !seen.contains(next)) {
                cur = next;
            } else {
                guessed = true;
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
                cur = after;
            } else {
                break;
            }
        }
        for (quint32 n : sectors) {
            const QByteArrayView s = sector(n);
            for (qsizetype o = 0; o + 128 <= s.size(); o += 128) {
                const QByteArrayView e = s.mid(o, 128);
                if (!plausibleEntry(e)) continue;
                DirEntry d;
                const int nameLen = rd16(e, 64);
                d.name = QString::fromUtf16(reinterpret_cast<const char16_t *>(e.constData()), nameLen / 2 - 1);
                d.type = quint8(e[66]);
                d.start = rd32(e, 116);
                d.size = m_ss == 512 ? rd32(e, 120) : rd64(e, 120);
                if (d.type == 5 && m_root.type != 5) m_root = d;
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
        for (quint32 n : chain(m_root.start, count, m_fat, m_sectors, true)) m_mini.append(sector(n).toByteArray());
        m_mini = m_mini.left(qsizetype(m_root.size));
        if (m_mini.isEmpty()) return;
        // (the table of the mini stream: one sector of it is left unknown if it can't be read)
        const qint64 mcount = std::min<qint64>(numMiniFat, qint64(m_mini.size() / m_mss / (m_ss / 4)) + 2);
        for (quint32 n : chain(miniFatStart, mcount, m_fat, m_sectors, true)) {
            const QByteArrayView s = sector(n);
            const qsizetype before = m_miniFat.size();
            m_miniFat.resize(before + qsizetype(m_ss / 4));
            for (qsizetype k = 0; k < m_ss / 4; ++k) m_miniFat[before + k] = s.size() < m_ss || allZero(s) ? kFree : rd32(s, k * 4);
        }
        m_haveMini = true;
    }

    const QByteArray &m_file;
    const CfbSalvageLimits &m_lim;
    qsizetype m_ss = 512, m_mss = 64;
    quint32 m_cutoff = 4096;
    qint64 m_sectors = 0;
    QVector<quint32> m_fat, m_miniFat;
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
    for (const CfbWish &w : wishes) {
        QByteArray found;
        bool fromDirectory = false;
        auto accept = [&](const QByteArray &data) { return !data.isEmpty() && data.size() <= limits.maxStreamBytes && (!w.validBytes || w.validBytes(QByteArrayView(data), out->streams) > 0); };
        if (opened)
            for (const DirEntry &e : std::as_const(tolerant.entries)) {
                if (e.type != 2 || !nameIs(w, e.name)) continue;
                for (bool contiguous : {false, true}) {
                    const QByteArray data = tolerant.stream(e, contiguous);
                    if (!accept(data)) continue;
                    found = data;
                    fromDirectory = true;
                    break;
                }
                if (!found.isEmpty()) break;
            }
        if (found.isEmpty()) {
            if (!soundTried) {
                soundTried = true;
                soundRead = jp::cfb::read(file, &sound);
            }
            if (soundRead)
                for (const QString &path : sound.streamPaths()) {
                    if (path.contains(QLatin1Char('/')) || !nameIs(w, path)) continue;
                    const QByteArray data = sound.stream(path);
                    if (!accept(data)) continue;
                    found = data;
                    break;
                }
        }
        CfbSalvage::How how = fromDirectory ? CfbSalvage::How::Directory : CfbSalvage::How::Whole;
        if (found.isEmpty() && w.validBytes && w.looksLike) {
            // 3. a scan: every place the stream could start, as far as the caller's own check says it is
            int checks = 0;
            const QByteArrayView all(file);
            for (qsizetype at = 0; at + 8 <= all.size() && checks < limits.maxChecks; at += kScanStep) {
                if (!w.looksLike(all.mid(at, qsizetype(std::min<qint64>(all.size() - at, limits.maxStreamBytes))), out->streams)) continue;
                ++checks;
                const QByteArrayView view = all.mid(at, qsizetype(std::min<qint64>(all.size() - at, limits.maxStreamBytes)));
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
            continue;
        }
        total += found.size();
        if (total > limits.maxTotalBytes) {
            if (error) *error = QCoreApplication::translate("jp::office::CfbSalvage", "The file holds more than can be repaired.");
            return false;
        }
        out->streams.insert(w.name, found);
        out->how.insert(w.name, how);
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
        out->cutShort = tolerant.cutShort;
        out->tablesGuessed = tolerant.guessed;
        out->directoryRead = tolerant.directoryRead;
        for (const DirEntry &e : std::as_const(tolerant.entries))
            if (e.type == 2) out->directoryNames << e.name;
    }
    // (a stream read through a container that needed no guessing is as it was written)
    if (opened && !tolerant.guessed && !tolerant.cutShort)
        for (auto it = out->how.begin(); it != out->how.end(); ++it)
            if (it.value() == CfbSalvage::How::Directory) it.value() = CfbSalvage::How::Whole;
    if (out->directoryNames.isEmpty() && soundRead) out->directoryNames = sound.streamPaths();
    bool byDirectory = false, byScan = false;
    for (auto it = out->how.cbegin(); it != out->how.cend(); ++it) {
        byDirectory |= it.value() == CfbSalvage::How::Directory;
        byScan |= it.value() == CfbSalvage::How::Scan;
    }
    if (byDirectory) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "The container of the file was damaged, so its parts were followed from its directory, as far as the file goes.");
    if (byScan) out->notes << QCoreApplication::translate("jp::office::CfbSalvage", "The directory of the file was damaged or gone, so its parts were found by their own records.");
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
