#pragma once
// The custom dictionary dialog (Options > Proofing > Custom Dictionary): the words the person added, which the spelling check accepts, and the excluded words, which it flags although a
// dictionary accepts them and never suggests. One list of each is kept for the person (app/spelling.h), the same for every app and language; a word is in one list or the other.

#include "core/export.h"

#include <QSet>
#include <QString>
#include <QStringList>
#include <QWidget>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTabWidget;

namespace jp {

class JO_COMMON_EXPORT UserDictionaryPage : public QWidget {
    Q_OBJECT
public:
    // The two lists, as they stand now in the speller (read again from the files first).
    explicit UserDictionaryPage(QWidget *parent = nullptr);
    QStringList addedWords() const { return sorted(m_words[0]); }
    QStringList excludedWords() const { return sorted(m_words[1]); }
    // Adds a word to a list (taking it from the other): false, with nothing changed, when it is not fit to be a word (empty, longer than kMaxWordLength, a space or control character in it).
    bool addWord(bool excluded, const QString &word);
    // Takes the words of a plain word list file (UTF-8, one a line) into a list; the sentence that says how it went is shown under the lists and returned. A file that can't be read says why.
    QString importFile(bool excluded, const QString &path);
    // Writes a list as a plain word list file (UTF-8, one a line); false, with the reason, when it can't be written.
    bool exportFile(bool excluded, const QString &path, QString *error = nullptr);
    // Keeps what was changed (as Speller::change, to the words the files hold now); false, with the reason, when a file could not be written.
    bool apply(QString *error = nullptr);

private:
    struct Tab {
        QWidget *page = nullptr;
        QLineEdit *find = nullptr, *edit = nullptr;
        QListWidget *list = nullptr;
    };
    Tab makeTab(bool excluded);
    void fill(int which);
    void removeSelected(int which);
    void addTyped(int which);
    void chooseImport(int which);
    void chooseExport(int which);
    static QStringList sorted(const QSet<QString> &words);
    QSet<QString> m_words[2];   // the user's words, and the excluded words (as Speller::wordKey)
    QSet<QString> m_opened[2];  // the same when the dialog opened
    Tab m_tabs[2];
    QTabWidget *m_tabWidget = nullptr;
    QLabel *m_status = nullptr;
};

// The dialog; true when the person accepted it and the lists were saved (a failure to save is told).
JO_COMMON_EXPORT bool userDictionaryDialog(QWidget *parent);

} // namespace jp
