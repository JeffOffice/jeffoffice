// The Table Design and Table Layout tabs: the table commands, the Table Styles gallery, and the boxes that show a table's parts. They show while a
// table, or a cell of one, is selected; the commands themselves are libs/page's (app/tableedit.h), the ones JeffPub's table tools use too.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/tableedit.h"
#include "app/widgets.h"
#include "canvas/canvas.h"
#include "core/deck.h"
#include "io/pptxtablestyles.h"
#include "office/drawingml_table.h"
#include "office/tablestyles.h"

#include <QAction>
#include <QCheckBox>
#include <QMenu>
#include <QPainter>
#include <QToolButton>
#include "app/plaintext.h"

namespace jp {

namespace {

// Where a table's styles come from: the presentation's theme for the colors, and the definitions its file carried for the styles it names.
TableStyles tableStyles(const Document &d)
{
    TableStyles styles;
    const deck::DeckTheme t = deck::themeOf(d);
    for (int i = 0; i < office::Theme::ColorCount; ++i) styles.theme.colors[i] = t.colors[i];
    styles.defs = deck::tableStyleDefsOf(d);
    return styles;
}

// The line Borders draws: Pen Style, Pen Weight, and Pen Color set it (one pen for the program, as the other tabs' pens are).
Stroke &pen()
{
    static Stroke s = Stroke::line(ColorRef::scheme(Main), 1);
    return s;
}

// A sample of a style for its tile in the gallery: a small table, with the header row and the banded rows on.
void paintStyleSample(QPainter *p, const QRectF &rc, const QString &styleId, const TableStyles &styles, const ColorScheme &colors)
{
    TableItem sample;
    sample.rows = 4;
    sample.cols = 3;
    sample.header = sample.banded = true;
    office::TableStyleEffect fx;
    p->fillRect(rc, Qt::white);
    if (!office::tableStyleEffect(styleId, sample, styles.theme, &fx, &styles.defs)) return;
    const QRectF area = rc.adjusted(3, 3, -3, -3);
    const double w = area.width() / sample.cols, h = area.height() / sample.rows;
    for (int r = 0; r < sample.rows; ++r)
        for (int c = 0; c < sample.cols; ++c) {
            const QRectF cell(area.left() + c * w, area.top() + r * h, w, h);
            const Fill &fill = fx.fill[size_t(r) * sample.cols + c];
            if (!fill.isNone()) p->fillRect(cell, fill.brush(cell, colors));
            // A word in each cell: a short line of the text's color.
            const QColor ink = fx.color[size_t(r) * sample.cols + c].isValid() ? fx.color[size_t(r) * sample.cols + c] : styles.theme.colors[office::Theme::Dark1];
            p->setPen(QPen(ink, 1.2));
            p->drawLine(QPointF(cell.left() + w * 0.2, cell.center().y()), QPointF(cell.left() + w * 0.7, cell.center().y()));
        }
    const auto line = [&](const Stroke &s, const QPointF &a, const QPointF &b) {
        if (s.isNone()) return;
        QPen pn = s.pen(colors);
        pn.setWidthF(std::clamp(s.width * 0.6, 0.5, 2.0));
        p->setPen(pn);
        p->drawLine(a, b);
    };
    for (int r = 0; r <= sample.rows; ++r)
        for (int c = 0; c < sample.cols; ++c) line(fx.above(r, c), QPointF(area.left() + c * w, area.top() + r * h), QPointF(area.left() + (c + 1) * w, area.top() + r * h));
    for (int r = 0; r < sample.rows; ++r)
        for (int c = 0; c <= sample.cols; ++c) line(fx.leftOf(r, c), QPointF(area.left() + c * w, area.top() + r * h), QPointF(area.left() + c * w, area.top() + (r + 1) * h));
}

} // namespace

void DeckWindow::createTableActions()
{
    Editor *ed = m_ed;
    mk("tbl.insAbove", tr("Insert Above"), "between-horizontal-end", QKeySequence(), [ed] { insertTableRows(ed, false); });
    mk("tbl.insBelow", tr("Insert Below"), "between-horizontal-start", QKeySequence(), [ed] { insertTableRows(ed, true); });
    mk("tbl.insLeft", tr("Insert Left"), "between-vertical-end", QKeySequence(), [ed] { insertTableColumns(ed, false); });
    mk("tbl.insRight", tr("Insert Right"), "between-vertical-start", QKeySequence(), [ed] { insertTableColumns(ed, true); });
    mk("tbl.delRow", tr("Delete Rows"), "", QKeySequence(), [ed] { deleteTableRows(ed); });
    mk("tbl.delCol", tr("Delete Columns"), "", QKeySequence(), [ed] { deleteTableColumns(ed); });
    mk("tbl.delTable", tr("Delete Table"), "", QKeySequence(), [ed] { deleteTable(ed); });
    mk("tbl.merge", tr("Merge Cells"), "table-cells-merge", QKeySequence(), [ed] { mergeTableCells(ed); });
    mk("tbl.split", tr("Split Cells"), "table-cells-split", QKeySequence(), [ed] { splitTableCells(ed); });
    mk("tbl.selectTable", tr("Select Table"), "", QKeySequence(), [ed] { selectTable(ed); });
    mk("tbl.selectCell", tr("Select Cell"), "", QKeySequence(), [ed] { selectTableCell(ed); });
    mk("tbl.selectRow", tr("Select Row"), "", QKeySequence(), [ed] { selectTableRow(ed); });
    mk("tbl.selectCol", tr("Select Column"), "", QKeySequence(), [ed] { selectTableColumn(ed); });
    mk("tbl.distributeRows", tr("Distribute Rows"), "", QKeySequence(), [ed] { distributeTableRows(ed); });
    mk("tbl.distributeCols", tr("Distribute Columns"), "", QKeySequence(), [ed] { distributeTableColumns(ed); });

    // Alignment: across with the text's own alignment commands; up and down are the cells'.
    const struct { const char *id; const char *icon; VAlign align; QString text; } aligns[] = {
        {"tbl.alignTop", "align-vertical-justify-start", VAlign::Top, tr("Align Top")},
        {"tbl.alignMiddle", "align-vertical-justify-center", VAlign::Middle, tr("Center Vertically")},
        {"tbl.alignBottom", "align-vertical-justify-end", VAlign::Bottom, tr("Align Bottom")}};
    for (const auto &a : aligns) {
        const VAlign align = a.align;
        mk(QString::fromLatin1(a.id), a.text, a.icon, QKeySequence(), [ed, align] { setTableCellAlignment(ed, align); }, true);
    }
    mk("tbl.direction", tr("Text Direction"), "arrow-down-wide-narrow", QKeySequence(), [ed] { toggleTableTextDirection(ed); }, true);
    const struct { const char *id; QString text; double sides, ends; } margins[] = {
        {"tbl.marginNone", tr("None"), 0, 0}, {"tbl.marginNarrow", tr("Narrow"), 2.88, 2.88}, {"tbl.marginNormal", tr("Normal"), 7.2, 3.6},
        {"tbl.marginModerate", tr("Moderate"), 7.2, 7.2}, {"tbl.marginWide", tr("Wide"), 14.4, 7.2}};
    for (const auto &m : margins) {
        const double sides = m.sides, ends = m.ends;
        mk(QString::fromLatin1(m.id), m.text, "", QKeySequence(), [ed, sides, ends] { setTableCellMargins(ed, QMarginsF(sides, ends, sides, ends)); });
    }

    // Table Style Options: each turns one part of the table's style on or off, and leaves what was done by hand.
    const struct { const char *id; QString text; bool TableStyleParts::*part; } options[] = {
        {"tbl.optHeader", tr("Header Row"), &TableStyleParts::header}, {"tbl.optTotal", tr("Total Row"), &TableStyleParts::totalRow},
        {"tbl.optBanded", tr("Banded Rows"), &TableStyleParts::banded}, {"tbl.optFirstCol", tr("First Column"), &TableStyleParts::firstCol},
        {"tbl.optLastCol", tr("Last Column"), &TableStyleParts::lastCol}, {"tbl.optBandedCols", tr("Banded Columns"), &TableStyleParts::bandedCols}};
    for (const auto &o : options) {
        auto part = o.part;
        mk(QString::fromLatin1(o.id), o.text, "", QKeySequence(), [this, ed, part] {
            TableItem *t = selectedTable(ed);
            if (!t) return;
            TableStyleParts parts = TableStyleParts::of(*t);
            parts.*part = !(parts.*part);
            setTableStyle(ed, t, t->format, parts, tableStyles(*ed->doc()), false);
        }, true);
    }

    // Borders: which edges the pen's line is drawn on.
    const struct { const char *id; QString text; BorderSides sides; } borders[] = {
        {"border.all", tr("All Borders"), BorderSides::All}, {"border.outside", tr("Outside Borders"), BorderSides::Outside},
        {"border.inside", tr("Inside Borders"), BorderSides::Inside}, {"border.insideH", tr("Inside Horizontal Border"), BorderSides::InsideHorizontal},
        {"border.insideV", tr("Inside Vertical Border"), BorderSides::InsideVertical}, {"border.none", tr("No Border"), BorderSides::None},
        {"border.top", tr("Top Border"), BorderSides::Top}, {"border.bottom", tr("Bottom Border"), BorderSides::Bottom},
        {"border.left", tr("Left Border"), BorderSides::Left}, {"border.right", tr("Right Border"), BorderSides::Right}};
    for (const auto &b : borders) {
        const BorderSides sides = b.sides;
        mk(QString::fromLatin1(b.id), b.text, "", QKeySequence(), [ed, sides] { setTableBorders(ed, sides, pen()); });
    }

    // Rows and columns that come and go keep the table in its style: the bands go on, and a new row has its neighbors' look.
    m_ed->tableEdited = [this](TableItem *t, const Editor::TableEdit &edit) { keepTableStyle(m_ed, t, edit, tableStyles(*m_ed->doc())); };

    auto sync = [this] { refreshTable(); };
    connect(m_ed, &Editor::selectionChanged, this, sync);
    connect(m_ed, &Editor::changed, this, sync);
    connect(m_ed, &Editor::textCursorChanged, this, sync);
    connect(m_ed, &Editor::pageChanged, this, sync);
    connect(m_ed, &Editor::documentReplaced, this, sync);
    refreshTable();
}

// A table just made has the style the presentation program gives a new one: Medium Style 2, Accent 1, with its header row and banded rows.
void DeckWindow::styleNewTable(TableItem *table)
{
    giveTableStyle(m_ed, table, office::defaultTableStyleId(), TableStyleParts(), tableStyles(*m_ed->doc()), true);
}

void DeckWindow::applyTableStyle(const QString &styleId)
{
    TableItem *t = selectedTable(m_ed);
    if (t) setTableStyle(m_ed, t, styleId, TableStyleParts::of(*t), tableStyles(*m_ed->doc()), true);
}

void DeckWindow::setCellShading(const ColorRef &color) { setTableShading(m_ed, color); }

// The tabs show while a table, or a cell of one, is selected; the boxes and buttons say what the selected cells are.
void DeckWindow::refreshTable()
{
    TableItem *t = selectedTable(m_ed);
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Table Tools"), t != nullptr);
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("tbl.")) || a->objectName().startsWith(QLatin1String("border."))) a->setEnabled(t != nullptr);
    if (!t) return;
    const TableStyleParts parts = TableStyleParts::of(*t);
    const struct { const char *id; bool on; } options[] = {{"tbl.optHeader", parts.header}, {"tbl.optTotal", parts.totalRow}, {"tbl.optBanded", parts.banded},
                                                           {"tbl.optFirstCol", parts.firstCol}, {"tbl.optLastCol", parts.lastCol}, {"tbl.optBandedCols", parts.bandedCols}};
    for (const auto &o : options) act(QString::fromLatin1(o.id))->setChecked(o.on);
    // Vertical alignment and text direction are on for the selected cells when each of them has it (with only the table selected, every cell).
    const CellRange rg = m_ed->cellsToFormat(t);
    bool turned = rg.valid();
    int align[3] = {0, 0, 0}, count = 0;
    for (int r = rg.r0; r <= rg.r1 && rg.valid(); ++r)
        for (int c = rg.c0; c <= rg.c1; ++c) {
            turned = turned && t->cell(r, c).vertical;
            ++align[int(t->cell(r, c).valign)];
            ++count;
        }
    act("tbl.direction")->setChecked(turned);
    const char *ids[3] = {"tbl.alignTop", "tbl.alignMiddle", "tbl.alignBottom"};
    for (int k = 0; k < 3; ++k) act(QString::fromLatin1(ids[k]))->setChecked(count > 0 && align[k] == count);
    for (MeasureSpin *s : m_tableSizeBoxes) {
        const bool row = s->objectName() == QLatin1String("tableRowHeight");
        const QSignalBlocker block(s);
        s->setEnabled(!t->locked);
        if (rg.valid()) s->setPoints(row ? t->rowH[rg.r0] : t->colW[rg.c0]);
    }
}

// The Table Design and Table Layout tabs' controls.
void DeckWindow::addTableParts(RibbonParts &parts)
{
    // The Table Style Options boxes, each one a command's.
    for (const char *id : {"tbl.optHeader", "tbl.optTotal", "tbl.optBanded", "tbl.optFirstCol", "tbl.optLastCol", "tbl.optBandedCols"}) {
        const QString command = QString::fromLatin1(id);
        parts.widgets[command] = [this, command]() -> QWidget * {
            QAction *a = act(command);
            auto *box = new QCheckBox(a->text());
            box->setObjectName(command);
            box->setToolTip(jp::plainToolTip(a->text()));
            box->setChecked(a->isChecked());
            connect(box, &QCheckBox::clicked, a, [a] { a->trigger(); });
            connect(a, &QAction::toggled, box, [box](bool on) {
                const QSignalBlocker block(box);
                box->setChecked(on);
            });
            box->setEnabled(a->isEnabled());
            connect(a, &QAction::enabledChanged, box, &QWidget::setEnabled);
            return box;
        };
    }

    parts.widgets[QStringLiteral("tableStyles")] = [this]() -> QWidget * {
        auto *gal = new Gallery(QSize(54, 42), 6);
        gal->setObjectName(QStringLiteral("tableStyles"));
        gal->setTitle(tr("Table Styles"));
        gal->setItemsProvider([this] {
            QVector<GalleryItem> v;
            const TableStyles styles = tableStyles(*m_ed->doc());
            const ColorScheme colors = m_ed->doc()->colors;
            for (const office::TableStyleInfo &s : office::tableStyleList()) {
                const QString id = s.id;
                const QString group = s.group == QLatin1String("Light") ? tr("Light") : s.group == QLatin1String("Medium") ? tr("Medium")
                                      : s.group == QLatin1String("Dark") ? tr("Dark") : tr("Best Match for Document");
                v << GalleryItem{id, s.name, drawnIcon([id, styles, colors](QPainter *p, const QRectF &rc) { paintStyleSample(p, rc, id, styles, colors); }), group};
            }
            return v;
        });
        connect(gal, &Gallery::activated, this, [this](const QString &id) {
            applyTableStyle(id);
            m_canvas->setFocus();
        });
        return gal;
    };

    parts.widgets[QStringLiteral("cellShading")] = [this]() -> QWidget * {
        auto *shading = new ColorButton("paint-bucket", tr("Shading"), true, tr("No Fill"));
        shading->setText(tr("Shading"));
        shading->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        connect(shading, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
            setCellShading(c);
            m_canvas->setFocus();
        });
        m_colorButtons << shading;
        return shading;
    };

    // Draw Borders: the pen's color, weight, and style; Borders draws with it.
    parts.widgets[QStringLiteral("penColor")] = [this]() -> QWidget * {
        auto *color = new ColorButton("pen-line", tr("Pen Color"), false, QString());
        color->setText(tr("Pen Color"));
        color->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        color->setCurrent(pen().color);
        connect(color, &ColorButton::colorPicked, this, [](const ColorRef &c) { pen().color = c; });
        m_colorButtons << color;
        return color;
    };
    parts.widgets[QStringLiteral("penWeight")] = [this]() -> QWidget * {
        auto *menu = new QMenu(this);
        for (double w : {0.25, 0.5, 0.75, 1.0, 1.5, 2.25, 3.0, 4.5, 6.0}) {
            QAction *a = menu->addAction(tr("%1 pt").arg(w));
            connect(a, &QAction::triggered, this, [w] { pen().width = w; });
        }
        auto *b = new QToolButton();
        b->setText(tr("Pen Weight"));
        b->setToolTip(tr("Pen Weight: how thick the lines are that Borders draws."));
        b->setIcon(icon("minus"));
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        b->setPopupMode(QToolButton::InstantPopup);
        b->setMenu(menu);
        b->setAutoRaise(true);
        return b;
    };
    parts.widgets[QStringLiteral("penStyle")] = [this]() -> QWidget * {
        auto *menu = new QMenu(this);
        const struct { QString name; Stroke::Dash dash; } styles[] = {
            {tr("Solid"), Stroke::SolidLine}, {tr("Round Dot"), Stroke::RoundDot}, {tr("Square Dot"), Stroke::SquareDot}, {tr("Dash"), Stroke::DashLine},
            {tr("Dash Dot"), Stroke::DashDot}, {tr("Long Dash"), Stroke::LongDash}, {tr("Long Dash Dot"), Stroke::LongDashDot}, {tr("Long Dash Dot Dot"), Stroke::LongDashDotDot}};
        for (const auto &s : styles) {
            const Stroke::Dash dash = s.dash;
            QAction *a = menu->addAction(s.name);
            connect(a, &QAction::triggered, this, [dash] { pen().dash = dash; });
        }
        auto *b = new QToolButton();
        b->setText(tr("Pen Style"));
        b->setToolTip(tr("Pen Style: solid or dashed lines for Borders."));
        b->setIcon(icon("minus"));
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        b->setPopupMode(QToolButton::InstantPopup);
        b->setMenu(menu);
        b->setAutoRaise(true);
        return b;
    };

    // Table Layout > Cell Size: the height of the rows, and the width of the columns, of the selected cells.
    for (const bool row : {true, false}) {
        parts.widgets[QString::fromLatin1(row ? "tableRowHeight" : "tableColumnWidth")] = [this, row]() -> QWidget * {
            auto *s = new MeasureSpin();
            s->setObjectName(QString::fromLatin1(row ? "tableRowHeight" : "tableColumnWidth"));
            s->setToolTip(row ? tr("Row Height") : tr("Column Width"));
            s->setAccessibleName(s->toolTip());
            s->setKeyboardTracking(false);
            m_tableSizeBoxes << s;
            connect(s, &QDoubleSpinBox::valueChanged, this, [this, row](double v) {
                if (v <= 0) return;
                if (row) setTableRowHeight(m_ed, v);
                else setTableColumnWidth(m_ed, v);
            });
            return s;
        };
    }
}

} // namespace jp
