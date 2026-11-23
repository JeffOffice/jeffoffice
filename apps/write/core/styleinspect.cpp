#include "core/styleinspect.h"

#include "core/editops.h"
#include "core/styleops.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <QTextBlock>
#include <algorithm>
#include <cmath>
#include <vector>

namespace jp::write {

namespace {

// The most letters' runs one look at a selection reads (the pane follows every move of the caret).
constexpr int kScanRuns = 400;

QString familyOf(const QTextFormat &f)
{
    if (f.hasProperty(QTextFormat::FontFamilies) && !f.property(QTextFormat::FontFamilies).toStringList().isEmpty()) return f.property(QTextFormat::FontFamilies).toStringList().first();
    return f.stringProperty(tp::ThemeFont);
}

QColor colorOf(const QTextFormat &f) { return f.hasProperty(QTextFormat::ForegroundBrush) && f.brushProperty(QTextFormat::ForegroundBrush).style() != Qt::NoBrush ? f.brushProperty(QTextFormat::ForegroundBrush).color() : QColor(); }

// One property the inspector lists and Clear takes off: the keys it is kept under (compared as the kind says: b bool, i int, d number, s text, v anything), the kept pieces of the file that
// say it too (by element name), and, where the keys alone don't tell, how to see that own and the style's agree.
struct Key {
    int id;
    char kind;
};
struct Prop {
    const char *label;
    std::vector<Key> keys;
    std::vector<const char *> raw;
    bool (*same)(const QTextFormat &own, const QTextFormat &base) = nullptr;
    bool (*skip)(const QTextFormat &own) = nullptr;   // not touched at all (a tracked deletion's own hiding)
    bool list = false;                                // the paragraph's list (taken off by the list commands, not by the keys)
};

bool sameKey(const QTextFormat &a, const QTextFormat &b, const Key &k)
{
    switch (k.kind) {
    case 'b': return a.boolProperty(k.id) == b.boolProperty(k.id);
    case 'i': return a.intProperty(k.id) == b.intProperty(k.id);
    case 'd': return std::abs(a.doubleProperty(k.id) - b.doubleProperty(k.id)) < 1e-6;
    case 's': return a.stringProperty(k.id) == b.stringProperty(k.id);
    default: return a.property(k.id) == b.property(k.id);
    }
}

bool rawNamed(const QByteArray &xml, const char *name)
{
    const int n = int(qstrlen(name));
    return xml.startsWith("<w:") && xml.mid(3, n) == name && (xml.size() <= 3 + n || !QChar(ushort(uchar(xml.at(3 + n)))).isLetterOrNumber());
}

bool hasRaw(const QTextFormat &f, int storeKey, const std::vector<const char *> &names)
{
    if (names.empty() || !f.hasProperty(storeKey)) return false;
    for (const office::wml::RawChild &c : unpackRaw(storedBytes(f, storeKey)))
        for (const char *n : names)
            if (rawNamed(c.xml, n)) return true;
    return false;
}

void stripRaw(QTextFormat &f, int storeKey, const std::vector<const char *> &names)
{
    if (!hasRaw(f, storeKey, names)) return;
    QVector<office::wml::RawChild> kept;
    for (const office::wml::RawChild &c : unpackRaw(storedBytes(f, storeKey))) {
        bool drop = false;
        for (const char *n : names) drop = drop || rawNamed(c.xml, n);
        if (!drop) kept.push_back(c);
    }
    if (kept.isEmpty()) f.clearProperty(storeKey);
    else setStored(f, storeKey, packRaw(kept));
}

bool isSame(const Prop &p, const QTextFormat &own, const QTextFormat &base, int storeKey)
{
    if (p.skip && p.skip(own)) return true;
    if (hasRaw(own, storeKey, p.raw)) return false;
    if (p.same) return p.same(own, base);
    for (const Key &k : p.keys)
        if (!sameKey(own, base, k)) return false;
    return true;
}

void reset(const Prop &p, QTextFormat &own, const QTextFormat &base, int storeKey)
{
    if (p.skip && p.skip(own)) return;
    for (const Key &k : p.keys) {
        if (base.hasProperty(k.id)) own.setProperty(k.id, base.property(k.id));
        else own.clearProperty(k.id);
    }
    stripRaw(own, storeKey, p.raw);
}

// ---- the paragraph's properties
bool sameAlignment(const QTextFormat &a, const QTextFormat &b) { return (a.toBlockFormat().alignment() & Qt::AlignHorizontal_Mask) == (b.toBlockFormat().alignment() & Qt::AlignHorizontal_Mask) && a.stringProperty(prop::JcRaw) == b.stringProperty(prop::JcRaw); }
bool sameTabs(const QTextFormat &a, const QTextFormat &b) { return a.toBlockFormat().tabPositions() == b.toBlockFormat().tabPositions() && a.stringProperty(tp::TabLeaders) == b.stringProperty(tp::TabLeaders) && a.property(prop::OddTabs) == b.property(prop::OddTabs); }
bool sameList(const QTextFormat &a, const QTextFormat &b) { return a.intProperty(prop::NumId) == b.intProperty(prop::NumId); }

const std::vector<Prop> &paragraphProps()
{
    static const std::vector<Prop> props = {
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Alignment"), {{QTextFormat::BlockAlignment, 'v'}, {prop::JcRaw, 's'}}, {}, sameAlignment},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Indents"), {{QTextFormat::BlockLeftMargin, 'd'}, {QTextFormat::BlockRightMargin, 'd'}, {QTextFormat::TextIndent, 'd'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Spacing before or after"), {{QTextFormat::BlockTopMargin, 'd'}, {QTextFormat::BlockBottomMargin, 'd'}, {tp::SpaceBeforeAuto, 'b'}, {tp::SpaceAfterAuto, 'b'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Line spacing"), {{QTextFormat::LineHeight, 'd'}, {QTextFormat::LineHeightType, 'i'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Tab stops"), {{QTextFormat::TabPositions, 'v'}, {tp::TabLeaders, 's'}, {prop::OddTabs, 'v'}}, {}, sameTabs},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Borders and shading"), {{tp::Borders, 's'}, {prop::BoxOwn, 'i'}}, {"pBdr", "shd"}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Keep with next"), {{tp::KeepWithNext, 'b'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Keep lines together"), {{tp::KeepTogether, 'b'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Page break before"), {{QTextFormat::PageBreakPolicy, 'i'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Outline level"), {{prop::Outline, 'v'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Widow and orphan control"), {{tp::WidowControl, 'b'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "No space between paragraphs of the same style"), {{tp::ContextualSpacing, 'b'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Right-to-left"), {}, {"bidi"}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Numbering or bullets"), {}, {}, sameList, nullptr, true},
    };
    return props;
}

// ---- the letters' properties
bool sameFont(const QTextFormat &a, const QTextFormat &b) { return familyOf(a) == familyOf(b); }
bool sameSize(const QTextFormat &a, const QTextFormat &b) { return !(a.doubleProperty(QTextFormat::FontPointSize) > 0 && b.doubleProperty(QTextFormat::FontPointSize) > 0) || std::abs(a.doubleProperty(QTextFormat::FontPointSize) - b.doubleProperty(QTextFormat::FontPointSize)) < 1e-6; }
bool sameBold(const QTextFormat &a, const QTextFormat &b) { return (a.intProperty(QTextFormat::FontWeight) >= QFont::DemiBold) == (b.intProperty(QTextFormat::FontWeight) >= QFont::DemiBold); }
bool sameUnderline(const QTextFormat &a, const QTextFormat &b) { return (a.intProperty(QTextFormat::TextUnderlineStyle) != QTextCharFormat::NoUnderline) == (b.intProperty(QTextFormat::TextUnderlineStyle) != QTextCharFormat::NoUnderline); }
bool sameStrike(const QTextFormat &a, const QTextFormat &b) { return a.boolProperty(QTextFormat::FontStrikeOut) == b.boolProperty(QTextFormat::FontStrikeOut) && a.boolProperty(prop::DoubleStrike) == b.boolProperty(prop::DoubleStrike); }
bool sameColor(const QTextFormat &a, const QTextFormat &b) { return colorOf(a) == colorOf(b); }
bool sameScale(const QTextFormat &a, const QTextFormat &b)
{
    auto scale = [](const QTextFormat &f) { return f.hasProperty(QTextFormat::FontStretch) && f.intProperty(QTextFormat::FontStretch) > 0 ? f.intProperty(QTextFormat::FontStretch) : 100; };
    return scale(a) == scale(b);
}
bool ownHiding(const QTextFormat &f) { return f.hasProperty(prop::TrackedDeleted); }   // (text a tracked change deleted stays hidden: the format says whether the style hides it itself)

const std::vector<Prop> &characterProps()
{
    static const std::vector<Prop> props = {
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Font"), {{QTextFormat::FontFamilies, 'v'}, {QTextFormat::FontFamily, 'v'}, {tp::ThemeFont, 's'}}, {}, sameFont},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Size"), {{QTextFormat::FontPointSize, 'd'}}, {}, sameSize},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Bold"), {{QTextFormat::FontWeight, 'i'}}, {}, sameBold},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Italic"), {{QTextFormat::FontItalic, 'b'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Underline"), {{QTextFormat::TextUnderlineStyle, 'i'}, {QTextFormat::TextUnderlineColor, 'v'}, {tp::UnderlineKind, 'i'}, {prop::UnderlineAttrs, 's'}}, {}, sameUnderline},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Strikethrough"), {{QTextFormat::FontStrikeOut, 'b'}, {prop::DoubleStrike, 'b'}}, {}, sameStrike},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Color"), {{QTextFormat::ForegroundBrush, 'v'}, {tp::ColorRefP, 's'}, {prop::ColorAttrs, 's'}, {prop::ColorAttrsRgb, 's'}}, {}, sameColor},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Highlight"), {{tp::HighlightRefP, 's'}, {prop::CharShade, 's'}}, {"shd"}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Capitals"), {{QTextFormat::FontCapitalization, 'i'}, {tp::TrueSmallCaps, 'b'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Superscript or subscript"), {{QTextFormat::TextVerticalAlignment, 'i'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Raised or lowered"), {{prop::Raised, 'i'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Character spacing"), {{QTextFormat::FontLetterSpacing, 'd'}, {QTextFormat::FontLetterSpacingType, 'i'}}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Character width"), {{QTextFormat::FontStretch, 'i'}}, {}, sameScale},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Hidden text"), {{tp::HiddenText, 'b'}}, {}, nullptr, ownHiding},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Border round the text"), {{tp::RunBorder, 's'}, {prop::CharBorder, 's'}}, {"bdr"}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Shadow or outline"), {{tp::Shadow, 'b'}, {tp::OutlineRef, 's'}, {tp::OutlineWidth, 'd'}}, {"shadow", "outline"}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Emboss or engrave"), {{tp::Emboss, 'b'}, {tp::Engrave, 'b'}}, {"emboss", "imprint"}},
        {QT_TRANSLATE_NOOP("jp::write::Inspector", "Do not check spelling"), {{tp::NoProof, 'b'}}},
    };
    return props;
}

// What the table style a paragraph is in gives its paragraphs, and its letters.
const office::wml::TableLayer *layerOf(WriteDoc &doc, const QTextBlockFormat &bf) { return bf.hasProperty(flow::fp::Cell) ? doc.tableLayerOf(bf.intProperty(flow::fp::Cell)) : nullptr; }

QString styleIdOf(WriteDoc &doc, const QTextBlockFormat &bf)
{
    const QString has = bf.stringProperty(tp::StyleName);
    return has.isEmpty() ? doc.defaultParagraphStyle() : has;
}

// What the paragraph has from its style, its table's style, and its list, as the file would write it (what it says beyond is its own).
QTextBlockFormat paragraphBase(WriteDoc &doc, const QTextBlockFormat &bf)
{
    const QString style = styleIdOf(doc, bf);
    const office::wml::TableLayer *layer = layerOf(doc, bf);
    QTextBlockFormat sbf = doc.styleBlockFormat(style, layer);
    if (!bf.hasProperty(prop::NumId)) return sbf;
    QTextBlockFormat base = blockFormatOf(doc.baselineParagraph(style, bf.intProperty(prop::NumId), bf.intProperty(prop::NumLevel), bf.boolProperty(prop::NumDirect), nullptr, layer));
    if (sbf.hasProperty(prop::NumId)) base.setProperty(prop::NumId, sbf.property(prop::NumId));
    else base.clearProperty(prop::NumId);
    return base;
}

QStringList paragraphLabels(WriteDoc &doc, const QTextBlock &block)
{
    QStringList out;
    const QTextBlockFormat bf = block.blockFormat();
    const QTextBlockFormat base = paragraphBase(doc, bf);
    for (const Prop &p : paragraphProps())
        if (!isSame(p, bf, base, prop::RawPPr)) out << QCoreApplication::translate("jp::write::Inspector", p.label);
    return out;
}

QStringList characterLabels(WriteDoc &doc, const QTextBlock &block, const QTextCharFormat &cf)
{
    QStringList out;
    const QTextBlockFormat bf = block.blockFormat();
    const QTextCharFormat base = doc.styleCharFormat(styleIdOf(doc, bf), cf.stringProperty(tp::CharStyle), layerOf(doc, bf));
    for (const Prop &p : characterProps())
        if (!isSame(p, cf, base, prop::RawRPr)) out << QCoreApplication::translate("jp::write::Inspector", p.label);
    return out;
}

QTextCharFormat runFormat(const QTextBlock &block, QTextCharFormat cf)
{
    if (isAtom(cf) || cf.fontPointSize() <= 0) cf = markFormat(block);
    return cf;
}

void addAll(QStringList *to, const QStringList &more)
{
    for (const QString &s : more)
        if (!to->contains(s)) to->push_back(s);
}

} // namespace

QStringList inspectedPropertyLabels(bool paragraph)
{
    QStringList out;
    for (const Prop &p : paragraph ? paragraphProps() : characterProps()) out << QCoreApplication::translate("jp::write::Inspector", p.label);
    return out;
}

AppliedFormatting appliedFormattingAt(WriteDoc &doc, const QTextCursor &c)
{
    AppliedFormatting out;
    const int from = c.selectionStart(), to = c.selectionEnd();
    QTextDocument *story = c.document();
    bool firstBlock = true, firstRun = true;
    QStringList firstLook;
    int runs = 0;
    // (A caret is the letter before it, as typing there would have; a selection is read paragraph by paragraph, and run by run.)
    for (QTextBlock block = story->findBlock(from); block.isValid() && (firstBlock || block.position() < to) && runs < kScanRuns; block = block.next()) {
        const QTextBlockFormat bf = block.blockFormat();
        const QString style = styleIdOf(doc, bf);
        const QStringList para = paragraphLabels(doc, block);
        if (firstBlock) {
            out.paragraphStyle = style;
            const std::optional<office::wml::Style> ps = styleOfDocument(doc, style);
            out.paragraphStyleName = ps ? styleScreenName(*ps) : style;
            if (bf.hasProperty(flow::fp::Cell) && bf.intProperty(flow::fp::Cell) > 0 && story == doc.story()) {
                const QString table = tablePropsAt(doc, c).style;
                if (!table.isEmpty()) {
                    const std::optional<office::wml::Style> ts = styleOfDocument(doc, table);
                    out.tableStyleName = ts ? styleScreenName(*ts) : table;
                }
            }
        } else if (style != out.paragraphStyle) {
            out.mixedParagraphStyle = true;
        }
        addAll(&out.paragraphDirect, para);
        QVector<QTextCharFormat> formats;
        if (!c.hasSelection()) {
            QTextCursor at = c;
            formats.push_back(runFormat(block, at.charFormat()));
        } else {
            for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                if (std::min(to, f.position() + f.length()) > std::max(from, f.position())) formats.push_back(runFormat(block, f.charFormat()));
            }
            if (formats.isEmpty() && from <= block.position() + block.length() - 1 && to > block.position()) formats.push_back(markFormat(block));
        }
        for (const QTextCharFormat &cf : std::as_const(formats)) {
            ++runs;
            const QString own = cf.stringProperty(tp::CharStyle);
            const QStringList look = characterLabels(doc, block, cf);
            if (firstRun) {
                out.characterStyle = own;
                if (!own.isEmpty()) {
                    const std::optional<office::wml::Style> cs = styleOfDocument(doc, own);
                    out.characterStyleName = cs ? styleScreenName(*cs) : own;
                }
                firstLook = look;
            } else {
                if (own != out.characterStyle) out.mixedCharacterStyle = true;
                if (QSet<QString>(look.begin(), look.end()) != QSet<QString>(firstLook.begin(), firstLook.end())) out.mixedLook = true;
            }
            firstRun = false;
            addAll(&out.characterDirect, look);
        }
        firstBlock = false;
    }
    return out;
}

QTextCursor inspectedRange(const QTextCursor &c)
{
    if (c.hasSelection()) return c;
    const QTextBlock b = c.block();
    if (b.length() <= 1) return c;
    const int at = std::max(c.position() - 1, b.position());
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (f.position() <= at && at < f.position() + f.length()) {
            QTextCursor run = c;
            run.setPosition(f.position());
            run.setPosition(f.position() + f.length(), QTextCursor::KeepAnchor);
            return run;
        }
    }
    return c;
}

void clearDirectParagraphFormatting(WriteDoc &doc, QTextCursor &c)
{
    const NoteScopeGuard scopeGuard(doc, c);
    const AutoRedefineOff noAutoUpdate;
    if (!intoBody(c)) return;
    c.beginEditBlock();
    // The list first (it takes its indents with it): a paragraph numbered by itself goes back to what its style says of numbering.
    for (const QTextBlock &b : selectedBlocks(c)) {
        const QTextBlockFormat bf = b.blockFormat();
        if (isLocked(b) || isSame(paragraphProps().back(), bf, paragraphBase(doc, bf), prop::RawPPr)) continue;
        QTextCursor one(b);
        if (doc.styleBlockFormat(styleIdOf(doc, bf), layerOf(doc, bf)).hasProperty(prop::NumId)) applyParagraphStyle(doc, one, styleIdOf(doc, bf));
        else clearList(doc, one);
    }
    changeBlockFormat(doc, c, [&](QTextBlockFormat &nf) {
        const QTextBlockFormat base = paragraphBase(doc, nf);
        for (const Prop &p : paragraphProps())
            if (!p.list && !isSame(p, nf, base, prop::RawPPr)) reset(p, nf, base, prop::RawPPr);
    });
    c.endEditBlock();
}

void clearDirectCharacterFormatting(WriteDoc &doc, QTextCursor &c)
{
    const NoteScopeGuard scopeGuard(doc, c);
    const AutoRedefineOff noAutoUpdate;
    if (!intoBody(c)) return;
    c.beginEditBlock();
    const int from = c.selectionStart(), to = c.selectionEnd();
    QTextDocument *story = c.document();
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() <= std::max(to, from); b = b.next()) {
        QTextCursor part(story);
        part.setPosition(std::max(from, b.position()));
        part.setPosition(std::min(to, b.position() + b.length() - 1), QTextCursor::KeepAnchor);
        const QTextBlockFormat bf = b.blockFormat();
        const QString paraStyle = styleIdOf(doc, bf);
        const office::wml::TableLayer *layer = layerOf(doc, bf);
        QHash<QString, QTextCharFormat> bases;
        changeCharFormat(doc, part, [&](QTextCharFormat &f) {
            const QString own = f.stringProperty(tp::CharStyle);
            if (!bases.contains(own)) bases.insert(own, doc.styleCharFormat(paraStyle, own, layer));
            const QTextCharFormat &base = bases[own];
            for (const Prop &p : characterProps())
                if (!isSame(p, f, base, prop::RawRPr)) reset(p, f, base, prop::RawRPr);
        });
    }
    c.endEditBlock();
}

void clearDirectCharacterStyles(WriteDoc &doc, QTextCursor &c)
{
    const NoteScopeGuard scopeGuard(doc, c);
    if (!intoBody(c)) return;
    QStringList styles;
    const int from = c.selectionStart(), to = c.selectionEnd();
    for (QTextBlock b = c.document()->findBlock(from); b.isValid() && b.position() <= std::max(to, from); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QString own = f.charFormat().stringProperty(tp::CharStyle);
            if (!own.isEmpty() && !styles.contains(own) && std::min(std::max(to, from + 1), f.position() + f.length()) > std::max(from, f.position())) styles << own;
        }
    c.beginEditBlock();
    for (const QString &s : std::as_const(styles)) clearCharacterStyle(doc, c, s);
    c.endEditBlock();
}

} // namespace jp::write
