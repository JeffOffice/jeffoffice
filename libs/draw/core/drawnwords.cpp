#include "core/drawnwords.h"

#include <QFont>
#include <QFontMetricsF>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPaintDevice>
#include <QPainter>

#include <algorithm>
#include <cmath>

namespace jp {

const char kDrawnWordsKey[] = "JeffOffice words";

namespace {

// A text is said at this many times its size and scaled down, so a size with a fraction keeps it (a font's pixel size is whole).
constexpr double kFine = 8;
// The sizes a text may be said at (points on the page, and pixels of the font): no font engine is asked for glyphs thousands of pixels tall.
constexpr double kLeast = 0.5, kMost = 250, kMostPixels = 8000;
// What a text of words may be.
constexpr qsizetype kMaxTextBytes = 1 << 20;

} // namespace

QString drawnWordsToText(const QVector<DrawnWord> &words)
{
    // No more than the reader takes (kMaxTextBytes), so that what is written is not dropped whole: the words are written to that many bytes.
    QJsonArray list;
    qint64 bytes = 2;   // the brackets
    for (const DrawnWord &w : words) {
        if (list.size() >= kMaxDrawnWords) break;
        if (w.text.isEmpty()) continue;
        QJsonObject o{{"t", w.text.left(kMaxDrawnWordChars)}, {"f", w.family}, {"x", w.at.x()}, {"y", w.at.y()}, {"s", w.size}};
        if (w.bold) o["b"] = true;
        if (w.italic) o["i"] = true;
        bytes += QJsonDocument(o).toJson(QJsonDocument::Compact).size() + 1;   // and a comma
        if (bytes > kMaxTextBytes) break;
        list.append(o);
    }
    return list.isEmpty() ? QString() : QString::fromUtf8(QJsonDocument(list).toJson(QJsonDocument::Compact));
}

QVector<DrawnWord> drawnWordsFromText(const QString &text)
{
    QVector<DrawnWord> out;
    if (text.isEmpty() || text.size() > kMaxTextBytes) return out;
    const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    if (!doc.isArray()) return out;
    for (const QJsonValue &v : doc.array()) {
        if (out.size() >= kMaxDrawnWords) break;
        const QJsonObject o = v.toObject();
        DrawnWord w;
        w.text = o.value("t").toString().left(kMaxDrawnWordChars);
        w.family = o.value("f").toString().left(200);
        w.at = QPointF(o.value("x").toDouble(), o.value("y").toDouble());
        w.size = o.value("s").toDouble();
        w.bold = o.value("b").toBool();
        w.italic = o.value("i").toBool();
        if (w.text.isEmpty() || !std::isfinite(w.at.x()) || !std::isfinite(w.at.y()) || !std::isfinite(w.size)) continue;
        out.append(w);
    }
    return out;
}

void drawUnseenText(QPainter *p, const QString &family, double size, bool bold, bool italic, const QPointF &at, const QString &text, double anchor, const QRectF &only)
{
    if (text.isEmpty() || !std::isfinite(size) || !std::isfinite(at.x()) || !std::isfinite(at.y()) || size <= 0) return;
    // Its size on the page in points, not in the painter's units (a drawing's units may be scaled down by eight, and a PDF's device has dots to the point).
    const double dotsToPoint = 72.0 / std::max(1, p->device()->logicalDpiX());
    const double onPage = size * std::sqrt(std::abs(p->transform().determinant())) * dotsToPoint;
    if (!(onPage >= kLeast && onPage <= kMost) || size * kFine > kMostPixels) return;
    QFont f(family);
    f.setPixelSize(std::max(1, int(std::lround(size * kFine))));
    f.setBold(bold);
    f.setItalic(italic);
    // Without ligatures: a glyph that stands for several letters has no letter to give a reader that copies the text.
    f.setFeature(QFont::Tag("liga"), 0);
    f.setFeature(QFont::Tag("clig"), 0);
    const QFontMetricsF fm(f);
    const double width = fm.horizontalAdvance(text) / kFine;
    const QPointF start(at.x() - anchor * width, at.y());
    if (!only.isNull() && !only.intersects(QRectF(start.x(), start.y() - size, width, size * 1.3))) return;
    p->save();
    p->setPen(QColor(0, 0, 0, 0));
    p->setFont(f);
    p->scale(1 / kFine, 1 / kFine);
    p->drawText(start * kFine, text);
    p->restore();
}

} // namespace jp
