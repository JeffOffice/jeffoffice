#include "app/compresspictures.h"

#include "app/plaintext.h"

#include <algorithm>

#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QImageReader>

namespace jp {

bool askCompressPictures(QWidget *parent, const QString &allText, CompressChoices *choices)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("compressPicturesDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::CompressPictures", "Compress Pictures"));
    auto *form = new QFormLayout(&dlg);
    auto *target = new QComboBox(&dlg);
    target->setObjectName(QStringLiteral("compressTarget"));
    target->addItems({QCoreApplication::translate("jp::CompressPictures", "Commercial printing (300 ppi)"), QCoreApplication::translate("jp::CompressPictures", "Desktop printing (220 ppi)"),
                      QCoreApplication::translate("jp::CompressPictures", "Web (96 ppi)")});
    auto *all = new QCheckBox(allText, &dlg);
    all->setObjectName(QStringLiteral("compressAll"));
    all->setChecked(true);
    auto *crop = new QCheckBox(QCoreApplication::translate("jp::CompressPictures", "Delete cropped areas of pictures"), &dlg);
    crop->setObjectName(QStringLiteral("compressCrop"));
    crop->setChecked(true);
    form->addRow(QCoreApplication::translate("jp::CompressPictures", "Target output:"), target);
    form->addRow(all);
    form->addRow(crop);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;
    choices->ppi = target->currentIndex() == 0 ? 300 : target->currentIndex() == 1 ? 220 : 96;
    choices->all = all->isChecked();
    choices->deleteCropped = crop->isChecked();
    return true;
}

QImage scaledForPpi(const QImage &image, double shownWidth, double ppi)
{
    const double wantW = shownWidth / 72.0 * ppi;
    if (wantW >= 1 && image.width() > wantW * 1.05) return image.scaledToWidth(int(wantW), Qt::SmoothTransformation);
    return image;
}

bool encodedIfSmaller(const QImage &image, qint64 originalBytes, QByteArray *bytes, QByteArray *format)
{
    QByteArray made;
    QBuffer b(&made);
    b.open(QIODevice::WriteOnly);
    const bool alpha = image.hasAlphaChannel();
    image.save(&b, alpha ? "PNG" : "JPG", alpha ? -1 : 88);
    if (made.size() >= originalBytes) return false;
    *bytes = made;
    *format = alpha ? "png" : "jpg";
    return true;
}

bool compressedPictureFile(const QByteArray &bytes, double shownWidth, double ppi, const QRectF &keep, QByteArray *out, QByteArray *format)
{
    QByteArray copy = bytes;
    QBuffer in(&copy);
    in.open(QIODevice::ReadOnly);
    QImageReader reader(&in);
    const QByteArray kind = reader.format().toLower();
    if (kind.isEmpty() || kind.startsWith("svg") || reader.imageCount() > 1) return false;
    const QImage image = reader.read();
    if (image.isNull() || shownWidth <= 0) return false;
    QImage made = scaledForPpi(image, shownWidth, ppi);
    const QRectF whole(0, 0, 1, 1);
    const QRectF part = keep.intersected(whole);
    if (part.width() <= 0 || part.height() <= 0) return false;
    if (part != whole) {
        const QRect px(int(part.x() * made.width()), int(part.y() * made.height()), std::max(1, int(part.width() * made.width() + 0.5)), std::max(1, int(part.height() * made.height() + 0.5)));
        made = made.copy(px.intersected(made.rect()));
    }
    return !made.isNull() && encodedIfSmaller(made, bytes.size(), out, format);
}

void tellCompressSaved(QWidget *parent, qint64 saved)
{
    jp::plainInformation(parent, QCoreApplication::translate("jp::PictureCompress", "Compress Pictures"), QCoreApplication::translate("jp::PictureCompress", "Saved %1 KB.").arg(saved / 1024));
}

} // namespace jp
