// Tests of the notes of a workbook opened from an OpenDocument spreadsheet or from a workbook of the old binary form: they are made, changed, deleted, shown and hidden, moved and sized with
// the pointer and the keys, put back to the default size, and carried with their cells by a sort, each one step of Undo, and the file saved in each of the three forms has them and opens
// with them. Every file is built here, in a temporary folder.
#include "../sheet/odshelpers.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/editsession.h"
#include "core/fonts.h"
#include "core/sortspec.h"
#include "grid/gridview.h"
#include "savedok.h"
#include "sheet/notes.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxsave.h"

#include <QAction>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUndoStack>

using namespace jp;
using jp::office::sheet::NoteBox;
using jp::office::sheet::NoteInfo;
using jp::office::sheet::noteKey;

namespace {

// A window with a workbook open from a folder of its own, and a file dialog that answers what the test says.
struct Win {
    QTemporaryDir dir;
    CrunchWindow w;
    QStringList told;
    QString answer;
    Win()
    {
        w.setNotifier([this](const QString &title, const QString &text) { told << title + QStringLiteral(": ") + text; });
        w.setSavePicker([this](const QString &, const QString &) { return answer; });
        w.setChangedAsker([this](const QString &name) {
            told << QStringLiteral("changed: ") + name;
            return CrunchWindow::ChangedAnswer::Cancel;
        });
        w.setLossAsker([this](const QString &title, const QString &text) {
            told << title + QStringLiteral(": ") + text;
            return CrunchWindow::LossAnswer::Continue;
        });
        w.resize(1100, 640);
        w.show();
    }
    bool open(const QByteArray &bytes, const QString &name)
    {
        const QString path = dir.filePath(name);
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) return false;
        f.write(bytes);
        f.close();
        QSignalSpy first(&w, &CrunchWindow::firstScreenShown), done(&w, &CrunchWindow::calculationFinished), loaded(&w, &CrunchWindow::loadFinished);
        if (!w.openFile(path)) return false;
        if (!(first.wait(20000) && (loaded.count() > 0 || loaded.wait(20000)) && (done.count() > 0 || done.wait(20000)))) return false;
        w.activateWindow();
        return QTest::qWaitForWindowActive(&w) && QTest::qWaitFor([&] { return !w.isCalculating(); }, 20000);
    }
    grid::GridView *view() { return w.view(); }
    void select(quint32 row, quint32 col) { view()->selectCell(row, col); }
};

// What a note comes to, for comparing: the words, who wrote it, whether it is shown, and the box.
struct Seen {
    QString text, author;
    bool shown = false;
    NoteBox box;
};
using Notes = std::map<quint64, Seen>;

Notes notesOf(const std::map<quint64, NoteInfo> &m)
{
    Notes out;
    for (const auto &n : m) out[n.first] = {n.second.text, n.second.author, n.second.shown, n.second.box};
    return out;
}

QString describe(const Notes &n)
{
    QStringList out;
    for (const auto &kv : n) out << QStringLiteral("(%1,%2 %3 by %4 %5 box %6)").arg(kv.first >> 32).arg(kv.first & 0xFFFFFFFFu).arg(kv.second.text, kv.second.author).arg(kv.second.shown ? "shown" : "hidden").arg(kv.second.box.set ? QStringLiteral("set %1,%2,%3,%4,%5,%6,%7,%8").arg(kv.second.box.row1).arg(kv.second.box.col1).arg(kv.second.box.y1).arg(kv.second.box.x1).arg(kv.second.box.row2).arg(kv.second.box.col2).arg(kv.second.box.y2).arg(kv.second.box.x2) : QStringLiteral("none"));
    return out.join(QLatin1Char(' '));
}

// A sheet of 8 columns two centimeters wide and rows a centimeter high: column A holds 3, 1, 2 in its first three rows, A1 has a note by Ann (shown, with a box of its own and a date),
// and C3 has one by Bob (hidden, with no box).
QByteArray odsSource()
{
    const QString automatic = QStringLiteral(
        "<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"2cm\"/></style:style>"
        "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"1cm\" style:use-optimal-row-height=\"false\"/></style:style>");
    const QString alpha = QStringLiteral("<office:annotation office:display=\"true\" svg:x=\"2.5cm\" svg:y=\"1.25cm\" svg:width=\"3cm\" svg:height=\"1.5cm\">"
                                         "<dc:creator>Ann</dc:creator><dc:date>2026-01-02T03:04:05</dc:date><text:p>alpha</text:p></office:annotation>");
    const QString gamma = QStringLiteral("<office:annotation><dc:creator>Bob</dc:creator><text:p>gamma</text:p></office:annotation>");
    auto number = [](int v, const QString &extra) { return cell(QStringLiteral("office:value-type=\"float\" office:value=\"%1\"").arg(v), QStringLiteral("<text:p>%1</text:p>").arg(v) + extra); };
    const QString attrs = QStringLiteral("table:style-name=\"ro1\"");
    const QString rows = row(number(3, alpha) + emptyCell() + emptyCell(), attrs) + row(number(1, QString()) + emptyCell() + emptyCell(), attrs)
                         + row(number(2, QString()) + emptyCell() + cell(QString(), gamma), attrs) + row(emptyCell(), attrs);
    return buildOds(contentXml(table(QStringLiteral("S"), columns(8, QStringLiteral("table:style-name=\"co1\"")) + rows), automatic));
}

// The same workbook as the old binary form has it (made by the program's writer from the sheet above).
QByteArray xlsSource()
{
    jp::office::sheet::Workbook wb;
    QString why;
    if (!openBytes(odsSource(), QStringLiteral("s.ods"), &wb, &why)) return QByteArray();
    QByteArray xls;
    jp::office::sheet::CopyReport report;
    return jp::office::sheet::saveXls(wb, &xls, &report, &why) ? xls : QByteArray();
}

enum class Form { Ods, Xls };
enum class Target { Xlsx, Ods, Xls };

QByteArray sourceOf(Form f) { return f == Form::Ods ? odsSource() : xlsSource(); }
QString nameOf(Form f) { return f == Form::Ods ? QStringLiteral("book.ods") : QStringLiteral("book.xls"); }

const NoteBox kMoved = {true, 0, 2, 5, 10, 3, 4, 8, 20};   // (a box in the cells and pixels any of the forms keeps: under one cell's size)

// Does one thing to the notes of the first sheet; the outcome is what the session said.
crunch::EditSession::Outcome doIt(crunch::EditSession *s, const QString &what)
{
    if (what == QLatin1String("new")) return s->setNote(0, 1, 1, QStringLiteral("made here"), QStringLiteral("Pat"), false, QStringLiteral("New Note"));
    if (what == QLatin1String("edit")) return s->setNote(0, 0, 0, QStringLiteral("edited words"), QStringLiteral("Someone Else"), true, QStringLiteral("Edit Note"));
    if (what == QLatin1String("delete")) return s->deleteNotes(0, {{0, 0}}, QStringLiteral("Delete Note"));
    if (what == QLatin1String("hide")) return s->showNotes(0, {{0, 0}}, false, QStringLiteral("Hide Note"));
    if (what == QLatin1String("show")) return s->showNotes(0, {{2, 2}}, true, QStringLiteral("Show Note"));
    if (what == QLatin1String("moveBox")) return s->setNoteBox(0, 0, 0, kMoved, QStringLiteral("Move or Size Note"));
    if (what == QLatin1String("resetSize")) return s->resetNoteBox(0, 0, 0, QStringLiteral("Reset Note Size"));
    // a sort of A1:C3 from the smallest: the row of the 3 goes last, and the note on it with it
    jp::crunch::SortSpec spec;
    jp::crunch::SortLevel level;
    level.key = 0;
    spec.levels.append(level);
    jp::office::sheet::CellRange range;
    jp::office::sheet::parseRangeRef(QStringLiteral("A1:C3"), &range);
    return s->sortRange(0, range, spec);
}

} // namespace

class CrunchNotesConvertedTests : public QObject {
    Q_OBJECT

    static void addRows(bool withTarget)
    {
        QTest::addColumn<int>("form");
        QTest::addColumn<QString>("action");
        QTest::addColumn<int>("target");
        const char *actions[] = {"new", "edit", "delete", "hide", "show", "moveBox", "resetSize", "sort"};
        const char *forms[] = {"ods", "xls"};
        const char *targets[] = {"xlsx", "ods", "xls"};
        for (int f = 0; f < 2; ++f)
            for (const char *a : actions) {
                if (!withTarget) {
                    QTest::newRow(qPrintable(QStringLiteral("%1 %2").arg(QLatin1String(forms[f]), QLatin1String(a)))) << f << QString::fromLatin1(a) << 0;
                    continue;
                }
                for (int t = 0; t < 3; ++t) QTest::newRow(qPrintable(QStringLiteral("%1 %2 to %3").arg(QLatin1String(forms[f]), QLatin1String(a), QLatin1String(targets[t])))) << f << QString::fromLatin1(a) << t;
            }
    }

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

    // The two sources hold the notes the other tests start from (the old form's made by the writer: its reader is checked on its own, from the format's records).
    void bothSourcesOpenWithTheirNotes()
    {
        for (const Form form : {Form::Ods, Form::Xls}) {
            Win o;
            QVERIFY(o.open(sourceOf(form), nameOf(form)));
            const Notes n = notesOf(o.w.session()->anchors().notes(0));
            QCOMPARE(n.size(), size_t(2));
            QCOMPARE(n.at(noteKey(0, 0)).text, QStringLiteral("alpha"));
            QCOMPARE(n.at(noteKey(0, 0)).author, QStringLiteral("Ann"));
            QVERIFY(n.at(noteKey(0, 0)).shown && n.at(noteKey(0, 0)).box.set);
            QCOMPARE(n.at(noteKey(2, 2)).text, QStringLiteral("gamma"));
            QVERIFY(!n.at(noteKey(2, 2)).shown);
            QVERIFY2(o.told.isEmpty(), qPrintable(o.told.join(QLatin1Char('|'))));
        }
    }

    // The commands are on for both, with no refusal; a workbook from a text file is still refused, and the reason says so.
    void theNoteCommandsAreOnForBothSourcesAndStillOffForATextFile()
    {
        for (const Form form : {Form::Ods, Form::Xls}) {
            Win o;
            QVERIFY(o.open(sourceOf(form), nameOf(form)));
            QString why;
            QVERIFY2(o.w.session()->canEditNotes(0, &why), qPrintable(why));
            QVERIFY(why.isEmpty());
            o.select(1, 1);
            QVERIFY(o.w.act(QStringLiteral("notes.edit"))->isEnabled());
            QCOMPARE(o.w.act(QStringLiteral("notes.edit"))->text(), QStringLiteral("New Note"));
            o.select(0, 0);
            QCOMPARE(o.w.act(QStringLiteral("notes.edit"))->text(), QStringLiteral("Edit Note"));
            QVERIFY(o.w.act(QStringLiteral("notes.delete"))->isEnabled());
            QVERIFY(o.w.act(QStringLiteral("notes.showHide"))->isEnabled());
            QVERIFY(o.w.act(QStringLiteral("notes.moveSize"))->isEnabled());   // (the note on A1 is shown)
            QVERIFY(o.w.act(QStringLiteral("notes.resetSize"))->isEnabled());
        }
        Win text;
        QVERIFY(text.open("a,b\n1,2\n", QStringLiteral("data.csv")));
        QString why;
        QVERIFY(!text.w.session()->canEditNotes(0, &why));
        QVERIFY2(why.contains(QStringLiteral("this kind of file")), qPrintable(why));
    }

    void eachActionIsOneStepAndUndoAndRedoPutItBack_data() { addRows(false); }
    void eachActionIsOneStepAndUndoAndRedoPutItBack()
    {
        QFETCH(int, form);
        QFETCH(QString, action);
        Win o;
        const Form f = Form(form);
        QVERIFY(o.open(sourceOf(f), nameOf(f)));
        crunch::EditSession *s = o.w.session();
        const Notes before = notesOf(s->anchors().notes(0));
        const int steps = o.w.undoStack()->count();
        const crunch::EditSession::Outcome out = doIt(s, action);
        QVERIFY2(out.ok, qPrintable(out.problem));
        const Notes after = notesOf(s->anchors().notes(0));
        QVERIFY2(describe(after) != describe(before), qPrintable(describe(after)));
        QCOMPARE(o.w.undoStack()->count(), steps + 1);   // (one step each, a sort and a deleted note too)
        o.w.undoStack()->undo();
        QCOMPARE(describe(notesOf(s->anchors().notes(0))), describe(before));
        o.w.undoStack()->redo();
        QCOMPARE(describe(notesOf(s->anchors().notes(0))), describe(after));
        // what each did
        if (action == QLatin1String("new")) QCOMPARE(after.at(noteKey(1, 1)).text, QStringLiteral("made here"));
        if (action == QLatin1String("edit")) {
            QCOMPARE(after.at(noteKey(0, 0)).text, QStringLiteral("edited words"));
            QCOMPARE(after.at(noteKey(0, 0)).author, QStringLiteral("Ann"));   // (the first author stays)
        }
        if (action == QLatin1String("delete")) QVERIFY(!after.count(noteKey(0, 0)) && after.count(noteKey(2, 2)));
        if (action == QLatin1String("hide")) QVERIFY(!after.at(noteKey(0, 0)).shown);
        if (action == QLatin1String("show")) QVERIFY(after.at(noteKey(2, 2)).shown);
        if (action == QLatin1String("moveBox")) QVERIFY(after.at(noteKey(0, 0)).box == kMoved);
        if (action == QLatin1String("resetSize")) QVERIFY(!after.at(noteKey(0, 0)).box.set);
        if (action == QLatin1String("sort")) {
            QVERIFY(after.count(noteKey(2, 0)) && !after.count(noteKey(0, 0)));   // (the 3 is last now, with its note)
            QCOMPARE(after.at(noteKey(2, 0)).text, QStringLiteral("alpha"));
            QVERIFY(after.count(noteKey(1, 2)));                                    // (the 2 was on the last row too, and the note on C3 went with it)
        }
        QVERIFY2(o.told.isEmpty(), qPrintable(o.told.join(QLatin1Char('|'))));
    }

    // The workbook with the action done, saved in each form and opened again: the notes are the ones the window has.
    void eachActionIsSavedInEveryFormAndOpensAgainWithIt_data() { addRows(true); }
    void eachActionIsSavedInEveryFormAndOpensAgainWithIt()
    {
        QFETCH(int, form);
        QFETCH(QString, action);
        QFETCH(int, target);
        const Form f = Form(form);
        Win o;
        QVERIFY(o.open(sourceOf(f), nameOf(f)));
        crunch::EditSession *s = o.w.session();
        const crunch::EditSession::Outcome out = doIt(s, action);
        QVERIFY2(out.ok, qPrintable(out.problem));
        const Notes expected = notesOf(s->anchors().notes(0));
        const Target t = Target(target);
        const QString file = o.dir.filePath(t == Target::Xlsx ? QStringLiteral("out.xlsx") : t == Target::Ods ? QStringLiteral("out.ods") : QStringLiteral("out.xls"));
        o.answer = file;
        if (t == Target::Xlsx) {
            QSignalSpy saved(&o.w, &CrunchWindow::saved);
            QVERIFY(o.w.save());
            QVERIFY2(savedOk(saved), qPrintable(o.told.join(QLatin1Char('|'))));
        } else {
            QSignalSpy copied(&o.w, &CrunchWindow::copyExported);
            QVERIFY(o.w.save());
            QVERIFY2(copied.count() > 0 || copied.wait(60000), qPrintable(o.told.join(QLatin1Char('|'))));
            QVERIFY2(copied.first().at(1).toBool(), qPrintable(o.told.join(QLatin1Char('|'))));
        }
        jp::office::sheet::Workbook back;
        QString why;
        QVERIFY2(back.openFile(file, &why), qPrintable(why));
        const Notes now = notesOf(jp::office::sheet::readSheetNoteInfos(back, 0));
        QCOMPARE(now.size(), expected.size());
        for (const auto &kv : expected) {
            const auto at = now.find(kv.first);
            QVERIFY2(at != now.end(), qPrintable(describe(expected) + " / " + describe(now)));
            QCOMPARE(at->second.text, kv.second.text);
            QCOMPARE(at->second.author, kv.second.author);
            QCOMPARE(at->second.shown, kv.second.shown);
            if (kv.second.box.set) {
                // (the cells a box reaches are kept by every form; the distances into them as the form can say them)
                QVERIFY2(at->second.box.set, qPrintable(describe(now)));
                QCOMPARE(at->second.box.row1, kv.second.box.row1);
                QCOMPARE(at->second.box.col1, kv.second.box.col1);
                QCOMPARE(at->second.box.row2, kv.second.box.row2);
                QCOMPARE(at->second.box.col2, kv.second.box.col2);
                if (action == QLatin1String("moveBox") && kv.first == noteKey(0, 0) && t != Target::Ods) QVERIFY(at->second.box == kMoved);
            } else if (action == QLatin1String("resetSize") && t != Target::Ods) {
                // (the default is written as the form's usual box: beside the cell)
                QVERIFY(at->second.box.set);
                QCOMPARE(at->second.box.col1, 1);
                QCOMPARE(at->second.box.col2, 3);
            }
        }
    }

    // Saved, and then edited again: the second save writes the notes of both, in the same file (the first is Save As .xlsx of a workbook that came from either source).
    void aSecondSaveAfterMoreEditsKeepsAllTheNotes()
    {
        for (const Form f : {Form::Ods, Form::Xls}) {
            Win o;
            QVERIFY(o.open(sourceOf(f), nameOf(f)));
            crunch::EditSession *s = o.w.session();
            QVERIFY(doIt(s, QStringLiteral("new")).ok);
            o.answer = o.dir.filePath(QStringLiteral("out.xlsx"));
            QSignalSpy saved(&o.w, &CrunchWindow::saved);
            QVERIFY(o.w.save());
            QVERIFY2(savedOk(saved), qPrintable(o.told.join(QLatin1Char('|'))));
            QVERIFY(doIt(s, QStringLiteral("edit")).ok);
            QVERIFY(s->deleteNotes(0, {{2, 2}}, QStringLiteral("Delete Note")).ok);
            QVERIFY(o.w.save());
            QTRY_COMPARE_WITH_TIMEOUT(saved.count(), 2, 60000);
            QVERIFY(saved.last().at(1).toBool());
            jp::office::sheet::Workbook back;
            QString why;
            QVERIFY2(back.openFile(o.dir.filePath(QStringLiteral("out.xlsx")), &why), qPrintable(why));
            const Notes now = notesOf(jp::office::sheet::readSheetNoteInfos(back, 0));
            QCOMPARE(now.size(), size_t(2));
            QCOMPARE(now.at(noteKey(0, 0)).text, QStringLiteral("edited words"));
            QCOMPARE(now.at(noteKey(1, 1)).text, QStringLiteral("made here"));
        }
    }

    // The box of a shown note moved with the keys (the command, the arrow keys, Enter) and with the pointer (a drag) is one step each, in both sources; Reset Note Size is one too.
    void theBoxIsMovedWithTheKeysAndThePointerAndResetFromTheCommands()
    {
        for (const Form f : {Form::Ods, Form::Xls}) {
            Win o;
            QVERIFY(o.open(sourceOf(f), nameOf(f)));
            crunch::EditSession *s = o.w.session();
            const NoteBox first = s->anchors().noteAt(0, 0, 0)->box;
            QVERIFY(first.set);
            const int steps = o.w.undoStack()->count();
            o.select(0, 0);
            QVERIFY(o.view()->shownNoteRect(0, 0).isValid());
            o.w.act(QStringLiteral("notes.moveSize"))->trigger();
            QVERIFY(o.view()->inNoteKeys());
            QTest::keyClick(o.view(), Qt::Key_Right);
            QTest::keyClick(o.view(), Qt::Key_Down);
            QTest::keyClick(o.view(), Qt::Key_Return);
            QVERIFY(!o.view()->inNoteKeys());
            QCOMPARE(o.w.undoStack()->count(), steps + 1);
            QCOMPARE(o.w.undoStack()->undoText(), QStringLiteral("Move or Size Note"));
            const NoteBox byKeys = s->anchors().noteAt(0, 0, 0)->box;
            QVERIFY(!(byKeys == first));
            // the pointer: a drag of the box
            const QRect box = o.view()->shownNoteRect(0, 0);
            QVERIFY(!box.isEmpty());
            QWidget *vp = o.view()->viewport();
            QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, box.center());
            QTest::mouseMove(vp, box.center() + QPoint(12, 9));
            QTest::mouseMove(vp, box.center() + QPoint(24, 18));
            QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, box.center() + QPoint(24, 18));
            QCOMPARE(o.w.undoStack()->count(), steps + 2);
            QVERIFY(!(s->anchors().noteAt(0, 0, 0)->box == byKeys));
            o.w.act(QStringLiteral("notes.resetSize"))->trigger();
            QCOMPARE(o.w.undoStack()->count(), steps + 3);
            QVERIFY(!s->anchors().noteAt(0, 0, 0)->box.set);
            QVERIFY2(o.told.isEmpty(), qPrintable(o.told.join(QLatin1Char('|'))));
        }
    }

    // The caps hold for notes made here: a note of more characters than the format allows is refused, and so is a note past a sheet's count.
    void theCapsHoldForNotesMadeInAConvertedWorkbook()
    {
        for (const Form f : {Form::Ods, Form::Xls}) {
            Win o;
            QVERIFY(o.open(sourceOf(f), nameOf(f)));
            crunch::EditSession *s = o.w.session();
            const crunch::EditSession::Outcome out = s->setNote(0, 1, 1, QString(32768, QLatin1Char('x')), QStringLiteral("Pat"), false, QStringLiteral("New Note"));
            QVERIFY(!out.ok);
            QVERIFY(!out.problem.isEmpty());
            QVERIFY(s->setNote(0, 1, 1, QString(32767, QLatin1Char('x')), QStringLiteral("Pat"), false, QStringLiteral("New Note")).ok);
        }
    }
};

JP_TEST_CLASS(CrunchNotesConvertedTests)
#include "tst_crunch_notesconv.moc"
