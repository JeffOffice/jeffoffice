#include "core/sheetobjects.h"

#include "grid/gridsheet.h"
#include "office/theme.h"
#include "office/xml.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QPainter>
#include <QPointer>
#include <QSvgRenderer>
#include <QThreadPool>

#include <algorithm>
#include <cmath>

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
        for (const SheetEntry &e : m_book->info().sheets) names << (e.isNew ? QString() : e.fileName);
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
    s.context = std::make_shared<DrawingContext>();
    DrawingContext &ctx = *s.context;
    ctx.package = &m_book->package();
    ctx.part = s.drawingPart;
    ctx.theme = m_theme;
    ctx.colors = ColorMap();
    ctx.formats = m_formats;
    ctx.images = &m_images;
    ctx.budget = &m_budget;
    ctx.addImage = [this](const QByteArray &bytes, const QString &format) { return m_doc->addImage(bytes, format); };
    ctx.addMedia = [this](const QByteArray &bytes, const QString &type, const QString &extension) { return m_doc->addMedia(bytes, type, extension); };
    ctx.newStory = [this] { return m_doc->createStory(); };
    ctx.storyDoc = [this](const QString &id) { return m_doc->storyDoc(id); };
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
        replay(sheet, *o);
        s.objects.push_back(std::move(o));
    }
}

void SheetObjects::replay(int sheet, Object &o)
{
    m_editSteps += qint64(m_applied.size());
    o.anchor = o.base;
    if (!m_applied.empty()) o.anchor = movedAnchor(o.base, sheet, m_applied);
    if (!o.refs.isEmpty() || !o.baseRefs.isEmpty()) {
        const QStringList names = sheetNames();
        o.movedRefs = o.baseRefs;
        for (QString &ref : o.movedRefs) ref = movedChartRef(ref, names, m_applied, sheet);
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
        o.baseRefs = chartRefs(*chart);
        o.movedRefs = o.baseRefs;
        const QStringList names = sheetNames();
        if (!m_applied.empty())
            for (QString &ref : o.movedRefs) ref = movedChartRef(ref, names, m_applied, sheet);
        refreshRefs(o);
        o.made = true;
    } else {
        o.item = readShape(o.meta.element, *s.context);
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
    QSet<QString> deletedParts, aliveParts;
    auto sheetGone = [&](int sheet) { return sheet >= 0 && sheet < m_book->info().sheets.size() && m_book->info().sheets[sheet].deleted; };
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        Sheet &s = m_sheets[i];
        if (!s.listed || s.drawingPart.isEmpty()) continue;
        std::shared_ptr<jp::grid::GridGeometry> geometry;
        for (const std::shared_ptr<Object> &o : s.objects) {
            if (o->deleted) {
                changes->removedObjects[s.drawingPart].push_back(o->meta.ordinal);
                if (!o->meta.chartPart.isEmpty()) deletedParts.insert(o->meta.chartPart.toLower());
                continue;
            }
            if (!o->meta.chartPart.isEmpty()) aliveParts.insert(o->meta.chartPart.toLower());
            // what the file will have for the object: the anchor as the cells' edits leave it, or, for one that keeps its size, as its size and place now give it
            const DrawingAnchor inFile = movedAnchor(o->fileAnchor, int(i), edits);
            o->savedAnchor = inFile;
            if (o->frozen && m_source) {
                if (!geometry) geometry = jp::grid::GridSheet::makeGeometry(m_source->sheet(int(i)), m_book->styles());
                const DrawingAnchor want = twoCellFor(*o, geometry->layout.rows, geometry->layout.cols);
                if (!nearly(want.from, inFile.from) || !nearly(want.to, inFile.to)) {
                    changes->anchors[s.drawingPart][o->meta.ordinal] = want;
                    o->savedAnchor = want;
                }
            }
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
    m_applied.clear();
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        Sheet &s = m_sheets[i];
        s.parsed.clear();
        s.failedParts.clear();
        // the objects that were taken away are not in the file: they are let go, and the places of the elements after them are one less each
        std::vector<int> removed;
        for (const std::shared_ptr<Object> &o : s.objects)
            if (o->deleted) removed.push_back(o->meta.ordinal);
        std::sort(removed.begin(), removed.end());
        std::vector<std::shared_ptr<Object>> kept;
        for (const std::shared_ptr<Object> &o : s.objects) {
            if (o->deleted) continue;
            o->meta.ordinal -= int(std::lower_bound(removed.begin(), removed.end(), o->meta.ordinal) - removed.begin());
            o->base = o->anchor;
            o->fileAnchor = o->savedAnchor;   // (what addToSave said the file has)
            o->baseRefs = o->refs;
            o->movedRefs = o->refs;
            kept.push_back(o);
        }
        const bool changed = kept.size() != s.objects.size();
        s.objects = std::move(kept);
        if (changed && int(i) == m_shown) show(int(i));
    }
}

Chart SheetObjects::chartNow(int sheet, int objectId)
{
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects)
        if (o->id == objectId) {
            ensureBuilt(sheet, *o);
            if (!o->chart) return Chart();
            Chart c = *o->chart;
            setChartRefs(&c, o->refs);
            if (m_cells) applyChartCells(&c, sheet, *m_cells);
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
