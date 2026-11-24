#pragma once
// The Quick Analysis button and its gallery: when two or more cells with data are selected, a small button by the selection's last corner opens a compact box of tabs (formatting, charts,
// totals, tables, sparklines). A choice looked at (the pointer over it, or the arrow keys) is told as `previewed`, and is shown by the window without a trace (`previewEnded` when the look
// is over); a choice made is told as `chosen`. What the choices are and what each does is the window's own.

#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QPainter>
#include <QString>
#include <QVector>
#include <QWidget>
#include <functional>

class QButtonGroup;
class QFrame;
class QToolButton;
class QWidget;

namespace jp {

struct QuickChoice {
    QString id, text, tip;
    QIcon icon;
    bool enabled = true;
};

struct QuickTab {
    QString id, title;
    QVector<QuickChoice> choices;
};

class QuickAnalysis : public QObject {
    Q_OBJECT
public:
    using Tabs = std::function<QVector<QuickTab>()>;
    // `host` is the widget the button floats over (the grid's viewport), and `focus` gets the keyboard back when the box closes. `tabs` is asked when the box opens.
    QuickAnalysis(QWidget *host, QWidget *focus, const Tabs &tabs);
    ~QuickAnalysis() override;

    // The button just outside the selection's last corner, clear of the fill handle (`corner` is that cell's rectangle in the host's pixels, `handle` the fill handle's, possibly empty); an empty
    // corner (scrolled out of sight) hides it.
    void showAt(const QRect &corner, const QRect &handle);
    void hide();
    bool isShown() const;
    QToolButton *button() const { return m_button; }
    void setButtonIcon(const QIcon &icon);
    void setButtonToolTip(const QString &text);

    // Opens the box (by the button when it is shown, else by the host's top left); false when there is nothing to show.
    bool open();
    void close();
    bool isOpen() const;
    QFrame *box() const { return m_box; }
    // The tab shown, and the choice looked at (the ids; empty for none), for tests and the window.
    QString currentTab() const;
    QString currentChoice() const { return m_current; }
    // What the arrow keys and the pointer do: a choice looked at.
    void look(const QString &id);
    void choose(QString id);   // (by value: the look being ended clears the member it may name)
    void selectTab(const QString &id);

Q_SIGNALS:
    void previewed(const QString &id);
    void previewEnded();
    void chosen(const QString &id);
    void opened();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void build();
    void move(int dx, int dy);
    void showTab(int index);
    void endLook();
    QPointer<QToolButton> m_button;
    QPointer<QFrame> m_box;
    QPointer<QWidget> m_host, m_focus;
    QVector<QuickTab> m_tabs;
    QVector<QToolButton *> m_tabButtons;
    QVector<QVector<QToolButton *>> m_tiles;
    int m_tab = 0;
    QString m_current;
    Tabs m_tabsOf;
    bool m_closing = false;
};

// A clear sheet over the grid's viewport that a look at a choice is drawn on (the pointer goes through it, and the workbook is not touched): the window gives it what to draw, which it asks for
// the cells' places again at each paint, so the look follows scrolling and zoom.
class QuickPreviewLayer : public QWidget {
public:
    using Painter = std::function<void(QPainter &)>;
    explicit QuickPreviewLayer(QWidget *host);
    void showLook(const Painter &painter);
    void clear();
    bool isShowing() const { return bool(m_painter); }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    Painter m_painter;
};

} // namespace jp
