#include "app/printsides.h"
#include "app/plaintext.h"

#include <QComboBox>
#include <algorithm>
#include <QCoreApplication>
#include <QPrinterInfo>
#include <QStandardItemModel>

namespace jp {

namespace {
std::function<QList<QPrinter::DuplexMode>(const QString &)> g_duplexStandIn;
const char *kWanted = "sidesWanted";
const char *kSettling = "sidesSettling";

}

void setDuplexSupportStandIn(const std::function<QList<QPrinter::DuplexMode>(const QString &)> &standIn) { g_duplexStandIn = standIn; }

QList<QPrinter::DuplexMode> supportedDuplexModes(const QString &printer)
{
    if (g_duplexStandIn) return g_duplexStandIn(printer);
    const QPrinterInfo info = printer.isEmpty() ? QPrinterInfo::defaultPrinter() : QPrinterInfo::printerInfo(printer);
    return info.isNull() ? QList<QPrinter::DuplexMode>() : info.supportedDuplexModes();
}

void fillSidesBox(QComboBox *box, bool withByHand, int wanted)
{
    box->clear();
    box->addItem(QCoreApplication::translate("PrintSides", "Print on one side"), int(SidesChoice::OneSide));
    box->addItem(QCoreApplication::translate("PrintSides", "Print on both sides, flip on long edge"), int(SidesChoice::LongEdge));
    box->addItem(QCoreApplication::translate("PrintSides", "Print on both sides, flip on short edge"), int(SidesChoice::ShortEdge));
    if (withByHand) {
        box->addItem(QCoreApplication::translate("PrintSides", "Print on both sides by hand"), int(SidesChoice::ByHand));
        box->setItemData(3, plainToolTip(QCoreApplication::translate("PrintSides", "For a printer that can't print on both sides: the odd pages first, then a message to turn the stack over, then the even pages.")), Qt::ToolTipRole);
    }
    box->setAccessibleName(QCoreApplication::translate("PrintSides", "Sides"));
    const int at = box->findData(wanted);
    box->setProperty(kWanted, at >= 0 ? wanted : int(SidesChoice::OneSide));
    box->setCurrentIndex(std::max(0, at));
    // (what the person picks is kept; a change made to match the printer is not)
    QObject::connect(box, &QComboBox::currentIndexChanged, box, [box] {
        if (!box->property(kSettling).toBool()) box->setProperty(kWanted, box->currentData());
    });
}

void updateSidesBox(QComboBox *box, const QString &printer, bool toFile)
{
    const QList<QPrinter::DuplexMode> modes = supportedDuplexModes(printer);
    auto *model = qobject_cast<QStandardItemModel *>(box->model());
    const bool automatic = modes.contains(QPrinter::DuplexLongSide) || modes.contains(QPrinter::DuplexShortSide) || modes.contains(QPrinter::DuplexAuto);
    box->setProperty(kSettling, true);
    for (const SidesChoice side : {SidesChoice::LongEdge, SidesChoice::ShortEdge}) {
        const int at = box->findData(int(side));
        if (at < 0 || !model) continue;
        model->item(at)->setEnabled(automatic);
        box->setItemData(at, automatic ? QString() : plainToolTip(QCoreApplication::translate("PrintSides", "This printer can't print on both sides.")), Qt::ToolTipRole);
    }
    const int wanted = box->findData(box->property(kWanted).toInt());
    const bool allowed = wanted >= 0 && (!model || model->item(wanted)->isEnabled());
    box->setCurrentIndex(allowed ? wanted : 0);
    box->setEnabled(!toFile);
    box->setToolTip(toFile ? plainToolTip(QCoreApplication::translate("PrintSides", "A PDF file has no sides.")) : QString());
    box->setProperty(kSettling, false);
}

int sidesWanted(const QComboBox *box) { return box->property(kWanted).toInt(); }

QPrinter::DuplexMode sidesDuplex(const QComboBox *box)
{
    if (!box->isEnabled()) return QPrinter::DuplexNone;
    switch (SidesChoice(box->currentData().toInt())) {
    case SidesChoice::LongEdge: return QPrinter::DuplexLongSide;
    case SidesChoice::ShortEdge: return QPrinter::DuplexShortSide;
    default: return QPrinter::DuplexNone;
    }
}

bool sidesByHand(const QComboBox *box) { return box->isEnabled() && SidesChoice(box->currentData().toInt()) == SidesChoice::ByHand; }

} // namespace jp
