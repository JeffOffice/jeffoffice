#include "grammar.h"

#include <QElapsedTimer>

namespace jp {

bool grammarAvailable() { return true; }

namespace {

// The language a tag is checked as: English, French, German, and Spanish, in their regional forms (the rules do not differ between them); an empty tag is US English.
grammar::Language languageOf(const QString &family)
{
    if (family == QLatin1String("fr")) return grammar::Language::French;
    if (family == QLatin1String("de")) return grammar::Language::German;
    if (family == QLatin1String("es")) return grammar::Language::Spanish;
    return grammar::Language::English;
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

QString grammarFamily(const QString &language)
{
    const QString l = language.trimmed().toLower();
    if (l.isEmpty()) return QStringLiteral("en");
    for (const char *family : {"en", "fr", "de", "es"}) {
        const QString f = QLatin1String(family);
        if (l == f || l.startsWith(f + QLatin1Char('-')) || l.startsWith(f + QLatin1Char('_'))) return f;
    }
    return QString();
}

bool grammarSupports(const QString &language) { return !grammarFamily(language).isEmpty(); }

QVector<GrammarIssue> grammarCheck(const QString &text, const QString &language, bool *truncated, GrammarBudget *budget, quint32 kinds)
{
    if (truncated) *truncated = false;
    QVector<GrammarIssue> out;
    const QString family = grammarFamily(language);
    if (family.isEmpty() || text.trimmed().isEmpty()) return out;
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
    out = grammar::check(len == text.size() ? text : text.left(len), nullptr, kinds, languageOf(family));
    if (budget) budget->spend(len, timer.elapsed());
    if (out.size() > kGrammarMaxIssues) {
        out.resize(kGrammarMaxIssues);
        if (truncated) *truncated = true;
    }
    return out;
}

} // namespace jp
