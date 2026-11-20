// Tests of libs/office/odt: an OpenDocument text read into the parts of a .docx and a .docx written as an OpenDocument text. Every document is built here from a few lines of XML.

#include "core/fonts.h"
#include "core/wrapoutline.h"
#include "io/zip.h"
#include "io/zipindex.h"
#include "odt/odt.h"
#include "odt/odtcommon.h"
#include "odt/odttemplate.h"
#include "office/chart.h"
#include "office/chart_write.h"
#include "office/theme.h"
#include "sheet/odschart.h"
#include "office/package.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <cmath>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QTest>

using namespace jp::office;
using namespace jp::office::odt;

namespace {

const char kNs[] =
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
    " xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\""
    " xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"";

const char kMime[] = "application/vnd.oasis.opendocument.text";

// An .odt: `body` goes in office:text, `autoStyles` in content.xml's automatic styles, `styles` in styles.xml's common styles, `masters` and `layouts` in its master pages and page layouts.
struct Made {
    QByteArray body, autoStyles, styles, masters, layouts, meta;
    QMap<QString, QByteArray> extra;
    QByteArray mime = kMime;
    QByteArray build() const
    {
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), mime, false);
        z.add(QStringLiteral("content.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-content") + kNs + " office:version=\"1.2\"><office:automatic-styles>" + autoStyles +
                  "</office:automatic-styles><office:body><office:text>" + body + "</office:text></office:body></office:document-content>",
              true);
        z.add(QStringLiteral("styles.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-styles") + kNs + " office:version=\"1.2\"><office:styles>" + styles + "</office:styles><office:automatic-styles>" + layouts +
                  "</office:automatic-styles><office:master-styles>" + masters + "</office:master-styles></office:document-styles>",
              true);
        if (!meta.isEmpty()) z.add(QStringLiteral("meta.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-meta") + kNs + " office:version=\"1.2\"><office:meta>" + meta + "</office:meta></office:document-meta>", true);
        z.add(QStringLiteral("META-INF/manifest.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\"/>"), true);
        for (auto it = extra.cbegin(); it != extra.cend(); ++it) z.add(it.key(), it.value(), false);
        return z.finish();
    }
};

const QByteArray kPng = QByteArray::fromBase64("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==");

// The .docx the reader makes, and one of its parts.
QByteArray docxOf(const QByteArray &odt, QStringList *notes = nullptr, QString *error = nullptr)
{
    return toDocxPackage(odt, error, notes);
}

QString part(const QByteArray &docx, const QString &name)
{
    Package p;
    if (!p.read(docx)) return QString();
    return QString::fromUtf8(p.part(name));
}

QString document(const Made &m, QStringList *notes = nullptr)
{
    const QByteArray docx = docxOf(m.build(), notes);
    return part(docx, QStringLiteral("/word/document.xml"));
}

// Well formed as a strict parser reads it (a repeated attribute, which the light tree lets by, is refused: another program refuses the file).
bool wellFormed(const QByteArray &xml)
{
    QXmlStreamReader r(xml);
    while (!r.atEnd()) r.readNext();
    return !r.hasError();
}

// The first element of the XML with this local name, found by walking the tree (-1 for none): for checks of what a value is, not how the file spells it.
int findElement(const wml::XmlTree &t, const char *local, int from = -1)
{
    for (int i = from + 1; i < t.count(); ++i)
        if (t.localName(i) == local) return i;
    return -1;
}

QString instructionsOf(const QString &documentXml)
{
    QString all;
    wml::XmlTree t;
    if (!t.parse(documentXml.toUtf8(), nullptr)) return all;
    for (int i = findElement(t, "instrText"); i >= 0; i = findElement(t, "instrText", i)) all += t.text(i) + QLatin1Char('\n');
    return all;
}

// A .docx made of this body (and, if given, more parts), written as an .odt: its content.xml, and what was not kept.
QString odtContentOf(const QByteArray &bodyXml, QStringList *notes = nullptr, const QByteArray &extraNs = QByteArray())
{
    PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"") + extraNs + "><w:body>" + bodyXml + "</w:body></w:document>",
              QStringLiteral("application/xml"));
    w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
    QMap<QString, QByteArray> files;
    if (!jp::readZip(fromDocxPackage(w.finish(), nullptr, notes), files)) return QString();
    return QString::fromUtf8(files.value(QStringLiteral("content.xml")));
}

// A .docx made of this body, written as an .odt: content.xml and styles.xml together (where each style the paragraphs use is written is not the test's business).
QString odtStylesAndContentOf(const QByteArray &bodyXml)
{
    PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><w:body>") + bodyXml +
                  "</w:body></w:document>",
              QStringLiteral("application/xml"));
    w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
    QMap<QString, QByteArray> files;
    if (!jp::readZip(fromDocxPackage(w.finish()), files)) return QString();
    return QString::fromUtf8(files.value(QStringLiteral("content.xml")) + files.value(QStringLiteral("styles.xml")));
}

// A .docx made of this body, written as an .odt and read again: the document part it comes back as.
QString roundTripBody(const QByteArray &bodyXml)
{
    PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><w:body>") + bodyXml +
                  "</w:body></w:document>",
              QStringLiteral("application/xml"));
    w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
    return part(docxOf(fromDocxPackage(w.finish())), QStringLiteral("/word/document.xml"));
}

// A .docx made of this body (the VML and drawing prefixes declared; `link`, if given, is the external target of rId9), written as an .odt.
QByteArray odtOfDrawing(const QByteArray &bodyXml, const QString &link = QString(), QStringList *notes = nullptr)
{
    PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:v=\"urn:schemas-microsoft-com:vml\" "
                         "xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                         "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><w:body>") + bodyXml + "</w:body></w:document>",
              QStringLiteral("application/xml"));
    w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
    if (!link.isEmpty())
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId9"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/image"), link, true);
    return fromDocxPackage(w.finish(), nullptr, notes);
}

QMap<QString, QByteArray> filesOf(const QByteArray &zip)
{
    QMap<QString, QByteArray> files;
    if (!jp::readZip(zip, files)) files.clear();
    return files;
}

// The content.xml a program that knows nothing of ours reads: every jo: attribute taken out first.
QString standardContentOf(const QMap<QString, QByteArray> &files)
{
    QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
    static const QRegularExpression ours(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\""));
    c.remove(ours);
    return c;
}

// A length of the file as inches (the files write them as "1.3889in").
double inchesOf(const QString &length)
{
    return length.endsWith(QLatin1String("in")) ? length.chopped(2).toDouble() : -1e9;
}

// The graphic properties of the automatic style this name gives, as a map.
QMap<QString, QString> graphicPropertiesOf(const wml::XmlTree &t, const QString &styleName)
{
    QMap<QString, QString> out;
    for (int s = findElement(t, "style"); s >= 0; s = findElement(t, "style", s)) {
        if (t.attrAny(s, "name") != styleName) continue;
        for (int g = t.firstChild(s); g >= 0; g = t.next(g))
            if (t.localName(g) == "graphic-properties") {
                for (const auto &a : t.attributes(g)) out.insert(QString::fromLatin1(a.local), a.value);
            }
        break;
    }
    return out;
}

QByteArray linkedPictureBody()
{
    return "<w:p><w:r><w:drawing><wp:inline><wp:extent cx=\"914400\" cy=\"457200\"/><wp:docPr id=\"1\" name=\"Linked\"/><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic>"
           "<pic:nvPicPr><pic:cNvPr id=\"1\" name=\"Linked\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:link=\"rId9\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>"
           "<pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"914400\" cy=\"457200\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>";
}

const char kOlderShapes[] =
    "<w:p><w:r><w:pict><v:rect id=\"r1\" style=\"position:absolute;margin-left:72pt;margin-top:36pt;width:144pt;height:72pt;z-index:251660288\" filled=\"t\" fillcolor=\"#ff0000\" stroked=\"t\" strokecolor=\"#0000ff\" strokeweight=\"2pt\"><w10:wrap type=\"square\"/></v:rect></w:pict></w:r>"
    "<w:r><w:pict><v:oval id=\"o1\" style=\"position:absolute;margin-left:10pt;margin-top:20pt;width:50pt;height:40pt;z-index:-251657216\" filled=\"f\" stroked=\"f\"></v:oval></w:pict></w:r>"
    "<w:r><w:pict><v:line id=\"l1\" style=\"position:absolute;margin-left:100pt;margin-top:200pt;width:150pt;height:0pt;z-index:251660288\" from=\"0pt,0pt\" to=\"150pt,0pt\" strokeweight=\"1pt\"/></w:pict></w:r>"
    "<w:r><w:pict><v:shape type=\"#_x0000_t202\" id=\"t1\" style=\"position:absolute;margin-left:0pt;margin-top:0pt;width:100pt;height:50pt;z-index:251660288\" filled=\"f\" stroked=\"f\"><v:textbox inset=\"7.2pt,3.6pt,7.2pt,3.6pt\">"
    "<w:txbxContent><w:p><w:r><w:t>Boxed words</w:t></w:r></w:p></w:txbxContent></v:textbox></v:shape></w:pict></w:r></w:p>";

// A .docx of this body with a settings part (its inner XML), written as an .odt.
QByteArray odtOfBodyWithSettings(const QByteArray &bodyXml, const QByteArray &settingsInner)
{
    const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), "<w:document " + ns + "><w:body>" + bodyXml + "</w:body></w:document>", QStringLiteral("application/xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/settings.xml"), "<w:settings " + ns + ">" + settingsInner + "</w:settings>", QStringLiteral("application/xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId8"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    return fromDocxPackage(w.finish());
}

// A part of the .odt as a program that knows nothing of ours reads it: every attribute of ours taken out.
QString standardPartOf(const QMap<QString, QByteArray> &files, const QString &name)
{
    QString c = QString::fromUtf8(files.value(name));
    static const QRegularExpression ours(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\""));
    c.remove(ours);
    return c;
}

// A length of the file in inches, whatever unit it is written in.
double lengthInches(const QString &length)
{
    static const QRegularExpression re(QStringLiteral("^\\s*(-?[0-9.]+(?:[eE][-+]?[0-9]+)?)\\s*(in|cm|mm|pt)\\s*$"));
    const QRegularExpressionMatch m = re.match(length);
    if (!m.hasMatch()) return -1e9;
    const double v = m.captured(1).toDouble();
    const QString u = m.captured(2);
    return u == QLatin1String("in") ? v : u == QLatin1String("cm") ? v / 2.54 : u == QLatin1String("mm") ? v / 25.4 : v / 72.0;
}

// What the standard's transform does to a box of this size (in inches) set at the origin: the functions apply in the order written (as every program writes and reads it; a turn is against the clock,
// about the box's own top left, and the translation then moves it). Where the box's center goes, the angle written, and the mirroring (a negative scale).
struct TurnedBox {
    bool ok = false;
    double centerX = 0, centerY = 0, angle = 0;
    bool flipX = false, flipY = false;
};

TurnedBox turnedBoxOf(const QString &transform, double w, double h)
{
    TurnedBox r;
    double x = w / 2, y = h / 2;
    static const QRegularExpression fn(QStringLiteral("([A-Za-z]+)\\s*\\(([^)]*)\\)"));
    r.ok = !transform.isEmpty();
    for (auto it = fn.globalMatch(transform); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        const QStringList a = m.captured(2).split(QRegularExpression(QStringLiteral("[\\s,]+")), Qt::SkipEmptyParts);
        const QString name = m.captured(1);
        if (name == QLatin1String("scale") && a.size() >= 1) {
            const double sx = a[0].toDouble(), sy = a.size() > 1 ? a[1].toDouble() : sx;
            x *= sx;
            y *= sy;
            r.flipX = r.flipX || sx < 0;
            r.flipY = r.flipY || sy < 0;
        } else if (name == QLatin1String("rotate") && a.size() == 1) {
            const double ang = a[0].toDouble();
            r.angle += ang;
            const double nx = x * std::cos(ang) + y * std::sin(ang), ny = -x * std::sin(ang) + y * std::cos(ang);
            x = nx;
            y = ny;
        } else if (name == QLatin1String("translate") && a.size() >= 1) {
            x += lengthInches(a[0]);
            y += a.size() > 1 ? lengthInches(a[1]) : 0;
        } else r.ok = false;
    }
    r.centerX = x;
    r.centerY = y;
    return r;
}

// A drawn shape of the older form: kind "rect", "oval", "box" (a text box), or "picture" (the linked one), a box of 144 by 72 points set at 72 and 36 points, with more style.
QByteArray vmlShape(const QString &kind, const QString &moreStyle = QString(), const QString &name = QStringLiteral("s1"))
{
    const QString style = QStringLiteral("position:absolute;margin-left:72pt;margin-top:36pt;width:144pt;height:72pt;z-index:251660288") + (moreStyle.isEmpty() ? QString() : QLatin1Char(';') + moreStyle);
    if (kind == QLatin1String("rect") || kind == QLatin1String("oval"))
        return QStringLiteral("<v:%1 id=\"%2\" style=\"%3\" filled=\"t\" fillcolor=\"#ff0000\" stroked=\"f\"/>").arg(kind == QLatin1String("rect") ? QStringLiteral("rect") : QStringLiteral("oval"), name, style).toUtf8();
    if (kind == QLatin1String("box"))
        return QStringLiteral("<v:shape type=\"#_x0000_t202\" id=\"%1\" style=\"%2\" filled=\"f\" stroked=\"f\"><v:textbox inset=\"0,0,0,0\"><w:txbxContent><w:p><w:r><w:t>Boxed</w:t></w:r></w:p></w:txbxContent></v:textbox></v:shape>").arg(name, style).toUtf8();
    return QStringLiteral("<v:shape type=\"#_x0000_t75\" id=\"%1\" style=\"%2\"><v:imagedata r:id=\"rId9\"/></v:shape>").arg(name, style).toUtf8();
}

QByteArray pictBody(const QByteArray &shapes)
{
    return "<w:p><w:r><w:pict>" + shapes + "</w:pict></w:r></w:p>";
}

// The graphic or text-box element of a written shape of this kind, found by its own kind (not by where it stands): the shape element, and the frame that holds it for a text box and a picture.
int shapeNodeOf(const wml::XmlTree &t, const QString &kind, int *frame)
{
    *frame = -1;
    if (kind == QLatin1String("rect")) return findElement(t, "rect");
    if (kind == QLatin1String("oval")) return findElement(t, "ellipse");
    const int inner = findElement(t, kind == QLatin1String("box") ? "text-box" : "image");
    if (inner >= 0) *frame = t.parent(inner);
    return *frame;
}

// The numbers the drawing part of a .docx holds for its first shape: the turn in degrees (60000ths read), the mirroring, and where its box is set (inches from the anchor).
struct ShapeInDocx {
    bool found = false;
    double turn = 0, left = -1e9, top = -1e9;
    bool flipH = false, flipV = false;
};

ShapeInDocx firstShapeOfDocx(const QByteArray &odt)
{
    ShapeInDocx s;
    wml::XmlTree d;
    if (!d.parse(part(docxOf(odt), QStringLiteral("/word/document.xml")).toUtf8(), nullptr)) return s;
    const int xfrm = findElement(d, "xfrm");
    if (xfrm < 0) return s;
    s.found = true;
    s.turn = d.attr(xfrm, "rot").toDouble() / 60000.0;
    s.flipH = d.attr(xfrm, "flipH") == QLatin1String("1");
    s.flipV = d.attr(xfrm, "flipV") == QLatin1String("1");
    for (int i = findElement(d, "posOffset"); i >= 0; i = findElement(d, "posOffset", i)) {
        const QByteArray holder = d.localName(d.parent(i));
        if (holder == "positionH") s.left = d.text(i).toDouble() / 914400.0;
        else if (holder == "positionV") s.top = d.text(i).toDouble() / 914400.0;
    }
    return s;
}

// The paragraphs of a document part, each as its runs' text with "\f" for a page break run (and "^" where the paragraph asks to begin a page).
QStringList paragraphsOf(const QString &documentXml)
{
    QStringList out;
    wml::XmlTree t;
    if (!t.parse(documentXml.toUtf8(), nullptr)) return out;
    for (int p = findElement(t, "p"); p >= 0; p = findElement(t, "p", p)) {
        QString text;
        if (const int pPr = t.child(p, "pPr"); pPr >= 0 && t.child(pPr, "pageBreakBefore") >= 0) text += QLatin1Char('^');
        for (int r = t.firstChild(p); r >= 0; r = t.next(r)) {
            if (!t.is(r, "r")) continue;
            for (int c = t.firstChild(r); c >= 0; c = t.next(c)) {
                if (t.is(c, "br") && t.attr(c, "type") == QLatin1String("page")) text += QLatin1Char('\f');
                else if (t.is(c, "t")) text += t.text(c);
            }
        }
        out << text;
    }
    return out;
}


// The paragraphs of the .odt content.xml a program that knows nothing of ours reads: every jo: attribute taken out first, then each text:p and text:h in the order of the document (the ones in tables too) as
// its text, with "^" in front when its style says it begins a page.
QStringList standardParagraphsOf(const QString &bodyXmlOfDocx)
{
    QString c = odtStylesAndContentOf(bodyXmlOfDocx.toUtf8());
    static const QRegularExpression ours(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\""));
    c.remove(ours);
    QStringList out;
    QXmlStreamReader r(c.toUtf8());
    QSet<QString> breaks;
    QString styleName;
    QString text, style;
    int depth = 0;
    bool inBody = false;
    while (!r.atEnd()) {
        r.readNext();
        if (r.isStartElement()) {
            const QString name = r.name().toString();
            if (name == QLatin1String("style")) styleName = r.attributes().value(QStringLiteral("style:name")).toString();
            else if (name == QLatin1String("paragraph-properties") && r.attributes().value(QStringLiteral("fo:break-before")) == QLatin1String("page")) breaks.insert(styleName);
            else if (name == QLatin1String("text")) inBody = true;
            else if ((name == QLatin1String("p") || name == QLatin1String("h")) && inBody && depth++ == 0) {
                text.clear();
                style = r.attributes().value(QStringLiteral("text:style-name")).toString();
            }
        } else if (r.isCharacters() && depth > 0) {
            text += r.text().toString();
        } else if (r.isEndElement() && (r.name() == QLatin1String("p") || r.name() == QLatin1String("h")) && inBody && depth > 0 && --depth == 0) {
            out << (breaks.contains(style) ? QStringLiteral("^") : QString()) + text;
        }
    }
    return out;
}

// ---- the standard's schema, when the machine has it (a developer's checkout; the build machines do not, and the tests that use it then skip)

const char kSchemaFile[] = "/home/user/jo-deps-dl/odf/1.2/OpenDocument-v1.2-os-schema.rng";

// What the schema says of the content.xml and styles.xml of a written document. Attributes and elements in a namespace that is not the standard's (ours, jo:, and the other program's names that
// the format has no words for) make a document an "extended" one by the standard's own terms: they are taken out before the check and listed apart, so that what is left is the standard's own part.
struct OdfCheck {
    bool ran = false;
    QString skip;
    QStringList errors, privateNames;
};

OdfCheck checkAgainstSchema(const QByteArray &odt)
{
    OdfCheck r;
    if (!QFileInfo::exists(QString::fromLatin1(kSchemaFile))) {
        r.skip = QStringLiteral("the standard's schema file is not on this machine");
        return r;
    }
    const QString python = QStandardPaths::findExecutable(QStringLiteral("python3"));
    if (python.isEmpty()) {
        r.skip = QStringLiteral("python3 is not on this machine");
        return r;
    }
    const QMap<QString, QByteArray> files = filesOf(odt);
    QTemporaryDir dir;
    if (!dir.isValid() || files.isEmpty()) {
        r.skip = QStringLiteral("no folder to write the parts in");
        return r;
    }
    QStringList parts;
    for (const QString &name : {QStringLiteral("content.xml"), QStringLiteral("styles.xml")}) {
        QFile f(dir.filePath(name));
        if (!f.open(QIODevice::WriteOnly) || f.write(files.value(name)) < 0) {
            r.skip = QStringLiteral("no folder to write the parts in");
            return r;
        }
        parts << f.fileName();
    }
    // (The checker is written beside the parts; it is run isolated, with only the schema file named.)
    static const char script[] =
        "import json, sys\n"
        "try:\n"
        "    from lxml import etree\n"
        "except Exception:\n"
        "    print(json.dumps({'skip': 'lxml is not installed'})); sys.exit(0)\n"
        "std = ('urn:oasis:names:tc:opendocument:', 'http://www.w3.org/1999/xlink', 'http://purl.org/dc/elements/1.1/', 'http://www.w3.org/1998/Math/MathML', 'http://www.w3.org/XML/1998/namespace')\n"
        "def standard(tag):\n"
        "    if not tag.startswith('{'): return True\n"
        "    return tag[1:].split('}')[0].startswith(std)\n"
        "out = {'errors': [], 'private': []}\n"
        "schema = etree.RelaxNG(etree.parse(sys.argv[1]))\n"
        "for path in sys.argv[2:]:\n"
        "    tree = etree.parse(path)\n"
        "    for e in list(tree.iter()):\n"
        "        if not isinstance(e.tag, str): continue\n"
        "        if not standard(e.tag):\n"
        "            out['private'].append(e.tag)\n"
        "            e.getparent().remove(e)\n"
        "            continue\n"
        "        for k in list(e.attrib):\n"
        "            if not standard(k):\n"
        "                out['private'].append(k)\n"
        "                del e.attrib[k]\n"
        "    if not schema.validate(tree):\n"
        "        for m in schema.error_log: out['errors'].append(path.rsplit('/', 1)[-1] + ' line ' + str(m.line) + ': ' + m.message[:300])\n"
        "print(json.dumps(out))\n";
    QFile s(dir.filePath(QStringLiteral("check.py")));
    if (!s.open(QIODevice::WriteOnly) || s.write(script) < 0) {
        r.skip = QStringLiteral("no folder to write the checker in");
        return r;
    }
    s.close();
    QProcess p;
    p.setProgram(python);
    p.setArguments(QStringList{QStringLiteral("-I"), s.fileName(), QString::fromLatin1(kSchemaFile)} + parts);
    p.start();
    if (!p.waitForFinished(120000)) {
        p.kill();
        p.waitForFinished();
        r.skip = QStringLiteral("the checker did not finish");
        return r;
    }
    const QJsonObject o = QJsonDocument::fromJson(p.readAllStandardOutput()).object();
    if (o.isEmpty()) {
        r.skip = QStringLiteral("the checker gave no answer: ") + QString::fromUtf8(p.readAllStandardError().left(200));
        return r;
    }
    if (o.contains(QLatin1String("skip"))) {
        r.skip = o.value(QStringLiteral("skip")).toString();
        return r;
    }
    r.ran = true;
    for (const QJsonValue &v : o.value(QStringLiteral("errors")).toArray()) r.errors << v.toString();
    for (const QJsonValue &v : o.value(QStringLiteral("private")).toArray()) {
        const QString name = v.toString();
        if (!r.privateNames.contains(name)) r.privateNames << name;
    }
    return r;
}

} // namespace

class OdtTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // Heavy dashed underlines keep their line and their weight both ways: the bold long-dash, dot-dash, and dot-dot-dash of an .odt are the heavy ones of a .docx, and back.
    void heavyDashedUnderlinesSurviveBothWays()
    {
        struct Row { const char *style, *width, *val; };
        const Row rows[] = {{"long-dash", "bold", "dashLongHeavy"}, {"dot-dash", "bold", "dashDotHeavy"}, {"dot-dot-dash", "bold", "dashDotDotHeavy"},
                            {"long-dash", "auto", "dashLong"}, {"dot-dash", "auto", "dotDash"}, {"dot-dot-dash", "auto", "dotDotDash"}, {"dash", "bold", "dashedHeavy"}};
        for (const Row &r : rows) {
            Made m;
            m.autoStyles = QByteArray("<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties style:text-underline-style=\"") + r.style +
                           "\" style:text-underline-type=\"single\" style:text-underline-width=\"" + r.width + "\"/></style:style>";
            m.body = "<text:p><text:span text:style-name=\"T1\">x</text:span></text:p>";
            const QString d = document(m);
            QVERIFY2(d.contains(QLatin1String("<w:u w:val=\"") + QLatin1String(r.val) + QLatin1Char('"')), r.style + QByteArray(" ") + r.width + ": " + d.toUtf8());
            const QString back = odtContentOf(QByteArray("<w:p><w:r><w:rPr><w:u w:val=\"") + r.val + "\"/></w:rPr><w:t>x</w:t></w:r></w:p>");
            QVERIFY2(back.contains(QLatin1String("style:text-underline-style=\"") + QLatin1String(r.style) + QLatin1Char('"')), r.val);
            QCOMPARE(back.contains(QLatin1String("style:text-underline-width=\"bold\"")), QLatin1String(r.width) == QLatin1String("bold"));
        }
    }

    // A package written as a template: the type in "mimetype" (first, stored) and in the manifest's root entry is the template's; every other part is the same.
    void aPackageIsWrittenAsATemplate()
    {
        const QByteArray manifest = "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\"><manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.2\" "
                                    "manifest:media-type=\"application/vnd.oasis.opendocument.text\"/><manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/></manifest:manifest>";
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), kMime, false);
        z.add(QStringLiteral("content.xml"), "<office:document-content/>", true);
        z.add(QStringLiteral("META-INF/manifest.xml"), manifest, true);
        const QByteArray made = odfAsTemplate(z.finish());
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(made, parts));
        QCOMPARE(parts.value(QStringLiteral("mimetype")), QByteArray("application/vnd.oasis.opendocument.text-template"));
        QVERIFY(parts.value(QStringLiteral("META-INF/manifest.xml")).contains("manifest:media-type=\"application/vnd.oasis.opendocument.text-template\""));
        QVERIFY(parts.value(QStringLiteral("META-INF/manifest.xml")).contains("manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\""));
        QCOMPARE(parts.value(QStringLiteral("content.xml")), QByteArray("<office:document-content/>"));
        QCOMPARE(made.mid(30, 8), QByteArray("mimetype"));   // (the first entry in the file)
        // A template already, and bytes that are no package, are given back as they were.
        QCOMPARE(odfAsTemplate(made), made);
        QCOMPARE(odfAsTemplate(QByteArray("not a zip")), QByteArray("not a zip"));
    }

    void detectsByContent()
    {
        Made m;
        m.body = "<text:p>x</text:p>";
        QVERIFY(looksLikeOdt(m.build()));
        Made sheet = m;
        sheet.mime = "application/vnd.oasis.opendocument.spreadsheet";
        QVERIFY(!looksLikeOdt(sheet.build()));
        QString error;
        QVERIFY(docxOf(sheet.build(), nullptr, &error).isEmpty());
        QVERIFY(error.contains(QLatin1String("not a text document")));
        QVERIFY(!looksLikeOdt(QByteArray("plain text")));
        // A package with parts of the other kind is no OpenDocument.
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"/>", QStringLiteral("application/xml"));
        QVERIFY(!looksLikeOdt(w.finish()));
        // The mimetype may be deflated or not the first entry: the directory says.
        jp::ZipWriter z;
        z.add(QStringLiteral("content.xml"), "<x/>", true);
        z.add(QStringLiteral("mimetype"), kMime, true);
        QVERIFY(looksLikeOdt(z.finish()));
        QVERIFY(looksLikeFlatOdt(QByteArray("<?xml version=\"1.0\"?><office:document xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" office:mimetype=\"application/vnd.oasis.opendocument.text\">")));
        QVERIFY(!looksLikeFlatOdt(QByteArray("<office:document office:mimetype=\"application/vnd.oasis.opendocument.spreadsheet\">")));
    }

    void paragraphsAndHeadings()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>"
                   "<style:style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:default-outline-level=\"1\">"
                   "<style:paragraph-properties fo:keep-with-next=\"always\"/><style:text-properties fo:font-size=\"16pt\" fo:font-weight=\"bold\"/></style:style>";
        m.body = "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">Title</text:h><text:p text:style-name=\"Standard\">Body &amp; text</text:p><text:h text:outline-level=\"2\">Sub</text:h>";
        const QString d = document(m);
        QVERIFY(d.contains(QLatin1String("<w:pStyle w:val=\"Heading1\"/>")));
        QVERIFY(d.contains(QLatin1String("Body &amp; text")));
        // A heading with no style of its own keeps its level.
        QVERIFY(d.contains(QLatin1String("<w:outlineLvl w:val=\"1\"/>")));
        const QString s = part(docxOf(m.build()), QStringLiteral("/word/styles.xml"));
        QVERIFY(s.contains(QLatin1String("w:styleId=\"Heading1\"")));
        QVERIFY(s.contains(QLatin1String("<w:name w:val=\"heading 1\"/>")));
        QVERIFY(s.contains(QLatin1String("<w:outlineLvl w:val=\"0\"/>")));
        QVERIFY(s.contains(QLatin1String("<w:keepNext/>")));
        QVERIFY(s.contains(QLatin1String("<w:sz w:val=\"32\"/>")));
        QVERIFY(s.contains(QLatin1String("w:styleId=\"Normal\"")));
    }

    void spansAndProperties()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\" fo:font-style=\"italic\" fo:color=\"#ff0000\" style:text-underline-style=\"solid\" "
                       "style:text-underline-type=\"single\" fo:font-size=\"14pt\" style:font-name=\"Arial\" fo:background-color=\"#ffff00\" style:text-position=\"super 58%\"/></style:style>"
                       "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:text-align=\"center\" fo:margin-top=\"0.1in\" fo:margin-left=\"0.5in\" fo:text-indent=\"-0.25in\" "
                       "fo:line-height=\"150%\" fo:background-color=\"#00ff00\" fo:border-bottom=\"0.5pt solid #000000\" fo:break-before=\"page\"/><style:text-properties fo:font-size=\"20pt\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">plain <text:span text:style-name=\"T1\">marked <text:span text:style-name=\"T1\">deep</text:span></text:span></text:p>";
        const QString d = document(m);
        QVERIFY(d.contains(QLatin1String("<w:jc w:val=\"center\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:spacing w:before=\"144\"")));
        QVERIFY(d.contains(QLatin1String("w:line=\"360\" w:lineRule=\"auto\"")));
        QVERIFY(d.contains(QLatin1String("<w:ind w:left=\"720\" w:hanging=\"360\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:pageBreakBefore/>")));
        QVERIFY(d.contains(QLatin1String("<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"00FF00\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:pBdr><w:bottom w:val=\"single\" w:sz=\"4\"")));
        // The run of plain text has the paragraph's size, and the marked one adds its own.
        QVERIFY(d.contains(QLatin1String("<w:sz w:val=\"40\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:b/>")));
        QVERIFY(d.contains(QLatin1String("<w:i/>")));
        QVERIFY(d.contains(QLatin1String("<w:color w:val=\"FF0000\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:u w:val=\"single\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:sz w:val=\"28\"/>")));
        QVERIFY(d.contains(QLatin1String("w:ascii=\"Arial\"")));
        QVERIFY(d.contains(QLatin1String("<w:vertAlign w:val=\"superscript\"/>")));
        QVERIFY(d.contains(QLatin1String("w:fill=\"FFFF00\"")));
        // The result is a document the .docx reader takes.
        wml::Document doc;
        QVERIFY(wml::readDocx(docxOf(m.build()), &doc));
        QVERIFY(!doc.body.isEmpty());
    }

    void whitespaceTabsAndBreaks()
    {
        Made m;
        m.body = "<text:p>  a   b<text:s text:c=\"3\"/>c<text:tab/>d<text:line-break/>e\n   f<text:soft-page-break/>g</text:p>";
        const QString d = document(m);
        // (Leading space and runs of it collapse; text:s is kept.)
        QVERIFY(d.contains(QLatin1String(">a b</w:t>")));
        QVERIFY(d.contains(QLatin1String(">   </w:t>")));
        QVERIFY(d.contains(QLatin1String("<w:tab/>")));
        QVERIFY(d.contains(QLatin1String("<w:br/>")));
        QVERIFY(d.contains(QLatin1String(">e f</w:t>")));
        QVERIFY(d.contains(QLatin1String(">g</w:t>")));
        QVERIFY(!d.contains(QLatin1String("soft-page")));
    }

    void styleInheritanceAndLoops()
    {
        Made m;
        m.styles = "<style:style style:name=\"A\" style:family=\"paragraph\" style:parent-style-name=\"B\"><style:text-properties fo:font-weight=\"bold\"/></style:style>"
                   "<style:style style:name=\"B\" style:family=\"paragraph\" style:parent-style-name=\"A\"><style:text-properties fo:font-style=\"italic\"/></style:style>"
                   "<style:style style:name=\"C\" style:family=\"paragraph\" style:parent-style-name=\"Missing\"/>";
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"A\"><style:paragraph-properties fo:text-align=\"end\"/></style:style>"
                       "<style:style style:name=\"P2\" style:family=\"paragraph\" style:parent-style-name=\"P3\"/><style:style style:name=\"P3\" style:family=\"paragraph\" style:parent-style-name=\"P2\"/>";
        m.body = "<text:p text:style-name=\"P1\">x</text:p><text:p text:style-name=\"P2\">y</text:p><text:p text:style-name=\"C\">z</text:p>";
        QElapsedTimer timer;
        timer.start();
        const QString d = document(m);
        QVERIFY(timer.elapsed() < 5000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
        QVERIFY(d.contains(QLatin1String("<w:pStyle w:val=\"A\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:jc w:val=\"right\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:pStyle w:val=\"C\"/>")));
        const QString s = part(docxOf(m.build()), QStringLiteral("/word/styles.xml"));
        QVERIFY(s.contains(QLatin1String("<w:basedOn w:val=\"B\"/>")));
        QVERIFY(!s.contains(QLatin1String("<w:basedOn w:val=\"Missing\"/>")));
        // A chain of parents is as long as the standard bound.
        Made deep;
        for (int i = 0; i < 200; ++i) deep.styles += "<style:style style:name=\"S" + QByteArray::number(i) + "\" style:family=\"paragraph\" style:parent-style-name=\"S" + QByteArray::number(i + 1) + "\"/>";
        deep.body = "<text:p text:style-name=\"S0\">x</text:p>";
        QVERIFY(!document(deep).isEmpty());
    }

    void listsAndNumbering()
    {
        Made m;
        m.autoStyles = "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\" style:num-suffix=\".\"><style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
                       "<style:list-level-label-alignment text:label-followed-by=\"listtab\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.5in\"/></style:list-level-properties></text:list-level-style-number>"
                       "<text:list-level-style-number text:level=\"2\" style:num-format=\"a\" style:num-prefix=\"(\" style:num-suffix=\")\" text:start-value=\"3\"/></text:list-style>"
                       "<text:list-style style:name=\"L2\"><text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"\xE2\x80\xA2\"/></text:list-style>";
        m.body = "<text:list text:style-name=\"L1\"><text:list-item><text:p>one</text:p><text:list><text:list-item><text:p>inner</text:p></text:list-item></text:list></text:list-item><text:list-item><text:p>two</text:p></text:list-item></text:list>"
                 "<text:p>between</text:p>"
                 "<text:list text:style-name=\"L1\"><text:list-item><text:p>again</text:p></text:list-item></text:list>"
                 "<text:list text:style-name=\"L1\" text:continue-numbering=\"true\"><text:list-item><text:p>goes on</text:p></text:list-item></text:list>"
                 "<text:list text:style-name=\"L2\"><text:list-item><text:p>dot</text:p></text:list-item></text:list>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml")), n = part(docx, QStringLiteral("/word/numbering.xml"));
        QVERIFY(d.contains(QLatin1String("<w:ilvl w:val=\"1\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:lvlText w:val=\"%1.\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:numFmt w:val=\"lowerLetter\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:lvlText w:val=\"(%2)\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:start w:val=\"3\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:ind w:left=\"720\" w:hanging=\"360\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:numFmt w:val=\"bullet\"/>")));
        // The second list of the same style begins again: a number of its own with the start given; the third goes on with the second's number.
        QRegularExpression ids(QStringLiteral("<w:numId w:val=\"(\\d+)\"/>"));
        QStringList used;
        for (auto it = ids.globalMatch(d); it.hasNext();) used << it.next().captured(1);
        QVERIFY(used.size() >= 5);
        QCOMPARE(used[0], used[1]);   // one and inner: one list
        QVERIFY(used[0] != used[3]);   // again: a new count
        QCOMPARE(used[3], used[4]);   // goes on
        QVERIFY(n.contains(QLatin1String("<w:startOverride")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    void tables()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"Table1\" style:family=\"table\"><style:table-properties style:width=\"4in\" table:align=\"center\"/></style:style>"
                       "<style:style style:name=\"Table1.A\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1in\"/></style:style>"
                       "<style:style style:name=\"Table1.B\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"3in\"/></style:style>"
                       "<style:style style:name=\"Table1.A1\" style:family=\"table-cell\"><style:table-cell-properties fo:background-color=\"#cccccc\" fo:border=\"0.5pt solid #000000\" style:vertical-align=\"middle\"/></style:style>"
                       "<style:style style:name=\"Table1.1\" style:family=\"table-row\"><style:table-row-properties style:min-row-height=\"0.5in\"/></style:style>";
        m.body = "<table:table table:style-name=\"Table1\"><table:table-column table:style-name=\"Table1.A\"/><table:table-column table:style-name=\"Table1.B\" table:number-columns-repeated=\"2\"/>"
                 "<table:table-header-rows><table:table-row table:style-name=\"Table1.1\"><table:table-cell table:style-name=\"Table1.A1\" table:number-columns-spanned=\"2\" table:number-rows-spanned=\"2\"><text:p>wide</text:p></table:table-cell>"
                 "<table:covered-table-cell/><table:table-cell><text:p>c</text:p></table:table-cell></table:table-row></table:table-header-rows>"
                 "<table:table-row><table:covered-table-cell table:number-columns-repeated=\"2\"/><table:table-cell><table:table><table:table-column/><table:table-row><table:table-cell><text:p>nested</text:p></table:table-cell></table:table-row></table:table>"
                 "</table:table-cell></table:table-row></table:table><text:p>after</text:p>";
        const QString d = document(m);
        QVERIFY(d.contains(QLatin1String("<w:tblGrid><w:gridCol w:w=\"1440\"/><w:gridCol w:w=\"4320\"/><w:gridCol w:w=\"4320\"/></w:tblGrid>")));
        QVERIFY(d.contains(QLatin1String("<w:jc w:val=\"center\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:gridSpan w:val=\"2\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:vMerge w:val=\"restart\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:vMerge/>")));
        QVERIFY(d.contains(QLatin1String("<w:tblHeader/>")));
        QVERIFY(d.contains(QLatin1String("<w:trHeight w:val=\"720\" w:hRule=\"atLeast\"/>")));
        QVERIFY(d.contains(QLatin1String("w:fill=\"CCCCCC\"")));
        QVERIFY(d.contains(QLatin1String("<w:tcBorders><w:top w:val=\"single\"")));
        QVERIFY(d.contains(QLatin1String("<w:vAlign w:val=\"center\"/>")));
        // The nested table is in a cell, and the cell ends in a paragraph.
        QCOMPARE(d.count(QLatin1String("<w:tbl>")), 2);
        QVERIFY(d.contains(QLatin1String("nested</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/></w:tc>")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docxOf(m.build()), &doc));
    }

    void pageLayoutAndSections()
    {
        Made m;
        m.layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" style:print-orientation=\"portrait\" fo:margin-top=\"0.5in\" fo:margin-bottom=\"0.5in\" "
                    "fo:margin-left=\"1in\" fo:margin-right=\"1in\"/><style:header-style><style:header-footer-properties fo:min-height=\"0.4in\" fo:margin-bottom=\"0.1in\"/></style:header-style></style:page-layout>"
                    "<style:page-layout style:name=\"pm2\"><style:page-layout-properties fo:page-width=\"11in\" fo:page-height=\"8.5in\" style:print-orientation=\"landscape\" fo:margin=\"0.75in\"/></style:page-layout>";
        m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"><style:header><text:p>H</text:p></style:header></style:master-page>"
                    "<style:master-page style:name=\"Wide\" style:page-layout-name=\"pm2\"/>";
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\" style:master-page-name=\"Wide\"/>"
                       "<style:style style:name=\"Sect1\" style:family=\"section\"><style:section-properties><style:columns fo:column-count=\"2\" fo:column-gap=\"0.25in\"/></style:section-properties></style:style>";
        m.body = "<text:p>first</text:p><text:p text:style-name=\"P1\">landscape</text:p><text:section text:style-name=\"Sect1\"><text:p>in columns</text:p></text:section><text:p>last</text:p>";
        const QString d = document(m);
        // The first section ends in the paragraph before the one that changes the page; the one after it ends there too, at the end of its columns.
        QVERIFY(d.contains(QLatin1String("<w:pgSz w:w=\"12240\" w:h=\"15840\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/>")));
        QVERIFY(d.contains(QLatin1String("w:left=\"1080\"")));
        // (The top margin is the page's plus the header and the space after it.)
        QVERIFY(d.contains(QLatin1String("w:top=\"1440\"")));
        QVERIFY(d.contains(QLatin1String("w:header=\"720\"")));
        QVERIFY(d.contains(QLatin1String("<w:cols w:num=\"2\" w:space=\"360\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:type w:val=\"continuous\"/>")));
        QVERIFY(d.count(QLatin1String("<w:sectPr>")) >= 4);
        wml::Document doc;
        QVERIFY(wml::readDocx(docxOf(m.build()), &doc));
    }

    void headersAndFooters()
    {
        Made m;
        m.layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" fo:margin-top=\"1in\" fo:margin-bottom=\"1in\" fo:margin-left=\"1in\" fo:margin-right=\"1in\"/></style:page-layout>";
        m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"><style:header><text:p>right header <text:page-number>1</text:page-number></text:p></style:header>"
                    "<style:header-left><text:p>left header</text:p></style:header-left><style:header-first><text:p>first header</text:p></style:header-first>"
                    "<style:footer><text:p>footer <text:page-count>3</text:page-count></text:p></style:footer></style:master-page>";
        m.body = "<text:p>body</text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<w:headerReference w:type=\"default\"")));
        QVERIFY(d.contains(QLatin1String("<w:headerReference w:type=\"even\"")));
        QVERIFY(d.contains(QLatin1String("<w:headerReference w:type=\"first\"")));
        QVERIFY(d.contains(QLatin1String("<w:footerReference w:type=\"default\"")));
        QVERIFY(d.contains(QLatin1String("<w:titlePg/>")));
        QVERIFY(part(docx, QStringLiteral("/word/settings.xml")).contains(QLatin1String("<w:evenAndOddHeaders/>")));
        Package p;
        QVERIFY(p.read(docx));
        QString all;
        for (const QString &n : p.partNames())
            if (n.contains(QLatin1String("header")) || n.contains(QLatin1String("footer"))) all += QString::fromUtf8(p.part(n));
        QVERIFY(all.contains(QLatin1String("right header")));
        QVERIFY(all.contains(QLatin1String("left header")));
        QVERIFY(all.contains(QLatin1String("first header")));
        QVERIFY(all.contains(QLatin1String("PAGE")));
        QVERIFY(all.contains(QLatin1String("NUMPAGES")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    void notesAndComments()
    {
        Made m;
        m.body = "<text:p>a<text:note text:id=\"ftn1\" text:note-class=\"footnote\"><text:note-citation>1</text:note-citation><text:note-body><text:p>foot text</text:p></text:note-body></text:note>"
                 "b<text:note text:id=\"edn1\" text:note-class=\"endnote\"><text:note-citation>i</text:note-citation><text:note-body><text:p>end text</text:p></text:note-body></text:note>"
                 "<office:annotation office:name=\"c1\"><dc:creator>Ann</dc:creator><dc:date>2024-01-02T03:04:05</dc:date><text:p>remark</text:p></office:annotation>spanned<office:annotation-end office:name=\"c1\"/>c</text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<w:footnoteReference w:id=\"1\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:endnoteReference w:id=\"1\"/>")));
        QVERIFY(part(docx, QStringLiteral("/word/footnotes.xml")).contains(QLatin1String("foot text")));
        QVERIFY(part(docx, QStringLiteral("/word/endnotes.xml")).contains(QLatin1String("end text")));
        const QString c = part(docx, QStringLiteral("/word/comments.xml"));
        QVERIFY(c.contains(QLatin1String("w:author=\"Ann\"")));
        QVERIFY(c.contains(QLatin1String("remark")));
        QVERIFY(d.contains(QLatin1String("<w:commentRangeStart")));
        QVERIFY(d.contains(QLatin1String("<w:commentRangeEnd")));
        QVERIFY(d.indexOf(QLatin1String("<w:commentRangeStart")) < d.indexOf(QLatin1String("spanned")));
        QVERIFY(d.indexOf(QLatin1String("spanned")) < d.indexOf(QLatin1String("<w:commentRangeEnd")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    void fieldsBookmarksAndContents()
    {
        Made m;
        m.body = "<text:table-of-content><text:table-of-content-source text:outline-level=\"2\"/><text:index-body><text:index-title><text:p>Contents</text:p></text:index-title>"
                 "<text:p>One<text:tab/>1</text:p><text:p>Two<text:tab/>2</text:p></text:index-body></text:table-of-content>"
                 "<text:p><text:bookmark-start text:name=\"here\"/>marked<text:bookmark-end text:name=\"here\"/><text:bookmark text:name=\"point\"/> "
                 "<text:bookmark-ref text:ref-name=\"here\" text:reference-format=\"text\">marked</text:bookmark-ref> <text:bookmark-ref text:ref-name=\"here\" text:reference-format=\"page\">1</text:bookmark-ref> "
                 "<text:title>T</text:title> <text:sequence text:name=\"Figure\" style:num-format=\"1\">1</text:sequence> <text:a xlink:href=\"http://example.com/x\">link</text:a> <text:a xlink:href=\"#here\">inner</text:a></text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("TOC \\o \"1-2\" \\h \\z \\u")));
        QVERIFY(d.indexOf(QLatin1String("fldCharType=\"begin\"")) < d.indexOf(QLatin1String(">One<")));
        QVERIFY(d.lastIndexOf(QLatin1String("fldCharType=\"end\"")) > d.indexOf(QLatin1String(">Two<")));
        QVERIFY(d.contains(QLatin1String("<w:bookmarkStart w:id=\"1\" w:name=\"here\"/>")));
        QVERIFY(d.contains(QLatin1String("w:name=\"point\"")));
        QVERIFY(d.contains(QLatin1String("REF \"here\" \\h")));
        QVERIFY(d.contains(QLatin1String("PAGEREF \"here\" \\h")));
        QVERIFY(d.contains(QLatin1String("TITLE")));
        QVERIFY(d.contains(QLatin1String("SEQ \"Figure\"")));
        QVERIFY(d.contains(QLatin1String("<w:hyperlink r:id=")));
        QVERIFY(d.contains(QLatin1String("<w:hyperlink w:anchor=\"here\"")));
        Package p;
        QVERIFY(p.read(docx));
        bool external = false;
        for (const Relationship &r : p.relationships(QStringLiteral("/word/document.xml")))
            if (r.external && r.target == QLatin1String("http://example.com/x")) external = true;
        QVERIFY(external);
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    // The chart document of an object of a made .odt: `points` numbers in one series.
    static QByteArray chartDocument(int points)
    {
        jp::office::Chart model = jp::office::blankChart(jp::office::defaultTheme());
        jp::office::ChartPlot plot;
        plot.type = jp::office::ChartType::Bar;
        jp::office::ChartSeries series;
        series.name = QStringLiteral("S");
        for (int i = 0; i < points; ++i) series.values << double(i % 5);
        plot.series.append(series);
        model.plots.append(plot);
        QStringList notes;
        return jp::office::sheet::odschart::chartToOdf(model, 4000000, 3000000, &notes);
    }
    static QByteArray chartFrame(const QString &href, const char *size = "svg:width=\"3in\" svg:height=\"2in\"")
    {
        return "<draw:frame text:anchor-type=\"as-char\" " + QByteArray(size) + "><draw:object xlink:href=\"" + href.toUtf8() + "\"/></draw:frame>";
    }
    static int chartParts(const QByteArray &docx)
    {
        Package p;
        if (!p.read(docx)) return -1;
        int n = 0;
        for (const QString &name : p.partNames())
            if (name.startsWith(QLatin1String("/word/charts/")) && name.endsWith(QLatin1String(".xml"))) ++n;
        return n;
    }

    // A chart is an object of its own: read into one chart part, however many frames name it and however its name is written, and a name that goes out of the package is not followed.
    void aChartObjectIsOnePartForEveryFrameThatNamesIt()
    {
        Made m;
        m.extra.insert(QStringLiteral("Object 1/content.xml"), chartDocument(5));
        QByteArray body = "<text:p>";
        for (const char *href : {"./Object 1", "./Object 1/", "Object 1", "object 1", "OBJECT 1/"}) body += chartFrame(QString::fromLatin1(href));
        for (int i = 0; i < 40; ++i) body += chartFrame(QStringLiteral("./Object 1"));
        m.body = body + "</text:p>";
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        QCOMPARE(chartParts(docx), 1);
        QCOMPARE(part(docx, QStringLiteral("/word/document.xml")).count(QLatin1String("<c:chart ")), 45);   // (every spelling is the one object)
        QCOMPARE(int(notes.count(QStringLiteral("charts the file does not hold"))), 0);
    }

    void aChartObjectThatLeavesThePackageIsNotFollowedAndIsToldOnce()
    {
        Made m;
        m.extra.insert(QStringLiteral("Object 1/content.xml"), chartDocument(5));
        QByteArray body = "<text:p>";
        for (int i = 0; i < 30; ++i)
            for (const char *href : {"../Object 1", "/Object 1", "Object 1/../Object 1", "file:///tmp/x", "#Object 1", "..\\Object 1"}) body += chartFrame(QString::fromLatin1(href));
        m.body = body + "</text:p>";
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        QCOMPARE(chartParts(docx), 0);
        QCOMPARE(int(notes.count(QStringLiteral("charts the file does not hold"))), 1);
    }

    void aDocumentIsHeldToTwoHundredCharts()
    {
        Made m;
        QByteArray body = "<text:p>";
        const QByteArray one = chartDocument(3);
        for (int i = 1; i <= 205; ++i) {
            m.extra.insert(QStringLiteral("Object %1/content.xml").arg(i), one);
            body += chartFrame(QStringLiteral("./Object %1").arg(i));
        }
        m.body = body + "</text:p>";
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        QCOMPARE(chartParts(docx), 200);
        QCOMPARE(int(notes.count(QStringLiteral("charts past the 200 a document may have"))), 1);
    }

    void allTheChartsOfADocumentShareOneBudgetOfPoints()
    {
        Made m;
        QByteArray body = "<text:p>";
        const QByteArray big = chartDocument(jp::office::sheet::odschart::kMaxPoints);
        for (int i = 1; i <= 12; ++i) {
            m.extra.insert(QStringLiteral("Object %1/content.xml").arg(i), big);
            body += chartFrame(QStringLiteral("./Object %1").arg(i));
        }
        m.body = body + "</text:p>";
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        // (a chart of 100,000 points: four of them are the most that fit in 400,000, so the fifth is told; the budget of bytes would let five through)
        const int made = chartParts(docx);
        QVERIFY2(made >= 2 && made <= 4, qPrintable(QString::number(made)));
        QVERIFY2(notes.join(QLatin1Char('|')).contains(QLatin1String("all the charts of a document")), qPrintable(notes.join(QLatin1Char('|'))));
    }

    // The bytes of all the chart documents of one file together are held to 96 MB (each is under the 32 MB a chart may be, and the points are few): the charts past the budget are not read, and one note says so.
    void allTheChartsOfADocumentShareOneBudgetOfBytes()
    {
        Made m;
        QByteArray padded = chartDocument(3);
        const int end = padded.lastIndexOf("</");
        QVERIFY(end > 0);
        padded.insert(end, "<!--" + QByteArray(30 << 20, ' ') + "-->");   // (a comment: 30 MB the chart does not use)
        QVERIFY(padded.size() < jp::office::sheet::odschart::kMaxPartBytes);
        QByteArray body = "<text:p>";
        for (int i = 1; i <= 5; ++i) {
            m.extra.insert(QStringLiteral("Object %1/content.xml").arg(i), padded);
            body += chartFrame(QStringLiteral("./Object %1").arg(i));
        }
        m.body = body + "</text:p>";
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        // (three of about 30 MB come to 90 MB; the fourth would pass 96 MB, and so would the fifth)
        QCOMPARE(chartParts(docx), 3);
        QCOMPARE(int(notes.count(QStringLiteral("charts past the size all the charts of a document may come to"))), 1);
    }

    // A chart named by two frames of two sizes is one object in the .odt, and each frame keeps its own size.
    void aChartInTwoSizesIsOneObjectAndEachFrameKeepsItsSize()
    {
        Made m;
        m.extra.insert(QStringLiteral("Object 1/content.xml"), chartDocument(4));
        m.body = "<text:p>" + chartFrame(QStringLiteral("./Object 1"), "svg:width=\"3in\" svg:height=\"2in\"") + chartFrame(QStringLiteral("./Object 1"), "svg:width=\"6in\" svg:height=\"1in\"") + "</text:p>";
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        QCOMPARE(chartParts(docx), 1);
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<wp:extent cx=\"2743200\" cy=\"1828800\"/>")));
        QVERIFY(d.contains(QLatin1String("<wp:extent cx=\"5486400\" cy=\"914400\"/>")));
        const QByteArray back = fromDocxPackage(docx, nullptr, &notes);
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(back, files));
        QVERIFY(files.contains(QStringLiteral("Object 1/content.xml")) && !files.contains(QStringLiteral("Object 2/content.xml")));
        const QByteArray content = files.value(QStringLiteral("content.xml"));
        QCOMPARE(int(content.count("<draw:object ")), 2);
        QVERIFY(content.contains("svg:width=\"3.0000in\"") && content.contains("svg:width=\"6.0000in\""));
    }

    void picturesAndTextBoxes()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"fr1\" style:family=\"graphic\"><style:graphic-properties style:wrap=\"parallel\" style:horizontal-pos=\"right\" style:horizontal-rel=\"page\" style:vertical-pos=\"top\" style:vertical-rel=\"paragraph\"/></style:style>";
        m.body = "<text:p>pic <draw:frame draw:name=\"P\" text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"0.5in\"><draw:image xlink:href=\"Pictures/a.png\"/><svg:desc>alt words</svg:desc></draw:frame>"
                 "<draw:frame draw:style-name=\"fr1\" text:anchor-type=\"paragraph\" svg:x=\"1in\" svg:y=\"0.5in\" svg:width=\"2in\" svg:height=\"1in\"><draw:text-box><text:p>in the box</text:p></draw:text-box></draw:frame>"
                 "<draw:frame text:anchor-type=\"paragraph\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/missing.png\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<wp:inline")));
        QVERIFY(d.contains(QLatin1String("<wp:extent cx=\"914400\" cy=\"457200\"/>")));
        QVERIFY(d.contains(QLatin1String("descr=\"alt words\"")));
        QVERIFY(d.contains(QLatin1String("<wp:anchor")));
        QVERIFY(d.contains(QLatin1String("<wp:wrapSquare wrapText=\"bothSides\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:txbxContent>")));
        QVERIFY(d.contains(QLatin1String("in the box")));
        Package p;
        QVERIFY(p.read(docx));
        bool media = false;
        for (const QString &n : p.partNames())
            if (n.startsWith(QLatin1String("/word/media/")) && p.part(n) == kPng) media = true;
        QVERIFY(media);
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    // The side the text goes on keeps its meaning through a save and an open: ODF's "biggest" is the larger side of DrawingML's wrapText="largest", and it is written back as "biggest".
    void theLargestSideOfAWrapSurvivesTheRoundTrip()
    {
        for (const char *side : {"biggest", "left", "right", "parallel"}) {
            Made m;
            m.autoStyles = "<style:style style:name=\"fr1\" style:family=\"graphic\"><style:graphic-properties style:wrap=\"" + QByteArray(side) + "\" style:horizontal-pos=\"left\" style:horizontal-rel=\"paragraph\"/></style:style>";
            m.body = "<text:p>x <draw:frame draw:style-name=\"fr1\" text:anchor-type=\"paragraph\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame></text:p>";
            m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
            const QByteArray odt2 = fromDocxPackage(docxOf(m.build()));
            QMap<QString, QByteArray> files;
            QVERIFY(jp::readZip(odt2, files));
            const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
            QVERIFY2(c.contains(QStringLiteral("style:wrap=\"%1\"").arg(QLatin1String(side))), side);
            const QString again = part(docxOf(odt2), QStringLiteral("/word/document.xml"));
            const QString want = QLatin1String(side) == QLatin1String("biggest") ? QStringLiteral("wrapText=\"largest\"") : QLatin1String(side) == QLatin1String("parallel") ? QStringLiteral("wrapText=\"bothSides\"") : QStringLiteral("wrapText=\"%1\"").arg(QLatin1String(side));
            QVERIFY2(again.contains(want), side);
        }
    }

    // A wrap contour from a file is bounded: a view box with no size makes no outline, and 600 points make 128, not 600.
    void aHostileContourPolygonIsBounded()
    {
        QByteArray points;
        for (int i = 0; i < 600; ++i) points += QByteArray::number(i % 100) + "," + QByteArray::number((i * 7) % 100) + " ";
        auto docxWith = [&](const QByteArray &viewBox, const QByteArray &pts) {
            Made m;
            m.autoStyles = "<style:style style:name=\"fr1\" style:family=\"graphic\"><style:graphic-properties style:wrap=\"parallel\" style:wrap-contour=\"true\" style:horizontal-pos=\"left\" style:horizontal-rel=\"paragraph\"/></style:style>";
            m.body = "<text:p>x <draw:frame draw:style-name=\"fr1\" text:anchor-type=\"paragraph\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/a.png\"/>"
                     "<draw:contour-polygon svg:width=\"1in\" svg:height=\"1in\" svg:viewBox=\"" + viewBox + "\" draw:points=\"" + pts + "\"/></draw:frame></text:p>";
            m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
            return part(docxOf(m.build()), QStringLiteral("/word/document.xml"));
        };
        const QString none = docxWith("0 0 0 0", points);
        QVERIFY(none.contains(QLatin1String("<wp:wrapSquare")) && !none.contains(QLatin1String("wrapPolygon")));
        const QString held = docxWith("0 0 100 100", points);
        QVERIFY(held.contains(QLatin1String("<wp:wrapTight")));
        QCOMPARE(held.count(QLatin1String("<wp:lineTo")), jp::kMostWrapPoints);   // (the start, the points kept after it, and the one that closes the polygon)
        QVERIFY(!docxWith("0 0 100 100", "nan,nan inf,1 1,2").contains(QLatin1String("wrapPolygon")));
    }

    // An ODF path from another program becomes a freeform shape: relative commands (m, l, c), S, Q, and T, and the pairs of numbers that repeat a command, are read; a number that is not a finite
    // number drops that shape and nothing else; a box with one long side keeps its decimals.
    void aPathIsReadWithRelativeCommandsAndRepeats()
    {
        auto docxOfPath = [&](const QByteArray &d, const QByteArray &viewBox = "0 0 100 100") {
            Made m;
            m.body = "<text:p>before <draw:path text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\" svg:viewBox=\"" + viewBox + "\" svg:d=\"" + d + "\"/> after</text:p>";
            return part(docxOf(m.build()), QStringLiteral("/word/document.xml"));
        };
        // Relative lines, with the pairs after the first repeating the command (after a move, they are lines).
        QString x = docxOfPath("m 10 10 l 20 0 0 20 z");
        QVERIFY(x.contains(QLatin1String("<a:custGeom>")));
        QVERIFY(x.contains(QLatin1String("<a:moveTo><a:pt x=\"10000\" y=\"10000\"/></a:moveTo>")));
        QVERIFY(x.contains(QLatin1String("<a:lnTo><a:pt x=\"30000\" y=\"10000\"/></a:lnTo><a:lnTo><a:pt x=\"30000\" y=\"30000\"/></a:lnTo><a:close/>")));
        x = docxOfPath("M 10 10 20 10 20 20 Z");
        QVERIFY(x.contains(QLatin1String("<a:lnTo><a:pt x=\"20000\" y=\"10000\"/></a:lnTo><a:lnTo><a:pt x=\"20000\" y=\"20000\"/></a:lnTo>")));
        // A relative curve, a smooth one (its first control point turned about the pen), and quadratic ones.
        x = docxOfPath("M0 0 c 10 0 20 10 20 20 s 10 10 20 0");
        QVERIFY(x.contains(QLatin1String("<a:cubicBezTo><a:pt x=\"10000\" y=\"0\"/><a:pt x=\"20000\" y=\"10000\"/><a:pt x=\"20000\" y=\"20000\"/></a:cubicBezTo>")));
        QVERIFY(x.contains(QLatin1String("<a:cubicBezTo><a:pt x=\"20000\" y=\"30000\"/><a:pt x=\"30000\" y=\"30000\"/><a:pt x=\"40000\" y=\"20000\"/></a:cubicBezTo>")));
        x = docxOfPath("M0 0 Q 30 30 60 0 T 90 0");
        QCOMPARE(x.count(QLatin1String("<a:cubicBezTo>")), 2);
        QVERIFY(x.contains(QLatin1String("<a:pt x=\"60000\" y=\"0\"/></a:cubicBezTo>")));
        QVERIFY(x.contains(QLatin1String("<a:pt x=\"90000\" y=\"0\"/></a:cubicBezTo>")));
        // Numbers that run together ("1.5-2") and exponents.
        x = docxOfPath("M1.5-2L1e1 5");
        QVERIFY(x.contains(QLatin1String("<a:pt x=\"1500\" y=\"-2000\"/>")));
        QVERIFY(x.contains(QLatin1String("<a:pt x=\"10000\" y=\"5000\"/>")));
        // An arc is not read; a number that is not finite drops the shape, and the words round it stay.
        for (const char *bad : {"M 0 0 A 5 5 0 0 1 10 10", "M 0 0 L nan 10 Z", "M 0 0 L inf 10 Z", "M 0 0 L 1e999 10 Z", "M 0 0 L -nan 1 Z", "M 0 0 L 5"}) {
            x = docxOfPath(bad);
            QVERIFY2(!x.contains(QLatin1String("<a:custGeom>")), bad);
            QVERIFY2(x.contains(QLatin1String("before")) && x.contains(QLatin1String("after")), bad);
        }
        x = docxOfPath("M 0 0 L 1 1", "0 0 nan 100");
        QVERIFY(!x.contains(QLatin1String("<a:custGeom>")));
        // A box with one long side keeps its decimals (the other side's size does not set the scale).
        x = docxOfPath("M0 0 L1.5 0.5", "0 0 200000 100");
        QVERIFY(x.contains(QLatin1String("<a:pt x=\"1500\" y=\"500\"/>")));
        QVERIFY(x.contains(QLatin1String("<a:path w=\"200000000\" h=\"100000\">")));
    }

    void trackedChangesAreKept()
    {
        Made m;
        m.body = "<text:tracked-changes><text:changed-region text:id=\"c1\"><text:deletion><office:change-info><dc:creator>Ann</dc:creator><dc:date>2024-05-06T07:08:09</dc:date></office:change-info><text:p>gone text</text:p></text:deletion></text:changed-region>"
                 "<text:changed-region text:id=\"c2\"><text:insertion><office:change-info><dc:creator>Bob</dc:creator></office:change-info></text:insertion></text:changed-region></text:tracked-changes>"
                 "<text:p>keep <text:change-start text:change-id=\"c2\"/>added<text:change-end text:change-id=\"c2\"/><text:change text:change-id=\"c1\"/> end</text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<w:ins w:id=\"1\" w:author=\"Bob\">")));
        QVERIFY(d.contains(QRegularExpression(QStringLiteral("<w:ins [^>]*><w:r><w:t xml:space=\"preserve\">added</w:t></w:r></w:ins>"))));
        QVERIFY(d.contains(QLatin1String("<w:del w:id=\"2\" w:author=\"Ann\" w:date=\"2024-05-06T07:08:09Z\">")));
        QVERIFY(d.contains(QLatin1String("<w:delText xml:space=\"preserve\">gone text</w:delText>")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
        // Written back, they are tracked changes of the .odt again.
        const QByteArray odt = fromDocxPackage(docx);
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        QVERIFY(c.contains(QLatin1String("<text:tracked-changes>")));
        QVERIFY(c.contains(QLatin1String("<text:insertion>")));
        QVERIFY(c.contains(QLatin1String("<text:deletion>")));
        QVERIFY(c.contains(QLatin1String("gone text")));
        QVERIFY(c.contains(QLatin1String("<text:change-start")));
        QVERIFY(c.contains(QLatin1String("<text:change text:change-id")));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
    }

    // A run that states a property twice (<w:i/> twice, two sizes) is written with each attribute once: a document with an attribute twice in an element is not well-formed XML, and another program refuses it.
    void aRunPropertyStatedTwiceIsWrittenOnce()
    {
        const QString c = odtContentOf("<w:p><w:r><w:rPr><w:i/><w:b/><w:i/><w:sz w:val=\"24\"/><w:sz w:val=\"28\"/></w:rPr><w:t>x</w:t></w:r></w:p>");
        QVERIFY(c.contains(QLatin1String("<text:p")));
        QVERIFY(wellFormed(c.toUtf8()));
        QCOMPARE(c.count(QLatin1String("fo:font-style=")), 1);
        QCOMPARE(c.count(QLatin1String("fo:font-size=")), 1);
        QVERIFY(c.contains(QLatin1String("fo:font-size=\"14pt\"")));   // (the last one stated)
    }

    // A page break that is a run of a paragraph comes back as one: at the start, in the middle, at the end, and in a paragraph of its own. A paragraph that asks to begin a page is not turned into one with a break.
    void pageBreakRunsComeBackAsRuns()
    {
        const QByteArray br = "<w:r><w:br w:type=\"page\"/></w:r>";
        const auto run = [](const char *text) { return QByteArray("<w:r><w:t>") + text + "</w:t></w:r>"; };
        const QByteArray body = "<w:p>" + br + run("lead") + "</w:p>"
                                "<w:p>" + run("one") + br + run("two") + "</w:p>"
                                "<w:p>" + run("end") + br + "</w:p>"
                                "<w:p>" + run("next") + "</w:p>"
                                "<w:p>" + br + "</w:p>"
                                "<w:p>" + run("after") + "</w:p>"
                                "<w:p><w:pPr><w:pageBreakBefore/></w:pPr>" + run("asks") + "</w:p>"
                                "<w:p>" + run("last") + "</w:p>";
        const QStringList paragraphs = paragraphsOf(roundTripBody(body));
        QCOMPARE(paragraphs, (QStringList{QStringLiteral("\flead"), QStringLiteral("one\ftwo"), QStringLiteral("end\f"), QStringLiteral("next"), QStringLiteral("\f"), QStringLiteral("after"), QStringLiteral("^asks"), QStringLiteral("last")}));
    }

    // A symbol (a code of a symbol font) comes back as a symbol, not as the letter it was written as, so a copy of a document has the same text as the document.
    void aSymbolComesBackAsASymbol()
    {
        const QString back = roundTripBody("<w:p><w:r><w:t>a</w:t></w:r><w:r><w:sym w:font=\"Symbol\" w:char=\"F0B0\"/></w:r><w:r><w:t>b</w:t></w:r><w:r><w:sym w:font=\"Unlisted Font\" w:char=\"F041\"/></w:r><w:r><w:sym w:char=\"00E9\"/></w:r></w:p>");
        wml::XmlTree t;
        QVERIFY(t.parse(back.toUtf8(), nullptr));
        QStringList seen;
        for (int i = findElement(t, "sym"); i >= 0; i = findElement(t, "sym", i)) seen << t.attr(i, "font") + QLatin1Char('/') + t.attr(i, "char");
        QCOMPARE(seen, (QStringList{QStringLiteral("Symbol/F0B0"), QStringLiteral("Unlisted Font/F041"), QStringLiteral("/00E9")}));
        QCOMPARE(paragraphsOf(back), (QStringList{QStringLiteral("ab")}));
    }

    // Automatic spacing before or after a paragraph (a paragraph from a web page) is the 14 points it is laid out with, not the 5 points the paragraph also states; one that says it is not automatic keeps its own.
    void automaticSpacingIsWrittenAsFourteenPoints()
    {
        const QString c = odtStylesAndContentOf("<w:p><w:pPr><w:spacing w:before=\"100\" w:beforeAutospacing=\"1\" w:after=\"240\" w:afterAutospacing=\"0\"/></w:pPr><w:r><w:t>x</w:t></w:r></w:p>"
                                                "<w:p><w:pPr><w:spacing w:before=\"100\" w:after=\"100\" w:afterAutospacing=\"true\"/></w:pPr><w:r><w:t>y</w:t></w:r></w:p>");
        QVERIFY2(c.contains(QLatin1String("fo:margin-top=\"0.1944in\" fo:margin-bottom=\"0.1667in\"")), qPrintable(c));
        QVERIFY2(c.contains(QLatin1String("fo:margin-top=\"0.0694in\" fo:margin-bottom=\"0.1944in\"")), qPrintable(c));
    }

    // A page break that ended a paragraph is still one page break when the .odt is read by a program that knows nothing of ours: the paragraph after it begins a page, once.
    void aPageBreakAtTheEndOfAParagraphBeginsThePageOfTheNext()
    {
        const QString c = odtStylesAndContentOf("<w:p><w:r><w:t>end</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r></w:p><w:p><w:r><w:t>next</w:t></w:r></w:p>");
        QCOMPARE(c.count(QLatin1String("fo:break-before=\"page\"")), 1);
    }

    // A page break that ended a paragraph, or was a paragraph of its own, comes back as it was when the paragraph after it is a heading or a list item too: the break is not read twice, and not lost.
    void aPageBreakBeforeAHeadingOrAListItemComesBackAsItWas()
    {
        const QByteArray br = "<w:r><w:br w:type=\"page\"/></w:r>";
        const auto run = [](const char *text) { return QByteArray("<w:r><w:t>") + text + "</w:t></w:r>"; };
        const QByteArray heading = "<w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr>";
        const QByteArray body = "<w:p>" + run("end") + br + "</w:p><w:p>" + heading + run("head") + "</w:p>"
                                "<w:p>" + br + "</w:p><w:p>" + heading + run("again") + "</w:p><w:p>" + heading + br + run("lead") + "</w:p>"
                                "<w:p>" + br + "</w:p><w:p>" + br + "</w:p><w:p>" + run("two breaks") + "</w:p>";
        QCOMPARE(paragraphsOf(roundTripBody(body)), (QStringList{QStringLiteral("end\f"), QStringLiteral("head"), QStringLiteral("\f"), QStringLiteral("again"), QStringLiteral("\flead"), QStringLiteral("\f"), QStringLiteral("\f"),
                                                                  QStringLiteral("two breaks")}));
    }

    // With every attribute of ours taken out, the file shows the paragraphs and page breaks it always did: a paragraph that is only a page break is not a paragraph of its own (nothing is an empty line at the top
    // of the new page), the break is the one the next paragraph begins with, and a break at the end of a paragraph is the next one's once.
    void aPageBreakIsTheStandardsAlsoWithOurAttributesIgnored()
    {
        const QByteArray br = "<w:r><w:br w:type=\"page\"/></w:r>";
        const auto run = [](const char *text) { return QByteArray("<w:r><w:t>") + text + "</w:t></w:r>"; };
        const QByteArray body = "<w:p>" + br + run("lead") + "</w:p>"
                                "<w:p>" + run("one") + br + run("two") + "</w:p>"
                                "<w:p>" + run("end") + br + "</w:p>"
                                "<w:p>" + run("next") + "</w:p>"
                                "<w:p>" + br + "</w:p>"
                                "<w:p>" + run("after") + "</w:p>"
                                "<w:p><w:pPr><w:pageBreakBefore/></w:pPr>" + run("asks") + "</w:p>"
                                "<w:p>" + run("last") + "</w:p>";
        QCOMPARE(standardParagraphsOf(QString::fromUtf8(body)), (QStringList{QStringLiteral("lead"), QStringLiteral("one"), QStringLiteral("^two"), QStringLiteral("end"), QStringLiteral("^next"), QStringLiteral("^after"),
                                                                              QStringLiteral("^asks"), QStringLiteral("last")}));
    }

    // A page break at the end of a paragraph of a table's cell is not the break of the paragraph before the next one in the body: the next one is not marked as carrying it (it is not what it reads back as).
    void aBreakInATableCellIsNotCarriedToTheBody()
    {
        const QByteArray br = "<w:r><w:br w:type=\"page\"/></w:r>";
        const QByteArray body = "<w:p><w:r><w:t>before</w:t></w:r></w:p><w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r>" + br
                                + "</w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
        const QString c = odtStylesAndContentOf(body);
        QVERIFY2(!c.contains(QLatin1String("break-carried")), qPrintable(c));
        const QStringList paragraphs = standardParagraphsOf(QString::fromUtf8(body));
        QCOMPARE(paragraphs.first(), QStringLiteral("before"));
        QString lastText = paragraphs.last();
        QCOMPARE(lastText.remove(QLatin1Char('^')), QStringLiteral("after"));
        // and a break that ended a paragraph of the body is told on the one that has it
        const QString body2 = odtStylesAndContentOf("<w:p><w:r><w:t>end</w:t></w:r>" + br + "</w:p><w:p><w:r><w:t>next</w:t></w:r></w:p>");
        QCOMPARE(body2.count(QLatin1String("jo:break-carried=\"true\"")), 1);
    }

    // The symbols in the standard's terms: with ours ignored, the span has the character and the symbol font's name (a symbol of no font is the character itself).
    void aSymbolIsTheStandardsAlsoWithOurAttributesIgnored()
    {
        QString c = odtStylesAndContentOf("<w:p><w:r><w:sym w:font=\"Unlisted Font\" w:char=\"F041\"/></w:r><w:r><w:sym w:char=\"00E9\"/></w:r></w:p>");
        QVERIFY2(c.contains(QLatin1String("jo:sym=\"F041\"")), qPrintable(c));
        c.remove(QRegularExpression(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\"")));
        QVERIFY2(!c.contains(QLatin1String("jo:")), qPrintable(c));
        QVERIFY2(c.contains(QLatin1String("style:font-name=\"Unlisted Font\"")), qPrintable(c));
        const int span = c.indexOf(QLatin1String("<text:span"));
        QVERIFY(span >= 0);
        QVERIFY2(c.mid(span).contains(QString(QChar(0xF041))), qPrintable(c));
        QVERIFY(c.contains(QString(QChar(0x00E9))));
    }

    // Attributes of ours that do not name our own namespace mean nothing: a page break named by another namespace's attribute of the same name is not a break.
    void aPageBreakAttributeOfAnotherNamespaceIsIgnored()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"Pb\" style:family=\"paragraph\"><style:paragraph-properties xmlns:other=\"urn:someone:else\" other:break-start=\"run\" other:break-end=\"run\" other:break-carried=\"alone\"/></style:style>";
        m.body = "<text:p text:style-name=\"Pb\">one</text:p><text:p>two</text:p>";
        QCOMPARE(paragraphsOf(document(m)), (QStringList{QStringLiteral("one"), QStringLiteral("two")}));
        // (the same attributes in ours are read)
        m.autoStyles = "<style:style style:name=\"Pb\" style:family=\"paragraph\"><style:paragraph-properties xmlns:jo=\"urn:jeffoffice:odf-extension\" jo:break-start=\"run\" jo:break-end=\"run\"/></style:style>";
        QCOMPARE(paragraphsOf(document(m)), (QStringList{QStringLiteral("\fone\f"), QStringLiteral("two")}));
    }

    // A paragraph that says it goes on in the one before is not joined to it when it is a heading, a list item, or has a bookmark or a tracked change: it is read as the paragraph it is.
    void aJoinedParagraphThatIsMoreThanTextIsNotJoined()
    {
        const QByteArray style = "<style:style style:name=\"Pj\" style:family=\"paragraph\"><style:paragraph-properties xmlns:jo=\"urn:jeffoffice:odf-extension\" fo:break-before=\"page\" jo:break-start=\"join\"/></style:style>";
        const struct {
            const char *name;
            const char *joined;
        } cases[] = {{"heading", "<text:h text:style-name=\"Pj\" text:outline-level=\"1\">b</text:h>"},
                     {"list", "<text:list><text:list-item><text:p text:style-name=\"Pj\">b</text:p></text:list-item></text:list>"},
                     {"bookmark", "<text:p text:style-name=\"Pj\"><text:bookmark-start text:name=\"bm\"/>b<text:bookmark-end text:name=\"bm\"/></text:p>"},
                     {"tracked change", "<text:p text:style-name=\"Pj\"><text:change-start text:change-id=\"c2\"/>b<text:change-end text:change-id=\"c2\"/></text:p>"}};
        for (const auto &c : cases) {
            Made m;
            m.autoStyles = style;
            m.body = QByteArray(c.name == QByteArray("tracked change") ? "<text:tracked-changes><text:changed-region text:id=\"c2\"><text:insertion><office:change-info><dc:creator>Bob</dc:creator></office:change-info></text:insertion></text:changed-region></text:tracked-changes>" : "")
                     + "<text:p>a</text:p>" + c.joined;
            const QString doc = document(m);
            QCOMPARE(paragraphsOf(doc).size(), 2);   // (a joined paragraph would be one)
            QVERIFY2(!paragraphsOf(doc).first().contains(QLatin1Char('\f')), c.name);
        }
        // a plain one is joined
        Made m;
        m.autoStyles = style;
        m.body = "<text:p>a</text:p><text:p text:style-name=\"Pj\">b</text:p>";
        QCOMPARE(paragraphsOf(document(m)), (QStringList{QStringLiteral("a\fb")}));
    }

    // A symbol that names a code the writer cannot have written (a control, half of a surrogate pair, a noncharacter, no number) is not a symbol: the span's own text is kept.
    void aSymbolOfACodeNoWriterMakesIsNotASymbol()
    {
        const struct {
            const char *code;
            bool symbol;
        } cases[] = {{"F0B0", true}, {"00E9", true}, {"0", false}, {"1F", false}, {"F01F", false}, {"D800", false}, {"DFFF", false}, {"FFFE", false}, {"FFFF", false}, {"FDD0", false}, {"10000", false}, {"zz", false}};
        for (const auto &c : cases) {
            Made m;
            m.body = QByteArray("<text:p>a<text:span xmlns:jo=\"urn:jeffoffice:odf-extension\" jo:sym=\"") + c.code + "\" jo:sym-font=\"Symbol\">kept</text:span>b</text:p>";
            const QString doc = document(m);
            QVERIFY2(doc.contains(QLatin1String("<w:sym")) == c.symbol, c.code);
            if (!c.symbol) QVERIFY2(paragraphsOf(doc).first() == QLatin1String("akeptb"), c.code);
        }
    }

    // A row of an .odt whose cells are all covered by merged cells that are not there still reads as a row (one empty cell as wide as the table), so the table stays a table and its text is not lost.
    void anOdtRowOfOnlyCoveredCellsIsStillARow()
    {
        Made m;
        m.body = "<table:table table:name=\"T\"><table:table-column table:number-columns-repeated=\"2\"/>"
                 "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>left</text:p></table:table-cell><table:table-cell office:value-type=\"string\"><text:p>right</text:p></table:table-cell></table:table-row>"
                 "<table:table-row><table:covered-table-cell/><table:covered-table-cell/></table:table-row>"
                 "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>below</text:p></table:table-cell><table:table-cell/></table:table-row></table:table>";
        const QString d = part(docxOf(m.build()), QStringLiteral("/word/document.xml"));
        QCOMPARE(d.count(QLatin1String("</w:tr>")), 3);
        QVERIFY(!d.contains(QRegularExpression(QStringLiteral("<w:tr>(<w:trPr>.*?</w:trPr>)?</w:tr>"))));
        QVERIFY(d.contains(QLatin1String("left")));
        QVERIFY(d.contains(QLatin1String("below")));
    }

    void flatForm()
    {
        const QByteArray flat = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document") + kNs + " office:version=\"1.2\" office:mimetype=\"application/vnd.oasis.opendocument.text\">"
                                "<office:styles><style:style style:name=\"Standard\" style:family=\"paragraph\"/></office:styles><office:automatic-styles/><office:body><office:text><text:p>flat words</text:p></office:text></office:body>"
                                "<office:meta><dc:title>Flat</dc:title></office:meta></office:document>";
        QVERIFY(looksLikeOdt(flat));
        const QByteArray docx = docxOf(flat);
        QVERIFY(part(docx, QStringLiteral("/word/document.xml")).contains(QLatin1String("flat words")));
        QVERIFY(part(docx, QStringLiteral("/docProps/core.xml")).contains(QLatin1String("Flat")));
    }

    void propertiesOfTheDocument()
    {
        Made m;
        m.body = "<text:p>x</text:p>";
        m.meta = "<dc:title>My title</dc:title><meta:initial-creator>Ann</meta:initial-creator><dc:subject>Sub</dc:subject><meta:keyword>k1</meta:keyword><meta:creation-date>2024-01-02T03:04:05</meta:creation-date>";
        const QString core = part(docxOf(m.build()), QStringLiteral("/docProps/core.xml"));
        QVERIFY(core.contains(QLatin1String("<dc:title>My title</dc:title>")));
        QVERIFY(core.contains(QLatin1String("<dc:creator>Ann</dc:creator>")));
        QVERIFY(core.contains(QLatin1String("<dcterms:created xsi:type=\"dcterms:W3CDTF\">2024-01-02T03:04:05Z")));
    }

    void tabStopsAndBreaks()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:break-after=\"page\"><style:tab-stops><style:tab-stop style:position=\"2in\" style:type=\"right\" "
                       "style:leader-style=\"dotted\" style:leader-text=\".\"/><style:tab-stop style:position=\"3in\"/></style:tab-stops></style:paragraph-properties></style:style>";
        m.body = "<text:p text:style-name=\"P1\">a<text:tab/>b</text:p><text:p>next</text:p><text:p>third</text:p>";
        const QString d = document(m);
        QVERIFY(d.contains(QLatin1String("<w:tab w:val=\"right\" w:leader=\"dot\" w:pos=\"2880\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:tab w:val=\"left\" w:pos=\"4320\"/>")));
        // The page break after the first paragraph begins the page of the next.
        const int next = d.indexOf(QLatin1String(">next<"));
        QVERIFY(next > 0);
        QVERIFY(d.lastIndexOf(QLatin1String("<w:pageBreakBefore/>"), next) > d.indexOf(QLatin1String(">a<")));
        QVERIFY(d.indexOf(QLatin1String("<w:pageBreakBefore/>"), d.indexOf(QLatin1String(">next<"))) < 0);
    }

    // The four settings JeffWrite sets that an .odt keeps: the distance of the default tab stops (style:tab-stop-distance of the default paragraph style), character scaling
    // (style:text-scale), a paragraph's suppressed line numbers (text:number-lines), and bar tab stops (a list of places of ours, jo:bar-tabs, for ODF has no bar).
    void defaultTabsScalingLineNumbersAndBarStopsAreReadFromAnOdt()
    {
        Made m;
        m.styles = "<style:default-style style:family=\"paragraph\"><style:paragraph-properties style:tab-stop-distance=\"0.25in\"/></style:default-style>";
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties xmlns:jo=\"urn:jeffoffice:odf-extension\" text:number-lines=\"false\" jo:bar-tabs=\"1440 2880\">"
                       "<style:tab-stops><style:tab-stop style:position=\"1in\" style:type=\"right\"/></style:tab-stops></style:paragraph-properties></style:style>"
                       "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties style:text-scale=\"200%\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">a<text:span text:style-name=\"T1\">wide</text:span></text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml")), s = part(docx, QStringLiteral("/word/settings.xml"));
        QVERIFY2(s.contains(QLatin1String("<w:defaultTabStop w:val=\"360\"/>")), qPrintable(s));
        QVERIFY2(d.contains(QLatin1String("<w:suppressLineNumbers/>")), qPrintable(d));
        QVERIFY2(d.contains(QLatin1String("<w:tab w:val=\"bar\" w:pos=\"1440\"/>")) && d.contains(QLatin1String("<w:tab w:val=\"bar\" w:pos=\"2880\"/>")), qPrintable(d));
        QVERIFY2(d.contains(QLatin1String("<w:tab w:val=\"right\" w:pos=\"1440\"/>")), qPrintable(d));
        QVERIFY2(d.contains(QLatin1String("<w:w w:val=\"200\"/>")), qPrintable(d));
        // A list of bar places that a file makes enormous is held to 64 stops, and a place that is no number or is out of range is none.
        QByteArray many = "1 x -5 99999999";
        for (int i = 0; i < 200; ++i) many += " " + QByteArray::number(i + 10);
        Made h;
        h.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties xmlns:jo=\"urn:jeffoffice:odf-extension\" jo:bar-tabs=\"" + many + "\"/></style:style>";
        h.body = "<text:p text:style-name=\"P1\">x</text:p>";
        QCOMPARE(int(document(h).count(QLatin1String("w:val=\"bar\""))), 64);
    }

    void defaultTabsScalingLineNumbersAndBarStopsAreWrittenToAnOdt()
    {
        const QString c = odtContentOf("<w:p><w:pPr><w:suppressLineNumbers/><w:tabs><w:tab w:val=\"bar\" w:pos=\"1440\"/><w:tab w:val=\"left\" w:pos=\"720\"/></w:tabs></w:pPr>"
                                       "<w:r><w:rPr><w:w w:val=\"150\"/></w:rPr><w:t>wide</w:t></w:r></w:p><w:p><w:pPr><w:suppressLineNumbers w:val=\"0\"/></w:pPr><w:r><w:t>numbered</w:t></w:r></w:p>");
        QVERIFY2(c.contains(QLatin1String("text:number-lines=\"false\"")) && c.contains(QLatin1String("text:number-lines=\"true\"")), qPrintable(c));
        QVERIFY2(c.contains(QLatin1String("jo:bar-tabs=\"1440\"")), qPrintable(c));
        QVERIFY2(c.contains(QLatin1String("<style:tab-stop style:position=\"0.5000in\" style:type=\"left\"/>")) && !c.contains(QLatin1String("<style:tab-stop style:position=\"1.0000in\"")), qPrintable(c));
        QVERIFY2(c.contains(QLatin1String("style:text-scale=\"150%\"")), qPrintable(c));
        QVERIFY(wellFormed(c.toUtf8()));
    }

    // ---- hostile input

    void refusesWhatItShould()
    {
        Made m;
        m.body = "<text:p>x</text:p>";
        QByteArray odt = m.build();
        // Truncated at every tenth of its length.
        for (int i = 1; i < 10; ++i) {
            QString error;
            const QByteArray cut = odt.left(odt.size() * i / 10);
            QVERIFY(docxOf(cut, nullptr, &error).isEmpty());
        }
        // A DOCTYPE (an entity) is refused.
        Made dt = m;
        QByteArray content = QByteArray("<?xml version=\"1.0\"?><!DOCTYPE office:document-content [<!ENTITY e \"boom\">]><office:document-content") + kNs + "><office:body><office:text><text:p>&e;</text:p></office:text></office:body></office:document-content>";
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), kMime, false);
        z.add(QStringLiteral("content.xml"), content, true);
        QString error;
        QVERIFY(docxOf(z.finish(), nullptr, &error).isEmpty());
        // A document with parts that are encrypted.
        jp::ZipWriter z2;
        z2.add(QStringLiteral("mimetype"), kMime, false);
        z2.add(QStringLiteral("content.xml"), "<x/>", true);
        z2.add(QStringLiteral("META-INF/manifest.xml"), "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\"><manifest:file-entry manifest:full-path=\"content.xml\"><manifest:encryption-data/></manifest:file-entry></manifest:manifest>", true);
        QVERIFY(docxOf(z2.finish(), nullptr, &error).isEmpty());
        QVERIFY(error.contains(QLatin1String("password")));
        // More parts than a package may have.
        jp::ZipWriter z3;
        z3.add(QStringLiteral("mimetype"), kMime, false);
        z3.add(QStringLiteral("content.xml"), "<x/>", true);
        for (int i = 0; i < 10100; ++i) z3.add(QStringLiteral("Pictures/%1.png").arg(i), "x", false);
        QVERIFY(docxOf(z3.finish(), nullptr, &error).isEmpty());
        // Nothing that is not XML.
        jp::ZipWriter z4;
        z4.add(QStringLiteral("mimetype"), kMime, false);
        z4.add(QStringLiteral("content.xml"), "not xml at all", true);
        QVERIFY(docxOf(z4.finish(), nullptr, &error).isEmpty());
    }

    void boundsWhatAFileCanMultiply()
    {
        // Counts of repeats are cut: a million columns and rows of nothing take no time.
        Made m;
        m.body = "<table:table><table:table-column table:number-columns-repeated=\"1000000000\"/><table:table-row table:number-rows-repeated=\"1000000000\"><table:table-cell table:number-columns-repeated=\"1000000000\"><text:p>x</text:p></table:table-cell></table:table-row></table:table>";
        QElapsedTimer timer;
        timer.start();
        const QByteArray docx = docxOf(m.build());
        QVERIFY(timer.elapsed() < 20000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
        QVERIFY(!docx.isEmpty());
        QVERIFY(document(m).size() < 40 * 1024 * 1024);
        // Nesting: spans, lists, and tables a few thousand deep read in bounded time, or are refused, never crash.
        Made nest;
        QByteArray inner = "x";
        for (int i = 0; i < 3000; ++i) inner = "<text:span>" + inner + "</text:span>";
        nest.body = "<text:p>" + inner + "</text:p>";
        timer.restart();
        document(nest);
        QVERIFY(timer.elapsed() < 20000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
        // Lists and tables nested as deep as the XML reader allows (about 250 levels) or a little more: read in bounded time or refused, never a crash.
        for (const int depth : {30, 70, 300}) {
            Made lists;
            QByteArray inner = "<text:p>x</text:p>";
            for (int i = 0; i < depth; ++i) inner = "<text:list><text:list-item>" + inner + "</text:list-item></text:list>";
            lists.body = inner;
            timer.restart();
            const QByteArray l = docxOf(lists.build());
            QVERIFY(timer.elapsed() < 20000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
            if (depth <= 70) QVERIFY(!l.isEmpty());
            Made tables;
            inner = "<text:p>x</text:p>";
            for (int i = 0; i < depth; ++i) inner = "<table:table><table:table-column/><table:table-row><table:table-cell>" + inner + "</table:table-cell></table:table-row></table:table>";
            tables.body = inner;
            timer.restart();
            const QByteArray tdocx = docxOf(tables.build());
            QVERIFY(timer.elapsed() < 20000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
            if (depth <= 30) QVERIFY(!tdocx.isEmpty());
        }
        // A numbered list used many times makes a bounded number of numberings.
        Made many;
        many.autoStyles = "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\"/></text:list-style>";
        for (int i = 0; i < 30000; ++i) many.body += "<text:list text:style-name=\"L1\"><text:list-item><text:p>x</text:p></text:list-item></text:list>";
        timer.restart();
        const QString numbering = part(docxOf(many.build()), QStringLiteral("/word/numbering.xml"));
        QVERIFY(timer.elapsed() < 20000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
        QVERIFY(numbering.count(QLatin1String("<w:num ")) <= kMaxMadeParts + 1);
    }

    void mutatedFilesDoNotCrash()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>";
        m.autoStyles = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\"/></style:style>";
        m.body = "<text:h text:outline-level=\"1\">T</text:h><text:p>a <text:span text:style-name=\"T1\">b</text:span></text:p><table:table><table:table-column/><table:table-row><table:table-cell><text:p>c</text:p></table:table-cell></table:table-row></table:table>"
                 "<text:list><text:list-item><text:p>l</text:p></text:list-item></text:list>";
        // The parts as XML text, mutated byte by byte (a byte replaced, a piece cut, a piece doubled), and packed again.
        const QString content = QStringLiteral("content.xml");
        jp::ZipEntry dummy;
        Q_UNUSED(dummy);
        Package source;
        QVERIFY(source.readOpenDocument(m.build()));
        const QByteArray xml = source.part(QStringLiteral("/content.xml"));
        unsigned seed = 12345;
        auto next = [&] { seed = seed * 1103515245u + 12345u; return (seed >> 8) & 0xFFFFFF; };
        int opened = 0, refused = 0;
        for (int i = 0; i < 300; ++i) {
            QByteArray mutated = xml;
            const int at = int(next() % uint(mutated.size()));
            switch (i % 3) {
            case 0: mutated[at] = char(next() & 0xFF); break;
            case 1: mutated.remove(at, int(next() % 40)); break;
            default: mutated.insert(at, mutated.mid(at, int(next() % 60))); break;
            }
            jp::ZipWriter z;
            z.add(QStringLiteral("mimetype"), kMime, false);
            z.add(content, mutated, true);
            z.add(QStringLiteral("styles.xml"), source.part(QStringLiteral("/styles.xml")), true);
            QString error;
            const QByteArray docx = docxOf(z.finish(), nullptr, &error);
            if (docx.isEmpty()) ++refused;
            else {
                ++opened;
                wml::Document doc;
                wml::readDocx(docx, &doc);   // (whatever it made, the .docx reader takes without crashing)
            }
        }
        QVERIFY(opened + refused == 300);
        QVERIFY(opened > 0);
    }

    // ---- writing

    void writesAValidPackage()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>";
        m.body = "<text:h text:outline-level=\"1\">Head</text:h><text:p>x<draw:frame text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        QString error;
        QStringList notes;
        const QByteArray odt = fromDocxPackage(docx, &error, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(error));
        QVector<jp::ZipEntry> entries;
        QVERIFY(jp::readZipDirectory(odt, &entries));
        QCOMPARE(entries.first().name, QStringLiteral("mimetype"));
        QCOMPARE(int(entries.first().method), 0);
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        QCOMPARE(files.value(QStringLiteral("mimetype")), QByteArray(kMime));
        for (const QString &n : {QStringLiteral("content.xml"), QStringLiteral("styles.xml"), QStringLiteral("meta.xml"), QStringLiteral("META-INF/manifest.xml")}) {
            QVERIFY2(files.contains(n), qPrintable(n));
            QVERIFY2(wellFormed(files.value(n)), qPrintable(n));
        }
        // The manifest lists every part but itself and the mimetype.
        const QString manifest = QString::fromUtf8(files.value(QStringLiteral("META-INF/manifest.xml")));
        for (auto it = files.cbegin(); it != files.cend(); ++it) {
            if (it.key() == QLatin1String("mimetype") || it.key() == QLatin1String("META-INF/manifest.xml")) continue;
            QVERIFY2(manifest.contains(QLatin1String("manifest:full-path=\"") + it.key() + QLatin1String("\"")), qPrintable(it.key()));
        }
        QVERIFY(files.contains(QStringLiteral("Pictures/image1.png")));
        QCOMPARE(files.value(QStringLiteral("Pictures/image1.png")), kPng);
        // The settings say how the text is laid out, with the prefix they name the set by declared (another program ignores the set without it).
        {
            wml::XmlTree st;
            QVERIFY(st.parse(files.value(QStringLiteral("settings.xml")), nullptr));
            QVERIFY(st.namespaceId("http://openoffice.org/2004/office") >= 0);
            bool found = false;
            for (int i = findElement(st, "config-item"); i >= 0 && !found; i = findElement(st, "config-item", i))
                found = st.attrAny(i, "name") == QLatin1String("AddParaTableSpacing") && st.text(i) == QLatin1String("false");
            QVERIFY(found);
        }
        QVERIFY(QString::fromUtf8(files.value(QStringLiteral("content.xml"))).contains(QLatin1String("<text:h")));
        // And it reads again.
        QVERIFY(looksLikeOdt(odt));
        QVERIFY(!docxOf(odt).isEmpty());
    }

    void roundTripKeepsWhatItHolds()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>"
                   "<style:style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:default-outline-level=\"1\"><style:text-properties fo:font-size=\"16pt\"/></style:style>";
        m.autoStyles = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\" fo:color=\"#ff0000\"/></style:style>"
                       "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:text-align=\"center\" fo:margin-left=\"0.5in\"/></style:style>"
                       "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\" style:num-suffix=\".\"/></text:list-style>";
        m.layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" fo:margin-top=\"1in\" fo:margin-bottom=\"1in\" fo:margin-left=\"1in\" fo:margin-right=\"1in\"/></style:page-layout>";
        m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"><style:header><text:p>head text</text:p></style:header><style:footer><text:p>page <text:page-number>1</text:page-number></text:p></style:footer></style:master-page>";
        m.body = "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">Heading words</text:h><text:p text:style-name=\"P1\">plain <text:span text:style-name=\"T1\">bold red</text:span> tail</text:p>"
                 "<text:list text:style-name=\"L1\"><text:list-item><text:p>item one</text:p></text:list-item><text:list-item><text:p>item two</text:p></text:list-item></text:list>"
                 "<table:table><table:table-column table:number-columns-repeated=\"2\"/><table:table-row><table:table-cell table:number-columns-spanned=\"2\"><text:p>spanned</text:p></table:table-cell><table:covered-table-cell/></table:table-row>"
                 "<table:table-row><table:table-cell><text:p>c1</text:p></table:table-cell><table:table-cell><text:p>c2</text:p></table:table-cell></table:table-row></table:table>"
                 "<text:p>note<text:note text:note-class=\"footnote\"><text:note-citation>1</text:note-citation><text:note-body><text:p>foot</text:p></text:note-body></text:note></text:p>";
        const QByteArray odt2 = fromDocxPackage(docxOf(m.build()));
        QVERIFY(!odt2.isEmpty());
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt2, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml"))), s = QString::fromUtf8(files.value(QStringLiteral("styles.xml")));
        QVERIFY(c.contains(QLatin1String("<text:h")));
        QVERIFY(c.contains(QLatin1String("text:outline-level=\"1\"")));
        QVERIFY(c.contains(QLatin1String("fo:font-weight=\"bold\"")));
        QVERIFY(c.contains(QLatin1String("fo:color=\"#FF0000\"")));
        QVERIFY(c.contains(QLatin1String("fo:text-align=\"center\"")));
        QVERIFY(c.contains(QLatin1String("<text:list text:style-name=\"L")));
        QVERIFY(c.contains(QLatin1String("style:num-format=\"1\"")));
        QVERIFY(c.contains(QLatin1String("table:number-columns-spanned=\"2\"")));
        QVERIFY(c.contains(QLatin1String("<table:covered-table-cell/>")));
        QVERIFY(c.contains(QLatin1String("<text:note ")));
        QVERIFY(c.contains(QLatin1String("foot")));
        QVERIFY(s.contains(QLatin1String("<style:header>")));
        QVERIFY(s.contains(QLatin1String("head text")));
        QVERIFY(s.contains(QLatin1String("<text:page-number")));
        QVERIFY(s.contains(QLatin1String("fo:page-width=\"8.5000in\"")));
        QVERIFY(s.contains(QLatin1String("style:default-outline-level=\"1\"")));
        // The text is as it was after the whole round trip.
        const QString again = part(docxOf(odt2), QStringLiteral("/word/document.xml"));
        for (const char *word : {"Heading words", "bold red", "item two", "spanned", "c2", "note"}) QVERIFY2(again.contains(QLatin1String(word)), word);
        QVERIFY(again.contains(QLatin1String("<w:b/>")));
        QVERIFY(again.contains(QLatin1String("gridSpan")));
    }

    void writesFieldsPicturesSectionsAndPageBreaks()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                          "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                          "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
        const QByteArray body =
            "<w:p><w:pPr><w:pStyle w:val=\"Title\"/><w:jc w:val=\"both\"/><w:spacing w:before=\"240\" w:after=\"120\" w:line=\"360\" w:lineRule=\"auto\"/><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr>"
            "<w:r><w:rPr><w:b/><w:i/><w:u w:val=\"single\"/><w:color w:val=\"FF0000\"/><w:sz w:val=\"28\"/></w:rPr><w:t xml:space=\"preserve\">Styled  words </w:t></w:r><w:r><w:tab/><w:t>tabbed</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>before break</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after break</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>Page </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
            "<w:r><w:t>7</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>9</w:t></w:r></w:fldSimple>"
            "<w:bookmarkStart w:id=\"3\" w:name=\"mark\"/><w:r><w:t>marked</w:t></w:r><w:bookmarkEnd w:id=\"3\"/><w:hyperlink r:id=\"rId9\"><w:r><w:t>site</w:t></w:r></w:hyperlink></w:p>"
            "<w:p><w:r><w:drawing><wp:inline><wp:extent cx=\"914400\" cy=\"457200\"/><wp:docPr id=\"1\" name=\"Picture 1\" descr=\"a picture\"/><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
            "<pic:pic><pic:blipFill><a:blip r:embed=\"rId8\"/></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>"
            "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr></w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>two</w:t></w:r></w:p>"
            "<w:sectPr><w:type w:val=\"continuous\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/><w:cols w:num=\"2\" w:space=\"720\"/></w:sectPr>";
        PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\"?><w:document ") + kW + "><w:body>" + body + "</w:body></w:document>", QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/media/image1.png"), kPng, QStringLiteral("image/png"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId8"), rel + QStringLiteral("image"), QStringLiteral("/word/media/image1.png"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId9"), rel + QStringLiteral("hyperlink"), QStringLiteral("http://example.org/"), true);
        QString error;
        const QByteArray odt = fromDocxPackage(w.finish(), &error);
        QVERIFY2(!odt.isEmpty(), qPrintable(error));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(c.contains(QLatin1String("fo:text-align=\"justify\"")));
        QVERIFY(c.contains(QLatin1String("fo:line-height=\"150%\"")));
        QVERIFY(c.contains(QLatin1String("fo:text-indent=\"-0.2500in\"")));
        QVERIFY(c.contains(QLatin1String("fo:font-size=\"14pt\"")));
        QVERIFY(c.contains(QLatin1String("style:text-underline-style=\"solid\"")));
        QVERIFY(c.contains(QLatin1String("Styled <text:s/>words")));
        QVERIFY(c.contains(QLatin1String("<text:tab/>tabbed")));
        // The page break begins a page in the paragraph after it.
        const int after = c.indexOf(QLatin1String(">after break<"));
        QVERIFY(after > 0);
        QVERIFY(c.contains(QLatin1String("fo:break-before=\"page\"")));
        QVERIFY(c.contains(QLatin1String("<text:page-number text:select-page=\"current\">7</text:page-number>")));
        QVERIFY(c.contains(QLatin1String("<text:page-count>9</text:page-count>")));
        QVERIFY(c.contains(QLatin1String("<text:bookmark-start text:name=\"mark\"/>")));
        QVERIFY(c.contains(QLatin1String("<text:bookmark-end text:name=\"mark\"/>")));
        QVERIFY(c.contains(QLatin1String("xlink:href=\"http://example.org/\"")));
        QVERIFY(c.contains(QLatin1String("<draw:frame")));
        QVERIFY(c.contains(QLatin1String("<svg:desc>a picture</svg:desc>")));
        QVERIFY(c.contains(QLatin1String("svg:width=\"1.0000in\"")));
        QVERIFY(c.contains(QLatin1String("<text:section")));
        QVERIFY(files.contains(QStringLiteral("Pictures/image1.png")));
        const QString s = QString::fromUtf8(files.value(QStringLiteral("styles.xml")));
        QVERIFY(s.contains(QLatin1String("<style:columns fo:column-count=\"2\"")) || c.contains(QLatin1String("<style:columns fo:column-count=\"2\"")));
        QVERIFY(wellFormed(files.value(QStringLiteral("styles.xml"))));
    }

    void writesTablesListsHeadersNotesAndComments()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        auto xml = [&](const char *root, const QByteArray &body) { return QByteArray("<?xml version=\"1.0\"?><w:") + root + " " + kW + ">" + body + "</w:" + root + ">"; };
        const QByteArray body =
            "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Chapter</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>step one</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"1\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>sub step</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>interrupted</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>step two</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"2\"/></w:numPr></w:pPr><w:r><w:t>a bullet</w:t></w:r></w:p>"
            "<w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"pct\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"8\" w:color=\"FF0000\"/><w:insideH w:val=\"single\" w:sz=\"4\"/></w:tblBorders></w:tblPr>"
            "<w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
            "<w:tr><w:trPr><w:tblHeader/></w:trPr><w:tc><w:tcPr><w:vMerge w:val=\"restart\"/><w:shd w:val=\"clear\" w:fill=\"DDEEFF\"/></w:tcPr><w:p><w:r><w:t>tall</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>b1</w:t></w:r></w:p></w:tc></w:tr>"
            "<w:tr><w:tc><w:tcPr><w:vMerge/></w:tcPr><w:p/></w:tc><w:tc><w:p><w:r><w:t>b2</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
            "<w:p><w:pPr><w:tabs><w:tab w:val=\"right\" w:pos=\"4320\"/></w:tabs></w:pPr><w:r><w:t>note here</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r>"
            "<w:commentRangeStart w:id=\"5\"/><w:r><w:t>commented</w:t></w:r><w:commentRangeEnd w:id=\"5\"/><w:r><w:commentReference w:id=\"5\"/></w:r>"
            "<w:hyperlink w:anchor=\"target\"><w:r><w:t>jump</w:t></w:r></w:hyperlink></w:p>"
            "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdH1\"/><w:headerReference w:type=\"first\" r:id=\"rIdH2\"/><w:footerReference w:type=\"default\" r:id=\"rIdF1\"/><w:titlePg/>"
            "<w:pgSz w:w=\"11906\" w:h=\"16838\" w:orient=\"portrait\"/><w:pgMar w:top=\"1700\" w:right=\"1000\" w:bottom=\"1700\" w:left=\"1200\" w:header=\"600\" w:footer=\"700\" w:gutter=\"0\"/></w:sectPr>";
        PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QString type = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");
        w.addPart(QStringLiteral("/word/document.xml"), xml("document", "<w:body>" + body + "</w:body>"), type + QStringLiteral("document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/styles.xml"),
                  xml("styles", "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                                "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:keepNext/><w:outlineLvl w:val=\"0\"/></w:pPr><w:rPr><w:b/><w:sz w:val=\"32\"/></w:rPr></w:style>"),
                  type + QStringLiteral("styles+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        w.addPart(QStringLiteral("/word/numbering.xml"),
                  xml("numbering", "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1)\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>"
                                   "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"lowerRoman\"/><w:lvlText w:val=\"%1.%2\"/></w:lvl></w:abstractNum>"
                                   "<w:abstractNum w:abstractNumId=\"1\"><w:lvl w:ilvl=\"0\"><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/></w:lvl></w:abstractNum>"
                                   "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>"),
                  type + QStringLiteral("numbering+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
        w.addPart(QStringLiteral("/word/settings.xml"), xml("settings", "<w:evenAndOddHeaders/>"), type + QStringLiteral("settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
        auto addPart = [&](const QString &name, const char *root, const QByteArray &inner, const QString &suffix, const QString &id) {
            w.addPart(name, xml(root, inner), type + suffix);
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), id, rel + suffix.left(suffix.indexOf(QLatin1Char('+'))), name);
        };
        addPart(QStringLiteral("/word/header1.xml"), "hdr", "<w:p><w:r><w:t>default header</w:t></w:r></w:p>", QStringLiteral("header+xml"), QStringLiteral("rIdH1"));
        addPart(QStringLiteral("/word/header2.xml"), "hdr", "<w:p><w:r><w:t>first page header</w:t></w:r></w:p>", QStringLiteral("header+xml"), QStringLiteral("rIdH2"));
        addPart(QStringLiteral("/word/footer1.xml"), "ftr", "<w:p><w:r><w:t xml:space=\"preserve\">page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple></w:p>", QStringLiteral("footer+xml"), QStringLiteral("rIdF1"));
        addPart(QStringLiteral("/word/footnotes.xml"), "footnotes", "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>the note text</w:t></w:r></w:p></w:footnote>", QStringLiteral("footnotes+xml"), QStringLiteral("rIdN"));
        addPart(QStringLiteral("/word/comments.xml"), "comments", "<w:comment w:id=\"5\" w:author=\"Cy\" w:initials=\"C\"><w:p><w:r><w:t>comment text</w:t></w:r></w:p></w:comment>", QStringLiteral("comments+xml"), QStringLiteral("rIdC"));
        QString error;
        const QByteArray odt = fromDocxPackage(w.finish(), &error);
        QVERIFY2(!odt.isEmpty(), qPrintable(error));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml"))), st = QString::fromUtf8(files.value(QStringLiteral("styles.xml")));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(wellFormed(files.value(QStringLiteral("styles.xml"))));
        // The heading, and its numbering of the style's own.
        QVERIFY(c.contains(QLatin1String("<text:h text:style-name=\"P1\" text:outline-level=\"1\">Chapter</text:h>")));
        // (The first paragraph names the master page, so its style is an automatic one that follows the heading's.)
        QVERIFY(c.contains(QLatin1String("style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"Heading_20_1\" style:master-page-name=\"MP1\"")));
        // Lists: the second run of the first list goes on (continue-numbering), nested items are a list in an item, and the bullet list is its own.
        QCOMPARE(c.count(QLatin1String("<text:list text:style-name=\"L1\"")), 2);
        QVERIFY(c.contains(QLatin1String("<text:list text:style-name=\"L1\" text:continue-numbering=\"true\">")));
        QVERIFY(c.contains(QRegularExpression(QStringLiteral("sub step</text:p>"))));
        QVERIFY(c.contains(QLatin1String("<text:list text:style-name=\"L2\">")));
        QVERIFY(c.contains(QLatin1String("style:num-suffix=\")\"")));
        QVERIFY(c.contains(QLatin1String("style:num-format=\"i\"")));
        QVERIFY(c.contains(QLatin1String("text:display-levels=\"2\"")));
        // The table: percent width, header row, merged cells, borders and fill.
        QVERIFY(c.contains(QLatin1String("style:rel-width=\"100%\"")));
        QVERIFY(c.contains(QLatin1String("<table:table-header-rows>")));
        QVERIFY(c.contains(QLatin1String("table:number-rows-spanned=\"2\"")));
        QVERIFY(c.contains(QLatin1String("<table:covered-table-cell/>")));
        QVERIFY(c.contains(QLatin1String("fo:background-color=\"#DDEEFF\"")));
        QVERIFY(c.contains(QLatin1String("fo:border-top=\"1.000pt solid #FF0000\"")));
        QVERIFY(c.contains(QLatin1String("fo:border-bottom=\"0.500pt solid #000000\"")));
        // The tab stop, the note, the comment, and the link.
        QVERIFY(c.contains(QLatin1String("<style:tab-stop style:position=\"3.0000in\" style:type=\"right\"/>")));
        QVERIFY(c.contains(QRegularExpression(QStringLiteral("<text:note text:id=\"ftn1\" text:note-class=\"footnote\"><text:note-citation>1</text:note-citation><text:note-body><text:p [^>]*>the note text</text:p>"))));
        QVERIFY(c.contains(QLatin1String("<office:annotation office:name=\"cmt5\">")));
        QVERIFY(c.contains(QLatin1String("<dc:creator>Cy</dc:creator>")));
        QVERIFY(c.contains(QLatin1String("<office:annotation-end office:name=\"cmt5\"/>")));
        QVERIFY(c.indexOf(QLatin1String("<office:annotation ")) < c.indexOf(QLatin1String("commented")) && c.indexOf(QLatin1String("commented")) < c.indexOf(QLatin1String("<office:annotation-end")));
        QVERIFY(c.contains(QLatin1String("xlink:href=\"#target\"")));
        // The page: A4, margins (the header and footer's room is the rest of the top and bottom margin), headers of three kinds, the footer's field.
        QVERIFY(st.contains(QLatin1String("fo:page-width=\"8.2681in\"")));
        QVERIFY(st.contains(QLatin1String("fo:margin-top=\"0.4167in\"")));
        QVERIFY(st.contains(QLatin1String("fo:margin-left=\"0.8333in\"")));
        QVERIFY(st.contains(QLatin1String("<style:header>")));
        QVERIFY(st.contains(QLatin1String("<style:header-left>")));
        QVERIFY(st.contains(QLatin1String("<style:header-first>")));
        QVERIFY(st.contains(QLatin1String("first page header")));
        QVERIFY(st.contains(QLatin1String("<style:footer>")));
        QVERIFY(st.contains(QLatin1String("<text:page-number text:select-page=\"current\">1</text:page-number>")));
        // The reader takes the file back as the same document.
        const QString again = part(docxOf(odt), QStringLiteral("/word/document.xml"));
        for (const char *word : {"Chapter", "step one", "sub step", "interrupted", "step two", "a bullet", "tall", "b2", "note here", "commented", "jump"}) QVERIFY2(again.contains(QLatin1String(word)), word);
        QVERIFY(again.contains(QLatin1String("<w:titlePg/>")));
    }

    // ---- what the review found

    void spacesAreBoundedForAParagraphAndTheDocument()
    {
        Made m;
        QByteArray para = "<text:p>";
        for (int i = 0; i < 1000; ++i) para += "<text:s text:c=\"10000\"/>";
        para += "x</text:p>";
        for (int i = 0; i < 100; ++i) m.body += para;   // (10 million spaces asked for in each paragraph, a billion in all)
        QElapsedTimer timer;
        timer.start();
        QStringList notes;
        const QString d = document(m, &notes);
        QVERIFY(timer.elapsed() < 30000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
        QVERIFY2(d.size() < 120 * 1024 * 1024, qPrintable(QString::number(d.size())));
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("spaces")));
    }

    void repeatedCellsAndRowsAreBoundedInBytes()
    {
        Made m;
        const QByteArray big(200 * 1024, 'w');
        m.body = "<table:table><table:table-column table:number-columns-repeated=\"1000\"/><table:table-row table:number-rows-repeated=\"200\"><table:table-cell table:number-columns-repeated=\"1000\"><text:p>" + big +
                 "</text:p></table:table-cell></table:table-row></table:table>";
        QElapsedTimer timer;
        timer.start();
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        QVERIFY(timer.elapsed() < 120000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
        QVERIFY(!docx.isEmpty());
        Package p;
        QVERIFY(p.read(docx));
        QVERIFY2(p.partSize(QStringLiteral("/word/document.xml")) < qint64(400) << 20, qPrintable(QString::number(p.partSize(QStringLiteral("/word/document.xml")))));
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("repeated")));
    }

    void repeatedCellsAloneAreBoundedInBytes()
    {
        // One row, so only the copies of the cell are what could multiply.
        Made m;
        const QByteArray big(600 * 1024, 'w');
        m.body = "<table:table><table:table-column table:number-columns-repeated=\"1000\"/><table:table-row><table:table-cell table:number-columns-repeated=\"1000\"><text:p>" + big + "</text:p></table:table-cell></table:table-row></table:table>";
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        QVERIFY(!docx.isEmpty());
        Package p;
        QVERIFY(p.read(docx));
        QVERIFY2(p.partSize(QStringLiteral("/word/document.xml")) < qint64(400) << 20, qPrintable(QString::number(p.partSize(QStringLiteral("/word/document.xml")))));
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("repeated")));
    }

    void hugeLengthsAreClamped()
    {
        bool ok = false;
        QCOMPARE(lengthToTwips(QStringLiteral("1e300pt"), &ok), qint64(kMaxLengthPoints * 20));
        QVERIFY(ok);
        QCOMPARE(lengthToTwips(QStringLiteral("-1e300cm"), &ok), qint64(-kMaxLengthPoints * 20));
        QCOMPARE(lengthToEmu(QStringLiteral("1e300in"), &ok), qint64(kMaxLengthPoints * 12700));
        QCOMPARE(clampedInt(1e300), qint64(1e12));
        QCOMPARE(clampedInt(std::nan("")), qint64(0));
        Made m;
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:margin-left=\"1e300pt\" fo:text-indent=\"-1e300cm\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">x<draw:frame text:anchor-type=\"as-char\" svg:width=\"1e300in\" svg:height=\"1e300cm\"><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        QVERIFY(!docx.isEmpty());
        QVERIFY(wellFormed(part(docx, QStringLiteral("/word/document.xml")).toUtf8()));
        // And the other way: a .docx whose numbers are far past what a length can be.
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:pPr><w:ind w:left=\"1e300\" w:hanging=\"-1e300\"/><w:spacing w:before=\"1e999\"/></w:pPr><w:r><w:t>x</w:t></w:r></w:p><w:tbl><w:tblGrid><w:gridCol w:w=\"1e300\"/></w:tblGrid>"
                                                             "<w:tr><w:tc><w:p/></w:tc></w:tr></w:tbl><w:sectPr><w:pgSz w:w=\"1e300\" w:h=\"-1e300\"/></w:sectPr></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        const QByteArray odt = fromDocxPackage(w.finish());
        QVERIFY(!odt.isEmpty());
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(wellFormed(files.value(QStringLiteral("styles.xml"))));
    }

    void charactersXmlCannotHoldAreDroppedBothWays()
    {
        QCOMPARE(esc(QStringLiteral("a\x01" "b\x0b" "c")), QByteArray("abc"));
        QCOMPARE(esc(QString(QChar(0xD800)) + QStringLiteral("x") + QChar(0xDC00)), QByteArray("x"));
        QCOMPARE(esc(QStringLiteral("a\xF0\x9F\x98\x80"), false), QByteArray("a\xC3\xB0\xC2\x9F\xC2\x98\xC2\x80"));   // (Latin-1 text: its characters are kept)
        Made m;
        m.body = "<text:p><text:bookmark text:name=\"b&#1;m\"/><draw:frame draw:name=\"n&#2;&#xFFFE;\" text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame>"
                 "<text:a xlink:href=\"http://e.com/&#3;x\">l</text:a><office:annotation><dc:creator>A&#4;B</dc:creator><text:p>c</text:p></office:annotation>"
                 "<text:change-start text:change-id=\"c1\"/>t<text:change-end text:change-id=\"c1\"/></text:p>"
                 "<text:tracked-changes><text:changed-region text:id=\"c1\"><text:insertion><office:change-info><dc:creator>Z&#5;Z</dc:creator></office:change-info></text:insertion></text:changed-region></text:tracked-changes>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        QVERIFY(!docx.isEmpty());
        Package p;
        QVERIFY(p.read(docx));
        for (const QString &n : p.partNames())
            if (n.endsWith(QLatin1String(".xml"))) QVERIFY2(wellFormed(p.part(n)), qPrintable(n));
        // The writer, from a .docx whose names and authors hold them.
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:bookmarkStart w:id=\"1\" w:name=\"b&#1;m\"/><w:r><w:t>x</w:t></w:r><w:bookmarkEnd w:id=\"1\"/><w:ins w:id=\"2\" w:author=\"A&#2;B\"><w:r><w:t>i</w:t></w:r></w:ins>"
                                                            "<w:r><w:rPr><w:color w:val=\"12&quot; x=&quot;\"/></w:rPr><w:t>c</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        const QByteArray odt = fromDocxPackage(w.finish());
        QVERIFY(!odt.isEmpty());
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(!QString::fromUtf8(files.value(QStringLiteral("content.xml"))).contains(QLatin1String("x=")));   // (a color that is no hex adds no attribute)
    }

    void namesInFieldCodesCannotAddSwitches()
    {
        Made m;
        m.body = "<text:p><text:bookmark-ref text:ref-name=\"x&quot; \\* MERGEFORMAT \\h &quot;y\" text:reference-format=\"text\">r</text:bookmark-ref>"
                 "<text:sequence text:name=\"F&quot; \\r 9\">1</text:sequence></text:p>"
                 "<text:illustration-index><text:illustration-index-source text:caption-sequence-name=\"c&quot; \\o &quot;1-9\"/><text:index-body><text:p>e</text:p></text:index-body></text:illustration-index>";
        const QString instr = instructionsOf(document(m));
        QRegularExpression ref(QStringLiteral("^ REF \"(?:[^\"\\\\]|\\\\.)*\" \\\\h $"), QRegularExpression::MultilineOption);
        QVERIFY2(instr.contains(ref), qPrintable(instr));
        QRegularExpression seq(QStringLiteral("^ SEQ \"(?:[^\"\\\\]|\\\\.)*\" \\\\\\* ARABIC $"), QRegularExpression::MultilineOption);
        QVERIFY2(instr.contains(seq), qPrintable(instr));
        QRegularExpression toc(QStringLiteral("^ TOC \\\\h \\\\z \\\\c \"(?:[^\"\\\\]|\\\\.)*\" $"), QRegularExpression::MultilineOption);
        QVERIFY2(instr.contains(toc), qPrintable(instr));
    }

    void everyLinkStaysALinkAsWritten()
    {
        const QStringList hrefs = {QStringLiteral("http://example.com/a"), QStringLiteral("mailto:a@b.c"), QStringLiteral("javascript:alert(1)"), QStringLiteral("file:///etc/passwd"),
                                   QStringLiteral("vbscript:x"), QStringLiteral("../other.odt")};
        Made m;
        m.body = "<text:p>";
        for (const QString &h : hrefs) m.body += "<text:a xlink:href=\"" + h.toUtf8() + "\">l</text:a>";
        m.body += "<draw:frame text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"../Pictures/a.png\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        Package p;
        QVERIFY(p.read(docx));
        QStringList targets;
        for (const Relationship &r : p.relationships(QStringLiteral("/word/document.xml")))
            if (r.external) targets << r.target;
        QStringList want = hrefs;
        want << QStringLiteral("../Pictures/a.png");   // (a picture named by a path that climbs out is a link, and is not read)
        targets.sort();
        want.sort();
        QCOMPARE(targets, want);
        QCOMPARE(part(docx, QStringLiteral("/word/document.xml")).count(QLatin1String("<w:hyperlink ")), hrefs.size());
        QVERIFY(part(docx, QStringLiteral("/word/document.xml")).contains(QLatin1String("r:link=")));
        for (const QString &name : p.partNames()) QVERIFY2(!name.contains(QLatin1String("/media/")), qPrintable(name));
        // And back: the .odt holds each as it was.
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(docx), files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        for (const QString &h : hrefs) QVERIFY2(c.contains(QLatin1String("xlink:href=\"") + h + QLatin1String("\"")), qPrintable(h));
    }

    void aListGoesOnFromTheListItNames()
    {
        Made m;
        m.autoStyles = "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\"/></text:list-style>";
        auto list = [](const char *id, const char *continues, const char *text) {
            return QByteArray("<text:list text:style-name=\"L1\" xml:id=\"") + id + "\"" + (*continues ? QByteArray(" text:continue-list=\"") + continues + "\"" : QByteArray()) + "><text:list-item><text:p>" + text + "</text:p></text:list-item></text:list>";
        };
        m.body = list("a", "", "a1") + list("b", "", "b1") + list("a2", "a", "a2") + list("b2", "b", "b2") + list("a3", "a2", "a3");
        const QString d = part(docxOf(m.build()), QStringLiteral("/word/document.xml"));
        QRegularExpression ids(QStringLiteral("<w:numId w:val=\"(\\d+)\"/>"));
        QStringList used;
        for (auto it = ids.globalMatch(d); it.hasNext();) used << it.next().captured(1);
        QCOMPARE(used.size(), 5);
        QVERIFY(used[0] != used[1]);          // two lists of one style, each its own count
        QCOMPARE(used[2], used[0]);           // a2 goes on from a
        QCOMPARE(used[3], used[1]);           // b2 goes on from b
        QCOMPARE(used[4], used[0]);           // a3 goes on from a2, which was a's
    }

    void valuesAreCheckedFromTheParsedXml()
    {
        // What a value is, read from the tree (not how the file spells it).
        Made m;
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:margin-left=\"1in\" fo:line-height=\"200%\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">x</text:p>";
        wml::XmlTree t;
        QVERIFY(t.parse(document(m).toUtf8(), nullptr));
        const int ind = findElement(t, "ind"), spacing = findElement(t, "spacing");
        QVERIFY(ind >= 0 && spacing >= 0);
        QCOMPARE(t.attr(ind, "left").toInt(), 1440);
        QCOMPARE(t.attr(spacing, "line").toInt(), 480);
        QCOMPARE(t.attr(spacing, "lineRule"), QStringLiteral("auto"));
    }

    void contextualSpacingGoesBothWays()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties style:contextual-spacing=\"true\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">x</text:p>";
        const QByteArray docx = docxOf(m.build());
        wml::XmlTree t;
        QVERIFY(t.parse(part(docx, QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
        QVERIFY(findElement(t, "contextualSpacing") >= 0);
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(docx), files));
        QVERIFY(QString::fromUtf8(files.value(QStringLiteral("content.xml"))).contains(QLatin1String("style:contextual-spacing=\"true\"")));
    }

    void aTextBoxKeepsItsPaddingFillAndLineBothWays()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"gr1\" style:family=\"graphic\"><style:graphic-properties draw:fill=\"solid\" draw:fill-color=\"#99ffcc\" draw:stroke=\"solid\" svg:stroke-color=\"#112233\" svg:stroke-width=\"0.02in\" "
                       "fo:padding-top=\"0.05in\" fo:padding-bottom=\"0.05in\" fo:padding-left=\"0.1in\" fo:padding-right=\"0.1in\"/></style:style>";
        m.body = "<text:p><draw:custom-shape text:anchor-type=\"char\" draw:style-name=\"gr1\" svg:width=\"2in\" svg:height=\"1in\"><text:p>boxed</text:p></draw:custom-shape></text:p>";
        const QByteArray docx = docxOf(m.build());
        wml::XmlTree t;
        QVERIFY(t.parse(part(docx, QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
        const int body = findElement(t, "bodyPr");
        QVERIFY(body >= 0);
        QCOMPARE(t.attr(body, "lIns").toInt(), 91440);
        QCOMPARE(t.attr(body, "tIns").toInt(), 45720);
        const int clr = findElement(t, "srgbClr");
        QVERIFY(clr >= 0);
        QCOMPARE(t.attr(clr, "val"), QStringLiteral("99FFCC"));
        // Back in an .odt: the frame has them again.
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(docx), files));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(wellFormed(files.value(QStringLiteral("styles.xml"))));
        wml::XmlTree c;
        QVERIFY(c.parse(files.value(QStringLiteral("content.xml")), nullptr));
        bool found = false;
        for (int i = findElement(c, "graphic-properties"); i >= 0 && !found; i = findElement(c, "graphic-properties", i))
            found = c.attrAny(i, "fill-color") == QLatin1String("#99FFCC") && c.attrAny(i, "padding-left") == QLatin1String("0.1000in") && c.attrAny(i, "stroke-color") == QLatin1String("#112233");
        QVERIFY(found);
    }

    void anOdtsLayoutSettingsGoBackAsTheyWere()
    {
        Made m;
        m.body = "<text:p>x</text:p>";
        m.extra.insert(QStringLiteral("settings.xml"),
                       "<?xml version=\"1.0\"?><office:document-settings xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:ooo=\"http://openoffice.org/2004/office\">"
                       "<office:settings><config:config-item-set config:name=\"ooo:view-settings\"><config:config-item config:name=\"ViewAreaTop\" config:type=\"long\">5</config:config-item></config:config-item-set>"
                       "<config:config-item-set config:name=\"ooo:configuration-settings\"><config:config-item config:name=\"AddExternalLeading\" config:type=\"boolean\">false</config:config-item>"
                       "<config:config-item config:name=\"PrinterName\" config:type=\"string\">Secret</config:config-item></config:config-item-set></office:settings></office:document-settings>");
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(docxOf(m.build())), files));
        const QString s = QString::fromUtf8(files.value(QStringLiteral("settings.xml")));
        QVERIFY(s.contains(QLatin1String("config:name=\"AddExternalLeading\" config:type=\"boolean\">false<")));
        QVERIFY(!s.contains(QLatin1String("PrinterName")));   // (nothing of the machine it was made on)
        QVERIFY(!s.contains(QLatin1String("ViewAreaTop")));
        // A document that did not come from an .odt gets the usual ones.
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<w:document ") + kW + "><w:body><w:p/></w:body></w:document>", QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        QVERIFY(jp::readZip(fromDocxPackage(w.finish()), files));
        QVERIFY(QString::fromUtf8(files.value(QStringLiteral("settings.xml"))).contains(QLatin1String("config:name=\"AddExternalLeading\" config:type=\"boolean\">true<")));
    }

    void aBoldParagraphMarkDoesNotMakeThePlainTextBold()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr><w:rPr><w:b/><w:sz w:val=\"20\"/></w:rPr></w:pPr><w:r><w:rPr><w:sz w:val=\"20\"/></w:rPr><w:t>plain words</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/word/numbering.xml"),
                  QByteArray("<w:numbering ") + kW + "><w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num></w:numbering>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"), QStringLiteral("/word/numbering.xml"));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(w.finish()), files));
        wml::XmlTree c;
        QVERIFY(c.parse(files.value(QStringLiteral("content.xml")), nullptr));
        bool sized = false;
        for (int i = findElement(c, "style"); i >= 0; i = findElement(c, "style", i)) {
            if (c.attrAny(i, "family") != QLatin1String("paragraph")) continue;   // (a text style named as the paragraph mark's keeps the mark's look)
            for (int k = c.firstChild(i); k >= 0; k = c.next(k)) {
                if (c.localName(k) != "text-properties") continue;
                QVERIFY(c.attrAny(k, "font-weight").isEmpty());   // (the mark is bold; the words are not)
                sized = sized || c.attrAny(k, "font-size") == QLatin1String("10pt");
            }
        }
        QVERIFY(sized);   // (the label keeps the mark's size)
    }

    void aListMarkSizeIsNotTheSizeOfRunsThatSayNone()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr><w:rPr><w:sz w:val=\"24\"/></w:rPr></w:pPr><w:r><w:rPr><w:sz w:val=\"24\"/></w:rPr><w:t>sized </w:t></w:r><w:hyperlink w:anchor=\"x\"><w:r><w:t>no size here</w:t></w:r></w:hyperlink></w:p></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/word/numbering.xml"),
                  QByteArray("<w:numbering ") + kW + "><w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num></w:numbering>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"), QStringLiteral("/word/numbering.xml"));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(w.finish()), files));
        wml::XmlTree c;
        QVERIFY(c.parse(files.value(QStringLiteral("content.xml")), nullptr));
        for (int i = findElement(c, "style"); i >= 0; i = findElement(c, "style", i)) {
            if (c.attrAny(i, "family") != QLatin1String("paragraph")) continue;
            for (int k = c.firstChild(i); k >= 0; k = c.next(k))
                if (c.localName(k) == "text-properties") QVERIFY(c.attrAny(k, "font-size").isEmpty());   // (the words have their style's size, not the mark's)
        }
    }

    void aFieldInDeletedTextStaysAField()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:del w:id=\"1\" w:author=\"A\"><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:delInstrText> PAGE </w:delInstrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                                                            "<w:r><w:delText>2</w:delText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:del><w:r><w:t>kept</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        QMap<QString, QByteArray> files;
        const QByteArray odt = fromDocxPackage(w.finish());
        QVERIFY(jp::readZip(odt, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        QVERIFY2(c.contains(QLatin1String("<text:deletion>")) && c.contains(QRegularExpression(QStringLiteral("<text:deletion>.*<text:page-number[^>]*>2</text:page-number>.*</text:deletion>"))), qPrintable(c));
        // And read again, the deleted field keeps its code (w:delInstrText).
        QVERIFY(part(docxOf(odt), QStringLiteral("/word/document.xml")).contains(QLatin1String("<w:delInstrText")));
    }

    void anEmptyParagraphWithNoListInAListStyleIsNotNumbered()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/><style:style style:name=\"Numbered\" style:family=\"paragraph\" style:list-style-name=\"L1\"/>";
        m.autoStyles = "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\"/></text:list-style>"
                       "<style:style style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"Numbered\" style:list-style-name=\"\"/>";
        m.body = "<text:p text:style-name=\"P1\">plain</text:p>";
        const QString d = document(m);
        wml::XmlTree t;
        QVERIFY(t.parse(d.toUtf8(), nullptr));
        const int numId = findElement(t, "numId");
        QVERIFY(numId >= 0);
        QCOMPARE(t.val(numId), QStringLiteral("0"));
    }

    void dynamicSpacingIsInsideTheFooter()
    {
        Made m;
        m.layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" fo:margin-top=\"1in\" fo:margin-bottom=\"0.5in\" fo:margin-left=\"1in\" fo:margin-right=\"1in\"/>"
                    "<style:footer-style><style:header-footer-properties fo:min-height=\"0.5in\" fo:margin-top=\"0.46in\" style:dynamic-spacing=\"true\"/></style:footer-style></style:page-layout>";
        m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"><style:footer><text:p>f</text:p></style:footer></style:master-page>";
        m.body = "<text:p>x</text:p>";
        wml::XmlTree t;
        QVERIFY(t.parse(document(m).toUtf8(), nullptr));
        const int mar = findElement(t, "pgMar");
        QVERIFY(mar >= 0);
        QCOMPARE(t.attr(mar, "bottom").toInt(), 1440);   // 0.5 in and the footer's 0.5 in; its spacing is inside it
    }

    void aFieldIsWrittenOnceWithItsResult()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:r><w:t xml:space=\"preserve\">before </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                                                            "<w:r><w:t>7</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:r><w:t xml:space=\"preserve\"> after</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(w.finish()), files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        {
            // (The paragraph reads as it was: the field's result is once, where the field is.)
            wml::XmlTree t;
            QVERIFY(t.parse(files.value(QStringLiteral("content.xml")), nullptr));
            const int p = findElement(t, "p");
            QVERIFY(p >= 0);
            QCOMPARE(t.text(p), QStringLiteral("before 7 after"));
        }
        QCOMPARE(c.count(QLatin1String("before")), 1);
        QCOMPARE(c.count(QLatin1String(">7<")), 1);
        QCOMPARE(c.count(QLatin1String(" after")), 1);
        QVERIFY(c.indexOf(QLatin1String("before")) < c.indexOf(QLatin1String("<text:page-number")));
        QVERIFY(c.indexOf(QLatin1String("<text:page-number")) < c.indexOf(QLatin1String(" after")));
    }

    void aCellInNestedContentControlsKeepsItsText()
    {
        const QString c = odtContentOf(
            "<w:tbl><w:tblGrid><w:gridCol w:w=\"4000\"/><w:gridCol w:w=\"4000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>Note</w:t></w:r></w:p></w:tc></w:tr>"
            "<w:sdt><w:sdtContent><w:sdt><w:sdtContent><w:tr><w:sdt><w:sdtContent><w:tc><w:p><w:r><w:t>Choose an item.</w:t></w:r></w:p></w:tc></w:sdtContent></w:sdt>"
            "<w:tc><w:p><w:r><w:t>Here is just a sample</w:t></w:r></w:p></w:tc></w:tr></w:sdtContent></w:sdt></w:sdtContent></w:sdt></w:tbl>");
        QVERIFY2(c.contains(QLatin1String("Choose an item.")), qPrintable(c));
        QVERIFY2(c.contains(QLatin1String("Here is just a sample")), qPrintable(c));
        QCOMPARE(c.count(QLatin1String("<table:table-row")), 2);
    }

    void aSymbolKeepsItsPictureOrItsFont()
    {
        // A known symbol font's character is written as the picture it stands for, not as the letter its code is.
        const QChar box = jp::symbolPictureOf(QStringLiteral("Wingdings"), 0xF0FE);
        QVERIFY(!box.isNull());
        const QString c = odtContentOf("<w:p><w:r><w:sym w:font=\"Wingdings\" w:char=\"F0FE\"/></w:r></w:p>");
        QVERIFY2(c.contains(box), qPrintable(c));
        QVERIFY(!c.contains(QChar(0xFE)));
        // Another font's character keeps its code and its font.
        const QString d = odtContentOf("<w:p><w:r><w:sym w:font=\"Pictographic Sans\" w:char=\"F041\"/></w:r></w:p>");
        QVERIFY2(d.contains(QChar(0xF041)), qPrintable(d));
        QVERIFY2(d.contains(QLatin1String("Pictographic Sans")), qPrintable(d));
    }

    void trackedFormatAndParagraphChangesAreKept()
    {
        QStringList notes;
        const QString c = odtContentOf(
            "<w:p><w:pPr><w:rPr><w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2026-01-02T03:04:05Z\"/></w:rPr><w:pPrChange w:id=\"2\" w:author=\"Bob\" w:date=\"2026-01-02T03:04:05Z\"><w:pPr/></w:pPrChange></w:pPr>"
            "<w:r><w:rPr><w:b/><w:rPrChange w:id=\"3\" w:author=\"Cy\" w:date=\"2026-01-02T03:04:05Z\"><w:rPr/></w:rPrChange></w:rPr><w:t>bold words</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:rPr><w:del w:id=\"4\" w:author=\"Dee\" w:date=\"2026-01-02T03:04:05Z\"/></w:rPr></w:pPr><w:r><w:t>second</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>third</w:t></w:r></w:p>",
            &notes);
        QCOMPARE(c.count(QLatin1String("<text:format-change>")), 2);
        QCOMPARE(c.count(QLatin1String("<text:insertion>")), 1);
        QCOMPARE(c.count(QLatin1String("<text:deletion>")), 1);
        // Every change begins and ends once, and the ends come after the beginnings.
        QStringList starts, ends;
        static const QRegularExpression rs(QStringLiteral("<text:change-start text:change-id=\"(\\w+)\"/>")), re(QStringLiteral("<text:change-end text:change-id=\"(\\w+)\"/>"));
        for (auto it = rs.globalMatch(c); it.hasNext();) starts << it.next().captured(1);
        for (auto it = re.globalMatch(c); it.hasNext();) ends << it.next().captured(1);
        QCOMPARE(starts.size(), 4);
        starts.sort();
        ends.sort();
        QCOMPARE(starts, ends);
        for (const QString &id : std::as_const(starts)) QVERIFY(c.indexOf(QStringLiteral("<text:change-start text:change-id=\"%1\"/>").arg(id)) < c.indexOf(QStringLiteral("<text:change-end text:change-id=\"%1\"/>").arg(id)));
        QVERIFY2(c.contains(QLatin1String("Bob")) && c.contains(QLatin1String("Cy")) && c.contains(QLatin1String("Dee")), qPrintable(c));
        // (Nothing that .odt can hold is said to be left out.)
        QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char(';'))));
    }

    void aParagraphMarkChangeOnTheLastParagraphHasBothEnds()
    {
        const QString c = odtContentOf("<w:p><w:r><w:t>only</w:t></w:r></w:p><w:p><w:pPr><w:rPr><w:ins w:id=\"1\" w:author=\"Ann\"/></w:rPr></w:pPr><w:r><w:t>last</w:t></w:r></w:p>");
        QCOMPARE(c.count(QLatin1String("<text:change-start")), 1);
        QCOMPARE(c.count(QLatin1String("<text:change-end")), 1);
    }

    void changesToTablesAreSaidToBeLeftAsTheyStand()
    {
        QStringList notes;
        odtContentOf("<w:tbl><w:tblPr><w:tblPrChange w:id=\"1\" w:author=\"A\"><w:tblPr/></w:tblPrChange></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>x</w:t></w:r></w:p></w:tc></w:tr></w:tbl>", &notes);
        QVERIFY2(notes.join(QLatin1Char(';')).contains(QLatin1String("tracked changes to tables")), qPrintable(notes.join(QLatin1Char(';'))));
    }

    void formFieldsKeepTheirBookmarksAndTheirShownValue()
    {
        QStringList notes;
        const QString box = QStringLiteral(
            "<w:p><w:r><w:t>unchecked: </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"><w:ffData><w:name w:val=\"Check1\"/><w:checkBox><w:default w:val=\"0\"/></w:checkBox></w:ffData></w:fldChar></w:r>"
            "<w:bookmarkStart w:id=\"0\" w:name=\"Check1\"/><w:r><w:instrText> FORMCHECKBOX </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:bookmarkEnd w:id=\"0\"/>"
            "<w:r><w:t> checked: </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"><w:ffData><w:name w:val=\"Check2\"/><w:checkBox><w:default w:val=\"1\"/></w:checkBox></w:ffData></w:fldChar></w:r>"
            "<w:bookmarkStart w:id=\"1\" w:name=\"Check2\"/><w:r><w:instrText> FORMCHECKBOX </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:bookmarkEnd w:id=\"1\"/>"
            "<w:r><w:t> text: </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"><w:ffData><w:name w:val=\"Text1\"/><w:textInput><w:default w:val=\"Your name\"/></w:textInput></w:ffData></w:fldChar></w:r>"
            "<w:r><w:instrText> FORMTEXT </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>");
        const QString c = odtContentOf(box.toUtf8(), &notes);
        QCOMPARE(c.count(QLatin1String("<text:bookmark-start")), 2);
        QCOMPARE(c.count(QLatin1String("<text:bookmark-end")), 2);
        QVERIFY2(c.contains(QChar(0x2610)) && c.contains(QChar(0x2611)), qPrintable(c));
        QVERIFY2(c.contains(QLatin1String("Your name")), qPrintable(c));
        QVERIFY2(notes.join(QLatin1Char(';')).contains(QLatin1String("form fields")), qPrintable(notes.join(QLatin1Char(';'))));
        // And it reads back with the same text.
        QVERIFY(c.indexOf(QLatin1String("<text:bookmark-start")) < c.indexOf(QLatin1String("<text:bookmark-end")));
    }

    void aMergeFieldKeepsItsNameAndItsText()
    {
        const QString c = odtContentOf("<w:p><w:fldSimple w:instr=\" MERGEFIELD FirstName \\* MERGEFORMAT \"><w:r><w:t>&#171;FirstName&#187;</w:t></w:r></w:fldSimple>"
                                       "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> MERGEFIELD \"Last Name\" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>Smith</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>");
        QVERIFY2(c.contains(QRegularExpression(QStringLiteral("<text:database-display[^>]*text:column-name=\"FirstName\"[^>]*>\u00ABFirstName\u00BB</text:database-display>"))), qPrintable(c));
        QVERIFY2(c.contains(QRegularExpression(QStringLiteral("<text:database-display[^>]*text:column-name=\"Last Name\"[^>]*>Smith</text:database-display>"))), qPrintable(c));
    }

    // Shapes of the older drawing form (a rectangle, an oval, a line, a text box) are the standard's drawing shapes, with place, size, fill, line, wrap, and the words in a text box.
    void shapesOfTheOlderFormAreTheStandardsDrawingShapes()
    {
        const QMap<QString, QByteArray> files = filesOf(odtOfDrawing(kOlderShapes));
        QVERIFY(!files.isEmpty());
        const QString c = standardContentOf(files);
        QVERIFY(wellFormed(c.toUtf8()));
        wml::XmlTree t;
        QVERIFY(t.parse(c.toUtf8(), nullptr));
        const int rect = findElement(t, "rect");
        QVERIFY2(rect >= 0, qPrintable(c));
        QCOMPARE(t.attrAny(rect, "anchor-type"), QStringLiteral("paragraph"));
        QVERIFY(qAbs(inchesOf(t.attrAny(rect, "width")) - 2.0) < 0.001);
        QVERIFY(qAbs(inchesOf(t.attrAny(rect, "height")) - 1.0) < 0.001);
        QVERIFY(qAbs(inchesOf(t.attrAny(rect, "x")) - 1.0) < 0.001);
        QVERIFY(qAbs(inchesOf(t.attrAny(rect, "y")) - 0.5) < 0.001);
        const QMap<QString, QString> rp = graphicPropertiesOf(t, t.attrAny(rect, "style-name"));
        QCOMPARE(rp.value(QStringLiteral("fill")), QStringLiteral("solid"));
        QCOMPARE(rp.value(QStringLiteral("fill-color")).toLower(), QStringLiteral("#ff0000"));
        QCOMPARE(rp.value(QStringLiteral("stroke")), QStringLiteral("solid"));
        QCOMPARE(rp.value(QStringLiteral("stroke-color")).toLower(), QStringLiteral("#0000ff"));
        QVERIFY(qAbs(inchesOf(rp.value(QStringLiteral("stroke-width"))) - 2.0 / 72.0) < 0.001);
        QCOMPARE(rp.value(QStringLiteral("wrap")), QStringLiteral("parallel"));
        const int oval = findElement(t, "ellipse");
        QVERIFY2(oval >= 0, qPrintable(c));
        QVERIFY(qAbs(inchesOf(t.attrAny(oval, "width")) - 50.0 / 72.0) < 0.001);
        const QMap<QString, QString> op = graphicPropertiesOf(t, t.attrAny(oval, "style-name"));
        QCOMPARE(op.value(QStringLiteral("fill")), QStringLiteral("none"));
        QCOMPARE(op.value(QStringLiteral("stroke")), QStringLiteral("none"));
        QCOMPARE(op.value(QStringLiteral("run-through")), QStringLiteral("background"));
        const int line = findElement(t, "line");
        QVERIFY2(line >= 0, qPrintable(c));
        QVERIFY(qAbs(inchesOf(t.attrAny(line, "x1")) - 100.0 / 72.0) < 0.001);
        QVERIFY(qAbs(inchesOf(t.attrAny(line, "y1")) - 200.0 / 72.0) < 0.001);
        QVERIFY(qAbs(inchesOf(t.attrAny(line, "x2")) - 250.0 / 72.0) < 0.001);
        QVERIFY(qAbs(inchesOf(t.attrAny(line, "y2")) - 200.0 / 72.0) < 0.001);
        const int box = findElement(t, "text-box");
        QVERIFY2(box >= 0, qPrintable(c));
        QCOMPARE(t.localName(t.parent(box)), QByteArray("frame"));
        QCOMPARE(t.text(box).trimmed(), QStringLiteral("Boxed words"));
        const QMap<QString, QString> bp = graphicPropertiesOf(t, t.attrAny(t.parent(box), "style-name"));
        QVERIFY(qAbs(inchesOf(bp.value(QStringLiteral("padding-left"))) - 0.1) < 0.001);
    }

    // And they come back as drawings of the same kinds, places, and sizes (the same count of drawn objects).
    void shapesOfTheOlderFormComeBackAsTheSameDrawings()
    {
        const QString doc = part(docxOf(odtOfDrawing(kOlderShapes)), QStringLiteral("/word/document.xml"));
        wml::XmlTree t;
        QVERIFY(t.parse(doc.toUtf8(), nullptr));
        int drawings = 0;
        for (int i = findElement(t, "drawing"); i >= 0; i = findElement(t, "drawing", i)) ++drawings;
        QCOMPARE(drawings, 4);
        QStringList presets;
        for (int i = findElement(t, "prstGeom"); i >= 0; i = findElement(t, "prstGeom", i)) presets << t.attr(i, "prst");
        presets.sort();
        QCOMPARE(presets, (QStringList{QStringLiteral("ellipse"), QStringLiteral("line"), QStringLiteral("rect"), QStringLiteral("rect")}));
        bool sized = false;
        for (int i = findElement(t, "extent"); i >= 0; i = findElement(t, "extent", i)) sized = sized || (t.attr(i, "cx").toLongLong() == 1828800 && t.attr(i, "cy").toLongLong() == 914400);
        QVERIFY(sized);
        const int box = findElement(t, "txbxContent");
        QVERIFY(box >= 0);
        QCOMPARE(t.text(box).trimmed(), QStringLiteral("Boxed words"));
        bool placed = false;
        for (int i = findElement(t, "posOffset"); i >= 0; i = findElement(t, "posOffset", i)) placed = placed || (t.localName(t.parent(i)) == "positionH" && t.text(i).toLongLong() == 914400);
        QVERIFY(placed);
    }

    // A picture that the file links to and does not hold is the standard's image with the address as its reference; it comes back as a link, and nothing is opened or fetched on the way.
    void aLinkedPictureIsAnImageThatNamesItsAddress()
    {
        const QString address = QStringLiteral("http://127.0.0.1:9/pictures/photo%201.png");
        const QByteArray odt = odtOfDrawing(linkedPictureBody(), address);
        const QMap<QString, QByteArray> files = filesOf(odt);
        QVERIFY(!files.isEmpty());
        const QString c = standardContentOf(files);
        QVERIFY2(c.contains(QLatin1String("<draw:image xlink:href=\"") + address + QLatin1String("\"")), qPrintable(c));
        for (auto it = files.cbegin(); it != files.cend(); ++it) QVERIFY2(!it.key().startsWith(QLatin1String("Pictures/")), qPrintable(it.key()));
        const QByteArray docx = docxOf(odt);
        Package p;
        QVERIFY(p.read(docx));
        const QString doc = QString::fromUtf8(p.part(QStringLiteral("/word/document.xml")));
        QVERIFY2(doc.contains(QRegularExpression(QStringLiteral("<a:blip [^>]*r:link=\"rId\\d+\""))), qPrintable(doc));
        QVERIFY2(!doc.contains(QLatin1String("r:embed")), qPrintable(doc));
        int external = 0;
        for (const Relationship &r : p.relationships(QStringLiteral("/word/document.xml")))
            if (r.external && r.target == address) ++external;
        QCOMPARE(external, 1);
        for (const QString &name : p.partNames()) QVERIFY2(!name.contains(QLatin1String("/media/")), qPrintable(name));
    }

    // An address that is not an address (control characters, a length no address has) is not made a link; each address is one relationship however many pictures name it; and links are bounded.
    void aHostileLinkedPictureAddressIsNotFollowedOrKept()
    {
        auto image = [](const QString &href) { return QString(QStringLiteral("<text:p><draw:frame draw:name=\"p\" text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"%1\"/></draw:frame></text:p>").arg(href)).toUtf8(); };
        auto linksOf = [](const Made &m) {
            Package p;
            if (!p.read(docxOf(m.build()))) return qsizetype(-1);
            qsizetype n = 0;
            for (const Relationship &r : p.relationships(QStringLiteral("/word/document.xml")))
                if (r.external) ++n;
            return n;
        };
        Made bad;
        bad.body = image(QStringLiteral("http://a/%1").arg(QString(5000, QLatin1Char('x')))) + image(QStringLiteral("http://a/b&#10;c")) + image(QStringLiteral("http://a/b&#1;c"));
        QCOMPARE(linksOf(bad), qsizetype(0));
        Made same;
        for (int i = 0; i < 50; ++i) same.body += image(QStringLiteral("http://example.invalid/one.png"));
        QCOMPARE(linksOf(same), qsizetype(1));
        Made many;
        for (int i = 0; i < 6000; ++i) many.body += image(QStringLiteral("http://example.invalid/%1.png").arg(i));
        QCOMPARE(linksOf(many), qsizetype(5000));
    }

    // A drawn line or shape whose numbers are absurd reads with a bounded size, and a document holds a bounded number of drawn shapes.
    void hostileShapeNumbersAreBoundedAndShapesAreCounted()
    {
        Made m;
        m.body = "<text:p><draw:line text:anchor-type=\"paragraph\" svg:x1=\"0in\" svg:y1=\"0in\" svg:x2=\"1e300in\" svg:y2=\"-99999999in\"/>"
                 "<draw:line text:anchor-type=\"paragraph\" svg:x1=\"abc\" svg:y1=\"nan\" svg:x2=\"1in\" svg:y2=\"1in\"/>"
                 "<draw:rect text:anchor-type=\"paragraph\" svg:x=\"-1e30in\" svg:y=\"1e30in\" svg:width=\"1e40in\" svg:height=\"-5in\"/></text:p>";
        const QString doc = part(docxOf(m.build()), QStringLiteral("/word/document.xml"));
        wml::XmlTree t;
        QVERIFY(t.parse(doc.toUtf8(), nullptr));
        int drawings = 0;
        for (int i = findElement(t, "extent"); i >= 0; i = findElement(t, "extent", i)) {
            ++drawings;
            QVERIFY(t.attr(i, "cx").toLongLong() <= qint64(12700) * 3000);
            QVERIFY(t.attr(i, "cy").toLongLong() <= qint64(12700) * 3000);
            QVERIFY(t.attr(i, "cx").toLongLong() > 0 && t.attr(i, "cy").toLongLong() > 0);
        }
        QVERIFY(drawings >= 2);
        for (int i = findElement(t, "posOffset"); i >= 0; i = findElement(t, "posOffset", i)) QVERIFY(qAbs(t.text(i).toLongLong()) <= qint64(12700) * 3000 * 40);
        Made many;
        for (int i = 0; i < 21000; ++i) many.body += "<text:p><draw:rect text:anchor-type=\"paragraph\" svg:width=\"1in\" svg:height=\"1in\"/></text:p>";
        QStringList notes;
        const QString big = part(docxOf(many.build(), &notes), QStringLiteral("/word/document.xml"));
        QCOMPARE(big.count(QLatin1String("<w:drawing>")), 20000);
    }

    // A picture of the older Macintosh form (.pct) is kept as the picture it is: it is not drawn here, but it is one of the document's drawn objects and goes back out as it came.
    void aPictPictureIsKeptAsAPicture()
    {
        Made m;
        m.body = "<text:p><draw:frame text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/a.pct\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.pct"), QByteArray("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16));
        const QByteArray docx = docxOf(m.build());
        QCOMPARE(part(docx, QStringLiteral("/word/document.xml")).count(QLatin1String("<w:drawing>")), 1);
        Package p;
        QVERIFY(p.read(docx));
        bool kept = false;
        for (const QString &name : p.partNames()) kept = kept || name.endsWith(QLatin1String(".pct"));
        QVERIFY(kept);
    }

    // A text form field that shows only blank space (the five en spaces a blank one is made of) keeps them; one with a default text and a blank result shows the default.
    void aTextFormFieldKeepsItsBlankResult()
    {
        // (and the runs of a result that shows text keep their font and size)
        const QString shown = odtStylesAndContentOf("<w:p><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> FORMTEXT </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                                                    "<w:r><w:rPr><w:rFonts w:ascii=\"Arial\"/><w:sz w:val=\"16\"/></w:rPr><w:t>Small words</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>");
        QVERIFY2(shown.contains(QRegularExpression(QStringLiteral("<text:span text:style-name=\"T\\d+\">Small words</text:span>"))), qPrintable(shown));
        QVERIFY2(shown.contains(QLatin1String("fo:font-size=\"8pt\"")), qPrintable(shown));
        const QString blank(5, QChar(0x2002));
        auto field = [&](const QString &defaultText, const QString &result) {
            return QStringLiteral("<w:p><w:r><w:t>Name:</w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"><w:ffData><w:name w:val=\"T\"/><w:textInput>") +
                   (defaultText.isEmpty() ? QString() : QStringLiteral("<w:default w:val=\"%1\"/>").arg(defaultText)) +
                   QStringLiteral("</w:textInput></w:ffData></w:fldChar></w:r><w:r><w:instrText> FORMTEXT </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>%1</w:t></w:r>"
                                  "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>").arg(result);
        };
        QVERIFY2(odtContentOf(field(QString(), blank).toUtf8()).contains(QStringLiteral("Name:") + blank + QLatin1String("</text:p>")), "blank result lost");
        QVERIFY2(odtContentOf(field(QStringLiteral("Your name"), blank).toUtf8()).contains(QStringLiteral("Name:Your name</text:p>")), "default not shown");
        QVERIFY2(odtContentOf(field(QStringLiteral("Your name"), QStringLiteral("Sam")).toUtf8()).contains(QStringLiteral("Name:Sam</text:p>")), "result not shown");
    }

    // The blank that follows the number of a note is the note's, not the text's: a note whose text begins with it keeps the one blank, not two, through the .odt and back.
    void aNoteKeepsTheOneBlankAfterItsNumber()
    {
        // (and one whose number is followed by a tab has the tab, not a blank before it)
        Made tabbed;
        tabbed.body = "<text:p>a<text:note text:id=\"ftn1\" text:note-class=\"footnote\"><text:note-citation>1</text:note-citation><text:note-body><text:p><text:tab/>tabbed</text:p></text:note-body></text:note></text:p>";
        const QString footnote = part(docxOf(tabbed.build()), QStringLiteral("/word/footnotes.xml"));
        QVERIFY2(footnote.contains(QLatin1String("<w:footnoteRef/></w:r><w:r><w:tab/></w:r>")), qPrintable(footnote));
        PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        w.addPart(QStringLiteral("/word/document.xml"), "<w:document " + ns + "><w:body><w:p><w:r><w:t>text</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p></w:body></w:document>", QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/footnotes.xml"), "<w:footnotes " + ns + "><w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> note text</w:t></w:r></w:p></w:footnote></w:footnotes>", QStringLiteral("application/xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdN"), rel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
        const QByteArray odt = fromDocxPackage(w.finish());
        QVERIFY(!odt.isEmpty());
        const QString c = QString::fromUtf8(filesOf(odt).value(QStringLiteral("content.xml")));
        QVERIFY2(c.contains(QRegularExpression(QStringLiteral("<text:note-body><text:p[^>]*>note text</text:p>"))), qPrintable(c));
        wml::XmlTree t;
        QVERIFY(t.parse(part(docxOf(odt), QStringLiteral("/word/footnotes.xml")).toUtf8(), nullptr));
        QString text;
        for (int i = findElement(t, "t"); i >= 0; i = findElement(t, "t", i)) text += t.text(i);
        QCOMPARE(text, QStringLiteral(" note text"));
    }

    // A row with no cell for some places of the grid (a row that starts late or ends early) is a rectangle of cells in the file, as the format needs, and comes back as the row it was: no cell, no paragraph is made for a place.
    void aRowThatDoesNotFillTheGridDoesNotGainParagraphs()
    {
        auto cell = [](const char *text) { return QByteArray("<w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>") + text + "</w:t></w:r></w:p></w:tc>"; };
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/></w:tblGrid>"
                                 "<w:tr>" + cell("A") + cell("B") + cell("C") + "</w:tr>"
                                 "<w:tr><w:trPr><w:gridBefore w:val=\"1\"/></w:trPr>" + cell("D") + cell("E") + "</w:tr>"
                                 "<w:tr>" + cell("F") + "</w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
        const QString c = odtContentOf(table);
        wml::XmlTree t;
        QVERIFY(t.parse(QString(c).remove(QRegularExpression(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\""))).toUtf8(), nullptr));
        int rows = 0;
        for (int r = findElement(t, "table-row"); r >= 0; r = findElement(t, "table-row", r)) {
            ++rows;
            int cells = 0;
            for (int k = t.firstChild(r); k >= 0; k = t.next(k))
                if (t.localName(k) == "table-cell" || t.localName(k) == "covered-table-cell") ++cells;
            QCOMPARE(cells, 3);
        }
        QCOMPARE(rows, 3);
        const QStringList paragraphs = paragraphsOf(roundTripBody(table));
        QCOMPARE(paragraphs, (QStringList{QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D"), QStringLiteral("E"), QStringLiteral("F"), QStringLiteral("after")}));
    }

    // Two tables with no paragraph between them are two tables in the file, and come back as they were, with no paragraph made between; with our attribute ignored they are still two tables.
    void twoTablesWithNothingBetweenThemStayAdjacent()
    {
        const QByteArray one = "<w:tbl><w:tblPr/><w:tblGrid><w:gridCol w:w=\"1000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>one</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        const QByteArray two = "<w:tbl><w:tblPr/><w:tblGrid><w:gridCol w:w=\"1000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>two</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        const QString back = roundTripBody(one + two + "<w:p><w:r><w:t>end</w:t></w:r></w:p>");
        QVERIFY2(back.contains(QLatin1String("</w:tbl><w:tbl>")), qPrintable(back));
        QCOMPARE(paragraphsOf(back), (QStringList{QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("end")}));
        QString c = odtContentOf(one + two + "<w:p><w:r><w:t>end</w:t></w:r></w:p>");
        c.remove(QRegularExpression(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\"")));
        QCOMPARE(c.count(QLatin1String("<table:table ")), 2);
        QVERIFY2(c.contains(QLatin1String("</table:table><table:table ")), qPrintable(c));
        // A table after a paragraph says nothing by the attribute.
        Made m;
        m.body = "<text:p>a</text:p><table:table jo:adjacent=\"true\"><table:table-column/><table:table-row><table:table-cell><text:p>x</text:p></table:table-cell></table:table-row></table:table>";
        const QString read = document(m);
        QVERIFY2(!read.contains(QLatin1String("<w:p/><w:tbl>")), qPrintable(read));
    }

    // A check box is its sign for a program that knows nothing of ours; ours reads it as a check box again (a field, not text), checked or not.
    void aCheckBoxComesBackAsACheckBox()
    {
        auto box = [](const char *on) {
            return QByteArray("<w:r><w:fldChar w:fldCharType=\"begin\"><w:ffData><w:checkBox><w:default w:val=\"") + on + "\"/></w:checkBox></w:ffData></w:fldChar></w:r><w:r><w:instrText> FORMCHECKBOX </w:instrText></w:r>"
                   "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        };
        const QByteArray body = "<w:p><w:r><w:t>a </w:t></w:r>" + box("0") + "<w:r><w:t> b </w:t></w:r>" + box("1") + "</w:p>";
        QString c = odtContentOf(body);
        QVERIFY2(c.contains(QLatin1String("jo:checkbox=\"checked\"")) && c.contains(QLatin1String("jo:checkbox=\"unchecked\"")), qPrintable(c));
        c.remove(QRegularExpression(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\"")));
        QVERIFY2(c.contains(QString(QChar(0x2610))) && c.contains(QString(QChar(0x2611))), qPrintable(c));
        const QString back = roundTripBody(body);
        QCOMPARE(back.count(QLatin1String("FORMCHECKBOX")), 2);
        QVERIFY2(back.contains(QLatin1String("<w:default w:val=\"1\"/>")) && back.contains(QLatin1String("<w:default w:val=\"0\"/>")), qPrintable(back));
        QCOMPARE(paragraphsOf(back), (QStringList{QStringLiteral("a  b ")}));
    }

    // A blank that ends a paragraph is still there after the .odt (the format drops a plain one at the end), also when a span or a link closes after it.
    void aBlankAtTheEndOfAParagraphIsKept()
    {
        const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">None </w:t></w:r></w:p><w:p><w:r><w:rPr><w:b/></w:rPr><w:t xml:space=\"preserve\">Bold </w:t></w:r></w:p>";
        QCOMPARE(paragraphsOf(roundTripBody(body)), (QStringList{QStringLiteral("None "), QStringLiteral("Bold ")}));
        QString c = odtContentOf(body);
        c.remove(QRegularExpression(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\"")));
        QVERIFY2(c.contains(QLatin1String("None<text:s/></text:p>")), qPrintable(c));
    }

    // A line break that clears the wrapping of a drawing beside it is a line break for any program, and ours reads it as the break that clears (a break of another kind than the plain one).
    void aLineBreakThatClearsKeepsWhatItClears()
    {
        const QByteArray body = "<w:p><w:r><w:t>a</w:t></w:r><w:r><w:br w:type=\"textWrapping\" w:clear=\"all\"/></w:r><w:r><w:t>b</w:t></w:r><w:r><w:br/></w:r><w:r><w:t>c</w:t></w:r></w:p>";
        QString c = odtContentOf(body);
        QVERIFY2(c.contains(QLatin1String("<text:line-break jo:clear=\"all\"/>")), qPrintable(c));
        c.remove(QRegularExpression(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\"")));
        QCOMPARE(c.count(QLatin1String("<text:line-break/>")), 2);
        const QString back = roundTripBody(body);
        QVERIFY2(back.contains(QLatin1String("<w:br w:clear=\"all\"/>")), qPrintable(back));
        QCOMPARE(back.count(QLatin1String("<w:br")), 2);
    }

    // A column break inside a paragraph is a line break for a program that knows nothing of ours (the words do not run together), and ours reads it back as the column break; a file of an earlier version said it with a span, and is read the same.
    void aColumnBreakInAParagraphIsKept()
    {
        const QByteArray body = "<w:p><w:r><w:t>before</w:t></w:r><w:r><w:br w:type=\"column\"/><w:t>after</w:t></w:r></w:p>";
        QString c = odtContentOf(body);
        QVERIFY2(c.contains(QLatin1String("jo:column-break=\"true\"")), qPrintable(c));
        c.remove(QRegularExpression(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\"")));
        QVERIFY2(c.contains(QLatin1String("before<text:line-break/>after</text:p>")), qPrintable(c));
        QVERIFY2(!c.contains(QLatin1String("<text:span/>")), qPrintable(c));
        const QString back = roundTripBody(body);
        QCOMPARE(back.count(QLatin1String("<w:br w:type=\"column\"/>")), 1);
        QCOMPARE(back.count(QLatin1String("<w:br")), 1);
        QCOMPARE(paragraphsOf(back), (QStringList{QStringLiteral("beforeafter")}));
        Made old;
        old.body = "<text:p xmlns:jo=\"urn:jeffoffice:odf-extension\">before<text:span jo:column-break=\"true\"/>after</text:p>";
        QCOMPARE(document(old).count(QLatin1String("<w:br w:type=\"column\"/>")), 1);
    }


    // Line spacing of "automatic" kind is a share of the line, not rounded to a whole percent: a spacing of 227 or 272 comes back as 227 or 272, not 228 or 271.
    void anAutomaticLineSpacingIsNotRoundedToAWholePercent()
    {
        const QString body = QStringLiteral("<w:p><w:pPr><w:spacing w:line=\"227\" w:lineRule=\"auto\"/></w:pPr><w:r><w:t>a</w:t></w:r></w:p><w:p><w:pPr><w:spacing w:line=\"272\" w:lineRule=\"auto\"/></w:pPr><w:r><w:t>b</w:t></w:r></w:p>"
                                            "<w:p><w:pPr><w:spacing w:line=\"360\" w:lineRule=\"auto\"/></w:pPr><w:r><w:t>c</w:t></w:r></w:p>");
        const QString c = odtStylesAndContentOf(body.toUtf8());
        QVERIFY2(c.contains(QLatin1String("fo:line-height=\"150%\"")), qPrintable(c));
        const QString back = roundTripBody(body.toUtf8());
        QVERIFY2(back.contains(QLatin1String("w:line=\"227\"")) && back.contains(QLatin1String("w:line=\"272\"")) && back.contains(QLatin1String("w:line=\"360\"")), qPrintable(back));
    }

    // A page's gutter (the room for the binding) is where the document puts it: at the left, at the right for a right-to-left section or when the document says so, at the top for a gutter there, and in the inside margin of mirrored pages; the file says it as margins (the standard has no gutter), and ours keeps the gutter apart from the margin, so that both come back.
    void aPageGutterIsWhereTheDocumentPutsIt()
    {
        auto body = [](const char *sectExtra) {
            return QByteArray("<w:p><w:r><w:t>x</w:t></w:r></w:p><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"432\"/>") +
                   sectExtra + "</w:sectPr>";
        };
        struct Case {
            const char *name, *sect, *settings;
            double left, right, top;
            const char *usage;
            bool rtlGutter, atTop, mirrored;
        };
        const Case cases[] = {
            {"left", "", "", 1.3, 1.0, 1.0, "", false, false, false},
            {"right: the section reads from the right", "<w:bidi/>", "", 1.0, 1.3, 1.0, "", true, false, false},
            {"right: the document says so", "<w:rtlGutter/>", "", 1.0, 1.3, 1.0, "", true, false, false},
            {"top", "", "<w:gutterAtTop/>", 1.0, 1.0, 1.3, "", false, true, false},
            {"mirrored", "", "<w:mirrorMargins/>", 1.3, 1.0, 1.0, "mirrored", false, false, true},
            {"mirrored, at the top", "", "<w:mirrorMargins/><w:gutterAtTop/>", 1.0, 1.0, 1.3, "mirrored", false, true, true},
        };
        for (const Case &k : cases) {
            const QByteArray odt = odtOfBodyWithSettings(body(k.sect), k.settings);
            const QMap<QString, QByteArray> files = filesOf(odt);
            QVERIFY2(!files.isEmpty(), k.name);
            wml::XmlTree t;
            QVERIFY(t.parse(standardPartOf(files, QStringLiteral("styles.xml")).toUtf8(), nullptr));
            const int props = findElement(t, "page-layout-properties");
            QVERIFY2(props >= 0, k.name);
            const QString what = QString::fromLatin1(k.name);
            QVERIFY2(qAbs(lengthInches(t.attrAny(props, "margin-left")) - k.left) < 0.002, qPrintable(what + " left " + t.attrAny(props, "margin-left")));
            QVERIFY2(qAbs(lengthInches(t.attrAny(props, "margin-right")) - k.right) < 0.002, qPrintable(what + " right " + t.attrAny(props, "margin-right")));
            QVERIFY2(qAbs(lengthInches(t.attrAny(props, "margin-top")) - k.top) < 0.002, qPrintable(what + " top " + t.attrAny(props, "margin-top")));
            QVERIFY2(qAbs(lengthInches(t.attrAny(props, "margin-bottom")) - 1.0) < 0.002, qPrintable(what));
            QCOMPARE(t.attrAny(t.parent(props), "page-usage") == QLatin1String("mirrored"), k.mirrored);
            // The gutter itself is ours, apart from the margin.
            const QRegularExpressionMatch own = QRegularExpression(QStringLiteral("jo:gutter=\"([^\"]+)\"")).match(QString::fromUtf8(files.value(QStringLiteral("styles.xml"))));
            QVERIFY2(own.hasMatch() && qAbs(lengthInches(own.captured(1)) - 0.3) < 0.002, qPrintable(what));
            // And back: the margin and the gutter apart, where the gutter was.
            const QByteArray docx = docxOf(odt);
            const QString doc = part(docx, QStringLiteral("/word/document.xml"));
            wml::XmlTree d;
            QVERIFY(d.parse(doc.toUtf8(), nullptr));
            const int mar = findElement(d, "pgMar");
            QVERIFY2(mar >= 0, k.name);
            QVERIFY2(qAbs(d.attr(mar, "left").toInt() - 1440) <= 2 && qAbs(d.attr(mar, "right").toInt() - 1440) <= 2 && qAbs(d.attr(mar, "top").toInt() - 1440) <= 2, qPrintable(what + doc));
            QVERIFY2(qAbs(d.attr(mar, "gutter").toInt() - 432) <= 2, qPrintable(what + " gutter " + d.attr(mar, "gutter")));
            QCOMPARE(findElement(d, "rtlGutter") >= 0, k.rtlGutter);
            wml::XmlTree s;
            QVERIFY(s.parse(part(docx, QStringLiteral("/word/settings.xml")).toUtf8(), nullptr));
            QCOMPARE(findElement(s, "gutterAtTop") >= 0, k.atTop);
            QCOMPARE(findElement(s, "mirrorMargins") >= 0, k.mirrored);
        }
        // A file of another program (no gutter of ours) has its margin as it is; a gutter larger than the margin it is taken from is bounded by it.
        Made foreign;
        foreign.body = "<text:p>x</text:p>";
        foreign.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"/>";
        const QString layout = QStringLiteral("<style:page-layout style:name=\"pm1\"><style:page-layout-properties xmlns:jo=\"urn:jeffoffice:odf-extension\" fo:page-width=\"8.5in\" fo:page-height=\"11in\" fo:margin-top=\"1in\" fo:margin-bottom=\"1in\" "
                                              "fo:margin-left=\"1.3in\" fo:margin-right=\"1in\"%1/></style:page-layout>");
        foreign.layouts = layout.arg(QString()).toUtf8();
        wml::XmlTree d;
        QVERIFY(d.parse(document(foreign).toUtf8(), nullptr));
        QCOMPARE(d.attr(findElement(d, "pgMar"), "gutter").toInt(), 0);
        QVERIFY(qAbs(d.attr(findElement(d, "pgMar"), "left").toInt() - 1872) <= 2);
        foreign.layouts = layout.arg(QStringLiteral(" jo:gutter=\"99in\" jo:gutter-side=\"left\"")).toUtf8();
        wml::XmlTree h;
        QVERIFY(h.parse(document(foreign).toUtf8(), nullptr));
        const int mar = findElement(h, "pgMar");
        QVERIFY(mar >= 0);
        QVERIFY2(h.attr(mar, "left").toInt() >= 0 && h.attr(mar, "left").toInt() + h.attr(mar, "gutter").toInt() <= 1872 + 2, qPrintable(h.attr(mar, "left") + "/" + h.attr(mar, "gutter")));
    }

    // A shape that is turned or mirrored is written as the standard's transform of its box (the box unturned, set where, turned, its center is where the source shape's was), and comes back turned and mirrored the same.
    void turnedAndMirroredShapesAreTheStandardsTransform()
    {
        struct Case {
            double turn;
            bool flipX, flipY;
        };
        const QList<Case> cases = {{30, false, false}, {90, false, false}, {270, false, false}, {0, true, false}, {0, false, true}, {30, true, false}, {270, false, true}};
        for (const QString &kind : {QStringLiteral("rect"), QStringLiteral("oval"), QStringLiteral("box"), QStringLiteral("picture")})
            for (const Case &k : cases) {
                const QString what = QStringLiteral("%1 turn %2 flipX %3 flipY %4").arg(kind).arg(k.turn).arg(k.flipX).arg(k.flipY);
                QString more = k.turn != 0 ? QStringLiteral("rotation:%1").arg(k.turn) : QString();
                if (k.flipX || k.flipY) more += QStringLiteral(";flip:%1%2").arg(k.flipX ? QStringLiteral("x") : QString(), k.flipY ? QStringLiteral("y") : QString());
                const QByteArray odt = odtOfDrawing(pictBody(vmlShape(kind, more)), QStringLiteral("http://127.0.0.1:9/p.png"));
                const QMap<QString, QByteArray> files = filesOf(odt);
                QVERIFY2(!files.isEmpty(), qPrintable(what));
                const QString c = standardContentOf(files);
                wml::XmlTree t;
                QVERIFY2(t.parse(c.toUtf8(), nullptr), qPrintable(what));
                int frame = -1;
                const int node = shapeNodeOf(t, kind, &frame);
                QVERIFY2(node >= 0, qPrintable(what + c));
                const int box = frame >= 0 ? frame : node;
                const double w = lengthInches(t.attrAny(box, "width"));
                const double h = lengthInches(t.attrAny(box, "height"));
                QVERIFY2(qAbs(w - 2.0) < 0.002 && qAbs(h - 1.0) < 0.002, qPrintable(what));
                TurnedBox b;
                if (t.attrAny(box, "transform").isEmpty()) {
                    // (a picture that is only mirrored, by its own property, has a plain place)
                    QVERIFY2(kind == QLatin1String("picture") && k.turn == 0, qPrintable(what + ": no transform"));
                    b.ok = true;
                    b.centerX = lengthInches(t.attrAny(box, "x")) + w / 2;
                    b.centerY = lengthInches(t.attrAny(box, "y")) + h / 2;
                } else {
                    QVERIFY2(t.attrAny(box, "x").isEmpty() && t.attrAny(box, "y").isEmpty(), qPrintable(what + ": the position is the transform's"));
                    b = turnedBoxOf(t.attrAny(box, "transform"), w, h);
                }
                QVERIFY2(b.ok, qPrintable(what + c));
                QVERIFY2(qAbs(b.centerX - 2.0) < 0.002 && qAbs(b.centerY - 1.0) < 0.002, qPrintable(QStringLiteral("%1: center %2, %3").arg(what).arg(b.centerX).arg(b.centerY)));
                double degrees = std::fmod(-b.angle * 180.0 / M_PI, 360.0);   // (the standard turns against the clock)
                if (degrees < -0.01) degrees += 360.0;
                QVERIFY2(qAbs(degrees - k.turn) < 0.01 || qAbs(degrees - k.turn - 360.0) < 0.01, qPrintable(QStringLiteral("%1: turn %2").arg(what).arg(degrees)));
                QString mirror;
                if (kind == QLatin1String("picture")) mirror = graphicPropertiesOf(t, t.attrAny(box, "style-name")).value(QStringLiteral("mirror"));
                QCOMPARE(b.flipX || mirror.contains(QLatin1String("horizontal")), k.flipX);
                QCOMPARE(b.flipY || mirror.contains(QLatin1String("vertical")), k.flipY);
                // And back: the same turn, the same mirroring, and the box where it was set.
                const ShapeInDocx back = firstShapeOfDocx(odt);
                QVERIFY2(back.found, qPrintable(what));
                QVERIFY2(qAbs(back.turn - k.turn) < 0.01, qPrintable(QStringLiteral("%1: back turn %2").arg(what).arg(back.turn)));
                QCOMPARE(back.flipH, k.flipX);
                QCOMPARE(back.flipV, k.flipY);
                QVERIFY2(qAbs(back.left - 1.0) < 0.003 && qAbs(back.top - 0.5) < 0.003, qPrintable(QStringLiteral("%1: back at %2, %3").arg(what).arg(back.left).arg(back.top)));
            }
    }

    // A picture of the older form that the file holds and that floats (it says its place) is a frame set from its paragraph with the wrap it had, not a picture in the line: set in the line it makes
    // the line as tall as itself, and the text under it goes a page further. One that says no place stays in the line.
    void aFloatingPictureOfTheOlderFormStaysFloating()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        for (const bool floating : {true, false}) {
            const QByteArray shape = floating ? QByteArray("<v:shape type=\"#_x0000_t75\" style=\"position:absolute;margin-left:10pt;margin-top:-20pt;width:100pt;height:50pt;z-index:251660288\"><v:imagedata r:id=\"rId8\"/>"
                                                           "<w10:wrap type=\"square\"/></v:shape>")
                                              : QByteArray("<v:shape type=\"#_x0000_t75\" style=\"width:100pt;height:50pt\"><v:imagedata r:id=\"rId8\"/></v:shape>");
            PackageWriter w;
            w.addPart(QStringLiteral("/word/document.xml"),
                      QByteArray("<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:v=\"urn:schemas-microsoft-com:vml\" "
                                 "xmlns:w10=\"urn:schemas-microsoft-com:office:word\"><w:body><w:p><w:r><w:t>text</w:t></w:r><w:r><w:pict>") + shape + "</w:pict></w:r></w:p></w:body></w:document>",
                      QStringLiteral("application/xml"));
            w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            w.addPart(QStringLiteral("/word/media/image1.png"), kPng, QStringLiteral("image/png"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId8"), rel + QStringLiteral("image"), QStringLiteral("/word/media/image1.png"));
            const QByteArray odt = fromDocxPackage(w.finish());
            const QString c = standardContentOf(filesOf(odt));
            QVERIFY2(c.contains(QLatin1String("<draw:image ")), qPrintable(c));
            QCOMPARE(c.contains(QLatin1String("text:anchor-type=\"as-char\"")), !floating);
            QCOMPARE(c.contains(QLatin1String("text:anchor-type=\"paragraph\"")), floating);
            if (floating) {
                QVERIFY2(c.contains(QLatin1String("style:wrap=\"parallel\"")), qPrintable(c));
                const ShapeInDocx back = firstShapeOfDocx(odt);
                QVERIFY(back.found && qAbs(back.top + 20.0 / 72.0) < 0.003);
            }
        }
    }

    // A text box has the height it was given, in the file's own words: a fixed box is the frame's height, an auto-height box is the text box's minimum height (the frame says none), so a program that reads the
    // standard sees the same box, and it comes back the same height, unturned or turned, with room inside and a wide line (the outer size is the frame's, whatever the padding and the line take of it).
    void aTextBoxKeepsItsHeightWhateverItsPaddingAndLine()
    {
        const double heightPt = 72.35;
        for (const QString &kind : {QStringLiteral("vml"), QStringLiteral("fixed"), QStringLiteral("auto")})
            for (const double turn : {0.0, 30.0, 90.0}) {
                const QString what = QStringLiteral("%1 turned %2").arg(kind).arg(turn);
                QByteArray body;
                if (kind == QLatin1String("vml")) {
                    body = pictBody(QStringLiteral("<v:shape type=\"#_x0000_t202\" id=\"b1\" style=\"position:absolute;margin-left:72pt;margin-top:36pt;width:144pt;height:%1pt;z-index:251660288%2\" filled=\"f\" "
                                                   "stroked=\"t\" strokecolor=\"#ff0000\" strokeweight=\"4pt\"><v:textbox inset=\"9pt,4pt,11pt,5pt\"><w:txbxContent><w:p><w:r><w:t>Boxed</w:t></w:r></w:p></w:txbxContent></v:textbox></v:shape>")
                                        .arg(heightPt).arg(turn != 0 ? QStringLiteral(";rotation:%1").arg(turn) : QString()).toUtf8());
                } else {
                    body = QStringLiteral("<w:p><w:r><w:drawing><wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"1\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
                                         "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"column\"><wp:posOffset>914400</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"paragraph\"><wp:posOffset>457200</wp:posOffset></wp:positionV>"
                                         "<wp:extent cx=\"1828800\" cy=\"%1\"/><wp:wrapSquare wrapText=\"bothSides\"/><wp:docPr id=\"1\" name=\"B\"/><a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\">"
                                         "<wps:wsp xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm rot=\"%2\"><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1828800\" cy=\"%1\"/></a:xfrm>"
                                         "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:ln w=\"50800\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></wps:spPr><wps:txbx><w:txbxContent><w:p><w:r><w:t>Boxed</w:t></w:r></w:p></w:txbxContent></wps:txbx>"
                                         "<wps:bodyPr lIns=\"114300\" tIns=\"50800\" rIns=\"139700\" bIns=\"63500\">%3</wps:bodyPr></wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing></w:r></w:p>")
                               .arg(qint64(std::llround(heightPt * 12700))).arg(qint64(turn * 60000)).arg(kind == QLatin1String("auto") ? QStringLiteral("<a:spAutoFit/>") : QStringLiteral("<a:noAutofit/>")).toUtf8();
                }
                const QByteArray odt = odtOfDrawing(body);
                const QString c = standardContentOf(filesOf(odt));
                wml::XmlTree t;
                QVERIFY2(t.parse(c.toUtf8(), nullptr), qPrintable(what));
                const int inner = findElement(t, "text-box");
                QVERIFY2(inner >= 0, qPrintable(what + c));
                const int frame = t.parent(inner);
                // (section 19.240: the frame says a fixed height as it is; a box that grows says its least height on the text box inside, and the frame says none)
                QVERIFY2(t.attrAny(frame, "min-height").isEmpty(), qPrintable(what + ": the frame says a minimum height"));
                const QString fixedHeight = t.attrAny(frame, "height"), leastHeight = t.attrAny(inner, "min-height");
                if (kind == QLatin1String("auto")) {
                    QVERIFY2(fixedHeight.isEmpty() && !leastHeight.isEmpty(), qPrintable(what + ": the text box is at least its height"));
                    QVERIFY2(qAbs(lengthInches(leastHeight) * 72.0 - heightPt) < 0.05, qPrintable(what + ": " + leastHeight));
                } else {
                    QVERIFY2(leastHeight.isEmpty() && !fixedHeight.isEmpty(), qPrintable(what + ": the frame is its height"));
                    QVERIFY2(qAbs(lengthInches(fixedHeight) * 72.0 - heightPt) < 0.05, qPrintable(what + ": " + fixedHeight));
                }
                // And back: the same outer height.
                wml::XmlTree d;
                QVERIFY2(d.parse(part(docxOf(odt), QStringLiteral("/word/document.xml")).toUtf8(), nullptr), qPrintable(what));
                const int extent = findElement(d, "extent");
                QVERIFY2(extent >= 0, qPrintable(what));
                QVERIFY2(qAbs(d.attr(extent, "cy").toDouble() / 12700.0 - heightPt) < 0.05, qPrintable(what + ": back " + d.attr(extent, "cy")));
            }
    }

    // A shape whose fill is a picture is not a plain rectangle: it is the picture (the picture's address is the image's), and a text box that has one is kept without it and says so.
    void aShapeFilledWithAPictureIsNotAWhiteRectangle()
    {
        const QString link = QStringLiteral("http://127.0.0.1:9/fill.png");
        const QByteArray fillRect = "<v:rect id=\"f\" style=\"position:absolute;margin-left:10pt;margin-top:10pt;width:100pt;height:50pt\" stroked=\"f\"><v:fill r:id=\"rId9\" type=\"frame\"/></v:rect>";
        const QMap<QString, QByteArray> files = filesOf(odtOfDrawing(pictBody(fillRect), link));
        const QString c = standardContentOf(files);
        QVERIFY2(c.contains(QLatin1String("xlink:href=\"") + link + QLatin1String("\"")), qPrintable(c));
        QVERIFY2(!c.contains(QLatin1String("<draw:rect")), qPrintable(c));
        QVERIFY2(!c.toLower().contains(QLatin1String("#ffffff")), qPrintable(c));
        // (the same shape beside a picture of its own: both are pictures)
        const QMap<QString, QByteArray> both = filesOf(odtOfDrawing(pictBody(fillRect + vmlShape(QStringLiteral("picture"), QString(), QStringLiteral("p2"))), link));
        QCOMPARE(standardContentOf(both).count(QLatin1String("<draw:image ")), 2);
        // A text box with a picture fill keeps its words and is the box without the picture; the notes say what was left out.
        const QByteArray fillBox = "<v:shape type=\"#_x0000_t202\" id=\"b\" style=\"position:absolute;margin-left:10pt;margin-top:10pt;width:100pt;height:50pt\" stroked=\"f\"><v:fill r:id=\"rId9\" type=\"frame\"/>"
                                   "<v:textbox><w:txbxContent><w:p><w:r><w:t>Boxed words</w:t></w:r></w:p></w:txbxContent></v:textbox></v:shape>";
        QStringList notes;
        const QString box = standardContentOf(filesOf(odtOfDrawing(pictBody(fillBox), link, &notes)));
        QVERIFY2(box.contains(QLatin1String("Boxed words")), qPrintable(box));
        QVERIFY2(notes.join(QLatin1Char('\n')).contains(QLatin1String("pictures that fill text boxes")), qPrintable(notes.join(QLatin1Char('\n'))));
    }

    // A group in the line is the standard's group with its members; a member that cannot be written does not take the rest with it, and the notes name what was left out.
    void aGroupIsAGroupOfItsMembers()
    {
        const QByteArray red = "<v:rect id=\"a\" style=\"position:absolute;left:0pt;top:0pt;width:100pt;height:50pt\" filled=\"t\" fillcolor=\"#ff0000\" stroked=\"f\"/>";
        const QByteArray blue = "<v:oval id=\"b\" style=\"position:absolute;left:100pt;top:50pt;width:100pt;height:50pt\" filled=\"t\" fillcolor=\"#0000ff\" stroked=\"f\"/>";
        const QByteArray art = "<v:shape type=\"#_x0000_t136\" id=\"c\" style=\"position:absolute;left:0pt;top:50pt;width:100pt;height:50pt\"><v:textpath on=\"t\" string=\"Words on a path\" style=\"font-family:Arial\"/></v:shape>";
        auto group = [&](const QByteArray &members) { return QByteArray("<v:group id=\"g\" style=\"width:200pt;height:100pt\" coordsize=\"200,100\">") + members + "</v:group>"; };
        QStringList notes;
        const QByteArray odt = odtOfDrawing(pictBody(group(red + art + blue)), QString(), &notes);
        wml::XmlTree t;
        QVERIFY(t.parse(standardContentOf(filesOf(odt)).toUtf8(), nullptr));
        const int g = findElement(t, "g");
        QVERIFY(g >= 0);
        QStringList kinds;
        for (int k = t.firstChild(g); k >= 0; k = t.next(k)) kinds << QString::fromLatin1(t.localName(k));
        kinds.sort();
        QCOMPARE(kinds, (QStringList{QStringLiteral("ellipse"), QStringLiteral("rect")}));
        QCOMPARE(findElement(t, "rect") >= 0 && t.parent(findElement(t, "rect")) == g, true);
        QVERIFY2(notes.join(QLatin1Char('\n')).contains(QLatin1String("drawings, shapes, and charts")), qPrintable(notes.join(QLatin1Char('\n'))));
        // The members come back with the distance between them (100 points across, 50 down).
        wml::XmlTree d;
        QVERIFY(d.parse(part(docxOf(odt), QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
        QList<double> xs, ys;
        for (int i = findElement(d, "posOffset"); i >= 0; i = findElement(d, "posOffset", i)) (d.localName(d.parent(i)) == "positionH" ? xs : ys) << d.text(i).toDouble() / 12700.0;
        QCOMPARE(xs.size(), 2);
        QCOMPARE(ys.size(), 2);
        std::sort(xs.begin(), xs.end());
        std::sort(ys.begin(), ys.end());
        QVERIFY(qAbs(xs[1] - xs[0] - 100.0) < 0.5 && qAbs(ys[1] - ys[0] - 50.0) < 0.5);
        // A group all of whose members are written says nothing was left out.
        QStringList none;
        const QString whole = standardContentOf(filesOf(odtOfDrawing(pictBody(group(red + blue)), QString(), &none)));
        QVERIFY2(whole.contains(QLatin1String("<draw:g")), qPrintable(whole));
        QVERIFY2(!none.join(QLatin1Char('\n')).contains(QLatin1String("drawings, shapes, and charts")), qPrintable(none.join(QLatin1Char('\n'))));
    }

    // Shapes behind the text keep their order among themselves, and so do those in front; a shape behind the text is run through "background" and one in front "foreground"; a shape in the line has an order too.
    void shapesKeepTheirOrderAndTheirSideOfTheText()
    {
        auto rect = [](int widthPt, qint64 z) {
            return QStringLiteral("<w:r><w:pict><v:rect style=\"position:absolute;margin-left:10pt;margin-top:10pt;width:%1pt;height:50pt;z-index:%2\" filled=\"t\" fillcolor=\"#ff0000\" stroked=\"f\"/></w:pict></w:r>").arg(widthPt).arg(z);
        };
        // (written out of their order: by z-index the widths run 102, 103, 101 behind and 203, 201, 202 in front)
        const QString body = QStringLiteral("<w:p>") + rect(101, -251658240 + 3072) + rect(102, -251658240 + 1024) + rect(103, -251658240 + 2048) + rect(201, 251660288 + 1024) + rect(202, 251660288 + 2048) + rect(203, 251660288) +
                             QStringLiteral("<w:r><w:pict><v:rect style=\"width:30pt;height:30pt;z-index:251661312\" filled=\"t\" fillcolor=\"#00ff00\" stroked=\"f\"/></w:pict></w:r></w:p>");
        const QMap<QString, QByteArray> files = filesOf(odtOfDrawing(body.toUtf8()));
        wml::XmlTree t;
        QVERIFY(t.parse(standardContentOf(files).toUtf8(), nullptr));
        QMap<int, qint64> order;   // width in points -> z-index
        QMap<int, QString> side;
        bool inlineHasOrder = false;
        for (int r = findElement(t, "rect"); r >= 0; r = findElement(t, "rect", r)) {
            const int width = int(std::lround(lengthInches(t.attrAny(r, "width")) * 72.0));
            const QMap<QString, QString> gp = graphicPropertiesOf(t, t.attrAny(r, "style-name"));
            if (width == 30) {
                inlineHasOrder = !t.attrAny(r, "z-index").isEmpty() && t.attrAny(r, "anchor-type") == QLatin1String("as-char");
                continue;
            }
            QVERIFY2(!t.attrAny(r, "z-index").isEmpty(), qPrintable(QString::number(width)));
            order.insert(width, t.attrAny(r, "z-index").toLongLong());
            side.insert(width, gp.value(QStringLiteral("run-through")));
        }
        QVERIFY(inlineHasOrder);
        QCOMPARE(order.size(), 6);
        QVERIFY2(order[102] < order[103] && order[103] < order[101], qPrintable(QStringLiteral("behind: %1 %2 %3").arg(order[102]).arg(order[103]).arg(order[101])));
        QVERIFY2(order[203] < order[201] && order[201] < order[202], qPrintable(QStringLiteral("in front: %1 %2 %3").arg(order[203]).arg(order[201]).arg(order[202])));
        for (int w : {101, 102, 103}) QCOMPARE(side[w], QStringLiteral("background"));
        for (int w : {201, 202, 203}) QCOMPARE(side[w], QStringLiteral("foreground"));
    }

    // A picture linked by a network address is the standard's address of a network location, and comes back as the same address (a UNC one).
    void aNetworkAddressOfALinkedPictureIsAFileAddress()
    {
        const QString unc = QStringLiteral("\\\\host\\share\\my pictures\\x.png");
        const QByteArray odt = odtOfDrawing(linkedPictureBody(), unc);
        const QString c = standardContentOf(filesOf(odt));
        QVERIFY2(c.contains(QLatin1String("xlink:href=\"file://host/share/my%20pictures/x.png\"")), qPrintable(c));
        QVERIFY2(!c.contains(QLatin1String("xlink:href=\"//")), qPrintable(c));
        Package p;
        QVERIFY(p.read(docxOf(odt)));
        QStringList targets;
        for (const Relationship &r : p.relationships(QStringLiteral("/word/document.xml")))
            if (r.external) targets << r.target;
        QCOMPARE(targets, QStringList{unc});
        // (a path on this computer still has slashes, and an address longer than any is not kept)
        QVERIFY2(standardContentOf(filesOf(odtOfDrawing(linkedPictureBody(), QStringLiteral("..\\pics\\y.png")))).contains(QLatin1String("xlink:href=\"../pics/y.png\"")), "a relative path");
        QVERIFY2(!standardContentOf(filesOf(odtOfDrawing(linkedPictureBody(), QStringLiteral("\\\\host\\") + QString(5000, QLatin1Char('x'))))).contains(QLatin1String("host")), "a length no address has");
    }

    // The older form's drawing is read once, and only if it is no longer than the most one holds; a line's ends are bounded like the other lengths; a forged place in a row with a span moves the grid by it.
    void hostileShapesAndPlacesAreBounded()
    {
        const qint64 parses = wml::readShapesParsesForTest();
        QByteArray big = "<w:p><w:r><w:pict><v:rect style=\"position:absolute;width:10pt;height:10pt\"/><!--";
        big += QByteArray(5 << 20, 'x');
        big += "--></w:pict></w:r></w:p>";
        QStringList notes;
        const QMap<QString, QByteArray> files = filesOf(odtOfDrawing(big, QString(), &notes));
        QVERIFY(!files.isEmpty());
        QCOMPARE(wml::readShapesParsesForTest(), parses);
        QVERIFY2(!standardContentOf(files).contains(QLatin1String("<draw:rect")), "an over-long drawing is not read");
        // (and one of the usual length is read once)
        const qint64 before = wml::readShapesParsesForTest();
        filesOf(odtOfDrawing(pictBody(vmlShape(QStringLiteral("rect")))));
        QVERIFY(wml::readShapesParsesForTest() - before <= 1);
        // A line's ends are bounded in the file and when it is read.
        Made far;
        far.body = "<text:p><draw:line text:anchor-type=\"paragraph\" svg:x1=\"-50000pt\" svg:y1=\"0pt\" svg:x2=\"40000pt\" svg:y2=\"60000pt\"/></text:p>";
        wml::XmlTree d;
        QVERIFY(d.parse(document(far).toUtf8(), nullptr));
        for (int i = findElement(d, "posOffset"); i >= 0; i = findElement(d, "posOffset", i)) QVERIFY2(qAbs(d.text(i).toLongLong()) <= qint64(12000) * 12700, qPrintable(d.text(i)));
        const QString written = standardContentOf(filesOf(odtOfDrawing(pictBody("<v:line style=\"position:absolute;margin-left:0pt;margin-top:0pt;width:10pt;height:10pt\" from=\"-5000000pt,0pt\" to=\"5000000pt,9000000pt\"/>"))));
        wml::XmlTree w;
        QVERIFY(w.parse(written.toUtf8(), nullptr));
        const int line = findElement(w, "line");
        QVERIFY2(line >= 0, qPrintable(written));
        for (const char *a : {"x1", "y1", "x2", "y2"}) QVERIFY2(qAbs(lengthInches(w.attrAny(line, a))) <= 12000.0 / 72.0 + 0.01, a);
        // A forged place (jo:pad) with a span of three, at the start of the row, is three places of the grid.
        Made m;
        m.body = "<table:table xmlns:jo=\"urn:jeffoffice:odf-extension\"><table:table-column table:number-columns-repeated=\"4\"/>"
                 "<table:table-row><table:table-cell><text:p>a</text:p></table:table-cell><table:table-cell><text:p>b</text:p></table:table-cell><table:table-cell><text:p>c</text:p></table:table-cell><table:table-cell><text:p>d</text:p></table:table-cell></table:table-row>"
                 "<table:table-row><table:table-cell jo:pad=\"true\" table:number-columns-spanned=\"3\"/><table:covered-table-cell/><table:covered-table-cell/><table:table-cell><text:p>e</text:p></table:table-cell></table:table-row></table:table>";
        const QString doc = document(m);
        QVERIFY2(doc.contains(QLatin1String("<w:gridBefore w:val=\"3\"/>")), qPrintable(doc));
        QCOMPARE(doc.count(QLatin1String("<w:tc>")), 5);
    }

    // Our attributes are in one namespace, declared once on the root of each part: nothing is declared again beside a shape or a picture, and the older namespace is only read; an arrowhead is read from either.
    void ourAttributesAreInOneNamespaceDeclaredOnce()
    {
        const QByteArray lines = pictBody("<v:line style=\"position:absolute;margin-left:10pt;margin-top:10pt;width:100pt;height:0pt\" from=\"0pt,0pt\" to=\"100pt,0pt\" strokeweight=\"1pt\"><v:stroke startarrow=\"block\" endarrow=\"classic\"/></v:line>");
        const QMap<QString, QByteArray> files = filesOf(odtOfDrawing(lines + pictBody(vmlShape(QStringLiteral("rect"), QStringLiteral("rotation:30")))));
        for (const QString &name : {QStringLiteral("content.xml"), QStringLiteral("styles.xml")}) {
            const QString text = QString::fromUtf8(files.value(name));
            QCOMPARE(text.count(QLatin1String("xmlns:jo=")), 1);
            QVERIFY2(!text.contains(QLatin1String("names:extension")), qPrintable(name));
            const int root = text.indexOf(QLatin1String("xmlns:jo="));
            QVERIFY2(root >= 0 && root < 3000, qPrintable(name));
        }
        const QString content = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        QVERIFY2(!content.contains(QLatin1String("jo:head-end=\"")) && !content.contains(QLatin1String("jo:tail-end=\"")) && !content.contains(QLatin1String("jo:rotation")), qPrintable(content));
        QVERIFY2(QString::fromUtf8(files.value(QStringLiteral("styles.xml"))).contains(QLatin1String("<draw:marker ")), "no marker styles");
        // A picture of ours with a barcode and a drawing shape of ours with a dash: the same one declaration.
        // The older namespace is still read (a file of an earlier version): its arrow ends come back as arrow ends.
        Made old;
        old.body = "<text:p><draw:custom-shape text:anchor-type=\"paragraph\" svg:width=\"2in\" svg:height=\"1in\" xmlns:jo=\"urn:jeffoffice:names:extension:1.0\" jo:head-end=\"triangle\" jo:tail-end=\"arrow\" jo:dash=\"sysDot\" jo:rotation=\"30.0000\">"
                   "<draw:enhanced-geometry draw:type=\"ooxml-rect\"/></draw:custom-shape></text:p>";
        old.autoStyles = "<style:style style:name=\"gr1\" style:family=\"graphic\"><style:graphic-properties draw:stroke=\"solid\" svg:stroke-color=\"#000000\" svg:stroke-width=\"0.0139in\"/></style:style>";
        old.body.replace("<draw:custom-shape ", "<draw:custom-shape draw:style-name=\"gr1\" ");
        const QString doc = document(old);
        QVERIFY2(doc.contains(QLatin1String("<a:headEnd type=\"triangle\"/>")) && doc.contains(QLatin1String("<a:tailEnd type=\"arrow\"/>")), qPrintable(doc));
        QVERIFY2(doc.contains(QLatin1String("sysDot")) && doc.contains(QLatin1String("rot=\"1800000\"")), qPrintable(doc));
        // And the ends of the new namespace the same.
        Made fresh;
        fresh.body = old.body;
        fresh.body.replace("xmlns:jo=\"urn:jeffoffice:names:extension:1.0\"", "xmlns:jo=\"urn:jeffoffice:odf-extension\"");
        fresh.autoStyles = old.autoStyles;
        const QString doc2 = document(fresh);
        QVERIFY2(doc2.contains(QLatin1String("<a:headEnd type=\"triangle\"/>")) && doc2.contains(QLatin1String("<a:tailEnd type=\"arrow\"/>")), qPrintable(doc2));
    }

    // Text boxes inside text boxes, three deep, are three boxes in the file, each inside the one before, and come back as three.
    void textBoxesThreeDeepAreKept()
    {
        auto box = [](const QByteArray &inner) {
            return QByteArray("<w:p><w:r><w:pict><v:shape type=\"#_x0000_t202\" style=\"position:absolute;margin-left:0pt;margin-top:0pt;width:200pt;height:100pt\" filled=\"f\" stroked=\"f\"><v:textbox><w:txbxContent>") + inner +
                   "</w:txbxContent></v:textbox></v:shape></w:pict></w:r></w:p>";
        };
        const QByteArray body = box("<w:p><w:r><w:t>outer</w:t></w:r></w:p>" + box("<w:p><w:r><w:t>middle</w:t></w:r></w:p>" + box("<w:p><w:r><w:t>deepest</w:t></w:r></w:p>")));
        const QByteArray odt = odtOfDrawing(body);
        wml::XmlTree t;
        QVERIFY(t.parse(standardContentOf(filesOf(odt)).toUtf8(), nullptr));
        QList<int> boxes;
        for (int i = findElement(t, "text-box"); i >= 0; i = findElement(t, "text-box", i)) boxes << i;
        QCOMPARE(boxes.size(), 3);
        // (one box at each depth: none, one, and two boxes around it)
        QList<int> depths;
        for (int b : boxes) {
            int depth = 0;
            for (int p = t.parent(b); p >= 0; p = t.parent(p))
                if (t.localName(p) == "text-box") ++depth;
            depths << depth;
        }
        std::sort(depths.begin(), depths.end());
        QCOMPARE(depths, (QList<int>{0, 1, 2}));
        const QString doc = part(docxOf(odt), QStringLiteral("/word/document.xml"));
        QCOMPARE(doc.count(QLatin1String("<w:txbxContent>")), 3);
        QVERIFY2(doc.contains(QLatin1String("deepest")) && doc.contains(QLatin1String("middle")) && doc.contains(QLatin1String("outer")), qPrintable(doc));
    }

    // Our place-holders of a row (jo:pad) taken out, every row has as many places as the table has columns, also with spans, and the places are the same.
    void everyRowHasAsManyPlacesAsTheTableHasColumnsWithoutOurAttribute()
    {
        auto cell = [](const char *text, int span = 1) {
            return QByteArray("<w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/>") + (span > 1 ? "<w:gridSpan w:val=\"" + QByteArray::number(span) + "\"/>" : QByteArray()) + "</w:tcPr><w:p><w:r><w:t>" + text + "</w:t></w:r></w:p></w:tc>";
        };
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"4000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/></w:tblGrid>"
                                 "<w:tr>" + cell("A") + cell("B", 2) + cell("C") + "</w:tr>"
                                 "<w:tr><w:trPr><w:gridBefore w:val=\"1\"/><w:gridAfter w:val=\"1\"/></w:trPr>" + cell("D", 2) + "</w:tr>"
                                 "<w:tr><w:trPr><w:gridBefore w:val=\"2\"/></w:trPr>" + cell("E") + cell("F") + "</w:tr></w:tbl><w:p/>";
        const QString c = QString(odtContentOf(table)).remove(QRegularExpression(QStringLiteral(" jo:[A-Za-z-]+=\"[^\"]*\"")));
        wml::XmlTree t;
        QVERIFY(t.parse(c.toUtf8(), nullptr));
        int columns = 0;
        for (int k = findElement(t, "table-column"); k >= 0; k = findElement(t, "table-column", k)) columns += std::max(1, t.attrAny(k, "number-columns-repeated").toInt());
        QCOMPARE(columns, 4);
        int rows = 0;
        for (int r = findElement(t, "table-row"); r >= 0; r = findElement(t, "table-row", r)) {
            ++rows;
            int places = 0;
            for (int k = t.firstChild(r); k >= 0; k = t.next(k))
                if (t.localName(k) == "table-cell" || t.localName(k) == "covered-table-cell") places += std::max(1, t.attrAny(k, "number-columns-repeated").toInt());
            QCOMPARE(places, columns);
        }
        QCOMPARE(rows, 3);
        // and ours reads the same rows back: the places are not cells
        const QString back = roundTripBody(table);
        QCOMPARE(back.count(QLatin1String("<w:gridBefore")), 2);
        QCOMPARE(back.count(QLatin1String("<w:tc>")), 6);
    }

    // ---- the text document against the standard (sections 16.5, 19.228, 19.231, 19.240, 19.759, 20.299, 20.398)

    // A paragraph "Before ", a shape of the drawing form (a preset) anchored to it with this place against these areas, and " after"; the shape's alternative text, its turn in degrees, and its line ends.
    static QByteArray anchoredShape(const QString &hFrom, qint64 hOff, const QString &vFrom, qint64 vOff, double turn = 0, bool ends = false, bool inLine = false)
    {
        const QString place = inLine ? QStringLiteral("<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"1828800\" cy=\"914400\"/>")
                                     : QStringLiteral("<wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"251660288\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
                                                      "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"%1\"><wp:posOffset>%2</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"%3\"><wp:posOffset>%4</wp:posOffset></wp:positionV>"
                                                      "<wp:extent cx=\"1828800\" cy=\"914400\"/><wp:wrapSquare wrapText=\"bothSides\"/>")
                                               .arg(hFrom).arg(hOff).arg(vFrom).arg(vOff);
        return QStringLiteral("<w:p><w:r><w:t xml:space=\"preserve\">Before </w:t></w:r><w:r><w:drawing>%1<wp:docPr id=\"1\" name=\"S\" descr=\"Alt words\"/><a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\">"
                             "<wps:wsp xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:cNvSpPr/><wps:spPr><a:xfrm rot=\"%2\"><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1828800\" cy=\"914400\"/></a:xfrm>"
                             "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill>%3</a:ln></wps:spPr><wps:bodyPr/></wps:wsp></a:graphicData></a:graphic>%4</w:drawing></w:r>"
                             "<w:r><w:t xml:space=\"preserve\"> after</w:t></w:r></w:p>")
            .arg(place).arg(qint64(turn * 60000)).arg(ends ? QStringLiteral("<a:headEnd type=\"triangle\"/><a:tailEnd type=\"oval\"/>") : QString()).arg(inLine ? QStringLiteral("</wp:inline>") : QStringLiteral("</wp:anchor>"))
            .toUtf8();
    }

    // The areas the standard's tables allow for each anchor (20.299 and 20.398, with the anchor values of 19.759).
    static bool areasAreAllowed(const QString &anchor, const QString &horizontal, const QString &vertical)
    {
        static const QStringList page = {QStringLiteral("page"), QStringLiteral("page-content"), QStringLiteral("page-start-margin"), QStringLiteral("page-end-margin")};
        static const QStringList paragraph = {QStringLiteral("paragraph"), QStringLiteral("paragraph-content"), QStringLiteral("paragraph-start-margin"), QStringLiteral("paragraph-end-margin")};
        if (anchor == QLatin1String("as-char"))
            return (horizontal.isEmpty() || horizontal == QLatin1String("char")) && (vertical.isEmpty() || QStringList{QStringLiteral("baseline"), QStringLiteral("char"), QStringLiteral("line"), QStringLiteral("text")}.contains(vertical));
        if (anchor != QLatin1String("paragraph") && anchor != QLatin1String("char")) return false;
        const bool c = anchor == QLatin1String("char");
        return (horizontal.isEmpty() || page.contains(horizontal) || paragraph.contains(horizontal) || (c && horizontal == QLatin1String("char"))) &&
               (vertical.isEmpty() || vertical == QLatin1String("page") || vertical == QLatin1String("page-content") || vertical == QLatin1String("paragraph") || vertical == QLatin1String("paragraph-content") ||
                (c && vertical == QLatin1String("char")));
    }

    // Every shape of the content, with the anchor it says and the two areas its style says (the anchor of a member is its group's); false and a description for the first that the tables do not allow.
    static bool everyAnchorIsAllowed(const QString &content, QString *why, int *count = nullptr)
    {
        wml::XmlTree t;
        if (!t.parse(content.toUtf8(), nullptr)) {
            *why = QStringLiteral("not parsed");
            return false;
        }
        int seen = 0;
        for (const char *name : {"frame", "custom-shape", "rect", "ellipse", "line", "path", "g"})
            for (int n = findElement(t, name); n >= 0; n = findElement(t, name, n)) {
                const QString anchor = t.attrAny(n, "anchor-type");
                if (anchor.isEmpty()) continue;
                const QMap<QString, QString> props = graphicPropertiesOf(t, t.attrAny(n, "style-name"));
                const QString h = props.value(QStringLiteral("horizontal-rel")), v = props.value(QStringLiteral("vertical-rel"));
                ++seen;
                if (!areasAreAllowed(anchor, h, v)) {
                    *why = QStringLiteral("%1 with horizontal %2 and vertical %3").arg(anchor, h, v);
                    return false;
                }
            }
        if (count) *count = seen;
        return true;
    }

    // A written page's layout has its usage on the layout itself (16.5, 19.509), not on its properties; both places are read, so that a file from another program or an earlier version of ours is too.
    void mirroredPagesAreSaidOnThePageLayout()
    {
        const QByteArray odt = odtOfBodyWithSettings("<w:p><w:r><w:t>x</w:t></w:r></w:p><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>", "<w:mirrorMargins/>");
        wml::XmlTree t;
        QVERIFY(t.parse(standardPartOf(filesOf(odt), QStringLiteral("styles.xml")).toUtf8(), nullptr));
        const int layout = findElement(t, "page-layout"), props = findElement(t, "page-layout-properties");
        QVERIFY(layout >= 0 && props >= 0);
        QCOMPARE(t.attrAny(layout, "page-usage"), QStringLiteral("mirrored"));
        QVERIFY2(t.attrAny(props, "page-usage").isEmpty(), "the usage is on the properties");
        // Read from either place (the properties are where our own files of yesterday put it).
        for (const bool onLayout : {true, false}) {
            Made m;
            m.body = "<text:p>x</text:p>";
            m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"/>";
            const QString usage = QStringLiteral(" style:page-usage=\"mirrored\"");
            m.layouts = QStringLiteral("<style:page-layout style:name=\"pm1\"%1><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" fo:margin-left=\"1in\" fo:margin-right=\"1in\"%2/></style:page-layout>")
                            .arg(onLayout ? usage : QString(), onLayout ? QString() : usage).toUtf8();
            const QByteArray docx = docxOf(m.build());
            wml::XmlTree s;
            QVERIFY(s.parse(part(docx, QStringLiteral("/word/settings.xml")).toUtf8(), nullptr));
            QVERIFY2(findElement(s, "mirrorMargins") >= 0, onLayout ? "usage on the layout is not read" : "usage on the properties is not read");
        }
    }

    // The description of a custom shape comes before its geometry (the format's order of children).
    void aCustomShapesDescriptionComesBeforeItsGeometry()
    {
        const QString c = standardContentOf(filesOf(odtOfDrawing(anchoredShape(QStringLiteral("column"), 0, QStringLiteral("paragraph"), 0))));
        wml::XmlTree t;
        QVERIFY(t.parse(c.toUtf8(), nullptr));
        const int shape = findElement(t, "custom-shape");
        QVERIFY2(shape >= 0, qPrintable(c));
        QStringList kids;
        for (int k = t.firstChild(shape); k >= 0; k = t.next(k)) kids << QString::fromLatin1(t.localName(k));
        QCOMPARE(kids, (QStringList{QStringLiteral("desc"), QStringLiteral("enhanced-geometry")}));
    }

    // A shape anchored to its paragraph is at the start of the paragraph's content (19.759); one in the line stays where it was.
    void aShapeAnchoredToAParagraphIsAtItsStart()
    {
        for (const bool inLine : {false, true}) {
            const QString c = standardContentOf(filesOf(odtOfDrawing(anchoredShape(QStringLiteral("column"), 0, QStringLiteral("paragraph"), 0, 0, false, inLine))));
            wml::XmlTree t;
            QVERIFY(t.parse(c.toUtf8(), nullptr));
            const int shape = findElement(t, "custom-shape");
            QVERIFY2(shape >= 0, qPrintable(c));
            QCOMPARE(t.attrAny(shape, "anchor-type"), inLine ? QStringLiteral("as-char") : QStringLiteral("paragraph"));
            // (the shape is the first thing in the paragraph when it is anchored to it, and after the words that came before it when it is in the line)
            static const QRegularExpression atStart(QStringLiteral("<text:p[^>]*><draw:custom-shape")), afterWords(QStringLiteral("<text:p[^>]*>Before <draw:custom-shape"));
            QCOMPARE(atStart.match(c).hasMatch(), !inLine);
            QCOMPARE(afterWords.match(c).hasMatch(), inLine);
            QVERIFY2(t.text(t.parent(shape)).contains(QLatin1String("after")), qPrintable(c));
        }
    }

    // Anchors and the areas a position is set against are the ones the tables allow: a position against a character, or against the line, is a shape anchored to a character, which comes back as it was
    // (the same areas, the same distances); the others are anchored to the paragraph; a shape in the line is set against a character and the baseline.
    void anchorsAndAreasAreTheOnesTheTablesAllow()
    {
        const QStringList horizontals = {QStringLiteral("page"), QStringLiteral("margin"), QStringLiteral("column"), QStringLiteral("character")};
        const QStringList verticals = {QStringLiteral("page"), QStringLiteral("margin"), QStringLiteral("paragraph"), QStringLiteral("line")};
        for (const QString &h : horizontals)
            for (const QString &v : verticals) {
                const QString what = h + QLatin1Char('/') + v;
                const QByteArray odt = odtOfDrawing(anchoredShape(h, 635000, v, 317500));
                const QString c = standardContentOf(filesOf(odt));
                QString why;
                QVERIFY2(everyAnchorIsAllowed(c, &why), qPrintable(what + ": " + why + c));
                wml::XmlTree t;
                QVERIFY(t.parse(c.toUtf8(), nullptr));
                const int shape = findElement(t, "custom-shape");
                QCOMPARE(t.attrAny(shape, "anchor-type"), (h == QLatin1String("character") || v == QLatin1String("line")) ? QStringLiteral("char") : QStringLiteral("paragraph"));
                // The same place when it is opened again: the same areas, and the distance within half a point.
                wml::XmlTree d;
                QVERIFY(d.parse(part(docxOf(odt), QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
                const int ph = findElement(d, "positionH"), pv = findElement(d, "positionV");
                QVERIFY2(ph >= 0 && pv >= 0, qPrintable(what));
                QCOMPARE(d.attr(ph, "relativeFrom"), h);
                QCOMPARE(d.attr(pv, "relativeFrom"), v);
                QVERIFY2(qAbs(d.text(d.firstChild(ph)).toDouble() - 635000.0) <= 6350.0, qPrintable(what + " across"));
                QVERIFY2(qAbs(d.text(d.firstChild(pv)).toDouble() - 317500.0) <= 6350.0, qPrintable(what + " down"));
            }
        // In the line: against the character and the baseline, for the drawing form and the older form.
        QString why;
        QVERIFY2(everyAnchorIsAllowed(standardContentOf(filesOf(odtOfDrawing(anchoredShape(QString(), 0, QString(), 0, 0, false, true)))), &why), qPrintable(why));
        const QByteArray older = pictBody(vmlShape(QStringLiteral("rect"))) + pictBody(vmlShape(QStringLiteral("rect"), QStringLiteral("mso-position-horizontal-relative:char;mso-position-vertical-relative:line"), QStringLiteral("s2"))) +
                                 pictBody("<v:rect id=\"in\" style=\"width:50pt;height:20pt\" filled=\"t\" fillcolor=\"#00ff00\" stroked=\"f\"/>");
        const QString oc = standardContentOf(filesOf(odtOfDrawing(older)));
        int seen = 0;
        QVERIFY2(everyAnchorIsAllowed(oc, &why, &seen), qPrintable(why + oc));
        QCOMPARE(seen, 3);
        QVERIFY2(oc.contains(QLatin1String("text:anchor-type=\"char\"")), qPrintable(oc));
    }

    // A box that grows with its text says its least height on the text box inside the frame (19.240), and the frame has no height of its own; a fixed box says it on the frame. The old place (the frame's
    // own least height) is still read.
    void aGrowingTextBoxSaysItsLeastHeightOnTheTextBox()
    {
        const QString body = QStringLiteral("<w:p><w:r><w:drawing><wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"1\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
                                            "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"column\"><wp:posOffset>0</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"paragraph\"><wp:posOffset>0</wp:posOffset></wp:positionV>"
                                            "<wp:extent cx=\"1828800\" cy=\"914400\"/><wp:wrapSquare wrapText=\"bothSides\"/><wp:docPr id=\"1\" name=\"B\"/><a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\">"
                                            "<wps:wsp xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1828800\" cy=\"914400\"/></a:xfrm>"
                                            "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></wps:spPr><wps:txbx><w:txbxContent><w:p><w:r><w:t>Boxed</w:t></w:r></w:p></w:txbxContent></wps:txbx><wps:bodyPr>%1</wps:bodyPr></wps:wsp>"
                                            "</a:graphicData></a:graphic></wp:anchor></w:drawing></w:r></w:p>");
        for (const bool grows : {true, false}) {
            const QString c = standardContentOf(filesOf(odtOfDrawing(body.arg(grows ? QStringLiteral("<a:spAutoFit/>") : QStringLiteral("<a:noAutofit/>")).toUtf8())));
            wml::XmlTree t;
            QVERIFY(t.parse(c.toUtf8(), nullptr));
            const int box = findElement(t, "text-box");
            QVERIFY2(box >= 0, qPrintable(c));
            const int frame = t.parent(box);
            QVERIFY2(t.attrAny(frame, "min-height").isEmpty(), qPrintable(c));
            if (grows) {
                QVERIFY2(t.attrAny(frame, "height").isEmpty() && qAbs(lengthInches(t.attrAny(box, "min-height")) - 1.0) < 0.002, qPrintable(c));
            } else {
                QVERIFY2(t.attrAny(box, "min-height").isEmpty() && qAbs(lengthInches(t.attrAny(frame, "height")) - 1.0) < 0.002, qPrintable(c));
            }
        }
        // Read from each of the three places, the height is the same.
        for (const QString &frameAttr : {QStringLiteral(" fo:min-height=\"1in\""), QStringLiteral(" svg:height=\"1in\""), QString()}) {
            Made m;
            m.body = QStringLiteral("<text:p><draw:frame text:anchor-type=\"paragraph\" svg:x=\"0in\" svg:y=\"0in\" svg:width=\"2in\"%1><draw:text-box%2><text:p>Boxed</text:p></draw:text-box></draw:frame></text:p>")
                         .arg(frameAttr, frameAttr.isEmpty() ? QStringLiteral(" fo:min-height=\"1in\"") : QString()).toUtf8();
            wml::XmlTree d;
            QVERIFY(d.parse(document(m).toUtf8(), nullptr));
            const int extent = findElement(d, "extent");
            QVERIFY2(extent >= 0 && qAbs(d.attr(extent, "cy").toDouble() - 914400.0) < 12700.0, qPrintable(frameAttr + ": " + d.attr(extent, "cy")));
        }
    }

    // The members of a group have no z-index (19.231): the group has the one, and the members are in the file in their order from the back to the front.
    void theMembersOfAGroupAreInTheirStackingOrder()
    {
        auto member = [](int width, qint64 z) {
            return QStringLiteral("<v:rect style=\"position:absolute;left:0pt;top:0pt;width:%1pt;height:50pt;z-index:%2\" filled=\"t\" fillcolor=\"#ff0000\" stroked=\"f\"/>").arg(width).arg(z).toUtf8();
        };
        // (written out of their order: by z-index the widths run 102, 103, 101)
        const QByteArray group = "<v:group id=\"g\" style=\"width:200pt;height:100pt\" coordsize=\"200,100\">" + member(101, 251658240 + 3072) + member(102, 251658240 + 1024) + member(103, 251658240 + 2048) + "</v:group>";
        const QByteArray odt = odtOfDrawing(pictBody(group));
        const QString c = standardContentOf(filesOf(odt));
        wml::XmlTree t;
        QVERIFY(t.parse(c.toUtf8(), nullptr));
        const int g = findElement(t, "g");
        QVERIFY2(g >= 0, qPrintable(c));
        QVERIFY2(!t.attrAny(g, "z-index").isEmpty(), "the group has no z-index");
        QList<int> widths;
        for (int k = t.firstChild(g); k >= 0; k = t.next(k)) {
            if (t.localName(k) != "rect") continue;
            QVERIFY2(t.attrAny(k, "z-index").isEmpty(), "a member has a z-index");
            widths << qRound(lengthInches(t.attrAny(k, "width")) * 72.0);
        }
        QCOMPARE(widths, (QList<int>{102, 103, 101}));
        // And back: the members keep their order from the back to the front.
        wml::XmlTree d;
        QVERIFY(d.parse(part(docxOf(odt), QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
        QMap<qint64, int> byHeight;
        for (int a = findElement(d, "anchor"); a >= 0; a = findElement(d, "anchor", a)) byHeight.insert(d.attr(a, "relativeHeight").toLongLong(), qRound(d.attr(findElement(d, "extent", a), "cx").toDouble() / 12700.0));
        QCOMPARE(byHeight.size(), 3);
        QCOMPARE(byHeight.values(), (QList<int>{102, 103, 101}));
    }

    // A shape's turn is the standard's transform, the line ends are its marker styles, and nothing of ours says either; the transform holds the shape's place, and the turn and the place come back.
    void aTurnedShapeAndItsLineEndsAreTheStandardsOwnWords()
    {
        const QByteArray odt = odtOfDrawing(anchoredShape(QStringLiteral("column"), 914400, QStringLiteral("paragraph"), 457200, 30, true));
        const QMap<QString, QByteArray> files = filesOf(odt);
        const QString all = QString::fromUtf8(files.value(QStringLiteral("content.xml")) + files.value(QStringLiteral("styles.xml")));
        QVERIFY2(!all.contains(QLatin1String("jo:rotation")) && !all.contains(QLatin1String("jo:head-end")) && !all.contains(QLatin1String("jo:tail-end")), qPrintable(all));
        const QString c = standardContentOf(files);
        wml::XmlTree t;
        QVERIFY(t.parse(c.toUtf8(), nullptr));
        const int shape = findElement(t, "custom-shape");
        QVERIFY2(shape >= 0, qPrintable(c));
        QVERIFY2(t.attrAny(shape, "x").isEmpty() && !t.attrAny(shape, "transform").isEmpty(), qPrintable(c));
        // The turn in radians against the clock, about the box's top left, then the move: the box's center is at (1 + 1, 0.5 + 0.5) inches.
        const TurnedBox b = turnedBoxOf(t.attrAny(shape, "transform"), 2.0, 1.0);
        QVERIFY2(b.ok && qAbs(b.centerX - 2.0) < 0.002 && qAbs(b.centerY - 1.0) < 0.002, qPrintable(t.attrAny(shape, "transform")));
        QVERIFY2(qAbs(b.angle + 30.0 * M_PI / 180.0) < 1e-5, qPrintable(t.attrAny(shape, "transform")));
        // The ends are marker styles that the common styles define, named by the shape's style.
        const QMap<QString, QString> props = graphicPropertiesOf(t, t.attrAny(shape, "style-name"));
        const QString start = props.value(QStringLiteral("marker-start")), end = props.value(QStringLiteral("marker-end"));
        QVERIFY2(!start.isEmpty() && !end.isEmpty() && start != end, qPrintable(all));
        const QString styles = QString::fromUtf8(files.value(QStringLiteral("styles.xml")));
        QVERIFY2(styles.contains(QLatin1String("<draw:marker draw:name=\"") + start + QLatin1String("\"")) && styles.contains(QLatin1String("<draw:marker draw:name=\"") + end + QLatin1String("\"")), qPrintable(styles));
        // And back: the same turn and ends, and the box where it was set (within half a point).
        wml::XmlTree d;
        QVERIFY(d.parse(part(docxOf(odt), QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
        QCOMPARE(d.attr(findElement(d, "xfrm"), "rot"), QStringLiteral("1800000"));
        QCOMPARE(d.attr(findElement(d, "headEnd"), "type"), QStringLiteral("triangle"));
        QCOMPARE(d.attr(findElement(d, "tailEnd"), "type"), QStringLiteral("oval"));
        const ShapeInDocx back = firstShapeOfDocx(odt);
        QVERIFY2(qAbs(back.left * 72.0 - 72.0) < 0.5 && qAbs(back.top * 72.0 - 36.0) < 0.5, qPrintable(QStringLiteral("%1, %2").arg(back.left * 72.0).arg(back.top * 72.0)));
    }

    // A turn that a file says in radians or in degrees, with or without a unit, is read right; a matrix of a turn and a move keeps both; a stretch or a slant is not kept (the shape is unturned, and the notes say so).
    void aTurnIsReadInRadiansOrDegreesAndMatricesAreReadToo()
    {
        struct Case {
            const char *transform;
            double turn;   // degrees with the clock; the box is 2 by 1 inches
            bool noted;
        };
        const double s = std::sin(M_PI / 6), c = std::cos(M_PI / 6);
        const QString pureMatrix = QStringLiteral("matrix(%1 %2 %3 %4 3in 2in)").arg(c, 0, 'f', 7).arg(s, 0, 'f', 7).arg(-s, 0, 'f', 7).arg(c, 0, 'f', 7);
        const QByteArray pm = pureMatrix.toUtf8();
        const Case cases[] = {{"rotate(-0.5235988) translate(1in 1in)", 30, false},
                              {"rotate(-30deg) translate(1in 1in)", 30, false},
                              {"rotate(-30) translate(1in 1in)", 30, false},
                              {"rotate(-0.5) translate(1in 1in)", 0.5 * 180.0 / M_PI, false},
                              {"rotate(-33.3333grad) translate(1in 1in)", 30, false},
                              {pm.constData(), 30, false},
                              {"matrix(2 0 0 2 1in 1in)", 0, true},
                              {"skewX(10deg) translate(1in 1in)", 0, true}};
        for (const Case &k : cases) {
            Made m;
            m.body = QStringLiteral("<text:p><draw:custom-shape text:anchor-type=\"paragraph\" svg:width=\"2in\" svg:height=\"1in\" draw:transform=\"%1\"><draw:enhanced-geometry draw:type=\"ooxml-rect\"/></draw:custom-shape></text:p>")
                         .arg(QString::fromLatin1(k.transform)).toUtf8();
            QStringList notes;
            const QByteArray docx = docxOf(m.build(), &notes);
            wml::XmlTree d;
            QVERIFY(d.parse(part(docx, QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
            const int xfrm = findElement(d, "xfrm");
            QVERIFY2(xfrm >= 0, k.transform);
            QVERIFY2(qAbs(d.attr(xfrm, "rot").toDouble() / 60000.0 - k.turn) < 0.01, qPrintable(QStringLiteral("%1: turn %2").arg(QString::fromLatin1(k.transform), d.attr(xfrm, "rot"))));
            QCOMPARE(notes.join(QLatin1Char('\n')).contains(QLatin1String("slanted or stretched")), k.noted);
            QVERIFY2(findElement(d, "posOffset") >= 0, k.transform);
        }
        // A matrix of a turn and a move: the center of the box is where the matrix takes it, and the box comes back there.
        Made m;
        m.body = QStringLiteral("<text:p><draw:custom-shape text:anchor-type=\"paragraph\" svg:width=\"2in\" svg:height=\"1in\" draw:transform=\"%1\"><draw:enhanced-geometry draw:type=\"ooxml-rect\"/></draw:custom-shape></text:p>").arg(pureMatrix).toUtf8();
        wml::XmlTree d;
        QVERIFY(d.parse(part(docxOf(m.build()), QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
        const double cx = c * 1.0 - s * 0.5 + 3.0, cy = s * 1.0 + c * 0.5 + 2.0;   // the center (1, 0.5) turned with the clock, then moved by (3, 2)
        double left = -1e9, top = -1e9;
        for (int i = findElement(d, "posOffset"); i >= 0; i = findElement(d, "posOffset", i)) (d.localName(d.parent(i)) == "positionH" ? left : top) = d.text(i).toDouble() / 914400.0;
        QVERIFY2(qAbs(left + 1.0 - cx) < 0.01 && qAbs(top + 0.5 - cy) < 0.01, qPrintable(QStringLiteral("%1, %2 for %3, %4").arg(left).arg(top).arg(cx).arg(cy)));
    }

    // A frame that says no anchor is anchored to its paragraph (19.759), or as its graphic style says; it is not set in the line.
    void aFrameWithNoAnchorIsAnchoredToItsParagraph()
    {
        for (const QString &how : {QStringLiteral("none"), QStringLiteral("style paragraph"), QStringLiteral("style as-char"), QStringLiteral("element as-char")}) {
            Made m;
            m.autoStyles = QStringLiteral("<style:style style:name=\"gr1\" style:family=\"graphic\"><style:graphic-properties draw:stroke=\"none\"%1/></style:style>")
                               .arg(how == QLatin1String("style paragraph") ? QStringLiteral(" text:anchor-type=\"paragraph\"") : how == QLatin1String("style as-char") ? QStringLiteral(" text:anchor-type=\"as-char\"") : QString()).toUtf8();
            m.body = QStringLiteral("<text:p>x<draw:custom-shape draw:style-name=\"gr1\"%1 svg:x=\"1in\" svg:y=\"1in\" svg:width=\"2in\" svg:height=\"1in\"><draw:enhanced-geometry draw:type=\"ooxml-rect\"/></draw:custom-shape></text:p>")
                         .arg(how == QLatin1String("element as-char") ? QStringLiteral(" text:anchor-type=\"as-char\"") : QString()).toUtf8();
            const QString doc = document(m);
            const bool inLine = how.endsWith(QLatin1String("as-char"));
            QCOMPARE(doc.contains(QLatin1String("<wp:inline")), inLine);
            QCOMPARE(doc.contains(QLatin1String("<wp:anchor")), !inLine);
        }
    }

    // ---- a written document, checked against the standard's schema (version 1.2, which is what the files say) when the machine has the schema file and a checker; else these tests skip

    // The check of a document, shared: the content and styles must have no error left once the names of ours are taken out, and what was taken out is only ours.
    static void mustBeValid(const QByteArray &odt, const char *what)
    {
        const OdfCheck r = checkAgainstSchema(odt);
        if (!r.ran) QSKIP(qPrintable(QStringLiteral("not checked: ") + r.skip));
        QVERIFY2(r.errors.isEmpty(), qPrintable(QString::fromLatin1(what) + QLatin1String(": ") + r.errors.mid(0, 6).join(QLatin1Char('\n'))));
        for (const QString &name : r.privateNames) QVERIFY2(name.startsWith(QLatin1String("{urn:jeffoffice:odf-extension}")), qPrintable(QString::fromLatin1(what) + QLatin1String(" has a name of another maker: ") + name));
    }

    void aPlainDocumentIsValidAgainstTheSchema()
    {
        const QByteArray odt = odtOfDrawing("<w:p><w:pPr><w:jc w:val=\"center\"/></w:pPr><w:r><w:rPr><w:b/></w:rPr><w:t>A heading</w:t></w:r></w:p><w:p><w:r><w:t>Some plain words, and more.</w:t></w:r></w:p>"
                                           "<w:p><w:r><w:t>Last</w:t></w:r></w:p>");
        mustBeValid(odt, "a plain document");
    }

    void aDocumentWithEachKindOfShapeIsValidAgainstTheSchema()
    {
        const QString link = QStringLiteral("http://127.0.0.1:9/p.png");
        const QByteArray vml = pictBody(vmlShape(QStringLiteral("rect"))) + pictBody(vmlShape(QStringLiteral("oval"))) + pictBody(vmlShape(QStringLiteral("box"))) + pictBody(vmlShape(QStringLiteral("picture"))) +
                               pictBody("<v:line style=\"position:absolute;margin-left:10pt;margin-top:10pt;width:100pt;height:0pt\" from=\"0pt,0pt\" to=\"100pt,0pt\" strokeweight=\"1pt\"><v:stroke startarrow=\"block\" endarrow=\"classic\"/></v:line>") +
                               pictBody("<v:group id=\"g\" style=\"width:200pt;height:100pt\" coordsize=\"200,100\"><v:rect style=\"position:absolute;left:0pt;top:0pt;width:100pt;height:50pt;z-index:251660288\" filled=\"t\" fillcolor=\"#ff0000\" stroked=\"f\"/>"
                                        "<v:oval style=\"position:absolute;left:100pt;top:50pt;width:100pt;height:50pt;z-index:251659264\" filled=\"t\" fillcolor=\"#0000ff\" stroked=\"f\"/></v:group>") +
                               pictBody(vmlShape(QStringLiteral("rect"), QStringLiteral("rotation:30;mso-position-horizontal-relative:char;mso-position-vertical-relative:line"), QStringLiteral("s3"))) +
                               pictBody("<v:rect id=\"in\" style=\"width:50pt;height:20pt\" filled=\"t\" fillcolor=\"#00ff00\" stroked=\"f\"/>");
        mustBeValid(odtOfDrawing(vml, link), "the older shapes");
        // The drawing form: a preset shape turned, with line ends and a description, in each place it can be set; a growing text box; a fixed one; a picture in the line.
        QByteArray dml;
        for (const QString &h : {QStringLiteral("page"), QStringLiteral("margin"), QStringLiteral("column"), QStringLiteral("character")})
            for (const QString &v : {QStringLiteral("page"), QStringLiteral("margin"), QStringLiteral("paragraph"), QStringLiteral("line")}) dml += anchoredShape(h, 635000, v, 317500, 30, true);
        dml += anchoredShape(QString(), 0, QString(), 0, 0, false, true);
        dml += linkedPictureBody();
        mustBeValid(odtOfDrawing(dml, link), "the drawing form");
    }

    void aDocumentWithATableIsValidAgainstTheSchema()
    {
        const QByteArray table = "<w:p><w:r><w:t>Before</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2500\"/><w:gridCol w:w=\"2500\"/></w:tblGrid>"
                                 "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2500\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>a</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2500\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>b</w:t></w:r></w:p></w:tc></w:tr>"
                                 "<w:tr><w:tc><w:p><w:r><w:t>c</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>d</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>After</w:t></w:r></w:p>";
        mustBeValid(odtOfDrawing(table), "a table");
    }

    void aDocumentWithMirroredPagesIsValidAgainstTheSchema()
    {
        const QByteArray odt = odtOfBodyWithSettings("<w:p><w:r><w:t>x</w:t></w:r></w:p><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>", "<w:mirrorMargins/>");
        QVERIFY(QString::fromUtf8(filesOf(odt).value(QStringLiteral("styles.xml"))).contains(QLatin1String("style:page-usage=\"mirrored\"")));
        mustBeValid(odt, "mirrored pages");
    }

};

QTEST_MAIN(OdtTests)
#include "tst_odt.moc"
