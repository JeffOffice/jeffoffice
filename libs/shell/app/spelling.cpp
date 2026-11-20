#include "app/spelling.h"

#include "app/settings.h"
#include "text/dictionaries.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QLockFile>
#include <QTimer>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
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
    if (w.isEmpty()) return true;
    if (!m_ignore.isEmpty() || !m_excluded.isEmpty()) {
        const QString key = wordKey(w);
        if (m_ignore.contains(key)) return true;                         // (Ignore All, for this session, beats the exclusion)
        if (m_excluded.contains(key)) return false;                      // (the person's own say beats every list that accepts)
    }
    if (isOwnName(w)) return true;
    if (m_custom.contains(w) || m_custom.contains(w.toLower())) return true;
    if (r.ignoreUpper && w == w.toUpper() && w.size() > 1) return true;
    if (r.ignoreNumbers && std::any_of(w.cbegin(), w.cend(), [](QChar c) { return c.isDigit(); })) return true;
    return dict::spell(code, w);
}

QStringList Speller::suggest(const QString &code, const QString &w)
{
    QStringList found = dict::suggest(code, w);
    if (!m_excluded.isEmpty())
        found.erase(std::remove_if(found.begin(), found.end(), [this](const QString &s) { return m_excluded.contains(wordKey(s)); }), found.end());
    return found;
}

void Speller::ignore(const QString &w)
{
    m_ignore.insert(wordKey(w));
    ++m_revision;
}

namespace {

QString s_userDictionaryPath;   // a test's own file

QString defaultDictionaryFolder()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir;
}

// Whether a line is fit to be a word of a list: not empty, not long, no space or control character in it (nor a stray byte of a text that was not UTF-8).
bool fitWord(const QString &w)
{
    if (w.isEmpty() || w.size() > kMaxWordLength) return false;
    for (const QChar c : w)
        if (c.isSpace() || c.category() == QChar::Other_Control || c.category() == QChar::Other_Surrogate || c == QChar(0xFFFD) || c == QChar(0x2028) || c == QChar(0x2029)) return false;
    return true;
}

// The bytes a list takes in its file.
qint64 listBytes(const QStringList &words)
{
    qint64 n = 0;
    for (const QString &w : words) n += w.toUtf8().size() + 1;
    return n;
}

} // namespace

bool userWordIsFit(const QString &word) { return fitWord(word); }

WordList parseWordList(const QByteArray &input)
{
    WordList list;
    QByteArray bytes = input;
    if (bytes.size() > kMaxWordListBytes) {
        bytes.truncate(kMaxWordListBytes);
        // (Cut in the middle of a word: that word goes.)
        const qsizetype end = std::max(bytes.lastIndexOf('\n'), bytes.lastIndexOf('\r'));
        bytes.truncate(end < 0 ? 0 : end);
        list.truncated = true;
    }
    if (bytes.startsWith("\xEF\xBB\xBF")) bytes.remove(0, 3);
    QSet<QString> seen;
    qint64 used = 0;
    qsizetype at = 0;
    while (at < bytes.size()) {
        // (A line ends with a line feed, a carriage return, or both: the old Macintosh files have only the second.)
        qsizetype end = at;
        while (end < bytes.size() && bytes[end] != '\n' && bytes[end] != '\r') ++end;
        const QByteArray raw = bytes.mid(at, end - at);
        at = end + 1;
        if (end + 1 < bytes.size() && bytes[end] == '\r' && bytes[end + 1] == '\n') ++at;
        const QString w = QString::fromUtf8(raw).trimmed();
        if (w.isEmpty()) continue;
        if (!fitWord(w) || QString::fromUtf8(raw).contains(QChar(0xFFFD))) {
            ++list.skipped;
            list.rejected << raw;
            continue;
        }
        if (seen.contains(w)) continue;
        used += w.toUtf8().size() + 1;
        if (used > kMaxWordListBytes) {
            list.truncated = true;
            break;
        }
        seen.insert(w);
        list.words << w;
    }
    return list;
}

WordList readWordListFile(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error && f.exists()) *error = f.errorString();
        return WordList();
    }
    return parseWordList(f.read(kMaxWordListBytes + 1));
}

bool writeWordListFile(const QString &path, const QStringList &words, QString *error, const QList<QByteArray> &rejected)
{
    // (A file that is read-only is the person's choice: the file is put in place by a rename, which would get round it.)
    const QFileInfo info(path);
    if (info.exists() && !info.isWritable()) {
        if (error) *error = QCoreApplication::translate("Proofing", "The file is read-only.");
        return false;
    }
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        if (error) *error = f.errorString();
        return false;
    }
    auto put = [&](const QByteArray &line) {
        if (f.write(line) == line.size()) return true;
        if (error) *error = f.errorString();
        f.cancelWriting();
        return false;
    };
    for (const QString &w : words)
        if (!put(w.toUtf8() + '\n')) return false;
    for (const QByteArray &line : rejected)
        if (!put(line + '\n')) return false;
    if (!f.commit()) {
        if (error) *error = f.errorString();
        return false;
    }
    return true;
}

QStringList userDictionaryWords(const QString &path) { return readWordListFile(path).words; }

QString Speller::wordKey(const QString &w)
{
    QString key = w.toLower();
    key.replace(QChar(0x2019), QLatin1Char('\''));
    return key;
}

Speller::Speller() { reload(); }

Speller::Speller(const QString &customPath) : m_fixedPath(customPath) { reload(); }

Speller::~Speller() { delete m_watch; }

QString Speller::customPath() const
{
    if (!m_fixedPath.isEmpty()) return m_fixedPath;
    if (!s_userDictionaryPath.isEmpty()) return s_userDictionaryPath;
    return defaultDictionaryFolder() + QStringLiteral("/custom.dic");
}

QString Speller::excludedPath() const
{
    if (!m_fixedPath.isEmpty() || !s_userDictionaryPath.isEmpty()) return customPath() + QStringLiteral(".excluded");
    return defaultDictionaryFolder() + QStringLiteral("/excluded.dic");
}

void Speller::setUserDictionaryPath(const QString &path)
{
    s_userDictionaryPath = path;
    Speller &s = get();
    s.reload();
    if (s.m_watch) s.m_watch->watchFiles({s.customPath(), s.excludedPath()});
}

bool Speller::reload()
{
    QSet<QString> custom, excluded;
    for (const QString &w : readWordListFile(customPath()).words) custom.insert(w);
    for (const QString &w : readWordListFile(excludedPath()).words) excluded.insert(wordKey(w));
    if (custom == m_custom && excluded == m_excluded) return false;
    m_custom = custom;
    m_excluded = excluded;
    ++m_revision;
    if (m_watch) Q_EMIT m_watch->changed();
    return true;
}

SpellingWatch *Speller::watch()
{
    if (!m_watch) {
        m_watch = new SpellingWatch(this);
        m_watch->watchFiles({customPath(), excludedPath()});
    }
    return m_watch;
}

SpellingWatch::SpellingWatch(Speller *speller) : m_speller(speller), m_files(new QFileSystemWatcher(this)), m_later(new QTimer(this))
{
    m_later->setSingleShot(true);
    m_later->setInterval(200);   // (a save is a few events: they are read once)
    connect(m_files, &QFileSystemWatcher::fileChanged, this, &SpellingWatch::touched);
    connect(m_files, &QFileSystemWatcher::directoryChanged, this, &SpellingWatch::touched);
    connect(m_later, &QTimer::timeout, this, [this] {
        watchFiles(m_paths);   // (a file put in place by a rename is a new file: watched again)
        m_speller->reload();
    });
}

void SpellingWatch::watchFiles(const QStringList &files)
{
    m_paths = files;
    if (!m_files->files().isEmpty()) m_files->removePaths(m_files->files());
    if (!m_files->directories().isEmpty()) m_files->removePaths(m_files->directories());
    for (const QString &f : files) {
        const QFileInfo info(f);
        if (info.exists()) m_files->addPath(f);
        if (info.dir().exists()) m_files->addPath(info.absolutePath());
    }
}

void SpellingWatch::touched() { m_later->start(); }

QStringList Speller::customWords(bool again)
{
    if (again) reload();
    QStringList out = m_custom.values();
    out.sort();
    return out;
}

QStringList Speller::excludedWords()
{
    QStringList out = m_excluded.values();
    out.sort();
    return out;
}

bool Speller::isExcluded(const QString &w) const { return m_excluded.contains(wordKey(w)); }

bool Speller::change(const WordChanges &c, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        reload();   // (what the files hold is what is kept: nothing is changed by a failure)
        return false;
    };
    if (c.isEmpty()) return true;
    for (const QStringList *list : {&c.addCustom, &c.addExcluded})
        for (const QString &w : *list)
            if (!fitWord(w.trimmed()))
                return fail(QCoreApplication::translate("Proofing", "A word can't be empty, have a space or a control character in it, or be longer than %n letters.", nullptr, kMaxWordLength));
    const QString customFile = customPath(), excludedFile = excludedPath();
    // One lock for both files, beside the first, waited for a short time: another app may be saving its words at this moment.
    QLockFile lock(customFile + QStringLiteral(".lock"));
    lock.setStaleLockTime(30000);
    if (!lock.tryLock(5000))
        return fail(lock.error() == QLockFile::LockFailedError ? QCoreApplication::translate("Proofing", "Another program is saving words just now. Try again in a moment.")
                                                               : QCoreApplication::translate("Proofing", "The folder of the dictionary can't be written to."));
    for (const QString &file : {customFile, excludedFile}) {
        const QFileInfo info(file);
        if (info.exists() && !info.isWritable()) return fail(QCoreApplication::translate("Proofing", "The file is read-only."));
    }
    QString reason;
    const WordList custom = readWordListFile(customFile, &reason);
    if (!reason.isEmpty()) return fail(reason);
    const WordList excluded = readWordListFile(excludedFile, &reason);
    if (!reason.isEmpty()) return fail(reason);
    // (A list that is longer than is read would lose its end when written whole.)
    if (custom.truncated || excluded.truncated) return fail(QCoreApplication::translate("Proofing", "The list of words is too long to change."));
    QStringList customWords = custom.words, excludedWords;
    for (const QString &w : excluded.words) {
        const QString key = wordKey(w);
        if (!excludedWords.contains(key)) excludedWords << key;
    }
    const QStringList excludedBefore = excludedWords;
    auto dropKey = [](QStringList &list, const QString &key) {
        list.erase(std::remove_if(list.begin(), list.end(), [&](const QString &w) { return wordKey(w) == key; }), list.end());
    };
    // (Removals first, then the additions: a word moved from one list to the other is both.)
    for (const QString &w : c.removeCustom) customWords.removeAll(w.trimmed());
    for (const QString &w : c.removeExcluded) dropKey(excludedWords, wordKey(w.trimmed()));
    for (const QString &w : c.addExcluded) {
        const QString key = wordKey(w.trimmed());
        if (!excludedWords.contains(key)) excludedWords << key;
        dropKey(customWords, key);   // (a word is in one list: the excluded one)
    }
    for (const QString &w : c.addCustom) {
        const QString word = w.trimmed();
        dropKey(excludedWords, wordKey(word));   // (a word added is no longer one the person wants flagged)
        if (!customWords.contains(word)) customWords << word;
    }
    if (listBytes(customWords) > kMaxWordListBytes || listBytes(excludedWords) > kMaxWordListBytes) return fail(QCoreApplication::translate("Proofing", "The list of words is too long to add to."));
    // (The excluded list first: if the next write fails, a word that is in both is excluded, and none is lost.)
    if (excludedWords != excludedBefore && !writeWordListFile(excludedFile, excludedWords, &reason, excluded.rejected)) return fail(reason);
    if (customWords != custom.words && !writeWordListFile(customFile, customWords, &reason, custom.rejected)) return fail(reason);
    reload();
    m_saveFailed = false;
    return true;
}

bool Speller::add(const QString &w, QString *error)
{
    QString reason;
    if (change(WordChanges{{w}, {}, {}, {}}, &reason)) return true;
    if (error && !m_saveFailed) *error = reason.isEmpty() ? QCoreApplication::translate("Proofing", "The word couldn't be saved.") : reason;
    m_saveFailed = true;
    return false;
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
    QString text = b.isValid() ? b.text() : QString();
    // Text that is hidden (what a tracked change deleted, for one) is not checked: it is blanks, so that the places hold.
    if (b.isValid())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().boolProperty(tp::HiddenText)) {
                const int at = it.fragment().position() - b.position();
                for (int k = 0; k < int(it.fragment().length()) && at + k < text.size(); ++k) text[at + k] = QLatin1Char(' ');
            }
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
