#include "app/uistyle.h"
#include "app/forecastdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace jp {

// ---- the picture

ForecastPreview::ForecastPreview(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("forecastPreview"));
    setMinimumSize(320, 200);
    setAccessibleName(tr("Preview of the forecast"));
}

void ForecastPreview::showResult(const crunch::ForecastResult &result, bool columns)
{
    m_result = result;
    m_columns = columns;
    m_has = result.problem.isEmpty() && !result.points.empty();
    update();
}

void ForecastPreview::clear()
{
    m_has = false;
    update();
}

void ForecastPreview::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), palette().base());
    p.setPen(palette().color(QPalette::Mid));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
    if (!m_has) return;
    const QRectF plot = QRectF(rect()).adjusted(12, 12, -12, -12);
    double lo = std::numeric_limits<double>::infinity(), hi = -lo, t0 = lo, t1 = hi;
    auto see = [&](double t, double v) {
        if (!std::isfinite(t) || !std::isfinite(v)) return;
        t0 = std::min(t0, t);
        t1 = std::max(t1, t);
        lo = std::min(lo, v);
        hi = std::max(hi, v);
    };
    for (const auto &x : m_result.used) see(x.first, x.second);
    for (const auto &x : m_result.later) see(x.first, x.second);
    for (const auto &x : m_result.points) {
        see(x.time, x.forecast);
        see(x.time, x.lower);
        see(x.time, x.upper);
    }
    if (!(t1 > t0) || !(hi >= lo)) return;
    if (hi == lo) {
        hi += 1;
        lo -= 1;
    }
    const double pad = (hi - lo) * 0.05;
    lo -= pad;
    hi += pad;
    auto X = [&](double t) { return plot.left() + (t - t0) / (t1 - t0) * plot.width(); };
    auto Y = [&](double v) { return plot.bottom() - (v - lo) / (hi - lo) * plot.height(); };
    p.setPen(QPen(palette().color(QPalette::Midlight), 1));
    for (int i = 0; i <= 4; ++i) p.drawLine(QPointF(plot.left(), plot.top() + plot.height() * i / 4), QPointF(plot.right(), plot.top() + plot.height() * i / 4));
    const QColor valueColor = palette().color(QPalette::Highlight), forecastColor(237, 125, 49);
    const auto &pts = m_result.points;
    if (!pts.empty()) {
        QPainterPath band;
        band.moveTo(X(pts.front().time), Y(pts.front().upper));
        for (const auto &x : pts) band.lineTo(X(x.time), Y(x.upper));
        for (auto it = pts.rbegin(); it != pts.rend(); ++it) band.lineTo(X(it->time), Y(it->lower));
        band.closeSubpath();
        QColor fill = forecastColor;
        fill.setAlpha(60);
        p.setPen(Qt::NoPen);
        p.setBrush(fill);
        p.drawPath(band);
    }
    std::vector<std::pair<double, double>> actual = m_result.used;
    actual.insert(actual.end(), m_result.later.begin(), m_result.later.end());
    if (m_columns) {
        const double base = Y(0 < lo ? lo : (0 > hi ? hi : 0));
        const double barWidth = std::max(1.0, plot.width() / double(std::max<size_t>(1, actual.size() + pts.size())) * 0.6);
        auto bar = [&](double t, double v) {
            if (std::isfinite(v)) p.drawRect(QRectF(X(t) - barWidth / 2, std::min(Y(v), base), barWidth, std::fabs(Y(v) - base)));
        };
        p.setPen(Qt::NoPen);
        p.setBrush(valueColor);
        for (const auto &x : actual) bar(x.first, x.second);
        p.setBrush(forecastColor);
        for (const auto &x : pts) bar(x.time, x.forecast);
    } else {
        QPolygonF line;
        for (const auto &x : actual)
            if (std::isfinite(x.second)) line << QPointF(X(x.first), Y(x.second));
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(valueColor, 2));
        p.drawPolyline(line);
        QPolygonF fl;
        if (!line.isEmpty() && m_result.later.empty()) fl << line.last();
        for (const auto &x : pts) fl << QPointF(X(x.time), Y(x.forecast));
        p.setPen(QPen(forecastColor, 2, Qt::DashLine));
        p.drawPolyline(fl);
    }
}

// ---- the dialog

ForecastDialog::ForecastDialog(const ForecastSourceReader &reader, jp::calc::DateSystem system, const QString &timeline, const QString &values, QWidget *parent)
    : QDialog(parent), m_reader(reader), m_system(system)
{
    setObjectName(QStringLiteral("forecastDialog"));
    setWindowTitle(tr("Forecast Sheet"));
    auto *layout = new QVBoxLayout(this);
    auto *about = new QLabel(tr("Makes a new sheet with a table and a chart of future values, from a timeline and a series of values."), this);
    about->setWordWrap(true);
    layout->addWidget(about);
    auto *kindRow = new QHBoxLayout;
    m_kind = new QComboBox(this);
    m_kind->setObjectName(QStringLiteral("forecastKind"));
    m_kind->setAccessibleName(tr("Chart type"));
    m_kind->addItem(tr("Line chart"), false);
    m_kind->addItem(tr("Column chart"), true);
    kindRow->addStretch(1);
    kindRow->addWidget(m_kind);
    layout->addLayout(kindRow);
    m_preview = new ForecastPreview(this);
    layout->addWidget(m_preview, 1);
    m_problem = new QLabel(this);
    m_problem->setObjectName(QStringLiteral("forecastProblem"));
    m_problem->setWordWrap(true);
    m_problem->setTextFormat(Qt::PlainText);
    m_problem->setAccessibleName(tr("What is wrong"));
    layout->addWidget(m_problem);
    m_note = new QLabel(this);
    m_note->setObjectName(QStringLiteral("forecastNote"));
    m_note->setWordWrap(true);
    m_note->setTextFormat(Qt::PlainText);
    m_note->hide();
    layout->addWidget(m_note);
    auto *form = new QFormLayout;
    m_timeline = new QLineEdit(timeline, this);
    m_timeline->setObjectName(QStringLiteral("forecastTimeline"));
    m_timeline->setAccessibleName(tr("Timeline range"));
    form->addRow(tr("Timeline range"), m_timeline);
    m_values = new QLineEdit(values, this);
    m_values->setObjectName(QStringLiteral("forecastValues"));
    m_values->setAccessibleName(tr("Values range"));
    form->addRow(tr("Values range"), m_values);
    m_start = new QLineEdit(this);
    m_start->setObjectName(QStringLiteral("forecastStart"));
    m_start->setAccessibleName(tr("Forecast start"));
    m_start->setPlaceholderText(tr("After the last point"));
    form->addRow(tr("Forecast start"), m_start);
    m_end = new QLineEdit(this);
    m_end->setObjectName(QStringLiteral("forecastEnd"));
    m_end->setAccessibleName(tr("Forecast end"));
    form->addRow(tr("Forecast end"), m_end);
    m_confidence = new QSpinBox(this);
    m_confidence->setObjectName(QStringLiteral("forecastConfidence"));
    m_confidence->setAccessibleName(tr("Confidence interval"));
    m_confidence->setRange(1, 99);
    m_confidence->setValue(95);
    m_confidence->setSuffix(QStringLiteral("%"));
    form->addRow(tr("Confidence interval"), m_confidence);
    auto *seasonRow = new QHBoxLayout;
    m_seasonMode = new QComboBox(this);
    m_seasonMode->setObjectName(QStringLiteral("forecastSeasonMode"));
    m_seasonMode->setAccessibleName(tr("Seasonality"));
    m_seasonMode->addItem(tr("Detect automatically"), 1);
    m_seasonMode->addItem(tr("Set manually"), 2);
    m_seasonMode->addItem(tr("None"), 0);
    m_season = new QSpinBox(this);
    m_season->setObjectName(QStringLiteral("forecastSeason"));
    m_season->setAccessibleName(tr("Length of the season"));
    m_season->setRange(2, 100000);
    m_season->setValue(12);
    m_season->setEnabled(false);
    seasonRow->addWidget(m_seasonMode, 1);
    seasonRow->addWidget(m_season);
    form->addRow(tr("Seasonality"), seasonRow);
    m_fill = new QComboBox(this);
    m_fill->setObjectName(QStringLiteral("forecastFill"));
    m_fill->setAccessibleName(tr("Fill missing points using"));
    m_fill->addItem(tr("Interpolation"), true);
    m_fill->addItem(tr("Zeros"), false);
    form->addRow(tr("Fill missing points using"), m_fill);
    m_aggregate = new QComboBox(this);
    m_aggregate->setObjectName(QStringLiteral("forecastAggregate"));
    m_aggregate->setAccessibleName(tr("Aggregate duplicates using"));
    m_aggregate->addItem(tr("Average"), int(calc::ets::Aggregation::Average));
    m_aggregate->addItem(tr("Count"), int(calc::ets::Aggregation::Count));
    m_aggregate->addItem(tr("CountA"), int(calc::ets::Aggregation::CountA));
    m_aggregate->addItem(tr("Max"), int(calc::ets::Aggregation::Max));
    m_aggregate->addItem(tr("Median"), int(calc::ets::Aggregation::Median));
    m_aggregate->addItem(tr("Min"), int(calc::ets::Aggregation::Min));
    m_aggregate->addItem(tr("Sum"), int(calc::ets::Aggregation::Sum));
    form->addRow(tr("Aggregate duplicates using"), m_aggregate);
    layout->addLayout(form);
    m_statistics = new QCheckBox(tr("Include forecast statistics"), this);
    m_statistics->setObjectName(QStringLiteral("forecastStatistics"));
    layout->addWidget(m_statistics);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Create"));
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        refreshNow();
        if (m_ok) accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(150);
    connect(m_timer, &QTimer::timeout, this, &ForecastDialog::refreshNow);
    for (QLineEdit *e : {m_timeline, m_values, m_start}) connect(e, &QLineEdit::textChanged, this, [this] { schedule(); });
    connect(m_end, &QLineEdit::textEdited, this, [this] {
        m_endTouched = true;
        schedule();
    });
    connect(m_confidence, &QSpinBox::valueChanged, this, [this] { schedule(); });
    connect(m_season, &QSpinBox::valueChanged, this, [this] { schedule(); });
    connect(m_seasonMode, &QComboBox::currentIndexChanged, this, [this] {
        m_season->setEnabled(m_seasonMode->currentData().toInt() == 2);
        schedule();
    });
    for (QComboBox *c : {m_kind, m_fill, m_aggregate}) connect(c, &QComboBox::currentIndexChanged, this, [this] { schedule(); });
    connect(m_statistics, &QCheckBox::toggled, this, [this] { schedule(); });
    refreshNow();
    m_timeline->setFocus();
}

QString ForecastDialog::timelineText() const { return m_timeline->text().trimmed(); }
QString ForecastDialog::valuesText() const { return m_values->text().trimmed(); }
QString ForecastDialog::problemText() const { return m_problem->text(); }
QString ForecastDialog::noteText() const { return m_note->isHidden() ? QString() : m_note->text(); }

void ForecastDialog::schedule()
{
    m_ok = false;
    m_timer->start();
}

bool ForecastDialog::parseTime(const QString &text, double *out) const
{
    const QString t = text.trimmed();
    if (t.isEmpty()) return false;
    if (m_dates && (calc::parseIsoDate(t, m_system, out) || calc::dateValueOf(t, m_system, out))) return true;
    bool ok = false;
    double d = QLocale().toDouble(t, &ok);
    if (!ok) d = t.toDouble(&ok);
    if (ok && std::isfinite(d)) {
        *out = d;
        return true;
    }
    return false;
}

QString ForecastDialog::timeText(double t) const
{
    int y, m, d;
    if (m_dates && calc::serialToDate(t, m_system, &y, &m, &d)) return QDate(y, m, d).toString(Qt::ISODate);
    return QString::number(t, 'g', 15);
}

crunch::ForecastSettings ForecastDialog::settings() const
{
    crunch::ForecastSettings s;
    double v;
    s.end = parseTime(m_end->text(), &v) ? v : std::numeric_limits<double>::quiet_NaN();
    if (!m_start->text().trimmed().isEmpty() && parseTime(m_start->text(), &v)) s.start = v;
    s.confidence = m_confidence->value() / 100.0;
    const int mode = m_seasonMode->currentData().toInt();
    s.seasonality = mode == 2 ? m_season->value() : mode;
    s.statistics = m_statistics->isChecked();
    s.interpolate = m_fill->currentData().toBool();
    s.aggregation = calc::ets::Aggregation(m_aggregate->currentData().toInt());
    s.columns = m_kind->currentData().toBool();
    return s;
}

void ForecastDialog::refreshNow()
{
    m_timer->stop();
    m_ok = false;
    if (m_timeline->text().trimmed() != m_loadedTimeline || m_values->text().trimmed() != m_loadedValues || m_loadedTimeline.isEmpty()) {
        m_loadedTimeline = m_timeline->text().trimmed();
        m_loadedValues = m_values->text().trimmed();
        const ForecastSourceInfo info = m_reader(m_loadedTimeline, m_loadedValues, crunch::kPreviewPoints);
        m_input = info.input;
        m_note->setText(info.sampled ? tr("The picture follows the last %1 points of the data. The sheet is made from all of them.").arg(QLocale().toString(crunch::kPreviewPoints)) : QString());
        m_note->setVisible(info.sampled);
        m_dates = info.dates;
        m_sourceOk = info.problem.isEmpty();
        if (!m_sourceOk) {
            m_problem->setText(info.problem);
            m_preview->clear();
            m_result = crunch::ForecastResult();
            return;
        }
        // a new source: the end is the default one again, unless a person typed it
        if (!m_endTouched || m_end->text().trimmed().isEmpty()) {
            const QSignalBlocker block(m_end);
            m_end->setText(timeText(crunch::defaultForecastEnd(m_input, m_system)));
            m_endTouched = false;
        }
    }
    if (!m_sourceOk) return;
    const crunch::ForecastSettings s = settings();
    if (!m_start->text().trimmed().isEmpty() && !std::isfinite(s.start)) {
        m_problem->setText(tr("Write the forecast start as a date or number, or leave it empty to start after the last point."));
        m_preview->clear();
        return;
    }
    if (!std::isfinite(s.end)) {
        m_problem->setText(tr("Write the forecast end as a date or number."));
        m_preview->clear();
        return;
    }
    m_result = crunch::computeForecast(m_input, s, m_system);
    m_problem->setText(m_result.problem);
    if (!m_result.problem.isEmpty()) {
        m_preview->clear();
        return;
    }
    m_preview->showResult(m_result, s.columns);
    m_ok = true;
}

} // namespace jp
