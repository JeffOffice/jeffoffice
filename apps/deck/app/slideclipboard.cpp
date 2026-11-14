#include "app/slideclipboard.h"

#include "core/deck.h"
#include "core/reuse.h"
#include "core/sections.h"
#include "io/pptxexport.h"
#include "io/pptximport.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>

namespace jp::deck {

static const char *const kSlidesType = "application/x-jeffdeck-slides";

void copySlidesToClipboard(const Document &d, const QList<int> &slides)
{
    if (slides.isEmpty()) return;
    // A presentation of just these slides, each with its layout and slide master.
    auto pack = newPresentation();
    pack->setup.size = d.pageSize();
    for (int i : slides) insertSlideFrom(*pack, int(pack->pages.size()), d, i, true);
    pack->pages.removeAt(0);
    slideRemoved(*pack, 0);
    const QByteArray bytes = exportPresentation(*pack);
    if (bytes.isEmpty()) return;
    auto *mime = new QMimeData;
    mime->setData(QLatin1String(kSlidesType), bytes);
    QGuiApplication::clipboard()->setMimeData(mime);
}

bool slidesOnClipboard()
{
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    return mime && mime->hasFormat(QLatin1String(kSlidesType));
}

int pasteSlidesFromClipboard(Document &d, int at, bool keepFormatting)
{
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    if (!mime || !mime->hasFormat(QLatin1String(kSlidesType))) return 0;
    const auto from = importPresentation(mime->data(QLatin1String(kSlidesType)));
    if (!from) return 0;
    int made = 0;
    for (int k = 0; k < from->pages.size(); ++k)
        if (insertSlideFrom(d, at + made, *from, k, keepFormatting) >= 0) ++made;
    return made;
}

} // namespace jp::deck
