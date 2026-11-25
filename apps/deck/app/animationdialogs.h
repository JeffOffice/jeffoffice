#pragma once
// The dialog for one animation effect: its Effect tab (direction, amount,
// smooth start and end), its Timing tab (how it starts, its delay, length,
// repeats, and whether it goes back when it ends), and its Text Animation
// tab (the whole shape at once, or by paragraph). Effect Options opens it on
// the first, Timing on the second.

#include "core/animation.h"
#include "core/color.h"

#include <QDialog>
#include <functional>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QRadioButton;
class QTabWidget;

namespace jp {
class ColorButton;
}

namespace jp {

class AnimationOptionsDialog : public QDialog {
    Q_OBJECT
public:
    enum Tab { EffectTab, TimingTab, TextTab };
    // `hasText`: the shape has text, so it can be animated by paragraph.
    // `scheme`: the presentation's theme colors, for the color a shape dims to.
    AnimationOptionsDialog(const deck::Anim &anim, bool hasText, Tab tab, QWidget *parent = nullptr, const ColorScheme &scheme = ColorScheme());
    // How "Other Sound…" picks a file: it asks the person, keeps the file in the presentation, and fills in the sound of the effect it is given; false when nothing was picked.
    void setSoundChooser(const std::function<bool(deck::Anim *)> &chooser) { m_chooser = chooser; }
    // The effect as the dialog's settings have it.
    deck::Anim result() const;
    // The Text Animation tab's choice.
    bool byParagraph() const;

private:
    deck::Anim m_anim;
    bool m_byParagraph = false;
    QTabWidget *m_tabs = nullptr;
    ColorButton *m_dimColor = nullptr;
    QComboBox *m_sound = nullptr;
    std::function<bool(deck::Anim *)> m_chooser;
    int m_soundShown = 0;
    QComboBox *m_variant = nullptr, *m_start = nullptr, *m_repeat = nullptr, *m_after = nullptr;
    QDoubleSpinBox *m_amount = nullptr, *m_delay = nullptr, *m_duration = nullptr;
    QCheckBox *m_smoothStart = nullptr, *m_smoothEnd = nullptr, *m_rewind = nullptr;
    QRadioButton *m_allAtOnce = nullptr, *m_byParagraphButton = nullptr;
    QComboBox *m_textBuild = nullptr;
    QDoubleSpinBox *m_textDelay = nullptr;
};

} // namespace jp
