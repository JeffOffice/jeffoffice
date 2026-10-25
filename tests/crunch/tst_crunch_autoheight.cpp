// Tests of automatic row heights: a row whose height is not a person's grows for text that wraps, a larger font, and an entry of several lines, shrinks back when they go (Undo and Redo
// too), keeps a height a person gave it, follows the width of its column, and is measured only when it is in view (or an edit changed it). Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "savedok.h"
#include "../localepin.h"

#include <QAction>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

const QString kLong = QStringLiteral("The quick brown fox jumps over the lazy dog and then keeps running through the long green field until the sun sets behind the hills far away");

// Rows 1 to 3: A1 a long text (not wrapped), B1 a number, A2 short, A3 empty.
// Styles with a font, a fill, and a border to change (a larger font needs the lists).
QByteArray autoStyles()
{
    return stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>"),
                     QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts><fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                    "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"));
}

QByteArray textBook(const QString &rowAttrs = QString())
{
    const QString rows = QStringLiteral("<row r=\"1\"%2><c r=\"A1\" t=\"inlineStr\"><is><t>%1</t></is></c><c r=\"B1\"><v>7</v></c></row><row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>short</t></is></c></row>").arg(kLong, rowAttrs);
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), autoStyles());
}

int rowPx(EditWindow &e, int row) { return e.view()->sheet().layout().rows.size(row); }

crunch::EditSession::Outcome wrap(EditWindow &e, const QString &ref, bool on)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    jp::office::sheet::CellRange range;
    range.valid = true;
    range.row1 = range.row2 = r;
    range.col1 = range.col2 = c;
    jp::office::sheet::FormatChange change;
    change.wrap = on;
    return e.win.w.session()->formatCells(0, range, change, QStringLiteral("Wrap Text"));
}

} // namespace

class CrunchAutoHeightTests : public QObject {
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

    void wrappedTextMakesARowTallerAndItShrinksBackWithUndoAndRedo()
    {
        EditWindow e;
        QVERIFY(e.open(textBook()));
        const int plain = rowPx(e, 0);
        QCOMPARE(rowPx(e, 1), plain);
        QVERIFY(wrap(e, QStringLiteral("A1"), true).ok);
        QTRY_VERIFY(rowPx(e, 0) >= plain * 2);
        QCOMPARE(rowPx(e, 1), plain);   // (the other rows are as they were)
        const int tall = rowPx(e, 0);
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(rowPx(e, 0), plain);
        e.win.w.undoStack()->redo();
        QTRY_COMPARE(rowPx(e, 0), tall);
        QVERIFY(wrap(e, QStringLiteral("A1"), false).ok);
        QTRY_COMPARE(rowPx(e, 0), plain);
    }

    void aRowAPersonGaveAHeightKeepsIt()
    {
        EditWindow e;
        QVERIFY(e.open(textBook(QStringLiteral(" ht=\"30\" customHeight=\"1\""))));
        const int fixed = rowPx(e, 0);
        QVERIFY(wrap(e, QStringLiteral("A1"), true).ok);
        QTest::qWait(300);
        QCOMPARE(rowPx(e, 0), fixed);
        // and one the file gave a height that is the program's own is measured again once its text changes
        EditWindow second;
        QVERIFY(second.open(textBook(QStringLiteral(" ht=\"15\""))));
        const int stored = rowPx(second, 0);
        QVERIFY(wrap(second, QStringLiteral("A1"), true).ok);
        QTRY_VERIFY(rowPx(second, 0) > stored);
    }

    void anEntryOfSeveralLinesTurnsOnWrapAndGrowsTheRowAsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(textBook()));
        const int plain = rowPx(e, 1);
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("one\ntwo\nthree")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        const auto &style = e.win.w.session()->workbook()->styles().format(e.win.w.session()->calculation()->content(0, 1, 0).style);
        QVERIFY(style.wrap);
        QTRY_VERIFY(rowPx(e, 1) >= plain * 2 + plain / 2);
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(rowPx(e, 1), plain);
        QVERIFY(!e.win.w.session()->workbook()->styles().format(e.win.w.session()->calculation()->content(0, 1, 0).style).wrap);
    }

    void aLargerFontMakesTheRowTaller()
    {
        EditWindow e;
        QVERIFY(e.open(textBook()));
        const int plain = rowPx(e, 1);
        jp::office::sheet::CellRange range;
        jp::office::sheet::parseRangeRef(QStringLiteral("A2"), &range);
        jp::office::sheet::FormatChange change;
        change.fontSize = 28.0;
        QVERIFY(e.win.w.session()->formatCells(0, range, change, QStringLiteral("Font Size")).ok);
        QTRY_VERIFY(rowPx(e, 1) > plain * 3 / 2);
    }

    void aWrappedRowFollowsTheWidthOfItsColumn()
    {
        EditWindow e;
        QVERIFY(e.open(textBook()));
        QVERIFY(wrap(e, QStringLiteral("A1"), true).ok);
        QTRY_VERIFY(rowPx(e, 0) > rowPx(e, 1));
        const int before = rowPx(e, 0);
        crunch::EditSession::SizeChange by;
        by.setSize = true;
        by.size = 4;   // (narrower than the usual 8)
        QVERIFY(e.win.w.session()->resizeColumns(0, 0, 0, by, QStringLiteral("Column Width")).ok);
        QTRY_VERIFY(rowPx(e, 0) > before);
        by.size = 250;
        QVERIFY(e.win.w.session()->resizeColumns(0, 0, 0, by, QStringLiteral("Column Width")).ok);
        QTRY_VERIFY(rowPx(e, 0) <= rowPx(e, 1) + 4);
    }

    void onlyTheRowsInViewAreMeasuredAndTheOthersWhenTheyComeIntoView()
    {
        // 3,000 rows of long wrapped text with no height: only those in view are measured at first
        QString rows;
        for (int r = 1; r <= 3000; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" s=\"1\" t=\"inlineStr\"><is><t>%2</t></is></c></row>").arg(r).arg(kLong);
        const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyAlignment=\"1\"><alignment wrapText=\"1\"/></xf>"));
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles)));
        const int plain = e.view()->sheet().layout().rows.defaultSize();
        QTRY_VERIFY(rowPx(e, 0) > plain);
        QCOMPARE(rowPx(e, 2900), plain);
        e.view()->selectCell(2900, 0);
        e.view()->ensureVisible(2900, 0);
        QTRY_VERIFY(rowPx(e, 2900) > plain);
        QCOMPARE(rowPx(e, 1500), plain);   // (never looked at)
    }

    void theHeightsOfAutomaticRowsAreSavedAndKeptAsNotAPersons()
    {
        EditWindow e;
        QVERIFY(e.open(textBook()));
        QVERIFY(wrap(e, QStringLiteral("A1"), true).ok);
        QTRY_VERIFY(rowPx(e, 0) > rowPx(e, 1));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> parts;
        jp::readZip(f.readAll(), parts);
        const QString xml = QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QRegularExpression firstRow(QStringLiteral("<row r=\"1\"[^>]*>"));
        const QString tag = firstRow.match(xml).captured();
        QVERIFY2(tag.contains(QStringLiteral("ht=\"")) && !tag.contains(QStringLiteral("customHeight=\"1\"")), qPrintable(tag));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.view()->sheet().layout().rows.size(0), rowPx(e, 0));
    }
};

JP_TEST_CLASS(CrunchAutoHeightTests)
#include "tst_crunch_autoheight.moc"
