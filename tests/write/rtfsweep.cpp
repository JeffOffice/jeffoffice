// Opens each file given (a .rtf, .docx, or .doc, found by its content) the way JeffWrite does, and prints its text, for the sweep over many files that checks
// every one opens with no crash or hang and that an .rtf says what the file it was made from says. One file for each run: a crash or a hang is that run's.
//   rtfsweep file            the text, on standard output; the reason on standard error and exit code 2 if it can't be opened
//   rtfsweep --count file    "words paragraphs notes" instead of the text
#include "core/writedoc.h"

#include <QApplication>
#include <QFile>
#include <QRegularExpression>
#include <QTextDocument>
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
        if (!args[i].startsWith(QLatin1String("--"))) path = args[i];
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        std::fprintf(stderr, "cannot read\n");
        return 2;
    }
    const QByteArray bytes = f.readAll();
    QString error;
    QStringList notes;
    const auto doc = jp::write::WriteDoc::fromFile(bytes, &error, &notes);
    if (!doc) {
        std::fprintf(stderr, "%s\n", qPrintable(error));
        return 2;
    }
    const QString text = doc->story()->toPlainText();
    if (count) std::printf("%lld %d %d\n", qint64(text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size()), int(doc->story()->blockCount()), int(notes.size()));
    else std::fwrite(text.toUtf8().constData(), 1, size_t(text.toUtf8().size()), stdout);
    return 0;
}
