#include "app/homedialogs.h"

#include "app/pageview.h"
#include "app/plaintext.h"
#include "core/accessops.h"

#include "app/proofingdialogs.h"
#include "text/dictionaries.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp::write {

bool sortDialog(QWidget *parent, SortOptions *options, bool inTable)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("sortDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Sort Text"));
    auto *v = new QVBoxLayout(&dlg);
    auto *grid = new QGridLayout;
    v->addLayout(grid);
    grid->addWidget(plainLabel(new QLabel(inTable ? QCoreApplication::translate("jp::write::HomeDialogs", "Column:") : QCoreApplication::translate("jp::write::HomeDialogs", "Field:"), &dlg)), 0, 1);
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
    separator->setVisible(!inTable);
    other->setVisible(!inTable);
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
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Count Words"));
    auto *v = new QVBoxLayout(&dlg);
    auto *title = plainLabel(new QLabel(selection ? QCoreApplication::translate("jp::write::HomeDialogs", "Selected text") : QCoreApplication::translate("jp::write::HomeDialogs", "Whole document"), &dlg));
    QFont bold = title->font();
    bold.setBold(true);
    title->setFont(bold);
    v->addWidget(title);
    auto *grid = new QGridLayout;
    v->addLayout(grid);
    const QStringList labels = {QCoreApplication::translate("jp::write::HomeDialogs", "Pages:"), QCoreApplication::translate("jp::write::HomeDialogs", "Words:"), QCoreApplication::translate("jp::write::HomeDialogs", "Characters (no spaces):"), QCoreApplication::translate("jp::write::HomeDialogs", "Characters (with spaces):"), QCoreApplication::translate("jp::write::HomeDialogs", "Paragraphs:"), QCoreApplication::translate("jp::write::HomeDialogs", "Lines:")};
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
        values[5]->setText(QString::number(s.lines));
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

QString thesaurusDialog(QWidget *parent, const QString &word)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("thesaurusDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Thesaurus"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Look up:"), &dlg)));
    auto *query = new QLineEdit(word, &dlg);
    query->setObjectName(QStringLiteral("thesaurusQuery"));
    v->addWidget(query);
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("thesaurusList"));
    v->addWidget(list);
    auto fill = [&] {
        list->clear();
        const QStringList found = thesaurusLookup(query->text());
        if (found.isEmpty()) {
            auto *none = new QListWidgetItem(query->text().trimmed().isEmpty() ? QCoreApplication::translate("jp::write::HomeDialogs", "(Type a word to look up)") : QCoreApplication::translate("jp::write::HomeDialogs", "(No suggestions)"), list);
            none->setFlags(Qt::NoItemFlags);
            return;
        }
        for (const QString &entry : found) {
            // (An entry is the word and, in parentheses, the kind of word it is: "glad (adj)".)
            QString text = entry;
            const int paren = text.lastIndexOf(QLatin1String(" ("));
            auto *item = new QListWidgetItem(entry, list);
            if (paren > 0) text.truncate(paren);
            item->setData(Qt::UserRole, text);
        }
        list->setCurrentRow(0);
    };
    fill();
    auto *buttons = new QDialogButtonBox(&dlg);
    auto *replace = buttons->addButton(QCoreApplication::translate("jp::write::HomeDialogs", "Replace"), QDialogButtonBox::AcceptRole);
    replace->setObjectName(QStringLiteral("thesaurusReplace"));
    auto *lookUp = buttons->addButton(QCoreApplication::translate("jp::write::HomeDialogs", "Look Up"), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close);
    QObject::connect(lookUp, &QPushButton::clicked, &dlg, [&] {
        if (list->currentItem() && list->currentItem()->data(Qt::UserRole).isValid()) query->setText(list->currentItem()->data(Qt::UserRole).toString());
        fill();
    });
    QObject::connect(query, &QLineEdit::returnPressed, &dlg, fill);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);
    v->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted || !list->currentItem()) return QString();
    return list->currentItem()->data(Qt::UserRole).toString();
}

bool translateDialog(QWidget *parent, bool wholeDocument, QString *targetCode, QString *server, int textLength)
{
    struct Target { const char *name; const char *code; };
    static const Target kTargets[] = {{QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "English"), "en"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Spanish"), "es"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "French"), "fr"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "German"), "de"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Italian"), "it"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Portuguese"), "pt"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Dutch"), "nl"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Polish"), "pl"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Russian"), "ru"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Ukrainian"), "uk"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Arabic"), "ar"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Hindi"), "hi"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Chinese (Simplified)"), "zh-Hans"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Japanese"), "ja"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Korean"), "ko"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Vietnamese"), "vi"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Turkish"), "tr"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Greek"), "el"}};
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("translateDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Translate"));
    auto *v = new QVBoxLayout(&dlg);
    auto *about = plainLabel(new QLabel(wholeDocument ? QCoreApplication::translate("jp::write::HomeDialogs", "Translate the whole document into:") : QCoreApplication::translate("jp::write::HomeDialogs", "Translate the selected text into:"), &dlg));
    v->addWidget(about);
    auto *target = new QComboBox(&dlg);
    target->setObjectName(QStringLiteral("translateTarget"));
    for (const Target &t : kTargets) {
        target->addItem(QCoreApplication::translate("jp::write::HomeDialogs", t.name), QString::fromLatin1(t.code));
        if (*targetCode == QLatin1String(t.code)) target->setCurrentIndex(target->count() - 1);
    }
    v->addWidget(target);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Translation service (a LibreTranslate address):"), &dlg)));
    auto *address = new QLineEdit(*server, &dlg);
    address->setObjectName(QStringLiteral("translateServer"));
    v->addWidget(address);
    auto *note = plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Your web browser will open that address with the text. Nothing is sent until you click OK. Only secure (https) addresses are used, and http ones for this computer."), &dlg));
    note->setWordWrap(true);
    v->addWidget(note);
    if (textLength > kMostTranslateLetters) {
        auto *cut = plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "The text has %1 letters. Only the first %2 will be sent to be translated.").arg(textLength).arg(kMostTranslateLetters), &dlg));
        cut->setObjectName(QStringLiteral("translateCutNote"));
        cut->setWordWrap(true);
        v->addWidget(cut);
    }
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted) return false;
    *targetCode = target->currentData().toString();
    *server = address->text().trimmed();
    return true;
}

int restrictEditingDialog(QWidget *parent, bool enforced, const QString &currentEdit, bool hasPassword, bool lockTracking, QString *edit, QString *password)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("restrictEditingDialog"));
    dlg.setWindowTitle(lockTracking ? QCoreApplication::translate("jp::write::HomeDialogs", "Lock Tracking") : QCoreApplication::translate("jp::write::HomeDialogs", "Restrict Editing"));
    auto *v = new QVBoxLayout(&dlg);
    struct Kind { const char *edit; const char *text; };
    static const Kind kKinds[] = {{"readOnly", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "No changes (read only)")},
                                  {"trackedChanges", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Tracked changes: all edits are tracked, and Track Changes cannot be turned off")},
                                  {"comments", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Comments: people can add comments but cannot change the text")},
                                  {"forms", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Filling in forms: the text is read only")}};
    QComboBox *kind = nullptr;
    QLineEdit *password1 = nullptr, *password2 = nullptr;
    auto *note = plainLabel(new QLabel(&dlg));
    note->setWordWrap(true);
    if (enforced) {
        QString name = currentEdit;
        for (const Kind &k : kKinds)
            if (currentEdit == QLatin1String(k.edit)) name = QCoreApplication::translate("jp::write::HomeDialogs", k.text);
        note->setText(QCoreApplication::translate("jp::write::HomeDialogs", "This document is protected: %1").arg(name));
        v->addWidget(note);
        if (hasPassword) {
            v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Password to stop the protection:"), &dlg)));
            password1 = new QLineEdit(&dlg);
            password1->setObjectName(QStringLiteral("protectPassword"));
            password1->setEchoMode(QLineEdit::Password);
            v->addWidget(password1);
        }
    } else {
        note->setText(lockTracking ? QCoreApplication::translate("jp::write::HomeDialogs", "Every change will be tracked, and Track Changes cannot be turned off without the password.")
                                   : QCoreApplication::translate("jp::write::HomeDialogs", "Choose what people may do in this document."));
        v->addWidget(note);
        kind = new QComboBox(&dlg);
        kind->setObjectName(QStringLiteral("protectKind"));
        for (const Kind &k : kKinds) {
            if (lockTracking && QLatin1String(k.edit) != QLatin1String("trackedChanges")) continue;
            kind->addItem(QCoreApplication::translate("jp::write::HomeDialogs", k.text), QString::fromLatin1(k.edit));
        }
        v->addWidget(kind);
        v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Password (you can leave this empty):"), &dlg)));
        password1 = new QLineEdit(&dlg);
        password1->setObjectName(QStringLiteral("protectPassword"));
        password1->setEchoMode(QLineEdit::Password);
        v->addWidget(password1);
        v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Type it again:"), &dlg)));
        password2 = new QLineEdit(&dlg);
        password2->setObjectName(QStringLiteral("protectPassword2"));
        password2->setEchoMode(QLineEdit::Password);
        v->addWidget(password2);
    }
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Cancel, &dlg);
    auto *go = bb->addButton(enforced ? QCoreApplication::translate("jp::write::HomeDialogs", "Stop Protection") : QCoreApplication::translate("jp::write::HomeDialogs", "Start Enforcing"), QDialogButtonBox::AcceptRole);
    go->setObjectName(QStringLiteral("protectGo"));
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, [&] {
        if (password2 && password1->text() != password2->text()) {
            note->setText(QCoreApplication::translate("jp::write::HomeDialogs", "The two passwords are not the same."));
            return;
        }
        dlg.accept();
    });
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted) return 0;
    if (password) *password = password1 ? password1->text() : QString();
    if (enforced) return 2;
    *edit = kind->currentData().toString();
    return 1;
}

bool trackOptionsDialog(QWidget *parent, QString *name, QString *initials)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("trackOptionsDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Track Changes Options"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Changes and comments are signed with this name and these initials."), &dlg)));
    auto *form = new QGridLayout;
    v->addLayout(form);
    auto *nameBox = new QLineEdit(*name, &dlg);
    nameBox->setObjectName(QStringLiteral("userName"));
    auto *initialsBox = new QLineEdit(*initials, &dlg);
    initialsBox->setObjectName(QStringLiteral("userInitials"));
    form->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "User name:"), &dlg)), 0, 0);
    form->addWidget(nameBox, 0, 1);
    form->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Initials:"), &dlg)), 1, 0);
    form->addWidget(initialsBox, 1, 1);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted) return false;
    *name = nameBox->text().trimmed();
    *initials = initialsBox->text().trimmed();
    return true;
}

bool compareDialog(QWidget *parent, QString *revisedPath, CompareOptions *options)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("compareDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Compare"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "This document is the original. Choose the revised version:"), &dlg)));
    auto *row = new QHBoxLayout;
    auto *path = new QLineEdit(*revisedPath, &dlg);
    path->setObjectName(QStringLiteral("revisedPath"));
    auto *browse = new QPushButton(QCoreApplication::translate("jp::write::HomeDialogs", "Browse…"), &dlg);
    row->addWidget(path, 1);
    row->addWidget(browse);
    v->addLayout(row);
    QObject::connect(browse, &QPushButton::clicked, &dlg, [&] {
        const QString p = QFileDialog::getOpenFileName(&dlg, QCoreApplication::translate("jp::write::HomeDialogs", "Revised document"), QString(),
                                                       QCoreApplication::translate("jp::write::HomeDialogs", "Documents (*.docx *.docm *.doc *.rtf *.odt *.fodt)") + QStringLiteral(";;") + QCoreApplication::translate("jp::write::HomeDialogs", "All Files (*)"));
        if (!p.isEmpty()) path->setText(p);
    });
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Label the changes with this name:"), &dlg)));
    auto *author = new QLineEdit(options->author, &dlg);
    author->setObjectName(QStringLiteral("compareAuthor"));
    v->addWidget(author);
    auto *byCharacter = new QCheckBox(QCoreApplication::translate("jp::write::HomeDialogs", "Compare letter by letter, not word by word"), &dlg);
    auto *ignoreCase = new QCheckBox(QCoreApplication::translate("jp::write::HomeDialogs", "Ignore differences in capital letters"), &dlg);
    auto *ignoreSpace = new QCheckBox(QCoreApplication::translate("jp::write::HomeDialogs", "Ignore differences in white space"), &dlg);
    byCharacter->setObjectName(QStringLiteral("compareByCharacter"));
    ignoreCase->setObjectName(QStringLiteral("compareIgnoreCase"));
    ignoreSpace->setObjectName(QStringLiteral("compareIgnoreSpace"));
    byCharacter->setChecked(options->byCharacter);
    ignoreCase->setChecked(options->ignoreCase);
    ignoreSpace->setChecked(options->ignoreWhiteSpace);
    v->addWidget(byCharacter);
    v->addWidget(ignoreCase);
    v->addWidget(ignoreSpace);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted || path->text().trimmed().isEmpty()) return false;
    *revisedPath = path->text().trimmed();
    options->author = author->text().trimmed();
    options->byCharacter = byCharacter->isChecked();
    options->ignoreCase = ignoreCase->isChecked();
    options->ignoreWhiteSpace = ignoreSpace->isChecked();
    return true;
}

AccessPane::AccessPane(PageView *view, QWidget *parent) : QWidget(parent), m_view(view)
{
    setObjectName(QStringLiteral("accessPane"));
    auto *v = new QVBoxLayout(this);
    m_summary = plainLabel(new QLabel(this));
    m_summary->setObjectName(QStringLiteral("accessSummary"));
    m_summary->setWordWrap(true);
    v->addWidget(m_summary);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("accessList"));
    m_list->setWordWrap(true);
    v->addWidget(m_list, 1);
    auto *again = new QPushButton(QCoreApplication::translate("jp::write::HomeDialogs", "Check Again"), this);
    again->setObjectName(QStringLiteral("accessAgain"));
    v->addWidget(again);
    connect(again, &QPushButton::clicked, this, &AccessPane::check);
    connect(m_list, &QListWidget::itemActivated, this, [this](QListWidgetItem *item) {
        const int at = item->data(Qt::UserRole).toInt();
        if (!m_view || !m_view->document()) return;
        QTextCursor c(m_view->document()->story());
        c.setPosition(std::min(at, m_view->document()->story()->characterCount() - 1));
        m_view->setCursor(c);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    connect(m_list, &QListWidget::itemClicked, m_list, [this](QListWidgetItem *item) { Q_EMIT m_list->itemActivated(item); });
}

int AccessPane::issueCount() const { return m_list->count(); }

void AccessPane::check()
{
    m_list->clear();
    if (!m_view || !m_view->document()) {
        m_summary->setText(QString());
        return;
    }
    const QVector<AccessIssue> issues = checkAccessibility(*m_view->document());
    for (const AccessIssue &i : issues) {
        const QString level = i.level == AccessIssue::Error ? QCoreApplication::translate("jp::write::HomeDialogs", "Error") : i.level == AccessIssue::Warning ? QCoreApplication::translate("jp::write::HomeDialogs", "Warning") : QCoreApplication::translate("jp::write::HomeDialogs", "Tip");
        auto *item = new QListWidgetItem(level + QStringLiteral(": ") + i.message, m_list);
        item->setData(Qt::UserRole, i.position);
    }
    m_summary->setText(issues.isEmpty() ? QCoreApplication::translate("jp::write::HomeDialogs", "No accessibility problems were found.")
                                        : QCoreApplication::translate("jp::write::HomeDialogs", "%n problem(s) found. Click one to go to it.", nullptr, int(issues.size())));
}

void formattingDialog(QWidget *parent, const QVector<QPair<QString, QString>> &facts)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("formattingDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Reveal Formatting"));
    auto *v = new QVBoxLayout(&dlg);
    auto *grid = new QGridLayout;
    int row = 0;
    for (const auto &f : facts) {
        auto *name = plainLabel(new QLabel(f.first + QLatin1Char(':'), &dlg));
        QFont bold = name->font();
        bold.setBold(true);
        name->setFont(bold);
        auto *value = plainLabel(new QLabel(f.second, &dlg));
        value->setTextInteractionFlags(Qt::TextSelectableByMouse);
        grid->addWidget(name, row, 0, Qt::AlignTop);
        grid->addWidget(value, row, 1);
        ++row;
    }
    v->addLayout(grid);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::accept);
    v->addWidget(bb);
    dlg.exec();
}

} // namespace jp::write
