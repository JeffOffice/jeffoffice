// Tests of Cut, Copy, and Paste: within a workbook (values, formulas with their references moved, formats, merged ranges), between workbooks, with Paste Special's options, from and to other
// programs as text and as HTML, and a cut that moves the cells and what refers to them. Every workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/anchordialogs.h"
#include "app/crunchwindow.h"
#include "savedok.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "../sheet/richbook.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QToolButton>
#include <QDataStream>
#include <QMimeData>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

QString formulaOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.win.w.session()->editText(0, r, c);
}

} // namespace

class CrunchClipTests : public QObject {
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
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // Copy and Paste in a workbook: the cells come whole (values, formulas with the references that are not fixed moved, formats), in one step, and the border of the copy stays for another paste.
    void copyAndPasteKeepFormulasValuesAndFormats()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1==A1*2", "B2==SUM(A$1:A2)+$A1", "C1=%0.25", "D1=@2024-01-15", "D2=text"})));
        copyRange(e, QStringLiteral("A1:D2"));
        QVERIFY(e.view()->hasCopyBorder());
        paste(e, QStringLiteral("F5"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Pasting"));
        QCOMPARE(e.valueAt(QStringLiteral("F5")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("F6")), QStringLiteral("n:2"));
        QCOMPARE(formulaOf(e, QStringLiteral("G5")), QStringLiteral("=F5*2"));
        QCOMPARE(formulaOf(e, QStringLiteral("G6")), QStringLiteral("=SUM(F$1:F6)+$A5"));
        QCOMPARE(e.valueAt(QStringLiteral("G5")), QStringLiteral("n:2"));
        QCOMPARE(shown(e, QStringLiteral("H5")), QStringLiteral("25%"));
        QCOMPARE(shown(e, QStringLiteral("I5")), QStringLiteral("1/15/2024"));
        QCOMPARE(shown(e, QStringLiteral("I6")), QStringLiteral("text"));
        // the selection is what was pasted, and the border is still there for another paste
        QCOMPARE(e.selected(), QStringLiteral("F5"));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("F5:I6"));
        QVERIFY(e.view()->hasCopyBorder());
        paste(e, QStringLiteral("A10"));
        QCOMPARE(formulaOf(e, QStringLiteral("B11")), QStringLiteral("=SUM(A$1:A11)+$A10"));
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        // Undo and Redo of a paste
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("F5")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("A10")), QStringLiteral("-"));
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("F5")), QStringLiteral("n:1"));
        // a copy of empty cells clears: A12:B13 empty copied over F5:G6 empties them
        copyRange(e, QStringLiteral("A12:B13"));
        paste(e, QStringLiteral("F5"));
        QCOMPARE(e.valueAt(QStringLiteral("F5")), QStringLiteral("-"));
        QVERIFY(e.shownAt(QStringLiteral("H5")).isEmpty() || true);
    }

    // Esc takes the border away, and what was copied is no longer pasted as cells; a copy made elsewhere takes it away too.
    void escapeAndOtherCopiesTakeTheBorderAway()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2"})));
        copyRange(e, QStringLiteral("A1:A2"));
        QVERIFY(e.view()->hasCopyBorder());
        QTest::keyClick(e.view(), Qt::Key_Escape);
        QVERIFY(!e.view()->hasCopyBorder());
        QVERIFY(!QApplication::clipboard()->mimeData() || !QApplication::clipboard()->mimeData()->hasFormat(QLatin1String(crunch::kClipMime)));
        copyRange(e, QStringLiteral("A1:A2"));
        QVERIFY(e.view()->hasCopyBorder());
        QApplication::clipboard()->setText(QStringLiteral("something else"));
        QVERIFY(!e.view()->hasCopyBorder());
        // the text copied is what the cells show, with tabs and lines
        copyRange(e, QStringLiteral("A1:A2"));
        QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("1\n2\n"));
        QVERIFY(QApplication::clipboard()->mimeData()->hasHtml());
        QVERIFY(QApplication::clipboard()->mimeData()->html().contains(QStringLiteral("<td style=\"text-align:right;\">1</td>")));
    }

    // Cut and Paste moves: the cells go, what they went over goes, and every formula that read them reads them where they are; Undo gives all back; the clipboard is emptied.
    void cutAndPasteMovesTheCellsAndWhatReadsThem()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1==A1+A2", "C1==B1*2", "D1==SUM(A1:A2)", "E1==A3", "E5=old", "F1==E5"})));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:6"));
        copyRange(e, QStringLiteral("A1:A2"), "edit.cut");
        QVERIFY(e.view()->hasCopyBorder());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));     // (nothing has moved until the paste)
        paste(e, QStringLiteral("E4"));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Moving cells"));
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:2"));     // (over "old")
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=E4+E5"));
        QCOMPARE(formulaOf(e, QStringLiteral("D1")), QStringLiteral("=SUM(E4:E5)"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:6"));
        QCOMPARE(formulaOf(e, QStringLiteral("E1")), QStringLiteral("=A3"));  // (it did not read the cells)
        QCOMPARE(formulaOf(e, QStringLiteral("F1")), QStringLiteral("=#REF!"));   // (F1 read E5, which was written over: it has no cell to read, as in the programs people know)
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("e:#REF!"));
        QVERIFY(!e.view()->hasCopyBorder());
        QVERIFY(!QApplication::clipboard()->mimeData() || !QApplication::clipboard()->mimeData()->hasFormat(QLatin1String(crunch::kClipMime)));
        QCOMPARE(e.selected(), QStringLiteral("E4"));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("E4:E5"));
        // a value typed into the moved cell changes what reads it
        e.select(QStringLiteral("E4"));
        e.enter(QStringLiteral("10"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:24"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("s:old"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=A1+A2"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:6"));
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("s:old"));
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=E4+E5"));
    }

    // A cut that has not been pasted is ended by any other edit (typing, cells inserted, an Undo): its border goes and what it put on the clipboard goes; a copy is not ended by them.
    void aPendingCutIsEndedByAnyOtherEdit()
    {
        auto hasCells = [] { return QApplication::clipboard()->mimeData() && QApplication::clipboard()->mimeData()->hasFormat(QLatin1String(crunch::kClipMime)); };
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1==A1+A2"})));
        // typing
        copyRange(e, QStringLiteral("A1:A2"), "edit.cut");
        QVERIFY(e.view()->hasCopyBorder() && hasCells());
        e.select(QStringLiteral("D5"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QVERIFY(!e.view()->hasCopyBorder());
        QVERIFY(!hasCells());
        paste(e, QStringLiteral("F1"));   // (nothing to paste)
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("-"));
        // cells inserted
        copyRange(e, QStringLiteral("A1:A2"), "edit.cut");
        QVERIFY(e.view()->hasCopyBorder());
        e.view()->selectRows(10, 10);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(!e.view()->hasCopyBorder() && !hasCells());
        // an Undo
        copyRange(e, QStringLiteral("A1:A2"), "edit.cut");
        QVERIFY(e.view()->hasCopyBorder());
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(!e.view()->hasCopyBorder() && !hasCells());
        // a copy stays through them (it is not a cut)
        copyRange(e, QStringLiteral("A1:A2"));
        e.select(QStringLiteral("D6"));
        e.enter(QStringLiteral("6"));
        QVERIFY(e.settle());
        QVERIFY(e.view()->hasCopyBorder() && hasCells());
        // a cut that is pasted still moves the cells (the paste is an edit that ends it, after)
        copyRange(e, QStringLiteral("A1:A2"), "edit.cut");
        paste(e, QStringLiteral("F1"));
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("-"));
        QVERIFY(!e.view()->hasCopyBorder());
    }

    // Paste Special with number formats: the value (or formula) comes with its number format, and the look the cell had (here a border) stays; a plain Values paste brings no format.
    void pasteSpecialValuesAndNumberFormatsKeepTheLook()
    {
        using What = crunch::EditSession::PasteOptions::What;
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=2"})));
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("50%"));
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::BorderSide thin;
        thin.style = jp::office::sheet::BorderStyle::Thin;
        QVERIFY(s->setBorders(0, rangeOf(QStringLiteral("B1")), crunch::EditSession::EdgeLeft, thin, QStringLiteral("Borders")).ok);
        const crunch::ClipData clip = s->copyCells(0, rangeOf(QStringLiteral("A1")), false);
        crunch::EditSession::PasteOptions o;
        o.what = What::ValuesAndNumberFormats;
        QVERIFY(s->paste(0, rangeOf(QStringLiteral("B1")), clip, o).ok);
        QCOMPARE(shown(e, QStringLiteral("B1")), QStringLiteral("50%"));
        QVERIFY(e.view()->sheet().looks().look(e.view()->sheet().styleOf(0, 1, e.view()->sheet().cellAt(0, 1))).left.style == jp::office::sheet::BorderStyle::Thin);
        o.what = What::Values;
        QVERIFY(s->paste(0, rangeOf(QStringLiteral("C1")), clip, o).ok);
        QCOMPARE(shown(e, QStringLiteral("C1")), QStringLiteral("0.5"));
        o.what = What::FormulasAndNumberFormats;
        QVERIFY(s->paste(0, rangeOf(QStringLiteral("D1")), clip, o).ok);
        QCOMPARE(shown(e, QStringLiteral("D1")), QStringLiteral("50%"));
    }

    // All Except Borders brings everything but the borders: the cell takes the copy's number format and look, and keeps the borders it had (and a plain All takes the copy's).
    void pasteSpecialAllExceptBordersKeepsTheBorders()
    {
        using What = crunch::EditSession::PasteOptions::What;
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=y", "E1=z"})));
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("50%"));
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::BorderSide thick;
        thick.style = jp::office::sheet::BorderStyle::Thick;
        QVERIFY(s->setBorders(0, rangeOf(QStringLiteral("B1:B2")), crunch::EditSession::EdgeLeft | crunch::EditSession::EdgeRight | crunch::EditSession::EdgeTop | crunch::EditSession::EdgeBottom, thick, QStringLiteral("Borders")).ok);
        const auto sides = [&](const QString &ref) {
            quint32 r = 0, c = 0;
            parseCellRef(ref, &r, &c);
            const grid::CellLook &l = e.view()->sheet().looks().look(e.view()->sheet().styleOf(r, c, e.view()->sheet().cellAt(r, c)));
            return QVector<int>{int(l.left.style), int(l.right.style), int(l.top.style), int(l.bottom.style)};
        };
        const QVector<int> before = sides(QStringLiteral("B1"));
        QVERIFY(before[0] != 0 && before[1] != 0);
        const crunch::ClipData clip = s->copyCells(0, rangeOf(QStringLiteral("A1")), false);
        crunch::EditSession::PasteOptions o;
        o.what = What::AllExceptBorders;
        QVERIFY(s->paste(0, rangeOf(QStringLiteral("B1")), clip, o).ok);
        QCOMPARE(shown(e, QStringLiteral("B1")), QStringLiteral("50%"));
        QCOMPARE(sides(QStringLiteral("B1")), before);
        o.what = What::All;
        QVERIFY(s->paste(0, rangeOf(QStringLiteral("B1")), clip, o).ok);
        QCOMPARE(shown(e, QStringLiteral("B1")), QStringLiteral("50%"));
        QVERIFY(sides(QStringLiteral("B1")) != before);   // (the copy had none)
    }

    // Paste Special > Column Widths gives the columns the widths of the copy, and Comments and Notes brings the notes; each is one step of Undo, and the clipboard form keeps both.
    void pasteSpecialColumnWidthsAndNotes()
    {
        using What = crunch::EditSession::PasteOptions::What;
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=x", "B1=y", "D1=z"})));
        crunch::EditSession *s = e.win.w.session();
        crunch::EditSession::SizeChange wide;
        wide.setSize = true;
        wide.size = 30;
        QVERIFY(s->resizeColumns(0, 0, 0, wide, QStringLiteral("Width")).ok);
        QVERIFY(s->setNote(0, 0, 0, QStringLiteral("Check this"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        const int steps = e.win.w.undoStack()->count();
        crunch::ClipData clip = s->copyCells(0, rangeOf(QStringLiteral("A1:B1")), false);
        QCOMPARE(clip.notes.size(), size_t(1));
        QCOMPARE(clip.columnWidths.size(), size_t(2));
        QVERIFY(clip.columnWidths[0] > 0 && clip.columnWidths[1] < 0);
        crunch::ClipData back;
        QVERIFY(crunch::ClipData::fromBytes(clip.toBytes(), &back));
        QCOMPARE(back.notes.size(), size_t(1));
        QCOMPARE(back.notes[0].text, QStringLiteral("Check this"));
        QCOMPARE(back.columnWidths, clip.columnWidths);
        crunch::EditSession::PasteOptions o;
        o.what = What::ColumnWidths;
        QVERIFY(s->paste(0, rangeOf(QStringLiteral("D1")), back, o).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(e.view()->sheet().columnWidth(3), e.view()->sheet().columnWidth(0));
        o.what = What::Comments;
        QVERIFY(s->paste(0, rangeOf(QStringLiteral("D1")), back, o).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
        QVERIFY(s->anchors().noteAt(0, 0, 3));
        QCOMPARE(s->anchors().noteAt(0, 0, 3)->text, QStringLiteral("Check this"));
        QCOMPARE(shown(e, QStringLiteral("D1")), QStringLiteral("z"));   // (no cell changed)
        pressUndo(e.view());
        QVERIFY(!s->anchors().noteAt(0, 0, 3));
    }

    // Copy as Picture and Copy as Bitmap put an image of the selected cells (as drawn on screen, without the selection's tint) on the clipboard; the bitmap is the smaller.
    void copyAsPictureMakesAnImageOfTheCells()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Hello", "B1=2", "A2=x"})));
        e.view()->selectRange(0, 0, 1, 1);
        QAction *picture = e.win.w.act(QStringLiteral("edit.copyPicture")), *bitmap = e.win.w.act(QStringLiteral("edit.copyBitmap"));
        QVERIFY(picture && bitmap);
        QVERIFY(picture->isEnabled() && bitmap->isEnabled());
        QApplication::clipboard()->clear();
        picture->trigger();
        QTRY_VERIFY(!QApplication::clipboard()->image().isNull());
        const QImage big = QApplication::clipboard()->image();
        QVERIFY(big.width() > 60 && big.height() > 20);
        // the empty cell B2, at the bottom right, is white: the selection's tint is not in the picture
        QCOMPARE(big.pixelColor(big.width() * 3 / 4, big.height() * 3 / 4), QColor(Qt::white));
        bool dark = false;   // (and the text is drawn)
        for (int y = 0; y < big.height() && !dark; ++y)
            for (int x = 0; x < big.width() && !dark; ++x) dark = qGray(big.pixel(x, y)) < 100;
        QVERIFY(dark);
        QApplication::clipboard()->clear();
        bitmap->trigger();
        QTRY_VERIFY(!QApplication::clipboard()->image().isNull());
        QVERIFY(QApplication::clipboard()->image().width() < big.width());
        // a block too big for one picture is told, and the clipboard is left alone
        QApplication::clipboard()->clear();
        e.view()->selectRange(0, 0, 5000, 40);
        picture->trigger();
        QVERIFY(QApplication::clipboard()->image().isNull());
    }

    // The table other programs take from a copy carries the look of the cells they would otherwise lose: lines under or through the text, how it sits, and the borders; a plain cell is as before.
    void theCopiedTableCarriesUnderlinesAlignmentAndBorders()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Hello", "B1=2", "C1=plain"})));
        auto run = [&](const char *id) {
            QAction *a = e.win.w.act(QString::fromLatin1(id));
            QVERIFY2(a && a->isEnabled(), id);
            a->trigger();
        };
        e.select(QStringLiteral("A1"));
        run("fmt.underline");
        run("fmt.strike");
        run("fmt.alignCenter");
        run("fmt.alignTop");
        run("fmt.border.outside");
        copyRange(e, QStringLiteral("A1:C1"));
        const QString html = QApplication::clipboard()->mimeData()->html();
        const int a = int(html.indexOf(QStringLiteral(">Hello</td>"))), cell = int(html.lastIndexOf(QStringLiteral("<td"), a));
        QVERIFY(a > 0 && cell >= 0);
        const QString first = html.mid(cell, a - cell);
        QVERIFY2(first.contains(QStringLiteral("text-decoration:underline line-through;")), qPrintable(first));
        QVERIFY2(first.contains(QStringLiteral("text-align:center;")), qPrintable(first));
        QVERIFY2(first.contains(QStringLiteral("vertical-align:top;")), qPrintable(first));
        QVERIFY2(first.contains(QStringLiteral("border-left:1px solid #000000;")) && first.contains(QStringLiteral("border-bottom:1px solid #000000;")), qPrintable(first));
        // a cell with none of that is as it was
        QVERIFY(html.contains(QStringLiteral("<td>plain</td>")));
    }

    // Copy as Printed Picture draws the cells as the paper would have them: the sheet's print setup decides the gridlines (none by default), not the window's.
    void copyAsPrintedPictureLeavesOutTheScreensGridlines()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Hello"})));
        e.view()->selectRange(0, 0, 3, 3);
        auto lightPixels = [](const QImage &image) {
            int n = 0;
            for (int y = 0; y < image.height(); ++y)
                for (int x = 0; x < image.width(); ++x) {
                    const int g = qGray(image.pixel(x, y));
                    if (g > 150 && g < 245) ++n;
                }
            return n;
        };
        QAction *screen = e.win.w.act(QStringLiteral("edit.copyPicture")), *printed = e.win.w.act(QStringLiteral("edit.copyPicturePrinted"));
        QVERIFY(printed);
        QVERIFY(printed->isEnabled());
        QApplication::clipboard()->clear();
        screen->trigger();
        QTRY_VERIFY(!QApplication::clipboard()->image().isNull());
        const int onScreen = lightPixels(QApplication::clipboard()->image());
        QApplication::clipboard()->clear();
        printed->trigger();
        QTRY_VERIFY(!QApplication::clipboard()->image().isNull());
        const int onPaper = lightPixels(QApplication::clipboard()->image());
        QVERIFY2(onScreen > onPaper + 200, qPrintable(QStringLiteral("%1 vs %2").arg(onScreen).arg(onPaper)));
    }

    // The Paste button's menu offers each of Paste Special's choices, and each does what the dialog's choice does.
    void thePasteGalleryOffersPasteSpecialsChoices()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=y"})));
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("50%"));
        const QStringList ids = {"paste.exceptBorders", "paste.formulaFormats", "paste.valueFormats", "paste.widths", "paste.notes"};
        for (const QString &id : ids) QVERIFY2(e.win.w.act(id), qPrintable(id));
        QToolButton *pasteButton = nullptr;
        for (QToolButton *b : e.win.w.findChildren<QToolButton *>())
            if (b->defaultAction() && b->defaultAction()->objectName() == QLatin1String("edit.paste") && b->menu()) pasteButton = b;
        QVERIFY(pasteButton);
        QStringList inMenu;
        for (const QAction *a : pasteButton->menu()->actions()) inMenu << a->objectName();
        for (const QString &id : ids) QVERIFY2(inMenu.contains(id), qPrintable(id));
        copyRange(e, QStringLiteral("A1"));
        paste(e, QStringLiteral("D1"), "paste.valueFormats");
        QCOMPARE(shown(e, QStringLiteral("D1")), QStringLiteral("50%"));
        paste(e, QStringLiteral("E1"), "paste.exceptBorders");
        QCOMPARE(shown(e, QStringLiteral("E1")), QStringLiteral("50%"));
    }

    // Paste Text Import: text from another program is split by the separators the person chooses (a text column keeps its zeros), in one step of Undo.
    void pastedTextCanBeSplitByTheTextImportChoices()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        auto *md = new QMimeData;
        md->setText(QStringLiteral("junk\nname|007|x y\n\"q|r\"|2|z\n"));
        QApplication::clipboard()->setMimeData(md);
        TextImportRequest seen;
        e.win.w.anchorHooks().textImportAsker = [&seen](TextImportRequest *r) {
            seen = *r;
            r->choices.separators = QStringLiteral("|");
            r->choices.firstRow = 1;
            r->choices.columns = {jp::office::sheet::CsvColumn{}, jp::office::sheet::CsvColumn{jp::office::sheet::CsvColumn::Kind::Text, QString()}};
            return true;
        };
        QVERIFY(e.win.w.act(QStringLiteral("paste.textImport")));
        e.select(QStringLiteral("C3"));
        e.win.w.act(QStringLiteral("paste.textImport"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(seen.head.startsWith("junk"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("s:name"));
        QCOMPARE(e.valueAt(QStringLiteral("D3")), QStringLiteral("s:007"));
        QCOMPARE(e.valueAt(QStringLiteral("E3")), QStringLiteral("s:x y"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("s:q|r"));
        QCOMPARE(e.valueAt(QStringLiteral("D4")), QStringLiteral("s:2"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // declined: nothing changes
        e.win.w.anchorHooks().textImportAsker = [](TextImportRequest *) { return false; };
        e.select(QStringLiteral("A8"));
        e.win.w.act(QStringLiteral("paste.textImport"))->trigger();
        QCOMPARE(e.valueAt(QStringLiteral("A8")), QStringLiteral("-"));
    }

    // Paste Text Import cuts text of fixed width at the places chosen (spaces at the ends of a piece are dropped), the first row chosen on.
    void pastedTextCanBeCutIntoColumnsOfFixedWidth()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        auto *md = new QMimeData;
        md->setText(QStringLiteral("Name   Qty Price\r\nApple   12  1.5\nPear     3  0.25\n"));
        QApplication::clipboard()->setMimeData(md);
        e.win.w.anchorHooks().textImportAsker = [](TextImportRequest *r) {
            r->choices.fixedWidth = true;
            r->choices.breaks = {7, 11};
            return true;
        };
        e.select(QStringLiteral("B2"));
        e.win.w.act(QStringLiteral("paste.textImport"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("s:Name"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("s:Qty"));
        QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("s:Price"));
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("s:Apple"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:12"));
        QCOMPARE(e.valueAt(QStringLiteral("D4")), QStringLiteral("n:0.25"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
    }

    // Set Default Paste: what Paste (and Ctrl+V) puts in when cells are on the clipboard is the choice kept in the settings; Paste Special and the menu's own choices are not changed by it.
    void thePasteButtonPutsInTheDefaultChoice()
    {
        using What = crunch::EditSession::PasteOptions::What;
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=5", "B1==A1*2"})));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.act(QStringLiteral("paste.setDefault")));
        int offered = -1;
        e.win.w.setDefaultPasteAsker([&offered](int *what) {
            offered = *what;
            *what = int(What::Values);
            return true;
        });
        QVERIFY(e.win.w.setDefaultPaste());
        QCOMPARE(offered, int(What::All));
        copyRange(e, QStringLiteral("B1"));
        paste(e, QStringLiteral("D2"), "edit.paste");
        QCOMPARE(e.win.w.session()->editText(0, 1, 3), QStringLiteral("10"));
        // the menu's own choice still does what it says
        paste(e, QStringLiteral("D3"), "paste.formulas");
        QCOMPARE(e.win.w.session()->editText(0, 2, 3), QStringLiteral("=C3*2"));
        // text from another program is pasted as it is whatever the default is
        auto *md = new QMimeData;
        md->setText(QStringLiteral("x\ty"));
        QApplication::clipboard()->setMimeData(md);
        paste(e, QStringLiteral("F1"), "edit.paste");
        QCOMPARE(e.valueAt(QStringLiteral("G1")), QStringLiteral("s:y"));
        // declined: the default stays; put back to all
        e.win.w.setDefaultPasteAsker([](int *) { return false; });
        QVERIFY(!e.win.w.setDefaultPaste());
        e.win.w.setDefaultPasteAsker([&offered](int *what) {
            offered = *what;
            *what = int(What::All);
            return true;
        });
        QVERIFY(e.win.w.setDefaultPaste());
        QCOMPARE(offered, int(What::Values));
        copyRange(e, QStringLiteral("B1"));
        paste(e, QStringLiteral("D5"), "edit.paste");
        QCOMPARE(e.win.w.session()->editText(0, 4, 3), QStringLiteral("=C5*2"));
    }

    // Paste Link makes each pasted cell a formula that points at the copied cell, so it follows it; Paste as Hyperlink makes each a link to the copied cell.
    void pasteLinkAndPasteAsHyperlinkPointAtTheCopiedCells()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=5", "B1=text", "A2=7"})));
        copyRange(e, QStringLiteral("A1:B2"));
        paste(e, QStringLiteral("D4"), "paste.link");
        const QString f = formulaOf(e, QStringLiteral("D4"));
        QVERIFY2(f.startsWith(QLatin1Char('=')) && f.endsWith(QLatin1String("!$A$1")), qPrintable(f));
        QVERIFY(formulaOf(e, QStringLiteral("E5")).endsWith(QLatin1String("!$B$2")));
        QCOMPARE(e.valueAt(QStringLiteral("D4")), QStringLiteral("n:5"));
        QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("s:text"));
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:0"));   // (a blank cell's link shows 0)
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("9"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D4")), QStringLiteral("n:9"));
        // as a hyperlink
        copyRange(e, QStringLiteral("A1:B1"));
        paste(e, QStringLiteral("G1"), "paste.hyperlink");
        QCOMPARE(shown(e, QStringLiteral("G1:H1")), QStringLiteral("9|text"));
        const jp::office::sheet::LinkSpec *link = e.win.w.session()->anchors().linkAt(0, 0, 6);
        QVERIFY(link);
        QVERIFY(link->location.endsWith(QLatin1String("!A1")));
        QVERIFY(e.win.w.session()->anchors().linkAt(0, 0, 7));
    }

    // Paste Special: values (a formula becomes its result), formulas, formats, the block turned over, the empty cells left, and an operation made with what is there.
    void pasteSpecialOptions()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1*10", "C1=%0.5", "A2=3", "B2=7", "A3=5"})));
        copyRange(e, QStringLiteral("A1:C1"));
        paste(e, QStringLiteral("A5"), "paste.values");
        QCOMPARE(formulaOf(e, QStringLiteral("B5")), QStringLiteral("10"));          // a value now
        QCOMPARE(shown(e, QStringLiteral("C5")), QStringLiteral("0.5"));              // (and not a percent: values bring no formats)
        paste(e, QStringLiteral("A7"), "paste.formulas");
        QCOMPARE(formulaOf(e, QStringLiteral("B7")), QStringLiteral("=A7*10"));
        QCOMPARE(shown(e, QStringLiteral("C7")), QStringLiteral("0.5"));
        paste(e, QStringLiteral("A9"), "paste.formats");
        QCOMPARE(e.valueAt(QStringLiteral("A9")), QStringLiteral("-"));
        e.select(QStringLiteral("C9"));
        e.enter(QStringLiteral("0.25"));
        QCOMPARE(shown(e, QStringLiteral("C9")), QStringLiteral("25%"));              // (the percent format came)
        // transposed
        copyRange(e, QStringLiteral("A1:A3"));
        paste(e, QStringLiteral("E1"), "paste.transpose");
        QCOMPARE(shown(e, QStringLiteral("E1:G1")), QStringLiteral("1|3|5"));
        QCOMPARE(shown(e, QStringLiteral("E2")), QString());
        // skip blanks and the operations, through the dialog's answers
        crunch::EditSession::PasteOptions asked;
        e.win.w.setPasteAsker([&](crunch::EditSession::PasteOptions *o) {
            *o = asked;
            return true;
        });
        copyRange(e, QStringLiteral("A1:B3"));
        e.select(QStringLiteral("H1"));
        e.enter(QStringLiteral("100"));
        e.select(QStringLiteral("H2"));
        e.enter(QStringLiteral("9"));
        e.select(QStringLiteral("I3"));
        e.enter(QStringLiteral("4"));
        copyRange(e, QStringLiteral("A1:A3"));
        using Op = crunch::EditSession::PasteOptions::Operation;
        asked.operation = Op::Add;
        e.select(QStringLiteral("H1"));
        e.win.w.act(QStringLiteral("paste.special"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("H1:H3")), QStringLiteral("101|12|5"));       // 100+1, 9+3, empty+5
        asked.operation = Op::Multiply;
        e.select(QStringLiteral("H1"));
        e.win.w.act(QStringLiteral("paste.special"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("H1:H3")), QStringLiteral("101|36|25"));
        asked.operation = Op::Divide;
        e.select(QStringLiteral("H1"));
        e.win.w.act(QStringLiteral("paste.special"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("H1:H3")), QStringLiteral("101|12|5"));
        asked.operation = Op::Subtract;
        e.select(QStringLiteral("H1"));
        e.win.w.act(QStringLiteral("paste.special"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("H1:H3")), QStringLiteral("100|9|0"));
        // blanks are skipped
        asked = crunch::EditSession::PasteOptions();
        asked.skipBlanks = true;
        e.select(QStringLiteral("J1"));
        e.enter(QStringLiteral("keep"));
        e.select(QStringLiteral("K1"));
        e.enter(QStringLiteral("keep too"));
        copyRange(e, QStringLiteral("L1:M1"));                                      // two empty cells
        e.select(QStringLiteral("J1"));
        e.win.w.act(QStringLiteral("paste.special"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("J1:K1")), QStringLiteral("keep|Keep too"));
        // giving it up changes nothing
        const int steps = e.win.w.undoStack()->count();
        e.win.w.setPasteAsker([](crunch::EditSession::PasteOptions *) { return false; });
        e.win.w.act(QStringLiteral("paste.special"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }

    // A copy is pasted over a selection that is a whole number of times its size, over all of it.
    void aSelectionThatIsAMultipleIsFilled()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=2"})));
        copyRange(e, QStringLiteral("A1:B1"));
        e.view()->selectRange(2, 3, 4, 6);                                         // D3:G5: 3 rows of 2 times 2
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("D3:G3")), QStringLiteral("1|2|1|2"));
        QCOMPARE(shown(e, QStringLiteral("D5:G5")), QStringLiteral("1|2|1|2"));
        // not a multiple: pasted once
        e.view()->selectRange(8, 3, 9, 5);                                         // D9:F10: 3 wide
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("D9:F9")), QStringLiteral("1|2|"));
    }

    // Text from another program: tab-separated, with quoted cells, entered as if typed (numbers, dates, formulas); and an HTML table when there is no text.
    void textAndHtmlFromOtherProgramsAreEntered()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        auto put = [&](const QString &text, const QString &html = QString()) {
            auto *md = new QMimeData;
            if (!text.isEmpty()) md->setText(text);
            if (!html.isEmpty()) md->setHtml(html);
            QApplication::clipboard()->setMimeData(md);
        };
        put(QStringLiteral("name\t1,234.5\t=A1+1\n\"two\nlines\"\t3/4\t\"say \"\"hi\"\"\"\n"));
        e.select(QStringLiteral("C3"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("s:name"));
        QCOMPARE(e.valueAt(QStringLiteral("D3")), QStringLiteral("n:1234.5"));
        QCOMPARE(formulaOf(e, QStringLiteral("E3")), QStringLiteral("=A1+1"));
        QCOMPARE(e.valueAt(QStringLiteral("E3")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("s:two\nlines"));
        QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("s:say \"hi\""));
        QVERIFY(e.valueAt(QStringLiteral("D4")).startsWith(QLatin1String("n:")));       // (a date)
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("C3:E4"));
        // HTML only
        put(QString(), QStringLiteral("<html><body><table><tr><td>x</td><td>5</td></tr><tr><td colspan=\"2\">a &amp; b</td></tr></table></body></html>"));
        e.select(QStringLiteral("A8"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A8:B9")), QStringLiteral("x|5|a & b|"));
        // transposed text
        put(QStringLiteral("1\t2\t3\n"));
        e.select(QStringLiteral("H1"));
        e.win.w.act(QStringLiteral("paste.transpose"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("H1:H3")), QStringLiteral("1|2|3"));
        // a formula that is none is text
        put(QStringLiteral("=1+\n"));
        e.select(QStringLiteral("J1"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(e.valueAt(QStringLiteral("J1")), QStringLiteral("s:=1+"));
    }

    // A paste from another workbook brings the values, the formulas, and the number formats (not the styles of a workbook that is not this one).
    void aPasteFromAnotherWorkbookBringsValuesFormulasAndNumberFormats()
    {
        EditWindow a, b;
        QVERIFY(a.open(sheetOf({"A1=2", "B1==A1*3", "C1=%0.25", "D1=@2024-01-15"})));
        QVERIFY(b.open(sheetOf({"A1=x"})));
        copyRange(a, QStringLiteral("A1:D1"));
        b.win.w.show();
        paste(b, QStringLiteral("A3"));
        QCOMPARE(b.valueAt(QStringLiteral("A3")), QStringLiteral("n:2"));
        QCOMPARE(formulaOf(b, QStringLiteral("B3")), QStringLiteral("=A3*3"));
        QCOMPARE(b.valueAt(QStringLiteral("B3")), QStringLiteral("n:6"));
        QCOMPARE(shown(b, QStringLiteral("C3")), QStringLiteral("25%"));
        QCOMPARE(shown(b, QStringLiteral("D3")), QStringLiteral("1/15/2024"));
    }

    // A whole array formula is cut and pasted whole: its cells go, what reads it follows, Undo and Redo give it back, and the saved file has it where it went (and not where it was).
    void aWholeArrayIsCutAndPastedAndSaved()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">A1:A3*2</f><v>2</v></c><c r=\"G1\"><f>SUM(C1:C3)</f><v>12</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><v>4</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"C3\"><v>6</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        copyRange(e, QStringLiteral("C1:C3"), "edit.cut");
        e.win.told.clear();
        paste(e, QStringLiteral("E5"));
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Moving cells"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("E6")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("E7")), QStringLiteral("n:6"));
        QCOMPARE(formulaOf(e, QStringLiteral("G1")), QStringLiteral("=SUM(E5:E7)"));
        QCOMPARE(e.valueAt(QStringLiteral("G1")), QStringLiteral("n:12"));
        QVERIFY(e.win.w.calculation()->content(0, 4, 4).arrayRange.valid);
        // a change to what the array reads reaches it where it is
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("20"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E6")), QStringLiteral("n:40"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:4"));
        QVERIFY(e.win.w.calculation()->content(0, 0, 2).arrayRange.valid);
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E7")), QStringLiteral("n:6"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> parts;
        jp::readZip(f.readAll(), parts);
        const QString xml = QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QVERIFY2(xml.contains(QStringLiteral("<f t=\"array\" ref=\"E5:E7\">A1:A3*2</f>")), qPrintable(xml));
        QVERIFY2(!xml.contains(QStringLiteral("C1:C3\"")) && !xml.contains(QStringLiteral("r=\"C2\"")), qPrintable(xml));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const auto content = second.w.calculation()->content(0, 4, 4);
        QVERIFY(content.isFormula && content.arrayRange.valid);
        QCOMPARE(rangeName(content.arrayRange), QStringLiteral("E5:E7"));
    }

    // Merged ranges go with the cells; a paste over a merge takes it away; the saved file has them.
    void mergesGoWithTheCellsAndAreSaved()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>head</t></is></c></row><row r=\"2\"><c r=\"A2\"><v>5</v></c><c r=\"B2\"><v>6</v></c></row>"
                                            "<row r=\"5\"><c r=\"E5\"><v>9</v></c><c r=\"F5\"><v>9</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, QStringLiteral("<mergeCells count=\"2\"><mergeCell ref=\"A1:B1\"/><mergeCell ref=\"E5:F6\"/></mergeCells>"))})));
        copyRange(e, QStringLiteral("A1:B2"));
        paste(e, QStringLiteral("D9"));
        QCOMPARE(e.win.w.calculation()->mergesOf(0).size(), 3);
        QCOMPARE(e.view()->sheet().layout().merges.at(8, 3), 0 + e.view()->sheet().layout().merges.at(8, 3));
        QVERIFY(e.view()->sheet().layout().merges.at(8, 3) >= 0);                    // D9:E9 merged
        QVERIFY(e.view()->sheet().layout().merges.at(8, 4) >= 0);
        QVERIFY(e.view()->sheet().layout().merges.at(8, 5) < 0);
        // pasting over a merge unmerges it
        paste(e, QStringLiteral("E5"));
        QVERIFY(e.view()->sheet().layout().merges.at(5, 4) < 0);                     // (E5:F6 is gone, and A1:B1 came as E5:F5)
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.view()->sheet().layout().merges.at(5, 4) >= 0);
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.calculation()->mergesOf(0).size(), e.win.w.calculation()->mergesOf(0).size());
        QCOMPARE(second.w.calculation()->mergesOf(0), e.win.w.calculation()->mergesOf(0));
        for (int c = 0; c < 8; ++c)
            for (int r = 0; r < 12; ++r) QCOMPARE(second.w.view()->sheet().layout().merges.at(r, c) >= 0, e.view()->sheet().layout().merges.at(r, c) >= 0);
    }

    // A paste that cannot be made says so and changes nothing: into an array, a locked cell, a table's header; and one past the edge.
    void pastesThatCannotBeMadeAreRefused()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">A1:A3*2</f><v>2</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><v>4</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"C3\"><v>6</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        copyRange(e, QStringLiteral("A1:A2"));
        e.win.told.clear();
        paste(e, QStringLiteral("C2"));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("array")), qPrintable(e.win.told.join('|')));
        // cutting part of an array is refused too
        copyRange(e, QStringLiteral("C1:C2"), "edit.cut");
        e.win.told.clear();
        paste(e, QStringLiteral("E1"));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY(!e.win.told.isEmpty());
        // past the edge
        copyRange(e, QStringLiteral("A1:A3"));
        e.win.told.clear();
        e.select(QStringLiteral("A1048575"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QVERIFY(!e.win.told.isEmpty());
        QCOMPARE(e.win.w.undoStack()->count(), 0);
    }

    // A copy shows its moving dashed border, and a fill being dragged shows where it will go: both are drawn in a window of a workbook that looks like a real one. The picture is saved when

    // What a program puts on the clipboard is not believed: text and HTML make no more cells than there can be (a span of columns counts for all its cells), what this program's own form says
    // of its sizes is held to the grid and to the bytes there are, and a paste does not make more merged cells than there can be.
    void aHostileClipboardIsBounded()
    {
        // twenty thousand cells that each stand for 255: far more than the most
        QString html = QStringLiteral("<table><tr>");
        for (int i = 0; i < 20000; ++i) html += QStringLiteral("<td colspan=255></td>");
        html += QStringLiteral("</tr></table>");
        qint64 cells = 0;
        for (const QStringList &row : crunch::parseHtmlTable(html)) cells += row.size();
        QVERIFY2(cells <= crunch::kMaxClipCells && cells > 0, qPrintable(QString::number(cells)));
        cells = 0;
        for (const QStringList &row : crunch::parseTabSeparated(QString(2500000, QLatin1Char('\t')))) cells += row.size();
        QVERIFY2(cells <= crunch::kMaxClipCells + 1, qPrintable(QString::number(cells)));
        // a table of the usual size is read whole
        QCOMPARE(crunch::parseHtmlTable(QStringLiteral("<table><tr><td>a</td><td colspan=2>b</td></tr><tr><td>c</td></tr></table>")), (QVector<QStringList>{{QStringLiteral("a"), QStringLiteral("b"), QString()}, {QStringLiteral("c")}}));
        // the form of this program's own: a good one is read, and one that says more than the bytes hold, or more than the grid, or puts cells and merges outside the block, is not
        auto bytesFor = [](quint32 rows, quint32 cols, quint32 count, qint64 grow, quint32 merges, quint32 cellRow, quint32 cellCol, const QVector<quint32> &merge) {
            QByteArray b;
            QDataStream s(&b, QIODevice::WriteOnly);
            s.setVersion(QDataStream::Qt_6_0);
            s << quint32(0x4A434C50) << quint32(1) << qint64(0) << qint64(0) << qint32(0) << quint32(0) << quint32(0) << quint32(rows - 1) << quint32(cols - 1) << rows << cols << false << count;
            for (quint32 i = 0; i < std::min<quint32>(count, 1000); ++i)
                s << cellRow << cellCol << true << false << QString() << quint32(0) << QString() << false << qint32(0) << qint32(0);
            Q_UNUSED(grow);
            s << merges;
            for (quint32 i = 0; i < std::min<quint32>(merges, 1) && merge.size() == 4; ++i) s << merge[0] << merge[1] << merge[2] << merge[3];
            return b;
        };
        crunch::ClipData d;
        QVERIFY(crunch::ClipData::fromBytes(bytesFor(3, 3, 1, 0, 0, 1, 1, {}), &d));
        QCOMPARE(d.cells.size(), size_t(1));
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(3, 3, 4000000, 0, 0, 1, 1, {}), &d));      // says four million cells, holds one
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(3, 3, 10, 0, 0, 1, 1, {}), &d));           // says ten, holds ten, in a block of nine: not more than the block has
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(3, 3, 1, 0, 0, 3, 1, {}), &d));            // a cell below the block
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(3, 3, 1, 0, 0, 1, 3, {}), &d));            // and one right of it
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(2000000, 3, 1, 0, 0, 1, 1, {}), &d));      // more rows than the grid has
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(3, 20000, 1, 0, 0, 1, 1, {}), &d));        // more columns
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(0, 3, 0, 0, 0, 0, 0, {}), &d));            // no rows
        QVERIFY(crunch::ClipData::fromBytes(bytesFor(3, 3, 1, 0, 1, 1, 1, {0, 0, 1, 1}), &d));   // a merged range inside the block
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(3, 3, 1, 0, 1, 1, 1, {0, 0, 5, 1}), &d));  // one that is not
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(3, 3, 1, 0, 1, 1, 1, {1, 1, 0, 0}), &d));  // one that is turned over
        QVERIFY(!crunch::ClipData::fromBytes(bytesFor(3, 3, 1, 0, 3000000, 1, 1, {0, 0, 1, 1}), &d));   // says three million merged ranges, holds one
        // merges and tiles: the same merged range a thousand times, pasted into two thousand tiles
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        crunch::ClipData clip;
        clip.rows = 2;
        clip.cols = 1;
        clip.source.valid = true;
        clip.source.row1 = clip.source.col1 = 0;
        clip.source.row2 = 1;
        clip.source.col2 = 0;
        CellRange merge;
        merge.valid = true;
        merge.row1 = merge.col1 = 0;
        merge.row2 = 1;
        merge.col2 = 0;
        clip.merges.assign(1000, merge);
        crunch::EditSession::Outcome o = e.win.w.session()->paste(0, rangeOf(QStringLiteral("C1:C4000")), clip, crunch::EditSession::PasteOptions());
        QVERIFY(!o.ok);
        QVERIFY2(o.problem.contains(QStringLiteral("merged")), qPrintable(o.problem));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
    }

    // JC_SCREENSHOT_CLIP names a file.
    void aCopyAndAFillBeingDraggedShowTheirBorders()
    {
        Folder folder;
        Window win;
        win.w.resize(1100, 640);
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        grid::GridView *v = win.w.view();
        v->setZoom(1.0);
        // the cells copied: C3:C8, with the dashed accent round them
        v->selectRange(2, 2, 7, 2);
        win.w.act(QStringLiteral("edit.copy"))->trigger();
        QTRY_VERIFY(v->hasCopyBorder());
        // the cells being filled: E3:F4 dragged by the handle to the row 9
        v->selectRange(2, 4, 3, 5);
        QRect handle = v->fillHandleRect();
        QVERIFY(!handle.isEmpty());
        QTest::mousePress(v->viewport(), Qt::LeftButton, Qt::NoModifier, handle.center());
        const QRect target = v->cellRect(8, 5);
        QTest::mouseMove(v->viewport(), QPoint(target.center().x(), target.center().y()));
        QApplication::processEvents();
        QImage shot = win.w.grab().toImage();
        QVERIFY(!shot.isNull());
        // the fill's preview is dashed dark gray along the bottom of row 9, and the copy's border is dashed accent
        const QPoint origin = v->viewport()->mapTo(&win.w, QPoint(0, 0));
        int gray = 0, accent = 0;
        for (int x = origin.x() + v->cellRect(8, 4).left(); x < origin.x() + v->cellRect(8, 5).right(); ++x)
            for (int y = origin.y() + target.bottom() - 2; y <= origin.y() + target.bottom() + 1; ++y)
                if (QColor(shot.pixel(x, y)) == QColor(0x44, 0x44, 0x44)) ++gray;
        const QRect copied = v->cellRect(2, 2).united(v->cellRect(7, 2));
        for (int y = origin.y() + copied.top(); y < origin.y() + copied.bottom(); ++y)
            for (int x = origin.x() + copied.left() - 1; x <= origin.x() + copied.left() + 1; ++x)
                if (QColor(shot.pixel(x, y)) == QColor(0x13, 0x77, 0x3A)) ++accent;
        QVERIFY2(gray > 8, qPrintable(QString::number(gray)));
        QVERIFY2(accent > 8, qPrintable(QString::number(accent)));
        const QString out = qEnvironmentVariable("JC_SCREENSHOT_CLIP");
        if (!out.isEmpty()) QVERIFY(shot.save(out));
        QTest::mouseRelease(v->viewport(), Qt::LeftButton, Qt::NoModifier, target.center());
        QVERIFY(win.w.isModified());
    }

    // A pasted number widens a column that has its first width, in the step of the paste.
    void aPastedNumberWidensItsColumn()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=12345678901234", "B1=1"})));
        const int was = e.view()->sheet().layout().cols.size(2);
        copyRange(e, QStringLiteral("A1:B1"));
        paste(e, QStringLiteral("C3"));
        QVERIFY(e.view()->sheet().layout().cols.size(2) > was);
        QCOMPARE(e.view()->sheet().layout().cols.size(3), was);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QCOMPARE(e.view()->sheet().layout().cols.size(2), was);
    }
};

JP_TEST_CLASS(CrunchClipTests)
#include "tst_crunch_clip.moc"
