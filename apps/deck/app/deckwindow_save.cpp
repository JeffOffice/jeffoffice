// Saving presentations in JeffDeck's window: File > Save and Save As.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/notespane.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "io/pptxexport.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStatusBar>

namespace jp {

namespace {

bool isKnownEnding(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QLatin1String("pptx") || suffix == QLatin1String("potx") || suffix == QLatin1String("ppsx");
}

} // namespace

void DeckWindow::createSaveActions()
{
    mk("file.save", tr("Save"), "save", QKeySequence::Save, [this] { save(); });
    mk("file.saveAs", tr("Save As"), "save-all", QKeySequence(Qt::Key_F12), [this] { saveAs(); });

    // A template opened from a file is not this window's own.
    connect(m_ed, &Editor::documentReplaced, this, [this] { m_templatePath.clear(); });
}

bool DeckWindow::save()
{
    const QString path = m_ed->filePath();
    // A presentation opened from a template is saved as a new presentation:
    // the template stays as it is, unless this window wrote it.
    const bool isTemplate = deck::kindOfFileName(path) == deck::PresentationKind::Template && path != m_templatePath;
    if (path.isEmpty() || isTemplate) return saveAs();
    return saveTo(path);
}

bool DeckWindow::saveAs()
{
    const QString from = m_ed->filePath();
    const QString dir = from.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(from).absolutePath();
    const QString name = from.isEmpty() ? m_ed->displayName() : QFileInfo(from).completeBaseName();
    // The dialog hands back the filter text it showed, so the choice is
    // recognized by comparing with these same (translated) texts.
    const QString pptxFilter = tr("Presentation (*.pptx)"), potxFilter = tr("Template (*.potx)"), ppsxFilter = tr("Show (*.ppsx)");
    const deck::PresentationKind was = deck::kindOfFileName(from);
    QString selected = was == deck::PresentationKind::Show ? ppsxFilter : was == deck::PresentationKind::Template && from == m_templatePath ? potxFilter : pptxFilter;
    const QString ending = selected == ppsxFilter ? QStringLiteral(".ppsx") : selected == potxFilter ? QStringLiteral(".potx") : QStringLiteral(".pptx");
    QString path = askSavePath(this, tr("Save As"), dir + QLatin1Char('/') + name + ending, pptxFilter + QStringLiteral(";;") + potxFilter + QStringLiteral(";;") + ppsxFilter, &selected);
    if (path.isEmpty()) return false;
    if (!isKnownEnding(path)) path += selected == ppsxFilter ? QStringLiteral(".ppsx") : selected == potxFilter ? QStringLiteral(".potx") : QStringLiteral(".pptx");
    return saveTo(path);
}

bool DeckWindow::saveTo(const QString &path)
{
    // What was typed in the notes or a slide's text goes into the file.
    m_notes->commit();
    m_ed->endTextEdit();
    Document *doc = m_ed->doc();
    doc->props.modified = QDateTime::currentDateTime();
    if (doc->props.author.isEmpty()) doc->props.author = Settings::get().userName();
    QString err;
    QStringList skipped;
    if (!deck::savePresentationFile(*doc, path, &err, &skipped)) {
        QMessageBox::warning(this, tr("Save"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    m_ed->setFilePath(path);
    m_ed->markSaved();
    m_templatePath = deck::kindOfFileName(path) == deck::PresentationKind::Template ? path : QString();
    Settings::get().addRecentFile(path);
    updateTitle();
    if (skipped.isEmpty()) statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(path).fileName()), 4000);
    else statusBar()->showMessage(tr("Saved %1, but %n object(s) can't be saved in this kind of file yet.", nullptr, int(skipped.size())).arg(QFileInfo(path).fileName()), 8000);
    return true;
}

} // namespace jp
