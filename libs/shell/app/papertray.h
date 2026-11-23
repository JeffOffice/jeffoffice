#pragma once
// The printer trays (paper sources) a document can ask for, the same in every app: the standard choices with their plain names, the trays a printer has where the system can list
// them, the box that lets a person choose one, and the one place a print job passes a tray to the printer (papertray.cpp).
// A tray in a file is a number the printer defines; these are the standard meanings of the first ones (the numbers the printing system gave its paper-source choices, which the
// file formats carry as they are). A number that is not here is a printer's own tray: it is kept and shown as "Tray N".

#include <QList>
#include <QPrinter>
#include <QString>
#include <functional>
#include <optional>

class QComboBox;

namespace jp {

struct StandardTray {
    int code;
    QString name;   // as a person reads it
};
// The standard trays, in the order a list shows them.
QList<StandardTray> standardTrays();
// The plain name of a tray: the standard one's, else "Tray N".
QString trayName(int code);
// The printing layer's choice for a standard tray (none for a number that is not one).
std::optional<QPrinter::PaperSource> printerSourceOf(int code);

// What the system's printing can do with a tray: choose one for each page of a job, or nothing (the person chooses in the printer's own settings; the toolkit's paper source is
// only put into effect on Windows).
enum class TrayReach { PerPage, None };
TrayReach trayReach();
// The trays (standard numbers) a printer has, when the system can list them; none when it can't. `printer` empty is the default one.
std::optional<QList<int>> printerTrays(const QString &printer);
// The sentence for a dialog on a system whose printing can't choose a tray; empty where it can.
QString trayHint();

// Fills the box: "Default tray" (the printer picks), the standard trays, and the document's own `stored` choice, always, even when this printer does not have it (named so);
// where the printer's trays are known, the standard ones it lacks are turned off. `stored` is the current choice.
void fillTrayBox(QComboBox *box, std::optional<int> stored, const QString &printer);
// What the box has chosen (none: the printer picks).
std::optional<int> trayChosen(const QComboBox *box);

// Sets the tray a page of a job is printed from (none: the printer picks). A job calls this before each page it starts. A printer engine that writes a file keeps no tray, so a test
// listens here for what was asked, with setTrayRecorder, and not to the printer.
void setPrinterTray(QPrinter *printer, std::optional<int> tray);
void setTrayRecorder(const std::function<void(std::optional<int>)> &recorder);
// Stand-ins for the system, for a test (the program never uses them).
void setTrayReachStandIn(const std::function<TrayReach()> &standIn);
void setPrinterTraysStandIn(const std::function<std::optional<QList<int>>(const QString &printer)> &standIn);

} // namespace jp
