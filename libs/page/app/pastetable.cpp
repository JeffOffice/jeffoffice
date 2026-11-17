#include "app/pastetable.h"

#include "core/document.h"
#include "text/textprops.h"

#include <QFontMetricsF>
#include <QHash>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextTable>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

double columnWidthOf(QStringView tag);

// The value of a span attribute: 1 when it is missing or not a number; far past any bound (and `*odd` set) when it is spelled so that this reader and the page parser might read it differently.
int spanValue(QStringView v, bool *odd)
{
    v = v.trimmed();
    if (v.contains(QLatin1Char('&'))) {
        *odd = true;
        return 1000000000;
    }
    if (v.startsWith(QLatin1Char('+'))) v = v.mid(1);
    qint64 n = 0;
    int digits = 0;
    for (QChar ch : v) {
        if (!ch.isDigit()) break;
        if (digits++ < 9) n = n * 10 + ch.digitValue();
    }
    if (digits == 0) return 1;
    return digits >= 9 ? 1000000000 : int(n);
}

constexpr qint64 kMostWorkArea = 1000000;   // cells of grid (rows times columns) a page may ask the parser for
constexpr int kMostScanColumns = 10000;
constexpr qint64 kMostPasteScanChars = 32 * 1024 * 1024;

// What one pass over the tags of a page finds, before the page is parsed. Quotes in attribute values, comments, CDATA, and script and style text are skipped as a page parser skips them, so
// that nothing hides a tag. Each character is looked at once (the work is the length of the page), and the rows and columns are counted with the spans carried down from earlier rows.
struct Scan {
    int tables = 0, rows = 0, extentRows = 0, cols = 0;
    qint64 cells = 0;
    bool outside = false;       // words, pictures, or lists outside the table
    bool spanOver = false;      // a cell spans past a table's bounds (not cells: a page laid out with a table)
    bool hostile = false;       // would make the parser build a grid past kMostWorkArea, or read oddly
    bool big = false;           // past the bounds of rows, columns, or cells (the rest is not counted)
    QVector<double> colWidths;
    qint64 area() const { return qint64(std::max(rows, extentRows)) * cols; }
};

bool isBlank(QStringView t)
{
    QString text = t.toString();
    if (text.contains(QLatin1Char('&'))) text.remove(QLatin1String("&nbsp;"));
    return text.trimmed().isEmpty();
}

Scan scanTags(const QString &html)
{
    Scan s;
    static const QStringList outsideTags = {QStringLiteral("img"), QStringLiteral("hr"), QStringLiteral("h1"), QStringLiteral("h2"), QStringLiteral("h3"), QStringLiteral("h4"), QStringLiteral("h5"), QStringLiteral("h6"),
                                            QStringLiteral("ul"), QStringLiteral("ol"), QStringLiteral("pre"), QStringLiteral("form"), QStringLiteral("iframe"), QStringLiteral("object"),
                                            QStringLiteral("embed"), QStringLiteral("video"), QStringLiteral("svg"), QStringLiteral("canvas"), QStringLiteral("blockquote")};
    const QStringView all(html);
    const qsizetype n = html.size();
    qsizetype i = 0;
    int depth = 0, col = 0;
    QVector<int> carry;
    auto newRow = [&] {
        if (s.big) return;
        if (++s.rows > kPasteMaxRows) {
            s.big = true;
            return;
        }
        for (int &c : carry)
            if (c > 0) --c;
        col = 0;
    };
    while (i < n) {
        const qsizetype lt = html.indexOf(QLatin1Char('<'), i);
        const QStringView text = lt < 0 ? all.mid(i) : all.mid(i, lt - i);
        if (depth == 0 && !isBlank(text)) s.outside = true;
        if (lt < 0) break;
        i = lt;
        if (all.mid(i).startsWith(QLatin1String("<!--"))) {
            const qsizetype e = html.indexOf(QLatin1String("-->"), i + 4);
            if (e < 0) break;
            i = e + 3;
            continue;
        }
        if (all.mid(i).startsWith(QLatin1String("<![CDATA["))) {
            const qsizetype e = html.indexOf(QLatin1String("]]>"), i + 9);
            if (e < 0) break;
            i = e + 3;
            continue;
        }
        if (i + 1 >= n) break;
        if (html[i + 1] == QLatin1Char('!') || html[i + 1] == QLatin1Char('?')) {
            const qsizetype e = html.indexOf(QLatin1Char('>'), i);
            if (e < 0) break;
            i = e + 1;
            continue;
        }
        const bool closing = html[i + 1] == QLatin1Char('/');
        qsizetype j = i + 1 + (closing ? 1 : 0);
        const qsizetype nameStart = j;
        while (j < n && html[j].isLetterOrNumber()) ++j;
        if (j == nameStart) {
            ++i;   // (a less-than sign that is a word)
            continue;
        }
        const QString name = all.mid(nameStart, j - nameStart).toString().toLower();
        QStringView colspan, rowspan, span, width, style;
        // the attributes: names, then = and a value in quotes (of either kind) or bare
        qsizetype k = j;
        bool ended = false;
        while (true) {
            while (k < n && (html[k].isSpace() || html[k] == QLatin1Char('/'))) ++k;
            if (k >= n) {
                ended = true;
                break;
            }
            if (html[k] == QLatin1Char('>')) break;
            const qsizetype as = k;
            while (k < n && !html[k].isSpace() && html[k] != QLatin1Char('=') && html[k] != QLatin1Char('>') && html[k] != QLatin1Char('/')) ++k;
            const QString an = all.mid(as, k - as).toString().toLower();
            while (k < n && html[k].isSpace()) ++k;
            QStringView value;
            if (k < n && html[k] == QLatin1Char('=')) {
                ++k;
                while (k < n && html[k].isSpace()) ++k;
                if (k < n && (html[k] == QLatin1Char('"') || html[k] == QLatin1Char('\''))) {
                    const qsizetype e = html.indexOf(html[k], k + 1);
                    if (e < 0) {
                        s.hostile = true;   // (a quote never closed: the parser may take the rest of the page for the value)
                        ended = true;
                        break;
                    }
                    value = all.mid(k + 1, e - k - 1);
                    k = e + 1;
                } else {
                    const qsizetype vs = k;
                    while (k < n && !html[k].isSpace() && html[k] != QLatin1Char('>')) ++k;
                    value = all.mid(vs, k - vs);
                }
            } else if (k == as) {
                ++k;   // (a stray character: always moves on)
            }
            if (an == QLatin1String("colspan")) colspan = value;
            else if (an == QLatin1String("rowspan")) rowspan = value;
            else if (an == QLatin1String("span")) span = value;
            else if (an == QLatin1String("width")) width = value;
            else if (an == QLatin1String("style")) style = value;
        }
        if (ended) break;
        i = k + 1;
        if (closing) {
            if (name == QLatin1String("table")) depth = std::max(0, depth - 1);
            continue;
        }
        // (a script or a style is not skipped: the page parser reads cells inside them, so they are counted)
        if (name == QLatin1String("table")) {
            ++s.tables;
            ++depth;
        } else if (depth == 0 && outsideTags.contains(name)) {
            s.outside = true;
        } else if (name == QLatin1String("tr")) {
            newRow();
        } else if ((name == QLatin1String("td") || name == QLatin1String("th")) && !s.big) {
            if (s.rows == 0) newRow();
            if (s.big) continue;
            bool odd = false;
            const int cs = std::max(1, spanValue(colspan, &odd)), rs = std::max(1, spanValue(rowspan, &odd));
            if (odd) s.hostile = true;
            if (cs > kPasteMaxColumns || rs > kPasteMaxRows) s.spanOver = true;
            if (++s.cells > kPasteMaxCells) {
                s.big = true;
                continue;
            }
            while (col < carry.size() && carry[col] > 0) ++col;
            if (col + qint64(cs) > kMostScanColumns) {
                s.hostile = true;
                s.big = true;
                continue;
            }
            if (carry.size() < col + cs) carry.resize(col + cs, 0);
            for (int c = col; c < col + cs; ++c) carry[c] = rs;
            col += cs;
            s.cols = std::max(s.cols, col);
            s.extentRows = std::max(s.extentRows, s.rows - 1 + rs);
            if (s.area() > kMostWorkArea) {
                s.hostile = true;
                s.big = true;
            }
        } else if (name == QLatin1String("col")) {
            const int count = std::clamp(spanValue(span, &s.hostile), 1, kPasteMaxColumns);
            double w = 0;
            if (style.contains(QLatin1String("width"), Qt::CaseInsensitive)) w = columnWidthOf(style);
            if (w <= 0 && !width.isEmpty()) w = columnWidthOf(QStringLiteral("width=") + width.toString());
            for (int c = 0; c < count && s.colWidths.size() < kPasteMaxColumns; ++c) s.colWidths.push_back(w);
        }
    }
    return s;
}

// A column's width in points from a col tag (width="64", or width:48pt in its style), 0 when it names none.
double columnWidthOf(QStringView tag)
{
    qsizetype at = tag.indexOf(QLatin1String("width"), 0, Qt::CaseInsensitive);
    while (at >= 0) {
        if (at == 0 || (!tag[at - 1].isLetterOrNumber() && tag[at - 1] != QLatin1Char('-'))) {
            qsizetype i = at + 5;
            while (i < tag.size() && tag[i].isSpace()) ++i;
            if (i < tag.size() && (tag[i] == QLatin1Char('=') || tag[i] == QLatin1Char(':'))) {
                ++i;
                while (i < tag.size() && (tag[i].isSpace() || tag[i] == QLatin1Char('"') || tag[i] == QLatin1Char('\''))) ++i;
                double v = 0;
                int digits = 0, dot = -1;
                while (i < tag.size() && (tag[i].isDigit() || tag[i] == QLatin1Char('.')) && digits < 9) {
                    if (tag[i] == QLatin1Char('.')) {
                        if (dot >= 0) break;
                        dot = 0;
                    } else {
                        if (dot >= 0) v += tag[i].digitValue() / std::pow(10.0, ++dot);
                        else v = v * 10 + tag[i].digitValue();
                    }
                    ++digits;
                    ++i;
                }
                if (digits == 0) return 0;
                const QStringView unit = tag.mid(i, 2);
                if (unit.startsWith(QLatin1String("%"), Qt::CaseInsensitive)) return 0;
                double pt = v * 0.75;   // (pixels)
                if (unit.compare(QLatin1String("pt"), Qt::CaseInsensitive) == 0) pt = v;
                else if (unit.compare(QLatin1String("in"), Qt::CaseInsensitive) == 0) pt = v * 72;
                else if (unit.compare(QLatin1String("cm"), Qt::CaseInsensitive) == 0) pt = v * 72 / 2.54;
                else if (unit.compare(QLatin1String("mm"), Qt::CaseInsensitive) == 0) pt = v * 72 / 25.4;
                return std::clamp(pt, 0.0, 2000.0);
            }
        }
        at = tag.indexOf(QLatin1String("width"), at + 1, Qt::CaseInsensitive);
    }
    return 0;
}


QTextCharFormat runFormat(const QTextCharFormat &cf)
{
    QTextCharFormat out;
    if (cf.hasProperty(QTextFormat::FontFamilies)) {
        const QStringList fams = cf.fontFamilies().toStringList();
        if (!fams.isEmpty() && !fams.first().isEmpty()) out.setFontFamilies(QStringList{fams.first().left(120)});
    } else if (cf.hasProperty(QTextFormat::FontFamily) && !cf.fontFamily().isEmpty()) {
        out.setFontFamilies(QStringList{cf.fontFamily().left(120)});
    }
    if (cf.hasProperty(QTextFormat::FontPointSize) && cf.fontPointSize() > 0) out.setFontPointSize(std::clamp(cf.fontPointSize(), 1.0, 999.0));
    else if (cf.hasProperty(QTextFormat::FontPixelSize) && cf.font().pixelSize() > 0) out.setFontPointSize(std::clamp(cf.font().pixelSize() * 0.75, 1.0, 999.0));
    if (cf.hasProperty(QTextFormat::FontWeight)) out.setFontWeight(cf.fontWeight());
    if (cf.fontItalic()) out.setFontItalic(true);
    if (cf.fontStrikeOut()) out.setFontStrikeOut(true);
    if (cf.verticalAlignment() == QTextCharFormat::AlignSuperScript || cf.verticalAlignment() == QTextCharFormat::AlignSubScript) out.setVerticalAlignment(cf.verticalAlignment());
    if (!cf.isAnchor()) {
        if (cf.fontUnderline()) out.setFontUnderline(true);
        if (cf.foreground().style() == Qt::SolidPattern && cf.foreground().color().alpha() > 0) out.setProperty(tp::ColorRefP, cf.foreground().color().name());
    }
    return out;
}

Stroke borderOf(double px, QTextFrameFormat::BorderStyle style, const QBrush &brush)
{
    if (px <= 0 || style == QTextFrameFormat::BorderStyle_None) return Stroke::none();
    Stroke s = Stroke::line(ColorRef::rgb(brush.style() == Qt::NoBrush ? QColor(Qt::black) : brush.color()), std::clamp(px * 0.75, 0.25, 12.0));
    if (style == QTextFrameFormat::BorderStyle_Dotted) s.dash = Stroke::RoundDot;
    else if (style == QTextFrameFormat::BorderStyle_Dashed) s.dash = Stroke::DashLine;
    else if (style == QTextFrameFormat::BorderStyle_Double) s.compound = Stroke::Double;
    return s;
}

// What one cell holds, as paragraphs of runs; at most kPasteMaxCellChars characters of it.
void readCell(const QTextTableCell &tc, PastedCell *out, bool *truncated)
{
    int chars = 0;
    for (QTextFrame::iterator it = tc.begin(); !it.atEnd(); ++it) {
        const QTextBlock b = it.currentBlock();
        if (!b.isValid()) continue;
        PastedParagraph para;
        const Qt::Alignment al = b.blockFormat().alignment() & Qt::AlignHorizontal_Mask;
        para.align = (al & Qt::AlignRight) ? Qt::AlignRight : (al & Qt::AlignHCenter) ? Qt::AlignHCenter : (al & Qt::AlignJustify) ? Qt::AlignJustify : Qt::AlignLeft;
        for (QTextBlock::iterator f = b.begin(); !f.atEnd(); ++f) {
            const QTextFragment fr = f.fragment();
            QString text = fr.text();
            text.remove(QChar::ObjectReplacementCharacter);
            if (text.isEmpty()) continue;
            if (chars + text.size() > kPasteMaxCellChars) {
                text.truncate(kPasteMaxCellChars - chars);
                *truncated = true;
            }
            chars += text.size();
            para.runs.push_back({text, runFormat(fr.charFormat())});
            if (chars >= kPasteMaxCellChars) break;
        }
        // (blanks at the edges of the paragraph are the page's own layout)
        if (!para.runs.isEmpty()) {
            para.runs.first().text.remove(0, para.runs.first().text.indexOf(QRegularExpression(QStringLiteral("[^\\s]|$"))));
            QString &last = para.runs.last().text;
            while (!last.isEmpty() && last.back().isSpace()) last.chop(1);
        }
        out->paragraphs.push_back(para);
        if (chars >= kPasteMaxCellChars) break;
    }
    bool any = false;
    for (const PastedParagraph &p : std::as_const(out->paragraphs))
        for (const PastedRun &r : p.runs) any = any || !r.text.isEmpty();
    if (!any) out->paragraphs.clear();
    while (!out->paragraphs.isEmpty() && out->paragraphs.last().runs.isEmpty()) out->paragraphs.removeLast();
}

} // namespace

PastedTableResult readPastedTable(const QString &html, PastedTable *out)
{
    *out = PastedTable();
    if (html.isEmpty() || !html.contains(QLatin1String("<table"), Qt::CaseInsensitive)) return PastedTableResult::NotATable;
    // (one look at the tags, before any parse: a page that asks the parser for a huge grid is refused whatever it is)
    if (html.size() > kMostPasteScanChars) return PastedTableResult::TooBig;
    const Scan scan = scanTags(html);
    if (scan.hostile || scan.area() > kMostWorkArea) return PastedTableResult::TooBig;
    // Only a page that is one table and nothing else (what a copy of cells puts on the clipboard) is cells: any other page is pasted as it always was.
    if (scan.tables != 1 || scan.outside || scan.spanOver) return PastedTableResult::NotATable;
    if (scan.big || html.size() > kPasteMaxHtmlBytes || std::max(scan.rows, scan.extentRows) > kPasteMaxRows || scan.cols > kPasteMaxColumns || scan.cells > kPasteMaxCells) return PastedTableResult::TooBig;
    const QVector<double> colWidths = scan.colWidths;
    QTextDocument page;
    page.setUndoRedoEnabled(false);
    page.setHtml(html);
    QTextTable *only = nullptr;
    for (QTextFrame::iterator it = page.rootFrame()->begin(); !it.atEnd(); ++it) {
        if (auto *table = qobject_cast<QTextTable *>(it.currentFrame())) {
            if (only) return PastedTableResult::NotATable;
            only = table;
        } else if (it.currentBlock().isValid() && !it.currentBlock().text().trimmed().isEmpty()) {
            return PastedTableResult::NotATable;
        }
    }
    if (!only || only->rows() <= 0 || only->columns() <= 0) return PastedTableResult::NotATable;
    const int rows = only->rows(), cols = only->columns();
    if (rows > kPasteMaxRows || cols > kPasteMaxColumns || qint64(rows) * cols > kPasteMaxCells) return PastedTableResult::TooBig;
    if (qint64(rows) * cols < 2) return PastedTableResult::NotATable;
    out->rows = rows;
    out->cols = cols;
    out->cells.resize(rows * cols);
    out->colWidths = QVector<double>(cols, 0.0);
    const QVector<QTextLength> cons = only->format().columnWidthConstraints();
    for (int c = 0; c < cols; ++c) {
        if (c < cons.size() && cons[c].type() == QTextLength::FixedLength && cons[c].rawValue() > 0) out->colWidths[c] = std::min(cons[c].rawValue() * 0.75, 2000.0);
        else if (c < colWidths.size()) out->colWidths[c] = colWidths[c];
    }
    // (a table with a border of its own, whose cells say nothing, has thin lines all round each)
    const bool tableBorder = only->format().border() > 0;
    const QColor tableBorderColor = only->format().borderBrush().style() == Qt::NoBrush ? QColor(Qt::black) : only->format().borderBrush().color();
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            const QTextTableCell tc = only->cellAt(r, c);
            PastedCell &pc = out->cell(r, c);
            if (!tc.isValid() || tc.row() != r || tc.column() != c) continue;
            pc.origin = true;
            pc.rowSpan = std::clamp(tc.rowSpan(), 1, rows - r);
            pc.colSpan = std::clamp(tc.columnSpan(), 1, cols - c);
            const QTextTableCellFormat f = tc.format().toTableCellFormat();
            if (f.background().style() == Qt::SolidPattern && f.background().color().alpha() > 0) {
                pc.hasFill = true;
                pc.fill = f.background().color();
                pc.fill.setAlpha(255);
            }
            pc.border.left = borderOf(f.leftBorder(), f.leftBorderStyle(), f.leftBorderBrush());
            pc.border.right = borderOf(f.rightBorder(), f.rightBorderStyle(), f.rightBorderBrush());
            pc.border.top = borderOf(f.topBorder(), f.topBorderStyle(), f.topBorderBrush());
            pc.border.bottom = borderOf(f.bottomBorder(), f.bottomBorderStyle(), f.bottomBorderBrush());
            if (tableBorder && pc.border.left.isNone() && pc.border.right.isNone() && pc.border.top.isNone() && pc.border.bottom.isNone())
                pc.border.left = pc.border.right = pc.border.top = pc.border.bottom = Stroke::line(ColorRef::rgb(tableBorderColor), 0.75);
            const QTextCharFormat::VerticalAlignment va = f.verticalAlignment();
            if (va == QTextCharFormat::AlignMiddle || va == QTextCharFormat::AlignBottom || va == QTextCharFormat::AlignTop) {
                pc.hasValign = true;
                pc.valign = va == QTextCharFormat::AlignMiddle ? VAlign::Middle : va == QTextCharFormat::AlignBottom ? VAlign::Bottom : VAlign::Top;
            }
            readCell(tc, &pc, &out->truncated);
        }
    return PastedTableResult::Ok;
}

void writePastedCell(Document *doc, TableCell &target, const PastedCell &source)
{
    // (a paste replaces what the cell had: its fill, borders, and the way its text sits)
    target.fill = source.hasFill ? Fill::solid(ColorRef::rgb(source.fill)) : Fill::none();
    target.border.left = source.border.left;
    target.border.right = source.border.right;
    target.border.top = source.border.top;
    target.border.bottom = source.border.bottom;
    target.valign = source.hasValign ? source.valign : VAlign::Top;
    QTextDocument *story = doc->storyDoc(target.storyId);
    if (!story) return;
    if (source.paragraphs.isEmpty() && story->isEmpty()) return;
    QTextCursor cur(story);
    const QTextCharFormat baseChar = cur.blockCharFormat();   // (every run and paragraph starts from the cell's own look, not from the one before)
    const QTextBlockFormat baseBlock = cur.blockFormat();
    cur.select(QTextCursor::Document);
    cur.removeSelectedText();
    bool first = true;
    for (const PastedParagraph &p : source.paragraphs) {
        if (!first) cur.insertBlock();
        first = false;
        QTextBlockFormat bf = baseBlock;
        bf.setAlignment(p.align);
        cur.setBlockFormat(bf);
        cur.setBlockCharFormat(baseChar);
        for (const PastedRun &run : p.runs) {
            QTextCharFormat f = baseChar;
            f.merge(run.format);
            cur.insertText(run.text, f);
        }
    }
}

std::shared_ptr<TableItem> makePastedTable(Document *doc, const PastedTable &pasted, const QSizeF &page)
{
    auto t = std::make_shared<TableItem>();
    t->rows = pasted.rows;
    t->cols = pasted.cols;
    t->format = QStringLiteral("None");
    t->header = false;
    t->banded = false;
    t->cells.resize(t->rows * t->cols);
    for (TableCell &c : t->cells) c.storyId = doc->createStory();
    QVector<double> natural(t->cols, 0.0), rowH(t->rows, 18.0);
    QFont base;
    QHash<QString, QFontMetricsF> metrics;   // (one for each look the text has, not one for each cell)
    if (!t->cells.isEmpty())
        if (QTextDocument *d = doc->storyDoc(t->cells.first().storyId)) base = d->defaultFont();
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < t->cols; ++c) {
            const PastedCell &pc = pasted.cell(r, c);
            TableCell &cell = t->cell(r, c);
            if (!pc.origin) {
                cell.covered = true;
                continue;
            }
            cell.rowSpan = pc.rowSpan;
            cell.colSpan = pc.colSpan;
            writePastedCell(doc, cell, pc);
            // What the text needs, from the font of its first run in each line.
            double wide = 0, high = 0;
            for (const PastedParagraph &p : pc.paragraphs) {
                QFont f = base;
                if (!p.runs.isEmpty()) {
                    const QTextCharFormat &rf = p.runs.first().format;
                    if (rf.hasProperty(QTextFormat::FontFamilies)) f.setFamilies(rf.fontFamilies().toStringList());
                    if (rf.hasProperty(QTextFormat::FontPointSize)) f.setPointSizeF(rf.fontPointSize());
                    if (rf.hasProperty(QTextFormat::FontWeight)) f.setWeight(QFont::Weight(rf.fontWeight()));
                }
                QString line;
                for (const PastedRun &run : p.runs) line += run.text;
                const QString key = f.families().join(QLatin1Char('|')) + QLatin1Char('/') + QString::number(f.pointSizeF()) + QLatin1Char('/') + QString::number(int(f.weight()));
                auto found = metrics.find(key);
                if (found == metrics.end()) found = metrics.insert(key, QFontMetricsF(f));
                const QFontMetricsF &fm = found.value();
                for (const QString &part : line.split(QChar(QChar::LineSeparator))) {
                    wide = std::max(wide, fm.horizontalAdvance(part));
                    high += fm.lineSpacing();
                }
            }
            const double w = std::min(wide + 2 * cell.margins.left() + 2, 300.0);
            if (pc.colSpan == 1) natural[c] = std::max(natural[c], w);
            if (pc.rowSpan == 1) rowH[r] = std::max(rowH[r], high + cell.margins.top() + cell.margins.bottom());
        }
    // A line between two cells is on both: a cell that has a border its neighbor lacks gives the neighbor the same on the shared edge (a file keeps one line for an edge).
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < t->cols; ++c) {
            const TableCell cell = t->cell(r, c);
            if (cell.covered) continue;
            const int r2 = r + cell.rowSpan, c2 = c + cell.colSpan;
            for (int k = c; k < c2 && r2 < t->rows; ++k)
                if (TableCell &n = t->cell(r2, k); !n.covered && n.border.top.isNone()) n.border.top = cell.border.bottom;
            for (int k = r; k < r2 && c2 < t->cols; ++k)
                if (TableCell &n = t->cell(k, c2); !n.covered && n.border.left.isNone()) n.border.left = cell.border.right;
            for (int k = c; k < c2 && r > 0; ++k)
                if (TableCell &n = t->cell(r - 1, k); !n.covered && n.border.bottom.isNone()) n.border.bottom = cell.border.top;
            for (int k = r; k < r2 && c > 0; ++k)
                if (TableCell &n = t->cell(k, c - 1); !n.covered && n.border.right.isNone()) n.border.right = cell.border.left;
        }
    for (int c = 0; c < t->cols; ++c) {
        const double given = c < pasted.colWidths.size() ? pasted.colWidths[c] : 0;
        t->colW << (given > 0 ? given : std::max(30.0, natural[c]));
    }
    double total = 0;
    for (double w : std::as_const(t->colW)) total += w;
    const double most = std::max(36.0, page.width() * 0.8);
    if (total > most)
        for (double &w : t->colW) w *= most / total;
    // (a row is never taller than the page, and the table is at most the page's height)
    double high = 0;
    for (double &h : rowH) high += (h = std::min(h, page.height() * 0.9));
    if (high > page.height() * 0.9)
        for (double &h : rowH) h *= page.height() * 0.9 / high;
    t->rowH = rowH;
    t->syncRect();
    const double x = std::max(0.0, (page.width() - t->rect.width()) / 2), y = page.height() * 0.1;
    t->rect.moveTo(x, y);
    return t;
}

} // namespace jp
