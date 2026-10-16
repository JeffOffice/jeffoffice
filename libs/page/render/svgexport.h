#pragma once
// Pages as SVG drawings: everything as vectors (pictures embedded), and
// letters as their outlines, so a page looks the same in any program
// whether or not it has the fonts.

#include "render/renderer.h"

#include <QByteArray>
#include <QString>

namespace jp {

// With `withText`, each text is also in the drawing as a text element with no ink (its letters are outlines): what draws the drawing into a PDF can
// then say the words as text, that can be found and copied.
QByteArray pageSvg(const PaintContext &ctx, int pageIndex, const QString &title, bool withText = false);
bool writePageSvg(const PaintContext &ctx, int pageIndex, const QString &path, const QString &title, QString *error);

} // namespace jp
