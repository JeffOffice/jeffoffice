#pragma once
// Grammar and style rules for English, French, German, and Spanish, written for JeffOffice: a paragraph of text goes in, and the issues in it come out, each with a message and the text to put in
// place of the words it is about. The rules are clear-cut ones only: where a text may be right, it is left alone (a missed mistake costs less than a false alarm).
//
// Each rule is one small function in grammarrules.cpp and one line of its table; the work of a check is linear in the length of the text (the tokens are made once, and
// every rule goes through them once), and `work` counts the steps, so that a test can prove it.

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp {

struct GrammarIssue {
    enum Kind { Grammar, Style, Punctuation, Capitalization, Repetition, WordChoice, Other };
    int start = 0, end = 0;     // [start, end) in the text checked, UTF-16 code units
    Kind kind = Grammar;
    QString message;
    QStringList suggestions;    // each is the whole text to put in place of [start, end)
    QString rule;               // the name of the rule that found it (for the measuring tool; not part of what two issues are equal by)
    bool operator==(const GrammarIssue &o) const { return start == o.start && end == o.end && kind == o.kind && message == o.message && suggestions == o.suggestions; }
};

namespace grammar {

// The language a text is checked as (each has its own rules; the apps pick it from the language of the paragraph).
enum class Language { English, French, FrenchCanada, German, Spanish };   // (FrenchCanada: French with the spacing of Quebec, which has no space before ; ! ?)

// Which kinds of issue a check looks for: a bit for each GrammarIssue::Kind. Style is off unless asked for.
constexpr quint32 kindBit(GrammarIssue::Kind k) { return quint32(1) << int(k); }
constexpr quint32 kAllKinds = 0xFFFFFFFFu;
constexpr quint32 kDefaultKinds = kAllKinds & ~kindBit(GrammarIssue::Style);

// The rules, in the order they run: each has a name and the kind of issue it finds (the style rules are for the people who ask for them).
struct RuleInfo {
    QString name;
    GrammarIssue::Kind kind;
};
QVector<RuleInfo> rules(Language language = Language::English);

// The issues in a paragraph of text, in order of position (none overlap in what they replace), of the kinds in `kinds` (a rule whose kind is not wanted does not run).
// `work`, if given, gets the number of steps the check took.
QVector<GrammarIssue> check(const QString &text, quint64 *work = nullptr, quint32 kinds = kDefaultKinds, Language language = Language::English);

} // namespace grammar
} // namespace jp
