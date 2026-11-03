#include "io/zip.h"

#include "core/appinfo.h"

#include <QCoreApplication>
#include <QFile>
#include <zlib.h>

#include <QtEndian>

#include <algorithm>
#include <cstring>

namespace jp {

quint32 crc32(const QByteArray &data)
{
    static quint32 table[256];
    static bool init = false;
    if (!init) {
        for (quint32 i = 0; i < 256; ++i) {
            quint32 c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[i] = c;
        }
        init = true;
    }
    quint32 c = 0xFFFFFFFFu;
    for (unsigned char b : data) c = table[(c ^ b) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

static void put16(QByteArray &a, quint16 v) { char b[2]; qToLittleEndian(v, b); a.append(b, 2); }
static void put32(QByteArray &a, quint32 v) { char b[4]; qToLittleEndian(v, b); a.append(b, 4); }

// Raw deflate (no zlib header), as ZIP entries hold it.
static QByteArray deflateRaw(const QByteArray &in)
{
    z_stream zs{};
    if (deflateInit2(&zs, 9, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK) return {};
    QByteArray out(qsizetype(deflateBound(&zs, uLong(in.size()))), Qt::Uninitialized);
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.constData()));
    zs.avail_in = uInt(in.size());
    zs.next_out = reinterpret_cast<Bytef *>(out.data());
    zs.avail_out = uInt(out.size());
    const int r = ::deflate(&zs, Z_FINISH);
    out.resize(qsizetype(zs.total_out));
    deflateEnd(&zs);
    return r == Z_STREAM_END ? out : QByteArray();
}

static QByteArray inflateRaw(const QByteArray &in, qsizetype expected, bool *ok)
{
    *ok = false;
    // Nothing this program saves is near this; more is a decompression bomb.
    if (expected < 0 || expected > (qsizetype(1) << 30)) return {};
    if (expected == 0) {
        *ok = true;
        return {};
    }
    z_stream zs{};
    if (inflateInit2(&zs, -15) != Z_OK) return {};
    QByteArray out(expected, Qt::Uninitialized);
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.constData()));
    zs.avail_in = uInt(in.size());
    zs.next_out = reinterpret_cast<Bytef *>(out.data());
    zs.avail_out = uInt(out.size());
    const int r = ::inflate(&zs, Z_FINISH);
    *ok = r == Z_STREAM_END && qsizetype(zs.total_out) == expected;
    inflateEnd(&zs);
    return out;
}

// Inflates raw deflate data as far as it goes: `consumed` is how many input bytes the stream took, `ended` whether it reached its end. What came out before a break is kept.
static QByteArray inflateAsFarAsItGoes(const char *in, qsizetype available, qint64 limit, qsizetype *consumed, bool *ended, bool *overLimit)
{
    *consumed = 0;
    *ended = false;
    *overLimit = false;
    QByteArray out;
    z_stream zs{};
    if (inflateInit2(&zs, -15) != Z_OK) return out;
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in));
    zs.avail_in = uInt(std::min<qsizetype>(available, 0x7fffffff));
    char buf[65536];
    for (;;) {
        zs.next_out = reinterpret_cast<Bytef *>(buf);
        zs.avail_out = sizeof(buf);
        const int r = ::inflate(&zs, Z_NO_FLUSH);
        out.append(buf, qsizetype(sizeof(buf) - zs.avail_out));
        if (out.size() > limit) {
            *overLimit = true;
            out.truncate(qsizetype(limit));
            break;
        }
        if (r == Z_STREAM_END) {
            *ended = true;
            break;
        }
        if (r != Z_OK || (zs.avail_in == 0 && zs.avail_out != 0)) break;   // damaged, or the data ran out
    }
    *consumed = qsizetype(zs.total_in);
    inflateEnd(&zs);
    return out;
}

QVector<ZipScanEntry> scanZip(const QByteArray &zip, int maxEntries, qint64 maxEntryBytes, qint64 maxTotalBytes)
{
    QVector<ZipScanEntry> found;
    qint64 total = 0;
    const char sig[4] = {'P', 'K', 3, 4};
    qsizetype at = 0;
    while (found.size() < maxEntries && total < maxTotalBytes) {
        at = zip.indexOf(QByteArrayView(sig, 4), at);
        if (at < 0 || zip.size() - at < 30) break;
        const uchar *h = reinterpret_cast<const uchar *>(zip.constData() + at);
        const quint16 flags = qFromLittleEndian<quint16>(h + 6), method = qFromLittleEndian<quint16>(h + 8);
        const quint32 crc = qFromLittleEndian<quint32>(h + 14), csize = qFromLittleEndian<quint32>(h + 18), usize = qFromLittleEndian<quint32>(h + 22);
        const quint16 nameLen = qFromLittleEndian<quint16>(h + 26), extraLen = qFromLittleEndian<quint16>(h + 28);
        const qsizetype dataAt = at + 30 + nameLen + extraLen;
        if (nameLen == 0 || dataAt > zip.size()) {
            at += 4;
            continue;
        }
        ZipScanEntry e;
        e.name = QString::fromUtf8(zip.constData() + at + 30, nameLen);
        const bool sizesKnown = !(flags & 8) || csize != 0;   // (bit 3: the sizes follow the data, and the header has none)
        const qsizetype room = zip.size() - dataAt;
        qsizetype next = dataAt;
        if (e.name.endsWith(QLatin1Char('/'))) {   // a folder
            at = dataAt;
            continue;
        }
        if (flags & 1) {
            e.problem = QStringLiteral("encrypted");
            next = sizesKnown && qsizetype(csize) <= room ? dataAt + qsizetype(csize) : dataAt;
        } else if (method == 0) {
            if (!sizesKnown) e.problem = QStringLiteral("stored with no size");
            else {
                const qsizetype n = std::min<qsizetype>(qsizetype(csize), room);
                if (qint64(n) > maxEntryBytes) e.problem = QStringLiteral("too large");
                else e.data = zip.mid(dataAt, n);
                e.whole = e.problem.isEmpty() && n == qsizetype(csize) && (crc == 0 || !sizesKnown || jp::crc32(e.data) == crc);
                if (e.problem.isEmpty() && !e.whole) e.problem = n < qsizetype(csize) ? QStringLiteral("cut short") : QStringLiteral("damaged");
                next = dataAt + n;
            }
        } else if (method == 8) {
            qsizetype consumed = 0;
            bool ended = false, over = false;
            const qsizetype span = sizesKnown ? std::min<qsizetype>(qsizetype(csize), room) : room;
            e.data = inflateAsFarAsItGoes(zip.constData() + dataAt, span, std::min<qint64>(maxEntryBytes, maxTotalBytes - total), &consumed, &ended, &over);
            e.whole = ended && (!sizesKnown || (qsizetype(usize) == e.data.size() && jp::crc32(e.data) == crc));
            if (over) e.problem = QStringLiteral("too large");
            else if (!e.whole) e.problem = ended ? QStringLiteral("damaged") : QStringLiteral("cut short");
            next = dataAt + std::max<qsizetype>(consumed, 0);
        } else {
            e.problem = QStringLiteral("a kind of compression that isn't read");
            next = sizesKnown && qsizetype(csize) <= room ? dataAt + qsizetype(csize) : dataAt;
        }
        total += e.data.size();
        found << e;
        at = std::max(next, at + 4);
    }
    return found;
}

void ZipWriter::add(const QString &name, const QByteArray &data, bool compress)
{
    const QByteArray n = name.toUtf8();
    QByteArray packed = compress ? deflateRaw(data) : QByteArray();
    const bool deflated = !packed.isEmpty() && packed.size() < data.size();
    if (!deflated) packed = data;
    Entry e{name, crc32(data), quint32(data.size()), quint32(packed.size()), quint16(deflated ? 8 : 0), quint32(m_out.size())};
    put32(m_out, 0x04034b50);
    put16(m_out, 20);        // version needed
    put16(m_out, 0x0800);    // UTF-8 names
    put16(m_out, e.method);  // stored or deflated
    put16(m_out, 0); put16(m_out, 0x21); // time, date (1980-01-01)
    put32(m_out, e.crc);
    put32(m_out, e.csize);
    put32(m_out, e.size);
    put16(m_out, quint16(n.size()));
    put16(m_out, 0);
    m_out.append(n);
    m_out.append(packed);
    m_entries << e;
}

void ZipWriter::addRaw(const QString &name, quint16 method, quint32 crc, quint32 csize, quint32 usize, const char *bytes)
{
    const QByteArray n = name.toUtf8();
    Entry e{name, crc, usize, csize, method, quint32(m_out.size())};
    put32(m_out, 0x04034b50);
    put16(m_out, 20);
    put16(m_out, 0x0800);
    put16(m_out, e.method);
    put16(m_out, 0); put16(m_out, 0x21);
    put32(m_out, e.crc);
    put32(m_out, e.csize);
    put32(m_out, e.size);
    put16(m_out, quint16(n.size()));
    put16(m_out, 0);
    m_out.append(n);
    m_out.append(bytes, qsizetype(csize));
    m_entries << e;
}

// A streamed entry: where its header is (the sizes and CRC are put in it at the end), what is counted so far, and the deflater.
struct ZipWriter::Stream {
    qsizetype headerAt = 0;
    QString name;
    bool deflated = false;
    quint32 crc = 0;
    quint64 usize = 0;
    z_stream z{};
};

ZipWriter::~ZipWriter()
{
    if (m_stream) {
        if (m_stream->deflated) deflateEnd(&m_stream->z);
        delete m_stream;
    }
}

void ZipWriter::beginStream(const QString &name, bool compress)
{
    if (m_stream) endStream();
    m_stream = new Stream;
    m_stream->name = name;
    m_stream->headerAt = m_out.size();
    m_stream->crc = quint32(::crc32(0L, Z_NULL, 0));
    const QByteArray n = name.toUtf8();
    put32(m_out, 0x04034b50);
    put16(m_out, 20);
    put16(m_out, 0x0800);
    m_stream->deflated = compress && deflateInit2(&m_stream->z, 6, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) == Z_OK;
    put16(m_out, m_stream->deflated ? 8 : 0);
    put16(m_out, 0); put16(m_out, 0x21);
    put32(m_out, 0); put32(m_out, 0); put32(m_out, 0);   // (the CRC and sizes: put in when the entry ends)
    put16(m_out, quint16(n.size()));
    put16(m_out, 0);
    m_out.append(n);
}

void ZipWriter::write(const char *data, qsizetype size)
{
    if (!m_stream || size <= 0) return;
    Stream &s = *m_stream;
    s.crc = quint32(::crc32(uLong(s.crc), reinterpret_cast<const Bytef *>(data), uInt(size)));
    s.usize += quint64(size);
    if (!s.deflated) {
        m_out.append(data, size);
        return;
    }
    s.z.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data));
    s.z.avail_in = uInt(size);
    char buffer[1 << 16];
    do {
        s.z.next_out = reinterpret_cast<Bytef *>(buffer);
        s.z.avail_out = sizeof buffer;
        ::deflate(&s.z, Z_NO_FLUSH);
        m_out.append(buffer, qsizetype(sizeof buffer - s.z.avail_out));
    } while (s.z.avail_out == 0);
}

void ZipWriter::endStream()
{
    if (!m_stream) return;
    Stream &s = *m_stream;
    const qsizetype dataAt = s.headerAt + 30 + s.name.toUtf8().size();
    if (s.deflated) {
        char buffer[1 << 16];
        int r;
        do {
            s.z.next_in = nullptr;
            s.z.avail_in = 0;
            s.z.next_out = reinterpret_cast<Bytef *>(buffer);
            s.z.avail_out = sizeof buffer;
            r = ::deflate(&s.z, Z_FINISH);
            m_out.append(buffer, qsizetype(sizeof buffer - s.z.avail_out));
        } while (r != Z_STREAM_END && r == Z_OK);
        deflateEnd(&s.z);
    }
    const quint32 csize = quint32(m_out.size() - dataAt), usize = quint32(s.usize);
    char fields[12];
    qToLittleEndian(s.crc, fields);
    qToLittleEndian(csize, fields + 4);
    qToLittleEndian(usize, fields + 8);
    std::memcpy(m_out.data() + s.headerAt + 14, fields, 12);
    m_entries << Entry{s.name, s.crc, usize, csize, quint16(s.deflated ? 8 : 0), quint32(s.headerAt)};
    delete m_stream;
    m_stream = nullptr;
}

QByteArray ZipWriter::finish()
{
    if (m_stream) endStream();
    const quint32 cdStart = quint32(m_out.size());
    for (const auto &e : m_entries) {
        const QByteArray n = e.name.toUtf8();
        put32(m_out, 0x02014b50);
        put16(m_out, 20); put16(m_out, 20);
        put16(m_out, 0x0800);
        put16(m_out, e.method);
        put16(m_out, 0); put16(m_out, 0x21);
        put32(m_out, e.crc);
        put32(m_out, e.csize);
        put32(m_out, e.size);
        put16(m_out, quint16(n.size()));
        put16(m_out, 0); put16(m_out, 0); put16(m_out, 0); put16(m_out, 0);
        put32(m_out, 0);
        put32(m_out, e.offset);
        m_out.append(n);
    }
    const quint32 cdSize = quint32(m_out.size()) - cdStart;
    put32(m_out, 0x06054b50);
    put16(m_out, 0); put16(m_out, 0);
    put16(m_out, quint16(m_entries.size()));
    put16(m_out, quint16(m_entries.size()));
    put32(m_out, cdSize);
    put32(m_out, cdStart);
    put16(m_out, 0);
    return m_out;
}

// Values past the end read as 0: a damaged file can't make these read
// outside the data, whatever the caller checked.
static quint16 get16(const QByteArray &a, qsizetype at) { return at >= 0 && at + 2 <= a.size() ? qFromLittleEndian<quint16>(a.constData() + at) : 0; }
static quint32 get32(const QByteArray &a, qsizetype at) { return at >= 0 && at + 4 <= a.size() ? qFromLittleEndian<quint32>(a.constData() + at) : 0; }

bool readZip(const QByteArray &zip, QMap<QString, QByteArray> &out, QString *error)
{
    auto fail = [&](const QString &m) { if (error) *error = m; return false; };
    if (zip.size() < 22) return fail(QCoreApplication::translate("Import", "The file is too small to be a publication."));
    qsizetype eocd = -1;
    for (qsizetype i = zip.size() - 22; i >= std::max<qsizetype>(0, zip.size() - 65557); --i)
        if (get32(zip, i) == 0x06054b50) { eocd = i; break; }
    if (eocd < 0) return fail(QCoreApplication::translate("Import", "The file is not a %1 publication (no ZIP directory).").arg(appInfo().name));
    const int count = get16(zip, eocd + 10);
    qsizetype p = get32(zip, eocd + 16);
    for (int i = 0; i < count; ++i) {
        if (p + 46 > zip.size() || get32(zip, p) != 0x02014b50) return fail(QCoreApplication::translate("Import", "The publication's file directory is damaged."));
        const quint16 method = get16(zip, p + 10);
        const quint32 crc = get32(zip, p + 16), csize = get32(zip, p + 20), usize = get32(zip, p + 24);
        const quint16 nlen = get16(zip, p + 28), xlen = get16(zip, p + 30), clen = get16(zip, p + 32);
        const quint32 local = get32(zip, p + 42);
        // The entry's name, extra field and comment must fit in the file too.
        if (p + 46 + qsizetype(nlen) + xlen + clen > zip.size()) return fail(QCoreApplication::translate("Import", "The publication's file directory is damaged."));
        const QString name = QString::fromUtf8(zip.constData() + p + 46, nlen);
        p += 46 + nlen + xlen + clen;
        if (qsizetype(local) + 30 > zip.size()) return fail(QCoreApplication::translate("Import", "The publication is truncated."));
        const qsizetype data = local + 30 + get16(zip, local + 26) + get16(zip, local + 28);
        if (method != 0 && method != 8) return fail(QCoreApplication::translate("Import", "The publication uses a compression method %1 cannot read.").arg(appInfo().name));
        if (data + qsizetype(csize) > zip.size()) return fail(QCoreApplication::translate("Import", "The publication is truncated."));
        if (method == 8) {
            bool ok = false;
            const QByteArray inflated = inflateRaw(zip.mid(data, csize), qsizetype(usize), &ok);
            if (!ok || crc32(inflated) != crc) return fail(QCoreApplication::translate("Import", "The publication is damaged."));
            out.insert(name, inflated);
        } else {
            out.insert(name, zip.mid(data, csize));
        }
    }
    return true;
}

QByteArray readZipEntry(const QString &path, const QString &name)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly) || f.size() < 22) return {};
    // The end-of-directory record sits in the last 65,557 bytes.
    const qint64 tailAt = std::max<qint64>(0, f.size() - 65557);
    f.seek(tailAt);
    const QByteArray tail = f.read(f.size() - tailAt);
    qsizetype eocd = -1;
    for (qsizetype i = tail.size() - 22; i >= 0; --i)
        if (get32(tail, i) == 0x06054b50) { eocd = i; break; }
    if (eocd < 0) return {};
    const quint32 dirSize = get32(tail, eocd + 12), dirAt = get32(tail, eocd + 16);
    if (qint64(dirAt) + dirSize > f.size() || dirSize > 64 * 1024 * 1024) return {};
    f.seek(dirAt);
    const QByteArray dir = f.read(dirSize);
    for (qsizetype p = 0; p + 46 <= dir.size() && get32(dir, p) == 0x02014b50;) {
        const quint16 method = get16(dir, p + 10), nlen = get16(dir, p + 28), xlen = get16(dir, p + 30), clen = get16(dir, p + 32);
        const quint32 csize = get32(dir, p + 20), local = get32(dir, p + 42);
        if (p + 46 + qsizetype(nlen) + xlen + clen > dir.size()) return {};
        const QString entry = QString::fromUtf8(dir.constData() + p + 46, nlen);
        p += 46 + nlen + xlen + clen;
        if (entry != name) continue;
        if (method != 0 || !f.seek(local)) return {};
        const QByteArray head = f.read(30);
        if (head.size() < 30) return {};
        const qint64 data = qint64(local) + 30 + get16(head, 26) + get16(head, 28);
        if (data + csize > f.size() || !f.seek(data)) return {};
        return f.read(csize);
    }
    return {};
}

} // namespace jp
