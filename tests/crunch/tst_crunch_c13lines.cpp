// Tests of the first-release lines closed in round 13 of JeffCrunch: the built-in themed cell styles, Repeat, and the Undo history list.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "core/fonts.h"
#include "core/cellstyles.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "savedok.h"
#include "sheet/sheetreader.h"

#include "app/protectdialogs.h"
#include "app/repeat.h"
#include "app/ribbon.h"
#include "app/ribboncustom.h"
#include "app/undohistory.h"
#include "sheet/protection.h"
#include "../localepin.h"
#include "../sheet/sheethelpers.h"

#include <QAccessible>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSettings>
#include <QTemporaryDir>
#include <QAction>
#include <QFile>
#include <QKeyEvent>
#include <QMenu>
#include <QSignalSpy>
#include <QToolButton>
#include <QUndoStack>
#include <memory>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

QColor lookFill(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::GridSheet &gs = e.view()->sheet();
    return QColor::fromRgb(gs.lookOf(r, c, gs.styleOf(r, c, gs.cellAt(r, c))).fg);
}

grid::CellLook lookOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::GridSheet &gs = e.view()->sheet();
    return gs.lookOf(r, c, gs.styleOf(r, c, gs.cellAt(r, c)));
}

QAction *entryOf(QMenu *menu, const QString &text)
{
    for (QAction *a : menu->actions())
        if (a->text() == text) return a;
    return nullptr;
}

// A workbook of the XML form that has its named styles (just Normal): the styles of a file made by a spreadsheet program.
QByteArray bookWithNamedStyles()
{
    const QByteArray styles = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
                              "<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
                              "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                              "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                              "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                              "<cellXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/></cellXfs>"
                              "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles></styleSheet>";
    return buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}, QByteArray(), QString(), QString(), styles);
}

// The name of the style the cell's format follows ("" when the workbook has none to name).
QString styleNameAt(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::GridSheet &gs = e.view()->sheet();
    const quint32 xf = gs.styleOf(r, c, gs.cellAt(r, c));
    const jp::office::sheet::Styles &st = e.win.w.session()->workbook()->styles();
    if (xf >= quint32(st.formats.size())) return QString();
    const quint32 id = st.formats[int(xf)].xfId;
    for (const jp::office::sheet::CellStyleName &n : st.cellStyles)
        if (n.xfId == id) return n.name;
    return QString();
}

// The fill of the cell's format as the workbook holds it (a theme reference stays one).
jp::office::sheet::FillSpec fillSpecAt(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::GridSheet &gs = e.view()->sheet();
    const quint32 xf = gs.styleOf(r, c, gs.cellAt(r, c));
    const jp::office::sheet::Styles &st = e.win.w.session()->workbook()->styles();
    if (xf >= quint32(st.formats.size()) || st.formats[int(xf)].fill >= quint32(st.fills.size())) return jp::office::sheet::FillSpec();
    return st.fills[int(st.formats[int(xf)].fill)];
}

bool applyFromGallery(EditWindow &e, const QString &cell, const QString &style)
{
    auto *gallery = e.win.w.findChild<QMenu *>(QStringLiteral("stylesMenu"));
    if (!gallery) return false;
    e.select(cell);
    Q_EMIT gallery->aboutToShow();
    QAction *a = entryOf(gallery, style);
    if (!a) return false;
    a->trigger();
    return true;
}

QByteArray savedBytes(EditWindow &e, const QString &name)
{
    const QString path = e.folder.dir.filePath(name);
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.saveAs(path) || !savedOk(saved)) return QByteArray();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return QByteArray();
    return f.readAll();
}

// Presses the key at the grid as a person does, with the window active.
void pressKey(EditWindow &e, Qt::Key key, Qt::KeyboardModifiers mods = Qt::NoModifier)
{
    e.win.w.activateWindow();
    QTest::qWaitForWindowActive(&e.win.w);
    e.view()->setFocus();
    QTest::keyClick(e.view(), key, mods);
}

} // namespace

class CrunchLines13Tests : public QObject {
    Q_OBJECT

    // (the text these tests match is English, whatever the machine's language is)
    std::unique_ptr<PinnedLocale> m_pin;

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;   // (the toolbar the person has is kept in the settings: a folder of this test's own)
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        jp::initCore();
        m_pin = std::make_unique<PinnedLocale>();
    }
    void cleanupTestCase() { m_pin.reset(); }

    // Each themed built-in style, applied from the gallery, draws its accent (not black), in a blank workbook and in one opened from a text file (which has no named styles of its own).
    void aThemedCellStyleDrawsItsFillWhereverTheWorkbookCameFrom()
    {
        for (int source : {0, 1, 2}) {
            EditWindow e;
            if (source == 0) QVERIFY(e.openBlank());
            else if (source == 1) QVERIFY(e.open(bookWithNamedStyles()));
            else QVERIFY(e.open(QByteArray("a,b\n1,2\n"), QStringLiteral("t.csv")));
            QVERIFY(e.settle());
            auto *gallery = e.win.w.findChild<QMenu *>(QStringLiteral("stylesMenu"));
            QVERIFY(gallery);
            for (int a = 1; a <= 6; ++a) {
                for (const QString &name : {QStringLiteral("Good"), QStringLiteral("Accent%1").arg(a), QStringLiteral("20% - Accent%1").arg(a), QStringLiteral("60% - Accent%1").arg(a)}) {
                    e.select(QStringLiteral("E5"));
                    Q_EMIT gallery->aboutToShow();
                    QAction *act = entryOf(gallery, name);
                    QVERIFY2(act, qPrintable(name));
                    act->trigger();
                    const QColor fill = lookFill(e, QStringLiteral("E5"));
                    quint32 rr = 0, cc = 0;
                    parseCellRef(QStringLiteral("E5"), &rr, &cc);
                    const grid::GridSheet &gs = e.view()->sheet();
                    const grid::CellLook look = gs.lookOf(rr, cc, gs.styleOf(rr, cc, gs.cellAt(rr, cc)));
                    QVERIFY2(look.pattern == grid::PatternKind::Solid, qPrintable(QStringLiteral("%1 source %2 has no solid fill").arg(name).arg(source)));
                    QVERIFY2(fill != QColor(Qt::black) && fill.alpha() == 255 && fill.lightness() > 20, qPrintable(QStringLiteral("%1 source %2 gave %3").arg(name).arg(source).arg(fill.name())));
                }
            }
        }
    }

    // A workbook opened from a text file has no named styles of the XML form: the style still gives its look to the cell, as one step of Undo, and the look is in the saved file when
    // the window saves and opens it again.
    void aBuiltInStyleOnATextFileWorkbookIsKeptByTheWindowsSaveAndOpen()
    {
        EditWindow e;
        QVERIFY(e.open(QByteArray("a,b\n1,2\n"), QStringLiteral("t.csv")));
        QVERIFY(e.settle());
        auto *gallery = e.win.w.findChild<QMenu *>(QStringLiteral("stylesMenu"));
        QVERIFY(gallery);
        e.select(QStringLiteral("A2"));
        const int steps = e.win.w.undoStack()->count();
        Q_EMIT gallery->aboutToShow();
        QAction *act = entryOf(gallery, QStringLiteral("Accent1"));
        QVERIFY(act);
        act->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        const QColor drawn = lookFill(e, QStringLiteral("A2"));
        QVERIFY(drawn != QColor(Qt::black));
        e.win.w.undoStack()->undo();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(lookFill(e, QStringLiteral("A2")) == QColor(Qt::black));
        e.win.w.undoStack()->redo();
        QCOMPARE(lookFill(e, QStringLiteral("A2")), drawn);
        const QString path = e.folder.dir.filePath(QStringLiteral("styled.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(path) && savedOk(saved));
        EditWindow again;
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        QVERIFY(again.open(bytes));
        QVERIFY(again.settle());
        QCOMPARE(lookFill(again, QStringLiteral("A2")), drawn);
        const grid::GridSheet &gs = again.view()->sheet();
        QVERIFY(gs.lookOf(1, 0, gs.styleOf(1, 0, gs.cellAt(1, 0))).pattern == grid::PatternKind::Solid);
    }

    // Repeat (F4) does the last format command again on the cells selected now, as one step of Undo, and the result is kept by the window's own save and open.
    void repeatDoesTheLastFormatAgainAsOneStep()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto *repeat = e.win.w.findChild<QAction *>(QStringLiteral("edit.repeat"));
        QVERIFY(repeat);
        QVERIFY(!repeat->isEnabled());
        e.select(QStringLiteral("A1"));
        const int s0 = e.win.w.undoStack()->count();
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), s0 + 1);
        QVERIFY(lookOf(e, QStringLiteral("A1")).bold);
        QVERIFY(repeat->isEnabled());
        QVERIFY(repeat->toolTip().contains(QStringLiteral("Bold")));
        e.view()->selectRange(1, 1, 2, 1);   // B2:B3
        const int steps = e.win.w.undoStack()->count();
        repeat->trigger();
        QVERIFY(lookOf(e, QStringLiteral("B2")).bold && lookOf(e, QStringLiteral("B3")).bold);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        e.win.w.undoStack()->undo();
        QVERIFY(!lookOf(e, QStringLiteral("B2")).bold && !lookOf(e, QStringLiteral("B3")).bold);
        e.win.w.undoStack()->redo();
        QVERIFY(lookOf(e, QStringLiteral("B2")).bold);
        // a style, then Repeat gives the same style to the new selection
        e.select(QStringLiteral("C1"));
        auto *gallery = e.win.w.findChild<QMenu *>(QStringLiteral("stylesMenu"));
        QVERIFY(gallery);
        Q_EMIT gallery->aboutToShow();
        QAction *good = entryOf(gallery, QStringLiteral("Good"));
        QVERIFY(good);
        good->trigger();
        const QRgb first = lookOf(e, QStringLiteral("C1")).fg;
        e.select(QStringLiteral("D1"));
        QVERIFY(repeat->toolTip().contains(QStringLiteral("Cell Style")));
        repeat->trigger();
        QCOMPARE(lookOf(e, QStringLiteral("D1")).fg, first);
        QVERIFY(first != 0);
        // a row inserted, then again at the new selection
        e.select(QStringLiteral("A5"));
        e.win.w.findChild<QAction *>(QStringLiteral("cells.insert.rows"))->trigger();
        const int rowsBefore = e.win.w.undoStack()->count();
        e.select(QStringLiteral("A9"));
        repeat->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), rowsBefore + 1);
        // saved and opened by the window
        QVERIFY(e.settle());
        const QString path = e.folder.dir.filePath(QStringLiteral("repeated.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(path) && savedOk(saved));
        EditWindow again;
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        QVERIFY(again.open(bytes));
        QVERIFY(again.settle());
        QVERIFY(lookOf(again, QStringLiteral("B2")).bold && lookOf(again, QStringLiteral("B3")).bold);
        QCOMPARE(lookOf(again, QStringLiteral("D1")).fg, first);
    }

    // Ctrl+Y repeats only while there is nothing to redo (while Redo has a step the key is Redo), and a command that was refused is not kept to repeat.
    void ctrlYRepeatsOnlyWhenThereIsNothingToRedo()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto *key = e.win.w.findChild<QAction *>(QStringLiteral("edit.repeat.ctrlY"));
        QVERIFY(key);
        e.select(QStringLiteral("A1"));
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.italic"))->trigger();
        QVERIFY(key->isEnabled());
        e.win.w.undoStack()->undo();
        QTRY_VERIFY(!key->isEnabled());   // (a step to redo: Ctrl+Y is Redo)
        e.win.w.undoStack()->redo();
        QTRY_VERIFY(key->isEnabled());
        key->trigger();
        QVERIFY(lookOf(e, QStringLiteral("A1")).italic);
    }

    // The Undo and Redo buttons have a list of steps, newest first: a click on one does it and the steps before it, as the steps themselves.
    void theHistoryListOnUndoDoesTheStepsUpToTheOneChosen()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        for (const char *cell : {"A1", "A2", "A3"}) {
            e.select(QString::fromLatin1(cell));
            e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"))->trigger();
        }
        e.select(QStringLiteral("B1"));
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.italic"))->trigger();
        auto *undoList = e.win.w.findChild<jp::UndoHistoryMenu *>(QStringLiteral("undoHistoryMenu"));
        auto *redoList = e.win.w.findChild<jp::UndoHistoryMenu *>(QStringLiteral("redoHistoryMenu"));
        QVERIFY(undoList && redoList);
        Q_EMIT undoList->aboutToShow();
        QCOMPARE(undoList->shownSteps(), 4);
        undoList->markSteps(3);
        QCOMPARE(undoList->marked(), 3);
        const int top = e.win.w.undoStack()->index();
        undoList->choose(3);
        QCOMPARE(e.win.w.undoStack()->index(), top - 3);
        QVERIFY(!lookOf(e, QStringLiteral("B1")).italic);
        QVERIFY(!lookOf(e, QStringLiteral("A3")).bold && !lookOf(e, QStringLiteral("A2")).bold);
        QVERIFY(lookOf(e, QStringLiteral("A1")).bold);
        Q_EMIT redoList->aboutToShow();
        QCOMPARE(redoList->shownSteps(), 3);
        redoList->choose(2);
        QCOMPARE(e.win.w.undoStack()->index(), top - 1);
        QVERIFY(lookOf(e, QStringLiteral("A2")).bold && lookOf(e, QStringLiteral("A3")).bold);
        QVERIFY(!lookOf(e, QStringLiteral("B1")).italic);
    }

    // ---- Repeat belongs to the workbook it was given in

    // A recording is made under the token of the document; another token drops it, and a new token drops what was recorded.
    void repeatKnowsWhichDocumentTheCommandWasGivenIn()
    {
        jp::RepeatCommand r;
        int done = 0;
        r.setDocument(1);
        r.record(QStringLiteral("One"), [&] { ++done; });
        QVERIFY(r.available());
        r.record(QStringLiteral("Other"), [&] { done += 100; }, 2);   // (given in a workbook that is not this one)
        QCOMPARE(r.name(), QStringLiteral("One"));
        r.setDocument(2);
        QVERIFY(!r.available());
        QVERIFY(!r.run());
        r.record(QStringLiteral("Late"), [&] { done += 100; }, 1);   // (a command of the workbook before, finished late)
        QVERIFY(!r.available());
        r.record(QStringLiteral("Two"), [&] { ++done; });
        QVERIFY(r.run());
        QCOMPARE(done, 1);
    }

    // Insert Rows done in a new workbook, undone, and the file opened in the same window: F4 does nothing there, and the action is off (and what was done and undone is still repeatable before).
    void repeatIsNotCarriedToAnotherWorkbookOpenedInTheSameWindow()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto *repeat = e.win.w.findChild<QAction *>(QStringLiteral("edit.repeat"));
        QVERIFY(repeat);
        e.select(QStringLiteral("A1"));
        e.win.w.findChild<QAction *>(QStringLiteral("cells.insert.rows"))->trigger();
        QVERIFY(repeat->isEnabled());
        e.win.w.undoStack()->undo();
        QVERIFY(repeat->isEnabled());   // (Repeat after Undo still repeats the kind of command that was done)
        QVERIFY(e.win.w.undoStack()->isClean());
        QVERIFY(e.open(editBook()));    // (a new workbook nobody has changed gives way to the file)
        QVERIFY(e.settle());
        QVERIFY(!repeat->isEnabled());
        e.select(QStringLiteral("A1"));
        const int steps = e.win.w.undoStack()->count();
        pressKey(e, Qt::Key_F4);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:10"));
        // (and what is done in this one is repeated in this one)
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"))->trigger();
        QTRY_VERIFY(repeat->isEnabled());
    }

    // F4 pressed with the focus in the Name box or another box of text is that box's, not Repeat's.
    void f4InAnotherBoxOfTextDoesNotRepeatOnTheGrid()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"))->trigger();
        e.select(QStringLiteral("B1"));
        auto *nameBox = e.win.w.findChild<QLineEdit *>(QStringLiteral("nameBox"));
        QVERIFY(nameBox);
        e.win.w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&e.win.w));
        nameBox->setFocus();
        QTRY_VERIFY(nameBox->hasFocus());
        const int steps = e.win.w.undoStack()->count();
        QTest::keyClick(nameBox, Qt::Key_F4);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!lookOf(e, QStringLiteral("B1")).bold);
        pressKey(e, Qt::Key_F4);
        QVERIFY(lookOf(e, QStringLiteral("B1")).bold);
    }

    // A command the sheet refuses is not kept to be repeated.
    void aRefusedCommandIsNotRecorded()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto *repeat = e.win.w.findChild<QAction *>(QStringLiteral("edit.repeat"));
        e.select(QStringLiteral("A1"));
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.italic"))->trigger();
        QVERIFY(repeat->toolTip().contains(QStringLiteral("Italic")));
        e.win.w.anchorHooks().protectAsker = [](ProtectRequest *) { return true; };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(e.win.w.session()->protectionOf(0).enabled);
        e.select(QStringLiteral("B1"));
        e.win.told.clear();
        const int steps = e.win.w.undoStack()->count();
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);   // (refused)
        QVERIFY(repeat->toolTip().contains(QStringLiteral("Italic")));
        QVERIFY(!repeat->toolTip().contains(QStringLiteral("Bold")));
    }

    // The keys of the commands are the original's (as this computer writes them), and no key belongs to two commands.
    void theKeysAreTheOriginalsAndNoKeyBelongsToTwoCommands()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto native = [](const QKeySequence &k) { return k.toString(QKeySequence::NativeText); };
        const QList<QPair<const char *, QKeySequence>> expected = {
            {"format.cells", QKeySequence(Qt::CTRL | Qt::Key_1)},          {"fill.down", QKeySequence(Qt::CTRL | Qt::Key_D)},
            {"fill.right", QKeySequence(Qt::CTRL | Qt::Key_R)},            {"edit.repeat", QKeySequence(Qt::Key_F4)},
            {"find.goto", QKeySequence(Qt::Key_F5)},                       {"review.spelling", QKeySequence(Qt::Key_F7)},
            {"formulas.calculate", QKeySequence(Qt::Key_F9)},              {"formulas.calculateSheet", QKeySequence(Qt::SHIFT | Qt::Key_F9)},
            {"notes.edit", QKeySequence(Qt::SHIFT | Qt::Key_F2)},          {"edit.autosum", crunch::autoSumKey()}};
        for (const auto &one : expected) {
            QAction *a = e.win.w.act(QString::fromLatin1(one.first));
            QVERIFY2(a, one.first);
            const QKeySequence want = QKeySequence(one.second);
            bool found = false;
            for (const QKeySequence &k : a->shortcuts()) found = found || native(k) == native(want);
            QVERIFY2(found, one.first);
        }
        QCOMPARE(crunch::autoSumKey(false), QKeySequence(Qt::ALT | Qt::Key_Equal));
        QCOMPARE(native(e.win.w.act(QStringLiteral("edit.undo"))->shortcut()), native(QKeySequence(QKeySequence::Undo)));
        // (Ctrl+Y is both Redo and Repeat on some computers, and only one is ever enabled; see below)
        QMap<QString, QStringList> byKey;
        for (const QString &id : e.win.w.actionIds()) {
            QAction *a = e.win.w.act(id);
            if (!a) continue;
            for (const QKeySequence &k : a->shortcuts())
                if (!k.isEmpty()) byKey[native(k)] << id;
        }
        QStringList shared;
        for (auto it = byKey.cbegin(); it != byKey.cend(); ++it)
            if (it.value().size() > 1) shared << it.key() + QLatin1Char('=') + it.value().join(QLatin1Char('+'));
        QVERIFY2(shared.isEmpty(), qPrintable(shared.join(QLatin1String("; "))));
    }

    // Ctrl+Y as a key: Redo while there is a step to redo; Repeat when there is none and something to repeat; nothing otherwise. Never both (the shortcut is not ambiguous).
    void ctrlYIsRedoRepeatOrNothingAsAKey()
    {
        const QKeyCombination ctrlY(Qt::CTRL | Qt::Key_Y);
        const bool ctrlYIsRedo = QKeySequence::keyBindings(QKeySequence::Redo).contains(QKeySequence(ctrlY));
        auto enabledWithCtrlY = [](EditWindow &e) {
            int n = 0;
            for (QAction *a : e.win.w.findChildren<QAction *>())
                if (a->isEnabled() && a->shortcuts().contains(QKeySequence(Qt::CTRL | Qt::Key_Y))) ++n;
            return n;
        };
        // neither: nothing happens
        {
            EditWindow e;
            QVERIFY(e.openBlank());
            QVERIFY(e.settle());
            const int steps = e.win.w.undoStack()->count();
            QVERIFY(enabledWithCtrlY(e) == 0);
            pressKey(e, Qt::Key_Y, Qt::ControlModifier);
            QCOMPARE(e.win.w.undoStack()->count(), steps);
            QCOMPARE(e.win.w.undoStack()->index(), steps);
        }
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"))->trigger();
        // a recording and nothing to redo: Repeat
        e.select(QStringLiteral("B1"));
        QTRY_COMPARE(enabledWithCtrlY(e), 1);
        pressKey(e, Qt::Key_Y, Qt::ControlModifier);
        QVERIFY(lookOf(e, QStringLiteral("B1")).bold);
        QCOMPARE(e.win.w.undoStack()->index(), e.win.w.undoStack()->count());
        // a step to redo: Redo (where Ctrl+Y is Redo), and only that
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(enabledWithCtrlY(e), ctrlYIsRedo ? 1 : 0);
        const int top = e.win.w.undoStack()->count();
        pressKey(e, Qt::Key_Y, Qt::ControlModifier);
        QCOMPARE(e.win.w.undoStack()->count(), top);   // (no step was added: it was not Repeat)
        QCOMPARE(e.win.w.undoStack()->index(), ctrlYIsRedo ? top : top - 1);
    }

    // The list shows 100 steps at most.
    void theHistoryListShowsAtMostOneHundredSteps()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto *bold = e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"));
        for (int i = 1; i <= 105; ++i) {
            e.select(QStringLiteral("A%1").arg(i));
            bold->trigger();
        }
        QVERIFY(e.win.w.undoStack()->count() >= 105);
        auto *undoList = e.win.w.findChild<jp::UndoHistoryMenu *>(QStringLiteral("undoHistoryMenu"));
        QVERIFY(undoList);
        Q_EMIT undoList->aboutToShow();
        QCOMPARE(undoList->shownSteps(), jp::UndoHistoryMenu::kMaxShown);
        // (the list itself holds to it, whatever an app's function gives)
        QStringList many;
        for (int i = 0; i < 150; ++i) many << QStringLiteral("Step %1").arg(i);
        jp::UndoHistoryMenu plain(false, [&] { return many; }, [](int) {});
        Q_EMIT plain.aboutToShow();
        QCOMPARE(plain.shownSteps(), 100);
        QCOMPARE(jp::UndoHistoryMenu::kMaxShown, 100);
        undoList->markSteps(5000);
        QCOMPARE(undoList->marked(), 100);
        const int top = e.win.w.undoStack()->index();
        undoList->choose(100);
        QCOMPARE(e.win.w.undoStack()->index(), top - 100);
    }

    // A step that is refused part way ends the run, and the window says how many were done.
    void aHistoryRunStopsAtARefusedStepAndSaysHowFarItGot()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("A1"));
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.italic"))->trigger();
        e.win.w.anchorHooks().protectAsker = [](ProtectRequest *r) {
            r->password = QStringLiteral("secret");
            return true;
        };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY_WITH_TIMEOUT(s->protectionOf(0).enabled, 60000);
        e.win.w.anchorHooks().passwordAsker = [](PasswordRequest *r) {
            r->password = QStringLiteral("secret");
            return true;
        };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();   // (takes it off: asks)
        QTRY_VERIFY_WITH_TIMEOUT(!s->protectionOf(0).enabled, 60000);
        int asked = 0;
        e.win.w.anchorHooks().passwordAsker = [&](PasswordRequest *) {
            ++asked;
            return false;   // (undoing the first protection takes the password off: the person says no)
        };
        const int top = e.win.w.undoStack()->index();
        auto *undoList = e.win.w.findChild<jp::UndoHistoryMenu *>(QStringLiteral("undoHistoryMenu"));
        QVERIFY(undoList);
        Q_EMIT undoList->aboutToShow();
        QVERIFY(undoList->shownSteps() >= 3);
        undoList->choose(3);
        QCOMPARE(e.win.w.undoStack()->index(), top - 1);   // (the first step went, the second was refused, the third was left)
        QCOMPARE(asked, 1);
        QVERIFY(s->protectionOf(0).enabled);
        QVERIFY(lookOf(e, QStringLiteral("A1")).italic);
        QVERIFY2(e.win.w.statusText().contains(QStringLiteral("1 of 3")), qPrintable(e.win.w.statusText()));
    }

    // ---- the history list for the keyboard and for screen readers

    void theHistoryListTakesKeys()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto *bold = e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"));
        for (int i = 1; i <= 12; ++i) {
            e.select(QStringLiteral("A%1").arg(i));
            bold->trigger();
        }
        auto *undoList = e.win.w.findChild<jp::UndoHistoryMenu *>(QStringLiteral("undoHistoryMenu"));
        QVERIFY(undoList);
        const int top = e.win.w.undoStack()->index();
        undoList->popup(e.win.w.mapToGlobal(QPoint(20, 20)));
        QTRY_VERIFY(undoList->isVisible());
        QCOMPARE(undoList->shownSteps(), 12);
        QCOMPARE(undoList->marked(), 0);
        QTest::keyClick(undoList, Qt::Key_Down);
        QCOMPARE(undoList->marked(), 1);
        QTest::keyClick(undoList, Qt::Key_Down);
        QTest::keyClick(undoList, Qt::Key_Down);
        QCOMPARE(undoList->marked(), 3);
        QTest::keyClick(undoList, Qt::Key_Up);
        QCOMPARE(undoList->marked(), 2);
        QTest::keyClick(undoList, Qt::Key_End);
        QCOMPARE(undoList->marked(), 12);
        QTest::keyClick(undoList, Qt::Key_Down);
        QCOMPARE(undoList->marked(), 12);   // (no further than the last)
        QTest::keyClick(undoList, Qt::Key_Home);
        QCOMPARE(undoList->marked(), 1);
        QTest::keyClick(undoList, Qt::Key_PageDown);
        const int paged = undoList->marked();
        QVERIFY(paged > 1);
        QTest::keyClick(undoList, Qt::Key_PageUp);
        QVERIFY(undoList->marked() < paged);
        // Escape closes it and does nothing
        QTest::keyClick(undoList, Qt::Key_Escape);
        QTRY_VERIFY(!undoList->isVisible());
        QCOMPARE(e.win.w.undoStack()->index(), top);
        // Enter does the marked steps
        undoList->popup(e.win.w.mapToGlobal(QPoint(20, 20)));
        QTRY_VERIFY(undoList->isVisible());
        QTest::keyClick(undoList, Qt::Key_Down);
        QTest::keyClick(undoList, Qt::Key_Down);
        QTest::keyClick(undoList, Qt::Key_Down);
        QTest::keyClick(undoList, Qt::Key_Return);
        QTRY_VERIFY(!undoList->isVisible());
        QCOMPARE(e.win.w.undoStack()->index(), top - 3);
    }

    void theHistoryListNamesItsRowsAndItsFooterForAScreenReader()
    {
        const QString longName = QStringLiteral("Typing %1 and more").arg(QString(300, QLatin1Char('x')));
        jp::UndoHistoryMenu menu(false, [&] { return QStringList{longName, QStringLiteral("Bold"), QStringLiteral("<b>Italic</b>")}; }, [](int) {});
        Q_EMIT menu.aboutToShow();
        auto *list = menu.findChild<QListWidget *>();
        auto *footer = menu.findChild<QLabel *>();
        QVERIFY(list && footer);
        QVERIFY(!list->accessibleName().isEmpty());
        // a long name is cut to the list's width, with the whole name as the tip and as the row's accessible name
        QVERIFY(menu.rowText(0).size() < longName.size());
        QVERIFY(menu.rowText(0).endsWith(QChar(0x2026)));
        QCOMPARE(menu.rowName(0), longName);
        QCOMPARE(list->item(0)->data(Qt::AccessibleTextRole).toString(), longName);
        QVERIFY(list->item(0)->toolTip().contains(longName));
        QCOMPARE(menu.rowText(1), QStringLiteral("Bold"));
        QVERIFY(menu.sizeHint().width() <= jp::UndoHistoryMenu::kMaxWidth + 40);
        QVERIFY(list->maximumWidth() <= jp::UndoHistoryMenu::kMaxWidth);
        // the footer is named by what it says, and the name follows the marked steps
        QCOMPARE(footer->accessibleName(), menu.footerText());
        menu.markSteps(2);
        QCOMPARE(footer->accessibleName(), menu.footerText());
        QVERIFY(footer->accessibleName().contains(QLatin1Char('2')));
        QCOMPARE(list->currentRow(), 1);
        menu.markSteps(3);
        QVERIFY(footer->accessibleName().contains(QLatin1Char('3')));
    }

    // ---- the quick toolbar the person has keeps the arrows of Undo and Redo

    void aCustomizedQuickToolbarKeepsTheUndoAndRedoArrows()
    {
        const auto reset = qScopeGuard([] {
            RibbonCustom().save();
            RibbonCustom::applyAll();
        });
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto arrowOf = [&](const char *id) {
            QAction *a = e.win.w.act(QString::fromLatin1(id));
            for (QToolButton *b : e.win.w.ribbon()->findChildren<QToolButton *>())
                if (b->defaultAction() == a && b->isVisibleTo(&e.win.w) && b->menu()) return b;
            return static_cast<QToolButton *>(nullptr);
        };
        QVERIFY(arrowOf("edit.undo"));   // (as the ribbon file has it)
        RibbonCustom c;
        c.quickSet = true;
        c.quick = {QStringLiteral("file.save"), QStringLiteral("edit.undo"), QStringLiteral("edit.redo")};   // (the commands' ids only, as an older build saved them)
        c.save();
        RibbonCustom::applyAll();
        QToolButton *undo = arrowOf("edit.undo"), *redo = arrowOf("edit.redo");
        QVERIFY(undo && redo);
        QCOMPARE(undo->popupMode(), QToolButton::MenuButtonPopup);
        QCOMPARE(redo->popupMode(), QToolButton::MenuButtonPopup);
        QVERIFY(!arrowOf("file.save"));   // (a command with no list has no arrow)
        // restored from the saved value in a later session
        RibbonCustom::load().applyTo(&e.win.w);
        QVERIFY(arrowOf("edit.undo") && arrowOf("edit.redo"));
    }

    // ---- built-in cell styles in a workbook that came from a text file

    // The style is applied by name: the cell keeps the name and a theme reference, and Undo, Redo, a style change, and the window's own save and open keep that.
    void aBuiltInStyleOnATextFileWorkbookIsAppliedByName()
    {
        EditWindow e;
        QVERIFY(e.open(QByteArray("a,b\n1,2\n"), QStringLiteral("t.csv")));
        QVERIFY(e.settle());
        QVERIFY(applyFromGallery(e, QStringLiteral("A2"), QStringLiteral("Accent1")));
        QCOMPARE(styleNameAt(e, QStringLiteral("A2")), QStringLiteral("Accent1"));
        const jp::office::sheet::FillSpec fill = fillSpecAt(e, QStringLiteral("A2"));
        QVERIFY(fill.fg.kind == jp::office::sheet::ColorSpec::Kind::Theme && fill.fg.index == 4);
        QCOMPARE(styleNameAt(e, QStringLiteral("B2")), QStringLiteral("Normal"));
        const QColor drawn = lookFill(e, QStringLiteral("A2"));
        QVERIFY(drawn != QColor(Qt::black));
        // the cell follows the style: a change of the style's look changes the cell
        jp::office::sheet::FormatChange change;
        change.fill = jp::office::sheet::FillSpec();
        change.fill->pattern = jp::office::sheet::PatternKind::Solid;
        change.fill->fg = jp::office::sheet::ColorSpec::fromRgb(0x336699);
        QVERIFY(e.win.w.session()->modifyCellStyle(QStringLiteral("Accent1"), change).ok);
        QCOMPARE(lookFill(e, QStringLiteral("A2")), QColor(0x33, 0x66, 0x99));
        e.win.w.undoStack()->undo();
        QCOMPARE(lookFill(e, QStringLiteral("A2")), drawn);
        // saved as the newer form and opened again: still the style, with a theme reference
        const QByteArray bytes = savedBytes(e, QStringLiteral("styled2.xlsx"));
        QVERIFY(!bytes.isEmpty());
        EditWindow again;
        QVERIFY(again.open(bytes));
        QVERIFY(again.settle());
        QCOMPARE(styleNameAt(again, QStringLiteral("A2")), QStringLiteral("Accent1"));
        const jp::office::sheet::FillSpec kept = fillSpecAt(again, QStringLiteral("A2"));
        QVERIFY(kept.fg.kind == jp::office::sheet::ColorSpec::Kind::Theme && kept.fg.index == 4);
        QCOMPARE(lookFill(again, QStringLiteral("A2")), drawn);
        // (a workbook made from a text file has no theme part to change, and the file saved from it has none: the reference is to the usual theme colors, which the drawn fill shows)
        QVERIFY(!e.win.w.session()->hasTheme() && !again.win.w.session()->hasTheme());
        // and a second style goes in the same tables
        QVERIFY(applyFromGallery(again, QStringLiteral("B2"), QStringLiteral("Good")));
        QCOMPARE(styleNameAt(again, QStringLiteral("B2")), QStringLiteral("Good"));
        QCOMPARE(styleNameAt(again, QStringLiteral("A2")), QStringLiteral("Accent1"));
    }

    // A workbook that has its named styles (made as a file of the XML form has them) takes a built-in style as it always did: one style more, the cell following it.
    void aWorkbookWithNamedStylesTakesABuiltInStyleAsBefore()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNamedStyles()));
        QVERIFY(e.settle());
        const qsizetype before = e.win.w.session()->workbook()->styles().styleXfs.size();
        QVERIFY(applyFromGallery(e, QStringLiteral("A1"), QStringLiteral("Accent2")));
        QCOMPARE(styleNameAt(e, QStringLiteral("A1")), QStringLiteral("Accent2"));
        QCOMPARE(e.win.w.session()->workbook()->styles().styleXfs.size(), before + 1);
        QCOMPARE(e.win.w.session()->workbook()->styles().cellStyles.size(), qsizetype(2));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        const QByteArray bytes = savedBytes(e, QStringLiteral("named.xlsx"));
        EditWindow again;
        QVERIFY(again.open(bytes));
        QVERIFY(again.settle());
        QCOMPARE(styleNameAt(again, QStringLiteral("A1")), QStringLiteral("Accent2"));
    }

    // When the style table is full the style is refused with the message, as before (no direct formatting is put in its place).
    void aFullStyleTableRefusesABuiltInStyleWithTheMessage()
    {
        QByteArray xfs;
        for (int i = 0; i < 100000; ++i) xfs += "<xf numFmtId=\"0\"/>";
        const QByteArray styles = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><cellStyleXfs count=\"100000\">" + xfs
            + "</cellStyleXfs><cellXfs count=\"1\"><xf numFmtId=\"0\"/></cellXfs><cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles></styleSheet>";
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}, QByteArray(), QString(), QString(), styles)));
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.session()->workbook()->styles().styleXfs.size(), qsizetype(100000));
        const int steps = e.win.w.undoStack()->count();
        e.win.told.clear();
        QVERIFY(applyFromGallery(e, QStringLiteral("A1"), QStringLiteral("Accent1")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("no place to keep the style")), qPrintable(e.win.told.join(QLatin1Char('\n'))));
        QVERIFY(lookFill(e, QStringLiteral("A1")) == QColor(Qt::black) || fillSpecAt(e, QStringLiteral("A1")).pattern != jp::office::sheet::PatternKind::Solid);
    }

    // The old binary form's list of sheets a formula names writes a reference to a sheet that is not there with the place 0xFFFF (sheet index -1 is packed unsigned, no shift of a negative number),
    // and a 3-D range of sheets and a formula of the other sheet are written beside it; the window's save and open keep them.
    void theOldFormatSaveWritesAReferenceToAMissingSheet()
    {
        EditWindow e;
        const QString one = QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>Gone!A1</f><v>0</v></c><c r=\"B1\"><f>SUM(Sheet1:Sheet2!A2)</f><v>3</v></c><c r=\"C1\"><f>Sheet2!A2*2</f><v>4</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>1</v></c></row>");
        const QString two = QStringLiteral("<row r=\"2\"><c r=\"A2\"><v>2</v></c></row>");
        QVERIFY(e.open(buildXlsx({worksheet(one), worksheet(two)}, QByteArray(), QString(), QString(), QByteArray())));
        QVERIFY(e.settle());
        const QString path = e.folder.dir.filePath(QStringLiteral("gone.xls"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        e.win.w.setLossAsker([](const QString &, const QString &) { return CrunchWindow::LossAnswer::Continue; });
        QVERIFY(e.win.w.saveAs(path, true));
        QTRY_VERIFY_WITH_TIMEOUT(!e.win.w.isSaving(), 60000);
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        QVERIFY(bytes.startsWith(QByteArray::fromHex("d0cf11e0a1b11ae1")));
        QVERIFY(bytes.contains(QByteArray::fromHex("0000ffffffff")));   // (the entry of the list for a sheet that is gone: this workbook, from 0xFFFF to 0xFFFF)
        EditWindow again;
        QVERIFY(again.open(bytes, QStringLiteral("gone.xls")));
        QVERIFY(again.settle());
        QCOMPARE(again.valueAt(QStringLiteral("B1")), e.valueAt(QStringLiteral("B1")));
        QCOMPARE(again.valueAt(QStringLiteral("C1")), e.valueAt(QStringLiteral("C1")));
    }
};

JP_TEST_CLASS(CrunchLines13Tests)
#include "tst_crunch_c13lines.moc"
