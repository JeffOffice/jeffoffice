// JeffPub's tests of rules the program relies on: a paragraph's properties fit a formatting page, a publication has a page, a .docx is
// read within limits, linked boxes that wrap are laid out in bounded work, and a few file and dialog limits.
#include "pubhelpers.h"
#include <zlib.h>
#include <QTableWidget>

namespace {

QString writePub(jp::Editor *ed, const QString &dir, const QString &name)
{
    const QString path = QDir(dir).filePath(name);
    QString err;
    if (!jp::exportPublisher(*ed->doc(), path, &err)) return QString();
    return path;
}

// A text box whose one paragraph has `tabs` tab stops (right-aligned, with leaders, to fill the format's room).
std::shared_ptr<jp::TextItem> boxWithTabs(jp::Editor *ed, int tabs)
{
    auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200), QStringLiteral("one\ttwo")));
    ed->addItem(box);
    QTextBlockFormat bf;
    QList<QTextOption::Tab> list;
    QString leaders;
    for (int i = 0; i < tabs; ++i) {
        list << QTextOption::Tab(10.0 + i * 5, i % 2 ? QTextOption::RightTab : QTextOption::LeftTab);
        leaders += QLatin1Char('.');
    }
    bf.setTabPositions(list);
    bf.setProperty(jp::tp::TabLeaders, leaders);
    QTextCursor c(ed->doc()->storyDoc(box->storyId));
    c.mergeBlockFormat(bf);
    return box;
}

int mostTabs(const jp::Document &d)
{
    int most = 0;
    for (const auto &s : d.stories)
        for (QTextBlock b = d.storyDoc(s->id)->begin(); b.isValid(); b = b.next()) most = std::max(most, int(b.blockFormat().tabPositions().size()));
    return most;
}

// A zip with the given entries (name, bytes) that all point at one stored or deflated stream: the first entry's data.
QByteArray zipSharingOneStream(const QStringList &names, const QByteArray &payload)
{
    z_stream zs{};
    deflateInit2(&zs, 9, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    QByteArray comp(int(deflateBound(&zs, uLong(payload.size()))), '\0');
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(payload.constData()));
    zs.avail_in = uInt(payload.size());
    zs.next_out = reinterpret_cast<Bytef *>(comp.data());
    zs.avail_out = uInt(comp.size());
    deflate(&zs, Z_FINISH);
    comp.resize(int(zs.total_out));
    deflateEnd(&zs);
    auto le16 = [](QByteArray &b, quint32 v) { b.append(char(v & 255)); b.append(char((v >> 8) & 255)); };
    auto le32 = [&](QByteArray &b, quint32 v) { le16(b, v & 0xffff); le16(b, v >> 16); };
    const QByteArray first = names.first().toUtf8();
    QByteArray out;
    le32(out, 0x04034b50); le16(out, 20); le16(out, 0); le16(out, 8); le16(out, 0); le16(out, 0);
    le32(out, 0); le32(out, quint32(comp.size())); le32(out, quint32(payload.size())); le16(out, quint32(first.size())); le16(out, 0);
    out.append(first);
    out.append(comp);
    QByteArray cd;
    for (const QString &n : names) {
        const QByteArray nm = n.toUtf8();
        le32(cd, 0x02014b50); le16(cd, 20); le16(cd, 20); le16(cd, 0); le16(cd, 8); le16(cd, 0); le16(cd, 0);
        le32(cd, 0); le32(cd, quint32(comp.size())); le32(cd, quint32(payload.size()));
        le16(cd, quint32(nm.size())); le16(cd, 0); le16(cd, 0); le16(cd, 0); le16(cd, 0); le32(cd, 0); le32(cd, 0);
        cd.append(nm);
    }
    const quint32 at = quint32(out.size());
    out.append(cd);
    le32(out, 0x06054b50); le16(out, 0); le16(out, 0); le16(out, quint32(names.size())); le16(out, quint32(names.size()));
    le32(out, quint32(cd.size())); le32(out, at); le16(out, 0);
    return out;
}

} // namespace

class PubInvariantTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // A paragraph with more tab stops than a 512-byte formatting page holds made the save crash (the page's last run end was read from an empty
    // list). The stops are capped at what the format holds, and a block that still doesn't fit is left out, not written.
    void manyTabStopsSaveAndReopen_data()
    {
        QTest::addColumn<int>("tabs");
        QTest::newRow("24") << 24;
        QTest::newRow("40") << 40;
        QTest::newRow("80") << 80;
    }
    void manyTabStopsSaveAndReopen()
    {
        QFETCH(int, tabs);
        jp::Editor ed;
        boxWithTabs(&ed, tabs);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writePub(&ed, dir.path(), QStringLiteral("tabs.pub"));
        QVERIFY(!path.isEmpty());
        QString err;
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(mostTabs(*back), std::min(tabs, jp::tp::kMaxTabStops));
    }

    // The Tabs dialog stops at the cap too: Add is dimmed when the list is full.
    void theTabsDialogStopsAtTheCap()
    {
        jp::Editor ed;
        boxWithTabs(&ed, 1);
        ed.beginTextEdit(ed.doc()->pages[0]->items[0]->id);
        int added = -1;
        bool dimmed = false;
        QTimer::singleShot(0, [&] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            auto *table = dlg->findChild<QTableWidget *>();
            QPushButton *add = nullptr;
            for (auto *b : dlg->findChildren<QPushButton *>())
                if (b->text() == QStringLiteral("Add")) add = b;
            if (table && add) {
                for (int i = 0; i < 100 && add->isEnabled(); ++i) add->click();
                added = table->rowCount();
                dimmed = !add->isEnabled();
            }
            dlg->reject();
        });
        jp::paragraphDialog(nullptr, &ed, 2);
        QCOMPARE(added, jp::tp::kMaxTabStops);
        QVERIFY(dimmed);
    }

    // A publication always has a page: a file with none gets a blank one, and the page commands never index an empty list.
    void aPublicationWithNoPagesGetsOne()
    {
        auto blank = jp::Document::blank(QSizeF(612, 792));
        QJsonObject o = blank->toJson();
        o["pages"] = QJsonArray();
        jp::Document d;
        d.fromJson(o);
        QCOMPARE(int(d.pages.size()), 1);
        QVERIFY(!d.masters.isEmpty());
    }
    void anEditorWithNoPagesDoesNotCrash()
    {
        jp::Editor ed;
        ed.doc()->pages.clear();
        ed.setCurrentPage(3);   // (no page to go to)
        QCOMPARE(ed.currentPage(), 0);
        auto fresh = jp::Document::blank(QSizeF(612, 792));
        fresh->pages.clear();
        ed.setDocument(std::move(fresh), QString());
        QCOMPARE(int(ed.doc()->pages.size()), 1);
        QVERIFY(ed.surface());
        // Saving a publication whose pages were all taken away writes a file.
        ed.doc()->pages.clear();
        QTemporaryDir dir;
        QVERIFY(!writePub(&ed, dir.path(), QStringLiteral("none.pub")).isEmpty());
    }

    // A file that lists no master pages keeps the colors, fonts, styles, and business information it holds.
    void noMastersKeepsTheLoadedSchemes()
    {
        auto blank = jp::Document::blank(QSizeF(612, 792));
        blank->colors.name = QStringLiteral("MINE");
        blank->fonts.name = QStringLiteral("MINEF");
        blank->biz[0].setName = QStringLiteral("MINEB");
        QJsonObject o = blank->toJson();
        o["masters"] = QJsonArray();
        jp::Document d;
        d.fromJson(o);
        QCOMPARE(int(d.masters.size()), 1);
        QCOMPARE(d.colors.name, QStringLiteral("MINE"));
        QCOMPARE(d.fonts.name, QStringLiteral("MINEF"));
        QCOMPARE(d.biz[0].setName, QStringLiteral("MINEB"));
    }

    // The business set a file calls current may not exist: the dialog indexed past the list.
    void aBusinessSetOutOfRangeIsTheLastOne()
    {
        auto blank = jp::Document::blank(QSizeF(612, 792));
        QJsonObject o = blank->toJson();
        o["businessCurrent"] = 5;
        jp::Document d;
        d.fromJson(o);
        QVERIFY(d.bizCurrent >= 0 && d.bizCurrent < d.biz.size());
        o["businessCurrent"] = -3;
        d.fromJson(o);
        QVERIFY(d.bizCurrent >= 0 && d.bizCurrent < d.biz.size());
    }

    // The undo list keeps whole-publication snapshots, so it holds a limited number of steps.
    void theEditorsUndoListIsLimited()
    {
        jp::Editor ed;
        for (int i = 0; i < 400; ++i) ed.change(QStringLiteral("Step"), [&] { ed.doc()->setup.margins = QMarginsF(i % 50, 0, 0, 0); });
        QVERIFY(ed.undoStack()->count() <= jp::Editor::kUndoSteps);
        QCOMPARE(ed.undoStack()->undoLimit(), jp::Editor::kUndoSteps);
    }

    // A .docx is read for its text only: one part is inflated, within a cap, and a part whose data other entries share is refused.
    void aDocxWhoseEntriesShareOneStreamIsRefused()
    {
        const QByteArray zeros(4 * 1024 * 1024, '\0');
        QStringList names{QStringLiteral("word/document.xml")};
        for (int i = 0; i < 40; ++i) names << QStringLiteral("e%1.bin").arg(i);
        const QByteArray bomb = zipSharingOneStream(names, zeros);
        QVERIFY(jp::docxToHtml(bomb).isEmpty());
    }
    void aDocxReadsItsDocumentPartAndNothingElse()
    {
        jp::ZipWriter z;
        z.add(QStringLiteral("word/document.xml"), QByteArray("<w:document xmlns:w=\"w\"><w:body><w:p><w:r><w:t>Hello</w:t></w:r></w:p></w:body></w:document>"), true);
        z.add(QStringLiteral("word/big.xml"), QByteArray(8 * 1024 * 1024, 'x'), true);
        const QString html = jp::docxToHtml(z.finish());
        QVERIFY(html.contains(QStringLiteral("Hello")));
    }

    // A length the file can't hold (32 bits) is kept within them, not wrapped to a small one.
    void aHugePositionDoesNotWrap()
    {
        jp::Editor ed;
        auto box = ed.newTextBox(QRectF(400000, 72, 100, 50), QStringLiteral("far"));
        ed.addItem(box);
        QTemporaryDir dir;
        const QString path = writePub(&ed, dir.path(), QStringLiteral("far.pub"));
        QVERIFY(!path.isEmpty());
        QString err;
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(!back->pages[0]->items.empty());
        const double x = back->pages[0]->items[0]->rect.x();
        QVERIFY2(x > 100000, qPrintable(QString::number(x)));
    }

    // A page's master is found by its id (a master's abbreviation is only its label).
    void aPageKeepsAMasterWhoseIdIsNotItsAbbreviation()
    {
        jp::Editor ed;
        auto m = std::make_shared<jp::MasterPage>();
        m->id = QStringLiteral("M9");
        m->abbr = QStringLiteral("B");
        m->name = QStringLiteral("Second");
        ed.doc()->masters << m;
        ed.doc()->pages[0]->masterId = QStringLiteral("M9");
        QTemporaryDir dir;
        const QString path = writePub(&ed, dir.path(), QStringLiteral("masters.pub"));
        QVERIFY(!path.isEmpty());
        QString err;
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->pages[0]->masterId, QStringLiteral("B"));
    }

    // A vertical tab in the text is a line break in the file, never a raw 0x0B (the publication program draws it as a box).
    void aVerticalTabIsWrittenAsALineBreak()
    {
        jp::Editor ed;
        auto box = std::static_pointer_cast<jp::TextItem>(ed.newTextBox(QRectF(72, 72, 300, 100)));
        ed.addItem(box);
        QTextCursor c(ed.doc()->storyDoc(box->storyId));
        c.insertText(QStringLiteral("one") + QChar(0x0b) + QStringLiteral("two"));
        QTemporaryDir dir;
        const QString path = writePub(&ed, dir.path(), QStringLiteral("vt.pub"));
        QVERIFY(!path.isEmpty());
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        const QByteArray raw("o\0n\0e\0\x0b\0t\0w\0o\0", 14);
        QVERIFY(!bytes.contains(raw));
    }

    // A chain of linked boxes that overlap and wrap around each other is laid out in work that grows by a bounded step per box: each box looks
    // at a limited number of the objects in front of it, and the page's chain is set up once, not once for each of its boxes.
    void aChainOfOverlappingWrapBoxesIsBoundedWork()
    {
        const int n = 2000;
        jp::Editor ed;
        ed.doc()->pages[0]->items.clear();
        QString previous;
        QVector<std::shared_ptr<jp::TextItem>> boxes;
        for (int i = 0; i < n; ++i) {
            auto b = std::static_pointer_cast<jp::TextItem>(ed.newTextBox(QRectF(10, 10, 50, 20), i ? QString() : QStringLiteral("text")));
            b->wrap.mode = jp::Wrap::Through;
            ed.doc()->pages[0]->items.push_back(b);
            boxes << b;
        }
        for (int i = 0; i + 1 < n; ++i) boxes[i]->nextId = boxes[i + 1]->id;
        for (int i = 1; i < n; ++i) boxes[i]->storyId = boxes[0]->storyId;
        const qint64 wrap0 = jp::Renderer::wrapWorkForTest(), walked0 = jp::Document::itemsWalkedForTest();
        jp::PaintContext ctx;
        ctx.doc = ed.doc();
        ctx.cache = &ed.cache();
        const QImage img = jp::Renderer::renderToImage(ctx, 0, 0.25);
        QVERIFY(!img.isNull());
        QVERIFY2(jp::Renderer::wrapWorkForTest() - wrap0 <= qint64(n) * 2048, qPrintable(QString::number(jp::Renderer::wrapWorkForTest() - wrap0)));
        QVERIFY2(jp::Document::itemsWalkedForTest() - walked0 <= qint64(8) * n * n, qPrintable(QString::number(jp::Document::itemsWalkedForTest() - walked0)));
    }

    // Who links to whom is found by one walk of the publication, kept until the publication changes: asking for a box's previous or its chain
    // costs no walk after that, however big the publication is, and a link made in a change is seen at once.
    void thePreviousBoxIsFoundWithoutWalkingThePublication_data()
    {
        QTest::addColumn<int>("n");
        QTest::newRow("200") << 200;
        QTest::newRow("2000") << 2000;
    }
    void thePreviousBoxIsFoundWithoutWalkingThePublication()
    {
        QFETCH(int, n);
        jp::Editor ed;
        ed.doc()->pages[0]->items.clear();
        QVector<std::shared_ptr<jp::TextItem>> boxes;
        for (int i = 0; i < n; ++i) {
            boxes << std::static_pointer_cast<jp::TextItem>(ed.newTextBox(QRectF(10 + i % 40, 10 + i / 40, 50, 20)));
            ed.doc()->pages[0]->items.push_back(boxes.last());
        }
        for (int i = 0; i + 1 < 100; ++i) boxes[i]->nextId = boxes[i + 1]->id;
        ed.doc()->dropChainIndex();
        QVERIFY(ed.doc()->prevFrame(boxes[50]->id) == boxes[49].get());   // (made)
        const qint64 walked0 = jp::Document::itemsWalkedForTest();
        for (int k = 0; k < 100; ++k) {
            QVERIFY(ed.doc()->prevFrame(boxes[1 + k % 98]->id));
            QCOMPARE(int(ed.doc()->chainOf(boxes[k % 100]->id).size()), 100);
        }
        QCOMPARE(jp::Document::itemsWalkedForTest() - walked0, qint64(0));
        // A link made in a change is seen.
        ed.change(QStringLiteral("Link"), [&] { boxes[150 % n]->nextId = boxes[151 % n]->id; });
        QVERIFY(ed.doc()->prevFrame(boxes[151 % n]->id) == boxes[150 % n].get());
    }

    // The editor's canvas sets up a chain once for the page it paints, not once for each of its boxes.
    void theCanvasPaintsAChainInWorkThatGrowsWithItsBoxes()
    {
        const int n = 300;
        jp::Editor ed;
        ed.doc()->pages[0]->items.clear();
        QVector<std::shared_ptr<jp::TextItem>> boxes;
        for (int i = 0; i < n; ++i) {
            auto b = std::static_pointer_cast<jp::TextItem>(ed.newTextBox(QRectF(10, 10, 50, 20), i ? QString() : QStringLiteral("text")));
            b->wrap.mode = jp::Wrap::Through;
            ed.doc()->pages[0]->items.push_back(b);
            boxes << b;
        }
        for (int i = 0; i + 1 < n; ++i) boxes[i]->nextId = boxes[i + 1]->id;
        for (int i = 1; i < n; ++i) boxes[i]->storyId = boxes[0]->storyId;
        ed.doc()->dropChainIndex();
        jp::Canvas canvas(&ed);
        canvas.resize(900, 1000);
        const qint64 walked0 = jp::Document::itemsWalkedForTest();
        const QPixmap shot = canvas.grab();
        QVERIFY(!shot.isNull());
        QVERIFY2(jp::Document::itemsWalkedForTest() - walked0 <= qint64(40) * n, qPrintable(QString::number(jp::Document::itemsWalkedForTest() - walked0)));
    }
};

JP_TEST_CLASS(PubInvariantTests)
#include "tst_pub_invariants.moc"
