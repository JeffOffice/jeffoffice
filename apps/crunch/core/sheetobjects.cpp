#include "core/sheetobjects.h"

#include "office/theme.h"
#include "office/xml.h"

#include <QPainter>
#include <QSvgRenderer>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::grid::LayerObject;
using jp::grid::RenderRequest;

namespace {

// What the drawing code may keep for the whole workbook, as a presentation's import may (the sample workbooks' largest drawing holds 1,889 shapes, 313 texts, and 75 pictures).
constexpr qint64 kBudgetObjects = kMaxWorkbookObjects, kBudgetStories = 30000, kBudgetParagraphs = 300000, kBudgetRuns = 500000, kBudgetLetters = 20000000;
// A chart part, once inflated, is read only if it is no larger than this (the largest real one is 1.8 MB).
constexpr qint64 kMaxChartPartBytes = qint64(32) << 20;

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

} // namespace

SheetObjects::SheetObjects(QObject *parent) : QObject(parent), m_doc(std::make_unique<jp::Document>()) {}

SheetObjects::~SheetObjects()
{
    m_layer.setObjects({});   // (the pictures' draw functions refer to this)
}

void SheetObjects::open(const std::shared_ptr<Workbook> &book)
{
    m_layer.setObjects({});
    m_book = book;
    m_cells.reset();
    m_source = nullptr;
    m_values = nullptr;
    m_sheets.clear();
    if (m_book) m_sheets.resize(size_t(m_book->info().sheets.size()));
    m_doc = std::make_unique<jp::Document>();
    m_layoutCache.clear();
    m_images.clear();
    m_budget = DrawingBudget();
    m_contextRead = false;
    m_shown = -1;
    m_objectsMade = 0;
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
        for (const SheetEntry &e : m_book->info().sheets) names << e.name;
    return names;
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
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return none;
    Sheet &s = m_sheets[size_t(sheet)];
    if (!s.built) build(sheet, s);
    return s;
}

std::shared_ptr<SheetObjects::Object> SheetObjects::buildObject(Sheet &s, const DrawingObject &d, const DrawingContext &ctx)
{
    auto o = std::make_shared<Object>();
    o->id = d.index;
    o->meta = d;
    o->base = o->anchor = d.anchor;
    DrawingCounts &c = s.counts;
    auto leaveOut = [&](const QString &why) {
        ++c.leftOut;
        c.why << why;
    };
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
        return o;
    }
    if (m_objectsMade >= kMaxWorkbookObjects) {
        leaveOut(tr("more objects than the %1 a workbook may show").arg(kMaxWorkbookObjects));
        return o;
    }
    if (d.kind == ObjectKind::Chart) {
        if (m_book->package().partSize(d.chartPart) > kMaxChartPartBytes) {
            leaveOut(tr("a chart part that is too large (%1)").arg(d.name));
            return o;
        }
        auto chart = std::make_shared<Chart>();
        QString why;
        if (!readChart(m_book->package().part(d.chartPart), m_theme, ColorMap(), chart.get(), &why)) {
            leaveOut(tr("a chart that could not be read (%1)").arg(why));
            return o;
        }
        o->baseChart = chart;
        o->chart = std::make_shared<Chart>(*chart);
        o->made = true;
    } else {
        o->item = readShape(d.element, ctx);
        if (!o->item) {
            leaveOut(d.kind == ObjectKind::Group ? tr("a group with nothing in it that is drawn") : tr("a shape or picture that could not be read"));
            return o;
        }
        o->made = true;
    }
    ++m_objectsMade;
    ++c.shown;
    return o;
}

void SheetObjects::build(int sheet, Sheet &s)
{
    s.built = true;
    if (!m_book) return;
    readContext();
    const SheetEntry &entry = m_book->info().sheets[sheet];
    s.chartSheet = entry.kind == SheetKind::Chartsheet;
    s.drawingPart = drawingPartOf(m_book->package(), entry.part);
    if (s.drawingPart.isEmpty()) return;
    QString why;
    if (!readDrawing(m_book->package(), s.drawingPart, &s.drawing, &why)) {
        s.counts.why << tr("a drawing that could not be read (%1)").arg(why);
        return;
    }
    s.counts.anchors = s.drawing.anchorsInPart;
    s.counts.skipped = s.drawing.skipped;
    s.counts.truncated = s.drawing.truncated;
    if (s.drawing.truncated) s.counts.why << tr("more than %1 objects in a drawing; the rest are left out").arg(kMaxDrawingAnchors);
    DrawingContext ctx;
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
    for (const DrawingObject &d : std::as_const(s.drawing.objects)) s.objects.push_back(buildObject(s, d, ctx));
    applyEdits(sheet, s);
}

void SheetObjects::applyEdits(int sheet, Sheet &s)
{
    if (!m_edits) return;
    const std::vector<StructEdit> edits = m_edits();
    const QStringList names = sheetNames();
    for (const std::shared_ptr<Object> &o : s.objects) {
        o->anchor = movedAnchor(o->base, sheet, edits);
        if (o->baseChart) {
            o->chart = std::make_shared<Chart>(*o->baseChart);
            moveChartRefs(o->chart.get(), names, edits);
        }
    }
}

DrawingCounts SheetObjects::counts(int sheet) { return sheetState(sheet).counts; }

void SheetObjects::remember(Object &o, const Chart &drawn)
{
    o.reads.clear();
    o.hull.clear();
    if (!m_cells) return;
    o.reads = jp::office::sheet::chartReads(drawn, 0, *m_cells);
    for (const ChartBlock &b : std::as_const(o.reads)) {
        jp::office::sheet::CellRange &h = o.hull[b.sheet];
        if (!h.valid) {
            h = b.range;
            h.valid = true;
        } else {
            h.row1 = std::min(h.row1, b.range.row1);
            h.col1 = std::min(h.col1, b.range.col1);
            h.row2 = std::max(h.row2, b.range.row2);
            h.col2 = std::max(h.col2, b.range.col2);
        }
    }
}

QImage SheetObjects::renderChart(int sheet, Object &o, const RenderRequest &q)
{
    Chart c = *o.chart;
    if (m_cells) {
        applyChartCells(&c, sheet, *m_cells);
        remember(o, c);
        ++m_chartsDrawn;
    }
    const double side = kChartMaxSide;
    const QSizeF points(std::clamp(q.units.width() * 0.75, 1.0, side), std::clamp(q.units.height() * 0.75, 1.0, side));
    const QByteArray svg = chartSvg(c, points);
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

QImage SheetObjects::renderItem(Object &o, const RenderRequest &q)
{
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
    if (o->item) {
        l.thin = o->meta.kind == ObjectKind::Connector || hasLine(*o->item, 0);
        l.margin = strokeReach(*o->item, 0);
        l.render = [this, o](const RenderRequest &q) { return renderItem(*o, q); };
    } else {
        l.render = [this, sheet, o](const RenderRequest &q) { return renderChart(sheet, *o, q); };
    }
    return l;
}

void SheetObjects::show(int sheet)
{
    Sheet &s = sheetState(sheet);
    std::vector<LayerObject> objects;
    for (const std::shared_ptr<Object> &o : s.objects)
        if (o->made) objects.push_back(layerObject(sheet, o));
    m_shown = sheet;
    m_layer.setObjects(std::move(objects));
    for (const std::shared_ptr<Object> &o : s.objects)
        if (o->deleted) m_layer.setDeleted(o->id, true);
    m_layer.setFillsView(s.chartSheet);
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
            if (!o->chart || o->hull.isEmpty()) continue;
            bool hit = false;
            for (const jp::grid::ValueStore::Item &c : cells) {
                const auto hull = o->hull.constFind(c.sheet);
                if (hull == o->hull.constEnd() || !hull->contains(c.row, c.col)) continue;
                for (const ChartBlock &b : std::as_const(o->reads))
                    if (b.sheet == c.sheet && b.range.contains(c.row, c.col)) {
                        hit = true;
                        break;
                    }
                if (hit) break;
            }
            if (hit && int(i) == m_shown) m_layer.invalidate(o->id);
            else if (hit) o->hull.clear();   // (drawn from the cells again when its sheet is shown)
        }
    }
}

void SheetObjects::cellsMoved()
{
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        Sheet &s = m_sheets[i];
        if (!s.built) continue;
        applyEdits(int(i), s);
        if (int(i) != m_shown) continue;
        for (const std::shared_ptr<Object> &o : s.objects) {
            m_layer.setAnchor(o->id, o->anchor);
            if (o->chart) m_layer.invalidate(o->id);
        }
    }
}

void SheetObjects::sizesChanged(int sheet)
{
    Q_UNUSED(sheet);
    if (m_cells) m_cells->sizesChanged();
    // (a chart that leaves out what is hidden is drawn again; the others come out the same)
    for (const std::shared_ptr<Object> &o : sheetState(m_shown).objects)
        if (o->chart && o->chart->plotVisibleOnly) m_layer.invalidate(o->id);
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

} // namespace

void SheetObjects::addToSave(SaveChanges *changes)
{
    if (!m_book) return;
    const QStringList names = sheetNames();
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        Sheet &s = sheetState(int(i));
        if (s.drawingPart.isEmpty()) continue;
        for (const std::shared_ptr<Object> &o : s.objects) {
            if (o->deleted) {
                changes->removedObjects[s.drawingPart].push_back(o->meta.ordinal);
                continue;
            }
            if (!o->chart || !m_cells || o->meta.chartPart.isEmpty()) continue;
            // the references whose cells changed, or that moved: their caches are made again
            struct Ref {
                const QString *now, *was;
            };
            std::vector<Ref> refs;
            const Chart &now = *o->chart, &was = *o->baseChart;
            for (size_t p = 0; p < now.plots.size() && p < was.plots.size(); ++p)
                for (size_t k = 0; k < now.plots[p].series.size() && k < was.plots[p].series.size(); ++k) {
                    const ChartSeries &a = now.plots[p].series[k], &b = was.plots[p].series[k];
                    refs.push_back({&a.nameRef, &b.nameRef});
                    refs.push_back({&a.categoryRef, &b.categoryRef});
                    refs.push_back({&a.valueRef, &b.valueRef});
                    refs.push_back({&a.xRef, &b.xRef});
                }
            refs.push_back({&now.title.ref, &was.title.ref});
            for (size_t k = 0; k < now.axes.size() && k < was.axes.size(); ++k) refs.push_back({&now.axes[k].title.ref, &was.axes[k].title.ref});
            for (const Ref &r : refs) {
                if (r.now->isEmpty()) continue;
                bool dirty = *r.now != *r.was;
                QVector<ChartBlock> blocks;
                if (!chartBlocks(*r.now, int(i), *m_cells, &blocks)) continue;
                for (const ChartBlock &b : std::as_const(blocks)) {
                    const auto sheetChanges = changes->sheets.find(b.sheet);
                    if (dirty || sheetChanges == changes->sheets.end()) continue;
                    dirty = changedIn(sheetChanges->second.cells, b.range) || changedIn(sheetChanges->second.results, b.range);
                }
                if (!dirty) continue;
                ChartRefData data;
                if (readChartRef(*r.now, int(i), *m_cells, kMaxChartSeriesCells, &data)) changes->chartCaches[o->meta.chartPart][*r.now] = data;
            }
        }
    }
    Q_UNUSED(names);
}

void SheetObjects::saved()
{
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        Sheet &s = m_sheets[i];
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
            if (o->chart) o->baseChart = std::make_shared<Chart>(*o->chart);
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
        if (o->id == objectId && o->chart) {
            Chart c = *o->chart;
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
        if (o->id != objectId || !o->chart) continue;
        auto count = [&](const QString &ref) {
            if (ref.isEmpty()) return;
            ++*refs;
            QVector<ChartBlock> blocks;
            if (chartBlocks(ref, sheet, *m_cells, &blocks)) ++*found;
            else if (missed) *missed << ref;
        };
        for (const ChartPlot &plot : o->chart->plots)
            for (const ChartSeries &s : plot.series) {
                count(s.nameRef);
                count(s.categoryRef);
                count(s.valueRef);
                count(s.xRef);
            }
        count(o->chart->title.ref);
        return;
    }
}

QVector<ChartBlock> SheetObjects::chartReads(int sheet, int objectId)
{
    if (!m_cells) return {};
    for (const std::shared_ptr<Object> &o : sheetState(sheet).objects)
        if (o->id == objectId && o->chart) return jp::office::sheet::chartReads(*o->chart, sheet, *m_cells);
    return {};
}

} // namespace jp::crunch
