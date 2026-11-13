#include "text/dictpack.h"

#include <QFile>
#include <QMutexLocker>
#include <map>
#include <zlib.h>

namespace jp::dictpack {

namespace {

const char kMagicFile[4] = {'J', 'Z', 'D', '1'};
const char kMagicThesaurus[4] = {'J', 'Z', 'T', '1'};
constexpr qint64 kBlockTarget = 64 << 10;
constexpr quint32 kMaxBlocks = 1 << 20;
constexpr quint32 kMaxKey = 4096;

void put32(QByteArray &b, quint32 v)
{
    for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xff));
}

quint32 get32(const char *p)
{
    quint32 v = 0;
    for (int i = 0; i < 4; ++i) v |= quint32(uchar(p[i])) << (8 * i);
    return v;
}

QByteArray deflateBytes(const char *data, qint64 size, int level)
{
    uLongf n = compressBound(uLong(size));
    QByteArray out(qsizetype(n), Qt::Uninitialized);
    if (compress2(reinterpret_cast<Bytef *>(out.data()), &n, reinterpret_cast<const Bytef *>(data), uLong(size), level) != Z_OK) return {};
    out.resize(qsizetype(n));
    return out;
}

// The zlib stream at `src` must end after exactly `rawSize` bytes and use all `n` of its own.
bool inflateExact(const char *src, qint64 n, QByteArray &out, quint32 rawSize)
{
    out.resize(qsizetype(rawSize));
    z_stream s{};
    if (inflateInit(&s) != Z_OK) { out.clear(); return false; }
    s.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(src));
    s.avail_in = uInt(n);
    s.next_out = reinterpret_cast<Bytef *>(out.data());
    s.avail_out = uInt(rawSize);
    const int r = inflate(&s, Z_FINISH);
    const bool ok = r == Z_STREAM_END && s.total_out == rawSize && s.avail_in == 0;
    inflateEnd(&s);
    if (!ok) out.clear();
    return ok;
}

} // namespace

QByteArray pack(const QByteArray &plain, int level)
{
    if (plain.size() > kMaxFile) return {};
    QByteArray z = deflateBytes(plain.constData(), plain.size(), level);
    if (z.isEmpty()) return {};
    QByteArray out(kMagicFile, 4);
    put32(out, quint32(plain.size()));
    return out + z;
}

bool unpack(const QByteArray &packed, QByteArray &out, qint64 cap)
{
    out.clear();
    if (packed.size() < 9 || memcmp(packed.constData(), kMagicFile, 4) != 0) return false;
    const quint32 rawSize = get32(packed.constData() + 4);
    if (qint64(rawSize) > cap || qint64(rawSize) > kMaxFile || packed.size() - 8 > kMaxFile) return false;
    return inflateExact(packed.constData() + 8, packed.size() - 8, out, rawSize);
}

QByteArray packThesaurus(const QByteArray &dat, int level)
{
    // The words and where their lines are, as the thesaurus reads them: a line that does not start with "(" and has a bar after at least
    // one character begins an entry (the word, lowercased), and the entry is that line and the number of lines the bar's right side says.
    std::map<QByteArray, QByteArray> entries;
    qsizetype p = dat.indexOf('\n') + 1;   // (the encoding line)
    while (p > 0 && p < dat.size()) {
        const qsizetype e = dat.indexOf('\n', p);
        if (e < 0) break;
        const QByteArray line = dat.mid(p, e - p).trimmed();
        if (!line.startsWith('(')) {
            const int bar = line.indexOf('|');
            if (bar > 0) {
                const int count = line.split('|').value(1).toInt();
                qsizetype q = e + 1;
                for (int i = 0; i < count && q > 0 && q < dat.size(); ++i) {
                    const qsizetype e2 = dat.indexOf('\n', q);
                    if (e2 < 0) { q = dat.size(); break; }
                    q = e2 + 1;
                }
                const qsizetype end = std::max<qsizetype>(q, e + 1);
                if (end - p > kMaxEntry) return {};
                entries[QString::fromUtf8(line.left(bar)).toLower().toUtf8()] = dat.mid(p, end - p);
            }
        }
        p = e + 1;   // (the lines of an entry are looked at in turn too, as the thesaurus reads them: they start with "(")
    }
    if (entries.empty()) return {};
    struct Packed { QByteArray firstKey; quint32 raw; QByteArray z; };
    std::vector<Packed> blocks;
    QByteArray raw, firstKey;
    auto flush = [&] {
        if (raw.isEmpty()) return;
        blocks.push_back({firstKey, quint32(raw.size()), deflateBytes(raw.constData(), raw.size(), level)});
        raw.clear();
    };
    for (const auto &[key, text] : entries) {
        if (raw.isEmpty()) firstKey = key;
        put32(raw, quint32(key.size()));
        raw += key;
        put32(raw, quint32(text.size()));
        raw += text;
        if (raw.size() >= kBlockTarget) flush();
    }
    flush();
    QByteArray out(kMagicThesaurus, 4);
    put32(out, quint32(blocks.size()));
    for (const Packed &b : blocks) {
        if (b.z.isEmpty()) return {};
        put32(out, b.raw);
        put32(out, quint32(b.z.size()));
        put32(out, quint32(b.firstKey.size()));
        out += b.firstKey;
    }
    for (const Packed &b : blocks) out += b.z;
    return out;
}

// The table: for each block its plain size, packed size, and first key; the packed blocks follow it in the same order.
bool Thesaurus::parse(const QByteArray &head, qint64 fileSize)
{
    m_blocks.clear();
    if (head.size() < 8 || memcmp(head.constData(), kMagicThesaurus, 4) != 0) return false;
    const quint32 n = get32(head.constData() + 4);
    if (n == 0 || n > kMaxBlocks) return false;
    qint64 at = 8;
    QVector<Block> blocks;
    for (quint32 i = 0; i < n; ++i) {
        if (at + 12 > head.size()) return false;
        Block b;
        b.rawSize = get32(head.constData() + at);
        b.compSize = get32(head.constData() + at + 4);
        const quint32 keyLen = get32(head.constData() + at + 8);
        at += 12;
        if (keyLen > kMaxKey || at + keyLen > head.size() || b.rawSize == 0 || b.rawSize > kMaxBlock) return false;
        b.firstKey = head.mid(at, keyLen);
        at += keyLen;
        if (!blocks.isEmpty() && !(blocks.last().firstKey < b.firstKey)) return false;   // (sorted, for the search)
        blocks << b;
    }
    qint64 pos = at;
    for (Block &b : blocks) {
        b.offset = pos;
        pos += b.compSize;
    }
    if (pos != fileSize) return false;
    m_blocks = blocks;
    return true;
}

void Thesaurus::close()
{
    QMutexLocker lock(&m_mutex);
    m_blocks.clear();
    m_cache.clear();
    m_mem.clear();
    m_path.clear();
}

bool Thesaurus::open(const QString &path)
{
    QMutexLocker lock(&m_mutex);
    m_blocks.clear();
    m_cache.clear();
    m_mem.clear();
    m_path = path;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    // The table is read in as much as it can be: the first 8 bytes give the block count, which bounds it (the keys are the rest).
    const QByteArray first = f.read(8);
    if (first.size() < 8 || memcmp(first.constData(), kMagicThesaurus, 4) != 0) return false;
    const quint32 n = get32(first.constData() + 4);
    if (n == 0 || n > kMaxBlocks) return false;
    const qint64 tableMax = qint64(n) * (12 + kMaxKey);
    const QByteArray head = first + f.read(std::min<qint64>(tableMax, f.size() - 8));
    return parse(head, f.size());
}

bool Thesaurus::openMemory(const QByteArray &packed)
{
    QMutexLocker lock(&m_mutex);
    m_blocks.clear();
    m_cache.clear();
    m_path.clear();
    m_mem = packed;
    return parse(packed, packed.size());
}

QByteArray Thesaurus::readBlock(int index) const
{
    if (const QByteArray *hit = m_cache.object(index)) return *hit;
    const Block &b = m_blocks.at(index);
    QByteArray z;
    if (!m_path.isEmpty()) {
        QFile f(m_path);
        if (!f.open(QIODevice::ReadOnly) || !f.seek(b.offset)) return {};
        z = f.read(b.compSize);
    } else {
        z = m_mem.mid(qsizetype(b.offset), qsizetype(b.compSize));
    }
    QByteArray raw;
    if (z.size() != qsizetype(b.compSize) || !inflateExact(z.constData(), z.size(), raw, b.rawSize)) return {};
    m_cache.insert(index, new QByteArray(raw), raw.size());
    return raw;
}

QByteArray Thesaurus::entry(const QByteArray &key) const
{
    QMutexLocker lock(&m_mutex);
    if (m_blocks.isEmpty()) return {};
    // The last block that starts at or before the key.
    int lo = 0, hi = int(m_blocks.size());
    while (lo < hi) {
        const int mid = (lo + hi) / 2;
        if (m_blocks.at(mid).firstKey <= key) lo = mid + 1;
        else hi = mid;
    }
    if (lo == 0) return {};
    const QByteArray raw = readBlock(lo - 1);
    qsizetype at = 0;
    while (at + 4 <= raw.size()) {
        const quint32 keyLen = get32(raw.constData() + at);
        at += 4;
        if (qint64(at) + keyLen + 4 > raw.size()) return {};
        const bool match = qsizetype(keyLen) == key.size() && memcmp(raw.constData() + at, key.constData(), keyLen) == 0;
        at += keyLen;
        const quint32 len = get32(raw.constData() + at);
        at += 4;
        if (qint64(at) + len > raw.size()) return {};
        if (match) return raw.mid(at, qsizetype(len));
        at += len;
    }
    return {};
}

} // namespace jp::dictpack
