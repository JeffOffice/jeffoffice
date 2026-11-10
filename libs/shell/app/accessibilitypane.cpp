#include "app/accessibilitypane.h"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

AccessibilityPane::AccessibilityPane(QWidget *parent) : QDialog(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("accessibilityPane"));
    setWindowTitle(tr("Accessibility Checker"));
    auto *layout = new QVBoxLayout(this);
    m_summary = new QLabel(this);
    m_summary->setObjectName(QStringLiteral("accessSummary"));
    m_summary->setTextFormat(Qt::PlainText);
    m_summary->setWordWrap(true);
    layout->addWidget(m_summary);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("accessList"));
    m_list->setAccessibleName(tr("Problems found"));
    m_list->setWordWrap(true);
    layout->addWidget(m_list, 1);
    auto *again = new QPushButton(tr("Check Again"), this);
    again->setObjectName(QStringLiteral("accessAgain"));
    m_buttons = new QHBoxLayout;
    m_buttons->addStretch(1);
    m_buttons->addWidget(again);
    layout->addLayout(m_buttons);
    resize(420, 340);
    connect(again, &QPushButton::clicked, this, &AccessibilityPane::recheckRequested);
    connect(m_list, &QListWidget::itemActivated, this, [this](QListWidgetItem *item) { Q_EMIT goToRequested(item->data(Qt::UserRole).toList()); });
    connect(m_list, &QListWidget::itemClicked, m_list, [this](QListWidgetItem *item) { Q_EMIT m_list->itemActivated(item); });
}

QPushButton *AccessibilityPane::addButton(const QString &text, const QString &objectName)
{
    auto *button = new QPushButton(text, this);
    button->setObjectName(objectName);
    m_buttons->insertWidget(1, button);
    return button;
}

void AccessibilityPane::setFindings(const QVector<AccessFinding> &findings)
{
    m_list->clear();
    for (const AccessFinding &i : findings) {
        const QString level = i.level == AccessFinding::Error ? tr("Error") : i.level == AccessFinding::Warning ? tr("Warning") : tr("Tip");
        auto *item = new QListWidgetItem(level + QStringLiteral(": ") + i.message, m_list);
        item->setData(Qt::UserRole, i.where);
    }
    m_summary->setText(findings.isEmpty() ? tr("No accessibility problems were found.") : tr("%1 problems found. Click one to go to it.").arg(findings.size()));
}

int AccessibilityPane::issueCount() const { return m_list->count(); }

} // namespace jp
