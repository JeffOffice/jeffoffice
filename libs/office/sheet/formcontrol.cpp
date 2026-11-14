#include "sheet/formcontrol.h"

#include "office/package.h"
#include "sheet/notes.h"
#include "sheet/xmlscan.h"

#include <QCoreApplication>
#include <QHash>
#include <QMap>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <optional>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;

constexpr qint64 kPxEmu = 9525;

std::atomic<qint64> g_shapeReads{0};
std::atomic<qint64> g_entryProbes{0};

struct KindName {
    ControlKind kind;
    const char *vml, *prop;
};
const KindName kKinds[] = {{ControlKind::CheckBox, "Checkbox", "CheckBox"}, {ControlKind::Radio, "Radio", "Radio"}, {ControlKind::Button, "Button", "Button"}, {ControlKind::Drop, "Drop", "Drop"},
                           {ControlKind::List, "List", "List"},         {ControlKind::Spin, "Spin", "Spin"},     {ControlKind::Scroll, "Scroll", "Scroll"},  {ControlKind::GroupBox, "GroupBox", "GBox"},
                           {ControlKind::Label, "Label", "Label"}};

bool kindOf(QByteArrayView name, bool prop, ControlKind *out)
{
    for (const KindName &k : kKinds)
        if (name.compare(prop ? k.prop : k.vml, Qt::CaseInsensitive) == 0) {
            *out = k.kind;
            return true;
        }
    return false;
}

qint64 clampInt(qint64 v, qint64 lo, qint64 hi) { return std::max(lo, std::min(hi, v)); }

int intOf(const QString &s, int def)
{
    bool ok = false;
    const double d = s.trimmed().toDouble(&ok);
    if (!ok || !std::isfinite(d)) return def;
    return int(clampInt(qint64(d), -1000000, 1000000));
}

// A length of a shape's style in points.
double pointsOf(QString v, bool *ok)
{
    v = v.trimmed();
    double f = 1;
    if (v.endsWith(QLatin1String("pt"))) v.chop(2);
    else if (v.endsWith(QLatin1String("px"))) {
        v.chop(2);
        f = 0.75;
    } else if (v.endsWith(QLatin1String("in"))) {
        v.chop(2);
        f = 72;
    } else if (v.endsWith(QLatin1String("cm"))) {
        v.chop(2);
        f = 72 / 2.54;
    } else if (v.endsWith(QLatin1String("mm"))) {
        v.chop(2);
        f = 72 / 25.4;
    }
    bool good = false;
    const double d = v.toDouble(&good);
    if (ok) *ok = good && std::isfinite(d);
    return good && std::isfinite(d) ? std::clamp(d * f, -1e7, 1e7) : 0;
}

QString textOf(QByteArrayView body, qsizetype from)
{
    qsizetype e = body.indexOf('<', from);
    if (e < 0) e = body.size();
    return xmlscan::unescape(body.sliced(from, e - from)).trimmed();
}

// The words of a shape's text box: its markup taken out, entities read, runs of white space one space, and a line break where the markup has one.
QString boxText(QByteArrayView box, QString *face, double *pt, quint32 *color)
{
    static const QRegularExpression br(QStringLiteral("<br\\s*/?>"), QRegularExpression::CaseInsensitiveOption), tags(QStringLiteral("<[^>]*>")), space(QStringLiteral("[ \\t\\r\\n]+"));
    static const QRegularExpression fontTag(QStringLiteral("<font\\b([^>]*)>"), QRegularExpression::CaseInsensitiveOption);
    QString s = QString::fromUtf8(box.data(), box.size());
    const QRegularExpressionMatch fm = fontTag.match(s);
    if (fm.hasMatch()) {
        const QByteArray attrs = "<f " + fm.captured(1).toUtf8() + ">";
        bool has = false;
        const QByteArrayView f = xmlscan::attribute(attrs, "face", &has);
        if (has) *face = xmlscan::unescape(f);
        const QByteArrayView sz = xmlscan::attribute(attrs, "size", &has);
        if (has) *pt = std::clamp(xmlscan::unescape(sz).toDouble() / 20.0, 0.0, 400.0);
        const QByteArrayView c = xmlscan::attribute(attrs, "color", &has);
        if (has) {
            const QString cs = xmlscan::unescape(c).trimmed();
            if (cs.size() == 7 && cs.startsWith(QLatin1Char('#'))) {
                bool ok = false;
                const quint32 v = cs.mid(1).toUInt(&ok, 16);
                if (ok) *color = 0xFF000000u | v;
            }
        }
    }
    s.replace(br, QStringLiteral("\x01"));
    s.replace(tags, QString());
    s.replace(QLatin1String("&nbsp;"), QStringLiteral(" "));
    s = xmlscan::unescape(s.toUtf8());
    s.replace(space, QStringLiteral(" "));
    s.replace(QLatin1Char('\x01'), QLatin1Char('\n'));
    return s.trimmed();
}

// The value of the style's property, as written.
QString styleValue(const QString &style, const QString &key)
{
    for (const QString &part : style.split(QLatin1Char(';'))) {
        const qsizetype colon = part.indexOf(QLatin1Char(':'));
        if (colon > 0 && part.left(colon).trimmed().compare(key, Qt::CaseInsensitive) == 0) return part.mid(colon + 1).trimmed();
    }
    return {};
}

struct PropAttrs {
    bool any = false;
    std::optional<ControlKind> kind;
    QString link, range;
    int checked = 0, val = 0, min = 0, max = 30000, inc = 1, page = 10, sel = 0, dropLines = 8;
    bool hasChecked = false, hasVal = false, hasMin = false, hasMax = false, hasInc = false, hasPage = false, hasSel = false, hasDrop = false, hasLink = false, hasRange = false;
    QString selType;
    bool horiz = false, noThreeD = false, firstButton = false;
};

PropAttrs readProp(const QByteArray &xml)
{
    PropAttrs p;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "formControlPr") continue;
        const QByteArrayView tag = QByteArrayView(xml).sliced(t.start, t.end - t.start);
        p.any = true;
        auto get = [&](const char *name, QString *out) {
            bool has = false;
            const QByteArrayView v = xmlscan::attribute(tag, name, &has);
            if (has) *out = xmlscan::unescape(v);
            return has;
        };
        QString v;
        if (get("objectType", &v)) {
            ControlKind k;
            if (kindOf(v.toUtf8(), true, &k)) p.kind = k;
        }
        if (get("checked", &v)) {
            p.hasChecked = true;
            p.checked = v.compare(QLatin1String("Checked"), Qt::CaseInsensitive) == 0 ? 1 : v.compare(QLatin1String("Mixed"), Qt::CaseInsensitive) == 0 ? 2 : 0;
        }
        p.hasLink = get("fmlaLink", &p.link);
        p.hasRange = get("fmlaRange", &p.range);
        if (get("val", &v)) { p.hasVal = true; p.val = intOf(v, 0); }
        if (get("min", &v)) { p.hasMin = true; p.min = intOf(v, 0); }
        if (get("max", &v)) { p.hasMax = true; p.max = intOf(v, 30000); }
        if (get("inc", &v)) { p.hasInc = true; p.inc = intOf(v, 1); }
        if (get("page", &v)) { p.hasPage = true; p.page = intOf(v, 10); }
        if (get("sel", &v)) { p.hasSel = true; p.sel = intOf(v, 0); }
        if (get("dropLines", &v)) { p.hasDrop = true; p.dropLines = intOf(v, 8); }
        get("selType", &p.selType);
        if (get("horiz", &v)) p.horiz = v == QLatin1String("1") || v == QLatin1String("true");
        if (get("noThreeD", &v)) p.noThreeD = v == QLatin1String("1") || v == QLatin1String("true");
        if (get("firstButton", &v)) p.firstButton = v == QLatin1String("1") || v == QLatin1String("true");
        break;
    }
    return p;
}

// What the sheet's `controls` says of a control: its name, the relationship to its properties part, the macro, whether it prints, and its anchor.
struct SheetControl {
    qint64 shapeId = 0;
    QString name, relId, macro;
    bool hasPr = false, prints = true, locked = true;
    bool hasAnchor = false;
    DrawingAnchor anchor;
};

QVector<SheetControl> readSheetControls(const QByteArray &sheet)
{
    QVector<SheetControl> out;
    const qsizetype start = sheet.indexOf("controls>");
    if (start < 0) return out;
    qsizetype open = sheet.lastIndexOf('<', start);
    if (open < 0) return out;
    Tag t;
    qsizetype pos = open;
    SheetControl cur;
    bool inControl = false, inFrom = false, inTo = false;
    QByteArray textFor;
    QHash<qint64, int> placeOf;   // (a shape's number to its place in `out`: a file may name a control a hundred thousand times)
    auto finish = [&] {
        if (!inControl) return;
        inControl = false;
        g_entryProbes.fetch_add(1, std::memory_order_relaxed);   // (one probe of the hash for each entry: a search through those kept would add one for each of them)
        const auto at = placeOf.constFind(cur.shapeId);
        if (at != placeOf.constEnd()) {
            SheetControl &o = out[*at];
            if (!o.hasPr && cur.hasPr) o = cur;
            return;
        }
        if (out.size() < kMaxFormControls) {
            placeOf.insert(cur.shapeId, int(out.size()));
            out.push_back(cur);
        }
    };
    int guard = 0;
    while (xmlscan::nextTag(sheet, pos, &t) && ++guard < 200000) {
        pos = t.end;
        if (t.local == "controls" && t.closing) break;
        if (t.local == "control") {
            if (t.closing) {
                finish();
                continue;
            }
            finish();
            cur = SheetControl();
            inControl = true;
            const QByteArrayView tag = QByteArrayView(sheet).sliced(t.start, t.end - t.start);
            bool has = false;
            const QByteArrayView id = xmlscan::attribute(tag, "shapeId", &has);
            bool numeric = false;
            const qint64 n = has ? QByteArray(id).toLongLong(&numeric) : 0;
            cur.shapeId = numeric && n > 0 && n <= kMaxShapeId ? n : 0;
            const QByteArrayView rid = xmlscan::attribute(tag, "r:id", &has);
            if (has) cur.relId = QString::fromUtf8(rid);
            const QByteArrayView name = xmlscan::attribute(tag, "name", &has);
            if (has) cur.name = xmlscan::unescape(name);
            if (t.selfClosing) finish();
        } else if (!inControl) {
            continue;
        } else if (t.local == "controlPr" && !t.closing) {
            cur.hasPr = true;
            const QByteArrayView tag = QByteArrayView(sheet).sliced(t.start, t.end - t.start);
            bool has = false;
            const QByteArrayView macro = xmlscan::attribute(tag, "macro", &has);
            if (has) cur.macro = xmlscan::unescape(macro);
            const QByteArrayView print = xmlscan::attribute(tag, "print", &has);
            if (has) cur.prints = !(print == "0" || print == "false");
            const QByteArrayView locked = xmlscan::attribute(tag, "locked", &has);
            if (has) cur.locked = !(locked == "0" || locked == "false");
        } else if (t.local == "anchor" && !t.closing) {
            cur.hasAnchor = true;
            const QByteArrayView tag = QByteArrayView(sheet).sliced(t.start, t.end - t.start);
            bool has = false;
            const QByteArrayView move = xmlscan::attribute(tag, "moveWithCells", &has);
            const QByteArrayView size = xmlscan::attribute(tag, "sizeWithCells", &has);
            const bool m = move == "1" || move == "true", s = size == "1" || size == "true";
            cur.anchor.kind = AnchorKind::TwoCell;
            cur.anchor.editAs = s ? AnchorKind::TwoCell : m ? AnchorKind::OneCell : AnchorKind::Absolute;
        } else if (t.local == "from" || t.local == "to") {
            inFrom = !t.closing && t.local == "from";
            inTo = !t.closing && t.local == "to";
        } else if ((inFrom || inTo) && !t.closing && !t.selfClosing && (t.local == "col" || t.local == "colOff" || t.local == "row" || t.local == "rowOff")) {
            const QString v = textOf(QByteArrayView(sheet), t.end);
            AnchorPoint &p = inFrom ? cur.anchor.from : cur.anchor.to;
            const qint64 n = clampInt(v.toLongLong(), 0, kMaxDrawingEmu);
            if (t.local == "col") p.col = quint32(std::min<qint64>(n, 16383));
            else if (t.local == "row") p.row = quint32(std::min<qint64>(n, 1048575));
            else if (t.local == "colOff") p.colOff = n;
            else p.rowOff = n;
        }
    }
    finish();
    return out;
}

void assign(FormControl *c, const PropAttrs &p)
{
    if (p.kind) c->kind = *p.kind;
    if (p.hasLink) c->link = p.link;
    if (p.hasRange) c->range = p.range;
    if (p.hasChecked) c->checked = p.checked;
    if (p.hasVal) c->val = p.val;
    if (p.hasMin) c->min = p.min;
    if (p.hasMax) c->max = p.max;
    if (p.hasInc) c->inc = p.inc;
    if (p.hasPage) c->page = p.page;
    if (p.hasSel) c->sel = p.sel;
    if (p.hasDrop) c->dropLines = p.dropLines;
    if (!p.selType.isEmpty()) c->selType = p.selType;
    if (p.horiz) c->horizontal = true;
    if (p.noThreeD) c->threeD = false;
    if (p.firstButton) c->firstButton = true;
}

qint64 shapeIdOf(QByteArrayView tag)
{
    bool has = false;
    const QByteArrayView id = xmlscan::attribute(tag, "id", &has);
    return has ? shapeNumberOf(id) : 0;
}

// Whether a drawing may have a control: some ClientData of it has an ObjectType other than Note (a look at the bytes only; what it cannot make out counts as one that may).
bool mayHaveControls(const QByteArray &vml)
{
    auto space = [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; };
    qsizetype from = 0;
    while ((from = vml.indexOf("ObjectType", from)) >= 0) {
        qsizetype i = from + 10;
        from = i;
        while (i < vml.size() && space(vml[i])) ++i;
        if (i >= vml.size() || vml[i] != '=') return true;
        ++i;
        while (i < vml.size() && space(vml[i])) ++i;
        if (i >= vml.size() || (vml[i] != '"' && vml[i] != '\'')) return true;
        const char quote = vml[i++];
        const qsizetype end = vml.indexOf(quote, i);
        if (end < 0) return true;
        if (QByteArrayView(vml).sliced(i, end - i).compare("Note", Qt::CaseInsensitive) != 0) return true;
        from = end;
    }
    return false;
}

// What one shape of a legacy drawing says of a control, if it is one.
bool readShape(const QByteArray &vml, const DrawingShape &shape, FormControl *c)
{
    const QByteArrayView body = QByteArrayView(vml).sliced(shape.start, shape.end - shape.start);
    Tag t;
    qsizetype pos = 0;
    bool first = true, isControl = false, haveAnchor = false, sizeWith = false, moveWith = false, inClient = false;
    qsizetype boxStart = -1, boxEnd = -1;
    QString style;
    int tags = 0;
    while (xmlscan::nextTag(body, pos, &t) && ++tags < 4096) {
        pos = t.end;
        const QByteArrayView tag = body.sliced(t.start, t.end - t.start);
        if (first) {
            first = false;
            c->shapeId = shapeIdOf(tag);
            bool has = false;
            const QByteArrayView s = xmlscan::attribute(tag, "style", &has);
            if (has) style = xmlscan::unescape(s);
            const QByteArrayView alt = xmlscan::attribute(tag, "alt", &has);
            if (has && c->name.isEmpty()) c->name = xmlscan::unescape(alt);
            continue;
        }
        if (t.local == "textbox") {
            if (!t.closing) boxStart = t.end;
            else boxEnd = t.start;
            continue;
        }
        if (t.local == "ClientData") {
            if (t.closing) {
                inClient = false;
                continue;
            }
            bool has = false;
            const QByteArrayView type = xmlscan::attribute(tag, "ObjectType", &has);
            ControlKind k;
            if (!has || !kindOf(type, false, &k)) return false;
            c->kind = k;
            isControl = true;
            inClient = !t.selfClosing;
            continue;
        }
        if (!inClient || t.closing) continue;
        const QByteArray name = QByteArray(t.local);
        if (name == "SizeWithCells") sizeWith = true;
        else if (name == "MoveWithCells") moveWith = true;
        else if (name == "Horiz") c->horizontal = true;
        else if (name == "NoThreeD") c->threeD = false;
        else if (name == "FirstButton") c->firstButton = true;
        else if (name == "Visible") {
            // (a control is hidden by its style's visibility; this element is a note's)
        } else if (!t.selfClosing) {
            const QString v = textOf(body, t.end);
            if (name == "Anchor") {
                const QStringList parts = v.split(QLatin1Char(','));
                if (parts.size() >= 8) {
                    qint64 n[8];
                    for (int i = 0; i < 8; ++i) n[i] = clampInt(parts[i].trimmed().toLongLong(), 0, 1000000);
                    c->anchor.kind = AnchorKind::TwoCell;
                    c->anchor.from.col = quint32(std::min<qint64>(n[0], 16383));
                    c->anchor.from.colOff = n[1] * kPxEmu;
                    c->anchor.from.row = quint32(std::min<qint64>(n[2], 1048575));
                    c->anchor.from.rowOff = n[3] * kPxEmu;
                    c->anchor.to.col = quint32(std::min<qint64>(n[4], 16383));
                    c->anchor.to.colOff = n[5] * kPxEmu;
                    c->anchor.to.row = quint32(std::min<qint64>(n[6], 1048575));
                    c->anchor.to.rowOff = n[7] * kPxEmu;
                    haveAnchor = true;
                }
            } else if (name == "FmlaLink") c->link = v;
            else if (name == "FmlaRange") c->range = v;
            else if (name == "Checked") c->checked = std::clamp(intOf(v, 1), 0, 2);
            else if (name == "Val") c->val = intOf(v, 0);
            else if (name == "Min") c->min = intOf(v, 0);
            else if (name == "Max") c->max = intOf(v, 30000);
            else if (name == "Inc") c->inc = intOf(v, 1);
            else if (name == "Page") c->page = intOf(v, 10);
            else if (name == "Sel") c->sel = intOf(v, 0);
            else if (name == "DropLines") c->dropLines = intOf(v, 8);
            else if (name == "SelType") c->selType = v;
            else if (name == "FmlaMacro") c->macro = v;
            else if (name == "PrintObject") c->prints = !(v.compare(QLatin1String("False"), Qt::CaseInsensitive) == 0 || v == QLatin1String("0"));
            else if (name == "Locked") c->locked = !(v.compare(QLatin1String("False"), Qt::CaseInsensitive) == 0 || v == QLatin1String("0"));
        }
    }
    if (!isControl) return false;
    if (boxStart >= 0 && boxEnd > boxStart) c->text = boxText(body.sliced(boxStart, boxEnd - boxStart), &c->fontFace, &c->fontPt, &c->color);
    const QString vis = styleValue(style, QStringLiteral("visibility"));
    c->hidden = vis.compare(QLatin1String("hidden"), Qt::CaseInsensitive) == 0;
    if (haveAnchor) {
        c->anchor.editAs = sizeWith ? AnchorKind::TwoCell : moveWith ? AnchorKind::OneCell : AnchorKind::Absolute;
    } else {
        // no cells named: the place in points the style gives
        bool a = false, b = false, d = false, e = false;
        const double left = pointsOf(styleValue(style, QStringLiteral("margin-left")), &a), top = pointsOf(styleValue(style, QStringLiteral("margin-top")), &b);
        const double w = pointsOf(styleValue(style, QStringLiteral("width")), &d), h = pointsOf(styleValue(style, QStringLiteral("height")), &e);
        c->anchor.kind = AnchorKind::Absolute;
        c->anchor.editAs = AnchorKind::Absolute;
        c->anchor.x = qint64(std::llround(std::clamp(left, 0.0, 1e7) * 12700.0));
        c->anchor.y = qint64(std::llround(std::clamp(top, 0.0, 1e7) * 12700.0));
        c->anchor.cx = qint64(std::llround(std::clamp(d ? w : 60.0, 0.0, 1e7) * 12700.0));
        c->anchor.cy = qint64(std::llround(std::clamp(e ? h : 20.0, 0.0, 1e7) * 12700.0));
        (void)a;
        (void)b;
    }
    return true;
}

// ---- writing

QByteArray num(qint64 n) { return QByteArray::number(n); }

// The element `name` among the children of the ClientData of `shape` ([start, end) of the element), or -1.
bool findChild(const QByteArray &shape, const QByteArray &name, qsizetype *start, qsizetype *end, QByteArray *prefix, qsizetype *closeAt)
{
    Tag t;
    qsizetype pos = 0;
    bool inClient = false;
    *start = *end = -1;
    *closeAt = -1;
    while (xmlscan::nextTag(shape, pos, &t)) {
        pos = t.end;
        if (t.local == "ClientData") {
            if (t.closing) {
                *closeAt = t.start;
                *prefix = QByteArray(xmlscan::prefixOf(t.name));
                return *start >= 0;
            }
            inClient = !t.selfClosing;
            *prefix = QByteArray(xmlscan::prefixOf(t.name));
            continue;
        }
        if (!inClient || t.closing || QByteArray(t.local) != name) continue;
        *start = t.start;
        *end = t.end;
        if (!t.selfClosing) {
            Tag c;
            if (xmlscan::nextTag(shape, t.end, &c) && c.closing && QByteArray(c.local) == name) *end = c.end;
        }
    }
    return *start >= 0;
}

// Sets (or, with no text, removes) a child of the ClientData.
bool setChild(QByteArray *shape, const QByteArray &name, const std::optional<QByteArray> &text)
{
    qsizetype s, e, close;
    QByteArray prefix;
    const bool had = findChild(*shape, name, &s, &e, &prefix, &close);
    if (close < 0) return false;
    if (had) {
        if (!text) shape->remove(s, e - s);
        else shape->replace(s, e - s, "<" + prefix + name + ">" + *text + "</" + prefix + name + ">");
    } else if (text) {
        shape->insert(close, "<" + prefix + name + ">" + *text + "</" + prefix + name + ">");
    }
    return true;
}

QByteArray escaped(const QString &s) { return xmlscan::escapeText(s); }

struct Field {
    const char *element;
    bool (*differs)(const FormControl &, const FormControl &);
    std::optional<QByteArray> (*value)(const FormControl &);
};

const Field kFields[] = {
    {"FmlaLink", [](const FormControl &a, const FormControl &b) { return a.link != b.link; }, [](const FormControl &c) -> std::optional<QByteArray> { return c.link.isEmpty() ? std::nullopt : std::optional<QByteArray>(escaped(c.link)); }},
    {"FmlaRange", [](const FormControl &a, const FormControl &b) { return a.range != b.range; }, [](const FormControl &c) -> std::optional<QByteArray> { return c.range.isEmpty() ? std::nullopt : std::optional<QByteArray>(escaped(c.range)); }},
    {"Checked", [](const FormControl &a, const FormControl &b) { return a.checked != b.checked; }, [](const FormControl &c) -> std::optional<QByteArray> { return c.checked ? std::optional<QByteArray>(num(c.checked)) : std::nullopt; }},
    {"Val", [](const FormControl &a, const FormControl &b) { return a.val != b.val; }, [](const FormControl &c) -> std::optional<QByteArray> { return num(c.val); }},
    {"Min", [](const FormControl &a, const FormControl &b) { return a.min != b.min; }, [](const FormControl &c) -> std::optional<QByteArray> { return num(c.min); }},
    {"Max", [](const FormControl &a, const FormControl &b) { return a.max != b.max; }, [](const FormControl &c) -> std::optional<QByteArray> { return num(c.max); }},
    {"Inc", [](const FormControl &a, const FormControl &b) { return a.inc != b.inc; }, [](const FormControl &c) -> std::optional<QByteArray> { return num(c.inc); }},
    {"Page", [](const FormControl &a, const FormControl &b) { return a.page != b.page; }, [](const FormControl &c) -> std::optional<QByteArray> { return num(c.page); }},
    {"Sel", [](const FormControl &a, const FormControl &b) { return a.sel != b.sel; }, [](const FormControl &c) -> std::optional<QByteArray> { return c.sel > 0 ? std::optional<QByteArray>(num(c.sel)) : std::nullopt; }},
    {"DropLines", [](const FormControl &a, const FormControl &b) { return a.dropLines != b.dropLines; }, [](const FormControl &c) -> std::optional<QByteArray> { return num(c.dropLines); }},
};

bool sameCells(const DrawingAnchor &a, const DrawingAnchor &b)
{
    return a.from.col == b.from.col && a.from.row == b.from.row && a.to.col == b.to.col && a.to.row == b.to.row;
}

QByteArray anchorText(const DrawingAnchor &a)
{
    auto px = [](qint64 emu) { return emu / kPxEmu; };
    return num(a.from.col) + ", " + num(px(a.from.colOff)) + ", " + num(a.from.row) + ", " + num(px(a.from.rowOff)) + ", " + num(a.to.col) + ", " + num(px(a.to.colOff)) + ", " + num(a.to.row) + ", "
           + num(px(a.to.rowOff));
}

constexpr const char *kShapeType = "<v:shapetype id=\"_x0000_t201\" coordsize=\"21600,21600\" o:spt=\"201\" path=\"m,l,21600r21600,l21600,xe\"><v:stroke joinstyle=\"miter\"/>"
                                   "<v:path shadowok=\"f\" o:extrusionok=\"f\" strokeok=\"f\" fillok=\"f\" o:connecttype=\"rect\"/><o:lock v:ext=\"edit\" shapetype=\"t\"/></v:shapetype>";

constexpr const char *kHead = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:x=\"urn:schemas-microsoft-com:office:excel\"><o:shapelayout v:ext=\"edit\"><o:idmap v:ext=\"edit\" data=\"1\"/></o:shapelayout>";

} // namespace

const char *controlVmlType(ControlKind kind)
{
    for (const KindName &k : kKinds)
        if (k.kind == kind) return k.vml;
    return "Button";
}

const char *controlPropType(ControlKind kind)
{
    for (const KindName &k : kKinds)
        if (k.kind == kind) return k.prop;
    return "Button";
}

bool sameControlState(const FormControl &a, const FormControl &b)
{
    return a.kind == b.kind && a.link == b.link && a.range == b.range && a.checked == b.checked && a.val == b.val && a.min == b.min && a.max == b.max && a.inc == b.inc && a.page == b.page && a.sel == b.sel
           && a.dropLines == b.dropLines && a.text == b.text && a.hidden == b.hidden && sameCells(a.anchor, b.anchor) && a.removed == b.removed;
}

QVector<FormControl> readFormControls(const Package &package, const QString &sheetPart, bool *truncated)
{
    QVector<FormControl> out;
    if (truncated) *truncated = false;
    if (sheetPart.isEmpty()) return out;
    // the properties parts, by the id the sheet's relationship has
    QMap<QString, QString> propParts;
    for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/ctrlProp")))
        if (!r.external && package.hasPart(r.target)) propParts.insert(r.id, r.target);
    QVector<SheetControl> sheetControls;
    if (!propParts.isEmpty() && package.partSize(sheetPart) <= kMaxControlPartBytes) sheetControls = readSheetControls(package.part(sheetPart));
    QMap<qint64, int> byShape;
    for (int i = 0; i < sheetControls.size(); ++i)
        if (sheetControls[i].shapeId > 0) byShape.insert(sheetControls[i].shapeId, i);
    QSet<qint64> seen;
    auto addProps = [&](FormControl *c, const SheetControl &sc) {
        if (!sc.name.isEmpty()) c->name = sc.name;
        // (a shape that says no cells, and a sheet that does: its anchor is the better)
        if (c->anchor.kind == AnchorKind::Absolute && sc.hasAnchor && (sc.anchor.to.col > sc.anchor.from.col || sc.anchor.to.row > sc.anchor.from.row)) c->anchor = sc.anchor;
        if (sc.hasPr) {
            c->macro = sc.macro.isEmpty() ? c->macro : sc.macro;
            c->prints = sc.prints;
            c->locked = sc.locked;
        }
        const auto part = propParts.constFind(sc.relId);
        if (part != propParts.constEnd() && package.partSize(*part) <= kMaxControlPartBytes) {
            c->ctrlPart = *part;
            assign(c, readProp(package.part(*part)));
        }
    };
    for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/vmlDrawing"))) {
        if (r.external) continue;
        const qint64 size = package.partSize(r.target);
        if (size <= 0 || size > kMaxControlPartBytes) continue;
        const QByteArray vml = package.part(r.target);
        if (!mayHaveControls(vml)) continue;
        for (const DrawingShape &shape : drawingShapes(vml, kMaxFormControls * 4)) {
            FormControl c;
            c.vmlPart = r.target;
            g_shapeReads.fetch_add(1, std::memory_order_relaxed);
            if (!readShape(vml, shape, &c)) continue;
            if (out.size() >= kMaxFormControls) {
                if (truncated) *truncated = true;
                return out;
            }
            const auto sc = c.shapeId > 0 ? byShape.constFind(c.shapeId) : byShape.constEnd();
            if (sc != byShape.constEnd()) addProps(&c, sheetControls[*sc]);
            seen.insert(c.shapeId);
            out.push_back(c);
        }
    }
    // controls the sheet names that no drawing has a shape for: the properties part and the sheet's anchor are all there is
    for (const SheetControl &sc : std::as_const(sheetControls)) {
        if (sc.shapeId <= 0 || seen.contains(sc.shapeId) || !sc.hasPr || !sc.hasAnchor || out.size() >= kMaxFormControls) continue;
        FormControl c;
        c.shapeId = sc.shapeId;
        addProps(&c, sc);
        if (c.ctrlPart.isEmpty()) continue;
        c.anchor = sc.anchor;
        out.push_back(c);
    }
    return out;
}

int shapeNumberOf(QByteArrayView id)
{
    if (!id.startsWith("_x0000_s")) return 0;
    bool ok = false;
    const qint64 n = QByteArray(id.sliced(8)).toLongLong(&ok);
    return ok && n > 0 && n <= kMaxShapeId ? int(n) : 0;
}

QSet<qint64> shapeIdsOf(const QByteArray &vml)
{
    QSet<qint64> ids;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(vml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "shape") continue;
        const qint64 id = shapeIdOf(QByteArrayView(vml).sliced(t.start, t.end - t.start));
        if (id > 0) ids.insert(id);
    }
    return ids;
}

qint64 largestShapeId(const QByteArray &vml)
{
    qint64 most = 0;
    for (qint64 id : shapeIdsOf(vml)) most = std::max(most, id);
    return most;
}

qint64 freeShapeId(const QSet<qint64> &taken, qint64 after)
{
    // (each step either finds a free number or passes one that is taken, so the search is as long as the set at most)
    const qint64 from = std::max<qint64>(after, 1024);
    for (qint64 id = from + 1; id <= kMaxShapeId; ++id)
        if (!taken.contains(id)) return id;
    for (qint64 id = 1025; id <= std::min(from, kMaxShapeId); ++id)
        if (!taken.contains(id)) return id;
    return 0;
}

qint64 controlShapeReads() { return g_shapeReads.load(std::memory_order_relaxed); }
qint64 controlEntryProbes() { return g_entryProbes.load(std::memory_order_relaxed); }

QByteArray newControlsDrawing(int block)
{
    QByteArray head = kHead;
    head.replace("data=\"1\"", "data=\"" + QByteArray::number(std::max(1, block)) + "\"");
    return head + kShapeType + "</xml>";
}

QByteArray controlShapeXml(const FormControl &c, qint64 id)
{
    const bool hasText = c.kind == ControlKind::CheckBox || c.kind == ControlKind::Radio || c.kind == ControlKind::Button || c.kind == ControlKind::GroupBox || c.kind == ControlKind::Label;
    const bool isButton = c.kind == ControlKind::Button;
    const bool plain = c.kind == ControlKind::CheckBox || c.kind == ControlKind::Radio || c.kind == ControlKind::GroupBox || c.kind == ControlKind::Label;
    auto pt = [](double v) { return QByteArray::number(std::round(v * 100) / 100, 'f', 2); };
    QByteArray x = "<v:shape id=\"_x0000_s" + num(id) + "\" type=\"#_x0000_t201\" style=\"position:absolute;margin-left:" + pt(c.place.left()) + "pt;margin-top:" + pt(c.place.top()) + "pt;width:"
                   + pt(c.place.width()) + "pt;height:" + pt(c.place.height()) + "pt;z-index:" + num(id % 1000000) + ";mso-wrap-style:tight\" ";
    if (isButton) x += "fillcolor=\"buttonFace [67]\" strokecolor=\"windowText [64]\" o:insetmode=\"auto\">";
    else if (plain) x += "filled=\"f\" fillcolor=\"window [65]\" stroked=\"f\" strokecolor=\"windowText [64]\" o:insetmode=\"auto\">";
    else x += "fillcolor=\"window [65]\" strokecolor=\"windowText [64]\" o:insetmode=\"auto\">";
    x += "<v:path shadowok=\"t\" strokeok=\"t\" fillok=\"t\"/><o:lock v:ext=\"edit\" rotation=\"t\"/>";
    if (hasText)
        x += "<v:textbox style=\"mso-direction-alt:auto\" o:singleclick=\"f\"><div style=\"text-align:" + QByteArray(isButton ? "center" : "left") + "\"><font face=\"Segoe UI\" size=\"160\" color=\"auto\">"
             + escaped(c.text) + "</font></div></v:textbox>";
    x += "<x:ClientData ObjectType=\"" + QByteArray(controlVmlType(c.kind)) + "\"><x:SizeWithCells/><x:Anchor>" + anchorText(c.anchor) + "</x:Anchor><x:PrintObject>" + (c.prints ? "True" : "False")
         + "</x:PrintObject><x:AutoFill>False</x:AutoFill>";
    if (hasText && !isButton) x += "<x:AutoLine>False</x:AutoLine>";
    if (hasText) x += "<x:TextVAlign>Center</x:TextVAlign>";
    if (isButton) x += "<x:TextHAlign>Center</x:TextHAlign>";
    if (!c.threeD) x += "<x:NoThreeD/>";
    if (c.kind == ControlKind::Radio && c.firstButton) x += "<x:FirstButton/>";
    if (c.horizontal) x += "<x:Horiz/>";
    QByteArray tail;
    switch (c.kind) {
    case ControlKind::Drop: tail += "<x:DropStyle>Combo</x:DropStyle><x:DropLines>" + num(c.dropLines) + "</x:DropLines>"; break;
    case ControlKind::List: tail += "<x:SelType>" + escaped(c.selType.isEmpty() ? QStringLiteral("Single") : c.selType) + "</x:SelType>"; break;
    default: break;
    }
    if (c.kind == ControlKind::Spin || c.kind == ControlKind::Scroll)
        tail += "<x:Val>" + num(c.val) + "</x:Val><x:Min>" + num(c.min) + "</x:Min><x:Max>" + num(c.max) + "</x:Max><x:Inc>" + num(c.inc) + "</x:Inc>" + (c.kind == ControlKind::Scroll ? "<x:Page>" + num(c.page) + "</x:Page>" : QByteArray());
    if ((c.kind == ControlKind::Drop || c.kind == ControlKind::List) && c.sel > 0) tail += "<x:Sel>" + num(c.sel) + "</x:Sel>";
    if (!c.macro.isEmpty() && isButton) tail += "<x:FmlaMacro>" + escaped(c.macro) + "</x:FmlaMacro>";
    if (!c.range.isEmpty()) tail += "<x:FmlaRange>" + escaped(c.range) + "</x:FmlaRange>";
    if (!c.link.isEmpty()) tail += "<x:FmlaLink>" + escaped(c.link) + "</x:FmlaLink>";
    if ((c.kind == ControlKind::CheckBox || c.kind == ControlKind::Radio) && c.checked) tail += "<x:Checked>" + num(c.checked) + "</x:Checked>";
    x += tail + "</x:ClientData></v:shape>";
    return x;
}

bool patchControlsVml(const QByteArray &vml, const std::vector<FormControl> &was, const std::vector<FormControl> &now, QByteArray *out, QString *why, std::map<qint64, qint64> *assigned, int block)
{
    *out = vml;
    QMap<qint64, const FormControl *> before;
    for (const FormControl &c : was) before.insert(c.shapeId, &c);
    QMap<qint64, const FormControl *> after;
    bool anyNew = false;
    for (const FormControl &c : now) {
        if (c.isNew) anyNew = true;
        else after.insert(c.shapeId, &c);
    }
    QByteArray result;
    qsizetype at = 0;
    bool changed = false;
    for (const DrawingShape &shape : drawingShapes(vml, kMaxFormControls * 8)) {
        const QByteArrayView body = QByteArrayView(vml).sliced(shape.start, shape.end - shape.start);
        Tag first;
        if (!xmlscan::nextTag(body, 0, &first)) continue;
        const qint64 id = shapeIdOf(body.sliced(first.start, first.end - first.start));
        if (id <= 0) continue;   // (a shape with no number this program uses is not one of the controls)
        const auto a = after.constFind(id), b = before.constFind(id);
        if (a != after.constEnd() && (*a)->removed) {
            // (a control that was taken away: its shape goes)
            result += vml.mid(at, shape.start - at);
            at = shape.end;
            changed = true;
            continue;
        }
        if (a == after.constEnd() || b == before.constEnd() || sameControlState(**a, **b)) continue;
        QByteArray piece = vml.mid(shape.start, shape.end - shape.start);
        if (!sameCells((*a)->anchor, (*b)->anchor) && !setChild(&piece, "Anchor", anchorText((*a)->anchor))) {
            if (why) *why = QCoreApplication::translate("jp::office::sheet::Save", "A form control's shape could not be changed.");
            return false;
        }
        for (const Field &f : kFields) {
            if (!f.differs(**a, **b)) continue;
            // (only the fields that mean something for the kind)
            const QByteArray name = f.element;
            if ((name == "Checked" && (*a)->kind != ControlKind::CheckBox && (*a)->kind != ControlKind::Radio) || ((name == "Val" || name == "Min" || name == "Max" || name == "Inc" || name == "Page")
                                                                                                                  && (*a)->kind != ControlKind::Spin && (*a)->kind != ControlKind::Scroll)
                || (name == "Sel" && (*a)->kind != ControlKind::Drop && (*a)->kind != ControlKind::List) || (name == "DropLines" && (*a)->kind != ControlKind::Drop))
                continue;
            if (!setChild(&piece, name, f.value(**a))) {
                if (why) *why = QCoreApplication::translate("jp::office::sheet::Save", "A form control's shape could not be changed.");
                return false;
            }
        }
        result += vml.mid(at, shape.start - at);
        result += piece;
        at = shape.end;
        changed = true;
    }
    if (!changed && !anyNew) return true;
    result += vml.mid(at);
    if (anyNew) {
        // (every number the drawing has, not those of its first few thousand shapes only: a note's shape far down is as taken as the first)
        QSet<qint64> taken = shapeIdsOf(result);
        const bool fresh = block > 0;
        qint64 id = fresh ? std::min<qint64>(qint64(block) * 1024, kMaxShapeId - 1) : 1024;
        if (!fresh)
            for (qint64 t : std::as_const(taken)) id = std::max(id, t);
        QByteArray added;
        if (!result.contains("_x0000_t201\"") && !result.contains("_x0000_t201'")) added += kShapeType;
        for (const FormControl &c : now) {
            if (!c.isNew) continue;
            const bool asked = !fresh && c.shapeId > 0 && c.shapeId <= kMaxShapeId && !taken.contains(c.shapeId);
            const qint64 use = asked ? c.shapeId : freeShapeId(taken, id);
            if (use <= 0) {
                if (why) *why = QCoreApplication::translate("jp::office::sheet::Save", "The drawing of a sheet's controls could not be changed.");
                return false;
            }
            if (assigned && use != c.shapeId) (*assigned)[c.shapeId] = use;
            taken.insert(use);
            id = std::max(id, use);
            added += controlShapeXml(c, use);
        }
        const qsizetype close = result.lastIndexOf("</");
        if (close < 0) {
            if (why) *why = QCoreApplication::translate("jp::office::sheet::Save", "The drawing of a sheet's controls could not be changed.");
            return false;
        }
        result.insert(close, added);
    }
    *out = result;
    return true;
}

namespace {

// The attribute's value range in a tag, or -1.
void setAttr(QByteArray *tag, const QByteArray &name, const std::optional<QByteArray> &value)
{
    qsizetype s, e;
    if (xmlscan::attributeValue(*tag, name, &s, &e)) {
        if (!value) {
            // the whole attribute, and the space before it
            qsizetype from = s - 1;   // the quote
            while (from > 0 && (*tag)[from] != ' ' && (*tag)[from] != '\n' && (*tag)[from] != '\t' && (*tag)[from] != '\r') --from;
            const qsizetype to = e + 1;
            tag->remove(from, to - from);
        } else {
            tag->replace(s, e - s, *value);
        }
    } else if (value) {
        qsizetype end = tag->size() - 1;   // before ">" or "/>"
        if (end > 0 && (*tag)[end - 1] == '/') --end;
        tag->insert(end, " " + name + "=\"" + *value + "\"");
    }
}

} // namespace

QByteArray patchControlProp(const QByteArray &xml, const FormControl &c)
{
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "formControlPr") continue;
        QByteArray tag = xml.mid(t.start, t.end - t.start);
        const PropAttrs p = readProp(xml);
        const bool box = c.kind == ControlKind::CheckBox || c.kind == ControlKind::Radio, num2 = c.kind == ControlKind::Spin || c.kind == ControlKind::Scroll;
        if (box && c.checked != p.checked) setAttr(&tag, "checked", c.checked == 1 ? std::optional<QByteArray>("Checked") : c.checked == 2 ? std::optional<QByteArray>("Mixed") : std::nullopt);
        if (c.link != p.link) setAttr(&tag, "fmlaLink", c.link.isEmpty() ? std::nullopt : std::optional<QByteArray>(xmlscan::escapeAttribute(c.link)));
        if (c.range != p.range) setAttr(&tag, "fmlaRange", c.range.isEmpty() ? std::nullopt : std::optional<QByteArray>(xmlscan::escapeAttribute(c.range)));
        if (num2) {
            if (c.val != p.val) setAttr(&tag, "val", num(c.val));
            if (c.min != p.min) setAttr(&tag, "min", num(c.min));
            if (c.max != p.max) setAttr(&tag, "max", num(c.max));
            if (c.inc != p.inc) setAttr(&tag, "inc", num(c.inc));
            if (c.page != p.page) setAttr(&tag, "page", num(c.page));
        }
        if ((c.kind == ControlKind::Drop || c.kind == ControlKind::List) && c.sel != p.sel) setAttr(&tag, "sel", c.sel > 0 ? std::optional<QByteArray>(num(c.sel)) : std::nullopt);
        if (c.kind == ControlKind::Drop && c.dropLines != p.dropLines) setAttr(&tag, "dropLines", num(c.dropLines));
        if (tag == xml.mid(t.start, t.end - t.start)) return xml;
        return xml.left(t.start) + tag + xml.mid(t.end);
    }
    return xml;
}

} // namespace jp::office::sheet
