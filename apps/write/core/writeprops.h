#pragma once
// What JeffWrite keeps in its text's formats beyond Qt's and libs/text's own properties: pieces of the file kept as XML, the
// properties of a paragraph or run that have no field here, and a paragraph's list.

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
    SectionData,                              // QByteArray: the section this paragraph ends (packSection), with flow::fp::SectionBreakAfter
    SpacingAttrs,                             // QByteArray: attributes of w:spacing that have no field, as written
    IndAttrs,                                 // QByteArray: the same for w:ind
    FontsOther,                               // QString: the fonts of the scripts the engine has none for, as a run says them (packOtherFonts)
    NumDirect,                                // bool: the paragraph says it is numbered as its style says (and so the list's indents are over the style's)
    JcRaw,                                    // QString: the alignment as the file wrote it, when it is a kind of justified ("distribute")
    OddTabs,                                  // QByteArray: tab stops of the paragraph's own that the engine has no kind for (a bar), packed (packTabs)
};
}

// The bytes that formats keep beside the text (a piece of the file kept as XML, the attributes of a paragraph...) are not in the formats, whose
// properties of that kind Qt does not hash (so that each one inserted would be compared with every one before it): a format holds a key, an
// int, and the bytes are in a store that is the same for every document. Equal bytes have the same key. The store is emptied when the last
// document is closed; `generation` says which store a key is of (a copy on the clipboard is of one).
int storeBytes(const QByteArray &bytes);
QByteArray storedBytesOf(int key);
void setStored(QTextFormat &format, int property, const QByteArray &bytes);
QByteArray storedBytes(const QTextFormat &format, int property);
void clearByteStore();
quint64 byteStoreGeneration();

// A piece kept as XML, with what the writer needs to keep it whole.
QByteArray packAtom(const office::wml::Inline &atom);
office::wml::Inline unpackAtom(const QByteArray &packed);

// Body elements kept as XML.
QByteArray packBlocks(const QVector<office::wml::Block> &blocks);
QVector<office::wml::Block> unpackBlocks(const QByteArray &packed);

// Tab stops.
// What a run says of its East Asian and complex-script fonts and its hint, in one string; and back.
QString packOtherFonts(const office::wml::Fonts &fonts);
office::wml::Fonts unpackOtherFonts(const QString &packed);
QByteArray packTabs(const QVector<office::wml::TabStop> &tabs);
QVector<office::wml::TabStop> unpackTabs(const QByteArray &packed);

// Properties kept as written, with their places.
QByteArray packRaw(const QVector<office::wml::RawChild> &children);
QVector<office::wml::RawChild> unpackRaw(const QByteArray &packed);

} // namespace jp::write
