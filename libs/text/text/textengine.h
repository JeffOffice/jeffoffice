#pragma once
// Lays out one story across a chain of frames (linked text boxes), each with
// columns, insets, vertical alignment and wrap obstacles. One QTextLayout is
// kept per paragraph; each line is positioned in the frame it landed in. Line
// y-coordinates are offset by the heights of the frames before (frameY) so
// that a single paragraph can span frames while staying in one QTextLayout.

#include "core/color.h"
#include "core/items.h"

#include <QDateTime>
#include <QFont>
#include <QHash>
#include <QLineF>
#include <QPolygonF>
#include <QSizeF>
#include <QTextBlock>
#include <QTextLayout>
#include <memory>

class QPainter;
class QTextDocument;

namespace jp {

// What fields and notes read from the publication they're in: its page
// numbering, business information, mail merge, and the text of its stories
// (a note's number counts the notes in every story). The publication
// answers; the text engine never needs the rest of it.
class FieldSource {
public:
    virtual ~FieldSource() = default;
    virtual int firstPageNumber() const = 0;
    virtual QString pageNumberFormat() const = 0;
    virtual QString businessField(const QString &key) const = 0;
    virtual QStringList mergeFields() const = 0;
    virtual QString mergeValue(int record, const QString &field) const = 0;
    virtual const QTextDocument *storyDoc(const QString &id) const = 0;
    virtual QList<const QTextDocument *> storyDocs() const = 0;   // every story, in the publication's order
};

// A stock font's average character width in ems (OS/2 xAvgCharWidth), the
// unit .pub tracking is measured in; -1 when it isn't one JeffPub knows.
double originalAverageCharEm(const QString &family, bool bold, bool italic);

// The moment automatic dates and times show: now, unless a tool has set another (setFieldClock, so that a render can be made again at the
// moment of another). The apps never set one.
QDateTime fieldNow();
void setFieldClock(const QDateTime &moment);

struct FieldContext {
    const FieldSource *doc = nullptr;
    int pageNumber = 1;           // 1-based
    int pageCount = 1;
    int mergeRecord = -1;         // -1 shows «Field» codes
    int continuedOnPage = 0, continuedFromPage = 0;
    int nextPage = 0, prevPage = 0;   // pages of the next and previous boxes of a linked chain (0: none)
    QDateTime now = fieldNow();
    QString resolve(const QString &code) const;
    // What makes a layout differ, for the cache of layouts. The minute (`now`) is part of it only if `withClock`: a layout is laid out
    // again when the date or time it shows changes, which fieldValuesKey says (not every minute).
    QString key(bool withClock = true) const;
};

// What a story's layout depends on besides its own text and its frames: the fields that show what nothing else in a layout's key says (the
// date and time, business information, mail merge values), and the stories of the notes it refers to.
struct StoryFields {
    QStringList codes;   // the code of each such field (in the story or in a note), once
    QStringList notes;   // the notes' stories
};
// The story's fields of that kind. `source` finds the notes' text; without it only the story's own is looked at.
StoryFields storyFields(const QTextDocument *doc, const FieldSource *source);
// What those fields show in a frame (`ctx`), as text. A layout changes when this does, though the story did not.
QString fieldValuesKey(const StoryFields &fields, const FieldContext &ctx);
// A number for the notes' text as it is (it changes when a note's story is edited or replaced).
quint64 notesKey(const QStringList &notes, const FieldSource *source);

struct FrameSpec {
    QSizeF size;
    QMarginsF insets;
    int columns = 1;
    double gap = 9;
    VAlign valign = VAlign::Top;
    bool hyphenate = true;          // insert soft hyphens at allowed break points
    double hyphenZone = 18;         // points: hyphenate only if moving the word would leave more space
    double baselineGrid = 0;        // the page's baseline guides: spacing (0 = none)
    double baselineOrigin = 0;      // and one baseline's position, frame-local
    QVector<QPolygonF> obstacles;   // frame-local, already expanded by wrap distances
    // The frame is a polygon (half of a cell divided by a diagonal), the obstacles being the rest of its box:
    // a word too long for the place beside them moves down to where it fits instead of being broken, and
    // middle or bottom aligned text goes as far down as it fits.
    bool fitToShape = false;
    // The text that doesn't fit runs out of the frame instead of waiting for
    // a next one (a slide's text box: the presentation program shows it all):
    // on down from text at the top, both ways from centered text, and up from
    // text at the bottom. Text that fits is set as without it. One frame, one
    // column, nothing to wrap around.
    bool runsOver = false;
    // The paragraphs aren't broken to the frame's width (a slide's text box that doesn't wrap): each is one line, against the frame as its
    // alignment says, and out of it where it is longer than the frame. One frame, one column.
    bool noWrap = false;
    // The block of the frame's text (its widest line, with its bullets) is set in the middle of the frame, the lines in it aligned as their
    // paragraphs say: a slide's text box whose text is anchored "centered". One column.
    bool centerBlock = false;
    FieldContext ctx;
};

struct LayoutEnv {
    ColorScheme colors;
    FontScheme fonts;
    double fontScale = 1.0;
    double defaultTab = 36;         // points between the tab stops of a paragraph without its own
    bool showFieldShading = false;
    // The text is set without ligatures (a slide's, as the presentation program sets it: a PDF's text then names each letter the page draws, where a
    // ligature's glyph stands for several letters that a reader that copies the text can't name, and the lines break as the screen's do).
    bool noLigatures = false;
    // Lines as the presentation program sets a slide's text, as probed in it (a text box per case, line positions read back through its object
    // model and the pictures it draws; 20 fonts, text shrunk to fit and not):
    //  - A single line is 1.2 times the largest text on it whatever the font, a line's extra spacing goes above it, the paragraph at the top
    //    of a box takes no space before it, and an empty paragraph shows no bullet.
    //  - A line's baseline: a single line, 1.2 em tall, is shared between above and below the baseline in the proportion of the font's Windows
    //    ascent to its Windows descent (OS/2 usWinAscent and usWinDescent, not hhea or typo): 0.974 of the line is above it in Arial, 0.966 in
    //    Times New Roman, 0.936 in Calibri, 0.928 in Book Antiqua (the fonts the program drew with, read from the tables embedded in its PDFs
    //    of 64 presentations; a font on the computer is its own, and any other stand-in keeps the descent it had). A line taller than that
    //    (a share over 100 percent, an exact height over 1.2 times the size) has it 3/4 of the way down, whatever the font; a shorter one keeps
    //    that descent but no higher than 3/4 down. Text that isn't shrunk is set so too. Against the probes (shares of 80 to 150 percent
    //    and an exact 20 points, 20 fonts) and 1,500 lines of the PDFs it is 0.1 points off on average and 0.5 at worst, where the single
    //    line's rule with a stand-in's descent was 0.6 off on average and 2.4 at worst.
    //  - Space before or after given as a share of the line (spcPct) is that share of 1.2 times the size the text is drawn at, so it follows
    //    that size however the size came about (shrunk, or changed); given in points it is as written.
    //  - Pair kerning counts the size the text is drawn at.
    //  - A list item's text follows its bullet at the bullet's advance when the bullet has no font of its own and no hang (no gap, unlike
    //    JeffPub's own lists); with a hang (marL, indent) it starts at marL.
    //
    // And text it shrank to fit (a:normAutofit, fontScale below 1 here; fontScale 25 to 100 percent, lnSpcReduction of 0, 10, and 20, sizes of
    // 1 to 60 points), drawn so here:
    //  - A run's size is its size times the scale, rounded to the nearest whole point (halves up, 1 at least): 28 points at 85 percent are 24.
    //    The line is 1.2 times that rounded size. A size at 100 percent isn't rounded, and a size can come out bigger than the text's own
    //    (10.6 points at 99.99 percent are 11). Bullets and numbers follow: a bullet that is a share of its text is that share of the
    //    rounded size, the text's own size otherwise.
    //  - Only the text shrinks. Indents (marL, indent), tab stops, space before and after given in points, an exact line spacing (spcPts),
    //    and the space added between letters stay as written; lnSpcReduction doesn't touch space.
    //  - lnSpcReduction is points of percent taken off a share-of-line spacing (single counts as 100, so 90 percent less 10 is 80 percent of
    //    a 1.2 em line), not a share of it. An exact spacing isn't reduced.
    //  - What is stored (fontScale, lnSpcReduction) is drawn as it is on opening: the program doesn't fit it again until the text is edited.
    bool slideLines = false;
    // Percent taken off each paragraph's line spacing (a slide's text the
    // presentation program shrank to fit: a:normAutofit's lnSpcReduction).
    double lineCut = 0;
    // The fonts a document carries of its own (a presentation's embedded fonts): each family it names (lower-cased) and the private family the
    // font was added to the font database under. A run that names one of these is drawn with that font, whatever the computer has under the
    // name; no other document, and no menu, sees them.
    QHash<QString, QString> familyAliases;
    QString aliasOf(const QString &family) const { return familyAliases.isEmpty() ? family : familyAliases.value(family.toLower(), family); }
    QString key() const;
};

// Where text is drawn: bold drawn by hand (a font with no bold face) is a
// different stroke on the screen and in pictures, in a PDF Publisher exports,
// and on paper.
enum class TextOutput { Picture, Pdf, Print };

struct PaintOptions {
    TextOutput output = TextOutput::Picture;
    int selFrom = -1, selTo = -1;   // document positions
    QColor selColor = QColor(51, 153, 255, 90);
    bool showSpecial = false;       // ¶, ·, →
    bool shadeFields = false;
    bool showSpelling = false;
    QVector<QPair<int, int>> misspelled;  // document ranges
};

// A paragraph's text layout. Qt works out a tab's width from the layout's tab
// stops whenever it is asked where text is (the caret, the drawing), not once
// when the line is laid out, and keeps one list of stops for every line. The
// lines of a paragraph that start in different places need stops of their own,
// so each is laid out with its list (setLineTabs) and handed out with it again
// (lineAt, draw). With one list for all its lines it works as a QTextLayout.
class ParagraphLayout : public QTextLayout {
public:
    using QTextLayout::QTextLayout;
    void resetTabs();                                        // before the lines are laid out again
    void setLineTabs(const QList<QTextOption::Tab> &tabs);   // the stops for the line to lay out next
    void keepLineTabs(int line);                             // the line just laid out used them
    QTextLine lineAt(int i);
    void draw(QPainter *p, const QPointF &pos, const QList<FormatRange> &selections = {}, const QRectF &clip = QRectF());

private:
    void use(int set);
    QVector<QList<QTextOption::Tab>> m_sets;   // the distinct lists of stops
    QVector<int> m_lineSet;                    // the list each line was laid out with
    int m_current = -1;                        // the list the option has now
};

class StoryLayout {
public:
    StoryLayout();
    ~StoryLayout();

    void build(const QTextDocument *doc, const QVector<FrameSpec> &frames, const LayoutEnv &env);

    // A footnote cut at the end of a page (a text box, or a column) goes on at the bottom of the next: its
    // story, its number, and where its text goes on from.
    struct CarriedNote { QString storyId; int number = 0; int from = 0; };
    // A stretch of the story laid out instead of all of it: a page of a long document (libs/flow lays out a
    // page at a time, each from where the one before ended). The defaults are `build` without it.
    struct Span {
        // Laying out starts with the paragraph that has this document position, the whole paragraph (its lines
        // break as they would from its start).
        int from = 0;
        // The first paragraph's first lines are already placed, in a frame before the ones that are given: it
        // goes on in the next frame after this many lines (-1: it doesn't). The caller gives a frame for them
        // first, as wide as the next, and tall enough, and reads the lines from the frames after it. (The
        // paragraph is laid out whole, so these lines are broken again: as many lines as there are, each time.)
        int firstBreakAfter = -1;
        // The first paragraph is the rest of one that began before `from`: it is laid out from there, as a
        // paragraph of its own with no bullet, first-line indent, or drop cap, and its lines are numbered from
        // 0. Its lines before are not laid out again, which is its use for a paragraph of thousands of lines.
        bool continued = false;
        // The most characters of a paragraph that are laid out (from `from`, if it is continued); a longer one
        // is cut there, its lines are marked (LineBrief::cut), and nothing after it is laid out. If a cut
        // paragraph's lines don't reach the frame laying out stops at (stopAtFrame), the window was too small,
        // and windowShort() says so. 0: whole paragraphs.
        int window = 0;
        // Laying out ends after the paragraph in which a line first lands in this frame or a later one (-1: it
        // runs to the story's end). The lines of that paragraph are all laid out.
        int stopAtFrame = -1;
        // Widow and orphan control, keeping lines together, and keeping with the next paragraph: false leaves
        // them to the caller, which then decides where the frames end.
        bool keepRules = true;
        // Space before the first paragraph at the top of the first frame. A paragraph that goes on from the
        // page before has none there, nor has one that the text only came to.
        bool spaceBeforeFirst = true;
        // Footnotes and endnotes in the story before the first paragraph, whose numbers the notes in this stretch go on from
        // (the first paragraph's own, in the lines that went before, the layout counts itself).
        int footnotesBefore = 0, endnotesBefore = 0;
        // Footnotes cut at the end of the page before: their rest goes first at the bottom of the first frame.
        QVector<CarriedNote> carried;
    };
    void build(const QTextDocument *doc, const QVector<FrameSpec> &frames, const LayoutEnv &env, const Span &span);

    int frameCount() const { return int(m_frames.size()); }
    bool overflow() const { return m_overflow; }
    double usedHeight(int frame) const;      // content height used, including insets
    int firstPosition(int frame) const;      // first document position laid out in frame (-1 if none)
    int lastPosition(int frame) const;

    void paint(QPainter *p, int frame, const PaintOptions &o) const;

    int hitTest(int frame, const QPointF &local) const;
    bool caretRect(int pos, int *frame, QRectF *rect) const;
    int frameOf(int pos) const;
    int moveVertical(int pos, int dir, double x) const;  // dir -1 up, +1 down
    int lineStart(int pos) const;
    int lineEnd(int pos) const;
    QVector<QRectF> rangeRects(int frame, int from, int to) const;
    QVector<QRectF> lineRects(int frame) const;   // every line box in the frame, frame-local
    struct LineInfo { QRectF rect; QString text; QString family; double pointSize = 0; int docStart = 0; double baseline = 0; double below = 0; double markerX = -1; int docPos = 0; int docEnd = 0; };   // docStart: where its paragraph starts; docPos, docEnd: the line's first character and just past its last, as document positions; below: line spacing under the text; markerX: where a list item's bullet is drawn (-1: none)
    QVector<LineInfo> lineInfo(int frame) const;  // for diagnostics (jpubtool layout)
    // Every line of the story in order, with where it is and nothing else (no text or fonts): for a caller that
    // works on the lines of a stretch of text, as the paginator does.
    struct LineBrief {
        int frame = -1;           // -1: past the last frame
        int block = 0;            // where its paragraph starts, a document position
        int line = 0;             // its place in its paragraph, from 0
        int lines = 0;            // the lines its paragraph has in all
        int pos = 0, end = 0;     // document positions: its first character, and just past its last
        QRectF rect;              // frame-local, with the spacing under the text
        double below = 0;         // the spacing under the text
        bool cut = false;         // its paragraph goes on past the lines laid out (Span::window); `lines` is how many there are, and the last is cut short
    };
    QVector<LineBrief> lineBriefs() const;
    // The footnotes that go on in `frame` or after it from a frame before: with the story's text left of each.
    QVector<CarriedNote> carriedNotes(int frame) const;
    // Whether the last build's window (Span::window) cut a paragraph before its lines reached the frame it stops at: it needs to be larger.
    bool windowShort() const { return m_windowShort; }

    // An object set in the text (tp::InlineObject): where it's drawn,
    // frame-local, and the item as JSON. It sits on the baseline, padded by
    // its wrap distances, as in Publisher.
    struct InlineObject { int frame = -1; QRectF rect; QString json; int docPos = 0; };
    const QVector<InlineObject> &inlineObjects() const { return m_inline; }

    struct Seg { int docPos, docLen, dispPos, dispLen; };
    struct Line { int frame = -1; int column = 0; QRectF rect; double below = 0; };   // below: the spacing under the text
    struct Block {
        int docStart = 0, docLen = 0;
        QString disp;
        QVector<Seg> map;
        std::unique_ptr<ParagraphLayout> tl;
        QVector<Line> lines;
        QString marker; QFont markerFont; QColor markerColor; double markerX = 0;
        double markerAdv = 0, markerGap = 0;         // the marker's advance, and the space after it
        double markerOff = 0;                        // where the text starts from the marker's origin, when the item doesn't hang
        bool markerFit = false, markerRound = false; // drawn to markerInk: a circle, or the stand-in glyph fitted to it
        QRectF markerInk;                            // from the marker's origin and the baseline, y down
        QString dropText; QFont dropFont; QColor dropColor; int dropLines = 0; double dropWidth = 0;
        bool dropUp = false;   // a raised cap: on the first line, rising above it
        QVector<QTextLayout::FormatRange> effects;   // ranges with shadow/emboss/engrave/glow
        bool textFx = false;                         // some of those ranges carry a:effectLst (tp::TextFx)
        bool directGlyphs = false;                   // drawn run by run (some letters drawn taller, shorter, or slanted)
        bool cut = false;                            // the paragraph's text was cut short (Span::window), at this place in it:
        int cutAt = 0;
        QVector<QPair<int, int>> fieldRanges;        // display ranges of fields
        QString leaders;                             // a leader character per tab stop (space = none)
        QVector<QPair<int, int>> noteRefs;           // a note reference's display position, and its index in notes()
        struct Box { int disp = 0; int docPos = 0; QString json; QSizeF size; QMarginsF pad; };
        QVector<Box> objects;                        // objects set in the text, by display position
        int dispFromDoc(int rel) const;
        int docFromDisp(int d) const;
    };

    // Footnotes and endnotes. A note is the field "footnote:<story>" or
    // "endnote:<story>" in the text, shown as its number (footnotes and
    // endnotes counted apart, in order within the story); its text is that
    // story, laid out at the bottom of the column the reference lands in
    // (footnotes, under a short rule) or after the story's last line, under
    // a "Notes" heading (endnotes). A footnote too tall for a whole column
    // is cut where the column ends: the rest goes on at the bottom of the
    // next column (or text box), above that column's own notes, under a rule
    // as wide as the column.
    struct Note {
        // The rest of a cut note in one column: its own layout, starting
        // where the column before left off.
        struct Piece { int frame = -1, column = 0; QRectF rect; double height = 0; std::shared_ptr<StoryLayout> layout; int from = 0; };   // from: where its text starts in the note's
        QString storyId;
        bool endnote = false;
        int number = 0;
        int frame = -1, column = 0;    // where it's drawn (-1: not, as overflow)
        QRectF rect;                   // frame-local
        double width = -1, height = 0, numberWidth = 0, firstBaseline = 0;
        QFont numberFont;
        QColor numberColor;
        std::shared_ptr<StoryLayout> layout;   // the part with the number (all of a note that isn't cut)
        QVector<Piece> more;           // the rest of a cut note, a piece for each column it goes on in
        int from = 0;                  // while a cut note goes on: where its rest starts in the note's text
    };
    const QVector<Note> &notes() const { return m_notes; }
    struct Rule { int frame; QLineF line; };
    const QVector<Rule> &noteRules() const { return m_noteRules; }   // above each column's footnotes

private:
    const Block *blockAt(int pos, int *rel) const;
    double frameY(int frame) const;   // where a frame's lines start in the paragraph layouts (frameCount() for overflow)
    // The effect lists of runs (tp::TextFx): under the letters (shadows beside them, glow, reflection) or over them (an inner shadow).
    void paintTextEffects(QPainter *p, int frame, bool over) const;
    // One layout pass; returns how far each frame's text moved down for its
    // vertical alignment.
    QVector<double> buildOnce(const QTextDocument *doc, const QVector<FrameSpec> &frames, const LayoutEnv &env);
    std::vector<std::unique_ptr<Block>> m_blocks;
    QVector<FrameSpec> m_frames;
    QVector<double> m_frameY;
    QVector<double> m_used;
    bool m_overflow = false;
    bool m_blankPast = false;   // blank paragraphs ran past the end of a frame that lets text run over (they move centered text, but are no overflow)
    LayoutEnv m_env;
    QVector<Note> m_notes;
    QVector<Rule> m_noteRules;
    struct Heading { int frame = -1; QPointF baseline; QFont font; QColor color; };
    Heading m_notesHeading;            // above the endnotes
    QVector<InlineObject> m_inline;
    quint64 m_serial = 0;              // this build's number, to keep what is made of it (pictures of effects) apart from another's
    Span m_span;                       // the stretch being laid out (all of the story, but for the build that was given one)
    bool m_windowShort = false;
};

// Resolve a character format for display: scheme colors, scheme fonts, scaling.
// Draws one line of text with its left end of baseline at `baseline`. Unlike
// QPainter::drawText, the size does not depend on the paint device's DPI, so a
// PDF at 1200 dpi matches the screen. Fonts must already be in layout units.
void drawPlainText(QPainter *p, const QPointF &baseline, const QFont &font, const QString &text);
// A font's single line spacing in ems as Publisher spaces it: the known
// metrics of common proprietary fonts (measured from Publisher's PDFs) when
// there are any, else the font's own tables.
double naturalLineEm(const QFont &f, const QString &requestedFamily);

QTextCharFormat resolveCharFormat(const QTextCharFormat &f, const LayoutEnv &env);

// A slide's text shrunk to fit by `scale` (a:normAutofit's fontScale), as the presentation program draws it (see LayoutEnv::slideLines): a
// run's size in whole points, and a paragraph's space before or after (tp::SpaceBeforeShare, tp::SpaceAfterShare) in points.
double slideShrunkSize(double size, double scale);
double slideSpaceOf(const QTextBlock &b, int shareProperty, double scale);
// The format ranges of a paragraph's display text with the letters of runs in
// a missing, measured stock font cut into pieces that take the original's
// widths (resolveCharFormat marks those runs); the layout does this itself.
QVector<QTextLayout::FormatRange> applyLetterWidths(const QVector<QTextLayout::FormatRange> &in, const QString &disp);

// The date and time formats Publisher offers (US English), in its order and
// in Qt's letters: a .pub date field's format number is the place here plus 1.
const QStringList &dateTimeFormats();
QFont baseFontFor(const QTextBlock &b, const LayoutEnv &env);
// While one page is painted, the pictures its text effects make (blurred shadows, glows, reflections) share a budget of pixels, all
// frames and both passes together, so that a file with many or large effects can't stall the painting: past it, the words are drawn
// without the effect. A scope inside another is part of it. With `pixels` below zero the budget is the painter's device's.
class EffectBudget {
public:
    explicit EffectBudget(const QPainter *p, qint64 pixels = -1);
    ~EffectBudget();
    qint64 left() const { return m_left; }   // what is left of this scope's own budget (nothing for one inside another)
    EffectBudget(const EffectBudget &) = delete;
    EffectBudget &operator=(const EffectBudget &) = delete;

private:
    qint64 m_left = 0;
    bool m_owner = false;
};
// How many pictures of effects have been made (not taken from those kept), for tests.
int effectPicturesBuiltForTest();

// How many outlines of fitted bullets (a missing symbol font's stand-in glyph
// scaled to Publisher's box) have been made, for the test that shows they are
// kept between paints.
int fittedMarkerOutlinesBuiltForTest();

} // namespace jp
