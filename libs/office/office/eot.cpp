#include "office/eot.h"

#include <QCoreApplication>
#include <QtEndian>

namespace jp::office {

namespace {

QString say(const char *text) { return QCoreApplication::translate("EmbeddedFont", text); }

constexpr quint32 kVersion1 = 0x00010000, kVersion21 = 0x00020001, kVersion22 = 0x00020002;
constexpr quint16 kMagic = 0x504C;
constexpr quint32 kFlagCompressed = 0x4, kFlagScrambled = 0x10000000;
// The fixed part of every version's header: through the four reserved numbers and the padding before the family name.
constexpr qint64 kFixedHeader = 82;
// The font's data ends the file; the names between the header and it are all the header may hold.
constexpr qint64 kLeastHeader = kFixedHeader + 2 + 2 * 4;

quint32 le32(const QByteArray &b, qint64 at) { return at >= 0 && at + 4 <= b.size() ? qFromLittleEndian<quint32>(b.constData() + at) : 0; }
quint16 le16(const QByteArray &b, qint64 at) { return at >= 0 && at + 2 <= b.size() ? qFromLittleEndian<quint16>(b.constData() + at) : 0; }

void put16(QByteArray &b, quint16 v)
{
    char c[2];
    qToLittleEndian<quint16>(v, c);
    b.append(c, 2);
}

void put32(QByteArray &b, quint32 v)
{
    char c[4];
    qToLittleEndian<quint32>(v, c);
    b.append(c, 4);
}

// A name, as the header holds it: its size in bytes, then the letters in UTF-16, little-endian. The program ends the family and style
// with a zero character that its size counts.
void putName(QByteArray &b, const QString &name, bool terminated = false)
{
    put16(b, 0);   // the padding before it
    put16(b, quint16(2 * (name.size() + (terminated ? 1 : 0))));
    for (QChar c : name) put16(b, c.unicode());
    if (terminated) put16(b, 0);
}

// What the header's checksum of the root string (the places the font may be used from) is when there is none: the program refuses a
// font whose header has any other. It is the checksum of the string's bytes, which are none, plus this.
constexpr quint32 kRootStringCheckSum = 0x50475342;
constexpr quint32 kLatinCodePage = 1252;

bool looksLikeFontFile(const QByteArray &data)
{
    if (data.size() < 4) return false;
    const quint32 version = qFromBigEndian<quint32>(data.constData());
    return version == 0x00010000 || version == 0x74727565 || version == 0x4F54544F;
}

} // namespace

QByteArray makeEot(const QByteArray &font)
{
    FontFileInfo info;
    if (!inspectFont(font, &info) || info.family.isEmpty()) return QByteArray();
    // Style, version, and full name are written as the font has them.
    QByteArray eot;
    put32(eot, 0);                                 // EOTSize, filled in at the end
    put32(eot, quint32(font.size()));              // FontDataSize
    put32(eot, kVersion22);
    put32(eot, 0);                                 // flags: not compressed, not scrambled, not a subset
    eot.append(reinterpret_cast<const char *>(info.panose), 10);
    eot.append(char(info.charset()));
    eot.append(char(info.italic ? 1 : 0));
    put32(eot, info.weight);
    put16(eot, info.fsType);
    put16(eot, kMagic);
    for (int i = 0; i < 4; ++i) put32(eot, info.unicodeRange[i]);
    for (int i = 0; i < 2; ++i) put32(eot, info.codePageRange[i]);
    put32(eot, info.checkSumAdjustment);
    for (int i = 0; i < 4; ++i) put32(eot, 0);     // reserved
    putName(eot, info.family, true);
    putName(eot, info.subfamily, true);
    putName(eot, info.versionName);
    putName(eot, info.fullName);
    put16(eot, 0);                                 // the padding before the root string
    put16(eot, 0);                                 // no root string (the places the font may be used from)
    put32(eot, kRootStringCheckSum);
    put32(eot, kLatinCodePage);                    // the code page of the font's private characters, which it has none of
    put16(eot, 0);
    put16(eot, 0);                                 // no signature
    put32(eot, 0);                                 // flags of the private-character font
    put32(eot, 0);                                 // and its size: none
    eot.append(font);
    qToLittleEndian<quint32>(quint32(eot.size()), eot.data());
    return eot;
}

bool readEmbeddedFont(const QByteArray &data, QByteArray *font, FontFileInfo *info, QString *why)
{
    if (font) font->clear();
    auto fail = [&](const QString &reason) {
        if (why) *why = reason;
        return false;
    };
    QByteArray bytes;
    if (looksLikeFontFile(data)) {
        bytes = data;
    } else {
        if (data.size() < kLeastHeader) return fail(say("The embedded font is too short to be a font."));
        const qint64 total = le32(data, 0), fontSize = le32(data, 4);
        const quint32 version = le32(data, 8), flags = le32(data, 12);
        if (version != kVersion1 && version != kVersion21 && version != kVersion22) return fail(say("The embedded font is in a form that is not read."));
        if (le16(data, 34) != kMagic) return fail(say("The embedded font is damaged."));
        if (total > data.size()) return fail(say("The embedded font is cut off."));
        if (fontSize > kMaxEmbeddedFontBytes) return fail(say("The embedded font is larger than 32 MB."));
        // Whatever the names and other things the header holds come to, the font is the last of the file.
        if (fontSize > total - kLeastHeader) return fail(say("The embedded font is damaged."));
        bytes = data.mid(total - fontSize, fontSize);
        if (flags & kFlagScrambled)
            for (char &c : bytes) c = char(c ^ 0x50);
        if (flags & kFlagCompressed) return fail(say("The embedded font is compressed in a way that is not read."));
    }
    if (!inspectFont(bytes, info, why)) return false;
    if (font) *font = bytes;
    return true;
}

} // namespace jp::office
