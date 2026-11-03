// The dialogs of the References tab beyond those of fields: Mark Entry for a table of contents, Insert Caption, Insert Table of Figures, Mark Entry and Insert Index, Mark Citation and
// Insert Table of Authorities, Insert Citation, and Manage Sources (the work on the document is core/captionops.h, core/indexops.h, core/citeops.h, and core/tocops.h).

#include "app/writedialogs.h"

#include "app/pageview.h"
#include "core/captionops.h"
#include "core/citeops.h"
#include "core/editops.h"
#include "core/indexops.h"
#include "core/refops.h"
#include "core/tocops.h"
#include "core/writedoc.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp::write {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::write::RefsDialogs", text); }

QComboBox *leaderBox(QDialog *dlg, const char *name)
{
    auto *leader = new QComboBox(dlg);
    leader->setObjectName(QString::fromLatin1(name));
    leader->addItem(tr("(none)"), QStringLiteral("none"));
    leader->addItem(QStringLiteral("......."), QStringLiteral("dot"));
    leader->addItem(QStringLiteral("-------"), QStringLiteral("hyphen"));
    leader->addItem(QStringLiteral("_______"), QStringLiteral("underscore"));
    leader->setCurrentIndex(1);
    return leader;
}

QDialogButtonBox *okCancel(QDialog *dlg)
{
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg);
    QObject::connect(buttons, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    return buttons;
}

} // namespace

bool tocEntryDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("tocEntryDialog"));
    dlg.setWindowTitle(tr("Mark Table of Contents Entry"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    auto *entry = new QLineEdit(view->cursor().selectedText().simplified(), &dlg);
    entry->setObjectName(QStringLiteral("tocEntryText"));
    auto *level = new QSpinBox(&dlg);
    level->setObjectName(QStringLiteral("tocEntryLevel"));
    level->setRange(1, 9);
    auto *type = new QLineEdit(&dlg);
    type->setObjectName(QStringLiteral("tocEntryType"));
    type->setMaxLength(1);
    form->addRow(tr("Entry:"), entry);
    form->addRow(tr("Level:"), level);
    form->addRow(tr("Table identifier (a letter, or none):"), type);
    v->addLayout(form);
    auto *noPage = new QCheckBox(tr("Leave out the page number"), &dlg);
    noPage->setObjectName(QStringLiteral("tocEntryNoPage"));
    v->addWidget(noPage);
    v->addWidget(okCancel(&dlg));
    if (dlg.exec() != QDialog::Accepted) return false;
    QTextCursor c = view->cursor();
    const bool ok = markTocEntry(*doc, c, entry->text(), level->value(), type->text(), noPage->isChecked());
    if (ok) view->showCursor(c);
    return ok;
}

bool captionDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("captionDialog"));
    dlg.setWindowTitle(tr("Insert Caption"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    auto *label = new QComboBox(&dlg);
    label->setObjectName(QStringLiteral("captionLabel"));
    label->setEditable(true);
    label->addItems(captionLabelChoices(*doc));
    auto *text = new QLineEdit(&dlg);
    text->setObjectName(QStringLiteral("captionText"));
    auto *position = new QComboBox(&dlg);
    position->setObjectName(QStringLiteral("captionPosition"));
    position->addItem(tr("Below the selection"), false);
    position->addItem(tr("Above the selection"), true);
    form->addRow(tr("Label:"), label);
    form->addRow(tr("Text after the number (such as \": Sales\"):"), text);
    form->addRow(tr("Position:"), position);
    v->addLayout(form);
    auto *exclude = new QCheckBox(tr("Do not show the label in the caption"), &dlg);
    exclude->setObjectName(QStringLiteral("captionExclude"));
    v->addWidget(exclude);
    auto *chapter = new QCheckBox(tr("Include the chapter number"), &dlg);
    chapter->setObjectName(QStringLiteral("captionChapter"));
    v->addWidget(chapter);
    auto *chapterForm = new QFormLayout;
    auto *level = new QSpinBox(&dlg);
    level->setObjectName(QStringLiteral("captionChapterLevel"));
    level->setRange(1, 9);
    level->setEnabled(false);
    auto *separator = new QLineEdit(QStringLiteral("-"), &dlg);
    separator->setObjectName(QStringLiteral("captionSeparator"));
    separator->setMaxLength(3);
    separator->setEnabled(false);
    chapterForm->addRow(tr("Chapter starts with heading level:"), level);
    chapterForm->addRow(tr("Separator:"), separator);
    v->addLayout(chapterForm);
    QObject::connect(chapter, &QCheckBox::toggled, level, &QWidget::setEnabled);
    QObject::connect(chapter, &QCheckBox::toggled, separator, &QWidget::setEnabled);
    auto *problem = new QLabel(&dlg);
    problem->setStyleSheet(QStringLiteral("color: #b00020"));
    v->addWidget(problem);
    auto *buttons = okCancel(&dlg);
    v->addWidget(buttons);
    bool done = false;
    QObject::disconnect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&] {
        CaptionOptions o;
        o.label = label->currentText();
        o.text = text->text();
        o.above = position->currentData().toBool();
        o.excludeLabel = exclude->isChecked();
        o.chapterLevel = chapter->isChecked() ? level->value() : 0;
        o.separator = separator->text();
        QString why;
        QTextCursor c = view->cursor();
        if (insertCaption(*doc, c, o, &why)) {
            view->showCursor(c);
            done = true;
            dlg.accept();
        } else {
            problem->setText(why);
        }
    });
    dlg.exec();
    return done;
}

bool tableOfFiguresDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("tableOfFiguresDialog"));
    dlg.setWindowTitle(tr("Table of Figures"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    auto *label = new QComboBox(&dlg);
    label->setObjectName(QStringLiteral("figuresLabel"));
    label->addItems(captionLabelChoices(*doc));
    auto *leader = leaderBox(&dlg, "figuresLeader");
    form->addRow(tr("Caption label:"), label);
    form->addRow(tr("Tab leader:"), leader);
    v->addLayout(form);
    auto *includeLabel = new QCheckBox(tr("Include label and number"), &dlg);
    includeLabel->setObjectName(QStringLiteral("figuresIncludeLabel"));
    includeLabel->setChecked(true);
    auto *pages = new QCheckBox(tr("Show page numbers"), &dlg);
    pages->setObjectName(QStringLiteral("figuresPages"));
    pages->setChecked(true);
    auto *right = new QCheckBox(tr("Right align page numbers"), &dlg);
    right->setObjectName(QStringLiteral("figuresRight"));
    right->setChecked(true);
    auto *links = new QCheckBox(tr("Use hyperlinks"), &dlg);
    links->setObjectName(QStringLiteral("figuresLinks"));
    links->setChecked(true);
    for (QWidget *w : {static_cast<QWidget *>(includeLabel), static_cast<QWidget *>(pages), static_cast<QWidget *>(right), static_cast<QWidget *>(links)}) v->addWidget(w);
    QObject::connect(pages, &QCheckBox::toggled, &dlg, [&](bool on) {
        right->setEnabled(on);
        leader->setEnabled(on && right->isChecked());
    });
    QObject::connect(right, &QCheckBox::toggled, &dlg, [&](bool on) { leader->setEnabled(on && pages->isChecked()); });
    v->addWidget(okCancel(&dlg));
    if (dlg.exec() != QDialog::Accepted) return false;
    FiguresOptions o;
    o.label = label->currentText();
    o.includeLabel = includeLabel->isChecked();
    o.pageNumbers = pages->isChecked();
    o.rightAlign = right->isChecked();
    o.leader = leader->currentData().toString();
    o.hyperlinks = links->isChecked();
    QTextCursor c = view->cursor();
    const bool ok = insertTableOfFigures(*doc, c, o);
    if (ok) view->showCursor(c);
    return ok;
}

bool markIndexEntryDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("markIndexDialog"));
    dlg.setWindowTitle(tr("Mark Index Entry"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    auto *mainEntry = new QLineEdit(view->cursor().selectedText().simplified(), &dlg);
    mainEntry->setObjectName(QStringLiteral("indexMain"));
    auto *sub = new QLineEdit(&dlg);
    sub->setObjectName(QStringLiteral("indexSub"));
    auto *type = new QLineEdit(&dlg);
    type->setObjectName(QStringLiteral("indexType"));
    type->setMaxLength(1);
    form->addRow(tr("Main entry:"), mainEntry);
    form->addRow(tr("Subentry:"), sub);
    form->addRow(tr("Index identifier (a letter, or none):"), type);
    v->addLayout(form);
    auto *current = new QRadioButton(tr("Current page"), &dlg);
    current->setObjectName(QStringLiteral("indexCurrent"));
    current->setChecked(true);
    auto *range = new QRadioButton(tr("Page range, from the bookmark:"), &dlg);
    range->setObjectName(QStringLiteral("indexRange"));
    auto *bookmark = new QComboBox(&dlg);
    bookmark->setObjectName(QStringLiteral("indexBookmark"));
    for (const BookmarkItem &b : bookmarkList(*doc)) bookmark->addItem(b.name);
    bookmark->setEnabled(false);
    auto *cross = new QRadioButton(tr("Cross-reference:"), &dlg);
    cross->setObjectName(QStringLiteral("indexCross"));
    auto *crossText = new QLineEdit(tr("See "), &dlg);
    crossText->setObjectName(QStringLiteral("indexCrossText"));
    crossText->setEnabled(false);
    QObject::connect(range, &QRadioButton::toggled, bookmark, &QWidget::setEnabled);
    QObject::connect(cross, &QRadioButton::toggled, crossText, &QWidget::setEnabled);
    v->addWidget(current);
    v->addWidget(range);
    v->addWidget(bookmark);
    v->addWidget(cross);
    v->addWidget(crossText);
    auto *bold = new QCheckBox(tr("Bold page number"), &dlg);
    bold->setObjectName(QStringLiteral("indexBold"));
    auto *italic = new QCheckBox(tr("Italic page number"), &dlg);
    italic->setObjectName(QStringLiteral("indexItalic"));
    v->addWidget(bold);
    v->addWidget(italic);
    auto *buttons = new QDialogButtonBox(&dlg);
    auto *mark = buttons->addButton(tr("Mark"), QDialogButtonBox::AcceptRole);
    auto *markAll = buttons->addButton(tr("Mark All"), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Cancel);
    v->addWidget(buttons);
    int choice = 0;
    QObject::connect(mark, &QPushButton::clicked, &dlg, [&] { choice = 1; dlg.accept(); });
    QObject::connect(markAll, &QPushButton::clicked, &dlg, [&] { choice = 2; dlg.accept(); });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted || choice == 0) return false;
    IndexMark m;
    m.main = mainEntry->text();
    m.sub = sub->text();
    m.type = type->text();
    m.bold = bold->isChecked();
    m.italic = italic->isChecked();
    if (range->isChecked()) {
        m.page = IndexMark::Range;
        m.bookmark = bookmark->currentText();
    } else if (cross->isChecked()) {
        m.page = IndexMark::CrossReference;
        m.crossReference = crossText->text();
    }
    bool ok;
    if (choice == 2) {
        ok = markAllIndexEntries(*doc, m) > 0;
    } else {
        QTextCursor c = view->cursor();
        ok = markIndexEntry(*doc, c, m);
        if (ok) view->showCursor(c);
    }
    return ok;
}

bool indexDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("indexDialog"));
    dlg.setWindowTitle(tr("Index"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    auto *columns = new QSpinBox(&dlg);
    columns->setObjectName(QStringLiteral("indexColumns"));
    columns->setRange(1, 4);
    columns->setValue(2);
    auto *leader = leaderBox(&dlg, "indexLeader");
    auto *type = new QLineEdit(&dlg);
    type->setObjectName(QStringLiteral("indexIdentifier"));
    type->setMaxLength(1);
    form->addRow(tr("Columns:"), columns);
    form->addRow(tr("Tab leader:"), leader);
    form->addRow(tr("Index identifier (a letter, or none):"), type);
    v->addLayout(form);
    auto *right = new QCheckBox(tr("Right align page numbers"), &dlg);
    right->setObjectName(QStringLiteral("indexRight"));
    right->setChecked(true);
    auto *headings = new QCheckBox(tr("Put a letter above each group"), &dlg);
    headings->setObjectName(QStringLiteral("indexHeadings"));
    headings->setChecked(true);
    auto *indented = new QRadioButton(tr("Indented: each subentry on its own line"), &dlg);
    indented->setObjectName(QStringLiteral("indexIndented"));
    indented->setChecked(true);
    auto *runIn = new QRadioButton(tr("Run-in: subentries follow the entry on one line"), &dlg);
    runIn->setObjectName(QStringLiteral("indexRunIn"));
    v->addWidget(right);
    v->addWidget(headings);
    v->addWidget(indented);
    v->addWidget(runIn);
    QObject::connect(runIn, &QRadioButton::toggled, right, [&](bool on) { if (on) right->setChecked(false); });
    QObject::connect(right, &QCheckBox::toggled, leader, &QWidget::setEnabled);
    v->addWidget(okCancel(&dlg));
    if (dlg.exec() != QDialog::Accepted) return false;
    IndexOptions o;
    o.columns = columns->value();
    o.rightAlign = right->isChecked();
    o.leader = leader->currentData().toString();
    o.runIn = runIn->isChecked();
    o.headings = headings->isChecked();
    o.type = type->text();
    QTextCursor c = view->cursor();
    const bool ok = insertIndex(*doc, c, o);
    if (ok) view->showCursor(c);
    return ok;
}

bool markCitationDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("markCitationDialog"));
    dlg.setWindowTitle(tr("Mark Citation"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    const QString selected = view->cursor().selectedText().simplified();
    auto *longCite = new QLineEdit(selected, &dlg);
    longCite->setObjectName(QStringLiteral("citationLong"));
    auto *shortCite = new QLineEdit(&dlg);
    shortCite->setObjectName(QStringLiteral("citationShort"));
    auto *category = new QComboBox(&dlg);
    category->setObjectName(QStringLiteral("citationCategory"));
    for (int i = 1; i <= 16; ++i) category->addItem(authorityCategoryName(i), i);
    form->addRow(tr("Long citation:"), longCite);
    form->addRow(tr("Short citation:"), shortCite);
    form->addRow(tr("Category:"), category);
    v->addLayout(form);
    auto *bold = new QCheckBox(tr("Bold page number"), &dlg);
    bold->setObjectName(QStringLiteral("citationBold"));
    auto *italic = new QCheckBox(tr("Italic page number"), &dlg);
    italic->setObjectName(QStringLiteral("citationItalic"));
    v->addWidget(bold);
    v->addWidget(italic);
    v->addWidget(okCancel(&dlg));
    if (dlg.exec() != QDialog::Accepted) return false;
    AuthorityMark m;
    m.longCitation = longCite->text();
    m.shortCitation = shortCite->text();
    m.category = category->currentData().toInt();
    m.bold = bold->isChecked();
    m.italic = italic->isChecked();
    QTextCursor c = view->cursor();
    const bool ok = markAuthority(*doc, c, m);
    if (ok) view->showCursor(c);
    return ok;
}

bool tableOfAuthoritiesDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("authoritiesDialog"));
    dlg.setWindowTitle(tr("Table of Authorities"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    auto *category = new QComboBox(&dlg);
    category->setObjectName(QStringLiteral("authoritiesCategory"));
    category->addItem(tr("All"), 0);
    for (int i = 1; i <= 16; ++i) category->addItem(authorityCategoryName(i), i);
    auto *leader = leaderBox(&dlg, "authoritiesLeader");
    form->addRow(tr("Category:"), category);
    form->addRow(tr("Tab leader:"), leader);
    v->addLayout(form);
    auto *passim = new QCheckBox(tr("Use passim for five or more pages"), &dlg);
    passim->setObjectName(QStringLiteral("authoritiesPassim"));
    passim->setChecked(true);
    auto *headings = new QCheckBox(tr("Put the category's name above its citations"), &dlg);
    headings->setObjectName(QStringLiteral("authoritiesHeadings"));
    headings->setChecked(true);
    v->addWidget(passim);
    v->addWidget(headings);
    v->addWidget(okCancel(&dlg));
    if (dlg.exec() != QDialog::Accepted) return false;
    AuthoritiesOptions o;
    o.category = category->currentData().toInt();
    o.passim = passim->isChecked();
    o.headings = headings->isChecked();
    o.leader = leader->currentData().toString();
    QTextCursor c = view->cursor();
    const bool ok = insertTableOfAuthorities(*doc, c, o);
    if (ok) view->showCursor(c);
    return ok;
}

// ---- sources

bool sourceDialog(QWidget *parent, Source *source, const QVector<Source> &others)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("sourceDialog"));
    dlg.setWindowTitle(tr("Source"));
    auto *v = new QVBoxLayout(&dlg);
    auto *top = new QFormLayout;
    auto *type = new QComboBox(&dlg);
    type->setObjectName(QStringLiteral("sourceType"));
    for (const QString &t : sourceTypes()) type->addItem(sourceTypeLabel(t), t);
    type->setCurrentIndex(std::max(0, int(sourceTypes().indexOf(source->type))));
    auto *authors = new QPlainTextEdit(&dlg);
    authors->setObjectName(QStringLiteral("sourceAuthors"));
    authors->setMaximumHeight(72);
    QStringList lines;
    for (const Person &p : source->authors) lines << (p.last + QStringLiteral(", ") + p.first + (p.middle.isEmpty() ? QString() : QLatin1Char(' ') + p.middle)).trimmed();
    authors->setPlainText(lines.join(QLatin1Char('\n')));
    auto *corporate = new QLineEdit(source->corporateAuthor, &dlg);
    corporate->setObjectName(QStringLiteral("sourceCorporate"));
    top->addRow(tr("Type of source:"), type);
    top->addRow(tr("Authors (one a line, as \"Last, First Middle\"):"), authors);
    top->addRow(tr("Organization as the author:"), corporate);
    v->addLayout(top);
    auto *fieldsForm = new QFormLayout;
    v->addLayout(fieldsForm);
    auto *tag = new QLineEdit(source->tag, &dlg);
    tag->setObjectName(QStringLiteral("sourceTag"));
    auto *tagForm = new QFormLayout;
    tagForm->addRow(tr("Tag name:"), tag);
    v->addLayout(tagForm);
    QMap<QString, QLineEdit *> edits;
    QMap<QString, QString> keep = source->fields;
    auto rebuild = [&] {
        for (auto it = edits.cbegin(); it != edits.cend(); ++it) keep.insert(it.key(), it.value()->text());
        while (fieldsForm->rowCount() > 0) fieldsForm->removeRow(0);
        edits.clear();
        for (const QString &f : sourceFieldsFor(type->currentData().toString())) {
            auto *e = new QLineEdit(keep.value(f), &dlg);
            e->setObjectName(QStringLiteral("sourceField_") + f);
            edits.insert(f, e);
            fieldsForm->addRow(sourceFieldLabel(f) + QLatin1Char(':'), e);
        }
    };
    QObject::connect(type, &QComboBox::currentIndexChanged, &dlg, rebuild);
    rebuild();
    auto *problem = new QLabel(&dlg);
    problem->setStyleSheet(QStringLiteral("color: #b00020"));
    v->addWidget(problem);
    auto *buttons = okCancel(&dlg);
    v->addWidget(buttons);
    bool done = false;
    QObject::disconnect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&] {
        Source s = *source;
        s.type = type->currentData().toString();
        s.authors.clear();
        for (const QString &line : authors->toPlainText().split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            Person p;
            const int comma = int(line.indexOf(QLatin1Char(',')));
            if (comma < 0) {
                p.last = line.trimmed();
            } else {
                p.last = line.left(comma).trimmed();
                const QStringList rest = line.mid(comma + 1).simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
                p.first = rest.value(0);
                p.middle = rest.mid(1).join(QLatin1Char(' '));
            }
            if (!p.last.isEmpty() && s.authors.size() < 50) s.authors.push_back(p);
        }
        s.corporateAuthor = corporate->text().trimmed();
        for (auto it = edits.cbegin(); it != edits.cend(); ++it) keep.insert(it.key(), it.value()->text().trimmed());
        s.fields = keep;
        for (auto it = s.fields.begin(); it != s.fields.end();)
            it = it.value().isEmpty() ? s.fields.erase(it) : std::next(it);
        s.tag = tag->text().trimmed();
        if (s.tag.isEmpty()) s.tag = newSourceTag(s, others);
        for (const Source &o : others)
            if (o.tag.compare(s.tag, Qt::CaseInsensitive) == 0) {
                problem->setText(tr("Another source has that tag name. Give this one a different tag."));
                return;
            }
        *source = s;
        done = true;
        dlg.accept();
    });
    dlg.exec();
    return done;
}

bool manageSourcesDialog(QWidget *parent, WriteDoc *doc)
{
    if (!doc) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("manageSourcesDialog"));
    dlg.setWindowTitle(tr("Source Manager"));
    dlg.resize(760, 420);
    QVector<Source> master = masterSources(), current = documentSources(*doc);
    auto *v = new QVBoxLayout(&dlg);
    auto *row = new QHBoxLayout;
    auto column = [&](const QString &title, const char *name, QListWidget **list) {
        auto *col = new QVBoxLayout;
        col->addWidget(new QLabel(title, &dlg));
        *list = new QListWidget(&dlg);
        (*list)->setObjectName(QString::fromLatin1(name));
        (*list)->setAccessibleName(title);
        col->addWidget(*list);
        return col;
    };
    QListWidget *masterList = nullptr, *currentList = nullptr;
    row->addLayout(column(tr("Master list (shared by your documents):"), "masterSources", &masterList), 1);
    auto *mid = new QVBoxLayout;
    mid->addStretch();
    auto *copyTo = new QPushButton(tr("Copy >>"), &dlg);
    copyTo->setObjectName(QStringLiteral("sourcesCopyTo"));
    auto *copyFrom = new QPushButton(tr("<< Copy"), &dlg);
    copyFrom->setObjectName(QStringLiteral("sourcesCopyFrom"));
    auto *remove = new QPushButton(tr("Delete"), &dlg);
    remove->setObjectName(QStringLiteral("sourcesDelete"));
    auto *edit = new QPushButton(tr("Edit..."), &dlg);
    edit->setObjectName(QStringLiteral("sourcesEdit"));
    auto *add = new QPushButton(tr("New..."), &dlg);
    add->setObjectName(QStringLiteral("sourcesNew"));
    for (QPushButton *b : {copyTo, copyFrom, remove, edit, add}) mid->addWidget(b);
    mid->addStretch();
    row->addLayout(mid);
    row->addLayout(column(tr("Current list (in this document):"), "currentSources", &currentList), 1);
    v->addLayout(row);
    auto *preview = new QLabel(&dlg);
    preview->setWordWrap(true);
    v->addWidget(preview);
    auto *close = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    QObject::connect(close, &QDialogButtonBox::rejected, &dlg, &QDialog::accept);
    QObject::connect(close, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    v->addWidget(close);
    auto fill = [&] {
        masterList->clear();
        for (const Source &s : std::as_const(master)) masterList->addItem(sourceSummary(s));
        currentList->clear();
        for (const Source &s : std::as_const(current)) currentList->addItem(sourceSummary(s));
    };
    auto selected = [&](QListWidget *list, QVector<Source> **vec) { *vec = list == masterList ? &master : &current; return list->currentRow(); };
    auto copy = [&](QListWidget *from, QVector<Source> *to, const QVector<Source> *src) {
        const int r = from->currentRow();
        if (r < 0 || r >= src->size() || to->size() >= kMostSources) return;
        Source s = (*src)[r];
        for (const Source &o : std::as_const(*to))
            if (o.tag.compare(s.tag, Qt::CaseInsensitive) == 0) {
                if (o == s) return;
                s.tag = newSourceTag(s, *to);
                break;
            }
        to->push_back(s);
        fill();
    };
    QObject::connect(copyTo, &QPushButton::clicked, &dlg, [&] { copy(currentList, &master, &current); });
    QObject::connect(copyFrom, &QPushButton::clicked, &dlg, [&] { copy(masterList, &current, &master); });
    auto active = [&]() { return masterList->hasFocus() || (masterList->currentRow() >= 0 && !currentList->hasFocus()) ? masterList : currentList; };
    QObject::connect(remove, &QPushButton::clicked, &dlg, [&] {
        QVector<Source> *vec;
        QListWidget *l = active();
        const int r = selected(l, &vec);
        if (r >= 0 && r < vec->size()) vec->removeAt(r);
        fill();
    });
    QObject::connect(edit, &QPushButton::clicked, &dlg, [&] {
        QVector<Source> *vec;
        QListWidget *l = active();
        const int r = selected(l, &vec);
        if (r < 0 || r >= vec->size()) return;
        Source s = (*vec)[r];
        QVector<Source> others = *vec;
        others.removeAt(r);
        if (sourceDialog(&dlg, &s, others)) (*vec)[r] = s;
        fill();
    });
    QObject::connect(add, &QPushButton::clicked, &dlg, [&] {
        Source s;
        if (current.size() >= kMostSources) return;
        if (sourceDialog(&dlg, &s, current)) current.push_back(s);
        fill();
    });
    auto showPreview = [&] {
        QVector<Source> *vec;
        QListWidget *l = active();
        const int r = selected(l, &vec);
        if (r < 0 || r >= vec->size()) {
            preview->clear();
            return;
        }
        QString text;
        for (const GenRun &run : bibliographyEntry((*vec)[r], documentCitationStyle(*doc), 1)) text += run.text;
        preview->setText(tr("Preview (%1): %2").arg(documentCitationStyle(*doc), text));
    };
    QObject::connect(masterList, &QListWidget::currentRowChanged, &dlg, showPreview);
    QObject::connect(currentList, &QListWidget::currentRowChanged, &dlg, showPreview);
    fill();
    const QVector<Source> before = current;
    dlg.exec();
    setMasterSources(master);
    if (current != before) {
        setDocumentSources(*doc, current);
        UpdateOptions u;
        u.skip = [](const QString &name) { return name != QLatin1String("CITATION") && name != QLatin1String("BIBLIOGRAPHY"); };
        updateFields(*doc, u);
        return true;
    }
    return false;
}

bool insertCitationDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("insertCitationDialog"));
    dlg.setWindowTitle(tr("Insert Citation"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(new QLabel(tr("Cite which source:"), &dlg));
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("citeSources"));
    list->setAccessibleName(tr("Sources"));
    v->addWidget(list);
    auto *form = new QFormLayout;
    auto *pages = new QLineEdit(&dlg);
    pages->setObjectName(QStringLiteral("citePages"));
    form->addRow(tr("Pages:"), pages);
    v->addLayout(form);
    auto *noAuthor = new QCheckBox(tr("Leave out the author"), &dlg);
    noAuthor->setObjectName(QStringLiteral("citeNoAuthor"));
    auto *noYear = new QCheckBox(tr("Leave out the year"), &dlg);
    noYear->setObjectName(QStringLiteral("citeNoYear"));
    auto *withTitle = new QCheckBox(tr("Show the title"), &dlg);
    withTitle->setObjectName(QStringLiteral("citeTitle"));
    for (QCheckBox *b : {noAuthor, noYear, withTitle}) v->addWidget(b);
    auto *buttons = new QDialogButtonBox(&dlg);
    auto *insert = buttons->addButton(tr("Insert"), QDialogButtonBox::AcceptRole);
    auto *newSource = buttons->addButton(tr("New Source..."), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Cancel);
    v->addWidget(buttons);
    QVector<Source> sources = documentSources(*doc);
    const QVector<Source> master = masterSources();
    auto fill = [&] {
        list->clear();
        for (const Source &s : std::as_const(sources)) list->addItem(sourceSummary(s));
        if (list->count() > 0 && list->currentRow() < 0) list->setCurrentRow(list->count() - 1);
        insert->setEnabled(list->count() > 0);
    };
    fill();
    bool changed = false;
    QObject::connect(newSource, &QPushButton::clicked, &dlg, [&] {
        Source s;
        if (sources.size() >= kMostSources || !sourceDialog(&dlg, &s, sources)) return;
        sources.push_back(s);
        changed = true;
        QVector<Source> shared = masterSources();
        bool have = false;
        for (const Source &m : std::as_const(shared)) have = have || m.tag.compare(s.tag, Qt::CaseInsensitive) == 0;
        if (!have) {
            shared.push_back(s);
            setMasterSources(shared);
        }
        fill();
        list->setCurrentRow(list->count() - 1);
    });
    Q_UNUSED(master);
    QObject::connect(insert, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    const bool accepted = dlg.exec() == QDialog::Accepted && list->currentRow() >= 0;
    if (changed) setDocumentSources(*doc, sources);
    if (!accepted) return changed;
    CitationOptions o;
    o.pages = pages->text();
    o.suppressAuthor = noAuthor->isChecked();
    o.suppressYear = noYear->isChecked();
    o.suppressTitle = !withTitle->isChecked();
    QTextCursor c = view->cursor();
    const bool ok = insertCitation(*doc, c, sources[list->currentRow()].tag, o);
    if (ok) view->showCursor(c);
    return ok;
}

bool citationStyleDialog(QWidget *parent, WriteDoc *doc)
{
    if (!doc) return false;
    const QStringList styles = citationStyles();
    bool ok = false;
    const QString chosen = QInputDialog::getItem(parent, tr("Citation Style"), tr("Show citations and the bibliography in the style:"), styles, std::max(0, int(styles.indexOf(documentCitationStyle(*doc)))), false, &ok);
    if (!ok || chosen == documentCitationStyle(*doc)) return false;
    setDocumentCitationStyle(*doc, chosen);
    return true;
}

bool notesOptionsDialog(QWidget *parent, WriteDoc *doc, bool *endnotesChanged)
{
    if (!doc) return false;
    if (endnotesChanged) *endnotesChanged = false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("notesOptionsDialog"));
    dlg.setWindowTitle(tr("Footnote and Endnote"));
    auto *v = new QVBoxLayout(&dlg);
    struct Box {
        QComboBox *position = nullptr, *format = nullptr, *restart = nullptr;
        QSpinBox *start = nullptr;
    };
    auto formats = [&](QComboBox *c) {
        c->addItem(tr("1, 2, 3, ..."), QStringLiteral("decimal"));
        c->addItem(tr("a, b, c, ..."), QStringLiteral("lowerLetter"));
        c->addItem(tr("A, B, C, ..."), QStringLiteral("upperLetter"));
        c->addItem(tr("i, ii, iii, ..."), QStringLiteral("lowerRoman"));
        c->addItem(tr("I, II, III, ..."), QStringLiteral("upperRoman"));
        c->addItem(tr("*, \u2020, \u2021, \u00A7, ..."), QStringLiteral("chicago"));
    };
    auto makeBox = [&](const QString &title, bool endnotes) {
        auto *group = new QGroupBox(title, &dlg);
        group->setObjectName(endnotes ? QStringLiteral("endnoteOptions") : QStringLiteral("footnoteOptions"));
        auto *form = new QFormLayout(group);
        Box b;
        if (endnotes) {
            b.position = new QComboBox(group);
            b.position->setObjectName(QStringLiteral("endnotePosition"));
            b.position->addItem(tr("End of section"), QStringLiteral("sectEnd"));
            b.position->addItem(tr("End of document"), QStringLiteral("docEnd"));
            form->addRow(tr("Location:"), b.position);
        }
        b.format = new QComboBox(group);
        b.format->setObjectName(endnotes ? QStringLiteral("endnoteFormat") : QStringLiteral("footnoteFormat"));
        formats(b.format);
        b.start = new QSpinBox(group);
        b.start->setObjectName(endnotes ? QStringLiteral("endnoteStart") : QStringLiteral("footnoteStart"));
        b.start->setRange(1, 32767);
        b.restart = new QComboBox(group);
        b.restart->setObjectName(endnotes ? QStringLiteral("endnoteRestart") : QStringLiteral("footnoteRestart"));
        b.restart->addItem(tr("Continuous"), QStringLiteral("continuous"));
        b.restart->addItem(tr("Restart each section"), QStringLiteral("eachSect"));
        if (!endnotes) b.restart->addItem(tr("Restart each page"), QStringLiteral("eachPage"));
        form->addRow(tr("Number format:"), b.format);
        form->addRow(tr("Start at:"), b.start);
        form->addRow(tr("Numbering:"), b.restart);
        const WriteDoc::NoteSettings now = doc->noteSettings(endnotes);
        b.format->setCurrentIndex(std::max(0, b.format->findData(now.format)));
        b.start->setValue(now.start);
        b.restart->setCurrentIndex(std::max(0, b.restart->findData(now.restart)));
        if (b.position) b.position->setCurrentIndex(std::max(0, b.position->findData(now.position.isEmpty() ? QStringLiteral("docEnd") : now.position)));
        v->addWidget(group);
        return b;
    };
    const Box foot = makeBox(tr("Footnotes"), false), end = makeBox(tr("Endnotes"), true);
    v->addWidget(okCancel(&dlg));
    if (dlg.exec() != QDialog::Accepted) return false;
    WriteDoc::NoteSettings f = doc->noteSettings(false), e = doc->noteSettings(true);
    const WriteDoc::NoteSettings f0 = f, e0 = e;
    f.format = foot.format->currentData().toString();
    f.start = foot.start->value();
    f.restart = foot.restart->currentData().toString();
    e.format = end.format->currentData().toString();
    e.start = end.start->value();
    e.restart = end.restart->currentData().toString();
    // (Where the endnotes are set is left as the file has it when the choice is the one that shows.)
    if (!(e0.position.isEmpty() && end.position->currentData().toString() == QLatin1String("docEnd"))) e.position = end.position->currentData().toString();
    const bool footChanged = !(f == f0), endChanged = !(e == e0);
    if (footChanged) doc->setNoteSettings(false, f);
    if (endChanged) doc->setNoteSettings(true, e);
    if (endnotesChanged) *endnotesChanged = endChanged;
    return footChanged || endChanged;
}

bool convertNotesDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    // The note the caret is in, if it is in one: a footnote's story, or the words of an endnote.
    int inFootnote = 0, inEndnote = 0;
    if (view->cursor().document() != doc->story()) {
        doc->footnoteIdOf(view->cursor().document(), &inFootnote);
    } else {
        doc->endnoteIdOf(view->cursor().block(), &inEndnote);
    }
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("convertNotesDialog"));
    dlg.setWindowTitle(tr("Convert Notes"));
    auto *v = new QVBoxLayout(&dlg);
    auto *toEnd = new QRadioButton(tr("Convert all footnotes to endnotes"), &dlg);
    toEnd->setObjectName(QStringLiteral("convertToEndnotes"));
    auto *toFoot = new QRadioButton(tr("Convert all endnotes to footnotes"), &dlg);
    toFoot->setObjectName(QStringLiteral("convertToFootnotes"));
    auto *swap = new QRadioButton(tr("Swap footnotes and endnotes"), &dlg);
    swap->setObjectName(QStringLiteral("convertSwap"));
    auto *one = new QRadioButton(inFootnote ? tr("Convert this footnote to an endnote") : (inEndnote ? tr("Convert this endnote to a footnote") : tr("Convert only the note the caret is in")), &dlg);
    one->setObjectName(QStringLiteral("convertOne"));
    one->setEnabled(inFootnote || inEndnote);
    toEnd->setChecked(!one->isEnabled());
    one->setChecked(one->isEnabled());
    for (QRadioButton *r : {toEnd, toFoot, swap, one}) v->addWidget(r);
    v->addWidget(okCancel(&dlg));
    if (dlg.exec() != QDialog::Accepted) return false;
    QTextCursor keep = view->cursor();
    int done = 0;
    if (one->isChecked()) done = inFootnote ? convertNotes(*doc, NoteConversion::FootnotesToEndnotes, inFootnote) : convertNotes(*doc, NoteConversion::EndnotesToFootnotes, 0, inEndnote);
    else done = convertNotes(*doc, toEnd->isChecked() ? NoteConversion::FootnotesToEndnotes : (toFoot->isChecked() ? NoteConversion::EndnotesToFootnotes : NoteConversion::Swap));
    if (done) {
        // (The caret was in a note that is no more: it goes to the text's start.)
        QTextCursor c(doc->story());
        view->showCursor(keep.document() == doc->story() ? keep : c);
    }
    return done > 0;
}

bool noteSeparatorsDialog(QWidget *parent, WriteDoc *doc, bool *endnotesChanged)
{
    if (!doc) return false;
    if (endnotesChanged) *endnotesChanged = false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("noteSeparatorsDialog"));
    dlg.setWindowTitle(tr("Note Separators"));
    auto *v = new QVBoxLayout(&dlg);
    struct Row {
        bool endnotes = false;
        QString type;
        QComboBox *kind = nullptr;
        QLineEdit *text = nullptr;
        WriteDoc::NoteSeparator was;
    };
    QVector<Row> rows;
    bool any = false;
    for (const bool endnotes : {false, true}) {
        auto *group = new QGroupBox(endnotes ? tr("Endnotes") : tr("Footnotes"), &dlg);
        group->setObjectName(endnotes ? QStringLiteral("endnoteSeparators") : QStringLiteral("footnoteSeparators"));
        auto *form = new QFormLayout(group);
        const bool has = doc->hasNotesOfKind(endnotes);
        any = any || has;
        group->setEnabled(has);
        if (!has) form->addRow(new QLabel(endnotes ? tr("The document has no endnotes.") : tr("The document has no footnotes."), group));
        const struct {
            const char *type;
            QString label;
        } kinds[] = {{"separator", tr("Separator (between the text and the notes):")},
                     {"continuationSeparator", tr("Continuation separator (above a note that goes on):")},
                     {"continuationNotice", tr("Continuation notice (below a note that goes on):")}};
        for (const auto &k : kinds) {
            Row r;
            r.endnotes = endnotes;
            r.type = QLatin1String(k.type);
            r.was = doc->noteSeparator(endnotes, r.type);
            const QString name = QLatin1String(endnotes ? "end" : "foot") + r.type;
            r.kind = new QComboBox(group);
            r.kind->setObjectName(name + QStringLiteral("Kind"));
            if (r.type != QLatin1String("continuationNotice")) r.kind->addItem(tr("Line"), int(WriteDoc::NoteSeparator::Line));
            r.kind->addItem(tr("Text"), int(WriteDoc::NoteSeparator::Text));
            r.kind->addItem(tr("Nothing"), int(WriteDoc::NoteSeparator::None));
            if (r.was.kind == WriteDoc::NoteSeparator::Kept) r.kind->addItem(tr("As in the file"), int(WriteDoc::NoteSeparator::Kept));
            r.kind->setCurrentIndex(std::max(0, r.kind->findData(int(r.was.kind))));
            r.text = new QLineEdit(r.was.text, group);
            r.text->setObjectName(name + QStringLiteral("Text"));
            r.text->setEnabled(r.was.kind == WriteDoc::NoteSeparator::Text);
            auto *line = new QHBoxLayout;
            line->addWidget(r.kind);
            line->addWidget(r.text, 1);
            form->addRow(k.label, line);
            QObject::connect(r.kind, &QComboBox::currentIndexChanged, r.text, [kind = r.kind, text = r.text] { text->setEnabled(kind->currentData().toInt() == int(WriteDoc::NoteSeparator::Text)); });
            rows.push_back(r);
        }
        v->addWidget(group);
    }
    if (!any) {
        QMessageBox::information(parent, tr("Note Separators"), tr("The document has no footnotes or endnotes yet."));
        return false;
    }
    v->addWidget(okCancel(&dlg));
    if (dlg.exec() != QDialog::Accepted) return false;
    bool changed = false;
    for (const Row &r : rows) {
        if (!doc->hasNotesOfKind(r.endnotes)) continue;
        WriteDoc::NoteSeparator now;
        now.kind = WriteDoc::NoteSeparator::Kind(r.kind->currentData().toInt());
        if (now.kind == WriteDoc::NoteSeparator::Text) now.text = r.text->text();
        if (now == r.was || now.kind == WriteDoc::NoteSeparator::Kept) continue;
        if (doc->setNoteSeparator(r.endnotes, r.type, now)) {
            changed = true;
            if (r.endnotes && endnotesChanged) *endnotesChanged = true;
        }
    }
    return changed;
}

} // namespace jp::write
