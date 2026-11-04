// The window of a mail merge's recipients that every app with a mail merge shares: the table of the list (a row each, checked to be included), a filter, new and deleted entries, columns,
// finding repeats, and checking the addresses.

#include "app/mergedialogs.h"

#include "app/filedialogs.h"
#include "app/plaintext.h"
#include "core/mergesource.h"

#include <QBrush>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHash>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QTableWidget>
#include <QVBoxLayout>

namespace jp {

namespace {

struct Dlg {
    QDialog d;
    QVBoxLayout *v;
    QDialogButtonBox *bb;
    explicit Dlg(QWidget *p, const QString &title, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel) : d(p)
    {
        d.setWindowTitle(title);
        v = new QVBoxLayout(&d);
        bb = new QDialogButtonBox(buttons, &d);
        QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    }
    bool exec()
    {
        v->addWidget(bb);
        return d.exec() == QDialog::Accepted;
    }
};

} // namespace

// ---------------- Mail merge recipients ----------------
bool editRecipientList(QWidget *p, MergeSource &m, bool typeNew)
{
    if (typeNew && m.isEmpty()) {
        m.fields = {"Title", "First Name", "Last Name", "Company Name", "Address Line 1", "Address Line 2", "City", "State", "ZIP Code", "Country or Region", "Home Phone", "Work Phone", "Email Address"};
        m.rows << QStringList(m.fields.size());
        m.include << true;
    }
    if (m.isEmpty()) {
        jp::plainInformation(p, QCoreApplication::translate("Dialogs", "Mail Merge Recipients"), QCoreApplication::translate("Dialogs", "Select a recipient list first (Mailings > Select Recipients)."));
        return false;
    }
    Dlg dlg(p, typeNew ? QCoreApplication::translate("Dialogs", "New Address List") : QCoreApplication::translate("Dialogs", "Mail Merge Recipients"));
    dlg.d.resize(900, 520);
    auto *search = new QLineEdit(&dlg.d);
    search->setPlaceholderText(QCoreApplication::translate("Dialogs", "Filter recipients"));
    dlg.v->addWidget(search);
    auto *table = new QTableWidget(m.rows.size(), m.fields.size() + 1, &dlg.d);
    QStringList headers{QString()};
    headers << m.fields;
    table->setHorizontalHeaderLabels(headers);
    for (int r = 0; r < m.rows.size(); ++r) {
        auto *check = new QTableWidgetItem();
        check->setCheckState(m.include.value(r, true) ? Qt::Checked : Qt::Unchecked);
        table->setItem(r, 0, check);
        for (int c = 0; c < m.fields.size(); ++c) table->setItem(r, c + 1, new QTableWidgetItem(m.rows[r].value(c)));
    }
    table->setSortingEnabled(true);
    dlg.v->addWidget(table, 1);
    auto *row = new QHBoxLayout();
    auto *add = new QPushButton(QCoreApplication::translate("Dialogs", "New Entry"), &dlg.d);
    auto *del = new QPushButton(QCoreApplication::translate("Dialogs", "Delete Entry"), &dlg.d);
    auto *addField = new QPushButton(QCoreApplication::translate("Dialogs", "Customize Columns…"), &dlg.d);
    auto *dups = new QPushButton(QCoreApplication::translate("Dialogs", "Find Duplicates"), &dlg.d);
    auto *validate = new QPushButton(QCoreApplication::translate("Dialogs", "Validate Addresses"), &dlg.d);
    row->addWidget(add);
    row->addWidget(del);
    row->addWidget(addField);
    row->addWidget(dups);
    row->addWidget(validate);
    row->addStretch(1);
    dlg.v->addLayout(row);
    QObject::connect(search, &QLineEdit::textChanged, &dlg.d, [=](const QString &q) {
        for (int r = 0; r < table->rowCount(); ++r) {
            bool match = q.isEmpty();
            for (int c = 1; c < table->columnCount() && !match; ++c)
                if (table->item(r, c) && table->item(r, c)->text().contains(q, Qt::CaseInsensitive)) match = true;
            table->setRowHidden(r, !match);
        }
    });
    QObject::connect(add, &QPushButton::clicked, &dlg.d, [=] {
        table->setSortingEnabled(false);
        const int r = table->rowCount();
        table->insertRow(r);
        auto *check = new QTableWidgetItem();
        check->setCheckState(Qt::Checked);
        table->setItem(r, 0, check);
        for (int c = 1; c < table->columnCount(); ++c) table->setItem(r, c, new QTableWidgetItem());
        table->setCurrentCell(r, 1);
        table->editItem(table->item(r, 1));
        table->setSortingEnabled(true);
    });
    QObject::connect(del, &QPushButton::clicked, &dlg.d, [=] { if (table->currentRow() >= 0) table->removeRow(table->currentRow()); });
    QObject::connect(addField, &QPushButton::clicked, &dlg.d, [=] {
        bool ok = false;
        const QString n = QInputDialog::getText(table, QCoreApplication::translate("Dialogs", "Add Field"), QCoreApplication::translate("Dialogs", "Field name:"), QLineEdit::Normal, QString(), &ok);
        if (!ok || n.trimmed().isEmpty()) return;
        const int c = table->columnCount();
        table->insertColumn(c);
        table->setHorizontalHeaderItem(c, new QTableWidgetItem(n.trimmed()));
    });
    QObject::connect(dups, &QPushButton::clicked, &dlg.d, [=] {
        QHash<QString, int> seen;
        int n = 0;
        for (int r = 0; r < table->rowCount(); ++r) {
            QString key;
            for (int c = 1; c < table->columnCount(); ++c) key += (table->item(r, c) ? table->item(r, c)->text().trimmed().toLower() : QString()) + '|';
            if (seen.contains(key)) { table->item(r, 0)->setCheckState(Qt::Unchecked); ++n; }
            else seen.insert(key, r);
        }
        jp::plainInformation(table, QCoreApplication::translate("Dialogs", "Find Duplicates"), QCoreApplication::translate("Dialogs", "Unchecked %1 duplicate entries.").arg(n));
    });
    QObject::connect(validate, &QPushButton::clicked, &dlg.d, [=] {
        int bad = 0;
        int zipCol = -1, addrCol = -1;
        for (int c = 1; c < table->columnCount(); ++c) {
            const QString h = table->horizontalHeaderItem(c)->text().toLower();
            if (h.contains("zip") || h.contains("postal")) zipCol = c;
            if (h.contains("address") && addrCol < 0) addrCol = c;
        }
        for (int r = 0; r < table->rowCount(); ++r) {
            const QString zip = zipCol > 0 && table->item(r, zipCol) ? table->item(r, zipCol)->text().trimmed() : QString();
            const QString addr = addrCol > 0 && table->item(r, addrCol) ? table->item(r, addrCol)->text().trimmed() : QString();
            const bool ok = !addr.isEmpty() && (zip.isEmpty() || QRegularExpression("^\\d{5}(-\\d{4})?$").match(zip).hasMatch() || zip.size() >= 3);
            for (int c = 1; c < table->columnCount(); ++c)
                if (table->item(r, c)) table->item(r, c)->setBackground(ok ? QBrush() : QBrush(QColor(255, 220, 200)));
            bad += !ok;
        }
        jp::plainInformation(table, QCoreApplication::translate("Dialogs", "Validate Addresses"), bad ? QCoreApplication::translate("Dialogs", "%1 entries have a missing address or a ZIP code that doesn't look right (highlighted).").arg(bad)
                                                                                  : QCoreApplication::translate("Dialogs", "All entries have an address and a well-formed ZIP code."));
    });
    if (!dlg.exec()) return false;
    MergeSource out;
    out.path = m.path;
    out.pictureField = m.pictureField;
    for (int c = 1; c < table->columnCount(); ++c) out.fields << table->horizontalHeaderItem(c)->text();
    for (int r = 0; r < table->rowCount(); ++r) {
        QStringList rowv;
        bool any = false;
        for (int c = 1; c < table->columnCount(); ++c) {
            const QString v = table->item(r, c) ? table->item(r, c)->text() : QString();
            any |= !v.isEmpty();
            rowv << v;
        }
        if (!any) continue;
        out.rows << rowv;
        out.include << (table->item(r, 0) && table->item(r, 0)->checkState() == Qt::Checked);
    }
    if (typeNew) {
        const QString path = askSavePath(p, QCoreApplication::translate("Dialogs", "Save Address List"), QStringLiteral("Recipients.csv"), QCoreApplication::translate("Dialogs", "CSV (*.csv)"));
        if (!path.isEmpty()) { saveMergeCsv(out, path); out.path = path; }
    }
    m = out;
    return true;
}

} // namespace jp
