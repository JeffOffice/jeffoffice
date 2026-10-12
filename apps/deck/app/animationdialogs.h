#pragma once
// The dialog for one animation effect: its Effect tab (direction, amount,
// smooth start and end), its Timing tab (how it starts, its delay, length,
// repeats, and whether it goes back when it ends), and its Text Animation
// tab (the whole shape at once, or by paragraph). Effect Options opens it on
// the first, Timing on the second.

#include "core/animation.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QRadioButton;
class QTabWidget;

namespace jp {

class AnimationOptionsDialog : public QDialog {
    Q_OBJECT
public:
    enum Tab { EffectTab, TimingTab, TextTab };
    // `hasText`: the shape has text, so it can be animated by paragraph.
    AnimationOptionsDialog(const deck::Anim &anim, bool hasText, Tab tab, QWidget *parent = nullptr);
    // The effect as the dialog's settings have it.
    deck::Anim result() const;
    // The Text Animation tab's choice.
    bool byParagraph() const;

private:
    deck::Anim m_anim;
    bool m_byParagraph = false;
    QTabWidget *m_tabs = nullptr;
    QComboBox *m_variant = nullptr, *m_start = nullptr, *m_repeat = nullptr, *m_after = nullptr;
    QDoubleSpinBox *m_amount = nullptr, *m_delay = nullptr, *m_duration = nullptr;
    QCheckBox *m_smoothStart = nullptr, *m_smoothEnd = nullptr, *m_rewind = nullptr;
    QRadioButton *m_allAtOnce = nullptr, *m_byParagraphButton = nullptr;
};

} // namespace jp
