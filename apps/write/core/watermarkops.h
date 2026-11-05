#pragma once
// Design > Watermark: words (Confidential, Draft...) set large and light behind the text of every page. A watermark lives in the headers, as a shape with its text on a path in the first
// paragraph of each header the document shows, so that it is drawn behind the page's text and a file keeps it as such. Put in, it replaces the one there; taken out, nothing else of the
// header goes. Each header (a section's, first page's, and even pages') is edited as a step of its own.

#include "core/writedoc.h"

#include <QColor>
#include <QString>
#include <QStringList>
#include <optional>

class QTextDocument;

namespace jp::write {

struct WatermarkSetup {
    QString text;
    QString font = QStringLiteral("Arial");
    double size = 0;                       // points; 0 fits the words to the width of the text
    QColor color{0xC0, 0xC0, 0xC0};
    bool semitransparent = true;
    bool diagonal = true;                  // else across the page
    bool operator==(const WatermarkSetup &o) const { return text == o.text && font == o.font && size == o.size && color == o.color && semitransparent == o.semitransparent && diagonal == o.diagonal; }
};
constexpr int kMostWatermarkText = 255;

// The words of the built-in watermarks, in the order of the menu.
QStringList builtInWatermarks();

// Puts the watermark in every header of the document (a header is made where a section has none). How many headers were changed; 0 for empty words or a header the program can't edit.
int insertWatermark(WriteDoc &doc, const WatermarkSetup &setup);
// Takes the watermark out of every header; how many headers it was taken from.
int removeWatermark(WriteDoc &doc);
// The watermark the document's headers show (the first found), or none.
std::optional<WatermarkSetup> watermarkOf(WriteDoc &doc);

// The watermark a header made now should begin with (the document's, if its headers show one); none while a watermark is being put in, which makes the headers itself.
std::optional<WatermarkSetup> watermarkForNewHeader(WriteDoc &doc);
// How many watermarks a story (a header) holds; 0 for none or no story.
int watermarksIn(QTextDocument *story);
// Puts a watermark at the start of a header's story, as part of the edit going on (it makes no step of its own: a header made with it has it from the start).
void putWatermark(QTextDocument *story, const WatermarkSetup &setup, int number);

// The XML of the shape (the child of a run) for a setup; exposed for the tests.
QByteArray watermarkXml(const WatermarkSetup &setup, int number = 1);

} // namespace jp::write
