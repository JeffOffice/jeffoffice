// Passwords for PDFs: the shared function (libs/base/io/pdfcrypt.h), the protection row of the export options, and opening a PDF that has a password.
#include "pubhelpers.h"
#include "app/passworddialogs.h"
#include "app/pdfprotection.h"
#include "io/pdfcrypt.h"
#include <private/qabstractfileengine_p.h>
#include <QBuffer>
#include <QDirIterator>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QScopeGuard>
#include <QSettings>
#include <QTemporaryDir>

// A one-page PDF as Qt writes it, in memory.
static QByteArray plainPdf()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    {
        QPdfWriter pdf(&buffer);
        pdf.setPageSize(QPageSize(QPageSize::A5));
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

// Looks, each time Qt makes a file object, at the PDFs in a folder: one that is not encrypted is written down.
struct PlainPdfWatch : QAbstractFileEngineHandler {
    QString dir;
    mutable bool busy = false;
    mutable QStringList plain;
    std::unique_ptr<QAbstractFileEngine> create(const QString &) const override
    {
        if (busy || dir.isEmpty()) return nullptr;
        busy = true;
        const QStringList names = QDir(dir).entryList(QDir::Files);
        for (const QString &name : names) {
            QFile in(QDir(dir).filePath(name));
            if (!in.open(QIODevice::ReadOnly)) continue;
            const QByteArray bytes = in.readAll();
            if (bytes.startsWith("%PDF-") && !bytes.contains("/Encrypt")) plain << name;
        }
        busy = false;
        return nullptr;
    }
};

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

class PubPdfCryptTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    void theFunctionEncryptsWithAes256()
    {
        const QByteArray plain = plainPdf();
        QVERIFY(plain.startsWith("%PDF-"));
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
        // The text of the page is not in the file as it was.
        QVERIFY(!made.contains("Quarterly"));
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

    void noUnprotectedFileIsLeftBehind()
    {
        // The function works on bytes: a folder of its own for temporary files is as empty after as before.
        QTemporaryDir scratch;
        QVERIFY(scratch.isValid());
        struct Env {
            QByteArray name, old;
            bool had;
            Env(const char *n, const QString &v) : name(n), old(qgetenv(n)), had(qEnvironmentVariableIsSet(n)) { qputenv(n, v.toLocal8Bit()); }
            ~Env() { if (had) qputenv(name, old); else qunsetenv(name); }
        };
        const Env a("TMPDIR", scratch.path()), b("TEMP", scratch.path()), c("TMP", scratch.path());
        const QByteArray plain = plainPdf();
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("x");
        QVERIFY(!jp::encryptPdf(plain, p).isEmpty());
        QVERIFY(QDir(scratch.path()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty());
    }

    void anExportWithAPasswordIsEncryptedAndNotPdfA()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QTemporaryDir dir, keep;
        jp::MainWindow::PdfSettings s;
        s.archival = true;
        // Control: an ordinary PDF/A export has the mark, and no encryption.
        QVERIFY(w.exportPdfTo(keep.filePath(QStringLiteral("a.pdf")), s));
        QVERIFY(bytesOf(keep.filePath(QStringLiteral("a.pdf"))).contains("pdfaid:part"));
        const QString path = dir.filePath(QStringLiteral("locked.pdf"));
        s.protect.openPassword = QStringLiteral("pw");
        PlainPdfWatch watch;
        watch.dir = dir.path();
        QVERIFY(w.exportPdfTo(path, s));
        watch.dir.clear();
        QVERIFY2(watch.plain.isEmpty(), "a PDF that is not encrypted was on the disk");
        const QByteArray made = bytesOf(path);
        jp::PdfEncryptionInfo info;
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("pw"), &info));
        QCOMPARE(info.version, 5);
        QCOMPARE(info.revision, 6);
        QVERIFY(!jp::PdfDocument(made).isValid());
        QVERIFY(jp::PdfDocument(made, QStringLiteral("pw")).isValid());
        // PDF/A was turned off: the mark is not in what the password opens.
        QVERIFY(!jp::decryptPdf(made, QStringLiteral("pw")).contains("pdfaid:part"));
        // Nothing else is left in the folder, and the settings passed in are as the caller has them (the caller wipes them).
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files), QStringList{QStringLiteral("locked.pdf")});
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
        // Not in the settings, whatever the export did.
        QSettings kept;
        for (const QString &key : kept.allKeys()) QVERIFY2(!kept.value(key).toString().contains(QStringLiteral("plum-7391")), qPrintable(key));
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
