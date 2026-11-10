#include "core/pageaccess.h"

#include "core/document.h"

#include <QCoreApplication>

namespace jp {

QVector<PageAccessIssue> checkPageAccessibility(const Document &doc, bool slides, const std::function<QString(int)> &titleOf, int most)
{
    QVector<PageAccessIssue> out;
    for (int p = 0; p < doc.pages.size() && out.size() < most; ++p) {
        if (titleOf && titleOf(p).trimmed().isEmpty())
            out.push_back({PageAccessIssue::NoPageTitle, PageAccessIssue::Warning, p, QString(),
                           QCoreApplication::translate("jp::PageAccess", "Slide %1 has no title. People who use a screen reader find a slide by its title.").arg(p + 1)});
        const QString picture = slides ? QCoreApplication::translate("jp::PageAccess", "A picture on slide %1").arg(p + 1) : QCoreApplication::translate("jp::PageAccess", "A picture on page %1").arg(p + 1);
        const QString shape = slides ? QCoreApplication::translate("jp::PageAccess", "A shape on slide %1").arg(p + 1) : QCoreApplication::translate("jp::PageAccess", "A shape on page %1").arg(p + 1);
        walkItems(doc.pages[p]->items, [&](const ItemPtr &it) {
            if (out.size() >= most || it->decorative || !it->altText.trimmed().isEmpty()) return;
            if (it->type() == ItemType::Picture)
                out.push_back({PageAccessIssue::MissingAltText, PageAccessIssue::Error, p, it->id,
                               QCoreApplication::translate("jp::PageAccess", "%1 has no alt text. Alt text tells a person who cannot see it what the picture shows.").arg(picture)});
            else if (it->type() == ItemType::Shape && !it->hasText())
                out.push_back({PageAccessIssue::MissingAltText, PageAccessIssue::Error, p, it->id,
                               QCoreApplication::translate("jp::PageAccess", "%1 has no alt text. Give it a description, or mark it as decoration.").arg(shape)});
        });
    }
    return out;
}

} // namespace jp
