#pragma once
// The mini toolbar: a small bar of the common character and paragraph commands that floats by words selected with the mouse. It holds
// no commands of its own: a formatting target (an app's window) says which it offers, and each button is that window's own action, so the
// bar does what the ribbon does. It never takes the keyboard (the text keeps it).

#include "core/export.h"
#include "core/color.h"
#include <QFrame>
#include <QList>
#include <functional>
#include <memory>

class QAbstractScrollArea;
class QAction;

namespace jp {

class ColorButton;
class FontCombo;

// What a window offers the mini toolbar. A command with no action is left off the bar.
class JO_COMMON_EXPORT MiniTarget {
public:
    enum class Command { Bold, Italic, Underline, Shrink, Grow, AlignLeft, AlignCenter, AlignRight, Bullets, Numbering };
    virtual ~MiniTarget() = default;
    virtual QAction *action(Command c) const = 0;
    virtual bool supportsFont() const { return true; }
    virtual bool supportsColor() const { return true; }
    virtual void chooseFont(const QString &family) = 0;
    virtual void chooseColor(const ColorRef &c) = 0;
    virtual QList<QAction *> colorExtras() const { return {}; }   // more choices under the color button (as the ribbon's has)
};

// A target made of functions, for a window that would rather not be one.
class JO_COMMON_EXPORT FunctionMiniTarget : public MiniTarget {
public:
    std::function<QAction *(Command)> actionFor;
    std::function<void(const QString &)> setFont;
    std::function<void(const ColorRef &)> setColor;
    QList<QAction *> extras;
    QAction *action(Command c) const override { return actionFor ? actionFor(c) : nullptr; }
    QList<QAction *> colorExtras() const override { return extras; }
    bool supportsFont() const override { return bool(setFont); }
    bool supportsColor() const override { return bool(setColor); }
    void chooseFont(const QString &family) override { if (setFont) setFont(family); }
    void chooseColor(const ColorRef &c) override { if (setColor) setColor(c); }
};

class JO_COMMON_EXPORT MiniToolbar : public QFrame {
    Q_OBJECT
public:
    // `host` is the widget the bar floats in (a view's viewport); the bar keeps `target`.
    MiniToolbar(QWidget *host, std::unique_ptr<MiniTarget> target);
    FontCombo *fontBox() const { return m_font; }
    ColorButton *colorButton() const { return m_color; }
    // Shown above the first line of a selection (`first` is where it starts and `last` where it ends, in the host's coordinates), below the last
    // line when there is no room above; kept inside the host.
    void showBeside(const QRect &first, const QRect &last);
    // Hidden by typing, by Escape, by the area changing size, and by scrolling it (the wheel, and any move of a scroll bar: a hand-tool pan too).
    void hideOnTypingAndScrolling(QAbstractScrollArea *area);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    FontCombo *m_font = nullptr;
    ColorButton *m_color = nullptr;
    std::unique_ptr<MiniTarget> m_target;
};

} // namespace jp
