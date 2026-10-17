// JeffDeck's tests: its presentation model on the shared page model, and its window.
// This is what the test classes (tst_deck_*.cpp) share: the files they build in memory, and the helpers that read them back.
#pragma once

#include "../../apps/pub/app/pubinfo.h"   // only its inline pubAppInfo(): the other app's colors, to compare
#include "app/animationdialogs.h"
#include "app/animationpane.h"
#include "app/commentspane.h"
#include "app/deckcanvas.h"
#include "app/deckdialogs.h"
#include "app/deckinfo.h"
#include "app/deckwidgets.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/filescreen.h"
#include "app/formatpane.h"
#include "app/paneframe.h"
#include "app/propertyeditors.h"
#include "app/deckfindpane.h"
#include "app/help.h"
#include "app/i18n.h"
#include "app/editormedia.h"
#include "app/mediadialogs.h"
#include "app/mediaplayer.h"
#include "app/onlinepictures.h"
#include "app/iconpicker.h"
#include "app/masterdialogs.h"
#include "app/masterpane.h"
#include "app/morph.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/presenterview.h"
#include "app/recovery.h"
#include "app/rehearsebar.h"
#include "app/reusepane.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/spelling.h"
#include "app/showdialogs.h"
#include "app/slideshow.h"
#include "app/showmedia.h"
#include "app/slidesorter.h"
#include "app/slidespane.h"
#include "app/textartgallery.h"
#include "app/textdialogs.h"
#include "app/themegallery.h"
#include "app/tableedit.h"
#include "app/telemetry.h"
#include "app/updater.h"
#include "app/widgets.h"
#include "core/animation.h"
#include "core/comments.h"
#include "core/deck.h"
#include "core/drawnwords.h"
#include "core/embeddedfonts.h"
#include "core/fontfiles.h"
#include "core/fonts.h"
#include "core/texteffects.h"
#include "core/ink.h"
#include "core/masteredit.h"
#include "core/media.h"
#include "core/reuse.h"
#include "core/sections.h"
#include "core/showsettings.h"
#include "core/themes.h"
#include "core/transition.h"
#include "io/cfb.h"
#include "io/deckoutput.h"
#include "io/odpexport.h"
#include "io/odpimport.h"
#include "io/pptimport.h"
#include "io/pptxexport.h"
#include "io/pptximport.h"
#include "io/pptxshow.h"
#include "io/pptxtransition.h"
#include "io/presentationfile.h"
#include "io/zip.h"
#include "office/chart.h"
#include "office/drawingml.h"
#include "office/drawingml_table.h"
#include "office/drawingml_write.h"
#include "office/tablestyles.h"
#include "office/eot.h"
#include "office/package.h"
#include "office/xml.h"
#include "render/pdfpage.h"
#include "render/renderer.h"
#include "render/shapes.h"
#include "render/textart.h"
#include "text/storyio.h"
#include "text/textprops.h"

#include <QAbstractButton>
#include <QApplication>
#include <QBuffer>
#include <QColorSpace>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QDeadlineTimer>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFontDatabase>
#include <QFrame>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontInfo>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSvgRenderer>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPrinter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QTcpServer>
#include <QScopeGuard>
#include <QTcpSocket>
#include <QSignalSpy>
#include <QSlider>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QProcess>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <QTimeZone>
#include <QTimer>
#include <QTranslator>
#include <QToolButton>
#include <QUrl>
#include <QAccessible>
#include <QWidgetAction>
#include <QXmlStreamWriter>
#include <cmath>
#include <functional>

#include "../testrunner.h"

using namespace jp;

namespace odptest {

extern const QString kNs;

extern const QString kBullets;

QString odpStyles();

QByteArray png(int w, int h, const QColor &color);

QByteArray odpZip(const QString &content, const QString &styles = odpStyles(), const QMap<QString, QByteArray> &extra = {},
                  const QByteArray &mime = "application/vnd.oasis.opendocument.presentation", const QString &manifest = QString(), bool mimeFirst = true);

// The content of a presentation: its own automatic styles, and `pages` (the draw:page elements) with `decls` before them.
QString odpContent(const QString &autoStyles, const QString &pages, const QString &decls = QString());

// The automatic styles of the sample presentation's content.
QString sampleAutomaticStyles();

QString samplePages();

QString sampleContent();

// The elements of a part with a name as written there ("draw:page"), at any depth, in order.
QList<QDomElement> byName(const QDomNode &from, const char *qualified);

} // namespace odptest

namespace deckhelp {
// Takes the addresses the slide show opens in place of a browser.
class LinkCatcher : public QObject {
    Q_OBJECT
public:
    QStringList urls;
public Q_SLOTS:
    void open(const QUrl &url) { urls << url.toString(); }
};
// Slides for the Slide Sorter and Outline tests: a Title Slide, then
// Title and Content slides, `titles` for their titles and `bodies` for
// the lines of their bodies (the Title Slide's subtitle), typed in the
// way that keeps the placeholders' formatting.
void fillSlides(DeckWindow &w, const QStringList &titles, const QList<QStringList> &bodies = {});
// The title of slide `i`, as its title placeholder holds it.
QString titleOfSlide(const Document &d, int i);
QStringList titlesOf(const Document &d);
// The story of slide `i`'s first body placeholder (a subtitle counts).
QTextDocument *bodyOfSlide(const Document &d, int i);
// The sections' names and slide counts, in order.
QStringList sectionNames(const Document &d);
QList<int> sectionCounts(const Document &d);
// A presentation of `n` Title and Content slides whose page titles are the letters A, B, C...
std::unique_ptr<Document> lettered(int n);
QString letters(const Document &d);
// Calls `poll` until it says it is done (the popup or box it waits for is up), or it has tried for a few seconds.
void untilDone(const std::function<bool()> &poll, int tries = 300);
// Saves a picture of a widget in the folder JP_SHOT_DIR names, when it is set (for looking at the new controls).
void shot(QWidget *w, const QString &name);
// Opens the right-click menu of a widget at `pos` and, while it is open, tells `inspect` about it; it closes after.
void rightClick(QWidget *viewport, const QPoint &pos, const std::function<void(QMenu *)> &inspect);
QByteArray le(quint64 v, int n);
// A record: its header (version, instance, type, length), then its data.
QByteArray pptRec(quint16 type, const QByteArray &data, quint16 inst = 0, quint8 ver = 0);
QByteArray pptBox(quint16 type, const QByteArray &kids, quint16 inst = 0);
QByteArray utf16(const QString &s);
// A color as the format stores it: red, green, blue, and a scheme index (0xFE: the RGB values are the color).
QByteArray raw(std::initializer_list<int> values);
QByteArray pptColor(int r, int g, int b, int index);
QByteArray fontEntity(const QString &name);
// The stream's name is fixed by [MS-PPT] section 2.1.2: the program's name and "Document".
QString pptDocumentStream();

// One text of a slide: a TextHeaderAtom with its type, the characters
// (8-bit unless `wide`), and the style runs (a StyleTextPropAtom).
QByteArray pptText(int type, const QString &s, const QByteArray &styles = QByteArray(), bool wide = false);
QByteArray pfRun(int count, int level, quint32 masks, const QByteArray &data = QByteArray());
QByteArray cfRun(int count, quint32 masks, const QByteArray &data = QByteArray());

QByteArray slidePersist(quint32 persistId, int texts, quint32 slideId);
// A slide's or master's SlideAtom: the layout geometry, the placeholders it names, the master and notes it refers to, and its flags.
QByteArray slideAtom(int geom, const QByteArray &placeholders, quint32 master, quint32 notes, int flags);
QByteArray colorScheme(int instance);
// A drawing: a group shape and the given shapes.
QByteArray pptDrawing(const QByteArray &shapes = QByteArray());
// A placeholder shape: a rectangle at `top, left, right, bottom` (master units), the placeholder it is, and what it holds (and its property record,
// its flags, and its kind of shape).
QByteArray pptShape(quint32 id, int placeholder, const QByteArray &textbox, int top = 100, int left = 200, int right = 5000, int bottom = 900, const QByteArray &props = QByteArray(), quint32 flags = 0xA00, quint16 type = 1,
                           const QByteArray &clientExtra = QByteArray());
QByteArray outlineRef(int index);

// A UserEditAtom, and a PersistDirectoryAtom mapping the given persist ids (each next to the last) to offsets.
QByteArray userEdit(quint32 lastEdit, quint32 directory, quint32 document, int extra = 0);
QByteArray persistDirectory(const QVector<QPair<quint32, quint32>> &entries);

// A presentation of two slides on one master, in the order First (persist
// id 5, with notes) and Second (4, hidden by the newer of two edits); with
// `titleMaster`, a slide on a title master; with `links`, a slide whose text
// has a field, a link, a language, and a ruler, and that fades in. `cycle`
// makes the older edit point back at the newer. `notesByAtom` leaves the
// first slide without the id of its notes, which then name the slide.
// `titleMasterRecord` is the kind of record the title master is kept in (the presentation program keeps it as a slide record).
// `bodyGroup` stands for the first slide's body text; `drawingGroup` is the document's drawing group, `firstSlideShapes` more
// shapes for the first slide's drawing, `masterTitleProps` a property record for the main master's title shape, `masterBackground`
// a shape for its drawing, `masterExtra` more records for the main master, `documentExtra` more records for the document, and `firstSlideExtra` more
// records for the first slide (a headers and footers record of its own), and `titleMasterShapes` shapes for the title master's drawing.
QByteArray buildPpt(bool titleMaster = false, bool cycle = false, bool links = false, bool notesByAtom = false, const QByteArray &bodyGroup = QByteArray(),
                           const QByteArray &drawingGroup = QByteArray(), const QByteArray &firstSlideShapes = QByteArray(), const QByteArray &masterTitleProps = QByteArray(), const QByteArray &masterBackground = QByteArray(),
                           const QByteArray &masterExtra = QByteArray(), quint16 titleMasterRecord = 1016, const QByteArray &documentExtra = QByteArray(),
                           const QByteArray &firstSlideExtra = QByteArray(), const QByteArray &titleMasterShapes = QByteArray());

// The text of a slide's placeholders by role.
QString placeholderText(const Document &d, const Page &slide, const QString &role);
QTextDocument *placeholderStory(const Document &d, const Page &slide, const QString &role);

// The words of a metafile record's parameters, a record, and a placeable metafile of a window 400 by 300 units around records.
QByteArray wmfWords(std::initializer_list<int> v);
QByteArray wmfDword(quint32 v);
QByteArray wmfRecord(int fn, const QByteArray &params);

// The first table on the slide being edited, and the last picture (an undo makes the objects over again, so these are looked up each time).
TableItem *theTable(Editor *ed);

// Runs a command by its id; false (and a failure that names it) when the window has none.
bool command(DeckWindow &w, const char *id);

const PictureItem *firstPicture(const Document &d);

TextEffects shadowOf(const QColor &color, double blur, double distance, double angle);

// `n` content slides, each titled and with notes of its own, so that
// each draws differently.
std::unique_ptr<Document> showDeck(int n);

deck::Transition showTransition(const char *type, double seconds);

// A slide's page props "timingXml" and the shapes it animates need a
// number each (the file's): the slide's title is 2 and its body 3.
void numberShowShapes(Page &slide);

// A slide painted with the page's hooks, or without: a picture the size of the slide.
QImage paintedSlide(Document *d, LayoutCache *cache, int slide, const std::function<void(PaintContext &)> &hook = nullptr);

// ---- animations: slide timings as the presentation program writes them ----

// XML written with ' where an attribute's quotes go, so that it reads without escapes.
QString animXml(const char *xml);

int animId();

QString animTarget(int spid, int paragraph = -1);

// What an entrance starts with and an exit ends with: the shape is shown or hidden at `delay` ms.
QString animShow(int spid, bool visible, int delay = 0, int paragraph = -1);

QString animFilter(int spid, const char *filter, bool entering, int ms, int paragraph = -1);

// An effect: a node of the step's tree with a preset and its behaviors.
QString animNode(const char *cls, int preset, int subtype, const char *nodeType, int delay, const QStringList &behaviors);

QString animFadeIn(int spid, int ms, const char *nodeType = "clickEffect", int delay = 0, int paragraph = -1);

QString animAppear(int spid, const char *nodeType = "clickEffect", int delay = 0);

QString animGroup(int delay, const QStringList &effects);

// A click step; one that starts by itself when the slide is shown has the main sequence's start as a second condition.
QString animStep(const QStringList &groups, bool automatic = false);

// Each effect in a step of its own.
QString animSteps(const QStringList &effects);

QString animTiming(const QString &steps, const QString &after = QString());

// Where the shapes the samples aim at are, on a 16:9 slide.
deck::Geometry animGeometry();

bool animNear(double a, double b);

// ---- editing animations ----

// A slide with shapes numbered as a file would number them: the title 2, the body 3, and more boxes from 10 on.
std::unique_ptr<Document> editDeck(int boxes = 0);

// ---- what the review of real slides found ----

// The problem with a timing's time nodes, "" for none: each has an id of its own, and what refers to a node refers to one there is.
QString animIdProblem(const QString &timing);

// ---- the show plays them ----

// Slide 1: the title fades in at the first click, the body appears at the second.
std::unique_ptr<Document> animatedDeck(int fadeMs);
// Types `text` in the first slide's title placeholder and leaves it being
// typed in, with from..to selected when that is a range. Returns the story.
QTextDocument *editTitle(DeckWindow &w, const QString &text, int from, int to);

struct Rel {
    QString source, type, target;   // type: the last word of one of the standard's, or a whole URI
    bool external = false;
};
struct Part {
    QString type;
    QByteArray xml;
};
// A presentation file as parts and relationships, so a test can change
// one before it is packed.
struct Parts {
    QMap<QString, Part> data;
    QVector<Rel> rels;
};

QString nsDecl();
const QString &relBase();
QByteArray tinyPng();
// A shape tree's start, and its end.
QString treeStart();
QString treeEnd();
QString placeholderShape(int id, const QString &name, const QString &ph, const QString &xfrm, const QString &body);
QString xfrm(qint64 x, qint64 y, qint64 cx, qint64 cy);
QString para(const QString &text);
QByteArray themeXml();
QString masterXml();
QString layoutXml(const QString &type, const QString &name, const QString &attrs, const QString &shapes);
QString slideXml(const QString &attrs, const QString &background, const QString &shapes, const QString &after = QString());
QString pictureShape(int id, qint64 x, qint64 y);
// A frame of a kind the reader doesn't know (a table by another name, as the reader reads tables), which is kept as it was.
QString unknownFrame(int id);
// A chart the reader doesn't know, standing in a pair of alternatives
// with a picture of it.
QString chartWithPicture(int id);

// An embedded object with a picture of itself, in a pair of alternatives for programs that
// know the other kind of drawing.
QString objectWithPicture(int id);

// The sample presentation: 16:9, one master over two layouts (Title
// Slide, Title and Content), and two slides.
Parts samplePptx();
QByteArray pack(const Parts &p);
// A comment of the older form (p:cm). `bias`: the author's offset from UTC in minutes
// (INT_MIN: none given); `parent`: "author:idx" of the comment it answers.
QString legacyComment(int author, int idx, const QString &when, qint64 x, qint64 y, const QString &text, int bias = INT_MIN, const QString &parent = QString());
QString legacyCommentList(const QString &body);
// The sample presentation with comments in the older form on its first slide: a comment
// by Pat and Lee's answer to it, then Lee's own, in the offset-free and the UTC form.
Parts withLegacyComments();
// Whether the shared reader reads pictures yet (a picture is the one
// shape these tests need that its first version leaves out).
bool readerReadsPictures();
QString textOf(const Document &d, const Item &it);
QByteArray imageBytes(const QImage &img, const char *format = "PNG");
// A 4 by 4 picture of one color (a different one from tinyPng's, which the theme's first accent matches).
QByteArray colorPng(const QColor &color);
QString dspNamespace();
QString diagramDrawingType();
// One shape of a diagram's drawing, as the presentation program writes it: the shape, its style, its text, and the rectangle the text goes in.
QString diagramShape(const QString &place, const QString &geometry, const QString &fill, const QString &text, const QString &textPlace);
QString diagramPlace(qint64 x, qint64 y, qint64 cx, qint64 cy);
// The diagram's drawing: three shapes (a rectangle in the theme's first accent color, a rounded one with its text inset, a rectangle filled with a
// picture of the drawing part's own), in the space of the frame, which is 3657600 by 1828800.
QByteArray diagramDrawing();
// The sample presentation with a diagram (SmartArt) on its first slide, as the presentation program writes one: the frame names the diagram's
// data, layout, quick style, and colors by the slide's links to them; the slide also links to the drawing part (the shapes as they were
// drawn), which the data part names by that link's id (dsp:dataModelExt). `alone`: the slide has nothing else, so the links
// are numbered as the presentation program numbers them (the layout 1, the data 2, and on to the drawing's 6).
Parts smartArtPptx(bool alone = false, bool withDrawing = true);
// The picture a kept frame of one kind ("diagram", "equation") is on the slide: the one the slide's kept frames of that kind name.
const PictureItem *keptPicture(const Document &d, const Page &page, const QString &kind);
// The place an a:xfrm (or p:xfrm) says, in EMU: x, y, cx, cy.
QVector<qint64> placeOf(const QDomElement &xfrm);
QString mathNamespace();
QByteArray equationPicture();
// An equation as the presentation program writes one: a pair of alternatives, the shape with its text (the math in a14:m, among the
// runs of the paragraph) for the programs that know it, and a picture of the shape for the others. `fallback`: "pic" (a picture) or "shape" (a
// shape filled with the picture, as an older version wrote it). `mixed`: a text box with other text around the equation.
QString equationAlternative(int id, int embed, const QString &fallback = QStringLiteral("pic"), bool mixed = false);
// The sample presentation with an equation on its first slide (the picture of it has a link of its own).
Parts equationPptx(const QString &fallback = QStringLiteral("pic"), bool mixed = false, bool withPicture = true);
// The sample presentation with a chart on its first slide: a column chart of two series, the workbook it takes its numbers from, a
// drawing of shapes laid over it (with a picture of its own), and a theme of its own.
Parts chartPptx();
// The picture a chart is on the slide: the one the slide's kept charts name.
const PictureItem *chartPicture(const Document &d, const Page &page);

// The presentation of the sample with its first slide's picture (a small square of one color, 182, 66, 15) given `blip`.
std::unique_ptr<Document> deckWithPictureBlip(const QString &blip, QString *err);
// The color at the middle of that picture, as the slide is drawn.
QColor colorOfSamplePicture(Document &d);
bool colorsNear(const QColor &a, const QColor &b, int by = 1);
// An exported file as the tests below look at it.
struct Exported {
    QMap<QString, QByteArray> zip;   // by entry name, no leading slash
    office::Package pkg;
};
bool unpack(const QByteArray &bytes, Exported *out, QString *why);
QString part(const Exported &e, const QString &name);
// The first slide, drawn.
QImage slideImage(Document &doc);

// The first thing wrong with a package, as a reader that stops at the first
// damage would find it; empty when it is sound: every part has a content
// type and every type names a part, every XML part parses, every
// relationship and every r:id in a part resolves, and every part is
// reached from the package's relationships.
QString packageProblem(const Exported &e);
QDomElement rootOf(const QByteArray &xml, QDomDocument *dom);
QStringList relationshipTargets(const Exported &e, const QString &source, const QString &typeSuffix);

// Answers the modal dialogs that open next, one for each answer: a button of a
// message box, or (NoButton) closing the dialog. The first label clicked is kept in `label`.
void answerDialogs(QList<QMessageBox::StandardButton> answers, QString *label = nullptr, QDeadlineTimer until = QDeadlineTimer(10000));
QByteArray fileBytes(const QString &path);

// Where the text of a placeholder is set: its first run's color.
QColor textColorOf(const Document &d, const TextItem &t);

const TextItem *placeholderOn(const Page &slide, const QString &type);

QStringList layoutTypes(const Document &d);
const Item *placeholderOn(const PageBase &page, const QString &type);
// The pages a PDF's page tree holds (a "/Type /Pages" node is not one).
int pdfPageCount(const QByteArray &pdf);
// Sets up the dialog that asks what to print, which opens next, and clicks its `button`.
void answerOutputDialog(const std::function<void(QDialog *)> &set = nullptr, QDialogButtonBox::StandardButton button = QDialogButtonBox::Ok,
                               QDeadlineTimer until = QDeadlineTimer(10000));

} // namespace deckhelp

using namespace deckhelp;

// What every test class does first. The tests never touch the user's own settings: they go to a folder that is
// made once, for the whole program.
void initDeckTestCase();
