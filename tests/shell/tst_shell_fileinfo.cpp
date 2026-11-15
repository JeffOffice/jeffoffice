// The File > Info page's two buttons for where the file is: Open File Location and Copy Path. Both are grey for a document that has not been saved; a file on another computer
// still has Copy Path, and Open File Location asks before it goes there.
#include "app/infopage.h"
#include "core/remotepath.h"
#include "../testrunner.h"

#include <QApplication>
#include <QClipboard>
#include <QPushButton>
#include <QTemporaryDir>
#include <private/qabstractfileengine_p.h>
#include <QtTest>

using namespace jp;

namespace {

QWidget *pageFor(const QString &path)
{
    InfoHost h;
    h.path = [path] { return path; };
    return buildInfoPage(nullptr, h);
}

struct Hooks {
    QStringList opened, asked;
    bool answer = true;
    Hooks()
    {
        fileLocationHooks().open = [this](const QString &file) { opened << file; };
        fileLocationHooks().confirm = [this](QWidget *, const QString &file) {
            asked << file;
            return answer;
        };
    }
    ~Hooks() { fileLocationHooks() = FileLocationHooks(); }
};

} // namespace

class ShellFileInfoTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void anotherComputerIsAShareName()
    {
        QVERIFY(isOnAnotherComputer(QStringLiteral("\\\\server\\share\\a.docx")));
        QVERIFY(isOnAnotherComputer(QStringLiteral("//server/share/a.docx")));
        QVERIFY(!isOnAnotherComputer(QStringLiteral("C:\\Users\\a.docx")));
        QVERIFY(!isOnAnotherComputer(QStringLiteral("/home/user/a.docx")));
        QVERIFY(!isOnAnotherComputer(QString()));
    }

    void bothButtonsAreGreyForAnUnsavedFile()
    {
        std::unique_ptr<QWidget> page(pageFor(QString()));
        auto *where = page->findChild<QPushButton *>(QStringLiteral("infoShowFolder"));
        auto *copy = page->findChild<QPushButton *>(QStringLiteral("infoCopyPath"));
        QVERIFY(where && copy);
        QCOMPARE(where->text(), QStringLiteral("Open File Location"));
        QVERIFY(!where->isEnabled());
        QVERIFY(!copy->isEnabled());
    }

    void aSavedFileOpensItsLocationAndCopiesItsPath()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("a.docx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.close();
        Hooks hooks;
        std::unique_ptr<QWidget> page(pageFor(path));
        auto *where = page->findChild<QPushButton *>(QStringLiteral("infoShowFolder"));
        auto *copy = page->findChild<QPushButton *>(QStringLiteral("infoCopyPath"));
        QVERIFY(where->isEnabled() && copy->isEnabled());
        where->click();
        QCOMPARE(hooks.opened, QStringList{path});
        QVERIFY(hooks.asked.isEmpty());   // (it is on this computer: no question)
        copy->click();
        QCOMPARE(QApplication::clipboard()->text(), pathForCopy(path));
    }

    // Nothing looks at a file on another computer (not a stat, an exists, or an open, nor a file object made for it) before the person agrees: that sends their credentials there.
    void aFileOnAnotherComputerIsNotTouchedBeforeThePersonAgrees()
    {
        struct Probe : QAbstractFileEngineHandler {
            mutable QStringList touched;
            std::unique_ptr<QAbstractFileEngine> create(const QString &name) const override
            {
                if (name.contains(QStringLiteral("probe-share"))) touched << name;
                return nullptr;
            }
        } probe;
        Hooks hooks;
        const QString path = QStringLiteral("//probe-share/share/a.docx");
        std::unique_ptr<QWidget> page(pageFor(path));
        auto *where = page->findChild<QPushButton *>(QStringLiteral("infoShowFolder"));
        auto *copy = page->findChild<QPushButton *>(QStringLiteral("infoCopyPath"));
        QVERIFY(where->isEnabled() && copy->isEnabled());   // (a path that is not empty is enough)
        copy->click();
        QVERIFY(QApplication::clipboard()->text().contains(QStringLiteral("probe-share")));
        hooks.answer = false;
        where->click();
        QCOMPARE(hooks.asked, QStringList{path});
        QVERIFY(hooks.opened.isEmpty());
        hooks.answer = true;
        where->click();
        QCOMPARE(hooks.opened, QStringList{path});
        QVERIFY2(probe.touched.isEmpty(), qPrintable(probe.touched.join(QLatin1Char(' '))));
    }
};

JP_TEST_CLASS(ShellFileInfoTests)
#include "tst_shell_fileinfo.moc"
