// Reading a .docx: the package into a wml::Document. See wml.h for what is understood and what is kept as written.

#include "wml/wml.h"

#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QSet>
#include <algorithm>

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
            } else if (name == "sz") {
                if (auto v = intOf(t, c)) p.size = std::clamp(*v, 1, 4000);
            } else if (name == "highlight") {
                p.highlight = t.val(c);
            } else if (name == "u") {
                p.underline = Underline{t.val(c).isEmpty() ? QStringLiteral("single") : t.val(c), t.attr(c, "color")};
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
            } else if (name == "pgNumType") {
                s.pageFormat = t.attr(c, "fmt");
                if (const std::optional<int> start = intOf(t, c, "start")) s.pageStart = std::clamp(*start, 0, 100000);
            } else if (name == "cols") {
                s.hasCols = true;
                s.cols = std::clamp(intOf(t, c, "num").value_or(1), 1, 45);
                s.colSpace = std::clamp(intOf(t, c, "space").value_or(720), 0, 63360);
                s.colSep = onOffString(t.attr(c, "sep"));
                s.colEqual = !t.has(c, "equalWidth") || onOffString(t.attr(c, "equalWidth"));
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
                p.extra.push_back({paraChildRank(name), t.raw(c)});
            }
        }
        return p;
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
                else if (name == "sym") a.display = QString(QChar(0x25A1));
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

    void bodyContent(int n, QVector<Block> *out, int depth, SectionProps *last, bool *lastStated)
    {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
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
                out->push_back(b);
            } else if (name == "sectPr") {
                if (last) {
                    *last = sectPr(c);
                    if (lastStated) *lastStated = true;
                }
            } else if (name == "tbl") {
                int rows = 0, cols = 0;
                for (int r = t.firstChild(c); r >= 0; r = t.next(r)) {
                    if (!t.is(r, "tr")) continue;
                    if (++rows == 1)
                        for (int cell = t.firstChild(r); cell >= 0; cell = t.next(cell))
                            if (t.is(cell, "tc")) ++cols;
                }
                out->push_back(rawBlock(c, QCoreApplication::translate("jp::office::wml", "[Table: %1 rows by %2 columns]").arg(rows).arg(cols)));
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
                    const int so = t.child(k, "startOverride");
                    if (so >= 0) m.startOverrides.insert(std::clamp(intOf(t, k, "ilvl").value_or(0), 0, 8), intOf(t, so).value_or(1));
                }
                num->nums.push_back(m);
            } else if (t.is(c, "numIdMacAtCleanup")) {
                num->suffix += t.raw(c);
            }
        }
    }
};

QString firstTarget(const Package &pkg, const QString &source, const char *typeSuffix)
{
    const QVector<Relationship> rels = pkg.relationshipsOfType(source, QString::fromLatin1(typeSuffix));
    for (const Relationship &r : rels)
        if (!r.external && pkg.hasPart(r.target)) return r.target;
    return QString();
}

} // namespace

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
    if (!tree.parse(pkg.part(main), &why)) {
        if (error) *error = QCoreApplication::translate("jp::office::wml", "The document can't be read: %1").arg(why);
        return false;
    }
    const int root = tree.root();
    const int body = tree.child(root, "body");
    if (!tree.is(root, "document") || body < 0) {
        if (error) *error = QCoreApplication::translate("jp::office::wml", "The file is not a word processing document.");
        return false;
    }
    doc->mainName = main;
    doc->mainContentType = pkg.contentType(main);

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
    if (reader.truncated) noteIt(QCoreApplication::translate("jp::office::wml", "The document is longer than this program can open: the end of it was left out."));

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
