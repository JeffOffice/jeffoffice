#pragma once
// Hyphenation by language (Liang patterns from LibreOffice's hyph_*.dic
// files), shared by automatic hyphenation in layout and the Hyphenate command.

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp {

// Positions inside word where a hyphen may go (a break before word[i]), for
// text in `language` (a BCP-47 tag; empty is US English). None for a
// language without patterns.
QVector<int> hyphenationPoints(const QString &word, const QString &language = QString());

// Whether automatic hyphenation may break this word: only words the
// language's dictionary knows, as .pub layouts hyphenate from a dictionary
// (names and coined words stay whole). Without the dictionary, any word.
bool hyphenationKnows(const QString &word, const QString &language = QString());

// For measuring against a computer that lacks some languages' dictionaries (jpubtool's --hyphenate-only; the programs never call it): automatic hyphenation
// is then done only in these languages, given as primary subtags ("en") or full tags ("pt-BR"); an empty language is US English. An empty list ends the limit.
// At most 64 entries of 16 characters are kept.
void limitAutomaticHyphenationTo(const QStringList &languages);

// Whether automatic hyphenation is done in `language` (always, unless the limit above is set).
bool automaticHyphenationIn(const QString &language);

} // namespace jp
