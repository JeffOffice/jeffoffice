// Go To (Ctrl+G): a slide by number, a section by name or number, a comment by number (or the next or previous), or an object by its name on the Selection pane.

#include "app/gotodialog.h"

#include "app/commentspane.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "core/gotoplace.h"
#include "core/sections.h"
#include "app/plaintext.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <algorithm>
#include <functional>

namespace jp {

namespace {

// A dialog that closes only when there is a place to go to (Go To, and an accept from code, run `tryAccept`; a refusal leaves it open, with the reason shown).
class GoToDialogBox : public QDialog {
public:
    using QDialog::QDialog;
    std::function<bool()> tryAccept;
    void accept() override
    {
        if (!tryAccept || tryAccept()) QDialog::accept();
    }
};

}

void showGoToDialog(DeckWindow *window)
{
    Editor *ed = window->editor();
    if (!ed || !ed->doc() || ed->doc()->pages.empty()) return;
    using deck::GoToKind;
    GoToDialogBox dlg(window);
    dlg.setObjectName(QStringLiteral("goToDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::GoToDialog", "Go To"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::GoToDialog", "Go to what:"), &dlg)));
    auto *kinds = new QListWidget(&dlg);
    kinds->setObjectName(QStringLiteral("goToKinds"));
    kinds->setAccessibleName(QCoreApplication::translate("jp::GoToDialog", "Go to what"));
    struct Kind {
        const char *text;
        const char *prompt;
        GoToKind kind;
    };
    static const Kind all[] = {{QT_TRANSLATE_NOOP("jp::GoToDialog", "Slide"), QT_TRANSLATE_NOOP("jp::GoToDialog", "Slide number:"), GoToKind::Slide},
                               {QT_TRANSLATE_NOOP("jp::GoToDialog", "Section"), QT_TRANSLATE_NOOP("jp::GoToDialog", "Section name or number:"), GoToKind::Section},
                               {QT_TRANSLATE_NOOP("jp::GoToDialog", "Comment"), QT_TRANSLATE_NOOP("jp::GoToDialog", "Comment number:"), GoToKind::Comment},
                               {QT_TRANSLATE_NOOP("jp::GoToDialog", "Object"), QT_TRANSLATE_NOOP("jp::GoToDialog", "Object name, as the Selection pane shows it:"), GoToKind::Object}};
    for (const Kind &k : all) kinds->addItem(QCoreApplication::translate("jp::GoToDialog", k.text));
    kinds->setMaximumHeight(110);
    v->addWidget(kinds);
    auto *prompt = plainLabel(new QLabel(&dlg));
    prompt->setObjectName(QStringLiteral("goToPrompt"));
    v->addWidget(prompt);
    // A slide is chosen with the number box it has always had; the others are typed.
    auto *number = new QSpinBox(&dlg);
    number->setObjectName(QStringLiteral("goToNumber"));
    const int slides = int(ed->doc()->pages.size());
    number->setRange(1, slides);
    number->setValue(std::clamp(ed->currentPage(), 0, slides - 1) + 1);
    prompt->setBuddy(number);
    v->addWidget(number);
    auto *what = new QLineEdit(&dlg);
    what->setObjectName(QStringLiteral("goToWhat"));
    v->addWidget(what);
    auto *choices = new QListWidget(&dlg);
    choices->setObjectName(QStringLiteral("goToChoices"));
    choices->setAccessibleName(QCoreApplication::translate("jp::GoToDialog", "Choices"));
    choices->setMaximumHeight(110);
    v->addWidget(choices);
    auto *problem = plainLabel(new QLabel(&dlg));
    problem->setObjectName(QStringLiteral("goToProblem"));
    problem->setStyleSheet(QStringLiteral("color: #b00020"));
    problem->setWordWrap(true);
    v->addWidget(problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close, &dlg);
    QPushButton *go = buttons->button(QDialogButtonBox::Ok);
    go->setText(QCoreApplication::translate("jp::GoToDialog", "Go To"));
    go->setObjectName(QStringLiteral("goToGo"));
    QPushButton *previous = buttons->addButton(QCoreApplication::translate("jp::GoToDialog", "Previous"), QDialogButtonBox::ActionRole);
    previous->setObjectName(QStringLiteral("goToPrevious"));
    previous->setAutoDefault(false);
    QPushButton *next = buttons->addButton(QCoreApplication::translate("jp::GoToDialog", "Next"), QDialogButtonBox::ActionRole);
    next->setObjectName(QStringLiteral("goToNext"));
    next->setAutoDefault(false);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto kindNow = [&] { return all[std::clamp(kinds->currentRow(), 0, int(std::size(all)) - 1)].kind; };
    QObject::connect(kinds, &QListWidget::currentRowChanged, &dlg, [&](int row) {
        if (row < 0) return;
        prompt->setText(QCoreApplication::translate("jp::GoToDialog", all[row].prompt));
        problem->clear();
        choices->clear();
        what->clear();
        const bool slide = all[row].kind == GoToKind::Slide;
        number->setVisible(slide);
        what->setVisible(!slide);
        prompt->setBuddy(slide ? static_cast<QWidget *>(number) : what);
        what->setAccessibleName(QCoreApplication::translate("jp::GoToDialog", all[row].prompt));
        if (all[row].kind == GoToKind::Section)
            for (const deck::Section &s : deck::sectionsOf(*ed->doc())) choices->addItem(s.name);
        choices->setVisible(choices->count() > 0);
    });
    QObject::connect(choices, &QListWidget::currentTextChanged, &dlg, [&](const QString &text) {
        if (!text.isEmpty()) what->setText(text);
    });
    QObject::connect(choices, &QListWidget::itemDoubleClicked, &dlg, [&] { go->click(); });

    // Finds the place (one pass over the presentation) and goes there; false, with the reason shown, when there is none.
    auto goTo = [&](int step) -> bool {
        const Document *d = ed->doc();
        deck::GoToFrom from;
        from.slide = ed->currentPage();
        from.commentId = window->commentsPane()->currentComment();
        if (!ed->selection().isEmpty()) from.itemId = ed->selection().first();
        const GoToKind kind = kindNow();
        const QString text = kind == GoToKind::Slide ? QString::number(number->value()) : what->text();
        QString why;
        const deck::GoToPlace place = deck::goToPlace(*d, kind, text, step, from, &why);
        if (place.slide < 0) {
            problem->setText(why);
            return false;
        }
        problem->clear();
        ed->endTextEdit();
        if (place.slide != ed->currentPage()) ed->setCurrentPage(place.slide);
        if (!place.commentId.isEmpty()) {
            window->showComments(true);
            window->commentsPane()->setCurrentComment(place.commentId);
        }
        if (!place.itemId.isEmpty()) ed->select(place.itemId);
        if (kind == GoToKind::Slide) number->setValue(place.slide + 1);
        return true;
    };
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    dlg.tryAccept = [&] { return goTo(0); };
    QObject::connect(previous, &QPushButton::clicked, &dlg, [&] { goTo(-1); });
    QObject::connect(next, &QPushButton::clicked, &dlg, [&] { goTo(1); });

    kinds->setCurrentRow(0);
    number->setFocus();
    number->selectAll();
    dlg.exec();
}

} // namespace jp
