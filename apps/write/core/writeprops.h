#pragma once
// What JeffWrite keeps in its text's formats beyond Qt's and libs/text's own properties: pieces of the file kept as XML, the
// properties of a paragraph or run that have no field here, and a paragraph's list.

#include <optional>
#include "wml/wml.h"

#include <QByteArray>
#include <QTextCharFormat>
#include <QTextFormat>
#include <QVector>

namespace jp::write {

namespace prop {
// The values of TrackedDeleted.
constexpr int HiddenByDeletion = 1, HiddenAlready = 2;
enum : int {
    // character formats
    Atom = QTextFormat::UserProperty + 400,   // QByteArray: a piece of the file kept as XML (packAtom); the text is U+FFFC
    RawRPr,                                   // QByteArray: run properties kept as written (packRaw)
    ColorAttrs,                               // QString: "themeColor;themeTint;themeShade" of the color while it is ColorAttrsRgb
    ColorAttrsRgb,                            // QString: the color those attributes gave, "RRGGBB"
    UnderlineAttrs,                           // QString: the attributes of w:u beside its val and color (themeColor...), as written
    DoubleStrike,                             // bool
    Hidden,                                   // (unused: hidden text is tp::HiddenText, which the text engine leaves out)
    Raised,                                   // int: half-points raised
    TrackedDeleted,                           // int: the text (or mark) is what a tracked change deleted, hidden (tp::HiddenText) for the lines: HiddenAlready if its
                                              // format hid it before (the file's w:vanish or its style's, which the run says as it is), HiddenByDeletion if only deleting does
    // block formats
    RawBefore = QTextFormat::UserProperty + 430,   // QByteArray: body elements kept as XML that come before the paragraph (packBlocks)
    RawAfter,                                 // QByteArray: the same after it
    RawBlock,                                 // QByteArray: the paragraph stands in for a body element kept as XML (packBlocks of one); it can't be edited
    ParaAttrs,                                // QByteArray: the attributes of w:p as written
    RawPPr,                                   // QByteArray: paragraph properties kept as written (packRaw)
    NumId,                                    // int: the list definition (w:numId) of a paragraph in a list
    NumLevel,                                 // int: its level, 0 to 8
    Outline,                                  // int: outline level 0 to 8, 9 for body text
    Contextual,                               // (unused: the paragraph's "no space between paragraphs of the same style" is tp::ContextualSpacing)
    SectionData,                              // QByteArray: the section this paragraph ends (packSection), with flow::fp::SectionBreakAfter
    LastSection,                              // a key of bytes (packSection), on the FIRST paragraph: the properties of the document's last section, which
                                              // belong to no paragraph; being a paragraph's property, a change to them undoes with the text
    MarkFormat,                               // a key of bytes (packFormat), on the FIRST paragraph only: its mark's character format, as the engine's own record of it (the
                                              // block's character format) can't be undone for the first block; readers take it from here (markFormat in editops.h)
    SpacingAttrs,                             // QByteArray: attributes of w:spacing that have no field, as written
    IndAttrs,                                 // QByteArray: the same for w:ind
    FontsOther,                               // QString: the fonts of the scripts the engine has none for, as a run says them (packOtherFonts)
    NumDirect,                                // bool: the paragraph says it is numbered as its style says (and so the list's indents are over the style's)
    JcRaw,                                    // QString: the alignment as the file wrote it, when it is a kind of justified ("distribute")
    OddTabs,                                  // QByteArray: tab stops of the paragraph's own that the engine has no kind for (a bar), packed (packTabs)
    EndnoteBlock,                             // bool: the paragraph is part of the document's endnotes, which the pages set after its last text (made from the endnotes part, which is kept as it is: it is not
                                              // written, and can't be edited)
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

// A character format as bytes, and back.
QByteArray packFormat(const QTextCharFormat &format);
QTextCharFormat unpackFormat(const QByteArray &packed);

// A piece kept as XML, with what the writer needs to keep it whole.
QByteArray packAtom(const office::wml::Inline &atom);
office::wml::Inline unpackAtom(const QByteArray &packed);
// The place from the page's top, in points, that an atom asks for if it is the instruction of an ADVANCE field with the \\y switch (the text after it is set so far down the
// page); none for any other atom.
std::optional<double> advanceTopOf(const office::wml::Inline &atom);

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
