// JeffPub's tests of the window: ribbon, help, keyboard, accessibility, themes, and translations.
#include "pubhelpers.h"
#include "canvas/canvasaccessible.h"

class PubWindowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // The ribbon as built, one control per line. JP_RIBBON_DUMP=file writes
    // it, to compare a rebuilt ribbon with the one before; with
    // JP_RIBBON_DUMP_KEYTIPS=file it carries the KeyTips as well.
    void ribbonDescription()
    {
        jp::MainWindow w;
        auto *r = w.findChild<jp::Ribbon *>();
        QVERIFY(r);
        // ribbon.json is built into the program; every command, widget and menu it names must exist.
        QVERIFY2(w.ribbonError().isEmpty(), qPrintable(w.ribbonError()));
        const QString d = r->describe();
        QVERIFY2(!d.contains(QStringLiteral("(none)")), "a ribbon button has no command");
        for (const char *tab : {"Home", "Insert", "Page Design", "Mailings", "Review", "View", "Text Box", "Table Layout"})
            QVERIFY2(d.contains(QStringLiteral("\ntab %1").arg(QLatin1String(tab))), tab);
        QVERIFY(d.contains(QStringLiteral("QToolButton edit.paste")));
        // Commands that only open a menu carry ids like every other command.
        QVERIFY(d.contains(QStringLiteral("QToolButton ribbon.changeCase")));
        // The only buttons without a command are the Table button, Line Weight and the bare drop-down arrows.
        for (const QString &line : d.split(QLatin1Char('\n')))
            if (line.trimmed().startsWith(QStringLiteral("QToolButton \"")))
                QVERIFY2(line.contains(QStringLiteral("\"Table\"")) || line.contains(QStringLiteral("\"Line Weight\"")) || line.contains(QStringLiteral("QToolButton \"\"")), qPrintable(line));
        const QString withKeytips = r->describe(true);
        QVERIFY(withKeytips.contains(QStringLiteral("tab Home keytip=H")));
        QVERIFY(withKeytips.contains(QStringLiteral("QToolButton edit.paste style=under icon=30 popup=split")));
        QVERIFY(withKeytips.contains(QStringLiteral(" keytip=V\n")));
        if (const QByteArray out = qgetenv("JP_RIBBON_DUMP"); !out.isEmpty()) {
            QFile f(QString::fromLocal8Bit(out));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(d.toUtf8());
        }
        if (const QByteArray out = qgetenv("JP_RIBBON_DUMP_KEYTIPS"); !out.isEmpty()) {
            QFile f(QString::fromLocal8Bit(out));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(withKeytips.toUtf8());
        }
    }

    // Every KeyTip on the ribbon is one or two of A-Z and 0-9; the keys that
    // can be typed first (File, Quick Access, the tabs) are unique, and in each
    // tab no KeyTip is another's beginning, so typing one never leaves doubt.
    void ribbonKeytips()
    {
        jp::MainWindow w;
        auto *r = w.findChild<jp::Ribbon *>();
        QVERIFY(r);
        QVERIFY2(w.ribbonError().isEmpty(), qPrintable(w.ribbonError()));
        static const QRegularExpression valid(QStringLiteral("^[A-Z0-9]{1,2}$"));
        auto check = [&](const QStringList &keys, const QString &where) {
            for (int i = 0; i < keys.size(); ++i) {
                QVERIFY2(valid.match(keys[i]).hasMatch(), qPrintable(QStringLiteral("%1: \"%2\" isn't a KeyTip").arg(where, keys[i])));
                for (int j = 0; j < keys.size(); ++j)
                    if (i != j) QVERIFY2(!keys[j].startsWith(keys[i]), qPrintable(QStringLiteral("%1: %2 and %3 can't both be KeyTips").arg(where, keys[i], keys[j])));
            }
        };

        QStringList top{r->fileKeytip()};
        QCOMPARE(r->fileKeytip(), QStringLiteral("F"));
        for (const QToolButton *b : r->quickAccessButtons()) top << jp::keytip(b);
        QCOMPARE(top.mid(1), (QStringList{"1", "2", "3", "4"}));
        // The tabs always there have one letter; the contextual ones are J and another.
        const QStringList contextual{"Master Page", "Text Box", "Shape Format", "Picture Format", "Table Design", "Table Layout", "Text Art Format"};
        for (int i = 0; i < r->tabCount(); ++i) {
            top << r->tabKeytip(i);
            if (contextual.contains(r->tabName(i))) QVERIFY2(r->tabKeytip(i).size() == 2 && r->tabKeytip(i)[0] == QLatin1Char('J'), qPrintable(r->tabName(i)));
            else QVERIFY2(r->tabKeytip(i).size() == 1, qPrintable(r->tabName(i)));
        }
        check(top, QStringLiteral("File, Quick Access and tabs"));
        QCOMPARE(r->tabCount(), 7 + contextual.size());   // Home, Insert, Page Design, Mailings, Review, View, Help
        QCOMPARE(r->tabKeytip(0), QStringLiteral("H"));
        QCOMPARE(r->tabKeytip(1), QStringLiteral("N"));

        int controls = 0;
        for (int i = 0; i < r->tabCount(); ++i) {
            QStringList keys;
            const QString where = r->tabName(i);
            const QString need = QStringLiteral("%1: %2 has no KeyTip");
            for (jp::RibbonGroup *g : r->tabAt(i)->groups()) {
                QVERIFY2(!jp::keytip(g).isEmpty(), qPrintable(need.arg(where, g->title())));
                QVERIFY2(jp::keytip(g).startsWith(QLatin1Char('Z')), qPrintable(where + ": a collapsed group's KeyTip is Z and a letter"));
                keys << jp::keytip(g);
                if (g->launcher()) {
                    QVERIFY2(!jp::keytip(g->launcher()).isEmpty(), qPrintable(need.arg(where, g->title() + " launcher")));
                    keys << jp::keytip(g->launcher());
                }
                for (QWidget *c : g->controls()) {
                    if (qobject_cast<QLabel *>(c)) continue;   // plain text
                    ++controls;
                    QVERIFY2(!jp::keytip(c).isEmpty(), qPrintable(need.arg(where, QString::fromLatin1(c->metaObject()->className()) + " in " + g->title())));
                    keys << jp::keytip(c);
                }
            }
            check(keys, where);
            // Only a group's own KeyTip may start with Z, so a control's never hides one.
            for (const QString &k : keys)
                if (k.startsWith(QLatin1Char('Z'))) QVERIFY2(k.size() == 2, qPrintable(where + ": " + k));
        }
        QVERIFY2(controls > 250, qPrintable(QString::number(controls)));
    }

    // jp::buildRibbon reports every unknown name at once, with where it was used.
    void ribbonBuilderReportsProblems()
    {
        jp::Ribbon r;
        QAction known(QStringLiteral("Known"));
        jp::RibbonParts parts;
        parts.action = [&](const QString &id) { return id == QLatin1String("known") ? &known : nullptr; };
        const QByteArray json = R"({"tabs": [{"name": "T", "keytip": "T", "groups": [
            {"name": "G", "keytip": "ZG", "launcher": "noLauncher", "items": [
              {"large": "missing.one"}, {"widget": "noWidget"}, {"large": "known", "menu": "@noMenu"},
              {"small": "known", "typo": 1}, {"row": [{"icon": "missing.two"}]},
              {"large": {"id": "ribbon.x", "text": "X", "icon": "no-such-icon"}}, {"large": "known", "keytip": "abc"}]},
            {"use": "noTemplate"}]},
            {"name": "C", "context": "Ctx", "color": "noColor", "keytip": "JC", "groups": []}]})";
        QString err;
        QVERIFY(!jp::buildRibbon(&r, json, parts, &r, &err));
        for (const char *needle : {"missing.one", "missing.two", "noWidget", "noMenu", "noLauncher", "noTemplate", "typo", "no-such-icon", "abc", "noColor", "(in T / G)"})
            QVERIFY2(err.contains(QLatin1String(needle)), qPrintable(QStringLiteral("%1 not reported in:\n%2").arg(QLatin1String(needle), err)));
        QVERIFY(!jp::buildRibbon(&r, "{not json", parts, &r, &err));
        QVERIFY(err.contains(QStringLiteral("JSON")));
    }

    // A submenu may be a menu built in code ("@name"): it is added with the title and icon the ribbon gives it; a name that is not a menu is reported.
    void ribbonSubmenuFromCode()
    {
        QAction known(QStringLiteral("Known"));
        known.setObjectName(QStringLiteral("known"));
        jp::RibbonParts parts;
        parts.action = [&](const QString &id) { return id == QLatin1String("known") ? &known : nullptr; };
        parts.menus[QStringLiteral("dyn")] = [&](QWidget *owner) {
            auto *m = new QMenu(owner);
            m->addAction(&known);
            return m;
        };
        auto json = [](const char *menu) {
            return QStringLiteral(R"({"tabs": [{"name": "T", "keytip": "T", "groups": [{"name": "G", "keytip": "ZG", "items": [
                {"large": {"id": "ribbon.m", "text": "M", "icon": "table"}, "keytip": "M", "menu": ["known", {"submenu": "Code", "icon": "square", "menu": "%1"}]}]}]}]})")
                .arg(QLatin1String(menu))
                .toUtf8();
        };
        jp::Ribbon r;
        QString err;
        QVERIFY2(jp::buildRibbon(&r, json("@dyn"), parts, &r, &err), qPrintable(err));
        auto *big = qobject_cast<QToolButton *>(r.tabAt(0)->groups()[0]->controls()[0]);
        QVERIFY(big && big->menu());
        const QList<QAction *> items = big->menu()->actions();
        QCOMPARE(items.size(), 2);
        QVERIFY(items[1]->menu());
        QCOMPARE(items[1]->text(), QStringLiteral("Code"));
        QVERIFY(!items[1]->icon().isNull());
        QCOMPARE(items[1]->menu()->actions(), QList<QAction *>{&known});
        jp::Ribbon bad;
        QVERIFY(!jp::buildRibbon(&bad, json("@nowhere"), parts, &bad, &err));
        QVERIFY2(err.contains(QStringLiteral("nowhere")), qPrintable(err));
    }

    // The loader's pieces together: a template used twice, a command that only
    // opens a menu (made once, shared by its uses, doing what its trigger says),
    // sections and submenus, a launcher, KeyTips, and translation.
    void ribbonBuilderBuilds()
    {
        struct Upper : QTranslator {
            bool isEmpty() const override { return false; }
            QString translate(const char *context, const char *source, const char *, int) const override
            {
                return QLatin1String(context) == QLatin1String("Ribbon") ? QString::fromUtf8(source).toUpper() : QString();
            }
        };
        QAction known(QStringLiteral("Known"));
        known.setObjectName(QStringLiteral("known"));
        int launched = 0, triggered = 0, boxes = 0;
        jp::RibbonParts parts;
        parts.action = [&](const QString &id) { return id == QLatin1String("known") ? &known : nullptr; };
        parts.widgets[QStringLiteral("box")] = [&]() -> QWidget * { ++boxes; return new QComboBox(); };
        parts.menus[QStringLiteral("dyn")] = [&](QWidget *owner) { auto *m = new QMenu(owner); m->addAction(&known); return m; };
        parts.launchers[QStringLiteral("dlg")] = [&] { ++launched; };
        parts.triggers[QStringLiteral("ribbon.menuOnly")] = [&] { ++triggered; };
        parts.colors[QStringLiteral("blue")] = QColor(Qt::blue);
        const QByteArray json = R"({
          "fileKeytip": "F",
          "quickAccess": [{"cmd": "known", "keytip": "1"}],
          "templates": {"Shared": {"name": "Shared", "keytip": "ZS", "launcher": "dlg", "launcherTip": "Shared Settings", "launcherKeytip": "SS", "items": [
            {"large": {"id": "ribbon.menuOnly", "text": "Menu Only", "icon": "table"}, "keytip": "MO", "menu": [
              "known", "-", {"section": "Heading"}, {"submenu": "Sub", "icon": "square", "menu": ["known"]}]},
            {"row": [{"label": "Label"}, {"widget": "box", "keytip": "BX"}, {"dropdown": {"icon": "chevron-down", "tip": "More"}, "menu": "@dyn", "keytip": "MR"}]}]}},
          "tabs": [
            {"name": "One", "keytip": "O", "groups": [{"use": "Shared"},
              {"name": "Own", "keytip": "ZO", "items": [{"small": "ribbon.menuOnly", "keytip": "OM"}, {"small": "known", "split": false, "keytip": "KN"}]}]},
            {"name": "Two", "keytip": "JT", "context": "Ctx", "color": "blue", "groups": [{"use": "Shared", "keytip": "ZX"}]}]})";
        Upper upper;
        QCoreApplication::installTranslator(&upper);
        jp::Ribbon r;
        QString err;
        const bool ok = jp::buildRibbon(&r, json, parts, &r, &err);
        QCoreApplication::removeTranslator(&upper);
        QVERIFY2(ok, qPrintable(err));

        QCOMPARE(r.tabCount(), 2);
        QCOMPARE(r.tabName(0), QStringLiteral("One"));   // how code finds the tab doesn't change
        QCOMPARE(r.tabTitle(0), QStringLiteral("ONE"));   // what the header shows does
        QCOMPARE(r.tabKeytip(1), QStringLiteral("JT"));
        QCOMPARE(r.fileKeytip(), QStringLiteral("F"));
        QCOMPARE(jp::keytip(r.quickAccessButtons().value(0)), QStringLiteral("1"));
        QCOMPARE(boxes, 2);   // the template's widget, once per use

        const auto groups = r.tabAt(0)->groups();
        QCOMPARE(groups.size(), 2);
        QCOMPARE(groups[0]->title(), QStringLiteral("SHARED"));
        QCOMPARE(jp::keytip(groups[0]), QStringLiteral("ZS"));
        QCOMPARE(jp::keytip(r.tabAt(1)->groups().value(0)), QStringLiteral("ZX"));   // a use may rekey its template
        QVERIFY(groups[0]->launcher());
        QCOMPARE(groups[0]->launcher()->toolTip(), QStringLiteral("SHARED SETTINGS"));
        QCOMPARE(jp::keytip(groups[0]->launcher()), QStringLiteral("SS"));
        groups[0]->launcher()->click();
        QCOMPARE(launched, 1);

        const auto controls = groups[0]->controls();   // the big button, the label, the box, the dropdown
        QCOMPARE(controls.size(), 4);
        auto *big = qobject_cast<QToolButton *>(controls[0]);
        QVERIFY(big);
        QCOMPARE(jp::keytip(big), QStringLiteral("MO"));
        QVERIFY(big->defaultAction());
        QCOMPARE(big->defaultAction()->objectName(), QStringLiteral("ribbon.menuOnly"));
        QCOMPARE(big->defaultAction()->text(), QStringLiteral("MENU ONLY"));
        QVERIFY(!big->defaultAction()->icon().isNull());
        QCOMPARE(big->popupMode(), QToolButton::InstantPopup);
        QVERIFY(big->menu());
        const QList<QAction *> items = big->menu()->actions();
        QCOMPARE(items.size(), 4);
        QCOMPARE(items[0], &known);
        QVERIFY(items[1]->isSeparator());
        QVERIFY(items[2]->isSeparator());   // a section is a separator with a heading
        QCOMPARE(items[2]->text(), QStringLiteral("HEADING"));
        QVERIFY(items[3]->menu());
        QCOMPARE(items[3]->text(), QStringLiteral("SUB"));
        QCOMPARE(items[3]->menu()->actions(), QList<QAction *>{&known});
        QCOMPARE(qobject_cast<QLabel *>(controls[1])->text(), QStringLiteral("LABEL"));
        QCOMPARE(jp::keytip(controls[2]), QStringLiteral("BX"));
        QVERIFY(jp::keytip(controls[1]).isEmpty());
        auto *more = qobject_cast<QToolButton *>(controls[3]);
        QVERIFY(more && more->menu());
        QCOMPARE(more->menu()->actions(), QList<QAction *>{&known});
        QCOMPARE(more->toolTip(), QStringLiteral("MORE"));

        // The command made for the first use is the one every later use shares.
        const auto own = groups[1]->controls();
        QCOMPARE(own.size(), 2);
        QCOMPARE(qobject_cast<QToolButton *>(own[0])->defaultAction(), big->defaultAction());
        QCOMPARE(qobject_cast<QToolButton *>(own[1])->defaultAction(), &known);
        QCOMPARE(qobject_cast<QToolButton *>(r.tabAt(1)->groups()[0]->controls()[0])->defaultAction(), big->defaultAction());
        big->defaultAction()->trigger();
        QCOMPARE(triggered, 1);

        // The contextual tab, and the plain description that tests compare.
        const QString d = r.describe(true);
        QVERIFY2(d.contains(QStringLiteral("tab Two (Ctx, #0000ff) keytip=JT")), qPrintable(d));
        QVERIFY2(d.contains(QStringLiteral("group SHARED launcher=\"SHARED SETTINGS\" keytip=ZS launcher-keytip=SS")), qPrintable(d));
        QVERIFY(!r.describe().contains(QStringLiteral("keytip")));
    }

    // Setting a group's launcher again cut every connection from its button,
    // including the one that tells Qt's screen reader support the button is
    // gone: its stale entry was later handed to a new control at the same
    // address, and asking its name crashed (only where a screen reader or
    // other accessibility client is active, as on the Mac and Windows
    // runners).
    void launcherKeepsAccessibilityInTouch()
    {
        auto *g = new jp::RibbonGroup(QStringLiteral("Font"));
        g->setLauncher([] {}, QStringLiteral("Font"));
        QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(g->launcher());
        QVERIFY(iface);
        const QAccessible::Id id = QAccessible::uniqueId(iface);
        g->setLauncher([] {}, QStringLiteral("Font Settings"));
        delete g;
        QVERIFY(!QAccessible::accessibleInterface(id));   // gone with its button
    }

    // Every ribbon control has a name screen readers read, and the keyboard
    // reaches it (the ribbon's buttons, galleries and color buttons refused
    // the focus; its tabs and File button were only drawn).
    void ribbonControlsAreNamedAndReachable()
    {
        jp::MainWindow w;
        auto *r = w.findChild<jp::Ribbon *>();
        QVERIFY(r);
        QStringList unnamed, unreachable, undescribed;
        int checked = 0;
        for (QWidget *c : r->findChildren<QWidget *>()) {
            QWidget *pw = c->parentWidget();
            // A box one types in hands the focus to its typing field, which
            // Windows' screen readers name on its own.
            const bool typingField = c->inherits("QLineEdit") && pw && pw->inherits("QComboBox");
            const bool control = qobject_cast<QAbstractButton *>(c) || qobject_cast<QComboBox *>(c) || qobject_cast<QAbstractSpinBox *>(c)
                                 || qobject_cast<QAbstractItemView *>(c) || qobject_cast<QSlider *>(c) || typingField;
            if (!control || !pw || qobject_cast<QAbstractItemView *>(pw) || pw->inherits("QComboBoxPrivateContainer")) continue;   // a combo's own list
            ++checked;
            QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(c);
            QVERIFY(iface);
            const QString name = iface->text(QAccessible::Name).trimmed();
            const auto *group = qobject_cast<jp::RibbonGroup *>(c->parentWidget());
            const QString where = QStringLiteral("%1 \"%2\" in %3").arg(QString::fromLatin1(c->metaObject()->className()), c->toolTip(),
                                                                          group ? group->title() : c->parentWidget() ? QString::fromLatin1(c->parentWidget()->metaObject()->className()) : QString());
            // Boxes, spin boxes and lists need a name of their own: some
            // systems read their current text instead, others nothing.
            if (name.isEmpty() || (!qobject_cast<QAbstractButton *>(c) && c->accessibleName().isEmpty())) unnamed << where;
            if (typingField) {
                // What the reader says once the box's letters put the keyboard there.
                if (iface->text(QAccessible::Help).trimmed().isEmpty()) undescribed << where + QStringLiteral(" [typing field]");
                continue;
            }
            if (!(c->focusPolicy() & Qt::TabFocus) && c->objectName() != QLatin1String("jpRibbonTab")) unreachable << where + QStringLiteral(" [") + name + QLatin1Char(']');
            // Windows' screen readers read Qt's help text; some read nothing
            // else of a control's description.
            if (iface->text(QAccessible::Help).trimmed().isEmpty()) undescribed << where + QStringLiteral(" [") + name + QLatin1Char(']');
        }
        QVERIFY2(checked > 300, qPrintable(QString::number(checked)));
        QVERIFY2(unnamed.isEmpty(), qPrintable(QStringLiteral("no name: ") + unnamed.join(QStringLiteral("; "))));
        QVERIFY2(unreachable.isEmpty(), qPrintable(QStringLiteral("no keyboard focus: ") + unreachable.join(QStringLiteral("; "))));
        QVERIFY2(undescribed.isEmpty(), qPrintable(QStringLiteral("no description: ") + undescribed.join(QStringLiteral("; "))));
    }

    // Outside the ribbon too: everything the keyboard stops on has a name,
    // and nothing that only holds controls takes a stop (Windows' screen
    // readers found the page, its thumbnails, the zoom slider, and the
    // font boxes' typing fields unnamed, and the ribbon's scroller empty).
    void windowControlsAreNamed()
    {
        jp::MainWindow w;
        QStringList unnamed;
        int checked = 0;
        for (QWidget *c : w.findChildren<QWidget *>()) {
            if (!(c->focusPolicy() & Qt::TabFocus) || c->focusProxy()) continue;
            bool inBackstage = false;
            for (QWidget *a = c; a; a = a->parentWidget()) inBackstage |= a->inherits("jp::Backstage") || a->inherits("QMenu");
            if (inBackstage) continue;
            QWidget *pw = c->parentWidget();
            if (pw && (qobject_cast<QAbstractItemView *>(pw) || pw->inherits("QComboBoxPrivateContainer"))) continue;   // a list's own parts
            ++checked;
            QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(c);
            QVERIFY(iface);
            if (iface->text(QAccessible::Name).trimmed().isEmpty())
                unnamed << QStringLiteral("%1 %2 in %3").arg(QString::fromLatin1(c->metaObject()->className()), c->objectName(),
                                                            pw ? QString::fromLatin1(pw->metaObject()->className()) : QString());
        }
        QVERIFY2(checked > 300, qPrintable(QString::number(checked)));
        QVERIFY2(unnamed.isEmpty(), qPrintable(QStringLiteral("no name: ") + unnamed.join(QStringLiteral("; "))));
        // The page list's entries say which page.
        auto *pages = w.findChild<jp::PagesPane *>();
        QVERIFY(pages && pages->count() > 0);
        QCOMPARE(pages->item(0)->data(Qt::AccessibleTextRole).toString(), QStringLiteral("Page 1"));
    }

    // No two commands share keys (Ctrl+M was both Increase Indent and
    // Master Page, so neither worked), and the keys the other program gives
    // these commands do the same here: Ctrl+M the master page, Ctrl+Shift+C
    // and Ctrl+Shift+V copy and paste formatting.
    void shortcutsAreOneEach()
    {
        jp::MainWindow w;
        QHash<QString, QStringList> byKeys;
        for (const QString &id : w.actionIds())
            for (const QKeySequence &k : w.act(id)->shortcuts()) byKeys[k.toString(QKeySequence::PortableText)] << id;
        QStringList shared;
        for (auto it = byKeys.begin(); it != byKeys.end(); ++it)
            if (it.value().size() > 1) shared << it.key() + QStringLiteral(": ") + it.value().join(QStringLiteral(", "));
        QVERIFY2(shared.isEmpty(), qPrintable(shared.join(QStringLiteral("; "))));
        QCOMPARE(byKeys.value(QStringLiteral("Ctrl+M")), QStringList{QStringLiteral("view.master")});
        QCOMPARE(byKeys.value(QStringLiteral("Ctrl+Shift+C")), QStringList{QStringLiteral("edit.copyFormat")});
        QCOMPARE(byKeys.value(QStringLiteral("Ctrl+Shift+V")), QStringList{QStringLiteral("edit.pasteFormat")});
        // Copy one box's formatting, paste it on another: its fill and its text's.
        jp::Editor *ed = w.editor();
        auto a = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 200, 60), QStringLiteral("Bold one")));
        auto b = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 200, 200, 60), QStringLiteral("Plain one")));
        ed->addItem(a);
        ed->addItem(b);
        ed->select(a->id);
        ed->toggleBold();
        ed->forEachSelected(QStringLiteral("Fill"), [](jp::Item *it) { it->fill = jp::Fill::solid(jp::ColorRef::fromString(QStringLiteral("#ff0000"))); });
        w.act(QStringLiteral("edit.copyFormat"))->trigger();
        QVERIFY(ed->hasCopiedFormatting());
        QCOMPARE(ed->tool(), jp::Tool::Select);   // no painter pointer
        ed->select(b->id);
        w.act(QStringLiteral("edit.pasteFormat"))->trigger();
        QTextCursor c(ed->doc()->storyDoc(b->storyId));
        c.movePosition(QTextCursor::NextCharacter);
        QCOMPARE(c.charFormat().fontWeight(), int(QFont::Bold));
        QCOMPARE(b->fill.toJson(), a->fill.toJson());
        ed->undo();
        c = QTextCursor(ed->doc()->storyDoc(b->storyId));
        c.movePosition(QTextCursor::NextCharacter);
        QVERIFY(c.charFormat().fontWeight() != int(QFont::Bold));
    }

    // Help's topics are all built in, each has a title and search words,
    // every link and F1 context leads to a topic, the contents lists them
    // all, and none names another company's products.
    void helpTopicsAreComplete()
    {
        const QString src = QStringLiteral(JP_TEST_DATA "/../../resources/help");
        QStringList onDisk, built;
        for (QDirIterator it(src, QDir::Files, QDirIterator::Subdirectories); it.hasNext();) onDisk << QDir(src).relativeFilePath(it.next());
        for (QDirIterator it(QStringLiteral(":/help"), QDir::Files, QDirIterator::Subdirectories); it.hasNext();) built << it.next().mid(7);
        onDisk.sort();
        built.sort();
        QCOMPARE(built, onDisk);   // resources.qrc lists every file in resources/help
        const QStringList ids = jp::help::topicIds();
        QVERIFY2(ids.size() >= 45, qPrintable(QString::number(ids.size())));
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)]+)\\)"));
        static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office)\\b"));
        const jp::help::Topic contents = jp::help::topic(QStringLiteral("index"));
        QStringList problems;
        for (const QString &id : ids) {
            const jp::help::Topic t = jp::help::topic(id);
            if (t.title.isEmpty()) problems << id + QStringLiteral(": no title");
            if (t.keywords.isEmpty()) problems << id + QStringLiteral(": no keywords");
            if (const auto m = banned.match(t.markdown); m.hasMatch()) problems << id + QStringLiteral(": says ") + m.captured(1);
            for (auto m = link.globalMatch(t.markdown); m.hasNext();) {
                const QString target = m.next().captured(1);
                if (!target.startsWith(QLatin1String("https://")) && !ids.contains(target)) problems << id + QStringLiteral(": link to ") + target;
            }
            if (id != QLatin1String("index") && !contents.markdown.contains(QStringLiteral("](%1)").arg(id))) problems << id + QStringLiteral(": not in the contents");
            QTextDocument doc;
            doc.setMarkdown(jp::help::displayMarkdown(t, nullptr), QTextDocument::MarkdownDialectGitHub);
            if (doc.toPlainText().trimmed().size() < 100) problems << id + QStringLiteral(": nearly empty");
        }
        QFile cf(src + QStringLiteral("/context.json"));
        QVERIFY(cf.open(QIODevice::ReadOnly));
        const QJsonObject context = QJsonDocument::fromJson(cf.readAll()).object();
        for (auto it = context.begin(); it != context.end(); ++it)
            if (!ids.contains(it.value().toString())) problems << QStringLiteral("context %1: no topic %2").arg(it.key(), it.value().toString());
        jp::MainWindow w;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (jp::help::contextTopic(QStringLiteral("tab:") + w.ribbon()->tabName(i)).isEmpty()) problems << QStringLiteral("no topic for the tab ") + w.ribbon()->tabName(i);
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
        // The keyboard topic's table comes from the commands themselves.
        const QString keys = jp::help::displayMarkdown(jp::help::topic(QStringLiteral("keyboard")), &w);
        QVERIFY(!keys.contains(QLatin1String("<!--")));
        const QString save = QKeySequence(QKeySequence::Save).toString(QKeySequence::NativeText);
        QVERIFY2(keys.contains(QStringLiteral("| Save | %1 | Quick Access Toolbar |").arg(save)), qPrintable(keys.right(3000)));
        QVERIFY(keys.contains(QStringLiteral("| Collapse the Ribbon | %1 |").arg(QKeySequence(Qt::CTRL | Qt::Key_F1).toString(QKeySequence::NativeText))));
    }

    // F1 opens help on what is being done: the contents from the page, a
    // tab's topic from the ribbon, an object's topic when one is selected,
    // and a window of its own over the File page and over dialogs.
    void helpFollowsWhatYouAreDoing()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        w.canvas()->setFocus();
        QTest::keyClick(w.canvas(), Qt::Key_F1);
        jp::HelpView *v = w.helpView();
        QVERIFY(v && v->isVisible());
        QCOMPARE(w.currentTaskPane(), QStringLiteral("help"));
        QCOMPARE(v->currentTopic(), QStringLiteral("index"));
        QVERIFY(v->browser()->toPlainText().contains(QLatin1String("Getting started")));
        // From the ribbon: the tab's topic.
        jp::Ribbon *r = w.ribbon();
        r->showTab(r->tab(QStringLiteral("Insert")));
        r->focusCurrentTab();
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_F1);
        QCOMPARE(v->currentTopic(), QStringLiteral("tab-insert"));
        // A selected text box: text boxes.
        jp::Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(72, 72, 300, 100), QStringLiteral("Hello")));
        QVERIFY(ed->single());
        w.canvas()->setFocus();
        QTest::keyClick(w.canvas(), Qt::Key_F1);
        QCOMPARE(v->currentTopic(), QStringLiteral("text-boxes"));
        // The "?" beside the collapse chevron opens it too.
        w.hideTaskPane();
        auto *qmark = r->findChild<QAbstractButton *>(QStringLiteral("jpRibbonHelp"));
        QVERIFY(qmark && qmark->isVisible());
        QCOMPARE(qmark->accessibleName(), QStringLiteral("Help"));
        qmark->click();
        QCOMPARE(w.currentTaskPane(), QStringLiteral("help"));
        // Over the File page, whose print page covers the pane: a window.
        w.showBackstage(QStringLiteral("print"));
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_F1);
        auto *hw = w.findChild<jp::HelpWindow *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(hw && hw->isVisible());
        QCOMPARE(hw->view()->currentTopic(), QStringLiteral("printing"));
        hw->close();
        QTRY_VERIFY(!w.findChild<jp::HelpWindow *>());   // closing deletes it
        w.hideBackstage();
        // Over a dialog: a window of the dialog's own, on its topic. (Dialogs
        // are modal, as exec() makes them, which keeps the window's own F1 off.)
        QDialog dlg(&w);
        dlg.setWindowModality(Qt::ApplicationModal);
        dlg.setWindowTitle(QStringLiteral("Mail Merge"));
        auto *field = new QLineEdit(&dlg);
        dlg.show();
        QVERIFY(QTest::qWaitForWindowActive(&dlg));
        field->setFocus();
        QTest::keyClick(field, Qt::Key_F1);
        auto *over = dlg.findChild<jp::HelpWindow *>();
        QVERIFY(over);
        QCOMPARE(over->view()->currentTopic(), QStringLiteral("mail-merge"));
        // There it finds topics only: commands can't run under a dialog.
        over->view()->search(QStringLiteral("text box"));
        bool command = false;
        for (int i = 0; i < over->view()->results()->count(); ++i) command |= over->view()->results()->item(i)->text() == QLatin1String("Commands");
        QVERIFY(!command);
    }

    // The search box finds topics (ignoring "how do I") and commands, and
    // runs a command; links move between topics, Back and Forward return,
    // and web links go to the browser.
    void helpSearchFindsTopicsAndCommands()
    {
        jp::MainWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        w.showHelp();
        jp::HelpView *v = w.helpView();
        QVERIFY(v);
        auto texts = [v] {
            QStringList t;
            for (int i = 0; i < v->results()->count(); ++i) t << v->results()->item(i)->text();
            return t;
        };
        QCOMPARE(jp::help::searchTopics(QStringLiteral("how do I print")).value(0), QStringLiteral("printing"));
        QCOMPARE(jp::help::searchTopics(QStringLiteral("textbox")).value(0), QStringLiteral("text-boxes"));
        QVERIFY(jp::help::searchTopics(QStringLiteral("zzqxv")).isEmpty());
        v->search(QStringLiteral("text box"));
        QVERIFY(v->results()->isVisible() && !v->browser()->isVisible());
        const QStringList found = texts();
        QVERIFY2(found.contains(QStringLiteral("Help topics")) && found.contains(QStringLiteral("Commands")), qPrintable(found.join(QStringLiteral(" / "))));
        QListWidgetItem *draw = nullptr;
        for (int i = 0; i < v->results()->count(); ++i)
            if (v->results()->item(i)->text().startsWith(QLatin1String("Draw Text Box  (Home > "))) draw = v->results()->item(i);
        QVERIFY2(draw, qPrintable(found.join(QStringLiteral(" / "))));
        QSignalSpy ran(w.act(QStringLiteral("ins.textbox")), &QAction::triggered);
        QTest::keyClick(v->searchBox(), Qt::Key_Down);
        QCOMPARE(QApplication::focusWidget(), v->results());
        v->results()->setCurrentItem(draw);
        QTest::keyClick(v->results(), Qt::Key_Return);
        QCOMPARE(ran.count(), 1);
        v->search(QStringLiteral("zzqxv"));
        QCOMPARE(texts(), QStringList{QStringLiteral("Nothing found. Try other words, or open the contents.")});
        // Enter in the search box opens the first topic.
        v->search(QStringLiteral("mail merge"));
        QTest::keyClick(v->searchBox(), Qt::Key_Return);
        QCOMPARE(v->currentTopic(), QStringLiteral("mail-merge"));
        QVERIFY(v->browser()->isVisible() && v->searchBox()->text().isEmpty());
        // Links: a topic, Back, Forward, and a web page.
        Q_EMIT v->browser()->anchorClicked(QUrl(QStringLiteral("linked-text")));
        QCOMPARE(v->currentTopic(), QStringLiteral("linked-text"));
        auto button = [v](const QString &name) {
            for (auto *b : v->findChildren<QToolButton *>())
                if (b->accessibleName() == name) return b;
            return static_cast<QToolButton *>(nullptr);
        };
        QVERIFY(button(QStringLiteral("Back"))->isEnabled());
        button(QStringLiteral("Back"))->click();
        QCOMPARE(v->currentTopic(), QStringLiteral("mail-merge"));
        button(QStringLiteral("Forward"))->click();
        QCOMPARE(v->currentTopic(), QStringLiteral("linked-text"));
        button(QStringLiteral("Contents"))->click();
        QCOMPARE(v->currentTopic(), QStringLiteral("index"));
        QList<QUrl> opened;
        const auto keep = jp::help::openUrl;
        jp::help::openUrl = [&opened](const QUrl &u) { opened << u; return true; };
        Q_EMIT v->browser()->anchorClicked(QUrl(QStringLiteral("https://github.com/JeffOffice/jeffoffice")));
        QCOMPARE(v->currentTopic(), QStringLiteral("index"));
        // The Help tab's web pages: Feedback (a problem or an idea) carries the
        // version and system; What's New is this version's release. There is
        // no Contact Support button: Feedback covers it.
        QVERIFY(!w.act(QStringLiteral("help.support")));
        w.act(QStringLiteral("help.feedback"))->trigger();
        w.act(QStringLiteral("help.whatsNew"))->trigger();
        jp::help::openUrl = keep;
        QCOMPARE(opened.size(), 3);
        QCOMPARE(opened[1].toString(QUrl::RemoveQuery), QStringLiteral("https://github.com/JeffOffice/jeffoffice/issues/new"));
        QVERIFY(QUrlQuery(opened[1]).queryItemValue(QStringLiteral("body"), QUrl::FullyDecoded).contains(QStringLiteral("JeffPub " JP_VERSION " on ")));
        QCOMPARE(opened[2].toString(), QStringLiteral("https://github.com/JeffOffice/jeffoffice/releases/tag/v" JP_VERSION));
        // Ctrl+F1 collapses the ribbon and brings it back, as there.
        QVERIFY(!w.ribbon()->isMinimized());
        QTest::keyClick(&w, Qt::Key_F1, Qt::ControlModifier);
        QVERIFY(w.ribbon()->isMinimized());
        QTest::keyClick(&w, Qt::Key_F1, Qt::ControlModifier);
        QVERIFY(!w.ribbon()->isMinimized());
    }

    // Help's topics are published to the project's wiki (JeffPub
    // --export-help-wiki): a page per topic named from its title, the contents
    // as Home, a sidebar, and a footer. Every link between topics points at a
    // page that exists, and the pages carry what Help shows (the shortcut
    // table, no comments).
    void helpExportsWikiPages()
    {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QString dir = tmp.filePath(QStringLiteral("wiki"));
        jp::MainWindow w;
        QString error;
        QVERIFY2(jp::help::exportWiki(dir, &w, &error), qPrintable(error));
        QHash<QString, QString> pages;   // file name -> text
        for (const QString &name : QDir(dir).entryList(QDir::Files)) {
            QFile f(dir + QLatin1Char('/') + name);
            QVERIFY(f.open(QIODevice::ReadOnly));
            pages.insert(name, QString::fromUtf8(f.readAll()));
        }
        QCOMPARE(pages.size(), jp::help::topicIds().size() + 2);   // a page per topic (the contents is Home), the sidebar, and the footer
        for (const char *name : {"Home.md", "_Sidebar.md", "_Footer.md", "Get-help-and-report-problems.md", "Work-with-.pub-files.md",
                                 "Change-JeffPub's-settings.md", "Bullets,-numbering,-and-tabs.md", "Table-Design-and-Table-Layout-tabs.md"})
            QVERIFY2(pages.contains(QString::fromLatin1(name)), name);
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)]+)\\)"));
        QStringList problems;
        for (auto it = pages.constBegin(); it != pages.constEnd(); ++it) {
            if (it.value().contains(QLatin1String("<!--"))) problems << it.key() + QStringLiteral(": a comment is left");
            // The wiki shows a page's name as its title, so a heading stays only
            // where that would read differently ("Home"; "e-book" without its hyphen).
            if (it.value().startsWith(QLatin1String("# ")) && QString(it.key()).chopped(3).replace(QLatin1Char('-'), QLatin1Char(' ')) == it.value().section(QLatin1Char('\n'), 0, 0).mid(2))
                problems << it.key() + QStringLiteral(": repeats its title");
            if (it.value().trimmed().size() < 100) problems << it.key() + QStringLiteral(": nearly empty");
            if (it.value().contains(QLatin1String("\n\n\n"))) problems << it.key() + QStringLiteral(": blank lines left by comments");
            for (auto m = link.globalMatch(it.value()); m.hasNext();) {
                const QString target = m.next().captured(1);
                if (!target.startsWith(QLatin1String("https://")) && !pages.contains(target + QStringLiteral(".md"))) problems << it.key() + QStringLiteral(": link to ") + target;
            }
        }
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
        QVERIFY(pages["Get-help-and-report-problems.md"].contains(QStringLiteral("[Use JeffPub with the keyboard](Use-JeffPub-with-the-keyboard)")));
        // The keyboard page holds the shortcut table, made from the commands.
        const QString bold = QKeySequence(QKeySequence::Bold).toString(QKeySequence::NativeText);
        const QString keys = pages["Use-JeffPub-with-the-keyboard.md"];
        QVERIFY2(keys.contains(QStringLiteral("| Bold | %1 | Home > Font |").arg(bold)), qPrintable(keys.right(2000)));
        // The sidebar is the contents, linked; the footer says where to edit.
        QVERIFY(pages["_Sidebar.md"].startsWith(QStringLiteral("**[JeffPub Help](Home)**")));
        QVERIFY(pages["_Sidebar.md"].contains(QStringLiteral("- [Welcome to JeffPub](Welcome-to-JeffPub)")));
        QVERIFY(pages["_Sidebar.md"].contains(QStringLiteral("**Getting started**")));
        QVERIFY(pages["Home.md"].startsWith(QStringLiteral("# JeffPub Help\n")));
        QVERIFY(!pages["Home.md"].contains(QStringLiteral("search box above")));   // that was the Help pane's
        QVERIFY(pages["_Footer.md"].contains(QStringLiteral("resources/help/en")));
        QCOMPARE(pages["_Footer.md"].count(QLatin1Char('\n')), 1);
        // A folder with files in it is left alone: the pages replace a wiki's.
        QVERIFY(!jp::help::exportWiki(dir, &w, &error));
        QVERIFY2(error.contains(QLatin1String("isn't empty")), qPrintable(error));
    }

    // A page name comes from the title with its spaces as hyphens, and a
    // link to a topic points at that topic's page; web links stay, and a link
    // to nothing is an error.
    void helpWikiNamesAndLinks()
    {
        using jp::help::wikiPageName;
        QCOMPARE(wikiPageName(QStringLiteral("Get help and report problems")), QStringLiteral("Get-help-and-report-problems"));
        QCOMPARE(wikiPageName(QStringLiteral("  Bullets, numbering,   and tabs ")), QStringLiteral("Bullets,-numbering,-and-tabs"));
        QCOMPARE(wikiPageName(QStringLiteral("Save as: a \"picture\"? (or 100% web) #1 <b>/\\|*")), QStringLiteral("Save-as-a-picture-or-100-web-1-b"));
        QCOMPARE(wikiPageName(QStringLiteral(".Work with .pub files.")), QStringLiteral("Work-with-.pub-files"));
        QCOMPARE(wikiPageName(QStringLiteral("?")), QString());
        const QHash<QString, QString> pages{{QStringLiteral("start"), QStringLiteral("Welcome-to-JeffPub")}, {QStringLiteral("tab-table"), QStringLiteral("Table-tabs")}};
        QString md = QStringLiteral("See [the start](start), [the site](https://example.com/start), [mail](mailto:a@b.c), [here](#top), and [tables](./tab-table.md#x).");
        QVERIFY(jp::help::wikiLinks(md, pages));
        QCOMPARE(md, QStringLiteral("See [the start](Welcome-to-JeffPub), [the site](https://example.com/start), [mail](mailto:a@b.c), [here](#top), and [tables](Table-tabs#x)."));
        QString bad = QStringLiteral("A [link](nowhere) to nothing."), error;
        QVERIFY(!jp::help::wikiLinks(bad, pages, &error));
        QVERIFY2(error.contains(QLatin1String("nowhere")), qPrintable(error));
        QCOMPARE(bad, QStringLiteral("A [link](nowhere) to nothing."));
    }

    // Screen readers see the page's objects: each is a child of the page,
    // named by its kind and its text or alt text; the selected one is the
    // focus (so Tab is read out object by object); while typing, its text,
    // cursor, and selection can be read. (The page was one item, "Page".)
    void pageObjectsForScreenReaders()
    {
        jp::MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        jp::Editor *ed = w.editor();
        jp::Canvas *cv = w.canvas();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 300, 60), QStringLiteral("Spring sale")));
        ed->addItem(box);
        auto pic = std::make_shared<jp::PictureItem>();
        pic->rect = QRectF(72, 200, 144, 144);
        pic->altText = QStringLiteral("A red barn");
        ed->addItem(pic);
        auto bare = std::make_shared<jp::PictureItem>();
        bare->rect = QRectF(300, 200, 72, 72);
        ed->addItem(bare);
        auto decoration = std::make_shared<jp::PictureItem>();   // marked as only decoration: not asked for a description
        decoration->rect = QRectF(400, 200, 72, 72);
        decoration->decorative = true;
        ed->addItem(decoration);
        cv->setFocus();
        QAccessibleInterface *page = QAccessible::queryAccessibleInterface(cv);
        QVERIFY(page);
        QCOMPARE(page->text(QAccessible::Name), QStringLiteral("Page"));
        QStringList names;
        for (int i = 0; i < page->childCount(); ++i)
            if (QAccessibleInterface *c = page->child(i); c && c->object() == nullptr) names << c->text(QAccessible::Name);
        QCOMPARE(names, (QStringList{QStringLiteral("Text box: Spring sale"), QStringLiteral("Picture: A red barn"), QStringLiteral("Picture, no alt text"), QStringLiteral("Picture, decorative")}));
        // Tab selects each object, and the selected one is the focus.
        ed->clearSelection();
        QVERIFY(!page->focusChild());
        QTest::keyClick(cv, Qt::Key_Tab);
        QAccessibleInterface *f = page->focusChild();
        QVERIFY(f);
        QCOMPARE(f->text(QAccessible::Name), QStringLiteral("Text box: Spring sale"));
        QCOMPARE(f->role(), QAccessible::EditableText);
        QVERIFY(f->state().focused && f->state().selected);
        QVERIFY(f->text(QAccessible::Description).contains(QStringLiteral("from the left")));
        QCOMPARE(page->indexOfChild(f), 0);
        QVERIFY(f->rect().width() > 0 && cv->rect().translated(cv->mapToGlobal(QPoint(0, 0))).intersects(f->rect()));
        QTest::keyClick(cv, Qt::Key_Tab);
        QCOMPARE(page->focusChild()->text(QAccessible::Name), QStringLiteral("Picture: A red barn"));
        QCOMPARE(page->focusChild()->role(), QAccessible::Graphic);
        QVERIFY(!f->state().focused);
        // Typing: the text, the cursor, and the selection.
        ed->select(box->id);
        QTest::keyClick(cv, Qt::Key_Return);
        QVERIFY(ed->isEditingText());
        f = page->focusChild();
        QAccessibleTextInterface *t = f->textInterface();
        QVERIFY(t);
        QCOMPARE(t->text(0, t->characterCount()), QStringLiteral("Spring sale"));
        t->setCursorPosition(7);
        QCOMPARE(ed->cursor().position(), 7);
        QCOMPARE(t->cursorPosition(), 7);
        t->setSelection(0, 0, 6);
        QCOMPARE(ed->cursor().selectedText(), QStringLiteral("Spring"));
        int a = -1, b = -1;
        t->selection(0, &a, &b);
        QCOMPARE(a, 0);
        QCOMPARE(b, 6);
        QVERIFY(t->characterRect(3).isValid());
        QTest::keyClick(cv, Qt::Key_End);
        QTest::keyClicks(cv, QStringLiteral("!"));
        QCOMPARE(t->text(0, t->characterCount()), QStringLiteral("Spring sale!"));
        // An object deleted goes from the list.
        QTest::keyClick(cv, Qt::Key_Escape);
        ed->select(bare->id);
        ed->deleteSelection();
        names.clear();
        for (int i = 0; i < page->childCount(); ++i)
            if (QAccessibleInterface *c = page->child(i); c && c->object() == nullptr) names << c->text(QAccessible::Name);
        QCOMPARE(names.size(), 3);
    }

    // A screen reader walks the page child by child; the page's list of
    // objects is kept until the publication changes (each child() call made the
    // whole list and looked every object up in the document: 4.9 seconds for
    // 1,500 objects), and still follows additions, deletions, undo, and pages.
    void pageObjectListIsKeptBetweenChanges()
    {
        jp::MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        jp::Canvas *cv = w.canvas();
        QAccessibleInterface *page = QAccessible::queryAccessibleInterface(cv);
        QVERIFY(page);
        const int own = page->childCount();   // the page's own controls
        for (int i = 0; i < 1500; ++i) {
            auto pic = std::make_shared<jp::PictureItem>();
            pic->rect = QRectF(10 + i % 40, 10 + i / 40, 20, 20);
            pic->altText = QStringLiteral("Picture %1").arg(i);
            ed->surfaceItems().push_back(pic);
        }
        const qint64 listed = jp::CanvasAccessible::idsListedForTest();
        const int scans = ed->doc()->findScansForTest();
        for (int i = 0; i < page->childCount(); ++i) QVERIFY(page->child(i));
        // Counted: the list of the page's objects is made once for the whole walk (1,500 ids, not 1,500 times 1,500), and the document is not searched for any of them.
        QVERIFY2(jp::CanvasAccessible::idsListedForTest() - listed <= 2 * 1500 + 200, qPrintable(QString::number(jp::CanvasAccessible::idsListedForTest() - listed)));
        QVERIFY2(ed->doc()->findScansForTest() - scans <= 2, qPrintable(QString::number(ed->doc()->findScansForTest() - scans)));
        QCOMPARE(page->childCount(), own + 1500);
        QCOMPARE(page->child(1499)->text(QAccessible::Name), QStringLiteral("Picture: Picture 1499"));
        // The list follows the publication.
        auto extra = std::make_shared<jp::PictureItem>();
        extra->rect = QRectF(300, 300, 20, 20);
        extra->altText = QStringLiteral("Added");
        ed->addItem(extra);
        QCOMPARE(page->childCount(), own + 1501);
        QCOMPARE(page->child(1500)->text(QAccessible::Name), QStringLiteral("Picture: Added"));
        ed->undo();
        QCOMPARE(page->childCount(), own + 1500);
        ed->redo();
        QCOMPARE(page->childCount(), own + 1501);
        ed->select(extra->id);
        ed->deleteSelection();
        QCOMPARE(page->childCount(), own + 1500);
        ed->insertPages(1, 1, false, false);
        ed->setCurrentPage(1);
        QCOMPARE(page->childCount(), own);
        ed->setCurrentPage(0);
        QCOMPARE(page->childCount(), own + 1500);
    }

    // Every task pane's controls have names too (the panes are made when
    // first opened, so windowControlsAreNamed doesn't see them).
    void taskPaneControlsAreNamed()
    {
        jp::MainWindow w;
        QStringList unnamed;
        for (const char *pane : {"help", "online", "find", "designchecker", "mailmerge", "graphics", "research", "catalog"}) {
            w.showTaskPane(QString::fromLatin1(pane));
            auto *tp = w.findChild<jp::TaskPane *>();
            QVERIFY(tp);
            for (QWidget *c : tp->findChildren<QWidget *>()) {
                if (!(c->focusPolicy() & Qt::TabFocus) || c->focusProxy() || !c->isVisibleTo(tp)) continue;
                QWidget *pw = c->parentWidget();
                if (pw && (qobject_cast<QAbstractItemView *>(pw) || pw->inherits("QComboBoxPrivateContainer"))) continue;
                QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(c);
                // A box that chooses needs a name of its own: some systems
                // read its current choice instead, Windows nothing.
                const bool ownName = qobject_cast<QComboBox *>(c) && c->accessibleName().isEmpty();
                if (!iface || iface->text(QAccessible::Name).trimmed().isEmpty() || ownName)
                    unnamed << QStringLiteral("%1: %2 %3").arg(QLatin1String(pane), QString::fromLatin1(c->metaObject()->className()), c->objectName());
            }
        }
        QVERIFY2(unnamed.isEmpty(), qPrintable(unnamed.join(QStringLiteral("; "))));
    }

    // KeyTips: Alt shows letters on the top row, a tab's letter opens it and
    // shows its controls' letters, a control's letters use it; Escape steps
    // back; Alt held while typing goes straight there; a click ends it.
    void keyTipsUseTheRibbon()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        auto *r = w.findChild<jp::Ribbon *>();
        jp::KeyTips *kt = w.keyTips();
        QVERIFY(kt);
        auto keys = [&] {
            QStringList k;
            for (const auto &p : kt->shown()) k << p.first;
            return k;
        };
        QTest::keyPress(&w, Qt::Key_Alt);
        QTest::keyRelease(&w, Qt::Key_Alt);
        QCOMPARE(kt->level(), jp::KeyTips::Top);
        // The other program's letters (its 2021 version): File F, Home H,
        // Insert N, Page Design P, Mailings M, Review R, View W, Help Y,
        // and the Quick Access Toolbar's buttons numbered.
        QStringList top = keys();
        top.sort();
        QCOMPARE(top.join(QLatin1Char(' ')), QStringLiteral("1 2 3 4 F H M N P R W Y"));
        QTest::keyClick(&w, Qt::Key_H);
        QCOMPARE(kt->level(), jp::KeyTips::InTab);
        QCOMPARE(r->current(), r->tab(QStringLiteral("Home")));
        QVERIFY(keys().contains(QStringLiteral("V")));   // Paste
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) w.grab(QRect(0, 0, 1400, 170)).save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/keytips-home.png"));
        for (const auto &p : kt->shown()) QVERIFY(r->current()->isAncestorOf(p.second));
        // Two letters on another tab: View > Show > Rulers (W, then S R).
        QTest::keyClick(&w, Qt::Key_Escape);
        QTest::keyClick(&w, Qt::Key_W);
        QCOMPARE(r->current(), r->tab(QStringLiteral("View")));
        QSignalSpy rulers(w.act(QStringLiteral("view.rulers")), &QAction::triggered);
        QTest::keyClick(&w, Qt::Key_S);
        QCOMPARE(rulers.count(), 0);   // "S" starts several; they wait for the second letter
        QVERIFY(!keys().contains(QStringLiteral("NM")));
        QTest::keyClick(&w, Qt::Key_R);
        QCOMPARE(rulers.count(), 1);
        QCOMPARE(kt->level(), jp::KeyTips::Off);
        // Escape steps back a level at a time.
        QTest::keyPress(&w, Qt::Key_Alt);
        QTest::keyRelease(&w, Qt::Key_Alt);
        QTest::keyClick(&w, Qt::Key_N);
        QCOMPARE(r->current(), r->tab(QStringLiteral("Insert")));
        QTest::keyClick(&w, Qt::Key_Escape);
        QCOMPARE(kt->level(), jp::KeyTips::Top);
        QTest::keyClick(&w, Qt::Key_Escape);
        QCOMPARE(kt->level(), jp::KeyTips::Off);
        // Alt held while typing: straight to the tab.
        QTest::keyPress(&w, Qt::Key_Alt);
        QTest::keyClick(&w, Qt::Key_P, Qt::AltModifier);
        QTest::keyRelease(&w, Qt::Key_Alt);
        QCOMPARE(r->current(), r->tab(QStringLiteral("Page Design")));
        QCOMPARE(kt->level(), jp::KeyTips::InTab);
        // A click puts the letters away.
        QTest::mouseClick(w.findChild<jp::Canvas *>(), Qt::LeftButton);
        QCOMPARE(kt->level(), jp::KeyTips::Off);
        // A box's letters put the keyboard in it, ready to type (Home > Font:
        // F F); while it can't be used (no text selected), the letters stay
        // up and nothing moves.
        QWidget *before = QApplication::focusWidget();
        QTest::keyPress(&w, Qt::Key_Alt);
        QTest::keyRelease(&w, Qt::Key_Alt);
        QTest::keyClick(&w, Qt::Key_H);
        QTest::keyClick(&w, Qt::Key_F);
        QTest::keyClick(&w, Qt::Key_F);
        QCOMPARE(kt->level(), jp::KeyTips::InTab);
        QCOMPARE(QApplication::focusWidget(), before);
        QVERIFY(keys().contains(QStringLiteral("FF")));
        QTest::keyClick(&w, Qt::Key_Escape);
        QTest::keyClick(&w, Qt::Key_Escape);
        QCOMPARE(kt->level(), jp::KeyTips::Off);
        jp::Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(72, 72, 200, 100)));
        QTRY_VERIFY(w.findChildren<jp::FontCombo *>().constFirst()->isEnabled());
        QTest::keyPress(&w, Qt::Key_Alt);
        QTest::keyRelease(&w, Qt::Key_Alt);
        QTest::keyClick(&w, Qt::Key_H);
        QTest::keyClick(&w, Qt::Key_F);
        QTest::keyClick(&w, Qt::Key_F);
        QCOMPARE(kt->level(), jp::KeyTips::Off);
        QWidget *typing = QApplication::focusWidget();
        QVERIFY2(typing && (typing->inherits("jp::FontCombo") || (typing->parentWidget() && typing->parentWidget()->inherits("jp::FontCombo"))),
                 typing ? typing->metaObject()->className() : "nothing");
    }

    // Tab belongs to the page: while typing it types a tab, otherwise it
    // selects the next object (Shift+Tab the one before), and the keyboard
    // stays on the page; F6 is the way to the ribbon. Once the ribbon's
    // controls took the keyboard, Tab left the page for the File button.
    void tabStaysOnThePage()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        jp::Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(72, 72, 200, 100)));
        ed->addItem(ed->newTextBox(QRectF(72, 300, 200, 100)));
        const auto &items = ed->surfaceItems();
        QCOMPARE(int(items.size()), 2);
        const QString first = items[0]->id, second = items[1]->id;
        auto *canvas = w.findChild<jp::Canvas *>();
        canvas->setFocus();
        ed->select(QStringList{first});
        QTest::keyClick(canvas, Qt::Key_Tab);
        QCOMPARE(QApplication::focusWidget(), canvas);
        QCOMPARE(ed->selection(), QStringList{second});
        QTest::keyClick(canvas, Qt::Key_Backtab, Qt::ShiftModifier);
        QCOMPARE(ed->selection(), QStringList{first});
        ed->beginTextEdit(first);
        QTest::keyClicks(canvas, QStringLiteral("a"));
        QTest::keyClick(canvas, Qt::Key_Tab);
        QTest::keyClicks(canvas, QStringLiteral("b"));
        QCOMPARE(QApplication::focusWidget(), canvas);
        QVERIFY2(ed->editDoc() && ed->editDoc()->toPlainText().contains(QStringLiteral("a\tb")), qPrintable(ed->editDoc() ? ed->editDoc()->toPlainText() : QString()));
    }

    // Using the ribbon with the keyboard alone: F6 enters it on the current
    // tab, Right switches to the next tab, Down goes into its controls, the
    // arrow keys move among them, and Escape goes back to the page.
    void ribbonByKeyboard()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        auto *r = w.findChild<jp::Ribbon *>();
        auto *canvas = w.findChild<jp::Canvas *>();
        jp::installFocusRing(qApp);
        QTest::mouseClick(canvas->viewport(), Qt::LeftButton, {}, QPoint(5, 5));   // the mouse in use: no ring
        canvas->setFocus();
        QTest::keyClick(&w, Qt::Key_F6);
        QWidget *f = QApplication::focusWidget();
        QVERIFY(f && f->objectName() == QLatin1String("jpRibbonTab"));
        // The first key, though a shortcut, shows where the keyboard went.
        auto *ring = w.findChild<QWidget *>(QStringLiteral("jpFocusRing"));
        QVERIFY(ring && ring->isVisible());
        QVERIFY(ring->geometry().contains(QRect(f->mapTo(&w, QPoint(0, 0)), f->size())));
        QCOMPARE(f->accessibleName(), QStringLiteral("Home"));
        QTest::keyClick(f, Qt::Key_Right);
        QCOMPARE(r->current(), r->tab(QStringLiteral("Insert")));
        f = QApplication::focusWidget();
        QCOMPARE(f->accessibleName(), QStringLiteral("Insert"));
        QTest::keyClick(f, Qt::Key_Down);
        f = QApplication::focusWidget();
        QVERIFY(f && r->current()->isAncestorOf(f));
        QTest::keyClick(f, Qt::Key_Right);
        QWidget *g = QApplication::focusWidget();
        QVERIFY(g && g != f && r->current()->isAncestorOf(g));
        QTest::keyClick(g, Qt::Key_Escape);
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(canvas));
    }

    // The keyboard reaches the shared controls: arrow keys move among
    // colors and table cells, Enter picks, and a ring shows the focus while
    // the keyboard is in use (the ribbon's controls drew none and refused
    // the focus).
    void keyboardReachesSharedControls()
    {
        // The nearest control in each direction, in a 3 x 3 grid.
        QWidget grid;
        auto *gl = new QGridLayout(&grid);
        QToolButton *b[3][3];
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) {
                b[r][c] = new QToolButton(&grid);
                b[r][c]->setFixedSize(30, 30);
                b[r][c]->setFocusPolicy(Qt::TabFocus);
                gl->addWidget(b[r][c], r, c);
            }
        grid.show();
        QVERIFY(QTest::qWaitForWindowExposed(&grid));
        QList<QWidget *> all;
        for (auto &row : b) for (QToolButton *x : row) all << x;
        QCOMPARE(jp::nearestInDirection(b[1][1], Qt::Key_Right, all), static_cast<QWidget *>(b[1][2]));
        QCOMPARE(jp::nearestInDirection(b[1][1], Qt::Key_Left, all), static_cast<QWidget *>(b[1][0]));
        QCOMPARE(jp::nearestInDirection(b[1][1], Qt::Key_Up, all), static_cast<QWidget *>(b[0][1]));
        QCOMPARE(jp::nearestInDirection(b[1][1], Qt::Key_Down, all), static_cast<QWidget *>(b[2][1]));
        QCOMPARE(jp::nearestInDirection(b[1][2], Qt::Key_Right, all), static_cast<QWidget *>(nullptr));

        // Colors: the first has the focus; Right moves; Enter picks.
        {
            auto *pop = new jp::ColorPopup(jp::ColorScheme(), true, QStringLiteral("No Color"));
            jp::ColorRef got;
            bool picked = false;
            connect(pop, &jp::ColorPopup::picked, &grid, [&](const jp::ColorRef &c) { got = c; picked = true; });
            pop->showBelow(&grid);
            QVERIFY(QTest::qWaitForWindowExposed(pop));
            QWidget *first = QApplication::focusWidget();
            QVERIFY(first && first->objectName() == QLatin1String("swatch"));
            QVERIFY(!first->accessibleName().isEmpty());
            QTest::keyClick(first, Qt::Key_Right);
            QWidget *second = QApplication::focusWidget();
            QVERIFY(second && second != first && second->objectName() == QLatin1String("swatch"));
            QTest::keyClick(second, Qt::Key_Return);
            QVERIFY(picked);
            QCOMPARE(got.kind(), jp::ColorRef::Scheme);
        }

        // Table size: arrows choose, Enter inserts.
        {
            jp::TableGrid tg;
            int rows = 0, cols = 0;
            connect(&tg, &jp::TableGrid::picked, &tg, [&](int r, int c) { rows = r; cols = c; });
            tg.show();
            QVERIFY(QTest::qWaitForWindowExposed(&tg));
            for (int k : {Qt::Key_Right, Qt::Key_Right, Qt::Key_Right, Qt::Key_Down}) QTest::keyClick(&tg, Qt::Key(k));
            QTest::keyClick(&tg, Qt::Key_Return);
            QCOMPARE(rows, 2);
            QCOMPARE(cols, 3);
        }

        // A gallery: Enter applies the tile with the focus.
        {
            jp::Gallery g(QSize(20, 20), 3);
            g.setItems({jp::GalleryItem{QStringLiteral("a"), QStringLiteral("First"), QIcon(), QString()},
                        jp::GalleryItem{QStringLiteral("b"), QStringLiteral("Second"), QIcon(), QString()}});
            g.setTitle(QStringLiteral("Styles"));
            QString chosen;
            connect(&g, &jp::Gallery::activated, &g, [&](const QString &id) { chosen = id; });
            g.show();
            QVERIFY(QTest::qWaitForWindowExposed(&g));
            auto *list = g.findChild<QListWidget *>();
            QVERIFY(list && (list->focusPolicy() & Qt::TabFocus));
            QCOMPARE(list->accessibleName(), QStringLiteral("Styles"));
            QCOMPARE(list->item(1)->data(Qt::AccessibleTextRole).toString(), QStringLiteral("Second"));
            list->setFocus();
            list->setCurrentRow(1);
            QTest::keyClick(list, Qt::Key_Return);
            QCOMPARE(chosen, QStringLiteral("b"));
        }

        // The ring: shown around the focused control after a key, gone after a click.
        jp::installFocusRing(qApp);
        grid.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&grid));
        b[2][2]->setFocus(Qt::TabFocusReason);
        QTest::keyClick(b[0][0], Qt::Key_Shift);
        b[0][0]->setFocus(Qt::TabFocusReason);
        QCoreApplication::processEvents();
        auto *ring = grid.findChild<QWidget *>(QStringLiteral("jpFocusRing"));
        QVERIFY(ring && ring->isVisible());
        QVERIFY(ring->geometry().contains(b[0][0]->geometry()));
        QTest::mouseClick(b[2][2], Qt::LeftButton);
        QVERIFY(!ring->isVisible());
    }

    // The Styles gallery's samples are the text as it prints: in a dark
    // interface or high contrast they sit on paper (black samples on a dark
    // tile couldn't be read in Windows' high contrast).
    void styleSamplesReadInDark()
    {
        jp::MainWindow w;
        QListWidgetItem *normal = nullptr;
        QTRY_VERIFY([&] {
            for (auto *g : w.findChildren<jp::Gallery *>())
                for (auto *list : g->findChildren<QListWidget *>())
                    for (int i = 0; i < list->count(); ++i)
                        if (list->item(i)->data(Qt::UserRole).toString() == QLatin1String("Normal")) normal = list->item(i);
            return normal != nullptr;
        }());
        const bool wasDark = jp::uiDark();
        auto paper = [&] {
            const QImage img = normal->icon().pixmap(QSize(76, 46)).toImage().convertToFormat(QImage::Format_ARGB32);
            int white = 0, total = 0;
            for (int y = 2; y < img.height() / 2; ++y)
                for (int x = 4; x < img.width() - 4; ++x, ++total) {
                    const QColor c = img.pixelColor(x, y);
                    if (c.alpha() > 200 && c.lightness() > 230) ++white;
                }
            return double(white) / total;
        };
        jp::setUiDark(true);
        const double dark = paper();
        jp::setUiDark(wasDark);
        QVERIFY2(dark > 0.4, qPrintable(QString::number(dark)));
    }

    // Switching light or dark in Options applies at once: it half-applied
    // (the palette and the parts with colors of their own stayed) until a
    // restart.
    void themeSwitchAppliesNow()
    {
        const QPalette pal = qApp->palette();
        const QString sheet = qApp->styleSheet();
        const bool wasDark = jp::uiDark();
        jp::MainWindow w;
        auto *side = w.findChild<QFrame *>(QStringLiteral("jpSide"));
        auto *pages = w.findChild<jp::PagesPane *>();
        QVERIFY(side && pages);
        for (int choice : {2, 1}) {
            const bool dark = choice == 2;
            jp::applyUiTheme(choice);
            QCOMPARE(jp::uiDark(), dark);
            QVERIFY2((qApp->palette().color(QPalette::Window).lightness() < 128) == dark, qPrintable(qApp->palette().color(QPalette::Window).name()));
            QVERIFY(qApp->styleSheet().contains(dark ? QStringLiteral("#22262d") : QStringLiteral("#ffffff")));
            QVERIFY2(side->styleSheet().contains(dark ? QStringLiteral("#171a1f") : QStringLiteral("#f1f2f5")), qPrintable(side->styleSheet()));
            QVERIFY(pages->styleSheet().contains(dark ? QStringLiteral("#171a1f") : QStringLiteral("#f1f2f5")));
            auto *ribbon = w.findChild<jp::Ribbon *>();
            QVERIFY2((ribbon->palette().color(QPalette::Window).lightness() < 128) == dark, qPrintable(ribbon->palette().color(QPalette::Window).name()));
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                w.resize(1200, 800);
                w.show();
                QTest::qWait(200);
                w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/theme-%1.png").arg(dark ? "dark" : "light"));
            }
        }
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
        jp::setUiDark(wasDark);
        qApp->setPalette(pal);
        qApp->setStyleSheet(sheet);
    }

    // Windows' high contrast: the system's colors everywhere (here the
    // stand-in black theme JP_HIGH_CONTRAST=1 selects), none of JeffPub's.
    void highContrastUsesSystemColors()
    {
        const QPalette pal = qApp->palette();
        const QString sheet = qApp->styleSheet();
        const bool wasDark = jp::uiDark();
        jp::MainWindow w;
        qputenv("JP_HIGH_CONTRAST", "1");
        jp::applyUiTheme(1);
        QVERIFY(jp::uiHighContrast());
        QVERIFY(qApp->styleSheet().isEmpty());
        QCOMPARE(qApp->palette().color(QPalette::Window), QColor(Qt::black));
        QCOMPARE(w.findChild<jp::Ribbon *>()->palette().color(QPalette::Window), QColor(Qt::black));
        QCOMPARE(jp::focusRingColor(), qApp->palette().color(QPalette::Highlight));
        QCOMPARE(jp::uiLine(), QColor(Qt::white));   // borders as strong as the text
        QVERIFY(w.findChild<QFrame *>(QStringLiteral("jpSide"))->styleSheet().contains(QStringLiteral("#000000")));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            w.resize(1200, 800);
            w.show();
            QTest::qWait(200);
            w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/theme-contrast.png"));
        }
        qputenv("JP_HIGH_CONTRAST", "0");
        jp::applyUiTheme(1);
        QVERIFY(!jp::uiHighContrast());
        QVERIFY(!qApp->styleSheet().isEmpty());
        qunsetenv("JP_HIGH_CONTRAST");
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
        jp::setUiDark(wasDark);
        qApp->setPalette(pal);
        qApp->setStyleSheet(sheet);
    }

    // English plurals come from a small translation of their own: without
    // it, "Added %n page(s)" reads "page(s)". Every plural in it has its
    // forms filled in (lupdate adds new ones empty).
    void englishPlurals()
    {
        QTranslator en;
        QVERIFY(en.load(QStringLiteral(":/i18n/jeffoffice_en.qm")));
        QCoreApplication::installTranslator(&en);
        QCOMPARE(QCoreApplication::translate("Dialogs", "Added %n page(s) for the rest of the text.", "", 1), QStringLiteral("Added 1 page for the rest of the text."));
        QCOMPARE(QCoreApplication::translate("Dialogs", "Added %n page(s) for the rest of the text.", "", 3), QStringLiteral("Added 3 pages for the rest of the text."));
        QCoreApplication::removeTranslator(&en);
        QVERIFY(!jp::availableUiLanguages().contains(QStringLiteral("en")));
        QFile ts(QStringLiteral(JP_TEST_DATA "/../../translations/jeffoffice_en.ts"));
        QVERIFY(ts.open(QIODevice::ReadOnly));
        const QByteArray xml = ts.readAll();
        QVERIFY2(!xml.contains("type=\"unfinished\"") && !xml.contains("<numerusform></numerusform>"),
                 "translations/jeffoffice_en.ts has a plural without its English forms");
    }

    // Every translation keeps what the program fills in or acts on: its
    // placeholders (%1, %n; a singular may spell the number out), HTML
    // tags, line breaks, & shortcut markers, and both plural forms. A
    // translation that lost one would show the wrong text, or none.
    void translationsKeepTheirMarks()
    {
        static const QRegularExpression placeholder(QStringLiteral("%L?\\d+|%n")), tag(QStringLiteral("</?[A-Za-z][^>]*>"));
        auto all = [](const QRegularExpression &re, const QString &s) {
            QStringList out;
            for (auto m = re.globalMatch(s); m.hasNext();) out << m.next().captured();
            out.sort();
            return out;
        };
        // A shortcut marker is an & right before a letter; "Header & Footer" has none.
        static const QRegularExpression marker(QStringLiteral("(?<!&)&(?=[^\\s&])"));
        auto marks = [](const QString &s) { return int(s.count(marker)); };
        const QDir dir(QStringLiteral(JP_TEST_DATA "/../../translations"));
        QStringList problems;
        int checked = 0;
        for (const QString &name : dir.entryList({QStringLiteral("jeffoffice_*.ts")}, QDir::Files)) {
            if (name == QLatin1String("jeffoffice_en.ts")) continue;
            QFile f(dir.filePath(name));
            QVERIFY(f.open(QIODevice::ReadOnly));
            QXmlStreamReader xml(&f);
            QString context, source;
            bool plural = false;
            QStringList forms;
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement()) {
                    if (xml.name() == QLatin1String("name")) context = xml.readElementText();
                    else if (xml.name() == QLatin1String("message")) {
                        plural = xml.attributes().value(QLatin1String("numerus")) == QLatin1String("yes");
                        forms.clear();
                    } else if (xml.name() == QLatin1String("source")) source = xml.readElementText();
                    else if (xml.name() == QLatin1String("translation")) {
                        const QStringView type = xml.attributes().value(QLatin1String("type"));
                        if (type == QLatin1String("vanished") || type == QLatin1String("obsolete")) {
                            xml.skipCurrentElement();
                            continue;
                        }
                        if (!plural) forms << xml.readElementText();
                    } else if (xml.name() == QLatin1String("numerusform")) forms << xml.readElementText();
                } else if (xml.isEndElement() && xml.name() == QLatin1String("message")) {
                    if (forms.join(QString()).isEmpty()) continue;   // not translated yet: shows in English
                    ++checked;
                    const QString where = QStringLiteral("%1 [%2] \"%3\"").arg(name, context, source);
                    if (plural && (forms.size() != 2 || forms.contains(QString()))) problems << where + QStringLiteral(": plural forms");
                    for (int i = 0; i < forms.size(); ++i) {
                        QStringList want = all(placeholder, source);
                        const QStringList have = all(placeholder, forms[i]);
                        if (plural && i == 0 && have != want) want.removeAll(QStringLiteral("%n"));
                        if (have != want) problems << where + QStringLiteral(": placeholders");
                        if (all(tag, forms[i]) != all(tag, source)) problems << where + QStringLiteral(": tags");
                        if (forms[i].count(QLatin1Char('\n')) != source.count(QLatin1Char('\n'))) problems << where + QStringLiteral(": line breaks");
                        if (marks(forms[i]) != marks(source)) problems << where + QStringLiteral(": & marker");
                    }
                }
            }
            QVERIFY2(!xml.hasError(), qPrintable(name + QStringLiteral(": ") + xml.errorString()));
        }
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("\n"))));
        qInfo("%d translated strings checked", checked);
    }

    // A translated Help topic is one of the English ones, with a title and
    // search words, the same links as the English, and no other company's
    // product names; each language with Help has the program's text too,
    // and Qt's own (standard buttons) built in.
    void translatedHelpIsComplete()
    {
        const QDir help(QStringLiteral(JP_TEST_DATA "/../../resources/help"));
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)\\s]+)\\)"));
        static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office|Word|Excel|PowerPoint|WordArt)\\b"));
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
            if (!jp::availableUiLanguages().contains(lang)) problems << lang + QStringLiteral(": Help without the program's text");
            if (!QFile::exists(QStringLiteral(":/i18n/qtbase_%1.qm").arg(lang))) problems << lang + QStringLiteral(": Qt's own text not built in");
            for (const QString &file : QDir(help.filePath(lang)).entryList({QStringLiteral("*.md")}, QDir::Files)) {
                const QString en = read(help.filePath(QStringLiteral("en/") + file)), md = read(help.filePath(lang + QLatin1Char('/') + file));
                const QString where = lang + QLatin1Char('/') + file;
                if (en.isEmpty()) {
                    problems << where + QStringLiteral(": no English topic");
                    continue;
                }
                if (!md.startsWith(QStringLiteral("# "))) problems << where + QStringLiteral(": no title");
                if (en.contains(QStringLiteral("<!-- keywords:")) && !md.contains(QStringLiteral("<!-- keywords:"))) problems << where + QStringLiteral(": no keywords");
                if (links(md) != links(en)) problems << where + QStringLiteral(": links differ from the English");
                if (const auto m = banned.match(md); m.hasMatch()) problems << where + QStringLiteral(": says ") + m.captured(1);
            }
        }
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
    }

    // A ribbon group is at least as wide as its title, so a translated title
    // longer than the controls under it ("TABELLEN" over one button) shows
    // whole instead of cut short.
    void ribbonGroupFitsItsTitle()
    {
        QAction a(QStringLiteral("Table"));
        jp::RibbonGroup shortTitle(QStringLiteral("Tables")), longTitle(QStringLiteral("Tabellen und Tabellenformate"));
        shortTitle.addLarge(&a);
        longTitle.addLarge(&a);
        QVERIFY(longTitle.sizeHint().width() > shortTitle.sizeHint().width());
        QVERIFY(longTitle.minimumSizeHint().width() > shortTitle.minimumSizeHint().width());
    }

    // The shared libraries name the program from its AppInfo: another app's
    // issue tracker and update server, not JeffPub's; the translations are
    // the suite's, the same for every app.
    void sharedPartsFollowTheAppInfo()
    {
        struct Restore {
            ~Restore() { jp::setAppInfo(jp::pubAppInfo()); }
        } restore;
        jp::setAppInfo({QStringLiteral("JeffDeck"), QStringLiteral("jeffdeck"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffoffice")});
        QVERIFY(jp::help::whatsNewUrl().toString().startsWith(QLatin1String("https://github.com/JeffOffice/jeffoffice/releases/tag/v")));
        QVERIFY(jp::help::feedbackUrl().toString().startsWith(QLatin1String("https://github.com/JeffOffice/jeffoffice/issues/new")));
        const QString digest = QStringLiteral("sha256:63e329c3c1a9aea0f90af5e2519c9315af365cb424a7bd00abb3ad9d201573e7");
        QVERIFY(jp::Updater::trustedInstaller(QStringLiteral("https://github.com/JeffOffice/jeffoffice/releases/download/v0.7.0/JeffDeck-Setup.exe"), QStringLiteral("v0.7.0"), digest));
        QVERIFY(!jp::Updater::trustedInstaller(QStringLiteral("https://github.com/JeffOffice/jeffpub/releases/download/v0.7.0/JeffPub-Setup.exe"), QStringLiteral("v0.7.0"), digest));
        QVERIFY(jp::availableUiLanguages().contains(QStringLiteral("de")));   // jeffoffice_*.qm, whichever app
    }

    // JeffPub's Help lists the topic files directly in :/help/en, and none of
    // the folder an app with Help of its own keeps beside them (JeffDeck's
    // deck/); each app's Help follows its AppInfo.
    void pubHelpListsOnlyItsOwnTopics()
    {
        struct Restore {
            ~Restore() { jp::setAppInfo(jp::pubAppInfo()); }
        } restore;
        QStringList files;
        for (const QString &f : QDir(QStringLiteral(":/help/en")).entryList(QDir::Files, QDir::Name)) {
            QVERIFY2(f.endsWith(QLatin1String(".md")), qPrintable(f));
            files << f.chopped(3);
        }
        const QStringList ids = jp::help::topicIds();
        QCOMPARE(ids, files);
        QVERIFY(ids.contains(QStringLiteral("printing")));
        // The app's folder decides: with JeffDeck's, Help lists that folder's topics, and with JeffPub's again, its own.
        const QStringList deckFiles = QDir(QStringLiteral(":/help/en/deck")).entryList({QStringLiteral("*.md")}, QDir::Files, QDir::Name);
        QVERIFY(!deckFiles.isEmpty());
        QStringList own;
        for (const QString &f : deckFiles) own << f.chopped(3);
        jp::AppInfo other = jp::pubAppInfo();
        other.helpFolder = QStringLiteral("deck");
        jp::setAppInfo(other);
        QCOMPARE(jp::help::topicIds(), own);
        QVERIFY(jp::help::topic(QStringLiteral("mail-merge")).isNull());
        jp::setAppInfo(jp::pubAppInfo());
        QCOMPARE(jp::help::topicIds(), files);
        QVERIFY(!jp::help::topic(QStringLiteral("mail-merge")).isNull());
    }

    // Until Options sets a unit, JeffPub measures in inches where the region
    // does (the United States) and in centimeters elsewhere.
    void defaultUnits()
    {
        QCOMPARE(jp::defaultUnit(QLocale(QStringLiteral("en_US"))), jp::Unit::Inch);
        QCOMPARE(jp::defaultUnit(QLocale(QStringLiteral("es_US"))), jp::Unit::Inch);
        QCOMPARE(jp::defaultUnit(QLocale(QStringLiteral("en_GB"))), jp::Unit::Centimeter);
        QCOMPARE(jp::defaultUnit(QLocale(QStringLiteral("de_DE"))), jp::Unit::Centimeter);
        QCOMPARE(jp::defaultUnit(QLocale(QStringLiteral("pt_BR"))), jp::Unit::Centimeter);
    }

    // A large ribbon button is wide enough for its label's longest word,
    // which can't wrap ("Onlineafbeeldingen"), and a long hyphenated word
    // breaks after its hyphen onto the second line instead of being cut.
    void largeRibbonButtonsFitLongWords()
    {
        QAction word(QStringLiteral("Onlineafbeeldingen")), hyphenated(QStringLiteral("Pagina-achtergrondinstellingen"));
        std::unique_ptr<QToolButton> a(jp::ribbonButton(&word, true, nullptr)), b(jp::ribbonButton(&hyphenated, true, nullptr));
        const QFontMetrics fm(a->font());
        QVERIFY(a->sizeHint().width() >= fm.horizontalAdvance(word.text()) + 14);
        QVERIFY(b->sizeHint().width() < fm.horizontalAdvance(hyphenated.text()));
        QVERIFY(b->sizeHint().width() >= fm.horizontalAdvance(QStringLiteral("achtergrondinstellingen")) + 14);
    }

    // The name each language goes by in Options, as its speakers write it,
    // with the country when one language comes in two.
    void uiLanguageNames()
    {
        QCOMPARE(jp::uiLanguageName(QStringLiteral("de")), QStringLiteral("Deutsch"));
        QCOMPARE(jp::uiLanguageName(QStringLiteral("pt_BR")), QStringLiteral("Português (Brasil)"));
        QCOMPARE(jp::uiLanguageName(QStringLiteral("pt_PT")), QStringLiteral("Português (Portugal)"));
    }

    // File > About lists the suite's apps under the JeffOffice logo, each row
    // named for screen readers, and the five still to come say so.
    void aboutListsTheSuiteApps()
    {
        QVERIFY(!QImage(QStringLiteral(":/brand/jeffoffice-lockup.png")).isNull());
        QVERIFY(!QImage(QStringLiteral(":/brand/jeffoffice-lockup-on-dark.png")).isNull());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        w.showBackstage(QStringLiteral("about"));
        const QStringList rows = {QStringLiteral("JeffPub, Publications, You're using it"), QStringLiteral("JeffDeck, Presentations, Available"),
                                  QStringLiteral("JeffWrite, Documents, Preview"), QStringLiteral("JeffCrunch, Spreadsheets, Preview"),
                                  QStringLiteral("JeffMail, Email and calendar, Coming soon"), QStringLiteral("JeffBase, Databases, Coming soon")};
        QStringList named;
        int comingSoon = 0;
        QStringList appNames;
        QLabel *version = nullptr;
        for (QWidget *c : w.findChildren<QWidget *>()) {
            if (!c->isVisibleTo(&w)) continue;
            if (rows.contains(c->accessibleName())) named << c->accessibleName();
            if (auto *l = qobject_cast<QLabel *>(c)) {
                if (l->text() == QLatin1String("Coming soon")) ++comingSoon;
                if (l->text().startsWith(QLatin1String("Version ")) && l->text().contains(QLatin1String("Qt"))) version = l;
                for (const QString &r : rows)
                    if (l->text() == r.section(QLatin1String(", "), 0, 0)) appNames << l->text();
            }
        }
        named.sort();
        QStringList want = rows;
        want.sort();
        QCOMPARE(named, want);
        QCOMPARE(comingSoon, 2);   // JeffDeck is out, and JeffWrite and JeffCrunch are previews
        for (const QString &r : rows) QVERIFY2(appNames.contains(r.section(QLatin1String(", "), 0, 0)), qPrintable(r));
        // The suite comes first on the page, above the version.
        QVERIFY(version);
        QLabel *logo = nullptr;
        for (QLabel *l : w.findChildren<QLabel *>())
            if (l->isVisibleTo(&w) && l->accessibleName() == QLatin1String("JeffOffice")) logo = l;
        QVERIFY(logo);
        QVERIFY(!logo->pixmap().isNull());
        QTRY_VERIFY(logo->mapTo(&w, QPoint(0, 0)).y() < version->mapTo(&w, QPoint(0, 0)).y());
    }

    // The ribbon's text, in resources/ribbon.json where lupdate can't see
    // it, is listed for translators in apps/pub/app/ribbon_strings.cpp; this
    // keeps that list current (run tools/ribbon_strings.py).
    void ribbonStringsListed()
    {
        QFile json(QStringLiteral(":/ribbon.json"));
        QVERIFY(json.open(QIODevice::ReadOnly));
        QFile listed(QStringLiteral(JP_TEST_DATA "/../../apps/pub/app/ribbon_strings.cpp"));
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

    // AutoRecover keeps one copy per document and run: two "Cover.pub"
    // files in different folders overwrote each other's copy, copies were
    // never offered after a crash, and never removed.
    // A page fitted to the window at the size where a scroll bar is just
    // needed: the bar came, the page area shrank, the page refitted smaller,
    // the bar went, and so on, a whole processor busy while JeffPub sat idle
    // (Windows' usual window size hit it). Each window size must settle.
    void fittedPageSettles()
    {
        QString err;
        auto doc = importPublisherFile(QStringLiteral(JP_TEST_DATA "/pub/poi-Sample2.pub"), &err);
        QVERIFY2(doc, qPrintable(err));
        jp::MainWindow w;
        w.editor()->setDocument(std::move(doc));
        w.show();
        auto *canvas = w.findChild<jp::Canvas *>();
        QVERIFY(canvas);
        struct Resizes : QObject {
            jp::Canvas *canvas = nullptr;
            int n = 0;
            bool runaway = false;
            bool eventFilter(QObject *, QEvent *e) override
            {
                if (e->type() == QEvent::Resize && ++n > 50 && !runaway) {
                    runaway = true;
                    canvas->zoomToFit(jp::Canvas::Fit::None);   // ends the loop, so the test fails rather than hangs
                }
                return false;
            }
        } resizes;
        resizes.canvas = canvas;
        canvas->viewport()->installEventFilter(&resizes);
        // Heights on both sides of where a bar is just needed, whatever the
        // system's ribbon and font sizes.
        for (int h = 560; h <= 800; ++h) {
            canvas->zoomToFit(jp::Canvas::Fit::WholePage);
            resizes.n = 0;
            w.resize(1000, h);
            for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
            QVERIFY2(!resizes.runaway, qPrintable(QStringLiteral("the page area kept resizing at 1000x%1").arg(h)));
        }
    }

    void autoRecoverCopies()
    {
        QTemporaryDir dir;
        jp::recovery::setRoot(dir.filePath(QStringLiteral("AutoRecover")));
        // Named for the document and its full path.
        const QString a = jp::recovery::copyPath(QStringLiteral("/one/Cover.pub"), QStringLiteral("Cover"), 1);
        const QString b = jp::recovery::copyPath(QStringLiteral("/two/Cover.pub"), QStringLiteral("Cover"), 2);
        QVERIFY(!a.isEmpty() && a != b);
        QCOMPARE(jp::recovery::copyPath(QStringLiteral("/one/Cover.pub"), QStringLiteral("Cover"), 7), a);
        QVERIFY(QFileInfo(a).fileName().startsWith(QLatin1String("Cover ")));
        QVERIFY(jp::recovery::copyPath(QString(), QStringLiteral("Publication1"), 1) != jp::recovery::copyPath(QString(), QStringLiteral("Publication1"), 2));

        // A run that crashed left a folder whose lock nobody holds; one
        // that's still going holds its lock; an older version left a copy
        // loose in the folder. This run's own copies aren't offered.
        const auto doc = jp::Document::blank(QSizeF(612, 792));
        QVERIFY(jp::saveRecoveryCopy(*doc, a, QStringLiteral("/one/Cover.pub"), QStringLiteral("Cover")));
        const QString crashed = dir.filePath(QStringLiteral("AutoRecover/crashed"));
        QDir().mkpath(crashed);
        QVERIFY(jp::saveRecoveryCopy(*doc, crashed + QStringLiteral("/Flyer 0123abcd.jpub"), QStringLiteral("/docs/Flyer.jpub"), QStringLiteral("Flyer")));
        const QString live = dir.filePath(QStringLiteral("AutoRecover/live"));
        QDir().mkpath(live);
        QLockFile liveLock(live + QStringLiteral("/session.lock"));
        QVERIFY(liveLock.tryLock(0));
        QVERIFY(jp::saveRecoveryCopy(*doc, live + QStringLiteral("/Card 0123abcd.jpub"), QString(), QStringLiteral("Card")));
        QVERIFY(QFile::copy(a, dir.filePath(QStringLiteral("AutoRecover/Publication1.autorecover.jpub"))));
        const QVector<jp::recovery::Recovered> found = jp::recovery::orphans();
        QStringList titles;
        for (const auto &r : found) titles << r.title;
        titles.sort();
        QCOMPARE(titles, (QStringList{QStringLiteral("Flyer"), QStringLiteral("Publication1")}));
        for (const auto &r : found)
            if (r.title == QLatin1String("Flyer")) {
                QCOMPARE(r.source, QStringLiteral("/docs/Flyer.jpub"));
                jp::recovery::discard(r);
            }
        QVERIFY(!QDir(crashed).exists());   // the crashed run's folder goes with its last copy

        // A normal exit removes this run's folder.
        const QString session = jp::recovery::sessionDir();
        QVERIFY(QDir(session).exists());
        jp::recovery::endSession();
        QVERIFY(!QDir(session).exists());

        // A window keeps a copy while its work is unsaved, and drops it
        // once the work is saved or the window closes.
        {
            jp::MainWindow w;
            w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
            jp::Editor *ed = w.editor();
            ed->addItem(ed->newTextBox(QRectF(72, 72, 200, 100)));
            QVERIFY(ed->isModified());
            w.autoRecover();
            const QString copy = w.recoveryCopy();
            QVERIFY(QFile::exists(copy));
            QVERIFY(w.saveTo(dir.filePath(QStringLiteral("Saved.jpub"))));
            QVERIFY(!QFile::exists(copy));
            ed->addItem(ed->newTextBox(QRectF(72, 300, 200, 100)));
            w.autoRecover();
            QVERIFY(QFile::exists(w.recoveryCopy()));
            QVERIFY(w.recoveryCopy() != copy);   // named for Saved.jpub now
            const QString second = w.recoveryCopy();
            ed->setDocument(jp::Document::blank(QSizeF(612, 792)));   // File > New in this window
            QVERIFY(!QFile::exists(second));
        }

        // Recovered work opens under its file's name, marked as recovered
        // in the window only: the publication's own title (saved in it and
        // in its PDFs) became "Cover (Recovered)".
        {
            auto titled = jp::Document::blank(QSizeF(612, 792));
            titled->props.title = QStringLiteral("Spring Newsletter");
            const QString lost = dir.filePath(QStringLiteral("AutoRecover/lost"));
            QDir().mkpath(lost);
            QVERIFY(jp::saveRecoveryCopy(*titled, lost + QStringLiteral("/Cover 0123abcd.jpub"), QStringLiteral("/docs/Cover.jpub"), QStringLiteral("Cover")));
            const QVector<jp::recovery::Recovered> left = jp::recovery::orphans();
            QCOMPARE(left.size(), 2);   // and the loose Publication1 copy
            const auto it = std::find_if(left.begin(), left.end(), [](const auto &r) { return r.title == QLatin1String("Cover"); });
            QVERIFY(it != left.end());
            jp::MainWindow w;
            QCOMPARE(w.openRecovered(*it), &w);
            QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("Spring Newsletter"));
            QCOMPARE(w.editor()->displayName(), QStringLiteral("Cover"));
            QVERIFY2(w.windowTitle().startsWith(QLatin1String("Cover (Recovered)")), qPrintable(w.windowTitle()));
            QVERIFY(w.editor()->isModified());
            QVERIFY(QFileInfo(w.recoveryCopy()).fileName().startsWith(QLatin1String("Cover ")));   // a second crash still says Cover
            QVERIFY(!QDir(lost).exists());   // the old copy goes once this run has its own
            QVERIFY(w.saveTo(dir.filePath(QStringLiteral("Cover.jpub"))));
            QVERIFY2(!w.windowTitle().contains(QLatin1String("Recovered")), qPrintable(w.windowTitle()));
            QCOMPARE(w.editor()->doc()->props.title, QStringLiteral("Spring Newsletter"));
        }

        // Long names are cut between characters, never inside one.
        const QString emoji = QString(59, QLatin1Char('a')) + QString::fromUtf8("\xF0\x9F\x98\x80");
        QVERIFY(QFileInfo(jp::recovery::copyPath(QString(), emoji, 1)).fileName().isValidUtf16());
        jp::recovery::endSession();
        jp::recovery::setRoot(QString());
    }

    // A file the system hands over (macOS: double-clicked in the Finder)
    // opens in the empty window, and in a new one once that has a file.
    void fileOpenEvent()
    {
        jp::MainWindow w;
        w.show();
        jp::installFileOpenHandler(qApp);
        const QString first = QStringLiteral(JP_TEST_DATA "/pub/poi-SampleBrochure.pub");
        QFileOpenEvent open1(first);
        QCoreApplication::sendEvent(qApp, &open1);
        QCOMPARE(QFileInfo(w.editor()->filePath()).fileName(), QFileInfo(first).fileName());
        const QString second = QStringLiteral(JP_TEST_DATA "/pub/poi-SampleNewsletter.pub");
        QFileOpenEvent open2(second);
        QCoreApplication::sendEvent(qApp, &open2);
        QCOMPARE(QFileInfo(w.editor()->filePath()).fileName(), QFileInfo(first).fileName());
        jp::MainWindow *other = nullptr;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *mw = qobject_cast<jp::MainWindow *>(top); mw && mw != &w && mw->editor()->filePath().endsWith(QLatin1String("poi-SampleNewsletter.pub"))) other = mw;
        QVERIFY(other);
        other->close();
    }

    // File > Options holds the statistics choice the first start points to,
    // and a new AutoRecover interval reaches windows already open.
    void optionsApplyToOpenWindows()
    {
        jp::MainWindow w;
        const bool wasOn = telemetry::enabled();
        const QVariant minutes = Settings::get().value(QStringLiteral("save/autoRecoverMinutes"));
        telemetry::setEnabled(false);
        int statsShown = -1;
        QTimer::singleShot(0, [&statsShown] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            auto *stats = d->findChild<QCheckBox *>(QStringLiteral("stats"));
            auto *spin = d->findChild<QSpinBox *>(QStringLiteral("autoRecoverMinutes"));
            if (stats && spin) {
                statsShown = stats->isChecked();
                stats->setChecked(true);
                spin->setValue(3);
            }
            d->accept();   // whatever was found, so a failure can't leave it open
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QCOMPARE(statsShown, 0);
        QVERIFY(telemetry::enabled());
        QCOMPARE(w.autoRecoverInterval(), 3 * 60 * 1000);
        telemetry::setEnabled(wasOn);
        Settings::get().setValue(QStringLiteral("save/autoRecoverMinutes"), minutes);
    }

    // About: the third-party table fits its card at a modest window size
    // (its last rows and long license names were cut off).
    void aboutLicensesFit()
    {
        for (const QSize size : {QSize(1000, 700), QSize(1400, 900)}) {
            jp::MainWindow w;
            w.resize(size);
            w.show();
            w.showBackstage(QStringLiteral("about"));
            QTest::qWait(100);   // the scroll area lays out in steps
            QLabel *table = nullptr;
            for (QLabel *l : w.findChildren<QLabel *>())
                if (l->isVisibleTo(&w) && l->text().contains(QLatin1String("<table")) && l->text().contains(QLatin1String("ISBN range table"))) table = l;
            QVERIFY(table);
            // A copy of the label's own document (its wrapping and margin),
            // laid out at the label's width.
            QTextDocument *own = table->findChild<QTextDocument *>();
            QVERIFY(own);
            std::unique_ptr<QTextDocument> copy(own->clone());
            QTextDocument &doc = *copy;
            doc.setDefaultFont(own->defaultFont());
            doc.setDefaultTextOption(own->defaultTextOption());
            doc.setDocumentMargin(own->documentMargin());
            doc.setTextWidth(table->contentsRect().width());
            QVERIFY2(doc.idealWidth() <= table->contentsRect().width() + 1,
                     qPrintable(QStringLiteral("%1 > %2").arg(doc.idealWidth()).arg(table->contentsRect().width())));
            QVERIFY2(doc.size().height() <= table->contentsRect().height() + 1,
                     qPrintable(QStringLiteral("%1 > %2 at %3").arg(doc.size().height()).arg(table->contentsRect().height()).arg(size.width())));
            for (QLabel *l : table->window()->findChildren<QLabel *>())
                if (l->isVisibleTo(&w) && l->wordWrap() && l->heightForWidth(l->width()) > 0)
                    QVERIFY2(l->height() >= l->heightForWidth(l->width()), qPrintable(l->text().left(40)));
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR") && size.width() == 1000) {
                table->window()->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/about.png");
                table->parentWidget()->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/about-licenses.png");
            }
        }
    }

    // JeffPub opens with the window size it had when it last closed.
    void windowSizeKept()
    {
        {
            jp::MainWindow w;
            w.show();
            w.resize(640, 480);   // within the test screen (a restored window is kept on screen)
            QApplication::processEvents();
            QVERIFY(w.close());
        }
        {
            jp::MainWindow again;
            QCOMPARE(again.size(), QSize(640, 480));
        }
        jp::Settings::get().setValue(QStringLiteral("ui/windowGeometry"), QVariant());
        jp::MainWindow fresh;
        QCOMPARE(fresh.size(), QSize(1400, 900));
    }

    // A narrow window collapses ribbon groups into buttons instead of cutting
    // them off, and a collapsed group opens with its commands.
    void ribbonCollapses()
    {
        jp::MainWindow w;
        w.resize(800, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        QTest::qWait(50);
        jp::RibbonTab *home = w.findChild<jp::Ribbon *>()->current();
        QVERIFY(home->collapsedCount() > 0);
        w.resize(2400, 700);
        QTest::qWait(50);
        QCOMPARE(home->collapsedCount(), 0);
    }

    // Tabbed dialogs open wide enough for every tab, and their color
    // pickers draw as full-width fields.
    void dialogTabsFit()
    {
        jp::installUiPolish();
        jp::MainWindow w;
        w.resize(1200, 800);
        w.show();
        jp::Fill fill;
        bool checked = false;
        QTimer::singleShot(300, [&] {
            QWidget *mod = QApplication::activeModalWidget();
            if (!mod) return;
            auto *tabs = mod->findChild<QTabWidget *>();
            if (tabs) {
                QTabBar *bar = tabs->tabBar();
                checked = true;
                for (int i = 0; i < bar->count(); ++i)
                    if (!bar->rect().contains(bar->tabRect(i))) checked = false;
                for (auto *b : mod->findChildren<jp::ColorButton *>())
                    if (b->isVisible() && b->width() < 120) checked = false;
            }
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) mod->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/fillfx.png");
            mod->close();
        });
        jp::fillEffectsDialog(&w, w.editor(), fill, QStringLiteral("Format Background"));
        QVERIFY(checked);
    }
};

JP_TEST_CLASS(PubWindowTests)
#include "tst_pub_window.moc"
