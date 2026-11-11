#pragma once
// The Accessibility Checker of an app with pages (JeffPub and JeffDeck): the shared list, filled from checkPageAccessibility, where a problem that is clicked goes to its page and
// selects its object.

#include "app/accessibilitypane.h"
#include "app/selectionpane.h"
#include "app/tableedit.h"
#include "core/pageaccess.h"

#include <QPointer>

namespace jp {

class Editor;


class PageAccessibilityPane : public AccessibilityPane {
    Q_OBJECT
public:
    // `titleOf` (may be empty): the title of a slide, for an app whose pages are slides.
    PageAccessibilityPane(Editor *editor, bool slides, const std::function<QString(int)> &titleOf, QWidget *parent);
    // Looks again at the document and fills the list.
    void recheck();
    QVector<PageAccessIssue> issues() const;
    // The Reading Order list (for tests, and to know it is open): null until it has been shown.
    SelectionPane *readingOrderPane() const { return m_reading; }
    void showReadingOrder();
    // Where the table styles come from, for the fix that turns a table's header row on (an app with its own styles, JeffDeck, gives them; none: JeffPub's formats).
    void setTableStyles(const std::function<TableStyles()> &styles) { m_styles = styles; }

private:
    Editor *m_ed;
    bool m_slides;
    std::function<QString(int)> m_titleOf;
    std::function<TableStyles()> m_styles;
    QPointer<SelectionPane> m_reading;
};

} // namespace jp
