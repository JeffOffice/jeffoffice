#include "core/notesmaster.h"

#include <QJsonArray>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {
// In points: the printed notes pages keep 0.75 inch clear on every side, a handout's header and footer sit 0.5 inch in from the page's edges (as output has always drawn them).
constexpr double kMargin = 54, kEdge = 36;
constexpr double kLeast = 0.005;
const QString kNotesKey = QStringLiteral("notesMaster"), kHandoutKey = QStringLiteral("handoutMaster"), kPerPageKey = QStringLiteral("handoutPerPage");

QRectF fitInside(const QSizeF &size, const QRectF &box)
{
    if (size.width() <= 0 || size.height() <= 0) return box;
    const double k = std::min(box.width() / size.width(), box.height() / size.height());
    const QSizeF fitted = size * k;
    return QRectF(box.center() - QPointF(fitted.width() / 2, fitted.height() / 2), fitted);
}

QRectF fraction(const QRectF &points, const QSizeF &paper) { return QRectF(points.x() / paper.width(), points.y() / paper.height(), points.width() / paper.width(), points.height() / paper.height()); }
QRectF inPoints(const QRectF &f, const QSizeF &paper) { return QRectF(f.x() * paper.width(), f.y() * paper.height(), f.width() * paper.width(), f.height() * paper.height()); }

QJsonArray toArray(const QRectF &r) { return QJsonArray{r.x(), r.y(), r.width(), r.height()}; }
QRectF fromArray(const QJsonValue &v, const QRectF &fallback)
{
    const QJsonArray a = v.toArray();
    if (a.size() != 4) return fallback;
    return sanitizedPlace(QRectF(a[0].toDouble(), a[1].toDouble(), a[2].toDouble(), a[3].toDouble()), fallback);
}
QColor colorOf(const QJsonObject &o)
{
    const QColor c(o.value(QStringLiteral("background")).toString());
    return c.isValid() ? QColor(c.red(), c.green(), c.blue()) : QColor();
}
bool sameColor(const QColor &a, const QColor &b) { return a.isValid() == b.isValid() && (!a.isValid() || a.rgb() == b.rgb()); }
bool sameRect(const QRectF &a, const QRectF &b)
{
    return std::abs(a.x() - b.x()) < 1e-9 && std::abs(a.y() - b.y()) < 1e-9 && std::abs(a.width() - b.width()) < 1e-9 && std::abs(a.height() - b.height()) < 1e-9;
}
} // namespace

QRectF sanitizedPlace(const QRectF &r, const QRectF &fallback)
{
    if (!std::isfinite(r.x()) || !std::isfinite(r.y()) || !std::isfinite(r.width()) || !std::isfinite(r.height())) return fallback;
    const double w = std::clamp(r.width(), kLeast, 1.0), h = std::clamp(r.height(), kLeast, 1.0);
    return QRectF(std::clamp(r.x(), 0.0, 1.0 - w), std::clamp(r.y(), 0.0, 1.0 - h), w, h);
}

QRectF &NotesMaster::part(NotesPart p)
{
    switch (p) {
    case NotesPart::SlideImage: return slideImage;
    case NotesPart::Body: return body;
    case NotesPart::Header: return header;
    case NotesPart::Date: return date;
    case NotesPart::Footer: return footer;
    default: return number;
    }
}
const QRectF &NotesMaster::part(NotesPart p) const { return const_cast<NotesMaster *>(this)->part(p); }

// What the file formats' own notes and handout masters have: the standard's places on the page (the proportions of a 7.5 by 10 inch notes page).
NotesMaster standardNotesMaster(const QSizeF &slide)
{
    constexpr double w = 6858000, h = 9144000;
    NotesMaster m;
    m.slideImage = QRectF(685800 / w, 1143000 / h, 5486400 / w, slide.width() > 0 ? 5486400 / w * slide.height() / slide.width() * w / h : 3086100 / h);
    m.body = QRectF(685800 / w, 4400550 / h, 5486400 / w, 3600450 / h);
    m.header = QRectF(0, 0, 2971800 / w, 458788 / h);
    m.date = QRectF(3884613 / w, 0, 2971800 / w, 458788 / h);
    m.footer = QRectF(0, 8685213 / h, 2971800 / w, 458788 / h);
    m.number = QRectF(3884613 / w, 8685213 / h, 2971800 / w, 458788 / h);
    return m;
}

HandoutMaster standardHandoutMaster()
{
    constexpr double w = 6858000, h = 9144000;
    HandoutMaster m = defaultHandoutMaster(QSizeF(w / 12700, h / 12700));
    m.header = QRectF(0, 0, 2971800 / w, 458788 / h);
    m.date = QRectF(3884613 / w, 0, 2971800 / w, 458788 / h);
    m.footer = QRectF(0, 8685213 / h, 2971800 / w, 458788 / h);
    m.number = QRectF(3884613 / w, 8685213 / h, 2971800 / w, 458788 / h);
    return m;
}

bool nearly(const QRectF &a, const QRectF &b, double tolerance)
{
    return std::abs(a.x() - b.x()) <= tolerance && std::abs(a.y() - b.y()) <= tolerance && std::abs(a.width() - b.width()) <= tolerance && std::abs(a.height() - b.height()) <= tolerance;
}

bool NotesMaster::operator==(const NotesMaster &o) const
{
    return custom == o.custom && sameRect(slideImage, o.slideImage) && sameRect(body, o.body) && sameRect(header, o.header) && sameRect(date, o.date) && sameRect(footer, o.footer)
           && sameRect(number, o.number) && sameColor(background, o.background);
}

QRectF &HandoutMaster::part(HandoutPart p)
{
    switch (p) {
    case HandoutPart::Area: return area;
    case HandoutPart::Header: return header;
    case HandoutPart::Date: return date;
    case HandoutPart::Footer: return footer;
    default: return number;
    }
}
const QRectF &HandoutMaster::part(HandoutPart p) const { return const_cast<HandoutMaster *>(this)->part(p); }

bool HandoutMaster::operator==(const HandoutMaster &o) const
{
    return custom == o.custom && perPage == o.perPage && landscape == o.landscape && sameRect(area, o.area) && sameRect(header, o.header) && sameRect(date, o.date) && sameRect(footer, o.footer)
           && sameRect(number, o.number) && sameColor(background, o.background);
}

bool isHandoutCount(int perPage) { return perPage == 1 || perPage == 2 || perPage == 3 || perPage == 4 || perPage == 6 || perPage == 9; }

NotesMaster defaultNotesMaster(const QSizeF &paper, const QSizeF &slide)
{
    NotesMaster m;
    if (paper.width() <= 0 || paper.height() <= 0) return m;
    const double w = paper.width() - 2 * kMargin, half = paper.height() / 2;
    // The slide at the top, as large as the upper half lets it be, and the notes under it.
    QRectF target = fitInside(slide, QRectF(kMargin, kMargin, w, half - kMargin - 18));
    target.moveTop(kMargin);
    const double top = target.bottom() + 24, edgeW = (paper.width() - 2 * kEdge) / 2;
    m.slideImage = fraction(target, paper);
    m.body = fraction(QRectF(kMargin, top, w, std::max(paper.height() - kMargin - 24 - top, 20.0)), paper);
    m.header = fraction(QRectF(kEdge, kEdge - 6, edgeW, 12), paper);
    m.date = fraction(QRectF(kEdge + edgeW, kEdge - 6, edgeW, 12), paper);
    m.footer = fraction(QRectF(kEdge, paper.height() - kEdge - 6, edgeW, 12), paper);
    m.number = fraction(QRectF(kEdge + edgeW, paper.height() - kEdge - 6, edgeW, 12), paper);
    return m;
}

HandoutMaster defaultHandoutMaster(const QSizeF &paper)
{
    HandoutMaster m;
    if (paper.width() <= 0 || paper.height() <= 0) return m;
    const double edgeW = (paper.width() - 2 * kEdge) / 2;
    m.area = fraction(QRectF(kEdge, 60, paper.width() - 2 * kEdge, paper.height() - 120), paper);
    m.header = fraction(QRectF(kEdge, kEdge - 6, edgeW, 12), paper);
    m.date = fraction(QRectF(kEdge + edgeW, kEdge - 6, edgeW, 12), paper);
    m.footer = fraction(QRectF(kEdge, paper.height() - kEdge - 6, edgeW, 12), paper);
    m.number = fraction(QRectF(kEdge + edgeW, paper.height() - kEdge - 6, edgeW, 12), paper);
    return m;
}

QJsonObject notesMasterJson(const NotesMaster &m)
{
    QJsonObject o{{QStringLiteral("slideImage"), toArray(m.slideImage)}, {QStringLiteral("body"), toArray(m.body)},     {QStringLiteral("header"), toArray(m.header)},
                  {QStringLiteral("date"), toArray(m.date)},             {QStringLiteral("footer"), toArray(m.footer)}, {QStringLiteral("number"), toArray(m.number)}};
    if (m.background.isValid()) o[QStringLiteral("background")] = m.background.name();
    return o;
}

NotesMaster notesMasterFromJson(const QJsonObject &o, const NotesMaster &fallback)
{
    NotesMaster m = fallback;
    m.custom = true;
    m.slideImage = fromArray(o.value(QStringLiteral("slideImage")), fallback.slideImage);
    m.body = fromArray(o.value(QStringLiteral("body")), fallback.body);
    m.header = fromArray(o.value(QStringLiteral("header")), fallback.header);
    m.date = fromArray(o.value(QStringLiteral("date")), fallback.date);
    m.footer = fromArray(o.value(QStringLiteral("footer")), fallback.footer);
    m.number = fromArray(o.value(QStringLiteral("number")), fallback.number);
    m.background = colorOf(o);
    return m;
}

QJsonObject handoutMasterJson(const HandoutMaster &m)
{
    QJsonObject o{{QStringLiteral("landscape"), m.landscape}, {QStringLiteral("area"), toArray(m.area)},     {QStringLiteral("header"), toArray(m.header)},
                  {QStringLiteral("date"), toArray(m.date)},   {QStringLiteral("footer"), toArray(m.footer)}, {QStringLiteral("number"), toArray(m.number)}};
    if (m.background.isValid()) o[QStringLiteral("background")] = m.background.name();
    return o;
}

HandoutMaster handoutMasterFromJson(const QJsonObject &o, const HandoutMaster &fallback)
{
    HandoutMaster m = fallback;
    m.custom = true;
    m.landscape = o.value(QStringLiteral("landscape")).toBool(fallback.landscape);
    m.area = fromArray(o.value(QStringLiteral("area")), fallback.area);
    m.header = fromArray(o.value(QStringLiteral("header")), fallback.header);
    m.date = fromArray(o.value(QStringLiteral("date")), fallback.date);
    m.footer = fromArray(o.value(QStringLiteral("footer")), fallback.footer);
    m.number = fromArray(o.value(QStringLiteral("number")), fallback.number);
    m.background = colorOf(o);
    return m;
}

bool hasNotesMaster(const Document &d) { return d.appData.value(kNotesKey).isObject(); }
bool hasHandoutMaster(const Document &d) { return d.appData.value(kHandoutKey).isObject(); }

NotesMaster notesMasterOf(const Document &d, const QSizeF &paper)
{
    const NotesMaster fallback = defaultNotesMaster(paper, d.pageSize());
    return hasNotesMaster(d) ? notesMasterFromJson(d.appData.value(kNotesKey).toObject(), fallback) : fallback;
}

HandoutMaster handoutMasterOf(const Document &d, const QSizeF &paper)
{
    HandoutMaster fallback = defaultHandoutMaster(paper);
    // (how many slides a page holds is kept apart from the places: a file can say it without a handout master)
    const int per = d.appData.value(kPerPageKey).toInt(fallback.perPage);
    fallback.perPage = isHandoutCount(per) ? per : fallback.perPage;
    return hasHandoutMaster(d) ? handoutMasterFromJson(d.appData.value(kHandoutKey).toObject(), fallback) : fallback;
}

void setNotesMaster(Document &d, const NotesMaster &m) { d.appData[kNotesKey] = notesMasterJson(m); }
void setHandoutMaster(Document &d, const HandoutMaster &m)
{
    d.appData[kHandoutKey] = handoutMasterJson(m);
    if (hasHandoutPerPage(d) || m.perPage != 6) setHandoutPerPage(d, m.perPage);
}
void setHandoutPerPage(Document &d, int perPage) { d.appData[kPerPageKey] = isHandoutCount(perPage) ? perPage : 6; }
bool hasHandoutPerPage(const Document &d) { return d.appData.contains(kPerPageKey); }
void clearNotesMaster(Document &d) { d.appData.remove(kNotesKey); }
void clearHandoutMaster(Document &d)
{
    d.appData.remove(kHandoutKey);
    d.appData.remove(kPerPageKey);
}

NotesPageGeometry notesPageGeometry(const NotesMaster &m, const QSizeF &paper, const QSizeF &slide)
{
    NotesPageGeometry g;
    g.slideImage = fitInside(slide, inPoints(m.slideImage, paper));
    g.body = inPoints(m.body, paper);
    g.header = inPoints(m.header, paper);
    g.date = inPoints(m.date, paper);
    g.footer = inPoints(m.footer, paper);
    g.number = inPoints(m.number, paper);
    return g;
}

HandoutGeometry handoutGeometry(const HandoutMaster &m, const QSizeF &paper, int perPage, bool horizontalOrder)
{
    HandoutGeometry g;
    g.area = inPoints(m.area, paper);
    g.header = inPoints(m.header, paper);
    g.date = inPoints(m.date, paper);
    g.footer = inPoints(m.footer, paper);
    g.number = inPoints(m.number, paper);
    int columns = 1, rows = 1;
    bool lines = false;
    switch (perPage) {
    case 2: rows = 2; break;
    case 3: rows = 3; lines = true; break;
    case 4: columns = 2; rows = 2; break;
    case 6: columns = 2; rows = 3; break;
    case 9: columns = 3; rows = 3; break;
    default: break;
    }
    const double cellW = g.area.width() / columns, cellH = g.area.height() / rows;
    constexpr double gap = 10;
    for (int k = 0; k < columns * rows; ++k) {
        const int column = horizontalOrder ? k % columns : k / rows, row = horizontalOrder ? k / columns : k % rows;
        HandoutSlot s;
        s.cell = QRectF(g.area.left() + column * cellW, g.area.top() + row * cellH, cellW, cellH);
        s.room = s.cell.adjusted(gap, gap, -gap, -gap);
        s.lines = lines;
        if (lines) s.room.setWidth(s.cell.width() * 0.46 - gap);
        g.slots << s;
    }
    return g;
}

QSizeF handoutPaper(const HandoutMaster &m, const QSizeF &paper)
{
    const bool wide = paper.width() > paper.height();
    return m.landscape != wide ? paper.transposed() : paper;
}

} // namespace jp::deck
