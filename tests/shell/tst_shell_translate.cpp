// The Translate pane's parts (libs/translate, libs/shell/app/translatepane): the catalog of models, their download with a check of every file, pivoting through English, the
// runs of a text kept through a translation, Replace and Insert Below, a cancel, and the pane's messages. A stand-in for the engine (it marks the text it is given) means no model is
// needed; one more test runs the real engine when JO_TRANSLATE_MODEL_DIR names a folder with the English to German model files.
#include "app/translatepane.h"
#include "app/settings.h"
#include "translate/catalog.h"
#include "translate/downloader.h"
#include "translate/languages.h"
#include "translate/modelstore.h"
#include "translate/runmap.h"
#include "translate/translator.h"
#include "../isolatesettings.h"
#include "../testrunner.h"

#include <QComboBox>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QtTest>

#include <atomic>
#include <thread>

using namespace jp;
using namespace jp::translate;

namespace {

// Marks what it translates: "de>en:" before the piece, the tags and the words as they were. Blocks on `hold` until it is released or canceled.
class StubBackend : public Backend {
public:
    bool translatePieces(const ModelPair &pair, const ModelStore &, const QStringList &pieces, bool html, QStringList *out, QString *error, const std::atomic<bool> *cancel) override
    {
        Q_UNUSED(html)
        Q_UNUSED(error)
        ++calls;
        hops << pair.key();
        for (const QString &p : pieces) {
            while (hold && !(cancel && cancel->load())) std::this_thread::sleep_for(std::chrono::milliseconds(5));
            if (cancel && cancel->load()) return false;
            out->push_back(pair.key() + QLatin1Char(':') + p);
        }
        return true;
    }
    std::atomic<bool> hold{false};
    std::atomic<int> calls{0};
    QStringList hops;
};

QByteArray bytesOf(const char *seed, int n)
{
    QByteArray b;
    while (b.size() < n) b += seed;
    return b.left(n);
}

// A folder of model files for the pairs named, and the JSON catalog (file addresses) that lists them; `wrongHash` spoils one file's SHA-256.
struct Fixture {
    QTemporaryDir files, store;
    QByteArray json;
    QMap<QString, ModelPair> pairs;
    explicit Fixture(const QStringList &names = {QStringLiteral("en-de"), QStringLiteral("de-en"), QStringLiteral("en-fr"), QStringLiteral("fr-en")}, const QString &wrongHash = QString())
    {
        QJsonObject all;
        for (const QString &name : names) {
            QJsonObject fs;
            for (const char *role : {"model", "shortlist", "vocab"}) {
                const QByteArray content = bytesOf((name + QString::fromLatin1(role)).toUtf8().constData(), 3000 + int(QByteArray(role).size()) * 100);
                QFile f(files.filePath(name + QLatin1Char('.') + QString::fromLatin1(role)));
                f.open(QIODevice::WriteOnly);
                f.write(content);
                f.close();
                QString hash = QString::fromLatin1(QCryptographicHash::hash(content, QCryptographicHash::Sha256).toHex());
                if (wrongHash == name + QLatin1Char('/') + QString::fromLatin1(role)) hash = QString(64, QLatin1Char('0'));
                fs[QString::fromLatin1(role)] = QJsonObject{{QStringLiteral("url"), QUrl::fromLocalFile(f.fileName()).toString()}, {QStringLiteral("size"), content.size()}, {QStringLiteral("sha256"), hash}};
            }
            all[name] = QJsonObject{{QStringLiteral("files"), fs}};
        }
        json = QJsonDocument(QJsonObject{{QStringLiteral("baseUrl"), QStringLiteral("https://example.invalid/")}, {QStringLiteral("pairs"), all}}).toJson();
    }
    Catalog catalog() const
    {
        QString error;
        const Catalog c = Catalog::fromJson(json, &error, true);
        return c;
    }
};

// A document of two paragraphs; the pane's host over it.
class DocHost : public TranslateHost {
public:
    DocHost()
    {
        QTextCursor c(&doc);
        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);
        QTextCharFormat plain;
        c.insertText(QStringLiteral("Hello "), plain);
        c.insertText(QStringLiteral("big"), bold);
        c.insertText(QStringLiteral(" world."), plain);
        c.insertBlock();
        c.insertText(QStringLiteral("Second paragraph."), plain);
        doc.setUndoRedoEnabled(true);
    }
    Source source() override
    {
        QTextCursor c(&doc);
        c.setPosition(selectFrom);
        if (selectTo > selectFrom) c.setPosition(selectTo, QTextCursor::KeepAnchor);
        return readSource(c);
    }
    void openInBrowser(const QString &text, const QString &code) override
    {
        browserText = text;
        browserCode = code;
    }
    QString sourceLanguage() override { return language; }
    QTextDocument doc;
    int selectFrom = 0, selectTo = 0;
    QString browserText, browserCode, language;
};

} // namespace

class ShellTranslateTests : public QObject {
    Q_OBJECT

    static Translator translatorFor(const Fixture &fx, std::shared_ptr<Backend> backend) { return Translator(fx.catalog(), ModelStore(fx.store.path()), backend); }

    // Runs the downloader for the pairs named, until it is done.
    static bool install(const Fixture &fx, const QVector<ModelPair> &pairs, QString *error = nullptr)
    {
        QNetworkAccessManager net;
        Downloader d(ModelStore(fx.store.path()), &net);
        QSignalSpy done(&d, &Downloader::finished);
        d.start(pairs);
        if (done.isEmpty() && !done.wait(20000)) return false;
        if (error) *error = done.first().at(2).toString();
        return done.first().at(0).toBool();
    }

private Q_SLOTS:
    // ---- the catalog
    void theBuiltInCatalogCoversEveryLanguageBothWays()
    {
        const Catalog &c = Catalog::builtIn();
        QVERIFY(!c.isEmpty());
        for (const Language &l : languages()) {
            if (l.code == QLatin1String("en")) continue;
            QVERIFY2(c.hasPair(QStringLiteral("en"), l.code), qPrintable(l.code));
            QVERIFY2(c.hasPair(l.code, QStringLiteral("en")), qPrintable(l.code));
        }
        const ModelPair p = c.pair(QStringLiteral("en"), QStringLiteral("de"));
        QCOMPARE(p.files.size(), 3);
        for (const ModelFile &f : p.files) {
            QCOMPARE(f.url.scheme(), QStringLiteral("https"));
            QCOMPARE(f.sha256.size(), 64);
            QVERIFY(f.size > 100000);
        }
        // A pair with no model of its own goes through English, and English to German is one step.
        QCOMPARE(c.route(QStringLiteral("de"), QStringLiteral("fr")).size(), 2);
        QCOMPARE(c.route(QStringLiteral("en"), QStringLiteral("de")).size(), 1);
        QVERIFY(c.route(QStringLiteral("de"), QStringLiteral("de")).isEmpty());
    }

    void theBuiltInCatalogsHashesAreTheOnesRecordedForTheModelsWeTried()
    {
        // (the English to German model's SHA-256, as the engine's spike measured it on the file as served)
        const ModelPair p = Catalog::builtIn().pair(QStringLiteral("en"), QStringLiteral("de"));
        for (const ModelFile &f : p.files)
            if (f.role == QLatin1String("model")) {
                QCOMPARE(f.sha256, QStringLiteral("8df29d9494d19f47fd5d97c6a73474c6f657e9f81c1a607c431d02befdf3810f"));
                QCOMPARE(f.size, qint64(31561787));
            }
    }

    void aBadCatalogIsRefused()
    {
        Fixture fx({QStringLiteral("en-de")});
        QString error;
        QVERIFY(Catalog::fromJson(fx.json, &error, false).isEmpty());   // (a file address is refused unless a test allows it)
        QVERIFY(!error.isEmpty());
        QJsonObject root = QJsonDocument::fromJson(fx.json).object();
        QJsonObject pairs = root[QStringLiteral("pairs")].toObject();
        QJsonObject de = pairs[QStringLiteral("en-de")].toObject();
        QJsonObject files = de[QStringLiteral("files")].toObject();
        QJsonObject model = files[QStringLiteral("model")].toObject();
        model[QStringLiteral("url")] = QStringLiteral("http://example.invalid/model.bin");
        files[QStringLiteral("model")] = model;
        de[QStringLiteral("files")] = files;
        pairs[QStringLiteral("en-de")] = de;
        root[QStringLiteral("pairs")] = pairs;
        QVERIFY(Catalog::fromJson(QJsonDocument(root).toJson(), &error, true).isEmpty());
        QVERIFY(error.contains(QStringLiteral("secure")));
        model[QStringLiteral("url")] = QStringLiteral("https://example.invalid/model.bin");
        model[QStringLiteral("sha256")] = QStringLiteral("abc");
        files[QStringLiteral("model")] = model;
        de[QStringLiteral("files")] = files;
        pairs[QStringLiteral("en-de")] = de;
        root[QStringLiteral("pairs")] = pairs;
        QVERIFY(Catalog::fromJson(QJsonDocument(root).toJson(), &error, true).isEmpty());
        QVERIFY(error.contains(QStringLiteral("SHA-256")));
    }

    // ---- the download
    void aPairDownloadsIsCheckedAndInstalled()
    {
        Fixture fx;
        const ModelPair p = fx.catalog().pair(QStringLiteral("en"), QStringLiteral("de"));
        ModelStore store(fx.store.path());
        QVERIFY(!store.isInstalled(p));
        QNetworkAccessManager net;
        Downloader d(store, &net);
        QSignalSpy progress(&d, &Downloader::progress);
        QSignalSpy done(&d, &Downloader::finished);
        d.start({p});
        QTRY_VERIFY_WITH_TIMEOUT(!done.isEmpty(), 20000);
        QVERIFY(done.first().at(0).toBool());
        QVERIFY(done.first().at(2).toString().isEmpty());
        QVERIFY(store.isInstalled(p));
        QVERIFY(QFileInfo::exists(store.filePath(p, QStringLiteral("model"))));
        QVERIFY(!progress.isEmpty());
        QCOMPARE(progress.last().at(0).toLongLong(), p.totalBytes());
        QVERIFY(!QFileInfo::exists(store.partFolder(QStringLiteral("en"), QStringLiteral("de"))));
        // (what is installed is not fetched again)
        QSignalSpy again(&d, &Downloader::finished);
        d.start({p});
        QCOMPARE(again.count(), 1);
        QVERIFY(again.first().at(0).toBool());
    }

    void aFileWithTheWrongHashIsRejectedAndNothingIsInstalled()
    {
        Fixture fx({QStringLiteral("en-de")}, QStringLiteral("en-de/vocab"));
        const ModelPair p = fx.catalog().pair(QStringLiteral("en"), QStringLiteral("de"));
        QString error;
        QVERIFY(!install(fx, {p}, &error));
        QVERIFY2(error.contains(QStringLiteral("checksum")), qPrintable(error));
        ModelStore store(fx.store.path());
        QVERIFY(!store.isInstalled(p));
        QVERIFY(!QFileInfo::exists(store.folder(QStringLiteral("en"), QStringLiteral("de"))));
        QVERIFY(!QFileInfo::exists(store.partFolder(QStringLiteral("en"), QStringLiteral("de"))));
    }

    void aFileLargerThanTheCatalogSaysIsRefused()
    {
        Fixture fx({QStringLiteral("en-de")});
        QJsonObject root = QJsonDocument::fromJson(fx.json).object();
        QJsonObject pairs = root[QStringLiteral("pairs")].toObject();
        QJsonObject de = pairs[QStringLiteral("en-de")].toObject();
        QJsonObject files = de[QStringLiteral("files")].toObject();
        QJsonObject model = files[QStringLiteral("model")].toObject();
        model[QStringLiteral("size")] = 100;
        files[QStringLiteral("model")] = model;
        de[QStringLiteral("files")] = files;
        pairs[QStringLiteral("en-de")] = de;
        root[QStringLiteral("pairs")] = pairs;
        const Catalog c = Catalog::fromJson(QJsonDocument(root).toJson(), nullptr, true);
        QString error;
        QVERIFY(!install(fx, {c.pair(QStringLiteral("en"), QStringLiteral("de"))}, &error));
        QVERIFY2(error.contains(QStringLiteral("larger")), qPrintable(error));
        QVERIFY(!ModelStore(fx.store.path()).isInstalled(c.pair(QStringLiteral("en"), QStringLiteral("de"))));
    }

    void aDownloadCanBeCanceledAndLeavesNothing()
    {
        Fixture fx;
        const ModelPair p = fx.catalog().pair(QStringLiteral("en"), QStringLiteral("de"));
        QNetworkAccessManager net;
        ModelStore store(fx.store.path());
        Downloader d(store, &net);
        QSignalSpy done(&d, &Downloader::finished);
        d.start({p});
        QVERIFY(d.isRunning());
        d.cancel();
        QCOMPARE(done.count(), 1);
        QVERIFY(!done.first().at(0).toBool());
        QVERIFY(done.first().at(1).toBool());
        QVERIFY(done.first().at(2).toString().isEmpty());
        QVERIFY(!d.isRunning());
        QVERIFY(!store.isInstalled(p));
        QVERIFY(!QFileInfo::exists(store.partFolder(QStringLiteral("en"), QStringLiteral("de"))));
    }

    // ---- the translator
    void aPairWithoutAModelOfItsOwnGoesThroughEnglish()
    {
        Fixture fx;
        auto stub = std::make_shared<StubBackend>();
        const Translator t = translatorFor(fx, stub);
        QCOMPARE(t.missing(QStringLiteral("de"), QStringLiteral("fr")).size(), 2);
        QVERIFY(t.downloadBytes(QStringLiteral("de"), QStringLiteral("fr")) > 0);
        QVERIFY(install(fx, t.missing(QStringLiteral("de"), QStringLiteral("fr"))));
        QVERIFY(t.missing(QStringLiteral("de"), QStringLiteral("fr")).isEmpty());
        QStringList out;
        QString error;
        QVERIFY(t.translatePieces(QStringLiteral("de"), QStringLiteral("fr"), {QStringLiteral("Guten Tag")}, true, &out, &error));
        QCOMPARE(out, QStringList{QStringLiteral("en-fr:de-en:Guten Tag")});
        QCOMPARE(stub->hops, (QStringList{QStringLiteral("de-en"), QStringLiteral("en-fr")}));
        // English as either end is one step.
        QVERIFY(install(fx, t.missing(QStringLiteral("en"), QStringLiteral("de"))));
        stub->hops.clear();
        QVERIFY(t.translatePieces(QStringLiteral("en"), QStringLiteral("de"), {QStringLiteral("x")}, true, &out, &error));
        QCOMPARE(stub->hops, QStringList{QStringLiteral("en-de")});
    }

    void aMissingModelIsSaidAndNothingIsTranslated()
    {
        Fixture fx;
        auto stub = std::make_shared<StubBackend>();
        const Translator t = translatorFor(fx, stub);
        QStringList out;
        QString error;
        QVERIFY(!t.translatePieces(QStringLiteral("en"), QStringLiteral("de"), {QStringLiteral("x")}, true, &out, &error));
        QVERIFY2(error.contains(QStringLiteral("not installed")), qPrintable(error));
        QCOMPARE(stub->calls.load(), 0);
        QVERIFY(!t.translatePieces(QStringLiteral("en"), QStringLiteral("zz"), {QStringLiteral("x")}, true, &out, &error));
        QVERIFY(error.contains(QStringLiteral("no model")));
        const Translator noEngine(fx.catalog(), ModelStore(fx.store.path()), nullptr);
        QVERIFY(!noEngine.hasEngine());
        QVERIFY(!noEngine.translatePieces(QStringLiteral("en"), QStringLiteral("de"), {QStringLiteral("x")}, true, &out, &error));
    }

    void aTranslationRunsOnAThreadOfItsOwnAndCanBeCanceled()
    {
        Fixture fx;
        auto stub = std::make_shared<StubBackend>();
        const Translator t = translatorFor(fx, stub);
        QVERIFY(install(fx, t.missing(QStringLiteral("en"), QStringLiteral("de"))));
        stub->hold = true;
        TranslateJob job(t, QStringLiteral("en"), QStringLiteral("de"), {QStringLiteral("one"), QStringLiteral("two")}, true);
        QSignalSpy done(&job, &TranslateJob::finished);
        job.start();
        QTRY_VERIFY(stub->calls.load() > 0);
        QVERIFY(done.isEmpty());   // (the main thread is free: the job is still held)
        job.cancel();
        QTRY_COMPARE(done.count(), 1);
        QVERIFY(!done.first().at(0).toBool());
        QVERIFY(done.first().at(1).toBool());
        // (a run that is let go finishes)
        stub->hold = false;
        TranslateJob again(t, QStringLiteral("en"), QStringLiteral("de"), {QStringLiteral("one")}, true);
        QSignalSpy done2(&again, &TranslateJob::finished);
        again.start();
        QTRY_COMPARE(done2.count(), 1);
        QVERIFY(done2.first().at(0).toBool());
        QCOMPARE(done2.first().at(2).toStringList(), QStringList{QStringLiteral("en-de:one")});
    }

    // ---- the runs
    void aParagraphIsReadAsRunsAndWrittenAsHtml()
    {
        DocHost host;
        Source s = host.source();   // the paragraph at the cursor (the first)
        QCOMPARE(s.paragraphs.size(), 1);
        const QVector<Piece> pcs = pieces(s);
        QCOMPARE(pcs.size(), 1);
        QCOMPARE(pieceHtml(s, pcs.first()), QStringLiteral("Hello <b>big</b> world."));
        QCOMPARE(s.plainText(), QStringLiteral("Hello big world."));
        // A selection over both paragraphs reads both.
        host.selectFrom = 2;
        host.selectTo = host.doc.characterCount() - 1;
        s = host.source();
        QCOMPARE(s.paragraphs.size(), 2);
        QCOMPARE(pieceHtml(s, pieces(s).first()), QStringLiteral("llo <b>big</b> world."));
    }

    void aTabOrALineBreakCutsAParagraphIntoPiecesAndIsKept()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        c.insertText(QStringLiteral("Name:\tJohn & Co <ltd>"));
        c.insertText(QString(QChar::LineSeparator));
        c.insertText(QStringLiteral("  Second line  "));
        c.setPosition(0);
        const Source s = readSource(c);
        const QVector<Piece> pcs = pieces(s);
        QCOMPARE(pcs.size(), 3);
        QCOMPARE(pieceHtml(s, pcs[0]), QStringLiteral("Name:"));
        QCOMPARE(pieceHtml(s, pcs[1]), QStringLiteral("John &amp; Co &lt;ltd&gt;"));
        QCOMPARE(pieceHtml(s, pcs[2]), QStringLiteral("Second line"));
        // Translated, and put back: the tab and the break stay, the white space at the ends of a piece too.
        const Result r = rebuild(s, pcs, {QStringLiteral("Nom:"), QStringLiteral("Jean &amp; Cie"), QStringLiteral("Deuxi&egrave;me &#233;")});
        QVERIFY(r.plainText().startsWith(QStringLiteral("Nom:\tJean & Cie")));
        EditHooks hooks;
        QVERIFY(applyReplace(s, r, hooks));
        // (toPlainText shows the line break as a line feed)
        QCOMPARE(doc.toPlainText(), QStringLiteral("Nom:\tJean & Cie\n  Deuxi&egrave;me é  "));
        QVERIFY(doc.toPlainText().contains(QLatin1Char('\t')));
    }

    void translatedHtmlGetsTheLookOfTheRunThatHadTheSameBoldAndItalic()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat red, boldBlue, italic;
        red.setForeground(Qt::red);
        boldBlue.setFontWeight(QFont::Bold);
        boldBlue.setForeground(Qt::blue);
        italic.setFontItalic(true);
        c.insertText(QStringLiteral("The "), red);
        c.insertText(QStringLiteral("strong"), boldBlue);
        c.insertText(QStringLiteral(" and "), red);
        c.insertText(QStringLiteral("slanted"), italic);
        c.setPosition(0);
        const Source s = readSource(c);
        const QVector<Piece> pcs = pieces(s);
        QCOMPARE(pieceHtml(s, pcs.first()), QStringLiteral("The <b>strong</b> and <i>slanted</i>"));
        // The translation moves the tags with the words.
        const Result r = rebuild(s, pcs, {QStringLiteral("El <i>inclinado</i> y <b>fuerte</b>")});
        const Paragraph &p = r.paragraphs.first();
        QCOMPARE(p.runs.size(), 4);
        QCOMPARE(p.runs[0].text, QStringLiteral("El "));
        QCOMPARE(p.runs[0].format.foreground().color(), QColor(Qt::red));   // (the plain run's look: the first run with no bold and no italic)
        QCOMPARE(p.runs[1].text, QStringLiteral("inclinado"));
        QVERIFY(p.runs[1].format.fontItalic());
        QCOMPARE(p.runs[3].text, QStringLiteral("fuerte"));
        QVERIFY(p.runs[3].format.fontWeight() >= QFont::Bold);
        QCOMPARE(p.runs[3].format.foreground().color(), QColor(Qt::blue));
        QVERIFY(r.html().contains(QStringLiteral("<b>fuerte</b>")));
    }

    void replaceIsOneUndoStepAndKeepsTheOtherParagraphs()
    {
        DocHost host;
        host.selectFrom = 0;
        host.selectTo = host.doc.characterCount() - 1;
        const Source s = host.source();
        const QVector<Piece> pcs = pieces(s);
        QCOMPARE(pcs.size(), 2);
        const Result r = rebuild(s, pcs, {QStringLiteral("Hallo <b>gro&szlig;e</b> Welt."), QStringLiteral("Zweiter Absatz.")});
        EditHooks hooks;
        host.doc.clearUndoRedoStacks();
        QVERIFY(applyReplace(s, r, hooks));
        QCOMPARE(host.doc.toPlainText(), QStringLiteral("Hallo gro&szlig;e Welt.\nZweiter Absatz."));   // (&szlig; is not an entity this reader knows: it stays as written)
        // The second run is bold.
        QTextCursor c(&host.doc);
        c.setPosition(7);
        c.setPosition(8, QTextCursor::KeepAnchor);
        QVERIFY(c.charFormat().fontWeight() >= QFont::Bold);
        host.doc.undo();
        QCOMPARE(host.doc.toPlainText(), QStringLiteral("Hello big world.\nSecond paragraph."));
    }

    void insertBelowAddsParagraphsAfterTheOriginalAndLeavesItAlone()
    {
        DocHost host;
        host.selectFrom = 0;   // the first paragraph
        const Source s = host.source();
        const Result r = rebuild(s, pieces(s), {QStringLiteral("Hallo <b>große</b> Welt.")});
        EditHooks hooks;
        QVERIFY(applyInsertBelow(s, r, hooks));
        QCOMPARE(host.doc.toPlainText(), QStringLiteral("Hello big world.\nHallo große Welt.\nSecond paragraph."));
        QCOMPARE(host.doc.blockCount(), 3);
    }

    void aTextThatChangedSinceItWasReadIsNotReplaced()
    {
        DocHost host;
        const Source s = host.source();
        const Result r = rebuild(s, pieces(s), {QStringLiteral("Hallo Welt.")});
        QTextCursor c(&host.doc);
        c.setPosition(0);
        c.insertText(QStringLiteral("Oh! "));
        EditHooks hooks;
        QVERIFY(!applyReplace(s, r, hooks));
        QVERIFY(!applyInsertBelow(s, r, hooks));
        QCOMPARE(host.doc.toPlainText(), QStringLiteral("Oh! Hello big world.\nSecond paragraph."));
    }

    // ---- the pane
    void aPaneWithoutTheModelsOffersTheDownloadWithItsSizeAndNothingIsFetchedUntilItIsClicked()
    {
        Fixture fx;
        auto stub = std::make_shared<StubBackend>();
        DocHost host;
        QNetworkAccessManager net;
        TranslatePane pane(&host, translatorFor(fx, stub), &net);
        pane.show();
        auto *from = pane.findChild<QComboBox *>(QStringLiteral("translateFrom"));
        auto *to = pane.findChild<QComboBox *>(QStringLiteral("translateTo"));
        from->setCurrentIndex(from->findData(QStringLiteral("en")));
        to->setCurrentIndex(to->findData(QStringLiteral("de")));
        QCOMPARE(pane.findChild<QPlainTextEdit *>(QStringLiteral("translateOriginal"))->toPlainText(), QStringLiteral("Hello big world."));
        pane.translateNow();
        auto *status = pane.findChild<QLabel *>(QStringLiteral("translateStatus"));
        auto *download = pane.findChild<QPushButton *>(QStringLiteral("translateDownload"));
        QVERIFY(status->isVisible());
        QVERIFY2(status->text().contains(QStringLiteral("No translation model is installed for")), qPrintable(status->text()));
        QVERIFY(download->isVisible());
        QVERIFY2(download->text().startsWith(QStringLiteral("Download ")) && download->text().endsWith(QStringLiteral(")?")), qPrintable(download->text()));
        QCOMPARE(stub->calls.load(), 0);
        QVERIFY(!pane.findChild<QPushButton *>(QStringLiteral("translateReplace"))->isEnabled());
        QVERIFY(!ModelStore(fx.store.path()).isInstalled(fx.catalog().pair(QStringLiteral("en"), QStringLiteral("de"))));
    }

    void clickingDownloadFetchesTheModelsThenTranslatesAndReplaceWritesItIn()
    {
        Fixture fx;
        auto stub = std::make_shared<StubBackend>();
        DocHost host;
        QNetworkAccessManager net;
        TranslatePane pane(&host, translatorFor(fx, stub), &net);
        pane.show();
        auto *from = pane.findChild<QComboBox *>(QStringLiteral("translateFrom"));
        auto *to = pane.findChild<QComboBox *>(QStringLiteral("translateTo"));
        from->setCurrentIndex(from->findData(QStringLiteral("en")));
        to->setCurrentIndex(to->findData(QStringLiteral("de")));
        pane.translateNow();
        pane.findChild<QPushButton *>(QStringLiteral("translateDownload"))->click();
        auto *result = pane.findChild<QTextBrowser *>(QStringLiteral("translateResult"));
        QTRY_VERIFY_WITH_TIMEOUT(result->toPlainText().contains(QStringLiteral("en-de:")), 20000);
        QCOMPARE(result->toPlainText().trimmed(), QStringLiteral("en-de:Hello big world."));
        QVERIFY(ModelStore(fx.store.path()).isInstalled(fx.catalog().pair(QStringLiteral("en"), QStringLiteral("de"))));
        auto *replace = pane.findChild<QPushButton *>(QStringLiteral("translateReplace"));
        QVERIFY(replace->isEnabled());
        replace->click();
        QCOMPARE(host.doc.toPlainText(), QStringLiteral("en-de:Hello big world.\nSecond paragraph."));
    }

    void insertBelowFromThePaneAndAnEditedDocumentIsNotOverwritten()
    {
        Fixture fx;
        auto stub = std::make_shared<StubBackend>();
        DocHost host;
        QNetworkAccessManager net;
        const Translator t = translatorFor(fx, stub);
        QVERIFY(install(fx, t.missing(QStringLiteral("en"), QStringLiteral("de"))));
        TranslatePane pane(&host, t, &net);
        pane.show();
        pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->findData(QStringLiteral("en")));
        pane.findChild<QComboBox *>(QStringLiteral("translateTo"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateTo"))->findData(QStringLiteral("de")));
        pane.translateNow();   // (installed: no download is asked for)
        auto *below = pane.findChild<QPushButton *>(QStringLiteral("translateInsertBelow"));
        QTRY_VERIFY_WITH_TIMEOUT(below->isEnabled(), 20000);
        below->click();
        QCOMPARE(host.doc.toPlainText(), QStringLiteral("Hello big world.\nen-de:Hello big world.\nSecond paragraph."));
        // Translate again, then change the document before Replace: nothing is overwritten.
        pane.translateNow();
        QTRY_VERIFY_WITH_TIMEOUT(below->isEnabled(), 20000);
        QTextCursor c(&host.doc);
        c.setPosition(0);
        c.insertText(QStringLiteral("!"));
        pane.findChild<QPushButton *>(QStringLiteral("translateReplace"))->click();
        QVERIFY(host.doc.toPlainText().startsWith(QStringLiteral("!Hello big world.")));
        QVERIFY(pane.findChild<QLabel *>(QStringLiteral("translateStatus"))->text().contains(QStringLiteral("changed")));
        QVERIFY(!pane.findChild<QPushButton *>(QStringLiteral("translateReplace"))->isEnabled());
    }

    void aHashMismatchInThePaneIsSaidAndNothingIsTranslated()
    {
        Fixture fx({QStringLiteral("en-de"), QStringLiteral("de-en")}, QStringLiteral("en-de/model"));
        auto stub = std::make_shared<StubBackend>();
        DocHost host;
        QNetworkAccessManager net;
        TranslatePane pane(&host, translatorFor(fx, stub), &net);
        pane.show();
        pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->findData(QStringLiteral("en")));
        pane.findChild<QComboBox *>(QStringLiteral("translateTo"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateTo"))->findData(QStringLiteral("de")));
        pane.translateNow();
        pane.findChild<QPushButton *>(QStringLiteral("translateDownload"))->click();
        auto *status = pane.findChild<QLabel *>(QStringLiteral("translateStatus"));
        QTRY_VERIFY_WITH_TIMEOUT(status->text().contains(QStringLiteral("checksum")), 20000);
        QCOMPARE(stub->calls.load(), 0);
        QVERIFY(!pane.findChild<QPushButton *>(QStringLiteral("translateReplace"))->isEnabled());
    }

    void aPairTheModelsDoNotCoverIsOfferedThroughTheWebsite()
    {
        Fixture fx({QStringLiteral("en-de"), QStringLiteral("de-en")});
        auto stub = std::make_shared<StubBackend>();
        DocHost host;
        QNetworkAccessManager net;
        TranslatePane pane(&host, translatorFor(fx, stub), &net);
        pane.show();
        pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->findData(QStringLiteral("en")));
        auto *to = pane.findChild<QComboBox *>(QStringLiteral("translateTo"));
        to->setCurrentIndex(to->findData(QStringLiteral("ja")));
        pane.translateNow();
        auto *browser = pane.findChild<QPushButton *>(QStringLiteral("translateBrowser"));
        QVERIFY(browser->isVisible());
        QVERIFY(!pane.findChild<QPushButton *>(QStringLiteral("translateDownload"))->isVisible());
        browser->click();
        QCOMPARE(host.browserText, QStringLiteral("Hello big world."));
        QCOMPARE(host.browserCode, QStringLiteral("ja"));
    }

    void aProgramWithoutTheEngineSaysSoAndOffersTheWebsite()
    {
        Fixture fx;
        DocHost host;
        QNetworkAccessManager net;
        TranslatePane pane(&host, Translator(fx.catalog(), ModelStore(fx.store.path()), nullptr), &net);
        pane.show();
        pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->findData(QStringLiteral("en")));
        pane.findChild<QComboBox *>(QStringLiteral("translateTo"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateTo"))->findData(QStringLiteral("de")));
        pane.translateNow();
        QVERIFY(pane.findChild<QPushButton *>(QStringLiteral("translateBrowser"))->isVisible());
        QVERIFY(pane.findChild<QLabel *>(QStringLiteral("translateStatus"))->text().contains(QStringLiteral("no translator")));
    }

    void theTextsOwnLanguageBecomesTheFromLanguageAndTheSameLanguageTwiceIsSaid()
    {
        Fixture fx;
        auto stub = std::make_shared<StubBackend>();
        DocHost host;
        host.language = QStringLiteral("fr");
        QNetworkAccessManager net;
        TranslatePane pane(&host, translatorFor(fx, stub), &net);
        pane.show();
        auto *from = pane.findChild<QComboBox *>(QStringLiteral("translateFrom"));
        auto *to = pane.findChild<QComboBox *>(QStringLiteral("translateTo"));
        QCOMPARE(from->currentData().toString(), QStringLiteral("fr"));
        to->setCurrentIndex(to->findData(QStringLiteral("fr")));
        pane.translateNow();
        QVERIFY(pane.findChild<QLabel *>(QStringLiteral("translateStatus"))->text().contains(QStringLiteral("different")));
        QCOMPARE(stub->calls.load(), 0);
    }

    void cancelStopsATranslationInThePane()
    {
        Fixture fx;
        auto stub = std::make_shared<StubBackend>();
        DocHost host;
        QNetworkAccessManager net;
        const Translator t = translatorFor(fx, stub);
        QVERIFY(install(fx, t.missing(QStringLiteral("en"), QStringLiteral("de"))));
        TranslatePane pane(&host, t, &net);
        pane.show();
        pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateFrom"))->findData(QStringLiteral("en")));
        pane.findChild<QComboBox *>(QStringLiteral("translateTo"))->setCurrentIndex(pane.findChild<QComboBox *>(QStringLiteral("translateTo"))->findData(QStringLiteral("de")));
        stub->hold = true;
        pane.translateNow();
        auto *cancel = pane.findChild<QPushButton *>(QStringLiteral("translateCancel"));
        QTRY_VERIFY(cancel->isVisible());
        cancel->click();
        auto *status = pane.findChild<QLabel *>(QStringLiteral("translateStatus"));
        QTRY_VERIFY_WITH_TIMEOUT(status->text() == QStringLiteral("Canceled."), 10000);
        QVERIFY(pane.findChild<QPushButton *>(QStringLiteral("translateRun"))->isVisible());
        QVERIFY(!pane.findChild<QPushButton *>(QStringLiteral("translateReplace"))->isEnabled());
        QCOMPARE(host.doc.toPlainText(), QStringLiteral("Hello big world.\nSecond paragraph."));
    }

    // ---- the real engine, when a model is at hand
    void theRealEngineTranslatesWithTheModelNamedByJoTranslateModelDir()
    {
        const QString dir = qEnvironmentVariable("JO_TRANSLATE_MODEL_DIR");
        if (dir.isEmpty()) QSKIP("JO_TRANSLATE_MODEL_DIR is not set");
        if (!builtInBackend()) QSKIP("built without the engine");
        // The folder holds the English to German files as Mozilla names them (model.ende.intgemm.alphas.bin, lex.50.50.ende.s2t.bin, vocab.ende.spm).
        QJsonObject fs;
        const QPair<const char *, const char *> files[] = {{"model", "model.ende.intgemm.alphas.bin"}, {"shortlist", "lex.50.50.ende.s2t.bin"}, {"vocab", "vocab.ende.spm"}};
        for (const auto &f : files) {
            QFile file(QDir(dir).filePath(QString::fromLatin1(f.second)));
            QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.fileName()));
            const QByteArray content = file.readAll();
            fs[QString::fromLatin1(f.first)] = QJsonObject{{QStringLiteral("url"), QUrl::fromLocalFile(file.fileName()).toString()}, {QStringLiteral("size"), content.size()},
                                                          {QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash(content, QCryptographicHash::Sha256).toHex())}};
        }
        const QByteArray json = QJsonDocument(QJsonObject{{QStringLiteral("baseUrl"), QStringLiteral("https://example.invalid/")},
                                                          {QStringLiteral("pairs"), QJsonObject{{QStringLiteral("en-de"), QJsonObject{{QStringLiteral("files"), fs}}}}}}).toJson();
        const Catalog catalog = Catalog::fromJson(json, nullptr, true);
        QTemporaryDir store;
        const Translator t(catalog, ModelStore(store.path()), builtInBackend());
        QNetworkAccessManager net;
        Downloader d(ModelStore(store.path()), &net);
        QSignalSpy done(&d, &Downloader::finished);
        d.start(t.missing(QStringLiteral("en"), QStringLiteral("de")));
        QTRY_VERIFY_WITH_TIMEOUT(!done.isEmpty(), 60000);
        QVERIFY(done.first().at(0).toBool());
        QStringList out;
        QString error;
        QElapsedTimer timer;
        timer.start();
        QVERIFY2(t.translatePieces(QStringLiteral("en"), QStringLiteral("de"), {QStringLiteral("Hello, <b>this</b> is a <i>small</i> test."), QString(), QStringLiteral("Good morning.")}, true, &out, &error), qPrintable(error));
        const qint64 first = timer.elapsed();
        qInfo().noquote() << "real model: first translation (load included)" << first << "ms:" << out;
        QCOMPARE(out.size(), 3);
        QVERIFY2(out[0].contains(QStringLiteral("<b>")) && out[0].contains(QStringLiteral("</b>")) && out[0].contains(QStringLiteral("<i>")), qPrintable(out[0]));
        QVERIFY2(out[0].startsWith(QStringLiteral("Hallo")), qPrintable(out[0]));
        QVERIFY(out[1].isEmpty());
        QVERIFY2(out[2].contains(QStringLiteral("Morgen")), qPrintable(out[2]));
        // The speed of a second run, on a longer text (the model is loaded now).
        QStringList many;
        for (int i = 0; i < 20; ++i) many << QStringLiteral("The committee reviewed the proposal, and several members raised questions about the schedule and the budget.");
        timer.restart();
        QVERIFY(t.translatePieces(QStringLiteral("en"), QStringLiteral("de"), many, true, &out, &error));
        const qint64 ms = timer.elapsed();
        int words = 0;
        for (const QString &m : many) words += int(m.split(QLatin1Char(' ')).size());
        qInfo().noquote() << "real model:" << words << "words in" << ms << "ms," << (ms > 0 ? words * 1000 / ms : 0) << "words per second";
        QCOMPARE(out.size(), many.size());
    }
};

JP_TEST_CLASS(ShellTranslateTests)
#include "tst_shell_translate.moc"
