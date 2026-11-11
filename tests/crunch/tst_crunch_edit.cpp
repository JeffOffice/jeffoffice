#include "tst_crunch_edit.h"
#include "app/anchordialogs.h"

void CrunchEditTests::initTestCase()
{
    static QTemporaryDir settings;
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(crunch);
    setAppInfo(crunchAppInfo());
    initCore();
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}


// Help shows the keys of this computer: {key:Open} and the rest in a topic are the platform's own (Ctrl+Z, or the Mac's), and none is left as it was written; the topics that talk about
// typing, formulas in other locales, protection, tables, and a file another program changed say so.
void CrunchEditTests::helpShowsThePlatformsKeysAndSaysWhatTypingDoes()
{
    Window win;
    auto text = [&](const QString &id) { return help::displayMarkdown(help::topic(id), &win.w); };
    for (const QString &id : help::topicIds()) QVERIFY2(!text(id).contains(QLatin1String("{key:")), qPrintable(id));
    const QString keyboard = text(QStringLiteral("keyboard"));
    for (const QKeySequence::StandardKey k : {QKeySequence::Undo, QKeySequence::Redo, QKeySequence::Save, QKeySequence::SelectAll})
        QVERIFY2(keyboard.contains(QKeySequence::keyBindings(k).first().toString(QKeySequence::NativeText)), qPrintable(QKeySequence(k).toString(QKeySequence::NativeText)));
    QVERIFY(text(QStringLiteral("start")).contains(QKeySequence(QKeySequence::Open).toString(QKeySequence::NativeText)));
    QVERIFY(text(QStringLiteral("open-save")).contains(QKeySequence(QKeySequence::Save).toString(QKeySequence::NativeText)));
    const QString typing = text(QStringLiteral("edit-cells"));
    for (const char *phrase : {"English", "semicolon", "protected sheet", "table", "Redo", "1,000 steps"}) QVERIFY2(typing.contains(QLatin1String(phrase)), phrase);
    const QString saving = text(QStringLiteral("open-save"));
    for (const char *phrase : {"Save a Copy", "Overwrite", "already open", "Esc"}) QVERIFY2(saving.contains(QLatin1String(phrase)), phrase);
}

// The Help tab, as JeffWrite's: What's New and Feedback open their pages; Keyboard Shortcuts opens the Help pane on the shortcuts topic.
void CrunchEditTests::theHelpTabHasHelpShortcutsWhatsNewAndFeedback()
{
    Window win;
    win.w.show();
    QVERIFY2(win.w.ribbonErrors().isEmpty(), qPrintable(win.w.ribbonErrors()));
    QVERIFY(win.w.ribbon()->tab(QStringLiteral("Help")));
    for (const char *id : {"help.shortcuts", "help.whatsNew", "help.feedback"}) QVERIFY2(win.w.act(QString::fromLatin1(id)), id);
    QCOMPARE(win.w.act(QStringLiteral("help.shortcuts"))->text(), QStringLiteral("Keyboard Shortcuts"));
    QCOMPARE(win.w.act(QStringLiteral("help.whatsNew"))->text(), QStringLiteral("What's New"));
    QCOMPARE(win.w.act(QStringLiteral("help.feedback"))->text(), QStringLiteral("Feedback"));
    const auto saved = help::openUrl;
    QList<QUrl> opened;
    help::openUrl = [&](const QUrl &u) {
        opened << u;
        return true;
    };
    win.w.act(QStringLiteral("help.feedback"))->trigger();
    win.w.act(QStringLiteral("help.whatsNew"))->trigger();
    help::openUrl = saved;
    QCOMPARE(opened, (QList<QUrl>{help::feedbackUrl(), help::whatsNewUrl()}));
    QDockWidget *dock = win.w.findChild<QDockWidget *>(QStringLiteral("helpDock"));
    QVERIFY(!dock || !dock->isVisible());
    win.w.act(QStringLiteral("help.shortcuts"))->trigger();
    dock = win.w.findChild<QDockWidget *>(QStringLiteral("helpDock"));
    QVERIFY(dock && dock->isVisible());
}


// ---- typing

// A key that prints, with a cell selected, starts an entry in that cell: the cell's line is over the cell with the character in it, and the formula bar shows it too; Enter puts it in the
// cell and goes down.
void CrunchEditTests::typingOverACellPutsWhatWasTypedInIt()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("A5"));
    QVERIFY(!e.win.w.isEditing());
    QTest::keyClick(e.view(), Qt::Key_4);
    QVERIFY(e.win.w.isEditing());
    CellEditor *editor = e.win.w.cellEditor();
    QVERIFY(editor->isVisible());
    QCOMPARE(editor->text(), QStringLiteral("4"));
    QCOMPARE(editor->geometry().topLeft(), e.view()->cellRect(4, 0).topLeft());
    QVERIFY(editor->hasFocus() || editor->isVisible());
    QTest::keyClick(editor, Qt::Key_2);
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("42"));
    QCOMPARE(e.valueAt(QStringLiteral("A5")), QStringLiteral("-"));      // (not in the cell until it is finished)
    e.key(Qt::Key_Return);
    QVERIFY(!e.win.w.isEditing());
    QVERIFY(!editor->isVisible());
    QCOMPARE(e.valueAt(QStringLiteral("A5")), QStringLiteral("n:42"));
    QCOMPARE(e.shownAt(QStringLiteral("A5")), QStringLiteral("42"));
    QCOMPARE(e.selected(), QStringLiteral("A6"));
    QCOMPARE(e.win.w.formulaBar()->text(), QString());   // (the formula bar shows the cell the selection is on)
    QCOMPARE(e.win.w.undoStack()->count(), 1);
    QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Typing in A5"));
    // text, replacing what the cell held
    e.select(QStringLiteral("C1"));
    e.enter(QStringLiteral("world"));
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:world"));
    // a space and Backspace start entries too: the second clears the cell
    e.select(QStringLiteral("A2"));
    QTest::keyClick(e.view(), Qt::Key_Backspace);
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(editor->text(), QString());
    e.key(Qt::Key_Return);
    QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("-"));
}


namespace {
// The grid of the symbols of the subset chosen (16 characters to a row), not the table of special characters.
QTableWidget *symbolGrid(QDialog *dlg)
{
    for (QTableWidget *g : dlg->findChildren<QTableWidget *>())
        if (g->columnCount() == 16 && g->rowCount() > 1) return g;
    return nullptr;
}
} // namespace

// Insert > Symbols > Symbol opens the Symbol dialog; the character chosen goes at the end of the cell's own entry when none is open (the entry is opened), and at the cursor of
// an entry that is open; the entry is then finished as any other.
void CrunchEditTests::theSymbolDialogPutsACharacterInTheEntry()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QAction *symbol = e.win.w.act(QStringLiteral("insert.symbol"));
    QVERIFY(symbol);
    QCOMPARE(symbol->text(), QStringLiteral("Symbol"));
    auto insertI = [&] {
        withDialog([&] { symbol->trigger(); }, QStringLiteral("symbolDialog"), [](QDialog *dlg) {
            QTableWidget *grid = symbolGrid(dlg);
            QVERIFY(grid);
            QTableWidgetItem *it = grid->item(2, 9);   // (Basic Latin: row 2 starts at 0x40; column 9 is "I")
            QVERIFY(it);
            Q_EMIT grid->itemDoubleClicked(it);
        });
    };
    e.select(QStringLiteral("C1"));
    QVERIFY(!e.win.w.isEditing());
    insertI();
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("helloI"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("helloI"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:helloI"));
    // An entry already open takes it at its cursor.
    e.select(QStringLiteral("C2"));
    QTest::keyClick(e.view(), Qt::Key_A);
    QTest::keyClick(e.win.w.cellEditor(), Qt::Key_B);
    e.win.w.cellEditor()->setCursorPosition(1);
    insertI();
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("aIb"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("s:aIb"));
}

// The Symbol dialog takes the character from a font the person chooses: the character goes in a run of its own set in that font (the rest of the entry keeps the cell's font); with the
// font left at "(normal text)" the entry has no runs.
void CrunchEditTests::aSymbolFromAChosenFontIsARunInThatFont()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QAction *symbol = e.win.w.act(QStringLiteral("insert.symbol"));
    QVERIFY(symbol);
    QString family;
    auto insertI = [&](bool chooseFont) {
        withDialog([&] { symbol->trigger(); }, QStringLiteral("symbolDialog"), [&](QDialog *dlg) {
            auto *font = dlg->findChild<QComboBox *>();
            QVERIFY(font && font->count() > 1);
            if (chooseFont) {
                int at = 1;
                while (at + 1 < font->count() && (font->itemText(at) == QLatin1String("Calibri") || font->itemText(at) == QLatin1String("Arial") || font->itemText(at) == QLatin1String("Carlito"))) ++at;   // (not the cell's own)
                font->setCurrentIndex(at);
                family = font->currentText();
            }
            QTableWidget *grid = symbolGrid(dlg);
            QVERIFY(grid);
            QTableWidgetItem *it = grid->item(2, 9);
            QVERIFY(it);
            Q_EMIT grid->itemDoubleClicked(it);
        });
    };
    e.select(QStringLiteral("C1"));
    insertI(true);
    e.key(Qt::Key_Return);
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:helloI"));
    const auto rich = e.win.w.session()->richOf(0, 0, 2);
    QVERIFY(rich.has_value());
    QCOMPARE(rich->text, QStringLiteral("helloI"));
    QCOMPARE(rich->runs.size(), 2);
    QCOMPARE(rich->runs[1].start, quint32(5));
    QVERIFY(rich->runs[1].font < quint32(rich->fonts.size()));
    QCOMPARE(rich->fonts[int(rich->runs[1].font)].name, family);
    // (the first run is the cell's own font)
    QCOMPARE(rich->runs[0].font, jp::office::sheet::RichRun::kCellFont);
    e.select(QStringLiteral("C2"));
    QTest::keyClick(e.view(), Qt::Key_A);
    insertI(false);
    e.key(Qt::Key_Return);
    QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("s:aI"));
    QVERIFY(!e.win.w.session()->richOf(0, 1, 2).has_value());
}

// The dialog has a Special Characters tab: the characters the other programs list (dashes, nonbreaking marks, quotation marks) with their names; one chosen goes into the entry.
void CrunchEditTests::theSymbolDialogHasSpecialCharacters()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QAction *symbol = e.win.w.act(QStringLiteral("insert.symbol"));
    QVERIFY(symbol);
    e.select(QStringLiteral("C1"));
    withDialog([&] { symbol->trigger(); }, QStringLiteral("symbolDialog"), [&](QDialog *dlg) {
        auto *table = dlg->findChild<QTableWidget *>(QStringLiteral("specialTable"));
        QVERIFY(table);
        QVERIFY(table->rowCount() >= 8);
        int emDash = -1;
        for (int r = 0; r < table->rowCount(); ++r)
            if (table->item(r, 0)->text() == QString(QChar(0x2014))) emDash = r;
        QVERIFY(emDash >= 0);
        QVERIFY(!table->item(emDash, 1)->text().isEmpty());
        Q_EMIT table->itemDoubleClicked(table->item(emDash, 0));
    });
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("hello") + QChar(0x2014));
    e.key(Qt::Key_Return);
}

// A symbol can be given a shortcut key from the dialog; it is a command of the window with that key (so it shows on the Keyboard page, and Options can change it), it puts the symbol in the
// cell as the dialog does, and a window opened later has it.
void CrunchEditTests::aSymbolCanHaveAShortcutKey()
{
    const jptest::SettingGuard keys(QStringLiteral("keys/custom"), QString(), QString());
    const jptest::SettingGuard kept(QStringLiteral("symbols/keys"), QStringList(), QStringList());
    const QKeySequence key(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_J);
    QString id;
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QAction *symbol = e.win.w.act(QStringLiteral("insert.symbol"));
        QVERIFY(symbol);
        withDialog([&] { symbol->trigger(); }, QStringLiteral("symbolDialog"), [&](QDialog *dlg) {
            QTableWidget *grid = symbolGrid(dlg);
            QVERIFY(grid);
            grid->setCurrentItem(grid->item(2, 9));   // "I"
            auto *button = dlg->findChild<QPushButton *>(QStringLiteral("symbolKeyButton"));
            QVERIFY(button);
            QTimer answer;
            answer.setInterval(20);
            QObject::connect(&answer, &QTimer::timeout, [&] {
                auto *k = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!k || k->objectName() != QLatin1String("symbolKeyDialog")) return;
                k->findChild<QKeySequenceEdit *>(QStringLiteral("symbolKey"))->setKeySequence(key);
                k->accept();
            });
            answer.start();
            button->click();
        });
        for (const QString &name : e.win.w.actionIds())
            if (name.startsWith(QLatin1String("insert.symbolKey."))) id = name;
        QVERIFY(!id.isEmpty());
        QAction *a = e.win.w.act(id);
        QCOMPARE(a->shortcut().toString(QKeySequence::NativeText), key.toString(QKeySequence::NativeText));
        e.select(QStringLiteral("C1"));
        a->trigger();
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("helloI"));
        e.key(Qt::Key_Escape);
    }
    EditWindow later;
    QVERIFY(later.open(editBook()));
    QAction *a = later.win.w.act(id);
    QVERIFY(a);
    QCOMPARE(a->shortcut().toString(QKeySequence::NativeText), key.toString(QKeySequence::NativeText));
}

// Enter and Shift+Enter finish and go down and up; Tab and Shift+Tab right and left; arrows, while typing over a cell, finish and move on; Escape gives the entry up and the cell is as
// it was; Tab, Tab, Enter goes down to the column the Tabs began in.
void CrunchEditTests::theKeysThatFinishAnEntry()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("F5"));
    e.type(QStringLiteral("1"));
    e.key(Qt::Key_Return, Qt::ShiftModifier);
    QCOMPARE(e.selected(), QStringLiteral("F4"));
    e.type(QStringLiteral("2"));
    e.key(Qt::Key_Tab);
    QCOMPARE(e.selected(), QStringLiteral("G4"));
    e.type(QStringLiteral("3"));
    e.key(Qt::Key_Tab);
    QCOMPARE(e.selected(), QStringLiteral("H4"));
    e.type(QStringLiteral("4"));
    e.key(Qt::Key_Return);                        // the Tabs began in F: Enter goes down to F5
    QCOMPARE(e.selected(), QStringLiteral("F5"));
    e.type(QStringLiteral("5"));
    e.key(Qt::Key_Backtab, Qt::ShiftModifier);
    QCOMPARE(e.selected(), QStringLiteral("E5"));
    e.type(QStringLiteral("6"));
    e.key(Qt::Key_Down);
    QCOMPARE(e.selected(), QStringLiteral("E6"));
    e.type(QStringLiteral("7"));
    e.key(Qt::Key_Left);
    QCOMPARE(e.selected(), QStringLiteral("D6"));
    e.type(QStringLiteral("8"));
    e.key(Qt::Key_Up);
    QCOMPARE(e.selected(), QStringLiteral("D5"));
    e.type(QStringLiteral("9"));
    e.key(Qt::Key_Right);
    QCOMPARE(e.selected(), QStringLiteral("E5"));
    QCOMPARE(e.valueAt(QStringLiteral("F4")), QStringLiteral("n:2"));
    QCOMPARE(e.valueAt(QStringLiteral("G4")), QStringLiteral("n:3"));
    QCOMPARE(e.valueAt(QStringLiteral("H4")), QStringLiteral("n:4"));
    QCOMPARE(e.valueAt(QStringLiteral("F5")), QStringLiteral("n:5"));   // (the 1 typed there first was typed over)
    QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:6"));
    QCOMPARE(e.valueAt(QStringLiteral("E6")), QStringLiteral("n:7"));
    QCOMPARE(e.win.w.undoStack()->count(), 9);
    // Escape
    e.select(QStringLiteral("A1"));
    e.type(QStringLiteral("9"));
    QTest::keyClicks(e.win.w.cellEditor(), "99");
    e.key(Qt::Key_Escape);
    QVERIFY(!e.win.w.isEditing());
    QVERIFY(!e.win.w.cellEditor()->isVisible());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
    QCOMPARE(e.selected(), QStringLiteral("A1"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("1"));
    QCOMPARE(e.win.w.undoStack()->count(), 9);
    // with several cells selected, Enter goes through them
    e.view()->selectRange(10, 0, 11, 1);
    QCOMPARE(e.selected(), QStringLiteral("A11"));
    e.enter(QStringLiteral("1"));
    QCOMPARE(e.selected(), QStringLiteral("A12"));
    e.enter(QStringLiteral("2"));
    QCOMPARE(e.selected(), QStringLiteral("B11"));
}


// After Enter the selection goes where the option says (down, right, up, left, or nowhere); Shift+Enter goes the other way.
void CrunchEditTests::enterGoesWhereTheOptionSays()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    CrunchWindow &w = e.win.w;
    QCOMPARE(w.enterMove(), CrunchWindow::EnterMove::Down);
    const struct {
        CrunchWindow::EnterMove move;
        const char *after, *back;
    } cases[] = {{CrunchWindow::EnterMove::Right, "F5", "D5"}, {CrunchWindow::EnterMove::Up, "E4", "E6"}, {CrunchWindow::EnterMove::Left, "D5", "F5"}, {CrunchWindow::EnterMove::Stay, "E5", "E5"}};
    for (const auto &c : cases) {
        w.setEnterMove(c.move);
        e.select(QStringLiteral("E5"));
        e.enter(QStringLiteral("1"));
        QCOMPARE(e.selected(), QString::fromLatin1(c.after));
        e.select(QStringLiteral("E5"));
        e.type(QStringLiteral("2"));
        e.key(Qt::Key_Return, Qt::ShiftModifier);
        QCOMPARE(e.selected(), QString::fromLatin1(c.back));
    }
    w.setEnterMove(CrunchWindow::EnterMove::Down);
}


// AutoComplete: typing the start of a text that is in the column offers the rest of it, selected; Enter keeps it, a key replaces it, Backspace takes it away; two different texts that start
// alike offer nothing, nor does a number; and the option turns it off.
void CrunchEditTests::autoCompleteOffersTheRestOfAnEntryInTheColumn()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    CrunchWindow &w = e.win.w;
    w.setAutoComplete(true);
    e.select(QStringLiteral("E1"));
    e.enter(QStringLiteral("Banana"));
    QCOMPARE(e.selected(), QStringLiteral("E2"));
    e.type(QStringLiteral("ba"));
    QCOMPARE(w.cellEditor()->text(), QStringLiteral("Banana"));
    QCOMPARE(w.cellEditor()->selectedText(), QStringLiteral("nana"));
    e.key(Qt::Key_Backspace);
    QCOMPARE(w.cellEditor()->text(), QStringLiteral("Ba"));
    e.key(Qt::Key_Escape);
    e.select(QStringLiteral("E2"));
    e.type(QStringLiteral("Bax"));
    QCOMPARE(w.cellEditor()->text(), QStringLiteral("Bax"));
    e.key(Qt::Key_Escape);
    e.select(QStringLiteral("E2"));
    e.type(QStringLiteral("B"));
    e.key(Qt::Key_Return);
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("E2")), QStringLiteral("s:Banana"));
    // another text that starts alike: neither is offered
    e.select(QStringLiteral("E3"));
    e.enter(QStringLiteral("Bake"));
    e.select(QStringLiteral("E4"));
    e.type(QStringLiteral("Ba"));
    QCOMPARE(w.cellEditor()->text(), QStringLiteral("Ba"));
    e.key(Qt::Key_Escape);
    // a number is not offered
    e.select(QStringLiteral("F1"));
    e.enter(QStringLiteral("12"));
    e.select(QStringLiteral("F2"));
    e.type(QStringLiteral("1"));
    QCOMPARE(w.cellEditor()->text(), QStringLiteral("1"));
    e.key(Qt::Key_Escape);
    // turned off
    w.setAutoComplete(false);
    e.select(QStringLiteral("E5"));
    e.type(QStringLiteral("Bana"));
    QCOMPARE(w.cellEditor()->text(), QStringLiteral("Bana"));
    e.key(Qt::Key_Escape);
    w.setAutoComplete(true);
}


// The option that turns the fill handle and the dragging of cells off: with it off there is no handle at the corner of the selection.
void CrunchEditTests::theFillHandleCanBeTurnedOff()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    CrunchWindow &w = e.win.w;
    e.select(QStringLiteral("A1"));
    QVERIFY(!e.view()->fillHandleRect().isEmpty());
    w.setDragEditing(false);
    QVERIFY(!w.view()->dragEditing());
    QVERIFY(e.view()->fillHandleRect().isEmpty());
    w.setDragEditing(true);
    QVERIFY(!e.view()->fillHandleRect().isEmpty());
}


// Automatic decimal places: a whole number typed over a cell gets its point that many places from the right; negative places add zeros; anything else is entered as typed.
void CrunchEditTests::automaticDecimalPlacesPutThePointIn()
{
    using crunch::withFixedDecimals;
    const QString dot = QStringLiteral(".");
    QCOMPARE(withFixedDecimals(QStringLiteral("1234"), 2, dot), QStringLiteral("12.34"));
    QCOMPARE(withFixedDecimals(QStringLiteral("-5"), 2, dot), QStringLiteral("-0.05"));
    QCOMPARE(withFixedDecimals(QStringLiteral("5"), -2, dot), QStringLiteral("500"));
    QCOMPARE(withFixedDecimals(QStringLiteral("1.5"), 2, dot), QStringLiteral("1.5"));
    QCOMPARE(withFixedDecimals(QStringLiteral("=A1"), 2, dot), QStringLiteral("=A1"));
    QCOMPARE(withFixedDecimals(QStringLiteral("12"), 0, dot), QStringLiteral("12"));
    EditWindow e;
    QVERIFY(e.open(editBook()));
    CrunchWindow &w = e.win.w;
    w.setFixedDecimals(2);
    e.select(QStringLiteral("G1"));
    e.enter(QStringLiteral("1234"));
    e.select(QStringLiteral("G3"));
    e.enter(QStringLiteral("abc"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("G1")), QStringLiteral("n:12.34"));
    QCOMPARE(e.valueAt(QStringLiteral("G3")), QStringLiteral("s:abc"));
    w.setFixedDecimals(0);
    e.select(QStringLiteral("G5"));
    e.enter(QStringLiteral("1234"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("G5")), QStringLiteral("n:1234"));
}


// Show zeros: a sheet can hide the zeros its cells hold; the choice is the sheet's, is kept when the sheet is left and shown again, and is saved in the file and read back.
void CrunchEditTests::zerosCanBeHiddenOnASheetAndAreSavedThatWay()
{
    EditWindow e;
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>0</v></c><c r=\"B1\"><v>5</v></c></row>");
    QVERIFY(e.open(buildXlsx({worksheet(rows), worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), {QStringLiteral("One"), QStringLiteral("Two")})));
    QVERIFY(e.settle());
    CrunchWindow &w = e.win.w;
    QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("0"));
    w.setShowZeros(false);
    QCOMPARE(e.shownAt(QStringLiteral("A1")), QString());
    QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("5"));
    // another sheet keeps its zeros, and this one has none when it comes back
    w.setActiveSheet(1);
    QVERIFY(w.showZeros());
    w.setActiveSheet(0);
    QVERIFY(!w.showZeros());
    QSignalSpy saved(&w, &CrunchWindow::saved);
    QVERIFY(w.save());
    QVERIFY2(savedOk(saved), "the save failed");
    QFile f(e.path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(f.readAll(), parts));
    QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("showZeros=\"0\""));
    QVERIFY(!parts.value(QStringLiteral("xl/worksheets/sheet2.xml")).contains("showZeros"));
    f.seek(0);
    EditWindow again;
    QVERIFY(again.open(f.readAll(), QStringLiteral("zeros.xlsx")));
    QVERIFY(!again.win.w.showZeros());
    QCOMPARE(again.shownAt(QStringLiteral("A1")), QString());
}


// Ctrl+Enter puts what was typed in every selected cell (a formula shifts its relative references, as a copy does) as one step of Undo, and the selection stays.
void CrunchEditTests::ctrlEnterFillsTheSelection()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.view()->selectRange(5, 4, 6, 5);   // E6:F7, the cursor in E6
    e.type(QStringLiteral("7"));
    const int steps = e.win.w.undoStack()->count();
    e.key(Qt::Key_Return, Qt::ControlModifier);
    QVERIFY(!e.win.w.isEditing());
    for (const char *ref : {"E6", "F6", "E7", "F7"}) QCOMPARE(e.valueAt(QString::fromLatin1(ref)), QStringLiteral("n:7"));
    QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
    QCOMPARE(e.view()->rangeName(), QStringLiteral("E6:F7"));
    pressUndo(e.view());
    for (const char *ref : {"E6", "F6", "E7", "F7"}) QCOMPARE(e.valueAt(QString::fromLatin1(ref)), QStringLiteral("-"));
    // a formula: the cell of each is the same distance from the cell it refers to
    e.view()->selectRange(5, 4, 6, 5);
    e.type(QStringLiteral("=A1"));
    e.key(Qt::Key_Return, Qt::ControlModifier);
    e.select(QStringLiteral("F7"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=B2"));
    e.select(QStringLiteral("E6"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=A1"));
    // one cell selected: Ctrl+Enter is the same as Enter, and stays
    e.select(QStringLiteral("H1"));
    e.type(QStringLiteral("3"));
    e.key(Qt::Key_Return, Qt::ControlModifier);
    QCOMPARE(e.valueAt(QStringLiteral("H1")), QStringLiteral("n:3"));
    QCOMPARE(e.selected(), QStringLiteral("H1"));
}


// Ctrl+Enter puts the entry in the top left cell of a merged area and leaves the other cells of the area as they are (they are hidden by it).
void CrunchEditTests::ctrlEnterKeepsToTheTopLeftCellOfAMergedArea()
{
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"2\"><c r=\"C2\"><v>1</v></c></row>"), QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"B2:C3\"/></mergeCells>"))})));
    e.view()->selectRange(1, 0, 3, 3);   // A2:D4
    e.type(QStringLiteral("7"));
    e.key(Qt::Key_Return, Qt::ControlModifier);
    for (const char *ref : {"A2", "A3", "A4", "B2", "D2", "D3", "D4", "B4", "C4"}) QCOMPARE(e.valueAt(QString::fromLatin1(ref)), QStringLiteral("n:7"));
    QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("-"));   // (hidden by the area)
    QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("-"));
    QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:1"));  // (and so is C2, which held something before)
}


// Ctrl+; puts today's date in the cell and Ctrl+Shift+; the time now, each as a number the cell shows as a date or a time; while an entry is typed they are put in the entry.
void CrunchEditTests::todaysDateAndTheTimeByKeys()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QVERIFY(e.win.w.act(QStringLiteral("edit.insertDate")));
    QCOMPARE(e.win.w.act(QStringLiteral("edit.insertDate"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_Semicolon));
    QCOMPARE(e.win.w.act(QStringLiteral("edit.insertTime"))->shortcut(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Semicolon));
    const qint64 before = QDate(1899, 12, 30).daysTo(QDate::currentDate());
    e.select(QStringLiteral("F9"));
    e.win.w.act(QStringLiteral("edit.insertDate"))->trigger();
    const qint64 after = QDate(1899, 12, 30).daysTo(QDate::currentDate());
    const QString got = e.valueAt(QStringLiteral("F9"));
    QVERIFY2(got == QStringLiteral("n:%1").arg(before) || got == QStringLiteral("n:%1").arg(after), qPrintable(got));
    QVERIFY(e.win.w.undoStack()->count() == 1);
    QVERIFY(!e.shownAt(QStringLiteral("F9")).isEmpty() && e.shownAt(QStringLiteral("F9")) != QStringLiteral("%1").arg(before));
    e.select(QStringLiteral("F10"));
    e.win.w.act(QStringLiteral("edit.insertTime"))->trigger();
    const QString t = e.valueAt(QStringLiteral("F10"));
    QVERIFY2(t.startsWith(QStringLiteral("n:0.")) || t == QStringLiteral("n:0"), qPrintable(t));
    QVERIFY(e.shownAt(QStringLiteral("F10")).contains(QLatin1Char(':')));
    // while typing: the date goes into the entry, and the cell is not changed until Enter
    e.select(QStringLiteral("F11"));
    e.type(QStringLiteral("x"));
    e.win.w.act(QStringLiteral("edit.insertDate"))->trigger();
    QVERIFY(e.win.w.cellEditor()->text().size() > 1);
    QVERIFY(e.win.w.cellEditor()->text().startsWith(QLatin1Char('x')));
    e.key(Qt::Key_Escape);
}


// Alt+Down on a cell with no list of its own lists the words already in its column (each once, in order), and choosing one types it.
void CrunchEditTests::altDownListsTheColumnsValues()
{
    auto word = [](int row, const char *w) { return QStringLiteral("<row r=\"%1\"><c r=\"J%1\" t=\"inlineStr\"><is><t>%2</t></is></c></row>").arg(row).arg(QString::fromLatin1(w)).toUtf8(); };
    EditWindow e;
    QVERIFY(e.open(editBook(word(10, "pear") + word(11, "apple") + word(12, "pear") + word(13, "fig"))));
    e.select(QStringLiteral("J14"));
    QTest::keyClick(e.view(), Qt::Key_Down, Qt::AltModifier);
    QMenu *menu = e.win.w.findChild<QMenu *>(QStringLiteral("listMenu"));
    QVERIFY(menu);
    QStringList items;
    for (QAction *a : menu->actions()) items << a->text();
    QCOMPARE(items, (QStringList{"apple", "fig", "pear"}));
    menu->actions()[1]->trigger();
    menu->close();
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("J14")), QStringLiteral("s:fig"));
    // a column with no words: a note says so
    e.select(QStringLiteral("Z1"));
    QTest::keyClick(e.view(), Qt::Key_Down, Qt::AltModifier);
    QMenu *none = e.win.w.findChild<QMenu *>(QStringLiteral("listMenu"));
    QVERIFY(none);
    QCOMPARE(none->actions().size(), 1);
    QVERIFY(!none->actions().first()->isEnabled());
    none->close();
}


// Enter after a run of Tabs goes down to the column the run began in; an arrow, a click, or Go To ends the run; Enter with no Tabs goes straight down.
void CrunchEditTests::enterAfterTabsGoesBackToTheStartColumn()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("C3"));
    e.type(QStringLiteral("1"));
    e.key(Qt::Key_Tab);
    e.type(QStringLiteral("2"));
    e.key(Qt::Key_Tab);
    e.type(QStringLiteral("3"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("C4"));
    // an arrow in between ends the run
    e.type(QStringLiteral("1"));
    e.key(Qt::Key_Tab);
    QCOMPARE(e.selected(), QStringLiteral("D4"));
    QTest::keyClick(e.view(), Qt::Key_Right);
    QCOMPARE(e.selected(), QStringLiteral("E4"));
    e.type(QStringLiteral("2"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("E5"));
    // Enter with no Tabs goes straight down
    e.type(QStringLiteral("3"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("E6"));
    // Shift+Tab goes left inside the run, and Enter goes to where the run began
    e.type(QStringLiteral("4"));
    e.key(Qt::Key_Tab);
    e.type(QStringLiteral("5"));
    e.key(Qt::Key_Tab);
    e.type(QStringLiteral("6"));
    e.key(Qt::Key_Backtab, Qt::ShiftModifier);
    QCOMPARE(e.selected(), QStringLiteral("F6"));
    e.type(QStringLiteral("7"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("E7"));
    // a click ends the run
    e.type(QStringLiteral("8"));
    e.key(Qt::Key_Tab);
    e.select(QStringLiteral("H2"));
    e.type(QStringLiteral("9"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("H3"));
}


// Moving without typing counts too: Tab, Tab, Enter on the grid goes down to the column the Tabs began in, and an arrow ends the run.
void CrunchEditTests::enterAfterMovingByTabsGoesBackToTheStartColumn()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("C3"));
    QTest::keyClick(e.view(), Qt::Key_Tab);
    QTest::keyClick(e.view(), Qt::Key_Tab);
    QCOMPARE(e.selected(), QStringLiteral("E3"));
    QTest::keyClick(e.view(), Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("C4"));
    QTest::keyClick(e.view(), Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("C5"));
    QTest::keyClick(e.view(), Qt::Key_Tab);
    QTest::keyClick(e.view(), Qt::Key_Backtab, Qt::ShiftModifier);
    QTest::keyClick(e.view(), Qt::Key_Tab);
    QTest::keyClick(e.view(), Qt::Key_Down);
    QCOMPARE(e.selected(), QStringLiteral("D6"));
    QTest::keyClick(e.view(), Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("D7"));
    // typing after moving by Tab keeps the run
    QTest::keyClick(e.view(), Qt::Key_Tab);
    e.type(QStringLiteral("1"));
    e.key(Qt::Key_Tab);
    e.type(QStringLiteral("2"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("D8"));
    // a range keeps the way Tab and Enter go inside it
    e.view()->selectRange(10, 0, 11, 1);
    QTest::keyClick(e.view(), Qt::Key_Tab);
    QCOMPARE(e.selected(), QStringLiteral("B11"));
    QTest::keyClick(e.view(), Qt::Key_Return);
    QCOMPARE(e.selected(), QStringLiteral("B12"));
}


// F2 and a double click edit what the cell holds, in the cell, with the cursor at the end; the arrows then move in the text (they do not finish the entry), and F2 changes that.
void CrunchEditTests::f2AndADoubleClickEditWhatTheCellHolds()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("B1"));
    QTest::keyClick(e.view(), Qt::Key_F2);
    QVERIFY(e.win.w.isEditing());
    CellEditor *editor = e.win.w.cellEditor();
    QCOMPARE(editor->text(), QStringLiteral("=A1*2"));
    QCOMPARE(editor->cursorPosition(), 5);
    QVERIFY(editor->mode() == CellEditor::Mode::Editing);
    QTest::keyClick(editor, Qt::Key_Left);
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.selected(), QStringLiteral("B1"));
    QCOMPARE(editor->cursorPosition(), 4);
    QTest::keyClicks(editor, "+1");
    QCOMPARE(editor->text(), QStringLiteral("=A1*+12"));
    QTest::keyClick(editor, Qt::Key_Escape);
    QVERIFY(!e.win.w.isEditing());
    // the double click, on a cell
    const QRect r = e.view()->cellRect(2, 0);   // A3: a percent
    QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, r.center());
    QCOMPARE(e.selected(), QStringLiteral("A3"));
    QVERIFY(!e.win.w.isEditing());
    QTest::mouseDClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, r.center());
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.selected(), QStringLiteral("A3"));
    QCOMPARE(editor->text(), QStringLiteral("25%"));
    QTest::keyClick(editor, Qt::Key_Escape);
    // F2 toggles what an arrow does
    e.select(QStringLiteral("A1"));
    e.type(QStringLiteral("5"));
    QVERIFY(editor->mode() == CellEditor::Mode::Typing);
    QTest::keyClick(editor, Qt::Key_F2);
    QVERIFY(editor->mode() == CellEditor::Mode::Editing);
    QTest::keyClick(editor, Qt::Key_Down);
    QVERIFY(e.win.w.isEditing());
    QTest::keyClick(editor, Qt::Key_Escape);
    // a text that would be read as a number is edited with an apostrophe
    e.select(QStringLiteral("C3"));
    QTest::keyClick(e.view(), Qt::Key_F2);
    QCOMPARE(editor->text(), QStringLiteral("7"));   // (a number in a Text-formatted cell is a number: C3 holds 7 as a number)
    QTest::keyClick(editor, Qt::Key_Escape);
}


// The formula bar is where an entry can be made too: typing in it starts one, Enter finishes it and goes down, Escape gives it up.
void CrunchEditTests::theFormulaBarEdits()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("A2"));
    QLineEdit *bar = e.win.w.formulaBar();
    QVERIFY(!bar->isReadOnly());
    QCOMPARE(bar->text(), QStringLiteral("10"));
    bar->setFocus();
    bar->selectAll();
    QTest::keyClicks(bar, "77");
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("77"));
    QTest::keyClick(bar, Qt::Key_Return);
    QVERIFY(!e.win.w.isEditing());
    QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:77"));
    QCOMPARE(e.selected(), QStringLiteral("A3"));
    // Escape
    bar->setFocus();
    bar->selectAll();
    QTest::keyClicks(bar, "5");
    QVERIFY(e.win.w.isEditing());
    QTest::keyClick(bar, Qt::Key_Escape);
    QVERIFY(!e.win.w.isEditing());
    QCOMPARE(bar->text(), QStringLiteral("25%"));
    QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:0.25"));
    // what is typed in the cell shows in the bar, and what is typed in the bar in the cell
    e.select(QStringLiteral("E1"));
    e.type(QStringLiteral("a"));
    QCOMPARE(bar->text(), QStringLiteral("a"));
    e.key(Qt::Key_Escape);
}


// F2 and then Enter (or a click on another cell) with nothing changed puts nothing in: a step is not made, the file has nothing to save, and a formula with a function this program lacks
// keeps the result the file stored (it was written over as this program reads it back: the prefix the format needs was dropped, and the cell had #NAME?).
void CrunchEditTests::anEntryThatWasNotChangedCommitsNothing()
{
    const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>_xludf.MYFUNC(B1)</f><v>77</v></c><c r=\"B1\"><v>5</v></c><c r=\"C1\"><f>_xlfn.IMAGE(\"x\")</f><v>88</v></c>"
                                                              "<c r=\"D1\" s=\"1\"><v>45293</v></c><c r=\"E1\"><f>B1*2</f><v>10</v></c></row>"))},
                                       QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"14\" applyNumberFormat=\"1\"/>")));
    EditWindow e;
    QVERIFY(e.open(book));
    for (const QString &ref : {QStringLiteral("A1"), QStringLiteral("C1"), QStringLiteral("D1"), QStringLiteral("E1")}) {
        e.select(ref);
        QTest::keyClick(e.view(), Qt::Key_F2);
        QVERIFY2(e.win.w.isEditing(), qPrintable(ref));
        e.key(Qt::Key_Return);
        QVERIFY2(!e.win.w.isEditing(), qPrintable(ref));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY(!e.win.w.isModified());
    }
    // (and a click on another cell, with the formula bar's text as it was)
    e.select(QStringLiteral("C1"));
    QTest::keyClick(e.view(), Qt::Key_F2);
    QVERIFY(e.win.w.isEditing());
    QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->cellRect(0, 1).center());
    QVERIFY(!e.win.w.isEditing());
    QCOMPARE(e.selected(), QStringLiteral("B1"));
    QCOMPARE(e.win.w.undoStack()->count(), 0);
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:77"));
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:88"));
    QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:10"));
    // a change is a change
    e.select(QStringLiteral("E1"));
    QTest::keyClick(e.view(), Qt::Key_F2);
    QTest::keyClicks(e.win.w.cellEditor(), QStringLiteral("+1"));
    e.key(Qt::Key_Return);
    QCOMPARE(e.win.w.undoStack()->count(), 1);
}


// A formula typed with a function the format prefixes (that this program lacks) is kept as a file has it: with the prefix, so that the other programs know the function; the cell
// shows #NAME? here, as it says.
void CrunchEditTests::aFormulaTypedWithAPrefixedFunctionKeepsItsPrefixInTheFile()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("F1"));
    e.enter(QStringLiteral("=image(\"x\")"));
    e.select(QStringLiteral("F2"));
    e.enter(QStringLiteral("=let(x,A1,x*2)"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("e:#NAME?"));
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(savedFormula(e.path, QStringLiteral("F1")), QStringLiteral("_xlfn.IMAGE(\"x\")"));
    QCOMPARE(savedFormula(e.path, QStringLiteral("F2")), QStringLiteral("_xlfn.LET(_xlpm.x,A1,_xlpm.x*2)"));
    // and F2 shows it as it was typed
    Window second;
    QVERIFY(second.open(e.copyOfFile()));
    second.w.view()->selectCell(1, 5);
    QCOMPARE(second.w.formulaBar()->text(), QStringLiteral("=LET(x,A1,x*2)"));
}


// Clear All on cells that hold nothing changes nothing, so there is nothing for the worker to work out: Save writes the file at once.
void CrunchEditTests::clearAllOnEmptyCellsLeavesNothingForTheWorker()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("100"));
    QVERIFY(e.settle());
    e.select(QStringLiteral("F20"));
    e.win.w.act(QStringLiteral("edit.clear.all"))->trigger();
    QVERIFY(!e.win.w.isCalculating());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:100"));
}

// A close that waits for a save which then finds nothing to save (the change was undone while the calculation went) goes on: nothing is lost. A save that fails gives the close up, and a
// later Save does not close the window.
void CrunchEditTests::aSaveThatDoesNothingWhenResumedDoesNotLeaveTheWindowToClose()
{
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Save; });
        w.setWorkerPace(20000);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("100"));
        QVERIFY(w.isCalculating());
        w.close();                                                                  // (asks, is told Save, and the save waits for the calculation)
        QVERIFY(w.isVisible());
        pressUndo(&w);
        QVERIFY(!w.session()->isModified());
        w.setWorkerPace(0);
        QTRY_VERIFY_WITH_TIMEOUT(!w.isVisible(), 20000);
    }
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Save; });
        w.setChangedAsker([](const QString &) { return CrunchWindow::ChangedAnswer::Cancel; });
        w.setWorkerPace(20000);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("100"));
        QVERIFY(w.isCalculating());
        w.close();
        QVERIFY(w.isVisible());
        QFile other(e.path);                                                        // (another program changes the file: the save is refused)
        QVERIFY(other.open(QIODevice::WriteOnly));
        other.write(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>7</v></c></row>"))}));
        other.close();
        w.setWorkerPace(0);
        QVERIFY(e.settle());
        QVERIFY(w.session()->isModified());
        QVERIFY(w.isVisible());
        w.setChangedAsker([](const QString &) { return CrunchWindow::ChangedAnswer::Overwrite; });
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QVERIFY(w.isVisible());
    }
}

// Save a Copy writes the workbook to another file and goes on with the original: the window keeps its name and its unsaved changes, and the copy has them.
void CrunchEditTests::saveACopyKeepsWorkingInTheOriginal()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    const QString copy = e.folder.dir.filePath(QStringLiteral("book copy.xlsx"));
    e.win.w.setSavePicker([&](const QString &, const QString &) { return copy; });
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("42"));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.isModified());
    QSignalSpy made(&e.win.w, &CrunchWindow::copyExported);
    QVERIFY(e.win.w.act(QStringLiteral("file.saveCopy"))->isEnabled());
    QVERIFY(e.win.w.saveCopy());
    QTRY_COMPARE_WITH_TIMEOUT(made.count(), 1, 20000);
    QVERIFY(made.first().at(1).toBool());
    QVERIFY(e.win.w.isModified());
    QCOMPARE(e.win.w.filePath(), e.path);
    QCOMPARE(savedCell(copy, QStringLiteral("A1")), QStringLiteral("n:42"));
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:1"));   // (the original file is as it was)
    // The window can still save itself over its own file.
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:42"));
    // A copy is never written over the workbook's own file.
    QStringList told;
    e.win.w.setNotifier([&](const QString &, const QString &text) { told << text; });
    e.win.w.setSavePicker([&](const QString &, const QString &) { return e.path; });
    QVERIFY(!e.win.w.saveCopy());
    QCOMPARE(told.size(), 1);
}


// With "Always create backup copy" on, a save keeps the file as it was as "Backup of <name>".
void CrunchEditTests::aBackupCopyIsKeptOnSave()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    const QString backup = e.folder.dir.filePath(QStringLiteral("Backup of book.xlsx"));
    auto saveWith = [&](const QString &value) {
        e.select(QStringLiteral("A1"));
        e.enter(value);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
    };
    saveWith(QStringLiteral("2"));
    QVERIFY(!QFileInfo::exists(backup));   // (the option is off)
    {
        jptest::SettingGuard keep(QStringLiteral("save/backup"), true, false);
        saveWith(QStringLiteral("3"));
    }
    QVERIFY(QFileInfo::exists(backup));
    QCOMPARE(savedCell(backup, QStringLiteral("A1")), QStringLiteral("n:2"));
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:3"));
}


// Open Read-Only: no cell can be changed (typing, commands), Save asks for a new name, and the new file is an ordinary workbook.
void CrunchEditTests::aWorkbookOpenedReadOnlyCannotBeChanged()
{
    EditWindow e;
    e.path = e.folder.write(QStringLiteral("book.xlsx"), editBook());
    QVERIFY(e.win.open(e.path, OpenHow::ReadOnly));
    QVERIFY(e.win.w.isReadOnly());
    QVERIFY(e.win.w.windowTitle().contains(QLatin1String("Read-Only")));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("5"));
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
    QVERIFY(!e.win.w.isModified());
    // Save asks for another name; the file it was opened from is as it was.
    const QString other = e.folder.dir.filePath(QStringLiteral("changed.xlsx"));
    e.win.w.setSavePicker([&](const QString &, const QString &) { return other; });
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(e.win.w.filePath(), other);
    QVERIFY(!e.win.w.isReadOnly());
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("5"));
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:5"));
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:1"));
}


// In a workbook open for reading only, Replace and Replace All (the dialog is there for Find) change nothing, and the commands that change the workbook are refused.
void CrunchEditTests::aReadOnlyWorkbookIsNotChangedByReplace()
{
    EditWindow e;
    e.path = e.folder.write(QStringLiteral("book.xlsx"), editBook());
    QVERIFY(e.win.open(e.path, OpenHow::ReadOnly));
    CrunchWindow &w = e.win.w;
    QVERIFY(!w.act(QStringLiteral("find.replace"))->isEnabled());
    QVERIFY(!CrunchWindow::commandAllowedReadOnly(QStringLiteral("find.replace")));
    const QString before = w.session()->editText(0, 0, 2);
    QCOMPARE(before, QStringLiteral("hello"));
    const int steps = w.undoStack()->count();
    w.act(QStringLiteral("find.replace"))->trigger();
    w.act(QStringLiteral("edit.find"))->trigger();   // (Find is allowed: its dialog has the Change buttons)
    FindDialog *d = w.findDialog();
    QVERIFY(d);
    d->setTab(FindDialog::Tab::Replace);
    d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("hello"));
    d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("bye"));
    e.select(QStringLiteral("C1"));
    d->findChild<QPushButton *>(QStringLiteral("replaceOne"))->click();
    d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
    QTest::qWait(300);
    QVERIFY(e.settle());
    QCOMPARE(w.session()->editText(0, 0, 2), before);
    QCOMPARE(w.undoStack()->count(), steps);
    QVERIFY(!w.isModified());
    // The commands that change it are off, or refused, one by one.
    for (const QString &id : w.actionIds())
        if (!CrunchWindow::commandAllowedReadOnly(id)) w.act(id)->trigger();
    QCOMPARE(w.undoStack()->count(), steps);
    QVERIFY(!w.isModified());
    QCOMPARE(w.session()->editText(0, 0, 2), before);
}


// The package kept for the reader of an Open and Repair is removed when the read is over, when the window goes before then, and when the repair's own load fails: tied to an object, not to one path.
void CrunchEditTests::theRepairFileGoesWithTheWindowOnEveryPath()
{
    EditWindow e;
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(editBook(), parts));
    const QString sheet = QStringLiteral("xl/worksheets/sheet1.xml");
    parts[sheet].truncate(parts[sheet].indexOf("<row r=\"3\"") + 20);
    jp::ZipWriter z;
    z.add(QStringLiteral("[Content_Types].xml"), parts.take(QStringLiteral("[Content_Types].xml")), true);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) z.add(it.key(), it.value(), true);
    const QString path = e.folder.write(QStringLiteral("cut.xlsx"), z.finish());
    // (1) read to the end: the file is gone
    QVERIFY(e.win.open(path, OpenHow::Repair));
    const QString made = e.win.w.lastRepairFileForTest();
    QVERIFY(!made.isEmpty());
    QVERIFY(!QFileInfo::exists(made));
    // (2) the window closes while the read is still running: the file exists until then, and goes with the window
    QString second;
    {
        CrunchWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        QVERIFY(w.openFile(path, OpenHow::Repair));
        QVERIFY(w.isLoading());   // (no event has run since: the read has not ended)
        second = w.lastRepairFileForTest();
        QVERIFY(!second.isEmpty() && second != made);
        QVERIFY(QFileInfo::exists(second));
    }
    QVERIFY(!QFileInfo::exists(second));
    // (3) the read fails: the file goes (the package has a workbook with no sheet in it)
    {
        CrunchWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        QSignalSpy loaded(&w, &CrunchWindow::loadFinished);
        jp::ZipWriter bz;
        bz.add(QStringLiteral("xl/workbook.xml"), "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheets/></workbook>", true);
        const QString badPath = e.folder.write(QStringLiteral("bad.xlsx"), bz.finish());
        QVERIFY(w.openFile(badPath, OpenHow::Repair));
        const QString third = w.lastRepairFileForTest();
        QVERIFY(!third.isEmpty());
        QVERIFY(QFileInfo::exists(third));
        QVERIFY(loaded.count() > 0 || loaded.wait(20000));
        QVERIFY2(!loaded.last().first().toBool(), "the read was expected to fail");
        QVERIFY(!QFileInfo::exists(third));
    }
    // (4) another open while the read goes on (it opens in a window of its own, as a window that is reading keeps its workbook): the first window's file stays until its read ends, then goes
    {
        CrunchWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        QSignalSpy loaded(&w, &CrunchWindow::loadFinished);
        QVERIFY(w.openFile(path, OpenHow::Repair));
        const QString one = w.lastRepairFileForTest();
        QVERIFY(QFileInfo::exists(one));
        QFile cutFile(path);
        QVERIFY(cutFile.open(QIODevice::ReadOnly));
        const QString otherPath = e.folder.write(QStringLiteral("cut2.xlsx"), cutFile.readAll());
        const int windows = AppWindow::windows().size();
        QVERIFY(w.openFile(otherPath, OpenHow::Repair));
        QCOMPARE(AppWindow::windows().size(), windows + 1);
        QCOMPARE(w.lastRepairFileForTest(), one);
        QVERIFY(QFileInfo::exists(one));
        QString two;
        for (AppWindow *other : AppWindow::windows())
            if (auto *c = qobject_cast<CrunchWindow *>(other); c && c != &w && !c->lastRepairFileForTest().isEmpty()) two = c->lastRepairFileForTest();
        QVERIFY(!two.isEmpty() && two != one);
        QVERIFY(loaded.count() > 0 || loaded.wait(20000));
        QVERIFY(!QFileInfo::exists(one));
        QTRY_VERIFY_WITH_TIMEOUT(!QFileInfo::exists(two), 20000);
        for (AppWindow *other : AppWindow::windows())
            if (auto *c = qobject_cast<CrunchWindow *>(other); c && c != &w && c != &e.win.w) {
                c->setAttribute(Qt::WA_DeleteOnClose, false);
                delete c;
            }
    }
}


// Open and Repair reads the file whole, so a file larger than the apps open is refused before anything is read, and the person is told.
void CrunchEditTests::aRepairOfAFileLargerThanTheMostIsRefused()
{
    EditWindow e;
    const QString path = e.folder.dir.filePath(QStringLiteral("huge.xlsx"));
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QVERIFY(f.resize(jp::office::kMaxOpenFileBytes + 1));   // (a sparse file: nothing is stored)
    }
    QStringList told;
    e.win.w.setNotifier([&](const QString &t, const QString &x) { told << t + QStringLiteral(": ") + x; });
    QVERIFY(!e.win.w.openFile(path, OpenHow::Repair));
    QVERIFY(!told.isEmpty());
    QVERIFY2(told.last().contains(QLatin1String("larger")), qPrintable(told.last()));
    QVERIFY(e.win.w.lastRepairFileForTest().isEmpty());
}


// A template made from a workbook with macros is a plain template: no macro project, the plain template type, and the person is told first, as Save As tells.
void CrunchEditTests::aTemplateMadeFromAMacroWorkbookHasNoMacros()
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
    jptest::TestModeGuard testMode;
    QDir(myTemplatesFolder()).removeRecursively();
    EditWindow e;
    QVERIFY(e.open(macroBook, QStringLiteral("macros.xlsm")));
    setTemplateNameStandIn([](const QString &) { return QStringLiteral("Macros"); });
    QSignalSpy made(&e.win.w, &CrunchWindow::copyExported);
    const bool started = e.win.w.saveAsTemplate();
    setTemplateNameStandIn({});
    QVERIFY(started);
    QTRY_COMPARE_WITH_TIMEOUT(made.count(), 1, 20000);
    QVERIFY(made.first().at(1).toBool());
    QVERIFY(e.win.told.last().contains(QLatin1String("macros")));
    const QStringList found = myTemplateFiles({QStringLiteral("*.xltx")});
    QCOMPARE(found.size(), 1);
    QFile f(found.first());
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray bytes = f.readAll();
    f.close();
    QVERIFY(!rawParts(bytes).contains(QStringLiteral("xl/vbaProject.bin")));
    const QByteArray types = entriesOf(bytes).value(QStringLiteral("[Content_Types].xml"));
    QVERIFY(!types.contains("macroEnabled"));
    QVERIFY(types.contains("spreadsheetml.template.main+xml"));
    QVERIFY(!entriesOf(bytes).value(QStringLiteral("xl/_rels/workbook.xml.rels")).contains("vbaProject"));
    QVERIFY(removeMyTemplate(found.first()));
}


// A backup that can't be made asks before the save goes on (not saving is the default): the workbook's file is as it was when the answer is no.
void CrunchEditTests::aBackupThatFailsAsksBeforeTheWorkbookIsSaved()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QFile before(e.path);
    QVERIFY(before.open(QIODevice::ReadOnly));
    const QByteArray original = before.readAll();
    before.close();
    jptest::SettingGuard keep(QStringLiteral("save/backup"), true, false);
    struct Reset {
        ~Reset() { setBackupFailureStandIn({}); }
    } reset;
    QVERIFY(QDir().mkpath(backupPathFor(e.path)));   // (a folder where the backup would go)
    QStringList asked;
    setBackupFailureStandIn([&](const QString &p) {
        asked << p;
        return false;
    });
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("5"));
    QVERIFY(e.settle());
    QVERIFY(!e.win.w.save());
    QCOMPARE(asked.size(), 1);
    QVERIFY(!e.win.w.isSaving());
    QFile after(e.path);
    QVERIFY(after.open(QIODevice::ReadOnly));
    QCOMPARE(after.readAll(), original);
    QVERIFY(e.win.w.isModified());
    after.close();
    // Saying yes saves.
    setBackupFailureStandIn([&](const QString &) { return true; });
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:5"));
}


// Open as Copy: the workbook has no file of its own, so Save asks where, and the file it was read from is never written.
void CrunchEditTests::aWorkbookOpenedAsACopyHasNoFile()
{
    EditWindow e;
    e.path = e.folder.write(QStringLiteral("book.xlsx"), editBook());
    QVERIFY(e.win.open(e.path, OpenHow::Copy));
    QVERIFY(e.win.w.filePath().isEmpty());
    QVERIFY(!e.win.w.isReadOnly());
    QVERIFY(e.win.w.displayName().contains(QLatin1String("book")));
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("7"));
    QVERIFY(e.settle());
    const QString other = e.folder.dir.filePath(QStringLiteral("second.xlsx"));
    e.win.w.setSavePicker([&](const QString &, const QString &) { return other; });
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(e.win.w.filePath(), other);
    QCOMPARE(savedCell(other, QStringLiteral("A1")), QStringLiteral("n:7"));
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:1"));
}


// Open in New Window: a window that holds a new workbook nobody has touched stays as it is, and the file opens in a window of its own.
void CrunchEditTests::aWorkbookCanBeOpenedInANewWindow()
{
    EditWindow e;
    const QString path = e.folder.write(QStringLiteral("book.xlsx"), editBook());
    QVERIFY(e.openBlank());
    const int before = AppWindow::windows().size();
    QVERIFY(e.win.w.openFile(path, OpenHow::NewWindow));
    QCOMPARE(AppWindow::windows().size(), before + 1);
    QVERIFY(e.win.w.filePath().isEmpty());
    CrunchWindow *opened = nullptr;
    for (AppWindow *other : AppWindow::windows())
        if (auto *w = qobject_cast<CrunchWindow *>(other); w && w != &e.win.w && w->filePath() == path) opened = w;
    QVERIFY(opened);
    QTRY_VERIFY_WITH_TIMEOUT(opened->session() != nullptr, 20000);
    QTRY_VERIFY_WITH_TIMEOUT(!opened->isCalculating(), 20000);
    opened->close();
}


// Save as Template puts an .xltx in My Templates (with a picture) and leaves the workbook as it was; a workbook made from it has its cells and no file.
void CrunchEditTests::saveAsTemplatePutsAWorkbookInMyTemplates()
{
    jptest::TestModeGuard testMode;
    QDir(myTemplatesFolder()).removeRecursively();
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("42"));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.isModified());
    setTemplateNameStandIn([](const QString &) { return QStringLiteral("Budget"); });
    QSignalSpy made(&e.win.w, &CrunchWindow::copyExported);
    QVERIFY(e.win.w.act(QStringLiteral("file.saveTemplate"))->isEnabled());
    const bool started = e.win.w.saveAsTemplate();
    setTemplateNameStandIn({});
    QVERIFY(started);
    QTRY_COMPARE_WITH_TIMEOUT(made.count(), 1, 20000);
    QVERIFY(made.first().at(1).toBool());
    QVERIFY(e.win.w.isModified());
    QCOMPARE(e.win.w.filePath(), e.path);
    const QStringList found = myTemplateFiles({QStringLiteral("*.xltx")});
    QCOMPARE(found.size(), 1);
    QCOMPARE(QFileInfo(found.first()).fileName(), QStringLiteral("Budget.xltx"));
    QVERIFY(!templateThumbnail(found.first()).isNull());
    QCOMPARE(savedCell(found.first(), QStringLiteral("A1")), QStringLiteral("n:42"));
    {
        Package p;
        QString why;
        QVERIFY(p.readLazyFile(found.first(), &why));
        QVERIFY(p.contentType(QStringLiteral("/xl/workbook.xml")).contains(QLatin1String("template")));
    }
    // A workbook made from it: its cells, no file, and a name offered as a workbook.
    EditWindow second;
    second.path = found.first();
    QVERIFY(second.win.open(found.first(), OpenHow::Copy));
    QVERIFY(second.win.w.filePath().isEmpty());
    QCOMPARE(second.valueAt(QStringLiteral("A1")), QStringLiteral("n:42"));
    QString offered;
    second.win.w.setSavePicker([&](const QString &start, const QString &) { offered = start; return QString(); });
    second.win.w.act(QStringLiteral("file.saveAs"))->trigger();
    QVERIFY2(offered.endsWith(QLatin1String(".xlsx")), qPrintable(offered));
    QVERIFY(removeMyTemplate(found.first()));
}


// Save as Template from a workbook that asks for a password writes a template that asks for it too.
void CrunchEditTests::aTemplateOfAProtectedWorkbookIsProtectedToo()
{
    jptest::TestModeGuard testMode;
    QDir(myTemplatesFolder()).removeRecursively();
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.win.w.anchorHooks().setFilePasswordAsker = [](SetFilePasswordRequest *r) {
        r->password = QStringLiteral("hunter2");
        r->remove = false;
        return true;
    };
    e.win.w.act(QStringLiteral("protect.password"))->trigger();
    setTemplateNameStandIn([](const QString &) { return QStringLiteral("Secret"); });
    QSignalSpy made(&e.win.w, &CrunchWindow::copyExported);
    const bool started = e.win.w.saveAsTemplate();
    setTemplateNameStandIn({});
    QVERIFY(started);
    QTRY_COMPARE_WITH_TIMEOUT(made.count(), 1, 20000);
    QVERIFY(made.first().at(1).toBool());
    const QStringList found = myTemplateFiles({QStringLiteral("*.xltx")});
    QCOMPARE(found.size(), 1);
    QFile f(found.first());
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(f.read(4) == QByteArray("\xD0\xCF\x11\xE0", 4));
    f.close();
    QVERIFY(removeMyTemplate(found.first()));
    e.win.w.setWindowModified(false);
}


// A workbook can be saved as an .xltx template, and as an .ots copy (an OpenDocument template).
void CrunchEditTests::aWorkbookCanBeSavedAsTemplatesInBothKinds()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    const QString xltx = e.folder.dir.filePath(QStringLiteral("t.xltx")), ots = e.folder.dir.filePath(QStringLiteral("t.ots"));
    QVERIFY(crunch::isCopyName(ots));
    QVERIFY(crunch::copyFilters().contains(QLatin1String("*.ots")));
    QVERIFY(crunch::workbookKindsSavedAs(QStringLiteral("xlsx")).contains(QLatin1String("xltx")));
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.saveAs(xltx));
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    {
        Package p;
        QString why;
        QVERIFY(p.readLazyFile(xltx, &why));
        QVERIFY(p.contentType(QStringLiteral("/xl/workbook.xml")).contains(QLatin1String("template")));
    }
    QSignalSpy made(&e.win.w, &CrunchWindow::copyExported);
    QVERIFY(e.win.w.saveAs(ots));
    QTRY_COMPARE_WITH_TIMEOUT(made.count(), 1, 20000);
    QVERIFY(made.first().at(1).toBool());
    QMap<QString, QByteArray> parts;
    QFile f(ots);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(jp::readZip(f.readAll(), parts));
    QCOMPARE(parts.value(QStringLiteral("mimetype")), QByteArray("application/vnd.oasis.opendocument.spreadsheet-template"));
}


// Open and Repair: a workbook cut short does not open as it is; opened with repair it has the sheets and cells that could be read, no file of its own, and a note of what was left out.
void CrunchEditTests::aWorkbookCutShortOpensRepaired()
{
    EditWindow e;
    // The damage is made on purpose: the same package with one named part, the first sheet, cut in the middle of its third row.
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(editBook(), parts));
    const QString sheet = QStringLiteral("xl/worksheets/sheet1.xml");
    QVERIFY(parts.contains(sheet));
    const qsizetype third = parts[sheet].indexOf("<row r=\"3\"");
    QVERIFY(third > 0);
    parts[sheet].truncate(third + 20);
    jp::ZipWriter z;
    z.add(QStringLiteral("[Content_Types].xml"), parts.take(QStringLiteral("[Content_Types].xml")), true);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) z.add(it.key(), it.value(), true);
    const QByteArray damaged = z.finish();
    const QString path = e.folder.write(QStringLiteral("cut.xlsx"), damaged);
    e.path = path;
    QVERIFY(e.win.open(path, OpenHow::Repair));
    QVERIFY(e.win.w.filePath().isEmpty());
    QVERIFY(e.win.w.displayName().contains(QLatin1String("cut")));
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
    QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:10"));
    QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("-"));   // (empty: the row that was cut is left out)
    QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("Open and Repair")), qPrintable(e.win.told.join(QLatin1Char('|'))));
    // The package kept for the reader is not left behind (the one this load made: other programs' are not this test's to count).
    QVERIFY(!e.win.w.lastRepairFileForTest().isEmpty());
    QVERIFY(!QFileInfo::exists(e.win.w.lastRepairFileForTest()));
    // Save asks where; the damaged file is as it was.
    const QString other = e.folder.dir.filePath(QStringLiteral("repaired.xlsx"));
    e.win.w.setSavePicker([&](const QString &, const QString &) { return other; });
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("5"));
    QVERIFY(e.settle());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(savedCell(other, QStringLiteral("A1")), QStringLiteral("n:5"));
    QCOMPARE(QFileInfo(path).size(), qint64(damaged.size()));
}


// Backspace on a cell starts an entry with nothing in it (Enter then clears the cell, Esc leaves it as it was), on every platform.
void CrunchEditTests::backspaceStartsAnEmptyEntry()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("A1"));
    QTest::keyClick(e.view(), Qt::Key_Backspace);
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.win.w.cellEditor()->text(), QString());
    e.key(Qt::Key_Escape);
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
    QCOMPARE(e.win.w.undoStack()->count(), 0);
    QTest::keyClick(e.view(), Qt::Key_Backspace);
    e.key(Qt::Key_Return);
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("-"));
    QCOMPARE(e.win.w.undoStack()->count(), 1);
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
}


// In a locale with a comma for the decimal point, formulas are typed with semicolons between arguments and commas in numbers, and the formula bar and F2 show them that way; the file
// has the format's own. (Names of functions are English.)
void CrunchEditTests::formulasAreTypedAndShownWithTheLocalesSeparators()
{
    const QLocale de(QLocale::German, QLocale::Germany);
    struct Restore {
        QLocale before = QLocale();   // (the default locale that was there, not one this assumes)
        ~Restore() { QLocale::setDefault(before); }
    } restore;
    QLocale::setDefault(de);
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.win.w.session()->setLocale(de);
    e.select(QStringLiteral("F1"));
    e.enter(QStringLiteral("=SUM(A1;A2)"));
    e.select(QStringLiteral("F2"));
    e.enter(QStringLiteral("=A1*0,5+IF(A2>1;\"a,b\";\"c\")&\"\""));
    e.select(QStringLiteral("F3"));
    e.enter(QStringLiteral("2,5"));
    e.select(QStringLiteral("F4"));
    e.enter(QStringLiteral("=SUM(1,5;2,5)"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:11"));
    QCOMPARE(e.valueAt(QStringLiteral("F3")), QStringLiteral("n:2.5"));
    QCOMPARE(e.valueAt(QStringLiteral("F4")), QStringLiteral("n:4"));
    // the formula bar, and F2
    e.select(QStringLiteral("F1"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=SUM(A1;A2)"));
    e.select(QStringLiteral("F2"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=A1*0,5+IF(A2>1;\"a,b\";\"c\")&\"\""));
    QTest::keyClick(e.view(), Qt::Key_F2);
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=A1*0,5+IF(A2>1;\"a,b\";\"c\")&\"\""));
    e.key(Qt::Key_Return);                                                              // (put in again as it is: nothing changes)
    QCOMPARE(e.win.w.undoStack()->count(), 4);
    e.select(QStringLiteral("F3"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("2,5"));
    e.select(QStringLiteral("F4"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=SUM(1,5;2,5)"));
    e.select(QStringLiteral("B1"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=A1*2"));
    // the file has the format's own
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(savedFormula(e.path, QStringLiteral("F1")), QStringLiteral("SUM(A1,A2)"));
    QCOMPARE(savedFormula(e.path, QStringLiteral("F2")), QStringLiteral("A1*0.5+IF(A2>1,\"a,b\",\"c\")&\"\""));
    QCOMPARE(savedFormula(e.path, QStringLiteral("F4")), QStringLiteral("SUM(1.5,2.5)"));
    QCOMPARE(savedCell(e.path, QStringLiteral("F3")), QStringLiteral("n:2.5"));
}


// ---- what is typed becomes

// A date typed in a locale is read in its order and shown, in the cell, in F2, and in the formula bar, in the same order: the built-in date format the cell is given is the system's short
// date, so a date typed as 4 March in a locale of day, month, year is not shown as 3/4/2025.
void CrunchEditTests::aTypedDateIsShownInTheOrderItWasTypedIn()
{
    struct Case {
        QLocale locale;
        QString typed, shown;
    };
    const Case cases[] = {{QLocale(QLocale::English, QLocale::UnitedStates), QStringLiteral("3/4/2025"), QStringLiteral("3/4/2025")},
                          {QLocale(QLocale::English, QLocale::UnitedKingdom), QStringLiteral("4/3/2025"), QStringLiteral("04/03/2025")},
                          {QLocale(QLocale::German, QLocale::Germany), QStringLiteral("4.3.2025"), QStringLiteral("04.03.2025")},
                          {QLocale(QLocale::Japanese, QLocale::Japan), QStringLiteral("2025/3/4"), QStringLiteral("2025/03/04")}};
    struct Restore {
        QLocale before = QLocale();   // (the default locale that was there, not one this assumes)
        ~Restore() { QLocale::setDefault(before); }
    } restore;
    for (const Case &c : cases) {
        const QString name = c.locale.name();
        QLocale::setDefault(c.locale);
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.session()->setLocale(c.locale);
        e.select(QStringLiteral("F1"));
        e.enter(c.typed);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:45720"));        // 4 March 2025, as typed
        QVERIFY2(e.shownAt(QStringLiteral("F1")) == c.shown, qPrintable(name + QLatin1String(" shows ") + e.shownAt(QStringLiteral("F1"))));
        e.select(QStringLiteral("F1"));
        QCOMPARE(e.win.w.formulaBar()->text(), c.shown);
        QTest::keyClick(e.view(), Qt::Key_F2);
        QCOMPARE(e.win.w.cellEditor()->text(), c.shown);
        e.key(Qt::Key_Return);                                                          // (put in again as it is: nothing changes)
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:45720"));
    }
}


void CrunchEditTests::typedNumbersGetTheFormatThatShowsThemAsTyped()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("F1"));
    e.enter(QStringLiteral("5%"));
    QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:0.05"));
    QCOMPARE(e.shownAt(QStringLiteral("F1")), QStringLiteral("5%"));
    e.enter(QStringLiteral("12.5%"));
    QCOMPARE(e.valueAt(QStringLiteral("F2")), QStringLiteral("n:0.125"));
    QCOMPARE(e.shownAt(QStringLiteral("F2")), QStringLiteral("12.5%"));
    e.enter(QStringLiteral("$1,234.5"));
    QCOMPARE(e.valueAt(QStringLiteral("F3")), QStringLiteral("n:1234.5"));
    QCOMPARE(e.shownAt(QStringLiteral("F3")), QStringLiteral("$1,234.50"));
    e.enter(QStringLiteral("3/4/2025"));
    QCOMPARE(e.valueAt(QStringLiteral("F4")), QStringLiteral("n:45720"));
    QCOMPARE(e.shownAt(QStringLiteral("F4")), QStringLiteral("3/4/2025"));
    e.enter(QStringLiteral("10:30 PM"));
    QCOMPARE(e.valueAt(QStringLiteral("F5")), QStringLiteral("n:0.9375"));
    QCOMPARE(e.shownAt(QStringLiteral("F5")), QStringLiteral("10:30 PM"));
    e.enter(QStringLiteral("1,000"));
    QCOMPARE(e.shownAt(QStringLiteral("F6")), QStringLiteral("1,000"));
    e.enter(QStringLiteral("TRUE"));
    QCOMPARE(e.valueAt(QStringLiteral("F7")), QStringLiteral("b:1"));
    e.enter(QStringLiteral("#N/A"));
    QCOMPARE(e.valueAt(QStringLiteral("F8")), QStringLiteral("e:#N/A"));
    e.enter(QStringLiteral("1.5"));
    QCOMPARE(e.shownAt(QStringLiteral("F9")), QStringLiteral("1.5"));
    // each is the cell's text again when edited
    e.select(QStringLiteral("F1"));
    QTest::keyClick(e.view(), Qt::Key_F2);
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("5%"));
    QTest::keyClick(e.win.w.cellEditor(), Qt::Key_Escape);
    e.select(QStringLiteral("F4"));
    QTest::keyClick(e.view(), Qt::Key_F2);
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("3/4/2025"));
    QTest::keyClick(e.win.w.cellEditor(), Qt::Key_Escape);
    // a number typed into a cell that has a format of its own keeps it
    e.select(QStringLiteral("A3"));
    e.enter(QStringLiteral("0.5"));
    QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("50%"));
    // and one into a cell formatted as text stays text
    e.select(QStringLiteral("C3"));
    e.enter(QStringLiteral("42"));
    QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("s:42"));
}


void CrunchEditTests::aFormulaIsParsedAndAFaultyOneKeepsWhatWasTyped()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("E1"));
    e.type(QStringLiteral("=(1+2"));
    e.key(Qt::Key_Return);
    QVERIFY(e.win.w.isEditing());                                 // still there
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=(1+2"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=(1+2"));
    QCOMPARE(e.selected(), QStringLiteral("E1"));
    QCOMPARE(e.win.told.size(), 1);
    QVERIFY2(e.win.told[0].startsWith(QLatin1String("Formula: ")) && e.win.told[0].contains(QLatin1String("can't be read")), qPrintable(e.win.told[0]));
    QCOMPARE(e.win.w.undoStack()->count(), 0);
    QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("-"));
    // fixed, it is accepted
    e.win.w.cellEditor()->setMode(CellEditor::Mode::Editing);
    QTest::keyClicks(e.win.w.cellEditor(), ")");
    e.key(Qt::Key_Return);
    QVERIFY(!e.win.w.isEditing());
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:3"));
    QCOMPARE(e.win.w.formulaBar()->text(), QString());
    // a text longer than a cell holds is refused with the numbers in the message
    e.select(QStringLiteral("E2"));
    e.type(QStringLiteral("x"));
    e.win.w.cellEditor()->setText(QString(32768, QLatin1Char('x')));
    e.key(Qt::Key_Return);
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.win.told.size(), 2);
    QVERIFY2(e.win.told[1].contains(QLatin1String("32,767")) && e.win.told[1].contains(QLatin1String("32,768")), qPrintable(e.win.told[1]));
    e.win.w.cellEditor()->setText(QString(32767, QLatin1Char('x')));
    e.key(Qt::Key_Return);
    QVERIFY(!e.win.w.isEditing());
    QCOMPARE(e.win.w.undoStack()->count(), 2);
}


void CrunchEditTests::deleteClearsContentsAsOneStep()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.view()->selectRange(0, 0, 2, 2);
    QTest::keyClick(e.view(), Qt::Key_Delete);
    QVERIFY(e.settle());
    for (const char *ref : {"A1", "B1", "C1", "A2", "B2", "A3", "B3", "C3"}) QCOMPARE(e.valueAt(QString::fromLatin1(ref)), QStringLiteral("-"));
    QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("n:45293"));   // (outside the range)
    QCOMPARE(e.win.w.undoStack()->count(), 1);
    QVERIFY(e.win.w.undoStack()->undoText().startsWith(QLatin1String("Clearing ")));
    // the formats stay: the percent cell shows a percent when something is typed in it
    e.select(QStringLiteral("A3"));
    e.enter(QStringLiteral("0.5"));
    QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("50%"));
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(e.settle());
    QCOMPARE(e.win.w.undoStack()->index(), 0);
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:2"));
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:hello"));
    QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:0.25"));
    // a cell with nothing to clear is not a step
    e.select(QStringLiteral("Q9"));
    QTest::keyClick(e.view(), Qt::Key_Delete);
    QCOMPARE(e.win.w.undoStack()->count(), 2);   // (the typing and the clearing, both undone and there to redo)
}


// On a protected sheet the locked cells (all of them, unless the format says it is not) refuse typing, F2, the formula bar, and Delete, with a plain message and no step; the cells
// that are not locked take them.
void CrunchEditTests::aProtectedSheetsLockedCellsRefuseEdits()
{
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" s=\"1\"><v>2</v></c><c r=\"C1\"><f>A1+B1</f><v>3</v></c></row>"
                                        "<row r=\"2\"><c r=\"A2\" s=\"1\"><v>20</v></c><c r=\"B2\"><v>30</v></c></row>");
    for (const bool isProtected : {true, false}) {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, isProtected ? QStringLiteral("<sheetProtection sheet=\"1\" objects=\"1\" scenarios=\"1\"/>") : QString())}, QByteArray(), QString(), QString(), styles)));
        auto told = [&] { return e.win.told.join(QLatin1Char('\n')); };
        // typing over a locked cell
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_5);
        if (isProtected) {
            QVERIFY(!e.win.w.isEditing());
            QVERIFY2(told().contains(QLatin1String("protected")) && told().contains(QLatin1String("locked")), qPrintable(told()));
            QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            // F2, and the formula bar
            e.win.told.clear();
            QTest::keyClick(e.view(), Qt::Key_F2);
            QVERIFY(!e.win.w.isEditing());
            QVERIFY(told().contains(QLatin1String("protected")));
            // Delete over the locked cells: nothing is cleared (not even the cells that are not locked in the same range)
            e.win.told.clear();
            e.view()->selectRange(0, 0, 1, 1);
            QTest::keyClick(e.view(), Qt::Key_Delete);
            QVERIFY(told().contains(QLatin1String("protected")));
            for (const char *ref : {"A1", "B1", "A2", "B2"}) QVERIFY2(e.valueAt(QString::fromLatin1(ref)) != QStringLiteral("-"), ref);
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            QVERIFY(!e.win.w.isModified());
            // the session says no too
            QVERIFY(!e.win.w.session()->type(0, 0, 0, QStringLiteral("9")).ok);
            QVERIFY(!e.win.w.session()->clear(0, 0, 0, 1, 1).ok);
            QVERIFY(!e.win.w.session()->canEditCell(0, 1, 1));
        } else {
            QVERIFY(e.win.w.isEditing());
            e.key(Qt::Key_Escape);
        }
        // the cells that are not locked take edits
        e.win.told.clear();
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("7"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:7"));
        e.view()->selectRange(1, 0, 1, 0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("-"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:8"));      // (what depends on it follows)
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        QVERIFY2(e.win.told.isEmpty(), qPrintable(told()));
    }
}


// A table's header row holds the names of its columns, and the file is valid only while the table's part has the same names: typing in a header cell renames the column in the part and in
// the formulas that name it (Undo and Redo too, and a save has them). A name another column has gets a number; an empty name, and clearing a header, are refused.
void CrunchEditTests::typingInATableHeaderRenamesTheColumn()
{
    const QString header = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Region</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>Q1</t></is></c><c r=\"C1\" t=\"inlineStr\"><is><t>Total</t></is></c>"
                                          "<c r=\"E1\"><f>SUM(Sales[Q1])</f><v>5</v></c></row>"
                                          "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>East</t></is></c><c r=\"B2\"><v>5</v></c><c r=\"C2\"><f>Sales[[#This Row],[Q1]]*2</f><v>10</v></c>"
                                          "<c r=\"E2\"><f>Other[Q1]</f><v>1</v></c><c r=\"E3\"><f>[@Q1]</f><v>2</v></c></row>");
    const QString table = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C2\">"
                                         "<autoFilter ref=\"A1:C2\"/><tableColumns count=\"3\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/>"
                                         "<tableColumn id=\"3\" name=\"Total\"><calculatedColumnFormula>Sales[[#This Row],[Q1]]*2</calculatedColumnFormula></tableColumn></tableColumns></table>");
    const QByteArray book = withTables(buildXlsx({worksheet(header)}), {table});
    auto partOf = [](const QString &path, const QString &name) {
        QFile f(path);
        f.open(QIODevice::ReadOnly);
        QMap<QString, QByteArray> parts;
        jp::readZip(f.readAll(), parts);
        return QString::fromUtf8(parts.value(name));
    };
    EditWindow e;
    QVERIFY(e.open(book));
    auto saveNow = [&] {
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
    };
    // an empty name, and a header that is cleared: refused, with a message
    e.select(QStringLiteral("B1"));
    e.win.told.clear();
    QVERIFY(!e.win.w.session()->type(0, 0, 1, QString()).ok);
    QVERIFY(!e.win.w.session()->type(0, 0, 1, QStringLiteral("   ")).ok);
    QVERIFY(!e.win.w.session()->clear(0, 0, 0, 0, 2).ok);
    e.view()->selectRange(0, 1, 0, 1);
    QTest::keyClick(e.view(), Qt::Key_Delete);
    QVERIFY2(e.win.told.join(QLatin1Char(' ')).contains(QLatin1String("column")), qPrintable(e.win.told.join(QLatin1Char(' '))));
    QCOMPARE(e.win.w.undoStack()->count(), 0);
    QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("Q1"));
    // a name
    e.select(QStringLiteral("B1"));
    e.enter(QStringLiteral("First quarter"));
    QVERIFY(e.settle());
    QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("First quarter"));
    QCOMPARE(e.win.w.undoStack()->count(), 1);
    saveNow();
    QCOMPARE(savedCell(e.path, QStringLiteral("B1")), QStringLiteral("i:First quarter"));
    QCOMPARE(savedFormula(e.path, QStringLiteral("C2")), QStringLiteral("Sales[[#This Row],[First quarter]]*2"));
    QCOMPARE(savedFormula(e.path, QStringLiteral("E1")), QStringLiteral("SUM(Sales[First quarter])"));
    QCOMPARE(savedFormula(e.path, QStringLiteral("E2")), QStringLiteral("Other[Q1]"));                // (another table's)
    QCOMPARE(savedCell(e.path, QStringLiteral("E1")), QStringLiteral("n:5"));                         // (what is not worked out keeps its result)
    const QString part = partOf(e.path, QStringLiteral("xl/tables/table1.xml"));
    QVERIFY2(part.contains(QLatin1String("<tableColumn id=\"2\" name=\"First quarter\"/>")), qPrintable(part));
    QVERIFY2(part.contains(QLatin1String("<calculatedColumnFormula>Sales[[#This Row],[First quarter]]*2</calculatedColumnFormula>")), qPrintable(part));
    // the name another column has: a number after it. Undo and Redo go back and forth with the part.
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("total"));
    QVERIFY(e.settle());
    QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("total2"));
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(e.settle());
    QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("Region"));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("Area"));
    QVERIFY(e.settle());
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();                                               // (the first rename is undone too)
    QVERIFY(e.settle());
    QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("Q1"));
    e.win.w.act(QStringLiteral("edit.redo"))->trigger();
    QVERIFY(e.settle());
    QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("First quarter"));
    e.select(QStringLiteral("G9"));
    e.enter(QStringLiteral("1"));
    QVERIFY(e.settle());
    saveNow();
    QVERIFY(partOf(e.path, QStringLiteral("xl/tables/table1.xml")).contains(QLatin1String("name=\"First quarter\"")));
    // a number typed in a header is the column's name, as text
    e.select(QStringLiteral("C1"));
    e.enter(QStringLiteral("2025"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:2025"));
    saveNow();
    QVERIFY(partOf(e.path, QStringLiteral("xl/tables/table1.xml")).contains(QLatin1String("name=\"2025\"")));
    // undo all the way, with a save of something else: the part is the file's own again
    for (int i = 0; i < 4; ++i) e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(e.settle());
    QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("Q1"));
    QCOMPARE(e.shownAt(QStringLiteral("C1")), QStringLiteral("Total"));
}


// ---- what depends on a cell

// What reads a cell is worked out again by the worker, and the results come back in batches; the window says it is calculating, and says Ready when they are all there. The thread
// that works them out is not the window's.
void CrunchEditTests::whatDependsOnACellIsWorkedOutAgain()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:2"));
    QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:11"));
    QSignalSpy done(&e.win.w, &CrunchWindow::calculationFinished);
    const qint64 batches = e.win.w.batchesTaken();
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("100"));
    QVERIFY(e.win.w.isCalculating());
    QVERIFY(done.count() > 0 || done.wait(20000));
    QVERIFY(!e.win.w.isCalculating());
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:200"));
    QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:110"));
    QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("200"));
    QVERIFY(e.win.w.batchesTaken() > batches);
    QCOMPARE(e.win.w.statusText(), QStringLiteral("Ready"));
    QVERIFY(e.win.w.loaded()->calcThread.load() != quintptr(QThread::currentThreadId()));
    // a formula typed into a cell is worked out, and what reads that
    e.select(QStringLiteral("E1"));
    e.enter(QStringLiteral("=B1+B2"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:310"));
    QCOMPARE(e.win.w.formulaBar()->text(), QString());
    e.select(QStringLiteral("E1"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=B1+B2"));
    e.select(QStringLiteral("A2"));
    e.enter(QStringLiteral("1"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:301"));   // B1 200 + B2 101
    // a function added after the format's first release is written with the prefix the file needs, and shown without it
    e.select(QStringLiteral("E2"));
    e.enter(QStringLiteral("=stdev.s(A1:A3)"));
    QVERIFY(e.settle());
    e.select(QStringLiteral("E2"));
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=STDEV.S(A1:A3)"));
    const Calculation *calc = e.win.w.calculation();
    QVERIFY(calc);
    QCOMPARE(calc->content(0, 1, 4).formula, QStringLiteral("_xlfn.STDEV.S(A1:A3)"));
}


// An edit made while the workbook is still being calculated is absorbed: what was going on is worked out again with it in, and the last values are those of the edited workbook.
void CrunchEditTests::anEditDuringTheCalculationIsAbsorbed()
{
    const int n = 60000;
    QString rows;
    rows.reserve(n * 90);
    for (int r = 1; r <= n; ++r)
        rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2+$D$1</f><v>-1</v></c>%2</row>").arg(r).arg(r == 1 ? QStringLiteral("<c r=\"D1\"><v>0</v></c>") : QString());
    Folder folder;
    Window win;
    QSignalSpy loaded(&win.w, &CrunchWindow::loadFinished);
    win.w.setWorkerPace(20000);   // (the calculation takes the time the test needs)
    QVERIFY(win.w.openFile(folder.write(QStringLiteral("big.xlsx"), buildXlsx({worksheet(rows)}))));
    QVERIFY(loaded.wait(60000));
    QVERIFY(win.w.isCalculating());
    // an entry in D1 while it goes
    win.w.view()->selectCell(0, 3);
    QTest::keyClick(win.w.view(), Qt::Key_5);
    QTest::keyClick(win.w.cellEditor(), Qt::Key_Return);
    QVERIFY(!win.w.isEditing());
    QVERIFY(win.w.isCalculating());
    win.w.setWorkerPace(0);
    QVERIFY(QTest::qWaitFor([&] { return !win.w.isCalculating(); }, 60000));
    const grid::GridSheet &gs = win.w.view()->sheet();
    for (const quint32 r : {0u, 1u, 15000u, quint32(n - 1)}) {
        const grid::Cell *c = gs.cellAt(r, 1);
        QVERIFY(c);
        QCOMPARE(gs.valueOf(r, 1, *c).debug(), QStringLiteral("n:%1").arg((r + 1) * 2 + 5));
    }
}


// Counted: an edit made with the worker held does not ask the graph who reads the cell, marks nothing, and registers no formula but the one typed; the worker does that after, and the
// results come back. (The cell is read by twenty thousand formulas: were the window to look for them, the graph's work would show it.)
void CrunchEditTests::anEditCostsTheWindowOnlyWhatPuttingTheCellInCosts()
{
    const int n = 20000;
    QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>");
    for (int r = 2; r <= n + 1; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"B%1\"><f>$A$1+%1</f><v>-1</v></c></row>").arg(r);
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(rows)})));
    qint64 queried0 = -1, queried1 = -1, dirty1 = -1, registered0 = -1, registered1 = -1, holds = 0;
    e.win.w.session()->setHoldHook([&](Calculation *calc, bool after) {
        ++holds;
        if (!after) {
            queried0 = calc->graph().queryWork();
            registered0 = calc->registrations();
        } else {
            queried1 = calc->graph().queryWork();
            dirty1 = calc->dirtyCount();
            registered1 = calc->registrations();
        }
    });
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("100"));
    QCOMPARE(holds, qint64(2));
    QCOMPARE(queried1, queried0);        // the graph was not asked
    QCOMPARE(dirty1, qint64(0));         // nothing was marked
    QCOMPARE(registered1, registered0);  // and no formula told it what it reads
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:102"));
    QCOMPARE(e.valueAt(QStringLiteral("B20001")), QStringLiteral("n:20101"));
    // a formula typed registers once
    e.select(QStringLiteral("D1"));
    e.enter(QStringLiteral("=A1*2"));
    QCOMPARE(registered1 - registered0, qint64(1));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:200"));
}


// The worker stays when the formulas are worked out, and goes with the window.
void CrunchEditTests::theWorkerStaysAndGoesWithTheWindow()
{
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.loaded()->calcIdle.load());
        QVERIFY(crunch::WorkbookLoader::workersRunning() >= 1);
        QVERIFY(!e.win.w.loaded()->workerEnded.load());
    }
    QTRY_COMPARE(crunch::WorkbookLoader::workersRunning(), 0);
}


// A calculation stopped with Esc, and then a cell typed in: the worker takes up what is left and the new change, and ends with every formula worked out.
void CrunchEditTests::anEditAfterEscResumesTheCalculation()
{
    const int n = 60000;
    QString rows;
    rows.reserve(n * 90);
    for (int r = 1; r <= n; ++r)
        rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2+$D$1</f><v>-1</v></c>%2</row>").arg(r).arg(r == 1 ? QStringLiteral("<c r=\"D1\"><v>0</v></c>") : QString());
    Folder folder;
    Window win;
    QSignalSpy loaded(&win.w, &CrunchWindow::loadFinished);
    win.w.setWorkerPace(20000);
    QVERIFY(win.w.openFile(folder.write(QStringLiteral("big.xlsx"), buildXlsx({worksheet(rows)}))));
    QVERIFY(loaded.wait(60000));
    QTRY_VERIFY(win.w.batchesTaken() >= 1);
    QTest::keyClick(&win.w, Qt::Key_Escape);
    QVERIFY(!win.w.isCalculating());
    QTRY_VERIFY(win.w.loaded()->calcIdle.load());
    win.w.setWorkerPace(0);
    win.w.view()->selectCell(0, 3);
    QTest::keyClick(win.w.view(), Qt::Key_9);
    QTest::keyClick(win.w.cellEditor(), Qt::Key_Return);
    QVERIFY(win.w.isCalculating());
    QVERIFY(QTest::qWaitFor([&] { return !win.w.isCalculating(); }, 60000));
    const grid::GridSheet &gs = win.w.view()->sheet();
    for (const quint32 r : {0u, 30000u, quint32(n - 1)}) QCOMPARE(gs.valueOf(r, 1, *gs.cellAt(r, 1)).debug(), QStringLiteral("n:%1").arg((r + 1) * 2 + 9));
}


// Edits one after the other, before the worker has worked out the first: all of them are worked out, and the window says it is calculating until the last is.
void CrunchEditTests::editsOneAfterTheOtherAreAllWorkedOut()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.win.w.setWorkerPace(5000);
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("100"));
    e.select(QStringLiteral("A2"));
    e.enter(QStringLiteral("200"));
    e.select(QStringLiteral("A3"));
    e.enter(QStringLiteral("300"));
    QVERIFY(e.win.w.isCalculating());
    e.win.w.setWorkerPace(0);
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:200"));
    QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:300"));      // SUM(A1:A2)
    QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:301"));
    QCOMPARE(e.win.w.undoStack()->count(), 3);
}


// Nothing can be changed until the workbook is read whole: the person is told so in the status line, with no message box in the way.
void CrunchEditTests::anEditIsRefusedWhileTheWorkbookIsBeingOpened()
{
    QString rows;
    for (int r = 1; r <= 30000; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%1</v></c></row>").arg(r);
    Folder folder;
    Window win;
    QSignalSpy first(&win.w, &CrunchWindow::firstScreenShown), loaded(&win.w, &CrunchWindow::loadFinished);
    win.w.setWorkerPace(20000);
    QVERIFY(win.w.openFile(folder.write(QStringLiteral("slow.xlsx"), buildXlsx({worksheet(rows)}))));
    QVERIFY(first.wait(60000));
    QVERIFY(win.w.isLoading());
    QTest::keyClick(win.w.view(), Qt::Key_7);
    QVERIFY(!win.w.isEditing());
    QVERIFY(win.w.statusText().contains(QLatin1String("still being opened")));
    QCOMPARE(win.told.size(), 0);
    QVERIFY(win.w.session() == nullptr);
    win.w.setWorkerPace(0);
    QVERIFY(loaded.wait(60000));
    QVERIFY(win.w.session() != nullptr);
}


// A cell of an array is not changed alone.
void CrunchEditTests::aCellOfAnArrayIsNotChanged()
{
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C2\">A1:B1*2</f><v>2</v></c></row>"))})));
    e.select(QStringLiteral("C1"));
    QTest::keyClick(e.view(), Qt::Key_9);
    QVERIFY(!e.win.w.isEditing());
    QCOMPARE(e.win.told.size(), 1);
    QVERIFY2(e.win.told[0].contains(QLatin1String("array")), qPrintable(e.win.told[0]));
    QTest::keyClick(e.view(), Qt::Key_Delete);
    QCOMPARE(e.win.told.size(), 2);
    QCOMPARE(e.win.w.undoStack()->count(), 0);
}


// ---- Undo and Redo

// A typed cell is one step. Undo puts back the value, the formula, the format, and what depends on the cell; Redo does it again; the keys work, and the command dims when there is nothing.
void CrunchEditTests::undoPutsBackValuesFormulasFormatsAndDependents()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QAction *undo = e.win.w.act(QStringLiteral("edit.undo")), *redo = e.win.w.act(QStringLiteral("edit.redo"));
    QVERIFY(undo->isEnabled() == false && redo->isEnabled() == false);
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("50"));           // a number over a number
    e.select(QStringLiteral("B1"));
    e.enter(QStringLiteral("typed"));        // text over a formula
    e.select(QStringLiteral("D2"));
    e.enter(QStringLiteral("7"));            // a number over a date (keeps its format)
    e.select(QStringLiteral("F1"));
    e.enter(QStringLiteral("25%"));          // a new format
    QVERIFY(e.settle());
    QCOMPARE(e.win.w.undoStack()->count(), 4);
    QVERIFY(undo->isEnabled());
    QVERIFY(!redo->isEnabled());
    QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:60"));    // A1 50 + A2 10
    pressUndo(&e.win.w);            // undo "25%"
    QVERIFY(redo->isEnabled());
    QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("-"));
    QCOMPARE(e.selected(), QStringLiteral("F1"));                          // (the cell it changed is shown)
    pressUndo(&e.win.w);            // 7 over the date
    QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("n:45293"));
    QCOMPARE(e.shownAt(QStringLiteral("D2")), QStringLiteral("1/2/2024"));
    pressUndo(&e.win.w);            // the text over the formula
    QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=A1*2"));
    pressUndo(&e.win.w);            // 50 over 1
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:2"));
    QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:11"));
    QVERIFY(!undo->isEnabled());
    QCOMPARE(e.win.w.undoStack()->index(), 0);
    QVERIFY(e.win.w.undoStack()->isClean());
    // Redo with Ctrl+Y puts them back one by one
    pressRedo(&e.win.w);
    pressRedo(&e.win.w);
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:50"));
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("s:typed"));
    QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:60"));
    pressRedo(&e.win.w);
    pressRedo(&e.win.w);
    QCOMPARE(e.shownAt(QStringLiteral("F1")), QStringLiteral("25%"));
    QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("n:7"));
    QVERIFY(!redo->isEnabled());
    // a new edit after an undo drops what could have been redone
    pressUndo(&e.win.w);
    QVERIFY(redo->isEnabled());
    e.select(QStringLiteral("H1"));
    e.enter(QStringLiteral("1"));
    QVERIFY(!redo->isEnabled());
    QCOMPARE(e.win.w.undoStack()->count(), 4);
}


// Typing over a formula that has a function this program lacks, and Undo: the formula comes back with the result the file stored (not #NAME?), and the cell shows it at once.
void CrunchEditTests::undoPutsBackTheStoredResultOfAFormulaWithAFunctionTheProgramLacks()
{
    const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>_xlfn.IMAGE(\"x\")</f><v>88</v></c><c r=\"B1\"><f>A1+1</f><v>89</v></c></row>"))});
    EditWindow e;
    QVERIFY(e.open(book));
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:88"));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("5"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:6"));
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:88"));   // (at once: the worker does not work it out)
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:88"));
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:89"));
    QVERIFY(!e.win.w.isModified());
    // and redo, undo again, then another change is saved with the formula and its result as the file had them
    e.win.w.act(QStringLiteral("edit.redo"))->trigger();
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(e.settle());
    e.select(QStringLiteral("D5"));
    e.enter(QStringLiteral("1"));
    QVERIFY(e.settle());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(savedFormula(e.path, QStringLiteral("A1")), QStringLiteral("_xlfn.IMAGE(\"x\")"));
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:88"));
    QCOMPARE(savedCell(e.path, QStringLiteral("B1")), QStringLiteral("n:89"));
}


// The steps of Undo hold at most a number of bytes (what the cells held, as they were and as they are): past it the oldest steps are forgotten (they stay in the list, empty, and Undo
// goes back to the oldest that is kept); the newest step is kept whatever its size. A step that was forgotten is counted off, so a long session does not grow.
void CrunchEditTests::undoHoldsAtMostItsBytesAndForgetsTheOldestSteps()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    crunch::EditSession *session = e.win.w.session();
    QCOMPARE(session->undoByteLimit(), crunch::kUndoBytes);
    session->setUndoByteLimit(1 << 20);
    const QString big(30000, QLatin1Char('x'));
    for (int i = 0; i < 100; ++i) QVERIFY(session->type(0, quint32(10 + i), 5, big + QString::number(i)).ok);
    QCOMPARE(e.win.w.undoStack()->count(), 100);
    QVERIFY2(session->undoBytes() <= (1 << 20), qPrintable(QString::number(session->undoBytes())));
    QVERIFY(session->undoBytes() > (1 << 19));
    int undone = 0;
    while (session->canUndo()) {
        QVERIFY(e.win.w.act(QStringLiteral("edit.undo"))->isEnabled());
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(++undone < 200);
    }
    QVERIFY2(undone >= 12 && undone <= 17, qPrintable(QString::number(undone)));
    QVERIFY(!e.win.w.act(QStringLiteral("edit.undo"))->isEnabled());
    QVERIFY(e.win.w.act(QStringLiteral("edit.redo"))->isEnabled());
    // the newest steps are undone, the older ones are as they were typed
    for (int i = 0; i < 100; ++i) QCOMPARE(session->editText(0, quint32(10 + i), 5).isEmpty(), i >= 100 - undone);
    // Redo goes forward through those, and Undo again
    for (int i = 0; i < undone; ++i) e.win.w.act(QStringLiteral("edit.redo"))->trigger();
    QVERIFY(!session->editText(0, 109, 5).isEmpty());
    QVERIFY(session->canUndo());
    // the size of what is held does not grow with the steps: a thousand steps more
    for (int i = 0; i < 1000; ++i) QVERIFY(session->type(0, quint32(200 + i % 50), 6, big + QString::number(i)).ok);
    QVERIFY2(session->undoBytes() <= (1 << 20), qPrintable(QString::number(session->undoBytes())));
    QCOMPARE(e.win.w.undoStack()->count(), crunch::kUndoSteps);
    // a step by itself larger than the limit is kept (the newest one is never forgotten), and undone
    session->setUndoByteLimit(10);
    QVERIFY(session->type(0, 300, 7, big).ok);
    QVERIFY(session->canUndo());
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(session->editText(0, 300, 7).isEmpty());
    QVERIFY(!session->canUndo());
    // saving still writes everything typed, those whose steps are forgotten too
    QVERIFY(e.settle());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 60000), "the save failed");
    QCOMPARE(saved.first().at(1).toBool(), true);
    const QString first = savedCell(e.path, QStringLiteral("F11"));
    QVERIFY2(first.size() == 2 + 30000 + 1 && (first.startsWith(QLatin1String("i:xxxx")) || first.startsWith(QLatin1String("s:xxxx"))), qPrintable(first.left(20)));
}


// Undo keeps what changed, not the workbook: a step holds the cells it changed, as they were and as they are; the stack holds at most its limit of steps.
void CrunchEditTests::undoHoldsWhatChangedAndNoMore()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    for (int i = 0; i < 1200; ++i) QVERIFY(e.win.w.session()->type(0, quint32(i % 50 + 10), 5, QString::number(i)).ok);
    QCOMPARE(e.win.w.undoStack()->count(), crunch::kUndoSteps);
}


// Undo back to where the file was says the file lacks nothing, but the results that follow the undo are still to come, and a Redo after them (or the same number typed again) must be
// saved with the results it gives: the file has the old ones. (What was kept of the engine's value before the edit was lost with the undo, and the old edited value was taken for it.)
void CrunchEditTests::anUndoAndARedoStillSaveTheResultsOfTheRedo()
{
    const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*2</f><v>2</v></c><c r=\"C1\"><f>B1+1</f><v>3</v></c></row>"))});
    for (int variant = 0; variant < 4; ++variant) {
        EditWindow e;
        QVERIFY(e.open(book));
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:11"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        if (variant == 0 || variant == 2) QVERIFY(e.settle());            // (the results of the undo come before the redo, or they do not)
        if (variant < 2) {
            e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        } else {
            e.select(QStringLiteral("A1"));
            e.enter(QStringLiteral("5"));                                   // the same number typed again
        }
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:10"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:11"));
        QVERIFY(e.win.w.isModified());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QVERIFY2(saved.first().at(1).toBool(), qPrintable(QString::number(variant)));
        QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:5"));
        QVERIFY2(savedCell(e.path, QStringLiteral("B1")) == QStringLiteral("n:10"), qPrintable(QStringLiteral("variant %1: B1 is %2").arg(variant).arg(savedCell(e.path, QStringLiteral("B1")))));
        QVERIFY2(savedCell(e.path, QStringLiteral("C1")) == QStringLiteral("n:11"), qPrintable(QStringLiteral("variant %1: C1 is %2").arg(variant).arg(savedCell(e.path, QStringLiteral("C1")))));
        // and it opens with the same values
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.view()->sheet().valueOf(0, 2, *second.w.view()->sheet().cellAt(0, 2)).debug(), QStringLiteral("n:11"));
    }
}


// A cell being edited in the window of a workbook that looks like a real one: its line is over the cell with the cell's formula, the formula bar shows the same, and the line has the accent
// round it. The picture is saved when JC_SCREENSHOT_EDIT names a file.
void CrunchEditTests::aCellBeingEditedShowsItsFormulaInTheCellAndTheBar()
{
    Folder folder;
    Window win;
    win.w.resize(1100, 640);
    QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
    win.w.view()->setZoom(1.0);
    win.w.view()->selectCell(8, 2);   // C9: the total of the second month
    QTest::keyClick(win.w.view(), Qt::Key_F2);
    QVERIFY(win.w.isEditing());
    QApplication::processEvents();
    CellEditor *editor = win.w.cellEditor();
    QVERIFY(editor->isVisible());
    QCOMPARE(editor->text(), QStringLiteral("=SUM(C3:C8)"));
    QCOMPARE(win.w.formulaBar()->text(), QStringLiteral("=SUM(C3:C8)"));
    QCOMPARE(win.w.nameBox()->text(), QStringLiteral("C9"));
    QImage shot = win.w.grab().toImage();
    QVERIFY(!shot.isNull());
    // the line is over C9: the accent runs round it
    const QPoint at = editor->mapTo(&win.w, QPoint(0, 0));
    QCOMPARE(QColor(shot.pixel(at.x() + 1, at.y() + editor->height() / 2)), QColor(0x13, 0x77, 0x3A));
    QCOMPARE(QColor(shot.pixel(at.x() + editor->width() / 2, at.y() + 1)), QColor(0x13, 0x77, 0x3A));
    QCOMPARE(editor->geometry().size(), win.w.view()->cellRect(8, 2).size() + QSize(editor->width() - win.w.view()->cellRect(8, 2).width(), 0));
    const QString out = qEnvironmentVariable("JC_SCREENSHOT_EDIT");
    if (!out.isEmpty()) QVERIFY(shot.save(out));
    QTest::keyClick(editor, Qt::Key_Escape);
}


// ---- saving

// Save writes the workbook as it was read with the edit in it: the edited cell, the formulas that read it with their new results, the new text and format; every part that did not
// change is the old file's, byte for byte; and the calculation chain is left out.
void CrunchEditTests::saveWritesWhatChangedAndKeepsTheRest()
{
    const QByteArray original = editBookWithParts();
    EditWindow e;
    QVERIFY(e.open(original));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("100"));
    e.select(QStringLiteral("F1"));
    e.enter(QStringLiteral("25%"));
    e.select(QStringLiteral("F2"));
    e.enter(QStringLiteral("new text"));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.isModified());
    QVERIFY(e.win.w.windowTitle().contains(QLatin1String("[*]")) || e.win.w.isWindowModified());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(saved.first().at(1).toBool(), true);
    QVERIFY(!e.win.w.isModified());
    QVERIFY(!e.win.w.isSaving());
    QCOMPARE(e.win.told.size(), 0);
    // the file
    QFile f(e.path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray now = f.readAll();
    QVERIFY(now != original);
    const auto before = rawParts(original), after = rawParts(now);
    QVERIFY(!after.contains(QStringLiteral("xl/calcChain.xml")));
    for (auto it = before.constBegin(); it != before.constEnd(); ++it) {
        if (it.key() == QLatin1String("xl/calcChain.xml")) continue;
        QVERIFY2(after.contains(it.key()), qPrintable(it.key()));
        const bool changed = it.key() == QLatin1String("xl/worksheets/sheet1.xml") || it.key() == QLatin1String("xl/sharedStrings.xml") || it.key() == QLatin1String("xl/styles.xml") || it.key() == QLatin1String("[Content_Types].xml")
            || it.key() == QLatin1String("xl/_rels/workbook.xml.rels");
        if (!changed) QVERIFY2(after.value(it.key()) == it.value(), qPrintable(it.key()));
    }
    // read back: the cells, the results, the text, the format
    Package p;
    QString why;
    QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
    Workbook b;
    QVERIFY2(b.open(p, &why), qPrintable(why));
    Sheet s;
    QVERIFY(b.readSheet(0, &s));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:100"));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B1")), QStringLiteral("n:200"));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B2")), QStringLiteral("n:110"));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B3")), QStringLiteral("n:999"));    // (nothing that was changed is read by it: it is as the file had it)
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("F1")), QStringLiteral("n:0.25"));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("F2")), QStringLiteral("s:new text"));
    QCOMPARE(b.styles().numFmtId(at(s, QStringLiteral("F1"))->style), 9);
    QCOMPARE(b.sharedStrings().strings.size(), 2);
    // the formula is still a formula
    QCOMPARE(s.formulaText(*at(s, QStringLiteral("B1"))), QStringLiteral("A1*2"));
    // and it opens in this program, with the same values
    Window second;
    QVERIFY(second.open(e.copyOfFile()));
    QCOMPARE(second.w.view()->sheet().valueOf(0, 1, *second.w.view()->sheet().cellAt(0, 1)).debug(), QStringLiteral("n:200"));
}


// The next save writes what changed since the last, over the file the last wrote; and a workbook that is back where it was saved has nothing to save.
void CrunchEditTests::aSecondSaveWritesWhatChangedSinceTheFirst()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("5"));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    e.select(QStringLiteral("A2"));
    e.enter(QStringLiteral("6"));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.isModified());
    QVERIFY(e.win.w.save());
    QVERIFY2((saved.count() == 2 || saved.wait(20000)) && saved.last().value(1).toBool(), "the save failed");
    QVERIFY(!e.win.w.isModified());
    {
        Package p;
        QVERIFY(p.readLazyFile(e.path));
        Workbook b;
        QVERIFY(b.open(p));
        Sheet s;
        QVERIFY(b.readSheet(0, &s));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:5"));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A2")), QStringLiteral("n:6"));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B2")), QStringLiteral("n:11"));
    }
    // an edit and its undo is a workbook as it was saved: nothing to save, and Save writes nothing
    const QByteArray bytes = [&] {
        QFile f(e.path);
        f.open(QIODevice::ReadOnly);
        return f.readAll();
    }();
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("9"));
    QVERIFY(e.win.w.isModified());
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(QTest::qWaitFor([&] { return !e.win.w.isCalculating(); }, 20000));
    QVERIFY(!e.win.w.isModified());
    QVERIFY(e.win.w.save());
    QCOMPARE(saved.count(), 3);   // (said at once)
    QFile f(e.path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(f.readAll() == bytes);
    QVERIFY(e.win.w.statusText().contains(QLatin1String("Nothing to save")));
}


JP_TEST_CLASS(CrunchEditTests)
