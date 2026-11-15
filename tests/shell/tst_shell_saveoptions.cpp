// The Save page of Options, one page for every app (libs/page/app/saveoptions.h): the default save format, how often the automatic recovery information is saved, and the default folder of
// Save As and Open.
#include "app/optionsdialog.h"
#include "app/saveoptions.h"
#include "app/settings.h"
#include "../testrunner.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QtTest>

using namespace jp;

namespace {

class ShellSaveOptionsTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void cleanup()
    {
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("save/format"), QString());
        st.setValue(QStringLiteral("save/autoRecoverMinutes"), 10);
        st.setValue(QStringLiteral("dirs/default"), QString());
        st.setValue(QStringLiteral("dirs/open"), QString());
    }

    void thePageHasTheThreeChoices()
    {
        const OptionsPage page = saveOptionsPage({{QStringLiteral("docx"), QStringLiteral("Document (*.docx)")}, {QStringLiteral("odt"), QStringLiteral("OpenDocument Text (*.odt)")}});
        QCOMPARE(page.title, QStringLiteral("Save"));
        std::unique_ptr<QWidget> w(page.build());
        auto *format = w->findChild<QComboBox *>(QStringLiteral("defaultFormat"));
        auto *minutes = w->findChild<QSpinBox *>(QStringLiteral("autoRecoverMinutes"));
        auto *folder = w->findChild<QLineEdit *>(QStringLiteral("defaultFolder"));
        QVERIFY(format && minutes && folder);
        QCOMPARE(format->count(), 2);
        QCOMPARE(format->currentData().toString(), QStringLiteral("docx"));   // (the app's first, until one is chosen)
        QCOMPARE(minutes->minimum(), 1);
        QCOMPARE(minutes->maximum(), 120);
        QCOMPARE(minutes->value(), 10);
        QVERIFY(folder->text().isEmpty());
    }

    void okSavesWhatWasChosen()
    {
        QTemporaryDir dir;
        const OptionsPage page = saveOptionsPage({{QStringLiteral("pptx"), QStringLiteral("Presentation (*.pptx)")}, {QStringLiteral("odp"), QStringLiteral("OpenDocument Presentation (*.odp)")}});
        std::unique_ptr<QWidget> w(page.build());
        w->findChild<QComboBox *>(QStringLiteral("defaultFormat"))->setCurrentIndex(1);
        w->findChild<QSpinBox *>(QStringLiteral("autoRecoverMinutes"))->setValue(25);
        w->findChild<QLineEdit *>(QStringLiteral("defaultFolder"))->setText(dir.path());
        page.apply();
        QCOMPARE(defaultSaveFormat(), QStringLiteral("odp"));
        QCOMPARE(Settings::get().autoRecoverMinutes(), 25);
        QCOMPARE(defaultFolder(), dir.path());
        // The page shows them the next time.
        std::unique_ptr<QWidget> again(page.build());
        QCOMPARE(again->findChild<QComboBox *>(QStringLiteral("defaultFormat"))->currentData().toString(), QStringLiteral("odp"));
        QCOMPARE(again->findChild<QSpinBox *>(QStringLiteral("autoRecoverMinutes"))->value(), 25);
    }

    void aFolderThatIsNotThereIsNotKept()
    {
        const OptionsPage page = saveOptionsPage({{QStringLiteral("docx"), QStringLiteral("Document (*.docx)")}});
        std::unique_ptr<QWidget> w(page.build());
        w->findChild<QLineEdit *>(QStringLiteral("defaultFolder"))->setText(QStringLiteral("/no/such/folder/here"));
        page.apply();
        QVERIFY(defaultFolder() != QStringLiteral("/no/such/folder/here"));
    }

    void aFormatTheAppDoesNotHaveIsTheFirstOne()
    {
        Settings::get().setValue(QStringLiteral("save/format"), QStringLiteral("rtf"));
        const OptionsPage page = saveOptionsPage({{QStringLiteral("docx"), QStringLiteral("Document (*.docx)")}, {QStringLiteral("odt"), QStringLiteral("OpenDocument Text (*.odt)")}});
        std::unique_ptr<QWidget> w(page.build());
        QCOMPARE(w->findChild<QComboBox *>(QStringLiteral("defaultFormat"))->currentData().toString(), QStringLiteral("docx"));
    }

    // Open starts in the default folder once there is one; until then, where the last file was opened from.
    void openStartsInTheDefaultFolderWhenThereIsOne()
    {
        QTemporaryDir dir, last;
        Settings::get().setValue(QStringLiteral("dirs/open"), last.path());
        QCOMPARE(openFolder(), last.path());
        Settings::get().setValue(QStringLiteral("dirs/default"), dir.path());
        QCOMPARE(openFolder(), dir.path());
    }

    // The extra rows an app adds go on the page and are applied with it.
    void anAppCanAddItsOwnRows()
    {
        bool built = false, applied = false;
        const OptionsPage page = saveOptionsPage({{QStringLiteral("docx"), QStringLiteral("Document (*.docx)")}}, [&built](QFormLayout *, QWidget *) { built = true; }, [&applied] { applied = true; });
        std::unique_ptr<QWidget> w(page.build());
        QVERIFY(built);
        page.apply();
        QVERIFY(applied);
    }
};

} // namespace

JP_TEST_CLASS(ShellSaveOptionsTests)
#include "tst_shell_saveoptions.moc"
