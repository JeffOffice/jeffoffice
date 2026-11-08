// Passwords for PDFs: the shared function (libs/base/io/pdfcrypt.h), the protection row of the export options, and opening a PDF that has a password.
#include "pubhelpers.h"
#include "app/passworddialogs.h"
#include "app/pdfprotection.h"
#include "io/pdfcrypt.h"
#include "../plainpdfwatch.h"
#include "../emlpdf.h"
#include "core/appinfo.h"
#include <QBuffer>
#include <QDirIterator>
#include <QCheckBox>
#include <QMessageBox>
#include <QGroupBox>
#include <QPushButton>
#include <QScopeGuard>
#include <QSettings>
#include <QTemporaryDir>

// A one-page PDF as Qt writes it, in memory.
static QByteArray plainPdf(const QString &title = QString())
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    {
        QPdfWriter pdf(&buffer);
        pdf.setPageSize(QPageSize(QPageSize::A5));
        if (!title.isEmpty()) {
            pdf.setTitle(title);
            pdf.setDocumentXmpMetadata("<x:xmpmeta xmlns:x=\"adobe:ns:meta/\"><rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"><rdf:Description xmlns:dc=\"http://purl.org/dc/elements/1.1/\"><dc:title>" +
                                        title.toUtf8() + "</dc:title></rdf:Description></rdf:RDF></x:xmpmeta>");
        }
        QPainter p(&pdf);
        p.drawText(200, 400, QStringLiteral("Quarterly figures"));
    }
    return buffer.data();
}

// A PDF the program wrote, on disk.
static QByteArray bytesOf(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// Answers the next modal dialog with the given name (or title), once, from the event loop.
static void answerDialog(const QString &name, const std::function<void(QDialog *)> &act, int triesLeft = 200)
{
    QTimer::singleShot(10, [=] {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog || (dialog->objectName() != name && dialog->windowTitle() != name)) {
            if (triesLeft > 0) answerDialog(name, act, triesLeft - 1);
            return;
        }
        act(dialog);
    });
}

// Whether the bytes hold the text in any of the ways a PDF writes it: as it is, as UTF-16 (big end first), or as hex of either.
static bool holdsText(const QByteArray &bytes, const QString &text)
{
    QByteArray be;
    for (const QChar c : text) be.append(char(c.unicode() >> 8)).append(char(c.unicode() & 0xff));
    for (const QByteArray &form : {text.toUtf8(), be, text.toUtf8().toHex(), text.toUtf8().toHex().toUpper(), be.toHex(), be.toHex().toUpper()})
        if (bytes.contains(form)) return true;
    return false;
}

// Whether the keys and values of the stores the app's settings use, or its recent files, hold the text.
static bool settingsHold(const QString &text)
{
    jp::Settings::get().sync();
    for (const QString &app : {jp::appInfo().name, QStringLiteral("JeffOffice")}) {
        QSettings store(QSettings::defaultFormat(), QSettings::UserScope, jp::appInfo().organization, app);
        for (const QString &key : store.allKeys())
            if (key.contains(text) || store.value(key).toString().contains(text) || store.value(key).toStringList().join(QLatin1Char('\n')).contains(text)) return true;
    }
    return jp::Settings::get().recentFiles().join(QLatin1Char('\n')).contains(text) || jp::Settings::get().pinnedFiles().join(QLatin1Char('\n')).contains(text);
}

class PubPdfCryptTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    void theFunctionEncryptsWithAes256()
    {
        // A title in characters Qt writes as they are, in /Info and in the XMP: a file that is only scrambled in its streams would still show it.
        const QString title = QStringLiteral("Zebra Quartz Mnemonic 7391");
        const QByteArray plain = plainPdf(title);
        QVERIFY(plain.startsWith("%PDF-"));
        QVERIFY2(holdsText(plain, title), "the control: the plain PDF shows the title");
        jp::PdfEncryptionInfo none;
        QVERIFY(jp::readPdfEncryption(plain, QString(), &none));
        QVERIFY(!none.encrypted);

        jp::PdfProtection p;
        p.openPassword = QStringLiteral("correct horse");
        QString why;
        const QByteArray made = jp::encryptPdf(plain, p, &why);
        QVERIFY2(!made.isEmpty(), qPrintable(why));
        QVERIFY(made.startsWith("%PDF-"));
        QVERIFY(made.contains("/Encrypt"));
        jp::PdfEncryptionInfo info;
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("correct horse"), &info));
        QVERIFY(info.encrypted);
        QCOMPARE(info.version, 5);
        QCOMPARE(info.revision, 6);
        // Without the right password, qpdf reads nothing.
        QVERIFY(!jp::readPdfEncryption(made, QString(), &info));
        QVERIFY(!jp::readPdfEncryption(made, QStringLiteral("wrong"), &info));
        // pdfium refuses it without the password and opens it with it; and /Info and the XMP are encrypted too: the title is nowhere in the file.
        QVERIFY(!jp::PdfDocument(made).isValid());
        QVERIFY(jp::PdfDocument(made).needsPassword());
        QVERIFY(jp::PdfDocument(made, QStringLiteral("correct horse")).isValid());
        QVERIFY2(!holdsText(made, title), "the title is in the encrypted file");
        QVERIFY(!holdsText(made, QStringLiteral("Quarterly figures")));
        QVERIFY(jp::decryptPdf(made, QStringLiteral("correct horse")).contains("/Info"));
    }

    void theFunctionRefusesWhatItCannotDo()
    {
        const QByteArray plain = plainPdf();
        QString why;
        jp::PdfProtection p;
        QVERIFY(jp::encryptPdf(plain, p, &why).isEmpty());   // no password
        QVERIFY(!why.isEmpty());
        p.openPassword = QStringLiteral("a");
        p.limited = true;
        why.clear();
        QVERIFY(jp::encryptPdf(plain, p, &why).isEmpty());   // limits without a password of their own
        QVERIFY(!why.isEmpty());
        p.ownerPassword = QStringLiteral("a");
        why.clear();
        QVERIFY(jp::encryptPdf(plain, p, &why).isEmpty());   // the same as the open password
        QVERIFY(!why.isEmpty());
        p.limited = false;
        why.clear();
        QVERIFY(jp::encryptPdf(QByteArray("not a pdf"), p, &why).isEmpty());
        QVERIFY(!why.isEmpty());
        // The cap is the same as for a picture file that is read.
        QCOMPARE(jp::kMostPdfEncryptBytes, qint64(256) << 20);
    }

    void pdfiumOpensItWithThePasswordAndNotWithout()
    {
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("s3cret");
        const QByteArray made = jp::encryptPdf(plainPdf(), p);
        QVERIFY(!made.isEmpty());
        const jp::PdfDocument without(made);
        QVERIFY(!without.isValid());
        QVERIFY(without.needsPassword());
        const jp::PdfDocument wrong(made, QStringLiteral("S3CRET"));
        QVERIFY(!wrong.isValid());
        QVERIFY(wrong.needsPassword());
        const jp::PdfDocument right(made, QStringLiteral("s3cret"));
        QVERIFY(right.isValid());
        QVERIFY(!right.needsPassword());
        QCOMPARE(right.pageCount(), 1);
        QVERIFY(right.text(0).contains(QStringLiteral("Quarterly")));
        // A PDF with no password opens as ever, and a file that is no PDF does not ask for one.
        const jp::PdfDocument plain(plainPdf());
        QVERIFY(plain.isValid());
        const jp::PdfDocument junk(QByteArray("%PDF-1.4 junk"));
        QVERIFY(!junk.isValid());
        QVERIFY(!junk.needsPassword());
    }

    void thePermissionsAreAsChosen()
    {
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("open");
        p.limited = true;
        p.ownerPassword = QStringLiteral("owner");
        p.allowPrint = false;
        p.allowCopy = true;
        p.allowChange = false;
        jp::PdfEncryptionInfo info;
        QByteArray made = jp::encryptPdf(plainPdf(), p);
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("open"), &info));
        QVERIFY(!info.ownerMatched);
        QVERIFY(!info.allowPrint);
        QVERIFY(info.allowCopy);
        QVERIFY(!info.allowChange);
        // The owner password opens it too, and lifts the limits.
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("owner"), &info));
        QVERIFY(info.ownerMatched);
        QVERIFY(info.allowPrint && info.allowCopy && info.allowChange);
        // The other way round.
        p.allowPrint = true;
        p.allowCopy = false;
        p.allowChange = true;
        made = jp::encryptPdf(plainPdf(), p);
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("open"), &info));
        QVERIFY(info.allowPrint);
        QVERIFY(!info.allowCopy);
        QVERIFY(info.allowChange);
        // No limits asked for: none are set, and there is no owner password anyone knows.
        p.limited = false;
        made = jp::encryptPdf(plainPdf(), p);
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("open"), &info));
        QVERIFY(info.allowPrint && info.allowCopy && info.allowChange);
        QVERIFY(!jp::readPdfEncryption(made, QStringLiteral("owner"), &info));
    }

    void anExportWithAPasswordIsEncryptedAndNotPdfA()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.editor()->doc()->props.title = QStringLiteral("Zebra Quartz Mnemonic 7391");
        ScratchTemp scratch;
        QTemporaryDir dir, keep;
        jp::MainWindow::PdfSettings s;
        s.archival = true;
        // Control: an ordinary PDF/A export has the mark, no encryption, and the watch sees the plain PDF it leaves.
        PlainPdfWatch watch;
        watch.dirs = {keep.path()};
        QVERIFY(w.exportPdfTo(keep.filePath(QStringLiteral("a.pdf")), s));
        QVERIFY(bytesOf(keep.filePath(QStringLiteral("a.pdf"))).contains("pdfaid:part"));
        QVERIFY2(!watch.plain.isEmpty(), "the watch does not see a plain PDF");
        const QString path = dir.filePath(QStringLiteral("locked.pdf"));
        s.protect.openPassword = QStringLiteral("pw");
        watch.dirs.clear();
        watch.plain.clear();
        watch.dirs = {dir.path(), scratch.path()};
        QVERIFY(w.exportPdfTo(path, s));
        watch.dirs.clear();
        QVERIFY2(watch.plain.isEmpty(), qPrintable(watch.plain.join(QLatin1Char(' '))));
        const QByteArray made = bytesOf(path);
        jp::PdfEncryptionInfo info;
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("pw"), &info));
        QCOMPARE(info.version, 5);
        QCOMPARE(info.revision, 6);
        QVERIFY(!jp::PdfDocument(made).isValid());
        QVERIFY(jp::PdfDocument(made, QStringLiteral("pw")).isValid());
        QVERIFY2(!holdsText(made, QStringLiteral("Zebra Quartz Mnemonic 7391")), "the title is in the file");
        // PDF/A was turned off: the mark is not in what the password opens.
        QVERIFY(!jp::decryptPdf(made, QStringLiteral("pw")).contains("pdfaid:part"));
        // Nothing else is left in the folders.
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files), QStringList{QStringLiteral("locked.pdf")});
        QVERIFY2(QDir(scratch.path()).entryList(QDir::Files).isEmpty(), qPrintable(QDir(scratch.path()).entryList(QDir::Files).join(QLatin1Char(' '))));
    }

    void aPasswordIsHeldToTheRulesOfR6()
    {
        const QString composed = QStringLiteral("caf\u00e9"), decomposed = QStringLiteral("cafe\u0301");
        QString made;
        QCOMPARE(jp::normalizePdfPassword(decomposed, &made), jp::PdfPasswordProblem::None);
        QCOMPARE(made, composed);
        QCOMPARE(jp::pdfOpeningPassword(decomposed), composed);
        // Made with the accent apart, it opens with it joined, and the other way round.
        jp::PdfProtection p;
        p.openPassword = decomposed;
        const QByteArray locked = jp::encryptPdf(plainPdf(), p);
        QVERIFY(!locked.isEmpty());
        jp::PdfEncryptionInfo info;
        QVERIFY(jp::readPdfEncryption(locked, composed, &info));
        QVERIFY(jp::PdfDocument(locked, composed).isValid());
        // Two spellings of one password are one password: not a different one for the limits.
        p.limited = true;
        p.ownerPassword = composed;
        QString why;
        QVERIFY(jp::encryptPdf(plainPdf(), p, &why).isEmpty());
        QVERIFY(!why.isEmpty());
        // 127 bytes are allowed and 128 are not: plain letters, and letters of two bytes.
        QCOMPARE(jp::normalizePdfPassword(QString(127, QLatin1Char('a')), &made), jp::PdfPasswordProblem::None);
        QCOMPARE(jp::normalizePdfPassword(QString(128, QLatin1Char('a')), &made), jp::PdfPasswordProblem::TooLong);
        QVERIFY(made.isEmpty());
        QCOMPARE(jp::normalizePdfPassword(QString(63, QChar(0xe9)) + QLatin1Char('a'), &made), jp::PdfPasswordProblem::None);
        QCOMPARE(made.toUtf8().size(), 127);
        QCOMPARE(jp::normalizePdfPassword(QString(64, QChar(0xe9)), &made), jp::PdfPasswordProblem::TooLong);
        // (the 128 bytes the decomposed way come to 128 bytes only when composed: 64 letters of two bytes)
        QCOMPARE(jp::normalizePdfPassword(QString(64, QLatin1Char('e')).replace(QLatin1Char('e'), QStringLiteral("e\u0301")), &made), jp::PdfPasswordProblem::TooLong);
        p.limited = false;
        p.openPassword = QString(128, QLatin1Char('a'));
        why.clear();
        QVERIFY(jp::encryptPdf(plainPdf(), p, &why).isEmpty());
        QVERIFY(!why.isEmpty());
        p.openPassword = QString(127, QLatin1Char('a'));
        QVERIFY(!jp::encryptPdf(plainPdf(), p).isEmpty());
        // A control character is refused; a longer password for a file that is opened is cut at 127 bytes.
        QCOMPARE(jp::normalizePdfPassword(QStringLiteral("a\u0007b"), &made), jp::PdfPasswordProblem::Forbidden);
        QCOMPARE(jp::pdfOpeningPassword(QString(130, QLatin1Char('a'))).toUtf8().size(), 127);
        QCOMPARE(jp::pdfOpeningPassword(QString(100, QChar(0xe9))).toUtf8().size(), 126);   // (not in the middle of a letter)
    }

    void aCopyOfTheProtectionSharesNothing()
    {
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("alpha-secret");
        p.ownerPassword = QStringLiteral("beta-secret");
        jp::PdfProtection q = p.copy();
        QVERIFY(p.openPassword.constData() != q.openPassword.constData());
        p.wipe();
        QCOMPARE(q.openPassword, QStringLiteral("alpha-secret"));
        QCOMPARE(q.ownerPassword, QStringLiteral("beta-secret"));
        q.wipe();
    }

    void aPdfSentByEmailCanHaveAPassword()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString eml = dir.filePath(QStringLiteral("mail.eml"));
        jp::setSavePathStandIn([&](const QString &, const QString &, const QString &, QString *) { return eml; });
        const auto reset = qScopeGuard([] { jp::setSavePathStandIn(nullptr); });
        auto closeNotice = [](auto &self, int tries) -> void {
            QTimer::singleShot(10, [=] {
                if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) box->accept();
                else if (tries > 0) self(self, tries - 1);
            });
        };
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("pw");
        ScratchTemp scratch;
        PlainPdfWatch watch;
        watch.dirs = {dir.path(), scratch.path()};
        closeNotice(closeNotice, 300);
        QVERIFY(jp::emailAsAttachment(&w, w.editor(), QStringLiteral("pdf"), p));
        watch.dirs.clear();
        QVERIFY2(watch.plain.isEmpty(), qPrintable(watch.plain.join(QLatin1Char(' '))));
        const QByteArray attached = pdfAttachmentOf(bytesOf(eml));
        jp::PdfEncryptionInfo info;
        QVERIFY(jp::readPdfEncryption(attached, QStringLiteral("pw"), &info));
        QCOMPARE(info.version, 5);
        QVERIFY(!jp::PdfDocument(attached).isValid());
        // With no password the attachment is a plain PDF, as before.
        closeNotice(closeNotice, 300);
        QVERIFY(jp::emailAsAttachment(&w, w.editor(), QStringLiteral("pdf")));
        QVERIFY(jp::PdfDocument(pdfAttachmentOf(bytesOf(eml))).isValid());
    }

    void theExportOptionsOfferThePasswordAndTurnOffPdfA()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QTemporaryDir dir;
        bool sawOptions = false;
        jp::setPdfProtectionStandIn([](bool, jp::PdfProtection *p) {
            p->openPassword = QStringLiteral("pw");
            return jp::NewPasswordChoice::Set;
        });
        const auto reset = qScopeGuard([] { jp::setPdfProtectionStandIn(nullptr); });
        answerDialog(QStringLiteral("Create PDF"), [&](QDialog *dialog) {
            QCheckBox *pdfa = nullptr;
            for (QCheckBox *box : dialog->findChildren<QCheckBox *>())
                if (box->text().contains(QStringLiteral("PDF/A"))) pdfa = box;
            auto *button = dialog->findChild<QPushButton *>(QStringLiteral("pdfEncryptButton"));
            sawOptions = pdfa && button && button->text().contains(QStringLiteral("Encrypt with a password"));
            if (sawOptions) {
                pdfa->setChecked(true);
                QVERIFY(pdfa->isEnabled());
                button->click();
                QVERIFY(!pdfa->isChecked());
                QVERIFY(!pdfa->isEnabled());
                QVERIFY(!dialog->findChild<QLabel *>(QStringLiteral("pdfEncryptNote"))->text().isEmpty());
            }
            dialog->reject();
        });
        w.exportPdfWithOptions();
        QVERIFY(sawOptions);
    }

    void theRowKeepsThePasswordOnlyUntilItIsCleared()
    {
        QWidget host;
        auto *pdfa = new QCheckBox(QStringLiteral("PDF/A"), &host);
        pdfa->setChecked(true);
        auto *row = new jp::PdfProtectionRow(&host);
        row->forbiddenBy({pdfa}, QStringLiteral("because"));
        QVERIFY(!row->isProtected());
        QVERIFY(pdfa->isEnabled());
        int changes = 0;
        QObject::connect(row, &jp::PdfProtectionRow::protectionChanged, &host, [&] { ++changes; });
        jp::setPdfProtectionStandIn([](bool, jp::PdfProtection *p) {
            p->openPassword = QStringLiteral("plum-7391");
            return jp::NewPasswordChoice::Set;
        });
        const auto reset = qScopeGuard([] { jp::setPdfProtectionStandIn(nullptr); });
        row->findChild<QPushButton *>(QStringLiteral("pdfEncryptButton"))->click();
        QVERIFY(row->isProtected());
        QCOMPARE(changes, 1);
        QVERIFY(!pdfa->isChecked() && !pdfa->isEnabled());
        // Not in the store the settings use, nor in the recent files. (The control: a value put there is found.)
        QVERIFY(!settingsHold(QStringLiteral("plum-7391")));
        jp::Settings::get().setValue(QStringLiteral("test/control"), QStringLiteral("plum-7391"));
        QVERIFY(settingsHold(QStringLiteral("plum-7391")));
        jp::Settings::get().setValue(QStringLiteral("test/control"), QString());
        QVERIFY(!settingsHold(QStringLiteral("plum-7391")));
        row->clear();
        QVERIFY(!row->isProtected());
        QVERIFY(pdfa->isEnabled());
        QCOMPARE(changes, 2);
    }

    void theDialogChecksWhatIsTyped()
    {
        jp::PdfProtection chosen;
        auto fill = [](QDialog *d, const QString &name, const QString &text) { d->findChild<QLineEdit *>(name)->setText(text); };
        // Two passwords that differ, and then an empty limits password.
        answerDialog(QStringLiteral("pdfProtectionDialog"), [&](QDialog *d) {
            fill(d, QStringLiteral("pdfPassword"), QStringLiteral("one"));
            fill(d, QStringLiteral("pdfPasswordAgain"), QStringLiteral("two"));
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            QVERIFY(d->isVisible());
            QVERIFY(!d->findChild<QLabel *>(QStringLiteral("pdfProtectionProblem"))->text().isEmpty());
            fill(d, QStringLiteral("pdfPasswordAgain"), QStringLiteral("one"));
            d->findChild<QGroupBox *>(QStringLiteral("pdfRestrict"))->setChecked(true);
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            QVERIFY(d->isVisible());   // (the limits need a password of their own)
            fill(d, QStringLiteral("pdfOwnerPassword"), QStringLiteral("one"));
            fill(d, QStringLiteral("pdfOwnerPasswordAgain"), QStringLiteral("one"));
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            QVERIFY(d->isVisible());   // (and not the same as the open one)
            fill(d, QStringLiteral("pdfOwnerPassword"), QStringLiteral("owner"));
            fill(d, QStringLiteral("pdfOwnerPasswordAgain"), QStringLiteral("owner"));
            d->findChild<QCheckBox *>(QStringLiteral("pdfAllowPrint"))->setChecked(false);
            d->findChild<QCheckBox *>(QStringLiteral("pdfAllowCopy"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("pdfAllowChange"))->setChecked(false);
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        QCOMPARE(jp::askPdfProtection(nullptr, false, &chosen), jp::NewPasswordChoice::Set);
        QCOMPARE(chosen.openPassword, QStringLiteral("one"));
        QVERIFY(chosen.limited);
        QCOMPARE(chosen.ownerPassword, QStringLiteral("owner"));
        QVERIFY(!chosen.allowPrint);
        QVERIFY(chosen.allowCopy);
        QVERIFY(!chosen.allowChange);
        // Too long a password is refused with a message; a password typed with the accent apart is the same as the one with it joined; the boxes are empty after.
        jp::PdfProtection third;
        answerDialog(QStringLiteral("pdfProtectionDialog"), [&](QDialog *d) {
            fill(d, QStringLiteral("pdfPassword"), QString(128, QLatin1Char('a')));
            fill(d, QStringLiteral("pdfPasswordAgain"), QString(128, QLatin1Char('a')));
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            QVERIFY(d->isVisible());
            QVERIFY(d->findChild<QLabel *>(QStringLiteral("pdfProtectionProblem"))->text().contains(QStringLiteral("127")));
            fill(d, QStringLiteral("pdfPassword"), QStringLiteral("cafe\u0301"));
            fill(d, QStringLiteral("pdfPasswordAgain"), QStringLiteral("caf\u00e9"));
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        QCOMPARE(jp::askPdfProtection(nullptr, false, &third), jp::NewPasswordChoice::Set);
        QCOMPARE(third.openPassword, QStringLiteral("cafe\u0301"));   // (as typed: it is normalized when the PDF is made)
        third.wipe();
        // Cancel keeps nothing new; the way to remove a password is offered when there is one.
        jp::PdfProtection other;
        answerDialog(QStringLiteral("pdfProtectionDialog"), [&](QDialog *d) { d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Cancel)->click(); });
        QCOMPARE(jp::askPdfProtection(nullptr, false, &other), jp::NewPasswordChoice::Cancelled);
        QVERIFY(!other.isSet());
        answerDialog(QStringLiteral("pdfProtectionDialog"), [&](QDialog *d) {
            for (QPushButton *b : d->findChildren<QPushButton *>())
                if (b->text() == QStringLiteral("Remove Password")) b->click();
        });
        QCOMPARE(jp::askPdfProtection(nullptr, true, &chosen), jp::NewPasswordChoice::Removed);
    }

    void insertingAPdfWithAPasswordAsksAndAWrongOneAsksAgain()
    {
        QTemporaryDir dir;
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("s3cret");
        const QByteArray locked = jp::encryptPdf(plainPdf(), p);
        const QString path = dir.filePath(QStringLiteral("locked.pdf"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(locked);
        }
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const auto reset = qScopeGuard([] { jp::setOpenPasswordStandIn(nullptr); });
        // A wrong password, then the right one: asked twice, the second time saying the first was wrong; one picture is placed.
        QList<bool> wrongBefore;
        jp::setOpenPasswordStandIn([&](const QString &, bool wrong, QString *password) {
            wrongBefore << wrong;
            *password = wrongBefore.size() == 1 ? QStringLiteral("nope") : QStringLiteral("s3cret");
            return true;
        });
        w.insertFiles({path}, QPointF(-1, -1));
        QCOMPARE(wrongBefore, (QList<bool>{false, true}));
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), 1);
        // The picture the publication keeps is the page, with no password on it.
        QCOMPARE(w.editor()->doc()->images.size(), 1);
        QVERIFY(jp::PdfDocument(w.editor()->doc()->images.begin()->bytes).isValid());
        // A wrong one and then Cancel: asked twice (each ask is the person's), and nothing more is placed.
        int asked = 0;
        jp::setOpenPasswordStandIn([&](const QString &, bool, QString *password) {
            *password = QStringLiteral("nope");
            return ++asked < 2;
        });
        w.insertFiles({path}, QPointF(-1, -1));
        QCOMPARE(asked, 2);
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), 1);
        // A PDF with no password asks nothing.
        asked = 0;
        const QString open = dir.filePath(QStringLiteral("open.pdf"));
        {
            QFile f(open);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(plainPdf());
        }
        w.insertFiles({open}, QPointF(-1, -1));
        QCOMPARE(asked, 0);
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), 2);
    }

    void aPasswordTypedWithTheAccentApartOpensTheFile()
    {
        QTemporaryDir dir;
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("caf\u00e9");
        const QString path = dir.filePath(QStringLiteral("cafe.pdf"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(jp::encryptPdf(plainPdf(), p));
        }
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const auto reset = qScopeGuard([] { jp::setOpenPasswordStandIn(nullptr); });
        int asked = 0;
        jp::setOpenPasswordStandIn([&](const QString &, bool, QString *password) {
            ++asked;
            *password = QStringLiteral("cafe\u0301");
            return true;
        });
        w.insertFiles({path}, QPointF(-1, -1));
        QCOMPARE(asked, 1);
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), 1);
    }

    void aPdfThatAsksNotToBeCopiedIsPlacedWithANote()
    {
        QTemporaryDir dir;
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("pw");
        p.limited = true;
        p.ownerPassword = QStringLiteral("owner");
        p.allowCopy = false;
        const QString path = dir.filePath(QStringLiteral("nocopy.pdf"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(jp::encryptPdf(plainPdf(), p));
        }
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const auto reset = qScopeGuard([] { jp::setOpenPasswordStandIn(nullptr); });
        jp::setOpenPasswordStandIn([](const QString &, bool, QString *password) {
            *password = QStringLiteral("pw");
            return true;
        });
        w.insertFiles({path}, QPointF(-1, -1));
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), 1);
        QVERIFY2(w.statusBar()->currentMessage().contains(QStringLiteral("asks not to be copied")), qPrintable(w.statusBar()->currentMessage()));
        // A PDF with no such request gets no note.
        w.statusBar()->clearMessage();
        p.limited = false;
        const QString free = dir.filePath(QStringLiteral("free.pdf"));
        {
            QFile f(free);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(jp::encryptPdf(plainPdf(), p));
        }
        w.insertFiles({free}, QPointF(-1, -1));
        QVERIFY(!w.statusBar()->currentMessage().contains(QStringLiteral("asks not to be copied")));
    }

    void aLinkedPdfWithAPasswordIsNotAskedAboutOnOpeningOnlyOnUpdateLink()
    {
        QTemporaryDir dir;
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("s3cret");
        const QString path = dir.filePath(QStringLiteral("linked.pdf"));
        auto writeLocked = [&](const QString &title) {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(jp::encryptPdf(plainPdf(title), p));
        };
        writeLocked(QStringLiteral("first"));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->allowFolder(dir.path());
        // The publication holds a copy of the page (as the link was made), and the file has since changed.
        const QString id = doc->addLinkedImage(plainPdf(), QStringLiteral("pdf"), path, true, 0);
        auto pic = std::make_shared<jp::PictureItem>();
        pic->imageId = id;
        pic->rect = QRectF(50, 50, 200, 200);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        doc->pages[0]->items.push_back(pic);
        QFile touch(path);
        QVERIFY(touch.open(QIODevice::Append));
        touch.write("\n%%changed\n");
        touch.close();
        int asked = 0;
        jp::setOpenPasswordStandIn([&](const QString &, bool, QString *password) {
            ++asked;
            *password = QStringLiteral("s3cret");
            return true;
        });
        const auto reset = qScopeGuard([] { jp::setOpenPasswordStandIn(nullptr); });
        const QByteArray held = doc->images[id].bytes;
        doc->refreshLinks();   // (opening a publication does this)
        QCOMPARE(asked, 0);
        QCOMPARE(doc->linkStatus(id), jp::LinkStatus::NotUpdated);
        QCOMPARE(doc->images[id].bytes, held);
        // Update Link is the person's, and asks.
        jp::MainWindow w;
        w.editor()->setDocument(std::move(doc), path);
        QVERIFY(w.editor()->updateLink(pic->id, &w));
        QCOMPARE(asked, 1);
        const auto *placed = static_cast<jp::PictureItem *>(w.editor()->doc()->pages[0]->items[0].get());
        QCOMPARE(w.editor()->doc()->linkStatus(placed->imageId), jp::LinkStatus::Linked);
        QVERIFY(jp::PdfDocument(w.editor()->doc()->images[placed->imageId].bytes).isValid());
    }

    void theProtectionIsWipedWhenAsked()
    {
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("abc");
        p.ownerPassword = QStringLiteral("def");
        QVERIFY(p.isSet());
        p.wipe();
        QVERIFY(!p.isSet());
        QVERIFY(p.ownerPassword.isEmpty());
    }
};

JP_TEST_CLASS(PubPdfCryptTests)
#include "tst_pub_pdfcrypt.moc"
