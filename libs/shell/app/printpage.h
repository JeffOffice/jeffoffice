#pragma once
// What the Print pages of the four apps share, so that each choice has the same words, the same name, and the same effect in all of them (printpage.cpp): the printer box, the
// copies and how they come out, the order of the pages, gray, the way the paper turns, and the zoom of the preview.

#include <QPageSize>
#include <QSizeF>
#include <QStringList>
#include <QVector>
#include <QLabel>
#include <algorithm>
#include <functional>

class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QComboBox;
class QSpinBox;
class QWidget;

namespace jp {

// The pages in the order the sheets come out for `copies` of them: collated, the whole set again for each copy (1 2 3 1 2 3), else each page's copies
// together (1 1 2 2 3 3). Never fewer than one copy.
QVector<int> sheetOrder(const QVector<int> &pages, int copies, bool collate);
// The pages of a job in the order they print: last page first when `reverse` is set.
QVector<int> inPrintOrder(QVector<int> pages, bool reverse);

// The most sheets one job sends to a printer, copies counted (a hundred thousand pages in a hundred copies is not a print job).
constexpr qint64 kMaxSentSheets = 2000000;
// Whether `sheets` sheets in `copies` copies are more than a job may send.
bool tooManyToSend(qint64 sheets, qint64 copies);
// A plain message for the person (a test listens here, with setPrintNoticeStandIn, instead of a message box; the program never does).
void printNotice(QWidget *parent, const QString &title, const QString &text);
void setPrintNoticeStandIn(const std::function<void(const QString &title, const QString &text)> &standIn);
// The printers the computer has, by name; a test stands in for them (the program never does). The list is asked for when a Print page is opened, never before.
QStringList printerNames();
void setPrinterNamesStandIn(const std::function<QStringList()> &standIn);
// Whether a printer of this name is there now (the empty name is the default printer, which is there when the computer has one).
bool printerExists(const QString &name);

// The sheets of a job (each a sheet of paper's side: a page, or a group of pages) in the order they print, for the copies and the order the person chose. With `reverse`
// the last sheet comes first; where both sides of the paper are used (`duplex`) the sheets are taken in pairs, a front with its back, and it is the pairs that are
// reversed (1-4 comes out as 3 4, then 1 2), a last front with no back getting `blank` behind it. Copies sent again by the program (`copies` more than 1) each begin on a
// new sheet of paper, so with `duplex` a copy of an odd number of sides ends with a blank back; collated is the whole set again for each copy, else each sheet's copies together.
template <class T>
QVector<T> sheetSequence(const QVector<T> &sheets, const T &blank, bool reverse, bool duplex, int copies, bool collate)
{
    copies = std::max(1, copies);
    const bool pairs = duplex && (reverse || copies > 1);
    QVector<QVector<T>> units;
    for (qsizetype i = 0; i < sheets.size(); i += pairs ? 2 : 1) {
        QVector<T> unit{sheets[i]};
        if (pairs) unit.push_back(i + 1 < sheets.size() ? sheets[i + 1] : blank);
        units.push_back(unit);
    }
    if (reverse) std::reverse(units.begin(), units.end());
    QVector<T> out;
    if (collate) {
        for (int c = 0; c < copies; ++c)
            for (const QVector<T> &unit : units) out += unit;
    } else {
        for (const QVector<T> &unit : units)
            for (int c = 0; c < copies; ++c) out += unit;
    }
    return out;
}

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
// Color, Grayscale, or Pure Black and White (printColor): the data of the entries are the values of PrintColor.
enum class PrintColor { Color = 0, Grayscale = 1, BlackAndWhite = 2 };
QComboBox *makeColorModeBox(QWidget *parent);

// What prints (JeffWrite's page): the markup of a document under review, or the text as it stands; and two switches (printMarkup, printHidden, printBackground).
// The data of the markup box's entries are 1 (Print Markup) and 0 (Without Markup).
QComboBox *makeMarkupBox(QWidget *parent);
QCheckBox *makeHiddenTextBox(QWidget *parent);
QCheckBox *makeBackgroundBox(QWidget *parent);

// The way the paper turns: what each page says (`asSet` words it for the app), tall, or wide. The data of the entries are the values of PrintTurn.
enum class PrintTurn { AsSet = -1, Tall = 1, Wide = 2 };
QComboBox *makeOrientationBox(QWidget *parent, const QString &asSet);
// The size a sheet takes for a page of `size` when the paper is turned: the same size when it is as the page says, else its long side across or its short side across.
QSizeF turnedSheet(const QSizeF &size, PrintTurn turn);

// The paper: what the job says, or a standard size (the data of an entry is a QPageSize::PageSizeId, or -1 for what the job says).
QComboBox *makePaperBox(QWidget *parent, const QString &asSet);
// The size in points, upright, of the paper an entry of the paper box stands for (empty for what the job says).
QSizeF paperChoiceSize(const QComboBox *box);

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

// The left column of a Print page, the same in the four apps (docs/design/STYLE.md): the Print button with Copies on top, then groups under small headings (PRINTER, PAGES, LAYOUT,
// PAPER), two fields to a row with a small label above each, two checkboxes on one line, and Page Setup as a link at the bottom. The app makes its own controls (their names and wiring
// are its own) and hands them over in order; nothing here reads or changes what they hold.
class PrintColumn {
public:
    explicit PrintColumn(QWidget *page);
    QVBoxLayout *layout() const { return m_column; }
    // The Print button (accent-filled) and, beside it, "Copies" with its box.
    void top(QPushButton *print, QSpinBox *copies);
    // A new group: its heading in capitals, and a short note beside it (empty for none).
    void group(const QString &heading, const QString &note = QString());
    // One field across the column, or two side by side; `b` may be null. A label may be empty.
    void field(const QString &label, QWidget *a);
    void pair(const QString &labelA, QWidget *a, const QString &labelB, QWidget *b);
    // Two checkboxes on one line (the second may be null).
    void checks(QWidget *a, QWidget *b = nullptr);
    // Any widget as it is (a row of radio buttons, a note).
    void wide(QWidget *w);
    // "Page Setup" as a link at the bottom.
    void link(QPushButton *button);
    // The words of the shared groups.
    static QString printerHeading();
    static QString pagesHeading();
    static QString layoutHeading();
    static QString paperHeading();
    static QString whatPrintsHeading();

private:
    QWidget *m_page;
    QVBoxLayout *m_column;
    bool m_first = true;
};

} // namespace jp
