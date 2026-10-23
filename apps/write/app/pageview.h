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
    SpellCheck *editedSpelling() const { return (m_hf.active || m_note.active) && m_hfSpell ? m_hfSpell.get() : m_spell.get(); }
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
    // (Where there is none, an empty one is shown to be typed in, and is made in the document when something is put in it: leaving it as it is leaves the document as it was. With
    // `makeNow` it is made at once. A blank page, which an odd-page or even-page section break leaves, has none to edit.)
    bool enterHeaderFooter(int page, bool footer, bool makeNow = false);
    bool enterHeaderFooterFor(int section, bool footer, int slot, int page, bool makeNow = false);
    bool enterHeaderFooterAtCaret(bool footer);   // for the page the caret is on (made at once: Insert > Header)
    void leaveHeaderFooter();
    // After a change to the sections or to their options: the header (footer) being edited is the one that its page uses now (else the one that is, made blank if `create`, else editing ends).
    void refreshHeaderFooter(bool create);
    Area areaAt(const QPoint &viewportPos);

    // ---- footnotes
    // A footnote being edited in place, at the foot of a page: the caret and every edit are in the note's own story (its number mark and the separator are not to be edited), and
    // Up and Down at the edges of its lines, Esc, or a click in the text leave it.
    struct NoteEdit {
        bool active = false;
        int id = 0, page = 0;   // the note (its w:id) and the page it is edited on
        QTextDocument *story = nullptr;
        int copy = 0;           // of the copies the pages set of it (it is set again for each citation), the one that is edited: its place in the order of the pages
        int number = 0;         // the number that copy has now (to tell its box from another copy's on the same page)
    };
    bool inNote() const { return m_note.active; }
    const NoteEdit &noteEdit() const { return m_note; }
    // Edits footnote `id` where the pages set it (the first page that has it). False where no page has it.
    bool enterFootnote(int id);
    // Back to the text: the caret where it was, or with `atReference` just after the note's reference.
    void leaveNote(bool atReference = false);
    // A header or footer and a note are never edited together: at most one of inHeaderFooter() and inNote() holds.
    bool sideModesExclusive() const { return !(m_hf.active && m_note.active); }
    // The section the caret is in (the text's, or that of the page whose header, footer, or note is being edited).
    int cursorSection();

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
    void noteModeChanged();                 // a footnote is edited now, or none is any more

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
    static bool onEdgeLine(const FrameSpot &spot, int dir);   // whether the place is on the first (dir < 0) or the last line of its frame's text
    bool frameSpotAt(Rendered &r, int position, FrameSpot *spot);
    int moveVertical(int position, int direction);
    // The same, for the header or footer that is being edited: where its boxes on the page being edited put a position, and which position a point is at.
    bool hfCaretLine(int position, int *page, QLineF *line);
    bool hfSpotAt(Rendered &r, int position, FrameSpot *spot);
    int hfPositionAt(const Hit &hit);
    int hfLineEdge(int position, bool end);
    int hfMoveVertical(int position, int direction);
    Area areaOf(const Hit &hit);
    // A footnote's story and its Id where a point on a page is in the box of one; false where it is not.
    bool noteAt(const Hit &hit, QTextDocument **story, int *id, int *number = nullptr);
    // The page whose foot has the note (the first that has a box of it), -1 where none is found.
    int pageShowingNote(const QTextDocument *story);
    bool enterNote(QTextDocument *story, int id, int page, int number = 0);
    // After the pages were laid out again: the page the note being edited is on.
    void settleNote();
    // Whether the caret is in the first (dir < 0) or the last (dir > 0) line of the note being edited (not of a part of it that goes on from or to another page).
    bool noteLineAtEdge(int dir);
    // The parts the note being edited is shown in on the pages, each a page and the position in the note's story where the part begins: a note that goes on from a page to the next is a
    // part for each page, and a note cited on two pages is shown in a copy for each citation (the parts of a copy begin again at the start of the note). They are the parts of the copy
    // that `hintPage` has, in order.
    struct NotePart {
        int page = 0, from = 0, number = 0;
    };
    // Every copy of the note being edited that the pages set (by the number each has, the parts of one copy on consecutive pages), in the order of the pages.
    QVector<QVector<NotePart>> copiesOfEditedNote();
    // The copy being edited: the one that `m_note.copy` names while it has a part on `hintPage`, else the nearest to that page; -1 where none.
    int editedCopyIndex(const QVector<QVector<NotePart>> &copies, int hintPage) const;
    QVector<NotePart> partsOfEditedNote(int hintPage);
    // The copy that has the box clicked (or the first that has a part on the page): which copy is edited, and its number.
    void chooseCopy(int page, int number);
    static int partIndexAt(const QVector<NotePart> &parts, int position);   // the part that has a place of the note: the last that begins at or before it
    // The page that shows a place in the note being edited (in the copy being edited), -1 where none does; and the note's page made that of the caret.
    int pageOfNotePosition(int position, int *number = nullptr, int *copy = nullptr);
    void syncNotePage();
    // The caret (and a selection that began in an endnote's words) held in them: a click put it there, or a key moved it inside the note. False where it is in none, or a key took it
    // there from the text (which skips the notes, as ever).
    bool holdInEndnote(int movedFrom);
    // A place in the second or a later copy of an endnote (a copy that is shown and not edited) is the same place in the first copy; any other place is itself.
    int placeInFirstCopy(int position) const;
    // Leaves the endnote whose words the caret is in, for the text just after its reference.
    void leaveEndnote(int id);
    // Entering a header or footer (or a note) leaves the note (or the header or footer) that was being edited, which puts the caret back in the text: the one place the two modes meet.
    void leaveSideModeFor(bool noteMode);
    // The caret back in the text, where it was before a header, footer, or note was entered (the start of the text where that place is not known).
    void restoreBodyCursor();
    // The story being edited beside the text (a header or footer, or a note), the page it is edited on, and the boxes of that story on a drawn page.
    const QTextDocument *sideStory() const { return m_hf.active ? m_hf.story : m_note.active ? m_note.story : nullptr; }
    int sidePage() const { return m_hf.active ? m_hf.page : m_note.page; }
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
    // The caret is in the text or in the header or footer being edited: a cursor in the other is not the view's. The text's takes the view out of the header; another story's is refused.
    bool takeCursorOf(const QTextCursor &c);
    HeaderFooterEdit m_hf;
    NoteEdit m_note;
    bool m_notePinned = false;      // the note was just entered: its page is the one that was clicked, not the page of the start of its text
    QTextCursor m_bodyCursor;       // the text's cursor while a header or footer is edited
    std::unique_ptr<SpellCheck> m_hfSpell;
    QMetaObject::Connection m_hfEdit;
};

} // namespace jp::write
