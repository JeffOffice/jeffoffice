#include "app/printpage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPrinterInfo>
#include <QPushButton>
#include <QVBoxLayout>
#include "app/plaintext.h"
#include "app/uistyle.h"
#include "app/icons.h"
#include <QSpinBox>
#include <algorithm>

namespace jp {

namespace {
constexpr int kIsFileRole = Qt::UserRole + 1;   // set on the entry that makes a PDF file
std::function<void(const QString &, const QString &)> g_noticeStandIn;
std::function<QStringList()> g_printersStandIn;
}

bool tooManyToSend(qint64 sheets, qint64 copies) { return sheets * std::max<qint64>(1, copies) > kMaxSentSheets; }

void setPrintNoticeStandIn(const std::function<void(const QString &, const QString &)> &standIn) { g_noticeStandIn = standIn; }

void printNotice(QWidget *parent, const QString &title, const QString &text)
{
    if (g_noticeStandIn) g_noticeStandIn(title, text);
    else plainWarning(parent, title, text);
}

void setPrinterNamesStandIn(const std::function<QStringList()> &standIn) { g_printersStandIn = standIn; }

QStringList printerNames()
{
    if (g_printersStandIn) return g_printersStandIn();
    QStringList names;
    for (const QPrinterInfo &info : QPrinterInfo::availablePrinters()) names << info.printerName();
    return names;
}

bool printerExists(const QString &name)
{
    if (name.isEmpty()) return true;
    return printerNames().contains(name);
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
    const QStringList names = printerNames();
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
    // plain: reviewed, the callers give their own tr() text
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

QComboBox *makeColorModeBox(QWidget *parent)
{
    auto *color = new QComboBox(parent);
    color->setObjectName(QStringLiteral("printColor"));
    color->setAccessibleName(QCoreApplication::translate("Print", "Color"));
    color->addItem(QCoreApplication::translate("Print", "Color"), int(PrintColor::Color));
    color->addItem(QCoreApplication::translate("Print", "Grayscale"), int(PrintColor::Grayscale));
    color->addItem(QCoreApplication::translate("Print", "Pure Black and White"), int(PrintColor::BlackAndWhite));
    color->setToolTip(jp::plainToolTip(QCoreApplication::translate("Print", "Color as it is, shades of gray, or black and white alone (every point is black or white).")));
    return color;
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

QComboBox *makePaperBox(QWidget *parent, const QString &asSet)
{
    auto *paper = new QComboBox(parent);
    paper->setObjectName(QStringLiteral("printPaper"));
    paper->setAccessibleName(QCoreApplication::translate("Print", "Paper size"));
    paper->addItem(asSet, -1);
    for (QPageSize::PageSizeId id : {QPageSize::Letter, QPageSize::Legal, QPageSize::Tabloid, QPageSize::A3, QPageSize::A4, QPageSize::A5}) paper->addItem(QPageSize::name(id), int(id));
    return paper;
}

QSizeF paperChoiceSize(const QComboBox *box)
{
    const int id = box->currentData().toInt();
    return id < 0 ? QSizeF() : QPageSize(QPageSize::PageSizeId(id)).size(QPageSize::Point);
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

PrintColumn::PrintColumn(QWidget *page) : m_page(page), m_column(new QVBoxLayout())
{
    m_column->setContentsMargins(0, 0, 0, 0);
    m_column->setSpacing(8);
}

QString PrintColumn::printerHeading() { return QCoreApplication::translate("Print", "Printer"); }
QString PrintColumn::pagesHeading() { return QCoreApplication::translate("Print", "Pages"); }
QString PrintColumn::layoutHeading() { return QCoreApplication::translate("Print", "Layout"); }
QString PrintColumn::paperHeading() { return QCoreApplication::translate("Print", "Paper"); }

void PrintColumn::top(QPushButton *print, QSpinBox *copies)
{
    print->setProperty("primary", true);
    print->setMinimumHeight(40);
    print->setMinimumWidth(120);
    print->setCursor(Qt::PointingHandCursor);
    auto *row = new QHBoxLayout();
    row->setSpacing(12);
    row->addWidget(print);
    auto *label = new QLabel(QCoreApplication::translate("Print", "Copies"), m_page);
    label->setBuddy(copies);
    row->addWidget(label);
    row->addWidget(copies);
    row->addStretch(1);
    m_column->addLayout(row);
    m_column->addSpacing(8);
}

void PrintColumn::group(const QString &heading, const QString &note)
{
    if (!m_first) m_column->addSpacing(8);
    m_first = false;
    auto *row = new QHBoxLayout();
    row->setSpacing(8);
    row->addWidget(sectionHeading(heading, m_page));
    if (!note.isEmpty()) {
        QLabel *n = noteLabel(note, m_page);
        n->setWordWrap(false);
        QFont f = n->font();
        f.setPixelSize(11);
        n->setFont(f);
        row->addWidget(n);
    }
    row->addStretch(1);
    m_column->addLayout(row);
}

void PrintColumn::field(const QString &label, QWidget *a)
{
    if (label.isEmpty()) m_column->addWidget(a);
    else m_column->addWidget(labeled(label, a, m_page));
}

void PrintColumn::pair(const QString &labelA, QWidget *a, const QString &labelB, QWidget *b)
{
    if (!b) field(labelA, a);
    else m_column->addWidget(twoUp(labelA, a, labelB, b, m_page));
}

void PrintColumn::checks(QWidget *a, QWidget *b)
{
    if (!b) m_column->addWidget(a);
    else m_column->addWidget(checkPair(a, b, m_page));
}

void PrintColumn::wide(QWidget *w) { m_column->addWidget(w); }

void PrintColumn::link(QPushButton *button)
{
    button->setProperty("link", true);
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    QPalette p = button->palette();
    p.setColor(QPalette::ButtonText, uiAccent());
    button->setPalette(p);
    m_column->addSpacing(8);
    m_column->addWidget(button, 0, Qt::AlignLeft);
}

} // namespace jp
