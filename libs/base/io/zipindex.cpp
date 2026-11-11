#include "io/zipindex.h"

#include <QCoreApplication>
#include <zlib.h>

#include <QtEndian>
#include <algorithm>
#include <cstring>
#include <vector>

namespace jp {

namespace {

// Values past the end read as 0: a damaged file can't make these read
// outside the data, whatever the caller checked.
quint16 get16(const QByteArray &a, qsizetype at) { return at >= 0 && at + 2 <= a.size() ? qFromLittleEndian<quint16>(a.constData() + at) : 0; }
quint32 get32(const QByteArray &a, qsizetype at) { return at >= 0 && at + 4 <= a.size() ? qFromLittleEndian<quint32>(a.constData() + at) : 0; }
quint64 get64(const QByteArray &a, qsizetype at) { return at >= 0 && at + 8 <= a.size() ? qFromLittleEndian<quint64>(a.constData() + at) : 0; }

} // namespace

bool readZipEnd(const std::function<QByteArray(qint64, qint64)> &readAt, qint64 size, ZipEnd *out)
{
    if (size < 22) return false;
    // The end record is in the last 65,557 bytes; the long form's locator is the 20 bytes before it.
    const qint64 tailAt = std::max<qint64>(0, size - 65557 - 20);
    const QByteArray tail = readAt(tailAt, size - tailAt);
    if (tail.size() != size - tailAt) return false;
    qsizetype eocd = -1;
    for (qsizetype i = tail.size() - 22; i >= 0; --i)
        if (get32(tail, i) == 0x06054b50) {
            eocd = i;
            break;
        }
    if (eocd < 0) return false;
    quint64 count = get16(tail, eocd + 10);
    qint64 dirSize = get32(tail, eocd + 12), dirAt = get32(tail, eocd + 16);
    if (eocd >= 20 && get32(tail, eocd - 20) == 0x07064b50) {
        const quint64 at = get64(tail, eocd - 20 + 8);
        if (at > quint64(size) || at + 56 > quint64(size)) return false;
        const QByteArray rec = readAt(qint64(at), 56);
        if (rec.size() < 56 || get32(rec, 0) != 0x06064b50) return false;
        const quint64 total = get64(rec, 32), cdSize = get64(rec, 40), cdAt = get64(rec, 48);
        // (a short record that holds its value is the one that counts, as for any reader)
        if (count == 0xFFFF) count = total;
        if (dirSize == qint64(0xFFFFFFFFu)) {
            if (cdSize > quint64(size)) return false;
            dirSize = qint64(cdSize);
        }
        if (dirAt == qint64(0xFFFFFFFFu)) {
            if (cdAt > quint64(size)) return false;
            dirAt = qint64(cdAt);
        }
    }
    // (every record is at least 46 bytes: a count past what the file can hold is no count)
    if (count > quint64(size) / 46) return false;
    out->count = count;
    out->dirAt = dirAt;
    out->dirSize = dirSize;
    return true;
}

bool readZipRecord(const QByteArray &dir, qint64 p, ZipRecord *out, qint64 *next)
{
    if (p < 0 || p + 46 > dir.size() || get32(dir, p) != 0x02014b50) return false;
    ZipRecord r;
    r.flags = get16(dir, p + 8);
    r.method = get16(dir, p + 10);
    r.crc = get32(dir, p + 16);
    const quint32 csize = get32(dir, p + 20), usize = get32(dir, p + 24), local = get32(dir, p + 42);
    r.csize = csize;
    r.usize = usize;
    r.local = local;
    const quint16 nlen = get16(dir, p + 28), xlen = get16(dir, p + 30), clen = get16(dir, p + 32);
    // The record's name, extra field and comment must fit too.
    if (p + 46 + qint64(nlen) + xlen + clen > dir.size()) return false;
    r.name = QString::fromUtf8(dir.constData() + p + 46, nlen);
    if (usize == 0xFFFFFFFFu || csize == 0xFFFFFFFFu || local == 0xFFFFFFFFu) {
        // what did not fit is in the extra field of kind 1, in this order, only for those that did not
        bool found = false;
        qint64 q = p + 46 + nlen;
        const qint64 extraEnd = q + xlen;
        while (q + 4 <= extraEnd && !found) {
            const quint16 id = get16(dir, q), len = get16(dir, q + 2);
            if (q + 4 + len > extraEnd) return false;
            if (id == 1) {
                found = true;
                qint64 at = q + 4;
                auto take = [&](qint64 *field) {
                    if (at + 8 > q + 4 + len) return false;
                    const quint64 v = get64(dir, at);
                    if (v > quint64(1) << 62) return false;
                    *field = qint64(v);
                    at += 8;
                    return true;
                };
                if (usize == 0xFFFFFFFFu && !take(&r.usize)) return false;
                if (csize == 0xFFFFFFFFu && !take(&r.csize)) return false;
                if (local == 0xFFFFFFFFu && !take(&r.local)) return false;
            }
            q += 4 + len;
        }
        if (!found) return false;
    }
    *next = p + 46 + qint64(nlen) + xlen + clen;
    *out = r;
    return true;
}

bool readZipDirectory(const QByteArray &zip, QVector<ZipEntry> *out)
{
    out->clear();
    ZipEnd end;
    if (!readZipEnd([&](qint64 at, qint64 n) { return at >= 0 && at <= zip.size() ? zip.mid(qsizetype(at), qsizetype(n)) : QByteArray(); }, zip.size(), &end)) return false;
    qint64 p = end.dirAt;
    out->reserve(qsizetype(end.count));
    // Where each entry's local header begins and its bytes end, to see that no two overlap.
    std::vector<std::pair<qint64, qint64>> spans;
    spans.reserve(size_t(end.count));
    for (quint64 i = 0; i < end.count; ++i) {
        ZipRecord r;
        qint64 next = 0;
        if (!readZipRecord(zip, p, &r, &next)) return false;
        p = next;
        ZipEntry e;
        e.name = r.name;
        e.method = r.method;
        e.flags = r.flags;
        e.crc = r.crc;
        e.csize = r.csize;
        e.usize = r.usize;
        const qint64 local = r.local;
        if (local + 30 > zip.size() || get32(zip, local) != 0x04034b50) return false;
        e.dataAt = local + 30 + get16(zip, local + 26) + get16(zip, local + 28);
        if (e.method != 0 && e.method != 8) return false;
        if (e.dataAt + e.csize > zip.size()) return false;
        if (e.method == 0) e.usize = e.csize;
        spans.emplace_back(local, e.dataAt + e.csize);
        out->append(e);
    }
    std::sort(spans.begin(), spans.end());
    for (size_t i = 1; i < spans.size(); ++i)
        if (spans[i].first < spans[i - 1].second) {
            out->clear();
            return false;
        }
    return true;
}

ZipFault zipFault(const QByteArray &zip)
{
    QVector<ZipEntry> unused;
    if (readZipDirectory(zip, &unused)) return ZipFault::None;
    if (zip.size() < 22) return ZipFault::NoEnd;
    qsizetype eocd = -1;
    for (qsizetype i = zip.size() - 22; i >= std::max<qsizetype>(0, zip.size() - 65557); --i)
        if (get32(zip, i) == 0x06054b50) {
            eocd = i;
            break;
        }
    if (eocd < 0) return ZipFault::NoEnd;
    // the zip64 form: a locator before the record, or a field of the record that holds "look in the zip64 record"
    if ((eocd >= 20 && get32(zip, eocd - 20) == 0x07064b50) || get16(zip, eocd + 10) == 0xFFFF || get32(zip, eocd + 16) == 0xFFFFFFFFu) return ZipFault::Zip64;
    const int count = get16(zip, eocd + 10);
    qint64 p = get32(zip, eocd + 16);
    for (int i = 0; i < count; ++i) {
        if (p + 46 > zip.size() || get32(zip, p) != 0x02014b50) return ZipFault::Damaged;
        const quint16 method = get16(zip, p + 10);
        if (get32(zip, p + 20) == 0xFFFFFFFFu || get32(zip, p + 24) == 0xFFFFFFFFu || get32(zip, p + 42) == 0xFFFFFFFFu) return ZipFault::Zip64;
        if (method != 0 && method != 8) return ZipFault::UnsupportedMethod;
        const quint16 nlen = get16(zip, p + 28), xlen = get16(zip, p + 30), clen = get16(zip, p + 32);
        p += 46 + qint64(nlen) + xlen + clen;
    }
    return ZipFault::Damaged;
}

namespace {

// No deflate stream makes more than about 1,032 bytes of each byte it has; a size past this is no size one of that length could have.
bool couldBeMadeOf(qint64 compressed, qint64 inflated) { return inflated <= compressed * 1100 + 1024; }

quint32 crcOf(quint32 crc, const char *data, qint64 size)
{
    while (size > 0) {
        const uInt n = uInt(std::min<qint64>(size, qint64(1) << 30));
        crc = quint32(::crc32(crc, reinterpret_cast<const Bytef *>(data), n));
        data += n;
        size -= n;
    }
    return crc;
}

} // namespace

bool inflateZipEntry(const QByteArray &zip, const ZipEntry &entry, QByteArray *out, const InflateCounter &budget, ZipFailure *why)
{
    out->clear();
    auto fail = [&](ZipFailure reason) {
        if (why) *why = reason;
        return false;
    };
    if (why) *why = ZipFailure::None;
    if (entry.dataAt < 0 || entry.csize < 0 || entry.dataAt + entry.csize > zip.size()) return fail(ZipFailure::Damaged);
    if (entry.method != 0 && entry.method != 8) return fail(ZipFailure::Damaged);
    const qint64 size = entry.method == 0 ? entry.csize : entry.usize;
    if (size < 0) return fail(ZipFailure::Damaged);
    if (size > kMaxZipEntryBytes) return fail(ZipFailure::TooLarge);
    if (budget && budget->limit >= 0 && budget->inflated.load() + size > budget->limit) return fail(ZipFailure::OverBudget);
    if (entry.method == 0) {
        if (crcOf(quint32(::crc32(0, nullptr, 0)), zip.constData() + entry.dataAt, entry.csize) != entry.crc) return fail(ZipFailure::Damaged);
        *out = zip.mid(qsizetype(entry.dataAt), qsizetype(entry.csize));
        if (budget) budget->inflated.fetch_add(out->size());
        return true;
    }
    if (entry.usize == 0) return ::crc32(0, nullptr, 0) == entry.crc ? true : fail(ZipFailure::Damaged);
    if (!couldBeMadeOf(entry.csize, entry.usize)) return fail(ZipFailure::Damaged);
    z_stream zs{};
    if (inflateInit2(&zs, -15) != Z_OK) return fail(ZipFailure::Damaged);
    QByteArray bytes(qsizetype(entry.usize), Qt::Uninitialized);
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(zip.constData() + entry.dataAt));
    zs.avail_in = uInt(entry.csize);
    zs.next_out = reinterpret_cast<Bytef *>(bytes.data());
    zs.avail_out = uInt(bytes.size());
    const int r = ::inflate(&zs, Z_FINISH);
    const bool good = r == Z_STREAM_END && qint64(zs.total_out) == entry.usize;
    inflateEnd(&zs);
    if (budget) budget->inflated.fetch_add(qint64(zs.total_out));
    if (!good || crcOf(quint32(::crc32(0, nullptr, 0)), bytes.constData(), bytes.size()) != entry.crc) return fail(ZipFailure::Damaged);
    *out = bytes;
    return true;
}

struct ZipEntryDevice::Inflater {
    z_stream zs{};
    bool open = false;
    ~Inflater()
    {
        if (open) inflateEnd(&zs);
    }
};

ZipEntryDevice::ZipEntryDevice(const QByteArray &zip, const ZipEntry &entry, const InflateCounter &budget)
    : m_zip(zip), m_entry(entry), m_budget(budget), m_crc(quint32(::crc32(0, nullptr, 0)))
{
    setOpenMode(QIODevice::ReadOnly);
    if (entry.dataAt < 0 || entry.csize < 0 || entry.dataAt + entry.csize > zip.size() || (entry.method != 0 && entry.method != 8)) {
        fail(Failure::Damaged, QStringLiteral("The entry is not inside the file, or is not stored or deflated."));
        return;
    }
    const qint64 size = entry.method == 0 ? entry.csize : entry.usize;
    if (size < 0) {
        fail(Failure::Damaged, QStringLiteral("The entry's size is wrong."));
    } else if (size > kMaxZipEntryBytes) {
        fail(Failure::TooLarge, QStringLiteral("The entry is larger than any real entry is."));
    } else if (budget && budget->limit >= 0 && budget->inflated.load() + size > budget->limit) {
        fail(Failure::OverBudget, QStringLiteral("The file's entries, read together, would come to more than the most that is read."));
    } else if (entry.method == 8) {
        // A size that no deflate stream of this length could make is damage, found before the stream is begun.
        if (!couldBeMadeOf(entry.csize, entry.usize)) {
            fail(Failure::Damaged, QStringLiteral("The entry says it is larger than its data could make."));
            return;
        }
        m_z = std::make_unique<Inflater>();
        if (inflateInit2(&m_z->zs, -15) != Z_OK) {
            m_z.reset();
            fail(Failure::Damaged, QStringLiteral("The inflater could not start."));
            return;
        }
        m_z->open = true;
        m_z->zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(m_zip.constData() + entry.dataAt));
        m_z->zs.avail_in = uInt(entry.csize);
    }
}

ZipEntryDevice::~ZipEntryDevice() = default;

bool ZipEntryDevice::fail(Failure why, const QString &message)
{
    m_failure = why;
    setErrorString(message);
    return false;
}

qint64 ZipEntryDevice::readData(char *data, qint64 maxlen)
{
    if (failed()) return -1;
    if (m_done || maxlen <= 0) return 0;
    qint64 produced = 0;
    // What has come out so far is given on; the failure is found by the next read.
    auto bad = [&](Failure why, const QString &message) {
        fail(why, message);
        return produced > 0 ? produced : qint64(-1);
    };
    // Counts what was made; the budget may not be passed even by a stream that makes more than it said.
    auto count = [&](qint64 n) {
        return !m_budget || (m_budget->inflated.fetch_add(n) + n <= m_budget->limit || m_budget->limit < 0);
    };
    if (m_entry.method == 0) {
        // Stored: the bytes as they are, in the piece asked for.
        const qint64 n = std::min(maxlen, m_entry.csize - m_out);
        if (n > 0) {
            std::memcpy(data, m_zip.constData() + m_entry.dataAt + m_out, size_t(n));
            m_crc = crcOf(m_crc, data, n);
            m_out += n;
            produced = n;
            if (!count(n)) return bad(Failure::OverBudget, QStringLiteral("The file's entries, read together, came to more than the most that is read."));
        }
        if (m_out >= m_entry.csize) {
            m_done = true;
            if (m_crc != m_entry.crc) return bad(Failure::Damaged, QStringLiteral("The entry is damaged: its checksum is wrong."));
        }
        return produced;
    }
    z_stream &zs = m_z->zs;
    while (produced < maxlen) {
        // Never more than the directory said: a stream that runs on is damage.
        const qint64 room = std::min<qint64>({maxlen - produced, m_entry.usize - m_out, qint64(1) << 30});
        if (room <= 0) {
            // It has given all it was said to hold, so it must end here.
            Bytef probe;
            zs.next_out = &probe;
            zs.avail_out = 1;
            const int r = ::inflate(&zs, Z_NO_FLUSH);
            if (r != Z_STREAM_END || zs.avail_out != 1) return bad(Failure::Damaged, QStringLiteral("The entry inflates to more than its directory says."));
            m_done = true;
            break;
        }
        zs.next_out = reinterpret_cast<Bytef *>(data + produced);
        zs.avail_out = uInt(room);
        const int r = ::inflate(&zs, Z_NO_FLUSH);
        const qint64 got = room - qint64(zs.avail_out);
        m_crc = crcOf(m_crc, data + produced, got);
        produced += got;
        m_out += got;
        if (!count(got)) return bad(Failure::OverBudget, QStringLiteral("The file's entries, read together, came to more than the most that is read."));
        if (r == Z_STREAM_END) {
            m_done = true;
            break;
        }
        // With room to write and no progress, the input has run out before the stream ended.
        if ((r != Z_OK && r != Z_BUF_ERROR) || got == 0)
            return bad(Failure::Damaged, r != Z_OK && r != Z_BUF_ERROR ? QStringLiteral("The entry's data is damaged.") : QStringLiteral("The entry's data ends early."));
    }
    if (m_done && (m_out != m_entry.usize || m_crc != m_entry.crc)) return bad(Failure::Damaged, QStringLiteral("The entry is damaged: its size or checksum is wrong."));
    return produced;
}

} // namespace jp
