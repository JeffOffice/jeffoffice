#pragma once
// Remove Duplicates asks which columns say that a row repeats: a box for each column of the table or range (all on) in a scroll area, so very many columns fit.

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QScrollArea>
#include <QStringList>
#include <QVBoxLayout>

namespace jp {

// Remove Duplicates: a box for each column, all on; the rows that repeat in the ones on are taken out. For a plain range (the Data tab) there is also a box that says the first row is a header: with it
// the boxes carry the header's names and the first row is left out of the comparison; without it they are "Column A" and so on and the first row is compared like the rest.
class RemoveDuplicatesDialog : public QDialog {
public:
    // A table's columns: no header box (the table has its header).
    RemoveDuplicatesDialog(const QStringList &columns, QWidget *parent) : QDialog(parent), m_headerNames(columns), m_plainNames(columns) { build(false, true); }
    // A range's columns, by the header's names and by their letters; `hasHeader` is where the header box starts.
    RemoveDuplicatesDialog(const QStringList &headerNames, const QStringList &plainNames, bool hasHeader, QWidget *parent)
        : QDialog(parent), m_headerNames(headerNames), m_plainNames(plainNames)
    {
        build(true, hasHeader);
    }
    QList<int> chosen() const
    {
        QList<int> out;
        for (int i = 0; i < m_boxes.size(); ++i)
            if (m_boxes[i]->isChecked()) out << i;
        return out;
    }
    bool hasHeader() const { return m_header ? m_header->isChecked() : true; }
    QCheckBox *headerBox() const { return m_header; }
    // The names the boxes show now, and all of them checked or none.
    QStringList columnNames() const
    {
        QStringList out;
        for (const QCheckBox *b : m_boxes) out << QString(b->text()).replace(QLatin1String("&&"), QLatin1String("&"));
        return out;
    }
    void checkAll(bool on)
    {
        for (QCheckBox *b : m_boxes) b->setChecked(on);
    }

private:
    void build(bool withHeaderBox, bool hasHeader)
    {
        setWindowTitle(QCoreApplication::translate("jp::RemoveDuplicatesDialog", "Remove Duplicates"));
        auto *layout = new QVBoxLayout(this);
        auto *label = new QLabel(QCoreApplication::translate("jp::RemoveDuplicatesDialog", "A row is a repeat when it shows the same as an earlier row in all of these columns. The first of the rows stays."), this);
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        layout->addWidget(label);
        if (withHeaderBox) {
            m_header = new QCheckBox(QCoreApplication::translate("jp::RemoveDuplicatesDialog", "My data has headers"), this);
            m_header->setObjectName(QStringLiteral("duplicatesHeader"));
            m_header->setChecked(hasHeader);
            layout->addWidget(m_header);
        }
        // (a scroll area: a table can have very many columns)
        auto *scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        scroll->setMinimumHeight(120);
        auto *inner = new QWidget(scroll);
        auto *innerLayout = new QVBoxLayout(inner);
        for (int i = 0; i < m_plainNames.size(); ++i) {
            auto *box = new QCheckBox(inner);
            box->setChecked(true);
            m_boxes << box;
            innerLayout->addWidget(box);
        }
        innerLayout->addStretch(1);
        scroll->setWidget(inner);
        layout->addWidget(scroll, 1);
        auto *row = new QHBoxLayout;
        auto *all = new QPushButton(QCoreApplication::translate("jp::RemoveDuplicatesDialog", "Select All"), this);
        auto *none = new QPushButton(QCoreApplication::translate("jp::RemoveDuplicatesDialog", "Unselect All"), this);
        row->addWidget(all);
        row->addWidget(none);
        row->addStretch(1);
        layout->addLayout(row);
        QObject::connect(all, &QPushButton::clicked, this, [this] { checkAll(true); });
        QObject::connect(none, &QPushButton::clicked, this, [this] { checkAll(false); });
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        QObject::connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttons);
        if (m_header) QObject::connect(m_header, &QCheckBox::toggled, this, [this] { showNames(); });
        showNames();
    }
    void showNames()
    {
        const QStringList &names = hasHeader() ? m_headerNames : m_plainNames;
        for (int i = 0; i < m_boxes.size(); ++i) {
            QString text = i < names.size() ? names[i] : m_plainNames.value(i);
            m_boxes[i]->setText(text.replace(QLatin1Char('&'), QLatin1String("&&")));   // (an & in a name is not a shortcut)
        }
    }
    QStringList m_headerNames, m_plainNames;
    QCheckBox *m_header = nullptr;
    QList<QCheckBox *> m_boxes;
};

} // namespace jp
