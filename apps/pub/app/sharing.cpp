// Share and export helpers: email files, Pack and Go, photo printer pictures,
// and templates.

#include "core/fonts.h"
#include "app/appfuncs.h"
#include "app/editor.h"
#include "app/mainwindow.h"
#include "io/jpubfile.h"
#include "io/zip.h"
#include "render/renderer.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFontDatabase>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTemporaryDir>
#include <QTextDocument>

namespace jp {

// A mail header value: one line (a line break would start a header of its
// own), with anything beyond ASCII encoded as mail headers require (RFC 2047).
QString emlHeaderText(const QString &s)
{
    QString one = s;
    one.replace(QLatin1Char('\r'), QLatin1Char(' ')).replace(QLatin1Char('\n'), QLatin1Char(' ')).replace(QLatin1Char('"'), QLatin1Char('\''));
    for (QChar c : one)
        if (c.unicode() >= 0x80 || c.unicode() < 0x20)
            return QStringLiteral("=?UTF-8?B?") + QString::fromLatin1(one.toUtf8().toBase64()) + QStringLiteral("?=");
    return one;
}

static QString emlEncode(const QByteArray &data)
{
    QString out;
    const QByteArray b64 = data.toBase64();
    for (int i = 0; i < b64.size(); i += 76) out += QString::fromLatin1(b64.mid(i, 76)) + "\r\n";
    return out;
}

static void saveEml(QWidget *parent, const QString &name, const QString &body)
{
    const QString path = askSavePath(parent, QCoreApplication::translate("Sharing", "Save Email"), name + ".eml", QCoreApplication::translate("Sharing", "Email Message (*.eml)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) f.write(body.toUtf8());
    QMessageBox::information(parent, QCoreApplication::translate("Sharing", "Email"), QCoreApplication::translate("Sharing", "Saved %1.\nOpen it in your email program, add recipients and send.").arg(QFileInfo(path).fileName()));
}

void emailCurrentPage(QWidget *parent, Editor *ed)
{
    PaintContext ctx;
    ctx.doc = ed->doc();
    ctx.cache = &ed->cache();
    ctx.opt.output = true;
    const QImage img = Renderer::renderToImage(ctx, ed->currentPage(), 1.5);
    QByteArray png;
    QBuffer b(&png);
    b.open(QIODevice::WriteOnly);
    img.save(&b, "PNG");
    QString eml = QStringLiteral("Subject: %1\r\nX-Unsent: 1\r\nMIME-Version: 1.0\r\nContent-Type: multipart/related; boundary=\"jp79\"\r\n\r\n").arg(emlHeaderText(ed->displayName()));
    eml += "--jp79\r\nContent-Type: text/html; charset=utf-8\r\n\r\n<html><body><img src=\"cid:page\" alt=\"\"></body></html>\r\n";
    eml += "--jp79\r\nContent-Type: image/png\r\nContent-Transfer-Encoding: base64\r\nContent-ID: <page>\r\n\r\n" + emlEncode(png) + "--jp79--\r\n";
    saveEml(parent, ed->displayName(), eml);
}

void emailAsAttachment(QWidget *parent, Editor *ed, const QString &format)
{
    QByteArray data;
    QString fileName, mime;
    if (format == "pdf") {
        // A private folder of its own: a fixed name in the shared temporary
        // folder could be a link another user placed there.
        QTemporaryDir tmpDir;
        const QString tmp = tmpDir.filePath(QStringLiteral("share.pdf"));
        auto *win = qobject_cast<MainWindow *>(parent->window());
        if (win && tmpDir.isValid()) win->exportPdf(tmp);
        QFile f(tmp);
        if (f.open(QIODevice::ReadOnly)) data = f.readAll();
        fileName = ed->displayName() + ".pdf";
        mime = "application/pdf";
    } else {
        data = publicationBytes(*ed->doc(), QImage(), QString(), true);
        fileName = ed->displayName() + ".jpub";
        mime = "application/x-jeffpub";
    }
    QString eml = QStringLiteral("Subject: %1\r\nX-Unsent: 1\r\nMIME-Version: 1.0\r\nContent-Type: multipart/mixed; boundary=\"jp79\"\r\n\r\n").arg(emlHeaderText(ed->displayName()));
    eml += "--jp79\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n" + QStringLiteral("Attached: %1").arg(fileName) + "\r\n";
    eml += QStringLiteral("--jp79\r\nContent-Type: %1; name=\"%2\"\r\nContent-Disposition: attachment; filename=\"%2\"\r\nContent-Transfer-Encoding: base64\r\n\r\n").arg(mime, emlHeaderText(fileName));
    eml += emlEncode(data) + "--jp79--\r\n";
    saveEml(parent, ed->displayName(), eml);
}

QString userBlocksDir() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/BuildingBlocks"); }

bool packForPrinter(MainWindow *win, const QString &dir, QString *error)
{
    Editor *ed = win->editor();
    const QString base = QDir(dir).filePath(ed->displayName());
    // The commercial press PDF: full-resolution pictures, printer's marks and bleeds.
    MainWindow::PdfSettings s;
    s.preset = MainWindow::PdfSettings::CommercialPress;
    if (!win->exportPdfTo(base + ".pdf", s)) {
        if (error) *error = QCoreApplication::translate("Sharing", "JeffPub couldn't save the PDF in %1.").arg(QDir::toNativeSeparators(dir));
        return false;
    }
    QString err;
    if (!savePublication(*ed->doc(), base + ".jpub", win->pageThumbnail(0, 256), &err, true)) {
        if (error) *error = QCoreApplication::translate("Sharing", "JeffPub couldn't save the publication in %1.\n%2").arg(QDir::toNativeSeparators(dir), err);
        return false;
    }
    QFile readme(QDir(dir).filePath("README-for-printer.txt"));
    const QByteArray note = QStringLiteral("Publication: %1\nPage size: %2 x %3 inches, %4 pages\nThe PDF has crop, bleed, and registration marks, color bars, and job information outside the page.\n"
                                           "PDF fonts are embedded. Pictures are at full resolution.\n")
                                .arg(ed->displayName()).arg(ed->doc()->pageSize().width() / 72, 0, 'f', 3).arg(ed->doc()->pageSize().height() / 72, 0, 'f', 3)
                                .arg(ed->doc()->pages.size()).toUtf8();
    if (!readme.open(QIODevice::WriteOnly) || readme.write(note) != note.size()) {
        if (error) *error = QCoreApplication::translate("Sharing", "JeffPub couldn't save the note for your printer in %1.").arg(QDir::toNativeSeparators(dir));
        return false;
    }
    return true;
}

bool packToZip(MainWindow *win, const QString &path, int *fontCount, QString *error)
{
    Editor *ed = win->editor();
    ZipWriter z;
    z.add(ed->displayName() + ".jpub", publicationBytes(*ed->doc(), win->pageThumbnail(0, 256), QString(), true));
    // Include the font files the publication uses, when they are bundled or installed as files.
    QSet<QString> families;
    for (auto it = ed->doc()->stories.cbegin(); it != ed->doc()->stories.cend(); ++it)
        for (QTextBlock b = (*it)->doc->begin(); b.isValid(); b = b.next())
            for (auto f = b.begin(); !f.atEnd(); ++f)
                for (const QString &fam : f.fragment().charFormat().fontFamilies().toStringList()) families.insert(fam);
    families.insert(ed->doc()->fonts.heading);
    families.insert(ed->doc()->fonts.body);
    const QStringList fontDirs = bundledFontDirs();
    int fonts = 0;
    for (const QString &d : fontDirs) {
        for (const QFileInfo &fi : QDir(d).entryInfoList({"*.ttf", "*.otf"}, QDir::Files)) {
            const QString stem = fi.completeBaseName().section('-', 0, 0);
            for (const QString &fam : families)
                if (QString(fam).remove(' ').compare(stem, Qt::CaseInsensitive) == 0) {
                    QFile f(fi.absoluteFilePath());
                    if (f.open(QIODevice::ReadOnly)) { z.add("fonts/" + fi.fileName(), f.readAll()); ++fonts; }
                }
        }
        if (fonts) break;
    }
    if (fontCount) *fontCount = fonts;
    const QByteArray bytes = z.finish();
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size() || !out.commit()) {
        if (error) *error = QCoreApplication::translate("Sharing", "JeffPub couldn't save %1.\n%2").arg(QDir::toNativeSeparators(path), out.errorString());
        return false;
    }
    return true;
}

void packAndGo(QWidget *parent, MainWindow *win, bool forPrinter)
{
    Editor *ed = win->editor();
    if (forPrinter) {
        const QString dir = QFileDialog::getExistingDirectory(parent, QCoreApplication::translate("Sharing", "Save for a Commercial Printer"));
        if (dir.isEmpty()) return;
        QString err;
        if (!packForPrinter(win, dir, &err)) {
            QMessageBox::warning(parent, QCoreApplication::translate("Sharing", "Pack and Go"), err);
            return;
        }
        QMessageBox::information(parent, QCoreApplication::translate("Sharing", "Pack and Go"), QCoreApplication::translate("Sharing", "Saved the PDF and publication for your printer in %1.").arg(dir));
        return;
    }
    const QString path = askSavePath(parent, QCoreApplication::translate("Sharing", "Save for Another Computer"), ed->displayName() + ".zip", QCoreApplication::translate("Sharing", "ZIP (*.zip)"));
    if (path.isEmpty()) return;
    int fonts = 0;
    QString err;
    if (!packToZip(win, path, &fonts, &err)) {
        QMessageBox::warning(parent, QCoreApplication::translate("Sharing", "Pack and Go"), err);
        return;
    }
    QMessageBox::information(parent, QCoreApplication::translate("Sharing", "Pack and Go"), QCoreApplication::translate("Sharing", "Saved %1 with the publication and %2 font file(s).").arg(QFileInfo(path).fileName()).arg(fonts));
}

void saveForPhotoPrinter(QWidget *parent, MainWindow *win)
{
    Editor *ed = win->editor();
    const QString dir = QFileDialog::getExistingDirectory(parent, QCoreApplication::translate("Sharing", "Save for a Photo Printer"));
    if (dir.isEmpty()) return;
    PaintContext ctx;
    ctx.doc = ed->doc();
    ctx.cache = &ed->cache();
    ctx.opt.output = true;
    for (int i = 0; i < ed->doc()->pages.size(); ++i) {
        QImage img = Renderer::renderToImage(ctx, i, 300.0 / 72);
        img.setDotsPerMeterX(11811);
        img.setDotsPerMeterY(11811);
        img.save(QDir(dir).filePath(QStringLiteral("%1-page%2.jpg").arg(ed->displayName()).arg(i + 1)), "JPG", 95);
    }
    QMessageBox::information(parent, QCoreApplication::translate("Sharing", "Save for a Photo Printer"), QCoreApplication::translate("Sharing", "Saved %1 picture(s) at 300 dpi.").arg(ed->doc()->pages.size()));
}

void saveAsTemplate(QWidget *parent, MainWindow *win)
{
    bool ok = false;
    const QString name = QInputDialog::getText(parent, QCoreApplication::translate("Sharing", "Save as Template"), QCoreApplication::translate("Sharing", "Template name:"), QLineEdit::Normal, win->editor()->displayName(), &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Templates";
    QDir().mkpath(dir);
    QString err;
    if (savePublication(*win->editor()->doc(), dir + "/" + name.trimmed() + ".jpub", win->pageThumbnail(0, 256), &err, true))
        QMessageBox::information(parent, QCoreApplication::translate("Sharing", "Save as Template"), QCoreApplication::translate("Sharing", "\"%1\" is now in File > New > My Templates.").arg(name.trimmed()));
    else QMessageBox::warning(parent, QCoreApplication::translate("Sharing", "Save as Template"), err);
}

} // namespace jp
