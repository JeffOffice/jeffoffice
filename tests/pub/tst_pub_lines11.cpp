// JeffPub's tests of the commands that closed lines of the feature list in slice 11: Equalize on Arrange, Duplicate Several Times, and Send to Master Page.
#include "pubhelpers.h"
#include "../withdialog.h"
#include "text/mergeformat.h"
#include "app/widgets.h"
#include "render/renderer.h"
#include "render/shapes.h"
#include "app/formatpresets.h"
#include "text/textprops.h"
#include "core/fonts.h"
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>

namespace {

QPoint viewOf(jp::MainWindow &w, QPointF page) { return w.canvas()->pageToView(page).toPoint(); }

void drag(jp::MainWindow &w, QPointF from, QPointF to)
{
    QWidget *vp = w.canvas()->viewport();
    QTest::qWait(QApplication::doubleClickInterval() + 50);   // (so that two draws in a row are not taken for a double click)
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, viewOf(w, from));
    for (const QPointF at : {(from + to) / 2, to}) {
        const QPoint v = viewOf(w, at);
        QMouseEvent mv(QEvent::MouseMove, v, vp->mapToGlobal(v), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(vp, &mv);
    }
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, viewOf(w, to));
}

jp::TextItem *textBoxOf(jp::Editor *ed)
{
    for (const jp::ItemPtr &it : ed->surfaceItems())
        if (it->type() == jp::ItemType::Text && it->role.isEmpty()) return static_cast<jp::TextItem *>(it.get());
    return nullptr;
}

} // namespace

class PubLines11Tests : public QObject {
    Q_OBJECT

    static QString addShape(jp::Editor *ed, const QRectF &r, const QColor &color = Qt::red)
    {
        auto s = std::make_shared<jp::ShapeItem>();
        s->rect = r;
        s->fill = jp::Fill::solid(jp::ColorRef::rgb(color));
        s->stroke = jp::Stroke::none();
        ed->doc()->pages[0]->items.push_back(s);
        return s->id;
    }
    static QRectF rectOf(jp::Editor *ed, const QString &id)
    {
        const jp::Item *it = ed->doc()->item(id);
        return it ? it->rect : QRectF();
    }
    static QColor pixel(jp::Editor *ed, int x, int y)
    {
        jp::PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &ed->cache();
        return jp::Renderer::renderToImage(ctx, 0, 1.0).pixelColor(x, y);
    }

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // Arrange > Align > Equalize Width, Equalize Height, and Equalize Width and Height give every other selected object the size of the one selected first, each keeping its top
    // left corner; each is one step of Undo and Redo, and the sizes are kept in a saved file that the app opens again.
    void equalizeGivesTheSelectedObjectsTheSizeOfTheFirstOne()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(50, 50, 100, 60)), b = addShape(ed, QRectF(200, 70, 40, 90)), c = addShape(ed, QRectF(300, 300, 70, 30));
        ed->select(a);
        QTRY_VERIFY(!w.act(QStringLiteral("arr.eqW"))->isEnabled());   // (one object has nothing to match)
        ed->select(QStringList{a, b, c});
        QTRY_VERIFY(w.act(QStringLiteral("arr.eqW"))->isEnabled());
        w.act(QStringLiteral("arr.eqW"))->trigger();
        QCOMPARE(rectOf(ed, a), QRectF(50, 50, 100, 60));
        QCOMPARE(rectOf(ed, b), QRectF(200, 70, 100, 90));
        QCOMPARE(rectOf(ed, c), QRectF(300, 300, 100, 30));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Equalize Width"));
        ed->undo();
        QCOMPARE(rectOf(ed, b), QRectF(200, 70, 40, 90));
        QCOMPARE(rectOf(ed, c), QRectF(300, 300, 70, 30));
        ed->redo();
        QCOMPARE(rectOf(ed, b), QRectF(200, 70, 100, 90));
        w.act(QStringLiteral("arr.eqH"))->trigger();
        QCOMPARE(rectOf(ed, b), QRectF(200, 70, 100, 60));
        QCOMPARE(rectOf(ed, c), QRectF(300, 300, 100, 60));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Equalize Height"));
        ed->undo();
        QCOMPARE(rectOf(ed, c), QRectF(300, 300, 100, 30));
        // Both at once, from a different first object.
        ed->select(QStringList{c, a, b});
        w.act(QStringLiteral("arr.eqBoth"))->trigger();
        QCOMPARE(rectOf(ed, a), QRectF(50, 50, 100, 30));
        QCOMPARE(rectOf(ed, b), QRectF(200, 70, 100, 30));
        QCOMPARE(rectOf(ed, c), QRectF(300, 300, 100, 30));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Equalize Size"));
        // A locked object keeps its size.
        ed->undo();
        ed->doc()->item(b)->locked = true;
        ed->select(QStringList{a, b, c});
        w.act(QStringLiteral("arr.eqBoth"))->trigger();
        QCOMPARE(rectOf(ed, b), QRectF(200, 70, 100, 90));
        QCOMPARE(rectOf(ed, c), QRectF(300, 300, 100, 60));
        // Saved through the window and opened again, the sizes stand.
        const QString path = dir.filePath(QStringLiteral("equal.jpub"));
        QVERIFY(w.saveTo(path));
        jp::MainWindow again;
        QVERIFY(again.openFile(path));
        QCOMPARE(rectOf(again.editor(), a), QRectF(50, 50, 100, 60));
        QCOMPARE(rectOf(again.editor(), b), QRectF(200, 70, 100, 90));
        QCOMPARE(rectOf(again.editor(), c), QRectF(300, 300, 100, 60));
    }

    // Duplicate Several Times: a dialog for the number of copies, the offset of each from the one before, a size change for each, and a fill color that steps from the object's
    // own to the chosen one at the last copy; all copies are one step of Undo, and they are kept in a saved file.
    void duplicateSeveralTimesStepsOffsetSizeAndColor()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(50, 50, 100, 60), QColor(255, 0, 0));
        ed->select(a);
        QTRY_VERIFY(w.act(QStringLiteral("edit.duplicateMany"))->isEnabled());
        withDialog([&] { w.act(QStringLiteral("edit.duplicateMany"))->trigger(); }, QStringLiteral("duplicateDialog"), [&](QDialog *d) {
            QCOMPARE(d->findChild<QSpinBox *>(QStringLiteral("duplicateCopies"))->value(), 3);
            d->findChild<QSpinBox *>(QStringLiteral("duplicateCopies"))->setValue(3);
            d->findChild<jp::MeasureSpin *>(QStringLiteral("duplicateOffsetX"))->setValue(20);
            d->findChild<jp::MeasureSpin *>(QStringLiteral("duplicateOffsetY"))->setValue(10);
            d->findChild<QDoubleSpinBox *>(QStringLiteral("duplicateGrow"))->setValue(10);
            auto *step = d->findChild<QCheckBox *>(QStringLiteral("duplicateStepColor"));
            auto *last = d->findChild<jp::ColorButton *>(QStringLiteral("duplicateLastColor"));
            QVERIFY(!last->isEnabled());
            step->setChecked(true);
            QVERIFY(last->isEnabled());
            last->setCurrent(jp::ColorRef::rgb(QColor(0, 0, 255)));
        });
        QCOMPARE(ed->surfaceItems().size(), 4);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Duplicate Several Times"));
        QCOMPARE(ed->selection().size(), 1);
        const jp::ColorScheme scheme = ed->doc()->colors;
        auto copy = [&](int k) { return ed->surfaceItems()[k].get(); };
        for (int k = 1; k <= 3; ++k) {
            const double f = std::pow(1.1, k);
            const QRectF r = copy(k)->rect;
            QVERIFY(qAbs(r.x() - (50 + 20 * k)) < 1e-6);
            QVERIFY(qAbs(r.y() - (50 + 10 * k)) < 1e-6);
            QVERIFY(qAbs(r.width() - 100 * f) < 1e-6);
            QVERIFY(qAbs(r.height() - 60 * f) < 1e-6);
            const QColor shown = copy(k)->fill.color.resolve(scheme);
            QCOMPARE(shown, jp::mix(QColor(255, 0, 0), QColor(0, 0, 255), k / 3.0));
        }
        QCOMPARE(copy(3)->fill.color.resolve(scheme), QColor(0, 0, 255));
        QCOMPARE(copy(0)->fill.color.resolve(scheme), QColor(255, 0, 0));
        QCOMPARE(ed->selection(), QStringList{copy(3)->id});
        ed->undo();
        QCOMPARE(ed->surfaceItems().size(), 1);
        ed->redo();
        QCOMPARE(ed->surfaceItems().size(), 4);
        // Cancel makes nothing, and the one-copy Duplicate still moves by 12 points.
        withDialog([&] { w.act(QStringLiteral("edit.duplicateMany"))->trigger(); }, QStringLiteral("duplicateDialog"), [&](QDialog *) {}, false);
        QCOMPARE(ed->surfaceItems().size(), 4);
        const QString path = dir.filePath(QStringLiteral("steps.jpub"));
        QVERIFY(w.saveTo(path));
        jp::MainWindow again;
        QVERIFY(again.openFile(path));
        const jp::ItemList items = again.editor()->surfaceItems();
        QCOMPARE(items.size(), 4);
        QCOMPARE(items[3]->fill.color.resolve(again.editor()->doc()->colors), QColor(0, 0, 255));
        QVERIFY(qAbs(items[2]->rect.width() - 100 * 1.21) < 1e-6);
        // The plain Duplicate is as it was.
        ed->select(a);
        w.act(QStringLiteral("edit.duplicate"))->trigger();
        QCOMPARE(ed->surfaceItems().size(), 5);
        QCOMPARE(ed->surfaceItems()[4]->rect, QRectF(62, 62, 100, 60));
    }

    // Send to Master Page moves the selected objects from the page onto the master page the page uses, where they look the same; from the master page's view the same
    // command (then named Send to Page) brings them back onto the page. One step of Undo each way, kept in a saved file.
    void sendToMasterPageMovesObjectsOntoTheMasterAndBack()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(250, 100, 100, 100), Qt::green);
        jp::MasterPage *master = ed->doc()->masterFor(*ed->doc()->pages[0]);
        QVERIFY(master);
        const QString masterId = master->id;
        QCOMPARE(pixel(ed, 300, 150), QColor(Qt::green));
        ed->select(a);
        QTRY_VERIFY(w.act(QStringLiteral("arr.sendMaster"))->isEnabled());
        QCOMPARE(w.act(QStringLiteral("arr.sendMaster"))->text(), QStringLiteral("Send to Master Page"));
        w.act(QStringLiteral("arr.sendMaster"))->trigger();
        QVERIFY(ed->doc()->pages[0]->items.empty());
        QCOMPARE(int(ed->doc()->master(masterId)->items.size()), 1);
        QCOMPARE(ed->doc()->master(masterId)->items[0]->rect, QRectF(250, 100, 100, 100));
        QCOMPARE(pixel(ed, 300, 150), QColor(Qt::green));   // (drawn from the master, in the same place)
        QVERIFY(ed->selection().isEmpty());
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Send to Master Page"));
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), 1);
        QVERIFY(ed->doc()->master(masterId)->items.empty());
        ed->redo();
        QVERIFY(ed->doc()->pages[0]->items.empty());
        // Saved through the window and opened again.
        const QString path = dir.filePath(QStringLiteral("master.jpub"));
        QVERIFY(w.saveTo(path));
        {
            jp::MainWindow again;
            QVERIFY(again.openFile(path));
            jp::Document *doc = again.editor()->doc();
            QVERIFY(doc->pages[0]->items.empty());
            QCOMPARE(int(doc->masters[0]->items.size()), 1);
        }
        // Back: from the master page's view, onto the page being shown.
        ed->setMasterView(masterId);
        ed->select(ed->doc()->master(masterId)->items[0]->id);
        QTRY_COMPARE(w.act(QStringLiteral("arr.sendMaster"))->text(), QStringLiteral("Send to Page"));
        QCOMPARE(int(ed->selection().size()), 1);
        QTRY_VERIFY(w.act(QStringLiteral("arr.sendMaster"))->isEnabled());
        w.act(QStringLiteral("arr.sendMaster"))->trigger();
        QVERIFY(ed->doc()->master(masterId)->items.empty());
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Send to Page"));
        ed->undo();
        QCOMPARE(int(ed->doc()->master(masterId)->items.size()), 1);
        ed->redo();
        ed->setMasterView(QString());
        QCOMPARE(pixel(ed, 300, 150), QColor(Qt::green));
    }

    // A two-page master's right half shows on a right-hand page: an object sent there keeps its place on the sheet, and sent back it lands on the page.
    void sendToMasterPageOnATwoPageMasterKeepsThePlace()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(250, 100, 100, 100), Qt::green);
        jp::MasterPage *master = ed->doc()->masterFor(*ed->doc()->pages[0]);
        master->twoPage = true;
        ed->select(a);
        QVERIFY(ed->sendToMaster());
        QCOMPARE(master->items[0]->rect, QRectF(250 + 612, 100, 100, 100));
        QCOMPARE(pixel(ed, 300, 150), QColor(Qt::green));
        ed->setMasterView(master->id);
        ed->select(master->items[0]->id);
        QVERIFY(ed->sendToPage());
        QCOMPARE(ed->doc()->pages[0]->items[0]->rect, QRectF(250, 100, 100, 100));
    }

    // Nothing is moved for a page without a master page, a locked object, or a text box linked to another; the reason is given.
    void sendToMasterPageRefusesWhatItCannotMove()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(250, 100, 100, 100)), b = addShape(ed, QRectF(10, 10, 50, 50));
        QString why;
        ed->select(a);
        ed->doc()->item(a)->locked = true;
        QVERIFY(!ed->sendToMaster(&why));
        QVERIFY(!why.isEmpty());
        ed->doc()->item(a)->locked = false;
        auto t1 = std::make_shared<jp::TextItem>(), t2 = std::make_shared<jp::TextItem>();
        t1->rect = QRectF(100, 400, 100, 100);
        t2->rect = QRectF(300, 400, 100, 100);
        t1->nextId = t2->id;
        ed->doc()->pages[0]->items.push_back(t1);
        ed->doc()->pages[0]->items.push_back(t2);
        ed->select(t2->id);
        why.clear();
        QVERIFY(!ed->sendToMaster(&why));
        QVERIFY(!why.isEmpty());
        ed->select(b);
        ed->doc()->pages[0]->masterId.clear();
        why.clear();
        QVERIFY(!ed->sendToMaster(&why));
        QVERIFY(!why.isEmpty());
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), 4);
    }

    // Insert > Text > Draw Vertical Text Box draws a text box by the rule every text box has, with its words turned a quarter from the start (Text Direction on the Text Box tab shows it set);
    // a click puts in the usual box on its side; one step of Undo; the next Draw Text Box is across again; the direction is kept in a saved file.
    void drawVerticalTextBoxDrawsABoxWhoseWordsAreTurned()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QVERIFY(w.act(QStringLiteral("ins.vtextbox")));
        w.act(QStringLiteral("ins.vtextbox"))->trigger();
        QCOMPARE(ed->tool(), jp::Tool::Text);
        const int steps = ed->undoStack()->count();
        drag(w, QPointF(100, 100), QPointF(160, 300));
        ed->endTextEdit();
        jp::TextItem *box = textBoxOf(ed);
        QVERIFY(box);
        QVERIFY(box->vertical);
        QVERIFY(!box->turnedBack);
        QVERIFY2(std::abs(box->rect.width() - 60) < 8 && std::abs(box->rect.height() - 200) < 8, qPrintable(QStringLiteral("%1x%2").arg(box->rect.width()).arg(box->rect.height())));
        QCOMPARE(ed->tool(), jp::Tool::Select);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        const QString path = dir.filePath(QStringLiteral("vertical.jpub"));
        QVERIFY(w.saveTo(path));
        {
            jp::MainWindow again;
            QVERIFY(again.openFile(path));
            jp::TextItem *kept = textBoxOf(again.editor());
            QVERIFY(kept);
            QVERIFY(kept->vertical);
        }
        ed->undoStack()->undo();
        QVERIFY(!textBoxOf(ed));
        // A click puts in the usual box on its side.
        w.act(QStringLiteral("ins.vtextbox"))->trigger();
        drag(w, QPointF(100, 100), QPointF(101, 101));
        ed->endTextEdit();
        box = textBoxOf(ed);
        QVERIFY(box);
        QVERIFY(box->vertical);
        QVERIFY2(std::abs(box->rect.width() - 72) < 3 && std::abs(box->rect.height() - 144) < 3, qPrintable(QStringLiteral("%1x%2").arg(box->rect.width()).arg(box->rect.height())));
        ed->undoStack()->undo();
        // Draw Text Box is across, as before.
        w.act(QStringLiteral("ins.textbox"))->trigger();
        drag(w, QPointF(100, 100), QPointF(300, 220));
        ed->endTextEdit();
        box = textBoxOf(ed);
        QVERIFY(box);
        QVERIFY(!box->vertical);
    }

    // Home > Effects > Reflection puts a reflection under the letters (Tight, Half, or Full), drawn by the text engine; No Reflection takes it off; each is one step of Undo, and it is kept in a saved file.
    void reflectionOnLettersIsOnTheRibbonAndDrawn()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::initCore();
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200), QStringLiteral("Reflect")));
        ed->addItem(box);
        ed->select(box->id);
        auto effects = [&](jp::Editor *e, const jp::TextItem *t) {
            const QTextCharFormat cf = e->doc()->storyDoc(t->storyId)->firstBlock().begin().fragment().charFormat();
            const QString list = cf.stringProperty(jp::tp::TextFx);
            return list.isEmpty() ? jp::TextEffects() : jp::TextEffects::fromJson(list);
        };
        QVERIFY(!effects(ed, box.get()).reflection.on);
        const QImage before = [&] {
            jp::PaintContext ctx;
            ctx.doc = ed->doc();
            ctx.cache = &ed->cache();
            return jp::Renderer::renderToImage(ctx, 0, 1.0);
        }();
        QVERIFY(w.act(QStringLiteral("tb.refl.none")));
        for (int i = 0; i < jp::formatpresets::reflectionCount(); ++i) QVERIFY(w.act(QStringLiteral("tb.refl.%1").arg(i)));
        QCOMPARE(w.act(QStringLiteral("tb.refl.1"))->text(), jp::formatpresets::reflectionName(1));
        w.act(QStringLiteral("tb.refl.1"))->trigger();
        QVERIFY(effects(ed, box.get()).reflection.on);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Text Reflection"));
        jp::PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &ed->cache();
        const QImage after = jp::Renderer::renderToImage(ctx, 0, 1.0);
        int different = 0;
        for (int y = 0; y < before.height(); ++y)
            for (int x = 0; x < before.width(); ++x) different += before.pixel(x, y) != after.pixel(x, y);
        QVERIFY2(different > 50, qPrintable(QString::number(different)));
        const QString path = dir.filePath(QStringLiteral("reflect.jpub"));
        QVERIFY(w.saveTo(path));
        {
            jp::MainWindow again;
            QVERIFY(again.openFile(path));
            jp::TextItem *kept = textBoxOf(again.editor());
            QVERIFY(kept);
            QVERIFY(effects(again.editor(), kept).reflection.on);
        }
        ed->undo();
        QVERIFY(!effects(ed, box.get()).reflection.on);
        ed->redo();
        QVERIFY(effects(ed, box.get()).reflection.on);
        w.act(QStringLiteral("tb.refl.none"))->trigger();
        QVERIFY(!effects(ed, box.get()).reflection.on);
        ed->undo();
        QVERIFY(effects(ed, box.get()).reflection.on);
    }

    // Format Object > Size has a corner radius for a rounded rectangle and a point to resize from: with the center chosen, a new width and height keep the center where it was; the radius
    // is in points (held to half the shorter side), is drawn (a corner pixel that was filled is not), and both are one step of Undo, kept in a saved file.
    void positionAndSizeHasACornerRadiusAndABasePoint()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        auto s = std::make_shared<jp::ShapeItem>();
        s->shape = QStringLiteral("roundRect");
        s->rect = QRectF(200, 200, 100, 100);
        s->fill = jp::Fill::solid(jp::ColorRef::rgb(Qt::green));
        s->stroke = jp::Stroke::none();
        ed->doc()->pages[0]->items.push_back(s);
        const QString id = s->id;
        ed->select(id);
        QTRY_VERIFY(w.act(QStringLiteral("obj.format"))->isEnabled());
        QVERIFY(jp::hasCornerRadius(QStringLiteral("roundRect")));
        QVERIFY(!jp::hasCornerRadius(QStringLiteral("ellipse")));
        QVERIFY(qAbs(jp::cornerRadius(QStringLiteral("roundRect"), {}, QSizeF(100, 100)) - 100.0 / 6) < 0.01);
        withDialog([&] { w.act(QStringLiteral("obj.format"))->trigger(); }, QStringLiteral("formatObjectDialog"), [&](QDialog *d) {
            auto *radius = d->findChild<jp::MeasureSpin *>(QStringLiteral("cornerRadius"));
            QVERIFY(radius);
            QVERIFY(qAbs(radius->value() - 100.0 / 6) < 0.05);
            radius->setValue(30);
            auto *base = d->findChild<QComboBox *>(QStringLiteral("basePoint"));
            QVERIFY(base);
            QCOMPARE(base->currentIndex(), 0);
            base->setCurrentIndex(4);   // the center
            const auto spins = d->findChildren<jp::MeasureSpin *>();
            // (the height and the width are the two other boxes of the Size page that hold 100)
            int set = 0;
            for (jp::MeasureSpin *m : spins)
                if (m != radius && qAbs(m->value() - 100) < 0.01 && m->parentWidget() == radius->parentWidget()) {
                    m->setValue(60);
                    ++set;
                }
            QCOMPARE(set, 2);
        });
        const QRectF now = ed->doc()->item(id)->rect;
        QVERIFY2(qAbs(now.width() - 60) < 0.01 && qAbs(now.height() - 60) < 0.01, qPrintable(QString::number(now.width())));
        QVERIFY2(qAbs(now.center().x() - 250) < 0.01 && qAbs(now.center().y() - 250) < 0.01, qPrintable(QString::number(now.center().x())));
        auto *shape = static_cast<jp::ShapeItem *>(ed->doc()->item(id));
        QVERIFY(qAbs(jp::cornerRadius(shape->shape, shape->adj, shape->rect.size()) - 30) < 0.01);   // (30 points of a 60 point frame: the most there is)
        ed->undo();
        QCOMPARE(ed->doc()->item(id)->rect, QRectF(200, 200, 100, 100));
        ed->redo();
        QVERIFY(qAbs(ed->doc()->item(id)->rect.width() - 60) < 0.01);
        const QString path = dir.filePath(QStringLiteral("radius.jpub"));
        QVERIFY(w.saveTo(path));
        jp::MainWindow again;
        QVERIFY(again.openFile(path));
        auto *kept = static_cast<jp::ShapeItem *>(again.editor()->doc()->item(id));
        QVERIFY(kept);
        QVERIFY(qAbs(jp::cornerRadius(kept->shape, kept->adj, kept->rect.size()) - 30) < 0.01);
    }

    // The formats a merge field can show its value in: 1234.5 as $1,234.50, and the number, percent, date, and time ones; a value that is not a number or a date shows as it is, and a name with
    // a bar in it is still a name.
    void mergeFormatsShowNumbersCurrencyDatesAndTimes()
    {
        using jp::formatMergeValue;
        QCOMPARE(formatMergeValue(QStringLiteral("1234.5"), QStringLiteral("currency")), QStringLiteral("$1,234.50"));
        QCOMPARE(formatMergeValue(QStringLiteral("$1,234.5"), QStringLiteral("currency:0")), QStringLiteral("$1,235"));
        QCOMPARE(formatMergeValue(QStringLiteral("1234,5"), QStringLiteral("number:1")), QStringLiteral("1,234.5"));
        QCOMPARE(formatMergeValue(QStringLiteral("1.234,56"), QStringLiteral("number:0")), QStringLiteral("1.234,56"));   // (in US English that is no number)
        QCOMPARE(formatMergeValue(QStringLiteral("0.256"), QStringLiteral("percent:1")), QStringLiteral("25.6%"));
        QCOMPARE(formatMergeValue(QStringLiteral("2026-11-20"), QStringLiteral("date:long")), QStringLiteral("Friday, November 20, 2026"));
        QCOMPARE(formatMergeValue(QStringLiteral("11/20/2026"), QStringLiteral("date:iso")), QStringLiteral("2026-11-20"));
        QCOMPARE(formatMergeValue(QStringLiteral("2026-11-20"), QStringLiteral("date:monthday")), QStringLiteral("November 20"));
        QCOMPARE(formatMergeValue(QStringLiteral("2026-11-20"), QStringLiteral("date:monthyear")), QStringLiteral("November 2026"));
        QCOMPARE(formatMergeValue(QStringLiteral("15:45"), QStringLiteral("time:12")), QStringLiteral("3:45 PM"));
        QCOMPARE(formatMergeValue(QStringLiteral("3:45 PM"), QStringLiteral("time:24")), QStringLiteral("15:45"));
        // What can't be read, and what is empty, is as it is; so is a format the program does not know.
        QCOMPARE(formatMergeValue(QStringLiteral("a lot"), QStringLiteral("currency")), QStringLiteral("a lot"));
        QCOMPARE(formatMergeValue(QStringLiteral("soon"), QStringLiteral("date:long")), QStringLiteral("soon"));
        QCOMPARE(formatMergeValue(QString(), QStringLiteral("currency")), QString());
        QCOMPARE(formatMergeValue(QStringLiteral("1234.5"), QStringLiteral("number:9")), QStringLiteral("1234.5"));
        QCOMPARE(formatMergeValue(QStringLiteral("1234.5"), QString()), QStringLiteral("1234.5"));
        const jp::MergeFieldCode priced = jp::splitMergeField(QStringLiteral("Price|currency"));
        QCOMPARE(priced.name, QStringLiteral("Price"));
        QCOMPARE(priced.format, QStringLiteral("currency"));
        QCOMPARE(jp::splitMergeField(QStringLiteral("A|B")).name, QStringLiteral("A|B"));
        QCOMPARE(jp::splitMergeField(QStringLiteral("A|B|number:2")).name, QStringLiteral("A|B"));
        QCOMPARE(jp::joinMergeField(QStringLiteral("Price"), QString()), QStringLiteral("Price"));
    }

    // What the formats read and show follows the language of the text at the field, never the machine's: English (US) month first, English (UK) and German day first, German's signs for
    // numbers on input and output; two-digit years, signs, parentheses, and the values that are no number or no date of the kind asked for.
    void mergeFormatsFollowTheDocumentsLanguageAndNeverInventData()
    {
        using jp::formatMergeValue;
        auto f = [](const char *value, const char *format, const char *language = "") { return formatMergeValue(QString::fromLatin1(value), QString::fromLatin1(format), QString::fromLatin1(language)); };
        QCOMPARE(f("03/04/2026", "date:iso"), QStringLiteral("2026-03-04"));
        QCOMPARE(f("03/04/2026", "date:iso", "en-US"), QStringLiteral("2026-03-04"));
        QCOMPARE(f("03/04/2026", "date:iso", "en-GB"), QStringLiteral("2026-04-03"));
        QCOMPARE(f("03.04.2026", "date:iso", "de-DE"), QStringLiteral("2026-04-03"));
        QCOMPARE(f("1.234,5", "number:1", "de-DE"), QStringLiteral("1.234,5"));
        QCOMPARE(f("1234,5", "number:2", "de-DE"), QStringLiteral("1.234,50"));
        QCOMPARE(f("1234.5", "number:2", "de-DE"), QStringLiteral("1.234,50"));   // (a list kept in the plain form)
        QVERIFY(f("1234.5", "currency", "de-DE").startsWith(QStringLiteral("1.234,50")));
        QVERIFY(f("1234.5", "currency", "de-DE").endsWith(QStringLiteral("€")));
        QCOMPARE(f("1234.5", "currency", "en-GB"), QStringLiteral("£1,234.50"));
        QCOMPARE(f("1234.5", "currency"), QStringLiteral("$1,234.50"));
        // Two digits for a year: 00 to 29 are the 2000s, 30 to 99 the 1900s.
        QCOMPARE(f("1/2/00", "date:iso"), QStringLiteral("2000-01-02"));
        QCOMPARE(f("1/2/29", "date:iso"), QStringLiteral("2029-01-02"));
        QCOMPARE(f("1/2/30", "date:iso"), QStringLiteral("1930-01-02"));
        QCOMPARE(f("1/2/99", "date:iso"), QStringLiteral("1999-01-02"));
        // No data is made up: a time with a date format, and a date with a time format, are left as the list has them.
        QCOMPARE(f("15:45", "date:long"), QStringLiteral("15:45"));
        QCOMPARE(f("2026-11-20", "time:24"), QStringLiteral("2026-11-20"));
        QCOMPARE(f("2026-11-20 15:45", "time:24"), QStringLiteral("15:45"));
        QCOMPARE(f("2026-11-20 15:45", "date:iso"), QStringLiteral("2026-11-20"));
        // What is no number: nan and inf in any case (a recipient named Nan), exponents, and what is too big.
        for (const char *word : {"nan", "NaN", "Nan", "inf", "INF", "-inf", "Infinity", "1e308", "1E5", "0x10"}) QCOMPARE(f(word, "number:1"), QString::fromLatin1(word));
        const QString huge = QString(40, QLatin1Char('9'));
        QCOMPARE(formatMergeValue(huge, QStringLiteral("currency")), huge);
        QCOMPARE(formatMergeValue(QString(30, QLatin1Char('9')), QStringLiteral("number:0")), QString(30, QLatin1Char('9')));
        // Signs, parentheses, and zero.
        QCOMPARE(f("(500)", "number:0"), QStringLiteral("-500"));
        QVERIFY(f("(500)", "currency").contains(QStringLiteral("500.00")) && (f("(500)", "currency").startsWith(QLatin1Char('-')) || f("(500)", "currency").startsWith(QLatin1Char('('))));
        QCOMPARE(f("-12.5", "number:1"), QStringLiteral("-12.5"));
        QCOMPARE(f("0", "currency"), QStringLiteral("$0.00"));
        QCOMPARE(f("$0.00", "number:0"), QStringLiteral("0"));
    }

    // Mailings > Format Merge Field sets how the field at the caret shows its value; the merged page shows it that way ($1,234.50 for 1234.5), the field's name is the same, and the
    // choice is one step of Undo and is kept in a saved file that the app opens again.
    void formatMergeFieldChangesWhatTheMergedPageShows()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::initCore();
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        ed->doc()->merge.fields = {QStringLiteral("Price")};
        ed->doc()->merge.rows = {{QStringLiteral("1234.5")}};
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 100)));
        {
            QTextCursor c(ed->doc()->storyDoc(box->storyId));
            QTextCharFormat cf;
            cf.setProperty(jp::tp::Field, QStringLiteral("merge:Price"));
            c.insertText(QString(QChar::ObjectReplacementCharacter), cf);
        }
        ed->addItem(box);
        auto fieldCode = [&](jp::Editor *e, const jp::TextItem *t) {
            return e->doc()->storyDoc(t->storyId)->firstBlock().begin().fragment().charFormat().stringProperty(jp::tp::Field);
        };
        auto shown = [&](jp::Document *d) {
            jp::LayoutCache cache;
            jp::PaintContext ctx;
            ctx.doc = d;
            ctx.cache = &cache;
            ctx.opt.output = true;
            ctx.opt.mergeRecord = 0;
            return jp::Renderer::renderToImage(ctx, 0, 1.0);
        };
        QVERIFY(!w.act(QStringLiteral("mm.formatField"))->isEnabled());
        ed->beginTextEdit(box->id, 0);
        QTRY_VERIFY(w.act(QStringLiteral("mm.formatField"))->isEnabled());
        const QImage plainImage = shown(ed->doc());
        withDialog([&] { w.act(QStringLiteral("mm.formatField"))->trigger(); }, QStringLiteral("mergeFormatDialog"), [&](QDialog *d) {
            auto *kind = d->findChild<QComboBox *>(QStringLiteral("mergeFormatKind"));
            QCOMPARE(kind->currentIndex(), 0);
            kind->setCurrentIndex(2);   // Currency
            QCOMPARE(d->findChild<QLabel *>(QStringLiteral("mergeFormatSample"))->text(), QStringLiteral("$1,234.50"));
        });
        ed->endTextEdit();
        QCOMPARE(fieldCode(ed, box.get()), QStringLiteral("merge:Price|currency"));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Format Merge Field"));
        const QImage currencyImage = shown(ed->doc());
        QVERIFY(currencyImage != plainImage);
        // The page shows what the text "$1,234.50" shows.
        {
            jp::MainWindow second;
            second.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
            auto text = std::static_pointer_cast<jp::TextItem>(second.editor()->newTextBox(QRectF(72, 72, 400, 100), QStringLiteral("$1,234.50")));
            second.editor()->addItem(text);
            second.editor()->doc()->merge = ed->doc()->merge;
            QCOMPARE(shown(second.editor()->doc()), currencyImage);
        }
        // The language of the text at the field is the one the value is read and shown in: German here shows 1.234,50 with the euro sign, as the same words typed in would.
        {
            QTextCursor c(ed->doc()->storyDoc(box->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat cf;
            cf.setProperty(jp::tp::Language, QStringLiteral("de-DE"));
            c.mergeCharFormat(cf);
            jp::MainWindow second;
            second.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
            auto text = std::static_pointer_cast<jp::TextItem>(second.editor()->newTextBox(QRectF(72, 72, 400, 100), jp::formatMergeValue(QStringLiteral("1234.5"), QStringLiteral("currency"), QStringLiteral("de-DE"))));
            second.editor()->addItem(text);
            QTextCursor c2(second.editor()->doc()->storyDoc(text->storyId));
            c2.select(QTextCursor::Document);
            c2.mergeCharFormat(cf);
            second.editor()->doc()->merge = ed->doc()->merge;
            QCOMPARE(shown(second.editor()->doc()), shown(ed->doc()));
            QVERIFY(shown(ed->doc()) != currencyImage);
            cf.setProperty(jp::tp::Language, QString());
            c.mergeCharFormat(cf);
        }
        const QString path = dir.filePath(QStringLiteral("merge.jpub"));
        QVERIFY(w.saveTo(path));
        {
            jp::MainWindow again;
            QVERIFY(again.openFile(path));
            jp::TextItem *kept = textBoxOf(again.editor());
            QVERIFY(kept);
            QCOMPARE(fieldCode(again.editor(), kept), QStringLiteral("merge:Price|currency"));
            QCOMPARE(shown(again.editor()->doc()), currencyImage);
        }
        ed->undo();
        QCOMPARE(fieldCode(ed, box.get()), QStringLiteral("merge:Price"));
        QCOMPARE(shown(ed->doc()), plainImage);
        ed->redo();
        QCOMPARE(shown(ed->doc()), currencyImage);
    }

    // Duplicate Several Times is bounded: a size that grows by 1000 % for 200 copies stops before a copy is bigger than 20 times the page's larger side, numbers that are not numbers are refused,
    // and more than 2,000 new objects are refused with a message and no step of Undo.
    void duplicateSeveralTimesStaysWithinBounds()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(50, 50, 100, 60));
        ed->select(a);
        jp::Editor::DuplicateSteps steps;
        steps.copies = 200;
        steps.growPercent = 1000;
        QVERIFY(ed->duplicateSteps(steps));
        const int made = int(ed->surfaceItems().size()) - 1;
        QVERIFY2(made >= 1 && made < 10, qPrintable(QString::number(made)));
        for (const jp::ItemPtr &it : ed->surfaceItems()) {
            QVERIFY(std::isfinite(it->rect.width()) && std::isfinite(it->rect.height()));
            QVERIFY(it->rect.width() <= 20 * 792 && it->rect.height() <= 20 * 792);
        }
        ed->undo();
        QCOMPARE(int(ed->surfaceItems().size()), 1);
        // Shrinking stops at a point.
        ed->select(a);
        steps.growPercent = -90;
        QVERIFY(ed->duplicateSteps(steps));
        for (const jp::ItemPtr &it : ed->surfaceItems()) QVERIFY(it->rect.width() >= 1 && it->rect.height() >= 1);
        ed->undo();
        // Offsets that run off the page stop the copies too.
        ed->select(a);
        steps.growPercent = 0;
        steps.offset = QPointF(10000, 0);
        QVERIFY(ed->duplicateSteps(steps));
        QVERIFY(int(ed->surfaceItems().size()) <= 3);
        ed->undo();
        // Not numbers.
        const int steps0 = ed->undoStack()->count();
        ed->select(a);
        QString why;
        for (const double bad : {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()}) {
            jp::Editor::DuplicateSteps nan;
            nan.copies = 2;
            nan.growPercent = bad;
            QVERIFY(!ed->duplicateSteps(nan, &why));
            nan.growPercent = 0;
            nan.offset = QPointF(bad, 0);
            QVERIFY(!ed->duplicateSteps(nan, &why));
            nan.offset = QPointF(0, bad);
            why.clear();
            QVERIFY(!ed->duplicateSteps(nan, &why));
            QVERIFY(!why.isEmpty());
        }
        // Eleven objects times 200 copies is more than 2,000.
        QStringList all{a};
        for (int i = 0; i < 10; ++i) all << addShape(ed, QRectF(10 * i, 300, 5, 5));
        ed->select(all);
        steps = jp::Editor::DuplicateSteps();
        steps.copies = 200;
        why.clear();
        QVERIFY(!ed->duplicateSteps(steps, &why));
        QVERIFY(why.contains(QStringLiteral("2,000")));
        QCOMPARE(int(ed->surfaceItems().size()), 11);
        QCOMPARE(ed->undoStack()->count(), steps0);
        steps.copies = 181;   // (11 x 181 is 1,991)
        QVERIFY(ed->duplicateSteps(steps));
        QCOMPARE(int(ed->surfaceItems().size()), 11 + 11 * 181);
    }

    // Equalize uses an object's own width and height, the frame it is turned about, not the outline of the turned object.
    void equalizeUsesTheOwnSizeOfATurnedObject()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(50, 50, 100, 60)), b = addShape(ed, QRectF(200, 70, 40, 90)), c = addShape(ed, QRectF(300, 300, 70, 30));
        ed->doc()->item(b)->rotation = 45;
        ed->select(QStringList{a, b, c});
        QVERIFY(ed->equalize(true, false));
        QCOMPARE(rectOf(ed, b), QRectF(200, 70, 100, 90));
        QCOMPARE(ed->doc()->item(b)->rotation, 45.0);
        // A turned first object gives its own size.
        ed->undo();
        ed->doc()->item(a)->rotation = 45;
        ed->select(QStringList{a, b, c});
        QVERIFY(ed->equalize(true, true));
        QCOMPARE(rectOf(ed, b), QRectF(200, 70, 100, 60));
        QCOMPARE(rectOf(ed, c), QRectF(300, 300, 100, 60));
        QCOMPARE(ed->doc()->item(b)->rotation, 45.0);
    }

    // Send to Master Page leaves an object inside a group where it is, and no step of Undo is made; the whole group moves; and Undo and Redo go back and forth from another page, and a
    // left-hand page of a two-page master lands where the object was seen.
    void sendToMasterPageGroupsPagesAndLeftHandPages()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString p = addShape(ed, QRectF(10, 10, 20, 20)), q = addShape(ed, QRectF(60, 10, 20, 20));
        ed->select(QStringList{p, q});
        ed->groupSelection();
        const QString group = ed->selection().first();
        ed->insertPages(0, 1, false, false);
        ed->setCurrentPage(0);
        ed->select(group);
        const int steps0 = ed->undoStack()->count();
        ed->select(p);   // (a member of the group)
        QString why;
        QVERIFY(!ed->sendToMaster(&why));
        QVERIFY(!why.isEmpty());
        QCOMPARE(ed->undoStack()->count(), steps0);
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), 1);
        ed->select(group);
        QVERIFY(ed->sendToMaster());
        QCOMPARE(int(ed->doc()->masterFor(*ed->doc()->pages[0])->items.size()), 1);
        QVERIFY(ed->doc()->pages[0]->items.empty());
        // From another page, Undo brings it back to the first page, and Redo sends it again.
        ed->setCurrentPage(1);
        ed->undo();
        QCOMPARE(ed->currentPage(), 0);
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), 1);
        QVERIFY(ed->doc()->masterFor(*ed->doc()->pages[0])->items.empty());
        ed->redo();
        QVERIFY(ed->doc()->pages[0]->items.empty());
        QCOMPARE(int(ed->doc()->masterFor(*ed->doc()->pages[0])->items.size()), 1);
        ed->undo();
        // A left-hand page (the second) of a two-page master: the left half is as the page is, and an object in the right half lands one page width back.
        jp::MasterPage *master = ed->doc()->masterFor(*ed->doc()->pages[0]);
        master->twoPage = true;
        ed->setCurrentPage(1);
        ed->doc()->pages[0]->items.clear();
        auto s = std::make_shared<jp::ShapeItem>();
        s->rect = QRectF(250, 100, 50, 50);
        ed->doc()->pages[1]->items.push_back(s);
        ed->select(s->id);
        QVERIFY(ed->sendToMaster());
        QCOMPARE(master->items.back()->rect, QRectF(250, 100, 50, 50));   // (a left-hand page shows the left half as it is)
        ed->setMasterView(master->id);
        ed->select(master->items.back()->id);
        QVERIFY(ed->sendToPage());
        QCOMPARE(ed->doc()->pages[1]->items.back()->rect, QRectF(250, 100, 50, 50));
        auto t = std::make_shared<jp::ShapeItem>();
        t->rect = QRectF(612 + 250, 100, 50, 50);
        master->items.push_back(t);
        ed->select(t->id);
        QVERIFY(ed->sendToPage());
        QCOMPARE(ed->doc()->pages[1]->items.back()->rect, QRectF(250, 100, 50, 50));
    }

    // The merge field at the caret is found in a long story: with everything selected, and as the caret moves beside a field; and a format reaches every field of a selection.
    void aMergeFieldIsFoundInALongStoryAndFormatReachesEachOne()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 400), QString(60000, QLatin1Char('x'))));
        {
            QTextCursor c(ed->doc()->storyDoc(box->storyId));
            c.movePosition(QTextCursor::End);
            QTextCharFormat cf;
            cf.setProperty(jp::tp::Field, QStringLiteral("merge:Price"));
            for (int i = 0; i < 3; ++i) c.insertText(QString(QChar::ObjectReplacementCharacter), cf);
            QTextCharFormat plain;
            c.insertText(QStringLiteral("y"), plain);
        }
        ed->addItem(box);
        ed->beginTextEdit(box->id, 0);
        QVERIFY(!ed->mergeFieldAtCaret());
        QTextCursor all(ed->doc()->storyDoc(box->storyId));
        all.select(QTextCursor::Document);
        ed->setCursor(all);
        QVERIFY(ed->mergeFieldAtCaret());
        QVERIFY(ed->setMergeFieldFormat(QStringLiteral("currency")));
        QCOMPARE(ed->mergeFieldFormat(), QStringLiteral("currency"));
        int fields = 0;
        for (QTextBlock b = ed->doc()->storyDoc(box->storyId)->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().stringProperty(jp::tp::Field) == QLatin1String("merge:Price|currency")) fields += it.fragment().length();
        QCOMPARE(fields, 3);
        QTextCursor none(ed->doc()->storyDoc(box->storyId));
        none.setPosition(100);
        ed->setCursor(none);
        QVERIFY(!ed->mergeFieldAtCaret());
        QTextCursor beside(ed->doc()->storyDoc(box->storyId));
        beside.setPosition(60001);
        ed->setCursor(beside);
        QVERIFY(ed->mergeFieldAtCaret());
    }

    // A corner radius a file gives is held to none through half of the shorter side when it is read, and a number that is not a number is none.
    void aCornerRadiusFromAFileIsHeld()
    {
        auto s = std::make_shared<jp::ShapeItem>();
        s->shape = QStringLiteral("roundRect");
        s->rect = QRectF(0, 0, 100, 60);
        QJsonObject o = s->toJson();
        for (const auto &pair : {std::pair<double, double>{9.0, 0.5}, {-3.0, 0.0}, {0.25, 0.25}}) {
            o["adj"] = QJsonArray{pair.first};
            jp::ShapeItem read;
            read.fromJson(o);
            QCOMPARE(read.adj.value(0), pair.second);
        }
        QCOMPARE(jp::cornerRadius(QStringLiteral("roundRect"), {std::numeric_limits<double>::quiet_NaN()}, QSizeF(100, 60)), 0.0);
        QCOMPARE(jp::cornerRadius(QStringLiteral("roundRect"), {7.0}, QSizeF(100, 60)), 30.0);
    }

    // Resize from, for a shape turned 30 degrees: the chosen point stays where it is on the page after a new width and height.
    void theBasePointOfATurnedShapeStaysWhereItIsOnThePage()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        auto s = std::make_shared<jp::ShapeItem>();
        s->shape = QStringLiteral("roundRect");
        s->rect = QRectF(200, 200, 100, 100);
        s->rotation = 30;
        ed->doc()->pages[0]->items.push_back(s);
        const QString id = s->id;
        ed->select(id);
        QTRY_VERIFY(w.act(QStringLiteral("obj.format"))->isEnabled());
        const QPointF corner = ed->doc()->item(id)->transform().map(QPointF(100, 100));   // (bottom right)
        withDialog([&] { w.act(QStringLiteral("obj.format"))->trigger(); }, QStringLiteral("formatObjectDialog"), [&](QDialog *d) {
            auto *radius = d->findChild<jp::MeasureSpin *>(QStringLiteral("cornerRadius"));
            d->findChild<QComboBox *>(QStringLiteral("basePoint"))->setCurrentIndex(8);
            int set = 0;
            for (jp::MeasureSpin *m : d->findChildren<jp::MeasureSpin *>())
                if (m != radius && qAbs(m->value() - 100) < 0.01 && m->parentWidget() == radius->parentWidget()) {
                    m->setValue(60);
                    ++set;
                }
            QCOMPARE(set, 2);
        });
        const jp::Item *now = ed->doc()->item(id);
        QVERIFY(qAbs(now->rect.width() - 60) < 0.01 && qAbs(now->rect.height() - 60) < 0.01);
        QCOMPARE(now->rotation, 30.0);
        const QPointF after = now->transform().map(QPointF(60, 60));
        QVERIFY2(QLineF(corner, after).length() < 0.01, qPrintable(QStringLiteral("%1,%2 to %3,%4").arg(corner.x()).arg(corner.y()).arg(after.x()).arg(after.y())));
    }
};

JP_TEST_CLASS(PubLines11Tests)
#include "tst_pub_lines11.moc"
