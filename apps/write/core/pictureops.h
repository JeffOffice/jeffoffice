#pragma once
// Pictures in the text: Insert Picture (from a file's bytes, or an image from the clipboard), and what is done to a picture once it is there: its size, how the text goes round it
// (in line with the text, square, tight, above and below, behind or in front of the text), its place, and its alt text. A picture is a character of the story that holds the
// drawing element of its run as XML; every command writes that element again and gives the character the form the pages show, in one change of the character's format, so each is
// one step that undoes and the file saved says what the pages show. Only pictures that are one plain drawing (a picture in line, or one anchored) can be changed; any other
// (a picture of the older drawing format, a group) is kept as it is.

#include "core/writedoc.h"

#include <QImage>
#include <QPointF>
#include <QTextCursor>
#include <optional>

namespace jp::write {

enum class PictureWrap { Inline, Square, Tight, TopAndBottom, BehindText, InFrontOfText };

// The most bytes of a picture's file that are taken, the most pixels it may hold, and the longest side; and the least and most a picture is made, in points, on a side.
constexpr qint64 kMostPictureBytes = qint64(48) << 20;
constexpr qint64 kMostPicturePixels = 40000000;
constexpr int kMostPictureSide = 30000;
constexpr double kLeastPictureSize = 2, kMostPictureSize = 2000;
// How far a picture may be moved from where its place is measured, in points.
constexpr double kMostPictureOffset = 5000;
// The longest alt text kept.
constexpr int kMostAltTextLength = 2000;

enum class PictureResult {
    Ok,
    Unreadable,    // the bytes are no picture the program can read
    TooLarge,      // more bytes, pixels, or a longer side than the limits above
    External,      // an SVG that refers to anything outside itself
    Refused,       // not where a picture can go (a header, a footer, a note, a kept element), or the document holds the most pictures and objects it can show
};

// The most bytes of an SVG file, and the most the pictures of a document come to together.
constexpr qint64 kMostSvgBytes = qint64(4) << 20;

// The bytes of a picture file, read no further than a picture may be: only a regular file is read (a pipe, a device, and a directory are not), and one longer than kMostPictureBytes is
// TooLarge and read no further. Unreadable for a file that can't be read.
PictureResult readPictureFile(const QString &path, QByteArray *bytes);
// A size held to kLeastPictureSize and kMostPictureSize on both sides together, so that its ratio stays what it was.
void clampKeepingRatio(double *width, double *height);
// The drawing with the id of its wp:docPr changed.
QByteArray withDrawingId(const QByteArray &raw, int id);
// Alt text as it is kept: no character XML refuses, whole pairs of surrogates kept and lone ones dropped, at most kMostAltTextLength characters (a pair is not cut).
QString cleanAltText(const QString &text);

// What a picture of the text is.
struct PictureInfo {
    bool valid = false;        // the character is a picture
    bool editable = false;     // the program can change it (see above)
    bool floating = false;
    PictureWrap wrap = PictureWrap::Inline;
    double width = 0, height = 0;   // points
    double left = 0, top = 0;       // a picture set at a place: from where it is measured (points)
    bool placeIsOffset = false;     // `left` and `top` are offsets (else the picture is aligned, and they are 0)
    QString alt;
    bool decorative = false;   // marked as adding nothing a screen reader should read
};

// The picture at a place of the story (the character just after `position`); `valid` is false for any other character.
PictureInfo pictureAt(const WriteDoc &doc, const QTextDocument *story, int position);
// The character format of the character just after `position`.
QTextCharFormat charFormatAt(const QTextDocument *story, int position);
// Whether the character just after `position` is a picture (a quick test for the pages).
bool isPictureAt(const QTextDocument *story, int position);

// A picture from the bytes of a picture file (PNG, JPEG, GIF, BMP, and TIFF are kept as they are; any other form the program can read is kept as PNG) at the cursor, in place of a
// selection, as one step that undoes. It is made as large as the picture asks (96 pixels to the inch) but no wider than the column or taller than the page's text. `name` is the file's
// name (the picture's name in the file); `alt` its alt text.
PictureResult insertPicture(WriteDoc &doc, QTextCursor &c, const QByteArray &bytes, const QString &name = QString(), PictureWrap wrap = PictureWrap::Inline, const QString &alt = QString());
// The same for an image (from the clipboard), kept as PNG.
PictureResult insertPictureImage(WriteDoc &doc, QTextCursor &c, const QImage &image, PictureWrap wrap = PictureWrap::Inline);

// Changes to the picture at `position` of the body text; each is one step, and false (nothing done) when the character is no picture that can be changed or the change would
// make no difference. Sizes are in points and held to the least and most above.
bool setPictureSize(WriteDoc &doc, int position, double width, double height);
// The size and the place (the picture's top left corner on the page) in one step; with no place, only the size; with no size (0), only the place.
bool setPictureBox(WriteDoc &doc, int position, double width, double height, const std::optional<QPointF> &pageTopLeft);
bool setPictureWrap(WriteDoc &doc, int position, PictureWrap wrap);
bool setPictureAltText(WriteDoc &doc, int position, const QString &alt);
// Mark as Decorative: the picture is marked as adding nothing to the page's meaning, and its description is taken off; off, the mark goes.
bool setPictureDecorative(WriteDoc &doc, int position, bool decorative);
// Sets a picture at a place on the page, measured from its top left corner (points): it becomes one that is set at a place (square, if it was in line).
bool setPicturePlace(WriteDoc &doc, int position, double pageX, double pageY);
// Moves an inline picture to another place in the text (`to`, a position outside the picture): one step.
bool movePictureInText(WriteDoc &doc, int position, int to);

} // namespace jp::write
