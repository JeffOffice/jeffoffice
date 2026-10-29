#include "appchecks.h"

#include "app/filedialogs.h"
#include "app/help.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "io/zip.h"

#include <QAbstractButton>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QCryptographicHash>
#include <QClipboard>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabBar>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <cstring>
#include <memory>

namespace appchecks {

namespace {

Watch *g_watch = nullptr;
QtMessageHandler g_previous = nullptr;

void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (g_watch && (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg)) {
        // The offscreen display can't size native windows, and a machine without the fonts says so once; neither is a fault of the program.
        const bool harmless = message.contains(QLatin1String("This plugin does not support")) || message.contains(QLatin1String("Populating font family aliases"));
        if (!harmless) g_watch->warnings << message;
        if (harmless) return;
    }
    if (g_previous) g_previous(type, context, message);
}

} // namespace

Watch::Watch()
{
    g_watch = this;
    g_previous = qInstallMessageHandler(handleMessage);
    qApp->installEventFilter(this);
    jp::help::openUrl = [this](const QUrl &url) {
        urls << url.toString();
        return true;
    };
    // Save As and every export write to a new file of the kind the dialog asked for, in a folder of the test's.
    jp::setSavePathStandIn([this](const QString &, const QString &suggested, const QString &filter) {
        static int n = 0;
        QString ending = QFileInfo(suggested).suffix();
        if (ending.isEmpty()) {
            const auto m = QRegularExpression(QStringLiteral("\\*\\.([A-Za-z0-9]+)")).match(filter);
            ending = m.hasMatch() ? m.captured(1) : QStringLiteral("out");
        }
        const QString path = tempFile(QStringLiteral("saved-%1.%2").arg(++n).arg(ending));
        saved << path;
        return path;
    });
    // (the program's own addresses go to the system's browser; here they are only kept)
    for (const char *scheme : {"http", "https", "mailto", "file"})
        QDesktopServices::setUrlHandler(QString::fromLatin1(scheme), this, "urlOpened");
}

Watch::~Watch()
{
    qApp->removeEventFilter(this);
    jp::setSavePathStandIn({});
    for (const char *scheme : {"http", "https", "mailto", "file"}) QDesktopServices::unsetUrlHandler(QString::fromLatin1(scheme));
    jp::help::openUrl = [](const QUrl &u) { return QDesktopServices::openUrl(u); };
    qInstallMessageHandler(g_previous);
    g_watch = nullptr;
}

void Watch::urlOpened(const QUrl &url) { urls << url.toString(); }

void Watch::begin()
{
    dialogs.clear();
    warnings.clear();
    urls.clear();
    saved.clear();
    said.clear();
    told.clear();
}

bool Watch::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() != QEvent::Show || !o->isWidgetType()) return false;
    auto *w = static_cast<QWidget *>(o);
    if (!w->isWindow()) return false;
    auto *dialog = qobject_cast<QDialog *>(w);
    auto *menu = qobject_cast<QMenu *>(w);
    if (!dialog && !menu && !w->isModal()) return false;
    QString words;
    if (auto *box = qobject_cast<QMessageBox *>(w)) words = box->text() + QLatin1Char(' ') + box->informativeText();
    else
        for (const QLabel *label : w->findChildren<QLabel *>()) words += label->text() + QLatin1Char(' ');
    dialogs << QString::fromLatin1(w->metaObject()->className()) + QLatin1String(": ") + words.simplified().left(200);
    said << words;
    // (closed once the call that opened it is waiting on it: a person would have answered by then)
    QPointer<QWidget> guard(w);
    QTimer::singleShot(0, this, [guard] {
        if (!guard) return;
        // Closed as Esc closes it: a message box answers with its button for "no" or "cancel" (it has no result of its own to give).
        if (auto *box = qobject_cast<QMessageBox *>(guard.data())) {
            QAbstractButton *no = box->escapeButton();
            for (QAbstractButton *b : box->buttons())
                if (!no && b->isEnabled() && (box->buttonRole(b) == QMessageBox::RejectRole || box->buttonRole(b) == QMessageBox::NoRole)) no = b;
            if (!no && box->buttons().size() == 1) no = box->buttons().first();
            if (no) no->click();
            else box->done(QDialog::Rejected);
        } else if (auto *d = qobject_cast<QDialog *>(guard.data()))
            d->done(QDialog::Rejected);
        else
            guard->close();
    });
    return false;
}

int Watch::closeNewWindows(const QList<QWidget *> &keep)
{
    int n = 0;
    for (QWidget *top : QApplication::topLevelWidgets()) {
        if (keep.contains(top) || !top->isVisible()) continue;
        ++n;
        top->close();
        // (a window made for one call is deleted when it closes; one that is not is hidden, and its owner deletes it)
        if (top->isVisible()) top->hide();
    }
    return n;
}

bool saysNotYet(const QString &text)
{
    static const QRegularExpression re(QStringLiteral("still being opened|not (?:available )?yet|isn't (?:available )?yet|can't (?:[a-z ]+ )?yet|cannot (?:[a-z ]+ )?yet|coming soon|not implemented|when it is open"),
                                       QRegularExpression::CaseInsensitiveOption);
    return re.match(text).hasMatch();
}

// ---- what a person can see

QString fingerprint(jp::AppWindow *w, const QAction *skip)
{
    QString out;
    out.reserve(1 << 16);
    out += w->windowTitle() + QLatin1Char('\n');
    out += QString::number(w->isVisible()) + QLatin1Char('\n');
    for (const QWidget *c : w->findChildren<QWidget *>()) {
        // The ribbon's own buttons show what an action does; the action is looked at below.
        out += QString::number(c->isVisibleTo(w)) + QString::number(c->isEnabled());
        if (const auto *label = qobject_cast<const QLabel *>(c)) out += label->text();
        else if (const auto *button = qobject_cast<const QAbstractButton *>(c)) out += button->text();
        else if (const auto *edit = qobject_cast<const QLineEdit *>(c)) out += edit->text();
        else if (const auto *combo = qobject_cast<const QComboBox *>(c)) out += combo->currentText();
        else if (const auto *spin = qobject_cast<const QAbstractSpinBox *>(c)) out += spin->text();
        else if (const auto *slider = qobject_cast<const QAbstractSlider *>(c)) out += QString::number(slider->value());
        else if (const auto *tabs = qobject_cast<const QTabBar *>(c)) out += QString::number(tabs->currentIndex());
        else if (const auto *stack = qobject_cast<const QStackedWidget *>(c)) out += QString::number(stack->currentIndex());
        else if (const auto *text = qobject_cast<const QTextEdit *>(c)) out += text->toPlainText();
        out += QLatin1Char('|');
    }
    if (const QStatusBar *bar = w->statusBar()) out += bar->currentMessage();
    // What the commands are now (enabled, shown checked, named): except the one being run, whose own check mark is its answer.
    for (const QString &id : w->actionIds()) {
        const QAction *a = w->act(id);
        if (!a || a == skip) continue;
        out += id + (a->isEnabled() ? QLatin1Char('e') : QLatin1Char('d')) + (a->isChecked() ? QLatin1Char('c') : QLatin1Char('u')) + a->text() + QLatin1Char('|');
    }
    return out;
}

QImage documentPixels(Adapter &app, jp::AppWindow *w)
{
    QWidget *view = app.documentView ? app.documentView(w) : nullptr;
    return pixels(view ? view : w);
}

QImage pixels(QWidget *w) { return w->grab().toImage().convertToFormat(QImage::Format_ARGB32); }

bool differByMoreThanACaret(const QImage &a, const QImage &b)
{
    if (a.size() != b.size()) return true;
    QRect box;
    for (int y = 0; y < a.height(); ++y) {
        const auto *p = reinterpret_cast<const QRgb *>(a.constScanLine(y));
        const auto *q = reinterpret_cast<const QRgb *>(b.constScanLine(y));
        if (std::memcmp(p, q, size_t(a.width()) * sizeof(QRgb)) == 0) continue;
        for (int x = 0; x < a.width(); ++x)
            if (p[x] != q[x]) box = box.united(QRect(x, y, 1, 1));
    }
    return !box.isNull() && (box.width() > 3 || box.height() > 48);
}

bool waitFor(const std::function<bool()> &done, int ceiling) { return QTest::qWaitFor(done, ceiling); }

void drain()
{
    for (int i = 0; i < 20; ++i) {
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
}

QString zipDigest(const QByteArray &zip)
{
    QMap<QString, QByteArray> parts;
    if (!jp::readZip(zip, parts)) return QStringLiteral("(not a zip)");
    QCryptographicHash h(QCryptographicHash::Md5);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        if (it.key().startsWith(QLatin1String("docProps/"))) continue;
        h.addData(it.key().toUtf8());
        h.addData(it.value());
    }
    return QString::fromLatin1(h.result().toHex());
}

void typeInto(QWidget *target, const QString &text)
{
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n')) QTest::keyClick(target, Qt::Key_Return);
        else QTest::keyClick(target, ch.toLatin1() ? ch.toLatin1() : '?');
    }
}

bool listsRecentFile(QWidget *screen, const QString &path)
{
    for (const QListWidget *list : screen->findChildren<QListWidget *>())
        for (int i = 0; i < list->count(); ++i)
            if (list->item(i)->data(Qt::UserRole).toString() == path) return true;
    return false;
}

QString tempFile(const QString &name)
{
    static QTemporaryDir dir;
    return dir.filePath(name);
}

void isolateSettings()
{
    static QTemporaryDir settings;
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
    QStandardPaths::setTestModeEnabled(true);
}

CloseResult closeWindow(jp::AppWindow *w, Watch &watch)
{
    CloseResult r;
    QPointer<jp::AppWindow> guard(w);
    watch.begin();
    w->close();
    drain();
    r.asked = int(watch.dialogs.size());
    r.closed = !guard || !guard->isVisible();
    return r;
}

std::unique_ptr<jp::AppWindow> Pool::take()
{
    if (m_ready.empty()) {
        QElapsedTimer clock;
        clock.start();
        for (int i = 0; i < m_batch; ++i)
            m_ready.emplace_back(m_fromFile ? m_app.launchWith(m_app.sampleFile) : m_app.launch());
        for (auto &w : m_ready)
            if (w) m_app.settle(w.get());
        if (qEnvironmentVariableIsSet("JP_CHECKS_TIMING")) std::fprintf(stderr, "timing: %d windows made and settled in %lld ms\n", m_batch, clock.elapsed());
    }
    std::unique_ptr<jp::AppWindow> w = std::move(m_ready.front());
    m_ready.pop_front();
    return w;
}

// ---- check 2

QList<Standard> standardCommands()
{
    auto keys = [](QKeySequence::StandardKey k) { return QKeySequence::keyBindings(k); };
    // (Save As is F12 in the apps that follow the other programs, and the system's own shortcut in the others: either is the usual one)
    QList<QKeySequence> saveAs = keys(QKeySequence::SaveAs);
    saveAs.prepend(QKeySequence(Qt::Key_F12));
    const QList<QKeySequence> redo = keys(QKeySequence::Redo);
    return {
        {QStringLiteral("file.new"), QStringLiteral("File > New"), keys(QKeySequence::New), QStringLiteral("New")},
        {QStringLiteral("file.open"), QStringLiteral("File > Open"), keys(QKeySequence::Open), QStringLiteral("Open")},
        {QStringLiteral("file.save"), QStringLiteral("File > Save"), keys(QKeySequence::Save), QStringLiteral("Save")},
        {QStringLiteral("file.saveAs"), QStringLiteral("File > Save As"), saveAs, QStringLiteral("Save As")},
        {QStringLiteral("file.print"), QStringLiteral("File > Print"), keys(QKeySequence::Print), QStringLiteral("Print")},
        {QStringLiteral("file.close"), QStringLiteral("File > Close"), keys(QKeySequence::Close), QStringLiteral("Close")},
        {QStringLiteral("edit.undo"), QStringLiteral("Edit > Undo"), keys(QKeySequence::Undo), QString()},
        {QStringLiteral("edit.redo"), QStringLiteral("Edit > Redo"), redo, QString()},
    };
}

namespace {

QString plain(QString s)
{
    s.remove(QLatin1Char('&'));
    s.remove(QChar(0x2026));
    s.remove(QLatin1String("..."));
    return s.simplified();
}

} // namespace

QString reachableFrom(jp::AppWindow *w, QAction *a, const QString &label)
{
    // The ribbon: a button of the command, or a menu that lists it.
    if (jp::Ribbon *r = w->ribbon()) {
        for (const QToolButton *b : r->findChildren<QToolButton *>())
            if (b->defaultAction() == a) return QStringLiteral("ribbon button");
        for (const QMenu *m : r->findChildren<QMenu *>())
            if (m->actions().contains(a)) return QStringLiteral("ribbon menu");
    }
    for (const QMenu *m : w->findChildren<QMenu *>())
        if (m->actions().contains(a)) return QStringLiteral("menu");
    if (!label.isEmpty()) {
        // The File screen (each app opens it its own way: the command for Print and the File button show the same one).
        for (const QAbstractButton *b : w->findChildren<QAbstractButton *>())
            if (plain(b->text()) == label && b->objectName() != QLatin1String("jpBack")) return QStringLiteral("File screen");
    }
    return {};
}

QStringList checkStandardCommands(Adapter &app, const QHash<QString, QList<QKeySequence>> &keysOf, const QHash<QString, QString> &labelOf)
{
    QStringList fails;
    Watch watch;
    Pool pool(app, false, 16);
    for (const Standard &s : standardCommands()) {
        auto fail = [&](const QString &what) { fails << app.name + QLatin1String(": ") + s.id + QLatin1String(" (") + s.what + QLatin1String(") ") + what; };
        std::unique_ptr<jp::AppWindow> w = pool.take();
        if (app.listen) app.listen(w.get(), &watch);
        QAction *a = w->act(s.id);
        if (!a) {
            fail(QStringLiteral("is not a command of the window"));
            continue;
        }
        // The usual shortcut.
        const QList<QKeySequence> want = keysOf.value(s.id, s.keys);
        QKeySequence key;
        for (const QKeySequence &k : a->shortcuts())
            if (want.contains(k)) key = k;
        if (key.isEmpty()) {
            QStringList have, wanted;
            for (const QKeySequence &k : a->shortcuts()) have << k.toString(QKeySequence::PortableText);
            for (const QKeySequence &k : want) wanted << k.toString(QKeySequence::PortableText);
            fail(QStringLiteral("has the shortcut [") + have.join(QLatin1String(", ")) + QLatin1String("], not one of [") + wanted.join(QLatin1String(", ")) + QLatin1Char(']'));
        }
        // On the ribbon or the File screen.
        const QString label = labelOf.contains(s.id) ? labelOf.value(s.id) : s.label;
        if (reachableFrom(w.get(), a, label).isEmpty()) fail(QStringLiteral("can't be reached from the ribbon or the File screen"));
        // It does something in a fresh document (Undo and Redo, once there is something to take back).
        const bool history = s.id == QLatin1String("edit.undo") || s.id == QLatin1String("edit.redo");
        if (history) {
            app.type(w.get());
            if (s.id == QLatin1String("edit.redo")) {
                QAction *undo = w->act(QStringLiteral("edit.undo"));
                if (!waitFor([&] { return undo->isEnabled(); })) {
                    fail(QStringLiteral("can't be tried: Undo stays dimmed after typing"));
                    continue;
                }
                undo->trigger();
                drain();
            }
            if (!waitFor([&] { return a->isEnabled(); })) {
                fail(QStringLiteral("stays dimmed after typing"));
                continue;
            }
        } else if (!a->isEnabled()) {
            fail(QStringLiteral("is dimmed in a fresh document"));
            continue;
        }
        const Outcome o = observe(app, w.get(), a, watch, [a] { a->trigger(); });
        if (!o.changed) fail(QStringLiteral("did nothing a person could see"));
        if (!o.warnings.isEmpty()) fail(QStringLiteral("made Qt warn: ") + o.warnings.join(QLatin1String(" / ")).left(300));
        if (saysNotYet(o.words)) fail(QStringLiteral("said it can't yet: ") + o.words.simplified().left(200));
        // And its shortcut reaches it: pressed in a window of its own, the same thing happens.
        if (!key.isEmpty()) {
            std::unique_ptr<jp::AppWindow> k = pool.take();
            if (app.listen) app.listen(k.get(), &watch);
            QAction *b = k->act(s.id);
            if (history) {
                app.type(k.get());
                if (s.id == QLatin1String("edit.redo")) {
                    QAction *undo = k->act(QStringLiteral("edit.undo"));
                    if (waitFor([&] { return undo->isEnabled(); })) undo->trigger();
                }
                waitFor([&] { return b->isEnabled(); });
            }
            k->activateWindow();
            if (!QTest::qWaitForWindowActive(k.get(), 5000)) {
                fail(QStringLiteral("can't be tried by its shortcut: the window did not become the active one"));
                continue;
            }
            const Outcome p = observe(app, k.get(), b, watch, [&] { QTest::keySequence(k.get(), key); });
            if (!p.changed) fail(QStringLiteral("does nothing when its shortcut ") + key.toString(QKeySequence::PortableText) + QLatin1String(" is pressed"));
            k->hide();
        }
        w->hide();
    }
    return fails;
}

// ---- check 3

namespace {

// Where two texts first differ (for the report of what a command changed).
QString firstDifference(const QString &a, const QString &b)
{
    int i = 0;
    while (i < a.size() && i < b.size() && a[i] == b[i]) ++i;
    const int from = std::max(0, i - 40);
    return QStringLiteral("... %1 => %2").arg(a.mid(from, 80).simplified(), b.mid(from, 80).simplified());
}

} // namespace

// What the clipboard holds, as text (a copy or a cut is seen by its change).
static QString clipboardState()
{
    const QMimeData *data = QGuiApplication::clipboard()->mimeData();
    if (!data) return {};
    QString s = data->formats().join(QLatin1Char(',')) + QLatin1Char('|') + data->text();
    if (data->hasHtml()) s += data->html().left(2000);
    return s;
}

Outcome observe(Adapter &app, jp::AppWindow *w, const QAction *skip, Watch &watch, const std::function<void()> &doIt, bool quiet, int ceiling)
{
    Outcome out;
    QElapsedTimer clock;
    clock.start();
    const QList<QWidget *> before = QApplication::topLevelWidgets();
    const QString screen = fingerprint(w, skip);
    const qint64 tScreen = clock.restart();
    const QString doc = app.docState(w);
    const qint64 tDoc = clock.restart();
    const QImage look = documentPixels(app, w);
    const QString clip = clipboardState();
    const qint64 tLook = clock.restart();
    watch.begin();
    doIt();
    drain();
    const qint64 tDo = clock.restart();
    if (qEnvironmentVariableIsSet("JP_CHECKS_TIMING")) std::fprintf(stderr, "timing: fingerprint %lld ms, document %lld ms, pixels %lld ms, run %lld ms\n", tScreen, tDoc, tLook, tDo);
    QPointer<jp::AppWindow> guard(w);
    // A person sees: a dialog or menu, an address sent to the browser, a message, a window, the window itself going, or a change of what it shows or holds.
    auto why = [&]() -> QString {
        if (!watch.dialogs.isEmpty()) return QStringLiteral("opened a dialog");
        if (!watch.urls.isEmpty()) return QStringLiteral("opened an address");
        if (!watch.saved.isEmpty()) return QStringLiteral("asked where to save a file");
        if (!watch.told.isEmpty()) return QStringLiteral("told something");
        for (QWidget *top : QApplication::topLevelWidgets())
            if (!before.contains(top) && top->isVisible()) return QStringLiteral("opened a window");
        if (!guard || !guard->isVisible()) return QStringLiteral("closed the window");
        if (clipboardState() != clip) return QStringLiteral("changed the clipboard");
        if (const QString now = app.docState(guard.data()); now != doc) {
            out.detail = firstDifference(doc, now);
            return QStringLiteral("changed the document");
        }
        if (const QString now = fingerprint(guard.data(), skip); now != screen) {
            out.detail = firstDifference(screen, now);
            return QStringLiteral("changed what the window shows");
        }
        if (const QImage now = documentPixels(app, guard.data()); differByMoreThanACaret(look, now)) {
            if (const QByteArray dir = qgetenv("JP_CHECKS_SAVE"); !dir.isEmpty()) {
                static int n = 0;
                look.save(QString::fromLocal8Bit(dir) + QStringLiteral("/%1-before.png").arg(++n));
                now.save(QString::fromLocal8Bit(dir) + QStringLiteral("/%1-after.png").arg(n));
                out.detail = QStringLiteral("pictures %1").arg(n);
            }
            return QStringLiteral("redrew the window");
        }
        return {};
    };
    out.how = why();
    // (JP_CHECKS_CEILING shortens the wait while the checks are being written; the tests run with the default)
    if (const int shorter = qEnvironmentVariableIntValue("JP_CHECKS_CEILING"); shorter > 0) ceiling = shorter;
    if (out.how.isEmpty() && !quiet) {
        waitFor([&] { return !(out.how = why()).isEmpty(); }, ceiling);
    }
    out.changed = !out.how.isEmpty();
    out.words = watch.said.join(QLatin1Char(' ')) + QLatin1Char(' ') + watch.told.join(QLatin1Char(' '));
    if (guard) {
        out.words += QLatin1Char(' ') + guard->statusBar()->currentMessage();
        for (const QLabel *label : guard->statusBar()->findChildren<QLabel *>()) out.words += QLatin1Char(' ') + label->text();
    }
    out.warnings = watch.warnings;
    out.dialogs = watch.dialogs;
    out.windows = watch.closeNewWindows(before);
    drain();
    return out;
}

// Why a command is allowed to do nothing a test can see in this state ("" if it is not): the key of the list is its id, or a start of ids ending in "*", with "@blank" or "@file" for one state.
QString quietBecause(const Adapter &app, const QString &id, bool fromFile, bool *wildcard)
{
    const QString state = fromFile ? QStringLiteral("file") : QStringLiteral("blank");
    for (auto it = app.quiet.cbegin(); it != app.quiet.cend(); ++it) {
        QString key = it.key();
        const int at = key.indexOf(QLatin1Char('@'));
        if (at >= 0) {
            if (key.mid(at + 1) != state) continue;
            key.truncate(at);
        }
        if (key == id) return it.value();
        if (key.endsWith(QLatin1Char('*')) && id.startsWith(key.chopped(1))) {
            if (wildcard) *wildcard = true;
            return it.value();
        }
    }
    return {};
}

QStringList runEveryAction(Adapter &app, bool fromFile, QStringList *report)
{
    QStringList fails;
    Watch watch;
    QStringList ids;
    {
        std::unique_ptr<jp::AppWindow> probe(app.launch());
        app.settle(probe.get());
        ids = probe->actionIds();
    }
    const QString state = fromFile ? QStringLiteral("opened from a file") : QStringLiteral("blank");
    auto fail = [&](const QString &id, const QString &what) { fails << app.name + QLatin1String(" (") + state + QLatin1String(") ") + id + QLatin1String(": ") + what; };
    Pool pool(app, fromFile);
    std::unique_ptr<jp::AppWindow> w;
    // A window is used again while nothing was done in it; put back (when the app can) after a command was run in it; and made new when it was closed, opened others, or has been used enough.
    bool fresh = true, dirty = false;
    int usedAgain = 0;
    auto ready = [&]() -> bool {
        if (w && !fresh && !dirty) return true;
        if (w && !fresh && app.reset && usedAgain < 40) {
            QElapsedTimer clock;
            clock.start();
            app.reset(w.get(), fromFile);
            const qint64 tReset = clock.restart();
            app.settle(w.get());
            if (qEnvironmentVariableIsSet("JP_CHECKS_TIMING")) std::fprintf(stderr, "timing: reset %lld ms, settle %lld ms\n", tReset, clock.elapsed());
            ++usedAgain;
            // (JP_CHECKS_RESIDUE says what a window put back still shows of what was done in it, against a window made new)
            if (qEnvironmentVariableIsSet("JP_CHECKS_RESIDUE")) {
                static QString baseline;
                if (baseline.isEmpty()) {
                    std::unique_ptr<jp::AppWindow> made = pool.take();
                    baseline = made ? fingerprint(made.get()) : QString();
                }
                const QString now = fingerprint(w.get());
                if (now != baseline) std::fprintf(stderr, "residue: %s\n", qPrintable(firstDifference(baseline, now)));
            }
        } else {
            w.reset();
            drain();
            w = pool.take();
            usedAgain = 0;
        }
        fresh = dirty = false;
        return bool(w);
    };
    // The places a person can be in, besides the window as launched (a title selected, words selected...): the commands each makes available.
    QList<const Adapter::Context *> contexts;
    for (const Adapter::Context &c : app.contexts)
        if (!c.fileOnly || fromFile) contexts << &c;
    QVector<QSet<QString>> enabledIn;
    for (const Adapter::Context *c : contexts) {
        QSet<QString> on;
        if (ready()) {
            c->enter(w.get(), fromFile);
            app.settle(w.get());
            for (const QString &id : ids)
                if (QAction *a = w->act(id); a && a->isEnabled()) on.insert(id);
        }
        enabledIn << on;
        dirty = true;
    }
    int enabled = 0;
    // (JP_CHECKS_ONLY names the commands to run, by their starts and commas, while the checks are being written)
    const QStringList only = QString::fromLocal8Bit(qgetenv("JP_CHECKS_ONLY")).split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString &id : ids) {
        if (!only.isEmpty() && std::none_of(only.begin(), only.end(), [&](const QString &o) { return id.startsWith(o); })) continue;
        if (app.skipped.contains(id)) {
            if (report) *report << id + QLatin1String(": skipped (") + app.skipped.value(id) + QLatin1Char(')');
            continue;
        }
        if (qEnvironmentVariableIsSet("JP_TEST_TRACE")) {
            std::fprintf(stderr, "command %s (%s)\n", qPrintable(id), qPrintable(state));
            std::fflush(stderr);
        }
        bool wildcard = false;
        const bool quiet = !quietBecause(app, id, fromFile, &wildcard).isEmpty();
        // The window as launched first, then each place that has the command (until it does something).
        bool wasEnabled = false, did = false;
        Outcome last;
        QString where;
        for (int k = -1; k < int(contexts.size()) && !did; ++k) {
            if (k >= 0 && !enabledIn[k].contains(id)) continue;
            if (!ready()) {
                fail(id, QStringLiteral("no window to run it in"));
                break;
            }
            QAction *a = w->act(id);
            if (!a) {
                fail(id, QStringLiteral("the window lists it but has no such command"));
                fresh = true;
                break;
            }
            if (app.listen) app.listen(w.get(), &watch);
            if (app.prepare) app.prepare(w.get(), id, fromFile);
            if (k >= 0) {
                contexts[k]->enter(w.get(), fromFile);
                dirty = true;
            }
            app.settle(w.get());
            if (!a->isEnabled()) continue;
            dirty = true;
            wasEnabled = true;
            const bool lastTry = k + 1 >= int(contexts.size()) || std::none_of(enabledIn.begin() + (k + 1), enabledIn.end(), [&](const QSet<QString> &on) { return on.contains(id); });
            const QString at = k < 0 ? QString() : QLatin1String(" (") + contexts[k]->name + QLatin1Char(')');
            // (a wait for something to change is made only in the last place it can be tried, or the command is not expected to change anything)
            const Outcome o = observe(app, w.get(), a, watch, [a] { a->trigger(); }, quiet || !lastTry);
            last = o;
            where = at;
            if (!o.warnings.isEmpty()) fail(id, QStringLiteral("Qt warned") + at + QLatin1String(": ") + o.warnings.join(QLatin1String(" / ")).left(300));
            if (saysNotYet(o.words)) fail(id, QStringLiteral("said it can't yet") + at + QLatin1String(": ") + o.words.simplified().left(200));
            did = o.changed && o.how != QLatin1String("redrew the window");
            // A window that was closed, or that opened others, is not used again.
            if (!w->isVisible() || o.windows > 0) fresh = true;
            if (o.changed && !did && lastTry) did = true;   // (a redraw is all there is to see, and it is the last place to look)
            if (did && quiet && !wildcard && o.how != QLatin1String("redrew the window"))
                fail(id, QStringLiteral("is listed as doing nothing visible, but it ") + o.how + at + QLatin1String(": take it off the list of quiet commands"));
        }
        if (!wasEnabled) {
            if (report) *report << id + QLatin1String(": dimmed");
            continue;
        }
        ++enabled;
        if (!did && !quiet) fail(id, QStringLiteral("is enabled and did nothing a person could see"));
        if (report) *report << id + QLatin1String(": ") + (did ? last.how : QStringLiteral("did nothing visible")) + where + (last.detail.isEmpty() ? QString() : QLatin1String("   [") + last.detail + QLatin1Char(']'));
    }
    w.reset();
    drain();
    if (report) *report << QStringLiteral("%1 (%2): %3 commands, %4 enabled and run").arg(app.name, state).arg(ids.size()).arg(enabled);
    return fails;
}

} // namespace appchecks
