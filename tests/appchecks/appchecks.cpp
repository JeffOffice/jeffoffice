#include "appchecks.h"
#include "withdialog.h"

#include "app/filedialogs.h"
#include "app/keymap.h"
#include "app/ribboncustom.h"
#include "app/help.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/icons.h"
#include "core/appinfo.h"
#include "app/theme.h"
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
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPrintDialog>
#include <QScrollArea>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QStyle>
#include <QTabWidget>
#include <QTreeWidget>
#include <QStyleHints>
#include <QStyleFactory>
#include <QScrollBar>
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
        // The offscreen display can't size native windows, a machine without the fonts says so once, and one without video drivers or a sound server
        // (a CI machine) says so when a sound is played; none is a fault of the program.
        const bool noMediaDevices = (message.startsWith(QLatin1String("Couldn't load va")) || message.startsWith(QLatin1String("Couldn't resolve va")) || message.startsWith(QLatin1String("PulseAudioService:")));
        const bool harmless = message.contains(QLatin1String("This plugin does not support")) || message.contains(QLatin1String("Populating font family aliases")) || noMediaDevices;
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

QStringList checkTypingWithoutAClick(Adapter &app)
{
    QStringList failures;
    Watch watch;
    const auto typeIn = [&](jp::AppWindow *w, const QString &which) {
        if (!w) {
            failures << which + QStringLiteral(": no window");
            return;
        }
        w->activateWindow();
        if (!QTest::qWaitForWindowActive(w)) failures << which + QStringLiteral(": the window did not become active");
        if (app.settle) app.settle(w);
        QWidget *focus = QApplication::focusWidget();
        if (!focus) {
            failures << which + QStringLiteral(": nothing has the keyboard focus");
            return;
        }
        QString where = QString::fromLatin1(focus->metaObject()->className());
        if (!focus->objectName().isEmpty()) where += QStringLiteral(" \"%1\"").arg(focus->objectName());
        if (auto *button = qobject_cast<QAbstractButton *>(focus)) where += QStringLiteral(" (%1)").arg(button->text());
        const QString state = app.docState ? app.docState(w) : QString();
        const QString before = fingerprint(w);
        watch.begin();
        for (const QChar ch : QStringLiteral("ab c")) {
            QWidget *to = QApplication::focusWidget();   // (the focus may move as the keys come: the first letter of a cell's entry starts its editor)
            if (to) QTest::keyClick(to, ch.toLatin1());
        }
        drain();
        if (app.settle) app.settle(w);
        if (app.typedText) {
            if (!app.typedText(w).contains(QStringLiteral("ab c"))) failures << which + QStringLiteral(": typing with the focus on a %1 did not land in the document (it holds \"%2\")").arg(where, app.typedText(w));
        } else {
            if (!watch.dialogs.isEmpty()) failures << which + QStringLiteral(": typing with the focus on a %1 opened %2").arg(where, watch.dialogs.join(QStringLiteral("; ")));
            if (app.docState && app.docState(w) != state) failures << which + QStringLiteral(": typing with the focus on a %1 changed the document or the view").arg(where);
            if (fingerprint(w) != before) failures << which + QStringLiteral(": typing with the focus on a %1 changed what the window shows (a command, or a screen)").arg(where);
        }
    };
    std::unique_ptr<jp::AppWindow> first(app.launch());
    typeIn(first.get(), app.name + QStringLiteral(" as launched"));
    if (first) {
        const int windows = jp::AppWindow::windows().size();
        const QStringList newCommands = app.newDocument.isEmpty() ? QStringList{QStringLiteral("file.new")} : app.newDocument;
        QAction *fileNew = first->act(newCommands.first());
        if (!fileNew) {
            failures << app.name + QStringLiteral(" has no ") + newCommands.first();
        } else {
            fileNew->trigger();
            drain();
            jp::AppWindow *second = nullptr;
            for (jp::AppWindow *other : jp::AppWindow::windows())
                if (other != first.get()) second = other;
            if (jp::AppWindow::windows().size() != windows + 1 || !second) {
                failures << app.name + QStringLiteral(": ") + newCommands.first() + QStringLiteral(" opened no window");
            } else {
                for (int i = 1; i < newCommands.size(); ++i)
                    if (QAction *step = second->act(newCommands.at(i))) step->trigger();
                drain();
                QPointer<jp::AppWindow> guard(second);
                typeIn(second, app.name + QStringLiteral(" after File > New"));
                if (guard) {
                    second->setAttribute(Qt::WA_DeleteOnClose, false);
                    delete second;
                }
            }
        }
    }
    return failures;
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

// JO_APPCHECK_SHARD="k/n" (k from 1 to n) makes a run check only its part of the commands: the commands are grouped by family (the id up to its last dot, which the
// "after another of its family" step needs together), the families are sorted, and family number i belongs to part i % n + 1. Every part together checks every command once.
bool ownsCommand(const Adapter &app, const QString &id)
{
    static const QString spec = QString::fromLocal8Bit(qgetenv("JO_APPCHECK_SHARD"));
    if (spec.isEmpty()) return true;
    const QStringList kn = spec.split(QLatin1Char('/'));
    bool okK = false, okN = false;
    const int k = kn.value(0).toInt(&okK), n = kn.value(1).toInt(&okN);
    if (!okK || !okN || n < 1 || k < 1 || k > n) return true;
    auto familyOf = [](const QString &x) { return x.left(std::max(0, int(x.lastIndexOf(QLatin1Char('.'))))); };
    static QStringList families;
    if (families.isEmpty()) {
        for (const QString &x : app.allIds) families << familyOf(x);
        families.removeDuplicates();
        families.sort();
    }
    return families.indexOf(familyOf(id)) % n + 1 == k;
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
        if (!ownsCommand(app, id)) continue;
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
                        fail(id, QStringLiteral("does something else when its shortcut %1 is pressed%6 (%2, not %3; key: %4 [windows %7, dialogs %8]; menu: %5 [windows %9, dialogs %10])")
                                 .arg(pressed, last.how, ref.how, last.detail.left(400), ref.detail.left(400), where).arg(last.windows).arg(last.dialogs.join(QLatin1Char('|'))).arg(ref.windows).arg(ref.dialogs.join(QLatin1Char('|'))));
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
        if (ownsCommand(app, id) && !app.ran.contains(id) && !app.skipped.contains(id)) left << id;
    left.sort();
    if (report) *report << QStringLiteral("%1: %2 of %3 commands are never run in either state").arg(app.name).arg(left.size()).arg(app.allIds.size());
    for (const QString &id : left) {
        if (!app.neverRun.contains(id)) fails << app.name + QLatin1String(": ") + id + QLatin1String(" is dimmed in every place a test puts the window in, and has no reason listed for it");
        if (report) *report << id + QLatin1String(": ") + app.neverRun.value(id, QStringLiteral("(no reason)"));
    }
    for (auto it = app.neverRun.cbegin(); it != app.neverRun.cend(); ++it)
        if (ownsCommand(app, it.key()) && app.ran.contains(it.key())) fails << app.name + QLatin1String(": ") + it.key() + QLatin1String(" is listed as never run, but it is: take it off the list");
    return fails;
}

// ---- the ribbon's own layout
namespace {
QString rectText(const QRect &r) { return QStringLiteral("(%1,%2 %3x%4)").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()); }
QString controlName(const QWidget *w)
{
    QString n = QString::fromLatin1(w->metaObject()->className());
    if (const auto *b = qobject_cast<const QAbstractButton *>(w)) n += QStringLiteral(" \"%1\"").arg(b->text());
    else if (const auto *l = qobject_cast<const QLabel *>(w)) n += QStringLiteral(" \"%1\"").arg(l->text());
    else if (!w->objectName().isEmpty()) n += QStringLiteral(" %1").arg(w->objectName());
    return n;
}
}

namespace {
// The program's look as it was (style, style sheet, palette, font, the color scheme the system hints at, the dark and high-contrast flags), put back whatever happens to the check.
class LookGuard {
public:
    LookGuard()
        : m_style(QApplication::style()->name()), m_css(qApp->styleSheet()), m_palette(QApplication::palette()), m_font(QApplication::font()), m_scheme(QGuiApplication::styleHints()->colorScheme()),
          m_dark(jp::uiDark()), m_contrast(jp::uiHighContrast())
    {
    }
    ~LookGuard()
    {
        QApplication::setStyle(QStyleFactory::create(m_style));
        qApp->setStyleSheet(m_css);
        QApplication::setPalette(m_palette);
        QApplication::setFont(m_font);
        QGuiApplication::styleHints()->unsetColorScheme();   // (the check pinned one; if the system's own differs from what it was, that one is pinned)
        if (QGuiApplication::styleHints()->colorScheme() != m_scheme) QGuiApplication::styleHints()->setColorScheme(m_scheme);
        jp::setUiDark(m_dark);
        jp::setUiHighContrast(m_contrast);
    }
    QFont font() const { return m_font; }
    QString style() const { return m_style; }

private:
    QString m_style, m_css;
    QPalette m_palette;
    QFont m_font;
    Qt::ColorScheme m_scheme;
    bool m_dark, m_contrast;
};
}

// The ribbon when its tabs are made as they are first shown looks and holds what it does when every tab is made at once: the same picture of each tab, the same description. Only the
// first tab is made at the start, so the other tabs get their controls (and the window's state in them) when shown. Compared in several places a person can be in: the window as launched
// with some words typed, one opened from a file, each kind of object selected that brings up contextual tabs (a picture, a shape, a table, a chart), and a document open for reading only.
QStringList checkLazyRibbon(Adapter &app)
{
    // One window at a time (a second window would take the first one's place as the active window, and typing would not reach it alike): each tab's picture, and the description.
    struct Shots {
        QStringList names;
        QList<QImage> pictures;
        QString description;
        QString accessible;   // what each control of each tab is called and described as, and whether it has help text, for screen readers (the help text of a button for a command that changes its own words is the words of the time it was made)
        int left = 0;   // tabs not yet made at the start
        qint64 showMs = 0;   // the time showTab took over all of them
        int notMade = 0;
        bool ok = false;
    };
    struct Scenario {
        QString label;
        bool file = false;
        int context = -1;   // an index into app.contexts
        bool readOnly = false;
    };
    QList<Scenario> scenarios{{QStringLiteral("typed words"), false, -1, false}, {QStringLiteral("from a file"), true, -1, false}, {QStringLiteral("read-only"), true, -1, true}};
    for (const QString &word : {QStringLiteral("picture"), QStringLiteral("shape"), QStringLiteral("table"), QStringLiteral("chart")})
        for (int c = 0; c < app.contexts.size(); ++c) {
            const QString name = app.contexts[c].name.toLower();
            if (name.contains(word) && !name.contains(QLatin1String("contents")) && !name.contains(QLatin1String("authorities")) && !name.contains(QLatin1String("pivot"))) {
                scenarios << Scenario{app.contexts[c].name, app.contexts[c].fileOnly, c, false};
                break;
            }
        }
    auto collect = [&](bool lazy, const Scenario &sc) {
        Shots out;
        jp::setLazyRibbonTabs(lazy);
        std::unique_ptr<jp::AppWindow> w(sc.file && !app.sampleFile.isEmpty() ? app.launchWith(app.sampleFile) : app.launch());
        jp::setLazyRibbonTabs(false);
        if (!w || !w->ribbon()) return out;
        w->resize(1280, 760);
        drain();
        if (app.settle) app.settle(w.get());
        // (the place first, so the window's state is not the blank one; the contextual tabs come as the selection says, and each tab is shown by showTab)
        if (sc.context >= 0) app.contexts[sc.context].enter(w.get(), sc.file);
        else if (!sc.file && app.type) app.type(w.get());
        drain();
        if (app.settle) app.settle(w.get());
        if (sc.readOnly) {
            w->setReadOnly(true);
            drain();
            if (app.settle) app.settle(w.get());
        }
        jp::Ribbon *r = w->ribbon();
        for (int i = 0; i < r->tabCount(); ++i) out.left += r->tabAt(i)->isBuilt() ? 0 : 1;
        for (int i = 0; i < r->tabCount(); ++i) {
            QElapsedTimer shown;
            shown.start();
            r->showTab(r->tabAt(i));
            out.showMs += shown.elapsed();
            drain();
            if (app.settle) app.settle(w.get());
            if (!r->tabAt(i)->isBuilt()) ++out.notMade;
            // (a button the pointer happens to be over is lit: no button is)
            r->setAttribute(Qt::WA_UnderMouse, false);
            for (QWidget *c : r->findChildren<QWidget *>()) c->setAttribute(Qt::WA_UnderMouse, false);
            out.names << r->tabName(i);
            // (a picture again until it stops changing: a gallery that fills itself a little at a time on a busy machine is done when it is)
            QImage picture = r->grab().toImage();
            for (int again = 0; again < 8; ++again) {
                drain();
                const QImage next = r->grab().toImage();
                if (next == picture) break;
                picture = next;
            }
            out.pictures << picture;
            for (QWidget *c : r->tabAt(i)->findChildren<QWidget *>()) {
                out.accessible += QStringLiteral("%1/%2|%3|%4|%5|%6|%7\n").arg(r->tabName(i), QString::fromLatin1(c->metaObject()->className()), c->objectName(), c->accessibleName(), c->accessibleDescription(), c->whatsThis().isEmpty() ? QStringLiteral("-") : QStringLiteral("help"), c->toolTip());
            }
        }
        out.description = r->describe(true);
        out.ok = true;
        return out;
    };
    QStringList fails;
    bool first = true;
    for (const Scenario &sc : scenarios) {
        const Shots whole = collect(false, sc), late = collect(true, sc);
        const QString where = app.name + QLatin1String(" (") + sc.label + QLatin1Char(')');
        if (!whole.ok || !late.ok || whole.names != late.names) {
            fails << where + QLatin1String(": no window with a ribbon, or its tabs differ");
            continue;
        }
        if (first) {
            qInfo("lazy ribbon: %s, %d of %lld tabs were made late; showing every tab took %lld ms when they were made late and %lld ms when they were there", qPrintable(app.name), late.left, qint64(late.names.size()), late.showMs, whole.showMs);
            if (late.left < 2) fails << QStringLiteral("%1: %2 tabs were left to make late").arg(where).arg(late.left);
            first = false;
        }
        if (late.notMade) fails << QStringLiteral("%1: %2 tabs were not made when shown").arg(where).arg(late.notMade);
        for (int i = 0; i < whole.names.size(); ++i) {
            const QImage &x = whole.pictures[i], &y = late.pictures[i];
            if (x == y) continue;
            // (where they differ: the first and last rows and columns that do)
            QRect box;
            if (x.size() == y.size())
                for (int row = 0; row < x.height(); ++row)
                    for (int col = 0; col < x.width(); ++col)
                        if (x.pixel(col, row) != y.pixel(col, row)) box = box.isNull() ? QRect(col, row, 1, 1) : box.united(QRect(col, row, 1, 1));
            if (const QString dir = qEnvironmentVariable("JP_LAZY_SHOTS"); !dir.isEmpty()) {
                QDir().mkpath(dir);
                const QString name = QString(sc.label + QLatin1Char('-') + whole.names[i]).replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9]+")), QStringLiteral("-"));
                x.save(QStringLiteral("%1/%2-%3-whole.png").arg(dir, app.name, name));
                y.save(QStringLiteral("%1/%2-%3-late.png").arg(dir, app.name, name));
            }
            fails << QStringLiteral("%1/%2: looks different when made late (differs in %3,%4 %5x%6)").arg(where, whole.names[i]).arg(box.x()).arg(box.y()).arg(box.width()).arg(box.height());
        }
        if (whole.accessible != late.accessible) fails << where + QLatin1String(": the controls are named, described, or told differently for screen readers when their tabs are made late");
        if (whole.description != late.description) fails << where + QLatin1String(": the ribbon holds something else when its tabs are made late");
    }
    return fails;
}

QStringList checkRibbonLayout(Adapter &app)
{
    QStringList fails;
    const QString shots = qEnvironmentVariable("JP_RIBBON_SHOTS");
    if (!shots.isEmpty()) QDir().mkpath(shots);
    // The look a person has: the Fusion style and the program's style sheet (as each app's main() sets them), whatever platform the test runs on, at the system font and at larger ones
    // (desktops differ), at three widths, and once right to left. The program's own settings come back at the end, even if the check throws.
    LookGuard guard;
    qInfo("ribbon layout: platform %s, style %s, font %s %g", qPrintable(QGuiApplication::platformName()), qPrintable(guard.style()), qPrintable(guard.font().family()), guard.font().pointSizeF());
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    jp::applyUiTheme(1);
    QFont base = QApplication::font();
#ifdef Q_OS_LINUX
    // (the offscreen platform has a font of its own; a Linux desktop's is "Sans" at 10 points. Other systems keep the font they have: nothing here names a font that has to be installed.)
    if (QGuiApplication::platformName() == QLatin1String("offscreen")) base = QFont(QStringLiteral("Sans"), 10);
#endif
    struct Pass {
        int width;
        double points;   // 0: the system font
        bool rtl;
    };
    const QList<Pass> passes{{1440, 0, false}, {1024, 0, false}, {1440, 11, false}, {1024, 11, false}, {1440, 12, false}, {1024, 12, false}, {1440, 14, false}, {1024, 14, false}, {800, 12, false}, {1024, 0, true}};
    for (const Pass &pass : passes) {
        const int width = pass.width;
        const double points = pass.points;
        QFont f = base;
        if (points > 0) f.setPointSizeF(points);
        QApplication::setFont(f);
        const QString at = QStringLiteral("%1%2%3").arg(width).arg(points > 0 ? QStringLiteral(" at %1 pt").arg(points) : QString(), pass.rtl ? QStringLiteral(" right to left") : QString());
        std::unique_ptr<jp::AppWindow> w(app.launch());
        if (!w || !w->ribbon()) {
            fails << app.name + QLatin1String(": no window with a ribbon");
            continue;
        }
        if (pass.rtl) w->setLayoutDirection(Qt::RightToLeft);
        w->resize(width, 760);
        drain();
        jp::Ribbon *r = w->ribbon();
        // Every contextual tab shown (the layout is the same as when the document puts the window there).
        QStringList contexts;
        for (int i = 0; i < r->tabCount(); ++i)
            if (!r->tabContextGroup(i).isEmpty() && !contexts.contains(r->tabContextGroup(i))) contexts << r->tabContextGroup(i);
        // The top row (File, the tabs, Help, the collapse chevron) with no contextual tab shown and with each shown alone: nothing overlaps or runs off the window.
        auto headerFails = [&](const QString &what) {
            drain();
            QList<QWidget *> parts;
            if (r->fileButton() && r->fileButton()->isVisible()) parts << r->fileButton();
            for (int i = 0; i < r->tabCount(); ++i)
                if (r->tabVisible(i) && r->tabButton(i) && r->tabButton(i)->isVisible()) parts << r->tabButton(i);
            for (const char *name : {"jpRibbonMore", "jpRibbonHelp", "jpRibbonCollapse"})
                if (QWidget *b = r->findChild<QWidget *>(QString::fromLatin1(name)); b && b->isVisible()) parts << b;
            // the program's name in the corner: whole, inside the window, and clear of every control of the row
            QFont brandFont;
            const QRect brand = r->brandRect(&brandFont);
            const int nameWidth = QFontMetrics(brandFont).horizontalAdvance(jp::appInfo().name);
            if (nameWidth > brand.width() || brand.left() < 0 || brand.right() > r->width())
                fails << QStringLiteral("%1/%2 at %3: the name %4 is cut (it needs %5 wide, its place %6 in a ribbon %7 wide)").arg(app.name, what).arg(at).arg(jp::appInfo().name).arg(nameWidth).arg(rectText(brand)).arg(r->width());
            for (QWidget *part : std::as_const(parts))
                if (QRect(part->mapTo(r, QPoint(0, 0)), part->size()).intersects(brand))
                    fails << QStringLiteral("%1/%2 at %3: %4 %5 overlaps the program's name %6").arg(app.name, what).arg(at).arg(controlName(part), rectText(QRect(part->mapTo(r, QPoint(0, 0)), part->size())), rectText(brand));
            for (int i = 0; i < parts.size(); ++i) {
                const QRect a(parts[i]->mapTo(r, QPoint(0, 0)), parts[i]->size());
                const QString name = controlName(parts[i]);
                // a control of the row shows its whole text, and lies inside the window
                if (auto *tb = qobject_cast<QAbstractButton *>(parts[i]); tb && !tb->text().isEmpty() && tb->fontMetrics().horizontalAdvance(tb->text()) > a.width())
                    fails << QStringLiteral("%1/%2 at %3: %4 %5 is too narrow for its text").arg(app.name, what).arg(at).arg(name, rectText(a));
                if (a.left() < 0 || a.top() < 0 || a.bottom() >= r->findChild<QWidget *>(QStringLiteral("jpRibbonTabs"))->height())
                    fails << QStringLiteral("%1/%2 at %3: %4 %5 is cut by the row").arg(app.name, what).arg(at).arg(name, rectText(a));
                if (auto *tb = qobject_cast<QAbstractButton *>(parts[i]); tb && parts[i]->objectName() == QLatin1String("jpRibbonTab") && a.width() - tb->fontMetrics().horizontalAdvance(tb->text()) < 14)
                    fails << QStringLiteral("%1/%2 at %3: the tab %4 %5 has less than 14 px around its text (the texts are then under 8 px apart)").arg(app.name, what).arg(at).arg(name, rectText(a));
                // the tab labels of the row: no pill touches the next (8 px between the texts at least)
                if (i + 1 < parts.size() && parts[i]->objectName() == QLatin1String("jpRibbonTab") && parts[i + 1]->objectName() == QLatin1String("jpRibbonTab")) {
                    const QRect nb(parts[i + 1]->mapTo(r, QPoint(0, 0)), parts[i + 1]->size());
                    const int gap = nb.left() - a.right() - 1;
                    if (gap < 3) fails << QStringLiteral("%1/%2 at %3: the tabs %4 %5 and %6 %7 are only %8 apart").arg(app.name, what).arg(at).arg(name, rectText(a), controlName(parts[i + 1]), rectText(nb)).arg(gap);
                }
                if (a.right() > r->width()) fails << QStringLiteral("%1/%2 at %3: %4 %5 runs off the ribbon (%6 wide)").arg(app.name, what).arg(at).arg(name, rectText(a)).arg(r->width());
                for (int k = i + 1; k < parts.size(); ++k) {
                    const QRect b(parts[k]->mapTo(r, QPoint(0, 0)), parts[k]->size());
                    if (a.intersects(b)) fails << QStringLiteral("%1/%2 at %3: %4 %5 overlaps %6 %7").arg(app.name, what).arg(at).arg(name, rectText(a), controlName(parts[k]), rectText(b));
                }
            }
        };
        headerFails(QStringLiteral("top row"));
        for (const QString &c : contexts) {
            r->setContextVisible(c, true);
            headerFails(QStringLiteral("top row with ") + c);
            r->setContextVisible(c, false);
        }
        for (const QString &c : contexts) r->setContextVisible(c, true);
        QList<jp::RibbonTab *> pages;
        for (int i = 0; i < r->tabCount(); ++i) pages << r->tabPage(i);
        drain();
        for (jp::RibbonTab *page : pages) {
            r->showTab(page);
            drain();
            QString tabName;
            for (int i = 0; i < r->tabCount(); ++i)
                if (r->tabPage(i) == page) tabName = r->tabName(i);
            const QString where = QStringLiteral("%1/%2 at %3").arg(app.name, tabName).arg(at);
            if (!shots.isEmpty()) {
                QString file = tabName;
                file.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9]+")), QStringLiteral("-"));
                r->grab().save(QStringLiteral("%1/%2-%3-%4.png").arg(shots, app.name, file).arg((points > 0 ? QStringLiteral("%1-%2pt").arg(width).arg(points) : QString::number(width)) + (pass.rtl ? QStringLiteral("-rtl") : QString())));
            }
            QWidget *viewport = page->parentWidget();
            if (auto *sa = qobject_cast<QScrollArea *>(viewport ? viewport->parentWidget() : nullptr))
                if (sa->horizontalScrollBar()->isVisible()) fails << where + QStringLiteral(": the tab scrolls sideways (its groups need %1 wide, there is %2)").arg(page->minimumSizeHint().width()).arg(viewport->width());
            const QRect body = viewport ? viewport->rect() : page->rect();
            bool fitReported = false;   // (every group of the tab is as tall as the tallest: one line says it)
            for (jp::RibbonGroup *g : page->groups()) {
                if (!g->isVisibleTo(page)) continue;
                const QString gw = where + QStringLiteral("/") + g->title();
                const QRect box = QRect(QPoint(0, 0), g->size());
                const QRect inside = box.marginsRemoved(g->layout()->contentsMargins());
                const QRect gr = QRect(g->mapTo(page, QPoint(0, 0)), g->size());
                if (!fitReported && (gr.bottom() > body.bottom() || gr.top() < 0)) {
                    fails << gw + QStringLiteral(": the group %1 does not fit the ribbon's body %2").arg(rectText(gr), rectText(body));
                    fitReported = true;
                }
                if (g->height() < g->minimumSizeHint().height()) fails << gw + QStringLiteral(": the group is %1 tall and its rows need %2").arg(g->height()).arg(g->minimumSizeHint().height());
                if (g->width() < g->minimumSizeHint().width()) fails << gw + QStringLiteral(": the title or the controls are cut (%1 wide, needs %2; its hint is %3, the tab is %4 wide with %5 groups collapsed)").arg(g->width()).arg(g->minimumSizeHint().width()).arg(g->sizeHint().width()).arg(page->width()).arg(page->collapsedCount());
                QList<QWidget *> shown;
                for (QWidget *c : g->controls())
                    if (c->isVisibleTo(g)) shown << c;
                for (int i = 0; i < shown.size(); ++i) {
                    QWidget *c = shown[i];
                    const QRect cr(c->mapTo(g, QPoint(0, 0)), c->size());
                    const QString cw = gw + QStringLiteral("/") + controlName(c);
                    // (the body's own edge, which clips whatever the group's size says)
                    if (viewport && !viewport->rect().contains(QRect(c->mapTo(viewport, QPoint(0, 0)), c->size()))) fails << cw + QStringLiteral(": sticks out of the ribbon's body %1").arg(rectText(viewport->rect()));
                    if (!inside.contains(cr)) fails << cw + QStringLiteral(": %1 is not inside the group's content %2").arg(rectText(cr), rectText(inside));
                    for (int k = i + 1; k < shown.size(); ++k) {
                        const QRect other(shown[k]->mapTo(g, QPoint(0, 0)), shown[k]->size());
                        if (cr.intersects(other)) fails << cw + QStringLiteral(": %1 overlaps %2 %3").arg(rectText(cr), controlName(shown[k]), rectText(other));
                    }
                    // (a drop-down shows its current choice and may be narrower than its longest one)
                    const bool text = qobject_cast<QAbstractButton *>(c) || qobject_cast<QLabel *>(c) || qobject_cast<QAbstractSpinBox *>(c);
                    // (a number box's hint is for the widest number its range allows, so only its height is held to the text)
                    const bool numberBox = qobject_cast<QAbstractSpinBox *>(c);
                    // (a large button holds an icon and up to two lines: its own hint says how much)
                    const bool large = c->property("ribbonLarge").toBool();
                    if (auto *sb = qobject_cast<QAbstractSpinBox *>(c))
                        if (QLineEdit *le = sb->findChild<QLineEdit *>(); le && sb->fontMetrics().horizontalAdvance(le->text()) > le->width())
                            fails << cw + QStringLiteral(": its number \"%1\" is cut (the text area is %2 wide and the number %3)").arg(le->text()).arg(le->width()).arg(sb->fontMetrics().horizontalAdvance(le->text()));
                    if (large) {
                        // the label's lines as they are drawn: each inside the button (the menu arrow beside the second), and the last above its bottom edge
                        auto *lb = static_cast<QToolButton *>(c);
                        const QStringList lines = jp::largeRibbonLabelLines(lb);
                        const QFontMetrics fm(lb->font());
                        const bool menu = lb->menu() || lb->popupMode() == QToolButton::InstantPopup;
                        for (int li = 0; li < lines.size(); ++li)
                            if (fm.horizontalAdvance(lines[li]) > lb->width() - 6 - (menu && li == 1 ? 12 : 0))
                                fails << cw + QStringLiteral(": the line \"%1\" is wider than the button (%2)").arg(lines[li], rectText(cr));
                        const int need = 1 + 4 + lb->iconSize().height() + 4 + int(lines.size() - 1) * (fm.height() - 1) + fm.height() - 3;
                        if (need > lb->height() || lines.size() > 2) fails << cw + QStringLiteral(": its %1 lines need %2 tall and the button is %3").arg(lines.size()).arg(need).arg(lb->height());
                    }
                    if (large ? false : text && ((!numberBox && c->sizeHint().width() > c->width()) || c->fontMetrics().height() > c->height()))
                        fails << cw + QStringLiteral(": its text is cut (it is %1 and wants %2 wide and %3 tall for a line)").arg(rectText(cr)).arg(c->sizeHint().width()).arg(c->fontMetrics().height());
                }
            }
            for (jp::RibbonGroup *g : page->groups()) {
                if (!g->isVisibleTo(page)) continue;
                // The fields beside the labels of one column start at the same place (a label and the box it names, row under row).
                QHash<int, QList<QPair<QWidget *, int>>> fieldsByLabelColumn;
                QList<QWidget *> shown;
                for (QWidget *c : g->controls())
                    if (c->isVisibleTo(g)) shown << c;
                for (QWidget *l : shown) {
                    if (!qobject_cast<QLabel *>(l)) continue;
                    const QRect lr(l->mapTo(g, QPoint(0, 0)), l->size());
                    QWidget *field = nullptr;
                    int fieldLeft = 0;
                    for (QWidget *f : shown) {
                        if (f == l || qobject_cast<QLabel *>(f)) continue;
                        const QRect fr(f->mapTo(g, QPoint(0, 0)), f->size());
                        if (fr.left() >= lr.right() && fr.top() < lr.bottom() && fr.bottom() > lr.top() && (!field || fr.left() < fieldLeft)) {
                            field = f;
                            fieldLeft = fr.left();
                        }
                    }
                    if (field) fieldsByLabelColumn[lr.left()].append({field, fieldLeft});
                }
                for (const auto &column : std::as_const(fieldsByLabelColumn))
                    for (const auto &f : column)
                        if (f.second != column.first().second)
                            fails << where + QStringLiteral("/") + g->title() + QStringLiteral("/") + controlName(f.first) + QStringLiteral(": starts at x=%1, the field above it at x=%2 (the fields of a column line up)").arg(f.second).arg(column.first().second);
            }
            for (int i = 0; i < page->groups().size(); ++i) {
                QToolButton *b = page->collapsedButton(i);
                if (!b || !b->isVisibleTo(page)) continue;
                const QRect br(b->mapTo(page, QPoint(0, 0)), b->size());
                if (!body.contains(br) || b->sizeHint().width() > b->width())
                    fails << where + QStringLiteral("/") + page->groups()[i]->title() + QStringLiteral(" (collapsed): the button %1 is cut in the body %2").arg(rectText(br), rectText(body));
            }
        }
    }
    // (JP_RIBBON_REPORT=<file> keeps every failure line: a test's message is cut short)
    if (const QString report = qEnvironmentVariable("JP_RIBBON_REPORT"); !report.isEmpty())
        if (QFile f(report); f.open(QIODevice::WriteOnly | QIODevice::Truncate)) f.write(fails.join('\n').toUtf8());
    return fails;
}

void checkKeyboardPage(Adapter &app)
{
    std::unique_ptr<jp::AppWindow> w(app.launch());
    QVERIFY2(w, "the app made no window");
    if (app.settle) app.settle(w.get());
    QAction *options = w->act(QStringLiteral("file.options"));
    QVERIFY(options);
    // The commands the page must list: every one with a name that is not a menu.
    QStringList wanted;
    QString keyed, keyedKeys;
    int withKeys = 0;   // (the commands that have a key: the page must show keys for each)
    for (const QString &id : w->actionIds()) {
        QAction *a = w->act(id);
        if (!a || a->menu() || QString(a->text()).remove(QLatin1Char('&')).simplified().isEmpty()) continue;
        wanted << id;
        withKeys += !a->shortcuts().isEmpty();
        if (keyed.isEmpty() && !a->shortcuts().isEmpty() && !QString(a->text()).contains(QLatin1Char('&'))) {
            keyed = id;
            QStringList s;
            for (const QKeySequence &k : a->shortcuts()) s << k.toString(QKeySequence::NativeText);
            keyedKeys = s.join(QStringLiteral(", "));
        }
    }
    QVERIFY(!wanted.isEmpty());
    QVERIFY(withKeys > 0);
    QVERIFY(!keyed.isEmpty());
    const QString tab = w->ribbon() ? w->ribbon()->tabTitle(0) : QString();
    withDialog([&] { options->trigger(); }, QStringLiteral("optionsDialog"), [&](QDialog *dlg) {
        QWidget *page = dlg->findChild<QWidget *>(QStringLiteral("keyboardPage"));
        QVERIFY2(page, "Options has no Keyboard page");
        auto *tree = page->findChild<QTreeWidget *>(QStringLiteral("keyCommands"));
        auto *search = page->findChild<QLineEdit *>(QStringLiteral("keySearch"));
        QVERIFY(tree && search);
        QHash<QString, QTreeWidgetItem *> items;
        QStringList groups;
        for (int i = 0; i < tree->topLevelItemCount(); ++i) {
            groups << tree->topLevelItem(i)->text(0);
            for (int j = 0; j < tree->topLevelItem(i)->childCount(); ++j) items.insert(tree->topLevelItem(i)->child(j)->data(0, Qt::UserRole).toString(), tree->topLevelItem(i)->child(j));
        }
        QStringList missing;
        for (const QString &id : wanted)
            if (!items.contains(id)) missing << id;
        QVERIFY2(missing.isEmpty(), qPrintable(app.name + QStringLiteral(" commands missing from the Keyboard page: ") + missing.join(QLatin1Char(' '))));
        // grouped by the ribbon's tabs, and each command shows its keys
        if (!tab.isEmpty()) QVERIFY2(groups.contains(tab), qPrintable(groups.join(QLatin1Char('|'))));
        QCOMPARE(items[keyed]->text(1), keyedKeys);
        int showing = 0;
        for (auto it = items.begin(); it != items.end(); ++it) showing += !it.value()->text(1).isEmpty();
        QCOMPARE(showing, withKeys);
        // the search finds a command by its name and hides the others
        search->setText(items[keyed]->text(0));
        QVERIFY(!items[keyed]->isHidden());
        int shown = 0;
        for (auto it = items.begin(); it != items.end(); ++it) shown += !it.value()->isHidden();
        QVERIFY2(shown < items.size() / 2, qPrintable(QString::number(shown)));
        search->clear();
        // a reserved key is said to be reserved
        tree->setCurrentItem(items[keyed]);
        // (JP_KEYBOARD_SHOT names a folder to keep a picture of the page in, for looking at)
        const QString shotDir = qEnvironmentVariable("JP_KEYBOARD_SHOT");
        auto *press = page->findChild<QKeySequenceEdit *>(QStringLiteral("keyPress"));
        auto *message = page->findChild<QLabel *>(QStringLiteral("keyMessage"));
        QVERIFY(press && message);
        press->setKeySequence(QKeySequence(Qt::CTRL | Qt::Key_C));
        QVERIFY2(message->text().contains(QLatin1String("reserved")), qPrintable(message->text()));
        QVERIFY(!page->findChild<QPushButton *>(QStringLiteral("keyAssign"))->isEnabled());
        if (!shotDir.isEmpty()) {
            for (QTabWidget *tabs : dlg->findChildren<QTabWidget *>())
                for (int i = 0; i < tabs->count(); ++i)
                    if (tabs->widget(i) == page || tabs->widget(i)->isAncestorOf(page)) tabs->setCurrentIndex(i);
            dlg->resize(720, 660);
            QApplication::processEvents();
            QDir().mkpath(shotDir);
            dlg->grab().save(shotDir + QLatin1Char('/') + app.name + QStringLiteral("-keyboard.png"));
        }
    }, false);
}

void checkRibbonPage(Adapter &app)
{
    std::unique_ptr<jp::AppWindow> w(app.launch());
    QVERIFY2(w, "the app made no window");
    if (app.settle) app.settle(w.get());
    QAction *options = w->act(QStringLiteral("file.options"));
    QVERIFY(options);
    jp::Ribbon *ribbon = w->ribbon();
    QVERIFY(ribbon);
    ribbon->buildAllTabs();
    QStringList titles, contextual;
    QList<int> groupCounts;
    for (int i = 0; i < ribbon->tabCount(); ++i) {
        (ribbon->tabContextGroup(i).isEmpty() ? titles : contextual) << ribbon->tabTitle(i);
        if (ribbon->tabContextGroup(i).isEmpty()) groupCounts << int(ribbon->tabPage(i)->groups().size());
    }
    QVERIFY(!titles.isEmpty());
    // the commands the page must list: the ones the keyboard page lists (one function decides), each with the ribbon tab it is on
    int wanted = 0;
    for (const QString &id : w->actionIds())
        if (jp::KeyMap::listsCommand(w->act(id))) ++wanted;
    const jp::KeyMap keys(w.get());
    QCOMPARE(int(keys.commands().size()), wanted);
    QCOMPARE(titles.first(), QStringLiteral("Home"));
    const int tabsBefore = ribbon->tabCount();
    withDialog([&] { options->trigger(); }, QStringLiteral("optionsDialog"), [&](QDialog *dlg) {
        QWidget *page = dlg->findChild<QWidget *>(QStringLiteral("ribbonPage"));
        QVERIFY2(page, "Options has no Ribbon page");
        QVERIFY2(dlg->findChild<QWidget *>(QStringLiteral("keyboardPage")), "the Keyboard page is gone");
        auto *tree = page->findChild<QTreeWidget *>(QStringLiteral("ribbonTree"));
        auto *list = page->findChild<QListWidget *>(QStringLiteral("ribbonCommands"));
        auto *search = page->findChild<QLineEdit *>(QStringLiteral("ribbonSearch"));
        QVERIFY(tree && list && search);
        // every tab, in the ribbon's order, with its groups; Home has no box to clear, and a tab that comes and goes has none either
        QStringList listed;
        for (int i = 0; i < tree->topLevelItemCount(); ++i) listed << tree->topLevelItem(i)->text(0);
        QCOMPARE(listed.mid(0, titles.size()), titles);
        QCOMPARE(listed.size(), ribbon->tabCount());
        QVERIFY(!(tree->topLevelItem(0)->flags() & Qt::ItemIsUserCheckable));
        for (int i = 0; i < titles.size(); ++i) QCOMPARE(tree->topLevelItem(i)->childCount(), groupCounts[i]);
        QVERIFY(tree->topLevelItem(1)->flags() & Qt::ItemIsUserCheckable);
        if (!contextual.isEmpty()) QVERIFY(!(tree->topLevelItem(tree->topLevelItemCount() - 1)->flags() & Qt::ItemIsUserCheckable));
        // every command with a name is there to be put in a group, and the search narrows them
        QCOMPARE(list->count(), wanted);
        const QString first = list->item(0)->data(Qt::UserRole + 1).toString();
        search->setText(first);
        // exactly the commands whose name or tab has all the words, as the keyboard page's own list says them
        int expected = 0;
        for (const jp::KeyCommand &c : keys.commands()) {
            const QString hay = (c.text + QStringLiteral(" (") + c.group + QLatin1Char(')')).toLower();
            bool all = true;
            for (const QString &word : first.simplified().toLower().split(QLatin1Char(' '), Qt::SkipEmptyParts)) all = all && hay.contains(word);
            expected += all;
        }
        int shown = 0;
        for (int i = 0; i < list->count(); ++i) shown += !list->item(i)->isHidden();
        QCOMPARE(shown, expected);
        QVERIFY(shown >= 1);
        search->clear();
        // a tab of one's own can be made (Cancel throws it away)
        auto *name = page->findChild<QLineEdit *>(QStringLiteral("ribbonName"));
        auto *newTab = page->findChild<QPushButton *>(QStringLiteral("ribbonNewTab"));
        QVERIFY(name && newTab);
        name->setText(QStringLiteral("My Tab"));
        newTab->click();
        bool found = false;
        for (int i = 0; i < tree->topLevelItemCount(); ++i) found = found || tree->topLevelItem(i)->text(0) == QLatin1String("My Tab");
        QVERIFY(found);
        // (JP_RIBBON_SHOT names a folder to keep a picture of the page in, for looking at)
        const QString shotDir = qEnvironmentVariable("JP_RIBBON_SHOT");
        if (!shotDir.isEmpty()) {
            for (QTabWidget *tabs : dlg->findChildren<QTabWidget *>())
                for (int i = 0; i < tabs->count(); ++i)
                    if (tabs->widget(i) == page || tabs->widget(i)->isAncestorOf(page)) tabs->setCurrentIndex(i);
            dlg->resize(760, 700);
            QApplication::processEvents();
            QDir().mkpath(shotDir);
            dlg->grab().save(shotDir + QLatin1Char('/') + app.name + QStringLiteral("-ribbon.png"));
        }
    }, false);
    // Cancel: the ribbon is as it was, and nothing is kept
    QCOMPARE(ribbon->tabCount(), tabsBefore);
    QVERIFY(jp::RibbonCustom::load().isEmpty());
}

} // namespace appchecks
