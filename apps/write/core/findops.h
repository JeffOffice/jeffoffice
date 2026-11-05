#pragma once
// Advanced find: the wildcard language of the Find pane ("Use wildcards") as a regular expression, and the replacement that may name the groups. Nothing here touches a document.
//
// In a pattern: ? is any one character, * is any run of characters (the shortest that fits), < and > are the start and the end of a word, [abc] and [a-z] are one of the characters, [!abc] is any
// but those, {n}, {n,}, and {n,m} are repeats of what comes before, @ is one or more of what comes before, ( and ) make a group, and \ takes the character after it as it is. The special codes
// are ^p (a paragraph mark), ^t (a tab), ^# (a digit), ^$ (a letter), ^w (white space), ^b (a section break), ^d (a field), ^f (a footnote mark), ^e (an endnote mark), and ^^ (a ^). Every other character is itself.
//
// In a replacement: \1 to \9 are the groups, ^& is all of what was found, ^t is a tab, ^p is a paragraph mark's place (a line break is not made: it is left as it is), and \\ is a backslash.

#include <QPair>
#include <QRegularExpression>
#include <QString>
#include <QTextCharFormat>

namespace jp::write {

constexpr int kMostPatternLength = 500;   // a longer pattern is refused
// What the search's text has in the place of a section break (a paragraph mark that ends a section), the start of a field, and a footnote's or an endnote's mark: one character each, from the
// private use area, so that the codes ^b, ^d, ^f, and ^e find them and nothing else does.
constexpr char16_t kFindSectionBreak = 0xE0B0, kFindField = 0xE0D0, kFindEndnoteMark = 0xE0E0, kFindFootnoteMark = 0xE0F0;
inline bool isFindMark(QChar c) { return c == QChar(kFindSectionBreak) || c == QChar(kFindField) || c == QChar(kFindEndnoteMark) || c == QChar(kFindFootnoteMark); }

struct FindPattern {
    QRegularExpression regex;
    QString error;   // empty when the pattern is good; else a sentence saying what is wrong with it
};

FindPattern wildcardPattern(const QString &pattern, bool matchCase);

// The replacement with its groups and codes worked out for one match.
QString expandReplacement(const QString &with, const QRegularExpressionMatch &match);

// The choices of Advanced Find that read the words in other ways than the letters typed: words that sound like the one typed (the American Soundex code), every form of the word (runs, running, ran),
// and letters found with punctuation or spaces between them that the box leaves out. The search is of one place at a time: the characters [first, second) of the text, or {-1, -1}.
struct AdvancedFind {
    QString query;
    bool matchCase = false, wholeWords = false;
    bool soundsLike = false, wordForms = false, ignorePunctuation = false, ignoreSpaces = false;
    bool active() const { return soundsLike || wordForms || ignorePunctuation || ignoreSpaces; }
};
QPair<int, int> advancedMatch(const QString &text, int from, const AdvancedFind &options);

// The four-character Soundex code of a word (letters only; empty for a word with none), and the stem that the forms of a word have in common (lower case; the irregular verbs and plurals of English are known).
QString soundexOf(const QString &word);
QString wordStem(const QString &word);

// Find by format: what a run of text must be. A choice left at -1 (or empty) is not asked of it.
struct FormatFilter {
    int bold = -1, italic = -1, underline = -1, strike = -1, highlighted = -1;
    QString font;    // the font family (compared without regard to case)
    QString style;   // the paragraph style's id
    bool any() const { return bold >= 0 || italic >= 0 || underline >= 0 || strike >= 0 || highlighted >= 0 || !font.isEmpty() || !style.isEmpty(); }
    // The character choices of the filter, and the paragraph's style (given by the caller, which knows the paragraph).
    bool matchesChar(const QTextCharFormat &format) const;
    bool matchesStyle(const QString &paragraphStyle) const { return style.isEmpty() || style == paragraphStyle; }
};

} // namespace jp::write
