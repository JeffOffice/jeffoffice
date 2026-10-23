#pragma once
// The view of the document's pages: a continuous column of pages on a gray ground, or several across, drawn at a zoom of 10% to 500%.
// Only the pages in view are made (the paginator keeps where each starts, and makes a page when it is asked for) and the rest of the
// document is laid out in the background, so the first page shows at once, and the number of pages grows while a long document is
// worked out. The view also takes the keyboard and the mouse: the caret is drawn on the page that holds it, and what is typed goes to
// the story through the commands of core/editops.h.

#include "core/writedoc.h"

#include <QAbstractScrollArea>
#include <QPixmap>
#include <QTextCursor>
#include <QTimer>
#include <memory>

class QMenu;
class QTextDocument;

namespace jp {
class LayoutCache;
}

namespace jp::write {

class SpellCheck;

class PageView : public QAbstractScrollArea {
    Q_OBJECT
public:
    enum class ZoomMode { Percent, PageWidth, OnePage, ManyPages };

    explicit PageView(QWidget *parent = nullptr);
    ~PageView() override;

    // The document shown (not owned; it must outlast the view or be taken away with null first).
    void setDocument(WriteDoc *doc);
    WriteDoc *document() const { return m_doc; }
    QTextCursor &cursor() { return m_cursor; }
    const QTextCursor &cursor() const { return m_cursor; }
    void setCursor(const QTextCursor &c);
    // Puts the cursor (a selection, as a search finds it) where `c` is and shows it: the pages are laid out as far as it if they are not yet, and the window
    // scrolls to put it in the middle when it was out of the window.
    void showCursor(const QTextCursor &c);

    // ---- zoom: 1 is 100%, from 0.1 to 5
    static constexpr double kLeastZoom = 0.1, kMostZoom = 5.0;
    double zoom() const { return m_zoom; }
    ZoomMode zoomMode() const { return m_mode; }
    void setZoom(double zoom);
    // Page Width fits the page to the width of the window, One Page the whole page in the window, Many Pages `across` pages by `down`.
    void setZoomMode(ZoomMode mode, int across = 2, int down = 1);
    int pagesAcross() const { return m_across; }

    // ---- pages
    // The number of pages a page made for the view shows (NUMPAGES), -1 where the page isn't made or shows none: what the page says, for the tests.
    int shownTotal(int index) const;
    int pageCount() const;         // the pages laid out so far
    bool pagesComplete() const;    // whether those are all the document has
    int currentPage() const;       // the page the caret is on, from 0
    int topPage() const;           // the page at the top of the window
    void goToPage(int index);
    // The pages that are drawn: the rectangle of page `index` in the window.
    QRect pageRect(int index) const;
    // A page made into a picture `width` pixels wide, as printed (for tests and thumbnails).
    QImage pageImage(int index, int width);
    // The caret: where it is in the window, if it is on a page that is shown.
    bool caretRect(QRect *rect);
    // Draws the caret while the view has not the keyboard (for screenshots).
    void setCaretWithoutFocus(bool on) { m_caretWithoutFocus = on; viewport()->update(); }

    // ---- showing
    void setShowMarks(bool on);
    bool showMarks() const { return m_marks; }
    // Spelling as you type: misspelled words get a red wave on the screen. The checking is the document's own, found a paragraph at a time for the pages in view.
    // (The check of what is being edited: the header's or footer's while one is, else the text's.)
    SpellCheck *editedSpelling() const { return m_hf.active && m_hfSpell ? m_hfSpell.get() : m_spell.get(); }
    SpellCheck *spelling() const { return m_spell.get(); }
    bool spellingEnabled() const { return m_spellOn; }
    // The place a menu is for: where the last right click was (a click inside a selection leaves the selection and the caret as they were), else the caret.
    int contextPosition() const { return m_contextAt >= 0 ? m_contextAt : m_cursor.position(); }
    void setSpellingEnabled(bool on);

    // ---- editing (each is a step that undoes)
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void insertText(const QString &text);
    void typeKey(int key, Qt::KeyboardModifiers modifiers);   // the keys that edit and move, as the keyboard sends them

    // ---- headers and footers
    // A header or footer being edited: the text of the page dims, the caret and every edit are in its story, and Esc, a double click in the text of a page, or leaveHeaderFooter ends it.
    struct HeaderFooterEdit {
        bool active = false;
        int section = 0, slot = 0, page = 0;   // the section and slot it is of, and the page it is edited on
        bool footer = false;
        QTextDocument *story = nullptr;
    };
    // The part of a page a point is in: the text, the area of the header above it, or the area of the footer below it.
    enum class Area { Body, Header, Footer };
    bool inHeaderFooter() const { return m_hf.active; }
    const HeaderFooterEdit &headerFooterEdit() const { return m_hf; }
    // Edits the header (footer) a page uses: its section's, in the slot that the page's number and place in the section choose. A blank one is made where there is none.
    bool enterHeaderFooter(int page, bool footer);
    bool enterHeaderFooterFor(int section, bool footer, int slot, int page);
    bool enterHeaderFooterAtCaret(bool footer);   // for the page the caret is on
    void leaveHeaderFooter();
    // After a change to the sections or to their options: the header (footer) being edited is the one that its page uses now (else the one that is, made blank if `create`, else editing ends).
    void refreshHeaderFooter(bool create);
    Area areaAt(const QPoint &viewportPos);

    // Lays out every page now (for the count).
    void paginateAll();
    // The pages the window is set to show, laid out and drawn once more.
    void refresh();

Q_SIGNALS:
    void zoomChanged(double zoom);
    void pagesChanged();                    // how many pages there are, or whether all are known
    void caretChanged();                    // the caret moved or the selection changed
    void contentChanged();                  // the text was edited
    void contextMenuWanted(const QPoint &globalPos);
    void headerFooterModeChanged();         // a header or footer is edited now, or none is any more

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void scrollContentsBy(int dx, int dy) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void inputMethodEvent(QInputMethodEvent *e) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery q) const override;
    bool focusNextPrevChild(bool next) override;

private:
    struct Rendered;
    struct Hit {
        int page = -1;
        QPointF point;       // on the page, in points
    };
    std::shared_ptr<Rendered> rendered(int index);
    void dropRendered(int fromPage);
    void relayout();
    void applyZoomMode();
    void setScrollRanges();
    Hit hitAt(const QPoint &viewportPos);
    int positionAt(const Hit &hit);
    bool caretLine(int position, int *page, QLineF *line);   // in points on the page
    // The text box of a frame on a drawn page that holds a position of the story (a frame's paragraphs have no lines in the page's text), its layout, and the position in its own text.
    struct FrameSpot;
    bool frameSpotAt(Rendered &r, int position, FrameSpot *spot);
    int moveVertical(int position, int direction);
    // The same, for the header or footer that is being edited: where its boxes on the page being edited put a position, and which position a point is at.
    bool hfCaretLine(int position, int *page, QLineF *line);
    bool hfSpotAt(Rendered &r, int position, FrameSpot *spot);
    int hfPositionAt(const Hit &hit);
    int hfLineEdge(int position, bool end);
    int hfMoveVertical(int position, int direction);
    Area areaOf(const Hit &hit);
    // The header (footer) clicked: the one being edited, or another that the click goes to.
    bool takeFurnitureAt(const Hit &hit, bool footer);
    int pageShowing(int section, bool footer, int slot);
    // After an undo or a redo that was in `story`: the view shows where it was done.
    void followHistory(QTextDocument *story, const QTextCursor &at);
    int lineEdge(int position, bool end);
    void caretMoved(bool keepWanted = false);
    int m_movedFrom = -1;   // where the caret (or the end of a drag) was before the key or the drag that moved it, for the way it went; -1 when it is not known (a click)
    void ensureCaretVisible();
    void documentChanged(int position, int removed, int added);
    void paginationStep();
    void startPagination();
    void blink();
    double scale() const { return m_zoom * 96.0 / 72.0; }
    QRectF contentRect(int page) const { return m_rects.value(page); }
    void startAutoscroll();

    WriteDoc *m_doc = nullptr;
    QTextCursor m_cursor;
    double m_zoom = 1.0;
    ZoomMode m_mode = ZoomMode::Percent;
    int m_across = 1, m_down = 1;
    bool m_marks = false;
    bool m_caretWithoutFocus = false;

    // Where each page is, in the content (pixels at the zoom), and how big the content is.
    QVector<QRectF> m_rects;
    QVector<double> m_rowTops;
    QSizeF m_content;
    int m_knownPages = 0;
    bool m_complete = false;

    QHash<int, std::shared_ptr<Rendered>> m_rendered;
    QList<int> m_recent;
    QTimer m_pagination, m_blink, m_autoscroll;
    bool m_caretOn = true;
    double m_wantedX = -1;          // the x that Up and Down keep (frame-local), -1 for none
    bool m_dragging = false;
    int m_clicks = 0;
    QPoint m_lastPos;
    qint64 m_lastClickTime = 0;
    bool m_relayouting = false;
    QTextDocument *m_connected = nullptr;
    std::unique_ptr<SpellCheck> m_spell;
    bool m_spellOn = true;
    int m_contextAt = -1;           // where the right click was, until the caret moves or the menu is gone
    int m_editEnd = -1;             // where the last edit left off (a word being typed ends the caret's way)
    HeaderFooterEdit m_hf;
    QTextCursor m_bodyCursor;       // the text's cursor while a header or footer is edited
    std::unique_ptr<SpellCheck> m_hfSpell;
    QMetaObject::Connection m_hfEdit;
};

} // namespace jp::write
