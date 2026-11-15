#include "app/commandsearch.h"

#include "app/appwindow.h"
#include "app/filescreen.h"
#include "app/help.h"
#include "app/i18n.h"
#include "app/icons.h"
#include "app/plaintext.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/widgets.h"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QFrame>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QListWidget>
#include <QMenu>
#include <QRegularExpression>
#include <QScreen>
#include <QSet>
#include <QShortcut>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <algorithm>

namespace jp {

namespace cmdsearch {

QString fold(const QString &text)
{
    QString out = text.normalized(QString::NormalizationForm_D);
    out.erase(std::remove_if(out.begin(), out.end(), [](QChar c) { return c.category() == QChar::Mark_NonSpacing; }), out.end());
    return out.toLower();
}

QStringList tokens(const QString &text)
{
    static const QRegularExpression split(QStringLiteral("[^\\p{L}\\p{N}]+"), QRegularExpression::UseUnicodePropertiesOption);
    return fold(text).split(split, Qt::SkipEmptyParts);
}

namespace {

// Whether two words differ by at most one letter: one changed, one left out or added, or two side by side swapped.
bool withinOne(const QString &a, const QString &b)
{
    const int n = a.size(), m = b.size();
    if (std::abs(n - m) > 1) return false;
    int i = 0;
    while (i < n && i < m && a[i] == b[i]) ++i;
    if (i == n && i == m) return true;
    if (n == m) {
        if (a.mid(i + 1) == b.mid(i + 1)) return true;                                          // one letter changed
        return i + 1 < n && a[i] == b[i + 1] && a[i + 1] == b[i] && a.mid(i + 2) == b.mid(i + 2);   // two swapped
    }
    return n > m ? a.mid(i + 1) == b.mid(i) : a.mid(i) == b.mid(i + 1);
}

// 3 whole word, 2 its start, 1 its start within one wrong letter (only for a word of four letters or more: a short one would fit too much), 0 none.
int wordFit(const QStringList &toks, const QString &w)
{
    int best = 0;
    for (const QString &t : toks) {
        if (t == w) return 3;
        if (t.startsWith(w)) best = std::max(best, 2);
        else if (best < 1 && w.size() >= 4)
            for (int len = w.size() - 1; len <= w.size() + 1; ++len)
                if (len <= t.size() && withinOne(t.left(len), w)) {
                    best = 1;
                    break;
                }
    }
    return best;
}

// A tooltip as plain text (the tooltips the program sets from text that looks like markup are wrapped so as to show it as it is). Names are never run through this: they are shown as written.
QString plainTip(const QString &tip)
{
    if (!Qt::mightBeRichText(tip)) return tip.simplified();
    return QTextDocumentFragment::fromHtml(tip).toPlainText().simplified();
}

QString cleaned(QString s)
{
    s.remove(QLatin1Char('&'));
    if (s.endsWith(QLatin1String("..."))) s.chop(3);
    if (s.endsWith(QChar(0x2026))) s.chop(1);
    return s.simplified();
}

QString nameOf(const QAction *a) { return cleaned(a->text()); }

// The English texts a shown text was translated from (none for English itself).
QStringList englishOf(const QString &shown)
{
    QStringList out;
    for (const QString &s : sourceTexts(shown))
        if (const QString c = cleaned(s); !c.isEmpty() && !out.contains(c)) out << c;
    return out;
}

// What stands for a text in an id that outlasts the language: its English, or itself.
QString keyOf(const QString &shown)
{
    QStringList en = englishOf(shown);
    en.sort();
    return en.isEmpty() ? cleaned(shown) : en.first();
}

Entry makeCommand(QAction *a, const QString &where, const QString &whereKey, int tab, QToolButton *button, const Words &words)
{
    Entry e;
    e.name = nameOf(a);
    if (e.name.isEmpty()) return e;
    e.where = where;
    e.tip = plainTip(a->toolTip());
    if (e.tip == e.name) e.tip.clear();
    e.id = QStringLiteral("cmd:") + (a->objectName().isEmpty() ? whereKey + QLatin1Char('/') + keyOf(a->text()) : a->objectName());
    e.icon = a->icon();
    e.action = a;
    e.button = button;
    e.tab = tab;
    for (const QString &en : englishOf(a->text()))
        if (en != e.name) e.english << en;
    e.english << englishOf(a->toolTip());
    e.extra = words.value(QStringLiteral("cmd:") + a->objectName());
    return e;
}

} // namespace

void index(Entry &e)
{
    e.nameTok = tokens(e.name);
    e.whereTok = tokens(e.where);
    e.tipTok = tokens(e.tip);
    e.englishTok = tokens(e.english.join(QLatin1Char(' ')));
    e.extraTok = tokens(e.extra.join(QLatin1Char(' ')));
    e.foldedName = fold(e.name);
    e.indexed = true;
}

Words parseWords(const QByteArray &json)
{
    Words out;
    const QJsonObject root = QJsonDocument::fromJson(json).object();
    for (const char *section : {"commands", "file", "options"}) {
        const QString prefix = QString::fromLatin1(section == QByteArray("commands") ? "cmd" : section) + QLatin1Char(':');
        const QJsonObject o = root.value(QString::fromLatin1(section)).toObject();
        for (auto it = o.begin(); it != o.end(); ++it) {
            QStringList list;
            for (const QJsonValue &v : it.value().toArray())
                if (v.isString()) list << v.toString();
            out.insert(prefix + it.key(), list);
        }
    }
    return out;
}

namespace {

// The score of an entry whose words are kept (index()), for a query already in words.
int scoreIndexed(const Entry &e, const QStringList &want, const QString &foldedQuery)
{
    if (want.isEmpty()) return -1;
    int total = 0;
    for (const QString &w : want) {
        const int fits[] = {wordFit(e.nameTok, w) * 10, wordFit(e.englishTok, w) * 9, wordFit(e.extraTok, w) * 8, wordFit(e.tipTok, w) * 3, wordFit(e.whereTok, w) * 2};
        const int best = *std::max_element(std::begin(fits), std::end(fits));
        if (!best) return -1;
        total += best;
    }
    if (e.foldedName == foldedQuery) total += 60;
    else if (e.foldedName.startsWith(foldedQuery)) total += 30;
    else if (e.foldedName.contains(foldedQuery)) total += 15;
    return total;
}

} // namespace

int score(const Entry &e, const QString &query)
{
    const QStringList want = tokens(query);
    const QString q = fold(query).simplified();
    if (e.indexed) return scoreIndexed(e, want, q);
    Entry copy = e;
    index(copy);
    return scoreIndexed(copy, want, q);
}

QList<Entry> find(const QList<Entry> &all, const QString &query, int limit)
{
    const QStringList want = tokens(query);   // (the query is read once; each entry's words were read when it was made)
    const QString q = fold(query).simplified();
    QList<QPair<int, int>> hits;   // score, place
    for (int i = 0; i < all.size(); ++i) {
        int s;
        if (all[i].indexed) s = scoreIndexed(all[i], want, q);
        else {
            Entry copy = all[i];
            index(copy);
            s = scoreIndexed(copy, want, q);
        }
        if (s >= 0) hits << qMakePair(s, i);
    }
    std::stable_sort(hits.begin(), hits.end(), [](const auto &a, const auto &b) { return a.first > b.first; });
    QList<Entry> out;
    for (const auto &h : hits) {
        if (out.size() >= limit) break;
        out << all[h.second];
    }
    return out;
}

namespace {

void walkMenu(QMenu *m, const QString &where, const QString &whereKey, int tab, QToolButton *owner, const Words &words, const std::function<void(const Entry &)> &add)
{
    for (QAction *a : m->actions()) {
        if (a->isSeparator() || qobject_cast<QWidgetAction *>(a)) continue;
        if (QMenu *sub = a->menu()) {
            walkMenu(sub, where + QStringLiteral(" > ") + nameOf(a), whereKey + QStringLiteral(" > ") + keyOf(a->text()), tab, owner, words, add);
            continue;
        }
        if (!a->isVisible()) continue;
        add(makeCommand(a, where, whereKey, tab, owner, words));   // (the button that owns the menu: when it is off, so is the item)
    }
}

} // namespace

QRect popupRect(const QRect &box, const QSize &size, const QRect &screen)
{
    const int x = std::clamp(box.right() + 1 - size.width(), screen.left(), std::max(screen.left(), screen.right() + 1 - size.width()));
    const int below = screen.bottom() + 1 - (box.bottom() + 3), above = box.top() - 3 - screen.top();
    if (below < std::min(size.height(), 200) && above > below) {   // (no room under it: over it)
        const int h = std::min(size.height(), above);
        return QRect(QPoint(x, box.top() - 3 - h), QSize(size.width(), h));
    }
    return QRect(QPoint(x, box.bottom() + 3), QSize(size.width(), std::min(size.height(), std::max(below, 1))));
}

QList<Entry> collect(AppWindow *win, const Words &words)
{
    QList<Entry> out;
    QHash<QString, int> placeOf;   // an id's place in `out`
    int galleryItems = 0;
    auto add = [&](const Entry &e) {
        if (e.name.isEmpty()) return;
        const auto it = placeOf.constFind(e.id);
        if (it == placeOf.constEnd()) {
            placeOf.insert(e.id, int(out.size()));
            out << e;
        } else if (!available(out[*it], win) && available(e, win)) {
            out[*it] = e;   // (a command on two tabs: the copy that can be used stands for it)
        }
    };
    auto addGallery = [&](QWidget *g, const QVector<GalleryItem> &items, const QString &where, const QString &whereKey, int tab) {
        for (const GalleryItem &it : items) {
            if (galleryItems >= kMaxGalleryItems) return;
            const QString name = it.tip.simplified();   // (as written: a name is not read as markup)
            if (name.isEmpty() || it.id.isEmpty()) continue;
            Entry e;
            e.kind = Kind::Gallery;
            e.id = QStringLiteral("gal:") + whereKey + QLatin1Char('/') + it.id;
            e.name = name;
            e.where = where;
            e.icon = it.icon;
            e.gallery = g;
            e.itemId = it.id;
            e.tab = tab;
            e.english = englishOf(it.tip);
            if (placeOf.contains(e.id)) continue;
            add(e);
            ++galleryItems;
        }
    };
    if (Ribbon *r = win ? win->ribbon() : nullptr) {
        for (int i = 0; i < r->tabCount(); ++i) {
            RibbonTab *page = r->tabPage(i);
            if (!page || r->tabUserHidden(i)) continue;
            for (RibbonGroup *g : page->groups()) {
                if (g->userHidden()) continue;
                const QString where = r->tabTitle(i) + QStringLiteral(" > ") + g->title();
                const QString whereKey = keyOf(r->tabTitle(i)) + QStringLiteral(" > ") + keyOf(g->title());
                for (Gallery *gal : g->findChildren<Gallery *>()) addGallery(gal, gal->items(), where, whereKey, i);
                for (QToolButton *b : g->findChildren<QToolButton *>()) {
                    if (b == g->launcher()) continue;
                    if (auto *gb = qobject_cast<GalleryButton *>(b)) {
                        addGallery(gb, gb->items(), where, whereKey, i);
                        continue;
                    }
                    QAction *a = b->defaultAction();
                    if (a && !a->isSeparator()) {
                        Entry e = makeCommand(a, where, whereKey, i, b, words);
                        e.opensMenu = b->menu() && b->popupMode() == QToolButton::InstantPopup;
                        add(e);
                    }
                    if (QMenu *m = b->menu()) {
                        const QString label = a ? nameOf(a) : QString(b->text()).remove(QLatin1Char('&')).simplified();   // (a bare drop-down has the button's text, or none)
                        const QString labelKey = a ? keyOf(a->text()) : keyOf(b->text());
                        walkMenu(m, label.isEmpty() ? where : where + QStringLiteral(" > ") + label, labelKey.isEmpty() ? whereKey : whereKey + QStringLiteral(" > ") + labelKey, i, b, words, add);
                    }
                }
            }
        }
    }
    const QString fileWhere = QCoreApplication::translate("Ribbon", "File");
    if (FileScreen *fs = win ? win->fileScreen() : nullptr) {
        for (const FileScreen::Entry &f : fs->entries()) {
            Entry e;
            e.kind = Kind::File;
            e.key = f.key;
            e.id = QStringLiteral("file:") + f.key;
            e.name = f.label;
            e.where = fileWhere;
            e.icon = icon(f.icon);
            e.english = englishOf(f.label);
            e.extra = words.value(e.id);
            add(e);
        }
    }
    // Options: the pages every app has, then the app's own.
    struct Page {
        QString key, shown, group;
        QStringList words;   // (the words that go with the pages every app has)
    };
    const QString customize = QCoreApplication::translate("Dialogs", "Customize");
    QList<Page> pages = {{QStringLiteral("General"), QCoreApplication::translate("Dialogs", "General"), QString(), {QStringLiteral("user name"), QStringLiteral("initials"), QStringLiteral("theme"), QStringLiteral("dark mode"), QStringLiteral("language"), QStringLiteral("updates")}},
                         {QStringLiteral("Proofing"), QCoreApplication::translate("Dialogs", "Proofing"), QString(), {QStringLiteral("spelling"), QStringLiteral("autocorrect"), QStringLiteral("grammar"), QStringLiteral("smart quotes")}},
                         {QStringLiteral("Customize/Ribbon"), QCoreApplication::translate("Dialogs", "Ribbon"), customize, {QStringLiteral("tabs"), QStringLiteral("groups"), QStringLiteral("customize ribbon")}},
                         {QStringLiteral("Customize/Quick Toolbar"), QCoreApplication::translate("Dialogs", "Quick Toolbar"), customize, {QStringLiteral("quick access"), QStringLiteral("toolbar")}},
                         {QStringLiteral("Customize/Keyboard"), QCoreApplication::translate("Dialogs", "Keyboard"), customize, {QStringLiteral("shortcuts"), QStringLiteral("keys"), QStringLiteral("hotkeys")}}};
    if (win)
        for (const QString &title : win->optionsPageTitles()) {
            const QString en = sourceText(title);
            pages << Page{en.isEmpty() ? title : en, title, QString(), {}};
        }
    for (const Page &p : std::as_const(pages)) {
        Entry e;
        e.kind = Kind::Options;
        e.key = p.key;
        e.id = QStringLiteral("options:") + p.key;
        e.name = QCoreApplication::translate("CommandSearch", "Options: %1").arg(p.shown);
        e.where = fileWhere + QStringLiteral(" > ") + QCoreApplication::translate("Ribbon", "Options") + (p.group.isEmpty() ? QString() : QStringLiteral(" > ") + p.group);
        e.icon = icon(QStringLiteral("settings"));
        e.english << p.key.section(QLatin1Char('/'), -1) << QStringLiteral("options");
        e.extra = p.words + words.value(e.id);
        add(e);
    }
    for (Entry &e : out) index(e);   // (each entry's words are read once, here: a keystroke reads only the query)
    return out;
}

bool available(const Entry &e, AppWindow *win)
{
    switch (e.kind) {
    case Kind::Command: {
        if (!e.action || !e.action->isEnabled() || !e.action->isVisible()) return false;
        if (e.button && !e.button->isEnabled()) return false;   // (a button, or a group, a tab page, or anything above it up to the window, that is turned off: read-only does this)
        Ribbon *r = win ? win->ribbon() : nullptr;
        return !(r && e.tab >= 0 && !r->tabVisible(e.tab));
    }
    case Kind::Gallery: {
        if (!e.gallery || !e.gallery->isEnabled()) return false;
        Ribbon *r = win ? win->ribbon() : nullptr;
        return !(r && e.tab >= 0 && !r->tabVisible(e.tab));
    }
    case Kind::File:
    case Kind::Options:
    case Kind::Help: return true;
    }
    return true;
}

} // namespace cmdsearch

// ---------------- the box ----------------

namespace {
QRect s_testScreen;
}

void CommandSearchBox::setScreenForTests(const QRect &screen) { s_testScreen = screen; }

QKeySequence CommandSearchBox::shortcutFor(bool mac)
{
    return mac ? QKeySequence(Qt::META | Qt::ALT | Qt::Key_Q) : QKeySequence(Qt::ALT | Qt::Key_Q);
}

QKeySequence CommandSearchBox::shortcut()
{
#ifdef Q_OS_MACOS
    return shortcutFor(true);
#else
    return shortcutFor(false);
#endif
}

QString CommandSearchBox::recentKey() { return QStringLiteral("cmdsearch/recent"); }

CommandSearchBox::CommandSearchBox(AppWindow *win, const QString &wordsResource) : QLineEdit(), m_win(win), m_resource(wordsResource)
{
    setObjectName(QStringLiteral("jpCommandSearch"));
    const QString keys = shortcut().toString(QKeySequence::NativeText);
    setPlaceholderText(tr("Search (%1)").arg(keys));
    addAction(icon(QStringLiteral("search")), QLineEdit::LeadingPosition);
    for (QToolButton *b : findChildren<QToolButton *>()) { b->setAccessibleName(tr("Search")); b->setAccessibleDescription(tr("The search box's icon. Type in the box beside it.")); b->setWhatsThis(b->accessibleDescription()); b->setFocusPolicy(Qt::TabFocus); }   // plain: reviewed: its own string   // (the icon at the left)
    setAccessibleName(tr("Search commands"));
    setAccessibleDescription(tr("Type what you want to do. Down goes to the commands found, and Enter runs the one chosen. Escape goes back to the document."));
    setToolTip(plainToolTip(tr("Search for a command and run it (%1)").arg(keys)));
    installEventFilter(this);
    m_popup = new QFrame(win, Qt::ToolTip | Qt::FramelessWindowHint);
    m_popup->setObjectName(QStringLiteral("jpCommandSearchPopup"));
    m_popup->setFrameShape(QFrame::StyledPanel);
    m_popup->setFocusPolicy(Qt::NoFocus);
    m_popup->setAttribute(Qt::WA_ShowWithoutActivating);
    auto *v = new QVBoxLayout(m_popup);
    v->setContentsMargins(2, 2, 2, 2);
    m_list = new QListWidget(m_popup);
    m_list->setObjectName(QStringLiteral("jpCommandSearchResults"));
    m_list->setAccessibleName(tr("Commands found"));
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setIconSize(QSize(18, 18));
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    v->addWidget(m_list);
    m_popup->hide();
    connect(this, &QLineEdit::textChanged, this, [this] { refill(); });
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *it) {
        const int row = it->data(Qt::UserRole).toInt();
        if (row >= 0 && row < m_rows.size() && (it->flags() & Qt::ItemIsEnabled)) {
            m_list->setCurrentItem(it);
            runCurrent();
        }
    });
    auto *key = new QShortcut(shortcut(), win);
    key->setContext(Qt::WindowShortcut);
    connect(key, &QShortcut::activated, this, &CommandSearchBox::focusBox);
    m_ribbon = win->ribbon();
    if (m_ribbon) m_ribbon->setSearchWidget(this);
}

void CommandSearchBox::setWide(bool wide)
{
    setProperty("jpWide", wide);
    if (m_ribbon) m_ribbon->updateSearchWidget();   // (not through the window: it may be going away)
}

void CommandSearchBox::ensureIndex()
{
    if (m_indexed) return;
    cmdsearch::Words words;
    if (QFile f(m_resource); f.open(QIODevice::ReadOnly)) words = cmdsearch::parseWords(f.readAll());
    m_all = cmdsearch::collect(m_win, words);
    m_indexed = true;
}

void CommandSearchBox::focusBox()
{
    // Not while the File screen covers the window, or the ribbon is put away (the box is not there to type in).
    if (!m_ribbon || !m_ribbon->isVisible()) return;
    if (FileScreen *fs = m_win->fileScreen(); fs && fs->isVisible()) return;
    m_indexed = false;
    setWide(true);   // (a box with no room at rest shows now)
    setFocus(Qt::ShortcutFocusReason);
    selectAll();
    refill();
}

QStringList CommandSearchBox::recent() const { return Settings::get().value(recentKey()).toStringList().mid(0, kRecent); }

QList<cmdsearch::Entry> CommandSearchBox::shownResults() const
{
    QList<cmdsearch::Entry> out;
    for (const Row &r : m_rows)
        if (!r.help) out << r.entry;
    return out;
}

void CommandSearchBox::refill()
{
    ensureIndex();
    m_list->clear();
    m_rows.clear();
    const QString q = text().simplified();
    auto addRow = [this](const cmdsearch::Entry &e, bool help) {
        const bool on = help ? true : cmdsearch::available(e, m_win);
        QString name = e.name;
        auto *it = new QListWidgetItem(e.icon, e.where.isEmpty() ? name : name + QLatin1Char('\n') + e.where, m_list);
        QFont f = font();
        it->setFont(f);
        it->setData(Qt::UserRole, int(m_rows.size()));
        it->setData(Qt::AccessibleTextRole, on ? (e.where.isEmpty() ? name : tr("%1, in %2").arg(name, e.where)) : tr("%1, not available now").arg(name));
        it->setSizeHint(QSize(0, e.where.isEmpty() ? fontMetrics().height() + 14 : 2 * fontMetrics().height() + 12));
        if (!on) it->setFlags(it->flags() & ~Qt::ItemIsEnabled);
        m_rows << Row{e, help};
    };
    auto heading = [this](const QString &s) {
        auto *it = new QListWidgetItem(s, m_list);
        QFont f = m_list->font();
        f.setBold(true);
        it->setFont(f);
        it->setFlags(Qt::NoItemFlags);
        it->setData(Qt::UserRole, -1);
        it->setData(Qt::AccessibleTextRole, s);
    };
    if (q.isEmpty()) {
        QList<cmdsearch::Entry> recents;
        for (const QString &id : recent())
            for (const cmdsearch::Entry &e : std::as_const(m_all))
                if (e.id == id) {
                    recents << e;
                    break;
                }
        if (!recents.isEmpty()) heading(tr("Recent"));
        for (const cmdsearch::Entry &e : std::as_const(recents)) addRow(e, false);
    } else {
        const QList<cmdsearch::Entry> found = cmdsearch::find(m_all, q);
        for (const cmdsearch::Entry &e : found) addRow(e, false);
        if (found.isEmpty()) {
            auto *it = new QListWidgetItem(tr("No command found"), m_list);
            it->setFlags(Qt::NoItemFlags);
            it->setData(Qt::UserRole, -1);
        }
        if (m_win->act(QStringLiteral("help.show")) && !help::topicIds().isEmpty()) {
            cmdsearch::Entry e;
            e.kind = cmdsearch::Kind::Help;
            e.id = QStringLiteral("help");
            e.name = tr("Search Help for %1").arg(q);
            e.icon = icon(QStringLiteral("circle-help"));
            addRow(e, true);
        }
    }
    if (m_list->count() == 0 || (q.isEmpty() && !hasFocus())) {
        m_popup->hide();
        return;
    }
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->flags() & Qt::ItemIsEnabled) {
            m_list->setCurrentRow(i);
            break;
        }
    place();
    m_popup->show();
    m_popup->raise();
}

void CommandSearchBox::place()
{
    int h = 8;
    for (int i = 0; i < m_list->count() && i < 11; ++i) h += m_list->item(i)->sizeHint().height() + 2;
    const QRect box(mapToGlobal(QPoint(0, 0)), size());
    const QRect screenRect = !s_testScreen.isEmpty() ? s_testScreen : screen() ? screen()->availableGeometry() : QRect(0, 0, 4000, 4000);
    m_popup->setGeometry(cmdsearch::popupRect(box, QSize(std::max(width(), 340), h), screenRect));
}

void CommandSearchBox::move(int delta)
{
    int row = m_list->currentRow();
    for (int i = row + delta; i >= 0 && i < m_list->count(); i += delta)
        if (m_list->item(i)->flags() & Qt::ItemIsEnabled) {
            m_list->setCurrentRow(i);
            return;
        }
}

void CommandSearchBox::leave()
{
    m_popup->hide();
    {
        const QSignalBlocker b(this);
        clear();
    }
    setWide(false);
    if (Ribbon *r = m_win->ribbon()) Q_EMIT r->leaveRequested();
}

void CommandSearchBox::runCurrent()
{
    QListWidgetItem *it = m_list->currentItem();
    if (!it || !(it->flags() & Qt::ItemIsEnabled)) return;
    const int row = it->data(Qt::UserRole).toInt();
    if (row < 0 || row >= m_rows.size()) return;
    const Row r = m_rows[row];
    const QString typed = text().simplified();
    leave();
    if (r.help) {
        if (QAction *a = m_win->act(QStringLiteral("help.show"))) a->trigger();
        for (HelpView *h : m_win->findChildren<HelpView *>())
            if (h->isVisible()) {
                h->search(typed);
                h->searchBox()->setFocus();
                break;
            }
        return;
    }
    run(r.entry);
}

void CommandSearchBox::run(const cmdsearch::Entry &e)
{
    // The list was made when the text last changed: what is on it may be off now (a read-only document, a closed tab).
    if (!cmdsearch::available(e, m_win)) return;
    QStringList recents = recent();
    recents.removeAll(e.id);
    recents.prepend(e.id);
    Settings::get().setValue(recentKey(), QStringList(recents.mid(0, kRecent)));
    switch (e.kind) {
    case cmdsearch::Kind::Command:
        if (!e.action) return;
        if (e.opensMenu && e.button && e.button->menu()) {
            QPointer<QToolButton> b = e.button;
            Ribbon *r = m_win->ribbon();
            if (r && e.tab >= 0) r->showTab(r->tabPage(e.tab));
            QTimer::singleShot(0, b, [b] { if (b) b->showMenu(); });   // (a menu waits for its choice: not inside this key)
            return;
        }
        e.action->trigger();
        return;
    case cmdsearch::Kind::File:
        if (FileScreen *fs = m_win->fileScreen()) {
            bool page = false;
            for (const FileScreen::Entry &f : fs->entries())
                if (f.key == e.key) page = f.page;
            if (page) m_win->showFileScreenPage(e.key);
            else fs->activateEntry(e.key);
        }
        return;
    case cmdsearch::Kind::Options: m_win->openOptionsPage(e.key); return;
    case cmdsearch::Kind::Gallery:
        if (auto *g = qobject_cast<Gallery *>(e.gallery.data())) g->activate(e.itemId);
        else if (auto *gb = qobject_cast<GalleryButton *>(e.gallery.data())) gb->activate(e.itemId);
        return;
    case cmdsearch::Kind::Help: return;
    }
}

bool CommandSearchBox::eventFilter(QObject *o, QEvent *ev)
{
    if (o == this) {
        if (ev->type() == QEvent::KeyPress) {
            auto *k = static_cast<QKeyEvent *>(ev);
            switch (k->key()) {
            case Qt::Key_Down: move(1); return true;
            case Qt::Key_Up: move(-1); return true;
            case Qt::Key_Return:
            case Qt::Key_Enter: runCurrent(); return true;
            case Qt::Key_Escape: leave(); return true;
            default: break;
            }
        } else if (ev->type() == QEvent::FocusOut) {
            m_popup->hide();
            setWide(false);
        } else if (ev->type() == QEvent::FocusIn) {
            m_indexed = false;   // (however it came, by the mouse, Tab, or the key: the ribbon may have changed since the last look)
            setWide(true);
            QTimer::singleShot(0, this, [this] { if (hasFocus()) refill(); });
        }
    }
    return QLineEdit::eventFilter(o, ev);
}

} // namespace jp
