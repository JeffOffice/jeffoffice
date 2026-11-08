#pragma once
// The Sides choice every Print page has: one side, both sides flipped on the long edge or on the short edge, and, where an app has it, both sides by hand
// (printsides.cpp).

#include <QList>
#include <QVector>
#include <QPrinter>
#include <QString>
#include <algorithm>
#include <functional>

class QComboBox;
class QWidget;

namespace jp {

enum class SidesChoice { OneSide, LongEdge, ShortEdge, ByHand };

// Fills the box with the choices; `wanted` is the one to start on (a SidesChoice as a number, as it was remembered; anything else is one side).
void fillSidesBox(QComboBox *box, bool withByHand, int wanted);
// Brings the box up to date with the printer the job goes to (`printer` empty is the default one): a printer that can't print on both sides has the two automatic
// choices turned off (the box shows one side then, and goes back to the person's choice when a printer that can is picked), and a PDF file has no sides, so the whole
// box is turned off.
void updateSidesBox(QComboBox *box, const QString &printer, bool toFile);
// What the person chose, and not more than the box allows: the duplex mode for the printer (none for one side, by hand, a PDF file, or a choice that is turned off), and
// whether the pages are printed in two runs by hand.
QPrinter::DuplexMode sidesDuplex(const QComboBox *box);
bool sidesByHand(const QComboBox *box);
// The choice as a number to keep, whatever the printer allows now.
int sidesWanted(const QComboBox *box);
// The modes a printer can print in (what Qt reports); a test stands in for it (the program never does).
QList<QPrinter::DuplexMode> supportedDuplexModes(const QString &printer);
// Sets the duplex mode a job asks the printer for. (A printer engine that writes a file, which is what a machine with no printers has, keeps no mode, so a test listens here
// for what was asked, with setDuplexRecorder, and not to the printer.)
void setPrinterDuplex(QPrinter *printer, QPrinter::DuplexMode mode);
void setDuplexRecorder(const std::function<void(QPrinter::DuplexMode)> &recorder);
// Whether the job goes to a file (a PDF) and not to paper, which has no sides. A machine with no printers can't tell a test's printer from a file, so a test stands in.
bool printsToFile(const QPrinter &printer);
void setPrintsToFileStandIn(const std::function<bool(const QPrinter &)> &standIn);
void setDuplexSupportStandIn(const std::function<QList<QPrinter::DuplexMode>(const QString &printer)> &standIn);

// Both sides by hand: the two runs of a job. `inOrder` is what the job prints, a page or a sheet each, in the order it prints them. The first run is the first, third, and
// so on, in order; the second run is the others, last first, so that each lands on the back of its front when the stack is turned over. When the last front has no
// back (an odd number), the second run starts with `blank`, a sheet to leave empty. A job of one has no second run.
template <class T>
struct HandRuns {
    QVector<T> first, second;
};
template <class T>
HandRuns<T> handRuns(const QVector<T> &inOrder, const T &blank = T())
{
    HandRuns<T> runs;
    for (qsizetype i = 0; i < inOrder.size(); ++i) (i % 2 ? runs.second : runs.first).push_back(inOrder[i]);
    std::reverse(runs.second.begin(), runs.second.end());
    // (an odd number of sheets: the last front has no back, and the second run starts with that one, which is a blank sheet)
    if (inOrder.size() > 1 && inOrder.size() % 2) runs.second.prepend(blank);
    return runs;
}
// Tells the person to turn the stack over and put it back, after the first run; false when they give the job up. `standIn` answers in place of the message when it is
// not empty (a test's way); a test can also stand in for every message (setStackTurnerStandIn; the program never does).
bool askToTurnStack(QWidget *parent, const std::function<bool()> &standIn = nullptr);
void setStackTurnerStandIn(const std::function<bool()> &standIn);

} // namespace jp
