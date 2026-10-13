#include "office/mtx.h"

#include "core/sfnt.h"

#include <QCoreApplication>
#include <QtEndian>
#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

namespace jp::office {

namespace {

QString say(const char *text) { return QCoreApplication::translate("MicroTypeExpress", text); }

using Bytes = std::vector<quint8>;

// What one block may come to once decompressed (the font may not be larger either), so that no number in the data can ask for more.
constexpr qint64 kMaxBlock = kMaxEmbeddedFontBytes;

// Bits are read from the most significant end of each byte.
struct BitReader {
    const quint8 *data;
    qint64 size;
    qint64 position = 0;   // in bits
    bool failed = false;
    int bit()
    {
        const qint64 byte = position >> 3;
        if (byte >= size) {
            failed = true;
            return 0;
        }
        const int b = (data[byte] >> (7 - (position & 7))) & 1;
        ++position;
        return b;
    }
    quint32 value(int bits)
    {
        quint32 v = 0;
        for (int i = 0; i < bits; ++i) v = v << 1 | quint32(bit());
        return v;
    }
};

// The adaptive Huffman code the symbols, copy lengths, and copy distances are written in: a tree of nodes (the root is 1, a node's children are
// 2n and 2n+1, the leaves are the last `range` nodes) whose leaves weigh how often each symbol came, and that is rearranged after each symbol
// so nodes of equal weight are neighbors. The tree for symbols starts as if it had seen the copy commands a little, so they are short.
class AdaptiveHuffman {
public:
    explicit AdaptiveHuffman(int range) : m_range(range)
    {
        const int nodes = 2 * range;
        m_up.assign(nodes, 0);
        m_left.assign(nodes, 0);
        m_right.assign(nodes, 0);
        m_code.assign(nodes, 0);
        m_weight.assign(nodes, 0);
        m_symbolNode.assign(range, 0);
        for (int i = 2; i < nodes; ++i) {
            m_up[i] = i / 2;
            m_weight[i] = 1;
        }
        for (int i = 1; i < range; ++i) {
            m_left[i] = 2 * i;
            m_right[i] = 2 * i + 1;
        }
        for (int i = 0; i < range; ++i) {
            m_code[i] = -1;
            m_code[range + i] = i;
            m_left[range + i] = -1;
            m_right[range + i] = -1;
            m_symbolNode[i] = range + i;
        }
        for (int i = range - 1; i > 0; --i) m_weight[i] = m_weight[2 * i] + m_weight[2 * i + 1];
        if (range >= 0x101 && range <= 0x1ff) {
            // The symbols 256 and 257, and the copies of one byte from two back and from four back.
            const int seen[4][2] = {{256, 1}, {257, 1}, {range - 3, 12}, {range - 2, 6}};
            for (const auto &each : seen)
                for (int i = 0; i < each[1]; ++i) update(m_symbolNode[each[0]]);
        } else {
            for (int pass = 0; pass < 2; ++pass)
                for (int s = 0; s < range; ++s) update(m_symbolNode[s]);
        }
    }

    // The next symbol, or -1 when the bits run out.
    int read(BitReader &bits)
    {
        int node = 1;
        for (int depth = 0; m_code[node] < 0; ++depth) {
            const int b = bits.bit();
            if (bits.failed || depth > 2 * m_range) return -1;
            node = b ? m_right[node] : m_left[node];
        }
        const int symbol = m_code[node];
        update(node);
        return symbol;
    }

private:
    void swapNodes(int a, int b)
    {
        std::swap(m_left[a], m_left[b]);
        std::swap(m_right[a], m_right[b]);
        std::swap(m_code[a], m_code[b]);
        std::swap(m_weight[a], m_weight[b]);
        for (const int n : {a, b}) {
            if (m_code[n] < 0) {
                m_up[m_left[n]] = n;
                m_up[m_right[n]] = n;
            } else {
                m_symbolNode[m_code[n]] = n;
            }
        }
    }
    void update(int node)
    {
        for (; node != 1; node = m_up[node]) {
            const qint64 weight = m_weight[node];
            int first = node - 1;
            if (m_weight[first] == weight) {
                do {
                    --first;
                } while (m_weight[first] == weight);
                ++first;
                if (first > 1) {
                    swapNodes(node, first);
                    node = first;
                }
            }
            m_weight[node] = weight + 1;
        }
        ++m_weight[1];
    }

    int m_range;
    std::vector<int> m_up, m_left, m_right, m_code, m_symbolNode;
    std::vector<qint64> m_weight;
};

// What LZCOMP starts with in the history the copies may reach into: 32 rows of 96 pairs (row, column), then each byte four times.
constexpr int kHistoryPrefix = 0x1c00;

void fillHistoryPrefix(quint8 *p)
{
    for (int row = 0; row < 32; ++row)
        for (int column = 0; column < 96; ++column) {
            *p++ = quint8(row);
            *p++ = quint8(column);
        }
    for (int b = 0; b < 256; ++b)
        for (int k = 0; k < 4; ++k) *p++ = quint8(b);
}

// One LZCOMP block. The first bit says whether the result is run-length coded (a marker byte, then runs written as marker, count, byte), then
// 24 bits give the length of what is decoded; each symbol is a byte, a copy of one byte from 2, 4, or 6 back, or a copy command: its number says
// how many distance symbols (3 bits each) follow and starts the length (2 bits a symbol, more symbols while the 3rd bit of each is set, plus 2);
// the distance counts back to the last byte copied, so the bytes come from that far plus the length less one back.
bool lzDecompress(const quint8 *data, qint64 size, Bytes *out, QString *why)
{
    out->clear();
    auto fail = [&](const char *reason) {
        if (why) *why = say(reason);
        return false;
    };
    if (size == 0) return true;
    BitReader bits{data, size};
    const bool runLength = bits.bit();
    const qint64 length = bits.value(24);
    if (bits.failed) return fail("The embedded font's data is cut off.");
    if (length > kMaxBlock) return fail("The embedded font is larger than 32 MB.");
    int distanceRanges = 1;
    if (length > 8)
        for (distanceRanges = 2; length > (qint64(1) << (3 * distanceRanges)); ++distanceRanges) {}
    const int dup2 = 256 + 8 * distanceRanges;
    AdaptiveHuffman symbols(dup2 + 3), lengths(8), distances(8);
    Bytes buffer(size_t(kHistoryPrefix + length));
    fillHistoryPrefix(buffer.data());
    quint8 *window = buffer.data() + kHistoryPrefix;
    qint64 at = 0;
    while (at < length) {
        const int symbol = symbols.read(bits);
        if (symbol < 0 || bits.failed) return fail("The embedded font's data is cut off.");
        if (symbol < 256) {
            window[at++] = quint8(symbol);
        } else if (symbol >= dup2) {
            window[at] = window[at - 2 * (symbol - dup2 + 1)];
            ++at;
        } else {
            const int distanceSymbols = (symbol - 256) / 8 + 1;
            int code = (symbol - 256) % 8;
            qint64 copy = 0;
            for (int chunks = 0;; ++chunks) {
                copy = copy << 2 | (code & 3);
                if (!(code & 4)) break;
                if (chunks >= 12) return fail("The embedded font is damaged.");
                code = lengths.read(bits);
                if (code < 0) return fail("The embedded font's data is cut off.");
            }
            copy += 2;
            qint64 distance = 0;
            for (int i = 0; i < distanceSymbols; ++i) {
                const int d = distances.read(bits);
                if (d < 0) return fail("The embedded font's data is cut off.");
                distance = distance << 3 | d;
            }
            distance += 1;
            if (distance > 0x1ff) ++copy;
            if (copy > length - at || distance + copy - 1 > at + kHistoryPrefix) return fail("The embedded font is damaged.");
            const quint8 *from = window + at - distance - copy + 1;
            for (qint64 i = 0; i < copy; ++i) window[at + i] = from[i];
            at += copy;
        }
    }
    if (!runLength) {
        out->assign(window, window + length);
        return true;
    }
    if (length < 1) return fail("The embedded font is damaged.");
    const quint8 marker = window[0];
    for (qint64 i = 1; i < length;) {
        const quint8 b = window[i++];
        if (b != marker) {
            out->push_back(b);
        } else {
            if (i >= length) return fail("The embedded font is damaged.");
            const int count = window[i++];
            if (count == 0) {
                out->push_back(marker);
            } else {
                if (i >= length) return fail("The embedded font is damaged.");
                out->insert(out->end(), size_t(count), window[i++]);
            }
        }
        if (qint64(out->size()) > kMaxBlock) return fail("The embedded font is larger than 32 MB.");
    }
    return true;
}

// Reads numbers and bytes from a block; a read past its end is remembered, and gives zeros.
struct Block {
    const Bytes *bytes;
    size_t position = 0;
    bool failed = false;
    quint8 u8()
    {
        if (position >= bytes->size()) {
            failed = true;
            return 0;
        }
        return (*bytes)[position++];
    }
    quint16 u16()
    {
        const quint16 hi = u8();
        return quint16(hi << 8 | u8());
    }
    qint16 s16() { return qint16(u16()); }
    // 255USHORT: a byte up to 252, else 253 and two bytes, 254 and a byte plus 506, or 255 and a byte plus 253.
    int u255()
    {
        const int code = u8();
        if (code == 253) return u16();
        if (code == 255) return 253 + u8();
        if (code == 254) return 506 + u8();
        return code;
    }
    bool take(size_t n, QByteArray *out)
    {
        if (bytes->size() - position < n) {
            failed = true;
            return false;
        }
        out->append(reinterpret_cast<const char *>(bytes->data() + position), qsizetype(n));
        position += n;
        return true;
    }
    bool atEnd() const { return position == bytes->size(); }
};

// A glyph program starts with a burst of pushes, which are kept apart as numbers: a byte up to 249; 250 and then the number, negated; 253 and a
// signed 16-bit number; 254 and a byte, plus 500; 255 and a byte, plus 250; and 251 and 252 stand for what comes back two numbers on: (A, x, 251,
// y) is A, x, A, y, A, and (A, x, 252, y, z) is A, x, A, y, A, z, A.
bool readPushedValues(Block &b, int count, std::vector<int> *values)
{
    values->clear();
    auto one = [&]() {
        int code = b.u8();
        bool negative = false;
        if (code == 250) {
            negative = true;
            code = b.u8();
        }
        int v;
        if (code == 253) v = b.s16();
        else if (code == 255) v = 250 + b.u8();
        else if (code == 254) v = 500 + b.u8();
        else v = code;
        return negative ? -v : v;
    };
    while (int(values->size()) < count && !b.failed) {
        const int code = b.position < b.bytes->size() ? (*b.bytes)[b.position] : 0;
        if (code == 251 || code == 252) {
            ++b.position;
            if (values->size() < 2) return false;
            values->push_back((*values)[values->size() - 2]);
            values->push_back(one());
            values->push_back((*values)[values->size() - 2]);
            if (code == 252) {
                values->push_back(one());
                values->push_back((*values)[values->size() - 2]);
            }
        } else {
            values->push_back(one());
        }
    }
    return !b.failed && int(values->size()) == count;
}

// The instructions that push the values, in the shortest runs the opcodes have: PUSHB or PUSHW up to 7 values, NPUSHB or NPUSHW up to 255; bytes
// (0 to 255) and words are in runs of their own.
void appendPushes(const std::vector<int> &values, QByteArray *out)
{
    for (size_t i = 0; i < values.size();) {
        const bool bytes = values[i] >= 0 && values[i] <= 255;
        size_t j = i;
        while (j < values.size() && j - i < 255 && (values[j] >= 0 && values[j] <= 255) == bytes) ++j;
        const size_t n = j - i;
        if (bytes) {
            if (n <= 7) out->append(char(0xB0 + n - 1));
            else out->append(char(0x40)).append(char(n));
            for (size_t k = i; k < j; ++k) out->append(char(values[k]));
        } else {
            if (n <= 7) out->append(char(0xB8 + n - 1));
            else out->append(char(0x41)).append(char(n));
            for (size_t k = i; k < j; ++k) {
                out->append(char(values[k] >> 8));
                out->append(char(values[k]));
            }
        }
        i = j;
    }
}

void appendU16(QByteArray *out, quint16 v)
{
    out->append(char(v >> 8));
    out->append(char(v & 0xff));
}

// A coordinate's change as a flag byte's index (0 to 127) says, with the bytes that follow it: indexes 0 to 9 give y alone from one byte, 10 to 19
// x alone, 20 to 83 both from the two halves of one byte, 84 to 119 both from two bytes, 120 to 123 both from three, and 124 to 127 both from four;
// the lowest bit of the index is x's sign (set, plus), the next y's.
bool readTriplet(Block &b, int index, int *dx, int *dy)
{
    auto withSign = [](int value, bool positive) { return positive ? value : -value; };
    const bool xPositive = index & 1, yPositive = index & 2;
    if (index < 10) {
        *dx = 0;
        *dy = withSign(((index & 14) << 7) + b.u8(), xPositive);
    } else if (index < 20) {
        *dx = withSign((((index - 10) & 14) << 7) + b.u8(), xPositive);
        *dy = 0;
    } else if (index < 84) {
        const int i = index - 20, byte = b.u8();
        *dx = withSign(1 + (i & 0x30) + (byte >> 4), xPositive);
        *dy = withSign(1 + ((i & 0x0c) << 2) + (byte & 15), yPositive);
    } else if (index < 120) {
        const int i = index - 84, first = b.u8(), second = b.u8();
        *dx = withSign(1 + (i / 12) * 256 + first, xPositive);
        *dy = withSign(1 + ((i % 12) / 4) * 256 + second, yPositive);
    } else if (index < 124) {
        const int first = b.u8(), second = b.u8(), third = b.u8();
        *dx = withSign((first << 4) + (second >> 4), xPositive);
        *dy = withSign(((second & 15) << 8) + third, yPositive);
    } else {
        const int first = b.u8(), second = b.u8(), third = b.u8(), fourth = b.u8();
        *dx = withSign((first << 8) + second, xPositive);
        *dy = withSign((third << 8) + fourth, yPositive);
    }
    return !b.failed;
}

// The glyphs of a translated glyf table, as TrueType glyph data (each padded to an even number of bytes), with where each starts. A glyph is its
// number of contours (0: none, and nothing else; -1: a composite; 0x7FFF: the box is written and the real number follows), then for a simple
// one the contours' point counts (the first's last point's number), a flag byte for each point (the high bit says off the curve), the
// coordinates, and the push and code sizes of its program; for a composite the box and the components as TrueType writes them, and the program's
// sizes if the last component has one.
bool readGlyphs(const Bytes &outlines, const Bytes &pushed, const Bytes &code, int glyphCount, QByteArray *glyf, std::vector<quint32> *starts, QString *why)
{
    Block r{&outlines}, pushes{&pushed}, instructions{&code};
    auto fail = [&](const char *reason) {
        if (why) *why = say(reason);
        return false;
    };
    for (int g = 0; g < glyphCount; ++g) {
        starts->push_back(quint32(glyf->size()));
        int contours = r.s16();
        QByteArray glyph;
        if (contours == 0) {
            if (r.failed) return fail("The embedded font is damaged.");
            continue;
        }
        QByteArray box;
        if (contours == -1) {
            if (!r.take(8, &box)) return fail("The embedded font is damaged.");
            QByteArray components;
            int flags;
            do {
                flags = r.u16();
                appendU16(&components, quint16(flags));
                r.take(2, &components);
                r.take((flags & 1) ? 4 : 2, &components);
                if (flags & 0x08) r.take(2, &components);
                else if (flags & 0x40) r.take(4, &components);
                else if (flags & 0x80) r.take(8, &components);
                if (r.failed) return fail("The embedded font is damaged.");
            } while (flags & 0x20);
            appendU16(&glyph, 0xFFFF);
            glyph += box + components;
            if (flags & 0x100) {
                const int pushCount = r.u255(), codeSize = r.u255();
                std::vector<int> values;
                QByteArray program;
                if (r.failed || !readPushedValues(pushes, pushCount, &values)) return fail("The embedded font is damaged.");
                appendPushes(values, &program);
                if (!instructions.take(size_t(codeSize), &program) || program.size() > 0xFFFF) return fail("The embedded font is damaged.");
                appendU16(&glyph, quint16(program.size()));
                glyph += program;
            }
        } else {
            bool haveBox = false;
            if (contours == 0x7FFF) {
                if (!r.take(8, &box)) return fail("The embedded font is damaged.");
                haveBox = true;
                contours = r.s16();
            }
            if (contours < 1) return fail("The embedded font is damaged.");
            std::vector<quint16> ends;
            int last = r.u255();
            if (last > 0xFFFF) return fail("The embedded font is damaged.");
            ends.push_back(quint16(last));
            for (int c = 1; c < contours; ++c) {
                const int count = r.u255();
                if (r.failed || count < 1 || last + count > 0xFFFF) return fail("The embedded font is damaged.");
                last += count;
                ends.push_back(quint16(last));
            }
            const int points = last + 1;
            QByteArray flagBytes;
            if (!r.take(size_t(points), &flagBytes)) return fail("The embedded font is damaged.");
            QByteArray flags, xs, ys;
            int x = 0, y = 0, xMin = 32767, yMin = 32767, xMax = -32768, yMax = -32768;
            for (int p = 0; p < points; ++p) {
                const quint8 f = quint8(flagBytes[p]);
                int dx, dy;
                if (!readTriplet(r, f & 0x7f, &dx, &dy)) return fail("The embedded font is damaged.");
                x += dx;
                y += dy;
                if (dx < -32768 || dx > 32767 || dy < -32768 || dy > 32767 || x < -32768 || x > 32767 || y < -32768 || y > 32767) return fail("The embedded font is damaged.");
                xMin = std::min(xMin, x);
                yMin = std::min(yMin, y);
                xMax = std::max(xMax, x);
                yMax = std::max(yMax, y);
                // TrueType's flag: on the curve; x and y as a byte (with the sign in the "same" bit) or a word, or not changed (except for the first point, as the program writes).
                int out = (f & 0x80) ? 0 : 1;
                if (dx == 0 && p > 0) {
                    out |= 0x10;
                } else if (dx >= -255 && dx <= 255) {
                    out |= 0x02 | (dx >= 0 ? 0x10 : 0);
                    xs.append(char(dx < 0 ? -dx : dx));
                } else {
                    appendU16(&xs, quint16(dx));
                }
                if (dy == 0 && p > 0) {
                    out |= 0x20;
                } else if (dy >= -255 && dy <= 255) {
                    out |= 0x04 | (dy >= 0 ? 0x20 : 0);
                    ys.append(char(dy < 0 ? -dy : dy));
                } else {
                    appendU16(&ys, quint16(dy));
                }
                flags.append(char(out));
            }
            const int pushCount = r.u255(), codeSize = r.u255();
            std::vector<int> values;
            QByteArray program;
            if (r.failed || !readPushedValues(pushes, pushCount, &values)) return fail("The embedded font is damaged.");
            appendPushes(values, &program);
            if (!instructions.take(size_t(codeSize), &program) || program.size() > 0xFFFF) return fail("The embedded font is damaged.");
            if (!haveBox) {
                appendU16(&box, quint16(xMin));
                appendU16(&box, quint16(yMin));
                appendU16(&box, quint16(xMax));
                appendU16(&box, quint16(yMax));
            }
            appendU16(&glyph, quint16(contours));
            glyph += box;
            for (quint16 e : ends) appendU16(&glyph, e);
            appendU16(&glyph, quint16(program.size()));
            glyph += program + flags + xs + ys;
        }
        if (glyph.size() % 2) glyph.append('\0');
        if (glyf->size() + glyph.size() > kMaxBlock) return fail("The embedded font is larger than 32 MB.");
        glyf->append(glyph);
    }
    if (r.failed || !r.atEnd()) return fail("The embedded font is damaged.");
    return true;
}

// The control values: a count, then each as the difference from the one before. A byte up to 237 is the difference; 238 and a signed 16-bit one;
// 239 to 247 and a byte is minus (238 times the code less 239, plus the byte); 248 to 255 and a byte is plus (238 times the code less 247, plus the byte).
bool readControlValues(const QByteArray &packed, QByteArray *out)
{
    const Bytes bytes(packed.begin(), packed.end());
    Block r{&bytes};
    const int count = r.u16();
    int value = 0;
    for (int i = 0; i < count; ++i) {
        const int code = r.u8();
        int delta;
        if (code < 238) delta = code;
        else if (code == 238) delta = r.s16();
        else if (code < 248) delta = -(238 * (code - 239) + r.u8());
        else delta = 238 * (code - 247) + r.u8();
        if (r.failed) return false;
        value = qint16(value + delta);
        appendU16(out, quint16(value));
    }
    return true;
}

quint32 checkSum(const QByteArray &data)
{
    quint32 sum = 0;
    const qsizetype n = data.size();
    for (qsizetype i = 0; i < n; i += 4) {
        quint32 word = 0;
        for (qsizetype k = 0; k < 4; ++k) word = word << 8 | (i + k < n ? quint8(data[i + k]) : 0);
        sum += word;
    }
    return sum;
}

} // namespace

bool mtxToTrueType(const QByteArray &mtx, QByteArray *font, QString *why)
{
    if (font) font->clear();
    auto fail = [&](const QString &reason) {
        if (why) *why = reason;
        return false;
    };
    auto damaged = [&] { return fail(say("The embedded font is damaged.")); };
    if (mtx.size() < 10) return fail(say("The embedded font is too short to be a font."));
    const auto u24 = [&](int at) { return qint64(quint8(mtx[at])) << 16 | quint8(mtx[at + 1]) << 8 | quint8(mtx[at + 2]); };
    if (quint8(mtx[0]) != 3) return fail(say("The embedded font is compressed in a way that is not read."));
    const qint64 second = u24(4), third = u24(7);
    if (second < 10 || third < second || third > mtx.size()) return damaged();
    const auto block = [&](qint64 from, qint64 to, Bytes *out) {
        return lzDecompress(reinterpret_cast<const quint8 *>(mtx.constData()) + from, to - from, out, why);
    };
    Bytes tables, pushed, code;
    if (!block(10, second, &tables) || !block(second, third, &pushed) || !block(third, mtx.size(), &code)) return false;

    // The table directory of block one, as a TrueType file has it.
    const QByteArray b1(reinterpret_cast<const char *>(tables.data()), qsizetype(tables.size()));
    if (b1.size() < 12 || qFromBigEndian<quint32>(b1.constData()) != 0x00010000) return damaged();
    const int count = qFromBigEndian<quint16>(b1.constData() + 4);
    if (count < 1 || count > kMaxFontTables || 12 + 16 * qint64(count) > b1.size()) return damaged();
    std::map<QByteArray, QByteArray> out;
    for (int i = 0; i < count; ++i) {
        const char *record = b1.constData() + 12 + 16 * i;
        const QByteArray tag(record, 4);
        const qint64 offset = qFromBigEndian<quint32>(record + 8), length = qFromBigEndian<quint32>(record + 12);
        if (offset + length > b1.size() || (length > 0 && offset < 12 + 16 * qint64(count)) || out.count(tag)) return damaged();
        out[tag] = b1.mid(offset, length);
    }
    for (const char *needed : {"head", "maxp", "glyf"})
        if (!out.count(needed)) return damaged();
    if (out["head"].size() < 54 || out["maxp"].size() < 6) return damaged();
    const int glyphCount = qFromBigEndian<quint16>(out["maxp"].constData() + 4);

    // The translated tables: the control values and the glyphs are made again, the glyph index too; the device metrics are left out.
    QByteArray glyf;
    std::vector<quint32> starts;
    const Bytes outlines(out["glyf"].begin(), out["glyf"].end());
    if (!readGlyphs(outlines, pushed, code, glyphCount, &glyf, &starts, why)) return false;
    starts.push_back(quint32(glyf.size()));
    out.erase("hdmx");
    out.erase("VDMX");
    if (out.count("cvt ")) {
        QByteArray values;
        if (!readControlValues(out["cvt "], &values)) return damaged();
        out["cvt "] = values;
    }
    const bool shortIndex = glyf.size() <= 0x1FFFE;
    QByteArray loca;
    for (quint32 start : starts) {
        if (shortIndex) appendU16(&loca, quint16(start / 2));
        else {
            appendU16(&loca, quint16(start >> 16));
            appendU16(&loca, quint16(start & 0xFFFF));
        }
    }
    out["glyf"] = glyf;
    out["loca"] = loca;
    QByteArray &head = out["head"];
    head[8] = head[9] = head[10] = head[11] = 0;
    head[50] = 0;
    head[51] = shortIndex ? 0 : 1;

    // The file: the offset table and the directory (sorted by tag), then each table on a four-byte boundary.
    const int n = int(out.size());
    int searchRange = 1, entrySelector = 0;
    while (searchRange * 2 <= n) {
        searchRange *= 2;
        ++entrySelector;
    }
    QByteArray file;
    file.append(char(0)).append(char(1)).append(char(0)).append(char(0));
    appendU16(&file, quint16(n));
    appendU16(&file, quint16(searchRange * 16));
    appendU16(&file, quint16(entrySelector));
    appendU16(&file, quint16(n * 16 - searchRange * 16));
    qint64 offset = 12 + 16 * qint64(n), total = offset;
    for (const auto &table : out) total += (table.second.size() + 3) & ~qsizetype(3);
    if (total > kMaxEmbeddedFontBytes) return fail(say("The embedded font is larger than 32 MB."));
    QByteArray body;
    qint64 headAt = 0;
    for (const auto &table : out) {
        if (table.first == "head") headAt = offset;
        file.append(table.first);
        for (quint32 v : {checkSum(table.second), quint32(offset), quint32(table.second.size())}) {
            appendU16(&file, quint16(v >> 16));
            appendU16(&file, quint16(v & 0xFFFF));
        }
        body.append(table.second);
        body.append(QByteArray((4 - table.second.size() % 4) % 4, '\0'));
        offset += (table.second.size() + 3) & ~qsizetype(3);
    }
    file.append(body);
    const quint32 adjustment = 0xB1B0AFBAu - checkSum(file);
    for (int i = 0; i < 4; ++i) file[qsizetype(headAt) + 8 + i] = char(adjustment >> (24 - 8 * i));
    if (font) *font = file;
    return true;
}

} // namespace jp::office
