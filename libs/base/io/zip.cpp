#include "io/zip.h"
#include "io/zipindex.h"

#include "core/appinfo.h"

#include <QCoreApplication>
#include <QFile>
#include <zlib.h>

#include <QtEndian>

#include <algorithm>
#include <cstring>
#include <memory>

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
static void put64(QByteArray &a, quint64 v) { char b[8]; qToLittleEndian(v, b); a.append(b, 8); }
// What a 32-bit field of a local header holds of a size: the size, or all ones when it is past the field (the directory has it whole).
static quint32 field32(quint64 v) { return v >= 0xFFFFFFFFu ? 0xFFFFFFFFu : quint32(v); }

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

// Inflates raw deflate data as far as it goes: `consumed` is how many input bytes the stream took, `ended` whether it reached its end. What came out before a break is kept.
static QByteArray inflateAsFarAsItGoes(const char *in, qsizetype available, qint64 limit, qsizetype *consumed, bool *ended, bool *overLimit, qint64 expected = -1, qint64 *made = nullptr)
{
    *consumed = 0;
    *ended = false;
    *overLimit = false;
    QByteArray out;
    // (the size the header gives, when it is within the limit: one buffer of that size, not one that grows by doubling to up to twice what is needed)
    if (expected > 0 && expected <= limit) out.reserve(qsizetype(expected) + 1);
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
        if (made) *made = out.size();
        if (out.size() > limit) {
            *overLimit = true;
            out.clear();   // (what is over the limit is not kept: not a part of it either)
            out.squeeze();
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

void scanZipEach(const QByteArray &zip, const std::function<void(ZipScanEntry &)> &each, int maxEntries, qint64 maxEntryBytes, qint64 maxTotalBytes, ZipScanStats *stats)
{
    if (stats) *stats = ZipScanStats();
    qint64 total = 0;
    int taken = 0;
    const char sig[4] = {'P', 'K', 3, 4};
    qsizetype at = 0;
    while (taken < maxEntries && total < maxTotalBytes) {
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
        const qint64 allowed = std::min<qint64>(maxEntryBytes, maxTotalBytes - total);
        qint64 inflated = -1;
        if (flags & 1) {
            e.problem = QStringLiteral("encrypted");
            next = sizesKnown && qsizetype(csize) <= room ? dataAt + qsizetype(csize) : dataAt;
        } else if (method == 0) {
            if (!sizesKnown) e.problem = QStringLiteral("stored with no size");
            else {
                // A size that is more than the file has left is a lie, or a cut: the entry is what is there up to the next entry's header (or the end of the file), and the scan goes on from
                // there, so a stranger's size never swallows the entries after it.
                qsizetype n = std::min<qsizetype>(qsizetype(csize), room);
                if (qsizetype(csize) > room) {
                    const qsizetype nextHeader = zip.indexOf(QByteArrayView(sig, 4), dataAt);
                    if (nextHeader >= 0) n = nextHeader - dataAt;
                }
                if (qint64(n) > allowed) e.problem = QStringLiteral("too large");
                else e.data = zip.mid(dataAt, n);
                e.whole = e.problem.isEmpty() && n == qsizetype(csize) && (crc == 0 || !sizesKnown || jp::crc32(e.data) == crc);
                // A size that stays inside the file but is not the entry's (its bytes do not match) may run over the entries after it: it ends at the next entry's header, and the scan goes on.
                if (!e.whole && e.problem.isEmpty() && n > 0) {
                    const qsizetype nextHeader = zip.indexOf(QByteArrayView(sig, 4), dataAt);
                    if (nextHeader >= 0 && nextHeader - dataAt < n) {
                        n = nextHeader - dataAt;
                        e.data = zip.mid(dataAt, n);
                    }
                }
                if (e.problem.isEmpty() && !e.whole) e.problem = n < qsizetype(csize) ? QStringLiteral("cut short") : QStringLiteral("damaged");
                next = dataAt + n;
            }
        } else if (method == 8) {
            qsizetype consumed = 0;
            bool ended = false, over = false;
            qint64 made = 0;
            const qsizetype span = sizesKnown ? std::min<qsizetype>(qsizetype(csize), room) : room;
            e.data = inflateAsFarAsItGoes(zip.constData() + dataAt, span, allowed, &consumed, &ended, &over, sizesKnown ? qint64(usize) : -1, &made);
            inflated = made;   // (an entry left out as too large was inflated as far as the limit: that is work done, and counts)
            e.whole = ended && (!sizesKnown || (qsizetype(usize) == e.data.size() && jp::crc32(e.data) == crc));
            if (over) e.problem = QStringLiteral("too large");
            else if (!e.whole) e.problem = ended ? QStringLiteral("damaged") : QStringLiteral("cut short");
            next = dataAt + std::max<qsizetype>(consumed, 0);
        } else {
            e.problem = QStringLiteral("a kind of compression that isn't read");
            next = sizesKnown && qsizetype(csize) <= room ? dataAt + qsizetype(csize) : dataAt;
        }
        total += std::max<qint64>(e.data.size(), inflated);
        ++taken;
        at = std::max(next, at + 4);
        each(e);
    }
    if (stats && at >= 0 && at + 30 <= zip.size()) {
        // (is there more to find, beyond what the limits let through?)
        const qsizetype more = zip.indexOf(QByteArrayView(sig, 4), at);
        if (more >= 0 && zip.size() - more >= 30) {
            if (taken >= maxEntries) stats->entriesLeftOut = true;
            else if (total >= maxTotalBytes) stats->sizeLeftOut = true;
        }
    }
}

QVector<ZipScanEntry> scanZip(const QByteArray &zip, int maxEntries, qint64 maxEntryBytes, qint64 maxTotalBytes)
{
    QVector<ZipScanEntry> found;
    scanZipEach(zip, [&](ZipScanEntry &e) { found << e; }, maxEntries, maxEntryBytes, maxTotalBytes);
    return found;
}

void ZipWriter::add(const QString &name, const QByteArray &data, bool compress)
{
    const QByteArray n = name.toUtf8();
    QByteArray packed = compress ? deflateRaw(data) : QByteArray();
    const bool deflated = !packed.isEmpty() && packed.size() < data.size();
    if (!deflated) packed = data;
    Entry e{name, crc32(data), quint64(data.size()), quint64(packed.size()), quint16(deflated ? 8 : 0), quint64(m_out.size())};
    put32(m_out, 0x04034b50);
    put16(m_out, 20);        // version needed
    put16(m_out, 0x0800);    // UTF-8 names
    put16(m_out, e.method);  // stored or deflated
    put16(m_out, 0); put16(m_out, 0x21); // time, date (1980-01-01)
    put32(m_out, e.crc);
    put32(m_out, field32(e.csize));
    put32(m_out, field32(e.size));
    put16(m_out, quint16(n.size()));
    put16(m_out, 0);
    m_out.append(n);
    m_out.append(packed);
    m_entries << e;
}

void ZipWriter::addRaw(const QString &name, quint16 method, quint32 crc, quint64 csize, quint64 usize, const char *bytes, quint16 flags)
{
    const QByteArray n = name.toUtf8();
    Entry e{name, crc, usize, csize, method, quint64(m_out.size()), quint16((flags & ~0x0008) | 0x0800)};
    put32(m_out, 0x04034b50);
    put16(m_out, 20);
    put16(m_out, e.flags);
    put16(m_out, e.method);
    put16(m_out, 0); put16(m_out, 0x21);
    put32(m_out, e.crc);
    put32(m_out, field32(e.csize));
    put32(m_out, field32(e.size));
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
    const quint64 csize = quint64(m_out.size() - dataAt), usize = s.usize;
    char fields[12];
    qToLittleEndian(s.crc, fields);
    qToLittleEndian(field32(csize), fields + 4);
    qToLittleEndian(field32(usize), fields + 8);
    std::memcpy(m_out.data() + s.headerAt + 14, fields, 12);
    m_entries << Entry{s.name, s.crc, usize, csize, quint16(s.deflated ? 8 : 0), quint64(s.headerAt)};
    delete m_stream;
    m_stream = nullptr;
}

QByteArray ZipWriter::finish()
{
    if (m_stream) endStream();
    const quint64 cdStart = quint64(m_out.size());
    for (const auto &e : m_entries) {
        const QByteArray n = e.name.toUtf8();
        // What does not fit a field of the record is put in the extra field of kind 1 (in this order), and the field holds all ones.
        const bool bigSize = m_long || e.size >= 0xFFFFFFFFu, bigCsize = m_long || e.csize >= 0xFFFFFFFFu, bigOffset = m_long || e.offset >= 0xFFFFFFFFu;
        const int longFields = int(bigSize) + int(bigCsize) + int(bigOffset);
        const bool isLong = longFields > 0;
        put32(m_out, 0x02014b50);
        put16(m_out, isLong ? 45 : 20); put16(m_out, isLong ? 45 : 20);
        put16(m_out, e.flags);
        put16(m_out, e.method);
        put16(m_out, 0); put16(m_out, 0x21);
        put32(m_out, e.crc);
        put32(m_out, bigCsize ? 0xFFFFFFFFu : quint32(e.csize));
        put32(m_out, bigSize ? 0xFFFFFFFFu : quint32(e.size));
        put16(m_out, quint16(n.size()));
        put16(m_out, isLong ? quint16(4 + 8 * longFields) : 0); put16(m_out, 0); put16(m_out, 0); put16(m_out, 0);
        put32(m_out, 0);
        put32(m_out, bigOffset ? 0xFFFFFFFFu : quint32(e.offset));
        m_out.append(n);
        if (isLong) {
            put16(m_out, 1);
            put16(m_out, quint16(8 * longFields));
            if (bigSize) put64(m_out, e.size);
            if (bigCsize) put64(m_out, e.csize);
            if (bigOffset) put64(m_out, e.offset);
        }
    }
    const quint64 cdSize = quint64(m_out.size()) - cdStart;
    const quint64 count = quint64(m_entries.size());
    // More entries, or a directory or ZIP, than the end record's fields hold: the long end record (and its locator) says it, and the end record's fields hold all ones.
    const bool longEnd = m_long || count >= 0xFFFF || cdStart >= 0xFFFFFFFFu || cdSize >= 0xFFFFFFFFu;
    if (longEnd) {
        const quint64 at = quint64(m_out.size());
        put32(m_out, 0x06064b50);
        put64(m_out, 44);
        put16(m_out, 45); put16(m_out, 45);
        put32(m_out, 0); put32(m_out, 0);
        put64(m_out, count); put64(m_out, count);
        put64(m_out, cdSize); put64(m_out, cdStart);
        put32(m_out, 0x07064b50);
        put32(m_out, 0);
        put64(m_out, at);
        put32(m_out, 1);
    }
    put32(m_out, 0x06054b50);
    put16(m_out, 0); put16(m_out, 0);
    put16(m_out, longEnd ? 0xFFFF : quint16(count));
    put16(m_out, longEnd ? 0xFFFF : quint16(count));
    put32(m_out, longEnd ? 0xFFFFFFFFu : quint32(cdSize));
    put32(m_out, longEnd ? 0xFFFFFFFFu : quint32(cdStart));
    put16(m_out, 0);
    return m_out;
}

// Values past the end read as 0: a damaged file can't make these read
// outside the data, whatever the caller checked.
static quint16 get16(const QByteArray &a, qsizetype at) { return at >= 0 && at + 2 <= a.size() ? qFromLittleEndian<quint16>(a.constData() + at) : 0; }
static quint32 get32(const QByteArray &a, qsizetype at) { return at >= 0 && at + 4 <= a.size() ? qFromLittleEndian<quint32>(a.constData() + at) : 0; }

bool readZip(const QByteArray &zip, QMap<QString, QByteArray> &out, QString *error, qint64 maxInflated)
{
    auto fail = [&](const QString &m) { if (error) *error = m; return false; };
    if (zip.size() < 22) return fail(QCoreApplication::translate("Import", "The file is too small to be a publication."));
    QVector<ZipEntry> directory;
    if (!readZipDirectory(zip, &directory)) {
        switch (zipFault(zip)) {
        case ZipFault::NoEnd: return fail(QCoreApplication::translate("Import", "The file is not a %1 publication (no ZIP directory).").arg(appInfo().name));
        case ZipFault::UnsupportedMethod: return fail(QCoreApplication::translate("Import", "The publication uses a compression method %1 cannot read.").arg(appInfo().name));
        default: return fail(QCoreApplication::translate("Import", "The publication's file directory is damaged."));
        }
    }
    // What the entries say they come to is checked before any is inflated, and each byte that comes out of an inflater is counted too, so no directory can make more.
    qint64 declared = 0;
    for (const ZipEntry &e : std::as_const(directory)) declared += e.usize;
    if (maxInflated >= 0 && declared > maxInflated) return fail(QCoreApplication::translate("Import", "The publication holds more than %1 can read.").arg(appInfo().name));
    auto budget = std::make_shared<InflateBudget>();
    budget->limit = maxInflated;
    for (const ZipEntry &e : std::as_const(directory)) {
        QByteArray bytes;
        if (!inflateZipEntry(zip, e, &bytes, budget)) return fail(QCoreApplication::translate("Import", "The publication is damaged."));
        out.insert(e.name, bytes);   // (of two entries of one name the later is the one read, as readZipEntry gives it)
    }
    return true;
}

QByteArray readZipEntry(const QString &path, const QString &name)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly) || f.size() < 22) return {};
    auto readAt = [&](qint64 at, qint64 n) {
        if (!f.seek(at)) return QByteArray();
        return f.read(n);
    };
    ZipEnd end;
    if (!readZipEnd(readAt, f.size(), &end)) return {};
    if (end.dirAt < 0 || end.dirAt + end.dirSize > f.size() || end.dirSize > 64 * 1024 * 1024) return {};
    const QByteArray dir = readAt(end.dirAt, end.dirSize);
    // (of two entries of one name the later is the one given, as readZip gives it)
    ZipRecord found;
    bool have = false;
    qint64 p = 0;
    for (quint64 i = 0; i < end.count; ++i) {
        ZipRecord r;
        qint64 next = 0;
        if (!readZipRecord(dir, p, &r, &next)) return {};
        p = next;
        if (r.name != name) continue;
        found = r;
        have = true;
    }
    if (!have || found.method != 0) return {};
    const QByteArray head = readAt(found.local, 30);
    if (head.size() < 30 || get32(head, 0) != 0x04034b50) return {};
    const qint64 data = found.local + 30 + get16(head, 26) + get16(head, 28);
    if (data + found.csize > f.size()) return {};
    const QByteArray bytes = readAt(data, found.csize);
    if (bytes.size() != found.csize || crc32(bytes) != found.crc) return {};
    return bytes;
}

} // namespace jp
