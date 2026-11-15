// Tests of JeffWrite's Page Number gallery: each design put in a header or footer, in a margin, or at the caret; saved to and read back from .docx and .odt; laid out with the right
// numbers on pages 1 to 3; Page X of Y; a different first page and different odd and even pages; Remove Page Numbers; and Undo.

#include "tst_write.h"
#include "app/pagenumbergallery.h"
#include "core/pagenumberops.h"
#include "render/pdfpage.h"
#include "text/textengine.h"

#include <QTemporaryDir>

namespace {

// A document of `pages` pages, a page break between them.
QByteArray pagesBody(int pages)
{
    QByteArray body;
    for (int i = 0; i < pages; ++i) {
        body += "<w:p><w:r><w:t>Page text " + QByteArray::number(i + 1) + "</w:t></w:r></w:p>";
        if (i + 1 < pages) body += "<w:p><w:r><w:br w:type=\"page\"/></w:r></w:p>";
    }
    return body;
}

std::unique_ptr<WriteDoc> openPages(int pages)
{
    QString err;
    auto doc = WriteDoc::fromDocx(makeDocx(pagesBody(pages)), &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

// What the page shows in its header (or footer) and in the boxes anchored there, as laid out with the page's own number and count: the text of each of their stories, in the order of
// the ids, with a bar between.
QString shownOnPage(WriteDoc &doc, int index, const QString &own)
{
    doc.pager().pageCount();   // (the count of the pages is known only when all are laid out)
    doc.pager().paginate(index);
    const auto page = doc.pager().materialize(index);
    if (!page) return QStringLiteral("<no page>");
    QStringList out;
    for (auto it = page->doc->stories.cbegin(); it != page->doc->stories.cend(); ++it) {
        const QString id = it.key();
        if (!id.startsWith(own) && !id.startsWith(QLatin1String("flow-boxtext"))) continue;
        FrameSpec fs;
        fs.size = QSizeF(468, 600);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        fs.ctx.doc = page->doc.get();
        StoryLayout layout;
        layout.build(it.value()->doc.get(), {fs}, doc.flow().env);
        QStringList lines;
        for (const auto &l : layout.lineInfo(0)) lines << QString(l.text).remove(QChar(0x2060)).trimmed();
        const QString t = lines.join(QLatin1Char(' ')).trimmed();
        if (!t.isEmpty()) out << t;
    }
    return out.join(QStringLiteral(" | "));
}

} // namespace

// The gallery lists at least 12 designs for the top and the bottom, 6 for the margins, and 4 for the caret, each with an id and a name of its own and none named for another program.
void WriteTests::theGalleryListsTheDesigns()
{
    const QVector<PageNumberDesign> all = pageNumberDesigns();
    QMap<PageNumberArea, int> count;
    QSet<QString> ids, labels;
    for (const PageNumberDesign &d : all) {
        ++count[d.area];
        ids.insert(d.id);
        labels.insert(QString::number(int(d.area)) + d.label);
        QVERIFY2(!d.label.contains(QLatin1String("Microsoft")) && !d.label.contains(QLatin1String("Word")) && !d.label.contains(QLatin1String("Office")), qPrintable(d.label));
    }
    QVERIFY(count[PageNumberArea::Top] >= 12);
    QVERIFY(count[PageNumberArea::Bottom] >= 12);
    QCOMPARE(count[PageNumberArea::Margin], 6);
    QCOMPARE(count[PageNumberArea::Cursor], 4);
    QCOMPARE(ids.size(), all.size());
    QCOMPARE(labels.size(), all.size());
    const QVector<GalleryItem> items = pageNumberGalleryItems(QColor(0x2F, 0x55, 0x97));
    QCOMPARE(items.size(), all.size());
    QSet<QString> tips;
    for (const GalleryItem &it : items) tips.insert(it.tip);
    QCOMPARE(tips.size(), all.size());
    QStringList groups;
    for (const GalleryItem &it : items)
        if (!groups.contains(it.group)) groups << it.group;
    QCOMPARE(groups, (QStringList{pageNumberAreaName(PageNumberArea::Top), pageNumberAreaName(PageNumberArea::Bottom), pageNumberAreaName(PageNumberArea::Margin),
                                  pageNumberAreaName(PageNumberArea::Cursor)}));
    // (Each picture is drawn.)
    for (const GalleryItem &it : items) QVERIFY(!it.icon.pixmap(52, 64).toImage().isNull());
}

// Each design of the top, the bottom, and the margins is put in, laid out with the page's own number on pages 1 to 3, saved and read back as .docx and as .odt, and laid out again.
void WriteTests::eachDesignIsInsertedSavedReopenedAndLaidOutWithTheRightNumbers()
{
    int tried = 0;
    for (const PageNumberDesign &d : pageNumberDesigns()) {
        if (d.area == PageNumberArea::Cursor) continue;
        ++tried;
        const bool footer = d.area == PageNumberArea::Bottom;
        auto doc = openPages(3);
        QVERIFY2(insertPageNumberDesign(*doc, 0, d.id, PageNumberText::Replace), qPrintable(d.id));
        const QString own = footer ? QStringLiteral("flow-footer-flow") : QStringLiteral("flow-header-flow");
        const QString other = footer ? QStringLiteral("flow-header-flow") : QStringLiteral("flow-footer-flow");
        auto check = [&](WriteDoc &made, const QString &how) {
            for (int i = 0; i < 3; ++i) {
                const QString n = QString::number(i + 1);
                const QString want = d.look == PageNumberLook::OfCount ? QStringLiteral("Page %1 of 3").arg(n) : d.look == PageNumberLook::PageLine ? QStringLiteral("Page %1").arg(n) : n;
                const QString got = shownOnPage(made, i, own);
                QVERIFY2(got == want, qPrintable(QStringLiteral("%1 (%2) page %3 shows [%4], not [%5]").arg(d.id, how).arg(i + 1).arg(got, want)));
                // (Nothing is put in the other of the header and footer, but the boxes, which are the same for both.)
                if (!pageNumberHasBox(d)) QVERIFY2(shownOnPage(made, i, other).isEmpty(), qPrintable(d.id + QStringLiteral(" (") + how + QStringLiteral(") shows in the other too")));
            }
        };
        check(*doc, QStringLiteral("made"));
        QString err;
        const QByteArray docx = doc->toDocx();
        const QString partName = d.area == PageNumberArea::Bottom ? QStringLiteral("/word/footer1.xml") : QStringLiteral("/word/header1.xml");
        const QByteArray part = partOf(docx, partName);
        QVERIFY2(part.contains("PAGE"), qPrintable(d.id));
        if (d.look == PageNumberLook::OfCount) QVERIFY(part.contains("NUMPAGES"));
        auto back = WriteDoc::fromDocx(docx, &err);
        QVERIFY2(back, qPrintable(d.id + err));
        check(*back, QStringLiteral("reopened .docx"));
        auto odt = WriteDoc::fromOdt(doc->toOdt(&err), &err);
        QVERIFY2(odt, qPrintable(d.id + err));
        check(*odt, QStringLiteral("reopened .odt"));
    }
    QVERIFY(tried >= 38);
}

// The boxes of a design are saved as drawings anchored to the page, with the field in the text of the box, and the shape of a circle or a rounded box is a drawing of its own.
void WriteTests::theBoxesOfADesignAreAnchoredDrawingsWithTheFieldInThem()
{
    auto doc = openPages(3);
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("bottom.circle.right"), PageNumberText::Replace));
    const QByteArray docx = doc->toDocx();
    QVERIFY(partOf(docx, QStringLiteral("/word/header1.xml")).isEmpty());   // (a bottom design is in the footer)
    const QByteArray xml = partOf(docx, QStringLiteral("/word/footer1.xml"));
    QCOMPARE(xml.count("<wp:anchor"), 2);
    QVERIFY(xml.contains("prst=\"ellipse\""));
    QVERIFY(xml.contains("w:instrText") && xml.contains("PAGE"));
    QVERIFY(xml.contains("schemeClr") || xml.contains("srgbClr"));
    auto bar = openPages(3);
    QVERIFY(insertPageNumberDesign(*bar, 0, QStringLiteral("top.bar.center"), PageNumberText::Replace));
    QCOMPARE(partOf(bar->toDocx(), QStringLiteral("/word/header1.xml")).count("<wp:anchor"), 1);
}

// Page X of Y: the count is the document's, on every page.
void WriteTests::pageXOfYShowsTheCountOfThePages()
{
    for (const int pages : {1, 3, 4}) {
        auto doc = openPages(pages);
        QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("bottom.ofCount.center"), PageNumberText::Replace));
        for (int i = 0; i < pages; ++i) QCOMPARE(shownOnPage(*doc, i, QStringLiteral("flow-footer-flow")), QStringLiteral("Page %1 of %2").arg(i + 1).arg(pages));
    }
}

// A section's number format is used: roman numerals, from 5.
void WriteTests::theSectionsNumberFormatIsUsed()
{
    auto doc = openPages(3);
    QCOMPARE(doc->changeSections(0, 0, [](office::wml::SectionProps &p) {
        p.pageFormat = QStringLiteral("lowerRoman");
        p.pageStart = 5;
        return true;
    }), 1);
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("bottom.plain.center"), PageNumberText::Replace));
    QCOMPARE(shownOnPage(*doc, 0, QStringLiteral("flow-footer-flow")), QStringLiteral("v"));
    QCOMPARE(shownOnPage(*doc, 2, QStringLiteral("flow-footer-flow")), QStringLiteral("vii"));
}

// With a different first page, a design put in from page 1 goes to the first page's footer, and from page 2 to the other pages'. With different odd and even pages, from an even
// page it goes to the even pages' header, from an odd page to the odd pages'.
void WriteTests::aDesignGoesToTheKindOfPageTheCursorIsOn()
{
    auto doc = openPages(4);
    doc->setDifferentFirstPage(0, true);
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("bottom.plain.left"), PageNumberText::Replace));
    QVERIFY(doc->hasOwnHeaderFooter(0, true, WriteDoc::kFirstSlot));
    QCOMPARE(shownOnPage(*doc, 0, QStringLiteral("flow-footer-flow")), QStringLiteral("1"));
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QString());
    QVERIFY(insertPageNumberDesign(*doc, 1, QStringLiteral("bottom.plain.right"), PageNumberText::Replace));
    QVERIFY(doc->hasOwnHeaderFooter(0, true, WriteDoc::kDefaultSlot));
    for (int i = 0; i < 4; ++i) QCOMPARE(shownOnPage(*doc, i, QStringLiteral("flow-footer-flow")), QString::number(i + 1));
    QVERIFY(doc->headerFooterStory(0, true, WriteDoc::kFirstSlot)->firstBlock().blockFormat().alignment() & Qt::AlignLeft);
    QVERIFY(doc->headerFooterStory(0, true, WriteDoc::kDefaultSlot)->firstBlock().blockFormat().alignment() & Qt::AlignRight);

    auto oddEven = openPages(4);
    oddEven->setDifferentOddEven(true);
    // Page 2 (an even page) first: only the even pages have a number.
    QVERIFY(insertPageNumberDesign(*oddEven, 1, QStringLiteral("top.plain.left"), PageNumberText::Replace));
    QVERIFY(oddEven->hasOwnHeaderFooter(0, false, WriteDoc::kEvenSlot));
    for (int i = 0; i < 4; ++i) QCOMPARE(shownOnPage(*oddEven, i, QStringLiteral("flow-header-flow")), i % 2 == 1 ? QString::number(i + 1) : QString());
    // Then page 3 (an odd page): the odd pages have one at the right.
    QVERIFY(insertPageNumberDesign(*oddEven, 2, QStringLiteral("top.plain.right"), PageNumberText::Replace));
    for (int i = 0; i < 4; ++i) QCOMPARE(shownOnPage(*oddEven, i, QStringLiteral("flow-header-flow")), QString::number(i + 1));
    QVERIFY(oddEven->headerFooterStory(0, false, WriteDoc::kEvenSlot)->firstBlock().blockFormat().alignment() & Qt::AlignLeft);
    QVERIFY(oddEven->headerFooterStory(0, false, WriteDoc::kDefaultSlot)->firstBlock().blockFormat().alignment() & Qt::AlignRight);
    // Saved and read back: the same.
    QString err;
    auto back = WriteDoc::fromDocx(oddEven->toDocx(), &err);
    QVERIFY2(back, qPrintable(err));
    for (int i = 0; i < 4; ++i) QCOMPARE(shownOnPage(*back, i, QStringLiteral("flow-header-flow")), QString::number(i + 1));
}

// A header that already holds text: Replace puts the number in its place, Keep leaves the text and adds the number. What holds text is asked about only for text that is not page numbers.
void WriteTests::replaceAndKeepDecideWhatBecomesOfTheText()
{
    auto doc = openPages(3);
    PageNumberPlace place;
    const std::optional<PageNumberDesign> top = pageNumberDesignById(QStringLiteral("top.plain.center"));
    QVERIFY(top && pageNumberPlaceOf(*doc, 0, *top, &place));
    QVERIFY(!pageNumberTargetHoldsText(*doc, place));
    QVERIFY(doc->createHeaderFooter(0, false, 0));
    typeAtEnd(*doc, doc->editableHeaderFooter(0, false, 0), QStringLiteral("Annual Report"));
    QVERIFY(pageNumberTargetHoldsText(*doc, place));
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("top.plain.center"), PageNumberText::Keep));
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-header-flow")), QStringLiteral("Annual Report 2"));
    QVERIFY(pageNumberTargetHoldsText(*doc, place));
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("top.pageLine.left"), PageNumberText::Replace));
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-header-flow")), QStringLiteral("Page 2"));
    QVERIFY(!pageNumberTargetHoldsText(*doc, place));
    // A number put in over a number replaces it, without the question.
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("top.ofCount.left"), PageNumberText::Keep));
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("top.circle.right"), PageNumberText::Replace));
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-header-flow")), QStringLiteral("2"));
}

// Remove Page Numbers takes the PAGE and NUMPAGES fields and the boxes of the designs out of every header and footer, and leaves the other text where it is.
void WriteTests::removePageNumbersLeavesTheOtherTextAlone()
{
    auto doc = openPages(3);
    QVERIFY(doc->createHeaderFooter(0, false, 0));
    typeAtEnd(*doc, doc->editableHeaderFooter(0, false, 0), QStringLiteral("Annual Report"));
    QVERIFY(doc->createHeaderFooter(0, true, 0));
    typeAtEnd(*doc, doc->editableHeaderFooter(0, true, 0), QStringLiteral("Draft"));
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("top.ofCount.right"), PageNumberText::Keep));
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("bottom.circle.right"), PageNumberText::Keep));
    QVERIFY(insertPageNumberDesign(*doc, 0, QStringLiteral("margin.left.bar"), PageNumberText::Keep));
    QVERIFY(shownOnPage(*doc, 0, QStringLiteral("flow-header-flow")).contains(QLatin1String("Page 1 of 3")));
    const int removed = removePageNumbers(*doc);
    QCOMPARE(removed, 5);   // (PAGE and NUMPAGES, the circle and the box over it, and the bar)
    for (int i = 0; i < 3; ++i) {
        QCOMPARE(shownOnPage(*doc, i, QStringLiteral("flow-header-flow")), QStringLiteral("Annual Report"));
        QCOMPARE(shownOnPage(*doc, i, QStringLiteral("flow-footer-flow")), QStringLiteral("Draft"));
    }
    const QByteArray docx = doc->toDocx();
    for (const QString &part : {QStringLiteral("/word/header1.xml"), QStringLiteral("/word/footer1.xml")}) {
        const QByteArray xml = partOf(docx, part);
        QVERIFY2(!xml.contains("PAGE") && !xml.contains("<wp:anchor") && !xml.contains("Page Number"), qPrintable(part));
    }
    QVERIFY(partOf(docx, QStringLiteral("/word/header1.xml")).contains("Annual Report"));
    QVERIFY(partOf(docx, QStringLiteral("/word/footer1.xml")).contains("Draft"));
    // One step undoes it.
    QVERIFY(doc->undo());
    QVERIFY(shownOnPage(*doc, 0, QStringLiteral("flow-header-flow")).contains(QLatin1String("Page 1 of 3")));
    // Nothing to remove: nothing is done.
    auto none = openPages(2);
    QCOMPARE(removePageNumbers(*none), 0);
    QVERIFY(!none->canUndo());
}

// A design is one step of Undo (a header made for it too), and Redo puts it back.
void WriteTests::aDesignIsOneStepOfUndo()
{
    for (const char *id : {"bottom.plain.center", "top.ofCount.right", "top.ruleBelow.center", "bottom.circle.right", "margin.right.tab"}) {
        auto doc = openPages(3);
        QVERIFY(!doc->canUndo());
        QVERIFY2(insertPageNumberDesign(*doc, 0, QString::fromLatin1(id), PageNumberText::Replace), id);
        QVERIFY(doc->canUndo());
        auto both = [&] { return shownOnPage(*doc, 1, QStringLiteral("flow-header-flow")) + QStringLiteral("/") + shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")); };
        const QString after = both();
        QVERIFY2(after != QStringLiteral("/"), id);
        QVERIFY2(doc->undo(), id);
        QVERIFY2(!doc->canUndo(), id);
        QCOMPARE(both(), QStringLiteral("/"));
        QVERIFY(doc->redo());
        QCOMPARE(both(), after);
    }
}

// The designs of the current position put the number at the caret: in the text and in a footer; the circled one is a drawing in line.
void WriteTests::theDesignsOfTheCurrentPositionPutTheNumberAtTheCaret()
{
    auto doc = openPages(3);
    QTextCursor c(doc->story());
    c.setPosition(5);
    QVERIFY(!insertPageNumberAtCursor(*doc, c, QStringLiteral("top.plain.center")));
    QVERIFY(insertPageNumberAtCursor(*doc, c, QStringLiteral("cursor.ofCount")));
    QVERIFY(doc->undo());
    QVERIFY(!doc->canUndo());
    QVERIFY(insertPageNumberAtCursor(*doc, c, QStringLiteral("cursor.plain")));
    QVERIFY(insertPageNumberAtCursor(*doc, c, QStringLiteral("cursor.bold")));
    QVERIFY(insertPageNumberAtCursor(*doc, c, QStringLiteral("cursor.circle")));
    const QByteArray xml = partOf(doc->toDocx(), QStringLiteral("/word/document.xml"));
    QVERIFY(xml.count("PAGE") >= 3);
    QVERIFY(xml.contains("<wp:inline"));
    QVERIFY(xml.contains("<w:b/>"));
    // In a footer: the same.
    QVERIFY(doc->createHeaderFooter(0, true, 0));
    QTextDocument *footer = doc->editableHeaderFooter(0, true, 0);
    typeAtEnd(*doc, footer, QStringLiteral("Draft "));
    QTextCursor f(footer);
    f.movePosition(QTextCursor::End);
    QVERIFY(insertPageNumberAtCursor(*doc, f, QStringLiteral("cursor.ofCount")));
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QStringLiteral("Draft Page 2 of 3"));
    QString err;
    auto back = WriteDoc::fromDocx(doc->toDocx(), &err);
    QVERIFY2(back, qPrintable(err));
    QCOMPARE(shownOnPage(*back, 2, QStringLiteral("flow-footer-flow")), QStringLiteral("Draft Page 3 of 3"));
}

// The window offers the gallery on Insert and on the Header & Footer tab, with Format Page Numbers and Remove Page Numbers under the designs.
void WriteTests::theWindowOffersTheGalleryOnInsertAndOnTheHeaderAndFooterTab()
{
    WriteWindow w;
    w.resize(1100, 800);
    w.show();
    w.setDocument(openPages(3), QString());
    QCOMPARE(w.pageNumberItems().size(), pageNumberDesigns().size());
    w.ribbon()->buildAllTabs();
    QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
    QVERIFY(w.findChildren<GalleryButton *>(QStringLiteral("pageNumberButton")).size() >= 2);
    QVERIFY(w.act("pn.format"));
    QVERIFY(w.act("pn.remove"));
    QCOMPARE(w.act("pn.format")->text(), QStringLiteral("Format Page Numbers…"));
    QCOMPARE(w.act("pn.remove")->text(), QStringLiteral("Remove Page Numbers"));
}

// A design chosen in the window goes to the page the caret is on, asks about a header that holds text, and is one step of Undo; Remove Page Numbers is one too.
void WriteTests::aDesignChosenInTheWindowIsAskedAboutAndIsOneStep()
{
    WriteWindow w;
    w.resize(1100, 800);
    w.show();
    w.setDocument(openPages(3), QString());
    WriteDoc *doc = w.document();
    w.view()->paginateAll();
    QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.ofCount.center")));
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QStringLiteral("Page 2 of 3"));
    w.view()->undo();
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QString());
    QVERIFY(!doc->canUndo());
    // A footer with text: asked; cancel leaves it; keep adds; replace replaces.
    QVERIFY(doc->createHeaderFooter(0, true, 0));
    typeAtEnd(*doc, doc->editableHeaderFooter(0, true, 0), QStringLiteral("Draft"));
    int asked = 0, answer = 0;
    w.setPageNumberAsker([&] {
        ++asked;
        return answer;
    });
    QVERIFY(!w.insertPageNumberDesign(QStringLiteral("bottom.plain.right")));
    QCOMPARE(asked, 1);
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QStringLiteral("Draft"));
    answer = 2;
    QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.plain.right")));
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QStringLiteral("Draft 2"));
    answer = 1;
    QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.plain.left")));
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QStringLiteral("2"));
    QCOMPARE(asked, 3);
    // A margin design is added without the question; the current position at the caret, in the text.
    QVERIFY(w.insertPageNumberDesign(QStringLiteral("margin.right.circle")));
    QCOMPARE(asked, 3);
    QVERIFY(w.insertPageNumberDesign(QStringLiteral("cursor.plain")));
    QVERIFY(!w.insertPageNumberDesign(QStringLiteral("nonsense")));
    QVERIFY(w.removePageNumbers() >= 3);
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QString());
    w.view()->undo();
    QCOMPARE(shownOnPage(*doc, 1, QStringLiteral("flow-footer-flow")), QStringLiteral("2 | 2"));
}

// The designs are in the PDF and in the print-out: each page has its own number (and the count), the circle's number too, in the words of the page.
void WriteTests::theDesignsAreInThePdfAndThePrintout()
{
    QTemporaryDir dir;
    WriteWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.setDocument(openPages(3), QString());
    QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.ofCount.center")));
    QVERIFY(w.insertPageNumberDesign(QStringLiteral("top.circle.right")));
    QString target = dir.filePath(QStringLiteral("export.pdf"));
    w.setSavePathChooser([&](const QString &, const QString &) { return target; });
    auto pageWords = [](const QString &path) {
        QFile f(path);
        QStringList out;
        if (!f.open(QIODevice::ReadOnly)) return out;
        const jp::PdfDocument pdf(f.readAll());
        for (int i = 0; pdf.isValid() && i < pdf.pageCount(); ++i) out << pdf.text(i);
        return out;
    };
    auto check = [&](const QString &path, const QString &how) {
        const QStringList pages = pageWords(path);
        QVERIFY2(pages.size() == 3, qPrintable(how + QStringLiteral(": %1 pages").arg(pages.size())));
        for (int i = 0; i < 3; ++i) {
            QVERIFY2(pages[i].contains(QStringLiteral("Page %1 of 3").arg(i + 1)), qPrintable(how + QStringLiteral(" page %1: ").arg(i + 1) + pages[i]));
            // (the circle's number, on its own: the number a second time)
            QVERIFY2(pages[i].count(QString::number(i + 1)) >= 3, qPrintable(how + QStringLiteral(" page %1: ").arg(i + 1) + pages[i]));
        }
    };
    QVERIFY(w.exportPdf());
    check(target, QStringLiteral("export"));
    target = dir.filePath(QStringLiteral("print.pdf"));
    WriteOutputOptions o;
    o.toFile = true;
    QVERIFY(w.printOutput(o));
    check(target, QStringLiteral("print"));
}
