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

// The namespaces a part of the package declares on its root element.
extern const char kNamespaces[];

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
    enum class Kind { Main, Footnote, Endnote, HeaderFooter, TextBox, Comment };
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
    // The data of a legacy form field kept at `position` of the data stream (section 2.9.158, 2.9.78, 2.9.79) as the w:ffData element, empty if there is none there.
    QByteArray formFieldXml(qint32 position);
    int runBuilders = 0;                 // (the number of paragraphs' runs begun, for telling them apart)
    // The VML shapes anchored at character `cp` of a story (a drawing the text holds a place for), empty if there are none.
    QByteArray shapeXml(qint64 cp, Kind kind, const QString &part);
    // A picture at `position` in the data stream as the XML of an inline drawing, empty if it can't be shown.
    // (`quiet`: a picture that can't be shown is not said to have been left out: the caller has something else to show.)
    QByteArray pictureXml(qint32 position, const QString &part, bool quiet = false);
    // What stands for an embedded object that has no picture of its own: a plain picture the size the object's header at `position` of the data stream says (when it holds one; else
    // an inch square). The objects themselves are not carried; the number met is `embeddedObjects`.
    QByteArray objectPlaceholderXml(qint32 position, const QString &part);
    int embeddedObjects = 0;
    // Remember that something of the file was left out (once for each `key`).
    void left(const QString &key, const QString &text);
    // Whether `key` has been said already (to skip making the words of a note that would not be added).
    bool wasLeft(const QString &key) const { return m_leftKeys.contains(key); }
    // The bytes of the table stream that a slot of the header names.
    Span tableRange(int slot) const;
    QString revisionAuthor(int index) const { return index >= 0 && index < authors.size() && !authors[index].isEmpty() ? authors[index] : QStringLiteral("Author"); }

    wml::ReadNotes *notes = nullptr;
    QByteArray fileWd, fileTable, fileData;
    QByteArray summaryStream, documentSummaryStream;   // the streams of the document's properties (section 2.25.1 of the property set format, and its document summary)
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
    // The comments of the main text, in the order of the characters that refer to them (section 2.8.7, 2.8.8): where the reference is, who made it, and where its text is
    // in the comment story.
    struct CommentInfo {
        qint64 reference = 0;
        QString author, initials;
        quint32 date = 0;
        qint64 textFrom = 0, textTo = 0;
    };
    QVector<CommentInfo> comments;
    // The number of the comment whose reference is at character `cp` (-1 if none).
    int commentAt(qint64 cp) const;
    int commentsDeclared = 0;            // the entries of the file's table of comments that were looked at
    bool inComments = false;             // the comments' own text is being made (it has a budget of its own, so the end of it being cut does not say the document was)
    // Once the main text `body` is made: writes the comments whose references are in it (each with the text it has in the file, within a budget of its own), takes out of `body` the marks
    // of every comment that is not written (and the starts of ranges that have no end), and says what was left out. The comments part's XML; empty if there is none.
    QByteArray finishComments(QByteArray *body);
    PackageWriter pkg;
    int paragraphs = 0;                  // made so far, in all the stories
    int tableCells = 0;                  // the cells of all the tables so far
    qint64 outputBytes = 0, outputElements = 0;   // the XML made so far (see spend)
    qint64 charsConverted = 0, picturesRead = 0;   // counted for tests (see ReadWork)
    qint64 shapeVisits() const;
    bool truncated = false;              // the text was cut at a limit
    // Counts XML just made against what the output may hold (doc.h). False once past it: the text stops where it is.
    bool spend(const QByteArray &xml);
    // Stops the text where it is: the rest is left out, and a note says so.
    void cutShort();
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
    void readComments();
    void addDocumentProperties();
    QByteArray commentsXml(const QVector<int> &ids, QVector<int> *written);
    QByteArray fontTableXml() const;
    Sep withNoteFormats(Sep sep) const;
    QByteArray settingsXml() const;
    QByteArray documentXml(const QByteArray &body, const QByteArray &lastSection) const;

    QSet<QString> m_leftKeys;
    std::shared_ptr<Drawings> m_drawings;
    int m_pictures = 0;
    qint64 m_pictureBytes = 0;
    QHash<QString, QString> m_mediaOfKey;           // where a picture came from -> the media part holding it
    QByteArray m_placeholderPng;                     // the picture that stands for an embedded object that has none (made once)
    QHash<qint32, QByteArray> m_formFieldAt;        // the form fields' data read so far, by place in the data stream
    QHash<qint32, Picture> m_pictureAt;             // the pictures of the data stream read so far (a place with none: empty bytes)
    qint64 m_pictureWork = 4000000;              // what is left of the work the pictures of the file may do (see odraw.h)
    QHash<QString, QString> m_relOfMedia;         // "part|media" -> relationship id
    int m_mediaCount = 0;
    int m_drawingId = 0;
};

// The strings of a string table (an STTB, as the file writes its bookmark names and author names), at most `limit` of them.
QStringList readStringTable(const Span &s, int limit);

} // namespace jp::office::doc
