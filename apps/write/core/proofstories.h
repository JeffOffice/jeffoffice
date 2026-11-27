#pragma once
// Every story of a document the grammar check reaches (see jeffwrite.md): the text (and in it the table cells and the endnotes, which are paragraphs of the story), the headers and footers, the
// footnotes, and the text boxes (of the text, of the notes, and of the boxes themselves). One function finds them and checks them with the check the window uses as you type (GrammarCheck), so
// that the program, its tests, and the measuring tool (writetool grammar) all see the same issues in the same places.

#include "app/grammarcheck.h"

#include <QMap>
#include <QString>
#include <QVector>
#include <memory>

class QTextDocument;

namespace jp::write {

class WriteDoc;

enum class StoryKind { Body, TableCell, Endnote, Header, Footer, Footnote, TextBox };
constexpr int kStoryKinds = int(StoryKind::TextBox) + 1;
QString storyKindName(StoryKind kind);   // ("body", "table", "endnote", "header", "footer", "footnote", "textbox": for the measuring tool, never shown)

// The most text boxes looked at in one document (the rest are left unchecked and `truncated` says so), and how deep boxes in boxes are read.
constexpr int kMostCheckedBoxes = 2000;
constexpr int kMostCheckedBoxDepth = 8;

struct ProofIssue {
    StoryKind kind = StoryKind::Body;
    GrammarCheck::Issue issue;   // as [start, end) in the story it is in
    QString text;                // the words the issue is on
    QString context;             // about 50 characters each side of them, with the words in [[ ]]
};

struct ProofReport {
    QVector<ProofIssue> issues;
    QMap<StoryKind, qint64> words, paragraphs;   // what was looked at, by kind of story (a paragraph of a language that is not checked is not counted)
    bool truncated = false;                      // a limit was reached: the boxes, or the budget of the checker
};

// Checks every story of the document (a header or footer the file names is read first; the notes the text cites are made editable, as the word count does) for the kinds of issue in `kinds`.
// On the calling thread: for tests and tools. Nothing of the document is changed.
ProofReport checkEveryStory(WriteDoc &doc, quint32 kinds = grammar::kDefaultKinds);

} // namespace jp::write
