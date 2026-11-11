#include "app/barcodeinsert.h"

#include "app/barcodedialog.h"
#include "app/editor.h"
#include "core/barcodeitem.h"
#include "core/document.h"
#include "core/items.h"

#include <QCoreApplication>

namespace jp {

void barcodeDialog(QWidget *p, Editor *ed, bool findRoom)
{
    // With a barcode selected, the dialog edits it: its own settings, and
    // the new one takes its place.
    GroupItem *editing = nullptr;
    if (const QVector<Item *> sel = ed->selectedItems(); sel.size() == 1)
        if (auto *g = dynamic_cast<GroupItem *>(sel.first()); g && !g->barcode.isEmpty()) editing = g;
    QJsonObject settings;
    if (!askBarcode(p, editing ? editing->barcode : QJsonObject(), &settings)) return;

    if (editing) {
        // In place: the same spot and identity, and one undo step.
        ItemPtr made = barcodeFromSettings(settings, editing->rect.topLeft());
        if (!made) return;
        made->id = editing->id;
        made->name = editing->name;
        ItemList &items = ed->surfaceItems();
        for (auto &it : items)
            if (it.get() == editing) {
                ed->change(QCoreApplication::translate("BarcodeDialog", "Edit Barcode"), [&] { it = made; });
                ed->select(made->id);
                return;
            }
        return;
    }
    ItemPtr made = barcodeFromSettings(settings, QPointF());
    if (!made) return;
    // In the middle of the page, or a quarter inch on from a barcode
    // already there, so a second one doesn't land on the first.
    const QSizeF page = ed->doc()->pageSize();
    QPointF at((page.width() - made->rect.width()) / 2, (page.height() - made->rect.height()) / 2);
    for (bool moved = true; moved;) {
        moved = false;
        for (const ItemPtr &it : ed->surfaceItems())
            if (auto *g = dynamic_cast<const GroupItem *>(it.get()); g && !g->barcode.isEmpty() && (g->rect.topLeft() - at).manhattanLength() < 2) {
                at += QPointF(18, 18);
                moved = true;
            }
    }
    if (findRoom) {
        // The lower right, the lower left, the upper right, and the upper left,  that touches nothing already on the page.
        const QSizeF s = made->rect.size();
        bool placed = false;
        for (const double m : {36.0, 18.0, 6.0})
            for (const QPointF &c : {QPointF(page.width() - s.width() - m, page.height() - s.height() - m), QPointF(m, page.height() - s.height() - m),
                                     QPointF(page.width() - s.width() - m, m), QPointF(m, m)}) {
                bool free = !placed && c.x() >= 0 && c.y() >= 0;
                for (const ItemPtr &it : ed->surfaceItems())
                    if (it->bounds().intersects(QRectF(c, s))) free = false;
                if (free) {
                    at = c;
                    placed = true;
                }
            }
    }
    made->moveBy(at.x() - made->rect.x(), at.y() - made->rect.y());
    ed->addItem(made);
}

} // namespace jp
