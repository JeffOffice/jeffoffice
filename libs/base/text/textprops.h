#pragma once
// Custom QTextFormat properties JeffPub stores alongside Qt's own.

#include <QColor>
#include <QString>
#include <QStringList>
#include <QTextFormat>
#include <QTextListFormat>

#include <cmath>

namespace jp {
// Qt's list styles are all negative; these three are bullets, the rest numbers.
inline bool isBulletList(QTextListFormat::Style s)
{
    return s == QTextListFormat::ListDisc || s == QTextListFormat::ListCircle || s == QTextListFormat::ListSquare;
}
} // namespace jp

namespace jp::tp {

enum : int {
    // character
    ColorRefP = QTextFormat::UserProperty + 1,   // QString ColorRef for the text color
    HighlightRefP,                               // QString ColorRef for highlight
    ThemeFont,                                   // QString "major" or "minor": follows the font scheme
    Field,                                       // QString field code, run text is U+FFFC
    CharStyle,                                   // QString character style name
    Shadow,                                      // bool
    Emboss,                                      // bool
    Engrave,                                     // bool
    OutlineRef,                                  // QString ColorRef for text outline
    OutlineWidth,                                // double points
    Language,                                    // QString BCP-47
    NumberStyle,                                 // int: 0 default, 1 lining, 2 old-style
    NumberSpacing,                               // int: 0 default, 1 proportional, 2 tabular
    Ligatures,                                   // int: 0 standard, 1 none, 2 all
    StylisticSet,                                // int 0..20
    Swash,                                       // bool
    Alternates,                                  // bool
    TrueSmallCaps,                               // bool
    TextFill,                                    // QString Fill JSON (gradient text)
    GlowRef,                                     // QString ColorRef, glow on text
    NoProof,                                     // bool, skip spelling
    KernAbove,                                   // double: automatic pair kerning from this size up (points; 14 when unset)
    Tracking,                                    // double: percent of normal letter spacing (100 when unset)
    LineSize,                                    // double: layout only, the (Qt) size a run's line spacing counts at
    InlineObject,                                // QString item JSON: an object set in the text, run text is U+FFFC
    GlyphScaleY,                                 // double: layout only, letters drawn this much taller (a stand-in imitating a missing font's height)
    UnderlineKind,                               // int flags beside the underline's style: 1 double, 2 words only (not under spaces), 4 thick
    SyntheticItalic,                             // double: layout only, the upright letters are drawn slanted this much (a stand-in for a font with no italic)
    HandBold,                                    // int: layout only, the letters are bold drawn by hand (a font with no bold face): 1 when bold is set, 2 whether or not (Arial Black, Rockwell Extra Bold)
    TextFx,                                      // QString TextEffects JSON: a:effectLst of a run (shadows, glow, reflection), drawn under the letters
    HiddenText,                                  // bool: the run is not shown or counted (LayoutEnv::wordLines)
    DisplayFamily,                               // QString: layout only, the family a run is set in when it isn't the family the format says (a symbol's font): never written to a file (LayoutEnv::wordLines)
    RunBorder,                                   // QString: a border round the run, "style;eighths;space;RRGGBB" (style single, double, dashed...; space in points; color empty for automatic): drawn by LayoutEnv::wordLines text
    DisplayStyle,                                // QString: layout only, how a run is set when its format does not say: "i" italic, "b" bold, "ib" both (an equation's letters, a styled letter of the mathematical alphanumeric block shown as its letter): never written to a file (LayoutEnv::wordLines)
    BlankFieldResult,                            // bool: layout only, the run is the result of a field (a reference) that is nothing but blanks: it is shown, and takes no part in its line's height (LayoutEnv::wordLines)

    // block
    StyleName = QTextFormat::UserProperty + 100, // QString paragraph style
    DropCapLines,                                // int
    DropCapChars,                                // int
    DropCapFont,                                 // QString
    DropCapColor,                                // QString ColorRef
    DropCapUpper,                                // bool: raise above first line instead of dropping
    KeepWithNext,                                // bool
    KeepTogether,                                // bool
    WidowControl,                                // bool
    StartInNextBox,                              // bool
    AlignToBaseline,                             // bool
    Distribute,                                  // bool: distributed alignment
    BulletChar,                                  // QString custom bullet
    BulletFont,                                  // QString
    BulletColor,                                 // QString ColorRef
    ListLevel,                                   // int
    NumberFormat,                                // int: 0 bullet, 1 "1.", 2 "a.", 3 "A.", 4 "i.", 5 "I.", 6 "1)", 7 "(1)"
    NumberStart,                                 // int
    ListId,                                      // QString: blocks with the same id share numbering
    TabLeaders,                                  // QString per-tab leader characters
    BulletSize,                                  // double points: a list marker's own size (list format)
    TocLevel,                                    // int: a table of contents' paragraph (0 its title, 1-3 an entry), rebuilt by Update
    BulletTab,                                   // double points: the list's indent: its text starts this far from the text area's edge, or at the bullet's end (list format)
    SpaceBeforeShare,                            // double: a slide paragraph's space before as a share of its line (0.2: 20 percent), where its file gave it so; the top margin holds what that came to
    SpaceAfterShare,                             // double: the same for the space after (the bottom margin)
    SpaceBeforeFrom,                             // double: the top margin (points) the share came to when it was set; the share counts only while the margin still is that
    SpaceAfterFrom,                              // double: the same for the bottom margin
    ContextualSpacing,                           // bool: no space before or after between paragraphs of this style (LayoutEnv::wordLines)
    OutOfFlow,                                   // bool: a paragraph set where the next one begins and taking no room (a word processor's frame beside the text); LayoutEnv::wordLines
    Borders,                                     // QString: the paragraph's borders and shading (ParaBox::spec), drawn by LayoutEnv::wordLines text
    BorderTopOff,                                // bool: the paragraph is the rest of one a page began, or one whose box the page before began (no top border)
    SpaceBeforeAuto,                             // bool: the space before is a word processor's automatic spacing (14 points), not the top margin (LayoutEnv::wordLines)
    SpaceAfterAuto,                              // bool: the same for the space after
    BorderBottomOff,                             // bool: the paragraph's box goes on after it, on the next page (no bottom border)
    NoHangPunct,                                 // bool: no mark of punctuation hangs past the margin in the paragraph (a word processor's overflowPunct turned off; LayoutEnv::wordLines)
    ListKey,                                     // int (list format): which list an item is in, where a file's lists are told apart (a word processor's numbering id; 0 when none)
    BulletOutline,                               // bool (list format): the marker is drawn hollow, as its level's outline effect asks (LayoutEnv::wordLines)
    NumberTab,                                   // double (block): the list tab stop the paragraph's numbering gives it (a word processor's "num" tab), in points from the text area's edge (LayoutEnv::wordLines)
    NoteMarkCustom,                              // bool: a note's reference whose mark is the text after it (w:customMarkFollows): it shows no number and takes none (LayoutEnv::wordLines)
    OutOfLayout,                                 // bool: the paragraph is set apart from the text (a word processor's frame, which the page sets where it asks): it has no lines and no room, and shows nothing (LayoutEnv::wordLines)
    BoxIndent,                                   // double (block): the first-line indent the paragraph's box begins from, where its text's own was taken off (the piece that goes on after a page break)
    LegacyNumber,                                // bool (list format): the text after the number begins at the indent, whatever the file says of the hanging indent as a tab stop (a level of an older program's list; LayoutEnv::wordLines)
    BulletBold,                                  // bool (list format): the marker is bold or, when false, not, as its level's run properties say; absent: as the paragraph's mark (LayoutEnv::wordLines)
    BulletItalic,                                // bool (list format): the same for italic
    NoAutoHyphen,                                // bool (paragraph): its words are not hyphenated by the layout though the story's frames hyphenate (a word processor's suppressAutoHyphens)
    AlignTab,                                    // QString (on a tab): a word processor's absolute position tab, "right|center|left:margin|indent:none|dot|hyphen|underscore|middleDot": the text after it goes to that place, whatever the paragraph's stops (LayoutEnv::wordLines)
    DisplayAlignment,                            // int (block): layout only, the paragraph is set with this horizontal alignment (Qt::Alignment) where its format does not say (an equation alone in its paragraph is centered): never written to a file (LayoutEnv::wordLines)
    PageSideAlign,                               // int (block): the paragraph is aligned to the side of the page's outside (1) or inside (2) by the page's number: the right and the left, or the left and the right, of a page with an odd and an even number (LayoutEnv::wordLines)
    NumberSuffixKind,                            // int (list format): what follows the number or bullet where it is not a tab: 1 a space, 2 nothing (a word processor's w:suff); absent: a tab (LayoutEnv::wordLines)
};

// The block property that holds the key of the table cell a paragraph is in (libs/flow's fp::Cell, which is this number): the text engine never joins paragraphs of two cells, or
// a cell's and one outside a table, whatever their marks (LayoutEnv::wordLines).
constexpr int TableCell = QTextFormat::UserProperty + 302;

// A paragraph's borders and shading as a word processor draws them (LayoutEnv::wordLines): each side's line style, width in eighths of a point, and its
// distance from the text in points, and the color the paragraph is filled with. Kept in a block's tp::Borders as text, ParaBox::spec.
struct BoxSide {
    QString style;                 // "single", "double", "dotted", "dashed", "thick"; empty for none
    int eighths = 0;
    int space = 0;
    QColor color;                  // invalid: automatic (black)
    bool visible() const { return !style.isEmpty() && eighths > 0; }
    // The width of the line or lines in points (a double border is three times a line: two lines and the gap between).
    double width() const { return eighths / 8.0 * (style == QLatin1String("double") ? 3 : 1); }
    // The room it takes above or below the text: its width and the space.
    double reserve() const { return visible() ? width() + space : 0.0; }
    bool operator==(const BoxSide &o) const { return style == o.style && eighths == o.eighths && space == o.space && color == o.color; }
};
struct ParaBox {
    BoxSide top, left, bottom, right, between;
    QColor fill;                   // invalid: none
    bool any() const { return top.visible() || left.visible() || bottom.visible() || right.visible() || between.visible() || fill.isValid(); }
    bool operator==(const ParaBox &o) const { return top == o.top && left == o.left && bottom == o.bottom && right == o.right && between == o.between && fill == o.fill; }
    // "T=single,4,1,FF0000;L=...;B=...;R=...;I=...;S=C0C0C0": a side by its letter (I: between), its style, width, space, and color (none for automatic).
    QString spec() const
    {
        QStringList parts;
        auto add = [&](const char *letter, const BoxSide &s) {
            if (s.visible())
                parts << QStringLiteral("%1=%2,%3,%4,%5").arg(QLatin1String(letter), s.style).arg(s.eighths).arg(s.space).arg(s.color.isValid() ? s.color.name().mid(1) : QStringLiteral("none"));
        };
        add("T", top);
        add("L", left);
        add("B", bottom);
        add("R", right);
        add("I", between);
        if (fill.isValid()) parts << QStringLiteral("S=") + fill.name().mid(1);
        return parts.join(QLatin1Char(';'));
    }
    static ParaBox parse(const QString &spec)
    {
        ParaBox b;
        for (const QString &part : spec.split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
            const QString key = part.section(QLatin1Char('='), 0, 0), value = part.section(QLatin1Char('='), 1);
            if (key == QLatin1String("S")) {
                b.fill = QColor(QLatin1Char('#') + value);
                continue;
            }
            const QStringList f = value.split(QLatin1Char(','));
            if (f.size() < 4) continue;
            BoxSide s;
            s.style = f[0];
            s.eighths = f[1].toInt();
            s.space = f[2].toInt();
            if (f[3] != QLatin1String("none")) s.color = QColor(QLatin1Char('#') + f[3]);
            if (key == QLatin1String("T")) b.top = s;
            else if (key == QLatin1String("L")) b.left = s;
            else if (key == QLatin1String("B")) b.bottom = s;
            else if (key == QLatin1String("R")) b.right = s;
            else if (key == QLatin1String("I")) b.between = s;
        }
        return b;
    }
};
// Whether text of the automatic color is white on this fill: it is dark (its brightness, by the weights of a gray picture of it, is under a quarter of the range). A fill that
// is none, or clear, is not.
inline bool isDarkFill(const QColor &fill) { return fill.isValid() && fill.alpha() > 0 && 0.299 * fill.red() + 0.587 * fill.green() + 0.114 * fill.blue() < 64; }

// A slide paragraph's space before (or after) as a share of its line, 0 when it has none in force. The share a file gave counts while the
// margin is still the points it came to (to the hundredth of a point that the Paragraph dialog shows it in, so pressing OK there changes
// nothing): a margin set to something else since ends it, and the text changing size doesn't.
inline double spaceShareOf(const QTextBlockFormat &bf, bool before)
{
    const double share = bf.doubleProperty(before ? SpaceBeforeShare : SpaceAfterShare);
    const double margin = before ? bf.topMargin() : bf.bottomMargin(), from = bf.doubleProperty(before ? SpaceBeforeFrom : SpaceAfterFrom);
    return share > 0 && std::isfinite(share) && std::abs(margin - from) < 0.0051 ? share : 0.0;
}

// Copies every property of `from` onto `to`. QTextFormat::properties()
// builds a new map on each call, so a loop must walk one copy of it: begin()
// of one call and end() of another belong to two maps (and the first is
// gone by then).
inline void setProperties(QTextFormat &to, const QTextFormat &from)
{
    const QMap<int, QVariant> props = from.properties();
    for (auto it = props.cbegin(); it != props.cend(); ++it) to.setProperty(it.key(), it.value());
}

// Publisher's two kinds of letter spacing. Tracking is a percentage (100
// normal); older files kept it as Qt percentage spacing. Kerning is space in
// points added after each letter, kept as Qt absolute spacing.
inline double trackingOf(const QTextCharFormat &f)
{
    if (f.hasProperty(Tracking)) return f.property(Tracking).toDouble();
    if (f.hasProperty(QTextFormat::FontLetterSpacing) && f.fontLetterSpacingType() == QFont::PercentageSpacing) return f.fontLetterSpacing();
    return 100;
}
inline double kerningOf(const QTextCharFormat &f)
{
    return f.hasProperty(QTextFormat::FontLetterSpacing) && f.fontLetterSpacingType() == QFont::AbsoluteSpacing ? f.fontLetterSpacing() : 0;
}

} // namespace jp::tp
