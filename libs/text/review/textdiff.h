#pragma once
// The difference between two texts, word by word (or letter by letter): the operations that turn the first into the second. A pure function of two strings, shared by the programs that compare.

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::textdiff {

struct DiffOptions {
    bool ignoreCase = false;        // a word that differs only in its capitals is the same
    bool ignoreWhiteSpace = false;  // runs of spaces and tabs are the same
    bool byCharacter = false;       // compare letter by letter, not word by word
};

// The words that are the same or differ between two paragraphs' texts, as the operations that turn `from` into `to`: a delete is a range of `from` (in its characters), an insert is text that
// goes after the range in `from` it is given at. Ranges and inserts are in order. Texts that are too long to compare word by word are one replacement.
struct TextEdit {
    int deleteFrom = 0, deleteTo = 0;   // a range of `from`; empty when nothing is deleted
    QString insertText;                 // put at deleteTo
};
QVector<TextEdit> diffText(const QString &from, const QString &to, const DiffOptions &options);

// The longest common run of two lists, as which of each are kept (a table of lengths, walked back).
void commonRun(const QStringList &a, const QStringList &b, QVector<int> *keepA, QVector<int> *keepB);

// The most words of a text that are told apart one by one; a text with more is one replacement.
constexpr int kMostTokens = 3000;
// The cells of the table of lengths that diffText fills for these two texts (the product of their word counts, as the most it works with), 0 when it would make one replacement of them or they are the same.
qint64 diffCells(const QString &from, const QString &to, const DiffOptions &options);

} // namespace jp::textdiff
