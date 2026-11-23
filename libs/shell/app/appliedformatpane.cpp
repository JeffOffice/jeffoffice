#include "app/appliedformatpane.h"

#include "app/plaintext.h"

#include <QCoreApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

AppliedFormatPane::AppliedFormatPane(const QStringList &titles, const QStringList &buttons, QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("appliedFormatPane"));
    auto *v = new QVBoxLayout(this);
    for (int i = 0; i < titles.size(); ++i) {
        Section s;
        auto *title = plainLabel(new QLabel(titles[i], this));
        QFont bold = title->font();
        bold.setBold(true);
        title->setFont(bold);
        v->addWidget(title);
        s.name = plainLabel(new QLabel(this));
        s.name->setObjectName(QStringLiteral("appliedName%1").arg(i));
        s.name->setWordWrap(true);
        s.name->setTextInteractionFlags(Qt::TextSelectableByMouse);
        v->addWidget(s.name);
        s.extras = plainLabel(new QLabel(this));
        s.extras->setObjectName(QStringLiteral("appliedExtras%1").arg(i));
        s.extras->setWordWrap(true);
        s.extras->setTextInteractionFlags(Qt::TextSelectableByMouse);
        v->addWidget(s.extras);
        s.button = new QPushButton(i < buttons.size() ? buttons[i] : QString(), this);
        s.button->setObjectName(QStringLiteral("appliedClear%1").arg(i));
        connect(s.button, &QPushButton::clicked, this, [this, i] { Q_EMIT clearRequested(i); });
        v->addWidget(s.button);
        m_sections.push_back(s);
    }
    v->addStretch(1);
}

void AppliedFormatPane::setSection(int i, const QString &name, const QStringList &extras, bool canClear, const QString &none)
{
    if (i < 0 || i >= m_sections.size()) return;
    const Section &s = m_sections[i];
    s.name->setText(name.isEmpty() ? none : name);
    s.extras->setText(extras.isEmpty() ? QString() : QCoreApplication::translate("jp::AppliedFormatPane", "Plus: %1").arg(extras.join(QStringLiteral(", "))));
    s.extras->setVisible(!extras.isEmpty());
    s.button->setEnabled(canClear);
}

QString AppliedFormatPane::nameOf(int i) const { return i >= 0 && i < m_sections.size() ? m_sections[i].name->text() : QString(); }

QString AppliedFormatPane::extrasOf(int i) const { return i >= 0 && i < m_sections.size() && !m_sections[i].extras->isHidden() ? m_sections[i].extras->text() : QString(); }

} // namespace jp
