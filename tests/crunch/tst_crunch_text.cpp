// Tests of opening a text file of values in the window: the question of how it is read (its preview, the separators, the quote, the code page, the row to begin at, and what each column is),
// the choices going to the reader, a cancelled question opening nothing, and the dialog's own preview staying small for a large file. Every file is built here.
#include "app/anchordialogs.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/textimportdialog.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"
#include "../localepin.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QSettings>
#include <QSpinBox>
#include <QTableWidget>
#include <QTemporaryDir>

using namespace jp;
using namespace crunchtest;

class CrunchTextTests : public QObject {
    Q_OBJECT

    QLocale m_oldLocale;

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
        m_oldLocale = QLocale();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    void cleanupTestCase() { QLocale::setDefault(m_oldLocale); }

    void aWindowThatIsNotToAskOpensTheFileAsItFindsIt()
    {
        EditWindow e;
        bool asked = false;
        e.win.w.anchorHooks().textImportAsker = [&asked](TextImportRequest *) {
            asked = true;
            return true;
        };
        QVERIFY(e.open("a;b\n1;2\n", QStringLiteral("data.csv")));
        QVERIFY(!asked);
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("2"));
    }

    void aTextFileAsksHowItIsReadAndOpensAsTheAnswerSays()
    {
        EditWindow e;
        e.win.w.setPromptForText(true);
        TextImportRequest seen;
        e.win.w.anchorHooks().textImportAsker = [&seen](TextImportRequest *r) {
            seen = *r;
            r->choices.separators = QStringLiteral("|");
            r->choices.firstRow = 1;
            r->choices.columns = {jp::office::sheet::CsvColumn{jp::office::sheet::CsvColumn::Kind::Text, QString()}};
            return true;
        };
        QVERIFY(e.open("junk line\n007|3.50|x\n008|4|y\n", QStringLiteral("parts.csv")));
        QVERIFY(seen.path.endsWith(QStringLiteral("parts.csv")));
        QVERIFY(seen.head.startsWith("junk line"));
        QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("007"));
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("4"));
        QCOMPARE(e.shownAt(QStringLiteral("C1")), QStringLiteral("x"));
    }

    void aTextFileOnlyAsksOtherKindsOfFilesDoNot()
    {
        EditWindow e;
        e.win.w.setPromptForText(true);
        bool asked = false;
        e.win.w.anchorHooks().textImportAsker = [&asked](TextImportRequest *) {
            asked = true;
            return true;
        };
        QVERIFY(e.open(editBook()));   // (a workbook)
        QVERIFY(!asked);
    }

    void cancellingTheQuestionOpensNothing()
    {
        Window w;
        w.w.setPromptForText(true);
        w.w.anchorHooks().textImportAsker = [](TextImportRequest *) { return false; };
        Folder folder;
        const QString path = folder.write(QStringLiteral("data.csv"), "a,b\n1,2\n");
        QVERIFY(!w.w.openFile(path));
        QVERIFY(!w.w.hasWorkbook());
        QVERIFY(!w.w.isLoading());
    }

    // Fixed columns are chosen on the first lines (a click between two characters starts or ends a column, and the places can be typed), and the marks of numbers are chosen too: the preview
    // follows both.
    void theDialogChoosesFixedColumnsAndTheMarksOfNumbers()
    {
        TextImportRequest req;
        req.path = QStringLiteral("data.txt");
        req.head = "Name  Qty   Price\nBob   12    1.234,50\nAl    7     9,5-\n";
        req.size = req.head.size();
        req.locale = QLocale::c();
        TextImportDialog dialog(req);
        auto *table = dialog.findChild<QTableWidget *>(QStringLiteral("textImportPreview"));
        auto *ruler = dialog.findChild<QPlainTextEdit *>(QStringLiteral("textImportRuler"));
        auto *breaks = dialog.findChild<QLineEdit *>(QStringLiteral("textImportBreaks"));
        auto *fixed = dialog.findChild<QCheckBox *>(QStringLiteral("textImportFixed"));
        QVERIFY(table && ruler && breaks && fixed);
        QVERIFY(!ruler->isEnabled());
        fixed->setChecked(true);
        QVERIFY(ruler->isEnabled());
        QVERIFY(dialog.choices().fixedWidth);
        QVERIFY(dialog.choices().breaks.isEmpty());
        QCOMPARE(dialog.previewColumns(), 1);
        // the places typed (counted from 1: the column begins at the 7th character)
        breaks->setText(QStringLiteral("7, 13"));
        QCOMPARE(dialog.choices().breaks, (QVector<int>{6, 12}));
        QTRY_COMPARE(dialog.previewColumns(), 3);
        QCOMPARE(table->item(2, 0)->text(), QStringLiteral("Bob"));
        QCOMPARE(table->item(2, 2)->text(), QStringLiteral("1.234,50"));
        // the bars in the first lines mark where the columns begin
        QVERIFY(ruler->toPlainText().startsWith(QStringLiteral("Name  \u2502Qty   \u2502Price")));
        // a click between the "P" and the "r" of "Price" in the first line starts a column there (place 14), and a second click takes it away
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));
        QTextCursor at(ruler->document()->firstBlock());
        at.setPosition(15);
        const QRect caret = ruler->cursorRect(at);
        QTest::mouseClick(ruler->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(caret.left(), caret.center().y()));
        QCOMPARE(breaks->text(), QStringLiteral("7, 13, 14"));
        QTRY_COMPARE(dialog.previewColumns(), 4);
        QTest::mouseClick(ruler->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(caret.left(), caret.center().y()));
        QCOMPARE(breaks->text(), QStringLiteral("7, 13"));
        // the marks of numbers
        auto *decimal = dialog.findChild<QComboBox *>(QStringLiteral("textImportDecimal"));
        auto *thousands = dialog.findChild<QComboBox *>(QStringLiteral("textImportThousands"));
        auto *minus = dialog.findChild<QCheckBox *>(QStringLiteral("textImportTrailingMinus"));
        QVERIFY(decimal && thousands && minus);
        decimal->setCurrentIndex(decimal->findText(QStringLiteral("Comma (,)")));
        thousands->setCurrentIndex(thousands->findText(QStringLiteral("Point (.)")));
        minus->setChecked(true);
        const jp::office::sheet::CsvImport c = dialog.choices();
        saveShot(&dialog, QStringLiteral("text-import-fixed"));
        QCOMPARE(c.decimalMark, QChar(','));
        QCOMPARE(c.thousandsMark, QChar('.'));
        QVERIFY(c.trailingMinus);
    }

    void theDialogShowsAPreviewThatFollowsEveryChoice()
    {
        TextImportRequest req;
        req.path = QStringLiteral("data.csv");
        req.head = "id;date;name\n1;05/03/2024;ann\n2;15/12/2023;bob\n";
        req.size = req.head.size();
        req.locale = QLocale::c();
        TextImportDialog dialog(req);
        auto *table = dialog.findChild<QTableWidget *>(QStringLiteral("textImportPreview"));
        QVERIFY(table);
        // the file's own separator is found
        QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("sepSemicolon"))->isChecked());
        QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("sepComma"))->isChecked());
        QCOMPARE(dialog.previewColumns(), 3);
        QCOMPARE(dialog.previewRows(), 3);
        QCOMPARE(table->item(2, 1)->text(), QStringLiteral("05/03/2024"));
        // the row to begin at
        dialog.findChild<QSpinBox *>(QStringLiteral("textImportStart"))->setValue(2);
        QCOMPARE(dialog.previewRows(), 2);
        QCOMPARE(table->item(1, 0)->text(), QStringLiteral("1"));
        // the column of dates read as day, month, year
        auto *kind = dialog.findChild<QComboBox *>(QStringLiteral("columnKind1"));
        QVERIFY(kind);
        kind->setCurrentIndex(kind->findText(QStringLiteral("Date: day, month, year")));
        QTRY_COMPARE(table->item(1, 1)->text(), QStringLiteral("2024-03-05"));
        QCOMPARE(dialog.choices().columns.value(1).dateOrder, QStringLiteral("DMY"));
        // the separators: with a comma too, nothing changes here; with no separator, each line is one field
        for (const char *name : {"sepSemicolon"}) dialog.findChild<QCheckBox *>(QLatin1String(name))->setChecked(false);
        QTRY_COMPARE(dialog.previewColumns(), 1);
        QCOMPARE(dialog.choices().firstRow, 1);
        // and another separator typed
        dialog.findChild<QLineEdit *>(QStringLiteral("sepOther"))->setText(QStringLiteral("/"));
        QTRY_COMPARE(dialog.previewColumns(), 3);
        // a column left out
        auto *kind0 = dialog.findChild<QComboBox *>(QStringLiteral("columnKind0"));
        kind0->setCurrentIndex(kind0->findText(QStringLiteral("Leave out")));
        QTRY_COMPARE(dialog.choices().columns.value(0).kind, jp::office::sheet::CsvColumn::Kind::Skip);
        // the code page
        auto *encoding = dialog.findChild<QComboBox *>(QStringLiteral("textImportEncoding"));
        QVERIFY(!dialog.choices().encodingSet);
        encoding->setCurrentIndex(encoding->findText(QStringLiteral("Cyrillic (1251)")));
        QVERIFY(dialog.choices().encodingSet);
        QCOMPARE(dialog.choices().codePage, 1251);
        // the ISO 8859 pages and the Macintosh page are among the choices
        for (const auto &page : {std::pair<const char *, int>{"Central European (ISO 8859-2)", 28592}, {"Cyrillic (ISO 8859-5)", 28595}, {"Greek (ISO 8859-7)", 28597},
                                 {"Turkish (ISO 8859-9)", 28599}, {"Western European (ISO 8859-15)", 28605}, {"Western European (ISO 8859-1)", 28591}, {"Mac Roman", 10000}}) {
            const int at = encoding->findText(QString::fromLatin1(page.first));
            QVERIFY2(at >= 0, page.first);
            encoding->setCurrentIndex(at);
            QCOMPARE(dialog.choices().codePage, page.second);
            QCOMPARE(dialog.choices().encoding, jp::office::sheet::CsvEncoding::Legacy);
        }
        // every control has a name a screen reader says
        for (QWidget *w : dialog.findChildren<QComboBox *>()) QVERIFY2(!w->accessibleName().isEmpty(), qPrintable(w->objectName()));
        QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("textImportStart"))->accessibleName().isEmpty());
    }

    void theDialogsPreviewStaysSmallForAHugeFile()
    {
        // 50,000 lines of 400 columns would be 80 MB: the dialog is given the first megabyte only, and shows 200 rows of 100 columns
        QByteArray head;
        while (head.size() < (1 << 20)) {
            QByteArray line;
            for (int c = 0; c < 400; ++c) line += QByteArray::number(c) + ",";
            head += line + "\n";
        }
        TextImportRequest req;
        req.path = QStringLiteral("huge.csv");
        req.head = head.left(int(jp::office::sheet::kCsvPreviewBytes));
        req.size = 80 << 20;
        req.locale = QLocale::c();
        TextImportDialog dialog(req);
        QCOMPARE(dialog.previewRows(), jp::office::sheet::kCsvPreviewRows);
        QCOMPARE(dialog.previewColumns(), jp::office::sheet::kCsvPreviewColumns);
        QVERIFY(!dialog.findChild<QLabel *>(QStringLiteral("textImportNote"))->text().isEmpty());
    }
};

JP_TEST_CLASS(CrunchTextTests)
#include "tst_crunch_text.moc"
