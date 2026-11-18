// The rules comment threads share in every app (libs/page/core/threads.h): the exact caps, how Previous and Next step, how a thread is found from a reply, and the twelve-step script
// (tests/pub/threadscript.h) run against the comments of a page, which JeffPub and JeffDeck share. JeffWrite's and JeffCrunch's review tests run the same script.
#include "pubhelpers.h"
#include "../isolatesettings.h"
#include "threadscript.h"
#include "core/comments.h"
#include "core/threads.h"
#include "sheet/threads.h"

#include <QFile>
#include <QTemporaryDir>

namespace {

const comments::CommentAuthor kWho{QStringLiteral("Zed Ypsilon"), QStringLiteral("ZY")};

// The page comments of a two-page publication, as the script's adapter sees them.
struct PageThreads {
    std::unique_ptr<jp::Document> doc = jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);

    jp::Page &page() { return *doc->pages[0]; }
    QVector<comments::Comment> all() { return comments::commentsOf(page()); }
    // The id of comment e of thread t, empty when there is none.
    QString idOf(int t, int e)
    {
        const QVector<comments::Comment> list = all();
        if (t < 0 || t >= list.size()) return QString();
        if (e == 0) return list[t].id;
        return e - 1 < list[t].replies.size() ? list[t].replies[e - 1].id : QString();
    }

    bool start(const QString &text) { return !comments::addComment(page(), kWho, text, QPointF(10, 10 + 20 * all().size())).isEmpty(); }
    bool reply(int t, int e, const QString &text)
    {
        const QString id = idOf(t, e);
        return !id.isEmpty() && !comments::addReply(page(), id, kWho, text).isEmpty();
    }
    bool edit(int t, int e, const QString &text) { return comments::editComment(page(), idOf(t, e), text); }
    bool resolve(int t, bool done) { return comments::resolveComment(page(), idOf(t, 0), done); }
    bool remove(int t, int e) { return comments::deleteComment(page(), idOf(t, e)); }
    void removeAll() { comments::deleteAllComments(*doc); }
    QStringList state()
    {
        QStringList out;
        for (const comments::Comment &c : all()) {
            QStringList words{c.text};
            for (const comments::CommentReply &r : c.replies) words << r.text;
            out << words.join(QLatin1String(" | ")) + (c.done ? QStringLiteral(" [resolved]") : QString());
        }
        return out;
    }
    QString author(int t, int e)
    {
        const QVector<comments::Comment> list = all();
        return e == 0 ? list[t].author : list[t].replies[e - 1].author;
    }
    int step(int t, bool forward)
    {
        const QVector<comments::CommentRef> refs = comments::allComments(*doc);
        const comments::CommentRef to = comments::stepComment(*doc, refs[t], forward);
        return to.slide < 0 ? -1 : int(refs.indexOf(to));
    }
    int entries(int t) { return 1 + int(all()[t].replies.size()); }
    int threadCount() { return int(all().size()); }
};

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

} // namespace

class PubThreadTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }
    void cleanup() { jp::threads::setCapsForTest(0, 0); }

    // The caps are these numbers, in every app: the slide reader's limits and the sheet reader's text limit are the same ones.
    void theCapsAreTheSharedOnes()
    {
        QCOMPARE(jp::threads::kMaxThreads, 2000);
        QCOMPARE(jp::threads::kMaxReplies, 200);
        QCOMPARE(jp::threads::kMaxChars, 32767);
        QCOMPARE(comments::kMaxComments, jp::threads::kMaxThreads);
        QCOMPARE(comments::kMaxReplies, jp::threads::kMaxReplies);
        QCOMPARE(comments::kMaxCommentText, jp::threads::kMaxChars);
        QCOMPARE(jp::office::sheet::kMaxThreadCommentChars, jp::threads::kMaxChars);
        using R = jp::threads::Refusal;
        QCOMPARE(jp::threads::checkNewThread(QString(jp::threads::kMaxChars, QLatin1Char('x')), jp::threads::kMaxThreads - 1), R::None);
        QCOMPARE(jp::threads::checkNewThread(QStringLiteral("x"), jp::threads::kMaxThreads), R::TooManyThreads);
        QCOMPARE(jp::threads::checkNewThread(QString(jp::threads::kMaxChars + 1, QLatin1Char('x')), 0), R::TooLong);
        QCOMPARE(jp::threads::checkReply(QStringLiteral("x"), jp::threads::kMaxReplies - 1), R::None);
        QCOMPARE(jp::threads::checkReply(QStringLiteral("x"), jp::threads::kMaxReplies), R::TooManyReplies);
        QCOMPARE(jp::threads::checkReply(QStringLiteral("  "), 0, true), R::Empty);
        QCOMPARE(jp::threads::checkReply(QStringLiteral("  "), 0, false), R::None);
        QVERIFY(jp::threads::refusalText(R::TooManyReplies).contains(QStringLiteral("200")));
        QVERIFY(jp::threads::refusalText(R::TooLong).contains(QStringLiteral("32767")));
    }

    void stepsAndRootsFollowTheRules()
    {
        const QVector<qint64> keys{30, 10, 20};   // (in any order)
        QCOMPARE(jp::threads::stepIndex(keys, 10, true), 2);
        QCOMPARE(jp::threads::stepIndex(keys, 20, true), 0);
        QCOMPARE(jp::threads::stepIndex(keys, 30, true), -1);
        QCOMPARE(jp::threads::stepIndex(keys, 25, false), 2);
        QCOMPARE(jp::threads::stepIndex(keys, 10, false), -1);
        QCOMPARE(jp::threads::stepIndex({}, 0, true), -1);
        // a loop of replies ends, and a comment with no parent is its own root
        const QHash<QString, QString> parent{{"a", "b"}, {"b", "c"}, {"c", "a"}, {"x", "y"}};
        const auto of = [&](const QString &id) { return parent.value(id); };
        QCOMPARE(jp::threads::threadRoot(QStringLiteral("y"), of), QStringLiteral("y"));
        QCOMPARE(jp::threads::threadRoot(QStringLiteral("x"), of), QStringLiteral("y"));
        QVERIFY(!jp::threads::threadRoot(QStringLiteral("a"), of, 10).isEmpty());
    }

    void aPageRunsTheScript()
    {
        PageThreads p;
        threadscript::run(p, kWho.name);
    }

    // Resolving a reply resolves the thread; a reply to a reply is in the first comment's thread.
    void aReplyIsInTheThreadOfItsComment()
    {
        PageThreads p;
        QVERIFY(p.start(QStringLiteral("Q")));
        QVERIFY(p.reply(0, 0, QStringLiteral("R1")));
        QVERIFY(comments::resolveComment(p.page(), p.idOf(0, 1), true));
        QVERIFY(p.all()[0].done);
        QVERIFY(comments::resolveComment(p.page(), p.idOf(0, 1), false));
        QVERIFY(!p.all()[0].done);
        QVERIFY(p.reply(0, 1, QStringLiteral("R2")));
        QCOMPARE(p.threadCount(), 1);
        QCOMPARE(p.entries(0), 3);
    }

    // The .jpub keeps replies both ways, and a file whose comments nobody changed is written again with the same bytes.
    void jpubKeepsRepliesAndAnUnchangedFileIsTheSame()
    {
        QTemporaryDir dir;
        PageThreads p;
        QVERIFY(p.start(QStringLiteral("Q")));
        QVERIFY(p.reply(0, 0, QStringLiteral("R1")));
        QVERIFY(p.reply(0, 1, QStringLiteral("R2")));
        QVERIFY(p.resolve(0, true));
        QVERIFY(p.start(QStringLiteral("Second")));
        const QString one = dir.filePath(QStringLiteral("one.jpub")), two = dir.filePath(QStringLiteral("two.jpub"));
        QString err;
        QVERIFY2(jp::savePublication(*p.doc, one, QImage(), &err), qPrintable(err));
        auto back = jp::loadPublication(one, &err);
        QVERIFY2(back, qPrintable(err));
        PageThreads q;
        q.doc = std::move(back);
        QCOMPARE(q.state(), (QStringList{QStringLiteral("Q | R1 | R2 [resolved]"), QStringLiteral("Second")}));
        QCOMPARE(q.author(0, 2), kWho.name);
        QVERIFY2(jp::savePublication(*q.doc, two, QImage(), &err), qPrintable(err));
        auto again = jp::loadPublication(two, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(again->pages[0]->props.value(QStringLiteral("comments")), p.page().props.value(QStringLiteral("comments")));
        QVERIFY(!fileBytes(two).isEmpty());
        QCOMPARE(fileBytes(two), fileBytes(one));
    }
};

JP_TEST_CLASS(PubThreadTests)
#include "tst_pub_threads.moc"
