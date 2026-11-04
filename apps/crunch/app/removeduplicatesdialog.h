#pragma once
// Remove Duplicates asks which columns say that a row repeats: a box for each column of the table or range (all on) in a scroll area, so very many columns fit.

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QList>
#include <QScrollArea>
#include <QStringList>
#include <QVBoxLayout>

namespace jp {

// Remove Duplicates: a box for each column of the table, all on; the rows that repeat in the ones on are taken out.
class RemoveDuplicatesDialog : public QDialog {
public:
    RemoveDuplicatesDialog(const QStringList &columns, QWidget *parent)
        : QDialog(parent)
    {
        setWindowTitle(QCoreApplication::translate("jp::RemoveDuplicatesDialog", "Remove Duplicates"));
        auto *layout = new QVBoxLayout(this);
        auto *label = new QLabel(QCoreApplication::translate("jp::RemoveDuplicatesDialog", "A row is a repeat when it shows the same as an earlier row in all of these columns. The first of the rows stays."), this);
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        layout->addWidget(label);
        // (a scroll area: a table can have very many columns)
        auto *scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        scroll->setMinimumHeight(120);
        auto *inner = new QWidget(scroll);
        auto *innerLayout = new QVBoxLayout(inner);
        for (int i = 0; i < columns.size(); ++i) {
            auto *box = new QCheckBox(inner);
            QString text = columns[i];
            box->setText(text.replace(QLatin1Char('&'), QLatin1String("&&")));   // (an & in a name is not a shortcut)
            box->setChecked(true);
            m_boxes << box;
            innerLayout->addWidget(box);
        }
        innerLayout->addStretch(1);
        scroll->setWidget(inner);
        layout->addWidget(scroll, 1);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        QObject::connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttons);
    }
    QList<int> chosen() const
    {
        QList<int> out;
        for (int i = 0; i < m_boxes.size(); ++i)
            if (m_boxes[i]->isChecked()) out << i;
        return out;
    }

private:
    QList<QCheckBox *> m_boxes;
};


} // namespace jp
