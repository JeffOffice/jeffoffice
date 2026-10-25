// The chart-making part of jcrunchtool (docs/jeffcrunch.md): charts made and changed in the window's model, saved, and read back.
//   jcrunchtool charts-roundtrip <in.xlsx> <out.xlsx>   opens the workbook as the window does, writes every chart it can change again from the model (so that the part is made by the writer),
//                                                       saves, opens the new file, and compares each chart before and after: its kind, its series' references, and its title. One line of
//                                                       tab-separated fields: path, exit (0 done, 1 not opened, 2 not saved), charts, charts written again, charts the writer would lose
//                                                       something of (left as they were), charts the same after, charts that differ, then the first difference
//   jcrunchtool charts-insert <in.xlsx> <out.xlsx> <sheet> <range> <kind> [kind...]   makes a chart of each kind (column, bar, line, pie, area, scatter, combo, or the names the ribbon
//                                                       uses) from the cells of the range on the sheet (from 1) and saves the workbook
//   jcrunchtool charts-shots <in.xlsx> <out-dir>        pictures of the window with a chart made from the first cells of the first sheet and picked, its elements menu, the Insert Chart dialog,
//                                                       Select Data, Change Chart Type, and an object's menu

#include "app/chartdialogs.h"
#include "app/crunchwindow.h"
#include "core/sheetobjects.h"
#include "grid/gridview.h"
#include "sheet/chartedit.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QMenu>
#include <QMetaObject>
#include <QThread>
#include <QToolButton>
#include <QPixmap>
#include <QTextStream>
#include <QTimer>

using namespace jp;
using namespace jp::office;
using namespace jp::office::sheet;

namespace {

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

// Runs the event loop until the condition holds or the seconds pass.
bool waitFor(const std::function<bool()> &done, int seconds)
{
    QElapsedTimer t;
    t.start();
    while (!done()) {
        if (t.elapsed() > seconds * 1000) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QThread::msleep(2);
    }
    return true;
}

bool openWindow(CrunchWindow &w, const QString &path)
{
    bool loaded = false, finished = false, ok = true;
    QObject::connect(&w, &CrunchWindow::loadFinished, &w, [&](bool good) {
        loaded = true;
        ok = good;
    });
    QObject::connect(&w, &CrunchWindow::calculationFinished, &w, [&] { finished = true; });
    w.setNotifier([](const QString &, const QString &) {});
    if (!w.openFile(path)) return false;
    if (!waitFor([&] { return loaded; }, 120)) return false;
    waitFor([&] { return finished || !w.isCalculating(); }, 120);
    // (the objects of the first sheet are made a moment after it is shown)
    QCoreApplication::processEvents();
    return ok;
}

bool saveWindow(CrunchWindow &w, const QString &path)
{
    bool done = false, ok = false;
    QObject::connect(&w, &CrunchWindow::saved, &w, [&](const QString &, bool good) {
        done = true;
        ok = good;
    });
    if (!w.saveAs(path)) return false;
    return waitFor([&] { return done; }, 120) && ok;
}

struct Sig {
    QString kind, title;
    QStringList refs;
    int series = 0;
    bool operator==(const Sig &o) const { return kind == o.kind && title == o.title && refs == o.refs && series == o.series; }
    QString text() const { return kind + QLatin1Char('/') + QString::number(series) + QLatin1Char('/') + refs.join(QLatin1Char(',')) + QLatin1Char('/') + title; }
};

QVector<Sig> signatures(CrunchWindow &w)
{
    QVector<Sig> sigs;
    crunch::SheetObjects *o = w.objects();
    const int sheets = w.loaded() && w.loaded()->book ? int(w.loaded()->book->info().sheets.size()) : 0;
    for (int sheet = 0; sheet < sheets; ++sheet) {
        for (const int id : o->idsOf(sheet)) {
            if (o->kindOf(sheet, id) != ObjectKind::Chart) continue;
            const Chart c = o->chartNow(sheet, id);
            Sig s;
            s.kind = QString::number(int(kindOfChart(c)));
            s.title = c.title.present ? (c.title.automatic ? QStringLiteral("(automatic)") : c.title.text) : QString();
            s.series = seriesCount(c);
            for (const ChartSeries &x : seriesOf(c)) s.refs << x.nameRef + QLatin1Char('|') + x.categoryRef + QLatin1Char('|') + x.valueRef + QLatin1Char('|') + x.xRef;
            sigs << s;
        }
    }
    return sigs;
}

int roundtrip(const QString &in, const QString &outPath)
{
    QElapsedTimer clock;
    clock.start();
    CrunchWindow w;
    if (!openWindow(w, in) || !w.session()) {
        out() << in << "\t1\n";
        return 1;
    }
    crunch::SheetObjects *o = w.objects();
    const QVector<Sig> before = signatures(w);
    int rewritten = 0, notEditable = 0;
    const int sheets = int(w.loaded()->book->info().sheets.size());
    for (int sheet = 0; sheet < sheets; ++sheet)
        for (const int id : o->idsOf(sheet)) {
            if (o->kindOf(sheet, id) != ObjectKind::Chart) continue;
            if (!o->canEditChart(sheet, id)) {
                ++notEditable;
                continue;
            }
            crunch::ObjectEdit e;
            e.sheet = sheet;
            e.id = id;
            e.before = o->stateOf(sheet, id);
            e.after = o->stateWithChart(sheet, id, o->chartNow(sheet, id));
            if (w.session()->changeObjects({e}, QStringLiteral("rewrite")).ok) ++rewritten;
        }
    if (!saveWindow(w, outPath)) {
        out() << in << "\t2\t" << before.size() << "\n";
        return 2;
    }
    CrunchWindow second;
    if (!openWindow(second, outPath)) {
        out() << in << "\t1\t" << before.size() << "\treopen\n";
        return 1;
    }
    const QVector<Sig> after = signatures(second);
    int same = 0, differ = 0;
    QString first;
    for (int i = 0; i < before.size(); ++i) {
        if (i < after.size() && before[i] == after[i]) {
            ++same;
        } else {
            ++differ;
            if (first.isEmpty()) first = before[i].text() + QStringLiteral(" -> ") + (i < after.size() ? after[i].text() : QStringLiteral("(gone)"));
        }
    }
    if (after.size() != before.size()) {
        ++differ;
        if (first.isEmpty()) first = QStringLiteral("%1 charts before, %2 after").arg(before.size()).arg(after.size());
    }
    out() << in << "\t0\t" << before.size() << "\t" << rewritten << "\t" << notEditable << "\t" << same << "\t" << differ << "\t" << first << "\t" << clock.elapsed() << "\n";
    return 0;
}

ChartKind kindNamed(const QString &name, bool *ok)
{
    static const QHash<QString, ChartKind> kinds = {{"column", ChartKind::Column}, {"column-stacked", ChartKind::ColumnStacked}, {"column-percent", ChartKind::ColumnPercent},
                                                    {"bar", ChartKind::Bar}, {"bar-stacked", ChartKind::BarStacked}, {"bar-percent", ChartKind::BarPercent}, {"line", ChartKind::Line},
                                                    {"line-markers", ChartKind::LineMarkers}, {"line-stacked", ChartKind::LineStacked}, {"pie", ChartKind::Pie}, {"doughnut", ChartKind::Doughnut},
                                                    {"area", ChartKind::Area}, {"area-stacked", ChartKind::AreaStacked}, {"scatter", ChartKind::Scatter}, {"scatter-lines", ChartKind::ScatterLines},
                                                    {"combo", ChartKind::Combo}};
    *ok = kinds.contains(name);
    return kinds.value(name, ChartKind::Column);
}

int insert(const QString &in, const QString &outPath, int sheet, const QString &range, const QStringList &kinds)
{
    CrunchWindow w;
    if (!openWindow(w, in)) {
        out() << in << "\t1\n";
        return 1;
    }
    w.setActiveSheet(sheet);
    waitFor([&] { return w.objects() && w.objects()->shownSheet() == sheet; }, 20);
    jp::office::sheet::CellRange r;
    if (!parseRangeRef(range, &r)) {
        out() << in << "\t3\tbad range\n";
        return 3;
    }
    int made = 0;
    for (const QString &name : kinds) {
        bool ok = false;
        const ChartKind kind = kindNamed(name, &ok);
        if (!ok) continue;
        w.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
        if (w.insertChart(kind)) ++made;
    }
    if (!saveWindow(w, outPath)) {
        out() << in << "\t2\n";
        return 2;
    }
    out() << in << "\t0\t" << made << "\n";
    return 0;
}

void grabTo(QWidget *widget, const QString &path)
{
    QCoreApplication::processEvents();
    widget->grab().save(path);
}

int shots(const QString &in, const QString &dir)
{
    QDir().mkpath(dir);
    CrunchWindow w;
    w.resize(1280, 800);
    w.show();
    if (!openWindow(w, in)) return 1;
    waitFor([&] { return w.objects() && w.objects()->shownSheet() == 0; }, 20);
    w.view()->selectRange(0, 0, 4, 3);
    // the Insert Chart dialog
    {
        std::unique_ptr<QDialog> d(w.makeInsertChartDialog());
        if (d) {
            d->show();
            auto *dialog = static_cast<InsertChartDialog *>(d.get());
            dialog->selectKind(ChartKind::LineMarkers);
            grabTo(d.get(), dir + QStringLiteral("/insert-chart.png"));
        }
    }
    if (!w.insertChart(ChartKind::Column)) return 3;
    w.objects()->layer()->prepareAll();
    // the chart picked, with its handles and the buttons beside it
    waitFor([&] { return w.chartElementsButton() && w.chartElementsButton()->isVisible(); }, 5);
    grabTo(&w, dir + QStringLiteral("/chart-picked.png"));
    // the elements list
    if (QToolButton *plus = w.chartElementsButton()) {
        QMenu *menu = plus->menu();
        if (menu) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            menu->popup(plus->mapToGlobal(QPoint(plus->width() + 4, 0)));
            QCoreApplication::processEvents();
            grabTo(menu, dir + QStringLiteral("/chart-elements.png"));
            menu->hide();
        }
    }
    if (std::unique_ptr<QDialog> d{w.makeSelectDataDialog()}) {
        d->show();
        grabTo(d.get(), dir + QStringLiteral("/select-data.png"));
    }
    if (std::unique_ptr<QDialog> d{w.makeChangeTypeDialog()}) {
        d->show();
        grabTo(d.get(), dir + QStringLiteral("/change-chart-type.png"));
    }
    {
        std::unique_ptr<QMenu> menu(w.buildObjectMenu(w.pickedObject(), &w));
        menu->popup(QPoint(300, 200));
        QCoreApplication::processEvents();
        grabTo(menu.get(), dir + QStringLiteral("/object-menu.png"));
    }
    out() << "shots in " << dir << "\n";
    return 0;
}

} // namespace

namespace jp::crunchtool {

int chartsMain(const QStringList &args)
{
    if (args.size() == 3 && args[0] == QLatin1String("charts-roundtrip")) return roundtrip(args[1], args[2]);
    if (args.size() >= 6 && args[0] == QLatin1String("charts-insert")) return insert(args[1], args[2], args[3].toInt() - 1, args[4], args.mid(5));
    if (args.size() == 3 && args[0] == QLatin1String("charts-shots")) return shots(args[1], args[2]);
    return -1;
}

} // namespace jp::crunchtool
