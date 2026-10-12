#include "app/animationdialogs.h"

#include "app/animationpane.h"
#include "app/appwindow.h"
#include "app/help.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QShortcut>
#include <QTabWidget>
#include <QVBoxLayout>

namespace jp {

AnimationOptionsDialog::AnimationOptionsDialog(const deck::Anim &anim, bool hasText, Tab tab, QWidget *parent) : QDialog(parent), m_anim(anim)
{
    setObjectName(QStringLiteral("animationOptionsDialog"));
    const deck::EffectKind *kind = anim.custom() ? nullptr : deck::effectKind(anim.kind);
    setWindowTitle(kind ? tr("%1 Options").arg(kind->name) : tr("Effect Options"));
    auto *lay = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName(QStringLiteral("animationOptionsTabs"));
    lay->addWidget(m_tabs);

    // Effect
    auto *effect = new QWidget();
    auto *form = new QFormLayout(effect);
    if (!kind) {
        auto *note = new QLabel(tr("This effect was made in another program. JeffDeck keeps it just as it is, and can move it, remove it, and start it another way."));
        note->setWordWrap(true);
        note->setMinimumWidth(320);
        form->addRow(note);
    } else {
        form->addRow(new QLabel(tr("Settings")));
        if (kind->variants.size() > 1) {
            m_variant = new QComboBox();
            m_variant->setObjectName(QStringLiteral("animVariant"));
            m_variant->setAccessibleName(tr("Direction"));
            for (const deck::EffectVariant &v : kind->variants) m_variant->addItem(v.name, v.key);
            m_variant->setCurrentIndex(std::max(0, m_variant->findData(anim.variant)));
            form->addRow(kind->cls == deck::EffectClass::MotionPath ? tr("Path:") : tr("Direction:"), m_variant);
        }
        if (kind->amount != deck::EffectKind::Amount::None) {
            m_amount = new QDoubleSpinBox();
            m_amount->setObjectName(QStringLiteral("animAmount"));
            const bool size = kind->amount == deck::EffectKind::Amount::Size;
            m_amount->setRange(size ? 1 : 1, size ? 1000 : 3600);
            m_amount->setDecimals(0);
            m_amount->setSingleStep(size ? 5 : 45);
            m_amount->setSuffix(size ? tr(" %") : tr(" degrees"));
            m_amount->setValue(anim.amount);
            m_amount->setAccessibleName(size ? tr("Size") : tr("Amount"));
            form->addRow(size ? tr("Size:") : tr("Amount:"), m_amount);
        }
        m_smoothStart = new QCheckBox(tr("Smooth start"));
        m_smoothStart->setObjectName(QStringLiteral("animSmoothStart"));
        m_smoothStart->setChecked(anim.smoothStart);
        m_smoothStart->setToolTip(tr("The effect speeds up as it begins"));
        m_smoothEnd = new QCheckBox(tr("Smooth end"));
        m_smoothEnd->setObjectName(QStringLiteral("animSmoothEnd"));
        m_smoothEnd->setChecked(anim.smoothEnd);
        m_smoothEnd->setToolTip(tr("The effect slows down as it ends"));
        form->addRow(m_smoothStart);
        form->addRow(m_smoothEnd);
        form->addRow(new QLabel(tr("Enhancements")));
        m_after = new QComboBox();
        m_after->setObjectName(QStringLiteral("animAfter"));
        m_after->setAccessibleName(tr("After animation"));
        m_after->addItem(tr("Don't Dim"), int(deck::AfterEffect::None));
        m_after->addItem(tr("Hide After Animation"), int(deck::AfterEffect::Hide));
        m_after->addItem(tr("Hide on Next Click"), int(deck::AfterEffect::HideOnNextClick));
        m_after->setCurrentIndex(m_after->findData(int(anim.after)));
        // An exit has hidden the shape already.
        m_after->setEnabled(kind->cls != deck::EffectClass::Exit);
        m_after->setToolTip(tr("After animation: what happens to the shape when the effect is over"));
        form->addRow(tr("After animation:"), m_after);
    }
    m_tabs->addTab(effect, tr("Effect"));

    // Timing
    auto *timing = new QWidget();
    auto *tform = new QFormLayout(timing);
    m_start = new QComboBox();
    m_start->setObjectName(QStringLiteral("animStart"));
    m_start->setAccessibleName(tr("Start"));
    for (deck::EffectStart s : {deck::EffectStart::OnClick, deck::EffectStart::WithPrevious, deck::EffectStart::AfterPrevious}) m_start->addItem(startLabel(s), int(s));
    m_start->setCurrentIndex(m_start->findData(int(anim.start)));
    tform->addRow(tr("Start:"), m_start);
    m_delay = new QDoubleSpinBox();
    m_delay->setObjectName(QStringLiteral("animDelay"));
    m_delay->setRange(0, 59);
    m_delay->setDecimals(2);
    m_delay->setSingleStep(0.25);
    m_delay->setSuffix(tr(" s"));
    m_delay->setValue(anim.delay / 1000.0);
    m_delay->setAccessibleName(tr("Delay"));
    m_delay->setEnabled(kind != nullptr);
    tform->addRow(tr("Delay:"), m_delay);
    m_duration = new QDoubleSpinBox();
    m_duration->setObjectName(QStringLiteral("animDuration"));
    m_duration->setRange(0.01, 59);
    m_duration->setDecimals(2);
    m_duration->setSingleStep(0.25);
    m_duration->setSuffix(tr(" s"));
    m_duration->setValue(anim.duration > 0 ? anim.duration / 1000.0 : 0.01);
    m_duration->setAccessibleName(tr("Duration"));
    m_duration->setEnabled(kind && kind->duration > 0);
    tform->addRow(tr("Duration:"), m_duration);
    m_repeat = new QComboBox();
    m_repeat->setObjectName(QStringLiteral("animRepeat"));
    m_repeat->setAccessibleName(tr("Repeat"));
    m_repeat->addItem(tr("(none)"), 1);
    for (int n : {2, 3, 4, 5, 10}) m_repeat->addItem(QString::number(n), n);
    int found = m_repeat->findData(int(anim.repeat));
    if (found < 0) {
        m_repeat->addItem(QString::number(anim.repeat), int(anim.repeat));
        found = m_repeat->count() - 1;
    }
    m_repeat->setCurrentIndex(found);
    m_repeat->setEnabled(kind != nullptr && (kind->duration > 0));
    tform->addRow(tr("Repeat:"), m_repeat);
    m_rewind = new QCheckBox(tr("Rewind when done playing"));
    m_rewind->setObjectName(QStringLiteral("animRewind"));
    m_rewind->setChecked(anim.rewind);
    m_rewind->setEnabled(kind != nullptr);
    tform->addRow(m_rewind);
    m_tabs->addTab(timing, tr("Timing"));

    // Text Animation
    auto *text = new QWidget();
    auto *column = new QVBoxLayout(text);
    column->addWidget(new QLabel(tr("Group text")));
    m_allAtOnce = new QRadioButton(tr("All at once"));
    m_allAtOnce->setObjectName(QStringLiteral("animAllAtOnce"));
    m_byParagraphButton = new QRadioButton(tr("By paragraph"));
    m_byParagraphButton->setObjectName(QStringLiteral("animByParagraph"));
    m_byParagraphButton->setChecked(anim.paragraphs());
    m_allAtOnce->setChecked(!anim.paragraphs());
    const bool textTab = hasText && kind != nullptr;
    m_allAtOnce->setEnabled(textTab);
    m_byParagraphButton->setEnabled(textTab);
    column->addWidget(m_allAtOnce);
    column->addWidget(m_byParagraphButton);
    if (!textTab) {
        auto *note = new QLabel(kind ? tr("This shape has no text.") : tr("This effect keeps the way it was made."));
        note->setWordWrap(true);
        column->addWidget(note);
    }
    column->addStretch(1);
    m_tabs->addTab(text, tr("Text Animation"));
    m_tabs->setCurrentIndex(tab);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(buttons);
    // F1 opens Help on the dialog, in a window over it (the dialog keeps the main window from its Help pane).
    auto *helpKey = new QShortcut(QKeySequence::HelpContents, this);
    connect(helpKey, &QShortcut::activated, this, [this] {
        auto *win = qobject_cast<AppWindow *>(parentWidget() ? parentWidget()->window() : nullptr);
        if (!win) return;
        auto *w = findChild<HelpWindow *>(QString(), Qt::FindDirectChildrenOnly);
        if (!w) w = new HelpWindow(win, this);
        const QString topic = help::contextTopic(QStringLiteral("dialog:") + (m_tabs->currentIndex() == TimingTab ? QStringLiteral("Timing") : QStringLiteral("Effect Options")));
        w->view()->showTopic(topic.isEmpty() ? QStringLiteral("index") : topic);
        w->show();
        w->raise();
        w->activateWindow();
        w->view()->focusTopic();
    });
}

deck::Anim AnimationOptionsDialog::result() const
{
    deck::Anim a = m_anim;
    a.start = deck::EffectStart(m_start->currentData().toInt());
    if (a.custom()) return a;
    if (m_variant) a.variant = m_variant->currentData().toString();
    if (m_amount) a.amount = m_amount->value();
    a.smoothStart = m_smoothStart->isChecked();
    a.smoothEnd = m_smoothEnd->isChecked();
    a.delay = qRound(m_delay->value() * 1000);
    if (m_duration->isEnabled()) a.duration = qRound(m_duration->value() * 1000);
    if (m_repeat->isEnabled()) a.repeat = m_repeat->currentData().toInt();
    a.rewind = m_rewind->isChecked();
    if (m_after && m_after->isEnabled()) a.after = deck::AfterEffect(m_after->currentData().toInt());
    return a;
}

bool AnimationOptionsDialog::byParagraph() const { return m_byParagraphButton->isChecked(); }

} // namespace jp
