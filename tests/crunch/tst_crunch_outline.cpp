// Tests of the outline of rows and columns: which groups there are, what collapsing, expanding, showing a level, grouping, and ungrouping change, how the session makes those changes (one step
// each, saved with the levels the file says the outline goes to), and the buttons in the margins. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/outlinegutter.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/outline.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "office/package.h"
#include "savedok.h"
#include "testrunner.h"

#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>
#include <QtTest>

using namespace jp;
using namespace crunchtest;
using jp::crunch::Outline;
using jp::crunch::OutlineChange;
using jp::crunch::OutlineLine;

namespace {

// Rows 1 to 8: row 1 a heading; rows 2 to 5 a group (level 1) with rows 3 and 4 a group inside it (level 2); row 5 is the inside group's summary and row 6 the outside group's; rows 7 and 8 plain.
// (Indexes from 0: the lines 1 to 4 have levels 1, 2, 2, 1.)
std::vector<OutlineLine> nested() { return {{1, 1, false, false}, {2, 2, false, false}, {3, 2, false, false}, {4, 1, false, false}}; }

QByteArray outlineSheet(bool summaryBelow = true)
{
    QString rows;
    const char *attrs[] = {"", " outlineLevel=\"1\"", " outlineLevel=\"2\"", " outlineLevel=\"2\"", " outlineLevel=\"1\"", "", "", ""};
    for (int r = 0; r < 8; ++r) rows += QStringLiteral("<row r=\"%1\"%2><c r=\"A%1\"><v>%3</v></c></row>").arg(r + 1).arg(QString::fromLatin1(attrs[r])).arg(r + 1);
    QString head = summaryBelow ? QString() : QStringLiteral("<sheetPr><outlinePr summaryBelow=\"0\" summaryRight=\"0\"/></sheetPr>");
    head += QStringLiteral("<sheetFormatPr defaultRowHeight=\"15\" outlineLevelRow=\"2\"/>");
    return buildXlsx({worksheet(rows, QString(), head)});
}

struct State {
    bool hidden, collapsed;
    int level;
};

State rowState(EditWindow &e, quint32 row)
{
    for (const auto &r : e.win.w.session()->calculation()->rowSpecsIn(0, row, row))
        if (r.row == row) return {r.hidden, r.collapsed, r.outline};
    return {false, false, 0};
}

QByteArray savedSheet(EditWindow &e)
{
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.save() || !savedOk(saved, 20000)) return QByteArray();
    Package pk;
    QString why;
    if (!pk.readLazyFile(e.path, &why)) return QByteArray();
    return pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
}

} // namespace

class CrunchOutlineTests : public QObject {
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

    // The groups are the runs of lines of a level or deeper; a group's summary is the line after it (before it where the sheet puts its summaries above), and says whether it is collapsed.
    void groupsAreRunsOfLevels()
    {
        const Outline o(nested(), 100, true);
        QCOMPARE(o.deepest(), 2);
        const auto all = o.groups(0, 99);
        QCOMPARE(int(all.size()), 2);
        QCOMPARE(all[0].level, 1);
        QCOMPARE(all[0].first, quint32(1));
        QCOMPARE(all[0].last, quint32(4));
        QCOMPARE(all[0].summary, qint64(5));
        QCOMPARE(all[1].level, 2);
        QCOMPARE(all[1].first, quint32(2));
        QCOMPARE(all[1].last, quint32(3));
        QCOMPARE(all[1].summary, qint64(4));
        QVERIFY(!all[0].collapsed && !all[1].collapsed);
        QCOMPARE(int(o.groups(6, 99).size()), 0);          // (none has a line there)
        QCOMPARE(int(o.groups(3, 3).size()), 2);
        const Outline above(nested(), 100, false);
        QCOMPARE(above.groups(0, 99)[0].summary, qint64(0));
        QCOMPARE(above.groups(0, 99)[1].summary, qint64(1));
        // a group at the top of the sheet with its summary above has none
        const Outline top({{0, 1, false, false}}, 10, false);
        QCOMPARE(top.groups(0, 9)[0].summary, qint64(-1));
        // a gap ends a group: two groups of the same level
        const Outline two({{1, 1, false, false}, {2, 1, false, false}, {4, 1, false, false}}, 10, true);
        QCOMPARE(int(two.groups(0, 9).size()), 2);
    }

    void collapsingAndExpandingChangeTheLinesAndTheSummary()
    {
        const Outline o(nested(), 100, true);
        const auto all = o.groups(0, 99);
        const auto inner = o.collapse(all[1]);
        QCOMPARE(int(inner.size()), 3);                       // lines 2 and 3 hidden, line 4 marked
        QCOMPARE(inner[0].index, quint32(2));
        QCOMPARE(inner[0].hidden, 1);
        QCOMPARE(inner[2].index, quint32(4));
        QCOMPARE(inner[2].collapsed, 1);
        // with the inner group collapsed, collapsing the outer one and expanding it again leaves the inner lines hidden
        std::vector<OutlineLine> state = nested();
        state[1].hidden = state[2].hidden = true;
        state[3].collapsed = true;
        const Outline innerCollapsed(state, 100, true);
        const auto groupsNow = innerCollapsed.groups(0, 99);
        QVERIFY(groupsNow[1].collapsed);
        const auto outer = innerCollapsed.collapse(groupsNow[0]);
        QVERIFY(std::any_of(outer.begin(), outer.end(), [](const OutlineChange &c) { return c.index == 1 && c.hidden == 1; }));
        QVERIFY(std::any_of(outer.begin(), outer.end(), [](const OutlineChange &c) { return c.index == 5 && c.collapsed == 1; }));
        for (OutlineLine &l : state) l.hidden = true;
        state.push_back({5, 0, false, true});
        const Outline allCollapsed(state, 100, true);
        const auto back = allCollapsed.expand(allCollapsed.groups(0, 99)[0]);
        auto find = [&](quint32 i) {
            for (const OutlineChange &c : back)
                if (c.index == i) return c;
            return OutlineChange{i, -1, -1, -1};
        };
        QCOMPARE(find(1).hidden, 0);       // the outer group's own line comes back
        QCOMPARE(find(2).hidden, -1);      // the lines of the collapsed group inside stay hidden
        QCOMPARE(find(3).hidden, -1);
        QCOMPARE(find(5).collapsed, 0);
    }

    // Expanding a group looks at each line a few times, not once for each group inside it: a group of 20,000 lines with every other line a group of its own, collapsed, takes about 20,000 looks.
    void expandingIsLinearInTheLines()
    {
        std::vector<OutlineLine> lines;
        const quint32 n = 20000;
        for (quint32 i = 0; i < n; ++i) {
            const bool inner = i % 2 == 1;
            lines.push_back({i + 1, quint8(inner ? 2 : 1), inner, false});
        }
        lines.push_back({n + 1, 0, false, true});   // (the summary of the whole, marked collapsed)
        for (OutlineLine &l : lines)
            if (l.level == 2) l.hidden = true;
        // the inner groups' summaries are the lines of level 1 after them, marked collapsed
        for (OutlineLine &l : lines)
            if (l.level == 1 && l.index > 1 && l.index <= n) l.collapsed = true;
        const Outline o(lines, 2000000, true);
        const auto all = o.groups(0, 0xFFFFFFFFu);
        QCOMPARE(all.front().level, 1);
        const auto changes = o.expand(all.front());
        QVERIFY2(o.stepsTaken() < qint64(n) * 20, qPrintable(QString::number(o.stepsTaken())));
        // the lines of the collapsed inner groups stay hidden, and the group's own summary is shown again
        QVERIFY(std::any_of(changes.begin(), changes.end(), [&](const OutlineChange &c) { return c.index == n + 1 && c.collapsed == 0; }));
        QVERIFY(std::none_of(changes.begin(), changes.end(), [](const OutlineChange &c) { return c.hidden == 0 && c.index % 2 == 0; }));
    }

    // A line that is hidden and in no group is hidden by hand: the level buttons leave it as it is.
    void theLevelButtonsLeaveLinesHiddenByHandAlone()
    {
        const Outline o({{1, 1, false, false}, {3, 1, true, false}, {4, 0, false, true}, {10, 0, true, false}}, 100, true);
        for (int button = 1; button <= 3; ++button)
            for (const OutlineChange &c : o.showButton(button)) QVERIFY2(c.index != 10, "a line in no group, hidden by hand, was changed");
        const auto three = o.showButton(3);
        QVERIFY(std::any_of(three.begin(), three.end(), [](const OutlineChange &c) { return c.index == 3 && c.hidden == 0; }));
    }

    void aLevelButtonShowsTheLinesOfLessThanThatLevel()
    {
        const Outline o(nested(), 100, true);
        auto states = [&](int button) {
            std::vector<OutlineLine> lines = nested();
            for (const OutlineChange &c : o.showButton(button)) {
                auto it = std::find_if(lines.begin(), lines.end(), [&](const OutlineLine &l) { return l.index == c.index; });
                if (it == lines.end()) {
                    lines.push_back({c.index, 0, false, false});
                    it = lines.end() - 1;
                }
                if (c.hidden >= 0) it->hidden = c.hidden != 0;
                if (c.collapsed >= 0) it->collapsed = c.collapsed != 0;
            }
            return lines;
        };
        auto lineOf = [](const std::vector<OutlineLine> &lines, quint32 i) {
            for (const OutlineLine &l : lines)
                if (l.index == i) return l;
            return OutlineLine{i, 0, false, false};
        };
        const auto one = states(1);   // only the lines with no level: all the groups collapsed
        QVERIFY(lineOf(one, 1).hidden && lineOf(one, 2).hidden && lineOf(one, 3).hidden && lineOf(one, 4).hidden);
        QVERIFY(lineOf(one, 4).collapsed && lineOf(one, 5).collapsed);
        const auto two = states(2);   // the lines of level 1 too: the inner group collapsed
        QVERIFY(!lineOf(two, 1).hidden && lineOf(two, 2).hidden && lineOf(two, 3).hidden && !lineOf(two, 4).hidden);
        QVERIFY(lineOf(two, 4).collapsed && !lineOf(two, 5).collapsed);
        const auto three = states(3);  // everything
        for (quint32 i = 1; i <= 4; ++i) QVERIFY(!lineOf(three, i).hidden);
        QVERIFY(!lineOf(three, 4).collapsed && !lineOf(three, 5).collapsed);
    }

    void groupingAndUngroupingShiftTheLevels()
    {
        const Outline o(nested(), 100, true);
        const auto grouped = o.group(6, 7);
        QCOMPARE(int(grouped.size()), 2);
        QCOMPARE(grouped[0].level, 1);
        const auto deeper = o.group(2, 3);
        QCOMPARE(deeper[0].level, 3);
        const auto ungrouped = o.ungroup(1, 4);
        QCOMPARE(ungrouped[0].level, 0);   // 1 to 0
        QCOMPARE(ungrouped[1].level, 1);   // 2 to 1
        // at the deepest: no change
        std::vector<OutlineLine> deep;
        for (quint32 i = 0; i < 3; ++i) deep.push_back({i, 7, false, false});
        QCOMPARE(int(Outline(deep, 10, true).group(0, 2).size()), 0);
        QCOMPARE(int(Outline(deep, 10, true).ungroup(5, 6).size()), 0);
        // ungrouping a collapsed group shows its lines and clears its summary's mark
        std::vector<OutlineLine> collapsed = {{1, 1, true, false}, {2, 1, true, false}, {3, 0, false, true}};
        const auto undone = Outline(collapsed, 10, true).ungroup(1, 2);
        QVERIFY(std::any_of(undone.begin(), undone.end(), [](const OutlineChange &c) { return c.index == 1 && c.level == 0 && c.hidden == 0; }));
        QVERIFY(std::any_of(undone.begin(), undone.end(), [](const OutlineChange &c) { return c.index == 3 && c.collapsed == 0; }));
    }

    // The session makes the changes as one step each, Undo takes them back, and the save writes the levels, the hidden rows, the collapsed mark, and how deep the outline goes.
    void theSessionChangesTheOutlineAndTheSaveWritesIt()
    {
        EditWindow e;
        QVERIFY(e.open(outlineSheet()));
        crunch::EditSession *s = e.win.w.session();
        const Outline o = s->outlineOf(0, false);
        QCOMPARE(o.deepest(), 2);
        const auto groups = o.groups(0, 100);
        QCOMPARE(int(groups.size()), 2);
        QCOMPARE(groups[1].summary, qint64(4));
        QVERIFY(s->changeOutline(0, false, o.collapse(groups[1]), QStringLiteral("Hide Detail")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(rowState(e, 2).hidden && rowState(e, 3).hidden && !rowState(e, 1).hidden);
        QVERIFY(rowState(e, 4).collapsed);
        const Outline now = s->outlineOf(0, false);
        QVERIFY(now.groups(0, 100)[1].collapsed);
        // group two plain rows: the outline gets deeper only if they were in a group; a new group of level 1
        QVERIFY(s->changeOutline(0, false, now.group(6, 7), QStringLiteral("Group")).ok);
        QCOMPARE(rowState(e, 6).level, 1);
        const QByteArray xml = savedSheet(e);
        QVERIFY(!xml.isEmpty());
        QVERIFY2(QRegularExpression(QStringLiteral("<row r=\"3\"[^>]*outlineLevel=\"2\"[^>]*hidden=\"1\"|<row r=\"3\"[^>]*hidden=\"1\"[^>]*outlineLevel=\"2\"")).match(QString::fromUtf8(xml)).hasMatch(), xml.left(1500).constData());
        QVERIFY2(QRegularExpression(QStringLiteral("<row r=\"5\"[^>]*collapsed=\"1\"")).match(QString::fromUtf8(xml)).hasMatch(), xml.left(1500).constData());
        QVERIFY(QRegularExpression(QStringLiteral("<row r=\"7\"[^>]*outlineLevel=\"1\"")).match(QString::fromUtf8(xml)).hasMatch());
        QVERIFY(xml.contains("outlineLevelRow=\"2\""));
        // Undo (twice) takes it all back
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(!rowState(e, 2).hidden && !rowState(e, 4).collapsed && rowState(e, 6).level == 0);
        // columns: a group of two, the sheet told how deep
        QVERIFY(s->changeOutline(0, true, s->outlineOf(0, true).group(1, 2), QStringLiteral("Group")).ok);
        const QByteArray cols = savedSheet(e);
        QVERIFY2(QRegularExpression(QStringLiteral("<col [^>]*min=\"2\"[^>]*max=\"3\"[^>]*outlineLevel=\"1\"|<col [^>]*outlineLevel=\"1\"[^>]*min=\"2\"[^>]*max=\"3\"")).match(QString::fromUtf8(cols)).hasMatch(), cols.left(1500).constData());
        QVERIFY(cols.contains("outlineLevelCol=\"1\""));
        // and ungrouped again: no outline at all in the columns
        QVERIFY(s->changeOutline(0, true, s->outlineOf(0, true).ungroup(1, 2), QStringLiteral("Ungroup")).ok);
        const QByteArray none = savedSheet(e);
        QVERIFY(!none.contains("outlineLevelCol"));
    }

    // Beside the rows there is a bracket for each group and a button on the line that sums it up; a click on the button hides the group's lines (one step), another shows them, and the level
    // buttons show the outline down to a level. The margin above the columns is there only while the columns have an outline.
    void theMarginsDrawTheGroupsAndTheButtonsWork()
    {
        EditWindow e;
        QVERIFY(e.open(outlineSheet()));
        e.win.w.resize(1000, 600);
        OutlineGutter *rows = e.win.w.rowGutter(), *columns = e.win.w.columnGutter();
        QVERIFY(rows && columns);
        QTRY_VERIFY(rows->isVisible());
        QVERIFY(columns->isHidden());
        QCOMPARE(rows->width(), OutlineGutter::thicknessFor(2));
        const auto groups = rows->visibleGroups();
        QCOMPARE(int(groups.size()), 2);
        // the button of the inner group is on its summary line (row 5), in the second column of the margin; the outer group's on row 6, in the first
        const crunch::Outline::Group inner = groups[1], outer = groups[0];
        const QRect button = rows->groupButton(inner);
        QVERIFY(!button.isEmpty());
        const QRect fifth = e.view()->cellRect(4, 0);
        const int center = rows->mapFromGlobal(e.view()->viewport()->mapToGlobal(fifth.center())).y();
        QVERIFY2(std::abs(button.center().y() - center) <= 1, qPrintable(QString::number(button.center().y()) + QStringLiteral(" ") + QString::number(center)));
        QVERIFY(rows->groupButton(outer).center().x() < button.center().x());
        QCOMPARE(rows->hitAt(button.center()).kind, OutlineGutter::Hit::Kind::Group);
        QCOMPARE(rows->hitAt(button.center()).group.level, 2);
        QCOMPARE(rows->hitAt(QPoint(1, rows->height() - 2)).kind, OutlineGutter::Hit::Kind::None);
        // a click hides the lines of the inner group
        QTest::mouseClick(rows, Qt::LeftButton, Qt::NoModifier, button.center());
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(rowState(e, 2).hidden && rowState(e, 3).hidden && !rowState(e, 1).hidden && rowState(e, 4).collapsed);
        QTRY_VERIFY(rows->visibleGroups()[1].collapsed);
        // and another shows them
        QTest::mouseClick(rows, Qt::LeftButton, Qt::NoModifier, rows->groupButton(rows->visibleGroups()[1]).center());
        QVERIFY(!rowState(e, 2).hidden && !rowState(e, 3).hidden && !rowState(e, 4).collapsed);
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        // the buttons of the levels: 1 shows the lines of no level
        QCOMPARE(rows->hitAt(rows->levelButton(1).center()).kind, OutlineGutter::Hit::Kind::Level);
        QTest::mouseClick(rows, Qt::LeftButton, Qt::NoModifier, rows->levelButton(1).center());
        for (quint32 r = 1; r <= 4; ++r) QVERIFY(rowState(e, r).hidden);
        QVERIFY(!rowState(e, 0).hidden && !rowState(e, 5).hidden);
        QTest::mouseClick(rows, Qt::LeftButton, Qt::NoModifier, rows->levelButton(3).center());
        for (quint32 r = 0; r <= 7; ++r) QVERIFY(!rowState(e, r).hidden);
        // the symbols can be turned off
        e.win.w.act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(rows->isHidden());
        e.win.w.act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(rows->isVisible());
    }

    // Group and Ungroup on the Data tab give the rows (columns) selected a level more (less); the margins appear with the first group and go with the last.
    void groupAndUngroupAreCommands()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.resize(1000, 600);
        QVERIFY(e.win.w.act(QStringLiteral("outline.groupRows")) && e.win.w.act(QStringLiteral("outline.ungroupColumns")));
        QVERIFY(e.win.w.rowGutter()->isHidden());
        e.view()->selectRows(1, 2);
        e.win.w.act(QStringLiteral("outline.groupRows"))->trigger();
        QCOMPARE(rowState(e, 1).level, 1);
        QCOMPARE(rowState(e, 2).level, 1);
        QTRY_VERIFY(e.win.w.rowGutter()->isVisible());
        QVERIFY(e.win.w.columnGutter()->isHidden());
        e.win.w.act(QStringLiteral("outline.groupRows"))->trigger();   // a second time: a group inside
        QCOMPARE(rowState(e, 1).level, 2);
        QCOMPARE(e.win.w.rowGutter()->width(), OutlineGutter::thicknessFor(2));
        e.win.w.act(QStringLiteral("outline.ungroupRows"))->trigger();
        e.win.w.act(QStringLiteral("outline.ungroupRows"))->trigger();
        QCOMPARE(rowState(e, 1).level, 0);
        QTRY_VERIFY(e.win.w.rowGutter()->isHidden());
        // columns
        e.view()->selectColumns(1, 3);
        e.win.w.act(QStringLiteral("outline.groupColumns"))->trigger();
        QTRY_VERIFY(e.win.w.columnGutter()->isVisible());
        QCOMPARE(e.win.w.columnGutter()->height(), OutlineGutter::thicknessFor(1));
        // nothing to ungroup: told
        e.view()->selectRows(5, 6);
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("outline.ungroupRows"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!e.win.told.isEmpty());
        // Undo takes the margin away
        for (int i = 0; i < steps; ++i) e.win.w.undoStack()->undo();
        QTRY_VERIFY(e.win.w.columnGutter()->isHidden());
    }

    // Drawing the margin looks at the lines in view of each part of the grid, not at every line between the frozen part and the part that scrolls: with row 1 frozen and a group of 99,999 lines, a view
    // 50,000 lines down is drawn from a few hundred looks, the groups are worked out once for the outline and not for each paint or click, and a change that is not to the lines (a merge) does not
    // make the outline again.
    void theMarginIsDrawnFromTheLinesInView()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.resize(1000, 600);
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->changeOutline(0, false, s->outlineOf(0, false).group(1, 99999), QStringLiteral("Group")).ok);
        OutlineGutter *rows = e.win.w.rowGutter();
        QTRY_VERIFY(rows->isVisible());
        e.win.w.act(QStringLiteral("view.freezeRow"))->trigger();
        e.view()->selectCell(50000, 0);
        QCoreApplication::processEvents();
        const qint64 built = rows->groupsBuilt();
        rows->resetWork();
        QImage image(rows->size(), QImage::Format_ARGB32);
        for (int i = 0; i < 3; ++i) rows->render(&image);
        QVERIFY2(rows->workDone() < 1500, qPrintable(QString::number(rows->workDone())));
        QCOMPARE(rows->groupsBuilt(), built);     // (three paints, no groups made again)
        rows->resetWork();
        for (int y = 0; y < 200; y += 20) rows->hitAt(QPoint(5, y));
        QVERIFY2(rows->workDone() < 1500, qPrintable(QString::number(rows->workDone())));
        QCOMPARE(rows->groupsBuilt(), built);
        // a merge changes how the sheet is laid out and not its lines: the outline is the one it was
        const qint64 outlines = s->outlinesBuilt();
        QVERIFY(s->mergeCells(0, rangeOf(QStringLiteral("F20:G21")), crunch::EditSession::MergeKind::Merge).ok);
        QCOMPARE(s->outlinesBuilt(), outlines);
        // and one that is to them makes it again, once for the rows and once for the columns
        QVERIFY(s->changeOutline(0, false, s->outlineOf(0, false).group(5, 6), QStringLiteral("Group")).ok);
        QCOMPARE(s->outlinesBuilt(), outlines + 1 + 2);
    }

    // A sheet whose summaries are above the rows (and to the left of the columns) has the buttons on the line before a group.
    void aSheetWithItsSummariesAboveIsRead()
    {
        EditWindow e;
        QVERIFY(e.open(outlineSheet(false)));
        const Outline o = e.win.w.session()->outlineOf(0, false);
        const auto groups = o.groups(0, 100);
        QCOMPARE(groups[0].summary, qint64(0));
        QCOMPARE(groups[1].summary, qint64(1));
    }
};

JP_TEST_CLASS(CrunchOutlineTests)
#include "tst_crunch_outline.moc"
