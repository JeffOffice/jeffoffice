// Cut, Copy, and Paste: the cells go on the clipboard as text, as an HTML table, and whole (see core/clipdata.h), and come back from it, from this program and from others. A cut and
// paste moves the cells; Paste Special asks what to paste and what to do with it.
#include "app/crunchwindow.h"

#include "sheet/cellref.h"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QGroupBox>
#include <QLineEdit>
#include <QMimeData>
#include <QRadioButton>
#include <QVBoxLayout>

#include <atomic>

namespace jp {

using crunch::ClipData;
using jp::office::sheet::CellRange;

void CrunchWindow::createClipboardActions()
{
    mk("edit.copy", tr("Copy"), "copy", QKeySequence::Copy, [this] { copySelection(); });
    mk("edit.cut", tr("Cut"), "scissors", QKeySequence::Cut, [this] { cutSelection(); });
    using Options = crunch::EditSession::PasteOptions;
    mk("edit.paste", tr("Paste"), "clipboard-paste", QKeySequence::Paste, [this] { pasteClipboard(Options()); });
    mk("paste.values", tr("Values"), "", QKeySequence(), [this] {
        Options o;
        o.what = Options::What::Values;
        pasteClipboard(o);
    });
    mk("paste.formulas", tr("Formulas"), "", QKeySequence(), [this] {
        Options o;
        o.what = Options::What::Formulas;
        pasteClipboard(o);
    });
    mk("paste.formats", tr("Formatting"), "", QKeySequence(), [this] {
        Options o;
        o.what = Options::What::Formats;
        pasteClipboard(o);
    });
    mk("paste.transpose", tr("Transpose"), "", QKeySequence(), [this] {
        Options o;
        o.transpose = true;
        pasteClipboard(o);
    });
    mk("paste.special", tr("Paste Special…"), "", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_V), [this] { pasteSpecial(); });
    for (const char *id : {"edit.cut", "edit.paste", "paste.values", "paste.formulas", "paste.formats", "paste.transpose", "paste.special"}) m_opsActions << QString::fromLatin1(id);
    connect(m_view, &grid::GridView::copyBorderCleared, this, [this] { forgetClip(); });
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this] { onClipboardChanged(); });
    updateEditActions();
}

// The clipboard was changed by someone: if it is not the copy made here that it holds, the border of the cells goes.
void CrunchWindow::onClipboardChanged()
{
    if (m_settingClipboard || !m_clip.valid()) return;
    const QMimeData *md = QApplication::clipboard()->mimeData();
    ClipData held;
    if (md && md->hasFormat(QLatin1String(crunch::kClipMime)) && ClipData::fromBytes(md->data(QLatin1String(crunch::kClipMime)), &held) && held.token == m_clip.token && held.serial == m_clip.serial) return;
    forgetClip();
    m_view->clearCopyBorder();
}

void CrunchWindow::forgetClip()
{
    if (m_clip.valid()) {
        // (what is on the clipboard of the copy made here goes with its border)
        const QMimeData *md = QApplication::clipboard()->mimeData();
        ClipData held;
        if (md && md->hasFormat(QLatin1String(crunch::kClipMime)) && ClipData::fromBytes(md->data(QLatin1String(crunch::kClipMime)), &held) && held.token == m_clip.token && held.serial == m_clip.serial) {
            m_settingClipboard = true;
            QApplication::clipboard()->clear();
            m_settingClipboard = false;
        }
    }
    m_clip = ClipData();
}

// An edit made while cells are cut (typing, anything that moves or sizes cells, an Undo) ends the cut, as in the programs people know: its border goes, and the cells are no longer ready to
// be pasted. (The paste of the cut is an edit too, and it ends the cut it moves.)
void CrunchWindow::cancelCut()
{
    if (m_pendingClip.valid() && m_pendingClip.cut) {
        m_copy.reset();
        m_copyTimer.stop();
        m_pendingClip = ClipData();
        m_pendingHtml.clear();
        m_view->clearCopyBorder();
        if (!m_calculating && !m_loading) setStatus(tr("Ready"));
    }
    if (m_clip.valid() && m_clip.cut) {
        forgetClip();
        m_view->clearCopyBorder();
    }
}

void CrunchWindow::copySelection() { copyOrCut(false); }
void CrunchWindow::cutSelection() { copyOrCut(true); }

// An HTML table of the cells, with the look of each (alignment, bold, italic, colors) for the programs that read it.
QString CrunchWindow::htmlOfClip(const ClipData &clip) const
{
    if (!clip.valid()) return QString();
    const grid::GridSheet &gs = m_view->sheet();
    const grid::GridLayout &L = gs.layout();
    QString html = QStringLiteral("<html><body><table>");
    for (quint32 r = 0; r < clip.rows; ++r) {
        const quint32 row = clip.source.row1 + r;
        if (L.rows.size(row) == 0) continue;
        html += QStringLiteral("<tr>");
        for (quint32 c = 0; c < clip.cols; ++c) {
            const quint32 col = clip.source.col1 + c;
            if (L.cols.size(col) == 0) continue;
            const grid::Cell *cell = gs.cellAt(row, col);
            QString text, style;
            if (cell) {
                text = gs.textOf(*cell).toHtmlEscaped();
                text.replace(QLatin1Char('\n'), QStringLiteral("<br>"));
                const grid::CellLook &look = gs.looks().look(cell->style);
                if (gs.valueOf(*cell).isNumber()) style += QStringLiteral("text-align:right;");
                if (look.bold) style += QStringLiteral("font-weight:bold;");
                if (look.italic) style += QStringLiteral("font-style:italic;");
                if (look.pattern == jp::office::sheet::PatternKind::Solid) style += QStringLiteral("background:%1;").arg(QColor::fromRgba(look.fg).name());
                if (look.textColor != 0xFF000000) style += QStringLiteral("color:%1;").arg(QColor::fromRgba(look.textColor).name());
            }
            html += QStringLiteral("<td") + (style.isEmpty() ? QString() : QStringLiteral(" style=\"%1\"").arg(style)) + QLatin1Char('>') + text + QStringLiteral("</td>");
        }
        html += QStringLiteral("</tr>");
    }
    return html + QStringLiteral("</table></body></html>");
}

void CrunchWindow::copyOrCut(bool cut)
{
    // in a line of text (the cell's, the formula bar's, the name box) the keys are the line's
    if (auto *line = qobject_cast<QLineEdit *>(QApplication::focusWidget())) {
        if (cut) line->cut();
        else line->copy();
        return;
    }
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active) return;
    if (pickedObject() >= 0) {   // (an object over the cells is picked: it is what is copied, if it can be)
        copyObject(cut);
        return;
    }
    m_copy = m_view->copyWalk();
    m_pendingClip = ClipData();
    if (m_copy && m_copy->tooMany()) {
        m_copy.reset();
        notice(tr("Copy"), tr("That is more than a million cells, which JeffCrunch doesn't copy as text. Select fewer cells."), false);
        return;
    }
    // the cells whole (when there are not too many), and the border that shows what is copied
    static std::atomic<qint64> serial{0};
    const grid::GridRange sel = m_view->selection().range;
    if (m_session && cut && !editingAllowed()) {
        m_copy.reset();
        return;
    }
    if (!m_copy) {
        // (cells with nothing to show: a copy that clears what it is pasted over)
        m_copy = std::make_unique<grid::TextWalk>(&m_view->sheet(), grid::GridRange{0, 0, -1, -1});
    }
    if (m_session) {
        m_pendingClip = m_session->copyCells(m_sheet, selectedRange(), cut);
        m_pendingClip.serial = ++serial;
        if (m_pendingClip.valid()) m_pendingHtml = htmlOfClip(m_pendingClip);
    }
    m_pendingBorder = sel;
    m_view->setCopyBorder(sel);
    // (a large selection is turned to text a few milliseconds at a time, and put on the clipboard when it is all text)
    setStatus(cut ? tr("Cutting…") : tr("Copying…"));
    m_copyTimer.start(0);
}

void CrunchWindow::copyStep()
{
    if (!m_copy) return;
    QElapsedTimer t;
    t.start();
    bool done = false;
    while (!done && t.elapsed() < 4) done = m_copy->step(20000);
    if (!done) {
        m_copyTimer.start(0);
        return;
    }
    if (m_copy->tooLarge()) {
        m_copy.reset();
        m_view->clearCopyBorder();
        notice(tr("Copy"), tr("That is more text than JeffCrunch copies at once. Select fewer cells."), false);
        if (!m_calculating && !m_loading) setStatus(tr("Ready"));
        return;
    }
    publishClipboard(m_copy->text());
    m_copy.reset();
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
}

void CrunchWindow::publishClipboard(const QString &text)
{
    auto *md = new QMimeData;
    md->setText(text);
    if (m_pendingClip.valid()) {
        md->setHtml(m_pendingHtml);
        md->setData(QLatin1String(crunch::kClipMime), m_pendingClip.toBytes());
    }
    m_settingClipboard = true;
    QApplication::clipboard()->setMimeData(md);
    m_settingClipboard = false;
    m_clip = m_pendingClip;
    if (!m_clip.valid()) m_view->clearCopyBorder();
    m_pendingClip = ClipData();
    m_pendingHtml.clear();
}

void CrunchWindow::pasteSpecial()
{
    using Options = crunch::EditSession::PasteOptions;
    Options o;
    if (m_pasteOptionsAsker) {
        if (!m_pasteOptionsAsker(&o)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Paste Special"));
        auto *layout = new QVBoxLayout(&dialog);
        auto *what = new QGroupBox(tr("Paste"));
        auto *whatLayout = new QVBoxLayout(what);
        auto *whatGroup = new QButtonGroup(&dialog);
        const QStringList whatNames = {tr("All"), tr("Formulas"), tr("Values"), tr("Formats")};
        for (int i = 0; i < whatNames.size(); ++i) {
            auto *b = new QRadioButton(whatNames[i]);
            b->setChecked(i == 0);
            whatGroup->addButton(b, i);
            whatLayout->addWidget(b);
        }
        layout->addWidget(what);
        auto *op = new QGroupBox(tr("Operation"));
        auto *opLayout = new QVBoxLayout(op);
        auto *opGroup = new QButtonGroup(&dialog);
        const QStringList opNames = {tr("None"), tr("Add"), tr("Subtract"), tr("Multiply"), tr("Divide")};
        for (int i = 0; i < opNames.size(); ++i) {
            auto *b = new QRadioButton(opNames[i]);
            b->setChecked(i == 0);
            opGroup->addButton(b, i);
            opLayout->addWidget(b);
        }
        layout->addWidget(op);
        auto *blanks = new QCheckBox(tr("Skip blanks"));
        auto *transpose = new QCheckBox(tr("Transpose"));
        layout->addWidget(blanks);
        layout->addWidget(transpose);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        o.what = Options::What(whatGroup->checkedId());
        o.operation = Options::Operation(opGroup->checkedId());
        o.skipBlanks = blanks->isChecked();
        o.transpose = transpose->isChecked();
    }
    pasteClipboard(o);
}

// Paste: what the clipboard holds goes in at the selected cell. The cells this program copied or cut go in whole (a cut moves them), and text or an HTML table from another program is entered
// cell by cell as if typed.
void CrunchWindow::pasteClipboard(const crunch::EditSession::PasteOptions &options)
{
    if (auto *line = qobject_cast<QLineEdit *>(QApplication::focusWidget())) {
        line->paste();
        return;
    }
    if (pasteObject()) return;   // (a chart was copied: it is what is pasted)
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if (!md) return;
    const CellRange target = selectedRange();
    const qint64 before = m_loader.changesMade();
    ClipData clip;
    using Options = crunch::EditSession::PasteOptions;
    const bool rich = md->hasFormat(QLatin1String(crunch::kClipMime)) && ClipData::fromBytes(md->data(QLatin1String(crunch::kClipMime)), &clip) && clip.valid();
    QVector<QStringList> table;
    if (!rich) {
        if (md->hasText()) table = crunch::parseTabSeparated(md->text());
        if (table.isEmpty() && md->hasHtml()) table = crunch::parseHtmlTable(md->html());
        if (table.isEmpty()) return;
    }
    auto run = [&](const Options &opt) {
        crunch::EditSession::Outcome result;
        if (!rich) return m_session->pasteText(m_sheet, target, table, opt);
        const bool plain = opt.what == Options::What::All && opt.operation == Options::Operation::None && !opt.skipBlanks && !opt.transpose;
        const bool mine = clip.token == m_session->token() && m_clip.valid() && clip.serial == m_clip.serial;
        if (clip.cut && mine && plain) {
            // a cut and paste moves the cells
            result = doStructural([&] { return m_session->moveCells(clip.sheet, clip.source, m_sheet, target.row1, target.col1); });
            if (result.ok) {
                result.range = clip.source;
                result.range.row2 = target.row1 + (clip.source.row2 - clip.source.row1);
                result.range.col2 = target.col1 + (clip.source.col2 - clip.source.col1);
                result.range.row1 = target.row1;
                result.range.col1 = target.col1;
                m_settingClipboard = true;
                QApplication::clipboard()->clear();
                m_settingClipboard = false;
                m_clip = ClipData();
                m_view->clearCopyBorder();
            }
            return result;
        }
        return m_session->paste(m_sheet, target, clip, opt);
    };
    crunch::EditSession::Outcome o = run(options);
    if (!o.ok) {
        if (!o.confirm) notice(tr("Paste"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    if (o.range.valid) {
        widenForCells(m_sheet, o.range);
        m_view->selectRange(o.range.row1, o.range.col1, o.range.row2, o.range.col2);
    }
}

// Numbers just put in columns that have the width they began with widen them to show the numbers (in the step of the entry).
void CrunchWindow::widenForCells(int sheet, const CellRange &cells)
{
    if (sheet != m_sheet || !m_session || !cells.valid) return;
    const int mdw = std::max(1, m_view->sheet().looks().maxDigitWidth());
    const quint32 lastCol = std::min<quint32>(cells.col2, cells.col1 + 255);
    for (quint32 col = cells.col1; col <= lastCol; ++col) {
        double need = 0;
        qint64 seen = 0;
        m_view->sheet().data().sheet->cells.forEachInRange(cells.row1, col, cells.row2, col, [&](const jp::office::sheet::Cell &c) {
            if (++seen > 20000) return false;
            if (c.hasFormula() || !m_view->sheet().valueOf(c).isNumber()) return true;
            need = std::max(need, m_view->sheet().fitWidthOfCell(c.row, c.col));
            return true;
        });
        if (need > 0 && need * mdw > m_view->sheet().columnWidth(col) + 0.5) m_session->widenColumnWithEntry(sheet, col, need);
    }
}

} // namespace jp
