#pragma once
// Imports .pub files through libmspub.

#include "core/document.h"

#include <QStringList>

namespace jp {

struct PubImportReport {
    int pages = 0;
    int textBoxes = 0, pictures = 0, shapes = 0, tables = 0;
    int linkedChains = 0;
    int attachedEnds = 0;   // connector ends attached to objects
    int inlineObjects = 0;  // objects set in text
    QStringList fontsUsed;
    QStringList warnings;
};

bool isPublisherFile(const QByteArray &head);
std::unique_ptr<Document> importPublisher(const QByteArray &data, QString *error, PubImportReport *report = nullptr);
std::unique_ptr<Document> importPublisherFile(const QString &path, QString *error, PubImportReport *report = nullptr);
// A Publisher date and time format (Windows' letters) as Qt's QDateTime
// writes it, for a "datetime:" field.
QString pubDateFormat(QString f);
// Where a picture is drawn, in its frame's coordinates: the whole picture
// around the frame, trimmed by its crops (fractions of the picture, from the
// top, bottom, left, and right). A picture used as a fill (a page's
// background, a shape filled with one) is drawn 2.5 points past the shape on
// every side before the crops trim it.
QRectF pubPictureRect(const QSizeF &frame, double top, double bottom, double left, double right, bool pictureFill);

} // namespace jp
