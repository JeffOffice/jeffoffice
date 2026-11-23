#include "core/styleinspect.h"

#include "core/editops.h"
#include "core/styleops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QTextBlock>

namespace jp::write {

namespace {

QString say(const char *text) { return QCoreApplication::translate("jp::write::Inspector", text); }

bool differs(const QTextFormat &a, const QTextFormat &b, int key) { return a.property(key) != b.property(key); }

QString familyOf(const QTextCharFormat &f)
{
    if (f.hasProperty(QTextFormat::FontFamilies) && !f.fontFamilies().toStringList().isEmpty()) return f.fontFamilies().toStringList().first();
    return f.stringProperty(tp::ThemeFont);
}

QColor colorOf(const QTextCharFormat &f) { return f.hasProperty(QTextFormat::ForegroundBrush) && f.foreground().style() != Qt::NoBrush ? f.foreground().color() : QColor(); }

} // namespace

AppliedFormatting appliedFormattingAt(WriteDoc &doc, const QTextCursor &c)
{
    AppliedFormatting out;
    QTextCursor at = c;
    if (at.hasSelection()) at.setPosition(at.selectionStart() + 1);   // (the letter the selection begins with, as the dialogs read it)
    const QTextBlock block = at.block();
    const QTextBlockFormat bf = block.blockFormat();
    QTextCharFormat cf = at.charFormat();
    if (isAtom(cf) || cf.fontPointSize() <= 0) cf = markFormat(block);
    const QString has = bf.stringProperty(tp::StyleName);
    out.paragraphStyle = has.isEmpty() ? doc.defaultParagraphStyle() : has;
    const std::optional<office::wml::Style> ps = styleOfDocument(doc, out.paragraphStyle);
    out.paragraphStyleName = ps ? styleScreenName(*ps) : out.paragraphStyle;
    out.characterStyle = cf.stringProperty(tp::CharStyle);
    if (!out.characterStyle.isEmpty()) {
        const std::optional<office::wml::Style> cs = styleOfDocument(doc, out.characterStyle);
        out.characterStyleName = cs ? styleScreenName(*cs) : out.characterStyle;
    }
    // The paragraph: what its format says that its style's does not.
    const QTextBlockFormat sbf = doc.styleBlockFormat(out.paragraphStyle);
    if (differs(bf, sbf, QTextFormat::BlockAlignment) && (bf.alignment() & Qt::AlignHorizontal_Mask) != (sbf.alignment() & Qt::AlignHorizontal_Mask)) out.paragraphDirect << say("Alignment");
    if (!bf.hasProperty(prop::NumId)) {
        if (!qFuzzyCompare(bf.leftMargin() + 1, sbf.leftMargin() + 1) || !qFuzzyCompare(bf.rightMargin() + 1, sbf.rightMargin() + 1) || !qFuzzyCompare(bf.textIndent() + 1, sbf.textIndent() + 1)) out.paragraphDirect << say("Indents");
    }
    if (!qFuzzyCompare(bf.topMargin() + 1, sbf.topMargin() + 1) || !qFuzzyCompare(bf.bottomMargin() + 1, sbf.bottomMargin() + 1)) out.paragraphDirect << say("Spacing before or after");
    if (bf.lineHeightType() != sbf.lineHeightType() || !qFuzzyCompare(bf.lineHeight() + 1, sbf.lineHeight() + 1)) out.paragraphDirect << say("Line spacing");
    if (bf.tabPositions() != sbf.tabPositions()) out.paragraphDirect << say("Tab stops");
    if (differs(bf, sbf, tp::Borders)) out.paragraphDirect << say("Borders and shading");
    if (bf.boolProperty(tp::KeepWithNext) != sbf.boolProperty(tp::KeepWithNext) || bf.boolProperty(tp::KeepTogether) != sbf.boolProperty(tp::KeepTogether)) out.paragraphDirect << say("Keep lines together");
    // The letters: what the run says that the paragraph's style and its character style do not.
    const QTextCharFormat base = doc.styleCharFormat(out.paragraphStyle, out.characterStyle);
    if (familyOf(cf) != familyOf(base)) out.characterDirect << say("Font");
    if (cf.fontPointSize() > 0 && base.fontPointSize() > 0 && !qFuzzyCompare(cf.fontPointSize(), base.fontPointSize())) out.characterDirect << say("Size");
    if ((cf.fontWeight() >= QFont::DemiBold) != (base.fontWeight() >= QFont::DemiBold)) out.characterDirect << say("Bold");
    if (cf.fontItalic() != base.fontItalic()) out.characterDirect << say("Italic");
    if ((cf.underlineStyle() != QTextCharFormat::NoUnderline) != (base.underlineStyle() != QTextCharFormat::NoUnderline)) out.characterDirect << say("Underline");
    if (cf.fontStrikeOut() != base.fontStrikeOut()) out.characterDirect << say("Strikethrough");
    if (colorOf(cf) != colorOf(base)) out.characterDirect << say("Color");
    if (cf.stringProperty(tp::HighlightRefP) != base.stringProperty(tp::HighlightRefP)) out.characterDirect << say("Highlight");
    if (cf.fontCapitalization() != base.fontCapitalization()) out.characterDirect << say("Capitals");
    if (cf.verticalAlignment() != base.verticalAlignment()) out.characterDirect << say("Superscript or subscript");
    if (!qFuzzyCompare(cf.fontLetterSpacing() + 1, base.fontLetterSpacing() + 1)) out.characterDirect << say("Character spacing");
    return out;
}

void clearDirectParagraphFormatting(WriteDoc &doc, QTextCursor &c)
{
    // The paragraph is given its style again: its own look goes, and what the file keeps of it and its list stay (applyParagraphStyle).
    const AutoRedefineOff noAutoUpdate;
    applyParagraphStyle(doc, c, paragraphStyleAt(doc, c));
}

void clearDirectCharacterFormatting(WriteDoc &doc, QTextCursor &c) { clearDirectCharacters(doc, c); }

} // namespace jp::write
