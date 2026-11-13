#pragma once
// The Translate pane: the text that is selected (or the paragraph the cursor is in), translated on this computer by the models in Translation/models, shown beside it, with
// Replace and Insert Below. The same in JeffWrite and JeffPub; each program says where the text is and how it is put in (a TranslateHost). The models are downloaded the first
// time a pair is used, after a click on Download that says how big they are; nothing is sent anywhere but the model files' own addresses, and no text ever leaves the computer.
// A pair the models do not cover is offered through the website (the old way) instead.

#include "translate/downloader.h"
#include "translate/runmap.h"
#include "translate/translator.h"

#include <QWidget>
#include <functional>
#include <memory>

class QComboBox;
class QLabel;
class QNetworkAccessManager;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QTextBrowser;

namespace jp {

// What came of putting the translation in.
enum class TranslateApplied { Done, Changed /* the document no longer holds what was read */, Refused /* the program said why (a read-only document) */ };

class TranslateHost {
public:
    virtual ~TranslateHost();
    // What is to be translated now: the selection, or the paragraph the cursor is in; empty when there is none.
    virtual translate::Source source() = 0;
    // Puts the translation in the document, as one step that undoes; Changed, with nothing changed, when the document no longer holds what was read.
    virtual TranslateApplied replace(const translate::Source &source, const translate::Result &result);
    virtual TranslateApplied insertBelow(const translate::Source &source, const translate::Result &result);
    // The language the text is in, as a code of the pane's pickers (the proofing language of the text), or empty when it is not known.
    virtual QString sourceLanguage() { return QString(); }
    // The text for the website path (a language no model covers).
    virtual void openInBrowser(const QString &text, const QString &targetCode) = 0;

protected:
    // How the text goes into the document: the program's own edits where it has them.
    virtual translate::EditHooks &hooks();
};

// A host made of functions, for a program that has the pieces at hand (a window with its editor).
class FunctionHost : public TranslateHost {
public:
    std::function<translate::Source()> sourceFn;
    std::function<QString()> languageFn;
    std::function<void(const QString &text, const QString &targetCode)> browserFn;
    std::function<TranslateApplied(const translate::Source &, const translate::Result &)> replaceFn, insertBelowFn;   // (the program's own way to make the change one step that undoes)
    translate::Source source() override { return sourceFn ? sourceFn() : translate::Source(); }
    QString sourceLanguage() override { return languageFn ? languageFn() : QString(); }
    void openInBrowser(const QString &text, const QString &targetCode) override { if (browserFn) browserFn(text, targetCode); }
    TranslateApplied replace(const translate::Source &s, const translate::Result &r) override { return replaceFn ? replaceFn(s, r) : TranslateHost::replace(s, r); }
    TranslateApplied insertBelow(const translate::Source &s, const translate::Result &r) override { return insertBelowFn ? insertBelowFn(s, r) : TranslateHost::insertBelow(s, r); }
};

class TranslatePane : public QWidget {
    Q_OBJECT
public:
    // `host` outlives the pane. `net` fetches the models.
    TranslatePane(TranslateHost *host, const translate::Translator &translator, QNetworkAccessManager *net, QWidget *parent = nullptr);
    ~TranslatePane() override;

    // The pane the programs use: the built-in catalog and engine, the models in the data folder, and the program's network manager.
    static TranslatePane *create(TranslateHost *host, QWidget *parent = nullptr);

    // Reads the text again from the host (the pane does it when it is shown; and, while it is showing, when the person asks).
    Q_INVOKABLE void refresh();
    // Starts a translation of what is shown (the Translate button).
    void translateNow();

Q_SIGNALS:
    // The text was put in the document.
    void applied();

private:
    void setBusy(bool busy);
    void showMessage(const QString &text);
    void updateButtons();
    void startDownload();
    void startJob();
    void jobFinished(bool ok, bool canceled, const QStringList &out, const QString &error);
    void cancelAll();
    void swapLanguages();
    void apply(bool below);
    QString from() const;
    QString to() const;

    TranslateHost *m_host;
    translate::Translator m_translator;
    QNetworkAccessManager *m_net;
    QComboBox *m_from, *m_to;
    QPushButton *m_swap, *m_use, *m_run, *m_cancel, *m_replace, *m_below, *m_download, *m_browser;
    QPlainTextEdit *m_original;
    QTextBrowser *m_result;
    QLabel *m_status, *m_note;
    QProgressBar *m_progress;
    translate::Source m_source;
    QVector<translate::Piece> m_pieces;
    translate::Result m_translation;
    bool m_hasResult = false;
    bool m_busy = false;
    translate::Downloader *m_downloader = nullptr;
    translate::TranslateJob *m_job = nullptr;
};

} // namespace jp
