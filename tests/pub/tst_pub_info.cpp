// JeffPub's tests of its Info page: the properties a .jpub keeps, Mark as Final, Inspect Publication, and Version History. Every file is made here, in a temporary folder.
#include "pubhelpers.h"
#include "../isolatesettings.h"
#include "app/backstage.h"
#include "app/filedialogs.h"
#include "app/infopage.h"
#include "app/passworddialogs.h"
#include "office/officecrypt.h"
#include "office/packageprops.h"
#include "../settingguard.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QtEndian>
#include <memory>

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
                box->button(yes ? QMessageBox::Yes : QMessageBox::No)->click();
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
        // a password is for .jpub files; JeffPub has no accessibility check: no card for it
        QVERIFY(page->findChild<QWidget *>(QStringLiteral("infoEncrypt")));
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
        QVERIFY(fileBytes(dir.filePath(QStringLiteral("Backup of v.jpub"))).startsWith(kCompound));
        const QString copy = dir.filePath(QStringLiteral("c.jpub"));
        QVERIFY(w.saveTo(copy, true));
        QVERIFY(fileBytes(copy).startsWith(kCompound));
        // going back to the version opens it with the password
        QVERIFY(w.restoreVersion(list.first().file));
        QVERIFY(w.isPasswordProtected());
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
