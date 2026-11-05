#include "app/csvsavedialog.h"


#include <QCoreApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

namespace {

constexpr int kOther = -1;

struct Code {
    int codePage;
    const char *label;
};
const Code kPages[] = {{1252, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Western European (1252)")}, {1250, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Central European (1250)")},
                       {1251, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Cyrillic (1251)")},        {1253, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Greek (1253)")},
                       {1254, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Turkish (1254)")},         {28591, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Western European (ISO 8859-1)")},
                       {28592, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Central European (ISO 8859-2)")}, {28605, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Western European (ISO 8859-15)")},
                       {10000, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Mac Roman")},
                       {932, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Japanese (932)")}, {936, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Chinese, simplified (936)")},
                       {949, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Korean (949)")}, {950, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Chinese, traditional (950)")},
                       {1255, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Hebrew (1255)")}, {1256, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Arabic (1256)")},
                       {1257, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Baltic (1257)")}, {1258, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Vietnamese (1258)")},
                       {874, QT_TRANSLATE_NOOP("jp::CsvSaveDialog", "Thai (874)")}};

} // namespace

CsvSaveDialog::CsvSaveDialog(const QString &explanation, const crunch::CsvWrite &start, QWidget *parent) : QDialog(parent), m_start(start)
{
    setObjectName(QStringLiteral("csvSaveDialog"));
    setWindowTitle(tr("Save"));
    auto *layout = new QVBoxLayout(this);
    auto *text = new QLabel(explanation, this);
    text->setObjectName(QStringLiteral("csvSaveText"));
    text->setWordWrap(true);
    text->setTextFormat(Qt::PlainText);
    text->setMinimumWidth(420);
    layout->addWidget(text);
    auto *form = new QFormLayout;
    m_separator = new QComboBox(this);
    m_separator->setObjectName(QStringLiteral("csvSeparator"));
    m_separator->setAccessibleName(tr("Separator between cells"));
    m_separator->addItem(tr("Comma"), QStringLiteral(","));
    m_separator->addItem(tr("Semicolon"), QStringLiteral(";"));
    m_separator->addItem(tr("Tab"), QStringLiteral("\t"));
    m_separator->addItem(tr("Space"), QStringLiteral(" "));
    m_separator->addItem(tr("Vertical bar"), QStringLiteral("|"));
    m_separator->addItem(tr("Another character…"), kOther);
    m_other = new QLineEdit(this);
    m_other->setObjectName(QStringLiteral("csvOtherSeparator"));
    m_other->setAccessibleName(tr("The other separator"));
    m_other->setMaxLength(1);
    m_other->setMaximumWidth(60);
    auto *sepRow = new QHBoxLayout;
    sepRow->addWidget(m_separator);
    sepRow->addWidget(m_other);
    sepRow->addStretch();
    form->addRow(tr("Separator:"), sepRow);
    m_encoding = new QComboBox(this);
    m_encoding->setObjectName(QStringLiteral("csvEncoding"));
    m_encoding->setAccessibleName(tr("Text encoding"));
    m_encoding->addItem(tr("Unicode (UTF-8 with a mark)"), -1);
    m_encoding->addItem(tr("Unicode (UTF-8)"), -2);
    m_encoding->addItem(tr("Unicode (UTF-16, little end first)"), -3);
    m_encoding->addItem(tr("Unicode (UTF-16, big end first)"), -4);
    for (const Code &c : kPages) m_encoding->addItem(QCoreApplication::translate("jp::CsvSaveDialog", c.label), c.codePage);
    form->addRow(tr("Text encoding:"), m_encoding);
    m_lineEnd = new QComboBox(this);
    m_lineEnd->setObjectName(QStringLiteral("csvLineEnd"));
    m_lineEnd->setAccessibleName(tr("How lines end"));
    m_lineEnd->addItem(tr("Windows (carriage return and line feed)"), QStringLiteral("\r\n"));
    m_lineEnd->addItem(tr("Linux and Mac (line feed)"), QStringLiteral("\n"));
    m_lineEnd->addItem(tr("Old Mac (carriage return)"), QStringLiteral("\r"));
    form->addRow(tr("Line ends:"), m_lineEnd);
    m_values = new QComboBox(this);
    m_values->setObjectName(QStringLiteral("csvValues"));
    m_values->setAccessibleName(tr("What the cells are written as"));
    m_values->addItem(tr("As shown in the cells"), false);
    m_values->addItem(tr("As stored (every digit, dates as ISO text)"), true);
    form->addRow(tr("Cells:"), m_values);
    layout->addLayout(form);
    // the choices now
    const int sep = m_separator->findData(QString(start.separator));
    if (sep >= 0) {
        m_separator->setCurrentIndex(sep);
    } else {
        m_separator->setCurrentIndex(m_separator->findData(kOther));
        m_other->setText(QString(start.separator));
    }
    using Enc = crunch::CsvWrite::Encoding;
    m_encoding->setCurrentIndex(m_encoding->findData(start.encoding == Enc::Utf8Mark ? -1 : start.encoding == Enc::Utf8 ? -2 : start.encoding == Enc::Utf16Le ? -3 : start.encoding == Enc::Utf16Be ? -4 : start.codePage));
    if (m_encoding->currentIndex() < 0) m_encoding->setCurrentIndex(0);
    m_lineEnd->setCurrentIndex(std::max(0, m_lineEnd->findData(start.lineEnd)));
    m_values->setCurrentIndex(start.asStored ? 1 : 0);
    refreshOther();
    connect(m_separator, &QComboBox::currentIndexChanged, this, [this] { refreshOther(); });
    auto *buttons = new QDialogButtonBox(this);
    QPushButton *go = buttons->addButton(tr("Save Values Only"), QDialogButtonBox::AcceptRole);
    go->setObjectName(QStringLiteral("csvSaveValues"));
    QPushButton *xlsx = buttons->addButton(tr("Save as .xlsx…"), QDialogButtonBox::ActionRole);
    xlsx->setObjectName(QStringLiteral("csvSaveXlsx"));
    QPushButton *cancel = buttons->addButton(QDialogButtonBox::Cancel);
    xlsx->setDefault(true);   // (the workbook is the one that loses nothing)
    layout->addWidget(buttons);
    connect(go, &QPushButton::clicked, this, [this] { m_answer = Answer::SaveValues; accept(); });
    connect(xlsx, &QPushButton::clicked, this, [this] { m_answer = Answer::SaveXlsx; accept(); });
    connect(cancel, &QPushButton::clicked, this, [this] { m_answer = Answer::Cancel; reject(); });
}

void CsvSaveDialog::refreshOther() { m_other->setEnabled(m_separator->currentData().toInt() == kOther && m_separator->currentData().typeId() == QMetaType::Int); }

crunch::CsvWrite CsvSaveDialog::options() const
{
    crunch::CsvWrite out = m_start;
    const QVariant sep = m_separator->currentData();
    if (sep.typeId() == QMetaType::QString) out.separator = sep.toString().front();
    else if (!m_other->text().isEmpty()) out.separator = m_other->text().front();
    const int code = m_encoding->currentData().toInt();
    using Enc = crunch::CsvWrite::Encoding;
    out.encoding = code == -1 ? Enc::Utf8Mark : code == -2 ? Enc::Utf8 : code == -3 ? Enc::Utf16Le : code == -4 ? Enc::Utf16Be : Enc::CodePage;
    if (code > 0) out.codePage = code;
    out.lineEnd = m_lineEnd->currentData().toString();
    out.asStored = m_values->currentData().toBool();
    out.sameAsFile = m_start.sameAsFile && out == m_start;
    return out;
}

} // namespace jp
