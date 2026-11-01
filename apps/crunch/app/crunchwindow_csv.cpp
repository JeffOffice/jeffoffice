// Saving the values of the sheet in view as a .csv file: the person is told what is written (only this sheet's values, as the cells show them), the file is written a few milliseconds at a
// time on the window's thread from a model of its own (so that changing sheets meanwhile changes nothing), and nothing can be edited until it is done. The workbook itself is not
// changed by it and is not saved by it.

#include "app/crunchwindow.h"

#include "app/otherformats.h"
#include "app/oldformat.h"
#include "core/appinfo.h"
#include "app/csvsavedialog.h"
#include "core/csvexport.h"
#include "sheet/csv.h"

#include <QElapsedTimer>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QFileInfo>
#include <QLocale>
#include <QTimer>

namespace jp {

CrunchWindow::LossAnswer CrunchWindow::askBeforeLosing(const QString &title, const QString &text, const QString &goLabel, const QString &otherLabel, bool defaultIsOther)
{
    if (m_lossAsker) return m_lossAsker(title, text);
    QMessageBox box(QMessageBox::Warning, appInfo().name, title, QMessageBox::NoButton, this);
    box.setObjectName(QStringLiteral("lossBox"));
    box.setText(text);
    box.setTextFormat(Qt::PlainText);
    QPushButton *go = box.addButton(goLabel.isEmpty() ? tr("Save Values Only") : goLabel, QMessageBox::AcceptRole);
    QPushButton *other = nullptr;
    if (goLabel.isEmpty() || !otherLabel.isEmpty()) other = box.addButton(otherLabel.isEmpty() ? tr("Save as .xlsx…") : otherLabel, QMessageBox::ActionRole);
    QPushButton *cancel = box.addButton(tr("Cancel"), QMessageBox::RejectRole);
    box.setDefaultButton(defaultIsOther && other ? other : go);
    box.setEscapeButton(cancel);
    box.exec();
    QAbstractButton *clicked = box.clickedButton();
    return clicked == go ? LossAnswer::Continue : (other && clicked == other) ? LossAnswer::SaveXlsx : LossAnswer::Cancel;
}

// How the sheet is written to a name: by its ending (a .csv with the person's separator, a .tsv or .txt with tabs), or, for a workbook that came from a file of values with the same ending, in
// the form that file had (its separator, line ends, and encoding).
crunch::CsvWrite CrunchWindow::csvWriteFor(const QString &path) const
{
    crunch::CsvWrite how;
    how.separator = crunch::csvSeparatorFor(QLocale());
    const QString ending = QFileInfo(path).suffix().toLower();
    if (ending != QLatin1String("csv")) how.separator = QLatin1Char('\t');
    const auto *csv = m_loaded && m_loaded->book ? dynamic_cast<const jp::office::sheet::CsvBook *>(m_loaded->book->foreignBook()) : nullptr;
    if (csv && !m_originalPath.isEmpty() && QFileInfo(m_originalPath).suffix().toLower() == ending) {
        using jp::office::sheet::CsvEncoding;
        const jp::office::sheet::CsvForm &form = csv->form();
        how.separator = form.delimiter;
        how.lineEnd = form.lineEnd;
        how.encoding = form.encoding == CsvEncoding::Utf8 ? crunch::CsvWrite::Encoding::Utf8
                       : form.encoding == CsvEncoding::Utf16Le ? crunch::CsvWrite::Encoding::Utf16Le
                       : form.encoding == CsvEncoding::Utf16Be ? crunch::CsvWrite::Encoding::Utf16Be
                       : form.encoding == CsvEncoding::Legacy && crunch::csvCanWriteCodePage(form.codePage) ? crunch::CsvWrite::Encoding::CodePage
                                                                                                          : crunch::CsvWrite::Encoding::Utf8Mark;
        how.codePage = form.codePage;
        how.sameAsFile = true;
    }
    return how;
}

bool CrunchWindow::exportCsv(const QString &path, const crunch::CsvWrite *forced)
{
    if (!m_session || !m_loaded || !m_loaded->book || m_saving) return false;
    if (!commitEdit()) return false;
    if (m_loading || !m_haveCalc) {
        m_closeAfterSave = false;
        notice(tr("Save"), tr("The workbook is still being read. Save it when the status bar says Ready."));
        return false;
    }
    // the file the workbook was read from is never written, and the values of a sheet are not written over the workbook's own file
    if (isTheOriginal(path) || crunch::sameFile(path, m_path)) {
        m_closeAfterSave = false;
        notice(tr("Save"), crunch::originalNotWrittenNotice(QFileInfo(isTheOriginal(path) ? m_originalPath : m_path).fileName()));
        return false;
    }
    // the file has what the formulas are worth: a calculation that is going is waited for
    if (m_calculating) {
        m_saveAfterCalculation = path;
        setStatus(tr("Saving when the calculation is done…"));
        return true;
    }
    crunch::CsvWrite how = forced ? *forced : csvWriteFor(path);
    const QString sheetName = m_loaded->book->info().sheets.value(m_sheet).name;
    // After a save (or a failure) the window is told the same way, whichever way the file was made.
    auto finish = [this, path](bool ok, const QString &message, qint64 replaced) {
        if (ok) {
            QString status = tr("Saved the values of %1 in %2").arg(m_loaded ? m_loaded->book->info().sheets.value(m_sheet).name : QString(), QFileInfo(path).fileName());
            if (replaced > 0) status += QLatin1Char(' ') + tr("(%1 characters the encoding has no letter for were written as \"?\")").arg(replaced);
            setStatus(status);
        } else {
            notice(tr("Save"), tr("%1 couldn't save \"%2\".\n%3\nThe file that was there is as it was.").arg(appInfo().name, QFileInfo(path).fileName(), message));
            setStatus(tr("Ready"));
        }
        Q_EMIT csvExported(path, ok);
        // (the window was being closed: a .csv written from that question was the answer to it, and the window closes; one that failed leaves the window as it is, and the close is given up)
        if (m_closeAfterSave) {
            m_closeAfterSave = false;
            if (ok) {
                m_closing = true;
                close();
                m_closing = false;
            }
        }
    };
    // (a sheet that was read only in part is written in part: the person is told)
    const bool partial = m_loaded->calc && m_loaded->calc->notWhole().contains(sheetName);
    // (the person is told what the file will not have, naming what this workbook has of it, and may choose how it is written, or save the workbook as .xlsx instead)
    const QString explanation = crunch::csvExportWarning(sheetName, how.separator, partial, crunch::leftOutOfCsv(*m_loaded->book, m_loaded->calc ? m_loaded->calc->formulaCount() : 0));
    LossAnswer answer = LossAnswer::Continue;
    if (forced || m_lossAsker) {
        answer = askBeforeLosing(tr("Save"), explanation);
    } else {
        CsvSaveDialog dialog(explanation, how, this);
        dialog.exec();
        answer = dialog.answer() == CsvSaveDialog::Answer::SaveValues ? LossAnswer::Continue : dialog.answer() == CsvSaveDialog::Answer::SaveXlsx ? LossAnswer::SaveXlsx : LossAnswer::Cancel;
        if (answer == LossAnswer::Continue) how = dialog.options();
    }
    if (answer != LossAnswer::Continue) {
        if (answer == LossAnswer::SaveXlsx) {
            // (a window being closed is closed once the workbook is saved; if no save was started, the person gave it up, and the window stays)
            const bool closing = m_closeAfterSave;
            setStatus(tr("Not saved as .csv."));
            act("file.saveAs")->trigger();
            m_closeAfterSave = closing && (m_saving || !m_saveAfterCalculation.isNull());
        } else {
            m_closeAfterSave = false;
        }
        return false;
    }
    // A workbook read from a file of values and not changed, saved with the choices that are the file's own, is written as the file's own bytes; any other choice is written as asked.
    const auto *csv = m_loaded->book->foreignBook() ? dynamic_cast<const jp::office::sheet::CsvBook *>(m_loaded->book->foreignBook()) : nullptr;
    if (csv && how.sameAsFile && !m_session->isModified() && m_sheet == 0) {
        QSaveFile file(path);
        const bool ok = file.open(QIODevice::WriteOnly) && file.write(csv->bytes()) == csv->bytes().size() && file.commit();
        finish(ok, file.errorString(), 0);
        return ok;
    }
    // the sheet in view, in a model of its own
    auto model = std::make_shared<grid::GridSheet>();
    model->setData(m_view->sheet().data());
    auto exporter = std::make_shared<crunch::CsvExport>(model.get(), how);
    QString why;
    if (!exporter->begin(path, &why)) {
        m_closeAfterSave = false;
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
    connect(timer, &QTimer::timeout, run, [this, run, timer, model, exporter, finish] {
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
        finish(ok, message, ok ? exporter->replaced() : 0);
    });
    timer->start(0);
    return true;
}

} // namespace jp
