// Tests of JeffCrunch's window saving a copy of the workbook as an .xls or .ods file: the file is written with what was edited in it, the person is told first what such a file cannot hold
// (and may give it up or save an .xlsx instead), the file the workbook came from is never written, and the workbook in the window is left as it was. Every file is built here, in a
// temporary folder.
#include "../sheet/odshelpers.h"
#include "app/crunchwindow.h"
#include "app/otherformats.h"
#include "grid/gridview.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace jp;

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace {

// A window with a workbook open from a folder of its own, a file dialog that answers what the test says, and what was said to the person.
struct Win {
    QTemporaryDir dir;
    CrunchWindow w;
    QStringList told, asked, filters;
    QString path, answer;
    QByteArray original;
    CrunchWindow::LossAnswer lossAnswer = CrunchWindow::LossAnswer::Continue;
    Win()
    {
        w.setNotifier([this](const QString &title, const QString &text) { told << title + QStringLiteral(": ") + text; });
        w.setSavePicker([this](const QString &start, const QString &filter) {
            asked << start;
            filters << filter;
            return answer;
        });
        w.setLossAsker([this](const QString &title, const QString &text) {
            told << title + QStringLiteral(": ") + text;
            return lossAnswer;
        });
        w.resize(1100, 640);
        w.show();
    }
    bool open(const QByteArray &bytes, const QString &name)
    {
        original = bytes;
        path = dir.filePath(name);
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) return false;
        f.write(bytes);
        f.close();
        QSignalSpy first(&w, &CrunchWindow::firstScreenShown), done(&w, &CrunchWindow::calculationFinished), loaded(&w, &CrunchWindow::loadFinished);
        if (!w.openFile(path)) return false;
        if (!(first.wait(20000) && (loaded.count() > 0 || loaded.wait(20000)) && (done.count() > 0 || done.wait(20000)))) return false;
        w.activateWindow();
        return QTest::qWaitForWindowActive(&w);
    }
    void enter(quint32 row, quint32 col, const QString &text)
    {
        w.view()->selectCell(row, col);
        QTest::keyClick(w.view(), text.at(0).toLatin1());
        if (text.size() > 1) QTest::keyClicks(w.cellEditor(), text.mid(1));
        QTest::keyClick(w.cellEditor(), Qt::Key_Return);
        QTest::qWaitFor([&] { return !w.isCalculating(); }, 20000);
    }
    bool originalIsUntouched() const
    {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) && f.readAll() == original;
    }
};

// An .ods: A1 = 2, A2 = 3, A3 = SUM(A1:A2), B1 the text "hello"; a second sheet with 7 in A1.
QByteArray odsBook()
{
    const QString r1 = row(floatCell(2) + stringCell(QStringLiteral("hello")));
    const QString r2 = row(floatCell(3));
    const QString r3 = row(cell(QStringLiteral("table:formula=\"of:=SUM([.A1:.A2])\" office:value-type=\"float\" office:value=\"5\""), QStringLiteral("<text:p>5</text:p>")));
    return buildOds(contentXml(table(QStringLiteral("S"), columns(2) + r1 + r2 + r3) + table(QStringLiteral("Other"), row(floatCell(7)))));
}

// An .xlsx with a conditional format and a validation, which a copy in the older forms does not write.
QByteArray xlsxWithRules()
{
    return buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>4</v></c></row>"),
                                QStringLiteral("<conditionalFormatting sqref=\"A1\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"1\" operator=\"greaterThan\"><formula>0</formula></cfRule></conditionalFormatting>"
                                               "<dataValidations count=\"1\"><dataValidation type=\"whole\" sqref=\"A1\"><formula1>1</formula1></dataValidation></dataValidations>"))});
}

bool read(const QString &path, Workbook *wb, Sheet *sheet, int index = 0)
{
    QString why;
    return wb->openFile(path, &why) && wb->readSheet(index, sheet);
}

QString shownCell(const Workbook &wb, const Sheet &s, const QString &ref) { return show(s, wb.sharedStrings(), ref); }

} // namespace

class CrunchCopyTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Saving a workbook opened from an .ods under an .xls name writes an .xls with the edit in it; the .ods is not touched and the window keeps its changes.
    void aWorkbookIsSavedAsAnXlsCopy()
    {
        Win o;
        QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
        o.enter(0, 0, QStringLiteral("9"));
        o.answer = o.dir.filePath(QStringLiteral("copy.xls"));
        QSignalSpy copied(&o.w, &CrunchWindow::copyExported);
        o.w.save();
        QVERIFY2(copied.count() > 0 || copied.wait(20000), qPrintable(o.told.join(QLatin1Char('|'))));
        QCOMPARE(copied.first().at(1).toBool(), true);
        Workbook wb;
        Sheet s;
        QVERIFY(read(o.answer, &wb, &s));
        QVERIFY(wb.isBinary());
        QCOMPARE(shownCell(wb, s, QStringLiteral("A1")), QStringLiteral("n:9"));
        QCOMPARE(shownCell(wb, s, QStringLiteral("B1")), QStringLiteral("s:hello"));
        QCOMPARE(shownCell(wb, s, QStringLiteral("A3")), QStringLiteral("n:12"));   // (the formula was worked out again with the edit)
        QCOMPARE(wb.info().sheets.size(), 2);
        QVERIFY(o.originalIsUntouched());
        QVERIFY(o.w.isModified());
    }

    void aWorkbookIsSavedAsAnOdsCopy()
    {
        Win o;
        QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
        o.enter(1, 0, QStringLiteral("8"));
        o.answer = o.dir.filePath(QStringLiteral("copy.ods"));
        QSignalSpy copied(&o.w, &CrunchWindow::copyExported);
        o.w.save();
        QVERIFY2(copied.count() > 0 || copied.wait(20000), qPrintable(o.told.join(QLatin1Char('|'))));
        QCOMPARE(copied.first().at(1).toBool(), true);
        Workbook wb;
        Sheet s;
        QVERIFY(read(o.answer, &wb, &s));
        QCOMPARE(shownCell(wb, s, QStringLiteral("A2")), QStringLiteral("n:8"));
        QCOMPARE(shownCell(wb, s, QStringLiteral("A3")), QStringLiteral("n:10"));
        QVERIFY(o.originalIsUntouched());
    }

    // The file the workbook came from is never written, whatever the kind chosen.
    void theFileTheWorkbookCameFromIsNotWritten()
    {
        Win o;
        QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
        QSignalSpy copied(&o.w, &CrunchWindow::copyExported);
        QVERIFY(!o.w.saveAs(o.path));
        QVERIFY(o.told.last().contains(QLatin1String("not written to")));
        QCOMPARE(copied.count(), 0);
        QVERIFY(o.originalIsUntouched());
    }

    // A second name for the file the workbook came from (a hard link) is the same file: a copy under it would overwrite the open workbook, and is refused.
    void aHardLinkToTheFileTheWorkbookCameFromIsNotWritten()
    {
#ifdef Q_OS_UNIX
        Win o;
        QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
        const QString link = o.dir.filePath(QStringLiteral("other-name.ods"));
        QVERIFY(::link(QFile::encodeName(o.path).constData(), QFile::encodeName(link).constData()) == 0);
        QSignalSpy copied(&o.w, &CrunchWindow::copyExported);
        QVERIFY(!o.w.saveAs(link));
        QVERIFY2(o.told.last().contains(QLatin1String("not written to")), qPrintable(o.told.join(QLatin1Char('|'))));
        QCOMPARE(copied.count(), 0);
        QVERIFY(o.originalIsUntouched());
#else
        QSKIP("hard links are made on Unix systems here");
#endif
    }

    // What the file cannot hold is said before it is written; Cancel writes nothing, and "save as .xlsx" asks for that name instead.
    void thePersonIsToldWhatTheFileCannotHold()
    {
        {
            Win o;
            QVERIFY(o.open(xlsxWithRules(), QStringLiteral("rules.xlsx")));
            o.lossAnswer = CrunchWindow::LossAnswer::Cancel;
            const QString target = o.dir.filePath(QStringLiteral("copy.xls"));
            QSignalSpy copied(&o.w, &CrunchWindow::copyExported);
            QVERIFY(o.w.saveAs(target));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            QCOMPARE(copied.first().at(1).toBool(), false);
            QVERIFY2(o.told.last().contains(QLatin1String("conditional formatting")) && o.told.last().contains(QLatin1String("data validation")), qPrintable(o.told.last()));
            QVERIFY(!QFile::exists(target));
        }
        {
            Win o;
            QVERIFY(o.open(xlsxWithRules(), QStringLiteral("rules.xlsx")));
            o.lossAnswer = CrunchWindow::LossAnswer::Continue;
            const QString target = o.dir.filePath(QStringLiteral("copy.ods"));
            QSignalSpy copied(&o.w, &CrunchWindow::copyExported);
            QVERIFY(o.w.saveAs(target));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            QCOMPARE(copied.first().at(1).toBool(), true);
            QVERIFY(QFile::exists(target));
        }
        {
            Win o;
            QVERIFY(o.open(xlsxWithRules(), QStringLiteral("rules.xlsx")));
            o.lossAnswer = CrunchWindow::LossAnswer::SaveXlsx;
            o.answer = o.dir.filePath(QStringLiteral("other.xlsx"));
            QSignalSpy copied(&o.w, &CrunchWindow::copyExported), saved(&o.w, &CrunchWindow::saved);
            QVERIFY(o.w.saveAs(o.dir.filePath(QStringLiteral("copy.xls"))));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            QCOMPARE(copied.first().at(1).toBool(), false);
            QVERIFY(saved.count() > 0 || saved.wait(20000));
            QVERIFY(QFile::exists(o.answer));
            QVERIFY(!QFile::exists(o.dir.filePath(QStringLiteral("copy.xls"))));
        }
    }

    // The kinds of file are in the Save As dialog.
    void theSaveDialogListsTheTwoKinds()
    {
        QVERIFY(crunch::isCopyName(QStringLiteral("/a/b.XLS")));
        QVERIFY(crunch::isCopyName(QStringLiteral("/a/b.ods")));
        QVERIFY(!crunch::isCopyName(QStringLiteral("/a/b.xlsx")));
        QVERIFY(!crunch::isCopyName(QStringLiteral("/a/xls")));
        QVERIFY(crunch::copyFilters().contains(QLatin1String("*.xls")) && crunch::copyFilters().contains(QLatin1String("*.ods")));
        QCOMPARE(crunch::withPickedEnding(QStringLiteral("/a/b"), crunch::xlsFilter()), QStringLiteral("/a/b.xls"));
        QCOMPARE(crunch::withPickedEnding(QStringLiteral("/a/b"), crunch::odsFilter()), QStringLiteral("/a/b.ods"));
        QCOMPARE(crunch::withPickedEnding(QStringLiteral("/a/b.v2"), crunch::odsFilter()), QStringLiteral("/a/b.v2"));
        QVERIFY(crunch::copyWarning(QStringLiteral("xls"), {}, {}).isEmpty());
        const QString text = crunch::copyWarning(QStringLiteral("xls"), {QStringLiteral("charts"), QStringLiteral("notes")}, {QStringLiteral("1 sheets cut")});
        QVERIFY2(text.contains(QLatin1String("charts, notes")) && text.contains(QLatin1String("1 sheets cut")) && text.contains(QLatin1String(".xls")), qPrintable(text));
        for (const QString &word : {QStringLiteral("Excel"), QStringLiteral("Microsoft"), QStringLiteral("Office")}) QVERIFY(!text.contains(word));
    }
};

JP_TEST_CLASS(CrunchCopyTests)
#include "tst_crunch_copy.moc"
