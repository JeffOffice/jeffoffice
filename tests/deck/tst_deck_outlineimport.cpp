// JeffDeck's tests of slides from an outline: the readers of text, Markdown, rich text, and .docx outlines, the slides they make, File > Open and Slides from Outline, and Save a Copy as an outline.
#include "deckhelpers.h"
#include "io/outlineimport.h"
#include "wml/wml.h"

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QPointer>
#include <QSemaphore>
#include <QThread>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QTextBlock>

namespace {

using namespace jp;

deck::Outline read(const QByteArray &bytes, const QString &name = QStringLiteral("a.txt"), const QString &folder = QString(), QString *error = nullptr)
{
    deck::Outline o;
    QString why;
    const bool ok = deck::readOutline(bytes, name, folder, &o, &why);
    if (error) *error = ok ? QString() : why;
    if (!ok) o = deck::Outline();
    return o;
}

QString titleOf(const deck::OutlineSlide &s)
{
    QString t;
    for (const auto &r : s.title) t += r.text;
    return t;
}
QString lineText(const deck::OutlineLine &l)
{
    QString t;
    for (const auto &r : l.runs) t += r.text;
    return t;
}
// "level:text" of each line of a slide
QStringList linesOf(const deck::OutlineSlide &s)
{
    QStringList out;
    for (const auto &l : s.body) out << QStringLiteral("%1:%2").arg(l.level).arg(lineText(l));
    return out;
}

bool writeFile(const QString &path, const QByteArray &bytes)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(bytes) == bytes.size();
}

QByteArray png(int w = 40, int h = 20)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(Qt::red);
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return bytes;
}

QString storyText(const Document &d, const TextItem &t)
{
    QStringList lines;
    for (QTextBlock b = d.storyDoc(t.storyId)->begin(); b.isValid(); b = b.next()) lines << b.text();
    return lines.join(QLatin1Char('\n'));
}

const TextItem *placeholderOf(const Document &d, int slide, const QString &type)
{
    for (const auto &it : d.pages[slide]->items)
        if (it->type() == ItemType::Text && deck::placeholderType(*it) == type) return static_cast<const TextItem *>(it.get());
    return nullptr;
}

// A one-page PDF built in memory (its cross-reference table with the true offsets).
QByteArray tinyPdf()
{
    QByteArray pdf = "%PDF-1.4\n";
    QList<int> at;
    const QByteArray objects[] = {"<< /Type /Catalog /Pages 2 0 R >>", "<< /Type /Pages /Kids [3 0 R] /Count 1 >>", "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 100] >>"};
    for (int i = 0; i < 3; ++i) {
        at << int(pdf.size());
        pdf += QByteArray::number(i + 1) + " 0 obj\n" + objects[i] + "\nendobj\n";
    }
    const int xref = int(pdf.size());
    pdf += "xref\n0 4\n0000000000 65535 f \n";
    for (int o : at) pdf += QByteArray::number(o).rightJustified(10, '0') + " 00000 n \n";
    pdf += "trailer\n<< /Size 4 /Root 1 0 R >>\nstartxref\n" + QByteArray::number(xref) + "\n%%EOF\n";
    return pdf;
}

struct Quiet {
    explicit Quiet(DeckWindow &w) { w.setNotifier([this](const QString &, const QString &text) { said << text; }); }
    QStringList said;
};

} // namespace

class DeckOutlineImportTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // Plain text: a line at the left edge is a title, the lines under it are its text, a level in for each deeper indent (tabs or spaces), and list markers go.
    void aPlainOutlineMakesSlidesWithLevels()
    {
        const deck::Outline o = read("Welcome\n\tAbout us\n\t\tFounded 2020\n\t- Mission\n\nNext steps\n  1. Call\n    * Email\n  - Visit\n");
        QCOMPARE(o.slides.size(), 2);
        QCOMPARE(titleOf(o.slides[0]), QStringLiteral("Welcome"));
        QCOMPARE(linesOf(o.slides[0]), (QStringList{"0:About us", "1:Founded 2020", "0:Mission"}));
        QCOMPARE(titleOf(o.slides[1]), QStringLiteral("Next steps"));
        QCOMPARE(linesOf(o.slides[1]), (QStringList{"0:Call", "1:Email", "0:Visit"}));
        // A plain text file does not read Markdown's marks.
        const deck::Outline p = read("Title\n  **not bold**\n");
        QCOMPARE(linesOf(p.slides[0]), (QStringList{"0:**not bold**"}));
        QVERIFY(!p.slides[0].body[0].runs[0].bold);
    }

    void everyLineEndingAndEncodingIsRead()
    {
        const QByteArray text = "One\n  a\nTwo\n  b\n";
        for (const char *ending : {"\n", "\r\n", "\r"}) {
            QByteArray t = text;
            t.replace("\n", ending);
            const deck::Outline o = read(t);
            QCOMPARE(o.slides.size(), 2);
            QCOMPARE(linesOf(o.slides[1]), (QStringList{"0:b"}));
        }
        const QString accents = QStringLiteral("Café 中文\n  naïve");
        // (a read that gives no slide fails the test here, with its words, and is not looked into)
        auto firstTitle = [&](const QByteArray &bytes) {
            QString why;
            const deck::Outline o = read(bytes, QStringLiteral("a.txt"), QString(), &why);
            return o.slides.isEmpty() ? QStringLiteral("(no slide: %1)").arg(why) : titleOf(o.slides[0]);
        };
        QCOMPARE(firstTitle("\xEF\xBB\xBF" + accents.toUtf8()), QStringLiteral("Café 中文"));
        QCOMPARE(firstTitle(accents.toUtf8()), QStringLiteral("Café 中文"));
        QByteArray le("\xFF\xFE"), be("\xFE\xFF");
        for (const QChar c : accents) {
            le += char(c.unicode() & 0xFF);
            le += char(c.unicode() >> 8);
            be += char(c.unicode() >> 8);
            be += char(c.unicode() & 0xFF);
        }
        QCOMPARE(firstTitle(le), QStringLiteral("Café 中文"));
        QCOMPARE(firstTitle(be), QStringLiteral("Café 中文"));
    }

    void aFileThatIsNotTextIsRefusedPlainly()
    {
        QString why;
        QVERIFY(read(QByteArray("abc\0def", 7), "a.txt", QString(), &why).slides.isEmpty());
        QVERIFY(!why.isEmpty());
        read(QByteArray("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1 binary"), "a.txt", QString(), &why);
        QVERIFY(!why.isEmpty());
        read(QByteArray("\xFF\xFF\xFF caf\xE9"), "a.txt", QString(), &why);
        QVERIFY(why.contains(QLatin1String("not text")));
        read("   \n\n", "a.txt", QString(), &why);
        QVERIFY(!why.isEmpty());
        read("not rich text", "a.rtf", QString(), &why);
        QVERIFY(!why.isEmpty());
        QVERIFY(read(QByteArray(int(deck::kMaxOutlineTextBytes) + 1, 'a'), "a.txt", QString(), &why).slides.isEmpty());
        QVERIFY(why.contains(QLatin1String("10 MB")));
    }

    // Markdown: the shallowest heading level is the slide's title; deeper headings are lines; --- breaks; lists are levels.
    void markdownHeadingsListsAndBreaksMakeSlides()
    {
        const deck::Outline o = read("# One\n- a\n  - b\n    - c\n1. d\n## Sub\ntext line\ncontinued\n\n---\n\n# Two\nplain\n---\nAfter break\n", "a.md");
        QCOMPARE(o.slides.size(), 3);
        QCOMPARE(titleOf(o.slides[0]), QStringLiteral("One"));
        QCOMPARE(linesOf(o.slides[0]), (QStringList{"0:a", "1:b", "2:c", "0:d", "0:Sub", "0:text line continued"}));
        // (the break began a slide that the next heading named)
        QCOMPARE(titleOf(o.slides[1]), QStringLiteral("Two"));
        QCOMPARE(linesOf(o.slides[1]), (QStringList{"0:plain"}));
        QCOMPARE(titleOf(o.slides[2]), QString());
        QCOMPARE(linesOf(o.slides[2]), (QStringList{"0:After break"}));
    }

    void withoutAHeadingOneTheSecondLevelMakesTheSlides()
    {
        const deck::Outline o = read("## A\n### a1\n- x\n## B\n", "a.md");
        QCOMPARE(o.slides.size(), 2);
        QCOMPARE(titleOf(o.slides[0]), QStringLiteral("A"));
        QCOMPARE(linesOf(o.slides[0]), (QStringList{"0:a1", "0:x"}));
        QCOMPARE(titleOf(o.slides[1]), QStringLiteral("B"));
        // A Markdown file with no heading at all is an outline of plain lines with Markdown's marks.
        const deck::Outline p = read("Slide one\n- **bold** point\n\nSlide two\n", "a.md");
        QCOMPARE(p.slides.size(), 2);
        QVERIFY(p.slides[0].body[0].runs[0].bold);
    }

    void boldItalicAndLinksStayAsRunFormatting()
    {
        const deck::Outline o = read("# T\n- a **b** *c* ***d*** `e` [site](https://example.com/x) and [bad](javascript:alert(1)) x_y_z\n", "a.md");
        const auto &runs = o.slides[0].body[0].runs;
        auto find = [&](const QString &text) -> deck::OutlineRun {
            for (const auto &r : runs)
                if (r.text == text) return r;
            return deck::OutlineRun{QStringLiteral("MISSING")};
        };
        QVERIFY(find("b").bold && !find("b").italic);
        QVERIFY(find("c").italic && !find("c").bold);
        QVERIFY(find("d").italic && find("d").bold);
        QVERIFY(find("e").mono);
        QCOMPARE(find("site").href, QStringLiteral("https://example.com/x"));
        QVERIFY(find("bad").href.isEmpty());   // a link that is not a web or mail address stays text
        QVERIFY(lineText(o.slides[0].body[0]).contains(QLatin1String("x_y_z")));
        QVERIFY(!o.notes.isEmpty());
        // marks with nothing to close them are the letters themselves
        QCOMPARE(lineText(read("# T\n- 2 * 3 and a_b\n", "a.md").slides[0].body[0]), QStringLiteral("2 * 3 and a_b"));
    }

    void fencedCodeIsACodeBlockNotLines()
    {
        const deck::Outline o = read("# Code\n- before\n```cpp\nint a;\n# not a heading\n  indented\n```\n- after\n", "a.md");
        QCOMPARE(o.slides.size(), 1);
        QCOMPARE(o.slides[0].code, (QStringList{"int a;", "# not a heading", "  indented"}));
        QCOMPARE(linesOf(o.slides[0]), (QStringList{"0:before", "0:after"}));
    }

    // Pictures: only from the outline's own folder, never from the network, and only the kinds that open as pictures.
    void picturesComeOnlyFromTheOutlinesOwnFolder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(writeFile(dir.filePath("sub/in.png"), png()));
        QVERIFY(writeFile(dir.filePath("top.png"), png()));
        QTemporaryDir other;
        QVERIFY(writeFile(other.filePath("out.png"), png()));
        QVERIFY(QFile::link(other.filePath("out.png"), dir.filePath("link.png")) || true);
        const QString md = QStringLiteral("# P\n![a](sub/in.png)\n![b](top.png)\n![c](../x.png)\n![d](https://example.com/p.png)\n![e](%1)\n![f](link.png)\n![g](missing.png)\n![h](top.txt)\n")
                               .arg(other.filePath("out.png"));
        const deck::Outline o = read(md.toUtf8(), "a.md", dir.path());
        QCOMPARE(o.slides[0].pictures.size(), 2);
        QCOMPARE(o.slides[0].pictures[0].alt, QStringLiteral("a"));
        QVERIFY(!o.slides[0].pictures[0].bytes.isEmpty());
        QVERIFY(!o.notes.isEmpty());   // the rest is said
        // with no folder no picture is read at all
        QCOMPARE(read(md.toUtf8(), "a.md", QString()).slides[0].pictures.size(), 0);
        // no more than four on a slide
        QString many = QStringLiteral("# M\n");
        for (int i = 0; i < 10; ++i) many += QStringLiteral("![](top.png)\n");
        QCOMPARE(read(many.toUtf8(), "a.md", dir.path()).slides[0].pictures.size(), deck::kMaxOutlinePicturesPerSlide);
    }

    // Limits: counted work, not a time.
    void hostileInputIsBoundedWhereItIsBuilt()
    {
        QByteArray many;
        for (int i = 0; i < deck::kMaxOutlineSlides + 50; ++i) many += "S\n";
        const deck::Outline a = read(many);
        QCOMPARE(a.slides.size(), deck::kMaxOutlineSlides);
        QVERIFY(!a.notes.isEmpty());
        // three hundred levels of indent: at most the app's deepest level
        QByteArray deep = "T\n";
        for (int i = 1; i <= 300; ++i) deep += QByteArray(i, ' ') + "x\n";
        const deck::Outline b = read(deep);
        QCOMPARE(b.slides[0].body.size(), 300);
        int deepest = 0;
        for (const auto &l : b.slides[0].body) deepest = std::max(deepest, l.level);
        QCOMPARE(deepest, deck::kMaxOutlineLevel);
        // one line is cut at its limit, and a slide holds so many lines
        const deck::Outline c = read("T\n  " + QByteArray(deck::kMaxOutlineLineChars * 3, 'a') + "\n");
        QCOMPARE(lineText(c.slides[0].body[0]).size(), deck::kMaxOutlineLineChars);
        QByteArray lines = "T\n";
        for (int i = 0; i < deck::kMaxOutlineLinesPerSlide + 100; ++i) lines += "  l\n";
        QCOMPARE(read(lines).slides[0].body.size(), deck::kMaxOutlineLinesPerSlide);
        QByteArray code = "# T\n```\n";
        for (int i = 0; i < deck::kMaxOutlineCodeLines + 100; ++i) code += "c\n";
        code += "```\n";
        QCOMPARE(read(code, "a.md").slides[0].code.size(), deck::kMaxOutlineCodeLines);
    }

    // A line is cut at its limit before anything reads it, and every scan for a closing mark is bounded: the work is counted (scanSteps), not timed.
    void aHugeLineIsCutBeforeItIsReadAndScansAreBounded()
    {
        struct Case {
            const char *unit;
            qint64 stepsPerChar;   // the most the bounded scans come to for each character of the cut line
        };
        for (const Case &c : {Case{"[", 700}, Case{"<http", 150}, Case{"![", 700}, Case{"[a](", 700}, Case{"*a", 700}, Case{"_ _", 700}, Case{"`", 700}, Case{"[[", 700}}) {
            QByteArray line;
            while (line.size() < 200000) line += c.unit;
            const deck::Outline o = read("# T\n" + line + "\n- " + line + "\n", "a.md");
            QVERIFY2(!o.slides.isEmpty(), c.unit);
            QVERIFY2(o.scanSteps > 0 || QByteArray(c.unit) == "_ _" || QByteArray(c.unit) == "`", c.unit);
            // two lines, each cut to the limit before it is read
            QVERIFY2(o.scanSteps <= 2 * qint64(deck::kMaxOutlineLineChars) * c.stepsPerChar, qPrintable(QStringLiteral("%1: %2 steps").arg(QString::fromLatin1(c.unit)).arg(o.scanSteps)));
            QVERIFY2(!o.notes.isEmpty(), c.unit);   // what was cut is said
            for (const auto &l : o.slides[0].body) QVERIFY(lineText(l).size() <= deck::kMaxOutlineLineChars);
        }
        // a code line and a plain line are cut too
        const deck::Outline p = read("T\n  " + QByteArray(200000, 'x') + "\n", "a.txt");
        QCOMPARE(lineText(p.slides[0].body[0]).size(), deck::kMaxOutlineLineChars);
        QVERIFY(!p.notes.isEmpty());
    }

    // The lines of one Markdown paragraph are joined, and the joined line is held to the limit too (a 10 MB paragraph was one line of ten million characters).
    void aMergedParagraphIsHeldToTheLineLimit()
    {
        QByteArray md = "# T\n";
        for (int i = 0; i < 10; ++i) md += QByteArray(3000, char('a' + i)) + "\n";
        const deck::Outline o = read(md, "a.md");
        QCOMPARE(o.slides[0].body.size(), 1);
        QCOMPARE(lineText(o.slides[0].body[0]).size(), deck::kMaxOutlineLineChars);
        QVERIFY(!o.notes.isEmpty());
        // many short lines
        QByteArray many = "# T\n";
        for (int i = 0; i < 30000; ++i) many += "word word\n";
        const deck::Outline m = read(many, "a.md");
        QCOMPARE(m.slides[0].body.size(), 1);
        QVERIFY(lineText(m.slides[0].body[0]).size() <= deck::kMaxOutlineLineChars);
        QVERIFY(!m.notes.isEmpty());
        // a paragraph under the limit is not touched
        const deck::Outline small = read("# T\none\ntwo\n", "a.md");
        QCOMPARE(lineText(small.slides[0].body[0]), QStringLiteral("one two"));
        QVERIFY(small.notes.isEmpty());
    }

    // A reading that is told to stop gives nothing, and gives it between lines.
    void aCancelledReadStops()
    {
        QByteArray lines;
        for (int i = 0; i < 5000; ++i) lines += "# S\n- [a](https://example.com)\n";
        std::atomic<bool> stop{true};
        deck::Outline o;
        QString why;
        QVERIFY(!deck::readOutline(lines, "a.md", QString(), &o, &why, &stop));
        QVERIFY(o.slides.isEmpty());
        QCOMPARE(o.scanSteps, qint64(0));
        stop = false;
        QVERIFY(deck::readOutline(lines, "a.md", QString(), &o, &why, &stop));
        QCOMPARE(o.slides.size(), 5000);
    }

    // A link's address is read to its own closing parenthesis (balanced ones inside are part of it); one that does not close is plain text, and said.
    void aLinkAddressMayHoldBalancedParentheses()
    {
        const deck::Outline o = read("# T\n- [wiki](https://en.example.org/wiki/Foo_(bar)) then (this) text\n- [x](https://example.com/a(b(c))d)e\n- [y](<https://example.com/a b)c>) end\n", "a.md");
        const auto &body = o.slides[0].body;
        QCOMPARE(body.size(), 3);
        QCOMPARE(body[0].runs[0].href, QStringLiteral("https://en.example.org/wiki/Foo_(bar)"));
        QCOMPARE(lineText(body[0]), QStringLiteral("wiki then (this) text"));
        QCOMPARE(body[1].runs[0].href, QStringLiteral("https://example.com/a(b(c))d"));
        QCOMPARE(lineText(body[1]), QStringLiteral("xe"));
        QCOMPARE(body[2].runs[0].href, QStringLiteral("https://example.com/a b)c"));
        QCOMPARE(lineText(body[2]), QStringLiteral("y end"));
        QVERIFY(o.notes.isEmpty());
        // an address that does not close, or is nested past the limit, stays as it was written, as plain text
        const deck::Outline bad = read("# T\n- [a](https://example.com/(b) tail\n- [c](https://example.com/" + QByteArray(50, '(') + QByteArray(50, ')') + ") tail\n", "a.md");
        QCOMPARE(lineText(bad.slides[0].body[0]), QStringLiteral("[a](https://example.com/(b) tail"));
        QCOMPARE(bad.slides[0].body[0].runs.size(), 1);
        QVERIFY(bad.slides[0].body[0].runs[0].href.isEmpty());
        QVERIFY(lineText(bad.slides[0].body[1]).startsWith(QLatin1String("[c](https://example.com/((")));
        QVERIFY(bad.slides[0].body[1].runs[0].href.isEmpty());
        QVERIFY(!bad.notes.isEmpty());
        // Save a Copy writes such an address so that it reads back whole
        deck::Outline made;
        deck::OutlineSlide slide;
        deck::OutlineRun title;
        title.text = QStringLiteral("T");
        slide.title << title;
        for (const QString &href : {QStringLiteral("https://example.com/Foo_(bar)"), QStringLiteral("https://example.com/a)b"), QStringLiteral("https://example.com/a(b"), QStringLiteral("https://example.com/a b")}) {
            deck::OutlineLine line;
            deck::OutlineRun r;
            r.text = QStringLiteral("link");
            r.href = href;
            line.runs << r;
            slide.body << line;
        }
        made.slides << slide;
        const auto doc = deck::presentationFromOutline(made);
        const QString text = deck::outlineMarkdown(*doc);
        const deck::Outline back = read(text.toUtf8(), "a.md");
        QCOMPARE(back.slides[0].body.size(), 4);
        for (int i = 0; i < 4; ++i) {
            QVERIFY2(!back.slides[0].body[i].runs.isEmpty(), qPrintable(text));
            QCOMPARE(back.slides[0].body[i].runs[0].href, slide.body[i].runs[0].href);
            QCOMPARE(lineText(back.slides[0].body[i]), QStringLiteral("link"));
        }
    }

    // What the importer would strip or read as marks is written so that it reads back as the same words.
    void saveACopyKeepsTitlesAndLinesThatLookLikeMarkup()
    {
        const QStringList words = {QStringLiteral("Plan #"),        QStringLiteral("Plan ##"),       QStringLiteral("# Plan"),       QStringLiteral("#hashtag"),
                                   QStringLiteral("C#"),            QStringLiteral("---"),           QStringLiteral("- item"),       QStringLiteral("+ item"),
                                   QStringLiteral("1. item"),       QStringLiteral("2) item"),       QStringLiteral("`code`"),       QStringLiteral("``"),
                                   QStringLiteral("[a](b)"),        QStringLiteral("![x](y.png)"),   QStringLiteral("> quote"),     QStringLiteral("```"),
                                   QStringLiteral("~~~"),           QStringLiteral("*"),             QStringLiteral("**bold**"),     QStringLiteral("snake_case_name"),
                                   QStringLiteral("<https://example.com>"), QStringLiteral("back\\slash \\#"), QStringLiteral("a # b #"),  QStringLiteral("ends with a backslash \\"),
                                   QStringLiteral("=== and ___"),   QStringLiteral("\\"),           QStringLiteral("# #")};
        deck::Outline made;
        for (const QString &w : words) {
            deck::OutlineSlide slide;
            deck::OutlineRun t;
            t.text = w;
            slide.title << t;
            for (int level = 0; level < 2; ++level) {
                deck::OutlineLine line;
                line.level = level;
                line.runs << t;
                slide.body << line;
            }
            made.slides << slide;
        }
        const auto doc = deck::presentationFromOutline(made);
        const QString text = deck::outlineMarkdown(*doc);
        const deck::Outline back = read(text.toUtf8(), "a.md");
        QCOMPARE(back.slides.size(), made.slides.size());
        for (int i = 0; i < words.size(); ++i) {
            const QString want = words[i].simplified();
            QCOMPARE(titleOf(back.slides[i]), want);
            QCOMPARE(back.slides[i].body.size(), 2);
            QCOMPARE(lineText(back.slides[i].body[0]), want);
            QCOMPARE(lineText(back.slides[i].body[1]), want);
            QCOMPARE(back.slides[i].body[1].level, 1);
        }
    }

    // A picture has to be a raster picture whatever its name says, and its pixels, and all the pictures' pixels, are held to limits.
    void picturesAreCheckedByTheirBytesAndPixels()
    {
        QTemporaryDir dir;
        QVERIFY(writeFile(dir.filePath("good.png"), png()));
        // a PDF, and bytes that are nothing, under a picture's name
        QVERIFY(writeFile(dir.filePath("doc.png"), tinyPdf()));
        QVERIFY(writeFile(dir.filePath("junk.jpg"), "not a picture at all, just words"));
        const deck::Outline o = read("# P\n![g](good.png)\n![d](doc.png)\n![j](junk.jpg)\n", "a.md", dir.path());
        QCOMPARE(o.slides[0].pictures.size(), 1);
        QCOMPARE(o.slides[0].pictures[0].alt, QStringLiteral("g"));
        QVERIFY(!o.notes.isEmpty());
        // one picture past the pixel limit
        QVERIFY(writeFile(dir.filePath("big.png"), png(4100, 4100)));   // 16.8 million pixels
        const deck::Outline big = read("# P\n![b](big.png)\n![g](good.png)\n", "a.md", dir.path());
        QCOMPARE(big.slides[0].pictures.size(), 1);
        QCOMPARE(big.slides[0].pictures[0].alt, QStringLiteral("g"));
        QVERIFY(!big.notes.isEmpty());
        // all the pictures together: 9 million pixels each, seven fit in 64 million
        QVERIFY(writeFile(dir.filePath("nine.png"), png(3000, 3000)));
        QString md;
        for (int i = 0; i < 9; ++i) md += QStringLiteral("# S%1\n![n](nine.png)\n").arg(i);
        const deck::Outline total = read(md.toUtf8(), "a.md", dir.path());
        int kept = 0;
        for (const auto &s : total.slides) kept += int(s.pictures.size());
        QCOMPARE(kept, 7);
        QVERIFY(!total.notes.isEmpty());
    }

    void richTextAndDocxHeadingsMakeSlides()
    {
        // .docx, built in memory: Heading 1 is the title, Heading 2 a line, the other paragraphs lines under the heading above.
        office::wml::Document doc = office::wml::newDocument();
        auto style = [&](const QString &id, const QString &name) {
            office::wml::Style s;
            s.id = id;
            s.name = name;
            s.type = QStringLiteral("paragraph");
            doc.styles.styles.push_back(s);
        };
        style("Normal", "Normal");
        style("Heading1", "heading 1");
        style("Heading2", "heading 2");
        auto para = [&](const QString &styleId, const QString &text, bool bold = false) {
            office::wml::Block b;
            b.para.props.style = styleId;
            office::wml::Inline in;
            in.text = text;
            if (bold) in.props.bold = true;
            b.para.content.push_back(in);
            doc.body.push_back(b);
        };
        para("Heading1", "First");
        para("Normal", "plain under first", true);
        para("Heading2", "Sub heading");
        para("Normal", "under the sub heading");
        para("Heading1", "Second");
        QString why;
        const QByteArray zip = office::wml::writeDocx(doc, &why);
        QVERIFY2(!zip.isEmpty(), qPrintable(why));
        const deck::Outline o = read(zip, "a.docx", QString(), &why);
        QVERIFY2(o.slides.size() == 2, qPrintable(why));
        QCOMPARE(titleOf(o.slides[0]), QStringLiteral("First"));
        QCOMPARE(linesOf(o.slides[0]), (QStringList{"0:plain under first", "0:Sub heading", "1:under the sub heading"}));
        QVERIFY(o.slides[0].body[0].runs[0].bold);
        QCOMPARE(titleOf(o.slides[1]), QStringLiteral("Second"));
        // a .docx with no heading gives no slide, and says why
        office::wml::Document none = doc;
        none.body.clear();
        office::wml::Block b;
        office::wml::Inline in;
        in.text = QStringLiteral("just text");
        b.para.content.push_back(in);
        none.body.push_back(b);
        read(office::wml::writeDocx(none), "a.docx", QString(), &why);
        QVERIFY(why.contains(QLatin1String("heading")));
        // Rich text: heading styles by their names
        const QByteArray rtf = "{\\rtf1\\ansi{\\stylesheet{\\s0 Normal;}{\\s1 heading 1;}{\\s2 heading 2;}}\\pard\\s1 Alpha\\par\\pard\\s2 Beta\\par\\pard\\s0 gamma\\par\\pard\\s1 Delta\\par}";
        const deck::Outline r = read(rtf, "a.rtf", QString(), &why);
        QVERIFY2(r.slides.size() == 2, qPrintable(why));
        QCOMPARE(titleOf(r.slides[0]), QStringLiteral("Alpha"));
        QCOMPARE(linesOf(r.slides[0]), (QStringList{"0:Beta", "1:gamma"}));
    }

    // The slides made: Title and Content with real placeholders, formatted runs, levels, and all in one undo step that restores the saved bytes.
    void slidesAreRealPlaceholderSlides()
    {
        const deck::Outline o = read("# One\n- a **bold** [l](https://example.com)\n  - deeper\n# Two\n", "a.md");
        auto d = deck::newPresentation();
        const int made = deck::insertOutline(*d, 1, o);
        QCOMPARE(made, 2);
        QCOMPARE(int(d->pages.size()), 3);
        for (int i = 1; i <= 2; ++i) QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[i])), QStringLiteral("obj"));
        const TextItem *title = placeholderOf(*d, 1, QStringLiteral("title"));
        const TextItem *body = placeholderOf(*d, 1, QStringLiteral("obj"));
        QVERIFY(title && body);
        QCOMPARE(storyText(*d, *title), QStringLiteral("One"));
        QCOMPARE(storyText(*d, *body), QStringLiteral("a bold l\ndeeper"));
        QCOMPARE(body->autofit, TextItem::ShrinkOnOverflow);
        const QTextDocument *story = d->storyDoc(body->storyId);
        QTextCursor c(story->findBlockByNumber(0));
        c.setPosition(c.block().position() + 3);
        c.movePosition(QTextCursor::NextCharacter);
        QVERIFY(c.charFormat().fontWeight() >= QFont::Bold);
        c.setPosition(c.block().position() + int(c.block().text().size()) - 1);
        c.movePosition(QTextCursor::NextCharacter);
        QCOMPARE(c.charFormat().anchorHref(), QStringLiteral("https://example.com"));
        QVERIFY(story->findBlockByNumber(1).blockFormat().intProperty(tp::ListLevel) == 1 || story->findBlockByNumber(1).textList());
        QVERIFY(story->findBlockByNumber(1).blockFormat().leftMargin() > story->findBlockByNumber(0).blockFormat().leftMargin());
        // a slide with code and a picture: the code is a monospaced text box, the picture a picture, the body place gives way when it holds no text
        QTemporaryDir dir;
        QVERIFY(writeFile(dir.filePath("p.png"), png()));
        const deck::Outline x = read("# X\n```\ncode\n```\n![alt](p.png)\n", "a.md", dir.path());
        deck::insertOutline(*d, 3, x);
        int pictures = 0, boxes = 0;
        for (const auto &it : d->pages[3]->items) {
            if (it->type() == ItemType::Picture) ++pictures;
            if (it->type() == ItemType::Text && deck::placeholderType(*it).isEmpty()) ++boxes;
        }
        QCOMPARE(pictures, 1);
        QCOMPARE(boxes, 1);
        QVERIFY(!placeholderOf(*d, 3, QStringLiteral("obj")));
        // a new presentation holds the outline's slides only
        const auto fresh = deck::presentationFromOutline(o);
        QCOMPARE(int(fresh->pages.size()), 2);
    }

    // The window: Slides from Outline after the current slide as one undo step, and File > Open as a new presentation that Save asks a name for.
    void theWindowInsertsAndOpensOutlines()
    {
        QTemporaryDir dir;
        const QString md = dir.filePath("talk.md");
        QVERIFY(writeFile(md, "# A\n- a1\n# B\n- b1\n# C\n"));
        DeckWindow w;
        Quiet quiet(w);
        w.editor()->setDocument(deck::newPresentation());
        QVERIFY(w.act(QStringLiteral("slide.fromOutline")));
        QCOMPARE(w.act(QStringLiteral("slide.fromOutline"))->text(), QStringLiteral("Slides from Outline…"));
        const QByteArray before = [&] {
            QString err;
            QStringList skipped;
            const QString p = dir.filePath("before.pptx");
            deck::saveAnyPresentationFile(*w.editor()->doc(), p, &err, &skipped);
            QFile f(p);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }();
        const int steps = w.editor()->undoStack()->count();
        QVERIFY(w.insertOutlineFile(md));   // (the file is read on a thread of its own: the slides come in when that ends)
        QTRY_COMPARE(int(w.editor()->doc()->pages.size()), 4);
        QCOMPARE(w.editor()->undoStack()->count(), steps + 1);
        QCOMPARE(w.editor()->currentPage(), 1);
        QCOMPARE(storyText(*w.editor()->doc(), *placeholderOf(*w.editor()->doc(), 2, QStringLiteral("title"))), QStringLiteral("B"));
        w.editor()->undo();
        QCOMPARE(int(w.editor()->doc()->pages.size()), 1);
        QString err;
        QStringList skipped;
        const QString after = dir.filePath("after.pptx");
        QVERIFY(deck::saveAnyPresentationFile(*w.editor()->doc(), after, &err, &skipped));
        QFile f(after);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll(), before);
        // a file that gives no slide changes nothing and says why
        QVERIFY(writeFile(dir.filePath("empty.txt"), "\n\n"));
        quiet.said.clear();
        const int stepsNow = w.editor()->undoStack()->count();
        QVERIFY(w.insertOutlineFile(dir.filePath("empty.txt")));
        QTRY_COMPARE(quiet.said.size(), 1);
        QCOMPARE(w.editor()->undoStack()->count(), stepsNow);
        // File > Open: a new, unsaved presentation of the slides
        DeckWindow o;
        Quiet quietOpen(o);
        QVERIFY(o.openFile(md));
        QTRY_COMPARE(int(o.editor()->doc()->pages.size()), 3);
        QVERIFY(o.editor()->filePath().isEmpty());
        QVERIFY(o.editor()->isModified());
        QCOMPARE(o.editor()->displayName(), QStringLiteral("talk"));
        QVERIFY(QFileInfo::exists(md));
    }

    // The file is read on a thread of its own with no event loop of the window's own: closing the window (or deleting it) during the read stops the read, waits for
    // it, and nothing is touched afterward.
    void closingTheWindowDuringAReadStopsItAndIsSafe()
    {
        QTemporaryDir dir;
        const QString md = dir.filePath("talk.md");
        QVERIFY(writeFile(md, "# A\n- a1\n# B\n"));
        constexpr int kSpins = 200000000;   // (the most the held read waits; it ends at once when the close stops it)
        for (int how = 0; how < 4; ++how) {   // 0: Slides from Outline, closed; 1: the same, deleted; 2: File > Open, closed; 3: File > Open, deleted
            QSemaphore entered;
            std::atomic<int> stopped{0}, ended{0};
            auto *w = new DeckWindow;
            QPointer<DeckWindow> guard(w);
            QStringList said;
            w->setNotifier([&said](const QString &, const QString &text) { said << text; });
            w->editor()->setDocument(deck::newPresentation());
            const int pages = int(w->editor()->doc()->pages.size());
            w->setOutlineReadHold([&](const std::atomic<bool> &cancel) {
                entered.release();
                for (int i = 0; i < kSpins && !cancel.load(); ++i) QThread::yieldCurrentThread();
                if (cancel.load()) ++stopped;
                ++ended;
            });
            QVERIFY(how < 2 ? w->insertOutlineFile(md) : w->openFile(md));
            // it came back at once: the read was not waited for with a loop of this window's
            QCOMPARE(ended.load(), 0);
            QCOMPARE(int(w->editor()->doc()->pages.size()), pages);
            QTRY_COMPARE(entered.available(), 1);
            if (how % 2 == 0) {
                w->setAttribute(Qt::WA_DeleteOnClose);
                QVERIFY(w->close());
            } else {
                delete w;
            }
            // the read was stopped and has ended before the close or the delete came back
            QCOMPARE(stopped.load(), 1);
            QCOMPARE(ended.load(), 1);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            QCoreApplication::processEvents();
            QVERIFY(guard.isNull());
            QVERIFY(said.isEmpty());   // (nothing was shown for a read nobody waited for)
        }
    }

    // A read that is not stopped ends in the window with the hold only delaying it.
    void aHeldReadFinishesInTheWindowWhenNothingStopsIt()
    {
        QTemporaryDir dir;
        const QString md = dir.filePath("talk.md");
        QVERIFY(writeFile(md, "# A\n- a1\n# B\n"));
        DeckWindow w;
        QSemaphore entered, go;
        w.setOutlineReadHold([&](const std::atomic<bool> &cancel) {
            entered.release();
            for (int i = 0; i < 200000000 && !cancel.load() && !go.tryAcquire(); ++i) QThread::yieldCurrentThread();
        });
        w.editor()->setDocument(deck::newPresentation());
        QVERIFY(w.insertOutlineFile(md));
        QTRY_COMPARE(entered.available(), 1);
        QCOMPARE(int(w.editor()->doc()->pages.size()), 1);
        go.release();
        QTRY_COMPARE(int(w.editor()->doc()->pages.size()), 3);
        w.setOutlineReadHold(nullptr);
    }

    // Save a Copy as an outline, and what it leaves out is said.
    void saveACopyAsAMarkdownOutline()
    {
        QTemporaryDir dir;
        DeckWindow w;
        Quiet quiet(w);
        auto d = deck::newPresentation();
        deck::insertOutline(*d, 1, read("# One\n- a **bold** [l](https://example.com)\n  - deeper *it*\n# Two\n", "a.md"));
        w.editor()->setDocument(std::move(d));
        const QString path = dir.filePath("out.md");
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        QVERIFY(w.saveCopy());
        QFile f(path);
        QVERIFY2(f.open(QIODevice::ReadOnly), qPrintable(path + quiet.said.join('|')));
        const QString text = QString::fromUtf8(f.readAll());
        QVERIFY2(text.contains(QLatin1String("# One\n- a **bold** [l](https://example.com)\n  - deeper *it*\n")), qPrintable(text));
        QCOMPARE(quiet.said.size(), 1);   // what it leaves out
        // it reads back as the same slides
        const deck::Outline back = read(text.toUtf8(), "out.md");
        QCOMPARE(back.slides.size(), 3);   // (the Title Slide a new presentation starts with has an empty title: # alone)
        QCOMPARE(titleOf(back.slides[1]), QStringLiteral("One"));
        QCOMPARE(linesOf(back.slides[1]), (QStringList{"0:a bold l", "1:deeper it"}));
        // Check Compatibility lists the kind
        QCOMPARE(w.compatHost().formats.size(), 3);
    }

    // Pictures of the slides an outline makes, for a person to look at (only when the folder is named).
    void screenshotsOfImportedSlides()
    {
        const QString shots = qEnvironmentVariable("JD_OUTLINE_SHOT_DIR");
        if (shots.isEmpty()) QSKIP("JD_OUTLINE_SHOT_DIR is not set");
        QDir().mkpath(shots);
        QTemporaryDir dir;
        QVERIFY(writeFile(dir.filePath("team.png"), png(300, 200)));
        const deck::Outline o = read("# Welcome\n- Our **plan** for the year\n  - Grow the *team*\n  - Read the [docs](https://example.com)\n# Code and picture\n- Notes on the left\n```\nint main() {\n  return 0;\n}\n```\n![Team](team.png)\n# Only code\n```\nprintf(\"hi\");\n```\n", "a.md", dir.path());
        const auto d = deck::presentationFromOutline(o);
        LayoutCache cache;
        for (int i = 0; i < d->pages.size(); ++i) QVERIFY(deck::slideImage(*d, &cache, i, 960).save(shots + QStringLiteral("/slide%1.png").arg(i + 1)));
    }
};

JP_TEST_CLASS(DeckOutlineImportTests)
#include "tst_deck_outlineimport.moc"
