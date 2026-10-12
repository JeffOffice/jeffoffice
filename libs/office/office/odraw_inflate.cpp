// Inflating (RFC 1951 deflate in an RFC 1950 zlib stream) for the pictures in
// odraw_blip.cpp: the canonical-code method of zlib's own small reference
// decoder, with the size of the output bounded.

#include "office/odraw_inflate.h"

namespace jp::office {

namespace {

constexpr int kMaxBits = 15;

struct Huffman {
    short count[kMaxBits + 1];   // codes of each length
    short symbol[288];           // symbols in order of code
};

struct Inflater {
    const uchar *in = nullptr;
    qsizetype size = 0, pos = 0;
    quint32 bitBuffer = 0;
    int bitCount = 0;
    bool bad = false;
    QByteArray out;
    qint64 limit = 0;

    int bits(int need)
    {
        quint32 value = bitBuffer;
        while (bitCount < need) {
            if (pos >= size) {
                bad = true;
                return 0;
            }
            value |= quint32(in[pos++]) << bitCount;
            bitCount += 8;
        }
        bitBuffer = value >> need;
        bitCount -= need;
        return int(value & ((1u << need) - 1));
    }

    int decode(const Huffman &h)
    {
        int code = 0, first = 0, index = 0;
        for (int len = 1; len <= kMaxBits; ++len) {
            code |= bits(1);
            if (bad) return -1;
            const int count = h.count[len];
            if (code - count < first) return h.symbol[index + (code - first)];
            index += count;
            first += count;
            first <<= 1;
            code <<= 1;
        }
        return -1;
    }

    // 0 when the code is complete, more when it leaves codes unused, less when over-subscribed.
    static int construct(Huffman &h, const short *length, int n)
    {
        for (int len = 0; len <= kMaxBits; ++len) h.count[len] = 0;
        for (int symbol = 0; symbol < n; ++symbol) ++h.count[length[symbol]];
        if (h.count[0] == n) return 0;
        int left = 1;
        for (int len = 1; len <= kMaxBits; ++len) {
            left <<= 1;
            left -= h.count[len];
            if (left < 0) return left;
        }
        short offs[kMaxBits + 1];
        offs[1] = 0;
        for (int len = 1; len < kMaxBits; ++len) offs[len + 1] = short(offs[len] + h.count[len]);
        for (int symbol = 0; symbol < n; ++symbol)
            if (length[symbol] != 0) h.symbol[offs[length[symbol]]++] = short(symbol);
        return left;
    }

    bool codes(const Huffman &lengthCode, const Huffman &distanceCode)
    {
        static const short lens[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
        static const short lext[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
        static const short dists[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
        static const short dext[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
        for (;;) {
            int symbol = decode(lengthCode);
            if (symbol < 0) return false;
            if (symbol < 256) {
                if (qint64(out.size()) >= limit) return false;
                out.append(char(symbol));
            } else if (symbol == 256) {
                return true;
            } else {
                symbol -= 257;
                if (symbol >= 29) return false;
                const int len = lens[symbol] + bits(lext[symbol]);
                if (bad) return false;
                symbol = decode(distanceCode);
                if (symbol < 0 || symbol >= 30) return false;
                const qint64 dist = dists[symbol] + bits(dext[symbol]);
                if (bad || dist > out.size() || qint64(out.size()) + len > limit) return false;
                const qsizetype from = out.size() - qsizetype(dist);
                for (int i = 0; i < len; ++i) out.append(out.at(from + i));
            }
        }
    }

    bool stored()
    {
        bitBuffer = 0;
        bitCount = 0;
        if (size - pos < 4) return false;
        const int len = in[pos] | (in[pos + 1] << 8), notLen = in[pos + 2] | (in[pos + 3] << 8);
        if (len != (~notLen & 0xFFFF)) return false;
        pos += 4;
        if (size - pos < len || qint64(out.size()) + len > limit) return false;
        out.append(reinterpret_cast<const char *>(in + pos), len);
        pos += len;
        return true;
    }

    bool fixedBlock()
    {
        static const struct Tables {
            Huffman lengthCode, distanceCode;
            Tables()
            {
                short lengths[288];
                int symbol = 0;
                for (; symbol < 144; ++symbol) lengths[symbol] = 8;
                for (; symbol < 256; ++symbol) lengths[symbol] = 9;
                for (; symbol < 280; ++symbol) lengths[symbol] = 7;
                for (; symbol < 288; ++symbol) lengths[symbol] = 8;
                Inflater::construct(lengthCode, lengths, 288);
                for (symbol = 0; symbol < 30; ++symbol) lengths[symbol] = 5;
                Inflater::construct(distanceCode, lengths, 30);
            }
        } tables;
        return codes(tables.lengthCode, tables.distanceCode);
    }

    bool dynamicBlock()
    {
        static const short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
        const int nlen = bits(5) + 257, ndist = bits(5) + 1, ncode = bits(4) + 4;
        if (bad || nlen > 286 || ndist > 30) return false;
        short lengths[320];
        int index = 0;
        for (; index < ncode; ++index) lengths[order[index]] = short(bits(3));
        for (; index < 19; ++index) lengths[order[index]] = 0;
        if (bad) return false;
        Huffman lengthCode, distanceCode;
        if (construct(lengthCode, lengths, 19) != 0) return false;
        index = 0;
        while (index < nlen + ndist) {
            int symbol = decode(lengthCode);
            if (symbol < 0) return false;
            if (symbol < 16) {
                lengths[index++] = short(symbol);
            } else {
                int len = 0;
                if (symbol == 16) {
                    if (index == 0) return false;
                    len = lengths[index - 1];
                    symbol = 3 + bits(2);
                } else if (symbol == 17) {
                    symbol = 3 + bits(3);
                } else {
                    symbol = 11 + bits(7);
                }
                if (bad || index + symbol > nlen + ndist) return false;
                while (symbol--) lengths[index++] = short(len);
            }
        }
        if (lengths[256] == 0) return false;
        int err = construct(lengthCode, lengths, nlen);
        if (err < 0 || (err > 0 && nlen - lengthCode.count[0] != 1)) return false;
        err = construct(distanceCode, lengths + nlen, ndist);
        if (err < 0 || (err > 0 && ndist - distanceCode.count[0] != 1)) return false;
        return codes(lengthCode, distanceCode);
    }

    bool run(qsizetype start)
    {
        pos = start;
        bitBuffer = 0;
        bitCount = 0;
        bad = false;
        out.clear();
        int last;
        do {
            last = bits(1);
            const int type = bits(2);
            if (bad) return false;
            const bool good = type == 0 ? stored() : type == 1 ? fixedBlock() : type == 2 ? dynamicBlock() : false;
            if (!good || bad) return false;
        } while (!last);
        return true;
    }
};

} // namespace

QByteArray odrawInflate(const QByteArray &in, qint64 limit, bool *ok)
{
    if (ok) *ok = false;
    Inflater z;
    z.in = reinterpret_cast<const uchar *>(in.constData());
    z.size = in.size();
    z.limit = limit;
    // With the zlib header first (method 8, a check value that divides by 31,
    // no preset dictionary), then as plain deflate data.
    const bool zlibHeader = in.size() >= 2 && (z.in[0] & 0x0F) == 8 && ((z.in[0] << 8) | z.in[1]) % 31 == 0 && !(z.in[1] & 0x20);
    if ((zlibHeader && z.run(2)) || z.run(0)) {
        if (ok) *ok = true;
        return z.out;
    }
    return QByteArray();
}

} // namespace jp::office
