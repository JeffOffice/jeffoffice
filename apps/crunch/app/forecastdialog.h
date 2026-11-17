#pragma once
// The question of Forecast Sheet: the cells of the timeline and of the values, a picture of the forecast that follows what is chosen, and the options of the forecast (its start and end,
// the confidence interval, the season, statistics, how missing points and repeated times are dealt with). What is wrong with the data or the options is said in the dialog, and OK waits.

#include "core/forecastsheet.h"

#include <QDialog>
#include <QWidget>

#include <functional>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTimer;

namespace jp {

// What the dialog is told of the cells named: the data, whether the timeline is shown as dates, or why it can't be read.
struct ForecastSourceInfo {
    crunch::ForecastInput input;
    bool dates = false;
    bool sampled = false;       // only the last `maxPoints` points were read
    QString problem;
};
// `maxPoints` above 0 asks for no more than that many of the last points (the picture of the dialog does not need a million).
using ForecastSourceReader = std::function<ForecastSourceInfo(const QString &timeline, const QString &values, long long maxPoints)>;

// The picture of a forecast: the values, the forecast, and the band between its bounds.
class ForecastPreview : public QWidget {
    Q_OBJECT
public:
    explicit ForecastPreview(QWidget *parent = nullptr);
    void showResult(const crunch::ForecastResult &result, bool columns);
    void clear();
    QSize sizeHint() const override { return QSize(460, 280); }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    crunch::ForecastResult m_result;
    bool m_columns = false;
    bool m_has = false;
};

class ForecastDialog : public QDialog {
    Q_OBJECT
public:
    ForecastDialog(const ForecastSourceReader &reader, jp::calc::DateSystem system, const QString &timeline, const QString &values, QWidget *parent = nullptr);
    QString timelineText() const;
    QString valuesText() const;
    // The settings as chosen (valid when ok()).
    crunch::ForecastSettings settings() const;
    bool ok() const { return m_ok; }
    // The picture is brought up to date at once, not after the pause that follows typing.
    void refreshNow();
    QString problemText() const;
    // What the picture says of itself when it is made from the last points of a longer series (empty otherwise).
    QString noteText() const;
    const crunch::ForecastResult &result() const { return m_result; }

private:
    void schedule();
    bool parseTime(const QString &text, double *out) const;
    QString timeText(double t) const;

    ForecastSourceReader m_reader;
    jp::calc::DateSystem m_system;
    QLineEdit *m_timeline, *m_values, *m_start, *m_end;
    QComboBox *m_kind, *m_seasonMode, *m_fill, *m_aggregate;
    QSpinBox *m_confidence, *m_season;
    QCheckBox *m_statistics;
    QLabel *m_problem, *m_note;
    ForecastPreview *m_preview;
    QTimer *m_timer;
    crunch::ForecastInput m_input;
    bool m_dates = false, m_ok = false, m_endTouched = false, m_sourceOk = false;
    QString m_loadedTimeline, m_loadedValues;
    crunch::ForecastResult m_result;
};

} // namespace jp
