#include "appchecks.h"

#include "app/filedialogs.h"
#include "app/help.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "io/zip.h"

#include <QAbstractButton>
#include <QAbstractSlider>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QCryptographicHash>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPrintDialog>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabBar>
#include <QTemporaryDir>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <cstring>
#include <memory>

#if defined(__linux__)
#include <csignal>
#include <execinfo.h>
#include <unistd.h>
namespace {
// (a stack for a crash, printed while the checks are being written)
struct CrashStack {
    CrashStack()
    {
        if (!qEnvironmentVariableIsSet("JP_CHECKS_STACK")) return;
        std::signal(SIGSEGV, [](int) {
            void *frames[40];
            const int n = backtrace(frames, 40);
            backtrace_symbols_fd(frames, n, 2);
            _exit(139);
        });
    }
} g_crashStack;
}
#endif

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
    // Save As and every export write to a new file of the kind the dialog asked for, in a folder of the test's; the filter it names is the one `pickFilter` says.
    jp::setSavePathStandIn([this](const QString &, const QString &suggested, const QString &filter, QString *selectedFilter) {
        static int n = 0;
        if (!asked) {
            asked = true;
            beforeWrite = filesNow();
        }
        offered = filter.split(QLatin1String(";;"), Qt::SkipEmptyParts);
        const QString chosen = pickFilter < offered.size() ? offered.at(pickFilter) : QString();
        if (selectedFilter && !chosen.isEmpty()) *selectedFilter = chosen;
        QString ending = chosen.isEmpty() ? QFileInfo(suggested).suffix() : QString();
        if (ending.isEmpty()) {
            const auto m = QRegularExpression(QStringLiteral("\\*\\.([A-Za-z0-9]+)")).match(chosen.isEmpty() ? filter : chosen);
            ending = m.hasMatch() ? m.captured(1) : QStringLiteral("out");
        }
        static const QString run = QString::number(QCoreApplication::applicationPid());
        const QString path = tempFile(QStringLiteral("saved-%1-%2.%3").arg(run).arg(++n).arg(ending));
        saved << path;
        return path;
    });
    // A print dialog is not shown; that it was asked for is what a test sees.
    jp::setPrintDialogStandIn([this](QPrintDialog &) {
        dialogs << QStringLiteral("QPrintDialog: ");
        return int(QDialog::Rejected);
    });
    // (the program's own addresses go to the system's browser; here they are only kept)
    for (const char *scheme : {"http", "https", "mailto", "file"})
        QDesktopServices::setUrlHandler(QString::fromLatin1(scheme), this, "urlOpened");
}

Watch::~Watch()
{
    qApp->removeEventFilter(this);
    jp::setSavePathStandIn({});
    jp::setPrintDialogStandIn({});
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
    beforeWrite.clear();
    asked = false;
    offered.clear();
    notices.clear();
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
    // A message box that only tells (an information, warning, or error box) is a notice; one that asks is a dialog.
    const auto *box = qobject_cast<QMessageBox *>(w);
    if (box && box->buttons().size() <= 1)
        notices << words.simplified();
    else
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
    const QWidget *status = w->statusBar();
    for (const QWidget *c : w->findChildren<QWidget *>()) {
        // (the status bar says what a command made of itself: its words and counts follow the change and are not one)
        if (status && (c == status || status->isAncestorOf(c))) continue;
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
        else if (const auto *plain = qobject_cast<const QPlainTextEdit *>(c)) out += plain->toPlainText();
        else if (const auto *view = qobject_cast<const QAbstractItemView *>(c)) {
            // the row the cursor is on, how many are chosen, and (in a list) how many are folded away
            out += QString::number(view->currentIndex().row()) + QLatin1Char('/') + QString::number(view->selectionModel() ? view->selectionModel()->selectedIndexes().size() : 0);
            if (const auto *list = qobject_cast<const QListView *>(c))
                for (int row = 0, n = view->model() ? view->model()->rowCount() : 0; row < n; ++row)
                    if (list->isRowHidden(row)) out += QLatin1Char('h');
        }
        out += QLatin1Char('|');
    }
    // What the commands are now (enabled, shown checked, named): except the one being run, whose own check mark is its answer.
    for (const QString &id : w->actionIds()) {
        const QAction *a = w->act(id);
        if (!a || a == skip) continue;
        out += id + (a->isEnabled() ? QLatin1Char('e') : QLatin1Char('d')) + (a->isChecked() ? QLatin1Char('c') : QLatin1Char('u')) + a->text() + QLatin1Char('|');
    }
    return out;
}

bool hasDrawnContent(const QPixmap &picture)
{
    const QImage image = picture.toImage();
    if (image.isNull()) return false;
    const QRgb first = image.pixel(0, 0);
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
            if (image.pixel(x, y) != first) return true;
    return false;
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

QPixmap printPreviewOf(Adapter &app, jp::AppWindow *w)
{
    QWidget *screen = app.showPrint(w);
    QPixmap shown;
    waitFor([&] {
        const QLabel *preview = screen ? screen->findChild<QLabel *>(QStringLiteral("printPreview")) : nullptr;
        if (!preview || preview->pixmap().isNull() || !hasDrawnContent(preview->pixmap())) return false;
        shown = preview->pixmap();
        return true;
    });
    return shown;
}

bool previewShowsTheDocument(Adapter &app, jp::AppWindow *w)
{
    const QPixmap shown = printPreviewOf(app, w);
    if (shown.isNull()) return false;
    // (what a blank document of the program prints: a page with its edge and its shadow, which is not the document)
    std::unique_ptr<jp::AppWindow> blank(app.launch());
    app.settle(blank.get());
    const QPixmap empty = printPreviewOf(app, blank.get());
    return empty.isNull() || shown.size() != empty.size() || shown.toImage() != empty.toImage();
}

bool listsRecentFile(QWidget *screen, const QString &path)
{
    for (const QListWidget *list : screen->findChildren<QListWidget *>())
        for (int i = 0; i < list->count(); ++i)
            if (list->item(i)->data(Qt::UserRole).toString() == path) return true;
    return false;
}

QStringList filesNow()
{
    QStringList files;
    for (const QString &root : {QFileInfo(tempFile(QStringLiteral("x"))).absolutePath(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)}) {
        QDirIterator it(root, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            files << it.filePath() + QLatin1Char(':') + QString::number(it.fileInfo().size());
        }
    }
    files.sort();
    return files;
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
    // (what the runs save as templates goes to the program's data folder: taken away again when the program ends)
    qAddPostRoutine([] {
        const QString run = QString::number(QCoreApplication::applicationPid());
        QDir templates(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/Templates"));
        for (const QString &name : templates.entryList({QStringLiteral("saved-%1-*").arg(run)}, QDir::Files)) templates.remove(name);
    });
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
        if (!o.changed) fail(QStringLiteral("did nothing a person could see") + (o.refusal.isEmpty() ? QString() : QLatin1String(", and said: ") + o.refusal.left(160)));
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
            const QString pressed = key.toString(QKeySequence::PortableText);
            if (!p.changed) fail(QStringLiteral("does nothing when its shortcut ") + pressed + QLatin1String(" is pressed"));
            // (the same thing as the command: the same kind of change, the same dialogs, the same windows)
            else if (p.how != o.how || p.windows != o.windows || QStringList(p.dialogs).replaceInStrings(QRegularExpression(QStringLiteral(": .*")), QString()) != QStringList(o.dialogs).replaceInStrings(QRegularExpression(QStringLiteral(": .*")), QString()))
                fail(QStringLiteral("does something else when its shortcut ") + pressed + QLatin1String(" is pressed (") + p.how + QLatin1String(", not ") + o.how + QLatin1Char(')'));
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

// What the clipboard holds, as text (a copy or a cut is seen by its change).
QString clipboardState()
{
    const QMimeData *data = QGuiApplication::clipboard()->mimeData();
    if (!data) return {};
    QString s = data->formats().join(QLatin1Char(',')) + QLatin1Char('|') + data->text();
    if (data->hasHtml()) s += data->html().left(2000);
    return s;
}

} // namespace

Outcome observe(Adapter &app, jp::AppWindow *w, const QAction *skip, Watch &watch, const std::function<void()> &doIt, bool quiet, int ceiling)
{
    Outcome out;
    const QList<QWidget *> before = QApplication::topLevelWidgets();
    const QString screen = fingerprint(w, skip);
    const QString doc = app.docState(w);
    const QString clip = clipboardState();
    const bool wasVisible = w->isVisible();
    const bool checkable = skip && skip->isCheckable();
    const bool markBefore = skip && skip->isChecked();
    const QString statusBefore = w->statusBar() ? w->statusBar()->currentMessage() : QString();
    watch.begin();
    doIt();
    drain();
    QPointer<jp::AppWindow> guard(w);
    // (the window brings its commands up to date a moment after a change: done now, a test need not wait for the moment)
    if (guard && guard->isVisible()) app.settle(guard.data());
    // A person sees: a dialog or menu that asks, an address sent to the browser, a file written, a window, the window itself going, the clipboard, a setting turned over, or a change of what the
    // window holds (the document, the selection, the view) or shows. A notice and a line in the status bar are only what the command said about itself.
    auto why = [&]() -> QString {
        if (!watch.dialogs.isEmpty()) return QStringLiteral("opened a dialog");
        if (!watch.urls.isEmpty()) return QStringLiteral("opened an address");
        if (watch.asked) {
            // A file counts when it is new or longer than it was and has something in it: the kind that was asked for is then read again, where the program can.
            QStringList written;
            for (const QString &f : filesNow()) {
                if (watch.beforeWrite.contains(f)) continue;
                const int colon = f.lastIndexOf(QLatin1Char(':'));
                if (f.mid(colon + 1).toLongLong() > 0) written << f.left(colon);
            }
            if (!written.isEmpty()) {
                out.written = written;
                return QStringLiteral("wrote a file");
            }
        }
        for (QWidget *top : QApplication::topLevelWidgets())
            if (!before.contains(top) && top->isVisible()) return QStringLiteral("opened a window");
        if (wasVisible && (!guard || !guard->isVisible())) return QStringLiteral("closed the window");
        if (clipboardState() != clip) return QStringLiteral("changed the clipboard");
        if (const QString now = app.docState(guard.data()); now != doc) {
            out.detail = firstDifference(doc, now);
            return QStringLiteral("changed the document, the selection, or the view");
        }
        if (const QString now = fingerprint(guard.data(), skip); now != screen) {
            out.detail = firstDifference(screen, now);
            return QStringLiteral("changed what the window shows");
        }
        return {};
    };
    // A command that turns something on or off must turn its mark over, and change something else a person can see: a mark alone is not a change.
    auto changes = [&]() -> QString {
        QString r = why();
        // (a notice that was shown takes the focus and the like with it: those are not what the command did)
        if (r == QLatin1String("changed what the window shows") && !watch.notices.isEmpty()) r.clear();
        // (a dialog that asks, an address, a file, and a window are not about the mark: a choice that is put off by a question has no mark to turn)
        const bool outward = r == QLatin1String("opened a dialog") || r == QLatin1String("opened an address") || r == QLatin1String("wrote a file") || r == QLatin1String("opened a window") || r == QLatin1String("closed the window");
        if (!r.isEmpty() && !outward && checkable && skip->isChecked() == markBefore) return {};
        return r;
    };
    out.how = changes();
    // (JP_CHECKS_CEILING shortens the wait while the checks are being written; the tests run with the default)
    if (const int shorter = qEnvironmentVariableIntValue("JP_CHECKS_CEILING"); shorter > 0) ceiling = shorter;
    if (out.how.isEmpty() && !quiet) waitFor([&] { return !(out.how = changes()).isEmpty(); }, ceiling);
    if (checkable && out.how.isEmpty() && skip->isChecked() != markBefore) out.refusal = QStringLiteral("turned its mark over and nothing else");
    for (const QString &path : out.written)
        if (app.reopen)
            if (const QString why = app.reopen(path); !why.isEmpty()) watch.warnings << QStringLiteral("wrote a file the program can't read again (%1): %2").arg(QFileInfo(path).fileName(), why);
    out.changed = !out.how.isEmpty();
    // What it said: notices, message boxes, the notifier, and the line in the status bar if that is new.
    out.words = watch.said.join(QLatin1Char(' ')) + QLatin1Char(' ') + watch.told.join(QLatin1Char(' '));
    if (guard && guard->statusBar()) {
        const QString now = guard->statusBar()->currentMessage();
        if (now != statusBefore) out.words += QLatin1Char(' ') + now;
    }
    if (!out.changed) {
        out.refusal = out.refusal + QLatin1Char(' ') + (watch.notices + watch.told).join(QLatin1Char(' '));
        if (guard && guard->statusBar() && guard->statusBar()->currentMessage() != statusBefore) out.refusal += QLatin1Char(' ') + guard->statusBar()->currentMessage();
        out.refusal = out.refusal.simplified();
    }
    out.warnings = watch.warnings;
    out.dialogs = watch.dialogs;
    out.offered = watch.offered;
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
    app.allIds = ids;
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
            app.reset(w.get(), fromFile);
            app.settle(w.get());
            ++usedAgain;
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
            if (qEnvironmentVariableIsSet("JP_TEST_TRACE")) {
                std::fprintf(stderr, "context %s (%s)\n", qPrintable(c->name), qPrintable(state));
                std::fflush(stderr);
            }
            c->enter(w.get(), fromFile);
            app.settle(w.get());
            for (const QString &id : ids)
                if (QAction *a = w->act(id); a && a->isEnabled()) on.insert(id);
        }
        enabledIn << on;
        dirty = true;
    }
    int enabled = 0, attempts = 0;
    // The commands that changed something in each place (0 is the window as launched): the ones that set what is set already are tried again after another of their family, there.
    QVector<QSet<QString>> workedIn(int(contexts.size()) + 1);
    struct Deferred {
        QString id, where;
        bool quiet = false;
        Outcome last;
    };
    QList<Deferred> deferred;
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
        const int triesBefore = attempts;
        bool wasEnabled = false, did = false, silent = false;
        Outcome last;
        QString where, said;
        int offered = 0, usedContext = -1;
        // One try: the window put back, the context entered, the command run. False when it was dimmed there.
        auto attempt = [&](int k, int pickFilter, bool waits, const QString &before = QString(), const QKeySequence &key = QKeySequence()) -> bool {
            ++attempts;
            if (!ready()) {
                fail(id, QStringLiteral("no window to run it in"));
                return false;
            }
            QAction *a = w->act(id);
            if (!a) {
                fail(id, QStringLiteral("the window lists it but has no such command"));
                fresh = true;
                return false;
            }
            if (app.listen) app.listen(w.get(), &watch);
            QGuiApplication::clipboard()->clear();   // (a copy is seen by what the clipboard then holds, so it starts empty)
            if (app.prepare) app.prepare(w.get(), id, fromFile);
            if (k >= 0) {
                if (qEnvironmentVariableIsSet("JP_TEST_TRACE")) std::fprintf(stderr, "  in %s\n", qPrintable(contexts[k]->name));
                contexts[k]->enter(w.get(), fromFile);
                dirty = true;
            }
            app.settle(w.get());
            if (!a->isEnabled()) return false;
            // A command that sets something to a value has nothing to do when it is so already: another of its family is run first, which changes it.
            if (!before.isEmpty()) {
                QAction *other = w->act(before);
                if (!other || !other->isEnabled()) return false;
                other->trigger();
                drain();
                watch.closeNewWindows(QApplication::topLevelWidgets());
                app.settle(w.get());
                if (!a->isEnabled()) return false;
            }
            dirty = true;
            wasEnabled = true;
            watch.pickFilter = pickFilter;
            where = k < 0 ? QString() : QLatin1String(" (") + contexts[k]->name + QLatin1Char(')');
            last = observe(app, w.get(), a, watch, [&] {
                if (key.isEmpty()) {
                    a->trigger();
                    return;
                }
                // (the keys of a window go to it when it is the active one)
                w->raise();
                w->activateWindow();
                waitFor([&] { return QApplication::activeWindow() == w.get(); }, 5000);
                drain();
                QTest::keySequence(w.get(), key);
            }, quiet || !waits);
            offered = std::max(offered, int(last.offered.size()));
            if (last.changed) workedIn[k + 1].insert(id);
            if (!last.changed && last.refusal.isEmpty()) silent = true;
            if (!last.warnings.isEmpty()) fail(id, QStringLiteral("Qt warned") + where + QLatin1String(": ") + last.warnings.join(QLatin1String(" / ")).left(300));
            if (saysNotYet(last.words)) said = QStringLiteral("said it can't yet") + where + QLatin1String(": ") + last.words.simplified().left(200);
            if (!w->isVisible() || last.windows > 0) fresh = true;
            return true;
        };
        // The window as launched first, then each place that has the command, until it does something (a refusal sends it on to the next). The places where commands have done something
        // most often come first, and a place the app names for this command comes before them; a command that is listed as doing nothing is run where it is enabled first, and no further.
        QVector<int> order;
        {
            for (int k = 0; k < int(contexts.size()); ++k)
                if (enabledIn[k].contains(id)) order.append(k);
            auto hinted = [&](int k) {
                for (auto it = app.hints.cbegin(); it != app.hints.cend(); ++it)
                    if (id.startsWith(it.key()) && it.value().contains(contexts[k]->name)) return true;
                return false;
            };
            std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
                if (hinted(a) != hinted(b)) return hinted(a);
                return workedIn[a + 1].size() > workedIn[b + 1].size();
            });
        }
        order.prepend(-1);
        for (int at = 0; at < int(order.size()) && !did && !(quiet && wasEnabled); ++at) {
            const int k = order[at];
            const bool lastTry = at + 1 >= int(order.size());
            if (!attempt(k, 0, lastTry)) continue;
            did = last.changed;
            usedContext = k;
            if (did && quiet && !wildcard) fail(id, QStringLiteral("is listed as doing nothing visible, but it ") + last.how + where + QLatin1String(": take it off the list of quiet commands"));
        }
        // The shortcut does what the command does: pressed in a window put in the same place, the same kind of change, the same dialogs, the same windows.
        if (did && !quiet && !app.keySkipped.contains(id)) {
            QAction *shortcutOf = w ? w->act(id) : nullptr;
            const QList<QKeySequence> keys = shortcutOf ? shortcutOf->shortcuts() : QList<QKeySequence>();
            if (!keys.isEmpty()) {
                const Outcome ref = last;
                const QString pressed = keys.first().toString(QKeySequence::PortableText);
                if (attempt(usedContext, 0, true, QString(), keys.first())) {
                    auto kinds = [](const QStringList &dialogs) { return QStringList(dialogs).replaceInStrings(QRegularExpression(QStringLiteral(": .*")), QString()); };
                    if (qEnvironmentVariableIsSet("JP_CHECKS_DEBUG")) std::fprintf(stderr, "key %s of %s in %s (was: %s): changed=%d how=%s dialogs=%s active=%p w=%p enabled=%d\n", qPrintable(pressed), qPrintable(id), usedContext >= 0 ? qPrintable(contexts[usedContext]->name) : "launch", qPrintable(ref.how), int(last.changed), qPrintable(last.how), qPrintable(last.dialogs.join('|')), static_cast<void *>(QApplication::activeWindow()), static_cast<void *>(w.get()), int(w->act(id)->isEnabled()));
                    if (!last.changed) fail(id, QStringLiteral("does nothing when its shortcut %1 is pressed").arg(pressed));
                    else if (last.how != ref.how || last.windows != ref.windows || kinds(last.dialogs) != kinds(ref.dialogs))
                        fail(id, QStringLiteral("does something else when its shortcut %1 is pressed (%2, not %3)").arg(pressed, last.how, ref.how));
                }
                last = ref;
            }
        }
        // A save dialog that offers several kinds of file: the command is run again for each, so each way of saving is tried.
        for (int f = 1; did && f < offered; ++f) {
            const bool ok = attempt(usedContext, f, true);
            if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT") && ok) std::fprintf(stderr, "filter %d of %s: %s [%s] dialogs=%s saved=%s\n", f + 1, qPrintable(id), qPrintable(last.how), qPrintable(last.offered.value(f)), qPrintable(last.dialogs.join('|')), qPrintable(watch.saved.join('|')));
            if (ok && !last.changed) fail(id, QStringLiteral("saves nothing when the dialog's kind of file number %1 is chosen").arg(f + 1));
        }
        if (!wasEnabled) {
            if (report) *report << id + QLatin1String(": dimmed");
            continue;
        }
        app.ran.insert(id);
        ++enabled;
        // Nothing happened, and nothing was said: it may be set to what it sets already. It is tried again after another of its family, once the places where those do something are known.
        if (!did && !quiet && silent) {
            deferred.append({id, where, quiet, last});
            continue;
        }
        if (!did && !quiet) fail(id, QStringLiteral("is enabled and did nothing a person could see") + (last.refusal.isEmpty() ? QString() : QLatin1String(", and said: ") + last.refusal.left(160)));
        if (!did || said.contains(QLatin1String("still being opened"))) {
            if (!said.isEmpty() && !quiet) fail(id, said);
        }
        if (report) *report << id + QLatin1String(": ") + (did ? last.how : last.refusal.isEmpty() ? QStringLiteral("did nothing visible") : QStringLiteral("refused: ") + last.refusal.left(100)) + where + (last.detail.isEmpty() ? QString() : QLatin1String("   [") + last.detail + QLatin1Char(']')) + QStringLiteral("  {%1 tries}").arg(attempts - triesBefore);
    }
    for (const Deferred &item : deferred) {
        const QString &id = item.id;
        const QString family = id.left(std::max(0, int(id.lastIndexOf(QLatin1Char('.')))));
        bool did = false;
        QString how, where;
        for (int k = -1; k < int(contexts.size()) && !did; ++k) {
            if (k >= 0 && !enabledIn[k].contains(id)) continue;
            QStringList others;
            for (const QString &candidate : ids)
                if (candidate != id && !family.isEmpty() && candidate.startsWith(family + QLatin1Char('.')) && candidate.indexOf(QLatin1Char('.'), family.size() + 1) < 0 && workedIn[k + 1].contains(candidate)) others << candidate;
            // (the ones that set the same thing as this are tried first: those that share the longest start of the name)
            std::stable_sort(others.begin(), others.end(), [&](const QString &a, const QString &b) {
                auto common = [&](const QString &x) { int n = 0; while (n < x.size() && n < id.size() && x[n] == id[n]) ++n; return n; };
                return common(a) > common(b);
            });
            // (and one that is a choice, as this one is, comes before one that is not)
            if (QAction *self = w ? w->act(id) : nullptr)
                std::stable_partition(others.begin(), others.end(), [&](const QString &x) { QAction *o = w->act(x); return o && o->isCheckable() == self->isCheckable(); });
            for (const QString &other : others.mid(0, 8)) {
                if (did) break;
                if (!ready()) break;
                QAction *a = w->act(id);
                if (!a) break;
                if (app.listen) app.listen(w.get(), &watch);
                if (app.prepare) app.prepare(w.get(), id, fromFile);
                if (k >= 0) contexts[k]->enter(w.get(), fromFile);
                app.settle(w.get());
                QAction *first = w->act(other);
                dirty = true;
                if (!a->isEnabled() || !first || !first->isEnabled()) continue;
                first->trigger();
                drain();
                if (!w->isVisible()) {
                    fresh = true;
                    continue;
                }
                app.settle(w.get());
                if (!a->isEnabled()) continue;
                const Outcome o = observe(app, w.get(), a, watch, [a] { a->trigger(); }, false);
                if (!o.warnings.isEmpty()) fail(id, QStringLiteral("Qt warned after ") + other + QLatin1String(": ") + o.warnings.join(QLatin1String(" / ")).left(300));
                if (!w->isVisible() || o.windows > 0) fresh = true;
                if (o.changed) {
                    did = true;
                    how = o.how;
                    where = QLatin1String(" after ") + other + (k < 0 ? QString() : QLatin1String(" (") + contexts[k]->name + QLatin1Char(')'));
                }
            }
        }
        if (!did) fail(id, QStringLiteral("is enabled and did nothing a person could see, in any place, or after another of its family"));
        if (report) *report << id + QLatin1String(": ") + (did ? how : QStringLiteral("did nothing visible")) + where;
    }
    w.reset();
    drain();
    if (report) *report << QStringLiteral("%1 (%2): %3 commands, %4 enabled and run").arg(app.name, state).arg(ids.size()).arg(enabled) + QStringLiteral(", %1 tries in %2 places").arg(attempts).arg(contexts.size());
    return fails;
}

QStringList checkNeverRun(Adapter &app, QStringList *report)
{
    QStringList fails, left;
    for (const QString &id : app.allIds)
        if (!app.ran.contains(id) && !app.skipped.contains(id)) left << id;
    left.sort();
    if (report) *report << QStringLiteral("%1: %2 of %3 commands are never run in either state").arg(app.name).arg(left.size()).arg(app.allIds.size());
    for (const QString &id : left) {
        if (!app.neverRun.contains(id)) fails << app.name + QLatin1String(": ") + id + QLatin1String(" is dimmed in every place a test puts the window in, and has no reason listed for it");
        if (report) *report << id + QLatin1String(": ") + app.neverRun.value(id, QStringLiteral("(no reason)"));
    }
    for (auto it = app.neverRun.cbegin(); it != app.neverRun.cend(); ++it)
        if (app.ran.contains(it.key())) fails << app.name + QLatin1String(": ") + it.key() + QLatin1String(" is listed as never run, but it is: take it off the list");
    return fails;
}

} // namespace appchecks
