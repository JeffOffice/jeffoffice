// Saving the values of the sheet in view as a .csv file: the person is told what is written (only this sheet's values, as the cells show them), the file is written a few milliseconds at a
// time on the window's thread from a model of its own (so that changing sheets meanwhile changes nothing), and nothing can be edited until it is done. The workbook itself is not
// changed by it and is not saved by it.

#include "app/crunchwindow.h"

#include "app/otherformats.h"
#include "app/oldformat.h"
#include "core/appinfo.h"
#include "core/csvexport.h"

#include <QElapsedTimer>
#include <QFileInfo>
#include <QLocale>
#include <QTimer>

namespace jp {

bool CrunchWindow::exportCsv(const QString &path)
{
    if (!m_session || !m_loaded || !m_loaded->book || m_saving) return false;
    if (!commitEdit()) return false;
    if (m_loading || !m_haveCalc) {
        notice(tr("Save"), tr("The workbook is still being read. Save it when the status bar says Ready."));
        return false;
    }
    // the file the workbook was read from is never written
    if (QFileInfo(path).absoluteFilePath() == QFileInfo(m_path).absoluteFilePath()) {
        notice(tr("Save"), crunch::originalNotWrittenNotice(QFileInfo(m_path).fileName()));
        return false;
    }
    // the file has what the formulas are worth: a calculation that is going is waited for
    if (m_calculating) {
        m_saveAfterCalculation = path;
        setStatus(tr("Saving when the calculation is done…"));
        return true;
    }
    const QChar separator = crunch::csvSeparatorFor(QLocale());
    const QString sheetName = m_loaded->book->info().sheets.value(m_sheet).name;
    notice(tr("Save"), crunch::csvExportNote(sheetName, separator), false);
    // the sheet in view, in a model of its own
    auto model = std::make_shared<grid::GridSheet>();
    model->setData(m_view->sheet().data());
    auto exporter = std::make_shared<crunch::CsvExport>(model.get(), separator);
    QString why;
    if (!exporter->begin(path, &why)) {
        notice(tr("Save"), tr("%1 couldn't save \"%2\".\n%3\nThe file that was there is as it was.").arg(appInfo().name, QFileInfo(path).fileName(), why));
        return false;
    }
    m_saving = true;
    m_session->setBusy(true);
    updateEditActions();
    setStatus(tr("Saving %1…").arg(QFileInfo(path).fileName()));
    auto *run = new QObject(this);
    m_csvRun = run;
    auto *timer = new QTimer(run);
    connect(timer, &QTimer::timeout, run, [this, run, timer, model, exporter, path] {
        QElapsedTimer clock;
        clock.start();
        bool done = false;
        while (!done && clock.elapsed() < 4) done = exporter->step(20000);
        if (!done) return;
        timer->stop();
        QString message;
        bool ok = !exporter->failed() && exporter->commit(&message);
        if (exporter->failed()) message = exporter->error();
        if (!ok) exporter->cancel();
        m_saving = false;
        if (m_session) m_session->setBusy(false);
        m_csvRun = nullptr;
        run->deleteLater();
        updateEditActions();
        if (ok) {
            setStatus(tr("Saved the values of %1 in %2").arg(m_loaded ? m_loaded->book->info().sheets.value(m_sheet).name : QString(), QFileInfo(path).fileName()));
        } else {
            notice(tr("Save"), tr("%1 couldn't save \"%2\".\n%3\nThe file that was there is as it was.").arg(appInfo().name, QFileInfo(path).fileName(), message));
            setStatus(tr("Ready"));
        }
        Q_EMIT csvExported(path, ok);
        if (ok && m_closeAfterSave) {
            m_closeAfterSave = false;
            close();
        }
    });
    timer->start(0);
    return true;
}

} // namespace jp
