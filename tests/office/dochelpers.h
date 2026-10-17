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
constexpr quint16 align = 0x2461, left = 0x845E, firstLine = 0x8460, before = 0xA413, after = 0xA414, keepNext = 0x2406, listInstance = 0x460B, listLevel = 0x260A;
constexpr quint16 inTable = 0x2416, endsRow = 0x2417, tableDepth = 0x6649;
constexpr quint16 pageWidth = 0xB01F, pageHeight = 0xB020, marginLeft = 0xB021, marginRight = 0xB022, marginTop = 0x9023, marginBottom = 0x9024, titlePage = 0x300A;
constexpr quint16 columns = 0x500B, columnSpace = 0x900C, sectionType = 0x3009, landscape = 0x301D;
} // namespace sprm

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
struct BookmarkSpec {
    QString name;
    int from, to;                      // characters [from, to) of the main text
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

class DocWriter {
public:
    QString text;                      // the main text: paragraphs end in \r (a table's cells in \a)
    bool unicode = false;
    int splitAt = -1;                  // a character where the text is stored again, as the other coding (8-bit before it, 16-bit after)
    QVector<CharRun> chars;
    QVector<ParaRun> paras;
    QVector<StyleSpec> styles;         // by istd; none: a Normal style
    QStringList fonts = {QStringLiteral("Times New Roman")};
    QVector<SectionSpec> sections;     // none: one section
    QStringList footnotes;             // each note's text (its own mark first, \r last)
    QVector<int> footnoteRefs;         // where each is cited in the main text
    QStringList headerStories;         // six for the notes' separators then six for each section; a story's text without the mark that ends it
    QVector<BookmarkSpec> bookmarks;
    QVector<ShapeSpec> shapes;
    QVector<QVector<ListLevelSpec>> lists;   // each list's levels; the instance of list i is number i + 1
    bool evenAndOddHeaders = false;
    int defaultTab = 720;
    QByteArray data;                   // the data stream: pictures
    bool password = false;             // the header says the file is protected
    bool oldVersion = false;           // a header from before 1997
    int repeatText = 1;                // the main text's pieces are listed this many times over the same stored text, one after another (a hostile file;
                                       // use it with no notes, headers, or shapes): the text is that many times as long

    // A picture (PNG bytes) set in the data stream, shown at the size in twips; the place to give a run (sprm::picture).
    int addPicture(const QByteArray &png, int widthTwips, int heightTwips);
    // A rule across the line, in the data stream as a shape with no picture; the place to give a run (sprm::picture).
    int addRule(int heightTwips);
    int footnoteFormat = 0;            // how the document numbers its footnotes (4 lower-case letters)
    // The file: a compound file with the three streams.
    QByteArray build() const;
};

// A small PNG picture.
QByteArray tinyPng(int width = 8, int height = 6);

} // namespace dochelp
