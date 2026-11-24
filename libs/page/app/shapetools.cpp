#include "app/shapetools.h"

#include "app/editor.h"
#include "app/uistyle.h"
#include "core/workthread.h"
#include "render/shapes.h"

#include <QCoreApplication>
#include <QDialog>
#include <QEventLoop>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

const char *const kMergeShapeIds[5] = {"shape.union", "shape.combine", "shape.subtract", "shape.intersect", "shape.fragment"};

void showMergeBox(QWidget *window, const QString &text, std::atomic<bool> *cancel)
{
    auto *box = new QDialog(window);
    box->setObjectName(QStringLiteral("busyBox"));
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setWindowModality(Qt::WindowModal);
    box->setWindowTitle(QCoreApplication::translate("jp::ShapeTools", "Merge Shapes"));
    auto *lay = new QVBoxLayout(box);
    auto *label = new QLabel(text, box);
    label->setTextFormat(Qt::PlainText);
    auto *bar = new QProgressBar(box);
    bar->setRange(0, 0);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Cancel, box);
    lay->addWidget(label);
    lay->addWidget(bar);
    lay->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, box, [cancel, label, buttons] {
        cancel->store(true);
        label->setText(QCoreApplication::translate("jp::ShapeTools", "Cancelling…"));
        buttons->setEnabled(false);
    });
    QObject::connect(box, &QDialog::rejected, box, [cancel] { cancel->store(true); });
    box->show();
}

void closeMergeBox(QWidget *window)
{
    for (QDialog *d : window->findChildren<QDialog *>(QStringLiteral("busyBox"), Qt::FindDirectChildrenOnly)) {
        d->disconnect(d);   // (only its own connections: a wildcard disconnect also cuts the style sheet's hold on the destroyed signal)
        d->close();
    }
}

bool shapeOutlineOnPage(const Item &item, QPainterPath *out)
{
    QString shape;
    QVector<double> adj;
    QPainterPath own;
    if (const auto *s = dynamic_cast<const ShapeItem *>(&item)) {
        if (s->isArt()) return false;
        shape = s->shape;
        adj = s->adj;
        own = s->customPath;
    } else if (const auto *t = dynamic_cast<const TextItem *>(&item)) {
        shape = t->shape.isEmpty() ? QStringLiteral("rect") : t->shape;
        adj = t->adj;
        own = t->customPath;
    } else {
        return false;
    }
    if (own.isEmpty()) own = shapePath(shape, item.rect.size(), adj);
    *out = item.transform().map(own);
    return !out->isEmpty();
}

QString mergeRefusal(int refused)
{
    return refused == 0 ? QCoreApplication::translate("jp::Editor", "The shapes leave nothing.") : refused == 2 ? QCoreApplication::translate("jp::Editor", "That would cut the shapes into too many pieces.")
         : refused == 3 ? QCoreApplication::translate("jp::Editor", "The shapes are too complex to merge in a reasonable time.") : refused == 4 ? QCoreApplication::translate("jp::Editor", "Cancelled. The shapes are as they were.")
         : QCoreApplication::translate("jp::Editor", "The shapes are too complex to merge.");
}

QVector<QPainterPath> mergeBounded(pathops::Op op, const QVector<QPainterPath> &outlines, int *refused, const std::function<void(const QString &, std::atomic<bool> *)> &started, const std::function<void()> &ended,
                                   const std::function<void(int, std::atomic<bool> &)> &stepHook)
{
    QVector<QPainterPath> result;
    if (const qint64 work = pathops::estimateWork(op, outlines); work <= pathops::kQuickWork || work > pathops::kMostWork) return pathops::merge(op, outlines, refused);   // (one past the most is refused at once)
    // (the work is counted before it starts and refused past pathops::kMostWork; what is left runs on a thread of its own, and the window stays alive and can cancel it)
    std::atomic<bool> cancel{false};
    QEventLoop loop;
    if (started) started(QCoreApplication::translate("jp::Editor", "Merging shapes…"), &cancel);
    {
        WorkThread worker([&] {
            result = pathops::merge(op, outlines, refused, [&](int step, int) {
                if (stepHook) stepHook(step, cancel);
                return !cancel.load();
            });
            QMetaObject::invokeMethod(&loop, &QEventLoop::quit, Qt::QueuedConnection);
        });
        if (worker.started()) loop.exec();
        else result = pathops::merge(op, outlines, refused);
    }
    if (ended) ended();
    return result;
}

void makeShapeCommands(QWidget *window, Editor *ed, const std::function<QAction *(const QString &, const QString &, const char *, const std::function<void()> &)> &make)
{
    make(QStringLiteral("shape.editPoints"), QCoreApplication::translate("jp::ShapeTools", "Edit Points"), "spline", [ed] {
        auto *s = dynamic_cast<ShapeItem *>(ed->single());
        if (!s) return;
        if (ed->pointsItem == s->id) { ed->setPointsItem(QString()); return; }
        // Convert to a freeform path so its points can be dragged; Ctrl+click a point deletes it, Ctrl+click an edge adds one, Esc finishes.
        if (s->customPath.isEmpty())
            ed->change(QCoreApplication::translate("jp::ShapeTools", "Edit Points"), [&] { s->customPath = shapePath(s->shape, s->rect.size(), s->adj); });
        ed->setPointsItem(s->id);
        Q_EMIT ed->status(QCoreApplication::translate("jp::ShapeTools", "Drag a point to reshape. Ctrl+click a point to delete it, or an edge to add one. Shift+click puts a glue point for connectors on the shape, or takes it away. Press Esc when done."));
    });
    // The box over a merge that runs on a worker thread (libs/page/app/editor_shapes.cpp): Cancel stops it at its next step, and the shapes stay as they were.
    QObject::connect(ed, &Editor::busyStarted, window, [window](const QString &text, std::atomic<bool> *cancel) { showMergeBox(window, text, cancel); });
    QObject::connect(ed, &Editor::busyEnded, window, [window] { closeMergeBox(window); });
    const pathops::Op ops[5] = {pathops::Op::Union, pathops::Op::Combine, pathops::Op::Subtract, pathops::Op::Intersect, pathops::Op::Fragment};
    const char *icons[5] = {"combine", "merge", "square-minus", "blend", "slice"};
    const QString names[5] = {QCoreApplication::translate("jp::ShapeTools", "Union"), QCoreApplication::translate("jp::ShapeTools", "Combine"), QCoreApplication::translate("jp::ShapeTools", "Subtract"), QCoreApplication::translate("jp::ShapeTools", "Intersect"), QCoreApplication::translate("jp::ShapeTools", "Fragment")};
    for (int i = 0; i < 5; ++i)
        make(QLatin1String(kMergeShapeIds[i]), names[i], icons[i], [window, ed, op = ops[i]] {
            // (the text of the shapes goes: with more than one shape that has some, ask once first)
            if (ed->mergeTextCount() > 1) {
                jp::MessageBox box(QMessageBox::Question, QCoreApplication::translate("jp::ShapeTools", "Merge Shapes"), QCoreApplication::translate("jp::ShapeTools", "More than one of the shapes has text. Merging removes the text of all of them. Merge anyway?"), QMessageBox::NoButton, window);
                box.setObjectName(QStringLiteral("mergeTextBox"));
                box.setTextFormat(Qt::PlainText);
                box.addButton(QMessageBox::Cancel);
                QPushButton *go = box.addButton(QCoreApplication::translate("jp::ShapeTools", "Merge"), QMessageBox::AcceptRole);
                box.exec();
                if (box.clickedButton() != go) return;
            }
            QString why;
            int made = 0, textRemoved = 0;
            const bool ok = ed->mergeShapes(op, &why, &made, &textRemoved);
            QString message = !ok ? why : made > 1 ? QCoreApplication::translate("jp::ShapeTools", "Made %n shapes.", "", made) : QString();
            if (ok && textRemoved > 0) message += (message.isEmpty() ? QString() : QStringLiteral(" ")) + QCoreApplication::translate("jp::ShapeTools", "The text in the merged shapes was removed.");
            if (!message.isEmpty()) Q_EMIT ed->status(message);
        });
}

void enableShapeCommands(Editor *ed, const std::function<QAction *(const QString &)> &find)
{
    const QStringList picked = ed->selection();
    // (what Editor::mergeShapes would refuse at once stays off: shapes inside a group, on more than one list, or more than the most)
    int shapes = 0;
    const ItemList *list = nullptr;
    bool together = picked.size() <= pathops::kMostShapes;
    for (qsizetype i = 0; i < picked.size() && together; ++i) {
        const Document::Loc at = ed->doc()->find(picked[i]);
        const auto *s = dynamic_cast<const ShapeItem *>(at.item);
        if (!s || s->isArt() || s->locked) continue;
        ++shapes;
        if (at.parent || (list && list != at.list)) together = false;
        list = at.list;
    }
    for (const char *id : kMergeShapeIds) find(QLatin1String(id))->setEnabled(together && shapes >= 2 && shapes == picked.size());
    const auto *one = picked.size() == 1 ? dynamic_cast<const ShapeItem *>(ed->doc()->item(picked.first())) : nullptr;
    find(QStringLiteral("shape.editPoints"))->setEnabled(one && !one->isArt() && !one->locked);
}

} // namespace jp
