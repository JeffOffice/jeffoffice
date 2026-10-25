// Tests of the defined names of a workbook as a person changes them through the edit session (one step of Undo each, the rules a name keeps, the formulas that use a name following it when it
// is renamed, and what a save leaves of the file's own list of names), and of how the workbook calculates. The workbook is built here.
#include "app/crunchwindow.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "savedok.h"

#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

QString cell(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }
QString formula(const QString &ref, const QString &text) { return QStringLiteral("<c r=\"%1\"><f>%2</f><v>0</v></c>").arg(ref, text.toHtmlEscaped()); }
QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

// The names are written as they are on purpose: the spaces are the file's, and a save must leave the names it does not change as they are.
const QString kKeep = QStringLiteral("<definedName name=\"Keep\"   hidden=\"1\">Sheet1!$C$2</definedName>");
const QString kNames = QStringLiteral("<definedName name=\"Rate\" comment=\"the rate\">Sheet1!$C$1</definedName>") + kKeep + QStringLiteral("<definedName name=\"Local\" localSheetId=\"1\">Sheet2!$A$1</definedName>");

//   Sheet1: A1 = Rate*2, B1 = SUM(Keep,1), C1 = 5, C2 = 7
//   Sheet2: A1 = 100, B1 = Local+1
QByteArray namesBook(const QString &calcPr = QString())
{
    const QByteArray one = worksheet(row(1, formula(QStringLiteral("A1"), QStringLiteral("Rate*2")) + formula(QStringLiteral("B1"), QStringLiteral("SUM(Keep,1)")) + cell(QStringLiteral("C1"), QStringLiteral("5"))) + row(2, cell(QStringLiteral("C2"), QStringLiteral("7"))));
    const QByteArray two = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("100")) + formula(QStringLiteral("B1"), QStringLiteral("Local+1"))));
    return buildXlsx({one, two}, QByteArray(), QString(), calcPr, QByteArray(), QStringList(), kNames);
}

struct Saved {
    Package package;
    Workbook book;
    bool open(const QString &path)
    {
        QString why;
        return package.readLazyFile(path, &why) && book.open(package, &why);
    }
    QString formula(int sheet, const QString &ref) const
    {
        Sheet s;
        if (!book.readSheet(sheet, &s)) return QStringLiteral("unreadable");
        const Cell *c = at(s, ref);
        return c ? s.formulaText(*c) : QStringLiteral("-");
    }
    // "name|scope|text|comment|hidden" of each name the file has (the program's own, such as print areas, left out), in the file's order
    QStringList names() const
    {
        QStringList out;
        for (const DefinedName &n : book.info().definedNames) out << QStringLiteral("%1|%2|%3|%4|%5").arg(n.name).arg(n.localSheet).arg(n.ref, n.comment).arg(n.hidden ? 1 : 0);
        return out;
    }
    QByteArray workbookXml() const { return package.part(QStringLiteral("/xl/workbook.xml")); }
};

QString textOf(EditWindow &e, int sheet, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.win.w.session()->editText(sheet, r, c);
}

QString valueOf(EditWindow &e, int sheet, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.win.w.session()->valueAt(sheet, r, c).debug();
}

bool saveNow(EditWindow &e)
{
    if (!e.settle()) return false;
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    return e.win.w.save() && savedOk(saved);
}

crunch::EditSession::NameDraft draft(const QString &name, const QString &text, int scope = -1, const QString &comment = QString())
{
    crunch::EditSession::NameDraft d;
    d.name = name;
    d.text = text;
    d.scope = scope;
    d.comment = comment;
    return d;
}

} // namespace

class CrunchNamesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    void theNamesOfAFileAreListedWithWhatTheyStandForAndTheirValues()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        const crunch::NameTable &t = s->names();
        QCOMPARE(t.aliveCount(), 3);
        const int rate = t.find(-1, QStringLiteral("RATE"));
        QVERIFY(rate >= 0);
        QCOMPARE(t.at(rate).name, QStringLiteral("Rate"));
        QCOMPARE(t.at(rate).comment, QStringLiteral("the rate"));
        QCOMPARE(s->nameTextOf(rate), QStringLiteral("=Sheet1!$C$1"));
        QCOMPARE(s->nameValueOf(rate), QStringLiteral("5"));
        QVERIFY(t.at(t.find(-1, QStringLiteral("Keep"))).hidden);
        const int local = t.find(1, QStringLiteral("local"));
        QVERIFY(local >= 0);
        QCOMPARE(t.at(local).scope, 1);
        QCOMPARE(t.find(0, QStringLiteral("Local")), -1);   // (it belongs to the second sheet)
        QCOMPARE(s->nameValueOf(local), QStringLiteral("100"));
    }

    void aNameIsMadeChangedAndTakenAwayEachInOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        int made = -1;
        QVERIFY(s->defineName(draft(QStringLiteral("Tax"), QStringLiteral("=Sheet1!$C$2*2")), &made).ok);
        QCOMPARE(u->count(), before + 1);
        QCOMPARE(u->undoText(), QStringLiteral("Define Name"));
        QVERIFY(s->nameListed(made));
        QCOMPARE(s->nameValueOf(made), QStringLiteral("14"));
        QVERIFY(s->type(0, 3, 0, QStringLiteral("=Tax+1")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 0, QStringLiteral("A4")), QStringLiteral("n:15"));
        u->undo();   // the entry
        u->undo();   // the name
        QVERIFY(e.settle());
        QVERIFY(!s->nameListed(made));
        QCOMPARE(s->names().find(-1, QStringLiteral("Tax")), -1);
        u->redo();
        u->redo();
        QVERIFY(e.settle());
        QVERIFY(s->nameListed(made));
        QCOMPARE(valueOf(e, 0, QStringLiteral("A4")), QStringLiteral("n:15"));
        // changed: what it stands for, and the comment
        QVERIFY(s->changeName(made, draft(QStringLiteral("Tax"), QStringLiteral("Sheet1!$C$1*10"), -1, QStringLiteral("a tenth"))).ok);
        QCOMPARE(u->undoText(), QStringLiteral("Change Name"));
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 0, QStringLiteral("A4")), QStringLiteral("n:51"));
        QCOMPARE(s->names().at(made).comment, QStringLiteral("a tenth"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 0, QStringLiteral("A4")), QStringLiteral("n:15"));
        QCOMPARE(s->names().at(made).comment, QString());
        // taken away: what uses it has #NAME?, and Undo brings it back
        QVERIFY(s->removeName(made).ok);
        QCOMPARE(u->undoText(), QStringLiteral("Delete Name"));
        QVERIFY(e.settle());
        QVERIFY(!s->nameListed(made));
        QCOMPARE(valueOf(e, 0, QStringLiteral("A4")), QStringLiteral("e:#NAME?"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 0, QStringLiteral("A4")), QStringLiteral("n:15"));
        QVERIFY(s->nameListed(made));
    }

    void aNameThatBreaksTheRulesIsRefusedWithAReason()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        const int count = e.win.w.undoStack()->count();
        // already there in that scope (any case), but a sheet may have a name the workbook has
        auto same = s->defineName(draft(QStringLiteral("rate"), QStringLiteral("Sheet1!$C$2")));
        QVERIFY(!same.ok);
        QVERIFY(same.problem.contains(QStringLiteral("already")));
        QVERIFY(s->defineName(draft(QStringLiteral("Rate"), QStringLiteral("Sheet2!$A$1"), 1)).ok);
        QVERIFY(!s->defineName(draft(QStringLiteral("Rate"), QStringLiteral("Sheet2!$A$2"), 1)).ok);
        // the shape of a name
        for (const char *bad : {"", "A1", "two words", "1st", "R1C1", "a-b"}) {
            const auto out = s->defineName(draft(QString::fromLatin1(bad), QStringLiteral("Sheet1!$C$2")));
            QVERIFY2(!out.ok && !out.problem.isEmpty(), bad);
        }
        // what it stands for must be something: and a formula
        QVERIFY(!s->defineName(draft(QStringLiteral("Empty"), QString())).ok);
        const auto broken = s->defineName(draft(QStringLiteral("Broken"), QStringLiteral("=SUM(")));
        QVERIFY(!broken.ok);
        QVERIFY(broken.problem.contains(QStringLiteral("formula")));
        // a sheet that is not there
        QVERIFY(!s->defineName(draft(QStringLiteral("Nowhere"), QStringLiteral("1"), 9)).ok);
        QCOMPARE(e.win.w.undoStack()->count(), count + 1);   // (only the second sheet's own Rate was made)
    }

    void aRenamedNameIsFollowedByTheFormulasThatUseIt()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        const int rate = s->names().find(-1, QStringLiteral("Rate"));
        QVERIFY(s->changeName(rate, draft(QStringLiteral("Tax"), QStringLiteral("=Sheet1!$C$1"), -1, QStringLiteral("the rate"))).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Change Name"));   // (one step for all of it)
        QCOMPARE(textOf(e, 0, QStringLiteral("A1")), QStringLiteral("=Tax*2"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("A1")), QStringLiteral("n:10"));
        QCOMPARE(s->names().find(-1, QStringLiteral("Rate")), -1);
        QVERIFY(s->names().find(-1, QStringLiteral("tax")) >= 0);
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(textOf(e, 0, QStringLiteral("A1")), QStringLiteral("=Rate*2"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("A1")), QStringLiteral("n:10"));
        QVERIFY(s->names().find(-1, QStringLiteral("Rate")) >= 0);
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(textOf(e, 0, QStringLiteral("A1")), QStringLiteral("=Tax*2"));
    }

    void theNamesAreSavedAndTheOnesNotChangedAreLeftByteForByte()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        QVERIFY(s->defineName(draft(QStringLiteral("Mine"), QStringLiteral("Sheet1!$C$1:$C$2"), -1, QStringLiteral("a <b> & c"))).ok);
        QVERIFY(s->defineName(draft(QStringLiteral("Own"), QStringLiteral("Sheet2!$A$1"), 1)).ok);
        const int rate = s->names().find(-1, QStringLiteral("Rate"));
        QVERIFY(s->changeName(rate, draft(QStringLiteral("Rate"), QStringLiteral("Sheet1!$C$2"))).ok);   // (the comment goes)
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.names(), QStringList({QStringLiteral("Rate|-1|Sheet1!$C$2||0"), QStringLiteral("Keep|-1|Sheet1!$C$2||1"), QStringLiteral("Local|1|Sheet2!$A$1||0"),
                                         QStringLiteral("Mine|-1|Sheet1!$C$1:$C$2|a <b> & c|0"), QStringLiteral("Own|1|Sheet2!$A$1||0")}));
        QVERIFY(f.workbookXml().contains(kKeep.toUtf8()));
        QVERIFY(f.workbookXml().contains("<definedName name=\"Local\" localSheetId=\"1\">Sheet2!$A$1</definedName>"));
        // taken away: gone from the file; the rest stays
        QVERIFY(s->removeName(s->names().find(-1, QStringLiteral("Mine"))).ok);
        QVERIFY(saveNow(e));
        Saved g;
        QVERIFY(g.open(e.path));
        QCOMPARE(g.names().size(), 4);
        QVERIFY(!g.names().join(QLatin1Char(' ')).contains(QStringLiteral("Mine")));
        QVERIFY(g.workbookXml().contains(kKeep.toUtf8()));
        // Undo after the save gives it back to the next one
        e.win.w.undoStack()->undo();
        QVERIFY(saveNow(e));
        Saved h;
        QVERIFY(h.open(e.path));
        QCOMPARE(h.names().size(), 5);
        QVERIFY(h.names().join(QLatin1Char(' ')).contains(QStringLiteral("Mine|-1|Sheet1!$C$1:$C$2|a <b> & c|0")));
    }

    void aRenamedNameIsWrittenInTheFormulasOfTheFile()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        QVERIFY(s->changeName(s->names().find(-1, QStringLiteral("Rate")), draft(QStringLiteral("Tax"), QStringLiteral("Sheet1!$C$1"), -1, QStringLiteral("the rate"))).ok);
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.formula(0, QStringLiteral("A1")), QStringLiteral("Tax*2"));
        QCOMPARE(f.formula(0, QStringLiteral("B1")), QStringLiteral("SUM(Keep,1)"));
        QVERIFY(f.names().first().startsWith(QStringLiteral("Tax|-1|Sheet1!$C$1|the rate|0")));
        QVERIFY(f.workbookXml().contains(kKeep.toUtf8()));
    }

    void aRenamedNameIsWrittenInTheRulesAndOtherNamesOfTheFileToo()
    {
        const QString rows = row(1, cell(QStringLiteral("A1"), QStringLiteral("5")) + cell(QStringLiteral("C1"), QStringLiteral("9")) + formula(QStringLiteral("D1"), QStringLiteral("Rate+1")));
        const QString after = QStringLiteral("<conditionalFormatting sqref=\"A1\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>A1&gt;Rate</formula></cfRule></conditionalFormatting>"
                                             "<dataValidations count=\"1\"><dataValidation type=\"whole\" operator=\"lessThan\" sqref=\"A1\"><formula1>Rate</formula1></dataValidation></dataValidations>");
        const QByteArray book = buildXlsx({worksheet(rows, after)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(),
                                          QStringLiteral("<definedName name=\"Rate\">Sheet1!$C$1</definedName><definedName name=\"Twice\">Rate*2</definedName>"));
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        QVERIFY(s->changeName(s->names().find(-1, QStringLiteral("Rate")), draft(QStringLiteral("Tax"), QStringLiteral("Sheet1!$C$1"))).ok);
        QCOMPARE(s->nameTextOf(s->names().find(-1, QStringLiteral("Twice"))), QStringLiteral("=Tax*2"));   // (in the model at once)
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        Sheet sheet;
        QVERIFY(f.book.readSheet(0, &sheet));
        QCOMPARE(sheet.validations.size(), 1);
        QCOMPARE(sheet.validations[0].formula1, QStringLiteral("Tax"));
        QCOMPARE(sheet.conditionalFormats.size(), 1);
        QCOMPARE(sheet.conditionalFormats[0].formula1, QStringLiteral("A1>Tax"));
        QStringList names = f.names();
        QVERIFY2(names.contains(QStringLiteral("Twice|-1|Tax*2||0")), qPrintable(names.join(QLatin1Char(' '))));
        QCOMPARE(f.formula(0, QStringLiteral("D1")), QStringLiteral("Tax+1"));
    }

    void whatAPersonTypesInTheirLocaleIsWrittenInTheFormulaLanguage()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::German, QLocale::Germany));
        QVERIFY(e.settle());
        int made = -1;
        QVERIFY(s->defineName(draft(QStringLiteral("Half"), QStringLiteral("=SUM(Sheet1!C1;Sheet1!C2)*0,5")), &made).ok);
        QCOMPARE(s->nameValueOf(made), QStringLiteral("6"));
        QCOMPARE(s->nameTextOf(made), QStringLiteral("=SUM(Sheet1!C1;Sheet1!C2)*0,5"));
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QVERIFY(f.names().join(QLatin1Char(' ')).contains(QStringLiteral("Half|-1|SUM(Sheet1!C1,Sheet1!C2)*0.5||0")));
    }

    void howTheWorkbookCalculatesIsChangedSavedAndUndone()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        QCOMPARE(int(s->calcSettings().mode), int(CalcMode::Auto));
        QVERIFY(!s->calcSettings().iterate);
        QSignalSpy changed(s, &crunch::EditSession::calcSettingsChanged);
        crunch::CalcSettings now = s->calcSettings();
        now.mode = CalcMode::Manual;
        now.iterate = true;
        now.count = 7;
        now.delta = 0.5;
        QVERIFY(s->setCalcSettings(now).ok);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Calculation Options"));
        QCOMPARE(changed.count(), 1);
        QCOMPARE(int(s->calcSettings().mode), int(CalcMode::Manual));
        QVERIFY(s->setCalcSettings(now).ok);   // (the same: nothing to do)
        QCOMPARE(changed.count(), 1);
        // (a workbook worked out by hand has formulas that wait: the save does not wait for them)
        auto save = [&] {
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            return e.win.w.save() && savedOk(saved);
        };
        QVERIFY(save());
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(int(f.book.info().calcMode), int(CalcMode::Manual));
        QVERIFY(f.book.info().iterate);
        QCOMPARE(f.book.info().iterateCount, 7);
        QCOMPARE(f.book.info().iterateDelta, 0.5);
        e.win.w.undoStack()->undo();
        QCOMPARE(changed.count(), 2);
        QCOMPARE(int(s->calcSettings().mode), int(CalcMode::Auto));
        QVERIFY(save());
        Saved g;
        QVERIFY(g.open(e.path));
        QCOMPARE(int(g.book.info().calcMode), int(CalcMode::Auto));
        QVERIFY(!g.book.info().iterate);
    }
};

JP_TEST_CLASS(CrunchNamesTests)
#include "tst_crunch_names.moc"
