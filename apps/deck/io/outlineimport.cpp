// Slides from an outline (see outlineimport.h): the readers (text, Markdown, rich text, .docx) make an Outline, which insertOutline puts in a presentation.

#include "io/outlineimport.h"

#include "core/deck.h"
#include "core/items.h"
#include "text/textprops.h"
#include "rtf/rtf.h"
#include "wml/wml.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringDecoder>
#include <QTextBlock>
#include <QTextList>
#include <QTextCursor>
#include <QTextDocument>

#include <algorithm>

namespace jp::deck {

namespace wml = jp::office::wml;

namespace {

QString tr(const char *text, int n = -1) { return QCoreApplication::translate("OutlineImport", text, nullptr, n); }

// What the readers count as they go, said at the end as the outline's notes.
struct Counts {
    int slidesLeft = 0, linesCut = 0, deepLines = 0, picturesLeft = 0, codeLeft = 0, tables = 0, odd = 0, objects = 0;
    bool slidesCapped = false;
};

bool allowedHref(const QString &h)
{
    if (h.size() > 2000) return false;
    return h.startsWith(QLatin1String("http://"), Qt::CaseInsensitive) || h.startsWith(QLatin1String("https://"), Qt::CaseInsensitive) ||
           h.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive);
}

void addRun(QVector<OutlineRun> &runs, const OutlineRun &r)
{
    if (r.text.isEmpty()) return;
    if (!runs.isEmpty()) {
        OutlineRun &l = runs.last();
        if (l.bold == r.bold && l.italic == r.italic && l.mono == r.mono && l.href == r.href) {
            l.text += r.text;
            return;
        }
    }
    runs.append(r);
}

// ---- what the readers hand slides to

class Sink {
public:
    explicit Sink(Outline *o) : m_o(o) {}
    Counts counts;
    OutlineSlide *current() const { return m_cur; }
    void startSlide(const QVector<OutlineRun> &title)
    {
        m_cur = nullptr;
        if (m_o->slides.size() >= kMaxOutlineSlides) {
            counts.slidesCapped = true;
            ++counts.slidesLeft;
            return;
        }
        m_o->slides.append(OutlineSlide());
        m_cur = &m_o->slides.last();
        m_cur->title = title;
        limit(m_cur->title);
        m_bodyLines = 0;
    }
    bool ensureSlide()
    {
        if (!m_cur && !counts.slidesCapped) startSlide({});
        return m_cur;
    }
    void addLine(int level, QVector<OutlineRun> runs, int deep = 0)
    {
        if (!ensureSlide()) return;
        if (m_bodyLines >= kMaxOutlineLinesPerSlide || m_lines >= kMaxOutlineLines) {
            ++counts.linesCut;
            return;
        }
        limit(runs);
        if (level > kMaxOutlineLevel || deep > kMaxOutlineLevel) ++counts.deepLines;
        OutlineLine l;
        l.level = std::clamp(level, 0, kMaxOutlineLevel);
        l.runs = runs;
        m_cur->body.append(l);
        ++m_bodyLines;
        ++m_lines;
    }
    void addCode(const QString &line)
    {
        if (!ensureSlide()) return;
        if (m_cur->code.size() >= kMaxOutlineCodeLines) {
            ++counts.codeLeft;
            return;
        }
        m_cur->code << line.left(kMaxOutlineLineChars);
    }
    void addPicture(const OutlinePicture &p)
    {
        if (!ensureSlide()) return;
        m_cur->pictures.append(p);
    }
    int pictureCount() const { return m_pictures; }
    qint64 pictureBytes() const { return m_pictureBytes; }
    void picturesRead(qint64 bytes)
    {
        ++m_pictures;
        m_pictureBytes += bytes;
    }
    int picturesOnSlide() { return ensureSlide() ? int(m_cur->pictures.size()) : 0; }

private:
    void limit(QVector<OutlineRun> &runs)
    {
        int room = kMaxOutlineLineChars;
        for (int i = 0; i < runs.size(); ++i) {
            if (room <= 0) {
                runs.resize(i);
                ++counts.linesCut;
                return;
            }
            if (runs[i].text.size() > room) {
                runs[i].text.truncate(room);
                runs.resize(i + 1);
                ++counts.linesCut;
                return;
            }
            room -= int(runs[i].text.size());
        }
    }
    Outline *m_o;
    OutlineSlide *m_cur = nullptr;
    int m_bodyLines = 0, m_lines = 0, m_pictures = 0;
    qint64 m_pictureBytes = 0;
};

// ---- Markdown's own marks inside a line

struct InlineState {
    bool bold = false, italic = false;
    QString href;
};

struct Image {
    QString alt, path;
};

void inlineRuns(const QString &s, InlineState st, QVector<OutlineRun> &out, QVector<Image> *images, Counts &counts, int depth)
{
    QString buf;
    auto flush = [&] {
        OutlineRun r;
        r.text = buf;
        r.bold = st.bold;
        r.italic = st.italic;
        r.href = st.href;
        addRun(out, r);
        buf.clear();
    };
    const int n = int(s.size());
    for (int i = 0; i < n;) {
        const QChar c = s[i];
        if (c == QLatin1Char('\\') && i + 1 < n && s[i + 1].isPunct()) {
            buf += s[i + 1];
            i += 2;
        } else if (c == QLatin1Char('`')) {
            const int j = int(s.indexOf(QLatin1Char('`'), i + 1));
            if (j > i + 1) {
                flush();
                OutlineRun r;
                r.text = s.mid(i + 1, j - i - 1);
                r.bold = st.bold;
                r.italic = st.italic;
                r.mono = true;
                r.href = st.href;
                addRun(out, r);
                i = j + 1;
            } else {
                buf += c;
                ++i;
            }
        } else if ((c == QLatin1Char('[') || (c == QLatin1Char('!') && i + 1 < n && s[i + 1] == QLatin1Char('['))) && depth < 3) {
            const bool image = c == QLatin1Char('!');
            const int open = i + (image ? 1 : 0);
            int level = 0, close = -1;
            for (int k = open; k < n; ++k) {
                if (s[k] == QLatin1Char('\\')) ++k;
                else if (s[k] == QLatin1Char('[')) ++level;
                else if (s[k] == QLatin1Char(']') && --level == 0) {
                    close = k;
                    break;
                }
            }
            int end = -1;
            QString dest;
            if (close > 0 && close + 1 < n && s[close + 1] == QLatin1Char('(')) {
                end = int(s.indexOf(QLatin1Char(')'), close + 2));
                if (end > 0) {
                    dest = s.mid(close + 2, end - close - 2).trimmed();
                    const int sp = int(dest.indexOf(QLatin1Char(' ')));
                    if (sp > 0) dest.truncate(sp);   // (a title after the address is not kept)
                    if (dest.startsWith(QLatin1Char('<')) && dest.endsWith(QLatin1Char('>'))) dest = dest.mid(1, dest.size() - 2);
                }
            }
            if (end < 0) {
                buf += c;
                ++i;
                continue;
            }
            flush();
            const QString label = s.mid(open + 1, close - open - 1);
            if (image) {
                if (images) images->append({label, dest});
                else ++counts.picturesLeft;
            } else {
                InlineState inner = st;
                if (allowedHref(dest)) inner.href = dest;
                else if (!dest.isEmpty() && !dest.startsWith(QLatin1Char('#'))) ++counts.odd;
                inlineRuns(label, inner, out, images, counts, depth + 1);
            }
            i = end + 1;
        } else if (c == QLatin1Char('<') && (s.mid(i + 1, 4) == QLatin1String("http") || s.mid(i + 1, 7) == QLatin1String("mailto:"))) {
            const int j = int(s.indexOf(QLatin1Char('>'), i + 1));
            const QString url = j > 0 ? s.mid(i + 1, j - i - 1) : QString();
            if (!url.isEmpty() && !url.contains(QLatin1Char(' ')) && allowedHref(url)) {
                flush();
                InlineState inner = st;
                inner.href = url;
                OutlineRun r;
                r.text = url;
                r.bold = st.bold;
                r.italic = st.italic;
                r.href = url;
                addRun(out, r);
                i = j + 1;
            } else {
                buf += c;
                ++i;
            }
        } else if (c == QLatin1Char('*') || c == QLatin1Char('_')) {
            int run = 1;
            while (i + run < n && s[i + run] == c) ++run;
            const QChar before = i > 0 ? s[i - 1] : QChar(QLatin1Char(' '));
            const QChar after = i + run < n ? s[i + run] : QChar(QLatin1Char(' '));
            const bool under = c == QLatin1Char('_');
            const int take = run >= 3 ? 3 : run;   // (more than three is the marks themselves)
            const bool wantBold = take >= 2, wantItalic = take != 2;
            // Both looks off: this opens them, when something later closes them. Both on: it closes them.
            const bool opening = (wantBold ? !st.bold : true) && (wantItalic ? !st.italic : true);
            const bool closing = (wantBold ? st.bold : true) && (wantItalic ? st.italic : true);
            const QString mark = s.mid(i, take);
            bool done = false;
            if (opening && !after.isSpace() && !(under && before.isLetterOrNumber()) && s.indexOf(mark, i + take) > 0) {
                flush();
                if (wantBold) st.bold = true;
                if (wantItalic) st.italic = true;
                done = true;
            } else if (closing && !before.isSpace() && !(under && after.isLetterOrNumber())) {
                flush();
                if (wantBold) st.bold = false;
                if (wantItalic) st.italic = false;
                done = true;
            }
            if (done) {
                i += take;
            } else {
                buf += s.mid(i, run);
                i += run;
            }
        } else {
            buf += c;
            ++i;
        }
    }
    flush();
}

QVector<OutlineRun> markdownRuns(const QString &text, QVector<Image> *images, Counts &counts)
{
    QVector<OutlineRun> out;
    inlineRuns(text, InlineState(), out, images, counts, 0);
    return out;
}

QVector<OutlineRun> plainRuns(const QString &text)
{
    QVector<OutlineRun> out;
    OutlineRun r;
    r.text = text;
    addRun(out, r);
    return out;
}

// ---- pictures named by Markdown, only from the outline's own folder

void takePicture(Sink &sink, const QString &folder, const Image &img)
{
    auto leave = [&] { ++sink.counts.picturesLeft; };
    const QString rel = img.path;
    if (folder.isEmpty() || rel.isEmpty() || rel.contains(QLatin1Char(':')) || rel.startsWith(QLatin1Char('/')) || rel.startsWith(QLatin1Char('\\')) || rel.startsWith(QLatin1Char('~'))) return leave();
    for (const QString &part : rel.split(QRegularExpression(QStringLiteral("[/\\\\]"))))
        if (part == QLatin1String("..")) return leave();
    static const QStringList kinds = {QStringLiteral("png"), QStringLiteral("jpg"), QStringLiteral("jpeg"), QStringLiteral("gif"), QStringLiteral("bmp"), QStringLiteral("webp"),
                                      QStringLiteral("tif"), QStringLiteral("tiff")};
    if (!kinds.contains(QFileInfo(rel).suffix().toLower())) return leave();
    if (sink.pictureCount() >= kMaxOutlinePictures || sink.picturesOnSlide() >= kMaxOutlinePicturesPerSlide) return leave();
    const QString root = QFileInfo(folder).canonicalFilePath();
    const QFileInfo fi(QDir(root).filePath(rel));
    const QString canon = fi.canonicalFilePath();   // (empty when it is not there; a link out of the folder lands outside it)
    if (root.isEmpty() || canon.isEmpty() || !canon.startsWith(root + QLatin1Char('/')) || !fi.isFile()) return leave();
    if (fi.size() > kMaxOutlinePictureBytes || sink.pictureBytes() + fi.size() > kMaxOutlinePictureTotal) return leave();
    OutlinePicture p;
    QSize pixels;
    if (!readPictureFile(canon, 0, &p.bytes, &p.format, &pixels)) return leave();
    p.path = canon;
    p.alt = img.alt;
    p.pixels = pixels;
    sink.picturesRead(p.bytes.size());
    sink.addPicture(p);
}

// ---- lines of text, and the levels their indenting makes

template <typename F>
void forEachLine(const QString &text, F &&f)
{
    const qsizetype n = text.size();
    qsizetype start = 0;
    for (qsizetype i = 0; i <= n; ++i) {
        const bool end = i == n;
        const QChar c = end ? QChar() : text[i];
        if (end || c == QLatin1Char('\n') || c == QLatin1Char('\r')) {
            if (!end || start < n) f(QStringView(text).mid(start, i - start));
            if (!end && c == QLatin1Char('\r') && i + 1 < n && text[i + 1] == QLatin1Char('\n')) ++i;
            start = i + 1;
        }
    }
}

// The columns a line's leading space comes to (a tab goes to the next multiple of four), and where its text starts.
int indentOf(QStringView line, int *textAt)
{
    int cols = 0, i = 0;
    for (; i < line.size(); ++i) {
        if (line[i] == QLatin1Char(' ')) ++cols;
        else if (line[i] == QLatin1Char('\t')) cols = (cols / 4 + 1) * 4;
        else break;
    }
    *textAt = i;
    return cols;
}

// The levels the indents stand for, as a stack of the deeper ones in turn: an indent past the one before is a level in; one that is back at an earlier indent is that level.
class Indents {
public:
    int level(int cols, bool *tooDeep = nullptr)
    {
        while (m_stack.size() > 1 && m_stack.last() > cols) m_stack.removeLast();
        if (m_stack.last() < cols) {
            if (m_stack.size() < kMaxOutlineIndents) m_stack.append(cols);
            else if (tooDeep) *tooDeep = true;
        }
        return int(m_stack.size()) - 1;
    }
    void reset() { m_stack = {0}; }

private:
    QVector<int> m_stack = {0};
};

// A list marker at the start of `rest`: its length (the marker and the space after it), or 0.
int markerLength(QStringView rest)
{
    if (rest.size() >= 2 && (rest[0] == QLatin1Char('-') || rest[0] == QLatin1Char('*') || rest[0] == QLatin1Char('+')) && (rest[1] == QLatin1Char(' ') || rest[1] == QLatin1Char('\t'))) return 2;
    int d = 0;
    while (d < rest.size() && d < 9 && rest[d].isDigit()) ++d;
    if (d > 0 && d + 1 < rest.size() && (rest[d] == QLatin1Char('.') || rest[d] == QLatin1Char(')')) && (rest[d + 1] == QLatin1Char(' ') || rest[d + 1] == QLatin1Char('\t'))) return d + 2;
    return 0;
}

bool isFence(QStringView line, QString *mark = nullptr)
{
    int at = 0;
    while (at < line.size() && at < 4 && line[at] == QLatin1Char(' ')) ++at;
    if (at > 3) return false;
    const QStringView rest = line.mid(at);
    if (rest.startsWith(QLatin1String("```")) || rest.startsWith(QLatin1String("~~~"))) {
        if (mark) *mark = rest.left(3).toString();
        return true;
    }
    return false;
}

// An ATX heading's level (1 to 6) and its text, or 0.
int headingOf(QStringView line, QString *text)
{
    int at = 0;
    while (at < line.size() && at < 4 && line[at] == QLatin1Char(' ')) ++at;
    if (at > 3) return 0;
    int h = 0;
    while (at + h < line.size() && line[at + h] == QLatin1Char('#')) ++h;
    if (h < 1 || h > 6) return 0;
    if (at + h < line.size() && line[at + h] != QLatin1Char(' ') && line[at + h] != QLatin1Char('\t')) return 0;
    QString t = line.mid(at + h).toString().trimmed();
    static const QRegularExpression closing(QStringLiteral("(^|\\s)#+$"));
    t.remove(closing);
    *text = t.trimmed();
    return h;
}

bool isSlideBreak(QStringView line)
{
    const QStringView t = line.trimmed();
    if (t.size() < 3) return false;
    for (const QChar c : t)
        if (c != QLatin1Char('-')) return false;
    return true;
}

void readText(const QString &text, bool markdown, const QString &folder, Sink &sink)
{
    // Markdown with headings: the shallowest heading level makes the slides; without them the file is read as an outline of plain lines.
    int titleLevel = 0;
    if (markdown) {
        bool fenced = false;
        QString mark;
        forEachLine(text, [&](QStringView line) {
            QString m, t;
            if (isFence(line, &m)) {
                if (!fenced) mark = m;
                if (!fenced || m == mark) fenced = !fenced;
                return;
            }
            if (fenced) return;
            if (const int h = headingOf(line, &t); h && (!titleLevel || h < titleLevel)) titleLevel = h;
        });
    }
    const bool headed = titleLevel > 0;
    Indents indents;
    bool fenced = false, brokeSlide = false, paragraph = false;
    QString fenceMark;
    forEachLine(text, [&](QStringView raw) {
        QString m, headText;
        if (markdown && isFence(raw, &m)) {
            if (!fenced) {
                fenced = true;
                fenceMark = m;
                sink.ensureSlide();
                paragraph = false;
                return;
            }
            if (m == fenceMark) {
                fenced = false;
                return;
            }
        }
        if (fenced) {
            sink.addCode(raw.toString());
            return;
        }
        if (raw.trimmed().isEmpty()) {
            paragraph = false;
            return;
        }
        if (markdown && isSlideBreak(raw)) {
            sink.startSlide({});   // (an empty title: a heading right after it names the slide)
            brokeSlide = true;
            indents.reset();
            paragraph = false;
            return;
        }
        QVector<Image> images;
        // (the slide a break began takes the title that comes next)
        auto beginTitled = [&](const QVector<OutlineRun> &runs) {
            if (brokeSlide && sink.current() && sink.current()->title.isEmpty() && sink.current()->body.isEmpty()) sink.current()->title = runs;
            else sink.startSlide(runs);
        };
        auto runsOf = [&](const QString &t) { return markdown ? markdownRuns(t, &images, sink.counts) : plainRuns(t); };
        auto takeImages = [&] {
            for (const Image &img : std::as_const(images)) takePicture(sink, folder, img);
            images.clear();
        };
        if (markdown && headed) {
            if (const int h = headingOf(raw, &headText)) {
                indents.reset();
                paragraph = false;
                if (h == titleLevel) {
                    beginTitled(runsOf(headText));
                    brokeSlide = false;
                    takeImages();
                } else {
                    sink.addLine(h - titleLevel - 1, runsOf(headText));
                    takeImages();
                }
                return;
            }
        }
        int at = 0;
        const int cols = indentOf(raw, &at);
        QStringView rest = raw.mid(at);
        if (markdown && rest.startsWith(QLatin1Char('>'))) rest = rest.mid(1).trimmed();
        const int marker = markerLength(rest);
        const bool listed = marker > 0;
        if (listed) rest = rest.mid(marker).trimmed();
        if (!headed && !listed && cols == 0) {
            // a title line of a plain outline
            indents.reset();
            paragraph = false;
            beginTitled(runsOf(rest.toString().trimmed()));
            brokeSlide = false;
            takeImages();
            return;
        }
        brokeSlide = false;
        bool deep = false;
        int level = indents.level(cols, &deep);
        if (!headed) level = std::max(0, level - 1);   // (the title is the first level)
        const QVector<OutlineRun> runs = runsOf(rest.toString().trimmed());
        // A line of a paragraph goes on the line before it (Markdown's lines wrap).
        if (markdown && headed && !listed && paragraph && sink.current() && !sink.current()->body.isEmpty() && !runs.isEmpty()) {
            OutlineLine &last = sink.current()->body.last();
            OutlineRun space;
            space.text = QStringLiteral(" ");
            addRun(last.runs, space);
            for (const OutlineRun &r : runs) addRun(last.runs, r);
            takeImages();
            return;
        }
        paragraph = markdown && !listed;
        if (!runs.isEmpty()) sink.addLine(level, runs, deep ? kMaxOutlineLevel + 1 : 0);
        takeImages();
    });
}

// ---- rich text and .docx

int headingLevelOf(const wml::Document &doc, const wml::Paragraph &p)
{
    if (p.props.outlineLvl && *p.props.outlineLvl < 9) return *p.props.outlineLvl + 1;
    QString id = p.props.style;
    for (int hops = 0; hops < 10 && !id.isEmpty(); ++hops) {
        const wml::Style *s = doc.styles.find(id);
        if (!s) break;
        static const QRegularExpression heading(QStringLiteral("^heading\\s*([1-9])$"), QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch m = heading.match(s->name);
        if (m.hasMatch()) return m.captured(1).toInt();
        if (s->pPr.outlineLvl && *s->pPr.outlineLvl < 9) return *s->pPr.outlineLvl + 1;
        id = s->basedOn;
    }
    return 0;
}

void readDocument(const wml::Document &doc, Sink &sink)
{
    struct Para {
        int heading = 0, listLevel = -1;
        QVector<OutlineRun> runs;
    };
    QVector<Para> paras;
    int titleLevel = 0;
    QString href;
    for (const wml::Block &b : doc.body) {
        if (b.table) ++sink.counts.tables;
        if (b.isRaw) continue;
        Para para;
        para.heading = headingLevelOf(doc, b.para);
        if (b.para.props.numId && *b.para.props.numId > 0) para.listLevel = std::clamp(b.para.props.ilvl.value_or(0), 0, 100);
        for (const wml::Inline &in : b.para.content) {
            if (in.kind == wml::Inline::Text) {
                OutlineRun r;
                r.text = in.text;
                r.text.replace(QLatin1Char('\t'), QLatin1Char(' '));
                r.text.replace(QChar(0x2028), QLatin1Char(' '));
                r.text.remove(QChar(0x0C));
                r.text.remove(QChar(0x0E));
                r.bold = in.props.bold.value_or(false);
                r.italic = in.props.italic.value_or(false);
                r.href = href;
                addRun(para.runs, r);
            } else if (in.kind == wml::Inline::RunAtom) {
                if (in.tag == QLatin1String("drawing") || in.tag == QLatin1String("pict") || in.tag == QLatin1String("object")) ++sink.counts.objects;
            } else if (in.tag == QLatin1String("hyperlink")) {
                href.clear();
                if (in.role == wml::Inline::Open) {
                    static const QRegularExpression rid(QStringLiteral("r:id=\"([^\"]+)\""));
                    const QRegularExpressionMatch m = rid.match(QString::fromUtf8(in.raw));
                    if (m.hasMatch())
                        for (const auto &rel : doc.mainRelationships)
                            if (rel.id == m.captured(1) && rel.external) {
                                if (allowedHref(rel.target)) href = rel.target;
                                else ++sink.counts.odd;
                            }
                }
            }
        }
        bool any = false;
        for (const OutlineRun &r : para.runs) any = any || !r.text.trimmed().isEmpty();
        if (!any && !para.heading) continue;
        if (para.heading && (!titleLevel || para.heading < titleLevel)) titleLevel = para.heading;
        paras.append(para);
    }
    if (!titleLevel) return;
    int under = 0;   // the levels the heading above puts its paragraphs in
    for (Para &p : paras) {
        if (p.runs.size() == 1) p.runs[0].text = p.runs[0].text.trimmed();
        if (p.heading == titleLevel) {
            sink.startSlide(p.runs);
            under = 0;
        } else if (p.heading > titleLevel) {
            sink.addLine(p.heading - titleLevel - 1, p.runs);
            under = p.heading - titleLevel;
        } else if (p.listLevel >= 0) {
            sink.addLine(under + p.listLevel, p.runs);
        } else {
            sink.addLine(under, p.runs);
        }
    }
}

QStringList notesOf(const Counts &c, const Outline &o)
{
    QStringList n;
    if (c.slidesCapped) n << tr("Only the first %1 slides were made. The rest of the file was left out.").arg(kMaxOutlineSlides);
    if (c.linesCut) n << tr("%n line(s) of text were cut or left out, because a line, a slide, or the file had more than it can hold.", c.linesCut);
    if (c.deepLines) n << tr("%n line(s) were indented deeper than a slide's text goes, and were put at its deepest level.", c.deepLines);
    if (c.picturesLeft) n << tr("%n picture(s) were left out. A picture has to be a PNG, JPEG, GIF, BMP, WebP, or TIFF file in the outline's own folder, or a folder in it.", c.picturesLeft);
    if (c.objects) n << tr("%n picture(s) or other object(s) in the file were left out.", c.objects);
    if (c.codeLeft) n << tr("%n line(s) of code past the limit for a slide were left out.", c.codeLeft);
    if (c.tables) n << tr("The text of tables in the file became lines of text.");
    if (c.odd) n << tr("%n link(s) to other kinds of address were kept as plain text.", c.odd);
    Q_UNUSED(o);
    return n;
}

} // namespace

bool isOutlineFileName(const QString &path)
{
    static const QStringList kinds = {QStringLiteral("txt"), QStringLiteral("text"), QStringLiteral("md"), QStringLiteral("markdown"), QStringLiteral("rtf"), QStringLiteral("docx")};
    return kinds.contains(QFileInfo(path).suffix().toLower());
}

bool decodeOutlineText(const QByteArray &bytes, QString *text, QString *error)
{
    auto refuse = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    if (bytes.size() > kMaxOutlineTextBytes) return refuse(tr("The file is larger than 10 MB, the most an outline can be."));
    QStringDecoder decode(QStringDecoder::Utf8, QStringDecoder::Flag::Stateless);
    QByteArray body = bytes;
    if (bytes.startsWith("\xEF\xBB\xBF")) {
        body = bytes.mid(3);
    } else if (bytes.startsWith("\xFF\xFE")) {
        decode = QStringDecoder(QStringDecoder::Utf16LE, QStringDecoder::Flag::Stateless);
        body = bytes.mid(2);
    } else if (bytes.startsWith("\xFE\xFF")) {
        decode = QStringDecoder(QStringDecoder::Utf16BE, QStringDecoder::Flag::Stateless);
        body = bytes.mid(2);
    } else if (bytes.contains('\0')) {
        return refuse(tr("The file is not text."));
    }
    const QString s = decode.decode(body);
    if (decode.hasError() || s.contains(QChar(0))) return refuse(tr("The file is not text in UTF-8 or UTF-16."));
    *text = s;
    return true;
}

bool readOutline(const QByteArray &bytes, const QString &name, const QString &folder, Outline *out, QString *error)
{
    *out = Outline();
    auto refuse = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    Sink sink(out);
    const QString ending = QFileInfo(name).suffix().toLower();
    if (office::rtf::looksLikeRtf(bytes)) {
        wml::Document doc;
        QString why;
        if (!office::rtf::readRtf(bytes, &doc, &why)) return refuse(why.isEmpty() ? tr("The file can't be read as rich text.") : why);
        readDocument(doc, sink);
        if (out->slides.isEmpty()) return refuse(tr("The file has no headings to make slides from. Give the slides' titles a heading style."));
    } else if (bytes.startsWith("PK")) {
        wml::Document doc;
        QString why;
        if (!wml::readDocx(bytes, &doc, &why)) return refuse(why.isEmpty() ? tr("The file can't be read as a document.") : why);
        readDocument(doc, sink);
        if (out->slides.isEmpty()) return refuse(tr("The file has no headings to make slides from. Give the slides' titles a heading style."));
    } else {
        if (ending == QLatin1String("rtf") || ending == QLatin1String("docx")) return refuse(tr("The file is not a valid %1 file.").arg(ending));
        QString text;
        QString why;
        if (!decodeOutlineText(bytes, &text, &why)) return refuse(why);
        readText(text, ending == QLatin1String("md") || ending == QLatin1String("markdown"), folder, sink);
        if (out->slides.isEmpty()) return refuse(tr("The file has no text to make slides from."));
    }
    out->notes = notesOf(sink.counts, *out);
    return true;
}

bool readOutlineFile(const QString &path, Outline *out, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = tr("The file can't be opened.");
        return false;
    }
    // (one byte more than the most: a file past it is told so, and no more of it is read)
    const QByteArray bytes = f.read(qint64(96) << 20);
    if (!f.atEnd()) {
        if (error) *error = tr("The file is too large to be an outline.");
        return false;
    }
    return readOutline(bytes, path, QFileInfo(path).absolutePath(), out, error);
}

// ---- the slides

namespace {

TextItem *placeholderOf(Page &page, std::initializer_list<const char *> types)
{
    for (const auto &it : page.items) {
        if (it->type() != ItemType::Text) continue;
        const QString type = placeholderType(*it);
        for (const char *t : types)
            if (type == QLatin1String(t)) return static_cast<TextItem *>(it.get());
    }
    return nullptr;
}

QTextCharFormat runFormat(const QTextCharFormat &base, const OutlineRun &r, const QColor &link)
{
    QTextCharFormat cf = base;
    if (r.bold) cf.setFontWeight(QFont::Bold);
    if (r.italic) cf.setFontItalic(true);
    if (r.mono) cf.setFontFamilies({QStringLiteral("Courier New")});
    if (!r.href.isEmpty()) {
        cf.setAnchor(true);
        cf.setAnchorHref(r.href);
        if (link.isValid()) {
            cf.setForeground(QBrush(link));
            cf.setProperty(tp::ColorRefP, ColorRef::rgb(link).toString());
        }
        cf.setFontUnderline(true);
    }
    return cf;
}

// The story's paragraphs become `lines` (the first paragraph is the one it has; the others copy its format), the levels are set afterward.
void fillStory(QTextDocument *sd, const QVector<OutlineLine> &lines, const QColor &link)
{
    QTextCursor c(sd);
    const QTextBlockFormat bf = c.blockFormat();
    const QTextCharFormat base = c.blockCharFormat();
    for (int i = 0; i < lines.size(); ++i) {
        if (i) c.insertBlock(bf, base);
        for (const OutlineRun &r : lines[i].runs) c.insertText(r.text, runFormat(base, r, link));
    }
}

void setLevels(Document &d, const TextItem &t, const QVector<OutlineLine> &lines)
{
    for (int i = 0; i < lines.size();) {
        int j = i;
        while (j + 1 < lines.size() && lines[j + 1].level == lines[i].level) ++j;
        if (lines[i].level > 0 && !changeListLevel(d, t, i, j, lines[i].level)) {
            // (a layout with no list: the level is the paragraph's own, as the Outline view sets it)
            QTextDocument *sd = d.storyDoc(t.storyId);
            for (int k = i; k <= j; ++k) {
                QTextCursor c(sd->findBlockByNumber(k));
                QTextBlockFormat bf = c.blockFormat();
                bf.setLeftMargin(bf.leftMargin() + lines[i].level * 36.0);
                bf.setProperty(tp::ListLevel, lines[i].level);
                c.setBlockFormat(bf);
            }
        }
        i = j + 1;
    }
}

} // namespace

int insertOutline(Document &d, int at, const Outline &outline)
{
    const QColor link = d.colors.slot(Hyperlink);
    at = std::clamp(at, 0, int(d.pages.size()));
    int made = 0;
    // The slides first, then their empty places take their layout's bullets once (as the places of a file's slides do), then the text goes in.
    for (int k = 0; k < outline.slides.size(); ++k) {
        const int before = std::clamp(at + made - 1, 0, int(d.pages.size()) - 1);
        const QString master = d.pages.isEmpty() ? QString() : slideMasterOf(d, before);
        const MasterPage *layout = layoutOfType(d, QStringLiteral("obj"), master);
        if (!layout) layout = layoutOfType(d, QStringLiteral("obj"));
        if (!layout) break;
        addSlide(d, *layout, std::clamp(at + made, 0, int(d.pages.size())));
        ++made;
    }
    if (made > 0) inheritLayoutLists(d);
    for (int k = 0; k < made; ++k) {
        const OutlineSlide &s = outline.slides[k];
        const int index = at + k;
        Page &page = *d.pages[index];
        if (TextItem *t = placeholderOf(page, {"title", "ctrTitle"})) {
            if (!s.title.isEmpty()) {
                OutlineLine l;
                l.runs = s.title;
                fillStory(d.storyDoc(t->storyId), {l}, link);
            }
        }
        TextItem *body = placeholderOf(page, {"body", "obj", "subTitle"});
        const bool extras = !s.code.isEmpty() || !s.pictures.isEmpty();
        QRectF area;
        if (body) area = body->rect;
        if (body && !s.body.isEmpty()) {
            fillStory(d.storyDoc(body->storyId), s.body, link);
            setLevels(d, *body, s.body);
            if (extras) {
                body->rect = QRectF(area.left(), area.top(), area.width() * 0.52, area.height());
                area = QRectF(area.left() + area.width() * 0.55, area.top(), area.width() * 0.45, area.height());
            }
        } else if (body && extras) {
            // (nothing to say in the body place: the code and pictures take its room)
            for (int k = 0; k < page.items.size(); ++k)
                if (page.items[k].get() == body) {
                    page.items.erase(page.items.begin() + k);
                    break;
                }
        }
        if (extras) {
            const int cells = (s.code.isEmpty() ? 0 : 1) + int(s.pictures.size());
            const double gap = 8, cellH = (area.height() - gap * (cells - 1)) / cells;
            double y = area.top();
            if (!s.code.isEmpty()) {
                auto code = std::make_shared<TextItem>();
                code->rect = QRectF(area.left(), y, area.width(), cellH);
                code->name = QCoreApplication::translate("Deck", "Code");
                code->autofit = TextItem::ShrinkOnOverflow;
                code->hyphenate = false;
                code->storyId = d.createStory(s.code.join(QLatin1Char('\n')));
                QTextCursor c(d.storyDoc(code->storyId));
                c.select(QTextCursor::Document);
                QTextCharFormat cf;
                cf.setFontFamilies({QStringLiteral("Courier New")});
                cf.setFontPointSize(16);
                c.mergeCharFormat(cf);
                page.items.push_back(code);
                y += cellH + gap;
            }
            for (const OutlinePicture &p : s.pictures) {
                const QString imageId = d.addImage(p.bytes, p.format, p.path);
                if (!imageId.isEmpty()) {
                    QSizeF size(p.pixels.width() * 0.75, p.pixels.height() * 0.75);
                    if (size.isEmpty()) size = QSizeF(144, 144);
                    size.scale(QSizeF(area.width(), cellH), Qt::KeepAspectRatio);
                    auto pic = std::make_shared<PictureItem>();
                    pic->rect = QRectF(area.left() + (area.width() - size.width()) / 2, y + (cellH - size.height()) / 2, size.width(), size.height());
                    pic->imgRect = QRectF(QPointF(0, 0), size);
                    pic->imageId = imageId;
                    pic->altText = p.alt;
                    page.items.push_back(pic);
                }
                y += cellH + gap;
            }
        }
    }
    return made;
}

std::unique_ptr<Document> presentationFromOutline(const Outline &outline)
{
    std::unique_ptr<Document> d = newPresentation();
    insertOutline(*d, 1, outline);
    if (d->pages.size() > 1) d->pages.remove(0);   // (the Title Slide a new presentation starts with)
    return d;
}

// ---- the reverse

namespace {

QString escaped(const QString &s)
{
    QString out;
    for (const QChar c : s) {
        if (c == QLatin1Char('\\') || c == QLatin1Char('*') || c == QLatin1Char('_') || c == QLatin1Char('`') || c == QLatin1Char('[') || c == QLatin1Char(']') || c == QLatin1Char('<'))
            out += QLatin1Char('\\');
        out += c;
    }
    return out;
}

QString markedText(const QTextBlock &b, bool marks)
{
    QString out;
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (!f.isValid()) continue;
        const QTextCharFormat cf = f.charFormat();
        if (cf.hasProperty(tp::Field)) continue;
        QString t = f.text();
        t.replace(QChar::LineSeparator, QLatin1Char(' '));
        t.remove(QChar::ObjectReplacementCharacter);
        if (t.isEmpty()) continue;
        const bool bold = marks && cf.fontWeight() >= QFont::Bold, italic = marks && cf.fontItalic(), link = marks && cf.isAnchor() && !cf.anchorHref().isEmpty();
        QString core = t;
        QString before, after;
        if (bold || italic || link) {
            const QString lt = t.trimmed();
            const int start = int(t.indexOf(lt.isEmpty() ? t : lt));
            before = t.left(start);
            after = t.mid(start + lt.size());
            core = lt;
        }
        QString piece = escaped(core);
        if (link) piece = QStringLiteral("[") + piece + QStringLiteral("](") + cf.anchorHref() + QStringLiteral(")");
        if (italic) piece = QStringLiteral("*") + piece + QStringLiteral("*");
        if (bold) piece = QStringLiteral("**") + piece + QStringLiteral("**");
        out += before + piece + after;
    }
    return out;
}

int levelOf(const QTextBlock &b)
{
    const QTextBlockFormat bf = b.blockFormat();
    int level = bf.intProperty(tp::ListLevel);
    if (!bf.hasProperty(tp::ListLevel))
        if (const QTextList *list = b.textList()) level = list->format().intProperty(tp::ListLevel);
    return std::clamp(level, 0, kMaxOutlineLevel);
}

} // namespace

QString outlineMarkdown(const Document &d, QStringList *leftOut)
{
    QString out;
    bool objects = false, notes = false, other = false;
    for (const auto &page : d.pages) {
        const TextItem *title = nullptr, *body = nullptr;
        for (const auto &it : page->items) {
            if (it->type() != ItemType::Text) {
                objects = true;
                continue;
            }
            const QString type = placeholderType(*it);
            if (!title && (type == QLatin1String("title") || type == QLatin1String("ctrTitle"))) title = static_cast<const TextItem *>(it.get());
            else if (!body && (type == QLatin1String("body") || type == QLatin1String("obj") || type == QLatin1String("subTitle"))) body = static_cast<const TextItem *>(it.get());
            else if (type != QLatin1String("dt") && type != QLatin1String("ftr") && type != QLatin1String("sldNum")) other = other || !d.storyDoc(static_cast<const TextItem *>(it.get())->storyId)->isEmpty();
        }
        if (const QTextDocument *nd = d.storyDoc(page->notesStoryId)) notes = notes || !nd->isEmpty();
        QString heading;
        if (title)
            if (const QTextDocument *sd = d.storyDoc(title->storyId)) {
                QStringList lines;
                for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) lines << markedText(b, false);
                heading = lines.join(QLatin1Char(' ')).trimmed();
            }
        if (!out.isEmpty()) out += QLatin1Char('\n');
        out += QStringLiteral("#") + (heading.isEmpty() ? QString() : QLatin1Char(' ') + heading) + QLatin1Char('\n');
        if (body)
            if (const QTextDocument *sd = d.storyDoc(body->storyId))
                for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
                    const QString line = markedText(b, true).trimmed();
                    if (line.isEmpty()) continue;
                    out += QString(levelOf(b) * 2, QLatin1Char(' ')) + QStringLiteral("- ") + line + QLatin1Char('\n');
                }
    }
    if (leftOut) {
        leftOut->clear();
        if (objects) *leftOut << tr("pictures, shapes, tables, charts, and other objects on the slides");
        if (other) *leftOut << tr("the text of text boxes other than the title and the first body place");
        if (notes) *leftOut << tr("speaker notes");
        *leftOut << tr("the look of the slides (layouts, themes, colors, fonts, and backgrounds), animations, and transitions");
    }
    return out;
}

} // namespace jp::deck
