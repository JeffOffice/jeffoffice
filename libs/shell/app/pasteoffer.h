#pragma once
// The Paste Options button: after a paste, a small button by what was pasted offers the other ways the same clipboard can go in. The button, its menu, the key that opens it (the Ctrl key tapped
// by itself), and the offer going away when the paste is no longer the last thing done are here, once, for every app; what the choices are and how a choice takes the paste back and does it again
// (as one step of Undo) is the app's own. The app says when the offer is made (showAt) and when it is over (hide); it never holds the button.

#include "core/export.h"

#include <QObject>
#include <QPointer>
#include <QRect>
#include <QString>
#include <QToolButton>
#include <QVector>
#include <functional>

class QKeyEvent;
class QWidget;

namespace jp {

// One choice of the menu: `name` is the action's object name, `run` does it; a separator goes before it when asked.
struct PasteChoice {
    QString text, name;
    std::function<void()> run;
    bool separatorBefore = false;
};

class JO_COMMON_EXPORT PasteOffer : public QObject {
    Q_OBJECT
public:
    using Choices = std::function<QVector<PasteChoice>()>;
    // `host` is the widget the button floats over (the viewport of a scroll area), and `focus` gets the keyboard back after a choice. `choices` is asked when the menu opens.
    PasteOffer(QWidget *host, QWidget *focus, const Choices &choices);
    ~PasteOffer() override;

    // What a screen reader says of the button (it names the key; the app words it).
    void setDescription(const QString &text);
    // Shows the button just below and right of `anchor` (the pasted text's end, or the pasted cells' last cell), in the host's pixels; an empty anchor (scrolled out of sight) hides it.
    void showAt(const QRect &anchor);
    void hide();
    bool isShown() const;
    // While `busy` says true (the paste is still being put in, a step is under way) the button does not offer its choices: a click, the key, and the menu do nothing, and the button is dimmed;
    // `refreshBusy` says the answer may have changed.
    void setBusyCheck(const std::function<bool()> &busy) { m_busy = busy; }
    void refreshBusy();
    bool isBusy() const { return m_busy && m_busy(); }
    QToolButton *button() const { return m_button; }
    // Pops the menu up by the button (the arrow keys and Enter choose); false when the button is not shown.
    bool openMenu();

    // The Ctrl key tapped by itself opens the menu: the key's press (`live`: there is an offer), anything between (a shortcut, a mouse press, the window losing the keyboard), and its release.
    void keyPressed(const QKeyEvent *e, bool live);
    void shortcutOverride(const QKeyEvent *e);
    void cancelTap() { m_tap = false; }
    // True when this release ends a tap (the tap is used up either way).
    bool wasTap(const QKeyEvent *e);
    // True when the release was a tap and the menu opened.
    bool keyReleased(const QKeyEvent *e);
    // The same, as an event filter on the widget that has the keyboard (an app that has no key handling of its own to put it in): Esc, too, says `escaped`. For a scroll area the filter is put on
    // its viewport as well, where the mouse and wheel events go (a click or the wheel between the key's press and its release is no tap).
    void installOn(QWidget *focus);

    // The key that opens it: Ctrl (the key by that name on a Mac is Meta to Qt), and the name to say; a test may set another.
    static int key();
    static QString keyName();
    static void setKeyForTest(int key);

Q_SIGNALS:
    void opened();     // the menu was opened by the key
    void escaped();    // Esc was pressed while the offer was shown

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void fill();
    QPointer<QToolButton> m_button;
    QPointer<QWidget> m_host, m_focus;
    QString m_description;
    Choices m_choices;
    std::function<bool()> m_busy;
    bool m_tap = false;   // the key went down and nothing else since
};

} // namespace jp
