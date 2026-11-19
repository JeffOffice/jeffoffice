// The fonts a word processing document carries (word/fonts/*.odttf, listed in the font table by w:embedRegular, w:embedBold, w:embedItalic, and
// w:embedBoldItalic, each with the r:id of its part and the w:fontKey that unscrambles it: the first 32 bytes of the font file are XORed with the
// 16 bytes of the key, taken from the GUID's digits read from the last to the first).
#include "wml/wml.h"

#include "office/embeddedfonts.h"
#include "office/eot.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QHash>

namespace jp::office::wml {

namespace {

// The 16 bytes of a font key ("{0F1E2D3C-4B5A-6978-8796-A5B4C3D2E1F0}" or the same without the braces and hyphens), the last digits first. Empty if it isn't one.
QByteArray fontKeyBytes(const QString &text)
{
    QByteArray digits;
    for (QChar c : text) {
        if (c == QLatin1Char('{') || c == QLatin1Char('}') || c == QLatin1Char('-')) continue;
        if (!c.isLetterOrNumber() || c.unicode() > 127) return QByteArray();
        digits += char(c.unicode());
    }
    if (digits.size() != 32) return QByteArray();
    const QByteArray bytes = QByteArray::fromHex(digits);
    if (bytes.size() != 16) return QByteArray();
    QByteArray out;
    for (int i = 15; i >= 0; --i) out += bytes[i];
    return out;
}

} // namespace

QByteArray unscrambledFont(const QByteArray &part, const QString &fontKey)
{
    const QByteArray key = fontKeyBytes(fontKey);
    if (key.isEmpty()) return QByteArray();
    QByteArray out = part;
    for (int i = 0; i < 32 && i < out.size(); ++i) out[i] = char(uchar(out[i]) ^ uchar(key[i % 16]));
    return out;
}

QVector<EmbeddedFace> readEmbeddedFonts(const Document &doc, QStringList *notes)
{
    QVector<EmbeddedFace> faces;
    QString tableName;
    for (const Relationship &r : doc.mainRelationships)
        if (!r.external && r.type.endsWith(QLatin1String("/fontTable"))) {
            tableName = r.target;
            break;
        }
    if (tableName.isEmpty()) return faces;
    QHash<QString, int> partAt;
    const KeptPart *table = nullptr;
    for (int i = 0; i < doc.parts.size(); ++i) {
        partAt.insert(doc.parts[i].name.toLower(), i);
        if (doc.parts[i].name.compare(tableName, Qt::CaseInsensitive) == 0) table = &doc.parts[i];
    }
    if (!table) return faces;
    XmlTree tree;
    if (!tree.parse(table->data, nullptr)) return faces;
    const QVector<Relationship> rels = doc.partRelationships.value(table->name);
    static const char *const kSlots[4] = {"embedRegular", "embedBold", "embedItalic", "embedBoldItalic"};
    qint64 total = 0;
    int unusable = 0, entries = 0;
    QString firstReason;
    bool tooMany = false, tooLarge = false;
    for (int c = tree.firstChild(tree.root()); c >= 0 && ++entries <= kMaxFonts; c = tree.next(c)) {
        if (!tree.is(c, "font")) continue;
        const QString typeface = tree.attr(c, "name").trimmed();
        if (typeface.isEmpty()) continue;
        for (int slot = 0; slot < 4; ++slot) {
            const int embed = tree.child(c, kSlots[slot]);
            if (embed < 0) continue;
            const QString id = tree.attr(embed, "id", NsR);
            const Relationship *link = nullptr;
            for (const Relationship &r : rels)
                if (r.id == id && !r.external) link = &r;
            const auto at = link ? partAt.constFind(link->target.toLower()) : partAt.constEnd();
            if (at == partAt.constEnd()) continue;
            if (faces.size() >= kMaxEmbeddedFonts) {
                tooMany = true;
                continue;
            }
            const KeptPart &part = doc.parts[at.value()];
            if (part.data.size() > kMaxEmbeddedFontBytes || total + part.data.size() > kMaxEmbeddedFontsTotal) {
                tooLarge = true;
                continue;
            }
            total += part.data.size();
            EmbeddedFace face;
            QString why;
            const QByteArray plain = unscrambledFont(part.data, tree.attr(embed, "fontKey"));
            if (plain.isEmpty()) {
                ++unusable;
                if (firstReason.isEmpty()) firstReason = QCoreApplication::translate("jp::office::wml", "the key that unscrambles it is not valid.");
                continue;
            }
            if (!readEmbeddedFont(plain, &face.font, &face.info, &why)) {
                ++unusable;
                if (firstReason.isEmpty()) firstReason = why;
                continue;
            }
            face.typeface = typeface;
            face.slot = FontSlot(slot);
            face.pitchFamily = face.info.pitchFamily();
            face.charset = face.info.charset();
            if (!fontMayBeShown(face.info)) face.font.clear();   // (its license: kept, not drawn with)
            faces << face;
        }
    }
    if (notes) {
        if (unusable == 1) *notes << QCoreApplication::translate("jp::office::wml", "An embedded font in the file can't be used: %1").arg(firstReason);
        else if (unusable > 1) *notes << QCoreApplication::translate("jp::office::wml", "%1 embedded fonts in the file can't be used: %2").arg(unusable).arg(firstReason);
        if (tooMany) *notes << QCoreApplication::translate("jp::office::wml", "Only the first %1 embedded fonts are read.").arg(kMaxEmbeddedFonts);
        if (tooLarge) *notes << QCoreApplication::translate("jp::office::wml", "Some embedded fonts are left out: they are too large.");
    }
    return faces;
}

} // namespace jp::office::wml
