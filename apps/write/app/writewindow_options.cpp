// JeffWrite's pages of Options (after the shared General and Proofing pages): Save (the kind new documents are saved as, how often recovery information is kept, and the folder
// the file boxes start in) and Display (what a window starts with on the screen).

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/settings.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <memory>

namespace jp {

QString WriteWindow::defaultFolder()
{
    const QString chosen = Settings::get().value(QStringLiteral("dirs/default")).toString();
    if (!chosen.isEmpty() && QDir(chosen).exists()) return chosen;
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void WriteWindow::applyDisplayDefaults()
{
    const bool marks = Settings::get().value(QStringLiteral("view/marks"), false).toBool();
    const bool grid = Settings::get().value(QStringLiteral("view/gridlines"), false).toBool();
    act(QStringLiteral("view.marks"))->setChecked(marks);
    m_view->setShowMarks(marks);
    act(QStringLiteral("table.gridlines"))->setChecked(grid);
    m_view->setShowGridlines(grid);
}

QVector<OptionsPage> WriteWindow::optionsPages()
{
    struct Fields {
        QSpinBox *minutes = nullptr;
        QComboBox *format = nullptr;
        QLineEdit *folder = nullptr;
        QCheckBox *marks = nullptr, *grid = nullptr;
    };
    auto f = std::make_shared<Fields>();
    QVector<OptionsPage> pages;
    pages << OptionsPage{QCoreApplication::translate("Dialogs", "Save"), [f] {
        Settings &st = Settings::get();
        auto *page = new QWidget();
        auto *form = new QFormLayout(page);
        f->format = new QComboBox(page);
        f->format->setObjectName(QStringLiteral("defaultFormat"));
        f->format->addItem(QCoreApplication::translate("Dialogs", "Document (*.docx)"), QStringLiteral("docx"));
        f->format->addItem(QCoreApplication::translate("Dialogs", "OpenDocument Text (*.odt)"), QStringLiteral("odt"));
        f->format->setCurrentIndex(std::max(0, f->format->findData(st.value(QStringLiteral("save/format"), QStringLiteral("docx")).toString())));
        form->addRow(QCoreApplication::translate("Dialogs", "Save new documents as:"), f->format);
        f->minutes = new QSpinBox(page);
        f->minutes->setObjectName(QStringLiteral("autoRecoverMinutes"));
        f->minutes->setRange(1, 120);
        f->minutes->setValue(st.autoRecoverMinutes());
        f->minutes->setSuffix(QCoreApplication::translate("Dialogs", " minutes"));
        form->addRow(QCoreApplication::translate("Dialogs", "Save automatic recovery information every:"), f->minutes);
        auto *row = new QWidget(page);
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
        return page;
    }, [f] {
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("save/format"), f->format->currentData().toString());
        st.setValue(QStringLiteral("save/autoRecoverMinutes"), f->minutes->value());
        const QString folder = f->folder->text().trimmed();
        st.setValue(QStringLiteral("dirs/default"), folder.isEmpty() || QDir(folder).exists() ? folder : st.value(QStringLiteral("dirs/default")).toString());
    }};
    pages << OptionsPage{QCoreApplication::translate("Dialogs", "Display"), [f] {
        Settings &st = Settings::get();
        auto *page = new QWidget();
        auto *form = new QFormLayout(page);
        f->marks = new QCheckBox(QCoreApplication::translate("Dialogs", "Show formatting marks (¶) when a window opens"), page);
        f->marks->setObjectName(QStringLiteral("startWithMarks"));
        f->marks->setChecked(st.value(QStringLiteral("view/marks"), false).toBool());
        form->addRow(f->marks);
        f->grid = new QCheckBox(QCoreApplication::translate("Dialogs", "Show table gridlines when a window opens"), page);
        f->grid->setObjectName(QStringLiteral("startWithGridlines"));
        f->grid->setChecked(st.value(QStringLiteral("view/gridlines"), false).toBool());
        form->addRow(f->grid);
        return page;
    }, [this, f] {
        Settings::get().setValue(QStringLiteral("view/marks"), f->marks->isChecked());
        Settings::get().setValue(QStringLiteral("view/gridlines"), f->grid->isChecked());
        applyDisplayDefaults();
    }};
    return pages;
}

} // namespace jp
