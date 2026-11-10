#include "app/compresspictures.h"

#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>

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

} // namespace jp
