#include "app/validationdialog.h"

#include "core/validationedit.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

namespace jp {

using jp::office::sheet::Validation;

ValidationDialog::ValidationDialog(const std::optional<Validation> &rule, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("validationDialog"));
    setWindowTitle(tr("Data Validation"));
    auto *layout = new QVBoxLayout(this);
    auto *tabs = new QTabWidget(this);
    layout->addWidget(tabs);

    auto *settings = new QWidget(tabs);
    auto *form = new QFormLayout(settings);
    m_allow = new QComboBox(settings);
    m_allow->setObjectName(QStringLiteral("validationAllow"));
    m_allow->setAccessibleName(tr("Allow"));
    const struct { const char *text; Validation::Type type; } kinds[] = {
        {QT_TR_NOOP("Any value"), Validation::Type::Any}, {QT_TR_NOOP("Whole number"), Validation::Type::Whole}, {QT_TR_NOOP("Decimal"), Validation::Type::Decimal},
        {QT_TR_NOOP("List"), Validation::Type::List}, {QT_TR_NOOP("Date"), Validation::Type::Date}, {QT_TR_NOOP("Time"), Validation::Type::Time},
        {QT_TR_NOOP("Text length"), Validation::Type::TextLength}, {QT_TR_NOOP("Custom"), Validation::Type::Custom}};
    for (const auto &k : kinds) m_allow->addItem(tr(k.text), int(k.type));
    m_data = new QComboBox(settings);
    m_data->setObjectName(QStringLiteral("validationData"));
    m_data->setAccessibleName(tr("Data"));
    const struct { const char *text; Validation::Compare op; } ops[] = {
        {QT_TR_NOOP("between"), Validation::Compare::Between}, {QT_TR_NOOP("not between"), Validation::Compare::NotBetween}, {QT_TR_NOOP("equal to"), Validation::Compare::Equal},
        {QT_TR_NOOP("not equal to"), Validation::Compare::NotEqual}, {QT_TR_NOOP("greater than"), Validation::Compare::Greater}, {QT_TR_NOOP("less than"), Validation::Compare::Less},
        {QT_TR_NOOP("greater than or equal to"), Validation::Compare::GreaterEqual}, {QT_TR_NOOP("less than or equal to"), Validation::Compare::LessEqual}};
    for (const auto &o : ops) m_data->addItem(tr(o.text), int(o.op));
    m_first = new QLineEdit(settings);
    m_first->setObjectName(QStringLiteral("validationFirst"));
    m_second = new QLineEdit(settings);
    m_second->setObjectName(QStringLiteral("validationSecond"));
    m_firstLabel = new QLabel(settings);
    m_firstLabel->setTextFormat(Qt::PlainText);
    m_secondLabel = new QLabel(settings);
    m_secondLabel->setTextFormat(Qt::PlainText);
    m_blank = new QCheckBox(tr("Ignore blank"), settings);
    m_blank->setObjectName(QStringLiteral("validationBlank"));
    m_dropDown = new QCheckBox(tr("In-cell drop-down"), settings);
    m_dropDown->setObjectName(QStringLiteral("validationDropDown"));
    m_dropDown->setChecked(true);
    form->addRow(tr("Allow:"), m_allow);
    form->addRow(tr("Data:"), m_data);
    form->addRow(m_firstLabel, m_first);
    form->addRow(m_secondLabel, m_second);
    form->addRow(QString(), m_blank);
    form->addRow(QString(), m_dropDown);
    tabs->addTab(settings, tr("Settings"));

    auto *input = new QWidget(tabs);
    auto *inputForm = new QFormLayout(input);
    m_showInput = new QCheckBox(tr("Show this message when the cell is selected"), input);
    m_showInput->setObjectName(QStringLiteral("validationShowInput"));
    m_promptTitle = new QLineEdit(input);
    m_promptTitle->setObjectName(QStringLiteral("validationPromptTitle"));
    m_promptTitle->setMaxLength(32);
    m_prompt = new QPlainTextEdit(input);
    m_prompt->setObjectName(QStringLiteral("validationPrompt"));
    inputForm->addRow(m_showInput);
    inputForm->addRow(tr("Title:"), m_promptTitle);
    inputForm->addRow(tr("Message:"), m_prompt);
    tabs->addTab(input, tr("Input Message"));

    auto *alert = new QWidget(tabs);
    auto *alertForm = new QFormLayout(alert);
    m_showError = new QCheckBox(tr("Show an alert when something that is not allowed is entered"), alert);
    m_showError->setObjectName(QStringLiteral("validationShowError"));
    m_showError->setChecked(true);
    m_style = new QComboBox(alert);
    m_style->setObjectName(QStringLiteral("validationStyle"));
    m_style->addItem(tr("Stop"), int(Validation::Style::Stop));
    m_style->addItem(tr("Warning"), int(Validation::Style::Warning));
    m_style->addItem(tr("Information"), int(Validation::Style::Information));
    m_errorTitle = new QLineEdit(alert);
    m_errorTitle->setObjectName(QStringLiteral("validationErrorTitle"));
    m_errorTitle->setMaxLength(32);
    m_error = new QPlainTextEdit(alert);
    m_error->setObjectName(QStringLiteral("validationError"));
    alertForm->addRow(m_showError);
    alertForm->addRow(tr("Style:"), m_style);
    alertForm->addRow(tr("Title:"), m_errorTitle);
    alertForm->addRow(tr("Message:"), m_error);
    tabs->addTab(alert, tr("Error Alert"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    auto *clear = buttons->addButton(tr("Clear All"), QDialogButtonBox::ResetRole);
    clear->setObjectName(QStringLiteral("validationClearAll"));
    connect(clear, &QPushButton::clicked, this, [this] {
        m_clearAll = true;
        accept();
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    if (rule) {
        m_allow->setCurrentIndex(std::max(0, m_allow->findData(int(rule->type))));
        m_data->setCurrentIndex(std::max(0, m_data->findData(int(rule->compare))));
        m_first->setText(crunch::validationLimitShown(rule->type, rule->formula1));
        m_second->setText(crunch::validationLimitShown(rule->type, rule->formula2));
        m_blank->setChecked(rule->allowBlank);
        m_dropDown->setChecked(rule->dropDown);
        m_showInput->setChecked(rule->showInput);
        m_promptTitle->setText(rule->promptTitle);
        m_prompt->setPlainText(rule->prompt);
        m_showError->setChecked(rule->showError);
        m_style->setCurrentIndex(std::max(0, m_style->findData(int(rule->style))));
        m_errorTitle->setText(rule->errorTitle);
        m_error->setPlainText(rule->error);
    }
    connect(m_allow, &QComboBox::currentIndexChanged, this, &ValidationDialog::refreshBoxes);
    connect(m_data, &QComboBox::currentIndexChanged, this, &ValidationDialog::refreshBoxes);
    refreshBoxes();
}

// What the limits are called and which are shown, for the kind of rule and the comparison.
void ValidationDialog::refreshBoxes()
{
    const auto type = Validation::Type(m_allow->currentData().toInt());
    const auto op = Validation::Compare(m_data->currentData().toInt());
    const bool two = op == Validation::Compare::Between || op == Validation::Compare::NotBetween;
    const bool any = type == Validation::Type::Any;
    const bool single = type == Validation::Type::List || type == Validation::Type::Custom;
    m_data->setEnabled(!any && !single);
    m_blank->setEnabled(!any);
    m_dropDown->setEnabled(type == Validation::Type::List);
    m_first->setEnabled(!any);
    m_firstLabel->setEnabled(!any);
    if (type == Validation::Type::List) m_firstLabel->setText(tr("Source:"));
    else if (type == Validation::Type::Custom) m_firstLabel->setText(tr("Formula:"));
    else m_firstLabel->setText(two ? tr("Minimum:") : tr("Value:"));
    const bool showSecond = !any && !single && two;
    m_second->setVisible(showSecond);
    m_secondLabel->setVisible(showSecond);
    m_secondLabel->setText(tr("Maximum:"));
}

std::optional<Validation> ValidationDialog::rule(QString *problem) const
{
    Validation v;
    v.type = Validation::Type(m_allow->currentData().toInt());
    v.compare = Validation::Compare(m_data->currentData().toInt());
    v.allowBlank = m_blank->isChecked();
    v.dropDown = m_dropDown->isChecked();
    v.showInput = m_showInput->isChecked();
    v.promptTitle = m_promptTitle->text();
    v.prompt = m_prompt->toPlainText();
    v.showError = m_showError->isChecked();
    v.style = Validation::Style(m_style->currentData().toInt());
    v.errorTitle = m_errorTitle->text();
    v.error = m_error->toPlainText();
    if (v.type == Validation::Type::Any) return v;
    const bool single = v.type == Validation::Type::List || v.type == Validation::Type::Custom;
    const bool two = !single && (v.compare == Validation::Compare::Between || v.compare == Validation::Compare::NotBetween);
    bool ok = false;
    v.formula1 = crunch::validationLimitText(v.type, m_first->text(), &ok);
    if (!ok) {
        if (problem) *problem = v.type == Validation::Type::List ? tr("Write the choices with commas between them, or a range of cells after =, such as =A1:A5.")
                                : v.type == Validation::Type::Custom ? tr("Write a formula after =, such as =A1>0.")
                                                                     : tr("Write the %1 as a value, or as a formula after =.").arg(two ? tr("minimum") : tr("value"));
        return std::nullopt;
    }
    if (two) {
        v.formula2 = crunch::validationLimitText(v.type, m_second->text(), &ok);
        if (!ok) {
            if (problem) *problem = tr("Write the maximum as a value, or as a formula after =.");
            return std::nullopt;
        }
    }
    return v;
}

} // namespace jp
