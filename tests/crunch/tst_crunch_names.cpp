// Tests of the defined names of a workbook as a person changes them through the edit session (one step of Undo each, the rules a name keeps, the formulas that use a name following it when it
// is renamed, and what a save leaves of the file's own list of names), and of how the workbook calculates. The workbook is built here.
#include "../plainprobe.h"
#include "app/crunchwindow.h"
#include "app/namesdialog.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "savedok.h"

#include <QFile>
#include <QSignalSpy>
#include <QUndoStack>

#include <cmath>

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

// The formats: 1 "0.00", 2 "0%", 3 "0.00%", 4 "0.00E+00", 5 "# ?/?", 6 a currency, 7 a date, 8 a time, 9 "0", 10 "#,##0.0".
QByteArray precisionStyles()
{
    return stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"&quot;$&quot;#,##0.00\"/><numFmt numFmtId=\"165\" formatCode=\"yyyy\\-mm\\-dd\"/><numFmt numFmtId=\"166\" formatCode=\"h:mm:ss\"/><numFmt numFmtId=\"167\" formatCode=\"#,##0.0\"/>"),
                     QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"2\"/><xf numFmtId=\"9\"/><xf numFmtId=\"10\"/><xf numFmtId=\"11\"/><xf numFmtId=\"12\"/><xf numFmtId=\"164\"/><xf numFmtId=\"165\"/>"
                                    "<xf numFmtId=\"166\"/><xf numFmtId=\"1\"/><xf numFmtId=\"167\"/>"));
}
//   Sheet1 column A: one number of each format (A1 General ... A11 "#,##0.0"), A12 text, A13 an empty "0.00" cell; B1 = A2*2, B2 = A2/3 (both "0.00"), B3 = A1*3 (General)
//   Sheet2: A1 1.005 ("0.00")
QByteArray precisionBook(const QString &calcPr = QString(), const QString &after = QString())
{
    auto num = [](const QString &ref, const char *v, int style) { return QStringLiteral("<c r=\"%1\" s=\"%3\"><v>%2</v></c>").arg(ref, QString::fromLatin1(v)).arg(style); };
    auto fm = [](const QString &ref, const QString &f, int style) { return QStringLiteral("<c r=\"%1\" s=\"%3\"><f>%2</f><v>0</v></c>").arg(ref, f).arg(style); };
    const char *values[] = {"1.23456789", "2.675", "0.456", "0.12345", "12345.678", "3.14159", "1234.5678", "45366.75", "0.123456789", "7.5", "1234567.89"};
    const int styles[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    QString rows;
    for (int i = 0; i < 11; ++i) {
        QString cells = num(QStringLiteral("A%1").arg(i + 1), values[i], styles[i]);
        if (i == 0) cells += fm(QStringLiteral("B1"), QStringLiteral("A2*2"), 1);
        if (i == 1) cells += fm(QStringLiteral("B2"), QStringLiteral("A2/3"), 1);
        if (i == 2) cells += fm(QStringLiteral("B3"), QStringLiteral("A1*3"), 0);
        rows += row(i + 1, cells);
    }
    rows += row(12, QStringLiteral("<c r=\"A12\" t=\"inlineStr\"><is><t>text</t></is></c>"));
    rows += row(13, QStringLiteral("<c r=\"A13\" s=\"1\"/>"));
    const QByteArray two = worksheet(row(1, num(QStringLiteral("A1"), "1.005", 1)));
    return buildXlsx({worksheet(rows, after), two}, QByteArray(), QString(), calcPr, precisionStyles());
}
double numberAt(crunch::EditSession *s, int sheet, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return s->valueAt(sheet, r, c).number();
}
double D(const char *text) { return QString::fromLatin1(text).toDouble(); }
// Every value the book above has, as it is before anything is rounded (what a person would see with all the digits).
const std::vector<std::pair<const char *, double>> &precisionOriginals()
{
static const std::vector<std::pair<const char *, double>> v = {{"A1", 1.23456789}, {"A2", 2.675}, {"A3", 0.456}, {"A4", 0.12345}, {"A5", 12345.678}, {"A6", 3.14159}, {"A7", 1234.5678},
                                                                  {"A8", 45366.75}, {"A9", 0.123456789}, {"A10", 7.5}, {"A11", 1234567.89}};
    return v;
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

    // A name's comment and text come from the file: the list's tips show them as the text they are, not as markup that names a picture.
    void aNamesCommentIsShownInItsTipAsText()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        QVERIFY(plainprobe::probeSeesTheTrap());
        int made = -1;
        QVERIFY(s->defineName(draft(QStringLiteral("Trap"), QStringLiteral("=Sheet1!$C$1"), -1, plainprobe::hostile()), &made).ok);
        NamesModel model(s);
        int found = 0;
        for (int row = 0; row < model.rowCount(); ++row) {
            const QVariant tip = model.data(model.index(row, NamesModel::CommentColumn), Qt::ToolTipRole);
            if (!tip.toString().contains(QStringLiteral("probe.invalid"))) continue;
            ++found;
            QVERIFY2(!plainprobe::readsAPicture(tip.toString()), qPrintable(tip.toString()));
            QCOMPARE(model.data(model.index(row, NamesModel::CommentColumn), Qt::DisplayRole).toString(), plainprobe::hostile());   // (the list itself shows it as it is)
        }
        QCOMPARE(found, 1);
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
        QCOMPARE(u->undoText(), QStringLiteral("New Name"));
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

    // A name that was changed after a column it names was deleted: Undo of the change, then Undo of the delete, give the name back as the file had it. (In the id frame the name is held by the
    // column's id, and the step of a change has to keep it so, or the delete's Undo has no column to bring back to it.)
    void undoOfANameChangeAfterADeleteThenOfTheDeleteGivesTheNameBack()
    {
        {
            EditWindow e;
            QVERIFY(e.open(namesBook()));
            crunch::EditSession *s = e.win.w.session();
            s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
            QVERIFY(e.settle());
            QUndoStack *u = e.win.w.undoStack();
            const int rate = s->names().find(-1, QStringLiteral("Rate"));
            QVERIFY(rate >= 0);
            QVERIFY(s->deleteColumns(0, 2, 1).ok);   // the column Rate names (C)
            QVERIFY(e.settle());
            QVERIFY2(s->nameTextOf(rate).contains(QLatin1String("#REF!")), qPrintable(s->nameTextOf(rate)));
            QVERIFY(s->changeName(rate, draft(QStringLiteral("Rate"), QStringLiteral("Sheet1!$A$1"), -1, QStringLiteral("the rate"))).ok);
            QVERIFY(e.settle());
            QCOMPARE(s->nameTextOf(rate), QStringLiteral("=Sheet1!$A$1"));
            u->undo();   // the change
            QVERIFY(e.settle());
            QVERIFY2(s->nameTextOf(rate).contains(QLatin1String("#REF!")), qPrintable(s->nameTextOf(rate)));
            u->undo();   // the delete
            QVERIFY(e.settle());
            QCOMPARE(s->nameTextOf(rate), QStringLiteral("=Sheet1!$C$1"));
            QCOMPARE(s->nameValueOf(rate), QStringLiteral("5"));
            u->redo();
            u->redo();
            QVERIFY(e.settle());
            QCOMPARE(s->nameTextOf(rate), QStringLiteral("=Sheet1!$A$1"));
        }
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
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("How Formulas Are Worked Out"));
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

    // ---- precision as displayed (Set Precision as Displayed)
    void eachFormatFamilyRoundsTheNumbersToWhatTheyShow()
    {
        EditWindow e;
        QVERIFY(e.open(precisionBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        QVERIFY(!s->calcSettings().precisionAsShown);
        QVERIFY(!e.win.w.act(QStringLiteral("formulas.precision"))->isChecked());
        QVERIFY(s->setPrecisionAsShown(true).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Set Precision as Displayed"));
        QVERIFY(s->calcSettings().precisionAsShown);
        QVERIFY(e.win.w.act(QStringLiteral("formulas.precision"))->isChecked());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A1")), 1.23456789);             // General keeps the digits
        QCOMPARE(numberAt(s, 0, QStringLiteral("A2")), D("2.68"));              // 0.00 (2.675 shows 2.68)
        QCOMPARE(numberAt(s, 0, QStringLiteral("A3")), D("0.46"));              // 0%
        QCOMPARE(numberAt(s, 0, QStringLiteral("A4")), D("0.1235"));            // 0.00%
        QCOMPARE(numberAt(s, 0, QStringLiteral("A5")), 12300.0);                // 0.00E+00
        QVERIFY(std::fabs(numberAt(s, 0, QStringLiteral("A6")) - (3.0 + 1.0 / 7.0)) < 1e-12);   // # ?/?
        QCOMPARE(numberAt(s, 0, QStringLiteral("A7")), D("1234.57"));           // a currency
        QCOMPARE(numberAt(s, 0, QStringLiteral("A8")), 45366.0);                // a date: the day
        QVERIFY(std::fabs(numberAt(s, 0, QStringLiteral("A9")) - 10667.0 / 86400.0) < 1e-12);   // a time: the second
        QCOMPARE(numberAt(s, 0, QStringLiteral("A10")), 8.0);                   // 0 (7.5 shows 8)
        QCOMPARE(numberAt(s, 0, QStringLiteral("A11")), D("1234567.9"));        // #,##0.0
        QCOMPARE(numberAt(s, 1, QStringLiteral("A1")), D("1.01"));              // (another sheet: 1.005 shows 1.01)
        QCOMPARE(s->editText(0, 11, 0), QStringLiteral("text"));                // a text is as it was
        // the formulas are as they were, and what they give is rounded (and reads what is rounded)
        QCOMPARE(s->editText(0, 0, 1), QStringLiteral("=A2*2"));
        QCOMPARE(numberAt(s, 0, QStringLiteral("B1")), D("5.36"));              // 2.68*2 (2.675*2 is 5.35)
        QCOMPARE(numberAt(s, 0, QStringLiteral("B2")), D("0.89"));              // 2.68/3 = 0.8933
        QCOMPARE(numberAt(s, 0, QStringLiteral("B3")), 3.0 * 1.23456789);       // (General)
    }

    void oneUndoStepBringsBackEveryNumberAndRedoRoundsThemAgain()
    {
        EditWindow e;
        QVERIFY(e.open(precisionBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(s->setPrecisionAsShown(true).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);   // (one step)
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A2")), D("2.68"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QVERIFY(!s->calcSettings().precisionAsShown);
        for (const auto &o : precisionOriginals()) QCOMPARE(numberAt(s, 0, QString::fromLatin1(o.first)), o.second);
        QCOMPARE(numberAt(s, 1, QStringLiteral("A1")), 1.005);
        QCOMPARE(numberAt(s, 0, QStringLiteral("B1")), 2.675 * 2);   // (the formulas have all their digits again)
        QVERIFY(!e.win.w.act(QStringLiteral("formulas.precision"))->isChecked());
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QVERIFY(s->calcSettings().precisionAsShown);
        QCOMPARE(numberAt(s, 0, QStringLiteral("A2")), D("2.68"));
        QCOMPARE(numberAt(s, 0, QStringLiteral("B1")), D("5.36"));
    }

    // A date cell stays a date (its type, not only its number) when the rounding is done, undone, and done again.
    void aDateCellKeepsItsTypeThroughTheRoundingAndUndo()
    {
        const QString cells = QStringLiteral("<c r=\"A1\" s=\"7\" t=\"d\"><v>2024-03-15T18:30:20</v></c><c r=\"A2\" s=\"7\"><v>45366.75</v></c>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(1, cells))}, QByteArray(), QString(), QString(), precisionStyles())));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        auto type = [&](quint32 r, quint32 c) { return int(s->calculation()->sheet(0).cells.find(r, c)->type); };
        const double before = s->calculation()->sheet(0).cells.find(0, 0)->number;
        QVERIFY(before > 45366.7 && before < 45366.8);
        QCOMPARE(type(0, 0), int(jp::office::sheet::CellType::Date));
        QCOMPARE(type(1, 0), int(jp::office::sheet::CellType::Number));
        QVERIFY(s->setPrecisionAsShown(true).ok);
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A1")), 45366.0);
        QCOMPARE(type(0, 0), int(jp::office::sheet::CellType::Date));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A1")), before);
        QCOMPARE(type(0, 0), int(jp::office::sheet::CellType::Date));
        QCOMPARE(type(1, 0), int(jp::office::sheet::CellType::Number));
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A1")), 45366.0);
        QCOMPARE(type(0, 0), int(jp::office::sheet::CellType::Date));
    }

    // What Undo holds is what a step may change: a rounding of more numbers than that is refused, with how many it would change, and nothing is changed; one that fits stays undoable.
    void aRoundingThatUndoCouldNotKeepIsRefusedWithTheCount()
    {
        EditWindow e;
        QVERIFY(e.open(precisionBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        QString why;
        auto plan = s->planPrecisionAsShown(&why);
        QVERIFY(plan);
        QVERIFY(s->preparePrecision(*plan, 0));
        const qint64 n = crunch::EditSession::precisionChanges(*plan);
        QVERIFY(n > 3);
        const qint64 per = qint64(sizeof(crunch::NumberCell));
        const qint64 other = qint64(sizeof(crunch::CalcEdit));
        const qint64 steps = e.win.w.undoStack()->count();
        // one number too many for the budget
        s->setUndoByteLimit(2 * ((n - 1) * per + other));
        QCOMPARE(s->precisionHolds(), n - 1);
        const crunch::EditSession::Outcome refused = s->setPrecisionAsShown(true);
        QVERIFY(!refused.ok);
        QVERIFY2(refused.problem.contains(QString::number(n)) && refused.problem.contains(QString::number(n - 1)), qPrintable(refused.problem));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!s->calcSettings().precisionAsShown);
        QCOMPARE(numberAt(s, 0, QStringLiteral("A2")), 2.675);
        // as many as it holds: done, and still there to undo after the next edit
        s->setUndoByteLimit(2 * (n * per + other) + 4096);   // (and room for the small edit that follows)
        QVERIFY(s->precisionHolds() >= n);
        QVERIFY(s->setPrecisionAsShown(true).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(e.settle());
        QVERIFY(s->type(0, 12, 1, QStringLiteral("5")).ok);
        QVERIFY(s->canUndo());
        QCOMPARE(e.win.w.undoStack()->index(), steps + 2);
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A2")), 2.675);
    }

    void whileItIsOnNewNumbersAndFormulaResultsAreKeptAsShown()
    {
        EditWindow e;
        QVERIFY(e.open(precisionBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        QVERIFY(s->setPrecisionAsShown(true).ok);
        QVERIFY(e.settle());
        // typed into a cell that shows two decimals: kept with two
        QVERIFY(s->type(0, 12, 0, QStringLiteral("1.23456")).ok);
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A13")), D("1.23"));
        QCOMPARE(s->editText(0, 12, 0), QStringLiteral("1.23"));
        // a formula's result changes with what it reads
        QVERIFY(s->type(0, 1, 0, QStringLiteral("3.14159")).ok);
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A2")), D("3.14"));
        QCOMPARE(numberAt(s, 0, QStringLiteral("B1")), D("6.28"));
        QCOMPARE(numberAt(s, 0, QStringLiteral("B2")), D("1.05"));   // 3.14/3 = 1.0467
        // a formula typed in a cell that shows two decimals
        QVERIFY(s->type(0, 12, 0, QStringLiteral("=1/3")).ok);
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A13")), D("0.33"));
        QCOMPARE(s->editText(0, 12, 0), QStringLiteral("=1/3"));   // (the formula is as typed)
        // a typed step is one step, and undone it brings the number it had back
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A13")), D("1.23"));
        QVERIFY(s->calcSettings().precisionAsShown);
    }

    void turningItOffStopsTheRoundingAndLeavesWhatIsRounded()
    {
        EditWindow e;
        QVERIFY(e.open(precisionBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(e.settle());
        QVERIFY(s->setPrecisionAsShown(true).ok);
        QVERIFY(e.settle());
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(s->setPrecisionAsShown(false).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(e.settle());
        QVERIFY(!s->calcSettings().precisionAsShown);
        QVERIFY(!e.win.w.act(QStringLiteral("formulas.precision"))->isChecked());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A4")), D("0.1235"));   // (the digits that were dropped do not come back)
        QCOMPARE(numberAt(s, 0, QStringLiteral("B2")), 2.68 / 3.0);   // (the formula has all its digits now)
        QVERIFY(s->type(0, 12, 0, QStringLiteral("9.87654")).ok);
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A13")), 9.87654);
        QVERIFY(s->setPrecisionAsShown(false).ok);   // (already off: nothing to do)
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
        // undoing the turning off turns it on again
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QVERIFY(s->calcSettings().precisionAsShown);
        QCOMPARE(numberAt(s, 0, QStringLiteral("B2")), D("0.89"));
    }

    void thePrecisionIsAskedForAndCancelKeepsEverything()
    {
        EditWindow e;
        QVERIFY(e.open(precisionBook()));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(e.settle());
        QStringList asked;
        bool agree = false;
        w.setConfirmAsker([&](const QString &title, const QString &text) {
            asked << title + QLatin1Char('|') + text;
            return agree;
        });
        const int steps = w.undoStack()->count();
        w.act(QStringLiteral("formulas.precision"))->trigger();   // (Cancel)
        QVERIFY(e.settle());
        QCOMPARE(asked.size(), 1);
        QVERIFY2(asked[0].startsWith(QStringLiteral("Set Precision as Displayed|")) && asked[0].contains(QStringLiteral("lost")), qPrintable(asked[0]));
        QVERIFY(!s->calcSettings().precisionAsShown);
        QVERIFY(!w.act(QStringLiteral("formulas.precision"))->isChecked());   // (the check mark is put back)
        QCOMPARE(w.undoStack()->count(), steps);
        QVERIFY(!w.isModified());
        for (const auto &o : precisionOriginals()) QCOMPARE(numberAt(s, 0, QString::fromLatin1(o.first)), o.second);
        agree = true;
        w.act(QStringLiteral("formulas.precision"))->trigger();   // (OK)
        QTRY_VERIFY(s->calcSettings().precisionAsShown);
        QTRY_VERIFY(!w.isApplying());
        QVERIFY(e.settle());
        QCOMPARE(asked.size(), 2);
        QCOMPARE(numberAt(s, 0, QStringLiteral("A2")), D("2.68"));
        QVERIFY(w.act(QStringLiteral("formulas.precision"))->isChecked());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        // turned off with the same command, with no question
        w.act(QStringLiteral("formulas.precision"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(asked.size(), 2);
        QVERIFY(!s->calcSettings().precisionAsShown);
    }

    void aSheetThatIsProtectedWithLockedNumbersIsNotRounded()
    {
        EditWindow e;
        QVERIFY(e.open(precisionBook(QString(), QStringLiteral("<sheetProtection sheet=\"1\"/>"))));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const int steps = e.win.w.undoStack()->count();
        const crunch::EditSession::Outcome o = s->setPrecisionAsShown(true);
        QVERIFY(!o.ok);
        QVERIFY2(o.problem.contains(QStringLiteral("protected")), qPrintable(o.problem));
        QVERIFY(!s->calcSettings().precisionAsShown);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(numberAt(s, 0, QStringLiteral("A2")), 2.675);
        QCOMPARE(numberAt(s, 1, QStringLiteral("A1")), 1.005);   // (nothing on any sheet)
    }

    void aBigSheetIsLookedAtAndRoundedASliceAtATimeAndCancelChangesNothing()
    {
        constexpr int kRows = 2000, kCols = 50;
        QString rows;
        // (columns past Z are named by two letters)
        auto colName = [](int c) { return c < 26 ? QString(QChar('A' + c)) : QString(QChar('A' + c / 26 - 1)) + QChar('A' + c % 26); };
        for (int r = 1; r <= kRows; ++r) {
            QString cells;
            for (int c = 0; c < kCols; ++c) cells += QStringLiteral("<c r=\"%1%2\" s=\"1\"><v>%3</v></c>").arg(colName(c)).arg(r).arg(r + 0.123456, 0, 'g', 15);
            rows += row(r, cells);
        }
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), precisionStyles())));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const int steps = e.win.w.undoStack()->count();
        QString why;
        auto plan = s->planPrecisionAsShown(&why);
        QVERIFY2(plan, qPrintable(why));
        QCOMPARE(crunch::EditSession::precisionTotal(*plan), qint64(kRows) * kCols);
        // a slice looks at about its budget (and the rest of its row): the work is in many calls
        constexpr qint64 kBudget = 1000;
        int calls = 0;
        qint64 most = 0, last = 0;
        while (calls < 40 && !s->preparePrecision(*plan, kBudget)) {
            const qint64 now = crunch::EditSession::precisionLooked(*plan);
            most = std::max(most, now - last);
            last = now;
            ++calls;
        }
        QCOMPARE(calls, 40);   // (a hundred thousand cells are not looked at in forty slices of a thousand)
        QVERIFY(most <= kBudget + kCols);
        QVERIFY(most >= kBudget / 2);
        // dropped half way: nothing is changed
        plan.reset();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!s->calcSettings().precisionAsShown);
        QVERIFY(!e.win.w.isModified());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A1")), 1.123456);
        // all of it, put in a slice at a time as one step
        plan = s->planPrecisionAsShown(&why);
        QVERIFY(plan);
        int slices = 0;
        while (!s->preparePrecision(*plan, kBudget)) QVERIFY(++slices < 1000);
        QVERIFY(slices >= 90);
        QCOMPARE(crunch::EditSession::precisionLooked(*plan), qint64(kRows) * kCols);
        QCOMPARE(crunch::EditSession::precisionChanges(*plan), qint64(kRows) * kCols);
        int pushes = 0;
        crunch::EditSession::Outcome o;
        do {
            o = s->applyPrecision(*plan, 5000);
            QVERIFY2(o.ok, qPrintable(o.problem));
            if (o.pending) QVERIFY(s->isSlicing());
            QVERIFY(++pushes < 1000);
        } while (o.pending);
        QVERIFY2(pushes >= 20 && pushes <= 21, qPrintable(QString::number(pushes)));   // (a slice does about 5,000 numbers: 20 slices, and the call that puts the step on the stack)
        QVERIFY(!s->isSlicing());
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(numberAt(s, 0, QStringLiteral("A1")), D("1.12"));
        QCOMPARE(numberAt(s, 0, QStringLiteral("AX2000")), D("2000.12"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(numberAt(s, 0, QStringLiteral("A1")), 1.123456);
        QCOMPARE(numberAt(s, 0, QStringLiteral("AX2000")), 2000.123456);
        QVERIFY(!s->calcSettings().precisionAsShown);
    }

    void precisionAsDisplayedIsSavedInTheFileAndTheOtherCalcPrAttributesAreKept()
    {
        const QString other = QStringLiteral("calcId=\"191029\" refMode=\"R1C1\" iterate=\"1\" iterateCount=\"9\"");
        {
            // on: written as fullPrecision="0", with the other attributes as they were
            EditWindow e;
            QVERIFY(e.open(precisionBook(other)));
            crunch::EditSession *s = e.win.w.session();
            QVERIFY(e.settle());
            QVERIFY(s->setPrecisionAsShown(true).ok);
            QVERIFY(saveNow(e));
            Saved f;
            QVERIFY(f.open(e.path));
            QVERIFY(f.book.info().precisionAsShown);
            const QByteArray xml = f.workbookXml();
            QVERIFY2(xml.contains("fullPrecision=\"0\"") && xml.contains("calcId=\"191029\"") && xml.contains("refMode=\"R1C1\"") && xml.contains("iterateCount=\"9\""), xml.constData());
            QCOMPARE(savedCell(e.path, QStringLiteral("A2")).mid(2).toDouble(), D("2.68"));
            // and it is on when the file is opened again
            EditWindow again;
            QByteArray bytes;
            {
                QFile saved(e.path);
                QVERIFY(saved.open(QIODevice::ReadOnly));
                bytes = saved.readAll();
            }
            QVERIFY(again.open(bytes));
            QVERIFY(again.settle());
            QVERIFY(again.win.w.session()->calcSettings().precisionAsShown);
            QVERIFY(again.win.w.act(QStringLiteral("formulas.precision"))->isChecked());
            // off again: the attribute goes, and the others stay
            QVERIFY(again.win.w.session()->setPrecisionAsShown(false).ok);
            QVERIFY(saveNow(again));
            Saved g;
            QVERIFY(g.open(again.path));
            QVERIFY(!g.book.info().precisionAsShown);
            const QByteArray plain = g.workbookXml();
            QVERIFY2(!plain.contains("fullPrecision") && plain.contains("calcId=\"191029\"") && plain.contains("refMode=\"R1C1\"") && plain.contains("iterateCount=\"9\""), plain.constData());
        }
        {
            // a file with no calcPr gets one; a file that has the setting and is not changed is left as it is
            EditWindow e;
            QVERIFY(e.open(precisionBook()));
            QVERIFY(e.settle());
            QVERIFY(e.win.w.session()->setPrecisionAsShown(true).ok);
            QVERIFY(saveNow(e));
            QVERIFY(e.win.w.session()->setPrecisionAsShown(true).problem.contains(QStringLiteral("already")));
            Saved f;
            QVERIFY(f.open(e.path));
            QVERIFY(f.workbookXml().contains("fullPrecision=\"0\""));
        }
    }

    void calculateBeforeSaveWorksTheFormulasOutBeforeAFileByHandIsWritten()
    {
        EditWindow e;
        QVERIFY(e.open(namesBook(QStringLiteral("calcMode=\"manual\""))));
        CrunchWindow &w = e.win.w;
        QTRY_VERIFY(!w.isCalculating());
        QVERIFY(w.session()->calcSettings().calcOnSave);
        QVERIFY(w.act(QStringLiteral("formulas.calcBeforeSave"))->isChecked());
        QVERIFY(w.session()->type(0, 0, 2, QStringLiteral("9")).ok);
        QTRY_VERIFY(w.calculationDue());
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(savedOk(saved));
        QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:18"));
        Saved f;
        QVERIFY(f.open(e.path));
        QVERIFY(f.book.info().calcOnSave);
        QVERIFY(!f.workbookXml().contains("calcOnSave"));
        // switched off: the file keeps the values it had, and says so
        w.act(QStringLiteral("formulas.calcBeforeSave"))->trigger();
        QVERIFY(!w.session()->calcSettings().calcOnSave);
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("How Formulas Are Worked Out"));
        QVERIFY(w.session()->type(0, 0, 2, QStringLiteral("11")).ok);
        QTRY_VERIFY(w.calculationDue());
        QSignalSpy again(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(savedOk(again));
        QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:18"));
        Saved g;
        QVERIFY(g.open(e.path));
        QVERIFY(!g.book.info().calcOnSave);
        QVERIFY(g.workbookXml().contains("calcOnSave=\"0\""));
        w.undoStack()->undo();   // (the cell)
        w.undoStack()->undo();   // (the switch)
        QVERIFY(w.act(QStringLiteral("formulas.calcBeforeSave"))->isChecked());
    }
};

JP_TEST_CLASS(CrunchNamesTests)
#include "tst_crunch_names.moc"
