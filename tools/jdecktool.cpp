// Command-line helper for testing JeffDeck's reading of presentation files.
//   jdecktool render <file.pptx|.ppt> <outdir> [width=1280]   each slide to slide-001.png ...
//   jdecktool info <file.pptx|.ppt>                           the masters, layouts, and slides found
//   jdecktool save <file> <out.pptx|.odp|.otp>                read it and save it again, as the ending says, listing what was left out
//   jdecktool pdf <file> <out.pdf> [slides|notes|handout6]   print it to a PDF (also handout1|2|3|4|9, outline; "hidden", "frame", "gray", "down" add options; "samplenotes" gives slides without notes
//   jdecktool animate-demo <out.pptx> <out.tsv>               a presentation with every effect JeffDeck offers (each way of each kind, started on a click, with, and after the one before, and by paragraph), and a table of what is where
//   jdecktool animations <file>                               each slide's effects as the animation pane lists them (the table animate-demo writes; "(custom)" for what JeffDeck keeps as it was)
//   jdecktool animate-edit <file> <out.pptx>                  adds a Fade on the first shape of each slide that has animations, as the animation pane does, and saves it (to see that what the file had survives an edit)
//   jdecktool steps <file> <slide> <outdir> [width=1280]       a slide's click steps listed, and the slide as the show has it at the start (step-00-start.png), halfway through each step (step-01-mid.png), and after it (step-01-end.png)
//   jdecktool morph <file> <from> <to> <outdir> [width=1280] [objects|words|characters]   the Morph transition from slide <from> to slide <to> at 0, 25, 50, 75, and 100% (morph-000.png ... morph-100.png), and its pairs listed
//   jdecktool morph-demo <out.pptx>                           a presentation of slides that morph into one another, by object, by word, and by character

#include "app/deckinfo.h"
#include "app/morph.h"
#include "app/slideshow.h"
#include "core/animation.h"
#include "core/appinfo.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "io/deckoutput.h"
#include "io/pptxexport.h"
#include "io/presentationfile.h"
#include "render/renderer.h"
#include "text/textprops.h"

#include <QBuffer>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QHash>
#include <QJsonArray>
#include <QPainter>
#include <QTextCursor>
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

// One behavior of an effect, in a line.
QString describe(const deck::Behavior &b)
{
    using Kind = deck::Behavior::Kind;
    QString what;
    switch (b.kind) {
    case Kind::Set: what = QStringLiteral("set %1=%2").arg(b.attribute, b.to); break;
    case Kind::Effect: what = QStringLiteral("%1 %2").arg(b.filter, b.entering ? QStringLiteral("in") : QStringLiteral("out")); break;
    case Kind::Animate: {
        QStringList keys;
        for (const deck::Keyframe &k : b.keys) keys << QStringLiteral("%1:%2").arg(k.at).arg(k.value);
        what = QStringLiteral("animate %1 %2").arg(b.attribute, keys.isEmpty() ? QStringLiteral("%1>%2+%3").arg(b.from, b.to, b.by) : keys.join(QLatin1Char(' ')));
        break;
    }
    case Kind::Motion: what = QStringLiteral("path %1 points").arg(b.path.elementCount()); break;
    case Kind::Scale: what = QStringLiteral("scale %1,%2 to %3,%4").arg(b.scaleFrom.x()).arg(b.scaleFrom.y()).arg(b.scaleTo.x()).arg(b.scaleTo.y()); break;
    case Kind::Rotate: what = QStringLiteral("rotate %1 to %2").arg(b.rotateFrom).arg(b.rotateTo); break;
    case Kind::Color: what = QStringLiteral("color %1 (not played)").arg(b.attribute); break;
    case Kind::Command: what = QStringLiteral("command %1 (not played)").arg(b.command); break;
    }
    QString target = QString::number(b.target.shape);
    if (b.target.paragraphs()) target += QStringLiteral(" p%1-%2").arg(b.target.firstParagraph).arg(b.target.lastParagraph);
    return QStringLiteral("shape %1: %2, %3 for %4 ms%5").arg(target, what).arg(b.timing.begin).arg(b.timing.duration).arg(b.timing.hold ? QStringLiteral(", held") : QString());
}

// The placeholders of a page, each with its place: type[:index]@x,y,wxh in points.
QString placeholders(const PageBase &page)
{
    QStringList out;
    for (const auto &it : page.items)
        if (it->role.startsWith(QLatin1String("ph:")))
            out << QStringLiteral("%1@%2,%3,%4x%5").arg(it->role.mid(3)).arg(it->rect.x(), 0, 'f', 0).arg(it->rect.y(), 0, 'f', 0).arg(it->rect.width(), 0, 'f', 0).arg(it->rect.height(), 0, 'f', 0);
    return out.join(QLatin1Char(' '));
}

// A presentation with a slide for each kind of effect: shapes A, B, and C start on a click, with the one before, and after it; a
// list animated by paragraph; D with a delay and its own length; and one shape for each other way. The table says, for each
// effect in order, the slide, its kind, its way, the shape's name, how it starts, its length, its delay, and its paragraph.
int animateDemo(const QString &path, const QString &tablePath)
{
    std::unique_ptr<Document> doc = deck::newPresentation();
    const MasterPage &layout = *deck::layoutOfType(*doc, QStringLiteral("obj"));
    QString table;
    QTextStream t(&table);
    t << "slide\tindex\tkind\tvariant\tshape\tstart\tduration\tdelay\tparagraph\tamount\tafter\n";
    int slideNumber = 0;
    for (const deck::EffectKind &kind : deck::effectKinds()) {
        const int at = deck::addSlide(*doc, layout);
        Page &page = *doc->pages[at];
        ++slideNumber;
        doc->storyDoc(static_cast<TextItem *>(page.items[0].get())->storyId)->setPlainText(kind.id);
        doc->storyDoc(static_cast<TextItem *>(page.items[1].get())->storyId)->setPlainText(QStringLiteral("one\ntwo\nthree"));
        page.items[0]->rect = QRectF(40, 10, 880, 50);
        page.items[1]->rect = QRectF(40, 380, 300, 140);
        int shapes = 0;
        auto shape = [&](const QString &name) {
            auto s = std::make_shared<ShapeItem>();
            s->name = name;
            s->rect = QRectF(40 + 76 * (shapes % 12), 80 + 54 * (shapes / 12), 66, 44);
            s->fill = Fill::solid(ColorRef::rgb(QColor(0x3A, 0x6E, 0xA5)));
            page.items.push_back(s);
            ++shapes;
            return s;
        };
        deck::AnimSequence seq;
        auto add = [&](const ItemPtr &item, const QString &variant, deck::EffectStart start, int duration = -1, int delay = 0, double amount = -1) {
            deck::Anim a = deck::makeAnim(kind, deck::shapeNumber(page, item->id));
            a.variant = variant;
            a.start = start;
            if (duration >= 0 && kind.duration > 0) a.duration = duration;
            a.delay = delay;
            if (amount >= 0) a.amount = amount;
            return seq.add(a);
        };
        const QString first = kind.firstVariant().key;
        add(shape(QStringLiteral("A")), first, deck::EffectStart::OnClick);
        // What happens to the shape when the effect is over (an exit has hidden it already).
        if (kind.cls != deck::EffectClass::Exit) {
            const int hideAfter = add(shape(QStringLiteral("H1")), first, deck::EffectStart::OnClick);
            seq.effects[hideAfter].after = deck::AfterEffect::Hide;
            const int hideNext = add(shape(QStringLiteral("H2")), first, deck::EffectStart::OnClick);
            seq.effects[hideNext].after = deck::AfterEffect::HideOnNextClick;
        }
        add(shape(QStringLiteral("B")), first, deck::EffectStart::WithPrevious);
        add(shape(QStringLiteral("C")), first, deck::EffectStart::AfterPrevious);
        add(shape(QStringLiteral("D")), first, deck::EffectStart::OnClick, 1500, 250);
        if (kind.duration > 0) {
            // Smooth start and end the other way round from the kind's, and repeats with a rewind.
            const int smooth = add(shape(QStringLiteral("S")), first, deck::EffectStart::OnClick);
            seq.effects[smooth].smoothStart = seq.effects[smooth].smoothEnd = !kind.smooth;
            const int again = add(shape(QStringLiteral("R")), first, deck::EffectStart::WithPrevious);
            seq.effects[again].repeat = 3;
            seq.effects[again].rewind = true;
        }
        // The list, by paragraph.
        const int body = add(page.items[1], first, deck::EffectStart::OnClick);
        deck::setByParagraph(seq, body, true, *doc, page);
        for (int i = 1; i < kind.variants.size(); ++i) add(shape(QStringLiteral("V%1").arg(i)), kind.variants[i].key, deck::EffectStart::OnClick);
        if (kind.amount != deck::EffectKind::Amount::None)
            for (double amount : kind.amount == deck::EffectKind::Amount::Size ? QVector<double>{25, 50, 400} : QVector<double>{90, 180, 720})
                add(shape(QStringLiteral("N%1").arg(amount)), first, deck::EffectStart::OnClick, -1, 0, amount);
        deck::setAnimations(*doc, page, seq);
        for (int i = 0; i < seq.effects.size(); ++i) {
            const deck::Anim &a = seq.effects[i];
            QString name;
            walkItems(page.items, [&](const ItemPtr &it) {
                if (it->sourceId == a.shape && name.isEmpty()) name = it->name;
            });
            t << slideNumber + 1 << '\t' << i + 1 << '\t' << a.kind << '\t' << a.variant << '\t' << name << '\t' << int(a.start) << '\t' << a.duration << '\t' << a.delay << '\t' << a.firstParagraph << '\t' << a.amount << '\t' << int(a.after) << '\n';
        }
    }
    // The first slide stays the title slide the presentation began with.
    QString error;
    QStringList skipped;
    if (!deck::saveAnyPresentationFile(*doc, path, &error, &skipped)) {
        QTextStream(stdout) << "cannot save: " << error << "\n";
        return 1;
    }
    for (const QString &s : skipped) QTextStream(stdout) << "left out\t" << s << "\n";
    QFile f(tablePath);
    if (!f.open(QIODevice::WriteOnly)) return 1;
    f.write(table.toUtf8());
    QTextStream(stdout) << "saved\t" << path << "\t" << slideNumber << " slides\n";
    return 0;
}

// A text box with its text at a size and in a color.
ItemPtr demoText(Document &doc, const QString &name, const QString &text, const QRectF &rect, double points, const QColor &color, bool bold = false)
{
    auto t = std::make_shared<TextItem>();
    t->name = name;
    t->rect = rect;
    t->storyId = doc.createStory(text);
    t->hyphenate = false;
    QTextDocument *story = doc.storyDoc(t->storyId);
    QTextCharFormat cf;
    cf.setFontPointSize(points);
    cf.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    cf.setForeground(color);
    cf.setProperty(tp::ColorRefP, ColorRef::rgb(color).toString());
    QTextCursor c(story);
    c.select(QTextCursor::Document);
    c.mergeCharFormat(cf);
    QTextCursor start(story);
    start.mergeBlockCharFormat(cf);
    return t;
}

ItemPtr demoShape(const QString &name, const QString &preset, const QRectF &rect, const QColor &color, double turn = 0)
{
    auto s = std::make_shared<ShapeItem>();
    s->name = name;
    s->shape = preset;
    s->rect = rect;
    s->rotation = turn;
    s->fill = Fill::solid(ColorRef::rgb(color));
    s->stroke = Stroke::none();
    return s;
}

// A presentation whose slides morph into one another: the first has a card, a dot, a square that goes, a photo, and a line of
// text; the second moves, grows, turns, and recolors the card and the dot, crops the photo another way (renamed, so that it is
// paired by its picture), brings a star, and moves the text by object; the third moves the words of a sentence (by word); the
// fourth moves the letters of a sentence and of a word (by character), and makes a pair of a circle and a star that share a name starting with !!.
int morphDemo(const QString &path)
{
    std::unique_ptr<Document> doc = deck::newPresentation();
    const MasterPage &layout = *deck::layoutOfType(*doc, QStringLiteral("titleOnly"));
    doc->pages.erase(doc->pages.begin());
    for (int i = 0; i < 4; ++i) deck::addSlide(*doc, layout);
    // A picture of bands of color, so that a crop and a move can be seen.
    QImage img(240, 160, QImage::Format_RGB32);
    {
        QPainter p(&img);
        for (int x = 0; x < img.width(); x += 20) p.fillRect(x, 0, 20, img.height(), QColor::fromHsv((x * 360 / img.width()) % 360, 140, 235));
        p.setPen(QPen(Qt::white, 8));
        p.drawEllipse(QRectF(70, 30, 100, 100));
    }
    QByteArray bytes;
    {
        QBuffer b(&bytes);
        b.open(QIODevice::WriteOnly);
        img.save(&b, "PNG");
    }
    const QString imageId = doc->addImage(bytes, QStringLiteral("png"));
    auto photo = [&](const QString &name, const QRectF &rect, const QRectF &crop) {
        auto p = std::make_shared<PictureItem>();
        p->name = name;
        p->imageId = imageId;
        p->rect = rect;
        p->imgRect = crop;
        return p;
    };
    const QColor ink(0x20, 0x2A, 0x3A), blue(0x2E, 0x75, 0xB6), orange(0xE0, 0x7B, 0x1B), green(0x2E, 0x9E, 0x5B), plum(0x8A, 0x3F, 0xA8), gray(0xA0, 0xA8, 0xB0);
    // The title's text, in the look the layout gave it.
    auto title = [&](int slide, const QString &text) {
        QTextCursor c(doc->storyDoc(static_cast<TextItem *>(doc->pages[slide]->items[0].get())->storyId));
        c.select(QTextCursor::Document);
        c.insertText(text);
    };
    auto put = [&](int slide, const ItemPtr &item) { doc->pages[slide]->items.push_back(item); };
    auto morph = [&](int slide, const char *by, double seconds) {
        deck::Transition t;
        t.type = QStringLiteral("morph");
        t.by = QString::fromLatin1(by);
        t.duration = seconds;
        deck::setTransition(*doc->pages[slide], t);
    };

    title(0, QStringLiteral("Morph: objects"));
    put(0, demoShape(QStringLiteral("Card"), QStringLiteral("roundRect"), QRectF(70, 170, 230, 130), blue));
    put(0, demoShape(QStringLiteral("Dot"), QStringLiteral("ellipse"), QRectF(360, 400, 90, 90), orange));
    put(0, demoShape(QStringLiteral("Square that goes"), QStringLiteral("rect"), QRectF(520, 400, 100, 90), gray));
    put(0, photo(QStringLiteral("Photo"), QRectF(660, 160, 240, 160), QRectF(0, 0, 240, 160)));
    put(0, demoText(*doc, QStringLiteral("Note"), QStringLiteral("What both slides have moves."), QRectF(70, 330, 520, 50), 24, ink));
    put(0, demoText(*doc, QStringLiteral("Sentence"), QStringLiteral("The lazy dog jumps over the quick brown fox."), QRectF(70, 430, 400, 90), 20, ink));

    title(1, QStringLiteral("Objects move, turn, and change color"));
    morph(1, "objects", 2);
    put(1, demoShape(QStringLiteral("Card"), QStringLiteral("roundRect"), QRectF(560, 330, 320, 170), green, 12));
    put(1, demoShape(QStringLiteral("Dot"), QStringLiteral("ellipse"), QRectF(90, 340, 160, 160), plum));
    put(1, demoShape(QStringLiteral("Spark"), QStringLiteral("star5"), QRectF(430, 170, 110, 110), orange));
    put(1, photo(QStringLiteral("Picture 7"), QRectF(70, 160, 200, 140), QRectF(-80, -50, 360, 240)));
    put(1, demoText(*doc, QStringLiteral("Note"), QStringLiteral("The ones not on both fade."), QRectF(300, 330, 520, 50), 24, ink));
    put(1, demoText(*doc, QStringLiteral("Sentence"), QStringLiteral("The lazy dog jumps over the quick brown fox."), QRectF(70, 430, 400, 90), 20, ink));

    title(2, QStringLiteral("Words move"));
    morph(2, "words", 2);
    put(2, demoShape(QStringLiteral("Card"), QStringLiteral("roundRect"), QRectF(70, 380, 200, 110), blue));
    put(2, demoShape(QStringLiteral("Dot"), QStringLiteral("ellipse"), QRectF(830, 330, 70, 70), orange));
    put(2, demoShape(QStringLiteral("!!pin"), QStringLiteral("ellipse"), QRectF(830, 430, 60, 60), plum));
    put(2, demoText(*doc, QStringLiteral("Sentence"), QStringLiteral("The quick brown fox jumps over the lazy dog."), QRectF(70, 160, 760, 150), 44, ink, true));
    put(2, demoText(*doc, QStringLiteral("Letters"), QStringLiteral("STONE"), QRectF(330, 390, 330, 100), 60, green, true));

    title(3, QStringLiteral("Characters move"));
    morph(3, "characters", 2);
    put(3, demoShape(QStringLiteral("Card"), QStringLiteral("roundRect"), QRectF(70, 380, 200, 110), blue));
    put(3, demoShape(QStringLiteral("!!pin"), QStringLiteral("star5"), QRectF(790, 300, 110, 110), orange));
    put(3, demoText(*doc, QStringLiteral("Sentence"), QStringLiteral("The quick brown fox jumps over the lazy dog."), QRectF(70, 160, 760, 60), 24, ink));
    put(3, demoText(*doc, QStringLiteral("Letters"), QStringLiteral("NOTES"), QRectF(420, 260, 330, 100), 80, plum, true));

    QString error;
    QStringList skipped;
    if (!deck::savePresentationFile(*doc, path, &error, &skipped)) {
        QTextStream(stdout) << "cannot save: " << error << "\n";
        return 1;
    }
    for (const QString &s : skipped) QTextStream(stdout) << "left out\t" << s << "\n";
    QTextStream(stdout) << "saved\t" << path << "\t" << doc->pages.size() << " slides\n";
    return 0;
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
    // JP_NOW (an ISO 8601 date and time, say 2026-10-15T19:31:36-04:00) is the moment automatic dates and times show, so that a render can be made
    // again as of the day the original was drawn. (A tool's: the app doesn't read it.)
    if (const QDateTime moment = QDateTime::fromString(qEnvironmentVariable("JP_NOW"), Qt::ISODate); moment.isValid()) setFieldClock(moment);
    const QStringList args = app.arguments();
    QTextStream out(stdout);
    if (args.size() >= 4 && args[1] == "animate-demo") return animateDemo(args[2], args[3]);
    if (args.size() >= 3 && args[1] == "morph-demo") return morphDemo(args[2]);
    if (args.size() < 3 || ((args[1] == "render" || args[1] == "save" || args[1] == "pdf") && args.size() < 4) || (args[1] == "steps" && args.size() < 5) || (args[1] == "lines" && args.size() < 4) ||
        (args[1] != "render" && args[1] != "info" && args[1] != "save" && args[1] != "pdf" && args[1] != "steps" && args[1] != "lines" && args[1] != "animations" && args[1] != "animate-edit" && args[1] != "morph") ||
        (args[1] == "animate-edit" && args.size() < 4) || (args[1] == "morph" && args.size() < 6)) {
        out << "usage: jdecktool render <file.pptx|.ppt> <outdir> [width=1280] | info <file.pptx|.ppt> | save <file> <out.pptx> [embed-fonts] | pdf <file> <out.pdf> [slides|notes|handout6 ...] | steps <file> <slide> <outdir> [width=1280] | lines <file> <slide> | animations <file> | animate-edit <file> <out.pptx> | animate-demo <out.pptx> <out.tsv> | morph <file> <from> <to> <outdir> [width=1280] [objects|words|characters] | morph-demo <out.pptx>\n";
        return 2;
    }
    const QString cmd = args[1], in = args[2];
    QElapsedTimer timer;
    timer.start();
    QString notes;
    std::unique_ptr<Document> doc = deck::importAnyPresentationFile(in, &notes);
    if (!doc) {
        out << "cannot open: " << notes << "\n";
        return 1;
    }
    out << "opened\t" << doc->pages.size() << " slides\t" << deck::layouts(*doc).size() << " layouts\t" << timer.elapsed() << " ms\n";
    if (!notes.isEmpty()) out << "notes:\n" << notes << "\n";

    if (cmd == "save") {
        QString error;
        QStringList skipped;
        // With "embed-fonts" after the name, the program's own fonts the text uses go in the file (as the window's Save does by default).
        const bool ok = deck::saveAnyPresentationFile(*doc, args[3], &error, &skipped, args.value(4) == QLatin1String("embed-fonts"));
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

    // Each text box of a slide as laid out: its frame, then each line's place, baseline, size, and text.
    if (cmd == "lines") {
        const int slide = args[3].toInt() - 1;
        if (slide < 0 || slide >= doc->pages.size()) {
            out << "no slide " << args[3] << "\n";
            return 1;
        }
        LayoutCache cache;
        walkItems(doc->pages[slide]->items, [&](const ItemPtr &it) {
            if (it->type() != ItemType::Text) return;
            const auto &t = static_cast<const TextItem &>(*it);
            const auto fl = cache.textFrame(*doc, t, slide + 1, RenderOptions());
            out << "box\t" << t.name << "\t" << t.rect.x() << "," << t.rect.y() << " " << t.rect.width() << "x" << t.rect.height() << "\tfit " << fl.fitScale
                << (fl.layout && fl.layout->overflow() ? "\toverflow" : "") << "\n";
            if (!fl.layout) return;
            for (const auto &l : fl.layout->lineInfo(0))
                out << "  y " << l.rect.top() << "\th " << l.rect.height() << "\tbase " << l.baseline << "\tbelow " << l.below << "\t" << l.pointSize << " pt " << l.family
                    << "\t" << l.text.left(40) << "\n";
        });
        return 0;
    }

    if (cmd == "animations") {
        out << "slide\tindex\tkind\tvariant\tshape\tstart\tduration\tdelay\tparagraph\tamount\tafter\n";
        for (int i = 0; i < doc->pages.size(); ++i) {
            const Page &page = *doc->pages[i];
            const deck::AnimSequence seq = deck::readAnimations(page, doc.get());
            for (int k = 0; k < seq.effects.size(); ++k) {
                const deck::Anim &a = seq.effects[k];
                QString name;
                walkItems(page.items, [&](const ItemPtr &it) {
                    if (it->sourceId == a.shape && name.isEmpty()) name = it->name;
                });
                out << i + 1 << '\t' << k + 1 << '\t' << (a.custom() ? QStringLiteral("(custom)") : a.kind) << '\t' << a.variant << '\t' << name << '\t' << int(a.start) << '\t' << a.duration << '\t'
                    << a.delay << '\t' << a.firstParagraph << '\t' << a.amount << '\t' << int(a.after) << '\n';
            }
        }
        return 0;
    }

    if (cmd == "animate-edit") {
        for (int i = 0; i < doc->pages.size(); ++i) {
            Page &page = *doc->pages[i];
            if (page.props.value(QStringLiteral("timingXml")).toString().isEmpty()) continue;
            deck::AnimSequence seq = deck::readAnimations(page, doc.get());
            if (!seq.editable) {
                out << "slide " << i + 1 << "\tnot editable\n";
                continue;
            }
            const qsizetype before = seq.effects.size();
            QString target;
            for (const ItemPtr &it : page.items)
                if (target.isEmpty() && (it->type() == ItemType::Shape || it->type() == ItemType::Text)) target = it->id;
            if (target.isEmpty()) continue;
            seq.add(deck::makeAnim(QStringLiteral("entr.fade"), deck::shapeNumber(page, target)));
            deck::setAnimations(*doc, page, seq);
            out << "slide " << i + 1 << "\t" << before << " effects, " << seq.kept.size() << " other nodes kept, now " << deck::readAnimations(page, doc.get()).effects.size() << " effects\n";
        }
        QString error;
        QStringList skipped;
        const bool ok = deck::saveAnyPresentationFile(*doc, args[3], &error, &skipped);
        for (const QString &s : skipped) out << "left out\t" << s << "\n";
        out << (ok ? "saved\t" + args[3] : "cannot save: " + error) << "\n";
        return ok ? 0 : 1;
    }

    if (cmd == "steps") {
        static const char *classes[] = {"entrance", "exit", "emphasis", "motion path", "media", "verb", "other"};
        static const char *starts[] = {"on click", "with previous", "after previous"};
        const int slide = args[3].toInt() - 1;
        if (slide < 0 || slide >= doc->pages.size()) {
            out << "no such slide: " << args[3] << " (the file has " << doc->pages.size() << ")\n";
            return 2;
        }
        const Page &page = *doc->pages[slide];
        const deck::Timeline tl = deck::playableTimeline(*doc, page);
        out << "steps\tslide " << slide + 1 << "\t" << tl.steps.size() << " click steps\t" << deck::timelineOf(page).steps.size() << " read\t" << tl.triggers.size() << " triggers\n";
        if (tl.isEmpty()) {
            out << (page.props.contains(QStringLiteral("timingXml")) ? "the slide's timing has no steps that show anything\n" : "the slide has no animations\n");
            return 0;
        }
        const double width = args.size() > 5 ? std::max(16.0, QString(args[5]).remove(QStringLiteral("width=")).toDouble()) : 1280;
        const QSize size(qRound(width), qRound(width * doc->pageSize().height() / doc->pageSize().width()));
        const deck::Geometry geometry = deck::geometryOf(*doc, page);
        LayoutCache cache;
        QDir().mkpath(args[4]);
        auto save = [&](const QString &name, int done, double ms) {
            const deck::States states = deck::statesAt(tl, geometry, done, ms);
            renderSlideImage(doc.get(), &cache, slide, size, 1.0, Qt::black, &states).save(QDir(args[4]).filePath(name));
            out << name << "\n";
        };
        save(QStringLiteral("step-00-start.png"), 0, -1);
        for (int k = 0; k < tl.steps.size(); ++k) {
            const deck::Step &step = tl.steps[k];
            out << "step " << k + 1 << (step.automatic ? "\tstarts by itself" : "\ton click") << "\t" << step.duration() << " ms\n";
            for (const deck::Effect &e : step.effects) {
                out << "  " << classes[int(e.cls)] << " preset " << e.presetId << "/" << e.presetSubtype << ", " << starts[int(e.start)] << ", at " << e.begin << " to " << e.end() << " ms\n";
                for (const deck::Behavior &b : e.behaviors) out << "    " << describe(b) << "\n";
            }
            save(QStringLiteral("step-%1-mid.png").arg(k + 1, 2, 10, QLatin1Char('0')), k, step.duration() / 2);
            save(QStringLiteral("step-%1-end.png").arg(k + 1, 2, 10, QLatin1Char('0')), k + 1, -1);
        }
        return 0;
    }

    if (cmd == "morph") {
        const int from = args[3].toInt() - 1, to = args[4].toInt() - 1;
        if (from < 0 || to < 0 || from >= doc->pages.size() || to >= doc->pages.size()) {
            out << "no such slide: " << args[3] << " or " << args[4] << " (the file has " << doc->pages.size() << ")\n";
            return 2;
        }
        const double width = args.size() > 6 ? std::max(16.0, QString(args[6]).remove(QStringLiteral("width=")).toDouble()) : 1280;
        const QSize size(qRound(width), qRound(width * doc->pageSize().height() / doc->pageSize().width()));
        // What the slide's own transition says to move, unless asked.
        const deck::Transition own = deck::transitionOf(*doc->pages[to]);
        const QString unit = args.size() > 7 ? args[7] : own.isMorph() ? own.by : QStringLiteral("objects");
        // The slides as the show has them: the first with its steps played, the second at its start.
        auto statesOf = [&](int slide, bool played) {
            const deck::Timeline tl = deck::playableTimeline(*doc, *doc->pages[slide]);
            return tl.isEmpty() ? deck::States() : deck::statesAt(tl, deck::geometryOf(*doc, *doc->pages[slide]), played ? int(tl.steps.size()) : 0);
        };
        const deck::States fromStates = statesOf(from, true), toStates = statesOf(to, false);
        timer.restart();
        MorphFrames morph(doc.get(), from, to, unit, size, 1.0, Qt::black, &fromStates, &toStates);
        out << "morph\tslide " << from + 1 << " to " << to + 1 << "\tby " << unit << "\t" << (morph.isUsable() ? "" : "too many objects: a fade\t") << timer.elapsed() << " ms to plan\n";
        for (const QString &line : morph.describe()) out << line << "\n";
        QDir().mkpath(args[5]);
        for (const int percent : {0, 25, 50, 75, 100}) {
            timer.restart();
            const QImage img = morph.frame(percent / 100.0);
            const qint64 ms = timer.elapsed();
            const QString name = QStringLiteral("morph-%1.png").arg(percent, 3, 10, QLatin1Char('0'));
            img.save(QDir(args[5]).filePath(name));
            out << name << "\t" << ms << " ms to draw\n";
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
