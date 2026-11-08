// JeffPub's tests of its Info page: the properties a .jpub keeps, Mark as Final, Inspect Publication, and Version History. Every file is made here, in a temporary folder.
#include "pubhelpers.h"
#include "../isolatesettings.h"
#include "app/backstage.h"
#include "app/infopage.h"
#include "office/packageprops.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTemporaryDir>

namespace {

// A window that says what it would have shown in a box.
struct Quiet {
    explicit Quiet(jp::MainWindow &w) { w.setNotifier([this](const QString &, const QString &text) { said << text; }); }
    QStringList said;
};

const jp::InspectItem *find(const QVector<jp::InspectItem> &items, const char *title)
{
    for (const jp::InspectItem &i : items)
        if (i.title == QLatin1String(title)) return &i;
    return nullptr;
}

QStringList labelTexts(QWidget *page)
{
    QStringList out;
    for (const QLabel *l : page->findChildren<QLabel *>()) out << l->text();
    return out;
}

} // namespace

class PubInfoTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }
    void init() { jp::Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Tester")); }

    // Every property the Properties page and its dialog showed: they are edited on the Info page now, kept with the publication, and come back from the file.
    void propertiesStillRoundTripThroughASave()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow a;
        Quiet quiet(a);
        office::PackageProps p = a.properties();
        p.title = QStringLiteral("Spring flyer");
        p.creator = QStringLiteral("Ann");
        p.subject = QStringLiteral("Sale");
        p.keywords = QStringLiteral("spring, sale");
        p.category = QStringLiteral("Marketing");
        p.comments = QStringLiteral("For the print shop");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo");
        office::CustomProperty c;
        c.name = QStringLiteral("Job");
        c.type = QStringLiteral("lpwstr");
        c.value = QStringLiteral("1234");
        p.custom.push_back(c);
        a.applyProperties(p);
        QVERIFY(a.editor()->isModified());
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("p.jpub"))));
        jp::MainWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("p.jpub"))));
        const office::PackageProps q = b.properties();
        QCOMPARE(q.title, p.title);
        QCOMPARE(q.creator, p.creator);
        QCOMPARE(q.subject, p.subject);
        QCOMPARE(q.keywords, p.keywords);
        QCOMPARE(q.category, p.category);
        QCOMPARE(q.comments, p.comments);
        QCOMPARE(q.company, p.company);
        QCOMPARE(q.manager, p.manager);
        QCOMPARE(q.custom.size(), 1);
        QCOMPARE(q.custom.first().value, QStringLiteral("1234"));
        QVERIFY(q.created.isValid());
        QVERIFY(q.modified.isValid());
    }

    void theInfoPageHasItsCardsAndTheFactsThePropertiesPageHad()
    {
        jp::MainWindow w;
        Quiet quiet(w);
        w.resize(1400, 900);
        w.show();
        w.showBackstage(QStringLiteral("info"));
        jp::Backstage *backstage = w.findChild<jp::Backstage *>();
        QVERIFY(backstage && backstage->isVisible());
        QCOMPARE(backstage->currentPage(), QStringLiteral("info"));
        // the page is called Info in the side list, as in the other apps
        QStringList entries;
        for (QAbstractButton *b : backstage->findChildren<QAbstractButton *>())
            if (b->parent() && b->parent()->objectName() == QLatin1String("jpSide")) entries << b->text();
        QVERIFY(entries.contains(QStringLiteral("Info")));
        QVERIFY(!entries.contains(QStringLiteral("Properties")));
        QWidget *page = backstage->findChild<QWidget *>(QStringLiteral("infoPage"));
        QVERIFY(page);
        for (const char *name : {"infoMarkFinal", "infoInspect", "infoVersions", "infoCustom", "infoShowFolder", "infoCopyPath", "infoBusiness", "infoPrintSettings", "infoDesignChecker"})
            QVERIFY2(page->findChild<QWidget *>(QString::fromLatin1(name)), name);
        // a .jpub file has no form for a password, and JeffPub has no accessibility check: no cards for them
        QVERIFY(!page->findChild<QWidget *>(QStringLiteral("infoEncrypt")));
        QVERIFY(!page->findChild<QWidget *>(QStringLiteral("infoAccessibility")));
        const QStringList texts = labelTexts(page);
        for (const QString &want : {QStringLiteral("Pages"), QStringLiteral("Page size"), QStringLiteral("Pictures"), QStringLiteral("Words"), QStringLiteral("Created"), QStringLiteral("Last modified")})
            QVERIFY2(texts.contains(want), qPrintable(want));
        auto *title = page->findChild<QLineEdit *>(QStringLiteral("infoTitle"));
        QVERIFY(title);
        title->setText(QStringLiteral("From the page"));
        Q_EMIT title->editingFinished();
        QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("From the page"));
        QVERIFY(w.editor()->isModified());
        w.hideBackstage();
    }

    void markedAsFinalOpensForReadingOnly()
    {
        QTemporaryDir dir;
        jp::MainWindow a;
        Quiet quiet(a);
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("f.jpub"))));
        a.setMarkedFinal(true);
        QVERIFY(a.isReadOnly());
        QVERIFY(a.markedAsFinal());
        QVERIFY(!a.editor()->isModified());
        jp::MainWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("f.jpub"))));
        QVERIFY(b.isReadOnly());
        QVERIFY(b.statusBar()->currentMessage().contains(QLatin1String("Marked as final")));
        b.setMarkedFinal(false);
        QVERIFY(!b.isReadOnly());
        QVERIFY(!b.markedAsFinal());
        jp::MainWindow c;
        Quiet quietC(c);
        QVERIFY(c.openFile(dir.filePath(QStringLiteral("f.jpub")), jp::OpenHow::Copy));
        QVERIFY(!c.isReadOnly());
        // a .pub file can't hold the mark: it says so, and a save to one leaves it out
        jp::MainWindow d;
        Quiet quietD(d);
        QVERIFY(d.saveTo(dir.filePath(QStringLiteral("g.pub"))));
        QVERIFY(!d.markFinalBlocked().isEmpty());
        d.setMarkedFinal(true);
        QVERIFY(!d.isReadOnly());
        QVERIFY(!quietD.said.isEmpty());
    }

    void inspectFindsAndRemovesWhatIsThere()
    {
        jp::MainWindow w;
        Quiet quiet(w);
        jp::Document *d = w.editor()->doc();
        d->props.author = QStringLiteral("Ann");
        d->props.company = QStringLiteral("Acme");
        if (d->biz.isEmpty()) d->biz.push_back(jp::BusinessInfo());
        d->biz[0].name = QStringLiteral("Acme Printing");
        d->biz[0].phone = QStringLiteral("555-0100");
        d->merge.path = QStringLiteral("/tmp/names.csv");
        d->merge.fields = QStringList{QStringLiteral("Name")};
        d->merge.rows = QVector<QStringList>{{QStringLiteral("Cy")}, {QStringLiteral("Di")}};
        d->merge.include = QVector<bool>{true, true};
        auto parked = std::make_shared<jp::ShapeItem>();
        parked->rect = QRectF(-300, 100, 80, 30);
        d->scratch.push_back(parked);
        const QVector<jp::InspectItem> items = w.inspectItems();
        QVERIFY(find(items, "Document properties and personal information"));
        QVERIFY(find(items, "Business information"));
        QVERIFY(find(items, "Mail merge data"));
        QVERIFY(find(items, "Objects on the scratch area"));
        QCOMPARE(find(items, "Document properties and personal information")->count(), 2);
        QCOMPARE(find(items, "Business information")->count(), 1);
        QCOMPARE(find(items, "Mail merge data")->count(), 2);
        QCOMPARE(find(items, "Objects on the scratch area")->count(), 1);
        QCOMPARE(find(items, "Objects on the scratch area")->remove(), 1);
        QVERIFY(d->scratch.empty());
        QCOMPARE(find(items, "Mail merge data")->remove(), 2);
        QVERIFY(d->merge.isEmpty());
        QCOMPARE(find(items, "Business information")->remove(), 1);
        QVERIFY(w.editor()->doc()->biz[0].name.isEmpty());
        QCOMPARE(find(items, "Document properties and personal information")->remove(), 2);
        QVERIFY(w.properties().creator.isEmpty());
        // the names stay out of later edits of the properties
        office::PackageProps p = w.properties();
        p.creator = QStringLiteral("Ann again");
        w.applyProperties(p);
        QVERIFY(w.properties().creator.isEmpty());
        for (const jp::InspectItem &i : items) QCOMPARE(i.count(), 0);
        jp::InspectDialog dialog(QStringLiteral("Inspect"), items, nullptr);
        QVERIFY(dialog.summaries().first().contains(QLatin1String("None found")));
    }

    void versionsAreKeptAndGoneBackTo()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        Quiet quiet(w);
        QVERIFY(!w.saveVersion(QStringLiteral("early")));   // (no file yet)
        QVERIFY(!quiet.said.isEmpty());
        w.editor()->doc()->props.title = QStringLiteral("Sample");
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("v.jpub"))));
        QVERIFY(w.saveVersion(QStringLiteral("First")));
        QCOMPARE(w.versions().size(), 1);
        QCOMPARE(w.versions().first().name, QStringLiteral("First"));
        w.editor()->change(QStringLiteral("x"), [&] { w.editor()->doc()->props.title = QStringLiteral("Second title"); });
        QVERIFY(w.restoreVersion(w.versions().first().file));
        QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("Sample"));
        QVERIFY(w.editor()->isModified());
        QCOMPARE(w.versions().size(), 2);
        QVERIFY(w.versions().first().name.startsWith(QLatin1String("Before restoring")));
    }
};

JP_TEST_CLASS(PubInfoTests)
#include "tst_pub_info.moc"
