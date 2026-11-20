// Fonts written into a saved word processing document (see wml.h embedFontsInDocx): the parts are the fonts as the format scrambles them (wmlfonts.cpp reads them),
// each listed in the font table by the style it is for.
#include "wml/wml.h"

#include "core/sfnt.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace jp::office::wml {

namespace {

constexpr int kMostFacesAsked = 4096;   // (names looked at; each costs a lookup)
const QString kWNs = QStringLiteral("http://schemas.openxmlformats.org/wordprocessingml/2006/main");
const QString kRNs = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");
const QString kStrictRNs = QStringLiteral("http://purl.oclc.org/ooxml/officeDocument/relationships");
const char *const kEmbedNames[4] = {"embedRegular", "embedBold", "embedItalic", "embedBoldItalic"};

struct Add {
    QString typeface;
    int slot = 0;
    QString rId, key, partName;
};

QString keyFor(const QByteArray &font, const QString &typeface, int slot)
{
    QCryptographicHash h(QCryptographicHash::Sha1);
    h.addData(font);
    h.addData(typeface.toUtf8());
    h.addData(QByteArray(1, char('0' + slot)));
    const QByteArray hex = h.result().toHex().left(32).toUpper();
    return QStringLiteral("{%1-%2-%3-%4-%5}").arg(QString::fromLatin1(hex.mid(0, 8)), QString::fromLatin1(hex.mid(8, 4)), QString::fromLatin1(hex.mid(12, 4)), QString::fromLatin1(hex.mid(16, 4)), QString::fromLatin1(hex.mid(20, 12)));
}

int slotOfElement(const QStringView name)
{
    for (int i = 0; i < 4; ++i)
        if (name == QLatin1String(kEmbedNames[i])) return i;
    return -1;
}

// The font table with the additions made; empty if it can't be (not well formed, a DOCTYPE, or a root that is not the font table).
QByteArray rewrittenTable(const QByteArray &table, const QVector<Add> &adds)
{
    // The additions by family (lower-cased), in the order the families come.
    QHash<QString, QVector<int>> byFamily;
    QStringList order;
    for (int i = 0; i < adds.size(); ++i) {
        const QString key = adds[i].typeface.toLower();
        if (!byFamily.contains(key)) order << key;
        byFamily[key] << i;
    }
    QSet<QString> seen;
    QByteArray out;
    QXmlStreamWriter w(&out);
    QXmlStreamReader r(table);
    r.setEntityExpansionLimit(1);
    QString wPrefix, rPrefix, aPrefix, wUri;   // (aPrefix: the prefix the attributes of the main namespace are written with; an element with no prefix is in the default namespace, an attribute never is)
    int depth = 0, skip = 0;
    bool inFont = false;
    QString fontKey;
    QXmlStreamAttributes held[4];
    bool has[4] = {false, false, false, false};
    auto name = [&](const QString &prefix, const char *local) { return prefix.isEmpty() ? QString::fromLatin1(local) : prefix + QLatin1Char(':') + QLatin1String(local); };
    auto writeNew = [&](const Add &a) {
        w.writeEmptyElement(name(wPrefix, kEmbedNames[a.slot]));
        w.writeAttribute(name(rPrefix, "id"), a.rId);
        w.writeAttribute(name(aPrefix, "fontKey"), a.key);
    };
    while (!r.atEnd()) {
        r.readNext();
        if (r.hasError() || r.isDTD()) return QByteArray();
        if (skip > 0) {
            if (r.isStartElement()) ++skip;
            else if (r.isEndElement()) --skip;
            continue;
        }
        if (r.isStartDocument()) {
            w.writeStartDocument(QStringLiteral("1.0"), true);
        } else if (r.isStartElement()) {
            ++depth;
            if (depth == 1) {
                wUri = r.namespaceUri().toString();
                wPrefix = r.prefix().toString();
                if (r.name() != QLatin1String("fonts")) return QByteArray();
                if (wPrefix.isEmpty()) {   // (the main namespace is the default: the root is written in it, so the elements the reader hands over keep their place)
                    w.writeStartElement(wUri, r.name().toString());
                    w.writeDefaultNamespace(wUri);
                } else {
                    w.writeStartElement(r.qualifiedName().toString());
                }
                bool haveR = false;
                aPrefix = wPrefix;
                QSet<QString> taken;
                for (const QXmlStreamNamespaceDeclaration &d : r.namespaceDeclarations()) {
                    if (wPrefix.isEmpty() && d.prefix().isEmpty()) continue;   // (written above)
                    w.writeNamespace(d.namespaceUri().toString(), d.prefix().toString());
                    taken.insert(d.prefix().toString());
                    if (aPrefix.isEmpty() && !d.prefix().isEmpty() && d.namespaceUri() == wUri) aPrefix = d.prefix().toString();
                    if (rPrefix.isEmpty() && !d.prefix().isEmpty() && (d.namespaceUri() == kRNs || d.namespaceUri() == kStrictRNs)) {
                        rPrefix = d.prefix().toString();
                        haveR = true;
                    }
                }
                if (aPrefix.isEmpty()) {
                    aPrefix = taken.contains(QStringLiteral("w")) ? QStringLiteral("wm") : QStringLiteral("w");
                    taken.insert(aPrefix);
                    w.writeNamespace(wUri, aPrefix);
                }
                if (!haveR) {
                    rPrefix = taken.contains(QStringLiteral("r")) ? QStringLiteral("rl") : QStringLiteral("r");
                    w.writeNamespace(wUri == kWNs ? kRNs : kStrictRNs, rPrefix);
                }
                w.writeAttributes(r.attributes());
            } else if (depth == 2 && r.name() == QLatin1String("font") && r.namespaceUri() == wUri) {
                inFont = true;
                QString own = r.attributes().value(wUri, QLatin1String("name")).toString();
                if (own.isEmpty()) own = r.attributes().value(QLatin1String("name")).toString();   // (some tools write it with no prefix)
                fontKey = own.trimmed().toLower();
                for (int i = 0; i < 4; ++i) {
                    has[i] = false;
                    held[i] = QXmlStreamAttributes();
                }
                w.writeCurrentToken(r);
            } else if (depth == 3 && inFont && r.namespaceUri() == wUri && slotOfElement(r.name()) >= 0) {
                const int slot = slotOfElement(r.name());
                has[slot] = true;
                held[slot] = r.attributes();
                skip = 1;   // (its end comes with it)
                --depth;
            } else {
                w.writeCurrentToken(r);
            }
        } else if (r.isEndElement()) {
            if (depth == 2 && inFont) {
                const QVector<int> mine = seen.contains(fontKey) ? QVector<int>() : byFamily.value(fontKey);
                for (int slot = 0; slot < 4; ++slot) {
                    if (has[slot]) {
                        w.writeEmptyElement(name(wPrefix, kEmbedNames[slot]));
                        w.writeAttributes(held[slot]);
                    } else {
                        for (int i : mine)
                            if (adds[i].slot == slot) writeNew(adds[i]);
                    }
                }
                if (!fontKey.isEmpty()) seen.insert(fontKey);
                inFont = false;
                w.writeEndElement();
            } else if (depth == 1) {
                for (const QString &key : std::as_const(order)) {
                    if (seen.contains(key)) continue;
                    w.writeStartElement(name(wPrefix, "font"));
                    w.writeAttribute(name(aPrefix, "name"), adds[byFamily[key].first()].typeface);
                    for (int slot = 0; slot < 4; ++slot)
                        for (int i : byFamily[key])
                            if (adds[i].slot == slot) writeNew(adds[i]);
                    w.writeEndElement();
                }
                w.writeEndElement();
            } else {
                w.writeCurrentToken(r);
            }
            --depth;
        } else if (r.isEndDocument()) {
            w.writeEndDocument();
        } else if (r.isCharacters() || r.isComment()) {
            if (depth >= 1) w.writeCurrentToken(r);
        }
    }
    if (r.hasError() || depth != 0) return QByteArray();
    return out;
}

} // namespace

FontsEmbedded embedFontsInDocx(Document *doc, const QVector<FontToEmbed> &wanted, const FontFileSource &source, const EmbeddedFontLimits &limits)
{
    FontsEmbedded out;
    if (!doc || wanted.isEmpty()) return out;
    // The font table the document has, and what its parts are called (one pass over the parts).
    QString tableName;
    for (const Relationship &rel : std::as_const(doc->mainRelationships))
        if (!rel.external && rel.type.endsWith(QLatin1String("/fontTable"))) {
            tableName = rel.target;
            break;
        }
    int tableAt = -1;
    QSet<QString> partNames;
    QHash<QString, qint64> partSize;
    for (int i = 0; i < doc->parts.size(); ++i) {
        const QString lower = doc->parts[i].name.toLower();
        partNames.insert(lower);
        partSize.insert(lower, doc->parts[i].data.size());
        if (!tableName.isEmpty() && lower == tableName.toLower()) tableAt = i;
    }
    if (!tableName.isEmpty() && tableAt < 0) return out;   // (a link to a part that is not there: nothing is written)
    const QByteArray table = tableAt >= 0 ? doc->parts[tableAt].data : QByteArray("<w:fonts xmlns:w=\"" + kWNs.toUtf8() + "\" xmlns:r=\"" + kRNs.toUtf8() + "\"></w:fonts>");
    // What the table carries already.
    QSet<QString> carried;
    int carriedFaces = 0;
    qint64 carriedBytes = 0;
    QSet<QString> usedIds;
    if (tableAt >= 0) {
        XmlTree tree;
        if (!tree.parse(table, nullptr)) return out;
        QHash<QString, QString> linkById;
        for (const Relationship &rel : doc->partRelationships.value(tableName)) {
            usedIds.insert(rel.id);
            if (!rel.external) linkById.insert(rel.id, rel.target);
        }
        for (int c = tree.firstChild(tree.root()); c >= 0; c = tree.next(c)) {
            if (!tree.is(c, "font")) continue;
            const QString family = tree.attr(c, "name").trimmed().toLower();
            for (int slot = 0; slot < 4; ++slot) {
                const int embed = tree.child(c, kEmbedNames[slot]);
                if (embed < 0) continue;
                carried.insert(family + QLatin1Char('\t') + QLatin1Char(char('0' + slot)));
                ++carriedFaces;
                carriedBytes += partSize.value(linkById.value(tree.attr(embed, "id", NsR)).toLower());
            }
        }
    } else {
        for (const Relationship &rel : std::as_const(doc->mainRelationships)) usedIds.insert(rel.id);
    }
    // The faces to write.
    QVector<Add> adds;
    QVector<QByteArray> scrambled;
    QSet<QString> asked, forbidden;
    qint64 total = carriedBytes;
    bool tooMany = false, tooLarge = false;
    int looked = 0;
    for (const FontToEmbed &f : wanted) {
        if (++looked > kMostFacesAsked) break;
        const QString typeface = f.typeface.trimmed();
        if (typeface.isEmpty() || typeface.size() > limits.nameChars) continue;
        const int slot = (f.bold ? 1 : 0) | (f.italic ? 2 : 0);
        const QString key = typeface.toLower() + QLatin1Char('\t') + QLatin1Char(char('0' + slot));
        if (carried.contains(key) || asked.contains(key)) continue;
        asked.insert(key);
        if (forbidden.contains(typeface.toLower())) continue;
        const QByteArray font = source ? source(typeface, f.bold, f.italic) : QByteArray();
        if (font.isEmpty()) continue;
        FontFileInfo info;
        if (!inspectFont(font, &info)) continue;
        if (!fontAllowsEmbedding(info.fsType)) {
            forbidden.insert(typeface.toLower());
            out.leftOut << typeface;
            continue;
        }
        if (carriedFaces + adds.size() >= limits.faces) {
            tooMany = true;
            continue;
        }
        if (font.size() > limits.partBytes || total + font.size() > limits.totalBytes) {
            tooLarge = true;
            continue;
        }
        total += font.size();
        Add a;
        a.typeface = typeface;
        a.slot = slot;
        a.key = keyFor(font, typeface, slot);
        adds << a;
        scrambled << unscrambledFont(font, a.key);   // (the same exclusive-or scrambles and unscrambles)
    }
    if (!out.leftOut.isEmpty())
        out.notes << QCoreApplication::translate("jp::office::wml", "These fonts were not saved in the file, because their licenses do not allow it: %1.").arg(out.leftOut.join(QStringLiteral(", ")));
    if (tooMany) out.notes << QCoreApplication::translate("jp::office::wml", "Some fonts were not saved in the file: it may carry at most %1.").arg(limits.faces);
    if (tooLarge) out.notes << QCoreApplication::translate("jp::office::wml", "Some fonts were not saved in the file: they are too large.");
    if (adds.isEmpty()) return out;
    // Names for the parts and links that are not taken (each counter only moves forward).
    int partNumber = 0, idNumber = 0;
    for (int i = 0; i < adds.size(); ++i) {
        QString name;
        do name = QStringLiteral("/word/fonts/font%1.odttf").arg(++partNumber);
        while (partNames.contains(name.toLower()));
        adds[i].partName = name;
        QString id;
        do id = QStringLiteral("rId%1").arg(++idNumber);
        while (usedIds.contains(id));
        adds[i].rId = id;
        usedIds.insert(id);
    }
    const QByteArray rewritten = rewrittenTable(table, adds);
    if (rewritten.isEmpty()) {
        out.notes << QCoreApplication::translate("jp::office::wml", "The fonts were not saved in the file: its font table can't be changed.");
        return out;
    }
    static const QString kFontRel = kRNs + QStringLiteral("/font"), kObfuscated = QStringLiteral("application/vnd.openxmlformats-officedocument.obfuscatedFont");
    if (tableAt >= 0) {
        doc->parts[tableAt].data = rewritten;
    } else {
        tableName = QStringLiteral("/word/fontTable.xml");
        if (partNames.contains(tableName.toLower())) return out;   // (a part of that name that nothing links to: not written over)
        QString id;
        int n = 0;
        QSet<QString> mainIds;
        for (const Relationship &rel : std::as_const(doc->mainRelationships)) mainIds.insert(rel.id);
        do id = QStringLiteral("rId%1").arg(1000 + ++n);
        while (mainIds.contains(id));
        doc->mainRelationships.push_back(Relationship{id, kRNs + QStringLiteral("/fontTable"), tableName, false});
        doc->parts.push_back(KeptPart{tableName, QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"), rewritten});
    }
    QVector<Relationship> &links = doc->partRelationships[tableName];
    for (int i = 0; i < adds.size(); ++i) {
        doc->parts.push_back(KeptPart{adds[i].partName, kObfuscated, scrambled[i]});
        links.push_back(Relationship{adds[i].rId, kFontRel, adds[i].partName, false});
        ++out.faces;
        out.bytes += scrambled[i].size();
    }
    return out;
}

} // namespace jp::office::wml
