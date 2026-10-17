#include "wml/wml.h"

#include "wml/xmltree.h"

#include <QRegularExpression>
#include <algorithm>
#include <tuple>

namespace jp::office::wml {

namespace {

int rankIn(const QByteArray &name, const char *const *names, int count, int unknown)
{
    for (int i = 0; i < count; ++i)
        if (name == names[i]) return i;
    return unknown;
}

QByteArray valAttr(const char *name, const QString &v) { return QByteArray(" w:") + name + "=\"" + escapeXml(v, true) + '"'; }
QByteArray intAttr(const char *name, int v) { return QByteArray(" w:") + name + "=\"" + QByteArray::number(v) + '"'; }

QByteArray onOff(const char *name, bool v) { return QByteArray("<w:") + name + (v ? "/>" : " w:val=\"0\"/>"); }
QByteArray valEl(const char *name, const QString &v) { return QByteArray("<w:") + name + valAttr("val", v) + "/>"; }
QByteArray intEl(const char *name, int v) { return QByteArray("<w:") + name + intAttr("val", v) + "/>"; }

// Children with their ranks, written in rank order (ties in the order they were added).
struct Parts {
    QVector<RawChild> items;
    void add(int rank, const QByteArray &xml) { items.push_back({rank, xml}); }
    QByteArray joined() const
    {
        QVector<RawChild> sorted = items;
        std::stable_sort(sorted.begin(), sorted.end(), [](const RawChild &a, const RawChild &b) { return a.rank < b.rank; });
        QByteArray out;
        for (const RawChild &c : std::as_const(sorted)) out += c.xml;
        return out;
    }
};

QString normalJc(const QString &jc)
{
    if (jc.isEmpty() || jc == QLatin1String("start")) return QStringLiteral("left");
    if (jc == QLatin1String("end")) return QStringLiteral("right");
    return jc;
}

} // namespace

int paraChildRank(const QByteArray &name)
{
    static const char *const names[] = {"pStyle", "keepNext", "keepLines", "pageBreakBefore", "framePr", "widowControl", "numPr", "suppressLineNumbers", "pBdr", "shd",
                                        "tabs", "suppressAutoHyphens", "kinsoku", "wordWrap", "overflowPunct", "topLinePunct", "autoSpaceDE", "autoSpaceDN", "bidi",
                                        "adjustRightInd", "snapToGrid", "spacing", "ind", "contextualSpacing", "mirrorIndents", "suppressOverlap", "jc", "textDirection",
                                        "textAlignment", "textboxTightWrap", "outlineLvl", "divId", "cnfStyle", "rPr", "sectPr", "pPrChange"};
    return rankIn(name, names, int(sizeof names / sizeof *names), 100);
}

int runChildRank(const QByteArray &name)
{
    static const char *const names[] = {"rStyle", "rFonts", "b", "bCs", "i", "iCs", "caps", "smallCaps", "strike", "dstrike", "outline", "shadow", "emboss", "imprint",
                                        "noProof", "snapToGrid", "vanish", "webHidden", "color", "spacing", "w", "kern", "position", "sz", "szCs", "highlight", "u",
                                        "effect", "bdr", "shd", "fitText", "vertAlign", "rtl", "cs", "em", "lang", "eastAsianLayout", "specVanish", "oMath"};
    // (The marks of a tracked change on a paragraph mark come before the rest, and the record of a change to the properties after it.)
    if (name == "ins") return -4;
    if (name == "del") return -3;
    if (name == "moveFrom") return -2;
    if (name == "moveTo") return -1;
    if (name == "rPrChange") return 1000;
    return rankIn(name, names, int(sizeof names / sizeof *names), 500);
}

int sectionChildRank(const QByteArray &name)
{
    static const char *const names[] = {"headerReference", "footnotePr", "endnotePr", "type", "pgSz", "pgMar", "paperSrc", "pgBorders", "lnNumType", "pgNumType",
                                        "cols", "formProt", "vAlign", "noEndnote", "titlePg", "textDirection", "bidi", "rtlGutter", "docGrid", "printerSettings",
                                        "sectPrChange"};
    if (name == "footerReference") return 0;
    return rankIn(name, names, int(sizeof names / sizeof *names), 100);
}

// ---- headers and footers

bool HeaderRefs::any() const
{
    for (int i = 0; i < 3; ++i)
        if (!header[i].isEmpty() || !footer[i].isEmpty()) return true;
    return false;
}

HeaderRefs headerRefsOf(const SectionProps &section)
{
    static const QRegularExpression tag(QStringLiteral("^<(?:[\\w.-]+:)?(header|footer)Reference\\b([^>]*)>"));
    static const QRegularExpression type(QStringLiteral("[\\s:]type\\s*=\\s*[\"']([^\"']*)[\"']")), id(QStringLiteral("[\\s:]id\\s*=\\s*[\"']([^\"']*)[\"']"));
    HeaderRefs out;
    for (const RawChild &c : section.extra) {
        const QRegularExpressionMatch m = tag.match(QString::fromUtf8(c.xml.left(2000)));
        if (!m.hasMatch()) continue;
        const QString attrs = m.captured(2), kind = type.match(attrs).captured(1), rid = id.match(attrs).captured(1);
        const int slot = kind == QLatin1String("first") ? 1 : kind == QLatin1String("even") ? 2 : 0;
        if (rid.isEmpty()) continue;
        (m.captured(1) == QLatin1String("header") ? out.header : out.footer)[slot] = rid;
    }
    return out;
}

// ---- Fonts

bool Fonts::operator==(const Fonts &o) const
{
    return ascii == o.ascii && hAnsi == o.hAnsi && eastAsia == o.eastAsia && cs == o.cs && asciiTheme == o.asciiTheme && hAnsiTheme == o.hAnsiTheme &&
           eastAsiaTheme == o.eastAsiaTheme && csTheme == o.csTheme && hint == o.hint;
}

bool Fonts::isEmpty() const { return *this == Fonts(); }

void Fonts::overlay(const Fonts &top)
{
    if (!top.ascii.isEmpty() || !top.asciiTheme.isEmpty()) { ascii = top.ascii; asciiTheme = top.asciiTheme; }
    if (!top.hAnsi.isEmpty() || !top.hAnsiTheme.isEmpty()) { hAnsi = top.hAnsi; hAnsiTheme = top.hAnsiTheme; }
    if (!top.eastAsia.isEmpty() || !top.eastAsiaTheme.isEmpty()) { eastAsia = top.eastAsia; eastAsiaTheme = top.eastAsiaTheme; }
    if (!top.cs.isEmpty() || !top.csTheme.isEmpty()) { cs = top.cs; csTheme = top.csTheme; }
    if (!top.hint.isEmpty()) hint = top.hint;
}

// ---- RunProps

void RunProps::overlay(const RunProps &t)
{
    if (t.fonts) {
        if (fonts) fonts->overlay(*t.fonts);
        else fonts = t.fonts;
    }
    if (t.bold) bold = t.bold;
    if (t.italic) italic = t.italic;
    if (t.caps) caps = t.caps;
    if (t.smallCaps) smallCaps = t.smallCaps;
    if (t.strike) strike = t.strike;
    if (t.dstrike) dstrike = t.dstrike;
    if (t.vanish) vanish = t.vanish;
    if (t.noProof) noProof = t.noProof;
    if (t.color) color = t.color;
    if (t.spacing) spacing = t.spacing;
    if (t.position) position = t.position;
    if (t.kern) kern = t.kern;
    if (t.size) size = t.size;
    if (!t.highlight.isEmpty()) highlight = t.highlight;
    if (t.underline) underline = t.underline;
    if (!t.vertAlign.isEmpty()) vertAlign = t.vertAlign;
}

RunProps RunProps::normalized() const
{
    RunProps n = *this;
    if (!n.fonts) n.fonts = Fonts();
    for (std::optional<bool> *b : {&n.bold, &n.italic, &n.caps, &n.smallCaps, &n.strike, &n.dstrike, &n.vanish, &n.noProof})
        if (!*b) *b = false;
    if (!n.color) n.color = Color{QStringLiteral("auto"), QString(), QString(), QString()};
    if (!n.spacing) n.spacing = 0;
    if (!n.position) n.position = 0;
    if (!n.kern) n.kern = 0;
    if (!n.size) n.size = 20;
    if (n.highlight.isEmpty()) n.highlight = QStringLiteral("none");
    if (!n.underline) n.underline = Underline{QStringLiteral("none"), QString()};
    if (n.vertAlign.isEmpty()) n.vertAlign = QStringLiteral("baseline");
    return n;
}

bool RunProps::operator==(const RunProps &o) const
{
    return style == o.style && fonts == o.fonts && bold == o.bold && italic == o.italic && caps == o.caps && smallCaps == o.smallCaps && strike == o.strike &&
           dstrike == o.dstrike && vanish == o.vanish && noProof == o.noProof && color == o.color && spacing == o.spacing && position == o.position && kern == o.kern && size == o.size &&
           highlight == o.highlight && underline == o.underline && vertAlign == o.vertAlign && extra == o.extra;
}

RunProps RunProps::differences(const RunProps &base) const
{
    const RunProps n = normalized(), b = base.normalized();
    RunProps d;
    d.style = style;
    d.extra = extra;
    // The Latin fonts are what the text engine's formats hold. The fonts of other scripts are held beside them, as the run itself said them (and
    // none for a run that said none): a run's own differ from its style's when it says other ones, and those it says alone are what is written
    // when the Latin fonts are the style's.
    auto latin = [](const std::optional<Fonts> &f) { return f ? std::make_tuple(f->ascii, f->hAnsi, f->asciiTheme, f->hAnsiTheme) : std::tuple<QString, QString, QString, QString>(); };
    if (fonts) {
        const bool otherScripts = n.fonts->hasOtherScripts() && !n.fonts->sameOtherScripts(*b.fonts);
        if (latin(n.fonts) != latin(b.fonts)) {
            d.fonts = fonts;
        } else if (otherScripts) {
            Fonts only;
            only.eastAsia = fonts->eastAsia;
            only.cs = fonts->cs;
            only.eastAsiaTheme = fonts->eastAsiaTheme;
            only.csTheme = fonts->csTheme;
            only.hint = fonts->hint;
            d.fonts = only;
        }
    }
    auto flag = [&](std::optional<bool> RunProps::*f) {
        if ((n.*f) != (b.*f)) (d.*f) = (n.*f);
    };
    flag(&RunProps::bold);
    flag(&RunProps::italic);
    flag(&RunProps::caps);
    flag(&RunProps::smallCaps);
    flag(&RunProps::strike);
    flag(&RunProps::dstrike);
    flag(&RunProps::vanish);
    flag(&RunProps::noProof);
    if (n.color != b.color) d.color = n.color;
    if (n.spacing != b.spacing) d.spacing = n.spacing;
    if (n.position != b.position) d.position = n.position;
    if (n.kern != b.kern) d.kern = n.kern;
    if (size && n.size != b.size) d.size = size;
    if (n.highlight != b.highlight) d.highlight = n.highlight;
    if (n.underline != b.underline) d.underline = n.underline;
    if (n.vertAlign != b.vertAlign) d.vertAlign = n.vertAlign;
    return d;
}

// ---- ParaProps

void ParaProps::overlay(const ParaProps &t)
{
    if (t.keepNext) keepNext = t.keepNext;
    if (t.keepLines) keepLines = t.keepLines;
    if (t.pageBreakBefore) pageBreakBefore = t.pageBreakBefore;
    if (t.widowControl) widowControl = t.widowControl;
    if (t.contextualSpacing) contextualSpacing = t.contextualSpacing;
    if (t.numId) numId = t.numId;
    if (t.ilvl) ilvl = t.ilvl;
    if (t.tabs) {
        QVector<TabStop> list = tabs.value_or(QVector<TabStop>());
        for (const TabStop &tab : *t.tabs) {
            list.erase(std::remove_if(list.begin(), list.end(), [&](const TabStop &x) { return x.pos == tab.pos; }), list.end());
            if (tab.val != QLatin1String("clear")) list.push_back(tab);
        }
        std::sort(list.begin(), list.end(), [](const TabStop &a, const TabStop &b) { return a.pos < b.pos; });
        tabs = list;
    }
    if (t.before) before = t.before;
    if (t.after) after = t.after;
    if (t.beforeAuto) beforeAuto = t.beforeAuto;
    if (t.afterAuto) afterAuto = t.afterAuto;
    if (t.line) line = t.line;
    if (!t.lineRule.isEmpty()) lineRule = t.lineRule;
    if (t.indLeft) indLeft = t.indLeft;
    if (t.indRight) indRight = t.indRight;
    if (t.indFirst) indFirst = t.indFirst;
    if (!t.jc.isEmpty()) jc = t.jc;
    if (t.outlineLvl) outlineLvl = t.outlineLvl;
    if (t.overflowPunct) overflowPunct = t.overflowPunct;
    if (t.borders) {
        ParaBorders b = borders.value_or(ParaBorders());
        if (t.borders->top) b.top = t.borders->top;
        if (t.borders->left) b.left = t.borders->left;
        if (t.borders->bottom) b.bottom = t.borders->bottom;
        if (t.borders->right) b.right = t.borders->right;
        if (t.borders->between) b.between = t.borders->between;
        borders = b;
    }
    if (t.shading) shading = t.shading;
}

ParaProps ParaProps::normalized() const
{
    ParaProps n = *this;
    for (std::optional<bool> *b : {&n.keepNext, &n.keepLines, &n.pageBreakBefore, &n.contextualSpacing})
        if (!*b) *b = false;
    if (!n.widowControl) n.widowControl = true;
    if (!n.numId) n.numId = 0;
    if (!n.ilvl) n.ilvl = 0;
    if (!n.tabs) n.tabs = QVector<TabStop>();
    for (TabStop &t : *n.tabs) {   // (a tab with no leader says "none" in some files, and one with no type is a left tab)
        if (t.leader == QLatin1String("none")) t.leader.clear();
        if (t.val.isEmpty()) t.val = QStringLiteral("left");
    }
    // (A bar and a list's tab are not tab stops the text engine has: they are kept apart from the tabs that are compared.)
    n.tabs->erase(std::remove_if(n.tabs->begin(), n.tabs->end(), [](const TabStop &t) { return t.val == QLatin1String("bar") || t.val == QLatin1String("num"); }), n.tabs->end());
    if (!n.before) n.before = 0;
    if (!n.after) n.after = 0;
    if (!n.line) n.line = 240;
    if (n.lineRule.isEmpty()) n.lineRule = QStringLiteral("auto");
    if (!n.indLeft) n.indLeft = 0;
    if (!n.indRight) n.indRight = 0;
    if (!n.indFirst) n.indFirst = 0;
    n.jc = normalJc(n.jc);
    if (!n.outlineLvl) n.outlineLvl = 9;
    return n;
}

ParaProps ParaProps::differences(const ParaProps &base) const
{
    const ParaProps n = normalized(), b = base.normalized();
    ParaProps d;
    d.style = style;
    d.extra = extra;
    d.spacingAttrs = spacingAttrs;
    d.indAttrs = indAttrs;
    d.markRun = markRun;
    d.section = section;
    auto flag = [&](std::optional<bool> ParaProps::*f) {
        if ((n.*f) != (b.*f)) (d.*f) = (n.*f);
    };
    flag(&ParaProps::keepNext);
    flag(&ParaProps::keepLines);
    flag(&ParaProps::pageBreakBefore);
    flag(&ParaProps::widowControl);
    flag(&ParaProps::contextualSpacing);
    if (n.numId != b.numId || (n.numId.value_or(0) != 0 && n.ilvl != b.ilvl)) {
        d.numId = n.numId;
        if (n.numId.value_or(0) != 0) d.ilvl = n.ilvl;
    }
    if (n.tabs != b.tabs) {
        QVector<TabStop> list;
        for (const TabStop &t : *n.tabs)
            if (!b.tabs->contains(t)) list.push_back(t);
        for (const TabStop &t : *b.tabs) {
            const bool kept = std::any_of(n.tabs->begin(), n.tabs->end(), [&](const TabStop &x) { return x.pos == t.pos; });
            if (!kept) list.push_back({t.pos, QStringLiteral("clear"), QString()});
        }
        d.tabs = list;
    }
    if (n.before != b.before) d.before = n.before;
    if (n.after != b.after) d.after = n.after;
    if (n.line != b.line || n.lineRule != b.lineRule) {
        d.line = n.line;
        d.lineRule = n.lineRule;
    }
    if (n.indLeft != b.indLeft) d.indLeft = n.indLeft;
    if (n.indRight != b.indRight) d.indRight = n.indRight;
    if (n.indFirst != b.indFirst) d.indFirst = n.indFirst;
    if (n.jc != b.jc) d.jc = n.jc;
    if (n.outlineLvl != b.outlineLvl) d.outlineLvl = n.outlineLvl;
    return d;
}

// ---- StyleSheet

void StyleSheet::reindex() const
{
    m_index.clear();
    for (int i = 0; i < styles.size(); ++i)
        if (!m_index.contains(styles[i].id)) m_index.insert(styles[i].id, i);   // (the first of an id stands)
    m_indexed = styles.size();
}

const Style *StyleSheet::find(const QString &id) const
{
    if (m_indexed != styles.size()) reindex();
    const auto it = m_index.constFind(id);
    return it == m_index.constEnd() ? nullptr : &styles[it.value()];
}

QString StyleSheet::defaultId(const QString &type) const
{
    for (const Style &s : styles)
        if (s.isDefault && s.type == type) return s.id;
    return QString();
}

bool StyleSheet::add(const Style &style)
{
    if (style.id.isEmpty()) return false;
    modified = true;
    for (Style &s : styles)
        if (s.id == style.id) {
            s = style;
            changed();
            return true;
        }
    styles.push_back(style);
    changed();
    return true;
}

namespace {
constexpr int kStyleDepth = 32;   // (real files nest styles 4 or 5 deep)
}

ParaProps StyleSheet::paragraph(const QString &id, const TableLayer *table) const
{
    const QString cacheKey = table ? id + QLatin1String("\n#") + table->key : id;
    const auto cached = m_paragraph.constFind(cacheKey);
    if (cached != m_paragraph.constEnd()) return cached.value();
    QVector<const Style *> chain;
    for (QString at = id; !at.isEmpty() && chain.size() < kStyleDepth;) {
        const Style *s = find(at);
        if (!s || chain.contains(s)) break;
        chain.push_back(s);
        at = s->basedOn;
    }
    ParaProps p = defaultPara;
    if (table) p.overlay(table->p);
    for (int i = int(chain.size()) - 1; i >= 0; --i) p.overlay(chain[i]->pPr);
    p.style = id;
    // (What a style says besides the properties that have a field here is the style's own: not carried to the paragraphs that have it.)
    p.extra.clear();
    p.spacingAttrs.clear();
    p.indAttrs.clear();
    p.markRun.reset();
    p.section.reset();
    m_paragraph.insert(cacheKey, p);
    return p;
}

ParaProps StyleSheet::paragraphWithNumbering(const QString &id, const ParaProps &level, const TableLayer *table) const
{
    QVector<const Style *> chain;
    for (QString at = id; !at.isEmpty() && chain.size() < kStyleDepth;) {
        const Style *s = find(at);
        if (!s || chain.contains(s)) break;
        chain.push_back(s);
        at = s->basedOn;
    }
    // The style that says the paragraphs are numbered: the nearest to the paragraph's own.
    int numbered = -1;
    for (int i = 0; i < chain.size() && numbered < 0; ++i)
        if (chain[i]->pPr.numId) numbered = i;
    if (numbered < 0) {
        ParaProps p = paragraph(id, table);
        p.overlay(level);
        return p;
    }
    ParaProps p = defaultPara;
    if (table) p.overlay(table->p);
    for (int i = int(chain.size()) - 1; i >= 0; --i) {
        if (i == numbered) p.overlay(level);
        p.overlay(chain[i]->pPr);
    }
    p.style = id;
    p.extra.clear();
    p.spacingAttrs.clear();
    p.indAttrs.clear();
    p.markRun.reset();
    p.section.reset();
    return p;
}

RunProps StyleSheet::run(const QString &paragraphStyle, const QString &characterStyle, const TableLayer *table) const
{
    const QString key = paragraphStyle + QLatin1Char('\n') + characterStyle + (table ? QLatin1String("\n#") + table->key : QString());
    const auto cached = m_run.constFind(key);
    if (cached != m_run.constEnd()) return cached.value();
    RunProps r = defaultRun;
    if (table) r.overlay(table->r);
    // What a style says over the styles it is based on (each over the one under it).
    auto chainOf = [&](const QString &id) {
        QVector<const Style *> chain;
        for (QString at = id; !at.isEmpty() && chain.size() < kStyleDepth;) {
            const Style *s = find(at);
            if (!s || chain.contains(s)) break;
            chain.push_back(s);
            at = s->basedOn;
        }
        RunProps said;
        for (int i = int(chain.size()) - 1; i >= 0; --i) said.overlay(chain[i]->rPr);
        return said;
    };
    r.overlay(chainOf(paragraphStyle));
    // The toggles of a character style flip the paragraph style's (Strong over a heading that is bold is not bold); an "off" flips nothing. The rest of what it says
    // goes over.
    RunProps chr = chainOf(characterStyle);
    for (std::optional<bool> RunProps::*toggle : {&RunProps::bold, &RunProps::italic, &RunProps::caps, &RunProps::smallCaps, &RunProps::strike, &RunProps::vanish}) {
        if ((chr.*toggle).value_or(false)) r.*toggle = !(r.*toggle).value_or(false);
        chr.*toggle = std::nullopt;
    }
    r.overlay(chr);
    r.style = characterStyle;
    r.extra.clear();
    m_run.insert(key, r);
    return r;
}

// ---- table styles

namespace {
void mergeBorders(TableMark *d, const TableMark &s)
{
    if (s.top.stated()) d->top = s.top;
    if (s.left.stated()) d->left = s.left;
    if (s.bottom.stated()) d->bottom = s.bottom;
    if (s.right.stated()) d->right = s.right;
    if (s.insideH.stated()) d->insideH = s.insideH;
    if (s.insideV.stated()) d->insideV = s.insideV;
    if (s.diagDown.stated()) d->diagDown = s.diagDown;
    if (s.diagUp.stated()) d->diagUp = s.diagUp;
}

} // namespace

void overlayTableMark(TableMark *dm, const TableMark &sm)
{
    mergeBorders(dm, sm);
    if (sm.marTop) dm->marTop = sm.marTop;
    if (sm.marLeft) dm->marLeft = sm.marLeft;
    if (sm.marBottom) dm->marBottom = sm.marBottom;
    if (sm.marRight) dm->marRight = sm.marRight;
    if (sm.cMarTop) dm->cMarTop = sm.cMarTop;
    if (sm.cMarLeft) dm->cMarLeft = sm.cMarLeft;
    if (sm.cMarBottom) dm->cMarBottom = sm.cMarBottom;
    if (sm.cMarRight) dm->cMarRight = sm.cMarRight;
    if (!sm.shdVal.isEmpty() || !sm.shdFill.isEmpty()) {
        dm->shdVal = sm.shdVal;
        dm->shdFill = sm.shdFill;
        dm->shdColor = sm.shdColor;
    }
    if (sm.indent.stated()) dm->indent = sm.indent;
    if (sm.spacing.stated()) dm->spacing = sm.spacing;
    if (!sm.jc.isEmpty()) dm->jc = sm.jc;
    if (!sm.layout.isEmpty()) dm->layout = sm.layout;
    if (!sm.vAlign.isEmpty()) dm->vAlign = sm.vAlign;
    if (sm.noWrap) dm->noWrap = true;
}

namespace {

void mergePart(TableStylePart *d, const TableStylePart &s)
{
    overlayTableMark(&d->tbl, s.tbl);
    overlayTableMark(&d->cell, s.cell);
    if (s.pPr) {
        if (d->pPr) d->pPr->overlay(*s.pPr);
        else d->pPr = s.pPr;
    }
    if (s.rPr) {
        if (d->rPr) d->rPr->overlay(*s.rPr);
        else d->rPr = s.rPr;
    }
}

} // namespace

const TableStyleDef &StyleSheet::tableStyle(const QString &id) const
{
    const auto cached = m_tableStyles.constFind(id);
    if (cached != m_tableStyles.constEnd()) return cached.value();
    QVector<const Style *> chain;
    for (QString at = id; !at.isEmpty() && chain.size() < kStyleDepth;) {
        const Style *s = find(at);
        if (!s || chain.contains(s)) break;
        chain.push_back(s);
        at = s->basedOn;
    }
    TableStyleDef def;
    for (int i = int(chain.size()) - 1; i >= 0; --i) {
        const TableStyleDef own = readTableStyle(*chain[i]);
        mergePart(&def.base, own.base);
        for (auto it = own.parts.cbegin(); it != own.parts.cend(); ++it) mergePart(&def.parts[it.key()], it.value());
        if (own.rowBand) def.rowBand = own.rowBand;
        if (own.colBand) def.colBand = own.colBand;
    }
    return m_tableStyles.insert(id, def).value();
}

// ---- Numbering

void Numbering::index() const
{
    if (m_numCount == nums.size() && m_abstractCount == abstracts.size()) return;
    m_nums.clear();
    m_abstracts.clear();
    for (int i = 0; i < nums.size(); ++i)
        if (!m_nums.contains(nums[i].id)) m_nums.insert(nums[i].id, i);
    for (int i = 0; i < abstracts.size(); ++i)
        if (!m_abstracts.contains(abstracts[i].id)) m_abstracts.insert(abstracts[i].id, i);
    m_numCount = nums.size();
    m_abstractCount = abstracts.size();
}

const Num *Numbering::num(int numId) const
{
    index();
    const auto it = m_nums.constFind(numId);
    return it == m_nums.constEnd() ? nullptr : &nums[it.value()];
}

const AbstractNum *Numbering::abstractOf(int numId) const
{
    const Num *n = num(numId);
    if (!n) return nullptr;
    const auto it = m_abstracts.constFind(n->abstractId);
    return it == m_abstracts.constEnd() ? nullptr : &abstracts[it.value()];
}

int Numbering::nextNumId() const
{
    int most = 0;
    for (const Num &n : nums) most = std::max(most, n.id);
    return most + 1;
}

int Numbering::nextAbstractId() const
{
    int most = -1;
    for (const AbstractNum &a : abstracts) most = std::max(most, a.id);
    return most + 1;
}

// ---- Property elements

QByteArray runPropsXml(const RunProps &p, bool)
{
    Parts parts;
    if (!p.style.isEmpty()) parts.add(runChildRank("rStyle"), valEl("rStyle", p.style));
    if (p.fonts && !p.fonts->isEmpty()) {
        const Fonts &f = *p.fonts;
        QByteArray e = "<w:rFonts";
        if (!f.hint.isEmpty()) e += valAttr("hint", f.hint);
        if (!f.ascii.isEmpty()) e += valAttr("ascii", f.ascii);
        if (!f.hAnsi.isEmpty()) e += valAttr("hAnsi", f.hAnsi);
        if (!f.eastAsia.isEmpty()) e += valAttr("eastAsia", f.eastAsia);
        if (!f.cs.isEmpty()) e += valAttr("cs", f.cs);
        if (!f.asciiTheme.isEmpty()) e += valAttr("asciiTheme", f.asciiTheme);
        if (!f.hAnsiTheme.isEmpty()) e += valAttr("hAnsiTheme", f.hAnsiTheme);
        if (!f.eastAsiaTheme.isEmpty()) e += valAttr("eastAsiaTheme", f.eastAsiaTheme);
        if (!f.csTheme.isEmpty()) e += valAttr("cstheme", f.csTheme);
        parts.add(runChildRank("rFonts"), e + "/>");
    }
    auto flag = [&](const char *name, const std::optional<bool> &v) {
        if (v) parts.add(runChildRank(name), onOff(name, *v));
    };
    flag("b", p.bold);
    flag("i", p.italic);
    flag("caps", p.caps);
    flag("smallCaps", p.smallCaps);
    flag("strike", p.strike);
    flag("dstrike", p.dstrike);
    flag("noProof", p.noProof);
    flag("vanish", p.vanish);
    if (p.color) {
        QByteArray e = "<w:color" + valAttr("val", p.color->val.isEmpty() ? QStringLiteral("auto") : p.color->val);
        if (!p.color->theme.isEmpty()) e += valAttr("themeColor", p.color->theme);
        if (!p.color->tint.isEmpty()) e += valAttr("themeTint", p.color->tint);
        if (!p.color->shade.isEmpty()) e += valAttr("themeShade", p.color->shade);
        parts.add(runChildRank("color"), e + "/>");
    }
    if (p.spacing) parts.add(runChildRank("spacing"), intEl("spacing", *p.spacing));
    if (p.kern) parts.add(runChildRank("kern"), intEl("kern", *p.kern));
    if (p.position) parts.add(runChildRank("position"), intEl("position", *p.position));
    if (p.size) parts.add(runChildRank("sz"), intEl("sz", *p.size));
    if (!p.highlight.isEmpty()) parts.add(runChildRank("highlight"), valEl("highlight", p.highlight));
    if (p.underline) {
        QByteArray e = "<w:u" + valAttr("val", p.underline->val);
        if (!p.underline->color.isEmpty()) e += valAttr("color", p.underline->color);
        parts.add(runChildRank("u"), e + p.underline->attrs + "/>");
    }
    if (!p.vertAlign.isEmpty()) parts.add(runChildRank("vertAlign"), valEl("vertAlign", p.vertAlign));
    for (const RawChild &c : p.extra) parts.add(c.rank, c.xml);
    return parts.joined();
}

QByteArray paraPropsXml(const ParaProps &p)
{
    Parts parts;
    if (!p.style.isEmpty()) parts.add(paraChildRank("pStyle"), valEl("pStyle", p.style));
    auto flag = [&](const char *name, const std::optional<bool> &v) {
        if (v) parts.add(paraChildRank(name), onOff(name, *v));
    };
    flag("keepNext", p.keepNext);
    flag("keepLines", p.keepLines);
    flag("pageBreakBefore", p.pageBreakBefore);
    flag("widowControl", p.widowControl);
    if (p.numId || p.ilvl) {
        QByteArray e = "<w:numPr>";
        if (p.ilvl) e += intEl("ilvl", *p.ilvl);
        if (p.numId) e += intEl("numId", *p.numId);
        parts.add(paraChildRank("numPr"), e + "</w:numPr>");
    }
    if (p.tabs && !p.tabs->isEmpty()) {
        QByteArray e = "<w:tabs>";
        for (const TabStop &t : *p.tabs) {
            e += "<w:tab" + valAttr("val", t.val.isEmpty() ? QStringLiteral("left") : t.val);
            if (!t.leader.isEmpty()) e += valAttr("leader", t.leader);
            e += intAttr("pos", t.pos) + "/>";
        }
        parts.add(paraChildRank("tabs"), e + "</w:tabs>");
    }
    if (p.before || p.after || p.line || !p.lineRule.isEmpty() || !p.spacingAttrs.isEmpty()) {
        QByteArray e = "<w:spacing";
        if (p.before) e += intAttr("before", *p.before);
        if (p.after) e += intAttr("after", *p.after);
        if (p.line) e += intAttr("line", *p.line);
        if (!p.lineRule.isEmpty()) e += valAttr("lineRule", p.lineRule);
        parts.add(paraChildRank("spacing"), e + p.spacingAttrs + "/>");
    }
    if (p.indLeft || p.indRight || p.indFirst || !p.indAttrs.isEmpty()) {
        QByteArray e = "<w:ind";
        if (p.indLeft) e += intAttr("left", *p.indLeft);
        if (p.indRight) e += intAttr("right", *p.indRight);
        if (p.indFirst && *p.indFirst > 0) e += intAttr("firstLine", *p.indFirst);
        if (p.indFirst && *p.indFirst < 0) e += intAttr("hanging", -*p.indFirst);
        if (p.indFirst && *p.indFirst == 0) e += intAttr("firstLine", 0);
        parts.add(paraChildRank("ind"), e + p.indAttrs + "/>");
    }
    flag("contextualSpacing", p.contextualSpacing);
    if (!p.jc.isEmpty()) parts.add(paraChildRank("jc"), valEl("jc", p.jc));
    if (p.outlineLvl) parts.add(paraChildRank("outlineLvl"), intEl("outlineLvl", *p.outlineLvl));
    if (p.markRun) {
        const QByteArray inner = runPropsXml(*p.markRun, true);
        if (!inner.isEmpty()) parts.add(paraChildRank("rPr"), "<w:rPr>" + inner + "</w:rPr>");
    }
    if (p.section) parts.add(paraChildRank("sectPr"), sectionPropsXml(*p.section));
    for (const RawChild &c : p.extra) parts.add(c.rank, c.xml);
    return parts.joined();
}

QByteArray sectionPropsXml(const SectionProps &s)
{
    Parts parts;
    if (!s.type.isEmpty()) parts.add(sectionChildRank("type"), valEl("type", s.type));
    QByteArray sz = "<w:pgSz" + intAttr("w", s.pageW) + intAttr("h", s.pageH);
    if (s.orient == QLatin1String("landscape")) sz += valAttr("orient", s.orient);
    parts.add(sectionChildRank("pgSz"), sz + s.pgSzAttrs + "/>");
    parts.add(sectionChildRank("pgMar"), "<w:pgMar" + intAttr("top", s.top) + intAttr("right", s.right) + intAttr("bottom", s.bottom) + intAttr("left", s.left) +
                                             intAttr("header", s.header) + intAttr("footer", s.footer) + intAttr("gutter", s.gutter) + s.pgMarAttrs + "/>");
    if (s.pageStart || !s.pageFormat.isEmpty()) {
        QByteArray e = "<w:pgNumType";
        if (!s.pageFormat.isEmpty()) e += valAttr("fmt", s.pageFormat);
        if (s.pageStart) e += intAttr("start", *s.pageStart);
        parts.add(sectionChildRank("pgNumType"), e + s.pgNumAttrs + "/>");
    }
    if (s.cols > 1 || s.hasCols || !s.colDefs.isEmpty()) {
        QByteArray e = "<w:cols";
        if (s.cols > 1) e += intAttr("num", s.cols);
        e += intAttr("space", s.colSpace);
        if (!s.colEqual) e += " w:equalWidth=\"0\"";
        if (s.colSep) e += " w:sep=\"1\"";
        e += s.colsAttrs;
        if (!s.colDefs.isEmpty()) {
            e += ">";
            for (const ColumnDef &c : s.colDefs) e += "<w:col" + intAttr("w", c.width) + intAttr("space", c.space) + "/>";
            e += "</w:cols>";
        } else {
            e += "/>";
        }
        parts.add(sectionChildRank("cols"), e);
    }
    if (s.titlePg) parts.add(sectionChildRank("titlePg"), "<w:titlePg/>");
    for (const RawChild &c : s.extra) parts.add(c.rank, c.xml);
    return "<w:sectPr" + s.attrs + ">" + parts.joined() + "</w:sectPr>";
}

} // namespace jp::office::wml
