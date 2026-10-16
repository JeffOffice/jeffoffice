#pragma once
// The Format pane: a task pane on the right that shows the formatting of the selected objects (a fill, a line, effects, size and place, the
// options of the text in a shape, a picture's corrections, and alt text) and changes it. It is built once for every app with pages: an
// app puts it in a dock and decides when it opens. It follows the selection, shows what the objects don't agree on as blank, and each
// change is one step that undoes, made through the Editor like every other change (a number dragged or a slider moved is one step, taken
// when it is let go).

#include "core/items.h"

#include <QWidget>
#include <functional>

namespace jp {

class Editor;
class PaneFrame;

// The change in progress. A control that is let go of at once (a check box, a color picked) changes the objects and ends the step; one
// that is dragged changes them again and again with `last` false, and ends the step with finish().
class FormatSession : public QObject {
    Q_OBJECT
public:
    explicit FormatSession(Editor *ed, QObject *parent = nullptr);
    Editor *editor() const { return m_ed; }
    // The objects selected, as they stand now (a table whose cells are selected is the table).
    QVector<Item *> items() const;
    // The same, with each group opened into the objects it holds (a fill or a line is its children's).
    QVector<Item *> leaves() const;
    enum class Scope { Selected, Leaves };
    QVector<Item *> targets(Scope scope) const { return scope == Scope::Leaves ? leaves() : items(); }
    // Runs `change` on every object of the scope that is not locked (or on all of them, `locked`) as part of the step in progress, starting one
    // named `label` if none is; with `last` it ends there.
    void apply(const QString &label, const std::function<void(Item &)> &change, Scope scope = Scope::Selected, bool last = true, bool locked = true);
    // Ends the step in progress (the control that was dragged was let go of).
    void finish();
    bool isOpen() const { return m_open; }
    // Pages enroll the function that sets their controls from the objects; refresh() calls them all.
    void enroll(const std::function<void()> &refresh) { m_refresh << refresh; }
    void refresh();

private:
    Editor *m_ed;
    bool m_open = false;
    QVector<std::function<void()>> m_refresh;
};

class FormatPane : public QWidget {
    Q_OBJECT
public:
    // The pages, in the order of the tabs.
    enum class Tab { FillLine, Effects, SizeProperties, Picture };
    explicit FormatPane(Editor *ed, QWidget *parent = nullptr);
    ~FormatPane() override;
    Editor *editor() const { return m_ed; }
    PaneFrame *frame() const { return m_frame; }
    FormatSession *session() const { return m_session; }
    // "Format Shape", or "Format Picture" while only pictures are selected.
    QString title() const;
    bool hasSelection() const { return !m_session->items().isEmpty(); }
    Tab currentTab() const;
    // Opens a page; a tab that is not shown for what is selected is not opened.
    void showTab(Tab tab);
    bool isTabShown(Tab tab) const;
    // The controls again from the objects (they follow the selection and every change while the pane is open on the screen).
    void reload();
    // Picks the file for a picture or texture fill and for Change Picture: asks with the file dialog unless this stands in for it (tests).
    void setPictureChooser(const std::function<QString()> &choose) { m_pictureChooser = choose; }
    QString choosePicture();

Q_SIGNALS:
    void closeRequested();
    void escaped();

protected:
    void showEvent(QShowEvent *e) override;

private:
    void follow();
    Editor *m_ed;
    PaneFrame *m_frame;
    FormatSession *m_session;
    std::function<QString()> m_pictureChooser;
};

} // namespace jp
