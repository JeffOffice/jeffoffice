// The command search: the matching (case, accents, word starts, one wrong letter, extra words, translated and English names), what it lists from a ribbon (menus, contextual tabs, greyed commands,
// the File screen, Options), Enter running a command or opening its menu, Alt+Q and Esc, the list of recent commands, and the row that searches Help. A small ribbon built from a file stands for
// an app's (each app's own test checks that its window has the box and that its words name commands it has).
#include "app/appwindow.h"
#include "app/commandsearch.h"
#include "app/filescreen.h"
#include "app/help.h"
#include "app/i18n.h"
#include "app/keytips.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "../testrunner.h"

#include <QAction>
#include <QListWidget>
#include <QMenu>
#include <QSignalSpy>
#include <QThread>
#include <QTimer>
#include <QTemporaryDir>
#include <QToolButton>
#include <QtTest>

using namespace jp;
using namespace jp::cmdsearch;

namespace {

const char *const kFile = R"({
  "fileKeytip": "F",
  "tabs": [
    {"name": "Home", "keytip": "H", "groups": [
      {"name": "Clipboard", "keytip": "ZC", "items": [ {"large": "edit.paste", "keytip": "V"}, {"small": "edit.copy", "keytip": "C"} ]},
      {"name": "Font", "keytip": "ZF", "items": [ {"small": "format.bold", "keytip": "B"}, {"small": "format.off", "keytip": "O"}, {"small": "format.cafe", "keytip": "E"} ]},
      {"name": "Paragraph", "keytip": "ZG", "items": [ {"large": "para.spacing", "keytip": "S"} ]},
      {"name": "Styles", "keytip": "ZY", "items": [ {"widget": "styles", "keytip": "L"} ]} ]},
    {"name": "Layout", "keytip": "P", "groups": [
      {"name": "Page Setup", "keytip": "ZP", "items": [
        {"large": {"id": "ribbon.size", "text": "Size"}, "menu": ["layout.a4", "layout.letter"], "keytip": "SZ"},
        {"large": {"id": "ribbon.margins", "text": "Margins"}, "menu": ["layout.narrow"], "keytip": "M"} ]} ]},
    {"name": "Sheet", "keytip": "J", "groups": [
      {"name": "Cells", "keytip": "ZL", "items": [ {"small": "cells.deleteRows", "keytip": "DR"} ]} ]},
    {"name": "Chart Format", "keytip": "JC", "context": "Chart Tools", "color": "chart", "groups": [
      {"name": "Styles", "keytip": "ZS", "items": [ {"large": "chart.style", "keytip": "S"}, {"small": "edit.copy", "keytip": "C"} ]} ]}
  ]})";

const char *const kWords = R"({
  "commands": { "ribbon.size": ["page size", "paper size"], "para.spacing": ["line spacing", "double space"] },
  "file": { "print": ["printer"] },
  "options": { "Save": ["autosave"] } })";

class CmdWindow : public AppWindow {
public:
    explicit CmdWindow(bool withHelp = true, int galleryItems = 2)
    {
        auto add = [this](const char *id, const QString &text) {
            auto *a = new QAction(text, this);
            a->setObjectName(QString::fromLatin1(id));
            a->setToolTip(text + QStringLiteral(" tip"));
            QObject::connect(a, &QAction::triggered, this, [this, id] { ran << QString::fromLatin1(id); });
            m_actions.insert(a->objectName(), a);
            return a;
        };
        add("edit.paste", QStringLiteral("Paste"));
        add("edit.copy", QStringLiteral("Copy"));
        add("format.bold", QStringLiteral("Bold"));
        add("format.off", QStringLiteral("Strike"))->setEnabled(false);
        add("format.cafe", QStringLiteral("Café Menu"));
        add("para.spacing", QStringLiteral("Line and Paragraph Spacing"));
        add("cells.deleteRows", QStringLiteral("Delete Sheet Rows"));
        add("chart.style", QStringLiteral("Chart Style"));
        add("layout.a4", QStringLiteral("A4 Paper"));
        add("layout.letter", QStringLiteral("Letter Paper"));
        add("layout.narrow", QStringLiteral("Narrow Margins"));
        if (withHelp) add("help.show", QStringLiteral("Help"));
        add("file.options", QStringLiteral("Options"));
        QObject::connect(m_actions.value(QStringLiteral("file.options")), &QAction::triggered, this, [this] { optionsAsked = m_optionsPage; });
        m_ribbon = new Ribbon(this);
        setMenuWidget(m_ribbon);
        RibbonParts parts;
        parts.action = [this](const QString &id) { return act(id); };
        parts.widgets.insert(QStringLiteral("styles"), [this, galleryItems]() -> QWidget * {
            auto *g = new Gallery(QSize(40, 30), 3);
            g->setItemsProvider([galleryItems] {
                QVector<GalleryItem> items;
                if (galleryItems == 2) items << GalleryItem{QStringLiteral("h1"), QStringLiteral("Heading 1"), QIcon(), QString()} << GalleryItem{QStringLiteral("title"), QStringLiteral("Title"), QIcon(), QString()}
                                                  << GalleryItem{QStringLiteral("raw"), QStringLiteral("<b>Raw</b> & co"), QIcon(), QString()};
                else
                    for (int i = 0; i < galleryItems; ++i) items << GalleryItem{QString::number(i), QStringLiteral("Style %1").arg(i), QIcon(), QString()};
                return items;
            });
            connect(g, &Gallery::activated, this, [this](const QString &id) { ran << QStringLiteral("gallery:") + id; });
            return g;
        });
        parts.colors.insert(QStringLiteral("chart"), QColor(0x33, 0x66, 0x99));
        QString err;
        buildRibbon(m_ribbon, QByteArray(kFile), parts, this, &err);
        Q_ASSERT(err.isEmpty());
        m_screen = new FileScreen(this);
        m_screen->addSection(QStringLiteral("Start"));
        m_screen->addPage(QStringLiteral("print"), QStringLiteral("printer"), QStringLiteral("Print"));
        m_screen->addCommand(QStringLiteral("save"), QStringLiteral("save"), QStringLiteral("Save"), [this] { ran << QStringLiteral("file:save"); });
        m_screen->hide();
        m_words = new QTemporaryDir;
        QFile f(m_words->filePath(QStringLiteral("words.json")));
        f.open(QIODevice::WriteOnly);
        f.write(kWords);
        f.close();
        m_box = new CommandSearchBox(this, f.fileName());
        connect(m_ribbon, &Ribbon::leaveRequested, this, [this] { ++left; });
        resize(1100, 500);
    }
    ~CmdWindow() override { delete m_words; }
    QAction *act(const QString &id) const override { return m_actions.value(id); }
    QStringList actionIds() const override
    {
        QStringList ids = m_actions.keys();
        ids.sort();
        return ids;
    }
    Ribbon *ribbon() const override { return m_ribbon; }
    FileScreen *fileScreen() const override { return m_screen; }
    void showFileScreenPage(const QString &key) override { ran << QStringLiteral("file-page:") + key; }
    QStringList optionsPageTitles() const override { return {QStringLiteral("Save")}; }
    CommandSearchBox *box() const { return m_box; }
    QStringList ran;
    QString optionsAsked;
    int left = 0;

private:
    QHash<QString, QAction *> m_actions;
    Ribbon *m_ribbon = nullptr;
    FileScreen *m_screen = nullptr;
    CommandSearchBox *m_box = nullptr;
    QTemporaryDir *m_words = nullptr;
};

Entry entry(const QString &name, const QString &where = QString(), const QStringList &extra = {}, const QString &tip = QString())
{
    Entry e;
    e.name = name;
    e.where = where;
    e.extra = extra;
    e.tip = tip;
    return e;
}

// The names the box lists for what is typed (headings and the Help row left out).
QStringList listed(CommandSearchBox *box, const QString &text)
{
    box->setText(text);
    QStringList out;
    for (const Entry &e : box->shownResults()) out << e.name;
    return out;
}

int rowOf(CommandSearchBox *box, const QString &name)
{
    for (int i = 0; i < box->list()->count(); ++i)
        if (box->list()->item(i)->text().section(QLatin1Char('\n'), 0, 0) == name) return i;
    return -1;
}

// How many of the ribbon's tabs that are shown are folded under the chevron.
// The key that puts the cursor in the box, as the app binds it on this platform (Alt+Q; Control+Option+Q on a Mac): one place, the app's own.
void pressSearchKey(QWidget *w)
{
    const QKeyCombination key = CommandSearchBox::shortcut()[0];
    QTest::keyClick(w, key.key(), key.keyboardModifiers());
}

int foldedTabs(const Ribbon *r)
{
    int n = 0;
    for (int i = 0; i < r->tabCount(); ++i)
        if (r->tabVisible(i) && r->tabButton(i) && !r->tabButton(i)->isVisible()) ++n;
    return n;
}

void clearRecent() { Settings::get().setValue(CommandSearchBox::recentKey(), QStringList()); }

} // namespace

class ShellCommandSearchTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void init() { clearRecent(); }

    void matchingIgnoresCaseAndAccents()
    {
        QVERIFY(score(entry(QStringLiteral("Café Menu")), QStringLiteral("CAFE")) >= 0);
        QVERIFY(score(entry(QStringLiteral("Cafe Menu")), QStringLiteral("café")) >= 0);
        QVERIFY(score(entry(QStringLiteral("Seitenränder")), QStringLiteral("SEITENRANDER")) >= 0);
        QVERIFY(score(entry(QStringLiteral("Paste")), QStringLiteral("PASTE")) >= 0);
        QCOMPARE(fold(QStringLiteral("ÉCOLE")), QStringLiteral("ecole"));
    }

    void matchingIsByTheStartOfAWord()
    {
        const Entry e = entry(QStringLiteral("Line and Paragraph Spacing"));
        QVERIFY(score(e, QStringLiteral("para")) >= 0);
        QVERIFY(score(e, QStringLiteral("line spa")) >= 0);
        QVERIFY(score(e, QStringLiteral("spacing para")) >= 0);   // (in any order)
        QCOMPARE(score(e, QStringLiteral("graph")), -1);          // (the middle of a word is not a start)
        QCOMPARE(score(e, QStringLiteral("line copy")), -1);      // (every word must fit)
        QCOMPARE(score(e, QStringLiteral("  ")), -1);
    }

    void matchingForgivesOneWrongLetter()
    {
        const Entry e = entry(QStringLiteral("Paragraph"));
        QVERIFY(score(e, QStringLiteral("paragrph")) >= 0);    // a letter left out
        QVERIFY(score(e, QStringLiteral("parragraph")) >= 0);  // one too many
        QVERIFY(score(e, QStringLiteral("paragreph")) >= 0);   // one changed
        QVERIFY(score(e, QStringLiteral("pargaraph")) >= 0);   // two side by side swapped
        QCOMPARE(score(e, QStringLiteral("pargrph")), -1);     // two wrong
        QCOMPARE(score(entry(QStringLiteral("Copy")), QStringLiteral("cpy")), -1);   // (a short word is not guessed at)
        QVERIFY(score(entry(QStringLiteral("Color")), QStringLiteral("colr")) >= 0);
        // an exact word ranks above one with a typo
        QVERIFY(score(entry(QStringLiteral("Color")), QStringLiteral("color")) > score(entry(QStringLiteral("Color")), QStringLiteral("colr")));
    }

    void extraWordsAndTipsFindACommand()
    {
        QVERIFY(score(entry(QStringLiteral("Size"), QString(), {QStringLiteral("page size")}), QStringLiteral("page size")) >= 0);
        QCOMPARE(score(entry(QStringLiteral("Size")), QStringLiteral("page size")), -1);
        QVERIFY(score(entry(QStringLiteral("Size"), QString(), {}, QStringLiteral("Changes the paper's size")), QStringLiteral("paper")) >= 0);
        // the name ranks above the same word in a tip or the place
        QVERIFY(score(entry(QStringLiteral("Margins")), QStringLiteral("margins")) > score(entry(QStringLiteral("Size"), QStringLiteral("Layout > Margins")), QStringLiteral("margins")));
    }

    void theAppsWordsFindTheCommandsTheyAreFor()
    {
        CmdWindow w;
        w.show();
        QVERIFY(listed(w.box(), QStringLiteral("page size")).startsWith(QStringLiteral("Size")));
        QVERIFY(listed(w.box(), QStringLiteral("line spacing")).startsWith(QStringLiteral("Line and Paragraph Spacing")));
        QVERIFY(listed(w.box(), QStringLiteral("delete row")).startsWith(QStringLiteral("Delete Sheet Rows")));
        QVERIFY(listed(w.box(), QStringLiteral("DOUBLE SPACE")).contains(QStringLiteral("Line and Paragraph Spacing")));
        QVERIFY(listed(w.box(), QStringLiteral("cafe")).contains(QStringLiteral("Café Menu")));
        QCOMPARE(listed(w.box(), QStringLiteral("zzzqqq")), QStringList());
    }

    void commandsInMenusAreListedWithWhereTheyAre()
    {
        CmdWindow w;
        w.show();
        QCOMPARE(listed(w.box(), QStringLiteral("a4")).value(0), QStringLiteral("A4 Paper"));
        const int row = rowOf(w.box(), QStringLiteral("A4 Paper"));
        QVERIFY(row >= 0);
        QCOMPARE(w.box()->list()->item(row)->text().section(QLatin1Char('\n'), 1), QStringLiteral("Layout > Page Setup > Size"));
        // a command's place is "tab > group"
        QCOMPARE(listed(w.box(), QStringLiteral("bold")).value(0), QStringLiteral("Bold"));
        QCOMPARE(w.box()->shownResults().value(0).where, QStringLiteral("Home > Font"));
    }

    void namesInTheTranslatedLanguageAndInEnglishBothFind()
    {
        const QVariant before = Settings::get().value(QStringLiteral("ui/language"));
        Settings::get().setValue(QStringLiteral("ui/language"), QStringLiteral("de"));
        installTranslations();
        const auto restore = qScopeGuard([&] {
            Settings::get().setValue(QStringLiteral("ui/language"), before);
            installTranslations();
        });
        QCOMPARE(currentUiLanguage(), QStringLiteral("de"));
        const QString german = QCoreApplication::translate("Ribbon", "Margins");
        QVERIFY(german != QStringLiteral("Margins"));
        QCOMPARE(sourceText(german), QStringLiteral("Margins"));
        CmdWindow w;
        auto *margins = new QAction(german, &w);
        margins->setObjectName(QStringLiteral("test.margins"));
        w.ribbon()->tab(QStringLiteral("Layout"))->groups().first()->addSmall(margins);
        w.show();
        QVERIFY(listed(w.box(), german).contains(german));                                 // its name here
        QVERIFY(listed(w.box(), QStringLiteral("seitenrander")).contains(german));          // (accents off)
        QVERIFY(listed(w.box(), QStringLiteral("margins")).contains(german));               // its name in English
    }

    void greyedCommandsAreListedAndNotRun()
    {
        CmdWindow w;
        w.show();
        w.box()->setText(QStringLiteral("strike"));
        const int row = rowOf(w.box(), QStringLiteral("Strike"));
        QVERIFY(row >= 0);
        QVERIFY(!(w.box()->list()->item(row)->flags() & Qt::ItemIsEnabled));
        QVERIFY(w.box()->list()->item(row)->data(Qt::AccessibleTextRole).toString().contains(QStringLiteral("not available")));
        QTest::keyClick(w.box(), Qt::Key_Return);
        QVERIFY(!w.ran.contains(QStringLiteral("format.off")));   // (what Enter reaches is the Help row, not the greyed command)
        w.act(QStringLiteral("format.off"))->setEnabled(true);
        w.box()->setText(QString());
        w.box()->setText(QStringLiteral("strike"));
        QVERIFY(w.box()->list()->item(rowOf(w.box(), QStringLiteral("Strike")))->flags() & Qt::ItemIsEnabled);
    }

    void commandsOfATabThatIsNotShownAreGreyUntilItShows()
    {
        CmdWindow w;
        w.show();
        w.box()->setText(QStringLiteral("chart style"));
        int row = rowOf(w.box(), QStringLiteral("Chart Style"));
        QVERIFY(row >= 0);   // (listed)
        QVERIFY(!(w.box()->list()->item(row)->flags() & Qt::ItemIsEnabled));   // (but not usable now)
        w.ribbon()->setContextVisible(QStringLiteral("Chart Tools"), true);
        w.box()->setText(QString());
        w.box()->setText(QStringLiteral("chart style"));
        row = rowOf(w.box(), QStringLiteral("Chart Style"));
        QVERIFY(w.box()->list()->item(row)->flags() & Qt::ItemIsEnabled);
    }

    void enterRunsTheCommandAsIfClicked()
    {
        CmdWindow w;
        w.show();
        w.box()->setText(QStringLiteral("bold"));
        QTest::keyClick(w.box(), Qt::Key_Return);
        QCOMPARE(w.ran, QStringList{QStringLiteral("format.bold")});
        QCOMPARE(w.left, 1);                       // the keyboard goes back to the document
        QVERIFY(w.box()->text().isEmpty());
        QVERIFY(!w.box()->list()->isVisible());
        // a click on a row runs it too
        w.box()->setText(QStringLiteral("copy"));
        QListWidget *list = w.box()->list();
        const int row = rowOf(w.box(), QStringLiteral("Copy"));
        QTest::mouseClick(list->viewport(), Qt::LeftButton, {}, list->visualItemRect(list->item(row)).center());
        QCOMPARE(w.ran.last(), QStringLiteral("edit.copy"));
    }

    void enterOnACommandWithAMenuOpensTheMenu()
    {
        CmdWindow w;
        w.show();
        w.box()->setText(QStringLiteral("size"));
        const Entry first = w.box()->shownResults().value(0);
        QCOMPARE(first.name, QStringLiteral("Size"));
        QVERIFY(first.opensMenu);
        QToolButton *button = first.button;
        QVERIFY(button && button->menu());
        // (the menu waits for its choice in a loop of its own: it is looked at, and closed, from there)
        bool opened = false;
        QTimer::singleShot(400, button, [&] {
            opened = button->menu()->isVisible();
            button->menu()->close();
        });
        QTest::keyClick(w.box(), Qt::Key_Return);
        QTRY_VERIFY(opened);
        QCOMPARE(w.ran, QStringList());   // (it only opens)
    }

    void arrowKeysMoveThroughTheList()
    {
        CmdWindow w;
        w.show();
        w.box()->setText(QStringLiteral("paper"));   // A4 Paper, Letter Paper (and Size, by its words)
        QListWidget *list = w.box()->list();
        const int count = w.box()->shownResults().size();
        QVERIFY(count >= 2);
        const int first = list->currentRow();
        QTest::keyClick(w.box(), Qt::Key_Down);
        QCOMPARE(list->currentRow(), first + 1);
        QTest::keyClick(w.box(), Qt::Key_Up);
        QCOMPARE(list->currentRow(), first);
        QTest::keyClick(w.box(), Qt::Key_Up);   // (not above the first)
        QCOMPARE(list->currentRow(), first);
        QTest::keyClick(w.box(), Qt::Key_Down);
        const QString name = w.box()->shownResults().value(1).name;
        QTest::keyClick(w.box(), Qt::Key_Return);
        QCOMPARE(w.ran.size(), 1);
        QCOMPARE(w.act(w.ran.first())->text().isEmpty(), false);
        QVERIFY(name.size() > 0);
    }

    void altQPutsTheCursorInTheBoxAndEscapeLeaves()
    {
        CmdWindow w;
        w.show();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QVERIFY(!w.box()->hasFocus());
        pressSearchKey(&w);
        QTRY_VERIFY(w.box()->hasFocus());
        QTest::keyClicks(w.box(), QStringLiteral("bol"));
        QVERIFY(w.box()->list()->isVisible());
        QTest::keyClick(w.box(), Qt::Key_Escape);
        QCOMPARE(w.left, 1);
        QVERIFY(w.box()->text().isEmpty());
        QVERIFY(!w.box()->list()->isVisible());
        QVERIFY(w.ran.isEmpty());
    }

    void theBoxIsInTheTabRowBesideHelp()
    {
        CmdWindow w;
        w.show();
        QCOMPARE(w.ribbon()->searchWidget(), static_cast<QWidget *>(w.box()));
        QVERIFY(w.box()->isVisible());
        QVERIFY(w.ribbon()->mapFromGlobal(w.box()->mapToGlobal(QPoint(0, 0))).y() < 50);   // (in the top row)
        // the tabs stop short of it
        const QRect box(w.ribbon()->mapFromGlobal(w.box()->mapToGlobal(QPoint(0, 0))), w.box()->size());
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) {
            QWidget *tab = w.ribbon()->tabButton(i);
            if (!tab || !tab->isVisible()) continue;
            const QRect t(w.ribbon()->mapFromGlobal(tab->mapToGlobal(QPoint(0, 0))), tab->size());
            QVERIFY2(!t.intersects(box), qPrintable(w.ribbon()->tabName(i)));
        }
    }

    void theLastFiveCommandsRunFromTheBoxAreListed()
    {
        CmdWindow w;
        w.show();
        for (const char *name : {"paste", "copy", "bold", "a4", "letter", "narrow"}) {
            w.box()->setText(QString::fromLatin1(name));
            QTest::keyClick(w.box(), Qt::Key_Return);
        }
        QCOMPARE(w.box()->recent().size(), 5);
        QCOMPARE(w.box()->recent().first(), QStringLiteral("cmd:layout.narrow"));
        QVERIFY(!w.box()->recent().contains(QStringLiteral("cmd:edit.paste")));   // (the oldest is gone)
        w.box()->focusBox();
        const QList<Entry> shown = w.box()->shownResults();
        QCOMPARE(shown.size(), 5);
        QCOMPARE(shown.first().name, QStringLiteral("Narrow Margins"));
        QCOMPARE(shown.last().name, QStringLiteral("Copy"));
        // running one again brings it to the front, with no repeat
        w.box()->setText(QStringLiteral("copy"));
        QTest::keyClick(w.box(), Qt::Key_Return);
        QCOMPARE(w.box()->recent().first(), QStringLiteral("cmd:edit.copy"));
        QCOMPARE(w.box()->recent().size(), 5);
        // the list is kept for the next window
        CmdWindow other;
        QCOMPARE(other.box()->recent(), w.box()->recent());
    }

    void aLastRowSearchesHelpForWhatWasTyped()
    {
        if (help::topicIds().isEmpty()) QSKIP("no Help in this build");
        CmdWindow w;
        w.show();
        w.box()->setText(QStringLiteral("zzzqqq"));
        QListWidget *list = w.box()->list();
        QVERIFY(list->count() >= 1);
        QCOMPARE(list->item(list->count() - 1)->text(), QStringLiteral("Search Help for zzzqqq"));
        w.box()->setText(QStringLiteral("bold"));
        QCOMPARE(list->item(list->count() - 1)->text(), QStringLiteral("Search Help for bold"));
        QTest::keyClick(w.box(), Qt::Key_Down);
        QCOMPARE(list->currentRow(), list->count() - 1);
        QTest::keyClick(w.box(), Qt::Key_Return);   // (the Help command runs; the row is not a command to remember)
        QVERIFY(!w.box()->recent().contains(QStringLiteral("help")));
        QVERIFY(w.ran.contains(QStringLiteral("help.show")));
    }

    void noHelpRowWhenThereIsNoHelp()
    {
        CmdWindow w(false);
        w.show();
        w.box()->setText(QStringLiteral("bold"));
        QListWidget *list = w.box()->list();
        QCOMPARE(list->count(), 1);
        QVERIFY(!list->item(0)->text().startsWith(QStringLiteral("Search Help")));
    }

    void theFileScreenAndOptionsPagesAreFoundToo()
    {
        CmdWindow w;
        w.show();
        // a page of the File screen shows it at that page
        QVERIFY(listed(w.box(), QStringLiteral("printer")).contains(QStringLiteral("Print")));
        const Entry print = w.box()->shownResults().value(0);
        QCOMPARE(print.where, QCoreApplication::translate("Ribbon", "File"));
        QTest::keyClick(w.box(), Qt::Key_Return);
        QCOMPARE(w.ran, QStringList{QStringLiteral("file-page:print")});
        // a command of the File screen runs
        w.ran.clear();
        w.box()->setText(QStringLiteral("save"));
        const QList<Entry> found = w.box()->shownResults();
        int at = -1;
        for (int i = 0; i < found.size(); ++i)
            if (found[i].kind == Kind::File) at = i;
        QVERIFY(at >= 0);
        for (int i = 0; i < at; ++i) QTest::keyClick(w.box(), Qt::Key_Down);
        QTest::keyClick(w.box(), Qt::Key_Return);
        QCOMPARE(w.ran, QStringList{QStringLiteral("file:save")});
        // Options pages: the app's own and the shared ones, by name or by the app's words
        QVERIFY(!listed(w.box(), QStringLiteral("keyboard")).isEmpty());
        QCOMPARE(w.box()->shownResults().value(0).kind, Kind::Options);
        QCOMPARE(w.box()->shownResults().value(0).key, QStringLiteral("Customize/Keyboard"));
        w.box()->setText(QStringLiteral("autosave"));
        QCOMPARE(w.box()->shownResults().value(0).key, QStringLiteral("Save"));
        QTest::keyClick(w.box(), Qt::Key_Return);
        QCOMPARE(w.optionsAsked, QStringLiteral("Save"));
    }

    void theEmptyBoxShowsAHint()
    {
        CmdWindow w;
        w.show();
        const QString keys = CommandSearchBox::shortcut().toString(QKeySequence::NativeText);   // (what this platform binds)
        QVERIFY(w.box()->placeholderText().contains(keys));
        QVERIFY(w.box()->toolTip().contains(keys));
        QVERIFY(w.box()->text().isEmpty());
    }

    void theKeyIsAltQAndOnAMacControlOptionQ()
    {
        QCOMPARE(CommandSearchBox::shortcutFor(false), QKeySequence(Qt::ALT | Qt::Key_Q));
        QCOMPARE(CommandSearchBox::shortcutFor(true), QKeySequence(Qt::META | Qt::ALT | Qt::Key_Q));   // (Option+Q types a character there)
    }

    void aPlainAltShowsTheLettersAndAltQDoesNot()
    {
        CmdWindow w;
        auto *tips = new KeyTips(&w, w.ribbon(), [] {});
        w.show();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QTest::keyPress(&w, Qt::Key_Alt);
        QTest::keyRelease(&w, Qt::Key_Alt);
        QCOMPARE(tips->level(), KeyTips::Top);   // (Alt tapped alone)
        tips->hide();
        QTest::keyPress(&w, Qt::Key_Alt);
        pressSearchKey(&w);
        QTRY_VERIFY(w.box()->hasFocus());
        QTest::keyRelease(&w, Qt::Key_Alt);
        QCOMPARE(tips->level(), KeyTips::Off);   // (Alt used for Alt+Q: no letters)
    }

    void holdingAltOrAltThenShiftStillShowsTheLettersOnRelease()
    {
        CmdWindow w;
        auto *tips = new KeyTips(&w, w.ribbon(), [] {});
        w.show();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QTest::keyPress(&w, Qt::Key_Alt);
        QKeyEvent repeat(QEvent::ShortcutOverride, Qt::Key_Alt, Qt::AltModifier, QString(), true);   // (the key held down)
        QApplication::sendEvent(&w, &repeat);
        QKeyEvent shift(QEvent::ShortcutOverride, Qt::Key_Shift, Qt::AltModifier | Qt::ShiftModifier, QString(), false);   // (then Shift)
        QApplication::sendEvent(&w, &shift);
        QTest::keyRelease(&w, Qt::Key_Alt);
        QCOMPARE(tips->level(), KeyTips::Top);
    }

    void f10DoesNotShowTheLetters()
    {
        CmdWindow w;
        auto *tips = new KeyTips(&w, w.ribbon(), [] {});
        w.show();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QTest::keyClick(&w, Qt::Key_F10);
        QCOMPARE(tips->level(), KeyTips::Off);   // (as before the search: F10 is not a KeyTips key)
    }

    void theBoxFollowsHelpInTheTabChain()
    {
        for (bool below : {false, true}) {
            CmdWindow w;
            w.ribbon()->setQuickToolbarBelow(below);
            w.show();
            w.activateWindow();
            QVERIFY(QTest::qWaitForWindowActive(&w));
            auto *help = w.ribbon()->findChild<QWidget *>(QStringLiteral("jpRibbonHelp"));
            QVERIFY(help);
            help->setFocus();
            QCOMPARE(w.focusWidget(), help);
            QTest::keyClick(help, Qt::Key_Tab);   // (the next in the chain, as Tab moves it)
            QCOMPARE(w.focusWidget(), static_cast<QWidget *>(w.box()));
        }
    }

    void theBoxNeverCoversATabAtRestOrWithTheCursor()
    {
        for (int width : {800, 1000, 1400}) {
            CmdWindow w;
            w.ribbon()->setContextVisible(QStringLiteral("Chart Tools"), true);
            w.resize(width, 500);
            w.show();
            w.activateWindow();
            QVERIFY(QTest::qWaitForWindowActive(&w));
            for (bool focused : {false, true}) {
                if (focused) w.box()->focusBox();
                else w.box()->clearFocus();
                QTRY_COMPARE(w.box()->hasFocus(), focused);
                QApplication::processEvents();
                const QRect box(w.ribbon()->mapFromGlobal(w.box()->mapToGlobal(QPoint(0, 0))), w.box()->size());
                QVERIFY(w.box()->width() <= (focused ? 300 : 160));
                QList<QWidget *> others;
                for (int i = 0; i < w.ribbon()->tabCount(); ++i)
                    if (w.ribbon()->tabButton(i) && w.ribbon()->tabButton(i)->isVisible()) others << w.ribbon()->tabButton(i);
                if (w.ribbon()->moreButton()) others << w.ribbon()->moreButton();
                for (QWidget *o : std::as_const(others)) {
                    const QRect t(w.ribbon()->mapFromGlobal(o->mapToGlobal(QPoint(0, 0))), o->size());
                    QVERIFY2(!t.intersects(box), qPrintable(QStringLiteral("%1 px, %2: %3").arg(width).arg(focused ? "focused" : "at rest").arg(o->accessibleName())));
                }
            }
        }
    }

    void theBoxWithTheCursorStaysOffTheTabsWhenTheRowIsTight()
    {
        CmdWindow w;
        w.ribbon()->setContextVisible(QStringLiteral("Chart Tools"), true);
        w.show();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        int tight = 0;
        for (int width = 700; width >= 200; width -= 4) {
            w.resize(width, 400);
            QApplication::processEvents();
            if (foldedTabs(w.ribbon()) > 0) break;
            tight = width;
        }
        QVERIFY(tight > 0);
        w.resize(tight, 400);
        w.box()->focusBox();
        QTRY_VERIFY(w.box()->hasFocus());
        QApplication::processEvents();
        const QRect box(w.ribbon()->mapFromGlobal(w.box()->mapToGlobal(QPoint(0, 0))), w.box()->size());
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) {
            QWidget *tab = w.ribbon()->tabButton(i);
            if (!tab || !tab->isVisible()) continue;
            const QRect t(w.ribbon()->mapFromGlobal(tab->mapToGlobal(QPoint(0, 0))), tab->size());
            QVERIFY2(!t.intersects(box), qPrintable(w.ribbon()->tabName(i)));
        }
    }

    void altQDoesNothingWithTheFileScreenOpenOrTheRibbonHidden()
    {
        CmdWindow w;
        w.show();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        w.fileScreen()->setGeometry(w.rect());
        w.fileScreen()->show();
        pressSearchKey(&w);
        QVERIFY(!w.box()->hasFocus());
        w.fileScreen()->hide();
        w.ribbon()->hide();
        pressSearchKey(&w);
        QVERIFY(!w.box()->hasFocus());
        w.ribbon()->show();
        pressSearchKey(&w);   // (and it works again when they are not in the way)
        QTRY_VERIFY(w.box()->hasFocus());
    }

    void aCommandWhoseButtonIsTurnedOffIsGreyAndDoesNotRun()
    {
        CmdWindow w;
        w.show();
        QToolButton *bold = nullptr;
        for (QToolButton *b : w.ribbon()->tab(QStringLiteral("Home"))->findChildren<QToolButton *>())
            if (b->defaultAction() == w.act(QStringLiteral("format.bold"))) bold = b;
        QVERIFY(bold);
        bold->setEnabled(false);   // (as a read-only document turns a control off: the command itself stays on)
        QVERIFY(w.act(QStringLiteral("format.bold"))->isEnabled());
        w.box()->setText(QStringLiteral("bold"));
        const int row = rowOf(w.box(), QStringLiteral("Bold"));
        QVERIFY(row >= 0);
        QVERIFY(!(w.box()->list()->item(row)->flags() & Qt::ItemIsEnabled));
        QTest::keyClick(w.box(), Qt::Key_Return);
        QVERIFY(!w.ran.contains(QStringLiteral("format.bold")));
    }

    void whatTheListShowedIsCheckedAgainWhenEnterRunsIt()
    {
        CmdWindow w;
        w.show();
        w.box()->setText(QStringLiteral("bold"));
        const int row = rowOf(w.box(), QStringLiteral("Bold"));
        QVERIFY(w.box()->list()->item(row)->flags() & Qt::ItemIsEnabled);   // (on when listed)
        w.act(QStringLiteral("format.bold"))->setEnabled(false);              // (off before Enter)
        QTest::keyClick(w.box(), Qt::Key_Return);
        QVERIFY(!w.ran.contains(QStringLiteral("format.bold")));
        QVERIFY(w.box()->recent().isEmpty());
    }

    void theIndexIsMadeAgainWhenTheBoxGetsTheCursorByTheMouse()
    {
        CmdWindow w;
        w.show();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QVERIFY(listed(w.box(), QStringLiteral("zebra")).isEmpty());   // (the index is made now)
        w.box()->setText(QString());
        RibbonTab *tab = w.ribbon()->addTab(QStringLiteral("Extra"));
        RibbonGroup *group = tab->addGroup(QStringLiteral("Animals"));
        auto *zebra = new QAction(QStringLiteral("Zebra Stripes"), &w);
        zebra->setObjectName(QStringLiteral("extra.zebra"));
        group->addLarge(zebra);
        tab->finish();
        w.box()->setFocus(Qt::MouseFocusReason);
        QTRY_VERIFY(w.box()->hasFocus());
        QCOMPARE(listed(w.box(), QStringLiteral("zebra")).value(0), QStringLiteral("Zebra Stripes"));
    }

    void aCommandOnTwoTabsIsListedWhereItCanBeUsed()
    {
        CmdWindow w;
        w.ribbon()->setContextVisible(QStringLiteral("Chart Tools"), true);
        QToolButton *home = nullptr;
        for (QToolButton *b : w.ribbon()->tab(QStringLiteral("Home"))->findChildren<QToolButton *>())
            if (b->defaultAction() == w.act(QStringLiteral("edit.copy"))) home = b;
        QVERIFY(home);
        home->setEnabled(false);   // (the copy that comes first cannot be used; the one on the shown tab can)
        w.show();
        QCOMPARE(listed(w.box(), QStringLiteral("copy")).value(0), QStringLiteral("Copy"));
        QCOMPARE(w.box()->shownResults().value(0).where, QStringLiteral("Chart Format > Styles"));
        QVERIFY(w.box()->list()->item(0)->flags() & Qt::ItemIsEnabled);
    }

    void namesAreShownAsWrittenEvenWhenTheyLookLikeMarkup()
    {
        CmdWindow w;
        w.show();
        QCOMPARE(listed(w.box(), QStringLiteral("raw")).value(0), QStringLiteral("<b>Raw</b> & co"));
    }

    void everyEntryHasItsWordsReadOnceAndScoresAsIfNot()
    {
        CmdWindow w;
        const QList<Entry> all = collect(&w, Words());
        QVERIFY(all.size() > 10);
        for (const Entry &e : all) {
            QVERIFY2(e.indexed, qPrintable(e.name));
            QVERIFY(!e.nameTok.isEmpty() || e.name.isEmpty());
            Entry bare = e;   // (the same entry made by hand scores the same)
            bare.indexed = false;
            QCOMPARE(score(bare, QStringLiteral("para spacing")), score(e, QStringLiteral("para spacing")));
        }
    }

    void aTranslatedTextRemembersEveryEnglishItCameFrom()
    {
        const QVariant before = Settings::get().value(QStringLiteral("ui/language"));
        Settings::get().setValue(QStringLiteral("ui/language"), QStringLiteral("de"));
        installTranslations();
        const auto restore = qScopeGuard([&] {
            Settings::get().setValue(QStringLiteral("ui/language"), before);
            installTranslations();
        });
        const QString circle = QCoreApplication::translate("Animation", "Circle"), pie = QCoreApplication::translate("jp::WriteWindow", "Pie");
        QCOMPARE(circle, pie);   // (one German word for two English ones)
        QCoreApplication::translate("jp::WriteWindow", "Pie");   // (asked for again: noted once)
        QVERIFY(sourceTexts(circle).contains(QStringLiteral("Circle")));
        QVERIFY(sourceTexts(circle).contains(QStringLiteral("Pie")));
        QCOMPARE(sourceTexts(circle).count(QStringLiteral("Pie")), 1);   // (noted once)
        CmdWindow w;
        auto *action = new QAction(circle, &w);
        action->setObjectName(QStringLiteral("test.kreis"));
        w.ribbon()->tab(QStringLiteral("Layout"))->groups().first()->addSmall(action);
        w.show();
        QCOMPARE(listed(w.box(), QStringLiteral("circle")).value(0), circle);
        QCOMPARE(listed(w.box(), QStringLiteral("pie")).value(0), circle);
    }

    void aTextAskedForInAnotherThreadIsNotNoted()
    {
        const QVariant before = Settings::get().value(QStringLiteral("ui/language"));
        Settings::get().setValue(QStringLiteral("ui/language"), QStringLiteral("de"));
        installTranslations();
        const auto restore = qScopeGuard([&] {
            Settings::get().setValue(QStringLiteral("ui/language"), before);
            installTranslations();
        });
        QString german;
        auto *worker = QThread::create([&german] { german = QCoreApplication::translate("Ribbon", "Bring Forward"); });
        worker->start();
        QVERIFY(worker->wait(10000));
        delete worker;
        QVERIFY(german != QStringLiteral("Bring Forward"));   // (the translation is still found there)
        QVERIFY(sourceTexts(german).isEmpty());                // (and nothing was noted from the thread)
        QCOMPARE(QCoreApplication::translate("Ribbon", "Bring Forward"), german);
        QCOMPARE(sourceTexts(german), QStringList{QStringLiteral("Bring Forward")});   // (here it is)
    }

    void anIdlessCommandKeepsItsRecentSlotWhenTheLanguageChanges()
    {
        const QVariant before = Settings::get().value(QStringLiteral("ui/language"));
        auto idIn = [](const QString &language) {
            Settings::get().setValue(QStringLiteral("ui/language"), language);
            installTranslations();
            CmdWindow w;
            auto *action = new QAction(QCoreApplication::translate("Ribbon", "Margins"), &w);   // (no object name)
            w.ribbon()->tab(QStringLiteral("Layout"))->groups().first()->addSmall(action);
            for (const Entry &e : collect(&w, Words()))
                if (e.action == action) return e.id;
            return QString();
        };
        const QString german = idIn(QStringLiteral("de")), english = idIn(QStringLiteral("en"));
        Settings::get().setValue(QStringLiteral("ui/language"), before);
        installTranslations();
        QVERIFY(!german.isEmpty());
        QCOMPARE(german, english);
    }

    void theBoxIsNarrowAtRestAndWideWithTheCursorWithoutFoldingATab()
    {
        CmdWindow w;
        w.resize(1000, 500);
        w.show();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        // (the widths are bounds, not pixels: a font's metrics move how much room the tabs leave)
        QTRY_VERIFY(w.box()->width() > 24 && w.box()->width() <= 160);
        const int rest = w.box()->width();
        QCOMPARE(foldedTabs(w.ribbon()), 0);
        pressSearchKey(&w);
        QTRY_VERIFY(w.box()->width() > rest && w.box()->width() <= 300);
        QCOMPARE(foldedTabs(w.ribbon()), 0);   // (it opens over the tabs: none moves)
        QTest::keyClick(w.box(), Qt::Key_Escape);
        QTRY_COMPARE(w.box()->width(), rest);
    }

    void theBoxShrinksToItsIconBeforeATabFolds()
    {
        CmdWindow w;
        w.show();
        int okBox = -1;
        bool folded = false;
        for (int width = 700; width >= 200 && !folded; width -= 4) {
            w.resize(width, 400);
            QApplication::processEvents();
            if (foldedTabs(w.ribbon()) == 0) okBox = w.box()->width();
            else folded = true;
        }
        QVERIFY(folded);
        QVERIFY2(okBox >= 0 && okBox < 160, qPrintable(QString::number(okBox)));   // (the last width that kept every tab: the box had given room)
    }

    void thePopupSitsUnderTheBoxAndStaysOnTheScreen()
    {
        const QRect screen(0, 0, 1920, 1080);
        const QRect box(700, 10, 300, 28);
        const QRect r = popupRect(box, QSize(340, 400), screen);
        QCOMPARE(r.top(), box.bottom() + 3);       // under it
        QCOMPARE(r.right(), box.right());          // its right edge with the box's
        QCOMPARE(r.size(), QSize(340, 400));
        const QRect edge = popupRect(QRect(1800, 10, 100, 28), QSize(340, 400), screen);
        QVERIFY(screen.contains(edge));
        const QRect low = popupRect(QRect(700, 1000, 300, 28), QSize(340, 400), screen);
        QVERIFY(low.bottom() <= screen.bottom());
    }

    void galleryItemsAreFoundAndAppliedAsAClickDoes()
    {
        CmdWindow w;
        w.show();
        QCOMPARE(listed(w.box(), QStringLiteral("heading 1")).value(0), QStringLiteral("Heading 1"));
        const Entry e = w.box()->shownResults().value(0);
        QCOMPARE(e.kind, Kind::Gallery);
        QCOMPARE(e.where, QStringLiteral("Home > Styles"));
        QTest::keyClick(w.box(), Qt::Key_Return);
        QCOMPARE(w.ran, QStringList{QStringLiteral("gallery:h1")});
        QCOMPARE(w.box()->recent().first(), e.id);
    }

    void galleryItemsAreCappedAtTwoThousand()
    {
        CmdWindow w(true, 2600);
        w.show();
        int n = 0;
        for (const Entry &e : collect(&w, Words()))
            if (e.kind == Kind::Gallery) ++n;
        QCOMPARE(n, kMaxGalleryItems);
    }

    void theProofingOptionsAreFoundByTheWordsOfTheCustomDictionary()
    {
        CmdWindow w;
        bool found = false;
        for (const Entry &e : collect(&w, Words()))
            if (e.id == QLatin1String("options:Proofing")) {
                found = true;
                for (const char *word : {"custom dictionary", "excluded words", "added words", "detect language"}) QVERIFY2(e.extra.contains(QString::fromLatin1(word)), word);
            }
        QVERIFY(found);
    }

    void theWordsFileIsReadForCommandsFileEntriesAndOptions()
    {
        const Words words = parseWords(QByteArray(kWords));
        QCOMPARE(words.value(QStringLiteral("cmd:ribbon.size")), (QStringList{QStringLiteral("page size"), QStringLiteral("paper size")}));
        QCOMPARE(words.value(QStringLiteral("file:print")), QStringList{QStringLiteral("printer")});
        QCOMPARE(words.value(QStringLiteral("options:Save")), QStringList{QStringLiteral("autosave")});
        QVERIFY(parseWords(QByteArray("not json")).isEmpty());
    }
};

JP_TEST_CLASS(ShellCommandSearchTests)
#include "tst_shell_cmdsearch.moc"
