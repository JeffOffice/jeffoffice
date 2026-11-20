#pragma once
// Spelling: the bundled dictionaries (text/dictionaries), with the user's own
// words and the words ignored for the session, and the misspelled ranges of
// a story, for every app's checking as you type and its Spelling dialog.

#include "core/export.h"

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

class QRegularExpression;
class QFileSystemWatcher;
class QTimer;
class QWidget;
class QTextBlock;
class QTextDocument;

namespace jp {

class SpellingWatch;

// The dictionaries themselves are shared with hyphenation (text/dictionaries);
// this adds the user's own words and the words ignored for the session.
class Speller {
public:
    // The options that let a word go unchecked (Options > Proofing), read once for a run of words.
    struct Rules {
        bool ignoreUpper = true, ignoreNumbers = true;
    };
    static Speller &get();
    static Rules rules();
    bool check(const QString &code, const QString &w);
    bool check(const QString &code, const QString &w, const Rules &rules);
    QStringList suggest(const QString &code, const QString &w);
    void ignore(const QString &w);
    // Adds a word to the user's dictionary, in the file of the app's own folder: the file is read again, the word put in, and the file written in one step, under a lock, so that a word another app
    // added a moment ago stays. The word counts once it is saved. False when it couldn't be (nothing is changed); `error` then says why, the first time after a save that worked (a person is told
    // once, not at every word).
    bool add(const QString &w, QString *error = nullptr);
    // Where the words are kept, in place of the app's own folder (a test's); empty for that folder again. The excluded words are kept beside them.
    static void setUserDictionaryPath(const QString &path);
    // A speller of its own over the words of one file (and the excluded ones beside it), as another app that has the same files open would have: for the tests.
    explicit Speller(const QString &customPath);
    ~Speller();
    Speller(const Speller &) = delete;
    Speller &operator=(const Speller &) = delete;
    // The user's own words and the excluded ones (words flagged although a dictionary accepts them, and never suggested), sorted, for the dictionary dialog. Both are the
    // person's, for every app and every language, and are kept in files of the app's folder, never in a document. `reload` reads the files again first (another app may have added words).
    QStringList customWords(bool reload = false);
    QStringList excludedWords();
    // Changes to the words, each applied to what the files hold now (never to a copy kept from before): other apps that have the files open add and exclude words too. Removals are made first.
    struct WordChanges {
        QStringList addCustom, removeCustom, addExcluded, removeExcluded;
        bool isEmpty() const { return addCustom.isEmpty() && removeCustom.isEmpty() && addExcluded.isEmpty() && removeExcluded.isEmpty(); }
    };
    // Takes a lock, reads both files, makes the changes (a word added to one list leaves the other), and writes each file in one step. Lines of a file that are not words it reads (a line with a
    // space in it, from an older release) stay in it as they were. False, with the reason in `error`, when the lock could not be had, a file is read-only or can't be read or written, or is too long
    // to change: nothing is then changed in memory, and a file that was written stays written (the excluded list first, so that no word is lost).
    bool change(const WordChanges &changes, QString *error = nullptr);
    bool isExcluded(const QString &w) const;
    // What the checker compares excluded and ignored words by: the word in lowercase, with the typographic apostrophe as the plain one.
    static QString wordKey(const QString &w);
    // The files are watched: another app's change is read in a moment after it, and `changed` says so (the pages are checked again). Made when first asked for, on the thread that asks.
    SpellingWatch *watch();
    // Reads both files again now; true when a list is not what it was (the revision then counts a change).
    bool reload();
    // Counts the changes to the words that are accepted (Ignore All, Add to Dictionary): an answer kept from before a change is stale.
    int revision() const { return m_revision; }

private:
    Speller();
    QString customPath() const;
    QString excludedPath() const;
    void pathsChanged();
    QSet<QString> m_custom, m_ignore;   // the user's words (every language), and this session's
    QSet<QString> m_excluded;           // the words flagged whatever the dictionary says (as wordKey)
    QString m_fixedPath;                // a speller made for a file of its own
    int m_revision = 0;
    bool m_saveFailed = false;          // the last save failed
    SpellingWatch *m_watch = nullptr;
    friend class SpellingWatch;
};

// Tells that the words of the files have changed under the speller (another app saved them).
class JO_COMMON_EXPORT SpellingWatch : public QObject {
    Q_OBJECT
public:
    explicit SpellingWatch(Speller *speller);
    void watchFiles(const QStringList &files);
Q_SIGNALS:
    void changed();
private:
    void touched();
    Speller *m_speller;
    QFileSystemWatcher *m_files;
    QTimer *m_later;
    QStringList m_paths;
    friend class Speller;
};

// What a word is, for checking.
const QRegularExpression &spellingWordPattern();
// Misspelled ranges in a story, skipping fields and "no proofing" text.
QVector<QPair<int, int>> misspellings(QTextDocument *doc); // [start, end) document ranges
// The same for one paragraph. Text that states no language is `fallback` (BCP-47; empty is US English).
QVector<QPair<int, int>> misspellingsIn(const QTextBlock &block, const QString &fallback = QString());
// A part of a paragraph: the words from the paragraph's offset `from`, at most `maxWords` of them. `stopped` is the offset to go on from (the paragraph's length when
// it is done), and `words` how many were looked at. The ranges are document positions, as above.
QVector<QPair<int, int>> misspellingsIn(const QTextBlock &block, const QString &fallback, int from, int maxWords, int *stopped, int *words);
// The words of a user dictionary file (one a line; a byte order mark, Windows line ends, and blanks are let go) as far as the limit on its size.
QStringList userDictionaryWords(const QString &path);
// A word list file as it is read for the user's dictionary: the most that is taken in, and what is let go.
constexpr qint64 kMaxWordListBytes = 1 << 20;   // a list file is read as far as this, and a list is kept as far as it (so it reads back whole)
constexpr int kMaxWordLength = 100;             // letters of one word
struct WordList {
    QStringList words;      // in the order of the file, each once, the ones that are fit to be words
    int skipped = 0;        // lines that were not (blank lines are not counted)
    QList<QByteArray> rejected;   // those lines as they are in the file (a line with a space, from an older release, is the person's: it is kept whenever the file is written)
    bool truncated = false; // the file, or the list, went past a limit and the rest was left
};
// Whether a word is fit to be in a list (not empty, at most kMaxWordLength letters, no space or control character in it).
bool userWordIsFit(const QString &word);
// The words of the bytes of a word list (UTF-8, one a line, ended by a line feed, a carriage return, or both; a byte order mark and blank lines are let go). A line is let go when it is longer than kMaxWordLength letters,
// has a control character or a space in it, or is not valid UTF-8. At most kMaxWordListBytes of the file are read, and no more words are kept than fit in a file of that size.
WordList parseWordList(const QByteArray &bytes);
// Reads a word list file (at most kMaxWordListBytes of it); `error` says why when it can't be opened.
WordList readWordListFile(const QString &path, QString *error = nullptr);
// Writes a word list, one word a line, UTF-8, and then `rejected` lines as they were: the file is made whole in a temporary file and put in place in one step, so that a failure leaves the file that
// was there as it was. A file that is read-only is not replaced (the error says so).
bool writeWordListFile(const QString &path, const QStringList &words, QString *error = nullptr, const QList<QByteArray> &rejected = QList<QByteArray>());
// Changes with Ignore All and Add to Dictionary: an answer kept from before is stale.
int spellingRevision();
QStringList spellingSuggestions(const QString &word, const QString &language = QString());   // language: BCP-47, "" US English
bool spellingAdd(const QString &word, QString *firstFailure = nullptr);   // add to the user dictionary (see Speller::add)
// Tells the person that a word couldn't be saved in their dictionary, with the reason `spellingAdd` gave (empty: nothing to say, as for a failure already told).
void spellingReportFailure(QWidget *parent, const QString &firstFailure);
void spellingIgnore(const QString &word);                 // ignore for this session

} // namespace jp
