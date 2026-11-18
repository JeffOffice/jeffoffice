#include "app/objectsdialog.h"

#include "app/uistyle.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace jp::objects {

QString sourceTitle(Source source)
{
    switch (source) {
    case Source::Range: return QCoreApplication::translate("jp::Objects", "From JeffCrunch Range");
    case Source::Chart: return QCoreApplication::translate("jp::Objects", "From JeffCrunch Chart");
    case Source::Page: break;
    }
    return QCoreApplication::translate("jp::Objects", "From JeffWrite Page");
}

QString sizeWords(qint64 widthEmu, qint64 heightEmu)
{
    const QLocale locale;
    return QCoreApplication::translate("jp::Objects", "%1 by %2 in").arg(locale.toString(double(widthEmu) / 914400.0, 'f', 1), locale.toString(double(heightEmu) / 914400.0, 'f', 1));
}

bool askSource(QWidget *parent, Source source, bool copiedAvailable, SourceChoice *choice)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("objectSourceDialog"));
    dlg.setWindowTitle(sourceTitle(source));
    auto *v = new QVBoxLayout(&dlg);
    QCheckBox *copied = nullptr;
    if (copiedAvailable) {
        copied = new QCheckBox(source == Source::Page ? QCoreApplication::translate("jp::Objects", "Use the page copied in JeffWrite")
                                                      : source == Source::Chart ? QCoreApplication::translate("jp::Objects", "Use the chart copied in JeffCrunch")
                                                                                : QCoreApplication::translate("jp::Objects", "Use the range copied in JeffCrunch"), &dlg);
        copied->setObjectName(QStringLiteral("objectUseCopied"));
        copied->setChecked(true);
        v->addWidget(copied);
    }
    auto *form = new QFormLayout;
    auto *path = new QLineEdit(&dlg);
    path->setObjectName(QStringLiteral("objectFile"));
    auto *browse = new QPushButton(QCoreApplication::translate("jp::Objects", "Browse…"), &dlg);
    auto *row = new QHBoxLayout;
    row->addWidget(path, 1);
    row->addWidget(browse);
    form->addRow(source == Source::Page ? QCoreApplication::translate("jp::Objects", "Document") : QCoreApplication::translate("jp::Objects", "Workbook"), row);
    QLineEdit *range = nullptr;
    QSpinBox *page = nullptr;
    if (source == Source::Range) {
        range = new QLineEdit(&dlg);
        range->setObjectName(QStringLiteral("objectRange"));
        range->setPlaceholderText(QCoreApplication::translate("jp::Objects", "A1:C5"));
        form->addRow(QCoreApplication::translate("jp::Objects", "Range"), range);
    } else if (source == Source::Page) {
        page = new QSpinBox(&dlg);
        page->setObjectName(QStringLiteral("objectPage"));
        page->setRange(1, kMostPages);
        form->addRow(QCoreApplication::translate("jp::Objects", "Page"), page);
    }
    v->addLayout(form);
    auto sync = [&] {
        const bool own = !copied || !copied->isChecked();
        path->setEnabled(own);
        browse->setEnabled(own);
        if (range) range->setEnabled(own);
        if (page) page->setEnabled(own);
    };
    if (copied) QObject::connect(copied, &QCheckBox::toggled, &dlg, sync);
    sync();
    QObject::connect(browse, &QPushButton::clicked, &dlg, [&] {
        const QString filter = source == Source::Page ? QCoreApplication::translate("jp::Objects", "Documents (*.docx)") : QCoreApplication::translate("jp::Objects", "Workbooks (*.xlsx)");
        const QString picked = QFileDialog::getOpenFileName(&dlg, sourceTitle(source), QString(), filter + QStringLiteral(";;") + QCoreApplication::translate("jp::Objects", "All Files (*)"));
        if (!picked.isEmpty()) path->setText(picked);
    });
    auto *bb = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;
    choice->copied = copied && copied->isChecked();
    choice->path = path->text().trimmed();
    choice->detail = range ? range->text().trimmed() : page ? QString::number(page->value()) : QString();
    return choice->copied || !choice->path.isEmpty();
}

ObjectListDialog::ObjectListDialog(const ListHost &host, QWidget *parent)
    : QDialog(parent)
    , m_host(host)
{
    setObjectName(QStringLiteral("objectListDialog"));
    setWindowTitle(QCoreApplication::translate("jp::Objects", "Objects"));
    auto *v = new QVBoxLayout(this);
    m_list = new QTreeWidget(this);
    m_list->setObjectName(QStringLiteral("objectList"));
    m_list->setRootIsDecorated(false);
    m_list->setColumnCount(3);
    m_list->setHeaderLabels({QCoreApplication::translate("jp::Objects", "Name"), QCoreApplication::translate("jp::Objects", "Kind"), QCoreApplication::translate("jp::Objects", "Size")});
    m_list->header()->setStretchLastSection(false);
    m_list->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    v->addWidget(m_list, 1);
    auto *buttons = new QHBoxLayout;
    m_open = new QPushButton(QCoreApplication::translate("jp::Objects", "Open"), this);
    m_open->setObjectName(QStringLiteral("objectOpen"));
    m_convert = new QPushButton(QCoreApplication::translate("jp::Objects", "Convert to Picture"), this);
    m_convert->setObjectName(QStringLiteral("objectConvert"));
    m_delete = new QPushButton(QCoreApplication::translate("jp::Objects", "Delete"), this);
    m_delete->setObjectName(QStringLiteral("objectDelete"));
    buttons->addWidget(m_open);
    buttons->addWidget(m_convert);
    buttons->addWidget(m_delete);
    buttons->addStretch(1);
    v->addLayout(buttons);
    auto *bb = new jp::ButtonBox(QDialogButtonBox::Close, this);
    v->addWidget(bb);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_list, &QTreeWidget::currentItemChanged, this, [this] {
        const bool any = currentId() >= 0;
        m_open->setEnabled(any && m_host.open);
        m_convert->setEnabled(any && m_host.convertToPicture && m_list->currentItem()->data(0, Qt::UserRole + 1).toBool());
        m_delete->setEnabled(any && m_host.remove);
    });
    connect(m_open, &QPushButton::clicked, this, [this] {
        if (const int id = currentId(); id >= 0 && m_host.open) m_host.open(id);
    });
    connect(m_convert, &QPushButton::clicked, this, [this] {
        if (const int id = currentId(); id >= 0 && m_host.convertToPicture && m_host.convertToPicture(id)) fill();
    });
    connect(m_delete, &QPushButton::clicked, this, [this] {
        if (const int id = currentId(); id >= 0 && m_host.remove && m_host.remove(id)) fill();
    });
    connect(m_list, &QTreeWidget::itemDoubleClicked, this, [this] { m_open->click(); });
    resize(520, 320);
    fill();
}

void ObjectListDialog::fill()
{
    m_list->clear();
    const QVector<ObjectRow> rows = m_host.rows ? m_host.rows() : QVector<ObjectRow>();
    for (const ObjectRow &r : rows) {
        auto *item = new QTreeWidgetItem(m_list, {r.name, r.kind, r.size});
        item->setData(0, Qt::UserRole, r.id);
        item->setData(0, Qt::UserRole + 1, r.convertible);
    }
    if (m_list->topLevelItemCount() > 0) m_list->setCurrentItem(m_list->topLevelItem(0));
    else {
        m_open->setEnabled(false);
        m_convert->setEnabled(false);
        m_delete->setEnabled(false);
    }
}

int ObjectListDialog::currentId() const
{
    const QTreeWidgetItem *item = m_list->currentItem();
    return item ? item->data(0, Qt::UserRole).toInt() : -1;
}

int ObjectListDialog::rowCount() const { return m_list->topLevelItemCount(); }

QString ObjectListDialog::textAt(int row, int column) const { return m_list->topLevelItem(row)->text(column); }

void ObjectListDialog::select(int row) { m_list->setCurrentItem(m_list->topLevelItem(row)); }

} // namespace jp::objects
