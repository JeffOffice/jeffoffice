// Tests of the File, Review, View, Formulas, and Data items of round 13: Custom Views, and the others as they were added. Every workbook is built here.
#include "app/anchordialogs.h"
#include "app/crunchwindow.h"
#include "app/customviewsdialog.h"
#include "core/editsession.h"
#include "core/errorcheck.h"
#include "sheet/calculation.h"
#include "sheet/customviews.h"
#include "sheet/protection.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "savedok.h"
#include "../localepin.h"

#include <QAction>
#include <QElapsedTimer>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QUndoStack>
#include <QTest>

#include <memory>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::CustomView;
using Spans = QVector<QPair<quint32, quint32>>;

namespace {

// Two sheets of six rows of numbers; the first has the customWorkbookViews `views` after its calculation settings.
QByteArray viewsBook(const QString &views = QString())
{
    QString rows;
    for (int r = 1; r <= 6; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>2</v></c><c r=\"C%1\"><v>3</v></c></row>").arg(r);
    return buildXlsx({worksheet(rows), worksheet(rows)}, QByteArray(), QString(), QStringLiteral("calcId=\"191029\""), QByteArray(), QStringList(), QString(), QByteArray(), QString(), QList<int>(), views);
}

QByteArray savedPart(const QString &path, const QString &part)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QMap<QString, QByteArray> parts;
    jp::readZip(f.readAll(), parts);
    return parts.value(part);
}

// Adds a view of how the workbook looks now to the request, as the box's Add does.
void addNow(CustomViewsRequest *r, const QString &name)
{
    CustomView v;
    QString problem;
    QVERIFY2(r->capture(name, &v, &problem), qPrintable(problem));
    v.name = name;
    r->views.append(v);
}

QVector<quint32> hiddenRows(CrunchWindow &w, int sheet)
{
    QVector<quint32> out;
    for (const jp::office::sheet::RowSpec &r : w.calculation()->rowSpecsIn(sheet, 0, 100))
        if (r.hidden) out.append(r.row);
    return out;
}

} // namespace

class CrunchC13FileTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        m_localeBefore = QLocale();
        m_pin = std::make_unique<PinnedLocale>();
    }
    void cleanup() { m_pin.reset(); }

    // ---- the review of round 13
    void spansStopAtTheirCapWithoutReadingTheRest()
    {
        using namespace jp::office::sheet;
        QString many = QStringLiteral("3 ").repeated(8 << 20);
        QElapsedTimer t;
        t.start();
        QCOMPARE(spansFromText(many, kCustomViewRows, 5).size(), 5);
        QVERIFY2(t.elapsed() < 500, "the whole text was split before the cap was applied");
        // a run reads inside the sheet: one that starts past the last row is dropped, one that ends past it is cut, and what is not a span is skipped
        QCOMPARE(spansFromText(QStringLiteral("1048576 1048577 5:99999999 abc 7:x 8:"), kCustomViewRows), (Spans{{1048575, 1048575}, {4, 1048575}}));
        QCOMPARE(spansFromText(QStringLiteral("16384 16385 2:20000"), kCustomViewColumns), (Spans{{16383, 16383}, {1, 16383}}));
    }

    void aViewIsReadWithinTheSheetAndEachSheetOnce()
    {
        using namespace jp::office::sheet;
        QString sheets;
        for (int i = 0; i < 1000; ++i)
            sheets += QStringLiteral("<jocv:sheet name=\"Sheet1\" zoom=\"100\" frozenRows=\"4000000\" frozenCols=\"5\" row=\"99999999\" col=\"99999\" hiddenRows=\"%1\"/>").arg(QString(QStringLiteral("2 ")).repeated(5000) + QStringLiteral("2000000"));
        EditWindow e;
        QVERIFY(e.open(viewsBook(QStringLiteral("<customWorkbookViews><customWorkbookView name=\"Hostile\" guid=\"{H}\" windowWidth=\"1\" windowHeight=\"1\" activeSheetId=\"1\"><extLst><ext uri=\"%1\">"
                                                  "<jocv:view xmlns:jocv=\"%2\" activeSheet=\"Sheet1\">%3</jocv:view></ext></extLst></customWorkbookView></customWorkbookViews>")
                                     .arg(QLatin1String(kCustomViewExtUri), QLatin1String(kCustomViewNs), sheets))));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->customViews().size(), 1);
        const CustomView v = s->customViews()[0];
        QCOMPARE(v.sheets.size(), 1);   // (the same sheet 1,000 times is one)
        QVERIFY(customViewSpanCount(v) <= kMaxCustomViewSpans);
        QCOMPARE(v.sheets[0].frozenRows, quint32(0));   // (more frozen than a window shows is no frozen pane)
        QCOMPARE(v.sheets[0].frozenCols, quint32(0));
        QVERIFY(v.sheets[0].activeRow < kCustomViewRows && v.sheets[0].activeCol < kCustomViewColumns);
        QVERIFY(v.sheets[0].hiddenRows.last().second < kCustomViewRows);
        // showing it is quick, one step, and leaves the cell inside the sheet
        QElapsedTimer t;
        t.start();
        QVERIFY(e.win.w.showCustomViewAt(0));
        QVERIFY2(t.elapsed() < 20000, "showing a view took too long");
        QVERIFY(e.view()->selection().activeRow < qint64(kCustomViewRows));
    }

    void aViewThatWouldReadBackDifferentIsNotKept()
    {
        QString rows;
        for (int r = 1; r <= 40001; r += 2) rows += QStringLiteral("<row r=\"%1\" hidden=\"1\"><c r=\"A%1\"><v>1</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        CrunchWindow &w = e.win.w;
        QString said;
        bool kept = true;
        w.anchorHooks().customViewsAsker = [&said, &kept](CustomViewsRequest *r) {
            CustomView v;
            kept = r->capture(QStringLiteral("Big"), &v, &said);
            return false;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QVERIFY(!kept);
        QVERIFY2(said.contains(QStringLiteral("can't be kept")), qPrintable(said));
        QVERIFY(w.session()->customViews().isEmpty());
        // the box says so and keeps nothing
        CustomViewsRequest req;
        req.capture = [](const QString &, CustomView *, QString *problem) {
            *problem = QStringLiteral("too much");
            return false;
        };
        CustomViewsDialog box(req);
        box.findChild<QLineEdit *>(QStringLiteral("customViewName"))->setText(QStringLiteral("X"));
        box.findChild<QPushButton *>(QStringLiteral("customViewAdd"))->click();
        QCOMPARE(box.findChild<QListWidget *>(QStringLiteral("customViewList"))->count(), 0);
        QCOMPARE(box.findChild<QLabel *>(QStringLiteral("customViewProblem"))->text(), QStringLiteral("too much"));
    }

    void aViewElementOutsideTheViewListIsNotAView()
    {
        EditWindow e;
        QVERIFY(e.open(viewsBook(QStringLiteral("<fileSharing><customWorkbookView name=\"Stray\" guid=\"{S}\" windowWidth=\"1\" windowHeight=\"1\" activeSheetId=\"1\"/></fileSharing>"))));
        QVERIFY(e.win.w.session()->customViews().isEmpty());
    }

    void errorCheckingForTheCellsFlagsRunsInSlicesAndNotOnEachChange()
    {
        QString rows;
        for (int r = 1; r <= 5; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*%2</f><v>0</v></c></row>").arg(r).arg(r == 3 ? 3 : 2);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        QVERIFY(e.settle());
        // without the neighbor rule the answer has none; the rule runs a few cells at a time and gives the same as the whole
        const crunch::EditSession &s = *e.win.w.session();
        const auto plain = crunch::findFormulaProblems(s, 0, nullptr, false);
        QVERIFY(plain.empty());
        crunch::InconsistentScan scan(s, 0);
        int slices = 0;
        while (!scan.step(1)) ++slices;
        QVERIFY(slices >= 4);
        QCOMPARE(scan.found().size(), size_t(1));
        QCOMPARE(crunch::findFormulaProblems(s, 0).size(), size_t(1));
        // the window shows the corner once the idle slices are through, and a person who asks gets it at once
        QTRY_VERIFY(!e.win.w.formulaProblems(false).empty());
        QCOMPARE(e.win.w.formulaProblems().size(), size_t(1));
    }

    void ignoredErrorsAreLookedUpByAnIndex()
    {
        QString rows, ignored = QStringLiteral("<ignoredErrors><ignoredError sqref=\"A10:A12 A20\" numberStoredAsText=\"1\"/>");
        for (int r = 1; r <= 4000; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>%1</t></is></c></row>").arg(r);
        for (int i = 0; i < 9000; ++i) ignored += QStringLiteral("<ignoredError sqref=\"C%1:D%2\" numberStoredAsText=\"1\"/>").arg(i * 2 + 1).arg(i * 2 + 2);
        ignored += QStringLiteral("</ignoredErrors>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, ignored)})));
        QVERIFY(e.settle());
        QElapsedTimer t;
        t.start();
        const auto problems = e.win.w.formulaProblems(false);
        QVERIFY2(t.elapsed() < 400, qPrintable(QString::number(t.elapsed()) + QStringLiteral(" ms")));
        // (A10:A12 and A20 are ignored, the rest of the first 4,000 are not; the list stops at its cap)
        QCOMPARE(problems.size(), size_t(4000 - 4));
        for (const auto &p : problems) QVERIFY(!(p.col == 0 && (p.row == 9 || p.row == 10 || p.row == 11 || p.row == 19)));
    }

    void spansAreReadAsFarAsTheyMakeSenseAndNeverGrowPastTheirCap()
    {
        using namespace jp::office::sheet;
        QCOMPARE(spansToText(Spans{{0, 0}, {2, 4}, {9, 9}}), QStringLiteral("1 3:5 10"));
        const Spans read = normalizedSpans(spansFromText(QStringLiteral("7 2:4 3 x -3 5:2 9999999999 5:6 :")));
        QCOMPARE(read, (Spans{{1, 6}}));   // 2:4, 3, 5:6, and 7 touch one another
        QCOMPARE(spansWithout(Spans{{0, 9}}, Spans{{2, 3}, {7, 20}}), (Spans{{0, 1}, {4, 6}}));
        QCOMPARE(spansWithout(Spans{{5, 5}}, Spans{{5, 5}}), Spans());
        QCOMPARE(spansWithout(Spans{{0, 4}}, Spans()), (Spans{{0, 4}}));
        QString many;
        for (int i = 0; i < kMaxCustomViewSpans + 500; ++i) many += QString::number(i * 2 + 1) + QLatin1Char(' ');
        QCOMPARE(spansFromText(many).size(), kMaxCustomViewSpans);
    }

    // A view another program made, with no hidden rows in it, is listed with what the standard holds, and saved again as it was when another change is saved.
    void aViewFromAnotherProgramIsListedAndKept()
    {
        EditWindow e;
        QVERIFY(e.open(viewsBook(QStringLiteral("<customWorkbookViews><customWorkbookView xmlns:ex=\"urn:example:other\" ex:uid=\"{U1}\" name=\"Plain\" guid=\"{AAAAAAAA-0000-0000-0000-000000000001}\" autoUpdate=\"1\" mergeInterval=\"5\" "
                                                  "windowWidth=\"16000\" windowHeight=\"9000\" activeSheetId=\"2\"><extLst><ext uri=\"{OTHER-1}\" xmlns:e2=\"urn:example:two\"><e2:thing a=\"1\">text &amp; more</e2:thing></ext></extLst>"
                                                  "</customWorkbookView></customWorkbookViews>"))));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QCOMPARE(s->customViews().size(), 1);
        const CustomView v = s->customViews()[0];
        QCOMPARE(v.name, QStringLiteral("Plain"));
        QCOMPARE(v.activeSheetId, 2);
        QVERIFY(!v.hasState);
        QCOMPARE(v.windowWidth, 16000);
        QVERIFY(v.foreignExt.contains("e2:thing"));
        QVERIFY(!s->customViewsChanged());
        // a change to the list of views makes the writer run: the view keeps all of it, namespaced attributes and the elements of other programs too
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            addNow(r, QStringLiteral("Mine"));
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/workbook.xml"));
        QVERIFY2(xml.contains("name=\"Plain\"") && xml.contains("autoUpdate=\"1\"") && xml.contains("mergeInterval=\"5\"") && xml.contains("windowWidth=\"16000\"") && xml.contains("activeSheetId=\"2\""), xml.constData());
        QVERIFY2(xml.contains("ex:uid=\"{U1}\"") && xml.contains("xmlns:ex=\"urn:example:other\""), xml.constData());
        QVERIFY2(xml.contains("uri=\"{OTHER-1}\"") && xml.contains("<e2:thing a=\"1\">text &amp; more</e2:thing>") && xml.contains("name=\"Mine\""), xml.constData());
        QVERIFY2(xml.indexOf("<calcPr") < xml.indexOf("<customWorkbookViews"), xml.constData());
        // and what was written is read again as it was
        EditWindow again;
        {
            // (closed again at once: Windows can't replace or reuse a file that is still open)
            QFile written(e.path);
            QVERIFY(written.open(QIODevice::ReadOnly));
            QVERIFY(again.open(written.readAll()));
        }
        QVERIFY2(again.win.w.session()->customViews().size() == 2, xml.constData());
        QVERIFY(again.win.w.session()->customViews()[0] == s->customViews()[0]);
    }

    void aViewKeepsTheLookOfTheWorkbookAndShowsItAgain()
    {
        EditWindow e;
        QVERIFY(e.open(viewsBook()));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        // Sheet1: row 2 and column C hidden, zoom 150, the top row frozen; Sheet2: row 4 hidden
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(s->resizeRows(0, 1, 1, hide, QStringLiteral("hide")).ok);
        QVERIFY(s->resizeColumns(0, 2, 2, hide, QStringLiteral("hide")).ok);
        QVERIFY(s->resizeRows(1, 3, 3, hide, QStringLiteral("hide")).ok);
        e.view()->setZoom(1.5);
        e.view()->freezeTopRow();
        e.select(QStringLiteral("B3"));
        const qint64 frozen = e.view()->sheet().layout().frozenRows();
        QVERIFY(frozen >= 1);
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            addNow(r, QStringLiteral("Review"));
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QCOMPARE(s->customViews().size(), 1);
        const CustomView &v = s->customViews()[0];
        QCOMPARE(v.name, QStringLiteral("Review"));
        QVERIFY(v.hasState);
        QCOMPARE(v.sheets.size(), 2);
        QCOMPARE(v.sheets[0].hiddenRows, (Spans{{1, 1}}));
        QCOMPARE(v.sheets[0].hiddenCols, (Spans{{2, 2}}));
        QCOMPARE(v.sheets[0].zoom, 150);
        QCOMPARE(qint64(v.sheets[0].frozenRows), frozen);
        QCOMPARE(v.sheets[0].activeRow, quint32(2));
        QCOMPARE(v.sheets[1].hiddenRows, (Spans{{3, 3}}));
        QVERIFY(w.isWindowModified());
        // everything changes: nothing hidden, zoom 100, no panes, another cell, and Sheet2 in front
        const int undoDepth = s->undoStack()->index();
        QVERIFY(s->resizeRows(0, 0, 10, [] { crunch::EditSession::SizeChange c; c.setHidden = true; return c; }(), QStringLiteral("show")).ok);
        QVERIFY(s->resizeColumns(0, 0, 10, [] { crunch::EditSession::SizeChange c; c.setHidden = true; return c; }(), QStringLiteral("show")).ok);
        QVERIFY(s->resizeRows(1, 0, 10, [] { crunch::EditSession::SizeChange c; c.setHidden = true; return c; }(), QStringLiteral("show")).ok);
        QVERIFY(hiddenRows(w, 1).isEmpty());
        e.view()->setZoom(1.0);
        e.view()->unfreeze();
        e.select(QStringLiteral("A1"));
        w.setActiveSheet(1);
        QCOMPARE(w.activeSheet(), 1);
        QVERIFY(hiddenRows(w, 0).isEmpty());
        // Show brings it all back in one step of Undo
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            r->show = 0;
            return true;
        };
        const int before = s->undoStack()->index();
        w.act(QStringLiteral("view.customViews"))->trigger();
        QCOMPARE(s->undoStack()->index(), before + 1);
        QCOMPARE(w.activeSheet(), 0);
        QCOMPARE(hiddenRows(w, 0), (QVector<quint32>{1}));
        QCOMPARE(hiddenRows(w, 1), (QVector<quint32>{3}));
        QCOMPARE(w.view()->zoom(), 1.5);
        QCOMPARE(w.view()->sheet().layout().frozenRows(), frozen);
        QCOMPARE(e.selected(), QStringLiteral("B3"));
        s->undoStack()->undo();
        QVERIFY(hiddenRows(w, 0).isEmpty());
        QVERIFY(hiddenRows(w, 1).isEmpty());
        Q_UNUSED(undoDepth);
    }

    void customViewsAreSavedReadAgainAndDeleted()
    {
        EditWindow e;
        QVERIFY(e.open(viewsBook()));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(s->resizeRows(0, 1, 2, hide, QStringLiteral("hide")).ok);
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            addNow(r, QStringLiteral("Two & <more>"));
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/workbook.xml"));
        QVERIFY2(xml.contains("<customWorkbookViews><customWorkbookView name=\"Two &amp; &lt;more&gt;\" guid=\"{") && xml.contains("hiddenRows=\"2:3\"") && xml.contains("<extLst><ext uri=\"{"), xml.constData());
        QVERIFY2(xml.contains("activeSheetId=\"1\"") && xml.contains("windowWidth=\""), xml.constData());
        // read again by a new window: the same view
        const QVector<CustomView> was = s->customViews();
        EditWindow again;
        {
            // (closed again at once: Windows can't replace or reuse a file that is still open)
            QFile written(e.path);
            QVERIFY(written.open(QIODevice::ReadOnly));
            QVERIFY(again.open(written.readAll()));
        }
        QCOMPARE(again.win.w.session()->customViews().size(), 1);
        QVERIFY(again.win.w.session()->customViews()[0] == was[0]);
        QVERIFY(!again.win.w.session()->customViewsChanged());
        // deleted, then saved: no element is left
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            r->views.clear();
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QVERIFY(s->customViews().isEmpty());
        QVERIFY(w.isWindowModified());
        QSignalSpy saved2(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved2, 20000), "the save failed");
        QVERIFY(!savedPart(e.path, QStringLiteral("xl/workbook.xml")).contains("customWorkbookView"));
    }

    // Subtotal can put a page break after each group's total, and Remove All takes those breaks away with the rows (a break of the person's own stays).
    void subtotalPutsAPageBreakBetweenGroups()
    {
        auto text = [](const QString &ref, const QString &t) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t); };
        const QString rows = QStringLiteral("<row r=\"1\">") + text(QStringLiteral("A1"), QStringLiteral("Region")) + text(QStringLiteral("B1"), QStringLiteral("Sales")) + QStringLiteral("</row>")
            + QStringLiteral("<row r=\"2\">") + text(QStringLiteral("A2"), QStringLiteral("East")) + QStringLiteral("<c r=\"B2\"><v>10</v></c></row>")
            + QStringLiteral("<row r=\"3\">") + text(QStringLiteral("A3"), QStringLiteral("East")) + QStringLiteral("<c r=\"B3\"><v>20</v></c></row>")
            + QStringLiteral("<row r=\"4\">") + text(QStringLiteral("A4"), QStringLiteral("West")) + QStringLiteral("<c r=\"B4\"><v>5</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        auto breaks = [s] {
            QVector<quint32> out;
            for (const auto &b : s->pages().sheet(0).rowBreaks) out.append(b.before);
            return out;
        };
        QVERIFY(breaks().isEmpty());
        e.select(QStringLiteral("A2"));
        QVERIFY(w.subtotalIn(0, 9, {1}, true, true));
        QVERIFY(e.settle());
        QCOMPARE(breaks(), (QVector<quint32>{4, 6}));   // before West, and before the grand total
        e.select(QStringLiteral("A2"));
        QVERIFY(w.removeSubtotalsIn());
        QVERIFY(e.settle());
        QVERIFY2(breaks().isEmpty(), "the breaks stayed after the rows went");
        // without the choice, no break is added
        e.select(QStringLiteral("A2"));
        QVERIFY(w.subtotalIn(0, 9, {1}));
        QVERIFY(e.settle());
        QVERIFY(breaks().isEmpty());
    }

    // A scenario set to prevent changes is kept as it is while the sheet is protected; once the sheet is unprotected it can be changed again.
    void aLockedScenarioIsKeptWhileTheSheetIsProtected()
    {
        EditWindow e;
        QVERIFY(e.open(viewsBook()));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(w.addScenario(QStringLiteral("Fixed"), QStringLiteral("A1"), QString(), {QStringLiteral("7")}, true));
        QVERIFY(w.addScenario(QStringLiteral("Free"), QStringLiteral("A1"), QString(), {QStringLiteral("8")}));
        QVERIFY(s->scenariosOf(0).list[0].locked);
        QVERIFY(!s->scenariosOf(0).list[1].locked);
        jp::office::sheet::SheetProtection on;
        on.enabled = true;
        QVERIFY(s->setSheetProtection(0, on, QStringLiteral("Protect")).ok);
        QVERIFY(!w.deleteScenario(0));
        QVERIFY(!w.editScenario(0, QStringLiteral("Fixed"), QStringLiteral("changed"), {QStringLiteral("9")}));
        QCOMPARE(s->scenariosOf(0).list.size(), 2);
        QCOMPARE(s->scenariosOf(0).list[0].cells[0].value, QStringLiteral("7"));
        QVERIFY(e.win.told.last().contains(QStringLiteral("prevent changes")));
        QVERIFY(w.deleteScenario(1));   // (the one that is free can go)
        QVERIFY(s->setSheetProtection(0, jp::office::sheet::SheetProtection(), QStringLiteral("Unprotect")).ok);
        QVERIFY(w.editScenario(0, QStringLiteral("Fixed"), QStringLiteral("changed"), {QStringLiteral("9")}));
        QVERIFY(w.deleteScenario(0));
        QVERIFY(s->scenariosOf(0).list.isEmpty());
    }

    // Error Checking also finds a formula that is not like the two on either side of it (which are alike), and a date kept as text with a year of two digits; Ignore Error keeps each.
    void errorCheckingFindsAnInconsistentFormulaAndATwoDigitYear()
    {
        QString rows;
        for (int r = 1; r <= 5; ++r) {
            const QString factor = r == 3 ? QStringLiteral("3") : QStringLiteral("2");
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*%2</f><v>0</v></c>").arg(r).arg(factor);
            rows += r == 1 ? QStringLiteral("<c r=\"C1\" t=\"inlineStr\"><is><t>3/4/24</t></is></c></row>") : r == 2 ? QStringLiteral("<c r=\"C2\" t=\"inlineStr\"><is><t>3/4/2024</t></is></c></row>") : QStringLiteral("</row>");
        }
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const auto problems = w.formulaProblems();
        QCOMPARE(problems.size(), size_t(2));
        QVERIFY(problems[0].kind == crunch::FormulaProblem::Kind::TwoDigitYear);
        QCOMPARE(problems[0].row, quint32(0));
        QCOMPARE(problems[0].col, quint32(2));
        QVERIFY(problems[1].kind == crunch::FormulaProblem::Kind::InconsistentFormula);
        QCOMPARE(problems[1].row, quint32(2));
        QCOMPARE(problems[1].col, quint32(1));
        // the same formula as its neighbors, written with a fixed row, is another pattern; and an absolute reference is compared as it is
        QVERIFY(w.session()->type(0, 2, 1, QStringLiteral("=A3*2"), false).ok);
        QVERIFY(e.settle());
        QCOMPARE(w.formulaProblems().size(), size_t(1));
        QVERIFY(w.session()->type(0, 2, 1, QStringLiteral("=$A$1*2"), false).ok);
        QVERIFY(e.settle());
        QCOMPARE(w.formulaProblems().size(), size_t(2));
        w.ignoreFormulaProblem(2, 1);
        w.ignoreFormulaProblem(0, 2);
        QVERIFY(w.formulaProblems().empty());
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray sheet = savedPart(e.path, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(sheet.contains("formula=\"1\"") && sheet.contains("twoDigitTextYear=\"1\""), sheet.constData());
    }

    // A formula in a cell that is not locked is found (it can be changed on a protected sheet), and Ignore Error keeps that too.
    void errorCheckingFindsAFormulaInAnUnlockedCell()
    {
        const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*2</f><v>2</v></c><c r=\"C1\" s=\"1\"><f>A1*3</f><v>3</v></c><c r=\"D1\" s=\"1\"><v>4</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles)));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        auto problems = w.formulaProblems();
        QCOMPARE(problems.size(), size_t(1));   // (the unlocked cell with no formula is no problem)
        QVERIFY(problems[0].kind == crunch::FormulaProblem::Kind::UnlockedFormula);
        QCOMPARE(problems[0].col, quint32(2));
        w.ignoreFormulaProblem(0, 2);
        QVERIFY(w.formulaProblems().empty());
        QCOMPARE(int(w.session()->ignoredErrorsOf(0).size()), 1);
        QVERIFY(w.session()->ignoredErrorsOf(0)[0].kinds & jp::office::sheet::IgnoredError::UnlockedFormula);
    }

    void aSheetRenamedOrDeletedIsFollowedInTheViews()
    {
        EditWindow e;
        QVERIFY(e.open(viewsBook()));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            addNow(r, QStringLiteral("All"));
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QCOMPARE(s->customViews()[0].sheets.size(), 2);
        QVERIFY(s->renameSheet(1, QStringLiteral("Totals")).ok);
        QCOMPARE(s->customViews()[0].sheets[1].name, QStringLiteral("Totals"));
        w.undoStack()->undo();
        QCOMPARE(s->customViews()[0].sheets[1].name, QStringLiteral("Sheet2"));
        w.undoStack()->redo();
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/workbook.xml"));
        QVERIFY2(xml.contains("<jocv:sheet name=\"Totals\"") && !xml.contains("<jocv:sheet name=\"Sheet2\""), xml.constData());
        // deleted, the sheet is no longer in any view; Undo brings it back
        QVERIFY(s->deleteSheet(1).ok);
        QCOMPARE(s->customViews()[0].sheets.size(), 1);
        QCOMPARE(s->customViews()[0].sheets[0].name, QStringLiteral("Sheet1"));
        w.undoStack()->undo();
        QCOMPARE(s->customViews()[0].sheets.size(), 2);
    }

    void showingAViewIsUndoneWholeAndRedoneWhole()
    {
        EditWindow e;
        QVERIFY(e.open(viewsBook()));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        w.resize(1100, 640);
        e.view()->setZoom(1.5);
        e.view()->freezeTopRow();
        e.select(QStringLiteral("B3"));
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(s->resizeRows(0, 1, 1, hide, QStringLiteral("hide")).ok);
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            addNow(r, QStringLiteral("Look"));
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        // now everything is otherwise
        QVERIFY(s->resizeRows(0, 0, 10, [] { crunch::EditSession::SizeChange c; c.setHidden = true; return c; }(), QStringLiteral("show")).ok);
        w.setActiveSheet(1);
        e.view()->setZoom(0.75);
        e.view()->setShowGridlines(false);
        e.view()->setShowHeadings(false);
        w.act(QStringLiteral("view.formulaBar"))->setChecked(false);
        w.resize(900, 600);
        e.select(QStringLiteral("C5"));
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            r->show = 0;
            return true;
        };
        const int steps = s->undoStack()->count();
        w.act(QStringLiteral("view.customViews"))->trigger();
        QCOMPARE(s->undoStack()->count(), steps + 1);
        auto isShown = [&] {
            return w.activeSheet() == 0 && w.view()->zoom() == 1.5 && w.view()->sheet().layout().frozenRows() >= 1 && e.selected() == QStringLiteral("B3") && w.view()->showGridlines() && w.view()->showHeadings()
                && w.act(QStringLiteral("view.formulaBar"))->isChecked() && w.width() == 1100 && hiddenRows(w, 0) == QVector<quint32>{1};
        };
        QVERIFY(isShown());
        s->undoStack()->undo();
        QCOMPARE(w.activeSheet(), 1);
        QCOMPARE(w.view()->zoom(), 0.75);
        QVERIFY(!w.view()->showGridlines() && !w.view()->showHeadings());
        QVERIFY(!w.act(QStringLiteral("view.formulaBar"))->isChecked());
        QCOMPARE(w.width(), 900);
        QCOMPARE(e.selected(), QStringLiteral("C5"));
        QVERIFY(hiddenRows(w, 0).isEmpty());
        s->undoStack()->redo();
        QVERIFY(isShown());
    }

    void rowsAFilterHidesAreLeftToTheFilter()
    {
        QString rows;
        for (int r = 1; r <= 6; ++r) rows += QStringLiteral("<row r=\"%1\"%2><c r=\"A%1\"><v>%1</v></c></row>").arg(r).arg(r == 1 || r == 3 ? QString() : QStringLiteral(" hidden=\"1\""));
        const QString filter = QStringLiteral("<autoFilter ref=\"A1:A6\"><filterColumn colId=\"0\"><filters><filter val=\"3\"/></filters></filterColumn></autoFilter>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, filter)})));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QCOMPARE(hiddenRows(w, 0), (QVector<quint32>{1, 3, 4, 5}));
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            addNow(r, QStringLiteral("Now"));
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QVERIFY2(s->customViews()[0].sheets[0].hiddenRows.isEmpty(), "the rows the filter hides are not the view's");
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            r->show = 0;
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QCOMPARE(hiddenRows(w, 0), (QVector<quint32>{1, 3, 4, 5}));   // (the filter's rows are still hidden)
    }

    void twoViewsWithOneNameAreToldApartByTheirPlace()
    {
        const QString two = QStringLiteral("<customWorkbookViews>"
            "<customWorkbookView name=\"Same\" guid=\"{A}\" windowWidth=\"9000\" windowHeight=\"9000\" activeSheetId=\"1\"/>"
            "<customWorkbookView name=\"Same\" guid=\"{B}\" windowWidth=\"9000\" windowHeight=\"9000\" activeSheetId=\"2\"><extLst><ext uri=\"%1\"><jocv:view xmlns:jocv=\"%2\" activeSheet=\"Sheet2\">"
            "<jocv:sheet name=\"Sheet2\" zoom=\"200\"/></jocv:view></ext></extLst></customWorkbookView></customWorkbookViews>");
        EditWindow e;
        QVERIFY(e.open(viewsBook(two.arg(QLatin1String(jp::office::sheet::kCustomViewExtUri), QLatin1String(jp::office::sheet::kCustomViewNs)))));
        CrunchWindow &w = e.win.w;
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            r->show = 1;   // (the second "Same")
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QCOMPARE(w.activeSheet(), 1);
        QCOMPARE(w.view()->zoom(), 2.0);
    }

    void aViewNameWithControlCharactersIsWrittenAsValidXml()
    {
        EditWindow e;
        QVERIFY(e.open(viewsBook()));
        CrunchWindow &w = e.win.w;
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            addNow(r, QStringLiteral("A\x01" "B\x1f\tC\n"));
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/workbook.xml"));
        for (const char bad : {'\x01', '\x1f'}) QVERIFY(!xml.contains(bad));
        QVERIFY(xml.contains("name=\"A B  C \""));
        EditWindow again;
        {
            // (closed again at once: Windows can't replace or reuse a file that is still open)
            QFile written(e.path);
            QVERIFY(written.open(QIODevice::ReadOnly));
            QVERIFY(again.open(written.readAll()));
        }
        QCOMPARE(again.win.w.session()->customViews().size(), 1);
    }

    // The entries of the sheets that name a view that was deleted go with it.
    void deletingAViewTakesItsSheetEntriesOut()
    {
        const QString sheetViews = QStringLiteral("<customSheetViews><customSheetView guid=\"{DEAD-1}\" scale=\"90\"><selection activeCell=\"A1\"/></customSheetView><customSheetView guid=\"{KEEP-2}\"/></customSheetViews>");
        const QString two = QStringLiteral("<customWorkbookViews><customWorkbookView name=\"Gone\" guid=\"{DEAD-1}\" windowWidth=\"9000\" windowHeight=\"9000\" activeSheetId=\"1\"/>"
                                           "<customWorkbookView name=\"Stays\" guid=\"{KEEP-2}\" windowWidth=\"9000\" windowHeight=\"9000\" activeSheetId=\"1\"/></customWorkbookViews>");
        QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, sheetViews)}, QByteArray(), QString(), QStringLiteral("calcId=\"1\""), QByteArray(), QStringList(), QString(), QByteArray(), QString(), QList<int>(), two)));
        CrunchWindow &w = e.win.w;
        w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            r->views.removeAt(0);
            return true;
        };
        w.act(QStringLiteral("view.customViews"))->trigger();
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray sheet = savedPart(e.path, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(!sheet.contains("{DEAD-1}") && sheet.contains("{KEEP-2}") && sheet.contains("<customSheetViews>"), sheet.constData());
        // with the last one gone too, the list goes
        EditWindow again;
        {
            // (closed again at once: Windows can't replace or reuse a file that is still open)
            QFile written(e.path);
            QVERIFY(written.open(QIODevice::ReadOnly));
            QVERIFY(again.open(written.readAll()));
        }
        again.win.w.anchorHooks().customViewsAsker = [](CustomViewsRequest *r) {
            r->views.clear();
            return true;
        };
        again.win.w.act(QStringLiteral("view.customViews"))->trigger();
        QSignalSpy saved2(&again.win.w, &CrunchWindow::saved);
        QVERIFY(again.win.w.save());
        QVERIFY2(savedOk(saved2, 20000), "the save failed");
        QVERIFY(!savedPart(again.path, QStringLiteral("xl/worksheets/sheet1.xml")).contains("customSheetView"));
    }

    // Remove All takes away the breaks Subtotal put in, and never a break the person made, whether it is by the subtotals or not.
    void subtotalRemoveAllKeepsThePersonsOwnBreaks()
    {
        auto text = [](const QString &ref, const QString &t) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t); };
        const QString rows = QStringLiteral("<row r=\"1\">") + text(QStringLiteral("A1"), QStringLiteral("Region")) + text(QStringLiteral("B1"), QStringLiteral("Sales")) + QStringLiteral("</row>")
            + QStringLiteral("<row r=\"2\">") + text(QStringLiteral("A2"), QStringLiteral("East")) + QStringLiteral("<c r=\"B2\"><v>10</v></c></row>")
            + QStringLiteral("<row r=\"3\">") + text(QStringLiteral("A3"), QStringLiteral("East")) + QStringLiteral("<c r=\"B3\"><v>20</v></c></row>")
            + QStringLiteral("<row r=\"4\">") + text(QStringLiteral("A4"), QStringLiteral("West")) + QStringLiteral("<c r=\"B4\"><v>5</v></c></row>");
        // the person's own break is before row 5 (below the list, where the first subtotal row will be: after Subtotal it is at the same place)
        const QString page = QStringLiteral("<rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"4\" max=\"16383\" man=\"1\"/></rowBreaks>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, page)})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        auto breaks = [s] {
            QVector<quint32> out;
            for (const auto &b : s->pages().sheet(0).rowBreaks) out.append(b.before);
            return out;
        };
        QCOMPARE(breaks(), (QVector<quint32>{4}));
        e.select(QStringLiteral("A2"));
        QVERIFY(w.subtotalIn(0, 9, {1}, true, true));
        QVERIFY(e.settle());
        // (the person's break before West moved down one row with the first subtotal row inserted above it? no: it follows its row, and ours are before West's subtotal's next row and the grand total)
        QVERIFY(breaks().size() >= 2);
        e.select(QStringLiteral("A2"));
        QVERIFY(w.removeSubtotalsIn());
        QVERIFY(e.settle());
        QCOMPARE(breaks(), (QVector<quint32>{4}));   // the person's own break is where it was
    }

    void theBoxAddsReplacesDeletesAndShows()
    {
        CustomViewsRequest req;
        int made = 0;
        req.capture = [&made](const QString &name, CustomView *v, QString *) {
            v->name = name;
            v->guid = QStringLiteral("{G%1}").arg(++made);
            return true;
        };
        CustomViewsDialog box(req);
        auto *list = box.findChild<QListWidget *>(QStringLiteral("customViewList"));
        auto *name = box.findChild<QLineEdit *>(QStringLiteral("customViewName"));
        auto *add = box.findChild<QPushButton *>(QStringLiteral("customViewAdd"));
        auto *del = box.findChild<QPushButton *>(QStringLiteral("customViewDelete"));
        auto *show = box.findChild<QPushButton *>(QStringLiteral("customViewShow"));
        QVERIFY(list && name && add && del && show);
        QVERIFY(!add->isEnabled() && !del->isEnabled() && !show->isEnabled());
        name->setText(QStringLiteral("Print"));
        QVERIFY(add->isEnabled());
        add->click();
        name->setText(QStringLiteral("Edit"));
        add->click();
        QCOMPARE(list->count(), 2);
        QVERIFY(name->text().isEmpty());
        // the same name (in other capitals) replaces the view and keeps its identity
        name->setText(QStringLiteral("print"));
        add->click();
        QCOMPARE(list->count(), 2);
        const CustomViewsRequest r = box.result();
        QCOMPARE(r.views[0].name, QStringLiteral("print"));
        QCOMPARE(r.views[0].guid, QStringLiteral("{G1}"));
        list->setCurrentRow(1);
        del->click();
        QCOMPARE(list->count(), 1);
        QCOMPARE(box.result().views.size(), 1);
        list->setCurrentRow(0);
        show->click();
        QCOMPARE(box.result().show, 0);
    }

private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchC13FileTests)
#include "tst_crunch_c13file.moc"
