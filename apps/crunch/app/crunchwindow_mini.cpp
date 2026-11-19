#include "app/crunchwindow.h"
#include "core/invalidcells.h"
#include "app/sortdialog.h"
#include "grid/pagelayoutview.h"
#include "app/icons.h"
#include "app/minitoolbar.h"
#include "app/settings.h"
#include <QAction>
#include <QEvent>
#include <QFontMetrics>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>
#include <QTextLayout>
#include <QWidgetAction>

// The mini toolbar in JeffCrunch: the bar every app has, with the commands of the cells. It floats by text selected with the mouse in a cell being edited (the characters
// get runs of their own), and sits over the shortcut menu of selected cells (every button is the ribbon's own command, one step of Undo).

using jp::office::sheet::FormatChange;

namespace jp {

namespace {

// Looks at the mouse released in the cell editor (the selection is made then) and at the editor going away.
class EditorWatch : public QObject {
public:
    EditorWatch(QObject *parent, std::function<void()> released, std::function<void()> hidden) : QObject(parent), m_released(std::move(released)), m_hidden(std::move(hidden)) {}
    bool eventFilter(QObject *, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton)
            QMetaObject::invokeMethod(this, [this] { m_released(); }, Qt::QueuedConnection);
        else if (event->type() == QEvent::Hide)
            m_hidden();
        return false;
    }

private:
    std::function<void()> m_released, m_hidden;
};

// Holds the window's one bar of cells for a menu: the menu is given the bar, and takes it back (before it is deleted, as its children go with it) into the window.
class BarHolder : public QWidgetAction {
public:
    BarHolder(QObject *parent, QFrame *bar, QWidget *home) : QWidgetAction(parent), m_bar(bar), m_home(home) {}
    void giveBack()
    {
        if (!m_bar) return;
        QWidget *bar = m_bar;
        releaseWidget(bar);   // (it is the bar's to keep: deleteWidget below only sends it home)
        if (bar->parentWidget() != m_home) bar->setParent(m_home);
        bar->hide();
    }

protected:
    QWidget *createWidget(QWidget *parent) override
    {
        m_bar->setParent(parent);
        return m_bar;
    }
    void deleteWidget(QWidget *widget) override
    {
        widget->hide();
        widget->setParent(m_home);
    }

private:
    QPointer<QFrame> m_bar;
    QWidget *m_home;
};

class CellMenu : public QMenu {
public:
    using QMenu::QMenu;
    BarHolder *holder = nullptr;
    ~CellMenu() override
    {
        if (holder) holder->giveBack();
    }
};

// Looks for a press on a box of the bar (the arrow that opens its list).
class ListWatch : public QObject {
public:
    ListWatch(QObject *parent, std::function<void()> pressed) : QObject(parent), m_pressed(std::move(pressed)) {}
    bool eventFilter(QObject *, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress) m_pressed();
        return false;
    }

private:
    std::function<void()> m_pressed;
};

} // namespace

QMenu *CrunchWindow::newCellMenu()
{
    auto *menu = new CellMenu(this);
    m_cellMenu = menu;
    return menu;
}

// The bar for cells (`inCells`: every cell selected is formatted) or for text selected in the cell editor (the selected characters are).
QFrame *CrunchWindow::makeMiniBar(bool inCells, QWidget *parent)
{
    auto target = std::make_unique<FunctionMiniTarget>();
    target->aligned = true;
    target->actionFor = [this](MiniTarget::Command c) -> QAction * {
        using C = MiniTarget::Command;
        switch (c) {
        case C::Bold: return act(QStringLiteral("fmt.bold"));
        case C::Italic: return act(QStringLiteral("fmt.italic"));
        case C::Underline: return act(QStringLiteral("fmt.underline"));
        case C::Shrink: return act(QStringLiteral("fmt.shrinkFont"));
        case C::Grow: return act(QStringLiteral("fmt.growFont"));
        default: return nullptr;
        }
    };
    // (a cell being edited keeps the keyboard: the entry, and its selection, stay as they are)
    auto keepEntry = [this] { if (m_edit.active) m_cellEditor->setFocus(Qt::OtherFocusReason); };
    target->setFont = [this, keepEntry](const QString &family) {
        FormatChange c;
        c.fontName = family;
        formatSelection(c, tr("Font"));
        keepEntry();
    };
    target->setSize = [this, keepEntry](double pt) {
        if (pt >= 1 && pt <= 409) setFontSize(pt);
        keepEntry();
    };
    target->setColor = [this, keepEntry](const ColorRef &c) {
        chooseFontColor(c);
        keepEntry();
    };
    if (inCells) {
        // (a command that asks something, or says no, in a box: the menu is closed first and the command run once it is gone, not from inside a click in it)
        target->takes = [this](QAction *a) {
            const bool asks = a == act(QStringLiteral("fmt.merge")) || (m_session && m_session->protectionOf(m_sheet).enabled);
            if (!asks || !m_cellMenu) return false;
            m_cellMenu->close();
            QMetaObject::invokeMethod(this, [a] { a->trigger(); }, Qt::QueuedConnection);
            return true;
        };
        target->setFill = [this](const ColorRef &c) { chooseFillColor(c); };
        auto *borders = new QAction(icon(QStringLiteral("grid-2x2")), tr("Borders"), parent);
        borders->setMenu(buildBordersMenu(parent));
        target->more << borders;
        for (const char *id : {"fmt.currency", "fmt.percent", "fmt.comma", "fmt.decimalInc", "fmt.decimalDec", "fmt.merge", "fmt.painter"})
            if (QAction *a = act(QString::fromLatin1(id))) target->more << a;
    }
    auto *bar = new MiniToolbar(parent, std::move(target));
    refreshMiniBar(bar);
    if (bar->colorButton()) bar->colorButton()->setScheme(workbookColors());
    if (bar->fillButton()) bar->fillButton()->setScheme(workbookColors());
    return bar;
}

// The font and size boxes show the cell with the cursor (the characters selected, in an entry, are of its first run's look as the ribbon's boxes are).
void CrunchWindow::refreshMiniBar(QFrame *frame)
{
    auto *bar = static_cast<MiniToolbar *>(frame);
    const FormatView v = viewOfActiveCell();
    if (bar->fontBox()) bar->fontBox()->setCurrentFamily(v.fontName);
    if (bar->sizeBox()) bar->sizeBox()->setSize(v.fontSize);
}

// At the top of the shortcut menu of cells: a menu is a popup that takes every click, so the bar is part of it, not a window beside it.
void CrunchWindow::addMiniToolbarTo(QMenu *menu)
{
    if (!miniToolbarOnSelection() || !m_session || m_chartSheetShown) return;
    if (!m_miniCells) {
        m_miniCells = makeMiniBar(true, this);
        m_miniCells->setObjectName(QStringLiteral("miniToolbar"));
        ++m_miniCellsBuilt;
        // (what a command made of the cell, the size it grew to: shown while the menu stays up)
        connect(static_cast<MiniToolbar *>(m_miniCells.data()), &MiniToolbar::commandRan, this, [this] {
            if (m_miniCells) refreshMiniBar(m_miniCells);
        });
    }
    refreshMiniBar(m_miniCells);
    if (auto *bar = static_cast<MiniToolbar *>(m_miniCells.data()); bar->colorButton()) {
        bar->colorButton()->setScheme(workbookColors());
        bar->fillButton()->setScheme(workbookColors());
    }
    auto *holder = new BarHolder(menu, m_miniCells, this);
    static_cast<CellMenu *>(menu)->holder = holder;
    menu->addAction(holder);
    holder->setEnabled(true);
    holder->setVisible(true);
    menu->addSeparator();
}

void CrunchWindow::createMiniToolbar()
{
    m_miniText = makeMiniBar(false, m_cellEditor->parentWidget());
    auto *bar = static_cast<MiniToolbar *>(m_miniText.data());
    bar->setObjectName(QStringLiteral("miniToolbar"));
    bar->hideOnTypingAndScrolling(m_view);
    if (m_pageView) bar->hideOnTypingAndScrolling(m_pageView);   // (the pages scroll under the entry as the grid does)
    bar->fadeAwayFromPointer(true);
    // (a click on the bar is a click in the grid's viewport, which would take the focus from the entry: the entry is the bar's focus)
    bar->setFocusPolicy(Qt::ClickFocus);
    bar->setFocusProxy(m_cellEditor);
    m_cellEditor->installEventFilter(bar);   // (typing, Enter, Tab, and Escape in the entry send it away)
    m_cellEditor->installEventFilter(new EditorWatch(bar, [this] { updateMiniToolbar(true); }, [bar] { bar->hide(); }));
    // (the lists of the font and size boxes take the keyboard as they open: the characters stay selected, and the bar stays, while one is open)
    for (QWidget *box : {static_cast<QWidget *>(bar->fontBox()), static_cast<QWidget *>(bar->sizeBox())})
        if (box) box->installEventFilter(new ListWatch(box, [this] { m_cellEditor->keepSelectionWhileAListOpens(); }));
    connect(m_cellEditor, &QLineEdit::selectionChanged, this, [this] { updateMiniToolbar(false); });
}

// Shown above the characters selected in the entry once a mouse selection ends; gone with the selection.
void CrunchWindow::updateMiniToolbar(bool show)
{
    if (!m_miniText) return;
    auto *bar = static_cast<MiniToolbar *>(m_miniText.data());
    const QString text = m_cellEditor->text();
    if (!m_edit.active || !m_cellEditor->isVisible() || !m_cellEditor->hasSelectedText() || text.startsWith(QLatin1Char('=')) || (!show && !bar->isVisible()) || !miniToolbarOnSelection()) {
        bar->hide();
        return;
    }
    QWidget *host = m_cellEditor->parentWidget();
    if (bar->parentWidget() != host) bar->setParent(host);
    const int from = m_cellEditor->selectionStart(), to = from + int(m_cellEditor->selectedText().size());
    const int x1 = m_cellEditor->xOfPosition(from), x2 = m_cellEditor->xOfPosition(to);
    const QRect line(m_cellEditor->mapTo(host, QPoint(std::min(x1, x2), 0)), QSize(std::max(1, std::abs(x2 - x1)), m_cellEditor->height()));
    if (show) refreshMiniBar(bar);
    bar->showBeside(line, line);
}

} // namespace jp
