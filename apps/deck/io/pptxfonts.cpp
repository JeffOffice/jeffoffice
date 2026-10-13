#include "io/pptxfonts.h"

#include "core/fontfiles.h"
#include "core/fonts.h"
#include "office/eot.h"
#include "office/xml.h"

#include <QCoreApplication>
#include <QFile>
#include <QFontDatabase>
#include <QMap>
#include <QXmlStreamReader>
#include <QtEndian>
#include <algorithm>

namespace jp::deck {

namespace {

QString say(const char *text) { return QCoreApplication::translate("PptxFonts", text); }

const QString kFontRel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/font");
const QString kFontType = QStringLiteral("application/x-fontdata");
const char *const kSlotNames[4] = {"regular", "bold", "italic", "boldItalic"};

bool isTextProperties(const QStringView name) { return name == u"rPr" || name == u"defRPr" || name == u"endParaRPr"; }

bool booleanOn(const QStringView value) { return value == u"1" || value == u"true"; }

// What the typeface says in a part: the theme's heading or body font, or the name itself. Empty for none a font could be.
QString resolved(const QString &typeface, const QString &major, const QString &minor)
{
    const QString t = typeface.trimmed();
    if (t.isEmpty()) return QString();
    if (t == QLatin1String("+mj-lt")) return major;
    if (t == QLatin1String("+mn-lt")) return minor;
    if (t.startsWith(QLatin1Char('+'))) return QString();   // the theme's East Asian and complex-script fonts, and anything else of that form
    return t;
}

QString hex(const quint8 *bytes, int n)
{
    QString out;
    for (int i = 0; i < n; ++i) out += QStringLiteral("%1").arg(bytes[i], 2, 16, QLatin1Char('0')).toUpper();
    return out;
}

} // namespace

void FontUse::add(const QString &typeface, bool bold, bool italic)
{
    if (typeface.isEmpty()) return;
    const QString key = typeface.toLower();
    if (!m_names.contains(key)) m_names.insert(key, typeface);
    m_faces.insert(key + QLatin1Char('\t') + QLatin1Char(char('0' + (bold ? 1 : 0) + (italic ? 2 : 0))));
}

void FontUse::addThemeFonts(const QString &major, const QString &minor)
{
    add(major.trimmed(), false, false);
    add(minor.trimmed(), false, false);
}

void FontUse::scan(const QByteArray &xml, const QString &major, const QString &minor)
{
    QXmlStreamReader r(xml);
    struct Properties {
        bool open = false, bold = false, italic = false;
        QStringList faces;
    } cur;
    auto finish = [&] {
        if (!cur.open) return;
        // No font named: the text takes what its place gives it, which is the theme's heading or body font.
        if (cur.faces.isEmpty())
            for (const QString &theme : {major, minor}) add(theme.trimmed(), cur.bold, cur.italic);
        for (const QString &face : std::as_const(cur.faces)) add(face, cur.bold, cur.italic);
        cur = Properties();
    };
    while (!r.atEnd()) {
        r.readNext();
        if (r.isStartElement()) {
            if (!office::sameNamespace(r.namespaceUri().toString(), office::ns::A)) continue;
            const QStringView name = r.name();
            if (isTextProperties(name)) {
                finish();
                cur.open = true;
                cur.bold = booleanOn(r.attributes().value(u"b"));
                cur.italic = booleanOn(r.attributes().value(u"i"));
            } else if (cur.open && (name == u"latin" || name == u"ea" || name == u"cs" || name == u"sym")) {
                const QString face = resolved(r.attributes().value(u"typeface").toString(), major, minor);
                if (!face.isEmpty()) cur.faces << face;
            } else if (name == u"buFont") {
                add(resolved(r.attributes().value(u"typeface").toString(), major, minor), false, false);
            }
        } else if (r.isEndElement() && cur.open && isTextProperties(r.name()) && office::sameNamespace(r.namespaceUri().toString(), office::ns::A)) {
            finish();
        }
    }
    finish();
}

QVector<FontUse::Face> FontUse::faces() const
{
    QVector<Face> out;
    for (const QString &key : m_faces) {
        const QString lower = key.section(QLatin1Char('\t'), 0, 0);
        const int style = key.section(QLatin1Char('\t'), 1, 1).toInt();
        out << Face{m_names.value(lower), bool(style & 1), bool(style & 2)};
    }
    std::sort(out.begin(), out.end(), [](const Face &a, const Face &b) {
        const int c = a.typeface.compare(b.typeface, Qt::CaseInsensitive);
        if (c != 0) return c < 0;
        return std::make_pair(a.bold, a.italic) < std::make_pair(b.bold, b.italic);
    });
    return out;
}

FontsWritten writeEmbeddedFonts(office::PackageWriter &pkg, const QString &presentationPart, const FontUse &use, const QVector<EmbeddedFace> &kept, bool embedBundled)
{
    FontsWritten out;
    // The faces by family, in the order the program lists them (by name), each with the styles the text uses.
    struct FamilyUse {
        QString typeface;
        bool slot[4] = {false, false, false, false};
    };
    QMap<QString, FamilyUse> families;
    for (const FontUse::Face &f : use.faces()) {
        FamilyUse &family = families[f.typeface.toLower()];
        family.typeface = f.typeface;
        family.slot[fontSlot(f.bold, f.italic)] = true;
    }
    QString list;
    int number = 0;
    for (auto it = families.cbegin(); it != families.cend(); ++it) {
        const QString &typeface = it->typeface;
        QString faces;
        QString fontAttributes;
        for (int slot = 0; slot < 4; ++slot) {
            if (!it->slot[slot]) continue;
            const bool bold = slot & 1, italic = slot & 2;
            QByteArray part;
            FontFileInfo info;
            QString panose;
            int pitch = 0, charset = 0;
            bool haveAttributes = false;
            if (embedBundled) {
                const BundledFontFile file = bundledFontFile(typeface, bold, italic);
                if (!file.path.isEmpty()) {
                    QFile f(file.path);
                    QByteArray font = f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
                    if (italic) {
                        // Letters that slant under flags that say upright are embedded with the flags right.
                        if (const QByteArray fixed = fontWithItalicFlags(font); !fixed.isEmpty()) font = fixed;
                    }
                    // A font whose license doesn't let it travel is left out (the file then says nothing of that face).
                    if (fontAllowsEmbedding(file.fsType) && inspectFont(font, &info)) {
                        part = office::makeEot(font);
                        if (!part.isEmpty()) {
                            const bool anyPanose = std::any_of(std::begin(info.panose), std::end(info.panose), [](quint8 b) { return b != 0; });
                            panose = anyPanose ? hex(info.panose, 10) : QString();
                            pitch = info.pitchFamily();
                            charset = info.charset();
                            haveAttributes = true;
                        }
                    }
                }
            }
            if (part.isEmpty()) {
                // What the file carried for it, as it was.
                for (const EmbeddedFace &k : kept)
                    if (k.slot == slot && k.typeface.compare(typeface, Qt::CaseInsensitive) == 0 && !k.part.isEmpty()) {
                        part = k.part;
                        // A bare font file (as some programs write the part) is not read by the presentation program: it wants the wrapped form.
                        if (part.size() > 4 && (qFromBigEndian<quint32>(part.constData()) == 0x00010000 || part.startsWith("true") || part.startsWith("OTTO"))) part = office::makeEot(part);
                        panose = k.panose;
                        pitch = k.pitchFamily;
                        charset = k.charset;
                        haveAttributes = true;
                        break;
                    }
            }
            if (part.isEmpty()) continue;
            const QString name = QStringLiteral("/ppt/fonts/font%1.fntdata").arg(++number);
            pkg.addDefault(QStringLiteral("fntdata"), kFontType);
            pkg.addPart(name, part, kFontType);
            const QString rId = pkg.addRelationship(presentationPart, kFontRel, name);
            faces += QStringLiteral("<p:%1 r:id=\"%2\"/>").arg(QLatin1String(kSlotNames[slot]), rId);
            ++out.faces;
            out.bytes += part.size();
            if (fontAttributes.isEmpty() && haveAttributes)
                fontAttributes = (panose.isEmpty() ? QString() : QStringLiteral(" panose=\"%1\"").arg(panose)) + QStringLiteral(" pitchFamily=\"%1\" charset=\"%2\"").arg(pitch).arg(charset);
        }
        if (faces.isEmpty()) continue;
        list += QStringLiteral("<p:embeddedFont><p:font typeface=\"%1\"%2/>%3</p:embeddedFont>").arg(typeface.toHtmlEscaped(), fontAttributes, faces);
    }
    if (!list.isEmpty()) out.listXml = QStringLiteral("<p:embeddedFontLst>%1</p:embeddedFontLst>").arg(list);
    return out;
}

QVector<EmbeddedFace> readEmbeddedFonts(const office::Package &pkg, const QString &presentationPart, const QDomElement &presentation, QStringList *notes)
{
    QVector<EmbeddedFace> faces;
    const QDomElement list = office::child(presentation, office::ns::P, QStringLiteral("embeddedFontLst"));
    if (list.isNull()) return faces;
    qint64 total = 0;
    int unusable = 0;
    QString firstReason;
    bool tooMany = false, tooLarge = false;
    for (const QDomElement &entry : office::children(list, office::ns::P, QStringLiteral("embeddedFont"))) {
        const QDomElement font = office::child(entry, office::ns::P, QStringLiteral("font"));
        const QString typeface = office::attr(font, QStringLiteral("typeface")).trimmed();
        if (typeface.isEmpty()) continue;
        for (int slot = 0; slot < 4; ++slot) {
            const QDomElement slotElement = office::child(entry, office::ns::P, QString::fromLatin1(kSlotNames[slot]));
            if (slotElement.isNull()) continue;
            const office::Relationship rel = pkg.relationship(presentationPart, office::attrNs(slotElement, office::ns::R, QStringLiteral("id")));
            if (rel.id.isEmpty() || rel.external || !pkg.hasPart(rel.target)) continue;
            if (faces.size() >= kMaxEmbeddedFonts) {
                tooMany = true;
                continue;
            }
            const QByteArray data = pkg.part(rel.target);
            if (data.size() > kMaxEmbeddedFontBytes + 65536 || total + data.size() > kMaxEmbeddedFontsTotal) {
                tooLarge = true;
                continue;
            }
            total += data.size();
            EmbeddedFace face;
            face.typeface = typeface;
            face.panose = office::attr(font, QStringLiteral("panose"));
            face.pitchFamily = int(office::parseInt(office::attr(font, QStringLiteral("pitchFamily")), 0));
            face.charset = int(office::parseInt(office::attr(font, QStringLiteral("charset")), 0));
            face.slot = FontSlot(slot);
            face.part = data;
            QString why;
            if (!office::readEmbeddedFont(data, &face.font, &face.info, &why)) {
                face.font.clear();
                // A font the computer has costs nothing to lose.
                if (!QFontDatabase::families().contains(typeface, Qt::CaseInsensitive)) {
                    ++unusable;
                    if (firstReason.isEmpty()) firstReason = why;
                }
            }
            faces << face;
        }
    }
    if (notes) {
        if (unusable == 1) *notes << say("An embedded font in the file can't be used: %1").arg(firstReason);
        else if (unusable > 1) *notes << say("%1 embedded fonts in the file can't be used: %2").arg(unusable).arg(firstReason);
        if (tooMany) *notes << say("Only the first %1 embedded fonts are read.").arg(kMaxEmbeddedFonts);
        if (tooLarge) *notes << say("Some embedded fonts are left out: they are too large.");
    }
    return faces;
}

QVector<EmbeddedFace> readEmbeddedFontFiles(const office::Package &pkg, const QString &folder, QStringList *notes)
{
    QVector<EmbeddedFace> faces;
    qint64 total = 0;
    int unusable = 0;
    bool tooMany = false, tooLarge = false;
    const QString prefix = folder.endsWith(QLatin1Char('/')) ? folder : folder + QLatin1Char('/');
    for (const QString &name : pkg.partNames()) {
        if (!name.startsWith(prefix, Qt::CaseInsensitive)) continue;
        const QString lower = name.toLower();
        if (!lower.endsWith(QLatin1String(".ttf")) && !lower.endsWith(QLatin1String(".otf"))) continue;
        if (faces.size() >= kMaxEmbeddedFonts) {
            tooMany = true;
            continue;
        }
        const QByteArray data = pkg.part(name);
        if (data.size() > kMaxEmbeddedFontBytes || total + data.size() > kMaxEmbeddedFontsTotal) {
            tooLarge = true;
            continue;
        }
        total += data.size();
        EmbeddedFace face;
        QString why;
        if (!office::readEmbeddedFont(data, &face.font, &face.info, &why) || face.info.family.isEmpty()) {
            ++unusable;
            continue;
        }
        face.typeface = face.info.family;
        face.slot = fontSlot(face.info.bold, face.info.italic);
        face.panose = QString();
        face.pitchFamily = face.info.pitchFamily();
        face.charset = face.info.charset();
        // Kept for a presentation file, which the program reads only in the wrapped form.
        face.part = office::makeEot(face.font);
        faces << face;
    }
    if (notes) {
        if (unusable) *notes << QCoreApplication::translate("PptxFonts", "%n embedded font(s) in the file can't be used.", nullptr, unusable);
        if (tooMany) *notes << say("Only the first %1 embedded fonts are read.").arg(kMaxEmbeddedFonts);
        if (tooLarge) *notes << say("Some embedded fonts are left out: they are too large.");
    }
    return faces;
}

} // namespace jp::deck
