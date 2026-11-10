// The Selection Pane over an editor's objects (see pageselectionpane.h).

#include "app/pageselectionpane.h"

#include "app/editor.h"

#include <QCoreApplication>
#include <QSet>

namespace jp {

namespace {

// A name as an object keeps it: no control characters, and at most 255 characters (a whole pair of surrogates is not cut).
QString cleanObjectName(const QString &text)
{
    QString out;
    out.reserve(std::min<qsizetype>(text.size(), 255));
    for (const QChar c : text) {
        if (c.unicode() < 0x20 || c.unicode() == 0x7F) continue;
        if (out.size() >= 255) break;
        out += c;
    }
    if (!out.isEmpty() && out.back().isHighSurrogate()) out.chop(1);
    return out.trimmed();
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
    // The rows are the surface's objects, the one in front first: row r is the object at n - 1 - r of the list, which is read where it stands, never copied.
    auto at = [ed](int row) -> Item * {
        const ItemList &l = ed->surfaceItems();
        const int n = int(l.size());
        return row >= 0 && row < n ? l[size_t(n - 1 - row)].get() : nullptr;
    };
    SelectionPane::Source src;
    src.rows = [ed] {
        QVector<SelectionPane::Row> rows;
        const ItemList &list = ed->surfaceItems();
        const QStringList &sel = ed->selection();
        const QSet<QString> picked(sel.cbegin(), sel.cend());
        const int n = int(list.size());
        rows.reserve(n);
        for (int row = 0; row < n; ++row) {
            const Item &it = *list[size_t(n - 1 - row)];
            rows.push_back({it.name.isEmpty() ? defaultObjectName(it, n - row) : it.name, !it.hidden, picked.contains(it.id)});
        }
        return rows;
    };
    src.pick = [ed, at](int row) {
        if (const Item *it = at(row)) ed->select(it->id);
    };
    src.rename = [ed, at](int row, const QString &name) {
        const Item *it = at(row);
        if (!it) return;
        const QString id = it->id, text = cleanObjectName(name);
        // (The name shown for an object with none is not a name until it is changed.)
        if (it->name.isEmpty() && (text.isEmpty() || text == defaultObjectName(*it, int(ed->surfaceItems().size()) - row))) return;
        if (text == it->name) return;
        ed->change(QCoreApplication::translate("PageSelectionPane", "Rename Object"), [&] {
            if (Item *target = ed->doc()->item(id)) target->name = text;
        });
    };
    src.show = [ed, at](int row, bool shown) {
        const Item *it = at(row);
        if (!it) return;
        const QString id = it->id;
        ed->change(shown ? QCoreApplication::translate("PageSelectionPane", "Show Object") : QCoreApplication::translate("PageSelectionPane", "Hide Object"), [&] {
            if (Item *target = ed->doc()->item(id)) target->hidden = !shown;
        });
    };
    src.showAll = [ed](bool shown) {
        bool any = false;
        for (const ItemPtr &it : ed->surfaceItems()) any = any || it->hidden == shown;
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
    QObject::connect(ed, &Editor::changed, pane, &SelectionPane::requestRefresh);
    QObject::connect(ed, &Editor::selectionChanged, pane, &SelectionPane::requestRefresh);
    QObject::connect(ed, &Editor::pageChanged, pane, &SelectionPane::requestRefresh);
    QObject::connect(ed, &Editor::documentReplaced, pane, &SelectionPane::requestRefresh);
    pane->show();
    pane->raise();
    return pane;
}

SelectionPane *showPageReadingOrderPane(QWidget *parent, Editor *ed, QPointer<SelectionPane> *slot)
{
    if (*slot) {
        (*slot)->refresh();
        (*slot)->show();
        (*slot)->raise();
        return *slot;
    }
    // Row r is the object at r of the list (the back one is read first), read where it stands.
    auto at = [ed](int row) -> Item * {
        const ItemList &l = ed->surfaceItems();
        return row >= 0 && row < int(l.size()) ? l[size_t(row)].get() : nullptr;
    };
    SelectionPane::Source src;
    src.title = QCoreApplication::translate("PageSelectionPane", "Reading Order");
    src.listName = QCoreApplication::translate("PageSelectionPane", "Objects in the order they are read");
    src.note = QCoreApplication::translate("PageSelectionPane", "Changing the reading order also changes which object is in front.");
    src.forwardText = QCoreApplication::translate("PageSelectionPane", "Read Earlier");
    src.backwardText = QCoreApplication::translate("PageSelectionPane", "Read Later");
    src.rows = [ed] {
        QVector<SelectionPane::Row> rows;
        const ItemList &list = ed->surfaceItems();
        const QStringList &sel = ed->selection();
        const QSet<QString> picked(sel.cbegin(), sel.cend());
        rows.reserve(int(list.size()));
        for (int row = 0; row < int(list.size()); ++row) {
            const Item &it = *list[size_t(row)];
            rows.push_back({it.name.isEmpty() ? defaultObjectName(it, row + 1) : it.name, !it.hidden, picked.contains(it.id)});
        }
        return rows;
    };
    src.pick = [ed, at](int row) {
        if (const Item *it = at(row)) ed->select(it->id);
    };
    src.move = [ed](int from, int to) {
        const int n = int(ed->surfaceItems().size());
        if (from < 0 || to < 0 || from >= n || to >= n || from == to) return;
        ed->change(QCoreApplication::translate("PageSelectionPane", "Change Reading Order"), [&] {
            ItemList &l = ed->surfaceItems();
            ItemPtr it = l[size_t(from)];
            l.erase(l.begin() + from);
            l.insert(l.begin() + to, it);
        });
    };
    auto *pane = new SelectionPane(parent, src);
    *slot = pane;
    QObject::connect(ed, &Editor::changed, pane, &SelectionPane::requestRefresh);
    QObject::connect(ed, &Editor::selectionChanged, pane, &SelectionPane::requestRefresh);
    QObject::connect(ed, &Editor::pageChanged, pane, &SelectionPane::requestRefresh);
    QObject::connect(ed, &Editor::documentReplaced, pane, &SelectionPane::requestRefresh);
    pane->show();
    pane->raise();
    return pane;
}

} // namespace jp
