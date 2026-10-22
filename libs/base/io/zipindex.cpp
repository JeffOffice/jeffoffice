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
        if (local + 30 > zip.size()) return false;
        e.dataAt = local + 30 + get16(zip, local + 26) + get16(zip, local + 28);
        if (e.method != 0 && e.method != 8) return false;
        if (e.dataAt + e.csize > zip.size()) return false;
        if (e.method == 0) e.usize = e.csize;
        out->append(e);
    }
    return true;
}

bool inflateZipEntry(const QByteArray &zip, const ZipEntry &entry, QByteArray *out, const InflateCounter &counter)
{
    out->clear();
    if (entry.dataAt < 0 || entry.csize < 0 || entry.dataAt + entry.csize > zip.size()) return false;
    if (entry.method == 0) {
        *out = zip.mid(qsizetype(entry.dataAt), qsizetype(entry.csize));
        if (counter) counter->fetch_add(out->size());
        return true;
    }
    if (entry.method != 8 || entry.usize < 0 || entry.usize > kMaxZipEntryBytes) return false;
    if (entry.usize == 0) return ::crc32(0, nullptr, 0) == entry.crc;
    z_stream zs{};
    if (inflateInit2(&zs, -15) != Z_OK) return false;
    QByteArray bytes(qsizetype(entry.usize), Qt::Uninitialized);
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(zip.constData() + entry.dataAt));
    zs.avail_in = uInt(entry.csize);
    zs.next_out = reinterpret_cast<Bytef *>(bytes.data());
    zs.avail_out = uInt(bytes.size());
    const int r = ::inflate(&zs, Z_FINISH);
    const bool good = r == Z_STREAM_END && qint64(zs.total_out) == entry.usize;
    inflateEnd(&zs);
    if (counter) counter->fetch_add(qint64(zs.total_out));
    if (!good || quint32(::crc32(0, reinterpret_cast<const Bytef *>(bytes.constData()), uInt(bytes.size()))) != entry.crc) return false;
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

ZipEntryDevice::ZipEntryDevice(const QByteArray &zip, const ZipEntry &entry, const InflateCounter &counter)
    : m_zip(zip), m_entry(entry), m_counter(counter), m_crc(quint32(::crc32(0, nullptr, 0)))
{
    setOpenMode(QIODevice::ReadOnly);
    if (entry.dataAt < 0 || entry.csize < 0 || entry.dataAt + entry.csize > zip.size()) {
        fail(QStringLiteral("The entry is not inside the file."));
        return;
    }
    if (entry.method == 8) {
        if (entry.usize < 0 || entry.usize > kMaxZipEntryBytes) {
            fail(QStringLiteral("The entry is too large."));
            return;
        }
        m_z = std::make_unique<Inflater>();
        if (inflateInit2(&m_z->zs, -15) != Z_OK) {
            m_z.reset();
            fail(QStringLiteral("The inflater could not start."));
            return;
        }
        m_z->open = true;
        m_z->zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(m_zip.constData() + entry.dataAt));
        m_z->zs.avail_in = uInt(entry.csize);
    } else if (entry.method != 0) {
        fail(QStringLiteral("The entry's compression is not one this program reads."));
    }
}

ZipEntryDevice::~ZipEntryDevice() = default;

bool ZipEntryDevice::fail(const QString &why)
{
    m_failed = true;
    setErrorString(why);
    return false;
}

qint64 ZipEntryDevice::readData(char *data, qint64 maxlen)
{
    if (m_failed) return -1;
    if (m_done || maxlen <= 0) return 0;
    if (m_entry.method == 0) {
        // Stored: the bytes as they are, in the piece asked for.
        const qint64 n = std::min(maxlen, m_entry.csize - m_out);
        if (n <= 0) {
            m_done = true;
            return 0;
        }
        std::memcpy(data, m_zip.constData() + m_entry.dataAt + m_out, size_t(n));
        m_out += n;
        if (m_counter) m_counter->fetch_add(n);
        return n;
    }
    z_stream &zs = m_z->zs;
    qint64 produced = 0;
    // What has come out so far is given on; the failure is found by the next read.
    auto bad = [&](const QString &why) {
        fail(why);
        return produced > 0 ? produced : qint64(-1);
    };
    while (produced < maxlen) {
        // Never more than the directory said: a stream that runs on is damage.
        const qint64 room = std::min<qint64>({maxlen - produced, m_entry.usize - m_out, qint64(1) << 30});
        if (room <= 0) {
            // It has given all it was said to hold, so it must end here.
            Bytef probe;
            zs.next_out = &probe;
            zs.avail_out = 1;
            const int r = ::inflate(&zs, Z_NO_FLUSH);
            if (r != Z_STREAM_END || zs.avail_out != 1) return bad(QStringLiteral("The entry inflates to more than its directory says."));
            m_done = true;
            break;
        }
        zs.next_out = reinterpret_cast<Bytef *>(data + produced);
        zs.avail_out = uInt(room);
        const int r = ::inflate(&zs, Z_NO_FLUSH);
        const qint64 got = room - qint64(zs.avail_out);
        m_crc = quint32(::crc32(m_crc, reinterpret_cast<const Bytef *>(data + produced), uInt(got)));
        produced += got;
        m_out += got;
        if (m_counter) m_counter->fetch_add(got);
        if (r == Z_STREAM_END) {
            m_done = true;
            break;
        }
        // With room to write and no progress, the input has run out before the stream ended.
        if ((r != Z_OK && r != Z_BUF_ERROR) || got == 0)
            return bad(r != Z_OK && r != Z_BUF_ERROR ? QStringLiteral("The entry's data is damaged.") : QStringLiteral("The entry's data ends early."));
    }
    if (m_done && (m_out != m_entry.usize || m_crc != m_entry.crc)) return bad(QStringLiteral("The entry is damaged: its size or checksum is wrong."));
    return produced;
}

} // namespace jp
