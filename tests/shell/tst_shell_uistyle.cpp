// The shared dialog style (libs/shell/app/uistyle.h): the sign of a message box by kind, the order and look of its buttons, the text in two parts, the button row of every other
// dialog, and the section heading.
#include "app/icons.h"
#include "app/theme.h"
#include "app/plaintext.h"
#include "app/uistyle.h"
#include "../testrunner.h"

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtTest>

using namespace jp;

namespace {

bool isRed(const QColor &c) { return c.alpha() > 200 && c.red() > 180 && c.green() < 110 && c.blue() < 110; }
bool isBlue(const QColor &c) { return c.alpha() > 200 && c.blue() > 160 && c.red() < 110; }

int leftOf(QWidget *w, QWidget *in) { return w->mapTo(in, QPoint(0, 0)).x(); }

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
        QVERIFY(save->property("primary").toBool());
        QVERIFY(!cancel->property("primary").toBool());
        QVERIFY(!discard->property("primary").toBool());
        for (QAbstractButton *b : {save, discard, cancel}) QVERIFY(b->icon().isNull());
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
        box.setInformativeText(QStringLiteral("More grey."));
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
        QVERIFY(shown.contains(QStringLiteral("Grey part.\nMore grey.")));
        QVERIFY(head);
        QVERIFY(head->font().weight() >= QFont::DemiBold);
        QCOMPARE(head->font().pixelSize(), 15);
        QCOMPARE(head->textFormat(), Qt::PlainText);
        QVERIFY(box.findChild<QLabel *>(QStringLiteral("jpBoxSign"))->isVisible());
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
        QVERIFY(leftOf(bb->button(QDialogButtonBox::Cancel), &dlg) < leftOf(bb->button(QDialogButtonBox::Ok), &dlg));
        QVERIFY(bb->button(QDialogButtonBox::Ok)->property("primary").toBool());
        QVERIFY(!del->property("primary").toBool());
        installButtonBoxStyle();
        QTRY_VERIFY(pb->button(QDialogButtonBox::Ok)->property("primary").toBool());
        QVERIFY(leftOf(pdel, &plain) < leftOf(pb->button(QDialogButtonBox::Cancel), &plain));
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
