#include "core/datafolder.h"
#include "app/keymap.h"
#include <QScopeGuard>
// JeffDeck's tests of the window: ribbon, help, status bar, headers and footers, backgrounds, the slide menu, and find.
#include "deckhelpers.h"
#include "office/connectionsites.h"
#include "../withdialog.h"
#include <QInputMethodEvent>
#include <QLabel>
#include "app/theme.h"
#include "../minitoolbartest.h"
#include "app/findreplace.h"
#include "app/deckfilescreen.h"
#include "app/keytips.h"
#include "io/deckoutput.h"
#include "core/threadtime.h"
#include "app/notespageview.h"
#include "core/notesmaster.h"
#include "core/printheaderfooter.h"
#include "core/gridview.h"
#include "app/deckcanvas.h"
#include "app/guidesdialog.h"
#include <QDoubleSpinBox>
#include "app/theme.h"
#include "app/settings.h"

// What the program's settings hold for the grid, put back when the test ends (the dialog writes them).
static auto keepGridSettings()
{
    const QStringList keys = {QStringLiteral("view/gridSpacing"), QStringLiteral("view/snapGrid"), QStringLiteral("view/snapObjects")};
    const QVector<QVariant> was = {jp::Settings::get().value(keys[0], 6.0), jp::Settings::get().value(keys[1], false), jp::Settings::get().value(keys[2], true)};
    return qScopeGuard([keys, was] {
        for (int i = 0; i < keys.size(); ++i) jp::Settings::get().setValue(keys[i], was[i]);
    });
}

class DeckWindowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The quick access toolbar starts with Open, as in the other apps.
    void theQuickAccessToolbarStartsWithOpen()
    {
        DeckWindow w;
        const QList<QToolButton *> buttons = w.ribbon()->quickAccessButtons();
        QVERIFY(!buttons.isEmpty());
        QVERIFY(buttons.first()->defaultAction());
        QCOMPARE(buttons.first()->defaultAction(), w.act(QStringLiteral("file.open")));
    }

    // The same command has the same name in all four apps: Spelling, Word Count, Find's two options, and the same quick toolbar (Open, Save, Print, Undo, Redo, Cut, Copy, Paste).
    void commandsUseTheSuitesWords()
    {
        DeckWindow w;
        w.show();
        QCOMPARE(w.act(QStringLiteral("review.spelling"))->text(), QStringLiteral("Spelling"));
        QCOMPARE(w.act(QStringLiteral("review.wordCount"))->text(), QStringLiteral("Word Count"));
        w.act(QStringLiteral("edit.find"))->trigger();
        QVERIFY(w.findPane());
        QCOMPARE(w.findPane()->matchCase()->text(), QStringLiteral("Match case"));
        QCOMPARE(w.findPane()->wholeWords()->text(), QStringLiteral("Find whole words only"));
        QList<QAction *> quick;
        for (QToolButton *b : w.ribbon()->quickAccessButtons()) quick << b->defaultAction();
        QCOMPARE(quick, (QList<QAction *>{w.act(QStringLiteral("file.open")), w.act(QStringLiteral("file.save")), w.act(QStringLiteral("file.print")), w.act(QStringLiteral("edit.undo")), w.act(QStringLiteral("edit.redo")), w.act(QStringLiteral("edit.cut")), w.act(QStringLiteral("edit.copy")), w.act(QStringLiteral("edit.paste"))}));
    }

    void initTestCase() { initDeckTestCase(); }

    // A new layout moves the placeholders to its places, by kind, and keeps
    // their text; its places that nothing filled get empty placeholders.
    void applyLayoutKeepsText()
    {
        auto d = deck::newPresentation();
        Page &slide = *d->pages[0];
        auto *title = static_cast<TextItem *>(slide.items[0].get());
        d->storyDoc(title->storyId)->setPlainText(QStringLiteral("Quarterly results"));
        const MasterPage *twoContent = deck::layoutOfType(*d, QStringLiteral("twoObj"));
        deck::applyLayout(*d, slide, *twoContent);
        QCOMPARE(slide.masterId, twoContent->id);
        QCOMPARE(title->rect, twoContent->items[0]->rect);
        QCOMPARE(d->storyDoc(title->storyId)->toPlainText(), QStringLiteral("Quarterly results"));
        QCOMPARE(int(slide.items.size()), 3);   // title, the subtitle in the first content place, a new second one
        // Reset puts a moved placeholder back.
        title->rect.translate(50, 50);
        deck::resetSlide(*d, slide);
        QCOMPARE(title->rect, twoContent->items[0]->rect);
    }

    // Each app names its color and badge for the shared window: the File
    // button and every accent in it follow, and the dark shade reads on the
    // dark ribbon.
    void appInfoColorsPerApp()
    {
        auto contrast = [](const QColor &a, const QColor &b) {
            auto lum = [](const QColor &c) {
                auto f = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
                return 0.2126 * f(c.redF()) + 0.7152 * f(c.greenF()) + 0.0722 * f(c.blueF());
            };
            const double la = lum(a), lb = lum(b);
            return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
        };
        const AppInfo pub = pubAppInfo(), deckInfo = deckAppInfo();
        QCOMPARE(pub.accent, QColor(0x9E, 0x1F, 0x63));
        QCOMPARE(pub.accentDark, QColor(0xE0, 0x6A, 0xA5));
        QCOMPARE(pub.badge, QStringLiteral("JP"));
        QCOMPARE(deckInfo.accent, QColor(0xB6, 0x42, 0x0F));
        QCOMPARE(deckInfo.badge, QStringLiteral("JD"));
        QVERIFY(deckInfo.accentDark.isValid());
        QVERIFY(contrast(deckInfo.accentDark, QColor(0x26, 0x2B, 0x33)) >= 4.5);   // on the dark ribbon
        QVERIFY(contrast(deckInfo.accent, QColor(0xFB, 0xFB, 0xFC)) >= 4.5);       // on the light one
        QVERIFY(contrast(deckInfo.highlight, Qt::white) >= 4.5);                   // selected text on the dark selection
        // The shared parts read the running app's.
        setUiDark(false);
        QCOMPARE(uiAccent(), deckInfo.accent);
        {
            DeckWindow w;
            auto *file = w.ribbon()->findChild<QAbstractButton *>(QStringLiteral("jpRibbonFile"));
            QVERIFY(file);
            file->resize(file->sizeHint());
            // The badge is a circle of the accent at the button's left.
            const QColor badge = file->grab().toImage().pixelColor(7, file->height() / 2);
            QVERIFY2(std::abs(badge.red() - deckInfo.accent.red()) < 12 && std::abs(badge.green() - deckInfo.accent.green()) < 12
                         && std::abs(badge.blue() - deckInfo.accent.blue()) < 12,
                     qPrintable(badge.name()));
        }
        setUiDark(true);
        QCOMPARE(uiAccent(), deckInfo.accentDark);
        setUiDark(false);
    }

    // Replace Fonts (Home > Editing): every font the presentation uses is listed (slides, notes, masters, and layouts), one is put in the place of another everywhere in one step of Undo,
    // and the theme's font changes only when it is the one picked.
    void replaceFontsSwapsAFontEverywhereAsOneStep()
    {
        DeckWindow w;
        w.show();
        Document &d = *w.editor()->doc();
        auto frame = std::make_shared<TextItem>();
        frame->id = QStringLiteral("f1");
        frame->rect = QRectF(40, 40, 400, 60);
        frame->storyId = d.createStory(QStringLiteral("words"));
        {
            QTextCursor c(d.storyDoc(frame->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies(QStringList{QStringLiteral("Zephyr Test")});
            c.mergeCharFormat(f);
        }
        d.pages[0]->items.push_back(frame);
        // (A layout's own text is a story of the document too.)
        QVERIFY(!d.masters.isEmpty());
        auto onLayout = std::make_shared<TextItem>();
        onLayout->id = QStringLiteral("l1");
        onLayout->rect = QRectF(40, 400, 400, 60);
        onLayout->storyId = d.createStory(QStringLiteral("layout words"));
        {
            QTextCursor c(d.storyDoc(onLayout->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies(QStringList{QStringLiteral("Zephyr Test")});
            c.mergeCharFormat(f);
        }
        d.masters.last()->items.push_back(onLayout);
        const QString heading = d.fonts.heading, slideStory = frame->storyId, layoutStory = onLayout->storyId;
        const int steps = w.editor()->undoStack()->count();
        withDialog([&] { w.act("edit.replaceFonts")->trigger(); }, QStringLiteral("replaceFontsDialog"), [](QDialog *dlg) {
            auto *from = dlg->findChild<QComboBox *>(QStringLiteral("replaceFontsFrom"));
            QVERIFY(from);
            const int at = from->findData(QStringLiteral("Zephyr Test"));
            QVERIFY(at >= 0);
            QVERIFY(from->itemText(at).contains(QLatin1String("used in 2")));
            from->setCurrentIndex(at);
            dlg->findChild<QComboBox *>(QStringLiteral("replaceFontsTo"))->setEditText(QStringLiteral("Zephyr Other"));
        });
        QCOMPARE(w.editor()->undoStack()->count(), steps + 1);
        auto familyOf = [&](const QString &storyId) {
            QTextCursor c(w.editor()->doc()->storyDoc(storyId));
            c.setPosition(1);
            return c.charFormat().fontFamilies().toStringList().value(0);
        };
        QCOMPARE(familyOf(slideStory), QStringLiteral("Zephyr Other"));
        QCOMPARE(familyOf(layoutStory), QStringLiteral("Zephyr Other"));
        QCOMPARE(w.editor()->doc()->fonts.heading, heading);
        w.editor()->undo();
        QCOMPARE(familyOf(slideStory), QStringLiteral("Zephyr Test"));
        QCOMPARE(familyOf(layoutStory), QStringLiteral("Zephyr Test"));
    }

    // The slide thumbnails: a row for each slide, the current row following
    // the editor and the editor following Up and Down; the current slide
    // outlined in the accent without a fill over it; a hidden slide marked.
    // JeffDeck's ribbon labels are listed for translators (tools/ribbon_strings.py), as JeffPub's are.
    void ribbonStringsListed()
    {
        QFile json(QStringLiteral(":/deck/ribbon.json"));
        QVERIFY(json.open(QIODevice::ReadOnly));
        QFile listed(QStringLiteral(JD_SOURCE_DIR "/apps/deck/app/ribbon_strings.cpp"));
        QVERIFY(listed.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString have = QString::fromUtf8(listed.readAll());
        QStringList missing;
        const QStringList keys = {"name", "text", "launcherTip", "tip", "label", "section", "submenu", "context"};
        std::function<void(const QJsonValue &)> walk = [&](const QJsonValue &v) {
            if (v.isObject()) {
                const QJsonObject o = v.toObject();
                for (auto it = o.begin(); it != o.end(); ++it) {
                    if (keys.contains(it.key()) && it.value().isString() && !it.value().toString().isEmpty()) {
                        const QString lit = QStringLiteral("QT_TRANSLATE_NOOP(\"Ribbon\", \"%1\")").arg(it.value().toString().replace(QLatin1Char('"'), QStringLiteral("\\\"")));
                        if (!have.contains(lit)) missing << it.value().toString();
                    }
                    walk(it.value());
                }
            } else if (v.isArray()) {
                for (const QJsonValue &x : v.toArray()) walk(x);
            }
        };
        walk(QJsonDocument::fromJson(json.readAll()).object());
        QVERIFY2(missing.isEmpty(), qPrintable(QStringLiteral("not in ribbon_strings.cpp (run tools/ribbon_strings.py): ") + missing.join(QStringLiteral(", "))));
    }

    // JeffDeck has the suite's translations built in, and speaks German when German is chosen.
    void translationsAreBuiltIn()
    {
        const QStringList have = availableUiLanguages();
        for (const char *lang : {"de", "es", "fr", "it", "nl", "pt_BR", "pt_PT"}) QVERIFY2(have.contains(QLatin1String(lang)), qPrintable(have.join(QLatin1Char(' '))));
        QTranslator de;
        QVERIFY(de.load(jp::dataFolder(QStringLiteral("i18n")) + QStringLiteral("/jeffoffice_de.qm")));
        // A string of the shared libraries, translated.
        QVERIFY(!de.translate("Ribbon", "File").isEmpty());
    }

    // JeffDeck's Help lists its own topics (resources/help/en/deck) and none of
    // JeffPub's: a topic only JeffPub has isn't found, and one both have (the
    // contents, the keyboard) is JeffDeck's.
    void helpListsDeckTopicsOnly()
    {
        QCOMPARE(appInfo().helpFolder, QStringLiteral("deck"));
        const QStringList ids = help::topicIds();
        QVERIFY2(ids.size() >= 20, qPrintable(ids.join(QLatin1Char(' '))));
        for (const char *id : {"index", "start", "window", "slides", "show", "printing", "keyboard", "tab-home", "tab-slide-show"})
            QVERIFY2(ids.contains(QLatin1String(id)), id);
        QCOMPARE(help::topic(QStringLiteral("index")).title, QStringLiteral("JeffDeck Help"));
        for (const QString &file : QDir(jp::dataFolder(QStringLiteral("help")) + QStringLiteral("/en")).entryList({QStringLiteral("*.md")}, QDir::Files)) {
            const QString id = file.chopped(3);
            QFile pub(jp::dataFolder(QStringLiteral("help")) + QStringLiteral("/en/") + file);
            QVERIFY(pub.open(QIODevice::ReadOnly));
            if (ids.contains(id)) QVERIFY2(help::topic(id).markdown.toUtf8() != pub.readAll(), qPrintable(id));
            else QVERIFY2(help::topic(id).isNull(), qPrintable(id));
        }
        // The search finds them.
        QVERIFY(help::searchTopics(QStringLiteral("how do I add a transition")).contains(QStringLiteral("transitions")));
        QVERIFY(help::searchTopics(QStringLiteral("zzqxv")).isEmpty());
    }

    // Every topic has a title and search words, every link and F1 context leads
    // to a topic, the contents lists them all, the keyboard topic's table comes
    // from JeffDeck's commands, and none names another company's products.
    void helpTopicsAreComplete()
    {
        const QStringList ids = help::topicIds();
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)]+)\\)"));
        static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office|PowerPoint|Excel|WordArt)\\b|(?<!Each )\\bWord\\b(?! Count)"));
        const help::Topic contents = help::topic(QStringLiteral("index"));
        QStringList problems;
        for (const QString &id : ids) {
            const help::Topic t = help::topic(id);
            if (t.title.isEmpty()) problems << id + QStringLiteral(": no title");
            if (t.keywords.isEmpty()) problems << id + QStringLiteral(": no keywords");
            if (const auto m = banned.match(t.markdown); m.hasMatch()) problems << id + QStringLiteral(": says ") + m.captured(0);
            for (auto m = link.globalMatch(t.markdown); m.hasNext();) {
                const QString target = m.next().captured(1);
                if (!target.startsWith(QLatin1String("https://")) && !ids.contains(target)) problems << id + QStringLiteral(": link to ") + target;
            }
            if (id != QLatin1String("index") && !contents.markdown.contains(QStringLiteral("](%1)").arg(id))) problems << id + QStringLiteral(": not in the contents");
            QTextDocument doc;
            doc.setMarkdown(help::displayMarkdown(t, nullptr), QTextDocument::MarkdownDialectGitHub);
            if (doc.toPlainText().trimmed().size() < 100) problems << id + QStringLiteral(": nearly empty");
        }
        QFile cf(jp::dataFolder(QStringLiteral("help")) + QStringLiteral("/en/deck/context.json"));
        QVERIFY(cf.open(QIODevice::ReadOnly));
        const QJsonObject context = QJsonDocument::fromJson(cf.readAll()).object();
        QVERIFY(!context.isEmpty());
        for (auto it = context.begin(); it != context.end(); ++it)
            if (!ids.contains(it.value().toString())) problems << QStringLiteral("context %1: no topic %2").arg(it.key(), it.value().toString());
        DeckWindow w;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (help::contextTopic(QStringLiteral("tab:") + w.ribbon()->tabName(i)).isEmpty()) problems << QStringLiteral("no topic for the tab ") + w.ribbon()->tabName(i);
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
        const QString keys = help::displayMarkdown(help::topic(QStringLiteral("keyboard")), &w);
        QVERIFY(!keys.contains(QLatin1String("<!--")));
        QVERIFY2(keys.contains(QStringLiteral("| New Slide | %1 |").arg(QKeySequence(Qt::CTRL | Qt::Key_M).toString(QKeySequence::NativeText))), qPrintable(keys.right(3000)));
        QVERIFY(keys.contains(QStringLiteral("| From Beginning | %1 |").arg(QKeySequence(Qt::Key_F5).toString(QKeySequence::NativeText))));
        QVERIFY(keys.contains(QStringLiteral("| Help | %1 |").arg(QKeySequence(QKeySequence::HelpContents).toString(QKeySequence::NativeText))));
    }

    // F1 and the "?" beside the ribbon's collapse arrow open JeffDeck's Help: the
    // contents from the slide, a tab's topic from the ribbon, and a window of its
    // own over the File screen.
    void helpOpensFromF1AndTheHelpButton()
    {
        DeckWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QVERIFY(!w.helpView());
        w.canvas()->setFocus();
        QTest::keyClick(w.canvas(), Qt::Key_F1);
        HelpView *v = w.helpView();
        QVERIFY(v && v->isVisible());
        QCOMPARE(v->currentTopic(), QStringLiteral("index"));
        QVERIFY(v->browser()->toPlainText().contains(QLatin1String("JeffDeck")));
        QVERIFY(!v->browser()->toPlainText().contains(QLatin1String("publication")));
        // From the ribbon: the tab's topic.
        Ribbon *r = w.ribbon();
        r->showTab(r->tab(QStringLiteral("Transitions")));
        r->focusCurrentTab();
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_F1);
        QCOMPARE(v->currentTopic(), QStringLiteral("tab-transitions"));
        // The "?" opens it again once it is put away.
        auto *dock = w.findChild<QDockWidget *>(QStringLiteral("helpDock"));
        QVERIFY(dock && dock->isVisible());
        dock->hide();
        auto *qmark = r->findChild<QAbstractButton *>(QStringLiteral("jpRibbonHelp"));
        QVERIFY(qmark && qmark->isVisible());
        qmark->click();
        QVERIFY(dock->isVisible());
        QCOMPARE(w.helpView(), v);
        // Over the File screen, which covers the pane: a window, on that page's topic.
        w.showFileScreen(QStringLiteral("print"));
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_F1);
        auto *hw = w.findChild<HelpWindow *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(hw && hw->isVisible());
        QCOMPARE(hw->view()->currentTopic(), QStringLiteral("printing"));
        hw->close();
        QTRY_VERIFY(!w.findChild<HelpWindow *>());   // closing deletes it
    }

    // A translated topic of JeffDeck's Help is one of the English ones, with a
    // title and search words, the same links as the English, and no other
    // company's product names.
    void translatedDeckHelpIsComplete()
    {
        const QDir help(QStringLiteral(JD_SOURCE_DIR "/resources/help"));
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)\\s]+)\\)"));
        static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office|PowerPoint|Excel|WordArt)\\b|(?<!Each )\\bWord\\b(?! Count)"));
        auto read = [](const QString &path) {
            QFile f(path);
            return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
        };
        auto links = [](const QString &md) {
            QStringList out;
            for (auto m = link.globalMatch(md); m.hasNext();) out << m.next().captured(1);
            out.sort();
            return out;
        };
        QStringList problems;
        for (const QString &lang : help.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (lang == QLatin1String("en")) continue;
            const QDir folder(help.filePath(lang + QStringLiteral("/deck")));
            for (const QString &file : folder.entryList({QStringLiteral("*.md")}, QDir::Files)) {
                const QString en = read(help.filePath(QStringLiteral("en/deck/") + file)), md = read(folder.filePath(file));
                const QString where = lang + QStringLiteral("/deck/") + file;
                if (en.isEmpty()) {
                    problems << where + QStringLiteral(": no English topic");
                    continue;
                }
                if (!md.startsWith(QStringLiteral("# "))) problems << where + QStringLiteral(": no title");
                if (en.contains(QStringLiteral("<!-- keywords:")) && !md.contains(QStringLiteral("<!-- keywords:"))) problems << where + QStringLiteral(": no keywords");
                // (A language whose contents were written before a topic was added has no link to it until the nightly translation makes the topic: the English links to topics it does not have yet are left out.)
                QStringList wanted;
                for (const QString &target : links(en))
                    if (md.contains(QStringLiteral("](") + target + QLatin1Char(')')) || QFile::exists(folder.filePath(target + QStringLiteral(".md")))) wanted << target;
                if (links(md) != wanted) problems << where + QStringLiteral(": links differ from the English");
                if (const auto m = banned.match(md); m.hasMatch()) problems << where + QStringLiteral(": says ") + m.captured(0);
            }
        }
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
    }

    void thumbnailsFollowTheEditor()
    {
        setUiDark(false);
        DeckWindow w;
        Editor *ed = w.editor();
        SlidesPane *pane = w.slidesPane();
        QCOMPARE(pane->count(), 1);
        QCOMPARE(pane->currentRow(), 0);
        w.newSlide();
        w.newSlide();
        QTRY_COMPARE(pane->count(), 3);
        QCOMPARE(pane->currentRow(), 2);
        ed->setCurrentPage(0);
        QCOMPARE(pane->currentRow(), 0);
        QCOMPARE(pane->item(1)->text(), QStringLiteral("2"));   // the number, which the row draws at the thumbnail's left
        QTest::keyClick(pane, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 1);
        QTest::keyClick(pane, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 2);
        QTest::keyClick(pane, Qt::Key_Down);   // the last stays
        QCOMPARE(ed->currentPage(), 2);
        QTest::keyClick(pane, Qt::Key_Up);
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(pane->accessibleName(), QStringLiteral("Slides"));
        QCOMPARE(pane->item(1)->data(Qt::AccessibleTextRole).toString(), QStringLiteral("Slide 2"));
        w.toggleHidden();
        QTRY_VERIFY(pane->item(1)->data(SlidesPane::HiddenRole).toBool());
        QCOMPARE(pane->item(1)->data(Qt::AccessibleTextRole).toString(), QStringLiteral("Slide 2, hidden"));
        QVERIFY(!pane->item(0)->data(SlidesPane::HiddenRole).toBool());

        // Drawn: the current thumbnail has the accent around it and its own white inside; the others a gray edge.
        ed->setCurrentPage(2);
        pane->resize(260, 420);
        pane->show();
        const QImage img = pane->viewport()->grab().toImage();
        const QRect cur = pane->thumbnailRect(2), other = pane->thumbnailRect(0), hid = pane->thumbnailRect(1);
        QCOMPARE(img.pixelColor(cur.left() - 2, cur.center().y()), deckAppInfo().accent);
        QCOMPARE(img.pixelColor(cur.center().x(), cur.top() - 2), deckAppInfo().accent);
        QCOMPARE(img.pixelColor(cur.left() + 4, cur.top() + 4), QColor(Qt::white));   // no fill over the slide
        QVERIFY(img.pixelColor(other.left() - 2, other.center().y()) != deckAppInfo().accent);
        QCOMPARE(img.pixelColor(other.left() + 4, other.top() + 4), QColor(Qt::white));
        // Hidden: faded, so its white isn't white any more.
        QVERIFY(img.pixelColor(hid.left() + 4, hid.top() + 4) != QColor(Qt::white));
        pane->hide();
    }

    // A new slide size fits everything on every slide, layout, and the slide
    // master to it (Ensure Fit): the layouts and placeholders take the new
    // shape, text shrinks by the width's ratio, and the other objects on a
    // slide keep their shape, smaller and centered.
    void slideSizeScalesEverything()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(100, 100, 200, 100);
        d->pages[1]->items.push_back(shape);
        const auto *title = static_cast<const TextItem *>(d->pages[1]->items[0].get());
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
        const QRectF slideBefore = title->rect, layoutBefore = layout->items[0]->rect, masterBefore = d->masters[0]->items[0]->rect;
        auto size = [&](const QString &storyId) { return QTextCursor(d->storyDoc(storyId)).charFormat().fontPointSize(); };
        const double textBefore = size(title->storyId);
        QVERIFY(textBefore > 0);

        deck::setSlideSize(*d, QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(d->pageSize(), QSizeF(720, 540));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Standard"));
        auto narrower = [](const QRectF &r) { return QRectF(r.x() * 0.75, r.y(), r.width() * 0.75, r.height()); };
        QCOMPARE(title->rect, narrower(slideBefore));
        QCOMPARE(layout->items[0]->rect, narrower(layoutBefore));
        QCOMPARE(d->masters[0]->items[0]->rect, narrower(masterBefore));
        QCOMPARE(size(title->storyId), textBefore * 0.75);
        QCOMPARE(shape->rect, QRectF(75, 100 * 0.75 + 67.5, 150, 75));   // in shape, smaller, and in the middle
        // A slide still draws, at its new size.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QCOMPARE(Renderer::renderToImage(ctx, 0, 0.5).size(), QSize(360, 270));

        // Wider again: the placeholders and layouts are where they were, nothing else grows.
        deck::setSlideSize(*d, QSizeF(deck::kSlideWidth, deck::kSlideHeight));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Widescreen"));
        QCOMPARE(title->rect, slideBefore);
        QCOMPARE(layout->items[0]->rect, layoutBefore);
        QCOMPARE(size(title->storyId), textBefore * 0.75);
        QCOMPARE(shape->rect, QRectF(75 + 120, 100 * 0.75 + 67.5, 150, 75));
    }

    // Header & Footer puts the slide number, the date, and a line of text in
    // each slide's own placeholders for them, and takes them away again.
    // A slide's text box shows text that doesn't fit, running out of it as
    // the presentation program draws it: down from text at the top, both ways
    // from centered text, up from text at the bottom. A publication's box
    // keeps it for a next box instead. Text that fits is set as before.
    void slideTextRunsOutOfItsBox()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat cf;
        cf.setFontPointSize(20);
        c.insertText(QStringLiteral("One"), cf);
        c.insertBlock();
        c.insertText(QStringLiteral("Two"), cf);
        c.insertBlock();
        c.insertText(QStringLiteral("Three"), cf);
        FrameSpec fs;
        fs.size = QSizeF(200, 30);
        fs.insets = QMarginsF(0, 0, 0, 0);
        StoryLayout held;
        held.build(&doc, {fs}, LayoutEnv());
        QVERIFY(held.overflow());
        QVERIFY(held.lineInfo(0).size() < 3);
        fs.runsOver = true;
        for (const VAlign v : {VAlign::Top, VAlign::Middle, VAlign::Bottom}) {
            fs.valign = v;
            StoryLayout lay;
            lay.build(&doc, {fs}, LayoutEnv());
            QVERIFY(!lay.overflow());
            const auto lines = lay.lineInfo(0);
            QCOMPARE(int(lines.size()), 3);
            const double top = lines.first().rect.top(), bottom = lines.last().rect.bottom() - lines.last().below;
            QVERIFY(bottom - top > 30);
            if (v == VAlign::Top) QVERIFY2(std::abs(top) < 0.5, qPrintable(QString::number(top)));
            if (v == VAlign::Middle) QVERIFY2(std::abs((top + bottom) / 2 - 15) < 0.5, qPrintable(QString::number((top + bottom) / 2)));
            if (v == VAlign::Bottom) QVERIFY2(std::abs(bottom - 30) < 0.5, qPrintable(QString::number(bottom)));
        }
        fs.size = QSizeF(200, 300);
        fs.valign = VAlign::Middle;
        FrameSpec plain = fs;
        plain.runsOver = false;
        StoryLayout a, b;
        a.build(&doc, {plain}, LayoutEnv());
        b.build(&doc, {fs}, LayoutEnv());
        QCOMPARE(int(a.lineInfo(0).size()), 3);
        for (int i = 0; i < 3; ++i) QCOMPARE(b.lineInfo(0)[i].rect, a.lineInfo(0)[i].rect);
        // Presentations' boxes run over, publications' don't.
        QVERIFY(deck::newPresentation()->slideText);
        QVERIFY(!Document().slideText);
        QString err;
        auto opened = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(opened && opened->slideText, qPrintable(err));
    }

    // A slide's empty placeholder shows the fill its layout gives it (a design's sample picture), whose picture is named by the
    // layout's relationships, not the slide's.
    void emptyPlaceholderShowsItsLayoutsPictureFill()
    {
        Parts p = samplePptx();
        QImage green(4, 4, QImage::Format_RGB32);
        green.fill(QColor(0x10, 0xC0, 0x30));
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        green.save(&buf, "PNG");
        p.data["/ppt/media/image2.png"] = {QStringLiteral("image/png"), png};
        // The layout's rId2 is the green picture; the slide's rId2 is the orange one.
        p.rels << Rel{"/ppt/slideLayouts/slideLayout2.xml", "image", "/ppt/media/image2.png"};
        QString layout = QString::fromUtf8(p.data["/ppt/slideLayouts/slideLayout2.xml"].xml);
        layout.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"9\" name=\"Picture Placeholder 8\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph type=\"pic\" idx=\"13\"/></p:nvPr></p:nvSpPr><p:spPr>")
                                                          + xfrm(6400800, 1825625, 4572000, 3429000)
                                                          + QStringLiteral("<a:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></a:blipFill></p:spPr></p:sp></p:spTree>"));
        p.data["/ppt/slideLayouts/slideLayout2.xml"].xml = layout.toUtf8();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"31\" name=\"Picture Placeholder 30\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph type=\"pic\" idx=\"13\"/></p:nvPr></p:nvSpPr><p:spPr/></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const PictureItem *slot = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("Picture Placeholder 30")) slot = static_cast<const PictureItem *>(it.get());
        QVERIFY(slot && slot->imageId.isEmpty());
        QCOMPARE(int(slot->fill.type), int(Fill::Picture));
        const QImage shown = d->image(slot->fill.imageId);
        QVERIFY(!shown.isNull());
        QCOMPARE(QColor(shown.pixel(1, 1)), QColor(0x10, 0xC0, 0x30));
    }

    // A shape filled with the slide's background (p:sp useBgFill) has the background's fill: the slide's own, else the one its
    // layout or master shows.
    void shapesFilledWithTheBackground()
    {
        const QString panel = QStringLiteral("<p:sp useBgFill=\"1\"><p:nvSpPr><p:cNvPr id=\"40\" name=\"Panel\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>")
                              + xfrm(0, 0, 6096000, 6858000)
                              + QStringLiteral("<a:ln><a:noFill/></a:ln></p:spPr><p:style><a:lnRef idx=\"2\"><a:schemeClr val=\"accent1\"/></a:lnRef><a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef>"
                                               "<a:effectRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:effectRef><a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef></p:style></p:sp>");
        auto panelOf = [](const Document &d, int slide) -> const Item * {
            for (const auto &it : d.pages[slide]->items)
                if (it->name == QLatin1String("Panel")) return it.get();
            return nullptr;
        };
        Parts p = samplePptx();
        // Slide 2 has a background of its own (112233); slide 1 shows its master's.
        for (const char *slide : {"/ppt/slides/slide1.xml", "/ppt/slides/slide2.xml"}) {
            QString xml = QString::fromUtf8(p.data[QString::fromLatin1(slide)].xml);
            xml.replace(QStringLiteral("</p:spTree>"), panel + QStringLiteral("</p:spTree>"));
            p.data[QString::fromLatin1(slide)].xml = xml.toUtf8();
        }
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const Item *own = panelOf(*d, 1), *inherited = panelOf(*d, 0);
        QVERIFY(own && inherited);
        QCOMPARE(int(own->fill.type), int(Fill::Solid));
        QCOMPARE(own->fill.color.resolve(d->colors), QColor(0x11, 0x22, 0x33));
        const MasterPage *layout = d->masterFor(*d->pages[0]);
        Fill shown = layout ? layout->background : Fill();
        for (const MasterPage *m : d->masterChain(layout))
            if (!m->background.isNone()) shown = m->background;
        if (layout && !layout->background.isNone()) shown = layout->background;
        QCOMPARE(int(inherited->fill.type), int(shown.type));
        QVERIFY(!(inherited->fill.type == Fill::Solid && inherited->fill.color.resolve(d->colors) == d->colors.c[Accent1]));
    }

    // Text art (an old file's) is saved as the presentation program keeps the older kind: text warped into its box and stretched
    // to fill it, with the art's letters' look; read again, it is the same art.
    void textArtIsSavedAndReadBack()
    {
        auto d = deck::newPresentation();
        auto art = std::make_shared<TextArtItem>();
        art->name = QStringLiteral("Art");
        art->rect = QRectF(100, 100, 400, 150);
        art->text = QStringLiteral("Hello\nWorld");
        art->font = QStringLiteral("Arial");
        art->bold = true;
        art->size = 40;
        art->transform_ = QStringLiteral("archUp");
        art->align = 0;
        art->fill = Fill::solid(ColorRef::rgb(QColor(0xCC, 0x11, 0x22)));
        art->stroke = Stroke::line(ColorRef::rgb(QColor(0x11, 0x22, 0xCC)), 1.5);
        d->pages[0]->items.push_back(art);
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.filter(QStringLiteral("text art")).isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<a:prstTxWarp prst=\"textArchUp\">")));
        QVERIFY(slide.contains(QStringLiteral("fromWordArt=\"1\"")));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const TextArtItem *read = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->name == QLatin1String("Art") && it->type() == ItemType::TextArt) read = static_cast<const TextArtItem *>(it.get());
        QVERIFY(read);
        QCOMPARE(read->text, art->text);
        QCOMPARE(read->transform_, QStringLiteral("archUp"));
        QCOMPARE(read->font, QStringLiteral("Arial"));
        QVERIFY(read->bold);
        QCOMPARE(read->size, 40.0);
        QCOMPARE(read->align, 0);
        QCOMPARE(read->fill.color.resolve(back->colors), QColor(0xCC, 0x11, 0x22));
        QCOMPARE(read->stroke.color.resolve(back->colors), QColor(0x11, 0x22, 0xCC));
        QVERIFY(std::abs(read->rect.width() - 400) < 0.5 && std::abs(read->rect.height() - 150) < 0.5);
    }

    void headerAndFooterPlaceholders()
    {
        auto d = deck::newPresentation();   // a title slide
        for (int i = 0; i < 2; ++i) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        auto typesOf = [&](int i) {
            QStringList t;
            for (const auto &it : d->pages[i]->items) t << deck::placeholderType(*it);
            return t;
        };
        deck::FooterOptions o;
        o.slideNumber = true;
        o.skipTitleSlide = true;
        deck::applyFooters(*d, o);
        QVERIFY(d->appData.value(QStringLiteral("footers")).toBool());
        QCOMPARE(typesOf(0), (QStringList{"ctrTitle", "subTitle"}));   // not on the title slide
        QCOMPARE(typesOf(1), (QStringList{"title", "obj", "sldNum"}));
        QCOMPARE(typesOf(2), (QStringList{"title", "obj", "sldNum"}));
        // The number is the page number field, in the layout's place and look.
        const auto *num = static_cast<const TextItem *>(d->pages[1]->items.back().get());
        QCOMPARE(num->rect, deck::layoutOfType(*d, QStringLiteral("obj"))->items.back()->rect);
        const QTextDocument *sd = d->storyDoc(num->storyId);
        QCOMPARE(sd->toPlainText(), QString(QChar(QChar::ObjectReplacementCharacter)));
        QCOMPARE(QTextCursor(const_cast<QTextDocument *>(sd)).charFormat().stringProperty(tp::Field), QStringLiteral("page"));
        // Apply to one slide: its own, and what new slides get stays.
        o.date = o.footer = true;
        o.footerText = QStringLiteral("Quarterly review");
        o.skipTitleSlide = false;
        deck::applyFooters(*d, o, d->pages[0].get());
        QCOMPARE(typesOf(0), (QStringList{"ctrTitle", "subTitle", "dt", "ftr", "sldNum"}));
        QCOMPARE(d->storyDoc(static_cast<const TextItem *>(d->pages[0]->items[3].get())->storyId)->toPlainText(), QStringLiteral("Quarterly review"));
        QCOMPARE(typesOf(1), (QStringList{"title", "obj", "sldNum"}));
        const int added = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("titleOnly")));
        QCOMPARE(typesOf(added), (QStringList{"title", "sldNum"}));
        // Apply to all with nothing chosen takes them all away.
        deck::applyFooters(*d, deck::FooterOptions());
        for (int i = 0; i < d->pages.size(); ++i) QVERIFY2(!typesOf(i).contains(QStringLiteral("sldNum")) && !typesOf(i).contains(QStringLiteral("ftr")), qPrintable(typesOf(i).join(',')));
        QVERIFY(!d->appData.value(QStringLiteral("footers")).toBool());
    }

    // A slide's own background, or one for all of them through the master.
    void backgroundsSetAndClear()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::setSlideBackground(*d->pages[1], QColor(0x33, 0x66, 0x99));
        QCOMPARE(d->pages[1]->background, Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99))));
        QVERIFY(d->pages[0]->background.type == Fill::NoFill);
        deck::setAllBackgrounds(*d, QColor(0x11, 0x22, 0x33));
        QVERIFY(d->pages[1]->background.type == Fill::NoFill);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        for (int i = 0; i < 2; ++i) QCOMPARE(Renderer::renderToImage(ctx, i, 0.1).pixelColor(3, 3), QColor(0x11, 0x22, 0x33));
    }

    // Ctrl+F1 collapses the ribbon and brings it back, as its button says.
    void ctrlF1CollapsesTheRibbon()
    {
        DeckWindow w;
        QAction *a = w.act(QStringLiteral("view.collapseRibbon"));
        QVERIFY(a);
        QCOMPARE(a->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_F1));
        const bool was = w.ribbon()->isMinimized();
        a->trigger();
        QCOMPARE(w.ribbon()->isMinimized(), !was);
        a->trigger();
        QCOMPARE(w.ribbon()->isMinimized(), was);
    }

    // The ribbon builds with nothing missing and has the commands of the
    // Home, Insert, Design, and View tabs; each of those tabs gives every
    // control its own KeyTip.
    void ribbonHasTheTabsCommands()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        for (const char *id : {"fmt.bold", "fmt.shadow", "fmt.grow", "fmt.shrink", "fmt.clear", "case.2", "spacing.Tight", "edit.formatPainter", "para.bullets",
                               "para.numbers", "para.indentDec", "para.indentInc", "ls.1.5", "fmt.alignCenter", "tb.dirVertical", "valign.1", "cols.2", "arr.front",
                               "arr.backward", "arr.group", "arr.flipH", "slide.new", "insert.picture", "insert.table", "insert.textBox", "insert.headerFooter",
                               "design.sizeStandard", "design.sizeWidescreen", "design.formatBackground", "view.rulers", "view.gridlines", "view.guides", "view.notes",
                               "view.fit", "view.normal", "view.sorter", "view.slideShow", "zoom.in", "zoom.out", "zoom.100"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QStringList tabs;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
        QVERIFY(tabs.indexOf(QStringLiteral("Insert")) < tabs.indexOf(QStringLiteral("Design")));
        QVERIFY(tabs.indexOf(QStringLiteral("Design")) < tabs.indexOf(QStringLiteral("Slide Show")));
        for (const QString &name : {QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("Design"), QStringLiteral("View")}) {
            QVERIFY2(w.ribbon()->tab(name), qPrintable(name));
            QStringList seen;
            for (const QWidget *c : w.ribbon()->tab(name)->findChildren<QWidget *>()) {
                const QString k = keytip(c);
                if (k.isEmpty()) continue;
                QVERIFY2(k.size() <= 2 && k.indexOf(QRegularExpression(QStringLiteral("[^A-Z0-9]"))) < 0, qPrintable(name + ": " + k));
                QVERIFY2(!seen.contains(k), qPrintable(name + ": two controls have the KeyTip " + k));
                seen << k;
            }
            QVERIFY2(seen.size() > 3, qPrintable(name));
        }
    }

    // A presentation shows no rulers until asked, and no outline around every
    // object; the View tab's check boxes turn the ruler, gridlines, and guides on.
    void normalViewDefaults()
    {
        DeckWindow w;
        QVERIFY(!w.editor()->view.rulers);
        QVERIFY(!w.editor()->view.boundaries);
        QVERIFY(w.canvas()->hRuler()->isHidden() && w.canvas()->vRuler()->isHidden());
        QVERIFY(!w.act("view.rulers")->isChecked() && !w.act("view.gridlines")->isChecked() && !w.act("view.guides")->isChecked());
        w.act("view.rulers")->trigger();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(!w.canvas()->hRuler()->isHidden() && !w.canvas()->vRuler()->isHidden());
        QVERIFY(w.act("view.rulers")->isChecked());
        w.act("view.rulers")->trigger();
        QVERIFY(w.canvas()->hRuler()->isHidden());
        w.act("view.gridlines")->trigger();
        w.act("view.guides")->trigger();
        QVERIFY(w.canvas()->gridlines() && w.canvas()->guides());
        // The boxes are the commands'.
        QCheckBox *ruler = nullptr;
        for (QCheckBox *b : w.ribbon()->findChildren<QCheckBox *>())
            if (b->text() == QStringLiteral("Ruler")) ruler = b;
        QVERIFY(ruler);
        ruler->click();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(ruler->isChecked());
    }

    // A placeholder is outlined (dotted) while it is empty or selected, and a
    // slide's objects aren't outlined the way every object is in the other apps.
    void placeholdersAreOutlinedOnlyWhileEmptyOrSelected()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        auto outlined = [&] {
            const QImage img = w.canvas()->viewport()->grab().toImage();
            const auto *title = static_cast<const TextItem *>(ed->doc()->pages[0]->items[0].get());
            const QPoint at = w.canvas()->pageToView(QPointF(title->rect.center().x(), title->rect.top())).toPoint();
            int marks = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -20; dx < 20; ++dx) {
                    const QColor c = img.pixelColor(at.x() + dx, at.y() + dy);
                    marks += c.lightness() < 235;
                }
            return marks;
        };
        QVERIFY(outlined() > 3);   // empty
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        ed->doc()->storyDoc(title->storyId)->setPlainText(QStringLiteral("Hi"));
        ed->notifyLive();
        QCOMPARE(outlined(), 0);   // text in it, not selected
        ed->select(title->id);
        QVERIFY(outlined() > 3);   // selected
        ed->clearSelection();
        QCOMPARE(outlined(), 0);
    }

    // The status bar: the slide's place, Notes, the views, and a zoom slider
    // with its percent and a fit button.
    void statusBarShowsSlideAndZoom()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        auto *place = w.findChild<QLabel *>(QStringLiteral("slideNumber"));
        QVERIFY(place);
        QCOMPARE(place->text(), QStringLiteral("Slide 1 of 1"));
        w.newSlide();
        QTRY_COMPARE(place->text(), QStringLiteral("Slide 2 of 2"));
        QVERIFY(w.act("view.sorter")->isEnabled());
        QVERIFY(w.act("view.normal")->isChecked());
        // Notes shows and hides the notes below the slide.
        QVERIFY(!w.notesPane()->isHidden());
        QTRY_VERIFY(w.act("view.notes")->isChecked());
        w.act("view.notes")->trigger();
        QVERIFY(w.notesPane()->isHidden());
        QTRY_VERIFY(!w.act("view.notes")->isChecked());
        w.act("view.notes")->trigger();
        QVERIFY(!w.notesPane()->isHidden());
        // Zoom: the slider zooms, the label follows the canvas, and Fit comes back to the window.
        auto *slider = w.findChild<QSlider *>(QStringLiteral("zoomSlider"));
        auto *percent = w.findChild<QLabel *>(QStringLiteral("zoomPercent"));
        QVERIFY(slider && percent);
        slider->setValue(0);
        QCOMPARE(w.canvas()->zoom(), 1.0);
        QCOMPARE(percent->text(), QStringLiteral("100%"));
        w.canvas()->setZoom(0.5);
        QCOMPARE(percent->text(), QStringLiteral("50%"));
        w.act("view.fit")->trigger();
        QVERIFY(w.canvas()->fitMode() == Canvas::Fit::WholePage);
        w.act("zoom.200")->trigger();
        QCOMPARE(w.canvas()->zoom(), 2.0);
    }

    // The slide size from the window is one step that undoes, and the
    // thumbnails take the slides' new shape.
    void slideSizeFromTheWindow()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const QRectF before = ed->doc()->pages[0]->items[0]->rect;
        w.setSlideSize(QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(ed->doc()->pageSize(), QSizeF(720, 540));
        QVERIFY(ed->doc()->pages[0]->items[0]->rect != before);
        QTRY_COMPARE(w.slidesPane()->iconSize(), QSize(168, 126));
        QTRY_VERIFY(w.act("design.sizeStandard")->isChecked());
        QVERIFY(!w.act("design.sizeWidescreen")->isChecked());
        ed->undo();
        QCOMPARE(ed->doc()->pageSize(), QSizeF(960, 540));
        QCOMPARE(ed->doc()->pages[0]->items[0]->rect, before);
        QTRY_VERIFY(w.act("design.sizeWidescreen")->isChecked());
    }

    // Format Background and Header & Footer from the window are steps that undo.
    void backgroundAndFooterFromTheWindow()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.setBackground(QColor(0x33, 0x66, 0x99), false);
        QCOMPARE(ed->doc()->pages[1]->background, Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99))));
        QVERIFY(ed->doc()->pages[0]->background.type == Fill::NoFill);
        ed->undo();
        QVERIFY(ed->doc()->pages[1]->background.type == Fill::NoFill);
        w.setBackground(QColor(0x11, 0x22, 0x33), true);
        QCOMPARE(ed->doc()->masters[0]->background, Fill::solid(ColorRef::rgb(QColor(0x11, 0x22, 0x33))));
        ed->undo();
        QCOMPARE(ed->doc()->masters[0]->background, Fill::solid(ColorRef::rgb(Qt::white)));

        deck::FooterOptions o;
        o.slideNumber = true;
        o.skipTitleSlide = true;
        w.applyFooters(o, true);
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), 2);
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), 3);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), 2);
        QVERIFY(!ed->doc()->appData.value(QStringLiteral("footers")).toBool());
    }

    // The Header & Footer dialog hands back what was chosen and which button.
    void headerFooterDialogReturnsTheChoice()
    {
        deck::FooterOptions o;
        o.slideNumber = true;
        o.footerText = QStringLiteral("Quarterly");
        HeaderFooterDialog dlg(o);
        QVERIFY(dlg.options().slideNumber && !dlg.options().date && !dlg.options().footer);
        QVERIFY(!dlg.findChild<QLineEdit *>(QStringLiteral("footerText"))->isEnabled());   // until Footer is checked
        dlg.findChild<QCheckBox *>(QStringLiteral("footer"))->setChecked(true);
        dlg.findChild<QCheckBox *>(QStringLiteral("date"))->setChecked(true);
        dlg.findChild<QCheckBox *>(QStringLiteral("skipTitle"))->setChecked(true);
        dlg.findChild<QLineEdit *>(QStringLiteral("footerText"))->setText(QStringLiteral("Q3 review"));
        const deck::FooterOptions got = dlg.options();
        QVERIFY(got.date && got.slideNumber && got.footer && got.skipTitleSlide);
        QCOMPARE(got.footerText, QStringLiteral("Q3 review"));
        dlg.findChild<QPushButton *>(QStringLiteral("applyToAll"))->click();
        QCOMPARE(dlg.result(), int(QDialog::Accepted));
        QVERIFY(dlg.applyToAll());
        HeaderFooterDialog one(o);
        one.findChild<QPushButton *>(QStringLiteral("apply"))->click();
        QCOMPARE(one.result(), int(QDialog::Accepted));
        QVERIFY(!one.applyToAll());
    }

    // The dialog gives back what it doesn't show as it was: a fixed date's text, the date's field, and that the date or the number is the text typed in
    // the master, so that Apply to All with only the footer's text changed leaves them.
    void headerFooterDialogKeepsWhatItDoesNotShow()
    {
        deck::FooterOptions o;
        o.date = true;
        o.slideNumber = true;
        o.footer = true;
        o.footerText = QStringLiteral("Quarterly");
        o.dateText = QStringLiteral("May 19, 2003");
        o.dateCode = QStringLiteral("datetime:M/d/yyyy");
        o.dateFromMaster = true;
        o.numberFromMaster = true;
        HeaderFooterDialog dlg(o);
        dlg.findChild<QLineEdit *>(QStringLiteral("footerText"))->setText(QStringLiteral("Q3 review"));
        const deck::FooterOptions got = dlg.options();
        QCOMPARE(got.footerText, QStringLiteral("Q3 review"));
        QVERIFY(got.date && got.slideNumber && got.footer);
        QCOMPARE(got.dateText, o.dateText);
        QCOMPARE(got.dateCode, o.dateCode);
        QVERIFY(got.dateFromMaster && got.numberFromMaster);
        // What it does show it changes: the date turned off.
        dlg.findChild<QCheckBox *>(QStringLiteral("date"))->setChecked(false);
        QVERIFY(!dlg.options().date);
    }

    // A gallery's pictures are drawn when it is first painted (the ones in the ribbon's menus are built with the window and shown much later).
    void layoutGalleryDrawsItsPicturesWhenFirstPainted()
    {
        DeckWindow w;
        LayoutGallery g(w.editor());
        g.resize(g.sizeHint());
        const QImage img = g.grab().toImage();
        // (each picture is a white slide; the gallery's own ground is not white)
        int white = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (img.pixelColor(x, y) == QColor(Qt::white)) ++white;
        QVERIFY2(white > 9 * 1000, qPrintable(QString::number(white)));
    }

    // New Slide's gallery shows each layout as a picture and says which was chosen.
    void layoutGalleryShowsEachLayout()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        LayoutGallery g(ed);
        QCOMPARE(g.count(), 9);
        QCOMPARE(g.typeAt(0), QStringLiteral("title"));
        QCOMPARE(g.typeAt(8), QStringLiteral("picTx"));
        QCOMPARE(layoutPicture(*ed->doc(), *deck::layoutOfType(*ed->doc(), QStringLiteral("obj")), 112).size(), QSize(112, 63));
        auto busy = [&](const QString &type) {
            const QImage img = layoutPicture(*ed->doc(), *deck::layoutOfType(*ed->doc(), type), 112);
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (img.pixelColor(x, y) != QColor(Qt::white)) return true;
            return false;
        };
        QVERIFY(busy(QStringLiteral("obj")));
        QVERIFY(busy(QStringLiteral("twoObj")));
        QVERIFY(!busy(QStringLiteral("blank")));
        QSignalSpy chosen(&g, &LayoutGallery::chosen);
        QTest::keyClick(&g, Qt::Key_Right);
        QTest::keyClick(&g, Qt::Key_Right);
        QTest::keyClick(&g, Qt::Key_Down);
        QTest::keyClick(&g, Qt::Key_Return);
        QCOMPARE(chosen.count(), 1);
        QCOMPARE(chosen.first().first().toString(), g.typeAt(4));   // the middle of the second row
    }

    // The layouts gallery in the New Slide menu makes a slide with the layout
    // chosen, and the one in the Layout menu changes the slide's.
    void layoutMenusUseTheGallery()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        // In the order the ribbon built them: Home's New Slide and Layout, then Insert's New Slide.
        QList<LayoutGallery *> galleries;
        for (QMenu *m : w.findChildren<QMenu *>())
            if (auto *g = m->findChild<LayoutGallery *>()) galleries << g;
        QCOMPARE(galleries.size(), 3);
        LayoutGallery *newSlide = galleries.at(0), *layout = galleries.at(1);
        for (int i = 0; i < 6; ++i) QTest::keyClick(newSlide, Qt::Key_Right);
        QTest::keyClick(newSlide, Qt::Key_Return);   // Title Only
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[1])), QStringLiteral("titleOnly"));
        QCOMPARE(ed->currentPage(), 1);
        QTest::keyClick(layout, Qt::Key_Home);
        QTest::keyClick(layout, Qt::Key_Return);   // Title Slide
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[1])), QStringLiteral("title"));
        QCOMPARE(int(ed->doc()->pages.size()), 2);
    }

    // Insert > Pictures centers the picture on the slide at its natural size, one
    // step that undoes; Insert > Table puts a table there too.
    void insertPictureAndTable()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(200, 100, QImage::Format_RGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(dir.filePath(QStringLiteral("red.png"))));
        QFile text(dir.filePath(QStringLiteral("notes.txt")));
        QVERIFY(text.open(QIODevice::WriteOnly));
        text.write("not a picture");
        text.close();
        DeckWindow w;
        Editor *ed = w.editor();
        const int before = int(ed->doc()->pages[0]->items.size());
        QVERIFY(!w.insertPicture(dir.filePath(QStringLiteral("notes.txt"))));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        QVERIFY(w.insertPicture(dir.filePath(QStringLiteral("red.png"))));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        const auto *pic = dynamic_cast<const PictureItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(480 - 75, 270 - 37.5, 150, 75));
        QCOMPARE(ed->selection(), QStringList{pic->id});
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        // Dropped on the slide, it goes where it was dropped.
        Q_EMIT w.canvas()->insertFilesWanted({dir.filePath(QStringLiteral("red.png"))}, QPointF(100, 50));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        QCOMPARE(ed->doc()->pages[0]->items.back()->rect.topLeft(), QPointF(100, 50));
        ed->undo();

        w.insertTable(3, 4);
        const auto *table = dynamic_cast<const TableItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(table);
        QCOMPARE(table->rows, 3);
        QCOMPARE(table->cols, 4);
        QCOMPARE(table->rect.center(), QPointF(480, 270));
        QCOMPARE(QTextCursor(ed->doc()->storyDoc(table->cells[0].storyId)).charFormat().fontPointSize(), 18.0);
    }

    void duplicateSlideCopiesNotes()
    {
        auto d = deck::newPresentation();
        d->storyDoc(d->pages[0]->notesStoryId)->setPlainText(QStringLiteral("Say hello"));
        const int i = deck::duplicateSlide(*d, 0);
        QCOMPARE(i, 1);
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Say hello"));
        QVERIFY(d->pages[1]->notesStoryId != d->pages[0]->notesStoryId);
        QVERIFY(d->pages[1]->items[0]->id != d->pages[0]->items[0]->id);
    }

private:
    // What a menu offers, by id: each command by its action's id (a spelling suggestion by its words), each submenu as [its title],
    // and what is in it with the title and a colon before.
    static QStringList menuIds(const QMenu *menu, const QString &prefix = QString())
    {
        QStringList out;
        for (const QAction *a : menu->actions()) {
            if (a->isSeparator()) continue;
            if (a->menu()) {
                out << prefix + QStringLiteral("[%1]").arg(a->text());
                out << menuIds(a->menu(), a->text() + QLatin1Char(':'));
            } else {
                out << prefix + (a->objectName().isEmpty() ? a->text() : a->objectName());
            }
        }
        return out;
    }
    // The ids of the slide's menu; with JP_SHOT_DIR set it is also opened and a picture of it saved under `shotName`.
    static QStringList slideMenu(DeckWindow &w, const QString &shotName = QString())
    {
        std::unique_ptr<QMenu> menu(w.slideContextMenu());
        if (!shotName.isEmpty() && !qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            menu->popup(w.canvas()->mapToGlobal(QPoint(380, 260)));
            QApplication::processEvents();
            shot(menu.get(), shotName);
            menu->close();
        }
        return menuIds(menu.get());
    }
    static TextItem *placeholderOfSlide(const Document &d, int i, int n) { return static_cast<TextItem *>(d.pages[i]->items[n].get()); }
    // A deck with its words in the places Find has to look: slide 1 (a title slide) has "Year review" and a subtitle;
    // slide 2 a body with "review" twice and notes with it once; slide 3 "Reviews and previews", a shape saying "Peer review", and a table
    // whose first cell says "Review".
    static void fillFindDeck(DeckWindow &w)
    {
        Editor *ed = w.editor();
        fillSlides(w, {QStringLiteral("Year review"), QStringLiteral("Profit"), QStringLiteral("Plans")},
                   {{QStringLiteral("Sub")}, {QStringLiteral("We review profit and Review again")}, {QStringLiteral("Reviews and previews")}});
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(d->pages[1]->notesStoryId)).insertText(QStringLiteral("Do not review the numbers"));
        auto shape = std::make_shared<ShapeItem>();
        shape->shape = QStringLiteral("rect");
        shape->rect = QRectF(100, 380, 200, 80);
        shape->storyId = d->createStory(QStringLiteral("Peer review"));
        d->pages[2]->items.push_back(shape);
        ed->setCurrentPage(2);
        w.insertTable(1, 2);
        auto *table = static_cast<TableItem *>(d->pages[2]->items.back().get());
        QTextCursor(d->storyDoc(table->cells[0].storyId)).insertText(QStringLiteral("Review"));
        QTextCursor(d->storyDoc(table->cells[1].storyId)).insertText(QStringLiteral("none"));
        ed->clearSelection();
        ed->setCurrentPage(0);
        ed->notifyLive();
        ed->undoStack()->clear();
    }
    // Where Find Next has put the match: "slide:text@start" in a box, or the same "in the notes".
    static QString matchShown(DeckWindow &w)
    {
        Editor *ed = w.editor();
        if (ed->isEditingText())
            return QStringLiteral("%1:%2@%3").arg(ed->currentPage() + 1).arg(ed->cursor().selectedText()).arg(ed->cursor().selectionStart());
        const QTextCursor c = w.notesPane()->textCursor();
        return QStringLiteral("%1:%2@%3 in the notes").arg(ed->currentPage() + 1).arg(c.selectedText()).arg(c.selectionStart());
    }
    static QStringList findAll(DeckWindow &w, int steps)
    {
        QStringList seen;
        for (int i = 0; i < steps; ++i) {
            w.findPane()->findNext();
            seen << matchShown(w);
        }
        return seen;
    }
    static QString storyText(DeckWindow &w, const QString &storyId) { return w.editor()->doc()->storyDoc(storyId)->toPlainText(); }

private Q_SLOTS:
    // Right-click menu of the slide: nothing selected offers the slide's commands, an object offers its own, text being typed in
    // offers the text's.
    void slideMenuOffersWhatTheSelectionCanDo()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        QStringList ids = slideMenu(w, QStringLiteral("menu-empty-slide"));
        for (const char *id : {"edit.paste", "slide.reset", "design.formatBackground", "view.rulers", "comment.new", "[Layout]", "[Grid and Guides]",
                               "Grid and Guides:view.gridlines", "Grid and Guides:view.guides"})
            QVERIFY2(ids.contains(QLatin1String(id)), qPrintable(QLatin1String(id) + QLatin1String(" in ") + ids.join(QLatin1Char(' '))));
        for (const char *id : {"edit.cut", "edit.copy", "edit.text", "arr.group", "insert.link", "fmt.fontDialog"}) QVERIFY2(!ids.contains(QLatin1String(id)), id);

        // A placeholder: clipboard, delete, edit text, order, align text, link, comment; no slide commands.
        const QString titleId = placeholderOfSlide(*ed->doc(), 0, 0)->id;
        ed->select(titleId);
        ids = slideMenu(w, QStringLiteral("menu-object"));
        for (const char *id : {"edit.cut", "edit.copy", "edit.paste", "edit.delete", "edit.text", "[Order]", "Order:arr.front", "Order:arr.forward", "Order:arr.backward",
                               "Order:arr.back", "[Alignment]", "Alignment:fmt.alignLeft", "Alignment:fmt.alignCenter", "Alignment:fmt.alignRight", "Alignment:fmt.justify",
                               "insert.link", "comment.new"})
            QVERIFY2(ids.contains(QLatin1String(id)), qPrintable(QLatin1String(id) + QLatin1String(" in ") + ids.join(QLatin1Char(' '))));
        for (const char *id : {"arr.group", "arr.ungroup", "[Layout]", "slide.reset", "design.formatBackground"}) QVERIFY2(!ids.contains(QLatin1String(id)), id);

        // Two objects can be grouped; the group can be taken apart.
        ed->select(QStringList{titleId, placeholderOfSlide(*ed->doc(), 0, 1)->id});
        ids = slideMenu(w);
        QVERIFY(ids.contains(QStringLiteral("arr.group")) && !ids.contains(QStringLiteral("arr.ungroup")));
        w.act(QStringLiteral("arr.group"))->trigger();
        QCOMPARE(ed->selectionKind(), QStringLiteral("group"));
        ids = slideMenu(w);
        QVERIFY(ids.contains(QStringLiteral("arr.ungroup")) && !ids.contains(QStringLiteral("arr.group")));
        ed->undo();

        // A picture has no words to edit or align.
        QTemporaryDir dir;
        QImage img(40, 30, QImage::Format_ARGB32);
        img.fill(Qt::red);
        QVERIFY(img.save(dir.filePath(QStringLiteral("red.png"))));
        QVERIFY(w.insertPicture(dir.filePath(QStringLiteral("red.png"))));
        QCOMPARE(ed->selectionKind(), QStringLiteral("picture"));
        ids = slideMenu(w);
        QVERIFY(ids.contains(QStringLiteral("edit.cut")) && ids.contains(QStringLiteral("[Order]")) && ids.contains(QStringLiteral("insert.link")));
        QVERIFY(!ids.contains(QStringLiteral("edit.text")) && !ids.contains(QStringLiteral("[Alignment]")));

        // A table can be typed in.
        w.insertTable(2, 2);
        QCOMPARE(ed->selectionKind(), QStringLiteral("table"));
        QVERIFY(slideMenu(w).contains(QStringLiteral("edit.text")));

        // Text being typed in: the text's commands, not the object's.
        ed->beginTextEdit(titleId);
        ids = slideMenu(w, QStringLiteral("menu-text"));
        for (const char *id : {"edit.cut", "edit.copy", "edit.paste", "fmt.fontDialog", "para.dialog", "para.bullets", "para.numbers", "insert.link", "review.thesaurus"})
            QVERIFY2(ids.contains(QLatin1String(id)), qPrintable(QLatin1String(id) + QLatin1String(" in ") + ids.join(QLatin1Char(' '))));
        for (const char *id : {"edit.delete", "edit.text", "[Order]", "[Layout]"}) QVERIFY2(!ids.contains(QLatin1String(id)), id);
    }

    // Every command in the menu is the very action the ribbon has (so it is enabled and checked as the ribbon's is, and counts the
    // same); Reset Slide is the ribbon's Reset under a clearer name, and calls it; the Text Fit choices are the menu's own.
    void slideMenuCommandsAreTheRibbonsActions()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        std::function<void(const QMenu *)> check = [&](const QMenu *menu) {
            for (QAction *a : menu->actions()) {
                if (a->menu()) {
                    check(a->menu());
                } else if (!a->isSeparator() && !a->objectName().isEmpty() && a->objectName() != QLatin1String("slide.reset") && !a->objectName().startsWith(QLatin1String("text.fit"))) {
                    QVERIFY2(w.act(a->objectName()), qPrintable(a->objectName()));
                    QCOMPARE(a, w.act(a->objectName()));
                }
            }
        };
        auto checkMenu = [&] {
            std::unique_ptr<QMenu> menu(w.slideContextMenu());
            QVERIFY(menu->actions().size() > 3);
            check(menu.get());
        };
        checkMenu();
        ed->select(placeholderOfSlide(*ed->doc(), 0, 0)->id);
        checkMenu();
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 0, 0)->id);
        checkMenu();
        // Reset Slide puts the placeholders back where the layout has them.
        ed->endTextEdit();
        ed->clearSelection();
        const QRectF home = placeholderOfSlide(*ed->doc(), 0, 0)->rect;
        ed->change(QStringLiteral("Move"), [&] { placeholderOfSlide(*ed->doc(), 0, 0)->rect.translate(40, 30); });
        QVERIFY(placeholderOfSlide(*ed->doc(), 0, 0)->rect != home);
        std::unique_ptr<QMenu> menu(w.slideContextMenu());
        QAction *reset = nullptr;
        for (QAction *a : menu->actions())
            if (a->text() == QLatin1String("Reset Slide")) reset = a;
        QVERIFY(reset);
        QCOMPARE(reset->objectName(), QStringLiteral("slide.reset"));
        reset->trigger();
        QCOMPARE(placeholderOfSlide(*ed->doc(), 0, 0)->rect, home);
        // The menu's Edit Text is the action that types in the selected box.
        ed->select(placeholderOfSlide(*ed->doc(), 0, 1)->id);
        std::unique_ptr<QMenu> objectMenu(w.slideContextMenu());
        QVERIFY(objectMenu->actions().contains(w.act(QStringLiteral("edit.text"))));
        QVERIFY(!ed->isEditingText());
        w.act(QStringLiteral("edit.text"))->trigger();
        QVERIFY(ed->isEditingText());
        QCOMPARE(ed->textTarget().itemId, placeholderOfSlide(*ed->doc(), 0, 1)->id);
    }

    // A right click on the slide opens the menu for what is there; a click in the layout gallery of its Layout submenu changes the layout and closes
    // every menu.
    void rightClickingTheSlideOpensItsMenu()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        QStringList ids;
        rightClick(w.canvas()->viewport(), QPoint(300, 200), [&](QMenu *m) { ids = menuIds(m); });
        QVERIFY2(ids.contains(QStringLiteral("slide.reset")), qPrintable(ids.join(QLatin1Char(' '))));
        ed->select(placeholderOfSlide(*ed->doc(), 0, 0)->id);
        ids.clear();
        rightClick(w.canvas()->viewport(), QPoint(300, 200), [&](QMenu *m) { ids = menuIds(m); });
        QVERIFY2(ids.contains(QStringLiteral("edit.cut")), qPrintable(ids.join(QLatin1Char(' '))));
        // The same menu is not opened over itself.
        ed->clearSelection();
        bool again = false;
        QTimer::singleShot(0, [&] {
            untilDone([&] {
                auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
                if (!m) return false;
                w.showSlideContextMenu(QPoint(10, 10));   // returns at once
                again = true;
                m->close();
                return true;
            });
        });
        w.showSlideContextMenu(w.canvas()->mapToGlobal(QPoint(300, 200)));
        QVERIFY(again);
        // Layout: choosing one changes the slide and closes the menu.
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[0])), QStringLiteral("title"));
        bool chose = false;
        QTimer::singleShot(0, [&] {
            untilDone([&] {
                auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
                if (!m) return false;
                for (QAction *a : m->actions())
                    if (a->menu() && a->text() == QLatin1String("Layout"))
                        if (auto *wa = qobject_cast<QWidgetAction *>(a->menu()->actions().value(0)))
                            if (auto *gallery = qobject_cast<LayoutGallery *>(wa->defaultWidget())) {
                                gallery->reload();
                                Q_EMIT gallery->chosen(QStringLiteral("titleOnly"));
                                chose = true;
                            }
                if (!chose) m->close();
                return true;
            });
        });
        w.showSlideContextMenu(w.canvas()->mapToGlobal(QPoint(300, 200)));   // comes back only once every menu is closed
        QVERIFY(chose);
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[0])), QStringLiteral("titleOnly"));
    }

    // Shift+F10 and the Menu key ask for the menu where the selection is: under the text cursor, else in the middle of the selected object,
    // else in the middle of the view.
    void shiftF10AndTheMenuKeyOpenTheMenuAtTheSelection()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Canvas *cv = w.canvas();
        cv->setFocus();
        QObject::disconnect(cv, &Canvas::contextMenuWanted, &w, nullptr);   // only to see where the menu would open
        QSignalSpy asked(cv, &Canvas::contextMenuWanted);
        auto where = [&](Qt::Key key, Qt::KeyboardModifiers mods) {
            asked.clear();
            QTest::keyClick(cv, key, mods);
            return asked.count() == 1 ? asked.takeFirst().at(0).toPoint() : QPoint(-9999, -9999);
        };
        TextItem *title = placeholderOfSlide(*ed->doc(), 0, 0);
        ed->select(title->id);
        for (const auto &[key, mods] : {std::pair{Qt::Key_F10, Qt::KeyboardModifiers(Qt::ShiftModifier)}, std::pair{Qt::Key_Menu, Qt::KeyboardModifiers()}}) {
            const QPoint at = where(key, mods);
            QVERIFY2(cv->screenRect(title->bounds()).contains(at), qPrintable(QStringLiteral("%1,%2").arg(at.x()).arg(at.y())));
        }
        // The platform's own request for the menu from the keyboard comes to the same place.
        asked.clear();
        QContextMenuEvent keyboard(QContextMenuEvent::Keyboard, QPoint(1, 1), cv->mapToGlobal(QPoint(1, 1)));
        QApplication::sendEvent(cv->viewport(), &keyboard);
        QCOMPARE(asked.count(), 1);
        QVERIFY(cv->screenRect(title->bounds()).contains(asked.first().at(0).toPoint()));
        // A mouse request stays where the mouse is.
        asked.clear();
        QContextMenuEvent mouse(QContextMenuEvent::Mouse, QPoint(7, 8), cv->viewport()->mapToGlobal(QPoint(7, 8)));
        QApplication::sendEvent(cv->viewport(), &mouse);
        QCOMPARE(asked.first().at(0).toPoint(), cv->viewport()->mapToGlobal(QPoint(7, 8)));
        // Typing in the box: just under the caret.
        ed->beginTextEdit(title->id);
        const QPoint under = where(Qt::Key_F10, Qt::ShiftModifier);
        const QRect caret = cv->caretScreenRect(ed->cursor().position());
        QVERIFY2(under.y() >= caret.bottom() && under.y() <= caret.bottom() + 8 && under.x() >= caret.left() - 2 && under.x() <= caret.right() + 2,
                 qPrintable(QStringLiteral("%1,%2 for %3,%4 %5x%6").arg(under.x()).arg(under.y()).arg(caret.x()).arg(caret.y()).arg(caret.width()).arg(caret.height())));
        // Nothing selected: somewhere in the view.
        ed->endTextEdit();
        ed->clearSelection();
        const QPoint mid = where(Qt::Key_Menu, Qt::NoModifier);
        QVERIFY(cv->viewport()->rect().contains(cv->viewport()->mapFromGlobal(mid)));
        // The menu itself: a key press opens it, and it comes back when it is closed.
        QObject::connect(cv, &Canvas::contextMenuWanted, &w, &DeckWindow::showSlideContextMenu);
        bool opened = false;
        QTimer::singleShot(0, [&] {
            untilDone([&] {
                auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
                if (!m) return false;
                opened = true;
                m->close();
                return true;
            });
        });
        QTest::keyClick(cv, Qt::Key_F10, Qt::ShiftModifier);
        QVERIFY(opened);
    }

    // Right-clicking a misspelled word being typed offers the dictionary's suggestions first; choosing one replaces the word.
    void slideMenuOffersSpellingSuggestions()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        const QString titleId = placeholderOfSlide(*ed->doc(), 0, 0)->id, storyId = placeholderOfSlide(*ed->doc(), 0, 0)->storyId;
        QTextCursor(ed->doc()->storyDoc(storyId)).insertText(QStringLiteral("This is teh test"));
        ed->beginTextEdit(titleId, 10);
        slideMenu(w, QStringLiteral("menu-spelling"));
        std::unique_ptr<QMenu> menu(w.slideContextMenu());
        const QList<QAction *> actions = menu->actions();
        QVERIFY(actions.size() > 6);
        const QStringList suggestions = spellingSuggestions(QStringLiteral("teh"));
        QVERIFY(suggestions.contains(actions.first()->text()));
        QVERIFY(actions.first()->font().bold());
        QStringList texts;
        for (const QAction *a : actions) texts << a->text();
        QVERIFY(texts.contains(QStringLiteral("Ignore All")) && texts.contains(QStringLiteral("Add to Dictionary")));
        QVERIFY(texts.indexOf(QStringLiteral("Ignore All")) < texts.indexOf(QStringLiteral("Cut")));
        const QString chosen = actions.first()->text();
        actions.first()->trigger();
        QCOMPARE(storyText(w, storyId), QStringLiteral("This is %1 test").arg(chosen));
        // Off a misspelled word there are none.
        ed->beginTextEdit(titleId, 2);
        std::unique_ptr<QMenu> plain(w.slideContextMenu());
        QCOMPARE(plain->actions().first()->objectName(), QStringLiteral("edit.cut"));
    }

    // Home > Editing has Find, Replace, and Select, with Ctrl+F and Ctrl+H; Select offers Select All and Select Objects.
    void homeTabHasFindReplaceAndSelect()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QCOMPARE(w.act(QStringLiteral("edit.find"))->shortcut(), QKeySequence(QKeySequence::Find));
        QCOMPARE(w.act(QStringLiteral("edit.replace"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_H));
        QToolButton *selectButton = nullptr;
        int buttons = 0;
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>()) {
            if (b->defaultAction() == w.act(QStringLiteral("edit.find")) || b->defaultAction() == w.act(QStringLiteral("edit.replace"))) ++buttons;
            if (b->text() == QLatin1String("Select") && b->menu()) selectButton = b;
        }
        QCOMPARE(buttons, 2);
        QVERIFY(selectButton);
        QVERIFY(selectButton->menu()->actions().contains(w.act(QStringLiteral("edit.selectAll"))));
        QVERIFY(selectButton->menu()->actions().contains(w.act(QStringLiteral("edit.selectObjects"))));
        // The commands open the pane, with the cursor in the box they are for.
        QVERIFY(!w.findPane());
        w.show();
        w.act(QStringLiteral("edit.find"))->trigger();
        QVERIFY(w.findPane() && w.findPane()->isVisible());
        QVERIFY(w.findPane()->speakerNotes()->isChecked());
        QVERIFY(!w.findPane()->matchCase()->isChecked() && !w.findPane()->wholeWords()->isChecked());
        w.act(QStringLiteral("edit.replace"))->trigger();
        QVERIFY(w.findPane()->isVisible());
        // Select Objects is the pointer again, with no text being typed in.
        Editor *ed = w.editor();
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 0, 0)->id);
        w.act(QStringLiteral("edit.selectObjects"))->trigger();
        QVERIFY(!ed->isEditingText());
        QCOMPARE(ed->tool(), Tool::Select);
    }

    // The Paste button has a menu with Keep Text Only and Paste Special (Ctrl+Alt+V), as in the other programs; Paste Special lists the forms the clipboard can go in as,
    // and pastes the one chosen: source formatting, text only, or a picture.
    void pasteSpecialPastesTheFormChosen()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QCOMPARE(w.act(QStringLiteral("edit.pasteSpecial"))->text(), QStringLiteral("Paste Special…"));
        QCOMPARE(w.act(QStringLiteral("edit.pasteSpecial"))->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_V).toString(QKeySequence::NativeText));
        QCOMPARE(w.act(QStringLiteral("edit.pasteText"))->text(), QStringLiteral("Keep Text Only"));
        bool inMenu = false;
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->defaultAction() == w.act(QStringLiteral("edit.paste")) && b->menu())
                inMenu = b->menu()->actions().contains(w.act(QStringLiteral("edit.pasteSpecial"))) && b->menu()->actions().contains(w.act(QStringLiteral("edit.pasteText")));
        QVERIFY(inMenu);
        Editor *ed = w.editor();
        auto *mime = new QMimeData;
        mime->setHtml(QStringLiteral("<i>Slanted</i>"));
        mime->setText(QStringLiteral("Slanted"));
        QApplication::clipboard()->setMimeData(mime);
        auto *title = placeholderOfSlide(*ed->doc(), 0, 0);
        QTextDocument *story = ed->doc()->storyDoc(title->storyId);
        story->clear();
        ed->beginTextEdit(title->id);
        auto choose = [&](int row) {
            withDialog([&w] { w.act(QStringLiteral("edit.pasteSpecial"))->trigger(); }, QStringLiteral("pasteSpecialDialog"), [row](QDialog *d) {
                auto *list = d->findChild<QListWidget *>(QStringLiteral("pasteSpecialList"));
                QVERIFY(list);
                QCOMPARE(list->count(), 3);
                list->setCurrentRow(row);
            });
        };
        auto slanted = [&](int at) {
            QTextCursor c(story);
            c.setPosition(at);
            return c.charFormat().fontItalic();
        };
        choose(1);
        QCOMPARE(story->toPlainText(), QStringLiteral("Slanted"));
        QVERIFY(!slanted(3));
        choose(0);
        QCOMPARE(story->toPlainText(), QStringLiteral("SlantedSlanted"));
        QVERIFY(!slanted(3));
        QVERIFY(slanted(11));
        // A picture on the clipboard goes on the slide as a picture.
        QImage img(40, 30, QImage::Format_ARGB32);
        img.fill(Qt::red);
        auto *pictureMime = new QMimeData;
        pictureMime->setImageData(img);
        QApplication::clipboard()->setMimeData(pictureMime);
        auto pictures = [&] {
            int n = 0;
            for (const auto &it : ed->doc()->pages[0]->items)
                if (it->type() == ItemType::Picture) ++n;
            return n;
        };
        QCOMPARE(pictures(), 0);
        choose(2);
        QCOMPARE(pictures(), 1);
    }

    // View > Window has New Window and Switch Windows, as in the other programs. New Window opens the presentation's file in a second window (a new, empty presentation when there is
    // no file yet), and Switch Windows lists the open windows.
    void newWindowOpensTheFileAgainAndSwitchWindowsListsThem()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("talk.pptx"));
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QCOMPARE(w.act(QStringLiteral("win.new"))->text(), QStringLiteral("New Window"));
        w.newSlide();
        w.newSlide();
        QVERIFY(w.saveTo(path));
        auto deckWindows = [&w] {
            QList<DeckWindow *> out;
            for (AppWindow *a : AppWindow::windows())
                if (auto *d = qobject_cast<DeckWindow *>(a); d && d != &w) out << d;
            return out;
        };
        QVERIFY(deckWindows().isEmpty());
        w.act(QStringLiteral("win.new"))->trigger();
        QList<DeckWindow *> others = deckWindows();
        QCOMPARE(others.size(), 1);
        QPointer<DeckWindow> second = others.first();
        QCOMPARE(second->editor()->filePath(), w.editor()->filePath());
        QCOMPARE(second->editor()->doc()->pages.size(), 3);
        // Switch Windows lists both, by their titles.
        QMenu *menu = nullptr;
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->text() == QLatin1String("Switch Windows") && b->menu()) menu = b->menu();
        QVERIFY(menu);
        Q_EMIT menu->aboutToShow();
        QStringList titles;
        for (QAction *a : menu->actions()) titles << a->text();
        QVERIFY2(titles.contains(w.windowTitle()) && titles.contains(second->windowTitle()), qPrintable(titles.join(QLatin1Char('|'))));
        QCOMPARE(titles.size(), 2);
        second->close();
        QTRY_VERIFY(second.isNull());
        // With no file yet, a new window is a new presentation.
        DeckWindow fresh;
        fresh.show();
        fresh.act(QStringLiteral("win.new"))->trigger();
        QList<DeckWindow *> made;
        for (DeckWindow *d : deckWindows())
            if (d != &fresh) made << d;
        QCOMPARE(made.size(), 1);
        QVERIFY(made.first()->editor()->filePath().isEmpty());
        made.first()->close();
    }

    // The Help tab has Help, Keyboard Shortcuts, What's New, and Feedback: the last two open their pages, and Keyboard Shortcuts opens the Help pane on the shortcuts topic.
    void theHelpTabHasHelpShortcutsWhatsNewAndFeedback()
    {
        DeckWindow w;
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.ribbon()->tab(QStringLiteral("Help")));
        QCOMPARE(w.act(QStringLiteral("help.shortcuts"))->text(), QStringLiteral("Keyboard Shortcuts"));
        QCOMPARE(w.act(QStringLiteral("help.whatsNew"))->text(), QStringLiteral("What's New"));
        QCOMPARE(w.act(QStringLiteral("help.feedback"))->text(), QStringLiteral("Feedback"));
        const auto saved = help::openUrl;
        QList<QUrl> opened;
        help::openUrl = [&](const QUrl &u) {
            opened << u;
            return true;
        };
        w.act(QStringLiteral("help.feedback"))->trigger();
        w.act(QStringLiteral("help.whatsNew"))->trigger();
        help::openUrl = saved;
        QCOMPARE(opened, (QList<QUrl>{help::feedbackUrl(), help::whatsNewUrl()}));
        QDockWidget *dock = w.findChild<QDockWidget *>(QStringLiteral("helpDock"));
        QVERIFY(!dock || !dock->isVisible());
        w.act(QStringLiteral("help.shortcuts"))->trigger();
        dock = w.findChild<QDockWidget *>(QStringLiteral("helpDock"));
        QVERIFY(dock && dock->isVisible());
    }

    // Go To (Ctrl+G) is in Home > Editing beside Find and Replace; it asks for a slide number and moves to that slide, out of any text being typed.
    void goToMovesToASlide()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        for (int i = 0; i < 4; ++i) w.newSlide();
        Editor *ed = w.editor();
        QCOMPARE(w.act(QStringLiteral("edit.goTo"))->text(), QStringLiteral("Go To…"));
        QCOMPARE(w.act(QStringLiteral("edit.goTo"))->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_G).toString(QKeySequence::NativeText));
        int inEditing = 0;
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->defaultAction() == w.act(QStringLiteral("edit.goTo"))) ++inEditing;
        QCOMPARE(inEditing, 1);
        ed->setCurrentPage(4);
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 4, 0)->id);
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            auto *n = d->findChild<QSpinBox *>(QStringLiteral("goToNumber"));
            QVERIFY(n);
            QCOMPARE(n->value(), 5);
            QCOMPARE(n->maximum(), 5);
            n->setValue(2);
        });
        QCOMPARE(ed->currentPage(), 1);
        QVERIFY(!ed->isEditingText());
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) { d->findChild<QSpinBox *>(QStringLiteral("goToNumber"))->setValue(4); }, false);
        QCOMPARE(ed->currentPage(), 1);
    }

    // Go To also goes to a section (by name or number), a comment (next, previous, or by number), and an object (by the name the Selection pane shows); the dialog has the "Go to what" list of JeffWrite's, an entry, and Next, Previous, and Go To.
    static void pickGoToKind(QDialog *d, const QString &kind)
    {
        auto *kinds = d->findChild<QListWidget *>(QStringLiteral("goToKinds"));
        QVERIFY(kinds);
        const auto found = kinds->findItems(kind, Qt::MatchExactly);
        QCOMPARE(found.size(), 1);
        kinds->setCurrentItem(found.first());
    }
    static void pressGoTo(QDialog *d, const char *button)
    {
        auto *b = d->findChild<QPushButton *>(QString::fromLatin1(button));
        QVERIFY(b);
        QVERIFY(b->isEnabled());
        b->click();
    }

    void goToMovesToASectionByName()
    {
        DeckWindow w;
        for (int i = 0; i < 5; ++i) w.newSlide();
        Editor *ed = w.editor();
        deck::addSection(*ed->doc(), 0, QStringLiteral("Opening"));
        deck::addSection(*ed->doc(), 2, QStringLiteral("Budget Review"));
        deck::addSection(*ed->doc(), 4, QStringLiteral("Closing"));
        ed->setCurrentPage(0);
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            pickGoToKind(d, QStringLiteral("Section"));
            auto *what = d->findChild<QLineEdit *>(QStringLiteral("goToWhat"));
            QVERIFY(what);
            what->setText(QStringLiteral("budget"));
            pressGoTo(d, "goToGo");
        });
        QCOMPARE(ed->currentPage(), 2);
        // Next from there.
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            pickGoToKind(d, QStringLiteral("Section"));
            pressGoTo(d, "goToNext");
        }, false);
        QCOMPARE(ed->currentPage(), 4);
        // An unknown name is refused, in words, and nothing moves.
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            pickGoToKind(d, QStringLiteral("Section"));
            d->findChild<QLineEdit *>(QStringLiteral("goToWhat"))->setText(QStringLiteral("Nowhere"));
            pressGoTo(d, "goToGo");
            QVERIFY(d->isVisible());
            QVERIFY(d->findChild<QLabel *>(QStringLiteral("goToProblem"))->text().contains(QStringLiteral("Nowhere")));
        }, false);
        QCOMPARE(ed->currentPage(), 4);
    }

    void goToNextAndPreviousComment()
    {
        DeckWindow w;
        w.resize(1200, 800);
        for (int i = 0; i < 4; ++i) w.newSlide();
        Editor *ed = w.editor();
        const comments::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")};
        // No comments: a clear message, nothing moves.
        ed->setCurrentPage(2);
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            pickGoToKind(d, QStringLiteral("Comment"));
            pressGoTo(d, "goToNext");
            QVERIFY(d->isVisible());
            QVERIFY(!d->findChild<QLabel *>(QStringLiteral("goToProblem"))->text().isEmpty());
        }, false);
        QCOMPARE(ed->currentPage(), 2);
        const QString a = comments::addComment(*ed->doc()->pages[1], pat, QStringLiteral("A"), QPointF(10, 10));
        const QString b = comments::addComment(*ed->doc()->pages[3], pat, QStringLiteral("B"), QPointF(10, 10));
        const QString c = comments::addComment(*ed->doc()->pages[3], pat, QStringLiteral("C"), QPointF(20, 20));
        ed->setCurrentPage(1);
        auto press = [&w](const char *button) {
            withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [button](QDialog *d) {
                pickGoToKind(d, QStringLiteral("Comment"));
                pressGoTo(d, button);
            }, false);
        };
        press("goToNext");   // nothing open: the first comment from this slide on
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(w.commentsPane()->currentComment(), a);
        QVERIFY(w.commentsPane()->isVisibleTo(&w));
        press("goToNext");
        QCOMPARE(ed->currentPage(), 3);
        QCOMPARE(w.commentsPane()->currentComment(), b);
        press("goToNext");
        QCOMPARE(w.commentsPane()->currentComment(), c);
        press("goToNext");   // wraps to the first
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(w.commentsPane()->currentComment(), a);
        press("goToPrevious");   // wraps to the last
        QCOMPARE(ed->currentPage(), 3);
        QCOMPARE(w.commentsPane()->currentComment(), c);
        // By number.
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            pickGoToKind(d, QStringLiteral("Comment"));
            d->findChild<QLineEdit *>(QStringLiteral("goToWhat"))->setText(QStringLiteral("2"));
            pressGoTo(d, "goToGo");
        });
        QCOMPARE(w.commentsPane()->currentComment(), b);
    }

    void goToAnObjectByItsNameSelectsItOnItsSlide()
    {
        DeckWindow w;
        for (int i = 0; i < 4; ++i) w.newSlide();
        Editor *ed = w.editor();
        auto shape = std::make_shared<ShapeItem>();
        shape->name = QStringLiteral("Revenue Chart");
        const QString id = shape->id;
        ed->doc()->pages[3]->items.push_back(shape);
        ed->setCurrentPage(0);
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            pickGoToKind(d, QStringLiteral("Object"));
            d->findChild<QLineEdit *>(QStringLiteral("goToWhat"))->setText(QStringLiteral("revenue chart"));
            pressGoTo(d, "goToGo");
        });
        QCOMPARE(ed->currentPage(), 3);
        QCOMPARE(ed->selection(), QStringList{id});
        // A name nothing has.
        ed->setCurrentPage(0);
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            pickGoToKind(d, QStringLiteral("Object"));
            d->findChild<QLineEdit *>(QStringLiteral("goToWhat"))->setText(QStringLiteral("Profit Table"));
            pressGoTo(d, "goToGo");
            QVERIFY(d->isVisible());
            QVERIFY(d->findChild<QLabel *>(QStringLiteral("goToProblem"))->text().contains(QStringLiteral("Profit Table")));
        }, false);
        QCOMPARE(ed->currentPage(), 0);
    }

    // The slide entry is the number box it always was, and is the list's first choice.
    void goToSlideStillHasItsNumberBox()
    {
        DeckWindow w;
        for (int i = 0; i < 3; ++i) w.newSlide();
        Editor *ed = w.editor();
        ed->setCurrentPage(0);
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            auto *kinds = d->findChild<QListWidget *>(QStringLiteral("goToKinds"));
            QVERIFY(kinds);
            QCOMPARE(kinds->item(0)->text(), QStringLiteral("Slide"));
            QCOMPARE(kinds->currentRow(), 0);
            auto *n = d->findChild<QSpinBox *>(QStringLiteral("goToNumber"));
            QVERIFY(n);
            QVERIFY(n->isVisibleTo(d));
            QCOMPARE(n->maximum(), 4);
            n->setValue(3);
        });
        QCOMPARE(ed->currentPage(), 2);
    }

    // Find Next goes through the text of every slide in order from the current one (titles, bodies, shapes, tables, and the notes) and round
    // to the start; each match is shown on its slide, selected.
    void findGoesThroughEverySlideNotesTablesAndShapes()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        fillFindDeck(w);
        Editor *ed = w.editor();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        const QStringList all = {QStringLiteral("1:review@5"), QStringLiteral("2:review@3"), QStringLiteral("2:Review@21"), QStringLiteral("2:review@7 in the notes"),
                                 QStringLiteral("3:Review@0"), QStringLiteral("3:review@13"), QStringLiteral("3:review@5"), QStringLiteral("3:Review@0")};
        QCOMPARE(findAll(w, 9), all + QStringList{QStringLiteral("1:review@5")});
        QVERIFY(pane->status().isEmpty());
        // The match is on its slide, in the notes pane when it is a note.
        ed->setCurrentPage(0);
        ed->endTextEdit();
        QStringList slides;
        for (int i = 0; i < 4; ++i) {
            pane->findNext();
            slides << QString::number(ed->currentPage() + 1);
        }
        QCOMPARE(slides, (QStringList{QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("2"), QStringLiteral("2")}));
        QVERIFY(w.notesPane()->isVisible());
        QCOMPARE(w.notesPane()->textCursor().selectedText(), QStringLiteral("review"));
        QVERIFY(!ed->isEditingText());
        QApplication::processEvents();
        shot(&w, QStringLiteral("find-notes-match"));
        pane->findNext();
        pane->findNext();
        QApplication::processEvents();
        shot(&w, QStringLiteral("find-slide-match"));
        // Not in the notes when they are turned off.
        pane->speakerNotes()->setChecked(false);
        ed->setCurrentPage(0);
        ed->endTextEdit();
        QCOMPARE(findAll(w, 8), (QStringList{QStringLiteral("1:review@5"), QStringLiteral("2:review@3"), QStringLiteral("2:Review@21"), QStringLiteral("3:Review@0"),
                                             QStringLiteral("3:review@13"), QStringLiteral("3:review@5"), QStringLiteral("3:Review@0"), QStringLiteral("1:review@5")}));
        // Match case, and whole words only.
        pane->speakerNotes()->setChecked(true);
        pane->matchCase()->setChecked(true);
        ed->setCurrentPage(0);
        ed->endTextEdit();
        QCOMPARE(findAll(w, 5), (QStringList{QStringLiteral("1:review@5"), QStringLiteral("2:review@3"), QStringLiteral("2:review@7 in the notes"), QStringLiteral("3:review@13"),
                                             QStringLiteral("3:review@5")}));
        pane->matchCase()->setChecked(false);
        pane->wholeWords()->setChecked(true);
        ed->setCurrentPage(0);
        ed->endTextEdit();
        QCOMPARE(findAll(w, 7), (QStringList{QStringLiteral("1:review@5"), QStringLiteral("2:review@3"), QStringLiteral("2:Review@21"), QStringLiteral("2:review@7 in the notes"),
                                             QStringLiteral("3:review@5"), QStringLiteral("3:Review@0"), QStringLiteral("1:review@5")}));
        // Nothing found says so and leaves everything as it was.
        pane->wholeWords()->setChecked(false);
        pane->findBox()->setText(QStringLiteral("nothing like this"));
        const QString before = matchShown(w);
        QVERIFY(!pane->findNext());
        QVERIFY2(pane->status().contains(QStringLiteral("\"nothing like this\" was not found")), qPrintable(pane->status()));
        QCOMPARE(matchShown(w), before);
    }

    // The search starts at the slide being edited, and at the text cursor when text is being typed in.
    void findStartsWhereYouAre()
    {
        DeckWindow w;
        w.show();
        fillFindDeck(w);
        Editor *ed = w.editor();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        ed->setCurrentPage(2);
        QCOMPARE(findAll(w, 3), (QStringList{QStringLiteral("3:Review@0"), QStringLiteral("3:review@13"), QStringLiteral("3:review@5")}));
        ed->setCurrentPage(1);
        ed->endTextEdit();
        ed->clearSelection();
        QCOMPARE(findAll(w, 2), (QStringList{QStringLiteral("2:review@3"), QStringLiteral("2:Review@21")}));
        // With the caret after the first match in the body, the next is the second, not the first again.
        ed->setCurrentPage(1);
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 1, 1)->id, 10);
        QCOMPARE(findAll(w, 1), (QStringList{QStringLiteral("2:Review@21")}));
        // A search from the end of the last slide's body goes on to its shape.
        ed->setCurrentPage(2);
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 2, 1)->id, 100);
        QVERIFY(ed->isEditingText());
        QCOMPARE(findAll(w, 1), (QStringList{QStringLiteral("3:review@5")}));
    }

    // Replace All swaps every match, notes included, as one step that undoes, and says how many.
    void replaceAllIsOneStepAndSaysHowMany()
    {
        DeckWindow w;
        w.show();
        fillFindDeck(w);
        Editor *ed = w.editor();
        Document *d = ed->doc();
        w.showFindPane(true);
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        pane->replaceBox()->setText(QStringLiteral("audit"));
        const QString title = storyText(w, placeholderOfSlide(*d, 0, 0)->storyId);
        const int steps = ed->undoStack()->count();
        pane->replaceAll();
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 8 replacements."));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("Year audit"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 1, 1)->storyId), QStringLiteral("We audit profit and audit again"));
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Do not audit the numbers"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 2, 1)->storyId), QStringLiteral("audits and paudits"));
        QCOMPARE(storyText(w, static_cast<ShapeItem *>(d->pages[2]->items[2].get())->storyId), QStringLiteral("Peer audit"));
        QCOMPARE(storyText(w, static_cast<TableItem *>(d->pages[2]->items[3].get())->cells[0].storyId), QStringLiteral("audit"));
        // The notes pane shows the new words when its slide is the one being edited.
        ed->setCurrentPage(1);
        QCOMPARE(w.notesPane()->toPlainText(), QStringLiteral("Do not audit the numbers"));
        // One Undo puts every word back.
        ed->undo();
        d = ed->doc();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), title);
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Do not review the numbers"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 2, 1)->storyId), QStringLiteral("Reviews and previews"));
        QCOMPARE(storyText(w, static_cast<TableItem *>(d->pages[2]->items[3].get())->cells[0].storyId), QStringLiteral("Review"));
        QCOMPARE(ed->undoStack()->index(), steps);
        // Without the notes, and with case and whole words; one match says "1 replacement", none says none and adds no step.
        pane->speakerNotes()->setChecked(false);
        pane->matchCase()->setChecked(true);
        pane->wholeWords()->setChecked(true);
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 3 replacements."));
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Do not review the numbers"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("Year audit"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 2, 1)->storyId), QStringLiteral("Reviews and previews"));
        ed->undo();
        pane->findBox()->setText(QStringLiteral("Profit"));
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 1 replacement."));
        ed->undo();
        const int stepsNow = ed->undoStack()->count();
        pane->findBox()->setText(QStringLiteral("zzz"));
        pane->replaceAll();
        QCOMPARE(ed->undoStack()->count(), stepsNow);
        QVERIFY2(pane->status().contains(QStringLiteral("no replacements")), qPrintable(pane->status()));
    }

    // Replace swaps the match that is shown, in a box or in the notes, and goes on to the next; each is a step of its own.
    void replaceSwapsTheMatchShownAndMovesOn()
    {
        DeckWindow w;
        w.show();
        fillFindDeck(w);
        Editor *ed = w.editor();
        Document *d = ed->doc();
        w.showFindPane(true);
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        pane->replaceBox()->setText(QStringLiteral("audit"));
        const int steps = ed->undoStack()->count();
        // Nothing is shown yet: Replace only finds the first.
        pane->replaceOne();
        QCOMPARE(matchShown(w), QStringLiteral("1:review@5"));
        QCOMPARE(ed->undoStack()->count(), steps);
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("Year audit"));
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QCOMPARE(matchShown(w), QStringLiteral("2:review@3"));
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 1, 1)->storyId), QStringLiteral("We audit profit and Review again"));
        QCOMPARE(matchShown(w), QStringLiteral("2:Review@20"));
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 1, 1)->storyId), QStringLiteral("We audit profit and audit again"));
        QCOMPARE(matchShown(w), QStringLiteral("2:review@7 in the notes"));
        // In the notes.
        pane->replaceOne();
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Do not audit the numbers"));
        QCOMPARE(ed->undoStack()->count(), steps + 4);
        QCOMPARE(matchShown(w), QStringLiteral("3:Review@0"));
        ed->undo();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[1]), QStringLiteral("Do not review the numbers"));
    }

    // The pane's boxes are named for screen readers.
    void findPaneControlsAreNamed()
    {
        DeckWindow w;
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        for (QLineEdit *e : pane->findChildren<QLineEdit *>()) {
            QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(e);
            QVERIFY(iface);
            QVERIFY2(!iface->text(QAccessible::Name).trimmed().isEmpty(), qPrintable(e->objectName()));
        }
        for (QAbstractButton *b : pane->findChildren<QAbstractButton *>()) QVERIFY(!b->text().isEmpty());
    }

private:
    // A real mouse click: pressed and let go on the slide at a point of the page, through the canvas.
    static void clickSlide(DeckWindow &w, const QPointF &page, Qt::MouseButton button)
    {
        Canvas *cv = w.canvas();
        const QPointF v = cv->pageToView(page);
        QMouseEvent press(QEvent::MouseButtonPress, v, cv->viewport()->mapToGlobal(v.toPoint()), button, button, Qt::NoModifier);
        QApplication::sendEvent(cv->viewport(), &press);
        QMouseEvent release(QEvent::MouseButtonRelease, v, cv->viewport()->mapToGlobal(v.toPoint()), button, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(cv->viewport(), &release);
    }
    static QString longWords()
    {
        QString out;
        for (int i = 0; i < 40; ++i) out += QStringLiteral("longer words ");
        return out;
    }

private Q_SLOTS:
    // Replace All fits what the new words need, as typing them would: a title a program had shrunk is fitted here from now on, a box that
    // grows to fit grows, a table's rows grow; one Undo gives all of it back.
    void replaceAllClearsAStoredShrinkAndRefitsBoxesAndTables()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        TextItem *title = placeholderOfSlide(*d, 0, 0);
        QTextCursor(d->storyDoc(title->storyId)).insertText(QStringLiteral("review"));
        title->autofit = TextItem::ShrinkOnOverflow;
        title->fitAsStored = true;
        title->fitScale = 0.8;
        auto grow = std::make_shared<TextItem>();
        grow->rect = QRectF(100, 300, 200, 30);
        grow->storyId = d->createStory(QStringLiteral("review"));
        grow->autofit = TextItem::GrowBox;
        d->pages[0]->items.push_back(grow);
        w.insertTable(1, 2);
        auto *table = static_cast<TableItem *>(d->pages[0]->items.back().get());
        QTextCursor(d->storyDoc(table->cells[0].storyId)).insertText(QStringLiteral("Review"));
        ed->clearSelection();
        ed->notifyLive();
        ed->undoStack()->clear();
        const double growHeight = grow->rect.height(), rowHeight = table->rowH[0];
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        pane->replaceBox()->setText(longWords());
        pane->replaceAll();
        QVERIFY(!title->fitAsStored);
        QVERIFY2(grow->rect.height() > growHeight + 50, qPrintable(QString::number(grow->rect.height())));
        QVERIFY2(table->rowH[0] > rowHeight + 50, qPrintable(QString::number(table->rowH[0])));
        const double h = grow->rect.height(), r = table->rowH[0];
        ed->autoGrowText(grow.get());
        ed->fitTableRows(table);
        QCOMPARE(grow->rect.height(), h);
        QCOMPARE(table->rowH[0], r);
        ed->undo();
        d = ed->doc();
        QVERIFY(placeholderOfSlide(*d, 0, 0)->fitAsStored);
        QCOMPARE(d->pages[0]->items[2]->rect.height(), growHeight);
        QCOMPARE(static_cast<TableItem *>(d->pages[0]->items[3].get())->rowH[0], rowHeight);
    }

    // Find reaches text in groups and on hidden slides, and Text Art words (shown by selecting the Text Art); the layouts and the
    // slide master are the presentation's design, not its words, and are left alone.
    void findReachesGroupsHiddenSlidesAndTextArtButNotLayouts()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 0)->storyId)).insertText(QStringLiteral("Plain review"));
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 1, 0)->storyId)).insertText(QStringLiteral("Hidden review"));
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 2, 0)->storyId)).insertText(QStringLiteral("Group review one"));
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 2, 1)->storyId)).insertText(QStringLiteral("Group review two"));
        d->pages[1]->hidden = true;
        auto art = std::make_shared<TextArtItem>();
        art->rect = QRectF(100, 400, 300, 80);
        art->text = QStringLiteral("Art review words");
        d->pages[2]->items.push_back(art);
        auto art2 = std::make_shared<TextArtItem>();
        art2->rect = QRectF(100, 300, 300, 80);
        art2->text = QStringLiteral("Second review art");
        d->pages[2]->items.push_back(art2);
        const QString artId = art->id, art2Id = art2->id;
        // A word in a layout's prompt: not the presentation's.
        QString layoutStory;
        for (const auto &m : d->masters)
            for (const auto &it : m->items)
                if (it->type() == ItemType::Text && layoutStory.isEmpty()) layoutStory = static_cast<TextItem *>(it.get())->storyId;
        QVERIFY(!layoutStory.isEmpty());
        QTextCursor(d->storyDoc(layoutStory)).insertText(QStringLiteral(" review"));
        ed->setCurrentPage(2);
        ed->select(QStringList{placeholderOfSlide(*d, 2, 0)->id, placeholderOfSlide(*d, 2, 1)->id});
        slideMenu(w);   // the commands are enabled as the selection allows
        w.act(QStringLiteral("arr.group"))->trigger();
        QCOMPARE(ed->selectionKind(), QStringLiteral("group"));
        ed->clearSelection();
        ed->setCurrentPage(0);
        ed->notifyLive();
        ed->undoStack()->clear();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        QCOMPARE(findAll(w, 4), (QStringList{QStringLiteral("1:review@6"), QStringLiteral("2:review@7"), QStringLiteral("3:review@6"), QStringLiteral("3:review@6")}));
        QCOMPARE(ed->currentPage(), 2);
        QVERIFY(ed->isEditingText());
        // The Text Art is shown by selecting it, and Replace swaps its words; the next search goes on from there.
        QVERIFY(pane->findNext());
        QCOMPARE(ed->currentPage(), 2);
        QVERIFY(!ed->isEditingText());
        QCOMPARE(ed->selection(), QStringList{artId});
        pane->replaceBox()->setText(QStringLiteral("audit"));
        pane->replaceOne();
        QCOMPARE(static_cast<TextArtItem *>(ed->doc()->item(artId))->text, QStringLiteral("Art audit words"));
        QCOMPARE(ed->selection(), QStringList{art2Id});   // on to the second
        // Replace All: the group's, the hidden slide's, the first slide's, and the other Text Art's, but not the layout's.
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 5 replacements."));
        d = ed->doc();
        QVERIFY(storyText(w, layoutStory).contains(QStringLiteral(" review")));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 1, 0)->storyId), QStringLiteral("Hidden audit"));
        QCOMPARE(static_cast<TextArtItem *>(d->item(art2Id))->text, QStringLiteral("Second audit art"));
        ed->undo();
        QCOMPARE(static_cast<TextArtItem *>(ed->doc()->item(art2Id))->text, QStringLiteral("Second review art"));
        QCOMPARE(static_cast<TextArtItem *>(ed->doc()->item(artId))->text, QStringLiteral("Art audit words"));
        pane->findBox()->setText(QStringLiteral("review words"));
        QVERIFY(!pane->findNext());
        pane->findBox()->setText(QStringLiteral("audit words"));
        QVERIFY(pane->findNext());
        QCOMPARE(ed->selection(), QStringList{artId});
    }

    // The words that replace a match are not searched for again: after Replace the next match is the next one, and Replace All makes one pass.
    void aReplacementThatHoldsTheSearchTextIsNotFoundAgain()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 0)->storyId)).insertText(QStringLiteral("cat and cat"));
        QTextCursor(d->storyDoc(d->pages[0]->notesStoryId)).insertText(QStringLiteral("cat"));
        ed->notifyLive();
        ed->undoStack()->clear();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("cat"));
        pane->replaceBox()->setText(QStringLiteral("concatenate"));
        pane->replaceOne();
        QCOMPARE(matchShown(w), QStringLiteral("1:cat@0"));
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("concatenate and cat"));
        QCOMPARE(matchShown(w), QStringLiteral("1:cat@16"));
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("concatenate and concatenate"));
        QCOMPARE(matchShown(w), QStringLiteral("1:cat@0 in the notes"));
        pane->replaceOne();
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("concatenate"));
        ed->undo();
        ed->undo();
        ed->undo();
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 3 replacements."));
        QCOMPARE(storyText(w, placeholderOfSlide(*ed->doc(), 0, 0)->storyId), QStringLiteral("concatenate and concatenate"));
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("concatenate"));
    }

    // Two matches in one notes box are both found, in order, and Replace on the first leaves the second to be found where it moved to.
    void twoMatchesInOneNotesBoxAreBothFound()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(d->pages[0]->notesStoryId)).insertText(QStringLiteral("review one and review two"));
        ed->notifyLive();
        ed->undoStack()->clear();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        QCOMPARE(findAll(w, 3), (QStringList{QStringLiteral("1:review@0 in the notes"), QStringLiteral("1:review@15 in the notes"), QStringLiteral("1:review@0 in the notes")}));
        pane->replaceBox()->setText(QStringLiteral("audit"));
        pane->replaceOne();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("audit one and review two"));
        QCOMPARE(matchShown(w), QStringLiteral("1:review@14 in the notes"));
        pane->replaceOne();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("audit one and audit two"));
        QVERIFY(!pane->findNext());
        QVERIFY2(pane->status().contains(QStringLiteral("was not found")), qPrintable(pane->status()));
    }

    // A real right click (pressed and let go on the slide, not a menu request made up in the test) selects the object it is on, keeps the
    // objects selected together when it is on one of them, and clears the selection on the bare slide.
    void aRightClickSelectsTheObjectUnderIt()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 0)->storyId)).insertText(QStringLiteral("Title words"));
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 1)->storyId)).insertText(QStringLiteral("Subtitle words"));
        ed->notifyLive();
        QApplication::processEvents();
        const QString titleId = placeholderOfSlide(*d, 0, 0)->id, subId = placeholderOfSlide(*d, 0, 1)->id;
        const QPointF titleAt = placeholderOfSlide(*d, 0, 0)->bounds().center(), subAt = placeholderOfSlide(*d, 0, 1)->bounds().center();
        ed->select(titleId);
        clickSlide(w, subAt, Qt::RightButton);
        QCOMPARE(ed->selection(), QStringList{subId});
        QVERIFY(slideMenu(w).contains(QStringLiteral("edit.cut")));
        // On one of two selected objects, both stay selected.
        ed->select(QStringList{titleId, subId});
        clickSlide(w, titleAt, Qt::RightButton);
        QCOMPARE(ed->selection().size(), 2);
        QVERIFY(slideMenu(w).contains(QStringLiteral("arr.group")));
        // On the bare slide, nothing is selected.
        clickSlide(w, QPointF(5, 5), Qt::RightButton);
        QVERIFY(ed->selection().isEmpty());
        QVERIFY(slideMenu(w).contains(QStringLiteral("slide.reset")));
    }

    // A right click on the bare slide while typing ends the typing, so the menu is the slide's; on the text being typed in, it stays.
    void aRightClickOnTheBareSlideWhileTypingEndsTheTyping()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 0)->storyId)).insertText(QStringLiteral("Title words"));
        ed->notifyLive();
        QApplication::processEvents();
        const QString titleId = placeholderOfSlide(*d, 0, 0)->id;
        const QPointF titleAt = placeholderOfSlide(*d, 0, 0)->bounds().center();
        ed->beginTextEdit(titleId);
        clickSlide(w, titleAt, Qt::RightButton);
        QVERIFY(ed->isEditingText());
        QVERIFY(slideMenu(w).contains(QStringLiteral("fmt.fontDialog")));
        clickSlide(w, QPointF(5, 5), Qt::RightButton);
        QVERIFY(!ed->isEditingText());
        QVERIFY(ed->selection().isEmpty());
        const QStringList ids = slideMenu(w);
        QVERIFY2(ids.contains(QStringLiteral("slide.reset")) && !ids.contains(QStringLiteral("fmt.fontDialog")), qPrintable(ids.join(QLatin1Char(' '))));
    }

    // The pane's message is plain text, and a long search text can't stretch the pane: it is cut short and may break anywhere.
    void findPaneMessageIsPlainTextAndNarrow()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        QLabel *status = pane->findChild<QLabel *>(QStringLiteral("findStatus"));
        QVERIFY(status);
        pane->findBox()->setText(QStringLiteral("<b>x"));
        pane->findNext();
        QCOMPARE(status->textFormat(), Qt::PlainText);
        QVERIFY2(pane->status().contains(QStringLiteral("\"<b>x\" was not found")), qPrintable(pane->status()));
        pane->findBox()->setText(QString(120, QLatin1Char('x')));
        pane->findNext();
        QVERIFY2(pane->status().size() < 200, qPrintable(QString::number(pane->status().size())));
        QVERIFY2(status->minimumSizeHint().width() < 300, qPrintable(QString::number(status->minimumSizeHint().width())));
        pane->replaceBox()->setText(QStringLiteral("y"));
        pane->replaceAll();
        QVERIFY2(pane->status().size() < 200, qPrintable(QString::number(pane->status().size())));
        QVERIFY2(status->minimumSizeHint().width() < 300, qPrintable(QString::number(status->minimumSizeHint().width())));
    }

    // The Selection Pane lists the slide's objects by name, the one in front first. A row picks the object; a name typed in a row names it; a box hides or shows it (a hidden object is not
    // drawn on the slide, and Show All and Hide All set every box); the buttons and a drag move it in the order. Each change is one step of Undo.
    void selectionPaneListsNamesHidesAndOrdersObjects()
    {
        DeckWindow w;
        w.resize(1400, 900);
        w.show();
        Editor *ed = w.editor();
        auto d = deck::newPresentation();
        d->pages[0]->items.clear();
        auto add = [&](const QString &name, const QColor &color, double x) {
            auto s = std::make_shared<jp::ShapeItem>();
            s->name = name;
            s->rect = QRectF(x, 100, 100, 100);
            s->fill = jp::Fill::solid(jp::ColorRef::rgb(color));
            s->stroke = jp::Stroke::none();
            d->pages[0]->items.push_back(s);
            return s->id;
        };
        const QString back = add(QStringLiteral("Back"), Qt::red, 50), middle = add(QString(), Qt::blue, 150), front = add(QStringLiteral("Front"), Qt::green, 250);
        ed->setDocument(std::move(d));
        auto item = [&](const QString &id) { return ed->doc()->item(id); };
        auto order = [&] {
            QStringList ids;
            for (const auto &it : ed->surfaceItems()) ids << it->id;
            return ids;
        };
        auto greenShown = [&] {
            PaintContext ctx;
            ctx.doc = ed->doc();
            ctx.cache = &ed->cache();
            return Renderer::renderToImage(ctx, 0, 1.0).pixelColor(300, 150) == QColor(Qt::green);
        };
        QAction *open = w.act(QStringLiteral("arr.selectionPane"));
        QVERIFY(open);
        QVERIFY(open->isEnabled());
        open->trigger();
        auto *list = w.findChild<QListWidget *>(QStringLiteral("selectionList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 3);
        QCOMPARE(list->item(0)->text(), QStringLiteral("Front"));
        QCOMPARE(list->item(2)->text(), QStringLiteral("Back"));
        QVERIFY(!list->item(1)->text().isEmpty());
        QCOMPARE(list->item(0)->checkState(), Qt::Checked);
        list->setCurrentRow(2);
        QCOMPARE(ed->selection(), QStringList{back});
        list->item(2)->setText(QStringLiteral("Logo"));
        QTRY_COMPARE(item(back)->name, QStringLiteral("Logo"));
        ed->undo();
        QCOMPARE(item(back)->name, QStringLiteral("Back"));
        ed->redo();
        QCOMPARE(item(back)->name, QStringLiteral("Logo"));
        QVERIFY(greenShown());
        QTRY_COMPARE(list->item(0)->text(), QStringLiteral("Front"));
        list->item(0)->setCheckState(Qt::Unchecked);
        QTRY_VERIFY(item(front)->hidden);
        QVERIFY(!greenShown());
        ed->undo();
        QVERIFY(!item(front)->hidden);
        QVERIFY(greenShown());
        auto *hideAll = w.findChild<QAbstractButton *>(QStringLiteral("selectionHideAll"));
        auto *showAll = w.findChild<QAbstractButton *>(QStringLiteral("selectionShowAll"));
        QVERIFY(hideAll && showAll);
        hideAll->click();
        QTRY_VERIFY(item(back)->hidden && item(middle)->hidden && item(front)->hidden);
        showAll->click();
        QTRY_VERIFY(!item(back)->hidden && !item(middle)->hidden && !item(front)->hidden);
        QCOMPARE(order(), (QStringList{back, middle, front}));
        QTRY_COMPARE(list->count(), 3);
        list->setCurrentRow(0);
        auto *backward = w.findChild<QAbstractButton *>(QStringLiteral("selectionBackward"));
        QVERIFY(backward);
        backward->click();
        QCOMPARE(order(), (QStringList{back, front, middle}));
        ed->undo();
        QCOMPARE(order(), (QStringList{back, middle, front}));
        QTRY_COMPARE(list->count(), 3);
        QVERIFY(list->model()->moveRow(QModelIndex(), 0, QModelIndex(), 3));
        QTRY_COMPARE(order(), (QStringList{front, back, middle}));
        ed->undo();
        QCOMPARE(order(), (QStringList{back, middle, front}));
        // Another slide has its own objects.
        ed->setCurrentPage(0);
        QTRY_COMPARE(list->count(), 3);
    }

    // Full Screen is a switch on the View tab (F11): the window fills the screen, the command's check follows the window, and the same command leaves it. Esc leaves it too, when the slide
    // has nothing of its own to put away.
    void fullScreenIsOnTheViewTabAndEscLeavesIt()
    {
        QFile file(QStringLiteral(":/ribbon.json"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QJsonArray tabs = QJsonDocument::fromJson(file.readAll()).object().value(QStringLiteral("tabs")).toArray();
        bool onViewTab = false;
        for (const QJsonValue &t : tabs)
            if (t.toObject().value(QStringLiteral("name")).toString() == QLatin1String("View"))
                onViewTab = QJsonDocument(t.toObject()).toJson().contains("view.fullScreen");
        QVERIFY(onViewTab);
        DeckWindow w;
        w.resize(1100, 750);
        w.show();
        QAction *fs = w.act(QStringLiteral("view.fullScreen"));
        QVERIFY(fs);
        QVERIFY(fs->isCheckable() && fs->isEnabled());
        QCOMPARE(fs->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::Key_F11).toString(QKeySequence::NativeText));
        QVERIFY(!w.isFullScreen() && !fs->isChecked());
        fs->trigger();
        QTRY_VERIFY(w.isFullScreen());
        QTRY_VERIFY(fs->isChecked());
        fs->trigger();
        QTRY_VERIFY(!w.isFullScreen());
        QTRY_VERIFY(!fs->isChecked());
        fs->trigger();
        QTRY_VERIFY(w.isFullScreen());
        w.editor()->clearSelection();
        w.canvas()->setFocus();
        QTest::keyClick(w.canvas(), Qt::Key_Escape);
        QTRY_VERIFY(!w.isFullScreen());
        QTRY_VERIFY(!fs->isChecked());
        // With an object selected the slide has it to put away first.
        w.editor()->select(w.editor()->surfaceItems().front()->id);
        fs->trigger();
        QTRY_VERIFY(w.isFullScreen());
        w.canvas()->setFocus();
        QTest::keyClick(w.canvas(), Qt::Key_Escape);
        QVERIFY(w.isFullScreen());
        QVERIFY(w.editor()->selection().isEmpty());
        QTest::keyClick(w.canvas(), Qt::Key_Escape);
        QTRY_VERIFY(!w.isFullScreen());
    }

    // A hidden object is left out of the outline, Find, Select All, and the empty-placeholder prompts; the slide, its thumbnail, and a layout draw without it.
    void hiddenObjectsAreLeftOutOfTheOutlineFindThePromptsAndThePictures()
    {
        auto d = deck::newPresentation();
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        d->storyDoc(title->storyId)->setPlainText(QStringLiteral("Needle title"));
        auto outline = [&](const Document &doc) { return deck::outputOutlineText(doc, deck::OutputOptions()); };
        QVERIFY(outline(*d).contains(QStringLiteral("Needle title")));
        auto found = [&](const Document &doc) {
            for (const FindStory &st : findStories(doc))
                if (doc.storyDoc(st.storyId) && doc.storyDoc(st.storyId)->toPlainText().contains(QStringLiteral("Needle"))) return true;
            return false;
        };
        QVERIFY(found(*d));
        title->hidden = true;
        QVERIFY(!outline(*d).contains(QStringLiteral("Needle title")));
        QVERIFY(!found(*d));
        // The window: Select All, and the prompts of the empty placeholders (a hidden one shows none).
        DeckWindow w;
        w.resize(1400, 900);
        w.show();
        Editor *ed = w.editor();
        auto bare = deck::newPresentation();
        for (const ItemPtr &it : bare->pages[0]->items) it->hidden = true;
        ed->setDocument(std::move(bare));
        w.act(QStringLiteral("edit.selectAll"))->trigger();
        QVERIFY(ed->selection().isEmpty());
        QTRY_VERIFY(!w.canvas()->grab().isNull());
        const QImage hiddenPrompts = w.canvas()->grab().toImage();
        auto empty = deck::newPresentation();
        empty->pages[0]->items.clear();
        ed->setDocument(std::move(empty));
        QTRY_VERIFY(!w.canvas()->grab().isNull());
        {
            const QImage other = w.canvas()->grab().toImage();
            QRect box;
            for (int y = 0; y < std::min(other.height(), hiddenPrompts.height()); ++y)
                for (int x = 0; x < std::min(other.width(), hiddenPrompts.width()); ++x)
                    if (other.pixel(x, y) != hiddenPrompts.pixel(x, y)) box = box.united(QRect(x, y, 1, 1));
            QVERIFY2(box.isNull() && other.size() == hiddenPrompts.size(), qPrintable(QStringLiteral("differ in %1,%2 %3x%4 sizes %5 %6").arg(box.x()).arg(box.y()).arg(box.width()).arg(box.height()).arg(other.width()).arg(hiddenPrompts.width())));
        }
        ed->setDocument(deck::newPresentation());
        QTRY_VERIFY(!w.canvas()->grab().isNull());
        QVERIFY(w.canvas()->grab().toImage() != hiddenPrompts);   // (the same prompts, shown, are on the screen)
        // A shape on the slide and one on its layout, hidden: the thumbnail is what the slide is without them, and shown they change it.
        auto picture = [&](bool onLayout, bool hidden) {
            auto doc = deck::newPresentation();
            auto red = std::make_shared<jp::ShapeItem>();
            red->rect = QRectF(0, 0, 900, 500);
            red->fill = jp::Fill::solid(jp::ColorRef::rgb(Qt::red));
            red->hidden = hidden;
            (onLayout ? doc->master(doc->pages[0]->masterId)->items : doc->pages[0]->items).insert((onLayout ? doc->master(doc->pages[0]->masterId)->items : doc->pages[0]->items).begin(), red);
            LayoutCache cache;
            return deck::slideImage(*doc, &cache, 0, 480);
        };
        const QImage plain = [&] {
            auto doc = deck::newPresentation();
            LayoutCache cache;
            return deck::slideImage(*doc, &cache, 0, 480);
        }();
        for (const bool onLayout : {false, true}) {
            QVERIFY(picture(onLayout, false) != plain);
            QCOMPARE(picture(onLayout, true), plain);
        }
    }

    // A slide of 20,000 objects: the pane lists them, Select All, a nudge, and a drag of a row 1,000 places each take a few seconds of the window thread's processor time at most. Processor time, not the wall clock: on a busy machine a nudge took over 6 s by the clock. The bound is six: on CI sharing a busy machine a nudge used 3.3 s of processor time, and what this guards against, work that grows with the square of the count, takes minutes.
    void aSlideOfTwentyThousandObjectsIsHandledQuickly()
    {
        DeckWindow w;
        w.resize(1400, 900);
        w.show();
        Editor *ed = w.editor();
        auto d = deck::newPresentation();
        d->pages[0]->items.clear();
        for (int i = 0; i < 20000; ++i) {
            auto s = std::make_shared<jp::ShapeItem>();
            s->rect = QRectF(i % 500, i / 500, 5, 5);
            d->pages[0]->items.push_back(s);
        }
        ed->setDocument(std::move(d));
        qint64 began = 0;
        auto within = [&](const char *what) {
            const qint64 ms = (jp::threadCpuMicros() - began) / 1000;
            began = jp::threadCpuMicros();
            QVERIFY2(ms < 6000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")), qPrintable(QStringLiteral("%1 took %2 ms").arg(QLatin1String(what)).arg(ms)));
        };
        began = jp::threadCpuMicros();
        w.act(QStringLiteral("arr.selectionPane"))->trigger();
        auto *list = w.findChild<QListWidget *>(QStringLiteral("selectionList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 20000);
        within("opening the pane");
        began = jp::threadCpuMicros();
        w.act(QStringLiteral("edit.selectAll"))->trigger();
        QCOMPARE(int(ed->selection().size()), 20000);
        w.findChild<SelectionPane *>(QStringLiteral("selectionPane"))->refresh();   // (the pane follows the selection)
        QCOMPARE(list->currentRow(), 0);
        within("Select All");
        ed->moveSelectionBy(3, 3);
        within("a nudge");
        const QString moved = ed->surfaceItems()[19999]->id;
        QVERIFY(list->model()->moveRow(QModelIndex(), 0, QModelIndex(), 1000));
        QCOMPARE(ed->surfaceItems()[19999 - 999]->id, moved);
        within("a drag of 1,000 places");
    }

    // Esc is left to what wants it (text being typed, KeyTips, the ribbon, a dialog, the File screen); a maximized window comes back maximized; and a window closed while full screen with the File
    // screen open leaves no filter behind.
    void escLeavesFullScreenOnlyWhenNothingElseWantsIt()
    {
        auto w = std::make_unique<DeckWindow>();
        w->resize(1100, 750);
        w->showMaximized();
        QVERIFY(QTest::qWaitForWindowActive(w.get()));
        QTRY_VERIFY(w->isMaximized());
        QAction *fs = w->act(QStringLiteral("view.fullScreen"));
        Editor *ed = w->editor();
        auto enter = [&] {
            if (!w->isFullScreen()) fs->trigger();
            QTRY_VERIFY(w->isFullScreen());
            w->canvas()->setFocus();
        };
        ed->select(ed->surfaceItems().front()->id);
        ed->beginTextEdit(ed->surfaceItems().front()->id, 0);
        enter();
        QTest::keyClick(w->canvas(), Qt::Key_Escape);
        QVERIFY(w->isFullScreen());
        QVERIFY(!ed->isEditingText());
        ed->clearSelection();
        KeyTips *tips = nullptr;
        for (QObject *child : w->children())
            if ((tips = dynamic_cast<KeyTips *>(child))) break;
        QVERIFY(tips);
        tips->showTop();
        QVERIFY(tips->level() != KeyTips::Off);
        QTest::keyClick(w->canvas(), Qt::Key_Escape);
        QVERIFY(w->isFullScreen());
        QCOMPARE(int(tips->level()), int(KeyTips::Off));
        w->ribbon()->focusCurrentTab();
        QTRY_VERIFY(QApplication::focusWidget() && w->ribbon()->isAncestorOf(QApplication::focusWidget()));
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_Escape);
        QVERIFY(w->isFullScreen());
        w->canvas()->setFocus();
        {
            QDialog dialog(w.get());
            QTimer::singleShot(100, &dialog, [&] { QTest::keyClick(&dialog, Qt::Key_Escape); });
            dialog.exec();
        }
        QVERIFY(w->isFullScreen());
        w->showFileScreen();
        QTest::keyClick(w.get(), Qt::Key_Escape);
        QVERIFY(w->isFullScreen());
        w->canvas()->setFocus();
        QTest::keyClick(w->canvas(), Qt::Key_Escape);
        QTRY_VERIFY(!w->isFullScreen());
        QTRY_VERIFY(w->isMaximized());   // (as it was before)
        // Closed while full screen with the File screen open.
        fs->trigger();
        QTRY_VERIFY(w->isFullScreen());
        QVERIFY(w->fullScreenRequested());
        w->showFileScreen();
        w.reset();
        QWidget probe;
        probe.show();
        QTest::keyClick(&probe, Qt::Key_Escape);   // (the keys of the program go through the filters of the program, and the closed window's is not among them)
        QCoreApplication::processEvents();
    }
    // Options > Customize > Keyboard: a custom key for Redo is what the Redo tooltip shows after the window rewrites it, and the tooltips it wrote are not undone by a change of another command's keys.
    void aCustomRedoKeyShowsInTheTooltip()
    {
        using namespace jp;
        const auto cleanup = qScopeGuard([] { Settings::get().setValue(QStringLiteral("keys/custom"), QString()); });
        Settings::get().setValue(QStringLiteral("keys/custom"), QString());
        DeckWindow w;
        w.refreshNow();
        QVERIFY(w.act(QStringLiteral("edit.redo"))->toolTip().endsWith(QStringLiteral("(%1)").arg(KeyMap::shownKey(w.act(QStringLiteral("edit.redo"))))));
        const QString undoTip = w.act(QStringLiteral("edit.undo"))->toolTip();
        const QKeySequence key(Qt::CTRL | Qt::ALT | Qt::Key_9);
        KeyMap map(&w);
        QCOMPARE(int(map.add(QStringLiteral("edit.redo"), key).status), int(KeyMap::Add::Added));
        QCOMPARE(int(map.add(QStringLiteral("edit.selectAll"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_8)).status), int(KeyMap::Add::Added));
        map.save();
        map.applyAll();
        QCOMPARE(w.act(QStringLiteral("edit.undo"))->toolTip(), undoTip);   // (a tooltip the window wrote is left as it is)
        w.refreshNow();   // (the window rewrites them at each change)
        const QString shown = key.toString(QKeySequence::NativeText);
        QVERIFY2(w.act(QStringLiteral("edit.redo"))->toolTip().endsWith(QStringLiteral("(%1)").arg(shown)), qPrintable(w.act(QStringLiteral("edit.redo"))->toolTip()));
        QVERIFY(!w.act(QStringLiteral("edit.redo"))->toolTip().contains(QStringLiteral("Ctrl+Y")));
    }

    // ---- Notes Page, Notes Master, and Handout Master views (November 15, 2026)

    // The Notes Page view shows the slide's page: the slide above, its notes below, typed in place as one step that undoes; PageDown goes on, and the zoom controls zoom it.
    void theNotesPageViewShowsTheSlideAboveItsNotes()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.newSlide();
        Document *d = w.editor()->doc();
        QVERIFY(d->pages.size() >= 2);
        w.editor()->setCurrentPage(0);
        QVERIFY(w.act("view.notesPage"));
        w.act("view.notesPage")->trigger();
        QCOMPARE(w.view(), DeckWindow::View::NotesPage);
        NotesPageView *v = w.notesPageView();
        QVERIFY(v);
        QTRY_VERIFY(v->isVisible() && v->notesBox()->isVisible());
        QTRY_VERIFY(v->viewport()->width() > 300);
        // The page fits in the view, the slide above the notes, both on the page.
        const QRectF page = v->pageRect(), slide = v->slideImageRect(), notes = v->notesRect();
        QVERIFY(page.width() > 100 && page.height() > page.width());
        QVERIFY(page.adjusted(-1, -1, 1, 1).contains(slide) && page.adjusted(-1, -1, 1, 1).contains(notes));
        QVERIFY2(slide.bottom() <= notes.top(), "the notes are under the slide");
        QVERIFY(qAbs(slide.width() / slide.height() - d->pageSize().width() / d->pageSize().height()) < 0.01);
        QCOMPARE(v->notesBox()->geometry(), notes.toRect());
        // The notes are typed on the page: one undo step.
        v->notesBox()->setFocus();
        v->notesBox()->setPlainText(QStringLiteral("Typed on the page"));
        v->commit();
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Typed on the page"));
        w.editor()->undo();
        QVERIFY(deck::notesText(*d, *d->pages[0]).isEmpty());
        v->notesBox()->setPlainText(QStringLiteral("Typed on the page"));
        v->commit();
        // PageDown goes to the next slide, which shows its own (no) notes.
        QTest::keyClick(v, Qt::Key_PageDown);
        QCOMPARE(w.editor()->currentPage(), 1);
        QVERIFY(v->notesBox()->toPlainText().isEmpty());
        QTest::keyClick(v, Qt::Key_PageUp);
        QCOMPARE(v->notesBox()->toPlainText(), QStringLiteral("Typed on the page"));
        // The zoom controls zoom the page.
        const double before = v->zoom();
        w.act("zoom.in")->trigger();
        QVERIFY2(v->zoom() > before, "zoom in");
        QVERIFY(v->pageRect().width() > page.width() * 1.01);
        w.act("zoom.out")->trigger();
        w.act("zoom.out")->trigger();
        QVERIFY2(v->zoom() < before, "zoom out");
        shotOf(&w, QStringLiteral("deck-notes-page-view"));
        // Back to Normal view: the slide and the notes under it.
        w.act("view.normal")->trigger();
        QCOMPARE(w.view(), DeckWindow::View::Normal);
        QVERIFY(!v->isVisible() && w.canvas()->isVisible());
        QCOMPARE(w.canvas()->zoom() > 0.05, true);
    }

    // The Notes Master view: the places of the notes page are dragged, each change a step that undoes; the header and footer choice, the background, and Reset Layout work on it, and Close
    // Master View returns to the view it was opened from. What it sets is in the printed page.
    void theNotesMasterViewEditsThePlacesOfTheNotesPage()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.act("view.notesMaster")->trigger();
        QCOMPARE(w.view(), DeckWindow::View::NotesMaster);
        NotesPageView *v = w.notesPageView();
        QTRY_VERIFY(v->isVisible() && v->viewport()->width() > 300);
        QVERIFY(!v->notesBox()->isVisible());
        Document *d = w.editor()->doc();
        QVERIFY(!deck::hasNotesMaster(*d));
        // Drag the footer's place up and to the left, by its middle.
        const QRectF was = v->placeRect(deck::NotesPart::Footer);
        const QPoint from = was.center().toPoint(), to = from + QPoint(-20, -120);
        QTest::mousePress(v->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(v->viewport(), to);
        QTest::mouseRelease(v->viewport(), Qt::LeftButton, Qt::NoModifier, to);
        QVERIFY(deck::hasNotesMaster(*d));
        const QRectF now = v->placeRect(deck::NotesPart::Footer);
        QVERIFY2(qAbs(now.top() - (was.top() - 120)) < 2 && qAbs(now.left() - (was.left() - 20)) < 2 && qAbs(now.width() - was.width()) < 1, "the place moved by the drag, and kept its size");
        // Size the header's place by its lower right corner (a place is selected by a click first).
        const QRectF headerWas = v->placeRect(deck::NotesPart::Header);
        QTest::mouseClick(v->viewport(), Qt::LeftButton, Qt::NoModifier, headerWas.center().toPoint());
        const QPoint corner = QPoint(int(headerWas.right()) - 3, int(headerWas.bottom()) - 3);
        QTest::mousePress(v->viewport(), Qt::LeftButton, Qt::NoModifier, corner);
        QTest::mouseMove(v->viewport(), corner + QPoint(30, 10));
        QTest::mouseRelease(v->viewport(), Qt::LeftButton, Qt::NoModifier, corner + QPoint(30, 10));
        QVERIFY2(qAbs(v->placeRect(deck::NotesPart::Header).width() - (headerWas.width() + 30)) < 2, "the place is sized by its corner");
        // The master is on the printed page: the footer's number moved with its place.
        QVERIFY(deck::notesMasterOf(*d, deck::localePaper()).footer.top() < deck::defaultNotesMaster(deck::localePaper(), d->pageSize()).footer.top());
        // Each is a step that undoes.
        w.editor()->undo();
        w.editor()->undo();
        QVERIFY(!deck::hasNotesMaster(*d));
        // The check boxes of the tab are the notes pages' header and footer choice, and the background is the master's.
        w.act("pagemaster.date")->trigger();
        QVERIFY(deck::printHeaderFooter(*d, deck::PrintedPages::Notes).date);
        v->setBackground(QColor(0xDD, 0xEE, 0xFF));
        QCOMPARE(deck::notesMasterOf(*d, deck::localePaper()).background, QColor(0xDD, 0xEE, 0xFF));
        shotOf(&w, QStringLiteral("deck-notes-master"));
        v->resetLayout();
        QVERIFY(!deck::hasNotesMaster(*d));
        // Close Master View returns to where it was opened from.
        w.act("pagemaster.close")->trigger();
        QCOMPARE(w.view(), DeckWindow::View::Normal);
        QVERIFY(w.canvas()->isVisible());
        // Opened from Notes Page, it returns to Notes Page.
        w.act("view.notesPage")->trigger();
        w.act("view.notesMaster")->trigger();
        w.act("pagemaster.close")->trigger();
        QCOMPARE(w.view(), DeckWindow::View::NotesPage);
    }

    // The Handout Master view: how many slides a page holds and how it lies, the header, date, footer, and number, and the background; each a step that undoes.
    void theHandoutMasterViewEditsTheHandout()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.act("view.handoutMaster")->trigger();
        QCOMPARE(w.view(), DeckWindow::View::HandoutMaster);
        NotesPageView *v = w.notesPageView();
        QTRY_VERIFY(v->isVisible() && v->viewport()->width() > 300);
        Document *d = w.editor()->doc();
        const QSizeF paper = deck::localePaper();
        QCOMPARE(deck::handoutMasterOf(*d, paper).perPage, 6);
        QVERIFY(w.act("handout.per.6")->isChecked() && w.act("handout.portrait")->isChecked());
        w.act("handout.per.9")->trigger();
        QCOMPARE(deck::handoutMasterOf(*d, paper).perPage, 9);
        QVERIFY(w.act("handout.per.9")->isChecked() && !w.act("handout.per.6")->isChecked());
        w.editor()->undo();
        QCOMPARE(deck::handoutMasterOf(*d, paper).perPage, 6);
        w.act("handout.per.3")->trigger();
        QVERIFY(v->pageRect().height() > v->pageRect().width());
        // Lying across: the page is wider than tall, and the printed handout is too.
        w.act("handout.landscape")->trigger();
        QVERIFY(deck::handoutMasterOf(*d, paper).landscape);
        QTRY_VERIFY(v->pageRect().width() > v->pageRect().height());
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::Handout3;
        QCOMPARE(deck::outputPageSize(*d, o), paper.transposed());
        shotOf(&w, QStringLiteral("deck-handout-master"));
        w.act("handout.portrait")->trigger();
        QTRY_VERIFY(v->pageRect().height() > v->pageRect().width());
        // Drag the slides' area (by its middle: no smaller place is on top there).
        const QRectF area = v->placeRect(deck::HandoutPart::Area);
        const QPoint from = area.center().toPoint(), to = from + QPoint(15, 25);
        QTest::mousePress(v->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(v->viewport(), to);
        QTest::mouseRelease(v->viewport(), Qt::LeftButton, Qt::NoModifier, to);
        QVERIFY2(qAbs(v->placeRect(deck::HandoutPart::Area).left() - (area.left() + 15)) < 2, "the slides' area moved");
        // The header choice of the tab is the handouts' (and not the notes pages').
        const bool notesDate = deck::printHeaderFooter(*d, deck::PrintedPages::Notes).date;
        const bool handoutDate = deck::printHeaderFooter(*d, deck::PrintedPages::Handouts).date;
        w.act("pagemaster.date")->trigger();
        QCOMPARE(deck::printHeaderFooter(*d, deck::PrintedPages::Handouts).date, !handoutDate);
        QCOMPARE(deck::printHeaderFooter(*d, deck::PrintedPages::Notes).date, notesDate);
        w.act("pagemaster.close")->trigger();
        QCOMPARE(w.view(), DeckWindow::View::Normal);
    }

    // The labels of the places in the master views fit their boxes (cut short with "..." to the width, in a size the height holds) at the smallest zoom and at a normal one, in the
    // small header, date, footer, and number boxes too.
    void masterLabelsFitTheirBoxes()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        const QFont base = w.font();
        for (const char *view : {"view.notesMaster", "view.handoutMaster"}) {
            w.act(view)->trigger();
            NotesPageView *v = w.notesPageView();
            QTRY_VERIFY(v->isVisible() && v->viewport()->width() > 300);
            for (double zoom : {0.1, 0.25, 0.5, 1.0}) {
                v->setZoom(zoom);
                const double scale = v->zoom() * v->logicalDpiX() / 72.0;
                const QStringList texts{QStringLiteral("Header"), QStringLiteral("Date"), QStringLiteral("Footer"), QStringLiteral("Page Number"), QStringLiteral("Slide Image")};
                const QVector<QRectF> boxes = QString::fromLatin1(view) == QLatin1String("view.notesMaster")
                    ? QVector<QRectF>{v->placeRect(deck::NotesPart::Header), v->placeRect(deck::NotesPart::Date), v->placeRect(deck::NotesPart::Footer), v->placeRect(deck::NotesPart::Number), v->placeRect(deck::NotesPart::SlideImage)}
                    : QVector<QRectF>{v->placeRect(deck::HandoutPart::Header), v->placeRect(deck::HandoutPart::Date), v->placeRect(deck::HandoutPart::Footer), v->placeRect(deck::HandoutPart::Number), v->placeRect(deck::HandoutPart::Area)};
                for (int i = 0; i < boxes.size(); ++i) {
                    const QFont f = NotesPageView::labelFont(base, boxes[i], scale);
                    const QString shown = NotesPageView::labelText(texts[i], f, boxes[i]);
                    const QString where = QStringLiteral("%1 zoom %2 box %3").arg(QLatin1String(view)).arg(zoom).arg(i);
                    QVERIFY2(f.pixelSize() <= boxes[i].height(), qPrintable(where));
                    if (shown.isEmpty()) continue;
                    QVERIFY2(QFontMetricsF(f).horizontalAdvance(shown) <= boxes[i].width(), qPrintable(where));
                    QVERIFY2(QFontMetricsF(f).height() <= boxes[i].height() + 3, qPrintable(where + QStringLiteral(" height")));
                }
            }
            // At a normal zoom the words are there, whole, in the big boxes.
            v->setZoom(1.0);
            QCOMPARE(NotesPageView::labelText(QStringLiteral("Slide Image"), NotesPageView::labelFont(base, QRectF(0, 0, 400, 300), 1.33), QRectF(0, 0, 400, 300)), QStringLiteral("Slide Image"));
            QVERIFY(NotesPageView::labelText(QStringLiteral("Slide Image"), NotesPageView::labelFont(base, QRectF(0, 0, 30, 12), 1.33), QRectF(0, 0, 30, 12)).endsWith(QChar(0x2026)));
            w.act("view.normal")->trigger();
        }
        // Screenshots at the smallest and a normal zoom.
        w.act("view.handoutMaster")->trigger();
        QTRY_VERIFY(w.notesPageView()->isVisible());
        w.notesPageView()->setZoom(1.0);
        shotOf(&w, QStringLiteral("deck-handout-master"));
        w.act("view.notesMaster")->trigger();
        w.notesPageView()->setZoom(1.0);
        shotOf(&w, QStringLiteral("deck-notes-master"));
        w.notesPageView()->setZoom(0.1);
        shotOf(&w, QStringLiteral("deck-notes-master-smallest"));
    }

    // A drag reads the layout from the presentation once, not for each move of the mouse; a change is read again.
    void aDragDoesNotReadTheLayoutForEachMove()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.act("view.notesMaster")->trigger();
        NotesPageView *v = w.notesPageView();
        QTRY_VERIFY(v->isVisible() && v->viewport()->width() > 300);
        const QPoint from = v->placeRect(deck::NotesPart::Footer).center().toPoint();
        QTest::mousePress(v->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        const int before = v->layoutReads();
        for (int i = 1; i <= 40; ++i) {
            QTest::mouseMove(v->viewport(), from + QPoint(-(i * 15) / 40, -2 * i));
            v->viewport()->repaint();   // (a paint for each move, as the screen would make: the paint reads the layout too)
        }
        QVERIFY2(v->layoutReads() - before <= 1, qPrintable(QString::number(v->layoutReads() - before)));
        QTest::mouseRelease(v->viewport(), Qt::LeftButton, Qt::NoModifier, from + QPoint(-15, -80));
        // The change is seen: the footer is where it was dropped.
        QVERIFY(deck::hasNotesMaster(*w.editor()->doc()));
        QVERIFY(qAbs(v->placeRect(deck::NotesPart::Footer).center().x() - (from.x() - 15)) < 2);
        w.editor()->undo();
        QVERIFY(qAbs(v->placeRect(deck::NotesPart::Footer).center().x() - from.x()) < 2);
    }

    // The mini toolbar floats above words selected with the mouse, shows the selection's state at once, and never takes the keyboard.
    void theMiniToolbarFloatsByWordsSelectedWithTheMouse()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        auto rig = [&] {
            const auto *title = placeholderOfSlide(*ed->doc(), 0, 0);
        const QString boxId = title->id, storyId = title->storyId;
        QTextCursor(ed->doc()->storyDoc(storyId)).insertText(minitest::kText);
        return minitest::Rig<DeckWindow>(w, boxId, storyId, 0);
        }();
        minitest::floats(w, rig, QStringLiteral("deck-mini-toolbar"));
    }

    // The option "Show Mini Toolbar on selection" turned off takes a bar that is showing away at once.
    void theMiniToolbarGoesWhenTheOptionIsTurnedOff()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        auto rig = [&] {
            const auto *title = placeholderOfSlide(*ed->doc(), 0, 0);
        const QString boxId = title->id, storyId = title->storyId;
        QTextCursor(ed->doc()->storyDoc(storyId)).insertText(minitest::kText);
        return minitest::Rig<DeckWindow>(w, boxId, storyId, 0);
        }();
        minitest::goesWhenTheOptionIsTurnedOff(rig);
    }

    // The font box, the color button, an alignment, bullets, and the size buttons each change the selected words in one step of Undo.
    void theMiniToolbarsControlsChangeTheSelectionInOneStepEach()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        auto rig = [&] {
            const auto *title = placeholderOfSlide(*ed->doc(), 0, 0);
        const QString boxId = title->id, storyId = title->storyId;
        QTextCursor(ed->doc()->storyDoc(storyId)).insertText(minitest::kText);
        return minitest::Rig<DeckWindow>(w, boxId, storyId, 0);
        }();
        minitest::buttonsChangeTheSelection(w, rig, "fmt.alignRight");
    }

    // The bar hides on typing, Escape, scrolling, zoom, and resize, and when the words go (undo, a deleted box).
    void theMiniToolbarGoesWhenTheViewOrTheWordsMove()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        auto rig = [&] {
            const auto *title = placeholderOfSlide(*ed->doc(), 0, 0);
        const QString boxId = title->id, storyId = title->storyId;
        QTextCursor(ed->doc()->storyDoc(storyId)).insertText(minitest::kText);
        return minitest::Rig<DeckWindow>(w, boxId, storyId, 0);
        }();
        minitest::goesWhenTheWordsMove(w, rig, "fmt.alignRight");
    }

    // The Grid and Guides dialog of the View tab sets the grid's spacing and what snaps, saves them with the presentation as one step of Undo, and shows the grid.
    void theGridDialogSetsSpacingAndSnap()
    {
        const auto keep = keepGridSettings();
        DeckWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        QCOMPARE(ed->view.gridSpacing, 6.0);
        QVERIFY(!ed->view.snapGrid);
        QVERIFY(!w.canvas()->gridlines());
        withDialog([&] { w.act(QStringLiteral("view.gridGuides"))->trigger(); }, QStringLiteral("gridGuidesDialog"), [&](QDialog *d) {
            auto *spacing = d->findChild<QDoubleSpinBox *>(QStringLiteral("gridSpacing"));
            QVERIFY(spacing);
            spacing->setValue(18);
            d->findChild<QCheckBox *>(QStringLiteral("snapGrid"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("showGrid"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("snapObjects"))->setChecked(false);
        });
        QCOMPARE(ed->view.gridSpacing, 18.0);
        QVERIFY(ed->view.snapGrid);
        QVERIFY(!ed->view.snapObjects);
        QVERIFY(w.canvas()->gridlines());
        const deck::GridView saved = deck::gridViewOf(*ed->doc());
        QCOMPARE(saved.spacing.value_or(0), 18.0);
        QVERIFY(saved.snapGrid.value_or(false));
        QVERIFY(!saved.snapObjects.value_or(true));
        QVERIFY(!Settings::get().value(QStringLiteral("view/snapObjects"), true).toBool());   // (the last choice is kept for other presentations)
        ed->undo();
        // (the file has none again; the view uses the last choice the program kept)
        QVERIFY(!deck::gridViewOf(*ed->doc()).spacing && !deck::gridViewOf(*ed->doc()).snapGrid);
        // Cancel changes nothing.
        const bool snapBefore = ed->view.snapGrid;
        withDialog([&] { w.act(QStringLiteral("view.gridGuides"))->trigger(); }, QStringLiteral("gridGuidesDialog"), [&](QDialog *d) {
            d->findChild<QCheckBox *>(QStringLiteral("snapGrid"))->setChecked(!snapBefore);
            d->findChild<QCheckBox *>(QStringLiteral("snapObjects"))->setChecked(false);
        }, false);
        QCOMPARE(ed->view.snapGrid, snapBefore);
        QVERIFY(!deck::gridViewOf(*ed->doc()).snapGrid);
    }

    // The dialog adds guides of both kinds to the presentation (never two at one place), shows them, and offers no more past the most one way.
    void theGridDialogAddsGuides()
    {
        const auto keep = keepGridSettings();
        DeckWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        QVERIFY(!w.canvas()->guides());
        withDialog([&] { w.act(QStringLiteral("view.gridGuides"))->trigger(); }, QStringLiteral("gridGuidesDialog"), [&](QDialog *d) {
            auto *addV = d->findChild<QPushButton *>(QStringLiteral("addVertical"));
            auto *addH = d->findChild<QPushButton *>(QStringLiteral("addHorizontal"));
            QVERIFY(addV && addH);
            addV->click();
            addV->click();
            addH->click();
        });
        const RulerGuides &g = ed->doc()->viewGuides;
        QCOMPARE(g.v.size(), 2);
        QCOMPARE(g.h.size(), 1);
        QVERIFY(std::abs(g.v[0] - g.v[1]) > 0.5);
        QVERIFY(std::abs(g.h[0] - ed->doc()->pageSize().height() / 2) < 1e-9);
        QVERIFY(w.canvas()->guides());
        QVERIFY(ed->doc()->pages[0]->guides.v.isEmpty());   // (the presentation's, not the slide's)
        ed->undo();
        QVERIFY(ed->doc()->viewGuides.v.isEmpty() && ed->doc()->viewGuides.h.isEmpty());
        // At the most, the button is off.
        for (int i = 0; i < kMaxGuidesPerAxis; ++i) ed->doc()->viewGuides.v << 5.0 + i;
        withDialog([&] { w.act(QStringLiteral("view.gridGuides"))->trigger(); }, QStringLiteral("gridGuidesDialog"), [&](QDialog *d) {
            QVERIFY(!d->findChild<QPushButton *>(QStringLiteral("addVertical"))->isEnabled());
            QVERIFY(d->findChild<QPushButton *>(QStringLiteral("addHorizontal"))->isEnabled());
        }, false);
    }

    // A connector drawn to a triangle's foot is glued to the nearest of the triangle's real sites, as in the other programs: the bottom left corner is its third site (number 2), the bottom
    // right its fifth (number 4), and a heart, which has two, offers its top and bottom only.
    void aConnectorDrawnInTheDeckGluesToTheNearestRealSite()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        ed->surfaceItems().clear();
        auto triangle = std::make_shared<ShapeItem>(), heart = std::make_shared<ShapeItem>(), box = std::make_shared<ShapeItem>();
        triangle->shape = QStringLiteral("triangle");
        triangle->rect = QRectF(100, 100, 400, 100);
        heart->shape = QStringLiteral("heart");
        heart->rect = QRectF(100, 300, 100, 100);
        box->rect = QRectF(700, 100, 100, 60);
        for (const auto &s : {triangle, heart, box}) ed->addItem(s, false);
        Canvas *c = w.canvas();
        QWidget *vp = c->viewport();
        const auto view = [&](QPointF page) { return c->pageToView(page).toPoint(); };
        const auto drag = [&](QPointF from, QPointF to) {
            QTest::qWait(QApplication::doubleClickInterval() + 50);
            QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(from));
            for (const QPointF at : {(from + to) / 2, to}) {
                const QPoint v = view(at);
                QMouseEvent mv(QEvent::MouseMove, v, vp->mapToGlobal(v), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(vp, &mv);
            }
            QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(to));
        };
        const auto lastLine = [&]() { return std::dynamic_pointer_cast<LineItem>(ed->surfaceItems().back()); };
        ed->setTool(Tool::Arrow, QString());
        // From a few points left of the triangle's bottom left corner (100, 200) to the box's left side.
        drag(QPointF(104, 197), QPointF(698, 130));
        auto line = lastLine();
        QVERIFY(line);
        QCOMPARE(line->start.id, triangle->id);
        QCOMPARE(line->start.site, 2);
        QVERIFY(std::abs(line->p1.x() - 100) < 1e-6 && std::abs(line->p1.y() - 200) < 1e-6);
        ed->setTool(Tool::Arrow, QString());
        drag(QPointF(496, 197), QPointF(698, 130));
        line = lastLine();
        QCOMPARE(line->start.id, triangle->id);
        QCOMPARE(line->start.site, 4);
        QVERIFY(std::abs(line->p1.x() - 500) < 1e-6 && std::abs(line->p1.y() - 200) < 1e-6);
        // The heart's bottom is its second site, and it has no site on its left side to be near.
        ed->setTool(Tool::Arrow, QString());
        drag(QPointF(150, 396), QPointF(698, 130));
        line = lastLine();
        QCOMPARE(line->start.id, heart->id);
        QCOMPARE(line->start.site, 1);
        QCOMPARE(connectionSiteCount(*heart), 2);
        QCOMPARE(connectionSiteCount(*triangle), 6);
        QCOMPARE(connectionSiteCount(*box), 4);
    }

    // A save from before the sites were counted the standard's way names a side of the shape (0 top, 1 left, 2 bottom, 3 right); it is made a site when the file is opened, so that saving
    // again, copying, and undoing never see the side as a site.
    void anOldSaveNamingASideIsGluedToTheRightSiteWhateverHappensNext()
    {
        auto source = Document::blank(QSizeF(960, 540));
        auto hex = std::make_shared<ShapeItem>(), box = std::make_shared<ShapeItem>();
        hex->shape = QStringLiteral("hexagon");
        hex->rect = QRectF(100, 100, 200, 100);
        box->rect = QRectF(500, 100, 100, 60);
        auto line = std::make_shared<LineItem>();
        line->start = LineItem::Glue{hex->id, 3};   // (the right side)
        line->end = LineItem::Glue{box->id, 1};
        for (const auto &it : {ItemPtr(hex), ItemPtr(box), ItemPtr(line)}) source->pages[0]->items.push_back(it);
        // (What 0.42 wrote: the same, with no "model" in the glue.)
        std::function<QJsonValue(const QJsonValue &)> strip = [&](const QJsonValue &v) -> QJsonValue {
            if (v.isArray()) {
                QJsonArray a;
                for (const auto &e : v.toArray()) a.append(strip(e));
                return a;
            }
            if (!v.isObject()) return v;
            QJsonObject o = v.toObject();
            for (auto it = o.begin(); it != o.end(); ++it) {
                if ((it.key() == QLatin1String("startGlue") || it.key() == QLatin1String("endGlue")) && it.value().isObject()) {
                    QJsonObject g = it.value().toObject();
                    g.remove(QStringLiteral("model"));
                    it.value() = g;
                } else {
                    it.value() = strip(it.value());
                }
            }
            return o;
        };
        const QJsonObject old = strip(source->toJson()).toObject();
        const auto open = [&]() {
            auto d = std::make_unique<Document>();
            d->fromJson(old);
            return d;
        };
        const auto glueOf = [](const Document &d, bool start) {
            LineItem::Glue g;
            walkItems(d.pages[0]->items, [&](const ItemPtr &it) {
                if (it->type() == ItemType::Line) g = start ? static_cast<const LineItem &>(*it).start : static_cast<const LineItem &>(*it).end;
            });
            return g;
        };
        const auto hexOf = [](const Document &d) {
            const Item *found = nullptr;
            walkItems(d.pages[0]->items, [&](const ItemPtr &it) {
                if (it->type() == ItemType::Shape && static_cast<const ShapeItem &>(*it).shape == QLatin1String("hexagon")) found = it.get();
            });
            return found;
        };
        auto first = open();
        const int right = connectionSiteOnSide(*hexOf(*first), 3);
        QVERIFY(right >= 0 && right != 3);
        // Opened and saved with no edit: the site, not the side.
        QCOMPARE(glueOf(*first, true).site, right);
        QJsonObject saved = first->toJson();
        auto again = std::make_unique<Document>();
        again->fromJson(saved);
        QCOMPARE(glueOf(*again, true).site, right);
        QCOMPARE(glueOf(*again, true).id, hexOf(*again)->id);
        QVERIFY(!glueOf(*first, true).sideNumber);
        // Copied and pasted (here duplicated: the same cloning): the copy's glue is on the copy's hexagon, at the same site.
        Editor ed;
        ed.setDocument(open());
        QStringList all;
        walkItems(ed.surfaceItems(), [&](const ItemPtr &it) { all << it->id; });
        ed.select(all);
        ed.duplicateSelection();
        int lines = 0, glued = 0;
        const Item *copiedHex = nullptr;
        walkItems(ed.surfaceItems(), [&](const ItemPtr &it) {
            if (it->type() == ItemType::Shape && static_cast<const ShapeItem &>(*it).shape == QLatin1String("hexagon") && it.get() != hexOf(*ed.doc())) copiedHex = it.get();
        });
        walkItems(ed.surfaceItems(), [&](const ItemPtr &it) {
            if (it->type() != ItemType::Line) return;
            ++lines;
            const auto &l = static_cast<const LineItem &>(*it);
            if (copiedHex && l.start.id == copiedHex->id && l.start.site == right && !l.start.sideNumber) ++glued;
        });
        QCOMPARE(lines, 2);
        QVERIFY(copiedHex);
        QCOMPARE(glued, 1);
        // The first edit, undone: back to the right site.
        Editor ed2;
        ed2.setDocument(open());
        ed2.beginChange(QStringLiteral("Move"));
        const_cast<Item *>(hexOf(*ed2.doc()))->moveBy(30, 0);
        ed2.endChange();
        QCOMPARE(glueOf(*ed2.doc(), true).site, right);
        ed2.undo();
        QCOMPARE(glueOf(*ed2.doc(), true).site, right);
        QVERIFY(!glueOf(*ed2.doc(), true).sideNumber);
        ed2.redo();
        QCOMPARE(glueOf(*ed2.doc(), true).site, right);
        // An end still holding a side (made by hand: opening a document never leaves one) keeps that through a copy.
        LineItem held;
        held.start = LineItem::Glue{QStringLiteral("a"), 3, true};
        ItemList list{std::make_shared<LineItem>(held)};
        remapGlue(list, QHash<QString, QString>{{QStringLiteral("a"), QStringLiteral("b")}});
        QVERIFY(static_cast<LineItem &>(*list[0]).start.sideNumber);
        QCOMPARE(static_cast<LineItem &>(*list[0]).start.id, QStringLiteral("b"));
    }

    // A callout's pointer is drawn outside its frame; a click on the pointer selects the callout, and the selection's bounds reach the pointer.
    void aClickOnACalloutsPointerOutsideItsFrameSelectsIt()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        ed->surfaceItems().clear();
        auto callout = std::make_shared<ShapeItem>();
        callout->shape = QStringLiteral("wedgeRectCallout");
        callout->rect = QRectF(100, 100, 200, 100);
        ed->addItem(callout, false);
        ed->select(QStringList());
        Canvas *c = w.canvas();
        // The tip is at 0.29 of the width and 1.125 of the height from the frame's top left: 12.5 points below the frame.
        const QPointF tip(100 + 0.29 * 200, 100 + 1.1 * 100);
        QVERIFY(!callout->rect.contains(tip));
        QTest::mouseClick(c->viewport(), Qt::LeftButton, Qt::NoModifier, c->pageToView(tip).toPoint());
        QCOMPARE(ed->selection(), QStringList{callout->id});
        QVERIFY(ed->selectionBounds().bottom() > 212.4);
        // Beside the pointer nothing is.
        ed->select(QStringList());
        QTest::mouseClick(c->viewport(), Qt::LeftButton, Qt::NoModifier, c->pageToView(QPointF(100 + 0.9 * 200, 100 + 1.1 * 100)).toPoint());
        QVERIFY(ed->selection().isEmpty());
    }

    // An object dragged on a grid lands on it (its edge on a multiple of the spacing, worked out from the grid, not from pixels); Alt leaves snapping off.
    void aDraggedObjectSnapsToTheGrid()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        ed->surfaceItems().clear();
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(107, 68, 114, 80);
        ed->addItem(shape, false);
        const QString id = shape->id;
        ed->clearSelection();
        deck::GridView grid;   // (the presentation's choices, which the view follows)
        grid.spacing = 18;
        grid.snapGrid = true;
        grid.snapObjects = false;
        deck::setGridView(*ed->doc(), grid);
        ed->setView([](ViewOptions &v) {
            v.snapGrid = true;
            v.gridSpacing = 18;
            v.snapObjects = false;
            v.snapGuides = false;
        });
        const QPointF startCenter = shape->rect.center();
        auto drag = [&](QPointF by, Qt::KeyboardModifiers mods) {
            QWidget *vp = w.canvas()->viewport();
            const QPoint a = w.canvas()->pageToView(startCenter).toPoint(), b = w.canvas()->pageToView(startCenter + by).toPoint();
            QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, a);
            QMouseEvent step(QEvent::MouseMove, (a + b) / 2, vp->mapToGlobal((a + b) / 2), Qt::NoButton, Qt::LeftButton, mods);
            QApplication::sendEvent(vp, &step);
            QMouseEvent last(QEvent::MouseMove, b, vp->mapToGlobal(b), Qt::NoButton, Qt::LeftButton, mods);
            QApplication::sendEvent(vp, &last);
            QTest::mouseRelease(vp, Qt::LeftButton, mods, b);
        };
        // Alt: where the mouse put it, not on the grid.
        drag(QPointF(40, 25), Qt::AltModifier);
        const QRectF free = ed->doc()->item(id)->bounds();
        QVERIFY(std::abs(free.left() - 147) < 3 && std::abs(free.top() - 93) < 3);
        QVERIFY(std::abs(free.left() / 18 - std::round(free.left() / 18)) > 0.02 && std::abs(free.top() / 18 - std::round(free.top() / 18)) > 0.02);
        ed->undo();
        QCOMPARE(ed->doc()->item(id)->bounds().left(), 107.0);
        // Without it: an edge on the grid, the nearer of the two on each side.
        drag(QPointF(40, 25), Qt::NoModifier);
        const QRectF now = ed->doc()->item(id)->bounds();
        auto onGrid = [](double v) { return std::abs(v / 18 - std::round(v / 18)) < 1e-9; };
        QVERIFY(onGrid(now.left()) || onGrid(now.right()));
        QVERIFY(onGrid(now.top()) || onGrid(now.bottom()));
        QCOMPARE(now.left(), 144.0);   // (at 147: its left edge is 3 from 144, its right edge, 261, 9 from the nearest line)
        QCOMPARE(now.top(), 90.0);     // (at 93: its top edge is 3 from 90, its bottom edge, 173, 7 from 180)
        QCOMPARE(now.width(), 114.0);
    }

    // An object or guide that is exactly where the point is still wins over the grid (the point is not moved by the grid then).
    void anObjectSnapBeatsTheGridOnTheExactPoint()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        ed->surfaceItems().clear();
        QWidget *vp = w.canvas()->viewport();
        // A pixel whose page position is off the grid (the spacing is 18).
        QPoint at;
        QPointF page;
        for (int x = 300;; ++x) {
            at = vp->mapToGlobal(QPoint(x, 200));
            page = w.canvas()->toPage(QPointF(vp->mapFromGlobal(at)));
            if (std::abs(page.x() / 18 - std::round(page.x() / 18)) > 0.2) break;
        }
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(page.x(), 20, 50, 30);   // (its left edge is exactly the point)
        ed->addItem(shape, false);
        ed->setView([](ViewOptions &v) {
            v.snapGrid = true;
            v.gridSpacing = 18;
            v.snapObjects = true;
        });
        w.canvas()->startGuideDrag(Qt::Vertical, at);
        w.canvas()->updateGuideDrag(at);
        w.canvas()->endGuideDrag(at);
        QCOMPARE(ed->doc()->viewGuides.v.size(), 1);
        QCOMPARE(ed->doc()->viewGuides.v[0], page.x());
    }

    // The view takes the presentation's grid settings again when they change (an undo of the dialog hides the guides it showed), and not at every edit.
    void theGridViewIsReappliedOnlyWhenItsSettingsChange()
    {
        const auto keep = keepGridSettings();
        Settings::get().setValue(QStringLiteral("view/snapGrid"), false);
        DeckWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        withDialog([&] { w.act(QStringLiteral("view.gridGuides"))->trigger(); }, QStringLiteral("gridGuidesDialog"), [&](QDialog *d) {
            d->findChild<QPushButton *>(QStringLiteral("addVertical"))->click();
        });
        QVERIFY(w.canvas()->guides());
        ed->undo();
        QVERIFY(!w.canvas()->guides());
        // An edit that is not about the grid leaves the view as it is (a changed program setting is not picked up by it).
        Settings::get().setValue(QStringLiteral("view/snapGrid"), true);
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(10, 10, 50, 30);
        ed->addItem(shape, false);
        QVERIFY(!ed->view.snapGrid);
    }

    // A spacing in the program's settings that is not a number is the program's own spacing.
    void aBrokenSpacingInTheSettingsIsIgnored()
    {
        const auto keep = keepGridSettings();
        Settings::get().setValue(QStringLiteral("view/gridSpacing"), QStringLiteral("nan"));
        DeckWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(deck::newPresentation());
        QCOMPARE(w.editor()->view.gridSpacing, 6.0);
    }

    // A guide dragged from the ruler is the presentation's, snaps to the grid, follows the mouse in small steps (not held where it started), and undoes.
    void aGuideDraggedFromTheRulerSnapsAndUndoes()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        auto global = [&](double x, double y) { return w.canvas()->viewport()->mapToGlobal(w.canvas()->pageToView(QPointF(x, y)).toPoint()); };
        deck::GridView grid;
        grid.spacing = 18;
        grid.snapGrid = true;
        grid.snapObjects = false;
        ed->change(QStringLiteral("Grid"), [&] { deck::setGridView(*ed->doc(), grid); });   // (as the dialog sets it)
        ed->setView([](ViewOptions &v) {
            v.snapGrid = true;
            v.gridSpacing = 18;
            v.snapObjects = false;
        });
        w.canvas()->startGuideDrag(Qt::Vertical, global(200, 100));
        w.canvas()->updateGuideDrag(global(300, 100));
        w.canvas()->endGuideDrag(global(300, 100));
        QCOMPARE(ed->doc()->viewGuides.v.size(), 1);
        QCOMPARE(ed->doc()->viewGuides.v[0], 306.0);   // (300 is 6 from 306 and 12 from 288)
        QVERIFY(ed->doc()->pages[0]->guides.v.isEmpty());
        QVERIFY(w.canvas()->guides());   // (a guide dragged out is shown)
        ed->undo();
        QVERIFY(ed->doc()->viewGuides.v.isEmpty());
        // In small steps with nothing to snap to: it goes where the mouse goes.
        grid.snapGrid = false;
        ed->change(QStringLiteral("Grid"), [&] { deck::setGridView(*ed->doc(), grid); });   // (as the dialog sets it)
        ed->setView([](ViewOptions &v) { v.snapGrid = false; });
        w.canvas()->startGuideDrag(Qt::Horizontal, global(300, 200));
        for (int y = 203; y <= 215; y += 3) w.canvas()->updateGuideDrag(global(300, y));
        w.canvas()->endGuideDrag(global(300, 215));
        QCOMPARE(ed->doc()->viewGuides.h.size(), 1);
        QVERIFY(std::abs(ed->doc()->viewGuides.h[0] - 215) < 3);
    }

    // Pictures for the change's notes (only with JO_SHOT_DIR set): a slide with the grid and two guides, and the dialog.
    void gridAndGuidesPictures()
    {
        if (qEnvironmentVariable("JO_SHOT_DIR").isEmpty()) QSKIP("no JO_SHOT_DIR");
        jp::applyUiTheme(1);
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        ed->doc()->viewGuides.v = {240, 720};
        ed->doc()->viewGuides.h = {135};
        deck::GridView grid;
        grid.spacing = 18;
        grid.snapGrid = true;
        deck::setGridView(*ed->doc(), grid);
        ed->setView([](ViewOptions &v) { v.gridSpacing = 18; v.snapGrid = true; });
        w.canvas()->setGridlines(true);
        w.canvas()->setGuides(true);
        w.canvas()->viewport()->update();
        QTRY_VERIFY(w.isVisible());
        shotOf(&w, QStringLiteral("deck-grid-and-guides-slide"));
        withDialog([&] { w.act(QStringLiteral("view.gridGuides"))->trigger(); }, QStringLiteral("gridGuidesDialog"), [&](QDialog *d) {
            d->findChild<QCheckBox *>(QStringLiteral("showGrid"))->setChecked(true);
            shotOf(d, QStringLiteral("deck-grid-and-guides-dialog"));
        }, false);
    }

    // Another page takes the bar away.
    void theMiniToolbarGoesWithAnotherPage()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        auto rig = [&] {
            const auto *title = placeholderOfSlide(*ed->doc(), 0, 0);
        const QString boxId = title->id, storyId = title->storyId;
        QTextCursor(ed->doc()->storyDoc(storyId)).insertText(minitest::kText);
        return minitest::Rig<DeckWindow>(w, boxId, storyId, 0);
        }();
        minitest::goesWithAnotherPage(rig);
    }
    // Overtype in the slide's text: as for JeffPub (the Insert key, the status bar's mode, the paragraph's end, one undo step).
    void overtypeReplacesLettersAndUndoesAsOneStep()
    {
        Settings::get().setValue(QStringLiteral("edit/insertKey"), false);
        const auto restore = qScopeGuard([] { Settings::get().setValue(QStringLiteral("edit/insertKey"), false); });
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        const auto *title = placeholderOfSlide(*ed->doc(), 0, 0);
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("abcd\nxy"));
        ed->beginTextEdit(title->id);
        const QString boxId = title->id, story = title->storyId;   // (undo puts the items back anew)
        auto textOf = [&] { return ed->doc()->storyDoc(story)->toPlainText(); };
        auto put = [&](int at) {
            QTextCursor c = ed->cursor();
            c.setPosition(at);
            ed->setCursor(c);
        };
        auto indicator = [&] { return w.statusBar()->findChild<QLabel *>(QStringLiteral("overtypeMode")); };
        QVERIFY(indicator());
        // The Insert key does nothing until Options lets it, and the indicator is not shown.
        put(2);
        QTest::keyClick(w.canvas(), Qt::Key_Insert);
        QVERIFY(!ed->overtype());
        QVERIFY(indicator()->isHidden());
        withDialog([&] { w.act(QStringLiteral("file.options"))->trigger(); }, QStringLiteral("optionsDialog"), [](QDialog *d) {
            auto *box = d->findChild<QCheckBox *>(QStringLiteral("insertKeyOvertype"));
            QVERIFY(box);
            QVERIFY(!box->isChecked());
            box->setChecked(true);
        });
        QVERIFY(Settings::get().value(QStringLiteral("edit/insertKey"), false).toBool());
        QVERIFY(!indicator()->isHidden());
        QCOMPARE(indicator()->text(), QStringLiteral("Insert"));
        // Insert turns overtype on: each letter takes the place of the one after the caret.
        QTest::keyClick(w.canvas(), Qt::Key_Insert);
        QVERIFY(ed->overtype());
        QCOMPARE(indicator()->text(), QStringLiteral("Overtype"));
        QTest::keyClicks(w.canvas(), QStringLiteral("12"));
        QCOMPARE(textOf(), QStringLiteral("ab12\nxy"));
        // At the end of the paragraph the letters are put in, and the next paragraph keeps its own.
        QTest::keyClicks(w.canvas(), QStringLiteral("3"));
        QCOMPARE(textOf(), QStringLiteral("ab123\nxy"));
        // One undo step for the run typed.
        ed->undo();
        QCOMPARE(textOf(), QStringLiteral("abcd\nxy"));
        ed->beginTextEdit(boxId);   // (undo leaves the text)
        // Insert again goes back to pushing the letters along.
        put(0);
        QTest::keyClick(w.canvas(), Qt::Key_Insert);
        QVERIFY(!ed->overtype());
        QTest::keyClicks(w.canvas(), QStringLiteral("Z"));
        QCOMPARE(textOf(), QStringLiteral("Zabcd\nxy"));
        // Turning the choice off in Options puts the mode back to Insert.
        QTest::keyClick(w.canvas(), Qt::Key_Insert);
        QVERIFY(ed->overtype());
        withDialog([&] { w.act(QStringLiteral("file.options"))->trigger(); }, QStringLiteral("optionsDialog"), [](QDialog *d) {
            d->findChild<QCheckBox *>(QStringLiteral("insertKeyOvertype"))->setChecked(false);
        });
        QVERIFY(!ed->overtype());
        QVERIFY(indicator()->isHidden());
        ed->endTextEdit();
    }

    // Overtype takes whole characters and leaves objects, an input method's text goes over the letters too, and the mode in the status bar stays when a message comes.
    void overtypeLeavesObjectsAndTakesWholeCharacters()
    {
        Settings::get().setValue(QStringLiteral("edit/insertKey"), false);
        const auto restore = qScopeGuard([] { Settings::get().setValue(QStringLiteral("edit/insertKey"), false); });
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        const auto *title = placeholderOfSlide(*ed->doc(), 0, 0);
        ed->beginTextEdit(title->id);
        const QString story = title->storyId;
        auto textOf = [&] { return ed->doc()->storyDoc(story)->toPlainText(); };
        auto at = [&](int pos) {
            QTextCursor c = ed->cursor();
            c.setPosition(pos);
            ed->setCursor(c);
        };
        auto fill = [&](const QString &words) {
            QTextCursor c = ed->cursor();
            c.select(QTextCursor::Document);
            c.insertText(words);
            ed->setCursor(c);
            at(0);
        };
        Settings::get().setValue(QStringLiteral("edit/insertKey"), true);
        ed->syncOvertypeOption();
        ed->setOvertype(true);
        // A letter typed takes a whole character: a pair of UTF-16 units, or a letter with its accent.
        fill(QString::fromUtf8("\xF0\x9F\x98\x80" "b"));
        QTest::keyClicks(w.canvas(), QStringLiteral("x"));
        QCOMPARE(textOf(), QStringLiteral("xb"));
        fill(QStringLiteral("e\u0301b"));
        QTest::keyClicks(w.canvas(), QStringLiteral("x"));
        QCOMPARE(textOf(), QStringLiteral("xb"));
        // One typed character that is a pair of units takes one character, not two.
        fill(QStringLiteral("abc"));
        ed->commitText(QString::fromUtf8("\xF0\x9F\x98\x80"));
        QCOMPARE(textOf(), QString::fromUtf8("\xF0\x9F\x98\x80" "bc"));
        // What an input method commits goes over the letters too.
        fill(QStringLiteral("abcd"));
        QInputMethodEvent commit;
        commit.setCommitString(QStringLiteral("XY"));
        QVERIFY(QCoreApplication::sendEvent(w.canvas(), &commit));
        QCOMPARE(textOf(), QStringLiteral("XYcd"));
        // The mode shown in the status bar stays shown when a message comes and goes.
        QLabel *mode = w.statusBar()->findChild<QLabel *>(QStringLiteral("overtypeMode"));
        QVERIFY(mode && mode->isVisible());
        w.statusBar()->showMessage(QStringLiteral("A message"));
        QVERIFY(mode->isVisible());
        w.statusBar()->clearMessage();
        QVERIFY(mode->isVisible());
        Settings::get().setValue(QStringLiteral("edit/insertKey"), false);
        ed->syncOvertypeOption();
        QVERIFY(!ed->overtype());
        w.statusBar()->showMessage(QStringLiteral("A message"));
        QVERIFY(!mode->isVisible());
        w.statusBar()->clearMessage();
        QVERIFY(!mode->isVisible());
    }

    // A picture of the status bar in overtype mode, for the shot folder (JO_SHOT_DIR); skipped otherwise.
    void overtypeStatusBarShotDeck()
    {
        if (qEnvironmentVariable("JO_SHOT_DIR").isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        Settings::get().setValue(QStringLiteral("edit/insertKey"), true);
        const auto restore = qScopeGuard([] { Settings::get().setValue(QStringLiteral("edit/insertKey"), false); });
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        const auto *title = placeholderOfSlide(*w.editor()->doc(), 0, 0);
        w.editor()->beginTextEdit(title->id);
        w.editor()->syncOvertypeOption();
        QTest::keyClick(w.canvas(), Qt::Key_Insert);
        QVERIFY(w.editor()->overtype());
        QTRY_VERIFY(w.statusBar()->findChild<QLabel *>(QStringLiteral("overtypeMode"))->isVisible());
        shotOf(w.statusBar(), QStringLiteral("overtype-deck-statusbar"));
    }

};

JP_TEST_CLASS(DeckWindowTests)
#include "tst_deck_window.moc"
