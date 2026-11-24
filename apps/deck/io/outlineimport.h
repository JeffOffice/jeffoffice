#pragma once
// Slides from an outline: a text outline, a Markdown file, or a rich text or .docx file whose headings make the slides, read into a plain list of
// slides (titles, lines of text with their levels, code, and pictures) that is then put in a presentation as slides on the Title and Content layout.
//
// The rules, as the Help page says them:
// - Plain text: a line that starts at the left edge and is not a list item is a slide's title. The lines under it are its text, each a level deeper
//   than the one before when it is indented further (tabs or spaces; a list marker -, *, +, or a number is dropped).
// - Markdown: the shallowest heading level the file uses (# when there is one, else ##) is the slide's title; deeper headings are lines of its text, a
//   level in for each level deeper. A line of --- starts a new slide. Lists, **bold**, *italic*, `code`, and links keep their look as the text's own
//   formatting. A fenced block of code is a monospaced text box. A picture on a line of its own is put on the slide when its file is in the outline's
//   own folder (or a folder below it). A file with no headings is read as plain text with that formatting.
// - Rich text and .docx: the shallowest heading style the file uses is the slide's title, deeper heading styles are lines of its text, and the other
//   paragraphs are lines under the heading above them (a list paragraph at the level the list has it).
// Everything a file can multiply is capped (the constants below), and what a cap, or a kind of content that has no place on a slide, leaves out is said.

#include "core/document.h"

#include <QByteArray>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QVector>
#include <atomic>
#include <memory>

namespace jp::deck {

constexpr int kMaxOutlineSlides = 5000;              // slides read from one file
constexpr qint64 kMaxOutlineTextBytes = qint64(10) << 20;   // a text or Markdown file
constexpr int kMaxOutlineIndents = 200;              // levels of indenting told apart (past it a line keeps the deepest)
constexpr int kMaxOutlineLevel = 8;                  // the deepest level of a slide's text (the Outline view's), the levels past it flatten to it
constexpr int kMaxOutlineLineChars = 4000;           // one line of text
constexpr int kMaxOutlineLinesPerSlide = 500;        // lines of text on one slide
constexpr int kMaxOutlineLines = 100000;             // lines of text on all the slides
constexpr int kMaxOutlineCodeLines = 200;            // lines of code on one slide
constexpr int kMaxOutlinePicturesPerSlide = 4;
constexpr int kMaxOutlinePictures = 100;             // pictures of one outline
constexpr qint64 kMaxOutlinePictureBytes = qint64(32) << 20;   // one picture file
constexpr qint64 kMaxOutlinePictureTotal = qint64(64) << 20;   // all the pictures' files together
constexpr qint64 kMaxOutlinePicturePixels = 16000000;          // one picture's pixels (the largest picture in the reference presentations has 9 million)
constexpr qint64 kMaxOutlinePictureTotalPixels = 64000000;     // all the pictures' pixels together (what drawing them holds in memory at four bytes each)

// A stretch of a line of text with one look.
struct OutlineRun {
    QString text;
    bool bold = false, italic = false, mono = false;
    QString href;   // a web or mail address
};
struct OutlineLine {
    int level = 0;   // 0 to kMaxOutlineLevel
    QVector<OutlineRun> runs;
};
struct OutlinePicture {
    QByteArray bytes;
    QString format, path, alt;
    QSize pixels;
};
struct OutlineSlide {
    QVector<OutlineRun> title;
    QVector<OutlineLine> body;
    QStringList code;   // lines of a fenced block of code
    QVector<OutlinePicture> pictures;
};
struct Outline {
    QVector<OutlineSlide> slides;
    QStringList notes;   // what was left out, in words for a person
    qint64 scanSteps = 0;   // the characters the Markdown reader looked at while matching marks (the tests count it: the work is linear in the line)
};

// The kinds of file Slides from Outline and File > Open take, by their endings: .txt, .text, .md, .markdown, .rtf, and .docx.
bool isOutlineFileName(const QString &path);

// Reads the bytes of such a file (`name` gives the ending; `folder` is where a Markdown picture's relative path starts, empty for none: no pictures).
// False, with the reason in `error`, for a file that is not text (nor rich text nor a .docx), or is past the limits, or has no slide in it.
// `cancel`, when given, is looked at between lines: once it is true the read stops and gives false.
bool readOutline(const QByteArray &bytes, const QString &name, const QString &folder, Outline *out, QString *error, const std::atomic<bool> *cancel = nullptr);
// The same for a file on disk (its folder is the folder of the pictures).
bool readOutlineFile(const QString &path, Outline *out, QString *error, const std::atomic<bool> *cancel = nullptr);

// The text as lines: LF, CRLF, and CR end a line; UTF-8 (with or without a byte order mark) and UTF-16 with one are read. False, with the reason, for
// anything else (and for bytes with a zero in them, which are not text).
bool decodeOutlineText(const QByteArray &bytes, QString *text, QString *error);

// Puts the outline's slides in the presentation as slides on the Title and Content layout (the one of the slide master of the slide before them),
// from index `at` on. Returns how many slides were made. The caller makes it one undo step.
int insertOutline(Document &d, int at, const Outline &outline);
// A new presentation made of only the outline's slides.
std::unique_ptr<Document> presentationFromOutline(const Outline &outline);

// The presentation's titles and lines of text as a Markdown outline: each slide a # line and its text as list lines indented by level, with the text's
// bold, italic, and links written as Markdown. `leftOut` gets what the outline does not hold, one line for each kind.
QString outlineMarkdown(const Document &d, QStringList *leftOut = nullptr);

} // namespace jp::deck
