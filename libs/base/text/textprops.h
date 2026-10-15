#pragma once
// Custom QTextFormat properties JeffPub stores alongside Qt's own.

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
};

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
