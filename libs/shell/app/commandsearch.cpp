#include "app/commandsearch.h"

#include "app/appwindow.h"
#include "app/filescreen.h"
#include "app/help.h"
#include "app/i18n.h"
#include "app/icons.h"
#include "app/plaintext.h"
#include "app/ribbon.h"
#include "app/settings.h"

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

QString plainTip(const QString &tip)
{
    if (!Qt::mightBeRichText(tip)) return tip.simplified();
    return QTextDocumentFragment::fromHtml(tip).toPlainText().simplified();
}

QString nameOf(const QAction *a)
{
    QString s = a->text();
    s.remove(QLatin1Char('&'));
    if (s.endsWith(QLatin1String("..."))) s.chop(3);
    if (s.endsWith(QChar(0x2026))) s.chop(1);
    return s.simplified();
}

QString englishOf(const QString &shown)
{
    QString s = sourceText(shown);
    s.remove(QLatin1Char('&'));
    if (s.endsWith(QLatin1String("..."))) s.chop(3);
    return s.simplified();
}

Entry makeCommand(QAction *a, const QString &where, int tab, QToolButton *button, const Words &words)
{
    Entry e;
    e.name = nameOf(a);
    if (e.name.isEmpty()) return e;
    e.where = where;
    e.tip = plainTip(a->toolTip());
    if (e.tip == e.name) e.tip.clear();
    e.id = QStringLiteral("cmd:") + (a->objectName().isEmpty() ? where + QLatin1Char('/') + e.name : a->objectName());
    e.icon = a->icon();
    e.action = a;
    e.button = button;
    e.tab = tab;
    if (const QString en = englishOf(a->text()); !en.isEmpty() && en != e.name) e.english << en;
    if (const QString en = englishOf(a->toolTip()); !en.isEmpty()) e.english << en;
    e.extra = words.value(QStringLiteral("cmd:") + a->objectName());
    return e;
}

} // namespace

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

int score(const Entry &e, const QString &query)
{
    const QStringList want = tokens(query);
    if (want.isEmpty()) return -1;
    const QStringList name = tokens(e.name), where = tokens(e.where), tip = tokens(e.tip), english = tokens(e.english.join(QLatin1Char(' '))),
                      extra = tokens(e.extra.join(QLatin1Char(' ')));
    int total = 0;
    for (const QString &w : want) {
        const int fits[] = {wordFit(name, w) * 10, wordFit(english, w) * 9, wordFit(extra, w) * 8, wordFit(tip, w) * 3, wordFit(where, w) * 2};
        const int best = *std::max_element(std::begin(fits), std::end(fits));
        if (!best) return -1;
        total += best;
    }
    const QString folded = fold(e.name), q = fold(query).simplified();
    if (folded == q) total += 60;
    else if (folded.startsWith(q)) total += 30;
    else if (folded.contains(q)) total += 15;
    return total;
}

QList<Entry> find(const QList<Entry> &all, const QString &query, int limit)
{
    struct Hit {
        int score, order;
    };
    QList<QPair<Hit, int>> hits;
    for (int i = 0; i < all.size(); ++i)
        if (const int s = score(all[i], query); s >= 0) hits << qMakePair(Hit{s, i}, i);
    std::stable_sort(hits.begin(), hits.end(), [](const auto &a, const auto &b) { return a.first.score > b.first.score; });
    QList<Entry> out;
    for (const auto &h : hits) {
        if (out.size() >= limit) break;
        out << all[h.second];
    }
    return out;
}

namespace {

void walkMenu(QMenu *m, const QString &where, int tab, const Words &words, const std::function<void(const Entry &)> &add)
{
    for (QAction *a : m->actions()) {
        if (a->isSeparator() || qobject_cast<QWidgetAction *>(a)) continue;
        if (QMenu *sub = a->menu()) {
            walkMenu(sub, where + QStringLiteral(" > ") + nameOf(a), tab, words, add);
            continue;
        }
        if (!a->isVisible()) continue;
        add(makeCommand(a, where, tab, nullptr, words));
    }
}

} // namespace

QList<Entry> collect(AppWindow *win, const Words &words)
{
    QList<Entry> out;
    QSet<QString> seen;
    auto add = [&](const Entry &e) {
        if (e.name.isEmpty() || seen.contains(e.id)) return;
        seen.insert(e.id);
        out << e;
    };
    if (Ribbon *r = win ? win->ribbon() : nullptr) {
        for (int i = 0; i < r->tabCount(); ++i) {
            RibbonTab *page = r->tabPage(i);
            if (!page || r->tabUserHidden(i)) continue;
            for (RibbonGroup *g : page->groups()) {
                if (g->userHidden()) continue;
                const QString where = r->tabTitle(i) + QStringLiteral(" > ") + g->title();
                for (QToolButton *b : g->findChildren<QToolButton *>()) {
                    if (b == g->launcher()) continue;
                    QAction *a = b->defaultAction();
                    if (a && !a->isSeparator()) {
                        Entry e = makeCommand(a, where, i, b, words);
                        e.opensMenu = b->menu() && b->popupMode() == QToolButton::InstantPopup;
                        add(e);
                    }
                    if (QMenu *m = b->menu()) {
                        const QString label = a ? nameOf(a) : QString(b->text()).remove(QLatin1Char('&')).simplified();   // (a bare drop-down has the button's text, or none)
                        walkMenu(m, label.isEmpty() ? where : where + QStringLiteral(" > ") + label, i, words, add);
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
            if (const QString en = englishOf(f.label); !en.isEmpty()) e.english << en;
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
    return out;
}

bool available(const Entry &e, AppWindow *win)
{
    switch (e.kind) {
    case Kind::Command: {
        if (!e.action || !e.action->isEnabled() || !e.action->isVisible()) return false;
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

QString CommandSearchBox::recentKey() { return QStringLiteral("cmdsearch/recent"); }

CommandSearchBox::CommandSearchBox(AppWindow *win, const QString &wordsResource) : QLineEdit(), m_win(win), m_resource(wordsResource)
{
    setObjectName(QStringLiteral("jpCommandSearch"));
    setClearButtonEnabled(true);
    setPlaceholderText(tr("Search commands"));
    setAccessibleName(tr("Search commands"));
    setAccessibleDescription(tr("Type what you want to do. Down goes to the commands found, and Enter runs the one chosen. Escape goes back to the document."));
    setToolTip(plainToolTip(tr("Search for a command and run it (Alt+Q)")));
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
    auto *key = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Q), win);
    key->setContext(Qt::WindowShortcut);
    connect(key, &QShortcut::activated, this, &CommandSearchBox::focusBox);
    if (Ribbon *r = win->ribbon()) r->setSearchWidget(this);
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
    m_indexed = false;   // (the ribbon may have changed since: Customize, a tab made late)
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
    const int w = std::max(width(), 340);
    QPoint at = mapToGlobal(QPoint(width() - w, height() + 2));
    if (const QScreen *s = screen()) {
        const QRect g = s->availableGeometry();
        at.setX(std::clamp(at.x(), g.left(), std::max(g.left(), g.right() - w)));
        h = std::min(h, std::max(80, g.bottom() - at.y()));
    }
    m_popup->setGeometry(QRect(at, QSize(w, h)));
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
        } else if (ev->type() == QEvent::FocusIn) {
            QTimer::singleShot(0, this, [this] { if (hasFocus()) refill(); });
        }
    }
    return QLineEdit::eventFilter(o, ev);
}

} // namespace jp
