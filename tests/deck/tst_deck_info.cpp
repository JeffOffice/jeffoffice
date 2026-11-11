// Tests of JeffDeck's Info page: the properties a .pptx keeps, Mark as Final, a password, Version History, and Inspect Document. Every file is made here, in a temporary folder.
#include <QElapsedTimer>
#include "../isolatesettings.h"
#include "../recoverygate.h"
#include "../privacyhooks.h"
#include "office/privacy.h"
#include "../settingguard.h"
#include "app/recovery.h"
#include "app/filetasks.h"
#include "app/infopage.h"
#include "app/passworddialogs.h"
#include "deckhelpers.h"
#include "office/officecrypt.h"
#include "office/packageprops.h"

#include "core/comments.h"
#include "io/zip.h"
#include "app/settings.h"

#include <QBuffer>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QDirIterator>
#include <QListWidget>
#include <QCheckBox>
#include <QTemporaryDir>
#include <memory>

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

    // The option takes every name out of the file, and nothing the file says: comment authors and initials as well as the properties; the slides, notes, and comment texts are as they were.
    void thePrivacyOptionTakesTheNamesOutOfTheFileAndKeepsTheContent()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QTemporaryDir dir;
        DeckWindow a;
        Quiet quiet(a);
        auto d = sample();
        deck::addComment(*d->pages[1], deck::CommentAuthor{QStringLiteral("Cy Mox"), QStringLiteral("CM")}, QStringLiteral("Check this"), QPointF(10, 10));
        a.editor()->setDocument(std::move(d));
        office::PackageProps p = a.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.lastModifiedBy = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo Ray");
        a.applyProperties(p);
        const int slides = int(a.editor()->doc()->pages.size());
        for (const char *name : {"p.pptx", "p.odp"}) {
            QVERIFY(a.saveTo(dir.filePath(QString::fromLatin1(name)), true));
            // Not one name in any part of the file.
            QMap<QString, QByteArray> parts;
            QVERIFY(readZip(fileBytes(dir.filePath(QString::fromLatin1(name))), parts));
            for (auto it = parts.cbegin(); it != parts.cend(); ++it)
                for (const char *who : {"Ann Lee", "Acme", "Bo Ray", "Cy Mox"})
                    QVERIFY2(!it.value().contains(who), qPrintable(QString::fromLatin1(name) + QLatin1Char(' ') + it.key() + QStringLiteral(": ") + QString::fromLatin1(who)));
        }
        DeckWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("p.pptx"))));
        const office::PackageProps q = b.properties();
        QVERIFY(q.creator.isEmpty() && q.lastModifiedBy.isEmpty() && q.company.isEmpty() && q.manager.isEmpty());
        QCOMPARE(q.title, QStringLiteral("Sample"));
        QCOMPARE(int(b.editor()->doc()->pages.size()), slides);
        const QVector<deck::Comment> comments = deck::commentsOf(*b.editor()->doc()->pages[1]);
        QCOMPARE(comments.size(), 1);
        QCOMPARE(comments[0].text, QStringLiteral("Check this"));
        QVERIFY(comments[0].author != QStringLiteral("Cy Mox"));
        QVERIFY(comments[0].initials != QStringLiteral("CM"));
        QCOMPARE(deck::notesText(*b.editor()->doc(), *b.editor()->doc()->pages[0]), QStringLiteral("Say hello first"));
    }

    // The open presentation is not what is scrubbed: a copy, a save, and a save that is cancelled leave its names, its history, and its mark as saved as they were.
    void thePrivacyOptionLeavesTheOpenPresentationAsItWas()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QTemporaryDir dir;
        DeckWindow a;
        Quiet quiet(a);
        auto d = sample();
        deck::addComment(*d->pages[1], deck::CommentAuthor{QStringLiteral("Cy Mox"), QStringLiteral("CM")}, QStringLiteral("Check this"), QPointF(10, 10));
        a.editor()->setDocument(std::move(d));
        office::PackageProps p = a.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        a.applyProperties(p);
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("first.pptx"))));
        QVERIFY(!a.editor()->isModified());
        const int steps = a.editor()->undoStack()->count();
        auto unchanged = [&] {
            QCOMPARE(a.properties().creator, QStringLiteral("Ann Lee"));
            QCOMPARE(a.properties().company, QStringLiteral("Acme"));
            QCOMPARE(deck::commentsOf(*a.editor()->doc()->pages[1]).value(0).author, QStringLiteral("Cy Mox"));
            QCOMPARE(a.editor()->undoStack()->count(), steps);
            QVERIFY(!a.editor()->isModified());
        };
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("copy.pptx")), true));
        unchanged();
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("second.pptx"))));
        unchanged();
        {
            jptest::DismissBoxes boxes;
            QVERIFY(!a.saveTo(dir.filePath(QStringLiteral("missing/third.pptx"))));   // (a save that fails)
        }
        unchanged();
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(fileBytes(dir.filePath(QStringLiteral("copy.pptx"))), parts));
        QVERIFY(!parts.value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
    }

    void whenTheNamesCantBeTakenOutThePersonIsToldAndTheFileIsStillWritten()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] {
            Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
            office::setPackageScrubHookForTest({});
        });
        QTemporaryDir dir;
        DeckWindow a;
        Quiet quiet(a);
        a.editor()->setDocument(sample());
        office::PackageProps p = a.properties();
        p.creator = QStringLiteral("Ann Lee");
        a.applyProperties(p);
        office::setPackageScrubHookForTest(jptest::damagePart("docProps/core.xml"));
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("told.pptx"))));
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("told.pptx"))));
        QVERIFY2(quiet.said.join(QLatin1Char('|')).contains(QLatin1String("personal information could not be taken out")), qPrintable(quiet.said.join(QLatin1Char('|'))));
        QVERIFY(quiet.said.join(QLatin1Char('|')).contains(QLatin1String("docProps/core.xml")));
    }

    void withThePrivacyOptionOffASaveIsUnchanged()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        QTemporaryDir dir;
        DeckWindow a;
        Quiet quiet(a);
        a.editor()->setDocument(sample());
        office::PackageProps p = a.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        a.applyProperties(p);
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("off.pptx"))));
        auto stable = [](const QByteArray &zip) {
            QMap<QString, QByteArray> parts, out;
            readZip(zip, parts);
            for (auto it = parts.cbegin(); it != parts.cend(); ++it)
                out.insert(it.key(), QString::fromUtf8(it.value()).replace(QRegularExpression(QStringLiteral("\\d{4}-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d")), QStringLiteral("T")).toUtf8());
            return out;
        };
        QCOMPARE(stable(fileBytes(dir.filePath(QStringLiteral("off.pptx")))), stable(deck::exportPresentation(*a.editor()->doc())));
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(fileBytes(dir.filePath(QStringLiteral("off.pptx"))), parts));
        QVERIFY(parts.value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
    }

    void aPresentationSentByEmailHasNoNamesWhenTheOptionIsOn()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QTemporaryDir dir;
        DeckWindow a;
        Quiet quiet(a);
        a.editor()->setDocument(sample());
        office::PackageProps p = a.properties();
        p.creator = QStringLiteral("Ann Lee");
        a.applyProperties(p);
        const QString eml = dir.filePath(QStringLiteral("m.eml"));
        a.setSavePathChooser([&](const QString &, const QString &) { return eml; });
        QVERIFY(a.exportEmail(DeckWindow::EmailWhat::Presentation));
        QMap<QString, QByteArray> parts;
        QVERIFY2(readZip(jptest::attachmentOf(fileBytes(eml), ".pptx"), parts), "the attachment is a package");
        QVERIFY(!parts.value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
    }

    void theInfoPageHasTheBoxForThePrivacyOption()
    {
        DeckWindow w;
        const QVariant priorSetting = Settings::get().value(QStringLiteral("privacy/removeOnSave"), false);
        const auto restore = qScopeGuard([priorSetting] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), priorSetting); });
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        std::unique_ptr<QWidget> page(w.buildInfoPage(nullptr));
        auto *box = page->findChild<QCheckBox *>(QStringLiteral("infoRemovePersonal"));
        QVERIFY(box);
        QCOMPARE(box->text(), QStringLiteral("Remove personal information from this file when saving"));
        QVERIFY(!box->isChecked());
        box->setChecked(true);
        QVERIFY(Settings::get().value(QStringLiteral("privacy/removeOnSave")).toBool());
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
        comments::addComment(*d->pages[1], comments::CommentAuthor{QStringLiteral("Cy"), QStringLiteral("C")}, QStringLiteral("Check this"), QPointF(10, 10));
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

    // Text is checked against its background: the contrast of the two colors must be 4.5 to 1 (3 to 1 for text of 18 points, or 14 and bold); the fill of its box, else the page's, is the
    // background, and a picture under it is for the person to look at.
    void textContrastIsCheckedAgainstItsBackground()
    {
        QVERIFY(std::abs(contrastRatio(QColor(QStringLiteral("#777777")), Qt::white) - 4.478) < 0.01);
        QVERIFY(contrastRatio(QColor(QStringLiteral("#777777")), Qt::white) < 4.5);
        QVERIFY(std::abs(contrastRatio(QColor(QStringLiteral("#767676")), Qt::white) - 4.542) < 0.01);
        QVERIFY(contrastRatio(QColor(QStringLiteral("#767676")), Qt::white) >= 4.5);
        QVERIFY(std::abs(contrastRatio(Qt::black, Qt::white) - 21) < 0.001);
        QCOMPARE(neededContrast(17.9, false), 4.5);
        QCOMPARE(neededContrast(18, false), 3.0);
        QCOMPARE(neededContrast(14, true), 3.0);
        QCOMPARE(neededContrast(13.9, true), 4.5);
        QCOMPARE(neededContrast(14, false), 4.5);
        auto doc = deck::newPresentation();
        doc->pages.clear();
        // one case to a page: the text's color, its size, bold, and the fill of its box (none: the page's white)
        struct Case { const char *color; double points; bool bold; const char *fill; bool flagged; };
        const QVector<Case> cases{{"#777777", 12, false, nullptr, true}, {"#767676", 12, false, nullptr, false}, {"#777777", 18, false, nullptr, false},
                                  {"#999999", 18, false, nullptr, true}, {"#888888", 14, true, nullptr, false}, {"#888888", 13, true, nullptr, true},
                                  {"#444444", 12, false, "#333333", true}, {"#FFFFFF", 12, false, "#333333", false}};
        for (const Case &c : cases) {
            doc->pages.push_back(std::make_shared<Page>());
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(50, 100, 300, 60);
            t->storyId = doc->createStory(QStringLiteral("Some words to read"));
            QTextCursor cur(doc->storyDoc(t->storyId));
            cur.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setProperty(tp::ColorRefP, QString::fromLatin1(c.color));
            f.setFontPointSize(c.points);
            f.setFontWeight(c.bold ? QFont::Bold : QFont::Normal);
            cur.mergeCharFormat(f);
            if (c.fill) t->fill = Fill::solid(ColorRef::rgb(QColor(QString::fromLatin1(c.fill))));
            doc->pages.back()->items.push_back(t);
        }
        // a picture under text that has no fill of its own
        doc->pages.push_back(std::make_shared<Page>());
        auto pic = std::make_shared<PictureItem>();
        pic->rect = QRectF(40, 90, 400, 200);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        pic->altText = QStringLiteral("A photo");
        doc->pages.back()->items.push_back(pic);
        auto over = std::make_shared<TextItem>();
        over->rect = QRectF(60, 110, 300, 60);
        over->storyId = doc->createStory(QStringLiteral("Words over a picture"));
        doc->pages.back()->items.push_back(over);
        const QVector<PageAccessIssue> found = checkPageAccessibility(*doc);
        for (int i = 0; i < cases.size(); ++i) {
            int n = 0;
            for (const auto &issue : found)
                if (issue.page == i && issue.kind == PageAccessIssue::LowContrast) {
                    ++n;
                    QCOMPARE(int(issue.level), int(PageAccessIssue::Warning));
                    QVERIFY(issue.message.contains(QLatin1String("contrast")));
                }
            QVERIFY2(n == (cases[i].flagged ? 1 : 0), qPrintable(QStringLiteral("case %1 (%2): %3 found").arg(i).arg(QString::fromLatin1(cases[i].color)).arg(n)));
        }
        int tips = 0;
        for (const auto &issue : found)
            if (issue.page == cases.size() && issue.kind == PageAccessIssue::LowContrast) {
                ++tips;
                QCOMPARE(int(issue.level), int(PageAccessIssue::Tip));
                QVERIFY(issue.message.contains(QLatin1String("Check contrast by eye")));
            }
        QCOMPARE(tips, 1);
    }

    // Objects read in another order than top to bottom, left to right are listed as tips, and the Reading Order list of the checker moves them.
    void objectsReadOutOfPlaceAreListedAndReorderedInTheReadingOrderView()
    {
        DeckWindow w;
        Quiet quiet(w);
        w.show();
        Editor *ed = w.editor();
        ed->setDocument(showDeck(1));
        auto page = ed->doc()->pages[0];
        page->items.clear();
        auto box = [&](const QString &words, const QRectF &r) {
            auto t = std::make_shared<TextItem>();
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
            for (const auto &i : checkPageAccessibility(*ed->doc()))
                if (i.kind == PageAccessIssue::ReadingOrder && i.level == PageAccessIssue::Tip) ids << i.itemId;
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
        w.act(QStringLiteral("review.accessibility"))->trigger();
        PageAccessibilityPane *pane = w.accessibilityPane();
        QVERIFY(pane);
        auto *button = pane->findChild<QPushButton *>(QStringLiteral("accessReadingOrder"));
        QVERIFY(button);
        QCOMPARE(button->text(), QStringLiteral("Reading Order"));
        button->click();
        SelectionPane *view = pane->readingOrderPane();
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
        auto doc = deck::newPresentation();
        doc->pages.clear();
        auto master = std::make_shared<MasterPage>();
        master->id = QStringLiteral("tm");
        auto band = std::make_shared<ShapeItem>();
        band->rect = QRectF(0, 0, 612, 200);
        band->fill = Fill::solid(ColorRef::rgb(QColor(QStringLiteral("#222222"))));
        master->items.push_back(band);
        doc->masters.push_back(master);
        auto story = [&](const QString &color, double points) {
            const QString id = doc->createStory(QStringLiteral("Some words to read"));
            QTextCursor cur(doc->storyDoc(id));
            cur.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setProperty(tp::ColorRefP, color);
            f.setFontPointSize(points);
            cur.mergeCharFormat(f);
            return id;
        };
        auto page = [&](const QString &masterId = QString()) {
            auto pg = std::make_shared<Page>();
            pg->masterId = masterId;
            doc->pages.push_back(pg);
            return pg;
        };
        auto box = [&](const QString &color, double points, const QRectF &r = QRectF(50, 50, 300, 60)) {
            auto t = std::make_shared<TextItem>();
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
            auto dark = std::make_shared<ShapeItem>();
            dark->rect = QRectF(50, 50, 100, 60);
            dark->fill = Fill::solid(ColorRef::rgb(QColor(QStringLiteral("#111111"))));
            pg->items.push_back(dark);
            pg->items.push_back(box(QStringLiteral("#000000"), 12, QRectF(50, 50, 400, 60)));
        }
        page()->items.push_back(box(QStringLiteral("#66000000"), 12));   // 4: black at 40% opacity on white
        {
            auto t = box(QStringLiteral("#888888"), 18);   // 5: large, but drawn at half size
            t->autofit = TextItem::ShrinkOnOverflow;
            t->fitScale = 0.5;
            page()->items.push_back(t);
        }
        page()->items.push_back(box(QStringLiteral("#888888"), 18));   // 6: the same, drawn at its size
        {
            auto g = std::make_shared<GroupItem>();   // 7: in a group
            g->children.push_back(box(QStringLiteral("#999999"), 12));
            g->rect = g->children.front()->rect;
            page()->items.push_back(g);
        }
        page()->items.push_back(box(QStringLiteral("#77767C"), 12));   // 8: 4.4998 to 1
        {
            auto pg = page();   // 9: thirty layers with no fill under the words
            for (int i = 0; i < 30; ++i) {
                auto layer = std::make_shared<ShapeItem>();
                layer->rect = QRectF(40 + i, 40 + i, 400, 100);
                layer->altText = QStringLiteral("layer");
                pg->items.push_back(layer);
            }
            pg->items.push_back(box(QStringLiteral("#999999"), 12, QRectF(80, 80, 300, 60)));
        }
        QElapsedTimer clock;
        clock.start();
        const QVector<PageAccessIssue> found = checkPageAccessibility(*doc);
        QVERIFY2(clock.elapsed() < 3000, qPrintable(QStringLiteral("%1 ms").arg(clock.elapsed())));
        auto count = [&](int pageNumber) {
            int n = 0;
            for (const auto &i : found)
                if (i.page == pageNumber && i.kind == PageAccessIssue::LowContrast && i.level == PageAccessIssue::Warning) ++n;
            return n;
        };
        const QVector<int> flagged{0, 1, 0, 1, 1, 1, 0, 1, 1, 1};
        for (int i = 0; i < flagged.size(); ++i) QVERIFY2(count(i) == flagged[i], qPrintable(QStringLiteral("page %1: %2 found, %3 wanted").arg(i).arg(count(i)).arg(flagged[i])));
        for (const auto &i : found)
            if (i.page == 8 && i.kind == PageAccessIssue::LowContrast) {
                QVERIFY2(i.message.contains(QLatin1String("4.49 to 1")), qPrintable(i.message));
                QVERIFY(!i.message.contains(QLatin1String("4.50")));
            }
    }

    // Reading order: a picture over most of the page is a backdrop and is left out; objects whose spans overlap by half are a row, however their tops differ; and a real swap is
    // a tip on the object that is read too early.
    void readingOrderTipsAreNotNoisyAndPinTheRightObject()
    {
        auto doc = deck::newPresentation();
        doc->pages.clear();
        auto text = [&](const QRectF &r) {
            auto t = std::make_shared<TextItem>();
            t->rect = r;
            t->storyId = doc->createStory(QStringLiteral("Words"));
            return t;
        };
        auto tipsOn = [&](int pageNumber) {
            QStringList ids;
            for (const auto &i : checkPageAccessibility(*doc))
                if (i.page == pageNumber && i.kind == PageAccessIssue::ReadingOrder && i.level == PageAccessIssue::Tip) ids << i.itemId;
            return ids;
        };
        // 0: words over a backdrop picture that is listed after them
        auto backdrop = std::make_shared<PictureItem>();
        backdrop->rect = QRectF(0, 0, 612, 792);
        backdrop->imgRect = QRectF(0, 0, 612, 792);
        backdrop->altText = QStringLiteral("A background");
        doc->pages.push_back(std::make_shared<Page>());
        doc->pages[0]->items = {text(QRectF(50, 300, 200, 40)), backdrop};
        QVERIFY(tipsOn(0).isEmpty());
        // 1: two columns, the left one read first and 20 points lower
        doc->pages.push_back(std::make_shared<Page>());
        doc->pages[1]->items = {text(QRectF(50, 120, 200, 100)), text(QRectF(300, 100, 200, 100))};
        QVERIFY(tipsOn(1).isEmpty());
        // 2: right read first, then left, then the one below: the right one is the one out of place
        doc->pages.push_back(std::make_shared<Page>());
        auto right = text(QRectF(300, 100, 200, 40)), left = text(QRectF(50, 100, 200, 40)), below = text(QRectF(50, 300, 200, 40));
        doc->pages[2]->items = {right, left, below};
        QCOMPARE(tipsOn(2), QStringList{right->id});
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
