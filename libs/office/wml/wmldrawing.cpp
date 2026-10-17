// Reading the drawn objects of a run: VML (a w:pict) and DrawingML (a w:drawing). See wml.h.

#include "wml/wml.h"

#include "wml/xmltree.h"

#include <QHash>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

namespace jp::office::wml {

namespace {

// The most objects of one element that are read (a group of more is cut there), and how deep groups go.
constexpr int kMostShapes = 200;
constexpr int kMostGroupDepth = 8;

// A number from a file brought into a range a page could be in: one that is past it, or is not a number, is the nearest end of it (or 0).
double fin(double v) { return std::isfinite(v) ? std::clamp(v, -100000.0, 100000.0) : 0.0; }

// A length written as VML writes it (a number and a unit: pt, in, cm, mm, px, pc), in points; `def` for what isn't one. A bare number is points.
double lengthOf(const QString &text, double def)
{
    const QString v = text.trimmed().toLower();
    if (v.isEmpty()) return def;
    static const QRegularExpression re(QStringLiteral("^(-?[0-9]*\\.?[0-9]+(?:e-?[0-9]+)?)\\s*(pt|in|cm|mm|px|pc|emu)?$"));
    const QRegularExpressionMatch m = re.match(v);
    if (!m.hasMatch()) return def;
    const double n = fin(m.captured(1).toDouble());
    const QString u = m.captured(2);
    if (u == QLatin1String("in")) return fin(n * 72);
    if (u == QLatin1String("cm")) return fin(n * 72 / 2.54);
    if (u == QLatin1String("mm")) return fin(n * 72 / 25.4);
    if (u == QLatin1String("px")) return fin(n * 0.75);
    if (u == QLatin1String("pc")) return fin(n * 12);
    if (u == QLatin1String("emu")) return fin(n / 12700);
    return fin(n);
}

// The properties of a VML style ("position:absolute;width:10pt").
QHash<QString, QString> styleOf(const QString &style)
{
    QHash<QString, QString> out;
    for (const QString &part : style.split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
        const int colon = part.indexOf(QLatin1Char(':'));
        if (colon > 0) out.insert(part.left(colon).trimmed().toLower(), part.mid(colon + 1).trimmed());
    }
    return out;
}

// A VML color: a name or #RRGGBB (and the "[n]" or "fill" forms it is not). Empty for the rest.
QString colorOf(const QString &v)
{
    static const QHash<QString, QString> names{{"black", "000000"}, {"white", "FFFFFF"}, {"red", "FF0000"}, {"green", "008000"}, {"blue", "0000FF"}, {"yellow", "FFFF00"},
                                               {"silver", "C0C0C0"}, {"gray", "808080"}, {"grey", "808080"}, {"navy", "000080"}, {"maroon", "800000"}, {"purple", "800080"},
                                               {"teal", "008080"}, {"olive", "808000"}, {"lime", "00FF00"}, {"aqua", "00FFFF"}, {"fuchsia", "FF00FF"}, {"orange", "FFA500"}};
    QString c = v.trimmed().toLower();
    const int space = c.indexOf(QLatin1Char(' '));
    if (space > 0) c = c.left(space);
    if (c.startsWith(QLatin1Char('#'))) {
        c = c.mid(1);
        if (c.size() == 3) c = QString(c[0]) + c[0] + c[1] + c[1] + c[2] + c[2];
        static const QRegularExpression six(QStringLiteral("^[0-9a-f]{6}$"));
        return six.match(c).hasMatch() ? c.toUpper() : QString();
    }
    return names.value(c);
}

bool offOf(const QString &v) { return v == QLatin1String("f") || v == QLatin1String("false") || v == QLatin1String("0") || v == QLatin1String("no"); }

Shape::Rel relOf(const QString &v, Shape::Rel def)
{
    if (v == QLatin1String("margin")) return Shape::Margin;
    if (v == QLatin1String("inner-margin-area")) return Shape::InsideMargin;
    if (v == QLatin1String("outer-margin-area")) return Shape::OutsideMargin;
    if (v == QLatin1String("left-margin-area")) return Shape::LeftMargin;
    if (v == QLatin1String("right-margin-area")) return Shape::RightMargin;
    if (v == QLatin1String("top-margin-area")) return Shape::TopMargin;
    if (v == QLatin1String("bottom-margin-area")) return Shape::BottomMargin;
    if (v == QLatin1String("page")) return Shape::Page;
    if (v == QLatin1String("text") || v == QLatin1String("column")) return Shape::Column;
    if (v == QLatin1String("paragraph")) return Shape::Paragraph;
    if (v == QLatin1String("line")) return Shape::LineOf;
    if (v == QLatin1String("char") || v == QLatin1String("character")) return Shape::Character;
    return def;
}

Shape::Align alignOf(const QString &v)
{
    if (v == QLatin1String("left") || v == QLatin1String("top") || v == QLatin1String("inside")) return Shape::Start;
    if (v == QLatin1String("center")) return Shape::Center;
    if (v == QLatin1String("right") || v == QLatin1String("bottom") || v == QLatin1String("outside")) return Shape::End;
    return Shape::NoAlign;
}

// The side of a wrap that the text goes on (DrawingML's wrapText, VML's side): both unless it says another.
Shape::Side sideOf(const QString &v)
{
    if (v == QLatin1String("left")) return Shape::SideLeft;
    if (v == QLatin1String("right")) return Shape::SideRight;
    if (v == QLatin1String("largest")) return Shape::SideLargest;
    return Shape::SideBoth;
}

// The prefixes an element kept as written may use, declared (the tree of a part declares them on its root, and a slice of it is not given those).
QByteArray declarations()
{
    return "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
           "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" "
           "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
           "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" "
           "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" "
           "xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
}

// A text box's content as a document of its own: the paragraphs it holds under a w:txbxContent that declares the prefixes.
QByteArray textBoxXml(const XmlTree &t, int content) { return "<w:txbxContent " + declarations() + ">" + t.inner(content) + "</w:txbxContent>"; }

class Vml {
public:
    explicit Vml(const XmlTree &tree) : t(tree) {}
    const XmlTree &t;
    QVector<Shape> out;

    // A text box's content, the wrap, and a picture's data are children of the shape.
    void shape(int n, const QByteArray &kind, double sx, double sy, double ox, double oy, int depth)
    {
        if (out.size() >= kMostShapes) return;
        Shape s;
        const QHash<QString, QString> st = styleOf(t.attr(n, "style"));
        s.floating = st.value(QStringLiteral("position")) == QLatin1String("absolute");
        const double w = lengthOf(st.value(QStringLiteral("width")), 0) * sx, h = lengthOf(st.value(QStringLiteral("height")), 0) * sy;
        s.width = std::clamp(w, 0.0, 10000.0);
        s.height = std::clamp(h, 0.0, 10000.0);
        // Where: left/top are the position in a group or where the shape is among the text; margin-left/margin-top from the anchor.
        auto at = [&](const char *a, const char *b) {
            return st.contains(QString::fromLatin1(a)) ? lengthOf(st.value(QString::fromLatin1(a)), 0) : lengthOf(st.value(QString::fromLatin1(b)), 0);
        };
        s.left = at("margin-left", "left") * sx + ox;
        s.top = at("margin-top", "top") * sy + oy;
        s.hRel = relOf(st.value(QStringLiteral("mso-position-horizontal-relative")), Shape::Column);
        s.vRel = relOf(st.value(QStringLiteral("mso-position-vertical-relative")), Shape::Paragraph);
        const QString hp = st.value(QStringLiteral("mso-position-horizontal")), vp = st.value(QStringLiteral("mso-position-vertical"));
        s.hAlign = alignOf(hp);
        s.vAlign = alignOf(vp);
        if (hp == QLatin1String("absolute")) s.hAlign = Shape::NoAlign;
        if (vp == QLatin1String("absolute")) s.vAlign = Shape::NoAlign;
        s.z = int(std::clamp<qint64>(st.value(QStringLiteral("z-index")).toLongLong(), -2000000000, 2000000000));
        s.behind = s.z < 0;
        s.rotation = st.value(QStringLiteral("rotation")).toDouble();
        const QString flip = st.value(QStringLiteral("flip"));
        s.flipH = flip.contains(QLatin1Char('x'));
        s.flipV = flip.contains(QLatin1Char('y'));
        // Fill and line: a shape is filled unless it says not, and has a line unless it says not (a picture has neither).
        const bool picture = t.child(n, "imagedata", otherNs(n, "imagedata")) >= 0;
        s.kind = kind == "line" ? Shape::Line : kind == "oval" ? Shape::Oval : Shape::Rect;
        s.filled = !offOf(t.attr(n, "filled")) && !picture;
        s.stroked = !offOf(t.attr(n, "stroked")) && !picture;
        s.fillColor = colorOf(t.attr(n, "fillcolor"));
        if (s.fillColor.isEmpty() && s.filled) s.fillColor = QStringLiteral("FFFFFF");
        s.strokeColor = colorOf(t.attr(n, "strokecolor"));
        if (s.strokeColor.isEmpty()) s.strokeColor = QStringLiteral("000000");
        s.strokeWeight = std::clamp(lengthOf(t.attr(n, "strokeweight"), 0.75), 0.0, 100.0);
        s.alt = t.attr(n, "alt");
        if (kind == "line") {
            auto point = [&](const QString &v, double *x, double *y) {
                const QStringList f = v.split(QLatin1Char(','));
                if (f.size() == 2) {
                    *x = lengthOf(f[0], 0) * sx + ox;
                    *y = lengthOf(f[1], 0) * sy + oy;
                }
            };
            point(t.attr(n, "from"), &s.x1, &s.y1);
            point(t.attr(n, "to"), &s.x2, &s.y2);
            s.width = std::abs(s.x2 - s.x1);
            s.height = std::abs(s.y2 - s.y1);
            s.stroked = true;
        }
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (t.ns(c) == NsW) continue;
            const QByteArray name = t.localName(c);
            if (name == "imagedata") {
                s.kind = Shape::Picture;
                s.embed = t.attr(c, "id", NsR);
                if (s.embed.isEmpty()) s.embed = t.attr(c, "pict", NsR);
            } else if (name == "wrap") {
                const QString type = t.attr(c, "type");
                s.wrap = type == QLatin1String("square") ? Shape::Square : type == QLatin1String("tight") || type == QLatin1String("through") ? Shape::Tight
                         : type == QLatin1String("topAndBottom") ? Shape::TopBottom : Shape::WrapNone;
                if (!s.floating) s.wrap = Shape::WrapNone;
                s.side = sideOf(t.attr(c, "side"));
            } else if (name == "fill") {
                if (offOf(t.attr(c, "on"))) s.filled = false;
                if (const QString color = colorOf(t.attr(c, "color")); !color.isEmpty() && t.attr(n, "fillcolor").isEmpty()) s.fillColor = color;
            } else if (name == "stroke") {
                if (offOf(t.attr(c, "on"))) s.stroked = false;
                if (const QString color = colorOf(t.attr(c, "color")); !color.isEmpty()) s.strokeColor = color;
                s.arrowEnd = !t.attr(c, "endarrow").isEmpty() && t.attr(c, "endarrow") != QLatin1String("none");
                s.arrowStart = !t.attr(c, "startarrow").isEmpty() && t.attr(c, "startarrow") != QLatin1String("none");
            } else if (name == "textbox") {
                const int content = t.child(c, "txbxContent");
                if (content >= 0) {
                    s.textBox = textBoxXml(t, content);
                    if (s.kind != Shape::Picture) s.kind = Shape::TextBox;
                }
                const QStringList in = t.attr(c, "inset").split(QLatin1Char(','));
                if (!t.attr(c, "inset").isEmpty())
                    for (int i = 0; i < 4 && i < in.size(); ++i)
                        if (!in[i].trimmed().isEmpty()) s.inset[i] = lengthOf(in[i], s.inset[i]);
            }
        }
        // (A rule or box wrapped in a "wrap" with anchors for the text it follows is read as floating: a shape with a z-index is set so.)
        if (!s.floating) s.wrap = Shape::WrapNone;
        s.gapLeft = lengthOf(st.value(QStringLiteral("mso-wrap-distance-left")), 9);
        s.gapRight = lengthOf(st.value(QStringLiteral("mso-wrap-distance-right")), 9);
        s.gapTop = lengthOf(st.value(QStringLiteral("mso-wrap-distance-top")), 0);
        s.gapBottom = lengthOf(st.value(QStringLiteral("mso-wrap-distance-bottom")), 0);
        for (double *v : {&s.left, &s.top, &s.x1, &s.y1, &s.x2, &s.y2, &s.rotation, &s.gapLeft, &s.gapRight, &s.gapTop, &s.gapBottom}) *v = fin(*v);
        s.width = std::clamp(fin(s.width), 0.0, 10000.0);
        s.height = std::clamp(fin(s.height), 0.0, 10000.0);
        out << s;
        Q_UNUSED(depth);
    }

    void walk(int n, double sx, double sy, double ox, double oy, int depth)
    {
        for (int c = t.firstChild(n); c >= 0 && out.size() < kMostShapes; c = t.next(c)) {
            if (t.ns(c) == NsW) continue;
            const QByteArray name = t.localName(c);
            if (name == "shape" || name == "rect" || name == "roundrect" || name == "oval" || name == "line") {
                shape(c, name == "roundrect" ? QByteArray("rect") : name, sx, sy, ox, oy, depth);
            } else if (name == "group" && depth < kMostGroupDepth) {
                const QHash<QString, QString> st = styleOf(t.attr(c, "style"));
                const QStringList size = t.attr(c, "coordsize").split(QLatin1Char(',')), origin = t.attr(c, "coordorigin").split(QLatin1Char(','));
                const double gw = lengthOf(st.value(QStringLiteral("width")), 0), gh = lengthOf(st.value(QStringLiteral("height")), 0);
                double cw = size.size() == 2 ? fin(size[0].toDouble()) : 0, ch = size.size() == 2 ? fin(size[1].toDouble()) : 0;
                const double kx = cw > 0 && gw > 0 ? gw / cw : 1, ky = ch > 0 && gh > 0 ? gh / ch : 1;
                const double gx = lengthOf(st.contains(QStringLiteral("margin-left")) ? st.value(QStringLiteral("margin-left")) : st.value(QStringLiteral("left")), 0) * sx + ox;
                const double gy = lengthOf(st.contains(QStringLiteral("margin-top")) ? st.value(QStringLiteral("margin-top")) : st.value(QStringLiteral("top")), 0) * sy + oy;
                const double originX = origin.size() == 2 ? fin(origin[0].toDouble()) : 0, originY = origin.size() == 2 ? fin(origin[1].toDouble()) : 0;
                // The members' places are in the group's own units: scaled, from its origin, and from where the group is.
                walkMembers(c, fin(kx * sx), fin(ky * sy), fin(gx - originX * kx * sx), fin(gy - originY * ky * sy), depth + 1);
            } else if (name == "shapetype") {
                continue;
            } else {
                walk(c, sx, sy, ox, oy, depth);   // (a wrapper such as an OLE object, with a shape inside)
            }
        }
    }

    // The members of a group: their styles are in its units (a number without a unit is one of them).
    void walkMembers(int group, double kx, double ky, double ox, double oy, int depth)
    {
        for (int c = t.firstChild(group); c >= 0 && out.size() < kMostShapes; c = t.next(c)) {
            if (t.ns(c) == NsW) continue;
            const QByteArray name = t.localName(c);
            if (name == "shape" || name == "rect" || name == "roundrect" || name == "oval" || name == "line") shape(c, name == "roundrect" ? QByteArray("rect") : name, kx, ky, ox, oy, depth);
            else if (name == "group" && depth < kMostGroupDepth) walk(group, kx, ky, ox, oy, depth);
        }
    }

    int otherNs(int n, const char *local) const
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c))
            if (t.ns(c) != NsW && t.localName(c) == local) return t.ns(c);
        return NsW;
    }
};

// DrawingML: wp:inline and wp:anchor. Lengths are EMU (12,700 to the point).
double emu(const QString &v) { return fin(v.toDouble() / 12700.0); }

void readDrawing(const XmlTree &t, int drawing, QVector<Shape> *out)
{
    for (int c = t.firstChild(drawing); c >= 0; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if (name != "inline" && name != "anchor") continue;
        Shape s;
        s.floating = name == "anchor";
        s.kind = Shape::Picture;
        s.behind = t.attr(c, "behindDoc") == QLatin1String("1") || t.attr(c, "behindDoc") == QLatin1String("true");
        s.gapLeft = emu(t.attr(c, "distL"));
        s.gapRight = emu(t.attr(c, "distR"));
        s.gapTop = emu(t.attr(c, "distT"));
        s.gapBottom = emu(t.attr(c, "distB"));
        for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
            const QByteArray tag = t.localName(k);
            if (tag == "extent") {
                s.width = std::clamp(emu(t.attr(k, "cx")), 0.0, 10000.0);
                s.height = std::clamp(emu(t.attr(k, "cy")), 0.0, 10000.0);
            } else if (tag == "positionH" || tag == "positionV") {
                const bool horizontal = tag == "positionH";
                // (The strips: leftMargin and rightMargin across, topMargin and bottomMargin down; inside and outside either way.)
                const QString from = t.attr(k, "relativeFrom");
                static const QHash<QString, Shape::Rel> strips{{QStringLiteral("leftMargin"), Shape::LeftMargin}, {QStringLiteral("rightMargin"), Shape::RightMargin},
                                                              {QStringLiteral("topMargin"), Shape::TopMargin}, {QStringLiteral("bottomMargin"), Shape::BottomMargin},
                                                              {QStringLiteral("insideMargin"), Shape::InsideMargin}, {QStringLiteral("outsideMargin"), Shape::OutsideMargin}};
                const Shape::Rel rel = strips.contains(from) ? strips.value(from) : relOf(from.toLower(), horizontal ? Shape::Column : Shape::Paragraph);
                (horizontal ? s.hRel : s.vRel) = rel;
                for (int p = t.firstChild(k); p >= 0; p = t.next(p)) {
                    if (t.localName(p) == "posOffset") (horizontal ? s.left : s.top) = emu(t.text(p).trimmed());
                    else if (t.localName(p) == "align") (horizontal ? s.hAlign : s.vAlign) = alignOf(t.text(p).trimmed());
                }
            } else if (tag == "wrapSquare" || tag == "wrapTight" || tag == "wrapThrough" || tag == "wrapTopAndBottom") {
                s.wrap = tag == "wrapSquare" ? Shape::Square : tag == "wrapTopAndBottom" ? Shape::TopBottom : Shape::Tight;
                s.side = sideOf(t.attr(k, "wrapText"));
                // The wrap's own distances come before the object's.
                if (t.has(k, "distL")) s.gapLeft = emu(t.attr(k, "distL"));
                if (t.has(k, "distR")) s.gapRight = emu(t.attr(k, "distR"));
                if (t.has(k, "distT")) s.gapTop = emu(t.attr(k, "distT"));
                if (t.has(k, "distB")) s.gapBottom = emu(t.attr(k, "distB"));
            } else if (tag == "docPr") {
                s.alt = t.attr(k, "descr");
            }
        }
        if (!s.floating) s.wrap = Shape::WrapNone;
        // The picture's data and a text box's text: found anywhere inside.
        std::vector<int> stack{c};
        while (!stack.empty() && s.embed.isEmpty() && s.textBox.isEmpty()) {
            const int n = stack.back();
            stack.pop_back();
            if (t.localName(n) == "blip" && s.embed.isEmpty()) s.embed = t.attr(n, "embed", NsR);
            if (t.localName(n) == "txbxContent" && s.textBox.isEmpty()) {
                s.textBox = textBoxXml(t, n);
                s.kind = Shape::TextBox;
            }
            for (int k = t.firstChild(n); k >= 0; k = t.next(k)) stack.push_back(k);
            if (stack.size() > 4000) break;
        }
        if (s.kind == Shape::Picture && s.embed.isEmpty()) continue;   // (a chart or other graphic: no picture of it)
        out->push_back(s);
        if (out->size() >= kMostShapes) return;
    }
}

} // namespace

QVector<Shape> readShapes(const QByteArray &xml)
{
    QVector<Shape> out;
    // (An element kept as written has no declarations of the prefixes it uses: the usual ones are given on a root of their own.)
    static const QByteArray open = "<x " + declarations() + ">";
    XmlTree tree;
    if (!tree.parse(open + xml + "</x>", nullptr)) return out;
    const int root = tree.firstChild(tree.root());
    if (root < 0) return out;
    const QByteArray name = tree.localName(root);
    if (name == "pict" || name == "object") {
        Vml vml(tree);
        vml.walk(root, 1, 1, 0, 0, 0);
        return vml.out;
    }
    if (name == "drawing") {
        readDrawing(tree, root, &out);
        return out;
    }
    if (name == "AlternateContent") {
        // The choice a reader of the newer form takes, or else the fallback in the older.
        for (const QByteArray &want : {QByteArray("Choice"), QByteArray("Fallback")})
            for (int c = tree.firstChild(root); c >= 0; c = tree.next(c)) {
                if (tree.localName(c) != want) continue;
                for (int k = tree.firstChild(c); k >= 0; k = tree.next(k)) {
                    if (tree.ns(k) == NsW && tree.localName(k) == "drawing") readDrawing(tree, k, &out);
                    else if (tree.ns(k) == NsW && (tree.localName(k) == "pict" || tree.localName(k) == "object")) {
                        Vml vml(tree);
                        vml.walk(k, 1, 1, 0, 0, 0);
                        out << vml.out;
                    }
                }
                if (!out.isEmpty()) return out;
            }
    }
    return out;
}

} // namespace jp::office::wml
