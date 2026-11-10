#include "app/printpage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QPrinterInfo>
#include <QSpinBox>
#include <algorithm>

namespace jp {

namespace {
constexpr int kIsFileRole = Qt::UserRole + 1;   // set on the entry that makes a PDF file
}

QVector<int> sheetOrder(const QVector<int> &pages, int copies, bool collate)
{
    copies = std::max(1, copies);
    QVector<int> out;
    if (collate) {
        for (int c = 0; c < copies; ++c) out += pages;
    } else {
        for (int page : pages)
            for (int c = 0; c < copies; ++c) out << page;
    }
    return out;
}

QVector<int> inPrintOrder(QVector<int> pages, bool reverse)
{
    if (reverse) std::reverse(pages.begin(), pages.end());
    return pages;
}

void fillPrinterBox(QComboBox *box, const QStringList &printers, const QString &current)
{
    box->clear();
    for (const QString &name : printers) box->addItem(name, name);
    box->addItem(QCoreApplication::translate("Print", "Save as PDF"), QString());
    box->setItemData(box->count() - 1, true, kIsFileRole);
    const int at = printers.indexOf(current);
    if (at >= 0) box->setCurrentIndex(at);
    else if (printers.isEmpty()) box->setCurrentIndex(0);
}

PrinterChoice printerChoice(const QComboBox *box)
{
    PrinterChoice choice;
    if (box->currentIndex() < 0) return choice;
    choice.toFile = box->currentData(kIsFileRole).toBool();
    if (!choice.toFile) choice.printer = box->currentData(Qt::UserRole).toString();
    return choice;
}

QComboBox *makePrinterBox(QWidget *parent)
{
    auto *printer = new QComboBox(parent);
    printer->setObjectName(QStringLiteral("printPrinter"));
    printer->setAccessibleName(QCoreApplication::translate("Print", "Printer"));
    QStringList names;
    for (const QPrinterInfo &info : QPrinterInfo::availablePrinters()) names << info.printerName();
    fillPrinterBox(printer, names, QPrinterInfo::defaultPrinter().isNull() ? QString() : QPrinterInfo::defaultPrinter().printerName());
    return printer;
}

QSpinBox *makeCopiesBox(QWidget *parent)
{
    auto *copies = new QSpinBox(parent);
    copies->setObjectName(QStringLiteral("printCopies"));
    copies->setRange(1, 999);
    copies->setAccessibleName(QCoreApplication::translate("Print", "Copies"));
    return copies;
}

QCheckBox *makeCollateBox(QWidget *parent, const QString &tip)
{
    auto *collate = new QCheckBox(QCoreApplication::translate("Print", "Collated"), parent);
    collate->setObjectName(QStringLiteral("printCollate"));
    collate->setChecked(true);
    collate->setToolTip(tip);
    return collate;
}

QCheckBox *makeReverseBox(QWidget *parent)
{
    auto *reverse = new QCheckBox(QCoreApplication::translate("Print", "Last page first"), parent);
    reverse->setObjectName(QStringLiteral("printReverse"));
    reverse->setToolTip(QCoreApplication::translate("Print", "Print the pages in reverse order, so the stack comes out in order."));
    return reverse;
}

QCheckBox *makeGrayscaleBox(QWidget *parent)
{
    auto *gray = new QCheckBox(QCoreApplication::translate("Print", "Grayscale"), parent);
    gray->setObjectName(QStringLiteral("printGray"));
    gray->setToolTip(QCoreApplication::translate("Print", "Print in shades of gray, whatever the colors are."));
    return gray;
}

QComboBox *makeOrientationBox(QWidget *parent, const QString &asSet)
{
    auto *orientation = new QComboBox(parent);
    orientation->setObjectName(QStringLiteral("printOrientation"));
    orientation->setAccessibleName(QCoreApplication::translate("Print", "Which way the paper turns"));
    orientation->addItem(asSet, int(PrintTurn::AsSet));
    orientation->addItem(QCoreApplication::translate("Print", "Tall"), int(PrintTurn::Tall));
    orientation->addItem(QCoreApplication::translate("Print", "Wide"), int(PrintTurn::Wide));
    return orientation;
}

QSizeF turnedSheet(const QSizeF &size, PrintTurn turn)
{
    if (turn == PrintTurn::Tall && size.width() > size.height()) return size.transposed();
    if (turn == PrintTurn::Wide && size.width() < size.height()) return size.transposed();
    return size;
}

QComboBox *makeZoomBox(QWidget *parent)
{
    auto *zoom = new QComboBox(parent);
    zoom->setObjectName(QStringLiteral("printZoom"));
    zoom->setAccessibleName(QCoreApplication::translate("Print", "Zoom"));
    zoom->addItem(QCoreApplication::translate("Print", "Fit"), 100);
    zoom->addItem(QCoreApplication::translate("Print", "150%"), 150);
    zoom->addItem(QCoreApplication::translate("Print", "200%"), 200);
    zoom->addItem(QCoreApplication::translate("Print", "300%"), 300);
    return zoom;
}

double zoomFactor(const QComboBox *box) { return box->currentData().toInt() / 100.0; }

void PreviewLabel::resizeEvent(QResizeEvent *e)
{
    QLabel::resizeEvent(e);
    if (resized) resized();
}

} // namespace jp
