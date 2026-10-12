// Text bodies (drawingml.h): paragraphs, runs, line breaks, and fields read
// into a story as the text engine takes them, every property resolved
// through the list styles to a concrete format (the engine never sees the
// stack).

#include "office/drawingml.h"

#include "core/color.h"
#include "office/xml.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextOption>

#include <algorithm>
#include <cmath>
#include <optional>

namespace jp::office {

namespace {

// A stranger's file: no more than this many paragraphs, runs, or letters.
constexpr int kMaxParagraphs = 100000, kMaxRuns = 100000;
constexpr qsizetype kMaxText = 20000000;
constexpr int kLevels = 9;

QDomElement firstChild(const QDomElement &e, QLatin1String local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == local) return c;
    return QDomElement();
}

QString at(const QDomElement &e, QLatin1String name)
{
    return attr(e, QString(name));
}

bool intOf(const QDomElement &e, QLatin1String name, qint64 *out)
{
    const QString s = at(e, name);
    if (s.isEmpty()) return false;
    bool ok = false;
    const qint64 v = parseInt(s, 0, &ok);
    if (ok) *out = v;
    return ok;
}

bool boolOf(const QDomElement &e, QLatin1String name, bool *out)
{
    const QString s = at(e, name);
    if (s.isEmpty()) return false;
    bool ok = false;
    const bool v = parseBool(s, false, &ok);
    if (ok) *out = v;
    return ok;
}

// A length attribute, in points.
bool lengthOf(const QDomElement &e, QLatin1String name, double *out)
{
    const QString s = at(e, name);
    if (s.isEmpty()) return false;
    bool ok = false;
    const double v = parseLength(s, 0, &ok);
    if (!ok || !std::isfinite(v)) return false;
    *out = v;
    return true;
}

// A theme font reference in a typeface ("+mj-lt", "+mn-ea"): "major", "minor", or empty.
QString themeFontOf(const QString &typeface)
{
    if (typeface.startsWith(QLatin1String("+mj"))) return QStringLiteral("major");
    if (typeface.startsWith(QLatin1String("+mn"))) return QStringLiteral("minor");
    return QString();
}

// ---- what the elements say

// A distance that is points or a share of the line.
struct Spacing {
    enum Kind { Unset, Points, Percent } kind = Unset;
    double value = 0;   // points, or a fraction (1 is 100%)
};


struct ParaProps {
    QString algn;
    std::optional<double> marL, marR, indent, defTab;
    std::optional<bool> rtl;
    Spacing lnSpc, spcBef, spcAft;
    bool hasTabs = false;
    QList<QTextOption::Tab> tabs;
};

struct RunProps {
    std::optional<double> size;       // points
    std::optional<bool> bold, italic, noProof;
    QString underline, strike, cap;   // as written; empty for unsaid
    std::optional<double> baseline;   // a fraction of the size; above zero is a superscript
    std::optional<double> spacing;    // points added after each letter
    std::optional<double> kern;       // the size, in points, from which pairs are kerned
    QString lang;
    QString latin, ea, cs;            // typefaces as written
    enum Fill { FillUnset, FillNone, FillColor } fill = FillUnset;
    QColor color, highlight;
};

Spacing spacingOf(const QDomElement &e)
{
    Spacing s;
    if (const QDomElement pct = firstChild(e, QLatin1String("spcPct")); !pct.isNull()) {
        bool ok = false;
        const double v = parsePercent(at(pct, QLatin1String("val")), 0, &ok);
        if (ok && std::isfinite(v) && v >= 0) s = {Spacing::Percent, v};
    } else if (const QDomElement pts = firstChild(e, QLatin1String("spcPts")); !pts.isNull()) {
        qint64 v = 0;
        if (intOf(pts, QLatin1String("val"), &v) && v >= 0) s = {Spacing::Points, std::min<qint64>(v, 158400) / 100.0};
    }
    return s;
}

void mergeParagraph(ParaProps &p, const QDomElement &pPr, const DrawingContext &ctx)
{
    if (pPr.isNull()) return;
    if (const QString a = at(pPr, QLatin1String("algn")); !a.isEmpty()) p.algn = a;
    double v = 0;
    if (lengthOf(pPr, QLatin1String("marL"), &v)) p.marL = v;
    if (lengthOf(pPr, QLatin1String("marR"), &v)) p.marR = v;
    if (lengthOf(pPr, QLatin1String("indent"), &v)) p.indent = v;
    if (lengthOf(pPr, QLatin1String("defTabSz"), &v)) p.defTab = v;
    bool b = false;
    if (boolOf(pPr, QLatin1String("rtl"), &b)) p.rtl = b;
    for (QDomElement c = pPr.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString name = c.localName();
        if (name == QLatin1String("lnSpc")) {
            if (const Spacing s = spacingOf(c); s.kind != Spacing::Unset) p.lnSpc = s;
        } else if (name == QLatin1String("spcBef")) {
            if (const Spacing s = spacingOf(c); s.kind != Spacing::Unset) p.spcBef = s;
        } else if (name == QLatin1String("spcAft")) {
            if (const Spacing s = spacingOf(c); s.kind != Spacing::Unset) p.spcAft = s;
        } else if (name == QLatin1String("tabLst")) {
            p.hasTabs = true;
            p.tabs.clear();
            for (QDomElement t = c.firstChildElement(); !t.isNull() && p.tabs.size() < 32; t = t.nextSiblingElement()) {
                double pos = 0;
                if (t.localName() != QLatin1String("tab") || !lengthOf(t, QLatin1String("pos"), &pos) || pos < 0 || pos > 20000) continue;
                const QString algn = at(t, QLatin1String("algn"));
                QTextOption::Tab tab;
                tab.position = pos;
                tab.type = algn == QLatin1String("r") ? QTextOption::RightTab : algn == QLatin1String("ctr") ? QTextOption::CenterTab
                         : algn == QLatin1String("dec") ? QTextOption::DelimiterTab : QTextOption::LeftTab;
                if (tab.type == QTextOption::DelimiterTab) tab.delimiter = QLatin1Char('.');
                p.tabs << tab;
            }
        }
    }
}

// The first stop of a gradient, for text it fills.
QColor firstStopOf(const QDomElement &grad, const DrawingContext &ctx)
{
    const QDomElement stops = firstChild(grad, QLatin1String("gsLst"));
    return readColor(firstChild(stops, QLatin1String("gs")), ctx.theme, ctx.colors);
}

void mergeRun(RunProps &r, const QDomElement &rPr, const DrawingContext &ctx)
{
    if (rPr.isNull()) return;
    qint64 n = 0;
    bool b = false;
    if (intOf(rPr, QLatin1String("sz"), &n)) r.size = std::clamp(n / 100.0, 1.0, 4000.0);
    if (boolOf(rPr, QLatin1String("b"), &b)) r.bold = b;
    if (boolOf(rPr, QLatin1String("i"), &b)) r.italic = b;
    if (boolOf(rPr, QLatin1String("noProof"), &b)) r.noProof = b;
    if (const QString u = at(rPr, QLatin1String("u")); !u.isEmpty()) r.underline = u;
    if (const QString s = at(rPr, QLatin1String("strike")); !s.isEmpty()) r.strike = s;
    if (const QString c = at(rPr, QLatin1String("cap")); !c.isEmpty()) r.cap = c;
    if (const QString l = at(rPr, QLatin1String("lang")); !l.isEmpty()) r.lang = l;
    if (intOf(rPr, QLatin1String("spc"), &n)) r.spacing = std::clamp(n / 100.0, -1584.0, 1584.0);
    if (intOf(rPr, QLatin1String("kern"), &n) && n >= 0) r.kern = std::min(n / 100.0, 4000.0);
    if (const QString s = at(rPr, QLatin1String("baseline")); !s.isEmpty()) {
        bool ok = false;
        const double v = parsePercent(s, 0, &ok);
        if (ok && std::isfinite(v)) r.baseline = v;
    }
    for (QDomElement c = rPr.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString name = c.localName();
        if (name == QLatin1String("solidFill") || name == QLatin1String("gradFill")) {
            const QColor color = name == QLatin1String("solidFill") ? readColor(c, ctx.theme, ctx.colors) : firstStopOf(c, ctx);
            if (color.isValid()) {
                r.fill = RunProps::FillColor;
                r.color = color;
            }
        } else if (name == QLatin1String("noFill")) {
            r.fill = RunProps::FillNone;
        } else if (name == QLatin1String("highlight")) {
            const QColor color = readColor(c, ctx.theme, ctx.colors);
            if (color.isValid()) r.highlight = color;
        } else if (name == QLatin1String("latin")) {
            if (const QString face = at(c, QLatin1String("typeface")); !face.isEmpty()) r.latin = face;
        } else if (name == QLatin1String("ea")) {
            if (const QString face = at(c, QLatin1String("typeface")); !face.isEmpty()) r.ea = face;
        } else if (name == QLatin1String("cs")) {
            if (const QString face = at(c, QLatin1String("typeface")); !face.isEmpty()) r.cs = face;
        }
    }
}

// ---- the engine's formats

QString colorRefOf(const QColor &c)
{
    return ColorRef::rgb(c).toString();
}

QTextCharFormat charFormatOf(const RunProps &r)
{
    QTextCharFormat cf;
    cf.setFontPointSize(r.size.value_or(18.0));
    if (r.bold.value_or(false)) cf.setFontWeight(QFont::Bold);
    if (r.italic.value_or(false)) cf.setFontItalic(true);

    // The underline's line is Qt's style; double, words only, and heavy are
    // the engine's own kinds beside it.
    if (!r.underline.isEmpty() && r.underline != QLatin1String("none")) {
        QString u = r.underline;
        int kind = 0;
        if (u == QLatin1String("words")) {
            kind = 2;
            u = QStringLiteral("sng");
        } else if (u == QLatin1String("dbl")) {
            kind = 1;
            u = QStringLiteral("sng");
        } else if (u == QLatin1String("wavyDbl")) {
            kind = 1;
            u = QStringLiteral("wavy");
        } else if (u.endsWith(QLatin1String("Heavy"))) {
            kind = 4;
            u.chop(5);
        } else if (u == QLatin1String("heavy")) {
            kind = 4;
            u = QStringLiteral("sng");
        }
        QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline;
        if (u == QLatin1String("sng")) style = QTextCharFormat::SingleUnderline;
        else if (u == QLatin1String("dotted")) style = QTextCharFormat::DotLine;
        else if (u == QLatin1String("dash") || u == QLatin1String("dashLong")) style = QTextCharFormat::DashUnderline;
        else if (u == QLatin1String("dotDash")) style = QTextCharFormat::DashDotLine;
        else if (u == QLatin1String("dotDotDash")) style = QTextCharFormat::DashDotDotLine;
        else if (u == QLatin1String("wavy")) style = QTextCharFormat::WaveUnderline;
        if (style != QTextCharFormat::NoUnderline) {
            cf.setUnderlineStyle(style);
            if (kind) cf.setProperty(tp::UnderlineKind, kind);
        }
    }
    if (r.strike == QLatin1String("sngStrike") || r.strike == QLatin1String("dblStrike")) cf.setFontStrikeOut(true);
    if (r.cap == QLatin1String("all")) cf.setFontCapitalization(QFont::AllUppercase);
    else if (r.cap == QLatin1String("small")) cf.setFontCapitalization(QFont::SmallCaps);
    if (r.baseline && *r.baseline > 0) cf.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    else if (r.baseline && *r.baseline < 0) cf.setVerticalAlignment(QTextCharFormat::AlignSubScript);
    // Spacing is points after each letter, which is the engine's kerning
    // (tracking is a share of the font's average letter).
    if (r.spacing && *r.spacing != 0) {
        cf.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        cf.setFontLetterSpacing(*r.spacing);
    }
    if (r.kern) cf.setProperty(tp::KernAbove, *r.kern);

    // A theme font names no family (the engine takes the story's font
    // scheme); a family may be followed by the fonts of other scripts.
    QString theme = themeFontOf(r.latin);
    if (!r.latin.isEmpty() && theme.isEmpty()) {
        QStringList families{r.latin};
        for (const QString &face : {r.ea, r.cs})
            if (!face.isEmpty() && themeFontOf(face).isEmpty() && !families.contains(face)) families << face;
        cf.setFontFamilies(families);
    } else if (r.latin.isEmpty()) {
        theme = !themeFontOf(r.ea).isEmpty() ? themeFontOf(r.ea) : themeFontOf(r.cs);
    }
    if (!theme.isEmpty()) cf.setProperty(tp::ThemeFont, theme);

    if (r.fill == RunProps::FillColor || r.fill == RunProps::FillNone) {
        const QColor c = r.fill == RunProps::FillNone ? QColor(0, 0, 0, 0) : r.color;
        cf.setForeground(QBrush(c));
        cf.setProperty(tp::ColorRefP, colorRefOf(c));
    }
    if (r.highlight.isValid()) cf.setProperty(tp::HighlightRefP, colorRefOf(r.highlight));
    if (!r.lang.isEmpty()) cf.setProperty(tp::Language, r.lang);
    if (r.noProof.value_or(false)) cf.setProperty(tp::NoProof, true);
    return cf;
}



// ---- text

// A run's text as the story keeps it: tabs as they are, any kind of line
// break (including the vertical tab, however the file writes it) the
// engine's line separator, and no control characters or object marks.
QString cleanText(const QString &s)
{
    QString out;
    out.reserve(s.size());
    const QChar lineSeparator(QChar::LineSeparator);
    for (qsizetype i = 0; i < s.size(); ++i) {
        const QChar ch = s[i];
        const ushort u = ch.unicode();
        if (u == '_' && i + 7 <= s.size() && s[i + 1] == QLatin1Char('x') && s[i + 2] == QLatin1Char('0') && s[i + 3] == QLatin1Char('0') && s[i + 6] == QLatin1Char('_')) {
            // The escape of a control character in XML text (_x000B_).
            bool ok = false;
            const int code = s.mid(i + 4, 2).toInt(&ok, 16);
            if (ok && (code == 0x09 || code == 0x0A || code == 0x0B || code == 0x0D)) {
                out += code == 0x09 ? QChar(QLatin1Char('\t')) : lineSeparator;
                i += 6;
                continue;
            }
        }
        if (u == '\r' && i + 1 < s.size() && s[i + 1] == QLatin1Char('\n')) continue;
        if (u == '\n' || u == '\r' || u == 0x0B || u == QChar::ParagraphSeparator) out += lineSeparator;
        else if (u == QChar::ObjectReplacementCharacter) continue;
        else if (u < 0x20 && u != '\t') continue;
        else out += ch;
    }
    return out;
}

// ---- the reading

struct Piece {
    QString text;
    QTextCharFormat format;
};

class Reader {
public:
    Reader(QTextDocument *doc, const DrawingContext &ctx, TextStyleStack styles)
        : m_ctx(ctx), m_styles(std::move(styles)), m_cursor(doc)
    {
    }

    void paragraph(const QDomElement &p);
    bool full() const { return m_paragraphs >= kMaxParagraphs; }
    int paragraphs() const { return m_paragraphs; }

private:
    // What the list styles give a paragraph at `level` and a run in it: each
    // style's defaults, then its level, lowest priority first.
    void stack(int level);

    const DrawingContext &m_ctx;
    TextStyleStack m_styles;
    QTextCursor m_cursor;
    bool m_first = true;
    int m_runs = 0, m_paragraphs = 0;
    qsizetype m_letters = 0;
    bool m_have[kLevels] = {};
    ParaProps m_para[kLevels];   // what the styles give a paragraph of each level
    RunProps m_run[kLevels];     // and a run in it
};

void Reader::stack(int level)
{
    if (m_have[level]) return;
    m_have[level] = true;
    const QLatin1String names[kLevels] = {QLatin1String("lvl1pPr"), QLatin1String("lvl2pPr"), QLatin1String("lvl3pPr"), QLatin1String("lvl4pPr"), QLatin1String("lvl5pPr"),
                                          QLatin1String("lvl6pPr"), QLatin1String("lvl7pPr"), QLatin1String("lvl8pPr"), QLatin1String("lvl9pPr")};
    for (const QDomElement &style : std::as_const(m_styles)) {
        for (const QDomElement &pPr : {firstChild(style, QLatin1String("defPPr")), firstChild(style, names[level])}) {
            mergeParagraph(m_para[level], pPr, m_ctx);
            mergeRun(m_run[level], firstChild(pPr, QLatin1String("defRPr")), m_ctx);
        }
    }
}



void Reader::paragraph(const QDomElement &p)
{
    ++m_paragraphs;
    const QDomElement pPr = firstChild(p, QLatin1String("pPr"));
    qint64 lvl = 0;
    intOf(pPr, QLatin1String("lvl"), &lvl);
    const int level = int(std::clamp<qint64>(lvl, 0, kLevels - 1));
    stack(level);
    ParaProps para = m_para[level];
    mergeParagraph(para, pPr, m_ctx);
    const RunProps &base = m_run[level];
    const QTextCharFormat baseFormat = charFormatOf(base);

    // The paragraph's pieces of text, each with its format.
    QVector<Piece> pieces;
    double firstSize = 0, largest = 0;
    bool tabbed = false;
    auto add = [&](const QString &text, const QTextCharFormat &format) {
        if (text.isEmpty()) return;
        m_letters += text.size();
        if (firstSize == 0) firstSize = format.fontPointSize();
        largest = std::max(largest, format.fontPointSize());
        tabbed |= text.contains(QLatin1Char('\t'));
        pieces.append({text, format});
    };
    for (QDomElement c = p.firstChildElement(); !c.isNull() && m_runs < kMaxRuns && m_letters < kMaxText; c = c.nextSiblingElement()) {
        const QString name = c.localName();
        const bool isRun = name == QLatin1String("r"), isField = name == QLatin1String("fld"), isBreak = name == QLatin1String("br");
        if (!isRun && !isField && !isBreak) continue;
        ++m_runs;
        const QDomElement rPr = firstChild(c, QLatin1String("rPr"));
        QTextCharFormat format = baseFormat;
        if (!rPr.isNull()) {
            RunProps run = base;
            mergeRun(run, rPr, m_ctx);
            format = charFormatOf(run);
        }
        if (isBreak) {
            add(QString(QChar(QChar::LineSeparator)), format);
            continue;
        }
        const QString text = firstChild(c, QLatin1String("t")).text();
        add(cleanText(text), format);
    }

    // The mark after the last letter: what an empty paragraph is as tall as.
    RunProps endRun = base;
    mergeRun(endRun, firstChild(p, QLatin1String("endParaRPr")), m_ctx);
    const QTextCharFormat endFormat = charFormatOf(endRun);
    if (pieces.isEmpty()) {
        firstSize = largest = endFormat.fontPointSize();
    }

    QTextBlockFormat bf;
    Qt::Alignment align = Qt::AlignLeft;
    if (para.algn == QLatin1String("ctr")) align = Qt::AlignHCenter;
    else if (para.algn == QLatin1String("r")) align = Qt::AlignRight;
    else if (para.algn == QLatin1String("just") || para.algn == QLatin1String("justLow")) align = Qt::AlignJustify;
    else if (para.algn == QLatin1String("dist") || para.algn == QLatin1String("thaiDist")) {
        align = Qt::AlignJustify;
        bf.setProperty(tp::Distribute, true);
    }
    // The standard's left and right are the page's sides, whichever way the
    // text runs; the engine swaps a right-to-left paragraph's unless told.
    if (para.rtl.value_or(false)) {
        align |= Qt::AlignAbsolute;
        bf.setLayoutDirection(Qt::RightToLeft);
    }
    bf.setAlignment(align);
    bf.setLeftMargin(std::clamp(para.marL.value_or(0), 0.0, 20000.0));
    bf.setRightMargin(std::clamp(para.marR.value_or(0), 0.0, 20000.0));
    bf.setTextIndent(std::clamp(para.indent.value_or(0), -20000.0, 20000.0));
    // A share of the line is of one line of the largest text in the paragraph.
    auto distance = [&](const Spacing &s) {
        if (s.kind == Spacing::Points) return s.value;
        return s.kind == Spacing::Percent ? std::clamp(s.value * 1.2 * largest, 0.0, 4000.0) : 0.0;
    };
    if (para.spcBef.kind != Spacing::Unset) bf.setTopMargin(distance(para.spcBef));
    if (para.spcAft.kind != Spacing::Unset) bf.setBottomMargin(distance(para.spcAft));
    if (para.lnSpc.kind == Spacing::Percent && para.lnSpc.value > 0) bf.setLineHeight(std::clamp(para.lnSpc.value * 100, 1.0, 1000.0), QTextBlockFormat::ProportionalHeight);
    else if (para.lnSpc.kind == Spacing::Points && para.lnSpc.value > 0) bf.setLineHeight(std::max(para.lnSpc.value, 0.1), QTextBlockFormat::FixedHeight);
    if (level > 0) bf.setProperty(tp::ListLevel, level);
    // Stops of its own, then (where the paragraph has a tab) the default ones
    // as far on as they are likely to be needed.
    QList<QTextOption::Tab> tabs = para.tabs;
    if (tabbed && para.defTab && *para.defTab >= 1) {
        const double step = *para.defTab, last = tabs.isEmpty() ? 0.0 : tabs.last().position;
        int made = 0;
        for (int k = int(std::min(last / step, 1e6)) + 1; made < 64 && k * step <= 20000; ++k)
            if (k * step > last + 0.01) {
                tabs << QTextOption::Tab(k * step, QTextOption::LeftTab);
                ++made;
            }
    }
    if (!tabs.isEmpty()) bf.setTabPositions(tabs);

    if (m_first) {
        m_cursor.setBlockFormat(bf);
        m_cursor.setBlockCharFormat(endFormat);
        m_first = false;
    } else {
        m_cursor.insertBlock(bf, endFormat);
    }
    for (const Piece &piece : std::as_const(pieces)) m_cursor.insertText(piece.text, piece.format);

}

} // namespace

void readTextBody(const QDomElement &txBody, QTextDocument *doc, const DrawingContext &ctx, const TextStyleStack &styles)
{
    if (!doc) return;
    const bool undo = doc->isUndoRedoEnabled();
    doc->setUndoRedoEnabled(false);
    doc->clear();

    // The body's own list style is the highest layer (unless the shape has
    // handed it over as one already).
    TextStyleStack stack = styles;
    const QDomElement own = firstChild(txBody, QLatin1String("lstStyle"));
    if (!own.isNull() && (stack.isEmpty() || stack.last() != own)) stack.append(own);

    Reader reader(doc, ctx, stack);
    QTextCursor edit(doc);
    edit.beginEditBlock();
    for (QDomElement p = txBody.firstChildElement(); !p.isNull() && !reader.full(); p = p.nextSiblingElement())
        if (p.localName() == QLatin1String("p")) reader.paragraph(p);
    // A body with no paragraph is one empty paragraph, in the styles' size.
    if (reader.paragraphs() == 0) reader.paragraph(QDomElement());
    edit.endEditBlock();
    doc->setUndoRedoEnabled(undo);
}

} // namespace jp::office
