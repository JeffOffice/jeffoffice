// JeffPub's tests of the Page Number gallery: the designs put on the master page, laid out with the right numbers on pages 1 to 3, saved and opened again, the page a design goes to (the
// master that page uses), Remove Page Numbers, Undo, and the gallery the window lists.
#include "pubhelpers.h"
#include "app/pagenumbers.h"
#include <QTemporaryDir>

namespace {

// An editor on a blank publication of `pages` pages.
struct Shown {
    jp::Editor ed;
    jp::Document *doc = nullptr;
    explicit Shown(int pages = 3)
    {
        ed.setDocument(jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), pages));
        doc = ed.doc();
    }
};

// What the master page of page `index` (from 0) shows as page numbers: the text of each of its boxes (a text box or a shape with words) as laid out with that page's number and the page count.
QString shownOnMaster(const jp::Document &doc, int index)
{
    using namespace jp;
    const MasterPage *m = doc.masterFor(*doc.pages[index]);
    QStringList out;
    if (!m) return QString();
    for (const ItemPtr &it : m->items) {
        QString storyId;
        if (it->type() == ItemType::Text) storyId = static_cast<TextItem *>(it.get())->storyId;
        else if (it->type() == ItemType::Shape) storyId = static_cast<ShapeItem *>(it.get())->storyId;
        QTextDocument *story = storyId.isEmpty() ? nullptr : doc.storyDoc(storyId);
        if (!story) continue;
        FrameSpec fs;
        fs.size = QSizeF(468, 200);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        fs.ctx.doc = &doc;
        fs.ctx.pageNumber = index + 1;
        fs.ctx.pageCount = int(doc.pages.size());
        StoryLayout layout;
        layout.build(story, {fs}, LayoutEnv());
        QStringList lines;
        for (const auto &l : layout.lineInfo(0)) lines << l.text.trimmed();
        const QString t = lines.join(QLatin1Char(' ')).trimmed();
        if (!t.isEmpty()) out << t;
    }
    return out.join(QStringLiteral(" | "));
}

} // namespace

class PubPageNumberTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // The window lists the gallery's designs (all but the circled one at the caret), in the order of the groups; and its button is on the Insert tab and the Master Page tab.
    void theGalleryListsTheDesignsAndHasItsButtons()
    {
        jp::MainWindow w;
        const QVector<jp::GalleryItem> items = w.pageNumberItems();
        int expected = 0;
        for (const jp::PageNumberDesign &d : jp::pageNumberDesigns()) expected += jp::pubOffersPageNumberDesign(d);
        QCOMPARE(int(items.size()), expected);
        QStringList groups;
        for (const jp::GalleryItem &it : items)
            if (!groups.contains(it.group)) groups << it.group;
        QCOMPARE(groups, (QStringList{jp::pageNumberAreaName(jp::PageNumberArea::Top), jp::pageNumberAreaName(jp::PageNumberArea::Bottom),
                                      jp::pageNumberAreaName(jp::PageNumberArea::Margin), jp::pageNumberAreaName(jp::PageNumberArea::Cursor)}));
        QVERIFY(items.size() >= 12 * 2 + 6 + 3);
        QVERIFY2(w.ribbonError().isEmpty(), qPrintable(w.ribbonError()));
        w.ribbon()->buildAllTabs();
        QVERIFY(w.findChildren<jp::GalleryButton *>(QStringLiteral("pageNumberButton")).size() >= 2);
        QVERIFY(w.act(QStringLiteral("ins.pageNumberRemove")));
        QVERIFY(w.act(QStringLiteral("ins.pageNumberFormat")));
    }

    // Each design of the top, the bottom, and the margins goes on the master page and shows the page's own number on pages 1 to 3, the same after saving and opening the file again.
    void eachDesignIsPutOnTheMasterAndShowsTheRightNumbers()
    {
        using namespace jp;
        int tried = 0;
        QTemporaryDir dir;
        for (const PageNumberDesign &d : pageNumberDesigns()) {
            if (d.area == PageNumberArea::Cursor) continue;
            ++tried;
            Shown s;
            QVERIFY2(placePageNumberDesign(&s.ed, d.id), qPrintable(d.id));
            const QString want = d.look == PageNumberLook::OfCount ? QStringLiteral("Page %1 of 3") : d.look == PageNumberLook::PageLine ? QStringLiteral("Page %1") : QStringLiteral("%1");
            auto check = [&](const Document &made, const QString &how) {
                for (int i = 0; i < 3; ++i)
                    QVERIFY2(shownOnMaster(made, i) == want.arg(i + 1), qPrintable(d.id + QStringLiteral(" (") + how + QStringLiteral(") page %1 shows [%2]").arg(i + 1).arg(shownOnMaster(made, i))));
            };
            check(*s.doc, QStringLiteral("made"));
            QString err;
            const QString path = dir.filePath(QStringLiteral("n.jpub"));
            QVERIFY2(savePublication(*s.doc, path, QImage(), &err), qPrintable(err));
            auto back = loadPublication(path, &err);
            QVERIFY2(back, qPrintable(err));
            check(*back, QStringLiteral("reopened"));
            // (The shape of a box design is the shape the facts say, in the accent; a rule is a box of its own.)
            const MasterPage *m = back->masters.first().get();
            QCOMPARE(int(m->items.size()), pageNumberRule(d) == PageNumberRule::None ? 1 : 2);
            if (pageNumberHasBox(d)) {
                QCOMPARE(int(m->items.front()->type()), int(ItemType::Shape));
                QCOMPARE(static_cast<ShapeItem *>(m->items.front().get())->shape, pageNumberBox(d, back->pageSize(), back->setup.margins).preset);
                QVERIFY(!m->items.front()->fill.isNone());
            }
        }
        QVERIFY(tried >= 38);
    }

    // A design goes to the master the page being shown uses: a first page with a master of its own (no number) and odd and even pages with two masters each get the number on their master only.
    void aDesignGoesToTheMasterOfThePageBeingShown()
    {
        using namespace jp;
        Shown s(4);
        Document *doc = s.doc;
        auto second = std::make_shared<MasterPage>();
        second->id = QStringLiteral("B");
        second->abbr = QStringLiteral("B");
        second->name = QStringLiteral("Even Pages");
        doc->masters << second;
        auto first = std::make_shared<MasterPage>();
        first->id = QStringLiteral("T");
        first->abbr = QStringLiteral("T");
        first->name = QStringLiteral("Title Page");
        doc->masters << first;
        doc->pages[0]->masterId = QStringLiteral("T");
        doc->pages[1]->masterId = QStringLiteral("B");
        doc->pages[3]->masterId = QStringLiteral("B");
        // From page 2 (even): the even master.
        s.ed.setCurrentPage(1);
        QVERIFY(placePageNumberDesign(&s.ed, QStringLiteral("top.plain.left")));
        QCOMPARE(shownOnMaster(*doc, 0), QString());
        QCOMPARE(shownOnMaster(*doc, 1), QStringLiteral("2"));
        QCOMPARE(shownOnMaster(*doc, 2), QString());
        QCOMPARE(shownOnMaster(*doc, 3), QStringLiteral("4"));
        // From page 3 (odd): the odd master, at the right; the even one keeps its own.
        s.ed.setCurrentPage(2);
        QVERIFY(placePageNumberDesign(&s.ed, QStringLiteral("top.plain.right")));
        QCOMPARE(shownOnMaster(*doc, 2), QStringLiteral("3"));
        QCOMPARE(shownOnMaster(*doc, 0), QString());   // (the title page stays without)
        QCOMPARE(shownOnMaster(*doc, 3), QStringLiteral("4"));
        QCOMPARE(int(doc->master(QStringLiteral("B"))->items.size()), 1);
        QCOMPARE(int(doc->masters.first()->items.size()), 1);
    }

    // A design put on a master that has one replaces it, and the other items of the master stay; Remove Page Numbers takes the numbers and the boxes of the designs and leaves the rest.
    void aNewDesignReplacesTheOldAndRemoveLeavesTheOtherItems()
    {
        using namespace jp;
        Shown s;
        Document *doc = s.doc;
        auto note = std::static_pointer_cast<TextItem>(s.ed.newTextBox(QRectF(72, 20, 200, 20), QStringLiteral("Draft")));
        doc->masters[0]->items.push_back(note);
        QVERIFY(placePageNumberDesign(&s.ed, QStringLiteral("bottom.ruleAbove.center")));
        QCOMPARE(int(doc->masters[0]->items.size()), 3);   // the note, the number's box, and its rule
        QVERIFY(placePageNumberDesign(&s.ed, QStringLiteral("bottom.circle.right")));
        QCOMPARE(int(doc->masters[0]->items.size()), 2);
        QCOMPARE(shownOnMaster(*doc, 1), QStringLiteral("Draft | 2"));
        // A field in the note is the person's own number: Remove leaves it.
        QTextCursor c(doc->storyDoc(note->storyId));
        c.movePosition(QTextCursor::End);
        QTextCharFormat f;
        f.setProperty(tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), f);
        QCOMPARE(shownOnMaster(*doc, 2), QStringLiteral("Draft3 | 3"));
        QCOMPARE(removePageNumbers(&s.ed), 1);   // (the circle's box: the gallery's only)
        for (int i = 0; i < 3; ++i) QCOMPARE(shownOnMaster(*doc, i), QStringLiteral("Draft%1").arg(i + 1));
        QCOMPARE(int(doc->masters[0]->items.size()), 1);
        // Undone, in one step.
        s.ed.undo();
        QCOMPARE(shownOnMaster(*doc, 2), QStringLiteral("Draft3 | 3"));
        // Nothing to remove: nothing is done.
        Shown none;
        QCOMPARE(removePageNumbers(&none.ed), 0);
    }

    // A design is one step of Undo, and Redo puts it back.
    void aDesignIsOneStepOfUndo()
    {
        using namespace jp;
        for (const char *id : {"top.plain.center", "bottom.ofCount.right", "margin.left.circle", "margin.right.bar"}) {
            Shown s;
            QVERIFY(!s.ed.undoStack()->canUndo());
            QVERIFY2(placePageNumberDesign(&s.ed, QString::fromLatin1(id)), id);
            QVERIFY(s.ed.undoStack()->canUndo());
            const QString after = shownOnMaster(*s.doc, 1);
            QVERIFY2(!after.isEmpty(), id);
            s.ed.undo();
            QCOMPARE(shownOnMaster(*s.doc, 1), QString());
            QVERIFY(!s.ed.undoStack()->canUndo());
            s.ed.redo();
            QCOMPARE(shownOnMaster(*s.doc, 1), after);
        }
    }

    // The designs of the current position go in the text being typed (and nothing is done when none is); the window's command chooses between them and the master.
    void theCurrentPositionDesignsGoInTheText()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("cursor.plain")));   // (no text being typed)
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("cursor.circle")));   // (not offered here)
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("nonsense")));
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 300, 40)));
        ed->addItem(box);
        ed->beginTextEdit(box->id, 0);
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("cursor.ofCount")));
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("cursor.bold")));
        ed->endTextEdit();
        QTextDocument *story = ed->doc()->storyDoc(box->storyId);
        int fields = 0;
        for (int i = 0; i < story->characterCount() - 1; ++i) fields += story->characterAt(i) == QChar::ObjectReplacementCharacter;
        QCOMPARE(fields, 3);
        QVERIFY(story->toPlainText().contains(QStringLiteral("Page ")));
        // On the master: the window's command.
        const int before = int(ed->doc()->masters[0]->items.size());
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.plain.center")));
        QCOMPARE(int(ed->doc()->masters[0]->items.size()), before + 1);
    }

    // A master that already has a page number in the person's own text: a design asks (cancel changes nothing, Keep adds the design beside the number, Replace takes the number out of its
    // text); and Remove Page Numbers takes only the design's boxes, never a page number of the person's own.
    void aPageNumberOfThePersonsOwnIsAskedAboutAndNeverRemoved()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        jp::Document *doc = ed->doc();
        auto note = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 20, 200, 20), QStringLiteral("Draft")));
        doc->masters[0]->items.push_back(note);
        QTextCursor c(doc->storyDoc(note->storyId));
        c.movePosition(QTextCursor::End);
        QTextCharFormat f;
        f.setProperty(jp::tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), f);
        QVERIFY(jp::pubMasterHasOwnPageNumber(ed));
        int asked = 0, answer = 0;
        w.setPageNumberAsker([&] {
            ++asked;
            return answer;
        });
        QVERIFY(!w.insertPageNumberDesign(QStringLiteral("bottom.plain.center")));
        QCOMPARE(asked, 1);
        QCOMPARE(int(doc->masters[0]->items.size()), 1);
        answer = 2;
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("bottom.plain.center")));
        QCOMPARE(int(doc->masters[0]->items.size()), 2);
        QCOMPARE(shownOnMaster(*doc, 0), QStringLiteral("Draft1 | 1"));
        QCOMPARE(jp::removePageNumbers(ed), 1);
        QCOMPARE(shownOnMaster(*doc, 0), QStringLiteral("Draft1"));   // (the person's own stays)
        QVERIFY(jp::pubMasterHasOwnPageNumber(ed));
        answer = 1;
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("top.plain.right")));
        QCOMPARE(asked, 3);
        QCOMPARE(shownOnMaster(*doc, 0), QStringLiteral("Draft | 1"));   // (taken out of its text)
        QVERIFY(!jp::pubMasterHasOwnPageNumber(ed));
        // A margin design asks too, and a master with none is not asked.
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("margin.left.circle")));
        QCOMPARE(asked, 3);
    }

    // Remove Page Numbers is on only while a master has a box of a design, and follows Undo and Redo.
    void theRemoveCommandFollowsTheMasters()
    {
        jp::MainWindow w;
        w.refreshForTest();
        QVERIFY(!w.act(QStringLiteral("ins.pageNumberRemove"))->isEnabled());
        QVERIFY(w.insertPageNumberDesign(QStringLiteral("top.plain.center")));
        w.refreshForTest();
        QVERIFY(w.act(QStringLiteral("ins.pageNumberRemove"))->isEnabled());
        w.editor()->undo();
        w.refreshForTest();
        QVERIFY(!w.act(QStringLiteral("ins.pageNumberRemove"))->isEnabled());
        w.editor()->redo();
        w.refreshForTest();
        QVERIFY(w.act(QStringLiteral("ins.pageNumberRemove"))->isEnabled());
        w.act(QStringLiteral("ins.pageNumberRemove"))->trigger();
        w.refreshForTest();
        QVERIFY(!w.act(QStringLiteral("ins.pageNumberRemove"))->isEnabled());
    }
};

JP_TEST_CLASS(PubPageNumberTests)
#include "tst_pub_pagenum.moc"
