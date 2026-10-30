// The checks every app's window must pass, for JeffPub (see appchecks.h): its first five minutes, its standard commands, and every command it has.
#include "appchecks.h"

#include "app/backstage.h"
#include "app/mainwindow.h"
#include "app/pubinfo.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "canvas/canvas.h"
#include "core/appinfo.h"
#include "core/document.h"
#include "core/fonts.h"
#include "io/jpubfile.h"
#include "io/pubimport.h"
#include "templates/templates.h"
#include "text/textprops.h"
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
        m_app.newDocument = {QStringLiteral("win.new"), QStringLiteral("file.blank")};
        m_app.type = [](AppWindow *w) {
            startTextBox(asPub(w));
            typeInto(asPub(w)->canvas(), QStringLiteral("Some words"));
            asPub(w)->editor()->flushTyping();   // (as a pause in typing does)
        };
        m_app.showPrint = [](AppWindow *w) -> QWidget * {
            w->act(QStringLiteral("file.print"))->trigger();
            return w->findChild<Backstage *>();
        };
        m_app.reopen = [](const QString &path) -> QString {
            const QString ending = QFileInfo(path).suffix().toLower();
            QString err;
            if (ending == QLatin1String("jpub")) return loadPublication(path, &err) ? QString() : err.isEmpty() ? QStringLiteral("it is not read") : err;
            if (ending == QLatin1String("pub")) return importPublisherFile(path, &err) ? QString() : err.isEmpty() ? QStringLiteral("it is not read") : err;
            return {};
        };
        m_app.settle = [](AppWindow *w) {
            // (the window brings its commands up to date 30 ms after a change: done now, the timer finds nothing to change)
            drain();
            asPub(w)->refreshUi();
            drain();
        };
        m_app.docState = [](AppWindow *aw) {
            auto *w = asPub(aw);
            Editor *ed = w->editor();
            QString s = QString::fromLatin1(QCryptographicHash::hash(publicationBytes(*ed->doc(), QImage()), QCryptographicHash::Md5).toHex());
            s += QStringLiteral("|%1|%2|%3|%4|%5|%6").arg(ed->selection().join(QLatin1Char(','))).arg(ed->isEditingText()).arg(ed->undoStack()->index()).arg(ed->currentPage()).arg(w->canvas()->zoom()).arg(int(ed->tool()));
            if (ed->isEditingText()) s += QStringLiteral("|%1,%2").arg(ed->cursor().position()).arg(ed->cursor().anchor());
            const ViewOptions &v = ed->view;
            s += QStringLiteral("|v%1%2%3%4%5%6%7%8%9%10%11%12%13|%14|%15").arg(v.boundaries).arg(v.guides).arg(v.fields).arg(v.rulers).arg(v.pageNav).arg(v.scratch).arg(v.baselines).arg(v.special).arg(v.snapGuides).arg(v.snapObjects).arg(v.snapRuler).arg(v.spelling).arg(v.gridlines).arg(ed->masterView()).arg(ed->twoPageSpread());
            s += QStringLiteral("|m%1c%2|crop%3").arg(ed->mergeRecord()).arg(ed->hasCopiedFormatting()).arg(ed->cropItem);
            return s + QLatin1Char('|') + ed->filePath() + QString::number(ed->isModified());
        };
        // The sample: a flyer from the program's own designs, with what the commands act on added to it: a table and a second page, a line, a text box with a misspelled word, two pictures (one
        // an SVG drawing), a group of two shapes, a mail merge list, and an object that is off the page (Extra Content), saved by the program.
        {
            std::unique_ptr<MainWindow> w(launchBlank());
            Editor *ed = w->editor();
            ed->setDocument(findTemplate(QStringLiteral("flyer-event"))->build(TemplateOptions()));
            Document *d = ed->doc();
            ed->addItem(ed->newTable(QRectF(72, 560, 300, 90), 3, 3));
            d->addPage(1, d->pages[0]->masterId);
            ed->addItem(ed->newTextBox(QRectF(72, 440, 300, 60), QStringLiteral("Teh quikc brown fox jumps over the lazy dog. A second sentence follows.")));
            auto line = std::make_shared<LineItem>();
            line->p1 = QPointF(72, 520);
            line->p2 = QPointF(300, 540);
            ed->addItem(line);
            QImage red(60, 40, QImage::Format_RGB32);
            red.fill(Qt::red);
            QVERIFY(red.save(tempFile(QStringLiteral("red.png"))));
            QImage blue(40, 60, QImage::Format_RGB32);
            blue.fill(Qt::blue);
            QVERIFY(blue.save(tempFile(QStringLiteral("blue.png"))));
            QFile svg(tempFile(QStringLiteral("drawing.svg")));
            QVERIFY(svg.open(QIODevice::WriteOnly));
            svg.write("<svg xmlns='http://www.w3.org/2000/svg' width='60' height='40' viewBox='0 0 60 40'><rect x='4' y='4' width='52' height='32' fill='#2a6' stroke='#000' stroke-width='2'/></svg>");
            svg.close();
            w->insertFiles({tempFile(QStringLiteral("red.png"))}, QPointF(400, 60));
            w->insertFiles({tempFile(QStringLiteral("drawing.svg"))}, QPointF(400, 160));
            QStringList shapes;
            for (const auto &it : d->pages[0]->items)
                if (it->type() == ItemType::Shape && shapes.size() < 2) shapes << it->id;
            ed->select(shapes);
            w->refreshUi();
            w->act(QStringLiteral("arr.group"))->trigger();
            {
                const QString first = ed->addItem(ed->newTextBox(QRectF(400, 300, 150, 40), QStringLiteral("A story that runs")));
                const QString second = ed->addItem(ed->newTextBox(QRectF(400, 360, 150, 40)));
                ed->linkFrames(first, second);
            }
            d->merge.fields = {QStringLiteral("Name"), QStringLiteral("City")};
            d->merge.rows = {{QStringLiteral("Ann"), QStringLiteral("Oslo")}, {QStringLiteral("Bo"), QStringLiteral("Rome")}, {QStringLiteral("Cy"), QStringLiteral("Lima")}};
            d->merge.include = {true, true, true};
            d->extra.push_back(ed->newTextBox(QRectF(0, 0, 120, 40), QStringLiteral("Left over")));
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
            w->editor()->painterItem = QJsonObject();   // (formatting copied in the window that was put back)
            w->editor()->painterHasText = false;
            QString error;
            if (fromFile) w->editor()->setDocument(loadPublication(m_app.sampleFile, &error), m_app.sampleFile);
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
        m_app.prepare = [this](AppWindow *aw, const QString &id, bool fromFile) {
            auto *w = asPub(aw);
            if (id == QLatin1String("edit.paste") || id == QLatin1String("edit.pasteText")) QGuiApplication::clipboard()->setText(QStringLiteral("Pasted words"));
            if (id == QLatin1String("file.save")) m_app.type(w);   // (Save has something to save once something was changed)
            if (id == QLatin1String("zoom.page") || id == QLatin1String("zoom.width")) w->canvas()->setZoom(0.9);
            if (id == QLatin1String("page.prev") || id == QLatin1String("page.moveUp")) {   // (the first page has no page before it)
                if (w->editor()->doc()->pages.size() < 2) w->editor()->doc()->addPage(1, w->editor()->doc()->pages[0]->masterId);
                w->editor()->setCurrentPage(1);
            }
            if (!fromFile || id == QLatin1String("edit.undo") || id == QLatin1String("edit.redo")) return;
            Editor *ed = w->editor();
            auto items = [&](ItemType t) {
                QStringList ids;
                for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
                    if (it->type() == t) ids << it->id;
                return ids;
            };
            auto firstOf = [&](ItemType t) -> QString { return items(t).value(0); };
            const QString cat = id == QLatin1String("ins.pageCount") ? id : id.section(QLatin1Char('.'), 0, 0);
            static const QSet<QString> pictures = {"pic", "corr", "recolor"}, textArt = {"wa", "waalign", "waspace"},
                                       shapes = {"shape", "fill", "line", "shadow", "obj", "wrap", "glow", "bevel", "refl", "rot3d", "soft"},
                                       lines = {"dash", "weight", "arrowStart", "arrowEnd"}, tables = {"tbl", "border"},
                                       words = {"fmt", "ins.pageCount", "para", "case", "dropcap", "style", "fit", "cols", "edit", "rev", "tb", "bullet", "number", "numstyle", "numspacing", "ls", "spacing", "lig", "ss", "valign", "tbmargin"};
            if (cat == QLatin1String("mp")) ed->setMasterView(ed->doc()->masters.first()->id);
            if (id == QLatin1String("zoom.selection")) ed->select(firstOf(ItemType::Shape));
            if (id == QLatin1String("pic.swap")) ed->select(items(ItemType::Picture).mid(0, 2));
            if (id == QLatin1String("pd.bgImage") || id == QLatin1String("pic.toShapes")) {
                // (the SVG drawing is the last picture)
                ed->select(items(ItemType::Picture).last());
            }
            if (cat == QLatin1String("arr")) {   // (aligning and spacing are for several objects)
                ed->select(items(ItemType::Shape).mid(0, 3));
            } else if (id == QLatin1String("pic.swap") || id == QLatin1String("pd.bgImage") || id == QLatin1String("pic.toShapes")) {
            } else if (tables.contains(cat)) {
                ed->select(firstOf(ItemType::Table));
                if (cat == QLatin1String("tbl")) ed->beginTextEdit(firstOf(ItemType::Table), 0, 0, 0);   // (the cursor in a cell)
            } else if (pictures.contains(cat)) ed->select(items(ItemType::Picture).value(0));
            else if (textArt.contains(cat)) ed->select(firstOf(ItemType::TextArt));
            else if (lines.contains(cat)) ed->select(firstOf(ItemType::Line));
            else if (shapes.contains(cat)) ed->select(firstOf(ItemType::Shape));
            else if (words.contains(cat)) {
                // (the box with the misspelled words)
                QString t;
                for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
                    if (it->type() == ItemType::Text)
                        if (QTextDocument *story = ed->doc()->storyDoc(static_cast<TextItem *>(it.get())->storyId); story && story->toPlainText().startsWith(QLatin1String("Teh quikc"))) t = it->id;
                ed->select(t);
                ed->beginTextEdit(t, 0);
                QTextCursor c = ed->cursor();
                c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
                ed->setCursor(c);
            }
        };
        // The places a person is in when a command is there to use. (Where a command only sets something that is set already, the others of its family are run first.)
        auto aBox = [](AppWindow *aw, bool fromFile, bool selected) {
            auto *w = asPub(aw);
            Editor *ed = w->editor();
            if (!fromFile) {
                startTextBox(w);
                typeInto(w->canvas(), QStringLiteral("Teh quikc words here"));
                ed->flushTyping();
            } else {
                QString t;
                for (const auto &it : ed->doc()->pages[0]->items)
                    if (it->type() == ItemType::Text)
                        if (QTextDocument *story = ed->doc()->storyDoc(static_cast<TextItem *>(it.get())->storyId); story && story->toPlainText().startsWith(QLatin1String("Teh quikc"))) t = it->id;
                ed->select(t);
                ed->beginTextEdit(t, 0);
            }
            if (selected) {
                QTextCursor c = ed->cursor();
                c.select(QTextCursor::Document);
                ed->setCursor(c);
            }
        };
        m_app.contexts = {
            {QStringLiteral("the cursor in words"), [aBox](AppWindow *w, bool f) { aBox(w, f, false); }, false},
            {QStringLiteral("words selected"), [aBox](AppWindow *w, bool f) { aBox(w, f, true); }, false},
            {QStringLiteral("a shape selected"), [](AppWindow *aw, bool f) {
                 Editor *ed = asPub(aw)->editor();
                 if (!f) {
                     auto shape = std::make_shared<ShapeItem>();
                     shape->rect = QRectF(100, 300, 120, 80);
                     ed->addItem(shape);
                 } else ed->select(ed->doc()->pages[0]->items.at(0)->id);
             }, false},
            {QStringLiteral("a shape over a text box"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 ed->addItem(ed->newTextBox(QRectF(60, 300, 240, 120), QStringLiteral("Some words that the shape sits over.")));
                 auto shape = std::make_shared<ShapeItem>();
                 shape->rect = QRectF(80, 320, 40, 40);
                 ed->addItem(shape);
             }, false},
            {QStringLiteral("three shapes selected"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 QStringList ids;
                 for (int i = 0; i < 3; ++i) {
                     auto shape = std::make_shared<ShapeItem>();
                     shape->rect = QRectF(60 + i * 140 + (i == 2 ? 30 : 0), 300 + (i == 2 ? 220 : i * 30), 120, 80);
                     ids << ed->addItem(shape);
                 }
                 ed->select(ids);
             }, false},
            {QStringLiteral("a line selected"), [](AppWindow *aw, bool) {
                 auto line = std::make_shared<LineItem>();
                 line->p1 = QPointF(80, 600);
                 line->p2 = QPointF(300, 640);
                 asPub(aw)->editor()->addItem(line);
             }, false},
            {QStringLiteral("a picture selected"), [](AppWindow *aw, bool) {
                 auto *w = asPub(aw);
                 w->insertFiles({tempFile(QStringLiteral("red.png"))}, QPointF(300, 300));
             }, false},
            {QStringLiteral("a table selected"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 const QString id = ed->addItem(ed->newTable(QRectF(72, 300, 300, 90), 3, 3));
                 ed->beginTextEdit(id, 0, 0, 0);
             }, false},
            {QStringLiteral("a recipient list chosen"), [](AppWindow *aw, bool) {
                 Document *d = asPub(aw)->editor()->doc();
                 if (d->merge.isEmpty()) {
                     d->merge.fields = {QStringLiteral("Name"), QStringLiteral("City")};
                     d->merge.rows = {{QStringLiteral("Ann"), QStringLiteral("Oslo")}, {QStringLiteral("Bo"), QStringLiteral("Rome")}};
                     d->merge.include = {true, true};
                 }
             }, false},
            {QStringLiteral("a table of contents out of date"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 asPub(aw)->act(QStringLiteral("ins.toc"))->trigger();
                 // (a heading is written after the table was made)
                 const QString id = ed->addItem(ed->newTextBox(QRectF(60, 600, 300, 40), QStringLiteral("Chapter one")));
                 QTextDocument *story = ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->item(id))->storyId);
                 QTextCursor c(story);
                 QTextBlockFormat bf = c.blockFormat();
                 bf.setProperty(tp::StyleName, QStringLiteral("Heading 1"));
                 c.setBlockFormat(bf);
                 ed->select(id);
             }, false},
            {QStringLiteral("two pictures selected"), [](AppWindow *aw, bool) {
                 auto *w = asPub(aw);
                 w->insertFiles({tempFile(QStringLiteral("red.png"))}, QPointF(300, 300));
                 w->insertFiles({tempFile(QStringLiteral("blue.png"))}, QPointF(420, 330));
                 QStringList ids;
                 for (const auto &it : w->editor()->doc()->pages[w->editor()->currentPage()]->items)
                     if (it->type() == ItemType::Picture) ids << it->id;
                 w->editor()->select(ids.mid(ids.size() - 2));
             }, false},
            {QStringLiteral("a cropped picture in a shape, with a tint"), [](AppWindow *aw, bool) {
                 auto *w = asPub(aw);
                 Editor *ed = w->editor();
                 w->insertFiles({tempFile(QStringLiteral("red.png"))}, QPointF(300, 300));
                 QString id;
                 for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
                     if (it->type() == ItemType::Picture) id = it->id;
                 ed->change(QStringLiteral("Picture"), [&] {
                     auto *p = static_cast<PictureItem *>(ed->doc()->item(id));
                     p->imgRect = p->imgRect.translated(-10, -6);
                     p->maskShape = QStringLiteral("ellipse");
                     p->brightness = 20;
                     p->recolor = PictureItem::Grayscale;
                 });
                 ed->select(id);
             }, false},
            {QStringLiteral("Text Art selected"), [](AppWindow *aw, bool) { asPub(aw)->editor()->addItem(std::make_shared<TextArtItem>()); }, false},
            {QStringLiteral("a whole table selected"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 ed->addItem(ed->newTable(QRectF(72, 300, 300, 90), 3, 3));
             }, false},
            {QStringLiteral("a table with uneven columns and rows"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 auto table = ed->newTable(QRectF(72, 300, 300, 90), 3, 3);
                 auto *t = static_cast<TableItem *>(table.get());
                 t->colW = {50, 100, 150};
                 t->rowH = {20, 30, 40};
                 const QString id = ed->addItem(table);
                 ed->beginTextEdit(id, 0, 0, 0);
             }, false},
            {QStringLiteral("a story in two linked boxes"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 const QString first = ed->addItem(ed->newTextBox(QRectF(60, 300, 150, 40), QStringLiteral("A story that runs on")));
                 const QString second = ed->addItem(ed->newTextBox(QRectF(60, 360, 150, 40)));
                 ed->linkFrames(first, second);
                 ed->select(first);
             }, false},
            {QStringLiteral("the second of two linked boxes"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 const QString first = ed->addItem(ed->newTextBox(QRectF(60, 300, 150, 40), QStringLiteral("A story that runs on")));
                 const QString second = ed->addItem(ed->newTextBox(QRectF(60, 360, 150, 40)));
                 ed->linkFrames(first, second);
                 ed->select(second);
             }, false},
            {QStringLiteral("pictures in the scratch area"), [](AppWindow *aw, bool) {
                 auto picture = std::make_shared<PictureItem>();
                 picture->rect = QRectF(700, 20, 300, 200);
                 asPub(aw)->editor()->doc()->scratch.push_back(picture);
             }, false},
            {QStringLiteral("two pages, the first with a background, and a style that sets a font"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 if (ed->doc()->pages.size() < 2) ed->doc()->addPage(1, ed->doc()->pages[0]->masterId);
                 ed->change(QStringLiteral("Background"), [ed] {
                     ed->doc()->pages[0]->background = Fill::solid(ColorRef::rgb(QColor(Qt::cyan)));
                     for (auto &st : ed->doc()->styles) st.chr.setFontFamilies({QStringLiteral("Arial")});
                 });
             }, false},
            {QStringLiteral("formatting copied from bold words"), [aBox](AppWindow *aw, bool f) {
                 auto *w = asPub(aw);
                 aBox(aw, f, true);
                 w->refreshUi();
                 w->act(QStringLiteral("fmt.bold"))->trigger();
                 w->refreshUi();
                 w->act(QStringLiteral("edit.copyFormat"))->trigger();
                 // (and other words, which have none of it)
                 Editor *ed = w->editor();
                 const QString other = ed->addItem(ed->newTextBox(QRectF(300, 500, 200, 40), QStringLiteral("Other words")));
                 ed->beginTextEdit(other, 0);
                 QTextCursor c = ed->cursor();
                 c.select(QTextCursor::Document);
                 ed->setCursor(c);
             }, false},
            {QStringLiteral("a change to undo"), [](AppWindow *aw, bool) {
                 auto shape = std::make_shared<ShapeItem>();
                 shape->rect = QRectF(100, 300, 120, 80);
                 asPub(aw)->editor()->addItem(shape);
             }, false},
            {QStringLiteral("a change undone"), [](AppWindow *aw, bool) {
                 auto shape = std::make_shared<ShapeItem>();
                 shape->rect = QRectF(100, 300, 120, 80);
                 asPub(aw)->editor()->addItem(shape);
                 asPub(aw)->editor()->undo();
             }, false},
            {QStringLiteral("two master pages"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 ed->addMaster(false);
                 ed->setMasterView(ed->doc()->masters.last()->id);
             }, false},
            {QStringLiteral("a page with a background"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 ed->change(QStringLiteral("Background"), [ed] {
                     for (auto &page : ed->doc()->pages) page->background = Fill::solid(ColorRef::rgb(QColor(Qt::yellow)));
                 });
             }, false},
            {QStringLiteral("a landscape page"), [](AppWindow *aw, bool) { asPub(aw)->act(QStringLiteral("pd.landscape"))->trigger(); }, false},
            {QStringLiteral("a line with arrows and dashes"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 const QString id = [&] {
                     for (const auto &it : ed->doc()->pages[0]->items)
                         if (it->type() == ItemType::Line) return it->id;
                     return QString();
                 }();
                 ed->change(QStringLiteral("Line"), [&] {
                     auto *l = static_cast<LineItem *>(ed->doc()->item(id));
                     l->stroke.dash = Stroke::DashLine;
                     l->stroke.endArrow = Arrow::Triangle;
                     l->stroke.startArrow = Arrow::Triangle;
                 });
                 ed->select(id);
             }, true},
            {QStringLiteral("a group selected"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 for (const auto &it : ed->doc()->pages[0]->items)
                     if (it->type() == ItemType::Group) ed->select(it->id);
             }, true},
            {QStringLiteral("objects just ungrouped"), [](AppWindow *aw, bool) {
                 Editor *ed = asPub(aw)->editor();
                 for (const auto &it : ed->doc()->pages[0]->items)
                     if (it->type() == ItemType::Group) {
                         ed->select(it->id);
                         break;
                     }
                 asPub(aw)->refreshUi();
                 asPub(aw)->act(QStringLiteral("arr.group"))->trigger();   // (Group is Ungroup while a group is selected)
             }, true},
            {QStringLiteral("a merge preview"), [](AppWindow *aw, bool) {
                 asPub(aw)->refreshUi();
                 asPub(aw)->act(QStringLiteral("mm.preview"))->trigger();
             }, true},
        };
        // (where a command is best tried first: the others come after, so the hints only make the run quicker)
        const QString aStyledPage = QStringLiteral("two pages, the first with a background, and a style that sets a font");
        m_app.hints = {
            {QStringLiteral("wa"), {QStringLiteral("Text Art selected")}},
            {QStringLiteral("pic."), {QStringLiteral("a picture selected"), QStringLiteral("two pictures selected"), QStringLiteral("a cropped picture in a shape, with a tint"), QStringLiteral("pictures in the scratch area")}},
            {QStringLiteral("corr"), {QStringLiteral("a cropped picture in a shape, with a tint")}},
            {QStringLiteral("recolor"), {QStringLiteral("a cropped picture in a shape, with a tint")}},
            {QStringLiteral("tbl."), {QStringLiteral("a table with uneven columns and rows"), QStringLiteral("a table selected")}},
            {QStringLiteral("border"), {QStringLiteral("a whole table selected")}},
            {QStringLiteral("mp."), {QStringLiteral("two master pages")}},
            {QStringLiteral("mm."), {QStringLiteral("a recipient list chosen"), QStringLiteral("a merge preview")}},
            {QStringLiteral("pd.bg"), {aStyledPage, QStringLiteral("a page with a background")}},
            {QStringLiteral("pd.portrait"), {QStringLiteral("a landscape page")}},
            {QStringLiteral("pd.updateFonts"), {aStyledPage}},
            {QStringLiteral("arr."), {QStringLiteral("three shapes selected"), QStringLiteral("a group selected"), QStringLiteral("objects just ungrouped")}},
            {QStringLiteral("page."), {aStyledPage}},
            {QStringLiteral("tb."), {QStringLiteral("a story in two linked boxes"), QStringLiteral("the second of two linked boxes")}},
            {QStringLiteral("edit.pasteFormat"), {QStringLiteral("formatting copied from bold words")}},
            {QStringLiteral("dash"), {QStringLiteral("a line with arrows and dashes"), QStringLiteral("a line selected")}},
            {QStringLiteral("arrow"), {QStringLiteral("a line with arrows and dashes"), QStringLiteral("a line selected")}},
            {QStringLiteral("ins.updateToc"), {QStringLiteral("a table of contents out of date")}},
        };
        m_app.skipped = {};
        m_app.quiet = {
            {QStringLiteral("rev.wordCount"), QStringLiteral("it tells the counts in a message and changes nothing")},
            {QStringLiteral("tb.textFit"), QStringLiteral("the name of a menu: its choices are the commands, and it has none of its own")},
            {QStringLiteral("arr.relMargins"), QStringLiteral("a setting that only the next Align reads: its check mark is all of it")},
            {QStringLiteral("win.cascade"), QStringLiteral("it places the open windows, and there is one")},
        };
        m_app.neverRun = {};
    }

    // ---- 1. the first five minutes
    // A new window, and one from File > New, take typing without a click first (a blank page has no text box until one is made: until then typing must do nothing).
    void typingNeedsNoClick()
    {
        const QStringList failed = checkTypingWithoutAClick(m_app);
        QVERIFY2(failed.isEmpty(), qPrintable(failed.join(QLatin1Char('\n'))));
    }

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

        // The print layout: the File screen's Print page, with the publication's page drawn (not a blank one).
        QVERIFY2(previewShowsTheDocument(m_app, again.get()), "the print preview shows no more than a blank page");
        backstage = again->findChild<Backstage *>();
        QVERIFY(backstage && backstage->isVisible());
        QCOMPARE(backstage->currentPage(), QStringLiteral("print"));
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
        for (bool fromFile : {false, true}) {
            std::unique_ptr<AppWindow> w(fromFile ? m_app.launchWith(m_app.sampleFile) : m_app.launch());
            QVERIFY(w);
            m_app.settle(w.get());
            const Outcome o = observe(m_app, w.get(), nullptr, watch, [] {}, false, 3000);
            QVERIFY2(!o.changed, qPrintable(o.how + o.detail));
        }
    }

    // Redo answers to Ctrl+Y (as Publisher's does) and to the system's own key for it (Cmd+Shift+Z on a Mac).
    void redoHasBothKeys()
    {
        std::unique_ptr<AppWindow> w(m_app.launch());
        const QList<QKeySequence> keys = w->act(QStringLiteral("edit.redo"))->shortcuts();
        QVERIFY(keys.contains(QKeySequence(Qt::CTRL | Qt::Key_Y)));
        QVERIFY2(keys.contains(QKeySequence(QKeySequence::Redo)), "the system's key for Redo does not work");
    }

    // Shift+F3 changes the words each time it is pressed, from what they are now: lower case, UPPER CASE, then Sentence case; what was done in another window does not decide it.
    void changeCaseGoesRoundFromWhatTheWordsAre()
    {
        for (int round = 0; round < 2; ++round) {   // (the second window begins as the first did, whatever the first went through)
            std::unique_ptr<MainWindow> w(static_cast<MainWindow *>(m_app.launch()));
            startTextBox(w.get());
            typeInto(w->canvas(), QStringLiteral("Hello world"));
            w->editor()->flushTyping();
            auto select = [&] {
                QTextCursor c = w->editor()->cursor();
                c.select(QTextCursor::Document);
                w->editor()->setCursor(c);
            };
            select();
            w->act(QStringLiteral("case.cycle"))->trigger();
            QCOMPARE(allText(w.get()), QStringLiteral("hello world|"));
            select();
            w->act(QStringLiteral("case.cycle"))->trigger();
            QCOMPARE(allText(w.get()), QStringLiteral("HELLO WORLD|"));
            select();
            w->act(QStringLiteral("case.cycle"))->trigger();
            QCOMPARE(allText(w.get()), QStringLiteral("Hello world|"));
        }
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
    // The commands that no test could enable in either state are named, each with the reason, and no more of them than the list says.
    void commandsNeverRun()
    {
        QStringList report;
        const QStringList fails = checkNeverRun(m_app, &report);
        qInfo("%s", qPrintable(report.first()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }
};

JP_TEST_CLASS(PubAppChecks)
#include "tst_appchecks_pub.moc"
