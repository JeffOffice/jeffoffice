#include "app/spelling.h"

#include "app/settings.h"
#include "text/dictionaries.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <algorithm>
#include <limits>
#include "app/plaintext.h"

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

namespace {

// The names of the suite's own apps and of the company, right in every language and every app: no dictionary lists them. A name is right as it is written,
// in capitals, and with a possessive ("JeffWrite's", "JEFFWRITE'S"); "JeffOffice LLC" is two words to the checker.
bool isOwnName(QString w)
{
    static const QSet<QString> names = [] {
        QSet<QString> set;
        for (const char *n : {"JeffWrite", "JeffPub", "JeffDeck", "JeffOffice", "JeffCrunch", "JeffMail", "JeffBase", "LLC"}) {
            set.insert(QString::fromLatin1(n));
            set.insert(QString::fromLatin1(n).toUpper());
        }
        return set;
    }();
    // (A name in capitals has its possessive in capitals: "JEFFWRITE'S".)
    if (w.size() > 2 && (w.endsWith(QLatin1String("'s"), Qt::CaseInsensitive) || w.endsWith(QString(QChar(0x2019)) + QLatin1Char('s'), Qt::CaseInsensitive))) w.chop(2);
    return names.contains(w);
}

} // namespace

bool Speller::check(const QString &code, const QString &w) { return check(code, w, rules()); }

bool Speller::check(const QString &code, const QString &w, const Rules &r)
{
    if (w.isEmpty() || isOwnName(w)) return true;
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

namespace {

constexpr qint64 kMaxUserDictionaryBytes = 1 << 20;   // as much of the file as is read
constexpr int kMaxUserWordLength = 100;
QString s_userDictionaryPath;                          // a test's own file

} // namespace

QStringList userDictionaryWords(const QString &path)
{
    QStringList words;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return words;
    QByteArray bytes = f.read(kMaxUserDictionaryBytes);
    // (A file longer than the limit is cut in the middle of a word: that word goes.)
    if (f.read(1).size() > 0) {
        const qsizetype end = bytes.lastIndexOf('\n');
        bytes.truncate(end < 0 ? 0 : end);
    }
    QString text = QString::fromUtf8(bytes);
    if (text.startsWith(QChar(0xFEFF))) text.remove(0, 1);
    for (const QString &line : text.split(QLatin1Char('\n'))) {
        const QString w = line.trimmed();
        if (!w.isEmpty() && w.size() <= kMaxUserWordLength) words << w;
    }
    return words;
}

bool addToUserDictionary(const QString &path, const QString &word, QString *error)
{
    const QString w = word.trimmed();
    if (w.isEmpty() || w.contains(QLatin1Char('\n')) || w.contains(QLatin1Char('\r'))) {
        if (error) *error = QCoreApplication::translate("Proofing", "A word to add can't be empty or run over more than one line.");
        return false;
    }
    QFile f(path);
    if (!f.open(QIODevice::ReadWrite)) {
        if (error) *error = f.errorString();
        return false;
    }
    // On a line of its own: the file may not end with a line end.
    QByteArray line = w.toUtf8() + '\n';
    if (f.size() > 0 && f.seek(f.size() - 1)) {
        char last = 0;
        if (f.read(&last, 1) == 1 && last != '\n') line.prepend('\n');
    }
    if (!f.seek(f.size()) || f.write(line) != line.size() || !f.flush()) {
        if (error) *error = f.errorString();
        return false;
    }
    return true;
}

bool Speller::add(const QString &w, QString *error)
{
    m_custom.insert(w);
    ++m_revision;
    QString reason;
    if (addToUserDictionary(userDict(), w, &reason)) {
        m_saveFailed = false;
        return true;
    }
    if (error && !m_saveFailed) *error = reason.isEmpty() ? QCoreApplication::translate("Proofing", "The word couldn't be saved.") : reason;
    m_saveFailed = true;
    return false;
}

void Speller::setUserDictionaryPath(const QString &path) { s_userDictionaryPath = path; }

Speller::Speller()
{
    for (const QString &w : userDictionaryWords(userDict())) m_custom.insert(w);
}

QString Speller::userDict()
{
    if (!s_userDictionaryPath.isEmpty()) return s_userDictionaryPath;
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
    int stopped = 0, words = 0;
    return misspellingsIn(b, fallback, 0, std::numeric_limits<int>::max(), &stopped, &words);
}

QVector<QPair<int, int>> misspellingsIn(const QTextBlock &b, const QString &fallback, int from, int maxWords, int *stopped, int *words)
{
    QVector<QPair<int, int>> out;
    const QString text = b.isValid() ? b.text() : QString();
    if (stopped) *stopped = int(text.size());
    if (words) *words = 0;
    if (dict::root().isEmpty() || !b.isValid() || text.size() < 2 || from >= text.size()) return out;
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
    int looked = 0;
    auto it = token.globalMatch(text, from);
    while (it.hasNext()) {
        const auto m = it.next();
        const int start = int(m.capturedStart());
        QString w = m.captured();
        while (w.endsWith('\'') || w.endsWith(QChar(0x2019))) w.chop(1);
        if (w.size() < 2 || std::none_of(w.cbegin(), w.cend(), [](QChar c) { return c.isLetter(); })) continue;
        while (at < stretches.size() && stretches[at].to <= start) ++at;
        if (at >= stretches.size()) break;
        ++looked;
        if (!stretches[at].skip && !speller.check(stretches[at].code, w, rules)) out << qMakePair(b.position() + start, b.position() + start + int(w.size()));
        if (looked >= maxWords && it.hasNext()) {
            if (stopped) *stopped = int(m.capturedEnd());
            break;
        }
    }
    if (words) *words = looked;
    return out;
}

int spellingRevision() { return Speller::get().revision(); }

QStringList spellingSuggestions(const QString &w, const QString &language)
{
    const QString code = dict::match(language);
    return code.isEmpty() ? QStringList() : Speller::get().suggest(code, w);
}
bool spellingAdd(const QString &w, QString *firstFailure) { return Speller::get().add(w, firstFailure); }

void spellingReportFailure(QWidget *parent, const QString &firstFailure)
{
    if (firstFailure.isEmpty()) return;
    jp::plainWarning(parent, QCoreApplication::translate("Proofing", "Add to Dictionary"),
                         QCoreApplication::translate("Proofing", "The word can't be saved in your dictionary, so it is accepted only until you close the program.\n%1").arg(firstFailure));
}
void spellingIgnore(const QString &w) { Speller::get().ignore(w); }

} // namespace jp
