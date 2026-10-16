#pragma once
// JeffWrite's main window: the ribbon, the pages, and the status bar, with the File screen over them.

#include "app/appwindow.h"
#include "core/writedoc.h"

#include <QHash>
#include <QKeySequence>
#include <QTimer>
#include <functional>
#include <memory>

class QDockWidget;
class QLabel;
class QSlider;

namespace jp {

class ColorButton;
class FontCombo;
class HelpView;
class HelpWindow;
class MeasureSpin;
class Ribbon;
struct RibbonParts;
class SizeCombo;
class WriteFileScreen;
namespace write {
class PageView;
}
namespace recovery {
struct Recovered;
}

// What Print and Export to PDF are asked to do: which pages, and how many copies (the printer's dialog asks for more).
struct WriteOutputOptions {
    enum class Range { All, Current, Pages };
    Range range = Range::All;
    QString pages;        // "1-3, 5": for Range::Pages
};

class WriteWindow : public AppWindow {
    Q_OBJECT
public:
    explicit WriteWindow(QWidget *parent = nullptr);
    ~WriteWindow() override;

    QAction *act(const QString &id) const override;
    QStringList actionIds() const override;
    Ribbon *ribbon() const override { return m_ribbon; }
    write::PageView *view() const { return m_view; }
    write::WriteDoc *document() const { return m_doc.get(); }
    QString ribbonErrors() const { return m_ribbonErrors; }

    // ---- the file
    // The name the title shows: the file's, or "Document1" for one never saved.
    QString displayName() const;
    QString filePath() const { return m_path; }
    bool isModified() const;
    // Replaces the document shown (as a new window's blank one, or one just read); the path is "" for an unsaved one.
    void setDocument(std::unique_ptr<write::WriteDoc> doc, const QString &path);
    // Opens a .docx in this window if it holds a new document nobody has touched, else in a window of its own. False, with the reason
    // shown, if the file can't be read.
    bool openFile(const QString &path);
    // Saves to the file the document came from, else asks where (Save As). False if nothing was written.
    bool save();
    bool saveAs();
    // Writes the file; the document is saved and takes the file's name. On failure says why and leaves it unsaved.
    bool saveTo(const QString &path);
    // What Save As asks for a name with: the file dialog, unless this stands in for it (tests): the name to suggest and the filter to
    // start on in, the path chosen out (empty for none).
    void setSavePathChooser(const std::function<QString(const QString &suggested, const QString &filter)> &chooser) { m_pathChooser = chooser; }
    // AutoRecover: a copy of unsaved work (the timer writes it at the interval Options sets), and the copies a run that crashed left.
    void autoRecover();
    QString recoveryCopy() const { return m_recoveryCopy; }
    void offerRecovery(bool askIfNone = false);
    WriteWindow *openRecovered(const recovery::Recovered &r, QString *error = nullptr);

    // The File screen over the window, on a page ("new", "open", "print", "export", "about"; Open when none is named), and back.
    void showFileScreen(const QString &page = QString());
    void hideFileScreen();
    bool printOutput(const WriteOutputOptions &options);
    bool exportPdf();
    // The pages that an output covers (from 0), for the options and the document.
    QVector<int> outputPages(const WriteOutputOptions &options);

    void showHelp(const QString &topic = QString());
    HelpView *helpView() const { return m_help; }

protected:
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QAction *mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn, bool checkable = false);
    void createActions();
    void createFormatActions();
    void createLayoutActions();
    void createViewActions();
    void createFileActions();
    void createHelpActions();
    void buildRibbon();
    void ribbonParts(RibbonParts &parts);
    void buildStatusBar();
    void refreshUi();
    void updateTitle();
    bool maybeSave();
    void dropRecoveryCopy();
    void changeSection(const std::function<void(flow::Section &)> &change);
    void applyIndentFields();
    void showParagraphDialog();
    void showZoomDialog();
    void showContextMenu(const QPoint &globalPos);
    QString helpContext() const;
    HelpWindow *showHelpOver(const QString &topic);
    void updateStatus();
    void updateWords();

    std::unique_ptr<write::WriteDoc> m_doc;
    write::PageView *m_view = nullptr;
    Ribbon *m_ribbon = nullptr;
    WriteFileScreen *m_fileScreen = nullptr;
    HelpView *m_help = nullptr;
    QDockWidget *m_helpDock = nullptr;
    QLabel *m_pageLabel = nullptr, *m_wordLabel = nullptr, *m_zoomLabel = nullptr;
    QSlider *m_zoomSlider = nullptr;
    QList<FontCombo *> m_fontCombos;
    QList<SizeCombo *> m_sizeCombos;
    QList<ColorButton *> m_colorButtons;
    QList<MeasureSpin *> m_indentFields;
    QWidget *m_styleGallery = nullptr;
    QHash<QString, QAction *> m_actions;
    QString m_ribbonErrors;
    QString m_path;
    QString m_untitled;
    QString m_recoveryCopy, m_recoveredFrom;
    QTimer m_refreshTimer, m_recoverTimer, m_wordTimer;
    quint64 m_serial = 0;
    bool m_updatingFields = false;
    std::function<QString(const QString &, const QString &)> m_pathChooser;
};

} // namespace jp
