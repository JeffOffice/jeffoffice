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
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

using namespace jp;

namespace {

bool setLists(Speller &s, const QStringList &custom, const QStringList &excluded, QString *error);

// Keeps the speller's files in a folder of the test's own, and leaves both lists empty at the end.
struct OwnDictionary {
    QTemporaryDir dir;
    QString path;
    OwnDictionary()
    {
        path = dir.filePath(QStringLiteral("words.txt"));
        Speller::setUserDictionaryPath(path);
        setLists(Speller::get(), {}, {}, nullptr);
    }
    ~OwnDictionary()
    {
        setLists(Speller::get(), {}, {}, nullptr);
        Speller::setUserDictionaryPath(QString());
    }
    QByteArray read(const QString &file) const { return slurp(file); }
    static QByteArray slurp(const QString &file)
    {
        QFile f(file);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    }
};

const QString kEn = QStringLiteral("en-US");

// Makes the two lists exactly these, as changes to what the files hold now (a word in both lists is excluded).
bool setLists(Speller &s, const QStringList &custom, const QStringList &excluded, QString *error)
{
    Speller::WordChanges c;
    QSet<QString> keys;
    for (const QString &w : excluded) keys.insert(Speller::wordKey(w));
    const QStringList haveCustom = s.customWords(true), haveExcluded = s.excludedWords();
    for (const QString &w : haveCustom)
        if (!custom.contains(w)) c.removeCustom << w;
    for (const QString &w : haveExcluded)
        if (!keys.contains(w)) c.removeExcluded << w;
    for (const QString &w : custom)
        if (!keys.contains(Speller::wordKey(w)) && !haveCustom.contains(w)) c.addCustom << w;
    for (const QString &w : keys)
        if (!haveExcluded.contains(w)) c.addExcluded << w;
    return s.change(c, error);
}

} // namespace

class ShellProofingTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void anExcludedWordIsFlaggedEvenThoughTheDictionaryAcceptsItAndInAnyCase()
    {
        OwnDictionary own;
        Speller &s = Speller::get();
        QVERIFY(s.check(kEn, QStringLiteral("house")));
        QVERIFY(setLists(s, {}, {QStringLiteral("House")}, nullptr));
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
        QVERIFY(setLists(s, {}, {before.first()}, nullptr));
        const QStringList after = s.suggest(kEn, QStringLiteral("speling"));
        QVERIFY(!after.contains(before.first(), Qt::CaseInsensitive));
        QCOMPARE(after.size(), before.size() - 1);
        QVERIFY(!spellingSuggestions(QStringLiteral("speling")).contains(before.first(), Qt::CaseInsensitive));
    }

    void aWordIsInOneListAndAddingTakesItOutOfTheExcludedOnes()
    {
        OwnDictionary own;
        Speller &s = Speller::get();
        QVERIFY(setLists(s, {QStringLiteral("zzqxa"), QStringLiteral("zzqxb")}, {QStringLiteral("zzqxb"), QStringLiteral("zzqxc")}, nullptr));
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
        QVERIFY(setLists(s, {QStringLiteral("beta"), QStringLiteral("alpha"), QStringLiteral("alpha")}, {QStringLiteral("gamma")}, nullptr));
        QCOMPARE(own.read(own.path), QByteArray("beta\nalpha\n"));
        QCOMPARE(own.read(own.path + QStringLiteral(".excluded")), QByteArray("gamma\n"));
        QCOMPARE(s.customWords(true), (QStringList{QStringLiteral("alpha"), QStringLiteral("beta")}));
        QCOMPARE(s.excludedWords(), QStringList{QStringLiteral("gamma")});
        QVERIFY(QDir(own.dir.path()).entryList(QDir::Files).size() == 2);   // (no temporary file is left)
    }

    void aWordListWrittenToAFolderThatIsNotThereSaysSo()
    {
        OwnDictionary own;
        QString failure;
        QVERIFY(!writeWordListFile(own.dir.filePath(QStringLiteral("none/list.txt")), {QStringLiteral("a")}, &failure));
        QVERIFY(!failure.isEmpty());
    }

    // The files are the person's, shared by four apps that may all be open: every change is made to what the files hold at that moment.
    void twoSpellersOnOneFolderLoseNothingFromInterleavedAddsExcludesAndRemoves()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("words.txt"));
        Speller a(path), b(path);   // (two apps, each started before the other's change)
        QVERIFY(a.add(QStringLiteral("zzone")));
        QVERIFY(b.add(QStringLiteral("zztwo")));
        QVERIFY(a.change({{QStringLiteral("zzthree")}, {}, {QStringLiteral("zzexa")}, {}}));
        QVERIFY(b.change({{}, {QStringLiteral("zzone")}, {QStringLiteral("zzexb")}, {}}));
        QVERIFY(a.change({{}, {}, {}, {QStringLiteral("zzexa")}}));
        QVERIFY(b.add(QStringLiteral("zzexb")));   // (adding takes it off the excluded list, as the other app left it)
        QCOMPARE(readWordListFile(path).words, (QStringList{QStringLiteral("zztwo"), QStringLiteral("zzthree"), QStringLiteral("zzexb")}));
        QCOMPARE(readWordListFile(path + QStringLiteral(".excluded")).words, QStringList());
        QCOMPARE(a.customWords(true), (QStringList{QStringLiteral("zzexb"), QStringLiteral("zzthree"), QStringLiteral("zztwo")}));
        // (Each file was written whole once per change, with no other file left beside them.)
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files).size(), 2);
    }

    void aDialogApplyAfterTheFilesChangedOnDiskKeepsTheOtherChange()
    {
        OwnDictionary own;
        QVERIFY(setLists(Speller::get(), {QStringLiteral("zzold")}, {QStringLiteral("zzexold")}, nullptr));
        UserDictionaryPage page;   // (opened with these)
        Speller other(own.path);   // (another app, which changes the files while the dialog is open)
        QVERIFY(other.change({{QStringLiteral("zzother")}, {}, {QStringLiteral("zzexother")}, {}}));
        QVERIFY(page.addWord(false, QStringLiteral("zzmine")));
        QVERIFY(page.addWord(true, QStringLiteral("zzexmine")));
        QVERIFY(page.addWord(true, QStringLiteral("zzold")));   // (moved to the excluded list)
        QVERIFY(page.apply());
        QCOMPARE(readWordListFile(own.path).words, (QStringList{QStringLiteral("zzother"), QStringLiteral("zzmine")}));
        QCOMPARE(Speller::get().excludedWords(), (QStringList{QStringLiteral("zzexmine"), QStringLiteral("zzexold"), QStringLiteral("zzexother"), QStringLiteral("zzold")}));
        QCOMPARE(Speller::get().customWords(), (QStringList{QStringLiteral("zzmine"), QStringLiteral("zzother")}));
    }

    void aSpellerReadsAnotherAppsChangeWithoutARestartAndTellsThePages()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("words.txt"));
        Speller a(path), b(path);
        QSignalSpy told(b.watch(), &SpellingWatch::changed);
        const int revision = b.revision();
        QVERIFY(b.check(kEn, QStringLiteral("house")));
        QVERIFY(a.change({{}, {}, {QStringLiteral("house")}, {}}));
        QTRY_VERIFY(b.isExcluded(QStringLiteral("house")));
        QVERIFY(!b.check(kEn, QStringLiteral("house")));
        QVERIFY(b.revision() > revision);
        QVERIFY(told.count() >= 1);
        QVERIFY(a.change({{QStringLiteral("zzlater")}, {}, {}, {}}));
        QTRY_VERIFY(b.check(kEn, QStringLiteral("zzlater")));
    }

    void aChangeThatCantBeWrittenLeavesTheFilesAndTheListsAsTheyWere()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("words.txt"));
        Speller s(path);
        QVERIFY(s.add(QStringLiteral("zzkeep")));
        const QByteArray before = OwnDictionary::slurp(path);
        QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));   // (a folder nothing can be written to)
        struct Restore {
            QString dir;
            ~Restore() { QFile::setPermissions(dir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner); }
        } restore{dir.path()};
        if (QFileInfo(dir.path()).isWritable()) QSKIP("This account can write to a folder that has no write permission.");
        QString error;
        QVERIFY(!s.add(QStringLiteral("zznew"), &error));
        QVERIFY(!error.isEmpty());
        QVERIFY(!s.check(kEn, QStringLiteral("zznew")));   // (a word that was not saved is not accepted)
        QVERIFY(!s.change({{}, {QStringLiteral("zzkeep")}, {QStringLiteral("zzex")}, {}}, &error));
        QVERIFY(s.check(kEn, QStringLiteral("zzkeep")));   // (nor is one that could not be taken out gone)
        QCOMPARE(OwnDictionary::slurp(path), before);
        QCOMPARE(s.customWords(), QStringList{QStringLiteral("zzkeep")});
        QVERIFY(s.excludedWords().isEmpty());
        QVERIFY(!s.check(kEn, QStringLiteral("zznew")));
        QVERIFY(s.check(kEn, QStringLiteral("zzkeep")));
    }

    void aReadOnlyFileIsRefusedTheSameWayByAddAndByTheDialogAndIsNotReplaced()
    {
        OwnDictionary own;
        {
            QFile f(own.path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("zzfixed\n");
        }
        Speller::get().reload();
        QVERIFY(QFile::setPermissions(own.path, QFileDevice::ReadOwner));
        struct Restore {
            QString file;
            ~Restore() { QFile::setPermissions(file, QFileDevice::ReadOwner | QFileDevice::WriteOwner); }
        } restore{own.path};
        if (QFileInfo(own.path).isWritable()) QSKIP("This account can write to a file that is read-only.");
        QString viaAdd, viaDialog;
        QVERIFY(!Speller::get().add(QStringLiteral("zznew"), &viaAdd));
        UserDictionaryPage page;
        QVERIFY(page.addWord(false, QStringLiteral("zzdialog")));
        QVERIFY(!page.apply(&viaDialog));
        QVERIFY(!viaAdd.isEmpty());
        QCOMPARE(viaDialog, viaAdd);
        QCOMPARE(viaAdd, QCoreApplication::translate("Proofing", "The file is read-only."));   // (said by the speller, not left to whatever the system says)
        QCOMPARE(OwnDictionary::slurp(own.path), QByteArray("zzfixed\n"));   // (not replaced by a rename)
        QVERIFY(!Speller::get().check(kEn, QStringLiteral("zznew")));
        QVERIFY(!Speller::get().check(kEn, QStringLiteral("zzdialog")));
    }

    void linesAnOlderReleaseKeptAreKeptInTheFileWhateverIsChanged()
    {
        OwnDictionary own;
        const QByteArray old = "zzfirst\nNew York\nzzsecond\nbad\xFF" "utf\n";
        {
            QFile f(own.path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(old);
        }
        Speller &s = Speller::get();
        s.reload();
        QCOMPARE(s.customWords(), (QStringList{QStringLiteral("zzfirst"), QStringLiteral("zzsecond")}));
        QVERIFY(s.add(QStringLiteral("zzthird")));
        QCOMPARE(OwnDictionary::slurp(own.path), QByteArray("zzfirst\nzzsecond\nzzthird\nNew York\nbad\xFF" "utf\n"));
        UserDictionaryPage page;   // (it does not show them, and does not drop them)
        QCOMPARE(page.addedWords(), (QStringList{QStringLiteral("zzfirst"), QStringLiteral("zzsecond"), QStringLiteral("zzthird")}));
        QVERIFY(page.addWord(true, QStringLiteral("zzsecond")));
        QVERIFY(page.apply());
        QCOMPARE(OwnDictionary::slurp(own.path), QByteArray("zzfirst\nzzthird\nNew York\nbad\xFF" "utf\n"));
        QVERIFY(s.change({{}, {QStringLiteral("zzfirst")}, {}, {}}));
        QVERIFY(OwnDictionary::slurp(own.path).contains("New York\nbad\xFF" "utf\n"));
    }

    void anExcludedWordIsComparedAsTheCheckerComparesWordsAndIgnoreAllBeatsIt()
    {
        OwnDictionary own;
        Speller &s = Speller::get();
        QVERIFY(setLists(s, {}, {QStringLiteral("Don\u2019t")}, nullptr));
        QCOMPARE(s.excludedWords(), QStringList{QStringLiteral("don't")});
        QVERIFY(!s.check(kEn, QStringLiteral("don't")));
        QVERIFY(!s.check(kEn, QStringLiteral("Don\u2019t")));
        QVERIFY(!s.check(kEn, QStringLiteral("DON'T")));
        QVERIFY(s.isExcluded(QStringLiteral("don\u2019t")));
        // Excluding it again in the other form is the same word; adding it in either form takes it off the list.
        QVERIFY(s.change({{}, {}, {QStringLiteral("don\u2019t")}, {}}));
        QCOMPARE(s.excludedWords(), QStringList{QStringLiteral("don't")});
        s.ignore(QStringLiteral("Don\u2019t"));   // (Ignore All, for the session: the ignore list is looked at before the exclusion)
        QVERIFY(s.check(kEn, QStringLiteral("don't")));
        QVERIFY(s.check(kEn, QStringLiteral("DON\u2019T")));
        QVERIFY(s.add(QStringLiteral("don\u2019t")));
        QVERIFY(s.excludedWords().isEmpty());
    }

    void aWordListMayEndItsLinesWithACarriageReturnAlone()
    {
        const WordList list = parseWordList("one\rtwo\r\nthree\n\rfour\r");
        QCOMPARE(list.words, (QStringList{QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three"), QStringLiteral("four")}));
        QCOMPARE(list.skipped, 0);
    }

    void anImportRefusesWhatIsNotARegularFile()
    {
        OwnDictionary own;
        UserDictionaryPage page;
        const QString refused = QCoreApplication::translate("UserDictionary", "The file can't be read: %1").arg(QCoreApplication::translate("UserDictionary", "It is not a regular file."));
        QCOMPARE(page.importFile(false, own.dir.path()), refused);   // (a folder)
        const QString zero = QStringLiteral("/dev/zero");   // (a device that never ends: read, it would give a megabyte of nothing)
        if (QFileInfo::exists(zero)) QCOMPARE(page.importFile(false, zero), refused);
        QVERIFY(page.addedWords().isEmpty());
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
        QVERIFY(setLists(Speller::get(), {QStringLiteral("alpha"), QStringLiteral("beta")}, {QStringLiteral("gamma")}, nullptr));
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
