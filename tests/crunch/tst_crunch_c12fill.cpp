// The Fill tab's gradient in Format Cells (round 12).
#include "app/formatdialog.h"
#include "app/widgets.h"
#include "core/color.h"
#include "core/editsession.h"
#include "core/fonts.h"

#include <QCheckBox>
#include <QComboBox>
#include <QtTest>

#include "../testrunner.h"

using namespace jp;

class CrunchFillDialogTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initCore(); }

    // Format Cells > Fill offers the gradient: a second color and a direction.
    void theFillTabMakesAGradient()
    {
        FormatCellsDialog::Input in;
        in.locale = QLocale(QLocale::English, QLocale::UnitedStates);
        FormatCellsDialog d(in);
        d.showTab(FormatCellsDialog::FillTab);
        QVERIFY(!d.result().change.fill);
        auto *on = d.findChild<QCheckBox *>(QStringLiteral("fillGradient"));
        auto *style = d.findChild<QComboBox *>(QStringLiteral("gradientStyle"));
        auto *second = d.findChild<ColorButton *>(QStringLiteral("gradientColor"));
        QVERIFY(on && style && second);
        d.findChild<ColorButton *>(QStringLiteral("fillColor"))->colorPicked(ColorRef::rgb(QColor(0x12, 0x34, 0x56)));
        second->colorPicked(ColorRef::rgb(QColor(0x00, 0x00, 0xFF)));
        on->setChecked(true);
        style->setCurrentIndex(1);   // from top to bottom
        const QString shotDir = QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR"));
        if (!shotDir.isEmpty()) {
            d.show();
            d.grab().save(shotDir + QStringLiteral("/format-cells-gradient.png"));
        }
        const FormatCellsDialog::Result r = d.result();
        QVERIFY(r.change.fill && r.change.fill->gradient);
        QCOMPARE(int(r.change.fill->stops.size()), 2);
        QCOMPARE(r.change.fill->stops[0].color.rgb, 0x123456u);
        QCOMPARE(r.change.fill->stops[1].color.rgb, 0x0000FFu);
        QCOMPARE(r.change.fill->degree, 90.0);
        style->setCurrentIndex(4);   // from the center
        QVERIFY(d.result().change.fill->path);
        on->setChecked(false);
        QVERIFY(!d.result().change.fill->gradient);
    }

};

JP_TEST_CLASS(CrunchFillDialogTests)
#include "tst_crunch_c12fill.moc"
