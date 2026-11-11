#include "sheet/odsdraw.h"

#include "odt/odtcommon.h"
#include "sheet/odsdraw_p.h"
#include "sheet/odsstyles.h"

#include <QBuffer>
#include <QImageReader>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet::odsdraw {

namespace {
constexpr double kEmuPerPoint = 12700.0;
}

// ---- the grid

void Grid::Axis::build(QVector<Run> list, double defaultPt)
{
    defaultEmu = qint64(std::llround(std::max(0.0, defaultPt) * kEmuPerPoint));
    std::stable_sort(list.begin(), list.end(), [](const Run &a, const Run &b) { return a.first < b.first; });
    runs.clear();
    sizeBefore.clear();
    qint64 extra = 0;
    for (const Run &r : std::as_const(list)) {
        if (r.last < r.first || (!runs.isEmpty() && r.first <= runs.last().last)) continue;
        runs.append(r);
        sizeBefore.append(extra);
        const qint64 size = qint64(std::llround(std::max(0.0, r.points) * kEmuPerPoint));
        extra += (size - defaultEmu) * qint64(r.last - r.first + 1);
    }
}

qint64 Grid::Axis::size(quint32 index) const
{
    int lo = 0, hi = int(runs.size()) - 1, found = -1;
    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        if (runs[mid].first <= index) {
            found = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    if (found < 0 || index > runs[found].last) return defaultEmu;
    return qint64(std::llround(std::max(0.0, runs[found].points) * kEmuPerPoint));
}

qint64 Grid::Axis::start(quint32 index) const
{
    int lo = 0, hi = int(runs.size()) - 1, found = -1;
    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        if (runs[mid].first <= index) {
            found = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    qint64 at = qint64(index) * defaultEmu;
    if (found >= 0) {
        const Run &r = runs[found];
        const qint64 size = qint64(std::llround(std::max(0.0, r.points) * kEmuPerPoint));
        at += sizeBefore[found] + (size - defaultEmu) * qint64(std::min(index, r.last + 1) - r.first);
    }
    return at;
}

void Grid::Axis::at(qint64 distance, quint32 count, quint32 *index, qint64 *offset) const
{
    if (distance < 0) distance = 0;
    // the last cell that starts at or before the distance (cells of no size share a start with the one after them)
    quint32 lo = 0, hi = count - 1;
    while (lo < hi) {
        const quint32 mid = lo + (hi - lo + 1) / 2;
        if (start(mid) <= distance) lo = mid;
        else hi = mid - 1;
    }
    *index = lo;
    *offset = std::max<qint64>(0, distance - start(lo));
}

Grid::Grid(const QVector<Run> &columns, double defaultColPt, const QVector<Run> &rows, double defaultRowPt)
{
    m_cols.build(columns, defaultColPt);
    m_rows.build(rows, defaultRowPt);
}

void Grid::boxOf(const DrawingAnchor &a, qint64 *x, qint64 *y, qint64 *cx, qint64 *cy) const
{
    if (a.kind == AnchorKind::Absolute) {
        *x = a.x;
        *y = a.y;
        *cx = a.cx;
        *cy = a.cy;
        return;
    }
    *x = columnStart(a.from.col) + a.from.colOff;
    *y = rowStart(a.from.row) + a.from.rowOff;
    if (a.kind == AnchorKind::OneCell) {
        *cx = a.cx;
        *cy = a.cy;
        return;
    }
    *cx = std::max<qint64>(0, columnStart(a.to.col) + a.to.colOff - *x);
    *cy = std::max<qint64>(0, rowStart(a.to.row) + a.to.rowOff - *y);
}

// ---- lengths

QString lengthText(qint64 emu) { return QString::number(double(emu) / 914400.0, 'f', 7) + QStringLiteral("in"); }

bool lengthEmu(const QString &text, qint64 *emu)
{
    double points = 0;
    if (!ods::lengthPoints(text, &points)) return false;
    const double v = points * kEmuPerPoint;
    *emu = v >= double(kMaxDrawingEmu) ? kMaxDrawingEmu : v <= -double(kMaxDrawingEmu) ? -kMaxDrawingEmu : qint64(std::llround(v));
    return true;
}

// ---- the elements

namespace {

QString nameOf(const QXmlStreamReader &xml)
{
    const QString uri = xml.namespaceUri().toString();
    if (uri == QLatin1String("http://www.w3.org/1999/xlink")) return QStringLiteral("xlink:") + xml.name().toString();
    return ods::canonicalName(uri, xml.name().toString());
}

} // namespace

bool readNode(QXmlStreamReader &xml, int depthLeft, Counters *counters, NodePtr *out)
{
    // (a stack, not recursion: the nesting is a stranger's)
    std::vector<NodePtr> stack;
    NodePtr root;
    auto begin = [&](bool keep) -> NodePtr {
        if (!keep) return nullptr;
        auto n = std::make_shared<Node>();
        n->name = nameOf(xml);
        for (const QXmlStreamAttribute &a : xml.attributes()) {
            const QString key = a.namespaceUri() == QLatin1String("http://www.w3.org/1999/xlink") ? QStringLiteral("xlink:") + a.name().toString() : ods::canonicalName(a.namespaceUri().toString(), a.name().toString());
            if (n->attrs.size() < 200) n->attrs.insert(key, a.value().toString());
        }
        n->texts << QString();
        return n;
    };
    // `kept` is false for what is past a limit: it is read through and not made.
    int skipping = 0;
    {
        const bool keep = counters->nodes < kMaxNodes;
        if (keep) ++counters->nodes;
        else counters->over = true;
        if (!keep) ++skipping;
        root = begin(keep);
        if (root) stack.push_back(root);
    }
    int depth = 1;
    while (depth > 0) {
        if (xml.atEnd()) return false;
        const QXmlStreamReader::TokenType t = xml.readNext();
        if (t == QXmlStreamReader::StartElement) {
            if (++depth > depthLeft) return false;
            const bool keep = !skipping && counters->nodes < kMaxNodes;
            if (keep) {
                ++counters->nodes;
                NodePtr n = begin(true);
                stack.back()->children.push_back(n);
                stack.back()->texts << QString();
                stack.push_back(n);
            } else {
                counters->over = true;
                ++skipping;
            }
        } else if (t == QXmlStreamReader::EndElement) {
            --depth;
            if (skipping) --skipping;
            else if (!stack.empty()) stack.pop_back();
        } else if (t == QXmlStreamReader::Characters) {
            if (!skipping && !stack.empty()) {
                const QStringView text = xml.text();
                if (counters->chars + text.size() > kMaxNodeChars) {
                    counters->over = true;
                } else {
                    counters->chars += text.size();
                    stack.back()->texts.last() += text;
                }
            }
        } else if (t == QXmlStreamReader::DTD || t == QXmlStreamReader::EntityReference || t == QXmlStreamReader::Invalid) {
            return false;
        }
    }
    *out = root;
    return true;
}


// ---- names and pictures

namespace {

const NamePair kShapes[] = {
    {"rect", "rectangle"},         {"roundRect", "round-rectangle"}, {"ellipse", "ellipse"},     {"diamond", "diamond"},        {"triangle", "isosceles-triangle"},
    {"rtTriangle", "right-triangle"}, {"parallelogram", "parallelogram"}, {"trapezoid", "trapezoid"}, {"hexagon", "hexagon"},   {"octagon", "octagon"},
    {"plus", "cross"},             {"donut", "ring"},                {"can", "can"},             {"cube", "cube"},              {"rightArrow", "right-arrow"},
    {"leftArrow", "left-arrow"},   {"upArrow", "up-arrow"},          {"downArrow", "down-arrow"}, {"star5", "star5"},            {"heart", "heart"},
    {"sun", "sun"},                {"moon", "moon"},                 {"smileyFace", "smiley"},   {"flowChartProcess", "flowchart-process"},
    {"flowChartDecision", "flowchart-decision"}, {"flowChartTerminator", "flowchart-terminator"},
};

// The fontwork shapes of the standard (the names its fontwork gallery has) and the text warp each stands for. A warp that is not here is written as plain text.
const NamePair kFontwork[] = {
    {"textPlain", "fontwork-plain-text"},         {"textStop", "fontwork-stop"},          {"textTriangle", "fontwork-triangle-up"}, {"textTriangleInverted", "fontwork-triangle-down"},
    {"textChevron", "fontwork-chevron-up"},       {"textChevronInverted", "fontwork-chevron-down"}, {"textRingInside", "fontwork-ring-inside"},
    {"textRingOutside", "fontwork-ring-outside"}, {"textArchUp", "fontwork-arch-up-curve"}, {"textArchDown", "fontwork-arch-down-curve"}, {"textCircle", "fontwork-circle-curve"},
    {"textButton", "fontwork-open-circle-curve"}, {"textArchUpPour", "fontwork-arch-up-pour"}, {"textArchDownPour", "fontwork-arch-down-pour"},
    {"textCirclePour", "fontwork-circle-pour"},   {"textButtonPour", "fontwork-open-circle-pour"}, {"textCurveUp", "fontwork-curve-up"}, {"textCurveDown", "fontwork-curve-down"},
    {"textCascadeUp", "fontwork-cascade-up"},     {"textCascadeDown", "fontwork-cascade-down"}, {"textWave1", "fontwork-wave"}, {"textDoubleWave1", "fontwork-double-wave"},
    {"textInflate", "fontwork-inflate"},          {"textDeflate", "fontwork-deflate"},    {"textDeflateInflate", "fontwork-deflate-inflate"},
    {"textDeflateInflateDeflate", "fontwork-deflate-inflate-deflate"}, {"textFadeRight", "fontwork-fade-right"}, {"textFadeLeft", "fontwork-fade-left"},
    {"textFadeUp", "fontwork-fade-up"},           {"textFadeDown", "fontwork-fade-down"}, {"textSlantUp", "fontwork-slant-up"},   {"textSlantDown", "fontwork-slant-down"},
    {"textCanUp", "fontwork-can-up"},             {"textCanDown", "fontwork-can-down"},
};

bool plainName(const QString &s)
{
    if (s.isEmpty() || s.size() > 60) return false;
    for (const QChar c : s)
        if (!c.isLetterOrNumber() || c.unicode() > 127) return false;
    return true;
}

} // namespace

const NamePair *shapeNames(int *count)
{
    *count = int(std::size(kShapes));
    return kShapes;
}
const NamePair *fontworkNames(int *count)
{
    *count = int(std::size(kFontwork));
    return kFontwork;
}

QString odfTypeOfPreset(const QString &prst)
{
    for (const NamePair &p : kShapes)
        if (prst == QLatin1String(p.drawingml)) return QLatin1String(p.odf);
    return plainName(prst) ? QStringLiteral("ooxml-") + prst : QStringLiteral("rectangle");
}

QString presetOfOdfType(const QString &type)
{
    for (const NamePair &p : kShapes)
        if (type == QLatin1String(p.odf)) return QLatin1String(p.drawingml);
    if (type.startsWith(QLatin1String("ooxml-")) && plainName(type.mid(6))) return type.mid(6);
    return QStringLiteral("rect");
}

QString fontworkOfWarp(const QString &warp)
{
    for (const NamePair &p : kFontwork)
        if (warp == QLatin1String(p.drawingml)) return QLatin1String(p.odf);
    return QString();
}

QString warpOfFontwork(const QString &type)
{
    for (const NamePair &p : kFontwork)
        if (type == QLatin1String(p.odf)) return QLatin1String(p.drawingml);
    return QStringLiteral("textPlain");
}

QString imageFormatOf(const QByteArray &b) { return odt::sniffExtension(b); }

QSizeF naturalSizePoints(const QByteArray &bytes)
{
    const QString format = imageFormatOf(bytes);
    if (format != QLatin1String("png") && format != QLatin1String("jpeg") && format != QLatin1String("gif") && format != QLatin1String("bmp")) return QSizeF();
    QSize px;
    {
        QBuffer buffer;
        buffer.setData(bytes);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer);
        px = reader.size();
    }
    if (px.isEmpty() || px.width() > 200000 || px.height() > 200000) return QSizeF();
    double dpiX = 96, dpiY = 96;
    auto be32 = [&](qsizetype at) { return quint32(quint8(bytes[at])) << 24 | quint32(quint8(bytes[at + 1])) << 16 | quint32(quint8(bytes[at + 2])) << 8 | quint32(quint8(bytes[at + 3])); };
    if (format == QLatin1String("png")) {
        // (the chunk that says the pixels to the meter)
        qsizetype at = 8;
        while (at + 12 <= bytes.size()) {
            const quint32 length = be32(at);
            const QByteArray type = bytes.mid(at + 4, 4);
            if (type == "pHYs" && length >= 9 && at + 8 + 9 <= bytes.size()) {
                const quint32 x = be32(at + 8), y = be32(at + 12);
                if (bytes[at + 16] == 1 && x > 0 && y > 0) {
                    dpiX = x * 0.0254;
                    dpiY = y * 0.0254;
                }
                break;
            }
            if (type == "IDAT" || type == "IEND" || length > quint32(bytes.size())) break;
            at += qsizetype(length) + 12;
        }
    } else if (format == QLatin1String("jpeg") && bytes.size() > 20 && bytes.mid(6, 4) == "JFIF") {
        const quint8 units = quint8(bytes[13]);
        const double x = quint16(quint8(bytes[14]) << 8 | quint8(bytes[15])), y = quint16(quint8(bytes[16]) << 8 | quint8(bytes[17]));
        if (x > 0 && y > 0 && (units == 1 || units == 2)) {
            dpiX = units == 1 ? x : x * 2.54;
            dpiY = units == 1 ? y : y * 2.54;
        }
    }
    if (dpiX < 1 || dpiY < 1) dpiX = dpiY = 96;
    return QSizeF(px.width() * 72.0 / dpiX, px.height() * 72.0 / dpiY);
}

QString xmlText(const QString &text) { return QString::fromUtf8(odt::esc(text)); }

} // namespace jp::office::sheet::odsdraw
