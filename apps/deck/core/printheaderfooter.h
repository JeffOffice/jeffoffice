#pragma once
// The header and footer of the printed notes pages and handouts (Print > Header & Footer): the date, a header text, a footer text, and the page number. They are the presentation's,
// kept in its appData "printHeaderFooter" and saved in the file the way .pptx has them: the hf element of the notes master and of the handout master, and the text in the master's
// header, footer, and date places.

#include "core/document.h"

#include <QString>

namespace jp::deck {

enum class PrintedPages { Notes, Handouts };

struct PrintHeaderFooter {
    bool date = false;
    QString dateText;   // a date typed in; empty with `date` on: today's
    bool header = false;
    QString headerText;
    bool footer = false;
    QString footerText;
    bool pageNumber = true;
    bool operator==(const PrintHeaderFooter &o) const
    {
        return date == o.date && dateText == o.dateText && header == o.header && headerText == o.headerText && footer == o.footer && footerText == o.footerText && pageNumber == o.pageNumber;
    }
};

// What the presentation has for its notes pages (or handouts): the choice made, or else what printing always showed: a handout's date, the presentation's title as its header, and the page
// number; a notes page's number alone.
PrintHeaderFooter printHeaderFooter(const Document &d, PrintedPages pages);
// Whether the presentation has a choice of its own for them.
bool hasPrintHeaderFooter(const Document &d, PrintedPages pages);
// Keeps the choice (no step of its own: the caller records one).
void setPrintHeaderFooter(Document &d, PrintedPages pages, const PrintHeaderFooter &hf);

} // namespace jp::deck
