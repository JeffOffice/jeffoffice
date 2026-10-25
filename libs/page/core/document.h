#pragma once
// The publication: page setup, master pages, pages, the shared scratch area,
// text stories, embedded pictures, styles, schemes, business information and
// the mail-merge recipient list.

#include "core/items.h"
#include "core/pictureinsert.h"
#include "text/textengine.h"

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QImage>
#include <QJsonObject>
#include <QMap>
#include <QMutex>
#include <QSizeF>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <algorithm>
#include <memory>

class QTextDocument;

namespace jp {

constexpr double PT_PER_IN = 72.0;
inline double in2pt(double in) { return in * PT_PER_IN; }

// Where a picture stands with its file.
enum class LinkStatus { Embedded, Linked, Missing, Modified, NotUpdated };

struct ImageData {
    // The picture's file bytes. Of a link whose file can't be found and that
    // keeps no copy, the preview stands in (and `format` is the preview's).
    QByteArray bytes;
    QString format;             // "png", "jpg", "svg", ...
    QSize pixelSize;
    QString sourcePath;         // where it was inserted from (Graphics Manager); a link's file
    bool linked = false;        // tied to the file at sourcePath
    // Of a link:
    bool keepsCopy = true;      // the full picture is stored in the publication too (Insert and Link)
    int linkPage = 0;           // which page of a PDF file
    qint64 fileSize = 0;        // the file as it was when the link was made or updated,
    qint64 fileTime = 0;        // msecs since the epoch
    QByteArray fileHash;        // SHA-1 of the file's bytes, in hex: a file with other contents is a Modified
                                // one. Empty in files from before it was kept: size and date decide then.
    QByteArray preview;         // a small picture of it (at most 512 pixels across), for when the file is gone
    QString previewFormat;      // "jpg" or "png"
    QImage image() const;       // decoded, cached
    void makePreview();         // fills preview from the picture
    mutable QImage cache;
    // When Document::image last gave the decode out, in its own count: the
    // decode of the picture used longest ago is the first to go (see
    // Document::imageCacheBudget).
    mutable quint64 used = 0;
    // Not saved: when the publication opened, the file was read and its
    // contents were not the linked ones. (A status check looks only at the
    // size and date, and doesn't read the file.)
    bool changed = false;
    // Not saved: whether the file may be looked at. A link made in this
    // session is; one from a file that lies outside its publication's folder
    // is not, until Update Link or Change Link is used on its picture.
    bool followed = true;
};

struct Story {
    QString id;
    std::unique_ptr<QTextDocument> doc;
    quint64 serial = nextSerial();   // unique per Story object, so caches never confuse a rebuilt story
    static quint64 nextSerial();
};

struct RulerGuides {
    QVector<double> h, v;       // page coordinates
    // Where a new guide goes: at `at`, or, when a guide is already there,
    // `step` further on (back to the start past `limit`), so guides added
    // one after another don't sit on top of each other.
    static double freeSpot(const QVector<double> &list, double at, double step, double limit);
    // Adds `count` guides from `start`, `spacing` apart, within 0..limit and
    // skipping places that already have one; returns how many were added.
    static int addSeries(QVector<double> *list, double start, double spacing, int count, double limit);
};

struct GridGuides {
    int cols = 1, rows = 1;
    double colGap = 14.4, rowGap = 14.4;
    bool centerGuide = false;
    double baseline = 12, baselineOffset = 0;
    double hBaseline = 12, hBaselineOffset = 0;
    QJsonObject toJson() const;
    static GridGuides fromJson(const QJsonObject &o);
};

struct PageBase {
    QString id = newId("p");
    ItemList items;
    Fill background;
    RulerGuides guides;
    // Whether the objects of the master pages under this one show on it (a
    // slide or a slide layout can hide the graphics beneath).
    bool showMasterItems = true;
    // What an app keeps about the page besides its objects (a slide's
    // layout type, its transition); saved and undone with the page.
    QJsonObject props;
};

struct MasterPage : PageBase {
    QString name = QStringLiteral("Master Page");
    QString abbr = QStringLiteral("A");
    bool twoPage = false;
    GridGuides grid;
    // The master page drawn under this one, if any: a slide layout's slide
    // master. Objects whose role starts "ph:" are placeholder prompts: drawn
    // on the master itself, never under a page.
    QString baseId;
};

struct Page : PageBase {
    QString masterId = QStringLiteral("A"); // empty = no master
    QString title;
    QString notesStoryId;   // a slide's speaker notes
    bool hidden = false;    // a slide the slide show skips
};

struct PageSetup {
    enum Layout { OnePerSheet, Booklet, MultiplePerSheet, Envelope, FoldedCard, Labels };
    enum Fold { SideFoldQuarter, TopFoldQuarter, SideFoldHalf, TopFoldHalf };
    QSizeF size{612, 792};
    QMarginsF margins{36, 36, 36, 36};
    QString sizeName = QStringLiteral("Letter");
    Layout layout = OnePerSheet;
    Fold fold = SideFoldHalf;
    QSizeF sheet{612, 792};
    int gridRows = 1, gridCols = 1;
    double gapH = 0, gapV = 0, sideMargin = 0, topMargin = 0;
    // Format Page Numbers: the first page's number, and the style of the page
    // numbers that don't choose one: "" 1 2 3, "alpha" a b c, "ALPHA" A B C,
    // "roman" i ii iii, "ROMAN" I II III.
    int firstPageNumber = 1;
    QString pageNumberFormat;
    QJsonObject toJson() const;
    static PageSetup fromJson(const QJsonObject &o);
};

struct TextStyle {
    QString name;
    bool charOnly = false;
    QString basedOn;
    QString next;               // style for the following paragraph
    QTextCharFormat chr;
    QTextBlockFormat blk;
    QJsonObject toJson() const;
    static TextStyle fromJson(const QJsonObject &o);
};

struct BusinessInfo {
    QString setName = QStringLiteral("Primary Business");
    QString name, tagline, person, title, address, phone, fax, email, web;
    QString logoImageId;
    QString field(const QString &key) const;
    void setField(const QString &key, const QString &v);
    QJsonObject toJson() const;
    static BusinessInfo fromJson(const QJsonObject &o);
    static QStringList keys();
    static QString label(const QString &key);
};

struct MergeSource {
    QString path;
    QStringList fields;
    QVector<QStringList> rows;
    QVector<bool> include;      // per row
    QString pictureField;       // field holding picture file paths
    bool isEmpty() const { return fields.isEmpty(); }
    QString value(int row, const QString &field) const;
    QVector<int> includedRows() const;
    QJsonObject toJson() const;
    static MergeSource fromJson(const QJsonObject &o);
};

// Catalog merge: an area on one page that repeats for each record of the
// data source, in rows and columns of equal cells (filled across, then
// down). The objects in the first cell are the template the others copy.
struct CatalogArea {
    QString pageId;            // the page holding the area; empty = none
    QRectF rect;
    int rows = 2, cols = 1;
    bool isActive() const { return !pageId.isEmpty() && rect.width() > 1 && rect.height() > 1; }
    int perPage() const { return std::max(1, rows) * std::max(1, cols); }
    QRectF cell(int k) const;                 // cell k in reading order
    bool inTemplate(const QRectF &bounds) const;   // centered in the first cell
    QJsonObject toJson() const;
    static CatalogArea fromJson(const QJsonObject &o);
};

struct DocProps {
    QString title, subject, author, manager, company, category, keywords, comments;
    QDateTime created = QDateTime::currentDateTime(), modified = QDateTime::currentDateTime();
    QJsonObject toJson() const;
    static DocProps fromJson(const QJsonObject &o);
};

// Black overprinting in files for a printer (io/overprint applies it).
struct OverprintSettings {
    bool text = true;          // black text below textBelow points
    double textBelow = 24;
    bool lines = true;         // black lines and outlines
    bool fills = false;        // black fills
    int threshold = 95;        // a black of at least this percent counts
    bool any() const { return text || lines || fills; }
    bool operator==(const OverprintSettings &b) const {
        return text == b.text && textBelow == b.textBelow && lines == b.lines && fills == b.fills && threshold == b.threshold;
    }
};

struct PrintInfo {
    enum ColorModel { RGB, SingleSpot, SpotColors, ProcessCMYK, ProcessPlusSpot };
    ColorModel model = RGB;
    QVector<QColor> spotColors;
    QStringList spotNames;           // a name for each spot color (an ink's name, "PANTONE 286 C")
    bool usesSpots() const { return model == SingleSpot || model == SpotColors || model == ProcessPlusSpot; }
    QString spotName(int i) const { return i < spotNames.size() && !spotNames[i].isEmpty() ? spotNames[i] : QStringLiteral("Spot color %1").arg(i + 1); }
    bool embedFonts = true;   // kept from files; PDFs always embed their fonts (PDF/X needs them)
    OverprintSettings overprint;
};

// A movie or sound as its file held it (a slide's video), kept to be written
// back.
struct MediaData {
    QByteArray bytes;
    QString contentType;        // "video/mp4", "audio/wav", ...
    QString extension;          // "mp4", "wav", ...
};

// What a picture is held to when it is added and drawn, and what real files
// use: the reference presentations' largest picture has 9 million pixels,
// JeffPub's 284 sample publications' 101 million (a book cover at 300 dpi), and
// the largest metafile in either is under 1 MB. Of 10,377 SVG files found on
// disk, none has more than 889 parts or is nested deeper than 5.
// The most pixels a picture may have: 256 million, a decode of a gigabyte at
// four bytes a pixel (2.5 times the largest above; Qt's own limit on one decode
// is 2 GB, and a higher one would let one picture take the machine).
constexpr qint64 kMaxPicturePixels = 256LL * 1024 * 1024;
// The most bytes a metafile may be (as inflated, from a presentation file): 32 MB.
constexpr qint64 kMaxMetafileBytes = 32LL * 1024 * 1024;
// The decodes of all a document's pictures kept at once: 1 GiB, twice what the
// sample publication with the most pictures decodes to (510 MB).
constexpr qint64 kImageCacheBudget = 1024LL * 1024 * 1024;
// An SVG is drawn at most this many pixels on its long side.
constexpr int kMaxSvgRasterSide = 4096;
// An SVG with more parts to draw than this, counting each use of a part again
// where it is used (a few lines can build millions), or nested deeper, is not
// drawn. A part takes about 10 microseconds, so 100,000 take a second or two.
constexpr int kMaxSvgParts = 100000;
constexpr int kMaxSvgDepth = 128;

// Whether an SVG picture is safe to read and draw: well formed, with no
// script or foreign content, nothing that names a file or an address (a
// picture of its own inline is read, and held to the picture limit), no
// part that is drawn inside itself, and no more to draw than kMaxSvgParts
// once every use of a part is counted. Document::addImage asks this of every SVG.
bool svgIsSafe(const QByteArray &svg);

class Document : public FieldSource {
public:
    Document();
    ~Document();
    Document(const Document &) = delete;
    Document &operator=(const Document &) = delete;

    PageSetup setup;
    QVector<std::shared_ptr<MasterPage>> masters;
    QVector<std::shared_ptr<Page>> pages;
    ItemList scratch;
    // Extra Content: the stories and pictures Change Template found no place
    // for in the new design, until they are placed on a page or discarded.
    ItemList extra;
    QMap<QString, std::shared_ptr<Story>> stories;
    QMap<QString, ImageData> images;
    // How many pictures addImage has left out since the document was made:
    // those past a limit, and SVG it won't draw. An importer reads it for its notes.
    int imagesLeftOut = 0;
    // The most bytes the pictures' decodes kept in `images` come to. Past it,
    // the decodes of the pictures used longest ago are dropped (to three
    // quarters of it), and made again when drawn.
    qint64 imageCacheBudget = kImageCacheBudget;
    // The movies and sounds pictures play (PictureItem::mediaId). Like the
    // pictures' bytes they are not in toJson, so undoing an edit leaves them
    // be; nothing takes one away while the document is open.
    QMap<QString, MediaData> media;
    ColorScheme colors;
    FontScheme fonts;
    QVector<TextStyle> styles;
    QVector<BusinessInfo> biz;
    int bizCurrent = 0;
    MergeSource merge;
    CatalogArea catalog;
    DocProps props;
    PrintInfo print;
    QString templateId;
    QJsonObject templateOptions;
    bool facingPages = false;   // default spread display
    // Publisher's default tab stop: where the tab stops of a paragraph
    // without its own are, in points apart from the text area's edge.
    double defaultTabStop = 36;
    // Where the rulers count from, in points from the page's corner. Saved
    // with the publication, and left out of the file at the corner.
    QPointF rulerZero;
    // The fonts a .pub file named, when the publication came from one:
    // saved back to .pub under these names, not swapped for the standard
    // fonts JeffPub's look-alikes match (interchangeFontName).
    QStringList pubFonts;
    // Where the publication is saved, when it is: a linked picture's path
    // relative to it is kept, and wins when the publication and its pictures
    // were moved together.
    QString folder;

    // What an app keeps about the whole document besides its pages (a
    // presentation's theme and sections); saved and undone with it.
    QJsonObject appData;
    // The fonts the file this came from carries (a presentation's embedded fonts): each family they are for (lower-cased) and the private family
    // the font was added to the font database under, so that this document draws with them and nothing else does (LayoutEnv::familyAliases).
    // Never saved; set by the reader that adds the fonts.
    QHash<QString, QString> fontAliases;
    // Text is set as the presentation program sets a slide's (LayoutEnv::
    // slideLines), and what doesn't fit its box runs out of it, shown
    // (FrameSpec::runsOver), instead of waiting for the next box in the chain
    // or hiding as a publication's does. Set by the app that makes the
    // document; not saved.
    bool slideText = false;
    // The pages a flowing document (libs/flow) has, and the pages of the section this page is in, which its NUMPAGES and SECTIONPAGES fields show;
    // 0 leaves them to the document's own page count. Set by the app that makes the document; not saved.
    int flowPageTotal = 0, flowSectionPageTotal = 0;
    QString flowPagePrefix;   // what comes before the number a flowing page's fields show (a chapter's number and its separator)
    QHash<QString, QString> flowPageRefs;   // what a flowing page's references to bookmarks show (the page each is on); a bookmark that is not here shows what the file had
    // And what the page's notes need from the document it is a page of: the notes before its text, the footnotes cut at the foot of the page before
    // ("story|number|from" each), and the height of the paragraph between the notes and the text.
    int flowFootnotesBefore = 0, flowEndnotesBefore = 0;
    QStringList flowCarriedNotes;
    double flowNoteSeparator = 0;
    // Text is set as a word processor sets its lines (LayoutEnv::wordLines). Set by the app that makes the document; not saved.
    bool wordText = false;
    bool wordNoHang = false;   // (LayoutEnv::wordNoHang)
    bool wordSpaceSums = false;   // (LayoutEnv::wordSpaceSums)
    bool wordNoHangStop = false;   // (LayoutEnv::wordNoHangStop)
    bool wordNoNumberIndentStop = false;   // (LayoutEnv::wordNoNumberIndentStop)
    bool wordWholePointLines = false;   // (LayoutEnv::wordWholePointLines)
    bool wordNoLeading = false;   // (LayoutEnv::wordNoLeading)
    double wordSpaceShrink = 0;   // (LayoutEnv::wordSpaceShrink)
    QHash<QString, QString> wordStandIns;   // (LayoutEnv::standIns)
    // A picture's brightness, contrast, black and white, and transparent color change its colors as the presentation program changes a
    // slide's pictures (PictureItem::colorEffects), not as Publisher changes a publication's. Set by the app that makes the document; not saved.
    bool slidePictures = false;

    // pages
    QSizeF pageSize() const { return setup.size; }
    MasterPage *master(const QString &id) const;
    MasterPage *masterFor(const Page &p) const;
    // The master pages drawn for `m`, bottom first: its base's base, its
    // base, then `m` (empty for none). A loop of bases ends at the first repeat.
    QVector<MasterPage *> masterChain(const MasterPage *m) const;
    std::shared_ptr<Page> addPage(int at = -1, const QString &masterId = QStringLiteral("A"));
    int pageIndexOf(const QString &pageId) const;
    // After setup.size changed from `from`, as Publisher does it: nothing is
    // scaled, every object moves by half the change on each axis (it keeps its
    // distance from the page center), ruler guides keep their share of the
    // page, and an object left wholly off its page goes to the scratch area,
    // where it stays. On a two-page master the objects keep their places
    // across their page, and move half the change down.
    void pageSizeChanged(const QSizeF &from);

    // stories
    Story *story(const QString &id) const;
    QTextDocument *storyDoc(const QString &id) const override;
    // What its fields read (FieldSource).
    int firstPageNumber() const override { return setup.firstPageNumber; }
    QString pageNumberFormat() const override { return setup.pageNumberFormat; }
    QString pageNumberPrefix() const override { return flowPagePrefix; }
    QString pageRefText(const QString &bookmark) const override { return flowPageRefs.value(bookmark); }
    int pageTotal() const override { return flowPageTotal; }
    int footnotesBefore() const override { return flowFootnotesBefore; }
    int endnotesBefore() const override { return flowEndnotesBefore; }
    QStringList carriedNotes() const override { return flowCarriedNotes; }
    double noteSeparatorHeight() const override { return flowNoteSeparator; }
    int sectionPageTotal() const override { return flowSectionPageTotal; }
    QString businessField(const QString &key) const override { return business().field(key); }
    QStringList mergeFields() const override { return merge.fields; }
    QString mergeValue(int record, const QString &field) const override { return merge.value(record, field); }
    QList<const QTextDocument *> storyDocs() const override;
    QString createStory(const QString &text = QString());
    QString copyStory(const QString &id);   // deep copy, returns new id
    void removeStory(const QString &id);
    // Stories in use: those of text boxes, shapes and table cells, and those
    // their text refers to (a note's, an object set in text's own), on down.
    QSet<QString> storiesInUse() const;
    void applyDefaultFont(QTextDocument *d) const;

    // pictures
    // `decoded`, when the caller has it, is the picture as image() would
    // read it, saving the decoding. Without it only the picture's size is read
    // here, and it is decoded (and kept within imageCacheBudget) when it is
    // drawn. A picture left out comes back with no id, and is counted in
    // imagesLeftOut: one past kMaxPicturePixels, a metafile past kMaxMetafileBytes,
    // an SVG that svgIsSafe won't have, or an SVG that isn't called one.
    QString addImage(const QByteArray &bytes, const QString &format, const QString &sourcePath = QString(), const QImage &decoded = QImage());
    QImage image(const QString &id) const;
    // Keeps a movie or sound and returns its id; the same bytes kept again
    // get the same id.
    QString addMedia(const QByteArray &bytes, const QString &contentType, const QString &extension);
    QSize imageSize(const QString &id) const;
    // A picture tied to its file at `path`; `bytes` are the file's (see
    // readPictureFile), kept in the publication when `keepCopy`, else only
    // a small preview is.
    // `fileHash` is the file's, as readPictureFile gives it (made from `bytes`
    // when left out, unless those are a PDF page, which is not the file).
    QString addLinkedImage(const QByteArray &bytes, const QString &format, const QString &path, bool keepCopy, int pdfPage = 0, const QImage &decoded = QImage(),
                           const QByteArray &fileHash = QByteArray());
    LinkStatus linkStatus(const QString &imageId) const;
    // After a publication opens: reads each link's file again (a changed
    // file replaces the stored copy of an Insert and Link picture; a Link
    // picture draws from the file and is Modified until updated), and a
    // file that is gone leaves the stored copy or the preview. Only the
    // files `mayFollow` allows: any other link is not touched, and shows the
    // stored copy or the preview as Not updated.
    void refreshLinks();
    // Whether the publication came from a file. What a file names (a linked
    // picture, a mail merge list's picture) is then read only when it lies in
    // the publication's own folder or below, until the user asks for it.
    bool fromFile = false;
    // Whether `path` is in `folder` or below, however it is spelled: written
    // as text first (a network or device name, a `..` that leaves, a path
    // that is not absolute are out without anything being touched), then
    // symbolic links on the way are read, one by one and not followed, so
    // one that leads out is seen first.
    bool inFolder(const QString &path) const;
    // The folders the user has pointed this session at since the publication
    // opened (the recipient list chosen, so its pictures show): what a file
    // names there may be read too. Never saved or read from a file, so a
    // publication can't add to it.
    QStringList allowedFolders;
    void allowFolder(const QString &dir);
    // Whether what a file names at `path` may be read: any path, in a
    // publication made in this session; in one that came from a file, those
    // in its folder, or below it, or in an allowed folder.
    bool mayFollow(const QString &path) const;

    // items
    struct Loc { Item *item = nullptr; ItemList *list = nullptr; int index = -1; int page = -1; QString masterId; bool scratch = false; GroupItem *parent = nullptr; };
    // Where an item is, by its id. It is found in an index of where items were last seen, kept honest
    // by checking each answer against the pages and lists as they are now: so it follows pages and
    // lists changed directly (added to, removed from, moved between, replaced, loaded again) with
    // nothing to tell it, and a lookup costs a hash and a few comparisons, not a look at every item.
    // An item the index has not seen yet, or has seen move, costs a look at every item once.
    Loc find(const QString &itemId) const;
    // How many times find had to look through every item, for tests.
    int findScansForTest() const { return m_scans; }
    // Counted since the program began on this thread, for tests of cost: the pictures decoded (made into pixels, from their bytes), the bytes of stored pictures compared with
    // the bytes of one offered (only when it is not the very same buffer), and the SVG pictures loaded into a renderer.
    static qint64 imageDecodesForTest();
    static qint64 imageBytesComparedForTest();
    static qint64 svgLoadsForTest();
    // The items forEachItem has handed out on this thread since the program began: a count for tests of cost (a walk of the whole document for each item is the square).
    static qint64 itemsWalkedForTest();
    // How many times the index of where items are was made again, for tests.
    int findIndexingsForTest() const { return m_indexings; }
    Item *item(const QString &itemId) const { return find(itemId).item; }
    ItemPtr itemPtr(const QString &itemId) const;
    QVector<TextItem *> chainOf(const QString &frameId) const;   // whole linked chain in order
    TextItem *prevFrame(const QString &frameId) const;
    // Deep copy incl. stories. A copied connector comes loose from its
    // objects unless `ids` collects old -> new ids for remapGlue.
    ItemPtr cloneItem(const Item &it, QHash<QString, QString> *ids = nullptr);
    // Copies of several objects; connectors among them stay attached.
    ItemList cloneItems(const ItemList &items);
    void forEachItem(const std::function<void(Item *, int page, const QString &master)> &fn) const;
    // Moves connector ends to the connection sites they're attached to (after
    // those objects moved); ends whose object is gone come loose. Returns
    // whether any line changed.
    bool routeConnectors();

    // styles and info
    const TextStyle *style(const QString &name) const;
    // A style with what it doesn't set taken from the style it's based on
    // (and that one's, and so on). A null name when there is no such style.
    TextStyle resolvedStyle(const QString &name) const;
    BusinessInfo &business() { return biz[std::clamp(bizCurrent, 0, int(biz.size()) - 1)]; }
    const BusinessInfo &business() const { return biz[std::clamp(bizCurrent, 0, int(biz.size()) - 1)]; }

    // serialization (pictures are stored separately)
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &o);
    void resetDefaults();

    static std::unique_ptr<Document> blank(const QSizeF &size, const QString &sizeName = QStringLiteral("Letter"), int pageCount = 1);

private:
    // The picture with these bytes, if one is kept: by the buffer itself, or
    // by a hash of the bytes (compared only when the hashes agree).
    QString findImage(const QByteArray &bytes) const;
    void indexImage(const QString &id);
    void rebuildImageIndex() const;
    // A picture was decoded: counts its decode, and drops others' past the budget.
    void noteDecoded(const QString &id) const;
    // Drops the decodes of the pictures used longest ago, but not `keep`'s, when
    // those held and `room` more would pass the budget.
    void trimImageCache(const QString &keep, qint64 room = 0) const;
    mutable QMultiHash<size_t, QString> m_imageHashes;
    mutable QHash<const char *, QString> m_imageBuffers;
    mutable qsizetype m_imagesIndexed = 0;      // how many images the index above was made for
    mutable qint64 m_decodedBytes = 0;          // the decodes held, as last counted
    mutable quint64 m_imageClock = 0;
    QByteArray m_refusedSvg;                    // the last SVG left out, so that the same one again costs nothing
    // Where an item was when last seen: the item itself (an item that is gone is not found by it), the list it was in (a page's, a master's,
    // the scratch area's, or a group's children), and its place in that list, and the page or master that holds it (so that it can be found
    // again if the pages have moved about). Checked against the document as it is before it is believed.
    struct ItemSpot {
        std::weak_ptr<Item> item;
        std::weak_ptr<Item> parent;     // the group whose children it is in; empty at the top of a page, master, or scratch
        std::weak_ptr<PageBase> owner;  // the page or master
        int page = -1;                  // the page, or -1
        int master = -1;                // the master page's place in `masters`, or -1
        bool scratch = false;
        int index = -1;
    };
    Loc findScan(const QString &itemId, ItemSpot *spot) const;
    bool spotHolds(const QString &itemId, const ItemSpot &spot, Loc *out, int depth) const;
    // An item not where its spot says: looked for in the list it was in, or in the page or master that had it, and the spot made right.
    bool respot(const QString &itemId, ItemSpot *spot, Loc *out, int depth) const;
    void indexItems() const;
    mutable QHash<QString, ItemSpot> m_itemSpots;
    mutable qsizetype m_spotsMade = 0;          // how many it was made for
    mutable QMutex m_spotsLock;
    mutable int m_scans = 0, m_indexings = 0;
};

// The most a picture file can be, in bytes; a larger file isn't read.
constexpr qint64 kMaxPictureFile = 256 * 1024 * 1024;
// A picture file's bytes: of a regular file only (never a pipe or a device),
// from 1 byte to kMaxPictureFile, and never more than a byte past that.
// Empty when the file isn't one of those or can't be read.
QByteArray readPictureBytes(const QString &path);

// Reads a picture file as Insert Picture does: its bytes and format (a PDF as
// the one page `pdfPage` alone), and its size in pixels. `bytes` may already
// hold the file's bytes. False when it can't be read as a picture (readPictureBytes
// says which files are not read at all).
// `fileHash`, when given, gets the SHA-1 of the file's bytes in hex.
bool readPictureFile(const QString &path, int pdfPage, QByteArray *bytes, QString *format, QSize *pixels, QByteArray *fileHash = nullptr);

QVector<TextStyle> defaultStyles();

} // namespace jp
