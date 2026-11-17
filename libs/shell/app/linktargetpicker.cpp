#include "app/linktargetpicker.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

LinkTargetPicker::LinkTargetPicker(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("linkTargetPicker"));
    setMinimumWidth(380);
    auto *form = new QFormLayout(this);
    form->setContentsMargins(0, 0, 0, 0);
    m_kind = new QComboBox(this);
    m_kind->setObjectName(QStringLiteral("linkKind"));
    m_kind->addItem(QCoreApplication::translate("jp::LinkTargetPicker", "Web address"), Web);
    m_kind->addItem(QCoreApplication::translate("jp::LinkTargetPicker", "Email address"), Email);
    m_placeKindName = QCoreApplication::translate("jp::LinkTargetPicker", "Place in this document");
    m_kind->addItem(m_placeKindName, Place);
    m_kind->addItem(QCoreApplication::translate("jp::LinkTargetPicker", "Existing file"), File);
    form->addRow(new QLabel(QCoreApplication::translate("jp::LinkTargetPicker", "Link to"), this), m_kind);
    m_address = new QLineEdit(this);
    m_address->setObjectName(QStringLiteral("linkAddress"));
    m_addressRow = new QWidget(this);
    auto *row = new QHBoxLayout(m_addressRow);
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(m_address);
    m_browse = new QPushButton(QCoreApplication::translate("jp::LinkTargetPicker", "Browse…"), m_addressRow);
    m_browse->setObjectName(QStringLiteral("linkBrowse"));
    m_browse->setAutoDefault(false);
    row->addWidget(m_browse);
    m_addressLabel = new QLabel(QCoreApplication::translate("jp::LinkTargetPicker", "Address"), this);
    m_addressLabel->setBuddy(m_address);
    form->addRow(m_addressLabel, m_addressRow);
    m_place = new QComboBox(this);
    m_place->setObjectName(QStringLiteral("linkPlace"));
    m_placeRow = m_place;
    m_placeLabel = new QLabel(QCoreApplication::translate("jp::LinkTargetPicker", "Place"), this);
    m_placeLabel->setTextFormat(Qt::PlainText);
    m_placeLabel->setBuddy(m_place);
    form->addRow(m_placeLabel, m_place);
    connect(m_kind, &QComboBox::currentIndexChanged, this, [this] { refresh(); });
    connect(m_address, &QLineEdit::textChanged, this, &LinkTargetPicker::changed);
    connect(m_place, &QComboBox::currentIndexChanged, this, &LinkTargetPicker::changed);
    connect(m_browse, &QPushButton::clicked, this, [this] { browse(); });
    refresh();
}

void LinkTargetPicker::setPlaceNames(const QString &kindName, const QString &fieldLabel)
{
    m_placeKindName = kindName;
    m_kind->setItemText(m_kind->findData(Place), kindName);
    m_placeLabel->setText(fieldLabel);
}

void LinkTargetPicker::setPlaces(const QVector<LinkPlace> &places)
{
    m_place->clear();
    m_truncated = places.size() > kMostLinkPlaces;
    const int n = int(std::min<qsizetype>(places.size(), kMostLinkPlaces));
    for (int i = 0; i < n; ++i) m_place->addItem(places[i].label, places[i].value);
}

int LinkTargetPicker::placeCount() const { return m_place->count(); }

void LinkTargetPicker::enableAppDocument(const QString &kindName, const QString &filter)
{
    m_appDocumentFilter = filter;
    if (m_kind->findData(AppDocument) < 0) m_kind->addItem(kindName, AppDocument);
}

void LinkTargetPicker::enableNewDocument(const QString &filter, const QString &suffix, const std::function<bool(const QString &, QString *)> &createEmpty)
{
    m_newFilter = filter;
    m_newSuffix = suffix;
    m_createEmpty = createEmpty;
    if (m_kind->findData(NewDocument) < 0) m_kind->addItem(QCoreApplication::translate("jp::LinkTargetPicker", "New document"), NewDocument);
}

LinkTargetPicker::Kind LinkTargetPicker::kind() const { return Kind(m_kind->currentData().toInt()); }

void LinkTargetPicker::setKind(Kind k)
{
    const int at = m_kind->findData(k);
    if (at >= 0) m_kind->setCurrentIndex(at);
}

QString LinkTargetPicker::address() const { return m_address->text(); }
void LinkTargetPicker::setAddress(const QString &text) { m_address->setText(text); }
QString LinkTargetPicker::placeValue() const { return m_place->currentData().toString(); }

bool LinkTargetPicker::setPlaceValue(const QString &value)
{
    const int at = m_place->findData(value);
    if (at < 0) return false;
    m_place->setCurrentIndex(at);
    return true;
}

QString LinkTargetPicker::target() const
{
    const QString text = m_address->text().trimmed();
    switch (kind()) {
    case Place: return placeValue();
    case Email:
        if (text.isEmpty()) return QString();
        return QStringLiteral("mailto:") + (text.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive) ? text.mid(7) : text);
    case NewDocument: return m_created.isEmpty() ? QString() : m_created;
    default: return text;
    }
}

bool LinkTargetPicker::prepare(QString *problem)
{
    if (kind() != NewDocument) return true;
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return false;
    };
    QString path = m_address->text().trimmed();
    if (path.isEmpty() || !QDir::isAbsolutePath(path)) return fail(QCoreApplication::translate("jp::LinkTargetPicker", "Choose where the new document goes: type its whole path, or press Browse."));
    if (!m_newSuffix.isEmpty() && QFileInfo(path).suffix().isEmpty()) path += QLatin1Char('.') + m_newSuffix;
    if (path == m_created) return true;   // (made by an earlier try)
    if (!m_created.isEmpty()) discardCreated();
    if (QFileInfo::exists(path)) return fail(QCoreApplication::translate("jp::LinkTargetPicker", "A file with that name is already there. Choose another name."));
    if (!QFileInfo(QFileInfo(path).absolutePath()).isDir()) return fail(QCoreApplication::translate("jp::LinkTargetPicker", "The folder for the new document is not there."));
    QString why;
    if (!m_createEmpty || !m_createEmpty(path, &why)) return fail(why.isEmpty() ? QCoreApplication::translate("jp::LinkTargetPicker", "The new document could not be made.") : why);
    m_created = path;
    return true;
}

void LinkTargetPicker::discardCreated()
{
    if (!m_created.isEmpty()) QFile::remove(m_created);
    m_created.clear();
}

void LinkTargetPicker::refresh()
{
    const Kind k = kind();
    const bool place = k == Place;
    m_addressRow->setVisible(!place);
    m_addressLabel->setVisible(!place);
    m_placeRow->setVisible(place);
    m_placeLabel->setVisible(place);
    m_browse->setVisible(k == File || k == AppDocument || k == NewDocument);
    switch (k) {
    case Email:
        m_addressLabel->setText(QCoreApplication::translate("jp::LinkTargetPicker", "Email address"));
        m_address->setPlaceholderText(QCoreApplication::translate("jp::LinkTargetPicker", "name@example.com"));
        break;
    case File:
    case AppDocument:
        m_addressLabel->setText(QCoreApplication::translate("jp::LinkTargetPicker", "File"));
        m_address->setPlaceholderText(QCoreApplication::translate("jp::LinkTargetPicker", "Path of the file"));
        break;
    case NewDocument:
        m_addressLabel->setText(QCoreApplication::translate("jp::LinkTargetPicker", "Save as"));
        m_address->setPlaceholderText(QCoreApplication::translate("jp::LinkTargetPicker", "Path of the new document"));
        break;
    default:
        m_addressLabel->setText(QCoreApplication::translate("jp::LinkTargetPicker", "Address"));
        m_address->setPlaceholderText(QStringLiteral("https://example.com"));
        break;
    }
    Q_EMIT changed();
}

void LinkTargetPicker::browse()
{
    QString path;
    if (kind() == NewDocument) path = QFileDialog::getSaveFileName(this, QCoreApplication::translate("jp::LinkTargetPicker", "New Document"), QString(), m_newFilter);
    else path = QFileDialog::getOpenFileName(this, QCoreApplication::translate("jp::LinkTargetPicker", "Link to File"), QString(), kind() == AppDocument ? m_appDocumentFilter : QString());
    if (!path.isEmpty()) m_address->setText(path);
}

} // namespace jp
