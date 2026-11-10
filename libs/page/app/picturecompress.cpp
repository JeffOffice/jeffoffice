#include "app/picturecompress.h"

#include "app/compresspictures.h"
#include "app/editor.h"
#include "app/plaintext.h"

#include <QCoreApplication>

namespace jp {

qint64 compressPictures(Editor *ed, double ppi, bool all, bool deleteCropped)
{
    QVector<PictureItem *> pics;
    if (all) ed->doc()->forEachItem([&](Item *it, int, const QString &) { if (auto *p = dynamic_cast<PictureItem *>(it)) pics << p; });
    else for (Item *it : ed->selectedItems()) if (auto *p = dynamic_cast<PictureItem *>(it)) pics << p;
    qint64 saved = 0;
    ed->change(QCoreApplication::translate("jp::PictureCompress", "Compress Pictures"), [&] {
        for (PictureItem *p : pics) {
            if (p->imageId.isEmpty() || !p->mediaKind.isEmpty()) continue;   // (a movie or sound has no picture of its own to make smaller)
            ImageData &data = ed->doc()->images[p->imageId];
            if (data.format == "svg" || data.format == "wmf" || data.format == "emf" || data.format == "pdf") continue;
            QImage img = data.image();
            if (img.isNull()) continue;
            QImage out = scaledForPpi(img, p->imgRect.width(), ppi);
            if (deleteCropped && out.width() > 0) {
                // Keep only the visible part of the picture.
                const double sx = out.width() / p->imgRect.width(), sy = out.height() / p->imgRect.height();
                const QRectF vis = QRectF(QPointF(0, 0), p->rect.size()).intersected(p->imgRect).translated(-p->imgRect.topLeft());
                const QRect px(int(vis.x() * sx), int(vis.y() * sy), int(vis.width() * sx), int(vis.height() * sy));
                if (px.isValid() && px != out.rect()) {
                    out = out.copy(px);
                    p->imgRect = QRectF(p->imgRect.topLeft() + vis.topLeft(), vis.size());
                }
            }
            QByteArray bytes, format;
            if (encodedIfSmaller(out, data.bytes.size(), &bytes, &format)) {
                saved += data.bytes.size() - bytes.size();
                p->imageId = ed->doc()->addImage(bytes, format.constData(), data.sourcePath);
            }
        }
    });
    return saved;
}

void compressPicturesDialog(QWidget *parent, Editor *ed, const QString &allText)
{
    CompressChoices choices;
    if (!askCompressPictures(parent, allText, &choices)) return;
    const qint64 saved = compressPictures(ed, choices.ppi, choices.all, choices.deleteCropped);
    jp::plainInformation(parent, QCoreApplication::translate("jp::PictureCompress", "Compress Pictures"), QCoreApplication::translate("jp::PictureCompress", "Saved %1 KB.").arg(saved / 1024));
}

} // namespace jp
