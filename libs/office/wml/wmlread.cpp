// Reading a .docx: the package into a wml::Document. See wml.h for what is understood and what is kept as written.

#include "wml/wml.h"

#include "wml/xmltree.h"
#include "core/fonts.h"

#include <QCoreApplication>
#include <QFontDatabase>
#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <atomic>

namespace jp::office::wml {

namespace {

// Containers nest only a few deep in real files (a hyperlink in a content control in a tracked insertion is three); past this a
// container is kept whole as one element instead of being read into.
constexpr int kMaxContainerDepth = 24;
// Sizes a file states are held to what a page or a ruler can have (a page is at most 44 inches, 63,360 twentieths of a point).
constexpr int kMostTwips = 1000000;

int clampTwips(qlonglong v) { return int(std::clamp<qlonglong>(v, -kMostTwips, kMostTwips)); }

std::optional<int> intOf(const XmlTree &t, int n, const char *attr = "val")
{
    const QString v = t.attr(n, attr);
    if (v.isEmpty()) return std::nullopt;
    bool ok = false;
    qlonglong x = v.toLongLong(&ok);
    if (!ok) {
        const double d = v.toDouble(&ok);   // ("36.5")
        if (!ok) return std::nullopt;
        x = qlonglong(d);
    }
    return clampTwips(x);
}

std::optional<bool> onOffOf(const XmlTree &t, int n)
{
    if (!t.has(n, "val")) return true;
    const QString v = t.attr(n, "val").toLower();
    return !(v == QLatin1String("0") || v == QLatin1String("false") || v == QLatin1String("off"));
}

bool onOffString(const QString &v) { return v == QLatin1String("1") || v == QLatin1String("true") || v == QLatin1String("on"); }

class Reader {
public:
    explicit Reader(const XmlTree &tree) : t(tree) {}
    const XmlTree &t;
    int footnotes = 0, endnotes = 0;
    // What the body holds so far; past the limits (wml.h) the rest is left out.
    int paragraphs = 0, inlines = 0;
    bool truncated = false;
    int standIns = 0;                 // the tables that are kept whole as XML, for want of room (or rows)
    bool tablesAsStandIns = false;    // a part whose tables are not laid out (a header, a footer, a note): they are stand-ins, as every table was

    // The text shown for a symbol (w:sym): where its font is a symbol font that this machine lacks, the picture it stands for; where the machine has the font, the
    // file's own character, in the font's range, which the font draws; otherwise (another font, or a picture not known) a box.
    QString symbolDisplay(const QString &font, const QString &charAttribute) const
    {
        const QString box(QChar(0x25A1));
        bool ok = false;
        const uint code = charAttribute.toUInt(&ok, 16);
        const SymbolPictures *pictures = ok ? symbolPicturesOf(font) : nullptr;
        if (!pictures) return box;
        auto cached = installed.constFind(font.toLower());
        if (cached == installed.constEnd()) cached = installed.insert(font.toLower(), QFontDatabase::hasFamily(font));
        if (*cached) return code <= 0xFFFF ? QString(QChar(char16_t(code))) : box;
        const QChar picture = pictures->of(code);
        return picture.isNull() ? box : QString(picture);
    }
    mutable QHash<QString, bool> installed;   // (whether the machine has a symbol font, by its name: asked once for each)

    RunProps rPr(int n) const
    {
        RunProps p;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) {
                p.extra.push_back({runChildRank(name), t.raw(c)});
            } else if (name == "rStyle") {
                p.style = t.val(c);
            } else if (name == "rFonts") {
                Fonts f;
                f.ascii = t.attr(c, "ascii");
                f.hAnsi = t.attr(c, "hAnsi");
                f.eastAsia = t.attr(c, "eastAsia");
                f.cs = t.attr(c, "cs");
                f.asciiTheme = t.attr(c, "asciiTheme");
                f.hAnsiTheme = t.attr(c, "hAnsiTheme");
                f.eastAsiaTheme = t.attr(c, "eastAsiaTheme");
                f.csTheme = t.attr(c, "cstheme");
                f.hint = t.attr(c, "hint");
                p.fonts = f;
            } else if (name == "b") {
                p.bold = onOffOf(t, c);
            } else if (name == "i") {
                p.italic = onOffOf(t, c);
            } else if (name == "caps") {
                p.caps = onOffOf(t, c);
            } else if (name == "smallCaps") {
                p.smallCaps = onOffOf(t, c);
            } else if (name == "strike") {
                p.strike = onOffOf(t, c);
            } else if (name == "dstrike") {
                p.dstrike = onOffOf(t, c);
            } else if (name == "vanish") {
                p.vanish = onOffOf(t, c);
            } else if (name == "noProof") {
                p.noProof = onOffOf(t, c);
            } else if (name == "color") {
                Color k;
                k.val = t.attr(c, "val");
                k.theme = t.attr(c, "themeColor");
                k.tint = t.attr(c, "themeTint");
                k.shade = t.attr(c, "themeShade");
                p.color = k;
            } else if (name == "spacing") {
                if (auto v = intOf(t, c)) p.spacing = *v;
            } else if (name == "position") {
                if (auto v = intOf(t, c)) p.position = *v;
            } else if (name == "kern") {
                if (auto v = intOf(t, c)) p.kern = std::max(0, *v);
            } else if (name == "sz") {
                if (auto v = intOf(t, c)) p.size = std::clamp(*v, 1, 4000);
            } else if (name == "highlight") {
                p.highlight = t.val(c);
            } else if (name == "u") {
                p.underline = Underline{t.val(c).isEmpty() ? QStringLiteral("single") : t.val(c), t.attr(c, "color"), t.otherAttributes(c, {"val", "color"})};
            } else if (name == "vertAlign") {
                p.vertAlign = t.val(c);
            } else {
                p.extra.push_back({runChildRank(name), t.raw(c)});
            }
        }
        return p;
    }

    SectionProps sectPr(int n) const
    {
        SectionProps s;
        s.attrs = t.otherAttributes(n, {});
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) {
                s.extra.push_back({sectionChildRank(name), t.raw(c)});
            } else if (name == "type") {
                s.type = t.val(c);
            } else if (name == "pgSz") {
                s.hasPgSz = true;
                s.pageW = std::clamp(intOf(t, c, "w").value_or(12240), 720, 63360);
                s.pageH = std::clamp(intOf(t, c, "h").value_or(15840), 720, 63360);
                s.orient = t.attr(c, "orient");
                s.pgSzAttrs = t.otherAttributes(c, {"w", "h", "orient"});
            } else if (name == "pgMar") {
                s.hasPgMar = true;
                auto m = [&](const char *a, int def) { return std::clamp(intOf(t, c, a).value_or(def), 0, 63360); };
                s.top = std::clamp(intOf(t, c, "top").value_or(1440), -63360, 63360);
                s.bottom = std::clamp(intOf(t, c, "bottom").value_or(1440), -63360, 63360);
                s.left = m("left", 1440);
                s.right = m("right", 1440);
                s.header = m("header", 720);
                s.footer = m("footer", 720);
                s.gutter = m("gutter", 0);
                s.pgMarAttrs = t.otherAttributes(c, {"top", "bottom", "left", "right", "header", "footer", "gutter"});
            } else if (name == "pgNumType") {
                s.pageFormat = t.attr(c, "fmt");
                s.pgNumAttrs = t.otherAttributes(c, {"fmt", "start"});
                if (const std::optional<int> start = intOf(t, c, "start")) s.pageStart = std::clamp(*start, 0, 100000);
            } else if (name == "cols") {
                s.hasCols = true;
                s.cols = std::clamp(intOf(t, c, "num").value_or(1), 1, 45);
                s.colSpace = std::clamp(intOf(t, c, "space").value_or(720), 0, 63360);
                s.colSep = onOffString(t.attr(c, "sep"));
                s.colEqual = !t.has(c, "equalWidth") || onOffString(t.attr(c, "equalWidth"));
                s.colsAttrs = t.otherAttributes(c, {"num", "space", "sep", "equalWidth"});
                for (int k = t.firstChild(c); k >= 0 && s.colDefs.size() < 45; k = t.next(k))
                    if (t.is(k, "col")) s.colDefs.push_back({std::clamp(intOf(t, k, "w").value_or(0), 0, 63360), std::clamp(intOf(t, k, "space").value_or(0), 0, 63360)});
            } else if (name == "titlePg") {
                s.titlePg = onOffOf(t, c).value_or(true);
            } else {
                s.extra.push_back({sectionChildRank(name), t.raw(c)});
            }
        }
        return s;
    }

    ParaProps pPr(int n) const
    {
        ParaProps p;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) {
                p.extra.push_back({paraChildRank(name), t.raw(c)});
            } else if (name == "pStyle") {
                p.style = t.val(c);
            } else if (name == "keepNext") {
                p.keepNext = onOffOf(t, c);
            } else if (name == "keepLines") {
                p.keepLines = onOffOf(t, c);
            } else if (name == "pageBreakBefore") {
                p.pageBreakBefore = onOffOf(t, c);
            } else if (name == "widowControl") {
                p.widowControl = onOffOf(t, c);
            } else if (name == "contextualSpacing") {
                p.contextualSpacing = onOffOf(t, c);
            } else if (name == "numPr") {
                for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                    if (t.is(k, "ilvl")) p.ilvl = std::clamp(intOf(t, k).value_or(0), 0, 8);
                    else if (t.is(k, "numId")) p.numId = std::max(0, intOf(t, k).value_or(0));
                }
            } else if (name == "tabs") {
                QVector<TabStop> tabs;
                for (int k = t.firstChild(c); k >= 0 && tabs.size() < 200; k = t.next(k))
                    if (t.is(k, "tab")) tabs.push_back({intOf(t, k, "pos").value_or(0), t.attr(k, "val").isEmpty() ? QStringLiteral("left") : t.attr(k, "val"), t.attr(k, "leader")});
                p.tabs = tabs;
            } else if (name == "spacing") {
                p.before = intOf(t, c, "before");
                p.after = intOf(t, c, "after");
                if (p.before || t.has(c, "beforeAutospacing")) p.beforeAuto = onOffString(t.attr(c, "beforeAutospacing"));
                if (p.after || t.has(c, "afterAutospacing")) p.afterAuto = onOffString(t.attr(c, "afterAutospacing"));
                p.line = intOf(t, c, "line");
                p.lineRule = t.attr(c, "lineRule");
                p.spacingAttrs = t.otherAttributes(c, {"before", "after", "line", "lineRule"});
            } else if (name == "ind") {
                p.indLeft = intOf(t, c, "left");
                if (!p.indLeft) p.indLeft = intOf(t, c, "start");
                p.indRight = intOf(t, c, "right");
                if (!p.indRight) p.indRight = intOf(t, c, "end");
                // (A hanging indent, even of none, takes the place of the first line's that a style gave, and the other way about.)
                const auto first = intOf(t, c, "firstLine"), hang = intOf(t, c, "hanging");
                if (hang) p.indFirst = -*hang;
                else if (first) p.indFirst = *first;
                p.indAttrs = t.otherAttributes(c, {"left", "right", "start", "end", "firstLine", "hanging"});
            } else if (name == "jc") {
                p.jc = t.val(c);
            } else if (name == "outlineLvl") {
                p.outlineLvl = std::clamp(intOf(t, c).value_or(9), 0, 9);
            } else if (name == "rPr") {
                p.markRun = rPr(c);
            } else if (name == "sectPr") {
                p.section = std::make_shared<SectionProps>(sectPr(c));
            } else {
                if (name == "pBdr") p.borders = borders(c);
                else if (name == "shd") p.shading = Shading{t.attr(c, "val"), t.attr(c, "color"), t.attr(c, "fill")};
                else if (name == "overflowPunct") p.overflowPunct = onOffOf(t, c);
                p.extra.push_back({paraChildRank(name), t.raw(c)});   // (borders and shading are kept as written as well)
            }
        }
        return p;
    }

    // The sides of a paragraph's borders: none of the bar or the box a frame has.
    ParaBorders borders(int n) const
    {
        ParaBorders b;
        auto side = [&](int c) {
            Border x;
            x.val = t.attr(c, "val");
            x.size = std::clamp(intOf(t, c, "sz").value_or(0), 0, 96);
            x.space = std::clamp(intOf(t, c, "space").value_or(0), 0, 31);
            x.color = t.attr(c, "color");
            return x;
        };
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (t.is(c, "top")) b.top = side(c);
            else if (t.is(c, "left") || t.is(c, "start")) b.left = side(c);
            else if (t.is(c, "bottom")) b.bottom = side(c);
            else if (t.is(c, "right") || t.is(c, "end")) b.right = side(c);
            else if (t.is(c, "between")) b.between = side(c);
        }
        return b;
    }

    // ---- inside a paragraph

    Inline atom(int c, Inline::Kind kind) const
    {
        Inline a;
        a.kind = kind;
        a.raw = t.raw(c);
        a.tag = QString::fromLatin1(t.localName(c));
        a.key = t.attr(c, "id");
        return a;
    }

    void run(int n, QVector<Inline> *out)
    {
        RunProps props;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c))
            if (t.is(c, "rPr")) props = rPr(c);
        QString text;
        auto flush = [&] {
            if (text.isEmpty()) return;
            Inline s;
            s.props = props;
            s.text = text;
            out->push_back(s);
            text.clear();
        };
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) {
                flush();
                Inline a = atom(c, Inline::RunAtom);
                a.props = props;
                out->push_back(a);
            } else if (name == "rPr" || name == "lastRenderedPageBreak") {
                continue;
            } else if (name == "t") {
                text += t.text(c);
            } else if (name == "tab") {
                text += QLatin1Char('\t');
            } else if (name == "cr") {
                text += QChar(QChar::LineSeparator);
            } else if (name == "noBreakHyphen") {
                text += QChar(0x2011);
            } else if (name == "softHyphen") {
                text += QChar(0x00AD);
            } else if (name == "br" && (t.attr(c, "type").isEmpty() || t.attr(c, "type") == QLatin1String("textWrapping")) && !t.has(c, "clear")) {
                text += QChar(QChar::LineSeparator);
            } else if (name == "br" && t.attr(c, "type") == QLatin1String("page")) {
                text += QChar(0x000C);   // (a page break is the character U+000C in text)
            } else {
                flush();
                Inline a = atom(c, Inline::RunAtom);
                a.props = props;
                if (name == "br" || name == "fldChar") a.key = t.attr(c, name == "br" ? "type" : "fldCharType");
                if (name == "footnoteReference") a.display = QString::number(++footnotes);
                else if (name == "endnoteReference") a.display = QString::number(++endnotes);
                else if (name == "sym") a.display = symbolDisplay(t.attr(c, "font"), t.attr(c, "char"));
                else if (name == "drawing") {
                    for (int k = t.firstChild(c); k >= 0; k = t.next(k))
                        if (t.localName(k) == "inline") a.display = QCoreApplication::translate("jp::office::wml", "[picture]");
                }
                out->push_back(a);
            }
        }
        flush();
    }

    // The elements that hold runs and paragraph content and may be read into, with their own start and end kept as tags.
    static bool isContainer(const QByteArray &name)
    {
        return name == "hyperlink" || name == "smartTag" || name == "ins" || name == "del" || name == "moveFrom" || name == "moveTo" || name == "fldSimple" ||
               name == "customXml" || name == "dir" || name == "bdo";
    }

    void paragraphContent(int n, QVector<Inline> *out, int depth)
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (inlines >= kMaxInlines) {
                truncated = true;
                return;
            }
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) {
                out->push_back(atom(c, Inline::ParaAtom));
            } else if (name == "pPr" || name == "proofErr") {
                continue;
            } else if (name == "r") {
                run(c, out);
            } else if (isContainer(name) && depth < kMaxContainerDepth && !t.isEmptyElement(c)) {
                Inline open;
                open.kind = Inline::ParaAtom;
                open.role = Inline::Open;
                open.raw = t.startTag(c);
                open.tag = QString::fromLatin1(name);
                open.key = t.attr(c, "id");   // (a revision mark's Id, which is kept one to a mark)
                Inline close = open;
                close.role = Inline::Close;
                close.raw = t.endTag(c);
                out->push_back(open);
                paragraphContent(c, out, depth + 1);
                out->push_back(close);
            } else if (name == "sdt" && depth < kMaxContainerDepth && t.child(c, "sdtContent") >= 0) {
                const int content = t.child(c, "sdtContent");
                Inline open;
                open.kind = Inline::ParaAtom;
                open.role = Inline::Open;
                open.tag = QStringLiteral("sdt");
                open.raw = t.startTag(c);
                for (int k = t.firstChild(c); k >= 0 && k != content; k = t.next(k)) open.raw += t.raw(k);
                open.raw += t.startTag(content);
                Inline close = open;
                close.role = Inline::Close;
                close.raw = t.endTag(content);
                for (int k = t.next(content); k >= 0; k = t.next(k)) close.raw += t.raw(k);
                close.raw += t.endTag(c);
                out->push_back(open);
                paragraphContent(content, out, depth + 1);
                out->push_back(close);
            } else {
                Inline a = atom(c, Inline::ParaAtom);
                out->push_back(a);
            }
        }
    }

    Paragraph paragraph(int n)
    {
        Paragraph p;
        p.attrs = t.otherAttributes(n, {});
        const int pr = t.child(n, "pPr");
        if (pr >= 0) p.props = pPr(pr);
        paragraphContent(n, &p.content, 0);
        inlines += int(p.content.size());
        return p;
    }

    // ---- tables

    int tableDepth = 0;               // the tables being read into (a table in a cell is one deeper)
    int tableCells = 0;               // the cells of the tables read so far

    TableWidth widthOf(int n) const
    {
        TableWidth w;
        w.type = t.attr(n, "type");
        QString v = t.attr(n, "w");
        if (v.endsWith(QLatin1Char('%'))) {
            v.chop(1);
            w.type = QStringLiteral("pct");
            w.w = clampTwips(qlonglong(v.toDouble() * 50));
            return w;
        }
        if (!v.isEmpty()) {
            bool ok = false;
            const double d = v.toDouble(&ok);
            if (ok) w.w = clampTwips(qlonglong(d));
            if (w.type.isEmpty()) w.type = QStringLiteral("dxa");
        }
        return w;
    }

    TableBorder borderOf(int n) const
    {
        TableBorder b;
        b.val = t.attr(n, "val");
        b.size = std::clamp(intOf(t, n, "sz").value_or(0), 0, 96);
        b.space = std::clamp(intOf(t, n, "space").value_or(0), 0, 31);
        b.color = t.attr(n, "color");
        return b;
    }

    // The sides of a w:tblBorders or a w:tcBorders (the strict form's start and end are the left and the right).
    void bordersOf(int n, TableMark *m) const
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (name == "top") m->top = borderOf(c);
            else if (name == "left" || name == "start") m->left = borderOf(c);
            else if (name == "bottom") m->bottom = borderOf(c);
            else if (name == "right" || name == "end") m->right = borderOf(c);
            else if (name == "insideH") m->insideH = borderOf(c);
            else if (name == "insideV") m->insideV = borderOf(c);
            else if (name == "tl2br") m->diagDown = borderOf(c);
            else if (name == "tr2bl") m->diagUp = borderOf(c);
        }
    }

    void marginsOf(int n, std::optional<TableWidth> *top, std::optional<TableWidth> *left, std::optional<TableWidth> *bottom, std::optional<TableWidth> *right) const
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (name == "top") *top = widthOf(c);
            else if (name == "left" || name == "start") *left = widthOf(c);
            else if (name == "bottom") *bottom = widthOf(c);
            else if (name == "right" || name == "end") *right = widthOf(c);
        }
    }

    void tblPr(int n, TableMark *m) const
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) continue;
            if (name == "tblStyle") {
                m->style = t.val(c);
            } else if (name == "tblW") {
                m->width = widthOf(c);
            } else if (name == "tblInd") {
                m->indent = widthOf(c);
            } else if (name == "tblCellSpacing") {
                m->spacing = widthOf(c);
            } else if (name == "jc") {
                m->jc = t.val(c);
            } else if (name == "tblLayout") {
                m->layout = t.attr(c, "type");
            } else if (name == "tblCellMar") {
                marginsOf(c, &m->marTop, &m->marLeft, &m->marBottom, &m->marRight);
            } else if (name == "tblBorders") {
                bordersOf(c, m);
            } else if (name == "shd") {
                m->shdVal = t.val(c);
                m->shdFill = t.attr(c, "fill");
                m->shdColor = t.attr(c, "color");
            } else if (name == "tblLook") {
                int look = 0;
                bool any = false;
                bool ok = false;
                const int hex = t.attr(c, "val").toInt(&ok, 16);
                if (ok) {
                    look = hex;
                    any = true;
                }
                static const struct { const char *name; int bit; } bits[] = {{"firstRow", 0x20}, {"lastRow", 0x40}, {"firstColumn", 0x80}, {"lastColumn", 0x100}, {"noHBand", 0x200}, {"noVBand", 0x400}};
                for (const auto &b : bits)
                    if (t.has(c, b.name)) {
                        any = true;
                        if (onOffString(t.attr(c, b.name).toLower())) look |= b.bit;
                        else look &= ~b.bit;
                    }
                if (any) m->look = look;
            } else if (name == "tblStyleRowBandSize") {
                m->rowBand = std::clamp(intOf(t, c).value_or(1), 1, 100);
            } else if (name == "tblStyleColBandSize") {
                m->colBand = std::clamp(intOf(t, c).value_or(1), 1, 100);
            } else if (name == "tblpPr") {
                m->floating = true;
            } else if (name == "bidiVisual") {
                m->bidi = onOffOf(t, c).value_or(true);
            }
        }
    }

    void tblGrid(int n, TableMark *m) const
    {
        for (int c = t.firstChild(n); c >= 0 && m->grid.size() < kMaxTableColumns; c = t.next(c))
            if (t.is(c, "gridCol")) m->grid.push_back(std::clamp(intOf(t, c, "w").value_or(0), 0, kMostTwips));
    }

    void trPr(int n, TableMark *m) const
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) continue;
            if (name == "gridBefore") {
                m->gridBefore = std::clamp(intOf(t, c).value_or(0), 0, kMaxTableColumns);
            } else if (name == "gridAfter") {
                m->gridAfter = std::clamp(intOf(t, c).value_or(0), 0, kMaxTableColumns);
            } else if (name == "wBefore") {
                m->widthBefore = widthOf(c);
            } else if (name == "wAfter") {
                m->widthAfter = widthOf(c);
            } else if (name == "cantSplit") {
                m->cantSplit = onOffOf(t, c).value_or(true);
            } else if (name == "tblHeader") {
                m->header = onOffOf(t, c).value_or(true);
            } else if (name == "hidden") {
                m->hidden = onOffOf(t, c).value_or(true);
            } else if (name == "trHeight") {
                m->height = std::clamp(intOf(t, c).value_or(0), 0, kMostTwips);
                m->heightRule = t.attr(c, "hRule");
            } else if (name == "tblCellSpacing") {
                m->spacing = widthOf(c);
            } else if (name == "jc") {
                m->jc = t.val(c);
            }
        }
    }

    void tcPr(int n, TableMark *m) const
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) continue;
            if (name == "tcW") {
                m->cellWidth = widthOf(c);
            } else if (name == "gridSpan") {
                m->span = std::clamp(intOf(t, c).value_or(1), 1, kMaxTableColumns);
            } else if (name == "vMerge") {
                m->vMerge = t.val(c).isEmpty() || t.val(c) == QLatin1String("continue") ? QStringLiteral("continue") : t.val(c);
            } else if (name == "tcBorders") {
                bordersOf(c, m);
            } else if (name == "shd") {
                m->shdVal = t.val(c);
                m->shdFill = t.attr(c, "fill");
                m->shdColor = t.attr(c, "color");
            } else if (name == "tcMar") {
                marginsOf(c, &m->cMarTop, &m->cMarLeft, &m->cMarBottom, &m->cMarRight);
            } else if (name == "vAlign") {
                m->vAlign = t.val(c);
            } else if (name == "noWrap") {
                m->noWrap = onOffOf(t, c).value_or(true);
            } else if (name == "textDirection") {
                m->textDirection = t.val(c);
            }
        }
    }

    // Whether a w:tbl is read as a table: it has rows and cells, no more than the limits (wml.h) hold, and is not nested deeper than they say.
    bool tableFits(int tbl) const
    {
        if (tablesAsStandIns || tableDepth >= kMaxTableDepth) return false;
        int rows = 0, cells = 0;
        for (int r = t.firstChild(tbl); r >= 0; r = t.next(r)) {
            if (!t.is(r, "tr")) continue;
            if (++rows > kMaxTableRows) return false;
            int inRow = 0;
            for (int cell = t.firstChild(r); cell >= 0; cell = t.next(cell))
                if (t.is(cell, "tc")) ++inRow;
            if (inRow == 0 || inRow > kMaxTableColumns) return false;
            cells += inRow;
            if (cells > kMaxTableCellsEach) return false;
        }
        const int grid = t.child(tbl, "tblGrid");
        if (grid >= 0) {
            int cols = 0;
            for (int c = t.firstChild(grid); c >= 0; c = t.next(c)) cols += t.is(c, "gridCol");
            if (cols > kMaxTableColumns) return false;
        }
        return rows > 0 && tableCells + cells <= kMaxTableCells;
    }

    static Block closeOf(const Block &open, const QByteArray &raw)
    {
        Block b;
        b.isRaw = true;
        b.role = Inline::Close;
        b.tag = open.tag;
        b.raw = raw;
        return b;
    }

    void cellBlocks(int tc, QVector<Block> *out, int depth)
    {
        Block open;
        open.isRaw = true;
        open.role = Inline::Open;
        open.tag = QStringLiteral("tc");
        open.raw = t.startTag(tc);
        auto mark = std::make_shared<TableMark>();
        mark->kind = TableMark::Cell;
        const int first = t.firstChild(tc);
        if (first >= 0 && t.is(first, "tcPr")) {
            tcPr(first, mark.get());
            open.raw += t.raw(first);
        }
        open.table = mark;
        Block close = closeOf(open, t.endTag(tc));
        out->push_back(std::move(open));
        const qsizetype from = out->size();
        ++tableDepth;
        bodyContent(tc, out, depth + 1, nullptr, nullptr, true);
        --tableDepth;
        // A cell holds a paragraph, or a table that is followed by one; one with neither (a damaged file's) is given an empty one, as a cell can't be shown without.
        bool content = false;
        for (qsizetype i = from; i < out->size() && !content; ++i) content = !(*out)[i].isRaw || ((*out)[i].role == Inline::Open && (*out)[i].tag == QLatin1String("tbl"));
        if (!content) {
            ++paragraphs;
            out->push_back(Block());
        }
        out->push_back(std::move(close));
    }

    void rowBlocks(int tr, QVector<Block> *out, int depth)
    {
        Block open;
        open.isRaw = true;
        open.role = Inline::Open;
        open.tag = QStringLiteral("tr");
        open.raw = t.startTag(tr);
        auto mark = std::make_shared<TableMark>();
        mark->kind = TableMark::Row;
        int c = t.firstChild(tr);
        for (; c >= 0 && !t.is(c, "tc"); c = t.next(c)) {
            if (t.is(c, "trPr")) trPr(c, mark.get());
            open.raw += t.raw(c);
        }
        open.table = mark;
        Block close = closeOf(open, t.endTag(tr));
        out->push_back(std::move(open));
        for (; c >= 0; c = t.next(c)) {
            if (t.is(c, "tc")) {
                ++tableCells;
                cellBlocks(c, out, depth);
            } else {
                out->push_back(rawBlock(c));
            }
        }
        out->push_back(std::move(close));
    }

    // A table as the blocks of its parts (see Block): false, with nothing added, when it is to be kept whole instead.
    bool tableBlocks(int tbl, QVector<Block> *out, int depth)
    {
        if (!tableFits(tbl)) {
            ++standIns;
            return false;
        }
        const qsizetype openAt = out->size();
        Block open;
        open.isRaw = true;
        open.role = Inline::Open;
        open.tag = QStringLiteral("tbl");
        open.raw = t.startTag(tbl);
        auto mark = std::make_shared<TableMark>();
        mark->kind = TableMark::Table;
        int c = t.firstChild(tbl);
        for (; c >= 0 && !t.is(c, "tr"); c = t.next(c)) {
            if (t.is(c, "tblPr")) tblPr(c, mark.get());
            if (t.is(c, "tblGrid")) {
                tblGrid(c, mark.get());
                mark->gridBegin = open.raw.size();
                open.raw += t.raw(c);
                mark->gridEnd = open.raw.size();
                continue;
            }
            open.raw += t.raw(c);
        }
        open.table = mark;
        out->push_back(open);
        for (; c >= 0; c = t.next(c)) {
            if (t.is(c, "tr")) rowBlocks(c, out, depth);
            else out->push_back(rawBlock(c));
        }
        out->push_back(closeOf(open, t.endTag(tbl)));
        // (What a table that nothing changes is written as, and how to tell.)
        mark->original = t.raw(tbl);
        mark->fingerprint = blocksFingerprint(*out, openAt, out->size());
        return true;
    }

    // ---- the body

    Block rawBlock(int c, const QString &display = QString())
    {
        Block b;
        b.isRaw = true;
        b.raw = t.raw(c);
        b.display = display;
        b.tag = QString::fromLatin1(t.localName(c));
        b.key = t.attr(c, "id");
        return b;
    }

    void bodyContent(int n, QVector<Block> *out, int depth, SectionProps *last, bool *lastStated, bool inCell = false)
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (inCell && t.is(c, "tcPr")) continue;   // (the cell's properties are in its Open block)
            if (paragraphs >= kMaxParagraphs || inlines >= kMaxInlines) {
                truncated = true;
                return;
            }
            const QByteArray name = t.localName(c);
            if (t.ns(c) != NsW) {
                out->push_back(rawBlock(c));
            } else if (name == "p") {
                ++paragraphs;
                Block b;
                b.para = paragraph(c);
                out->push_back(std::move(b));
            } else if (name == "sectPr") {
                if (last) {
                    *last = sectPr(c);
                    if (lastStated) *lastStated = true;
                }
            } else if (name == "tbl") {
                if (!tableBlocks(c, out, depth)) {
                    int rows = 0, cols = 0;
                    for (int r = t.firstChild(c); r >= 0; r = t.next(r)) {
                        if (!t.is(r, "tr")) continue;
                        if (++rows == 1)
                            for (int cell = t.firstChild(r); cell >= 0; cell = t.next(cell))
                                if (t.is(cell, "tc")) ++cols;
                    }
                    out->push_back(rawBlock(c, QCoreApplication::translate("jp::office::wml", "[Table: %1 rows by %2 columns]").arg(rows).arg(cols)));
                }
            } else if (name == "sdt" && depth < kMaxContainerDepth && t.child(c, "sdtContent") >= 0) {
                const int content = t.child(c, "sdtContent");
                Block open;
                open.isRaw = true;
                open.role = Inline::Open;
                open.tag = QStringLiteral("sdt");
                open.raw = t.startTag(c);
                for (int k = t.firstChild(c); k >= 0 && k != content; k = t.next(k)) open.raw += t.raw(k);
                open.raw += t.startTag(content);
                Block close = open;
                close.role = Inline::Close;
                close.raw = t.endTag(content);
                for (int k = t.next(content); k >= 0; k = t.next(k)) close.raw += t.raw(k);
                close.raw += t.endTag(c);
                out->push_back(open);
                bodyContent(content, out, depth + 1, nullptr, nullptr);
                out->push_back(close);
            } else if ((name == "customXml" || name == "ins" || name == "del" || name == "moveFrom" || name == "moveTo") && depth < kMaxContainerDepth && !t.isEmptyElement(c)) {
                Block open;
                open.isRaw = true;
                open.role = Inline::Open;
                open.tag = QString::fromLatin1(name);
                open.raw = t.startTag(c);
                open.key = t.attr(c, "id");
                Block close = open;
                close.role = Inline::Close;
                close.raw = t.endTag(c);
                out->push_back(open);
                bodyContent(c, out, depth + 1, nullptr, nullptr);
                out->push_back(close);
            } else {
                out->push_back(rawBlock(c));
            }
        }
    }

    // ---- styles and numbering

    void styles(StyleSheet *sheet)
    {
        const int root = t.root();
        sheet->rootStart = t.startTag(root);
        sheet->original = t.source();
        for (int c = t.firstChild(root); c >= 0; c = t.next(c)) {
            if (t.ns(c) != NsW) continue;
            if (t.is(c, "docDefaults")) {
                sheet->docDefaults = t.raw(c);
                for (int d = t.firstChild(c); d >= 0; d = t.next(d)) {
                    const int rp = t.is(d, "rPrDefault") ? t.child(d, "rPr") : -1;
                    const int pp = t.is(d, "pPrDefault") ? t.child(d, "pPr") : -1;
                    if (rp >= 0) sheet->defaultRun = rPr(rp);
                    if (pp >= 0) sheet->defaultPara = pPr(pp);
                }
            } else if (t.is(c, "latentStyles")) {
                sheet->latentStyles = t.raw(c);
            } else if (t.is(c, "style")) {
                Style s;
                s.id = t.attr(c, "styleId");
                if (s.id.isEmpty()) continue;
                s.type = t.attr(c, "type").isEmpty() ? QStringLiteral("paragraph") : t.attr(c, "type");
                s.isDefault = onOffString(t.attr(c, "default"));
                s.customStyle = onOffString(t.attr(c, "customStyle"));
                s.raw = t.raw(c);
                bool afterRun = false;
                for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                    const QByteArray name = t.localName(k);
                    if (name == "name") s.name = t.val(k);
                    else if (name == "basedOn") s.basedOn = t.val(k);
                    else if (name == "next") s.next = t.val(k);
                    else if (name == "link") s.link = t.val(k);
                    else if (name == "uiPriority") s.uiPriority = intOf(t, k);
                    else if (name == "semiHidden") s.semiHidden = onOffOf(t, k).value_or(true);
                    else if (name == "unhideWhenUsed") s.unhideWhenUsed = onOffOf(t, k).value_or(true);
                    else if (name == "qFormat") s.qFormat = onOffOf(t, k).value_or(true);
                    else if (name == "pPr") s.pPr = pPr(k);
                    else if (name == "rPr") { s.rPr = rPr(k); afterRun = true; }
                    else if (afterRun || name == "tblPr" || name == "trPr" || name == "tcPr" || name == "tblStylePr") s.tail += t.raw(k);
                }
                sheet->styles.push_back(s);
            }
        }
        sheet->changed();
    }

    Level level(int n) const
    {
        Level l;
        l.present = true;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (name == "start") l.start = intOf(t, c).value_or(1);
            else if (name == "numFmt") l.numFmt = t.val(c);
            else if (name == "lvlText") l.text = t.val(c);
            else if (name == "lvlJc") l.jc = t.val(c);
            else if (name == "suff") l.suff = t.val(c);
            else if (name == "pStyle") l.pStyle = t.val(c);
            else if (name == "isLgl") l.legal = onOffOf(t, c).value_or(true);
            else if (name == "legacy") l.legacy = onOffString(t.attr(c, "legacy").toLower());
            else if (name == "lvlRestart") l.restart = intOf(t, c);
            else if (name == "pPr") l.pPr = pPr(c);
            else if (name == "rPr") l.rPr = rPr(c);
        }
        return l;
    }

    void numbering(Numbering *num)
    {
        const int root = t.root();
        num->rootStart = t.startTag(root);
        num->original = t.source();
        for (int c = t.firstChild(root); c >= 0; c = t.next(c)) {
            if (t.ns(c) != NsW) continue;
            if (t.is(c, "numPicBullet")) {
                num->prefix += t.raw(c);
            } else if (t.is(c, "abstractNum")) {
                AbstractNum a;
                a.id = intOf(t, c, "abstractNumId").value_or(0);
                a.raw = t.raw(c);
                bool seenLevel = false;
                for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                    if (t.is(k, "lvl")) {
                        seenLevel = true;
                        const int ilvl = std::clamp(intOf(t, k, "ilvl").value_or(0), 0, 8);
                        a.levels[ilvl] = level(k);
                    } else if (t.is(k, "multiLevelType")) {
                        a.multiLevelType = t.val(k);
                        a.head += t.raw(k);
                    } else if (t.is(k, "styleLink") || t.is(k, "numStyleLink")) {
                        (t.is(k, "styleLink") ? a.styleLink : a.numStyleLink) = t.val(k);
                        a.head += t.raw(k);
                    } else if (seenLevel) {
                        a.tail += t.raw(k);
                    } else {
                        a.head += t.raw(k);
                    }
                }
                num->abstracts.push_back(a);
            } else if (t.is(c, "num")) {
                Num m;
                m.id = intOf(t, c, "numId").value_or(0);
                m.raw = t.raw(c);
                const int ab = t.child(c, "abstractNumId");
                if (ab >= 0) m.abstractId = intOf(t, ab).value_or(0);
                for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                    if (!t.is(k, "lvlOverride")) continue;
                    const int ilvl = std::clamp(intOf(t, k, "ilvl").value_or(0), 0, 8);
                    const int so = t.child(k, "startOverride");
                    if (so >= 0) m.startOverrides.insert(ilvl, intOf(t, so).value_or(1));
                    // (The level an id writes out itself is that id's, for the level the override names.)
                    const int own = t.child(k, "lvl");
                    if (own >= 0) m.levelOverrides.insert(ilvl, level(own));
                }
                num->nums.push_back(m);
            } else if (t.is(c, "numIdMacAtCleanup")) {
                num->suffix += t.raw(c);
            }
        }
    }
};

// A document's XML nested deeper than the XML reader goes (kMaxXmlDepth): the part with each element that starts below `limit` levels, and its
// nearest enclosing table (or, with none, the element at the limit), replaced by plain paragraphs holding the text that was in it (one for each of
// its paragraphs). What is above the limit is as it was. `changed` says whether anything was replaced. Nothing here recurses.
QByteArray flattenDeepXml(const QByteArray &xml, int limit, bool *changed)
{
    *changed = false;
    struct Open { qsizetype begin; bool table; };
    QVector<Open> stack;
    QByteArray out;
    qsizetype copied = 0, regionBegin = 0;
    int region = -1;
    QByteArray prefix;
    const qsizetype n = xml.size();
    auto textOf = [&](qsizetype from, qsizetype to) {
        // The paragraphs in xml[from, to): the text of each, as it is written.
        QByteArray paragraphs;
        QByteArray text;
        bool inParagraph = false;
        for (qsizetype i = xml.indexOf('<', from); i >= 0 && i < to; i = xml.indexOf('<', i + 1)) {
            qsizetype e = i + 1;
            const bool end = e < n && xml[e] == '/';
            if (end) ++e;
            const qsizetype nameAt = e;
            while (e < n && xml[e] != ' ' && xml[e] != '/' && xml[e] != '>' && xml[e] != '\r' && xml[e] != '\n' && xml[e] != '\t') ++e;
            const QByteArray name = xml.mid(nameAt, e - nameAt);
            const QByteArray local = name.mid(name.indexOf(':') + 1);
            if (local == "p" && !end) {
                inParagraph = true;
                text.clear();
            } else if (local == "p" && end && inParagraph) {
                if (!text.isEmpty()) paragraphs += "<" + prefix + "p><" + prefix + "r><" + prefix + "t xml:space=\"preserve\">" + text + "</" + prefix + "t></" + prefix + "r></" + prefix + "p>";
                inParagraph = false;
            } else if (local == "t" && !end && inParagraph) {
                const qsizetype close = xml.indexOf('>', e);
                if (close < 0 || xml[close - 1] == '/') continue;
                const qsizetype stop = xml.indexOf('<', close + 1);
                if (stop > close + 1) text += xml.mid(close + 1, stop - close - 1);
            }
        }
        return paragraphs;
    };
    for (qsizetype i = xml.indexOf('<'); i >= 0 && i < n; i = xml.indexOf('<', i + 1)) {
        if (i + 3 < n && xml[i + 1] == '!' && xml[i + 2] == '-' && xml[i + 3] == '-') {
            const qsizetype close = xml.indexOf("-->", i + 4);
            if (close < 0) break;
            i = close + 2;
            continue;
        }
        if (i + 8 < n && xml.mid(i, 9) == "<![CDATA[") {
            const qsizetype close = xml.indexOf("]]>", i + 9);
            if (close < 0) break;
            i = close + 2;
            continue;
        }
        if (i + 1 < n && xml[i + 1] == '?') {
            const qsizetype close = xml.indexOf("?>", i + 2);
            if (close < 0) break;
            i = close + 1;
            continue;
        }
        // The tag: to its '>' (a '>' in a quoted value does not end it).
        qsizetype e = i + 1;
        char quote = 0;
        for (; e < n; ++e) {
            if (quote) { if (xml[e] == quote) quote = 0; }
            else if (xml[e] == '"' || xml[e] == '\'') quote = xml[e];
            else if (xml[e] == '>') break;
        }
        if (e >= n) break;
        const bool end = xml[i + 1] == '/';
        const bool selfClosing = !end && xml[e - 1] == '/';
        qsizetype nameEnd = i + (end ? 2 : 1);
        while (nameEnd < e && xml[nameEnd] != ' ' && xml[nameEnd] != '/' && xml[nameEnd] != '\r' && xml[nameEnd] != '\n' && xml[nameEnd] != '\t') ++nameEnd;
        const QByteArray name = xml.mid(i + (end ? 2 : 1), nameEnd - i - (end ? 2 : 1));
        const QByteArray local = name.mid(name.indexOf(':') + 1);
        if (end) {
            if (!stack.isEmpty()) stack.removeLast();
            if (region >= 0 && stack.size() == region) {
                // The element that was to be replaced is closed: its text stands in its place.
                const qsizetype stop = e + 1;
                out += xml.mid(copied, regionBegin - copied) + textOf(regionBegin, stop);
                copied = stop;
                region = -1;
                *changed = true;
            }
        } else {
            if (region < 0 && stack.size() >= limit) {
                int at = limit - 1;
                for (int k = limit - 1; k >= 0; --k)
                    if (stack[k].table) {
                        at = k;
                        break;
                    }
                region = at;
                regionBegin = stack[at].begin;
                const qsizetype colon = xml.indexOf(':', regionBegin);
                const qsizetype nameStart = regionBegin + 1;
                prefix = colon > nameStart && colon < regionBegin + 12 ? xml.mid(nameStart, colon - nameStart + 1) : QByteArray();
            }
            if (!selfClosing) stack.push_back({i, local == "tbl"});
        }
        i = e;
    }
    if (!*changed) return xml;
    out += xml.mid(copied);
    return out;
}

QString firstTarget(const Package &pkg, const QString &source, const char *typeSuffix)
{
    const QVector<Relationship> rels = pkg.relationshipsOfType(source, QString::fromLatin1(typeSuffix));
    for (const Relationship &r : rels)
        if (!r.external && pkg.hasPart(r.target)) return r.target;
    return QString();
}

} // namespace

TableStyleDef readTableStyle(const Style &style)
{
    TableStyleDef def;
    def.base.pPr = style.pPr;
    def.base.rPr = style.rPr;
    if (style.tail.isEmpty()) return def;
    const QByteArray xml = QByteArray("<w:style xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">") + style.tail + "</w:style>";
    XmlTree t;
    if (!t.parse(xml, nullptr)) return def;
    Reader r(t);
    auto bands = [&](int tblPr, std::optional<int> *rows, std::optional<int> *cols) {
        for (int k = t.firstChild(tblPr); k >= 0; k = t.next(k)) {
            if (t.is(k, "tblStyleRowBandSize")) *rows = std::clamp(intOf(t, k).value_or(1), 1, 100);
            else if (t.is(k, "tblStyleColBandSize")) *cols = std::clamp(intOf(t, k).value_or(1), 1, 100);
        }
    };
    for (int c = t.firstChild(t.root()); c >= 0; c = t.next(c)) {
        if (t.is(c, "tblPr")) {
            r.tblPr(c, &def.base.tbl);
            bands(c, &def.rowBand, &def.colBand);
        } else if (t.is(c, "tcPr")) {
            r.tcPr(c, &def.base.cell);
        } else if (t.is(c, "tblStylePr")) {
            TableStylePart part;
            for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                if (t.is(k, "pPr")) part.pPr = r.pPr(k);
                else if (t.is(k, "rPr")) part.rPr = r.rPr(k);
                else if (t.is(k, "tblPr")) r.tblPr(k, &part.tbl);
                else if (t.is(k, "tcPr")) r.tcPr(k, &part.cell);
            }
            def.parts[t.attr(c, "type")] = part;
        }
    }
    return def;
}

bool readNotes(const QByteArray &xml, QVector<NotePart> *notes)
{
    XmlTree tree;
    if (!tree.parse(xml, nullptr)) return false;
    Reader reader(tree);
    reader.tablesAsStandIns = true;
    for (int c = tree.firstChild(tree.root()); c >= 0 && notes->size() < kMaxNotes; c = tree.next(c)) {
        if (!tree.is(c, "footnote") && !tree.is(c, "endnote")) continue;
        NotePart n;
        n.id = intOf(tree, c, "id").value_or(0);
        n.type = tree.attr(c, "type");
        reader.bodyContent(c, &n.blocks, 0, nullptr, nullptr);
        notes->push_back(std::move(n));
    }
    return true;
}

bool readFonts(const QByteArray &xml, QVector<FontEntry> *fonts)
{
    XmlTree tree;
    if (!tree.parse(xml, nullptr)) return false;
    for (int c = tree.firstChild(tree.root()); c >= 0 && fonts->size() < kMaxFonts; c = tree.next(c)) {
        if (!tree.is(c, "font")) continue;
        FontEntry f;
        f.name = tree.attr(c, "name");
        if (const int alt = tree.child(c, "altName"); alt >= 0) f.alternate = tree.val(alt);
        if (const int fam = tree.child(c, "family"); fam >= 0) f.family = tree.val(fam);
        if (const int pitch = tree.child(c, "pitch"); pitch >= 0) f.pitch = tree.val(pitch);
        if (const int panose = tree.child(c, "panose1"); panose >= 0) f.panose = tree.val(panose);
        if (const int charset = tree.child(c, "charset"); charset >= 0) f.charset = tree.val(charset);
        fonts->push_back(std::move(f));
    }
    return true;
}

QString noteNumberRestart(const QByteArray &settings, const char *group)
{
    XmlTree tree;
    if (!tree.parse(settings, nullptr)) return QString();
    const int notes = tree.child(tree.root(), group);
    const int restart = notes < 0 ? -1 : tree.child(notes, "numRestart");
    return restart < 0 ? QString() : tree.val(restart);
}

static std::atomic<qint64> s_noteNumberingReads{0};

qint64 noteNumberingReadsForTest() { return s_noteNumberingReads.load(); }

NoteNumbering readNoteNumbering(const QByteArray &settings, const char *group)
{
    ++s_noteNumberingReads;
    NoteNumbering n;
    XmlTree tree;
    if (!tree.parse(settings, nullptr)) return n;
    const int notes = tree.child(tree.root(), group);
    if (notes < 0) return n;
    if (const int format = tree.child(notes, "numFmt"); format >= 0) n.format = tree.val(format);
    if (const int start = tree.child(notes, "numStart"); start >= 0) n.start = std::clamp(tree.val(start).toInt(), 0, 32767);
    return n;
}

NoteNumbering noteNumberingOf(const SectionProps &section, const char *group)
{
    NoteNumbering n;
    for (const RawChild &child : section.extra) {   // (a section keeps its note properties as written: "<w:endnotePr>...</w:endnotePr>")
        if (!child.xml.contains(group)) continue;
        static const QRegularExpression format(QStringLiteral("numFmt\\s[^>]*?val=\"([^\"]*)\"")), start(QStringLiteral("numStart\\s[^>]*?val=\"(\\d+)\""));
        const QString text = QString::fromUtf8(child.xml.left(4096));
        if (const QRegularExpressionMatch m = format.match(text); m.hasMatch()) n.format = m.captured(1);
        if (const QRegularExpressionMatch m = start.match(text); m.hasMatch()) n.start = std::clamp(m.captured(1).toInt(), 0, 32767);
    }
    return n;
}

bool compatOption(const QByteArray &settings, const char *name)
{
    XmlTree tree;
    if (!tree.parse(settings, nullptr)) return false;
    const int compat = tree.child(tree.root(), "compat");
    if (compat < 0) return false;
    const int option = tree.child(compat, name);
    if (option < 0) return false;
    const QString value = tree.val(option).toLower();
    return !(value == QLatin1String("0") || value == QLatin1String("false") || value == QLatin1String("off"));
}

CompatOptions readCompatOptions(const QByteArray &settings)
{
    CompatOptions o;
    XmlTree tree;
    if (!tree.parse(settings, nullptr)) return o;
    const int compat = tree.child(tree.root(), "compat");
    if (compat < 0) return o;
    // (As compatOption has it: stated, and not turned off.)
    auto on = [&](const char *name) {
        const int option = tree.child(compat, name);
        if (option < 0) return false;
        const QString value = tree.val(option).toLower();
        return !(value == QLatin1String("0") || value == QLatin1String("false") || value == QLatin1String("off"));
    };
    o.useFELayout = on("useFELayout");
    o.doNotWrapTextWithPunct = on("doNotWrapTextWithPunct");
    o.doNotUseHTMLParagraphAutoSpacing = on("doNotUseHTMLParagraphAutoSpacing");
    o.noTabHangInd = on("noTabHangInd");
    o.doNotUseIndentAsNumberingTabStop = on("doNotUseIndentAsNumberingTabStop");
    o.splitPgBreakAndParaMark = on("splitPgBreakAndParaMark");
    o.suppressTopSpacingWP = on("suppressTopSpacingWP");
    o.noLeading = on("noLeading");
    o.suppressSpBfAfterPgBrk = on("suppressSpBfAfterPgBrk");
    o.wpJustification = on("wpJustification");
    return o;
}

bool readBlocks(const QByteArray &xml, QVector<Block> *blocks, bool tables)
{
    XmlTree tree;
    if (!tree.parse(xml, nullptr)) return false;
    Reader reader(tree);
    reader.tablesAsStandIns = !tables;
    reader.bodyContent(tree.root(), blocks, 0, nullptr, nullptr);
    return true;
}

bool readDocx(const QByteArray &zip, Document *doc, QString *error, ReadNotes *notes)
{
    *doc = Document();
    Package pkg;
    QString why;
    if (!pkg.read(zip, &why, kMaxInflatedBytes)) {
        if (error) *error = why;
        return false;
    }
    const QString main = pkg.mainPart();
    if (main.isEmpty() || !pkg.hasPart(main)) {
        if (error) *error = QCoreApplication::translate("jp::office::wml", "The file has no document in it.");
        return false;
    }
    XmlTree tree;
    bool flattened = false;
    if (!tree.parse(pkg.part(main), &why)) {
        // Nested past what the XML reader goes (a table in a cell in a table, thousands of times): what is below the limit is made plain text.
        bool changed = false;
        const QByteArray flat = flattenDeepXml(pkg.part(main), 200, &changed);
        if (!changed || !tree.parse(flat, nullptr)) {
            if (error) *error = QCoreApplication::translate("jp::office::wml", "The document can't be read: %1").arg(why);
            return false;
        }
        flattened = true;
    }
    const int root = tree.root();
    const int body = tree.child(root, "body");
    if (!tree.is(root, "document") || body < 0) {
        if (error) *error = QCoreApplication::translate("jp::office::wml", "The file is not a word processing document.");
        return false;
    }
    doc->mainName = main;
    doc->mainContentType = pkg.contentType(main);
    if (notes && flattened) {
        notes->leftOut = true;
        notes->notes << QCoreApplication::translate("jp::office::wml", "Tables nested deeper than this program can read were made plain paragraphs of text.");
    }
    if (notes) {
        notes->hadMacros = doc->mainContentType.contains(QLatin1String("macroEnabled"));
        for (const QString &name : pkg.partNames()) {
            const QString type = pkg.contentType(name);
            notes->hadMacros = notes->hadMacros || type.startsWith(QLatin1String("application/vnd.ms-office.vba")) || type.contains(QLatin1String("macroEnabled")) || type.contains(QLatin1String("vbaData"));
        }
        if (pkg.skippedCount() > 0) {
            notes->leftOut = true;
            notes->notes << QCoreApplication::translate("jp::office::wml", "%n part or link of the file had a name that could not be used, and was left out.", nullptr, pkg.skippedCount());
        }
    }

    QString stylesName = firstTarget(pkg, main, "/styles"), numberingName = firstTarget(pkg, main, "/numbering");
    QString themeName = firstTarget(pkg, main, "/theme"), settingsName = firstTarget(pkg, main, "/settings");

    // The parts that are read here and written afresh; one that can't be read stays a kept part, as it was.
    auto noteIt = [&](const QString &what) {
        if (notes) notes->notes << what;
    };
    {
        XmlTree styles;
        if (!stylesName.isEmpty() && styles.parse(pkg.part(stylesName), nullptr) && styles.is(styles.root(), "styles")) {
            Reader(styles).styles(&doc->styles);
            doc->stylesName = stylesName;
        } else if (!stylesName.isEmpty()) {
            noteIt(QCoreApplication::translate("jp::office::wml", "The styles could not be read."));
        }
    }
    {
        XmlTree num;
        if (!numberingName.isEmpty() && num.parse(pkg.part(numberingName), nullptr) && num.is(num.root(), "numbering")) {
            Reader(num).numbering(&doc->numbering);
            doc->numberingName = numberingName;
        } else if (!numberingName.isEmpty()) {
            noteIt(QCoreApplication::translate("jp::office::wml", "The list definitions could not be read."));
        }
    }
    if (!themeName.isEmpty() && readTheme(pkg.part(themeName), &doc->theme, nullptr)) {
        doc->hasTheme = true;
        doc->themeName = themeName;
    }
    {
        XmlTree settings;
        if (!settingsName.isEmpty() && settings.parse(pkg.part(settingsName), nullptr)) {
            doc->settingsName = settingsName;
            for (int c = settings.firstChild(settings.root()); c >= 0; c = settings.next(c)) {
                if (settings.is(c, "defaultTabStop")) doc->defaultTabStop = std::clamp(intOf(settings, c).value_or(720), 1, 20000);
                else if (settings.is(c, "evenAndOddHeaders")) doc->evenAndOddHeaders = onOffOf(settings, c).value_or(true);
                else if (settings.is(c, "compat"))
                    for (int k = settings.firstChild(c); k >= 0; k = settings.next(k))
                        if (settings.is(k, "compatSetting") && settings.attr(k, "name") == QLatin1String("compatibilityMode"))
                            doc->compatMode = std::clamp(settings.attr(k, "val").toInt(), 8, 99);
            }
        }
    }

    // The main part.
    doc->rootStart = tree.startTag(root);
    Reader reader(tree);
    {
        bool seenBody = false;
        for (int c = tree.firstChild(root); c >= 0; c = tree.next(c)) {
            if (c == body) seenBody = true;
            else if (seenBody) doc->afterBody += tree.raw(c);
            else doc->beforeBody += tree.raw(c);
        }
    }
    reader.bodyContent(body, &doc->body, 0, &doc->lastSection, &doc->lastSectionStated);
    if (reader.truncated) {
        noteIt(QCoreApplication::translate("jp::office::wml", "The document is longer than this program can open: the end of it was left out."));
        if (notes) notes->leftOut = true;
    }
    if (reader.standIns > 0)
        noteIt(QCoreApplication::translate("jp::office::wml", "%n table could not be shown as a table (it has more rows, columns, or cells than this program can lay out, or is nested too deeply, or has no rows): it is kept as it was and shown as a stand-in.", nullptr, reader.standIns));

    // The package: every relationship as read, and every part but the ones above.
    doc->packageRelationships = pkg.relationships(QStringLiteral("/"));
    doc->mainRelationships = pkg.relationships(main);
    for (const QString &name : pkg.partNames()) {
        if (name == main) continue;
        const QVector<Relationship> rels = pkg.relationships(name);
        // (The styles and the numbering are written afresh, but their links, a picture bullet's image, are kept with theirs.)
        if (name == doc->stylesName || name == doc->numberingName) {
            if (!rels.isEmpty()) doc->partRelationships.insert(name, rels);
            continue;
        }
        KeptPart part;
        part.name = name;
        part.contentType = pkg.contentType(name);
        part.data = pkg.part(name);
        doc->parts.push_back(part);
        if (!rels.isEmpty()) doc->partRelationships.insert(name, rels);
    }
    return true;
}

} // namespace jp::office::wml
