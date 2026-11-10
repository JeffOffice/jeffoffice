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
#include <QSet>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTimeZone>
#include <cmath>
#include <QUrl>
#include <QVBoxLayout>

namespace jp {

namespace {


QString dateText(const QDateTime &d) { return d.isValid() ? QLocale().toString(d.toLocalTime(), QStringLiteral("MMM d, yyyy h:mm AP")) : QCoreApplication::translate("jp::InfoPage", "Never"); }

QStringList kTypeNames() { return {QCoreApplication::translate("jp::InfoPage", "Text"), QCoreApplication::translate("jp::InfoPage", "Whole number"), QCoreApplication::translate("jp::InfoPage", "Number"), QCoreApplication::translate("jp::InfoPage", "Yes or no"), QCoreApplication::translate("jp::InfoPage", "Date")}; }
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

namespace {

enum Column { NameColumn, KindColumn, ValueColumn };
constexpr int kRawRole = Qt::UserRole + 1, kPropertyRole = Qt::UserRole + 2;

// The kind column: shown as the kind's name, and a list of the kinds in the cell when it is edited (one list for the table, not one for each row).
class KindDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override
    {
        if (!(index.flags() & Qt::ItemIsEditable)) return nullptr;
        auto *box = new QComboBox(parent);
        box->addItems(kTypeNames());
        return box;
    }
    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        auto *box = static_cast<QComboBox *>(editor);
        const int at = box->findText(index.data(Qt::DisplayRole).toString());
        box->setCurrentIndex(std::max(0, at));
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        model->setData(index, static_cast<QComboBox *>(editor)->currentText(), Qt::DisplayRole);
    }
};

QString kindName(const QString &code)
{
    for (int i = 0; i < 5; ++i)
        if (code == QLatin1String(kTypeCodes[i])) return kTypeNames().value(i);
    return {};
}

QString kindCode(const QString &name)
{
    const QStringList names = kTypeNames();
    for (int i = 0; i < 5; ++i)
        if (name == names.value(i)) return QLatin1String(kTypeCodes[i]);
    return QStringLiteral("lpwstr");
}

// The value as the file keeps it for its kind, or the reason it can't be.
bool valueFor(const QString &code, const QString &text, QString *value, QString *why)
{
    const QString t = text.trimmed();
    if (code == QLatin1String("i4")) {
        bool ok = false;
        const int n = QLocale::c().toInt(t, &ok);
        if (!ok) {
            *why = QCoreApplication::translate("jp::InfoPage", "\"%1\" is not a whole number.").arg(text);
            return false;
        }
        *value = QString::number(n);
    } else if (code == QLatin1String("r8")) {
        bool ok = false;
        const double d = QLocale::c().toDouble(t, &ok);
        if (!ok || !std::isfinite(d)) {
            *why = QCoreApplication::translate("jp::InfoPage", "\"%1\" is not a number.").arg(text);
            return false;
        }
        *value = QString::number(d, 'g', 15);
    } else if (code == QLatin1String("bool")) {
        const QString v = t.toLower();
        if (v == QLatin1String("true") || v == QLatin1String("yes") || v == QLatin1String("1")) *value = QStringLiteral("true");
        else if (v == QLatin1String("false") || v == QLatin1String("no") || v == QLatin1String("0")) *value = QStringLiteral("false");
        else {
            *why = QCoreApplication::translate("jp::InfoPage", "\"%1\" is not yes or no.").arg(text);
            return false;
        }
    } else if (code == QLatin1String("filetime")) {
        QDateTime d = QDateTime::fromString(t, Qt::ISODate);
        if (!d.isValid()) {
            const QDate day = QLocale().toDate(t, QLocale::ShortFormat);
            if (day.isValid()) d = QDateTime(day, QTime(0, 0), QTimeZone::UTC);
        }
        if (!d.isValid()) {
            *why = QCoreApplication::translate("jp::InfoPage", "\"%1\" is not a date. Type it as 2026-11-04, or as your computer writes dates.").arg(text);
            return false;
        }
        *value = d.toUTC().toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'"));
    } else {
        *value = text;
    }
    return true;
}

} // namespace

CustomPropertiesDialog::CustomPropertiesDialog(const QVector<office::CustomProperty> &now, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("customPropertiesDialog"));
    setWindowTitle(QCoreApplication::translate("jp::InfoPage", "Custom Properties"));
    resize(560, 360);
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(QCoreApplication::translate("jp::InfoPage", "A custom property is a name with a value that you choose, such as Project or Client. Fields in the file can show it."), this);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    m_model = new QStandardItemModel(0, 3, this);
    m_model->setHorizontalHeaderLabels({QCoreApplication::translate("jp::InfoPage", "Name"), QCoreApplication::translate("jp::InfoPage", "Kind"), QCoreApplication::translate("jp::InfoPage", "Value")});
    m_table = new QTableView(this);
    m_table->setObjectName(QStringLiteral("customPropertiesTable"));
    m_table->setModel(m_model);
    m_table->setItemDelegateForColumn(KindColumn, new KindDelegate(m_table));
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(m_table, 1);
    auto *row = new QHBoxLayout;
    auto *add = new QPushButton(QCoreApplication::translate("jp::InfoPage", "Add"), this);
    add->setObjectName(QStringLiteral("customAdd"));
    auto *remove = new QPushButton(QCoreApplication::translate("jp::InfoPage", "Delete"), this);
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
        const QModelIndex at = m_table->currentIndex();
        if (at.isValid()) m_model->removeRow(at.row());
    });
    for (const office::CustomProperty &p : now) {
        // (the mark of a file marked as final is the page's, not a property to edit)
        if (p.name == QLatin1String(office::kMarkAsFinalName)) continue;
        addRow(p);
    }
}

void CustomPropertiesDialog::addRow(const office::CustomProperty &p)
{
    const bool raw = !p.raw.isEmpty();
    auto *name = new QStandardItem(p.name);
    auto *kind = new QStandardItem(raw ? QCoreApplication::translate("jp::InfoPage", "Other") : kindName(p.type));
    auto *value = new QStandardItem(raw ? QCoreApplication::translate("jp::InfoPage", "(kept as it is)") : p.value);
    // (kept for the row: the property as the file had it, which a property not modeled is given back as)
    name->setData(QVariant::fromValue(p.raw), kRawRole);
    name->setData(p.pid, kPropertyRole);
    name->setData(p.fmtid, kPropertyRole + 1);
    name->setData(p.type, kPropertyRole + 2);
    name->setData(p.value, kPropertyRole + 3);
    if (raw)
        for (QStandardItem *i : {name, kind, value}) i->setEditable(false);
    m_model->appendRow({name, kind, value});
}

void CustomPropertiesDialog::addEmptyRow()
{
    addRow({QString(), QStringLiteral("lpwstr"), QString(), 0, QString(), QByteArray()});
    const QModelIndex at = m_model->index(m_model->rowCount() - 1, NameColumn);
    m_table->setCurrentIndex(at);
    m_table->edit(at);
}

int CustomPropertiesDialog::rowCount() const { return m_model->rowCount(); }

void CustomPropertiesDialog::setCell(int row, int column, const QString &text) { m_model->setData(m_model->index(row, column), text); }

QString CustomPropertiesDialog::problem() const
{
    QSet<QString> seen;
    for (int r = 0; r < m_model->rowCount(); ++r) {
        if (!m_model->item(r, NameColumn)->data(kRawRole).toByteArray().isEmpty()) continue;
        const QString name = m_model->item(r, NameColumn)->text().trimmed();
        if (name.isEmpty()) return QCoreApplication::translate("jp::InfoPage", "Row %1 has no name. Type a name, or delete the row.").arg(r + 1);
        if (name.startsWith(QLatin1Char('_'))) return QCoreApplication::translate("jp::InfoPage", "\"%1\" is a name the program keeps for its own use. Choose another name.").arg(name);
        if (seen.contains(name.toLower())) return QCoreApplication::translate("jp::InfoPage", "The name \"%1\" is used twice. Each property needs a name of its own.").arg(name);
        seen.insert(name.toLower());
        QString value, why;
        if (!valueFor(kindCode(m_model->item(r, KindColumn)->text()), m_model->item(r, ValueColumn)->text(), &value, &why)) return QCoreApplication::translate("jp::InfoPage", "%1: %2").arg(name, why);
    }
    return {};
}

void CustomPropertiesDialog::accept()
{
    const QString why = problem();
    if (!why.isEmpty()) {
        plainWarning(this, windowTitle(), why);
        return;
    }
    QDialog::accept();
}

QVector<office::CustomProperty> CustomPropertiesDialog::result() const
{
    QVector<office::CustomProperty> out;
    for (int r = 0; r < m_model->rowCount(); ++r) {
        const QStandardItem *name = m_model->item(r, NameColumn);
        office::CustomProperty p;
        p.raw = name->data(kRawRole).toByteArray();
        p.pid = name->data(kPropertyRole).toInt();
        p.fmtid = name->data(kPropertyRole + 1).toString();
        p.name = name->text().trimmed();
        if (!p.raw.isEmpty()) {
            p.type = name->data(kPropertyRole + 2).toString();
            p.value = name->data(kPropertyRole + 3).toString();
            out.push_back(p);   // (as the file had it)
            continue;
        }
        p.type = kindCode(m_model->item(r, KindColumn)->text());
        QString why;
        if (p.name.isEmpty() || !valueFor(p.type, m_model->item(r, ValueColumn)->text(), &p.value, &why)) continue;
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
    auto *intro = new QLabel(QCoreApplication::translate("jp::InfoPage", "This is what was found that you may not want to share. Remove what you don't want to keep: it can't be brought back, except with Undo where the app has it."), this);
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
        auto *button = new QPushButton(m_items[i].action.isEmpty() ? QCoreApplication::translate("jp::InfoPage", "Remove All") : m_items[i].action, this);
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
        m_counts[i]->setText(n > 0 ? QCoreApplication::translate("jp::InfoPage", "%1 found").arg(n) : QCoreApplication::translate("jp::InfoPage", "None found"));
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
    const InfoHost::Kind kind = h.kind;
    const bool book = kind == InfoHost::Kind::Workbook;
    auto *page = new QWidget(parent);
    page->setObjectName(QStringLiteral("infoPage"));
    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(40, 30, 40, 30);
    outer->addWidget(heading(QCoreApplication::translate("jp::InfoPage", "Info"), page));

    const QString path = h.path ? h.path() : QString();
    auto *where = new QHBoxLayout;
    where->setSpacing(10);
    auto *pathIcon = new QLabel(page);
    pathIcon->setPixmap(icon(path.isEmpty() ? "file" : "folder").pixmap(QSize(16, 16), page->devicePixelRatioF()));
    where->addWidget(pathIcon);
    auto *pathLabel = mutedLabel(path.isEmpty() ? (book ? QCoreApplication::translate("jp::InfoPage", "This workbook hasn't been saved yet.")
                                                       : kind == InfoHost::Kind::Presentation ? QCoreApplication::translate("jp::InfoPage", "This presentation hasn't been saved yet.")
                                                       : kind == InfoHost::Kind::Publication  ? QCoreApplication::translate("jp::InfoPage", "This publication hasn't been saved yet.")
                                                                                              : QCoreApplication::translate("jp::InfoPage", "This document hasn't been saved yet."))
                                                    : pathForCopy(path), page, 0.7);
    pathLabel->setObjectName(QStringLiteral("infoPath"));
    pathLabel->setTextFormat(Qt::PlainText);
    pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pathLabel->setWordWrap(true);
    where->addWidget(pathLabel, 1);
    auto *show = new QPushButton(QCoreApplication::translate("jp::InfoPage", "Show in Folder"), page);
    show->setObjectName(QStringLiteral("infoShowFolder"));
    show->setEnabled(!folderOfFile(path).isEmpty());
    QObject::connect(show, &QPushButton::clicked, page, [path] {
        const QString dir = folderOfFile(path);
        if (!dir.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });
    where->addWidget(show);
    auto *copy = new QPushButton(QCoreApplication::translate("jp::InfoPage", "Copy Path"), page);
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
    const QString blocked = h.markFinalBlocked ? h.markFinalBlocked() : QString();
    if (h.setMarkedFinal && !blocked.isEmpty()) {
        auto *b = new ActionCard(QStringLiteral("flag"), QCoreApplication::translate("jp::InfoPage", "Mark as Final"), blocked, page);
        b->setObjectName(QStringLiteral("infoMarkFinal"));
        b->setEnabled(false);
        left->addWidget(b);
    } else if (h.setMarkedFinal)
        card(QStringLiteral("infoMarkFinal"), QStringLiteral("flag"), final ? QCoreApplication::translate("jp::InfoPage", "Marked as Final") : QCoreApplication::translate("jp::InfoPage", "Mark as Final"),
             final ? (book ? QCoreApplication::translate("jp::InfoPage", "This workbook is marked as final: it opens for reading only. Choose this to let people edit it again.")
                           : kind == InfoHost::Kind::Presentation ? QCoreApplication::translate("jp::InfoPage", "This presentation is marked as final: it opens for reading only. Choose this to let people edit it again.")
                           : kind == InfoHost::Kind::Publication  ? QCoreApplication::translate("jp::InfoPage", "This publication is marked as final: it opens for reading only. Choose this to let people edit it again.")
                                                                  : QCoreApplication::translate("jp::InfoPage", "This document is marked as final: it opens for reading only. Choose this to let people edit it again."))
                   : (book ? QCoreApplication::translate("jp::InfoPage", "Tell readers this workbook is finished. It opens for reading only, with a note.")
                           : kind == InfoHost::Kind::Presentation ? QCoreApplication::translate("jp::InfoPage", "Tell readers this presentation is finished. It opens for reading only, with a note.")
                           : kind == InfoHost::Kind::Publication  ? QCoreApplication::translate("jp::InfoPage", "Tell readers this publication is finished. It opens for reading only, with a note.")
                                                                  : QCoreApplication::translate("jp::InfoPage", "Tell readers this document is finished. It opens for reading only, with a note.")),
             [h, final] { h.setMarkedFinal(!final); });
    if (h.encrypt && h.encryptBlocked && !h.encryptBlocked().isEmpty()) {
        const QString why = h.encryptBlocked();
        auto *b = new ActionCard(QStringLiteral("lock"), QCoreApplication::translate("jp::InfoPage", "Encrypt with Password"), why, page);
        b->setObjectName(QStringLiteral("infoEncrypt"));
        b->setToolTip(why);
        b->setEnabled(false);
        left->addWidget(b);
    } else if (h.encrypt) {
        const bool locked = h.hasPassword && h.hasPassword();
        card(QStringLiteral("infoEncrypt"), QStringLiteral("lock"), locked ? QCoreApplication::translate("jp::InfoPage", "Change or Remove the Password") : QCoreApplication::translate("jp::InfoPage", "Encrypt with Password"),
             locked ? QCoreApplication::translate("jp::InfoPage", "This file is protected: it asks for a password when it is opened. Change the password or take it away.")
                    : QCoreApplication::translate("jp::InfoPage", "Ask for a password to open this file. The file is scrambled when you save it."),
             [h] { h.encrypt(); });
    }
    if (h.protectSheet)
        card(QStringLiteral("infoProtectSheet"), QStringLiteral("lock"), QCoreApplication::translate("jp::InfoPage", "Protect Current Sheet"),
             QCoreApplication::translate("jp::InfoPage", "Choose what people may change on this sheet, with a password if you like."), [h] { h.protectSheet(); });
    if (h.protectStructure)
        card(QStringLiteral("infoProtectStructure"), QStringLiteral("lock"), QCoreApplication::translate("jp::InfoPage", "Protect Workbook Structure"),
             QCoreApplication::translate("jp::InfoPage", "Stop people from adding, deleting, moving, hiding, or renaming sheets."), [h] { h.protectStructure(); });
    if (h.inspect)
        card(QStringLiteral("infoInspect"), QStringLiteral("scan-search"), book ? QCoreApplication::translate("jp::InfoPage", "Inspect Workbook")
                                                                                                       : kind == InfoHost::Kind::Presentation ? QCoreApplication::translate("jp::InfoPage", "Inspect Presentation")
                                                                                                       : kind == InfoHost::Kind::Publication  ? QCoreApplication::translate("jp::InfoPage", "Inspect Publication")
                                                                                                                                              : QCoreApplication::translate("jp::InfoPage", "Inspect Document"),
             QCoreApplication::translate("jp::InfoPage", "Find what you may not want to share, such as comments and personal information, and remove it."), [h] { h.inspect(); });
    if (h.checkCompatibility)
        card(QStringLiteral("infoCompatibility"), QStringLiteral("list-checks"), QCoreApplication::translate("jp::InfoPage", "Check Compatibility"), QCoreApplication::translate("jp::InfoPage", "See what an older kind of file can't hold, before you save to it."),
             [h] { h.checkCompatibility(); });
    if (h.checkAccessibility)
        card(QStringLiteral("infoAccessibility"), QStringLiteral("list-checks"), QCoreApplication::translate("jp::InfoPage", "Check Accessibility"),
             QCoreApplication::translate("jp::InfoPage", "Find what may be hard for some people to read or use, such as a picture with no description."), [h] { h.checkAccessibility(); });
    if (h.versionHistory)
        card(QStringLiteral("infoVersions"), QStringLiteral("history"), QCoreApplication::translate("jp::InfoPage", "Version History"), QCoreApplication::translate("jp::InfoPage", "Keep named copies of this file on your computer, and compare them or go back to one."),
             [h] { h.versionHistory(); });
    for (const InfoHost::Card &c : h.extraCards) card(c.name, c.icon, c.title, c.text, c.run);
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
    auto *dh = new QLabel(QCoreApplication::translate("jp::InfoPage", "Properties"), details);
    QFont df = dh->font();
    df.setWeight(QFont::DemiBold);
    df.setPointSizeF(df.pointSizeF() * 1.1);
    dh->setFont(df);
    dv->addWidget(dh);
    if (props.truncated) {
        auto *note = mutedLabel(QCoreApplication::translate("jp::InfoPage", "This file has more properties than %1 reads, so they are shown here but can't be changed. The file keeps them as they are.").arg(QCoreApplication::applicationName()), details);
        note->setObjectName(QStringLiteral("infoPropertiesReadOnly"));
        note->setWordWrap(true);
        dv->addWidget(note);
    }
    auto *form = new QFormLayout;
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(6);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    // The edits are kept in one place and handed on when a box is left.
    auto state = std::make_shared<office::PackageProps>(props);
    auto field = [&](const char *objectName, const QString &label, const QString &value, QString office::PackageProps::*member) {
        auto *edit = new QLineEdit(value, details);
        edit->setReadOnly(props.truncated);
        edit->setObjectName(QString::fromLatin1(objectName));
        edit->setAccessibleName(label);
        edit->setPlaceholderText(QCoreApplication::translate("jp::InfoPage", "Add text"));
        QObject::connect(edit, &QLineEdit::editingFinished, page, [state, edit, member, h] {
            if (state.get()->*member == edit->text().trimmed()) return;
            state.get()->*member = edit->text().trimmed();
            if (h.setProps) h.setProps(*state);
        });
        form->addRow(label, edit);
    };
    field("infoTitle", QCoreApplication::translate("jp::InfoPage", "Title"), props.title, &office::PackageProps::title);
    field("infoAuthor", QCoreApplication::translate("jp::InfoPage", "Author"), props.creator, &office::PackageProps::creator);
    field("infoSubject", QCoreApplication::translate("jp::InfoPage", "Subject"), props.subject, &office::PackageProps::subject);
    field("infoKeywords", QCoreApplication::translate("jp::InfoPage", "Keywords"), props.keywords, &office::PackageProps::keywords);
    field("infoCategory", QCoreApplication::translate("jp::InfoPage", "Categories"), props.category, &office::PackageProps::category);
    field("infoComments", QCoreApplication::translate("jp::InfoPage", "Comments"), props.comments, &office::PackageProps::comments);
    field("infoCompany", QCoreApplication::translate("jp::InfoPage", "Company"), props.company, &office::PackageProps::company);
    field("infoManager", QCoreApplication::translate("jp::InfoPage", "Manager"), props.manager, &office::PackageProps::manager);
    dv->addLayout(form);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(5);
    QVector<QPair<QString, QString>> facts = {{QCoreApplication::translate("jp::InfoPage", "Created"), dateText(props.created)}, {QCoreApplication::translate("jp::InfoPage", "Last modified"), dateText(props.modified)}, {QCoreApplication::translate("jp::InfoPage", "Last printed"), dateText(props.printed)}};
    if (!props.lastModifiedBy.isEmpty()) facts.push_back({QCoreApplication::translate("jp::InfoPage", "Last modified by"), props.lastModifiedBy});
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
    auto *custom = new QPushButton(QCoreApplication::translate("jp::InfoPage", "Custom Properties…"), details);
    custom->setObjectName(QStringLiteral("infoCustom"));
    custom->setEnabled(!props.truncated);
    custom->setMinimumHeight(34);
    QObject::connect(custom, &QPushButton::clicked, page, [state, h, page] {
        CustomPropertiesDialog dialog(state->custom, page);
        if (dialog.exec() != QDialog::Accepted) return;
        QVector<office::CustomProperty> edited = dialog.result();
        // (the mark of a file marked as final is kept as it was)
        for (qsizetype i = 0; i < state->custom.size(); ++i)
            if (state->custom[i].name == QLatin1String(office::kMarkAsFinalName)) edited.insert(std::min(i, edited.size()), state->custom[i]);
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
