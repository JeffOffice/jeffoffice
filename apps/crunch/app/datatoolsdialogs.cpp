#include "app/datatoolsdialogs.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRegularExpression>
#include <QRadioButton>
#include <QVBoxLayout>

namespace jp {

RemoveDuplicatesDialog::RemoveDuplicatesDialog(const QStringList &names, bool hasHeader, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("removeDuplicatesDialog"));
    setWindowTitle(tr("Remove Duplicates"));
    auto *layout = new QVBoxLayout(this);
    auto *text = new QLabel(tr("Rows that have the same values in the columns you check are repeats. The first one stays and the others go."), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::PlainText);
    layout->addWidget(text);
    m_header = new QCheckBox(tr("My data has a header row"), this);
    m_header->setObjectName(QStringLiteral("duplicatesHeader"));
    m_header->setChecked(hasHeader);
    layout->addWidget(m_header);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("duplicatesColumns"));
    m_list->setAccessibleName(tr("Columns to compare"));
    for (const QString &name : names) {
        auto *item = new QListWidgetItem(name, m_list);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }
    layout->addWidget(m_list);
    auto *row = new QHBoxLayout;
    auto *all = new QPushButton(tr("Select All"), this);
    auto *none = new QPushButton(tr("Unselect All"), this);
    row->addWidget(all);
    row->addWidget(none);
    row->addStretch(1);
    layout->addLayout(row);
    connect(all, &QPushButton::clicked, this, [this] { checkAll(true); });
    connect(none, &QPushButton::clicked, this, [this] { checkAll(false); });
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

std::vector<int> RemoveDuplicatesDialog::columns() const
{
    std::vector<int> out;
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->checkState() == Qt::Checked) out.push_back(i);
    return out;
}

bool RemoveDuplicatesDialog::hasHeader() const { return m_header->isChecked(); }

void RemoveDuplicatesDialog::checkAll(bool on)
{
    for (int i = 0; i < m_list->count(); ++i) m_list->item(i)->setCheckState(on ? Qt::Checked : Qt::Unchecked);
}

TextToColumnsDialog::TextToColumnsDialog(const QStringList &lines, const QString &destination, QWidget *parent) : QDialog(parent), m_lines(lines)
{
    setObjectName(QStringLiteral("textToColumnsDialog"));
    setWindowTitle(tr("Text to Columns"));
    auto *layout = new QVBoxLayout(this);
    m_delimited = new QRadioButton(tr("Split at a separator, such as a comma or a tab"), this);
    m_delimited->setObjectName(QStringLiteral("splitDelimited"));
    m_delimited->setChecked(true);
    m_fixed = new QRadioButton(tr("Split at fixed places"), this);
    m_fixed->setObjectName(QStringLiteral("splitFixed"));
    layout->addWidget(m_delimited);
    layout->addWidget(m_fixed);
    auto *form = new QFormLayout;
    auto *separators = new QHBoxLayout;
    m_tab = new QCheckBox(tr("Tab"), this);
    m_semicolon = new QCheckBox(tr("Semicolon"), this);
    m_comma = new QCheckBox(tr("Comma"), this);
    m_space = new QCheckBox(tr("Space"), this);
    m_other = new QCheckBox(tr("Other:"), this);
    m_otherText = new QLineEdit(this);
    m_otherText->setMaxLength(8);
    m_otherText->setMaximumWidth(60);
    m_tab->setObjectName(QStringLiteral("splitTab"));
    m_semicolon->setObjectName(QStringLiteral("splitSemicolon"));
    m_comma->setObjectName(QStringLiteral("splitComma"));
    m_space->setObjectName(QStringLiteral("splitSpace"));
    m_other->setObjectName(QStringLiteral("splitOther"));
    m_otherText->setObjectName(QStringLiteral("splitOtherText"));
    m_otherText->setAccessibleName(tr("Other separator"));
    m_comma->setChecked(true);
    for (QWidget *w : std::initializer_list<QWidget *>{m_tab, m_semicolon, m_comma, m_space, m_other, m_otherText}) separators->addWidget(w);
    separators->addStretch(1);
    form->addRow(tr("Separators:"), separators);
    m_consecutive = new QCheckBox(tr("Treat separators in a row as one"), this);
    m_consecutive->setObjectName(QStringLiteral("splitConsecutive"));
    form->addRow(QString(), m_consecutive);
    m_qualifier = new QComboBox(this);
    m_qualifier->setObjectName(QStringLiteral("splitQualifier"));
    m_qualifier->setAccessibleName(tr("Text in quotes"));
    m_qualifier->addItem(tr("Double quote \""), QStringLiteral("\""));
    m_qualifier->addItem(tr("Single quote '"), QStringLiteral("'"));
    m_qualifier->addItem(tr("None"), QString());
    form->addRow(tr("Text in quotes:"), m_qualifier);
    m_breaks = new QLineEdit(this);
    m_breaks->setObjectName(QStringLiteral("splitBreaks"));
    m_breaks->setAccessibleName(tr("Places to split at"));
    m_breaks->setPlaceholderText(tr("For example 5, 12, 20"));
    form->addRow(tr("Fixed places:"), m_breaks);
    m_format = new QComboBox(this);
    m_format->setObjectName(QStringLiteral("splitFormat"));
    m_format->setAccessibleName(tr("Kind of value"));
    m_format->addItem(tr("General (numbers and dates are read)"));
    m_format->addItem(tr("Text (kept as it is)"));
    form->addRow(tr("Each piece is:"), m_format);
    m_destination = new QLineEdit(destination, this);
    m_destination->setObjectName(QStringLiteral("splitDestination"));
    m_destination->setAccessibleName(tr("First cell for the pieces"));
    form->addRow(tr("Put the pieces from:"), m_destination);
    layout->addLayout(form);
    m_preview = new QLabel(this);
    m_preview->setObjectName(QStringLiteral("splitPreview"));
    m_preview->setTextFormat(Qt::PlainText);
    m_preview->setFrameShape(QFrame::StyledPanel);
    m_preview->setMinimumHeight(70);
    layout->addWidget(m_preview);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    for (QCheckBox *box : {m_tab, m_semicolon, m_comma, m_space, m_other, m_consecutive}) connect(box, &QCheckBox::toggled, this, [this] { updatePreview(); });
    connect(m_delimited, &QRadioButton::toggled, this, [this] { updatePreview(); });
    connect(m_otherText, &QLineEdit::textChanged, this, [this] { updatePreview(); });
    connect(m_breaks, &QLineEdit::textChanged, this, [this] { updatePreview(); });
    connect(m_qualifier, &QComboBox::currentIndexChanged, this, [this] { updatePreview(); });
    updatePreview();
}

crunch::EditSession::SplitSpec TextToColumnsDialog::spec() const
{
    crunch::EditSession::SplitSpec s;
    s.fixedWidth = m_fixed->isChecked();
    if (m_tab->isChecked()) s.delimiters += QLatin1Char('\t');
    if (m_semicolon->isChecked()) s.delimiters += QLatin1Char(';');
    if (m_comma->isChecked()) s.delimiters += QLatin1Char(',');
    if (m_space->isChecked()) s.delimiters += QLatin1Char(' ');
    if (m_other->isChecked()) s.delimiters += m_otherText->text();
    s.consecutiveAsOne = m_consecutive->isChecked();
    const QString q = m_qualifier->currentData().toString();
    s.qualifier = q.isEmpty() ? QChar() : q[0];
    for (const QString &part : m_breaks->text().split(QRegularExpression(QStringLiteral("[,;\\s]+")), Qt::SkipEmptyParts)) {
        bool ok = false;
        const int at = part.toInt(&ok);
        if (ok && at > 1) s.breaks.push_back(at);
    }
    // (every piece gets the one kind: a long enough run of the same entry)
    s.types.assign(64, m_format->currentIndex() == 1 ? 1 : 0);
    return s;
}

QString TextToColumnsDialog::destination() const { return m_destination->text().trimmed(); }

void TextToColumnsDialog::updatePreview()
{
    m_otherText->setEnabled(m_other->isChecked());
    const bool delimited = m_delimited->isChecked();
    for (QWidget *w : std::initializer_list<QWidget *>{m_tab, m_semicolon, m_comma, m_space, m_other, m_consecutive, m_qualifier}) w->setEnabled(delimited);
    m_breaks->setEnabled(!delimited);
    const QVector<QStringList> table = crunch::EditSession::splitText(m_lines.mid(0, 4), spec());
    QStringList rows;
    for (const QStringList &row : table) rows << row.join(QStringLiteral(" | "));
    m_preview->setText(rows.join(QLatin1Char('\n')));
}

} // namespace jp
