#include "io/zipindex.h"

#include <QCoreApplication>
#include <zlib.h>

#include <QtEndian>
#include <algorithm>
#include <cstring>

namespace jp {

namespace {

// Values past the end read as 0: a damaged file can't make these read
// outside the data, whatever the caller checked.
quint16 get16(const QByteArray &a, qsizetype at) { return at >= 0 && at + 2 <= a.size() ? qFromLittleEndian<quint16>(a.constData() + at) : 0; }
quint32 get32(const QByteArray &a, qsizetype at) { return at >= 0 && at + 4 <= a.size() ? qFromLittleEndian<quint32>(a.constData() + at) : 0; }

} // namespace

bool readZipDirectory(const QByteArray &zip, QVector<ZipEntry> *out)
{
    out->clear();
    if (zip.size() < 22) return false;
    qsizetype eocd = -1;
    for (qsizetype i = zip.size() - 22; i >= std::max<qsizetype>(0, zip.size() - 65557); --i)
        if (get32(zip, i) == 0x06054b50) {
            eocd = i;
            break;
        }
    if (eocd < 0) return false;
    const int count = get16(zip, eocd + 10);
    qint64 p = get32(zip, eocd + 16);
    for (int i = 0; i < count; ++i) {
        if (p + 46 > zip.size() || get32(zip, p) != 0x02014b50) return false;
        ZipEntry e;
        e.method = get16(zip, p + 10);
        e.crc = get32(zip, p + 16);
        e.csize = get32(zip, p + 20);
        e.usize = get32(zip, p + 24);
        const quint16 nlen = get16(zip, p + 28), xlen = get16(zip, p + 30), clen = get16(zip, p + 32);
        const qint64 local = get32(zip, p + 42);
        // The entry's name, extra field and comment must fit in the file too.
        if (p + 46 + qint64(nlen) + xlen + clen > zip.size()) return false;
        e.name = QString::fromUtf8(zip.constData() + p + 46, nlen);
        p += 46 + qint64(nlen) + xlen + clen;
        if (local + 30 > zip.size() || get32(zip, local) != 0x04034b50) return false;
        e.dataAt = local + 30 + get16(zip, local + 26) + get16(zip, local + 28);
        if (e.method != 0 && e.method != 8) return false;
        if (e.dataAt + e.csize > zip.size()) return false;
        if (e.method == 0) e.usize = e.csize;
        out->append(e);
    }
    return true;
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
