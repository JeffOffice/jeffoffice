#include "app/pastedefaults.h"

#include "app/settings.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QMimeData>
#include <QUuid>

namespace jp {

namespace {
const char *const kOriginFormat = "application/x-jeffoffice-origin";
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
        if (box) Settings::get().setValue(row.key, box->currentData().toInt());
    }
}

void stampPasteOrigin(QMimeData *mime, const QString &documentId)
{
    if (mime) mime->setData(QLatin1String(kOriginFormat), (QCoreApplication::applicationName() + QLatin1Char('\n') + documentId).toUtf8());
}

PasteFrom pasteOriginOf(const QMimeData *mime, const QString &documentId)
{
    if (!mime || !mime->hasFormat(QLatin1String(kOriginFormat))) return PasteFrom::OtherProgram;
    const QStringList parts = QString::fromUtf8(mime->data(QLatin1String(kOriginFormat))).split(QLatin1Char('\n'));
    if (parts.size() != 2 || parts[0] != QCoreApplication::applicationName()) return PasteFrom::OtherProgram;
    return !documentId.isEmpty() && parts[1] == documentId ? PasteFrom::SameDocument : PasteFrom::OtherDocument;
}

QString newPasteDocumentId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

} // namespace jp
