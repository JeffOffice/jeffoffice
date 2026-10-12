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

namespace jp {

MediaTrimDialog::MediaTrimDialog(const QString &title, qint64 length, int startMs, int endMs, QWidget *parent) : QDialog(parent), m_length(length)
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
    m_start->setValue(startMs / 1000.0);
    m_end->setValue(endMs > 0 ? endMs / 1000.0 : (length > 0 ? length / 1000.0 : 0));
    form->addRow(tr("&Start Time:"), m_start);
    form->addRow(tr("&End Time:"), m_end);
    column->addLayout(form);
    m_note = new QLabel(this);
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
    const bool toEnd = end <= 0 || (m_length > 0 && end * 1000 >= m_length - 5);
    const bool sound = !toEnd && end <= start;
    m_ok->setEnabled(!sound);
    if (sound) m_note->setText(tr("The end has to be after the start."));
    else m_note->setText(tr("It plays for %1.").arg(MediaBar::timeText(qint64(std::round(((toEnd ? (m_length > 0 ? m_length / 1000.0 : 0) : end) - start) * 1000)))));
}

int MediaTrimDialog::startMs() const { return int(std::lround(m_start->value() * 1000)); }

int MediaTrimDialog::endMs() const
{
    const double end = m_end->value();
    if (end <= 0 || (m_length > 0 && end * 1000 >= m_length - 5)) return 0;
    return int(std::lround(end * 1000));
}

} // namespace jp
