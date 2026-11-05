#pragma once
// Flash Fill: from a few cells the person filled in by hand, finds how each came from the cells beside it, and gives what the same steps make for the other rows. A step is a piece of
// the output: text that is the same in every example, or a part of a neighbor cell (the whole text, a word counted from the start or the end, its first letter, its first or last few
// letters), in the case it had or in upper, lower, or capitalized form. No GUI.

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::crunch {

// The limits that keep a hostile sheet from multiplying the work: examples used, neighbor cells per row, the longest text read, and the steps of the search.
constexpr int kFlashFillMaxExamples = 8;
constexpr int kFlashFillMaxSources = 12;
constexpr int kFlashFillMaxLength = 256;
constexpr int kFlashFillMaxPieces = 8;

struct FlashFillExample {
    QStringList sources;   // the text of the neighbor cells of the row, nearest first
    QString output;        // what the person typed
};

// The pattern the examples share, or empty (a null result) if they share none. `apply` gives the output for a row, or a null string if the row lacks what the pattern reads.
class FlashFillPattern {
public:
    bool isValid() const { return !m_pieces.isEmpty(); }
    QString apply(const QStringList &sources) const;
    struct Piece {
        enum Kind { Constant, Whole, WordFromStart, WordFromEnd, Initial, Left, Right } kind = Constant;
        enum Case { AsIs, Upper, Lower, Capitalized } letterCase = AsIs;
        QString text;   // the constant
        int source = 0;   // the neighbor cell
        int count = 0;    // the word's place (from 0), or the number of letters
    };
    QVector<Piece> m_pieces;
};

FlashFillPattern learnFlashFill(const QVector<FlashFillExample> &examples);

} // namespace jp::crunch
