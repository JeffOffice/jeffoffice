#include "core/sheetobjects.h"

#include "grid/gridsheet.h"
#include "office/chart_write.h"
#include "sheet/drawingadd.h"
#include "sheet/drawingsave.h"
#include "io/zip.h"
#include "sheet/xmlscan.h"
#include "office/theme.h"
#include "office/xml.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QRegularExpression>
#include <QPainter>
#include <QPointer>
#include <QSvgRenderer>
#include <QThreadPool>

#include <algorithm>
#include <cmath>
#include <set>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::grid::LayerObject;
using jp::grid::RenderRequest;

namespace {

// What the numbers and words a chart is drawn from come to (never 0).
quint64 dataKey(const jp::office::Chart &c)
{
    quint64 h = 1469598103934665603ull;
    auto mix = [&h](quint64 v) { h = (h ^ v) * 1099511628211ull; };
    auto text = [&](const QString &t) { mix(quint64(qHash(t, 7))); mix(quint64(t.size())); };
    text(c.title.text);
    for (const jp::office::ChartAxis &a : c.axes) text(a.title.text);
    for (const jp::office::ChartPlot &plot : c.plots)
        for (const jp::office::ChartSeries &s : plot.series) {
            text(s.name);
            mix(quint64(s.categories.size()));
            for (const QString &t : s.categories) text(t);
            mix(quint64(s.values.size()));
            for (const double v : s.values) mix(quint64(qHash(v, 11)));
            mix(quint64(s.categoryNumbers.size()));
            for (const double v : s.categoryNumbers) mix(quint64(qHash(v, 13)));
            mix(quint64(s.xValues.size()));
            for (const double v : s.xValues) mix(quint64(qHash(v, 17)));
        }
    return h | 1;
}

// What the drawing code may keep for the whole workbook, as a presentation's import may (the sample workbooks' largest drawing holds 1,889 shapes, 313 texts, and 75 pictures).
constexpr qint64 kBudgetObjects = kMaxWorkbookObjects, kBudgetStories = 30000, kBudgetParagraphs = 300000, kBudgetRuns = 500000, kBudgetLetters = 20000000;
// A chart part, once inflated, is read only if it is no larger than this (the largest real one is 1.8 MB).
constexpr qint64 kMaxChartPartBytes = qint64(32) << 20;
// A sheet's chart parts' references are read for saving from at most this many parts.
constexpr int kMaxChartParts = 20000;
// When rows or columns are inserted or deleted, the charts on the screen are looked at to see whether they would be drawn from other numbers: at most this many points are read for it in all
// (past it, a chart is drawn again without looking).
constexpr qint64 kMaxMovedChartCells = 400000;

// How far a stroke reaches beyond the edge of the box, in the grid's units at 100% (points are 4/3 of them).
double strokeReach(const Item &item, int depth)
{
    double reach = item.stroke.width * 4.0 / 3.0 / 2.0 + 2.0;
    if (item.type() == ItemType::Line) reach += 4.0;   // (a line's arrow heads)
    if (item.type() == ItemType::Group && depth < 64)
        for (const ItemPtr &c : static_cast<const GroupItem &>(item).children) reach = std::max(reach, strokeReach(*c, depth + 1));
    return std::min(reach, 24.0);
}

bool hasLine(const Item &item, int depth)
{
    if (item.type() == ItemType::Line) return true;
    if (item.type() == ItemType::Group && depth < 64)
        for (const ItemPtr &c : static_cast<const GroupItem &>(item).children)
            if (hasLine(*c, depth + 1)) return true;
    return false;
}

bool sameEdit(const StructEdit &a, const StructEdit &b)
{
    return a.kind == b.kind && a.sheet == b.sheet && a.first == b.first && a.count == b.count && a.row1 == b.row1 && a.col1 == b.col1 && a.row2 == b.row2 && a.col2 == b.col2 && a.toSheet == b.toSheet &&
           a.toRow == b.toRow && a.toCol == b.toCol && a.dRow == b.dRow && a.dCol == b.dCol;
}

// A chart made into a picture: this runs on a worker, and touches nothing but what it is given.
QImage drawChart(const Chart &chart, const QSizeF &points, const RenderRequest &q)
{
    const QByteArray svg = chartSvg(chart, points);
    QSvgRenderer renderer(svg);
    if (!renderer.isValid()) return QImage();
    QImage img(q.pixels, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    renderer.render(&p, q.inner);
    return img;
}

QImage nothing(const RenderRequest &q)
{
    QImage img(q.pixels, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    return img;
}

} // namespace

SheetObjects::SheetObjects(QObject *parent) : QObject(parent), m_doc(std::make_unique<jp::Document>()) {}

SheetObjects::~SheetObjects()
{
    m_layer.setObjects({});   // (the pictures' draw functions refer to this)
}

void SheetObjects::open(const std::shared_ptr<Workbook> &book)
{
    ++m_generation;
    m_layer.setObjects({});
    m_book = book;
    m_cells.reset();
    m_source = nullptr;
    m_values = nullptr;
    m_sheets.clear();
    m_applied.clear();
    m_partRefs.clear();
    m_chartSheet.clear();
    if (m_book) {
        m_sheets.resize(size_t(m_book->info().sheets.size()));
        // the sheet each chart is on, by the drawing of the sheet (its relationships only: no part is read)
        const Package &package = m_book->package();
        for (int sheet = 0; sheet < m_book->info().sheets.size(); ++sheet) {
            const QString drawing = drawingPartOf(package, m_book->info().sheets[sheet].part);
            if (drawing.isEmpty()) continue;
            for (const Relationship &r : package.relationshipsOfType(drawing, QStringLiteral("/chart")))
                if (!r.external) m_chartSheet.insert(r.target.toLower(), sheet);
        }
    }
    m_doc = std::make_unique<jp::Document>();
    m_layoutCache.clear();
    m_images.clear();
    m_budget = DrawingBudget();
    m_contextRead = false;
    m_shown = -1;
    m_objectsMade = 0;
    m_chartPartsRead = 0;
}

void SheetObjects::setCells(const WorkbookSource *source, const jp::grid::ValueStore *values)
{
    m_source = source;
    m_values = values;
    m_cells.reset();
    if (!m_book || !source) return;
    m_cells = std::make_unique<jp::grid::WorkbookChartCells>(source, values, &m_book->styles(), m_book->info().dateSystem);
    // the charts shown are drawn again, from the cells
    m_layer.invalidateAll();
}

bool SheetObjects::isChartSheet(int sheet) const
{
    return m_book && sheet >= 0 && sheet < m_book->info().sheets.size() && m_book->info().sheets[sheet].kind == SheetKind::Chartsheet;
}

QStringList SheetObjects::sheetNames() const
{
    QStringList names;
    if (m_book)
        for (const SheetEntry &e : m_book->info().sheets) {
            if (!e.isNew) {
                names << e.fileName;
                continue;
            }
            // a sheet a person added is named in references by the name it was made with (the renames since are followed after the edits)
            QString name = e.deleted ? QString() : e.name;
            if (m_sheetOps && !name.isEmpty()) {
                const std::vector<SheetOp> &ops = m_sheetOps();
                for (size_t i = ops.size(); i-- > 0;)
                    if (ops[i].kind == SheetOp::Kind::Rename && ops[i].newName.compare(name, Qt::CaseInsensitive) == 0) name = ops[i].name;
            }
            names << name;
        }
    return names;
}

QString SheetObjects::followSheetOps(const QString &ref) const
{
    if (!m_sheetOps || ref.isEmpty()) return ref;
    QString f = ref;
    for (const SheetOp &op : m_sheetOps()) {
        if (op.kind == SheetOp::Kind::Rename) f = renameSheetInFormula(f, op.name, op.newName);
        else f = deleteSheetInFormula(f, op.name);
    }
    return f;
}

void SheetObjects::refreshRefs(Object &o) const
{
    o.refs = o.movedRefs;
    for (QString &ref : o.refs) ref = followSheetOps(ref);
}

void SheetObjects::sheetsChanged()
{
    if (!m_book) return;
    // room for the sheets there are (one added or copied has no drawing; one taken away by Undo is let go of)
    m_sheets.resize(size_t(m_book->info().sheets.size()));
    if (m_shown >= int(m_sheets.size())) m_shown = -1;
    qint64 budget = kMaxMovedChartCells;
    for (size_t i = 0; i < m_sheets.size(); ++i)
        for (const std::shared_ptr<Object> &o : m_sheets[i].objects) {
            if (o->movedRefs.isEmpty() && o->baseRefs.isEmpty()) continue;
            const QStringList before = o->refs;
            refreshRefs(*o);
            if (o->refs != before && int(i) == m_shown && chartDataChanged(int(i), *o, &budget)) m_layer.invalidate(o->id);
        }
}

void SheetObjects::readContext()
{
    if (m_contextRead) return;
    m_contextRead = true;
    m_theme = defaultTheme();
    m_formats = QDomElement();
    m_themeDoc = QDomDocument();
    using B = DrawingBudget;
    m_budget.left[B::Objects] = kBudgetObjects;
    m_budget.left[B::Stories] = kBudgetStories;
    m_budget.left[B::Cells] = 50000;
    m_budget.left[B::Paragraphs] = kBudgetParagraphs;
    m_budget.left[B::Runs] = kBudgetRuns;
    m_budget.left[B::Letters] = kBudgetLetters;
    const Package &package = m_book->package();
    for (const Relationship &rel : package.relationshipsOfType(m_book->mainPart(), QStringLiteral("/theme"))) {
        if (rel.external || !package.hasPart(rel.target)) continue;
        const QByteArray xml = package.part(rel.target);
        Theme t;
        if (!readTheme(xml, &t)) break;
        m_theme = t;
        if (parseXml(xml, &m_themeDoc)) m_formats = child(child(m_themeDoc.documentElement(), ns::A, QStringLiteral("themeElements")), ns::A, QStringLiteral("fmtScheme"));
        break;
    }
    m_doc->fonts = {m_theme.name, m_theme.majorLatin, m_theme.minorLatin};
}

const jp::office::Theme &SheetObjects::theme()
{
    if (m_book) readContext();
    return m_theme;
}

SheetObjects::Sheet &SheetObjects::sheetState(int sheet)
{
    static Sheet none;
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || !m_book) return none;
    Sheet &s = m_sheets[size_t(sheet)];
    if (!s.listed) list(sheet, s);
    return s;
}

void SheetObjects::list(int sheet, Sheet &s)
{
    s.listed = true;
    readContext();
    const SheetEntry &entry = m_book->info().sheets[sheet];
    s.chartSheet = entry.kind == SheetKind::Chartsheet;
    s.drawingPart = drawingPartOf(m_book->package(), entry.part);
    if (s.drawingPart.isEmpty()) return;
    QString why;
    const std::shared_ptr<QDomDocument> parsedApart = std::move(s.parsedDoc);   // (what a worker made, if one did)
    s.parsedDoc.reset();
    if (!readDrawing(m_book->package(), s.drawingPart, &s.drawing, &why, parsedApart)) {
        s.counts.why << tr("a drawing that could not be read (%1)").arg(why);
        return;
    }
    DrawingCounts &c = s.counts;
    c.anchors = s.drawing.anchorsInPart;
    c.skipped = s.drawing.skipped;
    c.truncated = s.drawing.truncated;
    if (s.drawing.truncated) c.why << tr("more than %1 objects in a drawing; the rest are left out").arg(kMaxDrawingAnchors);
    s.context = baseContext();
    s.context->package = &m_book->package();
    s.context->part = s.drawingPart;
    s.nextId = int(s.drawing.objects.size());
    for (const DrawingObject &d : std::as_const(s.drawing.objects)) s.nextShapeId = std::max<qint64>(s.nextShapeId, d.id + 1);
    for (const DrawingObject &d : std::as_const(s.drawing.objects)) {
        switch (d.kind) {
        case ObjectKind::Chart: ++c.charts; break;
        case ObjectKind::Picture: ++c.pictures; break;
        case ObjectKind::Shape: ++c.shapes; break;
        case ObjectKind::Group: ++c.groups; break;
        case ObjectKind::Connector: ++c.connectors; break;
        case ObjectKind::Other: break;
        }
        if (d.hidden) {
            ++c.hidden;
            continue;
        }
        auto o = std::make_shared<Object>();
        o->id = d.index;
        o->meta = d;
        o->fileAnchor = o->base = o->anchor = d.anchor;
        o->fileName = d.name;
        replay(sheet, *o);
        s.objects.push_back(std::move(o));
    }
}

std::vector<StructEdit> SheetObjects::editsFrom(int at, int to) const
{
    const int end = to < 0 ? int(m_applied.size()) : std::min(to, int(m_applied.size()));
    if (at < 0) at = 0;
    if (at >= end) return {};
    return std::vector<StructEdit>(m_applied.begin() + at, m_applied.begin() + end);
}

QStringList SheetObjects::movedRefList(const QStringList &refs, int sheet, int at, int to) const
{
    QStringList out = refs;
    const std::vector<StructEdit> edits = editsFrom(at, to);
    if (edits.empty()) return out;
    const QStringList names = sheetNames();
    for (QString &ref : out) ref = movedChartRef(ref, names, edits, sheet);
    return out;
}

std::shared_ptr<DrawingContext> SheetObjects::baseContext()
{
    readContext();
    auto context = std::make_shared<DrawingContext>();
    DrawingContext &ctx = *context;
    ctx.theme = m_theme;
    ctx.colors = ColorMap();
    ctx.formats = m_formats;
    ctx.images = &m_images;
    ctx.budget = &m_budget;
    ctx.addImage = [this](const QByteArray &bytes, const QString &format) { return m_doc->addImage(bytes, format); };
    ctx.addMedia = [this](const QByteArray &bytes, const QString &type, const QString &extension) { return m_doc->addMedia(bytes, type, extension); };
    ctx.newStory = [this] { return m_doc->createStory(); };
    ctx.storyDoc = [this](const QString &id) { return m_doc->storyDoc(id); };
    return context;
}

void SheetObjects::replay(int sheet, Object &o)
{
    m_editSteps += qint64(m_applied.size());
    const int n = int(m_applied.size());
    // the anchor: the file's, and each place a person put it, with the edits made between them
    {
        DrawingAnchor anchor = o.base;
        int done = 0;
        for (const jp::crunch::AnchorPlacement &p : o.placements) {
            if (p.at > n) break;
            if (p.at > done) anchor = movedAnchor(anchor, sheet, editsFrom(done, p.at));
            done = std::max(done, p.at);
            anchor = p.anchor;
        }
        if (done < n) anchor = movedAnchor(anchor, sheet, editsFrom(done, n));
        o.anchor = anchor;
    }
    if (!o.baseRefs.isEmpty() || !o.refPlacements.empty()) {
        QStringList refs = o.baseRefs;
        int done = 0;
        for (const jp::crunch::RefPlacement &p : o.refPlacements) {
            if (p.at > n) break;
            if (p.at > done) refs = movedRefList(refs, sheet, done, p.at);
            done = std::max(done, p.at);
            refs = p.refs;
        }
        if (done < n) refs = movedRefList(refs, sheet, done, n);
        o.movedRefs = refs;
        refreshRefs(o);
    }
}

void SheetObjects::applyEdit(int sheet, Object &o, const StructEdit &edit)
{
    ++m_editSteps;
    const std::vector<StructEdit> one = {edit};
    o.anchor = movedAnchor(o.anchor, sheet, one);
    if (o.movedRefs.isEmpty()) return;
    const QStringList names = sheetNames();
    for (QString &ref : o.movedRefs) ref = movedChartRef(ref, names, one, sheet);
    refreshRefs(o);
}

void SheetObjects::ensureBuilt(int sheet, Object &o)
{
    if (o.built) return;
    o.built = true;
    Sheet &s = m_sheets[size_t(sheet)];
    auto leaveOut = [&](const QString &why) {
        ++s.counts.leftOut;
        s.counts.why << why;
    };
    if (m_objectsMade >= kMaxWorkbookObjects) {
        leaveOut(tr("more objects than the %1 a workbook may show").arg(kMaxWorkbookObjects));
        return;
    }
    if (o.meta.kind == ObjectKind::Chart) {
        const QString part = o.meta.chartPart;
        std::shared_ptr<const Chart> chart = s.parsed.value(part);
        if (!chart) {
            if (s.failedParts.contains(part)) {
                leaveOut(s.failedParts.value(part));
                return;
            }
            // (a part is read once, however many anchors name it)
            QString why;
            if (m_book->package().partSize(part) > kMaxChartPartBytes) {
                why = tr("a chart part that is too large (%1)").arg(o.meta.name);
            } else {
                auto parsed = std::make_shared<Chart>();
                ++m_chartPartsRead;
                QString error;
                if (readChart(m_book->package().part(part), m_theme, ColorMap(), parsed.get(), &error)) {
                    chart = parsed;
                    s.parsed.insert(part, chart);
                } else {
                    why = tr("a chart that could not be read (%1)").arg(error);
                }
            }
            if (!chart) {
                s.failedParts.insert(part, why);
                leaveOut(why);
                return;
            }
        }
        o.chart = chart;
        o.fileChart = chart;
        o.baseRefs = chartRefs(*chart);
        o.movedRefs = o.baseRefs;
        const QStringList names = sheetNames();
        if (!m_applied.empty())
            for (QString &ref : o.movedRefs) ref = movedChartRef(ref, names, m_applied, sheet);
        refreshRefs(o);
        o.made = true;
    } else {
        o.item = readShape(o.meta.element, o.ownContext ? *o.ownContext : *s.context);
        if (!o.item) {
            leaveOut(o.meta.kind == ObjectKind::Group ? tr("a group with nothing in it that is drawn") : tr("a shape or picture that could not be read"));
            return;
        }
        o.made = true;
    }
    ++m_objectsMade;
    ++s.counts.shown;
}

DrawingCounts SheetObjects::counts(int sheet)
{
    Sheet &s = sheetState(sheet);
    for (const std::shared_ptr<Object> &o : s.objects) ensureBuilt(sheet, *o);
    s.counts.why.removeDuplicates();
    return s.counts;
}

std::function<QImage()> SheetObjects::prepareChart(int sheet, Object &o, const RenderRequest &q)
{
    ensureBuilt(sheet, o);
    if (!o.chart) return {};
    // what the picture needs is taken here (the cells it reads are the window's); the picture is made from that alone
    auto chart = std::make_shared<Chart>(*o.chart);
    setChartRefs(chart.get(), o.refs);
    if (m_cells) {
        applyChartCells(chart.get(), sheet, *m_cells);
        if (o.added || o.chart != o.fileChart) applyChartFormats(chart.get(), sheet, *m_cells);   // (a chart made here has no format of its own: its cells' are what it shows its numbers in)
        o.reads = ChartReadIndex(jp::office::sheet::chartReads(*chart, sheet, *m_cells));
        o.drawnKey = dataKey(*chart);
        ++m_chartsDrawn;
    }
    const double side = kChartMaxSide;
    const QSizeF points(std::clamp(q.units.width() * 0.75, 1.0, side), std::clamp(q.units.height() * 0.75, 1.0, side));
    return [chart, points, q] { return drawChart(*chart, points, q); };
}

QImage SheetObjects::renderItem(Object &o, const RenderRequest &q)
{
    if (!o.item) return nothing(q);
    QImage img(q.pixels, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    const QRectF b = o.item->bounds();
    double sx = b.width() > 1e-6 ? q.inner.width() / b.width() : 0, sy = b.height() > 1e-6 ? q.inner.height() / b.height() : 0;
    if (sx == 0) sx = sy;
    if (sy == 0) sy = sx;
    if (sx == 0) return img;
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::TextAntialiasing);
    // the object's frame (as the file has it) is put in the box the anchor gives, whatever the two say; a line, with no width or no height, is put in the middle of the picture
    p.translate(q.inner.center());
    p.scale(sx, sy);
    p.translate(-b.center());
    PaintContext ctx;
    ctx.doc = m_doc.get();
    ctx.cache = &m_layoutCache;
    ctx.opt.output = true;
    Renderer::paintItem(&p, ctx, *o.item);
    return img;
}

LayerObject SheetObjects::layerObject(int sheet, const std::shared_ptr<Object> &o)
{
    LayerObject l;
    l.id = o->id;
    l.anchor = o->anchor;
    l.kind = o->meta.kind;
    l.name = o->meta.name;
    l.thin = o->meta.kind == ObjectKind::Connector;
    l.margin = 4;   // (until the object is made and says)
    l.hitInk = o->meta.kind != ObjectKind::Picture && o->meta.kind != ObjectKind::Chart;
    const bool chart = o->meta.kind == ObjectKind::Chart;
    if (chart) l.prepare = [this, sheet, o](const RenderRequest &q) { return prepareChart(sheet, *o, q); };
    else l.render = [this, o](const RenderRequest &q) { return renderItem(*o, q); };
    // the object is made when a picture of it is first wanted: what is left of it to draw, and its margin, are known then
    l.settle = [this, sheet, o, chart](LayerObject *lo) {
        ensureBuilt(sheet, *o);
        if (!o->made) {
            lo->render = nullptr;
            lo->prepare = nullptr;
            return;
        }
        if (chart) return;
        lo->thin = o->meta.kind == ObjectKind::Connector || hasLine(*o->item, 0);
        lo->margin = strokeReach(*o->item, 0);
    };
    return l;
}

bool SheetObjects::parseApart(int sheet, Sheet &s)
{
    if (s.listed) return false;
    if (s.parsing) return true;
    const QString part = drawingPartOf(m_book->package(), m_book->info().sheets[sheet].part);
    if (part.isEmpty() || m_book->package().partSize(part) <= kParseApartBytes) return false;
    s.parsing = true;
    const QByteArray bytes = m_book->package().part(part);   // (inflated here, where the package is used; the parsing, which is most of the time, needs no package)
    const QPointer<SheetObjects> self(this);
    const quint64 generation = m_generation;
    QThreadPool::globalInstance()->start([self, sheet, generation, bytes] {
        QString why;
        const std::shared_ptr<QDomDocument> doc = parseDrawing(bytes, &why);
        QMetaObject::invokeMethod(QCoreApplication::instance(), [self, sheet, generation, doc, why] {
            if (self) self->parsed(sheet, generation, doc, why);
        }, Qt::QueuedConnection);
    });
    return true;
}

void SheetObjects::parsed(int sheet, quint64 generation, const std::shared_ptr<QDomDocument> &doc, const QString &why)
{
    Q_UNUSED(why);   // (a drawing that could not be parsed is read again by list, which says why)
    if (generation != m_generation || sheet < 0 || size_t(sheet) >= m_sheets.size()) return;
    Sheet &s = m_sheets[size_t(sheet)];
    if (!s.parsing) return;
    s.parsing = false;
    if (!s.listed) {
        s.parsedDoc = doc;
        if (doc) ++m_parsedApart;
        list(sheet, s);
    }
    if (s.showPending && m_shown == sheet) showNow(sheet);
    s.showPending = false;
}

void SheetObjects::show(int sheet)
{
    if (m_book && sheet >= 0 && size_t(sheet) < m_sheets.size()) {
        Sheet &s = m_sheets[size_t(sheet)];
        if (parseApart(sheet, s)) {
            // (nothing of the sheet before is left over its rows: the objects come when the drawing is read)
            s.showPending = true;
            m_layer.setObjects({});
            m_layer.setFillsView(false);
            m_shown = sheet;
            return;
        }
    }
    showNow(sheet);
}

std::vector<jp::grid::LayerObject> SheetObjects::printObjects(int sheet)
{
    std::vector<jp::grid::LayerObject> out;
    if (!m_book || sheet < 0 || size_t(sheet) >= m_sheets.size()) return out;
    Sheet &s = sheetState(sheet);
    for (const std::shared_ptr<Object> &o : s.objects)
        if (!o->deleted && o->meta.prints) out.push_back(layerObject(sheet, o));
    return out;
}

void SheetObjects::showNow(int sheet)
{
    Sheet &s = sheetState(sheet);
    std::vector<LayerObject> objects;
    objects.reserve(s.objects.size());
    for (const std::shared_ptr<Object> &o : s.objects) objects.push_back(layerObject(sheet, o));
    m_shown = sheet;
    m_layer.setObjects(std::move(objects));
    for (const std::shared_ptr<Object> &o : s.objects)
        if (o->deleted) m_layer.setDeleted(o->id, true);
    m_layer.setFillsView(s.chartSheet);
    // (a workbook that is whole has the sheet's rows and columns as the file gives them: the objects that keep their size are measured now)
    if (m_cells) geometryKnown(sheet);
}

void SheetObjects::geometryKnown(int sheet)
{
    if (sheet != m_shown || !m_applied.empty()) return;
    // (a drawing that is being parsed has no objects yet: they are measured when it is listed and shown)
    if (!m_book || sheet < 0 || size_t(sheet) >= m_sheets.size() || !m_sheets[size_t(sheet)].listed) return;
    Sheet &s = sheetState(sheet);
    for (const std::shared_ptr<Object> &o : s.objects) {
        if (o->frozen || o->anchor.kind != AnchorKind::TwoCell || o->meta.anchor.editAs == AnchorKind::TwoCell) continue;
        bool ok = false;
        const jp::grid::ObjectBox b = m_layer.boxNow(o->id, &ok);
        if (!ok) continue;
        DrawingAnchor f = o->anchor;
        const qint64 emu = kEmuPerPixel;
        if (o->meta.anchor.editAs == AnchorKind::OneCell) {
            f.kind = AnchorKind::OneCell;
            f.cx = qint64(std::llround(b.w * double(emu)));
            f.cy = qint64(std::llround(b.h * double(emu)));
        } else {
            f.kind = AnchorKind::Absolute;
            f.x = qint64(std::llround(b.x * double(emu)));
            f.y = qint64(std::llround(b.y * double(emu)));
            f.cx = qint64(std::llround(b.w * double(emu)));
            f.cy = qint64(std::llround(b.h * double(emu)));
        }
        o->base = o->anchor = f;
        o->frozen = true;
        m_layer.setAnchor(o->id, f);
    }
}

bool SheetObjects::canDelete(int sheet, int id, QString *why)
{
    Sheet &s = sheetState(sheet);
    int ordinal = -1;
    for (const std::shared_ptr<Object> &o : s.objects)
        if (o->id == id) ordinal = o->meta.ordinal;
    if (ordinal < 0) return true;
    int same = 0;
    for (const DrawingObject &d : std::as_const(s.drawing.objects))
        if (d.ordinal == ordinal) ++same;
    if (same <= 1) return true;
    if (why) *why = tr("The file stores this object together with %n other(s), so they can't be deleted one at a time.", nullptr, same - 1);
    return false;
}

void SheetObjects::setDeleted(int sheet, int id, bool deleted)
{
    Sheet &s = sheetState(sheet);
    for (const std::shared_ptr<Object> &o : s.objects)
        if (o->id == id) {
            o->deleted = deleted;
            if (sheet == m_shown) m_layer.setDeleted(id, deleted);
            return;
        }
}

bool SheetObjects::isDeleted(int sheet, int id)
{
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects)
        if (o->id == id) return o->deleted;
    return false;
}

void SheetObjects::cellsChanged(const std::vector<jp::grid::ValueStore::Item> &cells)
{
    if (cells.empty() || !m_cells) return;
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        Sheet &s = m_sheets[i];
        for (const std::shared_ptr<Object> &o : s.objects) {
            if (!o->chart || o->reads.isEmpty()) continue;
            bool hit = false;
            for (const jp::grid::ValueStore::Item &c : cells)
                if (o->reads.reads(c.sheet, c.row, c.col)) {
                    hit = true;
                    break;
                }
            if (!hit) continue;
            if (int(i) == m_shown) m_layer.invalidate(o->id);
            else o->reads = ChartReadIndex();   // (drawn from the cells again when its sheet is shown)
        }
    }
}

void SheetObjects::cellsMoved()
{
    if (!m_edits) return;
    const std::vector<StructEdit> &edits = m_edits();
    if (m_cells) m_cells->sizesChanged();   // (a hidden row moves with the rows inserted or deleted above it)
    const bool oneMore = edits.size() == m_applied.size() + 1 && (m_applied.empty() || sameEdit(edits[edits.size() - 2], m_applied.back()));
    if (oneMore) {
        // the edit that is new is worked in
        for (size_t i = 0; i < m_sheets.size(); ++i)
            for (const std::shared_ptr<Object> &o : m_sheets[i].objects) applyEdit(int(i), *o, edits.back());
        m_applied.push_back(edits.back());
    } else {
        // an Undo (or anything else that is not one more): they are worked out again from what the objects were
        m_applied = edits;
        for (size_t i = 0; i < m_sheets.size(); ++i)
            for (const std::shared_ptr<Object> &o : m_sheets[i].objects) replay(int(i), *o);
    }
    if (m_shown >= 0 && size_t(m_shown) < m_sheets.size()) {
        // the objects are where the edits put them, as they were made (a picture is drawn again only if its size is not the one it has); a chart is drawn again if what it reads is not what it
        // was drawn from (a chart whose cells only moved with it is the same picture)
        qint64 budget = kMaxMovedChartCells;
        for (const std::shared_ptr<Object> &o : m_sheets[size_t(m_shown)].objects) {
            m_layer.setAnchor(o->id, o->anchor);
            if (o->chart && chartDataChanged(m_shown, *o, &budget)) m_layer.invalidate(o->id);
        }
    }
    m_layer.layoutChanged();
}

bool SheetObjects::chartDataChanged(int sheet, Object &o, qint64 *budget)
{
    if (!m_cells || o.drawnKey == 0) return false;   // (never drawn from the cells: it is drawn from them when it is first wanted)
    if (*budget <= 0) return true;
    Chart c = *o.chart;
    setChartRefs(&c, o.refs);
    applyChartCells(&c, sheet, *m_cells);
    qint64 points = 1;
    for (const ChartPlot &plot : std::as_const(c.plots))
        for (const ChartSeries &s : plot.series) points += s.values.size() + s.categories.size();
    *budget -= points;
    return dataKey(c) != o.drawnKey;
}

void SheetObjects::sizesChanged(int sheet)
{
    Q_UNUSED(sheet);
    if (m_cells) m_cells->sizesChanged();
    // (a chart that leaves out what is hidden is drawn again if what is hidden changed what it reads; the others come out the same)
    if (m_shown >= 0 && size_t(m_shown) < m_sheets.size()) {
        qint64 budget = kMaxMovedChartCells;
        for (const std::shared_ptr<Object> &o : m_sheets[size_t(m_shown)].objects)
            if (o->chart && o->chart->plotVisibleOnly && chartDataChanged(m_shown, *o, &budget)) m_layer.invalidate(o->id);
    }
    m_layer.layoutChanged();
}

namespace {

// Whether a cell that changed is in the block (the cells are sorted by row, then column).
template <class Cells>
bool changedIn(const Cells &cells, const jp::office::sheet::CellRange &r)
{
    auto it = std::lower_bound(cells.begin(), cells.end(), r.row1, [](const auto &c, quint32 row) { return c.row < row; });
    for (; it != cells.end() && it->row <= r.row2; ++it)
        if (it->col >= r.col1 && it->col <= r.col2) return true;
    return false;
}

bool nearly(const AnchorPoint &a, const AnchorPoint &b)
{
    return a.col == b.col && a.row == b.row && std::llabs(a.colOff - b.colOff) <= kEmuPerPixel / 2 && std::llabs(a.rowOff - b.rowOff) <= kEmuPerPixel / 2;
}

} // namespace

const QStringList &SheetObjects::partRefs(const QString &part)
{
    auto it = m_partRefs.find(part);
    if (it == m_partRefs.end()) it = m_partRefs.insert(part, chartPartRefs(m_book->package().part(part)));
    return it.value();
}

DrawingAnchor SheetObjects::twoCellFor(const Object &o, const jp::grid::Axis &rows, const jp::grid::Axis &cols) const
{
    const jp::grid::ObjectBox b = jp::grid::anchorBox(o.anchor, rows, cols);
    DrawingAnchor a = o.meta.anchor;
    a.kind = AnchorKind::TwoCell;
    a.x = a.y = a.cx = a.cy = 0;
    a.from = o.anchor.kind == AnchorKind::OneCell ? o.anchor.from : jp::grid::anchorPointAt(rows, cols, b.x, b.y);
    a.to = jp::grid::anchorPointAt(rows, cols, b.x + b.w, b.y + b.h);
    return a;
}

void SheetObjects::addToSave(SaveChanges *changes)
{
    if (!m_book) return;
    const QStringList names = sheetNames();
    const std::vector<StructEdit> none;
    const std::vector<StructEdit> &edits = m_edits ? m_edits() : none;
    QSet<QString> deletedParts, aliveParts, editedParts, taken;
    auto sheetGone = [&](int sheet) { return sheet >= 0 && sheet < m_book->info().sheets.size() && m_book->info().sheets[sheet].deleted; };
    bool namesRead = false;
    auto takenNames = [&]() -> QSet<QString> & {
        if (!namesRead) {
            namesRead = true;
            for (const QString &n : m_book->package().partNames()) taken.insert(n.toLower());
        }
        return taken;
    };
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        Sheet &s = m_sheets[i];
        if (!s.listed || sheetGone(int(i))) continue;
        std::shared_ptr<jp::grid::GridGeometry> geometry;
        auto geometryOf = [&]() -> jp::grid::GridGeometry & {
            if (!geometry) geometry = jp::grid::GridSheet::makeGeometry(m_source->sheet(int(i)), m_book->styles());
            return *geometry;
        };
        AddedDrawing add;
        s.savedDrawing.clear();   // (chosen again for each save: a name kept from a save that failed could be one another sheet's save now takes)
        // where each object will be among the children of the drawing's root: the file's, less those taken away, then those added; and the order they are drawn in
        std::vector<int> removedOrdinals;
        for (const std::shared_ptr<Object> &o : s.objects)
            if (o->deleted && !o->added && !o->gone && o->meta.ordinal >= 0) removedOrdinals.push_back(o->meta.ordinal);
        std::sort(removedOrdinals.begin(), removedOrdinals.end());
        const int survivors = std::max(0, s.drawing.rootChildren - int(removedOrdinals.size()));
        std::vector<std::pair<std::shared_ptr<Object>, int>> placed;   // (object, its place among the children after what is above), in the order they are drawn
        int addedSoFar = 0;
        bool shares = false;
        {
            std::set<int> used;
            for (const std::shared_ptr<Object> &o : s.objects) {
                if (o->deleted || o->gone) continue;
                if (o->added && !(m_source && m_cells && (o->chart || o->item))) {
                    o->savedPart.clear();   // (it cannot be written now: it stays as it is, not in the file, and is not said to be)
                    continue;
                }
                int at = 0;
                if (o->added) {
                    at = survivors + addedSoFar++;
                } else {
                    at = o->meta.ordinal - int(std::lower_bound(removedOrdinals.begin(), removedOrdinals.end(), o->meta.ordinal) - removedOrdinals.begin());
                    if (!used.insert(at).second) shares = true;
                }
                placed.emplace_back(o, at);
            }
        }
        std::vector<int> perm(size_t(survivors + addedSoFar));
        for (size_t k = 0; k < perm.size(); ++k) perm[k] = int(k);
        bool reordered = false;
        if (!shares && !s.drawingPart.isEmpty() && placed.size() > 1) {
            std::vector<int> slots;
            for (const auto &p : placed) slots.push_back(p.second);
            std::vector<int> desired = slots;
            std::sort(slots.begin(), slots.end());
            for (size_t k = 0; k < slots.size(); ++k) {
                perm[size_t(slots[k])] = desired[k];
                if (desired[k] != slots[k]) reordered = true;
            }
        }
        std::vector<int> inverse(perm.size());
        for (size_t k = 0; k < perm.size(); ++k) inverse[size_t(perm[k])] = int(k);
        for (const auto &p : placed) p.first->savedOrdinal = size_t(p.second) < inverse.size() ? inverse[size_t(p.second)] : p.second;
        if (reordered) changes->drawingOrder[s.drawingPart] = perm;
        for (const std::shared_ptr<Object> &o : s.objects) {
            if (o->deleted) {
                if (!o->added && !o->gone && !s.drawingPart.isEmpty()) {
                    changes->removedObjects[s.drawingPart].push_back(o->meta.ordinal);
                    if (!o->meta.chartPart.isEmpty()) deletedParts.insert(o->meta.chartPart.toLower());
                }
                continue;
            }
            if (o->gone) continue;
            if (o->added) {
                // a chart a person made: its part, and its anchor at the end of the drawing
                if (!m_source || !m_cells || (!o->chart && !o->item)) continue;
                if (!o->chart) {
                    // a picture or shape from a copy: its element, and its pictures as parts of their own
                    DrawingAnchor want = o->anchor;
                    if (want.kind != AnchorKind::TwoCell) want = twoCellFor(*o, geometryOf().layout.rows, geometryOf().layout.cols);
                    else want.editAs = o->anchor.editAs;
                    o->savedAnchor = want;
                    AddedObject obj;
                    obj.xml = objectAnchorXml(want, o->pasteNamespaces, withFirstObjectName(o->pasteXml, o->meta.name));
                    o->savedName = o->meta.name;
                    for (const jp::crunch::ClipMedia &m : std::as_const(o->pasteMedia)) {
                        AddedPart part;
                        part.part = freePartName(QStringLiteral("/xl/media/image"), QLatin1Char('.') + m.extension, takenNames());
                        takenNames().insert(part.part.toLower());
                        part.relType = QStringLiteral("image");
                        part.contentType = m.contentType;
                        part.bytes = m.bytes;
                        part.token = m.token;
                        obj.parts.push_back(std::move(part));
                    }
                    o->savedPart = QStringLiteral("-");   // (written: there is no part of its own to name)
                    add.objects.push_back(std::move(obj));
                    continue;
                }
                auto c = std::make_shared<Chart>(*o->chart);
                setChartRefs(c.get(), o->refs);
                applyChartCells(c.get(), int(i), *m_cells);
                applyChartFormats(c.get(), int(i), *m_cells);
                o->savedPart = freePartName(QStringLiteral("/xl/charts/chart"), QStringLiteral(".xml"), takenNames());
                takenNames().insert(o->savedPart.toLower());
                DrawingAnchor want = o->anchor;
                if (want.kind != AnchorKind::TwoCell) want = twoCellFor(*o, geometryOf().layout.rows, geometryOf().layout.cols);
                else want.editAs = o->anchor.editAs;
                o->savedAnchor = want;
                if (o->meta.id <= 0) o->meta.id = s.nextShapeId++;
                const QString token = QStringLiteral("@@chart%1@@").arg(add.objects.size());
                AddedObject obj;
                obj.xml = chartAnchorXml(want, int(o->meta.id), o->meta.name, token);
                o->savedName = o->meta.name;
                // (the part is written by the thread that writes the file: it is made from the model, which is a copy)
                AddedPart part;
                part.part = o->savedPart;
                part.relType = QStringLiteral("chart");
                part.contentType = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml");
                part.produce = [c] { return writeChartPart(*c); };
                part.token = token;
                obj.parts.push_back(std::move(part));
                add.objects.push_back(std::move(obj));
                continue;
            }
            if (!o->meta.chartPart.isEmpty()) aliveParts.insert(o->meta.chartPart.toLower());
            // what the file will have for the object: the anchor as the cells' edits leave it, or, for one a person moved or that keeps its size, as its size and place now give it
            o->savedName = o->meta.name;
            if (o->meta.name != o->fileName && !s.drawingPart.isEmpty() && o->meta.ordinal >= 0) changes->objectNames[s.drawingPart][o->meta.ordinal] = o->meta.name;
            const DrawingAnchor inFile = movedAnchor(o->fileAnchor, int(i), edits);
            o->savedAnchor = inFile;
            if (o->frozen && m_source) {
                const DrawingAnchor want = twoCellFor(*o, geometryOf().layout.rows, geometryOf().layout.cols);
                if (!nearly(want.from, inFile.from) || !nearly(want.to, inFile.to) || want.editAs != inFile.editAs) {
                    changes->anchors[s.drawingPart][o->meta.ordinal] = want;
                    o->savedAnchor = want;
                }
            } else if (o->anchor.kind == AnchorKind::TwoCell && !s.drawingPart.isEmpty() && (!nearly(o->anchor.from, inFile.from) || !nearly(o->anchor.to, inFile.to) || o->anchor.editAs != inFile.editAs)) {
                changes->anchors[s.drawingPart][o->meta.ordinal] = o->anchor;
                o->savedAnchor = o->anchor;
            }
            // a chart a person changed is written whole from the model, over its part
            if (o->chart && o->fileChart && o->chart != o->fileChart && !o->meta.chartPart.isEmpty() && m_source && m_cells) {
                auto c = std::make_shared<Chart>(*o->chart);
                setChartRefs(c.get(), o->refs);
                applyChartCells(c.get(), int(i), *m_cells);
                applyChartFormats(c.get(), int(i), *m_cells);
                changes->producedParts[o->meta.chartPart] = [c] { return writeChartPart(*c); };
                editedParts.insert(o->meta.chartPart.toLower());
            }
        }
        if (!add.objects.empty()) {
            if (s.drawingPart.isEmpty()) {
                if (s.savedDrawing.isEmpty()) s.savedDrawing = freePartName(QStringLiteral("/xl/drawings/drawing"), QStringLiteral(".xml"), takenNames());
                takenNames().insert(s.savedDrawing.toLower());
                add.part = s.savedDrawing;
            } else {
                add.part = s.drawingPart;
            }
            changes->addedObjects[int(i)] = std::move(add);
        }
    }
    // the caches of the charts whose cells changed or whose references moved, found in the parts of the file: a sheet that was never shown is not read for it
    if (!m_cells) return;
    int partsLooked = 0;
    static const QRegularExpression chartPartRe(QStringLiteral("/charts/chart\\d*\\.xml$"), QRegularExpression::CaseInsensitiveOption);
    for (const QString &part : m_book->package().partNames()) {
        if (!chartPartRe.match(part).hasMatch() || ++partsLooked > kMaxChartParts) continue;
        const QString key = part.toLower();
        if (deletedParts.contains(key) && !aliveParts.contains(key)) continue;
        if (editedParts.contains(key)) continue;   // (written whole from the model)
        const int own = m_chartSheet.value(key, -1);
        if (sheetGone(own)) continue;   // (the part goes with its sheet)
        for (const QString &ref : partRefs(part)) {
            // (as the saver writes the part: the edits to rows and columns, then the renames and deletes of sheets)
            const QString moved = followSheetOps(movedChartRef(ref, names, edits, std::max(own, 0)));
            bool dirty = moved != ref;
            QVector<ChartBlock> blocks;
            if (!chartBlocks(moved, own, *m_cells, &blocks)) continue;
            for (const ChartBlock &b : std::as_const(blocks)) {
                const auto sheetChanges = changes->sheets.find(b.sheet);
                if (dirty || sheetChanges == changes->sheets.end()) continue;
                dirty = changedIn(sheetChanges->second.cells, b.range) || changedIn(sheetChanges->second.results, b.range);
            }
            if (!dirty) continue;
            ChartRefData data;
            if (readChartRef(moved, own, *m_cells, kMaxChartSeriesCells, &data)) changes->chartCaches[part][moved] = data;
        }
    }
}

void SheetObjects::saved()
{
    m_partRefs.clear();   // (the parts are the new file's: their references are the moved ones)
    if (!m_applied.empty()) ++m_epoch;   // (the places steps of Undo hold are of the rows and columns before the edits the file now has)
    m_applied.clear();
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        Sheet &s = m_sheets[i];
        s.parsed.clear();
        s.failedParts.clear();
        // what is in the file now: the objects taken away are not (they stay in the list as they were, so that the places Undo holds are what they were), those added are, at the places
        // addToSave said
        int removedNow = 0, addedNow = 0;
        for (const std::shared_ptr<Object> &o : s.objects) {
            if (o->deleted) {
                if (!o->gone && !o->added) ++removedNow;
                o->gone = true;
                continue;
            }
            if (o->gone) continue;
            if (o->added && o->savedPart.isEmpty()) continue;   // (not written: still to be)
            if (o->added) {
                ++addedNow;
                o->added = false;
                if (o->meta.kind == ObjectKind::Chart) {
                    o->meta.chartPart = o->savedPart;
                    m_chartSheet.insert(o->savedPart.toLower(), int(i));
                }
            }
            if (o->savedOrdinal >= 0) o->meta.ordinal = o->savedOrdinal;
            o->base = o->anchor;
            o->placements.clear();
            o->refPlacements.clear();
            o->fileAnchor = o->savedAnchor;   // (what addToSave said the file has)
            o->baseRefs = o->refs;
            o->movedRefs = o->refs;
            o->fileChart = o->chart;
            o->fileName = o->savedName;
        }
        s.drawing.rootChildren = std::max(0, s.drawing.rootChildren - removedNow) + addedNow;
        QVector<DrawingObject> kept;
        for (DrawingObject d : std::as_const(s.drawing.objects))
            for (const std::shared_ptr<Object> &o : s.objects)
                if (o->id == d.index && !o->gone) {
                    d.ordinal = o->meta.ordinal;
                    kept << d;
                    break;
                }
        s.drawing.objects = kept;
        if (addedNow > 0 && s.drawingPart.isEmpty()) s.drawingPart = s.savedDrawing;
    }
}

SheetObjects::Object *SheetObjects::find(int sheet, int id)
{
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects)
        if (o->id == id) return o.get();
    return nullptr;
}

QString SheetObjects::unfollowSheetOps(const QString &ref) const
{
    if (!m_sheetOps || ref.isEmpty()) return ref;
    QString f = ref;
    const std::vector<SheetOp> &ops = m_sheetOps();
    for (size_t i = ops.size(); i-- > 0;)
        if (ops[i].kind == SheetOp::Kind::Rename) f = renameSheetInFormula(f, ops[i].newName, ops[i].name);
    return f;
}

int SheetObjects::addedCount() const
{
    int n = 0;
    for (const Sheet &s : m_sheets)
        for (const std::shared_ptr<Object> &o : s.objects)
            if (o->added && !o->deleted) ++n;
    return n;
}

qint64 SheetObjects::chartPointsHeld() const
{
    qint64 n = 0;
    for (const Sheet &s : m_sheets)
        for (const std::shared_ptr<Object> &o : s.objects)
            if (!o->deleted && !o->gone && o->chart && (o->added || o->chart != o->fileChart)) n += o->points;
    return n;
}

qint64 SheetObjects::pointsOf(int sheet, const Chart &chart) const
{
    qint64 n = 0;
    for (const ChartPlot &plot : chart.plots)
        for (const ChartSeries &s : plot.series) {
            auto cells = [&](const QString &ref, qint64 held) -> qint64 {
                if (ref.isEmpty() || !m_cells) return held;
                QVector<ChartBlock> blocks;
                if (!chartBlocks(ref, sheet, *m_cells, &blocks)) return held;
                qint64 total = 0;
                for (const ChartBlock &b : std::as_const(blocks)) total += std::min<qint64>(b.range.cellCount(), kMaxChartSeriesCells);
                return std::min<qint64>(total, kMaxChartSeriesCells);
            };
            n += cells(s.valueRef, s.values.size()) + cells(s.categoryRef, s.categories.size()) + cells(s.xRef, s.xValues.size()) + 8;   // (and what its name and settings hold)
        }
    return n;
}

bool SheetObjects::chartRoom(int sheet, const Chart &chart, int replacing, QString *why) const
{
    qint64 held = chartPointsHeld();
    if (replacing >= 0)
        for (const std::shared_ptr<Object> &o : m_sheets[size_t(sheet)].objects)
            if (o->id == replacing && !o->deleted && o->chart && (o->added || o->chart != o->fileChart)) held -= o->points;
    if (held + pointsOf(sheet, chart) > m_limits.maxPoints) {
        if (why) *why = QCoreApplication::translate("jp::crunch::SheetObjects", "The charts made or changed here since the last save already hold as many numbers as JeffCrunch keeps (%1). Save the workbook first.").arg(QLocale().toString(m_limits.maxPoints));
        return false;
    }
    return true;
}

void SheetObjects::knowShapeIds(int sheet, Sheet &s)
{
    if (s.shapeIdsKnown) return;
    s.shapeIdsKnown = true;
    Q_UNUSED(sheet);
    if (s.drawingPart.isEmpty()) return;
    // the largest number any shape of the part has, found by looking through the part (a drawing is read for at most kMaxDrawingAnchors, and the rest must not be given ids again)
    const QByteArray xml = m_book->package().part(s.drawingPart);
    xmlscan::Tag t;
    qsizetype pos = 0;
    qint64 most = 1;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "cNvPr") continue;
        bool found = false;
        const QByteArrayView v = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "id", &found);
        if (found) most = std::max(most, QByteArray(v).toLongLong());
    }
    s.nextShapeId = std::max(s.nextShapeId, most + 1);
}

bool SheetObjects::exists(int sheet, int id) { return find(sheet, id) != nullptr; }

ObjectKind SheetObjects::kindOf(int sheet, int id)
{
    const Object *o = find(sheet, id);
    return o ? o->meta.kind : ObjectKind::Other;
}

QString SheetObjects::nameOf(int sheet, int id)
{
    const Object *o = find(sheet, id);
    return o ? o->meta.name : QString();
}

std::vector<int> SheetObjects::idsOf(int sheet)
{
    std::vector<int> ids;
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects)
        if (!o->deleted) ids.push_back(o->id);
    return ids;
}

QString SheetObjects::newName(int sheet, ObjectKind kind)
{
    const QString base = kind == ObjectKind::Chart ? tr("Chart") : kind == ObjectKind::Picture ? tr("Picture") : tr("Shape");
    Sheet &s = sheetState(sheet);
    QSet<QString> used;
    for (const std::shared_ptr<Object> &o : s.objects) used.insert(o->meta.name);
    for (int n = 1;; ++n) {
        const QString name = QStringLiteral("%1 %2").arg(base).arg(n);
        if (!used.contains(name)) return name;
    }
}

ObjectState SheetObjects::stateOf(int sheet, int id)
{
    ObjectState st;
    Object *o = find(sheet, id);
    if (!o) return st;
    ensureBuilt(sheet, *o);
    st.placements = o->placements;
    st.chart = o->chart;
    st.name = o->meta.name;
    st.refs = o->refPlacements;
    st.deleted = o->deleted;
    st.epoch = m_epoch;
    const Sheet &s = sheetState(sheet);
    for (size_t k = 0; k < s.objects.size(); ++k)
        if (s.objects[k].get() == o) st.order = int(k);
    return st;
}

ObjectState SheetObjects::stateWithChart(int sheet, int id, const Chart &chart)
{
    ObjectState st = stateOf(sheet, id);
    st.chart = std::make_shared<const Chart>(chart);
    jp::crunch::RefPlacement p;
    p.at = int(m_applied.size());
    p.refs = chartRefs(chart);
    for (QString &ref : p.refs) ref = unfollowSheetOps(ref);
    st.refs.push_back(p);
    return st;
}

ObjectState SheetObjects::stateMoved(int sheet, int id, const DrawingAnchor &anchor)
{
    ObjectState st = stateOf(sheet, id);
    st.placements.push_back(jp::crunch::AnchorPlacement{int(m_applied.size()), anchor});
    return st;
}

void SheetObjects::applyState(int sheet, int id, const ObjectState &st)
{
    Object *o = find(sheet, id);
    if (!o) return;
    // (a state of before a save that followed edits to rows and columns has places that are of other rows and columns: what it says of them is not used)
    const bool placed = st.epoch == m_epoch;
    if (placed) {
        o->placements = st.placements;
        if (st.chart) {
            o->chart = st.chart;
            o->refPlacements = st.refs;
            o->built = o->made = true;
            o->drawnKey = 0;
            o->points = pointsOf(sheet, *st.chart);
        }
    }
    if (!st.name.isEmpty()) o->meta.name = st.name;
    replay(sheet, *o);
    o->frozen = o->anchor.kind != AnchorKind::TwoCell;
    o->meta.anchor.editAs = o->anchor.editAs;
    o->deleted = st.deleted;
    // a chart put on and taken away again lets go of its numbers (the step that put it on holds them, and gives them back)
    if (o->deleted && o->added && o->chart) {
        o->chart.reset();
        o->points = 0;
    }
    // its place among the others, in the order they are drawn
    Sheet &s = sheetState(sheet);
    size_t at = s.objects.size();
    for (size_t k = 0; k < s.objects.size(); ++k)
        if (s.objects[k].get() == o) at = k;
    if (placed && at < s.objects.size() && st.order >= 0 && size_t(st.order) < s.objects.size() && size_t(st.order) != at) {
        const std::shared_ptr<Object> keep = s.objects[at];
        s.objects.erase(s.objects.begin() + qsizetype(at));
        s.objects.insert(s.objects.begin() + st.order, keep);
        if (sheet == m_shown) m_layer.moveObject(o->id, size_t(st.order));
    }
    if (sheet != m_shown) return;
    m_layer.setName(o->id, o->meta.name);
    m_layer.setAnchor(o->id, o->anchor);
    m_layer.setDeleted(o->id, o->deleted);
    if (placed && st.chart) m_layer.invalidate(o->id);
    m_layer.layoutChanged();
}

int SheetObjects::makeChart(int sheet, const Chart &chart, const DrawingAnchor &anchor, QString *why)
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return -1;
    };
    if (!m_book || sheet < 0 || sheet >= int(m_sheets.size())) return refuse(tr("There is no such sheet."));
    if (m_book->info().sheets[sheet].kind != SheetKind::Worksheet) return refuse(tr("A chart sheet holds one chart. Put charts on a worksheet."));
    Sheet &s = sheetState(sheet);
    if (addedCount() >= m_limits.maxAdded)
        return refuse(tr("%1 new objects are as many as JeffCrunch saves in one go. Save the workbook, and then add more.").arg(QLocale().toString(m_limits.maxAdded)));
    if (m_objectsMade >= kMaxWorkbookObjects) return refuse(tr("The workbook has as many objects as JeffCrunch holds (%1).").arg(QLocale().toString(kMaxWorkbookObjects)));
    if (!chartRoom(sheet, chart, -1, why)) return -1;
    knowShapeIds(sheet, s);
    auto o = std::make_shared<Object>();
    o->id = s.nextId++;
    o->added = true;
    o->deleted = true;   // (until the step that makes it is done)
    o->meta.kind = ObjectKind::Chart;
    o->meta.name = newName(sheet, ObjectKind::Chart);
    o->meta.id = s.nextShapeId++;
    o->meta.anchor = anchor;
    o->meta.ordinal = -1;
    o->fileAnchor = o->base = o->anchor = anchor;
    o->frozen = anchor.kind != AnchorKind::TwoCell;
    o->placements.push_back(jp::crunch::AnchorPlacement{int(m_applied.size()), anchor});
    {
        // (a chart put in a part of its own has none of the links the part it came from had)
        Chart fresh = chart;
        fresh.clearPartLinks();
        o->chart = std::make_shared<const Chart>(fresh);
    }
    o->baseRefs = chartRefs(chart);
    for (QString &ref : o->baseRefs) ref = unfollowSheetOps(ref);
    o->refPlacements.push_back(jp::crunch::RefPlacement{int(m_applied.size()), o->baseRefs});
    o->movedRefs = o->baseRefs;
    refreshRefs(*o);
    o->points = pointsOf(sheet, chart);
    o->built = o->made = true;
    ++m_objectsMade;
    ++s.counts.charts;
    ++s.counts.shown;
    s.objects.push_back(o);
    if (sheet == m_shown) {
        m_layer.addObject(layerObject(sheet, o));
        m_layer.setDeleted(o->id, true);
    }
    return o->id;
}

void SheetObjects::discardMade(int sheet, int id)
{
    Sheet &s = sheetState(sheet);
    for (size_t k = 0; k < s.objects.size(); ++k)
        if (s.objects[k]->id == id && s.objects[k]->added) {
            s.objects.erase(s.objects.begin() + qsizetype(k));
            --m_objectsMade;
            --s.counts.charts;
            --s.counts.shown;
            if (sheet == m_shown) show(sheet);   // (the layer has one object less)
            return;
        }
}

int SheetObjects::duplicate(int sheet, int fromSheet, int fromId, const DrawingAnchor &anchor, QString *why)
{
    Object *from = find(fromSheet, fromId);
    if (!from) {
        if (why) *why = tr("The object is gone.");
        return -1;
    }
    ensureBuilt(fromSheet, *from);
    if (from->meta.kind != ObjectKind::Chart || !from->chart) {
        if (why) *why = tr("Only a chart can be copied here.");
        return -1;
    }
    Chart c = chartNow(fromSheet, fromId);
    // its references name the sheets as the person sees them: they are the same here
    return makeChart(sheet, c, anchor, why);
}

int SheetObjects::zTarget(int sheet, int id, int how)
{
    Sheet &s = sheetState(sheet);
    int at = -1;
    for (size_t k = 0; k < s.objects.size(); ++k)
        if (s.objects[k]->id == id) at = int(k);
    if (at < 0) return -1;
    const int last = int(s.objects.size()) - 1;
    auto live = [&](int k) { return !s.objects[size_t(k)]->deleted; };
    int to = at;
    if (how == 2) {
        to = last;
    } else if (how == -2) {
        to = 0;
    } else if (how > 0) {
        for (int k = at + 1; k <= last; ++k)
            if (live(k)) {
                to = k;
                break;
            }
    } else {
        for (int k = at - 1; k >= 0; --k)
            if (live(k)) {
                to = k;
                break;
            }
    }
    return to == at ? -1 : to;
}

bool SheetObjects::canReorder(int sheet, int id, QString *why)
{
    Sheet &s = sheetState(sheet);
    const Object *o = find(sheet, id);
    if (!o) return false;
    if (!o->added) {
        int same = 0;
        for (const std::shared_ptr<Object> &p : s.objects)
            if (!p->added && !p->gone && p->meta.ordinal == o->meta.ordinal) ++same;
        if (same > 1) {
            if (why) *why = tr("The file stores this object together with %n other(s), so it can't be put in another place in the order they are drawn.", nullptr, same - 1);
            return false;
        }
    }
    return true;
}


namespace {

// The relationship ids (the values of r:embed, r:link, r:pict, and r:id attributes) of the XML replaced by others, only where they stand as those attributes' values.
QByteArray remapIds(const QByteArray &xml, const QHash<QString, QString> &map)
{
    static const QRegularExpression attr(QStringLiteral("(\\b[A-Za-z_][\\w.-]*:(?:embed|link|pict|id)\\s*=\\s*)(\"([^\"]*)\"|'([^']*)')"));
    const QString text = QString::fromUtf8(xml);
    QString out;
    qsizetype last = 0;
    for (QRegularExpressionMatchIterator it = attr.globalMatch(text); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        const QString id = m.captured(3).isEmpty() ? m.captured(4) : m.captured(3);
        if (!map.contains(id)) continue;
        out += text.mid(last, m.capturedStart(2) - last) + QLatin1Char('"') + map.value(id) + QLatin1Char('"');
        last = m.capturedEnd(2);
    }
    return (out + text.mid(last)).toUtf8();
}

const char *const kPastePart = "/xl/drawings/p.xml";

} // namespace

bool SheetObjects::copyElement(int sheet, int id, jp::crunch::ObjectClip *clip, QString *why)
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    Object *o = find(sheet, id);
    if (!o || o->meta.kind == ObjectKind::Chart || o->meta.kind == ObjectKind::Other) return refuse(tr("This object can't be copied."));
    Sheet &s = sheetState(sheet);
    ensureBuilt(sheet, *o);
    if (!o->made || o->meta.element.isNull()) return refuse(tr("This object can't be shown here, so it can't be copied."));
    QString text;
    {
        QTextStream ts(&text);
        o->meta.element.save(ts, -1);
    }
    QByteArray xml = text.toUtf8();
    if (xml.size() > jp::crunch::kMaxClipElementBytes) return refuse(tr("This object is too large to copy."));
    // the namespaces its element uses (the anchor that holds it will declare the usual three)
    QByteArray namespaces;
    const QDomNamedNodeMap attrs = o->meta.element.ownerDocument().documentElement().attributes();
    for (int i = 0; i < attrs.size() && i < 64; ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (a.name().startsWith(QLatin1String("xmlns:")) && a.name() != QLatin1String("xmlns:xdr") && a.name() != QLatin1String("xmlns:a") && a.name() != QLatin1String("xmlns:r"))
            namespaces += " " + a.name().toUtf8() + "=\"" + xmlscan::escapeAttribute(a.value()) + "\"";
    }
    // the pictures it names
    const Package &package = o->ownPackage ? *o->ownPackage : m_book->package();
    const QString part = o->ownPackage ? QString::fromLatin1(kPastePart) : s.drawingPart;
    QHash<QString, QString> tokens;
    QList<jp::crunch::ClipMedia> media;
    qint64 total = 0;
    for (const QString &rid : namedRelationships(xml)) {
        const Relationship r = package.relationship(part, rid);
        if (r.id.isEmpty() || r.external || !r.type.endsWith(QLatin1String("/image")) || !package.hasPart(r.target))
            return refuse(tr("This object uses something that is not a picture in the workbook (a link, say), so it can't be copied."));
        const QByteArray bytes = package.part(r.target);
        total += bytes.size();
        if (bytes.isEmpty() || bytes.size() > jp::crunch::kMaxClipMediaBytes || total > jp::crunch::kMaxClipMediaTotal || media.size() >= jp::crunch::kMaxClipMedia) return refuse(tr("The pictures in this object are too large to copy."));
        jp::crunch::ClipMedia m;
        m.token = QStringLiteral("rIdPaste%1Z").arg(media.size());
        m.extension = r.target.mid(r.target.lastIndexOf(QLatin1Char('.')) + 1).toLower();
        bool plain = !m.extension.isEmpty() && m.extension.size() <= 8;
        for (const QChar ch : m.extension) plain = plain && ch.isLetterOrNumber();
        if (!plain) return refuse(tr("This object has a picture this program can't copy."));
        m.contentType = package.contentType(r.target);
        if (m.contentType.isEmpty()) m.contentType = QStringLiteral("image/") + m.extension;
        m.bytes = bytes;
        tokens.insert(rid, m.token);
        media.append(m);
    }
    jp::crunch::ObjectClip c;
    const QString kind = o->meta.element.localName();
    c.kind = kind == QLatin1String("pic") ? QStringLiteral("picture") : kind == QLatin1String("grpSp") ? QStringLiteral("group") : kind == QLatin1String("cxnSp") ? QStringLiteral("connector") : QStringLiteral("shape");
    const jp::grid::ObjectBox box = m_layer.boxNow(id);
    c.width = std::clamp(box.w, 4.0, 20000.0);
    c.height = std::clamp(box.h, 4.0, 20000.0);
    c.sheetName = m_book->info().sheets[sheet].name;
    c.elementXml = remapIds(xml, tokens);
    c.namespaces = namespaces;
    c.media = media;
    *clip = c;
    return true;
}

int SheetObjects::makeElement(int sheet, const jp::crunch::ObjectClip &clip, const DrawingAnchor &anchor, QString *why)
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return -1;
    };
    if (!m_book || sheet < 0 || sheet >= int(m_sheets.size()) || !clip.valid() || clip.kind == QLatin1String("chart")) return refuse(tr("There is nothing to paste here."));
    if (m_book->info().sheets[sheet].kind != SheetKind::Worksheet) return refuse(tr("A chart sheet holds one chart. Put objects on a worksheet."));
    Sheet &s = sheetState(sheet);
    if (addedCount() >= m_limits.maxAdded)
        return refuse(tr("%1 new objects are as many as JeffCrunch saves in one go. Save the workbook, and then add more.").arg(QLocale().toString(m_limits.maxAdded)));
    if (m_objectsMade >= kMaxWorkbookObjects) return refuse(tr("The workbook has as many objects as JeffCrunch holds (%1).").arg(QLocale().toString(kMaxWorkbookObjects)));
    qint64 held = 0;
    for (const Object *o : {static_cast<const Object *>(nullptr)}) Q_UNUSED(o);
    for (const jp::crunch::ClipMedia &m : clip.media) held += m.bytes.size();
    knowShapeIds(sheet, s);
    // the element in a document of its own
    QByteArray wrapped = "<w xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"" + clip.namespaces + ">" + clip.elementXml + "</w>";
    auto doc = std::make_shared<QDomDocument>();
    QString problem;
    if (!parseXml(wrapped, doc.get(), &problem)) return refuse(tr("The object on the clipboard can't be read."));
    QDomElement el = doc->documentElement().firstChildElement();
    if (el.isNull()) return refuse(tr("The object on the clipboard can't be read."));
    // its numbers in the drawing: each shape in it a new one
    qint64 nextId = s.nextShapeId;
    QString name;
    qint64 firstId = 0;
    int count = 0;
    std::function<bool(QDomElement &, int)> number = [&](QDomElement &e, int depth) {
        if (depth > 64 || count > 1000) return false;
        if (e.localName() == QLatin1String("cNvPr")) {
            ++count;
            if (name.isEmpty()) name = e.attribute(QStringLiteral("name"));
            if (!firstId) firstId = nextId;
            e.setAttribute(QStringLiteral("id"), QString::number(nextId++));
        }
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
            if (!number(c, depth + 1)) return false;
        return true;
    };
    if (!number(el, 0)) return refuse(tr("The object on the clipboard has more parts than JeffCrunch keeps."));
    s.nextShapeId = nextId;
    QString text;
    {
        QTextStream ts(&text);
        el.save(ts, -1);
    }
    // a package of its own, with the pictures it names, so that it is read as any object of a drawing is
    jp::ZipWriter zip;
    QByteArray types = "<?xml version=\"1.0\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"><Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/>";
    QSet<QString> seen;
    for (const jp::crunch::ClipMedia &m : clip.media)
        if (!seen.contains(m.extension.toLower())) {
            seen.insert(m.extension.toLower());
            types += "<Default Extension=\"" + xmlscan::escapeAttribute(m.extension) + "\" ContentType=\"" + xmlscan::escapeAttribute(m.contentType) + "\"/>";
        }
    types += "</Types>";
    zip.add(QStringLiteral("[Content_Types].xml"), types, true);
    zip.add(QStringLiteral("xl/drawings/p.xml"), QByteArray("<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\"/>"), true);
    QByteArray rels;
    for (int i = 0; i < clip.media.size(); ++i) {
        const jp::crunch::ClipMedia &m = clip.media[i];
        zip.add(QStringLiteral("xl/media/paste%1.%2").arg(i).arg(m.extension), m.bytes, true);
        rels = rels.isEmpty() ? QByteArray() : rels;
        rels += relationshipXml(m.token, QStringLiteral("image"), QStringLiteral("../media/paste%1.%2").arg(i).arg(m.extension));
    }
    zip.add(QStringLiteral("xl/drawings/_rels/p.xml.rels"), "<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" + rels + "</Relationships>", true);
    auto package = std::make_shared<Package>();
    if (!package->readLazy(zip.finish(), &problem)) return refuse(tr("The object on the clipboard can't be read."));
    auto context = std::make_shared<DrawingContext>(*baseContext());
    context->package = package.get();
    context->part = QString::fromLatin1(kPastePart);
    jp::ItemPtr item = readShape(el, *context);
    if (!item) return refuse(tr("The object on the clipboard can't be drawn."));
    auto o = std::make_shared<Object>();
    o->id = s.nextId++;
    o->added = true;
    o->deleted = true;
    const QString kind = el.localName();
    o->meta.kind = kind == QLatin1String("pic") ? ObjectKind::Picture : kind == QLatin1String("grpSp") ? ObjectKind::Group : kind == QLatin1String("cxnSp") ? ObjectKind::Connector : ObjectKind::Shape;
    o->meta.name = name.isEmpty() ? newName(sheet, o->meta.kind) : name;
    o->meta.id = firstId;
    o->meta.anchor = anchor;
    o->meta.element = el;
    o->meta.ordinal = -1;
    o->fileAnchor = o->base = o->anchor = anchor;
    o->frozen = anchor.kind != AnchorKind::TwoCell;
    o->placements.push_back(jp::crunch::AnchorPlacement{int(m_applied.size()), anchor});
    o->item = item;
    o->ownDoc = doc;
    o->ownPackage = package;
    o->ownContext = context;
    o->pasteXml = text.toUtf8();
    o->pasteNamespaces = clip.namespaces;
    o->pasteMedia = clip.media;
    o->built = o->made = true;
    ++m_objectsMade;
    ++s.counts.shown;
    switch (o->meta.kind) {
    case ObjectKind::Picture: ++s.counts.pictures; break;
    case ObjectKind::Group: ++s.counts.groups; break;
    case ObjectKind::Connector: ++s.counts.connectors; break;
    default: ++s.counts.shapes; break;
    }
    s.objects.push_back(o);
    if (sheet == m_shown) {
        m_layer.addObject(layerObject(sheet, o));
        m_layer.setDeleted(o->id, true);
    }
    return o->id;
}

bool SheetObjects::canEditChart(int sheet, int id, QString *why)
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    Sheet &s = sheetState(sheet);
    Object *o = find(sheet, id);
    if (!o || o->meta.kind != ObjectKind::Chart) return refuse(tr("This is not a chart."));
    ensureBuilt(sheet, *o);
    if (!o->chart || !o->made) return refuse(tr("This chart can't be shown here, so it can't be changed."));
    if (isChartSheet(sheet)) return refuse(tr("This chart is on a chart sheet."));
    if (o->added || o->chart != o->fileChart) return true;   // (made here, or changed here: the model is what there is)
    if (o->editable < 0) {
        int same = 0;
        for (const DrawingObject &d : std::as_const(s.drawing.objects))
            if (!d.chartPart.isEmpty() && d.chartPart.compare(o->meta.chartPart, Qt::CaseInsensitive) == 0) ++same;
        QString problem;
        o->editable = 1;
        if (same > 1) {
            o->editable = 0;
            o->editWhy = tr("The file stores this chart together with %n other(s), so it can't be changed here.", nullptr, same - 1);
        } else if (!chartIsFullyModeled(m_book->package().part(o->meta.chartPart), &problem)) {
            o->editable = 0;
            o->editWhy = tr("This chart has something JeffCrunch can't keep when it changes a chart (%1). It can be moved, sized, and deleted.").arg(problem);
        }
    }
    if (o->editable == 0) return refuse(o->editWhy);
    return true;
}

Chart SheetObjects::chartNow(int sheet, int objectId)
{
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects)
        if (o->id == objectId) {
            ensureBuilt(sheet, *o);
            if (!o->chart) return Chart();
            Chart c = *o->chart;
            setChartRefs(&c, o->refs);
            if (m_cells) {
                applyChartCells(&c, sheet, *m_cells);
                if (o->added || o->chart != o->fileChart) applyChartFormats(&c, sheet, *m_cells);
            }
            return c;
        }
    return Chart();
}

void SheetObjects::chartRefCounts(int sheet, int objectId, int *refs, int *found, QStringList *missed)
{
    *refs = *found = 0;
    if (!m_cells) return;
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects) {
        if (o->id != objectId) continue;
        ensureBuilt(sheet, *o);
        for (const QString &ref : std::as_const(o->refs)) {
            if (ref.isEmpty()) continue;
            ++*refs;
            QVector<ChartBlock> blocks;
            if (chartBlocks(ref, sheet, *m_cells, &blocks)) ++*found;
            else if (missed) *missed << ref;
        }
        return;
    }
}

QVector<ChartBlock> SheetObjects::chartReads(int sheet, int objectId)
{
    if (!m_cells) return {};
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects)
        if (o->id == objectId) {
            ensureBuilt(sheet, *o);
            if (!o->chart) return {};
            Chart c = *o->chart;
            setChartRefs(&c, o->refs);
            return jp::office::sheet::chartReads(c, sheet, *m_cells);
        }
    return {};
}

DrawingAnchor SheetObjects::anchorOf(int sheet, int objectId)
{
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects)
        if (o->id == objectId) return o->anchor;
    return DrawingAnchor();
}

} // namespace jp::crunch
