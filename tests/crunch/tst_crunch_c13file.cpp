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

    void spansAreReadAsFarAsTheyMakeSenseAndNeverGrowPastTheirCap()
    {
        using namespace jp::office::sheet;
        QCOMPARE(spansToText(Spans{{0, 0}, {2, 4}, {9, 9}}), QStringLiteral("1 3:5 10"));
        const Spans read = normalizedSpans(spansFromText(QStringLiteral("7 2:4 3 x -3 5:2 9999999999 1:9999999999 5:6 :")));
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
        QVERIFY(e.open(viewsBook(QStringLiteral("<customWorkbookViews><customWorkbookView name=\"Plain\" guid=\"{AAAAAAAA-0000-0000-0000-000000000001}\" autoUpdate=\"1\" mergeInterval=\"5\" "
                                                  "windowWidth=\"16000\" windowHeight=\"9000\" activeSheetId=\"2\"/></customWorkbookViews>"))));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->customViews().size(), 1);
        const CustomView &v = s->customViews()[0];
        QCOMPARE(v.name, QStringLiteral("Plain"));
        QCOMPARE(v.activeSheetId, 2);
        QVERIFY(!v.hasState);
        QCOMPARE(v.windowWidth, 16000);
        QVERIFY(!s->customViewsChanged());
        QVERIFY(s->type(0, 0, 0, QStringLiteral("5")).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/workbook.xml"));
        QVERIFY2(xml.contains("name=\"Plain\"") && xml.contains("autoUpdate=\"1\"") && xml.contains("mergeInterval=\"5\"") && xml.contains("windowWidth=\"16000\"") && xml.contains("activeSheetId=\"2\""), xml.constData());
        QVERIFY2(xml.indexOf("<calcPr") < xml.indexOf("<customWorkbookViews"), xml.constData());
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
            r->views.append(r->capture(QStringLiteral("Review")));
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
            r->show = QStringLiteral("review");   // (names match without regard to case)
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
            r->views.append(r->capture(QStringLiteral("Two & <more>")));
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
        QFile written(e.path);
        QVERIFY(written.open(QIODevice::ReadOnly));
        QVERIFY(again.open(written.readAll()));
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

    void theBoxAddsReplacesDeletesAndShows()
    {
        CustomViewsRequest req;
        int made = 0;
        req.capture = [&made](const QString &name) {
            CustomView v;
            v.name = name;
            v.guid = QStringLiteral("{G%1}").arg(++made);
            return v;
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
        QCOMPARE(box.result().show, QStringLiteral("print"));
    }

private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchC13FileTests)
#include "tst_crunch_c13file.moc"
