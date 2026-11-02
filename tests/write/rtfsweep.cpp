// Opens each file given (a .rtf, .docx, or .doc, found by its content) the way JeffWrite does, and prints its text, for the sweep over many files that checks
// every one opens with no crash or hang and that an .rtf says what the file it was made from says. One file for each run: a crash or a hang is that run's.
//   rtfsweep file            the text, on standard output; the reason on standard error and exit code 2 if it can't be opened
//   rtfsweep --shot out.png [--page N] file   page N (1 by default) of the file in a window, as a picture; a message about what was left out is not shown
//   rtfsweep --count file    "words paragraphs notes" instead of the text
#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/writedoc.h"
#include "rtf/rtf.h"

#include <QApplication>
#include <QFile>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTimer>
#include <cstdio>

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("JP_NO_TELEMETRY", "1");
    QApplication app(argc, argv);
    const QStringList args = app.arguments();
    const bool count = args.contains(QStringLiteral("--count"));
    QString path;
    for (int i = 1; i < args.size(); ++i)
        if (!args[i].startsWith(QLatin1String("--")) && !(i > 0 && (args[i - 1] == QLatin1String("--shot") || args[i - 1] == QLatin1String("--page")))) path = args[i];
    if (const int at = int(args.indexOf(QStringLiteral("--shot"))); at >= 0 && at + 1 < args.size()) {
        const QString out = args[at + 1];
        const int pageAt = int(args.indexOf(QStringLiteral("--page")));
        const int page = pageAt >= 0 && pageAt + 1 < args.size() ? args[pageAt + 1].toInt() : 1;
        jp::WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        if (!w.openFile(path)) return 2;
        w.resize(900, 1150);
        w.show();
        QTimer::singleShot(300, &w, [&w, out, page] {
            w.view()->paginateAll();
            w.view()->setZoomMode(jp::write::PageView::ZoomMode::OnePage);
            w.view()->goToPage(page - 1);
            w.view()->setCaretWithoutFocus(true);
            QTimer::singleShot(300, &w, [&w, out] {
                w.grab().save(out);
                QApplication::exit(0);
            });
        });
        return app.exec();
    }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        std::fprintf(stderr, "cannot read\n");
        return 2;
    }
    const QByteArray bytes = f.readAll();
    if (args.contains(QStringLiteral("--package"))) {
        // The .docx package the .rtf comes to, on standard output.
        const QByteArray zip = jp::office::rtf::toDocxPackage(bytes);
        std::fwrite(zip.constData(), 1, size_t(zip.size()), stdout);
        return 0;
    }
    QString error;
    QStringList notes;
    const auto doc = jp::write::WriteDoc::fromFile(bytes, &error, &notes);
    if (!doc) {
        std::fprintf(stderr, "%s\n", qPrintable(error));
        return 2;
    }
    for (const QString &n : notes) std::fprintf(stderr, "note: %s\n", qPrintable(n));
    const QString text = doc->story()->toPlainText();
    if (count) std::printf("%lld %d %d\n", qint64(text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size()), int(doc->story()->blockCount()), int(notes.size()));
    else std::fwrite(text.toUtf8().constData(), 1, size_t(text.toUtf8().size()), stdout);
    return 0;
}
