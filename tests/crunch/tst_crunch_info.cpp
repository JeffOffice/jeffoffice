// Tests of JeffCrunch's Info page: the properties a workbook keeps (written at the next save, into parts the file may not have had), Mark as Final, and Inspect Workbook. Every workbook is
// built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/filescreen.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "office/packageprops.h"
#include "../localepin.h"

#include <QAction>
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
        for (const char *name : {"infoMarkFinal", "infoEncrypt", "infoInspect", "infoCustom", "infoShowFolder", "infoCopyPath"})
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
