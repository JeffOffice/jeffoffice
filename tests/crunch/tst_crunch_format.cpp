// Tests of how cells look and how they are formatted: numbers in the person's language, conditional formats, the formatting commands and the Format Cells dialog, the sheets of a
// workbook, and the default sizes. Every workbook is built here, and every locale is set explicitly.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QAction>
#include <QApplication>
#include <QLineEdit>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// Numbers in four formats:
//         A (General)   B (#,##0.00)   C (0.00%)    D (d.m.yyyy)    E (0.0E+00)
//   1     1.5           1234.5         0.1234       45000           12345
//   2     1234.5        -1234567.891   2.5          45001.5         0.000123
//   3     -0.25         0.5            -0.01        0               1e15
QByteArray numbersBook()
{
    const QByteArray styles = stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"#,##0.00\"/><numFmt numFmtId=\"165\" formatCode=\"d.m.yyyy\"/><numFmt numFmtId=\"166\" formatCode=\"0.0E+00\"/>"),
                                        QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\" applyNumberFormat=\"1\"/><xf numFmtId=\"10\" applyNumberFormat=\"1\"/><xf numFmtId=\"165\" applyNumberFormat=\"1\"/><xf numFmtId=\"166\" applyNumberFormat=\"1\"/>"));
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1.5</v></c><c r=\"B1\" s=\"1\"><v>1234.5</v></c><c r=\"C1\" s=\"2\"><v>0.1234</v></c><c r=\"D1\" s=\"3\"><v>45000</v></c><c r=\"E1\" s=\"4\"><v>12345</v></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>1234.5</v></c><c r=\"B2\" s=\"1\"><v>-1234567.891</v></c><c r=\"C2\" s=\"2\"><v>2.5</v></c><c r=\"D2\" s=\"3\"><v>45001.5</v></c><c r=\"E2\" s=\"4\"><v>0.000123</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\"><v>-0.25</v></c><c r=\"B3\" s=\"1\"><v>0.5</v></c><c r=\"C3\" s=\"2\"><v>-0.01</v></c><c r=\"D3\" s=\"3\"><v>0</v></c><c r=\"E3\" s=\"4\"><v>1e15</v></c></row>");
    return buildXlsx({worksheet(rows, QString(), QStringLiteral("<cols><col min=\"1\" max=\"5\" width=\"16\" customWidth=\"1\"/></cols>"))}, QByteArray(), QString(), QString(), styles);
}

} // namespace

class CrunchFormatTests : public QObject {
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

    // Numbers are shown with the decimal mark and the digit grouping of the person's language, in General and in the number formats (which stay as the file has them); a date is by its code.
    // The formula bar and F2 give the number the way it is typed in that language.
    void numbersAreShownInTheLanguageOfTheComputer()
    {
        struct Case {
            QLocale locale;
            QString decimal;   // what the locale writes (the group is the locale's own)
        };
        const QVector<Case> cases = {
            {QLocale(QLocale::English, QLocale::UnitedStates), QStringLiteral(".")},
            {QLocale(QLocale::German, QLocale::Germany), QStringLiteral(",")},
            {QLocale(QLocale::French, QLocale::France), QStringLiteral(",")},
            {QLocale(QLocale::German, QLocale::Switzerland), QStringLiteral(".")},
            {QLocale(QLocale::Japanese, QLocale::Japan), QStringLiteral(".")},
        };
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        for (const Case &c : cases) {
            const QString name = c.locale.name();
            e.win.w.setLocale(c.locale);
            const QString d = c.decimal, g = c.locale.groupSeparator();
            QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("1") + d + QStringLiteral("5"));
            QCOMPARE(e.shownAt(QStringLiteral("A2")), QStringLiteral("1234") + d + QStringLiteral("5"));
            QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("-0") + d + QStringLiteral("25"));
            QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("1") + g + QStringLiteral("234") + d + QStringLiteral("50"));
            QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("-1") + g + QStringLiteral("234") + g + QStringLiteral("567") + d + QStringLiteral("89"));
            QCOMPARE(e.shownAt(QStringLiteral("B3")), QStringLiteral("0") + d + QStringLiteral("50"));
            QCOMPARE(e.shownAt(QStringLiteral("C1")), QStringLiteral("12") + d + QStringLiteral("34%"));
            QCOMPARE(e.shownAt(QStringLiteral("D1")), QStringLiteral("15.3.2023"));           // (the code, whatever the language)
            QCOMPARE(e.shownAt(QStringLiteral("E1")), QStringLiteral("1") + d + QStringLiteral("2E+04"));
            QCOMPARE(e.shownAt(QStringLiteral("E2")), QStringLiteral("1") + d + QStringLiteral("2E-04"));
            // F2 and the formula bar: as the language types a number
            QCOMPARE(e.win.w.session()->editText(0, 0, 0), QStringLiteral("1") + d + QStringLiteral("5"));
            QCOMPARE(e.win.w.session()->editText(0, 1, 1), QString(QStringLiteral("-1234567") + d + QStringLiteral("891")));
            e.select(QStringLiteral("A1"));
            QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("1") + d + QStringLiteral("5"));
            e.select(QStringLiteral("B2"));
            QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("-1234567") + d + QStringLiteral("891"));
            // what is typed in the language comes back as the same number
            e.select(QStringLiteral("A3"));
            e.enter(QStringLiteral("2") + d + QStringLiteral("75"));
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:2.75"));
            QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("2") + d + QStringLiteral("75"));
            e.win.w.undoStack()->undo();
            QVERIFY(e.settle());
            QVERIFY2(e.shownAt(QStringLiteral("A3")) == QStringLiteral("-0") + d + QStringLiteral("25"), qPrintable(name));
        }
        // a copy gives the text as shown
        e.win.w.setLocale(QLocale(QLocale::German, QLocale::Germany));
        QCOMPARE(e.view()->sheet().textOf(*e.view()->sheet().cellAt(1, 1)), QStringLiteral("-1.234.567,89"));
    }
};

JP_TEST_CLASS(CrunchFormatTests)
#include "tst_crunch_format.moc"
