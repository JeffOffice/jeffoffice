#include "app/mediadialogs.h"

#include "app/mediaplayer.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include "app/plaintext.h"

namespace jp {

MediaTrimDialog::MediaTrimDialog(const QString &title, qint64 length, int cutStartMs, int cutEndMs, QWidget *parent) : QDialog(parent), m_length(length), m_cutEnd(cutEndMs)
{
    setObjectName(QStringLiteral("mediaTrimDialog"));
    setWindowTitle(title);
    auto *column = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    const double most = length > 0 ? length / 1000.0 : 86400.0;
    auto spin = [&](const char *name, const QString &accessible) {
        auto *s = new QDoubleSpinBox(this);
        s->setObjectName(QLatin1String(name));
        s->setRange(0, most);
        s->setDecimals(2);
        s->setSingleStep(0.1);
        s->setSuffix(tr(" s"));
        s->setKeyboardTracking(false);
        s->setAccessibleName(accessible);
        return s;
    };
    m_start = spin("trimStart", tr("Start Time"));
    m_end = spin("trimEnd", tr("End Time"));
    m_start->setValue(cutStartMs / 1000.0);
    // The end is where it plays to: the length less what is cut off.
    m_end->setValue(length > 0 ? std::max<qint64>(0, length - cutEndMs) / 1000.0 : 0);
    m_end->setEnabled(length > 0);
    if (length <= 0) m_end->setToolTip(tr("How long it is isn't known until it has been played, so the end can't be set yet."));
    form->addRow(tr("&Start Time:"), m_start);
    form->addRow(tr("&End Time:"), m_end);
    column->addLayout(form);
    m_note = plainLabel(new QLabel(this));
    m_note->setWordWrap(true);
    column->addWidget(m_note);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_ok = buttons->button(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    column->addWidget(buttons);
    connect(m_start, &QDoubleSpinBox::valueChanged, this, [this] { check(); });
    connect(m_end, &QDoubleSpinBox::valueChanged, this, [this] { check(); });
    check();
}

void MediaTrimDialog::check()
{
    const double start = m_start->value(), end = m_end->value();
    const bool known = m_length > 0;
    const bool bad = known && end <= start;
    m_ok->setEnabled(!bad);
    if (bad) m_note->setText(tr("The end has to be after the start."));
    else if (known) m_note->setText(tr("It plays for %1.").arg(MediaBar::timeText(qint64(std::round((end - start) * 1000)))));
    else m_note->setText(QString());
}

int MediaTrimDialog::cutStartMs() const { return int(std::lround(m_start->value() * 1000)); }

int MediaTrimDialog::cutEndMs() const
{
    if (m_length <= 0) return m_cutEnd;
    const qint64 cut = m_length - std::lround(m_end->value() * 1000);
    return cut <= 5 ? 0 : int(cut);
}

} // namespace jp
