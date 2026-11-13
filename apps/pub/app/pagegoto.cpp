// Go To (Ctrl+G): a page by number, or a comment by number (or the next or previous one).

#include "app/pagegoto.h"

#include "app/commentspane.h"
#include "app/mainwindow.h"
#include "app/plaintext.h"
#include "core/comments.h"

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

void showPageGoToDialog(MainWindow *window)
{
    Editor *ed = window->editor();
    if (!ed || !ed->doc() || ed->doc()->pages.isEmpty()) return;
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
        bool comment;
    };
    static const Kind all[] = {{QT_TRANSLATE_NOOP("jp::GoToDialog", "Page"), QT_TRANSLATE_NOOP("jp::GoToDialog", "Page number:"), false},
                               {QT_TRANSLATE_NOOP("jp::GoToDialog", "Comment"), QT_TRANSLATE_NOOP("jp::GoToDialog", "Comment number:"), true}};
    for (const Kind &k : all) kinds->addItem(QCoreApplication::translate("jp::GoToDialog", k.text));
    kinds->setMaximumHeight(70);
    v->addWidget(kinds);
    auto *prompt = plainLabel(new QLabel(&dlg));
    prompt->setObjectName(QStringLiteral("goToPrompt"));
    v->addWidget(prompt);
    // A page is chosen with the number box it has always had; a comment is typed.
    auto *number = new QSpinBox(&dlg);
    number->setObjectName(QStringLiteral("goToNumber"));
    const int pages = int(ed->doc()->pages.size());
    number->setRange(1, pages);
    number->setValue(std::clamp(ed->currentPage(), 0, pages - 1) + 1);
    v->addWidget(number);
    auto *what = new QLineEdit(&dlg);
    what->setObjectName(QStringLiteral("goToWhat"));
    v->addWidget(what);
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

    QObject::connect(kinds, &QListWidget::currentRowChanged, &dlg, [&](int row) {
        if (row < 0) return;
        prompt->setText(QCoreApplication::translate("jp::GoToDialog", all[row].prompt));
        problem->clear();
        what->clear();
        number->setVisible(!all[row].comment);
        what->setVisible(all[row].comment);
        prompt->setBuddy(all[row].comment ? static_cast<QWidget *>(what) : number);
        number->setAccessibleName(prompt->text());
        what->setAccessibleName(prompt->text());
        // A page has no Next or Previous: Page Down and Page Up do that.
        previous->setVisible(all[row].comment);
        next->setVisible(all[row].comment);
    });

    // Goes there; false, with the reason shown, when there is no such place.
    auto goTo = [&](int step) -> bool {
        if (!all[std::clamp(kinds->currentRow(), 0, int(std::size(all)) - 1)].comment) {
            ed->setCurrentPage(number->value() - 1);
            return true;
        }
        const QVector<comments::CommentRef> list = comments::allComments(*ed->doc());
        const int n = int(list.size());
        if (n == 0) {
            problem->setText(QCoreApplication::translate("jp::GoToDialog", "This publication has no comments."));
            return false;
        }
        if (step == 0) {
            bool ok = false;
            const int k = what->text().trimmed().toInt(&ok);
            if (!ok || k < 1) {
                problem->setText(QCoreApplication::translate("jp::GoToDialog", "Type a comment number."));
                return false;
            }
            if (k > n) {
                problem->setText(QCoreApplication::translate("jp::GoToDialog", "There are only %1 comments.").arg(n));
                return false;
            }
            problem->clear();
            window->showComments(true);
            if (list[k - 1].slide != ed->currentPage()) ed->setCurrentPage(list[k - 1].slide);
            window->commentsPane()->setCurrentComment(list[k - 1].id);
            return true;
        }
        problem->clear();
        window->showComments(true);
        window->commentsPane()->visit(step);
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
