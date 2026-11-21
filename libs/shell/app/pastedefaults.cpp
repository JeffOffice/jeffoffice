#include "app/pastedefaults.h"

#include "app/settings.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QFormLayout>
#include <QMimeData>
#include <QUuid>

namespace jp {

namespace {
const char *const kOriginFormat = "application/x-jeffoffice-origin";
constexpr int kMostStampBytes = 256;   // (a stamp is a short name and a UUID: nothing longer is read)

// What a copy says of the program: a short digest of its name, not the name (the stamp need only tell one program from another).
QString programTag()
{
    return QString::fromLatin1(QCryptographicHash::hash(QCoreApplication::applicationName().toUtf8(), QCryptographicHash::Sha1).toHex().left(8));
}
}

PasteDefaultsSection::PasteDefaultsSection(const QVector<PasteDefaultRow> &rows, QWidget *parent) : QWidget(parent), m_rows(rows)
{
    setObjectName(QStringLiteral("pasteDefaultsSection"));
    auto *form = new QFormLayout(this);
    for (const PasteDefaultRow &row : m_rows) {
        auto *box = new QComboBox(this);
        box->setObjectName(QStringLiteral("pasteDefault%1").arg(int(row.from)));
        box->setAccessibleName(row.label);
        for (const PasteDefaultChoice &c : row.choices) box->addItem(c.text, c.value);
        box->setCurrentIndex(std::max(0, box->findData(value(row))));
        box->setProperty("shownAtOpen", box->currentData());
        form->addRow(row.label, box);
    }
}

int PasteDefaultsSection::value(const PasteDefaultRow &row)
{
    const int kept = Settings::get().value(row.key, row.fallback).toInt();
    for (const PasteDefaultChoice &c : row.choices)
        if (c.value == kept) return kept;
    return row.fallback;
}

const PasteDefaultRow *PasteDefaultsSection::rowFor(const QVector<PasteDefaultRow> &rows, PasteFrom from)
{
    for (const PasteDefaultRow &r : rows)
        if (r.from == from) return &r;
    return nullptr;
}

void PasteDefaultsSection::apply()
{
    for (const PasteDefaultRow &row : m_rows) {
        auto *box = findChild<QComboBox *>(QStringLiteral("pasteDefault%1").arg(int(row.from)));
        // (only what the person changed is kept: a setting never chosen stays as it was, and what another command of the app keeps for it still counts)
        if (box && box->currentData() != box->property("shownAtOpen")) Settings::get().setValue(row.key, box->currentData().toInt());
    }
}

void stampPasteOrigin(QMimeData *mime, const QString &documentId)
{
    if (mime) mime->setData(QLatin1String(kOriginFormat), (programTag() + QLatin1Char('\n') + documentId).toUtf8());
}

PasteFrom pasteOriginOf(const QMimeData *mime, const QString &documentId)
{
    if (!mime || !mime->hasFormat(QLatin1String(kOriginFormat))) return PasteFrom::OtherProgram;
    const QByteArray bytes = mime->data(QLatin1String(kOriginFormat));
    if (bytes.size() > kMostStampBytes) return PasteFrom::OtherProgram;
    static const QRegularExpression shape(QStringLiteral("^[0-9a-f]{8}\\n[0-9A-Fa-f-]{1,64}\\z"));
    const QString text = QString::fromUtf8(bytes);
    if (!shape.match(text).hasMatch()) return PasteFrom::OtherProgram;
    const QStringList parts = text.split(QLatin1Char('\n'));
    if (parts.size() != 2 || parts[0] != programTag()) return PasteFrom::OtherProgram;
    return !documentId.isEmpty() && parts[1] == documentId ? PasteFrom::SameDocument : PasteFrom::OtherDocument;
}

QString newPasteDocumentId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

} // namespace jp
