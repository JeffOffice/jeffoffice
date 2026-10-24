// The formatting commands of JeffCrunch's Home tab and the Format Cells dialog: the font, its size and color, bold, italic, and the underlines, the fill, the borders, the alignment
// (horizontal, vertical, wrap, indent, orientation), merging, the number format and its buttons, the cell styles, the Format Painter, and Clear. Each is one step of Undo (the edit
// session makes them; this part says what the buttons mean and keeps the buttons in step with the cell with the cursor).
#include "app/crunchwindow.h"

#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "core/cellstyles.h"
#include "core/formatcodes.h"
#include "sheet/builtinstyles.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QFontDatabase>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QUndoStack>
#include <QWidgetAction>

#include <algorithm>
#include <cmath>

namespace jp {

using namespace jp::office::sheet;
using jp::crunch::NumberCategory;
using jp::crunch::NumberOptions;
using MergeKind = crunch::EditSession::MergeKind;

namespace {

// The border commands of the Borders menu.
enum BorderCommand {
    BottomBorder, TopBorder, LeftBorder, RightBorder, NoBorder, AllBorders, OutsideBorders, ThickOutside, BottomDouble, ThickBottom, TopBottom, TopThickBottom, TopDoubleBottom,
    InsideHorizontal, InsideVertical, InsideBorders, BorderCommandCount
};

const double kSizes[] = {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};

QIcon styleIconOf(const crunch::StyleSample &s)
{
    QPixmap pm(44, 22);
    pm.fill(s.hasFill ? QColor::fromRgb(s.fillRgb) : QColor(Qt::white));
    QPainter p(&pm);
    p.setPen(s.hasBorder ? QColor::fromRgb(s.borderRgb) : QColor(0xC8, 0xC8, 0xC8));
    p.drawRect(0, 0, 43, 21);
    QFont f = p.font();
    f.setBold(s.bold);
    f.setItalic(s.italic);
    f.setUnderline(s.underline);
    f.setPixelSize(13);
    p.setFont(f);
    p.setPen(QColor::fromRgb(s.textRgb));
    p.drawText(QRect(0, 0, 44, 22), Qt::AlignCenter, QStringLiteral("AaBb"));
    return QIcon(pm);
}

} // namespace

// ---------------------------------------------------------------- the commands

void CrunchWindow::createFormatActions()
{
    auto add = [this](const QString &id, const QString &text, const char *iconName, const QKeySequence &key, const std::function<void()> &fn, bool checkable = false) {
        mk(id, text, QString::fromLatin1(iconName), key, fn, checkable, true);
        m_formatActions << id;
    };
    add(QStringLiteral("fmt.bold"), tr("Bold"), "bold", QKeySequence(Qt::CTRL | Qt::Key_B), [this] { toggleFont(0); }, true);
    add(QStringLiteral("fmt.italic"), tr("Italic"), "italic", QKeySequence(Qt::CTRL | Qt::Key_I), [this] { toggleFont(1); }, true);
    add(QStringLiteral("fmt.underline"), tr("Underline"), "underline", QKeySequence(Qt::CTRL | Qt::Key_U), [this] { toggleFont(2); }, true);
    add(QStringLiteral("fmt.underline.single"), tr("Underline"), "underline", QKeySequence(), [this] { toggleFont(2); });
    add(QStringLiteral("fmt.underline.double"), tr("Double Underline"), "underline", QKeySequence(), [this] { toggleFont(3); });
    add(QStringLiteral("fmt.strike"), tr("Strikethrough"), "strikethrough", QKeySequence(Qt::CTRL | Qt::Key_5), [this] { toggleFont(4); }, true);
    add(QStringLiteral("fmt.growFont"), tr("Increase Font Size"), "a-arrow-up", QKeySequence(), [this] { growFont(1); });
    add(QStringLiteral("fmt.shrinkFont"), tr("Decrease Font Size"), "a-arrow-down", QKeySequence(), [this] { growFont(-1); });
    // alignment
    add(QStringLiteral("fmt.alignLeft"), tr("Align Left"), "align-left", QKeySequence(), [this] { alignCommand(0); }, true);
    add(QStringLiteral("fmt.alignCenter"), tr("Center"), "align-center", QKeySequence(), [this] { alignCommand(1); }, true);
    add(QStringLiteral("fmt.alignRight"), tr("Align Right"), "align-right", QKeySequence(), [this] { alignCommand(2); }, true);
    add(QStringLiteral("fmt.alignTop"), tr("Top Align"), "align-vertical-justify-start", QKeySequence(), [this] { alignCommand(3); }, true);
    add(QStringLiteral("fmt.alignMiddle"), tr("Middle Align"), "align-vertical-justify-center", QKeySequence(), [this] { alignCommand(4); }, true);
    add(QStringLiteral("fmt.alignBottom"), tr("Bottom Align"), "align-vertical-justify-end", QKeySequence(), [this] { alignCommand(5); }, true);
    add(QStringLiteral("fmt.wrap"), tr("Wrap Text"), "wrap-text", QKeySequence(), [this] { alignCommand(6); }, true);
    add(QStringLiteral("fmt.indentInc"), tr("Increase Indent"), "indent-increase", QKeySequence(), [this] { indentCommand(1); });
    add(QStringLiteral("fmt.indentDec"), tr("Decrease Indent"), "indent-decrease", QKeySequence(), [this] { indentCommand(-1); });
    add(QStringLiteral("fmt.merge"), tr("Merge and Center"), "table-cells-merge", QKeySequence(), [this] { mergeCommand(MergeKind::MergeAndCenter); }, true);
    add(QStringLiteral("fmt.merge.center"), tr("Merge and Center"), "table-cells-merge", QKeySequence(), [this] { mergeCommand(MergeKind::MergeAndCenter); });
    add(QStringLiteral("fmt.merge.across"), tr("Merge Across"), "table-cells-merge", QKeySequence(), [this] { mergeCommand(MergeKind::Across); });
    add(QStringLiteral("fmt.merge.cells"), tr("Merge Cells"), "table-cells-merge", QKeySequence(), [this] { mergeCommand(MergeKind::Merge); });
    add(QStringLiteral("fmt.merge.unmerge"), tr("Unmerge Cells"), "table-cells-split", QKeySequence(), [this] { mergeCommand(MergeKind::Unmerge); });
    // number
    add(QStringLiteral("fmt.currency"), tr("Currency"), "dollar-sign", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Dollar), [this] { numberFormatCommand(2); });
    add(QStringLiteral("fmt.percent"), tr("Percent Style"), "percent", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Percent), [this] { numberFormatCommand(7); });
    add(QStringLiteral("fmt.comma"), tr("Comma Style"), "hash", QKeySequence(), [this] { numberFormatCommand(13); });
    add(QStringLiteral("fmt.decimalInc"), tr("Increase Decimal"), "arrow-up-0-1", QKeySequence(), [this] { decimalsCommand(1); });
    add(QStringLiteral("fmt.decimalDec"), tr("Decrease Decimal"), "arrow-down-1-0", QKeySequence(), [this] { decimalsCommand(-1); });
    add(QStringLiteral("fmt.numberGeneral"), tr("General"), "", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_AsciiTilde), [this] { numberFormatCommand(0); });
    // the borders, one command each
    const struct {
        const char *id;
        const char *text;
    } borders[] = {{"fmt.border.bottom", QT_TR_NOOP("Bottom Border")},          {"fmt.border.top", QT_TR_NOOP("Top Border")},
                   {"fmt.border.left", QT_TR_NOOP("Left Border")},              {"fmt.border.right", QT_TR_NOOP("Right Border")},
                   {"fmt.border.none", QT_TR_NOOP("No Border")},                {"fmt.border.all", QT_TR_NOOP("All Borders")},
                   {"fmt.border.outside", QT_TR_NOOP("Outside Borders")},       {"fmt.border.thickOutside", QT_TR_NOOP("Thick Outside Borders")},
                   {"fmt.border.bottomDouble", QT_TR_NOOP("Bottom Double Border")}, {"fmt.border.thickBottom", QT_TR_NOOP("Thick Bottom Border")},
                   {"fmt.border.topBottom", QT_TR_NOOP("Top and Bottom Border")}, {"fmt.border.topThickBottom", QT_TR_NOOP("Top and Thick Bottom Border")},
                   {"fmt.border.topDoubleBottom", QT_TR_NOOP("Top and Double Bottom Border")}, {"fmt.border.insideH", QT_TR_NOOP("Inside Horizontal Border")},
                   {"fmt.border.insideV", QT_TR_NOOP("Inside Vertical Border")}, {"fmt.border.inside", QT_TR_NOOP("Inside Borders")}};
    for (int i = 0; i < int(sizeof(borders) / sizeof(borders[0])); ++i) add(QString::fromLatin1(borders[i].id), tr(borders[i].text), "grid-2x2", QKeySequence(), [this, i] { borderCommand(i); });
    add(QStringLiteral("fmt.borders"), tr("Borders"), "grid-2x2", QKeySequence(), [this] { borderCommand(m_lastBorder); });
    // Clear
    add(QStringLiteral("edit.clear.all"), tr("Clear All"), "eraser", QKeySequence(), [this] { clearCommand(0); });
    add(QStringLiteral("edit.clear.formats"), tr("Clear Formats"), "eraser", QKeySequence(), [this] { clearCommand(1); });
    add(QStringLiteral("edit.clear.contents"), tr("Clear Contents"), "eraser", QKeySequence(), [this] { clearCommand(2); });
    add(QStringLiteral("edit.clear"), tr("Clear"), "eraser", QKeySequence(), [this] { clearCommand(0); });
    // the dialog, styles, the painter
    add(QStringLiteral("format.cells"), tr("Format Cells…"), "", QKeySequence(Qt::CTRL | Qt::Key_1), [this] { formatCellsDialog(0); });
    add(QStringLiteral("fmt.painter"), tr("Format Painter"), "paintbrush", QKeySequence(), [this] { formatPainterCommand(); }, true);
    act(QStringLiteral("fmt.painter"))->setToolTip(tr("Format Painter: copies the format of the selected cells to the cells you select next. Double-click to paint more than once."));
    add(QStringLiteral("fmt.fontDialog"), tr("Format Cells: Font"), "", QKeySequence(), [this] { formatCellsDialog(2); });
    add(QStringLiteral("fmt.alignDialog"), tr("Format Cells: Alignment"), "", QKeySequence(), [this] { formatCellsDialog(1); });
    add(QStringLiteral("fmt.numberDialog"), tr("More Number Formats…"), "", QKeySequence(), [this] { formatCellsDialog(0); });
    // the orientation commands
    const struct {
        const char *id;
        const char *text;
        int degrees;
    } turns[] = {{"fmt.turn.none", QT_TR_NOOP("Horizontal Text"), 0},       {"fmt.turn.ccw", QT_TR_NOOP("Angle Counterclockwise"), 45},
                 {"fmt.turn.cw", QT_TR_NOOP("Angle Clockwise"), -45},      {"fmt.turn.vertical", QT_TR_NOOP("Vertical Text"), 1000},
                 {"fmt.turn.up", QT_TR_NOOP("Rotate Text Up"), 90},         {"fmt.turn.down", QT_TR_NOOP("Rotate Text Down"), -90}};
    for (const auto &t : turns) {
        const int degrees = t.degrees;
        add(QString::fromLatin1(t.id), tr(t.text), "rotate-ccw", QKeySequence(), [this, degrees] { orientationCommand(degrees); });
    }
    connect(m_view, &grid::GridView::selectionSettled, this, [this] { paintFormats(); });
}

// Whether a command of formatting may be used: the workbook is open and nothing is being saved.
void CrunchWindow::updateFormatState(bool force)
{
    if (!m_actions.contains(QStringLiteral("fmt.painter"))) return;   // (the commands are not made yet)
    const bool open = m_session && !m_saving && m_loaded && m_loaded->problem.isEmpty();
    if (force || int(open) != m_formatOpen) {
        m_formatOpen = int(open);
        for (const QString &id : m_formatActions)
            if (QAction *a = act(id)) a->setEnabled(open);
        for (const auto &c : m_fontCombos)
            if (c) c->setEnabled(open);
        for (const auto &c : m_sizeCombos)
            if (c) c->setEnabled(open);
        for (const QPointer<ColorButton> &b : {m_fillButton, m_fontColorButton})
            if (b) b->setEnabled(open);
        if (m_numberBox) m_numberBox->setEnabled(open);
    }
    if (!open || m_formatting) return;
    // (the format the buttons show is that of the cell format of the cell with the cursor, with whether it is merged and what the Format Painter is doing: a cell that has the same as the
    // last one shown needs nothing done, and a person's edit, which keeps the cursor on cells of the same format, then costs the buttons nothing)
    FormatShown shown;
    shown.book = m_loaded ? m_loaded->book.get() : nullptr;
    if (m_loaded && m_loaded->book) {
        const grid::GridSheet &gs = m_view->sheet();
        const grid::Selection &sel = m_view->selection();
        const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
        shown.xf = gs.styleOf(row, col, gs.cellAt(row, col));
        const bool merged = gs.layout().merges.count() > 0 && gs.layout().merges.at(row, col) >= 0;
        shown.flags = (merged ? 1u : 0u) | (quint32(m_painter.mode) << 1) | (quint32(m_sheet + 1) << 8);
    }
    if (!force && shown == m_formatShown) return;
    m_formatShown = shown;
    ++m_formatStateRuns;
    const FormatView v = viewOfActiveCell();
    auto check = [this](const char *id, bool on) {
        if (QAction *a = act(QString::fromLatin1(id))) {
            const QSignalBlocker block(a);
            a->setChecked(on);
        }
    };
    check("fmt.bold", v.bold);
    check("fmt.italic", v.italic);
    check("fmt.underline", v.underline == Underline::Single || v.underline == Underline::SingleAccounting);
    check("fmt.strike", v.strike);
    check("fmt.alignLeft", v.horizontal == HAlign::Left);
    check("fmt.alignCenter", v.horizontal == HAlign::Center || v.horizontal == HAlign::CenterContinuous);
    check("fmt.alignRight", v.horizontal == HAlign::Right);
    check("fmt.alignTop", v.vertical == VAlign::Top);
    check("fmt.alignMiddle", v.vertical == VAlign::Center);
    check("fmt.alignBottom", v.vertical == VAlign::Bottom);
    check("fmt.wrap", v.wrap);
    check("fmt.merge", v.merged);
    for (const auto &c : m_fontCombos)
        if (c) c->setCurrentFamily(v.fontName);
    for (const auto &c : m_sizeCombos)
        if (c) c->setSize(v.fontSize);
    if (m_numberBox) {
        const QSignalBlocker block(m_numberBox);
        const NumberCategory category = jp::crunch::categoryOf(v.numberFormat);
        int index = m_numberBox->count() - 2;   // Custom
        switch (category) {
        case NumberCategory::General: index = 0; break;
        case NumberCategory::Number: index = 1; break;
        case NumberCategory::Currency: index = 2; break;
        case NumberCategory::Accounting: index = 3; break;
        case NumberCategory::Date: index = v.numberFormat.contains(QStringLiteral("dddd")) ? 5 : 4; break;
        case NumberCategory::Time: index = 6; break;
        case NumberCategory::Percentage: index = 7; break;
        case NumberCategory::Fraction: index = 8; break;
        case NumberCategory::Scientific: index = 9; break;
        case NumberCategory::Text: index = 10; break;
        default: break;
        }
        m_numberBox->setCurrentIndex(index);
    }
    act(QStringLiteral("fmt.painter"))->setChecked(m_painter.mode != PainterMode::Off);
}

bool CrunchWindow::confirm(const QString &title, const QString &text)
{
    if (m_confirmAsker) return m_confirmAsker(title, text);
    return QMessageBox::question(this, title, text, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) == QMessageBox::Ok;
}

// Gives the selected cells a change of format: one step of Undo called `text`. An entry in progress is finished first.
bool CrunchWindow::formatSelection(const FormatChange &change, const QString &text)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return false;
    if (!editingAllowed()) return false;
    m_formatting = true;
    const crunch::EditSession::Outcome o = m_session->formatCells(m_sheet, selectedRange(), change, text);
    m_formatting = false;
    if (!o.ok) {
        notice(tr("Format Cells"), o.problem);
        return false;
    }
    updateFormatState();
    return true;
}

// ---------------------------------------------------------------- the format of the cell with the cursor

ColorScheme CrunchWindow::workbookColors() const
{
    ColorScheme scheme = builtinColorSchemes().first();
    if (!m_loaded || !m_loaded->book) return scheme;
    const Styles &st = m_loaded->book->styles();
    auto theme = [&st](int index, const QColor &fallback) {
        ColorSpec c;
        c.kind = ColorSpec::Kind::Theme;
        c.index = index;
        return QColor::fromRgb(st.resolve(c, quint32(fallback.rgb() & 0xFFFFFF)));
    };
    scheme.c[Main] = theme(1, scheme.c[Main]);
    for (int i = 0; i < 5; ++i) scheme.c[Accent1 + i] = theme(4 + i, scheme.c[Accent1 + i]);
    scheme.c[Hyperlink] = theme(10, scheme.c[Hyperlink]);
    scheme.c[Followed] = theme(11, scheme.c[Followed]);
    return scheme;
}

ColorSpec CrunchWindow::colorSpecOf(const ColorRef &c) const
{
    if (c.isNone()) return ColorSpec();
    return ColorSpec::fromRgb(quint32(c.resolve(workbookColors()).rgb() & 0xFFFFFF));
}

FormatView CrunchWindow::viewOfActiveCell() const
{
    FormatView v;
    if (!m_loaded || !m_loaded->book) return v;
    const Styles &st = m_loaded->book->styles();
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const grid::GridSheet &gs = m_view->sheet();
    const quint32 xf = gs.styleOf(row, col, gs.cellAt(row, col));
    const CellFormat f = xf < quint32(st.formats.size()) ? st.formats[int(xf)] : CellFormat();
    const FontSpec &font = st.font(f.font);
    v.fontName = st.fontNameOf(font);
    if (v.fontName.isEmpty()) v.fontName = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();   // (a workbook with no usual font: this computer's)
    v.fontSize = font.size;
    v.bold = font.bold;
    v.italic = font.italic;
    v.strike = font.strike;
    v.underline = font.underline;
    v.vertAlign = font.vertAlign;
    if (font.color.isSet() && font.color.kind != ColorSpec::Kind::Auto) v.fontColor = QColor::fromRgb(st.resolve(font.color, 0x000000));
    const FillSpec &fill = st.fill(f.fill);
    v.pattern = fill.pattern;
    if (fill.pattern == PatternKind::Solid) {
        v.fillColor = QColor::fromRgb(st.resolve(fill.fg, 0xFFFFFF));
    } else if (fill.pattern != PatternKind::None) {
        v.patternColor = QColor::fromRgb(st.resolve(fill.fg, 0x000000));
        v.fillColor = QColor::fromRgb(st.resolve(fill.bg, 0xFFFFFF));
    }
    BorderSpec b = st.border(f.border);
    for (BorderSide *s : {&b.left, &b.right, &b.top, &b.bottom, &b.diagonal})
        if (s->style != BorderStyle::None) s->color = ColorSpec::fromRgb(st.resolve(s->color, 0x000000));
    v.border = b;
    v.horizontal = f.horizontal;
    v.vertical = f.vertical;
    v.wrap = f.wrap;
    v.shrink = f.shrink;
    v.indent = f.indent;
    v.rotation = f.rotation;
    v.readingOrder = f.readingOrder;
    v.locked = f.locked;
    v.formulaHidden = f.formulaHidden;
    v.merged = gs.layout().merges.count() > 0 && gs.layout().merges.at(row, col) >= 0;
    v.numberFormat = m_session ? m_session->formatCodeOf(m_sheet, row, col) : QStringLiteral("General");
    return v;
}

FormatCellsDialog::Input CrunchWindow::formatInput() const
{
    FormatCellsDialog::Input in;
    in.view = viewOfActiveCell();
    in.locale = m_session ? m_session->locale() : QLocale();
    if (m_loaded && m_loaded->book) {
        in.dateSystem = m_loaded->book->info().dateSystem;
        const Styles &st = m_loaded->book->styles();
        for (auto it = st.numFmts.constBegin(); it != st.numFmts.constEnd(); ++it)
            if (!in.customCodes.contains(it.value())) in.customCodes << it.value();
        std::sort(in.customCodes.begin(), in.customCodes.end());
    }
    const grid::Selection &sel = m_view->selection();
    in.oneCell = sel.range.row1 == sel.range.row2 && sel.range.col1 == sel.range.col2;
    if (m_session) {
        const jp::calc::Value v = m_session->valueAt(m_sheet, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
        if (v.isNumber()) {
            in.sampleIsNumber = true;
            in.sampleNumber = v.number();
        } else if (v.isText()) {
            in.sampleIsNumber = false;
            in.sampleText = v.text();
        }
    }
    const ColorScheme scheme = workbookColors();
    in.palette = {scheme.c[Main], scheme.c[Accent1], scheme.c[Accent2], scheme.c[Accent3], scheme.c[Accent4], scheme.c[Accent5], scheme.c[Hyperlink], scheme.c[Followed]};
    return in;
}

// ---------------------------------------------------------------- font, fill

// What the buttons of the font do: bold, italic, underline, double underline, strikethrough. The state of the cell with the cursor decides whether the command puts the style on or takes it
// off (for all the cells selected).
void CrunchWindow::toggleFont(int what)
{
    const FormatView v = viewOfActiveCell();
    FormatChange c;
    QString text;
    switch (what) {
    case 0: c.bold = !v.bold; text = tr("Bold"); break;
    case 1: c.italic = !v.italic; text = tr("Italic"); break;
    case 2: c.underline = (v.underline == Underline::Single || v.underline == Underline::SingleAccounting) ? Underline::None : Underline::Single; text = tr("Underline"); break;
    case 3: c.underline = (v.underline == Underline::Double || v.underline == Underline::DoubleAccounting) ? Underline::None : Underline::Double; text = tr("Double Underline"); break;
    case 4: c.strike = !v.strike; text = tr("Strikethrough"); break;
    default: return;
    }
    formatSelection(c, text);
    updateFormatState();
}

void CrunchWindow::setFontSize(double size)
{
    FormatChange c;
    c.fontSize = size;
    formatSelection(c, tr("Font Size"));
}

// Increase and Decrease Font Size: the next size of the list the size box has (or one point for a size past it).
void CrunchWindow::growFont(int steps)
{
    double size = viewOfActiveCell().fontSize;
    if (steps > 0) {
        double next = size + 1;
        for (double s : kSizes)
            if (s > size) {
                next = s;
                break;
            }
        size = next;
    } else {
        double next = std::max(1.0, size - 1);
        for (int i = int(sizeof(kSizes) / sizeof(kSizes[0])) - 1; i >= 0; --i)
            if (kSizes[i] < size) {
                next = kSizes[i];
                break;
            }
        size = next;
    }
    setFontSize(size);
}

void CrunchWindow::chooseFillColor(const ColorRef &c)
{
    FormatChange change;
    FillSpec fill;
    if (!c.isNone()) {
        fill.pattern = PatternKind::Solid;
        fill.fg = colorSpecOf(c);
        fill.bg.kind = ColorSpec::Kind::Indexed;
        fill.bg.index = 64;
    }
    change.fill = fill;
    formatSelection(change, c.isNone() ? tr("No Fill") : tr("Fill Color"));
}

void CrunchWindow::chooseFontColor(const ColorRef &c)
{
    FormatChange change;
    change.fontColor = colorSpecOf(c);
    formatSelection(change, tr("Font Color"));
}

// ---------------------------------------------------------------- alignment

// 0 left, 1 center, 2 right, 3 top, 4 middle, 5 bottom, 6 wrap text: a button that is on turns off what it set (the general alignment, the bottom).
void CrunchWindow::alignCommand(int kind)
{
    const FormatView v = viewOfActiveCell();
    FormatChange c;
    QString text = tr("Alignment");
    switch (kind) {
    case 0: c.horizontal = v.horizontal == HAlign::Left ? HAlign::General : HAlign::Left; break;
    case 1: c.horizontal = (v.horizontal == HAlign::Center) ? HAlign::General : HAlign::Center; break;
    case 2: c.horizontal = v.horizontal == HAlign::Right ? HAlign::General : HAlign::Right; break;
    case 3: c.vertical = v.vertical == VAlign::Top ? VAlign::Bottom : VAlign::Top; break;
    case 4: c.vertical = v.vertical == VAlign::Center ? VAlign::Bottom : VAlign::Center; break;
    case 5: c.vertical = VAlign::Bottom; break;
    case 6:
        c.wrap = !v.wrap;
        if (*c.wrap) c.shrink = false;
        text = tr("Wrap Text");
        break;
    default: return;
    }
    formatSelection(c, text);
    updateFormatState();
}

// Increase and Decrease Indent: a place more or less; text with the general alignment (or centered) is aligned to the left to be indented.
void CrunchWindow::indentCommand(int delta)
{
    const FormatView v = viewOfActiveCell();
    FormatChange c;
    const int indent = std::max(0, std::min(250, v.indent + delta));
    if (indent == v.indent) return;
    c.indent = indent;
    if (delta > 0 && (v.horizontal == HAlign::General || v.horizontal == HAlign::Center || v.horizontal == HAlign::Fill || v.horizontal == HAlign::Justify || v.horizontal == HAlign::CenterContinuous))
        c.horizontal = HAlign::Left;
    if (indent == 0 && v.horizontal == HAlign::Left) c.horizontal = HAlign::General;
    formatSelection(c, delta > 0 ? tr("Increase Indent") : tr("Decrease Indent"));
}

// The orientation menu: degrees counterclockwise (-90 to 90), or 1000 for the letters one under another.
void CrunchWindow::orientationCommand(int degrees)
{
    FormatChange c;
    c.rotation = degrees == 1000 ? 255 : degrees >= 0 ? degrees : 90 - degrees;
    formatSelection(c, tr("Orientation"));
}

void CrunchWindow::mergeCommand(MergeKind kind)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    const CellRange range = selectedRange();
    if (kind == MergeKind::MergeAndCenter && viewOfActiveCell().merged) kind = MergeKind::Unmerge;   // (the button turns off what it did)
    crunch::EditSession::Outcome o = m_session->mergeCells(m_sheet, range, kind, false);
    if (o.ok == false && o.confirm) {
        if (!confirm(tr("Merge Cells"), o.problem)) return;
        o = m_session->mergeCells(m_sheet, range, kind, true);
    }
    if (!o.ok) notice(tr("Merge Cells"), o.problem);
    updateFormatState();
}

// ---------------------------------------------------------------- numbers

// 0 General, 1 Number, 2 Currency, 3 Accounting, 4 Short Date, 5 Long Date, 6 Time, 7 Percentage, 8 Fraction, 9 Scientific, 10 Text; 13 Comma Style.
void CrunchWindow::numberFormatCommand(int kind)
{
    const QLocale locale = m_session ? m_session->locale() : QLocale();
    QString code;
    switch (kind) {
    case 0: code = QStringLiteral("General"); break;
    case 1: code = QStringLiteral("0.00"); break;
    case 2: code = jp::crunch::currencyCode(locale); break;
    case 3: {
        NumberOptions o;
        o.decimals = 2;
        o.thousands = true;
        o.symbol = jp::crunch::currencySymbolOf(locale, &o.symbolFirst);
        code = jp::crunch::codeFor(NumberCategory::Accounting, o);
        break;
    }
    case 4: code = jp::calc::shortDateFormatCode(locale); break;
    case 5: code = QStringLiteral("dddd, mmmm d, yyyy"); break;
    case 6: code = QStringLiteral("h:mm:ss"); break;
    case 7: code = kind == 7 && viewOfActiveCell().numberFormat == jp::crunch::percentCode() ? QStringLiteral("General") : jp::crunch::percentCode(); break;
    case 8: code = QStringLiteral("# ?/?"); break;
    case 9: code = QStringLiteral("0.00E+00"); break;
    case 10: code = QStringLiteral("@"); break;
    case 13: code = jp::crunch::commaCode(); break;
    default: return;
    }
    FormatChange c;
    c.numberFormat = code;
    formatSelection(c, tr("Number Format"));
}

void CrunchWindow::decimalsCommand(int delta)
{
    const QString code = viewOfActiveCell().numberFormat;
    const QString now = jp::crunch::withDecimals(code, delta);
    if (now == code) return;
    FormatChange c;
    c.numberFormat = now;
    formatSelection(c, delta > 0 ? tr("Increase Decimal") : tr("Decrease Decimal"));
}

// ---------------------------------------------------------------- borders

void CrunchWindow::borderCommand(int kind)
{
    if (kind < 0 || kind >= BorderCommandCount) return;
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    using E = crunch::EditSession;
    BorderSide line;
    line.style = m_lineStyle;
    line.color = m_lineColor;
    BorderSide thick = line, dbl = line, none;
    thick.style = BorderStyle::Thick;
    dbl.style = BorderStyle::Double;
    const int outside = E::EdgeLeft | E::EdgeRight | E::EdgeTop | E::EdgeBottom;
    struct Op {
        int edges;
        BorderSide side;
    };
    QVector<Op> ops;
    switch (kind) {
    case BottomBorder: ops = {{E::EdgeBottom, line}}; break;
    case TopBorder: ops = {{E::EdgeTop, line}}; break;
    case LeftBorder: ops = {{E::EdgeLeft, line}}; break;
    case RightBorder: ops = {{E::EdgeRight, line}}; break;
    case NoBorder: ops = {{outside | E::EdgeInsideHorizontal | E::EdgeInsideVertical | E::EdgeDiagonalUp | E::EdgeDiagonalDown, none}}; break;
    case AllBorders: ops = {{outside | E::EdgeInsideHorizontal | E::EdgeInsideVertical, line}}; break;
    case OutsideBorders: ops = {{outside, line}}; break;
    case ThickOutside: ops = {{outside, thick}}; break;
    case BottomDouble: ops = {{E::EdgeBottom, dbl}}; break;
    case ThickBottom: ops = {{E::EdgeBottom, thick}}; break;
    case TopBottom: ops = {{E::EdgeTop | E::EdgeBottom, line}}; break;
    case TopThickBottom: ops = {{E::EdgeTop, line}, {E::EdgeBottom, thick}}; break;
    case TopDoubleBottom: ops = {{E::EdgeTop, line}, {E::EdgeBottom, dbl}}; break;
    case InsideHorizontal: ops = {{E::EdgeInsideHorizontal, line}}; break;
    case InsideVertical: ops = {{E::EdgeInsideVertical, line}}; break;
    case InsideBorders: ops = {{E::EdgeInsideHorizontal | E::EdgeInsideVertical, line}}; break;
    }
    m_lastBorder = kind;
    const CellRange range = selectedRange();
    QUndoStack *u = m_session->undoStack();
    const QString text = kind == NoBorder ? tr("Remove Borders") : tr("Borders");
    if (ops.size() > 1) u->beginMacro(text);
    bool ok = true;
    for (const Op &op : ops) {
        const crunch::EditSession::Outcome o = m_session->setBorders(m_sheet, range, op.edges, op.side, text);
        if (!o.ok) {
            notice(tr("Borders"), o.problem);
            ok = false;
            break;
        }
    }
    if (ops.size() > 1) u->endMacro();
    Q_UNUSED(ok);
    updateFormatState();
}

QMenu *CrunchWindow::buildBordersMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("bordersMenu"));
    const struct {
        const char *id;
    } order[] = {{"fmt.border.bottom"},       {"fmt.border.top"},        {"fmt.border.left"},         {"fmt.border.right"},
                 {nullptr},                    {"fmt.border.none"},       {"fmt.border.all"},          {"fmt.border.outside"},
                 {"fmt.border.thickOutside"}, {nullptr},                 {"fmt.border.bottomDouble"}, {"fmt.border.thickBottom"},
                 {"fmt.border.topBottom"},    {"fmt.border.topThickBottom"}, {"fmt.border.topDoubleBottom"}, {nullptr},
                 {"fmt.border.insideH"},      {"fmt.border.insideV"},    {"fmt.border.inside"}};
    for (const auto &o : order) {
        if (!o.id) menu->addSeparator();
        else menu->addAction(act(QString::fromLatin1(o.id)));
    }
    menu->addSeparator();
    // the color and style of the lines the commands draw
    QMenu *colorMenu = menu->addMenu(icon(QStringLiteral("palette")), tr("Line Color"));
    auto *colorAction = new QWidgetAction(colorMenu);
    colorAction->setDefaultWidget(colorGridWidget(workbookColors(), [this, colorMenu](const ColorRef &c) {
        m_lineColor = colorSpecOf(c);
        colorMenu->close();
        if (QMenu *parent = qobject_cast<QMenu *>(colorMenu->parentWidget())) parent->close();
    }, colorMenu));
    colorMenu->addAction(colorAction);
    QMenu *styleMenu = menu->addMenu(icon(QStringLiteral("minus")), tr("Line Style"));
    auto *group = new QActionGroup(styleMenu);
    const struct {
        BorderStyle style;
        const char *name;
    } styles[] = {{BorderStyle::Thin, QT_TR_NOOP("Thin")},     {BorderStyle::Medium, QT_TR_NOOP("Medium")}, {BorderStyle::Thick, QT_TR_NOOP("Thick")},
                  {BorderStyle::Dashed, QT_TR_NOOP("Dashed")}, {BorderStyle::Dotted, QT_TR_NOOP("Dotted")}, {BorderStyle::Double, QT_TR_NOOP("Double")}, {BorderStyle::Hair, QT_TR_NOOP("Hairline")}};
    for (const auto &s : styles) {
        QAction *a = styleMenu->addAction(tr(s.name));
        a->setCheckable(true);
        a->setActionGroup(group);
        a->setChecked(s.style == m_lineStyle);
        const BorderStyle style = s.style;
        connect(a, &QAction::triggered, this, [this, style] { m_lineStyle = style; });
    }
    menu->addSeparator();
    menu->addAction(tr("More Borders…"), this, [this] { formatCellsDialog(3); });
    return menu;
}

QMenu *CrunchWindow::buildOrientationMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("orientationMenu"));
    for (const char *id : {"fmt.turn.none", "fmt.turn.ccw", "fmt.turn.cw", "fmt.turn.vertical", "fmt.turn.up", "fmt.turn.down"}) menu->addAction(act(QString::fromLatin1(id)));
    menu->addSeparator();
    menu->addAction(act(QStringLiteral("fmt.alignDialog")));
    return menu;
}

// ---------------------------------------------------------------- clear, styles, the painter

// Clear All, Clear Formats, Clear Contents (the Delete key).
void CrunchWindow::clearCommand(int what)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    const CellRange range = selectedRange();
    crunch::EditSession::Outcome o;
    if (what == 0) o = m_session->clearAll(m_sheet, range);
    else if (what == 1) o = m_session->clearFormats(m_sheet, range);
    else {
        clearSelection();
        return;
    }
    if (!o.ok) notice(tr("Clear"), o.problem);
    else if (what == 0) m_calculating = true;
    updateFormatState();
}

void CrunchWindow::applyCellStyle(const QString &name)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    m_formatting = true;
    const crunch::EditSession::Outcome o = m_session->applyStyleNamed(m_sheet, selectedRange(), name, tr("Cell Style"));
    m_formatting = false;
    if (!o.ok) notice(tr("Cell Styles"), o.problem);
    updateFormatState();
}

QMenu *CrunchWindow::buildStylesMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("stylesMenu"));
    connect(menu, &QMenu::aboutToShow, this, [this, menu] {
        menu->clear();
        if (!m_loaded || !m_loaded->book) return;
        int group = -1;
        for (const crunch::StyleSample &s : crunch::styleSamples(m_loaded->book->styles())) {
            if (s.group != group) {
                group = s.group;
                menu->addSection(crunch::styleGroupName(group));
            }
            QAction *a = menu->addAction(styleIconOf(s), s.name);
            const QString name = s.name;
            connect(a, &QAction::triggered, this, [this, name] { applyCellStyle(name); });
        }
    });
    return menu;
}

// The Format Painter: the first click takes the format of the cells selected and waits; the cells selected next are given it (and the painter is done); a second click soon after the first
// (a double click) keeps it on until it is clicked again or Esc is pressed.
void CrunchWindow::formatPainterCommand()
{
    const bool quick = m_painter.mode == PainterMode::Once && m_painter.started.isValid() && m_painter.started.elapsed() <= QApplication::doubleClickInterval();
    if (quick) {
        m_painter.mode = PainterMode::Locked;
        m_view->viewport()->setCursor(Qt::CrossCursor);
        updateFormatState();
        return;
    }
    if (m_painter.mode != PainterMode::Off) {
        cancelFormatPainter();
        return;
    }
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) {
        act(QStringLiteral("fmt.painter"))->setChecked(false);
        return;
    }
    if (!editingAllowed()) {
        act(QStringLiteral("fmt.painter"))->setChecked(false);
        return;
    }
    m_painter.clip = m_session->copyCells(m_sheet, selectedRange(), false);
    if (!m_painter.clip.valid()) {
        act(QStringLiteral("fmt.painter"))->setChecked(false);
        notice(tr("Format Painter"), tr("Those cells are too many to copy the format of."));
        return;
    }
    m_painter.mode = PainterMode::Once;
    m_painter.started.start();
    m_view->viewport()->setCursor(Qt::CrossCursor);
    setStatus(tr("Select the cells to give this format to."));
    updateFormatState();
}

void CrunchWindow::cancelFormatPainter()
{
    if (m_painter.mode == PainterMode::Off) return;
    m_painter.mode = PainterMode::Off;
    m_painter.clip = crunch::ClipData();
    m_view->viewport()->unsetCursor();
    setStatus(tr("Ready"));
    updateFormatState();
}

// The selection was settled with the painter waiting: the cells get the format that was taken.
void CrunchWindow::paintFormats()
{
    if (m_painter.mode == PainterMode::Off || !m_session) return;
    if (m_painter.started.isValid() && m_painter.mode == PainterMode::Once && m_painter.started.elapsed() < 0) return;
    using Options = crunch::EditSession::PasteOptions;
    Options o;
    o.what = Options::What::Formats;
    const crunch::EditSession::Outcome out = m_session->paste(m_sheet, selectedRange(), m_painter.clip, o);
    if (!out.ok) notice(tr("Format Painter"), out.problem);
    if (m_painter.mode == PainterMode::Once) cancelFormatPainter();
    updateFormatState();
}

// ---------------------------------------------------------------- Format Cells

void CrunchWindow::formatCellsDialog(int tab)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    const FormatCellsDialog::Input in = formatInput();
    FormatCellsDialog::Result result;
    if (m_formatAsker) {
        if (!m_formatAsker(in, &result)) return;
    } else {
        FormatCellsDialog dialog(in, this);
        dialog.showTab(tab);
        if (dialog.exec() != QDialog::Accepted) return;
        result = dialog.result();
    }
    applyFormatResult(result);
}

// What the dialog changed, made as one step of Undo: the format of the cells, the lines on the edges of the block, and the merge.
bool CrunchWindow::applyFormatResult(const FormatCellsDialog::Result &result)
{
    if (result.isEmpty()) return true;
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return false;
    if (!editingAllowed()) return false;
    const CellRange range = selectedRange();
    QUndoStack *u = m_session->undoStack();
    FormatCellsDialog::Result r = result;
    const bool hasChange = [&] {
        FormatCellsDialog::Result only;
        only.change = r.change;
        return !only.isEmpty();
    }();
    const int steps = (hasChange ? 1 : 0) + int(r.borders.size()) + (r.merge ? 1 : 0);
    const QString text = tr("Format Cells");
    if (steps > 1) u->beginMacro(text);
    bool ok = true;
    m_formatting = true;
    if (hasChange) {
        const crunch::EditSession::Outcome o = m_session->formatCells(m_sheet, range, r.change, text);
        if (!o.ok) {
            notice(tr("Format Cells"), o.problem);
            ok = false;
        }
    }
    for (const FormatCellsDialog::BorderOp &op : r.borders) {
        if (!ok) break;
        const crunch::EditSession::Outcome o = m_session->setBorders(m_sheet, range, op.edges, op.line, text);
        if (!o.ok) {
            notice(tr("Format Cells"), o.problem);
            ok = false;
        }
    }
    if (ok && r.merge) {
        MergeKind kind = *r.merge ? MergeKind::Merge : MergeKind::Unmerge;
        crunch::EditSession::Outcome o = m_session->mergeCells(m_sheet, range, kind, false);
        if (!o.ok && o.confirm) {
            if (confirm(tr("Merge Cells"), o.problem)) o = m_session->mergeCells(m_sheet, range, kind, true);
            else o.ok = true;
        }
        if (!o.ok) {
            notice(tr("Format Cells"), o.problem);
            ok = false;
        }
    }
    m_formatting = false;
    if (steps > 1) u->endMacro();
    updateFormatState();
    return ok;
}

// ---------------------------------------------------------------- the ribbon's controls

void CrunchWindow::formatRibbonParts(RibbonParts &parts)
{
    parts.widgets[QStringLiteral("fontCombo")] = [this]() -> QWidget * {
        auto *font = new FontCombo();
        font->setObjectName(QStringLiteral("fmt.fontName"));
        connect(font, &FontCombo::familyChosen, this, [this](const QString &family) {
            FormatChange c;
            c.fontName = family;
            formatSelection(c, tr("Font"));
            m_view->setFocus();
        });
        m_fontCombos << font;
        return font;
    };
    parts.widgets[QStringLiteral("sizeCombo")] = [this]() -> QWidget * {
        auto *size = new SizeCombo();
        size->setObjectName(QStringLiteral("fmt.fontSize"));
        connect(size, &SizeCombo::sizeChosen, this, [this](double s) {
            if (s >= 1 && s <= 409) setFontSize(s);
            m_view->setFocus();
        });
        m_sizeCombos << size;
        return size;
    };
    parts.widgets[QStringLiteral("fillColor")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("paint-bucket"), tr("Fill Color"), true, tr("No Fill"));
        b->setObjectName(QStringLiteral("fmt.fillColor"));
        b->setCurrent(ColorRef::rgb(QColor(0xFF, 0xFF, 0x00)));
        connect(b, &ColorButton::colorPicked, this, [this, b](const ColorRef &c) {
            b->setCurrent(c);
            chooseFillColor(c);
        });
        m_fillButton = b;
        return b;
    };
    parts.widgets[QStringLiteral("fontColor")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("baseline"), tr("Font Color"), true, tr("Automatic"));
        b->setObjectName(QStringLiteral("fmt.fontColor"));
        b->setCurrent(ColorRef::rgb(QColor(0xFF, 0x00, 0x00)));
        connect(b, &ColorButton::colorPicked, this, [this, b](const ColorRef &c) {
            b->setCurrent(c);
            chooseFontColor(c);
        });
        m_fontColorButton = b;
        return b;
    };
    parts.widgets[QStringLiteral("numberFormat")] = [this]() -> QWidget * {
        auto *box = new QComboBox();
        box->setObjectName(QStringLiteral("fmt.numberFormat"));
        box->setAccessibleName(tr("Number Format"));
        box->addItems({tr("General"), tr("Number"), tr("Currency"), tr("Accounting"), tr("Short Date"), tr("Long Date"), tr("Time"), tr("Percentage"), tr("Fraction"), tr("Scientific"), tr("Text"),
                       tr("Custom"), tr("More Number Formats…")});
        box->setMinimumContentsLength(14);
        connect(box, qOverload<int>(&QComboBox::activated), this, [this, box](int index) {
            if (index == box->count() - 1) {
                formatCellsDialog(0);
                updateFormatState();
            } else if (index == box->count() - 2) {
                formatCellsDialog(0);
                updateFormatState();
            } else {
                numberFormatCommand(index);
            }
            m_view->setFocus();
        });
        m_numberBox = box;
        return box;
    };
    parts.menus[QStringLiteral("tabColors")] = [this](QWidget *owner) { return buildTabColorMenu(owner); };
    parts.menus[QStringLiteral("unhideSheets")] = [this](QWidget *owner) { return buildUnhideMenu(owner); };
    parts.menus[QStringLiteral("borders")] = [this](QWidget *owner) { return buildBordersMenu(owner); };
    parts.menus[QStringLiteral("orientation")] = [this](QWidget *owner) { return buildOrientationMenu(owner); };
    parts.menus[QStringLiteral("cellStyles")] = [this](QWidget *owner) { return buildStylesMenu(owner); };
    parts.launchers[QStringLiteral("font")] = [this] { formatCellsDialog(2); };
    parts.launchers[QStringLiteral("alignment")] = [this] { formatCellsDialog(1); };
    parts.launchers[QStringLiteral("number")] = [this] { formatCellsDialog(0); };
}

} // namespace jp
