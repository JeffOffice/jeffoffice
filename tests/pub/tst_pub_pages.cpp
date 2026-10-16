// JeffPub's tests of rulers, page sizes, and new pages.
#include "pubhelpers.h"

class PubPageTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // The Master Page tab as the other program has it: Show Header/Footer
    // goes to the master's header, then its footer; Insert Date and Insert
    // Time (Alt+Shift+D, Alt+Shift+T) add fields; the Mailings tab hides.
    // Select Recipients also offers contacts (vCard files). In master view the
    // Master Page tab comes first, right after File, and is the one showing;
    // the other contextual tabs stay where they were, and the KeyTips stay
    // unique.
    void masterPageTabAsTheOtherProgram()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        jp::Ribbon *r = w.ribbon();
        auto visible = [r](const QString &name) {
            for (int i = 0; i < r->tabCount(); ++i)
                if (r->tabName(i) == name) return r->tabVisible(i);
            return false;
        };
        auto shown = [r] {
            QStringList names;
            for (int i = 0; i < r->tabCount(); ++i)
                if (r->tabVisible(i)) names << r->tabName(i);
            return names;
        };
        const QStringList onThePage = shown();
        QCOMPARE(onThePage, (QStringList{"Home", "Insert", "Page Design", "Mailings", "Review", "View", "Help"}));
        QVERIFY(visible(QStringLiteral("Mailings")));
        w.act(QStringLiteral("mp.showHeaderFooter"))->trigger();
        QVERIFY(!ed->masterView().isEmpty());
        QVERIFY(visible(QStringLiteral("Master Page")));
        QVERIFY(!visible(QStringLiteral("Mailings")));
        QCOMPARE(shown(), (QStringList{"Master Page", "Home", "Insert", "Page Design", "Review", "View", "Help", "Text Box"}));
        QCOMPARE(r->current(), r->tab(QStringLiteral("Master Page")));
        // Screen readers go through the tab buttons in the order the parent holds them: the order shown.
        int last = -1;
        for (int i = 0; i < r->tabCount(); ++i) {
            if (!r->tabVisible(i)) continue;
            QWidget *b = r->tabButton(i);
            const int at = b->parentWidget()->children().indexOf(b);
            QVERIFY2(at > last, qPrintable(r->tabName(i)));
            last = at;
        }
        // Their KeyTips are still all different (Master Page is JM), and the first tab is the one reached by it.
        QStringList keys{r->fileKeytip()};
        for (int i = 0; i < r->tabCount(); ++i)
            if (r->tabVisible(i)) keys << r->tabKeytip(i);
        QCOMPARE(QSet<QString>(keys.begin(), keys.end()).size(), keys.size());
        QCOMPARE(keys.mid(0, 3), (QStringList{"F", "JM", "H"}));
        QVERIFY(ed->isEditingText());
        const QString header = ed->textTarget().itemId;
        QCOMPARE(ed->doc()->item(header)->name, QStringLiteral("Header"));
        w.act(QStringLiteral("ins.date"))->trigger();
        w.act(QStringLiteral("ins.time"))->trigger();
        QStringList fields;
        for (QTextBlock b = ed->editDoc()->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (const QString f = it.fragment().charFormat().stringProperty(jp::tp::Field); !f.isEmpty()) fields << f;
        QCOMPARE(fields, (QStringList{QStringLiteral("date"), QStringLiteral("time")}));
        w.act(QStringLiteral("mp.showHeaderFooter"))->trigger();
        QVERIFY(ed->isEditingText());
        QCOMPARE(ed->doc()->item(ed->textTarget().itemId)->name, QStringLiteral("Footer"));
        w.act(QStringLiteral("mp.showHeaderFooter"))->trigger();
        QCOMPARE(ed->textTarget().itemId, header);
        QCOMPARE(w.act(QStringLiteral("ins.date"))->shortcut(), QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_D));
        w.act(QStringLiteral("mp.close"))->trigger();
        QVERIFY(visible(QStringLiteral("Mailings")));
        QVERIFY(w.act(QStringLiteral("mm.contacts")));
        // Closed, the tab goes and the others are as they were, Home showing.
        QCOMPARE(shown(), onThePage);
        QCOMPARE(r->current(), r->tab(QStringLiteral("Home")));
        // From another tab, opening master view (Ctrl+M) shows the Master Page tab; a tab picked afterward stays, even as the selection brings other tabs.
        r->showTab(r->tab(QStringLiteral("View")));
        w.act(QStringLiteral("view.master"))->trigger();
        QCOMPARE(shown(), (QStringList{"Master Page", "Home", "Insert", "Page Design", "Review", "View", "Help"}));
        QCOMPARE(r->current(), r->tab(QStringLiteral("Master Page")));
        r->showTab(r->tab(QStringLiteral("Insert")));
        auto note = ed->newTextBox(QRectF(72, 72, 200, 100), QStringLiteral("x"));
        ed->addItem(note);
        ed->select(QStringList{note->id});
        QCOMPARE(shown().first(), QStringLiteral("Master Page"));
        QVERIFY(visible(QStringLiteral("Text Box")));
        QCOMPARE(r->current(), r->tab(QStringLiteral("Insert")));
        w.act(QStringLiteral("view.master"))->trigger();
        QCOMPARE(shown(), onThePage);
        // The contextual tabs that come with the selection keep their place, after Help.
        auto box = ed->newTextBox(QRectF(72, 72, 200, 100), QStringLiteral("y"));
        ed->addItem(box);
        ed->select(QStringList{box->id});
        QCOMPARE(shown(), onThePage + QStringList{"Text Box"});
    }

    // Page Setup's layout type is the DOCUMENT chunk's field 11 (1 booklet,
    // 3 folded card, 7 envelope; checked by choosing each in Publisher). A
    // booklet opens set to print two pages to a sheet, and its PDF holds
    // those sheets in folding order (8 and 1, 2 and 7...), as Publisher's does.
    void pubBookletLayout()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(396, 612));
        while (doc->pages.size() < 8) doc->addPage();
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("booklet.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        // Make the file a booklet: its DOCUMENT chunk's 41 field (u32 0)
        // becomes 11 = 1, the same length.
        QFile in(path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(in.readAll(), &file));
        in.close();
        QByteArray c = file.stream(QStringLiteral("Contents"));
        const qsizetype at = c.indexOf(QByteArray("\x41\x20\x00\x00\x00\x00\x44\x70", 8));
        QVERIFY(at > 0);
        c.replace(at, 6, QByteArray("\x11\x20\x01\x00\x00\x00", 6));
        QVERIFY(file.setStream(QStringLiteral("Contents"), c));
        QFile out(path);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(cfb::write(file));
        out.close();
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->setup.layout, PageSetup::Booklet);
        QCOMPARE(back->setup.sheet, QSizeF(792, 612));
        // Its PDF: four sheets of two pages, the first holding pages 8 and 1.
        for (int i = 0; i < 8; ++i) {
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(36, 36, 300, 60);
            t->storyId = back->createStory(QStringLiteral("Page %1").arg(i + 1));
            back->pages[i]->items.push_back(t);
        }
        MainWindow w;
        w.editor()->setDocument(std::move(back));
        const QString pdfPath = dir.filePath(QStringLiteral("booklet.pdf"));
        QVERIFY(w.exportPdfTo(pdfPath, MainWindow::PdfSettings()));
        QFile pf(pdfPath);
        QVERIFY(pf.open(QIODevice::ReadOnly));
        const QByteArray pdf = pf.readAll();
        QCOMPARE(pdf.count("/MediaBox [0 0 792.000000 612.000000]"), 4);
        // Reader pages instead when asked.
        MainWindow::PdfSettings single;
        single.booklet = false;
        QVERIFY(w.exportPdfTo(pdfPath, single));
        QFile pf2(pdfPath);
        QVERIFY(pf2.open(QIODevice::ReadOnly));
        QCOMPARE(pf2.readAll().count("/MediaBox [0 0 396.000000 612.000000]"), 8);
    }

    void pubBookletRoundTrip()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(396, 612));
        doc->setup.layout = PageSetup::Booklet;
        doc->setup.sheet = QSizeF(792, 612);
        for (int i = 1; i < 4; ++i) doc->addPage();
        auto mark = std::make_shared<ShapeItem>();
        mark->rect = QRectF(36, 36, 50, 50);
        mark->fill = Fill::solid(ColorRef::rgb(Qt::blue));
        doc->masters[0]->items.push_back(mark);
        for (int i = 0; i < 4; ++i) {
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(72, 300, 200, 40);
            t->storyId = doc->createStory(QStringLiteral("Page %1").arg(i + 1));
            doc->pages[i]->items.push_back(t);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("booklet.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->setup.layout), int(PageSetup::Booklet));
        QCOMPARE(int(back->pages.size()), 4);
        QCOMPARE(int(back->masters.size()), 1);
        QVERIFY(back->masters[0]->twoPage);
        for (const auto &pg : back->pages) QCOMPARE(pg->masterId, back->masters[0]->id);
        // The master's mark on both halves of the spread.
        QCOMPARE(int(back->masters[0]->items.size()), 2);
        QList<double> xs;
        for (const auto &it : back->masters[0]->items) xs << it->rect.left();
        std::sort(xs.begin(), xs.end());
        QVERIFY2(std::abs(xs[0] - 36) < 0.5 && std::abs(xs[1] - (396 + 36)) < 0.5, qPrintable(QStringLiteral("%1 %2").arg(xs[0]).arg(xs[1])));
        // Each page keeps its own text.
        for (int i = 0; i < 4; ++i) {
            auto *t = dynamic_cast<TextItem *>(back->pages[i]->items.front().get());
            QVERIFY(t);
            QCOMPARE(back->storyDoc(t->storyId)->toPlainText(), QStringLiteral("Page %1").arg(i + 1));
        }
    }

    // Measurement boxes keep three decimal places in any unit, without rounding.
    void measurementsThreeDecimals()
    {
        jp::Settings &st = jp::Settings::get();
        const jp::Unit was = st.unit();
        st.setUnit(jp::Unit::Inch);
        jp::MeasureSpin m;
        m.setRange(0, 72 * 240);
        double pt = 0;
        QVERIFY(st.parse(QStringLiteral("8.125"), &pt));
        m.setValue(pt);
        QCOMPARE(m.text(), QStringLiteral("8.125\""));
        QCOMPARE(m.value(), 585.0);
        QVERIFY(st.parse(QStringLiteral("11"), &pt));
        m.setValue(pt);
        QCOMPARE(m.text(), QStringLiteral("11\""));
        QVERIFY(st.parse(QStringLiteral("0.003"), &pt));
        m.setValue(pt);
        QCOMPARE(m.text(), QStringLiteral("0.003\""));
        st.setUnit(jp::Unit::Centimeter);
        QVERIFY(st.parse(QStringLiteral("1.234"), &pt));
        m.setValue(pt);
        QCOMPARE(m.text(), QStringLiteral("1.234 cm"));
        st.setUnit(jp::Unit::Point);
        QVERIFY(st.parse(QStringLiteral("10.125"), &pt));
        m.setValue(pt);
        QCOMPARE(m.text(), QStringLiteral("10.125 pt"));
        st.setUnit(was);
        jp::DecimalSpin d;
        d.setRange(0, 1000);
        d.setSuffix(QStringLiteral(" pt"));
        d.setValue(12.345);
        QCOMPARE(d.text(), QStringLiteral("12.345 pt"));
        d.setValue(12);
        QCOMPARE(d.text(), QStringLiteral("12 pt"));
    }

    // A color given as ink amounts keeps them: in the file, in tints and
    // shades, from the Colors dialog, and in the PDF of a process-color
    // publication.
    // The New Publication page lists its template categories in view, and
    // picking one shows that category's templates.
    void newPageCategories()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        w.showBackstage(QStringLiteral("new"));
        QApplication::processEvents();
        QListWidget *cats = nullptr;
        for (QListWidget *l : w.findChildren<QListWidget *>())
            if (l->count() > 3 && l->item(0)->data(Qt::UserRole).toString() == QLatin1String("Featured")) cats = l;
        QVERIFY(cats);
        QVERIFY(cats->isVisible());
        QCOMPARE(cats->item(cats->count() - 1)->data(Qt::UserRole).toString(), QStringLiteral("My Templates"));
        QListWidget *grid = nullptr;
        for (QListWidget *l : cats->parentWidget()->findChildren<QListWidget *>())
            if (l != cats && l->viewMode() == QListView::IconMode) grid = l;
        QVERIFY(grid);
        cats->setCurrentRow(1);
        QApplication::processEvents();
        const QString label = cats->item(1)->text();
        const int want = label.mid(label.lastIndexOf('(') + 1).chopped(1).toInt();
        QVERIFY(want > 0);
        QCOMPARE(grid->count(), want);
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            cats->setCurrentRow(0);
            QApplication::processEvents();
            w.screenshotTo(qEnvironmentVariable("JP_SHOT_DIR") + "/new-page.png");
        }
    }

    // Clicking a category lists its templates at once with blank pages; the
    // thumbnails are drawn afterward between events, a newer click replaces
    // the work left, and thumbnails already made come back at once.
    void newPageThumbnailsFillAfter()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        w.showBackstage(QStringLiteral("new"));
        QApplication::processEvents();
        QListWidget *cats = nullptr, *grid = nullptr;
        for (QListWidget *l : w.findChildren<QListWidget *>())
            if (l->count() > 3 && l->item(0)->data(Qt::UserRole).toString() == QLatin1String("Featured")) cats = l;
        QVERIFY(cats);
        for (QListWidget *l : cats->parentWidget()->findChildren<QListWidget *>())
            if (l != cats && l->viewMode() == QListView::IconMode) grid = l;
        QVERIFY(grid);
        auto row = [&](const QString &key) {
            for (int i = 0; i < cats->count(); ++i)
                if (cats->item(i)->data(Qt::UserRole).toString() == key) return i;
            return -1;
        };
        const int calendars = row(QStringLiteral("Calendars")), banners = row(QStringLiteral("Banners"));
        QVERIFY(calendars > 0 && banners > 0);
        // Distinct thumbnails in the grid (all blank placeholders count as one).
        auto distinct = [&] {
            QSet<qint64> keys;
            for (int i = 0; i < grid->count(); ++i) keys.insert(grid->item(i)->icon().cacheKey());
            return keys.size();
        };
        cats->setCurrentRow(calendars);
        QVERIFY(grid->count() >= 2);
        QCOMPARE(distinct(), 1);   // listed, nothing drawn yet
        cats->setCurrentRow(banners);   // before any calendar was drawn
        const int bannerCount = grid->count();
        QVERIFY(bannerCount >= 2);
        QTRY_COMPARE(distinct(), bannerCount);
        cats->setCurrentRow(calendars);
        QTRY_COMPARE(distinct(), grid->count());
        cats->setCurrentRow(banners);
        QCOMPARE(distinct(), bannerCount);   // kept, no waiting
    }

    // Several ruler guides at once: quick adds no longer stack at the page's
    // middle, and the Ruler Guides dialog adds, moves, removes and adds a
    // series of evenly spaced guides in one step.
    void rulerGuidesSeveral()
    {
        QVector<double> g{396};
        QCOMPARE(jp::RulerGuides::freeSpot(g, 396, 36, 792), 432.0);
        g << 432;
        QCOMPARE(jp::RulerGuides::freeSpot(g, 396, 36, 792), 468.0);
        QCOMPARE(jp::RulerGuides::freeSpot(g, 100, 36, 792), 100.0);
        QVector<double> s;
        QCOMPARE(jp::RulerGuides::addSeries(&s, 72, 72, 20, 612), 8);   // 72..576, stops at the page edge
        QCOMPARE(s.last(), 576.0);
        QCOMPARE(jp::RulerGuides::addSeries(&s, 72, 144, 3, 612), 0);   // all already there

        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.act(QStringLiteral("pd.addH"))->trigger();
        w.act(QStringLiteral("pd.addH"))->trigger();
        w.act(QStringLiteral("pd.addV"))->trigger();
        const jp::RulerGuides &rg = w.editor()->surface()->guides;
        QCOMPARE(rg.h, (QVector<double>{396, 432}));
        QCOMPARE(rg.v, (QVector<double>{306}));

        // The dialog, as a user would: a guide at 2 in on the horizontal side,
        // a series of three vertical guides from 1 in every 1 in, then remove
        // the 432 pt horizontal one.
        QTimer::singleShot(0, &w, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(d);
            const auto boxes = d->findChildren<QGroupBox *>(QString(), Qt::FindDirectChildrenOnly);
            QCOMPARE(boxes.size(), 2);
            auto button = [](QWidget *in, const QString &text) {
                for (auto *b : in->findChildren<QPushButton *>())
                    if (b->text() == text) return b;
                return static_cast<QPushButton *>(nullptr);
            };
            auto spins = [](QWidget *in) { return in->findChildren<jp::MeasureSpin *>(); };
            QWidget *hBox = boxes[0], *vBox = boxes[1];
            spins(hBox).first()->setValue(144);
            button(hBox, QStringLiteral("Add"))->click();
            auto *series = vBox->findChild<QGroupBox *>();
            series->findChild<QSpinBox *>()->setValue(3);
            const auto ss = spins(series);
            ss[0]->setValue(72);
            ss[1]->setValue(72);
            button(vBox, QStringLiteral("Add Series"))->click();
            auto *hList = hBox->findChild<QListWidget *>();
            QCOMPARE(hList->count(), 3);
            hList->setCurrentRow(2);   // 432 pt, sorted after 144 and 396
            button(hBox, QStringLiteral("Remove"))->click();
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) d->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/ruler-guides.png");
            d->accept();
        });
        jp::rulerGuidesDialog(&w, w.editor());
        QCOMPARE(rg.h, (QVector<double>{144, 396}));
        QCOMPARE(rg.v, (QVector<double>{72, 144, 216, 306}));
        // One undo step takes the dialog's changes back.
        w.editor()->undoStack()->undo();
        QCOMPARE(w.editor()->surface()->guides.h, (QVector<double>{396, 432}));
    }

    // The rulers' zero point can move. Shift and the right button dragging on
    // a ruler sets that ruler's zero where you let go; dragging from the box
    // where the two rulers meet sets both. A double-click on a ruler puts
    // that ruler's zero back at the page's corner, and one on the box puts
    // both back. Only the rulers' numbers change: guides are still placed from
    // the page's corner.
    void rulerZeroPoint()
    {
        jp::Settings &st = jp::Settings::get();
        const jp::Unit was = st.unit();
        st.setUnit(jp::Unit::Inch);
        jp::MainWindow w;
        w.resize(1200, 900);
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        QTest::qWait(50);   // the whole page fits the window
        jp::Canvas *c = w.canvas();
        c->setRulersVisible(true);
        jp::Ruler *h = c->hRuler(), *v = c->vRuler();
        QWidget *corner = c->rulerCorner();
        QVERIFY(corner);
        // A place on the page, as a place on a ruler or on the box where they meet.
        auto view = [&](QWidget *on, QPointF page) { return on->mapFromGlobal(c->viewport()->mapToGlobal(c->pageToView(page).toPoint())); };
        auto onH = [&](double x) { return QPoint(view(h, QPointF(x, 0)).x(), 5); };
        auto onV = [&](double y) { return QPoint(5, view(v, QPointF(0, y)).y()); };
        auto send = [](QWidget *on, QEvent::Type type, Qt::MouseButton button, Qt::MouseButtons held, Qt::KeyboardModifiers mods, QPoint pos) {
            QMouseEvent e(type, QPointF(pos), QPointF(on->mapToGlobal(pos)), button, held, mods);
            QApplication::sendEvent(on, &e);
        };
        auto drag = [&](QWidget *on, Qt::MouseButton button, Qt::KeyboardModifiers mods, QPoint from, QPoint to) {
            send(on, QEvent::MouseButtonPress, button, button, mods, from);
            send(on, QEvent::MouseMove, Qt::NoButton, button, mods, (from + to) / 2);
            send(on, QEvent::MouseMove, Qt::NoButton, button, mods, to);
            send(on, QEvent::MouseButtonRelease, button, Qt::NoButton, mods, to);
        };
        const double pixel = 2 / c->ppp();   // a pixel or two, in points
        auto closeTo = [&](double a, double b) { return qAbs(a - b) <= pixel; };
        auto look = [](jp::Ruler *r) {
            r->setMouse(-1e9);
            return r->grab().toImage();
        };

        // At first both count from the page's corner.
        const QImage plainH = look(h), plainV = look(v);
        QCOMPARE(h->zero(), 0.0);
        QCOMPARE(v->zero(), 0.0);
        QCOMPARE(h->valueAt(144), 2.0);
        QCOMPARE(v->valueAt(216), 3.0);

        // Shift and the right button on the top ruler: its zero goes to 2 in. The side ruler stays.
        drag(h, Qt::RightButton, Qt::ShiftModifier, onH(72), onH(144));
        QVERIFY2(closeTo(h->zero(), 144), qPrintable(QString::number(h->zero())));
        QCOMPARE(v->zero(), 0.0);
        const double hz = h->zero();
        QVERIFY(qAbs(h->valueAt(hz)) < 1e-9);
        QVERIFY(qAbs(h->valueAt(hz + 72) - 1.0) < 1e-9);
        QVERIFY(qAbs(h->valueAt(hz - 36) + 0.5) < 1e-9);
        QVERIFY(look(h) != plainH);
        QCOMPARE(look(v), plainV);
        // And the same on the side ruler.
        drag(v, Qt::RightButton, Qt::ShiftModifier, onV(300), onV(216));
        QVERIFY2(closeTo(v->zero(), 216), qPrintable(QString::number(v->zero())));
        QCOMPARE(h->zero(), hz);
        QVERIFY(qAbs(v->valueAt(v->zero() + 72) - 1.0) < 1e-9);
        // Other buttons, and the right button without Shift, leave the zero alone.
        drag(h, Qt::RightButton, Qt::NoModifier, onH(300), onH(400));
        drag(h, Qt::MiddleButton, Qt::ShiftModifier, onH(300), onH(400));
        QCOMPARE(h->zero(), hz);
        // Shift and the right button do not bring up the page's pop-up menu.
        QSignalSpy menu(c, &jp::Canvas::contextMenuWanted);
        QContextMenuEvent ce(QContextMenuEvent::Mouse, onH(144), h->mapToGlobal(onH(144)), Qt::ShiftModifier);
        QApplication::sendEvent(h, &ce);
        QCOMPARE(menu.count(), 0);

        // The box where they meet moves both; a click on it moves nothing.
        drag(corner, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), view(corner, QPointF(72, 108)));
        QVERIFY2(closeTo(h->zero(), 72), qPrintable(QString::number(h->zero())));
        QVERIFY2(closeTo(v->zero(), 108), qPrintable(QString::number(v->zero())));
        const double hz2 = h->zero(), vz2 = v->zero();
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/ruler-zero.png");
        QTest::mouseClick(corner, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
        QCOMPARE(h->zero(), hz2);
        QCOMPARE(v->zero(), vz2);

        // Guides are placed from the page's corner all the same.
        drag(h, Qt::LeftButton, Qt::NoModifier, onH(300), QPoint(onH(300).x(), view(h, QPointF(0, 250)).y()));
        const QVector<double> &guides = w.editor()->surface()->guides.h;
        QCOMPARE(guides.size(), 1);
        QVERIFY2(closeTo(guides[0], 250), qPrintable(QString::number(guides[0])));
        QCOMPARE(h->zero(), hz2);

        // A double-click anywhere on a ruler puts that ruler's zero back, and only that one's; no guide comes of the clicks.
        QTest::mouseDClick(h, Qt::LeftButton, Qt::NoModifier, onH(300));
        QCOMPARE(h->zero(), 0.0);
        QCOMPARE(v->zero(), vz2);
        QTest::mouseDClick(v, Qt::LeftButton, Qt::NoModifier, onV(400));
        QCOMPARE(v->zero(), 0.0);
        QCOMPARE(guides.size(), 1);
        QCOMPARE(w.editor()->surface()->guides.v.size(), 0);

        // A double-click on the box puts both back, and the rulers look as they did.
        drag(corner, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), view(corner, QPointF(72, 108)));
        QVERIFY(h->zero() > 1 && v->zero() > 1);
        QTest::mouseDClick(corner, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
        QCOMPARE(h->zero(), 0.0);
        QCOMPARE(v->zero(), 0.0);
        QCOMPARE(look(h), plainH);
        QCOMPARE(look(v), plainV);

        // A new publication starts with the rulers at the corner.
        drag(corner, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), view(corner, QPointF(72, 108)));
        QVERIFY(h->zero() > 1 && v->zero() > 1);
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QCOMPARE(h->zero(), 0.0);
        QCOMPARE(v->zero(), 0.0);
        st.setUnit(was);
    }

    // The moved zero is saved with the publication in a .jpub, and comes back
    // when it is opened; a file with the zero at the corner has no mention of
    // it. Moving it is one undo step, and changes the publication.
    void rulerZeroSaved()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        jp::Ruler *h = w.canvas()->hRuler(), *v = w.canvas()->vRuler();
        QVERIFY(!ed->isModified());
        QVERIFY(!jp::publicationBytes(*ed->doc(), QImage()).contains("rulerZero"));
        h->setZero(216);
        v->setZero(72);
        QVERIFY(ed->isModified());
        QCOMPARE(ed->doc()->rulerZero, QPointF(216, 72));
        QVERIFY(jp::publicationBytes(*ed->doc(), QImage()).contains("\"rulerZero\":[216,72]"));
        // Each move is a step: undo and redo take it back and bring it again, the other ruler's unmoved.
        ed->undoStack()->undo();
        QCOMPARE(v->zero(), 0.0);
        QCOMPARE(h->zero(), 216.0);
        ed->undoStack()->undo();
        QCOMPARE(h->zero(), 0.0);
        QVERIFY(!ed->isModified());
        ed->undoStack()->redo();
        ed->undoStack()->redo();
        QCOMPARE(ed->doc()->rulerZero, QPointF(216, 72));

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("zero.jpub"));
        QString err;
        QVERIFY2(jp::savePublication(*ed->doc(), path, QImage(), &err), qPrintable(err));
        jp::MainWindow other;
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->rulerZero, QPointF(216, 72));
        other.editor()->setDocument(std::move(back), path);
        QCOMPARE(other.canvas()->hRuler()->zero(), 216.0);
        QCOMPARE(other.canvas()->vRuler()->zero(), 72.0);
        // Put back at the corner and saved again, the file says nothing of it.
        other.canvas()->hRuler()->setZero(0);
        other.canvas()->vRuler()->setZero(0);
        QVERIFY(!jp::publicationBytes(*other.editor()->doc(), QImage()).contains("rulerZero"));
        // A new publication starts at the corner.
        h->setZero(100);
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QCOMPARE(h->zero(), 0.0);
        QCOMPARE(v->zero(), 0.0);
    }

    // The status bar's position counts from the rulers' zero, with a minus
    // sign before it; the sizes are as ever.
    void rulerZeroInStatusBar()
    {
        jp::Settings &st = jp::Settings::get();
        const jp::Unit was = st.unit();
        st.setUnit(jp::Unit::Inch);
        jp::MainWindow w;
        w.resize(1200, 900);
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        jp::Canvas *c = w.canvas();
        auto box = std::make_shared<jp::ShapeItem>();
        box->rect = QRectF(72, 90, 144, 36);
        ed->addItem(box);
        ed->select(QStringList{box->id});
        auto position = [&] {
            for (QLabel *l : w.statusBar()->findChildren<QLabel *>())
                if (l->text().contains(QLatin1String(", "))) return l->text();
            return QString();
        };
        auto size = [&] {
            for (QLabel *l : w.statusBar()->findChildren<QLabel *>())
                if (l->text().contains(QChar(0x00D7))) return l->text();
            return QString();
        };
        QTRY_COMPARE(position(), QStringLiteral("1\", 1.25\""));
        const QString sized = size();
        QVERIFY(!sized.isEmpty());
        // With the zero at 3 in across and 1 in down, the shape (1 in, 1.25 in) reads -2 in, 0.25 in.
        c->hRuler()->setZero(216);
        c->vRuler()->setZero(72);
        QTRY_COMPARE(position(), QStringLiteral("-2\", 0.25\""));
        QCOMPARE(size(), sized);
        // So does the pointer's place: the page's 4 in across and 5 in down reads 1 in, 4 in.
        QWidget *vp = c->viewport();
        QSignalSpy moved(c, &jp::Canvas::mouseMovedPage);
        const QPoint at = c->pageToView(QPointF(288, 360)).toPoint();
        QMouseEvent mv(QEvent::MouseMove, QPointF(at), QPointF(vp->mapToGlobal(at)), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(vp, &mv);
        QVERIFY(moved.count() > 0);
        const QPointF page = moved.last().first().toPointF();
        QVERIFY(qAbs(page.x() - 288) < 3 && qAbs(page.y() - 360) < 3);
        QCOMPARE(position(), QStringLiteral("%1, %2").arg(st.format(page.x() - 216), st.format(page.y() - 72)));
        // The Position boxes and the file still measure from the page's corner.
        QCOMPARE(box->rect.topLeft(), QPointF(72, 90));
        st.setUnit(was);
    }

    // The text ruler (indents and tabs) is measured from the text box, so it
    // works as ever with the zero moved.
    void rulerZeroLeavesTextRuler()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        QTest::qWait(50);
        jp::Editor *ed = w.editor();
        jp::Canvas *c = w.canvas();
        c->setRulersVisible(true);
        jp::Ruler *h = c->hRuler();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 300, 200), QStringLiteral("one two three")));
        ed->addItem(box);
        ed->beginTextEdit(box->id);
        const double left = box->rect.left() + box->insets.left();   // where the text starts
        auto onH = [&](double x) {
            const QPoint p = h->mapFromGlobal(c->viewport()->mapToGlobal(c->pageToView(QPointF(x, 0)).toPoint()));
            return QPoint(p.x(), 5);
        };
        auto send = [&](QEvent::Type type, Qt::MouseButton button, Qt::MouseButtons held, QPoint pos) {
            QMouseEvent e(type, QPointF(pos), QPointF(h->mapToGlobal(pos)), button, held, Qt::NoModifier);
            QApplication::sendEvent(h, &e);
        };
        const double pixel = 2 / c->ppp();
        h->setZero(200);
        // Drag the first-line marker half an inch in.
        send(QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton, onH(left));
        send(QEvent::MouseMove, Qt::NoButton, Qt::LeftButton, onH(left + 18));
        send(QEvent::MouseMove, Qt::NoButton, Qt::LeftButton, onH(left + 36));
        send(QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton, onH(left + 36));
        const QTextBlockFormat bf = ed->cursor().blockFormat();
        QVERIFY2(qAbs(bf.textIndent() - 36) <= pixel, qPrintable(QString::number(bf.textIndent())));
        // A click inside the text area sets a tab stop that far from the text, wherever the zero is.
        send(QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton, onH(left + 100));
        send(QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton, onH(left + 100));
        const auto tabs = ed->cursor().blockFormat().tabPositions();
        QCOMPARE(tabs.size(), 1);
        QVERIFY2(qAbs(tabs[0].position - 100) <= pixel, qPrintable(QString::number(tabs[0].position)));
        QCOMPARE(h->zero(), 200.0);
    }

    // A custom page size (business cards, ten to a sheet) is saved, offered
    // again, and applied with its sheet margins, apart from margin guides.
    void customPageSizes()
    {
        jp::installUiPolish();
        const QJsonArray before = jp::Settings::get().customPageSizes();
        jp::Settings::get().setCustomPageSizes(QJsonArray());
        jp::MainWindow w;
        w.resize(1200, 800);
        w.show();
        QString perSheet;
        QTimer::singleShot(300, [&] {
            QWidget *mod = QApplication::activeModalWidget();
            if (!mod) return;
            mod->findChild<QLineEdit *>("pageSizeName")->setText(QStringLiteral("Test Card"));
            mod->findChild<QComboBox *>("pageLayout")->setCurrentIndex(jp::PageSetup::MultiplePerSheet);
            auto set = [&](const char *n, double pt) { mod->findChild<jp::MeasureSpin *>(n)->setValue(pt); };
            set("pageWidth", 252);
            set("pageHeight", 144);
            set("paperWidth", 612);
            set("paperHeight", 792);
            set("sideMargin", 54);
            set("topMargin", 36);
            set("gapH", 0);
            set("gapV", 0);
            perSheet = mod->findChild<QLabel *>("perSheet")->text();
            QTimer::singleShot(100, [mod] {
                if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) mod->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/newpagesize.png");
                if (auto *box = mod->findChild<QDialogButtonBox *>()) box->button(QDialogButtonBox::Ok)->click();
            });
        });
        QVERIFY(jp::createPageSizeDialog(&w, w.editor()));
        QCOMPARE(perSheet, QStringLiteral("2 across × 5 down = 10 per sheet"));
        const auto saved = jp::customPageSizes();
        QCOMPARE(saved.size(), 1);
        QCOMPARE(saved[0].first, QStringLiteral("Test Card"));
        const jp::PageSetup &ps = w.editor()->doc()->setup;
        QCOMPARE(ps.layout, jp::PageSetup::MultiplePerSheet);
        QCOMPARE(ps.size, QSizeF(252, 144));
        QCOMPARE(ps.sideMargin, 54.0);
        QCOMPARE(ps.gridCols, 2);
        QCOMPARE(ps.gridRows, 5);
        QCOMPARE(ps.sizeName, QStringLiteral("Test Card"));
        jp::Settings::get().setCustomPageSizes(before);
    }

    // Changing the page size, in the Size gallery or the Page Setup window,
    // scales nothing: every object moves by half the change on each axis and
    // keeps its distance from the page center, as in Publisher (Letter to A4:
    // 8.36 points left, 24.94 down). Ruler guides keep their share of the
    // page. Undo puts everything back in one step.
    void pageSizeKeepsDistanceFromCenter()
    {
        using namespace jp;
        const QSizeF a4(595.2756, 841.8898);
        auto make = [] {
            auto doc = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);
            auto shape = [](const QRectF &r) {
                auto s = std::make_shared<ShapeItem>();
                s->rect = r;
                return s;
            };
            auto text = std::make_shared<TextItem>();
            text->rect = QRectF(72, 300, 200, 60);
            text->storyId = doc->createStory(QStringLiteral("Hello"));
            QTextCursor tc(doc->storyDoc(text->storyId));
            tc.select(QTextCursor::Document);
            QTextCharFormat big;
            big.setFontPointSize(18);
            tc.mergeCharFormat(big);
            auto turned = shape(QRectF(300, 400, 80, 40));
            turned->rotation = 30;
            auto line = std::make_shared<LineItem>();
            line->p1 = QPointF(50, 500);
            line->p2 = QPointF(200, 520);
            line->syncRect();
            auto group = std::make_shared<GroupItem>();
            group->children = {shape(QRectF(400, 100, 30, 30)), shape(QRectF(440, 100, 30, 30))};
            group->syncRect();
            doc->pages[0]->items = {shape(QRectF(72, 72, 100, 50)), text, turned, line, group};
            doc->pages[0]->guides.v = {200};
            doc->pages[0]->guides.h = {300};
            doc->pages[1]->items = {shape(QRectF(100, 200, 80, 40))};
            doc->masters[0]->items = {shape(QRectF(36, 36, 50, 50))};
            doc->masters[0]->guides.v = {100};
            doc->scratch = {shape(QRectF(-200, 100, 50, 50))};
            return doc;
        };
        const double dx = (a4.width() - 612) / 2, dy = (a4.height() - 792) / 2;
        QCOMPARE(QString::number(dx, 'f', 2), QStringLiteral("-8.36"));
        QCOMPARE(QString::number(dy, 'f', 2), QStringLiteral("24.94"));
        for (const bool gallery : {true, false}) {
            Editor ed;
            ed.setDocument(make());
            Document *d = ed.doc();
            const QByteArray before = QJsonDocument(d->toJson()).toJson();
            const QRectF first = d->pages[0]->items[0]->rect, turned = d->pages[0]->items[2]->rect;
            const QPointF p1 = static_cast<LineItem *>(d->pages[0]->items[3].get())->p1;
            const QRectF member = static_cast<GroupItem *>(d->pages[0]->items[4].get())->children[1]->rect;
            const QRectF masterItem = d->masters[0]->items[0]->rect, scratchItem = d->scratch[0]->rect;
            if (gallery) {
                applyPageSize(&ed, a4, QStringLiteral("A4"));
            } else {
                PageSetup ns = d->setup;
                ns.size = ns.sheet = a4;
                applyPageSetup(&ed, ns, QStringLiteral("A4"), QStringLiteral("Page Setup"));
            }
            QCOMPARE(d->setup.size, a4);
            QCOMPARE(d->setup.sizeName, QStringLiteral("A4"));
            // Nothing is scaled: sizes stay, positions shift.
            QCOMPARE(d->pages[0]->items[0]->rect, first.translated(dx, dy));
            QCOMPARE(d->pages[0]->items[2]->rect, turned.translated(dx, dy));
            QCOMPARE(d->pages[0]->items[2]->rotation, 30.0);
            QCOMPARE(static_cast<LineItem *>(d->pages[0]->items[3].get())->p1, p1 + QPointF(dx, dy));
            QCOMPARE(static_cast<GroupItem *>(d->pages[0]->items[4].get())->children[1]->rect, member.translated(dx, dy));
            QCOMPARE(d->pages[1]->items[0]->rect, QRectF(100 + dx, 200 + dy, 80, 40));
            QCOMPARE(d->masters[0]->items[0]->rect, masterItem.translated(dx, dy));
            QCOMPARE(d->scratch[0]->rect, scratchItem);
            auto *t = static_cast<TextItem *>(d->pages[0]->items[1].get());
            QCOMPARE(t->rect.size(), QSizeF(200, 60));
            QCOMPARE(QTextCursor(d->storyDoc(t->storyId)->begin()).charFormat().fontPointSize(), 18.0);
            QCOMPARE(d->setup.margins, QMarginsF(36, 36, 36, 36));
            // Ruler guides keep their share of the page.
            QCOMPARE(d->pages[0]->guides.v[0], 200 * a4.width() / 612);
            QCOMPARE(d->pages[0]->guides.h[0], 300 * a4.height() / 792);
            QCOMPARE(d->masters[0]->guides.v[0], 100 * a4.width() / 612);
            QCOMPARE(d->scratch.size(), size_t(1));
            // One undo step restores all of it.
            QCOMPARE(ed.undoStack()->count(), 1);
            ed.undo();
            QCOMPARE(QJsonDocument(d->toJson()).toJson(), before);
        }
    }

    // What a smaller page leaves wholly off it goes to the scratch area, where
    // the shift put it, and the scratch area itself doesn't move. On a
    // two-page master each page's objects keep their places across and move
    // half the change down.
    void pageSizeSendsOffPageObjectsToScratch()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);
        auto shape = [](const QRectF &r) {
            auto s = std::make_shared<ShapeItem>();
            s->rect = r;
            return s;
        };
        auto corner = shape(QRectF(72, 72, 100, 50)), far = shape(QRectF(500, 700, 60, 60)), mid = shape(QRectF(300, 500, 60, 60));
        auto onSecond = shape(QRectF(100, 200, 80, 40)), masterItem = shape(QRectF(36, 36, 50, 50)), kept = shape(QRectF(-200, 100, 50, 50));
        doc->pages[0]->items = {corner, far, mid};
        doc->pages[1]->items = {onSecond};
        doc->masters[0]->items = {masterItem};
        doc->scratch = {kept};
        auto spread = std::make_shared<MasterPage>();
        spread->id = QStringLiteral("S");
        spread->twoPage = true;
        auto leftOff = shape(QRectF(100, 100, 40, 40)), rightOff = shape(QRectF(700, 100, 40, 40));
        auto leftOn = shape(QRectF(100, 400, 40, 40)), rightOn = shape(QRectF(700, 400, 40, 40));
        spread->items = {leftOff, rightOff, leftOn, rightOn};
        doc->masters.push_back(spread);
        Editor ed;
        ed.setDocument(std::move(doc));
        Document *d = ed.doc();
        applyPageSize(&ed, QSizeF(300, 300), QStringLiteral("Small"));
        // Half of 312 across and 492 down.
        QCOMPARE(d->pages[0]->items.size(), size_t(1));
        QCOMPARE(mid->rect, QRectF(144, 254, 60, 60));
        QVERIFY(d->pages[1]->items.empty());
        QVERIFY(d->masters[0]->items.empty());
        QCOMPARE(d->scratch.size(), size_t(7));
        QVERIFY(d->scratch[0] == kept);
        QCOMPARE(kept->rect, QRectF(-200, 100, 50, 50));
        QVERIFY(d->scratch[1] == corner);
        QCOMPARE(corner->rect, QRectF(-84, -174, 100, 50));
        QVERIFY(d->scratch[2] == far);
        QCOMPARE(far->rect, QRectF(344, 454, 60, 60));
        QVERIFY(d->scratch[3] == onSecond);
        QCOMPARE(onSecond->rect, QRectF(-56, -46, 80, 40));
        QVERIFY(d->scratch[4] == masterItem);
        QCOMPARE(masterItem->rect, QRectF(-120, -210, 50, 50));
        // Two-page master: the right page's objects follow the page's edge
        // (now at 300), and what is wholly off goes to the scratch area at
        // its place on its own page.
        QVERIFY(d->scratch[5] == leftOff);
        QCOMPARE(leftOff->rect, QRectF(100, -146, 40, 40));
        QVERIFY(d->scratch[6] == rightOff);
        QCOMPARE(rightOff->rect, QRectF(88, -146, 40, 40));
        QCOMPARE(d->masters[1]->items.size(), size_t(2));
        QCOMPARE(leftOn->rect, QRectF(100, 154, 40, 40));
        QCOMPARE(rightOn->rect, QRectF(388, 154, 40, 40));
        // A later change leaves the scratch area where it is.
        applyPageSize(&ed, QSizeF(400, 400), QStringLiteral("Square"));
        QCOMPARE(far->rect, QRectF(344, 454, 60, 60));
        QCOMPARE(mid->rect, QRectF(194, 304, 60, 60));
    }

    // Slides use the page model: speaker notes, hidden slides, an app's own
    // data on a page and on the document, and masters drawn under masters
    // (a slide's layout over its slide master), whose placeholder boxes are
    // prompts that don't show on the slides themselves.
    void slidePagesKeepNotesLayoutsAndMasterChains()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(200, 100));
        auto box = [](const QRectF &r, const QColor &c, const QString &role = QString()) {
            auto s = std::make_shared<ShapeItem>();
            s->rect = r;
            s->fill = Fill::solid(ColorRef::rgb(c));
            s->stroke = Stroke();
            s->role = role;
            return s;
        };
        auto slideMaster = doc->masters[0];
        slideMaster->items.push_back(box(QRectF(0, 0, 40, 40), Qt::red));
        slideMaster->items.push_back(box(QRectF(50, 0, 40, 40), Qt::green, QStringLiteral("ph:title")));
        slideMaster->background = Fill::solid(ColorRef::rgb(QColor(0, 0, 255)));
        auto layout = std::make_shared<MasterPage>();
        layout->abbr = QStringLiteral("L");
        layout->baseId = slideMaster->id;
        layout->items.push_back(box(QRectF(100, 0, 40, 40), Qt::black));
        layout->props[QStringLiteral("type")] = QStringLiteral("title");
        doc->masters.push_back(layout);
        Page &slide = *doc->pages[0];
        slide.masterId = layout->id;
        slide.notesStoryId = doc->createStory(QStringLiteral("Speaker notes"));
        slide.hidden = true;
        slide.props[QStringLiteral("transition")] = QStringLiteral("fade");
        doc->appData[QStringLiteral("theme")] = QStringLiteral("JeffOffice");

        QCOMPARE(doc->masterChain(layout.get()), (QVector<MasterPage *>{slideMaster.get(), layout.get()}));
        QVERIFY(doc->storiesInUse().contains(slide.notesStoryId));

        // Saved and read back whole.
        Document back;
        back.fromJson(doc->toJson());
        QCOMPARE(back.pages[0]->notesStoryId, slide.notesStoryId);
        QVERIFY(back.pages[0]->hidden);
        QCOMPARE(back.pages[0]->props.value(QStringLiteral("transition")).toString(), QStringLiteral("fade"));
        QCOMPARE(back.masters[1]->baseId, slideMaster->id);
        QCOMPARE(back.masters[1]->props.value(QStringLiteral("type")).toString(), QStringLiteral("title"));
        QCOMPARE(back.appData.value(QStringLiteral("theme")).toString(), QStringLiteral("JeffOffice"));
        QCOMPARE(back.storyDoc(slide.notesStoryId)->toPlainText(), QStringLiteral("Speaker notes"));

        // A publication without them saves as before.
        auto pub = Document::blank(QSizeF(200, 100));
        const QJsonObject pj = pub->toJson();
        const QJsonObject pageJ = pj[QStringLiteral("pages")].toArray()[0].toObject(), masterJ = pj[QStringLiteral("masters")].toArray()[0].toObject();
        for (const char *key : {"notes", "hidden", "props", "masterItems"}) QVERIFY2(!pageJ.contains(QLatin1String(key)), key);
        for (const char *key : {"base", "props", "masterItems"}) QVERIFY2(!masterJ.contains(QLatin1String(key)), key);
        QVERIFY(!pj.contains(QStringLiteral("appData")));

        // Drawn: the slide master's background and objects under the
        // layout's, but not the placeholder prompt.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = doc.get();
        ctx.cache = &cache;
        auto at = [&](int x, int y) { return Renderer::renderToImage(ctx, 0, 1.0).pixelColor(x, y); };
        QCOMPARE(at(20, 20), QColor(Qt::red));
        QCOMPARE(at(70, 20), QColor(0, 0, 255));   // the placeholder isn't drawn; the background shows
        QCOMPARE(at(120, 20), QColor(Qt::black));
        QCOMPARE(at(170, 70), QColor(0, 0, 255));
        // A layout can hide its slide master's objects, and a slide all of them.
        layout->showMasterItems = false;
        QCOMPARE(at(20, 20), QColor(0, 0, 255));
        QCOMPARE(at(120, 20), QColor(Qt::black));
        slide.showMasterItems = false;
        QCOMPARE(at(120, 20), QColor(0, 0, 255));
        // A loop of bases ends.
        slideMaster->baseId = layout->id;
        QVERIFY(doc->masterChain(layout.get()).size() <= 2);
    }

    // Folded cards and envelopes keep their layout through a .pub, written
    // as Publisher writes them (DOCUMENT field 11: 3 and 7; a card's pages as
    // spreads, flags 06 and 0b); Publisher kept both when saving them again.
    void pubFoldedCardAndEnvelope()
    {
        for (const auto &[id, layout, code] : {std::tuple{QStringLiteral("greeting-birthday"), PageSetup::FoldedCard, 3u},
                                                std::tuple{QStringLiteral("envelope-10"), PageSetup::Envelope, 7u}}) {
            const TemplateInfo *t = findTemplate(id);
            QVERIFY(t);
            auto doc = t->build(TemplateOptions());
            QCOMPARE(doc->setup.layout, layout);
            QTemporaryDir dir;
            const QString path = dir.filePath(id + QStringLiteral(".pub"));
            QString err;
            QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
            const QByteArray contents = cfb::readStream(path, QStringLiteral("Contents"));
            QByteArray field("\x11\x20\x00\x00\x00\x00", 6);
            field[2] = char(code);
            QVERIFY(contents.contains(field));
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            auto back = importPublisher(f.readAll(), nullptr);
            QVERIFY(back);
            QCOMPARE(back->setup.layout, layout);
            QCOMPARE(back->pages.size(), doc->pages.size());
            QCOMPARE(back->pageSize(), doc->pageSize());
        }
    }
};

JP_TEST_CLASS(PubPageTests)
#include "tst_pub_pages.moc"
