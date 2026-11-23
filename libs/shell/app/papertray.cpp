#include "app/papertray.h"
#include "app/plaintext.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QPrinterInfo>
#include <QStandardItemModel>

namespace jp {

namespace {
std::function<void(std::optional<int>)> g_recorder;
std::function<TrayReach()> g_reachStandIn;
std::function<std::optional<QList<int>>(const QString &)> g_traysStandIn;

struct TrayRow {
    int code;
    QPrinter::PaperSource source;
};
const TrayRow kRows[] = {
    {1, QPrinter::Upper}, {2, QPrinter::Lower}, {3, QPrinter::Middle}, {4, QPrinter::Manual}, {5, QPrinter::Envelope}, {6, QPrinter::EnvelopeManual}, {7, QPrinter::Auto},
    {8, QPrinter::Tractor}, {9, QPrinter::SmallFormat}, {10, QPrinter::LargeFormat}, {11, QPrinter::LargeCapacity}, {14, QPrinter::Cassette}, {15, QPrinter::FormSource},
};
}

QList<StandardTray> standardTrays()
{
    // (the names are looked up with translate() here, each marked where it is written for the extractor)
    static const struct {
        int code;
        const char *name;
    } names[] = {{1, QT_TRANSLATE_NOOP("PaperTray", "Upper tray")}, {2, QT_TRANSLATE_NOOP("PaperTray", "Lower tray")}, {3, QT_TRANSLATE_NOOP("PaperTray", "Middle tray")},
                 {4, QT_TRANSLATE_NOOP("PaperTray", "Manual feed")}, {5, QT_TRANSLATE_NOOP("PaperTray", "Envelope feeder")}, {6, QT_TRANSLATE_NOOP("PaperTray", "Envelope, manual feed")},
                 {7, QT_TRANSLATE_NOOP("PaperTray", "Automatic selection")}, {8, QT_TRANSLATE_NOOP("PaperTray", "Tractor feed")}, {9, QT_TRANSLATE_NOOP("PaperTray", "Small-format tray")},
                 {10, QT_TRANSLATE_NOOP("PaperTray", "Large-format tray")}, {11, QT_TRANSLATE_NOOP("PaperTray", "Large-capacity tray")}, {14, QT_TRANSLATE_NOOP("PaperTray", "Cassette")},
                 {15, QT_TRANSLATE_NOOP("PaperTray", "Tray chosen by the paper size")}};
    QList<StandardTray> out;
    for (const auto &n : names) out.push_back({n.code, QCoreApplication::translate("PaperTray", n.name)});
    return out;
}

QString trayName(int code)
{
    for (const StandardTray &t : standardTrays())
        if (t.code == code) return t.name;
    return QCoreApplication::translate("PaperTray", "Tray %1").arg(code);
}

std::optional<QPrinter::PaperSource> printerSourceOf(int code)
{
    for (const TrayRow &r : kRows)
        if (r.code == code) return r.source;
    return std::nullopt;
}

void setTrayReachStandIn(const std::function<TrayReach()> &standIn) { g_reachStandIn = standIn; }
void setPrinterTraysStandIn(const std::function<std::optional<QList<int>>(const QString &)> &standIn) { g_traysStandIn = standIn; }
void setTrayRecorder(const std::function<void(std::optional<int>)> &recorder) { g_recorder = recorder; }

TrayReach trayReach()
{
    if (g_reachStandIn) return g_reachStandIn();
#ifdef Q_OS_WIN
    return TrayReach::PerPage;
#else
    return TrayReach::None;
#endif
}

std::optional<QList<int>> printerTrays(const QString &printer)
{
    if (g_traysStandIn) return g_traysStandIn(printer);
#ifdef Q_OS_WIN
    const QPrinterInfo info = printer.isEmpty() ? QPrinterInfo::defaultPrinter() : QPrinterInfo::printerInfo(printer);
    if (info.isNull()) return std::nullopt;
    const QPrinter probe(info);
    QList<int> codes;
    for (const QPrinter::PaperSource s : probe.supportedPaperSources())
        for (const TrayRow &r : kRows)
            if (r.source == s && !codes.contains(r.code)) codes << r.code;
    return codes;
#else
    Q_UNUSED(printer);
    return std::nullopt;
#endif
}

QString trayHint()
{
    if (trayReach() == TrayReach::PerPage) return QString();
    return QCoreApplication::translate("PaperTray", "The trays are saved in the document, but this system can't send them to the printer. Choose the tray in the printer's own settings when you print.");
}

void fillTrayBox(QComboBox *box, std::optional<int> stored, const QString &printer)
{
    box->clear();
    const std::optional<QList<int>> has = printerTrays(printer);
    box->addItem(QCoreApplication::translate("PaperTray", "Default tray"), QVariant());
    box->setItemData(0, plainToolTip(QCoreApplication::translate("PaperTray", "The printer picks the tray.")), Qt::ToolTipRole);
    auto *model = qobject_cast<QStandardItemModel *>(box->model());
    const bool storedStandard = stored && printerSourceOf(*stored);
    for (const StandardTray &t : standardTrays()) {
        const bool missing = has && !has->contains(t.code);
        const bool isStored = stored && *stored == t.code;
        box->addItem(missing && isStored ? QCoreApplication::translate("PaperTray", "%1 (not on this printer)").arg(t.name) : t.name, t.code);
        if (missing && !isStored && model) {
            model->item(box->count() - 1)->setEnabled(false);
            box->setItemData(box->count() - 1, plainToolTip(QCoreApplication::translate("PaperTray", "This printer doesn't have this tray.")), Qt::ToolTipRole);
        }
    }
    // (the document's own choice is always in the list: one this printer, or a standard list, does not have is named as it is)
    if (stored && !storedStandard) box->addItem(QCoreApplication::translate("PaperTray", "%1 (not on this printer)").arg(trayName(*stored)), *stored);
    box->setCurrentIndex(stored ? std::max(0, int(box->findData(*stored))) : 0);
}

std::optional<int> trayChosen(const QComboBox *box)
{
    const QVariant v = box->currentData();
    return v.isValid() && !v.isNull() ? std::optional<int>(v.toInt()) : std::nullopt;
}

void setPrinterTray(QPrinter *printer, std::optional<int> tray)
{
    if (g_recorder) g_recorder(tray);
    // (a number that is not a standard tray, or none, asks the printer to pick)
    const std::optional<QPrinter::PaperSource> source = tray ? printerSourceOf(*tray) : std::nullopt;
    printer->setPaperSource(source ? *source : QPrinter::Auto);
}

} // namespace jp
