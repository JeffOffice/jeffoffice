#pragma once
// Reading a picture file for a person who is there to answer: a PDF with a password asks for it (the shared password box; a wrong one asks again, and Cancel stops).

#include <QByteArray>
#include <QSize>
#include <QString>

class QWidget;

namespace jp {

// As readPictureFile (core/document.h); false too when the person cancels the password. `asksNotToBeCopied` says the PDF's owner asks that it not be copied (it is read
// all the same: the limit is a request, and the caller shows a note).
bool readPictureFileAsking(QWidget *parent, const QString &path, int pdfPage, QByteArray *bytes, QString *format, QSize *pixels, QByteArray *fileHash = nullptr,
                           bool *asksNotToBeCopied = nullptr);

} // namespace jp
