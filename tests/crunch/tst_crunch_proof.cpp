// Tests of what the programs of the suite share and JeffCrunch's window uses: automatic recovery (a copy of unsaved work at the Options interval, offered again after a crash, gone once
// saved or closed), Spelling (F7 over the text cells, and as you type in the cell editor), and AutoCorrect as you type in the cell editor (and web addresses made links).
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "app/spellingdialog.h"
#include "app/spelling.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/spellwalk.h"
#include "sheet/entry.h"
#include "text/autocorrect.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "../localepin.h"
#include "../settingguard.h"
#include "../sheet/sheethelpers.h"
#include "io/zip.h"
#include "office/package.h"

#include <QAction>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QSettings>
#include <QSemaphore>
#include <QToolButton>
#include "app/ribbon.h"
#include <QSignalSpy>
#include <QThread>
#include <thread>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// The files with the ending the recovery copies have, in every folder under the root.
QStringList copiesUnder(const QString &root)
{
    QStringList found;
    QDirIterator it(root, {QStringLiteral("*.xlsx")}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) found << it.next();
    return found;
}

bool saveAndWait(CrunchWindow &w, const QString &path)
{
    QSignalSpy saved(&w, &CrunchWindow::saved);
    if (!w.saveAs(path)) return false;
    return (saved.count() > 0 || saved.wait(20000)) && saved.last().value(1).toBool();
}

// Makes the copy now and waits until it is on disk.
bool recoverNow(CrunchWindow &w)
{
    QSignalSpy written(&w, &CrunchWindow::recoveryWritten);
    w.autoRecover();
    return written.count() > 0 || written.wait(20000);
}

const jp::office::sheet::Cell *at_(const jp::office::sheet::Sheet &s, const char *ref) { return at(s, QString::fromLatin1(ref)); }

// A thread that holds at a point until the test lets it go.
struct Pause {
    QSemaphore entered, proceed;
    void hold()
    {
        entered.release();
        proceed.tryAcquire(1, 30000);
    }
};

} // namespace

class CrunchProofTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        static QTemporaryDir words;
        Speller::setUserDictionaryPath(words.filePath(QStringLiteral("words.txt")));
    }
    void init()
    {
        Settings &st = Settings::get();
        for (const char *key : {"proof/autocorrect", "proof/smartQuotes", "proof/autoformat", "proof/autolink", "proof/ignoreUpper", "proof/ignoreNumbers"}) st.setValue(QString::fromLatin1(key), true);
    }

    // ---- automatic recovery

    // A changed workbook keeps a copy in the file form it is saved in, at the interval Options sets; one nobody changed keeps none.
    void recoveryCopyIsWrittenAfterAChangeAndAtTheInterval()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            Settings::get().setValue(QStringLiteral("save/autoRecoverMinutes"), 7);
            EditWindow e;
            QVERIFY(e.open(editBook()));
            QCOMPARE(e.win.w.autoRecoverInterval(), 7 * 60 * 1000);
            e.win.w.autoRecover();   // nothing changed: nothing to keep
            QVERIFY(e.win.w.recoveryCopy().isEmpty());
            QVERIFY(copiesUnder(root.path()).isEmpty());
            e.select("E5");
            e.enter("kept");
            QVERIFY(e.settle());
            QVERIFY(recoverNow(e.win.w));
            const QString copy = e.win.w.recoveryCopy();
            QVERIFY(copy.endsWith(QLatin1String(".xlsx")));
            QVERIFY(QFileInfo::exists(copy));
            QVERIFY(copy.startsWith(root.path()));
            // the copy is the workbook as it is now, as a workbook
            Window again;
            const QString second = e.folder.dir.filePath(QStringLiteral("copy-of-copy.xlsx"));
            QVERIFY(QFile::copy(copy, second));
            QVERIFY(again.open(second));
            quint32 r = 0, c = 0;
            parseCellRef(QStringLiteral("E5"), &r, &c);
            QCOMPARE(again.w.session()->valueAt(0, r, c).text(), QStringLiteral("kept"));
            // the interval follows Options
            Settings::get().setValue(QStringLiteral("save/autoRecoverMinutes"), 3);
            e.win.w.autoRecover();
            QCOMPARE(e.win.w.autoRecoverInterval(), 3 * 60 * 1000);
        }
        Settings::get().setValue(QStringLiteral("save/autoRecoverMinutes"), 10);
        recovery::setRoot(QString());
    }

    // After a crash (no clean close: the run's folder is left, its lock gone) the copy is offered, and opens as work that was never saved, with a copy of its own made before the old one goes.
    void aCopyACrashLeftIsOfferedAndOpensAsUnsavedWork()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            EditWindow lost;
            QVERIFY(lost.open(editBook()));
            lost.select("F6");
            lost.enter("lost work");
            QVERIFY(lost.settle());
            QVERIFY(recoverNow(lost.win.w));
            const QString copy = lost.win.w.recoveryCopy();
            QVERIFY(QFileInfo::exists(copy));
            // the crashed run: its folder, as it stood, with nobody holding its lock
            const QString crashed = QDir(root.path()).filePath(QStringLiteral("crashed-run"));
            QVERIFY(QDir().mkpath(crashed));
            QVERIFY(QFile::copy(copy, crashed + QStringLiteral("/lost.xlsx")));
            QVERIFY(QFile::copy(copy.chopped(5) + QStringLiteral(".json"), crashed + QStringLiteral("/lost.json")));
            const QVector<recovery::Recovered> found = recovery::orphans();
            QCOMPARE(found.size(), 1);
            QVERIFY(found[0].file.startsWith(crashed));
            Window host;   // a window with nothing in it takes the copy itself
            QVERIFY(host.openBlank());
            QSignalSpy loaded(&host.w, &CrunchWindow::loadFinished);
            QString err;
            CrunchWindow *opened = host.w.openRecovered(found[0], &err);
            QVERIFY2(opened, qPrintable(err));
            QCOMPARE(opened, &host.w);
            QTRY_VERIFY_WITH_TIMEOUT(loaded.count() > 0 && !host.w.recoveryCopy().isEmpty(), 20000);
            QVERIFY(host.w.filePath().isEmpty());   // never saved: Save asks where
            QVERIFY(host.w.isWindowModified());
            quint32 r = 0, c = 0;
            parseCellRef(QStringLiteral("F6"), &r, &c);
            QCOMPARE(host.w.session()->valueAt(0, r, c).text(), QStringLiteral("lost work"));
            QTRY_VERIFY_WITH_TIMEOUT(!QFileInfo::exists(found[0].file), 20000);   // the old copy goes once this window keeps its own
            QVERIFY(QFileInfo::exists(host.w.recoveryCopy()));
            lost.win.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
            host.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }

    // Saved, the work needs no copy; closed (a clean close), neither.
    void theRecoveryCopyGoesWhenSavedOrClosed()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            e.select("E5");
            e.enter("changed");
            QVERIFY(e.settle());
            QVERIFY(recoverNow(e.win.w));
            const QString copy = e.win.w.recoveryCopy();
            QVERIFY(QFileInfo::exists(copy));
            QVERIFY(saveAndWait(e.win.w, e.path));
            QVERIFY(!QFileInfo::exists(copy));
            QVERIFY(e.win.w.recoveryCopy().isEmpty());
            // changed again, then closed without saving
            e.select("E6");
            e.enter("again");
            QVERIFY(e.settle());
            QVERIFY(recoverNow(e.win.w));
            const QString second = e.win.w.recoveryCopy();
            QVERIFY(QFileInfo::exists(second));
            e.win.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
            QVERIFY(e.win.w.close());
            QVERIFY(!QFileInfo::exists(second));
        }
        recovery::setRoot(QString());
    }

    // ---- Spelling (F7)

    // The same command has the same name in all four apps: Spelling, and a quick toolbar that starts with Save, Undo, and Redo. (Find's labels are checked with the dialog, and Print's with the Print page.)
    void commandsUseTheSuitesWords()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        QCOMPARE(w.act(QStringLiteral("review.spelling"))->text(), QStringLiteral("Spelling"));
        QList<QAction *> quick;
        for (QToolButton *b : w.ribbon()->quickAccessButtons()) quick << b->defaultAction();
        QCOMPARE(quick.mid(0, 3), (QList<QAction *>{w.act(QStringLiteral("file.save")), w.act(QStringLiteral("edit.undo")), w.act(QStringLiteral("edit.redo"))}));
    }

    void theSpellingWalkFindsMisspelledTextCellsAndSkipsTheRest()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        QCOMPARE(w.act(QStringLiteral("review.spelling"))->shortcut(), QKeySequence(Qt::Key_F7));
        auto put = [&](const char *ref, const QString &text) {
            quint32 r = 0, c = 0;
            parseCellRef(QString::fromLatin1(ref), &r, &c);
            QVERIFY(w.session()->type(0, r, c, text, false).ok);
        };
        put("A5", QStringLiteral("a zzqxwrd here"));
        put("A6", QStringLiteral("=\"zzqxwrd\""));   // a formula that gives the same word
        put("A7", QStringLiteral("12345"));
        put("A8", QStringLiteral("2024-01-02"));
        put("A9", QStringLiteral("All words are fine"));
        put("B5", QStringLiteral("zzqxother"));
        QVERIFY(e.settle());
        SpellingDialog d(&w, 0, jp::office::sheet::CellRange{0, 0, 20, 10, true});
        QVERIFY(d.start(true));
        QVERIFY(d.current());
        QCOMPARE(d.current()->word, QStringLiteral("zzqxwrd"));
        QCOMPARE(d.current()->row, quint32(4));
        QCOMPARE(d.current()->col, quint32(0));
        QVERIFY(d.ignore());
        QCOMPARE(d.current()->word, QStringLiteral("zzqxother"));   // the formula, the number, and the date were passed
        QCOMPARE(d.current()->col, quint32(1));
        QVERIFY(!d.ignore());   // that was the last
        // the cells selected only
        SpellingDialog only(&w, 0, jp::office::sheet::CellRange{4, 0, 4, 0, true});
        QVERIFY(only.start(true));
        QCOMPARE(only.current()->word, QStringLiteral("zzqxwrd"));
        QVERIFY(!only.ignore());
    }

    void changeFixesTheWordAndChangeAllFixesEveryCell()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        auto put = [&](const char *ref, const QString &text) {
            quint32 r = 0, c = 0;
            parseCellRef(QString::fromLatin1(ref), &r, &c);
            QVERIFY(w.session()->type(0, r, c, text, false).ok);
        };
        auto text = [&](const char *ref) {
            quint32 r = 0, c = 0;
            parseCellRef(QString::fromLatin1(ref), &r, &c);
            return w.session()->valueAt(0, r, c).text();
        };
        put("A5", QStringLiteral("one zzqxwrd and"));
        put("A6", QStringLiteral("zzqxwrd twice zzqxwrd"));
        put("A7", QStringLiteral("zzqxwrdx stays"));   // a longer word is another word
        QVERIFY(e.settle());
        const jp::office::sheet::CellRange all{0, 0, 20, 10, true};
        {
            SpellingDialog d(&w, 0, all);
            QVERIFY(d.start(true));
            d.setReplacement(QStringLiteral("word"));
            QVERIFY(d.change());   // this one cell's word
            QCOMPARE(text("A5"), QStringLiteral("one word and"));
            QCOMPARE(text("A6"), QStringLiteral("zzqxwrd twice zzqxwrd"));
            QCOMPARE(d.current()->row, quint32(5));
            QCOMPARE(d.current()->word, QStringLiteral("zzqxwrd"));
            w.session()->undoStack()->undo();
            QCOMPARE(text("A5"), QStringLiteral("one zzqxwrd and"));
        }
        {
            SpellingDialog d(&w, 0, all);
            QVERIFY(d.start(true));
            d.setReplacement(QStringLiteral("word"));
            const int steps = w.session()->undoStack()->index();
            d.changeAll();
            QCOMPARE(text("A5"), QStringLiteral("one word and"));
            QCOMPARE(text("A6"), QStringLiteral("word twice word"));
            QCOMPARE(text("A7"), QStringLiteral("zzqxwrdx stays"));
            QCOMPARE(w.session()->undoStack()->index(), steps + 1);   // one step for every cell
            w.session()->undoStack()->undo();
            QCOMPARE(text("A6"), QStringLiteral("zzqxwrd twice zzqxwrd"));
        }
    }

    void ignoreAllAndAddToDictionaryUseTheSharedDictionaries()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        auto put = [&](const char *ref, const QString &text) {
            quint32 r = 0, c = 0;
            parseCellRef(QString::fromLatin1(ref), &r, &c);
            QVERIFY(w.session()->type(0, r, c, text, false).ok);
        };
        put("A5", QStringLiteral("zzignoreme zzaddme"));
        put("A6", QStringLiteral("zzignoreme zzaddme zzstill"));
        QVERIFY(e.settle());
        SpellingDialog d(&w, 0, jp::office::sheet::CellRange{0, 0, 20, 10, true});
        QVERIFY(d.start(true));
        QCOMPARE(d.current()->word, QStringLiteral("zzignoreme"));
        QVERIFY(d.ignoreAll());
        QCOMPARE(d.current()->word, QStringLiteral("zzaddme"));
        QVERIFY(d.addToDictionary());
        QCOMPARE(d.current()->word, QStringLiteral("zzstill"));   // the rest of both words in the next row is passed
        QVERIFY(crunch::misspelledRanges(QStringLiteral("zzaddme")).isEmpty());
    }

    void theSpellingOptionsOfProofingApply()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(!crunch::misspelledRanges(QStringLiteral("ZZQXWRD")).size());   // words in capitals are let go
        Settings::get().setValue(QStringLiteral("proof/ignoreUpper"), false);
        QCOMPARE(crunch::misspelledRanges(QStringLiteral("ZZQXWRD")).size(), 1);
        QVERIFY(!crunch::misspelledRanges(QStringLiteral("abc123")).size());   // with numbers
        Settings::get().setValue(QStringLiteral("proof/ignoreNumbers"), false);
        QCOMPARE(crunch::misspelledRanges(QStringLiteral("abc123")).size(), 1);
    }

    // ---- as you type in the cell editor

    void aMisspellingTypedInTheCellEditorIsUnderlinedAndSuggestionsAreOffered()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select("E5");
        e.type("see speling ");
        CellEditor *editor = e.win.w.cellEditor();
        QCOMPARE(editor->text(), QStringLiteral("see speling "));
        QTRY_COMPARE(editor->misspellings().size(), 1);
        QCOMPARE(editor->misspellings().first(), qMakePair(4, 11));
        QMenu menu;
        QVERIFY(editor->addSpellingSuggestions(menu, 6));
        QStringList items;
        for (QAction *a : menu.actions()) items << a->text();
        QVERIFY2(items.contains(QStringLiteral("spelling")), qPrintable(items.join(QLatin1Char('|'))));
        QVERIFY(items.contains(QStringLiteral("Ignore All")));
        QVERIFY(items.contains(QStringLiteral("Add to Dictionary")));
        QMenu right;
        QVERIFY(!editor->addSpellingSuggestions(right, 1));   // "see" is spelled right
        // a suggestion replaces the word
        for (QAction *a : menu.actions())
            if (a->text() == QLatin1String("spelling")) a->trigger();
        QCOMPARE(editor->text(), QStringLiteral("see spelling "));
        QTRY_VERIFY(editor->misspellings().isEmpty());
        e.key(Qt::Key_Escape);
        // a formula is not underlined
        e.select("E6");
        e.type("=speling ");
        QTest::qWait(300);
        QVERIFY(editor->misspellings().isEmpty());
        e.key(Qt::Key_Escape);
    }

    // ---- AutoCorrect as you type

    void autoCorrectFixesAWordInTheCellAndLeavesAFormulaAlone()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select("E5");
        e.type("see teh ");
        CellEditor *editor = e.win.w.cellEditor();
        QCOMPARE(editor->text(), QStringLiteral("see the "));
        // one Undo takes the fix back, and leaves what was typed
        pressUndo(editor);
        QCOMPARE(editor->text(), QStringLiteral("see teh "));
        e.key(Qt::Key_Escape);
        e.select("E5");
        e.enter("see teh car");
        QCOMPARE(e.shownAt("E5"), QStringLiteral("see the car"));
        // a formula is left as typed
        e.select("E6");
        e.type("=teh ");
        QCOMPARE(editor->text(), QStringLiteral("=teh "));
        e.key(Qt::Key_Escape);
        e.select("E7");
        e.type("-teh ");
        QCOMPARE(editor->text(), QStringLiteral("-teh "));
        e.key(Qt::Key_Escape);
        // smart quotes, a dash, and a fraction
        e.select("E8");
        e.type("say \"hi\" a--b add 1/2 ");
        QCOMPARE(editor->text(), QString::fromUtf8("say “hi” a—b add ½ "));
        e.key(Qt::Key_Escape);
        // the options turn it off
        Settings::get().setValue(QStringLiteral("proof/autocorrect"), false);
        e.select("E9");
        e.type("see teh ");
        QCOMPARE(editor->text(), QStringLiteral("see teh "));
        e.key(Qt::Key_Escape);
        // a number is a number
        e.select("E10");
        e.enter("1/2");
        QVERIFY(e.shownAt("E10") != QString::fromUtf8("½"));
    }

    void aWebAddressBecomesALinkOnlyWithTheBoxOn()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        quint32 r = 0, c = 0;
        e.select("E5");
        e.enter("https://example.org/page");
        parseCellRef(QStringLiteral("E5"), &r, &c);
        const auto *link = e.win.w.session()->anchors().linkAt(0, r, c);
        QVERIFY(link);
        QCOMPARE(link->target, QStringLiteral("https://example.org/page"));
        e.select("E6");
        e.enter("write to ann@example.org");
        parseCellRef(QStringLiteral("E6"), &r, &c);
        QVERIFY(!e.win.w.session()->anchors().linkAt(0, r, c));   // a sentence is not a link
        e.select("E7");
        e.enter("ann@example.org");
        parseCellRef(QStringLiteral("E7"), &r, &c);
        QVERIFY(e.win.w.session()->anchors().linkAt(0, r, c));
        QCOMPARE(e.win.w.session()->anchors().linkAt(0, r, c)->target, QStringLiteral("mailto:ann@example.org"));
        Settings::get().setValue(QStringLiteral("proof/autolink"), false);
        e.select("E8");
        e.enter("https://example.org/other");
        parseCellRef(QStringLiteral("E8"), &r, &c);
        QVERIFY(!e.win.w.session()->anchors().linkAt(0, r, c));
        Settings::get().setValue(QStringLiteral("proof/autolink"), true);
        e.select("E9");
        e.enter("=\"https://example.org\"");
        parseCellRef(QStringLiteral("E9"), &r, &c);
        QVERIFY(!e.win.w.session()->anchors().linkAt(0, r, c));
    }


    // The copy is the workbook as it was when the copy was begun, whatever the window does while the thread writes it: the thread is held (a test hook) after it has read the file, the
    // window renames a sheet, types new texts and numbers, adds and deletes sheets, and defines a name, and then the thread goes on. The copy is the workbook before all that; the next
    // copy has it.
    void aRecoveryCopyIsTheWorkbookAsItWasWhenTheCopyWasBegun()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            CrunchWindow &w = e.win.w;
            auto put = [&](const char *ref, const QString &text) {
                quint32 r = 0, c = 0;
                QVERIFY(parseCellRef(QString::fromLatin1(ref), &r, &c));
                QVERIFY(w.session()->type(0, r, c, text, false).ok);
            };
            put("F2", QStringLiteral("before the copy"));
            QVERIFY(e.settle());
            Pause pause;
            const void *writtenFrom = nullptr;
            w.setRecoveryHook([&pause, &writtenFrom](const void *book) {
                writtenFrom = book;
                pause.hold();
            });
            QSignalSpy written(&w, &CrunchWindow::recoveryWritten);
            w.autoRecover();
            QVERIFY(w.isRecovering());
            QVERIFY(pause.entered.tryAcquire(1, 20000));
            // the file is made from a workbook object of the thread's own, never the one the window edits
            QVERIFY(writtenFrom != nullptr);
            QVERIFY(writtenFrom != w.liveBookForTest());
            // ... the thread is held; the person goes on
            QVERIFY(w.session()->renameSheet(0, QStringLiteral("Renamed")).ok);
            for (int i = 0; i < 300; ++i) put("G2", QStringLiteral("later text %1").arg(i));   // (new shared strings, each)
            put("H2", QStringLiteral("5%"));
            put("H3", QStringLiteral("1/2/2024"));
            int added = -1;
            QVERIFY(w.session()->insertSheet(1, QStringLiteral("Added"), &added).ok);
            QVERIFY(w.session()->deleteSheet(0).ok);
            crunch::EditSession::NameDraft name;
            name.name = QStringLiteral("LaterName");
            name.text = QStringLiteral("=Added!$A$1");
            QVERIFY(w.session()->defineName(name).ok);
            // (a save of the new state, taken meanwhile, plans the parts of the new sheet in the workbook object: the copy that is held is not changed by it)
            QByteArray otherBytes;
            QString otherWhy;
            QVERIFY2(w.snapshotBytesForTest(&otherBytes, &otherWhy), qPrintable(otherWhy));
            pause.proceed.release();
            QVERIFY(written.count() > 0 || written.wait(60000));
            QVERIFY(written.last().first().toBool());
            const QString copy = w.recoveryCopy();
            QVERIFY(QFileInfo::exists(copy));
            QMap<QString, QByteArray> parts;
            {
                QFile f(copy);
                QVERIFY(f.open(QIODevice::ReadOnly));
                QVERIFY(readZip(f.readAll(), parts));
            }
            const QByteArray book = parts.value(QStringLiteral("xl/workbook.xml"));
            QVERIFY2(book.contains("name=\"Sheet1\""), book.constData());
            QVERIFY(!book.contains("Renamed"));
            QVERIFY(!book.contains("Added"));
            QVERIFY(!book.contains("LaterName"));
            QVERIFY(!parts.value(QStringLiteral("xl/sharedStrings.xml")).contains("later text"));
            QVERIFY(parts.value(QStringLiteral("xl/sharedStrings.xml")).contains("before the copy"));
            QVERIFY(!parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("r=\"G2\""));
            // the next copy has what the window did since
            w.setRecoveryHook({});
            QVERIFY(recoverNow(w));
            QMap<QString, QByteArray> later;
            QFile f(w.recoveryCopy());
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY(readZip(f.readAll(), later));
            QVERIFY(later.value(QStringLiteral("xl/workbook.xml")).contains("Added"));
            QVERIFY(later.value(QStringLiteral("xl/workbook.xml")).contains("LaterName"));
            QVERIFY(later.value(QStringLiteral("xl/sharedStrings.xml")).contains("later text 299"));
            w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }

    // A copy that was being made when the workbook was saved, closed, given a password, or the program quit is not kept: nothing is left in the recovery folder.
    void aCopyBeingMadeWhenTheWorkIsSavedIsNotKept()
    {
        QTemporaryDir root;
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            CrunchWindow &w = e.win.w;
            e.select("E5");
            e.enter("work");
            QVERIFY(e.settle());
            Pause pause;
            w.setRecoveryHook([&pause](const void *) { pause.hold(); });
            QSignalSpy written(&w, &CrunchWindow::recoveryWritten);
            w.autoRecover();
            QVERIFY(pause.entered.tryAcquire(1, 20000));
            QVERIFY(saveAndWait(w, e.path));
            pause.proceed.release();
            QVERIFY(written.count() > 0 || written.wait(60000));
            QVERIFY(!written.last().first().toBool());
            QVERIFY(w.recoveryCopy().isEmpty());
            QVERIFY(copiesUnder(root.path()).isEmpty());
        }
        recovery::setRoot(QString());
    }

    void aCopyBeingMadeWhenAPasswordIsSetIsNotKept()
    {
        QTemporaryDir root;
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            CrunchWindow &w = e.win.w;
            e.select("E5");
            e.enter("work");
            QVERIFY(e.settle());
            Pause pause;
            w.setRecoveryHook([&pause](const void *) { pause.hold(); });
            QSignalSpy written(&w, &CrunchWindow::recoveryWritten);
            w.autoRecover();
            QVERIFY(pause.entered.tryAcquire(1, 20000));
            w.anchorHooks().setFilePasswordAsker = [](SetFilePasswordRequest *r) {
                r->password = QStringLiteral("secret");
                return true;
            };
            w.act("protect.password")->trigger();
            pause.proceed.release();
            QVERIFY(written.count() > 0 || written.wait(60000));
            QVERIFY(!written.last().first().toBool());
            QVERIFY(copiesUnder(root.path()).isEmpty());
            // and none is made while the workbook has a password of a person's; the person is told once
            w.setRecoveryHook({});
            w.autoRecover();
            QVERIFY(!w.isRecovering());
            QVERIFY(copiesUnder(root.path()).isEmpty());
            QVERIFY2(w.statusText().contains(QLatin1String("password")), qPrintable(w.statusText()));
            w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }

    void closingOrQuittingDuringACopyStopsItAndLeavesNothing()
    {
        for (const bool quit : {false, true}) {
            QTemporaryDir root;
            recovery::setRoot(root.path());
            recovery::setCopySuffix(QStringLiteral(".xlsx"));
            {
                EditWindow e;
                QVERIFY(e.open(editBook()));
                CrunchWindow &w = e.win.w;
                e.select("E5");
                e.enter("work");
                QVERIFY(e.settle());
                Pause pause;
                w.setRecoveryHook([&pause](const void *) { pause.hold(); });
                w.autoRecover();
                QVERIFY(pause.entered.tryAcquire(1, 20000));
                w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
                // (the window waits for the thread, which is held: it is let go from another thread a little later)
                std::thread letGo([&pause] {
                    QThread::msleep(300);
                    pause.proceed.release();
                });
                if (quit) {
                    CrunchWindow::finishRecoveries();
                    recovery::endSession();
                } else {
                    QVERIFY(w.close());
                }
                letGo.join();
                QVERIFY(!w.isRecovering());
                QTest::qWait(50);   // (a finish that was on its way is dealt with)
                QVERIFY(copiesUnder(root.path()).isEmpty());
            }
            recovery::setRoot(QString());
        }
    }

    // Nothing changed since the last copy: the next tick makes none; a change makes one. A copy that can't be written is told once.
    void aTickWithNothingNewMakesNoCopyAndAFailureIsTold()
    {
        QTemporaryDir root;
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            CrunchWindow &w = e.win.w;
            e.select("E5");
            e.enter("work");
            QVERIFY(e.settle());
            QVERIFY(recoverNow(w));
            QSignalSpy written(&w, &CrunchWindow::recoveryWritten);
            w.autoRecover();
            QVERIFY(!w.isRecovering());
            QTest::qWait(30);
            QCOMPARE(written.count(), 0);
            e.select("E6");
            e.enter("more");
            QVERIFY(e.settle());
            w.autoRecover();
            QVERIFY(w.isRecovering());
            QVERIFY(written.wait(20000));
            // a folder that cannot be made into the run's: the copy fails, and the status says so
            QFile blocked(root.filePath(QStringLiteral("blocked")));
            QVERIFY(blocked.open(QIODevice::WriteOnly));
            blocked.close();
            recovery::endSession();
            recovery::setRoot(root.filePath(QStringLiteral("blocked")));
            e.select("E7");
            e.enter("again");
            QVERIFY(e.settle());
            QVERIFY(recoverNow(w));
            QVERIFY(!written.last().first().toBool());
            QVERIFY2(w.statusText().contains(QLatin1String("recovery")), qPrintable(w.statusText()));
            w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }


    // A copy made for recovery keeps what it takes out of a sheet's conditional formats to itself: the window's own map of them (told by a real save) is not written by the thread.
    void aRecoveryCopyDoesNotWriteTheWindowsConditionalFormatMap()
    {
        QTemporaryDir root;
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row><row r=\"2\"><c r=\"A2\"><v>7</v></c></row>");
            const QString cf = QStringLiteral("<conditionalFormatting sqref=\"A1:A2\"><cfRule type=\"cellIs\" operator=\"greaterThan\" priority=\"1\"><formula>6</formula></cfRule></conditionalFormatting>");
            EditWindow e;
            QVERIFY(e.open(buildXlsx({worksheet(rows, cf)})));
            CrunchWindow &w = e.win.w;
            QVERIFY(w.session()->clearCondRules(0, jp::office::sheet::CellRange{0, 0, 1, 0, true}).ok);
            QVERIFY(recoverNow(w));
            QCOMPARE(w.session()->condRawKept(0), qint64(0));
            // (the copy took the rule out)
            QMap<QString, QByteArray> parts;
            QFile f(w.recoveryCopy());
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY(readZip(f.readAll(), parts));
            QVERIFY(!parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("cfRule"));
            // and a real save tells the session, as it always did
            QVERIFY(saveAndWait(w, e.path));
            w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }

    // A workbook read from a text file is copied for the thread: the copy is the workbook as it was when the copy was begun.
    void aTextFileCopyIsTheWorkbookAsItWasWhenTheCopyWasBegun()
    {
        QTemporaryDir root;
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            Folder folder;
            Window win;
            QVERIFY(win.open(folder.write(QStringLiteral("values.csv"), "a,b\n1,2\n3,4\n")));
            CrunchWindow &w = win.w;
            QVERIFY(w.session()->type(0, 0, 3, QStringLiteral("before"), false).ok);
            Pause pause;
            w.setRecoveryHook([&pause](const void *) { pause.hold(); });
            QSignalSpy written(&w, &CrunchWindow::recoveryWritten);
            w.autoRecover();
            QVERIFY(w.isRecovering());
            QVERIFY(pause.entered.tryAcquire(1, 20000));
            QVERIFY(w.session()->renameSheet(0, QStringLiteral("Renamed")).ok);
            for (int i = 0; i < 100; ++i) QVERIFY(w.session()->type(0, 5, 5, QStringLiteral("later %1").arg(i), false).ok);
            pause.proceed.release();
            QVERIFY(written.count() > 0 || written.wait(60000));
            QVERIFY(written.last().first().toBool());
            QMap<QString, QByteArray> parts;
            QFile f(w.recoveryCopy());
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY(readZip(f.readAll(), parts));
            QVERIFY(!parts.value(QStringLiteral("xl/workbook.xml")).contains("Renamed"));
            QVERIFY(parts.value(QStringLiteral("xl/sharedStrings.xml")).contains("before"));
            QVERIFY(!parts.value(QStringLiteral("xl/sharedStrings.xml")).contains("later"));
            w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }

    // A new blank workbook, and a workbook read from a text file (which is written on the window's thread), are recovered too.
    void aBlankWorkbookAndATextFileAreRecovered()
    {
        QTemporaryDir root;
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            Window blank;
            QVERIFY(blank.openBlank());
            QVERIFY(blank.w.session()->type(0, 0, 0, QStringLiteral("typed in a new book"), false).ok);
            QVERIFY(recoverNow(blank.w));
            Folder folder;
            Window again;
            const QString copy = folder.dir.filePath(QStringLiteral("blank-copy.xlsx"));
            QVERIFY(QFile::copy(blank.w.recoveryCopy(), copy));
            QVERIFY(again.open(copy));
            QCOMPARE(again.w.session()->valueAt(0, 0, 0).text(), QStringLiteral("typed in a new book"));
            blank.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
            Window csv;
            const QString csvPath = folder.write(QStringLiteral("values.csv"), "a,b\n1,2\n3,4\n");
            QVERIFY(csv.open(csvPath));
            QVERIFY(csv.w.session()->type(0, 0, 3, QStringLiteral("edited"), false).ok);
            QVERIFY(recoverNow(csv.w));
            Window fromCsv;
            const QString csvCopy = folder.dir.filePath(QStringLiteral("csv-copy.xlsx"));
            QVERIFY(QFile::copy(csv.w.recoveryCopy(), csvCopy));
            QVERIFY(fromCsv.open(csvCopy));
            QCOMPARE(fromCsv.w.session()->valueAt(0, 0, 3).text(), QStringLiteral("edited"));
            QCOMPARE(fromCsv.w.session()->valueAt(0, 1, 0).number(), 1.0);
            csv.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }

    // ---- Spelling keeps the formatting of the cell

    void changingAWordKeepsTheOtherRunsOfARichCell()
    {
        const QByteArray strings = "<?xml version=\"1.0\"?><sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"1\" uniqueCount=\"1\"><si>"
                                   "<r><rPr><b/><sz val=\"12\"/><rFont val=\"Arial\"/></rPr><t xml:space=\"preserve\">Big </t></r>"
                                   "<r><rPr><sz val=\"12\"/><color rgb=\"FFFF0000\"/><rFont val=\"Arial\"/></rPr><t>zzqxwrd</t></r>"
                                   "<r><t xml:space=\"preserve\"> plain here</t></r></si></sst>";
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"s\"><v>0</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, strings)));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const auto before = w.session()->richOf(0, 0, 0);
        QVERIFY(before.has_value());
        QCOMPARE(int(before->runs.size()), 3);
        SpellingDialog d(&w, 0, jp::office::sheet::CellRange{0, 0, 5, 5, true});
        QVERIFY(d.start(true));
        QCOMPARE(d.current()->word, QStringLiteral("zzqxwrd"));
        d.setReplacement(QStringLiteral("wonder"));
        d.change();
        const auto after = w.session()->richOf(0, 0, 0);
        QVERIFY(after.has_value());
        QCOMPARE(after->text, QStringLiteral("Big wonder plain here"));
        QCOMPARE(int(after->runs.size()), 3);
        QCOMPARE(int(after->runs[0].start), 0);
        QCOMPARE(int(after->runs[1].start), 4);
        QCOMPARE(int(after->runs[2].start), 10);   // (the plain run moved on by the word's new length)
        QVERIFY(after->fonts[int(after->runs[0].font)].bold);
        QCOMPARE(after->fonts[int(after->runs[1].font)].color.rgb, 0xFF0000u);
        QVERIFY(!after->fonts[int(after->runs[1].font)].bold);
        QCOMPARE(after->runs[2].font, jp::office::sheet::RichRun::kCellFont);
        w.session()->undoStack()->undo();
        QCOMPARE(w.session()->richOf(0, 0, 0)->text, before->text);
        QCOMPARE(int(w.session()->richOf(0, 0, 0)->runs.size()), 3);
    }



    // Options > Proofing > Check spelling as you type: off, the cell editor computes and paints nothing and offers nothing on right-click, at once in an editor that is open; F7 still works; on, it is back.
    void spellingAsYouTypeCanBeTurnedOff()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        QVERIFY(CellEditor::spellingOn());   // (on unless turned off)
        e.select("E5");
        e.type("see speling here ");
        CellEditor *editor = w.cellEditor();
        QTRY_COMPARE(editor->misspellings().size(), 1);
        w.setSpellAsYouType(false);
        QVERIFY(editor->misspellings().isEmpty());   // (at once)
        const int checked = editor->checkCount();
        QTest::keyClicks(editor, "and mistaek ");
        QTest::qWait(300);
        QVERIFY(editor->misspellings().isEmpty());
        QCOMPARE(editor->checkCount(), checked);   // (nothing looked at, at a key or on the timer)
        QMenu menu;
        QVERIFY(!editor->addSpellingSuggestions(menu, 6));
        // F7 is asked for, and goes on working
        quint32 r = 0, c = 0;
        parseCellRef(QStringLiteral("A5"), &r, &c);
        QVERIFY(w.session()->type(0, r, c, QStringLiteral("a zzqxwrd"), false).ok);
        SpellingDialog d(&w, 0, jp::office::sheet::CellRange{0, 0, 20, 10, true});
        QVERIFY(d.start(true));
        QCOMPARE(d.current()->word, QStringLiteral("zzqxwrd"));
        w.setSpellAsYouType(true);
        QVERIFY(!editor->misspellings().isEmpty());   // (back at once)
        QMenu again;
        QVERIFY(editor->addSpellingSuggestions(again, 6));
        e.key(Qt::Key_Escape);
    }

    // The one shared choice: Options has the box, off there stops the checking in an editor that is open, and the old key of JeffCrunch (and of JeffWrite) moves over once.
    void spellingAsYouTypeIsOneChoiceInOptions()
    {
        const jptest::SettingGuard keep(QStringLiteral("proof/checkAsYouType"), true, true);
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select("E5");
        e.type("see speling here ");
        CellEditor *editor = w.cellEditor();
        QTRY_COMPARE(editor->misspellings().size(), 1);
        // Options: the box is found (by its words), looked at, set, and the dialog accepted; the timer keeps looking until the dialog is up.
        QString boxText;
        bool boxWas = false;
        QTimer watch;
        watch.setInterval(20);
        QObject::connect(&watch, &QTimer::timeout, &watch, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            watch.stop();
            for (QCheckBox *box : d->findChildren<QCheckBox *>())
                if (box->text().contains(QLatin1String("spelling as you type"), Qt::CaseInsensitive)) {
                    boxText = box->text();
                    boxWas = box->isChecked();
                    box->setChecked(false);
                }
            d->accept();
        });
        watch.start();
        w.act(QStringLiteral("file.options"))->trigger();
        QCOMPARE(boxText, QStringLiteral("Check spelling as you type"));
        QVERIFY(boxWas);
        QVERIFY(editor->misspellings().isEmpty());   // (at once, in the open editor)
        QVERIFY(!CellEditor::spellingOn());
        QVERIFY(!jp::Settings::get().value(QStringLiteral("proof/checkAsYouType"), true).toBool());
        e.key(Qt::Key_Escape);
    }

    void theOldCrunchKeyMovesToTheSharedChoice()
    {
        QTemporaryDir dir;
        const QString key = QStringLiteral("proof/checkAsYouType");
        {   // JeffCrunch's old key, off: the shared choice starts off, read once
            QSettings app(dir.filePath(QStringLiteral("a.ini")), QSettings::IniFormat), suite(dir.filePath(QStringLiteral("s.ini")), QSettings::IniFormat);
            app.setValue(QStringLiteral("proof/spellAsYouType"), false);
            jp::migrateSpellSetting(app, suite);
            QVERIFY(suite.contains(key));
            QVERIFY(!suite.value(key).toBool());
            suite.setValue(key, true);   // (the person turns it on again; the old key is not read a second time)
            jp::migrateSpellSetting(app, suite);
            QVERIFY(suite.value(key).toBool());
        }
        {   // JeffWrite's old key, off, likewise
            QSettings app(dir.filePath(QStringLiteral("b.ini")), QSettings::IniFormat), suite(dir.filePath(QStringLiteral("t.ini")), QSettings::IniFormat);
            app.setValue(QStringLiteral("proof/asYouType"), false);
            jp::migrateSpellSetting(app, suite);
            QVERIFY(!suite.value(key, true).toBool());
        }
        {   // on, or never set: nothing is written, and it stays on
            QSettings app(dir.filePath(QStringLiteral("c.ini")), QSettings::IniFormat), suite(dir.filePath(QStringLiteral("u.ini")), QSettings::IniFormat);
            app.setValue(QStringLiteral("proof/spellAsYouType"), true);
            jp::migrateSpellSetting(app, suite);
            QVERIFY(!suite.contains(key));
        }
        QVERIFY(jp::suiteWide(key));
    }

    // ---- AutoCorrect in a cell: what is a formula, and what is prose

    void anEntryIsAFormulaForAutoCorrectAsTheCellTakesIt()
    {
        using jp::office::sheet::EntryOptions;
        using jp::office::sheet::readEntry;
        EntryOptions options;
        for (const QString &typed : {QStringLiteral("=1+2"), QStringLiteral("+ A1 - B1"), QStringLiteral("- hello"), QStringLiteral("-5"), QStringLiteral("+5"), QStringLiteral("-"), QStringLiteral("+A1"),
                                     QStringLiteral("- 5"), QStringLiteral("-1/2"), QStringLiteral("-A1*2"), QStringLiteral("hello"), QStringLiteral("'-x"), QStringLiteral("--teh")}) {
            const jp::office::sheet::EntryResult r = readEntry(typed, options);
            const bool cell = r.entry.kind == jp::office::sheet::Entry::Kind::Formula || (!r.ok && (typed.startsWith(QLatin1Char('=')) || typed.startsWith(QLatin1Char('+')) || typed.startsWith(QLatin1Char('-'))));
            QCOMPARE(CellEditor::isFormulaEntry(typed), cell && typed != QLatin1String("-") && typed != QLatin1String("+") && typed.size() > 0);
        }
        QVERIFY(CellEditor::isFormulaEntry(QStringLiteral("+ A1 - B1")));
        QVERIFY(CellEditor::isFormulaEntry(QStringLiteral("- hello")));
        QVERIFY(!CellEditor::isFormulaEntry(QStringLiteral("-5")));
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select("E5");
        e.type("+ A1 - B1 teh ");
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("+ A1 - B1 teh "));
        e.key(Qt::Key_Escape);
    }

    void autoCorrectInACellLeavesCodesNamesAndAddressesAlone()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CellEditor *editor = e.win.w.cellEditor();
        auto typed = [&](const char *ref, const char *text) {
            e.select(ref);
            e.type(QString::fromUtf8(text));
            const QString got = editor->text();
            e.key(Qt::Key_Escape);
            return got;
        };
        // the first word of an entry is not made a capital; one after the end of a sentence is
        QCOMPARE(typed("E5", "total of sales. next year "), QStringLiteral("total of sales. Next year "));
        // a word in a name, an address, or a path is not
        QCOMPARE(typed("E6", "see. report.xlsx and john.smith@x.com "), QStringLiteral("see. report.xlsx and john.smith@x.com "));
        QCOMPARE(typed("E7", "Done. c:\\temp\\file "), QStringLiteral("Done. c:\\temp\\file "));
        QCOMPARE(typed("E8", "Done. a/b "), QStringLiteral("Done. a/b "));
        // an identifier typed alone is not changed by the replacement list, and prose is
        QCOMPARE(typed("E9", "(c) "), QStringLiteral("(c) "));
        QCOMPARE(typed("E10", "FO "), QStringLiteral("FO "));
        QCOMPARE(typed("E11", "IM "), QStringLiteral("IM "));
        QCOMPARE(typed("E12", "a (c) b "), QString::fromUtf8("a \u00a9 b "));
        QCOMPARE(typed("E13", "x FO "), QStringLiteral("x OF "));
        // two initial capitals are still fixed in a first word
        QCOMPARE(typed("E14", "THursday "), QStringLiteral("Thursday "));
        // an address typed whole is a link to the address as typed
        quint32 r = 0, c = 0;
        e.select("F5");
        e.enter("https://example.org/Page");
        parseCellRef(QStringLiteral("F5"), &r, &c);
        const auto *link = e.win.w.session()->anchors().linkAt(0, r, c);
        QVERIFY(link);
        QCOMPARE(link->target, QStringLiteral("https://example.org/Page"));
        QCOMPARE(e.shownAt("F5"), QStringLiteral("https://example.org/Page"));
    }

    // ---- Spelling writes text as text

    void aCorrectedTextStaysTextWhateverItLooksLike()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        auto put = [&](const char *ref, const QString &text) {
            quint32 r = 0, c = 0;
            QVERIFY(parseCellRef(QString::fromLatin1(ref), &r, &c));
            QVERIFY(w.session()->type(0, r, c, text, false).ok);
        };
        put("A5", QStringLiteral("'Febuary 3"));
        put("A6", QStringLiteral("'-Febuary 3"));
        put("A7", QStringLiteral("'Truee"));
        put("A8", QStringLiteral("'zzqxwrd"));
        QVERIFY(e.settle());
        auto textAt = [&](quint32 row) { return w.session()->valueAt(0, row, 0); };
        QVERIFY(textAt(4).isText());
        SpellingDialog d(&w, 0, jp::office::sheet::CellRange{4, 0, 6, 0, true});
        QVERIFY(d.start(true));
        QCOMPARE(d.current()->word, QStringLiteral("Febuary"));
        d.setReplacement(QStringLiteral("February"));
        QVERIFY(d.change());
        QVERIFY(textAt(4).isText());   // not a date
        QCOMPARE(textAt(4).text(), QStringLiteral("February 3"));
        QCOMPARE(d.current()->word, QStringLiteral("Febuary"));
        d.setReplacement(QStringLiteral("February"));
        QVERIFY(d.change());
        QVERIFY(textAt(5).isText());   // not a formula
        QCOMPARE(textAt(5).text(), QStringLiteral("-February 3"));
        QCOMPARE(d.current()->word, QStringLiteral("Truee"));
        d.setReplacement(QStringLiteral("TRUE"));
        d.change();
        QVERIFY(textAt(6).isText());   // not a truth value
        QCOMPARE(textAt(6).text(), QStringLiteral("TRUE"));
        // Change All is all or none: a locked cell among them stops it before any is changed
        put("B5", QStringLiteral("zzqxwrd"));
        put("B6", QStringLiteral("zzqxwrd"));
        QVERIFY(e.settle());
        const int steps = w.session()->undoStack()->index();
        SpellingDialog all(&w, 0, jp::office::sheet::CellRange{0, 0, 20, 10, true});
        QVERIFY(all.start(true));
        all.setReplacement(QStringLiteral("word"));
        QVERIFY(all.current());
        all.changeAll();
        QCOMPARE(w.session()->undoStack()->index(), steps + 1);
        QCOMPARE(w.session()->valueAt(0, 4, 1).text(), QStringLiteral("word"));
        QCOMPARE(w.session()->valueAt(0, 7, 0).text(), QStringLiteral("word"));
        w.session()->undoStack()->undo();
        QCOMPARE(w.session()->valueAt(0, 4, 1).text(), QStringLiteral("zzqxwrd"));
        QCOMPARE(w.session()->valueAt(0, 7, 0).text(), QStringLiteral("zzqxwrd"));
        // and all or none: one cell would be longer than a cell may be, so no cell is changed
        put("C5", QStringLiteral("'zzqxwrd ") + QString(32755, QLatin1Char('x')));
        QVERIFY(e.settle());
        const int before = w.session()->undoStack()->index();
        SpellingDialog tooLong(&w, 0, jp::office::sheet::CellRange{0, 0, 20, 10, true});
        QVERIFY(tooLong.start(true));
        tooLong.setReplacement(QStringLiteral("abcdefghijklmnop"));
        tooLong.changeAll();
        QCOMPARE(w.session()->undoStack()->index(), before);
        QCOMPARE(w.session()->valueAt(0, 4, 1).text(), QStringLiteral("zzqxwrd"));
        QCOMPARE(w.session()->valueAt(0, 7, 0).text(), QStringLiteral("zzqxwrd"));
    }

    // ---- a big sheet: the walk goes in slices

    void theSpellingWalkOfAMillionCellsHoldsTheWindowOnlyBriefly()
    {
        QByteArray rows;
        rows.reserve(60 * 1000 * 1000);
        for (int r = 1; r <= 1000000; ++r) rows += "<row r=\"" + QByteArray::number(r) + "\"><c r=\"A" + QByteArray::number(r) + "\" t=\"s\"><v>" + QByteArray::number(r % 40) + "</v></c></row>";
        QStringList strings;
        for (int i = 0; i < 40; ++i) strings << QStringLiteral("<t>words that are right number %1 and some more of them</t>").arg(i);
        strings[17] = QStringLiteral("<t>the cache holds this text of many words that are spelled right</t>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QString::fromLatin1(rows))}, sharedStringsXml(strings))));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.session()->type(0, 999998, 0, QStringLiteral("'at the end zzqxwrd"), false).ok);
        QElapsedTimer total;
        total.start();
        SpellingDialog d(&w, 0, jp::office::sheet::CellRange{0, 0, 1048575, 16383, true});
        QVERIFY(d.start(true));
        qInfo("SPELLING walk of 1,000,000 text cells: %lld ms in all, the longest slice %lld ms", total.elapsed(), d.longestSliceMs());
        QCOMPARE(d.current()->word, QStringLiteral("zzqxwrd"));
        QCOMPARE(d.current()->row, quint32(999998));
        QVERIFY2(d.longestSliceMs() < 300, qPrintable(QString::number(d.longestSliceMs())));
    }

    // ---- the cell editor measures once, and the address rule is fast

    void theCellEditorMeasuresItsWordsOncePerTextNotAtEveryPaint()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select("E5");
        e.type("see speling here ");
        CellEditor *editor = e.win.w.cellEditor();
        QTRY_COMPARE(editor->misspellings().size(), 1);
        const int measured = editor->measureCount();
        QVERIFY(measured >= 1);
        for (int i = 0; i < 5; ++i) {
            editor->update();
            QCoreApplication::processEvents();
            editor->grab();
        }
        QCOMPARE(editor->measureCount(), measured);
        e.key(Qt::Key_Escape);
    }

    void theAddressRuleTakesLinearTimeOnHostileText()
    {
        const int n = 30000;
        const QStringList hostile{QStringLiteral("a@b") + QStringLiteral(".c").repeated(n / 2) + QLatin1Char(' '),
                                  QStringLiteral("http://") + QString(n, QLatin1Char('a')) + QLatin1Char(' '),
                                  QStringLiteral("a.").repeated(n / 2) + QLatin1Char('@'),
                                  QStringLiteral("www.") + QStringLiteral(".a").repeated(n / 2) + QStringLiteral(" x"),
                                  QStringLiteral("a@") + QStringLiteral("b-").repeated(n / 2) + QStringLiteral(" "),
                                  QStringLiteral("x@y.z") + QStringLiteral("-.").repeated(n / 2) + QStringLiteral(" "),
                                  QStringLiteral(" ").repeated(n),
                                  QStringLiteral("a ").repeated(n / 2) + QStringLiteral("a@")};
        qint64 worst = 0;
        for (const QString &text : hostile) {
            QElapsedTimer t;
            t.start();
            (void)autotext::linkWord(text);
            worst = std::max(worst, t.elapsed());
        }
        qInfo("ADDRESS rule on 30,000 characters of hostile text: the slowest took %lld ms", worst);
        QVERIFY2(worst < 50, qPrintable(QString::number(worst)));
    }

    // ---- a workbook with macros is recovered with its macros

    void aRecoveredWorkbookWithMacrosStillHasThem()
    {
        QByteArray macroBook;
        {
            QMap<QString, QByteArray> parts;
            QVERIFY(readZip(editBook(), parts));
            ZipWriter z;
            for (auto it = parts.begin(); it != parts.end(); ++it) {
                QByteArray data = it.value();
                if (it.key() == QLatin1String("[Content_Types].xml"))
                    data.replace("spreadsheetml.sheet.main+xml", "ms-excel.sheet.macroEnabled.main+xml").replace("</Types>", "<Override PartName=\"/xl/vbaProject.bin\" ContentType=\"application/vnd.ms-office.vbaProject\"/></Types>");
                if (it.key() == QLatin1String("xl/_rels/workbook.xml.rels"))
                    data.replace("</Relationships>", "<Relationship Id=\"rIdVba\" Type=\"http://schemas.microsoft.com/office/2006/relationships/vbaProject\" Target=\"vbaProject.bin\"/></Relationships>");
                z.add(it.key(), data, true);
            }
            z.add(QStringLiteral("xl/vbaProject.bin"), QByteArray("VBA project bytes \x01\x02\x03", 20), true);
            macroBook = z.finish();
        }
        QTemporaryDir root;
        QVERIFY(root.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            EditWindow e;
            QVERIFY(e.open(macroBook, QStringLiteral("macros.xlsm")));
            e.select("E5");
            e.enter("macro work");
            QVERIFY(e.settle());
            QVERIFY(recoverNow(e.win.w));
            const QString copy = e.win.w.recoveryCopy();
            QMap<QString, QByteArray> parts;
            {
                QFile f(copy);
                QVERIFY(f.open(QIODevice::ReadOnly));
                QVERIFY(readZip(f.readAll(), parts));
            }
            QVERIFY(parts.contains(QStringLiteral("xl/vbaProject.bin")));
            QCOMPARE(parts.value(QStringLiteral("xl/vbaProject.bin")), QByteArray("VBA project bytes \x01\x02\x03", 20));
            QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("macroEnabled.main+xml"));
            QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("ms-office.vbaProject"));
            QVERIFY(parts.value(QStringLiteral("xl/_rels/workbook.xml.rels")).contains("vbaProject"));
            // a crash, and the copy opened as unsaved work: the macros are still in it, and it is still a workbook with macros
            const QString crashed = QDir(root.path()).filePath(QStringLiteral("crashed-run"));
            QVERIFY(QDir().mkpath(crashed));
            QVERIFY(QFile::copy(copy, crashed + QStringLiteral("/macros.xlsx")));
            QVERIFY(QFile::copy(copy.chopped(5) + QStringLiteral(".json"), crashed + QStringLiteral("/macros.json")));
            const QVector<recovery::Recovered> found = recovery::orphans();
            QCOMPARE(found.size(), 1);
            Window host;
            QVERIFY(host.openBlank());
            QSignalSpy loaded(&host.w, &CrunchWindow::loadFinished);
            QString err;
            QVERIFY2(host.w.openRecovered(found[0], &err), qPrintable(err));
            QTRY_VERIFY_WITH_TIMEOUT(loaded.count() > 0 && !host.w.recoveryCopy().isEmpty(), 20000);
            // saved under a name of the kind, the macros are in the file
            const QString out = e.folder.dir.filePath(QStringLiteral("recovered.xlsm"));
            QVERIFY(saveAndWait(host.w, out));
            QMap<QString, QByteArray> saved;
            QFile f(out);
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY(readZip(f.readAll(), saved));
            QCOMPARE(saved.value(QStringLiteral("xl/vbaProject.bin")), QByteArray("VBA project bytes \x01\x02\x03", 20));
            QVERIFY(saved.value(QStringLiteral("[Content_Types].xml")).contains("macroEnabled.main+xml"));
            e.win.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }

    // The time a recovery copy takes for a big text file's workbook, which is written on the window's thread (JP_BIG_CSV names one; it is only measured, never checked).
    void bigTextFileRecoveryIsTimed()
    {
        const QString big = qEnvironmentVariable("JP_BIG_CSV");
        if (big.isEmpty()) QSKIP("JP_BIG_CSV is not set");
        QTemporaryDir root;
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            Window win;
            QVERIFY(win.open(big));
            QVERIFY(win.w.session()->type(0, 0, 5, QStringLiteral("changed"), false).ok);
            QSignalSpy written(&win.w, &CrunchWindow::recoveryWritten);
            QElapsedTimer held, total;
            held.start();
            total.start();
            win.w.autoRecover();
            const double ms = held.nsecsElapsed() / 1e6;
            QVERIFY(written.count() > 0 || written.wait(600000));
            qInfo("RECOVERY text file %.1f MB: the window was held %.1f ms by the tick (%.1f ms of it copying the workbook object); the copy took %lld ms in all", QFileInfo(big).size() / 1048576.0, ms,
                  win.w.lastRecoveryCopyMs(), total.elapsed());
            win.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }

    // The time a recovery copy takes for a big workbook (JP_BIG_XLSX names one; it is only measured, never checked).
    void bigWorkbookRecoveryIsTimed()
    {
        const QString big = qEnvironmentVariable("JP_BIG_XLSX");
        if (big.isEmpty()) QSKIP("JP_BIG_XLSX is not set");
        QTemporaryDir root;
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            Window win;
            QVERIFY(win.open(big));
            for (quint32 i = 0; i < 2000; ++i) QVERIFY(win.w.session()->type(0, i, 9, QStringLiteral("changed %1").arg(i), false).ok);
            QElapsedTimer total, loop;
            QSignalSpy written(&win.w, &CrunchWindow::recoveryWritten);
            total.start();
            loop.start();
            win.w.autoRecover();
            const double blocked = loop.nsecsElapsed() / 1e6;
            QVERIFY(written.wait(600000));
            qInfo("RECOVERY big workbook %.1f MB: the window was held %.1f ms (2000 edited cells), the copy took %lld ms in all", QFileInfo(big).size() / 1048576.0, blocked, total.elapsed());
            win.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
        }
        recovery::setRoot(QString());
    }
};

JP_TEST_CLASS(CrunchProofTests)
#include "tst_crunch_proof.moc"
