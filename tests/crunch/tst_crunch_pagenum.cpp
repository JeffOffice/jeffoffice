// Tests of the Page Number gallery in JeffCrunch: only the designs a printed header or footer can show are offered; a design is the left, center, or right part of the header or footer being
// edited (the pages in general, the even pages, or the first page) with &P and &N; it shows the right number on pages 1 to 3 and is saved and opened again; Remove Page Numbers; and Undo.
#include "tst_crunch_print.h"

#include "core/pagenumbers.h"
#include "sheet/headerfooter.h"
#include "../testrunner.h"

#include <QUndoStack>

using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

// What a header (footer) string shows on page `page` of `pages`.
QString shownOn(const QString &code, int page, int pages)
{
    HfContext c;
    c.page = page;
    c.pages = pages;
    c.sheet = QStringLiteral("Sheet1");
    c.file = QStringLiteral("book.xlsx");
    return headerFooterPreview(code, c).trimmed();
}

} // namespace

class CrunchPageNumberTests : public QObject {
    Q_OBJECT

    // The setup of sheet 0 with the header and footer texts given.
    static void setHeaderFooter(CrunchWindow &w, const std::function<void(HeaderFooter *)> &change)
    {
        PageSetup p = w.session()->pages().sheet(0);
        change(&p.headerFooter);
        QVERIFY(w.session()->setPageSetup(0, p, QStringLiteral("Header and footer")).ok);
    }

private Q_SLOTS:
    // The gallery offers the designs that are text only: the top and the bottom (plain, X of Y, bold, at the left, center, and right) and the caret's (plain, X of Y, bold); not a circle, a
    // box, a tab, a bar, a rule, or a margin. Its button is on the Page Layout tab and on the Header and Footer tab.
    void theGalleryOffersOnlyTheDesignsThatFit()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        const QVector<jp::GalleryItem> items = w.pageNumberItems();
        QCOMPARE(items.size(), 9 + 9 + 3);
        for (const jp::GalleryItem &it : items) {
            const std::optional<jp::PageNumberDesign> d = jp::pageNumberDesignById(it.id);
            QVERIFY(d);
            QVERIFY2(jp::pageNumberIsTextOnly(*d), qPrintable(it.id));
            QVERIFY(d->area != jp::PageNumberArea::Margin);
            QVERIFY(!jp::pageNumberHasBox(*d));
        }
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("top.circle.right")));
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("margin.left.bar")));
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("cursor.circle")));
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("nonsense")));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddHeader, QString());
        w.ribbon()->buildAllTabs();
        QVERIFY(w.findChildren<jp::GalleryButton *>(QStringLiteral("pageNumberButton")).size() >= 2);
        QVERIFY(w.act(QStringLiteral("hf.pageNumberRemove")));
        QVERIFY(w.act(QStringLiteral("hf.pageNumberFormat")));
    }

    // Each design is the text of the part it names; it shows the right number on pages 1 to 3; it is saved and opened again.
    void eachDesignIsTheTextOfItsPartAndShowsTheRightNumber()
    {
        for (const jp::PageNumberDesign &d : jp::pageNumberDesigns()) {
            if (d.area == jp::PageNumberArea::Cursor || !jp::crunch::offersPageNumberDesign(d)) continue;
            EditWindow e;
            QVERIFY(e.open(rowsWithFormulas(100, {})));
            CrunchWindow &w = e.win.w;
            const int steps = w.session()->undoStack()->count();
            QVERIFY2(w.insertPageNumberDesign(d.id), qPrintable(d.id));
            QCOMPARE(w.session()->undoStack()->count(), steps + 1);
            const HeaderFooter &hf = w.session()->pages().sheet(0).headerFooter;
            const bool header = d.area == jp::PageNumberArea::Top;
            const QString code = header ? hf.oddHeader : hf.oddFooter;
            QVERIFY2((header ? hf.oddFooter : hf.oddHeader).isEmpty(), qPrintable(d.id));
            QString parts[3];
            splitHeaderFooter(code, &parts[0], &parts[1], &parts[2]);
            const int own = d.side == jp::PageNumberSide::Left ? 0 : d.side == jp::PageNumberSide::Right ? 2 : 1;
            for (int i = 0; i < 3; ++i) QVERIFY2((i == own) == !parts[i].isEmpty(), qPrintable(d.id + QStringLiteral(" ") + code));
            QVERIFY(parts[own].contains(QLatin1String("&P")));
            QCOMPARE(parts[own].contains(QLatin1String("&N")), d.look == jp::PageNumberLook::OfCount);
            QCOMPARE(parts[own].contains(QLatin1String("&B")), d.look == jp::PageNumberLook::Bold);
            for (int page = 1; page <= 3; ++page)
                QCOMPARE(shownOn(code, page, 3), d.look == jp::PageNumberLook::OfCount ? QStringLiteral("Page %1 of 3").arg(page) : QString::number(page));
            QSignalSpy saved(&w, &CrunchWindow::saved);
            QVERIFY(w.save());
            QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
            EditWindow again;
            QVERIFY(again.win.open(e.copyOfFile()));
            const HeaderFooter &back = again.win.w.session()->pages().sheet(0).headerFooter;
            QCOMPARE(header ? back.oddHeader : back.oddFooter, code);
            // Undo takes it away, in one step.
            w.session()->undoStack()->undo();
            QCOMPARE(w.session()->undoStack()->count(), steps + 1);
            QVERIFY((header ? w.session()->pages().sheet(0).headerFooter.oddHeader : w.session()->pages().sheet(0).headerFooter.oddFooter).isEmpty());
        }
    }

    // A design goes to the part it names and empties the parts that held only page numbers, so the number is not there twice; the parts with other words keep them.
    void aDesignKeepsTheOtherWordsAndReplacesTheOtherNumbers()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        setHeaderFooter(w, [](HeaderFooter *hf) { hf->oddFooter = QStringLiteral("&LDraft&CPage &P of &N"); });
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.plain.right")));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddFooter, QStringLiteral("&LDraft&R&P"));
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.ofCount.left")));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddFooter, QStringLiteral("&LPage &P of &N"));   // (the left part held "Draft": it is the design's now, as the design says)
    }

    // The version being edited is the one that gets the design: the first page's, an even page's, or the pages' in general.
    void theDesignGoesToTheKindOfPageBeingEdited()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        setHeaderFooter(w, [](HeaderFooter *hf) {
            hf->differentFirst = true;
            hf->differentOddEven = true;
        });
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        // The first page: its own.
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(0, true, 1).center());
        QVERIFY(w.zoneEditor());
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("top.plain.left")));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.firstHeader, QStringLiteral("&L&P"));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.evenHeader, QString());
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddHeader, QString());
        // The second page (an even one).
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->scrollToPage(1);
        w.pageView()->drawPagesNow();
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(1, true, 1).center());
        QVERIFY(w.zoneEditor());
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("top.plain.right")));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.evenHeader, QStringLiteral("&R&P"));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddHeader, QString());
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.firstHeader, QStringLiteral("&L&P"));
        // The third page (an odd one, not the first).
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->scrollToPage(2);
        w.pageView()->drawPagesNow();
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(2, false, 1).center());
        QVERIFY(w.zoneEditor());
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.ofCount.center")));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddFooter, QStringLiteral("&CPage &P of &N"));
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.firstFooter, QString());
    }

    // The designs of the caret's place go into the part being typed in, with their codes; without a part being typed in nothing is done.
    void theCaretsDesignsGoIntoThePartBeingTypedIn()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("cursor.plain")));
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(0, true, 1).center());
        QVERIFY(w.zoneEditor());
        QTest::keyClicks(w.zoneEditor(), "Report ");
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("cursor.ofCount")));
        QCOMPARE(w.zoneEditor()->toPlainText(), QStringLiteral("Report Page &P of &N"));
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("cursor.bold")));
        QVERIFY(w.zoneEditor()->toPlainText().endsWith(QStringLiteral("&B&P")));
        QTest::keyClick(w.zoneEditor(), Qt::Key_Escape);
    }

    // Remove Page Numbers takes the codes of the number and the count, and the words round them that are left alone, out of every header and footer of the sheet, and leaves the other
    // text; one step of Undo.
    void removePageNumbersLeavesTheOtherText()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        QCOMPARE(w.removePageNumbers(), 0);
        setHeaderFooter(w, [](HeaderFooter *hf) {
            hf->differentFirst = true;
            hf->oddHeader = QStringLiteral("&LDraft&CPage &P of &N&R&P");
            hf->firstFooter = QStringLiteral("&LBudget &P");
            hf->oddFooter = QStringLiteral("&CPage &P");
            hf->evenFooter = QStringLiteral("&A");
        });
        const int steps = w.session()->undoStack()->count();
        QCOMPARE(w.removePageNumbers(), 3);
        QCOMPARE(w.session()->undoStack()->count(), steps + 1);
        const HeaderFooter &hf = w.session()->pages().sheet(0).headerFooter;
        QCOMPARE(hf.oddHeader, QStringLiteral("&LDraft"));
        QCOMPARE(hf.firstFooter, QStringLiteral("&LBudget "));
        QCOMPARE(hf.oddFooter, QString());
        QCOMPARE(hf.evenFooter, QStringLiteral("&A"));
        w.session()->undoStack()->undo();
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddFooter, QStringLiteral("&CPage &P"));
        QCOMPARE(w.session()->undoStack()->count(), steps + 1);
    }

    // The Page Setup dialog's Header/Footer tab has the gallery beside the header and the footer: a design chosen there is the text of every page.
    void thePageSetupDialogOffersTheDesignsToo()
    {
        PageSetupRequest request;
        request.sheetName = QStringLiteral("Q3 Sales");
        request.fileName = QStringLiteral("Budget.xlsx");
        request.tab = 2;
        PageSetupDialog dialog(request);
        auto *header = dialog.findChild<jp::GalleryButton *>(QStringLiteral("psHeaderPageNumber"));
        auto *footer = dialog.findChild<jp::GalleryButton *>(QStringLiteral("psFooterPageNumber"));
        QVERIFY(header && footer);
        QCOMPARE(header->items().size(), 9);
        QCOMPARE(footer->items().size(), 9);
        for (const jp::GalleryItem &it : header->items()) QVERIFY(it.id.startsWith(QLatin1String("top.")));
        for (const jp::GalleryItem &it : footer->items()) QVERIFY(it.id.startsWith(QLatin1String("bottom.")));
        Q_EMIT header->activated(QStringLiteral("top.ofCount.center"));
        Q_EMIT footer->activated(QStringLiteral("bottom.bold.right"));
        dialog.accept();
        const HeaderFooter hf = dialog.setup().headerFooter;
        QCOMPARE(hf.oddHeader, QStringLiteral("&CPage &P of &N"));
        QCOMPARE(hf.oddFooter, QStringLiteral("&R&B&22&P"));
    }
};

JP_TEST_CLASS(CrunchPageNumberTests)
#include "tst_crunch_pagenum.moc"
