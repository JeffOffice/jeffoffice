// Tests of JeffWrite's File screen: where editing stopped, the warning before a save to a format that holds less, and the Options pages. Every file is made here in code.

#include "../isolatesettings.h"
#include "app/settings.h"
#include "app/pageview.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/writedoc.h"

#include <QAction>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

using namespace jp;
using namespace jp::write;

namespace {

void typeText(WriteDoc &doc, QTextCursor &c, const QString &text)
{
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n')) insertParagraph(doc, c);
        else insertText(doc, c, QString(ch));
    }
}

bool saveInto(WriteWindow &w, const QTemporaryDir &dir, const QString &name)
{
    w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(name); });
    return w.saveAs();
}

struct Quiet {
    Quiet(WriteWindow &w) { w.setNotifier([this](const QString &, const QString &text) { said << text; }); }
    QStringList said;
};

void moveTo(WriteWindow &w, int pos)
{
    QTextCursor c(w.document()->story());
    c.setPosition(pos);
    w.view()->setCursor(c);
}

} // namespace

class FileTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }
    void init() { Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Tester")); }

    void aDocumentReopensWhereEditingStopped()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("a.docx"));
        {
            WriteWindow w;
            Quiet quiet(w);
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("alpha beta\ngamma delta\nepsilon zeta"));
            QVERIFY(saveInto(w, dir, QStringLiteral("a.docx")));
            moveTo(w, 14);
            w.close();
        }
        WriteWindow v;
        Quiet quiet(v);
        QVERIFY(v.openFile(path));
        QCOMPARE(v.view()->cursor().position(), 14);
    }

    void aPlaceBeyondAShorterFileIsKeptInside()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("a.docx"));
        {
            WriteWindow w;
            Quiet quiet(w);
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("a long line of words to leave behind"));
            QVERIFY(saveInto(w, dir, QStringLiteral("a.docx")));
            moveTo(w, 30);
            w.close();
        }
        {
            WriteWindow w;
            Quiet quiet(w);
            QVERIFY(w.openFile(path));
            QTextCursor c(w.document()->story());
            c.setPosition(0);
            c.setPosition(w.document()->story()->characterCount() - 1, QTextCursor::KeepAnchor);
            c.removeSelectedText();
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("hi"));
            QVERIFY(w.save());
        }
        // the remembered place (30) is past the end of "hi": the caret is put at the end
        WriteWindow v;
        Quiet quiet(v);
        QVERIFY(v.openFile(path));
        QVERIFY(v.view()->cursor().position() <= 2);
    }

    void lastPlaceCyclesThroughTheLastThreeEdits()
    {
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(!w.act(QStringLiteral("edit.lastPlace"))->isEnabled());
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("0123456789"));
        // four far-apart edits: only the last three are kept
        w.document()->story()->setPlainText(QString(2500, QLatin1Char('x')));
        w.resetEditPlaces();
        for (int at : {100, 700, 1300, 1900}) {
            QTextCursor c(w.document()->story());
            c.setPosition(at);
            c.insertText(QStringLiteral("q"));
        }
        QCOMPARE(w.editPlaces().size(), 3);
        QVERIFY(w.act(QStringLiteral("edit.lastPlace"))->isEnabled());
        QStringList went;
        for (int i = 0; i < 4; ++i) {
            w.act(QStringLiteral("edit.lastPlace"))->trigger();
            went << QString::number(w.view()->cursor().position());
        }
        QCOMPARE(went, (QStringList{QStringLiteral("1901"), QStringLiteral("1301"), QStringLiteral("701"), QStringLiteral("1901")}));
    }

    void aSaveToOdtAsksFirstAndCanSaveADocxInstead()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("x"));
        w.document()->setPageColor(QColor(0xFF, 0xF4, 0xD6));
        QStringList asked;
        WriteWindow::FormatChoice answer = WriteWindow::FormatChoice::Cancel;
        w.setFormatAsker([&](const QStringList &lost) {
            asked = lost;
            return answer;
        });
        // Cancel: nothing is written, and the document is as it was
        QVERIFY(!saveInto(w, dir, QStringLiteral("a.odt")));
        QVERIFY(asked.join(QLatin1Char(' ')).contains(QLatin1String("page color")));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("a.odt"))));
        QVERIFY(w.isModified());
        // A .docx instead: it is written beside, and the window has that file
        answer = WriteWindow::FormatChoice::Docx;
        QVERIFY(saveInto(w, dir, QStringLiteral("a.odt")));
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("a.docx"))));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("a.odt"))));
        QCOMPARE(QFileInfo(w.filePath()).fileName(), QStringLiteral("a.docx"));
        // Keep: written as an .odt, and not asked again of the same thing
        answer = WriteWindow::FormatChoice::Keep;
        asked.clear();
        QVERIFY(saveInto(w, dir, QStringLiteral("b.odt")));
        QVERIFY(!asked.isEmpty());
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("b.odt"))));
        asked.clear();
        quiet.said.clear();
        QVERIFY(saveInto(w, dir, QStringLiteral("c.odt")));
        QVERIFY(asked.isEmpty());
        QVERIFY(quiet.said.isEmpty());
    }

    void aDocumentAnOdtHoldsWholeIsNotAsked()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("plain"));
        bool asked = false;
        w.setFormatAsker([&](const QStringList &) {
            asked = true;
            return WriteWindow::FormatChoice::Cancel;
        });
        QVERIFY(saveInto(w, dir, QStringLiteral("a.odt")));
        QVERIFY(!asked);
    }
};

QTEST_MAIN(FileTests)
#include "tst_write_file.moc"
