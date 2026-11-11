// JeffPub's tests of its Info page: the properties a .jpub keeps, Mark as Final, Inspect Publication, and Version History. Every file is made here, in a temporary folder.
#include <QElapsedTimer>
#include "pubhelpers.h"
#include "../isolatesettings.h"
#include "app/backstage.h"
#include "app/filedialogs.h"
#include "app/infopage.h"
#include "app/passworddialogs.h"
#include "office/officecrypt.h"
#include "office/packageprops.h"
#include "../settingguard.h"
#include "../privacyhooks.h"

#include "io/zip.h"
#include "app/settings.h"

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QCheckBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <QtEndian>
#include <memory>

namespace {

using jptest::TestModeGuard;

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

const QByteArray kCompound("\xD0\xCF\x11\xE0", 4);

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// Asks for these passwords, in turn, when a protected file is opened (and counts the questions); then cancels.
void answerWith(const QStringList &passwords, int *asked = nullptr)
{
    auto remaining = std::make_shared<QStringList>(passwords);
    jp::setOpenPasswordStandIn([remaining, asked](const QString &, bool, QString *given) {
        if (asked) ++*asked;
        if (remaining->isEmpty()) return false;
        *given = remaining->takeFirst();
        return true;
    });
}

void choosePassword(jp::MainWindow &w, const QString &password)
{
    jp::setNewPasswordStandIn([password](bool, QString *given) {
        *given = password;
        return jp::NewPasswordChoice::Set;
    });
    w.encryptWithPassword();
}

// Answers the next question box (looking for it every 20 ms) with Yes or No; `said` gets its text.
struct BoxAnswer {
    BoxAnswer(bool yes, QString *said)
    {
        auto *t = new QTimer(qApp);
        QObject::connect(t, &QTimer::timeout, t, [t, yes, said] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                if (said) *said = box->text();
                QAbstractButton *b = box->button(yes ? QMessageBox::Yes : QMessageBox::No);
                if (!b) b = box->button(QMessageBox::Ok);
                if (b) b->click();
                else box->accept();
                t->deleteLater();
            }
        });
        t->start(20);
        m_timer = t;
    }
    ~BoxAnswer() { delete m_timer; }
    QPointer<QTimer> m_timer;
};

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

    // The option takes the names out of the file as it is saved, in a .jpub and in a .pub, and nothing the file says: all of the file but its properties is as it is saved without the option.
    void thePrivacyOptionTakesTheNamesOutOfTheFileAndKeepsTheContent()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow a;
        Quiet quiet(a);
        office::PackageProps p = a.properties();
        p.title = QStringLiteral("Spring flyer");
        p.creator = QStringLiteral("Ann Lee");
        p.lastModifiedBy = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo Ray");
        a.applyProperties(p);
        auto document = [&](const QString &name) {
            QMap<QString, QByteArray> parts;
            QFile f(dir.filePath(name));
            if (!f.open(QIODevice::ReadOnly) || !jp::readZip(f.readAll(), parts)) return QJsonObject();
            return QJsonDocument::fromJson(parts.value(QStringLiteral("document.json"))).object();
        };
        jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("named.jpub")), true));
        QCOMPARE(document(QStringLiteral("named.jpub")).value(QStringLiteral("props")).toObject().value(QStringLiteral("author")).toString(), QStringLiteral("Ann Lee"));
        jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("clean.jpub")), true));
        QJsonObject named = document(QStringLiteral("named.jpub")), clean = document(QStringLiteral("clean.jpub"));
        const QJsonObject props = clean.value(QStringLiteral("props")).toObject();
        for (const char *key : {"author", "lastModifiedBy", "company", "manager"}) QVERIFY2(props.value(QLatin1String(key)).toString().isEmpty(), key);
        QCOMPARE(props.value(QStringLiteral("title")).toString(), QStringLiteral("Spring flyer"));
        named.remove(QStringLiteral("props"));
        clean.remove(QStringLiteral("props"));
        QVERIFY(!clean.isEmpty());
        for (const QString &key : named.keys()) QVERIFY2(clean.value(key) == named.value(key), qPrintable(key));
        QCOMPARE(clean.keys().join(QLatin1Char(',')), named.keys().join(QLatin1Char(',')));
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("clean.pub")), true));
        jp::MainWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("clean.pub"))));
        const office::PackageProps q = b.properties();
        QVERIFY(q.creator.isEmpty() && q.lastModifiedBy.isEmpty() && q.company.isEmpty() && q.manager.isEmpty());
        QCOMPARE(b.editor()->doc()->pages.size(), a.editor()->doc()->pages.size());
    }

    // The open publication is not what is scrubbed: a copy, a save, and a save that fails leave its names, its history, and its mark as saved as they were.
    void thePrivacyOptionLeavesTheOpenPublicationAsItWas()
    {
        jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow a;
        Quiet quiet(a);
        office::PackageProps p = a.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.lastModifiedBy = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo Ray");
        a.applyProperties(p);
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("first.jpub"))));
        QVERIFY(!a.editor()->isModified());
        const int steps = a.editor()->undoStack()->count();
        auto unchanged = [&] {
            QCOMPARE(a.properties().creator, QStringLiteral("Ann Lee"));
            QCOMPARE(a.properties().lastModifiedBy, QStringLiteral("Tester"));   // (the person who edited it, as the properties were applied)
            QCOMPARE(a.properties().company, QStringLiteral("Acme"));
            QCOMPARE(a.properties().manager, QStringLiteral("Bo Ray"));
            QCOMPARE(a.editor()->undoStack()->count(), steps);
            QVERIFY(!a.editor()->isModified());
        };
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("copy.jpub")), true));
        unchanged();
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("copy.pub")), true));
        unchanged();
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("second.jpub"))));
        unchanged();
        {
            jptest::DismissBoxes boxes;
            QVERIFY(!a.saveTo(dir.filePath(QStringLiteral("missing/third.jpub"))));   // (a save that fails)
        }
        unchanged();
    }

    // The scope that leaves the names out while a file is written gives them back, whatever happens.
    void theNamesComeBackWhenTheScopeThatLeftThemOutEnds()
    {
        jp::DocProps props;
        props.author = QStringLiteral("Ann Lee");
        props.lastModifiedBy = QStringLiteral("Ann Lee");
        props.company = QStringLiteral("Acme");
        props.manager = QStringLiteral("Bo Ray");
        props.title = QStringLiteral("Kept");
        {
            jp::NamesOutScope off(props, false);
            QCOMPARE(props.author, QStringLiteral("Ann Lee"));
        }
        {
            jp::NamesOutScope scope(props, true);
            QVERIFY(props.author.isEmpty() && props.lastModifiedBy.isEmpty() && props.company.isEmpty() && props.manager.isEmpty());
            QCOMPARE(props.title, QStringLiteral("Kept"));
        }
        QCOMPARE(props.author, QStringLiteral("Ann Lee"));
        QCOMPARE(props.lastModifiedBy, QStringLiteral("Ann Lee"));
        QCOMPARE(props.company, QStringLiteral("Acme"));
        QCOMPARE(props.manager, QStringLiteral("Bo Ray"));
    }

    void withThePrivacyOptionOffASaveIsUnchanged()
    {
        jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow a;
        Quiet quiet(a);
        office::PackageProps p = a.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.lastModifiedBy = QStringLiteral("Ann Lee");
        a.applyProperties(p);
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("one.jpub")), true));
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("two.jpub")), true));
        auto props = [&](const QString &name) {
            QMap<QString, QByteArray> parts;
            QFile f(dir.filePath(name));
            if (!f.open(QIODevice::ReadOnly) || !jp::readZip(f.readAll(), parts)) return QJsonObject();
            return QJsonDocument::fromJson(parts.value(QStringLiteral("document.json"))).object().value(QStringLiteral("props")).toObject();
        };
        QCOMPARE(props(QStringLiteral("one.jpub")).value(QStringLiteral("author")).toString(), QStringLiteral("Ann Lee"));
        QCOMPARE(props(QStringLiteral("two.jpub")).value(QStringLiteral("lastModifiedBy")).toString(), QStringLiteral("Tester"));
    }

    // Nothing but the properties holds a name in a .pub or a .jpub: the whole of each file is without them (a .pub's strings are in 8 bits, and also in 16).
    void thePrivacyOptionKeepsNamesOutOfTheWholeOfAPubAndAJpubFile()
    {
        jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow a;
        Quiet quiet(a);
        office::PackageProps p = a.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo Ray");
        a.applyProperties(p);
        for (const char *name : {"all.jpub", "all.pub"}) {
            QVERIFY(a.saveTo(dir.filePath(QString::fromLatin1(name)), true));
            QFile f(dir.filePath(QString::fromLatin1(name)));
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray bytes = f.readAll();
            QMap<QString, QByteArray> parts;
            if (!jp::readZip(bytes, parts)) parts.insert(QStringLiteral("whole"), bytes);
            for (auto it = parts.cbegin(); it != parts.cend(); ++it)
                for (const char *who : {"Ann Lee", "Acme", "Bo Ray", "Tester"}) {
                    const QString w = QString::fromLatin1(who);
                    QVERIFY2(!it.value().contains(who), qPrintable(QString::fromLatin1(name) + QLatin1Char(' ') + it.key() + QStringLiteral(": ") + w));
                    QVERIFY2(!it.value().contains(QByteArray(reinterpret_cast<const char *>(w.utf16()), qsizetype(w.size() * 2))), qPrintable(QString::fromLatin1(name) + QStringLiteral(" UTF-16 ") + w));
                }
        }
    }

    void theInfoPageHasTheBoxForThePrivacyOption()
    {
        jp::MainWindow w;
        const QVariant priorSetting = jp::Settings::get().value(QStringLiteral("privacy/removeOnSave"), false);
        const auto restore = qScopeGuard([priorSetting] { jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), priorSetting); });
        jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        std::unique_ptr<QWidget> page(w.buildInfoPage(nullptr));
        auto *box = page->findChild<QCheckBox *>(QStringLiteral("infoRemovePersonal"));
        QVERIFY(box);
        QCOMPARE(box->text(), QStringLiteral("Remove personal information from this file when saving"));
        QVERIFY(!box->isChecked());
        box->setChecked(true);
        QVERIFY(jp::Settings::get().value(QStringLiteral("privacy/removeOnSave")).toBool());
    }

    // The command that was Properties is Info, as in the other apps, and opens the Info page.
    void thePropertiesCommandIsInfo()
    {
        jp::MainWindow w;
        Quiet quiet(w);
        w.resize(1400, 900);
        w.show();
        QCOMPARE(w.act(QStringLiteral("file.properties"))->text(), QStringLiteral("Info"));
        w.act(QStringLiteral("file.properties"))->trigger();
        jp::Backstage *backstage = w.findChild<jp::Backstage *>();
        QVERIFY(backstage && backstage->isVisible());
        QCOMPARE(backstage->currentPage(), QStringLiteral("info"));
        w.hideBackstage();
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
        // a password is for .jpub files; the accessibility check has its card, as in the other apps
        QVERIFY(page->findChild<QWidget *>(QStringLiteral("infoEncrypt")));
        QVERIFY(page->findChild<QWidget *>(QStringLiteral("infoAccessibility")));
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

    // Check Accessibility lists a picture with no alt text and goes to it when it is clicked; a picture given alt text, or marked as decoration, is not listed.
    void checkAccessibilityListsAPictureWithNoAltTextAndSelectsIt()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        jp::Editor *ed = w.editor();
        QImage img(40, 40, QImage::Format_RGB32);
        img.fill(Qt::red);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        const QString imageId = ed->doc()->addImage(png, "png");
        auto picture = [&](const QRectF &r) {
            auto p = std::make_shared<jp::PictureItem>();
            p->imageId = imageId;
            p->rect = r;
            p->imgRect = QRectF(QPointF(0, 0), r.size());
            ed->addItem(p, false);
            return p;
        };
        auto bare = picture(QRectF(72, 72, 100, 100));
        auto described = picture(QRectF(200, 72, 100, 100));
        described->altText = QStringLiteral("A red square");
        auto decoration = picture(QRectF(330, 72, 100, 100));
        decoration->decorative = true;
        ed->clearSelection();
        w.act(QStringLiteral("rev.accessibility"))->trigger();
        jp::PageAccessibilityPane *pane = w.accessibilityPane();
        QVERIFY(pane);
        QCOMPARE(pane->issueCount(), 1);
        QVERIFY(pane->list()->item(0)->text().startsWith(QLatin1String("Error: ")));
        QVERIFY(ed->selection().isEmpty());
        Q_EMIT pane->list()->itemActivated(pane->list()->item(0));
        QCOMPARE(ed->selection(), QStringList{bare->id});
        // alt text given: the list is empty after Check Again
        ed->change(QStringLiteral("alt"), [&] { ed->doc()->item(bare->id)->altText = QStringLiteral("A red square"); });
        pane->recheck();
        QCOMPARE(pane->issueCount(), 0);
    }

    // A table whose first row is not marked as a header row is a warning (a screen reader finds the column names in the header row); one with a header row is not listed.
    void aTableWithNoHeaderRowIsReported()
    {
        auto doc = std::make_shared<jp::Document>();
        doc->pages.clear();
        doc->pages.push_back(std::make_shared<jp::Page>());
        auto table = [&](double top) {
            auto t = std::make_shared<jp::TableItem>();
            t->rows = t->cols = 2;
            t->colW = {100, 100};
            t->rowH = {30, 30};
            t->cells.resize(4);
            t->rect = QRectF(40, top, 200, 60);
            return t;
        };
        auto bare = table(40);
        auto marked = table(200);
        bare->header = false;
        doc->pages[0]->items.push_back(bare);
        doc->pages[0]->items.push_back(marked);
        QVector<jp::PageAccessIssue> found;
        for (const auto &i : jp::checkPageAccessibility(*doc))
            if (i.kind == jp::PageAccessIssue::NoTableHeader) found << i;
        QCOMPARE(int(found.size()), 1);
        QCOMPARE(found[0].itemId, bare->id);
        QCOMPARE(found[0].page, 0);
        QCOMPARE(int(found[0].level), int(jp::PageAccessIssue::Warning));
        QVERIFY2(found[0].message.contains(QLatin1String("header row")), qPrintable(found[0].message));
    }

    // The pane lists it with a fix, "Use the first row as a header", which turns the header row on in one step that undoes; the list is empty after it.
    void theFixTurnsTheHeaderRowOnAndUndoes()
    {
        jp::MainWindow w;
        
        w.resize(1400, 900);
        w.show();
        jp::Editor *ed = w.editor();
        
        
        auto table = ed->newTable(QRectF(72, 72, 200, 60), 2, 2);
        auto *t = static_cast<jp::TableItem *>(table.get());
        t->header = false;
        ed->addItem(table, false);
        ed->clearSelection();
        w.act(QStringLiteral("rev.accessibility"))->trigger();
        jp::PageAccessibilityPane *pane = w.accessibilityPane();
        QVERIFY(pane);
        int row = -1;
        for (int i = 0; i < pane->list()->count(); ++i)
            if (pane->list()->item(i)->text().contains(QLatin1String("header row"))) row = i;
        QVERIFY(row >= 0);
        auto *fix = pane->findChild<QPushButton *>(QStringLiteral("accessFix"));
        QVERIFY(fix);
        QVERIFY(!fix->isEnabled());
        pane->list()->setCurrentRow(row);
        QVERIFY(fix->isEnabled());
        QCOMPARE(fix->text(), QStringLiteral("Use the first row as a header"));
        const int before = ed->undoStack()->index();
        fix->click();
        QVERIFY(static_cast<jp::TableItem *>(ed->doc()->item(table->id))->header);
        QCOMPARE(ed->undoStack()->index(), before + 1);
        for (int i = 0; i < pane->list()->count(); ++i) QVERIFY(!pane->list()->item(i)->text().contains(QLatin1String("header row")));
        ed->undoStack()->undo();
        QVERIFY(!static_cast<jp::TableItem *>(ed->doc()->item(table->id))->header);
        QCOMPARE(ed->undoStack()->index(), before);
        pane->recheck();
        bool listed = false;
        for (int i = 0; i < pane->list()->count(); ++i) listed = listed || pane->list()->item(i)->text().contains(QLatin1String("header row"));
        QVERIFY(listed);
    }

    // Many problems are cut at the cap, and at the number asked for.
    void theTableCheckKeepsToTheCap()
    {
        auto doc = std::make_shared<jp::Document>();
        doc->pages.clear();
        doc->pages.push_back(std::make_shared<jp::Page>());
        for (int n = 0; n < jp::kMaxPageAccessIssues + 20; ++n) {
            auto t = std::make_shared<jp::TableItem>();
            t->rows = t->cols = 1;
            t->colW = {50};
            t->rowH = {20};
            t->cells.resize(1);
            t->rect = QRectF(10, 10 + n * 2, 50, 20);
            t->header = false;
            t->altText = QStringLiteral("x");
            doc->pages[0]->items.push_back(t);
        }
        QCOMPARE(int(jp::checkPageAccessibility(*doc).size()), jp::kMaxPageAccessIssues);
        QCOMPARE(int(jp::checkPageAccessibility(*doc, false, {}, 7).size()), 7);
    }

    // Text is checked against its background: the contrast of the two colors must be 4.5 to 1 (3 to 1 for text of 18 points, or 14 and bold); the fill of its box, else the page's, is the
    // background, and a picture under it is for the person to look at.
    void textContrastIsCheckedAgainstItsBackground()
    {
        QVERIFY(std::abs(jp::contrastRatio(QColor(QStringLiteral("#777777")), Qt::white) - 4.478) < 0.01);
        QVERIFY(jp::contrastRatio(QColor(QStringLiteral("#777777")), Qt::white) < 4.5);
        QVERIFY(std::abs(jp::contrastRatio(QColor(QStringLiteral("#767676")), Qt::white) - 4.542) < 0.01);
        QVERIFY(jp::contrastRatio(QColor(QStringLiteral("#767676")), Qt::white) >= 4.5);
        QVERIFY(std::abs(jp::contrastRatio(Qt::black, Qt::white) - 21) < 0.001);
        QCOMPARE(jp::neededContrast(17.9, false), 4.5);
        QCOMPARE(jp::neededContrast(18, false), 3.0);
        QCOMPARE(jp::neededContrast(14, true), 3.0);
        QCOMPARE(jp::neededContrast(13.9, true), 4.5);
        QCOMPARE(jp::neededContrast(14, false), 4.5);
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->pages.clear();
        // one case to a page: the text's color, its size, bold, and the fill of its box (none: the page's white)
        struct Case { const char *color; double points; bool bold; const char *fill; bool flagged; };
        const QVector<Case> cases{{"#777777", 12, false, nullptr, true}, {"#767676", 12, false, nullptr, false}, {"#777777", 18, false, nullptr, false},
                                  {"#999999", 18, false, nullptr, true}, {"#888888", 14, true, nullptr, false}, {"#888888", 13, true, nullptr, true},
                                  {"#444444", 12, false, "#333333", true}, {"#FFFFFF", 12, false, "#333333", false}};
        for (const Case &c : cases) {
            doc->pages.push_back(std::make_shared<jp::Page>());
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(50, 100, 300, 60);
            t->storyId = doc->createStory(QStringLiteral("Some words to read"));
            QTextCursor cur(doc->storyDoc(t->storyId));
            cur.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setProperty(jp::tp::ColorRefP, QString::fromLatin1(c.color));
            f.setFontPointSize(c.points);
            f.setFontWeight(c.bold ? QFont::Bold : QFont::Normal);
            cur.mergeCharFormat(f);
            if (c.fill) t->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(QString::fromLatin1(c.fill))));
            doc->pages.back()->items.push_back(t);
        }
        // a picture under text that has no fill of its own
        doc->pages.push_back(std::make_shared<jp::Page>());
        auto pic = std::make_shared<jp::PictureItem>();
        pic->rect = QRectF(40, 90, 400, 200);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        pic->altText = QStringLiteral("A photo");
        doc->pages.back()->items.push_back(pic);
        auto over = std::make_shared<jp::TextItem>();
        over->rect = QRectF(60, 110, 300, 60);
        over->storyId = doc->createStory(QStringLiteral("Words over a picture"));
        doc->pages.back()->items.push_back(over);
        const QVector<jp::PageAccessIssue> found = jp::checkPageAccessibility(*doc);
        for (int i = 0; i < cases.size(); ++i) {
            int n = 0;
            for (const auto &issue : found)
                if (issue.page == i && issue.kind == jp::PageAccessIssue::LowContrast) {
                    ++n;
                    QCOMPARE(int(issue.level), int(jp::PageAccessIssue::Warning));
                    QVERIFY(issue.message.contains(QLatin1String("contrast")));
                }
            QVERIFY2(n == (cases[i].flagged ? 1 : 0), qPrintable(QStringLiteral("case %1 (%2): %3 found").arg(i).arg(QString::fromLatin1(cases[i].color)).arg(n)));
        }
        int tips = 0;
        for (const auto &issue : found)
            if (issue.page == cases.size() && issue.kind == jp::PageAccessIssue::LowContrast) {
                ++tips;
                QCOMPARE(int(issue.level), int(jp::PageAccessIssue::Tip));
                QVERIFY(issue.message.contains(QLatin1String("Check contrast by eye")));
            }
        QCOMPARE(tips, 1);
    }

    // Objects read in another order than top to bottom, left to right are listed as tips, and the Reading Order list of the checker moves them.
    void objectsReadOutOfPlaceAreListedAndReorderedInTheReadingOrderView()
    {
        jp::MainWindow w;
        Quiet quiet(w);
        w.show();
        jp::Editor *ed = w.editor();
        {
            auto d = jp::Document::blank(QSizeF(612, 792));
            d->pages.push_back(std::make_shared<jp::Page>());
            ed->setDocument(std::move(d));
        }
        auto page = ed->doc()->pages[0];
        page->items.clear();
        auto box = [&](const QString &words, const QRectF &r) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = r;
            t->storyId = ed->doc()->createStory(words);
            page->items.push_back(t);
            return t;
        };
        auto right = box(QStringLiteral("Right"), QRectF(300, 100, 200, 40));   // read first, though it is the second by place
        auto left = box(QStringLiteral("Left"), QRectF(50, 100, 200, 40));
        auto below = box(QStringLiteral("Below"), QRectF(50, 300, 200, 40));
        ed->setCurrentPage(0);
        auto orderTips = [&]() {
            QStringList ids;
            for (const auto &i : jp::checkPageAccessibility(*ed->doc()))
                if (i.kind == jp::PageAccessIssue::ReadingOrder && i.level == jp::PageAccessIssue::Tip) ids << i.itemId;
            return ids;
        };
        const QStringList tips = orderTips();
        QCOMPARE(tips.size(), 1);
        QVERIFY(tips == QStringList{right->id} || tips == QStringList{left->id});
        // in order by place: none
        {
            auto saved = page->items;
            page->items = {left, right, below};
            QVERIFY(orderTips().isEmpty());
            page->items = saved;
        }
        // the view of the checker lists them in reading order, and moving one changes the order
        w.act(QStringLiteral("rev.accessibility"))->trigger();
        jp::PageAccessibilityPane *pane = w.accessibilityPane();
        QVERIFY(pane);
        auto *button = pane->findChild<QPushButton *>(QStringLiteral("accessReadingOrder"));
        QVERIFY(button);
        QCOMPARE(button->text(), QStringLiteral("Reading Order"));
        button->click();
        jp::SelectionPane *view = pane->readingOrderPane();
        QVERIFY(view);
        QCOMPARE(view->windowTitle(), QStringLiteral("Reading Order"));
        auto *note = view->findChild<QLabel *>(QStringLiteral("selectionNote"));
        QVERIFY2(note, "no note in the Reading Order list");
        QCOMPARE(note->text(), QStringLiteral("Changing the reading order also changes which object is in front."));
        auto *list = view->findChild<QListWidget *>(QStringLiteral("selectionList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 3);
        list->setCurrentRow(0);   // Right
        QCOMPARE(ed->selection(), QStringList{right->id});
        auto *later = view->findChild<QPushButton *>(QStringLiteral("selectionBackward"));
        QVERIFY(later);
        QCOMPARE(later->text(), QStringLiteral("Read Later"));
        later->click();
        QCOMPARE(ed->doc()->pages[0]->items[0]->id, left->id);
        QCOMPARE(ed->doc()->pages[0]->items[1]->id, right->id);
        QVERIFY(orderTips().isEmpty());
        // Undo puts the order back
        ed->undo();
        QCOMPARE(ed->doc()->pages[0]->items[0]->id, right->id);
    }

    // Text is judged where it is: over the fill of a master page's shape, over a shape under only part of it, with its own see-through color and its drawn size, and in a group; the
    // contrast is shown cut, not rounded; and stacks of objects cost no more than their number.
    void textContrastLooksAtMastersPlacesAlphaSizeAndGroups()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->pages.clear();
        auto master = std::make_shared<jp::MasterPage>();
        master->id = QStringLiteral("tm");
        auto band = std::make_shared<jp::ShapeItem>();
        band->rect = QRectF(0, 0, 612, 200);
        band->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(QStringLiteral("#222222"))));
        master->items.push_back(band);
        doc->masters.push_back(master);
        auto story = [&](const QString &color, double points) {
            const QString id = doc->createStory(QStringLiteral("Some words to read"));
            QTextCursor cur(doc->storyDoc(id));
            cur.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setProperty(jp::tp::ColorRefP, color);
            f.setFontPointSize(points);
            cur.mergeCharFormat(f);
            return id;
        };
        auto page = [&](const QString &masterId = QString()) {
            auto pg = std::make_shared<jp::Page>();
            pg->masterId = masterId;
            doc->pages.push_back(pg);
            return pg;
        };
        auto box = [&](const QString &color, double points, const QRectF &r = QRectF(50, 50, 300, 60)) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = r;
            t->storyId = story(color, points);
            return t;
        };
        page(QStringLiteral("tm"))->items.push_back(box(QStringLiteral("#FFFFFF"), 12));   // 0: white on the master's dark band: fine
        page(QStringLiteral("tm"))->items.push_back(box(QStringLiteral("#000000"), 12));   // 1: black on it: not
        {
            auto pg = page(QStringLiteral("tm"));   // 2: the page hides the master's objects: black on white
            pg->showMasterItems = false;
            pg->items.push_back(box(QStringLiteral("#000000"), 12));
        }
        {
            auto pg = page();   // 3: a dark shape under the left part of a wide box
            auto dark = std::make_shared<jp::ShapeItem>();
            dark->rect = QRectF(50, 50, 100, 60);
            dark->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(QStringLiteral("#111111"))));
            pg->items.push_back(dark);
            pg->items.push_back(box(QStringLiteral("#000000"), 12, QRectF(50, 50, 400, 60)));
        }
        page()->items.push_back(box(QStringLiteral("#66000000"), 12));   // 4: black at 40% opacity on white
        {
            auto t = box(QStringLiteral("#888888"), 18);   // 5: large, but drawn at half size
            t->autofit = jp::TextItem::ShrinkOnOverflow;
            t->fitScale = 0.5;
            page()->items.push_back(t);
        }
        page()->items.push_back(box(QStringLiteral("#888888"), 18));   // 6: the same, drawn at its size
        {
            auto g = std::make_shared<jp::GroupItem>();   // 7: in a group
            g->children.push_back(box(QStringLiteral("#999999"), 12));
            g->rect = g->children.front()->rect;
            page()->items.push_back(g);
        }
        page()->items.push_back(box(QStringLiteral("#77767C"), 12));   // 8: 4.4998 to 1
        {
            auto pg = page();   // 9: thirty layers with no fill under the words
            for (int i = 0; i < 30; ++i) {
                auto layer = std::make_shared<jp::ShapeItem>();
                layer->rect = QRectF(40 + i, 40 + i, 400, 100);
                layer->altText = QStringLiteral("layer");
                pg->items.push_back(layer);
            }
            pg->items.push_back(box(QStringLiteral("#999999"), 12, QRectF(80, 80, 300, 60)));
        }
        QElapsedTimer clock;
        clock.start();
        const QVector<jp::PageAccessIssue> found = jp::checkPageAccessibility(*doc);
        QVERIFY2(clock.elapsed() < 3000, qPrintable(QStringLiteral("%1 ms").arg(clock.elapsed())));
        auto count = [&](int pageNumber) {
            int n = 0;
            for (const auto &i : found)
                if (i.page == pageNumber && i.kind == jp::PageAccessIssue::LowContrast && i.level == jp::PageAccessIssue::Warning) ++n;
            return n;
        };
        const QVector<int> flagged{0, 1, 0, 1, 1, 1, 0, 1, 1, 1};
        for (int i = 0; i < flagged.size(); ++i) QVERIFY2(count(i) == flagged[i], qPrintable(QStringLiteral("page %1: %2 found, %3 wanted").arg(i).arg(count(i)).arg(flagged[i])));
        for (const auto &i : found)
            if (i.page == 8 && i.kind == jp::PageAccessIssue::LowContrast) {
                QVERIFY2(i.message.contains(QLatin1String("4.49 to 1")), qPrintable(i.message));
                QVERIFY(!i.message.contains(QLatin1String("4.50")));
            }
    }

    // Reading order: a picture over most of the page is a backdrop and is left out; objects whose spans overlap by half are a row, however their tops differ; and a real swap is
    // a tip on the object that is read too early.
    void readingOrderTipsAreNotNoisyAndPinTheRightObject()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->pages.clear();
        auto text = [&](const QRectF &r) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = r;
            t->storyId = doc->createStory(QStringLiteral("Words"));
            return t;
        };
        auto tipsOn = [&](int pageNumber) {
            QStringList ids;
            for (const auto &i : jp::checkPageAccessibility(*doc))
                if (i.page == pageNumber && i.kind == jp::PageAccessIssue::ReadingOrder && i.level == jp::PageAccessIssue::Tip) ids << i.itemId;
            return ids;
        };
        // 0: words over a backdrop picture that is listed after them
        auto backdrop = std::make_shared<jp::PictureItem>();
        backdrop->rect = QRectF(0, 0, 612, 792);
        backdrop->imgRect = QRectF(0, 0, 612, 792);
        backdrop->altText = QStringLiteral("A background");
        doc->pages.push_back(std::make_shared<jp::Page>());
        doc->pages[0]->items = {text(QRectF(50, 300, 200, 40)), backdrop};
        QVERIFY(tipsOn(0).isEmpty());
        // 1: two columns, the left one read first and 20 points lower
        doc->pages.push_back(std::make_shared<jp::Page>());
        doc->pages[1]->items = {text(QRectF(50, 120, 200, 100)), text(QRectF(300, 100, 200, 100))};
        QVERIFY(tipsOn(1).isEmpty());
        // 2: right read first, then left, then the one below: the right one is the one out of place
        doc->pages.push_back(std::make_shared<jp::Page>());
        auto right = text(QRectF(300, 100, 200, 40)), left = text(QRectF(50, 100, 200, 40)), below = text(QRectF(50, 300, 200, 40));
        doc->pages[2]->items = {right, left, below};
        QCOMPARE(tipsOn(2), QStringList{right->id});
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

    // Mark as Final on a publication that is then saved as a .pub file, which can't hold the mark, leaves the window open for editing.
    void markAsFinalDoesNotHoldAWindowWhoseFileCantKeepTheMark()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        Quiet quiet(w);
        const QString path = dir.filePath(QStringLiteral("m.pub"));
        jp::setSavePathStandIn([&](const QString &, const QString &, const QString &, QString *) { return path; });
        w.setMarkedFinal(true);
        jp::setSavePathStandIn({});
        QVERIFY(QFileInfo::exists(path));
        QVERIFY(!w.markedAsFinal());
        QVERIFY(!w.isReadOnly());
        w.editor()->markSaved();
    }

    void cleanup()
    {
        jp::setNewPasswordStandIn(nullptr);
        jp::setOpenPasswordStandIn(nullptr);
    }

    // The password card is for .jpub files: on for a publication that is or will be one, off, with a reason, for a .pub file.
    void thePasswordCardIsOnlyForJpubFiles()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        Quiet quiet(w);
        QVERIFY(w.encryptBlocked().isEmpty());
        w.resize(1400, 900);
        w.show();
        w.showBackstage(QStringLiteral("info"));
        QWidget *card = w.findChild<jp::Backstage *>()->findChild<QWidget *>(QStringLiteral("infoEncrypt"));
        QVERIFY(card && card->isEnabled());
        w.hideBackstage();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("p.pub"))));
        QVERIFY(w.encryptBlocked().contains(QLatin1String(".jpub")));
        w.showBackstage(QStringLiteral("info"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);   // (the page it replaced goes)
        card = w.findChild<jp::Backstage *>()->findChild<QWidget *>(QStringLiteral("infoEncrypt"));
        QVERIFY(card);
        QVERIFY(!card->isEnabled());
        QVERIFY(card->toolTip().contains(QLatin1String(".jpub")));
        w.hideBackstage();
        // (asking for one anyway changes nothing)
        choosePassword(w, QStringLiteral("hunter2"));
        QVERIFY(!w.isPasswordProtected());
        w.editor()->markSaved();
    }

    // A password is put on when the publication is saved: the file is a protected container, it asks for the password, a wrong one asks again, Cancel stops, and the password can be taken off.
    void aPasswordProtectsAJpubAndItOpensAgain()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("s.jpub"));
        {
            jp::MainWindow a;
            Quiet quiet(a);
            a.editor()->doc()->props.title = QStringLiteral("Secret flyer");
            choosePassword(a, QStringLiteral("hunter2"));
            QVERIFY(a.isPasswordProtected());
            QVERIFY(a.saveTo(path));
            QVERIFY(a.isPasswordProtected());
        }
        const QByteArray bytes = fileBytes(path);
        QVERIFY(bytes.startsWith(kCompound));
        QVERIFY(!bytes.contains("document.json"));
        QByteArray info, package;
        QVERIFY(office::splitProtectedPackage(bytes, &info, &package));
        // cancelled: the file does not open
        int asked = 0;
        answerWith({}, &asked);
        {
            jp::MainWindow b;
            Quiet quiet(b);
            QVERIFY(!b.openFile(path));
            QCOMPARE(asked, 1);
            QVERIFY(b.editor()->filePath().isEmpty());
        }
        // a wrong password asks again; then the right one opens it
        asked = 0;
        answerWith({QStringLiteral("nope"), QStringLiteral("hunter2")}, &asked);
        {
            jp::MainWindow c;
            Quiet quiet(c);
            QVERIFY(c.openFile(path));
            QCOMPARE(asked, 2);
            QVERIFY(c.isPasswordProtected());
            QCOMPARE(c.editor()->doc()->props.title, QStringLiteral("Secret flyer"));
            // saved again, it stays protected (with no question asked)
            QVERIFY(c.saveTo(path));
            QVERIFY(fileBytes(path).startsWith(kCompound));
            // the password taken off: the next save is a plain ZIP
            jp::setNewPasswordStandIn([](bool has, QString *) { return has ? jp::NewPasswordChoice::Removed : jp::NewPasswordChoice::Cancelled; });
            c.encryptWithPassword();
            QVERIFY(!c.isPasswordProtected());
            QVERIFY(c.saveTo(path));
            QVERIFY(fileBytes(path).startsWith("PK"));
        }
        // a plain file opens with no question
        asked = 0;
        answerWith({}, &asked);
        jp::MainWindow d;
        Quiet quiet(d);
        QVERIFY(d.openFile(path));
        QCOMPARE(asked, 0);
        QVERIFY(!d.isPasswordProtected());
    }

    // Versions, "Always create backup copy", and Save a Copy of a protected publication are protected too; no recovery copy of it is written in the clear.
    void versionsBackupsAndCopiesStayProtected()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("v.jpub"));
        jp::MainWindow w;
        Quiet quiet(w);
        w.editor()->doc()->props.title = QStringLiteral("Version one");
        QVERIFY(w.saveTo(path));   // (a plain file)
        choosePassword(w, QStringLiteral("hunter2"));
        QVERIFY(w.saveVersion(QStringLiteral("One")));
        const QVector<write::DocVersion> list = w.versions();
        QCOMPARE(list.size(), 1);
        QVERIFY(write::VersionStore(path, QStringLiteral(".jpub")).read(list.first().file).startsWith(kCompound));
        {
            jptest::SettingGuard keep(QStringLiteral("save/backup"), true, false);
            QVERIFY(w.saveTo(path));
        }
        // (the plain file on disk was backed up protected; the file is protected now, and the next backup of it keeps its password)
        const QString backup = dir.filePath(QStringLiteral("Backup of v.jpub"));
        QVERIFY(fileBytes(backup).startsWith(kCompound));
        QVERIFY(fileBytes(path).startsWith(kCompound));
        QFile::remove(backup);
        {
            jptest::SettingGuard keep(QStringLiteral("save/backup"), true, false);
            QVERIFY(w.saveTo(path));
        }
        QVERIFY(fileBytes(backup).startsWith(kCompound));
        answerWith({QStringLiteral("hunter2")});
        {
            jp::MainWindow b;
            Quiet quietB(b);
            QVERIFY(b.openFile(backup));
            QCOMPARE(b.editor()->doc()->props.title, QStringLiteral("Version one"));
        }
        jp::setOpenPasswordStandIn(nullptr);
        const QString copy = dir.filePath(QStringLiteral("c.jpub"));
        QVERIFY(w.saveTo(copy, true));
        QVERIFY(fileBytes(copy).startsWith(kCompound));
        // going back to the version opens it with the password
        w.editor()->doc()->props.title = QStringLiteral("Changed since");
        QVERIFY(w.restoreVersion(list.first().file));
        QVERIFY(w.isPasswordProtected());
        QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("Version one"));
        // a recovery copy would hold the text in the clear: none is written
        w.editor()->markUnsaved();
        w.autoRecover();
        QVERIFY(w.recoveryCopy().isEmpty());
        w.editor()->markSaved();
    }

    // A protected publication saved as a .pub file: asked first; Yes saves it without the password, No writes nothing.
    void savingAProtectedPublicationAsPubAsksFirst()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        Quiet quiet(w);
        const QString jpub = dir.filePath(QStringLiteral("k.jpub")), pub = dir.filePath(QStringLiteral("k.pub"));
        QVERIFY(w.saveTo(jpub));
        choosePassword(w, QStringLiteral("hunter2"));
        QString said;
        {
            BoxAnswer no(false, &said);
            QVERIFY(!w.saveTo(pub));
        }
        QVERIFY(said.contains(QLatin1String(".pub")));
        QVERIFY(!QFileInfo::exists(pub));
        QVERIFY(w.isPasswordProtected());
        {
            BoxAnswer yes(true, nullptr);
            QVERIFY(w.saveTo(pub));
        }
        QVERIFY(QFileInfo::exists(pub));
        QVERIFY(!w.isPasswordProtected());
        QVERIFY(!fileBytes(pub).contains("EncryptedPackage"));
        w.editor()->markSaved();
    }

    // A container made to tie up the reader (a package that says it is enormous, one cut short, key data that is nonsense) fails with a message, once, and asks for no memory it declares.
    void aHostileProtectedFileFailsCleanly()
    {
        QTemporaryDir dir;
        jp::MainWindow src;
        const QByteArray zip = jp::publicationBytes(*src.editor()->doc(), QImage());
        QVERIFY(!zip.isEmpty());
        office::EncryptOptions quick;
        quick.spinCount = 50;
        const QByteArray good = office::encryptPackage(zip, QStringLiteral("pw"), quick);
        QVERIFY(good.startsWith(kCompound));
        QByteArray size(8, '\0');
        qToLittleEndian<qint64>(zip.size(), size.data());
        const qsizetype at = good.indexOf(size);
        QVERIFY(at > 0);
        QByteArray huge = good;
        qToLittleEndian<qint64>(Q_INT64_C(0x7FFFFFFFFFFFFF00), huge.data() + at);
        const QByteArray cut = good.left(good.size() / 2);
        QByteArray noise = good;
        for (qsizetype i = 512; i < noise.size() && i < 1536; ++i) noise[i] = char(noise[i] ^ 0x5A);
        int n = 0;
        for (const QByteArray &bad : {huge, cut, noise}) {
            const QString path = dir.filePath(QStringLiteral("h%1.jpub").arg(n++));
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(bad);
            f.close();
            int asked = 0;
            answerWith({QStringLiteral("pw"), QStringLiteral("pw"), QStringLiteral("pw")}, &asked);
            jp::MainWindow w;
            Quiet quiet(w);
            QVERIFY(!w.openFile(path));
            QVERIFY(asked <= 3);
            if (n == 1) QVERIFY(asked >= 1);   // (the one that still reads as a protected file is asked about)
            QVERIFY(w.editor()->filePath().isEmpty());
            QVERIFY(!quiet.said.isEmpty());
        }
    }

    // A protected container whose package is not a ZIP (the password is right) fails with the reason, once, and makes nothing.
    void aProtectedFileHoldingNoZipSaysSo()
    {
        QTemporaryDir dir;
        office::EncryptOptions quick;
        quick.spinCount = 50;
        const QString path = dir.filePath(QStringLiteral("nozip.jpub"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(office::encryptPackage(QByteArray("this is not a zip file at all"), QStringLiteral("pw"), quick));
        f.close();
        int asked = 0;
        answerWith({QStringLiteral("pw")}, &asked);
        jp::MainWindow w;
        Quiet quiet(w);
        QVERIFY(!w.openFile(path));
        QCOMPARE(asked, 1);
        QCOMPARE(quiet.said.size(), 1);
        QVERIFY2(quiet.said.first().contains(QLatin1String("can't open \"nozip.jpub\"")), qPrintable(quiet.said.first()));
        QVERIFY2(quiet.said.first().contains(QLatin1String("ZIP"), Qt::CaseInsensitive) || quiet.said.first().contains(QLatin1String("zip")), qPrintable(quiet.said.first()));
        QVERIFY(w.editor()->filePath().isEmpty());
    }

    // The content of a protected publication that goes into a window of its own (Change Template, in a new window) is protected there too: its saves and recovery copies follow.
    void aNewWindowMadeFromAProtectedPublicationIsProtected()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        Quiet quiet(w);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("o.jpub"))));
        choosePassword(w, QStringLiteral("hunter2"));
        QPointer<jp::MainWindow> target = w.changeTemplate(jp::Document::blank(QSizeF(612, 792)), true);
        QVERIFY(target);
        QVERIFY(target->isPasswordProtected());
        target->autoRecover();
        QVERIFY(target->recoveryCopy().isEmpty());
        const QString saved = dir.filePath(QStringLiteral("n.jpub"));
        QVERIFY(target->saveTo(saved));
        QVERIFY(fileBytes(saved).startsWith(kCompound));
        target->editor()->markSaved();
        delete target.data();
        w.editor()->markSaved();
    }

    // A recovery copy written before the password was set is in the clear: it goes when the password is put on.
    void aPasswordRemovesTheRecoveryCopyWrittenBefore()
    {
        TestModeGuard testMode;
        jp::MainWindow w;
        Quiet quiet(w);
        w.editor()->change(QStringLiteral("x"), [&] { w.editor()->doc()->props.title = QStringLiteral("Draft"); });
        w.autoRecover();
        const QString copy = w.recoveryCopy();
        QVERIFY(!copy.isEmpty());
        QVERIFY(QFileInfo::exists(copy));
        choosePassword(w, QStringLiteral("hunter2"));
        QVERIFY(w.isPasswordProtected());
        QVERIFY(!QFileInfo::exists(copy));
        QVERIFY(w.recoveryCopy().isEmpty());
        w.editor()->markSaved();
    }

    // Open and Repair of a file with a password says so, and asks nothing.
    void aFileWithAPasswordCannotBeRepairedAndSaysSo()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("r.jpub"));
        {
            jp::MainWindow a;
            Quiet quiet(a);
            choosePassword(a, QStringLiteral("hunter2"));
            QVERIFY(a.saveTo(path));
        }
        int asked = 0;
        answerWith({QStringLiteral("hunter2")}, &asked);
        jp::MainWindow w;
        Quiet quiet(w);
        QVERIFY(!w.openFile(path, jp::OpenHow::Repair));
        QCOMPARE(asked, 0);
        QCOMPARE(quiet.said.size(), 1);
        QVERIFY2(quiet.said.first().contains(QLatin1String("password")), qPrintable(quiet.said.first()));
        QVERIFY2(quiet.said.first().contains(QLatin1String("can't be repaired")), qPrintable(quiet.said.first()));
    }

    // Mark as Final is kept in a .jpub file, protected or not: saved, reopened read-only with the note, and the mark can be taken off.
    void markAsFinalSurvivesAProtectedSaveAndReopen()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("fin.jpub"));
        {
            jp::MainWindow a;
            Quiet quiet(a);
            QVERIFY(a.saveTo(path));
            choosePassword(a, QStringLiteral("hunter2"));
            a.setMarkedFinal(true);
            QVERIFY(a.markedAsFinal());
            QVERIFY(a.isReadOnly());
        }
        QVERIFY(fileBytes(path).startsWith(kCompound));
        answerWith({QStringLiteral("hunter2")});
        jp::MainWindow b;
        Quiet quiet(b);
        QVERIFY(b.openFile(path));
        QVERIFY(b.markedAsFinal());
        QVERIFY(b.isReadOnly());
        QVERIFY(b.isPasswordProtected());
        b.setMarkedFinal(false);
        QVERIFY(!b.isReadOnly());
    }

    // A template made from a protected publication is protected too (a template is a file somebody can read). It shows in the New page's My Templates; making a publication from it asks for the
    // password as Open does: a wrong one asks again, and Cancel makes nothing. The new publication is protected, as a copy of a protected file is.
    void aTemplateOfAProtectedPublicationAsksForThePasswordWhenUsed()
    {
        TestModeGuard testMode;
        const QString tdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/Templates");
        QDir(tdir).removeRecursively();
        const auto cleanTemplates = qScopeGuard([&] { QDir(tdir).removeRecursively(); });
        QTemporaryDir dir;
        {
            jp::MainWindow w;
            Quiet quiet(w);
            w.editor()->doc()->props.title = QStringLiteral("Club flyer");
            QVERIFY(w.saveTo(dir.filePath(QStringLiteral("t.jpub"))));
            choosePassword(w, QStringLiteral("hunter2"));
            jp::setSavePathStandIn([&](const QString &, const QString &, const QString &, QString *selected) {
                *selected = QCoreApplication::translate("jp::MainWindow", "JeffPub Template (*.jpub)");
                return dir.filePath(QStringLiteral("pwtemplate-test.jpub"));
            });
            const auto reset = qScopeGuard([] { jp::setSavePathStandIn({}); });
            QVERIFY(w.saveAs());
            QVERIFY(w.isPasswordProtected());
            w.editor()->markSaved();
        }
        const QString tpl = tdir + QStringLiteral("/pwtemplate-test.jpub");
        QVERIFY(QFileInfo::exists(tpl));
        QVERIFY(fileBytes(tpl).startsWith(kCompound));
        jp::MainWindow n;
        Quiet quietN(n);
        n.resize(1400, 900);
        n.show();
        n.showBackstage(QStringLiteral("new"));
        QApplication::processEvents();
        QListWidget *cats = nullptr, *grid = nullptr;
        for (QListWidget *l : n.findChildren<QListWidget *>())
            if (l->count() > 3 && l->item(0)->data(Qt::UserRole).toString() == QLatin1String("Featured")) cats = l;
        QVERIFY(cats);
        for (QListWidget *l : cats->parentWidget()->findChildren<QListWidget *>())
            if (l != cats && l->viewMode() == QListView::IconMode) grid = l;
        QVERIFY(grid);
        cats->setCurrentRow(cats->count() - 1);
        QApplication::processEvents();
        QCOMPARE(grid->count(), 1);   // (listed, with no question asked)
        grid->setCurrentRow(0);
        // Cancel: nothing is made
        int asked = 0;
        answerWith({}, &asked);
        Q_EMIT grid->itemDoubleClicked(grid->item(0));
        QCOMPARE(asked, 1);
        QVERIFY(n.editor()->doc()->props.title != QStringLiteral("Club flyer"));
        QVERIFY(!n.isPasswordProtected());
        // a wrong password asks again; the right one makes the publication
        asked = 0;
        answerWith({QStringLiteral("nope"), QStringLiteral("hunter2")}, &asked);
        Q_EMIT grid->itemDoubleClicked(grid->item(0));
        QCOMPARE(asked, 2);
        QCOMPARE(n.editor()->doc()->props.title, QStringLiteral("Club flyer"));
        QVERIFY(n.editor()->filePath().isEmpty());
        QVERIFY(n.isPasswordProtected());
        n.editor()->markSaved();
    }

    // Pack and Go (to a printer's folder, and to a ZIP) and an email with the publication attached keep the password on the .jpub they write.
    void packAndGoAndEmailKeepThePassword()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        Quiet quiet(w);
        w.editor()->doc()->props.title = QStringLiteral("Shared flyer");
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("share.jpub"))));
        choosePassword(w, QStringLiteral("hunter2"));
        const QString name = w.editor()->displayName();
        auto opensWith = [&](const QByteArray &bytes, const char *what) {
            QVERIFY2(bytes.startsWith(kCompound), what);
            QVERIFY2(!bytes.contains("document.json"), what);
            const QString file = dir.filePath(QStringLiteral("check.jpub"));
            QFile f(file);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(bytes);
            f.close();
            answerWith({QStringLiteral("hunter2")});
            jp::MainWindow o;
            Quiet quietO(o);
            QVERIFY2(o.openFile(file), what);
            QVERIFY2(o.isPasswordProtected(), what);
        };
        // Pack and Go to a printer
        QTemporaryDir out;
        QString err;
        QVERIFY(jp::packForPrinter(&w, out.path(), &err));
        opensWith(fileBytes(QDir(out.path()).filePath(name + QStringLiteral(".jpub"))), "printer folder");
        // Pack and Go to a ZIP
        const QString zip = dir.filePath(QStringLiteral("pack.zip"));
        int fonts = 0;
        QVERIFY(jp::packToZip(&w, zip, &fonts, &err));
        opensWith(jp::readZipEntry(zip, name + QStringLiteral(".jpub")), "zip");
        // an email
        const QString eml = dir.filePath(QStringLiteral("mail.eml"));
        jp::setSavePathStandIn([&](const QString &, const QString &, const QString &, QString *) { return eml; });
        const auto reset = qScopeGuard([] { jp::setSavePathStandIn({}); });
        BoxAnswer ok(true, nullptr);
        QVERIFY(jp::emailAsAttachment(&w, w.editor(), QStringLiteral("jpub")));
        const QByteArray mail = fileBytes(eml);
        const qsizetype from = mail.indexOf("base64\r\n\r\n", mail.indexOf("application/x-jeffpub"));
        QVERIFY(from > 0);
        const qsizetype to = mail.indexOf("\r\n--jo-", from);
        opensWith(QByteArray::fromBase64(mail.mid(from + 10, to - from - 10)), "email");
        w.editor()->markSaved();
    }

    // A .jpub file can't make the program hold an endless list of custom properties, nor one with a huge value.
    void aPublicationFileCannotCarryAnEndlessListOfCustomProperties()
    {
        QJsonArray many;
        for (int i = 0; i < 5000; ++i) many.append(QJsonObject{{"name", QStringLiteral("p%1").arg(i)}, {"type", "lpwstr"}, {"value", "v"}});
        QCOMPARE(jp::DocProps::fromJson(QJsonObject{{"custom", many}}).custom.size(), 1000);
        QJsonArray big;
        big.append(QJsonObject{{"name", "big"}, {"type", "lpwstr"}, {"value", QString(100000, QLatin1Char('x'))}});
        big.append(QJsonObject{{"name", "small"}, {"type", "lpwstr"}, {"value", "ok"}});
        const jp::DocProps p = jp::DocProps::fromJson(QJsonObject{{"custom", big}});
        QCOMPARE(p.custom.size(), 1);
        QCOMPARE(p.custom.first().name, QStringLiteral("small"));
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
