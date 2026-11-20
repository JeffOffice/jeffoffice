#pragma once
// Which language a paragraph of text is in, for "detect the language as you type": decided by the bundled spelling dictionaries (text/dictionaries), by how many of the words each one
// accepts. It is plain and the same for the same text: a language is named only when it is clearly the one, and never for a short text, so that it does not change its mind.

#include <QString>

namespace jp {

constexpr int kDetectMinWords = 12;   // words of three or more letters that a text needs before a language is named
constexpr int kDetectMaxWords = 120;  // the words looked at (the first ones)

// The dictionary language (a tag such as "fr-FR") that `text` is clearly in, when it is not the language of `current` (a BCP-47 tag; empty is American English); empty when the text is too short,
// when `current` is what it is in, or when no language stands out. A language stands out when its dictionary accepts at least 80% of the words, `current`'s accepts at least 30 points
// fewer, and every other language's at least 15 points fewer. Regional forms of one language are not told apart: English text is never taken from British to American English.
QString detectLanguage(const QString &text, const QString &current);

// The choice to detect the language of a paragraph as it is typed (off until it is turned on). An app that does it says so, and only then does Options > Proofing offer the choice.
bool detectLanguageAsYouType();
void setDetectLanguageAsYouType(bool on);
void setLanguageDetectionOffered(bool offered);
bool languageDetectionOffered();

} // namespace jp
