#include "app/homedialogs.h"

#include "app/plaintext.h"

#include "text/dictionaries.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp::write {

bool sortDialog(QWidget *parent, SortOptions *options)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("sortDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Sort Text"));
    auto *v = new QVBoxLayout(&dlg);
    auto *grid = new QGridLayout;
    v->addLayout(grid);
    grid->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Field:"), &dlg)), 0, 1);
    grid->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Type:"), &dlg)), 0, 2);
    grid->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Order:"), &dlg)), 0, 3);
    const char *const names[] = {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Sort by"), QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Then by"), QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Then by")};
    struct Row { QSpinBox *field; QComboBox *type; QComboBox *order; };
    Row rows[3];
    for (int i = 0; i < 3; ++i) {
        const SortKey key = i < options->keys.size() ? options->keys.at(i) : SortKey();
        grid->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", names[i]), &dlg)), i + 1, 0);
        rows[i].field = new QSpinBox(&dlg);
        rows[i].field->setObjectName(QStringLiteral("sortField%1").arg(i + 1));
        rows[i].field->setRange(1, 99);
        rows[i].field->setValue(key.field);
        rows[i].type = new QComboBox(&dlg);
        rows[i].type->setObjectName(QStringLiteral("sortType%1").arg(i + 1));
        rows[i].type->addItems({QCoreApplication::translate("jp::write::HomeDialogs", "Text"), QCoreApplication::translate("jp::write::HomeDialogs", "Number"), QCoreApplication::translate("jp::write::HomeDialogs", "Date")});
        rows[i].type->setCurrentIndex(int(key.type));
        rows[i].order = new QComboBox(&dlg);
        rows[i].order->setObjectName(QStringLiteral("sortOrder%1").arg(i + 1));
        rows[i].order->addItems({QCoreApplication::translate("jp::write::HomeDialogs", "Ascending"), QCoreApplication::translate("jp::write::HomeDialogs", "Descending")});
        rows[i].order->setCurrentIndex(key.descending ? 1 : 0);
        grid->addWidget(rows[i].field, i + 1, 1);
        grid->addWidget(rows[i].type, i + 1, 2);
        grid->addWidget(rows[i].order, i + 1, 3);
    }
    auto *separator = new QComboBox(&dlg);
    separator->setObjectName(QStringLiteral("sortSeparator"));
    separator->addItems({QCoreApplication::translate("jp::write::HomeDialogs", "Paragraphs (the whole paragraph)"), QCoreApplication::translate("jp::write::HomeDialogs", "Tab-separated fields"), QCoreApplication::translate("jp::write::HomeDialogs", "Comma-separated fields"), QCoreApplication::translate("jp::write::HomeDialogs", "Fields separated by:")});
    auto *other = new QLineEdit(&dlg);
    other->setObjectName(QStringLiteral("sortOther"));
    other->setMaxLength(1);
    other->setEnabled(false);
    int sep = 0;
    if (options->separator == QLatin1Char('\t')) sep = 1;
    else if (options->separator == QLatin1Char(',')) sep = 2;
    else if (!options->separator.isNull()) {
        sep = 3;
        other->setText(QString(options->separator));
        other->setEnabled(true);
    }
    separator->setCurrentIndex(sep);
    QObject::connect(separator, &QComboBox::currentIndexChanged, other, [other](int i) { other->setEnabled(i == 3); });
    auto *fieldsRow = new QHBoxLayout;
    fieldsRow->addWidget(separator, 1);
    fieldsRow->addWidget(other);
    v->addLayout(fieldsRow);
    auto *header = new QCheckBox(QCoreApplication::translate("jp::write::HomeDialogs", "The first paragraph is a header (leave it where it is)"), &dlg);
    header->setObjectName(QStringLiteral("sortHeader"));
    header->setChecked(options->header);
    auto *caseBox = new QCheckBox(QCoreApplication::translate("jp::write::HomeDialogs", "Match case"), &dlg);
    caseBox->setObjectName(QStringLiteral("sortCase"));
    caseBox->setChecked(options->caseSensitive);
    v->addWidget(header);
    v->addWidget(caseBox);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted) return false;
    options->keys.clear();
    options->separator = QChar();
    if (separator->currentIndex() == 1) options->separator = QLatin1Char('\t');
    else if (separator->currentIndex() == 2) options->separator = QLatin1Char(',');
    else if (separator->currentIndex() == 3 && !other->text().isEmpty()) options->separator = other->text().at(0);
    for (int i = 0; i < 3; ++i) {
        SortKey k;
        k.field = rows[i].field->value();
        k.type = SortKey::Type(rows[i].type->currentIndex());
        k.descending = rows[i].order->currentIndex() == 1;
        // (A key below the first is used only when it is not the plain default of an untouched row.)
        if (i == 0 || k.field != 1 || k.type != SortKey::Text || k.descending) options->keys.push_back(k);
    }
    options->header = header->isChecked();
    options->caseSensitive = caseBox->isChecked();
    return true;
}

void wordCountDialog(QWidget *parent, int pages, bool selection, const std::function<WordStats(bool)> &recount)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("wordCountDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Word Count"));
    auto *v = new QVBoxLayout(&dlg);
    auto *title = plainLabel(new QLabel(selection ? QCoreApplication::translate("jp::write::HomeDialogs", "Selected text") : QCoreApplication::translate("jp::write::HomeDialogs", "Whole document"), &dlg));
    QFont bold = title->font();
    bold.setBold(true);
    title->setFont(bold);
    v->addWidget(title);
    auto *grid = new QGridLayout;
    v->addLayout(grid);
    const QStringList labels = {QCoreApplication::translate("jp::write::HomeDialogs", "Pages:"), QCoreApplication::translate("jp::write::HomeDialogs", "Words:"), QCoreApplication::translate("jp::write::HomeDialogs", "Characters (no spaces):"), QCoreApplication::translate("jp::write::HomeDialogs", "Characters (with spaces):"), QCoreApplication::translate("jp::write::HomeDialogs", "Paragraphs:")};
    QVector<QLabel *> values;
    for (int i = 0; i < labels.size(); ++i) {
        grid->addWidget(plainLabel(new QLabel(labels.at(i), &dlg)), i, 0);
        auto *value = plainLabel(new QLabel(&dlg));
        value->setObjectName(QStringLiteral("count%1").arg(i));
        value->setAlignment(Qt::AlignRight);
        grid->addWidget(value, i, 1);
        values << value;
    }
    auto *notes = new QCheckBox(QCoreApplication::translate("jp::write::HomeDialogs", "Include footnotes and endnotes"), &dlg);
    notes->setObjectName(QStringLiteral("countNotes"));
    notes->setChecked(true);
    v->addWidget(notes);
    auto fill = [&] {
        const WordStats s = recount(notes->isChecked());
        values[0]->setText(selection ? QString() : QString::number(pages));
        values[1]->setText(QString::number(s.words));
        values[2]->setText(QString::number(s.charactersNoSpaces));
        values[3]->setText(QString::number(s.charactersWithSpaces));
        values[4]->setText(QString::number(s.paragraphs));
    };
    QObject::connect(notes, &QCheckBox::toggled, &dlg, fill);
    fill();
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    v->addWidget(bb);
    dlg.exec();
}

int pasteSpecialDialog(QWidget *parent, const QStringList &choices)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("pasteSpecialDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Paste Special"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Paste the clipboard as:"), &dlg)));
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("pasteChoices"));
    list->addItems(choices);
    if (list->count()) list->setCurrentRow(0);
    v->addWidget(list);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);
    v->addWidget(bb);
    return dlg.exec() == QDialog::Accepted ? list->currentRow() : -1;
}

bool languageDialog(QWidget *parent, const QString &current, bool noProof, QString *tag, bool *noProofOut)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("languageDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Language"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Mark the selected text as:"), &dlg)));
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("languages"));
    int row = 0;
    for (const dict::Language &l : dict::languages()) {
        auto *item = new QListWidgetItem(l.name, list);
        item->setData(Qt::UserRole, l.code);
        if (l.code == current) row = list->count() - 1;
    }
    list->setCurrentRow(row);
    v->addWidget(list);
    auto *skip = new QCheckBox(QCoreApplication::translate("jp::write::HomeDialogs", "Do not check spelling"), &dlg);
    skip->setObjectName(QStringLiteral("doNotCheck"));
    skip->setChecked(noProof);
    v->addWidget(skip);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted || !list->currentItem()) return false;
    *tag = list->currentItem()->data(Qt::UserRole).toString();
    *noProofOut = skip->isChecked();
    return true;
}

} // namespace jp::write
