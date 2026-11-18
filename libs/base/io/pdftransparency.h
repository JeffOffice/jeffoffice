#pragma once
// PDF/A-1 allows no transparency: no constant alpha below 1 (/ca, /CA) and no soft mask (/SMask). This reads a PDF Qt wrote and says whether it has any, and on which pages.

#include <QByteArray>
#include <QVector>

namespace jp {

struct PdfTransparency {
    bool found = false;
    QVector<int> pages;   // the pages (from 1) that draw with it; empty when something is in the file but no page was found to use it
};

PdfTransparency pdfTransparency(const QByteArray &pdf);

} // namespace jp
