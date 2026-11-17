// Cells copied in JeffCrunch and pasted in JeffPub and JeffDeck (Editor::pasteHtmlTable of libs/page, which both use): a real copy's HTML read into a table.
#include "editwindow.h"
#include "savedok.h"

#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "grid/gridview.h"

#include "app/editor.h"
#include "app/pastetable.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QSettings>
#include <QTemporaryDir>

#include "../testrunner.h"

using namespace crunchtest;

class CrunchPasteTableTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // What JeffPub and JeffDeck paste for a JeffCrunch copy (Editor::pasteHtmlTable, the one pasting of libs/page): a table with the fonts, fill, borders, and alignment the copy carries, and the text
    // the cells show for their numbers.
    void aCopyOfStyledCellsPastesAsATableWithItsLook()
    {
        
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Total", "B1=1234.5", "A2=plain", "B2=0.25"})));
        auto run = [&](const char *id) {
            QAction *a = e.win.w.act(QString::fromLatin1(id));
            QVERIFY2(a && a->isEnabled(), id);
            a->trigger();
        };
        e.select(QStringLiteral("A1"));
        run("fmt.bold");
        run("fmt.underline");
        run("fmt.alignCenter");
        run("fmt.border.outside");
        auto *fill = e.win.w.findChild<ColorButton *>(QStringLiteral("fmt.fillColor"));
        auto *font = e.win.w.findChild<FontCombo *>(QStringLiteral("fmt.fontName"));
        QVERIFY(fill && font);
        fill->colorPicked(jp::ColorRef::rgb(QColor(0xFF, 0xFF, 0x00)));
        font->familyChosen(QStringLiteral("Archivo Black"));
        e.select(QStringLiteral("B1"));
        run("fmt.currency");
        e.select(QStringLiteral("B2"));
        run("fmt.percent");
        copyRange(e, QStringLiteral("A1:B2"));
        jp::Editor ed;
        QCOMPARE(ed.pasteHtmlTable(QApplication::clipboard()->mimeData()), jp::PastedTableResult::Ok);
        auto *t = dynamic_cast<jp::TableItem *>(ed.doc()->pages[0]->items.back().get());
        QVERIFY(t);
        QCOMPARE(t->rows, 2);
        QCOMPARE(t->cols, 2);
        auto story = [&](int r, int c) { return ed.doc()->storyDoc(t->cell(r, c).storyId); };
        QCOMPARE(story(0, 0)->toPlainText(), QStringLiteral("Total"));
        QCOMPARE(story(0, 1)->toPlainText(), e.shownAt(QStringLiteral("B1")));
        QCOMPARE(story(1, 1)->toPlainText(), e.shownAt(QStringLiteral("B2")));
        QVERIFY(e.shownAt(QStringLiteral("B2")).contains(QLatin1Char('%')));
        QCOMPARE(t->cell(0, 0).fill.color, jp::ColorRef::rgb(QColor(0xFF, 0xFF, 0x00)));
        QVERIFY(t->cell(1, 0).fill.isNone());
        QCOMPARE(t->cell(0, 0).border.left.color, jp::ColorRef::rgb(QColor(Qt::black)));
        QCOMPARE(t->cell(0, 0).border.bottom.width, 0.75);
        QVERIFY(t->cell(1, 0).border.left.isNone());
        QTextCursor cur(story(0, 0));
        cur.setPosition(1);
        QVERIFY(cur.charFormat().fontWeight() >= QFont::Bold && cur.charFormat().fontUnderline());
        QCOMPARE(cur.charFormat().fontFamilies().toStringList().value(0), QStringLiteral("Archivo Black"));
        QCOMPARE(cur.blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
        QTextCursor nc(story(0, 1));
        nc.setPosition(1);
        QVERIFY(nc.blockFormat().alignment() & Qt::AlignRight);
    }
};

JP_TEST_CLASS(CrunchPasteTableTests)
#include "tst_crunch_pastetable.moc"
