#include "core/sheetcontrols.h"

#include "core/controlpaint.h"
#include "sheet/cellref.h"
#include "sheet/xlsxsave.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QImage>
#include <QPainter>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

using namespace jp::office::sheet;

SheetControls::SheetControls(QObject *parent) : QObject(parent) {}

void SheetControls::open(const std::shared_ptr<Workbook> &book)
{
    m_book = book;
    m_sheets.clear();
    if (m_book) m_sheets.resize(size_t(std::max(0, int(m_book->info().sheets.size()))));
}

QString SheetControls::kindName(ControlKind kind)
{
    switch (kind) {
    case ControlKind::CheckBox: return tr("Check Box");
    case ControlKind::Radio: return tr("Option Button");
    case ControlKind::Button: return tr("Button");
    case ControlKind::Drop: return tr("Drop Down");
    case ControlKind::List: return tr("List Box");
    case ControlKind::Spin: return tr("Spinner");
    case ControlKind::Scroll: return tr("Scroll Bar");
    case ControlKind::GroupBox: return tr("Group Box");
    case ControlKind::Label: return tr("Label");
    }
    return {};
}

SheetControls::Sheet &SheetControls::sheetState(int sheet)
{
    if (sheet >= int(m_sheets.size())) m_sheets.resize(size_t(sheet) + 1);
    Sheet &s = m_sheets[size_t(sheet)];
    if (!s.listed) list(sheet, s);
    return s;
}

void SheetControls::list(int sheet, Sheet &s)
{
    s.listed = true;
    if (!m_book || sheet < 0 || sheet >= int(m_book->info().sheets.size())) return;
    const SheetEntry &se = m_book->info().sheets[sheet];
    if (se.kind != SheetKind::Worksheet || se.deleted || se.isNew || se.part.isEmpty()) return;
    const QVector<FormControl> found = readFormControls(m_book->objectPackage(), se.part);
    for (const FormControl &c : found) {
        auto item = std::make_shared<Item>();
        item->id = s.nextId++;
        item->now = c;
        if (item->now.name.isEmpty()) item->now.name = kindName(c.kind) + QLatin1Char(' ') + QString::number(c.shapeId % 1024 > 0 ? c.shapeId % 1024 : c.shapeId);
        item->was = item->now;
        s.items.push_back(item);
    }
}

const SheetControls::Item *SheetControls::find(int sheet, int id) const
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || !isControlId(id)) return nullptr;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items)
        if (i->id == id) return i.get();
    return nullptr;
}

SheetControls::Item *SheetControls::find(int sheet, int id)
{
    return const_cast<Item *>(static_cast<const SheetControls *>(this)->find(sheet, id));
}

FormControl SheetControls::controlOf(int sheet, int id, bool *found) const
{
    const Item *i = find(sheet, id);
    if (found) *found = i != nullptr;
    return i ? i->now : FormControl();
}

int SheetControls::countOf(int sheet) const
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return 0;
    int n = 0;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items)
        if (!i->deleted) ++n;
    return n;
}

bool SheetControls::isDeleted(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i && i->deleted;
}

// ---- cells

bool SheetControls::resolve(int sheet, const QString &ref, int *cellSheet, CellRange *range) const
{
    QString text = ref.trimmed();
    if (text.startsWith(QLatin1Char('='))) text.remove(0, 1);
    if (text.isEmpty() || !m_book) return false;
    *cellSheet = sheet;
    const qsizetype bang = text.lastIndexOf(QLatin1Char('!'));
    if (bang >= 0) {
        QString name = text.left(bang);
        if (name.size() >= 2 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
        int found = -1;
        for (int i = 0; i < int(m_book->info().sheets.size()); ++i)
            if (!m_book->info().sheets[i].deleted && m_book->info().sheets[i].name.compare(name, Qt::CaseInsensitive) == 0) found = i;
        if (found < 0) return false;
        *cellSheet = found;
        text = text.mid(bang + 1);
    }
    return parseRangeRef(text, range) && range->valid;
}

bool SheetControls::validReference(int sheet, const QString &ref, bool range) const
{
    CellRange r;
    int cs = sheet;
    return resolve(sheet, ref, &cs, &r) && (range || r.cellCount() == 1);
}

bool SheetControls::linkOf(int sheet, int id, int *cellSheet, quint32 *row, quint32 *col) const
{
    const Item *item = find(sheet, id);
    if (!item) return false;
    QString link = item->now.link;
    if (link.isEmpty() && item->now.kind == ControlKind::Radio) {
        for (const Item *m : groupOf(m_sheets[size_t(sheet)], *item))
            if (!m->now.link.isEmpty()) {
                link = m->now.link;
                break;
            }
    }
    CellRange r;
    int cs = sheet;
    if (!resolve(sheet, link, &cs, &r) || r.cellCount() != 1) return false;
    if (cellSheet) *cellSheet = cs;
    if (row) *row = r.row1;
    if (col) *col = r.col1;
    return true;
}

QString SheetControls::valueText(const jp::calc::Value &v)
{
    switch (v.kind()) {
    case jp::calc::Value::Kind::Number: return QString::number(v.number(), 'g', 15);
    case jp::calc::Value::Kind::Text: return v.text();
    case jp::calc::Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    default: return QString();
    }
}

QStringList SheetControls::itemsOf(int sheet, int id) const
{
    QStringList out;
    const Item *item = find(sheet, id);
    if (!item || !m_values) return out;
    CellRange r;
    int cs = sheet;
    if (!resolve(sheet, item->now.range, &cs, &r)) return out;
    for (quint32 row = r.row1; row <= r.row2 && out.size() < kMaxListItems; ++row)
        for (quint32 col = r.col1; col <= r.col2 && out.size() < kMaxListItems; ++col) out << valueText(m_values(cs, row, col));
    return out;
}

// ---- groups of option buttons

std::vector<const SheetControls::Item *> SheetControls::groupOf(const Sheet &s, const Item &item) const
{
    // A group begins at an option button that says it does or at a group box, and goes on to the next.
    std::vector<const Item *> current;
    bool has = false;
    for (const std::shared_ptr<Item> &i : s.items) {
        if (i->deleted) continue;
        if (i->now.kind == ControlKind::GroupBox || (i->now.kind == ControlKind::Radio && i->now.firstButton)) {
            if (has) return current;
            current.clear();
        }
        if (i->now.kind != ControlKind::Radio) continue;
        current.push_back(i.get());
        if (i.get() == &item) has = true;
    }
    if (has) return current;
    return {&item};
}

// ---- looks and clicks

jp::grid::LayerObject SheetControls::layerObject(int sheet, const std::shared_ptr<Item> &item)
{
    jp::grid::LayerObject o;
    o.id = item->id;
    o.anchor = item->now.anchor;
    o.kind = ObjectKind::Other;
    o.name = item->now.name;
    o.fixed = true;
    o.hitInk = false;
    const std::weak_ptr<Item> weak = item;
    auto items = [this, sheet, weak]() -> QStringList {
        const std::shared_ptr<Item> i = weak.lock();
        return i ? itemsOf(sheet, i->id) : QStringList();
    };
    o.render = [weak, items](const jp::grid::RenderRequest &q) -> QImage {
        const std::shared_ptr<Item> i = weak.lock();
        if (!i || q.units.width() < 1 || q.units.height() < 1) return QImage();
        QImage image(q.pixels, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter p(&image);
        p.translate(q.inner.topLeft());
        p.scale(q.inner.width() / q.units.width(), q.inner.height() / q.units.height());
        paintControl(&p, QRectF(QPointF(0, 0), q.units), i->now, items());
        return image;
    };
    o.operate = [this, sheet, item](const QPointF &fraction, const QSizeF &units) { return operate(sheet, item, fraction, units); };
    if (item->now.kind == ControlKind::Button && !item->now.macro.isEmpty()) o.tooltip = tr("This button runs a macro. Macros don't run in JeffCrunch, so it does nothing here.");
    return o;
}

std::vector<jp::grid::LayerObject> SheetControls::layerObjects(int sheet, bool forPrint)
{
    std::vector<jp::grid::LayerObject> out;
    if (!m_book || sheet < 0 || sheet >= int(m_book->info().sheets.size())) return out;
    Sheet &s = sheetState(sheet);
    refresh(sheet);
    for (const std::shared_ptr<Item> &i : s.items) {
        if (i->deleted || i->now.hidden || (forPrint && !i->now.prints)) continue;
        out.push_back(layerObject(sheet, i));
    }
    return out;
}

std::optional<jp::grid::LayerObject> SheetControls::layerObjectOf(int sheet, int id)
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return std::nullopt;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items)
        if (i->id == id) {
            if (i->deleted || i->now.hidden) return std::nullopt;
            return layerObject(sheet, i);
        }
    return std::nullopt;
}

bool SheetControls::takeValue(const Sheet &s, Item &item, const jp::calc::Value &v)
{
    FormControl &c = item.now;
    const FormControl before = c;
    switch (c.kind) {
    case ControlKind::CheckBox:
        if (v.isBool()) c.checked = v.boolean() ? 1 : 0;
        else if (v.isNumber()) c.checked = v.number() != 0 ? 1 : 0;
        else if (v.isError()) c.checked = 2;
        else if (v.isEmpty()) c.checked = 0;
        break;
    case ControlKind::Radio: {
        const std::vector<const Item *> group = groupOf(s, item);
        int place = 1;
        for (const Item *m : group) {
            if (m == &item) break;
            ++place;
        }
        if (v.isNumber()) c.checked = std::lround(v.number()) == place ? 1 : 0;
        else if (v.isEmpty()) c.checked = 0;
        break;
    }
    case ControlKind::Drop:
    case ControlKind::List:
        if (v.isNumber()) c.sel = int(std::clamp<long>(std::lround(v.number()), 0, 1000000));
        else if (v.isEmpty()) c.sel = 0;
        break;
    case ControlKind::Spin:
    case ControlKind::Scroll:
        if (v.isNumber()) c.val = int(std::clamp<long>(std::lround(v.number()), std::min(c.min, c.max), std::max(c.min, c.max)));
        break;
    default: break;
    }
    return !(c.checked == before.checked && c.sel == before.sel && c.val == before.val);
}

std::vector<int> SheetControls::refresh(int sheet)
{
    std::vector<int> changed;
    if (!m_values || !m_book || sheet < 0 || size_t(sheet) >= m_sheets.size() || !m_sheets[size_t(sheet)].listed) return changed;
    Sheet &s = m_sheets[size_t(sheet)];
    for (const std::shared_ptr<Item> &i : s.items) {
        int cs = 0;
        quint32 row = 0, col = 0;
        if (i->deleted || !linkOf(sheet, i->id, &cs, &row, &col)) continue;
        const jp::calc::Value v = m_values(cs, row, col);
        const QString key = v.debug();
        if (!i->seenKnown) {
            // (the file's own look stays until the cell changes: a program that opens the file shows what the file stored)
            i->seenKnown = true;
            i->seen = key;
            continue;
        }
        if (key == i->seen) continue;
        i->seen = key;
        if (takeValue(s, *i, v)) {
            changed.push_back(i->id);
            Q_EMIT this->changed(sheet, i->id);
        }
    }
    return changed;
}

void SheetControls::apply(int sheet, int id, const FormControl &state, bool deleted)
{
    Item *i = find(sheet, id);
    if (!i) return;
    i->now = state;
    i->deleted = deleted;
    i->seenKnown = false;
    Q_EMIT changed(sheet, id);
}

int SheetControls::make(int sheet, ControlKind kind, const DrawingAnchor &anchor, const QRectF &placePt, QString *why)
{
    if (!m_book || sheet < 0 || sheet >= int(m_book->info().sheets.size()) || m_book->info().sheets[sheet].kind != SheetKind::Worksheet) {
        if (why) *why = tr("Form controls can only be put on a sheet of cells.");
        return -1;
    }
    Sheet &s = sheetState(sheet);
    int made = 0;
    qint64 most = 1024;
    for (const std::shared_ptr<Item> &i : s.items) {
        if (i->now.isNew) ++made;
        most = std::max(most, i->now.shapeId);
    }
    if (made >= kMaxMadeControls) {
        if (why) *why = tr("A sheet can't have more than %1 controls made in one session.").arg(kMaxMadeControls);
        return -1;
    }
    if (m_book->info().sheets[sheet].isNew) {
        if (why) *why = tr("Save the workbook first: controls can't be put on a sheet that has not been saved yet.");
        return -1;
    }
    // (a shape's number: past every one the drawings of the sheet have)
    for (const jp::office::Relationship &r : m_book->objectPackage().relationshipsOfType(m_book->info().sheets[sheet].part, QStringLiteral("/vmlDrawing")))
        if (!r.external && m_book->objectPackage().partSize(r.target) < kMaxControlPartBytes) most = std::max(most, largestShapeId(m_book->objectPackage().part(r.target)));
    auto item = std::make_shared<Item>();
    item->id = s.nextId++;
    FormControl &c = item->now;
    c.kind = kind;
    c.anchor = anchor;
    c.anchor.kind = AnchorKind::TwoCell;
    c.anchor.editAs = AnchorKind::TwoCell;
    c.place = placePt;
    c.isNew = true;
    c.shapeId = most + 1;
    // (the number a person sees in the name is the control's own place among the sheet's)
    c.name = kindName(kind) + QLatin1Char(' ') + QString::number(int(s.items.size()) + 1);
    if (kind == ControlKind::CheckBox || kind == ControlKind::Radio || kind == ControlKind::Button || kind == ControlKind::GroupBox || kind == ControlKind::Label) c.text = c.name;
    if (kind == ControlKind::Radio && made == 0) c.firstButton = true;
    c.min = 0;
    c.max = kind == ControlKind::Scroll ? 100 : 30000;
    item->was = c;
    item->deleted = true;
    s.items.push_back(item);
    return item->id;
}

void SheetControls::addToSave(SaveChanges *changes)
{
    for (size_t sheet = 0; sheet < m_sheets.size(); ++sheet) {
        const Sheet &s = m_sheets[sheet];
        if (!s.listed) continue;
        std::vector<FormControl> was, now;
        for (const std::shared_ptr<Item> &i : s.items) {
            if (i->deleted) continue;
            if (i->now.isNew) {
                now.push_back(i->now);
                continue;
            }
            if (sameControlState(i->now, i->was)) continue;
            was.push_back(i->was);
            now.push_back(i->now);
        }
        if (now.empty()) continue;
        SheetChanges &sc = changes->sheets[int(sheet)];
        sc.controlsWas = std::move(was);
        sc.controlsNow = std::move(now);
    }
}

void SheetControls::saved()
{
    for (Sheet &s : m_sheets)
        for (const std::shared_ptr<Item> &i : s.items) {
            if (i->deleted) continue;
            i->now.isNew = false;
            i->was = i->now;
        }
}

bool SheetControls::formatted(int sheet, int id, const FormControl &after, ControlOperation *out) const
{
    const Item *i = find(sheet, id);
    if (!i) return false;
    *out = ControlOperation();
    out->sheet = sheet;
    out->changes.push_back({id, i->now, after});
    return true;
}

bool SheetControls::choose(int sheet, int id, int place, ControlOperation *out)
{
    Item *i = find(sheet, id);
    if (!i) return false;
    *out = ControlOperation();
    out->sheet = sheet;
    int cs = 0;
    quint32 row = 0, col = 0;
    if (linkOf(sheet, id, &cs, &row, &col)) {
        out->setsCell = true;
        out->cellSheet = cs;
        out->row = row;
        out->col = col;
        out->text = QString::number(place);
        return true;
    }
    FormControl after = i->now;
    after.sel = place;
    out->changes.push_back({id, i->now, after});
    return true;
}

bool SheetControls::operate(int sheet, const std::shared_ptr<Item> &item, const QPointF &fraction, const QSizeF &units)
{
    const FormControl &c = item->now;
    Sheet &s = sheetState(sheet);
    const QStringList lines = itemsOf(sheet, item->id);
    const ControlHit hit = controlHit(c, units, QPointF(fraction.x() * units.width(), fraction.y() * units.height()), int(lines.size()));
    if (hit.part == ControlPart::None) return false;
    ControlOperation op;
    op.sheet = sheet;
    int cs = 0;
    quint32 row = 0, col = 0;
    const bool linked = linkOf(sheet, item->id, &cs, &row, &col);
    if (linked) {
        op.setsCell = true;
        op.cellSheet = cs;
        op.row = row;
        op.col = col;
    }
    // What the cell holds now, to leave it alone when a click would write what it has.
    const jp::calc::Value now = linked && m_values ? m_values(cs, row, col) : jp::calc::Value();
    auto number = [&](int value) {
        if (linked) {
            if (now.isNumber() && std::lround(now.number()) == value && now.number() == double(value)) return;
            op.text = QString::number(value);
        }
    };
    switch (c.kind) {
    case ControlKind::GroupBox:
    case ControlKind::Label: return false;
    case ControlKind::Button: Q_EMIT macroSkipped(sheet, item->id); return true;
    case ControlKind::Drop: Q_EMIT listRequested(sheet, item->id, lines, c.sel); return true;
    case ControlKind::CheckBox: {
        const bool was = linked && now.isBool() ? now.boolean() : c.checked == 1;
        const int next = was ? 0 : 1;
        if (linked) op.text = next ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
        else {
            FormControl after = c;
            after.checked = next;
            op.changes.push_back({item->id, c, after});
        }
        break;
    }
    case ControlKind::Radio: {
        const std::vector<const Item *> group = groupOf(s, *item);
        int place = 1;
        for (const Item *m : group) {
            if (m == item.get()) break;
            ++place;
        }
        if (c.checked == 1 && !linked) return true;
        if (linked) number(place);
        else
            for (const Item *m : group) {
                const int want = m == item.get() ? 1 : 0;
                if (m->now.checked == want) continue;
                FormControl after = m->now;
                after.checked = want;
                op.changes.push_back({m->id, m->now, after});
            }
        break;
    }
    case ControlKind::List: {
        if (hit.part != ControlPart::Line) return true;
        if (linked) number(hit.line);
        else if (c.sel != hit.line) {
            FormControl after = c;
            after.sel = hit.line;
            op.changes.push_back({item->id, c, after});
        }
        break;
    }
    case ControlKind::Spin:
    case ControlKind::Scroll: {
        int step = 0;
        switch (hit.part) {
        case ControlPart::Up: step = c.kind == ControlKind::Spin ? c.inc : -c.inc; break;
        case ControlPart::Down: step = c.kind == ControlKind::Spin ? -c.inc : c.inc; break;
        case ControlPart::PageUp: step = -c.page; break;
        case ControlPart::PageDown: step = c.page; break;
        default: return true;
        }
        const int lo = std::min(c.min, c.max), hi = std::max(c.min, c.max);
        // (the cell's own number is where it goes from, when it has one inside the limits)
        const int from = linked && now.isNumber() ? int(std::clamp<long>(std::lround(now.number()), lo, hi)) : c.val;
        const int next = int(std::clamp<qint64>(qint64(from) + step, lo, hi));
        if (linked) number(next);
        else if (next != c.val) {
            FormControl after = c;
            after.val = next;
            op.changes.push_back({item->id, c, after});
        }
        break;
    }
    }
    if ((op.setsCell && !op.text.isEmpty()) || !op.changes.empty()) Q_EMIT operated(op);
    return true;
}

} // namespace jp::crunch
