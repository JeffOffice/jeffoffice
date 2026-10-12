// Command-line helper for testing JeffDeck's reading of presentation files.
//   jdecktool render <file.pptx> <outdir> [width=1280]   each slide to slide-001.png ...
//   jdecktool info <file.pptx>                           the masters, layouts, and slides found
//   jdecktool save <file> <out.pptx>                     read it and save it again, listing what was left out
//   jdecktool pdf <file> <out.pdf> [slides|notes|handout6]   print it to a PDF (also handout1|2|3|4|9, outline; "hidden", "frame", "gray", "down" add options; "samplenotes" gives slides without notes some)

#include "app/deckinfo.h"
#include "core/appinfo.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "io/deckoutput.h"
#include "io/pptxexport.h"
#include "io/pptximport.h"
#include "render/renderer.h"

#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QHash>
#include <QJsonArray>
#include <QTextDocument>
#include <QTextStream>

using namespace jp;

namespace {

// The objects on a page, those inside groups counted too.
int countObjects(const ItemList &items)
{
    int n = 0;
    for (const auto &it : items) {
        ++n;
        if (it->type() == ItemType::Group) n += countObjects(static_cast<const GroupItem &>(*it).children);
    }
    return n;
}

int unreadFrames(const PageBase &page) { return int(page.props.value(QStringLiteral("unread")).toArray().size()); }

// The placeholders of a page, each with its place: type[:index]@x,y,wxh in points.
QString placeholders(const PageBase &page)
{
    QStringList out;
    for (const auto &it : page.items)
        if (it->role.startsWith(QLatin1String("ph:")))
            out << QStringLiteral("%1@%2,%3,%4x%5").arg(it->role.mid(3)).arg(it->rect.x(), 0, 'f', 0).arg(it->rect.y(), 0, 'f', 0).arg(it->rect.width(), 0, 'f', 0).arg(it->rect.height(), 0, 'f', 0);
    return out.join(QLatin1Char(' '));
}

} // namespace

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(resources);   // the font measurements, in the shared libraries
    // No window, and no typing: no input method either (Qt's IBus plugin can wait 15-20 seconds for a slow daemon).
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
    if (!qEnvironmentVariableIsSet("QT_IM_MODULE")) qputenv("QT_IM_MODULE", "none");
    QGuiApplication app(argc, argv);
    setAppInfo(deckAppInfo());
    initCore();
    const QStringList args = app.arguments();
    QTextStream out(stdout);
    if (args.size() < 3 || ((args[1] == "render" || args[1] == "save" || args[1] == "pdf") && args.size() < 4) ||
        (args[1] != "render" && args[1] != "info" && args[1] != "save" && args[1] != "pdf")) {
        out << "usage: jdecktool render <file.pptx> <outdir> [width=1280] | info <file.pptx> | save <file> <out.pptx> | pdf <file> <out.pdf> [slides|notes|handout6 ...]\n";
        return 2;
    }
    const QString cmd = args[1], in = args[2];
    QElapsedTimer timer;
    timer.start();
    QString notes;
    std::unique_ptr<Document> doc = deck::importPresentationFile(in, &notes);
    if (!doc) {
        out << "cannot open: " << notes << "\n";
        return 1;
    }
    out << "opened\t" << doc->pages.size() << " slides\t" << deck::layouts(*doc).size() << " layouts\t" << timer.elapsed() << " ms\n";
    if (!notes.isEmpty()) out << "notes:\n" << notes << "\n";

    if (cmd == "save") {
        QString error;
        QStringList skipped;
        const bool ok = deck::savePresentationFile(*doc, args[3], &error, &skipped);
        for (const QString &s : skipped) out << "left out\t" << s << "\n";
        out << (ok ? "saved\t" + args[3] : "cannot save: " + error) << "\t" << timer.elapsed() << " ms\n";
        return ok ? 0 : 1;
    }

    if (cmd == "pdf") {
        // What to print: a layout, and any of hidden, frame, and gray.
        static const QHash<QString, deck::PrintLayout> layouts = {
            {"slides", deck::PrintLayout::Slides},     {"notes", deck::PrintLayout::NotesPages}, {"handout1", deck::PrintLayout::Handout1},
            {"handout2", deck::PrintLayout::Handout2}, {"handout3", deck::PrintLayout::Handout3}, {"handout4", deck::PrintLayout::Handout4},
            {"handout6", deck::PrintLayout::Handout6}, {"handout9", deck::PrintLayout::Handout9}, {"outline", deck::PrintLayout::Outline}};
        deck::OutputOptions options;
        for (int i = 4; i < args.size(); ++i) {
            if (layouts.contains(args[i])) options.layout = layouts[args[i]];
            else if (args[i] == "hidden") options.hiddenSlides = true;
            else if (args[i] == "frame") options.frameSlides = true;
            else if (args[i] == "gray") options.grayscale = true;
            else if (args[i] == "down") options.horizontalOrder = false;
            else if (args[i] == "samplenotes") {
                // To see notes pages of a presentation that has none: a few lines on each slide, many on every fifth.
                for (int k = 0; k < doc->pages.size(); ++k) {
                    Page &slide = *doc->pages[k];
                    if (!deck::notesText(*doc, slide).isEmpty()) continue;
                    QStringList paragraphs;
                    for (int n = 0; n < (k % 5 == 4 ? 40 : 1 + k % 3); ++n)
                        paragraphs << QStringLiteral("Say that slide %1 shows the third quarter's results, and mention point %2 before moving on to the next topic.").arg(k + 1).arg(n + 1);
                    slide.notesStoryId = doc->createStory(paragraphs.join(QLatin1Char('\n')));
                }
            }
            else {
                out << "unknown option: " << args[i] << "\n";
                return 2;
            }
        }
        QString error;
        const bool ok = deck::exportPdf(*doc, args[3], options, &error);
        out << (ok ? "wrote\t" + args[3] + "\t" + QString::number(deck::outputPageCount(*doc, options)) + " pages" : "cannot write: " + error) << "\t" << timer.elapsed() << " ms\n";
        return ok ? 0 : 1;
    }

    if (cmd == "info") {
        out << "size\t" << doc->pageSize().width() << " x " << doc->pageSize().height() << " pt\t" << doc->setup.sizeName << "\n";
        const deck::DeckTheme theme = deck::themeOf(*doc);
        out << "theme\t" << theme.name << "\t" << theme.headingFont << " / " << theme.bodyFont << "\n";
        for (const auto &m : doc->masters) {
            if (deck::isSlideMaster(*m))
                out << "master\t" << m->id << "\t" << m->name << "\t" << m->items.size() << " objects\t" << placeholders(*m) << "\n";
            else
                out << "layout\t" << m->id << "\t" << m->name << "\ttype " << deck::layoutType(*m) << "\ton " << m->baseId << "\t" << m->items.size() << " objects"
                    << (m->showMasterItems ? "" : "\tno master graphics") << "\t" << placeholders(*m) << "\n";
        }
        for (int i = 0; i < doc->pages.size(); ++i) {
            const Page &p = *doc->pages[i];
            const MasterPage *layout = doc->masterFor(p);
            out << "slide\t" << i + 1 << "\t" << (layout ? layout->name : QStringLiteral("(no layout)")) << "\t" << p.items.size() << " objects\t"
                << placeholders(p) << (p.hidden ? "\thidden" : "") << (p.showMasterItems ? "" : "\tno master graphics") << (p.background.isNone() ? "" : "\tbackground")
                << (unreadFrames(p) ? QStringLiteral("\t%1 unread").arg(unreadFrames(p)) : QString())
                << (deck::notesText(*doc, p).isEmpty() ? QString() : QStringLiteral("\tnotes")) << "\n";
        }
        return 0;
    }

    // The width in pixels, as 1280 or width=1280.
    const double width = args.size() > 4 ? std::max(16.0, QString(args[4]).remove(QStringLiteral("width=")).toDouble()) : 1280;
    QDir().mkpath(args[3]);
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = doc.get();
    ctx.cache = &cache;
    ctx.opt.output = true;
    for (int i = 0; i < doc->pages.size(); ++i) {
        timer.restart();
        const QImage img = Renderer::renderToImage(ctx, i, width / doc->pageSize().width());
        const QString name = QStringLiteral("slide-%1.png").arg(i + 1, 3, 10, QLatin1Char('0'));
        img.save(QDir(args[3]).filePath(name));
        out << name << "\tobjects=" << countObjects(doc->pages[i]->items) << "\tunread=" << unreadFrames(*doc->pages[i]) << "\t" << timer.elapsed() << " ms\n";
    }
    return 0;
}
