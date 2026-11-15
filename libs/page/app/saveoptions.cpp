#include "app/saveoptions.h"

#include "app/settings.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <memory>

namespace jp {

OptionsPage saveOptionsPage(const QVector<SaveFormat> &formats, const std::function<void(QFormLayout *, QWidget *)> &more, const std::function<void()> &applyMore)
{
    struct Fields {
        QPointer<QComboBox> format;
        QPointer<QSpinBox> minutes;
        QPointer<QLineEdit> folder;
    };
    auto f = std::make_shared<Fields>();
    OptionsPage page;
    page.title = QCoreApplication::translate("Dialogs", "Save");
    page.build = [f, formats, more] {
        Settings &st = Settings::get();
        auto *w = new QWidget();
        auto *form = new QFormLayout(w);
        f->format = new QComboBox(w);
        f->format->setObjectName(QStringLiteral("defaultFormat"));
        for (const SaveFormat &k : formats) f->format->addItem(k.label, k.key);
        f->format->setCurrentIndex(std::max(0, int(f->format->findData(defaultSaveFormat()))));
        f->format->setEnabled(formats.size() > 1);
        form->addRow(QCoreApplication::translate("Dialogs", "Save new documents as:"), f->format);
        f->minutes = new QSpinBox(w);
        f->minutes->setObjectName(QStringLiteral("autoRecoverMinutes"));
        f->minutes->setRange(1, 120);
        f->minutes->setValue(st.autoRecoverMinutes());
        f->minutes->setSuffix(QCoreApplication::translate("Dialogs", " minutes"));
        form->addRow(QCoreApplication::translate("Dialogs", "Save automatic recovery information every:"), f->minutes);
        auto *row = new QWidget(w);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        f->folder = new QLineEdit(st.value(QStringLiteral("dirs/default")).toString(), row);
        f->folder->setObjectName(QStringLiteral("defaultFolder"));
        f->folder->setPlaceholderText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        auto *browse = new QPushButton(QCoreApplication::translate("Dialogs", "Browse…"), row);
        QObject::connect(browse, &QPushButton::clicked, f->folder, [edit = f->folder] {
            const QString d = QFileDialog::getExistingDirectory(edit, QCoreApplication::translate("Dialogs", "Default File Location"), edit->text());
            if (!d.isEmpty()) edit->setText(d);
        });
        rowLayout->addWidget(f->folder, 1);
        rowLayout->addWidget(browse);
        form->addRow(QCoreApplication::translate("Dialogs", "Default file location:"), row);
        if (more) more(form, w);
        return w;
    };
    page.apply = [f, applyMore] {
        Settings &st = Settings::get();
        if (f->format) st.setValue(QStringLiteral("save/format"), f->format->currentData().toString());
        if (f->minutes) st.setValue(QStringLiteral("save/autoRecoverMinutes"), f->minutes->value());
        if (f->folder) {
            // (a folder that is not there is not kept)
            const QString folder = f->folder->text().trimmed();
            st.setValue(QStringLiteral("dirs/default"), folder.isEmpty() || QDir(folder).exists() ? folder : st.value(QStringLiteral("dirs/default")).toString());
        }
        if (applyMore) applyMore();
    };
    return page;
}

} // namespace jp
