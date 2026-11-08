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
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "../localepin.h"
#include "../sheet/sheethelpers.h"
#include "io/zip.h"
#include "office/package.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QSettings>
#include <QSignalSpy>
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
            QCOMPARE(host.w.session()->valueAt(0, r, c).text(), QStringLiteral("Lost work"));
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
        QCOMPARE(editor->text(), QStringLiteral("See speling "));   // (the capital at the start of the entry is AutoCorrect's)
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
        QCOMPARE(editor->text(), QStringLiteral("See spelling "));
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
        QCOMPARE(editor->text(), QStringLiteral("See the "));
        // one Undo takes the fix back, and leaves what was typed
        pressUndo(editor);
        QCOMPARE(editor->text(), QStringLiteral("See teh "));
        e.key(Qt::Key_Escape);
        e.select("E5");
        e.enter("see teh car");
        QCOMPARE(e.shownAt("E5"), QStringLiteral("See the car"));
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
        QCOMPARE(editor->text(), QString::fromUtf8("Say “hi” a—b add ½ "));
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


    // The copy is the workbook as it was when the copy was begun, however the typing goes on while it is written (the file is made from a private reading of the file and a copy of
    // what changed, never from the workbook the window edits).
    void aRecoveryCopyIsAConsistentSnapshotWhileTheEditingGoesOn()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".xlsx"));
        {
            // (a sheet big enough that the copy takes a while to write)
            QByteArray rows;
            for (int r = 10; r < 90000; ++r) {
                rows += "<row r=\"" + QByteArray::number(r) + "\">";
                for (const char *c : {"A", "B", "C", "D", "H"}) rows += "<c r=\"" + QByteArray(c) + QByteArray::number(r) + "\"><v>" + QByteArray::number(r * 7) + ".25</v></c>";
                rows += "</row>";
            }
            EditWindow e;
            QVERIFY(e.open(editBook(rows)));
            CrunchWindow &w = e.win.w;
            auto at = [](const char *ref, quint32 *r, quint32 *c) { QVERIFY(parseCellRef(QString::fromLatin1(ref), r, c)); };
            quint32 r = 0, c = 0;
            at("F2", &r, &c);
            QVERIFY(w.session()->type(0, r, c, QStringLiteral("before the copy"), false).ok);
            QVERIFY(e.settle());
            QSignalSpy written(&w, &CrunchWindow::recoveryWritten);
            w.autoRecover();   // the copy is begun: what it holds is what the workbook holds now
            QVERIFY(w.isRecovering());
            // ... and the person goes on: a new sheet name, new texts (new shared strings), a new number format, until the copy is written
            QVERIFY(w.session()->renameSheet(0, QStringLiteral("Renamed")).ok);
            int edits = 0;
            QElapsedTimer limit;
            limit.start();
            while (written.isEmpty() && limit.elapsed() < 60000) {
                at("G2", &r, &c);
                QVERIFY(w.session()->type(0, r + quint32(edits % 50), c, QStringLiteral("later %1").arg(edits), false).ok);
                at("H2", &r, &c);
                QVERIFY(w.session()->type(0, r + quint32(edits % 50), c, QStringLiteral("%1.5%").arg(edits % 97), false).ok);
                ++edits;
                QCoreApplication::processEvents();
            }
            QVERIFY(!written.isEmpty() || written.wait(60000));
            QVERIFY(written.last().first().toBool());
            qInfo("RECOVERY race test: %d edit rounds while the copy was written", edits);
            const QString copy = w.recoveryCopy();
            QVERIFY(QFileInfo::exists(copy));
            Package p;
            QString why;
            QVERIFY2(p.readLazyFile(copy, &why), qPrintable(why));
            Workbook b;
            QVERIFY2(b.open(p, &why), qPrintable(why));
            QCOMPARE(b.info().sheets.value(0).name, QStringLiteral("Sheet1"));   // not the new name
            Sheet s;
            QVERIFY(b.readSheet(0, &s));
            QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("F2")), QStringLiteral("s:before the copy"));
            QVERIFY(!at_(s, "G2"));   // nothing typed after the copy was begun is in it
            QVERIFY(!at_(s, "G3"));
            w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
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
