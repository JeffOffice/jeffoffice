// The check of what a saved document keeps (docs/features/jeffwrite.md), one document for each run, so that a crash or a hang is that run's:
//   jwritetool save-check <file> [out-dir]   opens the file as JeffWrite does (any kind it reads: .docx, .doc, .rtf, .odt), saves it as .docx and as .odt, reads each saved file again with
//                                            the program's own reader, and compares what a person would see: the text of every paragraph in order, the pages, and how many tables,
//                                            pictures (and other drawn objects), and notes there are. One line for each format, tab-separated: `save-check`, the path, the format,
//                                            the result (`ok`, `differs`, `not saved`, `does not open again`, or `not opened`), the bytes saved, the paragraphs, the pages, the tables, the
//                                            pictures, the notes (each as opened, then as read again), then the kinds of difference (separated by "; ") what differs in each kind, and what the reader says it left out (or, where it did not open again, the reason).
//                                            With an out-dir the files saved are kept there, named <n>-docx.docx and <n>-odt.odt, <n> being JW_SAVE_CHECK_NAME or `file`.
// (tools/write_save_checks.py samples the collections, runs this for each file under a time limit and a memory cap, and opens each saved file in LibreOffice.)

#include "core/tableops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "core/fonts.h"
#include "app/writeinfo.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextStream>

#include <set>

using namespace jp;
using namespace jp::write;

namespace {

// What a person would see of a document, in numbers and words.
struct Look {
    QStringList paragraphs;   // the text of each, in order (the places of drawn objects are left out)
    int pages = 0, tables = 0, pictures = 0, notes = 0;
};

Look lookOf(WriteDoc &doc)
{
    Look look;
    QTextDocument *story = doc.story();
    std::set<const void *> tables;
    int endnotes = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        QString t = b.text();
        t.remove(QChar::ObjectReplacementCharacter);
        look.paragraphs << t;
        if (const int key = cellKeyOf(b)) {
            if (const flow::CellRecord *r = doc.flow().tables.get(key)) tables.insert(r->table.get());
        }
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat f = it.fragment().charFormat();
            if (!f.hasProperty(prop::Atom)) continue;
            const QString tag = unpackAtom(storedBytesOf(f.property(prop::Atom).toInt())).tag;
            if (tag == QLatin1String("pict") || tag == QLatin1String("drawing") || tag == QLatin1String("object") || tag == QLatin1String("AlternateContent")) ++look.pictures;
            else if (tag == QLatin1String("endnoteReference")) ++endnotes;
        }
    }
    look.tables = int(tables.size());
    look.notes = int(doc.citedFootnotes().size()) + endnotes;
    look.pages = doc.pager().pageCount();
    return look;
}

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

QString clean(QString s)
{
    return s.replace(QLatin1Char('\n'), QLatin1Char(' ')).replace(QLatin1Char('\r'), QLatin1Char(' ')).replace(QLatin1Char('\t'), QLatin1Char(' '));
}

} // namespace

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("JP_NO_TELEMETRY", "1");
    QApplication app(argc, argv);
    const QStringList args = app.arguments();
    if (args.size() < 3 || args[1] != QLatin1String("save-check")) {
        QTextStream(stderr) << "usage: jwritetool save-check <file> [out-dir]\n";
        return 64;
    }
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(write);
    setAppInfo(writeAppInfo());
    initCore();   // (the program's own fonts: the pages are counted with them)
    const QString path = args[2], outDir = args.value(3);
    const QString name = qEnvironmentVariable("JW_SAVE_CHECK_NAME", QStringLiteral("file"));
    auto line = [&](const QString &format, const QString &result, const QStringList &rest = QStringList()) {
        out() << "save-check\t" << path << '\t' << format << '\t' << result;
        for (const QString &r : rest) out() << '\t' << clean(r);
        out() << '\n';
        out().flush();
    };
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        line(QStringLiteral("docx"), QStringLiteral("not opened"), {QStringLiteral("cannot read")});
        return 1;
    }
    const QByteArray bytes = f.readAll();
    QString why;
    std::unique_ptr<WriteDoc> doc = WriteDoc::fromFile(bytes, &why, nullptr, QFileInfo(path).absolutePath());
    if (!doc) {
        line(QStringLiteral("docx"), QStringLiteral("not opened"), {why});
        line(QStringLiteral("odt"), QStringLiteral("not opened"), {why});
        return 1;
    }
    const Look before = lookOf(*doc);
    int code = 0;
    for (const QString format : {QStringLiteral("docx"), QStringLiteral("odt")}) {
        QString error;
        QByteArray saved;
        if (format == QLatin1String("docx")) saved = doc->toDocx(&error);
        else saved = doc->toOdt(&error);
        if (saved.isEmpty()) {
            line(format, QStringLiteral("not saved"), {error});
            code = 3;
            continue;
        }
        if (!outDir.isEmpty()) {
            QDir().mkpath(outDir);
            QFile o(QDir(outDir).filePath(name + QLatin1Char('-') + format + QLatin1Char('.') + format));
            if (o.open(QIODevice::WriteOnly)) o.write(saved);
        }
        QStringList readNotes;
        std::unique_ptr<WriteDoc> back = format == QLatin1String("docx") ? WriteDoc::fromDocx(saved, &error, &readNotes) : WriteDoc::fromOdt(saved, &error, &readNotes);
        if (!back) {
            line(format, QStringLiteral("does not open again"), {QString::number(saved.size()), QString(), QString(), QString(), QString(), QString(), QString(), error});
            code = 3;
            continue;
        }
        const Look after = lookOf(*back);
        QStringList kinds;
        QString first;
        auto differ = [&](const QString &kind, const QString &what) {
            if (!kinds.contains(kind)) kinds << kind;
            first += (first.isEmpty() ? QString() : QStringLiteral(" | ")) + what;
        };
        // (the first paragraph that is not the same, where the counts differ too)
        const auto shown = [](const QStringList &l, int i) { return i < l.size() ? l[i].left(30) : QStringLiteral("(none)"); };
        if (before.paragraphs.size() != after.paragraphs.size()) {
            int i = 0;
            while (i < before.paragraphs.size() && i < after.paragraphs.size() && before.paragraphs[i] == after.paragraphs[i]) ++i;
            differ(QStringLiteral("paragraph count"), QStringLiteral("%1 paragraphs, then %2; from paragraph %3: \"%4\" then \"%5\"").arg(before.paragraphs.size()).arg(after.paragraphs.size()).arg(i + 1).arg(shown(before.paragraphs, i), shown(after.paragraphs, i)));
        } else {
            for (int i = 0; i < before.paragraphs.size(); ++i)
                if (before.paragraphs[i] != after.paragraphs[i]) {
                    differ(QStringLiteral("paragraph text"), QStringLiteral("paragraph %1: \"%2\" then \"%3\"").arg(i + 1).arg(shown(before.paragraphs, i), shown(after.paragraphs, i)));
                    break;
                }
        }
        if (before.pages != after.pages) differ(QStringLiteral("page count"), QStringLiteral("%1 pages, then %2").arg(before.pages).arg(after.pages));
        if (before.tables != after.tables) differ(QStringLiteral("table count"), QStringLiteral("%1 tables, then %2").arg(before.tables).arg(after.tables));
        if (before.pictures != after.pictures) differ(QStringLiteral("picture count"), QStringLiteral("%1 pictures, then %2").arg(before.pictures).arg(after.pictures));
        if (before.notes != after.notes) differ(QStringLiteral("note count"), QStringLiteral("%1 notes, then %2").arg(before.notes).arg(after.notes));
        const auto n = [](int a, int b) { return QStringLiteral("%1>%2").arg(a).arg(b); };
        line(format, kinds.isEmpty() ? QStringLiteral("ok") : QStringLiteral("differs"),
             {QString::number(saved.size()), n(before.paragraphs.size(), after.paragraphs.size()), n(before.pages, after.pages), n(before.tables, after.tables), n(before.pictures, after.pictures), n(before.notes, after.notes),
              kinds.join(QStringLiteral("; ")), first, readNotes.join(QStringLiteral(" | "))});
    }
    return code;
}
