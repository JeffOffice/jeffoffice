#pragma once
// Matching a text in the text of cells, for Find and Replace: the text to look for may have wildcards (? is any one character, * is any run of characters, and ~ before ?, *, or ~ makes it
// the character itself), the match may ignore case, and it may be asked to cover the whole text of the cell. A match is the longest the pattern can make from the leftmost place it can
// begin: "a*c" in "abcxc" is all five letters. Everything is bounded: a pattern is at most kMaxPatternChars long, and the work done on one text is counted and stops at kMaxMatchSteps.

#include <QString>
#include <QStringView>
#include <QVector>

namespace jp::office::sheet {

constexpr int kMaxPatternChars = 255;
// The most characters of a cell's text that are looked at, and the most steps the matching of one text takes (a step is one comparison of a character).
constexpr qsizetype kMaxMatchChars = 32767;   // (the most a cell holds)
constexpr qint64 kMaxMatchSteps = 1000000;

struct TextPattern {
    QString text;
    bool matchCase = false;
    bool wholeCell = false;     // the match must be all of the text, not a part of it
    bool wildcards = true;      // ? * ~ mean what they say (else they are characters like the rest)
};

class TextMatcher {
public:
    explicit TextMatcher(const TextPattern &pattern);
    // The pattern can be used: it is not empty and not past kMaxPatternChars.
    bool valid() const { return m_valid; }
    // The first match of the text at or after `from`: where it begins and how many characters it takes. A text that is empty never matches. `steps`, if given, is added the comparisons made.
    bool find(QStringView text, qsizetype from, qsizetype *start, qsizetype *length, qint64 *steps = nullptr) const;
    bool matches(QStringView text, qint64 *steps = nullptr) const;
    // The text with each match (one after another, none overlapping) put right by `with` (a whole cell: the text itself is replaced); `count` is how many. The text itself if there is no match.
    // `with` is put as it is (no wildcards in it).
    QString replaced(const QString &text, const QString &with, int *count = nullptr) const;

private:
    enum class Kind : quint8 { Char, One, Many };
    struct Token {
        Kind kind;
        QChar ch;
    };
    // The fixed pieces between the runs of `*`: a piece is its tokens (characters and `?`) in `m_tokens` from `first`, `size` long.
    struct Piece {
        qsizetype first = 0, size = 0;
    };
    bool sameChar(QChar a, QChar b) const;
    bool pieceAt(const Piece &p, QStringView text, qsizetype at, qint64 *steps) const;
    qsizetype findPiece(const Piece &p, QStringView text, qsizetype from, qsizetype limit, qint64 *steps) const;
    qsizetype findPieceLast(const Piece &p, QStringView text, qsizetype from, qsizetype limit, qint64 *steps) const;

    TextPattern m_pattern;
    bool m_valid = false;
    QVector<Token> m_tokens;
    QVector<Piece> m_pieces;       // k+1 pieces for k runs of `*`
};

} // namespace jp::office::sheet
