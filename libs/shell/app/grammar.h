#pragma once
// Grammar and style checking, next to the spelling service (spelling.h): the rules of libs/text/grammar, offline, English first. It finds the issues of a piece of text
// with a message and the text to put in place; spelling stays with the dictionaries, so none of the issues is a misspelled word.
//
// A hostile document must not hang the program, so the work is bounded before it begins: a call takes at most kGrammarMaxChars (more is left unchecked and said so),
// and a GrammarBudget holds the total a run of calls may spend.

#include "grammar/grammarrules.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp {

// The most UTF-16 code units one call looks at. Real paragraphs have 100 to 1,500 characters (a very long one 4,000), so this is 25 times the long one; the work of a
// check is linear in the length (a megabyte takes well under a second), so the limit is for memory and for the issues it can make more than for time.
constexpr int kGrammarMaxChars = 100000;
// The most issues one call reports (a text of nothing but "a a a a" would otherwise give one for every word).
constexpr int kGrammarMaxIssues = 500;
// What a run (a document, a pass of the Spelling and Grammar pane) may spend in all: characters sent to the checker, and milliseconds in it.
// A 300-page book has about 600,000 characters; the budget is 5 million, and checking them takes minutes, so the time is the tighter bound.
constexpr qint64 kGrammarMaxTotalChars = 5000000;
constexpr int kGrammarMaxTotalMs = 120000;

// What a run may spend. Every call spends what it looked at and how long it took; once the budget is gone calls return nothing and say so.
class GrammarBudget {
public:
    explicit GrammarBudget(qint64 maxChars = kGrammarMaxTotalChars, int maxMs = kGrammarMaxTotalMs) : m_maxChars(maxChars), m_maxMs(maxMs) {}
    bool exhausted() const { return m_chars >= m_maxChars || m_ms >= m_maxMs; }
    void spend(qint64 chars, qint64 ms) { m_chars += chars; m_ms += ms; }
    qint64 chars() const { return m_chars; }
    qint64 ms() const { return m_ms; }

private:
    qint64 m_maxChars, m_maxMs, m_chars = 0, m_ms = 0;
};

// Whether there is a checker (always: it is part of the program).
bool grammarAvailable();
// Whether a language (BCP-47) is one that is checked: English, in its regional forms; an empty tag is US English.
bool grammarSupports(const QString &language);
// The issues in `text`, in order. At most kGrammarMaxChars are looked at (cut after the last sentence end that fits, or at the limit); `truncated` says that some were
// not. A `budget` that is gone gives nothing, with `truncated` set. Empty for a language that is not checked. Thread safe (calls take turns).
QVector<GrammarIssue> grammarCheck(const QString &text, const QString &language = QString(), bool *truncated = nullptr, GrammarBudget *budget = nullptr);

} // namespace jp
