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
class QTextDocument;

namespace jp {

// The dictionaries themselves are shared with hyphenation (text/dictionaries);
// this adds the user's own words and the words ignored for the session.
class Speller {
public:
    static Speller &get();
    bool check(const QString &code, const QString &w);
    QStringList suggest(const QString &code, const QString &w);
    void ignore(const QString &w);
    void add(const QString &w);

private:
    Speller();
    static QString userDict();
    QSet<QString> m_custom, m_ignore;   // the user's words (every language), and this session's
};

// What a word is, for checking.
const QRegularExpression &spellingWordPattern();
// Misspelled ranges in a story, skipping fields and "no proofing" text.
QVector<QPair<int, int>> misspellings(QTextDocument *doc); // [start, end) document ranges
QStringList spellingSuggestions(const QString &word, const QString &language = QString());   // language: BCP-47, "" US English
void spellingAdd(const QString &word);                    // add to the user dictionary
void spellingIgnore(const QString &word);                 // ignore for this session

} // namespace jp
