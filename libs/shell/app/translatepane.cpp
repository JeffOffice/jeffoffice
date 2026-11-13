#include "app/translatepane.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "translate/languages.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

TranslateHost::~TranslateHost() = default;

translate::EditHooks &TranslateHost::hooks()
{
    static translate::EditHooks plain;
    return plain;
}

TranslateApplied TranslateHost::replace(const translate::Source &source, const translate::Result &result) { return translate::applyReplace(source, result, hooks()) ? TranslateApplied::Done : TranslateApplied::Changed; }

TranslateApplied TranslateHost::insertBelow(const translate::Source &source, const translate::Result &result) { return translate::applyInsertBelow(source, result, hooks()) ? TranslateApplied::Done : TranslateApplied::Changed; }

namespace {

QString sizeText(qint64 bytes) { return QLocale().formattedDataSize(bytes, 0, QLocale::DataSizeSIFormat); }

} // namespace

TranslatePane::TranslatePane(TranslateHost *host, const translate::Translator &translator, QNetworkAccessManager *net, QWidget *parent)
    : QWidget(parent), m_host(host), m_translator(translator), m_net(net)
{
    setObjectName(QStringLiteral("translatePane"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);

    auto *row = new QHBoxLayout;
    auto *fromBox = new QVBoxLayout;
    auto *fromLabel = new QLabel(tr("From:"), this);
    m_from = new QComboBox(this);
    m_from->setObjectName(QStringLiteral("translateFrom"));
    fromLabel->setBuddy(m_from);
    fromBox->addWidget(fromLabel);
    fromBox->addWidget(m_from);
    auto *toBox = new QVBoxLayout;
    auto *toLabel = new QLabel(tr("To:"), this);
    m_to = new QComboBox(this);
    m_to->setObjectName(QStringLiteral("translateTo"));
    toLabel->setBuddy(m_to);
    toBox->addWidget(toLabel);
    toBox->addWidget(m_to);
    m_swap = new QPushButton(tr("Swap"), this);
    m_swap->setObjectName(QStringLiteral("translateSwap"));
    m_swap->setToolTip(tr("Swap the languages"));
    row->addLayout(fromBox, 1);
    row->addWidget(m_swap, 0, Qt::AlignBottom);
    row->addLayout(toBox, 1);
    v->addLayout(row);

    const QVector<translate::Language> langs = translate::languages();
    for (const translate::Language &l : langs) {
        m_from->addItem(l.name, l.code);
        m_to->addItem(l.name, l.code);
    }
    const QString lastFrom = Settings::get().value(QStringLiteral("translate/pane/from"), QStringLiteral("en")).toString();
    const QString lastTo = Settings::get().value(QStringLiteral("translate/pane/to"), QStringLiteral("es")).toString();
    m_from->setCurrentIndex(std::max(0, m_from->findData(lastFrom)));
    m_to->setCurrentIndex(std::max(0, m_to->findData(lastTo)));

    auto *originalLabel = new QLabel(tr("Original:"), this);
    m_original = new QPlainTextEdit(this);
    m_original->setObjectName(QStringLiteral("translateOriginal"));
    m_original->setReadOnly(true);
    m_original->setAccessibleName(tr("Original text"));
    originalLabel->setBuddy(m_original);
    m_use = new QPushButton(tr("Use Selection"), this);
    m_use->setObjectName(QStringLiteral("translateUse"));
    m_use->setToolTip(tr("Read the selected text, or the paragraph the cursor is in, again"));
    v->addWidget(originalLabel);
    v->addWidget(m_original, 1);
    v->addWidget(m_use);

    auto *buttons = new QHBoxLayout;
    m_run = new QPushButton(tr("Translate"), this);
    m_run->setObjectName(QStringLiteral("translateRun"));
    m_run->setDefault(true);
    m_cancel = new QPushButton(tr("Cancel"), this);
    m_cancel->setObjectName(QStringLiteral("translateCancel"));
    m_cancel->hide();
    buttons->addWidget(m_run);
    buttons->addWidget(m_cancel);
    v->addLayout(buttons);

    m_status = plainLabel(new QLabel(this));
    m_status->setObjectName(QStringLiteral("translateStatus"));
    m_status->setWordWrap(true);
    m_status->hide();
    v->addWidget(m_status);
    m_note = plainLabel(new QLabel(this));
    m_note->setObjectName(QStringLiteral("translateDownloadNote"));
    m_note->setWordWrap(true);
    m_note->hide();
    v->addWidget(m_note);
    m_download = new QPushButton(this);
    m_download->setObjectName(QStringLiteral("translateDownload"));
    m_download->hide();
    v->addWidget(m_download);
    m_browser = new QPushButton(tr("Open in Your Browser Instead"), this);
    m_browser->setObjectName(QStringLiteral("translateBrowser"));
    m_browser->setToolTip(tr("Opens a translation website with the text. Nothing is sent until the page opens."));
    m_browser->hide();
    v->addWidget(m_browser);
    m_progress = new QProgressBar(this);
    m_progress->setObjectName(QStringLiteral("translateProgress"));
    m_progress->setRange(0, 100);
    m_progress->hide();
    v->addWidget(m_progress);

    auto *resultLabel = new QLabel(tr("Translation:"), this);
    m_result = new QTextBrowser(this);
    m_result->setObjectName(QStringLiteral("translateResult"));
    m_result->setOpenLinks(false);
    m_result->setAccessibleName(tr("Translation"));
    resultLabel->setBuddy(m_result);
    v->addWidget(resultLabel);
    v->addWidget(m_result, 1);

    auto *put = new QHBoxLayout;
    m_replace = new QPushButton(tr("Replace"), this);
    m_replace->setObjectName(QStringLiteral("translateReplace"));
    m_replace->setToolTip(tr("Put the translation in the place of the original text"));
    m_below = new QPushButton(tr("Insert Below"), this);
    m_below->setObjectName(QStringLiteral("translateInsertBelow"));
    m_below->setToolTip(tr("Put the translation in new paragraphs below the original text"));
    put->addWidget(m_replace);
    put->addWidget(m_below);
    v->addLayout(put);
    auto *privacy = new QLabel(tr("The translation is made on this computer. Your text is not sent anywhere."), this);
    privacy->setWordWrap(true);
    v->addWidget(privacy);
    setMinimumWidth(240);

    connect(m_use, &QPushButton::clicked, this, &TranslatePane::refresh);
    connect(m_run, &QPushButton::clicked, this, &TranslatePane::translateNow);
    connect(m_cancel, &QPushButton::clicked, this, &TranslatePane::cancelAll);
    connect(m_swap, &QPushButton::clicked, this, &TranslatePane::swapLanguages);
    connect(m_download, &QPushButton::clicked, this, &TranslatePane::startDownload);
    connect(m_replace, &QPushButton::clicked, this, [this] { apply(false); });
    connect(m_below, &QPushButton::clicked, this, [this] { apply(true); });
    connect(m_browser, &QPushButton::clicked, this, [this] { m_host->openInBrowser(m_source.plainText(), to()); });
    auto changed = [this] {
        Settings::get().setValue(QStringLiteral("translate/pane/from"), from());
        Settings::get().setValue(QStringLiteral("translate/pane/to"), to());
        m_hasResult = false;
        m_result->clear();
        m_note->hide();
        m_download->hide();
        m_browser->hide();
        m_status->hide();
        updateButtons();
    };
    connect(m_from, &QComboBox::currentIndexChanged, this, changed);
    connect(m_to, &QComboBox::currentIndexChanged, this, changed);
    refresh();
}

TranslatePane::~TranslatePane()
{
    if (m_downloader) m_downloader->cancel();
    delete m_job;
}

TranslatePane *TranslatePane::create(TranslateHost *host, QWidget *parent)
{
    static QNetworkAccessManager *net = new QNetworkAccessManager(qApp);
    const translate::Translator translator(translate::Catalog::builtIn(), translate::ModelStore(translate::ModelStore::defaultRoot()), translate::builtInBackend());
    return new TranslatePane(host, translator, net, parent);
}

QString TranslatePane::from() const { return m_from->currentData().toString(); }
QString TranslatePane::to() const { return m_to->currentData().toString(); }

void TranslatePane::refresh()
{
    if (m_busy) return;
    m_source = m_host->source();
    m_pieces = translate::pieces(m_source);
    m_original->setPlainText(m_source.plainText());
    m_hasResult = false;
    m_result->clear();
    m_note->hide();
    m_download->hide();
    m_browser->hide();
    // (a first look at a text in a language the pickers have: the text's own language is the language from)
    const QString lang = m_host->sourceLanguage();
    if (!lang.isEmpty() && !m_source.isEmpty() && m_from->findData(lang) >= 0 && m_from->currentData().toString() != lang) {
        const QSignalBlocker block(m_from);
        m_from->setCurrentIndex(m_from->findData(lang));
    }
    if (m_source.isEmpty()) showMessage(tr("Select the text you want to translate, then click Use Selection."));
    else m_status->hide();
    updateButtons();
}

void TranslatePane::showMessage(const QString &text)
{
    m_status->setText(text);
    m_status->setVisible(!text.isEmpty());
}

void TranslatePane::setBusy(bool busy)
{
    m_busy = busy;
    m_cancel->setVisible(busy);
    m_run->setVisible(!busy);
    m_from->setEnabled(!busy);
    m_to->setEnabled(!busy);
    m_swap->setEnabled(!busy);
    m_use->setEnabled(!busy);
    if (!busy) m_progress->hide();
    updateButtons();
}

void TranslatePane::updateButtons()
{
    m_run->setEnabled(!m_busy && !m_source.isEmpty());
    m_replace->setEnabled(!m_busy && m_hasResult);
    m_below->setEnabled(!m_busy && m_hasResult);
}

void TranslatePane::swapLanguages()
{
    const int f = m_from->currentIndex(), t = m_to->currentIndex();
    m_from->setCurrentIndex(t);
    m_to->setCurrentIndex(f);
}

void TranslatePane::translateNow()
{
    if (m_busy) return;
    m_source = m_host->source();
    m_pieces = translate::pieces(m_source);
    m_original->setPlainText(m_source.plainText());
    m_hasResult = false;
    m_result->clear();
    m_note->hide();
    m_download->hide();
    m_browser->hide();
    updateButtons();
    if (m_source.isEmpty() || m_pieces.isEmpty()) {
        showMessage(tr("There is no text to translate. Select some text, then click Use Selection."));
        return;
    }
    if (from() == to()) {
        showMessage(tr("Choose two different languages."));
        return;
    }
    const bool engine = m_translator.hasEngine();
    if (!engine || !m_translator.canTranslate(from(), to())) {
        // No model for this pair: the website is the way left, and the person chooses it.
        showMessage(engine ? tr("There is no translation model for %1 to %2. You can open a translation website with the text instead.").arg(translate::languageName(from()), translate::languageName(to()))
                           : tr("This copy of the program has no translator built in. You can open a translation website with the text instead."));
        m_browser->show();
        return;
    }
    const qint64 bytes = m_translator.downloadBytes(from(), to());
    if (bytes > 0) {
        // (The models are not installed: the download waits for a click.)
        QStringList names;
        for (const translate::ModelPair &p : m_translator.missing(from(), to())) {
            const QString other = p.from == QLatin1String(translate::kPivot) ? p.to : p.from;
            const QString name = translate::languageName(other);
            if (!names.contains(name)) names << name;
        }
        showMessage(tr("No translation model is installed for %1 yet.").arg(names.join(tr(" and "))));
        m_note->setText(tr("The models are downloaded once from Mozilla's translation files and kept on this computer. After that, translating needs no connection."));
        m_note->show();
        m_download->setText(tr("Download %1 (%2)?").arg(names.join(tr(" and ")), sizeText(bytes)));
        m_download->show();
        return;
    }
    startJob();
}

void TranslatePane::startDownload()
{
    if (m_busy) return;
    m_download->hide();
    m_note->hide();
    if (!m_downloader) {
        m_downloader = new translate::Downloader(m_translator.store(), m_net, this);
        connect(m_downloader, &translate::Downloader::progress, this, [this](qint64 done, qint64 total) {
            m_progress->setRange(0, 1000);
            m_progress->setValue(total > 0 ? int(done * 1000 / total) : 0);
            m_progress->setFormat(tr("%1 of %2").arg(sizeText(done), sizeText(total)));
        });
        connect(m_downloader, &translate::Downloader::finished, this, [this](bool ok, bool canceled, const QString &error) {
            setBusy(false);
            if (ok) {
                showMessage(QString());
                startJob();
            } else if (canceled) {
                showMessage(tr("The download was canceled. Nothing was installed."));
            } else {
                showMessage(error);
            }
        });
    }
    setBusy(true);
    showMessage(tr("Downloading the translation models..."));
    m_progress->setValue(0);
    m_progress->setFormat(QString());
    m_progress->show();
    m_downloader->start(m_translator.missing(from(), to()));
}

void TranslatePane::startJob()
{
    QStringList pieces;
    for (const translate::Piece &p : m_pieces) pieces << translate::pieceHtml(m_source, p);
    delete m_job;
    m_job = new translate::TranslateJob(m_translator, from(), to(), pieces, true, this);
    connect(m_job, &translate::TranslateJob::finished, this, &TranslatePane::jobFinished);
    setBusy(true);
    showMessage(tr("Translating..."));
    m_job->start();
}

void TranslatePane::jobFinished(bool ok, bool canceled, const QStringList &out, const QString &error)
{
    setBusy(false);
    if (canceled) {
        showMessage(tr("Canceled."));
        return;
    }
    if (!ok) {
        showMessage(error.isEmpty() ? tr("The translation failed.") : error);
        return;
    }
    m_translation = translate::rebuild(m_source, m_pieces, out);
    m_translation.language = to();
    m_result->setHtml(m_translation.html());
    m_hasResult = true;
    showMessage(QString());
    updateButtons();
}

void TranslatePane::cancelAll()
{
    if (m_downloader && m_downloader->isRunning()) m_downloader->cancel();
    if (m_job && m_job->isRunning()) m_job->cancel();
}

void TranslatePane::apply(bool below)
{
    if (!m_hasResult) return;
    const TranslateApplied done = below ? m_host->insertBelow(m_source, m_translation) : m_host->replace(m_source, m_translation);
    if (done == TranslateApplied::Refused) return;
    if (done == TranslateApplied::Changed) {
        m_hasResult = false;
        updateButtons();
        showMessage(tr("The text has changed since it was read. Click Use Selection, then Translate again."));
        return;
    }
    Q_EMIT applied();
    // (the document is not what was read now: the next translation reads it again)
    m_hasResult = false;
    refresh();
    showMessage(below ? tr("The translation was put in below the text.") : tr("The text was replaced by its translation."));
}

} // namespace jp
