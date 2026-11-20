#pragma once
// Spelling: the bundled dictionaries (text/dictionaries), with the user's own
// words and the words ignored for the session, and the misspelled ranges of
// a story, for every app's checking as you type and its Spelling dialog.

#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

class QRegularExpression;
class QWidget;
class QTextBlock;
class QTextDocument;

namespace jp {

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
    // Adds a word to the user's dictionary: it counts at once, and is saved in a file of the app's own folder. False when it couldn't be saved; `error` then says why,
    // the first time after a save that worked (a person is told once, not at every word).
    bool add(const QString &w, QString *error = nullptr);
    // Where the words are kept, in place of the app's own folder (a test's); empty for that folder again. The excluded words are kept beside them.
    static void setUserDictionaryPath(const QString &path);
    // The user's own words and the excluded ones (words flagged although a dictionary accepts them, and never suggested), sorted, for the dictionary dialog. Both are the
    // person's, for every app and every language, and are kept in files of the app's folder, never in a document. `reload` reads the files again first (another app may have added words).
    QStringList customWords(bool reload = false);
    QStringList excludedWords();
    // Keeps these lists as the user's words and the excluded words, each file written whole and in one step (a file that can't be written is left as it was). False, with the reason in
    // `error`, when a file could not be written: the list of that file is then not changed (the excluded list may have been saved when the user's words were not).
    bool setWordLists(const QStringList &custom, const QStringList &excluded, QString *error = nullptr);
    bool isExcluded(const QString &w) const;
    // Counts the changes to the words that are accepted (Ignore All, Add to Dictionary): an answer kept from before a change is stale.
    int revision() const { return m_revision; }

private:
    Speller();
    static QString userDict();
    static QString excludedDict();
    QSet<QString> m_custom, m_ignore;   // the user's words (every language), and this session's
    QSet<QString> m_excluded;           // the words flagged whatever the dictionary says (lowercase)
    int m_revision = 0;
    bool m_saveFailed = false;          // the last save failed
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
// The words of a user dictionary file (one a line; a byte order mark, Windows line ends, and blanks are let go) as far as the limit on its size, and a word
// added to it: on a line of its own, whether or not the file ended with a line end. False, with the reason, when the word is empty, has a line in it, or can't be written.
QStringList userDictionaryWords(const QString &path);
bool addToUserDictionary(const QString &path, const QString &word, QString *error = nullptr);
// A word list file as it is read for the user's dictionary: the most that is taken in, and what is let go.
constexpr qint64 kMaxWordListBytes = 1 << 20;   // a list file is read as far as this, and a list is kept as far as it (so it reads back whole)
constexpr int kMaxWordLength = 100;             // letters of one word
struct WordList {
    QStringList words;      // in the order of the file, each once, the ones that are fit to be words
    int skipped = 0;        // lines that were not (blank lines are not counted)
    bool truncated = false; // the file, or the list, went past a limit and the rest was left
};
// Whether a word is fit to be in a list (not empty, at most kMaxWordLength letters, no space or control character in it).
bool userWordIsFit(const QString &word);
// The words of the bytes of a word list (UTF-8, one a line; a byte order mark, Windows line ends, and blank lines are let go). A line is let go when it is longer than kMaxWordLength letters,
// has a control character or a space in it, or is not valid UTF-8. At most kMaxWordListBytes of the file are read, and no more words are kept than fit in a file of that size.
WordList parseWordList(const QByteArray &bytes);
// Reads a word list file (at most kMaxWordListBytes of it); `error` says why when it can't be opened.
WordList readWordListFile(const QString &path, QString *error = nullptr);
// Writes a word list, one word a line, UTF-8: the file is made whole in a temporary file and put in place in one step, so that a failure leaves the file that was there as it was.
bool writeWordListFile(const QString &path, const QStringList &words, QString *error = nullptr);
// Changes with Ignore All and Add to Dictionary: an answer kept from before is stale.
int spellingRevision();
QStringList spellingSuggestions(const QString &word, const QString &language = QString());   // language: BCP-47, "" US English
bool spellingAdd(const QString &word, QString *firstFailure = nullptr);   // add to the user dictionary (see Speller::add)
// Tells the person that a word couldn't be saved in their dictionary, with the reason `spellingAdd` gave (empty: nothing to say, as for a failure already told).
void spellingReportFailure(QWidget *parent, const QString &firstFailure);
void spellingIgnore(const QString &word);                 // ignore for this session

} // namespace jp
