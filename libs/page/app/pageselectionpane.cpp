// The Selection Pane over an editor's objects (see pageselectionpane.h).

#include "app/pageselectionpane.h"

#include "app/editor.h"

#include <QCoreApplication>

namespace jp {

namespace {

// The surface's objects, the one in front first.
ItemList frontFirst(Editor *ed)
{
    const ItemList &list = ed->surfaceItems();
    return ItemList(list.rbegin(), list.rend());
}

} // namespace

QString defaultObjectName(const Item &item, int numberFromBack)
{
    return QStringLiteral("%1 %2").arg(itemTypeName(item.type())).arg(numberFromBack);
}

SelectionPane *showPageSelectionPane(QWidget *parent, Editor *ed, QPointer<SelectionPane> *slot)
{
    if (*slot) {
        (*slot)->refresh();
        (*slot)->show();
        (*slot)->raise();
        return *slot;
    }
    SelectionPane::Source src;
    src.rows = [ed] {
        QVector<SelectionPane::Row> rows;
        const ItemList list = frontFirst(ed);
        for (size_t i = 0; i < list.size(); ++i) {
            const Item &it = *list[i];
            rows.push_back({it.name.isEmpty() ? defaultObjectName(it, int(list.size() - i)) : it.name, !it.hidden, ed->selection().contains(it.id)});
        }
        return rows;
    };
    src.pick = [ed](int row) {
        const ItemList list = frontFirst(ed);
        if (row >= 0 && row < int(list.size())) ed->select(list[size_t(row)]->id);
    };
    src.rename = [ed](int row, const QString &name) {
        const ItemList list = frontFirst(ed);
        if (row < 0 || row >= int(list.size())) return;
        const QString id = list[size_t(row)]->id;
        const QString text = name.trimmed();
        // (The name shown for an object with none is not a name until it is changed.)
        if (list[size_t(row)]->name.isEmpty() && (text.isEmpty() || text == defaultObjectName(*list[size_t(row)], int(list.size()) - row))) return;
        ed->change(QCoreApplication::translate("PageSelectionPane", "Rename Object"), [&] {
            if (Item *it = ed->doc()->item(id)) it->name = text;
        });
    };
    src.show = [ed](int row, bool shown) {
        const ItemList list = frontFirst(ed);
        if (row < 0 || row >= int(list.size())) return;
        const QString id = list[size_t(row)]->id;
        ed->change(shown ? QCoreApplication::translate("PageSelectionPane", "Show Object") : QCoreApplication::translate("PageSelectionPane", "Hide Object"), [&] {
            if (Item *it = ed->doc()->item(id)) it->hidden = !shown;
        });
    };
    src.showAll = [ed](bool shown) {
        const ItemList list = frontFirst(ed);
        bool any = false;
        for (const ItemPtr &it : list) any = any || it->hidden == shown;
        if (!any) return;
        ed->change(shown ? QCoreApplication::translate("PageSelectionPane", "Show All Objects") : QCoreApplication::translate("PageSelectionPane", "Hide All Objects"), [&] {
            for (const ItemPtr &it : ed->surfaceItems()) it->hidden = !shown;
        });
    };
    src.move = [ed](int from, int to) {
        const int n = int(ed->surfaceItems().size());
        if (from < 0 || to < 0 || from >= n || to >= n || from == to) return;
        ed->change(QCoreApplication::translate("PageSelectionPane", "Arrange"), [&] {
            ItemList &l = ed->surfaceItems();
            ItemPtr it = l[size_t(n - 1 - from)];
            l.erase(l.begin() + (n - 1 - from));
            l.insert(l.begin() + (n - 1 - to), it);
        });
    };
    auto *pane = new SelectionPane(parent, src);
    *slot = pane;
    QObject::connect(ed, &Editor::changed, pane, &SelectionPane::refresh);
    QObject::connect(ed, &Editor::selectionChanged, pane, &SelectionPane::refresh);
    QObject::connect(ed, &Editor::pageChanged, pane, &SelectionPane::refresh);
    QObject::connect(ed, &Editor::documentReplaced, pane, &SelectionPane::refresh);
    pane->show();
    pane->raise();
    return pane;
}

} // namespace jp
