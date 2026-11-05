// Go To (Ctrl+G): a page, section, line, bookmark, footnote, endnote, comment, field, table, graphic, equation, or heading, by number or name; the caret goes there (the work of finding the place is core/gotoops.h).

#include "app/writedialogs.h"

#include "app/pageview.h"
#include "core/gotoops.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>
#include <functional>

namespace jp::write {

namespace {

// A dialog that closes only when there is a place to go to (its Go To, and an accept from code, run `tryAccept`; a refusal leaves it open, with the reason shown).
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

bool goToDialog(QWidget *parent, WriteDoc *doc, PageView *view)
{
    if (!doc || !view) return false;
    GoToDialogBox dlg(parent);
    dlg.setObjectName(QStringLiteral("goToDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::GoToDialog", "Go To"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(new QLabel(QCoreApplication::translate("jp::write::GoToDialog", "Go to what:"), &dlg));
    auto *kinds = new QListWidget(&dlg);
    kinds->setObjectName(QStringLiteral("goToKinds"));
    kinds->setAccessibleName(QCoreApplication::translate("jp::write::GoToDialog", "Go to what"));
    struct Kind {
        const char *text;
        const char *prompt;
        GoToKind kind;
    };
    static const Kind all[] = {{QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Page"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Page number (+ or - for a page from here):"), GoToKind::Page},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Section"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Section number (+ or - for a section from here):"), GoToKind::Section},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Line"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Line number:"), GoToKind::Line},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Bookmark"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Bookmark name:"), GoToKind::Bookmark},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Footnote"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Footnote number (+ or - for a footnote from here):"), GoToKind::Footnote},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Endnote"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Endnote number (+ or - for an endnote from here):"), GoToKind::Endnote},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Comment"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Comment number (+ or - for a comment from here):"), GoToKind::Comment},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Field"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Field number (+ or - for a field from here):"), GoToKind::Field},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Table"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Table number (+ or - for a table from here):"), GoToKind::Table},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Graphic"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Graphic number (+ or - for a graphic from here):"), GoToKind::Graphic},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Equation"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Equation number (+ or - for an equation from here):"), GoToKind::Equation},
                               {QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Heading"), QT_TRANSLATE_NOOP("jp::write::GoToDialog", "Heading number, or part of its text:"), GoToKind::Heading}};
    for (const Kind &k : all) kinds->addItem(QCoreApplication::translate("jp::write::GoToDialog", k.text));
    kinds->setMaximumHeight(170);
    v->addWidget(kinds);
    auto *prompt = new QLabel(&dlg);
    prompt->setObjectName(QStringLiteral("goToPrompt"));
    v->addWidget(prompt);
    auto *what = new QLineEdit(&dlg);
    what->setObjectName(QStringLiteral("goToWhat"));
    v->addWidget(what);
    auto *choices = new QListWidget(&dlg);
    choices->setObjectName(QStringLiteral("goToChoices"));
    choices->setAccessibleName(QCoreApplication::translate("jp::write::GoToDialog", "Choices"));
    v->addWidget(choices);
    auto *problem = new QLabel(&dlg);
    problem->setObjectName(QStringLiteral("goToProblem"));
    problem->setStyleSheet(QStringLiteral("color: #b00020"));
    v->addWidget(problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText(QCoreApplication::translate("jp::write::GoToDialog", "Go To"));
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    auto kindNow = [&] { return all[std::clamp(kinds->currentRow(), 0, int(std::size(all)) - 1)].kind; };
    QObject::connect(kinds, &QListWidget::currentRowChanged, &dlg, [&](int row) {
        if (row < 0) return;
        prompt->setText(QCoreApplication::translate("jp::write::GoToDialog", all[row].prompt));
        problem->clear();
        choices->clear();
        what->clear();
        for (const GoToItem &item : goToItems(*doc, kindNow())) choices->addItem(item.label);
        choices->setVisible(choices->count() > 0);
    });
    QObject::connect(choices, &QListWidget::currentTextChanged, &dlg, [&](const QString &text) {
        if (!text.isEmpty()) what->setText(text);
    });
    QObject::connect(choices, &QListWidget::itemDoubleClicked, &dlg, [&] { buttons->button(QDialogButtonBox::Ok)->click(); });
    bool done = false;
    dlg.tryAccept = [&]() -> bool {
        QString why;
        const int at = goToPosition(*doc, kindNow(), what->text(), view->cursor().position(), &why);
        if (at < 0) {
            problem->setText(why);
            return false;
        }
        QTextCursor c = view->cursor();
        c.setPosition(at);
        view->showCursor(c);
        done = true;
        return true;
    };
    kinds->setCurrentRow(0);
    what->setFocus();
    dlg.exec();
    return done;
}

} // namespace jp::write
