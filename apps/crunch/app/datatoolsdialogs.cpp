#include "app/uistyle.h"
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
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp {

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
    m_other = new QCheckBox(tr("Other"), this);
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
    form->addRow(tr("Separators"), separators);
    m_consecutive = new QCheckBox(tr("Treat separators in a row as one"), this);
    m_consecutive->setObjectName(QStringLiteral("splitConsecutive"));
    form->addRow(QString(), m_consecutive);
    m_qualifier = new QComboBox(this);
    m_qualifier->setObjectName(QStringLiteral("splitQualifier"));
    m_qualifier->setAccessibleName(tr("Text in quotes"));
    m_qualifier->addItem(tr("Double quote \""), QStringLiteral("\""));
    m_qualifier->addItem(tr("Single quote '"), QStringLiteral("'"));
    m_qualifier->addItem(tr("None"), QString());
    form->addRow(tr("Text in quotes"), m_qualifier);
    m_breaks = new QLineEdit(this);
    m_breaks->setObjectName(QStringLiteral("splitBreaks"));
    m_breaks->setAccessibleName(tr("Places to split at"));
    m_breaks->setPlaceholderText(tr("For example 5, 12, 20"));
    form->addRow(tr("Fixed places"), m_breaks);
    m_format = new QComboBox(this);
    m_format->setObjectName(QStringLiteral("splitFormat"));
    m_format->setAccessibleName(tr("Kind of value"));
    m_format->addItem(tr("General (numbers and dates are read)"));
    m_format->addItem(tr("Text (kept as it is)"));
    form->addRow(tr("Each piece is"), m_format);
    // one piece at a time: how it is read (the kinds SplitSpec::types has), or left out
    m_pieceTypes.assign(64, -1);
    auto *pieceRow = new QHBoxLayout;
    m_piece = new QSpinBox(this);
    m_piece->setObjectName(QStringLiteral("splitPiece"));
    m_piece->setAccessibleName(tr("Piece number"));
    m_piece->setRange(1, int(m_pieceTypes.size()));
    m_pieceKind = new QComboBox(this);
    m_pieceKind->setObjectName(QStringLiteral("splitPieceKind"));
    m_pieceKind->setAccessibleName(tr("How this piece is read"));
    m_pieceKind->addItem(tr("Same as above"), -1);
    m_pieceKind->addItem(tr("As typed"), 0);
    m_pieceKind->addItem(tr("Text"), 1);
    m_pieceKind->addItem(tr("Date: month, day, year"), 3);
    m_pieceKind->addItem(tr("Date: day, month, year"), 4);
    m_pieceKind->addItem(tr("Date: year, month, day"), 5);
    m_pieceKind->addItem(tr("Leave out"), 2);
    pieceRow->addWidget(m_piece);
    pieceRow->addWidget(m_pieceKind, 1);
    form->addRow(tr("Piece"), pieceRow);
    connect(m_piece, &QSpinBox::valueChanged, this, [this](int n) {
        m_pieceKind->setCurrentIndex(std::max(0, m_pieceKind->findData(m_pieceTypes[size_t(n - 1)])));
    });
    connect(m_pieceKind, &QComboBox::currentIndexChanged, this, [this] {
        m_pieceTypes[size_t(m_piece->value() - 1)] = m_pieceKind->currentData().toInt();
        updatePreview();
    });
    m_destination = new QLineEdit(destination, this);
    m_destination->setObjectName(QStringLiteral("splitDestination"));
    m_destination->setAccessibleName(tr("First cell for the pieces"));
    form->addRow(tr("Put the pieces from"), m_destination);
    layout->addLayout(form);
    m_preview = new QLabel(this);
    m_preview->setObjectName(QStringLiteral("splitPreview"));
    m_preview->setTextFormat(Qt::PlainText);
    jp::softPanel(m_preview);
    m_preview->setMinimumHeight(70);
    layout->addWidget(m_preview);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
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
    // (the kind of "Each piece is" for the pieces that were not told otherwise: a long enough run of the same entry)
    s.types.assign(m_pieceTypes.size(), m_format->currentIndex() == 1 ? 1 : 0);
    for (size_t i = 0; i < m_pieceTypes.size(); ++i)
        if (m_pieceTypes[i] >= 0) s.types[i] = m_pieceTypes[i];
    return s;
}

QString TextToColumnsDialog::destination() const { return m_destination->text().trimmed(); }

void TextToColumnsDialog::updatePreview()
{
    m_otherText->setEnabled(m_other->isChecked());
    const bool delimited = m_delimited->isChecked();
    for (QWidget *w : std::initializer_list<QWidget *>{m_tab, m_semicolon, m_comma, m_space, m_other, m_consecutive, m_qualifier}) w->setEnabled(delimited);
    m_breaks->setEnabled(!delimited);
    const crunch::EditSession::SplitSpec chosen = spec();
    const QVector<QStringList> table = crunch::EditSession::splitText(m_lines.mid(0, 4), chosen);
    QStringList rows;
    for (const QStringList &row : table) {
        QStringList kept;   // (the pieces that are left out are not shown)
        for (int i = 0; i < row.size(); ++i)
            if (!(i < int(chosen.types.size()) && chosen.types[size_t(i)] == 2)) kept << row[i];
        rows << kept.join(QStringLiteral(" | "));
    }
    m_preview->setText(rows.join(QLatin1Char('\n')));
}

} // namespace jp
