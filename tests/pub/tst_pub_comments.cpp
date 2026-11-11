// JeffPub's tests of comments: on an object or a place of a page, the Comments pane, Undo, the .jpub file, what a .pub file and a PDF leave out, and the limits on a file from a stranger.
#include "pubhelpers.h"
#include "../isolatesettings.h"
#include "app/commentspane.h"
#include "app/infopage.h"
#include "core/comments.h"
#include "render/pdfpage.h"

#include <QFile>
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
        QCOMPARE(comments::commentPlace(*ed->doc(), *comments::findComment(*ed->doc()->pages[0], onObject)), QPointF(340, 210));
        QCOMPARE(hit(QPointF(340, 210)), onObject);
        QVERIFY(hit(QPointF(300, 120)).isEmpty());
        // A click on a marker opens its comment in the pane.
        const QPointF at = canvas->pageToView(QPointF(50, 400)) + QPointF(6, 6);
        QTest::mouseClick(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, at.toPoint());
        QCOMPARE(w.commentsPane()->currentComment(), onPage);
        // The object goes: its comment stays where it was last put.
        ed->change(QStringLiteral("Delete"), [&] { ed->doc()->pages[0]->items.clear(); });
        QCOMPARE(comments::commentPlace(*ed->doc(), *comments::findComment(*ed->doc()->pages[0], onObject)), QPointF(300, 120));
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
        QVERIFY2(errNoted.contains(QLatin1String("2 comment")), qPrintable(errNoted));   // (the replies are not counted)
        // Nothing of the comments is in the file, and nothing else differs.
        const QByteArray a = fileBytes(dir.filePath(QStringLiteral("plain.pub"))), b = fileBytes(dir.filePath(QStringLiteral("noted.pub")));
        QVERIFY(!a.isEmpty());
        QVERIFY(!b.contains("Shorter headline?") && !b.contains("Reply words") && !b.contains("Second page"));
        QVERIFY(!QString::fromLatin1(b).contains(QLatin1String("Pat Doe")));
        QCOMPARE(b.size(), a.size());
        // Opening it again: no comments.
        QString err;
        auto back = jp::importPublisherFile(dir.filePath(QStringLiteral("noted.pub")), &err);
        QVERIFY2(back, qPrintable(err));
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
            QVERIFY(!pdf.contains("Zebra") && !pdf.contains("Giraffe") && !pdf.contains("Pat Doe"));
            QVERIFY(!pdf.contains("/Subtype /Text") && !pdf.contains("/Subtype/Text"));   // (no note annotation either)
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
        const jp::InspectItem *none = findItem(w.inspectItems(), "Comments");
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
