#pragma once
// Spelling: the bundled dictionaries (text/dictionaries), with the user's own
// words and the words ignored for the session, and the misspelled ranges of
// a story, for every app's checking as you type and its Spelling dialog.

#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

class QRegularExpression;
class QTextBlock;
class QTextDocument;

namespace jp {

// The dictionaries themselves are shared with hyphenation (text/dictionaries);
// this adds the user's own words and the words ignored for the session.
class Speller {
public:
    // The options that let a word go unchecked (Options > Proofing), read once for a run of words.
    struct Rules {
        bool ignoreUpper = true, ignoreNumbers = true;
    };
    static Speller &get();
    static Rules rules();
    bool check(const QString &code, const QString &w);
    bool check(const QString &code, const QString &w, const Rules &rules);
    QStringList suggest(const QString &code, const QString &w);
    void ignore(const QString &w);
    void add(const QString &w);
    // Counts the changes to the words that are accepted (Ignore All, Add to Dictionary): an answer kept from before a change is stale.
    int revision() const { return m_revision; }

private:
    Speller();
    static QString userDict();
    QSet<QString> m_custom, m_ignore;   // the user's words (every language), and this session's
    int m_revision = 0;
};

// What a word is, for checking.
const QRegularExpression &spellingWordPattern();
// Misspelled ranges in a story, skipping fields and "no proofing" text.
QVector<QPair<int, int>> misspellings(QTextDocument *doc); // [start, end) document ranges
// The same for one paragraph. Text that states no language is `fallback` (BCP-47; empty is US English).
QVector<QPair<int, int>> misspellingsIn(const QTextBlock &block, const QString &fallback = QString());
// Changes with Ignore All and Add to Dictionary: an answer kept from before is stale.
int spellingRevision();
QStringList spellingSuggestions(const QString &word, const QString &language = QString());   // language: BCP-47, "" US English
void spellingAdd(const QString &word);                    // add to the user dictionary
void spellingIgnore(const QString &word);                 // ignore for this session

} // namespace jp
