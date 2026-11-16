#pragma once
// One chart editor for every app: the menus that add, take away, and place the parts of a chart (Add Chart Element), its layouts, colors, and looks, the series it shows, the buttons beside a
// chart that open them, and the commands of the Chart Design tab (Edit Data, Select Data, Switch Row/Column, Change Chart Type, Format Axis). The model is changed by libs/office's
// sheet/chartedit.h; what an app gives is where its chart is, how a change is made as one step that undoes, and (when it has cells) how a cell reference is read.

#include "core/export.h"
#include "office/chart.h"
#include "office/theme.h"
#include "sheet/chartops.h"

#include <QObject>
#include <QString>
#include <QToolButton>
#include <QVector>
#include <functional>

class QAction;
class QMenu;
class QWidget;

namespace jp {

class ChartDesignHost {
public:
    virtual ~ChartDesignHost();
    // The window that dialogs belong to.
    virtual QWidget *chartParent() = 0;
    // The chart that is selected now (a copy); false when none is.
    virtual bool chartNow(office::Chart *out) = 0;
    // The theme the chart is colored by.
    virtual office::Theme chartTheme() = 0;
    // Changes the selected chart as one step named `label`: `edit` is given the chart. False when nothing was changed (no chart, or one that can't be changed, or no difference).
    virtual bool changeChart(const QString &label, const std::function<void(office::Chart *)> &edit) = 0;
    // Tells the person something (`warning`: a refusal, not just news).
    virtual void chartNotice(const QString &title, const QString &text, bool warning = true) = 0;
    // A short text (the words of a title, a format code); false when cancelled.
    virtual bool askChartText(const QString &caption, const QString &prompt, QString *text);
    // Whether the file the chart is in keeps a data table (an OpenDocument file does not: its charts are saved without one). The Data Table entries are off, with the reason, when not.
    virtual bool chartKeepsDataTable() const { return true; }
    // Whether the chart's cells are there to be named: titles and labels may follow cells, and error bars may read them. An app with none keeps its charts' data in the charts.
    virtual bool chartHasCells() const { return false; }
    // `typed` made into a reference that is good for the chart ("Sheet1!$A$1"), one cell or a range; empty (after telling the person) when it names none.
    virtual QString chartCellRef(const QString &typed, bool oneCell, const QString &caption)
    {
        Q_UNUSED(typed);
        Q_UNUSED(oneCell);
        Q_UNUSED(caption);
        return QString();
    }
    // Puts what the cells hold now in the chart.
    virtual void chartReadCells(office::Chart *chart) { Q_UNUSED(chart); }
    // The workbook kept beside the selected chart (the bytes of the part its c:externalData names); empty for a chart with none, and for an app that does not keep one.
    virtual QByteArray chartWorkbook() { return QByteArray(); }
    // Changes the numbers of a chart that reads cells of that workbook: the cells and the values the chart stores, from the grid, as one step named `label`. False when nothing changed or it can't be.
    virtual bool changeChartWorkbookData(const QString &label, const office::sheet::ChartData &data)
    {
        Q_UNUSED(label);
        Q_UNUSED(data);
        return false;
    }
};

class JO_COMMON_EXPORT ChartDesign : public QObject {
    Q_OBJECT
public:
    explicit ChartDesign(ChartDesignHost *host, QObject *parent = nullptr);

    // Add Chart Element (the menu is made again each time it opens, for the chart that is selected); Quick Layout; Change Colors; Chart Styles; and the one menu the Looks button has (the colors, then the looks).
    QMenu *elementsMenu(QWidget *owner);
    QMenu *layoutsMenu(QWidget *owner);
    QMenu *colorsMenu(QWidget *owner);
    QMenu *stylesMenu(QWidget *owner);
    QMenu *looksMenu(QWidget *owner);
    // Chart Filters: the series of the chart, as a menu of check boxes.
    QMenu *filtersMenu(QWidget *owner);
    struct FilterEntry {
        QString name;
        bool shown = true;
    };
    // The series of the chart (those drawn, then those filtered out), and whether each is drawn. A filtered series is left out of the drawing; the file keeps it as a series like the others; the last
    // one drawn can't be filtered out.
    QVector<FilterEntry> filterEntries() const;
    bool setSeriesShown(int entry, bool shown);

    // The commands of the Chart Design tab, for a chart that holds its own numbers (every chart that has no cells).
    // Whether the numbers of the chart can be edited here: it plots numbers of its own, not cells.
    bool dataEditable() const;
    // Why not, in a sentence (empty when it can, and for no chart): for the tooltip of a command that is off, and what a command says when it is chosen anyway.
    QString dataProblem() const;
    // The same for the commands that change the rows and columns (Select Data, Swap Rows and Columns): numbers that are the cells of a workbook kept with the chart can be edited but not
    // rearranged, and that is the reason then; empty when the numbers are the chart's own and can be.
    QString structureProblem() const;
    // That reason, for a chart known to read the cells of its workbook.
    static QString workbookShapeProblem();
    void editData();
    void selectData();
    // The data with rows and columns the other way (the series become the categories); false (with a notice) when it can't be.
    bool switchRowsAndColumns();
    void changeType();
    // Format Axis: the bounds, units, order, crossing, number format, and ticks of the horizontal or vertical axis.
    void formatAxis(bool horizontal);
    // Format Chart Part: the look of the chart area, the plot area, or a series (`part`: 0, 1, or 2 and on for the series).
    void formatPart(int part = 0);

    ChartDesignHost *host() const { return m_host; }

private:
    ChartDesignHost *m_host;
};

// The three buttons beside a selected chart: Chart Parts (the elements menu), Chart Looks (colors and looks), and Chart Filters (the series). They are children of `viewport`, which the app
// places the chart in; place() puts them outside `chart` (a rectangle in the viewport's coordinates), at its right, or at its left when there is no room, so that they never cover its handles; they are
// hidden while a mouse button is down (a drag).
class JO_COMMON_EXPORT ChartButtons : public QObject {
    Q_OBJECT
public:
    ChartButtons(ChartDesign *design, QWidget *viewport, const std::function<QIcon(const QString &)> &icon, QObject *parent = nullptr);
    void place(const QRect &chart);
    void hide();
    QToolButton *elements() const { return m_elements; }
    QToolButton *looks() const { return m_looks; }
    QToolButton *filters() const { return m_filters; }
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    // A mouse button was let go on the viewport (a drag of the chart ended): the buttons may stand again.
    void released();

private:
    ChartDesign *m_design;
    QWidget *m_viewport;
    std::function<QIcon(const QString &)> m_icon;
    QToolButton *m_elements = nullptr, *m_looks = nullptr, *m_filters = nullptr;
};

// Whether a file's ending is an OpenDocument one (.odt, .ods, .odp, their templates, and their flat forms).
JO_COMMON_EXPORT bool isOpenDocumentPath(const QString &path);

// The chart entries of the right-click menu of a selected chart, the same in every program: Chart Data, Swap Rows and Columns, Chart Kind, Format Chart Part, and the two axes, each the window's own
// action `chartdesign.*` (found with `find`), so that an entry that is off says why in its tooltip, as the Chart tab's does. The menu shows tooltips.
JO_COMMON_EXPORT void addChartContextEntries(QMenu *menu, const std::function<QAction *(const QString &)> &find);

// The chart as a picture at a size, for the dialogs that show what a choice does.
QImage chartPicture(const office::Chart &chart, const QSize &pixels);

// The numbers a chart plots when they are its own (not read from cells); false for a chart that reads cells, has no series, or is a pivot chart.
bool chartOwnData(const office::Chart &chart, office::sheet::ChartData *data);
// The data with its rows and columns switched: the names of the series become the words under the points, and the words become the names. False when the result would be more than a chart holds.
bool switchedChartData(const office::sheet::ChartData &data, office::sheet::ChartData *out);

} // namespace jp
