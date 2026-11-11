#include "tst_write.h"

// The run's settings are its own, and none reached the person's (checked at the end).
void WriteTests::cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }


void WriteTests::initTestCase()
{
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(write);
    setAppInfo(writeAppInfo());
    initCore();   // the program's own fonts, as JeffWrite loads them (a test machine may have none: Qt on Windows ships no fonts)
}


// ---------------------------------------------------------------- the app
void WriteTests::theWindowHasItsTabsAndBuildsCleanly()
{
    WriteWindow w;
    QCOMPARE(w.ribbonErrors(), QString());
    QStringList tabs;
    for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
    QCOMPARE(tabs, (QStringList{"Home", "Insert", "Design", "Layout", "References", "Mailings", "Review", "View", "Help", "Header & Footer", "Outlining", "Table Design", "Table Layout", "Picture Format", "Shape Format", "Chart Format"}));   // (the last five are there while a header or footer is edited, Outline view is open, and the caret is in a table)
    QCOMPARE(w.ribbon()->fileKeytip(), QStringLiteral("F"));
    QVERIFY(w.windowTitle().contains(QLatin1String("JeffWrite")));
    QVERIFY(w.windowTitle().startsWith(QLatin1String("Document")));
    // The commands a person looks for.
    for (const char *id : {"file.new", "file.open", "file.save", "file.saveAs", "file.print", "file.exportPdf", "file.options", "edit.undo", "edit.redo", "fmt.bold", "para.bullets", "insert.pageBreak",
                           "layout.landscape", "zoom.pageWidth", "help.show", "review.spelling", "review.checkAsType"})
        QVERIFY2(w.act(QString::fromLatin1(id)), id);
}


void WriteTests::itsIdentityIsItsOwn()
{
    QCOMPARE(appInfo().name, QStringLiteral("JeffWrite"));
    QCOMPARE(appInfo().accent, QColor(0x1D, 0x55, 0xC4));
    QCOMPARE(appInfo().badge, QStringLiteral("JW"));
    QCOMPARE(appInfo().helpFolder, QStringLiteral("write"));
}


void WriteTests::ribbonStringsListed()
{
    // The list that translators start from is made from the ribbon's file: run tools/ribbon_strings.py after changing its text.
    QFile json(QStringLiteral(JW_SOURCE_DIR "/apps/write/resources/ribbon.json")), listed(QStringLiteral(JW_SOURCE_DIR "/apps/write/app/ribbon_strings.cpp"));
    QVERIFY(json.open(QIODevice::ReadOnly) && listed.open(QIODevice::ReadOnly));
    QSet<QString> want;
    std::function<void(const QJsonValue &)> walk = [&](const QJsonValue &v) {
        if (v.isObject()) {
            const QJsonObject o = v.toObject();
            for (auto it = o.begin(); it != o.end(); ++it) {
                if (it.value().isString() && QSet<QString>{"name", "text", "launcherTip", "tip", "label", "section", "submenu", "context"}.contains(it.key()) && !it.value().toString().isEmpty())
                    want.insert(it.value().toString());
                walk(it.value());
            }
        } else if (v.isArray()) {
            for (const QJsonValue &x : v.toArray()) walk(x);
        }
    };
    walk(QJsonDocument::fromJson(json.readAll()).object());
    QSet<QString> have;
    static const QRegularExpression re(QStringLiteral("QT_TRANSLATE_NOOP\\(\"Ribbon\", \"([^\"]*)\"\\)"));
    auto it = re.globalMatch(QString::fromUtf8(listed.readAll()));
    while (it.hasNext()) have.insert(it.next().captured(1));
    QCOMPARE(have, want);
}


void WriteTests::theFileScreenHasItsPages()
{
    WriteWindow w;
    w.show();
    for (const char *page : {"new", "open", "print", "export", "about"}) {
        w.showFileScreen(QString::fromLatin1(page));
        auto *screen = w.findChild<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
        Q_UNUSED(screen);
        QVERIFY(w.findChild<FileScreen *>());
        QCOMPARE(w.findChild<FileScreen *>()->currentPage(), QString::fromLatin1(page));
    }
    w.hideFileScreen();
}


void WriteTests::saveAndOpenKeepTheText()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WriteWindow a;
    typeText(*a.document(), a.view()->cursor(), QStringLiteral("Dear reader,\nThis is the letter."));
    QVERIFY(a.isModified());
    a.setSavePathChooser([&](const QString &suggested, const QString &) { return dir.filePath(QFileInfo(suggested).fileName()); });
    QVERIFY(a.saveAs());
    QVERIFY(!a.isModified());
    QVERIFY(a.filePath().endsWith(QLatin1String(".docx")));
    QVERIFY(QFileInfo(a.filePath()).size() > 1000);
    WriteWindow b;
    QVERIFY(b.openFile(a.filePath()));
    QCOMPARE(b.document()->story()->toPlainText(), QStringLiteral("Dear reader,\nThis is the letter."));
    QCOMPARE(b.filePath(), a.filePath());
    QVERIFY(!b.isModified());
}


// Save a Copy writes the document to another file and goes on with the original: the window keeps its name and its unsaved changes.
void WriteTests::saveACopyKeepsWorkingInTheOriginal()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WriteWindow w;
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("First."));
    const QString original = dir.filePath(QStringLiteral("letter.docx")), copy = dir.filePath(QStringLiteral("copy.docx"));
    w.setSavePathChooser([&](const QString &, const QString &) { return original; });
    QVERIFY(w.saveAs());
    typeText(*w.document(), w.view()->cursor(), QStringLiteral(" More."));
    QVERIFY(w.isModified());
    w.setSavePathChooser([&](const QString &, const QString &) { return copy; });
    QVERIFY(w.saveCopy());
    QVERIFY(w.isModified());
    QCOMPARE(w.filePath(), original);
    WriteWindow a;
    QVERIFY(a.openFile(copy));
    QVERIFY(a.document()->story()->toPlainText().contains(QStringLiteral("More.")));
    WriteWindow b;
    QVERIFY(b.openFile(original));
    QVERIFY(!b.document()->story()->toPlainText().contains(QStringLiteral("More.")));
    // No name chosen: nothing is written.
    w.setSavePathChooser([&](const QString &, const QString &) { return QString(); });
    QVERIFY(!w.saveCopy());
}


// With "Always create backup copy" on, a save keeps the file as it was as "Backup of <name>".
void WriteTests::aBackupCopyIsKeptOnSave()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WriteWindow w;
    const QString path = dir.filePath(QStringLiteral("letter.docx"));
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("First."));
    SettingGuard off(QStringLiteral("save/backup"), false, false);   // (the test does not rely on what the option was)
    QVERIFY(w.saveTo(path));
    typeText(*w.document(), w.view()->cursor(), QStringLiteral(" Second."));
    QVERIFY(w.saveTo(path));
    QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("Backup of letter.docx"))));   // (the option is off)
    bool saved = false;
    {
        SettingGuard keep(QStringLiteral("save/backup"), true, false);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral(" Third."));
        saved = w.saveTo(path);
    }
    QVERIFY(saved);
    WriteWindow old;
    QVERIFY(old.openFile(dir.filePath(QStringLiteral("Backup of letter.docx"))));   // the file as it was before this save
    QVERIFY(old.document()->story()->toPlainText().contains(QStringLiteral("Second.")));
    QVERIFY(!old.document()->story()->toPlainText().contains(QStringLiteral("Third.")));
}


// Open Read-Only: the keys that move the caret work and nothing else changes the text; commands that change are refused; Save asks for a new name, and the new file is an ordinary one.
void WriteTests::aDocumentOpenedReadOnlyCannotBeChanged()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("letter.docx"));
    {
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Dear reader."));
        QVERIFY(a.saveTo(path));
    }
    WriteWindow w;
    QVERIFY(w.openFile(path, OpenHow::ReadOnly));
    QVERIFY(w.isReadOnly());
    QVERIFY(w.windowTitle().contains(QLatin1String("Read-Only")));
    const QString before = w.document()->story()->toPlainText();
    QTest::keyClicks(w.view(), QStringLiteral("xyz"));
    QTest::keyClick(w.view(), Qt::Key_Backspace);
    QTest::keyClick(w.view(), Qt::Key_Return);
    w.act(QStringLiteral("insert.pageBreak"))->trigger();
    QCOMPARE(w.document()->story()->toPlainText(), before);
    QVERIFY(!w.isModified());
    // Save asks for a new name (never the file it was opened from).
    const QString other = dir.filePath(QStringLiteral("changed.docx"));
    QStringList told;
    w.setNotifier([&](const QString &, const QString &text) { told << text; });
    w.setSavePathChooser([&](const QString &, const QString &) { return other; });
    QVERIFY(w.save());
    QCOMPARE(told.size(), 1);
    QCOMPARE(w.filePath(), other);
    QVERIFY(!w.isReadOnly());
    QTest::keyClicks(w.view(), QStringLiteral("xyz"));
    QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("xyz")));
}


// In a document open for reading only, no control changes it: not the font and size boxes, the color buttons, the style gallery, the number boxes, the correction a spelling menu
// offers, and not a command (each is refused). The controls are turned off too.
void WriteTests::aReadOnlyDocumentIsChangedByNoControl()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("letter.docx"));
    {
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Dear reader, the adress is below."));
        QVERIFY(a.saveTo(path));
    }
    WriteWindow w;
    QVERIFY(w.openFile(path, OpenHow::ReadOnly));
    QVERIFY(w.isReadOnly());
    w.view()->selectAll();
    const QString before = w.document()->story()->toHtml();
    QString err;
    const QByteArray fileBefore = w.document()->toDocx(&err);
    QVERIFY(!fileBefore.isEmpty());
    int tried = 0;
    for (FontCombo *f : w.ribbon()->findChildren<FontCombo *>()) {
        QVERIFY(!f->isEnabled());
        Q_EMIT f->familyChosen(QStringLiteral("Courier New"));
        ++tried;
    }
    for (SizeCombo *c : w.ribbon()->findChildren<SizeCombo *>()) {
        QVERIFY(!c->isEnabled());
        Q_EMIT c->sizeChosen(40.0);
        ++tried;
    }
    for (ColorButton *b : w.ribbon()->findChildren<ColorButton *>()) {
        QVERIFY(!b->isEnabled());
        Q_EMIT b->colorPicked(ColorRef::rgb(QColor(200, 0, 0)));
        ++tried;
    }
    for (Gallery *g : w.ribbon()->findChildren<Gallery *>()) {
        Q_EMIT g->activated(QStringLiteral("p:Heading1"));
        ++tried;
    }
    for (MeasureSpin *m : w.ribbon()->findChildren<MeasureSpin *>()) {
        QVERIFY(!m->isEnabled());
        m->setValue(m->value() + 20);
        Q_EMIT m->editingFinished();
        ++tried;
    }
    QVERIFY(tried >= 8);
    // The highlight menu (made in code, with no command behind it) is refused too.
    for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
        if (b->menu() && b->menu()->actions().size() > 10 && b->menu()->actions().first()->objectName().isEmpty()) b->menu()->actions().first()->trigger();
    // Every command that is not safe is refused (the ones that open a window are not run: they are safe by their name).
    for (const QString &id : w.actionIds())
        if (!AppWindow::commandAllowedReadOnly(id)) w.act(id)->trigger();
    // A correction from the spelling menu changes nothing.
    QTextCursor &c = w.view()->cursor();
    c.setPosition(w.document()->story()->toPlainText().indexOf(QLatin1String("adress")) + 2);
    w.view()->setCursor(c);
    // (not vacuous: spell checking is on, the word is flagged, and the same text in an ordinary window is offered corrections)
    w.view()->spelling()->setEnabled(true);
    QVERIFY(w.view()->spelling()->enabled());
    WriteWindow ordinary;
    QVERIFY(ordinary.openFile(path));
    ordinary.view()->spelling()->setEnabled(true);
    QTextCursor &oc = ordinary.view()->cursor();
    oc.setPosition(ordinary.document()->story()->toPlainText().indexOf(QLatin1String("adress")) + 2);
    ordinary.view()->setCursor(oc);
    QMenu offered;
    QVERIFY(addSpellingSuggestions(offered, ordinary.view()));
    QVERIFY(offered.actions().size() > 2);
    QMenu menu;
    QVERIFY(!addSpellingSuggestions(menu, w.view()));
    QVERIFY(menu.isEmpty());
    QCOMPARE(w.document()->story()->toHtml(), before);
    QCOMPARE(w.document()->toDocx(&err), fileBefore);
    QVERIFY(!w.isModified());
    // Made ordinary by Save As, the same controls work again.
    w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("other.docx")); });
    w.setNotifier([](const QString &, const QString &) {});
    QVERIFY(w.save());
    QVERIFY(!w.isReadOnly());
    for (FontCombo *f : w.ribbon()->findChildren<FontCombo *>()) QVERIFY(f->isEnabled());
    w.view()->selectAll();
    for (FontCombo *f : w.ribbon()->findChildren<FontCombo *>()) Q_EMIT f->familyChosen(QStringLiteral("Courier New"));
    QVERIFY(w.document()->story()->toHtml() != before);
}


// Save As in a document open for reading only suggests a name of its own and refuses the file's own name; Save a Copy never writes over the file the window has open.
void WriteTests::aReadOnlyDocumentIsNeverWrittenOverItsOwnFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("letter.docx"));
    {
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Dear reader."));
        QVERIFY(a.saveTo(path));
    }
    auto bytesOf = [](const QString &p) {
        QFile f(p);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray("(none)");
    };
    const QByteArray original = bytesOf(path);
    WriteWindow w;
    QStringList told;
    w.setNotifier([&](const QString &, const QString &text) { told << text; });
    QVERIFY(w.openFile(path, OpenHow::ReadOnly));
    QString suggested;
    w.setSavePathChooser([&](const QString &s, const QString &) {
        suggested = s;
        return path;   // (the person accepts the file's own name)
    });
    QVERIFY(!w.saveAs());
    QVERIFY(!suggested.isEmpty() && QFileInfo(suggested).absoluteFilePath() != QFileInfo(path).absoluteFilePath());
    QVERIFY(!told.isEmpty());
    QCOMPARE(bytesOf(path), original);
    QVERIFY(w.isReadOnly());
    QVERIFY(!w.saveTo(path));
    QCOMPARE(bytesOf(path), original);
    // (the same file by another spelling of its name)
    QVERIFY(!w.saveTo(dir.path() + QStringLiteral("/./letter.docx")));
    QCOMPARE(bytesOf(path), original);
    // An ordinary window: Save a Copy over its own file is refused, and the file is as it was.
    WriteWindow o;
    o.setNotifier([&](const QString &, const QString &text) { told << text; });
    QVERIFY(o.openFile(path));
    typeText(*o.document(), o.view()->cursor(), QStringLiteral(" More."));
    told.clear();
    o.setSavePathChooser([&](const QString &, const QString &) { return path; });
    QVERIFY(!o.saveCopy());
    QCOMPARE(told.size(), 1);
    QCOMPARE(bytesOf(path), original);
    QVERIFY(o.isModified());
}


// A template opened read-only is the template itself, for reading (not a new document made from it).
void WriteTests::aTemplateOpenedReadOnlyIsOpenedReadOnly()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("letter.dotx"));
    {
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Dear <name>,"));
        QVERIFY(a.saveTo(path));
    }
    WriteWindow w;
    QVERIFY(w.openFile(path, OpenHow::ReadOnly));
    QVERIFY(w.isReadOnly());
    QCOMPARE(w.filePath(), path);
    QVERIFY(!w.isModified());
    QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("Dear <name>,")));
    const QString before = w.document()->story()->toPlainText();
    QTest::keyClicks(w.view(), QStringLiteral("xyz"));
    QCOMPARE(w.document()->story()->toPlainText(), before);
    // Opened as usual, it is still a new document made from the template.
    WriteWindow b;
    QVERIFY(b.openFile(path));
    QVERIFY(b.filePath().isEmpty());
    QVERIFY(!b.isReadOnly());
}


// A backup that can't be made asks whether to save without one (the default is not to save): the file is as it was when the answer is no; the backup there before is kept.
void WriteTests::aBackupThatFailsAsksBeforeSaving()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("letter.docx"));
    const QString backup = dir.filePath(QStringLiteral("Backup of letter.docx"));
    WriteWindow w;
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("First."));
    QVERIFY(w.saveTo(path));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray first = f.readAll();
    f.close();
    SettingGuard keep(QStringLiteral("save/backup"), true, false);
    // The backup's place is a folder: no copy can go there.
    QVERIFY(QDir().mkpath(backup));
    QStringList asked;
    setBackupFailureStandIn([&](const QString &p) {
        asked << p;
        return false;
    });
    struct Reset { ~Reset() { setBackupFailureStandIn({}); } } reset;
    typeText(*w.document(), w.view()->cursor(), QStringLiteral(" Second."));
    QVERIFY(!w.saveTo(path));
    QCOMPARE(asked.size(), 1);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(f.readAll(), first);
    f.close();
    QVERIFY(w.isModified());
    QVERIFY(QFileInfo(backup).isDir());
    // Yes: the save goes on.
    setBackupFailureStandIn([&](const QString &) { return true; });
    QVERIFY(w.saveTo(path));
    QVERIFY(!w.isModified());
    // A backup that can be made replaces the old one whole, and leaves no half-made file.
    QDir(backup).removeRecursively();
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("junk");
    f.close();
    QFile old(backup);
    QVERIFY(old.open(QIODevice::WriteOnly));
    old.write("the old backup");
    old.close();
    QCOMPARE(keepBackupBeforeSave(path), BackupResult::Made);
    QVERIFY(old.open(QIODevice::ReadOnly));
    QCOMPARE(old.readAll(), QByteArray("junk"));
    old.close();
    QStringList left = QDir(dir.path()).entryList(QDir::Files);
    left.removeAll(QStringLiteral("letter.docx"));
    left.removeAll(QStringLiteral("Backup of letter.docx"));
    QVERIFY2(left.isEmpty(), qPrintable(left.join(QLatin1Char(','))));
}


// The real question (no stand-in): its default button is No, and pressing Enter leaves the file as it was.
void WriteTests::theBackupQuestionDefaultsToNo()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("letter.docx"));
    WriteWindow w;
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("First."));
    QVERIFY(w.saveTo(path));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray first = f.readAll();
    f.close();
    SettingGuard keep(QStringLiteral("save/backup"), true, false);
    QVERIFY(QDir().mkpath(dir.filePath(QStringLiteral("Backup of letter.docx"))));
    setBackupFailureStandIn({});
    typeText(*w.document(), w.view()->cursor(), QStringLiteral(" Second."));
    QString defaultText;
    bool asked = false;
    QTimer press;
    press.setInterval(20);
    QObject::connect(&press, &QTimer::timeout, &press, [&] {
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *box = qobject_cast<QMessageBox *>(top); box && box->isVisible()) {
                asked = true;
                if (box->defaultButton()) defaultText = box->defaultButton()->text();
                QTest::keyClick(box, Qt::Key_Return);
                press.stop();
                return;
            }
    });
    press.start();
    QVERIFY(!w.saveTo(path));
    QVERIFY(asked);
    QVERIFY2(defaultText.remove(QLatin1Char('&')) == QLatin1String("No"), qPrintable(defaultText));
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(f.readAll(), first);
}


// Open as Copy: the file's contents as a new document that has no file yet, so Save asks where, and the file is never written.
void WriteTests::aFileOpenedAsACopyIsANewDocument()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("letter.docx"));
    {
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Dear reader."));
        QVERIFY(a.saveTo(path));
    }
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray bytes = f.readAll();
    f.close();
    WriteWindow w;
    QVERIFY(w.openFile(path, OpenHow::Copy));
    QVERIFY(w.filePath().isEmpty());
    QVERIFY(w.isModified());
    QVERIFY(!w.isReadOnly());
    QVERIFY(w.displayName().contains(QLatin1String("letter")));
    QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("Dear reader.")));
    const QString other = dir.filePath(QStringLiteral("second.docx"));
    w.setSavePathChooser([&](const QString &, const QString &) { return other; });
    QVERIFY(w.save());
    QVERIFY(QFileInfo::exists(other));
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(f.readAll(), bytes);
}


// Open in New Window: even a window that holds a new document nobody has touched stays as it is, and the file opens in a window of its own.
void WriteTests::aFileCanBeOpenedInANewWindow()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("letter.docx"));
    {
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Dear reader."));
        QVERIFY(a.saveTo(path));
    }
    WriteWindow w;
    const int before = AppWindow::windows().size();
    QVERIFY(w.openFile(path, OpenHow::NewWindow));
    QVERIFY(w.filePath().isEmpty());
    QCOMPARE(AppWindow::windows().size(), before + 1);
    WriteWindow *opened = nullptr;
    for (AppWindow *other : AppWindow::windows())
        if (auto *ww = qobject_cast<WriteWindow *>(other); ww && ww != &w && ww->filePath() == path) opened = ww;
    QVERIFY(opened);
    opened->setAttribute(Qt::WA_DeleteOnClose, false);
    delete opened;
}


// Save as Template puts a .dotx in My Templates (with a picture of its first page) and leaves the document as it was; a document made from it has its text and no file.
void WriteTests::saveAsTemplatePutsADocumentInMyTemplates()
{
    jptest::TestModeGuard testMode;
    QDir(myTemplatesFolder()).removeRecursively();
    WriteWindow w;
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("Dear <name>,"));
    QVERIFY(w.isModified());
    setTemplateNameStandIn([](const QString &) { return QStringLiteral("Letter"); });
    const bool saved = w.saveAsTemplate();
    setTemplateNameStandIn({});
    QVERIFY(saved);
    QVERIFY(w.isModified());
    QVERIFY(w.filePath().isEmpty());
    const QStringList found = myTemplateFiles({QStringLiteral("*.dotx")});
    QCOMPARE(found.size(), 1);
    QCOMPARE(QFileInfo(found.first()).fileName(), QStringLiteral("Letter.dotx"));
    QVERIFY(!templateThumbnail(found.first()).isNull());
    // The New page lists it, and choosing it makes a new document from it.
    w.showFileScreen(QStringLiteral("new"));
    auto *list = w.findChild<QListWidget *>(QStringLiteral("myTemplates"));
    QVERIFY(list);
    QCOMPARE(list->count(), 1);
    w.hideFileScreen();
    WriteWindow b;
    QVERIFY(b.openFile(found.first()));
    QVERIFY(b.filePath().isEmpty());
    QVERIFY(b.document()->story()->toPlainText().contains(QStringLiteral("Dear <name>,")));
    QVERIFY(removeMyTemplate(found.first()));
    QVERIFY(myTemplateFiles({QStringLiteral("*.dotx")}).isEmpty());
}


// Save As lists .ott, an OpenDocument template: the file is a template by its type, and opens as a new document.
void WriteTests::aDocumentCanBeSavedAsAnOpenDocumentTemplate()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WriteWindow w;
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("Dear reader."));
    const QString path = dir.filePath(QStringLiteral("letter.ott"));
    QVERIFY(w.saveTo(path));
    QMap<QString, QByteArray> parts;
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(jp::readZip(f.readAll(), parts));
    QCOMPARE(parts.value(QStringLiteral("mimetype")), QByteArray("application/vnd.oasis.opendocument.text-template"));
    WriteWindow b;
    QVERIFY(b.openFile(path));
    QVERIFY(b.filePath().isEmpty());
    QVERIFY(b.document()->story()->toPlainText().contains(QStringLiteral("Dear reader.")));
}


// Open and Repair: a document cut short does not open as it is, and opens as a repaired copy with the text that could be read; what was left out is said, and Save asks for a name.
void WriteTests::aDamagedDocxOpensRepairedWithWhatCouldBeRead()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString good = dir.filePath(QStringLiteral("long.docx")), cutPath = dir.filePath(QStringLiteral("cut.docx"));
    {
        WriteWindow a;
        QString text;
        for (int i = 1; i <= 400; ++i) text += QStringLiteral("Paragraph %1 has some words in it, and a few more.\n").arg(i);
        typeText(*a.document(), a.view()->cursor(), text);
        QVERIFY(a.saveTo(good));
    }
    // The damage is made on purpose: the same package with the bytes of one named part, the document's text, cut part way.
    QFile f(good);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(f.readAll(), parts));
    f.close();
    QVERIFY(parts.contains(QStringLiteral("word/document.xml")));
    const qsizetype before = parts[QStringLiteral("word/document.xml")].size();
    parts[QStringLiteral("word/document.xml")].truncate(before * 6 / 10);
    jp::ZipWriter z;
    z.add(QStringLiteral("[Content_Types].xml"), parts.take(QStringLiteral("[Content_Types].xml")), true);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) z.add(it.key(), it.value(), true);
    QFile out(cutPath);
    QVERIFY(out.open(QIODevice::WriteOnly));
    out.write(z.finish());
    out.close();
    const qint64 damagedSize = QFileInfo(cutPath).size();
    WriteWindow w;
    QStringList told;
    w.setNotifier([&](const QString &title, const QString &text) { told << title + QStringLiteral(": ") + text; });
    QVERIFY(w.openFile(cutPath, OpenHow::Repair));
    QVERIFY(w.filePath().isEmpty());
    QVERIFY(w.isModified());
    QVERIFY(w.displayName().contains(QLatin1String("cut")));
    const QString text = w.document()->story()->toPlainText();
    QVERIFY(text.startsWith(QStringLiteral("Paragraph 1 has some words")));
    QVERIFY(text.contains(QStringLiteral("Paragraph 100 has")));
    QVERIFY(text.size() < 400 * 40);   // (the end of the document was cut)
    QVERIFY2(!told.isEmpty() && told.first().contains(QLatin1String("Open and Repair")) && told.first().contains(QLatin1String("word/document.xml")), qPrintable(told.join(QLatin1Char('|'))));
    // The damaged file is as it was, and Save asks where the repaired copy goes.
    const QString other = dir.filePath(QStringLiteral("repaired.docx"));
    w.setSavePathChooser([&](const QString &, const QString &) { return other; });
    QVERIFY(w.save());
    QFile again(cutPath);
    QVERIFY(again.open(QIODevice::ReadOnly));
    QCOMPARE(again.size(), damagedSize);
    WriteWindow b;
    QVERIFY(b.openFile(other));
    QCOMPARE(b.document()->story()->toPlainText(), text);
}


// ---- an old binary file

void WriteTests::anOldBinaryFileOpensAndShowsItsText()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    dochelp::DocWriter doc;
    doc.text = QStringLiteral("Dear reader,\rThis is the old letter.\r");
    doc.chars = {{13, 17, dochelp::sprm1(dochelp::sprm::bold, 1)}};
    const QString path = dir.filePath(QStringLiteral("letter.doc"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(doc.build());
    f.close();
    WriteWindow w;
    QStringList notices;
    w.setNotifier([&](const QString &, const QString &text) { notices << text; });
    QVERIFY(w.openFile(path));
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Dear reader,\nThis is the old letter."));
    QVERIFY(!w.isModified());
    QVERIFY(notices.isEmpty());
    // The title says it is a .doc until it is saved as something else.
    QVERIFY2(w.windowTitle().startsWith(QLatin1String("letter.doc")), qPrintable(w.windowTitle()));
    // (And the layout made pages of it.)
    QVERIFY(w.document()->pager().pageCount() >= 1);
}


void WriteTests::savingAnOldBinaryFileAsksForADocxName()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    dochelp::DocWriter doc;
    doc.text = QStringLiteral("Old text\r");
    const QString path = dir.filePath(QStringLiteral("old.doc"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(doc.build());
    f.close();
    WriteWindow w;
    QStringList notices;
    QString suggested;
    w.setNotifier([&](const QString &, const QString &text) { notices << text; });
    w.setSavePathChooser([&](const QString &name, const QString &) {
        suggested = name;
        return dir.filePath(QStringLiteral("new.docx"));
    });
    QVERIFY(w.openFile(path));
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("!"));
    QVERIFY(w.save());
    // One sentence says why the name is asked for, and the name offered is a .docx beside the old file.
    QCOMPARE(notices.size(), 1);
    QVERIFY2(notices[0].contains(QLatin1String(".docx")) && notices[0].contains(QLatin1String(".doc ")), qPrintable(notices[0]));
    QVERIFY2(suggested.endsWith(QLatin1String("old.docx")), qPrintable(suggested));
    QVERIFY(w.filePath().endsWith(QLatin1String("new.docx")));
    QVERIFY(!w.windowTitle().contains(QLatin1String(".doc")));
    // The old file is as it was, and the new one opens with the edit.
    QFile again(path);
    QVERIFY(again.open(QIODevice::ReadOnly));
    QCOMPARE(again.readAll(), doc.build());
    WriteWindow b;
    QVERIFY(b.openFile(w.filePath()));
    QCOMPARE(b.document()->story()->toPlainText().left(9), QStringLiteral("!Old text"));
}


void WriteTests::whatAnOldFileHoldsThatCannotBeShownIsSaid()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    dochelp::DocWriter doc;
    doc.text = QStringLiteral("a\x01""b\r");
    doc.chars = {{1, 2, dochelp::sprm1(dochelp::sprm::special, 1) + dochelp::sprm4(dochelp::sprm::picture, 9000)}};   // (a picture the file doesn't hold)
    const QString path = dir.filePath(QStringLiteral("odd.doc"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(doc.build());
    f.close();
    WriteWindow w;
    QStringList notices;
    w.setNotifier([&](const QString &, const QString &text) { notices << text; });
    QVERIFY(w.openFile(path));
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("ab"));
    QCOMPARE(notices.size(), 1);
    QVERIFY2(notices[0].contains(QLatin1String("left out")), qPrintable(notices[0]));
    QVERIFY(w.document()->leftSomethingOut());
}


void WriteTests::trackedChangesOfAnOldFileComeInWithTheirAuthorsAndDates()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    dochelp::DocWriter doc;
    // "keep ", then "added" inserted by Ann, " and ", then "dropped" deleted by Bob, then " end."
    doc.text = QStringLiteral("keep added and dropped end.\r");
    doc.revisionAuthors = {QStringLiteral("Ann"), QStringLiteral("Bob")};
    using namespace dochelp;
    doc.chars = {{5, 10, sprm1(sprm::inserted, 0x81) + sprm2(sprm::insertedBy, 0) + sprm4(sprm::insertedAt, dttm(2003, 4, 7, 9, 30))},
                 {15, 22, sprm1(sprm::deleted, 0x81) + sprm2(sprm::deletedBy, 1) + sprm4(sprm::deletedAt, dttm(2004, 11, 23, 16, 5))}};
    const QString path = dir.filePath(QStringLiteral("tracked.doc"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(doc.build());
    f.close();
    WriteWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    QVERIFY(w.openFile(path));
    // The inserted words are in the text, and the deleted ones too (hidden: the lines leave them out; the marks of the changes are objects in it, not text).
    QString shown = w.document()->story()->toPlainText();
    shown.remove(QChar(0xFFFC));
    QVERIFY2(shown.contains(QLatin1String("keep added and dropped end.")), qPrintable(shown));
    // The window shows a document that has changes with its markup: the deleted words are in the lines, struck through. With No Markup they are not.
    QString lines = boxLines(*w.document()).join(QLatin1Char(' '));
    QVERIFY2(lines.contains(QLatin1String("keep added and dropped end.")), qPrintable(lines));
    w.act(QStringLiteral("review.view.none"))->trigger();
    lines = boxLines(*w.document()).join(QLatin1Char(' '));
    QVERIFY2(lines.contains(QLatin1String("keep added and")) && !lines.contains(QLatin1String("dropped")), qPrintable(lines));
    // Written out, each is still a tracked change, with its author and its date.
    const QByteArray x = documentXml(w.document()->toDocx());
    const qsizetype ins = x.indexOf("<w:ins ");
    const qsizetype del = x.indexOf("<w:del ");
    QVERIFY2(ins >= 0 && del > ins, x.constData());
    const QByteArray insTag = x.mid(ins, x.indexOf('>', ins) - ins), delTag = x.mid(del, x.indexOf('>', del) - del);
    QVERIFY2(insTag.contains("w:author=\"Ann\"") && insTag.contains("w:date=\"2003-04-07T09:30:00Z\""), insTag.constData());
    QVERIFY2(delTag.contains("w:author=\"Bob\"") && delTag.contains("w:date=\"2004-11-23T16:05:00Z\""), delTag.constData());
    QVERIFY(x.mid(ins, del - ins).contains("<w:t xml:space=\"preserve\">added</w:t>"));
    QVERIFY(x.mid(del, x.indexOf("</w:del>", del) - del).contains("<w:delText xml:space=\"preserve\">dropped</w:delText>"));
}


void WriteTests::anOldTemplateOpensAsANewDocumentMadeFromIt()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    dochelp::DocWriter doc;
    doc.text = QStringLiteral("Dear <name>,\r");
    const QString path = dir.filePath(QStringLiteral("letter.dot"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(doc.build());
    f.close();
    WriteWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    QVERIFY(w.openFile(path));
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Dear <name>,"));
    QVERIFY(w.filePath().isEmpty());   // (it is not the template that is changed and saved)
    QVERIFY(w.isModified());
}


void WriteTests::aFileThatIsNotADocumentSaysSoAndOpensNothing()
{
    QString why;
    QVERIFY(!WriteDoc::fromFile(QByteArray("plain bytes, no container"), &why));
    QVERIFY(!why.isEmpty());
    // Both kinds are told apart by what is in the file, not its name.
    dochelp::DocWriter old;
    old.text = QStringLiteral("old kind\r");
    QVERIFY(WriteDoc::fromFile(old.build()));
    QVERIFY(WriteDoc::fromFile(WriteDoc::blank()->toDocx()));
}


// ---- Rich Text Format files

QString WriteTests::writeFile(const QTemporaryDir &dir, const QString &name, const QByteArray &bytes)
{
    const QString path = dir.filePath(name);
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) f.write(bytes);
    return path;
}


void WriteTests::anRtfFileOpensByItsContentAndShowsItsText()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QByteArray rtf = "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Arial;}}\\pard Dear reader,\\par {\\b This is the old letter.}\\par "
                           "\\trowd\\cellx2000\\cellx4000\\pard\\intbl one\\cell two\\cell\\row\\pard {\\field{\\*\\fldinst PAGE}{\\fldrslt 7}} \\u8364?\\par}";
    WriteWindow w;
    QStringList notices;
    w.setNotifier([&](const QString &, const QString &text) { notices << text; });
    QVERIFY(w.openFile(writeFile(dir, QStringLiteral("letter.rtf"), rtf)));
    const QString text = w.document()->story()->toPlainText();
    QVERIFY2(text.startsWith(QLatin1String("Dear reader,\nThis is the old letter.\n")), qPrintable(text));
    QVERIFY2(text.contains(QLatin1String("one")) && text.contains(QLatin1String("two")) && text.contains(QString::fromUtf8("\u20AC")), qPrintable(text));
    QVERIFY(!w.isModified());
    QVERIFY(notices.isEmpty());
    QVERIFY2(w.windowTitle().startsWith(QLatin1String("letter.rtf")), qPrintable(w.windowTitle()));
    QVERIFY(w.document()->pager().pageCount() >= 1);
    // The bold run is bold.
    QTextCursor c(w.document()->story());
    c.setPosition(w.document()->story()->toPlainText().indexOf(QLatin1String("This")) + 1);
    QVERIFY(c.charFormat().fontWeight() >= QFont::Bold);
    // The kind is told by what is in the file, not by its name.
    WriteWindow other;
    QVERIFY(other.openFile(writeFile(dir, QStringLiteral("letter.txt"), rtf)));
    QCOMPARE(other.document()->story()->toPlainText(), text);
    QVERIFY(WriteDoc::fromFile(rtf));
}


void WriteTests::savingAnRtfFileWritesAnRtfFileThatReadsBackTheSame()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QByteArray rtf = "{\\rtf1\\ansi Old text\\par}";
    const QString path = writeFile(dir, QStringLiteral("old.rtf"), rtf);
    WriteWindow w;
    QStringList notices;
    w.setNotifier([&](const QString &, const QString &text) { notices << text; });
    QVERIFY(w.openFile(path));
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("!"));
    QVERIFY(w.save());
    QVERIFY(notices.isEmpty());   // (nothing in it that the format can't hold)
    QCOMPARE(w.filePath(), path);
    QFile again(path);
    QVERIFY(again.open(QIODevice::ReadOnly));
    const QByteArray saved = again.readAll();
    QVERIFY2(saved.startsWith("{\\rtf1\\ansi") && saved != rtf, saved.constData());
    WriteWindow b;
    QVERIFY(b.openFile(path));
    QCOMPARE(b.document()->story()->toPlainText().left(9), QStringLiteral("!Old text"));
}


// Document files dropped on the pages open; a picture dropped there is put in the text; and several files at once each open.
void WriteTests::droppingDocumentFilesOpensThemAndAPictureStaysAPicture()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString one = writeFile(dir, QStringLiteral("one.rtf"), "{\\rtf1\\ansi First dropped\\par}");
    const QString two = writeFile(dir, QStringLiteral("two.rtf"), "{\\rtf1\\ansi Second dropped\\par}");
    const QString pic = dir.filePath(QStringLiteral("pic.png"));
    QImage image(8, 8, QImage::Format_RGB32);
    image.fill(Qt::red);
    QVERIFY(image.save(pic));
    auto windows = [] {
        int n = 0;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (qobject_cast<WriteWindow *>(top)) ++n;
        return n;
    };
    auto drop = [](QWidget *target, const QStringList &paths) {
        QMimeData mime;
        QList<QUrl> urls;
        for (const QString &path : paths) urls << QUrl::fromLocalFile(path);
        mime.setUrls(urls);
        QDragEnterEvent enter(QPoint(10, 10), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(target, &enter);
        const bool accepted = enter.isAccepted();
        QDropEvent dropped(QPointF(10, 10), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(target, &dropped);
        return accepted;
    };
    WriteWindow w;
    w.show();
    const int base = windows();
    // One .rtf: opened in this window, which was empty and untouched.
    QVERIFY(drop(w.view()->viewport(), {one}));
    QTRY_VERIFY2(windows() > base || w.document()->story()->toPlainText().contains(QLatin1String("First dropped")), "nothing opened");
    QVERIFY2(w.document()->story()->toPlainText().contains(QLatin1String("First dropped")), qPrintable(QStringLiteral("windows %1 base %2").arg(windows()).arg(base)));
    QCOMPARE(windows(), base);
    // A picture: put in the text, and no window opens.
    QVERIFY(drop(w.view()->viewport(), {pic}));
    QCOMPARE(windows(), base);
    QVERIFY(w.isModified());
    // Two files together: a window for each.
    QVERIFY(drop(w.view()->viewport(), {one, two}));
    QTRY_COMPARE(windows(), base + 2);
    // Something that is neither is not taken.
    QVERIFY(!drop(w.view()->viewport(), {dir.filePath(QStringLiteral("none.xyz"))}));
    QCOMPARE(windows(), base + 2);
    QList<WriteWindow *> extra;
    for (QWidget *top : QApplication::topLevelWidgets())
        if (auto *ww = qobject_cast<WriteWindow *>(top); ww && ww != &w) extra << ww;
    for (WriteWindow *ww : extra) ww->close();
}


// A field's instruction is never run: opening a document with one that names an address on this machine reaches nothing, and no picture comes of it.
void WriteTests::aFieldThatNamesAPictureOrAnAddressIsNeverFollowed()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    const QByteArray url = "http://127.0.0.1:" + QByteArray::number(server.serverPort()) + "/x.png";
    const QString path = writeFile(dir, QStringLiteral("fields.rtf"),
                                   "{\\rtf1\\ansi\\pard {\\field{\\*\\fldinst INCLUDEPICTURE \"" + url + "\"}{\\fldrslt shown}} {\\field{\\*\\fldinst HYPERLINK \"" + url + "\"}{\\fldrslt link}}\\par}");
    WriteWindow w;
    QVERIFY(w.openFile(path));
    QVERIFY(w.document()->pager().pageCount() >= 1);
    QVERIFY(!server.waitForNewConnection(300));
    QVERIFY(w.document()->story()->toPlainText().contains(QLatin1String("shown")));
}


void WriteTests::anRtfFileWithPartsTheLimitsCutSaysSoAndKeepsTheRest()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QByteArray rtf = "{\\rtf1\\ansi Kept text ";
    rtf += "{\\pict\\pngblip 0102}\\par}";
    WriteWindow w;
    QStringList notices;
    w.setNotifier([&](const QString &, const QString &text) { notices << text; });
    QVERIFY(w.openFile(writeFile(dir, QStringLiteral("cut.rtf"), rtf)));
    QVERIFY(w.document()->story()->toPlainText().contains(QLatin1String("Kept text")));
    QVERIFY(w.document()->leftSomethingOut());
    QCOMPARE(notices.size(), 1);
}


void WriteTests::unsavedWorkIsKeptAndComesBack()
{
    QTemporaryDir dir;
    recovery::setRoot(dir.path());
    recovery::setCopySuffix(QStringLiteral(".docx"));
    WriteWindow a;
    QVERIFY(a.recoveryCopy().isEmpty());
    a.autoRecover();   // (nothing is changed yet: no copy)
    QVERIFY(a.recoveryCopy().isEmpty());
    typeText(*a.document(), a.view()->cursor(), QStringLiteral("Work that is not saved"));
    a.autoRecover();
    QTRY_VERIFY(!a.recoveryCopy().isEmpty());   // (made on a worker thread)
    QVERIFY(QFileInfo::exists(a.recoveryCopy()));
    const QString copy = a.recoveryCopy();
    // A run that crashed leaves its copy: it opens as unsaved work, and the old copy goes once this run has one of its own.
    recovery::Recovered r;
    r.file = copy;
    r.title = QStringLiteral("Lost letter");
    r.saved = QDateTime::currentDateTime();
    WriteWindow b;
    WriteWindow *opened = b.openRecovered(r);
    QVERIFY(opened);
    QCOMPARE(opened->document()->story()->toPlainText(), QStringLiteral("Work that is not saved"));
    QVERIFY(opened->isModified());
    QCOMPARE(opened->displayName(), QStringLiteral("Lost letter"));
    QVERIFY(!opened->recoveryCopy().isEmpty());
    // Saving takes the copy away.
    opened->setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("letter.docx")); });
    const QString second = opened->recoveryCopy();
    QVERIFY(opened->save());
    QVERIFY(!QFileInfo::exists(second));
    QVERIFY(opened->recoveryCopy().isEmpty());
}

// The copy is made on a worker thread: the window's thread only takes what is written, a second tick while one is being made is skipped, and what is written opens and matches the document.
void WriteTests::aRecoveryCopyIsMadeOffTheWindowsThread()
{
    QTemporaryDir dir;
    recovery::setRoot(dir.path());
    recovery::setCopySuffix(QStringLiteral(".docx"));
    {
        auto gate = std::make_shared<RecoveryGate>();
        const auto letGo = qScopeGuard([gate] { gate->open(); });
        WriteWindow w;
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Kept in the background"));
        w.setRecoveryHook(RecoveryGate::hook(gate));
        w.autoRecover();
        QVERIFY(w.recoveryBusy());   // (the window is free while the copy is made)
        QVERIFY(w.recoveryCopy().isEmpty());
        w.autoRecover();   // (skipped: one is being made)
        gate->open();
        QTRY_VERIFY(!w.recoveryBusy());
        QCOMPARE(int(gate->made), 1);
        QVERIFY(gate->madeOn != nullptr);
        QVERIFY(gate->madeOn != QThread::currentThread());
        QFile f(w.recoveryCopy());
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray kept = f.readAll();
        auto back = WriteDoc::fromDocx(kept);
        QVERIFY(back);
        QCOMPARE(back->story()->toPlainText(), QStringLiteral("Kept in the background"));
        QVERIFY(kept == w.document()->toDocx());   // (the bytes the window itself would save)
        w.document()->setModified(false);
    }
    recovery::setRoot(QString());
}

// A save while a copy is being made leaves no copy: the one being made is never written. The same when the window is closed (deleted) meanwhile.
void WriteTests::aSaveWhileACopyIsBeingMadeLeavesNoCopy()
{
    QTemporaryDir dir;
    QTemporaryDir root;
    recovery::setRoot(root.path());
    recovery::setCopySuffix(QStringLiteral(".docx"));
    auto none = [&] {
        QDirIterator left(root.path(), {QStringLiteral("*.docx"), QStringLiteral("*.json")}, QDir::Files, QDirIterator::Subdirectories);
        return !left.hasNext();
    };
    {
        auto gate = std::make_shared<RecoveryGate>();
        const auto letGo = qScopeGuard([gate] { gate->open(); });
        WriteWindow w;
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Saved meanwhile"));
        w.setRecoveryHook(RecoveryGate::hook(gate));
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("a.docx")); });
        w.autoRecover();
        QVERIFY(w.recoveryBusy());
        QVERIFY(w.save());
        QVERIFY(!w.recoveryBusy());
        gate->open();
        recovery::AsyncWriter::waitForAll();
        QCoreApplication::processEvents();
        QVERIFY(w.recoveryCopy().isEmpty());
        QVERIFY(none());
    }
    {
        auto gate = std::make_shared<RecoveryGate>();
        const auto letGo = qScopeGuard([gate] { gate->open(); });
        auto *w = new WriteWindow;
        typeText(*w->document(), w->view()->cursor(), QStringLiteral("Closed meanwhile"));
        w->setRecoveryHook(RecoveryGate::hook(gate));
        w->autoRecover();
        QVERIFY(w->recoveryBusy());
        delete w;
        gate->open();
        recovery::AsyncWriter::waitForAll();
        QCoreApplication::processEvents();
        QVERIFY(none());
    }
    recovery::setRoot(QString());
}

// The other order: the copy is written, and the window has not yet heard of it when the work is saved. The written copy goes.
void WriteTests::aSaveAfterACopyIsWrittenRemovesItBeforeItIsReported()
{
    QTemporaryDir dir, root;
    recovery::setRoot(root.path());
    recovery::setCopySuffix(QStringLiteral(".docx"));
    {
        WriteWindow w;
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Saved after"));
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("a.docx")); });
        w.autoRecover();
        // (waiting without running the event loop: the window is not told)
        QElapsedTimer waited;
        waited.start();
        bool written = false;
        while (!written && waited.elapsed() < 60000) {
            QDirIterator it(root.path(), {QStringLiteral("*.docx")}, QDir::Files, QDirIterator::Subdirectories);
            written = it.hasNext();
            if (!written) QThread::msleep(2);
        }
        QVERIFY(written);
        QVERIFY(w.recoveryBusy());
        QVERIFY(w.save());
        recovery::AsyncWriter::waitForAll();
        QCoreApplication::processEvents();
        QDirIterator left(root.path(), {QStringLiteral("*.docx"), QStringLiteral("*.json")}, QDir::Files, QDirIterator::Subdirectories);
        QVERIFY(!left.hasNext());
        QVERIFY(w.recoveryCopy().isEmpty());
    }
    recovery::setRoot(QString());
}

void WriteTests::theParagraphDialogChangesWhatWasChanged()
{
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("one\ntwo"));
    w.view()->selectAll();
    withDialog([&w] { w.act("para.dialog")->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *dlg) {
        auto *alignment = dlg->findChild<QComboBox *>(QStringLiteral("alignment"));
        auto *left = dlg->findChild<QDoubleSpinBox *>(QStringLiteral("left"));
        auto *lineSpacing = dlg->findChild<QComboBox *>(QStringLiteral("lineSpacing"));
        QVERIFY(alignment && left && lineSpacing);
        alignment->setCurrentIndex(1);
        left->setValue(36);
        lineSpacing->setCurrentIndex(2);
    });
    for (int i = 0; i < 2; ++i) {
        const QTextBlockFormat bf = w.document()->story()->findBlockByNumber(i).blockFormat();
        QCOMPARE(bf.alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
        QCOMPARE(bf.leftMargin(), 36.0);
        QCOMPARE(bf.lineHeight(), 200.0);
        QCOMPARE(bf.rightMargin(), 0.0);   // (not changed: left as it was)
    }
    withDialog([&w] { w.act("zoom.dialog")->trigger(); }, QStringLiteral("zoomDialog"), [](QDialog *dlg) {
        auto *percent = dlg->findChild<QSpinBox *>(QStringLiteral("percent"));
        QVERIFY(percent);
        percent->setValue(150);
        for (auto *r : dlg->findChildren<QRadioButton *>())
            if (r->property("zoomChoice").toInt() == 0) r->setChecked(true);
    });
    QCOMPARE(w.view()->zoom(), 1.5);
}


// ---------------------------------------------------------------- Find and Replace
// Home > Editing has Find (Ctrl+F) and Replace (Ctrl+H); each opens the pane on the right.
void WriteTests::homeTabHasFindAndReplace()
{
    WriteWindow w;
    QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
    QCOMPARE(w.act(QStringLiteral("edit.find"))->shortcut(), QKeySequence(QKeySequence::Find));
    QCOMPARE(w.act(QStringLiteral("edit.replace"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_H));
    int buttons = 0;
    for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
        if (b->defaultAction() == w.act(QStringLiteral("edit.find")) || b->defaultAction() == w.act(QStringLiteral("edit.replace"))) ++buttons;
    QCOMPARE(buttons, 2);
    QVERIFY(!w.findPane());
    w.show();
    w.act(QStringLiteral("edit.find"))->trigger();
    QVERIFY(w.findPane() && w.findPane()->isVisible());
    QVERIFY(!w.findPane()->matchCase()->isChecked() && !w.findPane()->wholeWords()->isChecked());
    w.act(QStringLiteral("edit.replace"))->trigger();
    QVERIFY(w.findPane()->isVisible());
    // The boxes are named for screen readers.
    for (QLineEdit *e : w.findPane()->findChildren<QLineEdit *>()) {
        QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(e);
        QVERIFY(iface);
        QVERIFY2(!iface->text(QAccessible::Name).trimmed().isEmpty(), qPrintable(e->objectName()));
    }
}

// The same command has the same name in all four apps: Spelling (JeffWrite has no grammar check), Word Count, Find's two options, and a quick toolbar that starts with Save, Undo, and Redo.
void WriteTests::commandsUseTheSuitesWords()
{
    WriteWindow w;
    w.resize(1000, 700);
    w.show();
    QCOMPARE(w.act(QStringLiteral("review.spelling"))->text(), QStringLiteral("Spelling"));
    QCOMPARE(w.act(QStringLiteral("review.wordCount"))->text(), QStringLiteral("Word Count"));
    w.act(QStringLiteral("edit.find"))->trigger();
    QVERIFY(w.findPane());
    QCOMPARE(w.findPane()->matchCase()->text(), QStringLiteral("Match case"));
    QCOMPARE(w.findPane()->wholeWords()->text(), QStringLiteral("Find whole words only"));
    QList<QAction *> quick;
    for (QToolButton *b : w.ribbon()->quickAccessButtons()) quick << b->defaultAction();
    QCOMPARE(quick.mid(0, 3), (QList<QAction *>{w.act(QStringLiteral("file.save")), w.act(QStringLiteral("edit.undo")), w.act(QStringLiteral("edit.redo"))}));
}


// Find Next selects the match and scrolls the pages to it, also when the page it is on has not been laid out yet; after the last match it
// goes round to the first; and with none it says so and leaves the selection where it was.
void WriteTests::findNextSelectsTheMatchAndScrollsToIt()
{
    WriteWindow w;
    w.resize(1100, 800);
    w.setDocument(open(makeDocx(paragraphs(300) + "<w:p><w:r><w:t>The zebrafish swims.</w:t></w:r></w:p>" + paragraphs(40), kStyles)), QString());
    w.show();
    w.showFindPane();
    WriteFindPane *pane = w.findPane();
    QCOMPARE(w.view()->verticalScrollBar()->value(), 0);
    // The match is past the pages laid out so far (so that finding it has to lay out the pages as far as it).
    const int matchAt = w.document()->story()->toPlainText().indexOf(QStringLiteral("zebrafish"));
    const int known = w.view()->pageCount();
    QVERIFY(matchAt > 0 && known >= 1);
    QVERIFY(!w.view()->pagesComplete());
    QVERIFY2(w.document()->pager().page(known - 1).start.pos <= matchAt, qPrintable(QStringLiteral("%1 pages known, the last from %2, the match at %3").arg(known).arg(w.document()->pager().page(known - 1).start.pos).arg(matchAt)));
    pane->findBox()->setText(QStringLiteral("zebrafish"));
    QVERIFY(pane->findNext());
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("zebrafish"));
    QVERIFY(pane->status().startsWith(QLatin1String("Match ")));   // (it says which match it is on)
    QVERIFY(w.view()->pageCount() > known);
    QVERIFY(w.view()->currentPage() > 5);
    QVERIFY(w.view()->verticalScrollBar()->value() > 0);
    QRect r;
    QVERIFY(w.view()->caretRect(&r));
    QVERIFY2(r.top() >= 0 && r.bottom() <= w.view()->viewport()->height(), qPrintable(QStringLiteral("%1..%2 in %3").arg(r.top()).arg(r.bottom()).arg(w.view()->viewport()->height())));
    // The only match, again: round to it.
    const int at = w.view()->cursor().selectionStart();
    QVERIFY(pane->findNext());
    QCOMPARE(w.view()->cursor().selectionStart(), at);
    // From the start of the document, the first match is the nearest.
    QTextCursor c(w.document()->story());
    w.view()->setCursor(c);
    QVERIFY(pane->findNext());
    QCOMPARE(w.view()->cursor().selectionStart(), at);
    // Nothing found: said, and nothing moves.
    pane->findBox()->setText(QStringLiteral("<b>narwhal"));
    QVERIFY(!pane->findNext());
    QVERIFY2(pane->status().contains(QStringLiteral("\"<b>narwhal\" was not found")), qPrintable(pane->status()));
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("zebrafish"));
    QCOMPARE(w.view()->cursor().selectionStart(), at);
}


void WriteTests::findFollowsMatchCaseAndWholeWords()
{
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("Cat cat concat CAT."));
    w.showFindPane();
    WriteFindPane *pane = w.findPane();
    pane->findBox()->setText(QStringLiteral("cat"));
    // The places the matches start, from the start of the document, round to the first again.
    auto starts = [&] {
        QTextCursor c(w.document()->story());
        w.view()->setCursor(c);
        QList<int> out;
        for (int i = 0; i < 8; ++i) {
            if (!pane->findNext()) break;
            const int at = w.view()->cursor().selectionStart();
            if (!out.isEmpty() && at == out.first()) break;
            out << at;
        }
        return out;
    };
    QCOMPARE(starts(), (QList<int>{0, 4, 11, 15}));
    pane->matchCase()->setChecked(true);
    QCOMPARE(starts(), (QList<int>{4, 11}));
    pane->wholeWords()->setChecked(true);
    QCOMPARE(starts(), (QList<int>{4}));
    pane->matchCase()->setChecked(false);
    QCOMPARE(starts(), (QList<int>{0, 4, 15}));
}


// The text of a table's cells is in the story, so it is found there; text that is hidden is not shown, and so it is not found.
void WriteTests::findReachesCellsAndPassesHiddenText()
{
    WriteWindow w;
    w.setDocument(open(tableDocx(2, 2)), QString());
    w.show();
    w.showFindPane();
    WriteFindPane *pane = w.findPane();
    pane->findBox()->setText(QStringLiteral("r1c1"));
    QVERIFY(pane->findNext());
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("r1c1"));
    QCOMPARE(w.view()->cursor().block().text(), QStringLiteral("r1c1"));
    // "hide" is hidden in its first place.
    w.setDocument(WriteDoc::blank(), QString());
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("hide and seek, hide"));
    selectRange(w, 0, 4);
    changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) { f.setProperty(tp::HiddenText, true); });
    QTextCursor c(w.document()->story());
    w.view()->setCursor(c);
    pane->findBox()->setText(QStringLiteral("hide"));
    QVERIFY(pane->findNext());
    QCOMPARE(w.view()->cursor().selectionStart(), 15);
    pane->replaceBox()->setText(QStringLiteral("show"));
    pane->replaceAll();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("hide and seek, show"));
    QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 1 replacement."));
}


// Replace swaps the match that is selected and goes on to the next; when what is selected is not a match, it shows the next match first.
void WriteTests::replaceReplacesOneMatchAndGoesToTheNext()
{
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("cat cat cat"));
    QTextCursor c(w.document()->story());
    w.view()->setCursor(c);
    w.showFindPane(true);
    WriteFindPane *pane = w.findPane();
    pane->findBox()->setText(QStringLiteral("cat"));
    pane->replaceBox()->setText(QStringLiteral("dog"));
    pane->replaceOne();   // (nothing is selected: it shows the first)
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("cat cat cat"));
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("cat"));
    QCOMPARE(w.view()->cursor().selectionStart(), 0);
    pane->replaceOne();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("dog cat cat"));
    QCOMPARE(w.view()->cursor().selectionStart(), 4);
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("cat"));
    pane->replaceOne();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("dog dog cat"));
    // Each swap undoes on its own.
    w.view()->undo();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("dog cat cat"));
    w.view()->undo();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("cat cat cat"));
    // A selection that is not a match is left alone: the next match is shown.
    selectRange(w, 4, 7);
    pane->findBox()->setText(QStringLiteral("t c"));
    pane->replaceOne();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("cat cat cat"));
    QCOMPARE(w.view()->cursor().selectionStart(), 2);
    // Words that the options do not match are not swapped either.
    pane->findBox()->setText(QStringLiteral("at"));
    pane->wholeWords()->setChecked(true);
    selectRange(w, 1, 3);
    pane->replaceOne();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("cat cat cat"));
    QVERIFY2(!pane->status().isEmpty(), "none of the words is \"at\"");
}


// Replace All is one step that undoes, says how many it swapped, and leaves what the new words hold alone; the new words look as the old ones did.
void WriteTests::replaceAllIsOneStepThatUndoesAndSaysHowMany()
{
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("a cat sat; the cat's hat, a cat."));
    selectRange(w, 15, 18);
    toggleBold(*w.document(), w.view()->cursor());   // the second "cat" is bold
    w.document()->story()->clearUndoRedoStacks();
    w.showFindPane(true);
    WriteFindPane *pane = w.findPane();
    const QString before = w.document()->story()->toPlainText();
    pane->findBox()->setText(QStringLiteral("cat"));
    pane->replaceBox()->setText(QStringLiteral("dog"));
    pane->replaceAll();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("a dog sat; the dog's hat, a dog."));
    QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 3 replacements."));
    QTextCursor probe(w.document()->story());
    probe.setPosition(17);
    QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Bold));
    probe.setPosition(5);
    QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Normal));
    // One step undoes all three, and one redoes them.
    w.view()->undo();
    QCOMPARE(w.document()->story()->toPlainText(), before);
    w.view()->redo();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("a dog sat; the dog's hat, a dog."));
    w.view()->undo();
    QVERIFY2(!w.document()->story()->isUndoAvailable(), "the three were one step");
    // A match of several looks gives its first one to the new words: "AB" is bold at the A and plain at the B.
    w.setDocument(WriteDoc::blank(), QString());
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("x AB y"));
    selectRange(w, 2, 3);
    toggleBold(*w.document(), w.view()->cursor());
    pane->findBox()->setText(QStringLiteral("ab"));
    pane->replaceBox()->setText(QStringLiteral("zz"));
    pane->replaceAll();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("x zz y"));
    probe = QTextCursor(w.document()->story());
    probe.setPosition(3);
    QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Bold));
    probe.setPosition(4);
    QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Bold));
    probe.setPosition(5);
    QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Normal));
    // The words that were put in are not searched again: "a" for "aa" ends; and nothing as the new words deletes.
    w.setDocument(WriteDoc::blank(), QString());
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("banana"));
    pane->findBox()->setText(QStringLiteral("a"));
    pane->replaceBox()->setText(QStringLiteral("aa"));
    pane->replaceAll();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("baanaanaa"));
    pane->findBox()->setText(QStringLiteral("aa"));
    pane->replaceBox()->clear();
    pane->replaceAll();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("bnn"));
    // None found: it says so, and the document is not touched.
    w.document()->setModified(false);
    pane->findBox()->setText(QStringLiteral("zzz"));
    pane->replaceAll();
    QVERIFY2(pane->status().contains(QStringLiteral("made no replacements. \"zzz\" was not found")), qPrintable(pane->status()));
    QVERIFY(!w.document()->isModified());
    pane->findBox()->setText(QStringLiteral("bnn"));
    pane->replaceBox()->setText(QStringLiteral("x"));
    pane->replaceAll();
    QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 1 replacement."));
}


// ---------------------------------------------------------------- the Font dialog
// The Font group's launcher and Ctrl+D open the dialog.
void WriteTests::theFontLauncherAndTheKeyOpenTheDialog()
{
    WriteWindow w;
    QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
    QCOMPARE(w.act(QStringLiteral("fmt.fontDialog"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_D));
    QToolButton *launcher = nullptr;
    for (RibbonGroup *g : w.ribbon()->tab(QStringLiteral("Home"))->groups())
        if (g->title() == QLatin1String("Font")) launcher = g->launcher();
    QVERIFY(launcher);
    QVERIFY(!keytip(launcher).isEmpty());
    QString title;
    withDialog([launcher] { launcher->click(); }, QStringLiteral("fontDialog"), [&title](QDialog *d) { title = d->windowTitle(); }, false);
    QCOMPARE(title, QStringLiteral("Font"));
}


// The dialog shows the look of the selection: the first characters' where they differ, and a box for an effect that only some of the
// characters have is neither checked nor clear. Cancel changes nothing.
void WriteTests::theFontDialogShowsTheLookOfTheSelection()
{
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("Some words here"));
    selectRange(w, 5, 10);
    toggleBold(*w.document(), w.view()->cursor());
    toggleItalic(*w.document(), w.view()->cursor());
    setFontSize(*w.document(), w.view()->cursor(), 20);
    setTextColor(*w.document(), w.view()->cursor(), QColor(0xC0, 0, 0));
    changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) {
        f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        f.setProperty(tp::UnderlineKind, 1);   // double
        f.setUnderlineColor(QColor(0, 0, 0xFF));
        f.setFontStrikeOut(true);
        f.setProperty(prop::DoubleStrike, true);
        f.setVerticalAlignment(QTextCharFormat::AlignSubScript);
        f.setFontCapitalization(QFont::SmallCaps);
        f.setProperty(tp::HiddenText, true);
    });
    w.document()->story()->clearUndoRedoStacks();
    const QString was = w.document()->story()->toHtml();
    bool shown = false;
    withFontDialog(w, [&](QDialog *d) {
        QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("fontStyle"))->currentIndex(), 3);   // bold italic
        QCOMPARE(d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->value(), 20.0);
        QCOMPARE(d->findChild<ColorButton *>(QStringLiteral("fontColor"))->current().rgbValue(), QColor(0xC0, 0, 0));
        QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->currentText(), QStringLiteral("Double"));
        QCOMPARE(d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->current().rgbValue(), QColor(0, 0, 0xFF));
        for (const char *on : {"doubleStrikethrough", "subscript", "smallCaps", "hidden"}) QCOMPARE(d->findChild<QCheckBox *>(QString::fromLatin1(on))->checkState(), Qt::Checked);
        for (const char *off : {"strikethrough", "superscript", "allCaps"}) QCOMPARE(d->findChild<QCheckBox *>(QString::fromLatin1(off))->checkState(), Qt::Unchecked);
        // (Double strikethrough and single strikethrough are not both on: choosing one clears the other.)
        d->findChild<QCheckBox *>(QStringLiteral("strikethrough"))->setChecked(true);
        QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("doubleStrikethrough"))->checkState(), Qt::Unchecked);
        shown = true;
    }, false);
    QVERIFY(shown);
    QCOMPARE(w.document()->story()->toHtml(), was);
    QVERIFY(!w.document()->story()->isUndoAvailable());
    // Over "Some words": half of it has each look, and what differs shows as nothing (the effects as partly checked).
    selectRange(w, 0, 10);
    shown = false;
    withFontDialog(w, [&](QDialog *d) {
        QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("fontStyle"))->currentIndex(), -1);
        QVERIFY(d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->text().trimmed().isEmpty());
        QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->currentIndex(), -1);
        QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("subscript"))->checkState(), Qt::PartiallyChecked);
        QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("hidden"))->checkState(), Qt::PartiallyChecked);
        QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("superscript"))->checkState(), Qt::Unchecked);
        shown = true;
    }, false);
    QVERIFY(shown);
}


// OK gives the selection what was changed, as one step that undoes; what was not touched stays as each character had it.
void WriteTests::theFontDialogAppliesWhatWasChangedAsOneStep()
{
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("Some words here"));
    selectRange(w, 5, 10);
    toggleBold(*w.document(), w.view()->cursor());
    w.document()->story()->clearUndoRedoStacks();
    selectRange(w, 0, 10);   // (the bold "words" with the plain "Some ")
    withFontDialog(w, [](QDialog *d) {
        Q_EMIT d->findChild<FontCombo *>(QStringLiteral("fontName"))->familyChosen(QStringLiteral("Cousine"));
        d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->setValue(18);
        d->findChild<ColorButton *>(QStringLiteral("fontColor"))->setCurrent(ColorRef::rgb(QColor(0, 0x80, 0)));
        d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->setCurrentIndex(5);   // dotted
        d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->setCurrent(ColorRef::rgb(QColor(0xFF, 0, 0)));
        d->findChild<QCheckBox *>(QStringLiteral("strikethrough"))->setChecked(true);
        d->findChild<QCheckBox *>(QStringLiteral("superscript"))->setChecked(true);
        d->findChild<QCheckBox *>(QStringLiteral("smallCaps"))->setChecked(true);
    });
    auto at = [&](int position) {
        QTextCursor c(w.document()->story());
        c.setPosition(position + 1);
        return c.charFormat();
    };
    for (int position : {0, 6}) {
        const QTextCharFormat f = at(position);
        QCOMPARE(f.fontFamilies().toStringList().value(0), QStringLiteral("Cousine"));
        QCOMPARE(f.fontPointSize(), 18.0);
        QCOMPARE(f.foreground().color(), QColor(0, 0x80, 0));
        QCOMPARE(f.underlineStyle(), QTextCharFormat::DotLine);
        QCOMPARE(f.underlineColor(), QColor(0xFF, 0, 0));
        QVERIFY(f.fontStrikeOut() && !f.boolProperty(prop::DoubleStrike));
        QCOMPARE(f.verticalAlignment(), QTextCharFormat::AlignSuperScript);
        QCOMPARE(f.fontCapitalization(), QFont::SmallCaps);
    }
    // The style was not touched: "words" is still bold, and "Some " is not.
    QCOMPARE(at(6).fontWeight(), int(QFont::Bold));
    QCOMPARE(at(0).fontWeight(), int(QFont::Normal));
    QVERIFY(at(11).fontPointSize() != 18.0);   // (nothing past the selection)
    // One undo takes all of it back; one redo gives it again.
    w.view()->undo();
    for (int position : {0, 6}) {
        const QTextCharFormat f = at(position);
        QVERIFY(f.fontFamilies().toStringList().value(0) != QLatin1String("Cousine"));
        QVERIFY(f.fontPointSize() != 18.0);
        QCOMPARE(f.underlineStyle(), QTextCharFormat::NoUnderline);
        QVERIFY(!f.fontStrikeOut());
        QCOMPARE(f.verticalAlignment(), QTextCharFormat::AlignNormal);
        QCOMPARE(f.fontCapitalization(), QFont::MixedCase);
    }
    QCOMPARE(at(6).fontWeight(), int(QFont::Bold));
    QVERIFY(!w.document()->story()->isUndoAvailable());
    w.view()->redo();
    QCOMPARE(at(0).fontPointSize(), 18.0);
    QCOMPARE(at(6).underlineStyle(), QTextCharFormat::DotLine);
    // With no selection the choices are for what is typed next.
    QTextCursor end(w.document()->story());
    end.movePosition(QTextCursor::End);
    w.view()->setCursor(end);
    withFontDialog(w, [](QDialog *d) {
        d->findChild<QComboBox *>(QStringLiteral("fontStyle"))->setCurrentIndex(2);
        d->findChild<QCheckBox *>(QStringLiteral("allCaps"))->setChecked(true);
    });
    insertText(*w.document(), w.view()->cursor(), QStringLiteral("!"));
    const QTextCharFormat typed = at(w.document()->story()->characterCount() - 2);
    QCOMPARE(typed.fontWeight(), int(QFont::Bold));
    QCOMPARE(typed.fontCapitalization(), QFont::AllUppercase);
    QCOMPARE(at(w.document()->story()->characterCount() - 3).fontCapitalization(), QFont::MixedCase);
}


// Each effect set in the dialog is written to the file as the element it is and comes back as the same effect, on its words alone.
void WriteTests::aFontEffectIsSavedInTheFileAndComesBack_data()
{
    QTest::addColumn<QString>("box");
    QTest::addColumn<QString>("xml");
    QTest::newRow("strikethrough") << "strikethrough" << "<w:strike/>";
    QTest::newRow("double strikethrough") << "doubleStrikethrough" << "<w:dstrike/>";
    QTest::newRow("superscript") << "superscript" << "<w:vertAlign w:val=\"superscript\"/>";
    QTest::newRow("subscript") << "subscript" << "<w:vertAlign w:val=\"subscript\"/>";
    QTest::newRow("small caps") << "smallCaps" << "<w:smallCaps/>";
    QTest::newRow("all caps") << "allCaps" << "<w:caps/>";
    QTest::newRow("hidden") << "hidden" << "<w:vanish/>";
}


void WriteTests::aFontEffectIsSavedInTheFileAndComesBack()
{
    QFETCH(QString, box);
    QFETCH(QString, xml);
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("one two three"));
    selectRange(w, 4, 7);
    withFontDialog(w, [&](QDialog *d) {
        d->findChild<QCheckBox *>(box)->setChecked(true);
    });
    auto has = [&](WriteDoc &doc, int position) {
        QTextCursor c(doc.story());
        c.setPosition(position + 1);
        const QTextCharFormat f = c.charFormat();
        if (box == QLatin1String("strikethrough")) return f.fontStrikeOut() && !f.boolProperty(prop::DoubleStrike);
        if (box == QLatin1String("doubleStrikethrough")) return f.fontStrikeOut() && f.boolProperty(prop::DoubleStrike);
        if (box == QLatin1String("superscript")) return f.verticalAlignment() == QTextCharFormat::AlignSuperScript;
        if (box == QLatin1String("subscript")) return f.verticalAlignment() == QTextCharFormat::AlignSubScript;
        if (box == QLatin1String("smallCaps")) return f.fontCapitalization() == QFont::SmallCaps;
        if (box == QLatin1String("allCaps")) return f.fontCapitalization() == QFont::AllUppercase;
        return f.boolProperty(tp::HiddenText);
    };
    QVERIFY(has(*w.document(), 4) && !has(*w.document(), 0) && !has(*w.document(), 8));
    const QByteArray zip = w.document()->toDocx();
    QVERIFY(rt::strictProblems(zip).isEmpty());
    const QByteArray x = documentXml(zip);
    QVERIFY2(runPropsOfText(x, QStringLiteral("two")).contains(xml), x.constData());
    QVERIFY2(runPropsOfText(x, QStringLiteral("one ")).isEmpty() && runPropsOfText(x, QStringLiteral(" three")).isEmpty(), x.constData());
    auto back = open(zip);
    QVERIFY(back);
    QCOMPARE(back->story()->toPlainText(), QStringLiteral("one two three"));
    QVERIFY(has(*back, 4) && !has(*back, 0) && !has(*back, 8));
    // Written once more, the same element.
    QVERIFY(runPropsOfText(documentXml(back->toDocx()), QStringLiteral("two")).contains(xml));
    // And turned off in the dialog, it is gone from the file.
    WriteWindow v;
    v.show();
    v.setDocument(std::move(back), QString());
    selectRange(v, 4, 7);
    withFontDialog(v, [&](QDialog *d) {
        QCOMPARE(d->findChild<QCheckBox *>(box)->checkState(), Qt::Checked);
        d->findChild<QCheckBox *>(box)->setChecked(false);
    });
    QVERIFY(!has(*v.document(), 4));
    QVERIFY2(!documentXml(v.document()->toDocx()).contains(xml.toUtf8()), documentXml(v.document()->toDocx()).constData());
}


// The underline's style and color are written as w:u with its val and color, and read back into the dialog as they were set.
void WriteTests::anUnderlineStyleAndColorFromTheDialogAreSavedInTheFileAndComeBack()
{
    // The choices of the dialog, in its order, and the val the file has for each.
    const QStringList vals = {QStringLiteral("none"), QStringLiteral("single"), QStringLiteral("words"), QStringLiteral("double"), QStringLiteral("thick"), QStringLiteral("dotted"),
                              QStringLiteral("dash"), QStringLiteral("dotDash"), QStringLiteral("dotDotDash"), QStringLiteral("wave"), QStringLiteral("wavyDouble")};
    WriteWindow w;
    w.show();
    QString text;
    for (int i = 1; i < vals.size(); ++i) text += QStringLiteral("w%1 ").arg(i, 2, 10, QLatin1Char('0'));
    typeText(*w.document(), w.view()->cursor(), text.trimmed());
    for (int i = 1; i < vals.size(); ++i) {
        selectRange(w, (i - 1) * 4, (i - 1) * 4 + 3);
        withFontDialog(w, [&](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->setCurrentIndex(i);
            // (Colored in the second half; the first half's lines take the text's color.)
            if (i > 5) d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->setCurrent(ColorRef::rgb(QColor(0xFF, 0, 0)));
        });
    }
    const QByteArray zip = w.document()->toDocx();
    QVERIFY(rt::strictProblems(zip).isEmpty());
    const QByteArray x = documentXml(zip);
    for (int i = 1; i < vals.size(); ++i) {
        const QString props = runPropsOfText(x, QStringLiteral("w%1").arg(i, 2, 10, QLatin1Char('0')));
        const QString want = i > 5 ? QStringLiteral("<w:u w:val=\"%1\" w:color=\"FF0000\"/>").arg(vals[i]) : QStringLiteral("<w:u w:val=\"%1\"/>").arg(vals[i]);
        QVERIFY2(props.contains(want), qPrintable(QStringLiteral("%1: %2").arg(want, props)));
    }
    // Read again, the dialog shows each choice and color.
    WriteWindow v;
    v.show();
    v.setDocument(open(zip), QString());
    for (int i = 1; i < vals.size(); ++i) {
        selectRange(v, (i - 1) * 4, (i - 1) * 4 + 3);
        withFontDialog(v, [&](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->currentIndex(), i);
            const ColorRef color = d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->current();
            if (i > 5) QCOMPARE(color.rgbValue(), QColor(0xFF, 0, 0));
            else QVERIFY(color.isNone());
        }, false);
    }
    // Underline off, and the color automatic again: the file says no line where there was one, and no color where there was one.
    selectRange(v, 20, 23);   // "w06", dashed
    withFontDialog(v, [](QDialog *d) { d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->setCurrentIndex(0); });
    QVERIFY(!documentXml(v.document()->toDocx()).contains("w:val=\"dash\""));
    selectRange(v, 24, 27);   // "w07", dot dash in red
    withFontDialog(v, [](QDialog *d) { d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->setCurrent(ColorRef::none()); });
    QVERIFY2(runPropsOfText(documentXml(v.document()->toDocx()), QStringLiteral("w07")) == QLatin1String("<w:u w:val=\"dotDash\"/>"), documentXml(v.document()->toDocx()).constData());
}


// A file's own effects are read into the dialog's, and written back as they were (a style that has one, and a run that turns it off, too).
void WriteTests::aFilesEffectsAreReadAndWrittenBack()
{
    const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Shouting\"><w:name w:val=\"Shouting\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:caps/></w:rPr></w:style>";
    const QByteArray body =
        "<w:p><w:r><w:rPr><w:strike/></w:rPr><w:t>strike</w:t></w:r><w:r><w:t xml:space=\"preserve\"> </w:t></w:r><w:r><w:rPr><w:dstrike/></w:rPr><w:t>dstrike</w:t></w:r></w:p>"
        "<w:p><w:r><w:rPr><w:vertAlign w:val=\"superscript\"/></w:rPr><w:t>super</w:t></w:r><w:r><w:rPr><w:vertAlign w:val=\"subscript\"/></w:rPr><w:t>sub</w:t></w:r></w:p>"
        "<w:p><w:r><w:rPr><w:smallCaps/></w:rPr><w:t>small</w:t></w:r><w:r><w:rPr><w:vanish/></w:rPr><w:t>hidden</w:t></w:r></w:p>"
        "<w:p><w:r><w:rPr><w:u w:val=\"double\" w:color=\"00B050\"/></w:rPr><w:t>double</w:t></w:r><w:r><w:rPr><w:u w:val=\"dottedHeavy\"/></w:rPr><w:t>heavy</w:t></w:r>"
        "<w:r><w:rPr><w:u w:val=\"dashLong\" w:color=\"0070C0\"/></w:rPr><w:t>long</w:t></w:r></w:p>"
        "<w:p><w:pPr><w:pStyle w:val=\"Shouting\"/></w:pPr><w:r><w:t>loud</w:t></w:r><w:r><w:rPr><w:caps w:val=\"0\"/><w:smallCaps/></w:rPr><w:t>quiet</w:t></w:r></w:p>";
    // Every underline a file can name, each in a run of its own (the runs differ, so none is joined to the next).
    const QStringList lines = {QStringLiteral("single"),     QStringLiteral("words"),         QStringLiteral("double"),          QStringLiteral("thick"),        QStringLiteral("dotted"),
                               QStringLiteral("dottedHeavy"), QStringLiteral("dash"),         QStringLiteral("dashedHeavy"),     QStringLiteral("dashLong"),     QStringLiteral("dashLongHeavy"),
                               QStringLiteral("dotDash"),   QStringLiteral("dashDotHeavy"), QStringLiteral("dotDotDash"),      QStringLiteral("dashDotDotHeavy"), QStringLiteral("wave"),
                               QStringLiteral("wavyHeavy"), QStringLiteral("wavyDouble")};
    QByteArray underlines = "<w:p>";
    for (const QString &val : lines) underlines += "<w:r><w:rPr><w:u w:val=\"" + val.toUtf8() + "\"/></w:rPr><w:t>u-" + val.toUtf8() + "</w:t></w:r>";
    underlines += "</w:p>";
    auto doc = open(makeDocx(body + underlines, styles));
    QVERIFY(doc);
    const QByteArray zip = doc->toDocx();
    QVERIFY(rt::strictProblems(zip).isEmpty());
    const QByteArray x = documentXml(zip);
    auto props = [&](const char *word) { return runPropsOfText(x, QString::fromLatin1(word)); };
    for (const QString &val : lines) {
        const QString got = runPropsOfText(x, QStringLiteral("u-") + val);
        QVERIFY2(got == QStringLiteral("<w:u w:val=\"%1\"/>").arg(val), qPrintable(val + QStringLiteral(" came back as ") + got));
    }
    QVERIFY2(props("strike") == QLatin1String("<w:strike/>"), qPrintable(props("strike")));
    QVERIFY2(props("dstrike") == QLatin1String("<w:dstrike/>"), qPrintable(props("dstrike")));
    QVERIFY2(props("super") == QLatin1String("<w:vertAlign w:val=\"superscript\"/>"), qPrintable(props("super")));
    QVERIFY2(props("sub") == QLatin1String("<w:vertAlign w:val=\"subscript\"/>"), qPrintable(props("sub")));
    QVERIFY2(props("small") == QLatin1String("<w:smallCaps/>"), qPrintable(props("small")));
    QVERIFY2(props("hidden") == QLatin1String("<w:vanish/>"), qPrintable(props("hidden")));
    QVERIFY2(props("double") == QLatin1String("<w:u w:val=\"double\" w:color=\"00B050\"/>"), qPrintable(props("double")));
    QVERIFY2(props("heavy") == QLatin1String("<w:u w:val=\"dottedHeavy\"/>"), qPrintable(props("heavy")));
    QVERIFY2(props("long") == QLatin1String("<w:u w:val=\"dashLong\" w:color=\"0070C0\"/>"), qPrintable(props("long")));
    // The paragraph's style is all caps, and so "loud" says nothing of it; "quiet" turns it off and small caps on.
    QVERIFY2(props("loud").isEmpty(), qPrintable(props("loud")));
    QVERIFY2(props("quiet").contains(QLatin1String("<w:caps w:val=\"0\"/>")) && props("quiet").contains(QLatin1String("<w:smallCaps/>")), qPrintable(props("quiet")));
    // The dialog on a style's all caps: clearing it says "not caps" in the run.
    WriteWindow w;
    w.show();
    w.setDocument(std::move(doc), QString());
    const int loud = w.document()->story()->toPlainText().indexOf(QLatin1String("loud"));
    selectRange(w, loud, loud + 4);
    withFontDialog(w, [](QDialog *d) {
        QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("allCaps"))->checkState(), Qt::Checked);
        d->findChild<QCheckBox *>(QStringLiteral("allCaps"))->setChecked(false);
    });
    QVERIFY2(runPropsOfText(documentXml(w.document()->toDocx()), QStringLiteral("loud")).contains(QLatin1String("<w:caps w:val=\"0\"/>")), documentXml(w.document()->toDocx()).constData());
}


// ---------------------------------------------------------------- after the review
// What a dialog test does to the dialog can fail without leaving the dialog open: the helper closes it from outside, and reports a dialog that is not the one asked for.
void WriteTests::aFailedCheckInADialogTestStillClosesTheDialog()
{
    WriteWindow w;
    w.show();
    bool filled = false;
    withFontDialog(w, [&filled](QDialog *) {
        filled = true;
        QEXPECT_FAIL("", "this check is meant to fail", Continue);
        QCOMPARE(1, 2);
    });
    QVERIFY(filled && !QApplication::activeModalWidget());
    // The zoom dialog opens where the paragraph dialog was expected: it is closed without being touched, and the test says so.
    const double zoom = w.view()->zoom();
    QEXPECT_FAIL("", "the dialog that opens is the zoom dialog, not the paragraph dialog: meant to be reported", Continue);
    withDialog([&w] { w.act("zoom.dialog")->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *) { QFAIL("not to run on the wrong dialog"); });
    QVERIFY(!QApplication::activeModalWidget());
    QCOMPARE(w.view()->zoom(), zoom);
}


// The welcome topic does not say that headers and footers are missing: they are shown.
void WriteTests::theWelcomeTopicDoesNotSayHeadersAreNotShown()
{
    const QString start = jp::help::topic(QStringLiteral("start")).markdown;
    QVERIFY2(!start.contains(QLatin1String("Headers, footers, and page numbers are not shown")), "headers and footers are drawn now");
}


// The welcome topic says what this version does and does not do: it shows tables, headers and footers, notes, pictures, shapes, and text boxes, reads .doc files, and lays out sections
// that go on the same page (none of which it used to say), and it does not say the old things. The topics on opening and on spelling say the same.
void WriteTests::theWelcomeTopicSaysWhatThisVersionDoesAndDoesNotDo()
{
    const QString start = jp::help::topic(QStringLiteral("start")).markdown;
    for (const char *stale : {"Tables are not shown", "Pictures, shapes, and text boxes are not shown", "show their numbers, but not their text", "open or save .doc", "start a new page instead", "**[picture]**", "You can't open .rtf or .odt files yet", "Only the page number field can be inserted", "are kept, but not shown", "can't save .doc, .rtf, or .odt"})
        QVERIFY2(!start.contains(QLatin1String(stale)), stale);
    for (const char *now : {".dot, .rtf, .odt, .ott, and .fodt", "Saving .docx, .docm, .dotx, .dotm, .odt, .rtf, and .txt files", "tables, headers and footers, footnotes and endnotes, pictures", "continue on the same page", "Tab", "The ruler is across the top only"})
        QVERIFY2(start.contains(QLatin1String(now)), now);
    QVERIFY(jp::help::topic(QStringLiteral("open-save")).markdown.contains(QLatin1String("**.doc**")));
    const QString spelling = jp::help::topic(QStringLiteral("spelling")).markdown;
    QVERIFY2(!spelling.contains(QLatin1String("not show in tables")), "red lines show in a table's cells");
    QVERIFY(spelling.contains(QLatin1String("JeffWrite, JeffPub")));
}


// Over characters that differ in font, style, size, color, or underline, the boxes show nothing; whatever is set then goes to all of them, also what the first
// characters have.
void WriteTests::aFontBoxOverSelectionsThatDifferShowsNothingAndWhatIsSetGoesToAll()
{
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("Some words here"));
    selectRange(w, 5, 10);
    toggleBold(*w.document(), w.view()->cursor());
    toggleItalic(*w.document(), w.view()->cursor());
    setFontSize(*w.document(), w.view()->cursor(), 20);
    setTextColor(*w.document(), w.view()->cursor(), QColor(0xC0, 0, 0));
    setFontFamily(*w.document(), w.view()->cursor(), QStringLiteral("Cousine"));
    changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) {
        f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        f.setProperty(tp::UnderlineKind, 1);   // double
        f.setUnderlineColor(QColor(0, 0, 0xFF));
    });
    w.document()->story()->clearUndoRedoStacks();
    auto at = [&](int position) {
        QTextCursor c(w.document()->story());
        c.setPosition(position + 1);
        return c.charFormat();
    };
    const QTextCharFormat plain = at(0);   // ("Some ")
    // Opened and closed with nothing touched, the dialog changes nothing.
    selectRange(w, 0, 10);
    withFontDialog(w, [](QDialog *d) {
        auto *font = d->findChild<FontCombo *>(QStringLiteral("fontName"));
        auto *style = d->findChild<QComboBox *>(QStringLiteral("fontStyle"));
        auto *size = d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"));
        auto *underline = d->findChild<QComboBox *>(QStringLiteral("underlineStyle"));
        QVERIFY(font && style && size && underline);
        QVERIFY(font->currentText().trimmed().isEmpty());
        QCOMPARE(style->currentIndex(), -1);
        QVERIFY2(size->text().trimmed().isEmpty(), qPrintable(size->text()));
        QCOMPARE(underline->currentIndex(), -1);
    });
    QVERIFY(!w.document()->story()->isUndoAvailable());
    QCOMPARE(at(6).fontPointSize(), 20.0);
    // Each is set to what "Some " has, which is what the box would have shown before: all of the selection takes it.
    selectRange(w, 0, 10);
    withFontDialog(w, [&](QDialog *d) {
        auto *font = d->findChild<FontCombo *>(QStringLiteral("fontName"));
        auto *style = d->findChild<QComboBox *>(QStringLiteral("fontStyle"));
        auto *size = d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"));
        auto *color = d->findChild<ColorButton *>(QStringLiteral("fontColor"));
        auto *underline = d->findChild<QComboBox *>(QStringLiteral("underlineStyle"));
        QVERIFY(font && style && size && color && underline);
        Q_EMIT font->familyChosen(QStringLiteral("+Body"));
        style->setCurrentIndex(0);
        size->setValue(plain.fontPointSize());
        color->setCurrent(ColorRef::none());
        Q_EMIT color->colorPicked(ColorRef::none());
        underline->setCurrentIndex(0);
    });
    for (int position : {0, 6}) {
        const QTextCharFormat f = at(position);
        QVERIFY2(!f.hasProperty(QTextFormat::FontFamilies) && f.stringProperty(tp::ThemeFont) == QLatin1String("minor"), qPrintable(QString::number(position)));
        QCOMPARE(f.fontPointSize(), plain.fontPointSize());
        QCOMPARE(f.fontWeight(), int(QFont::Normal));
        QVERIFY(!f.fontItalic());
        QVERIFY(!f.hasProperty(QTextFormat::ForegroundBrush) || f.foreground().style() == Qt::NoBrush);
        QCOMPARE(f.underlineStyle(), QTextCharFormat::NoUnderline);
    }
    // One step undoes it.
    w.view()->undo();
    QCOMPARE(at(6).fontPointSize(), 20.0);
    QVERIFY(!w.document()->story()->isUndoAvailable());
}


// The colors of underlines that differ are shown as nothing too, and setting one colors the underlines there are, and no other text.
void WriteTests::anUnderlineColorSetOverUnderlinesThatDifferColorsOnlyTheUnderlines()
{
    WriteWindow w;
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("red blue plain"));
    selectRange(w, 0, 3);
    changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) { f.setUnderlineStyle(QTextCharFormat::SingleUnderline); f.setUnderlineColor(QColor(0xFF, 0, 0)); });
    selectRange(w, 4, 8);
    changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) { f.setUnderlineStyle(QTextCharFormat::SingleUnderline); f.setUnderlineColor(QColor(0, 0, 0xFF)); });
    selectRange(w, 0, 14);
    // (Red, which is what the first of the lines has, goes to the blue one as well.)
    withFontDialog(w, [](QDialog *d) {
        auto *color = d->findChild<ColorButton *>(QStringLiteral("underlineColor"));
        QVERIFY(color);
        color->setCurrent(ColorRef::rgb(QColor(0xFF, 0, 0)));
        Q_EMIT color->colorPicked(ColorRef::rgb(QColor(0xFF, 0, 0)));
    });
    auto at = [&](int position) {
        QTextCursor c(w.document()->story());
        c.setPosition(position + 1);
        return c.charFormat();
    };
    QCOMPARE(at(1).underlineColor(), QColor(0xFF, 0, 0));
    QCOMPARE(at(5).underlineColor(), QColor(0xFF, 0, 0));
    QCOMPARE(at(10).underlineStyle(), QTextCharFormat::NoUnderline);   // ("plain" gets no line, and no color for one)
    QVERIFY(!at(10).hasProperty(QTextFormat::TextUnderlineColor));
}


// A bookmark or a field's code in a selection is not a character that has no effect: a selection that is struck through all over is, with a bookmark in it.
void WriteTests::anAtomInTheSelectionDoesNotMakeAnEffectPartial()
{
    auto run = [](const char *text) { return QByteArray("<w:r><w:rPr><w:strike/></w:rPr><w:t>") + text + "</w:t></w:r>"; };
    auto doc = open(makeDocx("<w:p>" + run("aa") + "<w:bookmarkStart w:id=\"1\" w:name=\"mark\"/>" + run("bb") + "<w:bookmarkEnd w:id=\"1\"/></w:p>", kStyles));
    QVERIFY(doc);
    // (The bookmark is a piece of the story that is not struck through itself.)
    int atoms = 0;
    for (QTextBlock::iterator it = doc->story()->firstBlock().begin(); !it.atEnd(); ++it)
        if (isAtom(it.fragment().charFormat())) {
            ++atoms;
            QVERIFY(!it.fragment().charFormat().fontStrikeOut());
        }
    QVERIFY(atoms >= 1);
    WriteWindow w;
    w.show();
    w.setDocument(std::move(doc), QString());
    w.view()->selectAll();
    withFontDialog(w, [](QDialog *d) {
        QCheckBox *strike = d->findChild<QCheckBox *>(QStringLiteral("strikethrough"));
        QVERIFY(strike);
        QCOMPARE(strike->checkState(), Qt::Checked);
        for (const char *other : {"doubleStrikethrough", "superscript", "subscript", "smallCaps", "allCaps", "hidden"}) QCOMPARE(d->findChild<QCheckBox *>(QString::fromLatin1(other))->checkState(), Qt::Unchecked);
    }, false);
    // The ribbon's button is down, and pressing it takes the line off all of the text.
    QTRY_VERIFY(w.act(QStringLiteral("fmt.strike"))->isChecked());
    w.act(QStringLiteral("fmt.strike"))->trigger();
    for (QTextBlock::iterator it = w.document()->story()->firstBlock().begin(); !it.atEnd(); ++it)
        if (!isAtom(it.fragment().charFormat())) QVERIFY2(!it.fragment().charFormat().fontStrikeOut(), qPrintable(it.fragment().text()));
}


// A line with several flags is written as the one the flags are first for (double, then words, then thick); every name a file has for an underline is read as a
// line and flags and written as the same name; and a long dash is a flag of its own.
void WriteTests::anUnderlinesFlagsAndNamesMapBothWays()
{
    auto valOf = [](QTextCharFormat::UnderlineStyle style, int kind) {
        QTextCharFormat f;
        f.setUnderlineStyle(style);
        if (kind) f.setProperty(tp::UnderlineKind, kind);
        return runPropsOf(f).underline->val;
    };
    // (JeffPub's double plus thick underline is kind 5; pasted into JeffWrite and saved it must stay a double line.)
    const QList<QPair<int, QString>> single = {{0, QStringLiteral("single")}, {1, QStringLiteral("double")}, {2, QStringLiteral("words")}, {4, QStringLiteral("thick")}, {3, QStringLiteral("double")},
                                               {5, QStringLiteral("double")}, {6, QStringLiteral("words")}, {7, QStringLiteral("double")}};
    for (const auto &k : single) QCOMPARE(valOf(QTextCharFormat::SingleUnderline, k.first), k.second);
    const QList<QPair<int, QString>> wave = {{0, QStringLiteral("wave")}, {1, QStringLiteral("wavyDouble")}, {3, QStringLiteral("wavyDouble")}, {5, QStringLiteral("wavyDouble")}, {7, QStringLiteral("wavyDouble")}, {4, QStringLiteral("wavyHeavy")}};
    for (const auto &k : wave) QCOMPARE(valOf(QTextCharFormat::WaveUnderline, k.first), k.second);
    QCOMPARE(valOf(QTextCharFormat::DotLine, 1), QStringLiteral("dotted"));   // (a flag the line has no name for: the plain line)
    struct Name { const char *val; QTextCharFormat::UnderlineStyle style; int kind; };
    const Name names[] = {{"single", QTextCharFormat::SingleUnderline, 0}, {"words", QTextCharFormat::SingleUnderline, 2}, {"double", QTextCharFormat::SingleUnderline, 1}, {"thick", QTextCharFormat::SingleUnderline, 4},
                          {"dotted", QTextCharFormat::DotLine, 0}, {"dottedHeavy", QTextCharFormat::DotLine, 4}, {"dash", QTextCharFormat::DashUnderline, 0}, {"dashedHeavy", QTextCharFormat::DashUnderline, 4},
                          {"dashLong", QTextCharFormat::DashUnderline, 8}, {"dashLongHeavy", QTextCharFormat::DashUnderline, 12}, {"dotDash", QTextCharFormat::DashDotLine, 0},
                          {"dashDotHeavy", QTextCharFormat::DashDotLine, 4}, {"dotDotDash", QTextCharFormat::DashDotDotLine, 0}, {"dashDotDotHeavy", QTextCharFormat::DashDotDotLine, 4},
                          {"wave", QTextCharFormat::WaveUnderline, 0}, {"wavyHeavy", QTextCharFormat::WaveUnderline, 4}, {"wavyDouble", QTextCharFormat::WaveUnderline, 1}};
    for (const Name &n : names) {
        wml::RunProps r;
        wml::Underline u;
        u.val = QString::fromLatin1(n.val);
        r.underline = u;
        const QTextCharFormat f = charFormatOf(r);
        QVERIFY2(f.underlineStyle() == n.style && f.intProperty(tp::UnderlineKind) == n.kind, n.val);
        QCOMPARE(runPropsOf(f).underline->val, QString::fromLatin1(n.val));
    }
}


// Replace All takes time in step with the matches, also when they are all in one paragraph (it was the square of them: each swap in a long paragraph cost its length).
void WriteTests::replaceAllInOneLongParagraphTakesTimeInStepWithTheMatches()
{
    // A paragraph of 96,000 characters with this many "cat"s in it; the characters of the story that Replace All takes out and puts in to turn them into "dog"s (counted from
    // the story's own changes: one swap of three letters is six characters, and a swap that cost its paragraph's length would be 96,000).
    auto changed = [](int matches) {
        std::unique_ptr<WriteWindow> w;
        WriteFindPane *pane = nullptr;
        QString text;
        const int per = 96000 / matches;
        while (text.size() < 96000) text += QString(per - 4, QLatin1Char('x')) + QStringLiteral(" cat");
        w = std::make_unique<WriteWindow>();
        QTextCursor c(w->document()->story());
        c.insertText(text);
        w->showFindPane(true);
        pane = w->findPane();
        pane->findBox()->setText(QStringLiteral("cat"));
        pane->replaceBox()->setText(QStringLiteral("dog"));
        qint64 touched = 0;
        QObject::connect(w->document()->story(), &QTextDocument::contentsChange, w->document()->story(), [&](int, int removed, int added) { touched += removed + added; });
        pane->replaceAll();
        if (pane->status() != QStringLiteral("JeffWrite made %1 replacements.").arg(text.count(QLatin1String("cat")))) return qint64(-1);
        if (w->document()->story()->toPlainText().contains(QLatin1String("cat"))) return qint64(-1);
        return touched;
    };
    const qint64 few = changed(2000), many = changed(16000);
    QVERIFY2(few > 0 && many > 0, "Replace All made the replacements");
    // (Eight times the matches: eight times the characters changed, with some room; the square would be sixty-four.)
    QVERIFY2(many < few * 12, qPrintable(QStringLiteral("2,000 matches changed %1 characters, 16,000 changed %2").arg(few).arg(many)));
}


// A bookmark in the middle of words does not hide them: they are found, shown selected, and replaced, and the bookmark is kept.
void WriteTests::findAndReplaceSeeWordsThatABookmarkSplits()
{
    const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">say foo </w:t></w:r><w:bookmarkStart w:id=\"0\" w:name=\"_GoBack\"/><w:r><w:t>bar</w:t></w:r><w:bookmarkEnd w:id=\"0\"/>"
                            "<w:r><w:t xml:space=\"preserve\"> and foo bar again</w:t></w:r></w:p>";
    WriteWindow w;
    w.setDocument(open(makeDocx(body, kStyles)), QString());
    w.show();
    QVERIFY2(w.document()->story()->toPlainText().contains(QChar::ObjectReplacementCharacter), "the bookmark is in the story, between the words");
    auto words = [&] {
        QString t = w.document()->story()->toPlainText();
        t.remove(QChar::ObjectReplacementCharacter);
        return t;
    };
    w.showFindPane(true);
    WriteFindPane *pane = w.findPane();
    pane->findBox()->setText(QStringLiteral("foo bar"));
    QTextCursor start(w.document()->story());
    w.view()->setCursor(start);
    QVERIFY(pane->findNext());
    QString shown = w.view()->cursor().selectedText();
    shown.remove(QChar::ObjectReplacementCharacter);
    QCOMPARE(shown, QStringLiteral("foo bar"));
    QCOMPARE(w.view()->cursor().selectionStart(), 4);
    QVERIFY(pane->findNext());   // the second, which has no bookmark in it
    QVERIFY(w.view()->cursor().selectionStart() > 10);
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("foo bar"));
    // Replace swaps the first: the new words are where the old were, and the bookmark is still there.
    w.view()->setCursor(start);
    pane->replaceBox()->setText(QStringLiteral("baz"));
    pane->replaceOne();   // (shows the first)
    pane->replaceOne();
    QCOMPARE(words(), QStringLiteral("say baz and foo bar again"));
    QVERIFY(w.document()->story()->toPlainText().contains(QChar::ObjectReplacementCharacter));
    QVERIFY(w.document()->toDocx().size() > 0);
    QVERIFY2(documentXml(w.document()->toDocx()).contains("_GoBack"), "the bookmark is saved");
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("foo bar"));   // (on to the next)
    // Undo puts it back as it was, with one step.
    w.view()->undo();
    QCOMPARE(words(), QStringLiteral("say foo bar and foo bar again"));
    // Replace All: both, one step.
    w.document()->story()->clearUndoRedoStacks();
    pane->replaceAll();
    QCOMPARE(words(), QStringLiteral("say baz and baz again"));
    QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 2 replacements."));
    QVERIFY2(documentXml(w.document()->toDocx()).contains("_GoBack"), "the bookmark is saved");
    w.view()->undo();
    QCOMPARE(words(), QStringLiteral("say foo bar and foo bar again"));
    QVERIFY(!w.document()->story()->isUndoAvailable());
}


// Replace All of the shared search finds the matches first and puts the new words in from the last (the one way that does not cost a long paragraph its length
// for each); it gives what putting them in from the first, finding on from each, gave: the same words, the same count, the same looks, also for whole words that
// touch (the letter before a match is then the last of the words put in place of the one before). JeffDeck and JeffPub swap this way.
void WriteTests::replacingFromTheLastMatchGivesWhatReplacingFromTheFirstGave()
{
    int differ = 0;   // (the cases in which the two ways would not give the same words, if the shared search did not tell them apart)
    auto check = [&differ](const QString &text, const QString &q, const QString &with, QTextDocument::FindFlags flags, const QString &where) {
        QTextDocument a, b, c;
        for (QTextDocument *d : {&a, &b, &c}) {
            QTextCursor cursor(d);
            cursor.insertText(text);
            for (int i = 0; i + 1 < d->characterCount(); i += 2) {   // (every other character bold, so that the looks can be told)
                QTextCursor t(d);
                t.setPosition(i);
                t.setPosition(i + 1, QTextCursor::KeepAnchor);
                QTextCharFormat f;
                f.setFontWeight(QFont::Bold);
                t.mergeCharFormat(f);
            }
        }
        // From the first match on, finding on from each swap.
        int expected = 0;
        for (QTextCursor m = a.find(q, 0, flags); !m.isNull(); m = a.find(q, m, flags)) {
            m.insertText(with);
            ++expected;
        }
        // The matches in the words as they are, put in from the last (what would be given if the search did not look at whole words that touch).
        QVector<QPair<int, int>> found;
        for (QTextCursor m = c.find(q, 0, flags); !m.isNull(); m = c.find(q, m, flags)) found.push_back({m.selectionStart(), m.selectionEnd()});
        for (int i = int(found.size()) - 1; i >= 0; --i) {
            QTextCursor m(&c);
            m.setPosition(found[i].first);
            m.setPosition(found[i].second, QTextCursor::KeepAnchor);
            m.insertText(with);
        }
        if (c.toPlainText() != a.toPlainText()) ++differ;
        FindWords words;
        words.story = [&b](const FindStory &) { return &b; };
        QVector<FindStory> stories;
        stories << FindStory{QString(), QStringLiteral("s")};
        const int made = replaceInStories(words, stories, q, flags, with);
        QVERIFY2(made == expected, qPrintable(where + QStringLiteral(" made %1, not %2").arg(made).arg(expected)));
        QVERIFY2(a.toPlainText() == b.toPlainText(), qPrintable(where + QStringLiteral(" gave ") + b.toPlainText() + QStringLiteral(" and not ") + a.toPlainText()));
        for (int i = 0; i + 1 < a.characterCount(); ++i) {
            QTextCursor ca(&a), cb(&b);
            ca.setPosition(i + 1);
            cb.setPosition(i + 1);
            QVERIFY2(ca.charFormat().fontWeight() == cb.charFormat().fontWeight(), qPrintable(where + QStringLiteral(" at %1").arg(i)));
        }
    };
    const QTextDocument::FindFlags whole = QTextDocument::FindWholeWords;
    // Whole words that touch: "--" is two words of "-", until the first is put out for "x", and the second then has a letter before it.
    check(QStringLiteral("-- --"), QStringLiteral("-"), QStringLiteral("x"), whole, QStringLiteral("touching"));
    check(QStringLiteral(" ,, ,"), QStringLiteral(","), QStringLiteral("ab"), whole, QStringLiteral("touching, with two letters"));
    check(QStringLiteral("-- --"), QStringLiteral("-"), QStringLiteral("-"), whole, QStringLiteral("touching, and put out for the same"));
    check(QStringLiteral("-- --"), QStringLiteral("-"), QString(), whole, QStringLiteral("touching, put out for nothing"));
    check(QStringLiteral("cat cat\ncat"), QStringLiteral("cat"), QStringLiteral("dog"), QTextDocument::FindFlags(), QStringLiteral("apart"));
    QVERIFY2(differ >= 2, qPrintable(QString::number(differ)));
    std::mt19937 rng(11);
    const QStringList alphabet = {QStringLiteral("a"), QStringLiteral("b"), QStringLiteral(" "), QStringLiteral("-"), QStringLiteral("A"), QStringLiteral("-"), QStringLiteral(" "), QStringLiteral("\n")};
    const QStringList queries = {QStringLiteral("a"), QStringLiteral("-"), QStringLiteral("ab"), QStringLiteral("aa"), QStringLiteral("b-"), QStringLiteral("A"), QStringLiteral("a-a"), QStringLiteral("-")};
    const QStringList withs = {QString(), QStringLiteral("a"), QStringLiteral("x"), QStringLiteral("ab"), QStringLiteral("-"), QStringLiteral("xyz")};
    for (int round = 0; round < 400; ++round) {
        QString text;
        for (int i = 0, n = 1 + int(rng() % 30); i < n; ++i) text += alphabet[rng() % alphabet.size()];
        QTextDocument::FindFlags flags;
        if (rng() % 2) flags |= QTextDocument::FindCaseSensitively;
        if (rng() % 2) flags |= QTextDocument::FindWholeWords;
        const QString q = queries[rng() % queries.size()], with = withs[rng() % withs.size()];
        check(text, q, with, flags, QStringLiteral("%1: \"%2\" for \"%3\" in \"%4\"").arg(round).arg(q, with, QString(text).replace(QLatin1Char('\n'), QLatin1Char('|'))));
        if (QTest::currentTestFailed()) return;
    }
}


void WriteTests::exportPdfWritesAPdfWithAPagePerPage()
{
    QTemporaryDir dir;
    WriteWindow w;
    w.setDocument(open(makeDocx(paragraphs(120), kStyles)), QString());
    w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("out.pdf")); });
    QVERIFY(w.exportPdf());
    QFile f(dir.filePath(QStringLiteral("out.pdf")));
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray pdf = f.readAll();
    QVERIFY(pdf.startsWith("%PDF"));
    int pages = 0;
    for (qsizetype at = pdf.indexOf("/Type /Page\n"); at >= 0; at = pdf.indexOf("/Type /Page\n", at + 1)) ++pages;
    QCOMPARE(pages, w.view()->pageCount());
    QVERIFY(w.view()->pageCount() > 3);
}


void WriteTests::pagesToPrintFollowTheRange()
{
    WriteWindow w;
    w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
    const int n = (w.view()->paginateAll(), w.view()->pageCount());
    QVERIFY(n > 5);
    WriteOutputOptions o;
    QCOMPARE(w.outputPages(o).size(), n);
    o.range = WriteOutputOptions::Range::Pages;
    o.pages = QStringLiteral("2-4, 2, 9999, x, 1");
    QCOMPARE(w.outputPages(o), (QVector<int>{1, 2, 3, 0}));
    o.range = WriteOutputOptions::Range::Current;
    QCOMPARE(w.outputPages(o).size(), 1);
}


// ---------------------------------------------------------------- printing
void WriteTests::pageRangesAreReadAsTheyAreTyped()
{
    using R = QVector<int>;
    QCOMPARE(parsePageRange(QStringLiteral("1-3, 5"), 10), (R{0, 1, 2, 4}));
    QCOMPARE(parsePageRange(QStringLiteral("5-3;9"), 10), (R{2, 3, 4, 8}));       // (backwards, and a semicolon)
    QCOMPARE(parsePageRange(QStringLiteral("2-4, 2, 9999, x, 1"), 10), (R{1, 2, 3, 0}));   // (once each, in the order given; what names no page is passed by)
    QCOMPARE(parsePageRange(QStringLiteral(" 7 - 20 "), 10), (R{6, 7, 8, 9}));    // (to the last page there is)
    QCOMPARE(parsePageRange(QStringLiteral("8-"), 10), (R{7, 8, 9}));              // (to the end)
    QCOMPARE(parsePageRange(QStringLiteral("-2"), 10), (R{0, 1}));                 // (from the start)
    QCOMPARE(parsePageRange(QStringLiteral("3"), 3), (R{2}));
    for (const char *bad : {"", "  ", "0", "11", "x", "abc", "1-2-3", ",,", "-", "-0", "0-", "0-5", "11-", "99-", "1.5", "99999999999", "99999999999-", "-,-"})
        QVERIFY2(parsePageRange(QString::fromLatin1(bad), 10).isEmpty(), bad);
    QVERIFY(parsePageRange(QStringLiteral("1"), 0).isEmpty());
    QVERIFY(parsePageRange(QStringLiteral("1-"), 0).isEmpty());
    // An open end is the last page (or the first) and no further: it never turns about.
    QCOMPARE(parsePageRange(QStringLiteral("10-"), 10), (R{9}));
    QCOMPARE(parsePageRange(QStringLiteral("-10"), 10), (R{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    QCOMPARE(parsePageRange(QStringLiteral("-99"), 10), (R{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    QCOMPARE(parsePageRange(QStringLiteral("-1"), 10), (R{0}));
    // Backwards ranges read forwards; numbers too big for any page are past the last; the rest of a list is still taken.
    QCOMPARE(parsePageRange(QStringLiteral("5-2"), 10), (R{1, 2, 3, 4}));
    QCOMPARE(parsePageRange(QStringLiteral("3-3"), 10), (R{2}));
    QCOMPARE(parsePageRange(QStringLiteral("8-99999999999"), 10), (R{7, 8, 9}));
    QCOMPARE(parsePageRange(QStringLiteral("99999999999-3"), 10), (R{2, 3, 4, 5, 6, 7, 8, 9}));
    QCOMPARE(parsePageRange(QStringLiteral(" , 4 ,, 2 , abc, 99-, -0"), 10), (R{3, 1}));
    QCOMPARE(parsePageRange(QStringLiteral("1-3,2-4"), 10), (R{0, 1, 2, 3}));
}


void WriteTests::copiesComeOutCollatedOrEachPageTogether()
{
    using R = QVector<int>;
    QCOMPARE(sheetOrder(R{0, 1, 2}, 2, true), (R{0, 1, 2, 0, 1, 2}));
    QCOMPARE(sheetOrder(R{0, 1, 2}, 2, false), (R{0, 0, 1, 1, 2, 2}));
    QCOMPARE(sheetOrder(R{4, 2}, 1, true), (R{4, 2}));
    QCOMPARE(sheetOrder(R{4, 2}, 0, false), (R{4, 2}));   // (never fewer than one)
    QCOMPARE(sheetOrder(R(), 3, true), R());
}


QTEST_MAIN(WriteTests)
