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

using namespace jp;

namespace {

class ShellTrustTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void init() { cleanup(); }

    void cleanup()
    {
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("trust/linkedFiles"), QString());   // (an empty choice is the default)
        st.setValue(QStringLiteral("trust/linkedMedia"), QString());
        st.setValue(QStringLiteral("trust/outsideReadOnly"), true);
        trust::setOutsideStandIn({});
        trust::setAskStandIn({});
    }

    void theDefaultsAreTheCautiousOnes()
    {
        QCOMPARE(trust::linkedFiles(), trust::Choice::Ask);
        QCOMPARE(trust::linkedMedia(), trust::Choice::Never);
        QVERIFY(trust::outsideFilesReadOnly());
    }

    void theChoicesAreKeptForTheWholeSuite()
    {
        trust::setLinkedFiles(trust::Choice::Never);
        trust::setLinkedMedia(trust::Choice::Yes);
        trust::setOutsideFilesReadOnly(false);
        QCOMPARE(trust::linkedFiles(), trust::Choice::Never);
        QCOMPARE(trust::linkedMedia(), trust::Choice::Yes);
        QVERIFY(!trust::outsideFilesReadOnly());
        for (const char *key : {"trust/linkedFiles", "trust/linkedMedia", "trust/outsideReadOnly"}) QVERIFY2(suiteWide(QString::fromLatin1(key)), key);
        // (a value that is not one of ours is the default, not a stranger's choice)
        Settings::get().setValue(QStringLiteral("trust/linkedFiles"), QStringLiteral("whatever"));
        QCOMPARE(trust::linkedFiles(), trust::Choice::Ask);
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
    // The mark a browser leaves on a download (the address it came from) is the system's own word for "from the internet".
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
        if (setxattr(QFile::encodeName(marked).constData(), "user.xdg.origin.url", url.constData(), size_t(url.size()), 0) != 0) QSKIP("this file system keeps no extended attributes");
        QVERIFY(trust::fromOutside(marked));
        QVERIFY(!trust::fromOutside(plain));
        QCOMPARE(trust::openHowFor(marked, OpenHow::Normal), OpenHow::ReadOnly);
        QCOMPARE(trust::openHowFor(plain, OpenHow::Normal), OpenHow::Normal);
    }
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
        trust::setLinkedFiles(trust::Choice::Yes);
        trust::setLinkedMedia(trust::Choice::Yes);
        // an app with only linked media: its page keeps the choice of the row it has, and leaves the others as the person set them in another app
        trust::TrustPage deck(trust::appliesTo(QStringLiteral("jeffdeck")));
        deck.findChild<QComboBox *>(QStringLiteral("trustLinkedMedia"))->setCurrentIndex(int(trust::Choice::Never));
        deck.findChild<QComboBox *>(QStringLiteral("trustOutside"))->setCurrentIndex(1);
        deck.apply();
        QCOMPARE(trust::linkedMedia(), trust::Choice::Never);
        QCOMPARE(trust::linkedFiles(), trust::Choice::Yes);
        QVERIFY(!trust::outsideFilesReadOnly());
        trust::TrustPage crunch(trust::appliesTo(QStringLiteral("jeffcrunch")));
        QCOMPARE(crunch.findChild<QComboBox *>(QStringLiteral("trustLinkedFiles"))->currentIndex(), int(trust::Choice::Yes));
        QCOMPARE(crunch.findChild<QComboBox *>(QStringLiteral("trustOutside"))->currentIndex(), 1);
        crunch.findChild<QComboBox *>(QStringLiteral("trustLinkedFiles"))->setCurrentIndex(int(trust::Choice::Ask));
        crunch.findChild<QComboBox *>(QStringLiteral("trustOutside"))->setCurrentIndex(0);
        crunch.apply();
        QCOMPARE(trust::linkedFiles(), trust::Choice::Ask);
        QCOMPARE(trust::linkedMedia(), trust::Choice::Never);
        QVERIFY(trust::outsideFilesReadOnly());
    }

    // The loosest of every choice does not reach what is always refused: a program or script as a link's target, in whatever way it is written, and a file on another computer.
    void noChoiceLoosensTheRefusals()
    {
        trust::setLinkedFiles(trust::Choice::Yes);
        trust::setLinkedMedia(trust::Choice::Yes);
        trust::setOutsideFilesReadOnly(false);
        trust::setAskStandIn([](const QString &) { return true; });
        for (const char *program : {"./run.exe", "./setup.bat", "C:\\tools\\x.cmd", "file:///usr/bin/x.sh", "\\\\host\\share\\run.exe", "./x.desktop", "./a.lnk", "./tool.js", "./page.hta"})
            QVERIFY2(office::isProgramFile(QString::fromLatin1(program)), program);
        for (const char *remote : {"\\\\host\\share\\a.pdf", "//host/share/a.pdf", "file://host/share/a.pdf", "file://///host/share/a.pdf", "\\\\?\\UNC\\host\\share\\a.pdf", "file:///%5C%5Chost/s/a.pdf"})
            QVERIFY2(isOnAnotherComputer(QString::fromLatin1(remote)), remote);
        QVERIFY(!isOnAnotherComputer(QStringLiteral("/home/a/b.pdf")));
    }
};

} // namespace

JP_TEST_CLASS(ShellTrustTests)
#include "tst_shell_trust.moc"
