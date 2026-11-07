// Tests of the cell and shape items added in round 12: the Scenario Manager, grouping and aligning shapes, gradient fills in cells, and formatting inside one cell's text. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "office/package.h"
#include "savedok.h"
#include "findbooks.h"
#include "grid/gridview.h"
#include "core/sheetobjects.h"
#include "sheet/drawing.h"
#include "sheet/calculation.h"
#include "app/celleditor.h"
#include "app/objectpicker.h"
#include "core/editsession.h"
#include "core/objectgroup.h"
#include "core/scenarios.h"
#include "core/objectstate.h"
#include "app/scenariodialogs.h"

#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

const char *const kScenarios =
    "<scenarios current=\"0\" show=\"0\" sqref=\"A1:A2\">"
    "<scenario name=\"Low\" locked=\"1\" count=\"2\" user=\"Pat\" comment=\"The cautious plan\"><inputCells r=\"A1\" val=\"1\"/><inputCells r=\"A2\" val=\"2\"/></scenario>"
    "<scenario name=\"High\" count=\"2\" user=\"Pat\"><inputCells r=\"A1\" val=\"10\"/><inputCells r=\"A2\" val=\"20\"/></scenario>"
    "</scenarios>";

// A sheet with two inputs and a formula that sums them.
QByteArray inputsBook(const QString &after = QString())
{
    return bookOfSheets({{"A1=3", "A2=4", "B1==A1+A2"}}, {{0, after}});
}

QByteArray savedSheetXml(const QString &path)
{
    jp::office::Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return QByteArray();
    return p.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
}

int liveSheets(CrunchWindow &w)
{
    int n = 0;
    for (const auto &s : w.loaded()->book->info().sheets)
        if (!s.deleted) ++n;
    return n;
}

// Three rectangles on a sheet, placed apart from each other (a new shape goes to the middle of the cells in view): the ids, back to front.
std::vector<int> threeShapes(EditWindow &e)
{
    CrunchWindow &w = e.win.w;
    const double places[3][2] = {{20, 20}, {300, 140}, {560, 260}};
    for (const auto &place : places) {
        if (!w.insertShape(QStringLiteral("rect"))) return {};
        const int id = w.objects()->layer()->selectedId();
        grid::ObjectBox box = w.objects()->layer()->boxNow(id);
        box.x = place[0];
        box.y = place[1];
        jp::office::sheet::DrawingAnchor anchor;
        w.objects()->layer()->attach(&e.view()->sheet().layout());
        if (!w.objects()->layer()->anchorAtBox(box, jp::office::sheet::AnchorKind::TwoCell, &anchor)) return {};
        jp::crunch::ObjectEdit edit;
        edit.sheet = 0;
        edit.id = id;
        edit.before = w.objects()->stateOf(0, id);
        edit.after = w.objects()->stateMoved(0, id, anchor);
        if (!w.session()->changeObjects({edit}, QStringLiteral("Move")).ok) return {};
    }
    return w.objects()->idsOf(0);
}

QPoint centerOf(EditWindow &e, int id) { return e.win.w.objects()->layer()->deviceRect(id, grid::Pane::Body).center(); }

// A workbook with gradient fills: A1 a linear gradient of three stops at 90 degrees, A2 a path gradient of two, A3 plain.
QByteArray gradientBook()
{
    const QString xfs = QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" fillId=\"2\" applyFill=\"1\"/><xf numFmtId=\"0\" fillId=\"3\" applyFill=\"1\"/>");
    const QString after = QStringLiteral(
        "<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
        "<fills count=\"4\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
        "<fill><gradientFill degree=\"90\"><stop position=\"0\"><color rgb=\"FFFF0000\"/></stop><stop position=\"0.5\"><color rgb=\"FF00FF00\"/></stop><stop position=\"1\"><color rgb=\"FF0000FF\"/></stop></gradientFill></fill>"
        "<fill><gradientFill type=\"path\" left=\"0.5\" right=\"0.5\" top=\"0.5\" bottom=\"0.5\"><stop position=\"0\"><color rgb=\"FFFFFFFF\"/></stop><stop position=\"1\"><color rgb=\"FF000000\"/></stop></gradientFill></fill></fills>");
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\" s=\"2\"><v>2</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c></row>");
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), stylesXml(QString(), xfs, after));
}

jp::office::sheet::FillSpec fillAt(CrunchWindow &w, quint32 row, quint32 col)
{
    const auto &styles = w.session()->workbook()->styles();
    return styles.fill(styles.format(w.session()->calculation()->content(0, row, col).style).fill);
}

} // namespace

class CrunchScenarioShapeTests : public QObject {
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
    }

    // ---- Scenario Manager

    // The scenarios a file has are read (names, who made them, the notes, locked, and each changing cell with its value) and written back where the format puts them: after the cells and
    // before the filter.
    void scenariosInAFileAreReadAndSavedAtTheirPlace()
    {
        EditWindow e;
        QVERIFY(e.open(inputsBook(QString::fromLatin1(kScenarios) + QStringLiteral("<autoFilter ref=\"A1:B2\"/>"))));
        QVERIFY(e.settle());
        const auto &list = e.win.w.session()->scenariosOf(0);
        QCOMPARE(list.list.size(), 2);
        QCOMPARE(list.list[0].name, QStringLiteral("Low"));
        QCOMPARE(list.list[0].user, QStringLiteral("Pat"));
        QCOMPARE(list.list[0].comment, QStringLiteral("The cautious plan"));
        QVERIFY(list.list[0].locked);
        QCOMPARE(list.list[1].cells.size(), 2);
        QCOMPARE(list.list[1].cells[1].ref, QStringLiteral("A2"));
        QCOMPARE(list.list[1].cells[1].value, QStringLiteral("20"));
        QCOMPARE(list.sqref, QStringLiteral("A1:A2"));
        // one more, then saved: the three are in the file, before the filter
        QVERIFY(e.win.w.addScenario(QStringLiteral("Mid"), QStringLiteral("A1:A2"), QStringLiteral("In between"), {QStringLiteral("5"), QStringLiteral("6")}));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000) && saved.last().value(1).toBool());
        const QByteArray xml = savedSheetXml(e.path);
        const int data = xml.indexOf("</sheetData>"), scenarios = xml.indexOf("<scenarios"), filter = xml.indexOf("<autoFilter");
        QVERIFY(data > 0 && scenarios > data && filter > scenarios);
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const auto &back = second.w.session()->scenariosOf(0);
        QCOMPARE(back.list.size(), 3);
        QCOMPARE(back.list[2].name, QStringLiteral("Mid"));
        QCOMPARE(back.list[2].comment, QStringLiteral("In between"));
        QCOMPARE(back.list[2].cells[0].value, QStringLiteral("5"));
        QCOMPARE(back.list[0].user, QStringLiteral("Pat"));
    }

    // The two dialogs: the list with its buttons, and the one that asks for a scenario's values.
    void theScenarioDialogsListAndAsk()
    {
        jp::office::sheet::ScenarioList list;
        list.list = {jp::office::sheet::Scenario{QStringLiteral("Low"), QStringLiteral("Pat"), QStringLiteral("The cautious plan"), false, false, {{QStringLiteral("B1"), QStringLiteral("1")}, {QStringLiteral("B2"), QStringLiteral("2")}}},
                     jp::office::sheet::Scenario{QStringLiteral("High"), QString(), QString(), false, false, {{QStringLiteral("B1"), QStringLiteral("10")}}}};
        jp::ScenarioManagerDialog manager;
        manager.setScenarios(list);
        manager.show();
        QCOMPARE(manager.currentIndex(), 0);
        QVERIFY(manager.findChild<QPushButton *>(QStringLiteral("scenarioShow"))->isEnabled());
        saveShot(&manager, QStringLiteral("scenario-manager"));
        manager.setScenarios(jp::office::sheet::ScenarioList());
        QVERIFY(!manager.findChild<QPushButton *>(QStringLiteral("scenarioShow"))->isEnabled());
        QVERIFY(!manager.findChild<QPushButton *>(QStringLiteral("scenarioSummary"))->isEnabled());
        jp::ScenarioEditDialog edit(true, [](const QString &ref) { return ref == QStringLiteral("B2") ? QStringLiteral("5") : QStringLiteral("0"); });
        edit.setName(QStringLiteral("Mid"));
        edit.findChild<QLineEdit *>(QStringLiteral("scenarioCells"))->setText(QStringLiteral("B1:B2, D4"));
        QCOMPARE(edit.values(), (QStringList{QStringLiteral("0"), QStringLiteral("5"), QStringLiteral("0")}));
        edit.show();
        saveShot(&edit, QStringLiteral("scenario-add"));
        edit.findChild<QLineEdit *>(QStringLiteral("scenarioCells"))->setText(QStringLiteral("nonsense"));
        QCOMPARE(edit.values().size(), 0);
    }

    // The cells a person types for a scenario: names and ranges, apart by commas, semicolons, or spaces; none twice; at most 32.
    void changingCellsAreReadFromWhatIsTyped()
    {
        QStringList cells;
        QString why;
        QVERIFY(jp::crunch::parseScenarioCells(QStringLiteral("b1:b3, $D$5;b2"), &cells, &why));
        QCOMPARE(cells, (QStringList{QStringLiteral("B1"), QStringLiteral("B2"), QStringLiteral("B3"), QStringLiteral("D5")}));
        QVERIFY(!jp::crunch::parseScenarioCells(QStringLiteral("B1, hello"), &cells, &why));
        QVERIFY(why.contains(QStringLiteral("hello")));
        QVERIFY(!jp::crunch::parseScenarioCells(QStringLiteral("  "), &cells, &why));
        QVERIFY(jp::crunch::parseScenarioCells(QStringLiteral("A1:A32"), &cells, &why));
        QCOMPARE(int(cells.size()), 32);
        QVERIFY(!jp::crunch::parseScenarioCells(QStringLiteral("A1:A33"), &cells, &why));
        QVERIFY(!jp::crunch::parseScenarioCells(QStringLiteral("A1:A20 C1:C20"), &cells, &why));
    }

    // The changing cells of a scenario follow rows and columns inserted or deleted (a cell that is deleted drops out), in the same step as the edit, and Show and the saved file use the new cells.
    void scenarioCellsFollowRowsAndColumnsInsertedOrDeleted()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "B2=2", "C3=3", "D4=4"}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.addScenario(QStringLiteral("Set"), QStringLiteral("B2 C3 D4"), QString(), {QStringLiteral("20"), QStringLiteral("30"), QStringLiteral("40")}));
        auto refs = [&] {
            QStringList out;
            for (const auto &c : w.session()->scenariosOf(0).list[0].cells) out << c.ref;
            return out;
        };
        QVERIFY(w.session()->insertRows(0, 0, 1).ok);   // a row above them all
        QCOMPARE(refs(), (QStringList{QStringLiteral("B3"), QStringLiteral("C4"), QStringLiteral("D5")}));
        QVERIFY(w.session()->deleteColumns(0, 2, 1).ok);   // column C goes: its cell drops out, D moves to C
        QCOMPARE(refs(), (QStringList{QStringLiteral("B3"), QStringLiteral("C5")}));
        QCOMPARE(w.session()->scenariosOf(0).sqref, QStringLiteral("B3 C5"));
        // Undo puts all three cells back where they were, and Redo moves them again
        w.undoStack()->undo();
        w.undoStack()->undo();
        QCOMPARE(refs(), (QStringList{QStringLiteral("B2"), QStringLiteral("C3"), QStringLiteral("D4")}));
        w.undoStack()->redo();
        w.undoStack()->redo();
        QCOMPARE(refs(), (QStringList{QStringLiteral("B3"), QStringLiteral("C5")}));
        QVERIFY(w.showScenario(0));
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 2, 1), QStringLiteral("20"));
        QCOMPARE(w.session()->editText(0, 4, 2), QStringLiteral("40"));
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(savedOk(saved));
        QVERIFY(savedSheetXml(e.path).contains("r=\"C5\""));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.session()->scenariosOf(0).list[0].cells.size(), 2);
    }

    // The same for Insert Cells and Delete Cells (shifting down, up, right, or left).
    void scenarioCellsFollowInsertedAndDeletedCells()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "B2=2", "C3=3", "D4=4"}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.addScenario(QStringLiteral("Set"), QStringLiteral("B2 C3 D4"), QString(), {QStringLiteral("20"), QStringLiteral("30"), QStringLiteral("40")}));
        auto refs = [&] {
            QStringList out;
            for (const auto &c : w.session()->scenariosOf(0).list[0].cells) out << c.ref;
            return out;
        };
        QVERIFY(w.session()->insertCells(0, rangeOf(QStringLiteral("B1:B1")), true).ok);
        QCOMPARE(refs(), (QStringList{QStringLiteral("B3"), QStringLiteral("C3"), QStringLiteral("D4")}));
        QVERIFY(w.session()->deleteCells(0, rangeOf(QStringLiteral("C3:C3")), false).ok);   // C3 is gone, D4 does not move (row 4), the cell right of C3 in row 3 would
        QCOMPARE(refs(), (QStringList{QStringLiteral("B3"), QStringLiteral("D4")}));
        QVERIFY(w.session()->deleteCells(0, rangeOf(QStringLiteral("B3:B3")), true).ok);   // B3 is gone
        QCOMPARE(refs(), (QStringList{QStringLiteral("D4")}));
        QVERIFY(w.session()->insertCells(0, rangeOf(QStringLiteral("D1:D1")), false).ok);   // shifted right: D4 is untouched (another row)
        QCOMPARE(refs(), (QStringList{QStringLiteral("D4")}));
        QVERIFY(w.session()->insertCells(0, rangeOf(QStringLiteral("A4:C4")), false).ok);   // three cells right in row 4
        QCOMPARE(refs(), (QStringList{QStringLiteral("G4")}));
    }

    // A file that holds more scenarios than any real one is read up to a limit.
    void scenariosPastTheLimitAreLeftOut()
    {
        QString many = QStringLiteral("<scenarios>");
        for (int i = 0; i < 3000; ++i) many += QStringLiteral("<scenario name=\"S%1\" count=\"1\"><inputCells r=\"A1\" val=\"%1\"/></scenario>").arg(i);
        many += QStringLiteral("</scenarios>");
        EditWindow e;
        QVERIFY(e.open(inputsBook(many)));
        QVERIFY(e.settle());
        const int n = e.win.w.session()->scenariosOf(0).list.size();
        QVERIFY(n > 0 && n <= 1000);
    }

    // Add, edit, show, and delete: each is one step of Undo; Show puts the values into the cells.
    void scenariosAreAddedEditedShownAndDeleted()
    {
        EditWindow e;
        QVERIFY(e.open(inputsBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.act(QStringLiteral("data.scenarioManager")));
        QVERIFY(w.act(QStringLiteral("data.scenarioManager"))->isEnabled());
        const int steps = w.undoStack()->count();
        QVERIFY(w.addScenario(QStringLiteral("High"), QStringLiteral("A1:A2"), QString(), {QStringLiteral("10"), QStringLiteral("20")}));
        QCOMPARE(w.undoStack()->count(), steps + 1);
        // a name that is taken, a bad cell, and a missing name are refused
        QVERIFY(!w.addScenario(QStringLiteral("high"), QStringLiteral("A1"), QString(), {QStringLiteral("1")}));
        QVERIFY(!w.addScenario(QStringLiteral("Other"), QStringLiteral("not a cell"), QString(), {}));
        QVERIFY(!w.addScenario(QString(), QStringLiteral("A1"), QString(), {QStringLiteral("1")}));
        QCOMPARE(w.session()->scenariosOf(0).list.size(), 1);
        // Show
        QVERIFY(w.showScenario(0));
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 0, 0), QStringLiteral("10"));
        QCOMPARE(w.session()->editText(0, 1, 0), QStringLiteral("20"));
        QTRY_COMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:30"));
        pressUndo(&w);
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 0, 0), QStringLiteral("3"));
        // Edit: new values (and a note)
        QVERIFY(w.editScenario(0, QStringLiteral("High"), QStringLiteral("Aim high"), {QStringLiteral("11"), QStringLiteral("22")}));
        QCOMPARE(w.session()->scenariosOf(0).list[0].comment, QStringLiteral("Aim high"));
        QCOMPARE(w.session()->scenariosOf(0).list[0].cells[1].value, QStringLiteral("22"));
        // Delete, and Undo brings it back
        QVERIFY(w.deleteScenario(0));
        QCOMPARE(w.session()->scenariosOf(0).list.size(), 0);
        pressUndo(&w);
        QCOMPARE(w.session()->scenariosOf(0).list.size(), 1);
        QCOMPARE(w.session()->scenariosOf(0).list[0].comment, QStringLiteral("Aim high"));
    }

    // A summary puts the scenarios side by side on a new sheet, with what the result cells come to in each; the sheet's own cells are as they were.
    void scenarioSummaryMakesASheetWithTheResults()
    {
        EditWindow e;
        QVERIFY(e.open(inputsBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.addScenario(QStringLiteral("Low"), QStringLiteral("A1:A2"), QString(), {QStringLiteral("1"), QStringLiteral("2")}));
        QVERIFY(w.addScenario(QStringLiteral("High"), QStringLiteral("A1:A2"), QString(), {QStringLiteral("10"), QStringLiteral("20")}));
        const int sheets = liveSheets(w);
        QVERIFY(w.scenarioSummary(QStringLiteral("B1")));
        QVERIFY(e.settle());
        QCOMPARE(liveSheets(w), sheets + 1);
        const int made = w.activeSheet();
        QVERIFY(made > 0);
        auto text = [&](quint32 r, quint32 c) { return w.session()->editText(made, r, c); };
        QStringList all;
        for (quint32 r = 0; r < 14; ++r)
            for (quint32 c = 0; c < 6; ++c) all << text(r, c);
        QVERIFY(all.contains(QStringLiteral("Scenario Summary")));
        QVERIFY(all.contains(QStringLiteral("Low")));
        QVERIFY(all.contains(QStringLiteral("High")));
        QVERIFY(all.contains(QStringLiteral("30")));   // B1 under High
        QVERIFY(all.contains(QStringLiteral("3")));    // B1 under Low
        QVERIFY(all.contains(QStringLiteral("7")));    // the current values: 3 + 4
        // the input sheet is as it was
        QCOMPARE(w.session()->editText(0, 0, 0), QStringLiteral("3"));
        QCOMPARE(w.session()->editText(0, 1, 0), QStringLiteral("4"));
        pressUndo(&w);
        QCOMPARE(liveSheets(w), sheets);
    }

    void scenarioSummaryIsRefusedWhileASaveWaitsForTheCalculation()
    {
        EditWindow e;
        QVERIFY(e.open(inputsBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.addScenario(QStringLiteral("Low"), QStringLiteral("A1:A2"), QString(), {QStringLiteral("1"), QStringLiteral("2")}));
        w.setWorkerPace(20000);
        e.select(QStringLiteral("C5"));
        e.enter(QStringLiteral("9"));
        QVERIFY(w.isCalculating());
        QVERIFY(w.save());
        const int sheets = liveSheets(w);
        QVERIFY(!w.scenarioSummary(QStringLiteral("B1")));
        QCOMPARE(liveSheets(w), sheets);
        w.setWorkerPace(0);
        QVERIFY(e.settle());
    }

    // ---- Shapes: picking several, Align, Group, and Ungroup

    // What each Align choice does to boxes, as numbers.
    void alignPutsBoxesAtTheirEdgeOrMiddleAndDistributeSpacesThemEvenly()
    {
        using jp::crunch::ObjectAlign;
        const std::vector<jp::grid::ObjectBox> boxes = {{10, 20, 30, 40}, {100, 50, 60, 10}, {300, 5, 20, 80}};
        auto at = [&](ObjectAlign how) { return jp::crunch::alignedBoxes(boxes, how); };
        auto left = at(ObjectAlign::Left), right = at(ObjectAlign::Right), center = at(ObjectAlign::Center);
        for (size_t i = 0; i < boxes.size(); ++i) {
            QCOMPARE(left[i].x, 10.0);
            QCOMPARE(right[i].x + right[i].w, 320.0);
            QCOMPARE(center[i].x + center[i].w / 2, 165.0);
            QCOMPARE(left[i].w, boxes[i].w);
            QCOMPARE(left[i].y, boxes[i].y);
        }
        auto top = at(ObjectAlign::Top), bottom = at(ObjectAlign::Bottom), middle = at(ObjectAlign::Middle);
        for (size_t i = 0; i < boxes.size(); ++i) {
            QCOMPARE(top[i].y, 5.0);
            QCOMPARE(bottom[i].y + bottom[i].h, 85.0);
            QCOMPARE(middle[i].y + middle[i].h / 2, 45.0);
        }
        // the first and last stay; the one between is moved so that the gaps are equal: (300 - 40) - 60 = 200 of space, 100 each
        auto spread = at(ObjectAlign::DistributeHorizontally);
        QCOMPARE(spread[0].x, 10.0);
        QCOMPARE(spread[2].x, 300.0);
        QCOMPARE(spread[1].x - (spread[0].x + spread[0].w), spread[2].x - (spread[1].x + spread[1].w));
        const std::vector<jp::grid::ObjectBox> column = {{0, 0, 10, 10}, {0, 50, 10, 20}, {0, 200, 10, 40}};
        const auto down = jp::crunch::alignedBoxes(column, ObjectAlign::DistributeVertically);
        QCOMPARE(down[0].y, 0.0);
        QCOMPARE(down[2].y, 200.0);
        QCOMPARE(down[1].y, 95.0);   // (240 high in all, 70 of it objects: 85 of space between each)
    }

    // Shift or Ctrl and a click adds an object to the picked ones or takes it off; with Select Objects on, a box picks the objects inside it.
    void modifierClicksAndADragBoxPickSeveralShapes()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        const std::vector<int> ids = threeShapes(e);
        QCOMPARE(int(ids.size()), 3);
        e.win.w.objects()->layer()->prepareAll();
        QWidget *vp = e.view()->viewport();
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, centerOf(e, ids[0]));
        QCOMPARE(int(e.win.w.pickedObjects().size()), 1);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ShiftModifier, centerOf(e, ids[1]));
        QCOMPARE(int(e.win.w.pickedObjects().size()), 2);
        QCOMPARE(e.win.w.pickedObject(), -1);   // (the commands for one object wait while there are two)
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ControlModifier, centerOf(e, ids[2]));
        QCOMPARE(int(e.win.w.pickedObjects().size()), 3);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ControlModifier, centerOf(e, ids[0]));
        QCOMPARE(e.win.w.pickedObjects(), (std::vector<int>{ids[1], ids[2]}));
        // a click with no modifier picks that one alone
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, centerOf(e, ids[2]));
        QCOMPARE(e.win.w.pickedObjects(), (std::vector<int>{ids[2]}));
        QCOMPARE(e.win.w.pickedObject(), ids[2]);
        // a box
        e.win.w.pickObjects({});
        e.win.w.setSelectObjectsMode(true);
        const QRect a = e.win.w.objects()->layer()->deviceRect(ids[0], grid::Pane::Body), b = e.win.w.objects()->layer()->deviceRect(ids[1], grid::Pane::Body);
        const QPoint from(a.left() - 6, a.top() - 6), to(b.right() + 6, b.bottom() + 6);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(vp, to);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(e.win.w.pickedObjects(), (std::vector<int>{ids[0], ids[1]}));
        e.win.w.setSelectObjectsMode(false);
    }

    // Align moves the picked objects (one step of Undo each time), and its commands are dimmed until there are objects to line up.
    void alignMovesThePickedShapesAsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        CrunchWindow &w = e.win.w;
        QVERIFY(!w.act(QStringLiteral("object.alignLeft"))->isEnabled());
        const std::vector<int> ids = threeShapes(e);
        QCOMPARE(int(ids.size()), 3);
        w.pickObjects({ids[0]});
        QVERIFY(!w.act(QStringLiteral("object.alignLeft"))->isEnabled());
        w.pickObjects(ids);
        w.resize(1100, 640);
        w.show();
        w.objects()->layer()->prepareAll();
        saveShot(&w, QStringLiteral("shapes-picked-together"));
        QVERIFY(w.act(QStringLiteral("object.alignLeft"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("object.distributeH"))->isEnabled());
        auto box = [&](int id) { return w.objects()->layer()->boxNow(id); };
        const double x0 = box(ids[0]).x, y2 = box(ids[2]).y + box(ids[2]).h;
        const int steps = w.undoStack()->count();
        QVERIFY(w.alignObjects(jp::crunch::ObjectAlign::Left));
        QCOMPARE(w.undoStack()->count(), steps + 1);
        for (const int id : ids) QVERIFY(qAbs(box(id).x - x0) < 1.5);
        pressUndo(&w);
        QVERIFY(box(ids[1]).x > x0 + 20);
        QVERIFY(w.alignObjects(jp::crunch::ObjectAlign::Bottom));
        for (const int id : ids) QVERIFY(qAbs(box(id).y + box(id).h - y2) < 1.5);
        // the middle one is moved to the middle of the other two
        pressUndo(&w);
        QVERIFY(w.alignObjects(jp::crunch::ObjectAlign::DistributeHorizontally));
        const double gap1 = box(ids[1]).x - (box(ids[0]).x + box(ids[0]).w), gap2 = box(ids[2]).x - (box(ids[1]).x + box(ids[1]).w);
        QVERIFY(qAbs(gap1 - gap2) < 1.5);
        // it is saved where it was put
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(savedOk(saved));
    }

    // Group puts the picked shapes in one xdr:grpSp (one step of Undo), the file has it, and it reads back as one group; Ungroup gives the shapes back where they were.
    void groupAndUngroupKeepEachShapeWhereItIs()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        CrunchWindow &w = e.win.w;
        const std::vector<int> ids = threeShapes(e);
        QCOMPARE(int(ids.size()), 3);
        QVERIFY(!w.act(QStringLiteral("object.group"))->isEnabled());
        w.objects()->layer()->prepareAll();
        std::vector<jp::grid::ObjectBox> boxes;
        for (const int id : ids) boxes.push_back(w.objects()->layer()->boxNow(id));
        w.pickObjects(ids);
        QVERIFY(w.act(QStringLiteral("object.group"))->isEnabled());
        const int steps = w.undoStack()->count();
        QVERIFY(w.groupObjects());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(int(w.objects()->idsOf(0).size()), 1);
        const int group = w.objects()->idsOf(0).front();
        QCOMPARE(w.objects()->kindOf(0, group), jp::office::sheet::ObjectKind::Group);
        QCOMPARE(w.pickedObjects(), (std::vector<int>{group}));
        QVERIFY(w.act(QStringLiteral("object.ungroup"))->isEnabled());
        const jp::grid::ObjectBox gb = w.objects()->layer()->boxNow(group);
        QVERIFY(qAbs(gb.x - boxes[0].x) < 1.5 && qAbs(gb.y - boxes[0].y) < 1.5);
        QVERIFY(gb.x + gb.w >= boxes[2].x + boxes[2].w - 1.5 && gb.y + gb.h >= boxes[2].y + boxes[2].h - 1.5);
        // saved as a group of three shapes
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(savedOk(saved));
        jp::office::Package p;
        QString why;
        QVERIFY(p.readLazyFile(e.path, &why));
        const QByteArray drawing = p.part(QStringLiteral("/xl/drawings/drawing1.xml"));
        QCOMPARE(drawing.count("<xdr:grpSp>") + drawing.count("<xdr:grpSp "), 1);
        QCOMPARE(drawing.count("<xdr:sp "), 3);
        QCOMPARE(drawing.count("<xdr:twoCellAnchor"), 1);
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0);
        const jp::crunch::DrawingCounts c = second.w.objects()->counts(0);
        QCOMPARE(c.groups, 1);
        // Ungroup, and each is back where it was
        w.pickObjects({group});
        QVERIFY(w.ungroupObjects());
        const std::vector<int> back = w.objects()->idsOf(0);
        QCOMPARE(int(back.size()), 3);
        for (size_t i = 0; i < back.size(); ++i) {
            const jp::grid::ObjectBox b = w.objects()->layer()->boxNow(back[i]);
            QVERIFY(qAbs(b.x - boxes[i].x) < 1.5 && qAbs(b.y - boxes[i].y) < 1.5 && qAbs(b.w - boxes[i].w) < 1.5 && qAbs(b.h - boxes[i].h) < 1.5);
            QCOMPARE(w.objects()->kindOf(0, back[i]), jp::office::sheet::ObjectKind::Shape);
        }
        // and Undo brings the group back
        w.undoStack()->undo();
        QCOMPARE(int(w.objects()->idsOf(0).size()), 1);
    }

    // The XML of a group: members' elements inside xdr:grpSp, each with its place written in; taking it apart gives the same boxes.
    void aGroupClipHoldsEachMemberAtItsPlaceAndSplitsBack()
    {
        auto member = [](const char *name, double x, double y, double w, double h) {
            jp::crunch::GroupMember m;
            m.clip.kind = QStringLiteral("shape");
            m.clip.elementXml = QByteArray("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"2\" name=\"") + name
                                + "\"/><xdr:cNvSpPr/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"10\" cy=\"10\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:sp>";
            m.clip.width = w;
            m.clip.height = h;
            m.box = {x, y, w, h};
            return m;
        };
        const std::vector<jp::crunch::GroupMember> members = {member("One", 10, 20, 30, 40), member("Two", 100, 60, 50, 20)};
        jp::crunch::ObjectClip group;
        jp::grid::ObjectBox box;
        QString why;
        QVERIFY2(jp::crunch::buildGroupClip(members, &group, &box, &why), qPrintable(why));
        QCOMPARE(group.kind, QStringLiteral("group"));
        QVERIFY(group.valid());
        QCOMPARE(box.x, 10.0);
        QCOMPARE(box.y, 20.0);
        QCOMPARE(box.w, 140.0);
        QCOMPARE(box.h, 60.0);
        QVERIFY(group.elementXml.startsWith("<xdr:grpSp"));
        QCOMPARE(group.elementXml.count("<xdr:sp "), 2);
        // a member's offset is where it is on the sheet, in EMU (9525 to a unit)
        for (const char *want : {"x=\"95250\"", "y=\"190500\"", "cx=\"285750\"", "cy=\"381000\""}) QVERIFY2(group.elementXml.contains(want), want);
        std::vector<jp::crunch::GroupMember> parts;
        QVERIFY2(jp::crunch::splitGroupClip(group, box, &parts, &why), qPrintable(why));
        QCOMPARE(int(parts.size()), 2);
        QVERIFY(parts[0].clip.elementXml.contains("name=\"One\""));
        QVERIFY(qAbs(parts[0].box.x - 10) < 0.01 && qAbs(parts[0].box.y - 20) < 0.01 && qAbs(parts[0].box.w - 30) < 0.01 && qAbs(parts[0].box.h - 40) < 0.01);
        QVERIFY(qAbs(parts[1].box.x - 100) < 0.01 && qAbs(parts[1].box.y - 60) < 0.01 && qAbs(parts[1].box.w - 50) < 0.01);
        // the group moved to the right by 5: the members come out 5 to the right
        std::vector<jp::crunch::GroupMember> moved;
        QVERIFY(jp::crunch::splitGroupClip(group, {box.x + 5, box.y, box.w, box.h}, &moved, &why));
        QVERIFY(qAbs(moved[0].box.x - 15) < 0.01);
        // one member, or a chart, is no group
        QVERIFY(!jp::crunch::buildGroupClip({members[0]}, &group, &box, &why));
        jp::crunch::ObjectClip none;
        QVERIFY(!jp::crunch::splitGroupClip(none, box, &parts, &why));
    }

    // ---- Gradient fills

    // A gradient fill keeps its stops, its angle, and its kind through the reader; the first stop is still the color of a plain reading of it.
    void gradientFillsKeepTheirStopsAndAngle()
    {
        EditWindow e;
        QVERIFY(e.open(gradientBook()));
        QVERIFY(e.settle());
        const jp::office::sheet::FillSpec linear = fillAt(e.win.w, 0, 0), path = fillAt(e.win.w, 1, 0);
        QVERIFY(linear.gradient);
        QCOMPARE(int(linear.stops.size()), 3);
        QCOMPARE(linear.stops[1].position, 0.5);
        QCOMPARE(linear.stops[1].color.rgb, 0x00FF00u);
        QCOMPARE(linear.stops[2].color.rgb, 0x0000FFu);
        QCOMPARE(linear.degree, 90.0);
        QVERIFY(!linear.path);
        QCOMPARE(linear.fg.rgb, 0xFF0000u);
        QVERIFY(path.gradient && path.path);
        QCOMPARE(int(path.stops.size()), 2);
        QCOMPARE(path.left, 0.5);
        QCOMPARE(path.bottom, 0.5);
        QVERIFY(!fillAt(e.win.w, 2, 0).gradient);
    }

    // A gradient made in Format Cells is written as a gradientFill, and a workbook that had gradients keeps them when it is saved with a change elsewhere.
    void gradientFillsAreWrittenAndKept()
    {
        EditWindow e;
        QVERIFY(e.open(gradientBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        // a gradient put on A3 (what the dialog gives): two colors, from top to bottom
        jp::office::sheet::FormatChange change;
        jp::office::sheet::FillSpec fill;
        fill.pattern = jp::office::sheet::PatternKind::Solid;
        fill.gradient = true;
        fill.degree = 90;
        fill.stops = {{0, jp::office::sheet::ColorSpec::fromRgb(0x112233)}, {1, jp::office::sheet::ColorSpec::fromRgb(0xAABBCC)}};
        fill.fg = fill.stops.first().color;
        fill.bg = fill.stops.last().color;
        change.fill = fill;
        QVERIFY(w.session()->formatCells(0, rangeOf(QStringLiteral("A3")), change, QStringLiteral("Fill")).ok);
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(savedOk(saved));
        jp::office::Package p;
        QString why;
        QVERIFY(p.readLazyFile(e.path, &why));
        const QByteArray styles = p.part(QStringLiteral("/xl/styles.xml"));
        QCOMPARE(styles.count("<gradientFill"), 3);   // (the two the file had, and the new one)
        QVERIFY(styles.contains("112233") && styles.contains("AABBCC"));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const jp::office::sheet::FillSpec back = fillAt(second.w, 2, 0), kept = fillAt(second.w, 0, 0);
        QVERIFY(back.gradient);
        QCOMPARE(int(back.stops.size()), 2);
        QCOMPARE(back.stops[1].color.rgb, 0xAABBCCu);
        QCOMPARE(back.degree, 90.0);
        QCOMPARE(int(kept.stops.size()), 3);   // (the file's own, as it was)
    }

    // The grid draws a gradient across the cell: the two ends differ, and a plain cell beside it is one color.
    void aGradientIsDrawnAcrossTheCell()
    {
        EditWindow e;
        QVERIFY(e.open(gradientBook()));
        QVERIFY(e.settle());
        e.win.w.resize(900, 500);
        e.win.w.show();
        QTRY_VERIFY(e.win.w.view()->isVisible());
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect cell = L.cellRect(0, 0);
        QTRY_VERIFY(!e.view()->viewport()->grab().isNull());
        const QImage img = e.view()->viewport()->grab().toImage();
        const QRgb top = img.pixel(cell.center().x(), cell.top() + 2), bottom = img.pixel(cell.center().x(), cell.bottom() - 3);
        QVERIFY2(top != bottom, "a gradient from top to bottom has two ends");
        QVERIFY(qRed(top) > qBlue(top));       // red at the start of A1's gradient (90 degrees: top to bottom)
        QVERIFY(qBlue(bottom) > qRed(bottom)); // and blue at the end
        saveShot(&e.win.w, QStringLiteral("gradient-cells"));
        const QRect plain = L.cellRect(2, 0);
        QCOMPARE(img.pixel(plain.center().x(), plain.top() + 2), img.pixel(plain.center().x(), plain.bottom() - 3));
    }

    // ---- Rich text in cells

    // Runs of formatting in a shared string and in an inline string are read; a plain cell has none.
    void richTextRunsAreReadFromSharedAndInlineStrings()
    {
        const QByteArray strings = "<?xml version=\"1.0\"?><sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"1\" uniqueCount=\"1\"><si><r><rPr><b/><sz val=\"12\"/><color rgb=\"FFFF0000\"/><rFont val=\"Arial\"/></rPr><t>Bold</t></r><r><t xml:space=\"preserve\"> plain</t></r></si></sst>";
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"s\"><v>0</v></c><c r=\"B1\" t=\"inlineStr\"><is><r><rPr><i/><sz val=\"14\"/></rPr><t>Ita</t></r><r><t>lic</t></r></is></c></row>"
                                          "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>plain</t></is></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, strings)));
        QVERIFY(e.settle());
        const auto shared = e.win.w.session()->richOf(0, 0, 0);
        QVERIFY(shared.has_value());
        QCOMPARE(shared->text, QStringLiteral("Bold plain"));
        QCOMPARE(int(shared->runs.size()), 2);
        QCOMPARE(int(shared->runs[1].start), 4);
        const jp::office::sheet::FontSpec &bold = shared->fonts[int(shared->runs[0].font)];
        QVERIFY(bold.bold);
        QCOMPARE(bold.size, 12.0);
        QCOMPARE(bold.name, QStringLiteral("Arial"));
        QCOMPARE(bold.color.rgb, 0xFF0000u);
        QCOMPARE(shared->runs[1].font, jp::office::sheet::RichRun::kCellFont);
        const auto inlined = e.win.w.session()->richOf(0, 0, 1);
        QVERIFY(inlined.has_value());
        QCOMPARE(inlined->text, QStringLiteral("Italic"));
        QVERIFY(inlined->fonts[int(inlined->runs[0].font)].italic);
        QCOMPARE(inlined->fonts[int(inlined->runs[0].font)].size, 14.0);
        QVERIFY(!e.win.w.session()->richOf(0, 1, 0).has_value());
    }

    // A person selects part of the text in the cell editor and presses Bold: that part is bold in the cell, one step of Undo, and written as runs; the plain cell beside it is written plain.
    void boldOnSelectedTextInTheCellEditorMakesRuns()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Hello world", "A2=plain"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QVERIFY(w.cellEditor()->isVisible());
        w.cellEditor()->setSelection(0, 5);
        const int steps = w.undoStack()->count();
        w.act(QStringLiteral("fmt.bold"))->trigger();
        QVERIFY(w.cellEditor()->isVisible());   // (the entry goes on)
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        const auto rich = w.session()->richOf(0, 0, 0);
        QVERIFY(rich.has_value());
        QCOMPARE(rich->text, QStringLiteral("Hello world"));
        QVERIFY(rich->runs.size() >= 2);
        QVERIFY(rich->fonts[int(rich->runs[0].font)].bold);
        QCOMPARE(int(rich->runs[0].start), 0);
        QCOMPARE(int(rich->runs[1].start), 5);
        QVERIFY(rich->runs[1].font == jp::office::sheet::RichRun::kCellFont || !rich->fonts[int(rich->runs[1].font)].bold);
        QCOMPARE(w.undoStack()->count(), steps + 1);
        // Undo takes the formatting and the entry away together, and Redo brings them back
        pressUndo(&w);
        QVERIFY(!w.session()->richOf(0, 0, 0).has_value());
        pressRedo(&w);
        QVERIFY(w.session()->richOf(0, 0, 0).has_value());
        // written as runs, and read back
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(savedOk(saved));
        const QByteArray xml = savedSheetXml(e.path);
        QVERIFY2(xml.contains("<r>") && xml.contains("<b/>"), xml.constData());
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const auto back = second.w.session()->richOf(0, 0, 0);
        QVERIFY(back.has_value() && back->text == QStringLiteral("Hello world") && back->fonts[int(back->runs[0].font)].bold);
        QVERIFY(!second.w.session()->richOf(0, 1, 0).has_value());
        QCOMPARE(w.session()->editText(0, 0, 0), QStringLiteral("Hello world"));
    }

    // The grid draws the runs: the bold word is wider than the same word plain.
    void aCellWithRunsIsDrawnWithEachRunInItsOwnFont()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Hello world", "A2=Hello world"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        jp::office::sheet::RichText rich;
        rich.text = QStringLiteral("Hello world");
        jp::office::sheet::FontSpec red;
        red.name = QStringLiteral("Calibri");
        red.size = 11;
        red.bold = true;
        red.color = jp::office::sheet::ColorSpec::fromRgb(0xFF0000);
        rich.fonts = {red};
        rich.runs = {{0, 0}, {5, jp::office::sheet::RichRun::kCellFont}};
        QVERIFY(w.session()->setRichText(0, 0, 0, rich, QStringLiteral("Format Text")).ok);
        w.resize(900, 500);
        w.show();
        QTRY_VERIFY(w.view()->isVisible());
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect a1 = L.cellRect(0, 0);
        QTRY_VERIFY(!e.view()->viewport()->grab().isNull());
        const QImage img = e.view()->viewport()->grab().toImage();
        bool anyRed = false;
        for (int y = a1.top(); y < a1.bottom() && !anyRed; ++y)
            for (int x = a1.left(); x < a1.left() + 40 && !anyRed; ++x) anyRed = qRed(img.pixel(x, y)) > 180 && qGreen(img.pixel(x, y)) < 90 && qBlue(img.pixel(x, y)) < 90;
        QVERIFY2(anyRed, "the first word is red");
        saveShot(&w, QStringLiteral("rich-text-cell"));
    }
};

JP_TEST_CLASS(CrunchScenarioShapeTests)
#include "tst_crunch_c12cells.moc"
