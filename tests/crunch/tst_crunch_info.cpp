// Tests of JeffCrunch's Info page: the properties a workbook keeps (written at the next save, into parts the file may not have had), Mark as Final, and Inspect Workbook. Every workbook is
// built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/filescreen.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/templates.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "office/packageprops.h"
#include "../localepin.h"

#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>

using namespace jp;
using namespace crunchtest;

namespace {

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QMap<QString, QByteArray> parts;
    jp::readZip(f.readAll(), parts);
    return parts;
}

bool saveAndWaitHere(CrunchWindow &w)
{
    QSignalSpy saved(&w, &CrunchWindow::saved);
    if (!w.save()) return false;
    return (saved.count() > 0 || saved.wait(20000)) && saved.last().value(1).toBool();
}

} // namespace

class CrunchInfoTests : public QObject {
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
    }
    void init() { Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Tester")); }

    void propertiesAreWrittenIntoPartsTheFileDidNotHave()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(!partsOf(e.path).contains(QStringLiteral("docProps/core.xml")));
        office::PackageProps p = e.win.w.properties();
        p.title = QStringLiteral("Budget 2027");
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.custom = {{QStringLiteral("Project"), QStringLiteral("lpwstr"), QStringLiteral("Apollo")}};
        e.win.w.applyProperties(p);
        QVERIFY(e.win.w.isWindowModified());
        QVERIFY(saveAndWaitHere(e.win.w));
        const auto parts = partsOf(e.path);
        QVERIFY(parts.contains(QStringLiteral("docProps/core.xml")));
        QVERIFY(parts.contains(QStringLiteral("docProps/app.xml")));
        QVERIFY(parts.contains(QStringLiteral("docProps/custom.xml")));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("/docProps/core.xml"));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("custom-properties+xml"));
        QVERIFY(parts.value(QStringLiteral("_rels/.rels")).contains("metadata/core-properties"));
        QVERIFY(parts.value(QStringLiteral("_rels/.rels")).contains("custom-properties"));
        // reopened, the workbook has them
        const QString second = e.folder.dir.filePath(QStringLiteral("second.xlsx"));
        QVERIFY(QFile::copy(e.path, second));
        Window again;
        QVERIFY(again.open(second));
        const office::PackageProps back = again.w.properties();
        QCOMPARE(back.title, QStringLiteral("Budget 2027"));
        QCOMPARE(back.creator, QStringLiteral("Ann Lee"));
        QCOMPARE(back.company, QStringLiteral("Acme"));
        QCOMPARE(back.custom.size(), 1);
        QVERIFY(back.created.isValid());
        // and a second change replaces the parts that are there now
        p = again.w.properties();
        p.title = QStringLiteral("Budget 2028");
        p.custom.clear();
        again.w.applyProperties(p);
        QVERIFY(saveAndWaitHere(again.w));
        const auto after = partsOf(second);
        QVERIFY(after.value(QStringLiteral("docProps/core.xml")).contains("Budget 2028"));
        QVERIFY(!after.contains(QStringLiteral("docProps/custom.xml")));
        QVERIFY(!after.value(QStringLiteral("_rels/.rels")).contains("custom-properties"));
        QVERIFY(!after.value(QStringLiteral("[Content_Types].xml")).contains("custom-properties"));
    }

    void aWorkbookMarkedAsFinalOpensForReadingOnly()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.setMarkedFinal(true);
        QTRY_VERIFY(e.win.w.isReadOnly());
        QVERIFY(!e.win.w.isWindowModified());
        const QString second = e.folder.dir.filePath(QStringLiteral("second.xlsx"));
        QVERIFY(QFile::copy(e.path, second));
        Window again;
        QVERIFY(again.open(second));
        QVERIFY(again.w.isReadOnly());
        QVERIFY(again.w.markedAsFinal());
        again.w.setMarkedFinal(false);
        QVERIFY(!again.w.isReadOnly());
        QVERIFY(!again.w.markedAsFinal());
        // a copy of it is for editing
        Window copy;
        QVERIFY(copy.open(second, OpenHow::Copy));
        QVERIFY(!copy.w.isReadOnly());
    }

    void inspectFindsHiddenSheetsAndPersonalInformation()
    {
        EditWindow e;
        QVERIFY(e.open(budgetWorkbook()));
        office::PackageProps p = e.win.w.properties();
        p.creator = QStringLiteral("Ann");
        p.manager = QStringLiteral("Bo");
        e.win.w.applyProperties(p);
        const QVector<InspectItem> items = e.win.w.inspectItems();
        auto find = [&](const char *title) -> const InspectItem * {
            for (const InspectItem &i : items)
                if (i.title == QLatin1String(title)) return &i;
            return nullptr;
        };
        QVERIFY(find("Hidden sheets"));
        QCOMPARE(find("Hidden sheets")->count(), 1);
        QCOMPARE(find("Document properties and personal information")->count(), 3);   // (the person who saved is named by the edit)
        QCOMPARE(find("Document properties and personal information")->remove(), 3);
        QCOMPARE(find("Document properties and personal information")->count(), 0);
        QCOMPARE(find("Hidden sheets")->remove(), 1);
        QCOMPARE(find("Hidden sheets")->count(), 0);
        QVERIFY(!find("External links")->removable);
    }

    void inspectFindsAndShowsHiddenRowsAndColumns()
    {
        EditWindow e;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\" hidden=\"1\"><c r=\"A2\"><v>2</v></c></row>"
                                            "<row r=\"3\" hidden=\"1\"><c r=\"A3\"><v>3</v></c></row><row r=\"5\" hidden=\"1\"><c r=\"A5\"><v>5</v></c></row>");
        const QString cols = QStringLiteral("<cols><col min=\"2\" max=\"3\" width=\"9\" hidden=\"1\" customWidth=\"1\"/></cols>");
        QVERIFY(e.open(buildXlsx({worksheet(rows, QString(), cols)})));
        const QVector<InspectItem> items = e.win.w.inspectItems();
        const InspectItem *item = nullptr;
        for (const InspectItem &i : items)
            if (i.title == QLatin1String("Hidden rows and columns")) item = &i;
        QVERIFY(item);
        QCOMPARE(item->count(), 5);   // (three rows and two columns)
        QVERIFY(item->removable);
        QCOMPARE(item->remove(), 5);
        QCOMPARE(item->count(), 0);
    }

    void inspectFindsAndTakesOutHeadersAndFooters()
    {
        EditWindow e;
        const QString after = QStringLiteral("<headerFooter><oddHeader>&amp;CQuarterly</oddHeader><oddFooter>&amp;LPage &amp;P</oddFooter></headerFooter>");
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), after)})));
        const QVector<InspectItem> items = e.win.w.inspectItems();
        const InspectItem *item = nullptr;
        for (const InspectItem &i : items)
            if (i.title == QLatin1String("Headers and footers")) item = &i;
        QVERIFY(item);
        QCOMPARE(item->count(), 1);
        QCOMPARE(item->remove(), 1);
        QCOMPARE(item->count(), 0);
    }

    // Every built-in template opens, works its formulas out with no error, and says what it should.
    void everyBuiltInTemplateOpensAndWorksItsFormulasOut()
    {
        const QVector<crunch::TemplateInfo> all = crunch::builtinTemplates();
        QVERIFY(all.size() >= 12);
        QStringList ids;
        for (const crunch::TemplateInfo &t : all) {
            QVERIFY(!ids.contains(t.id));
            ids << t.id;
            QVERIFY(!t.name.isEmpty() && !t.category.isEmpty() && !t.description.isEmpty());
            const QByteArray bytes = crunch::builtinTemplateBytes(t.id);
            QVERIFY2(!bytes.isEmpty(), qPrintable(t.id));
            const QImage picture = crunch::builtinTemplatePreview(t.id, 200);
            QVERIFY2(picture.width() == 200 && picture.height() > 50, qPrintable(t.id));
            bool drawn = false;
            for (int y = 0; y < picture.height() && !drawn; ++y)
                for (int x = 0; x < picture.width() && !drawn; ++x) drawn = picture.pixelColor(x, y) != QColor(Qt::white);
            QVERIFY2(drawn, qPrintable(t.id));
            EditWindow e;
            QVERIFY2(e.open(bytes), qPrintable(t.id));
            QVERIFY2(e.settle(), qPrintable(t.id));
            for (int row = 1; row <= 80; ++row)
                for (int col = 0; col < 16; ++col) {
                    const QString ref = QString(QChar(u'A' + col)) + QString::number(row);
                    QVERIFY2(!e.shownAt(ref).startsWith(QLatin1Char('#')), qPrintable(t.id + QLatin1Char(' ') + ref + QLatin1Char(' ') + e.shownAt(ref)));
                }
            if (t.id == QLatin1String("loan")) {
                QCOMPARE(e.shownAt(QStringLiteral("B6")), QStringLiteral("$386.66"));
                QVERIFY(e.shownAt(QStringLiteral("E70")).endsWith(QLatin1String("0.00")));   // (sixty payments: the balance is paid off at the last)
                QVERIFY(e.shownAt(QStringLiteral("A71")).isEmpty());
            }
            if (t.id == QLatin1String("calendar")) {
                QCOMPARE(e.shownAt(QStringLiteral("A4")), QStringLiteral("January 2027"));
                QCOMPARE(e.shownAt(QStringLiteral("F6")), QStringLiteral("1"));   // (January 1, 2027 is a Friday)
            }
            if (t.id == QLatin1String("budget")) QCOMPARE(e.shownAt(QStringLiteral("B7")), QStringLiteral("$0.00"));
        }
        QVERIFY(crunch::builtinTemplateBytes(QStringLiteral("none")).isEmpty());
    }

    // Open and Repair of an .xls file whose container is cut short: the workbook is found by its records, the cells that can be read are kept, and the person is told.
    void aDamagedXlsFileIsRepairedByItsRecords()
    {
        QByteArray rows;
        for (int r = 1; r <= 700; ++r) rows += "<row r=\"" + QByteArray::number(r) + "\"><c r=\"A" + QByteArray::number(r) + "\"><v>" + QByteArray::number(r * 3) + "</v></c></row>";
        QByteArray damaged;
        {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({worksheet(QString::fromLatin1(rows))})));
            const QString target = e.folder.dir.filePath(QStringLiteral("whole.xls"));
            QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(target));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            QFile f(target);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray whole = f.readAll();
            QVERIFY(whole.size() > 6000);
            damaged = whole.left(whole.size() * 4 / 5);   // (the tables at the end that name the streams are gone)
        }
        EditWindow fixed;
        const QString again = fixed.folder.write(QStringLiteral("cut.xls"), damaged);
        QVERIFY2(fixed.win.open(again, OpenHow::Repair), qPrintable(fixed.win.told.join(QLatin1Char('|'))));
        QVERIFY(fixed.settle());
        QCOMPARE(fixed.shownAt(QStringLiteral("A1")), QStringLiteral("3"));
        QCOMPARE(fixed.shownAt(QStringLiteral("A100")), QStringLiteral("300"));
        QVERIFY2(fixed.win.told.join(QLatin1Char('|')).contains(QLatin1String("repaired")), qPrintable(fixed.win.told.join(QLatin1Char('|'))));
    }

    void personalInformationStaysOutOfLaterEdits()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        office::PackageProps p = e.win.w.properties();
        p.creator = QStringLiteral("Ann");
        e.win.w.applyProperties(p);
        QCOMPARE(e.win.w.removePersonalInformation(), 2);   // (the author, and the person who saved: the name in Settings)
        p = e.win.w.properties();
        p.title = QStringLiteral("After");
        e.win.w.applyProperties(p);
        QVERIFY(e.win.w.properties().lastModifiedBy.isEmpty());
        QVERIFY(e.win.w.properties().creator.isEmpty());
        QCOMPARE(e.win.w.properties().title, QStringLiteral("After"));
    }

    void markAsFinalIsRefusedForTextFilesAndStaysVisibleOnTheStatusBar()
    {
        EditWindow t;
        QVERIFY(t.open("a,b\n1,2\n", QStringLiteral("data.csv")));
        QVERIFY(!t.win.w.markFinalBlocked().isEmpty());
        t.win.w.setMarkedFinal(true);
        QVERIFY(!t.win.w.isReadOnly());
        QVERIFY(!t.win.w.markedAsFinal());
        QVERIFY(t.win.told.last().contains(QLatin1String("can't hold the mark")));
        // an .xls and an .ods hold no mark either (named so, whatever they hold)
        for (const char *name : {"old.xls", "sheet.ods"}) {
            EditWindow o;
            QVERIFY(o.open(editBook(), QString::fromLatin1(name)));
            QVERIFY2(!o.win.w.markFinalBlocked().isEmpty(), name);
            o.win.w.setMarkedFinal(true);
            QVERIFY(!o.win.w.isReadOnly());
            QVERIFY(!o.win.w.markedAsFinal());
        }
        // a workbook marked as final shows its note on the status bar for as long as it is read-only, whatever else the bar says
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.markFinalBlocked().isEmpty());
        e.win.w.setMarkedFinal(true);
        QTRY_VERIFY(e.win.w.isReadOnly());
        QLabel *note = e.win.w.findChild<QLabel *>(QStringLiteral("finalNote"));
        QVERIFY(note);
        QVERIFY(note->isVisibleTo(&e.win.w));
        QVERIFY(note->text().contains(QLatin1String("final")));
        e.win.w.setMarkedFinal(false);
        QVERIFY(!note->isVisibleTo(&e.win.w));
    }

    void theInfoPageHasItsCardsAndEditsProperties()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.showFileScreen(QStringLiteral("info"));
        auto *screen = e.win.w.findChild<FileScreen *>();
        QVERIFY(screen);
        QCOMPARE(screen->currentPage(), QStringLiteral("info"));
        QWidget *page = screen->findChild<QWidget *>(QStringLiteral("infoPage"));
        QVERIFY(page);
        for (const char *name : {"infoMarkFinal", "infoEncrypt", "infoProtectSheet", "infoProtectStructure", "infoInspect", "infoCompatibility", "infoCustom", "infoShowFolder", "infoCopyPath"})
            QVERIFY2(page->findChild<QWidget *>(QString::fromLatin1(name)), name);
        auto *title = page->findChild<QLineEdit *>(QStringLiteral("infoTitle"));
        QVERIFY(title);
        title->setText(QStringLiteral("From the page"));
        Q_EMIT title->editingFinished();
        QCOMPARE(e.win.w.properties().title, QStringLiteral("From the page"));
        QVERIFY(e.win.w.isWindowModified());
        e.win.w.hideFileScreen();
    }
};

JP_TEST_CLASS(CrunchInfoTests)
#include "tst_crunch_info.moc"
