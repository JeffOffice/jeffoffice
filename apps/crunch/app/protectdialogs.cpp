#include "app/protectdialogs.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

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
    {"selectLocked", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Select locked cells"), &SheetProtection::allowSelectLocked},
    {"selectUnlocked", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Select unlocked cells"), &SheetProtection::allowSelectUnlocked},
    {"formatCells", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Format cells"), &SheetProtection::allowFormatCells},
    {"formatColumns", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Format columns (width, hiding)"), &SheetProtection::allowFormatColumns},
    {"formatRows", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Format rows (height, hiding)"), &SheetProtection::allowFormatRows},
    {"insertColumns", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Insert columns"), &SheetProtection::allowInsertColumns},
    {"insertRows", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Insert rows"), &SheetProtection::allowInsertRows},
    {"insertLinks", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Put in links"), &SheetProtection::allowInsertLinks},
    {"deleteColumns", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Delete columns"), &SheetProtection::allowDeleteColumns},
    {"deleteRows", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Delete rows"), &SheetProtection::allowDeleteRows},
    {"sort", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Sort"), &SheetProtection::allowSort},
    {"filter", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Use filters"), &SheetProtection::allowFilter},
    {"pivot", QT_TRANSLATE_NOOP("jp::ProtectSheetDialog", "Use PivotTables"), &SheetProtection::allowPivot},
};

} // namespace

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
    layout->addWidget(new QLabel(tr("Everyone may:"), this));
    for (const Item &item : kItems) {
        auto *box = new QCheckBox(QCoreApplication::translate("jp::ProtectSheetDialog", item.label), this);
        box->setObjectName(QStringLiteral("protect_") + QLatin1String(item.id));
        box->setChecked(request.settings.*(item.field));
        layout->addWidget(box);
        m_checks.insert(QLatin1String(item.id), box);
    }
    auto *objects = new QCheckBox(tr("Change drawing objects and notes"), this);
    objects->setObjectName(QStringLiteral("protect_objects"));
    objects->setChecked(!request.settings.objects);
    layout->addWidget(objects);
    m_checks.insert(QStringLiteral("objects"), objects);
    m_problem = new QLabel(this);
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
    m_problem = new QLabel(this);
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
