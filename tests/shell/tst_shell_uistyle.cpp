// The shared dialog style (libs/shell/app/uistyle.h): the sign of a message box by kind, the order and look of its buttons, the text in two parts, the button row of every other
// dialog, and the section heading.
#include "app/icons.h"
#include "app/theme.h"
#include "app/plaintext.h"
#include "app/uistyle.h"
#include "app/printpage.h"
#include "../testrunner.h"

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QStyle>
#include <QStyleOptionGroupBox>
#include <QtTest>
#include <cmath>

using namespace jp;

namespace {

bool isRed(const QColor &c) { return c.alpha() > 200 && c.red() > 180 && c.green() < 110 && c.blue() < 110; }
bool isBlue(const QColor &c) { return c.alpha() > 200 && c.blue() > 160 && c.red() < 110; }

int leftOf(QWidget *w, QWidget *in) { return w->mapTo(in, QPoint(0, 0)).x(); }

// The buttons of a box, left to right.
QStringList inOrder(QDialogButtonBox *bb)
{
    QList<QAbstractButton *> all = bb->buttons();
    std::sort(all.begin(), all.end(), [&](QAbstractButton *a, QAbstractButton *b) { return a->mapTo(bb, QPoint(0, 0)).x() < b->mapTo(bb, QPoint(0, 0)).x(); });
    QStringList names;
    for (QAbstractButton *b : all) names << b->text().remove(QLatin1Char('&'));
    return names;
}

class ShellUiStyleTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::applyUiTheme(1); }

    // Warning: a red triangle (its base runs the width of the sign, so a bottom corner is red); Critical: a red circle (a corner is empty); Information and Question: blue circles
    // that are told apart by their marks.
    void theSignIsDrawnByKind()
    {
        auto pixel = [](QMessageBox::Icon k, int x, int y) { return boxSign(k, 32, 1.0).toImage().pixelColor(x, y); };
        QVERIFY(isRed(pixel(QMessageBox::Warning, 16, 12)) || isRed(pixel(QMessageBox::Warning, 8, 26)));
        QVERIFY(isRed(pixel(QMessageBox::Warning, 5, 26)));
        QVERIFY(isRed(pixel(QMessageBox::Critical, 6, 16)));
        QCOMPARE(pixel(QMessageBox::Critical, 2, 28).alpha(), 0);
        QVERIFY(isBlue(pixel(QMessageBox::Information, 6, 16)));
        QVERIFY(isBlue(pixel(QMessageBox::Question, 6, 16)));
        QVERIFY(!isRed(pixel(QMessageBox::Information, 6, 16)));
        QVERIFY(boxSign(QMessageBox::Information, 32, 1.0).toImage() != boxSign(QMessageBox::Question, 32, 1.0).toImage());
        QVERIFY(boxSign(QMessageBox::Warning, 32, 1.0).toImage() != boxSign(QMessageBox::Critical, 32, 1.0).toImage());
        QCOMPARE(boxSign(QMessageBox::Warning, 32, 2.0).size(), QSize(64, 64));
    }

    // A destructive choice sits alone at the left; Cancel comes before the accent-filled default at the right; the buttons have no icons.
    void theButtonsAreInTheOrderOfTheStyle()
    {
        MessageBox box(QMessageBox::Warning, QStringLiteral("T"), QStringLiteral("Save the work? Nothing was saved."), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        box.setDefaultButton(QMessageBox::Save);
        box.show();
        QTRY_VERIFY(box.isVisible());
        QAbstractButton *save = box.button(QMessageBox::Save), *discard = box.button(QMessageBox::Discard), *cancel = box.button(QMessageBox::Cancel);
        QTRY_VERIFY(leftOf(discard, &box) < leftOf(cancel, &box));
        QVERIFY(leftOf(cancel, &box) < leftOf(save, &box));
        // alone: the gap after Don't Save is wider than the one between Cancel and Save
        QVERIFY(leftOf(cancel, &box) - (leftOf(discard, &box) + discard->width()) > leftOf(save, &box) - (leftOf(cancel, &box) + cancel->width()) + 20);
        QTRY_VERIFY(save->property("primary").toBool());
        QVERIFY(!cancel->property("primary").toBool());
        QVERIFY(!discard->property("primary").toBool());
        for (QAbstractButton *b : {save, discard, cancel}) QVERIFY2(b->icon().isNull(), qPrintable(b->text() + QStringLiteral(" has an icon")));
        QVERIFY(qobject_cast<QPushButton *>(save)->isDefault());
    }

    // A box with one choice has it at the right, accent-filled; Yes and No keep their order.
    void anOkAndAYesNoBox()
    {
        MessageBox ok(QMessageBox::Information, QStringLiteral("T"), QStringLiteral("Done."), QMessageBox::Ok);
        ok.show();
        QTRY_VERIFY(ok.isVisible());
        QTRY_VERIFY(ok.button(QMessageBox::Ok)->property("primary").toBool());
        QVERIFY(leftOf(ok.button(QMessageBox::Ok), &ok) > ok.width() / 2);
        MessageBox yn(QMessageBox::Question, QStringLiteral("T"), QStringLiteral("Go on?"), QMessageBox::Yes | QMessageBox::No);
        yn.setDefaultButton(QMessageBox::Yes);
        yn.show();
        QTRY_VERIFY(yn.isVisible());
        QTRY_VERIFY(yn.button(QMessageBox::Yes)->property("primary").toBool());
        QVERIFY(leftOf(yn.button(QMessageBox::No), &yn) < leftOf(yn.button(QMessageBox::Yes), &yn));
    }

    // The first sentence is the bold line; the rest and the informative text are the grey line under it; text() is what was given.
    void theTextIsInTwoParts()
    {
        QString lead, rest;
        MessageBox::splitText(QStringLiteral("The file is damaged. Try another copy.\nOr ask for a new one."), &lead, &rest);
        QCOMPARE(lead, QStringLiteral("The file is damaged."));
        QCOMPARE(rest, QStringLiteral("Try another copy.\nOr ask for a new one."));
        MessageBox::splitText(QStringLiteral("Just one sentence"), &lead, &rest);
        QCOMPARE(lead, QStringLiteral("Just one sentence"));
        QVERIFY(rest.isEmpty());
        MessageBox box(QMessageBox::Warning, QStringLiteral("T"), QStringLiteral("Bold part. Grey part."), QMessageBox::Ok);
        box.setTextFormat(Qt::PlainText);
        box.show();
        QTRY_VERIFY(box.findChild<QLabel *>(QStringLiteral("jpBoxSign")));
        QCOMPARE(box.text(), QStringLiteral("Bold part. Grey part."));
        QStringList shown;
        QLabel *head = nullptr;
        for (QLabel *l : box.findChildren<QLabel *>())
            if (l->isVisible() && !l->text().isEmpty()) {
                shown << l->text();
                if (l->text() == QLatin1String("Bold part.")) head = l;
            }
        QVERIFY(shown.contains(QStringLiteral("Bold part.")));
        QVERIFY(shown.contains(QStringLiteral("Grey part.")));
        QVERIFY(head);
        QVERIFY(head->font().weight() >= QFont::DemiBold);
        QCOMPARE(head->font().pixelSize(), 15);
        QCOMPARE(head->textFormat(), Qt::PlainText);
        QVERIFY(box.findChild<QLabel *>(QStringLiteral("jpBoxSign"))->isVisible());
        // a caller that gives two parts is taken at its word: the text is the bold line whole, the informative text the grey one
        MessageBox two(QMessageBox::Warning, QStringLiteral("T"), QStringLiteral("Save changes to \"a. b\"? Really."), QMessageBox::Ok);
        two.setInformativeText(QStringLiteral("Grey."));
        two.show();
        QTRY_VERIFY(two.findChild<QLabel *>(QStringLiteral("jpBoxSign")));
        QStringList shown2;
        for (QLabel *l : two.findChildren<QLabel *>())
            if (l->isVisible() && !l->text().isEmpty()) shown2 << l->text();
        QVERIFY(shown2.contains(QStringLiteral("Save changes to \"a. b\"? Really.")));
        QVERIFY(shown2.contains(QStringLiteral("Grey.")));
    }

    // A full stop inside quotes, or before a lower-case word, does not end the first sentence.
    void theFirstSentenceIsNotCutInAFileName()
    {
        QString lead, rest;
        MessageBox::splitText(QStringLiteral("Could not save \"plan. xlsx\". Try again."), &lead, &rest);
        QCOMPARE(lead, QStringLiteral("Could not save \"plan. xlsx\"."));
        QCOMPARE(rest, QStringLiteral("Try again."));
        MessageBox::splitText(QStringLiteral("Use e.g. a name. Then go on."), &lead, &rest);
        QCOMPARE(lead, QStringLiteral("Use e.g. a name."));
        QCOMPARE(rest, QStringLiteral("Then go on."));
        MessageBox::splitText(QString::fromUtf8("The file \u201Cx. y\u201D is gone. Sorry."), &lead, &rest);
        QCOMPARE(rest, QStringLiteral("Sorry."));
    }

    // The save-on-close box: red triangle, the question with the name, the grey line, Don't Save at the left, Cancel and Save at the right.
    void theSaveOnCloseBox()
    {
        std::unique_ptr<MessageBox> box(saveChangesBox(nullptr, QStringLiteral("Plan.jcrunch")));
        QCOMPARE(box->icon(), QMessageBox::Warning);
        QCOMPARE(box->text(), QStringLiteral("Save changes to \"Plan.jcrunch\"?"));
        QCOMPARE(box->text(), saveChangesText(QStringLiteral("Plan.jcrunch")));
        QCOMPARE(box->informativeText(), QStringLiteral("If you don't save, your changes since the last save are lost."));
        QCOMPARE(box->button(QMessageBox::Discard)->text(), QStringLiteral("Don't Save"));
        QCOMPARE(box->defaultButton(), box->button(QMessageBox::Save));
        box->show();
        QTRY_VERIFY(box->isVisible());
        QTRY_VERIFY(leftOf(box->button(QMessageBox::Discard), box.get()) < leftOf(box->button(QMessageBox::Cancel), box.get()));
        QVERIFY(leftOf(box->button(QMessageBox::Cancel), box.get()) < leftOf(box->button(QMessageBox::Save), box.get()));
        QVERIFY(box->button(QMessageBox::Save)->property("primary").toBool());
    }

    // Every other dialog's button row gets the same: ButtonBox does it at once, a plain QDialogButtonBox when it is shown.
    void aButtonBoxPutsADestructiveChoiceAtTheLeft()
    {
        QDialog dlg;
        auto *v = new QVBoxLayout(&dlg);
        auto *bb = new ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        QPushButton *del = bb->addDestructive(QStringLiteral("Delete"));
        v->addWidget(bb);
        QDialog plain;
        auto *pv = new QVBoxLayout(&plain);
        auto *pb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &plain);
        QPushButton *pdel = pb->addButton(QStringLiteral("Delete"), QDialogButtonBox::DestructiveRole);
        pv->addWidget(pb);
        dlg.show();
        plain.show();
        QTRY_VERIFY(leftOf(del, &dlg) < leftOf(bb->button(QDialogButtonBox::Cancel), &dlg));
        QTRY_VERIFY(leftOf(bb->button(QDialogButtonBox::Cancel), &dlg) < leftOf(bb->button(QDialogButtonBox::Ok), &dlg));
        QTRY_VERIFY(bb->button(QDialogButtonBox::Ok)->property("primary").toBool());
        QVERIFY(!del->property("primary").toBool());
        // (no install call here: applying the theme in initTestCase installed the filter, as it does in the programs)
        QTRY_VERIFY(pb->button(QDialogButtonBox::Ok)->property("primary").toBool());
        QVERIFY(leftOf(pdel, &plain) < leftOf(pb->button(QDialogButtonBox::Cancel), &plain));
    }

    // A box is deleted with a button added after it was made still holding the focus: nothing it was drawn with is gone first (Nov 16:
    // the box's own style was deleted among its children before such a button, whose focus-out then went through it: a crash).
    void aButtonBoxCanBeDeletedWhileALaterButtonHasTheFocus()
    {
        QDialog dlg;
        auto *v = new QVBoxLayout(&dlg);
        auto *bb = new ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close, &dlg);
        QPushButton *late = bb->addButton(QStringLiteral("Next"), QDialogButtonBox::ActionRole);
        v->addWidget(bb);
        // (the style a box is drawn with is not one of the box's own children, so deleting them can't free it first)
        QVERIFY(bb->findChildren<QStyle *>(Qt::FindDirectChildrenOnly).isEmpty());
        dlg.show();
        dlg.activateWindow();
        if (QTest::qWaitForWindowActive(&dlg)) {
            late->setFocus();
            QTRY_VERIFY(late->hasFocus());
        }
        delete bb;   // (a crash here before the fix, with the focus on "Next")
        QVERIFY(dlg.findChildren<QPushButton *>().isEmpty());
    }

    // A section heading is the text in capitals, small, grey; its accessible name is the text as given; a Section keeps the text it was given.
    void aSectionHeadingIsInCapitals()
    {
        QLabel *h = sectionHeading(QStringLiteral("Orientation"));
        QCOMPARE(h->text(), QStringLiteral("ORIENTATION"));
        QCOMPARE(h->accessibleName(), QStringLiteral("Orientation"));
        QCOMPARE(h->font().pixelSize(), 11);
        QCOMPARE(h->font().weight(), QFont::Medium);
        QCOMPARE(h->palette().color(QPalette::WindowText), uiMuted());
        delete h;
        Section s(QStringLiteral("Effects"));
        QCOMPARE(s.heading(), QStringLiteral("Effects"));
        QCOMPARE(s.title(), QStringLiteral("EFFECTS"));
    }

    // Only a destructive choice is at the left; every other button is at the right in reading order, Cancel just before the primary, the primary last. This is the lossy-save box: its
    // accent default is an ActionRole button, which Qt's own layouts put in the left cluster.
    void aDefaultActionButtonIsLastAndNotAlone()
    {
        MessageBox box(QMessageBox::Warning, QStringLiteral("T"), QStringLiteral("Some things are left out."), QMessageBox::NoButton);
        QPushButton *anyway = box.addButton(QStringLiteral("Save Anyway"), QMessageBox::AcceptRole);
        QPushButton *own = box.addButton(QStringLiteral("Save as Format"), QMessageBox::ActionRole);
        box.addButton(QMessageBox::Cancel);
        box.setDefaultButton(own);
        box.show();
        auto *bb = box.findChild<QDialogButtonBox *>();
        QTRY_COMPARE(inOrder(bb), (QStringList{QStringLiteral("Save Anyway"), QStringLiteral("Cancel"), QStringLiteral("Save as Format")}));
        QVERIFY(own->property("primary").toBool());
        QVERIFY(!anyway->property("primary").toBool());
    }

    // The order is the style's own, whatever the layouts of the platform do: a destructive choice, then the helpers, Cancel, and the primary. (A Windows layout would put the primary
    // before Cancel, a macOS layout the helpers at the left.)
    void theOrderIsMovedByTheStyleItself()
    {
        QDialog dlg;
        auto *v = new QVBoxLayout(&dlg);
        auto *bb = new ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, &dlg);
        bb->addButton(QStringLiteral("Reset"), QDialogButtonBox::ResetRole);
        bb->addButton(QStringLiteral("More"), QDialogButtonBox::ActionRole);
        bb->addButton(QStringLiteral("Delete all"), QDialogButtonBox::DestructiveRole);
        v->addWidget(bb);
        dlg.show();
        QTRY_COMPARE(inOrder(bb), (QStringList{QStringLiteral("Delete all"), QStringLiteral("Help"), QStringLiteral("Reset"), QStringLiteral("More"), QStringLiteral("Cancel"), QStringLiteral("OK")}));
        // the layout itself: the destructive button, then the stretch, then the rest
        auto *lay = qobject_cast<QBoxLayout *>(bb->layout());
        QVERIFY(lay);
        QCOMPARE(qobject_cast<QAbstractButton *>(lay->itemAt(0)->widget())->text(), QStringLiteral("Delete all"));
        QVERIFY(!lay->itemAt(1)->widget());
        // Tab follows the picture
        // Tab visits the buttons in the order of the picture (the widgets that take no focus are not counted)
        QTRY_VERIFY([&] {
            QStringList chain;
            QWidget *w = bb->buttons().first();
            QAbstractButton *first = nullptr;
            for (QAbstractButton *b : bb->buttons())
                if (b->text() == QLatin1String("Delete all")) first = b;
            w = first;
            for (int i = 0; i < 40 && w; ++i, w = w->nextInFocusChain())
                if (auto *b = qobject_cast<QAbstractButton *>(w); b && bb->buttons().contains(b) && !chain.contains(b->text().remove(QLatin1Char('&')))) chain << b->text().remove(QLatin1Char('&'));
            return chain == QStringList({QStringLiteral("Delete all"), QStringLiteral("Help"), QStringLiteral("Reset"), QStringLiteral("More"), QStringLiteral("Cancel"), QStringLiteral("OK")});
        }());
    }

    // A list editor's buttons (New, Change, Delete) are not destructive choices: none is alone at the left.
    void aListEditorsDeleteIsNotAlone()
    {
        QDialog dlg;
        auto *v = new QVBoxLayout(&dlg);
        auto *bb = new ButtonBox(&dlg);
        bb->addButton(QStringLiteral("New"), QDialogButtonBox::ActionRole);
        bb->addButton(QStringLiteral("Change"), QDialogButtonBox::ActionRole);
        bb->addButton(QStringLiteral("Delete"), QDialogButtonBox::ActionRole);
        bb->addButton(QDialogButtonBox::Close);
        v->addWidget(bb);
        dlg.show();
        QTRY_COMPARE(inOrder(bb), (QStringList{QStringLiteral("New"), QStringLiteral("Change"), QStringLiteral("Delete"), QStringLiteral("Close")}));
        QVERIFY(leftOf(bb->buttons().first(), &dlg) > bb->width() / 3);   // (together at the right)
    }

    // Qt lays the buttons out again when one is added and when the style changes: Don't Save stays alone at the left, for a ButtonBox and for a plain box the filter styled.
    void theOrderSurvivesALateButtonAndAStyleChange()
    {
        for (const bool ours : {true, false}) {
            QDialog dlg;
            auto *v = new QVBoxLayout(&dlg);
            QDialogButtonBox *bb = ours ? new ButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg) : new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
            bb->addButton(QStringLiteral("Don't Save"), QDialogButtonBox::DestructiveRole);
            v->addWidget(bb);
            dlg.show();
            const QStringList want{QStringLiteral("Don't Save"), QStringLiteral("Cancel"), QStringLiteral("Save")};
            QTRY_VERIFY2(inOrder(bb) == want, qPrintable((ours ? QStringLiteral("ButtonBox: ") : QStringLiteral("plain box: ")) + inOrder(bb).join(QLatin1Char(',')) + QStringLiteral(" layout:") + [&] { QStringList l; auto *lay = bb->layout(); for (int i = 0; i < lay->count(); ++i) l << (lay->itemAt(i)->widget() ? lay->itemAt(i)->widget()->property("text").toString() + QStringLiteral("@") + QString::number(lay->itemAt(i)->widget()->x()) : QStringLiteral("-")); return l.join(QLatin1Char(',')); }()));
            bb->addButton(QStringLiteral("Late"), QDialogButtonBox::HelpRole);
            QTRY_VERIFY2(inOrder(bb) == (QStringList{QStringLiteral("Don't Save"), QStringLiteral("Late"), QStringLiteral("Cancel"), QStringLiteral("Save")}), qPrintable((ours ? QStringLiteral("ButtonBox: ") : QStringLiteral("plain box: ")) + inOrder(bb).join(QLatin1Char(',')) + QStringLiteral(" layout:") + [&] { QStringList l; auto *lay = bb->layout(); for (int i = 0; i < lay->count(); ++i) l << (lay->itemAt(i)->widget() ? lay->itemAt(i)->widget()->property("text").toString() + QStringLiteral("@") + QString::number(lay->itemAt(i)->widget()->x()) : QStringLiteral("-")); return l.join(QLatin1Char(',')); }()));
            QEvent change(QEvent::StyleChange);
            QCoreApplication::sendEvent(bb, &change);
            QTRY_VERIFY2(inOrder(bb) == (QStringList{QStringLiteral("Don't Save"), QStringLiteral("Late"), QStringLiteral("Cancel"), QStringLiteral("Save")}), qPrintable((ours ? QStringLiteral("ButtonBox: ") : QStringLiteral("plain box: ")) + inOrder(bb).join(QLatin1Char(',')) + QStringLiteral(" layout:") + [&] { QStringList l; auto *lay = bb->layout(); for (int i = 0; i < lay->count(); ++i) l << (lay->itemAt(i)->widget() ? lay->itemAt(i)->widget()->property("text").toString() + QStringLiteral("@") + QString::number(lay->itemAt(i)->widget()->x()) : QStringLiteral("-")); return l.join(QLatin1Char(',')); }()));
            QAbstractButton *dontSave = nullptr;
            for (QAbstractButton *b : bb->buttons())
                if (b->text() == QLatin1String("Don't Save")) dontSave = b;
            QVERIFY(dontSave);
            QVERIFY(leftOf(bb->button(QDialogButtonBox::Cancel), &dlg) - (leftOf(dontSave, &dlg) + dontSave->width()) > 20);   // (alone: a gap after it)
        }
    }

    // The accent is on the button Enter triggers, and on none when no button is the default.
    void theAccentIsOnTheButtonEnterPresses()
    {
        for (const QMessageBox::StandardButton def : {QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton}) {
            MessageBox box(QMessageBox::Warning, QStringLiteral("T"), QStringLiteral("Go on?"), QMessageBox::Yes | QMessageBox::No);
            if (def != QMessageBox::NoButton) box.setDefaultButton(def);
            box.show();
            QTRY_VERIFY(box.isVisible());
            QAbstractButton *accented = nullptr;
            QTRY_VERIFY((accented = box.button(QMessageBox::Yes)->property("primary").toBool() ? box.button(QMessageBox::Yes) : box.button(QMessageBox::No)->property("primary").toBool() ? box.button(QMessageBox::No) : nullptr) || def == QMessageBox::NoButton);
            if (def != QMessageBox::NoButton) QCOMPARE(accented, static_cast<QAbstractButton *>(box.button(def)));
            if (def != QMessageBox::NoButton) QVERIFY(!box.button(def == QMessageBox::Yes ? QMessageBox::No : QMessageBox::Yes)->property("primary").toBool());
            // the accent is on the default button and on no other
            for (QAbstractButton *b : {box.button(QMessageBox::Yes), box.button(QMessageBox::No)})
                QTRY_COMPARE(b->property("primary").toBool(), qobject_cast<QPushButton *>(b)->isDefault());
            QTest::keyClick(&box, Qt::Key_Return);
            // Enter pressed the accented button, or, with no accent, the box's default
            if (accented) QCOMPARE(box.clickedButton(), accented);
        }
        // no button is the default (two plain actions): the accent is on none
        QDialog dlg;
        auto *v = new QVBoxLayout(&dlg);
        auto *bb = new ButtonBox(&dlg);
        QPushButton *one = bb->addButton(QStringLiteral("One"), QDialogButtonBox::ActionRole), *two = bb->addButton(QStringLiteral("Two"), QDialogButtonBox::ActionRole);
        one->setAutoDefault(false);
        two->setAutoDefault(false);
        v->addWidget(bb);
        dlg.show();
        QTest::qWait(150);
        QVERIFY(!one->isDefault() && !two->isDefault());
        QVERIFY(!one->property("primary").toBool());
        QVERIFY(!two->property("primary").toBool());
    }

    // Don't Save taking the focus does not make it the default, and it takes no accent: Enter still means Save.
    void aDestructiveButtonTakingFocusIsNotTheDefault()
    {
        std::unique_ptr<MessageBox> box(saveChangesBox(nullptr, QStringLiteral("Plan")));
        box->show();
        QTRY_VERIFY(box->isVisible());
        QAbstractButton *discard = box->button(QMessageBox::Discard), *save = box->button(QMessageBox::Save);
        QTRY_VERIFY(save->property("primary").toBool());
        discard->setFocus();
        QEvent focus(QEvent::FocusIn);
        QCoreApplication::sendEvent(discard, &focus);
        QTest::qWait(150);
        QVERIFY(!qobject_cast<QPushButton *>(discard)->isDefault());
        QVERIFY(!discard->property("primary").toBool());
        QVERIFY(qobject_cast<QPushButton *>(save)->isDefault());
        QVERIFY(save->property("primary").toBool());
        // and a late destructive button of a plain ButtonBox is no default either
        QDialog dlg;
        auto *v = new QVBoxLayout(&dlg);
        auto *bb = new ButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
        QPushButton *dont = bb->addButton(QStringLiteral("Don't Save"), QDialogButtonBox::DestructiveRole);
        v->addWidget(bb);
        dlg.show();
        dont->setFocus();
        QTest::qWait(150);
        QVERIFY(!dont->isDefault());
        QVERIFY(!dont->property("primary").toBool());
        QVERIFY(bb->button(QDialogButtonBox::Save)->property("primary").toBool());
    }

    // When the default changes hands (focus moves among the buttons), the accent follows.
    void theAccentFollowsTheDefault()
    {
        QDialog dlg;
        auto *v = new QVBoxLayout(&dlg);
        auto *bb = new ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        v->addWidget(bb);
        dlg.show();
        QPushButton *ok = bb->button(QDialogButtonBox::Ok), *cancel = bb->button(QDialogButtonBox::Cancel);
        QTRY_VERIFY(ok->property("primary").toBool());
        cancel->setDefault(true);
        ok->setDefault(false);
        QEvent focus(QEvent::FocusIn);
        QCoreApplication::sendEvent(cancel, &focus);
        QTRY_VERIFY(cancel->property("primary").toBool());
        QVERIFY(!ok->property("primary").toBool());
    }

    // A box with no button of its own gets the OK that Qt adds, styled as the others (accent-filled).
    void anAutoAddedOkIsStyled()
    {
        MessageBox box(QMessageBox::Information, QStringLiteral("T"), QStringLiteral("Done."), QMessageBox::NoButton);
        box.show();
        QTRY_VERIFY(box.isVisible());
        QTRY_VERIFY(box.button(QMessageBox::Ok));
        QTRY_VERIFY(box.button(QMessageBox::Ok)->property("primary").toBool());
    }

    // On the dark theme the mark on the sign is dark (at least 3:1 on the sign); on the light theme it is white.
    void theMarkOnTheSignIsReadableInBothThemes()
    {
        auto lum = [](const QColor &c) {
            auto f = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
            return 0.2126 * f(c.redF()) + 0.7152 * f(c.greenF()) + 0.0722 * f(c.blueF());
        };
        for (const int theme : {1, 2}) {
            jp::applyUiTheme(theme);
            for (const QMessageBox::Icon k : {QMessageBox::Critical, QMessageBox::Information, QMessageBox::Warning, QMessageBox::Question}) {
                const QImage im = boxSign(k, 32, 1.0).toImage();
                // a point of the sign's own color, and a point of the mark on it
                const QColor sign = k == QMessageBox::Warning ? im.pixelColor(16, 26) : im.pixelColor(6, 16);
                const QColor mark = k == QMessageBox::Critical ? im.pixelColor(16, 16) : k == QMessageBox::Information ? im.pixelColor(16, 19) : k == QMessageBox::Warning ? im.pixelColor(16, 15) : im.pixelColor(16, 23);
                const double a = lum(sign) + 0.05, b = lum(mark) + 0.05;
                QVERIFY2((std::max(a, b) / std::min(a, b)) >= 3.0, qPrintable(QStringLiteral("theme %1 kind %2").arg(theme).arg(int(k))));
            }
        }
        jp::applyUiTheme(1);
    }

    // The heading, the small label, and the note follow a change of theme.
    void theGreyTextFollowsTheTheme()
    {
        jp::applyUiTheme(1);
        QLabel *h = sectionHeading(QStringLiteral("Heading"));
        QLabel *n = noteLabel(QStringLiteral("Note"));
        QWidget host;
        QWidget *two = twoUp(QStringLiteral("A"), new QCheckBox(&host), QStringLiteral("B"), new QCheckBox(&host), &host);
        QLabel *small = two->findChild<QLabel *>();
        const QColor light = uiMuted();
        QCOMPARE(h->palette().color(QPalette::WindowText), light);
        jp::applyUiTheme(2);
        QVERIFY(uiMuted() != light);
        QCOMPARE(h->palette().color(QPalette::WindowText), uiMuted());
        QCOMPARE(n->palette().color(QPalette::WindowText), uiMuted());
        QCOMPARE(small->palette().color(QPalette::WindowText), uiMuted());
        jp::applyUiTheme(1);
        QCOMPARE(h->palette().color(QPalette::WindowText), light);
        delete h;
        delete n;
    }

    // A checkable section's box sits beside the heading text, not on it.
    void aCheckableSectionsBoxDoesNotCoverItsHeading()
    {
        Section s(QStringLiteral("Restrict printing and editing"));
        s.setCheckable(true);
        new QVBoxLayout(&s);
        s.resize(400, 120);
        s.show();
        QTRY_VERIFY(s.isVisible());
        QStyleOptionGroupBox opt;
        opt.initFrom(&s);
        opt.text = s.title();
        opt.lineWidth = 1;
        opt.subControls = QStyle::SC_GroupBoxCheckBox | QStyle::SC_GroupBoxLabel;
        opt.state |= QStyle::State_On;
        const QRect box = s.style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxCheckBox, &s);
        const QRect text = s.style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, &s);
        QVERIFY(box.isValid() && text.isValid());
        QVERIFY2(!box.intersects(text), qPrintable(QStringLiteral("box %1,%2 %3x%4 text %5,%6 %7x%8").arg(box.x()).arg(box.y()).arg(box.width()).arg(box.height()).arg(text.x()).arg(text.y()).arg(text.width()).arg(text.height())));
        QVERIFY(box.right() < text.left());
    }

    // The note beside a Print group's heading wraps.
    void aPrintGroupNoteWraps()
    {
        QWidget page;
        PrintColumn column(&page);
        column.group(PrintColumn::paperHeading(), QStringLiteral("(as each sheet says unless changed)"));
        QLabel *note = nullptr;
        for (QLabel *l : page.findChildren<QLabel *>())
            if (l->property("note").toBool()) note = l;
        QVERIFY(note);
        QVERIFY(note->wordWrap());
    }

    void twoUpHoldsTwoFieldsWithLabelsAbove()
    {
        QWidget host;
        auto *a = new QCheckBox(QStringLiteral("A"), &host), *b = new QCheckBox(QStringLiteral("B"), &host);
        QWidget *row = twoUp(QStringLiteral("First"), a, QStringLiteral("Second"), b, &host);
        host.resize(400, 100);
        host.show();
        QTRY_VERIFY(row->isVisible());
        QVERIFY(a->mapTo(&host, QPoint()).y() > 0);
        QVERIFY(b->mapTo(&host, QPoint()).x() > a->mapTo(&host, QPoint()).x());
        QLabel *la = nullptr;
        for (QLabel *l : row->findChildren<QLabel *>())
            if (l->text() == QLatin1String("First")) la = l;
        QVERIFY(la);
        QVERIFY(la->mapTo(&host, QPoint()).y() < a->mapTo(&host, QPoint()).y());
        QCOMPARE(la->buddy(), static_cast<QWidget *>(a));
    }
};

} // namespace

JP_TEST_CLASS(ShellUiStyleTests)
#include "tst_shell_uistyle.moc"
