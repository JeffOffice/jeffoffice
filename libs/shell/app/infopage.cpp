#include "app/infopage.h"

#include "app/filescreenparts.h"
#include "app/icons.h"
#include "app/plaintext.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace jp {

namespace {

struct Words {
    Q_DECLARE_TR_FUNCTIONS(InfoPage)
};

QString dateText(const QDateTime &d) { return d.isValid() ? QLocale().toString(d.toLocalTime(), QStringLiteral("MMM d, yyyy h:mm AP")) : Words::tr("Never"); }

QStringList kTypeNames() { return {Words::tr("Text"), Words::tr("Whole number"), Words::tr("Number"), Words::tr("Yes or no"), Words::tr("Date")}; }
const char *const kTypeCodes[] = {"lpwstr", "i4", "r8", "bool", "filetime"};

} // namespace

QString folderOfFile(const QString &path)
{
    if (path.isEmpty()) return {};
    const QFileInfo fi(path);
    return fi.exists() ? fi.absolutePath() : QString();
}

QString pathForCopy(const QString &path) { return QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath()); }

// ---- the custom properties box

CustomPropertiesDialog::CustomPropertiesDialog(const QVector<office::CustomProperty> &now, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("customPropertiesDialog"));
    setWindowTitle(Words::tr("Custom Properties"));
    resize(560, 360);
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(Words::tr("A custom property is a name with a value that you choose, such as Project or Client. Fields in the file can show it."), this);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    m_table = new QTableWidget(0, 3, this);
    m_table->setObjectName(QStringLiteral("customPropertiesTable"));
    m_table->setHorizontalHeaderLabels({Words::tr("Name"), Words::tr("Kind"), Words::tr("Value")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(m_table, 1);
    auto *row = new QHBoxLayout;
    auto *add = new QPushButton(Words::tr("Add"), this);
    add->setObjectName(QStringLiteral("customAdd"));
    auto *remove = new QPushButton(Words::tr("Delete"), this);
    remove->setObjectName(QStringLiteral("customDelete"));
    row->addWidget(add);
    row->addWidget(remove);
    row->addStretch(1);
    layout->addLayout(row);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(add, &QPushButton::clicked, this, [this] { addEmptyRow(); });
    connect(remove, &QPushButton::clicked, this, [this] {
        const int r = m_table->currentRow();
        if (r >= 0) m_table->removeRow(r);
    });
    for (const office::CustomProperty &p : now) {
        // (the mark of a file marked as final is the page's, not a property to edit)
        if (p.name == QLatin1String(office::kMarkAsFinalName)) continue;
        addRow(p);
    }
}

void CustomPropertiesDialog::addRow(const office::CustomProperty &p)
{
    const int r = m_table->rowCount();
    m_table->insertRow(r);
    m_table->setItem(r, 0, new QTableWidgetItem(p.name));
    auto *kind = new QComboBox(m_table);
    kind->addItems(kTypeNames());
    int at = 0;
    for (int i = 0; i < 5; ++i)
        if (p.type == QLatin1String(kTypeCodes[i])) at = i;
    kind->setCurrentIndex(at);
    m_table->setCellWidget(r, 1, kind);
    m_table->setItem(r, 2, new QTableWidgetItem(p.value));
}

void CustomPropertiesDialog::addEmptyRow()
{
    addRow({QString(), QStringLiteral("lpwstr"), QString()});
    m_table->setCurrentCell(m_table->rowCount() - 1, 0);
    m_table->editItem(m_table->item(m_table->rowCount() - 1, 0));
}

int CustomPropertiesDialog::rowCount() const { return m_table->rowCount(); }

QVector<office::CustomProperty> CustomPropertiesDialog::result() const
{
    QVector<office::CustomProperty> out;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        office::CustomProperty p;
        p.name = m_table->item(r, 0) ? m_table->item(r, 0)->text().trimmed() : QString();
        auto *kind = qobject_cast<QComboBox *>(m_table->cellWidget(r, 1));
        p.type = QLatin1String(kTypeCodes[kind ? std::clamp(kind->currentIndex(), 0, 4) : 0]);
        p.value = m_table->item(r, 2) ? m_table->item(r, 2)->text() : QString();
        if (p.name.isEmpty()) continue;
        // A value that its kind can't hold is made one that it can.
        bool ok = true;
        if (p.type == QLatin1String("i4")) p.value = QString::number(p.value.toInt(&ok));
        else if (p.type == QLatin1String("r8")) p.value = QString::number(p.value.toDouble(&ok), 'g', 15);
        else if (p.type == QLatin1String("bool")) {
            const QString v = p.value.trimmed().toLower();
            p.value = (v == QLatin1String("true") || v == QLatin1String("yes") || v == QLatin1String("1")) ? QStringLiteral("true") : QStringLiteral("false");
        } else if (p.type == QLatin1String("filetime")) {
            QDateTime d = QDateTime::fromString(p.value.trimmed(), Qt::ISODate);
            if (!d.isValid()) d = QDateTime(QLocale().toDate(p.value.trimmed(), QLocale::ShortFormat), QTime(0, 0), QTimeZone::UTC);
            if (!d.isValid()) d = QDateTime::currentDateTimeUtc();
            p.value = d.toUTC().toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'"));
        }
        out.push_back(p);
    }
    return out;
}

// ---- the Inspect box

InspectDialog::InspectDialog(const QString &heading, const QVector<InspectItem> &items, QWidget *parent) : QDialog(parent), m_items(items)
{
    setObjectName(QStringLiteral("inspectDialog"));
    setWindowTitle(heading);
    resize(560, 120);
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(Words::tr("This is what was found that you may not want to share. Remove what you don't want to keep: it can't be brought back, except with Undo where the app has it."), this);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *grid = new QGridLayout;
    grid->setColumnStretch(0, 1);
    for (int i = 0; i < m_items.size(); ++i) {
        auto *title = plainLabel(new QLabel(m_items[i].title, this));
        QFont f = title->font();
        f.setWeight(QFont::DemiBold);
        title->setFont(f);
        auto *detail = mutedLabel(m_items[i].detail, this);
        detail->setWordWrap(true);
        auto *text = new QVBoxLayout;
        text->setSpacing(1);
        text->addWidget(title);
        text->addWidget(detail);
        grid->addLayout(text, i, 0);
        auto *count = plainLabel(new QLabel(this));
        count->setObjectName(QStringLiteral("inspectCount%1").arg(i));
        m_counts.push_back(count);
        grid->addWidget(count, i, 1);
        auto *button = new QPushButton(m_items[i].action.isEmpty() ? Words::tr("Remove All") : m_items[i].action, this);
        button->setObjectName(QStringLiteral("inspectRemove%1").arg(i));
        m_buttons.push_back(button);
        grid->addWidget(button, i, 2);
        connect(button, &QPushButton::clicked, this, [this, i] { removeRow(i); });
    }
    layout->addLayout(grid);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);
    refresh();
}

void InspectDialog::refresh()
{
    for (int i = 0; i < m_items.size(); ++i) {
        const int n = m_items[i].count ? m_items[i].count() : 0;
        m_counts[i]->setText(n > 0 ? Words::tr("%1 found").arg(n) : Words::tr("None found"));
        m_buttons[i]->setEnabled(n > 0 && m_items[i].removable);
        m_buttons[i]->setVisible(m_items[i].removable);
    }
}

QStringList InspectDialog::summaries() const
{
    QStringList out;
    for (int i = 0; i < m_items.size(); ++i) out << m_items[i].title + QStringLiteral(": ") + m_counts[i]->text();
    return out;
}

void InspectDialog::removeRow(int row)
{
    if (row < 0 || row >= m_items.size() || !m_items[row].remove || !m_items[row].removable) return;
    m_items[row].remove();
    refresh();
}

// ---- the page

QWidget *buildInfoPage(QWidget *parent, const InfoHost &h)
{
    const bool book = h.kind == InfoHost::Kind::Workbook;
    auto *page = new QWidget(parent);
    page->setObjectName(QStringLiteral("infoPage"));
    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(40, 30, 40, 30);
    outer->addWidget(heading(Words::tr("Info"), page));

    const QString path = h.path ? h.path() : QString();
    auto *where = new QHBoxLayout;
    where->setSpacing(10);
    auto *pathIcon = new QLabel(page);
    pathIcon->setPixmap(icon(path.isEmpty() ? "file" : "folder").pixmap(QSize(16, 16), page->devicePixelRatioF()));
    where->addWidget(pathIcon);
    auto *pathLabel = mutedLabel(path.isEmpty() ? (book ? Words::tr("This workbook hasn't been saved yet.") : Words::tr("This document hasn't been saved yet.")) : pathForCopy(path), page, 0.7);
    pathLabel->setObjectName(QStringLiteral("infoPath"));
    pathLabel->setTextFormat(Qt::PlainText);
    pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pathLabel->setWordWrap(true);
    where->addWidget(pathLabel, 1);
    auto *show = new QPushButton(Words::tr("Show in Folder"), page);
    show->setObjectName(QStringLiteral("infoShowFolder"));
    show->setEnabled(!folderOfFile(path).isEmpty());
    QObject::connect(show, &QPushButton::clicked, page, [path] {
        const QString dir = folderOfFile(path);
        if (!dir.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });
    where->addWidget(show);
    auto *copy = new QPushButton(Words::tr("Copy Path"), page);
    copy->setObjectName(QStringLiteral("infoCopyPath"));
    copy->setEnabled(!path.isEmpty());
    QObject::connect(copy, &QPushButton::clicked, page, [path] { QApplication::clipboard()->setText(pathForCopy(path)); });
    where->addWidget(copy);
    outer->addLayout(where);
    outer->addSpacing(14);

    auto *body = new QHBoxLayout;
    body->setSpacing(20);
    auto *left = new QVBoxLayout;
    left->setSpacing(10);
    auto card = [&](const QString &name, const QString &ic, const QString &title, const QString &text, const std::function<void()> &run) {
        auto *b = new ActionCard(ic, title, text, page);
        b->setObjectName(name);
        QObject::connect(b, &QAbstractButton::clicked, page, [run] { run(); });
        left->addWidget(b);
    };
    const bool final = h.markedFinal && h.markedFinal();
    if (h.setMarkedFinal)
        card(QStringLiteral("infoMarkFinal"), QStringLiteral("flag"), final ? Words::tr("Marked as Final") : Words::tr("Mark as Final"),
             final ? (book ? Words::tr("This workbook is marked as final: it opens for reading only. Choose this to let people edit it again.") : Words::tr("This document is marked as final: it opens for reading only. Choose this to let people edit it again."))
                   : (book ? Words::tr("Tell readers this workbook is finished. It opens for reading only, with a note.") : Words::tr("Tell readers this document is finished. It opens for reading only, with a note.")),
             [h, final] { h.setMarkedFinal(!final); });
    if (h.encrypt) {
        const bool locked = h.hasPassword && h.hasPassword();
        card(QStringLiteral("infoEncrypt"), QStringLiteral("lock"), locked ? Words::tr("Change or Remove the Password") : Words::tr("Encrypt with Password"),
             locked ? Words::tr("This file is protected: it asks for a password when it is opened. Change the password or take it away.")
                    : Words::tr("Ask for a password to open this file. The file is scrambled when you save it."),
             [h] { h.encrypt(); });
    }
    if (h.inspect)
        card(QStringLiteral("infoInspect"), QStringLiteral("scan-search"), book ? Words::tr("Inspect Workbook") : Words::tr("Inspect Document"),
             Words::tr("Find what you may not want to share, such as comments and personal information, and remove it."), [h] { h.inspect(); });
    if (h.checkCompatibility)
        card(QStringLiteral("infoCompatibility"), QStringLiteral("list-checks"), Words::tr("Check Compatibility"), Words::tr("See what an older kind of file can't hold, before you save to it."),
             [h] { h.checkCompatibility(); });
    if (h.versionHistory)
        card(QStringLiteral("infoVersions"), QStringLiteral("history"), Words::tr("Version History"), Words::tr("Keep named copies of this file on your computer, and compare them or go back to one."),
             [h] { h.versionHistory(); });
    left->addStretch(1);
    body->addLayout(left, 1);

    // The properties card.
    const office::PackageProps props = h.props ? h.props() : office::PackageProps();
    auto *details = cardFrame(page);
    details->setObjectName(QStringLiteral("infoProperties"));
    details->setFixedWidth(360);
    auto *dv = new QVBoxLayout(details);
    dv->setContentsMargins(18, 18, 18, 18);
    dv->setSpacing(10);
    if (h.thumbnail) {
        const QImage img = h.thumbnail();
        if (!img.isNull()) {
            auto *thumb = plainLabel(new QLabel(details));
            thumb->setFixedSize(324, 190);
            thumb->setAlignment(Qt::AlignCenter);
            const qreal dpr = page->devicePixelRatioF();
            QImage scaled = img;
            scaled.setDevicePixelRatio(dpr);
            thumb->setPixmap(paperPixmap(scaled.convertToFormat(QImage::Format_ARGB32_Premultiplied), thumb->size(), dpr));
            dv->addWidget(thumb, 0, Qt::AlignHCenter);
        }
    }
    auto *dh = new QLabel(Words::tr("Properties"), details);
    QFont df = dh->font();
    df.setWeight(QFont::DemiBold);
    df.setPointSizeF(df.pointSizeF() * 1.1);
    dh->setFont(df);
    dv->addWidget(dh);
    auto *form = new QFormLayout;
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(6);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    // The edits are kept in one place and handed on when a box is left.
    auto state = std::make_shared<office::PackageProps>(props);
    auto field = [&](const char *objectName, const QString &label, const QString &value, QString office::PackageProps::*member) {
        auto *edit = new QLineEdit(value, details);
        edit->setObjectName(QString::fromLatin1(objectName));
        edit->setAccessibleName(label);
        edit->setPlaceholderText(Words::tr("Add text"));
        QObject::connect(edit, &QLineEdit::editingFinished, page, [state, edit, member, h] {
            if (state.get()->*member == edit->text().trimmed()) return;
            state.get()->*member = edit->text().trimmed();
            if (h.setProps) h.setProps(*state);
        });
        form->addRow(label, edit);
    };
    field("infoTitle", Words::tr("Title"), props.title, &office::PackageProps::title);
    field("infoAuthor", Words::tr("Author"), props.creator, &office::PackageProps::creator);
    field("infoSubject", Words::tr("Subject"), props.subject, &office::PackageProps::subject);
    field("infoKeywords", Words::tr("Keywords"), props.keywords, &office::PackageProps::keywords);
    field("infoCategory", Words::tr("Categories"), props.category, &office::PackageProps::category);
    field("infoComments", Words::tr("Comments"), props.comments, &office::PackageProps::comments);
    field("infoCompany", Words::tr("Company"), props.company, &office::PackageProps::company);
    field("infoManager", Words::tr("Manager"), props.manager, &office::PackageProps::manager);
    dv->addLayout(form);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(5);
    QVector<QPair<QString, QString>> facts = {{Words::tr("Created"), dateText(props.created)}, {Words::tr("Last modified"), dateText(props.modified)}, {Words::tr("Last printed"), dateText(props.printed)}};
    if (!props.lastModifiedBy.isEmpty()) facts.push_back({Words::tr("Last modified by"), props.lastModifiedBy});
    if (h.statistics) facts += h.statistics();
    for (int i = 0; i < facts.size(); ++i) {
        grid->addWidget(mutedLabel(facts[i].first, details), i, 0, Qt::AlignLeft | Qt::AlignTop);
        auto *val = plainLabel(new QLabel(facts[i].second, details));
        val->setWordWrap(true);
        val->setTextInteractionFlags(Qt::TextSelectableByMouse);
        grid->addWidget(val, i, 1);
    }
    grid->setColumnStretch(1, 1);
    dv->addLayout(grid);
    auto *custom = new QPushButton(Words::tr("Custom Properties…"), details);
    custom->setObjectName(QStringLiteral("infoCustom"));
    custom->setMinimumHeight(34);
    QObject::connect(custom, &QPushButton::clicked, page, [state, h, page] {
        CustomPropertiesDialog dialog(state->custom, page);
        if (dialog.exec() != QDialog::Accepted) return;
        QVector<office::CustomProperty> edited = dialog.result();
        // (the mark of a file marked as final is kept as it was)
        if (office::isMarkedFinal(state->custom)) edited = office::withMarkAsFinal(edited, true);
        if (edited == state->custom) return;
        state->custom = edited;
        if (h.setProps) h.setProps(*state);
    });
    dv->addWidget(custom);
    body->addWidget(details, 0, Qt::AlignTop);
    outer->addLayout(body, 1);
    return page;
}

} // namespace jp
