// Tests of opening a text file of values in the window: the question of how it is read (its preview, the separators, the quote, the code page, the row to begin at, and what each column is),
// the choices going to the reader, a cancelled question opening nothing, and the dialog's own preview staying small for a large file. Every file is built here.
#include "app/anchordialogs.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/textimportdialog.h"
#include "core/appinfo.h"
#include "core/clipdata.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"
#include "../localepin.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QSettings>
#include <QSpinBox>
#include <QHeaderView>
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

    // The command line says how a text file is read (--separator, --encoding, --decimal): no question, and what it says is used.
    void theCommandLineChoicesAreUnderstood()
    {
        using jp::office::sheet::CsvEncoding;
        jp::office::sheet::CsvImport how;
        QString why;
        QVERIFY(crunch::textImportFromOptions(QStringLiteral("semicolon"), QStringLiteral("Windows-1250"), QStringLiteral("comma"), &how, &why));
        QCOMPARE(how.separators, QStringLiteral(";"));
        QVERIFY(how.encodingSet);
        QVERIFY(how.encoding == CsvEncoding::Legacy);
        QCOMPARE(how.codePage, 1250);
        QCOMPARE(how.decimalMark, QChar(','));
        QVERIFY(crunch::textImportFromOptions(QStringLiteral("tab"), QStringLiteral("utf-16le"), QString(), &how, &why));
        QCOMPARE(how.separators, QStringLiteral("\t"));
        QVERIFY(how.encoding == CsvEncoding::Utf16Le);
        QVERIFY(crunch::textImportFromOptions(QStringLiteral("#"), QStringLiteral("iso-8859-15"), QStringLiteral("."), &how, &why));
        QCOMPARE(how.codePage, 28605);
        QVERIFY(crunch::textImportFromOptions(QString(), QString(), QString(), &how, &why));
        QVERIFY(how.separators.isEmpty() && !how.encodingSet);
        QVERIFY(!crunch::textImportFromOptions(QStringLiteral("two"), QString(), QString(), &how, &why));
        QVERIFY(why.contains(QStringLiteral("two")));
        QVERIFY(!crunch::textImportFromOptions(QString(), QStringLiteral("klingon"), QString(), &how, &why));
        QVERIFY(!crunch::textImportFromOptions(QString(), QString(), QStringLiteral("x"), &how, &why));
        // an empty separator, a line end, a character that is not one, and the separator being the decimal mark are refused
        QVERIFY(!crunch::textImportFromOptions(QStringLiteral(""), QString(), QString(), &how, &why));
        QVERIFY(!crunch::textImportFromOptions(QStringLiteral("\n"), QString(), QString(), &how, &why));
        QVERIFY(!crunch::textImportFromOptions(QStringLiteral("\r"), QString(), QString(), &how, &why));
        QVERIFY(!crunch::textImportFromOptions(QString(QChar(0)), QString(), QString(), &how, &why));
        QVERIFY(!crunch::textImportFromOptions(QStringLiteral(","), QString(), QStringLiteral("comma"), &how, &why));
        QVERIFY(why.contains(QStringLiteral("same")));
    }

    // The guess for pasted text looks at 100 lines and 64 KB, nothing more.
    void theGuessForPastedTextLooksAtTheFirstHundredLinesOnly()
    {
        jp::office::sheet::CsvImport c;
        QString text;
        for (int i = 0; i < 100; ++i) text += QStringLiteral("a,b,c\n");
        text += QStringLiteral("a,b,c,d\n");   // (line 101: not looked at)
        QVERIFY(crunch::guessDelimitedText(text, &c));
        QCOMPARE(c.separators, QStringLiteral(","));
        QVERIFY(!crunch::guessDelimitedText(QStringLiteral("a,b,c\n") + QStringLiteral("a,b,c,d\n"), &c));
        // one line past the bytes looked at, with no line end before it: no guess
        QVERIFY(!crunch::guessDelimitedText(QString(crunch::kGuessBytes + 10, QLatin1Char(',')), &c));
        // tabs, one line, and ordinary words are not guessed
        QVERIFY(!crunch::guessDelimitedText(QStringLiteral("a,b,c\td\ne,f,g\th\n"), &c));
        QVERIFY(!crunch::guessDelimitedText(QStringLiteral("a,b,c\n"), &c));
        QVERIFY(!crunch::guessDelimitedText(QStringLiteral("some words here\nmore words there\nand a third line\n"), &c));
    }

    // A paste is guessed only when it is clearly a table; what is asked for (Paste Text Import) takes two columns.
    void theGuessIsStrictForAPasteAndLooseWhenAskedFor()
    {
        jp::office::sheet::CsvImport c;
        for (const QString &not_table : {QStringLiteral("int x = 1;\nint y = 2;\n"), QStringLiteral("a,b,c,\nd,e,f,\n"), QStringLiteral("1,234\n5,678\n"), QStringLiteral("1,234,567\n8,901,234\n"),
                                         QStringLiteral("Hello, world\nBye, moon\n"), QStringLiteral("a;b;c;\nd;e;f;\n")})
            QVERIFY2(!crunch::guessDelimitedText(not_table, &c, true), qPrintable(not_table));
        QVERIFY(crunch::guessDelimitedText(QStringLiteral("Hello, world\nBye, moon\n"), &c, false));
        QVERIFY(crunch::guessDelimitedText(QStringLiteral("a,b,c\n1,2,3\n"), &c, true));
        // a line end in quotes is not the end of a line, and a blank line in quotes is kept: the cut at the byte limit is made outside quotes
        QString text = QStringLiteral("a,b,c\n\"x\n\ny\",2,3\n");
        QVERIFY(crunch::guessDelimitedText(text, &c, true));
        QString big;
        while (big.size() < crunch::kGuessBytes - 30) big += QStringLiteral("\"q\n\nq\",2,3\n");
        big += QStringLiteral("\"cut here\n\nstill in the quotes\",2,3\n") + QString(100, QLatin1Char('z')) + QStringLiteral(",1,2\n");
        QVERIFY(crunch::guessDelimitedText(big, &c, true));
    }

    void aFileOpenedWithTheCommandLineChoicesAsksNothingAndReadsAsTheyAre()
    {
        EditWindow e;
        e.win.w.setPromptForText(true);
        bool asked = false;
        e.win.w.anchorHooks().textImportAsker = [&asked](TextImportRequest *) {
            asked = true;
            return true;
        };
        jp::office::sheet::CsvImport how;
        QString why;
        QVERIFY(crunch::textImportFromOptions(QStringLiteral("|"), QStringLiteral("windows-1252"), QStringLiteral("comma"), &how, &why));
        e.win.w.setTextImport(std::make_shared<const jp::office::sheet::CsvImport>(how));
        QVERIFY(e.open(QByteArray("a|b;c\n1,5|caf\xe9\n"), QStringLiteral("data.csv")));
        QVERIFY(!asked);
        QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("b;c"));
        QCOMPARE(e.shownAt(QStringLiteral("A2")), QStringLiteral("1.5"));
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QString::fromUtf8("caf\xc3\xa9"));
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

    // The window reads cleanly: pasted text is named as such (not a blank name), the separator choices stand apart, and the
    // preview has room for several rows (Nov 16: "Comma[ ]Semicolon" touched, " (62 bytes)", one row of preview).
    void theDialogNamesPastedTextAndHasRoomForItsChoicesAndPreview()
    {
        TextImportRequest req;
        req.head = "name,qty,price\nBob,12,1.5\nAl,7,9.5\nCy,3,2\n";
        req.size = req.head.size();
        req.locale = QLocale::c();
        TextImportDialog dialog(req);
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));
        auto *name = dialog.findChild<QLabel *>(QStringLiteral("textImportName"));
        QVERIFY(name);
        QVERIFY(name->text().startsWith(QStringLiteral("Pasted text")));
        QStringList ids = {QStringLiteral("sepComma"), QStringLiteral("sepSemicolon"), QStringLiteral("sepTab"), QStringLiteral("sepSpace"), QStringLiteral("sepPipe")};
        for (int i = 0; i + 1 < ids.size(); ++i) {
            auto *a = dialog.findChild<QCheckBox *>(ids[i]), *b = dialog.findChild<QCheckBox *>(ids[i + 1]);
            QVERIFY(a && b);
            QVERIFY2(b->mapTo(&dialog, QPoint(0, 0)).x() - a->mapTo(&dialog, QPoint(a->width(), 0)).x() >= 10, qPrintable(ids[i]));
        }
        auto *table = dialog.findChild<QTableWidget *>(QStringLiteral("textImportPreview"));
        QVERIFY(table);
        QVERIFY(table->viewport()->height() >= 3 * table->verticalHeader()->defaultSectionSize());
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
        auto *readFormulas = dialog.findChild<QCheckBox *>(QStringLiteral("textImportReadFormulas"));
        QVERIFY(readFormulas);
        QVERIFY(!readFormulas->isChecked());   // (off unless the person asks)
        QVERIFY(!dialog.choices().readFormulas);
        readFormulas->setChecked(true);
        QVERIFY(dialog.choices().readFormulas);
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

    // ---- the kind of each column, guessed from the first rows where the person sees the preview

    static int kindIndex(TextImportDialog &dialog, int col)
    {
        auto *box = dialog.findChild<QComboBox *>(QStringLiteral("columnKind%1").arg(col));
        return box ? box->currentIndex() : -1;
    }
    static TextImportRequest guessing(const QByteArray &head, bool on = true)
    {
        TextImportRequest req;
        req.path = QStringLiteral("data.csv");
        req.head = head;
        req.size = head.size();
        req.locale = QLocale(QLocale::English, QLocale::UnitedStates);
        req.guessKinds = on;
        return req;
    }

    // The boxes above the columns start at the guess: 0 as typed (a number), 1 text, 3 and up dates and the like. A dialog that is not asked to guess leaves every column as typed, as before.
    void theDialogProposesEachColumnsKindAndTheBoxesShowIt()
    {
        const QByteArray file = "name,qty,born,zip,code\nAnn,3,1990-05-13,02134,1E5\nBob,12,1985-12-01,90210,7\n";
        TextImportDialog dialog(guessing(file));
        QCOMPARE(dialog.previewColumns(), 5);
        QCOMPARE(kindIndex(dialog, 0), 0);   // (words: as typed keeps them as text)
        QCOMPARE(kindIndex(dialog, 1), 0);
        QCOMPARE(kindIndex(dialog, 2), 4);   // (year, month, day)
        QCOMPARE(kindIndex(dialog, 3), 1);
        QCOMPARE(kindIndex(dialog, 4), 1);
        QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("textImportNames"))->isChecked());   // (text over values: the first row is names)
        const jp::office::sheet::CsvImport c = dialog.choices();
        QCOMPARE(c.columns.size(), 5);
        QCOMPARE(c.columns[2].dateOrder, QStringLiteral("YMD"));
        QCOMPARE(c.rest.kind, jp::office::sheet::CsvColumn::Kind::Text);
        QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("textImportNote"))->text().contains(QStringLiteral("guessed from its first 200 rows")));
        // not asked to guess: as typed, no box of names, no note
        TextImportDialog plain(guessing(file, false));
        for (int col = 0; col < 5; ++col) QCOMPARE(kindIndex(plain, col), 0);
        QVERIFY(!plain.findChild<QCheckBox *>(QStringLiteral("textImportNames")));
        QVERIFY(plain.findChild<QLabel *>(QStringLiteral("textImportNote"))->text().isEmpty());
        QCOMPARE(plain.choices().rest.kind, jp::office::sheet::CsvColumn::Kind::General);
        QVERIFY(plain.choices().columns.size() <= 5);
        for (const auto &k : plain.choices().columns) QCOMPARE(k.kind, jp::office::sheet::CsvColumn::Kind::General);
    }

    // Dates that fit two orders are kept as text, the note names the column, and the person picks the order from the box.
    void datesThatFitTwoOrdersAreTextUntilThePersonChoosesAnOrder()
    {
        TextImportDialog dialog(guessing("id,day\n1,05/06/2024\n2,01/02/2024\n"));
        QCOMPARE(kindIndex(dialog, 1), 1);
        const QString note = dialog.findChild<QLabel *>(QStringLiteral("textImportNote"))->text();
        QVERIFY2(note.contains(QStringLiteral("not for certain")) && note.contains(QStringLiteral(": B.")), qPrintable(note));
        dialog.findChild<QComboBox *>(QStringLiteral("columnKind1"))->setCurrentIndex(3);
        QTRY_COMPARE(dialog.choices().columns.value(1).dateOrder, QStringLiteral("DMY"));
        QTRY_VERIFY(!dialog.findChild<QLabel *>(QStringLiteral("textImportNote"))->text().contains(QStringLiteral("not for certain")));
        QCOMPARE(kindIndex(dialog, 1), 3);
        // a date that proves its order is proposed in it, with no question
        TextImportDialog proven(guessing("id,day\n1,05/13/2024\n2,01/02/2024\n"));
        QCOMPARE(kindIndex(proven, 1), 2);
    }

    // A column of version numbers is left as typed (main keeps it as text), the note offers the date choice, and the file comes in exactly as in a window that does not guess.
    void versionNumbersAreLeftAsTypedAndNeverBecomeDates()
    {
        const QByteArray file = "ver,n\n1.13.2,1\n1.2.3,2\n2.1.4,3\n";
        TextImportDialog dialog(guessing(file));
        QCOMPARE(kindIndex(dialog, 0), 0);
        const QString note = dialog.findChild<QLabel *>(QStringLiteral("textImportNote"))->text();
        QVERIFY2(note.contains(QStringLiteral("not for certain")) && note.contains(QStringLiteral(": A.")), qPrintable(note));
        // opened through the preview and opened where nobody is asked: the same cells
        QStringList asked, plain;
        for (const bool ask : {true, false}) {
            EditWindow e;
            e.win.w.setPromptForText(ask);
            e.win.w.anchorHooks().textImportAsker = [](TextImportRequest *r) {
                TextImportDialog d(*r);
                r->choices = d.choices();
                return true;
            };
            QVERIFY(e.open(file, QStringLiteral("g.csv")));
            for (const QString &ref : {QStringLiteral("A2"), QStringLiteral("A3"), QStringLiteral("A4")}) (ask ? asked : plain) << e.shownAt(ref);
        }
        QCOMPARE(asked, plain);
        QCOMPARE(asked, (QStringList{QStringLiteral("1.13.2"), QStringLiteral("1.2.3"), QStringLiteral("2.1.4")}));
        // dates with a four-digit year are dates, in the order they prove
        TextImportDialog dates(guessing("d\n13.05.2024\n01.02.2024\n"));
        QCOMPARE(kindIndex(dates, 0), 3);
        QVERIFY(!dates.findChild<QLabel *>(QStringLiteral("textImportNote"))->text().contains(QStringLiteral("not for certain")));
    }

    // The guess never switches "Read formulas" off: ticked, a value that begins with = is a formula as in a window that does not guess, through File > Open and through Paste Text Import; not ticked, it is text.
    void theGuessKeepsReadFormulasWorkingInOpenAndInPaste()
    {
        const QByteArray file = "f,g\n=1+1,1\n=2+2,2\n";
        for (const bool formulas : {true, false}) {
            QStringList guessed, notGuessed;
            for (const bool guess : {true, false}) {
                EditWindow e;
                e.win.w.setPromptForText(true);
                e.win.w.anchorHooks().textImportAsker = [guess, formulas](TextImportRequest *r) {
                    r->guessKinds = guess;
                    r->choices.readFormulas = formulas;
                    TextImportDialog d(*r);
                    r->choices = d.choices();
                    return true;
                };
                QVERIFY(e.open(file, QStringLiteral("g.csv")));
                (guess ? guessed : notGuessed) << e.shownAt(QStringLiteral("A2")) << e.shownAt(QStringLiteral("A3"));
            }
            QCOMPARE(guessed, notGuessed);
            QCOMPARE(guessed.first(), formulas ? QStringLiteral("2") : QStringLiteral("=1+1"));
            // pasted
            QStringList pasted, pastedPlain;
            for (const bool guess : {true, false}) {
                EditWindow e;
                QVERIFY(e.open(editBook()));
                auto *md = new QMimeData;
                md->setText(QStringLiteral("=1+1,1\n=2+2,2\n"));
                QApplication::clipboard()->setMimeData(md);
                e.win.w.anchorHooks().textImportAsker = [guess, formulas](TextImportRequest *r) {
                    r->guessKinds = guess;
                    r->choices.readFormulas = formulas;
                    TextImportDialog d(*r);
                    r->choices = d.choices();
                    return true;
                };
                e.select(QStringLiteral("D5"));
                e.win.w.act(QStringLiteral("paste.textImport"))->trigger();
                QVERIFY(e.settle());
                (guess ? pasted : pastedPlain) << e.shownAt(QStringLiteral("D5"));
            }
            QCOMPARE(pasted.first(), formulas ? QStringLiteral("2") : QStringLiteral("=1+1"));
            QCOMPARE(pastedPlain.first(), QStringLiteral("2"));   // (pasting as typed always reads a formula; the box is what the guess respects)
        }
    }

    // Whatever as typed reads right stays as typed: the file opened through the guessing preview comes in exactly as in a window that does not guess (booleans, times, dates with month names, signed numbers).
    void whatAsTypedReadsRightComesInAsInAWindowThatDoesNotGuess()
    {
        const QByteArray file = "b,t,d,n,w\nTRUE,12:30,\"May 13, 2024\",-5,x\nFALSE,13:45,\"Jun 1, 2024\",-7.5,y\ntrue,9:05,\"Jul 4, 2024\",+3,z\n";
        QStringList asked, plain;
        for (const bool ask : {true, false}) {
            EditWindow e;
            e.win.w.setPromptForText(ask);
            e.win.w.anchorHooks().textImportAsker = [](TextImportRequest *r) {
                TextImportDialog d(*r);
                for (int col = 0; col < 5; ++col) {
                    auto *box = d.findChild<QComboBox *>(QStringLiteral("columnKind%1").arg(col));
                    if (!box || box->currentIndex() != 0) return false;   // (every column is proposed as typed)
                }
                r->choices = d.choices();
                return true;
            };
            QVERIFY(e.open(file, QStringLiteral("g.csv")));
            for (const QString &col : {QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D")})
                for (int row = 2; row <= 4; ++row) (ask ? asked : plain) << e.shownAt(col + QString::number(row));
        }
        QCOMPARE(asked, plain);
        QCOMPARE(asked.size(), 12);
        QCOMPARE(asked.at(0), QStringLiteral("TRUE"));
        QVERIFY(asked.at(3).contains(QLatin1Char(':')));
        QCOMPARE(asked.at(9), QStringLiteral("-5"));
    }

    // A date column the guess makes is shown as the same column is in a window that does not guess (the format a typed date gets), when opened and when pasted.
    void aGuessedDateColumnIsShownInTheFormatAsTypedGives()
    {
        const QByteArray file = "d,n\n05/13/2024,1\n01/02/2024,2\n";
        QStringList asked, plain;
        for (const bool ask : {true, false}) {
            EditWindow e;
            e.win.w.setPromptForText(ask);
            e.win.w.anchorHooks().textImportAsker = [](TextImportRequest *r) {
                TextImportDialog d(*r);
                r->choices = d.choices();
                return true;
            };
            QVERIFY(e.open(file, QStringLiteral("g.csv")));
            (ask ? asked : plain) << e.shownAt(QStringLiteral("A2")) << e.shownAt(QStringLiteral("A3"));
        }
        QCOMPARE(asked, plain);
        QCOMPARE(asked.first(), QStringLiteral("5/13/2024"));
        QStringList pasted, pastedPlain;
        for (const bool guess : {true, false}) {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            auto *md = new QMimeData;
            md->setText(QStringLiteral("05/13/2024,1\n01/02/2024,2\n"));
            QApplication::clipboard()->setMimeData(md);
            e.win.w.anchorHooks().textImportAsker = [guess](TextImportRequest *r) {
                r->guessKinds = guess;
                TextImportDialog d(*r);
                r->choices = d.choices();
                return true;
            };
            e.select(QStringLiteral("D5"));
            e.win.w.act(QStringLiteral("paste.textImport"))->trigger();
            QVERIFY(e.settle());
            (guess ? pasted : pastedPlain) << e.shownAt(QStringLiteral("D5")) << e.shownAt(QStringLiteral("D6"));
        }
        QCOMPARE(pasted, pastedPlain);
    }

    // Pasted text is entered as typed in the person's own marks, so the dialog for it offers no marks and the proposal is what the paste makes; the dialog for a file still offers them.
    void pastedTextHasNoMarksSoTheProposalIsWhatThePasteMakes()
    {
        TextImportRequest given = guessing("007;\"1.234.567.890.123.456\"\n");
        given.marks = false;
        given.choices.decimalMark = QLatin1Char(',');   // (what a caller left in the request is not offered either, and does not decide)
        given.choices.thousandsMark = QLatin1Char('.');
        TextImportDialog pasted(given);
        QVERIFY(!pasted.findChild<QComboBox *>(QStringLiteral("textImportDecimal"))->isVisibleTo(&pasted));
        QVERIFY(!pasted.findChild<QComboBox *>(QStringLiteral("textImportThousands"))->isVisibleTo(&pasted));
        QVERIFY(!pasted.findChild<QCheckBox *>(QStringLiteral("textImportTrailingMinus"))->isVisibleTo(&pasted));
        QVERIFY(pasted.choices().decimalMark.isNull() && pasted.choices().thousandsMark.isNull() && !pasted.choices().trailingMinus);
        TextImportDialog file(guessing("a\n1\n"));
        QVERIFY(file.findChild<QComboBox *>(QStringLiteral("textImportDecimal"))->isVisibleTo(&file));
        // the paste: the request says there are no marks, the proposal for the column is as typed, and the cell is what as typed makes of the text
        EditWindow e;
        QVERIFY(e.open(editBook()));
        auto *md = new QMimeData;
        md->setText(QStringLiteral("x;\"1.234.567.890.123.456\"\ny;\"1.234,50\"\n"));
        QApplication::clipboard()->setMimeData(md);
        bool offered = true;
        int proposed = -1;
        e.win.w.anchorHooks().textImportAsker = [&offered, &proposed](TextImportRequest *r) {
            offered = r->marks;
            r->choices.separators = QStringLiteral(";");
            TextImportDialog d(*r);
            proposed = kindIndex(d, 1);
            r->choices = d.choices();
            return true;
        };
        e.select(QStringLiteral("D5"));
        e.win.w.act(QStringLiteral("paste.textImport"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(!offered);
        QCOMPARE(proposed, 0);
        QCOMPARE(e.shownAt(QStringLiteral("E5")), QStringLiteral("1.234.567.890.123.456"));
        QCOMPARE(e.shownAt(QStringLiteral("E6")), QStringLiteral("1.234,50"));   // (as typed in the person's marks: text, as the proposal said)
    }

    // A kind the person set stays through every later change (separator, marks, names), and a column they did not set is guessed again from the new split.
    void aKindThePersonSetIsNeverReplacedByALaterGuess()
    {
        TextImportDialog dialog(guessing("007,b,c\n1,2,3\n4,5,6\n"));
        QCOMPARE(kindIndex(dialog, 0), 0);
        QCOMPARE(kindIndex(dialog, 1), 0);
        QCOMPARE(kindIndex(dialog, 2), 0);
        dialog.findChild<QComboBox *>(QStringLiteral("columnKind1"))->setCurrentIndex(1);   // text
        QTRY_COMPARE(dialog.choices().columns.value(1).kind, jp::office::sheet::CsvColumn::Kind::Text);
        // the names box off: the 007 is a code in column 0, which is text again, and the person's column 1 stays as it was set
        dialog.findChild<QCheckBox *>(QStringLiteral("textImportNames"))->setChecked(false);
        QTRY_COMPARE(kindIndex(dialog, 0), 1);
        QCOMPARE(kindIndex(dialog, 2), 0);
        QCOMPARE(kindIndex(dialog, 1), 1);
        // a different separator: column 1 is the person's still, and the others follow the new split
        dialog.findChild<QCheckBox *>(QStringLiteral("sepComma"))->setChecked(false);
        dialog.findChild<QCheckBox *>(QStringLiteral("sepSemicolon"))->setChecked(true);
        QTRY_COMPARE(dialog.previewColumns(), 1);
        QCOMPARE(dialog.choices().columns.value(0).kind, jp::office::sheet::CsvColumn::Kind::General);   // (the guess of the one column: as typed)
        dialog.findChild<QCheckBox *>(QStringLiteral("sepSemicolon"))->setChecked(false);
        dialog.findChild<QCheckBox *>(QStringLiteral("sepComma"))->setChecked(true);
        QTRY_COMPARE(dialog.previewColumns(), 3);
        QCOMPARE(dialog.choices().columns.value(1).kind, jp::office::sheet::CsvColumn::Kind::Text);
        // a column the caller gave is a choice too
        TextImportRequest given = guessing("a,b\n1,2\n3,4\n");
        given.choices.columns = {jp::office::sheet::CsvColumn{jp::office::sheet::CsvColumn::Kind::Skip, QString()}};
        TextImportDialog kept(given);
        QCOMPARE(kindIndex(kept, 0), 5);
    }

    // The marks the person chooses change what as typed reads as a number: the guess follows them.
    void theMarksChosenDecideWhichColumnsAreNumbers()
    {
        TextImportDialog dialog(guessing("amount\n\"1.234.567.890.123.456\"\n\"2.000.000.000.000.000\"\n"));
        QCOMPARE(kindIndex(dialog, 0), 0);   // (the language's own marks are the point and comma: that is no number in them, and as typed keeps it as the text it is)
        auto *decimal = dialog.findChild<QComboBox *>(QStringLiteral("textImportDecimal"));
        auto *thousands = dialog.findChild<QComboBox *>(QStringLiteral("textImportThousands"));
        decimal->setCurrentIndex(decimal->findData(int(',')));
        thousands->setCurrentIndex(thousands->findData(int('.')));
        QTRY_COMPARE(kindIndex(dialog, 0), 1);   // (a number of 19 digits now: text, so that its last digits are not lost)
    }

    // A file of 300 columns: the first 100 are shown and guessed, the others come in as text, and the note says so.
    void aWideFileGuessesWhatIsShownAndBringsTheRestInAsText()
    {
        QByteArray line;
        for (int c = 0; c < 300; ++c) line += QByteArray::number(c) + ",";
        TextImportDialog dialog(guessing(line + "\n" + line + "\n"));
        QCOMPARE(dialog.previewColumns(), jp::office::sheet::kCsvPreviewColumns);
        QCOMPARE(dialog.choices().columns.size(), jp::office::sheet::kCsvPreviewColumns);
        QCOMPARE(dialog.choices().rest.kind, jp::office::sheet::CsvColumn::Kind::Text);
        QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("textImportNote"))->text().contains(QStringLiteral("first 100 are brought in as text")));
    }

    // Where the person sees the preview first (a text file opened in a window that asks, and pasted text) the request says to guess; where nobody is asked (a window that does not prompt, or the
    // command line's choices) the file is read exactly as before: a code stays what the reader makes of it, and nothing is proposed.
    void onlyWhereThePreviewIsSeenFirstIsTheKindGuessed()
    {
        const QByteArray file = "code,when\n1E5,05/13/2024\n007,01/02/2024\n";
        {
            EditWindow e;
            e.win.w.setPromptForText(true);
            bool guessed = false;
            e.win.w.anchorHooks().textImportAsker = [&guessed](TextImportRequest *r) {
                guessed = r->guessKinds;
                TextImportDialog dialog(*r);
                r->choices = dialog.choices();
                return true;
            };
            QVERIFY(e.open(file, QStringLiteral("g.csv")));
            QVERIFY(guessed);
            QCOMPARE(e.shownAt(QStringLiteral("A2")), QStringLiteral("1E5"));   // (kept: as typed it would be 100000)
            QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("007"));
            QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("5/13/2024"));   // (month first: 13 is no month; shown as a typed date is)
        }
        {
            // a window that does not ask reads the file as before
            EditWindow e;
            bool asked = false;
            e.win.w.anchorHooks().textImportAsker = [&asked](TextImportRequest *) {
                asked = true;
                return true;
            };
            QVERIFY(e.open(file, QStringLiteral("g.csv")));
            QVERIFY(!asked);
            QCOMPARE(e.shownAt(QStringLiteral("A2")), QStringLiteral("1.00E+05"));   // (read as a number, as on main)
            QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("007"));
            QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("5/13/2024"));
        }
        {
            // pasted text asks the same way
            EditWindow e;
            QVERIFY(e.open(editBook()));
            auto *md = new QMimeData;
            md->setText(QStringLiteral("code,qty\n007,5\n008,6\n"));
            QApplication::clipboard()->setMimeData(md);
            bool guessed = false;
            e.win.w.anchorHooks().textImportAsker = [&guessed](TextImportRequest *r) {
                guessed = r->guessKinds;
                TextImportDialog dialog(*r);
                r->choices = dialog.choices();
                return true;
            };
            e.select(QStringLiteral("D5"));
            e.win.w.act(QStringLiteral("paste.textImport"))->trigger();
            QVERIFY(e.settle());
            QVERIFY(guessed);
            QCOMPARE(e.shownAt(QStringLiteral("D6")), QStringLiteral("007"));
            QCOMPARE(e.shownAt(QStringLiteral("E6")), QStringLiteral("5"));
        }
    }
};

JP_TEST_CLASS(CrunchTextTests)
#include "tst_crunch_text.moc"
