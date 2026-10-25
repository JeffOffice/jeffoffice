#pragma once
// The legacy drawing that holds the pictures of a sheet's headers and footers, written: one shape for each picture (its id the place, its style the size) naming the image by the
// relationship the drawing's own relationships part gives it. Reading is Workbook::readHeaderPictures.

#include "sheet/pagesetup.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::office::sheet {

// The drawing for the pictures; the image of the n-th picture is the relationship "rId<n+1>".
QByteArray headerPicturesVml(const QVector<HeaderPicture> &pictures);
// The content type of the image part for the extension ("png" -> "image/png"); empty for one the file format does not take here.
QString headerPictureContentType(const QString &extension);

} // namespace jp::office::sheet
