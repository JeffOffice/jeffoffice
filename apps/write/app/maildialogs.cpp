// The dialog boxes of the Mailings tab (app/maildialogs.h).

#include "app/maildialogs.h"

#include "core/mergeops.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp::write {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::write::MailDialogs", text); }

// A line of text, or a text with line breaks, as the preview shows it.
QString asLines(QString s) { return s.replace(QChar::LineSeparator, QLatin1Char('\n')); }

} // namespace

bool addressBlockDialog(QWidget *parent, const MergeSource &data, const QHash<QString, QString> &map, QString *format, QString *homeCountry)
{
    QDialog d(parent);
    d.setWindowTitle(tr("Insert Address Block"));
    auto *v = new QVBoxLayout(&d);
    v->addWidget(new QLabel(tr("Choose the lines of the address. A field in braces, such as {City}, is filled in from each recipient. A line with nothing in it is left out.")));
    auto *formats = new QComboBox(&d);
    for (const QString &f : mail::addressBlockFormats()) formats->addItem(QString(f).replace(QLatin1Char('|'), QLatin1String(" / ")), f);
    formats->setEditable(false);
    v->addWidget(formats);
    auto *text = new QPlainTextEdit(&d);
    text->setFixedHeight(110);
    text->setPlainText((*format).isEmpty() ? mail::addressBlockFormats().first().replace(QLatin1Char('|'), QLatin1Char('\n')) : QString(*format).replace(QLatin1Char('|'), QLatin1Char('\n')));
    v->addWidget(text);
    auto *country = new QLineEdit(*homeCountry, &d);
    country->setPlaceholderText(tr("Leave out the country when it is"));
    v->addWidget(country);
    v->addWidget(new QLabel(tr("Preview (first recipient):")));
    auto *preview = new QLabel(&d);
    preview->setFrameShape(QFrame::StyledPanel);
    preview->setMinimumHeight(90);
    preview->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    v->addWidget(preview);
    auto format0 = [text] { return text->toPlainText().trimmed().replace(QLatin1Char('\n'), QLatin1Char('|')); };
    auto update = [=] {
        const int row = data.includedRows().value(0, 0);
        preview->setText(data.rows.isEmpty() ? tr("(No recipients yet.)") : asLines(mail::addressBlockText(data, row, map, format0(), country->text())));
    };
    QObject::connect(formats, &QComboBox::activated, &d, [=](int i) {
        text->setPlainText(formats->itemData(i).toString().replace(QLatin1Char('|'), QLatin1Char('\n')));
    });
    QObject::connect(text, &QPlainTextEdit::textChanged, &d, update);
    QObject::connect(country, &QLineEdit::textChanged, &d, update);
    update();
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    v->addWidget(bb);
    if (d.exec() != QDialog::Accepted) return false;
    *format = format0();
    *homeCountry = country->text().trimmed();
    return true;
}

bool greetingLineDialog(QWidget *parent, const MergeSource &data, const QHash<QString, QString> &map, QString *format, QString *fallback)
{
    QDialog d(parent);
    d.setWindowTitle(tr("Insert Greeting Line"));
    auto *v = new QVBoxLayout(&d);
    auto *form = new QFormLayout();
    auto *formats = new QComboBox(&d);
    formats->setEditable(true);
    for (const QString &f : mail::greetingFormats()) formats->addItem(f);
    formats->setCurrentText(format->isEmpty() ? mail::greetingFormats().first() : *format);
    form->addRow(tr("Greeting line format:"), formats);
    auto *other = new QLineEdit(fallback->isEmpty() ? tr("Dear Sir or Madam,") : *fallback, &d);
    form->addRow(tr("For a recipient with no name:"), other);
    v->addLayout(form);
    v->addWidget(new QLabel(tr("Use {Title}, {First}, {Last}, and {Suffix} for the parts of the name.")));
    auto *preview = new QLabel(&d);
    preview->setFrameShape(QFrame::StyledPanel);
    v->addWidget(preview);
    auto update = [=] {
        const int row = data.includedRows().value(0, 0);
        preview->setText(data.rows.isEmpty() ? tr("(No recipients yet.)") : mail::greetingText(data, row, map, formats->currentText(), other->text()));
    };
    QObject::connect(formats, &QComboBox::currentTextChanged, &d, update);
    QObject::connect(other, &QLineEdit::textChanged, &d, update);
    update();
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    v->addWidget(bb);
    if (d.exec() != QDialog::Accepted) return false;
    *format = formats->currentText().trimmed();
    *fallback = other->text().trimmed();
    return true;
}

bool matchFieldsDialog(QWidget *parent, const MergeSource &data, QHash<QString, QString> *map)
{
    QDialog d(parent);
    d.setWindowTitle(tr("Match Fields"));
    auto *v = new QVBoxLayout(&d);
    v->addWidget(new QLabel(tr("Tell the address block and the greeting line which column of your list holds each part. \"(not matched)\" uses the usual column names.")));
    auto *form = new QFormLayout();
    const QHash<QString, QString> labels = {
        {QStringLiteral("Title"), tr("Courtesy Title")}, {QStringLiteral("First"), tr("First Name")}, {QStringLiteral("Last"), tr("Last Name")}, {QStringLiteral("Suffix"), tr("Suffix")},
        {QStringLiteral("Company"), tr("Company")},      {QStringLiteral("Street1"), tr("Address 1")}, {QStringLiteral("Street2"), tr("Address 2")}, {QStringLiteral("City"), tr("City")},
        {QStringLiteral("State"), tr("State")},          {QStringLiteral("Postal"), tr("Postal Code")}, {QStringLiteral("Country"), tr("Country or Region")}, {QStringLiteral("Email"), tr("Email Address")}};
    QHash<QString, QComboBox *> boxes;
    for (const QString &logical : mail::logicalFields()) {
        auto *box = new QComboBox(&d);
        box->addItem(tr("(not matched)"), QString());
        for (const QString &f : data.fields) box->addItem(f, f);
        const QString chosen = map->value(logical);
        const int at = box->findData(chosen.isEmpty() ? mail::columnFor(data, QHash<QString, QString>(), logical) : chosen);
        box->setCurrentIndex(at >= 0 ? at : 0);
        boxes.insert(logical, box);
        form->addRow(labels.value(logical) + QLatin1Char(':'), box);
    }
    v->addLayout(form);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    v->addWidget(bb);
    if (d.exec() != QDialog::Accepted) return false;
    map->clear();
    for (auto it = boxes.cbegin(); it != boxes.cend(); ++it) {
        const QString column = it.value()->currentData().toString();
        // (Only a choice that differs from the usual column is kept.)
        if (!column.isEmpty() && column != mail::columnFor(data, QHash<QString, QString>(), it.key())) map->insert(it.key(), column);
    }
    return true;
}

bool ruleDialog(QWidget *parent, const QString &rule, const QStringList &columns, QString *code)
{
    QDialog d(parent);
    const bool condition = rule == QLatin1String("IF") || rule == QLatin1String("SKIPIF") || rule == QLatin1String("NEXTIF");
    d.setWindowTitle(rule == QLatin1String("IF")       ? tr("Insert Field: If...Then...Else")
                     : rule == QLatin1String("ASK")    ? tr("Insert Field: Ask")
                     : rule == QLatin1String("FILLIN") ? tr("Insert Field: Fill-in")
                     : rule == QLatin1String("SET")    ? tr("Insert Field: Set Bookmark")
                     : rule == QLatin1String("SKIPIF") ? tr("Insert Field: Skip Record If")
                                                       : tr("Insert Field: Next Record If"));
    auto *form = new QFormLayout(&d);
    auto *field = new QComboBox(&d);
    field->addItems(columns);
    auto *op = new QComboBox(&d);
    op->addItems({QStringLiteral("="), QStringLiteral("<>"), QStringLiteral("<"), QStringLiteral(">"), QStringLiteral("<="), QStringLiteral(">=")});
    auto *compare = new QLineEdit(&d);
    auto *first = new QLineEdit(&d), *second = new QLineEdit(&d), *third = new QLineEdit(&d);
    if (condition) {
        form->addRow(tr("Field name:"), field);
        form->addRow(tr("Comparison:"), op);
        form->addRow(tr("Compare to:"), compare);
    }
    if (rule == QLatin1String("IF")) {
        form->addRow(tr("Insert this text:"), first);
        form->addRow(tr("Otherwise insert this text:"), second);
    } else if (rule == QLatin1String("ASK")) {
        first->setPlaceholderText(tr("A name for the answer"));
        form->addRow(tr("Bookmark:"), first);
        form->addRow(tr("Prompt:"), second);
        form->addRow(tr("Default answer:"), third);
    } else if (rule == QLatin1String("FILLIN")) {
        form->addRow(tr("Prompt:"), first);
        form->addRow(tr("Default answer:"), third);
    } else if (rule == QLatin1String("SET")) {
        form->addRow(tr("Bookmark:"), first);
        form->addRow(tr("Value:"), second);
    }
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    form->addRow(bb);
    if (d.exec() != QDialog::Accepted) return false;
    auto q = [](QString s) { return QLatin1Char('"') + s.replace(QLatin1Char('"'), QLatin1Char('\'')) + QLatin1Char('"'); };
    const QString test = condition ? QLatin1String("\"{ ") + mail::mergeFieldCode(field->currentText()) + QLatin1String(" }\" ") + op->currentText() + QLatin1Char(' ') + q(compare->text()) : QString();
    if (rule == QLatin1String("IF")) *code = QStringLiteral("IF ") + test + QLatin1Char(' ') + q(first->text()) + QLatin1Char(' ') + q(second->text());
    else if (rule == QLatin1String("SKIPIF") || rule == QLatin1String("NEXTIF")) *code = rule + QLatin1Char(' ') + test;
    else if (rule == QLatin1String("ASK")) {
        QString name = first->text().trimmed();
        name.replace(QLatin1Char(' '), QLatin1Char('_'));
        if (name.isEmpty()) return false;
        *code = QStringLiteral("ASK ") + name + QLatin1Char(' ') + q(second->text()) + (third->text().isEmpty() ? QString() : QStringLiteral(" \\d ") + q(third->text()));
    } else if (rule == QLatin1String("FILLIN")) *code = QStringLiteral("FILLIN ") + q(first->text()) + (third->text().isEmpty() ? QString() : QStringLiteral(" \\d ") + q(third->text()));
    else {
        QString name = first->text().trimmed();
        name.replace(QLatin1Char(' '), QLatin1Char('_'));
        if (name.isEmpty()) return false;
        *code = QStringLiteral("SET ") + name + QLatin1Char(' ') + q(second->text());
    }
    return true;
}

bool messageDialog(QWidget *parent, const QStringList &columns, QString *column, QString *subject, QString *folder)
{
    QDialog d(parent);
    d.setWindowTitle(tr("Merge to Email Messages"));
    auto *v = new QVBoxLayout(&d);
    v->addWidget(new QLabel(tr("One message file (.eml) is saved for each recipient. They are not sent: open them in your email program.")));
    auto *form = new QFormLayout();
    auto *box = new QComboBox(&d);
    box->addItems(columns);
    for (int i = 0; i < columns.size(); ++i)
        if (columns[i].contains(QLatin1String("mail"), Qt::CaseInsensitive)) {
            box->setCurrentIndex(i);
            break;
        }
    form->addRow(tr("To:"), box);
    auto *subj = new QLineEdit(*subject, &d);
    form->addRow(tr("Subject line:"), subj);
    auto *where = new QLineEdit(*folder, &d);
    auto *browse = new QPushButton(tr("Browse…"), &d);
    auto *row = new QHBoxLayout();
    row->addWidget(where, 1);
    row->addWidget(browse);
    form->addRow(tr("Folder:"), row);
    QObject::connect(browse, &QPushButton::clicked, &d, [&] {
        const QString dir = QFileDialog::getExistingDirectory(&d, tr("Choose a Folder"), where->text());
        if (!dir.isEmpty()) where->setText(dir);
    });
    v->addLayout(form);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    v->addWidget(bb);
    if (d.exec() != QDialog::Accepted || where->text().trimmed().isEmpty()) return false;
    *column = box->currentText();
    *subject = subj->text();
    *folder = where->text().trimmed();
    return true;
}

bool envelopeDialog(QWidget *parent, bool merging, EnvelopeChoice *choice)
{
    QDialog d(parent);
    d.setWindowTitle(tr("Envelopes"));
    auto *v = new QVBoxLayout(&d);
    auto *form = new QFormLayout();
    auto *block = new QCheckBox(tr("Use the address block of the merge"), &d);
    block->setVisible(merging);
    block->setChecked(merging && choice->useBlock);
    form->addRow(block);
    auto *delivery = new QPlainTextEdit(choice->delivery, &d);
    delivery->setFixedHeight(90);
    form->addRow(tr("Delivery address:"), delivery);
    auto *ret = new QPlainTextEdit(choice->returnAddress, &d);
    ret->setFixedHeight(70);
    form->addRow(tr("Return address:"), ret);
    auto *sizes = new QComboBox(&d);
    for (const mail::EnvelopeSize &s : mail::envelopeSizes()) sizes->addItem(s.name);
    sizes->setCurrentIndex(std::clamp(choice->size, 0, int(sizes->count()) - 1));
    form->addRow(tr("Envelope size:"), sizes);
    v->addLayout(form);
    QObject::connect(block, &QCheckBox::toggled, delivery, &QWidget::setDisabled);
    delivery->setDisabled(block->isChecked());
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Cancel, &d);
    auto *add = bb->addButton(tr("Add to Document"), QDialogButtonBox::AcceptRole);
    auto *print = bb->addButton(tr("Print"), QDialogButtonBox::AcceptRole);
    bool printNow = false;
    QObject::connect(print, &QPushButton::clicked, &d, [&] { printNow = true; });
    Q_UNUSED(add)
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    v->addWidget(bb);
    if (d.exec() != QDialog::Accepted) return false;
    choice->useBlock = block->isChecked();
    choice->delivery = delivery->toPlainText();
    choice->returnAddress = ret->toPlainText();
    choice->size = sizes->currentIndex();
    choice->print = printNow;
    return true;
}

bool labelsDialog(QWidget *parent, bool merging, LabelChoice *choice)
{
    QDialog d(parent);
    d.setWindowTitle(tr("Labels"));
    auto *v = new QVBoxLayout(&d);
    auto *form = new QFormLayout();
    auto *block = new QCheckBox(tr("Use the address block of the merge"), &d);
    block->setVisible(merging);
    block->setChecked(merging && choice->useBlock);
    form->addRow(block);
    auto *address = new QPlainTextEdit(choice->address, &d);
    address->setFixedHeight(90);
    form->addRow(tr("Address:"), address);
    auto *sheets = new QComboBox(&d);
    for (const mail::LabelSheet &s : mail::labelSheets()) sheets->addItem(s.name);
    sheets->addItem(tr("Your own sheet…"));
    sheets->setCurrentIndex(choice->sheet < 0 ? sheets->count() - 1 : std::clamp(choice->sheet, 0, int(sheets->count()) - 2));
    form->addRow(tr("Label sheet:"), sheets);
    // Your own sheet: the sizes in inches.
    auto *own = new QWidget(&d);
    auto *ownForm = new QFormLayout(own);
    ownForm->setContentsMargins(0, 0, 0, 0);
    auto spin = [&](double value, double max) {
        auto *s = new QDoubleSpinBox(own);
        s->setRange(0, max);
        s->setDecimals(3);
        s->setSingleStep(0.125);
        s->setSuffix(tr(" in"));
        s->setValue(value);
        return s;
    };
    auto *columns = new QSpinBox(own), *rows = new QSpinBox(own);
    columns->setRange(1, 20);
    rows->setRange(1, 60);
    columns->setValue(choice->custom.columns);
    rows->setValue(choice->custom.rows);
    auto *w = spin(choice->custom.labelW / 72, 20), *h = spin(choice->custom.labelH / 72, 20), *top = spin(choice->custom.top / 72, 10), *left = spin(choice->custom.left / 72, 10),
         *hp = spin(choice->custom.hPitch / 72, 20), *vp = spin(choice->custom.vPitch / 72, 20);
    ownForm->addRow(tr("Labels across:"), columns);
    ownForm->addRow(tr("Labels down:"), rows);
    ownForm->addRow(tr("Label width:"), w);
    ownForm->addRow(tr("Label height:"), h);
    ownForm->addRow(tr("Top margin:"), top);
    ownForm->addRow(tr("Side margin:"), left);
    ownForm->addRow(tr("Horizontal pitch:"), hp);
    ownForm->addRow(tr("Vertical pitch:"), vp);
    own->setVisible(sheets->currentIndex() == sheets->count() - 1);
    QObject::connect(sheets, &QComboBox::currentIndexChanged, &d, [=](int i) { own->setVisible(i == sheets->count() - 1); });
    form->addRow(own);
    auto *full = new QRadioButton(tr("Full page of the same label"), &d);
    auto *one = new QRadioButton(tr("Single label"), &d);
    full->setChecked(!choice->single);
    one->setChecked(choice->single);
    auto *rowBox = new QSpinBox(&d), *colBox = new QSpinBox(&d);
    rowBox->setRange(1, 60);
    colBox->setRange(1, 20);
    rowBox->setValue(choice->row);
    colBox->setValue(choice->column);
    auto *place = new QHBoxLayout();
    place->addWidget(one);
    place->addWidget(new QLabel(tr("Row:")));
    place->addWidget(rowBox);
    place->addWidget(new QLabel(tr("Column:")));
    place->addWidget(colBox);
    place->addStretch(1);
    form->addRow(full);
    form->addRow(place);
    v->addLayout(form);
    QObject::connect(block, &QCheckBox::toggled, address, &QWidget::setDisabled);
    address->setDisabled(block->isChecked());
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Cancel, &d);
    bb->addButton(tr("New Document"), QDialogButtonBox::AcceptRole);
    auto *print = bb->addButton(tr("Print"), QDialogButtonBox::AcceptRole);
    bool printNow = false;
    QObject::connect(print, &QPushButton::clicked, &d, [&] { printNow = true; });
    QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    v->addWidget(bb);
    if (d.exec() != QDialog::Accepted) return false;
    choice->useBlock = block->isChecked();
    choice->address = address->toPlainText();
    choice->sheet = sheets->currentIndex() == sheets->count() - 1 ? -1 : sheets->currentIndex();
    mail::LabelSheet c;
    c.name = tr("Your own sheet");
    c.pageW = 8.5 * 72;
    c.pageH = 11 * 72;
    c.columns = columns->value();
    c.rows = rows->value();
    c.labelW = w->value() * 72;
    c.labelH = h->value() * 72;
    c.top = top->value() * 72;
    c.left = left->value() * 72;
    c.hPitch = std::max(c.labelW, hp->value() * 72);
    c.vPitch = std::max(c.labelH, vp->value() * 72);
    choice->custom = c;
    choice->single = one->isChecked();
    choice->row = rowBox->value();
    choice->column = colBox->value();
    choice->print = printNow;
    return true;
}

} // namespace jp::write
