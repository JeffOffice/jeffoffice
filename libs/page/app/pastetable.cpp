#include "app/pastetable.h"

#include "core/document.h"
#include "text/textprops.h"

#include <QFontMetricsF>
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

// The whole number after name= in a tag's text (1 when there is none); a long run of digits is far past any bound.
int attributeNumber(QStringView tag, QLatin1String name)
{
    qsizetype at = tag.indexOf(name, 0, Qt::CaseInsensitive);
    while (at >= 0 && at > 0 && (tag[at - 1].isLetterOrNumber() || tag[at - 1] == QLatin1Char('-'))) at = tag.indexOf(name, at + 1, Qt::CaseInsensitive);
    if (at < 0) return 1;
    qsizetype i = at + name.size();
    while (i < tag.size() && (tag[i].isSpace() || tag[i] == QLatin1Char('='))) ++i;
    while (i < tag.size() && (tag[i] == QLatin1Char('"') || tag[i] == QLatin1Char('\''))) ++i;
    qint64 v = 0;
    int digits = 0;
    while (i < tag.size() && tag[i].isDigit()) {
        if (digits < 9) v = v * 10 + tag[i].digitValue();
        ++digits;
        ++i;
    }
    if (digits == 0) return 1;
    return digits >= 9 ? 1000000000 : int(v);
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

enum class Look { Ok, NotOne, Big };

// Looks at the tags of the page once, without parsing it, for what a hostile page would multiply: how many tables, rows, and cells, and how far a cell spans. Each tag is looked at once, so the
// work is the length of the page. Fills the widths the col tags name.
Look scanTags(const QString &html, QVector<double> *colWidths)
{
    int tables = 0, rows = 0, cells = 0, rowColumns = 0;
    const qsizetype n = html.size();
    qsizetype i = 0;
    while (i < n) {
        i = html.indexOf(QLatin1Char('<'), i);
        if (i < 0) break;
        qsizetype j = i + 1;
        while (j < n && html[j].isLetterOrNumber()) ++j;
        const QStringView name = QStringView(html).mid(i + 1, j - i - 1);
        const qsizetype gt = html.indexOf(QLatin1Char('>'), j);
        if (gt < 0) break;
        const QStringView tag = QStringView(html).mid(j, gt - j);
        if (name.compare(QLatin1String("table"), Qt::CaseInsensitive) == 0) {
            if (++tables > 1) return Look::NotOne;
        } else if (name.compare(QLatin1String("tr"), Qt::CaseInsensitive) == 0) {
            if (++rows > kPasteMaxRows) return Look::Big;
            rowColumns = 0;
        } else if (name.compare(QLatin1String("td"), Qt::CaseInsensitive) == 0 || name.compare(QLatin1String("th"), Qt::CaseInsensitive) == 0) {
            if (++cells > kPasteMaxCells) return Look::Big;
            const int cs = attributeNumber(tag, QLatin1String("colspan")), rs = attributeNumber(tag, QLatin1String("rowspan"));
            if (cs > kPasteMaxColumns || rs > kPasteMaxRows) return Look::Big;
            rowColumns += std::max(1, cs);
            if (rowColumns > kPasteMaxColumns) return Look::Big;
        } else if (name.compare(QLatin1String("col"), Qt::CaseInsensitive) == 0 && colWidths) {
            const int span = std::clamp(attributeNumber(tag, QLatin1String("span")), 1, kPasteMaxColumns);
            const double w = columnWidthOf(tag);
            for (int k = 0; k < span && colWidths->size() < kPasteMaxColumns; ++k) colWidths->push_back(w);
        }
        i = gt + 1;
    }
    return Look::Ok;
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
    if (html.size() > kPasteMaxHtmlBytes) return PastedTableResult::TooBig;
    QVector<double> colWidths;
    switch (scanTags(html, &colWidths)) {
    case Look::NotOne: return PastedTableResult::NotATable;
    case Look::Big: return PastedTableResult::TooBig;
    case Look::Ok: break;
    }
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
    if (source.hasFill) target.fill = Fill::solid(ColorRef::rgb(source.fill));
    if (!source.border.left.isNone()) target.border.left = source.border.left;
    if (!source.border.right.isNone()) target.border.right = source.border.right;
    if (!source.border.top.isNone()) target.border.top = source.border.top;
    if (!source.border.bottom.isNone()) target.border.bottom = source.border.bottom;
    if (source.hasValign) target.valign = source.valign;
    QTextDocument *story = doc->storyDoc(target.storyId);
    if (!story) return;
    QTextCursor cur(story);
    cur.select(QTextCursor::Document);
    cur.removeSelectedText();
    bool first = true;
    for (const PastedParagraph &p : source.paragraphs) {
        if (!first) cur.insertBlock();
        first = false;
        if (p.align != Qt::AlignLeft) {
            QTextBlockFormat bf = cur.blockFormat();
            bf.setAlignment(p.align);
            cur.setBlockFormat(bf);
        }
        for (const PastedRun &run : p.runs) {
            QTextCharFormat f = cur.charFormat();
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
                const QFontMetricsF fm(f);
                for (const QString &part : line.split(QChar(QChar::LineSeparator))) {
                    wide = std::max(wide, fm.horizontalAdvance(part));
                    high += fm.lineSpacing();
                }
            }
            const double w = std::min(wide + 2 * cell.margins.left() + 2, 300.0);
            if (pc.colSpan == 1) natural[c] = std::max(natural[c], w);
            if (pc.rowSpan == 1) rowH[r] = std::max(rowH[r], high + cell.margins.top() + cell.margins.bottom());
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
    t->rowH = rowH;
    t->syncRect();
    const double x = std::max(0.0, (page.width() - t->rect.width()) / 2), y = page.height() * 0.1;
    t->rect.moveTo(x, y);
    return t;
}

} // namespace jp
