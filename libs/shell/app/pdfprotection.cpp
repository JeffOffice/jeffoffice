#include "app/pdfprotection.h"

#include "app/passworddialogs.h"
#include "app/plaintext.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

PdfProtectionRow::PdfProtectionRow(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("pdfProtectionRow"));
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_button = new QPushButton(QCoreApplication::translate("jp::PdfProtection", "Encrypt with a password…"), this);
    m_button->setObjectName(QStringLiteral("pdfEncryptButton"));
    connect(m_button, &QPushButton::clicked, this, &PdfProtectionRow::ask);
    layout->addWidget(m_button, 0, Qt::AlignLeft);
    m_note = plainLabel(new QLabel(this));
    m_note->setObjectName(QStringLiteral("pdfEncryptNote"));
    m_note->setWordWrap(true);
    layout->addWidget(m_note);
    refresh();
}

PdfProtectionRow::~PdfProtectionRow() { m_protection.wipe(); }

void PdfProtectionRow::forbiddenBy(const QList<QAbstractButton *> &boxes, const QString &why)
{
    m_boxes = boxes;
    m_why = why;
    refresh();
}

void PdfProtectionRow::setProtection(const PdfProtection &protection)
{
    m_protection.wipe();
    m_protection = protection.copy();
    refresh();
    Q_EMIT protectionChanged(isProtected());
}

void PdfProtectionRow::clear()
{
    if (!isProtected()) return;
    m_protection.wipe();
    refresh();
    Q_EMIT protectionChanged(false);
}

void PdfProtectionRow::ask()
{
    PdfProtection chosen = m_protection.copy();
    switch (askPdfProtection(window(), isProtected(), &chosen)) {
    case NewPasswordChoice::Set:
        m_protection.wipe();
        m_protection = chosen.copy();
        break;
    case NewPasswordChoice::Removed:
        m_protection.wipe();
        break;
    case NewPasswordChoice::Cancelled: break;
    }
    chosen.wipe();
    refresh();
    Q_EMIT protectionChanged(isProtected());
}

void PdfProtectionRow::refresh()
{
    const bool on = isProtected();
    QString note;
    if (on) {
        note = m_protection.limited ? QCoreApplication::translate("jp::PdfProtection", "The PDF will need a password to open, with limits on printing, copying, and changing.")
                                     : QCoreApplication::translate("jp::PdfProtection", "The PDF will need a password to open.");
        if (!m_why.isEmpty()) note += QLatin1Char(' ') + m_why;
    }
    m_note->setText(note);
    m_note->setVisible(on);
    for (QAbstractButton *box : m_boxes) {
        if (on) box->setChecked(false);   // (a button that is not a check box has nothing to untick)
        box->setEnabled(!on);
    }
}

} // namespace jp
