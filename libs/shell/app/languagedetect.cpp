#include "app/languagedetect.h"

#include "app/settings.h"
#include "app/spelling.h"
#include "text/dictionaries.h"

#include <QRegularExpression>
#include <QStringList>
#include <algorithm>

namespace jp {

namespace {

QString baseOf(const QString &tag) { return tag.section(QLatin1Char('-'), 0, 0).section(QLatin1Char('_'), 0, 0).toLower(); }

} // namespace

QString detectLanguage(const QString &text, const QString &current)
{
    if (dict::root().isEmpty()) return QString();
    // The words that tell a language: letters only, three or more of them.
    QStringList words;
    auto it = spellingWordPattern().globalMatch(text);
    while (it.hasNext() && words.size() < kDetectMaxWords) {
        QString w = it.next().captured();
        while (w.endsWith(QLatin1Char('\'')) || w.endsWith(QChar(0x2019))) w.chop(1);
        if (w.size() >= 3) words << w;
    }
    if (words.size() < kDetectMinWords) return QString();
    // One dictionary for each language, the usual one of its regional forms.
    struct Candidate {
        QString base, code;
        double rate = 0;
    };
    QVector<Candidate> candidates;
    for (const dict::Language &l : dict::languages()) {
        const QString base = baseOf(l.code);
        const bool seen = std::any_of(candidates.cbegin(), candidates.cend(), [&](const Candidate &c) { return c.base == base; });
        const QString code = dict::match(base);
        // (A dictionary that accepts a string of letters no language has tells nothing: it is left out.)
        if (!seen && !code.isEmpty() && dict::hasSpelling(code) && !dict::spell(code, QStringLiteral("qzxkvjwq")) && !dict::spell(code, QStringLiteral("яяфжцъщ"))) candidates.push_back({base, code, 0});
    }
    const QString own = current.isEmpty() ? QStringLiteral("en") : baseOf(current);
    for (Candidate &c : candidates) {
        int accepted = 0;
        for (const QString &w : std::as_const(words))
            if (dict::spell(c.code, w)) ++accepted;
        c.rate = double(accepted) / double(words.size());
    }
    const Candidate *best = nullptr;
    double ownRate = 0;
    for (const Candidate &c : candidates) {
        if (c.base == own) ownRate = c.rate;
        if (!best || c.rate > best->rate) best = &c;
    }
    if (!best || best->base == own || best->rate < 0.8 || best->rate - ownRate < 0.3) return QString();
    // (The runner up among the other languages: not the language the text is in already.)
    for (const Candidate &c : candidates)
        if (c.base != best->base && c.base != own && best->rate - c.rate < 0.15) return QString();
    return best->code;
}

namespace {
bool s_offered = false;
}

void setLanguageDetectionOffered(bool offered) { s_offered = offered; }

bool languageDetectionOffered() { return s_offered; }

bool detectLanguageAsYouType() { return Settings::get().value(QStringLiteral("proof/detectLanguage"), false).toBool(); }

void setDetectLanguageAsYouType(bool on) { Settings::get().setValue(QStringLiteral("proof/detectLanguage"), on); }

} // namespace jp
