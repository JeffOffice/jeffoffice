// JeffDeck's Slides from Outline: the slides a text, Markdown, rich text, or .docx outline makes (in this presentation after the current slide, or as a new
// one when the file is opened), and Save a Copy as a Markdown outline.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/settings.h"
#include "core/deck.h"
#include "io/outlineimport.h"
#include "core/appinfo.h"

#include "core/workthread.h"

#include <QAction>
#include <QApplication>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTimer>
#include <atomic>

namespace jp {

namespace {

// Reads the file on a thread of its own, with the wait cursor and the event loop going (the window's input is not let through): a big outline takes a while.
bool readOutlineBusy(const QString &path, deck::Outline *outline, QString *error)
{
    std::atomic<bool> done{false};
    bool ok = false;
    QEventLoop loop;
    QTimer ticker;
    ticker.setInterval(30);
    QObject::connect(&ticker, &QTimer::timeout, &loop, [&] {
        if (done.load()) loop.quit();
    });
    QApplication::setOverrideCursor(Qt::WaitCursor);
    WorkThread worker([&] {
        ok = deck::readOutlineFile(path, outline, error);
        done.store(true);
    });
    if (worker.started()) {
        ticker.start();
        if (!done.load()) loop.exec(QEventLoop::ExcludeUserInputEvents);
    }
    worker.join();
    QApplication::restoreOverrideCursor();
    return ok;
}

} // namespace

void DeckWindow::createOutlineActions()
{
    mk("slide.fromOutline", tr("Slides from Outline…"), "list-tree", QKeySequence(), [this] {
        if (isReadOnly()) {
            refuseReadOnlyChange();
            return;
        }
        const QString dir = Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        const QString path = QFileDialog::getOpenFileName(this, tr("Slides from Outline"), dir,
                                                          tr("Outlines (*.txt *.text *.md *.markdown *.rtf *.docx)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (path.isEmpty()) return;
        Settings::get().setValue(QStringLiteral("dirs/open"), QFileInfo(path).absolutePath());
        insertOutlineFile(path);
    });
}

bool DeckWindow::insertOutlineFile(const QString &path)
{
    if (isReadOnly()) {
        refuseReadOnlyChange();
        return false;
    }
    deck::Outline outline;
    QString why;
    if (!readOutlineBusy(path, &outline, &why)) {
        notice(tr("Slides from Outline"), tr("\"%1\" can't make slides.\n%2").arg(QFileInfo(path).fileName(), why), true);
        return false;
    }
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Slides from Outline"));
    const int first = m_ed->currentPage() + 1;
    const int made = deck::insertOutline(*m_ed->doc(), first, outline);
    m_ed->endChange();
    if (made > 0) m_ed->setCurrentPage(first);
    if (!outline.notes.isEmpty()) notice(tr("Slides from Outline"), tr("Some of \"%1\" was left out.\n\n%2").arg(QFileInfo(path).fileName(), outline.notes.join(QLatin1Char('\n'))), true);
    else statusBar()->showMessage(tr("Added %n slide(s) from \"%1\".", nullptr, made).arg(QFileInfo(path).fileName()), 6000);
    return made > 0;
}

std::unique_ptr<Document> DeckWindow::openedOutline(const QString &path, QString *notes)
{
    deck::Outline outline;
    QString why;
    if (!readOutlineBusy(path, &outline, &why)) {
        *notes = why;
        return nullptr;
    }
    *notes = outline.notes.join(QLatin1Char('\n'));
    return deck::presentationFromOutline(outline);
}

bool DeckWindow::saveOutlineCopy(const QString &path)
{
    if (!m_ed->filePath().isEmpty() && sameFile(path, m_ed->filePath())) {
        notice(tr("Save"), tr("A copy can't be saved over \"%1\". Choose another name.").arg(QFileInfo(path).fileName()), true);
        return false;
    }
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    QStringList left;
    const QByteArray bytes = deck::outlineMarkdown(*m_ed->doc(), &left).toUtf8();
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size() || !f.commit()) {
        notice(tr("Save a Copy"), tr("%1 couldn't save \"%2\".").arg(appInfo().name, QFileInfo(path).fileName()), true);
        return false;
    }
    notice(tr("Save a Copy"), tr("Saved the titles and text of the slides in \"%1\". It does not hold:\n\n%2").arg(QFileInfo(path).fileName(), left.join(QLatin1Char('\n'))), false);
    return true;
}

} // namespace jp
