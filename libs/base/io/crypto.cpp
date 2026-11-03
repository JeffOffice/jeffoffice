// The ciphers of crypto.h: AES from its published tables (generated at the first use, not typed in), RC4, and Blowfish (whose starting values are the digits of pi: the table below is
// the first 1,042 words of them, as the cipher's author published).

#include "io/crypto.h"

#include <QString>

#include <algorithm>
#include <cstring>
#include <new>
#include <vector>

namespace jp::crypto {

namespace {

// ---- AES

inline quint8 mul2(quint8 x) { return quint8((x << 1) ^ ((x & 0x80) ? 0x1B : 0)); }
inline quint8 gmul(quint8 a, quint8 b)
{
    quint8 r = 0;
    while (b) {
        if (b & 1) r ^= a;
        a = mul2(a);
        b >>= 1;
    }
    return r;
}
inline quint32 ror8(quint32 x) { return (x >> 8) | (x << 24); }

struct AesTables {
    quint8 sbox[256], inv[256];
    quint32 te[4][256], td[4][256];
    AesTables()
    {
        quint8 p = 1, q = 1;
        do {
            p = quint8(p ^ mul2(p));   // p times 3
            q ^= quint8(q << 1);       // q divided by 3
            q ^= quint8(q << 2);
            q ^= quint8(q << 4);
            if (q & 0x80) q ^= 0x09;
            const quint8 x = quint8(q ^ ((q << 1) | (q >> 7)) ^ ((q << 2) | (q >> 6)) ^ ((q << 3) | (q >> 5)) ^ ((q << 4) | (q >> 4)));
            sbox[p] = quint8(x ^ 0x63);
        } while (p != 1);
        sbox[0] = 0x63;
        for (int i = 0; i < 256; ++i) inv[sbox[i]] = quint8(i);
        for (int i = 0; i < 256; ++i) {
            const quint8 s = sbox[i];
            te[0][i] = (quint32(mul2(s)) << 24) | (quint32(s) << 16) | (quint32(s) << 8) | quint32(mul2(s) ^ s);
            const quint8 v = inv[i];
            td[0][i] = (quint32(gmul(v, 14)) << 24) | (quint32(gmul(v, 9)) << 16) | (quint32(gmul(v, 13)) << 8) | quint32(gmul(v, 11));
            for (int k = 1; k < 4; ++k) {
                te[k][i] = ror8(te[k - 1][i]);
                td[k][i] = ror8(td[k - 1][i]);
            }
        }
    }
};

const AesTables &aesTables()
{
    static const AesTables t;
    return t;
}

inline quint32 load32(const quint8 *p) { return (quint32(p[0]) << 24) | (quint32(p[1]) << 16) | (quint32(p[2]) << 8) | p[3]; }
inline void store32(quint8 *p, quint32 v)
{
    p[0] = quint8(v >> 24);
    p[1] = quint8(v >> 16);
    p[2] = quint8(v >> 8);
    p[3] = quint8(v);
}

struct AesKey {
    int rounds = 0;
    quint32 enc[60], dec[60];
    bool set(const QByteArray &key)
    {
        if (key.size() != 16 && key.size() != 24 && key.size() != 32) return false;
        const int nk = int(key.size() / 4);
        const AesTables &t = aesTables();
        rounds = nk + 6;
        const quint8 *k = reinterpret_cast<const quint8 *>(key.constData());
        for (int i = 0; i < nk; ++i) enc[i] = load32(k + 4 * i);
        quint8 rcon = 1;
        for (int i = nk; i < 4 * (rounds + 1); ++i) {
            quint32 w = enc[i - 1];
            if (i % nk == 0) {
                w = (w << 8) | (w >> 24);
                w = (quint32(t.sbox[w >> 24]) << 24) | (quint32(t.sbox[(w >> 16) & 0xFF]) << 16) | (quint32(t.sbox[(w >> 8) & 0xFF]) << 8) | t.sbox[w & 0xFF];
                w ^= quint32(rcon) << 24;
                rcon = mul2(rcon);
            } else if (nk > 6 && i % nk == 4) {
                w = (quint32(t.sbox[w >> 24]) << 24) | (quint32(t.sbox[(w >> 16) & 0xFF]) << 16) | (quint32(t.sbox[(w >> 8) & 0xFF]) << 8) | t.sbox[w & 0xFF];
            }
            enc[i] = enc[i - nk] ^ w;
        }
        // the decryption schedule: the round keys in reverse, the middle ones through the inverse column mix
        for (int r = 0; r <= rounds; ++r) {
            for (int c = 0; c < 4; ++c) {
                quint32 w = enc[4 * (rounds - r) + c];
                if (r != 0 && r != rounds)
                    w = t.td[0][t.sbox[w >> 24]] ^ t.td[1][t.sbox[(w >> 16) & 0xFF]] ^ t.td[2][t.sbox[(w >> 8) & 0xFF]] ^ t.td[3][t.sbox[w & 0xFF]];
                dec[4 * r + c] = w;
            }
        }
        return true;
    }
    void encryptBlock(const quint8 *in, quint8 *out) const
    {
        const AesTables &t = aesTables();
        quint32 s0 = load32(in) ^ enc[0], s1 = load32(in + 4) ^ enc[1], s2 = load32(in + 8) ^ enc[2], s3 = load32(in + 12) ^ enc[3];
        const quint32 *rk = enc + 4;
        for (int r = 1; r < rounds; ++r, rk += 4) {
            const quint32 t0 = t.te[0][s0 >> 24] ^ t.te[1][(s1 >> 16) & 0xFF] ^ t.te[2][(s2 >> 8) & 0xFF] ^ t.te[3][s3 & 0xFF] ^ rk[0];
            const quint32 t1 = t.te[0][s1 >> 24] ^ t.te[1][(s2 >> 16) & 0xFF] ^ t.te[2][(s3 >> 8) & 0xFF] ^ t.te[3][s0 & 0xFF] ^ rk[1];
            const quint32 t2 = t.te[0][s2 >> 24] ^ t.te[1][(s3 >> 16) & 0xFF] ^ t.te[2][(s0 >> 8) & 0xFF] ^ t.te[3][s1 & 0xFF] ^ rk[2];
            const quint32 t3 = t.te[0][s3 >> 24] ^ t.te[1][(s0 >> 16) & 0xFF] ^ t.te[2][(s1 >> 8) & 0xFF] ^ t.te[3][s2 & 0xFF] ^ rk[3];
            s0 = t0, s1 = t1, s2 = t2, s3 = t3;
        }
        auto last = [&](quint32 a, quint32 b, quint32 c, quint32 d, quint32 k) {
            return ((quint32(t.sbox[a >> 24]) << 24) | (quint32(t.sbox[(b >> 16) & 0xFF]) << 16) | (quint32(t.sbox[(c >> 8) & 0xFF]) << 8) | t.sbox[d & 0xFF]) ^ k;
        };
        store32(out, last(s0, s1, s2, s3, rk[0]));
        store32(out + 4, last(s1, s2, s3, s0, rk[1]));
        store32(out + 8, last(s2, s3, s0, s1, rk[2]));
        store32(out + 12, last(s3, s0, s1, s2, rk[3]));
    }
    void decryptBlock(const quint8 *in, quint8 *out) const
    {
        const AesTables &t = aesTables();
        quint32 s0 = load32(in) ^ dec[0], s1 = load32(in + 4) ^ dec[1], s2 = load32(in + 8) ^ dec[2], s3 = load32(in + 12) ^ dec[3];
        const quint32 *rk = dec + 4;
        for (int r = 1; r < rounds; ++r, rk += 4) {
            const quint32 t0 = t.td[0][s0 >> 24] ^ t.td[1][(s3 >> 16) & 0xFF] ^ t.td[2][(s2 >> 8) & 0xFF] ^ t.td[3][s1 & 0xFF] ^ rk[0];
            const quint32 t1 = t.td[0][s1 >> 24] ^ t.td[1][(s0 >> 16) & 0xFF] ^ t.td[2][(s3 >> 8) & 0xFF] ^ t.td[3][s2 & 0xFF] ^ rk[1];
            const quint32 t2 = t.td[0][s2 >> 24] ^ t.td[1][(s1 >> 16) & 0xFF] ^ t.td[2][(s0 >> 8) & 0xFF] ^ t.td[3][s3 & 0xFF] ^ rk[2];
            const quint32 t3 = t.td[0][s3 >> 24] ^ t.td[1][(s2 >> 16) & 0xFF] ^ t.td[2][(s1 >> 8) & 0xFF] ^ t.td[3][s0 & 0xFF] ^ rk[3];
            s0 = t0, s1 = t1, s2 = t2, s3 = t3;
        }
        auto last = [&](quint32 a, quint32 b, quint32 c, quint32 d, quint32 k) {
            return ((quint32(t.inv[a >> 24]) << 24) | (quint32(t.inv[(b >> 16) & 0xFF]) << 16) | (quint32(t.inv[(c >> 8) & 0xFF]) << 8) | t.inv[d & 0xFF]) ^ k;
        };
        store32(out, last(s0, s3, s2, s1, rk[0]));
        store32(out + 4, last(s1, s0, s3, s2, rk[1]));
        store32(out + 8, last(s2, s1, s0, s3, rk[2]));
        store32(out + 12, last(s3, s2, s1, s0, rk[3]));
    }
};

// ---- Blowfish

const quint32 kBlowfishInit[1042] = {
    0x243F6A88, 0x85A308D3, 0x13198A2E, 0x03707344, 0xA4093822, 0x299F31D0,
    0x082EFA98, 0xEC4E6C89, 0x452821E6, 0x38D01377, 0xBE5466CF, 0x34E90C6C,
    0xC0AC29B7, 0xC97C50DD, 0x3F84D5B5, 0xB5470917, 0x9216D5D9, 0x8979FB1B,
    0xD1310BA6, 0x98DFB5AC, 0x2FFD72DB, 0xD01ADFB7, 0xB8E1AFED, 0x6A267E96,
    0xBA7C9045, 0xF12C7F99, 0x24A19947, 0xB3916CF7, 0x0801F2E2, 0x858EFC16,
    0x636920D8, 0x71574E69, 0xA458FEA3, 0xF4933D7E, 0x0D95748F, 0x728EB658,
    0x718BCD58, 0x82154AEE, 0x7B54A41D, 0xC25A59B5, 0x9C30D539, 0x2AF26013,
    0xC5D1B023, 0x286085F0, 0xCA417918, 0xB8DB38EF, 0x8E79DCB0, 0x603A180E,
    0x6C9E0E8B, 0xB01E8A3E, 0xD71577C1, 0xBD314B27, 0x78AF2FDA, 0x55605C60,
    0xE65525F3, 0xAA55AB94, 0x57489862, 0x63E81440, 0x55CA396A, 0x2AAB10B6,
    0xB4CC5C34, 0x1141E8CE, 0xA15486AF, 0x7C72E993, 0xB3EE1411, 0x636FBC2A,
    0x2BA9C55D, 0x741831F6, 0xCE5C3E16, 0x9B87931E, 0xAFD6BA33, 0x6C24CF5C,
    0x7A325381, 0x28958677, 0x3B8F4898, 0x6B4BB9AF, 0xC4BFE81B, 0x66282193,
    0x61D809CC, 0xFB21A991, 0x487CAC60, 0x5DEC8032, 0xEF845D5D, 0xE98575B1,
    0xDC262302, 0xEB651B88, 0x23893E81, 0xD396ACC5, 0x0F6D6FF3, 0x83F44239,
    0x2E0B4482, 0xA4842004, 0x69C8F04A, 0x9E1F9B5E, 0x21C66842, 0xF6E96C9A,
    0x670C9C61, 0xABD388F0, 0x6A51A0D2, 0xD8542F68, 0x960FA728, 0xAB5133A3,
    0x6EEF0B6C, 0x137A3BE4, 0xBA3BF050, 0x7EFB2A98, 0xA1F1651D, 0x39AF0176,
    0x66CA593E, 0x82430E88, 0x8CEE8619, 0x456F9FB4, 0x7D84A5C3, 0x3B8B5EBE,
    0xE06F75D8, 0x85C12073, 0x401A449F, 0x56C16AA6, 0x4ED3AA62, 0x363F7706,
    0x1BFEDF72, 0x429B023D, 0x37D0D724, 0xD00A1248, 0xDB0FEAD3, 0x49F1C09B,
    0x075372C9, 0x80991B7B, 0x25D479D8, 0xF6E8DEF7, 0xE3FE501A, 0xB6794C3B,
    0x976CE0BD, 0x04C006BA, 0xC1A94FB6, 0x409F60C4, 0x5E5C9EC2, 0x196A2463,
    0x68FB6FAF, 0x3E6C53B5, 0x1339B2EB, 0x3B52EC6F, 0x6DFC511F, 0x9B30952C,
    0xCC814544, 0xAF5EBD09, 0xBEE3D004, 0xDE334AFD, 0x660F2807, 0x192E4BB3,
    0xC0CBA857, 0x45C8740F, 0xD20B5F39, 0xB9D3FBDB, 0x5579C0BD, 0x1A60320A,
    0xD6A100C6, 0x402C7279, 0x679F25FE, 0xFB1FA3CC, 0x8EA5E9F8, 0xDB3222F8,
    0x3C7516DF, 0xFD616B15, 0x2F501EC8, 0xAD0552AB, 0x323DB5FA, 0xFD238760,
    0x53317B48, 0x3E00DF82, 0x9E5C57BB, 0xCA6F8CA0, 0x1A87562E, 0xDF1769DB,
    0xD542A8F6, 0x287EFFC3, 0xAC6732C6, 0x8C4F5573, 0x695B27B0, 0xBBCA58C8,
    0xE1FFA35D, 0xB8F011A0, 0x10FA3D98, 0xFD2183B8, 0x4AFCB56C, 0x2DD1D35B,
    0x9A53E479, 0xB6F84565, 0xD28E49BC, 0x4BFB9790, 0xE1DDF2DA, 0xA4CB7E33,
    0x62FB1341, 0xCEE4C6E8, 0xEF20CADA, 0x36774C01, 0xD07E9EFE, 0x2BF11FB4,
    0x95DBDA4D, 0xAE909198, 0xEAAD8E71, 0x6B93D5A0, 0xD08ED1D0, 0xAFC725E0,
    0x8E3C5B2F, 0x8E7594B7, 0x8FF6E2FB, 0xF2122B64, 0x8888B812, 0x900DF01C,
    0x4FAD5EA0, 0x688FC31C, 0xD1CFF191, 0xB3A8C1AD, 0x2F2F2218, 0xBE0E1777,
    0xEA752DFE, 0x8B021FA1, 0xE5A0CC0F, 0xB56F74E8, 0x18ACF3D6, 0xCE89E299,
    0xB4A84FE0, 0xFD13E0B7, 0x7CC43B81, 0xD2ADA8D9, 0x165FA266, 0x80957705,
    0x93CC7314, 0x211A1477, 0xE6AD2065, 0x77B5FA86, 0xC75442F5, 0xFB9D35CF,
    0xEBCDAF0C, 0x7B3E89A0, 0xD6411BD3, 0xAE1E7E49, 0x00250E2D, 0x2071B35E,
    0x226800BB, 0x57B8E0AF, 0x2464369B, 0xF009B91E, 0x5563911D, 0x59DFA6AA,
    0x78C14389, 0xD95A537F, 0x207D5BA2, 0x02E5B9C5, 0x83260376, 0x6295CFA9,
    0x11C81968, 0x4E734A41, 0xB3472DCA, 0x7B14A94A, 0x1B510052, 0x9A532915,
    0xD60F573F, 0xBC9BC6E4, 0x2B60A476, 0x81E67400, 0x08BA6FB5, 0x571BE91F,
    0xF296EC6B, 0x2A0DD915, 0xB6636521, 0xE7B9F9B6, 0xFF34052E, 0xC5855664,
    0x53B02D5D, 0xA99F8FA1, 0x08BA4799, 0x6E85076A, 0x4B7A70E9, 0xB5B32944,
    0xDB75092E, 0xC4192623, 0xAD6EA6B0, 0x49A7DF7D, 0x9CEE60B8, 0x8FEDB266,
    0xECAA8C71, 0x699A17FF, 0x5664526C, 0xC2B19EE1, 0x193602A5, 0x75094C29,
    0xA0591340, 0xE4183A3E, 0x3F54989A, 0x5B429D65, 0x6B8FE4D6, 0x99F73FD6,
    0xA1D29C07, 0xEFE830F5, 0x4D2D38E6, 0xF0255DC1, 0x4CDD2086, 0x8470EB26,
    0x6382E9C6, 0x021ECC5E, 0x09686B3F, 0x3EBAEFC9, 0x3C971814, 0x6B6A70A1,
    0x687F3584, 0x52A0E286, 0xB79C5305, 0xAA500737, 0x3E07841C, 0x7FDEAE5C,
    0x8E7D44EC, 0x5716F2B8, 0xB03ADA37, 0xF0500C0D, 0xF01C1F04, 0x0200B3FF,
    0xAE0CF51A, 0x3CB574B2, 0x25837A58, 0xDC0921BD, 0xD19113F9, 0x7CA92FF6,
    0x94324773, 0x22F54701, 0x3AE5E581, 0x37C2DADC, 0xC8B57634, 0x9AF3DDA7,
    0xA9446146, 0x0FD0030E, 0xECC8C73E, 0xA4751E41, 0xE238CD99, 0x3BEA0E2F,
    0x3280BBA1, 0x183EB331, 0x4E548B38, 0x4F6DB908, 0x6F420D03, 0xF60A04BF,
    0x2CB81290, 0x24977C79, 0x5679B072, 0xBCAF89AF, 0xDE9A771F, 0xD9930810,
    0xB38BAE12, 0xDCCF3F2E, 0x5512721F, 0x2E6B7124, 0x501ADDE6, 0x9F84CD87,
    0x7A584718, 0x7408DA17, 0xBC9F9ABC, 0xE94B7D8C, 0xEC7AEC3A, 0xDB851DFA,
    0x63094366, 0xC464C3D2, 0xEF1C1847, 0x3215D908, 0xDD433B37, 0x24C2BA16,
    0x12A14D43, 0x2A65C451, 0x50940002, 0x133AE4DD, 0x71DFF89E, 0x10314E55,
    0x81AC77D6, 0x5F11199B, 0x043556F1, 0xD7A3C76B, 0x3C11183B, 0x5924A509,
    0xF28FE6ED, 0x97F1FBFA, 0x9EBABF2C, 0x1E153C6E, 0x86E34570, 0xEAE96FB1,
    0x860E5E0A, 0x5A3E2AB3, 0x771FE71C, 0x4E3D06FA, 0x2965DCB9, 0x99E71D0F,
    0x803E89D6, 0x5266C825, 0x2E4CC978, 0x9C10B36A, 0xC6150EBA, 0x94E2EA78,
    0xA5FC3C53, 0x1E0A2DF4, 0xF2F74EA7, 0x361D2B3D, 0x1939260F, 0x19C27960,
    0x5223A708, 0xF71312B6, 0xEBADFE6E, 0xEAC31F66, 0xE3BC4595, 0xA67BC883,
    0xB17F37D1, 0x018CFF28, 0xC332DDEF, 0xBE6C5AA5, 0x65582185, 0x68AB9802,
    0xEECEA50F, 0xDB2F953B, 0x2AEF7DAD, 0x5B6E2F84, 0x1521B628, 0x29076170,
    0xECDD4775, 0x619F1510, 0x13CCA830, 0xEB61BD96, 0x0334FE1E, 0xAA0363CF,
    0xB5735C90, 0x4C70A239, 0xD59E9E0B, 0xCBAADE14, 0xEECC86BC, 0x60622CA7,
    0x9CAB5CAB, 0xB2F3846E, 0x648B1EAF, 0x19BDF0CA, 0xA02369B9, 0x655ABB50,
    0x40685A32, 0x3C2AB4B3, 0x319EE9D5, 0xC021B8F7, 0x9B540B19, 0x875FA099,
    0x95F7997E, 0x623D7DA8, 0xF837889A, 0x97E32D77, 0x11ED935F, 0x16681281,
    0x0E358829, 0xC7E61FD6, 0x96DEDFA1, 0x7858BA99, 0x57F584A5, 0x1B227263,
    0x9B83C3FF, 0x1AC24696, 0xCDB30AEB, 0x532E3054, 0x8FD948E4, 0x6DBC3128,
    0x58EBF2EF, 0x34C6FFEA, 0xFE28ED61, 0xEE7C3C73, 0x5D4A14D9, 0xE864B7E3,
    0x42105D14, 0x203E13E0, 0x45EEE2B6, 0xA3AAABEA, 0xDB6C4F15, 0xFACB4FD0,
    0xC742F442, 0xEF6ABBB5, 0x654F3B1D, 0x41CD2105, 0xD81E799E, 0x86854DC7,
    0xE44B476A, 0x3D816250, 0xCF62A1F2, 0x5B8D2646, 0xFC8883A0, 0xC1C7B6A3,
    0x7F1524C3, 0x69CB7492, 0x47848A0B, 0x5692B285, 0x095BBF00, 0xAD19489D,
    0x1462B174, 0x23820E00, 0x58428D2A, 0x0C55F5EA, 0x1DADF43E, 0x233F7061,
    0x3372F092, 0x8D937E41, 0xD65FECF1, 0x6C223BDB, 0x7CDE3759, 0xCBEE7460,
    0x4085F2A7, 0xCE77326E, 0xA6078084, 0x19F8509E, 0xE8EFD855, 0x61D99735,
    0xA969A7AA, 0xC50C06C2, 0x5A04ABFC, 0x800BCADC, 0x9E447A2E, 0xC3453484,
    0xFDD56705, 0x0E1E9EC9, 0xDB73DBD3, 0x105588CD, 0x675FDA79, 0xE3674340,
    0xC5C43465, 0x713E38D8, 0x3D28F89E, 0xF16DFF20, 0x153E21E7, 0x8FB03D4A,
    0xE6E39F2B, 0xDB83ADF7, 0xE93D5A68, 0x948140F7, 0xF64C261C, 0x94692934,
    0x411520F7, 0x7602D4F7, 0xBCF46B2E, 0xD4A20068, 0xD4082471, 0x3320F46A,
    0x43B7D4B7, 0x500061AF, 0x1E39F62E, 0x97244546, 0x14214F74, 0xBF8B8840,
    0x4D95FC1D, 0x96B591AF, 0x70F4DDD3, 0x66A02F45, 0xBFBC09EC, 0x03BD9785,
    0x7FAC6DD0, 0x31CB8504, 0x96EB27B3, 0x55FD3941, 0xDA2547E6, 0xABCA0A9A,
    0x28507825, 0x530429F4, 0x0A2C86DA, 0xE9B66DFB, 0x68DC1462, 0xD7486900,
    0x680EC0A4, 0x27A18DEE, 0x4F3FFEA2, 0xE887AD8C, 0xB58CE006, 0x7AF4D6B6,
    0xAACE1E7C, 0xD3375FEC, 0xCE78A399, 0x406B2A42, 0x20FE9E35, 0xD9F385B9,
    0xEE39D7AB, 0x3B124E8B, 0x1DC9FAF7, 0x4B6D1856, 0x26A36631, 0xEAE397B2,
    0x3A6EFA74, 0xDD5B4332, 0x6841E7F7, 0xCA7820FB, 0xFB0AF54E, 0xD8FEB397,
    0x454056AC, 0xBA489527, 0x55533A3A, 0x20838D87, 0xFE6BA9B7, 0xD096954B,
    0x55A867BC, 0xA1159A58, 0xCCA92963, 0x99E1DB33, 0xA62A4A56, 0x3F3125F9,
    0x5EF47E1C, 0x9029317C, 0xFDF8E802, 0x04272F70, 0x80BB155C, 0x05282CE3,
    0x95C11548, 0xE4C66D22, 0x48C1133F, 0xC70F86DC, 0x07F9C9EE, 0x41041F0F,
    0x404779A4, 0x5D886E17, 0x325F51EB, 0xD59BC0D1, 0xF2BCC18F, 0x41113564,
    0x257B7834, 0x602A9C60, 0xDFF8E8A3, 0x1F636C1B, 0x0E12B4C2, 0x02E1329E,
    0xAF664FD1, 0xCAD18115, 0x6B2395E0, 0x333E92E1, 0x3B240B62, 0xEEBEB922,
    0x85B2A20E, 0xE6BA0D99, 0xDE720C8C, 0x2DA2F728, 0xD0127845, 0x95B794FD,
    0x647D0862, 0xE7CCF5F0, 0x5449A36F, 0x877D48FA, 0xC39DFD27, 0xF33E8D1E,
    0x0A476341, 0x992EFF74, 0x3A6F6EAB, 0xF4F8FD37, 0xA812DC60, 0xA1EBDDF8,
    0x991BE14C, 0xDB6E6B0D, 0xC67B5510, 0x6D672C37, 0x2765D43B, 0xDCD0E804,
    0xF1290DC7, 0xCC00FFA3, 0xB5390F92, 0x690FED0B, 0x667B9FFB, 0xCEDB7D9C,
    0xA091CF0B, 0xD9155EA3, 0xBB132F88, 0x515BAD24, 0x7B9479BF, 0x763BD6EB,
    0x37392EB3, 0xCC115979, 0x8026E297, 0xF42E312D, 0x6842ADA7, 0xC66A2B3B,
    0x12754CCC, 0x782EF11C, 0x6A124237, 0xB79251E7, 0x06A1BBE6, 0x4BFB6350,
    0x1A6B1018, 0x11CAEDFA, 0x3D25BDD8, 0xE2E1C3C9, 0x44421659, 0x0A121386,
    0xD90CEC6E, 0xD5ABEA2A, 0x64AF674E, 0xDA86A85F, 0xBEBFE988, 0x64E4C3FE,
    0x9DBC8057, 0xF0F7C086, 0x60787BF8, 0x6003604D, 0xD1FD8346, 0xF6381FB0,
    0x7745AE04, 0xD736FCCC, 0x83426B33, 0xF01EAB71, 0xB0804187, 0x3C005E5F,
    0x77A057BE, 0xBDE8AE24, 0x55464299, 0xBF582E61, 0x4E58F48F, 0xF2DDFDA2,
    0xF474EF38, 0x8789BDC2, 0x5366F9C3, 0xC8B38E74, 0xB475F255, 0x46FCD9B9,
    0x7AEB2661, 0x8B1DDF84, 0x846A0E79, 0x915F95E2, 0x466E598E, 0x20B45770,
    0x8CD55591, 0xC902DE4C, 0xB90BACE1, 0xBB8205D0, 0x11A86248, 0x7574A99E,
    0xB77F19B6, 0xE0A9DC09, 0x662D09A1, 0xC4324633, 0xE85A1F02, 0x09F0BE8C,
    0x4A99A025, 0x1D6EFE10, 0x1AB93D1D, 0x0BA5A4DF, 0xA186F20F, 0x2868F169,
    0xDCB7DA83, 0x573906FE, 0xA1E2CE9B, 0x4FCD7F52, 0x50115E01, 0xA70683FA,
    0xA002B5C4, 0x0DE6D027, 0x9AF88C27, 0x773F8641, 0xC3604C06, 0x61A806B5,
    0xF0177A28, 0xC0F586E0, 0x006058AA, 0x30DC7D62, 0x11E69ED7, 0x2338EA63,
    0x53C2DD94, 0xC2C21634, 0xBBCBEE56, 0x90BCB6DE, 0xEBFC7DA1, 0xCE591D76,
    0x6F05E409, 0x4B7C0188, 0x39720A3D, 0x7C927C24, 0x86E3725F, 0x724D9DB9,
    0x1AC15BB4, 0xD39EB8FC, 0xED545578, 0x08FCA5B5, 0xD83D7CD3, 0x4DAD0FC4,
    0x1E50EF5E, 0xB161E6F8, 0xA28514D9, 0x6C51133C, 0x6FD5C7E7, 0x56E14EC4,
    0x362ABFCE, 0xDDC6C837, 0xD79A3234, 0x92638212, 0x670EFA8E, 0x406000E0,
    0x3A39CE37, 0xD3FAF5CF, 0xABC27737, 0x5AC52D1B, 0x5CB0679E, 0x4FA33742,
    0xD3822740, 0x99BC9BBE, 0xD5118E9D, 0xBF0F7315, 0xD62D1C7E, 0xC700C47B,
    0xB78C1B6B, 0x21A19045, 0xB26EB1BE, 0x6A366EB4, 0x5748AB2F, 0xBC946E79,
    0xC6A376D2, 0x6549C2C8, 0x530FF8EE, 0x468DDE7D, 0xD5730A1D, 0x4CD04DC6,
    0x2939BBDB, 0xA9BA4650, 0xAC9526E8, 0xBE5EE304, 0xA1FAD5F0, 0x6A2D519A,
    0x63EF8CE2, 0x9A86EE22, 0xC089C2B8, 0x43242EF6, 0xA51E03AA, 0x9CF2D0A4,
    0x83C061BA, 0x9BE96A4D, 0x8FE51550, 0xBA645BD6, 0x2826A2F9, 0xA73A3AE1,
    0x4BA99586, 0xEF5562E9, 0xC72FEFD3, 0xF752F7DA, 0x3F046F69, 0x77FA0A59,
    0x80E4A915, 0x87B08601, 0x9B09E6AD, 0x3B3EE593, 0xE990FD5A, 0x9E34D797,
    0x2CF0B7D9, 0x022B8B51, 0x96D5AC3A, 0x017DA67D, 0xD1CF3ED6, 0x7C7D2D28,
    0x1F9F25CF, 0xADF2B89B, 0x5AD6B472, 0x5A88F54C, 0xE029AC71, 0xE019A5E6,
    0x47B0ACFD, 0xED93FA9B, 0xE8D3C48D, 0x283B57CC, 0xF8D56629, 0x79132E28,
    0x785F0191, 0xED756055, 0xF7960E44, 0xE3D35E8C, 0x15056DD4, 0x88F46DBA,
    0x03A16125, 0x0564F0BD, 0xC3EB9E15, 0x3C9057A2, 0x97271AEC, 0xA93A072A,
    0x1B3F6D9B, 0x1E6321F5, 0xF59C66FB, 0x26DCF319, 0x7533D928, 0xB155FDF5,
    0x03563482, 0x8ABA3CBB, 0x28517711, 0xC20AD9F8, 0xABCC5167, 0xCCAD925F,
    0x4DE81751, 0x3830DC8E, 0x379D5862, 0x9320F991, 0xEA7A90C2, 0xFB3E7BCE,
    0x5121CE64, 0x774FBE32, 0xA8B6E37E, 0xC3293D46, 0x48DE5369, 0x6413E680,
    0xA2AE0810, 0xDD6DB224, 0x69852DFD, 0x09072166, 0xB39A460A, 0x6445C0DD,
    0x586CDECF, 0x1C20C8AE, 0x5BBEF7DD, 0x1B588D40, 0xCCD2017F, 0x6BB4E3BB,
    0xDDA26A7E, 0x3A59FF45, 0x3E350A44, 0xBCB4CDD5, 0x72EACEA8, 0xFA6484BB,
    0x8D6612AE, 0xBF3C6F47, 0xD29BE463, 0x542F5D9E, 0xAEC2771B, 0xF64E6370,
    0x740E0D8D, 0xE75B1357, 0xF8721671, 0xAF537D5D, 0x4040CB08, 0x4EB4E2CC,
    0x34D2466A, 0x0115AF84, 0xE1B00428, 0x95983A1D, 0x06B89FB4, 0xCE6EA048,
    0x6F3F3B82, 0x3520AB82, 0x011A1D4B, 0x277227F8, 0x611560B1, 0xE7933FDC,
    0xBB3A792B, 0x344525BD, 0xA08839E1, 0x51CE794B, 0x2F32C9B7, 0xA01FBAC9,
    0xE01CC87E, 0xBCC7D1F6, 0xCF0111C3, 0xA1E8AAC7, 0x1A908749, 0xD44FBD9A,
    0xD0DADECB, 0xD50ADA38, 0x0339C32A, 0xC6913667, 0x8DF9317C, 0xE0B12B4F,
    0xF79E59B7, 0x43F5BB3A, 0xF2D519FF, 0x27D9459C, 0xBF97222C, 0x15E6FC2A,
    0x0F91FC71, 0x9B941525, 0xFAE59361, 0xCEB69CEB, 0xC2A86459, 0x12BAA8D1,
    0xB6C1075E, 0xE3056A0C, 0x10D25065, 0xCB03A442, 0xE0EC6E0E, 0x1698DB3B,
    0x4C98A0BE, 0x3278E964, 0x9F1F9532, 0xE0D392DF, 0xD3A0342B, 0x8971F21E,
    0x1B0A7441, 0x4BA3348C, 0xC5BE7120, 0xC37632D8, 0xDF359F8D, 0x9B992F2E,
    0xE60B6F47, 0x0FE3F11D, 0xE54CDA54, 0x1EDAD891, 0xCE6279CF, 0xCD3E7E6F,
    0x1618B166, 0xFD2C1D05, 0x848FD2C5, 0xF6FB2299, 0xF523F357, 0xA6327623,
    0x93A83531, 0x56CCCD02, 0xACF08162, 0x5A75EBB5, 0x6E163697, 0x88D273CC,
    0xDE966292, 0x81B949D0, 0x4C50901B, 0x71C65614, 0xE6C6C7BD, 0x327A140A,
    0x45E1D006, 0xC3F27B9A, 0xC9AA53FD, 0x62A80F00, 0xBB25BFE2, 0x35BDD2F6,
    0x71126905, 0xB2040222, 0xB6CBCF7C, 0xCD769C2B, 0x53113EC0, 0x1640E3D3,
    0x38ABBD60, 0x2547ADF0, 0xBA38209C, 0xF746CE76, 0x77AFA1C5, 0x20756060,
    0x85CBFE4E, 0x8AE88DD8, 0x7AAAF9B0, 0x4CF9AA7E, 0x1948C25C, 0x02FB8A8C,
    0x01C36AE4, 0xD6EBE1F9, 0x90D4F869, 0xA65CDEA0, 0x3F09252D, 0xC208E69F,
    0xB74E6132, 0xCE77E25B, 0x578FDFE3, 0x3AC372E6,
};

struct Blowfish {
    quint32 p[18], s[4][256];
    bool set(const QByteArray &key)
    {
        if (key.size() < 4 || key.size() > 56) return false;
        std::memcpy(p, kBlowfishInit, sizeof p);
        std::memcpy(s, kBlowfishInit + 18, sizeof s);
        const quint8 *k = reinterpret_cast<const quint8 *>(key.constData());
        int at = 0;
        for (int i = 0; i < 18; ++i) {
            quint32 w = 0;
            for (int j = 0; j < 4; ++j) {
                w = (w << 8) | k[at];
                at = (at + 1) % int(key.size());
            }
            p[i] ^= w;
        }
        quint32 l = 0, r = 0;
        for (int i = 0; i < 18; i += 2) {
            encrypt(&l, &r);
            p[i] = l;
            p[i + 1] = r;
        }
        for (int b = 0; b < 4; ++b)
            for (int i = 0; i < 256; i += 2) {
                encrypt(&l, &r);
                s[b][i] = l;
                s[b][i + 1] = r;
            }
        return true;
    }
    quint32 f(quint32 x) const { return ((s[0][x >> 24] + s[1][(x >> 16) & 0xFF]) ^ s[2][(x >> 8) & 0xFF]) + s[3][x & 0xFF]; }
    void encrypt(quint32 *lp, quint32 *rp) const
    {
        quint32 l = *lp, r = *rp;
        for (int i = 0; i < 16; ++i) {
            l ^= p[i];
            r ^= f(l);
            std::swap(l, r);
        }
        std::swap(l, r);
        r ^= p[16];
        l ^= p[17];
        *lp = l;
        *rp = r;
    }
    void decrypt(quint32 *lp, quint32 *rp) const
    {
        quint32 l = *lp, r = *rp;
        for (int i = 17; i > 1; --i) {
            l ^= p[i];
            r ^= f(l);
            std::swap(l, r);
        }
        std::swap(l, r);
        r ^= p[1];
        l ^= p[0];
        *lp = l;
        *rp = r;
    }
};

} // namespace

void secureZero(void *where, size_t size)
{
    volatile unsigned char *p = static_cast<volatile unsigned char *>(where);
    for (size_t i = 0; i < size; ++i) p[i] = 0;
}

bool equal(const QByteArray &a, const QByteArray &b)
{
    unsigned diff = unsigned(a.size() != b.size());
    const qsizetype n = std::min(a.size(), b.size());
    for (qsizetype i = 0; i < n; ++i) diff |= unsigned(quint8(a[i]) ^ quint8(b[i]));
    return diff == 0;
}

namespace {

// ---- GCM

struct Ghash {
    quint64 powH[128], powL[128];   // H times x to the power i, for each bit i of a block (the multiplication is a sum of these for the bits that are set)
    void init(const quint8 *h)
    {
        quint64 vh = 0, vl = 0;
        for (int i = 0; i < 8; ++i) {
            vh = (vh << 8) | h[i];
            vl = (vl << 8) | h[8 + i];
        }
        for (int i = 0; i < 128; ++i) {
            powH[i] = vh;
            powL[i] = vl;
            const quint64 carry = 0 - (vl & 1);   // (all ones if the bit shifted out is set: the field's polynomial is then taken in)
            vl = (vh << 63) | (vl >> 1);
            vh = (vh >> 1) ^ (carry & 0xe100000000000000ULL);
        }
    }
    // y = (y xor block) times H
    void mul(quint8 *y, const quint8 *block) const
    {
        quint64 zh = 0, zl = 0;
        for (int i = 0; i < 128; ++i) {
            const quint64 mask = 0 - quint64(((y[i >> 3] ^ block[i >> 3]) >> (7 - (i & 7))) & 1);
            zh ^= powH[i] & mask;
            zl ^= powL[i] & mask;
        }
        for (int i = 0; i < 8; ++i) {
            y[i] = quint8(zh >> (56 - 8 * i));
            y[8 + i] = quint8(zl >> (56 - 8 * i));
        }
    }
    // Hashes `data` (the last block padded with zeroes) into y.
    void update(quint8 *y, const QByteArray &data) const
    {
        const quint8 *p = reinterpret_cast<const quint8 *>(data.constData());
        qsizetype n = data.size();
        for (; n >= 16; n -= 16, p += 16) mul(y, p);
        if (n > 0) {
            quint8 last[16] = {};
            std::memcpy(last, p, size_t(n));
            mul(y, last);
        }
    }
    void lengths(quint8 *y, quint64 aBytes, quint64 cBytes) const
    {
        quint8 block[16];
        for (int i = 0; i < 8; ++i) {
            block[i] = quint8((aBytes * 8) >> (56 - 8 * i));
            block[8 + i] = quint8((cBytes * 8) >> (56 - 8 * i));
        }
        mul(y, block);
    }
};

// The counter blocks from `first` on (the last four bytes counting, modulo 2^32), enciphered, xored into data.
void gcmCtr(const AesKey &k, const quint8 *first, const QByteArray &in, QByteArray *out)
{
    out->resize(in.size());
    quint8 counter[16], stream[16];
    std::memcpy(counter, first, 16);
    for (qsizetype at = 0; at < in.size(); at += 16) {
        k.encryptBlock(counter, stream);
        const qsizetype n = std::min<qsizetype>(16, in.size() - at);
        for (qsizetype i = 0; i < n; ++i) (*out)[at + i] = char(quint8(in[at + i]) ^ stream[i]);
        for (int i = 15; i >= 12; --i)
            if (++counter[i]) break;
    }
    secureZero(counter, sizeof counter);
    secureZero(stream, sizeof stream);
}

// The tag (before the xor with the enciphered J0) and J0, from the key, iv, aad, and the enciphered data.
bool gcmSetup(const QByteArray &key, const QByteArray &iv, AesKey *k, Ghash *g, quint8 *j0)
{
    if (iv.isEmpty() || !k->set(key)) return false;
    quint8 zero[16] = {}, h[16];
    k->encryptBlock(zero, h);
    g->init(h);
    secureZero(h, sizeof h);
    if (iv.size() == 12) {
        std::memcpy(j0, iv.constData(), 12);
        j0[12] = j0[13] = j0[14] = 0;
        j0[15] = 1;
    } else {
        quint8 y[16] = {};
        g->update(y, iv);
        g->lengths(y, 0, quint64(iv.size()));
        std::memcpy(j0, y, 16);
    }
    return true;
}

QByteArray gcmTag(const AesKey &k, const Ghash &g, const quint8 *j0, const QByteArray &aad, const QByteArray &cipher)
{
    quint8 y[16] = {};
    g.update(y, aad);
    g.update(y, cipher);
    g.lengths(y, quint64(aad.size()), quint64(cipher.size()));
    quint8 mask[16];
    k.encryptBlock(j0, mask);
    QByteArray tag(16, Qt::Uninitialized);
    for (int i = 0; i < 16; ++i) tag[i] = char(y[i] ^ mask[i]);
    return tag;
}

void incCounter(quint8 *block)
{
    for (int i = 15; i >= 12; --i)
        if (++block[i]) break;
}

// ---- BLAKE2b

struct Blake2b {
    quint64 h[8];
    quint8 buf[128];
    size_t fill = 0;
    quint64 t = 0;
    int outLen;

    static quint64 rotr(quint64 x, int n) { return (x >> n) | (x << (64 - n)); }
    static const quint64 *iv()
    {
        static const quint64 v[8] = {0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL};
        return v;
    }
    Blake2b(int out, const QByteArray &key) : outLen(out)
    {
        std::memcpy(h, iv(), sizeof h);
        h[0] ^= 0x01010000ULL ^ (quint64(key.size()) << 8) ^ quint64(out);
        if (!key.isEmpty()) {
            quint8 block[128] = {};
            std::memcpy(block, key.constData(), size_t(key.size()));
            update(block, 128);
            secureZero(block, sizeof block);
        }
    }
    void compress(const quint8 *block, bool last)
    {
        static const quint8 sigma[10][16] = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
                                             {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4}, {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
                                             {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13}, {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
                                             {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11}, {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
                                             {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5}, {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0}};
        quint64 m[16], v[16];
        for (int i = 0; i < 16; ++i) {
            m[i] = 0;
            for (int b = 7; b >= 0; --b) m[i] = (m[i] << 8) | block[8 * i + b];
        }
        for (int i = 0; i < 8; ++i) {
            v[i] = h[i];
            v[i + 8] = iv()[i];
        }
        v[12] ^= t;
        if (last) v[14] = ~v[14];
        auto g = [&](int a, int b, int c, int d, quint64 x, quint64 y) {
            v[a] = v[a] + v[b] + x;
            v[d] = rotr(v[d] ^ v[a], 32);
            v[c] = v[c] + v[d];
            v[b] = rotr(v[b] ^ v[c], 24);
            v[a] = v[a] + v[b] + y;
            v[d] = rotr(v[d] ^ v[a], 16);
            v[c] = v[c] + v[d];
            v[b] = rotr(v[b] ^ v[c], 63);
        };
        for (int r = 0; r < 12; ++r) {
            const quint8 *s = sigma[r % 10];
            g(0, 4, 8, 12, m[s[0]], m[s[1]]);
            g(1, 5, 9, 13, m[s[2]], m[s[3]]);
            g(2, 6, 10, 14, m[s[4]], m[s[5]]);
            g(3, 7, 11, 15, m[s[6]], m[s[7]]);
            g(0, 5, 10, 15, m[s[8]], m[s[9]]);
            g(1, 6, 11, 12, m[s[10]], m[s[11]]);
            g(2, 7, 8, 13, m[s[12]], m[s[13]]);
            g(3, 4, 9, 14, m[s[14]], m[s[15]]);
        }
        for (int i = 0; i < 8; ++i) h[i] ^= v[i] ^ v[i + 8];
    }
    void update(const quint8 *p, size_t n)
    {
        while (n > 0) {
            if (fill == 128) {   // (a full block is compressed only when more follows: the last one is flagged)
                t += 128;
                compress(buf, false);
                fill = 0;
            }
            const size_t take = std::min(n, 128 - fill);
            std::memcpy(buf + fill, p, take);
            fill += take;
            p += take;
            n -= take;
        }
    }
    void update(const QByteArray &d) { update(reinterpret_cast<const quint8 *>(d.constData()), size_t(d.size())); }
    void update32(quint32 v)
    {
        const quint8 b[4] = {quint8(v), quint8(v >> 8), quint8(v >> 16), quint8(v >> 24)};
        update(b, 4);
    }
    QByteArray finish()
    {
        t += fill;
        std::memset(buf + fill, 0, 128 - fill);
        compress(buf, true);
        QByteArray out(outLen, Qt::Uninitialized);
        for (int i = 0; i < outLen; ++i) out[i] = char(h[i / 8] >> (8 * (i % 8)));
        secureZero(this, sizeof *this);
        return out;
    }
};

// ---- Argon2

constexpr size_t kBlockWords = 128;   // (1,024 bytes)

// H' of RFC 9106: a hash of any length from BLAKE2b.
QByteArray argonLongHash(int outLen, const QByteArray &a)
{
    Blake2b first(outLen <= 64 ? outLen : 64, QByteArray());
    first.update32(quint32(outLen));
    first.update(a);
    QByteArray v = first.finish();
    if (outLen <= 64) return v;
    QByteArray out = v.left(32);
    int left = outLen - 32;
    while (left > 64) {
        Blake2b next(64, QByteArray());
        next.update(v);
        v = next.finish();
        out += v.left(32);
        left -= 32;
    }
    Blake2b lastOne(left, QByteArray());
    lastOne.update(v);
    out += lastOne.finish();
    return out;
}

inline quint64 argonRotr(quint64 x, int n) { return (x >> n) | (x << (64 - n)); }
inline quint64 fBlaMka(quint64 x, quint64 y) { return x + y + 2 * (x & 0xFFFFFFFFULL) * (y & 0xFFFFFFFFULL); }

void argonRound(quint64 &a, quint64 &b, quint64 &c, quint64 &d)
{
    a = fBlaMka(a, b);
    d = argonRotr(d ^ a, 32);
    c = fBlaMka(c, d);
    b = argonRotr(b ^ c, 24);
    a = fBlaMka(a, b);
    d = argonRotr(d ^ a, 16);
    c = fBlaMka(c, d);
    b = argonRotr(b ^ c, 63);
}

void argonPermute(quint64 *v0, quint64 *v1, quint64 *v2, quint64 *v3, quint64 *v4, quint64 *v5, quint64 *v6, quint64 *v7, quint64 *v8, quint64 *v9, quint64 *v10, quint64 *v11,
                  quint64 *v12, quint64 *v13, quint64 *v14, quint64 *v15)
{
    argonRound(*v0, *v4, *v8, *v12);
    argonRound(*v1, *v5, *v9, *v13);
    argonRound(*v2, *v6, *v10, *v14);
    argonRound(*v3, *v7, *v11, *v15);
    argonRound(*v0, *v5, *v10, *v15);
    argonRound(*v1, *v6, *v11, *v12);
    argonRound(*v2, *v7, *v8, *v13);
    argonRound(*v3, *v4, *v9, *v14);
}

// out = G(x, y), and with `xorInto` out = out xor G(x, y).
void argonCompress(quint64 *out, const quint64 *x, const quint64 *y, bool xorInto)
{
    quint64 r[kBlockWords], z[kBlockWords];
    for (size_t i = 0; i < kBlockWords; ++i) z[i] = r[i] = x[i] ^ y[i];
    for (int i = 0; i < 8; ++i) {
        quint64 *q = z + 16 * i;
        argonPermute(q, q + 1, q + 2, q + 3, q + 4, q + 5, q + 6, q + 7, q + 8, q + 9, q + 10, q + 11, q + 12, q + 13, q + 14, q + 15);
    }
    for (int i = 0; i < 8; ++i)
        argonPermute(z + 2 * i, z + 2 * i + 1, z + 2 * i + 16, z + 2 * i + 17, z + 2 * i + 32, z + 2 * i + 33, z + 2 * i + 48, z + 2 * i + 49, z + 2 * i + 64, z + 2 * i + 65, z + 2 * i + 80,
                     z + 2 * i + 81, z + 2 * i + 96, z + 2 * i + 97, z + 2 * i + 112, z + 2 * i + 113);
    for (size_t i = 0; i < kBlockWords; ++i) out[i] = (xorInto ? out[i] : 0) ^ z[i] ^ r[i];
    secureZero(r, sizeof r);
    secureZero(z, sizeof z);
}

inline void putLe32(QByteArray &a, quint32 v)
{
    for (int i = 0; i < 4; ++i) a.append(char(v >> (8 * i)));
}

} // namespace

QByteArray aesGcmSeal(const QByteArray &key, const QByteArray &iv, const QByteArray &aad, const QByteArray &plain)
{
    AesKey k;
    Ghash g;
    quint8 j0[16];
    if (!gcmSetup(key, iv, &k, &g, j0)) return QByteArray();
    quint8 first[16];
    std::memcpy(first, j0, 16);
    incCounter(first);
    QByteArray cipher;
    gcmCtr(k, first, plain, &cipher);
    const QByteArray tag = gcmTag(k, g, j0, aad, cipher);
    secureZero(&k, sizeof k);
    return cipher + tag;
}

QByteArray aesGcmOpen(const QByteArray &key, const QByteArray &iv, const QByteArray &aad, const QByteArray &sealed, bool *ok)
{
    if (ok) *ok = false;
    AesKey k;
    Ghash g;
    quint8 j0[16];
    if (sealed.size() < 16 || !gcmSetup(key, iv, &k, &g, j0)) return QByteArray();
    const QByteArray cipher = sealed.left(sealed.size() - 16);
    const QByteArray tag = gcmTag(k, g, j0, aad, cipher);
    QByteArray plain;
    const bool good = equal(tag, sealed.right(16));
    if (good) {
        quint8 first[16];
        std::memcpy(first, j0, 16);
        incCounter(first);
        gcmCtr(k, first, cipher, &plain);
    }
    secureZero(&k, sizeof k);
    if (ok) *ok = good;
    return plain;
}

QByteArray blake2b(const QByteArray &data, int outLen, const QByteArray &key)
{
    if (outLen < 1 || outLen > 64 || key.size() > 64) return QByteArray();
    Blake2b b(outLen, key);
    b.update(data);
    return b.finish();
}

QByteArray argon2id(const QByteArray &password, const QByteArray &salt, quint32 iterations, quint32 memoryKiB, quint32 lanes, int outLen, const QByteArray &secret, const QByteArray &ad,
                    const std::function<bool()> &cancelled)
{
    if (iterations < 1 || lanes < 1 || lanes > 0xFFFFFF || memoryKiB < 8 * lanes || outLen < 4 || salt.size() < 8) return QByteArray();
    const quint32 blocks = memoryKiB / (4 * lanes) * (4 * lanes);   // (the memory is made a whole number of segments)
    const quint32 laneLen = blocks / lanes, segLen = laneLen / 4;
    std::vector<quint64> mem;
    try {
        mem.assign(size_t(blocks) * kBlockWords, 0);
    } catch (const std::bad_alloc &) {
        return QByteArray();
    }
    Blake2b h0(64, QByteArray());
    h0.update32(lanes);
    h0.update32(quint32(outLen));
    h0.update32(memoryKiB);
    h0.update32(iterations);
    h0.update32(0x13);
    h0.update32(2);
    h0.update32(quint32(password.size()));
    h0.update(password);
    h0.update32(quint32(salt.size()));
    h0.update(salt);
    h0.update32(quint32(secret.size()));
    h0.update(secret);
    h0.update32(quint32(ad.size()));
    h0.update(ad);
    QByteArray seed = h0.finish();
    auto blockAt = [&](quint32 lane, quint32 col) { return mem.data() + (size_t(lane) * laneLen + col) * kBlockWords; };
    auto load = [&](quint64 *to, const QByteArray &bytes) {
        for (size_t i = 0; i < kBlockWords; ++i) {
            quint64 w = 0;
            for (int b = 7; b >= 0; --b) w = (w << 8) | quint8(bytes[int(8 * i) + b]);
            to[i] = w;
        }
    };
    for (quint32 l = 0; l < lanes; ++l)
        for (quint32 col = 0; col < 2; ++col) {
            QByteArray in = seed;
            putLe32(in, col);
            putLe32(in, l);
            load(blockAt(l, col), argonLongHash(1024, in));
        }
    wipe(seed);
    quint64 zero[kBlockWords] = {}, input[kBlockWords], addr[kBlockWords], tmp[kBlockWords];
    for (quint32 pass = 0; pass < iterations; ++pass)
        for (quint32 slice = 0; slice < 4; ++slice)
            for (quint32 lane = 0; lane < lanes; ++lane) {
                if (cancelled && cancelled()) {
                    secureZero(mem.data(), mem.size() * sizeof(quint64));
                    return QByteArray();
                }
                const bool independent = pass == 0 && slice < 2;
                quint32 start = 0;
                if (independent) {
                    std::memset(input, 0, sizeof input);
                    input[0] = pass;
                    input[1] = lane;
                    input[2] = slice;
                    input[3] = blocks;
                    input[4] = iterations;
                    input[5] = 2;
                }
                auto nextAddresses = [&]() {
                    ++input[6];
                    argonCompress(tmp, zero, input, false);
                    argonCompress(addr, zero, tmp, false);
                };
                if (pass == 0 && slice == 0) {
                    start = 2;
                    if (independent) nextAddresses();
                }
                for (quint32 i = start; i < segLen; ++i) {
                    const quint32 col = slice * segLen + i;
                    const quint32 prevCol = col == 0 ? laneLen - 1 : col - 1;
                    const quint64 *prev = blockAt(lane, prevCol);
                    quint64 rand;
                    if (independent) {
                        if (i % kBlockWords == 0) nextAddresses();
                        rand = addr[i % kBlockWords];
                    } else {
                        rand = prev[0];
                    }
                    const quint64 j1 = rand & 0xFFFFFFFFULL;
                    quint32 refLane = quint32((rand >> 32) % lanes);
                    if (pass == 0 && slice == 0) refLane = lane;
                    const bool same = refLane == lane;
                    qint64 area;
                    if (pass == 0) {
                        if (slice == 0) area = qint64(i) - 1;
                        else if (same) area = qint64(slice) * segLen + i - 1;
                        else area = qint64(slice) * segLen + (i == 0 ? -1 : 0);
                    } else {
                        if (same) area = qint64(laneLen) - segLen + i - 1;
                        else area = qint64(laneLen) - segLen + (i == 0 ? -1 : 0);
                    }
                    quint64 rel = (j1 * j1) >> 32;
                    rel = quint64(area) - 1 - ((quint64(area) * rel) >> 32);
                    quint64 startPos = 0;
                    if (pass != 0) startPos = slice == 3 ? 0 : quint64(slice + 1) * segLen;
                    const quint32 refCol = quint32((startPos + rel) % laneLen);
                    argonCompress(blockAt(lane, col), prev, blockAt(refLane, refCol), pass != 0);
                    if ((i & 0xFF) == 0xFF && cancelled && cancelled()) {
                        secureZero(mem.data(), mem.size() * sizeof(quint64));
                        return QByteArray();
                    }
                }
            }
    quint64 fin[kBlockWords];
    std::memcpy(fin, blockAt(0, laneLen - 1), sizeof fin);
    for (quint32 l = 1; l < lanes; ++l) {
        const quint64 *b = blockAt(l, laneLen - 1);
        for (size_t i = 0; i < kBlockWords; ++i) fin[i] ^= b[i];
    }
    QByteArray bytes(1024, Qt::Uninitialized);
    for (size_t i = 0; i < kBlockWords; ++i)
        for (int b = 0; b < 8; ++b) bytes[int(8 * i) + b] = char(fin[i] >> (8 * b));
    const QByteArray tag = argonLongHash(outLen, bytes);
    wipe(bytes);
    secureZero(fin, sizeof fin);
    secureZero(mem.data(), mem.size() * sizeof(quint64));
    return tag;
}

QByteArray aesEcb(const QByteArray &key, const QByteArray &data, bool encrypt)
{
    AesKey k;
    if (!k.set(key) || data.size() % 16 != 0) return QByteArray();
    QByteArray out(data.size(), Qt::Uninitialized);
    const quint8 *in = reinterpret_cast<const quint8 *>(data.constData());
    quint8 *o = reinterpret_cast<quint8 *>(out.data());
    for (qsizetype at = 0; at < data.size(); at += 16) {
        if (encrypt) k.encryptBlock(in + at, o + at);
        else k.decryptBlock(in + at, o + at);
    }
    secureZero(&k, sizeof k);
    return out;
}

QByteArray aesCbc(const QByteArray &key, const QByteArray &iv, const QByteArray &data, bool encrypt)
{
    AesKey k;
    if (iv.size() != 16 || !k.set(key) || data.size() % 16 != 0) return QByteArray();
    QByteArray out(data.size(), Qt::Uninitialized);
    const quint8 *in = reinterpret_cast<const quint8 *>(data.constData());
    quint8 *o = reinterpret_cast<quint8 *>(out.data());
    quint8 chain[16], block[16];
    std::memcpy(chain, iv.constData(), 16);
    for (qsizetype at = 0; at < data.size(); at += 16) {
        if (encrypt) {
            for (int i = 0; i < 16; ++i) block[i] = in[at + i] ^ chain[i];
            k.encryptBlock(block, o + at);
            std::memcpy(chain, o + at, 16);
        } else {
            k.decryptBlock(in + at, block);
            for (int i = 0; i < 16; ++i) o[at + i] = block[i] ^ chain[i];
            std::memcpy(chain, in + at, 16);
        }
    }
    secureZero(&k, sizeof k);
    secureZero(chain, sizeof chain);
    secureZero(block, sizeof block);
    return out;
}

Rc4::Rc4(const QByteArray &key)
{
    for (int i = 0; i < 256; ++i) m_s[size_t(i)] = quint8(i);
    if (key.isEmpty()) return;
    quint8 j = 0;
    for (int i = 0; i < 256; ++i) {
        j = quint8(j + m_s[size_t(i)] + quint8(key[i % key.size()]));
        std::swap(m_s[size_t(i)], m_s[j]);
    }
}

Rc4::~Rc4()
{
    secureZero(m_s.data(), m_s.size());
    m_i = m_j = 0;
}

void Rc4::apply(char *data, qsizetype size)
{
    for (qsizetype n = 0; n < size; ++n) {
        m_i = quint8(m_i + 1);
        m_j = quint8(m_j + m_s[m_i]);
        std::swap(m_s[m_i], m_s[m_j]);
        data[n] = char(quint8(data[n]) ^ m_s[quint8(m_s[m_i] + m_s[m_j])]);
    }
}

QByteArray Rc4::apply(const QByteArray &data)
{
    QByteArray out = data;
    apply(out.data(), out.size());
    return out;
}

void Rc4::skip(qsizetype count)
{
    for (qsizetype n = 0; n < count; ++n) {
        m_i = quint8(m_i + 1);
        m_j = quint8(m_j + m_s[m_i]);
        std::swap(m_s[m_i], m_s[m_j]);
    }
}

QByteArray blowfishCfb(const QByteArray &key, const QByteArray &iv, const QByteArray &data, bool encrypt)
{
    Blowfish bf;
    if (iv.size() != 8 || !bf.set(key)) return QByteArray();
    QByteArray out(data.size(), Qt::Uninitialized);
    const quint8 *in = reinterpret_cast<const quint8 *>(data.constData());
    quint8 *o = reinterpret_cast<quint8 *>(out.data());
    quint8 reg[8];
    std::memcpy(reg, iv.constData(), 8);
    for (qsizetype at = 0; at < data.size(); at += 8) {
        quint32 l = load32(reg), r = load32(reg + 4);
        bf.encrypt(&l, &r);
        quint8 stream[8];
        store32(stream, l);
        store32(stream + 4, r);
        const int n = int(std::min<qsizetype>(8, data.size() - at));
        for (int i = 0; i < n; ++i) o[at + i] = in[at + i] ^ stream[i];
        // what is fed back is the cipher text
        const quint8 *cipher = encrypt ? o + at : in + at;
        if (n == 8) std::memcpy(reg, cipher, 8);
    }
    secureZero(&bf, sizeof bf);
    return out;
}

QByteArray blowfishEcbBlock(const QByteArray &key, const QByteArray &block, bool encrypt)
{
    Blowfish bf;
    if (block.size() != 8 || !bf.set(key)) return QByteArray();
    const quint8 *in = reinterpret_cast<const quint8 *>(block.constData());
    quint32 l = load32(in), r = load32(in + 4);
    if (encrypt) bf.encrypt(&l, &r);
    else bf.decrypt(&l, &r);
    QByteArray out(8, Qt::Uninitialized);
    store32(reinterpret_cast<quint8 *>(out.data()), l);
    store32(reinterpret_cast<quint8 *>(out.data()) + 4, r);
    return out;
}

void wipe(QByteArray &bytes)
{
    if (bytes.isEmpty()) return;
    bytes.detach();
    secureZero(bytes.data(), size_t(bytes.size()));
    bytes.clear();
}

void wipe(QString &text)
{
    if (text.isEmpty()) return;
    text.detach();
    secureZero(text.data(), size_t(text.size()) * sizeof(QChar));
    text.clear();
}

} // namespace jp::crypto
