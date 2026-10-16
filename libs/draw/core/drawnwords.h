#pragma once
// The words a picture shows that are in its pixels or outlines, kept as text so that a PDF made from it can carry them: a Macintosh picture's
// strings are drawn once into a PNG, and the file keeps where each stood (beside the pixels, in the PNG's own text), for a PDF to say again with
// no ink, so that they can be found and copied.

#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

class QPainter;

namespace jp {

// The key the PNG's text keeps them under.
extern const char kDrawnWordsKey[];

struct DrawnWord {
    QString text, family;
    QPointF at;             // where its baseline starts, as shares of the picture's width and height
    double size = 0;        // the letters' height (the font's pixel size), as a share of the picture's height
    bool bold = false, italic = false;
};

// What a file may hold: a few thousand strings of a thousand letters at most.
constexpr int kMaxDrawnWords = 2000, kMaxDrawnWordChars = 1000;

// The words as the text a PNG keeps them in, and back (a text that isn't the words of a picture gives none; more than a file may hold are left out).
QString drawnWordsToText(const QVector<DrawnWord> &words);
QVector<DrawnWord> drawnWordsFromText(const QString &text);

// Says `text` in the painter's coordinates, its baseline starting at `at` (`anchor`: 0 starts there, 0.5 is centered on it, 1 ends there), in the
// font of that family, size (in the painter's units), and style, with no ink: a PDF holds it as text and shows nothing. Nothing is said of a text
// no part of which falls in `only` (when it is given, as the painter's coordinates), or too small or large (on the page) to be a font.
void drawUnseenText(QPainter *p, const QString &family, double size, bool bold, bool italic, const QPointF &at, const QString &text, double anchor = 0,
                    const QRectF &only = QRectF());

} // namespace jp
