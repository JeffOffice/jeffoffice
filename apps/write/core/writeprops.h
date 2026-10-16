#pragma once
// What JeffWrite keeps in its text's formats beyond Qt's and libs/text's own properties: pieces of the file kept as XML, the
// properties of a paragraph or run that have no field here, a paragraph's list, and its place in a paragraph that a page break split.

#include "wml/wml.h"

#include <QByteArray>
#include <QTextFormat>
#include <QVector>

namespace jp::write {

namespace prop {
enum : int {
    // character formats
    Atom = QTextFormat::UserProperty + 400,   // QByteArray: a piece of the file kept as XML (packAtom); the text is U+FFFC
    RawRPr,                                   // QByteArray: run properties kept as written (packRaw)
    ColorAttrs,                               // QString: "themeColor;themeTint;themeShade" of the color while it is ColorAttrsRgb
    ColorAttrsRgb,                            // QString: the color those attributes gave, "RRGGBB"
    DoubleStrike,                             // bool
    Hidden,                                   // bool: hidden text
    Raised,                                   // int: half-points raised
    // block formats
    RawBefore = QTextFormat::UserProperty + 430,   // QByteArray: body elements kept as XML that come before the paragraph (packBlocks)
    RawAfter,                                 // QByteArray: the same after it
    RawBlock,                                 // QByteArray: the paragraph stands in for a body element kept as XML (packBlocks of one); it can't be edited
    ParaAttrs,                                // QByteArray: the attributes of w:p as written
    RawPPr,                                   // QByteArray: paragraph properties kept as written (packRaw)
    NumId,                                    // int: the list definition (w:numId) of a paragraph in a list
    NumLevel,                                 // int: its level, 0 to 8
    Outline,                                  // int: outline level 0 to 8, 9 for body text
    Contextual,                               // bool: don't add space between paragraphs of the same style
    JoinNext,                                 // bool: a page break follows, and the next paragraph is the rest of this one
    Continued,                                // bool: this paragraph is the rest of the one before, after a page break
    SectionData,                              // QByteArray: the section this paragraph ends (packSection), with flow::fp::SectionBreakAfter
    SpacingAttrs,                             // QByteArray: attributes of w:spacing that have no field, as written
    IndAttrs,                                 // QByteArray: the same for w:ind
};
}

// A piece kept as XML, with what the writer needs to keep it whole.
QByteArray packAtom(const office::wml::Inline &atom);
office::wml::Inline unpackAtom(const QByteArray &packed);

// Body elements kept as XML.
QByteArray packBlocks(const QVector<office::wml::Block> &blocks);
QVector<office::wml::Block> unpackBlocks(const QByteArray &packed);

// Properties kept as written, with their places.
QByteArray packRaw(const QVector<office::wml::RawChild> &children);
QVector<office::wml::RawChild> unpackRaw(const QByteArray &packed);

} // namespace jp::write
