#include "sheet/pagesetup.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QXmlStreamAttributes>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

bool PageSetup::operator==(const PageSetup &o) const
{
    return paperSize == o.paperSize && paperWidth == o.paperWidth && paperHeight == o.paperHeight && orientation == o.orientation && scale == o.scale && fitToWidth == o.fitToWidth
           && fitToHeight == o.fitToHeight && fitToPage == o.fitToPage && firstPageNumber == o.firstPageNumber && useFirstPageNumber == o.useFirstPageNumber && pageOrder == o.pageOrder
           && blackAndWhite == o.blackAndWhite && draft == o.draft && comments == o.comments && errors == o.errors && margins == o.margins && gridLines == o.gridLines
           && headings == o.headings && centerHorizontally == o.centerHorizontally && centerVertically == o.centerVertically && headerFooter == o.headerFooter
           && rowBreaks == o.rowBreaks && colBreaks == o.colBreaks && printArea == o.printArea && titles == o.titles;
}

namespace {

// An attribute's value, or null when the element has none by that name.
QStringView attr(const QXmlStreamAttributes &a, const char *name) { return a.value(QLatin1String(name)); }

bool boolOf(QStringView v, bool *value)
{
    const QStringView t = v.trimmed();
    if (t == QLatin1String("1") || t.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0) {
        *value = true;
        return true;
    }
    if (t == QLatin1String("0") || t.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0) {
        *value = false;
        return true;
    }
    return false;
}

void setBool(const QXmlStreamAttributes &a, const char *name, bool *into)
{
    bool value = false;
    if (boolOf(attr(a, name), &value)) *into = value;
}

// A whole number held between two ends; false (and `into` as it was) when the text is not one.
bool setInt(const QXmlStreamAttributes &a, const char *name, int low, int high, int *into, bool clamp = true)
{
    const QStringView v = attr(a, name).trimmed();
    if (v.isEmpty()) return false;
    bool ok = false;
    const qlonglong n = v.toLongLong(&ok);
    if (!ok) return false;
    if (!clamp && (n < low || n > high)) return false;
    *into = int(std::clamp<qlonglong>(n, low, high));
    return true;
}

void setMargin(const QXmlStreamAttributes &a, const char *name, double *into)
{
    const QStringView v = attr(a, name).trimmed();
    if (v.isEmpty()) return;
    bool ok = false;
    const double d = v.toDouble(&ok);
    if (!ok || !std::isfinite(d)) return;
    *into = std::clamp(d, 0.0, kMaxMarginInches);
}

// A length with its unit ("210mm", "8.5in") in points; false for anything else or for a length that is no paper's.
bool lengthPoints(QStringView text, double *points)
{
    static const QRegularExpression pattern(QStringLiteral("^\\s*([0-9]+(?:\\.[0-9]+)?)\\s*(mm|cm|in|pt|pc|pi)\\s*$"));
    const QRegularExpressionMatch m = pattern.match(text);
    if (!m.hasMatch()) return false;
    const double v = m.captured(1).toDouble();
    const QString unit = m.captured(2);
    double pt = v;
    if (unit == QLatin1String("mm")) pt = v / 25.4 * 72;
    else if (unit == QLatin1String("cm")) pt = v / 2.54 * 72;
    else if (unit == QLatin1String("in")) pt = v * 72;
    else if (unit == QLatin1String("pc") || unit == QLatin1String("pi")) pt = v * 12;
    if (!std::isfinite(pt) || pt < 72 || pt > 14400) return false;
    *points = pt;
    return true;
}

} // namespace

void readPageSetupAttributes(const QXmlStreamAttributes &a, PageSetup *out)
{
    setInt(a, "paperSize", 0, 32767, &out->paperSize, false);
    double pt = 0;
    if (lengthPoints(attr(a, "paperWidth"), &pt)) out->paperWidth = pt;
    if (lengthPoints(attr(a, "paperHeight"), &pt)) out->paperHeight = pt;
    setInt(a, "scale", kMinPageScale, kMaxPageScale, &out->scale);
    setInt(a, "firstPageNumber", 0, 32767, &out->firstPageNumber, false);
    setInt(a, "fitToWidth", 0, kMaxFitPages, &out->fitToWidth);
    setInt(a, "fitToHeight", 0, kMaxFitPages, &out->fitToHeight);
    const QStringView order = attr(a, "pageOrder");
    if (order == QLatin1String("overThenDown")) out->pageOrder = PageOrder::OverThenDown;
    else if (order == QLatin1String("downThenOver")) out->pageOrder = PageOrder::DownThenOver;
    const QStringView orientation = attr(a, "orientation");
    if (orientation == QLatin1String("landscape")) out->orientation = PageOrientation::Landscape;
    else if (orientation == QLatin1String("portrait")) out->orientation = PageOrientation::Portrait;
    else if (orientation == QLatin1String("default")) out->orientation = PageOrientation::Default;
    setBool(a, "blackAndWhite", &out->blackAndWhite);
    setBool(a, "draft", &out->draft);
    setBool(a, "useFirstPageNumber", &out->useFirstPageNumber);
    const QStringView comments = attr(a, "cellComments");
    if (comments == QLatin1String("atEnd")) out->comments = PrintComments::AtEnd;
    else if (comments == QLatin1String("asDisplayed")) out->comments = PrintComments::AsDisplayed;
    else if (comments == QLatin1String("none")) out->comments = PrintComments::None;
    const QStringView errors = attr(a, "errors");
    if (errors == QLatin1String("blank")) out->errors = PrintErrors::Blank;
    else if (errors == QLatin1String("dash")) out->errors = PrintErrors::Dash;
    else if (errors == QLatin1String("NA")) out->errors = PrintErrors::NA;
    else if (errors == QLatin1String("displayed")) out->errors = PrintErrors::Displayed;
}

void readPageMarginsAttributes(const QXmlStreamAttributes &a, PageSetup *out)
{
    setMargin(a, "left", &out->margins.left);
    setMargin(a, "right", &out->margins.right);
    setMargin(a, "top", &out->margins.top);
    setMargin(a, "bottom", &out->margins.bottom);
    setMargin(a, "header", &out->margins.header);
    setMargin(a, "footer", &out->margins.footer);
}

void readPrintOptionsAttributes(const QXmlStreamAttributes &a, PageSetup *out)
{
    setBool(a, "horizontalCentered", &out->centerHorizontally);
    setBool(a, "verticalCentered", &out->centerVertically);
    setBool(a, "headings", &out->headings);
    setBool(a, "gridLines", &out->gridLines);
}

void readHeaderFooterAttributes(const QXmlStreamAttributes &a, PageSetup *out)
{
    setBool(a, "differentOddEven", &out->headerFooter.differentOddEven);
    setBool(a, "differentFirst", &out->headerFooter.differentFirst);
    setBool(a, "scaleWithDoc", &out->headerFooter.scaleWithDoc);
    setBool(a, "alignWithMargins", &out->headerFooter.alignWithMargins);
}

void readBreakAttributes(const QXmlStreamAttributes &a, bool rows, PageSetup *out)
{
    QVector<PageBreak> &list = rows ? out->rowBreaks : out->colBreaks;
    int &looked = rows ? out->rowBreakElements : out->colBreakElements;
    if (list.size() >= kMaxPageBreaks || looked >= kMaxBreakElements) return;
    ++looked;
    const QStringView idText = attr(a, "id").trimmed();
    bool ok = false;
    const qulonglong id = idText.toULongLong(&ok);
    // a new page begins at the row (column) of that number counted from 0, so it must be a row (column) the sheet has
    if (!ok || id < 1 || id >= (rows ? qulonglong(kMaxRows) : qulonglong(kMaxColumns))) return;
    for (const PageBreak &b : std::as_const(list))
        if (b.before == quint32(id)) return;
    PageBreak b;
    b.before = quint32(id);
    int v = 0;
    b.min = setInt(a, "min", 0, 1048575, &v) ? quint32(v) : 0;
    b.max = setInt(a, "max", 0, 1048575, &v) ? quint32(v) : 0;
    b.manual = false;
    setBool(a, "man", &b.manual);
    list.append(b);
}

void readFitToPageAttributes(const QXmlStreamAttributes &a, PageSetup *out) { setBool(a, "fitToPage", &out->fitToPage); }

QString *headerFooterText(PageSetup *out, QStringView name)
{
    HeaderFooter &h = out->headerFooter;
    if (name == QLatin1String("oddHeader")) return &h.oddHeader;
    if (name == QLatin1String("oddFooter")) return &h.oddFooter;
    if (name == QLatin1String("evenHeader")) return &h.evenHeader;
    if (name == QLatin1String("evenFooter")) return &h.evenFooter;
    if (name == QLatin1String("firstHeader")) return &h.firstHeader;
    if (name == QLatin1String("firstFooter")) return &h.firstFooter;
    return nullptr;
}

// ---- the names

namespace {

// The pieces of a name's reference text, split at the commas that are not inside a quoted sheet name.
QStringList splitReferences(const QString &text)
{
    QStringList parts;
    QString current;
    bool quoted = false;
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        if (c == QLatin1Char('\'')) {
            // ('' inside a quoted name is a quote)
            if (quoted && i + 1 < text.size() && text[i + 1] == QLatin1Char('\'')) {
                current += c;
                current += c;
                ++i;
                continue;
            }
            quoted = !quoted;
        }
        if (c == QLatin1Char(',') && !quoted) {
            parts << current;
            current.clear();
            continue;
        }
        current += c;
    }
    parts << current;
    return parts;
}

// Splits "Sheet1!$A$1" or "'My Sheet'!A:B" into the sheet's name and the rest; no sheet name gives an empty one with `hasSheet` false.
void splitSheet(const QString &piece, QString *sheet, QString *rest, bool *hasSheet)
{
    const QString t = piece.trimmed();
    *hasSheet = false;
    *rest = t;
    if (t.isEmpty()) return;
    if (t[0] == QLatin1Char('\'')) {
        QString name;
        int i = 1;
        for (; i < t.size(); ++i) {
            if (t[i] == QLatin1Char('\'')) {
                if (i + 1 < t.size() && t[i + 1] == QLatin1Char('\'')) {
                    name += QLatin1Char('\'');
                    ++i;
                    continue;
                }
                break;
            }
            name += t[i];
        }
        if (i + 1 < t.size() && t[i + 1] == QLatin1Char('!')) {
            *sheet = name;
            *rest = t.mid(i + 2);
            *hasSheet = true;
        }
        return;
    }
    const int bang = t.indexOf(QLatin1Char('!'));
    if (bang > 0) {
        *sheet = t.left(bang);
        *rest = t.mid(bang + 1);
        *hasSheet = true;
    }
}

bool allLetters(QStringView s)
{
    if (s.isEmpty() || s.size() > 3) return false;
    for (QChar c : s)
        if (!c.isLetter() || c.unicode() > 127) return false;
    return true;
}

bool allDigits(QStringView s)
{
    if (s.isEmpty() || s.size() > 7) return false;
    for (QChar c : s)
        if (!c.isDigit() || c.unicode() > 127) return false;
    return true;
}

enum class BlockKind { Cells, Rows, Columns };

// "$A$1:$D$20", "C4", "$A:$B", "$5:$6": a block of the sheet; false for anything else.
bool parseBlock(const QString &text, CellRange *range, BlockKind *kind)
{
    QString t = text.trimmed();
    t.remove(QLatin1Char('$'));
    if (t.isEmpty()) return false;
    const int colon = t.indexOf(QLatin1Char(':'));
    if (colon > 0) {
        const QStringView a = QStringView(t).left(colon), b = QStringView(t).mid(colon + 1);
        quint32 c1 = 0, c2 = 0, r = 0;
        if (allLetters(a) && allLetters(b) && parseCellRef(a.toString() + QLatin1Char('1'), &r, &c1) && parseCellRef(b.toString() + QLatin1Char('1'), &r, &c2)) {
            range->row1 = 0;
            range->row2 = kMaxRows - 1;
            range->col1 = std::min(c1, c2);
            range->col2 = std::max(c1, c2);
            range->valid = true;
            *kind = BlockKind::Columns;
            return true;
        }
        if (allDigits(a) && allDigits(b)) {
            const qint64 r1 = a.toLongLong() - 1, r2 = b.toLongLong() - 1;
            if (r1 < 0 || r2 < 0 || r1 >= kMaxRows || r2 >= kMaxRows) return false;
            range->row1 = quint32(std::min(r1, r2));
            range->row2 = quint32(std::max(r1, r2));
            range->col1 = 0;
            range->col2 = kMaxColumns - 1;
            range->valid = true;
            *kind = BlockKind::Rows;
            return true;
        }
    }
    if (!parseRangeRef(t, range)) return false;
    *kind = BlockKind::Cells;
    return true;
}

// The pieces of a name's text that are on the sheet.
struct OnSheet {
    CellRange range;
    BlockKind kind = BlockKind::Cells;
};

QVector<OnSheet> blocksOn(const QString &text, const QString &sheetName)
{
    QVector<OnSheet> out;
    for (const QString &piece : splitReferences(text)) {
        QString sheet, rest;
        bool hasSheet = false;
        splitSheet(piece, &sheet, &rest, &hasSheet);
        if (hasSheet && sheet.compare(sheetName, Qt::CaseInsensitive) != 0) continue;
        OnSheet b;
        if (!parseBlock(rest, &b.range, &b.kind)) continue;
        out.append(b);
        if (out.size() >= kMaxPrintAreas) break;
    }
    return out;
}

bool looksLikeCell(const QString &name)
{
    quint32 r = 0, c = 0;
    if (parseCellRef(name, &r, &c)) return true;
    static const QRegularExpression rc(QStringLiteral("^[Rr]\\d*[Cc]\\d*$"));
    return rc.match(name).hasMatch();
}

QString sheetPrefix(const QString &name)
{
    static const QRegularExpression plain(QStringLiteral("^[\\p{L}_][\\p{L}\\p{N}_.]*$"));
    if (plain.match(name).hasMatch() && !looksLikeCell(name)) return name + QLatin1Char('!');
    QString q = name;
    q.replace(QLatin1Char('\''), QLatin1String("''"));
    return QLatin1Char('\'') + q + QStringLiteral("'!");
}

QString blockText(const CellRange &r)
{
    const bool allRows = r.row1 == 0 && r.row2 == kMaxRows - 1, allCols = r.col1 == 0 && r.col2 == kMaxColumns - 1;
    if (allRows && !allCols) return QLatin1Char('$') + columnName(r.col1) + QStringLiteral(":$") + columnName(r.col2);
    if (allCols && !allRows) return QLatin1Char('$') + QString::number(r.row1 + 1) + QStringLiteral(":$") + QString::number(r.row2 + 1);
    QString one = QLatin1Char('$') + columnName(r.col1) + QLatin1Char('$') + QString::number(r.row1 + 1);
    if (r.row1 == r.row2 && r.col1 == r.col2) return one;
    return one + QStringLiteral(":$") + columnName(r.col2) + QLatin1Char('$') + QString::number(r.row2 + 1);
}

} // namespace

void readPrintAreaText(const QString &text, const QString &sheetName, PageSetup *out)
{
    out->printArea.clear();
    for (const OnSheet &b : blocksOn(text, sheetName)) out->printArea.append(b.range);
}

void readPrintTitlesText(const QString &text, const QString &sheetName, PageSetup *out)
{
    out->titles = PrintTitles();
    for (const OnSheet &b : blocksOn(text, sheetName)) {
        if (b.kind == BlockKind::Rows && !out->titles.hasRows) {
            out->titles.hasRows = true;
            out->titles.row1 = b.range.row1;
            out->titles.row2 = b.range.row2;
        } else if (b.kind == BlockKind::Columns && !out->titles.hasCols) {
            out->titles.hasCols = true;
            out->titles.col1 = b.range.col1;
            out->titles.col2 = b.range.col2;
        }
    }
}

QString printAreaText(const PageSetup &setup, const QString &sheetName)
{
    QStringList parts;
    const QString prefix = sheetPrefix(sheetName);
    for (const CellRange &r : setup.printArea)
        if (r.valid) parts << prefix + blockText(r);
    return parts.join(QLatin1Char(','));
}

QString printTitlesText(const PageSetup &setup, const QString &sheetName)
{
    QStringList parts;
    const QString prefix = sheetPrefix(sheetName);
    if (setup.titles.hasCols) {
        CellRange c;
        c.row1 = 0;
        c.row2 = kMaxRows - 1;
        c.col1 = setup.titles.col1;
        c.col2 = setup.titles.col2;
        c.valid = true;
        parts << prefix + blockText(c);
    }
    if (setup.titles.hasRows) {
        CellRange r;
        r.row1 = setup.titles.row1;
        r.row2 = setup.titles.row2;
        r.col1 = 0;
        r.col2 = kMaxColumns - 1;
        r.valid = true;
        parts << prefix + blockText(r);
    }
    return parts.join(QLatin1Char(','));
}

// ---- the paper

namespace {

struct Paper {
    int id;
    const char *name;   // null: not offered by name
    double width, height;   // points
};

constexpr double mm(double v) { return v / 25.4 * 72; }
constexpr double in(double v) { return v * 72; }

// The format's numbers, the sizes the standards give them (in points, upright), and the names a menu shows.
const Paper kPapers[] = {
    {1, QT_TRANSLATE_NOOP("Paper", "Letter"), in(8.5), in(11)},
    {2, QT_TRANSLATE_NOOP("Paper", "Letter Small"), in(8.5), in(11)},
    {3, QT_TRANSLATE_NOOP("Paper", "Tabloid"), in(11), in(17)},
    {4, QT_TRANSLATE_NOOP("Paper", "Ledger"), in(17), in(11)},
    {5, QT_TRANSLATE_NOOP("Paper", "Legal"), in(8.5), in(14)},
    {6, QT_TRANSLATE_NOOP("Paper", "Statement"), in(5.5), in(8.5)},
    {7, QT_TRANSLATE_NOOP("Paper", "Executive"), in(7.25), in(10.5)},
    {8, QT_TRANSLATE_NOOP("Paper", "A3"), mm(297), mm(420)},
    {9, QT_TRANSLATE_NOOP("Paper", "A4"), mm(210), mm(297)},
    {10, QT_TRANSLATE_NOOP("Paper", "A4 Small"), mm(210), mm(297)},
    {11, QT_TRANSLATE_NOOP("Paper", "A5"), mm(148), mm(210)},
    {12, QT_TRANSLATE_NOOP("Paper", "B4 (JIS)"), mm(257), mm(364)},
    {13, QT_TRANSLATE_NOOP("Paper", "B5 (JIS)"), mm(182), mm(257)},
    {14, QT_TRANSLATE_NOOP("Paper", "Folio"), in(8.5), in(13)},
    {15, QT_TRANSLATE_NOOP("Paper", "Quarto"), mm(215), mm(275)},
    {16, QT_TRANSLATE_NOOP("Paper", "10 x 14 in"), in(10), in(14)},
    {17, QT_TRANSLATE_NOOP("Paper", "11 x 17 in"), in(11), in(17)},
    {18, QT_TRANSLATE_NOOP("Paper", "Note"), in(8.5), in(11)},
    {19, QT_TRANSLATE_NOOP("Paper", "Envelope No. 9"), in(3.875), in(8.875)},
    {20, QT_TRANSLATE_NOOP("Paper", "Envelope No. 10"), in(4.125), in(9.5)},
    {21, QT_TRANSLATE_NOOP("Paper", "Envelope No. 11"), in(4.5), in(10.375)},
    {22, QT_TRANSLATE_NOOP("Paper", "Envelope No. 12"), in(4.75), in(11)},
    {23, QT_TRANSLATE_NOOP("Paper", "Envelope No. 14"), in(5), in(11.5)},
    {24, QT_TRANSLATE_NOOP("Paper", "C Sheet"), in(17), in(22)},
    {25, QT_TRANSLATE_NOOP("Paper", "D Sheet"), in(22), in(34)},
    {26, QT_TRANSLATE_NOOP("Paper", "E Sheet"), in(34), in(44)},
    {27, QT_TRANSLATE_NOOP("Paper", "Envelope DL"), mm(110), mm(220)},
    {28, QT_TRANSLATE_NOOP("Paper", "Envelope C5"), mm(162), mm(229)},
    {29, QT_TRANSLATE_NOOP("Paper", "Envelope C3"), mm(324), mm(458)},
    {30, QT_TRANSLATE_NOOP("Paper", "Envelope C4"), mm(229), mm(324)},
    {31, QT_TRANSLATE_NOOP("Paper", "Envelope C6"), mm(114), mm(162)},
    {32, QT_TRANSLATE_NOOP("Paper", "Envelope C65"), mm(114), mm(229)},
    {33, QT_TRANSLATE_NOOP("Paper", "B4 (ISO)"), mm(250), mm(353)},
    {34, QT_TRANSLATE_NOOP("Paper", "B5 (ISO)"), mm(176), mm(250)},
    {35, QT_TRANSLATE_NOOP("Paper", "B6 (ISO)"), mm(125), mm(176)},
    {36, QT_TRANSLATE_NOOP("Paper", "Envelope Italy"), mm(110), mm(230)},
    {37, QT_TRANSLATE_NOOP("Paper", "Envelope Monarch"), in(3.875), in(7.5)},
    {38, QT_TRANSLATE_NOOP("Paper", "Envelope 6 3/4"), in(3.625), in(6.5)},
    {39, QT_TRANSLATE_NOOP("Paper", "US Standard Fanfold"), in(14.875), in(11)},
    {40, QT_TRANSLATE_NOOP("Paper", "German Standard Fanfold"), in(8.5), in(12)},
    {41, QT_TRANSLATE_NOOP("Paper", "German Legal Fanfold"), in(8.5), in(13)},
    {43, QT_TRANSLATE_NOOP("Paper", "Japanese Postcard"), mm(100), mm(148)},
    {44, QT_TRANSLATE_NOOP("Paper", "9 x 11 in"), in(9), in(11)},
    {45, QT_TRANSLATE_NOOP("Paper", "10 x 11 in"), in(10), in(11)},
    {46, QT_TRANSLATE_NOOP("Paper", "15 x 11 in"), in(15), in(11)},
    {47, QT_TRANSLATE_NOOP("Paper", "Envelope Invite"), mm(220), mm(220)},
    {50, QT_TRANSLATE_NOOP("Paper", "Letter Extra"), in(9.5), in(12)},
    {51, QT_TRANSLATE_NOOP("Paper", "Legal Extra"), in(9.5), in(15)},
    {52, QT_TRANSLATE_NOOP("Paper", "Tabloid Extra"), in(11.69), in(18)},
    {53, QT_TRANSLATE_NOOP("Paper", "A4 Extra"), mm(235.5), mm(322.3)},
    {54, QT_TRANSLATE_NOOP("Paper", "Letter Transverse"), in(8.5), in(11)},
    {55, QT_TRANSLATE_NOOP("Paper", "A4 Transverse"), mm(210), mm(297)},
    {56, QT_TRANSLATE_NOOP("Paper", "Letter Extra Transverse"), in(9.5), in(12)},
    {57, QT_TRANSLATE_NOOP("Paper", "Super A"), mm(227), mm(356)},
    {58, QT_TRANSLATE_NOOP("Paper", "Super B"), mm(305), mm(487)},
    {59, QT_TRANSLATE_NOOP("Paper", "Letter Plus"), in(8.5), in(12.69)},
    {60, QT_TRANSLATE_NOOP("Paper", "A4 Plus"), mm(210), mm(330)},
    {61, QT_TRANSLATE_NOOP("Paper", "A5 Transverse"), mm(148), mm(210)},
    {62, QT_TRANSLATE_NOOP("Paper", "B5 (JIS) Transverse"), mm(182), mm(257)},
    {63, QT_TRANSLATE_NOOP("Paper", "A3 Extra"), mm(322), mm(445)},
    {64, QT_TRANSLATE_NOOP("Paper", "A5 Extra"), mm(174), mm(235)},
    {65, QT_TRANSLATE_NOOP("Paper", "B5 (ISO) Extra"), mm(201), mm(276)},
    {66, QT_TRANSLATE_NOOP("Paper", "A2"), mm(420), mm(594)},
    {67, QT_TRANSLATE_NOOP("Paper", "A3 Transverse"), mm(297), mm(420)},
    {68, QT_TRANSLATE_NOOP("Paper", "A3 Extra Transverse"), mm(322), mm(445)},
    {69, QT_TRANSLATE_NOOP("Paper", "Japanese Double Postcard"), mm(200), mm(148)},
    {70, QT_TRANSLATE_NOOP("Paper", "A6"), mm(105), mm(148)},
};

const Paper *findPaper(int id)
{
    for (const Paper &p : kPapers)
        if (p.id == id) return &p;
    return nullptr;
}

} // namespace

void paperSizePoints(int paperSize, double *width, double *height)
{
    const Paper *p = findPaper(paperSize);
    if (!p) p = findPaper(1);
    *width = p->width;
    *height = p->height;
}

bool isKnownPaperSize(int paperSize) { return findPaper(paperSize) != nullptr; }

QVector<int> commonPaperSizes() { return {1, 5, 3, 7, 6, 8, 9, 11, 12, 13, 14, 20, 27, 28}; }

QString paperSizeName(int paperSize)
{
    const Paper *p = findPaper(paperSize);
    return p ? QCoreApplication::translate("Paper", p->name) : QString();
}

bool isLandscape(const PageSetup &setup) { return setup.orientation == PageOrientation::Landscape; }

void pageSizePoints(const PageSetup &setup, double *width, double *height)
{
    double w = 0, h = 0;
    if (!isKnownPaperSize(setup.paperSize) && setup.paperWidth > 0 && setup.paperHeight > 0) {
        w = setup.paperWidth;
        h = setup.paperHeight;
    } else {
        paperSizePoints(setup.paperSize, &w, &h);
    }
    if (isLandscape(setup) && w < h) std::swap(w, h);
    *width = w;
    *height = h;
}

} // namespace jp::office::sheet
