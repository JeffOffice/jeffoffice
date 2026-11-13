#include "grammar.h"

#include <QElapsedTimer>

namespace jp {

bool grammarAvailable() { return true; }

namespace {

// English, in its regional forms (the rules do not differ between them); an empty tag is US English.
bool isEnglish(const QString &language)
{
    const QString l = language.trimmed().toLower();
    return l.isEmpty() || l == QLatin1String("en") || l.startsWith(QLatin1String("en-")) || l.startsWith(QLatin1String("en_"));
}

// Where to cut a text that is too long: after the last sentence end within the limit, or at the limit (not between the halves of a surrogate pair).
int cutAt(const QString &text, int limit)
{
    for (int i = limit - 1; i > limit / 2; --i) {
        const QChar c = text.at(i);
        if ((c == QLatin1Char('.') || c == QLatin1Char('!') || c == QLatin1Char('?') || c == QChar::ParagraphSeparator || c == QLatin1Char('\n')) && text.at(i + 1).isSpace())
            return i + 1;
    }
    return text.at(limit - 1).isHighSurrogate() ? limit - 1 : limit;
}

} // namespace

bool grammarSupports(const QString &language) { return isEnglish(language); }

QVector<GrammarIssue> grammarCheck(const QString &text, const QString &language, bool *truncated, GrammarBudget *budget, quint32 kinds)
{
    if (truncated) *truncated = false;
    QVector<GrammarIssue> out;
    if (!isEnglish(language) || text.trimmed().isEmpty()) return out;
    if (budget && budget->exhausted()) {
        if (truncated) *truncated = true;
        return out;
    }
    int len = text.size();
    if (len > kGrammarMaxChars) {
        len = cutAt(text, kGrammarMaxChars);
        if (truncated) *truncated = true;
    }
    QElapsedTimer timer;
    timer.start();
    out = grammar::check(len == text.size() ? text : text.left(len), nullptr, kinds);
    if (budget) budget->spend(len, timer.elapsed());
    if (out.size() > kGrammarMaxIssues) {
        out.resize(kGrammarMaxIssues);
        if (truncated) *truncated = true;
    }
    return out;
}

} // namespace jp
