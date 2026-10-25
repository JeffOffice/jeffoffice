#include "app/namesdialog.h"

#include "core/formulaspot.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

namespace jp {

using crunch::EditSession;

// ---- the small dialog

NameEditDialog::NameEditDialog(EditSession *session, int id, const EditSession::NameDraft &initial, const QStringList &sheets, QWidget *parent) : QDialog(parent), m_session(session), m_id(id)
{
    setObjectName(QStringLiteral("nameEditDialog"));
    setWindowTitle(id < 0 ? tr("Define Name") : tr("Change Name"));
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    m_name = new QLineEdit(initial.name);
    m_name->setObjectName(QStringLiteral("nameBox"));
    m_name->setAccessibleName(tr("Name"));
    form->addRow(tr("Name:"), m_name);
    m_scope = new QComboBox;
    m_scope->setObjectName(QStringLiteral("nameScope"));
    m_scope->setAccessibleName(tr("Where the name works"));
    m_scope->addItem(tr("Whole workbook"), -1);
    for (int i = 0; i < sheets.size(); ++i)
        if (!sheets[i].isEmpty()) m_scope->addItem(sheets[i], i);
    const int at = m_scope->findData(initial.scope);
    m_scope->setCurrentIndex(at < 0 ? 0 : at);
    m_scope->setEnabled(id < 0);   // (a name stays where it is: to move it, make it again)
    form->addRow(tr("Works on:"), m_scope);
    m_text = new QLineEdit(initial.text);
    m_text->setObjectName(QStringLiteral("nameText"));
    m_text->setAccessibleName(tr("What the name stands for"));
    m_text->setPlaceholderText(tr("A block of cells such as =Sheet1!$A$1:$A$9, a number, or a formula"));
    form->addRow(tr("Stands for:"), m_text);
    m_comment = new QLineEdit(initial.comment);
    m_comment->setObjectName(QStringLiteral("nameComment"));
    m_comment->setAccessibleName(tr("Comment"));
    m_comment->setMaxLength(255);
    form->addRow(tr("Comment:"), m_comment);
    layout->addLayout(form);
    m_problem = new QLabel;
    m_problem->setObjectName(QStringLiteral("nameProblem"));
    m_problem->setWordWrap(true);
    m_problem->setTextFormat(Qt::PlainText);
    m_problem->setAccessibleName(tr("What is wrong"));
    layout->addWidget(m_problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_ok = buttons->button(QDialogButtonBox::Ok);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_name, &QLineEdit::textChanged, this, [this] { check(); });
    connect(m_text, &QLineEdit::textChanged, this, [this] { check(); });
    connect(m_comment, &QLineEdit::textChanged, this, [this] { check(); });
    connect(m_scope, &QComboBox::currentIndexChanged, this, [this] { check(); });
    check();
    m_name->setFocus();
    m_name->selectAll();
}

EditSession::NameDraft NameEditDialog::draft() const
{
    EditSession::NameDraft d;
    d.name = m_name->text().trimmed();
    d.scope = m_scope->currentData().toInt();
    d.text = m_text->text();
    d.comment = m_comment->text();
    return d;
}

// What is wrong is said as it is typed, once there is something to be wrong (an empty box is not scolded before it has been tried).
void NameEditDialog::check()
{
    const EditSession::NameDraft d = draft();
    const bool touched = !d.name.isEmpty() || !d.text.trimmed().isEmpty();
    const QString problem = m_session->nameProblem(d, m_id);
    m_ok->setEnabled(problem.isEmpty());
    m_problem->setText(touched ? problem : QString());
}

// ---- the list

NamesModel::NamesModel(EditSession *session, QObject *parent) : QAbstractTableModel(parent), m_session(session) { reload(); }

void NamesModel::setFilter(const QString &text)
{
    m_filter = text.trimmed();
    reload();
}

QString NamesModel::scopeText(const crunch::NameEntry &e) const
{
    if (e.scope < 0) return tr("Workbook");
    const jp::office::sheet::Workbook *book = m_session->workbook();
    return book && e.scope < book->info().sheets.size() ? book->info().sheets[e.scope].name : QString();
}

void NamesModel::reload()
{
    beginResetModel();
    m_ids.clear();
    m_total = 0;
    m_leftOut = 0;
    const crunch::NameTable &t = m_session->names();
    int looked = 0;
    for (int id = 0; id < t.slots(); ++id) {
        if (!m_session->nameListed(id) || t.at(id).hidden) continue;   // (a hidden name is the file's own business: it is kept, and not listed)
        // (a workbook may hold a hundred thousand names: the first kMaxNamesIndexed are what the list works over)
        if (++looked > crunch::kMaxNamesIndexed) {
            m_leftOut = 1;
            break;
        }
        const crunch::NameEntry &e = t.at(id);
        if (!m_filter.isEmpty()) {
            bool fits = e.name.contains(m_filter, Qt::CaseInsensitive) || e.comment.contains(m_filter, Qt::CaseInsensitive) || scopeText(e).contains(m_filter, Qt::CaseInsensitive);
            if (!fits) {
                QString refers;
                if (const jp::office::sheet::Calculation *calc = m_session->calculation()) calc->nameText(e.scope, e.name, &refers);
                fits = refers.contains(m_filter, Qt::CaseInsensitive);
            }
            if (!fits) continue;
        }
        ++m_total;
        m_ids.push_back(id);
    }
    endResetModel();
}

int NamesModel::rowOf(int id) const
{
    for (size_t i = 0; i < m_ids.size(); ++i)
        if (m_ids[i] == id) return int(i);
    return -1;
}

QVariant NamesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= int(m_ids.size())) return QVariant();
    const int id = m_ids[size_t(index.row())];
    const crunch::NameEntry &e = m_session->names().at(id);
    if (role == Qt::UserRole) return id;
    if (role != Qt::DisplayRole && role != Qt::ToolTipRole) return QVariant();
    switch (index.column()) {
    case NameColumn: return e.name;
    case ValueColumn: return m_session->nameValueOf(id);   // (worked out for the rows that are drawn)
    case RefersColumn: return m_session->nameTextOf(id);
    case ScopeColumn: return scopeText(e);
    case CommentColumn: return e.comment;
    default: return QVariant();
    }
}

QVariant NamesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return QVariant();
    switch (section) {
    case NameColumn: return tr("Name");
    case ValueColumn: return tr("Value");
    case RefersColumn: return tr("Stands for");
    case ScopeColumn: return tr("Works on");
    case CommentColumn: return tr("Comment");
    default: return QVariant();
    }
}

NamesDialog::NamesDialog(EditSession *session, int sheet, const QString &selection, QWidget *parent) : QDialog(parent), m_session(session), m_sheet(sheet), m_selection(selection)
{
    setObjectName(QStringLiteral("namesDialog"));
    setWindowTitle(tr("Names"));
    resize(760, 420);
    auto *layout = new QVBoxLayout(this);
    m_filter = new QLineEdit;
    m_filter->setObjectName(QStringLiteral("namesFilter"));
    m_filter->setAccessibleName(tr("Filter the names"));
    m_filter->setPlaceholderText(tr("Type to show only the names that fit"));
    m_filter->setClearButtonEnabled(true);
    layout->addWidget(m_filter);
    m_model = new NamesModel(session, this);
    m_table = new QTableView;
    m_table->setObjectName(QStringLiteral("namesTable"));
    m_table->setAccessibleName(tr("Names"));
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(NamesModel::NameColumn, 140);
    m_table->setColumnWidth(NamesModel::ValueColumn, 110);
    m_table->setColumnWidth(NamesModel::RefersColumn, 220);
    m_table->setColumnWidth(NamesModel::ScopeColumn, 100);
    layout->addWidget(m_table, 1);
    m_count = new QLabel;
    m_count->setObjectName(QStringLiteral("namesCount"));
    layout->addWidget(m_count);
    m_notice = new QLabel;
    m_notice->setObjectName(QStringLiteral("namesNotice"));
    m_notice->setWordWrap(true);
    m_notice->setTextFormat(Qt::PlainText);
    layout->addWidget(m_notice);
    auto *buttons = new QDialogButtonBox;
    m_new = buttons->addButton(tr("New…"), QDialogButtonBox::ActionRole);
    m_edit = buttons->addButton(tr("Change…"), QDialogButtonBox::ActionRole);
    m_delete = buttons->addButton(tr("Delete"), QDialogButtonBox::ActionRole);
    m_new->setObjectName(QStringLiteral("namesNew"));
    m_edit->setObjectName(QStringLiteral("namesChange"));
    m_delete->setObjectName(QStringLiteral("namesDelete"));
    buttons->addButton(QDialogButtonBox::Close);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_new, &QPushButton::clicked, this, [this] { newName(); });
    connect(m_edit, &QPushButton::clicked, this, [this] { editName(); });
    connect(m_delete, &QPushButton::clicked, this, [this] { deleteName(); });
    connect(m_filter, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_model->setFilter(text);
        update();
    });
    connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this] { update(); });
    connect(m_table, &QTableView::doubleClicked, this, [this] { editName(); });
    update();
}

QStringList NamesDialog::sheetNames() const
{
    QStringList out;
    const jp::office::sheet::Workbook *book = m_session->workbook();
    if (!book) return out;
    for (int i = 0; i < book->info().sheets.size(); ++i) {
        const jp::office::sheet::SheetEntry &e = book->info().sheets[i];
        out << (e.deleted || e.kind != jp::office::sheet::SheetKind::Worksheet ? QString() : e.name);
    }
    return out;
}

void NamesDialog::refresh()
{
    const int keep = m_model->idAt(m_table->currentIndex().row());
    m_model->reload();
    const int row = m_model->rowOf(keep);
    if (row >= 0) m_table->selectRow(row);
    update();
}

void NamesDialog::update()
{
    const int row = m_table->currentIndex().isValid() ? m_table->currentIndex().row() : -1;
    m_edit->setEnabled(row >= 0);
    m_delete->setEnabled(row >= 0);
    m_count->setText(m_model->total() == 0 ? tr("No names.") : tr("%n name(s)", nullptr, m_model->total()));
    m_notice->setText(m_model->leftOut() > 0 ? tr("Only the first %1 names are listed.").arg(crunch::kMaxNamesIndexed) : QString());
}

bool NamesDialog::ask(int id, EditSession::NameDraft *d)
{
    if (m_asker) return m_asker(d);
    NameEditDialog dialog(m_session, id, *d, sheetNames(), this);
    if (dialog.exec() != QDialog::Accepted) return false;
    *d = dialog.draft();
    return true;
}

void NamesDialog::newName()
{
    EditSession::NameDraft d;
    d.text = m_selection.isEmpty() ? QString() : QLatin1Char('=') + m_selection;
    if (!ask(-1, &d)) return;
    int made = -1;
    const EditSession::Outcome out = m_session->defineName(d, &made);
    if (!out.ok) {
        QMessageBox::warning(this, windowTitle(), out.problem);
        return;
    }
    refresh();
    const int row = m_model->rowOf(made);
    if (row >= 0) m_table->selectRow(row);
}

void NamesDialog::editName()
{
    const int id = m_model->idAt(m_table->currentIndex().row());
    if (id < 0) return;
    const crunch::NameEntry &e = m_session->names().at(id);
    EditSession::NameDraft d;
    d.name = e.name;
    d.scope = e.scope;
    d.comment = e.comment;
    d.hidden = e.hidden;
    d.text = m_session->nameTextOf(id);
    if (!ask(id, &d)) return;
    const EditSession::Outcome out = m_session->changeName(id, d);
    if (!out.ok) {
        QMessageBox::warning(this, windowTitle(), out.problem);
        return;
    }
    refresh();
}

void NamesDialog::deleteName()
{
    const int id = m_model->idAt(m_table->currentIndex().row());
    if (id < 0) return;
    const EditSession::Outcome out = m_session->removeName(id);
    if (!out.ok) {
        QMessageBox::warning(this, windowTitle(), out.problem);
        return;
    }
    refresh();
}

} // namespace jp
