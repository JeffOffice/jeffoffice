// The checks every app's window must pass, for JeffPub (see appchecks.h): its first five minutes, its standard commands, and every command it has.
#include "appchecks.h"

#include "app/backstage.h"
#include "app/mainwindow.h"
#include "app/pagespane.h"
#include "app/pubinfo.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "canvas/canvas.h"
#include "core/appinfo.h"
#include "core/document.h"
#include "core/fonts.h"
#include "io/jpubfile.h"
#include "templates/templates.h"
#include "testrunner.h"

#include <QClipboard>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QStatusBar>
#include <memory>

using namespace jp;
using namespace appchecks;

namespace {

MainWindow *asPub(AppWindow *w) { return static_cast<MainWindow *>(w); }

// The window as launching the program makes it: shown, with the File screen's New page over it (the program opens on it until a design is chosen).
MainWindow *launchStart()
{
    auto *w = new MainWindow();
    w->resize(1000, 640);
    w->show();
    w->showBackstage(QStringLiteral("new"));
    return w;
}

// A blank publication, as the New page's first choice makes it.
MainWindow *launchBlank()
{
    MainWindow *w = launchStart();
    w->act(QStringLiteral("file.blank"))->trigger();
    return w;
}

// The text box tool, and a click on the page: a box with the cursor in it.
void startTextBox(MainWindow *w, const QPointF &page = QPointF(120, 120))
{
    w->act(QStringLiteral("ins.textbox"))->trigger();
    QTest::mouseClick(w->canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, w->canvas()->pageToView(page).toPoint());
}

// The words of every text box on the pages, one after the other.
QString allText(MainWindow *w)
{
    QString out;
    for (const auto &page : w->editor()->doc()->pages)
        for (const auto &item : page->items)
            if (item->type() == ItemType::Text)
                if (QTextDocument *story = w->editor()->doc()->storyDoc(static_cast<TextItem *>(item.get())->storyId)) out += story->toPlainText() + QLatin1Char('|');
    return out;
}

} // namespace

class PubAppChecks : public QObject {
    Q_OBJECT

    Adapter m_app;

private Q_SLOTS:
    void initTestCase()
    {
        isolateSettings();
        setAppInfo(pubAppInfo());
        initCore();
        jp::Settings::get().setUnit(jp::Unit::Inch);
        QLocale::setDefault(QLocale::c());
        m_app.name = QStringLiteral("JeffPub");
        m_app.launch = [] { return static_cast<AppWindow *>(launchBlank()); };
        m_app.launchWith = [](const QString &path) -> AppWindow * {
            auto *w = new MainWindow();
            w->resize(1000, 640);
            w->show();
            if (!w->openFile(path)) {
                delete w;
                return nullptr;
            }
            return w;
        };
        m_app.type = [](AppWindow *w) {
            startTextBox(asPub(w));
            typeInto(asPub(w)->canvas(), QStringLiteral("Some words"));
            asPub(w)->editor()->flushTyping();   // (as a pause in typing does)
        };
        m_app.documentView = [](AppWindow *w) -> QWidget * { return asPub(w)->canvas()->viewport(); };
        m_app.settle = [](AppWindow *w) {
            // (the window brings its commands up to date 30 ms after a change: done now, the timer finds nothing to change)
            drain();
            asPub(w)->refreshUi();
            // (and draws the thumbnails of the pages one at a time, when the window is idle)
            if (auto *pages = w->findChild<PagesPane *>())
                waitFor([&] {
                    // (a thumbnail not yet drawn is a picture of nothing)
                    for (int i = 0; i < pages->count(); ++i) {
                        const QImage thumbnail = pages->item(i)->icon().pixmap(110, 110).toImage();
                        bool drawn = false;
                        for (int y = 0; y < thumbnail.height() && !drawn; ++y)
                            for (int x = 0; x < thumbnail.width() && !drawn; ++x) drawn = qAlpha(thumbnail.pixel(x, y)) > 0;
                        if (!drawn) return false;
                    }
                    return true;
                });
            drain();
        };
        m_app.docState = [](AppWindow *aw) {
            auto *w = asPub(aw);
            Editor *ed = w->editor();
            QString s = QString::fromLatin1(QCryptographicHash::hash(publicationBytes(*ed->doc(), QImage()), QCryptographicHash::Md5).toHex());
            s += QStringLiteral("|%1|%2|%3|%4|%5|%6").arg(ed->selection().join(QLatin1Char(','))).arg(ed->isEditingText()).arg(ed->undoStack()->index()).arg(ed->currentPage()).arg(w->canvas()->zoom()).arg(int(ed->tool()));
            if (ed->isEditingText()) s += QStringLiteral("|%1,%2").arg(ed->cursor().position()).arg(ed->cursor().anchor());
            return s + QLatin1Char('|') + ed->filePath() + QString::number(ed->isModified());
        };
        // The sample: a flyer from the program's own designs with a table added (a picture, shapes, Text Art, text, and a table: what the commands act on), saved by the program.
        {
            std::unique_ptr<MainWindow> w(launchBlank());
            w->editor()->setDocument(findTemplate(QStringLiteral("flyer-event"))->build(TemplateOptions()));
            w->editor()->addItem(w->editor()->newTable(QRectF(72, 560, 300, 90), 3, 3));
            w->editor()->doc()->addPage(1, w->editor()->doc()->pages[0]->masterId);
            m_app.sampleFile = tempFile(QStringLiteral("sample.jpub"));
            QVERIFY(w->saveTo(m_app.sampleFile));
        }
        m_app.reset = [this](AppWindow *aw, bool fromFile) {
            auto *w = asPub(aw);
            w->showNormal();
            w->resize(1000, 640);   // (a window command may have made it another size)
            w->hideBackstage();
            w->hideTaskPane();
            w->editor()->endTextEdit();
            w->editor()->setTool(Tool::Select);
            w->editor()->clearSelection();
            QString error;
            if (fromFile) w->newPublication(loadPublication(m_app.sampleFile, &error));
            else w->newPublication(Document::blank(QSizeF(612, 792)));
            w->editor()->setCurrentPage(0);
            w->editor()->setMasterView(QString());
            w->editor()->setTwoPageSpread(false);
            // (what the View commands switch, and where the window looks)
            w->editor()->setView([](ViewOptions &v) { v = ViewOptions(); });
            w->canvas()->zoomToFit(Canvas::Fit::WholePage);
            if (RibbonTab *home = w->ribbon()->tab(QStringLiteral("Home"))) w->ribbon()->showTab(home);
        };
        // A command needs the thing it acts on selected: a picture, a shape, Text Art, a table, or words (selected inside a text box).
        m_app.prepare = [](AppWindow *aw, const QString &id, bool fromFile) {
            auto *w = asPub(aw);
            if (id == QLatin1String("edit.paste") || id == QLatin1String("edit.pasteText")) QGuiApplication::clipboard()->setText(QStringLiteral("Pasted words"));
            if (!fromFile || id == QLatin1String("edit.undo") || id == QLatin1String("edit.redo")) return;
            Editor *ed = w->editor();
            auto firstOf = [&](ItemType t) -> QString {
                for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
                    if (it->type() == t) return it->id;
                return {};
            };
            const QString cat = id == QLatin1String("ins.pageCount") ? id : id.section(QLatin1Char('.'), 0, 0);
            static const QSet<QString> pictures = {"pic", "corr", "recolor"}, textArt = {"wa", "waalign", "waspace"},
                                       shapes = {"shape", "fill", "line", "shadow", "obj", "arr", "wrap", "glow", "bevel", "refl", "rot3d", "soft", "dash", "weight", "arrowStart", "arrowEnd"},
                                       tables = {"tbl", "border"},
                                       words = {"fmt", "ins.pageCount", "para", "case", "dropcap", "style", "fit", "cols", "edit", "rev", "tb", "bullet", "number", "numstyle", "numspacing", "ls", "spacing", "lig", "ss", "valign", "tbmargin"};
            if (id == QLatin1String("page.prev") || id == QLatin1String("page.moveUp")) ed->setCurrentPage(1);   // (the first page has no page before it)
            if (cat == QLatin1String("mp")) ed->setMasterView(ed->doc()->masters.first()->id);
            if (id == QLatin1String("zoom.selection")) ed->select(firstOf(ItemType::Shape));
            if (cat == QLatin1String("arr")) {   // (aligning and spacing are for several objects)
                QStringList shapes;
                for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
                    if (it->type() == ItemType::Shape && shapes.size() < 3) shapes << it->id;
                ed->select(shapes);
            }
            if (tables.contains(cat)) {
                ed->select(firstOf(ItemType::Table));
                if (cat == QLatin1String("tbl")) ed->beginTextEdit(firstOf(ItemType::Table), 0, 0, 0);   // (the cursor in a cell)
            }
            else if (pictures.contains(cat)) ed->select(firstOf(ItemType::Picture));
            else if (textArt.contains(cat)) ed->select(firstOf(ItemType::TextArt));
            else if (shapes.contains(cat) && cat != QLatin1String("arr")) ed->select(firstOf(ItemType::Shape));
            else if (words.contains(cat)) {
                QString t = firstOf(ItemType::Text);
                for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)   // (the box with the most words)
                    if (it->type() == ItemType::Text)
                        if (QTextDocument *story = ed->doc()->storyDoc(static_cast<TextItem *>(it.get())->storyId); story && story->characterCount() > ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->item(t))->storyId)->characterCount()) t = it->id;
                ed->select(t);
                ed->beginTextEdit(t, 0);
                QTextCursor c = ed->cursor();
                c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
                ed->setCursor(c);
            }
        };
        // A blank publication has no object, no words, and no second page for these to act on.
        const QString nothing = QStringLiteral("a blank publication has nothing of the kind selected for it to act on");
        m_app.quiet = {};
        for (const char *prefix : {"arr.", "arrowStart.", "arrowEnd.", "bevel.", "border.", "bullet.", "case.", "cols.", "corr.", "dash.", "dropcap.", "fit.", "glow.", "lig.", "ls.", "number.",
                                   "numspacing.", "numstyle.", "recolor.", "refl.", "rot3d.", "shadow.", "soft.", "spacing.", "ss.", "tb.", "tbmargin.", "valign.", "waalign.", "waspace.",
                                   "weight.", "tbl.", "pic.", "wa.", "wrap.", "shape.", "fill.", "line.", "obj.", "fmt.", "para.", "sel.", "mp.", "pd."})
            m_app.quiet.insert(QString::fromLatin1(prefix) + QLatin1String("*@blank"), nothing);
        // What a command may leave as it is, in a publication where it has nothing to change.
        m_app.quiet.insert(QStringLiteral("view.boundaries@blank"), QStringLiteral("it outlines the objects, and a blank page has none"));
        m_app.quiet.insert(QStringLiteral("view.gridlines@blank"), QStringLiteral("it shows the layout grid, and a new publication has none"));
        m_app.quiet.insert(QStringLiteral("view.scratch"), QStringLiteral("it shows what lies off the page, and nothing does"));
        m_app.quiet.insert(QStringLiteral("pd.alignGuides"), QStringLiteral("a setting for how objects snap while they are dragged"));
        m_app.quiet.insert(QStringLiteral("pd.alignObjects"), QStringLiteral("a setting for how objects snap while they are dragged"));
        m_app.quiet.insert(QStringLiteral("pd.clearGuides"), QStringLiteral("there are no guides to clear"));
        m_app.quiet.insert(QStringLiteral("pd.bgNone"), QStringLiteral("the page has no background to take away"));
        m_app.quiet.insert(QStringLiteral("pd.bgAllPages"), QStringLiteral("the pages all have the same background already"));
        m_app.quiet.insert(QStringLiteral("pd.portrait"), QStringLiteral("the page is portrait already"));
        m_app.quiet.insert(QStringLiteral("pd.updateFonts"), QStringLiteral("no style sets a font of its own"));
        m_app.quiet.insert(QStringLiteral("margins.Moderate"), QStringLiteral("a new page has these margins already"));
        m_app.quiet.insert(QStringLiteral("mp.applyAll"), QStringLiteral("every page has the master page already"));
        m_app.quiet.insert(QStringLiteral("mp.applyCurrent"), QStringLiteral("the page has the master page already"));
        m_app.quiet.insert(QStringLiteral("win.cascade"), QStringLiteral("it places the open windows, and there is one"));
        m_app.quiet.insert(QStringLiteral("edit.pasteFormat"), QStringLiteral("nothing has been copied with Copy Formatting"));
        m_app.quiet.insert(QStringLiteral("edit.selectAll"), QStringLiteral("a blank publication has nothing to select; in the sample the words are all selected already"));
        m_app.quiet.insert(QStringLiteral("edit.selectObjects@blank"), QStringLiteral("a blank page has no objects"));
        m_app.quiet.insert(QStringLiteral("edit.selectText"), QStringLiteral("a blank page has no text; in the sample the words are all selected already"));
        m_app.quiet.insert(QStringLiteral("ins.pageCount@blank"), QStringLiteral("it goes in a text box that has the cursor, and a blank page has none"));
        m_app.quiet.insert(QStringLiteral("page.moveDown@blank"), QStringLiteral("a blank publication has one page, with no page to move past"));
        m_app.quiet.insert(QStringLiteral("page.moveUp@blank"), QStringLiteral("a blank publication has one page, with no page to move past"));
        m_app.quiet.insert(QStringLiteral("page.next@blank"), QStringLiteral("a blank publication has one page"));
        m_app.quiet.insert(QStringLiteral("page.prev@blank"), QStringLiteral("a blank publication has one page"));
        m_app.quiet.insert(QStringLiteral("rev.checkAsType"), QStringLiteral("it shows wavy lines under misspelled words, and the words are spelled right"));
        m_app.quiet.insert(QStringLiteral("view.fields"), QStringLiteral("it shows the codes of fields, and a blank page has none"));
        m_app.quiet.insert(QStringLiteral("view.normal"), QStringLiteral("a window begins in the normal view"));
        m_app.quiet.insert(QStringLiteral("zoom.selection@blank"), QStringLiteral("nothing is selected to zoom to"));
        m_app.quiet.insert(QStringLiteral("arr.relMargins"), QStringLiteral("a setting for the next Align, not a change of the page"));
        m_app.quiet.insert(QStringLiteral("arrowStart.*"), QStringLiteral("the sample has no line to put an arrow on"));
        m_app.quiet.insert(QStringLiteral("arrowEnd.*"), QStringLiteral("the sample has no line to put an arrow on"));
        for (const char *none : {"bevel.0", "glow.0", "refl.0", "shadow.0", "soft.0", "line.none", "corr.0.0", "fit.none", "ls.1", "para.spaceBefore0", "recolor.0", "pic.clearCrop", "pic.shapeRect", "rot3d.No Rotation", "tbl.distributeCols", "tbl.distributeRows", "tbmargin.Narrow", "valign.0", "waspace.n", "wrap.none"})
            m_app.quiet.insert(QString::fromLatin1(none), QStringLiteral("the sample's object has nothing of this kind to take away, or has it already"));
        m_app.quiet.insert(QStringLiteral("tbl.selectCell"), QStringLiteral("the cell with the cursor is the only one, and selecting it shows nothing more"));
        m_app.quiet.insert(QStringLiteral("cols.*"), QStringLiteral("the box has that many columns already"));
        m_app.quiet.insert(QStringLiteral("fmt.clear"), QStringLiteral("the sample's words carry no formatting of their own to clear"));
        m_app.quiet.insert(QStringLiteral("dash.*"), QStringLiteral("the sample's shape has no outline to dash"));
        m_app.quiet.insert(QStringLiteral("tb.textFit"), QStringLiteral("the name of a menu: its choices are the commands, and it has none of its own"));
        m_app.quiet.insert(QStringLiteral("pic.arrangeThumbs"), QStringLiteral("the Graphics Manager is not open"));
        m_app.quiet.insert(QStringLiteral("waalign.1"), QStringLiteral("the sample's Text Art is aligned that way already"));
        m_app.quiet.insert(QStringLiteral("view.single"), QStringLiteral("a window begins with one page shown"));
        m_app.quiet.insert(QStringLiteral("zoom.page"), QStringLiteral("a window begins as the whole page"));
        m_app.skipped = {};
    }

    // ---- 1. the first five minutes
    void firstFiveMinutes()
    {
        Watch watch;
        std::unique_ptr<MainWindow> w(launchStart());
        // The program opens on the File screen's New page, with the name a new publication has in the title.
        QVERIFY(w->windowTitle().startsWith(QLatin1String("Publication")));
        QVERIFY(w->windowTitle().contains(QLatin1String("JeffPub")));
        auto *screen = w->findChild<Backstage *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("new"));
        // A blank publication takes the screen's place.
        w->act(QStringLiteral("file.blank"))->trigger();
        QVERIFY(!screen->isVisible());
        QVERIFY(!w->editor()->isModified());

        // A text box, and typing in it, shows in the title.
        startTextBox(w.get());
        QVERIFY(w->editor()->isEditingText());
        typeInto(w->canvas(), QStringLiteral("Hello there"));
        QCOMPARE(allText(w.get()), QStringLiteral("Hello there|"));
        QVERIFY(w->editor()->isModified());
        QVERIFY2(w->windowTitle().contains(QLatin1Char('*')), qPrintable(w->windowTitle()));
        QVERIFY2(!saysNotYet(w->statusBar()->currentMessage()), qPrintable(w->statusBar()->currentMessage()));

        // Undo takes it back (the box with it), Redo puts both again.
        QAction *undo = w->act(QStringLiteral("edit.undo")), *redo = w->act(QStringLiteral("edit.redo"));
        QVERIFY(waitFor([&] { return undo->isEnabled(); }));
        for (int i = 0; i < 40 && undo->isEnabled(); ++i) {
            undo->trigger();
            drain();
        }
        QCOMPARE(allText(w.get()), QString());
        QVERIFY(waitFor([&] { return redo->isEnabled(); }));
        for (int i = 0; i < 40 && redo->isEnabled(); ++i) {
            redo->trigger();
            drain();
        }
        QCOMPARE(allText(w.get()), QStringLiteral("Hello there|"));

        // Save As in the program's own format (the dialog's answer is a path in the test's folder): the title takes the file's name.
        w->act(QStringLiteral("file.saveAs"))->trigger();
        const QString path = w->editor()->filePath();
        QVERIFY(path.endsWith(QLatin1String(".jpub")));
        QVERIFY(QFileInfo::exists(path));
        QVERIFY(!w->editor()->isModified());
        QVERIFY2(w->windowTitle().startsWith(QFileInfo(path).completeBaseName()), qPrintable(w->windowTitle()));
        QVERIFY2(!w->windowTitle().contains(QLatin1Char('*')), qPrintable(w->windowTitle()));

        // The saved file is among the recent ones, on the File screen's Open page.
        QVERIFY(Settings::get().recentFiles().contains(path));
        w->showBackstage(QStringLiteral("open"));
        QVERIFY(listsRecentFile(w->findChild<Backstage *>(), path));
        w->hideBackstage();

        // Closed (nothing changed since the save: no question), and opened again: the same text.
        const CloseResult closed = closeWindow(w.get(), watch);
        QVERIFY(closed.closed);
        QCOMPARE(closed.asked, 0);
        w.reset();
        std::unique_ptr<MainWindow> again(asPub(m_app.launchWith(path)));
        QVERIFY(again);
        QCOMPARE(allText(again.get()), QStringLiteral("Hello there|"));
        QVERIFY(!again->editor()->isModified());
        QCOMPARE(again->editor()->filePath(), path);

        // File > New shows the New page; a second window (Window > New Window) takes typing as well.
        again->act(QStringLiteral("file.new"))->trigger();
        auto *backstage = again->findChild<Backstage *>();
        QVERIFY(backstage && backstage->isVisible());
        QCOMPARE(backstage->currentPage(), QStringLiteral("new"));
        again->hideBackstage();
        const int windows = AppWindow::windows().size();
        again->act(QStringLiteral("win.new"))->trigger();
        drain();
        QCOMPARE(AppWindow::windows().size(), windows + 1);
        MainWindow *second = nullptr;
        for (AppWindow *other : AppWindow::windows())
            if (other != again.get()) second = asPub(other);
        QVERIFY(second);
        QPointer<MainWindow> guard(second);
        // (a copy of the open publication: it holds its text, and is not changed)
        QCOMPARE(allText(second), QStringLiteral("Hello there|"));
        second->act(QStringLiteral("file.blank"))->trigger();
        startTextBox(second);
        typeInto(second->canvas(), QStringLiteral("Second"));
        QCOMPARE(allText(second), QStringLiteral("Second|"));
        // (a changed publication asks before it goes; the answer here is no)
        const CloseResult asked = closeWindow(second, watch);
        QCOMPARE(asked.asked, 1);
        QVERIFY(!asked.closed);
        second->setAttribute(Qt::WA_DeleteOnClose, false);
        delete second;

        // The print layout: the File screen's Print page, with the page drawn.
        again->act(QStringLiteral("file.print"))->trigger();
        backstage = again->findChild<Backstage *>();
        QVERIFY(backstage && backstage->isVisible());
        QCOMPARE(backstage->currentPage(), QStringLiteral("print"));
        QVERIFY(waitFor([&] {
            const QLabel *preview = backstage->findChild<QLabel *>(QStringLiteral("printPreview"));
            return preview && !preview->pixmap().isNull();
        }));
        again->hideBackstage();

        // An unchanged publication goes without a question.
        const CloseResult plain = closeWindow(again.get(), watch);
        QVERIFY(plain.closed);
        QCOMPARE(plain.asked, 0);
        QVERIFY2(watch.warnings.isEmpty(), qPrintable(watch.warnings.join('\n')));
    }

    // The way these checks see a window does not change by itself: a call that does nothing is seen to do nothing (or every command would pass).
    void aCallThatDoesNothingChangesNothing()
    {
        Watch watch;
        std::unique_ptr<AppWindow> w(m_app.launch());
        m_app.settle(w.get());
        const Outcome o = observe(m_app, w.get(), nullptr, watch, [] {}, false, 300);
        QVERIFY2(!o.changed, qPrintable(o.how + o.detail));
    }

    // ---- 2. the standard commands
    void standardCommands()
    {
        const QStringList fails = checkStandardCommands(m_app);
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }

    // ---- 3. every command
    void everyCommandOnABlankDocument()
    {
        QStringList report;
        const QStringList fails = runEveryAction(m_app, false, &report);
        qInfo("%s", qPrintable(report.last()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }

    void everyCommandOnADocumentFromAFile()
    {
        QStringList report;
        const QStringList fails = runEveryAction(m_app, true, &report);
        qInfo("%s", qPrintable(report.last()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }
};

JP_TEST_CLASS(PubAppChecks)
#include "tst_appchecks_pub.moc"
