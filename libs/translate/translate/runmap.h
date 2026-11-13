#pragma once
// From text in a document to what the translator is given, and back. The text is read paragraph by paragraph as runs (a run is a stretch of text with one look); what a
// paragraph says is cut at what a translator must not touch (a tab, a line break, an inline object such as a picture) into pieces, and a piece is written as HTML in
// which bold, italic, and underline are tags: the translator keeps a tag beside the words it enclosed. The translated HTML is read back into runs, each given the look
// of the run of the original that had the same bold, italic, and underline (the first one that did: a color or a size is kept where the look is the same as before),
// and the text is put in the place of the text it translates. What is not a piece (the tab, the break, the picture) is not touched.

#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QVector>

class QTextDocument;

namespace jp {
namespace translate {

struct Run {
    QString text;
    QTextCharFormat format;
    bool fixed = false;   // a tab, a line break, or an inline object: it is kept as it is
    int start = -1;       // its place in the document when it was read (-1 for a run made by a translation)
};

struct Paragraph {
    QVector<Run> runs;
    QTextBlockFormat blockFormat;
    int start = 0, end = 0;   // the stretch of the document the runs were read from
};

// What is to be translated: the paragraphs of a selection (the part of the first and the last that is selected), or the paragraph the cursor is in.
struct Source {
    QPointer<QTextDocument> document;
    QVector<Paragraph> paragraphs;
    int first = 0, last = 0;
    QString snapshot;   // the text read (QTextCursor::selectedText), to see whether it has changed since
    bool isEmpty() const;
    // The text of every paragraph, one after the other with a line feed between.
    QString plainText() const;
    // Whether the document still holds the text that was read.
    bool isCurrent() const;
};

// Reads the source from `c` (its selection, or its paragraph when nothing is selected).
Source readSource(const QTextCursor &c);

// A piece of a paragraph: runs `first` to `last` of it, none of them fixed, and at least one letter or digit in them.
struct Piece {
    int paragraph = 0, first = 0, last = 0;
};
QVector<Piece> pieces(const Source &source);
// The HTML of a piece (what it says, with <b>, <i>, and <u>); the text is written with &amp;, &lt;, and &gt;, and the white space at its ends is left out (it is kept by `rebuild`).
QString pieceHtml(const Source &source, const Piece &piece);

// The translation of every piece, in the order `pieces` gave them (an empty one is left as it was): the paragraphs with their text translated and the rest as it was.
struct Replacement {
    int start = 0, end = 0;   // the stretch of the document the piece was read from
    QVector<Run> runs;        // what takes its place
};
struct Result {
    QVector<Paragraph> paragraphs;     // the paragraphs as they are with the translation in them
    QVector<Replacement> replacements; // the pieces, one for one (those not translated are left out)
    QString language;                  // the code of the language the text is in now (the pane sets it)
    QString plainText() const;
    // The result as HTML for showing it (paragraphs of <p>, with the bold, italic, and underline of the runs).
    QString html() const;
};
Result rebuild(const Source &source, const QVector<Piece> &pieces, const QStringList &translatedHtml);

// The runs of a translated piece, from its HTML, looks taken from `original` (the runs of the piece's source).
QVector<Run> runsFromHtml(const QString &html, const QVector<Run> &original);

// How the text is put in the document. The default does it with the cursor; a program with edits of its own (tracked changes, fields) says how in a subclass.
class EditHooks {
public:
    virtual ~EditHooks();
    // The look of a run of text in `language` (a code of languages.h): a program marks it as that language for its proofing. The same look by default.
    virtual QTextCharFormat languageFormat(const QTextCharFormat &format, const QString &language) { Q_UNUSED(language) return format; }
    // The cursor has a stretch of a piece selected: it is replaced by `runs`.
    virtual void replaceSelection(QTextCursor &c, const QVector<Run> &runs);
    // The cursor is at the end of a paragraph: a paragraph with the format `like` follows it, with `runs`, and the cursor is left at its end.
    virtual void insertParagraphAfter(QTextCursor &c, const QTextBlockFormat &like, const QVector<Run> &runs);
};

// Replaces what was read by the result (a piece for a piece); false, with nothing changed, when the document no longer holds what was read.
bool applyReplace(const Source &source, const Result &result, EditHooks &hooks);
// Puts the result below the paragraphs that were read, a paragraph for a paragraph, in the document order; the same checks.
bool applyInsertBelow(const Source &source, const Result &result, EditHooks &hooks);

} // namespace translate
} // namespace jp
