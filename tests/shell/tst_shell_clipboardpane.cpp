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
#include <QSettings>
#include <QtTest>
#include <any>

using namespace jp;

namespace {

QMimeData *textCopy(const QString &text)
{
    auto *md = new QMimeData;
    md->setText(text);
    return md;
}

// The setting "keep collecting when the pane is closed" as the test found it.
struct CollectSettingGuard {
    QVariant was = QSettings().value(QStringLiteral("clipboard/collectWhenClosed"));
    ~CollectSettingGuard()
    {
        if (was.isValid()) QSettings().setValue(QStringLiteral("clipboard/collectWhenClosed"), was);
        else QSettings().remove(QStringLiteral("clipboard/collectWhenClosed"));
    }
};

// The history of the test, started over, collecting.
struct Fresh {
    CollectSettingGuard setting;
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

// An offer that never holds the bytes: it counts what a read hands over, which is at most what was asked for (a bounded read), or nothing when the size was told beforehand.
class CountingSource : public ClipboardSource {
public:
    QMap<QString, qint64> offered;   // form -> bytes offered
    bool tellSizes = false;
    mutable qint64 handed = 0;
    mutable QStringList asked;
    QByteArray smallPng;
    QStringList formats() const override { return offered.keys(); }
    qint64 sizeOf(const QString &format) const override { return tellSizes ? offered.value(format) : -1; }
    QByteArray read(const QString &format, qint64 most, bool *over) const override
    {
        asked << format;
        if (!smallPng.isEmpty() && format.startsWith(QLatin1String("image/"))) {
            handed += smallPng.size();
            return smallPng;
        }
        const qint64 total = offered.value(format);
        handed += std::min(total, most + 1);
        if (total > most) {
            *over = true;
            return QByteArray();
        }
        return QByteArray(int(total), 'x');
    }
};

// A mime data that says what was fetched from it, as the platform's offer does.
class FetchLogMime : public QMimeData {
public:
    QMap<QString, QByteArray> have;
    mutable QStringList fetched;
    QStringList formats() const override { return have.keys(); }

protected:
    QVariant retrieveData(const QString &format, QMetaType) const override
    {
        fetched << format;
        return have.value(format);
    }
};

QByteArray pngOf(int w, int h, QImage::Format format)
{
    QImage image(w, h, format);
    image.fill(0);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return png;
}

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

    // Another program's copy of 300 MB is never fetched whole: the read stops at the cap, and a size told beforehand refuses it without any read.
    void anOfferFromAnotherProgramIsReadOnlyUpToTheCap()
    {
        if (!ClipboardHistory::canCollectOtherPrograms()) QSKIP("other programs are not collected here");
        Fresh f;
        f.h->setCollectOtherPrograms(true);
        CountingSource text;
        text.offered.insert(QStringLiteral("text/plain"), qint64(300) << 20);
        QCOMPARE(f.h->noteSource(text, false), quint64(0));
        QCOMPARE(f.h->tooLargeCount(), 1);
        QVERIFY(text.handed <= ClipboardHistory::kMostItemBytes + 1);
        CountingSource told;
        told.tellSizes = true;
        told.offered.insert(QStringLiteral("text/plain"), qint64(300) << 20);
        QCOMPARE(f.h->noteSource(told, false), quint64(0));
        QCOMPARE(told.handed, qint64(0));
        QVERIFY(told.asked.isEmpty());
        QCOMPARE(f.h->tooLargeCount(), 2);
        // A big picture form the same way.
        CountingSource picture;
        picture.offered.insert(QStringLiteral("image/png"), qint64(300) << 20);
        QCOMPARE(f.h->noteSource(picture, false), quint64(0));
        QVERIFY(picture.handed <= ClipboardHistory::kMostItemBytes + 1);
        QCOMPARE(f.h->tooLargeCount(), 3);
    }

    // A copy made here is checked by the size it tells before its bytes are taken.
    void anOwnCopyIsRefusedBySizeBeforeAnyByteIsTaken()
    {
        Fresh f;
        CountingSource own;
        own.tellSizes = true;
        own.offered.insert(QStringLiteral("application/x-test"), qint64(300) << 20);
        QCOMPARE(f.h->noteSource(own, true), quint64(0));
        QCOMPARE(own.handed, qint64(0));
        QCOMPARE(f.h->tooLargeCount(), 1);
    }

    // Of what another program offers, only the plain text and one picture form are fetched.
    void onlyTheTwoFormsKeptAreFetchedFromAnotherProgram()
    {
        if (!ClipboardHistory::canCollectOtherPrograms()) QSKIP("other programs are not collected here");
        Fresh f;
        f.h->setCollectOtherPrograms(true);
        FetchLogMime md;
        md.have.insert(QStringLiteral("text/plain"), "hello");
        md.have.insert(QStringLiteral("text/html"), "<b>hello</b>");
        md.have.insert(QStringLiteral("application/x-other"), QByteArray(1000, 'z'));
        md.have.insert(QStringLiteral("image/png"), pngOf(20, 10, QImage::Format_RGB32));
        md.have.insert(QStringLiteral("image/jpeg"), QByteArray("not a picture"));
        QVERIFY(f.h->noteClipboard(&md, false) != 0);
        QStringList got = md.fetched;
        got.sort();
        QCOMPARE(got, (QStringList{QStringLiteral("image/png"), QStringLiteral("text/plain")}));
        QCOMPARE(f.h->items().first().preview, QStringLiteral("hello"));
    }

    // A picture's size is read from its header: a big one that would decode to less than 8 MB is still refused before it is decoded, and the limit is one number.
    void aPictureOverThePixelLimitIsRefusedFromItsHeader()
    {
        if (!ClipboardHistory::canCollectOtherPrograms()) QSKIP("other programs are not collected here");
        Fresh f;
        f.h->setCollectOtherPrograms(true);
        QCOMPARE(ClipboardHistory::kMostImagePixels * 4, ClipboardHistory::kMostItemBytes);
        CountingSource source;
        source.offered.insert(QStringLiteral("image/png"), 1);
        source.smallPng = pngOf(3000, 3000, QImage::Format_Mono);   // 9 million pixels, about 1 MB decoded, a few KB of file
        QVERIFY(source.smallPng.size() < 100000);
        QCOMPARE(f.h->noteSource(source, false), quint64(0));
        QCOMPARE(f.h->tooLargeCount(), 1);
        source.smallPng = pngOf(1000, 1000, QImage::Format_Mono);   // a million: kept
        QVERIFY(f.h->noteSource(source, false) != 0);
    }

    // A copy made here keeps what the pane needs to paste it back, not its picture form when that is large; and it keeps a bounded number of forms.
    void aLargePictureFormDoesNotDropAnOwnCopy()
    {
        Fresh f;
        auto *md = new QMimeData;
        md->setData(QStringLiteral("application/x-jeffcrunch-cells"), "cells");
        md->setText(QStringLiteral("a\tb"));
        QImage big(3000, 3000, QImage::Format_ARGB32);   // 36 MB
        big.fill(Qt::blue);
        md->setImageData(big);
        const std::unique_ptr<QMimeData> keep(md);
        QVERIFY(f.h->noteClipboard(md, true) != 0);
        QCOMPARE(f.h->tooLargeCount(), 0);
        const ClipboardHistory::Item item = f.h->items().first();
        QCOMPARE(item.kind, ClipboardHistory::Kind::Cells);
        QVERIFY(item.picture.isNull());
        QVERIFY(!item.thumbnail.isNull() && item.thumbnail.width() <= 64);
        QVERIFY(item.bytes < 1000);
        // A picture alone that is over the cap is not kept, and it is said.
        auto *only = new QMimeData;
        only->setImageData(big);
        const std::unique_ptr<QMimeData> keepOnly(only);
        QCOMPARE(f.h->noteClipboard(only, true), quint64(0));
        QCOMPARE(f.h->tooLargeCount(), 1);
    }

    void aCopyWithThousandsOfFormsKeepsAFewIncludingTheAppsOwn()
    {
        Fresh f;
        auto *md = new QMimeData;
        for (int i = 0; i < 10001; ++i) md->setData(QStringLiteral("application/x-filler-%1").arg(i), "x");
        md->setData(QStringLiteral("application/x-jeffcrunch-cells"), "cells");
        md->setText(QStringLiteral("text"));
        const std::unique_ptr<QMimeData> keep(md);
        QVERIFY(f.h->noteClipboard(md, true) != 0);
        const ClipboardHistory::Item item = f.h->items().first();
        QVERIFY(item.formats.size() <= ClipboardHistory::kMostFormats);
        QStringList names;
        for (const auto &p : item.formats) names << p.first;
        QVERIFY(names.contains(QStringLiteral("application/x-jeffcrunch-cells")));
        QVERIFY(names.contains(QStringLiteral("text/plain")));
        QCOMPARE(item.kind, ClipboardHistory::Kind::Cells);
    }

    // "Keep collecting when this pane is closed" is in force from the first copy of a new run, without the pane being opened; when it is off nothing is made.
    void collectingWhenClosedStartsWithTheApp()
    {
        ClipboardGuard guard;
        CollectSettingGuard setting;
        delete ClipboardHistory::instance();
        QSettings().setValue(QStringLiteral("clipboard/collectWhenClosed"), false);
        ClipboardHistory::startIfAsked();
        QVERIFY(!qApp->findChild<ClipboardHistory *>(QString(), Qt::FindDirectChildrenOnly));
        QSettings().setValue(QStringLiteral("clipboard/collectWhenClosed"), true);
        ClipboardHistory::startIfAsked();
        ClipboardHistory *h = qApp->findChild<ClipboardHistory *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(h);
        QVERIFY(!h->paneOpen());
        h->setOwnerCheckForTest([] { return true; });
        QApplication::clipboard()->setMimeData(textCopy(QStringLiteral("before any pane")));
        QTRY_COMPARE(h->items().size(), 1);
        delete h;
    }

    // The sentence under the list says what the history does: it goes with the last window, and an item stays after its document is closed.
    void theNoteSaysWhenItemsGo()
    {
        Fresh f;
        ClipboardPane pane{ClipboardPaneHost()};
        const QString note = pane.noteLabel()->text();
        QVERIFY2(note.contains(QStringLiteral("last window")), qPrintable(note));
        QVERIFY2(note.contains(QStringLiteral("after the document")), qPrintable(note));
        QVERIFY(!note.contains(QStringLiteral("until it is closed")));
    }

    // Putting an item back is not a new copy when the change notice comes after putOnClipboard has returned: the list does not move.
    void aLateChangeNoticeOfAnItemPutBackIsNotACopy()
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
        for (int turn = 0; turn < 3; ++turn) QCoreApplication::processEvents();
        Q_EMIT QApplication::clipboard()->dataChanged();   // (a system that tells late)
        QCOMPARE(f.h->items().first().preview, QStringLiteral("y"));
        QCOMPARE(f.h->items().size(), 2);
        // A real copy of the same words afterwards is a copy: it comes to the front.
        QApplication::clipboard()->setMimeData(textCopy(QStringLiteral("x")));
        QTRY_COMPARE(f.h->items().first().preview, QStringLiteral("x"));
    }

    // What an app keeps of its own copy outside the clipboard goes with the item and is given back when it is put back.
    void anItemCarriesWhatTheAppKeptOfItsCopy()
    {
        ClipboardGuard guard;
        Fresh f;
        f.h->setOwnerCheckForTest([] { return true; });
        QString state = QStringLiteral("newest");
        QStringList given;
        ClipboardHistory::setOwnStateHooks([&state](const QMimeData *) { return std::any(state); }, [&given](const std::any &a) { given << std::any_cast<QString>(a); });
        struct Reset {
            ~Reset() { ClipboardHistory::setOwnStateHooks(nullptr, nullptr); }
        } reset;
        for (const char *w : {"one", "two"}) {
            state = QLatin1String(w);
            const std::unique_ptr<QMimeData> md(textCopy(QLatin1String(w)));
            f.h->noteClipboard(md.get(), true);
        }
        QVERIFY(f.h->putOnClipboard(f.h->items().last().id));
        QCOMPARE(given, QStringList{QStringLiteral("one")});
    }

    // Two panes of one app (two windows) share the history and the count of open panes; Delete and Clear All reach both.
    void twoPanesShareOneHistory()
    {
        Fresh f;
        f.h->addOpenPane(-1);
        ClipboardPane a{ClipboardPaneHost()}, b{ClipboardPaneHost()};
        a.setShown(true);
        b.setShown(true);
        QVERIFY(f.h->paneOpen());
        for (const char *w : {"p", "q", "r"}) {
            const std::unique_ptr<QMimeData> md(textCopy(QLatin1String(w)));
            f.h->noteClipboard(md.get(), true);
        }
        QCOMPARE(a.list()->count(), 3);
        QCOMPARE(b.list()->count(), 3);
        a.list()->setCurrentRow(0);
        a.deleteCurrent();
        QCOMPARE(b.list()->count(), 2);
        a.setShown(false);
        QVERIFY(f.h->paneOpen());
        b.clearAll();
        QCOMPARE(a.list()->count(), 0);
        QVERIFY(f.h->items().isEmpty());
        b.setShown(false);
        QVERIFY(!f.h->paneOpen());
    }

    // A picture from another program is not decoded past the pixel limit, and a small one is kept with a thumbnail of at most 64 pixels.
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
