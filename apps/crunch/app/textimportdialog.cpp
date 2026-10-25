#include "app/textimportdialog.h"

#include "sheet/csv.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace jp {

using namespace jp::office::sheet;

namespace {

enum EncodingChoice { Detect, Utf8, Utf8Mark, Utf16Le, Utf16Be, Win1252, Win1250, Win1251, Win1253, Win1254, Shift932, Gb936, Euc949, Big950 };

struct Code {
    EncodingChoice id;
    const char *label;
    CsvEncoding encoding;
    int codePage;
};

const Code kCodes[] = {
    {Utf8, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Unicode (UTF-8)"), CsvEncoding::Utf8, 65001},
    {Utf8Mark, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Unicode (UTF-8 with a mark)"), CsvEncoding::Utf8Bom, 65001},
    {Utf16Le, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Unicode (UTF-16, little end first)"), CsvEncoding::Utf16Le, 1200},
    {Utf16Be, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Unicode (UTF-16, big end first)"), CsvEncoding::Utf16Be, 1201},
    {Win1252, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Western European (1252)"), CsvEncoding::Legacy, 1252},
    {Win1250, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Central European (1250)"), CsvEncoding::Legacy, 1250},
    {Win1251, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Cyrillic (1251)"), CsvEncoding::Legacy, 1251},
    {Win1253, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Greek (1253)"), CsvEncoding::Legacy, 1253},
    {Win1254, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Turkish (1254)"), CsvEncoding::Legacy, 1254},
    {Shift932, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Japanese (932)"), CsvEncoding::Legacy, 932},
    {Gb936, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Chinese, simplified (936)"), CsvEncoding::Legacy, 936},
    {Euc949, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Korean (949)"), CsvEncoding::Legacy, 949},
    {Big950, QT_TRANSLATE_NOOP("jp::TextImportDialog", "Chinese, traditional (950)"), CsvEncoding::Legacy, 950},
};

} // namespace

TextImportDialog::TextImportDialog(const TextImportRequest &request, QWidget *parent) : QDialog(parent), m_request(request)
{
    setObjectName(QStringLiteral("textImportDialog"));
    setWindowTitle(tr("Open a Text File"));
    resize(820, 560);
    auto *layout = new QVBoxLayout(this);
    auto *name = new QLabel(tr("%1 (%2 bytes)").arg(QFileInfo(request.path).fileName(), QLocale().toString(request.size)), this);
    name->setObjectName(QStringLiteral("textImportName"));
    layout->addWidget(name);
    auto *form = new QFormLayout;
    m_encoding = new QComboBox(this);
    m_encoding->setObjectName(QStringLiteral("textImportEncoding"));
    m_encoding->setAccessibleName(tr("Text encoding"));
    m_encoding->addItem(tr("Detect it"), int(Detect));
    for (const Code &c : kCodes) m_encoding->addItem(QCoreApplication::translate("jp::TextImportDialog", c.label), int(c.id));
    form->addRow(tr("Text encoding:"), m_encoding);
    auto *separators = new QHBoxLayout;
    auto box = [&](const QString &label, const QString &id, bool on) {
        auto *b = new QCheckBox(label, this);
        b->setObjectName(id);
        b->setChecked(on);
        separators->addWidget(b);
        return b;
    };
    // what the file is found to use
    CsvForm found;
    csvPreview(request.head, request.path, CsvImport(), request.locale, &found);
    const QString use = request.choices.separators.isEmpty() ? QString(found.delimiter) : request.choices.separators;
    m_comma = box(tr("Comma"), QStringLiteral("sepComma"), use.contains(QLatin1Char(',')));
    m_semicolon = box(tr("Semicolon"), QStringLiteral("sepSemicolon"), use.contains(QLatin1Char(';')));
    m_tab = box(tr("Tab"), QStringLiteral("sepTab"), use.contains(QLatin1Char('\t')));
    m_space = box(tr("Space"), QStringLiteral("sepSpace"), use.contains(QLatin1Char(' ')));
    m_pipe = box(tr("Bar (|)"), QStringLiteral("sepPipe"), use.contains(QLatin1Char('|')));
    m_other = new QLineEdit(this);
    m_other->setObjectName(QStringLiteral("sepOther"));
    m_other->setAccessibleName(tr("Another separator"));
    m_other->setMaxLength(4);
    m_other->setMaximumWidth(60);
    QString others = use;
    for (const QChar c : QStringLiteral(",;\t |")) others.remove(c);
    m_other->setText(others);
    separators->addWidget(new QLabel(tr("Other:"), this));
    separators->addWidget(m_other);
    separators->addStretch(1);
    form->addRow(tr("Fields are separated by:"), separators);
    m_merge = new QCheckBox(tr("Count several separators in a row as one"), this);
    m_merge->setObjectName(QStringLiteral("sepMerge"));
    m_merge->setChecked(request.choices.mergeSeparators);
    form->addRow(QString(), m_merge);
    m_quote = new QComboBox(this);
    m_quote->setObjectName(QStringLiteral("textImportQuote"));
    m_quote->setAccessibleName(tr("Text quote"));
    m_quote->addItem(tr("Double quote (\")"), int('"'));
    m_quote->addItem(tr("Single quote (')"), int('\''));
    m_quote->addItem(tr("None"), 0);
    m_quote->setCurrentIndex(std::max(0, m_quote->findData(request.choices.quote.unicode())));
    form->addRow(tr("Text is in:"), m_quote);
    m_start = new QSpinBox(this);
    m_start->setObjectName(QStringLiteral("textImportStart"));
    m_start->setAccessibleName(tr("First row to bring in"));
    m_start->setRange(1, 1000000);
    m_start->setValue(request.choices.firstRow + 1);
    form->addRow(tr("Begin at row:"), m_start);
    layout->addLayout(form);
    m_note = new QLabel(this);
    m_note->setObjectName(QStringLiteral("textImportNote"));
    m_note->setWordWrap(true);
    layout->addWidget(m_note);
    m_table = new QTableWidget(this);
    m_table->setObjectName(QStringLiteral("textImportPreview"));
    m_table->setAccessibleName(tr("Preview of the first rows"));
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    layout->addWidget(m_table, 1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    m_columns = request.choices.columns;
    auto again = [this] { refresh(); };
    for (QCheckBox *b : {m_comma, m_semicolon, m_tab, m_space, m_pipe, m_merge}) connect(b, &QCheckBox::toggled, this, again);
    connect(m_other, &QLineEdit::textChanged, this, again);
    connect(m_encoding, &QComboBox::currentIndexChanged, this, again);
    connect(m_quote, &QComboBox::currentIndexChanged, this, again);
    connect(m_start, &QSpinBox::valueChanged, this, again);
    refresh();
}

CsvImport TextImportDialog::choices() const
{
    CsvImport c = m_request.choices;
    QString sep;
    if (m_comma->isChecked()) sep += QLatin1Char(',');
    if (m_semicolon->isChecked()) sep += QLatin1Char(';');
    if (m_tab->isChecked()) sep += QLatin1Char('\t');
    if (m_space->isChecked()) sep += QLatin1Char(' ');
    if (m_pipe->isChecked()) sep += QLatin1Char('|');
    for (const QChar ch : m_other->text())
        if (!sep.contains(ch) && !ch.isSpace()) sep += ch;
    // (no separator at all: the whole of a line is one field)
    c.separators = sep.isEmpty() ? QString(QChar(0x1F)) : sep;
    c.mergeSeparators = m_merge->isChecked();
    c.quote = QChar(ushort(m_quote->currentData().toInt()));
    c.firstRow = m_start->value() - 1;
    const int enc = m_encoding->currentData().toInt();
    c.encodingSet = enc != Detect;
    for (const Code &code : kCodes)
        if (int(code.id) == enc) {
            c.encoding = code.encoding;
            c.codePage = code.codePage;
        }
    c.columns = m_columns;
    return c;
}

int TextImportDialog::previewRows() const { return std::max(0, m_table->rowCount() - 1); }
int TextImportDialog::previewColumns() const { return m_table->columnCount(); }

void TextImportDialog::refresh()
{
    if (m_filling) return;
    m_filling = true;
    const CsvImport c = choices();
    int columns = 0;
    const QVector<QStringList> rows = csvPreview(m_request.head, m_request.path, c, m_request.locale, nullptr, &columns);
    m_table->clear();
    m_table->setRowCount(int(rows.size()) + 1);
    m_table->setColumnCount(columns);
    m_kinds.clear();
    m_table->setVerticalHeaderItem(0, new QTableWidgetItem(tr("Read as")));
    for (int r = 0; r < rows.size(); ++r) m_table->setVerticalHeaderItem(r + 1, new QTableWidgetItem(QString::number(c.firstRow + r + 1)));
    QStringList heads;
    for (int col = 0; col < columns; ++col) heads << columnName(quint32(col));
    m_table->setHorizontalHeaderLabels(heads);
    for (int r = 0; r < rows.size(); ++r)
        for (int col = 0; col < rows[r].size(); ++col) {
            auto *item = new QTableWidgetItem(rows[r][col]);
            if (col < c.columns.size() && c.columns[col].kind == CsvColumn::Kind::Skip) item->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
            m_table->setItem(r + 1, col, item);
        }
    fillKinds();
    m_table->resizeColumnsToContents();
    for (int col = 0; col < columns; ++col) m_table->setColumnWidth(col, std::min(m_table->columnWidth(col), 220));
    const bool bounded = m_request.size > kCsvPreviewBytes || rows.size() >= kCsvPreviewRows;
    m_note->setText(bounded ? tr("The preview shows the first %1 rows of the file. The whole file is opened.").arg(rows.size()) : QString());
    m_filling = false;
}

// The box above each column that says how it is read.
void TextImportDialog::fillKinds()
{
    const int columns = m_table->columnCount();
    while (m_columns.size() < columns) m_columns.append(CsvColumn());
    for (int col = 0; col < columns; ++col) {
        auto *box = new QComboBox;
        box->setObjectName(QStringLiteral("columnKind%1").arg(col));
        box->setAccessibleName(tr("How column %1 is read").arg(columnName(quint32(col))));
        box->addItem(tr("As typed"), 0);
        box->addItem(tr("Text"), 1);
        box->addItem(tr("Date: month, day, year"), 2);
        box->addItem(tr("Date: day, month, year"), 3);
        box->addItem(tr("Date: year, month, day"), 4);
        box->addItem(tr("Leave out"), 5);
        const CsvColumn &k = m_columns[col];
        int index = 0;
        switch (k.kind) {
        case CsvColumn::Kind::General: index = 0; break;
        case CsvColumn::Kind::Text: index = 1; break;
        case CsvColumn::Kind::Date: index = k.dateOrder == QLatin1String("DMY") ? 3 : k.dateOrder == QLatin1String("YMD") ? 4 : 2; break;
        case CsvColumn::Kind::Skip: index = 5; break;
        }
        box->setCurrentIndex(index);
        connect(box, &QComboBox::currentIndexChanged, this, [this, col](int i) {
            if (m_filling) return;
            CsvColumn k;
            switch (i) {
            case 1: k.kind = CsvColumn::Kind::Text; break;
            case 2: k.kind = CsvColumn::Kind::Date; k.dateOrder = QStringLiteral("MDY"); break;
            case 3: k.kind = CsvColumn::Kind::Date; k.dateOrder = QStringLiteral("DMY"); break;
            case 4: k.kind = CsvColumn::Kind::Date; k.dateOrder = QStringLiteral("YMD"); break;
            case 5: k.kind = CsvColumn::Kind::Skip; break;
            default: break;
            }
            m_columns[col] = k;
            QMetaObject::invokeMethod(this, [this] { refresh(); }, Qt::QueuedConnection);   // (the boxes are made again: not while one is telling us)
        });
        m_table->setCellWidget(0, col, box);
        m_kinds.push_back(box);
    }
    m_table->setRowHeight(0, 28);
}

} // namespace jp
