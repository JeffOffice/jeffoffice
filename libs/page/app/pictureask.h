#pragma once
// Reading a picture file for a person who is there to answer: a PDF with a password asks for it (the shared password box; a wrong one asks again, and Cancel stops).

#include <QByteArray>
#include <QSize>
#include <QString>

class QWidget;

namespace jp {

// As readPictureFile (core/document.h); false too when the person cancels the password.
bool readPictureFileAsking(QWidget *parent, const QString &path, int pdfPage, QByteArray *bytes, QString *format, QSize *pixels, QByteArray *fileHash = nullptr);

} // namespace jp
