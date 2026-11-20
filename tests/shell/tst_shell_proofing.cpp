// The user's dictionary (libs/shell/app/spelling, libs/page/app/userdictionarydialog): excluded words, word list files read and written safely, and the dialog that edits both lists.
#include "app/optionsdialog.h"
#include "app/settings.h"
#include "app/spelling.h"
#include "app/userdictionarydialog.h"
#include "../isolatesettings.h"
#include "../testrunner.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

using namespace jp;

namespace {

// Keeps the speller's files in a folder of the test's own, and leaves both lists empty at the end.
struct OwnDictionary {
    QTemporaryDir dir;
    QString path;
    OwnDictionary()
    {
        path = dir.filePath(QStringLiteral("words.txt"));
        Speller::setUserDictionaryPath(path);
        Speller::get().setWordLists({}, {});
    }
    ~OwnDictionary()
    {
        Speller::get().setWordLists({}, {});
        Speller::setUserDictionaryPath(QString());
    }
    QByteArray read(const QString &file) const
    {
        QFile f(file);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    }
};

const QString kEn = QStringLiteral("en-US");

} // namespace

class ShellProofingTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void anExcludedWordIsFlaggedEvenThoughTheDictionaryAcceptsItAndInAnyCase()
    {
        OwnDictionary own;
        Speller &s = Speller::get();
        QVERIFY(s.check(kEn, QStringLiteral("house")));
        QVERIFY(s.setWordLists({}, {QStringLiteral("House")}));
        QVERIFY(!s.check(kEn, QStringLiteral("house")));
        QVERIFY(!s.check(kEn, QStringLiteral("House")));
        QVERIFY(!s.check(kEn, QStringLiteral("HOUSE")));   // (the option that lets capitals go does not rescue it)
        QVERIFY(s.check(kEn, QStringLiteral("houses")));
        QCOMPARE(s.excludedWords(), QStringList{QStringLiteral("house")});
    }

    void anExcludedWordIsNeverSuggested()
    {
        OwnDictionary own;
        Speller &s = Speller::get();
        const QStringList before = s.suggest(kEn, QStringLiteral("speling"));
        QVERIFY(!before.isEmpty());
        QVERIFY(s.setWordLists({}, {before.first()}));
        const QStringList after = s.suggest(kEn, QStringLiteral("speling"));
        QVERIFY(!after.contains(before.first(), Qt::CaseInsensitive));
        QCOMPARE(after.size(), before.size() - 1);
        QVERIFY(!spellingSuggestions(QStringLiteral("speling")).contains(before.first(), Qt::CaseInsensitive));
    }

    void aWordIsInOneListAndAddingTakesItOutOfTheExcludedOnes()
    {
        OwnDictionary own;
        Speller &s = Speller::get();
        QVERIFY(s.setWordLists({QStringLiteral("zzqxa"), QStringLiteral("zzqxb")}, {QStringLiteral("zzqxb"), QStringLiteral("zzqxc")}));
        QCOMPARE(s.customWords(), QStringList{QStringLiteral("zzqxa")});   // (the excluded list wins over the words added)
        QVERIFY(!s.check(kEn, QStringLiteral("zzqxb")));
        QVERIFY(s.add(QStringLiteral("zzqxc")));
        QVERIFY(s.check(kEn, QStringLiteral("zzqxc")));
        QCOMPARE(s.excludedWords(), QStringList{QStringLiteral("zzqxb")});
        QCOMPARE(own.read(own.path + QStringLiteral(".excluded")), QByteArray("zzqxb\n"));   // (and the file says it)
    }

    void theListsAreWrittenWholeAndReadBackAndAreNotInTheDocuments()
    {
        OwnDictionary own;
        Speller &s = Speller::get();
        QVERIFY(s.setWordLists({QStringLiteral("beta"), QStringLiteral("alpha"), QStringLiteral("alpha")}, {QStringLiteral("gamma")}));
        QCOMPARE(own.read(own.path), QByteArray("beta\nalpha\n"));
        QCOMPARE(own.read(own.path + QStringLiteral(".excluded")), QByteArray("gamma\n"));
        QCOMPARE(s.customWords(true), (QStringList{QStringLiteral("alpha"), QStringLiteral("beta")}));
        QCOMPARE(s.excludedWords(), QStringList{QStringLiteral("gamma")});
        QVERIFY(QDir(own.dir.path()).entryList(QDir::Files).size() == 2);   // (no temporary file is left)
    }

    void aListThatCantBeWrittenLeavesTheFileAsItWasAndTheListsUnchanged()
    {
        OwnDictionary own;
        Speller &s = Speller::get();
        QVERIFY(s.setWordLists({QStringLiteral("keepme")}, {}));
        // (A folder where the excluded list's file goes: it can't be written.)
        QVERIFY(QFile::remove(own.path + QStringLiteral(".excluded")));
        QVERIFY(QDir().mkdir(own.path + QStringLiteral(".excluded")));
        QString error;
        QVERIFY(!s.setWordLists({QStringLiteral("other")}, {QStringLiteral("x")}, &error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(own.read(own.path), QByteArray("keepme\n"));
        QCOMPARE(s.customWords(), QStringList{QStringLiteral("keepme")});
        QVERIFY(s.excludedWords().isEmpty());
        QVERIFY(QDir().rmdir(own.path + QStringLiteral(".excluded")));
        QString failure;
        QVERIFY(!writeWordListFile(own.dir.filePath(QStringLiteral("none/list.txt")), {QStringLiteral("a")}, &failure));
        QVERIFY(!failure.isEmpty());
    }

    void aWordListIsReadByLineWithWhatIsNotAWordLeftOut()
    {
        QByteArray bytes("\xEF\xBB\xBF" "one\r\n\r\n  two \nthree\none\nwith space\nctl\x01" "x\n");
        bytes += "bad\xFF" "utf\n";
        bytes += QByteArray(kMaxWordLength, 'a') + "\n";
        bytes += QByteArray(kMaxWordLength + 1, 'b') + "\n";
        bytes += "tab\there\nlast";
        const WordList list = parseWordList(bytes);
        QCOMPARE(list.words, (QStringList{QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three"), QString(kMaxWordLength, QLatin1Char('a')), QStringLiteral("last")}));
        QCOMPARE(list.skipped, 5);   // (a space, a control character, a stray byte, one too long, a tab)
        QVERIFY(!list.truncated);
    }

    void aHugeWordListIsReadOnlyAsFarAsTheLimit()
    {
        // Half a million short words, more than fit.
        QByteArray big;
        for (int i = 0; i < 500000; ++i) big += "w" + QByteArray::number(i) + "\n";
        const WordList list = parseWordList(big);
        QVERIFY(list.truncated);
        qint64 bytes = 0;
        for (const QString &w : list.words) bytes += w.toUtf8().size() + 1;
        QVERIFY(bytes <= kMaxWordListBytes);
        QVERIFY(bytes + 8 > kMaxWordListBytes - 8);   // (as many as fit: the next word did not)
        QCOMPARE(list.words.first(), QStringLiteral("w0"));
        QCOMPARE(list.words.last(), QStringLiteral("w%1").arg(list.words.size() - 1));
        // A million blank lines have no words, and a file of long lines is left out line by line.
        const WordList blank = parseWordList(QByteArray(1000000, '\n'));
        QVERIFY(blank.words.isEmpty());
        QCOMPARE(blank.skipped, 0);
        QByteArray longLines;
        for (int i = 0; i < 5000; ++i) longLines += QByteArray(kMaxWordLength + 1, 'x') + "\n";
        const WordList lines = parseWordList(longLines);
        QVERIFY(lines.words.isEmpty());
        QCOMPARE(lines.skipped, 5000);
        // A file over the limit is read only as far as it.
        QTemporaryDir dir;
        QFile f(dir.filePath(QStringLiteral("huge.txt")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        for (int i = 0; i < 3; ++i) f.write(big);
        f.close();
        QVERIFY(f.size() > 3 * kMaxWordListBytes);
        const WordList huge = readWordListFile(f.fileName());
        QVERIFY(huge.truncated);
        QCOMPARE(huge.words, list.words);
    }

    void aWordTypedOrReadMustBeFit()
    {
        QVERIFY(userWordIsFit(QStringLiteral("don't")));
        QVERIFY(!userWordIsFit(QString()));
        QVERIFY(!userWordIsFit(QStringLiteral("two words")));
        QVERIFY(!userWordIsFit(QStringLiteral("line\nbreak")));
        QVERIFY(!userWordIsFit(QString(kMaxWordLength + 1, QLatin1Char('a'))));
    }

    void theDialogsPageAddsRemovesFindsAndMovesWordsBetweenTheTwoLists()
    {
        OwnDictionary own;
        QVERIFY(Speller::get().setWordLists({QStringLiteral("alpha"), QStringLiteral("beta")}, {QStringLiteral("gamma")}));
        UserDictionaryPage page;
        auto *added = page.findChild<QListWidget *>(QStringLiteral("addedWords"));
        auto *excluded = page.findChild<QListWidget *>(QStringLiteral("excludedWords"));
        QVERIFY(added && excluded);
        QCOMPARE(added->count(), 2);
        QCOMPARE(excluded->count(), 1);
        // Add by typing.
        page.findChild<QLineEdit *>(QStringLiteral("addedWordEdit"))->setText(QStringLiteral("delta"));
        page.findChild<QPushButton *>(QStringLiteral("addedWordAdd"))->click();
        QCOMPARE(added->count(), 3);
        QVERIFY(page.findChild<QLineEdit *>(QStringLiteral("addedWordEdit"))->text().isEmpty());
        // A word that can't be one is not added.
        page.findChild<QLineEdit *>(QStringLiteral("addedWordEdit"))->setText(QStringLiteral("two words"));
        page.findChild<QPushButton *>(QStringLiteral("addedWordAdd"))->click();
        QCOMPARE(added->count(), 3);
        // Find.
        page.findChild<QLineEdit *>(QStringLiteral("addedWordsFind"))->setText(QStringLiteral("ALP"));
        QCOMPARE(added->count(), 1);
        QCOMPARE(added->item(0)->text(), QStringLiteral("alpha"));
        page.findChild<QLineEdit *>(QStringLiteral("addedWordsFind"))->clear();
        // Remove the selected.
        added->item(0)->setSelected(true);
        QVERIFY(page.findChild<QPushButton *>(QStringLiteral("addedWordRemove"))->isEnabled());
        page.findChild<QPushButton *>(QStringLiteral("addedWordRemove"))->click();
        QCOMPARE(page.addedWords(), (QStringList{QStringLiteral("beta"), QStringLiteral("delta")}));
        // Excluding a word takes it out of the added words, and adding it takes it out of the excluded ones.
        QVERIFY(page.addWord(true, QStringLiteral("Beta")));
        QCOMPARE(page.addedWords(), QStringList{QStringLiteral("delta")});
        QCOMPARE(page.excludedWords(), (QStringList{QStringLiteral("beta"), QStringLiteral("gamma")}));
        QVERIFY(page.addWord(false, QStringLiteral("gamma")));
        QCOMPARE(page.excludedWords(), QStringList{QStringLiteral("beta")});
        // Nothing is kept until apply.
        QCOMPARE(Speller::get().customWords(), (QStringList{QStringLiteral("alpha"), QStringLiteral("beta")}));
        QVERIFY(page.apply());
        QCOMPARE(Speller::get().customWords(), (QStringList{QStringLiteral("delta"), QStringLiteral("gamma")}));
        QCOMPARE(Speller::get().excludedWords(), QStringList{QStringLiteral("beta")});
    }

    void theDialogsPageImportsAndExportsAPlainWordList()
    {
        OwnDictionary own;
        const QString file = own.dir.filePath(QStringLiteral("in.txt"));
        {
            QFile f(file);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("\xEF\xBB\xBF" "caf\xC3\xA9\r\nnaïve\nbad word\nzzqxa\n");
        }
        UserDictionaryPage page;
        const QString said = page.importFile(false, file);
        QVERIFY(!said.isEmpty());
        QCOMPARE(page.addedWords(), (QStringList{QStringLiteral("café"), QStringLiteral("naïve"), QStringLiteral("zzqxa")}));
        const QString out = own.dir.filePath(QStringLiteral("out.txt"));
        QVERIFY(page.exportFile(false, out));
        QCOMPARE(own.read(out), QStringLiteral("café\nnaïve\nzzqxa\n").toUtf8());
        // Imported into the excluded list, they leave the added one.
        page.importFile(true, out);
        QVERIFY(page.addedWords().isEmpty());
        QCOMPARE(page.excludedWords().size(), 3);
        // A file that isn't there says so and changes nothing.
        page.importFile(false, own.dir.filePath(QStringLiteral("none.txt")));
        QVERIFY(page.addedWords().isEmpty());
    }

    void theCustomDictionaryIsReachedFromTheProofingOptionsAndKeptWhenAccepted()
    {
        OwnDictionary own;
        bool found = false, kept = false;
        QTimer poll;
        poll.setInterval(20);
        QObject::connect(&poll, &QTimer::timeout, &poll, [&] {
            for (QWidget *w : QApplication::topLevelWidgets()) {
                auto *dlg = qobject_cast<QDialog *>(w);
                if (!dlg || !dlg->isVisible()) continue;
                if (dlg->objectName() == QLatin1String("userDictionaryDialog")) {
                    auto *page = dlg->findChild<UserDictionaryPage *>();
                    kept = page && page->addWord(false, QStringLiteral("zzqxopt"));
                    dlg->accept();
                } else if (dlg->objectName() == QLatin1String("optionsDialog")) {
                    auto *button = dlg->findChild<QPushButton *>(QStringLiteral("customDictionaryButton"));
                    if (!found && button) {
                        found = true;
                        QTimer::singleShot(0, button, &QPushButton::click);
                    } else if (found && kept) {
                        dlg->reject();
                    }
                }
            }
        });
        poll.start();
        optionsDialog(nullptr, nullptr, {}, false);
        QVERIFY(found && kept);
        QCOMPARE(Speller::get().customWords(), QStringList{QStringLiteral("zzqxopt")});
    }
};

JP_TEST_CLASS(ShellProofingTests)
#include "tst_shell_proofing.moc"
