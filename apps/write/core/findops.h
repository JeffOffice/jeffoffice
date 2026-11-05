#pragma once
// Advanced find: the wildcard language of the Find pane ("Use wildcards") as a regular expression, and the replacement that may name the groups. Nothing here touches a document.
//
// In a pattern: ? is any one character, * is any run of characters (the shortest that fits), < and > are the start and the end of a word, [abc] and [a-z] are one of the characters, [!abc] is any
// but those, {n}, {n,}, and {n,m} are repeats of what comes before, @ is one or more of what comes before, ( and ) make a group, and \ takes the character after it as it is. The special codes
// are ^p (a paragraph mark), ^t (a tab), ^# (a digit), ^$ (a letter), ^w (white space), and ^^ (a ^). Every other character is itself.
//
// In a replacement: \1 to \9 are the groups, ^& is all of what was found, ^t is a tab, ^p is a paragraph mark's place (a line break is not made: it is left as it is), and \\ is a backslash.

#include <QRegularExpression>
#include <QString>

namespace jp::write {

constexpr int kMostPatternLength = 500;   // a longer pattern is refused

struct FindPattern {
    QRegularExpression regex;
    QString error;   // empty when the pattern is good; else a sentence saying what is wrong with it
};

FindPattern wildcardPattern(const QString &pattern, bool matchCase);

// The replacement with its groups and codes worked out for one match.
QString expandReplacement(const QString &with, const QRegularExpressionMatch &match);

} // namespace jp::write
