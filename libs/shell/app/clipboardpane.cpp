#include "app/clipboardpane.h"

#include "app/icons.h"
#include "app/plaintext.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDockWidget>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

QString clipboardLauncherKeytip() { return QStringLiteral("FO"); }

QString ClipboardPane::kindName(ClipboardHistory::Kind kind)
{
    using K = ClipboardHistory::Kind;
    switch (kind) {
    case K::Text: return QCoreApplication::translate("jp::ClipboardPane", "Text");
    case K::FormattedText: return QCoreApplication::translate("jp::ClipboardPane", "Formatted text");
    case K::Cells: return QCoreApplication::translate("jp::ClipboardPane", "Cells");
    case K::Picture: return QCoreApplication::translate("jp::ClipboardPane", "Picture");
    case K::Shapes: return QCoreApplication::translate("jp::ClipboardPane", "Shapes");
    case K::Slides: return QCoreApplication::translate("jp::ClipboardPane", "Slides");
    case K::Other: break;
    }
    return QCoreApplication::translate("jp::ClipboardPane", "Other");
}

static QString kindIcon(ClipboardHistory::Kind kind)
{
    using K = ClipboardHistory::Kind;
    switch (kind) {
    case K::Text:
    case K::FormattedText: return QStringLiteral("file-text");
    case K::Cells: return QStringLiteral("sheet");
    case K::Picture: return QStringLiteral("image");
    case K::Shapes: return QStringLiteral("shapes");
    case K::Slides: return QStringLiteral("presentation");
    case K::Other: break;
    }
    return QStringLiteral("clipboard-list");
}

ClipboardPane::ClipboardPane(const ClipboardPaneHost &host, QWidget *parent) : QWidget(parent), m_host(host)
{
    setObjectName(QStringLiteral("clipboardPane"));
    auto *col = new QVBoxLayout(this);
    col->setContentsMargins(8, 8, 8, 8);
    auto *buttons = new QHBoxLayout;
    m_pasteAll = new QPushButton(QCoreApplication::translate("jp::ClipboardPane", "Paste All"), this);
    m_pasteAll->setObjectName(QStringLiteral("clipboardPasteAll"));
    m_delete = new QPushButton(QCoreApplication::translate("jp::ClipboardPane", "Delete"), this);
    m_delete->setObjectName(QStringLiteral("clipboardDelete"));
    m_clearAll = new QPushButton(QCoreApplication::translate("jp::ClipboardPane", "Clear All"), this);
    m_clearAll->setObjectName(QStringLiteral("clipboardClearAll"));
    buttons->addWidget(m_pasteAll);
    buttons->addWidget(m_delete);
    buttons->addWidget(m_clearAll);
    col->addLayout(buttons);
    m_empty = plainLabel(new QLabel(QCoreApplication::translate("jp::ClipboardPane", "Copy or cut something and it will be here."), this));
    m_empty->setObjectName(QStringLiteral("clipboardEmpty"));
    m_empty->setWordWrap(true);
    col->addWidget(m_empty);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("clipboardList"));
    m_list->setIconSize(QSize(48, 48));
    m_list->setUniformItemSizes(true);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    m_list->setAccessibleName(QCoreApplication::translate("jp::ClipboardPane", "Clipboard"));
    m_list->installEventFilter(this);
    col->addWidget(m_list, 1);
    m_status = plainLabel(new QLabel(this));
    m_status->setObjectName(QStringLiteral("clipboardStatus"));
    m_status->setWordWrap(true);
    m_status->hide();
    col->addWidget(m_status);
    m_closedBox = new QCheckBox(QCoreApplication::translate("jp::ClipboardPane", "Keep collecting when this pane is closed"), this);
    m_closedBox->setObjectName(QStringLiteral("clipboardCollectClosed"));
    col->addWidget(m_closedBox);
    m_othersBox = new QCheckBox(QCoreApplication::translate("jp::ClipboardPane", "Also collect what is copied in other programs"), this);
    m_othersBox->setObjectName(QStringLiteral("clipboardCollectOthers"));
    m_othersBox->setToolTip(QCoreApplication::translate("jp::ClipboardPane", "Only plain text and pictures are kept, and nothing a password manager marks as secret."));
    col->addWidget(m_othersBox);
    if (!ClipboardHistory::canCollectOtherPrograms()) m_othersBox->hide();
    auto *note = plainLabel(new QLabel(QCoreApplication::translate("jp::ClipboardPane", "Items are kept in this program only, until it is closed. Nothing is saved to a file."), this));
    note->setWordWrap(true);
    col->addWidget(note);

    ClipboardHistory *h = ClipboardHistory::instance();
    m_closedBox->setChecked(h->collectWhenClosed());
    m_othersBox->setChecked(h->collectOtherPrograms());
    connect(m_closedBox, &QCheckBox::toggled, this, [](bool on) { ClipboardHistory::instance()->setCollectWhenClosed(on); });
    connect(m_othersBox, &QCheckBox::toggled, this, [](bool on) { ClipboardHistory::instance()->setCollectOtherPrograms(on); });
    connect(m_pasteAll, &QPushButton::clicked, this, [this] { pasteAll(); });
    connect(m_delete, &QPushButton::clicked, this, [this] { deleteCurrent(); });
    connect(m_clearAll, &QPushButton::clicked, this, [this] { clearAll(); });
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *it) {
        if (it) pasteItem(it->data(Qt::UserRole).toULongLong());
    });
    connect(m_list, &QWidget::customContextMenuRequested, this, [this](const QPoint &at) { contextMenu(at); });
    connect(h, &ClipboardHistory::changed, this, [this] { refresh(); });
    connect(h, &ClipboardHistory::tooLarge, this, [this] { updateStatus(); });
    refresh();
}

ClipboardPane::~ClipboardPane()
{
    setShown(false);
}

void ClipboardPane::setShown(bool shown)
{
    if (shown == m_counted) return;
    m_counted = shown;
    if (ClipboardHistory *h = ClipboardHistory::instance()) h->addOpenPane(shown ? 1 : -1);
    if (shown) refresh();
}

quint64 ClipboardPane::idAt(int row) const
{
    QListWidgetItem *it = m_list->item(row);
    return it ? it->data(Qt::UserRole).toULongLong() : 0;
}

void ClipboardPane::refresh()
{
    const ClipboardHistory *h = ClipboardHistory::instance();
    const quint64 current = m_list->currentItem() ? m_list->currentItem()->data(Qt::UserRole).toULongLong() : 0;
    m_list->clear();
    for (const ClipboardHistory::Item &it : h->items()) {
        QString words = it.preview.simplified().left(80);
        auto *row = new QListWidgetItem(kindName(it.kind) + (words.isEmpty() ? QString() : QLatin1Char('\n') + words), m_list);
        row->setData(Qt::UserRole, it.id);
        row->setIcon(it.thumbnail.isNull() ? jp::icon(kindIcon(it.kind)) : QIcon(QPixmap::fromImage(it.thumbnail)));
        row->setToolTip(plainToolTip(it.preview.left(200)));
        if (it.id == current) m_list->setCurrentItem(row);
    }
    m_empty->setVisible(m_list->count() == 0);
    m_pasteAll->setEnabled(m_list->count() > 0);
    m_clearAll->setEnabled(m_list->count() > 0);
    m_delete->setEnabled(m_list->count() > 0);
    updateStatus();
}

void ClipboardPane::updateStatus()
{
    ClipboardHistory *h = ClipboardHistory::instance();
    QStringList said;
    if (h->tooLargeCount() > 0)
        said << QCoreApplication::translate("jp::ClipboardPane", "%n copy was too large to keep (the limit is 8 MB for one copy).", "%n is a count", h->tooLargeCount());
    if (h->droppedCount() > 0)
        said << QCoreApplication::translate("jp::ClipboardPane", "%n oldest copy was dropped to stay under 64 MB in all.", "%n is a count", h->droppedCount());
    m_status->setText(said.join(QLatin1Char('\n')));
    m_status->setVisible(!said.isEmpty());
}

void ClipboardPane::pasteItem(quint64 id)
{
    ClipboardHistory *h = ClipboardHistory::instance();
    if (!h->putOnClipboard(id)) return;
    if (m_host.focusBack) m_host.focusBack();
    if (m_host.paste) m_host.paste();
}

void ClipboardPane::pasteAll()
{
    QList<quint64> ids;
    for (const ClipboardHistory::Item &it : ClipboardHistory::instance()->items()) ids.prepend(it.id);
    for (quint64 id : ids) pasteItem(id);
}

void ClipboardPane::clearAll()
{
    ClipboardHistory *h = ClipboardHistory::instance();
    h->takeCounts();
    h->clear();
    refresh();
}

void ClipboardPane::deleteCurrent()
{
    if (QListWidgetItem *it = m_list->currentItem()) ClipboardHistory::instance()->remove(it->data(Qt::UserRole).toULongLong());
}

void ClipboardPane::contextMenu(const QPoint &at)
{
    QListWidgetItem *it = m_list->itemAt(at);
    if (!it) return;
    m_list->setCurrentItem(it);
    QMenu menu(this);
    const quint64 id = it->data(Qt::UserRole).toULongLong();
    menu.addAction(QCoreApplication::translate("jp::ClipboardPane", "Paste"), this, [this, id] { pasteItem(id); });
    menu.addAction(QCoreApplication::translate("jp::ClipboardPane", "Delete"), this, [this] { deleteCurrent(); });
    menu.exec(m_list->viewport()->mapToGlobal(at));
}

bool ClipboardPane::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_list && event->type() == QEvent::KeyPress) {
        const int key = static_cast<QKeyEvent *>(event)->key();
        if (key == Qt::Key_Delete) {
            deleteCurrent();
            return true;
        }
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            if (QListWidgetItem *it = m_list->currentItem()) pasteItem(it->data(Qt::UserRole).toULongLong());
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

QDockWidget *clipboardDock(QMainWindow *win, const ClipboardPaneHost &host)
{
    if (!win) return nullptr;
    auto *dock = win->findChild<QDockWidget *>(QStringLiteral("clipboardDock"));
    if (dock) return dock;
    dock = new QDockWidget(QCoreApplication::translate("jp::ClipboardPane", "Clipboard"), win);
    dock->setObjectName(QStringLiteral("clipboardDock"));
    dock->setFeatures(QDockWidget::DockWidgetClosable);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto *pane = new ClipboardPane(host, dock);
    dock->setWidget(pane);
    win->addDockWidget(Qt::LeftDockWidgetArea, dock);
    QObject::connect(dock, &QDockWidget::visibilityChanged, pane, [pane](bool visible) { pane->setShown(visible); });
    dock->hide();
    return dock;
}

void showClipboardPane(QMainWindow *win, const ClipboardPaneHost &host)
{
    QDockWidget *dock = clipboardDock(win, host);
    if (!dock) return;
    dock->show();
    dock->raise();
    if (auto *pane = dock->findChild<ClipboardPane *>()) {
        pane->setShown(true);
        pane->list()->setFocus();
    }
}

} // namespace jp
