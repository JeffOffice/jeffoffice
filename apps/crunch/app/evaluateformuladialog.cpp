#include "app/evaluateformuladialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

namespace jp {

EvaluateFormulaDialog::EvaluateFormulaDialog(const QString &cell, const QString &expression, crunch::FormulaSteps::Eval eval, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("evaluateFormulaDialog"));
    setWindowTitle(tr("Evaluate Formula"));
    m_steps = std::make_unique<crunch::FormulaSteps>(expression, std::move(eval));
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel(tr("Formula in cell %1. The underlined part is worked out next.").arg(cell), this);
    title->setTextFormat(Qt::PlainText);
    title->setWordWrap(true);
    layout->addWidget(title);
    m_box = new QTextEdit(this);
    m_box->setObjectName(QStringLiteral("evaluateBox"));
    m_box->setReadOnly(true);
    m_box->setAcceptRichText(false);
    m_box->setMinimumSize(380, 70);
    layout->addWidget(m_box);
    m_note = new QLabel(this);
    m_note->setObjectName(QStringLiteral("evaluateNote"));
    m_note->setTextFormat(Qt::PlainText);
    m_note->setWordWrap(true);
    layout->addWidget(m_note);
    auto *buttons = new QDialogButtonBox(this);
    m_evaluate = buttons->addButton(tr("Evaluate"), QDialogButtonBox::ActionRole);
    m_evaluate->setObjectName(QStringLiteral("evaluateStep"));
    auto *restart = buttons->addButton(tr("Restart"), QDialogButtonBox::ActionRole);
    restart->setObjectName(QStringLiteral("evaluateRestart"));
    auto *close = buttons->addButton(tr("Close"), QDialogButtonBox::RejectRole);
    connect(close, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_evaluate, &QPushButton::clicked, this, [this] {
        constexpr int kMaxSteps = 500;
        if (++m_taken > kMaxSteps) return;
        m_steps->step();
        refresh();
    });
    connect(restart, &QPushButton::clicked, this, [this] {
        m_taken = 0;
        m_steps->restart();
        refresh();
    });
    layout->addWidget(buttons);
    refresh();
}

void EvaluateFormulaDialog::refresh()
{
    m_box->setPlainText(QLatin1Char('=') + m_steps->text());
    if (m_steps->underlineLength() > 0) {
        QTextCursor c(m_box->document());
        c.setPosition(1 + m_steps->underlineStart());
        c.setPosition(1 + m_steps->underlineStart() + m_steps->underlineLength(), QTextCursor::KeepAnchor);
        QTextCharFormat f;
        f.setFontUnderline(true);
        c.mergeCharFormat(f);
    }
    m_evaluate->setEnabled(!m_steps->finished() && m_steps->underlineLength() > 0 && m_taken < 500);
    m_note->setText(!m_steps->problem().isEmpty() ? m_steps->problem() : m_steps->finished() ? tr("This is the value of the formula. Restart to go through it again.") : QString());
}

} // namespace jp
