#include "app/crunchwindow.h"
#include "app/icons.h"
#include "app/minitoolbar.h"
#include "app/settings.h"
#include <QAction>
#include <QEvent>
#include <QFontMetrics>
#include <QMenu>
#include <QMouseEvent>
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

} // namespace

// The bar for cells (`inCells`: every cell selected is formatted) or for text selected in the cell editor (the selected characters are).
QFrame *CrunchWindow::makeMiniBar(bool inCells, QWidget *parent)
{
    auto target = std::make_unique<FunctionMiniTarget>();
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
        target->setFill = [this](const ColorRef &c) { chooseFillColor(c); };
        auto *borders = new QAction(icon(QStringLiteral("grid-2x2")), tr("Borders"), parent);
        borders->setMenu(buildBordersMenu(parent));
        target->more << borders;
        for (const char *id : {"fmt.currency", "fmt.percent", "fmt.comma", "fmt.decimalInc", "fmt.decimalDec", "fmt.merge", "fmt.painter"})
            if (QAction *a = act(QString::fromLatin1(id))) target->more << a;
    }
    auto *bar = new MiniToolbar(parent, std::move(target));
    const FormatView v = viewOfActiveCell();
    if (bar->fontBox()) bar->fontBox()->setCurrentFamily(v.fontName);
    if (bar->sizeBox()) bar->sizeBox()->setSize(v.fontSize);
    if (bar->colorButton()) bar->colorButton()->setScheme(workbookColors());
    if (bar->fillButton()) bar->fillButton()->setScheme(workbookColors());
    return bar;
}

// At the top of the shortcut menu of cells: a menu is a popup that takes every click, so the bar is part of it, not a window beside it.
void CrunchWindow::addMiniToolbarTo(QMenu *menu)
{
    if (!miniToolbarOnSelection() || !m_session || m_chartSheetShown) return;
    QFrame *bar = makeMiniBar(true, menu);
    bar->setObjectName(QStringLiteral("miniToolbar"));
    auto *holder = new QWidgetAction(menu);
    holder->setDefaultWidget(bar);
    menu->addAction(holder);
    holder->setEnabled(true);
    bar->setEnabled(true);
    bar->show();   // (made hidden, as the bar is for text; in the menu it is always there)
    menu->addSeparator();
}

void CrunchWindow::createMiniToolbar()
{
    m_miniText = makeMiniBar(false, m_cellEditor->parentWidget());
    auto *bar = static_cast<MiniToolbar *>(m_miniText.data());
    bar->setObjectName(QStringLiteral("miniToolbar"));
    bar->hideOnTypingAndScrolling(m_view);
    bar->fadeAwayFromPointer(true);
    // (a click on the bar is a click in the grid's viewport, which would take the focus from the entry: the entry is the bar's focus)
    bar->setFocusPolicy(Qt::ClickFocus);
    bar->setFocusProxy(m_cellEditor);
    m_cellEditor->installEventFilter(bar);   // (typing, Enter, Tab, and Escape in the entry send it away)
    m_cellEditor->installEventFilter(new EditorWatch(bar, [this] { updateMiniToolbar(true); }, [bar] { bar->hide(); }));
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
    if (show) {
        const FormatView v = viewOfActiveCell();
        if (bar->fontBox()) bar->fontBox()->setCurrentFamily(v.fontName);
        if (bar->sizeBox()) bar->sizeBox()->setSize(v.fontSize);
    }
    bar->showBeside(line, line);
}

} // namespace jp
