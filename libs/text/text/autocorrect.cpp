#include "text/autocorrect.h"

#include <QHash>
#include <QRegularExpression>

namespace jp::autotext {

// AutoCorrect: common typing slips and symbols, as most word processors
// fix them by default.
QString baseLanguage(const QString &language)
{
    if (language.isEmpty()) return QStringLiteral("en");
    return language.left(language.indexOf(QRegularExpression(QStringLiteral("[-_]")))).toLower();
}

QList<QPair<QString, QString>> defaultReplacements(const QString &language)
{
    QList<QPair<QString, QString>> list;
    if (baseLanguage(language) == QLatin1String("en")) {
        const QStringList pairs = {
            "i", "I", "teh", "the", "adn", "and", "nad", "and", "taht", "that", "thier", "their", "recieve", "receive",
            "recieved", "received", "beleive", "believe", "belive", "believe", "wierd", "weird", "freind", "friend",
            "accomodate", "accommodate", "acheive", "achieve", "occured", "occurred", "occurence", "occurrence",
            "seperate", "separate", "definately", "definitely", "goverment", "government", "untill", "until",
            "wich", "which", "whcih", "which", "becuase", "because", "becasue", "because", "alot", "a lot",
            "dont", "don't", "doesnt", "doesn't", "didnt", "didn't", "cant", "can't", "wont", "won't",
            "isnt", "isn't", "wasnt", "wasn't", "arent", "aren't", "couldnt", "couldn't", "shouldnt", "shouldn't",
            "wouldnt", "wouldn't", "im", "I'm", "ive", "I've", "youre", "you're", "theyre", "they're",
            "tommorow", "tomorrow", "tomorow", "tomorrow", "calender", "calendar", "neccessary", "necessary",
            "neccesary", "necessary", "publically", "publicly", "existance", "existence", "independant", "independent",
            "embarass", "embarrass", "enviroment", "environment", "foriegn", "foreign", "greatful", "grateful",
            "harrass", "harass", "knowlege", "knowledge", "libary", "library", "maintainance", "maintenance",
            "millenium", "millennium", "mispell", "misspell", "noticable", "noticeable", "posession", "possession",
            "prefered", "preferred", "recomend", "recommend", "refered", "referred", "relevent", "relevant",
            "succesful", "successful", "sucessful", "successful", "supercede", "supersede", "truely", "truly",
            "wether", "whether", "writting", "writing", "yeild", "yield", "hte", "the", "tje", "the", "fo", "of"};
        for (int i = 0; i + 1 < pairs.size(); i += 2) list.append({pairs[i], pairs[i + 1]});
    }
    list.append({QStringLiteral("(c)"), QStringLiteral("©")});
    list.append({QStringLiteral("(r)"), QStringLiteral("®")});
    list.append({QStringLiteral("(tm)"), QStringLiteral("™")});
    list.append({QStringLiteral("..."), QStringLiteral("…")});
    list.append({QStringLiteral("->"), QStringLiteral("→")});
    list.append({QStringLiteral("<-"), QStringLiteral("←")});
    list.append({QStringLiteral("--"), QStringLiteral("—")});
    return list;
}

QStringList defaultExceptions(const QString &language)
{
    if (baseLanguage(language) != QLatin1String("en")) return {};
    return {"a.m.", "p.m.", "e.g.", "i.e.", "etc.", "vs.", "Mr.", "Mrs.", "Ms.", "Dr.", "Prof.", "Sr.", "Jr.", "St.", "approx.", "Inc.", "Ltd.", "Co.", "No.", "Fig.", "Gen.", "Gov.", "Rev.", "Sen."};
}

QStringList defaultInitialExceptions(const QString &)
{
    return {"IDs", "PCs", "TVs", "CDs", "DVDs", "PDFs", "URLs", "FAQs", "APIs", "CEOs", "NGOs", "SSDs", "GPUs", "CPUs", "UIs", "OKs", "VIPs", "MBs", "GBs", "KBs"};
}

Config defaultConfig(const QString &language)
{
    Config c;
    c.language = language;
    c.replacements = defaultReplacements(language);
    c.exceptions = defaultExceptions(language);
    c.initialExceptions = defaultInitialExceptions(language);
    return c;
}

std::optional<Fix> correctWord(const QString &text, int rel, bool smartQuotes)
{
    static const Config config = defaultConfig();
    return correctWord(text, rel, smartQuotes, config);
}

std::optional<Fix> correctWord(const QString &text, int rel, bool smartQuotes, const Config &config, bool inCell)
{
    int start = rel;
    auto wordChar = [](QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('\'') || c == QChar(0x2019) || c == QLatin1Char('(') || c == QLatin1Char(')'); };
    while (start > 0 && wordChar(text[start - 1])) --start;
    QString word = text.mid(start, rel - start);
    if (word.contains(QChar::ObjectReplacementCharacter)) return std::nullopt;
    const bool english = baseLanguage(config.language) == QLatin1String("en");
    auto wordLike = [&](const QString &key) {
        bool all = !key.isEmpty();
        for (const QChar ch : key) all = all && wordChar(ch);
        return all;
    };
    QString fixed;
    if (config.replace) {
        // An entry that is a symbol (an arrow, dots), at the end of the text and not inside a longer run of the same mark.
        for (const auto &entry : config.replacements) {
            const QString &key = entry.first;
            if (key.isEmpty() || wordLike(key) || !text.left(rel).endsWith(key)) continue;
            const int at = rel - int(key.size());
            if (at > 0 && text[at - 1] == key[0] && !key[0].isLetterOrNumber()) continue;
            if (key[0].isLetterOrNumber() && at > 0 && wordChar(text[at - 1])) continue;
            return Fix{at, rel, entry.second, false};
        }
        // An entry that is a word: the word just typed.
        if (!word.isEmpty()) {
            for (const auto &entry : config.replacements) {
                if (entry.first.compare(word, Qt::CaseInsensitive) != 0 || entry.second == word || !wordLike(entry.first)) continue;
                fixed = entry.second;
                // Keep the typed capitals: "Teh" becomes "The", "TEH" becomes "THE".
                if (word.size() > 1 && word == word.toUpper() && word != word.toLower()) fixed = fixed.toUpper();
                else if (word[0].isUpper()) fixed[0] = fixed[0].toUpper();
                break;
            }
        }
    }
    if (word.isEmpty()) return std::nullopt;
    if (!fixed.isEmpty()) {
    } else if (config.twoInitials && word.size() > 2 && word[0].isUpper() && word[1].isUpper() && word.mid(2) == word.mid(2).toLower() && word.mid(2) != word.mid(2).toUpper() &&
               !config.initialExceptions.contains(word)) {
        // Two initial capitals: "THursday" becomes "Thursday".
        fixed = word;
        fixed[1] = fixed[1].toLower();
    } else if (config.capsLock && word.size() > 3 && word[0].isLower() && word.mid(1) == word.mid(1).toUpper() && word.mid(1) != word.mid(1).toLower() && word[1].isLetter() &&
               !config.initialExceptions.contains(word)) {
        // The caps lock key was on: "cAPS" is "Caps".
        fixed = QString(word[0].toUpper()) + word.mid(1).toLower();
    } else if (config.days && english && word[0].isLower()) {
        static const QStringList days = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"};
        if (days.contains(word)) fixed = QString(word[0].toUpper()) + word.mid(1);
    }
    // A capital at the start of a sentence.
    QString base = fixed.isEmpty() ? word : fixed;
    if (!base.isEmpty() && base[0].isLower()) {
        int k = start - 1;
        while (k >= 0 && text[k].isSpace()) --k;
        // The paragraph's first word (of a cell: when its switch is on), or one after a sentence's end and a space.
        const bool paragraphStart = k < 0;
        const bool sentenceStart = paragraphStart ? (inCell ? config.cells : config.sentences)
                                                  : (config.sentences && k < start - 1 && (text[k] == '.' || text[k] == '!' || text[k] == '?'));
        // Not after an abbreviation such as "e.g." or "a.m.", or one on the list of exceptions ("Mr.").
        bool abbreviation = !paragraphStart && k >= 2 && text[k] == '.' && text[k - 2] == '.';
        if (!paragraphStart && text[k] == '.') {
            int from = k;
            while (from > 0 && !text[from - 1].isSpace()) --from;
            for (const QString &e : config.exceptions)
                if (e.compare(text.mid(from, k + 1 - from), Qt::CaseInsensitive) == 0) abbreviation = true;
        }
        if (sentenceStart && !abbreviation && base[0].isLetter()) {
            base[0] = base[0].toUpper();
            fixed = base;
        }
    }
    if (fixed.isEmpty() || fixed == word) return std::nullopt;
    if (smartQuotes) fixed.replace(QLatin1Char('\''), QChar(0x2019));
    return Fix{start, rel, fixed, false};
}

std::optional<Fix> formatWord(const QString &text)
{
    static const QRegularExpression emDash(QStringLiteral("[\\w.,!?)\"'”’]--[\\w(\"'“‘]\\S*$"));
    static const QRegularExpression enDash(QStringLiteral("\\w - [\\w(\"'“‘]\\S*$"));
    static const QRegularExpression fraction(QStringLiteral("(?:^|\\s)(1/2|1/4|3/4)$"));
    static const QRegularExpression ordinal(QStringLiteral("(?:^|\\s)\\d*(?:1st|2nd|3rd|[04-9]th|1[1-3]th)$"));
    if (const auto m = emDash.match(text); m.hasMatch()) return Fix{int(m.capturedStart()) + 1, int(m.capturedStart()) + 3, QString(QChar(0x2014)), false};
    if (const auto m = enDash.match(text); m.hasMatch()) return Fix{int(m.capturedStart()) + 2, int(m.capturedStart()) + 3, QString(QChar(0x2013)), false};
    if (const auto m = fraction.match(text); m.hasMatch()) {
        const QString f = m.captured(1);
        return Fix{int(m.capturedStart(1)), int(m.capturedEnd(1)), f == QLatin1String("1/2") ? QStringLiteral("½") : f == QLatin1String("1/4") ? QStringLiteral("¼") : QStringLiteral("¾"), false};
    }
    if (ordinal.match(text).hasMatch()) return Fix{int(text.size()) - 2, int(text.size()), QString(), true};
    return std::nullopt;
}

QString smartQuote(const QString &s, QChar before)
{
    // Opening after a space, an opening bracket or the start of the paragraph; closing (and apostrophes) otherwise.
    const bool opening = before.isNull() || before.isSpace() || QStringLiteral("([{<—–“‘").contains(before);
    if (s == QLatin1String("\"")) return opening ? QStringLiteral("“") : QStringLiteral("”");
    if (s == QLatin1String("'")) return opening ? QStringLiteral("‘") : QStringLiteral("’");
    return s;
}

QString smartQuote(const QString &s, QChar before, const QString &language)
{
    const QString base = baseLanguage(language);
    if (base != QLatin1String("de") && base != QLatin1String("fr")) return smartQuote(s, before);
    const bool opening = before.isNull() || before.isSpace() || QStringLiteral("([{<—–„‚«").contains(before);
    const QChar nb(0x00A0);
    if (s == QLatin1String("\"")) {
        if (base == QLatin1String("de")) return opening ? QStringLiteral("„") : QStringLiteral("“");
        if (opening) return QStringLiteral("«") + QString(nb);
        return before == nb || before == QChar(0x202F) ? QStringLiteral("»") : QString(nb) + QStringLiteral("»");
    }
    if (s == QLatin1String("'")) {
        if (base == QLatin1String("de")) return opening ? QStringLiteral("‚") : QStringLiteral("‘");
        return opening ? QStringLiteral("‘") : QStringLiteral("’");
    }
    return s;
}

std::optional<ListStart> listStart(const QString &before)
{
    static const QRegularExpression numbered(QStringLiteral("^(\\d{1,3})([.)])$"));
    static const QRegularExpression lettered(QStringLiteral("^([a-z])\\)$"));
    const auto m = numbered.match(before);
    ListStart ls;
    ls.length = int(before.size());
    if (const auto l = lettered.match(before); l.hasMatch()) {
        ls.numbered = true;
        ls.paren = true;
        ls.letter = true;
        ls.number = l.captured(1)[0].unicode() - 'a' + 1;
        return ls;
    }
    if (m.hasMatch()) {
        ls.numbered = true;
        ls.paren = m.captured(2) == QLatin1String(")");
        ls.number = m.captured(1).toInt();
        return ls;
    }
    if (before == QLatin1String("*") || before == QLatin1String("-")) return ls;
    return std::nullopt;
}

std::optional<Fix> linkWord(const QString &before)
{
    static const QRegularExpression address(QStringLiteral("(?:^|[\\s(\\[<\"'“‘])((?:(?:https?://|www\\.)[^\\s<>]++|[\\w.+-]++@[\\w-]++(?:\\.[\\w-]++)++[^\\s<>]*+))$"));   // (possessive: a text that is not an address is looked at once, not again for every way to split it)
    const auto m = address.match(before);
    if (!m.hasMatch()) return std::nullopt;
    const int from = int(m.capturedStart(1));
    int to = int(m.capturedEnd(1));
    // The marks that end a sentence or close a bracket are not the address's.
    while (to > from && QStringLiteral(".,;:!?)\"'”’").contains(before[to - 1])) --to;
    const QString typed = before.mid(from, to - from);
    const bool mail = !typed.startsWith(QLatin1String("http")) && !typed.startsWith(QLatin1String("www.")) && typed.contains(QLatin1Char('@'));
    static const QRegularExpression plainMail(QStringLiteral("^[\\w.+-]+@[\\w-]+(?:\\.[\\w-]+)+$"));
    if (mail && !plainMail.match(typed).hasMatch()) return std::nullopt;
    if (!mail && typed.size() < 8) return std::nullopt;
    return Fix{from, to, mail ? QStringLiteral("mailto:") + typed : typed.startsWith(QLatin1String("www.")) ? QStringLiteral("https://") + typed : typed, false};
}

std::optional<Mark> markWord(const QString &before)
{
    static const QRegularExpression marked(QStringLiteral("(?:^|[\\s(])([*_])([^\\s*_](?:[^*_]*[^\\s*_])?)\\1[.,;:!?)]*$"));
    const auto m = marked.match(before);
    if (!m.hasMatch()) return std::nullopt;
    const int from = int(m.capturedStart(1));
    return Mark{from, int(m.capturedEnd(2)) + 1, m.captured(1)[0]};
}

std::optional<QChar> ruleLine(const QString &paragraph)
{
    if (paragraph.size() < 3) return std::nullopt;
    const QChar c = paragraph[0];
    if (c != QLatin1Char('-') && c != QLatin1Char('=') && c != QLatin1Char('_')) return std::nullopt;
    for (const QChar ch : paragraph)
        if (ch != c) return std::nullopt;
    return c;
}

} // namespace jp::autotext
