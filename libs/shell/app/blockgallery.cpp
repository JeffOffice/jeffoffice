#include "app/blockgallery.h"

#include "app/filedialogs.h"
#include "app/plaintext.h"
#include "app/uistyle.h"

#include <QAction>
#include <QComboBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QWidgetAction>

namespace jp {

namespace {
constexpr int kIdRole = Qt::UserRole;
const QSize kCell(150, 104);
} // namespace

QString blockGalleryName(const QString &gallery) { return gallery == kShortcutsGallery ? QCoreApplication::translate("jp::Blocks", "Text Shortcuts") : QCoreApplication::translate("jp::Blocks", "Saved Blocks"); }

// Draws a block when its cell is first painted: the picture the app gives, over the block's name.
class BlockGalleryList::Delegate : public QStyledItemDelegate {
public:
    explicit Delegate(BlockGalleryList *owner) : QStyledItemDelegate(owner->m_list), m_owner(owner) {}
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override { return kCell; }
    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem o = option;
        initStyleOption(&o, index);
        o.text.clear();
        o.icon = QIcon();
        QStyledItemDelegate::paint(p, o, index);
        const QRect cell = option.rect.adjusted(3, 3, -3, -3);
        const QRect pictureBox = cell.adjusted(0, 0, 0, -18);
        const QPixmap pix = m_owner->previewOf(index.data(kIdRole).toString());
        p->save();
        p->setClipRect(cell);
        if (!pix.isNull()) {
            p->fillRect(pictureBox, Qt::white);
            p->drawPixmap(pictureBox.topLeft(), pix);
            p->setPen(option.palette.color(QPalette::Mid));
            p->drawRect(pictureBox.adjusted(0, 0, -1, -1));
        }
        p->setPen(option.palette.color(option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::WindowText));
        p->drawText(QRect(cell.left(), cell.bottom() - 16, cell.width(), 16), Qt::AlignCenter, p->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, cell.width()));
        p->restore();
    }

private:
    BlockGalleryList *m_owner;
};

BlockGalleryList::BlockGalleryList(const QString &gallery, const QString &format, const BlockPreview &preview, QWidget *parent)
    : QWidget(parent), m_gallery(gallery), m_format(format), m_preview(preview)
{
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(4, 4, 4, 4);
    v->setSpacing(4);
    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("blockSearch"));
    m_search->setPlaceholderText(QCoreApplication::translate("jp::Blocks", "Search by name"));
    m_search->setClearButtonEnabled(true);
    m_search->setAccessibleName(QCoreApplication::translate("jp::Blocks", "Search saved blocks"));
    v->addWidget(m_search);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("blockList"));
    m_list->setViewMode(QListView::IconMode);
    m_list->setMovement(QListView::Static);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setUniformItemSizes(true);
    m_list->setGridSize(kCell);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setFixedWidth(kCell.width() * 3 + 24);
    m_list->setMinimumHeight(kCell.height() * 2 + 8);
    m_list->setAccessibleName(QCoreApplication::translate("jp::Blocks", "Saved blocks"));
    m_list->setItemDelegate(new Delegate(this));
    v->addWidget(m_list);
    auto *empty = new QLabel(QCoreApplication::translate("jp::Blocks", "Nothing is saved here yet. Select some text, then choose Save Selection as a Block."), this);
    empty->setObjectName(QStringLiteral("blockEmpty"));
    empty->setWordWrap(true);
    empty->setTextFormat(Qt::PlainText);
    empty->setMaximumWidth(kCell.width() * 3);
    m_empty = empty;
    v->addWidget(empty);
    connect(m_search, &QLineEdit::textChanged, this, [this] { fill(); });
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *it) { Q_EMIT picked(it->data(kIdRole).toString()); });
    connect(m_list, &QListWidget::itemActivated, this, [this](QListWidgetItem *it) { Q_EMIT picked(it->data(kIdRole).toString()); });
}

void BlockGalleryList::setGallery(const QString &gallery)
{
    m_gallery = gallery;
    fill();
}

// The file is read again each time the gallery opens (another window may have added blocks); a large file is read on a thread of its own and the gallery fills when it is in.
void BlockGalleryList::reload()
{
    m_pictures.clear();
    BlockLibrary::shared().loadInBackground(this, [this](bool) { fill(); });
    fill();
}

QString BlockGalleryList::currentId() const { return m_list->currentItem() ? m_list->currentItem()->data(kIdRole).toString() : QString(); }

QStringList BlockGalleryList::shownIds() const
{
    QStringList out;
    for (int i = 0; i < m_list->count(); ++i) out << m_list->item(i)->data(kIdRole).toString();
    return out;
}

void BlockGalleryList::fill()
{
    const QString keep = currentId();
    const QString words = m_search->text().trimmed();
    m_list->clear();
    for (const BlockInfo &i : BlockLibrary::shared().list(m_gallery)) {
        if (i.format != m_format) continue;
        if (!words.isEmpty() && !i.name.contains(words, Qt::CaseInsensitive) && !i.category.contains(words, Qt::CaseInsensitive) && !i.description.contains(words, Qt::CaseInsensitive)) continue;
        auto *it = new QListWidgetItem(i.name, m_list);
        it->setData(kIdRole, i.id);
        it->setSizeHint(kCell);
        it->setToolTip(jp::plainToolTip(i.description.isEmpty() ? i.name : i.name + QLatin1Char('\n') + i.description));
        it->setData(Qt::AccessibleTextRole, i.description.isEmpty() ? i.name : i.name + QStringLiteral(". ") + i.description);
        if (i.id == keep) m_list->setCurrentItem(it);
    }
    m_empty->setVisible(m_list->count() == 0);
    m_list->setVisible(m_list->count() > 0);
}

QPixmap BlockGalleryList::previewOf(const QString &id)
{
    const auto have = m_pictures.constFind(id);
    if (have != m_pictures.cend()) return *have;
    QPixmap pix;
    if (m_preview) {
        if (const auto info = BlockLibrary::shared().find(id)) {
            const qreal dpr = devicePixelRatioF();
            BlockEntry e;
            e.info = *info;
            e.data = BlockLibrary::shared().data(id);
            const QImage img = m_preview(e, QSize(kCell.width() - 6, kCell.height() - 6 - 18), dpr);
            if (!img.isNull()) pix = QPixmap::fromImage(img);
            pix.setDevicePixelRatio(dpr);
        }
        ++m_previewsMade;
    }
    m_pictures.insert(id, pix);
    return pix;
}

QMenu *blocksMenu(QWidget *owner, const QString &gallery, const QString &format, const BlockPreview &preview, const BlockInsertFn &insert, const QList<QAction *> &footer)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("blocksMenu"));
    auto *gal = new BlockGalleryList(gallery, format, preview, menu);
    gal->setObjectName(QStringLiteral("blockGallery"));
    auto *host = new QWidgetAction(menu);
    host->setDefaultWidget(gal);
    menu->addAction(host);
    menu->addSeparator();
    for (QAction *a : footer) menu->addAction(a);
    QObject::connect(menu, &QMenu::aboutToShow, gal, [gal] {
        gal->search()->clear();
        gal->reload();
        gal->search()->setFocus(Qt::PopupFocusReason);
    });
    QObject::connect(gal, &BlockGalleryList::picked, menu, [menu, insert](const QString &id) {
        const auto info = BlockLibrary::shared().find(id);
        const QByteArray data = BlockLibrary::shared().data(id);
        menu->close();
        if (info && insert) insert(*info, data);
    });
    return menu;
}

bool saveBlockBox(QWidget *parent, BlockInfo *info, bool editing)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("saveBlockBox"));
    dlg.setWindowTitle(editing ? QCoreApplication::translate("jp::Blocks", "Block Properties") : QCoreApplication::translate("jp::Blocks", "Save as a Block"));
    auto *form = new QFormLayout(&dlg);
    auto *name = new QLineEdit(info->name, &dlg);
    name->setObjectName(QStringLiteral("blockName"));
    name->setMaxLength(BlockLibrary::kMostNameChars);
    name->setAccessibleName(QCoreApplication::translate("jp::Blocks", "Name"));
    form->addRow(QCoreApplication::translate("jp::Blocks", "Name"), name);
    auto *gallery = new QComboBox(&dlg);
    gallery->setObjectName(QStringLiteral("blockGalleryChoice"));
    gallery->setAccessibleName(QCoreApplication::translate("jp::Blocks", "Gallery"));
    gallery->addItem(blockGalleryName(kBlocksGallery), kBlocksGallery);
    gallery->addItem(blockGalleryName(kShortcutsGallery), kShortcutsGallery);
    gallery->setCurrentIndex(info->gallery == kShortcutsGallery ? 1 : 0);
    gallery->setEnabled(!editing);
    form->addRow(QCoreApplication::translate("jp::Blocks", "Gallery"), gallery);
    auto *category = new QLineEdit(info->category, &dlg);
    category->setObjectName(QStringLiteral("blockCategory"));
    category->setAccessibleName(QCoreApplication::translate("jp::Blocks", "Category"));
    form->addRow(QCoreApplication::translate("jp::Blocks", "Category"), category);
    auto *description = new QPlainTextEdit(info->description, &dlg);
    description->setObjectName(QStringLiteral("blockDescription"));
    description->setAccessibleName(QCoreApplication::translate("jp::Blocks", "Description"));
    description->setFixedHeight(64);
    form->addRow(QCoreApplication::translate("jp::Blocks", "Description"), description);
    auto *how = new QComboBox(&dlg);
    how->setObjectName(QStringLiteral("blockInsertAs"));
    how->setAccessibleName(QCoreApplication::translate("jp::Blocks", "Put it in"));
    how->addItem(QCoreApplication::translate("jp::Blocks", "As it is, at the cursor"), int(BlockInsert::Content));
    how->addItem(QCoreApplication::translate("jp::Blocks", "In a paragraph of its own"), int(BlockInsert::Paragraph));
    how->addItem(QCoreApplication::translate("jp::Blocks", "On a page of its own"), int(BlockInsert::Page));
    how->setCurrentIndex(int(info->insertAs));
    form->addRow(QCoreApplication::translate("jp::Blocks", "Put it in"), how);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText(QCoreApplication::translate("jp::Blocks", "Save"));
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    auto check = [&] { buttons->button(QDialogButtonBox::Ok)->setEnabled(!BlockLibrary::cleanName(name->text()).isEmpty()); };
    QObject::connect(name, &QLineEdit::textChanged, &dlg, check);
    check();
    if (dlg.exec() != QDialog::Accepted) return false;
    info->name = BlockLibrary::cleanName(name->text());
    info->gallery = gallery->currentData().toString();
    info->category = category->text().trimmed();
    info->description = description->toPlainText().trimmed();
    info->insertAs = BlockInsert(how->currentData().toInt());
    return true;
}

bool saveBlock(QWidget *parent, const BlockInfo &info, const QByteArray &data)
{
    BlockLibrary &lib = BlockLibrary::shared();
    lib.load();
    BlockEntry e;
    e.info = info;
    e.data = data;
    QString err;
    bool replace = false;
    if (lib.findByName(info.gallery, info.name)) {
        if (plainQuestion(parent, QCoreApplication::translate("jp::Blocks", "Save as a Block"), QCoreApplication::translate("jp::Blocks", "A block with this name is already saved. Replace it?")) != QMessageBox::Yes) return false;
        replace = true;
    }
    if (lib.add(e, nullptr, &err, replace)) return true;
    plainWarning(parent, QCoreApplication::translate("jp::Blocks", "Save as a Block"), err);
    return false;
}

void manageBlocksBox(QWidget *parent, const QString &format, const BlockPreview &preview)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("manageBlocksBox"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::Blocks", "Manage Saved Blocks"));
    auto *column = new QVBoxLayout(&dlg);
    auto *which = new QComboBox(&dlg);
    which->setObjectName(QStringLiteral("manageGallery"));
    which->setAccessibleName(QCoreApplication::translate("jp::Blocks", "Gallery"));
    which->addItem(blockGalleryName(kBlocksGallery), kBlocksGallery);
    which->addItem(blockGalleryName(kShortcutsGallery), kShortcutsGallery);
    column->addWidget(which);
    auto *gal = new BlockGalleryList(kBlocksGallery, format, preview, &dlg);
    column->addWidget(gal);
    auto *message = new QLabel(&dlg);
    message->setObjectName(QStringLiteral("manageMessage"));
    message->setTextFormat(Qt::PlainText);
    message->setWordWrap(true);
    column->addWidget(message);
    auto *row = new QHBoxLayout();
    auto button = [&](const QString &text, const char *objectName) {
        auto *b = new QPushButton(text, &dlg);
        b->setObjectName(QLatin1String(objectName));
        b->setAutoDefault(false);
        row->addWidget(b);
        return b;
    };
    QPushButton *edit = button(QCoreApplication::translate("jp::Blocks", "Edit Properties…"), "manageEdit");
    QPushButton *del = button(QCoreApplication::translate("jp::Blocks", "Delete"), "manageDelete");
    QPushButton *undo = button(QCoreApplication::translate("jp::Blocks", "Undo Delete"), "manageUndo");
    QPushButton *exp = button(QCoreApplication::translate("jp::Blocks", "Export…"), "manageExport");
    QPushButton *imp = button(QCoreApplication::translate("jp::Blocks", "Import…"), "manageImport");
    row->addStretch(1);
    auto *close = new jp::ButtonBox(QDialogButtonBox::Close, &dlg);
    column->addLayout(row);
    column->addWidget(close);
    QObject::connect(close, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(close, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QVector<BlockEntry> deleted;   // what was deleted, newest last: Undo Delete puts it back until the box closes
    auto refresh = [&] {
        gal->setGallery(which->currentData().toString());
        const bool have = !gal->currentId().isEmpty();
        edit->setEnabled(have);
        del->setEnabled(have);
        undo->setEnabled(!deleted.isEmpty());
        exp->setEnabled(!BlockLibrary::shared().list().isEmpty());
    };
    auto tell = [&](const QString &text) { message->setText(text); };
    QObject::connect(which, &QComboBox::currentIndexChanged, &dlg, [&] { refresh(); });
    QObject::connect(gal->list(), &QListWidget::currentItemChanged, &dlg, [&] {
        const bool have = !gal->currentId().isEmpty();
        edit->setEnabled(have);
        del->setEnabled(have);
    });
    QObject::connect(edit, &QPushButton::clicked, &dlg, [&] {
        BlockLibrary &lib = BlockLibrary::shared();
        const auto info = lib.find(gal->currentId());
        if (!info) return;
        BlockInfo now = *info;
        if (!saveBlockBox(&dlg, &now, true)) return;
        QString err;
        tell(lib.update(now, &err) ? QString() : err);
        refresh();
    });
    QObject::connect(del, &QPushButton::clicked, &dlg, [&] {
        BlockLibrary &lib = BlockLibrary::shared();
        const QString id = gal->currentId();
        const auto info = lib.find(id);
        if (!info) return;
        BlockEntry e;
        e.info = *info;
        e.data = lib.data(id);
        QString err;
        if (!lib.remove({id}, &err)) {
            tell(err);
            return;
        }
        deleted.push_back(e);
        tell(QCoreApplication::translate("jp::Blocks", "Deleted \"%1\". Undo Delete puts it back until you close this box.").arg(e.info.name));
        refresh();
    });
    QObject::connect(undo, &QPushButton::clicked, &dlg, [&] {
        if (deleted.isEmpty()) return;
        const BlockEntry e = deleted.takeLast();
        QString err;
        tell(BlockLibrary::shared().add(e, nullptr, &err) ? QString() : err);
        refresh();
    });
    QObject::connect(exp, &QPushButton::clicked, &dlg, [&] {
        const QString path = askSavePath(&dlg, QCoreApplication::translate("jp::Blocks", "Export Saved Blocks"), QCoreApplication::translate("jp::Blocks", "Saved Blocks.jblocks"), QCoreApplication::translate("jp::Blocks", "Saved blocks (*.jblocks)"));
        if (path.isEmpty()) return;
        QString err;
        tell(BlockLibrary::shared().exportTo(path, QStringList(), &err) ? QCoreApplication::translate("jp::Blocks", "The saved blocks were exported.") : err);
    });
    QObject::connect(imp, &QPushButton::clicked, &dlg, [&] {
        const QString path = QFileDialog::getOpenFileName(&dlg, QCoreApplication::translate("jp::Blocks", "Import Saved Blocks"), QString(), QCoreApplication::translate("jp::Blocks", "Saved blocks (*.jblocks)"));
        if (path.isEmpty()) return;
        QString err;
        int skipped = 0;
        const int n = BlockLibrary::shared().importFrom(path, &err, &skipped);
        if (n < 0) tell(err);
        else tell(QCoreApplication::translate("jp::Blocks", "%n block(s) were added.", nullptr, n) + QLatin1Char(' ') + QCoreApplication::translate("jp::Blocks", "%n block(s) were left out because that name is already saved.", nullptr, skipped));
        refresh();
    });
    BlockLibrary::shared().load();
    refresh();
    dlg.exec();
}

} // namespace jp
