#include "app/uistyle.h"
#include "app/errorcheckdialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

ErrorCheckDialog::ErrorCheckDialog(const QStringList &cells, const QStringList &messages, int first, QWidget *parent, const QStringList &fixes) : QDialog(parent), m_cells(cells), m_messages(messages), m_fixes(fixes)
{
    setObjectName(QStringLiteral("errorCheckDialog"));
    setWindowTitle(tr("Error Checking"));
    auto *layout = new QVBoxLayout(this);
    m_title = new QLabel(this);
    m_title->setObjectName(QStringLiteral("errorCheckTitle"));
    m_title->setTextFormat(Qt::PlainText);
    m_text = new QLabel(this);
    m_text->setObjectName(QStringLiteral("errorCheckText"));
    m_text->setTextFormat(Qt::PlainText);
    m_text->setWordWrap(true);
    m_text->setMinimumWidth(360);
    layout->addWidget(m_title);
    layout->addWidget(m_text);
    auto *buttons = new jp::ButtonBox(this);
    m_fix = buttons->addButton(QString(), QDialogButtonBox::ActionRole);
    m_fix->setObjectName(QStringLiteral("errorCheckFix"));
    while (m_fixes.size() < m_cells.size()) m_fixes << QString();
    m_ignore = buttons->addButton(tr("Ignore Error"), QDialogButtonBox::ActionRole);
    m_previous = buttons->addButton(tr("Previous"), QDialogButtonBox::ActionRole);
    m_next = buttons->addButton(tr("Next"), QDialogButtonBox::ActionRole);
    auto *close = buttons->addButton(tr("Close"), QDialogButtonBox::RejectRole);
    m_ignore->setObjectName(QStringLiteral("errorCheckIgnore"));
    m_previous->setObjectName(QStringLiteral("errorCheckPrevious"));
    m_next->setObjectName(QStringLiteral("errorCheckNext"));
    connect(close, &QPushButton::clicked, this, &QDialog::reject);
    layout->addWidget(buttons);
    connect(m_previous, &QPushButton::clicked, this, [this] { showProblem((m_current + count() - 1) % count()); });
    connect(m_next, &QPushButton::clicked, this, [this] { showProblem((m_current + 1) % count()); });
    connect(m_fix, &QPushButton::clicked, this, [this] {
        const int gone = m_current;
        m_cells.removeAt(gone);
        m_messages.removeAt(gone);
        m_fixes.removeAt(gone);
        Q_EMIT fixed(gone);
        if (m_cells.isEmpty()) {
            accept();
            return;
        }
        showProblem(std::min(gone, count() - 1));
    });
    connect(m_ignore, &QPushButton::clicked, this, [this] {
        const int gone = m_current;
        m_cells.removeAt(gone);
        m_messages.removeAt(gone);
        m_fixes.removeAt(gone);
        Q_EMIT ignored(gone);
        if (m_cells.isEmpty()) {
            accept();
            return;
        }
        showProblem(std::min(gone, count() - 1));
    });
    showProblem(std::clamp(first, 0, std::max(0, count() - 1)));
}

void ErrorCheckDialog::showProblem(int index)
{
    m_current = index;
    m_title->setText(tr("Cell %1 (%2 of %3)").arg(m_cells.value(index)).arg(index + 1).arg(count()));
    m_text->setText(m_messages.value(index));
    m_fix->setText(m_fixes.value(index));
    m_fix->setVisible(!m_fixes.value(index).isEmpty());
    m_previous->setEnabled(count() > 1);
    m_next->setEnabled(count() > 1);
    Q_EMIT showing(index);
}

} // namespace jp
