#pragma once
// What the Print pages of the four apps share, so that each choice has the same words, the same name, and the same effect in all of them (printpage.cpp): the printer box, the
// copies and how they come out, the order of the pages, gray, the way the paper turns, and the zoom of the preview.

#include <QSizeF>
#include <QStringList>
#include <QVector>
#include <QLabel>
#include <functional>

class QCheckBox;
class QComboBox;
class QSpinBox;
class QWidget;

namespace jp {

// The pages in the order the sheets come out for `copies` of them: collated, the whole set again for each copy (1 2 3 1 2 3), else each page's copies
// together (1 1 2 2 3 3). Never fewer than one copy.
QVector<int> sheetOrder(const QVector<int> &pages, int copies, bool collate);
// The pages of a job in the order they print: last page first when `reverse` is set.
QVector<int> inPrintOrder(QVector<int> pages, bool reverse);

// The choices of the Print page's printer box: each printer, by its name, and last "Save as PDF". The entries are told apart by their role, not by their text, so a
// printer that is called "pdf" (or "Save as PDF") is a printer. `current` is the printer to start on.
void fillPrinterBox(QComboBox *box, const QStringList &printers, const QString &current);
struct PrinterChoice {
    QString printer;      // the printer's name (empty with a file)
    bool toFile = false;  // "Save as PDF"
};
PrinterChoice printerChoice(const QComboBox *box);
// The printer box filled with the computer's printers, starting on its default one.
QComboBox *makePrinterBox(QWidget *parent);

// The controls, named as the apps' tests find them: printCopies, printCollate, printReverse, printGray, printOrientation, and printZoom.
QSpinBox *makeCopiesBox(QWidget *parent);
// "Collated"; `tip` says what the order is for the app's own pages (1, 2, 3, 1, 2, 3 against 1, 1, 2, 2, 3, 3).
QCheckBox *makeCollateBox(QWidget *parent, const QString &tip);
QCheckBox *makeReverseBox(QWidget *parent);
QCheckBox *makeGrayscaleBox(QWidget *parent);

// The way the paper turns: what each page says (`asSet` words it for the app), tall, or wide. The data of the entries are the values of PrintTurn.
enum class PrintTurn { AsSet = -1, Tall = 1, Wide = 2 };
QComboBox *makeOrientationBox(QWidget *parent, const QString &asSet);
// The size a sheet takes for a page of `size` when the paper is turned: the same size when it is as the page says, else its long side across or its short side across.
QSizeF turnedSheet(const QSizeF &size, PrintTurn turn);

// The zoom of the preview: Fit, 150%, 200%, and 300%. The zoom is how many times larger than the fit the page is drawn (1 is the page that fits the window).
QComboBox *makeZoomBox(QWidget *parent);
double zoomFactor(const QComboBox *box);

// The preview of a Print page, which says when its box changed size so that the page is drawn again to fit.
class PreviewLabel : public QLabel {
public:
    using QLabel::QLabel;
    std::function<void()> resized;

protected:
    void resizeEvent(QResizeEvent *e) override;
};

} // namespace jp
