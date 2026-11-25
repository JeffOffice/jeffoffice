// The Clipboard pane every app shares (libs/shell/app/clipboardpane.h) and the history behind it (clipboardhistory.h): what is kept and what is never kept, the bounds, a click that pastes,
// and a history that goes with the program. The tests never touch the real clipboard but where they say so, and put back what was there.
#include "app/clipboardhistory.h"
#include "app/clipboardpane.h"
#include "../testrunner.h"

#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QClipboard>
#include <QDockWidget>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMimeData>
#include <QPushButton>
#include <QtTest>

using namespace jp;

namespace {

QMimeData *textCopy(const QString &text)
{
    auto *md = new QMimeData;
    md->setText(text);
    return md;
}

// The history of the test, started over, collecting.
struct Fresh {
    ClipboardHistory *h = nullptr;
    Fresh()
    {
        delete ClipboardHistory::instance();
        h = ClipboardHistory::instance();
        h->setCollectWhenClosed(false);
        h->addOpenPane(1);
    }
    ~Fresh()
    {
        h->setCollectWhenClosed(false);
        delete h;
    }
};

// What the clipboard held, put back at the end.
struct ClipboardGuard {
    QMimeData *was = new QMimeData;
    ClipboardGuard()
    {
        if (const QMimeData *now = QApplication::clipboard()->mimeData())
            for (const QString &f : now->formats()) was->setData(f, now->data(f));
    }
    ~ClipboardGuard() { QApplication::clipboard()->setMimeData(was); }
};

QString textOnClipboard()
{
    return QApplication::clipboard()->text();
}

} // namespace

class ShellClipboardPaneTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // 24 items, the newest first; the 25th copy takes the oldest out.
    void itWillKeepTheLast24NewestFirst()
    {
        Fresh f;
        for (int i = 0; i < 30; ++i) {
            const std::unique_ptr<QMimeData> md(textCopy(QStringLiteral("copy %1").arg(i)));
            QVERIFY(f.h->noteClipboard(md.get(), true) != 0);
        }
        QCOMPARE(f.h->items().size(), 24);
        QCOMPARE(f.h->items().first().preview, QStringLiteral("copy 29"));
        QCOMPARE(f.h->items().last().preview, QStringLiteral("copy 6"));
    }

    // Nothing is watched while no pane is open, unless the person set it so.
    void nothingIsKeptWhileThePaneIsClosedUnlessAskedFor()
    {
        Fresh f;
        f.h->addOpenPane(-1);
        const std::unique_ptr<QMimeData> md(textCopy(QStringLiteral("closed")));
        QCOMPARE(f.h->noteClipboard(md.get(), true), quint64(0));
        QVERIFY(f.h->items().isEmpty());
        f.h->setCollectWhenClosed(true);
        QVERIFY(f.h->noteClipboard(md.get(), true) != 0);
        QCOMPARE(f.h->items().size(), 1);
    }

    // The clipboard is watched only while something collects: a copy made through the real clipboard shows up when a pane is open, and not after it is closed.
    void theClipboardIsWatchedOnlyWhileCollecting()
    {
        ClipboardGuard guard;
        Fresh f;
        f.h->setOwnerCheckForTest([] { return true; });
        QApplication::clipboard()->setMimeData(textCopy(QStringLiteral("seen")));
        QTRY_COMPARE(f.h->items().size(), 1);
        QCOMPARE(f.h->items().first().preview, QStringLiteral("seen"));
        f.h->addOpenPane(-1);
        QApplication::clipboard()->setMimeData(textCopy(QStringLiteral("not seen")));
        for (int turn = 0; turn < 5; ++turn) QCoreApplication::processEvents();
        QCOMPARE(f.h->items().size(), 1);
    }

    // What other programs copy is kept only when asked for, and then only as plain text and a picture.
    void otherProgramsAreKeptOnlyWhenAskedAndOnlyAsTextAndPictures()
    {
        Fresh f;
        auto *md = new QMimeData;
        md->setText(QStringLiteral("from elsewhere"));
        md->setHtml(QStringLiteral("<b>from elsewhere</b>"));
        md->setData(QStringLiteral("application/x-other"), QByteArray("some bytes"));
        const std::unique_ptr<QMimeData> keep(md);
        QCOMPARE(f.h->noteClipboard(md, false), quint64(0));
        QVERIFY(f.h->items().isEmpty());
        if (!ClipboardHistory::canCollectOtherPrograms()) {
            f.h->setCollectOtherPrograms(true);
            QVERIFY(!f.h->collectOtherPrograms());
            return;
        }
        f.h->setCollectOtherPrograms(true);
        QVERIFY(f.h->noteClipboard(md, false) != 0);
        QCOMPARE(f.h->items().size(), 1);
        const ClipboardHistory::Item &it = f.h->items().first();
        QVERIFY(it.fromOtherProgram);
        QCOMPARE(it.formats.size(), 1);
        QCOMPARE(it.formats.first().first, QStringLiteral("text/plain"));
    }

    // Anything marked as secret is never kept, from this program or another, with the option on or off.
    void secretsAreNeverKept()
    {
        Fresh f;
        f.h->setCollectOtherPrograms(true);
        for (const char *hint : {"x-kde-passwordManagerHint", "org.nspasteboard.ConcealedType", "application/x-qt-windows-mime;value=\"ExcludeClipboardContentFromMonitorProcessing\""}) {
            for (bool own : {true, false}) {
                auto *md = new QMimeData;
                md->setText(QStringLiteral("hunter2"));
                md->setData(QLatin1String(hint), QByteArray("secret"));
                const std::unique_ptr<QMimeData> keep(md);
                QVERIFY(ClipboardHistory::isSecret(md));
                QCOMPARE(f.h->noteClipboard(md, own), quint64(0));
            }
        }
        // A Windows copy that says it may not go in a history.
        auto *md = new QMimeData;
        md->setText(QStringLiteral("hunter3"));
        md->setData(QStringLiteral("application/x-qt-windows-mime;value=\"CanIncludeInClipboardHistory\""), QByteArray(4, '\0'));
        const std::unique_ptr<QMimeData> keep(md);
        QCOMPARE(f.h->noteClipboard(md, true), quint64(0));
        QVERIFY(f.h->items().isEmpty());
    }

    // An item over 8 MB is not kept, and the pane says so; the total stays under 64 MB, the oldest dropped.
    void anItemIsBoundedBeforeItIsKept()
    {
        Fresh f;
        QSignalSpy large(f.h, &ClipboardHistory::tooLarge);
        auto *big = new QMimeData;
        big->setData(QStringLiteral("application/x-test"), QByteArray(int(ClipboardHistory::kMostItemBytes) + 1, 'x'));
        const std::unique_ptr<QMimeData> keepBig(big);
        QCOMPARE(f.h->noteClipboard(big, true), quint64(0));
        QCOMPARE(large.count(), 1);
        QCOMPARE(f.h->tooLargeCount(), 1);
        QVERIFY(f.h->items().isEmpty());
        // Ten copies of 7 MB: at most nine fit in 64 MB.
        for (int i = 0; i < 10; ++i) {
            auto *md = new QMimeData;
            md->setData(QStringLiteral("application/x-test"), QByteArray(7 << 20, char('a' + i)));
            const std::unique_ptr<QMimeData> keep(md);
            QVERIFY(f.h->noteClipboard(md, true) != 0);
        }
        QCOMPARE(f.h->items().size(), 9);
        QVERIFY(f.h->totalBytes() <= ClipboardHistory::kMostTotalBytes);
        QCOMPARE(f.h->droppedCount(), 1);
    }

    // A picture from another program is not decoded past 40 million pixels, and a small one is kept with a thumbnail of at most 64 pixels.
    void aHugePictureFromAnotherProgramIsNotDecoded()
    {
        if (!ClipboardHistory::canCollectOtherPrograms()) QSKIP("other programs are not collected here");
        Fresh f;
        f.h->setCollectOtherPrograms(true);
        QImage tall(7000, 7000, QImage::Format_Mono);
        tall.fill(0);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        QVERIFY(tall.save(&buffer, "PNG"));
        auto *md = new QMimeData;
        md->setData(QStringLiteral("image/png"), png);
        const std::unique_ptr<QMimeData> keep(md);
        QCOMPARE(f.h->noteClipboard(md, false), quint64(0));
        QCOMPARE(f.h->tooLargeCount(), 1);
        QImage small(300, 100, QImage::Format_RGB32);
        small.fill(Qt::red);
        QByteArray png2;
        QBuffer b2(&png2);
        b2.open(QIODevice::WriteOnly);
        QVERIFY(small.save(&b2, "PNG"));
        auto *md2 = new QMimeData;
        md2->setData(QStringLiteral("image/png"), png2);
        const std::unique_ptr<QMimeData> keep2(md2);
        QVERIFY(f.h->noteClipboard(md2, false) != 0);
        const ClipboardHistory::Item &it = f.h->items().first();
        QCOMPARE(it.kind, ClipboardHistory::Kind::Picture);
        QVERIFY(!it.thumbnail.isNull() && it.thumbnail.width() <= 64 && it.thumbnail.height() <= 64);
    }

    // The kinds, from the forms each app puts on the clipboard.
    void eachCopyHasItsKind()
    {
        Fresh f;
        auto make = [&](const char *form, bool text, bool html) {
            auto *md = new QMimeData;
            if (text) md->setText(QStringLiteral("some words"));
            if (html) md->setHtml(QStringLiteral("<b>x</b>"));
            if (form) md->setData(QLatin1String(form), QByteArray("1"));
            const std::unique_ptr<QMimeData> keep(md);
            return f.h->noteClipboard(md, true);
        };
        QVERIFY(make("application/x-jeffdeck-slides", false, false) != 0);
        QCOMPARE(f.h->items().first().kind, ClipboardHistory::Kind::Slides);
        QVERIFY(make("application/x-jeffcrunch-cells", true, true) != 0);
        QCOMPARE(f.h->items().first().kind, ClipboardHistory::Kind::Cells);
        QVERIFY(make("application/x-jeffpub-items", false, false) != 0);
        QCOMPARE(f.h->items().first().kind, ClipboardHistory::Kind::Shapes);
        QVERIFY(make(nullptr, true, true) != 0);
        QCOMPARE(f.h->items().first().kind, ClipboardHistory::Kind::FormattedText);
        QVERIFY(make(nullptr, true, false) != 0);
        QCOMPARE(f.h->items().first().kind, ClipboardHistory::Kind::Text);
    }

    // The same copy twice is one item, brought to the front.
    void theSameCopyIsKeptOnce()
    {
        Fresh f;
        const std::unique_ptr<QMimeData> a(textCopy(QStringLiteral("a"))), b(textCopy(QStringLiteral("b")));
        f.h->noteClipboard(a.get(), true);
        f.h->noteClipboard(b.get(), true);
        QCOMPARE(f.h->noteClipboard(b.get(), true), quint64(0));
        f.h->noteClipboard(a.get(), true);
        QCOMPARE(f.h->items().size(), 2);
        QCOMPARE(f.h->items().first().preview, QStringLiteral("a"));
    }

    // The history goes with the program's last window.
    void theHistoryEndsWithTheLastWindow()
    {
        Fresh f;
        const std::unique_ptr<QMimeData> a(textCopy(QStringLiteral("a")));
        f.h->noteClipboard(a.get(), true);
        QCOMPARE(f.h->items().size(), 1);
        Q_EMIT qApp->lastWindowClosed();
        QVERIFY(f.h->items().isEmpty());
        QCOMPARE(f.h->totalBytes(), qint64(0));
    }

    // The pane: rows with the kind, newest first; a real click puts the item on the clipboard and pastes it once; Paste All goes oldest first; delete and Clear All.
    void theListPastesWithAClickAndKeepsItsOtherJobs()
    {
        ClipboardGuard guard;
        Fresh f;
        QStringList pasted;
        ClipboardPaneHost host;
        host.paste = [&pasted] { pasted << textOnClipboard(); };
        int focused = 0;
        host.focusBack = [&focused] { ++focused; };
        ClipboardPane pane(host);
        pane.resize(320, 420);
        pane.show();
        QVERIFY(QTest::qWaitForWindowExposed(&pane));
        for (const char *w : {"one", "two", "three"}) {
            const std::unique_ptr<QMimeData> md(textCopy(QLatin1String(w)));
            f.h->noteClipboard(md.get(), true);
        }
        QCOMPARE(pane.list()->count(), 3);
        QVERIFY(pane.list()->item(0)->text().startsWith(ClipboardPane::kindName(ClipboardHistory::Kind::Text)));
        QVERIFY(pane.list()->item(0)->text().contains(QStringLiteral("three")));
        QVERIFY(pane.list()->item(2)->text().contains(QStringLiteral("one")));
        // A click on the middle row, a third of the way in.
        const QRect r = pane.list()->visualItemRect(pane.list()->item(1));
        const QPoint inside(r.left() + r.width() / 3, r.center().y());
        QTest::mouseMove(pane.list()->viewport(), inside);
        QTest::mouseClick(pane.list()->viewport(), Qt::LeftButton, Qt::NoModifier, inside);
        QCOMPARE(pasted, QStringList{QStringLiteral("two")});
        QCOMPARE(focused, 1);
        // Paste All: oldest first.
        pasted.clear();
        QTest::mouseClick(pane.pasteAllButton(), Qt::LeftButton);
        QCOMPARE(pasted, (QStringList{QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three")}));
        // Delete the current row, then Clear All.
        pane.list()->setCurrentRow(0);
        QTest::mouseClick(pane.deleteButton(), Qt::LeftButton);
        QCOMPARE(pane.list()->count(), 2);
        QCOMPARE(f.h->items().size(), 2);
        QTest::mouseClick(pane.clearAllButton(), Qt::LeftButton);
        QCOMPARE(pane.list()->count(), 0);
        QVERIFY(f.h->items().isEmpty());
        QVERIFY(!pane.statusLabel()->isVisibleTo(&pane));
    }

    // Putting an item back is an ordinary copy of it, and the history does not add it again.
    void anItemPutBackIsNotAddedAgain()
    {
        ClipboardGuard guard;
        Fresh f;
        f.h->setOwnerCheckForTest([] { return true; });
        for (const char *w : {"x", "y"}) {
            const std::unique_ptr<QMimeData> md(textCopy(QLatin1String(w)));
            f.h->noteClipboard(md.get(), true);
        }
        const quint64 older = f.h->items().last().id;
        QVERIFY(f.h->putOnClipboard(older));
        QCOMPARE(textOnClipboard(), QStringLiteral("x"));
        for (int turn = 0; turn < 5; ++turn) QCoreApplication::processEvents();
        QCOMPARE(f.h->items().size(), 2);
        QCOMPARE(f.h->items().first().preview, QStringLiteral("y"));
        QVERIFY(!f.h->putOnClipboard(99999));
    }

    // The options of the pane reach the history, and an item that was too large is said.
    void theOptionsAndTheSizeNotice()
    {
        Fresh f;
        ClipboardPaneHost host;
        ClipboardPane pane(host);
        pane.show();
        QVERIFY(!pane.collectClosedBox()->isChecked());
        pane.collectClosedBox()->setChecked(true);
        QVERIFY(f.h->collectWhenClosed());
        pane.collectClosedBox()->setChecked(false);
        QVERIFY(!f.h->collectWhenClosed());
        QVERIFY(!pane.collectOthersBox()->isChecked());
        if (ClipboardHistory::canCollectOtherPrograms()) {
            pane.collectOthersBox()->setChecked(true);
            QVERIFY(f.h->collectOtherPrograms());
        } else {
            QVERIFY(pane.collectOthersBox()->isHidden());
        }
        auto *big = new QMimeData;
        big->setData(QStringLiteral("application/x-test"), QByteArray(int(ClipboardHistory::kMostItemBytes) + 1, 'x'));
        const std::unique_ptr<QMimeData> keep(big);
        f.h->noteClipboard(big, true);
        QVERIFY(pane.statusLabel()->isVisibleTo(&pane));
        QVERIFY(pane.statusLabel()->text().contains(QStringLiteral("8 MB")));
    }

    // The dock counts as open for the history while it is shown, and not after it is closed.
    void theDockOpensAndClosesTheCollecting()
    {
        delete ClipboardHistory::instance();
        ClipboardHistory *h = ClipboardHistory::instance();
        QMainWindow win;
        win.show();
        QVERIFY(QTest::qWaitForWindowExposed(&win));
        QVERIFY(!h->paneOpen());
        showClipboardPane(&win, ClipboardPaneHost());
        QDockWidget *dock = win.findChild<QDockWidget *>(QStringLiteral("clipboardDock"));
        QVERIFY(dock);
        QVERIFY(dock->isVisible());
        QVERIFY(h->paneOpen());
        QCOMPARE(dock->windowTitle(), QStringLiteral("Clipboard"));
        dock->close();
        QVERIFY(!h->paneOpen());
        QVERIFY(!h->collecting());
        delete h;
    }
};

JP_TEST_CLASS(ShellClipboardPaneTests)
#include "tst_shell_clipboardpane.moc"
