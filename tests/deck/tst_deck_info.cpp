// Tests of JeffDeck's Info page: the properties a .pptx keeps, Mark as Final, a password, Version History, and Inspect Document. Every file is made here, in a temporary folder.
#include "../isolatesettings.h"
#include "../recoverygate.h"
#include "../settingguard.h"
#include "app/recovery.h"
#include "app/filetasks.h"
#include "app/infopage.h"
#include "app/passworddialogs.h"
#include "deckhelpers.h"
#include "office/officecrypt.h"
#include "office/packageprops.h"

#include <QBuffer>
#include <QDirIterator>
#include <QListWidget>
#include <QTemporaryDir>

namespace {

// A window that says what it would have shown in a box.
struct Quiet {
    explicit Quiet(DeckWindow &w) { w.setNotifier([this](const QString &, const QString &text) { said << text; }); }
    QStringList said;
};

office::EncryptOptions quickKey()
{
    office::EncryptOptions o;
    o.spinCount = 50;
    return o;
}

// A presentation of two slides: the first has speaker notes, the second a comment and a box that lies off the slide.
std::unique_ptr<Document> sample()
{
    auto d = deck::newPresentation();
    deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
    d->props.title = QStringLiteral("Sample");
    d->storyDoc(d->pages[0]->notesStoryId)->setPlainText(QStringLiteral("Say hello first"));
    return d;
}

QString putFile(const QTemporaryDir &dir, const QString &name, const QByteArray &bytes)
{
    const QString path = dir.filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) return {};
    return path;
}

QByteArray protectedFile(const QString &password)
{
    auto d = sample();
    return office::encryptPackage(deck::exportPresentation(*d), password, quickKey());
}

const QByteArray kCompound("\xD0\xCF\x11\xE0", 4);

// The file is a protected package: it does not open when the question is cancelled, and it opens with this password.
void verifyProtected(const QString &path, const QString &password)
{
    QVERIFY(fileBytes(path).startsWith(kCompound));
    setOpenPasswordStandIn([](const QString &, bool, QString *) { return false; });
    {
        DeckWindow w;
        Quiet quiet(w);
        QVERIFY(!w.openFile(path));
    }
    setOpenPasswordStandIn([password](const QString &, bool, QString *given) {
        *given = password;
        return true;
    });
    DeckWindow w;
    Quiet quiet(w);
    QVERIFY(w.openFile(path));
    QVERIFY(w.isPasswordProtected());
    w.editor()->markSaved();
}

void protect(DeckWindow &w, const QString &password)
{
    setNewPasswordStandIn([password](bool, QString *given) {
        *given = password;
        return NewPasswordChoice::Set;
    });
    w.encryptWithPassword();
}

const InspectItem *find(const QVector<InspectItem> &items, const char *title)
{
    for (const InspectItem &i : items)
        if (i.title == QLatin1String(title)) return &i;
    return nullptr;
}

} // namespace

class DeckInfoTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }
    void init() { Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Tester")); }
    void cleanup()
    {
        setNewPasswordStandIn(nullptr);
        setOpenPasswordStandIn(nullptr);
    }

    void aProtectedPresentationOpensWithItsPassword()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = putFile(dir, QStringLiteral("p.pptx"), protectedFile(QStringLiteral("hunter2")));
        QStringList tries = {QStringLiteral("nope"), QStringLiteral("hunter2")};
        QList<bool> wrongFlags;
        setOpenPasswordStandIn([&](const QString &, bool wrong, QString *password) {
            wrongFlags << wrong;
            if (tries.isEmpty()) return false;
            *password = tries.takeFirst();
            return true;
        });
        DeckWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(path));
        QCOMPARE(int(w.editor()->doc()->pages.size()), 2);
        QCOMPARE(wrongFlags, (QList<bool>{false, true}));
        QVERIFY(w.isPasswordProtected());
        QVERIFY(quiet.said.isEmpty());
    }

    void cancellingTheQuestionOpensNothingAndSaysNothing()
    {
        QTemporaryDir dir;
        const QString path = putFile(dir, QStringLiteral("p.pptx"), protectedFile(QStringLiteral("hunter2")));
        setOpenPasswordStandIn([](const QString &, bool, QString *) { return false; });
        DeckWindow w;
        Quiet quiet(w);
        QVERIFY(!w.openFile(path));
        QVERIFY(quiet.said.isEmpty());
        QVERIFY(!w.isPasswordProtected());
    }

    void aDamagedProtectedFileSaysSoPlainly()
    {
        QTemporaryDir dir;
        QByteArray bytes = protectedFile(QStringLiteral("hunter2"));
        QByteArray info, package;
        QVERIFY(office::splitProtectedPackage(bytes, &info, &package));
        const qsizetype at = bytes.indexOf(package.mid(1000, 16));
        QVERIFY(at > 0);
        bytes[at + 3] = char(bytes[at + 3] ^ 1);
        const QString path = putFile(dir, QStringLiteral("d.pptx"), bytes);
        setOpenPasswordStandIn([](const QString &, bool, QString *password) {
            *password = QStringLiteral("hunter2");
            return true;
        });
        DeckWindow w;
        Quiet quiet(w);
        QVERIFY(!w.openFile(path));
        QCOMPARE(quiet.said.size(), 1);
        QVERIFY2(quiet.said.first().contains(QLatin1String("The file is damaged.")), qPrintable(quiet.said.first()));
    }

    // A password set while a copy is being made: the copy is not kept (it would hold the text in the clear).
    void aPasswordSetWhileACopyIsBeingMadeLeavesNoCopy()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".pptx"));
        {
            auto gate = std::make_shared<RecoveryGate>();
            const auto letGo = qScopeGuard([gate] { gate->open(); });
            DeckWindow w;
            Quiet quiet(w);
            w.newSlide();
            w.setRecoveryHook(RecoveryGate::hook(gate));
            w.autoRecover();
            QVERIFY(w.recoveryBusy());
            setNewPasswordStandIn([](bool, QString *password) {
                *password = QStringLiteral("hunter2");
                return NewPasswordChoice::Set;
            });
            w.encryptWithPassword();
            QVERIFY(w.isPasswordProtected());
            gate->open();
            recovery::AsyncWriter::waitForAll();
            QCoreApplication::processEvents();
            QVERIFY(w.recoveryCopy().isEmpty());
            QDirIterator left(root.path(), {QStringLiteral("*.pptx"), QStringLiteral("*.json")}, QDir::Files, QDirIterator::Subdirectories);
            QVERIFY(!left.hasNext());
            w.editor()->markSaved();
        }
        recovery::setRoot(QString());
    }

    void aPasswordProtectsTheFileOnSaveAndOpensItAgain()
    {
        QTemporaryDir dir;
        DeckWindow a;
        Quiet quiet(a);
        a.editor()->setDocument(sample());
        setNewPasswordStandIn([](bool, QString *password) {
            *password = QStringLiteral("hunter2");
            return NewPasswordChoice::Set;
        });
        a.encryptWithPassword();
        QVERIFY(a.isPasswordProtected());
        QVERIFY(a.editor()->isModified());
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("s.pptx"))));
        const QByteArray bytes = fileBytes(dir.filePath(QStringLiteral("s.pptx")));
        QVERIFY(bytes.startsWith(kCompound));
        verifyProtected(dir.filePath(QStringLiteral("s.pptx")), QStringLiteral("hunter2"));
        if (QTest::currentTestFailed()) return;
        setOpenPasswordStandIn([](const QString &, bool, QString *password) {
            *password = QStringLiteral("hunter2");
            return true;
        });
        DeckWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("s.pptx"))));
        QVERIFY(b.isPasswordProtected());
        QCOMPARE(deck::notesText(*b.editor()->doc(), *b.editor()->doc()->pages[0]), QStringLiteral("Say hello first"));
        // a plain save keeps the protection
        QVERIFY(b.save());
        QVERIFY(fileBytes(dir.filePath(QStringLiteral("s.pptx"))).startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)));
        // no copy of the work is kept in the clear for a protected presentation
        b.editor()->change(QStringLiteral("x"), [&] { b.editor()->doc()->props.title = QStringLiteral("Changed"); });
        b.autoRecover();
        QVERIFY(b.recoveryCopy().isEmpty());
        // a file for another program's kind can't hold the password
        QVERIFY(!b.saveTo(dir.filePath(QStringLiteral("s.odp"))));
        QVERIFY(!quietB.said.isEmpty());
        // the password taken off
        setNewPasswordStandIn([](bool has, QString *) { return has ? NewPasswordChoice::Removed : NewPasswordChoice::Cancelled; });
        b.encryptWithPassword();
        QVERIFY(!b.isPasswordProtected());
        QVERIFY(b.save());
        QVERIFY(fileBytes(dir.filePath(QStringLiteral("s.pptx"))).startsWith("PK"));
    }

    // Save as Template (to My Templates) and every Save As keep the password on: a template is a file somebody can read.
    void aTemplateOfAProtectedPresentationIsProtectedToo()
    {
        jptest::TestModeGuard testMode;
        QDir(myTemplatesFolder()).removeRecursively();
        QTemporaryDir dir;
        DeckWindow w;
        Quiet quiet(w);
        w.editor()->setDocument(sample());
        setNewPasswordStandIn([](bool, QString *password) {
            *password = QStringLiteral("hunter2");
            return NewPasswordChoice::Set;
        });
        w.encryptWithPassword();
        setTemplateNameStandIn([](const QString &) { return QStringLiteral("Secret"); });
        const bool saved = w.saveAsTemplate();
        setTemplateNameStandIn({});
        QVERIFY(saved);
        const QStringList found = myTemplateFiles({QStringLiteral("*.potx")});
        QCOMPARE(found.size(), 1);
        const QByteArray bytes = fileBytes(found.first());
        QVERIFY(bytes.startsWith(kCompound));
        verifyProtected(found.first(), QStringLiteral("hunter2"));
        if (QTest::currentTestFailed()) return;
        QVERIFY(removeMyTemplate(found.first()));
        // the other kinds of Save As and Save a Copy
        for (const char *name : {"a.potx", "b.ppsx", "c.pptx"}) {
            QVERIFY(w.saveTo(dir.filePath(QString::fromLatin1(name)), true));
            QVERIFY2(fileBytes(dir.filePath(QString::fromLatin1(name))).startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)), name);
        }
        QVERIFY(!w.saveTo(dir.filePath(QStringLiteral("d.otp")), true));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("d.otp"))));
        QVERIFY(!quiet.said.isEmpty());
        // (the message names the kind of file it refused)
        QVERIFY2(quiet.said.last().contains(QLatin1String(".otp")) && !quiet.said.last().contains(QLatin1String(".odp")), qPrintable(quiet.said.last()));
        w.editor()->markSaved();
    }

    // The programs' parts run side by side on one machine: the data folder a test works in (My Templates) is this process's own, so no other part can take a file from it.
    void theTestDataFolderBelongsToThisProcess()
    {
        jptest::TestModeGuard testMode;
        QVERIFY2(myTemplatesFolder().contains(QLatin1Char('-') + QString::number(QCoreApplication::applicationPid()) + QLatin1Char('/')), qPrintable(myTemplatesFolder()));
    }

    // Going back to a version keeps the password: the window and the next save are protected as before, and the version itself was kept protected.
    void goingBackToAVersionKeepsThePassword()
    {
        QTemporaryDir dir;
        DeckWindow w;
        Quiet quiet(w);
        w.editor()->setDocument(sample());
        protect(w, QStringLiteral("hunter2"));
        const QString path = dir.filePath(QStringLiteral("v.pptx"));
        QVERIFY(w.saveTo(path));
        QVERIFY(w.saveVersion(QStringLiteral("First")));
        QVERIFY(fileBytes(write::VersionStore(path, QStringLiteral(".pptx")).folder() + QLatin1Char('/') + w.versions().first().file).startsWith(kCompound));
        w.editor()->change(QStringLiteral("x"), [&] { w.editor()->doc()->props.title = QStringLiteral("Second title"); });
        QVERIFY(w.restoreVersion(w.versions().first().file));
        QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("Sample"));
        QVERIFY(w.isPasswordProtected());
        QVERIFY(w.save());
        verifyProtected(path, QStringLiteral("hunter2"));
    }

    // A password put on, changed, or taken off is put on every kept version when the file is saved with it, so none is left in the clear and Go Back still works.
    void versionsFollowThePassword()
    {
        QTemporaryDir dir;
        DeckWindow w;
        Quiet quiet(w);
        w.editor()->setDocument(sample());
        const QString path = dir.filePath(QStringLiteral("f.pptx"));
        QVERIFY(w.saveTo(path));
        QVERIFY(w.saveVersion(QStringLiteral("Plain")));
        const write::VersionStore store(path, QStringLiteral(".pptx"));
        auto versionFile = [&] { return store.folder() + QLatin1Char('/') + w.versions().first().file; };
        QVERIFY(fileBytes(versionFile()).startsWith("PK"));
        protect(w, QStringLiteral("one"));
        QVERIFY(fileBytes(versionFile()).startsWith("PK"));   // (the file is not saved with it yet)
        QVERIFY(w.saveTo(path));
        QVERIFY(fileBytes(versionFile()).startsWith(kCompound));
        protect(w, QStringLiteral("two"));
        QVERIFY(w.saveTo(path));
        QVERIFY(w.restoreVersion(w.versions().first().file));   // (opened with the new password)
        QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("Sample"));
        QVERIFY(w.isPasswordProtected());
        setNewPasswordStandIn([](bool has, QString *) { return has ? NewPasswordChoice::Removed : NewPasswordChoice::Cancelled; });
        w.encryptWithPassword();
        QVERIFY(w.saveTo(path));
        for (const write::DocVersion &v : w.versions()) QVERIFY2(fileBytes(store.folder() + QLatin1Char('/') + v.file).startsWith("PK"), qPrintable(v.name));
        w.editor()->markSaved();
    }

    // A password taken off or changed and then not saved leaves the versions as the file on disk has them: protected, and opening with the file's password.
    void aPasswordChangeThatIsNotSavedLeavesTheVersionsAlone()
    {
        QTemporaryDir dir;
        DeckWindow w;
        Quiet quiet(w);
        w.editor()->setDocument(sample());
        const QString path = dir.filePath(QStringLiteral("g.pptx"));
        protect(w, QStringLiteral("one"));
        QVERIFY(w.saveTo(path));
        QVERIFY(w.saveVersion(QStringLiteral("V")));
        const write::VersionStore store(path, QStringLiteral(".pptx"));
        auto versionFile = [&] { return store.folder() + QLatin1Char('/') + w.versions().first().file; };
        auto opensWith = [&](const QString &password) {
            QByteArray package;
            return write::versionPackage(fileBytes(versionFile()), password, &package);
        };
        QVERIFY(fileBytes(versionFile()).startsWith(kCompound));
        // taken off, not saved: a version kept now, and the old one, are still protected with "one"
        setNewPasswordStandIn([](bool has, QString *) { return has ? NewPasswordChoice::Removed : NewPasswordChoice::Cancelled; });
        w.encryptWithPassword();
        QVERIFY(w.saveVersion(QStringLiteral("W")));
        for (const write::DocVersion &v : w.versions()) QVERIFY2(fileBytes(store.folder() + QLatin1Char('/') + v.file).startsWith(kCompound), qPrintable(v.name));
        // changed, not saved: they open with the file's password, not the new one
        protect(w, QStringLiteral("two"));
        QVERIFY(opensWith(QStringLiteral("one")));
        QVERIFY(!opensWith(QStringLiteral("two")));
        // saved: now they follow
        QVERIFY(w.saveTo(path));
        QVERIFY(opensWith(QStringLiteral("two")));
        QVERIFY(!opensWith(QStringLiteral("one")));
        w.editor()->markSaved();
    }

    // A file protected with the password every program knows opens without a question.
    void aFileWithTheCommonPasswordOpensWithoutAsking()
    {
        QTemporaryDir dir;
        auto d = sample();
        office::EncryptOptions o = quickKey();
        const QString path = putFile(dir, QStringLiteral("c.pptx"), office::encryptPackage(deck::exportPresentation(*d), office::defaultPassword(), o));
        QVERIFY(fileBytes(path).startsWith(kCompound));
        bool asked = false;
        setOpenPasswordStandIn([&](const QString &, bool, QString *) {
            asked = true;
            return false;
        });
        DeckWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(path));
        QVERIFY(!asked);
        QCOMPARE(int(w.editor()->doc()->pages.size()), 2);
        QVERIFY(quiet.said.isEmpty());
    }

    // A copy of a protected file or version is protected too: it keeps the password.
    void aCopyOfAProtectedPresentationKeepsThePassword()
    {
        QTemporaryDir dir;
        const QString path = putFile(dir, QStringLiteral("p.pptx"), protectedFile(QStringLiteral("hunter2")));
        setOpenPasswordStandIn([](const QString &, bool, QString *given) {
            *given = QStringLiteral("hunter2");
            return true;
        });
        DeckWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(path, OpenHow::Copy));
        QVERIFY(w.isPasswordProtected());
        QVERIFY(w.editor()->filePath().isEmpty());
        const QString other = dir.filePath(QStringLiteral("copy.pptx"));
        QVERIFY(w.saveTo(other));
        verifyProtected(other, QStringLiteral("hunter2"));
    }

    // "Always create backup copy": the backup of a protected presentation is protected, and so is one made when the password has just been put on.
    void theBackupCopyIsProtectedToo()
    {
        QTemporaryDir dir;
        DeckWindow w;
        Quiet quiet(w);
        w.editor()->setDocument(sample());
        const QString path = dir.filePath(QStringLiteral("b.pptx"));
        QVERIFY(w.saveTo(path));   // (a plain file)
        protect(w, QStringLiteral("hunter2"));
        jptest::SettingGuard keep(QStringLiteral("save/backup"), true, false);
        QVERIFY(w.saveTo(path));
        const QString backup = dir.filePath(QStringLiteral("Backup of b.pptx"));
        QVERIFY(QFileInfo::exists(backup));
        verifyProtected(backup, QStringLiteral("hunter2"));
    }

    // Mark as Final on a presentation that is then saved where the mark can't be kept leaves the window open for editing.
    void markAsFinalDoesNotHoldAWindowWhoseFileCantKeepTheMark()
    {
        QTemporaryDir dir;
        DeckWindow w;
        Quiet quiet(w);
        w.editor()->setDocument(sample());
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("m.odp")); });
        w.setMarkedFinal(true);
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("m.odp"))));
        QVERIFY(!w.markedAsFinal());
        QVERIFY(!w.isReadOnly());
    }

    void propertiesSurviveASaveAndReopen()
    {
        QTemporaryDir dir;
        DeckWindow a;
        Quiet quiet(a);
        a.editor()->setDocument(sample());
        office::PackageProps p = a.properties();
        p.title = QStringLiteral("Annual plan");
        p.creator = QStringLiteral("Ann");
        p.subject = QStringLiteral("Budget");
        p.keywords = QStringLiteral("money, plan");
        p.category = QStringLiteral("Finance");
        p.comments = QStringLiteral("A note to the reader");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo");
        office::CustomProperty c;
        c.name = QStringLiteral("Project");
        c.type = QStringLiteral("lpwstr");
        c.value = QStringLiteral("Zed");
        p.custom.push_back(c);
        a.applyProperties(p);
        QVERIFY(a.editor()->isModified());
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("p.pptx"))));
        DeckWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("p.pptx"))));
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
        QCOMPARE(q.custom.first().name, QStringLiteral("Project"));
        QCOMPARE(q.custom.first().value, QStringLiteral("Zed"));
        QVERIFY(q.created.isValid());
        QVERIFY(q.modified.isValid());
        // a property the file had that this program does not model is kept through another save
        QVERIFY(b.save());
        DeckWindow c2;
        Quiet quietC(c2);
        QVERIFY(c2.openFile(dir.filePath(QStringLiteral("p.pptx"))));
        QCOMPARE(c2.properties().custom.size(), 1);
        QCOMPARE(c2.properties().manager, QStringLiteral("Bo"));
    }

    void markedAsFinalOpensForReadingOnly()
    {
        QTemporaryDir dir;
        DeckWindow a;
        Quiet quiet(a);
        a.editor()->setDocument(sample());
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("f.pptx"))));
        a.setMarkedFinal(true);
        QVERIFY(a.isReadOnly());
        QVERIFY(a.markedAsFinal());
        QVERIFY(!a.editor()->isModified());
        DeckWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("f.pptx"))));
        QVERIFY(b.isReadOnly());
        QVERIFY(b.statusBar()->currentMessage().contains(QLatin1String("Marked as final")));
        b.setMarkedFinal(false);
        QVERIFY(!b.isReadOnly());
        QVERIFY(!b.markedAsFinal());
        // opened as a copy, it is a presentation to edit
        DeckWindow c;
        Quiet quietC(c);
        QVERIFY(c.openFile(dir.filePath(QStringLiteral("f.pptx")), OpenHow::Copy));
        QVERIFY(!c.isReadOnly());
    }

    void inspectFindsNotesCommentsHiddenSlidesAndWhatIsOffTheSlide()
    {
        DeckWindow w;
        Quiet quiet(w);
        auto d = sample();
        deck::addComment(*d->pages[1], deck::CommentAuthor{QStringLiteral("Cy"), QStringLiteral("C")}, QStringLiteral("Check this"), QPointF(10, 10));
        d->pages[1]->hidden = true;
        auto off = std::make_shared<ShapeItem>();
        off->rect = QRectF(d->pageSize().width() + 50, 20, 80, 30);
        d->pages[1]->items.push_back(off);
        auto on = std::make_shared<ShapeItem>();
        on->rect = QRectF(20, 20, 80, 30);
        d->pages[1]->items.push_back(on);
        d->props.author = QStringLiteral("Ann");
        d->props.company = QStringLiteral("Acme");
        w.editor()->setDocument(std::move(d));
        const QVector<InspectItem> items = w.inspectItems();
        QVERIFY(find(items, "Comments"));
        QVERIFY(find(items, "Speaker notes"));
        QVERIFY(find(items, "Hidden slides"));
        QVERIFY(find(items, "Document properties and personal information"));
        QVERIFY(find(items, "Content off the slides"));
        QCOMPARE(find(items, "Comments")->count(), 1);
        QCOMPARE(find(items, "Speaker notes")->count(), 1);
        QCOMPARE(find(items, "Hidden slides")->count(), 1);
        QCOMPARE(find(items, "Document properties and personal information")->count(), 2);
        QCOMPARE(find(items, "Content off the slides")->count(), 1);
        const int slideItems = int(w.editor()->doc()->pages[1]->items.size());
        QCOMPARE(find(items, "Content off the slides")->remove(), 1);
        QCOMPARE(int(w.editor()->doc()->pages[1]->items.size()), slideItems - 1);
        QCOMPARE(find(items, "Comments")->remove(), 1);
        QCOMPARE(find(items, "Speaker notes")->remove(), 1);
        QCOMPARE(deck::notesText(*w.editor()->doc(), *w.editor()->doc()->pages[0]), QString());
        QCOMPARE(find(items, "Hidden slides")->remove(), 1);
        QVERIFY(!w.editor()->doc()->pages[1]->hidden);
        QCOMPARE(find(items, "Document properties and personal information")->remove(), 2);
        for (const InspectItem &i : items) QCOMPARE(i.count(), 0);
        // the removals are steps that undo
        QVERIFY(w.editor()->isModified());
        // the names stay out of the properties as they are saved
        QVERIFY(w.properties().creator.isEmpty());
        InspectDialog dialog(QStringLiteral("Inspect"), items, nullptr);
        QVERIFY(dialog.summaries().first().contains(QLatin1String("None found")));
    }

    void versionsAreKeptAndGoneBackTo()
    {
        QTemporaryDir dir;
        DeckWindow w;
        Quiet quiet(w);
        w.editor()->setDocument(sample());
        // nothing to keep before the presentation has a file
        QVERIFY(!w.saveVersion(QStringLiteral("early")));
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("v.pptx"))));
        QVERIFY(w.saveVersion(QStringLiteral("First")));
        QCOMPARE(w.versions().size(), 1);
        QCOMPARE(w.versions().first().name, QStringLiteral("First"));
        w.editor()->change(QStringLiteral("x"), [&] { w.editor()->doc()->props.title = QStringLiteral("Second title"); });
        QVERIFY(w.restoreVersion(w.versions().first().file));
        QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("Sample"));
        QVERIFY(w.editor()->isModified());
        // going back kept what was there as a version first
        QCOMPARE(w.versions().size(), 2);
        QVERIFY(w.versions().first().name.startsWith(QLatin1String("Before restoring")));
    }

    // Check Accessibility lists a picture with no alt text and a slide with no title, and goes to the picture (its slide, selected) when it is clicked.
    void checkAccessibilityListsAPictureWithNoAltTextAndASlideWithNoTitle()
    {
        DeckWindow w;
        Quiet quiet(w);
        w.show();
        Editor *ed = w.editor();
        ed->setDocument(sample());
        QImage img(40, 40, QImage::Format_RGB32);
        img.fill(Qt::red);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = ed->doc()->addImage(png, "png");
        pic->rect = QRectF(100, 100, 120, 120);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        ed->setCurrentPage(1);
        ed->addItem(pic, false);
        ed->setCurrentPage(0);
        ed->clearSelection();
        w.act(QStringLiteral("review.accessibility"))->trigger();
        PageAccessibilityPane *pane = w.accessibilityPane();
        QVERIFY(pane);
        QStringList lines;
        for (int i = 0; i < pane->list()->count(); ++i) lines << pane->list()->item(i)->text();
        int picRow = -1, titleRows = 0;
        for (int i = 0; i < lines.size(); ++i) {
            if (lines[i].contains(QLatin1String("alt text"))) picRow = i;
            if (lines[i].contains(QLatin1String("no title"))) ++titleRows;
        }
        QVERIFY2(picRow >= 0, qPrintable(lines.join(QLatin1Char('|'))));
        QVERIFY(titleRows >= 1);
        Q_EMIT pane->list()->itemActivated(pane->list()->item(picRow));
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(ed->selection(), QStringList{pic->id});
        // alt text given: no picture problem is left
        ed->change(QStringLiteral("alt"), [&] { ed->doc()->item(pic->id)->altText = QStringLiteral("A red square"); });
        pane->recheck();
        for (int i = 0; i < pane->list()->count(); ++i) QVERIFY(!pane->list()->item(i)->text().contains(QLatin1String("alt text")));
    }

    void theInfoPageHasItsCardsAndEditsProperties()
    {
        DeckWindow w;
        Quiet quiet(w);
        w.show();
        w.showFileScreen(QStringLiteral("info"));
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen);
        QCOMPARE(screen->currentPage(), QStringLiteral("info"));
        QWidget *page = screen->findChild<QWidget *>(QStringLiteral("infoPage"));
        QVERIFY(page);
        for (const char *name : {"infoMarkFinal", "infoEncrypt", "infoInspect", "infoVersions", "infoCustom", "infoShowFolder", "infoCopyPath"})
            QVERIFY2(page->findChild<QWidget *>(QString::fromLatin1(name)), name);
        QVERIFY(page->findChild<QWidget *>(QStringLiteral("infoAccessibility")));
        QVERIFY(!page->findChild<QPushButton *>(QStringLiteral("infoCopyPath"))->isEnabled());   // (nothing saved yet)
        auto *title = page->findChild<QLineEdit *>(QStringLiteral("infoTitle"));
        QVERIFY(title);
        title->setText(QStringLiteral("From the page"));
        Q_EMIT title->editingFinished();
        QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("From the page"));
        QVERIFY(w.editor()->isModified());
        w.hideFileScreen();
    }
};

JP_TEST_CLASS(DeckInfoTests)
#include "tst_deck_info.moc"
