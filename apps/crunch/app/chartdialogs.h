#pragma once
// The dialogs of charts: New Chart (a kind of chart, shown on the cells selected), Chart Data (the series a chart reads, their names, values, and categories, and what to do with empty and
// hidden cells), Chart Kind (the kind of the chart, and of each series), and the Properties of an object over the cells (its size, and whether it follows the cells when they change).
// Each is given what it shows as plain data and gives back what the person chose; the window turns that into a step of Undo.

#include "office/chart.h"
#include "sheet/chartedit.h"
#include "sheet/chartmake.h"
#include "sheet/drawing.h"

#include <QDialog>
#include <QImage>
#include <QSize>

#include <functional>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QListWidget;
class QRadioButton;
class QTableWidget;

namespace jp {

// The kinds a person picks from, in the order they are listed, with the words for each.
struct ChartKindEntry {
    jp::office::sheet::ChartKind kind;
    const char *name;
};
const QVector<ChartKindEntry> &chartKindEntries();
QString chartKindName(jp::office::sheet::ChartKind kind);

// A picture of a chart for a kind and a way of reading the cells (-1 as the shape of the selection says, 0 series down columns, 1 along rows), at a size in pixels; null if there is nothing to chart.
using ChartPreviewFn = std::function<QImage(jp::office::sheet::ChartKind kind, int inRows, const QSize &pixels)>;

class InsertChartDialog : public QDialog {
    Q_OBJECT
public:
    // `only`, when it has kinds, lists those in that order (Recommended Charts), under `title`.
    InsertChartDialog(const ChartPreviewFn &preview, jp::office::sheet::ChartKind initial, QWidget *parent = nullptr, const QVector<jp::office::sheet::ChartKind> &only = {}, const QString &title = QString());
    jp::office::sheet::ChartKind kind() const;
    // -1 as the cells say, 0 the series are columns, 1 they are rows.
    int inRows() const;
    void selectKind(jp::office::sheet::ChartKind kind);
    void setRowsAndColumns(bool switched);
    // The picture in the big preview now (for tests).
    QImage previewShown() const;

private:
    void refresh();
    ChartPreviewFn m_preview;
    QListWidget *m_list = nullptr;
    QLabel *m_big = nullptr;
    QCheckBox *m_switch = nullptr;
};

// What Chart Data starts from and gives back.
struct SelectDataRequest {
    jp::office::Chart chart;
    jp::office::Theme theme;
    // Whether a reference names cells that can be read (and why not); unset: any reference that looks like one.
    std::function<bool(const QString &ref, QString *why)> checkRef;
    // The series a block of cells makes (the data range typed in), with the way they run (-1 as the shape says); empty if there is none.
    std::function<jp::office::sheet::ChartPlan(const QString &range, int inRows)> planFor;
    // The chart with its series run the other way, if that can be done.
    std::function<bool(const jp::office::Chart &now, jp::office::Chart *switched)> switcher;
    // The name a series has now, from the cells (for the list).
    std::function<QString(const jp::office::ChartSeries &)> nameOf;
};

class SelectDataDialog : public QDialog {
    Q_OBJECT
public:
    explicit SelectDataDialog(const SelectDataRequest &request, QWidget *parent = nullptr);
    // The chart as the person left it.
    jp::office::Chart result() const;
    // For tests and the window: the list and its commands.
    int seriesShown() const;
    void addSeries(const jp::office::ChartSeries &series);
    void removeSeries(int index);
    void moveSeries(int index, int delta);
    void setSeries(int index, const jp::office::ChartSeries &series);
    jp::office::ChartSeries seriesAt(int index) const;
    void setCategoryRef(const QString &ref);
    void applyDataRange(const QString &range);
    void switchRowsAndColumns();
    void setBlanks(const QString &how, bool plotHidden);

private:
    void rebuildList();
    bool editSeries(int index);
    SelectDataRequest m_request;
    QVector<jp::office::ChartSeries> m_series;
    QVector<jp::office::sheet::SeriesKind> m_kinds;
    jp::office::Chart m_chart;
    QListWidget *m_list = nullptr;
    QLineEdit *m_categories = nullptr, *m_range = nullptr;
    QComboBox *m_blanks = nullptr;
    QCheckBox *m_hidden = nullptr;
    QLabel *m_problem = nullptr;
};

class ChangeChartTypeDialog : public QDialog {
    Q_OBJECT
public:
    // `preview` draws the chart with each series as the kinds say, at a size.
    using KindsPreviewFn = std::function<QImage(const QVector<jp::office::sheet::SeriesKind> &kinds, const QSize &pixels)>;
    ChangeChartTypeDialog(const QStringList &seriesNames, const QVector<jp::office::sheet::SeriesKind> &now, const KindsPreviewFn &preview, QWidget *parent = nullptr);
    // What each series is drawn as.
    QVector<jp::office::sheet::SeriesKind> kinds() const;
    // The whole chart as one kind (every series), or a combination series by series.
    void chooseKind(jp::office::sheet::ChartKind kind);
    void chooseCombination(const QVector<jp::office::sheet::SeriesKind> &kinds);
    bool isCombination() const;

private:
    void refresh();
    QStringList m_names;
    KindsPreviewFn m_preview;
    QListWidget *m_list = nullptr;
    QTableWidget *m_table = nullptr;
    QLabel *m_big = nullptr;
    bool m_loading = false;
};

// Format Axis: the bounds and units (automatic or a number), the order, where the other axis crosses, the labels' number format, turn, and place, and the tick marks of one axis. It is given the
// axis as it is and gives it back with those parts as the person set them.
class FormatAxisDialog : public QDialog {
    Q_OBJECT
public:
    FormatAxisDialog(const jp::office::ChartAxis &axis, const QString &title, QWidget *parent = nullptr);
    jp::office::ChartAxis result() const;
    // Why what is set can't be used (a sentence), or empty: the lowest value not under the highest, a unit that is not more than 0 when it is not automatic.
    QString problem() const;
    void accept() override;

private:
    jp::office::ChartAxis m_axis;
    QCheckBox *m_autoMin = nullptr, *m_autoMax = nullptr, *m_autoMajor = nullptr, *m_autoMinor = nullptr, *m_reversed = nullptr, *m_log = nullptr, *m_linked = nullptr, *m_rotate = nullptr, *m_unitLabel = nullptr;
    QDoubleSpinBox *m_min = nullptr, *m_max = nullptr, *m_major = nullptr, *m_minor = nullptr, *m_crossAt = nullptr, *m_angle = nullptr, *m_logBase = nullptr;
    QComboBox *m_crosses = nullptr, *m_majorTick = nullptr, *m_minorTick = nullptr, *m_labels = nullptr, *m_units = nullptr;
    QLineEdit *m_format = nullptr;
};

// The size of an object, and whether it follows the cells when they are moved or made larger or smaller.
struct ObjectProperties {
    double widthPoints = 0, heightPoints = 0;
    jp::office::sheet::AnchorKind follows = jp::office::sheet::AnchorKind::TwoCell;   // two cells: moves and sizes with them; one: moves; absolute: neither
    QString name;
    QString description;   // its alternative text, for people who cannot see it
    bool keepProportions = false;
    bool prints = true;   // on the printed pages
};

class ObjectPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    explicit ObjectPropertiesDialog(const ObjectProperties &now, QWidget *parent = nullptr);
    ObjectProperties result() const;

private:
    QDoubleSpinBox *m_width = nullptr, *m_height = nullptr;
    QRadioButton *m_sizes = nullptr, *m_moves = nullptr, *m_stays = nullptr;
    QCheckBox *m_ratio = nullptr, *m_prints = nullptr;
    QLineEdit *m_name = nullptr;
    QPlainTextEdit *m_alt = nullptr;
    double m_ratioOf = 1;
    ObjectProperties m_now;
};

} // namespace jp
