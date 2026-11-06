#include "app/protectdialogs.h"

#include "sheet/cellref.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

using jp::office::sheet::SheetProtection;

namespace {

struct Item {
    const char *id;
    const char *label;
    bool SheetProtection::*field;
};

// What a protected sheet can still allow, in the order the dialog lists it.
const Item kItems[] = {
    {"formatRows", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Change row heights or hide rows"), &SheetProtection::allowFormatRows},
    {"formatColumns", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Change column widths or hide columns"), &SheetProtection::allowFormatColumns},
    {"insertRows", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Add new rows"), &SheetProtection::allowInsertRows},
    {"insertColumns", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Add new columns"), &SheetProtection::allowInsertColumns},
    {"deleteRows", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Remove rows"), &SheetProtection::allowDeleteRows},
    {"deleteColumns", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Remove columns"), &SheetProtection::allowDeleteColumns},
    {"formatCells", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Change how cells look"), &SheetProtection::allowFormatCells},
    {"insertLinks", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Add or remove links"), &SheetProtection::allowInsertLinks},
    {"sort", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Sort ranges"), &SheetProtection::allowSort},
    {"filter", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Filter lists"), &SheetProtection::allowFilter},
    {"pivot", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Work with pivot tables"), &SheetProtection::allowPivot},
    {"selectUnlocked", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Click cells that are unlocked"), &SheetProtection::allowSelectUnlocked},
    {"selectLocked", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Click cells that are locked"), &SheetProtection::allowSelectLocked},
};

} // namespace

SetFilePasswordDialog::SetFilePasswordDialog(bool hasPassword, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("setFilePasswordDialog"));
    setWindowTitle(tr("Protect with Password"));
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(hasPassword ? tr("This workbook is protected with a password. Type a new one to change it, or remove the password.")
                                         : tr("Anyone who opens this workbook will have to type the password. The password protects the file when you save it."), this);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *form = new QFormLayout;
    m_password = new QLineEdit(this);
    m_password->setObjectName(QStringLiteral("filePassword"));
    m_password->setAccessibleName(tr("Password"));
    m_password->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Password:"), m_password);
    m_again = new QLineEdit(this);
    m_again->setObjectName(QStringLiteral("filePasswordAgain"));
    m_again->setAccessibleName(tr("Password again"));
    m_again->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Type it again:"), m_again);
    layout->addLayout(form);
    auto *warn = new QLabel(tr("A password can't be found again if it is forgotten. Keep it somewhere safe."), this);
    warn->setWordWrap(true);
    layout->addWidget(warn);
    m_problem = plainLabel(new QLabel(this));
    m_problem->setObjectName(QStringLiteral("filePasswordProblem"));
    m_problem->setWordWrap(true);
    layout->addWidget(m_problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Protect"));
    if (hasPassword) {
        QPushButton *remove = buttons->addButton(tr("Remove Password"), QDialogButtonBox::DestructiveRole);
        remove->setObjectName(QStringLiteral("filePasswordRemove"));
        connect(remove, &QPushButton::clicked, this, [this] {
            m_removed = true;
            QDialog::accept();
        });
    }
    connect(buttons, &QDialogButtonBox::accepted, this, &SetFilePasswordDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void SetFilePasswordDialog::clearPasswords()
{
    m_password->clear();
    m_again->clear();
}

QString SetFilePasswordDialog::password() const { return m_password->text(); }

void SetFilePasswordDialog::accept()
{
    if (m_password->text().isEmpty()) {
        m_problem->setText(tr("Type a password, or cancel."));
        return;
    }
    if (m_password->text() != m_again->text()) {
        m_problem->setText(tr("The two passwords are not the same."));
        return;
    }
    QDialog::accept();
}

ProtectSheetDialog::ProtectSheetDialog(const ProtectRequest &request, QWidget *parent) : QDialog(parent), m_request(request)
{
    setObjectName(QStringLiteral("protectSheetDialog"));
    setWindowTitle(tr("Protect Sheet"));
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("A protected sheet can't be changed in its locked cells. Choose what everyone may still do."), this);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *form = new QFormLayout;
    m_password = new QLineEdit(this);
    m_password->setObjectName(QStringLiteral("protectPassword"));
    m_password->setAccessibleName(tr("Password"));
    m_password->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Password (optional):"), m_password);
    m_again = new QLineEdit(this);
    m_again->setObjectName(QStringLiteral("protectPasswordAgain"));
    m_again->setAccessibleName(tr("Password again"));
    m_again->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Type it again:"), m_again);
    layout->addLayout(form);
    auto *warn = new QLabel(tr("A password can't be found again if it is forgotten. Keep it somewhere safe."), this);
    warn->setWordWrap(true);
    layout->addWidget(warn);
    layout->addWidget(new QLabel(tr("Anyone may also:"), this));
    for (const Item &item : kItems) {
        auto *box = new QCheckBox(QCoreApplication::translate("jp::ProtectSheetDialog", item.label), this);
        box->setObjectName(QStringLiteral("protect_") + QLatin1String(item.id));
        box->setChecked(request.settings.*(item.field));
        layout->addWidget(box);
        m_checks.insert(QLatin1String(item.id), box);
    }
    auto *objects = new QCheckBox(tr("Edit notes and drawing objects"), this);
    objects->setObjectName(QStringLiteral("protect_objects"));
    objects->setChecked(!request.settings.objects);
    layout->addWidget(objects);
    m_checks.insert(QStringLiteral("objects"), objects);
    m_problem = plainLabel(new QLabel(this));
    m_problem->setObjectName(QStringLiteral("protectProblem"));
    m_problem->setWordWrap(true);
    layout->addWidget(m_problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        QString problem;
        ProtectRequest probe = m_request;
        if (!result(&probe, &problem)) {
            m_problem->setText(problem);
            return;
        }
        accept();
    });
    layout->addWidget(buttons);
    m_password->setFocus();
}

bool ProtectSheetDialog::result(ProtectRequest *request, QString *problem) const
{
    if (m_password->text() != m_again->text()) {
        *problem = tr("The two passwords are not the same. Type the same password twice.");
        return false;
    }
    SheetProtection p = request->settings;
    p.enabled = true;
    for (const Item &item : kItems) p.*(item.field) = m_checks.value(QLatin1String(item.id))->isChecked();
    p.objects = !m_checks.value(QStringLiteral("objects"))->isChecked();
    request->settings = p;
    request->password = m_password->text();
    return true;
}

ProtectWorkbookDialog::ProtectWorkbookDialog(const WorkbookProtectRequest &request, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("protectWorkbookDialog"));
    setWindowTitle(tr("Protect Workbook"));
    auto *layout = new QVBoxLayout(this);
    m_structure = new QCheckBox(tr("Lock the sheets: no sheet can be added, deleted, renamed, moved, hidden, or shown"), this);
    m_structure->setObjectName(QStringLiteral("protectStructure"));
    m_structure->setChecked(request.lockStructure);
    layout->addWidget(m_structure);
    m_windows = new QCheckBox(tr("Keep the windows as they are"), this);
    m_windows->setObjectName(QStringLiteral("protectWindows"));
    m_windows->setChecked(request.lockWindows);
    layout->addWidget(m_windows);
    auto *form = new QFormLayout;
    m_password = new QLineEdit(this);
    m_password->setObjectName(QStringLiteral("protectPassword"));
    m_password->setAccessibleName(tr("Password"));
    m_password->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Password (optional):"), m_password);
    m_again = new QLineEdit(this);
    m_again->setObjectName(QStringLiteral("protectPasswordAgain"));
    m_again->setAccessibleName(tr("Password again"));
    m_again->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Type it again:"), m_again);
    layout->addLayout(form);
    auto *warn = new QLabel(tr("A password can't be found again if it is forgotten. Keep it somewhere safe."), this);
    warn->setWordWrap(true);
    layout->addWidget(warn);
    m_problem = plainLabel(new QLabel(this));
    m_problem->setObjectName(QStringLiteral("protectProblem"));
    m_problem->setWordWrap(true);
    layout->addWidget(m_problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        WorkbookProtectRequest probe;
        QString problem;
        if (!result(&probe, &problem)) {
            m_problem->setText(problem);
            return;
        }
        accept();
    });
    layout->addWidget(buttons);
}

bool ProtectWorkbookDialog::result(WorkbookProtectRequest *request, QString *problem) const
{
    if (m_password->text() != m_again->text()) {
        *problem = tr("The two passwords are not the same. Type the same password twice.");
        return false;
    }
    if (!m_structure->isChecked() && !m_windows->isChecked()) {
        *problem = tr("Choose what to lock.");
        return false;
    }
    request->lockStructure = m_structure->isChecked();
    request->lockWindows = m_windows->isChecked();
    request->password = m_password->text();
    return true;
}

} // namespace jp

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::EditRange;

EditRangesDialog::EditRangesDialog(const EditRangesRequest &request, QWidget *parent) : QDialog(parent), m_request(request)
{
    setObjectName(QStringLiteral("editRangesDialog"));
    setWindowTitle(tr("Allow Edit Ranges"));
    auto *layout = new QVBoxLayout(this);
    auto *note = new QLabel(request.sheetProtected ? tr("These ranges can be changed on this protected sheet, with the password if one is set.")
                                                    : tr("These ranges can be changed once the sheet is protected, with the password if one is set."), this);
    note->setWordWrap(true);
    layout->addWidget(note);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("editRangesList"));
    m_list->setAccessibleName(tr("Ranges"));
    layout->addWidget(m_list);
    auto *row = new QHBoxLayout;
    auto *add = new QPushButton(tr("New…"), this);
    add->setObjectName(QStringLiteral("editRangesNew"));
    auto *modify = new QPushButton(tr("Modify…"), this);
    modify->setObjectName(QStringLiteral("editRangesModify"));
    auto *remove = new QPushButton(tr("Delete"), this);
    remove->setObjectName(QStringLiteral("editRangesDelete"));
    row->addWidget(add);
    row->addWidget(modify);
    row->addWidget(remove);
    row->addStretch(1);
    layout->addLayout(row);
    connect(add, &QPushButton::clicked, this, [this] { ask(-1); });
    connect(modify, &QPushButton::clicked, this, [this] {
        if (m_list->currentRow() >= 0) ask(m_list->currentRow());
    });
    connect(remove, &QPushButton::clicked, this, [this] { removeRow(m_list->currentRow()); });
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);
    reload();
}

void EditRangesDialog::reload()
{
    const int keep = m_list->currentRow();
    m_list->clear();
    for (const EditRange &r : std::as_const(m_request.ranges)) {
        const bool password = r.password.isSet() || m_request.passwords.contains(r.title);
        m_list->addItem(password ? tr("%1: %2 (password)").arg(r.title, r.sqref) : tr("%1: %2").arg(r.title, r.sqref));
    }
    if (keep >= 0 && keep < m_list->count()) m_list->setCurrentRow(keep);
}

bool EditRangesDialog::setRange(int row, const EditRange &range, const QString &typedPassword, bool removePassword, QString *problem)
{
    EditRange r = range;
    r.title = r.title.trimmed();
    r.sqref = r.sqref.simplified().replace(QLatin1Char(','), QLatin1Char(' ')).replace(QLatin1Char('$'), QString());
    if (r.title.isEmpty()) {
        if (problem) *problem = tr("The range needs a title.");
        return false;
    }
    for (int i = 0; i < m_request.ranges.size(); ++i)
        if (i != row && m_request.ranges[i].title.compare(r.title, Qt::CaseInsensitive) == 0) {
            if (problem) *problem = tr("Another range has the title \"%1\".").arg(r.title);
            return false;
        }
    const QStringList parts = r.sqref.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    bool good = !parts.isEmpty();
    for (const QString &part : parts) {
        CellRange cells;
        if (!jp::office::sheet::parseRangeRef(part, &cells)) good = false;
    }
    if (!good) {
        if (problem) *problem = tr("Type the cells as a range such as A1:C5, with a space between ranges.");
        return false;
    }
    if (m_request.ranges.size() >= jp::office::sheet::kMaxEditRanges && row < 0) {
        if (problem) *problem = tr("A sheet can have up to %1 ranges.").arg(jp::office::sheet::kMaxEditRanges);
        return false;
    }
    if (row >= 0 && row < m_request.ranges.size()) {
        const EditRange old = m_request.ranges[row];
        r.password = removePassword ? jp::office::sheet::PasswordHash() : old.password;
        r.securityDescriptor = old.securityDescriptor;
        m_request.passwords.remove(old.title);
        m_request.removePasswords.removeAll(old.title);
        m_request.ranges[row] = r;
    } else {
        m_request.ranges.append(r);
    }
    if (!typedPassword.isEmpty()) m_request.passwords.insert(r.title, typedPassword);
    else if (removePassword) m_request.removePasswords.append(r.title);
    reload();
    return true;
}

void EditRangesDialog::removeRow(int row)
{
    if (row < 0 || row >= m_request.ranges.size()) return;
    const QString title = m_request.ranges[row].title;
    m_request.ranges.removeAt(row);
    m_request.passwords.remove(title);
    m_request.removePasswords.removeAll(title);
    reload();
}

// The small box that asks for one range: its title, its cells, and its password (typed twice).
void EditRangesDialog::ask(int row)
{
    QDialog box(this);
    box.setObjectName(QStringLiteral("editRangeDialog"));
    box.setWindowTitle(row < 0 ? tr("New Range") : tr("Modify Range"));
    auto *layout = new QVBoxLayout(&box);
    auto *form = new QFormLayout;
    auto *title = new QLineEdit(&box);
    title->setObjectName(QStringLiteral("editRangeTitle"));
    title->setAccessibleName(tr("Title"));
    auto *cells = new QLineEdit(&box);
    cells->setObjectName(QStringLiteral("editRangeCells"));
    cells->setAccessibleName(tr("Refers to cells"));
    auto *password = new QLineEdit(&box);
    password->setObjectName(QStringLiteral("editRangePassword"));
    password->setAccessibleName(tr("Range password"));
    password->setEchoMode(QLineEdit::Password);
    auto *again = new QLineEdit(&box);
    again->setObjectName(QStringLiteral("editRangePasswordAgain"));
    again->setAccessibleName(tr("Range password again"));
    again->setEchoMode(QLineEdit::Password);
    auto *remove = new QCheckBox(tr("Take the password off"), &box);
    remove->setObjectName(QStringLiteral("editRangeRemovePassword"));
    const bool had = row >= 0 && (m_request.ranges[row].password.isSet() || m_request.passwords.contains(m_request.ranges[row].title));
    remove->setVisible(had);
    if (row >= 0) {
        title->setText(m_request.ranges[row].title);
        cells->setText(m_request.ranges[row].sqref);
    } else {
        title->setText(tr("Range%1").arg(m_request.ranges.size() + 1));
        cells->setText(m_request.selection);
    }
    form->addRow(tr("Title:"), title);
    form->addRow(tr("Refers to cells:"), cells);
    form->addRow(had ? tr("New password (optional):") : tr("Password (optional):"), password);
    form->addRow(tr("Type it again:"), again);
    layout->addLayout(form);
    layout->addWidget(remove);
    QLabel *problem = plainLabel(new QLabel(&box));
    problem->setWordWrap(true);
    layout->addWidget(problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &box);
    connect(buttons, &QDialogButtonBox::rejected, &box, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &box, [&] {
        if (password->text() != again->text()) {
            problem->setText(tr("The two passwords are not the same."));
            return;
        }
        EditRange r;
        r.title = title->text();
        r.sqref = cells->text();
        QString why;
        if (!setRange(row, r, password->text(), remove->isChecked(), &why)) {
            problem->setText(why);
            return;
        }
        box.accept();
    });
    layout->addWidget(buttons);
    box.exec();
    password->clear();
    again->clear();
}

} // namespace jp
