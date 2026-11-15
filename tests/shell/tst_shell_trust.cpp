// The Security of Options (libs/shell/app/trust.h): the choices and their defaults, how a file from outside is opened, the page every app shows, and that no choice
// reaches the refusals that stay (a link that starts a program, a file on another computer).
#include "app/filetasks.h"
#include "app/settings.h"
#include "app/trust.h"
#include "core/remotepath.h"
#include "office/hyperlink.h"
#include "../testrunner.h"

#include <QComboBox>
#include <QFile>
#include <QLabel>
#include <QTemporaryDir>
#include <QtTest>

#ifdef Q_OS_LINUX
#include <sys/xattr.h>
#endif
#ifdef Q_OS_MACOS
#include <sys/xattr.h>
#endif

using namespace jp;

namespace {

class ShellTrustTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void init() { cleanup(); }

    void cleanup()
    {
        Settings &st = Settings::get();
        for (const char *app : {"jeffpub", "jeffcrunch", "jeffdeck", "jeffwrite"}) st.setValue(QStringLiteral("trust/linkedFiles/") + QString::fromLatin1(app), QString());   // (an empty choice is the default)
        st.setValue(QStringLiteral("trust/linkedMedia"), QString());
        st.setValue(QStringLiteral("trust/outsideReadOnly"), true);
        trust::setOutsideStandIn({});
        trust::setAskStandIn({});
    }

    void theDefaultsAreTheCautiousOnes()
    {
        // JeffPub updates the pictures in the publication's folder as it always did; JeffCrunch asks
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffpub")), trust::Choice::Yes);
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffcrunch")), trust::Choice::Ask);
        QCOMPARE(trust::linkedMedia(), trust::Choice::Never);
        QVERIFY(trust::outsideFilesReadOnly());
        // (JeffPub has no question to put: a stored "ask" is an update)
        trust::setLinkedFiles(trust::Choice::Ask, QStringLiteral("jeffpub"));
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffpub")), trust::Choice::Yes);
    }

    void theChoicesAreKeptForTheWholeSuite()
    {
        trust::setLinkedFiles(trust::Choice::Never, QStringLiteral("jeffcrunch"));
        trust::setLinkedMedia(trust::Choice::Yes);
        trust::setOutsideFilesReadOnly(false);
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffcrunch")), trust::Choice::Never);
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffpub")), trust::Choice::Yes);   // (each app has its own)
        QCOMPARE(trust::linkedMedia(), trust::Choice::Yes);
        QVERIFY(!trust::outsideFilesReadOnly());
        for (const char *key : {"trust/linkedFiles/jeffcrunch", "trust/linkedMedia", "trust/outsideReadOnly"}) QVERIFY2(suiteWide(QString::fromLatin1(key)), key);
        // (a value that is not one of ours is the default, not a stranger's choice)
        Settings::get().setValue(QStringLiteral("trust/linkedFiles/jeffcrunch"), QStringLiteral("whatever"));
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffcrunch")), trust::Choice::Ask);
    }

    void aFileFromOutsideOpensForReadingUnlessTheChoiceSaysNormally()
    {
        trust::setOutsideStandIn([](const QString &path) { return path.endsWith(QLatin1String("downloaded.docx")); });
        bool held = false;
        QCOMPARE(trust::openHowFor(QStringLiteral("/x/downloaded.docx"), OpenHow::Normal, &held), OpenHow::ReadOnly);
        QVERIFY(held);
        QCOMPARE(trust::openHowFor(QStringLiteral("/x/mine.docx"), OpenHow::Normal, &held), OpenHow::Normal);
        QVERIFY(!held);
        // the other ways to open are the person's own asking
        for (OpenHow how : {OpenHow::Copy, OpenHow::NewWindow, OpenHow::Repair, OpenHow::ReadOnly}) {
            QCOMPARE(trust::openHowFor(QStringLiteral("/x/downloaded.docx"), how, &held), how);
            QVERIFY(!held);
        }
        trust::setOutsideFilesReadOnly(false);
        QCOMPARE(trust::openHowFor(QStringLiteral("/x/downloaded.docx"), OpenHow::Normal, &held), OpenHow::Normal);
        QVERIFY(!held);
    }

    void aFileOnAnotherComputerIsFromOutsideByItsNameAlone()
    {
        QVERIFY(trust::fromOutside(QStringLiteral("\\\\host\\share\\a.docx")));
        QVERIFY(trust::fromOutside(QStringLiteral("//host/share/a.docx")));
        QVERIFY(!trust::fromOutside(QStringLiteral("/home/someone/a.docx")));
        QVERIFY(!trust::fromOutside(QString()));
    }

#ifdef Q_OS_LINUX
    // The mark a browser leaves on a download (the address it came from) is the system's own word for "from the internet"; taking it off makes the file an ordinary one.
    void aDownloadsMarkIsFound()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString marked = dir.filePath(QStringLiteral("marked.docx")), plain = dir.filePath(QStringLiteral("plain.docx"));
        for (const QString &path : {marked, plain}) {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("x");
        }
        const QByteArray url = "https://example.com/a.docx";
        if (setxattr(QFile::encodeName(marked).constData(), "user.xdg.origin.url", url.constData(), size_t(url.size()), 0) != 0) QSKIP("this file system keeps no extended attributes (a tmpfs older than Linux 6.6 does not), so a mark can not be set");
        QVERIFY(trust::fromOutside(marked));
        QVERIFY(!trust::fromOutside(plain));
        QCOMPARE(trust::openHowFor(marked, OpenHow::Normal), OpenHow::ReadOnly);
        QCOMPARE(trust::openHowFor(plain, OpenHow::Normal), OpenHow::Normal);
        QVERIFY(trust::clearOutsideMark(marked));
        QVERIFY(!trust::fromOutside(marked));
    }
#else
    void aDownloadsMarkIsFound() { QSKIP("this checks the Linux mark (extended attributes); the Windows and Mac marks have tests of their own"); }
#endif

    // Each row is on for an app that has something of its kind, and says plainly that it does not apply in one that has none.
    void thePageHasARowForEachChoiceAndSaysWhenItDoesNotApply()
    {
        struct Case {
            const char *app;
            bool files, media;
        };
        for (const Case &c : {Case{"jeffpub", true, false}, Case{"jeffcrunch", true, false}, Case{"jeffdeck", false, true}, Case{"jeffwrite", false, false}}) {
            const trust::Applies a = trust::appliesTo(QString::fromLatin1(c.app));
            QCOMPARE(a.linkedFiles, c.files);
            QCOMPARE(a.linkedMedia, c.media);
            trust::TrustPage page(a);
            auto *files = page.findChild<QComboBox *>(QStringLiteral("trustLinkedFiles"));
            auto *media = page.findChild<QComboBox *>(QStringLiteral("trustLinkedMedia"));
            auto *outside = page.findChild<QComboBox *>(QStringLiteral("trustOutside"));
            QVERIFY(files && media && outside);
            QCOMPARE(files->isEnabled(), c.files);
            QCOMPARE(media->isEnabled(), c.media);
            QVERIFY(outside->isEnabled());
            auto text = [&page](const char *name) { return page.findChild<QLabel *>(QString::fromLatin1(name))->text(); };
            QCOMPARE(text("trustLinkedFilesNote").contains(QStringLiteral("doesn't apply")), !c.files);
            QCOMPARE(text("trustLinkedMediaNote").contains(QStringLiteral("doesn't apply")), !c.media);
            // two lines that are the same in every app: macros are kept and never run, and a link to a program is always refused
            QVERIFY(text("trustMacros").contains(QStringLiteral("never run")));
            QVERIFY(text("trustPrograms").contains(QStringLiteral("always refused")));
        }
    }

    void okKeepsTheChoicesOfTheRowsThatApply()
    {
        trust::setLinkedFiles(trust::Choice::Yes, QStringLiteral("jeffcrunch"));
        trust::setLinkedMedia(trust::Choice::Yes);
        // an app with only linked media: its page keeps the choice of the row it has, and leaves the others as the person set them in another app
        trust::TrustPage deck(trust::appliesTo(QStringLiteral("jeffdeck")));
        deck.findChild<QComboBox *>(QStringLiteral("trustLinkedMedia"))->setCurrentIndex(int(trust::Choice::Never));
        deck.findChild<QComboBox *>(QStringLiteral("trustOutside"))->setCurrentIndex(1);
        deck.apply();
        QCOMPARE(trust::linkedMedia(), trust::Choice::Never);
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffcrunch")), trust::Choice::Yes);
        QVERIFY(!trust::outsideFilesReadOnly());
        trust::TrustPage crunch(trust::appliesTo(QStringLiteral("jeffcrunch")));
        QCOMPARE(crunch.findChild<QComboBox *>(QStringLiteral("trustLinkedFiles"))->currentIndex(), int(trust::Choice::Yes));
        QCOMPARE(crunch.findChild<QComboBox *>(QStringLiteral("trustOutside"))->currentIndex(), 1);
        crunch.findChild<QComboBox *>(QStringLiteral("trustLinkedFiles"))->setCurrentIndex(int(trust::Choice::Ask));
        crunch.findChild<QComboBox *>(QStringLiteral("trustOutside"))->setCurrentIndex(0);
        crunch.apply();
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffcrunch")), trust::Choice::Ask);
        QCOMPARE(trust::linkedMedia(), trust::Choice::Never);
        QVERIFY(trust::outsideFilesReadOnly());
        // JeffPub's row has two choices (it has nothing to ask), and keeps its own choice
        trust::TrustPage pub(trust::appliesTo(QStringLiteral("jeffpub")));
        auto *files = pub.findChild<QComboBox *>(QStringLiteral("trustLinkedFiles"));
        QCOMPARE(files->count(), 2);
        QCOMPARE(files->currentIndex(), 0);   // (update them)
        files->setCurrentIndex(1);
        pub.apply();
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffpub")), trust::Choice::Never);
        QCOMPARE(trust::linkedFiles(QStringLiteral("jeffcrunch")), trust::Choice::Ask);
    }

    // \\?\C:\ and \\.\C:\ are a drive of this computer; only \\?\UNC\ (and a path of two slashes) is another computer.
    void aDriveInTheSystemsOwnFormIsLocalAndOnlyUncIsRemote()
    {
        for (const char *local : {"\\\\?\\C:\\media\\a.mp3", "\\\\.\\D:\\a.mp3", "\\\\?\\c:", "//?/C:/a.mp3", "C:\\a.mp3", "/home/a/b.mp3"})
            QVERIFY2(!isOnAnotherComputer(QString::fromLatin1(local)), local);
        for (const char *remote : {"\\\\?\\UNC\\host\\share\\a.mp3", "\\\\?\\unc\\host\\share", "\\\\host\\share\\a.mp3", "\\\\.\\pipe\\x", "\\\\?\\GLOBALROOT\\x", "//host/share/a.mp3"})
            QVERIFY2(isOnAnotherComputer(QString::fromLatin1(remote)), remote);
    }

    // A stream of a file (a colon after the drive letter) and the device names are no files to read; a drive letter's own colon, and names that only look like them, are fine.
    void streamsAndDeviceNamesAreSpecial()
    {
        for (const char *special : {"C:\\a\\b.mp3:stream", "a.mp3:Zone.Identifier", "CON.mp4", "con", "C:\\x\\NUL.txt", "dir/prn.wav", "AUX", "folder\\COM1.mp3", "lpt9.x", "C:\\a\\Com5", "nul . mp3", "\\\\?\\C:\\x\\CON.mp3"})
            QVERIFY2(isWindowsSpecialPath(QString::fromLatin1(special)), special);
        for (const char *plain : {"C:\\a\\b.mp3", "\\\\?\\C:\\a.mp3", "/C:/a.mp3", "console.mp3", "com10.mp3", "com0.wav", "lpt.x", "a b/c.mp3", "nulls.wav", "icon.mp3"})
            QVERIFY2(!isWindowsSpecialPath(QString::fromLatin1(plain)), plain);
        // (where a colon is an ordinary character, only the device names count)
        QVERIFY(!isWindowsSpecialPath(QStringLiteral("/home/a/b:c.mp3"), false));
        QVERIFY(isWindowsSpecialPath(QStringLiteral("/home/a/CON.mp3"), false));
    }

#ifdef Q_OS_WIN
    // The system's zone mark (3 is the internet), written as a browser does.
    void aWindowsZoneMarkIsFound()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString marked = dir.filePath(QStringLiteral("marked.docx")), plain = dir.filePath(QStringLiteral("plain.docx")), local = dir.filePath(QStringLiteral("local.docx"));
        for (const QString &path : {marked, plain, local}) {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("x");
        }
        QFile zone(marked + QStringLiteral(":Zone.Identifier"));
        QVERIFY(zone.open(QIODevice::WriteOnly));
        zone.write("[ZoneTransfer]\r\nZoneId=3\r\n");
        zone.close();
        QFile intranet(local + QStringLiteral(":Zone.Identifier"));
        QVERIFY(intranet.open(QIODevice::WriteOnly));
        intranet.write("[ZoneTransfer]\r\nZoneId=1\r\n");
        intranet.close();
        QVERIFY(trust::fromOutside(marked));
        QVERIFY(!trust::fromOutside(plain));
        QVERIFY(!trust::fromOutside(local));
        QVERIFY(trust::clearOutsideMark(marked));
        QVERIFY(!trust::fromOutside(marked));
    }
#else
    void aWindowsZoneMarkIsFound() { QSKIP("the zone mark is Windows's: this runs on the Windows machines"); }
#endif

#ifdef Q_OS_MACOS
    void aMacQuarantineMarkIsFound()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString marked = dir.filePath(QStringLiteral("marked.docx")), plain = dir.filePath(QStringLiteral("plain.docx"));
        for (const QString &path : {marked, plain}) {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("x");
        }
        const QByteArray value = "0083;5f0e0000;Browser;";
        QVERIFY(setxattr(QFile::encodeName(marked).constData(), "com.apple.quarantine", value.constData(), size_t(value.size()), 0, 0) == 0);
        QVERIFY(trust::fromOutside(marked));
        QVERIFY(!trust::fromOutside(plain));
        QVERIFY(trust::clearOutsideMark(marked));
        QVERIFY(!trust::fromOutside(marked));
    }
#else
    void aMacQuarantineMarkIsFound() { QSKIP("the quarantine mark is the Mac's: this runs on the Mac machines"); }
#endif

    // Enable Editing is for one file, this session; and a save in place takes the download mark off.
    void editingEnabledForOneFileOpensItNormallyAndItsMarkCanBeCleared()
    {
        trust::setOutsideStandIn([](const QString &) { return true; });
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString a = dir.filePath(QStringLiteral("a.docx")), b = dir.filePath(QStringLiteral("b.docx"));
        for (const QString &path : {a, b}) {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("x");
        }
        QCOMPARE(trust::openHowFor(a, OpenHow::Normal), OpenHow::ReadOnly);
        QVERIFY(!trust::editingAllowed(a));
        trust::allowEditing(a);
        QVERIFY(trust::editingAllowed(a));
        QCOMPARE(trust::openHowFor(a, OpenHow::Normal), OpenHow::Normal);
        QCOMPARE(trust::openHowFor(b, OpenHow::Normal), OpenHow::ReadOnly);   // (that file only)
        QVERIFY(!trust::clearOutsideMark(QStringLiteral("//host/share/a.docx")));   // (a file on another computer is not touched)
    }
};

} // namespace

JP_TEST_CLASS(ShellTrustTests)
#include "tst_shell_trust.moc"
