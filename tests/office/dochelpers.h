// Tests of the old binary word processing format reader (libs/office/doc) and of opening such a file in JeffWrite: a tiny writer of the structures
// the tests need, so every file they read is built here in code (never a real-world file). What it makes is a real compound file with the
// header, the piece table, the bins of properties, the styles, the lists, the sections, the notes, and the pictures that a program of the
// period wrote, in the simplest form that reader accepts.
#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

namespace dochelp {

// One change to a property (a sprm) with a one, two, or four byte operand, or with a length first.
QByteArray sprm1(quint16 op, int value);
QByteArray sprm2(quint16 op, int value);
QByteArray sprm4(quint16 op, quint32 value);
QByteArray sprmVar(quint16 op, const QByteArray &operand);

// The sprms the tests use.
namespace sprm {
constexpr quint16 bold = 0x0835, italic = 0x0836, size = 0x4A43, fontAscii = 0x4A4F, fontOther = 0x4A51, color = 0x2A42, underline = 0x2A3E;
constexpr quint16 special = 0x0855, picture = 0x6A03, charStyle = 0x4A30;
constexpr quint16 deleted = 0x0800, inserted = 0x0801, insertedBy = 0x4804, deletedBy = 0x4863, insertedAt = 0x6805, deletedAt = 0x6864;   // (tracked changes)
constexpr quint16 align = 0x2461, left = 0x845E, firstLine = 0x8460, before = 0xA413, after = 0xA414, keepNext = 0x2406, listInstance = 0x460B, listLevel = 0x260A;
constexpr quint16 inTable = 0x2416, endsRow = 0x2417, tableDepth = 0x6649;
constexpr quint16 pageWidth = 0xB01F, pageHeight = 0xB020, marginLeft = 0xB021, marginRight = 0xB022, marginTop = 0x9023, marginBottom = 0x9024, titlePage = 0x300A;
constexpr quint16 columns = 0x500B, columnSpace = 0x900C, sectionType = 0x3009, landscape = 0x301D;
} // namespace sprm

// The short modifier of a piece (a Prm0): the sprm number in the old numbering, and its one-byte operand.
quint16 prm0(int isprm, int val);

// A date and time as the old format keeps them (a DTTM): minutes, hours, day, month (1 to 12), and years since 1900, packed in four bytes; for a sprm4.
quint32 dttm(int year, int month, int day, int hour, int minute);

// The operand of a table row's definition: the edges of its cells (cells + 1 of them), twips from the margin.
QByteArray tableRow(const QVector<int> &edges);

struct StyleSpec {
    QString name;
    int sgc = 1;                       // 1 paragraph, 2 character
    int base = 0xFFF, next = 0xFFF;
    int sti = 0xFFE;                   // the built-in number (0 Normal, 1 heading 1...); 0xFFE for a style of the person's own
    QByteArray paragraph, character;   // the sprms
};

struct CharRun {
    int from, to;                      // characters [from, to) of the main text
    QByteArray sprms;
};
struct ParaRun {
    int from, to;                      // the paragraph, its mark included
    int istd = 0;
    QByteArray sprms;
};
struct ListLevelSpec {
    int format = 0;                    // 0 decimal, 23 bullet
    QString text;                      // the number text: "\x00." for "1.", or a bullet
    int start = 1;
    QByteArray paragraph, character;
};
// A stretch of the main text whose stored pieces carry a short modifier (as a fast-saved file's do): the one sprm of the old numbering `isprm` with the
// one-byte operand `val`, by `prm0()`.
struct PieceMod {
    int from, to;                      // characters [from, to) of the main text
    quint16 prm;
};
struct BookmarkSpec {
    QString name;
    int from, to;                      // characters [from, to) of the main text
};
// A comment: the character of the main text that cites it (a \x05, which the writer marks as special), who made it, when, its text (paragraphs end in \r), and the
// characters [from, to) of the main text it covers (-1: none, as a comment on a point).
struct CommentSpec {
    int ref = 0;
    QString author, initials, text;
    quint32 date = 0;                  // a DTTM (0: the file keeps no date, as the files of before 2002)
    int from = -1, to = -1;
    int ownerIndex = -1;               // the author's place in the list of authors, where not -1 (a hostile file may name one past the list)
    int endIndex = -1;                 // the place of the range's end in the table of ends, where not -1 (a hostile file may name one that is not an end)
};
struct SectionSpec {
    int end;                           // the character after the section's last
    QByteArray sprms;
};

// A drawing shape that a paragraph anchors: a text box, a picture, or a box; its place on the page is given in twips from where it is placed
// (bx and by say from what: 0 the margin, 1 the page, 2 the text).
struct ShapeSpec {
    int spid = 1025;
    int type = 202;                    // 202 text box, 75 picture frame, 1 rectangle, 3 oval, 20 line
    int cp = 0;                        // where its anchor character (\x08) is in the main text, or in the headers' text
    bool header = false;
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    int bx = 2, by = 2, wr = 2;
    bool below = false;                // behind the text
    QString text;                      // a text box's text: paragraphs end in \r
    QByteArray png;                    // a picture's bytes
    qint64 fill = -1, line = -1;       // colors as 0xBBGGRR; -1 for none
    int group = -1;                    // the shape that is the group this one is a member of (by place in the list)
    int groupLeft = 0, groupTop = 0, groupRight = 1000, groupBottom = 1000;   // a group shape's coordinates for its members
    int memberLeft = 0, memberTop = 0, memberRight = 0, memberBottom = 0;     // a member's place in its group's coordinates
    QVector<int> moreAnchors;          // other characters of the main text that anchor this same shape (a hostile file does; a program's file doesn't)
};

// One property of a property set (section 2.20 of the property set format) for a test's stream: its number and type, and its value: a text in the code page (`raw`, the bytes as
// the page has them, for type 0x1E), a wide text (0x1F), a whole number (2, 3), or a time as 100-nanosecond ticks since 1601 (0x40).
struct PropSpec {
    quint32 id = 0;
    quint16 type = 0;
    QString text;
    QByteArray raw;
    qint64 number = 0;
};
// The two kinds of set, by the 16 bytes that name them.
QByteArray summaryFormat();
QByteArray documentSummaryFormat();
// A property set stream (section 2.21): one set of the given kind with its code page property and the properties, or two (the user-defined set second, with nothing in it) for the
// document summary.
QByteArray propertySetStream(const QByteArray &format, int codePage, const QVector<PropSpec> &props, bool twoSets = false);
// The ticks of the property set format's time value (section 2.8) for a date and time (UTC).
qint64 fileTime(int year, int month, int day, int hour, int minute);

// The data of a legacy form field (section 2.9.78, 2.9.79): `type` 0 text field, 1 check box, 2 drop-down; `state` is the check box's (0, 1, 25 none) or the drop-down's chosen entry.
struct FormFieldSpec {
    int type = 0, state = 0;
    QString name, text, format, help, status;   // (`text` is a text field's default)
    int longest = 0, wDef = 0, textType = 0, boxSize = 20;
    bool locked = false, autoSize = true, ownHelp = false, ownStatus = false;
    QStringList entries;
};
QByteArray formFieldData(const FormFieldSpec &f);

class DocWriter {
public:
    QString text;                      // the main text: paragraphs end in \r (a table's cells in \a)
    bool unicode = false;
    int splitAt = -1;                  // a character where the text is stored again, as the other coding (8-bit before it, 16-bit after)
    QVector<CharRun> chars;
    QVector<ParaRun> paras;
    QVector<StyleSpec> styles;         // by istd; none: a Normal style
    QStringList fonts = {QStringLiteral("Times New Roman")};
    QStringList revisionAuthors;       // the people who made the tracked changes (a change names one by its place here)
    QVector<SectionSpec> sections;     // none: one section
    QStringList footnotes;             // each note's text (its own mark first, \r last)
    QVector<int> footnoteRefs;         // where each is cited in the main text
    QVector<int> footnoteStarts;       // where the file says each note's text begins in the notes' text, other than where it does (a hostile file); none: where it does
    QStringList headerStories;         // six for the notes' separators then six for each section; a story's text without the mark that ends it
    QVector<BookmarkSpec> bookmarks;
    QVector<CommentSpec> comments;
    QVector<PieceMod> pieceMods;       // stretches (not overlapping) of the main text that are pieces of their own, with a short modifier
    QVector<ShapeSpec> shapes;
    QVector<QVector<ListLevelSpec>> lists;   // each list's levels; the instance of list i is number i + 1
    bool evenAndOddHeaders = false;
    int compatByte84 = -1;   // the first of the document settings' compatibility bytes (at 84), where not -1 (the settings then reach it)
    int defaultTab = 720;
    QByteArray data;                   // the data stream: pictures
    QByteArray summaryInformation, documentSummaryInformation;   // the streams of the document's properties (by propertySetStream), when not empty
    bool password = false;             // the header says the file is protected
    int tableReserve = 0;              // bytes left at the start of the table stream (where a protected file keeps its description of the protection)
    bool oldVersion = false;           // a header from before 1997
    int repeatText = 1;                // the main text's pieces are listed this many times over the same stored text, one after another (a hostile file;
                                       // use it with no notes, headers, or shapes): the text is that many times as long

    // A picture (PNG bytes) set in the data stream, shown at the size in twips; the place to give a run (sprm::picture).
    int addPicture(const QByteArray &png, int widthTwips, int heightTwips);
    // The data of a form field (from formFieldData) set in the data stream with the header that goes before it (section 2.9.158); the place to give a run (sprm::picture).
    int addFormFieldData(const QByteArray &ffData);
    // A picture that is a metafile of that many bytes (all zero), kept deflated as such pictures are: a few kilobytes of file that make that many once read.
    int addMetafile(int inflatedBytes, int widthTwips, int heightTwips);
    // A rule across the line, in the data stream as a shape with no picture; the place to give a run (sprm::picture).
    int addRule(int heightTwips);
    int footnoteFormat = 0;            // how the document numbers its footnotes (4 lower-case letters)
    // The file: a compound file with the three streams.
    QByteArray build() const;
    // The file protected with `password`, made here from the published rules (not by the program's own code): `form` 0 is RC4 with a 40-bit key, 1 is RC4 with the cryptographic
    // provider's key (128 bits, SHA-1), 2 is the obfuscation (XOR). The document stream from its byte 68, the table stream after the description, and the data stream are enciphered.
    QByteArray buildProtected(const QString &password, int form) const;
};

// A small PNG picture.
QByteArray tinyPng(int width = 8, int height = 6);

} // namespace dochelp
