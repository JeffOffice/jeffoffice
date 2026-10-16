#include "app/masterdialogs.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

namespace jp {

MasterLayoutDialog::MasterLayoutDialog(const deck::MasterParts &p, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Master Layout"));
    setObjectName(QStringLiteral("masterLayoutDialog"));
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel(tr("Places on the slide master")));
    auto box = [&](const QString &name, const QString &text, bool on) {
        auto *b = new QCheckBox(text);
        b->setObjectName(name);
        b->setChecked(on);
        lay->addWidget(b);
        return b;
    };
    m_title = box(QStringLiteral("title"), tr("Title"), p.title);
    m_text = box(QStringLiteral("text"), tr("Text"), p.text);
    m_date = box(QStringLiteral("date"), tr("Date"), p.date);
    m_number = box(QStringLiteral("slideNumber"), tr("Slide number"), p.number);
    m_footer = box(QStringLiteral("footer"), tr("Footer"), p.footer);
    lay->addSpacing(6);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(buttons);
}

deck::MasterParts MasterLayoutDialog::parts() const
{
    deck::MasterParts p;
    p.title = m_title->isChecked();
    p.text = m_text->isChecked();
    p.date = m_date->isChecked();
    p.number = m_number->isChecked();
    p.footer = m_footer->isChecked();
    return p;
}

} // namespace jp
