#include "app/uistyle.h"
#include "app/userdictionarydialog.h"

#include "app/plaintext.h"
#include "app/spelling.h"

#include <QCoreApplication>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

QStringList UserDictionaryPage::sorted(const QSet<QString> &words)
{
    QStringList out = words.values();
    out.sort();
    return out;
}

UserDictionaryPage::UserDictionaryPage(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("userDictionaryPage"));
    Speller &speller = Speller::get();
    const QStringList added = speller.customWords(true);
    m_words[0] = QSet<QString>(added.begin(), added.end());
    const QStringList excluded = speller.excludedWords();
    m_words[1] = QSet<QString>(excluded.begin(), excluded.end());
    m_opened[0] = m_words[0];
    m_opened[1] = m_words[1];
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setObjectName(QStringLiteral("userDictionaryTabs"));
    m_tabs[0] = makeTab(false);
    m_tabs[1] = makeTab(true);
    m_tabWidget->addTab(m_tabs[0].page, QCoreApplication::translate("UserDictionary", "Added Words"));
    m_tabWidget->addTab(m_tabs[1].page, QCoreApplication::translate("UserDictionary", "Excluded Words"));
    v->addWidget(m_tabWidget, 1);
    m_status = plainLabel(new QLabel(this));
    m_status->setObjectName(QStringLiteral("userDictionaryStatus"));
    m_status->setWordWrap(true);
    m_status->setAccessibleName(QCoreApplication::translate("UserDictionary", "Result"));
    v->addWidget(m_status);
    fill(0);
    fill(1);
}

UserDictionaryPage::Tab UserDictionaryPage::makeTab(bool excluded)
{
    const int which = excluded ? 1 : 0;
    Tab t;
    t.page = new QWidget(m_tabWidget);
    t.page->setObjectName(excluded ? QStringLiteral("excludedWordsTab") : QStringLiteral("addedWordsTab"));
    auto *v = new QVBoxLayout(t.page);
    auto *note = plainLabel(new QLabel(excluded ? QCoreApplication::translate("UserDictionary", "Words that are always marked as misspelled, even when a dictionary accepts them, and never suggested. A word is excluded just as it is written, not its other forms (\"house\" does not exclude \"houses\"). Adding a word to the dictionary takes it off this list.")
                                                : QCoreApplication::translate("UserDictionary", "Words you added: the spelling check accepts them in every language.")));
    note->setWordWrap(true);
    v->addWidget(note);
    t.find = new QLineEdit(t.page);
    t.find->setObjectName(excluded ? QStringLiteral("excludedWordsFind") : QStringLiteral("addedWordsFind"));
    t.find->setPlaceholderText(QCoreApplication::translate("UserDictionary", "Find"));
    t.find->setAccessibleName(QCoreApplication::translate("UserDictionary", "Find a word in the list"));
    t.find->setClearButtonEnabled(true);
    v->addWidget(t.find);
    t.list = new QListWidget(t.page);
    t.list->setObjectName(excluded ? QStringLiteral("excludedWords") : QStringLiteral("addedWords"));
    t.list->setAccessibleName(excluded ? QCoreApplication::translate("UserDictionary", "Excluded words") : QCoreApplication::translate("UserDictionary", "Added words"));
    t.list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    v->addWidget(t.list, 1);
    auto *row = new QHBoxLayout();
    t.edit = new QLineEdit(t.page);
    t.edit->setObjectName(excluded ? QStringLiteral("excludedWordEdit") : QStringLiteral("addedWordEdit"));
    t.edit->setAccessibleName(QCoreApplication::translate("UserDictionary", "Word to add"));
    auto *add = new QPushButton(QCoreApplication::translate("UserDictionary", "Add"), t.page);
    add->setObjectName(excluded ? QStringLiteral("excludedWordAdd") : QStringLiteral("addedWordAdd"));
    auto *remove = new QPushButton(QCoreApplication::translate("UserDictionary", "Remove"), t.page);
    remove->setObjectName(excluded ? QStringLiteral("excludedWordRemove") : QStringLiteral("addedWordRemove"));
    remove->setEnabled(false);
    row->addWidget(t.edit, 1);
    row->addWidget(add);
    row->addWidget(remove);
    v->addLayout(row);
    auto *files = new QHBoxLayout();
    auto *import = new QPushButton(QCoreApplication::translate("UserDictionary", "Import..."), t.page);
    import->setObjectName(excluded ? QStringLiteral("excludedWordImport") : QStringLiteral("addedWordImport"));
    auto *export_ = new QPushButton(QCoreApplication::translate("UserDictionary", "Export..."), t.page);
    export_->setObjectName(excluded ? QStringLiteral("excludedWordExport") : QStringLiteral("addedWordExport"));
    files->addWidget(import);
    files->addWidget(export_);
    files->addStretch(1);
    v->addLayout(files);
    connect(t.find, &QLineEdit::textChanged, this, [this, which] { fill(which); });
    connect(add, &QPushButton::clicked, this, [this, which] { addTyped(which); });
    connect(t.edit, &QLineEdit::returnPressed, add, &QPushButton::click);
    connect(remove, &QPushButton::clicked, this, [this, which] { removeSelected(which); });
    connect(t.list, &QListWidget::itemSelectionChanged, remove, [list = t.list, remove] { remove->setEnabled(!list->selectedItems().isEmpty()); });
    connect(import, &QPushButton::clicked, this, [this, which] { chooseImport(which); });
    connect(export_, &QPushButton::clicked, this, [this, which] { chooseExport(which); });
    return t;
}

// The list as shown: the words that have what is in the Find box in them, in order.
void UserDictionaryPage::fill(int which)
{
    const QString look = m_tabs[which].find->text().trimmed();
    QStringList shown;
    for (const QString &w : m_words[which])
        if (look.isEmpty() || w.contains(look, Qt::CaseInsensitive)) shown << w;
    shown.sort();
    QListWidget *list = m_tabs[which].list;
    list->setUpdatesEnabled(false);
    list->clear();
    list->addItems(shown);
    list->setUpdatesEnabled(true);
}

bool UserDictionaryPage::addWord(bool excluded, const QString &raw)
{
    const QString word = raw.trimmed();
    if (!userWordIsFit(word)) return false;
    const int which = excluded ? 1 : 0;
    if (excluded) {
        const QString key = Speller::wordKey(word);
        m_words[1].insert(key);
        for (const QString &w : m_words[0].values())
            if (Speller::wordKey(w) == key) m_words[0].remove(w);
        fill(0);
    } else {
        m_words[0].insert(word);
        m_words[1].remove(Speller::wordKey(word));
        fill(1);
    }
    fill(which);
    return true;
}

void UserDictionaryPage::addTyped(int which)
{
    const QString word = m_tabs[which].edit->text();
    if (word.trimmed().isEmpty()) return;
    if (!addWord(which == 1, word)) {
        m_status->setText(QCoreApplication::translate("UserDictionary", "A word can't have a space or a control character in it, or be longer than %n letters.", nullptr, kMaxWordLength));
        return;
    }
    m_status->clear();
    m_tabs[which].edit->clear();
}

void UserDictionaryPage::removeSelected(int which)
{
    for (const QListWidgetItem *item : m_tabs[which].list->selectedItems()) m_words[which].remove(item->text());
    fill(which);
}

QString UserDictionaryPage::importFile(bool excluded, const QString &path)
{
    QString error;
    // (A device or a pipe is not read: it may never end, and it is not a list.)
    const QFileInfo info(path);
    WordList list;
    if (info.exists() && !info.isFile()) error = QCoreApplication::translate("UserDictionary", "It is not a regular file.");
    else list = readWordListFile(path, &error);
    if (!error.isEmpty()) {
        m_status->setText(QCoreApplication::translate("UserDictionary", "The file can't be read: %1").arg(error));
        return m_status->text();
    }
    const int which = excluded ? 1 : 0;
    // (Each word goes in with a set's insert, and the lists are drawn once at the end: a long file takes as long as it is long.)
    QSet<QString> lower;
    if (excluded) {
        for (const QString &w : list.words) m_words[1].insert(Speller::wordKey(w));
        for (const QString &w : list.words) lower.insert(Speller::wordKey(w));
        for (const QString &w : m_words[0].values())
            if (lower.contains(Speller::wordKey(w))) m_words[0].remove(w);
    } else {
        for (const QString &w : list.words) {
            m_words[0].insert(w);
            lower.insert(Speller::wordKey(w));
        }
        for (const QString &w : lower) m_words[1].remove(w);
    }
    fill(0);
    fill(1);
    QString text = QCoreApplication::translate("UserDictionary", "Words read from the file: %1.").arg(list.words.size());
    if (list.skipped > 0) text += QLatin1Char(' ') + QCoreApplication::translate("UserDictionary", "Lines left out: %1. A word can't have a space or a control character in it, or be longer than %n letters.", nullptr, kMaxWordLength).arg(list.skipped);
    if (list.truncated) text += QLatin1Char(' ') + QCoreApplication::translate("UserDictionary", "The file is longer than a word list can be, so the rest was left out.");
    m_status->setText(text);
    return text;
}

bool UserDictionaryPage::exportFile(bool excluded, const QString &path, QString *error)
{
    return writeWordListFile(path, excluded ? excludedWords() : addedWords(), error);
}

void UserDictionaryPage::chooseImport(int which)
{
    const QString path = QFileDialog::getOpenFileName(this, QCoreApplication::translate("UserDictionary", "Import a Word List"), QString(), QCoreApplication::translate("UserDictionary", "Word lists (*.txt *.dic);;All Files (*)"));
    if (!path.isEmpty()) importFile(which == 1, path);
}

void UserDictionaryPage::chooseExport(int which)
{
    const QString path = QFileDialog::getSaveFileName(this, QCoreApplication::translate("UserDictionary", "Export a Word List"), which == 1 ? QStringLiteral("excluded-words.txt") : QStringLiteral("added-words.txt"),
                                                      QCoreApplication::translate("UserDictionary", "Word lists (*.txt);;All Files (*)"));
    if (path.isEmpty()) return;
    QString error;
    if (exportFile(which == 1, path, &error)) m_status->setText(QCoreApplication::translate("UserDictionary", "Words written to the file: %1.").arg((which == 1 ? m_words[1] : m_words[0]).size()));
    else m_status->setText(QCoreApplication::translate("UserDictionary", "The file can't be written: %1").arg(error));
}

// What the person changed since the dialog opened, as changes to what the files hold now (a word that another app added meanwhile stays).
bool UserDictionaryPage::apply(QString *error)
{
    Speller::WordChanges changes;
    for (const QString &w : sorted(m_words[0]))
        if (!m_opened[0].contains(w)) changes.addCustom << w;
    for (const QString &w : sorted(m_opened[0]))
        if (!m_words[0].contains(w)) changes.removeCustom << w;
    for (const QString &w : sorted(m_words[1]))
        if (!m_opened[1].contains(w)) changes.addExcluded << w;
    for (const QString &w : sorted(m_opened[1]))
        if (!m_words[1].contains(w)) changes.removeExcluded << w;
    return Speller::get().change(changes, error);
}

bool userDictionaryDialog(QWidget *parent)
{
    QDialog d(parent);
    d.setObjectName(QStringLiteral("userDictionaryDialog"));
    d.setWindowTitle(QCoreApplication::translate("UserDictionary", "Custom Dictionary"));
    auto *v = new QVBoxLayout(&d);
    auto *page = new UserDictionaryPage(&d);
    v->addWidget(page, 1);
    auto *bb = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    d.setMinimumSize(480, 520);
    if (d.exec() != QDialog::Accepted) return false;
    QString error;
    if (!page->apply(&error)) {
        jp::plainWarning(parent, QCoreApplication::translate("UserDictionary", "Custom Dictionary"), QCoreApplication::translate("UserDictionary", "The words can't be saved.\n%1").arg(error));
        return false;
    }
    return true;
}

} // namespace jp
