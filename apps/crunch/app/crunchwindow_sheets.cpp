// The sheets of a workbook in the window: the tabs (a click shows a sheet, Ctrl and Shift pick several, a drag moves one, a double click renames it, a right click opens a menu), and the
// commands that add, copy, rename, delete, move, hide, show, and color them. Each is a step of the edit session (see core/editsheets.cpp); this part asks the person what the step needs and
// keeps the tabs and the sheet shown in step with the sheets.
#include "app/crunchwindow.h"
#include "app/plaintext.h"

#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/sheettabs.h"
#include "core/sheetobjects.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <algorithm>

namespace jp {

using namespace jp::office::sheet;

QVector<int> CrunchWindow::selectedSheets() const
{
    QVector<int> out;
    for (const int tab : m_tabs->selectedTabs())
        if (tab >= 0 && tab < m_tabSheets.size()) out.append(m_tabSheets[tab]);
    if (out.isEmpty() && m_sheet >= 0) out.append(m_sheet);
    return out;
}

void CrunchWindow::createSheetActions()
{
    auto add = [this](const QString &id, const QString &text, const char *iconName, const QKeySequence &key, const std::function<void()> &fn) {
        mk(id, text, QString::fromLatin1(iconName), key, fn, false, true);
        m_opsActions << id;
    };
    add(QStringLiteral("sheet.insert"), tr("Insert Sheet"), "plus", QKeySequence(Qt::SHIFT | Qt::Key_F11), [this] { insertSheetCommand(false); });
    add(QStringLiteral("sheet.insertEnd"), tr("New Sheet"), "plus", QKeySequence(), [this] { insertSheetCommand(true); });
    add(QStringLiteral("sheet.delete"), tr("Delete Sheet"), "trash-2", QKeySequence(), [this] { deleteSheetsCommand(); });
    add(QStringLiteral("sheet.rename"), tr("Rename Sheet"), "pencil", QKeySequence(), [this] {
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        if (!editingAllowed()) return;
        m_tabs->beginRename(m_tabs->currentIndex());
    });
    add(QStringLiteral("sheet.moveCopy"), tr("Move or Copy Sheet…"), "copy", QKeySequence(), [this] { moveCopyCommand(); });
    add(QStringLiteral("sheet.hide"), tr("Hide Sheet"), "eye-off", QKeySequence(), [this] { hideSheetsCommand(); });
    add(QStringLiteral("sheet.tabColor"), tr("Tab Color"), "palette", QKeySequence(), [] {});
    add(QStringLiteral("sheet.unhide"), tr("Unhide Sheet"), "eye", QKeySequence(), [] {});
    // the button that adds a sheet at the end of the tabs
    auto *plus = new QToolButton(m_sheetTabRow);
    plus->setObjectName(QStringLiteral("addSheet"));
    plus->setDefaultAction(act(QStringLiteral("sheet.insertEnd")));
    plus->setAutoRaise(true);
    plus->setToolButtonStyle(Qt::ToolButtonIconOnly);
    plus->setToolTip(tr("New sheet"));
    plus->setAccessibleName(tr("New sheet"));
    qobject_cast<QHBoxLayout *>(m_sheetTabRow->layout())->addWidget(plus);
}

// ---- the tabs follow the sheets

void CrunchWindow::refreshStripes()
{
    if (!m_loaded || !m_loaded->book) return;
    const Styles &styles = m_loaded->book->styles();
    for (int tab = 0; tab < m_tabSheets.size(); ++tab) {
        ColorSpec color;
        // (the calculation is the worker's until the read is told to be over: it is looked at only once m_haveCalc says so, as everywhere else)
        if (m_haveCalc && m_loaded->calc && m_tabSheets[tab] < m_loaded->calc->sheetCount()) color = m_loaded->calc->sheet(m_tabSheets[tab]).tabColor;
        else if (m_loaded->preview && m_tabSheets[tab] == m_sheet) color = m_loaded->preview->tabColor;
        m_tabs->setStripe(tab, color.isSet() ? QColor::fromRgb(styles.resolve(color, 0x808080)) : QColor());
    }
}

// The sheets changed (a step, Undo, or Redo): the tabs are made again, and the sheet shown is one that is still there and shown (else the one beside where it was).
void CrunchWindow::onSheetsChanged()
{
    if (!m_loaded || !m_loaded->book) return;
    const WorkbookInfo &info = m_loaded->book->info();
    auto shown = [&](int i) { return i >= 0 && i < info.sheets.size() && !info.sheets[i].deleted && info.sheets[i].state == SheetState::Visible; };
    const int oldTab = m_tabSheets.indexOf(m_sheet);
    const QVector<int> picked = selectedSheets();
    const int previous = m_sheet;
    int target = m_sheet;
    fillTabs();
    if (!shown(target)) {
        target = -1;
        if (!m_tabSheets.isEmpty()) target = m_tabSheets[std::max(0, std::min(oldTab, int(m_tabSheets.size()) - 1))];
    }
    if (target < 0) return;
    if (target != m_sheet) {
        // (the sheet that was shown is gone or hidden: another takes its place, and what was done in it is not kept for a sheet that is not there)
        if (shown(m_sheet)) rememberView();
        m_sheet = target;
        const int tab = m_tabSheets.indexOf(target);
        if (tab >= 0) {
            m_switchingTabs = true;
            m_tabs->setCurrentIndex(tab);
            m_switchingTabs = false;
        }
        showSheet(target, false);
    } else {
        showSheet(m_sheet, true);
    }
    // the tabs that were picked and are still tabs stay picked, while the sheet shown is the same (another sheet shown is picked alone)
    if (target == previous) {
        QSet<int> again;
        for (const int sheet : picked) {
            const int tab = m_tabSheets.indexOf(sheet);
            if (tab >= 0) again.insert(tab);
        }
        again.insert(m_tabSheets.indexOf(m_sheet));
        m_tabs->setSelectedTabs(again);
    }
    updateBars();
    placeEditor();
}

// A tab dragged to another place: the sheet goes before the one that is now after it (the tabs are made again from the order the session has).
void CrunchWindow::onTabMoved(int from, int to)
{
    if (m_switchingTabs || from < 0 || to < 0 || from >= m_tabSheets.size() || to >= m_tabSheets.size() || from == to) return;
    const int sheet = m_tabSheets[from];
    int before = -1;
    if (to < from) before = m_tabSheets[to];
    else if (to + 1 < m_tabSheets.size()) before = m_tabSheets[to + 1];
    // (the tabs are already in their new order: the list of the sheets they show is put in step, so the tab picked is the sheet moved)
    m_tabSheets.move(from, to);
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) {
        fillTabs();
        return;
    }
    if (!editingAllowed()) {
        fillTabs();
        return;
    }
    const crunch::EditSession::Outcome o = m_session->moveSheet(sheet, before);
    if (!o.ok) {
        notice(tr("Move Sheet"), o.problem);
        fillTabs();
    }
}

bool CrunchWindow::renameSheetTo(int sheet, const QString &name)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return false;
    if (!editingAllowed()) return false;
    const crunch::EditSession::Outcome o = m_session->renameSheet(sheet, name);
    if (!o.ok) {
        notice(tr("Rename Sheet"), o.problem);
        return false;
    }
    return true;
}

// ---- the commands

void CrunchWindow::insertSheetCommand(bool atEnd)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    int made = -1;
    const crunch::EditSession::Outcome o = m_session->insertSheet(atEnd ? -1 : m_sheet, QString(), &made);
    if (!o.ok) {
        notice(tr("Insert Sheet"), o.problem);
        return;
    }
    setActiveSheet(made);
}

void CrunchWindow::deleteSheetsCommand()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    const QVector<int> sheets = selectedSheets();
    const WorkbookInfo &info = m_loaded->book->info();
    bool anyCells = false;
    for (const int s : sheets) anyCells = anyCells || m_session->sheetHasCells(s);
    // (a workbook keeps at least one sheet that is shown)
    int shownLeft = 0;
    for (int i = 0; i < info.sheets.size(); ++i)
        if (!info.sheets[i].deleted && info.sheets[i].state == SheetState::Visible && !sheets.contains(i)) ++shownLeft;
    if (shownLeft == 0) {
        notice(tr("Delete Sheet"), tr("A workbook must have at least one sheet that is shown. Show another sheet, or add one, and then delete these."));
        return;
    }
    if (anyCells) {
        const QString text = sheets.size() == 1 ? tr("Delete the sheet \"%1\" and everything on it? Formulas that read it will show #REF!. You can undo this until the workbook is saved.").arg(info.sheets[sheets.first()].name)
                                                : tr("Delete the %1 selected sheets and everything on them? Formulas that read them will show #REF!. You can undo this until the workbook is saved.").arg(sheets.size());
        if (!confirm(tr("Delete Sheet"), text)) return;
    }
    QUndoStack *u = m_session->undoStack();
    if (sheets.size() > 1) u->beginMacro(tr("Delete Sheets"));
    for (const int s : sheets) {
        const crunch::EditSession::Outcome o = m_session->deleteSheet(s);
        if (!o.ok) {
            notice(tr("Delete Sheet"), o.problem);
            break;
        }
    }
    if (sheets.size() > 1) u->endMacro();
}

void CrunchWindow::moveCopyCommand()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || !isReadOnly()) {
        if (!editingAllowed()) return;   // (a workbook open for reading only can still be copied from)
    }
    MoveCopyRequest request;
    bool toOther = false;   // (the sheets are to go to another workbook: chosen, whether or not it is still open)
    request.sheets = selectedSheets();
    const WorkbookInfo &info = m_loaded->book->info();
    if (m_moveCopyAsker) {
        if (!m_moveCopyAsker(&request)) return;
        toOther = request.toOther || (!request.toBook.isNull() && request.toBook != this);
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Move or Copy Sheets"));
        auto *layout = new QVBoxLayout(&dialog);
        QStringList names;
        for (const int s : request.sheets) names << info.sheets[s].name;
        layout->addWidget(plainLabel(new QLabel(tr("Sheets chosen: %1").arg(names.join(QStringLiteral(", "))))));
        auto *label = new QLabel(tr("&Put the sheets in front of"));
        layout->addWidget(label);
        auto *list = new QListWidget;
        list->setObjectName(QStringLiteral("beforeSheet"));
        label->setBuddy(list);
        for (const int place : info.order)
            if (!request.sheets.contains(place)) {
                auto *item = new QListWidgetItem(info.sheets[place].name + (info.sheets[place].state == SheetState::Visible ? QString() : tr(" (hidden)")), list);
                item->setData(Qt::UserRole, place);
            }
        auto *end = new QListWidgetItem(tr("(after the last sheet)"), list);
        end->setData(Qt::UserRole, -1);
        list->setCurrentItem(end);
        layout->addWidget(list);
        auto *copy = new QCheckBox(tr("&Copy them, and leave the originals where they are"));
        copy->setObjectName(QStringLiteral("makeCopy"));
        layout->addWidget(copy);
        // (the other workbooks that are open: a copy can go to one of them)
        QVector<QPointer<CrunchWindow>> others;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *w = qobject_cast<CrunchWindow *>(top))
                if (w != this && w->m_session && w->isVisible()) others.push_back(w);
        QComboBox *toBook = nullptr;
        QLabel *toNote = nullptr;
        if (!others.isEmpty()) {
            auto *toLabel = new QLabel(tr("&To workbook"));
            layout->addWidget(toLabel);
            toBook = new QComboBox;
            toBook->setObjectName(QStringLiteral("toBook"));
            toLabel->setBuddy(toBook);
            toBook->addItem(tr("(this workbook)"));
            for (const QPointer<CrunchWindow> &w : others) toBook->addItem(w->filePath().isEmpty() ? w->windowTitle() : QFileInfo(w->filePath()).fileName());
            layout->addWidget(toBook);
            toNote = plainLabel(new QLabel);
            toNote->setObjectName(QStringLiteral("toBookNote"));
            toNote->setWordWrap(true);
            layout->addWidget(toNote);
            // (said before it happens: what the copy will not have, and what becomes a link)
            auto explain = [&, toBook, toNote] {
                const int at = toBook->currentIndex() - 1;
                toNote->clear();
                if (at >= 0 && others[at].isNull()) {
                    toNote->setText(tr("That workbook has been closed."));
                    return;
                }
                if (at < 0) {
                    copy->setEnabled(true);
                    return;
                }
                copy->setChecked(true);
                copy->setEnabled(false);   // (moving a sheet to another workbook is not done yet)
                QStringList said;
                syncPanes();
                for (const int s : request.sheets) {
                    const crunch::EditSession::SheetExport data = m_session->exportSheet(s, filePath(), drawnKindsOn(s));
                    const crunch::EditSession::Outcome o = others[at]->m_session->checkImport(data);
                    said << (request.sheets.size() > 1 ? info.sheets[s].name + QStringLiteral(": ") : QString()) + o.problem;
                }
                toNote->setText(said.join(QLatin1Char('\n')));
            };
            connect(toBook, QOverload<int>::of(&QComboBox::currentIndexChanged), &dialog, explain);
        }
        auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        request.before = list->currentItem() ? list->currentItem()->data(Qt::UserRole).toInt() : -1;
        request.copy = copy->isChecked();
        if (toBook && toBook->currentIndex() > 0) {
            toOther = true;
            request.toBook = others[toBook->currentIndex() - 1];
        }
    }
    if (toOther) {
        copySheetsToWorkbook(request);
        return;
    }
    if (isReadOnly() && !editingAllowed()) return;
    // (a sheet is not put before itself or before one of those that go with it)
    if (request.sheets.contains(request.before)) request.before = -1;
    QUndoStack *u = m_session->undoStack();
    const QString text = request.copy ? tr("Copy Sheets") : tr("Move Sheets");
    if (request.sheets.size() > 1) u->beginMacro(text);
    int last = -1;
    for (const int s : request.sheets) {
        int made = -1;
        const crunch::EditSession::Outcome o = request.copy ? m_session->copySheet(s, request.before, QString(), &made) : m_session->moveSheet(s, request.before);
        if (!o.ok) {
            notice(request.copy ? tr("Copy Sheet") : tr("Move Sheet"), o.problem);
            break;
        }
        if (!o.problem.isEmpty()) notice(tr("Copy Sheet"), o.problem, false);
        if (made >= 0) last = made;
    }
    if (request.sheets.size() > 1) u->endMacro();
    if (last >= 0) setActiveSheet(last);
}

// Copies the sheets to another open workbook, as one step of Undo there (the first workbook is only read). All of them or none: every sheet is looked at before the first is put in. Moving a
// sheet there is not done yet.
void CrunchWindow::copySheetsToWorkbook(const MoveCopyRequest &request)
{
    if (!request.copy) {
        notice(tr("Move Sheet"), tr("Moving a sheet to another workbook is not done yet. Choose Copy, and then delete the sheet here if you do not need it."));
        return;
    }
    CrunchWindow *target = request.toBook.data();
    if (!target) {
        notice(tr("Copy Sheet"), tr("The other workbook was closed, so nothing was copied."));
        return;
    }
    if (!target->m_session || !target->editingAllowed()) {
        notice(tr("Copy Sheet"), tr("The other workbook can't be changed now. It may be open for reading only, or still opening."));
        return;
    }
    syncPanes();   // (the panes of the sheet in view are what the window shows now)
    std::vector<crunch::EditSession::SheetExport> taken;
    for (const int s : request.sheets) {
        taken.push_back(m_session->exportSheet(s, filePath(), drawnKindsOn(s)));
        const crunch::EditSession::Outcome o = target->m_session->checkImport(taken.back());
        if (!o.ok) {
            notice(tr("Copy Sheet"), request.sheets.size() > 1 ? m_loaded->book->info().sheets[s].name + QStringLiteral(": ") + o.problem : o.problem);
            return;
        }
    }
    QUndoStack *u = target->m_session->undoStack();
    const int start = u->index();
    u->beginMacro(request.sheets.size() > 1 ? tr("Copy Sheets to Another Workbook") : tr("Copy Sheet to Another Workbook"));
    int last = -1;
    QStringList said;
    for (size_t i = 0; i < taken.size(); ++i) {
        int made = -1;
        const crunch::EditSession::Outcome o = target->m_session->importSheet(taken[i], -1, QString(), &made);
        if (!o.ok) {
            // (the sheets put in before this one are taken out again: all or none)
            u->endMacro();
            while (u->index() > start) u->undo();
            notice(tr("Copy Sheet"), o.problem);
            return;
        }
        if (!o.problem.isEmpty()) said << (taken.size() > 1 ? m_loaded->book->info().sheets[request.sheets[int(i)]].name + QStringLiteral(": ") : QString()) + o.problem;
        if (made >= 0) last = made;
    }
    u->endMacro();
    if (last >= 0) target->setActiveSheet(last);
    if (!said.isEmpty()) notice(tr("Copy Sheet"), said.join(QLatin1Char('\n')), false);
}

// What the drawing layer has on the sheet (read from the file or made since), named as the notes of a copy name them.
QStringList CrunchWindow::drawnKindsOn(int sheet)
{
    using Kind = jp::office::sheet::ObjectKind;
    QStringList out;
    auto add = [&out](const QString &what) {
        if (!out.contains(what)) out << what;
    };
    if (!m_objects) return out;
    for (const crunch::SheetObjects::ObjectInfo &o : m_objects->describe(sheet)) {
        switch (o.kind) {
        case Kind::Chart:
        case Kind::Picture: add(tr("pictures and charts")); break;
        case Kind::Shape:
        case Kind::Group:
        case Kind::Connector: add(tr("shapes, text boxes, and lines")); break;
        case Kind::Diagram: add(tr("diagrams")); break;
        case Kind::Other: add(tr("other objects over the cells")); break;
        }
    }
    return out;
}

void CrunchWindow::hideSheetsCommand()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    const QVector<int> sheets = selectedSheets();
    // (a workbook keeps at least one sheet that is shown: if these are all of them, nothing is hidden)
    const WorkbookInfo &info = m_loaded->book->info();
    int shownLeft = 0;
    for (int i = 0; i < info.sheets.size(); ++i)
        if (!info.sheets[i].deleted && info.sheets[i].state == SheetState::Visible && !sheets.contains(i)) ++shownLeft;
    if (shownLeft == 0) {
        notice(tr("Hide Sheet"), tr("A workbook must have at least one sheet that is shown."));
        return;
    }
    QUndoStack *u = m_session->undoStack();
    if (sheets.size() > 1) u->beginMacro(tr("Hide Sheets"));
    for (const int s : sheets) {
        const crunch::EditSession::Outcome o = m_session->setSheetHidden(s, true);
        if (!o.ok) {
            notice(tr("Hide Sheet"), o.problem);
            break;
        }
    }
    if (sheets.size() > 1) u->endMacro();
}

void CrunchWindow::setTabColorCommand(const ColorSpec &color)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    const QVector<int> sheets = selectedSheets();
    QUndoStack *u = m_session->undoStack();
    if (sheets.size() > 1) u->beginMacro(tr("Tab Color"));
    for (const int s : sheets) {
        const crunch::EditSession::Outcome o = m_session->setTabColor(s, color);
        if (!o.ok) {
            notice(tr("Tab Color"), o.problem);
            break;
        }
    }
    if (sheets.size() > 1) u->endMacro();
}

QMenu *CrunchWindow::buildTabColorMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("tabColorMenu"));
    connect(menu, &QMenu::aboutToShow, this, [this, menu] {
        menu->clear();
        auto *grid = new QWidgetAction(menu);
        grid->setDefaultWidget(colorGridWidget(workbookColors(), [this, menu](const ColorRef &c) {
            menu->close();
            setTabColorCommand(colorSpecOf(c));
        }, menu));
        menu->addAction(grid);
        menu->addSeparator();
        menu->addAction(icon(QStringLiteral("circle-slash")), tr("No Color"), this, [this] { setTabColorCommand(ColorSpec()); });
    });
    return menu;
}

QMenu *CrunchWindow::buildUnhideMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("unhideMenu"));
    connect(menu, &QMenu::aboutToShow, this, [this, menu] {
        menu->clear();
        int n = 0;
        if (m_loaded && m_loaded->book) {
            const WorkbookInfo &info = m_loaded->book->info();
            for (const int place : info.order) {
                if (info.sheets[place].deleted || info.sheets[place].state == SheetState::Visible) continue;
                ++n;
                const QString name = info.sheets[place].name;
                menu->addAction(name, this, [this, place] {
                    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
                    if (!editingAllowed()) return;
                    const crunch::EditSession::Outcome o = m_session->setSheetHidden(place, false);
                    if (!o.ok) notice(tr("Unhide Sheet"), o.problem);
                });
            }
        }
        if (n == 0) menu->addAction(tr("No hidden sheets"))->setEnabled(false);
    });
    return menu;
}

// The menu of a tab (a right click): the commands for the sheets picked.
void CrunchWindow::showTabMenu(const QPoint &globalPos)
{
    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->setObjectName(QStringLiteral("sheetMenu"));
    menu->addAction(act(QStringLiteral("sheet.insert")));
    menu->addAction(act(QStringLiteral("sheet.delete")));
    menu->addAction(act(QStringLiteral("sheet.rename")));
    menu->addAction(act(QStringLiteral("sheet.moveCopy")));
    menu->addSeparator();
    QMenu *color = buildTabColorMenu(menu);
    color->setTitle(tr("Tab Color"));
    color->setIcon(icon(QStringLiteral("palette")));
    menu->addMenu(color);
    menu->addSeparator();
    menu->addAction(act(QStringLiteral("sheet.hide")));
    QMenu *unhide = buildUnhideMenu(menu);
    unhide->setTitle(tr("Unhide Sheet"));
    unhide->setIcon(icon(QStringLiteral("eye")));
    menu->addMenu(unhide);
    menu->addSeparator();
    menu->addAction(act(QStringLiteral("protect.sheet")));
    menu->popup(globalPos);
}

} // namespace jp
