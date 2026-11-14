// JeffPub's tests of comments: on an object or a place of a page, the Comments pane, Undo, the .jpub file, what a .pub file and a PDF leave out, and the limits on a file from a stranger.
#include "pubhelpers.h"
#include "../isolatesettings.h"
#include "app/commentspane.h"
#include "app/infopage.h"
#include "core/comments.h"
#include "render/pdfpage.h"
#include "templates/templates.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>
#include <QTimer>
#include <QFrame>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTemporaryDir>

namespace {

const comments::CommentAuthor kPat{QStringLiteral("Pat Doe"), QStringLiteral("PD")};

// A text box holding `text`, on the first page; returns its id.
QString addTextBox(jp::Document &d, const QString &text, const QRectF &rect)
{
    auto t = std::make_shared<jp::TextItem>();
    t->rect = rect;
    t->storyId = d.createStory(text);
    d.pages[0]->items.push_back(t);
    return t->id;
}

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// Answers the next message box (looking for it every 20 ms) with OK; `said` gets its text.
struct BoxWatch {
    explicit BoxWatch(QString *said)
    {
        m_timer = new QTimer(qApp);
        QObject::connect(m_timer, &QTimer::timeout, m_timer, [this, said] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                *said = box->text();
                if (QAbstractButton *b = box->button(QMessageBox::Ok)) b->click();
                else box->accept();
                m_timer->stop();
            }
        });
        m_timer->start(20);
    }
    ~BoxWatch() { delete m_timer; }
    QPointer<QTimer> m_timer;
};

const jp::InspectItem *findItem(const QVector<jp::InspectItem> &items, const char *title)
{
    for (const jp::InspectItem &i : items)
        if (i.title == QLatin1String(title)) return &i;
    return nullptr;
}

} // namespace

class PubCommentTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }
    void init()
    {
        jp::Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Pat Doe"));
        jp::Settings::get().setValue(QStringLiteral("user/initials"), QStringLiteral("PD"));
    }

    // New Comment with an object selected attaches the comment to it; with nothing selected it stands at a point of the page. Reply, edit, resolve, and
    // delete each make one undo step, and Undo and Redo bring each back.
    void addReplyResolveAndDeleteWithUndo()
    {
        jp::MainWindow w;
        w.resize(1280, 800);
        w.show();
        jp::Editor *ed = w.editor();
        jp::CommentsPane *pane = w.commentsPane();
        QVERIFY(pane);
        QVERIFY(pane->isHidden());
        QString boxId;
        ed->change(QStringLiteral("Add"), [&] { boxId = addTextBox(*ed->doc(), QStringLiteral("Headline"), QRectF(100, 120, 200, 60)); });
        auto page = [&]() -> jp::Page & { return *ed->doc()->pages[ed->currentPage()]; };
        auto card = [&](int i = 0) { return pane->findChildren<QFrame *>(QStringLiteral("commentCard")).value(i); };
        const int baseSteps = ed->undoStack()->count();

        // New Comment on the selected object.
        ed->select(boxId);
        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(!pane->isHidden());
        QVERIFY(w.act(QStringLiteral("comment.show"))->isChecked());
        auto *draft = pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"));
        QVERIFY(draft && draft->isVisible());
        draft->setPlainText(QStringLiteral("Shorter headline?"));
        QCOMPARE(comments::commentCount(page()), 0);   // not until Post
        pane->findChild<QPushButton *>(QStringLiteral("commentPost"))->click();
        QCOMPARE(comments::commentCount(page()), 1);
        QCOMPARE(ed->undoStack()->count(), baseSteps + 1);
        const comments::Comment c = comments::commentsOf(page())[0];
        QCOMPARE(c.itemId, boxId);
        QCOMPARE(c.at, QPointF(300, 120));   // the object's top right corner
        QCOMPARE(c.author, QStringLiteral("Pat Doe"));
        QCOMPARE(c.initials, QStringLiteral("PD"));
        QCOMPARE(c.text, QStringLiteral("Shorter headline?"));
        QVERIFY(c.date.isValid() && qAbs(c.date.secsTo(QDateTime::currentDateTimeUtc())) < 60);
        QCOMPARE(pane->currentComment(), c.id);

        // Undo takes the comment away, Redo brings it back whole.
        ed->undo();
        QCOMPARE(comments::commentCount(page()), 0);
        QCOMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        ed->redo();
        QCOMPARE(comments::commentCount(page()), 1);
        QCOMPARE(comments::commentsOf(page())[0].text, QStringLiteral("Shorter headline?"));
        QCOMPARE(comments::commentsOf(page())[0].itemId, boxId);
        QCOMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 1);

        // A reply.
        auto *reply = card()->findChild<QLineEdit *>(QStringLiteral("commentReplyEdit"));
        QVERIFY(reply);
        reply->setText(QStringLiteral("Agreed"));
        card()->findChild<QAbstractButton *>(QStringLiteral("commentReplyPost"))->click();
        QCOMPARE(int(comments::commentsOf(page())[0].replies.size()), 1);
        QCOMPARE(comments::commentsOf(page())[0].replies[0].text, QStringLiteral("Agreed"));
        QCOMPARE(comments::commentsOf(page())[0].replies[0].author, QStringLiteral("Pat Doe"));
        QCOMPARE(ed->undoStack()->count(), baseSteps + 2);
        ed->undo();
        QVERIFY(comments::commentsOf(page())[0].replies.isEmpty());
        ed->redo();
        QCOMPARE(int(comments::commentsOf(page())[0].replies.size()), 1);

        // Edit.
        card()->findChild<QAbstractButton *>(QStringLiteral("commentEdit"))->click();
        auto *editor = card()->findChild<QPlainTextEdit *>(QStringLiteral("commentEditor"));
        QVERIFY(editor);
        editor->setPlainText(QStringLiteral("Shorter headline, please"));
        card()->findChild<QPushButton *>(QStringLiteral("commentSave"))->click();
        QCOMPARE(comments::commentsOf(page())[0].text, QStringLiteral("Shorter headline, please"));
        QCOMPARE(ed->undoStack()->count(), baseSteps + 3);
        ed->undo();
        QCOMPARE(comments::commentsOf(page())[0].text, QStringLiteral("Shorter headline?"));
        ed->redo();

        // Resolve, then Reopen.
        card()->findChild<QPushButton *>(QStringLiteral("commentResolve"))->click();
        QVERIFY(comments::commentsOf(page())[0].done);
        QVERIFY(card()->findChild<QLabel *>(QStringLiteral("commentResolved")));
        ed->undo();
        QVERIFY(!comments::commentsOf(page())[0].done);
        ed->redo();
        QVERIFY(comments::commentsOf(page())[0].done);
        card()->findChild<QPushButton *>(QStringLiteral("commentResolve"))->click();   // (Reopen)
        QVERIFY(!comments::commentsOf(page())[0].done);

        // Delete.
        const int before = ed->undoStack()->count();
        card()->findChild<QAbstractButton *>(QStringLiteral("commentDelete"))->click();
        QCOMPARE(comments::commentCount(page()), 0);
        QCOMPARE(ed->undoStack()->count(), before + 1);
        ed->undo();
        QCOMPARE(comments::commentCount(page()), 1);
        QCOMPARE(int(comments::commentsOf(page())[0].replies.size()), 1);
        ed->redo();
        QCOMPARE(comments::commentCount(page()), 0);

        // With nothing selected the comment stands at a point of the page, attached to no object.
        ed->clearSelection();
        w.act(QStringLiteral("comment.new"))->trigger();
        pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->setPlainText(QStringLiteral("Whole page note"));
        pane->findChild<QPushButton *>(QStringLiteral("commentPost"))->click();
        QCOMPARE(comments::commentCount(page()), 1);
        const comments::Comment p = comments::commentsOf(page())[0];
        QVERIFY(p.itemId.isEmpty());
        const QSizeF size = ed->doc()->pageSize();
        QVERIFY(p.at.x() >= 0 && p.at.x() < size.width() && p.at.y() >= 0 && p.at.y() < size.height());
    }

    // Show Comments shows the markers on the page and hides them; a marker stands at its object's top right corner and follows the object; a click on it opens the comment.
    void markersShowOnThePageAndFollowTheirObject()
    {
        jp::MainWindow w;
        w.resize(1280, 800);
        w.show();
        jp::Editor *ed = w.editor();
        QString boxId;
        ed->change(QStringLiteral("Add"), [&] { boxId = addTextBox(*ed->doc(), QStringLiteral("Headline"), QRectF(100, 120, 200, 60)); });
        const QString onObject = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("On the box"), QPointF(300, 120), QDateTime::currentDateTimeUtc(), boxId);
        const QString onPage = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("On the page"), QPointF(50, 400));
        ed->notifyLive();
        jp::Canvas *canvas = w.canvas();
        auto hit = [&](const QPointF &place) { return canvas->commentAt(canvas->pageToView(place) + QPointF(6, 6)); };
        // Not shown: no marker to click.
        QVERIFY(!canvas->commentsShown());
        QVERIFY(hit(QPointF(300, 120)).isEmpty());
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(canvas->commentsShown());
        QVERIFY(!w.commentsPane()->isHidden());
        QCOMPARE(hit(QPointF(300, 120)), onObject);
        QCOMPARE(hit(QPointF(50, 400)), onPage);
        // The object moves, and its comment's marker with it.
        ed->change(QStringLiteral("Move"), [&] { ed->doc()->item(boxId)->rect.translate(40, 90); });
        QCOMPARE(comments::commentPlace(*ed->doc()->pages[0], *comments::findComment(*ed->doc()->pages[0], onObject)), QPointF(340, 210));
        QCOMPARE(hit(QPointF(340, 210)), onObject);
        QVERIFY(hit(QPointF(300, 120)).isEmpty());
        // A click on a marker opens its comment in the pane.
        const QPointF at = canvas->pageToView(QPointF(50, 400)) + QPointF(6, 6);
        QTest::mouseClick(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, at.toPoint());
        QCOMPARE(w.commentsPane()->currentComment(), onPage);
        // The object goes: its comment stays where it was last put.
        ed->change(QStringLiteral("Delete"), [&] { ed->doc()->pages[0]->items.clear(); });
        QCOMPARE(comments::commentPlace(*ed->doc()->pages[0], *comments::findComment(*ed->doc()->pages[0], onObject)), QPointF(300, 120));
        // Hidden again.
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(!canvas->commentsShown());
        QVERIFY(w.commentsPane()->isHidden());
        QVERIFY(hit(QPointF(50, 400)).isEmpty());
    }

    // A page with two comments and the pane open: two cards in the pane, and a picture of the window when JP_SHOT_DIR names a folder.
    void twoCommentsShowInThePaneAndOnThePage()
    {
        jp::MainWindow w;
        w.resize(1280, 820);
        w.show();
        jp::Editor *ed = w.editor();
        QString boxId;
        ed->change(QStringLiteral("Add"), [&] {
            boxId = addTextBox(*ed->doc(), QStringLiteral("Spring Sale\nEverything half price, Saturday only."), QRectF(72, 72, 400, 120));
            auto shape = std::make_shared<jp::ShapeItem>();
            shape->rect = QRectF(72, 260, 250, 160);
            ed->doc()->pages[0]->items.push_back(shape);
        });
        const QDateTime when = QDateTime::currentDateTimeUtc().addSecs(-3600);
        const QString first = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("Can the headline be shorter? It wraps on the card."), QPointF(472, 72), when, boxId);
        comments::addReply(*ed->doc()->pages[0], first, comments::CommentAuthor{QStringLiteral("Lee Roy"), QStringLiteral("LR")}, QStringLiteral("Yes, I will cut it to two words."), when.addSecs(900));
        comments::addComment(*ed->doc()->pages[0], comments::CommentAuthor{QStringLiteral("Lee Roy"), QStringLiteral("LR")}, QStringLiteral("Use the new logo here."), QPointF(330, 300), when.addSecs(1800));
        ed->notifyLive();
        w.act(QStringLiteral("comment.show"))->trigger();
        QTRY_COMPARE(int(w.commentsPane()->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 2);
        w.commentsPane()->setCurrentComment(first);
        QCoreApplication::processEvents();
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            QDir().mkpath(qEnvironmentVariable("JP_SHOT_DIR"));
            QVERIFY(w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/pub-comments.png")));
        }
    }

    // A publication without comments, put in a window that showed the Comments pane, does not keep the pane open.
    void aPublicationWithoutCommentsClosesThePane()
    {
        jp::MainWindow w;
        w.resize(1280, 820);
        w.show();
        w.showComments(true);
        QVERIFY(w.commentsPane()->isVisible());
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QVERIFY(!w.commentsPane()->isVisible());
        QVERIFY(!w.act(QStringLiteral("comment.show"))->isChecked());
        QVERIFY(!w.canvas()->commentsShown());
    }

    // A language without its own Comments topic shows the English one.
    void commentsHelpFallsBackToEnglish()
    {
        const auto say = [](const char *lang) {
            jp::Settings::get().setValue(QStringLiteral("ui/language"), QLatin1String(lang));
            return jp::installTranslations();
        };
        // (a language with the page shows its own; one without it, as British English, which holds only the pages that differ, shows the English: so the fallback is tested
        // however much is translated)
        int fellBack = 0;
        for (const char *lang : {"de", "fr", "pt_BR", "en_GB"}) {
            say(lang);
            const bool has = QFile::exists(QStringLiteral(JP_TEST_DATA "/../../resources/help/") + QLatin1String(lang) + QStringLiteral("/comments.md"));
            const jp::help::Topic t = jp::help::topic(QStringLiteral("comments"));
            QVERIFY2(!t.title.isEmpty(), lang);
            if (has) continue;
            ++fellBack;
            QCOMPARE(t.title, QStringLiteral("Comments"));
            QVERIFY(t.markdown.contains(QLatin1String("New Comment")));
        }
        if (fellBack == 0) {
            // (every language has this page now: the fallback is checked with a page British English, which holds only the pages that differ, leaves to the English)
            const QString help = QStringLiteral(JP_TEST_DATA "/../../resources/help/");
            QString missing;
            for (const QFileInfo &f : QDir(help + QStringLiteral("en")).entryInfoList({QStringLiteral("*.md")}, QDir::Files, QDir::Name)) {
                if (!QFile::exists(help + QStringLiteral("en_GB/") + f.fileName())) { missing = f.completeBaseName(); break; }
            }
            QVERIFY(!missing.isEmpty());
            say("en");
            const QString english = jp::help::topic(missing).title;
            QVERIFY(!english.isEmpty());
            say("en_GB");
            QCOMPARE(jp::help::topic(missing).title, english);
            ++fellBack;
        }
        QVERIFY(fellBack > 0);
        say("en");
    }

    // The marker of a comment on a place of the page can be dragged, and the step Undo names is in the language of the window (JeffDeck's translation of the same step).
    void movingACommentIsNamedInTheWindowsLanguage()
    {
        const auto say = [](const char *lang) {
            jp::Settings::get().setValue(QStringLiteral("ui/language"), QLatin1String(lang));
            return jp::installTranslations();
        };
        QCOMPARE(say("de"), QStringLiteral("de"));
        const QString german = QCoreApplication::translate("jp::DeckCanvas", "Move Comment");
        QVERIFY(german != QLatin1String("Move Comment"));
        {
            jp::MainWindow w;
            w.resize(1280, 800);
            w.show();
            jp::Editor *ed = w.editor();
            const QString id = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("Drag me"), QPointF(100, 400));
            ed->notifyLive();
            w.act(QStringLiteral("comment.show"))->trigger();
            jp::Canvas *canvas = w.canvas();
            const QPoint from = (canvas->pageToView(QPointF(100, 400)) + QPointF(6, 6)).toPoint();
            QTest::mousePress(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, from);
            QTest::mouseMove(canvas->viewport(), from + QPoint(40, 30));
            QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, from + QPoint(40, 30));
            QVERIFY(comments::findComment(*ed->doc()->pages[0], id)->at != QPointF(100, 400));
            QCOMPARE(ed->undoStack()->undoText(), german);
        }
        say("en");
    }

    // A stranger's file: a comment's place is brought to the page, ids that are empty or used twice are made anew, and an object that is not on the page is let go.
    void hostileFileIsTidied()
    {
        QTemporaryDir dir;
        auto doc = jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);
        const QString boxId = addTextBox(*doc, QStringLiteral("Headline"), QRectF(100, 120, 200, 60));
        const QString onOtherPage = QStringLiteral("not-on-page-one");
        auto reply = [](const QString &id) { return QJsonObject{{"id", id}, {"author", QStringLiteral("A")}, {"initials", QStringLiteral("A")}, {"text", QStringLiteral("r")}}; };
        auto comment = [&](const QString &id, double x, double y, const QString &item, const QJsonArray &replies) {
            QJsonObject o{{"id", id}, {"author", QStringLiteral("A")}, {"initials", QStringLiteral("A")}, {"text", QStringLiteral("c")}, {"x", x}, {"y", y}, {"replies", replies}};
            if (!item.isEmpty()) o["item"] = item;
            return o;
        };
        QJsonArray list;
        list.append(comment(QStringLiteral("dup"), 1e9, -1e9, onOtherPage, QJsonArray{reply(QStringLiteral("dup")), reply(QString()), reply(QStringLiteral("dup"))}));
        list.append(comment(QString(), 50, 60, boxId, {}));
        list.append(comment(QStringLiteral("dup"), 70, 80, QString(), {}));
        doc->pages[0]->props[QStringLiteral("comments")] = list;
        QString err;
        const QString path = dir.filePath(QStringLiteral("odd.jpub"));
        QVERIFY2(jp::savePublication(*doc, path, QImage(), &err), qPrintable(err));
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        const QVector<comments::Comment> got = comments::commentsOf(*back->pages[0]);
        QCOMPARE(int(got.size()), 3);
        QSet<QString> ids;
        for (const comments::Comment &c : got) {
            QVERIFY(!c.id.isEmpty());
            QVERIFY2(!ids.contains(c.id), "an id is used twice");
            ids << c.id;
            for (const comments::CommentReply &r : c.replies) {
                QVERIFY(!r.id.isEmpty());
                QVERIFY2(!ids.contains(r.id), "an id is used twice");
                ids << r.id;
            }
        }
        QCOMPARE(got[0].at, QPointF(612 + comments::kPlaceMargin, -comments::kPlaceMargin));
        QVERIFY(got[0].itemId.isEmpty());   // (an object that is not on the page)
        QCOMPARE(got[1].itemId, boxId);     // (one that is)
        QCOMPARE(got[2].at, QPointF(70, 80));
    }

    // All the replies of a file, and all the text of its comments, are held to a total as well.
    void hostileFileIsHeldToTotalRepliesAndText()
    {
        QTemporaryDir dir;
        auto doc = jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 1);
        QJsonArray list;
        const int threads = 150;   // (200 replies each: 30,000 in all)
        for (int i = 0; i < threads; ++i) {
            QJsonArray rs;
            for (int r = 0; r < comments::kMaxReplies; ++r) rs.append(QJsonObject{{"id", QStringLiteral("r%1_%2").arg(i).arg(r)}, {"author", QStringLiteral("A")}, {"initials", QStringLiteral("A")}, {"text", QStringLiteral("r")}});
            list.append(QJsonObject{{"id", QStringLiteral("c%1").arg(i)}, {"author", QStringLiteral("A")}, {"initials", QStringLiteral("A")}, {"text", QStringLiteral("c")}, {"x", 1}, {"y", 1}, {"replies", rs}});
        }
        // ... and long texts: a little over the total of what a file may hold.
        const int longOnes = comments::kMaxTotalText / comments::kMaxCommentText + 40;
        for (int i = 0; i < longOnes; ++i)
            list.append(QJsonObject{{"id", QStringLiteral("t%1").arg(i)}, {"author", QStringLiteral("A")}, {"initials", QStringLiteral("A")}, {"text", QString(comments::kMaxCommentText, QLatin1Char('t'))}, {"x", 1}, {"y", 1}});
        doc->pages[0]->props[QStringLiteral("comments")] = list;
        QString err;
        const QString path = dir.filePath(QStringLiteral("big.jpub"));
        QVERIFY2(jp::savePublication(*doc, path, QImage(), &err), qPrintable(err));
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        qint64 replies = 0, text = 0;
        for (const comments::Comment &c : comments::commentsOf(*back->pages[0])) {
            replies += c.replies.size();
            text += c.text.size();
            for (const comments::CommentReply &r : c.replies) text += r.text.size();
        }
        QCOMPARE(replies, qint64(comments::kMaxTotalReplies));
        QVERIFY2(text <= comments::kMaxTotalText, qPrintable(QString::number(text)));
        QVERIFY2(text > comments::kMaxTotalText - 2 * comments::kMaxCommentText, qPrintable(QString::number(text)));
    }

    // Drawing and clicking use a list kept for the page: 2,000 comments with objects that are not there are not looked for again at each paint.
    void markersAreNotRescannedAtEachPaint()
    {
        jp::MainWindow w;
        w.resize(1280, 800);
        w.show();
        jp::Editor *ed = w.editor();
        QString boxId;
        ed->change(QStringLiteral("Add"), [&] { boxId = addTextBox(*ed->doc(), QStringLiteral("Headline"), QRectF(100, 120, 200, 60)); });
        QJsonArray list;
        for (int i = 0; i < comments::kMaxComments; ++i)
            list.append(QJsonObject{{"id", QStringLiteral("c%1").arg(i)}, {"author", QStringLiteral("A")}, {"initials", QStringLiteral("A")}, {"text", QStringLiteral("c")},
                                    {"x", 10 + i % 50}, {"y", 10 + i / 50}, {"item", QStringLiteral("bogus%1").arg(i)}, {"replies", QJsonArray()}});
        ed->doc()->pages[0]->props[QStringLiteral("comments")] = list;
        ed->notifyLive();
        w.act(QStringLiteral("comment.show"))->trigger();
        jp::Canvas *canvas = w.canvas();
        canvas->grab();
        const int builds = comments::commentMarkBuilds(), scans = comments::commentItemScans();
        QVERIFY(builds >= 1);
        for (int i = 0; i < 5; ++i) {
            canvas->grab();
            canvas->commentAt(canvas->pageToView(QPointF(30, 30)));
        }
        QCOMPARE(comments::commentMarkBuilds(), builds);
        QCOMPARE(comments::commentItemScans(), scans);
        // A change of the page makes the list again, with one look at the objects, not one for each of the comments.
        ed->notifyLive();
        canvas->grab();
        QCOMPARE(comments::commentMarkBuilds(), builds + 1);
        QCOMPARE(comments::commentItemScans(), scans + 1);
    }

    // With Remove personal information when saving on, the file has no comment author or initials, the window keeps its own, and nothing is marked as changed.
    void personalInformationOutKeepsAuthorsOutOfTheFileOnly()
    {
        QTemporaryDir dir;
        struct Restore {
            ~Restore() { jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); }
        } restore;
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const QString first = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("Keep the words"), QPointF(40, 50));
        comments::addReply(*ed->doc()->pages[0], first, kPat, QStringLiteral("And this"));
        for (const bool on : {true, false}) {
            jp::Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), on);
            const QString path = dir.filePath(QStringLiteral("p%1.jpub").arg(on));
            const int steps = ed->undoStack()->index();
            QVERIFY(w.saveTo(path));
            QCOMPARE(ed->undoStack()->index(), steps);
            QVERIFY(!ed->isModified());
            QString err;
            auto back = jp::loadPublication(path, &err);
            QVERIFY2(back, qPrintable(err));
            const comments::Comment c = comments::commentsOf(*back->pages[0])[0];
            QCOMPARE(c.text, QStringLiteral("Keep the words"));
            QCOMPARE(c.replies[0].text, QStringLiteral("And this"));
            QCOMPARE(c.author.isEmpty(), on);
            QCOMPARE(c.initials.isEmpty(), on);
            QCOMPARE(c.replies[0].author.isEmpty(), on);
            QCOMPARE(c.replies[0].initials.isEmpty(), on);
            // The open publication is as it was.
            const comments::Comment mine = comments::commentsOf(*ed->doc()->pages[0])[0];
            QCOMPARE(mine.author, QStringLiteral("Pat Doe"));
            QCOMPARE(mine.replies[0].initials, QStringLiteral("PD"));
        }
    }

    // Change Template keeps the comments of each page: on the same page, with their replies and state, at the place they were (the object they were attached to is a new copy).
    void changeTemplateKeepsComments()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        QString boxId;
        ed->change(QStringLiteral("Add"), [&] { boxId = addTextBox(*ed->doc(), QStringLiteral("Headline words"), QRectF(100, 120, 200, 60)); });
        const QString first = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("Shorter?"), QPointF(300, 120), QDateTime::currentDateTimeUtc(), boxId);
        comments::addReply(*ed->doc()->pages[0], first, kPat, QStringLiteral("Yes"));
        comments::resolveComment(*ed->doc()->pages[0], first);
        QVERIFY(w.changeTemplate(jp::findTemplate(QStringLiteral("flyer-event"))->build(jp::TemplateOptions()), false));
        QCOMPARE(comments::commentCount(*ed->doc()->pages[0]), 1);
        const comments::Comment c = comments::commentsOf(*ed->doc()->pages[0])[0];
        QCOMPARE(c.text, QStringLiteral("Shorter?"));
        QCOMPARE(c.at, QPointF(300, 120));
        QVERIFY(c.itemId.isEmpty() || ed->doc()->item(c.itemId));   // (never a link to an object that is not there)
        QVERIFY(c.done);
        QCOMPARE(int(c.replies.size()), 1);
        ed->undo();   // (Change Template is one step)
        QCOMPARE(comments::commentsOf(*ed->doc()->pages[0])[0].itemId, boxId);
    }

    // A publication open for reading only keeps its comments: the commands that delete them stay off, whatever changes.
    void readOnlyWindowCannotDeleteComments()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("Mine"), QPointF(40, 50));
        ed->notifyLive();
        w.commentsPane()->setCurrentComment(comments::commentsOf(*ed->doc()->pages[0])[0].id);
        for (const char *id : {"comment.delete", "comment.deletePage", "comment.deleteAll"}) QVERIFY2(w.act(QLatin1String(id))->isEnabled(), id);
        w.setReadOnly(true);
        for (int round = 0; round < 2; ++round) {
            for (const char *id : {"comment.delete", "comment.deletePage", "comment.deleteAll"}) QVERIFY2(!w.act(QLatin1String(id))->isEnabled(), id);
            ed->notifyLive();   // (the comments changed, or so the window is told)
            w.commentsPane()->setCurrentComment(comments::commentsOf(*ed->doc()->pages[0])[0].id);
        }
        QVERIFY(w.act(QStringLiteral("comment.next"))->isEnabled());   // (looking is allowed)
        w.setReadOnly(false);
        QVERIFY(w.act(QStringLiteral("comment.deleteAll"))->isEnabled());
    }

    // Duplicate Page copies the page's comments, with ids of their own, each on the copy of its object.
    void duplicatePageCopiesItsComments()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        QString boxId;
        ed->change(QStringLiteral("Add"), [&] { boxId = addTextBox(*ed->doc(), QStringLiteral("Headline"), QRectF(100, 120, 200, 60)); });
        const QString first = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("Shorter?"), QPointF(300, 120), QDateTime::currentDateTimeUtc(), boxId);
        comments::addReply(*ed->doc()->pages[0], first, kPat, QStringLiteral("Yes"));
        ed->insertPages(0, 1, true, false);
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        QCOMPARE(comments::commentCount(*ed->doc()->pages[1]), 1);
        const comments::Comment c = comments::commentsOf(*ed->doc()->pages[1])[0];
        const comments::Comment original = comments::commentsOf(*ed->doc()->pages[0])[0];
        QCOMPARE(c.text, QStringLiteral("Shorter?"));
        QVERIFY(c.id != original.id);
        QVERIFY(c.replies[0].id != original.replies[0].id);
        QVERIFY(!c.itemId.isEmpty() && c.itemId != boxId);
        QVERIFY(ed->doc()->pages[1]->items.size() == 1 && ed->doc()->pages[1]->items[0]->id == c.itemId);
        QCOMPARE(comments::commentsOf(*ed->doc()->pages[0])[0].itemId, boxId);   // (the first page is as it was)
    }

    // Next and Previous go through the comments of every page, in order, and turn to their page.
    void nextAndPreviousVisitEveryPage()
    {
        jp::MainWindow w;
        w.resize(1280, 800);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 3));
        const QString a = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("One"), QPointF(10, 10));
        const QString b = comments::addComment(*ed->doc()->pages[2], kPat, QStringLiteral("Three"), QPointF(10, 10));
        ed->notifyLive();
        w.act(QStringLiteral("comment.next"))->trigger();
        QCOMPARE(ed->currentPage(), 0);
        QCOMPARE(w.commentsPane()->currentComment(), a);
        w.act(QStringLiteral("comment.next"))->trigger();
        QCOMPARE(ed->currentPage(), 2);
        QCOMPARE(w.commentsPane()->currentComment(), b);
        w.act(QStringLiteral("comment.prev"))->trigger();
        QCOMPARE(ed->currentPage(), 0);
        QCOMPARE(w.commentsPane()->currentComment(), a);
    }

    // The .jpub file keeps each comment whole: its text, author, date, place and object, its replies, and whether it is resolved.
    void jpubKeepsComments()
    {
        QTemporaryDir dir;
        auto doc = jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);
        const QString boxId = addTextBox(*doc, QStringLiteral("Headline"), QRectF(100, 120, 200, 60));
        const QDateTime noon(QDate(2026, 11, 9), QTime(12, 0, 5, 125), QTimeZone::UTC);
        const comments::CommentAuthor lee{QStringLiteral("Lee Roy"), QStringLiteral("LR")};
        const QString first = comments::addComment(*doc->pages[0], kPat, QStringLiteral("Shorter, \"quoted\" headline?\nSecond line é"), QPointF(300.5, 120.25), noon, boxId);
        const QString answer = comments::addReply(*doc->pages[0], first, lee, QStringLiteral("Agreed"), noon.addSecs(90));
        comments::resolveComment(*doc->pages[0], first);
        const QString second = comments::addComment(*doc->pages[1], lee, QStringLiteral("Page two"), QPointF(40, 50), noon.addSecs(600));
        const QString path = dir.filePath(QStringLiteral("c.jpub"));
        QString err;
        QVERIFY2(jp::savePublication(*doc, path, QImage(), &err), qPrintable(err));
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 2);
        QCOMPARE(comments::commentCount(*back->pages[0]), 1);
        const comments::Comment c = comments::commentsOf(*back->pages[0])[0];
        QCOMPARE(c.id, first);
        QCOMPARE(c.text, QStringLiteral("Shorter, \"quoted\" headline?\nSecond line é"));
        QCOMPARE(c.author, QStringLiteral("Pat Doe"));
        QCOMPARE(c.initials, QStringLiteral("PD"));
        QCOMPARE(c.date, noon);
        QCOMPARE(c.at, QPointF(300.5, 120.25));
        QCOMPARE(c.itemId, boxId);
        QVERIFY(c.done);
        QCOMPARE(int(c.replies.size()), 1);
        QCOMPARE(c.replies[0].id, answer);
        QCOMPARE(c.replies[0].text, QStringLiteral("Agreed"));
        QCOMPARE(c.replies[0].author, QStringLiteral("Lee Roy"));
        QCOMPARE(c.replies[0].initials, QStringLiteral("LR"));
        QCOMPARE(c.replies[0].date, noon.addSecs(90));
        const comments::Comment s = comments::commentsOf(*back->pages[1])[0];
        QCOMPARE(s.id, second);
        QVERIFY(s.itemId.isEmpty());
        QVERIFY(!s.done);
        QCOMPARE(s.date, noon.addSecs(600));
        QVERIFY(back->item(boxId));   // (the object the comment is attached to comes back with it)
        // The window that opens it shows them.
        jp::MainWindow w;
        QVERIFY(w.openFile(path));
        QVERIFY(!w.commentsPane()->isHidden());
        QVERIFY(w.canvas()->commentsShown());
    }

    // Saving as a .pub file: the copy report names the comments, and the file is the same as one without them. Opening a .pub file never makes comments.
    void pubFileNamesTheCommentsAndWritesNothingElse()
    {
        QTemporaryDir dir;
        auto make = [] {
            auto doc = jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);
            addTextBox(*doc, QStringLiteral("Headline"), QRectF(100, 120, 200, 60));
            return doc;
        };
        auto plain = make(), noted = make();
        const QString boxId = noted->pages[0]->items.front()->id;
        comments::addComment(*noted->pages[0], kPat, QStringLiteral("Shorter headline?"), QPointF(300, 120), QDateTime::currentDateTimeUtc(), boxId);
        const QString reply = comments::commentsOf(*noted->pages[0])[0].id;
        comments::addReply(*noted->pages[0], reply, kPat, QStringLiteral("Reply words"));
        comments::addComment(*noted->pages[1], kPat, QStringLiteral("Second page"), QPointF(40, 50));
        QString errPlain, errNoted;
        QVERIFY2(jp::exportPublisher(*plain, dir.filePath(QStringLiteral("plain.pub")), &errPlain), qPrintable(errPlain));
        QVERIFY2(jp::exportPublisher(*noted, dir.filePath(QStringLiteral("noted.pub")), &errNoted), qPrintable(errNoted));
        QVERIFY2(errPlain.isEmpty(), qPrintable(errPlain));
        QVERIFY2(errNoted.contains(QLatin1String("comments are kept only in .jpub")), qPrintable(errNoted));
        // Nothing of the comments is in the file, and nothing else differs.
        const QByteArray b = fileBytes(dir.filePath(QStringLiteral("noted.pub")));
        QVERIFY(!b.isEmpty());
        QVERIFY(!b.contains("Shorter headline?") && !b.contains("Reply words") && !b.contains("Second page"));
        QVERIFY(!QString::fromLatin1(b).contains(QLatin1String("Pat Doe")));
        // Opening both again: the same publication (its pages, objects, and words), and no comments in either.
        QString err;
        auto back = jp::importPublisherFile(dir.filePath(QStringLiteral("noted.pub")), &err);
        QVERIFY2(back, qPrintable(err));
        auto backPlain = jp::importPublisherFile(dir.filePath(QStringLiteral("plain.pub")), &err);
        QVERIFY2(backPlain, qPrintable(err));
        QCOMPARE(back->pages.size(), backPlain->pages.size());
        for (int i = 0; i < back->pages.size(); ++i) {
            QCOMPARE(back->pages[i]->items.size(), backPlain->pages[i]->items.size());
            for (int k = 0; k < back->pages[i]->items.size(); ++k) {
                const jp::ItemPtr &x = back->pages[i]->items[k], &y = backPlain->pages[i]->items[k];
                QCOMPARE(int(x->type()), int(y->type()));
                QCOMPARE(x->rect, y->rect);
                if (x->type() == jp::ItemType::Text)
                    QCOMPARE(back->storyDoc(static_cast<const jp::TextItem *>(x.get())->storyId)->toPlainText(), backPlain->storyDoc(static_cast<const jp::TextItem *>(y.get())->storyId)->toPlainText());
            }
        }
        for (const auto &page : back->pages) QCOMPARE(comments::commentCount(*page), 0);
        QVERIFY(comments::allComments(*back).isEmpty());
    }

    // The same through File > Save As: the person is told which comments the .pub file left out, and the window keeps them.
    void savingAPubFileSaysSoAndTheWindowKeepsTheComments()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("Keep me"), QPointF(40, 50));
        QString said;
        BoxWatch watch(&said);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("a.pub"))));
        QTRY_VERIFY(!said.isEmpty());
        QVERIFY2(said.contains(QLatin1String("comments are kept only in .jpub")), qPrintable(said));
        QCOMPARE(comments::commentCount(*ed->doc()->pages[0]), 1);
        // A .jpub file says nothing of the kind.
        said.clear();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("a.jpub"))));
        QVERIFY(said.isEmpty());
    }

    // A PDF holds no comment text, shown or not.
    void pdfHasNoCommentText()
    {
        QTemporaryDir dir;
        jp::MainWindow w;
        w.resize(1280, 800);
        w.show();
        jp::Editor *ed = w.editor();
        QString boxId;
        ed->change(QStringLiteral("Add"), [&] { boxId = addTextBox(*ed->doc(), QStringLiteral("Headline words"), QRectF(72, 72, 400, 60)); });
        const QString first = comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("Zebra comment words"), QPointF(472, 72), QDateTime::currentDateTimeUtc(), boxId);
        comments::addReply(*ed->doc()->pages[0], first, kPat, QStringLiteral("Giraffe reply words"));
        ed->notifyLive();
        for (const bool shown : {false, true}) {
            if (w.act(QStringLiteral("comment.show"))->isChecked() != shown) w.act(QStringLiteral("comment.show"))->trigger();
            const QString path = dir.filePath(QStringLiteral("c%1.pdf").arg(shown));
            QVERIFY(w.exportPdfTo(path, jp::MainWindow::PdfSettings()));
            const QByteArray pdf = fileBytes(path);
            QVERIFY(!pdf.isEmpty());
            const QString text = jp::PdfDocument(pdf).text(0);
            QVERIFY2(text.contains(QLatin1String("Headline words")), qPrintable(text));   // (the page's own text is read, so that the absence of the rest means something)
            QVERIFY2(!text.contains(QLatin1String("Zebra")) && !text.contains(QLatin1String("Giraffe")) && !text.contains(QLatin1String("Pat Doe")), qPrintable(text));
        }
    }

    // A file from a stranger can't hold more than the limits: comments in the document, replies to one comment, and characters in one.
    void hostileFileIsHeldToTheLimits()
    {
        QVERIFY(comments::kMaxComments > 0 && comments::kMaxReplies > 0 && comments::kMaxCommentText > 0);
        QTemporaryDir dir;
        auto doc = jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);
        auto entry = [](int i, int replies, int textSize) {
            QJsonArray rs;
            for (int r = 0; r < replies; ++r) rs.append(QJsonObject{{"id", QStringLiteral("r%1").arg(r)}, {"author", QStringLiteral("A")}, {"initials", QStringLiteral("A")}, {"text", QString(textSize, QLatin1Char('y'))}});
            return QJsonObject{{"id", QStringLiteral("c%1").arg(i)},      {"author", QString(5000, QLatin1Char('n'))}, {"initials", QString(400, QLatin1Char('i'))},
                               {"text", QString(textSize, QLatin1Char('x'))}, {"x", 10}, {"y", 10},                    {"replies", rs}};
        };
        const int onFirst = comments::kMaxComments - 100, onSecond = 400;   // (together 300 more than the limit)
        QVERIFY(onFirst + onSecond > comments::kMaxComments);
        QJsonArray first, second;
        first.append(entry(0, comments::kMaxReplies + 300, comments::kMaxCommentText + 5000));
        first.append(7);   // (not a comment at all)
        for (int i = 1; i < onFirst; ++i) first.append(entry(i, 0, 5));
        for (int i = 0; i < onSecond; ++i) second.append(entry(onFirst + i, 0, 5));
        doc->pages[0]->props[QStringLiteral("comments")] = first;
        doc->pages[1]->props[QStringLiteral("comments")] = second;
        QString err;
        const QString path = dir.filePath(QStringLiteral("hostile.jpub"));
        QVERIFY2(jp::savePublication(*doc, path, QImage(), &err), qPrintable(err));
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        int total = 0;
        for (const auto &page : back->pages) total += comments::commentCount(*page);
        QCOMPARE(total, comments::kMaxComments);
        QCOMPARE(comments::commentCount(*back->pages[0]), onFirst);   // (the entry that was not a comment is gone; the first pages keep theirs)
        QCOMPARE(comments::commentCount(*back->pages[1]), comments::kMaxComments - onFirst);
        const comments::Comment c = comments::commentsOf(*back->pages[0])[0];
        QCOMPARE(int(c.replies.size()), comments::kMaxReplies);
        QCOMPARE(int(c.text.size()), comments::kMaxCommentText);
        QCOMPARE(int(c.replies[0].text.size()), comments::kMaxCommentText);
        QVERIFY(c.author.size() <= 200);
        QVERIFY(c.initials.size() <= 8);
        // (a window that opens it holds up as well)
        jp::MainWindow w;
        QVERIFY(w.openFile(path));
    }

    // Inspect Publication lists the comments (their authors are names of people) and Remove All takes them out as one undo step.
    void inspectListsAndRemovesComments()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2));
        // (the list is kept: a pointer into the list inspectItems() returns would point at freed memory once that list is gone)
        const QVector<jp::InspectItem> before = w.inspectItems();
        const jp::InspectItem *none = findItem(before, "Comments");
        QVERIFY(none);
        QCOMPARE(none->count(), 0);
        comments::addComment(*ed->doc()->pages[0], kPat, QStringLiteral("One"), QPointF(10, 10));
        comments::addComment(*ed->doc()->pages[1], kPat, QStringLiteral("Two"), QPointF(10, 10));
        const QVector<jp::InspectItem> items = w.inspectItems();
        const jp::InspectItem *row = findItem(items, "Comments");
        QVERIFY(row);
        QVERIFY(row->removable);
        QCOMPARE(row->count(), 2);
        const int steps = ed->undoStack()->count();
        QCOMPARE(row->remove(), 2);
        QCOMPARE(row->count(), 0);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        ed->undo();
        QCOMPARE(row->count(), 2);
    }
};

JP_TEST_CLASS(PubCommentTests)
#include "tst_pub_comments.moc"
