#include "app/customviewsdialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

using jp::office::sheet::CustomView;

CustomViewsDialog::CustomViewsDialog(const CustomViewsRequest &request, QWidget *parent) : QDialog(parent), m_request(request)
{
    setObjectName(QStringLiteral("customViewsDialog"));
    setWindowTitle(tr("Custom Views"));
    auto *layout = new QVBoxLayout(this);
    auto *about = new QLabel(tr("A view keeps which sheet is in front, the hidden rows and columns, the zoom, the frozen panes, the gridlines and headings, and the size of the window. Type a name and choose Add to keep how the workbook looks now. A view with the same name is replaced."), this);
    about->setWordWrap(true);
    about->setTextFormat(Qt::PlainText);
    layout->addWidget(about);
    auto *listLabel = new QLabel(tr("Views:"), this);
    layout->addWidget(listLabel);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("customViewList"));
    m_list->setAccessibleName(tr("Views"));
    listLabel->setBuddy(m_list);
    layout->addWidget(m_list, 1);
    auto *row = new QHBoxLayout;
    auto *nameLabel = new QLabel(tr("Name:"), this);
    m_name = new QLineEdit(this);
    m_name->setObjectName(QStringLiteral("customViewName"));
    m_name->setMaxLength(jp::office::sheet::kMaxCustomViewNameChars);
    nameLabel->setBuddy(m_name);
    m_add = new QPushButton(tr("Add"), this);
    m_add->setObjectName(QStringLiteral("customViewAdd"));
    row->addWidget(nameLabel);
    row->addWidget(m_name, 1);
    row->addWidget(m_add);
    layout->addLayout(row);
    auto *buttons = new QHBoxLayout;
    m_show = new QPushButton(tr("Show"), this);
    m_show->setObjectName(QStringLiteral("customViewShow"));
    m_delete = new QPushButton(tr("Delete"), this);
    m_delete->setObjectName(QStringLiteral("customViewDelete"));
    auto *close = new QPushButton(tr("Close"), this);
    close->setObjectName(QStringLiteral("customViewClose"));
    buttons->addWidget(m_show);
    buttons->addWidget(m_delete);
    buttons->addStretch(1);
    buttons->addWidget(close);
    layout->addLayout(buttons);
    connect(m_add, &QPushButton::clicked, this, &CustomViewsDialog::addView);
    connect(m_delete, &QPushButton::clicked, this, &CustomViewsDialog::deleteView);
    connect(m_show, &QPushButton::clicked, this, &CustomViewsDialog::showView);
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &CustomViewsDialog::showView);
    connect(m_list, &QListWidget::currentRowChanged, this, [this] { refresh(); });
    connect(m_name, &QLineEdit::textChanged, this, [this] { refresh(); });
    connect(m_name, &QLineEdit::returnPressed, m_add, &QPushButton::click);
    m_add->setDefault(false);
    m_list->setCurrentRow(m_request.views.isEmpty() ? -1 : 0);
    refresh();
}

void CustomViewsDialog::refresh()
{
    const int keep = m_list->currentRow();
    QSignalBlocker block(m_list);
    m_list->clear();
    for (const CustomView &v : m_request.views) m_list->addItem(v.name);
    m_list->setCurrentRow(m_request.views.isEmpty() ? -1 : std::min<int>(std::max(keep, 0), int(m_request.views.size()) - 1));
    const bool any = m_list->currentRow() >= 0;
    m_show->setEnabled(any);
    m_delete->setEnabled(any);
    m_add->setEnabled(!m_name->text().trimmed().isEmpty() && bool(m_request.capture) && m_request.views.size() < jp::office::sheet::kMaxCustomViews);
}

void CustomViewsDialog::addView()
{
    const QString name = m_name->text().trimmed();
    if (name.isEmpty() || !m_request.capture) return;
    CustomView made = m_request.capture(name);
    made.name = name;
    for (CustomView &v : m_request.views) {
        if (v.name.compare(name, Qt::CaseInsensitive) != 0) continue;
        made.guid = v.guid;   // (replacing a view keeps its identity)
        v = made;
        m_name->clear();
        refresh();
        return;
    }
    m_request.views.append(made);
    m_name->clear();
    m_list->setCurrentRow(int(m_request.views.size()) - 1);
    refresh();
    m_list->setCurrentRow(int(m_request.views.size()) - 1);
}

void CustomViewsDialog::deleteView()
{
    const int row = m_list->currentRow();
    if (row < 0 || row >= m_request.views.size()) return;
    m_request.views.removeAt(row);
    refresh();
}

void CustomViewsDialog::showView()
{
    const int row = m_list->currentRow();
    if (row < 0 || row >= m_request.views.size()) return;
    m_request.show = m_request.views[row].name;
    accept();
}

CustomViewsRequest CustomViewsDialog::result() const
{
    return m_request;
}

} // namespace jp
