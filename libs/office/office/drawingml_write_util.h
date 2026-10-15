#pragma once
// What the files that write DrawingML share (drawingml_write.cpp writes the
// shapes, drawingml_write_text.cpp the text): lengths and colors as the
// standard writes them, and the links from a part to the pictures and
// addresses it names.

#include "office/drawingml_write.h"

#include "core/texteffects.h"

#include <QColor>
#include <QString>

namespace jp::office::dmlwrite {

// A length in points as EMU; positions and sizes are held to what a drawing
// can hold, as the reading does.
QString emu(double points);
// A position or size in EMU: the position anywhere in the page's reach, the size
// (a frame's extent) not below nothing.
qint64 position(double points);
qint64 extent(double points);
// A number rounded to a whole one: 0 for what is not a number, and none beyond a quadrillion.
qint64 rounded(double value);
// An angle in degrees as 60,000ths of a degree, from 0 up to a full turn.
qint64 angle(double degrees);
// A share (1 is all) as 100,000ths of a percent.
qint64 percent(double fraction);

// The text of an element or an attribute: what XML 1.0 cannot hold (control
// characters, lone surrogates, U+FFFE and U+FFFF) left out.
QString clean(const QString &text);

// The color a ColorRef stands for, with the scheme the context gives; invalid for none.
QColor resolve(const ColorRef &color, const DrawingWriteContext &ctx);
// <a:srgbClr val="RRGGBB"/>, with <a:alpha> inside it where the color is not whole:
// `transparency` is added to the color's own.
void writeColor(QXmlStreamWriter &w, const QColor &color, double transparency = 0);

// A run's a:effectLst: the list as the file had it when it still has it, else
// written from the effects themselves in the schema's order (an empty list for
// effects that say there are none).
void writeTextEffects(QXmlStreamWriter &w, const TextEffects &fx);

// The id of the relationship from the part being written to `target`, made
// the first time. An internal target is a part name. `type` is the last word
// of a relationship type of the standard ("image"), or a whole type URI.
QString relate(DrawingWriteContext &ctx, const QString &type, const QString &target, bool external = false);
// The relationship to a picture (the part is stored once for an image id);
// empty if the context has no package or the picture has no bytes.
QString relatePicture(DrawingWriteContext &ctx, const QString &imageId);
// The relationship to a picture's SVG (stored once for an image id, beside the
// PNG relatePicture stores for it); empty when the picture is not an SVG, or
// the context has no package.
QString relateSvg(DrawingWriteContext &ctx, const QString &imageId);
// The relationship to an address a hyperlink goes to.
QString relateAddress(DrawingWriteContext &ctx, const QString &address);
// The relationships of the movie or sound a picture plays: `link` for its
// a:videoFile or a:audioFile, and `copy` for the p14:media that says the file
// holds it (empty when the picture links to it outside). Both empty when it
// plays none, or its media can't be had.
struct MediaRelations { QString link, copy; };
MediaRelations relateMedia(DrawingWriteContext &ctx, const PictureItem &p);

} // namespace jp::office::dmlwrite
