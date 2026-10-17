#pragma once
// The conversion of an old binary document into the parts of a .docx package: the state that reading it shares across the files of this
// folder (the file reader, the story writer, and the pictures). Internal to libs/office/doc.

#include "doc/doclists.h"
#include "doc/docstyles.h"
#include "office/package.h"

#include <QHash>
#include <QSet>
#include <memory>

namespace jp::office::doc {

struct Picture {
    QByteArray bytes;
    QString extension;                    // png, jpeg, emf, wmf, bmp, or tiff
    double width = 0, height = 0;         // as shown, in points
    double cropL = 0, cropT = 0, cropR = 0, cropB = 0;   // the parts cut off, as fractions of the whole
    QByteArray shape;                     // a shape with no picture of its own (a rule, a box): its drawing record, and the size above
};

class Converter {
public:
    // What kind of text a story is.
    enum class Kind { Main, Footnote, Endnote, HeaderFooter, TextBox };
    struct Section {
        qint64 end = 0;                   // the character position after the section's last character
        int index = 0;                    // its place in the file's table of sections (where its headers and footers are found)
        Sep sep;
        QByteArray xml;                   // its <w:sectPr>, once the header and footer references are known
    };

    explicit Converter(wml::ReadNotes *notes) : notes(notes) {}
    // Reads the compound file, the header, and the tables of text and properties. False, with the reason, if it isn't a document this reads.
    bool open(const QByteArray &file, QString *error);
    // The package.
    QByteArray build();

    // The text of a story as the XML of a body: paragraphs and tables. `part` is the part it goes in (for its relationships).
    QByteArray story(qint64 cp0, qint64 cp1, Kind kind, const QString &part);
    // The VML shapes anchored at character `cp` of a story (a drawing the text holds a place for), empty if there are none.
    QByteArray shapeXml(qint64 cp, Kind kind, const QString &part);
    // A picture at `position` in the data stream as the XML of an inline drawing, empty if it can't be shown.
    QByteArray pictureXml(qint32 position, const QString &part);
    // Remember that something of the file was left out (once for each `key`).
    void left(const QString &key, const QString &text);
    // The bytes of the table stream that a slot of the header names.
    Span tableRange(int slot) const;
    QString revisionAuthor(int index) const { return index >= 0 && index < authors.size() && !authors[index].isEmpty() ? authors[index] : QStringLiteral("Author"); }

    wml::ReadNotes *notes = nullptr;
    QByteArray fileWd, fileTable, fileData;
    Span wd, tbl, data;
    Fib fib;
    PieceTable pieces;
    BinTable chpBin, papBin;
    StyleSheet styles;
    FontNames fonts;
    Lists lists;
    QVector<Section> sections;
    QVector<qint64> sectionEnds;                    // where each section of the file's table ends (those that are breaks, and those that aren't)
    QVector<qint64> footnoteRefs, endnoteRefs;      // where the references to the notes are in the text
    QVector<qint64> footnoteText, endnoteText;      // where each note's text begins, and the end of the last
    QStringList authors;
    struct BookmarkEvent {
        qint64 cp;
        QByteArray xml;
    };
    QVector<BookmarkEvent> bookmarkEvents;           // where the bookmarks of the main text begin and end, in order
    PackageWriter pkg;
    int paragraphs = 0;                  // made so far, in all the stories
    int tableCells = 0;                  // the cells of all the tables so far
    qint64 outputBytes = 0;
    bool truncated = false;              // the text was cut at a limit
    int revisionId = 0;

    struct Drawings;

private:
    void readDrawings();
    Picture *storedPicture(int index);
    bool readPicture(qint32 position, Picture *out);
    QByteArray drawingXml(const Picture &pic, const QString &rid);
    QByteArray inlineShapeXml(const Picture &pic, const QString &part);
    QString addMedia(const QString &part, const Picture &pic, const QString &key);
    void readFontTable();
    void readSections();
    void readNotePlcs();
    void readBookmarks();
    QByteArray fontTableXml() const;
    Sep withNoteFormats(Sep sep) const;
    QByteArray settingsXml() const;
    QByteArray documentXml(const QByteArray &body, const QByteArray &lastSection) const;

    QSet<QString> m_leftKeys;
    std::shared_ptr<Drawings> m_drawings;
    int m_pictures = 0;
    qint64 m_pictureBytes = 0;
    QHash<QString, QString> m_mediaOfKey;           // where a picture came from -> the media part holding it
    QHash<qint32, Picture> m_pictureAt;             // the pictures of the data stream read so far (a place with none: empty bytes)
    qint64 m_pictureWork = 4000000;              // what is left of the work the pictures of the file may do (see odraw.h)
    QHash<QString, QString> m_relOfMedia;         // "part|media" -> relationship id
    int m_mediaCount = 0;
    int m_drawingId = 0;
};

// The strings of a string table (an STTB, as the file writes its bookmark names and author names), at most `limit` of them.
QStringList readStringTable(const Span &s, int limit);

} // namespace jp::office::doc
