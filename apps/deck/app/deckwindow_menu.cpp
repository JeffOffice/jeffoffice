// The right-click menu of the slide.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/icons.h"
#include "app/proofingdialogs.h"

#include <QAction>
#include <QMenu>
#include <memory>

namespace jp {

QMenu *DeckWindow::slideContextMenu(QWidget *parent)
{
    refreshUi();   // what the commands may do is up to date, not as of the last timer tick
    auto *menu = new QMenu(parent ? parent : this);
    auto add = [this, menu](QMenu *to, std::initializer_list<const char *> ids) {
        for (const char *id : ids) (to ? to : menu)->addAction(act(QString::fromLatin1(id)));
    };
    // A command under another name (the ribbon's label would not say enough here) that calls the ribbon's action.
    auto alias = [this, menu](const QString &id, const QString &text) {
        QAction *a = menu->addAction(text, menu, [this, id] { act(id)->trigger(); });
        a->setObjectName(id);
        a->setEnabled(act(id)->isEnabled());
    };
    const QString kind = m_ed->selectionKind();
    // Format Shape opens the Format pane; Format Picture, when that is all that is selected.
    act("shape.format")->setText(kind == QLatin1String("picture") ? tr("Format Picture…") : tr("Format Shape…"));
    if (m_ed->isEditingText()) {
        addSpellingSuggestions(*menu, m_ed);
        add(nullptr, {"edit.cut", "edit.copy", "edit.paste"});
        menu->addSeparator();
        add(nullptr, {"fmt.fontDialog", "para.dialog", "para.bullets", "para.numbers"});
        menu->addSeparator();
        add(nullptr, {"insert.link", "review.thesaurus"});
        menu->addSeparator();
        add(nullptr, {"shape.format", "comment.new"});
    } else if (!kind.isEmpty()) {
        add(nullptr, {"edit.cut", "edit.copy", "edit.paste", "edit.delete"});
        menu->addSeparator();
        const Item *one = m_ed->single();
        const bool holdsText = one && (one->type() == ItemType::Text || one->type() == ItemType::Shape || one->type() == ItemType::Table || one->type() == ItemType::TextArt);
        act("edit.text")->setEnabled(holdsText);
        if (holdsText) add(nullptr, {"edit.text"});
        if (kind == "multi") add(nullptr, {"arr.group"});
        if (kind == "group") add(nullptr, {"arr.ungroup"});
        QMenu *order = menu->addMenu(act("arr.front")->icon(), tr("Order"));
        add(order, {"arr.front", "arr.forward", "arr.backward", "arr.back"});
        if (act("fmt.alignLeft")->isEnabled()) {
            QMenu *align = menu->addMenu(act("fmt.alignLeft")->icon(), tr("Alignment"));
            add(align, {"fmt.alignLeft", "fmt.alignCenter", "fmt.alignRight", "fmt.justify"});
        }
        menu->addSeparator();
        add(nullptr, {"shape.format", "insert.link", "comment.new"});
    } else {
        add(nullptr, {"edit.paste"});
        menu->addSeparator();
        if (m_view != View::Master) {   // (a slide's, not a master's)
            QMenu *layout = layoutMenu(menu, true);
            layout->setTitle(tr("Layout"));
            layout->setIcon(icon("layout-template"));
            menu->addMenu(layout);
            alias(QStringLiteral("slide.reset"), tr("Reset Slide"));
            menu->addSeparator();
        }
        add(nullptr, {"design.formatBackground"});
        QMenu *grid = menu->addMenu(act("view.gridlines")->icon(), tr("Grid and Guides"));
        add(grid, {"view.gridlines", "view.guides"});
        add(nullptr, {"view.rulers"});
        menu->addSeparator();
        add(nullptr, {"comment.new"});
    }
    return menu;
}

void DeckWindow::showSlideContextMenu(const QPoint &global)
{
    if (m_menuOpen) return;   // the Menu key arrives twice on some systems
    m_menuOpen = true;
    std::unique_ptr<QMenu> menu(slideContextMenu(this));
    menu->exec(global);
    m_menuOpen = false;
}

} // namespace jp
