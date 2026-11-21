// JeffWrite's pages of Options (after the shared General and Proofing pages): Save (the kind new documents are saved as, how often recovery information is kept, and the folder
// the file boxes start in) and Display (what a window starts with on the screen).

#include "app/saveoptions.h"
#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/privacy.h"
#include "app/ruler.h"
#include "app/plaintext.h"
#include "app/settings.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <memory>

namespace jp {

namespace {
// The kinds of formatting mark Options > Display can keep on, with the words for each (the bit, the name the box has, and its label).
struct MarkKind {
    int bit;
    const char *name;
    const char *label;
};
const MarkKind *markKinds(int *count)
{
    static const MarkKind kinds[] = {{tp::special::Tab, "alwaysTabs", QT_TRANSLATE_NOOP("Dialogs", "Tab characters")},
                                     {tp::special::Space, "alwaysSpaces", QT_TRANSLATE_NOOP("Dialogs", "Spaces")},
                                     {tp::special::Paragraph, "alwaysParagraphs", QT_TRANSLATE_NOOP("Dialogs", "Paragraph marks")},
                                     {tp::special::Hidden, "alwaysHidden", QT_TRANSLATE_NOOP("Dialogs", "Hidden text")},
                                     {tp::special::OptionalHyphen, "alwaysHyphens", QT_TRANSLATE_NOOP("Dialogs", "Optional hyphens")},
                                     {tp::special::Anchor, "alwaysAnchors", QT_TRANSLATE_NOOP("Dialogs", "Object anchors")}};
    *count = int(sizeof(kinds) / sizeof(kinds[0]));
    return kinds;
}
} // namespace

void WriteWindow::applyDisplayDefaults()
{
    m_view->setAlwaysMarks(Settings::get().value(QStringLiteral("view/alwaysMarks"), 0).toInt());
    const bool marks = Settings::get().value(QStringLiteral("view/marks"), false).toBool();
    const bool grid = Settings::get().value(QStringLiteral("view/gridlines"), false).toBool();
    act(QStringLiteral("view.marks"))->setChecked(marks);
    m_view->setShowMarks(marks);
    act(QStringLiteral("table.gridlines"))->setChecked(grid);
    m_view->setShowGridlines(grid);
}

void WriteWindow::applyEditingDefaults()
{
    m_view->setLinkNeedsCtrl(Settings::get().value(QStringLiteral("edit/ctrlClickLinks"), true).toBool());
    m_view->setDragTextOption(Settings::get().value(QStringLiteral("edit/dragText"), true).toBool());
    m_view->setTypingReplaces(Settings::get().value(QStringLiteral("edit/typingReplaces"), true).toBool());
    m_view->setInsertKeyOvertype(Settings::get().value(QStringLiteral("edit/insertKey"), false).toBool());
}

QVector<OptionsPage> WriteWindow::optionsPages()
{
    struct Fields {
        QCheckBox *marks = nullptr, *grid = nullptr;
        QVector<QCheckBox *> always;   // one for each of the marks of displayMarkKinds()
        QComboBox *units = nullptr;
        PasteDefaultsSection *pasteDefaults = nullptr;
        QCheckBox *ctrlLinks = nullptr, *dragText = nullptr, *replaces = nullptr, *insertKey = nullptr, *privacy = nullptr, *smart = nullptr, *embedFonts = nullptr;
    };
    auto f = std::make_shared<Fields>();
    QVector<OptionsPage> pages;
    pages << saveOptionsPage({{QStringLiteral("docx"), QCoreApplication::translate("Dialogs", "Document (*.docx)")}, {QStringLiteral("odt"), QCoreApplication::translate("Dialogs", "OpenDocument Text (*.odt)")}},
                             [f](QFormLayout *form, QWidget *page) {
                                 f->embedFonts = new QCheckBox(QCoreApplication::translate("Dialogs", "Save fonts in the file"), page);
                                 f->embedFonts->setObjectName(QStringLiteral("embedFonts"));
                                 const QString about = QCoreApplication::translate("Dialogs", "Puts the fonts that come with the program, and that the text uses, in a saved .docx file, so the text looks the same on a computer that doesn't have them. "
                                                                                              "It makes the file larger. A font whose license does not allow it is left out, and fonts every computer has are never put in.");
                                 f->embedFonts->setToolTip(jp::plainToolTip(about));
                                 f->embedFonts->setWhatsThis(jp::plainToolTip(about));
                                 f->embedFonts->setAccessibleDescription(about);
                                 f->embedFonts->setChecked(Settings::get().value(QStringLiteral("save/embedFonts"), false).toBool());
                                 form->addRow(f->embedFonts);
                                 f->privacy = privacy::makeOption(page, QStringLiteral("removePersonalOnSave"));
                                 form->addRow(f->privacy);
                             },
                             [f] {
                                 Settings::get().setValue(QStringLiteral("save/embedFonts"), f->embedFonts->isChecked());
                                 privacy::setRemoveOnSave(f->privacy->isChecked());
                             });
    pages << OptionsPage{QCoreApplication::translate("Dialogs", "Display"), [f] {
        Settings &st = Settings::get();
        auto *page = new QWidget();
        auto *form = new QFormLayout(page);
        f->marks = new QCheckBox(QCoreApplication::translate("Dialogs", "Show formatting marks (¶) when a window opens"), page);
        f->marks->setObjectName(QStringLiteral("startWithMarks"));
        f->marks->setChecked(st.value(QStringLiteral("view/marks"), false).toBool());
        form->addRow(f->marks);
        form->addRow(new QLabel(QCoreApplication::translate("Dialogs", "Always show these formatting marks on the screen"), page));
        int n = 0;
        const MarkKind *kinds = markKinds(&n);
        const int kept = st.value(QStringLiteral("view/alwaysMarks"), 0).toInt();
        f->always.clear();
        for (int i = 0; i < n; ++i) {
            auto *box = new QCheckBox(QCoreApplication::translate("Dialogs", kinds[i].label), page);
            box->setObjectName(QString::fromLatin1(kinds[i].name));
            box->setChecked((kept & kinds[i].bit) != 0);
            f->always << box;
            form->addRow(box);
        }
        f->grid = new QCheckBox(QCoreApplication::translate("Dialogs", "Show table gridlines when a window opens"), page);
        f->grid->setObjectName(QStringLiteral("startWithGridlines"));
        f->grid->setChecked(st.value(QStringLiteral("view/gridlines"), false).toBool());
        form->addRow(f->grid);
        return page;
    }, [this, f] {
        Settings::get().setValue(QStringLiteral("view/marks"), f->marks->isChecked());
        Settings::get().setValue(QStringLiteral("view/gridlines"), f->grid->isChecked());
        int n = 0, bits = 0;
        const MarkKind *kinds = markKinds(&n);
        for (int i = 0; i < n && i < f->always.size(); ++i)
            if (f->always[i]->isChecked()) bits |= kinds[i].bit;
        Settings::get().setValue(QStringLiteral("view/alwaysMarks"), bits);
        applyDisplayDefaults();
    }};
    pages << OptionsPage{QCoreApplication::translate("Dialogs", "Advanced"), [f] {
        auto *page = new QWidget();
        auto *form = new QFormLayout(page);
        f->units = new QComboBox(page);
        f->units->setObjectName(QStringLiteral("measurementUnits"));
        f->units->addItem(QCoreApplication::translate("Dialogs", "Inches"), int(Unit::Inch));
        f->units->addItem(QCoreApplication::translate("Dialogs", "Centimeters"), int(Unit::Centimeter));
        f->units->addItem(QCoreApplication::translate("Dialogs", "Millimeters"), int(Unit::Millimeter));
        f->units->addItem(QCoreApplication::translate("Dialogs", "Points"), int(Unit::Point));
        f->units->addItem(QCoreApplication::translate("Dialogs", "Picas"), int(Unit::Pica));
        f->units->setCurrentIndex(std::max(0, f->units->findData(int(Settings::get().unit()))));
        form->addRow(QCoreApplication::translate("Dialogs", "Show measurements in"), f->units);
        f->ctrlLinks = new QCheckBox(QCoreApplication::translate("Dialogs", "Use Ctrl+click to follow a link"), page);
        f->ctrlLinks->setObjectName(QStringLiteral("ctrlClickLinks"));
        f->ctrlLinks->setChecked(Settings::get().value(QStringLiteral("edit/ctrlClickLinks"), true).toBool());
        form->addRow(f->ctrlLinks);
        f->dragText = new QCheckBox(QCoreApplication::translate("Dialogs", "Allow text to be dragged and dropped"), page);
        f->dragText->setObjectName(QStringLiteral("dragText"));
        f->dragText->setChecked(Settings::get().value(QStringLiteral("edit/dragText"), true).toBool());
        form->addRow(f->dragText);
        f->replaces = new QCheckBox(QCoreApplication::translate("Dialogs", "Typing replaces the selected text"), page);
        f->replaces->setObjectName(QStringLiteral("typingReplaces"));
        f->replaces->setChecked(Settings::get().value(QStringLiteral("edit/typingReplaces"), true).toBool());
        form->addRow(f->replaces);
        f->insertKey = new QCheckBox(QCoreApplication::translate("Dialogs", "Use the Insert key to turn overtype mode on and off"), page);
        f->insertKey->setObjectName(QStringLiteral("insertKeyOvertype"));
        f->insertKey->setChecked(Settings::get().value(QStringLiteral("edit/insertKey"), false).toBool());
        form->addRow(f->insertKey);
        f->smart = new QCheckBox(QCoreApplication::translate("Dialogs", "Smart cut and paste (fix the spaces round words that are moved)"), page);
        f->smart->setObjectName(QStringLiteral("smartCutPaste"));
        f->smart->setChecked(Settings::get().value(QStringLiteral("edit/smartCutPaste"), true).toBool());
        form->addRow(f->smart);
        form->addRow(new QLabel(QCoreApplication::translate("Dialogs", "What Paste does, by where the content came from"), page));
        f->pasteDefaults = new PasteDefaultsSection(write::PageView::pasteDefaultRows(), page);
        form->addRow(f->pasteDefaults);
        return page;
    }, [this, f] {
        Settings::get().setUnit(static_cast<Unit>(f->units->currentData().toInt()));
        Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), f->smart->isChecked());
        f->pasteDefaults->apply();
        m_ruler->update();   // (the ruler's marks are in the unit chosen)
        Settings::get().setValue(QStringLiteral("edit/ctrlClickLinks"), f->ctrlLinks->isChecked());
        Settings::get().setValue(QStringLiteral("edit/dragText"), f->dragText->isChecked());
        Settings::get().setValue(QStringLiteral("edit/typingReplaces"), f->replaces->isChecked());
        Settings::get().setValue(QStringLiteral("edit/insertKey"), f->insertKey->isChecked());
        applyEditingDefaults();
    }};
    return pages;
}

} // namespace jp
