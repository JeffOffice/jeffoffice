// Command-line helper for the flowing document (libs/flow): lays a plain text file out in pages, and measures how fast it does.
//   writetool render <file.txt|.docx> <outdir> [width=800] [options]   each page to page-001.png ...
//   writetool pdf <file.txt|.docx> <out.pdf> [options]                 the pages as a PDF
//   writetool info <file.txt> [options]                          the pages and where each starts
//   writetool docpkg <file.doc> <out.docx>                       the package an old binary document is turned into (to look at what the reader made of it)
//   writetool bench [paragraphs=26443] [options]                 a made-up long story: the time to its first page, to all of its pages, and to repaginate after a typed character,
//                                                                against the targets (all of 26,443 paragraphs in 5 s, the first page in 0.3 s, a typed character in 50 ms); exits 1 if one is missed
// The text: paragraphs are separated by blank lines; "# " and "## " start headings (kept with the paragraph after); "- " starts a bullet and
// "1. " a numbered item; "@page-break" puts a page break before the next paragraph; "@section [letter|a4|legal] [landscape] [columns N]" ends
// the section and starts the next with a page of its own.
// Options: columns=N, a4, legal, landscape, margin=INCHES, hyphenate, justify.

#include "core/fonts.h"
#include "flow/paginator.h"
#include "render/renderer.h"
#include "text/textprops.h"
#ifdef JW_DOCX
#include "core/writedoc.h"
#include "doc/doc.h"
#endif

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QSaveFile>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <QTextStream>
#include <algorithm>
#include <memory>
#include <random>

using namespace jp;
using namespace jp::flow;

namespace {

struct Options {
    int columns = 1;
    QSizeF paper{612, 792};
    bool landscape = false;
    double margin = 1;   // inches
    bool hyphenate = false;
    bool justify = false;
};

QSizeF paperNamed(const QString &name)
{
    if (name == QLatin1String("a4")) return QSizeF(595.28, 841.89);
    if (name == QLatin1String("legal")) return QSizeF(612, 1008);
    return QSizeF(612, 792);
}

Section sectionOf(const Options &o)
{
    Section s;
    s.paper = o.paper;
    s.orientation = o.landscape ? Section::Orientation::Landscape : Section::Orientation::Portrait;
    s.margins = QMarginsF(o.margin * 72, o.margin * 72, o.margin * 72, o.margin * 72);
    s.columns = std::max(1, o.columns);
    return s;
}

bool parseOption(const QString &a, Options *o)
{
    if (a.startsWith(QLatin1String("columns="))) o->columns = a.mid(8).toInt();
    else if (a == QLatin1String("a4") || a == QLatin1String("legal")) o->paper = paperNamed(a);
    else if (a == QLatin1String("landscape")) o->landscape = true;
    else if (a.startsWith(QLatin1String("margin="))) o->margin = a.mid(7).toDouble();
    else if (a == QLatin1String("hyphenate")) o->hyphenate = true;
    else if (a == QLatin1String("justify")) o->justify = true;
    else return false;
    return true;
}

QTextCharFormat bodyChar() { QTextCharFormat f; f.setFontPointSize(11); return f; }

QTextBlockFormat bodyBlock(const Options &o)
{
    QTextBlockFormat bf;
    bf.setBottomMargin(6);
    bf.setProperty(tp::WidowControl, true);
    if (o.justify) bf.setAlignment(Qt::AlignJustify);
    return bf;
}

// Appends a paragraph to the story.
void addParagraph(QTextDocument *story, const QString &text, const QTextBlockFormat &bf, const QTextCharFormat &cf, QTextListFormat::Style list = QTextListFormat::ListStyleUndefined)
{
    QTextCursor c(story);
    c.movePosition(QTextCursor::End);
    if (!story->isEmpty()) c.insertBlock(bf, cf); else { c.setBlockFormat(bf); c.setBlockCharFormat(cf); }
    c.insertText(text, cf);
    if (list != QTextListFormat::ListStyleUndefined) {
        QTextListFormat lf;
        lf.setStyle(list);
        lf.setIndent(1);
        c.createList(lf);
    }
}

bool loadText(FlowDocument &fd, const QString &path, Options o, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        *error = f.errorString();
        return false;
    }
    const QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
    fd.sections.clear();
    fd.sections << sectionOf(o);
    fd.hyphenate = o.hyphenate;
    QTextDocument *story = fd.story();
    QStringList para;
    bool pageBreak = false;
    QTextListFormat::Style list = QTextListFormat::ListStyleUndefined;
    auto flush = [&] {
        if (para.isEmpty()) return;
        QString text = para.join(QLatin1Char(' ')).simplified();
        para.clear();
        QTextBlockFormat bf = bodyBlock(o);
        QTextCharFormat cf = bodyChar();
        QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;
        if (text.startsWith(QLatin1String("## ")) || text.startsWith(QLatin1String("# "))) {
            const bool big = text.startsWith(QLatin1String("# "));
            text = text.mid(big ? 2 : 3);
            cf.setFontPointSize(big ? 20 : 15);
            cf.setFontWeight(QFont::Bold);
            bf.setTopMargin(big ? 18 : 12);
            bf.setBottomMargin(big ? 8 : 6);
            bf.setProperty(tp::KeepWithNext, true);
            bf.setProperty(tp::KeepTogether, true);
            bf.setAlignment(Qt::AlignLeft);
        } else if (text.startsWith(QLatin1String("- ")) || text.startsWith(QLatin1String("* "))) {
            text = text.mid(2);
            style = QTextListFormat::ListDisc;
            bf.setBottomMargin(3);
        } else {
            static const QRegularExpression numbered(QStringLiteral("^\\d+\\.\\s+"));
            const auto m = numbered.match(text);
            if (m.hasMatch()) {
                text = text.mid(m.capturedLength());
                style = QTextListFormat::ListDecimal;
                bf.setBottomMargin(3);
            }
        }
        if (pageBreak) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
        pageBreak = false;
        // Items of one list share it (a list of one kind goes on while the next paragraph is an item of that kind).
        if (style != QTextListFormat::ListStyleUndefined && style == list) {
            QTextCursor c(story);
            c.movePosition(QTextCursor::End);
            c.insertBlock(bf, cf);
            c.insertText(text, cf);
            QTextCursor prev(story->lastBlock().previous());
            if (QTextList *l = prev.currentList()) l->add(story->lastBlock());
        } else {
            addParagraph(story, text, bf, cf, style);
        }
        list = style;
    };
    for (const QString &raw : lines) {
        const QString line = raw.trimmed();
        if (line.startsWith(QLatin1Char('@'))) {
            flush();
            const QStringList w = line.mid(1).split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (w.value(0) == QLatin1String("page-break")) pageBreak = true;
            else if (w.value(0) == QLatin1String("section")) {
                if (story->isEmpty()) continue;
                QTextCursor c(story->lastBlock());
                QTextBlockFormat bf = c.blockFormat();
                bf.setProperty(fp::SectionBreakAfter, true);
                c.setBlockFormat(bf);
                Options next = o;
                next.landscape = false;
                for (int i = 1; i < w.size(); ++i) {
                    if (w[i] == QLatin1String("columns")) next.columns = w.value(++i).toInt();
                    else if (w[i] == QLatin1String("landscape")) next.landscape = true;
                    else next.paper = paperNamed(w[i]);
                }
                fd.sections << sectionOf(next);
            }
            continue;
        }
        if (line.isEmpty()) { flush(); continue; }
        // A heading or an item is a paragraph of its own, whatever follows.
        if (!para.isEmpty() && (line.startsWith(QLatin1String("#")) || line.startsWith(QLatin1String("- ")) || line.startsWith(QLatin1String("* ")))) flush();
        para << line;
        if (line.startsWith(QLatin1String("#")) || line.startsWith(QLatin1String("- ")) || line.startsWith(QLatin1String("* "))) flush();
    }
    flush();
    if (story->isEmpty()) {
        *error = QStringLiteral("no text");
        return false;
    }
    return true;
}

// A long made-up story: paragraphs of about 380 characters of ordinary words, and a heading in every twenty.
void makeBenchStory(QTextDocument *story, int paragraphs, bool justify)
{
    static const char *const words[] = {"the", "of", "and", "to", "in", "a", "is", "that", "for", "it", "as", "was", "with", "be", "by", "on", "not", "this", "are", "or", "from", "at", "which",
                                        "but", "have", "an", "had", "they", "you", "were", "their", "one", "all", "we", "can", "has", "there", "been", "if", "more", "when", "will", "would",
                                        "who", "so", "no", "court", "statute", "section", "rule", "appeal", "judgment", "evidence", "applicable", "jurisdiction", "pursuant", "therefore",
                                        "defendant", "plaintiff", "provision", "constitutional", "interpretation", "argument", "reasonable", "proceedings", "additional", "regulation",
                                        "publication", "manuscript", "chapter", "paragraph", "reference", "consideration", "requirement", "determination", "responsibility", "relationship"};
    const int n = sizeof words / sizeof *words;
    std::mt19937 rng(42);
    QTextCursor c(story);
    // (Fonts named, as the measurements of the text engine were made with: the time a layout takes depends on them.)
    QTextCharFormat body = bodyChar(), head;
    body.setFontFamilies({QStringLiteral("Liberation Serif")});
    head.setFontFamilies({QStringLiteral("Liberation Sans")});
    head.setFontPointSize(16);
    head.setFontWeight(QFont::Bold);
    c.beginEditBlock();
    for (int i = 0; i < paragraphs; ++i) {
        const bool isHead = i % 20 == 0;
        QTextBlockFormat bf;
        if (isHead) {
            bf.setTopMargin(12);
            bf.setBottomMargin(6);
            bf.setProperty(tp::KeepWithNext, true);
            bf.setProperty(tp::KeepTogether, true);
        } else {
            bf.setBottomMargin(6);
            bf.setProperty(tp::WidowControl, true);
            if (justify) bf.setAlignment(Qt::AlignJustify);
        }
        if (i == 0) c.setBlockFormat(bf); else c.insertBlock(bf);
        c.setBlockCharFormat(isHead ? head : body);
        const int count = isHead ? 4 : 40 + int(rng() % 41);
        QString text;
        for (int k = 0; k < count; ++k) text += (k ? " " : "") + QString::fromLatin1(words[rng() % n]);
        text[0] = text[0].toUpper();
        c.insertText(text + QLatin1Char('.'), isHead ? head : body);
    }
    c.endEditBlock();
}

long residentKb()
{
    QFile f(QStringLiteral("/proc/self/status"));
    if (!f.open(QIODevice::ReadOnly)) return 0;
    for (const QByteArray &l : f.readAll().split('\n'))
        if (l.startsWith("VmRSS:")) return l.mid(6).trimmed().split(' ').value(0).toLong();
    return 0;
}

QImage pageImage(Paginator &pg, LayoutCache *cache, int index, int width, bool pdf = false, int pageCount = 1)
{
    const auto page = pg.materialize(index);
    if (!page) return QImage();
    const QSizeF size = page->doc->pageSize();
    QImage img(QSize(width, std::max(1, int(std::lround(width * size.height() / size.width())))), QImage::Format_RGB32);
    img.fill(Qt::white);
    PaintContext ctx;
    ctx.doc = page->doc.get();
    ctx.cache = cache;
    ctx.pageNumber = page->number;   // (what a page's number fields show, as JeffWrite's page view draws them)
    ctx.pageCount = pageCount;
    ctx.opt.output = pdf;
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.scale(img.width() / size.width(), img.height() / size.height());
    Renderer::paintPage(&p, ctx, 0);
    TablePaint tp;
    tp.output = pdf ? TextOutput::Pdf : TextOutput::Picture;
    paintTables(&p, *page, tp);
    return img;
}

double ms(const QElapsedTimer &t) { return t.nsecsElapsed() / 1e6; }

// What the flowing document is to do with a story of 26,443 paragraphs (CI's tests allow ten times as much, for slow machines).
constexpr double kTargetFirstPageMs = 300, kTargetAllPagesMs = 5000, kTargetTypedMs = 50;

int bench(int paragraphs, const Options &o, QTextStream &out)
{
    FlowDocument fd;
    fd.sections[0] = sectionOf(o);
    fd.hyphenate = o.hyphenate;
    QElapsedTimer t;
    t.start();
    makeBenchStory(fd.story(), paragraphs, o.justify);
    out << "story\t" << paragraphs << " paragraphs\t" << fd.story()->characterCount() << " characters\t" << ms(t) << " ms\n";
    Paginator pg(&fd);
    LayoutCache cache;
    bool met = true;
    auto target = [&](const char *what, double took, double limit) {
        const bool ok = took <= limit;
        met &= ok;
        out << "target\t" << what << "\t" << took << " ms of " << limit << " ms\t" << (ok ? "met" : "MISSED") << "\n";
    };
    t.restart();
    const QImage first = pageImage(pg, &cache, 0, 800);
    const double firstMs = ms(t);
    out << "first page ready\t" << firstMs << " ms\t(" << (first.isNull() ? "none" : "drawn") << ")\n";
    t.restart();
    const int pages = pg.pageCount();
    const double allMs = ms(t);
    out << "all pages\t" << pages << " pages\t" << allMs / 1000 << " s\t" << allMs / std::max(1, pages) << " ms a page\t" << residentKb() / 1024 << " MB\n";
    if (paragraphs == 26443) {
        target("the first page", firstMs, kTargetFirstPageMs);
        target("all of the pages", allMs, kTargetAllPagesMs);
    }
    // A typed character: at the start, the middle, and the end. What a screen needs is the pages by the edit; the rest goes on after.
    const int end = fd.story()->characterCount() - 1;
    const std::pair<const char *, int> places[] = {{"start", 0}, {"middle", end / 2}, {"end", end - 1}};
    for (const auto &where : places) {
        double nearest = 0, worst = 0, settle = 0;
        qint64 laid = 0;
        const int tries = 10;
        for (int i = 0; i < tries; ++i) {
            QTextCursor c(fd.story());
            const int at = std::min(where.second + i * 37, fd.story()->characterCount() - 1);
            c.setPosition(at);
            const qint64 before = pg.pagesLaidOut();
            t.restart();
            c.insertText(QStringLiteral("x"));
            pg.paginate(pg.pageOfPosition(at) + 1);
            const double near = ms(t);
            nearest += near;
            worst = std::max(worst, near);
            t.restart();
            pg.pageCount();
            settle += ms(t);
            laid += pg.pagesLaidOut() - before;
        }
        out << "typed a character at the " << where.first << "\t" << nearest / tries << " ms on average for the pages by it (worst " << worst << " ms)\t" << settle / tries
            << " ms more to settle the rest\t" << double(laid) / tries << " pages laid out in all\n";
        if (paragraphs == 26443) target(qPrintable(QStringLiteral("a typed character at the %1 (worst)").arg(QLatin1String(where.first))), worst, kTargetTypedMs);
    }
    // The page after an edit, drawn.
    t.restart();
    const QImage again = pageImage(pg, &cache, pg.pageOfPosition(end / 2), 800);
    out << "a page made and drawn\t" << ms(t) << " ms\n";
    if (!met) out << "a target was missed (a busy machine slows these: run it again when it isn't)\n";
    return again.isNull() || !met ? 1 : 0;
}

} // namespace

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(resources);   // the font measurements, in the shared libraries
#ifdef JW_DOCX
    Q_INIT_RESOURCE(write);       // JeffWrite's own (its styles for a document that lacks them)
#endif
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
    if (!qEnvironmentVariableIsSet("QT_IM_MODULE")) qputenv("QT_IM_MODULE", "none");
    QGuiApplication app(argc, argv);
    initCore();
    const QStringList args = app.arguments();
    QTextStream out(stdout);
    if (args.size() < 2 || (args[1] != "bench" && args.size() < 3) || ((args[1] == "render" || args[1] == "pdf") && args.size() < 4)) {
        out << "usage: writetool render <file.txt|.docx|.doc> <outdir> [width=800] | pdf <file.txt|.docx|.doc> <out.pdf> | info <file.txt|.docx|.doc> | docpkg <file.doc> <out.docx> | bench [paragraphs=26443]   (options for a .txt: columns=N a4 legal landscape margin=INCHES hyphenate justify)\n";
        return 2;
    }
    const QString cmd = args[1];
#ifdef JW_DOCX
    if (cmd == "docpkg" && args.size() >= 4) {
        QFile f(args[2]);
        if (!f.open(QIODevice::ReadOnly)) { out << "cannot read: " << f.errorString() << "\n"; return 1; }
        QString why;
        jp::office::wml::ReadNotes rn;
        const QByteArray zip = jp::office::doc::toDocxPackage(f.readAll(), &why, &rn);
        for (const QString &n : rn.notes) out << "note\t" << n << "\n";
        QFile w(args[3]);
        if (zip.isEmpty() || !w.open(QIODevice::WriteOnly) || w.write(zip) != zip.size()) { out << "cannot make the package: " << why << "\n"; return 1; }
        out << "wrote\t" << zip.size() << " bytes\n";
        return 0;
    }
#endif
    Options o;
    int width = 800;
    int paragraphs = 26443;
    for (int i = cmd == "bench" ? 2 : cmd == "info" ? 3 : 4; i < args.size(); ++i) {
        if (parseOption(args[i], &o)) continue;
        bool ok = false;
        const int v = args[i].startsWith(QLatin1String("width=")) ? args[i].mid(6).toInt(&ok) : args[i].toInt(&ok);
        if (!ok || v < 1) { out << "unknown option: " << args[i] << "\n"; return 2; }
        (cmd == "bench" ? paragraphs : width) = v;
    }
    if (cmd == "bench") return bench(paragraphs, o, out);

    QElapsedTimer timer;
    timer.start();
    // A .docx is read and laid out as JeffWrite does (its own setup, sections, and styles); a text file with the options given.
    FlowDocument fd;
    std::unique_ptr<Paginator> ownPager;
    Paginator *pager = nullptr;
    QString error;
#ifdef JW_DOCX
    std::unique_ptr<write::WriteDoc> wdoc;
    const QString suffix = QFileInfo(args[2]).suffix().toLower();
    const bool old = suffix == QLatin1String("doc") || suffix == QLatin1String("dot");
    if (old || suffix == QLatin1String("docx") || suffix == QLatin1String("docm") || suffix == QLatin1String("dotx") || suffix == QLatin1String("dotm")) {
        QFile f(args[2]);
        if (!f.open(QIODevice::ReadOnly)) {
            out << "cannot read: " << f.errorString() << "\n";
            return 1;
        }
        QStringList notes;
        wdoc = old ? write::WriteDoc::fromDoc(f.readAll(), &error, &notes) : write::WriteDoc::fromDocx(f.readAll(), &error, &notes);
        if (!wdoc) {
            out << "cannot read: " << error << "\n";
            return 1;
        }
        for (const QString &n : notes) out << "note\t" << n << "\n";
        out << "read\t" << timer.elapsed() << " ms\n";
        out.flush();   // (a file that takes long to lay out has told how long it took to read)
        pager = &wdoc->pager();
    }
#endif
    if (!pager) {
        if (!loadText(fd, args[2], o, &error)) {
            out << "cannot read: " << error << "\n";
            return 1;
        }
        ownPager = std::make_unique<Paginator>(&fd);
        pager = ownPager.get();
    }
    Paginator &pg = *pager;
    const int pages = pg.pageCount();
    out << "paginated\t" << pages << " pages\t" << timer.elapsed() << " ms\n";
    LayoutCache cache;

    if (cmd == "info") {
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.page(k);
            const auto lines = pg.lines(k);
            out << "page\t" << k + 1 << "\tsection " << page.start.section + 1 << "\t" << page.size.width() << " x " << page.size.height() << "\tfrom " << page.start.pos << (page.start.skip ? "+" + QString::number(page.start.skip) + " lines" : QString())
                << (page.start.hard ? "\thard" : "") << "\t" << lines.size() << " lines\n";
        }
        return 0;
    }
    if (cmd == "render") {
        QDir dir(args[3]);
        if (!dir.mkpath(QStringLiteral("."))) { out << "cannot make " << args[3] << "\n"; return 1; }
        for (int k = 0; k < pages; ++k) {
            const QImage img = pageImage(pg, &cache, k, width, false, pages);
            if (img.isNull() || !img.save(dir.filePath(QStringLiteral("page-%1.png").arg(k + 1, 3, 10, QLatin1Char('0'))))) { out << "cannot save page " << k + 1 << "\n"; return 1; }
        }
        out << "wrote\t" << pages << " pages\t" << timer.elapsed() << " ms\n";
        return 0;
    }
    if (cmd == "pdf") {
        QSaveFile file(args[3]);
        if (!file.open(QIODevice::WriteOnly)) { out << "cannot write: " << file.errorString() << "\n"; return 1; }
        {
            QPdfWriter pdf(&file);
            pdf.setCreator(QStringLiteral("writetool"));
            pdf.setResolution(1200);
            pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
            QPainter p;
            for (int k = 0; k < pages; ++k) {
                const auto page = pg.materialize(k);
                if (!page) { out << "cannot make page " << k + 1 << "\n"; return 1; }
                const QSizeF size = page->doc->pageSize();
                pdf.setPageSize(QPageSize(size, QPageSize::Point, QString(), QPageSize::ExactMatch));
                if (k == 0) {
                    if (!p.begin(&pdf)) { out << "cannot start the PDF\n"; return 1; }
                } else {
                    pdf.newPage();
                }
                p.save();
                p.scale(1200.0 / 72, 1200.0 / 72);
                PaintContext ctx;
                ctx.doc = page->doc.get();
                ctx.cache = &cache;
                ctx.opt.output = true;
                ctx.opt.textOutput = TextOutput::Pdf;
                ctx.pageNumber = page->number;
                ctx.pageCount = pages;
                Renderer::paintPage(&p, ctx, 0);
                TablePaint tp;
                tp.output = TextOutput::Pdf;
                paintTables(&p, *page, tp);
                p.restore();
            }
            p.end();
        }
        if (!file.commit()) { out << "cannot write: " << file.errorString() << "\n"; return 1; }
        out << "wrote\t" << args[3] << "\t" << pages << " pages\t" << timer.elapsed() << " ms\n";
        return 0;
    }
    out << "unknown command: " << cmd << "\n";
    return 2;
}
