#pragma once
// The view of the document's pages: a continuous column of pages on a gray ground, or several across, drawn at a zoom of 10% to 500%.
// Only the pages in view are made (the paginator keeps where each starts, and makes a page when it is asked for) and the rest of the
// document is laid out in the background, so the first page shows at once, and the number of pages grows while a long document is
// worked out. The view also takes the keyboard and the mouse: the caret is drawn on the page that holds it, and what is typed goes to
// the story through the commands of core/editops.h.

#include "core/pictureops.h"
#include "core/writedoc.h"

#include <QAbstractScrollArea>
#include <QPixmap>
#include <QTextCursor>
#include <QTimer>
#include <functional>
#include <memory>

class QMenu;
class QMimeData;
class QMouseEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QTextDocument;

namespace jp {
class LayoutCache;
}

class QToolButton;

namespace jp::write {

class SpellCheck;

class PageView : public QAbstractScrollArea {
    Q_OBJECT
public:
    enum class ZoomMode { Percent, PageWidth, OnePage, ManyPages };
    // The local document files (.docx, .doc, .rtf...) a drag holds, at most 20.
    static QStringList documentFilesIn(const QMimeData *mime);
    // A document opened read-only: the keys that move the caret work, and nothing is typed, dropped, or dragged.
    void setReadOnly(bool on) { m_readOnly = on; }
    bool readOnly() const { return m_readOnly; }

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

    // ---- marks over the pages for review: a bar in the margin beside each line a tracked change touches, a marker in the margin for each comment, and (with `deletions`) a balloon
    // for each deletion in place of its text. Nothing here is the document's.
    struct ReviewMarks {
        bool changeBars = false, formatBars = false, comments = false, deletions = false;
        bool operator==(const ReviewMarks &o) const { return changeBars == o.changeBars && formatBars == o.formatBars && comments == o.comments && deletions == o.deletions; }
    };
    void setReviewMarks(const ReviewMarks &marks);
    ReviewMarks reviewMarks() const { return m_reviewMarks; }

    // ---- zoom: 1 is 100%, from 0.1 to 5
    static constexpr double kLeastZoom = 0.1, kMostZoom = 5.0;
    double zoom() const { return m_zoom; }
    ZoomMode zoomMode() const { return m_mode; }
    void setZoom(double zoom);
    // Page Width fits the page to the width of the window, One Page the whole page in the window, Many Pages `across` pages by `down`.
    void setZoomMode(ZoomMode mode, int across = 2, int down = 1);
    int pagesAcross() const { return m_across; }
    // Page movement: the pages in a column (the usual), or all in one row that the window scrolls side to side.
    // Draft: the pages run on without edges, shadows, headers, or footers, a dotted line (named for a page or section break) where each ends.
    void setDraft(bool on);
    bool draft() const { return m_draft; }
    // While a header or footer is edited the page's own text is shown dimmed (the usual), or left out.
    void setShowBodyWhileEditingHeader(bool on);
    bool showBodyWhileEditingHeader() const { return m_showBodyInHeader; }
    void setSideToSide(bool on);
    bool sideToSide() const { return m_sideToSide; }

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
    // The line of text at a position of the document, as a rectangle of the viewport (a caret's place); false when that page is not laid out.
    bool positionRect(int position, QRect *rect);
    // Draws the caret while the view has not the keyboard (for screenshots).
    void setCaretWithoutFocus(bool on) { m_caretWithoutFocus = on; viewport()->update(); }

    // ---- showing
    void setShowMarks(bool on);
    void setShadeFields(bool on);   // the fields shaded gray on the screen (never in print or a PDF)
    bool shadeFields() const { return m_shadeFields; }
    bool showMarks() const { return m_marks; }
    // The words the Find pane found, as ranges of the document's text: drawn on a yellow band until it sets none.
    void setFindHighlights(const QVector<QPair<int, int>> &ranges);
    const QVector<QPair<int, int>> &findHighlights() const { return m_findHighlights; }
    // How the Find pane says where its words are in the story of a header, a footer, or a note (given that story): drawn with the others.
    void setFindSideMatcher(const std::function<QVector<QPair<int, int>>(const QTextDocument *)> &matcher);
    // The faint edges of the cells of tables that have no lines (View Gridlines).
    void setShowGridlines(bool on);
    bool showGridlines() const { return m_gridlines; }
    // Spelling as you type: misspelled words get a red wave on the screen. The checking is the document's own, found a paragraph at a time for the pages in view.
    // (The check of what is being edited: the header's or footer's while one is, else the text's.)
    SpellCheck *editedSpelling() const { return (m_hf.active || m_note.active) && m_hfSpell ? m_hfSpell.get() : m_spell.get(); }
    SpellCheck *spelling() const { return m_spell.get(); }
    bool spellingEnabled() const { return m_spellOn; }
    // Follows the link at a position of the text as a Ctrl+click does (a web or email address is handed on, a link to a bookmark or a reference moves the caret); false where there is none.
    bool followLinkAt(int position);
    // Edits the footnote whose mark is at the position (a double click on a mark), at the foot of its page; false where there is no mark.
    bool openNoteAt(int position);
    // The place a menu is for: where the last right click was (a click inside a selection leaves the selection and the caret as they were), else the caret.
    int contextPosition() const { return m_contextAt >= 0 ? m_contextAt : m_cursor.position(); }
    void setSpellingEnabled(bool on);

    // ---- editing (each is a step that undoes)
    void undo();
    void redo();
    static bool smartCutPaste();
    void cut();
    void copy();
    void paste();
    // Paste with a choice: keep the source's formatting (as Paste), merge with the formatting around (the emphasis stays), text only, a picture, or the clipboard's HTML. Which of them the
    // clipboard can give now.
    enum class PasteKind { Keep, Merge, TextOnly, Picture, Html };
    bool canPaste(PasteKind kind) const;
    void pasteAs(PasteKind kind);
    // After a paste, the Paste Options button offers the other ways to paste the same clipboard. The offer holds until anything else changes the text or the caret moves.
    bool pasteOptionsAvailable() const;
    // Takes the last paste back (one step) and pastes the clipboard again the way `kind` says; false when the offer is over or the clipboard has nothing for `kind`.
    bool changeLastPaste(PasteKind kind);
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
    bool enterHeaderFooterAtCaret(bool footer);
    // Previous and Next section (-1 and 1) while a header or footer is edited: the same kind of one (header or footer) on the first page of the section before or after. False where there is none.
    bool enterNeighborSection(int step);
    // Go to Header and Go to Footer: the other one on the page being edited.
    bool enterOtherHeaderFooter(bool footer);   // for the page the caret is on (made at once: Insert > Header)
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

    // ---- pictures
    // A picture is selected when the selection is the one character that is a picture; handles at its corners and sides then change its size, and a drag on it moves it (a
    // picture in line goes to another place in the text, one set at a place on the page to another place there). Each is one step.
    enum class PictureHandle { None, TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, Body };
    // The place in the text of the selected picture, -1 for none.
    int selectedPicture() const;
    void selectPicture(int position);
    // The picture's rectangle in the window; false where it is not on a page that is drawn.
    bool pictureRect(int position, QRectF *rect);
    // The handle (or the picture itself) at a point of the window; and the point a handle is at.
    PictureHandle pictureHandleAt(const QPoint &viewportPos);
    bool pictureHandlePoint(PictureHandle handle, QPointF *point);
    // How many times the rectangle of a picture has been worked out since the program began (a cost test).
    static qint64 pictureRectsComputedForTest();
    // The place in the text of the picture at a point of the window, -1 for none.
    int pictureAtPoint(const QPoint &viewportPos);
    // A picture from the bytes of a file, or an image (the clipboard's), at the caret in place of a selection; the picture is then the selection. The reason is told (notice) when it fails.
    bool insertPictureBytes(const QByteArray &bytes, const QString &name, PictureWrap wrap = PictureWrap::Inline);
    bool insertPictureImage(const QImage &image, PictureWrap wrap = PictureWrap::Inline);
    // A shape of the gallery, or a text box of a design, at the cursor; it is selected when it is in.
    bool insertShapeObject(const QString &preset);
    bool insertTextBoxObject(const TextBoxDesign &design);
    bool insertChartObject(ChartForm form);
    // The picture's size, wrap, and alt text, with the notice told when the change cannot be made.
    bool changeSelectedPicture(const std::function<bool(WriteDoc &, int)> &change);

    // ---- dragging text
    // The selected text as a drag carries it: its words, and a mark that it is this view's own; a drop of it on the view moves it (copies it, with the Ctrl key) as one step of the
    // history, and the text is selected where it went. Null where nothing is selected or the selection is in a header, footer, or note. The caller owns what it returns. (A press inside
    // the selection and a pull makes such a drag; test code drops what this returns.)
    QMimeData *prepareTextDrag();
    // Whether a press inside the selection begins a drag of it (Copy Formatting turns it off: a press there is the start of a new selection).
    void setTextDragAllowed(bool allowed) { m_textDragAllowed = allowed; }
    // Options > Advanced: whether a link is followed by Ctrl+click (true, the default) or by a plain click, and whether selected text can be dragged to move or copy it.
    void setLinkNeedsCtrl(bool on) { m_linkNeedsCtrl = on; }
    bool linkNeedsCtrl() const { return m_linkNeedsCtrl; }
    void setDragTextOption(bool on) { m_dragTextOption = on; }
    // Options > Advanced: typing over a selection replaces it (true, the default) or puts the new letters after it; the Insert key turns overtype mode (each letter typed replaces the one after the caret) on and off when it is let to.
    void setTypingReplaces(bool on) { m_typingReplaces = on; }
    void setInsertKeyOvertype(bool on) { m_insertKeyOvertype = on; if (!on) m_overtype = false; }
    bool overtype() const { return m_overtype; }

    // Lays out every page now (for the count).
    void paginateAll();
    // The pages the window is set to show, laid out and drawn once more.
    void refresh();

Q_SIGNALS:
    // Document files dropped on the pages (the window opens them).
    void documentsDropped(const QStringList &paths);
    void zoomChanged(double zoom);
    void pagesChanged();                    // how many pages there are, or whether all are known
    void caretChanged();                    // the caret moved or the selection changed
    void contentChanged();                  // the text was edited
    void selectionMade();                   // a click or a drag with the mouse ended: the selection (or the caret) is where it was left
    void contextMenuWanted(const QPoint &globalPos);
    void headerFooterModeChanged();         // a header or footer is edited now, or none is any more
    void noteModeChanged();                 // a footnote is edited now, or none is any more
    void notice(const QString &text);       // a sentence for the person about what a command could not do
    void linkFollowed(const QString &address, const QString &shown);   // Ctrl+click on a link to an address outside the document, with the words it shows (the window asks if need be, then opens it)

protected:
    bool viewportEvent(QEvent *e) override;   // (the tip of a link: where it goes, and its screen tip)
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void scrollContentsBy(int dx, int dy) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    bool m_documentDrag = false;   // what is being dragged over is a document file, which the window opens
    bool m_readOnly = false;
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

    // A drag on the selected picture: which handle, and where the picture was and is (in the window), until the button is released.
    struct PictureDrag {
        bool active = false, moved = false;
        PictureHandle handle = PictureHandle::None;
        int position = -1;
        QPointF from;
        QRectF start, now;
        QPoint scrollAtStart;
        bool floating = false;
        PictureInfo info;
    };
    bool beginPictureDrag(QMouseEvent *e);
    void updatePictureDrag(const QPoint &pos);
    void endPictureDrag(const QPoint &pos);
    bool inlinePictureRect(int position, const PictureInfo &info, QRectF *rect);
    // A picture set at a place on the page: its rectangle on the page (points) and the page, from the items the page was made with; and the position of the picture a point of a page
    // is in (the front-most), -1 for none.
    bool floatingPictureRect(int position, QRectF *onPage, int *page);
    int floatingPictureAt(const Hit &hit);
    // A drag of the selected text: begun by a press inside the selection (`pending`, until the pointer has moved far enough), carried while the drag runs (`active`), and `done` once a
    // drop on this view has moved or copied the text.
    struct TextDrag {
        bool pending = false, active = false, done = false;
        QPoint start;
        QTextCursor selection;
    };
    void startTextDrag();
    // A drop of text on the view: this view's own selection moved or copied, or text from elsewhere put where it is dropped. False where the drop is not of text.
    bool dropText(QDropEvent *e);
    bool wantsTextDrop(const QMimeData *mime) const;
    TextDrag m_textDrag;
    bool m_textDragAllowed = true;
    bool m_linkNeedsCtrl = true, m_dragTextOption = true;
    bool m_typingReplaces = true, m_insertKeyOvertype = false, m_overtype = false;
    PictureDrag m_pictureDrag;
    // Dragging the border at the right of a table's column, or at the foot of a row: the position of a cell of the column, its width when the drag began, and where the pointer began.
    struct ColumnDrag {
        bool active = false;
        int position = 0, page = 0;
        bool pending = false;                     // pressed on a border, not pulled yet (it is a click until the pointer has moved a few pixels)
        QPoint startPx;                           // (where the press was, in the view)
        bool whole = false;                       // the resize handle at the table's corner: the whole table's width
        bool row = false;                         // the bottom border of a row, not the right one of a column
        double width = 0, startX = 0, nowX = 0;   // (points on the page; for a row, the height and the pointer's y)
        double startY = 0, nowY = 0;
        double left = 0, right = 0;
        double top = 0, bottom = 0;               // the table's top and bottom on the page, for the line that shows where the border would go
    };
    ColumnDrag m_columnDrag;
    // The border of a column or a row of a table on the page under the pointer (within a few pixels of a cell's right or bottom edge, one column or row wide), or false.
    bool columnBorderAt(const QPoint &viewportPos, ColumnDrag *found);
    // The resize handle at the lower right corner of the table the caret is in: its box on the page and where the table is (or false). The handle is drawn there, and pulling it sets the table's width.
    bool tableHandle(QRectF *box, int *page, int *cellPosition, QRectF *table);
    bool tableHandleAt(const QPoint &viewportPos, ColumnDrag *found);
    int m_hoverCursorShape = -1;
    bool pictureRectUncached(int position, QRectF *rect);
    struct RectKey {
        const void *doc = nullptr;   // the document (another one made as this one is is not it)
        int position = -1, revision = 0, hscroll = 0, vscroll = 0;
        double zoom = 0;
        quint64 layout = 0;
        int pages = 0;
        bool operator==(const RectKey &o) const { return doc == o.doc && position == o.position && revision == o.revision && hscroll == o.hscroll && vscroll == o.vscroll && zoom == o.zoom && layout == o.layout && pages == o.pages; }
    };
    struct RectCache {
        bool valid = false;
        RectKey key;
        QRectF rect;
    };
    RectCache m_rectCache;
    quint64 m_layoutSerial = 0;   // counted up each time the pages are made again
    void paintPictureHandles(QPainter &p);
    void paintReviewMarks(QPainter &p, Rendered &r);
    ReviewMarks m_reviewMarks;
    // What the marks of a page are made from, for the text as it was last looked at (the revision of the story): found for the stretch of the story a page shows, never for all of it.
    struct ReviewCache {
        struct Mark { int at; QString initials, text; QColor color; bool done; };
        struct PageMarks {
            int lo = 0, hi = 0;
            QVector<std::pair<int, int>> spans, formatSpans;
            QVector<Mark> comments, deletions;
        };
        int revision = -1;
        const void *doc = nullptr;
        QHash<int, PageMarks> pages;   // by where the page's text begins in the story
    } m_reviewCache;
    const ReviewCache::PageMarks &reviewMarksOf(int storyFrom, int lo, int hi);
    int m_pictureDragPage = -1;

    WriteDoc *m_doc = nullptr;
    QTextCursor m_cursor;
    double m_zoom = 1.0;
    ZoomMode m_mode = ZoomMode::Percent;
    int m_across = 1, m_down = 1;
    bool m_sideToSide = false, m_draft = false, m_showBodyInHeader = true;
    bool m_marks = false;
    QVector<QPair<int, int>> m_findHighlights;
    std::function<QVector<QPair<int, int>>(const QTextDocument *)> m_findSide;
    struct LastPaste {
        int from = -1, selectionEnd = -1, to = -1;   // the place the paste began (and where the selection it replaced ended), and where it ended
        int revision = -1;
        QTextDocument *story = nullptr;
    } m_lastPaste;
    QToolButton *m_pasteButton = nullptr;
    void notePaste(int from, int selectionEnd);
    void updatePasteButton();
    bool m_gridlines = false;
    bool m_shadeFields = false;
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
