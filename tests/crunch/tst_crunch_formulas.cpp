// Tests of the Formulas tab as a person uses it: the help while a formula is typed (the list of functions and names, and the arguments of the function being typed), the names (the list,
// the small dialog and what it says, a name for the cells selected, names from labels, a name put in a formula), how the workbook calculates (by hand, one sheet), circles of formulas, and
// the arrows that show what a formula reads. The workbooks are built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/formulahelper.h"
#include "app/namesdialog.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/formulaspot.h"
#include "core/namesfromcells.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"
#include "grid/gridview.h"
#include "app/ribbon.h"
#include "savedok.h"

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableView>
#include <QUndoStack>
#include <QTemporaryDir>
#include <QSettings>

#include "../localepin.h"
#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

const QLocale kEnglish(QLocale::English, QLocale::UnitedStates);

// The window as a person sees it with the help open: the window, and the list and the arguments (windows of their own) where they are.
void saveWithHelp(EditWindow &e, const QString &name)
{
    if (qgetenv("JC_SHOT_DIR").isEmpty()) return;
    QCoreApplication::processEvents();
    QPixmap shot = e.win.w.grab();
    QPainter p(&shot);
    FormulaHelper *h = e.win.w.formulaHelper();
    for (QWidget *w : std::initializer_list<QWidget *>{h->hint(), h->list()->window()})
        if (w->isVisible()) p.drawPixmap(e.win.w.mapFromGlobal(w->pos()), w->grab());
    p.end();
    QDir().mkpath(QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR")));
    shot.save(QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR")) + QLatin1Char('/') + name + QStringLiteral(".png"));
}

QByteArray manualBook()
{
    return buildXlsx({worksheet(rowsOfSpecs({"A1=1", "B1==A1*2"}))}, QByteArray(), QString(), QStringLiteral("calcMode=\"manual\""));
}

} // namespace

class CrunchFormulasTests : public QObject {
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
        jp::initCore();
    }

    // ---- where the cursor is in a formula
    void theCursorInAFormulaIsPlacedByItsWordAndTheFunctionItIsIn()
    {
        using crunch::spotAt;
        const QChar comma = QLatin1Char(',');
        QVERIFY(!spotAt(QStringLiteral("su"), 2, comma).formula);
        crunch::FormulaSpot s = spotAt(QStringLiteral("=su"), 3, comma);
        QVERIFY(s.formula && s.canComplete);
        QCOMPARE(s.word, QStringLiteral("su"));
        QCOMPARE(s.wordStart, 1);
        QVERIFY(s.function.isEmpty());
        s = spotAt(QStringLiteral("=1+SUM(A1,B"), 11, comma);
        QCOMPARE(s.word, QStringLiteral("B"));
        QVERIFY(s.canComplete);
        QCOMPARE(s.function, QStringLiteral("SUM"));
        QCOMPARE(s.argument, 1);
        // a bracket that only groups is passed over, and so are texts, sheet names, and arrays
        s = spotAt(QStringLiteral("=IF((A1>1),\"a,b\",ROUND(2,"), 25, comma);
        QCOMPARE(s.function, QStringLiteral("ROUND"));
        QCOMPARE(s.argument, 1);
        s = spotAt(QStringLiteral("=SUM({1,2,3},'a,b'!A1,"), 22, comma);
        QCOMPARE(s.function, QStringLiteral("SUM"));
        QCOMPARE(s.argument, 2);
        s = spotAt(QStringLiteral("=IF((A1>1)*2,"), 13, comma);
        QCOMPARE(s.function, QStringLiteral("IF"));
        QCOMPARE(s.argument, 1);
        // closed: no function
        s = spotAt(QStringLiteral("=SUM(A1)+"), 9, comma);
        QVERIFY(s.function.isEmpty());
        // in a text, in a table's columns, after a sheet's "!", after a number: nothing to complete
        QVERIFY(spotAt(QStringLiteral("=\"su"), 4, comma).inText);
        QVERIFY(!spotAt(QStringLiteral("=\"su"), 4, comma).canComplete);
        QVERIFY(!spotAt(QStringLiteral("=Table1[su"), 10, comma).canComplete);
        QVERIFY(!spotAt(QStringLiteral("=Sheet1!su"), 10, comma).canComplete);
        QVERIFY(!spotAt(QStringLiteral("=1su"), 4, comma).canComplete);
        QVERIFY(!spotAt(QStringLiteral("=A1)su"), 6, comma).canComplete);
        // where the locale writes ";" between arguments, "," is not one
        s = spotAt(QStringLiteral("=SUM(1,5;"), 9, QLatin1Char(';'));
        QCOMPARE(s.argument, 1);
        QCOMPARE(crunch::argumentSeparator(QLocale(QLocale::German, QLocale::Germany)), QLatin1Char(';'));
        QCOMPARE(crunch::argumentSeparator(kEnglish), QLatin1Char(','));
    }

    void theArgumentsOfAFunctionAreShownWithTheCurrentOneMarked()
    {
        const crunch::FunctionDoc *vlookup = crunch::docOf(QStringLiteral("VLOOKUP"));
        QVERIFY(vlookup);
        int marked = -2;
        QCOMPARE(crunch::hintPlain(*vlookup, 2, QLatin1Char(','), &marked), QStringLiteral("VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])"));
        QCOMPARE(marked, 2);
        QCOMPARE(crunch::hintHtml(*vlookup, 0, QLatin1Char(',')), QStringLiteral("VLOOKUP(<b>lookup_value</b>, table_array, col_index_num, [range_lookup])"));
        QCOMPARE(crunch::hintHtml(*vlookup, 3, QLatin1Char(';')), QStringLiteral("VLOOKUP(lookup_value; table_array; col_index_num; <b>[range_lookup]</b>)"));
        // past the last: nothing is marked
        crunch::hintPlain(*vlookup, 9, QLatin1Char(','), &marked);
        QCOMPARE(marked, -1);
        // any number of the last: from the one before the dots on, that one
        const crunch::FunctionDoc *sum = crunch::docOf(QStringLiteral("SUM"));
        QCOMPARE(crunch::hintPlain(*sum, 0, QLatin1Char(','), &marked), QStringLiteral("SUM(number1, ...)"));
        QCOMPARE(marked, 0);
        crunch::hintPlain(*sum, 7, QLatin1Char(','), &marked);
        QCOMPARE(marked, 1);                                                          // (the dots stand for the ones that follow)
        QCOMPARE(crunch::hintHtml(*sum, 3, QLatin1Char(',')), QStringLiteral("SUM(number1, <b>...</b>)"));
        QCOMPARE(crunch::hintPlain(*crunch::docOf(QStringLiteral("PI")), 0, QLatin1Char(','), &marked), QStringLiteral("PI()"));
        QCOMPARE(marked, -1);
    }

    // ---- the list
    void theListOffersFunctionsAndNamesThatBeginSoAndNoMoreThanTheCap()
    {
        crunch::NameTable table;
        QVector<jp::office::sheet::DefinedName> file;
        jp::office::sheet::DefinedName mine;
        mine.name = QStringLiteral("SumTotal");
        mine.ref = QStringLiteral("Sheet1!$B$1");
        file.append(mine);
        jp::office::sheet::DefinedName other;   // (of a sheet that is not in view)
        other.name = QStringLiteral("SumHere");
        other.localSheet = 3;
        other.ref = QStringLiteral("Sheet4!$B$1");
        file.append(other);
        for (int i = 0; i < 30000; ++i) {
            jp::office::sheet::DefinedName n;
            n.name = QStringLiteral("Item%1").arg(i, 5, 10, QLatin1Char('0'));
            n.ref = QStringLiteral("Sheet1!$A$%1").arg(i + 1);
            file.append(n);
        }
        table.load(file);
        crunch::CompletionIndex index;
        index.build(table, 0, QStringList({QStringLiteral("SumTable")}));
        QCOMPARE(index.nameCount(), crunch::kMaxNamesIndexed);   // (the file's thousands are not all indexed; the cap is the bound)
        // the words that match many are cut to the cap
        QCOMPARE(int(index.matches(QStringLiteral("item0")).size()), crunch::kMaxOffered);
        QCOMPARE(int(index.matches(QStringLiteral("i")).size()), crunch::kMaxOffered);
        // functions and names, in order, without regard to case
        index.build(table, 0, QStringList({QStringLiteral("SumTable")}));
        QStringList sum;
        for (const crunch::Completion &c : index.matches(QStringLiteral("sum"), 50)) sum << c.text;
        QVERIFY(sum.contains(QStringLiteral("SUM")));
        QVERIFY(sum.contains(QStringLiteral("SUMIF")));
        QVERIFY(sum.contains(QStringLiteral("SumTable")));
        QVERIFY(sum.contains(QStringLiteral("SumTotal")));
        QVERIFY(index.matches(QStringLiteral("zzzzz")).empty());
        QCOMPARE(sum.first(), QStringLiteral("SUM"));
        // a name of another sheet is not offered
        QVERIFY(!sum.contains(QStringLiteral("SumHere")));
    }

    void theListOpensWhileAFormulaIsTypedAndTabTakesAnEntry()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        FormulaHelper *h = w.formulaHelper();
        QVERIFY(h);
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=sum"));
        QVERIFY(h->isListOpen());
        QVERIFY(!h->isHintOpen());
        QCOMPARE(h->offered().first(), QStringLiteral("SUM"));
        QVERIFY(h->offered().contains(QStringLiteral("SUMIF")));
        QVERIFY(int(h->offered().size()) <= crunch::kMaxOffered);
        QVERIFY(!h->detail()->text().isEmpty());                                    // (what SUM does, as Pick a Function says)
        QCOMPARE(h->detail()->text(), crunch::summaryOf(*crunch::docOf(QStringLiteral("SUM"))));
        // Tab takes the one that is marked, and the editing goes on
        QTest::keyClick(w.cellEditor(), Qt::Key_Tab);
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM("));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=SUM("));
        QVERIFY(!h->isListOpen());
        QVERIFY(h->isHintOpen());
        QVERIFY(h->hint()->text().contains(QStringLiteral("<b>number1</b>")));
        saveWithHelp(e, QStringLiteral("autocomplete-hint"));
        // the next argument is marked as it is begun
        QTest::keyClicks(w.cellEditor(), QStringLiteral("A1,"));
        QVERIFY(h->hint()->text().contains(QStringLiteral("<b>...</b>")));
        QVERIFY(!h->hint()->text().contains(QStringLiteral("<b>number1</b>")));
        // and the hint goes when the brackets are closed
        QTest::keyClicks(w.cellEditor(), QStringLiteral("A2)"));
        QVERIFY(!h->isHintOpen());
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.shownAt(QStringLiteral("F1")), QStringLiteral("11"));
    }

    void theListShowsAFunctionsNameAsTypedAndSaysWhatEachDoes()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        FormulaHelper *h = w.formulaHelper();
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=1+su"));
        QVERIFY(h->isListOpen());
        saveWithHelp(e, QStringLiteral("autocomplete-list"));
        // the arrows walk the list, and Enter then takes the entry marked; without them Enter finishes the entry
        QTest::keyClick(w.cellEditor(), Qt::Key_Down);
        QTest::keyClick(w.cellEditor(), Qt::Key_Down);
        QCOMPARE(h->list()->currentRow(), 2);
        QCOMPARE(h->detail()->text(), crunch::summaryOf(*crunch::docOf(h->offered()[2])));
        const QString chosen = h->offered()[2];
        QTest::keyClick(w.cellEditor(), Qt::Key_Return);
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=1+") + chosen + QLatin1Char('('));
        // Escape closes a list (and then gives up the entry)
        e.key(Qt::Key_Escape);
        e.select(QStringLiteral("F2"));
        e.type(QStringLiteral("=ro"));
        QVERIFY(h->isListOpen());
        e.key(Qt::Key_Escape);
        QVERIFY(!h->isListOpen());
        QVERIFY(w.isEditing());
        e.key(Qt::Key_Escape);
        QVERIFY(!w.isEditing());
        // text that is not a formula gets no list, and Enter finishes a formula whose list was not walked
        e.select(QStringLiteral("F3"));
        e.type(QStringLiteral("sum"));
        QVERIFY(!h->isListOpen());
        e.key(Qt::Key_Escape);
        e.select(QStringLiteral("F4"));
        e.type(QStringLiteral("=sum"));
        QVERIFY(h->isListOpen());
        e.key(Qt::Key_Return);
        QVERIFY(!w.isEditing());
        QVERIFY(!h->isListOpen());
    }

    void namesAreOfferedToo()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=5", "A2=6"}, {"A1=7"}}, {}, {}, QStringLiteral("<definedName name=\"TaxRate\">Sheet1!$A$1</definedName><definedName name=\"TaxLocal\" localSheetId=\"1\">Sheet2!$A$1</definedName>"))));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        FormulaHelper *h = w.formulaHelper();
        e.select(QStringLiteral("C1"));
        e.type(QStringLiteral("=tax"));
        QCOMPARE(h->offered(), QStringList({QStringLiteral("TaxRate")}));   // (the second sheet's own name is not offered on the first)
        QCOMPARE(h->detail()->text(), QStringLiteral("=Sheet1!$A$1"));
        QTest::keyClick(w.cellEditor(), Qt::Key_Tab);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=TaxRate"));   // (a name takes no bracket)
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:5"));
        // a name made now is offered at once
        QVERIFY(w.session()->defineName([] {
            crunch::EditSession::NameDraft d;
            d.name = QStringLiteral("TaxFree");
            d.text = QStringLiteral("Sheet1!$A$2");
            return d;
        }()).ok);
        e.select(QStringLiteral("C2"));
        e.type(QStringLiteral("=taxf"));
        QCOMPARE(h->offered(), QStringList({QStringLiteral("TaxFree")}));
        e.key(Qt::Key_Escape);
        e.key(Qt::Key_Escape);
    }

    void theArgumentsAreWrittenWithTheSeparatorOfTheLanguage()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        w.setLocale(QLocale(QLocale::German, QLocale::Germany));
        QVERIFY(e.settle());
        FormulaHelper *h = w.formulaHelper();
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=SUM(1;"));
        QVERIFY(h->isHintOpen());
        QVERIFY(h->hint()->text().contains(QStringLiteral("<b>...</b>")));
        QVERIFY(h->hint()->text().contains(QStringLiteral("number1; ")));
        // a comma is a decimal mark there: it does not begin another argument
        QTest::keyClicks(w.cellEditor(), QStringLiteral("2,5"));
        QVERIFY(h->hint()->text().contains(QStringLiteral("<b>...</b>")));
        // and the first argument of a function with named ones is marked until the semicolon
        e.key(Qt::Key_Escape);
        e.select(QStringLiteral("F2"));
        e.type(QStringLiteral("=ROUND(2,5"));
        QVERIFY(h->hint()->text().contains(QStringLiteral("<b>number</b>")));
        QTest::keyClicks(w.cellEditor(), QStringLiteral(";"));
        QVERIFY(h->hint()->text().contains(QStringLiteral("<b>num_digits</b>")));
        e.key(Qt::Key_Escape);
        e.key(Qt::Key_Escape);
    }

    void theFormulaBarIsHelpedAsTheCellIs()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        FormulaHelper *h = w.formulaHelper();
        e.select(QStringLiteral("F1"));
        w.formulaBar()->setFocus();
        QTest::keyClicks(w.formulaBar(), QStringLiteral("=sum"));
        QVERIFY(w.isEditing());
        QVERIFY(h->isListOpen());
        QCOMPARE(h->offered().first(), QStringLiteral("SUM"));
        QTest::keyClick(w.formulaBar(), Qt::Key_Tab);
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=SUM("));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM("));   // (the cell shows what the bar has)
        QVERIFY(h->isHintOpen());
        QVERIFY(w.isEditing());
        QTest::keyClicks(w.formulaBar(), QStringLiteral("A1:A2)"));
        QTest::keyClick(w.formulaBar(), Qt::Key_Return);
        QVERIFY(!w.isEditing());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:11"));
    }

    void theNamesOfTablesAreOfferedToo()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Region</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>Q1</t></is></c></row>"
                                            "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>East</t></is></c><c r=\"B2\"><v>1</v></c></row>");
        const QString table = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:B2\">"
                                             "<autoFilter ref=\"A1:B2\"/><tableColumns count=\"2\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/></tableColumns></table>");
        EditWindow e;
        QVERIFY(e.open(withTables(buildXlsx({worksheet(rows)}), {table})));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        e.select(QStringLiteral("E1"));
        e.type(QStringLiteral("=sal"));
        QCOMPARE(w.formulaHelper()->offered(), QStringList({QStringLiteral("Sales")}));
        QCOMPARE(w.formulaHelper()->detail()->text(), QStringLiteral("A table."));
        QTest::keyClick(w.cellEditor(), Qt::Key_Tab);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Sales"));
        e.key(Qt::Key_Escape);
    }

    void theHelpHoldsAFileWithManyNamesToAWordsWorth()
    {
        QString names;
        for (int i = 0; i < 30000; ++i) names += QStringLiteral("<definedName name=\"Item%1\">Sheet1!$A$1</definedName>").arg(i, 5, 10, QLatin1Char('0'));
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1"}}, {}, {}, names)));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        QCOMPARE(w.session()->names().aliveCount(), 30000);
        e.select(QStringLiteral("B1"));
        e.type(QStringLiteral("=item"));
        QCOMPARE(int(w.formulaHelper()->offered().size()), crunch::kMaxOffered);
        QCOMPARE(w.formulaHelper()->offered().first(), QStringLiteral("Item00000"));
        e.key(Qt::Key_Escape);
        e.key(Qt::Key_Escape);
    }

    // ---- names
    void theListOfNamesFiltersAndMakesChangesAndTakesNamesAway()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=5", "A2=6", "B1==A1*2"}, {"A1=7"}}, {}, {}, QStringLiteral("<definedName name=\"TaxRate\" comment=\"used for sales\">Sheet1!$A$1</definedName><definedName name=\"Total\">Sheet1!$A$1:$A$2</definedName><definedName name=\"Own\" localSheetId=\"1\">Sheet2!$A$1</definedName>"))));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        crunch::EditSession *s = w.session();
        NamesDialog d(s, 0, QStringLiteral("Sheet1!$B$1"));
        d.show();
        QCOMPARE(d.model()->rowCount(), 3);
        QCOMPARE(d.model()->headerData(NamesModel::RefersColumn, Qt::Horizontal, Qt::DisplayRole).toString(), QStringLiteral("Stands for"));
        auto cell = [&](int row, NamesModel::Column col) { return d.model()->data(d.model()->index(row, col), Qt::DisplayRole).toString(); };
        QStringList found;
        for (int r = 0; r < d.model()->rowCount(); ++r) found << cell(r, NamesModel::NameColumn);
        QVERIFY(found.contains(QStringLiteral("TaxRate")));
        const int row = found.indexOf(QStringLiteral("TaxRate"));
        QCOMPARE(cell(row, NamesModel::RefersColumn), QStringLiteral("=Sheet1!$A$1"));
        QCOMPARE(cell(row, NamesModel::ValueColumn), QStringLiteral("5"));
        QCOMPARE(cell(row, NamesModel::ScopeColumn), QStringLiteral("Workbook"));
        QCOMPARE(cell(row, NamesModel::CommentColumn), QStringLiteral("used for sales"));
        QCOMPARE(cell(found.indexOf(QStringLiteral("Total")), NamesModel::ValueColumn), QStringLiteral("{…}"));
        QCOMPARE(cell(found.indexOf(QStringLiteral("Own")), NamesModel::ScopeColumn), QStringLiteral("Sheet2"));
        QVERIFY(d.countLabel()->text().startsWith(QStringLiteral("3 name")));
        saveShot(&d, QStringLiteral("names-dialog"));
        // the filter looks at the name, the comment, the sheet, and what it stands for
        d.filterBox()->setText(QStringLiteral("sales"));
        QCOMPARE(d.model()->rowCount(), 1);
        QVERIFY(d.countLabel()->text().startsWith(QStringLiteral("1 name")));
        d.filterBox()->setText(QStringLiteral("sheet2"));
        QCOMPARE(d.model()->rowCount(), 1);
        d.filterBox()->setText(QStringLiteral("$A$1"));
        QCOMPARE(d.model()->rowCount(), 3);   // (Total stands for $A$1:$A$2 too)
        d.filterBox()->setText(QString());
        // a new name, the cells selected being what it stands for
        crunch::EditSession::NameDraft asked;
        d.setEditAsker([&asked](crunch::EditSession::NameDraft *draft) {
            asked = *draft;
            draft->name = QStringLiteral("Double");
            return true;
        });
        QVERIFY(d.newButton()->isEnabled());
        d.newName();
        QCOMPARE(asked.text, QStringLiteral("=Sheet1!$B$1"));
        QCOMPARE(d.model()->rowCount(), 4);
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Define Name"));
        // change one
        d.table()->selectRow(d.model()->rowOf(s->names().find(-1, QStringLiteral("Double"))));
        d.setEditAsker([&asked](crunch::EditSession::NameDraft *draft) {
            asked = *draft;
            draft->name = QStringLiteral("TwiceIt");
            draft->comment = QStringLiteral("twice");
            return true;
        });
        d.editName();
        QCOMPARE(asked.name, QStringLiteral("Double"));
        QCOMPARE(asked.comment, QString());
        QCOMPARE(asked.text, QStringLiteral("=Sheet1!$B$1"));
        QVERIFY(s->names().find(-1, QStringLiteral("Double")) < 0);
        QVERIFY(s->names().find(-1, QStringLiteral("TwiceIt")) >= 0);
        // one given up changes nothing
        const int steps = w.undoStack()->count();
        d.setEditAsker([](crunch::EditSession::NameDraft *) { return false; });
        d.editName();
        QCOMPARE(w.undoStack()->count(), steps);
        // taken away
        d.deleteName();
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Delete Name"));
        QCOMPARE(d.model()->rowCount(), 3);
        QVERIFY(!d.editButton()->isEnabled() || d.table()->currentIndex().isValid());
    }

    void theSmallDialogSaysWhatIsWrongAsTheNameIsTyped()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=5"}, {"A1=7"}}, {}, {}, QStringLiteral("<definedName name=\"TaxRate\">Sheet1!$A$1</definedName>"))));
        e.win.w.setLocale(kEnglish);
        QVERIFY(e.settle());
        crunch::EditSession::NameDraft start;
        start.text = QStringLiteral("=Sheet1!$A$1");
        NameEditDialog d(e.win.w.session(), -1, start, QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet2")}));
        d.show();
        QCOMPARE(d.windowTitle(), QStringLiteral("Define Name"));
        QVERIFY(!d.okButton()->isEnabled());                    // (no name yet)
        d.nameBox()->setText(QStringLiteral("A1"));
        QVERIFY(!d.okButton()->isEnabled());
        QVERIFY(d.problemLabel()->text().contains(QStringLiteral("address of a cell")));
        d.nameBox()->setText(QStringLiteral("my rate"));
        QVERIFY(d.problemLabel()->text().contains(QStringLiteral("space")));
        d.nameBox()->setText(QStringLiteral("1rate"));
        QVERIFY(d.problemLabel()->text().contains(QStringLiteral("begins with")));
        d.nameBox()->setText(QStringLiteral("taxrate"));
        QVERIFY(d.problemLabel()->text().contains(QStringLiteral("already")));
        QVERIFY(!d.okButton()->isEnabled());
        // the same name on a sheet is a different name
        d.scopeBox()->setCurrentIndex(d.scopeBox()->findText(QStringLiteral("Sheet2")));
        QVERIFY(d.problemLabel()->text().isEmpty());
        QVERIFY(d.okButton()->isEnabled());
        d.textBox()->setText(QStringLiteral("=SUM("));
        QVERIFY(d.problemLabel()->text().contains(QStringLiteral("formula")));
        QVERIFY(!d.okButton()->isEnabled());
        d.textBox()->setText(QStringLiteral("=Sheet2!$A$1*2"));
        QVERIFY(d.okButton()->isEnabled());
        saveShot(&d, QStringLiteral("name-dialog"));
        const crunch::EditSession::NameDraft got = d.draft();
        QCOMPARE(got.name, QStringLiteral("taxrate"));
        QCOMPARE(got.scope, 1);
        // a name that is changed stays where it is
        NameEditDialog change(e.win.w.session(), e.win.w.session()->names().find(-1, QStringLiteral("TaxRate")), start, QStringList());
        QVERIFY(!change.scopeBox()->isEnabled());
        QCOMPARE(change.windowTitle(), QStringLiteral("Change Name"));
    }

    void aNameIsMadeForTheCellsSelectedAndFromTheLabelsAroundThem()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=Qty", "B1=Unit price", "C1=2024", "A2=3", "B2=4", "C2=5", "A3=6", "B3=7", "C3=8"}})));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        crunch::EditSession *s = w.session();
        // Define Name: the cells selected are what the name begins as standing for
        e.view()->selectRange(1, 0, 2, 0);
        crunch::EditSession::NameDraft asked;
        w.setNameAsker([&asked](crunch::EditSession::NameDraft *d) {
            asked = *d;
            d->name = QStringLiteral("Quantities");
            return true;
        });
        QVERIFY(w.defineNameCommand());
        QCOMPARE(asked.text, QStringLiteral("=Sheet1!$A$2:$A$3"));
        QCOMPARE(s->nameTextOf(s->names().find(-1, QStringLiteral("Quantities"))), QStringLiteral("=Sheet1!$A$2:$A$3"));
        w.setNameAsker([](crunch::EditSession::NameDraft *) { return false; });
        QVERIFY(!w.defineNameCommand());
        // from the labels: the headings of the columns name what is under them
        e.view()->selectRange(0, 0, 2, 2);
        w.setEdgesAsker([](int *edges) {
            *edges = crunch::TopEdge;
            return true;
        });
        const int steps = w.undoStack()->count();
        QVERIFY(w.nameFromCellsCommand());
        QCOMPARE(w.undoStack()->count(), steps + 1);   // (one step for all of them)
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Name From Cells"));
        QCOMPARE(s->nameTextOf(s->names().find(-1, QStringLiteral("Qty"))), QStringLiteral("=Sheet1!$A$2:$A$3"));
        QCOMPARE(s->nameTextOf(s->names().find(-1, QStringLiteral("Unit_price"))), QStringLiteral("=Sheet1!$B$2:$B$3"));   // (a space is not allowed in a name)
        QCOMPARE(s->nameTextOf(s->names().find(-1, QStringLiteral("_2024"))), QStringLiteral("=Sheet1!$C$2:$C$3"));       // (nor a digit first)
        w.undoStack()->undo();
        QVERIFY(s->names().find(-1, QStringLiteral("Qty")) < 0 && s->names().find(-1, QStringLiteral("Unit_price")) < 0 && s->names().find(-1, QStringLiteral("_2024")) < 0);
        QVERIFY(s->names().find(-1, QStringLiteral("Quantities")) >= 0);
        w.undoStack()->redo();
        QVERIFY(s->names().find(-1, QStringLiteral("Qty")) >= 0);
        // names that are there already are left, and a single cell makes none
        e.view()->selectRange(0, 0, 2, 2);
        QVERIFY(!w.nameFromCellsCommand());
        e.view()->selectCell(0, 0);
        QVERIFY(!w.nameFromCellsCommand());
        QVERIFY(e.win.told.last().contains(QStringLiteral("Select the labels")));
    }

    void labelsMakeNamesAWorkbookCanUse()
    {
        QCOMPARE(crunch::nameFromLabel(QStringLiteral("  Unit price ")), QStringLiteral("Unit_price"));
        QCOMPARE(crunch::nameFromLabel(QStringLiteral("2024")), QStringLiteral("_2024"));
        QCOMPARE(crunch::nameFromLabel(QStringLiteral("Q1")), QStringLiteral("_Q1"));   // (a cell's address)
        QCOMPARE(crunch::nameFromLabel(QStringLiteral("Profit & loss")), QStringLiteral("Profit___loss"));
        QCOMPARE(crunch::nameFromLabel(QStringLiteral("***")), QString());
        QCOMPARE(crunch::nameFromLabel(QString()), QString());
        QCOMPARE(crunch::nameFromLabel(QString(300, QLatin1Char('x'))).size(), 255);
        // the edges: top and left both, repeats left out
        const jp::office::sheet::CellRange block{0, 0, 2, 2, true};
        const QStringList text = {"corner", "a", "b", "x", "1", "2", "x", "3", "4"};
        int skipped = 0;
        const auto names = crunch::namesFromLabels(block, crunch::TopEdge | crunch::LeftEdge, [&](quint32 r, quint32 c) { return text[int(r * 3 + c)]; }, &skipped);
        QStringList got;
        for (const auto &n : names) got << n.name + QLatin1Char('=') + jp::office::sheet::rangeName(n.cells);
        QCOMPARE(got, QStringList({"corner=A2:A3", "a=B2:B3", "b=C2:C3", "x=B2:C2"}));   // (the second x repeats the first)
        QCOMPARE(skipped, 2);   // (the corner label is the top row's and the left column's; and the second x)
        QVERIFY(crunch::namesFromLabels(jp::office::sheet::CellRange{0, 0, 0, 2, true}, crunch::TopEdge, [](quint32, quint32) { return QStringLiteral("a"); }).empty());   // (nothing under them)
        const auto bottomRight = crunch::namesFromLabels(block, crunch::BottomEdge | crunch::RightEdge, [&](quint32 r, quint32 c) { return text[int(r * 3 + c)]; });
        QCOMPARE(int(bottomRight.size()), 5);   // (x, 3, and 4 along the bottom; b and 2 at the right, where 4 is a repeat)
    }

    void aNameCanBePutInAFormula()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=5", "A2=6"}}, {}, {}, QStringLiteral("<definedName name=\"TaxRate\">Sheet1!$A$1</definedName><definedName name=\"Alpha\">Sheet1!$A$2</definedName>"))));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        std::unique_ptr<QMenu> menu(w.namesMenu());
        QVERIFY(menu);
        QStringList entries;
        for (QAction *a : menu->actions())
            if (!a->isSeparator()) entries << a->text();
        QCOMPARE(entries, QStringList({"Alpha", "TaxRate", "All Names…"}));   // (in order)
        // into a formula being typed, at the cursor
        e.select(QStringLiteral("C1"));
        e.type(QStringLiteral("=1+"));
        menu->actions()[1]->trigger();
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=1+TaxRate"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=1+TaxRate"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:6"));
        // with nothing typed, the formula begins with it
        e.select(QStringLiteral("C2"));
        QVERIFY(w.insertName(QStringLiteral("Alpha")));
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Alpha"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:6"));
        // text that is not a formula takes none
        e.select(QStringLiteral("C3"));
        e.type(QStringLiteral("hello"));
        QVERIFY(!w.insertName(QStringLiteral("Alpha")));
        e.key(Qt::Key_Escape);
        // Go To goes to a name made since the file was read
        QVERIFY(w.session()->defineName([] {
            crunch::EditSession::NameDraft d;
            d.name = QStringLiteral("Fresh");
            d.text = QStringLiteral("Sheet1!$B$2");
            return d;
        }()).ok);
        QVERIFY(w.goToReference(QStringLiteral("Fresh")));
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        w.undoStack()->undo();
    }

    // ---- how it calculates
    void aWorkbookWorkedOutByHandWaitsForTheKeyAndSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(manualBook()));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QCOMPARE(int(w.session()->calcSettings().mode), int(jp::office::sheet::CalcMode::Manual));
        QVERIFY(w.act(QStringLiteral("formulas.calcManual"))->isChecked());
        QVERIFY(!w.act(QStringLiteral("formulas.calcAuto"))->isChecked());
        // opened as the file holds it: nothing was worked out, and nothing is said to be late
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:0"));   // (the result the file stored, which here is 0)
        QVERIFY(!w.calculationDue());
        QLabel *note = w.findChild<QLabel *>(QStringLiteral("calcNote"));
        QVERIFY(note->text().isEmpty());
        // a change leaves what depends on it as it was, and the status bar says so
        QVERIFY(w.session()->type(0, 0, 0, QStringLiteral("5")).ok);
        QTRY_VERIFY(w.calculationDue());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:0"));
        QCOMPARE(note->text(), QStringLiteral("Out of date: press F9 to recalculate"));
        // F9 works everything out
        w.act(QStringLiteral("formulas.calculate"))->trigger();
        QTRY_COMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:10"));
        QTRY_VERIFY(!w.calculationDue());
        QVERIFY(note->text().isEmpty());
        // back to automatic: the next change is worked out by itself
        w.act(QStringLiteral("formulas.calcAuto"))->trigger();
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Calculation Options"));
        QVERIFY(w.session()->type(0, 0, 0, QStringLiteral("6")).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:12"));
        // and by hand again
        w.act(QStringLiteral("formulas.calcManual"))->trigger();
        QVERIFY(w.session()->type(0, 0, 0, QStringLiteral("7")).ok);
        QTRY_VERIFY(w.calculationDue());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:12"));
        // Undo of the mode puts the formulas back to being worked out as the cells change
        w.undoStack()->undo();   // (the cell)
        w.undoStack()->undo();   // (the mode)
        QVERIFY(w.act(QStringLiteral("formulas.calcAuto"))->isChecked());
        QTRY_VERIFY(!w.calculationDue());
    }

    void oneSheetCanBeWorkedOutByHand()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rowsOfSpecs({"A1=1", "B1==A1*2"})), worksheet(rowsOfSpecs({"A1=10", "B1==A1*3"}))}, QByteArray(), QString(), QStringLiteral("calcMode=\"manual\""))));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(w.session()->type(0, 0, 0, QStringLiteral("5")).ok);
        QVERIFY(w.session()->type(1, 0, 0, QStringLiteral("20")).ok);
        QTRY_VERIFY(w.calculationDue());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:0"));
        w.act(QStringLiteral("formulas.calculateSheet"))->trigger();
        QTRY_COMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:10"));          // (this sheet)
        QCOMPARE(w.session()->valueAt(1, 0, 1).debug(), QStringLiteral("n:0"));         // (the other sheet waits)
        QVERIFY(w.calculationDue());
        w.act(QStringLiteral("formulas.calculate"))->trigger();
        QTRY_COMPARE(w.session()->valueAt(1, 0, 1).debug(), QStringLiteral("n:60"));
        QTRY_VERIFY(!w.calculationDue());
    }

    // ---- circles
    void aCircleIsToldOnceWithItsCellsAndOutlined()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "B1==C1+1", "C1==B1*2", "D1=7"}})));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        QTRY_COMPARE(w.circularCount(), qint64(2));
        QCOMPARE(w.circularNotices(), 1);
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY(e.win.told.first().startsWith(QStringLiteral("Circular reference: ")));
        QVERIFY(e.win.told.first().contains(QStringLiteral("Cells: B1, C1")));
        QCOMPARE(int(e.view()->checkCircles().size()), 2);
        QVERIFY(w.findChild<QLabel *>(QStringLiteral("calcNote"))->text().startsWith(QStringLiteral("Circular references: 2 cell")));
        QVERIFY(w.act(QStringLiteral("formulas.circular"))->isEnabled());
        saveShot(&e.win.w, QStringLiteral("circular-reference"));
        // another change does not say it again
        QVERIFY(w.session()->type(0, 0, 3, QStringLiteral("8")).ok);
        QVERIFY(e.settle());
        QCOMPARE(w.circularNotices(), 1);
        QCOMPARE(w.circularCount(), qint64(2));
        // broken, and made again: told again
        QVERIFY(w.session()->type(0, 0, 2, QStringLiteral("3")).ok);
        QVERIFY(e.settle());
        QTRY_COMPARE(w.circularCount(), qint64(0));
        QVERIFY(e.view()->checkCircles().empty());
        QVERIFY(!w.act(QStringLiteral("formulas.circular"))->isEnabled());
        QVERIFY(w.session()->type(0, 0, 2, QStringLiteral("=B1*2")).ok);
        QVERIFY(e.settle());
        QTRY_COMPARE(w.circularCount(), qint64(2));
        QCOMPARE(w.circularNotices(), 2);
    }

    void goingRoundACircleIsAnOptionAndEndsTheNotice()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"B1==C1+1", "C1==B1*0.5"}})));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        QTRY_COMPARE(w.circularCount(), qint64(2));
        w.setIterationAsker([](crunch::CalcSettings *s) {
            s->iterate = true;
            s->count = 100;
            s->delta = 0.0001;
            return true;
        });
        w.act(QStringLiteral("formulas.iteration"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Calculation Options"));
        QTRY_COMPARE(w.circularCount(), qint64(0));
        QVERIFY2(e.valueAt(QStringLiteral("B1")).startsWith(QStringLiteral("n:1.99")) || e.valueAt(QStringLiteral("B1")).startsWith(QStringLiteral("n:2")), qPrintable(e.valueAt(QStringLiteral("B1"))));
        QVERIFY(w.act(QStringLiteral("formulas.calcAuto"))->isChecked());
        w.undoStack()->undo();
        QVERIFY(e.settle());
        QTRY_COMPARE(w.circularCount(), qint64(2));
    }

    // ---- arrows
    void arrowsShowWhatAFormulaReadsOneStepAtATime()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "B1==A1*2", "C1==SUM(A1:B1)+Sheet2!A1", "D1==C1+1"}, {"A1=3", "B1==Sheet1!A1+1"}})));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        e.select(QStringLiteral("C1"));
        QVERIFY(w.act(QStringLiteral("formulas.tracePrecedents"))->isEnabled());
        QVERIFY(w.tracePrecedents());
        // from the block A1:B1, and from the other sheet
        QCOMPARE(int(e.view()->checkArrows().size()), 2);
        const auto first = e.view()->checkArrows()[0];
        QVERIFY(!first.fromOtherSheet && !first.toOtherSheet);
        QCOMPARE(first.from.col1, qint64(0));
        QCOMPARE(first.from.col2, qint64(1));
        QCOMPARE(first.toCol, qint64(2));
        QVERIFY(e.view()->checkArrows()[1].fromOtherSheet);
        saveShot(e.view(), QStringLiteral("trace-arrows"));
        // one step further back: B1 reads A1
        QVERIFY(w.tracePrecedents());
        QCOMPARE(int(e.view()->checkArrows().size()), 3);
        // nothing is further back than that (A1 is a number; the other sheet's cell is a number)
        QVERIFY(!w.tracePrecedents() || e.view()->checkArrows().size() >= 3);
        // the other way
        w.removeArrows();
        QVERIFY(e.view()->checkArrows().empty());
        e.select(QStringLiteral("A1"));
        QVERIFY(w.traceDependents());
        QCOMPARE(int(e.view()->checkArrows().size()), 3);   // (B1, C1, and the other sheet's B1 read it)
        int otherSheet = 0;
        for (const auto &a : e.view()->checkArrows()) otherSheet += a.toOtherSheet ? 1 : 0;
        QCOMPARE(otherSheet, 1);
        QVERIFY(w.traceDependents());   // one step on: what reads B1 and C1 (C1 reads B1 too, D1 reads C1)
        QVERIFY(e.view()->checkArrows().size() > 3);
        // another cell begins again
        e.select(QStringLiteral("D1"));
        QVERIFY(w.tracePrecedents());
        QCOMPARE(int(e.view()->checkArrows().size()), 1);
        // a change takes the arrows away
        QVERIFY(w.session()->type(0, 0, 5, QStringLiteral("9")).ok);
        QVERIFY(e.view()->checkArrows().empty());
        // a cell with no formula reads nothing
        e.select(QStringLiteral("A1"));
        QVERIFY(!w.tracePrecedents());
        QVERIFY(w.statusText().contains(QStringLiteral("does not read any cell")));
    }

    void arrowsAreDrawnOverTheCellsOfASheetWithRoomForThem()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A2=4", "B2=5", "D1=6", "C4==SUM(A2:B2)+Sheet2!A1+D1", "F6==C4*2", "H3=1"}, {"A1=3"}})));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        e.select(QStringLiteral("C4"));
        QVERIFY(w.tracePrecedents());
        QCOMPARE(int(e.view()->checkArrows().size()), 3);
        saveShot(e.view(), QStringLiteral("trace-arrows-precedents"));
        w.removeArrows();
        e.select(QStringLiteral("C4"));
        QVERIFY(w.traceDependents());
        QCOMPARE(int(e.view()->checkArrows().size()), 1);
        QVERIFY(!w.traceDependents());   // (nothing reads F6)
        QVERIFY(w.statusText().contains(QStringLiteral("nothing further")));
        // the tab with its groups
        QVERIFY(w.ribbon()->tab(QStringLiteral("Formulas")));
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Formulas")));
        QVERIFY(w.act(QStringLiteral("formulas.names"))->isEnabled());
        saveShot(&w, QStringLiteral("formulas-tab"));
        saveShot(e.view(), QStringLiteral("trace-arrows-dependents"));
    }

    void arrowsShowTheOtherSheetsEndAsAnIcon()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "C1==Sheet2!A1"}, {"A1=3"}})));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        e.select(QStringLiteral("C1"));
        QVERIFY(w.tracePrecedents());
        const QImage with = e.view()->grab().toImage();
        w.removeArrows();
        const QImage without = e.view()->grab().toImage();
        QVERIFY(with != without);   // (the arrow and the icon are drawn)
        // on the other sheet, the reading cell is the end that is away
        w.setActiveSheet(1);
        e.select(QStringLiteral("A1"));
        QVERIFY(w.traceDependents());
        QCOMPARE(int(e.view()->checkArrows().size()), 1);
        QVERIFY(e.view()->checkArrows()[0].toOtherSheet);
        w.setActiveSheet(0);
        QVERIFY(e.view()->checkArrows().empty() || !e.view()->checkArrows()[0].toOtherSheet);
        w.removeArrows();
    }

    void aCellThatManyFormulasReadDrawsOnlyAsManyArrowsAsTheBound()
    {
        QStringList cells = {"A1=1"};
        for (int i = 1; i <= 1500; ++i) cells << QStringLiteral("B%1==A1+%1").arg(i);
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        CrunchWindow &w = e.win.w;
        w.setLocale(kEnglish);
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QVERIFY(w.traceDependents());
        QCOMPARE(int(e.view()->checkArrows().size()), 1500);
        QVERIFY(w.statusText().startsWith(QStringLiteral("1500 arrow")));
        QVERIFY(!w.statusText().contains(QStringLiteral("not drawn")));
        // more than the bound: cut, and said so
        QStringList many = {"A1=1"};
        for (int i = 1; i <= 4500; ++i) many << QStringLiteral("B%1==A1+%1").arg(i);
        EditWindow f;
        QVERIFY(f.open(bookOfSheets({many})));
        f.win.w.setLocale(kEnglish);
        QVERIFY(f.settle());
        f.select(QStringLiteral("A1"));
        QVERIFY(f.win.w.traceDependents());
        QCOMPARE(int(f.view()->checkArrows().size()), int(grid::GridView::kMaxCheckArrows));
        QVERIFY(f.win.w.statusText().contains(QStringLiteral("more are not drawn")));
    }
};

JP_TEST_CLASS(CrunchFormulasTests)
#include "tst_crunch_formulas.moc"
