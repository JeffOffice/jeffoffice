// The dialogs of fields and references: Bookmark, Cross-reference, Field, and Table of Contents (the work on the document is core/refops.h, core/fieldops.h, and core/tocops.h).

#include "app/writedialogs.h"

#include "app/pageview.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/refops.h"
#include "core/tocops.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp::write {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::write::FieldDialogs", text); }

} // namespace

bool bookmarkDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("bookmarkDialog"));
    dlg.setWindowTitle(tr("Bookmark"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(new QLabel(tr("Bookmark name:"), &dlg));
    auto *name = new QLineEdit(&dlg);
    name->setObjectName(QStringLiteral("bookmarkName"));
    v->addWidget(name);
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("bookmarkList"));
    list->setAccessibleName(tr("Bookmarks"));
    v->addWidget(list);
    auto *sortRow = new QHBoxLayout;
    sortRow->addWidget(new QLabel(tr("Sort by:"), &dlg));
    auto *byName = new QRadioButton(tr("Name"), &dlg);
    auto *byPlace = new QRadioButton(tr("Location"), &dlg);
    byName->setChecked(true);
    byName->setObjectName(QStringLiteral("bookmarkSortName"));
    byPlace->setObjectName(QStringLiteral("bookmarkSortLocation"));
    sortRow->addWidget(byName);
    sortRow->addWidget(byPlace);
    sortRow->addStretch();
    v->addLayout(sortRow);
    auto *hidden = new QCheckBox(tr("Hidden bookmarks"), &dlg);
    hidden->setObjectName(QStringLiteral("bookmarkHidden"));
    v->addWidget(hidden);
    auto *problem = new QLabel(&dlg);
    problem->setObjectName(QStringLiteral("bookmarkProblem"));
    problem->setStyleSheet(QStringLiteral("color: #b00020"));
    v->addWidget(problem);
    auto *buttons = new QDialogButtonBox(&dlg);
    auto *add = buttons->addButton(tr("Add"), QDialogButtonBox::ActionRole);
    auto *del = buttons->addButton(tr("Delete"), QDialogButtonBox::ActionRole);
    auto *go = buttons->addButton(tr("Go To"), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close);
    add->setObjectName(QStringLiteral("bookmarkAdd"));
    del->setObjectName(QStringLiteral("bookmarkDelete"));
    go->setObjectName(QStringLiteral("bookmarkGoTo"));
    v->addWidget(buttons);
    bool changed = false;
    auto fill = [&] {
        list->clear();
        for (const BookmarkItem &b : bookmarkList(*doc, hidden->isChecked(), byPlace->isChecked())) list->addItem(b.name);
    };
    QObject::connect(byName, &QRadioButton::toggled, &dlg, fill);
    QObject::connect(hidden, &QCheckBox::toggled, &dlg, fill);
    QObject::connect(list, &QListWidget::currentTextChanged, &dlg, [&](const QString &text) {
        if (!text.isEmpty()) name->setText(text);
    });
    QObject::connect(add, &QPushButton::clicked, &dlg, [&] {
        QString why;
        if (addBookmark(*doc, view->cursor(), name->text().trimmed(), &why)) {
            problem->clear();
            changed = true;
            fill();
        } else {
            problem->setText(why);
        }
    });
    QObject::connect(del, &QPushButton::clicked, &dlg, [&] {
        if (deleteBookmark(*doc, name->text().trimmed())) {
            problem->clear();
            changed = true;
            fill();
        } else {
            problem->setText(tr("There is no such bookmark."));
        }
    });
    auto goTo = [&] {
        const int at = bookmarkStart(*doc, name->text().trimmed());
        if (at < 0) {
            problem->setText(tr("There is no such bookmark."));
            return;
        }
        QTextCursor c = view->cursor();
        c.setPosition(at);
        view->showCursor(c);
        changed = true;
        dlg.accept();
    };
    QObject::connect(go, &QPushButton::clicked, &dlg, goTo);
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg, goTo);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    fill();
    name->setFocus();
    dlg.exec();
    return changed;
}

bool crossReferenceDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("crossReferenceDialog"));
    dlg.setWindowTitle(tr("Cross-reference"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    auto *type = new QComboBox(&dlg);
    type->setObjectName(QStringLiteral("refType"));
    struct Type {
        QString text;
        RefKind kind;
        QString label;
    };
    QVector<Type> types = {{tr("Heading"), RefKind::Heading, QString()}, {tr("Bookmark"), RefKind::Bookmark, QString()}, {tr("Footnote"), RefKind::Footnote, QString()},
                           {tr("Endnote"), RefKind::Endnote, QString()}, {tr("Numbered item"), RefKind::NumberedItem, QString()}};
    QStringList labels = captionLabels(*doc);
    for (const char *l : {"Figure", "Table"})
        if (!labels.contains(QString::fromLatin1(l), Qt::CaseInsensitive)) labels << QString::fromLatin1(l);
    for (const QString &l : std::as_const(labels)) types.push_back({l, RefKind::Caption, l});
    for (const Type &t : std::as_const(types)) type->addItem(t.text);
    form->addRow(tr("Reference type:"), type);
    auto *what = new QComboBox(&dlg);
    what->setObjectName(QStringLiteral("refWhat"));
    form->addRow(tr("Insert reference to:"), what);
    v->addLayout(form);
    auto *link = new QCheckBox(tr("Insert as hyperlink"), &dlg);
    link->setObjectName(QStringLiteral("refHyperlink"));
    link->setChecked(true);
    v->addWidget(link);
    v->addWidget(new QLabel(tr("For which item:"), &dlg));
    auto *items = new QListWidget(&dlg);
    items->setObjectName(QStringLiteral("refItems"));
    items->setAccessibleName(tr("Items"));
    v->addWidget(items);
    auto *problem = new QLabel(&dlg);
    problem->setStyleSheet(QStringLiteral("color: #b00020"));
    v->addWidget(problem);
    auto *buttons = new QDialogButtonBox(&dlg);
    auto *insert = buttons->addButton(tr("Insert"), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close);
    v->addWidget(buttons);
    QVector<RefTarget> targets;
    bool inserted = false;
    auto currentType = [&]() -> const Type & { return types[std::clamp(type->currentIndex(), 0, int(types.size()) - 1)]; };
    auto whatList = [&]() -> QVector<std::pair<QString, RefWhat>> {
        switch (currentType().kind) {
        case RefKind::Heading: return {{tr("Heading text"), RefWhat::Text}, {tr("Page number"), RefWhat::Page}, {tr("Heading number"), RefWhat::Number}, {tr("Heading number (no context)"), RefWhat::NumberNoContext},
                                       {tr("Heading number (full context)"), RefWhat::NumberFullContext}, {tr("Above/below"), RefWhat::AboveBelow}};
        case RefKind::Bookmark: return {{tr("Bookmark text"), RefWhat::Text}, {tr("Page number"), RefWhat::Page}, {tr("Paragraph number"), RefWhat::Number}, {tr("Paragraph number (no context)"), RefWhat::NumberNoContext},
                                        {tr("Paragraph number (full context)"), RefWhat::NumberFullContext}, {tr("Above/below"), RefWhat::AboveBelow}};
        case RefKind::Footnote:
        case RefKind::Endnote: return {{tr("Note number"), RefWhat::FootnoteNumber}, {tr("Page number"), RefWhat::Page}, {tr("Above/below"), RefWhat::AboveBelow}, {tr("Note number (formatted)"), RefWhat::FootnoteNumberFormatted}};
        case RefKind::NumberedItem: return {{tr("Paragraph number"), RefWhat::Number}, {tr("Paragraph number (no context)"), RefWhat::NumberNoContext}, {tr("Paragraph number (full context)"), RefWhat::NumberFullContext},
                                            {tr("Paragraph text"), RefWhat::Text}, {tr("Page number"), RefWhat::Page}, {tr("Above/below"), RefWhat::AboveBelow}};
        case RefKind::Caption: return {{tr("Entire caption"), RefWhat::Text}, {tr("Only label and number"), RefWhat::LabelAndNumber}, {tr("Only caption text"), RefWhat::CaptionText}, {tr("Page number"), RefWhat::Page},
                                       {tr("Above/below"), RefWhat::AboveBelow}};
        }
        return {};
    };
    auto refill = [&] {
        what->clear();
        for (const auto &w : whatList()) what->addItem(w.first);
        items->clear();
        targets = crossReferenceTargets(*doc, currentType().kind, currentType().label);
        for (const RefTarget &t : std::as_const(targets)) items->addItem(t.label);
        if (items->count() > 0) items->setCurrentRow(0);
    };
    QObject::connect(type, &QComboBox::currentIndexChanged, &dlg, refill);
    QObject::connect(insert, &QPushButton::clicked, &dlg, [&] {
        const int row = items->currentRow();
        if (row < 0 || row >= targets.size()) {
            problem->setText(tr("Choose an item to refer to."));
            return;
        }
        const auto list = whatList();
        QString why;
        QTextCursor c = view->cursor();
        if (insertCrossReference(*doc, c, targets[row], currentType().kind, list.value(what->currentIndex()).second, link->isChecked(), &why)) {
            view->showCursor(c);
            inserted = true;
            problem->clear();
            refill();
        } else {
            problem->setText(why);
        }
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    refill();
    dlg.exec();
    return inserted;
}

bool fieldDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("fieldDialog"));
    dlg.setWindowTitle(tr("Field"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(new QLabel(tr("Field names:"), &dlg));
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("fieldNames"));
    list->setAccessibleName(tr("Field names"));
    struct Entry {
        QString name, code;
    };
    const QVector<Entry> entries = {{QStringLiteral("Author"), QStringLiteral("AUTHOR")},
                                    {QStringLiteral("CreateDate"), QStringLiteral("CREATEDATE \\@ \"M/d/yyyy h:mm:ss am/pm\"")},
                                    {QStringLiteral("Date"), QStringLiteral("DATE \\@ \"MMMM d, yyyy\"")},
                                    {QStringLiteral("DocProperty"), QStringLiteral("DOCPROPERTY \"Title\"")},
                                    {QStringLiteral("FileName"), QStringLiteral("FILENAME")},
                                    {QStringLiteral("FileSize"), QStringLiteral("FILESIZE")},
                                    {QStringLiteral("If"), QStringLiteral("IF 1 = 1 \"yes\" \"no\"")},
                                    {QStringLiteral("NoteRef"), QStringLiteral("NOTEREF  \\h")},
                                    {QStringLiteral("NumChars"), QStringLiteral("NUMCHARS")},
                                    {QStringLiteral("NumPages"), QStringLiteral("NUMPAGES")},
                                    {QStringLiteral("NumWords"), QStringLiteral("NUMWORDS")},
                                    {QStringLiteral("Page"), QStringLiteral("PAGE")},
                                    {QStringLiteral("PageRef"), QStringLiteral("PAGEREF  \\h")},
                                    {QStringLiteral("Quote"), QStringLiteral("QUOTE \"text\"")},
                                    {QStringLiteral("Ref"), QStringLiteral("REF  \\h")},
                                    {QStringLiteral("SaveDate"), QStringLiteral("SAVEDATE")},
                                    {QStringLiteral("Section"), QStringLiteral("SECTION")},
                                    {QStringLiteral("SectionPages"), QStringLiteral("SECTIONPAGES")},
                                    {QStringLiteral("Seq"), QStringLiteral("SEQ Figure \\* ARABIC")},
                                    {QStringLiteral("StyleRef"), QStringLiteral("STYLEREF \"Heading 1\"")},
                                    {QStringLiteral("Subject"), QStringLiteral("SUBJECT")},
                                    {QStringLiteral("Time"), QStringLiteral("TIME \\@ \"h:mm am/pm\"")},
                                    {QStringLiteral("Title"), QStringLiteral("TITLE")},
                                    {QStringLiteral("Formula"), QStringLiteral("= 2 + 3")}};
    for (const Entry &e : entries) list->addItem(e.name);
    v->addWidget(list);
    v->addWidget(new QLabel(tr("Field codes:"), &dlg));
    auto *code = new QLineEdit(&dlg);
    code->setObjectName(QStringLiteral("fieldCode"));
    v->addWidget(code);
    auto *preview = new QLabel(&dlg);
    preview->setObjectName(QStringLiteral("fieldPreview"));
    v->addWidget(preview);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(list, &QListWidget::currentRowChanged, &dlg, [&](int row) {
        if (row >= 0 && row < entries.size()) code->setText(entries[row].code);
    });
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg, [&] { buttons->button(QDialogButtonBox::Ok)->click(); });
    QObject::connect(code, &QLineEdit::textChanged, &dlg, [&](const QString &t) {
        buttons->button(QDialogButtonBox::Ok)->setEnabled(!t.trimmed().isEmpty());
        const fields::Code parsed = fields::parseCode(t);
        preview->setText(fields::isKnown(parsed.name) ? QString() : tr("This field is kept as it is and not worked out."));
    });
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    list->setCurrentRow(0);
    if (dlg.exec() != QDialog::Accepted) return false;
    QTextCursor c = view->cursor();
    if (!insertField(*doc, c, code->text())) return false;
    view->showCursor(c);
    return true;
}

bool tableOfContentsDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("tocDialog"));
    dlg.setWindowTitle(tr("Table of Contents"));
    auto *v = new QVBoxLayout(&dlg);
    auto *pages = new QCheckBox(tr("Show page numbers"), &dlg);
    pages->setObjectName(QStringLiteral("tocPages"));
    pages->setChecked(true);
    auto *right = new QCheckBox(tr("Right align page numbers"), &dlg);
    right->setObjectName(QStringLiteral("tocRight"));
    right->setChecked(true);
    auto *links = new QCheckBox(tr("Use hyperlinks"), &dlg);
    links->setObjectName(QStringLiteral("tocLinks"));
    links->setChecked(true);
    v->addWidget(pages);
    v->addWidget(right);
    v->addWidget(links);
    auto *form = new QFormLayout;
    auto *leader = new QComboBox(&dlg);
    leader->setObjectName(QStringLiteral("tocLeader"));
    leader->addItem(tr("(none)"), QStringLiteral("none"));
    leader->addItem(QStringLiteral("......."), QStringLiteral("dot"));
    leader->addItem(QStringLiteral("-------"), QStringLiteral("hyphen"));
    leader->addItem(QStringLiteral("_______"), QStringLiteral("underscore"));
    leader->setCurrentIndex(1);
    auto *levels = new QSpinBox(&dlg);
    levels->setObjectName(QStringLiteral("tocLevels"));
    levels->setRange(1, 9);
    levels->setValue(3);
    form->addRow(tr("Tab leader:"), leader);
    form->addRow(tr("Show levels:"), levels);
    v->addLayout(form);
    QObject::connect(pages, &QCheckBox::toggled, &dlg, [&](bool on) {
        right->setEnabled(on);
        leader->setEnabled(on && right->isChecked());
    });
    QObject::connect(right, &QCheckBox::toggled, &dlg, [&](bool on) { leader->setEnabled(on && pages->isChecked()); });
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;
    TocOptions o;
    o.levels = levels->value();
    o.pageNumbers = pages->isChecked();
    o.rightAlign = right->isChecked();
    o.leader = leader->currentData().toString();
    o.hyperlinks = links->isChecked();
    QTextCursor c = view->cursor();
    const bool ok = insertTableOfContents(*doc, c, o);
    if (ok) view->showCursor(c);
    return ok;
}

int updateTableDialog(QWidget *parent)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("updateTableDialog"));
    dlg.setWindowTitle(tr("Update Table of Contents"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(new QLabel(tr("What do you want to update?"), &dlg));
    auto *only = new QRadioButton(tr("Update page numbers only"), &dlg);
    auto *all = new QRadioButton(tr("Update entire table"), &dlg);
    only->setObjectName(QStringLiteral("updatePagesOnly"));
    all->setObjectName(QStringLiteral("updateEntire"));
    only->setChecked(true);
    v->addWidget(only);
    v->addWidget(all);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return 0;
    return all->isChecked() ? 2 : 1;
}

} // namespace jp::write
