// JeffPub's tests of every command of the window run once (the slowest test of the program, alone in its class so that the other tests run beside it).
#include "pubhelpers.h"

class PubCommandsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // Every command in the window runs without crashing or Qt warnings, with a
    // fitting selection for its kind. Dialogs and menus it opens are closed.
    void allCommandsRun()
    {
        static QStringList warnings;
        static QString current;
        warnings.clear();
        QtMessageHandler prev = qInstallMessageHandler([](QtMsgType t, const QMessageLogContext &, const QString &m) {
            // The offscreen test display can't size native windows; that note is not a fault.
            if ((t == QtWarningMsg || t == QtCriticalMsg) && !m.contains(QLatin1String("This plugin does not support"))) warnings << current + ": " + m;
        });
        QTimer closer;
        closer.setInterval(20);
        QObject::connect(&closer, &QTimer::timeout, [] {
            if (QWidget *pop = QApplication::activePopupWidget()) pop->close();
            if (QWidget *mod = QApplication::activeModalWidget()) {
                if (auto *d = qobject_cast<QDialog *>(mod)) d->reject();
                else mod->close();
            }
        });
        closer.start();
        // Commands that end the session or open more windows are run elsewhere.
        QSet<QString> skip = {"file.exit", "win.new", "win.arrange", "win.cascade"};
#ifdef Q_OS_MACOS
        // macOS always shows its own print panel, which a test can't close.
        skip << QStringLiteral("file.printNow");
#endif
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QStringList ran;
        for (const QString &id : w.actionIds()) {
            if (skip.contains(id)) continue;
            QAction *a = w.act(id);
            if (!a) continue;
            current = id;
            // Which command a hang or crash is in (CI asks for this).
            if (qEnvironmentVariableIsSet("JP_TEST_TRACE")) {
                std::fprintf(stderr, "command %s\n", qPrintable(id));
                std::fflush(stderr);
            }
            const QString cat = id.section('.', 0, 0);
            // Fresh publication: the flyer has a picture, shapes, TextArt and text;
            // the newsletter's page 2 has a table.
            const bool table = cat == "tbl";
            w.editor()->setDocument(jp::findTemplate(table ? QStringLiteral("newsletter-classic") : QStringLiteral("flyer-event"))->build(jp::TemplateOptions()));
            jp::Editor *ed = w.editor();
            if (table) ed->setCurrentPage(1);
            auto firstOf = [&](jp::ItemType t) -> QString {
                for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
                    if (it->type() == t) return it->id;
                return {};
            };
            const QSet<QString> textCats = {"fmt", "para", "case", "dropcap", "style", "fit", "cols", "edit", "rev", "tb"};
            if (table) ed->select(firstOf(jp::ItemType::Table));
            else if (cat == "pic") ed->select(firstOf(jp::ItemType::Picture));
            else if (cat == "wa") ed->select(firstOf(jp::ItemType::TextArt));
            else if (cat == "shape" || cat == "fill" || cat == "line" || cat == "shadow" || cat == "obj" || cat == "arr" || cat == "wrap")
                ed->select(firstOf(jp::ItemType::Shape));
            else if (textCats.contains(cat)) {
                const QString t = firstOf(jp::ItemType::Text);
                ed->select(t);
                ed->beginTextEdit(t, 0);
                QTextCursor c = ed->cursor();
                c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
                ed->setCursor(c);
            }
            QTest::qWait(45);   // the window updates command states 30 ms after a change
            if (a->isEnabled()) {
                a->trigger();
                QCoreApplication::processEvents();
                ran << id;
            } else if (qEnvironmentVariableIsSet("JP_LIST_DISABLED")) {
                fprintf(stderr, "disabled: %s\n", qPrintable(id));
            }
            ed->endTextEdit();
        }
        current.clear();
        closer.stop();
        qInstallMessageHandler(prev);
        qInfo("ran %d of %d commands", int(ran.size()), int(w.actionIds().size()));
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.mid(0, 40).join('\n')));
    }
};

JP_TEST_CLASS(PubCommandsTests)
#include "tst_pub_commands.moc"
