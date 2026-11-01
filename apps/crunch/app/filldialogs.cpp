#include "app/filldialogs.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace jp {

FillAcrossDialog::FillAcrossDialog(const QStringList &names, int current, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("fillAcrossDialog"));
    setWindowTitle(tr("Fill Across Worksheets"));
    auto *layout = new QVBoxLayout(this);
    auto *text = new QLabel(tr("Copy the selected cells to the same cells of these sheets:"), this);
    text->setWordWrap(true);
    layout->addWidget(text);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("fillAcrossSheets"));
    m_list->setAccessibleName(tr("Sheets to copy to"));
    for (int i = 0; i < names.size(); ++i) {
        if (i == current) continue;
        auto *item = new QListWidgetItem(names[i], m_list);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        m_places.push_back(i);
    }
    layout->addWidget(m_list);
    auto *form = new QFormLayout;
    m_what = new QComboBox(this);
    m_what->setObjectName(QStringLiteral("fillAcrossWhat"));
    m_what->setAccessibleName(tr("What to copy"));
    m_what->addItem(tr("All"));
    m_what->addItem(tr("Contents"));
    m_what->addItem(tr("Formats"));
    form->addRow(tr("Copy:"), m_what);
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

std::vector<int> FillAcrossDialog::sheets() const
{
    std::vector<int> out;
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->checkState() == Qt::Checked) out.push_back(m_places[size_t(i)]);
    return out;
}

crunch::EditSession::FillWhat FillAcrossDialog::what() const
{
    using What = crunch::EditSession::FillWhat;
    return m_what->currentIndex() == 1 ? What::ValuesOnly : m_what->currentIndex() == 2 ? What::FormatsOnly : What::All;
}

} // namespace jp
