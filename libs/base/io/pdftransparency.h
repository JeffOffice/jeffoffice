#pragma once
// PDF/A-1 and PDF/X-1a allow no transparency: no constant alpha below 1 (/ca, /CA), no soft mask (/SMask), no blend mode but Normal (/BM), and no transparency group (/Group with /S /Transparency).
// This reads a PDF Qt wrote and says whether it has any, and on which pages: in the page's own resources and in the forms and patterns it draws with (a few levels down), its content given as one
// stream or a list of them. A file it can't read well enough to say is refused as well (it says so): an export is never passed as clean for not having been checked.

#include <QByteArray>
#include <QString>
#include <QVector>
#include <functional>

namespace jp {

struct PdfTransparency {
    bool found = false;
    bool unreadable = false;   // the file could not be read (or is bigger than what is looked at): nothing can be said of it
    QVector<int> pages;   // the pages (from 1, in the order of the page tree) that draw with it; empty when something is in the file but no page was found to use it
    bool refused() const { return found || unreadable; }
};

PdfTransparency pdfTransparency(const QByteArray &pdf);

// For tests: a function that is given each PDF before it is looked at and returns the PDF that is looked at in its place (an empty function: none). Qt's writer leaves out most of what the check looks
// for in a PDF/A file, so a test puts a file that has some there to see what an export does with a refusal.
void setPdfTransparencyProbeForTest(std::function<QByteArray(const QByteArray &)> probe);

// What an export tells the person when it refuses (`standard`: "PDF/A" or "PDF/X-1a"). `name` gives what the export calls the PDF's page (the publication's page, a slide, a sheet's page: "page 3"), or an empty
// string where it doesn't know; the page is then "PDF page N".
QString pdfTransparencyRefusal(const PdfTransparency &t, const QString &standard, const std::function<QString(int pdfPage)> &name = nullptr);

} // namespace jp
