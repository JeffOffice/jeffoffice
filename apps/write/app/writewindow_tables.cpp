// JeffWrite's tables: the commands of the Table Design and Table Layout tabs and Insert > Table, the menu of a right click in a cell, and what keeps their controls in step
// with the cursor (the work on the document is core/tableops.h).

#include "app/writewindow.h"

#include "app/icons.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/tabledialogs.h"
#include "app/widgets.h"
#include "core/fieldops.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "wml/wml.h"

#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QStatusBar>
#include <QWidgetAction>

namespace jp {

namespace {

// A picture of a table style for the gallery: its header row, its banded rows, and its lines, as the style says them.
QIcon tableStyleIcon(const QString &id, const office::wml::Style &style, const QSize &size, qreal dpr)
{
    const office::wml::TableStyleDef def = office::wml::readTableStyle(style);
    auto color = [](const QString &hex, const QColor &fallback) {
        bool ok = false;
        const uint v = hex.toUInt(&ok, 16);
        return hex.size() == 6 && ok ? QColor((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF) : fallback;
    };
    const QColor lineColor = color(def.base.tbl.top.color, color(def.base.tbl.insideH.color, QColor(0x80, 0x80, 0x80)));
    const bool lines = def.base.tbl.top.stated() || def.base.tbl.insideH.stated();
    const bool grid = def.base.tbl.insideV.stated();
    const QColor header = color(def.parts.value(QStringLiteral("firstRow")).cell.shdFill, QColor());
    const QColor band = color(def.parts.value(QStringLiteral("band1Horz")).cell.shdFill, color(def.parts.value(QStringLiteral("band1Vert")).cell.shdFill, QColor()));
    QPixmap pm(size * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, false);
    const QRectF box = QRectF(QPointF(0, 0), QSizeF(size)).adjusted(4, 4, -4, -4);
    p.setPen(QPen(QColor(0, 0, 0, 40), 1));
    p.setBrush(Qt::white);
    p.drawRect(QRectF(QPointF(0.5, 0.5), QSizeF(size) - QSizeF(1, 1)));
    const int rows = 4, columns = 3;
    const double rh = box.height() / rows, cw = box.width() / columns;
    for (int r = 0; r < rows; ++r) {
        QColor fill;
        if (r == 0 && header.isValid()) fill = header;
        else if (r > 0 && band.isValid() && (r % 2) == 1) fill = band;
        if (fill.isValid()) p.fillRect(QRectF(box.left(), box.top() + r * rh, box.width(), rh), fill);
    }
    p.setPen(QPen(lines || grid ? lineColor : QColor(0xC0, 0xC0, 0xC0), 1));
    for (int r = 0; r <= rows; ++r)
        if (lines || grid || r == 0 || r == 1) p.drawLine(QPointF(box.left(), box.top() + r * rh), QPointF(box.right(), box.top() + r * rh));
    if (grid)
        for (int c = 0; c <= columns; ++c) p.drawLine(QPointF(box.left() + c * cw, box.top()), QPointF(box.left() + c * cw, box.bottom()));
    Q_UNUSED(id);
    return QIcon(pm);
}

} // namespace

// ---- the commands

void WriteWindow::tableCommand(const std::function<void(write::WriteDoc &, QTextCursor &)> &fn)
{
    if (!m_doc || refuseIfReadOnly()) return;
    m_doc->tableNotice.clear();
    fn(*m_doc, m_view->cursor());
    if (!m_doc->tableNotice.isEmpty()) statusBar()->showMessage(m_doc->tableNotice, 8000);   // (what a command that did nothing says of why)
    m_view->refresh();
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::insertTableNow(const write::NewTable &table)
{
    if (!m_doc || refuseIfReadOnly()) return;
    m_doc->tableNotice.clear();
    if (!write::insertTable(*m_doc, m_view->cursor(), table)) {
        statusBar()->showMessage(m_doc->tableNotice.isEmpty() ? tr("A table can't be put in a table cell here.") : m_doc->tableNotice, 5000);
        return;
    }
    m_view->refresh();
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::createTableActions()
{
    // Ready-Made Tables: tables with their words in place.
    const struct { const char *id; const char *text; const char *icon; write::ReadyTable kind; } ready[] = {
        {"insert.ready.calendar", QT_TR_NOOP("Calendar"), "calendar-range", write::ReadyTable::Calendar},    {"insert.ready.matrix", QT_TR_NOOP("Matrix"), "grid-3x3", write::ReadyTable::Matrix},
        {"insert.ready.list", QT_TR_NOOP("Tabular List"), "table-properties", write::ReadyTable::List},            {"insert.ready.schedule", QT_TR_NOOP("Weekly Schedule"), "table", write::ReadyTable::Schedule}};
    for (const auto &q : ready) {
        const write::ReadyTable kind = q.kind;
        mk(QString::fromLatin1(q.id), tr(q.text), q.icon, QKeySequence(), [this, kind] {
            if (!m_doc) return;
            m_doc->tableNotice.clear();
            if (!write::insertReadyTable(*m_doc, m_view->cursor(), kind)) {
                statusBar()->showMessage(m_doc->tableNotice.isEmpty() ? tr("A table can't be put in a table cell here.") : m_doc->tableNotice, 5000);
                return;
            }
            m_view->refresh();
            m_view->setFocus(Qt::OtherFocusReason);
        });
    }
    mk("insert.table", tr("Insert Table…"), "table", QKeySequence(), [this] {
        write::NewTable table;
        if (!write::insertTableDialog(this, &table)) return;
        insertTableNow(table);
    });
    auto cmd = [this](const QString &id, const QString &text, const char *iconName, const std::function<void(write::WriteDoc &, QTextCursor &)> &fn) {
        mk(id, text, iconName, QKeySequence(), [this, fn] { tableCommand(fn); });
    };
    // Rows and columns.
    cmd(QStringLiteral("table.insertAbove"), tr("Insert Above"), "between-vertical-start", [](write::WriteDoc &d, QTextCursor &c) { write::insertTableRow(d, c, false); });
    cmd(QStringLiteral("table.insertBelow"), tr("Insert Below"), "between-vertical-end", [](write::WriteDoc &d, QTextCursor &c) { write::insertTableRow(d, c, true); });
    cmd(QStringLiteral("table.insertLeft"), tr("Insert Left"), "between-horizontal-start", [](write::WriteDoc &d, QTextCursor &c) { write::insertTableColumn(d, c, false); });
    cmd(QStringLiteral("table.insertRight"), tr("Insert Right"), "between-horizontal-end", [](write::WriteDoc &d, QTextCursor &c) { write::insertTableColumn(d, c, true); });
    cmd(QStringLiteral("table.deleteCells"), tr("Delete Cells, Shift Left"), "grid-2x2-x", [](write::WriteDoc &d, QTextCursor &c) { write::deleteTableCellsShiftLeft(d, c); });
    cmd(QStringLiteral("table.deleteRows"), tr("Delete Rows"), "grid-2x2-x", [](write::WriteDoc &d, QTextCursor &c) { write::deleteTableRows(d, c); });
    cmd(QStringLiteral("table.deleteColumns"), tr("Delete Columns"), "table-columns-split", [](write::WriteDoc &d, QTextCursor &c) { write::deleteTableColumns(d, c); });
    cmd(QStringLiteral("table.deleteTable"), tr("Delete Table"), "trash-2", [](write::WriteDoc &d, QTextCursor &c) { write::deleteTable(d, c); });
    cmd(QStringLiteral("table.selectTable"), tr("Select Table"), "table", [](write::WriteDoc &d, QTextCursor &c) { write::selectInTable(d, c, write::TableUnit::Table); });
    cmd(QStringLiteral("table.selectRow"), tr("Select Row"), "rows-2", [](write::WriteDoc &d, QTextCursor &c) { write::selectInTable(d, c, write::TableUnit::Row); });
    cmd(QStringLiteral("table.selectColumn"), tr("Select Column"), "columns-2", [](write::WriteDoc &d, QTextCursor &c) { write::selectInTable(d, c, write::TableUnit::Column); });
    cmd(QStringLiteral("table.selectCell"), tr("Select Cell"), "square", [](write::WriteDoc &d, QTextCursor &c) { write::selectInTable(d, c, write::TableUnit::Cell); });
    // Merge.
    cmd(QStringLiteral("table.merge"), tr("Merge Cells"), "table-cells-merge", [](write::WriteDoc &d, QTextCursor &c) { write::mergeCells(d, c); });
    mk("table.split", tr("Split Cells…"), "table-cells-split", QKeySequence(), [this] {
        if (!m_doc) return;
        int columns = 2, rows = 1;
        if (!write::splitCellsDialog(this, &columns, &rows, 50)) return;
        tableCommand([=](write::WriteDoc &d, QTextCursor &c) { write::splitCells(d, c, columns, rows); });
    });
    // Size.
    cmd(QStringLiteral("table.autofitContents"), tr("Fit to Contents"), "unfold-horizontal", [](write::WriteDoc &d, QTextCursor &c) { write::fitTable(d, c, write::TableFit::Contents); });
    cmd(QStringLiteral("table.autofitWindow"), tr("Fit to Window"), "move-horizontal", [](write::WriteDoc &d, QTextCursor &c) { write::fitTable(d, c, write::TableFit::Window); });
    cmd(QStringLiteral("table.fixedWidth"), tr("Fixed Column Width"), "panel-left-dashed", [](write::WriteDoc &d, QTextCursor &c) { write::fitTable(d, c, write::TableFit::Fixed); });
    cmd(QStringLiteral("table.distributeRows"), tr("Distribute Rows"), "align-vertical-distribute-center", [](write::WriteDoc &d, QTextCursor &c) { write::distributeRows(d, c); });
    cmd(QStringLiteral("table.distributeColumns"), tr("Distribute Columns"), "align-horizontal-distribute-center", [](write::WriteDoc &d, QTextCursor &c) { write::distributeColumns(d, c); });
    // The nine places of text in a cell.
    struct Place { const char *id; const char *text; const char *iconName; Qt::Alignment across; flow::CellAlign down; };
    const Place places[] = {{"tl", QT_TR_NOOP("Align Top Left"), "move-up-left", Qt::AlignLeft, flow::CellAlign::Top},
                            {"tc", QT_TR_NOOP("Align Top Center"), "move-up", Qt::AlignHCenter, flow::CellAlign::Top},
                            {"tr", QT_TR_NOOP("Align Top Right"), "move-up-right", Qt::AlignRight, flow::CellAlign::Top},
                            {"ml", QT_TR_NOOP("Align Center Left"), "move-left", Qt::AlignLeft, flow::CellAlign::Center},
                            {"mc", QT_TR_NOOP("Align Center"), "crosshair", Qt::AlignHCenter, flow::CellAlign::Center},
                            {"mr", QT_TR_NOOP("Align Center Right"), "move-right", Qt::AlignRight, flow::CellAlign::Center},
                            {"bl", QT_TR_NOOP("Align Bottom Left"), "move-down-left", Qt::AlignLeft, flow::CellAlign::Bottom},
                            {"bc", QT_TR_NOOP("Align Bottom Center"), "move-down", Qt::AlignHCenter, flow::CellAlign::Bottom},
                            {"br", QT_TR_NOOP("Align Bottom Right"), "move-down-right", Qt::AlignRight, flow::CellAlign::Bottom}};
    for (const Place &p : places) {
        const Qt::Alignment across = p.across;
        const flow::CellAlign down = p.down;
        cmd(QStringLiteral("table.align.") + QLatin1String(p.id), tr(p.text), p.iconName, [across, down](write::WriteDoc &d, QTextCursor &c) { write::setCellAlignment(d, c, across, down); });
    }
    // Borders: the sides of the cells selected, in a single line (or none).
    struct Border { const char *id; const char *text; const char *iconName; int sides; bool none; };
    const Border borders[] = {{"bottom", QT_TR_NOOP("Bottom Border"), "panel-bottom", write::BorderSide::Bottom, false},
                              {"top", QT_TR_NOOP("Top Border"), "panel-top", write::BorderSide::Top, false},
                              {"left", QT_TR_NOOP("Left Border"), "panel-left", write::BorderSide::Left, false},
                              {"right", QT_TR_NOOP("Right Border"), "panel-right", write::BorderSide::Right, false},
                              {"none", QT_TR_NOOP("No Border"), "square-dashed", write::BorderSide::All, true},
                              {"all", QT_TR_NOOP("All Borders"), "grid-2x2", write::BorderSide::All, false},
                              {"outside", QT_TR_NOOP("Outside Borders"), "square", write::BorderSide::Outside, false},
                              {"inside", QT_TR_NOOP("Inside Borders"), "grid-3x3", write::BorderSide::Inside, false},
                              {"insideH", QT_TR_NOOP("Inside Horizontal Border"), "between-vertical-end", write::BorderSide::InsideH, false},
                              {"insideV", QT_TR_NOOP("Inside Vertical Border"), "between-horizontal-end", write::BorderSide::InsideV, false}};
    for (const Border &b : borders) {
        const int sides = b.sides;
        const bool none = b.none;
        cmd(QStringLiteral("table.border.") + QLatin1String(b.id), tr(b.text), b.iconName, [sides, none](write::WriteDoc &d, QTextCursor &c) {
            flow::TableLine line;
            if (!none) {
                line.style = flow::TableLine::Style::Single;
                line.width = 0.5;
            }
            write::setBorders(d, c, sides, line);
        });
    }
    mk("table.bordersShading", tr("Borders and Shading…"), "paint-bucket", QKeySequence(), [this] {
        if (!m_doc) return;
        write::bordersShadingDialog(this, m_doc.get(), &m_view->cursor());
        m_view->refresh();
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("table.properties", tr("Table Properties…"), "table-properties", QKeySequence(), [this] {
        if (!m_doc) return;
        write::tablePropertiesDialog(this, m_doc.get(), &m_view->cursor());
        m_view->refresh();
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("table.style.new", tr("New Table Style…"), "plus", QKeySequence(), [this] {
        if (!m_doc) return;
        QString made;
        if (write::tableStyleDialog(this, m_doc.get(), QString(), &made) && !made.isEmpty() && cursorInTable()) tableCommand([made](write::WriteDoc &d, QTextCursor &c) { write::setTableStyle(d, c, made); });
        m_view->refresh();
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("table.style.modify", tr("Modify Table Style…"), "pencil", QKeySequence(), [this] {
        if (!m_doc || !cursorInTable()) return;
        const QString style = write::tablePropsAt(*m_doc, m_view->cursor()).style;
        if (style.isEmpty() || style == QLatin1String("TableNormal")) return;
        write::tableStyleDialog(this, m_doc.get(), style);
        m_view->refresh();
        m_view->setFocus(Qt::OtherFocusReason);
    });
    // The style options.
    struct Option { const char *id; const char *text; int bit; bool inverted; };
    const Option options[] = {{"header", QT_TR_NOOP("Header Row"), 0x20, false},        {"total", QT_TR_NOOP("Total Row"), 0x40, false},
                              {"firstColumn", QT_TR_NOOP("First Column"), 0x80, false}, {"lastColumn", QT_TR_NOOP("Last Column"), 0x100, false},
                              {"bandedRows", QT_TR_NOOP("Banded Rows"), 0x200, true},   {"bandedColumns", QT_TR_NOOP("Banded Columns"), 0x400, true}};
    for (const Option &o : options) {
        const QString id = QStringLiteral("table.look.") + QLatin1String(o.id);
        const int bit = o.bit;
        const bool inverted = o.inverted;
        mk(id, tr(o.text), "", QKeySequence(), [this, id, bit, inverted] { tableCommand([=](write::WriteDoc &d, QTextCursor &c) { write::setTableLook(d, c, bit, inverted ? !act(id)->isChecked() : act(id)->isChecked()); }); }, true);
    }
    // (A new table has these three on; they are read from the table when a tab that shows them is in front.)
    for (const char *id : {"table.look.header", "table.look.firstColumn", "table.look.bandedRows"}) act(QString::fromLatin1(id))->setChecked(true);
    // Convert, split, sort, header rows, formula, and gridlines.
    mk("insert.convertTextToTable", tr("Convert Text to Table…"), "table", QKeySequence(), [this] {
        if (!m_doc) return;
        write::TextSeparator separator = write::TextSeparator::Tabs;
        QChar other;
        int columns = 1;
        if (!write::convertTextToTableDialog(this, &separator, &other, &columns)) return;
        tableCommand([=](write::WriteDoc &d, QTextCursor &c) {
            if (!write::convertTextToTable(d, c, separator, other, columns) && d.tableNotice.isEmpty()) d.tableNotice = tr("Text that is in a table can't be made into a table.");
        });
    });
    mk("table.convertToText", tr("Convert to Text…"), "file-text", QKeySequence(), [this] {
        if (!m_doc) return;
        write::TextSeparator separator = write::TextSeparator::Tabs;
        QChar other;
        if (!write::convertTableToTextDialog(this, &separator, &other)) return;
        tableCommand([=](write::WriteDoc &d, QTextCursor &c) { write::convertTableToText(d, c, separator, other); });
    });
    cmd(QStringLiteral("table.splitTable"), tr("Split Table"), "square-split-horizontal", [](write::WriteDoc &d, QTextCursor &c) { write::splitTable(d, c); });
    mk("table.sort", tr("Sort…"), "arrow-down-a-z", QKeySequence(), [this] {
        if (!m_doc) return;
        const write::CellPlace place = write::cellPlace(*m_doc, m_view->cursor());
        if (!place.inTable) return;
        QVector<write::TableSortKey> keys(1);
        keys[0].column = place.column;
        bool header = write::tablePropsAt(*m_doc, m_view->cursor()).look & 0x20;
        if (!write::sortTableDialog(this, place.columns, &keys, &header)) return;
        tableCommand([=](write::WriteDoc &d, QTextCursor &c) { write::sortTableRows(d, c, keys, header); });
    });
    mk("table.repeatHeader", tr("Repeat Header Rows"), "heading", QKeySequence(), [this] {
        tableCommand([this](write::WriteDoc &d, QTextCursor &c) {
            const write::TableProps was = write::tablePropsAt(d, c);
            write::TableProps now = was;
            now.header = act("table.repeatHeader")->isChecked();
            write::setTableProps(d, c, was, now);
        });
    }, true);
    mk("table.formula", tr("Formula…"), "sigma", QKeySequence(), [this] {
        if (!m_doc) return;
        QTextCursor &cur = m_view->cursor();
        if (!write::cellPlace(*m_doc, cur).inTable) return;
        const auto has = [&](const char *name) {
            const auto values = write::tableCellValues(*m_doc, cur.position(), QString::fromLatin1(name));
            return values && !values->isEmpty();
        };
        QString formula = has("ABOVE") || !has("LEFT") ? QStringLiteral("SUM(ABOVE)") : QStringLiteral("SUM(LEFT)"), picture;
        if (!write::tableFormulaDialog(this, &formula, &picture)) return;
        QString code = QStringLiteral("= ") + formula;
        if (!picture.isEmpty()) code += QStringLiteral(" \\# \"") + picture + QLatin1Char('"');
        m_doc->tableNotice.clear();
        if (!write::insertField(*m_doc, cur, code)) return;
        write::updateFieldsAt(*m_doc, cur);
        m_view->refresh();
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("table.gridlines", tr("View Gridlines"), "grid-3x3", QKeySequence(), [this] { m_view->setShowGridlines(act("table.gridlines")->isChecked()); }, true);
    act("table.gridlines")->setEnabled(false);   // (until the caret is in a table)
    cmd(QStringLiteral("table.style.clear"), tr("Clear"), "eraser", [](write::WriteDoc &d, QTextCursor &c) { write::setTableStyle(d, c, QStringLiteral("TableNormal")); });
}

// ---- the ribbon's parts

void WriteWindow::tableRibbonParts(RibbonParts &parts)
{
    // Insert > Table: the grid to point at, and the dialog.
    parts.menus[QStringLiteral("tableGrid")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *grid = new TableGrid();
        auto *wa = new QWidgetAction(m);
        wa->setDefaultWidget(grid);
        m->addAction(wa);
        m->addAction(act("insert.table"));
        m->addAction(act("insert.convertTextToTable"));
        auto *readyMenu = m->addMenu(tr("Ready-Made Tables"));
        for (const char *id : {"insert.ready.calendar", "insert.ready.matrix", "insert.ready.list", "insert.ready.schedule"}) readyMenu->addAction(act(QString::fromLatin1(id)));
        connect(grid, &TableGrid::picked, this, [this, m](int rows, int cols) {
            m->close();
            write::NewTable table;
            table.rows = rows;
            table.columns = cols;
            insertTableNow(table);
        });
        return m;
    };
    // (the ids are copied out of the list: an initializer_list's items live only as long as the call that made it, and the menu is built later)
    auto menuOf = [this](std::initializer_list<const char *> list) {
        QStringList ids;
        for (const char *id : list) ids << QString::fromLatin1(id);
        return [this, ids](QWidget *owner) {
            auto *m = new QMenu(owner);
            for (const QString &id : ids) {
                if (id == QLatin1String("-")) m->addSeparator();
                else m->addAction(act(id));
            }
            return m;
        };
    };
    parts.menus[QStringLiteral("tableBorders")] = menuOf({"table.border.bottom", "table.border.top", "table.border.left", "table.border.right", "-", "table.border.none", "table.border.all", "table.border.outside",
                                                           "table.border.inside", "table.border.insideH", "table.border.insideV", "-", "table.bordersShading"});
    parts.menus[QStringLiteral("tableAutoFit")] = menuOf({"table.autofitContents", "table.autofitWindow", "table.fixedWidth"});
    parts.menus[QStringLiteral("tableDelete")] = menuOf({"table.deleteCells", "-", "table.deleteRows", "table.deleteColumns", "table.deleteTable"});
    parts.menus[QStringLiteral("tableSelect")] = menuOf({"table.selectTable", "table.selectRow", "table.selectColumn", "table.selectCell"});
    // The color of the cells' fill.
    parts.widgets[QStringLiteral("tableShading")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("paint-bucket"), tr("Shading"), true, tr("No Color"));
        b->setCurrent(ColorRef::rgb(QColor(0xFF, 0xFF, 0x99)));
        connect(b, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
            if (!m_doc) return;
            const QColor fill = c.isNone() ? QColor() : c.resolve(m_doc->flow().env.colors);
            tableCommand([fill](write::WriteDoc &d, QTextCursor &cur) { write::setShading(d, cur, fill); });
        });
        m_colorButtons << b;
        return b;
    };
    // Height and width of the row and column the cursor is in.
    for (int which = 0; which < 2; ++which) {
        parts.widgets[which == 0 ? QStringLiteral("tableRowHeight") : QStringLiteral("tableColumnWidth")] = [this, which]() -> QWidget * {
            auto *spin = new MeasureSpin();
            spin->setObjectName(which == 0 ? QStringLiteral("tableRowHeightField") : QStringLiteral("tableColumnWidthField"));
            spin->setAccessibleName(which == 0 ? tr("Table row height") : tr("Table column width"));
            m_tableFields << spin;
            connect(spin, &QDoubleSpinBox::editingFinished, this, [this, spin, which] {
                if (m_updatingFields || !m_doc || !write::cellPlace(*m_doc, m_view->cursor()).inTable) return;
                const double v = spin->value();
                const write::TableProps now = write::tablePropsAt(*m_doc, m_view->cursor());
                if (std::abs(v - (which == 0 ? now.rowHeight : now.columnWidth)) < 0.005) return;   // (the same value is no command: no step to undo, nothing to save)
                if (which == 0) {
                    const bool exact = now.rowExact;
                    tableCommand([=](write::WriteDoc &d, QTextCursor &c) { write::setRowHeight(d, c, v, exact); });
                } else {
                    tableCommand([=](write::WriteDoc &d, QTextCursor &c) { write::setColumnWidth(d, c, v); });
                }
            });
            return spin;
        };
    }
    // The gallery of table styles.
    parts.widgets[QStringLiteral("tableStyleGallery")] = [this]() -> QWidget * {
        auto *g = new Gallery(QSize(64, 48), 6);
        g->setTitle(tr("Table Styles"));
        g->setItemsProvider([this]() {
            QVector<GalleryItem> items;
            if (!m_doc) return items;
            const qreal dpr = devicePixelRatioF();
            QStringList seen;
            auto add = [&](const office::wml::Style &s) {
                if (s.id == QLatin1String("TableNormal") || seen.contains(s.id)) return;
                seen << s.id;
                items << GalleryItem{s.id, office::wml::tableStyleScreenName(s), tableStyleIcon(s.id, s, QSize(64, 48), dpr), QString()};
            };
            for (const QString &id : office::wml::builtinTableStyleIds()) {
                const office::wml::Style *own = m_doc->styleSheet().find(id);
                add(own ? *own : office::wml::builtinStyle(id));
            }
            for (const office::wml::Style &s : m_doc->styleSheet().styles)
                if (s.type == QLatin1String("table") && !s.semiHidden) add(s);
            return items;
        });
        g->setFooterActions({act("table.style.new"), act("table.style.modify"), act("table.style.clear")});
        g->reload();
        connect(g, &Gallery::activated, this, [this](const QString &id) { tableCommand([id](write::WriteDoc &d, QTextCursor &c) { write::setTableStyle(d, c, id); }); });
        m_tableStyleGallery = g;
        return g;
    };
}

// ---- keeping the controls in step with the cursor

bool WriteWindow::cursorInTable() const
{
    return m_doc && !m_view->inHeaderFooter() && !m_view->inNote() && write::inTable(m_view->cursor().block());
}

void WriteWindow::refreshTableUi()
{
    const bool in = cursorInTable();
    for (auto it = m_actions.constBegin(); it != m_actions.constEnd(); ++it)
        if (it.key().startsWith(QLatin1String("table."))) it.value()->setEnabled(in);
    if (in) {
        // Merging needs cells in more than one place; the rest is worked out when it is run.
        const QTextCursor &c = m_view->cursor();
        const QTextDocument *d = c.document();
        act("table.merge")->setEnabled(c.hasSelection() && write::cellKeyOf(d->findBlock(c.selectionStart())) != write::cellKeyOf(d->findBlock(c.selectionEnd())));
    }
    const bool inBody = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    act("insert.table")->setEnabled(inBody);
    act("insert.convertTextToTable")->setEnabled(inBody && !in);
    {
        QTextCursor probe = m_view->cursor();
        act("table.deleteCells")->setEnabled(in && write::deleteTableCellsShiftLeft(*m_doc, probe, false));   // (a cell with others left in its row, not joined down)
    }
    act("table.splitTable")->setEnabled(in && write::canSplitTable(*m_doc, m_view->cursor()));   // (a table is cut above the caret's row: not the first, and not through a cell joined down)
    act("table.gridlines")->setEnabled(in || act("table.gridlines")->isChecked());   // (it shows the edges of the cells of tables: nothing to show elsewhere, except to turn it off)
    m_ribbon->setContextVisible(QStringLiteral("Table Tools"), in, false);
    if (!in) return;
    // The fields and options are read only while a tab that shows them is the one in front (the table is looked at from its start, which a long document makes dear).
    bool shown = false;
    for (int i = 0; i < m_ribbon->tabCount(); ++i)
        if (m_ribbon->tabPage(i) == m_ribbon->current() && (m_ribbon->tabName(i) == QLatin1String("Table Layout") || m_ribbon->tabName(i) == QLatin1String("Table Design"))) shown = true;
    if (!shown) return;
    const write::TableProps props = write::tablePropsAt(*m_doc, m_view->cursor());
    m_updatingFields = true;
    if (m_tableFields.size() >= 2) {
        m_tableFields[0]->setPoints(props.rowHeight);
        m_tableFields[1]->setPoints(props.columnWidth);
    }
    m_updatingFields = false;
    act("table.repeatHeader")->setChecked(props.header);
    act("table.look.header")->setChecked(props.look & 0x20);
    act("table.look.total")->setChecked(props.look & 0x40);
    act("table.look.firstColumn")->setChecked(props.look & 0x80);
    act("table.look.lastColumn")->setChecked(props.look & 0x100);
    act("table.look.bandedRows")->setChecked(!(props.look & 0x200));
    act("table.look.bandedColumns")->setChecked(!(props.look & 0x400));
    if (auto *g = qobject_cast<Gallery *>(m_tableStyleGallery)) g->setCurrent(props.style);
}

// The part of the right-click menu for a cell.
void WriteWindow::addTableContextMenu(QMenu &menu)
{
    menu.addSeparator();
    auto *insert = menu.addMenu(tr("Insert"));
    for (const char *id : {"table.insertAbove", "table.insertBelow", "table.insertLeft", "table.insertRight"}) insert->addAction(act(QString::fromLatin1(id)));
    auto *remove = menu.addMenu(tr("Delete"));
    for (const char *id : {"table.deleteCells", "table.deleteRows", "table.deleteColumns", "table.deleteTable"}) remove->addAction(act(QString::fromLatin1(id)));
    auto *select = menu.addMenu(tr("Select"));
    for (const char *id : {"table.selectTable", "table.selectRow", "table.selectColumn", "table.selectCell"}) select->addAction(act(QString::fromLatin1(id)));
    menu.addAction(act("table.merge"));
    menu.addAction(act("table.split"));
    menu.addSeparator();
    menu.addAction(act("table.bordersShading"));
    auto *fit = menu.addMenu(tr("Fit"));
    for (const char *id : {"table.autofitContents", "table.autofitWindow", "table.fixedWidth"}) fit->addAction(act(QString::fromLatin1(id)));
    menu.addAction(act("table.distributeRows"));
    menu.addAction(act("table.distributeColumns"));
    auto *align = menu.addMenu(tr("Cell Alignment"));
    for (const char *id : {"tl", "tc", "tr", "ml", "mc", "mr", "bl", "bc", "br"}) align->addAction(act(QStringLiteral("table.align.") + QLatin1String(id)));
    menu.addAction(act("table.properties"));
}

} // namespace jp
