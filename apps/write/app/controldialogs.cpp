#include "app/controldialogs.h"

#include "app/plaintext.h"
#include "app/uistyle.h"
#include "app/widgets.h"

#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidgetAction>

namespace jp::write {

namespace {

const char *const kDateFormats[] = {"M/d/yyyy", "dddd, MMMM d, yyyy", "MMMM d, yyyy", "d MMMM yyyy", "yyyy-MM-dd", "M/d/yy", "d-MMM-yy", "MMMM yyyy"};

} // namespace

bool contentControlDialog(QWidget *parent, WriteDoc *doc, int start)
{
    if (!doc) return false;
    const ContentControl ctl = contentControlAt(*doc, start + 1);
    if (!ctl.valid() || ctl.start != start || !ctl.editable) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("contentControlDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::ControlDialogs", "Content Control Properties"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    auto *title = new QLineEdit(ctl.props.title, &dlg);
    title->setObjectName(QStringLiteral("title"));
    title->setMaxLength(255);
    auto *tag = new QLineEdit(ctl.props.tag, &dlg);
    tag->setObjectName(QStringLiteral("tag"));
    tag->setMaxLength(255);
    form->addRow(QCoreApplication::translate("jp::write::ControlDialogs", "Title"), title);
    form->addRow(QCoreApplication::translate("jp::write::ControlDialogs", "Tag"), tag);
    QComboBox *format = nullptr;
    QLabel *example = nullptr;
    if (ctl.props.kind == ControlKind::Date) {
        format = new QComboBox(&dlg);
        format->setObjectName(QStringLiteral("dateFormat"));
        format->setEditable(true);
        for (const char *f : kDateFormats) format->addItem(QString::fromLatin1(f));
        format->setCurrentText(ctl.props.dateFormat);
        format->lineEdit()->setMaxLength(60);
        example = new QLabel(&dlg);
        example->setObjectName(QStringLiteral("example"));
        example->setTextFormat(Qt::PlainText);
        auto showExample = [=] { example->setText(formatControlDate(QDate::currentDate(), format->currentText())); };
        QObject::connect(format, &QComboBox::editTextChanged, &dlg, showExample);
        showExample();
        form->addRow(QCoreApplication::translate("jp::write::ControlDialogs", "Date format"), format);
        form->addRow(QCoreApplication::translate("jp::write::ControlDialogs", "Today looks like"), example);
    }
    v->addLayout(form);
    QListWidget *items = nullptr;
    if (ctl.props.kind == ControlKind::DropDown) {
        auto *label = new QLabel(QCoreApplication::translate("jp::write::ControlDialogs", "Items in the list"), &dlg);
        v->addWidget(label);
        items = new QListWidget(&dlg);
        items->setObjectName(QStringLiteral("items"));
        for (const ControlItem &it : ctl.props.items) {
            auto *row = new QListWidgetItem(it.display.isEmpty() ? it.value : it.display, items);
            row->setData(Qt::UserRole, it.value.isEmpty() ? it.display : it.value);
            row->setFlags(row->flags() | Qt::ItemIsEditable);
        }
        v->addWidget(items);
        auto *buttons = new QHBoxLayout;
        auto *add = new QPushButton(QCoreApplication::translate("jp::write::ControlDialogs", "Add"), &dlg);
        add->setObjectName(QStringLiteral("addItem"));
        auto *remove = new QPushButton(QCoreApplication::translate("jp::write::ControlDialogs", "Remove"), &dlg);
        remove->setObjectName(QStringLiteral("removeItem"));
        auto *up = new QPushButton(QCoreApplication::translate("jp::write::ControlDialogs", "Move Up"), &dlg);
        up->setObjectName(QStringLiteral("itemUp"));
        auto *down = new QPushButton(QCoreApplication::translate("jp::write::ControlDialogs", "Move Down"), &dlg);
        down->setObjectName(QStringLiteral("itemDown"));
        for (QPushButton *b : {add, remove, up, down}) buttons->addWidget(b);
        buttons->addStretch();
        v->addLayout(buttons);
        QObject::connect(add, &QPushButton::clicked, &dlg, [=] {
            if (items->count() >= kMostControlItems) return;
            auto *row = new QListWidgetItem(QCoreApplication::translate("jp::write::ControlDialogs", "New item"), items);
            row->setData(Qt::UserRole, row->text());
            row->setFlags(row->flags() | Qt::ItemIsEditable);
            items->setCurrentItem(row);
            items->editItem(row);
        });
        QObject::connect(remove, &QPushButton::clicked, &dlg, [=] { delete items->takeItem(items->currentRow()); });
        auto move = [=](int by) {
            const int at = items->currentRow(), to = at + by;
            if (at < 0 || to < 0 || to >= items->count()) return;
            items->insertItem(to, items->takeItem(at));
            items->setCurrentRow(to);
        };
        QObject::connect(up, &QPushButton::clicked, &dlg, [=] { move(-1); });
        QObject::connect(down, &QPushButton::clicked, &dlg, [=] { move(1); });
    }
    auto *deleteLock = new QCheckBox(QCoreApplication::translate("jp::write::ControlDialogs", "The control can't be deleted"), &dlg);
    deleteLock->setObjectName(QStringLiteral("lockDelete"));
    deleteLock->setChecked(ctl.props.lockDelete);
    auto *contentsLock = new QCheckBox(QCoreApplication::translate("jp::write::ControlDialogs", "The contents can't be edited"), &dlg);
    contentsLock->setObjectName(QStringLiteral("lockContents"));
    contentsLock->setChecked(ctl.props.lockContents);
    v->addWidget(deleteLock);
    v->addWidget(contentsLock);
    auto *bb = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    bool applied = false;
    // (OK applies the properties and closes; a box closed any other way as accepted gets them applied once it is gone.)
    auto apply = [&]() {
        ControlProps p = ctl.props;
        p.title = title->text().trimmed();
        p.tag = tag->text().trimmed();
        p.lockDelete = deleteLock->isChecked();
        p.lockContents = contentsLock->isChecked();
        if (format) p.dateFormat = format->currentText().trimmed();
        if (items) {
            p.items.clear();
            for (int i = 0; i < items->count(); ++i) {
                const QString display = items->item(i)->text().trimmed();
                if (display.isEmpty()) continue;
                // (An item keeps the value it had, unless its words were what the value was and they changed.)
                QString value = items->item(i)->data(Qt::UserRole).toString();
                bool wasWords = false;
                for (const ControlItem &old : ctl.props.items)
                    if (old.value == value && (old.display == value || old.display.isEmpty())) wasWords = true;
                if (value.isEmpty() || wasWords || value == items->item(i)->text()) value = display;
                p.items.push_back({display, value});
            }
        }
        QString why;
        applied = setContentControlProps(*doc, start, p, &why);
        if (!applied && !why.isEmpty()) jp::plainWarning(&dlg, dlg.windowTitle(), why);
        return applied;
    };
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, [&] {
        if (apply()) dlg.accept();
    });
    if (dlg.exec() != QDialog::Accepted) return false;
    return applied || apply();
}

QMenu *controlChoicesMenu(QWidget *parent, WriteDoc *doc, int start, const std::function<void()> &done)
{
    if (!doc) return nullptr;
    const ContentControl ctl = contentControlAt(*doc, start + 1);
    if (!ctl.valid() || ctl.start != start || !ctl.editable) return nullptr;
    if (ctl.props.kind == ControlKind::DropDown) {
        auto *menu = new QMenu(parent);
        menu->setObjectName(QStringLiteral("controlChoices"));
        menu->setAttribute(Qt::WA_DeleteOnClose);
        const int shown = controlItemShown(ctl);
        for (int i = 0; i < ctl.props.items.size(); ++i) {
            const ControlItem &it = ctl.props.items[i];
            QAction *a = menu->addAction(it.display.isEmpty() ? it.value : it.display);
            a->setCheckable(true);
            a->setChecked(i == shown);
            QObject::connect(a, &QAction::triggered, menu, [=] {
                if (chooseControlItem(*doc, start, i) && done) done();
            });
        }
        if (ctl.props.items.isEmpty()) menu->addAction(QCoreApplication::translate("jp::write::ControlDialogs", "The list has no items yet"))->setEnabled(false);
        return menu;
    }
    if (ctl.props.kind == ControlKind::Date) {
        auto *menu = new QMenu(parent);
        menu->setObjectName(QStringLiteral("controlChoices"));
        menu->setAttribute(Qt::WA_DeleteOnClose);
        auto *panel = new QWidget(menu);
        auto *col = new QVBoxLayout(panel);
        col->setContentsMargins(8, 8, 8, 8);
        auto *calendar = new QCalendarWidget(panel);
        calendar->setObjectName(QStringLiteral("controlCalendar"));
        calendar->setGridVisible(false);
        calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        if (ctl.props.date.isValid()) calendar->setSelectedDate(ctl.props.date);
        col->addWidget(calendar);
        auto *row = new QHBoxLayout;
        auto *today = new QPushButton(QCoreApplication::translate("jp::write::ControlDialogs", "Today"), panel);
        today->setObjectName(QStringLiteral("today"));
        auto *clear = new QPushButton(QCoreApplication::translate("jp::write::ControlDialogs", "Clear"), panel);
        clear->setObjectName(QStringLiteral("clear"));
        row->addWidget(today);
        row->addWidget(clear);
        row->addStretch();
        col->addLayout(row);
        auto choose = [=](const QDate &date) {
            menu->close();
            if (setControlDate(*doc, start, date) && done) done();
        };
        QObject::connect(calendar, &QCalendarWidget::clicked, menu, choose);
        QObject::connect(today, &QPushButton::clicked, menu, [=] { choose(QDate::currentDate()); });
        QObject::connect(clear, &QPushButton::clicked, menu, [=] { choose(QDate()); });
        auto *wa = new QWidgetAction(menu);
        wa->setDefaultWidget(panel);
        menu->addAction(wa);
        return menu;
    }
    return nullptr;
}

} // namespace jp::write
