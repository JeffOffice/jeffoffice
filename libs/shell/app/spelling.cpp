#include "app/spelling.h"

#include "app/settings.h"
#include "text/dictionaries.h"
#include "text/textprops.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <algorithm>

namespace jp {

Speller &Speller::get()
{
    static Speller s;
    return s;
}

Speller::Rules Speller::rules()
{
    Settings &st = Settings::get();
    Rules r;
    r.ignoreUpper = st.value("proof/ignoreUpper", true).toBool();
    r.ignoreNumbers = st.value("proof/ignoreNumbers", true).toBool();
    return r;
}

bool Speller::check(const QString &code, const QString &w) { return check(code, w, rules()); }

bool Speller::check(const QString &code, const QString &w, const Rules &r)
{
    if (w.isEmpty()) return true;
    if (m_ignore.contains(w.toLower()) || m_custom.contains(w) || m_custom.contains(w.toLower())) return true;
    if (r.ignoreUpper && w == w.toUpper() && w.size() > 1) return true;
    if (r.ignoreNumbers && std::any_of(w.cbegin(), w.cend(), [](QChar c) { return c.isDigit(); })) return true;
    return dict::spell(code, w);
}

QStringList Speller::suggest(const QString &code, const QString &w) { return dict::suggest(code, w); }

void Speller::ignore(const QString &w)
{
    m_ignore.insert(w.toLower());
    ++m_revision;
}

void Speller::add(const QString &w)
{
    m_custom.insert(w);
    ++m_revision;
    QFile f(userDict());
    if (f.open(QIODevice::Append)) f.write((w + "\n").toUtf8());
}

Speller::Speller()
{
    QFile f(userDict());
    if (f.open(QIODevice::ReadOnly))
        for (const QByteArray &line : f.readAll().split('\n'))
            if (!line.trimmed().isEmpty()) m_custom.insert(QString::fromUtf8(line.trimmed()));
}

QString Speller::userDict()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/custom.dic";
}

const QRegularExpression &spellingWordPattern()
{
    static const QRegularExpression re(QStringLiteral("[\\p{L}][\\p{L}\\p{M}'’]*"));
    return re;
}

QVector<QPair<int, int>> misspellings(QTextDocument *doc)
{
    QVector<QPair<int, int>> out;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) out += misspellingsIn(b);
    return out;
}

QVector<QPair<int, int>> misspellingsIn(const QTextBlock &b, const QString &fallback)
{
    QVector<QPair<int, int>> out;
    if (dict::root().isEmpty() || !b.isValid()) return out;
    const QString text = b.text();
    if (text.size() < 2) return out;
    // The dictionary and the "do not check" of each stretch of the paragraph, in order. Text in a language without a dictionary isn't checked.
    struct Stretch {
        int to;
        QString code;
        bool skip;
    };
    QVector<Stretch> stretches;
    QString lastLanguage, lastCode;
    bool lastSkip = false, haveLast = false;
    for (auto it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat cf = f.charFormat();
        Stretch s{f.position() - b.position() + f.length(), QString(), cf.boolProperty(tp::NoProof)};
        if (!s.skip) {
            const QString language = cf.stringProperty(tp::Language);
            if (!haveLast || language != lastLanguage) {
                lastLanguage = language;
                lastCode = dict::match(language.isEmpty() ? fallback : language);
                lastSkip = lastCode.isEmpty() || !dict::hasSpelling(lastCode);
                haveLast = true;
            }
            s.code = lastCode;
            s.skip = lastSkip;
        }
        stretches << s;
    }
    const Speller::Rules rules = Speller::rules();
    Speller &speller = Speller::get();
    int at = 0;
    // (A word with digits in it is one word, which the options may let go: "3rd" and "abc123", not "rd" and "abc".)
    static const QRegularExpression token(QStringLiteral("[\\p{L}\\p{Nd}][\\p{L}\\p{M}\\p{Nd}'’]*"));
    auto it = token.globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        const int start = int(m.capturedStart());
        QString w = m.captured();
        while (w.endsWith('\'') || w.endsWith(QChar(0x2019))) w.chop(1);
        if (w.size() < 2 || std::none_of(w.cbegin(), w.cend(), [](QChar c) { return c.isLetter(); })) continue;
        while (at < stretches.size() && stretches[at].to <= start) ++at;
        if (at >= stretches.size()) break;
        if (stretches[at].skip) continue;
        if (!speller.check(stretches[at].code, w, rules)) out << qMakePair(b.position() + start, b.position() + start + int(w.size()));
    }
    return out;
}

int spellingRevision() { return Speller::get().revision(); }

QStringList spellingSuggestions(const QString &w, const QString &language)
{
    const QString code = dict::match(language);
    return code.isEmpty() ? QStringList() : Speller::get().suggest(code, w);
}
void spellingAdd(const QString &w) { Speller::get().add(w); }
void spellingIgnore(const QString &w) { Speller::get().ignore(w); }

} // namespace jp
