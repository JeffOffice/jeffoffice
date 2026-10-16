// JeffPub's tests of pictures: linked, online, dropped, and the building blocks.
#include "pubhelpers.h"

class PubPictureTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // A picture dragged onto another swaps with it, as the scratch area's
    // message says (nothing did): the dragged frame goes back, and the two
    // pictures trade frames.
    void pictureDroppedOnPictureSwaps()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        auto picture = [&](const QColor &c, const QRectF &r) {
            QImage img(40, 40, QImage::Format_RGB32);
            img.fill(c);
            QByteArray png;
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            img.save(&buf, "PNG");
            auto p = std::make_shared<jp::PictureItem>();
            p->imageId = ed->doc()->addImage(png, "png");
            p->rect = r;
            p->imgRect = QRectF(QPointF(0, 0), r.size());
            ed->addItem(p);
            return p;
        };
        auto red = picture(Qt::red, QRectF(72, 72, 144, 144));
        auto blue = picture(Qt::blue, QRectF(300, 300, 144, 144));
        const QString redImage = red->imageId, blueImage = blue->imageId;
        jp::Canvas *cv = w.canvas();
        QTest::qWait(50);
        const QPoint a = cv->pageToView(red->rect.center()).toPoint(), b = cv->pageToView(blue->rect.center()).toPoint();
        QTest::mousePress(cv->viewport(), Qt::LeftButton, Qt::NoModifier, a);
        for (int k = 1; k <= 4; ++k) {
            const QPoint at = a + (b - a) * k / 4;
            QMouseEvent mv(QEvent::MouseMove, QPointF(at), cv->viewport()->mapToGlobal(QPointF(at)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(cv->viewport(), &mv);
        }
        QTest::mouseRelease(cv->viewport(), Qt::LeftButton, Qt::NoModifier, b);
        QCOMPARE(red->rect, QRectF(72, 72, 144, 144));   // back where it was
        QCOMPARE(red->imageId, blueImage);
        QCOMPARE(blue->imageId, redImage);
        ed->undo();   // undo makes the objects afresh
        QCOMPARE(static_cast<jp::PictureItem *>(ed->doc()->item(red->id))->imageId, redImage);
    }

    // Ctrl+drag copies a picture, and a copy let go over another picture stays
    // where it was dropped (it swapped, which put the copy back over the
    // original, and gave the other picture the same image).
    void pictureCopiedOntoPictureDoesNotSwap()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        auto picture = [&](const QColor &c, const QRectF &r) {
            QImage img(40, 40, QImage::Format_RGB32);
            img.fill(c);
            QByteArray png;
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            img.save(&buf, "PNG");
            auto p = std::make_shared<jp::PictureItem>();
            p->imageId = ed->doc()->addImage(png, "png");
            p->rect = r;
            p->imgRect = QRectF(QPointF(0, 0), r.size());
            ed->addItem(p);
            return p;
        };
        auto red = picture(Qt::red, QRectF(72, 72, 144, 144));
        auto blue = picture(Qt::blue, QRectF(300, 300, 144, 144));
        const QString redImage = red->imageId, blueImage = blue->imageId;
        jp::Canvas *cv = w.canvas();
        ed->select(red->id);
        QTest::qWait(50);
        const QPoint a = cv->pageToView(red->rect.center()).toPoint(), b = cv->pageToView(blue->rect.center()).toPoint();
        QTest::mousePress(cv->viewport(), Qt::LeftButton, Qt::ControlModifier, a);
        for (int k = 1; k <= 4; ++k) {
            const QPoint at = a + (b - a) * k / 4;
            QMouseEvent mv(QEvent::MouseMove, QPointF(at), cv->viewport()->mapToGlobal(QPointF(at)), Qt::NoButton, Qt::LeftButton, Qt::ControlModifier);
            QApplication::sendEvent(cv->viewport(), &mv);
        }
        QTest::mouseRelease(cv->viewport(), Qt::LeftButton, Qt::ControlModifier, b);
        QCOMPARE(red->rect, QRectF(72, 72, 144, 144));   // the original has not moved
        QCOMPARE(red->imageId, redImage);
        QCOMPARE(blue->imageId, blueImage);              // and the other picture is as it was
        QCOMPARE(int(ed->surfaceItems().size()), 3);
        auto *copy = dynamic_cast<jp::PictureItem *>(ed->single());
        QVERIFY(copy && copy != red.get() && copy != blue.get());
        QCOMPARE(copy->imageId, redImage);
        QVERIFY(copy->rect.center().x() > 250 && copy->rect.center().y() > 250);   // where it was let go
        ed->undo();   // one step takes the copy back out
        QCOMPARE(int(ed->surfaceItems().size()), 2);
    }

    // Text and files dropped on the page land at the pointer, as Extra Content
    // does (they landed about 27 points up and to the left of it, by the
    // width of the ruler).
    void textAndFileDropsLandAtThePointer()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        jp::Canvas *cv = w.canvas();
        const QPoint at = cv->pageToView(QPointF(300, 250)).toPoint();
        const QPointF want = cv->toPage(QPointF(at));
        auto dropOn = [&](QMimeData &md) {
            QDragEnterEvent enter(at, Qt::CopyAction, &md, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(cv->viewport(), &enter);
            QVERIFY(enter.isAccepted());
            QDropEvent drop(QPointF(at), Qt::CopyAction, &md, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(cv->viewport(), &drop);
            QVERIFY(drop.isAccepted());
        };
        QMimeData text;
        text.setText(QStringLiteral("dropped words"));
        dropOn(text);
        auto *box = dynamic_cast<jp::TextItem *>(ed->surfaceItems().back().get());
        QVERIFY(box);
        QCOMPARE(box->rect.topLeft(), want);
        QSignalSpy wanted(cv, &jp::Canvas::insertFilesWanted);
        QMimeData files;
        files.setUrls({QUrl::fromLocalFile(QStringLiteral("/nowhere/a picture.png"))});
        dropOn(files);
        QCOMPARE(wanted.count(), 1);
        QCOMPARE(wanted.at(0).at(1).toPointF(), want);
    }

    // Online Pictures searches Openverse and Wikimedia Commons (it was a list
    // of links), shows each picture's author and license, and inserts the one
    // chosen with its title as alt text and a credit under it. Offline: the
    // libraries' answers are canned, in the form their documentation gives.
    void onlinePicturesSearchAndInsert()
    {
        // (Built here rather than written out: moc misreads raw string literals.)
        auto ovResult = [](const char *id, const char *title, const char *creator, const char *lic, const char *ver, const char *url) {
            return QJsonObject{{"id", id}, {"title", title}, {"creator", creator}, {"license", lic}, {"license_version", ver},
                               {"license_url", QStringLiteral("https://creativecommons.org/licenses/%1/%2/").arg(QLatin1String(lic), QLatin1String(ver))},
                               {"foreign_landing_url", QStringLiteral("https://example.org/page/%1").arg(QLatin1String(id))}, {"url", url},
                               {"thumbnail", QStringLiteral("https://api.openverse.org/v1/images/%1/thumb/").arg(QLatin1String(id))}};
        };
        const QByteArray openverse = QJsonDocument(QJsonObject{{"result_count", 2}, {"results", QJsonArray{
            ovResult("a1", "Red barn", "Pat Lee", "by", "2.0", "https://example.org/barn.png"),
            ovResult("a2", "Blue sky", "", "cc0", "1.0", "https://example.org/sky.png")}}}).toJson();
        const QList<jp::online::Picture> ov = jp::online::parseOpenverse(openverse);
        QCOMPARE(ov.size(), 2);
        QCOMPARE(ov[0].license, QStringLiteral("CC BY 2.0"));
        QVERIFY(ov[0].needsCredit);
        QCOMPARE(ov[1].license, QStringLiteral("CC0 1.0"));
        QVERIFY(!ov[1].needsCredit);
        QCOMPARE(jp::online::credit(ov[0]), QStringLiteral("“Red barn” by Pat Lee, CC BY 2.0"));
        auto meta = [](const QJsonObject &fields) {
            QJsonObject o;
            for (auto it = fields.begin(); it != fields.end(); ++it) o[it.key()] = QJsonObject{{"value", it.value()}};
            return o;
        };
        auto page = [](int index, const char *title, const char *url, const QJsonObject &ext) {
            return QJsonObject{{"pageid", index * 10}, {"index", index}, {"title", title},
                               {"imageinfo", QJsonArray{QJsonObject{{"url", url}, {"descriptionurl", QStringLiteral("https://commons.wikimedia.org/wiki/%1").arg(QLatin1String(title))},
                                                                    {"thumburl", QStringLiteral("%1.thumb").arg(QLatin1String(url))}, {"extmetadata", ext}}}}};
        };
        const QByteArray commons = QJsonDocument(QJsonObject{{"query", QJsonObject{{"pages", QJsonObject{
            {"20", page(2, "File:Second.jpg", "https://upload.wikimedia.org/2.jpg", meta({{"LicenseShortName", "Public domain"}, {"AttributionRequired", "false"}}))},
            {"10", page(1, "File:First photo.jpg", "https://upload.wikimedia.org/1.jpg",
                        meta({{"LicenseShortName", "CC BY-SA 4.0"}, {"Artist", "<a href=\"//x\">Sam <b>Rivera</b></a>"}, {"AttributionRequired", "true"}}))}}}}}}).toJson();
        const QList<jp::online::Picture> wc = jp::online::parseCommons(commons);
        QCOMPARE(wc.size(), 2);
        QCOMPARE(wc[0].title, QStringLiteral("First photo"));   // the search's order, the name without "File:"
        QCOMPARE(wc[0].creator, QStringLiteral("Sam Rivera"));
        QVERIFY(wc[0].needsCredit);
        QVERIFY(!wc[1].needsCredit);
        QVERIFY(jp::online::searchUrl(jp::online::Openverse, QStringLiteral("red barn")).toString(QUrl::FullyEncoded).contains(QStringLiteral("q=red%20barn")));
        // The pane, with the libraries answered from here.
        QImage red(30, 20, QImage::Format_RGB32);
        red.fill(Qt::red);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        red.save(&buf, "PNG");
        QStringList asked;
        const auto keep = jp::online::fetch;
        jp::online::fetch = [&](const QUrl &u, QObject *, const jp::online::Done &done, qint64) {
            asked << u.toString();
            done(u.host() == QLatin1String("api.openverse.org") && u.path() == QLatin1String("/v1/images/") ? openverse : png, QString());
        };
        jp::MainWindow w;
        w.showTaskPane(QStringLiteral("online"));
        auto *pane = w.findChild<jp::OnlinePicturesPane *>();
        QVERIFY(pane);
        pane->search(QStringLiteral("barn"));
        QCOMPARE(pane->results()->count(), 2);
        QVERIFY(!pane->results()->item(0)->icon().isNull());   // its small copy came
        QVERIFY(pane->creditBox()->isChecked());
        pane->insertButton()->click();
        jp::online::fetch = keep;
        QVERIFY(asked.contains(QStringLiteral("https://example.org/barn.png")));
        auto *pic = dynamic_cast<jp::PictureItem *>(w.editor()->single());
        QVERIFY(pic);
        QCOMPARE(pic->altText, QStringLiteral("Red barn"));
        auto *cap = dynamic_cast<jp::TextItem *>(w.editor()->doc()->item(pic->caption));
        QVERIFY(cap);
        QCOMPARE(w.editor()->doc()->storyDoc(cap->storyId)->toPlainText(), QStringLiteral("“Red barn” by Pat Lee, CC BY 2.0"));
        QCOMPARE(w.editor()->doc()->imageSize(pic->imageId), QSize(30, 20));
    }

    // Clearing the search box and pressing Enter left the old search's small
    // copies on their way, and they were set on list items that had gone (a
    // crash). A new search, with words or without, now ends the old one's
    // requests and drops any answer that still comes.
    void onlinePicturesNewSearchEndsPendingAnswers()
    {
        QJsonArray found;
        for (int i = 0; i < 3; ++i)
            found.append(QJsonObject{{"title", QStringLiteral("Pic %1").arg(i)}, {"license", "by"}, {"license_version", "2.0"},
                                     {"url", QStringLiteral("https://example.org/%1.png").arg(i)}, {"thumbnail", QStringLiteral("https://example.org/%1.thumb").arg(i)}});
        const QByteArray openverse = QJsonDocument(QJsonObject{{"results", found}}).toJson();
        QImage red(30, 20, QImage::Format_RGB32);
        red.fill(Qt::red);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        red.save(&buf, "PNG");
        std::vector<jp::online::Done> waiting;   // small copies, not yet arrived
        QList<QPointer<QObject>> asking;         // who asked for them
        const auto keep = jp::online::fetch;
        const auto restore = qScopeGuard([&] { jp::online::fetch = keep; });
        jp::online::fetch = [&](const QUrl &u, QObject *context, const jp::online::Done &done, qint64) {
            if (u.host() == QLatin1String("api.openverse.org")) {
                done(openverse, QString());
                return;
            }
            waiting.push_back(done);
            asking << context;
        };
        jp::MainWindow w;
        w.showTaskPane(QStringLiteral("online"));
        auto *pane = w.findChild<jp::OnlinePicturesPane *>();
        QVERIFY(pane);
        pane->search(QStringLiteral("barn"));
        QCOMPARE(pane->results()->count(), 3);
        QCOMPARE(int(waiting.size()), 3);
        pane->search(QString());   // the box cleared, and Enter pressed
        QCOMPARE(pane->results()->count(), 0);
        for (const auto &who : asking) QVERIFY(who.isNull());   // their requests are ended
        for (const auto &done : waiting) done(png, QString());   // and any that still come are dropped
        QCOMPARE(pane->results()->count(), 0);
        // A new search with words does the same for the one before it.
        waiting.clear();
        asking.clear();
        pane->search(QStringLiteral("barn"));
        pane->search(QStringLiteral("sky"));
        QCOMPARE(pane->results()->count(), 3);
        QCOMPARE(int(waiting.size()), 6);
        for (int i = 0; i < 3; ++i) QVERIFY(asking[i].isNull());
        for (int i = 0; i < 3; ++i) waiting[i](png, QString());   // the first search's
        for (int i = 0; i < 3; ++i) QVERIFY(pane->results()->item(i)->icon().isNull());
        for (int i = 3; i < 6; ++i) waiting[i](png, QString());   // the second's
        for (int i = 0; i < 3; ++i) QVERIFY(!pane->results()->item(i)->icon().isNull());
    }

    // A library's answer can name a file on this computer, a program, or data
    // in the address, and the pane would fetch it or show a link to it. Only
    // https addresses are used: results with another kind are left out, and the
    // download itself refuses them.
    void onlinePicturesTrustOnlySecureAddresses()
    {
        const QString pic = QStringLiteral("https://example.org/a.png"), thumb = QStringLiteral("https://example.org/a.thumb");
        const QString license = QStringLiteral("https://creativecommons.org/licenses/by/2.0/"), landing = QStringLiteral("https://example.org/p");
        auto result = [](const char *title, const QString &url, const QString &thumbnail, const QString &licenseUrl, const QString &page) {
            return QJsonObject{{"title", title}, {"license", "by"}, {"license_version", "2.0"}, {"license_url", licenseUrl}, {"foreign_landing_url", page}, {"url", url}, {"thumbnail", thumbnail}};
        };
        const QByteArray openverse = QJsonDocument(QJsonObject{{"results", QJsonArray{
            result("good", pic, thumb, license, landing),
            result("file picture", QStringLiteral("file:///etc/hostname"), thumb, license, landing),
            result("data picture", QStringLiteral("data:text/plain;base64,SEVMTE8="), thumb, license, landing),
            result("plain http picture", QStringLiteral("http://example.org/b.png"), thumb, license, landing),
            result("file thumbnail", pic, QStringLiteral("file:///etc/hostname"), license, landing),
            result("program link", pic, thumb, QStringLiteral("file:///C:/Windows/System32/calc.exe"), landing),
            result("script link", pic, thumb, license, QStringLiteral("javascript:alert(1)")),
            result("no thumbnail", pic, QString(), license, landing)}}}).toJson();
        const QList<jp::online::Picture> list = jp::online::parseOpenverse(openverse);
        QStringList titles;
        for (const auto &p : list) titles << p.title;
        QCOMPARE(titles, (QStringList{QStringLiteral("good"), QStringLiteral("no thumbnail")}));
        QVERIFY(list[1].thumb.isEmpty());
        for (const auto &p : list) {
            QCOMPARE(p.full.scheme(), QStringLiteral("https"));
            QVERIFY(p.licenseUrl.startsWith(QStringLiteral("https://")));
        }
        // Commons: the same for each of its addresses.
        auto meta = [](const QString &licenseUrl) {
            return QJsonObject{{"LicenseShortName", QJsonObject{{"value", "CC BY 2.0"}}}, {"LicenseUrl", QJsonObject{{"value", licenseUrl}}}};
        };
        auto entry = [&](int index, const char *title, const QString &url, const QString &licenseUrl) {
            return QJsonObject{{"index", index}, {"title", title}, {"imageinfo", QJsonArray{QJsonObject{{"url", url}, {"thumburl", thumb}, {"descriptionurl", landing}, {"extmetadata", meta(licenseUrl)}}}}};
        };
        const QByteArray commons = QJsonDocument(QJsonObject{{"query", QJsonObject{{"pages", QJsonObject{
            {"1", entry(1, "File:Good.png", pic, license)}, {"2", entry(2, "File:Local.png", QStringLiteral("file:///etc/hostname"), license)},
            {"3", entry(3, "File:Program.png", pic, QStringLiteral("file:///C:/Windows/System32/calc.exe"))}, {"4", entry(4, "File:Plain.png", QStringLiteral("http://example.org/c.png"), license)}}}}}}).toJson();
        const QList<jp::online::Picture> fromCommons = jp::online::parseCommons(commons);
        QCOMPARE(fromCommons.size(), 1);
        QCOMPARE(fromCommons[0].title, QStringLiteral("Good"));
        // The pane shows no link to them either.
        {
            const auto keep = jp::online::fetch;
            const auto restore = qScopeGuard([&] { jp::online::fetch = keep; });
            jp::online::fetch = [&](const QUrl &, QObject *, const jp::online::Done &done, qint64) { done(openverse, QString()); };
            jp::MainWindow w;
            w.showTaskPane(QStringLiteral("online"));
            auto *pane = w.findChild<jp::OnlinePicturesPane *>();
            QVERIFY(pane);
            pane->search(QStringLiteral("x"));
            QCOMPARE(pane->results()->count(), 2);
            for (auto *l : pane->findChildren<QLabel *>()) QVERIFY(!l->text().contains(QStringLiteral("file:")) && !l->text().contains(QStringLiteral("javascript:")));
        }
        // The download refuses a file, data, and a plain web address without reading them.
        QTemporaryDir dir;
        QFile secret(dir.filePath(QStringLiteral("secret.txt")));
        QVERIFY(secret.open(QIODevice::WriteOnly));
        secret.write("SECRET-LOCAL-FILE");
        secret.close();
        QObject context;
        for (const QUrl &u : {QUrl::fromLocalFile(secret.fileName()), QUrl(QStringLiteral("data:text/plain;base64,SEVMTE8=")), QUrl(QStringLiteral("http://127.0.0.1:9/never"))}) {
            bool answered = false;
            QByteArray body;
            QString error;
            jp::online::fetch(u, &context, [&](const QByteArray &b, const QString &e) { body = b; error = e; answered = true; }, jp::online::kMaxSmallBytes);
            QTRY_VERIFY_WITH_TIMEOUT(answered, 5000);
            QVERIFY2(body.isEmpty() && !error.isEmpty(), qPrintable(u.toString()));
        }
    }

    // A library's answer is held to a size: no more results than were asked
    // for, no more bytes than a picture or a small copy should be (the download
    // stops when they pass it, and when it takes too long overall, not only when
    // it goes quiet), and a Commons picture wider than 1,920 pixels comes as a
    // copy that wide, not the original upload. (The downloads are from a server
    // in this program, on this computer; nothing goes online.)
    void onlinePicturesLimitWhatALibraryCanSend()
    {
        QCOMPARE(jp::online::kMaxResults, 24);
        QVERIFY(jp::online::searchUrl(jp::online::Openverse, QStringLiteral("x")).query().contains(QStringLiteral("page_size=24")));
        QVERIFY(jp::online::searchUrl(jp::online::Commons, QStringLiteral("x")).query().contains(QStringLiteral("gsrlimit=24")));
        QJsonArray many;
        QJsonObject pages;
        for (int i = 0; i < 100; ++i) {
            many.append(QJsonObject{{"title", QStringLiteral("Pic %1").arg(i)}, {"license", "by"}, {"license_version", "2.0"}, {"url", QStringLiteral("https://example.org/%1.png").arg(i)}});
            pages[QString::number(i)] = QJsonObject{{"index", i}, {"title", QStringLiteral("File:Pic %1.png").arg(i)},
                                                    {"imageinfo", QJsonArray{QJsonObject{{"url", QStringLiteral("https://example.org/%1.png").arg(i)}}}}};
        }
        const QList<jp::online::Picture> ov = jp::online::parseOpenverse(QJsonDocument(QJsonObject{{"results", many}}).toJson());
        QCOMPARE(ov.size(), 24);
        QCOMPARE(ov.last().title, QStringLiteral("Pic 23"));
        const QList<jp::online::Picture> wc = jp::online::parseCommons(QJsonDocument(QJsonObject{{"query", QJsonObject{{"pages", pages}}}}).toJson());
        QCOMPARE(wc.size(), 24);
        QCOMPARE(wc.last().title, QStringLiteral("Pic 23"));   // the first 24 in the search's order
        // A big original comes as a sized copy; a small one, or one of unknown width, as itself.
        auto commons = [](const char *file, int width) {
            const QString original = QStringLiteral("https://upload.wikimedia.org/wikipedia/commons/a/ab/%1").arg(QLatin1String(file));
            QJsonObject info{{"url", original}, {"thumburl", QStringLiteral("https://upload.wikimedia.org/wikipedia/commons/thumb/a/ab/%1/240px-%1").arg(QLatin1String(file))}};
            if (width > 0) info["width"] = width;
            return jp::online::parseCommons(QJsonDocument(QJsonObject{{"query", QJsonObject{{"pages", QJsonObject{{"1", QJsonObject{{"index", 1}, {"title", "File:x"}, {"imageinfo", QJsonArray{info}}}}}}}}}).toJson());
        };
        QCOMPARE(commons("Big.jpg", 6000).value(0).full.toString(), QStringLiteral("https://upload.wikimedia.org/wikipedia/commons/thumb/a/ab/Big.jpg/1920px-Big.jpg"));
        QCOMPARE(commons("Big.jpg", 1920).value(0).full.toString(), QStringLiteral("https://upload.wikimedia.org/wikipedia/commons/a/ab/Big.jpg"));
        QCOMPARE(commons("Big.jpg", 0).value(0).full.toString(), QStringLiteral("https://upload.wikimedia.org/wikipedia/commons/a/ab/Big.jpg"));
        QVERIFY(jp::online::searchUrl(jp::online::Commons, QStringLiteral("x")).query().contains(QStringLiteral("size")));   // the width is asked for
        // The download.
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        enum Reply { Small, Declared, Endless, Stalled, Elsewhere } reply = Small;
        connect(&server, &QTcpServer::newConnection, &server, [&] {
            QTcpSocket *s = server.nextPendingConnection();
            connect(s, &QTcpSocket::disconnected, s, &QObject::deleteLater);
            connect(s, &QTcpSocket::readyRead, s, [&reply, s] {
                s->readAll();
                switch (reply) {
                case Small: s->write("HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello"); break;
                case Declared: s->write("HTTP/1.1 200 OK\r\nContent-Length: 3000000\r\nConnection: close\r\n\r\n" + QByteArray(10000, 'x')); break;
                case Stalled: s->write("HTTP/1.1 200 OK\r\nContent-Length: 1000\r\nConnection: close\r\n\r\nhello"); break;
                case Elsewhere: s->write("HTTP/1.1 302 Found\r\nLocation: file:///etc/hostname\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"); break;
                case Endless: {
                    s->write("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n");
                    auto pump = [s] {
                        while (s->state() == QAbstractSocket::ConnectedState && s->bytesToWrite() < 200000) s->write(QByteArray(65536, 'x'));
                    };
                    connect(s, &QTcpSocket::bytesWritten, s, pump);
                    pump();
                    break;
                }
                }
            });
        });
        QNetworkAccessManager nam;
        auto get = [&](Reply r, qint64 most, int totalMs, QByteArray *body, QString *error) {
            reply = r;
            bool answered = false;
            QObject context;
            jp::online::download(&nam, QUrl(QStringLiteral("http://127.0.0.1:%1/p").arg(server.serverPort())), &context, [&](const QByteArray &b, const QString &e) { *body = b; *error = e; answered = true; }, most, totalMs);
            QTRY_VERIFY_WITH_TIMEOUT(answered, 10000);
        };
        QByteArray body;
        QString error;
        get(Small, 100000, 5000, &body, &error);
        QCOMPARE(body, QByteArray("hello"));
        QVERIFY(error.isEmpty());
        get(Declared, 100000, 5000, &body, &error);   // says it is 3 MB: stopped as soon as it says so
        QVERIFY(body.isEmpty());
        QVERIFY2(error.contains(QStringLiteral("too large")), qPrintable(error));
        get(Endless, 100000, 5000, &body, &error);    // never says, never ends: stopped when it passes the limit
        QVERIFY(body.isEmpty());
        QVERIFY2(error.contains(QStringLiteral("too large")), qPrintable(error));
        get(Stalled, 100000, 300, &body, &error);     // sends a little and waits: stopped by the overall time
        QVERIFY(body.isEmpty());
        QVERIFY2(error.contains(QStringLiteral("too long")), qPrintable(error));
        get(Elsewhere, 100000, 5000, &body, &error);  // sends the download to a file on this computer: not followed
        QVERIFY(body.isEmpty());
        QVERIFY(!error.isEmpty());
    }

    // "C++" searched for "C" (a plus sign was sent as it is, and a server reads
    // it as a space); and a literal "%20" was read as a space.
    void onlinePicturesSearchWordsAreEncoded()
    {
        // What a server reads back from the address: "+" is a space, and %XX a byte.
        auto serverReads = [](const QUrl &u, const QString &key) {
            for (const QString &pair : QString::fromLatin1(u.toEncoded()).section(QLatin1Char('?'), 1).split(QLatin1Char('&')))
                if (pair.startsWith(key + QLatin1Char('='))) return QUrl::fromPercentEncoding(pair.mid(key.size() + 1).replace(QLatin1Char('+'), QLatin1Char(' ')).toLatin1());
            return QString();
        };
        for (const QString &words : {QStringLiteral("C++"), QStringLiteral("a&b=c#d+e%20f"), QStringLiteral("100% fun"), QStringLiteral("caf\u00e9 \"red barn\""), QStringLiteral("red barn")}) {
            QCOMPARE(serverReads(jp::online::searchUrl(jp::online::Openverse, words), QStringLiteral("q")), words);
            QCOMPARE(serverReads(jp::online::searchUrl(jp::online::Commons, words), QStringLiteral("gsrsearch")), words + QStringLiteral(" filetype:bitmap"));
        }
        QVERIFY(jp::online::searchUrl(jp::online::Openverse, QStringLiteral("red barn")).toString(QUrl::FullyEncoded).contains(QStringLiteral("q=red%20barn")));
    }

    // A picture that cannot be loaded (too large, too slow, cut short, or not a
    // picture) says so in the pane; it stayed at "Downloading...", and the
    // chosen picture of the person's own was not touched.
    void onlinePicturesSayWhenAPictureFails()
    {
        const QByteArray openverse = QJsonDocument(QJsonObject{{"results", QJsonArray{QJsonObject{{"title", "Red barn"}, {"creator", "Pat"}, {"license", "by"}, {"license_version", "2.0"},
                                                                                                   {"url", "https://example.org/barn.bmp"}, {"thumbnail", "https://example.org/barn.thumb"}}}}}).toJson();
        QByteArray cut("\x89PNG\r\n\x1a\n", 8);   // sniffs as a PNG, but is cut short
        QString pictureError;
        const auto keep = jp::online::fetch;
        const auto restore = qScopeGuard([&] { jp::online::fetch = keep; });
        jp::online::fetch = [&](const QUrl &u, QObject *, const jp::online::Done &done, qint64) {
            if (u.host() == QLatin1String("api.openverse.org")) done(openverse, QString());
            else if (u.path().endsWith(QLatin1String(".bmp"))) done(pictureError.isEmpty() ? cut : QByteArray(), pictureError);
            else done(QByteArray(), QStringLiteral("no thumbnail"));
        };
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        QImage green(40, 40, QImage::Format_RGB32);
        green.fill(Qt::green);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        green.save(&buf, "PNG");
        auto mine = std::make_shared<jp::PictureItem>();
        mine->imageId = ed->doc()->addImage(png, "png");
        mine->rect = QRectF(72, 72, 100, 100);
        mine->imgRect = QRectF(0, 0, 100, 100);
        mine->altText = QStringLiteral("My family photo");
        ed->addItem(mine);
        w.showTaskPane(QStringLiteral("online"));
        auto *pane = w.findChild<jp::OnlinePicturesPane *>();
        QVERIFY(pane);
        auto shown = [&] {
            QStringList s;
            for (auto *l : pane->findChildren<QLabel *>()) s << l->text();
            return s.join(QLatin1Char('\n'));
        };
        pane->search(QStringLiteral("barn"));
        QCOMPARE(pane->results()->count(), 1);
        const size_t before = ed->surfaceItems().size();
        pane->insertButton()->click();   // cut short
        QVERIFY2(shown().contains(QStringLiteral("couldn't be loaded")), qPrintable(shown()));
        QVERIFY(!shown().contains(QStringLiteral("Downloading")));
        QCOMPARE(ed->surfaceItems().size(), before);
        QCOMPARE(mine->altText, QStringLiteral("My family photo"));
        QVERIFY(mine->caption.isEmpty());
        QVERIFY(pane->insertButton()->isEnabled());
        pictureError = QStringLiteral("It is too large.");
        pane->insertButton()->click();
        QVERIFY2(shown().contains(QStringLiteral("couldn't be downloaded: It is too large.")), qPrintable(shown()));
        QVERIFY(!shown().contains(QStringLiteral("Downloading")));
    }

    // Save as Building Block's blocks show in Insert > Page Parts, under My
    // Building Blocks, and insert from there (they were saved where no
    // gallery looked).
    void savedBuildingBlocksInsert()
    {
        QStandardPaths::setTestModeEnabled(true);   // a test folder, not the real one
        QDir(jp::userBlocksDir()).removeRecursively();
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 200, 60), QStringLiteral("Block text")));
        ed->addItem(box);
        QTimer::singleShot(0, [] {
            if (auto *d = qobject_cast<QInputDialog *>(QApplication::activeModalWidget())) {
                d->setTextValue(QStringLiteral("Pull quote"));
                d->accept();
            }
        });
        w.act(QStringLiteral("obj.saveBlock"))->trigger();
        QVERIFY(QFile::exists(QDir(jp::userBlocksDir()).filePath(QStringLiteral("Pull quote.json"))));
        jp::GalleryButton *pageParts = nullptr;
        for (auto *g : w.findChildren<jp::GalleryButton *>())
            if (g->text() == QLatin1String("Page Parts")) pageParts = g;
        QVERIFY(pageParts);
        QString id;
        for (const auto &it : pageParts->items())
            if (it.tip == QLatin1String("Pull quote") && it.group == QLatin1String("My Building Blocks")) id = it.id;
        QVERIFY(!id.isEmpty());
        const int before = int(ed->surfaceItems().size());
        Q_EMIT pageParts->activated(id);
        QCOMPARE(int(ed->surfaceItems().size()), before + 1);
        auto *made = dynamic_cast<jp::TextItem *>(ed->single());
        QVERIFY(made && made->id != box->id);
        QCOMPARE(ed->doc()->storyDoc(made->storyId)->toPlainText(), QStringLiteral("Block text"));
        QDir(jp::userBlocksDir()).removeRecursively();
        QStandardPaths::setTestModeEnabled(false);
    }

    // A picture field shows each record's picture in the preview, printing,
    // PDF, and email (only merging to a new publication did).
    void pictureFieldsShowEachRecord()
    {
        QTemporaryDir dir;
        auto save = [&](const QString &name, const QColor &c) {
            QImage img(20, 20, QImage::Format_RGB32);
            img.fill(c);
            img.save(dir.filePath(name));
        };
        save(QStringLiteral("a.png"), Qt::red);
        save(QStringLiteral("b.png"), Qt::blue);
        auto doc = jp::Document::blank(QSizeF(200, 200));
        doc->merge.path = dir.filePath(QStringLiteral("list.csv"));
        doc->merge.fields = {QStringLiteral("Name"), QStringLiteral("Photo")};
        doc->merge.rows = {{QStringLiteral("Ann"), QStringLiteral("a.png")}, {QStringLiteral("Bo"), QStringLiteral("b.png")}};
        doc->merge.include = {true, true};
        auto pic = std::make_shared<jp::PictureItem>();
        pic->name = QStringLiteral("merge:Photo");
        pic->rect = QRectF(50, 50, 100, 100);
        doc->pages[0]->items.push_back(pic);
        auto centre = [&](int record) {
            jp::LayoutCache cache;
            jp::PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            ctx.opt.output = true;
            ctx.opt.mergeRecord = record;
            QImage img(200, 200, QImage::Format_RGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            jp::Renderer::paintPage(&p, ctx, 0);
            p.end();
            return img.pixelColor(100, 100);
        };
        QCOMPARE(centre(0), QColor(Qt::red));
        QCOMPARE(centre(1), QColor(Qt::blue));
        QCOMPARE(centre(-1), QColor(Qt::white));   // field codes shown: no record's picture
    }

    // A picture saved as a shape filled with it (see-through, or cut to a
    // shape) is drawn by Publisher 2.5 points past the shape on every side,
    // as it draws any picture fill (measured Oct 11 on rectangles of 200 to
    // 600 points: 2.5 points each side, from a picture with a red edge 5%
    // wide). The picture saved is the part showing and 2.5 points of it
    // around, so what shows in the frame stays as JeffPub draws it; the edge
    // pixels go on over the part of the margin the picture doesn't reach.
    void seeThroughPictureIsSavedWithTheFillBleed()
    {
        using namespace jp;
        QImage src(200, 150, QImage::Format_RGB32);   // 2 pixels to the point in a frame 100 x 75
        src.fill(Qt::white);
        {
            QPainter p(&src);
            p.fillRect(QRect(0, 0, 200, 150), Qt::red);
            p.fillRect(QRect(20, 20, 160, 110), Qt::white);
        }
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        src.save(&buf, "PNG");
        auto saved = [&](const QRectF &imgRect, QImage *out, QRectF *placed = nullptr) {
            auto doc = Document::blank(QSizeF(300, 200));
            auto pic = std::make_shared<PictureItem>();
            pic->imageId = doc->addImage(png, QStringLiteral("png"));
            pic->rect = QRectF(50, 50, 100, 75);
            pic->imgRect = imgRect;
            pic->transparency = 0.3;
            doc->pages[0]->items.push_back(pic);
            QTemporaryDir dir;
            const QString path = dir.filePath(QStringLiteral("see-through.pub"));
            QString err;
            if (!exportPublisher(*doc, path, &err)) return false;
            auto back = importPublisherFile(path, &err);
            if (!back) return false;
            for (const auto &it : back->pages[0]->items)
                if (it->type() == ItemType::Picture) {
                    *out = back->image(static_cast<const PictureItem &>(*it).imageId);
                    if (placed) *placed = static_cast<const PictureItem &>(*it).imgRect;
                }
            return !out->isNull();
        };
        // The whole picture in its frame: 2.5 points more on each side, 5 pixels.
        QImage whole, bigger;
        QRectF placed;
        QVERIFY(saved(QRectF(0, 0, 100, 75), &whole, &placed));
        QVERIFY2(std::abs(whole.width() - 210) <= 2 && std::abs(whole.height() - 160) <= 2, qPrintable(QStringLiteral("%1 x %2").arg(whole.width()).arg(whole.height())));
        // The red edge, 20 pixels wide, is 25 with the margin, which goes on in the edge's color.
        QVERIFY(QColor(whole.pixel(2, 80)) == QColor(Qt::red) && QColor(whole.pixel(22, 80)) == QColor(Qt::red) && QColor(whole.pixel(30, 80)) == QColor(Qt::white));
        QVERIFY(QColor(whole.pixel(100, 2)) == QColor(Qt::red) && QColor(whole.pixel(100, 80)) == QColor(Qt::white));
        // Read back, the picture is drawn as Publisher draws it: past its frame by those 2.5 points (so it shows what it did).
        QVERIFY2(std::abs(placed.left() + 2.5) < 0.01 && std::abs(placed.top() + 2.5) < 0.01 && std::abs(placed.width() - 105) < 0.01 && std::abs(placed.height() - 80) < 0.01,
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(placed.left()).arg(placed.top()).arg(placed.width()).arg(placed.height())));
        // A picture reaching beyond its frame (as a background does): only 2.5 points of it past the frame are kept, as it is.
        QVERIFY(saved(QRectF(-10, -10, 120, 95), &bigger));   // 200 pixels over 120 points
        const double perPt = 200.0 / 120, perPtY = 150.0 / 95;
        QVERIFY2(std::abs(bigger.width() - 105 * perPt) <= 2 && std::abs(bigger.height() - 80 * perPtY) <= 2, qPrintable(QStringLiteral("%1 x %2").arg(bigger.width()).arg(bigger.height())));
    }

    // A page's background picture is drawn past the page's edge, 2.5 points
    // on every side, and the picture's crops (fractions of the picture) then
    // trim that larger picture. Publisher (Oct 11), a background made from a
    // picture 2000 x 1500 with a red edge 5% wide: on a page 600 x 450 the
    // edge showed 17 pixels, not 20, at 96 dpi, at each side, at 300, 600,
    // and 1200 points wide alike; the corpus's crop-fitted covers (crops of
    // 29%, 7%, and 0.4% on the left and right) fit to 0.5 point. A picture
    // in a frame of its own is drawn to the frame and its crops.
    void backgroundPictureReachesPastThePage()
    {
        using namespace jp;
        const QSizeF page(981.144, 681.12);
        // No crops: the page and 2.5 points all round.
        QCOMPARE(pubPictureRect(page, 0, 0, 0, 0, true), QRectF(-2.5, -2.5, page.width() + 5, page.height() + 5));
        QCOMPARE(pubPictureRect(page, 0, 0, 0, 0, false), QRectF(QPointF(0, 0), page));
        // Crops of 29.4% each side, left and right: the larger page is what they leave.
        const double l = 19270.0 / 65536;
        const QRectF r = pubPictureRect(page, 0, 0, l, l, true);
        const double w = (page.width() + 5) / (1 - 2 * l);
        QVERIFY2(std::abs(r.width() - w) < 0.001, qPrintable(QString::number(r.width())));
        QVERIFY2(std::abs(r.left() - (-2.5 - l * w)) < 0.001, qPrintable(QString::number(r.left())));
        QVERIFY2(std::abs(r.top() + 2.5) < 0.001 && std::abs(r.height() - (page.height() + 5)) < 0.001, qPrintable(QString::number(r.height())));
        // In a frame, only the crops.
        const QRectF f = pubPictureRect(page, 0, 0, l, l, false);
        QVERIFY2(std::abs(f.width() - page.width() / (1 - 2 * l)) < 0.001 && std::abs(f.left() + l * f.width()) < 0.001, qPrintable(QString::number(f.width())));
    }

    // A picture much bigger than its frame is drawn as the average of the
    // pixels each screen pixel covers, as Publisher draws it: sampling it
    // dropped thin strokes and left a scanned table's digits broken. Noise
    // averages out to nearly flat gray (a sixth of its spread at 6 pixels
    // to one); a few samples of it stay rough.
    void bigPicturesAreAveragedWhenShrunk()
    {
        using namespace jp;
        QImage noise(1000, 1000, QImage::Format_RGB32);
        QRandomGenerator rng(11);
        for (int y = 0; y < noise.height(); ++y)
            for (int x = 0; x < noise.width(); ++x) {
                const int g = int(rng.bounded(256));
                noise.setPixel(x, y, qRgb(g, g, g));
            }
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        noise.save(&buf, "PNG");
        for (double shown : {100.0, 150.0, 200.0, 340.0}) {
            auto doc = Document::blank(QSizeF(612, 792));
            auto pic = std::make_shared<PictureItem>();
            pic->imageId = doc->addImage(png, QStringLiteral("png"));
            pic->rect = QRectF(50, 50, shown, shown);
            pic->imgRect = QRectF(0, 0, shown, shown);
            doc->pages[0]->items.push_back(pic);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            const QImage out = Renderer::renderToImage(ctx, 0, 1.0);
            double sum = 0, sumSq = 0;
            int n = 0;
            for (int y = 55; y < 45 + shown; ++y)
                for (int x = 55; x < 45 + shown; ++x) {
                    const double g = qGray(out.pixel(x, y));
                    sum += g;
                    sumSq += g * g;
                    ++n;
                }
            const double mean = sum / n, spread = std::sqrt(std::max(0.0, sumSq / n - mean * mean));
            // 1000 / shown source pixels to one: 10, 6.7, 5, and 2.9.
            const double expected = 74 * shown / 1000;
            QVERIFY2(spread < expected * 1.6, qPrintable(QStringLiteral("shown %1: spread %2, averaged would be about %3").arg(shown).arg(spread).arg(expected)));
        }
    }
    // The Open page's thumbnails read only the thumbnail out of each file.
    void thumbnailWithoutLoadingFile()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QImage thumb(40, 52, QImage::Format_RGB32);
        thumb.fill(QColor(10, 120, 200));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("t.jpub"));
        QString err;
        QVERIFY2(jp::savePublication(*doc, path, thumb, &err), qPrintable(err));
        const QImage got = jp::publicationThumbnail(path);
        QCOMPARE(got.size(), thumb.size());
        QCOMPARE(got.pixelColor(5, 5), QColor(10, 120, 200));
        QFile junk(dir.filePath(QStringLiteral("junk.jpub")));
        QVERIFY(junk.open(QIODevice::WriteOnly));
        junk.write(QByteArray(100, 'x'));
        junk.close();
        QVERIFY(jp::publicationThumbnail(junk.fileName()).isNull());
    }

    // Mail-merge sources from elsewhere: a vCard with fields outside any card
    // (once written into an empty list), and a spreadsheet cell at column
    // "ZZZZZZZZZZ" (once padded with billions of empty cells).
    void mergeSourcesHostile()
    {
        QTemporaryDir dir;
        {
            QFile f(dir.filePath(QStringLiteral("contacts.vcf")));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("N:Stray;Field\nTEL:555\nBEGIN:VCARD\nN:Doe;Jane\nEND:VCARD\nEMAIL:after@example.com\n");
        }
        jp::MergeSource m;
        QString err;
        QVERIFY2(jp::loadMergeSource(dir.filePath(QStringLiteral("contacts.vcf")), &m, &err), qPrintable(err));
        QCOMPARE(m.rows.size(), 1);
        QCOMPARE(m.rows[0].value(1), QStringLiteral("Doe"));

        jp::ZipWriter z;
        z.add(QStringLiteral("xl/worksheets/sheet1.xml"),
              "<worksheet><sheetData><row><c r=\"A1\" t=\"inlineStr\"><is><t>Name</t></is></c></row>"
              "<row><c r=\"ZZZZZZZZZZ2\" t=\"inlineStr\"><is><t>far</t></is></c><c r=\"A2\" t=\"inlineStr\"><is><t>Jane</t></is></c></row></sheetData></worksheet>");
        {
            QFile f(dir.filePath(QStringLiteral("list.xlsx")));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(z.finish());
        }
        jp::MergeSource x;
        QVERIFY2(jp::loadMergeSource(dir.filePath(QStringLiteral("list.xlsx")), &x, &err), qPrintable(err));
        for (const QStringList &r : x.rows) QVERIFY(r.size() <= 16384);
    }

    // Online Pictures tells a library which program asks, as Wikimedia asks programs to: the running app's name and version, and
    // where it lives (JeffPub's is as it always was).
    void onlinePicturesNameTheProgramThatAsks()
    {
        struct Restore {
            ~Restore() { jp::setAppInfo(jp::pubAppInfo()); }
        } restore;
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        QByteArray request;
        connect(&server, &QTcpServer::newConnection, &server, [&] {
            QTcpSocket *s = server.nextPendingConnection();
            connect(s, &QTcpSocket::disconnected, s, &QObject::deleteLater);
            connect(s, &QTcpSocket::readyRead, s, [&request, s] {
                request += s->readAll();
                if (request.contains("\r\n\r\n")) s->write("HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nok");
            });
        });
        QNetworkAccessManager nam;
        auto agent = [&] {
            request.clear();
            bool answered = false;
            QObject context;
            jp::online::download(&nam, QUrl(QStringLiteral("http://127.0.0.1:%1/p").arg(server.serverPort())), &context, [&](const QByteArray &, const QString &) { answered = true; }, 1000, 5000);
            [&] { QTRY_VERIFY_WITH_TIMEOUT(answered, 10000); }();
            for (const QByteArray &line : request.split('\n'))
                if (line.toLower().startsWith("user-agent:")) return QString::fromLatin1(line.mid(11)).trimmed();
            return QString();
        };
        QCOMPARE(agent(), QStringLiteral("JeffPub/" JP_VERSION " (https://github.com/JeffOffice/jeffoffice)"));
        jp::setAppInfo({QStringLiteral("JeffDeck"), QStringLiteral("jeffdeck"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffoffice")});
        QCOMPARE(agent(), QStringLiteral("JeffDeck/" JP_VERSION " (https://github.com/JeffOffice/jeffoffice)"));
    }

    // An icon as an SVG picture (what JeffDeck inserts): the bundled icon's own file, drawn in the color asked for instead of the
    // color of the text around it, and safe to keep in a presentation. A name that is no icon gives nothing.
    void iconSvgIsTheIconInOneColor()
    {
        const QByteArray svg = jp::iconSvg(QStringLiteral("heart"), QColor(0x22, 0x66, 0xCC));
        QVERIFY(!svg.isEmpty());
        QVERIFY(!svg.contains("currentColor"));
        QVERIFY(svg.contains("#2266cc"));
        QVERIFY(jp::svgIsSafe(svg));
        QSvgRenderer renderer(svg);
        QVERIFY(renderer.isValid());
        QCOMPARE(renderer.defaultSize(), QSize(24, 24));
        QImage shown(48, 48, QImage::Format_ARGB32);
        shown.fill(Qt::transparent);
        QPainter painter(&shown);
        renderer.render(&painter);
        painter.end();
        QSet<QRgb> colors;
        for (int y = 0; y < shown.height(); ++y)
            for (int x = 0; x < shown.width(); ++x)
                if (qAlpha(shown.pixel(x, y)) == 255) colors.insert(shown.pixel(x, y));
        QVERIFY(colors.contains(qRgb(0x22, 0x66, 0xCC)));
        QVERIFY(jp::iconSvg(QStringLiteral("no-such-icon"), Qt::black).isEmpty());
    }

    // Inserting several pictures at once puts them in the picture tray: the
    // scratch area beside the page, as thumbnails in a column.
    void pictureTray()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QTemporaryDir dir;
        QStringList paths;
        for (int i = 0; i < 3; ++i) {
            QImage img(600, 400 + i * 100, QImage::Format_RGB32);
            img.fill(QColor::fromHsv(i * 100, 200, 220));
            const QString f = dir.filePath(QStringLiteral("p%1.png").arg(i));
            QVERIFY(img.save(f));
            paths << f;
        }
        w.insertFiles(paths, QPointF(-1, -1));
        const jp::Document *d = w.editor()->doc();
        QCOMPARE(int(d->pages[0]->items.size()), 0);
        QCOMPARE(int(d->scratch.size()), 3);
        double y = -1;
        for (const auto &it : d->scratch) {
            QVERIFY(it->rect.left() >= 612 + 30);
            QVERIFY(std::max(it->rect.width(), it->rect.height()) <= 108.01);
            QVERIFY(it->rect.top() > y);
            y = it->rect.top();
        }
        // One picture still goes on the page.
        w.insertFiles({paths.first()}, QPointF(-1, -1));
        QCOMPARE(int(d->pages[0]->items.size()), 1);
    }

    // ---- Pictures linked to their files ----

    // A picture file for the linked-picture tests: its left half is `color`
    // and its right half the opposite color; `noisy` makes it large to store.
    static QString linkPicture(const QString &path, const QSize &size, const QColor &color, bool noisy = false)
    {
        QImage img(size, QImage::Format_RGB32);
        img.fill(color);
        QPainter p(&img);
        p.fillRect(QRect(size.width() / 2, 0, size.width() - size.width() / 2, size.height()), QColor(255 - color.red(), 255 - color.green(), 255 - color.blue()));
        p.end();
        if (noisy) {
            QRandomGenerator rng(7);
            for (int y = 0; y < size.height(); ++y)
                for (int x = 0; x < size.width(); ++x) img.setPixel(x, y, rng.generate() | 0xff000000);
        }
        img.save(path);
        return path;
    }

    // The page as it prints, picture and all.
    static QImage linkRender(jp::Document *doc)
    {
        jp::LayoutCache cache;
        jp::PaintContext ctx;
        ctx.doc = doc;
        ctx.cache = &cache;
        ctx.opt.output = true;
        return jp::Renderer::renderToImage(ctx, 0, 1.0);
    }

    static bool linkColorsClose(const QColor &a, const QColor &b)
    {
        return std::abs(a.red() - b.red()) < 14 && std::abs(a.green() - b.green()) < 14 && std::abs(a.blue() - b.blue()) < 14;
    }

    static jp::PictureItem *linkFirstPicture(jp::Document *doc)
    {
        for (const auto &it : doc->pages[0]->items)
            if (auto *p = dynamic_cast<jp::PictureItem *>(it.get())) return p;
        return nullptr;
    }

    static QByteArray linkFileBytes(const QString &path)
    {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    }

    // Link to File keeps the file's path and a preview of at most 512 pixels,
    // not the picture; the picture draws from the file, whole, when the
    // publication opens again.
    void linkedPictureDrawsFromFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(1000, 800), QColor(200, 30, 30), true);
        const QByteArray fileBytes = linkFileBytes(file);
        QVERIFY(!fileBytes.isEmpty());
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::Link);
        jp::Document *doc = w.editor()->doc();
        jp::PictureItem *pic = linkFirstPicture(doc);
        QVERIFY(pic);
        const QString id = pic->imageId;
        const jp::ImageData &img = doc->images[id];
        QVERIFY(img.linked);
        QVERIFY(!img.keepsCopy);
        QCOMPARE(doc->linkStatus(id), jp::LinkStatus::Linked);
        QCOMPARE(img.sourcePath, QFileInfo(file).absoluteFilePath());
        const QImage preview = QImage::fromData(img.preview);
        QVERIFY(!preview.isNull());
        QCOMPARE(std::max(preview.width(), preview.height()), 512);

        const QString path = dir.filePath(QStringLiteral("book.jpub"));
        QVERIFY(w.saveTo(path));
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(linkFileBytes(path), entries));
        QStringList stored;
        for (auto it = entries.cbegin(); it != entries.cend(); ++it) stored << it.key();
        QVERIFY2(std::none_of(stored.cbegin(), stored.cend(), [](const QString &k) { return k.startsWith(QLatin1String("images/")); }), qPrintable(stored.join(' ')));
        QVERIFY2(std::any_of(stored.cbegin(), stored.cend(), [&](const QString &k) { return k.startsWith(QLatin1String("previews/") + id + QLatin1Char('.')); }), qPrintable(stored.join(' ')));
        QVERIFY(QFileInfo(path).size() < QFileInfo(file).size() / 4);   // the picture itself isn't in it

        QString err;
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->linkStatus(id), jp::LinkStatus::Linked);
        QCOMPARE(back->images[id].sourcePath, QFileInfo(file).absoluteFilePath());
        QCOMPARE(back->images[id].bytes, fileBytes);   // read from the file, not the preview
        QCOMPARE(back->image(id).size(), QSize(1000, 800));
        QCOMPARE(back->imageSize(id), QSize(1000, 800));

        // Drawn from the file: left half red, right half the opposite.
        auto flat = jp::Document::blank(QSizeF(612, 792));
        const QString flatFile = linkPicture(dir.filePath(QStringLiteral("flat.png")), QSize(400, 200), QColor(200, 30, 30));
        auto item = std::make_shared<jp::PictureItem>();
        item->rect = QRectF(100, 100, 400, 200);
        item->imgRect = QRectF(0, 0, 400, 200);
        item->imageId = flat->addLinkedImage(linkFileBytes(flatFile), QStringLiteral("png"), flatFile, false);
        flat->pages[0]->items.push_back(item);
        const QString flatPath = dir.filePath(QStringLiteral("flat.jpub"));
        QVERIFY(jp::savePublication(*flat, flatPath, QImage(), &err));
        auto flatBack = jp::loadPublication(flatPath, &err);
        QVERIFY2(flatBack, qPrintable(err));
        const QImage page = linkRender(flatBack.get());
        QVERIFY(linkColorsClose(QColor(page.pixel(200, 200)), QColor(200, 30, 30)));
        QVERIFY(linkColorsClose(QColor(page.pixel(400, 200)), QColor(55, 225, 225)));
    }

    // Insert and Link keeps the full picture and the link: the picture stays
    // when the file goes, and a changed file replaces the stored copy when
    // the publication opens.
    void insertAndLinkRefreshesStoredCopy()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(800, 600), QColor(20, 120, 200));
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::EmbedAndLink);
        const QString id = linkFirstPicture(w.editor()->doc())->imageId;
        QVERIFY(w.editor()->doc()->images[id].linked);
        QVERIFY(w.editor()->doc()->images[id].keepsCopy);
        const QString path = dir.filePath(QStringLiteral("book.jpub"));
        QVERIFY(w.saveTo(path));
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(linkFileBytes(path), entries));
        QStringList keys = entries.keys();
        QVERIFY2(keys.filter(QRegularExpression(QStringLiteral("^images/"))).size() == 1, qPrintable(keys.join(' ')));
        QVERIFY2(keys.filter(QRegularExpression(QStringLiteral("^previews/"))).isEmpty(), qPrintable(keys.join(' ')));

        // The file changes: the stored copy follows when the publication opens.
        const QString oldId = id;
        linkPicture(file, QSize(500, 500), QColor(20, 200, 60));
        QString err;
        auto changed = jp::loadPublication(path, &err);
        QVERIFY2(changed, qPrintable(err));
        QCOMPARE(changed->linkStatus(oldId), jp::LinkStatus::Linked);   // refreshed, so no longer Modified
        QCOMPARE(changed->images[oldId].bytes, linkFileBytes(file));
        QCOMPARE(changed->image(oldId).size(), QSize(500, 500));
        const jp::PictureItem *pic = linkFirstPicture(changed.get());
        QVERIFY(std::abs(pic->imgRect.width() - pic->imgRect.height()) < 0.5);   // not stretched to the old shape

        // The file goes: the stored picture is all there is, and all that's needed.
        QVERIFY(w.saveTo(path));
        QVERIFY(QFile::remove(file));
        auto gone = jp::loadPublication(path, &err);
        QVERIFY2(gone, qPrintable(err));
        QCOMPARE(gone->linkStatus(oldId), jp::LinkStatus::Missing);
        QCOMPARE(gone->image(oldId).size(), QSize(800, 600));
        jp::MainWindow w2;
        w2.show();
        w2.editor()->setDocument(std::move(gone), path);
        w2.showTaskPane(QStringLiteral("designchecker"));
        w2.refreshUi();
        QStringList found;
        for (auto *l : w2.findChildren<QListWidget *>())
            for (int i = 0; i < l->count(); ++i) found << l->item(i)->text();
        QVERIFY2(!found.filter(QStringLiteral("(Page 1)")).isEmpty(), qPrintable(found.join('|')));   // the checker did list things
        QVERIFY2(!found.join('|').contains(QStringLiteral("Linked picture is missing")), qPrintable(found.join('|')));
        QVERIFY2(!found.join('|').contains(QStringLiteral("linked, not embedded")), qPrintable(found.join('|')));
    }

    // The path relative to the publication wins when the publication was
    // moved together with its pictures, even where the old path still exists.
    void linkedPictureMovedWithPublication()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir root(dir.path());
        QVERIFY(root.mkpath(QStringLiteral("a/pictures")));
        const QString file = linkPicture(root.filePath(QStringLiteral("a/pictures/p.png")), QSize(400, 300), QColor(200, 30, 30));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto item = std::make_shared<jp::PictureItem>();
        item->rect = QRectF(100, 100, 400, 300);
        item->imgRect = QRectF(0, 0, 400, 300);
        item->imageId = doc->addLinkedImage(linkFileBytes(file), QStringLiteral("png"), file, false);
        const QString id = item->imageId;
        doc->pages[0]->items.push_back(item);
        QString err;
        QVERIFY(jp::savePublication(*doc, root.filePath(QStringLiteral("a/book.jpub")), QImage(), &err));

        // Moved together: the copy in b has other contents, and is the one found.
        QVERIFY(root.mkpath(QStringLiteral("b/pictures")));
        QVERIFY(QFile::copy(root.filePath(QStringLiteral("a/book.jpub")), root.filePath(QStringLiteral("b/book.jpub"))));
        const QString moved = linkPicture(root.filePath(QStringLiteral("b/pictures/p.png")), QSize(400, 300), QColor(30, 30, 200));
        auto back = jp::loadPublication(root.filePath(QStringLiteral("b/book.jpub")), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->images[id].sourcePath, QFileInfo(moved).absoluteFilePath());
        QCOMPARE(back->images[id].bytes, linkFileBytes(moved));
        QCOMPARE(back->linkStatus(id), jp::LinkStatus::Modified);   // not the file it was made from

        // Not moved: the full path is still the link, but the file is outside the folder
        // the publication is in now, so it is not looked at (see linksOutsideTheFolderAreNotFollowed).
        QVERIFY(root.mkpath(QStringLiteral("c")));
        QVERIFY(QFile::copy(root.filePath(QStringLiteral("a/book.jpub")), root.filePath(QStringLiteral("c/book.jpub"))));
        auto stayed = jp::loadPublication(root.filePath(QStringLiteral("c/book.jpub")), &err);
        QVERIFY2(stayed, qPrintable(err));
        QCOMPARE(stayed->images[id].sourcePath, QFileInfo(file).absoluteFilePath());
        QCOMPARE(stayed->linkStatus(id), jp::LinkStatus::NotUpdated);

        // Saved somewhere else, the link is kept relative to the new place.
        QVERIFY(jp::savePublication(*stayed, root.filePath(QStringLiteral("c/again.jpub")), QImage(), &err));
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(linkFileBytes(root.filePath(QStringLiteral("c/again.jpub"))), entries));
        const QJsonObject json = QJsonDocument::fromJson(entries["document.json"]).object();
        QCOMPARE(json["images"].toArray()[0].toObject()["relative"].toString(), QStringLiteral("../a/pictures/p.png"));
    }

    // A file that has gone leaves the preview on the page; the picture is
    // Missing, its size is still the file's, and the Design Checker says so.
    void linkedPictureMissingShowsPreview()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(1200, 900), QColor(200, 30, 30));
        jp::MainWindow w;
        w.show();
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::Link);
        const QString path = dir.filePath(QStringLiteral("book.jpub"));
        QVERIFY(w.saveTo(path));
        const QString id = linkFirstPicture(w.editor()->doc())->imageId;
        QVERIFY(QFile::remove(file));
        QCOMPARE(w.editor()->doc()->linkStatus(id), jp::LinkStatus::Missing);   // gone while the publication is open

        QString err;
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->linkStatus(id), jp::LinkStatus::Missing);
        QCOMPARE(back->imageSize(id), QSize(1200, 900));
        const QImage shown = back->image(id);
        QVERIFY(!shown.isNull());
        QCOMPARE(std::max(shown.width(), shown.height()), 512);   // the preview
        // ... and it is what the page shows.
        jp::PictureItem *pic = linkFirstPicture(back.get());
        const QImage page = linkRender(back.get());
        const QPoint left = (pic->rect.topLeft() + QPointF(pic->rect.width() * 0.25, pic->rect.height() * 0.5)).toPoint();
        const QPoint right = (pic->rect.topLeft() + QPointF(pic->rect.width() * 0.75, pic->rect.height() * 0.5)).toPoint();
        QVERIFY(linkColorsClose(QColor(page.pixel(left)), QColor(200, 30, 30)));
        QVERIFY(linkColorsClose(QColor(page.pixel(right)), QColor(55, 225, 225)));

        // The Design Checker names both problems.
        w.editor()->setDocument(std::move(back), path);
        w.showTaskPane(QStringLiteral("designchecker"));
        w.refreshUi();
        QStringList found;
        for (auto *l : w.findChildren<QListWidget *>())
            for (int i = 0; i < l->count(); ++i) found << l->item(i)->text();
        QVERIFY2(found.contains(QStringLiteral("Linked picture is missing (Page 1)")), qPrintable(found.join('|')));
        QVERIFY2(found.contains(QStringLiteral("Picture is linked, not embedded (Page 1)")), qPrintable(found.join('|')));
    }

    // A changed file shows as Modified, in the Graphics Manager too, until
    // Update Link reads it again; Update Link can be undone.
    void linkedPictureModifiedUpdates()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(800, 600), QColor(200, 30, 30));
        jp::MainWindow w;
        w.show();
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::Link);
        jp::Document *doc = w.editor()->doc();
        jp::PictureItem *pic = linkFirstPicture(doc);
        const QString oldId = pic->imageId;
        QCOMPARE(doc->linkStatus(oldId), jp::LinkStatus::Linked);

        // Changed while open: Modified, still drawn as it was, then updated.
        linkPicture(file, QSize(600, 300), QColor(30, 200, 30));
        QCOMPARE(doc->linkStatus(oldId), jp::LinkStatus::Modified);
        QCOMPARE(doc->image(oldId).size(), QSize(800, 600));
        w.editor()->select(pic->id);
        w.showTaskPane(QStringLiteral("graphics"));
        w.refreshUi();
        QListWidget *list = nullptr;
        for (auto *l : w.findChildren<QListWidget *>())
            if (l->accessibleName() == QLatin1String("Pictures")) list = l;
        QVERIFY(list);
        QCOMPARE(list->count(), 1);
        QVERIFY2(list->item(0)->text().contains(QStringLiteral("Modified")), qPrintable(list->item(0)->text()));
        list->setCurrentRow(0);
        QPushButton *update = nullptr;
        for (auto *b : w.findChildren<QPushButton *>())
            if (b->text() == QLatin1String("Update Link")) update = b;
        QVERIFY(update);
        QVERIFY(update->isEnabled());
        update->click();
        w.refreshUi();
        pic = linkFirstPicture(doc);
        QVERIFY(pic->imageId != oldId);
        QCOMPARE(doc->linkStatus(pic->imageId), jp::LinkStatus::Linked);
        QCOMPARE(doc->image(pic->imageId).size(), QSize(600, 300));
        QVERIFY(std::abs(pic->imgRect.width() / pic->imgRect.height() - 2.0) < 0.02);   // the new shape, not the old one
        QVERIFY2(list->item(0)->text().contains(QStringLiteral("Linked")), qPrintable(list->item(0)->text()));
        QCOMPARE(doc->images[pic->imageId].preview.isEmpty(), false);
        QCOMPARE(QImage::fromData(doc->images[pic->imageId].preview).size(), QSize(512, 256));   // the preview follows the file

        w.editor()->undo();
        QCOMPARE(linkFirstPicture(doc)->imageId, oldId);
        QCOMPARE(doc->linkStatus(oldId), jp::LinkStatus::Modified);

        // Changed while closed: the picture draws from the file, Modified until updated.
        const QString path = dir.filePath(QStringLiteral("book.jpub"));
        QVERIFY(w.saveTo(path));
        linkPicture(file, QSize(300, 600), QColor(30, 30, 200));
        QString err;
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->linkStatus(oldId), jp::LinkStatus::Modified);
        QCOMPARE(back->image(oldId).size(), QSize(300, 600));
    }

    // Embed Picture turns a link into a stored copy that outlives the file;
    // Change Link points the picture at another file.
    void embedAndChangeLinkedPicture()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(800, 600), QColor(200, 30, 30), true);
        const QString other = linkPicture(dir.filePath(QStringLiteral("other.png")), QSize(400, 400), QColor(30, 30, 200));
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::Link);
        jp::Document *doc = w.editor()->doc();
        const QString picId = linkFirstPicture(doc)->id;
        const QString linkedId = linkFirstPicture(doc)->imageId;

        QVERIFY(w.editor()->changeLink(picId, other));
        QCOMPARE(doc->images[linkFirstPicture(doc)->imageId].sourcePath, QFileInfo(other).absoluteFilePath());
        QCOMPARE(doc->image(linkFirstPicture(doc)->imageId).size(), QSize(400, 400));
        QVERIFY(!w.editor()->changeLink(picId, dir.filePath(QStringLiteral("nothing.png"))));   // no such file: nothing changes
        QCOMPARE(doc->images[linkFirstPicture(doc)->imageId].sourcePath, QFileInfo(other).absoluteFilePath());
        w.editor()->undo();
        QCOMPARE(linkFirstPicture(doc)->imageId, linkedId);

        QVERIFY(w.editor()->embedPicture(picId));
        const QString embeddedId = linkFirstPicture(doc)->imageId;
        QVERIFY(embeddedId != linkedId);
        QVERIFY(!doc->images[embeddedId].linked);
        QCOMPARE(doc->linkStatus(embeddedId), jp::LinkStatus::Embedded);
        QCOMPARE(doc->images[embeddedId].bytes, linkFileBytes(file));
        QVERIFY(!w.editor()->embedPicture(picId));   // nothing left to embed

        const QString path = dir.filePath(QStringLiteral("book.jpub"));
        QVERIFY(w.saveTo(path));
        QVERIFY(QFile::remove(file));
        QString err;
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->linkStatus(embeddedId), jp::LinkStatus::Embedded);
        QCOMPARE(back->image(embeddedId).size(), QSize(800, 600));

        w.editor()->undo();   // back to the link
        QCOMPARE(linkFirstPicture(doc)->imageId, linkedId);
        QCOMPARE(doc->linkStatus(linkedId), jp::LinkStatus::Missing);
    }

    // Saving to .pub embeds the full picture from the file; when the file is
    // gone, the preview.
    void pubExportEmbedsLinkedPictures()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(1000, 800), QColor(200, 30, 30), true);
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::Link);
        const QString path = dir.filePath(QStringLiteral("book.jpub"));
        QVERIFY(w.saveTo(path));

        QString err;
        auto exportedSize = [&](jp::Document *doc, const QString &name) {
            const QString pub = dir.filePath(name);
            if (!jp::exportPublisher(*doc, pub, &err)) return QSize();
            auto back = jp::importPublisherFile(pub, &err);
            if (!back) return QSize();
            for (const auto &it : back->pages[0]->items)
                if (auto *p = dynamic_cast<jp::PictureItem *>(it.get())) return back->image(p->imageId).size();
            return QSize();
        };
        auto present = jp::loadPublication(path, &err);
        QVERIFY2(present, qPrintable(err));
        QCOMPARE(exportedSize(present.get(), QStringLiteral("full.pub")), QSize(1000, 800));

        QVERIFY(QFile::remove(file));
        auto gone = jp::loadPublication(path, &err);
        QVERIFY2(gone, qPrintable(err));
        const QSize small = exportedSize(gone.get(), QStringLiteral("preview.pub"));
        QVERIFY2(!small.isEmpty() && std::max(small.width(), small.height()) == 512, qPrintable(err));
    }

    // Pack and Go and e-mail send the pictures themselves, not links to files
    // the other computer doesn't have.
    void sharedPublicationsEmbedLinkedPictures()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(900, 700), QColor(200, 30, 30));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto item = std::make_shared<jp::PictureItem>();
        item->rect = QRectF(100, 100, 400, 300);
        item->imgRect = QRectF(0, 0, 400, 300);
        item->imageId = doc->addLinkedImage(linkFileBytes(file), QStringLiteral("png"), file, false);
        const QString id = item->imageId;
        doc->pages[0]->items.push_back(item);
        QString err;
        const QByteArray sent = jp::publicationBytes(*doc, QImage(), QString(), true);
        QVERIFY(QFile::remove(file));
        auto there = jp::publicationFromBytes(sent, &err);
        QVERIFY2(there, qPrintable(err));
        QCOMPARE(there->linkStatus(id), jp::LinkStatus::NotUpdated);   // opened with no folder of its own: no file is looked at
        QCOMPARE(there->image(id).size(), QSize(900, 700));   // whole, not the preview
    }

    // Files from before pictures could be linked have the same pictures,
    // embedded, and a picture that isn't linked writes nothing about links.
    void embeddedPicturesWriteNoLinkFields()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(300, 200), QColor(200, 30, 30));
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1));
        const QString id = linkFirstPicture(w.editor()->doc())->imageId;
        const QJsonObject io = w.editor()->doc()->toJson()["images"].toArray()[0].toObject();
        QStringList keys = io.keys();
        keys.sort();
        QCOMPARE(keys, (QStringList{"format", "h", "id", "linked", "source", "w"}));
        QString err;
        auto back = jp::publicationFromBytes(jp::publicationBytes(*w.editor()->doc(), QImage()), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->linkStatus(id), jp::LinkStatus::Embedded);
        QVERIFY(!back->images[id].linked);
        QCOMPARE(back->images[id].bytes, linkFileBytes(file));
        // An embedded copy of a file that is also linked stays a separate picture.
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::Link);
        w.insertFiles({file}, QPointF(-1, -1));
        QSet<QString> ids;
        for (const auto &it : w.editor()->doc()->pages[0]->items)
            if (auto *p = dynamic_cast<jp::PictureItem *>(it.get())) ids.insert(p->imageId);
        QCOMPARE(ids.size(), 2);
    }

    // Insert Picture's file dialog has an "Insert as" choice with a name, and
    // the choice decides how the picture joins the publication.
    void insertPictureDialogOffersLinking()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(300, 200), QColor(200, 30, 30));
        jp::Settings::get().setValue(QStringLiteral("dirs/pictures"), dir.path());
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QString name;
        QStringList choices;
        QTimer poll;
        poll.setInterval(20);
        QObject::connect(&poll, &QTimer::timeout, [&] {
            auto *dlg = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            poll.stop();
            if (auto *how = dlg->findChild<QComboBox *>(QStringLiteral("insertAs"))) {
                name = how->accessibleName();
                for (int i = 0; i < how->count(); ++i) choices << how->itemText(i);
                how->setCurrentIndex(2);
            }
            if (auto *typed = dlg->findChild<QLineEdit *>(QStringLiteral("fileNameEdit"))) typed->setText(file);
            QMetaObject::invokeMethod(dlg, "accept");
        });
        poll.start();
        w.insertPictureFromFile();
        poll.stop();
        QCOMPARE(name, QStringLiteral("Insert as"));
        QCOMPARE(choices, (QStringList{"Insert", "Link to File", "Insert and Link"}));
        jp::PictureItem *pic = linkFirstPicture(w.editor()->doc());
        QVERIFY(pic);
        const jp::ImageData &img = w.editor()->doc()->images[pic->imageId];
        QVERIFY(img.linked);
        QVERIFY(img.keepsCopy);
    }

    // A linked picture that is copied and pasted stays linked to its file.
    void copiedLinkedPictureStaysLinked()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = linkPicture(dir.filePath(QStringLiteral("photo.png")), QSize(300, 200), QColor(200, 30, 30));
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::Link);
        jp::Document *doc = w.editor()->doc();
        const QString id = linkFirstPicture(doc)->imageId;
        w.editor()->select(linkFirstPicture(doc)->id);
        w.editor()->copy();
        w.editor()->paste();
        QCOMPARE(int(doc->pages[0]->items.size()), 2);
        for (const auto &it : doc->pages[0]->items) {
            auto *p = dynamic_cast<jp::PictureItem *>(it.get());
            QVERIFY(p);
            QVERIFY(doc->images[p->imageId].linked);
            QCOMPARE(p->imageId, id);
        }
    }

    // A picture file is read only when it is a regular file of at most
    // kMaxPictureFile bytes: not a pipe or a device, not an empty file, not a
    // huge one. Every way in (open, Insert, Update Link, Change Link) uses it.
    void pictureFilesAreReadSafely()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString good = linkPicture(dir.filePath(QStringLiteral("good.png")), QSize(40, 30), QColor(200, 30, 30));
        auto read = [](const QString &path) {
            QByteArray bytes;
            QString format;
            QSize px;
            return jp::readPictureFile(path, 0, &bytes, &format, &px);
        };
        QVERIFY(read(good));
        QVERIFY(!read(dir.filePath(QStringLiteral("nothing.png"))));
        QVERIFY(!read(dir.path()));   // a folder
        QFile empty(dir.filePath(QStringLiteral("empty.png")));
        QVERIFY(empty.open(QIODevice::WriteOnly));
        empty.close();
        QVERIFY(!read(empty.fileName()));
        // A real picture followed by zeros, one byte past the limit: too big, though it would decode.
        QByteArray padded = linkFileBytes(good);
        QFile big(dir.filePath(QStringLiteral("big.png")));
        QVERIFY(big.open(QIODevice::WriteOnly));
        QVERIFY(big.write(padded) == padded.size());
        QVERIFY(big.resize(jp::kMaxPictureFile + 1));
        big.close();
        QVERIFY(!read(big.fileName()));
        QVERIFY(big.resize(jp::kMaxPictureFile));   // exactly the limit is still fine
        QVERIFY(read(big.fileName()));
        QVERIFY(big.remove());
#ifdef Q_OS_UNIX
        // A named pipe would block the first open of it forever, and a file the
        // system reports as empty can be endless.
        const QString fifo = dir.filePath(QStringLiteral("pipe.png"));
        QVERIFY(::mkfifo(QFile::encodeName(fifo).constData(), 0600) == 0);
        QVERIFY(!read(fifo));
        QVERIFY(!read(QStringLiteral("/dev/zero")));
        QVERIFY(!read(QStringLiteral("/dev/null")));
        if (QFileInfo::exists(QStringLiteral("/proc/self/maps"))) QVERIFY(!read(QStringLiteral("/proc/self/maps")));   // a file of size 0

        // Insert, Change Link, and Update Link on it do nothing, and don't wait.
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({fifo}, QPointF(-1, -1), jp::PictureInsert::Embed);
        QVERIFY(!linkFirstPicture(w.editor()->doc()));
        w.insertFiles({good}, QPointF(-1, -1), jp::PictureInsert::Link);
        jp::PictureItem *pic = linkFirstPicture(w.editor()->doc());
        QVERIFY(pic);
        const QString picId = pic->id, imageId = pic->imageId;
        QVERIFY(!w.editor()->changeLink(picId, fifo));
        QVERIFY(QFile::remove(good));
        QVERIFY(::mkfifo(QFile::encodeName(good).constData(), 0600) == 0);   // the linked file becomes a pipe
        QCOMPARE(w.editor()->doc()->linkStatus(imageId), jp::LinkStatus::Missing);
        QVERIFY(!w.editor()->updateLink(picId));
        QCOMPARE(linkFirstPicture(w.editor()->doc())->imageId, imageId);
#endif
    }

    // A .jpub with its image entries changed by `edit` (a crafted file).
    static QByteArray rewriteImageEntries(const QByteArray &jpub, const std::function<void(QJsonArray &)> &edit)
    {
        QMap<QString, QByteArray> entries;
        if (!jp::readZip(jpub, entries)) return QByteArray();
        QJsonObject json = QJsonDocument::fromJson(entries["document.json"]).object();
        QJsonArray images = json["images"].toArray();
        edit(images);
        json["images"] = images;
        entries["document.json"] = QJsonDocument(json).toJson(QJsonDocument::Compact);
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), entries.take(QStringLiteral("mimetype")));
        for (auto it = entries.cbegin(); it != entries.cend(); ++it) z.add(it.key(), it.value());
        return z.finish();
    }

    // A picture as file bytes: the left half `color`, the right half the opposite.
    static QByteArray linkPictureBytes(const QSize &size, const QColor &color)
    {
        QTemporaryDir tmp;
        const QString path = linkPicture(tmp.filePath(QStringLiteral("p.png")), size, color);
        return linkFileBytes(path);
    }

    static void linkWrite(const QString &path, const QByteArray &bytes, qint64 msecs)
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QVERIFY(f.write(bytes) == bytes.size());
        QVERIFY(f.flush());   // closing the file later must not touch its date again
        QVERIFY(f.setFileTime(QDateTime::fromMSecsSinceEpoch(msecs), QFileDevice::FileModificationTime));
    }

    // A linked picture is Modified when the file's contents differ, not when
    // its size and date do: two different files can share both (written in
    // one clock tick), and a file saved again unchanged differs in neither
    // way that matters.
    void linkedPictureModifiedByContent()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QByteArray red = linkPictureBytes(QSize(100, 80), QColor(200, 30, 30));
        QByteArray blue = linkPictureBytes(QSize(100, 80), QColor(30, 30, 200));
        // Zeros after the picture's end change nothing in it: the two files get one size.
        const qsizetype size = std::max(red.size(), blue.size());
        red.append(QByteArray(size - red.size(), '\0'));
        blue.append(QByteArray(size - blue.size(), '\0'));
        QVERIFY(red != blue && red.size() == blue.size());
        const qint64 then = 1700000000000;
        const QString file = dir.filePath(QStringLiteral("photo.png"));
        linkWrite(file, red, then);

        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto item = std::make_shared<jp::PictureItem>();
        item->rect = QRectF(100, 100, 200, 160);
        item->imgRect = QRectF(0, 0, 200, 160);
        item->imageId = doc->addLinkedImage(red, QStringLiteral("png"), file, false);
        const QString id = item->imageId;
        doc->pages[0]->items.push_back(item);
        QCOMPARE(doc->images[id].fileHash, QCryptographicHash::hash(red, QCryptographicHash::Sha1).toHex());
        QString err;
        const QString path = dir.filePath(QStringLiteral("book.jpub"));
        QVERIFY(jp::savePublication(*doc, path, QImage(), &err));
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(linkFileBytes(path), entries));
        QCOMPARE(QJsonDocument::fromJson(entries["document.json"]).object()["images"].toArray()[0].toObject()["fileHash"].toString().toLatin1(),
                 QCryptographicHash::hash(red, QCryptographicHash::Sha1).toHex());

        auto same = jp::loadPublication(path, &err);
        QVERIFY2(same, qPrintable(err));
        QCOMPARE(same->linkStatus(id), jp::LinkStatus::Linked);

        // Another picture, the same size, the same date.
        linkWrite(file, blue, then);
        QCOMPARE(QFileInfo(file).size(), qint64(size));
        auto other = jp::loadPublication(path, &err);
        QVERIFY2(other, qPrintable(err));
        QCOMPARE(other->linkStatus(id), jp::LinkStatus::Modified);
        QCOMPARE(other->images[id].bytes, blue);   // drawn from the file until updated
        const jp::PictureItem *shown = linkFirstPicture(other.get());
        const QImage page = linkRender(other.get());
        const QPoint left = (shown->rect.topLeft() + QPointF(shown->rect.width() * 0.25, shown->rect.height() * 0.5)).toPoint();
        QVERIFY(linkColorsClose(QColor(page.pixel(left)), QColor(30, 30, 200)));

        // The same picture, saved again a minute later: still the one that was linked.
        linkWrite(file, red, then + 60000);
        auto touched = jp::loadPublication(path, &err);
        QVERIFY2(touched, qPrintable(err));
        QCOMPARE(touched->linkStatus(id), jp::LinkStatus::Linked);

        // A file from before links had a hash has only the size and date to go by.
        const QByteArray old = rewriteImageEntries(linkFileBytes(path), [](QJsonArray &images) {
            QJsonObject io = images[0].toObject();
            io.remove(QStringLiteral("fileHash"));
            images[0] = io;
        });
        QVERIFY(!old.isEmpty());
        linkWrite(file, blue, then);
        auto before = jp::publicationFromBytes(old, &err, dir.path());
        QVERIFY2(before, qPrintable(err));
        QCOMPARE(before->linkStatus(id), jp::LinkStatus::Linked);   // the same size and date: no way to tell
        linkWrite(file, blue + QByteArray(10, '\0'), then);
        auto longer = jp::publicationFromBytes(old, &err, dir.path());
        QVERIFY2(longer, qPrintable(err));
        QCOMPARE(longer->linkStatus(id), jp::LinkStatus::Modified);

        // Update Link keeps the hash of what it read.
        jp::MainWindow w;
        w.editor()->setDocument(std::move(other), path);
        const QString picId = linkFirstPicture(w.editor()->doc())->id;
        linkWrite(file, blue, then);
        QVERIFY(w.editor()->updateLink(picId));
        const jp::ImageData &updated = w.editor()->doc()->images[linkFirstPicture(w.editor()->doc())->imageId];
        QCOMPARE(updated.fileHash, QCryptographicHash::hash(blue, QCryptographicHash::Sha1).toHex());
    }

    // A publication that arrives from someone else must not make JeffPub
    // reach for files of the computer it opens on. A link to a file in the
    // publication's own folder or below is followed when it opens; any other
    // (elsewhere, up a `..`, over a symbolic link, a network or device path)
    // is not touched at all, not even to ask if it is there: the page shows
    // the stored copy or preview, and the status is Not updated.
    void linksOutsideTheFolderAreNotFollowed()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir root(dir.path());
        QVERIFY(root.mkpath(QStringLiteral("pub/sub")) && root.mkpath(QStringLiteral("pub2")) && root.mkpath(QStringLiteral("outside")));
        const QString pub = root.filePath(QStringLiteral("pub"));
        const QSize big(1000, 800);
        const QString inside = linkPicture(pub + QStringLiteral("/sub/inside.png"), big, QColor(200, 30, 30));
        const QString secret = linkPicture(root.filePath(QStringLiteral("outside/secret.png")), big, QColor(30, 30, 200));
        const QString sibling = linkPicture(root.filePath(QStringLiteral("pub2/near.png")), big, QColor(30, 200, 30));   // "pub" is the start of "pub2"
        auto doc = jp::Document::blank(QSizeF(612, 792));
        int row = 0;
        auto link = [&](const QString &file) {
            auto item = std::make_shared<jp::PictureItem>();
            item->rect = QRectF(50, 50 + 100 * row++, 100, 80);
            item->imgRect = QRectF(0, 0, 100, 80);
            item->imageId = doc->addLinkedImage(linkFileBytes(file), QStringLiteral("png"), file, false);
            doc->pages[0]->items.push_back(item);
            return item->imageId;
        };
        const QString idInside = link(inside), idSecret = link(secret), idSibling = link(sibling);
        QString err;
        const QString path = pub + QStringLiteral("/book.jpub");
        QVERIFY(jp::savePublication(*doc, path, QImage(), &err));
        const QByteArray saved = linkFileBytes(path);

        // Each file changes after the link was made, so that a file that is read shows.
        for (const QString &f : {inside, secret, sibling}) linkPicture(f, big, QColor(250, 250, 10));
        auto isStored = [&](const jp::Document &d, const QString &id) {   // the preview, not the file
            return d.images[id].bytes != linkFileBytes(inside) && d.images[id].bytes != linkFileBytes(secret) && d.images[id].bytes != linkFileBytes(sibling) &&
                   std::max(d.image(id).width(), d.image(id).height()) == 512;
        };
        auto loaded = jp::publicationFromBytes(saved, &err, pub);
        QVERIFY2(loaded, qPrintable(err));
        QCOMPARE(loaded->linkStatus(idInside), jp::LinkStatus::Modified);   // in the folder: read, and found changed
        QCOMPARE(loaded->images[idInside].bytes, linkFileBytes(inside));
        QCOMPARE(loaded->linkStatus(idSecret), jp::LinkStatus::NotUpdated);
        QVERIFY(isStored(*loaded, idSecret));
        QCOMPARE(loaded->linkStatus(idSibling), jp::LinkStatus::NotUpdated);
        QVERIFY(isStored(*loaded, idSibling));
        QCOMPARE(loaded->imageSize(idSecret), big);   // the file's size is still known
        // The page shows the preview of the outside picture (the second): its left half is blue, not the yellow of the file.
        const QImage page = linkRender(loaded.get());
        QVERIFY(linkColorsClose(QColor(page.pixel(75, 190)), QColor(30, 30, 200)));

        // Without a folder to be in (a publication from a message, say), no link is followed.
        auto nowhere = jp::publicationFromBytes(saved, &err);
        QVERIFY2(nowhere, qPrintable(err));
        QCOMPARE(nowhere->linkStatus(idInside), jp::LinkStatus::NotUpdated);
        QVERIFY(isStored(*nowhere, idInside));

        // Paths a file can name that reach other computers and devices, and
        // `..`, absolute, and bad relative ones. None of them is there, and
        // none is looked for: Not updated, never Missing.
        const QStringList sources{QStringLiteral("\\\\server\\share\\x.png"), QStringLiteral("//server/share/x.png"), QStringLiteral("\\\\?\\C:\\x.png"),
                                  QStringLiteral("\\\\?\\UNC\\server\\share\\x.png"), QStringLiteral("\\\\.\\pipe\\x"), QStringLiteral("\\\\.\\C:"),
                                  pub + QStringLiteral("/../outside/secret.png"), pub + QStringLiteral("/sub/../../outside/nothing.png"),
                                  QStringLiteral("pictures/x.png"), QStringLiteral("/nonexistent-folder/x.png"), QStringLiteral("/dev/zero"),
                                  QStringLiteral("/proc/self/pagemap"), QString()};
        for (const QString &source : sources) {
            const QByteArray crafted = rewriteImageEntries(saved, [&](QJsonArray &images) {
                for (int i = 0; i < images.size(); ++i) {
                    QJsonObject io = images[i].toObject();
                    if (io["id"].toString() != idSecret) continue;
                    io["source"] = source;
                    io.remove(QStringLiteral("relative"));
                    images[i] = io;
                }
            });
            auto d = jp::publicationFromBytes(crafted, &err, pub);
            QVERIFY2(d, qPrintable(err));
            QVERIFY2(d->linkStatus(idSecret) == jp::LinkStatus::NotUpdated, qPrintable(source));
            QVERIFY2(isStored(*d, idSecret), qPrintable(source));
        }
        // A `relative` that leaves the folder, or is absolute, is not followed either.
        for (const QString &relative : {QStringLiteral("../outside/secret.png"), root.filePath(QStringLiteral("outside/secret.png")), QStringLiteral("\\\\server\\share\\x.png"),
                                        QStringLiteral("//server/share/x.png"), QStringLiteral("sub/../../outside/secret.png"), QStringLiteral("C:\\x.png")}) {
            const QByteArray crafted = rewriteImageEntries(saved, [&](QJsonArray &images) {
                for (int i = 0; i < images.size(); ++i) {
                    QJsonObject io = images[i].toObject();
                    if (io["id"].toString() != idSecret) continue;
                    io["source"] = QString();
                    io["relative"] = relative;
                    images[i] = io;
                }
            });
            auto d = jp::publicationFromBytes(crafted, &err, pub);
            QVERIFY2(d, qPrintable(err));
            QVERIFY2(d->linkStatus(idSecret) == jp::LinkStatus::NotUpdated, qPrintable(relative));
            QVERIFY2(isStored(*d, idSecret), qPrintable(relative));
        }
        // A relative path inside the folder still finds the file when the full path is stale.
        const QByteArray moved = rewriteImageEntries(saved, [&](QJsonArray &images) {
            for (int i = 0; i < images.size(); ++i) {
                QJsonObject io = images[i].toObject();
                if (io["id"].toString() != idInside) continue;
                io["source"] = QStringLiteral("/somewhere/else/inside.png");
                io["relative"] = QStringLiteral("sub/inside.png");
                images[i] = io;
            }
        });
        auto found = jp::publicationFromBytes(moved, &err, pub);
        QVERIFY2(found, qPrintable(err));
        QCOMPARE(found->images[idInside].sourcePath, QFileInfo(inside).absoluteFilePath());
        QCOMPARE(found->linkStatus(idInside), jp::LinkStatus::Modified);
        // A `..` that comes back down stays inside.
        const QByteArray round = rewriteImageEntries(saved, [&](QJsonArray &images) {
            for (int i = 0; i < images.size(); ++i) {
                QJsonObject io = images[i].toObject();
                if (io["id"].toString() != idInside) continue;
                io["source"] = pub + QStringLiteral("/sub/../sub/inside.png");
                io.remove(QStringLiteral("relative"));
                images[i] = io;
            }
        });
        auto roundDown = jp::publicationFromBytes(round, &err, pub);
        QVERIFY2(roundDown, qPrintable(err));
        QCOMPARE(roundDown->linkStatus(idInside), jp::LinkStatus::Modified);
        QCOMPARE(roundDown->images[idInside].bytes, linkFileBytes(inside));

#ifndef Q_OS_WIN
        // A symbolic link in the folder that leads out of it is outside; one that stays in is not.
        QVERIFY(QFile::link(secret, pub + QStringLiteral("/escape.png")));
        QVERIFY(QFile::link(root.filePath(QStringLiteral("outside")), pub + QStringLiteral("/escapedir")));
        QVERIFY(QFile::link(inside, pub + QStringLiteral("/alias.png")));
        QVERIFY(QFile::link(pub + QStringLiteral("/sub"), pub + QStringLiteral("/aliasdir")));
        struct Case { QString source; jp::LinkStatus status; };
        for (const Case &c : {Case{pub + QStringLiteral("/escape.png"), jp::LinkStatus::NotUpdated}, Case{pub + QStringLiteral("/escapedir/secret.png"), jp::LinkStatus::NotUpdated},
                              Case{pub + QStringLiteral("/alias.png"), jp::LinkStatus::Modified}, Case{pub + QStringLiteral("/aliasdir/inside.png"), jp::LinkStatus::Modified}}) {
            const QByteArray crafted = rewriteImageEntries(saved, [&](QJsonArray &images) {
                for (int i = 0; i < images.size(); ++i) {
                    QJsonObject io = images[i].toObject();
                    if (io["id"].toString() != idSecret) continue;
                    io["source"] = c.source;
                    io.remove(QStringLiteral("relative"));
                    images[i] = io;
                }
            });
            auto d = jp::publicationFromBytes(crafted, &err, pub);
            QVERIFY2(d, qPrintable(err));
            QVERIFY2(d->linkStatus(idSecret) == c.status, qPrintable(c.source));
            QCOMPARE(d->images[idSecret].bytes == linkFileBytes(inside), c.status == jp::LinkStatus::Modified);
            QVERIFY2(c.status == jp::LinkStatus::Modified || isStored(*d, idSecret), qPrintable(c.source));
        }
#endif
    }

    // The one way to follow a link to a file outside the folder is Update
    // Link (or Change Link) on that picture in the Graphics Manager; it holds
    // for the session, and saving keeps the path as it was.
    void outsideLinkIsFollowedByUpdateLinkOnly()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir root(dir.path());
        QVERIFY(root.mkpath(QStringLiteral("pub")) && root.mkpath(QStringLiteral("outside")));
        const QString one = linkPicture(root.filePath(QStringLiteral("outside/one.png")), QSize(800, 600), QColor(200, 30, 30));
        const QString two = linkPicture(root.filePath(QStringLiteral("outside/two.png")), QSize(800, 600), QColor(30, 200, 30));
        const QString other = linkPicture(root.filePath(QStringLiteral("outside/other.png")), QSize(400, 400), QColor(30, 30, 200));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QStringList imageIds, pictureIds;
        for (const QString &file : {one, two}) {
            auto item = std::make_shared<jp::PictureItem>();
            item->rect = QRectF(50, 50 + 200 * imageIds.size(), 160, 120);
            item->imgRect = QRectF(0, 0, 160, 120);
            item->imageId = doc->addLinkedImage(linkFileBytes(file), QStringLiteral("png"), file, false);
            imageIds << item->imageId;
            pictureIds << item->id;
            doc->pages[0]->items.push_back(item);
        }
        QString err;
        const QString path = root.filePath(QStringLiteral("pub/book.jpub"));
        QVERIFY(jp::savePublication(*doc, path, QImage(), &err));
        const QByteArray fileOne = linkFileBytes(one);
        linkPicture(one, QSize(800, 600), QColor(250, 250, 10));   // changed since
        linkPicture(two, QSize(800, 600), QColor(250, 250, 10));

        auto loaded = jp::loadPublication(path, &err);
        QVERIFY2(loaded, qPrintable(err));
        QCOMPARE(loaded->linkStatus(imageIds[0]), jp::LinkStatus::NotUpdated);
        QCOMPARE(loaded->linkStatus(imageIds[1]), jp::LinkStatus::NotUpdated);
        jp::MainWindow w;
        w.show();
        w.editor()->setDocument(std::move(loaded), path);
        jp::Document *d = w.editor()->doc();
        w.editor()->select(pictureIds[0]);
        w.showTaskPane(QStringLiteral("graphics"));
        w.refreshUi();
        QListWidget *list = nullptr;
        for (auto *l : w.findChildren<QListWidget *>())
            if (l->accessibleName() == QLatin1String("Pictures")) list = l;
        QVERIFY(list);
        QCOMPARE(list->count(), 2);
        for (int i = 0; i < 2; ++i) QVERIFY2(list->item(i)->text().contains(QStringLiteral("Not updated")), qPrintable(list->item(i)->text()));
        list->setCurrentRow(0);
        QString details;
        for (auto *l : w.findChildren<QLabel *>())
            if (l->text().startsWith(QLatin1String("Status:"))) details = l->text();
        QVERIFY2(details.contains(QStringLiteral("Not updated")) && details.contains(QStringLiteral("outside the publication's folder")), qPrintable(details));

        QPushButton *update = nullptr;
        for (auto *b : w.findChildren<QPushButton *>())
            if (b->text() == QLatin1String("Update Link")) update = b;
        QVERIFY(update && update->isEnabled());
        update->click();
        w.refreshUi();
        jp::PictureItem *first = dynamic_cast<jp::PictureItem *>(d->item(pictureIds[0]));
        jp::PictureItem *second = dynamic_cast<jp::PictureItem *>(d->item(pictureIds[1]));
        QVERIFY(first && second);
        QCOMPARE(d->linkStatus(first->imageId), jp::LinkStatus::Linked);
        QCOMPARE(d->images[first->imageId].bytes, linkFileBytes(one));
        QVERIFY(d->images[first->imageId].bytes != fileOne);
        QCOMPARE(d->linkStatus(second->imageId), jp::LinkStatus::NotUpdated);   // not followed with it
        QVERIFY(d->images[second->imageId].bytes != linkFileBytes(two));

        // Saved, the link is the one it was; opened again, it is outside again.
        QVERIFY(w.saveTo(path));
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(linkFileBytes(path), entries));
        QJsonObject io;
        for (const auto &v : QJsonDocument::fromJson(entries["document.json"]).object()["images"].toArray())
            if (v.toObject()["source"].toString() == QFileInfo(one).absoluteFilePath()) io = v.toObject();
        QVERIFY(!io.isEmpty());
        QCOMPARE(io["relative"].toString(), QStringLiteral("../outside/one.png"));
        auto again = jp::loadPublication(path, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(again->linkStatus(first->imageId), jp::LinkStatus::NotUpdated);

        w.editor()->undo();   // Update Link can be undone
        QCOMPARE(d->linkStatus(dynamic_cast<jp::PictureItem *>(d->item(pictureIds[0]))->imageId), jp::LinkStatus::NotUpdated);

        // Change Link points the other picture at a file, and follows it.
        QVERIFY(w.editor()->changeLink(pictureIds[1], other));
        const QString changed = dynamic_cast<jp::PictureItem *>(d->item(pictureIds[1]))->imageId;
        QCOMPARE(d->linkStatus(changed), jp::LinkStatus::Linked);
        QCOMPARE(d->image(changed).size(), QSize(400, 400));
    }

    // The mail merge's picture files follow the same rule: those in the
    // publication's folder (or below) show, others are not looked for. A file
    // that is not there is not asked after at every repaint.
    void mergePicturesStayInTheFolder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir root(dir.path());
        QVERIFY(root.mkpath(QStringLiteral("pub/faces")) && root.mkpath(QStringLiteral("outside")));
        linkPicture(root.filePath(QStringLiteral("pub/faces/in.png")), QSize(200, 160), QColor(200, 30, 30));
        linkPicture(root.filePath(QStringLiteral("outside/out.png")), QSize(200, 160), QColor(30, 30, 200));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto frame = std::make_shared<jp::PictureItem>();
        frame->rect = QRectF(100, 100, 200, 160);
        frame->imgRect = QRectF(0, 0, 200, 160);
        frame->name = QStringLiteral("merge:Photo");
        doc->pages[0]->items.push_back(frame);
        doc->merge.path = root.filePath(QStringLiteral("pub/list.csv"));
        doc->merge.fields = {QStringLiteral("Photo")};
        doc->merge.rows = {{QStringLiteral("faces/in.png")}, {QStringLiteral("../outside/out.png")}, {root.filePath(QStringLiteral("outside/out.png"))},
                           {QStringLiteral("\\\\server\\share\\x.png")}, {QStringLiteral("later.png")}};
        doc->merge.include = {true, true, true, true, true};
        auto shows = [&](jp::Document *d, int record) {
            jp::LayoutCache cache;
            jp::PaintContext ctx;
            ctx.doc = d;
            ctx.cache = &cache;
            ctx.opt.output = true;
            ctx.opt.mergeRecord = record;
            const QImage page = jp::Renderer::renderToImage(ctx, 0, 1.0);
            return QColor(page.pixel(150, 180));   // the left half of the frame
        };
        // A publication made in this session has no folder it must keep to.
        QVERIFY(linkColorsClose(shows(doc.get(), 0), QColor(200, 30, 30)));
        QVERIFY(linkColorsClose(shows(doc.get(), 1), QColor(30, 30, 200)));

        QString err;
        const QString path = root.filePath(QStringLiteral("pub/book.jpub"));
        QVERIFY(jp::savePublication(*doc, path, QImage(), &err));
        auto loaded = jp::loadPublication(path, &err);
        QVERIFY2(loaded, qPrintable(err));
        QVERIFY(linkColorsClose(shows(loaded.get(), 0), QColor(200, 30, 30)));
        for (int record : {1, 2, 3}) QVERIFY2(linkColorsClose(shows(loaded.get(), record), QColor(255, 255, 255)), qPrintable(QString::number(record)));

        // A picture that is not there yet: not there at the next repaint either, though it has come.
        QVERIFY(linkColorsClose(shows(loaded.get(), 4), QColor(255, 255, 255)));
        linkPicture(root.filePath(QStringLiteral("pub/later.png")), QSize(200, 160), QColor(30, 200, 30));
        QVERIFY(linkColorsClose(shows(loaded.get(), 4), QColor(255, 255, 255)));
    }

    // What a mail merge picture frame shows for a record, at the frame's left half.
    static QColor mergeShows(jp::Document *d, int record)
    {
        jp::LayoutCache cache;
        jp::PaintContext ctx;
        ctx.doc = d;
        ctx.cache = &cache;
        ctx.opt.output = true;
        ctx.opt.mergeRecord = record;
        return QColor(jp::Renderer::renderToImage(ctx, 0, 1.0).pixel(150, 180));
    }

    // Merge pictures outside the publication's folder wait for the user: a
    // publication that is opened again shows none of them until the recipient
    // list is chosen again (Use an Existing List, Select from Contacts), and
    // then those in the list's folder and below, for that session only. What
    // the user chose is never saved, and a file cannot say it.
    void mergePicturesFollowTheListChosenThisSession()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir root(dir.path());
        QVERIFY(root.mkpath(QStringLiteral("pub")) && root.mkpath(QStringLiteral("lists/faces")) && root.mkpath(QStringLiteral("elsewhere")));
        linkPicture(root.filePath(QStringLiteral("lists/faces/ann.png")), QSize(200, 160), QColor(200, 30, 30));
        linkPicture(root.filePath(QStringLiteral("elsewhere/bob.png")), QSize(200, 160), QColor(30, 30, 200));
        const QString list = root.filePath(QStringLiteral("lists/people.csv"));
        {
            QFile f(list);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("Name,Photo\nAnn,faces/ann.png\nBob,../../elsewhere/bob.png\n");
        }
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto frame = std::make_shared<jp::PictureItem>();
        frame->rect = QRectF(100, 100, 200, 160);
        frame->imgRect = QRectF(0, 0, 200, 160);
        frame->name = QStringLiteral("merge:Photo");
        doc->pages[0]->items.push_back(frame);
        QString err;
        QVERIFY2(jp::loadMergeSource(list, &doc->merge, &err), qPrintable(err));
        QVERIFY(linkColorsClose(mergeShows(doc.get(), 0), QColor(200, 30, 30)));   // made in this session: shows
        const QString path = root.filePath(QStringLiteral("pub/book.jpub"));
        QVERIFY(jp::savePublication(*doc, path, QImage(), &err));

        auto loaded = jp::loadPublication(path, &err);
        QVERIFY2(loaded, qPrintable(err));
        QCOMPARE(loaded->merge.rows.size(), 2);
        QVERIFY(loaded->allowedFolders.isEmpty());
        QVERIFY(linkColorsClose(mergeShows(loaded.get(), 0), QColor(255, 255, 255)));
        QVERIFY(linkColorsClose(mergeShows(loaded.get(), 1), QColor(255, 255, 255)));

        // A file that says which folders to allow doesn't get them.
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(linkFileBytes(path), entries));
        QJsonObject json = QJsonDocument::fromJson(entries["document.json"]).object();
        json["allowedFolders"] = QJsonArray{root.filePath(QStringLiteral("lists")), root.path(), QStringLiteral("/")};
        json["merge"] = [&] { QJsonObject m = json["merge"].toObject(); m["allowedFolders"] = QJsonArray{root.path()}; return m; }();
        entries["document.json"] = QJsonDocument(json).toJson(QJsonDocument::Compact);
        jp::ZipWriter crafted;
        crafted.add(QStringLiteral("mimetype"), entries.take(QStringLiteral("mimetype")));
        for (auto it = entries.cbegin(); it != entries.cend(); ++it) crafted.add(it.key(), it.value());
        auto sneaky = jp::publicationFromBytes(crafted.finish(), &err, root.filePath(QStringLiteral("pub")));
        QVERIFY2(sneaky, qPrintable(err));
        QVERIFY(sneaky->allowedFolders.isEmpty());
        QVERIFY(linkColorsClose(mergeShows(sneaky.get(), 0), QColor(255, 255, 255)));

        // Opened in a window, still nothing; the list chosen again brings its folder's pictures.
        jp::MainWindow w;
        w.editor()->setDocument(std::move(loaded), path);
        QVERIFY(linkColorsClose(mergeShows(w.editor()->doc(), 0), QColor(255, 255, 255)));
        jp::MergeSource again;
        QVERIFY2(jp::loadMergeSource(list, &again, &err), qPrintable(err));
        w.editor()->selectRecipients(again);
        jp::Document *d = w.editor()->doc();
        QCOMPARE(d->allowedFolders, QStringList{QFileInfo(list).absolutePath()});
        QVERIFY(linkColorsClose(mergeShows(d, 0), QColor(200, 30, 30)));
        QVERIFY(linkColorsClose(mergeShows(d, 1), QColor(255, 255, 255)));   // outside the list's folder too
        w.editor()->undo();   // the list goes back, the folder stays allowed for the session
        QVERIFY(linkColorsClose(mergeShows(d, 0), QColor(200, 30, 30)));

        // Saved, nothing of it is in the file; opened again, it is asked again.
        QVERIFY(w.saveTo(path));
        QMap<QString, QByteArray> saved;
        QVERIFY(jp::readZip(linkFileBytes(path), saved));
        QVERIFY(!saved["document.json"].contains("allowed"));
        QVERIFY(!saved["document.json"].contains(QFileInfo(list).absolutePath().toUtf8() + "\"]"));
        auto reopened = jp::loadPublication(path, &err);
        QVERIFY2(reopened, qPrintable(err));
        QVERIFY(reopened->allowedFolders.isEmpty());
        QVERIFY(linkColorsClose(mergeShows(reopened.get(), 0), QColor(255, 255, 255)));
        // A copy made in memory (Merge to a new publication) keeps what this session allowed.
        auto copy = jp::mergeToNewPublication(*d);
        QVERIFY(copy);
        QCOMPARE(copy->allowedFolders, d->allowedFolders);
    }

    // A recovered copy is opened as if in the folder of the file it was made
    // from: linked pictures in that folder load as usual; the others do not.
    void recoveredPublicationFollowsLinksInItsOriginalFolder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir root(dir.path());
        QVERIFY(root.mkpath(QStringLiteral("pub/sub")) && root.mkpath(QStringLiteral("outside")));
        jp::recovery::setRoot(root.filePath(QStringLiteral("AutoRecover")));
        const QSize size(900, 700);
        const QString inside = linkPicture(root.filePath(QStringLiteral("pub/sub/in.png")), size, QColor(200, 30, 30));
        const QString outside = linkPicture(root.filePath(QStringLiteral("outside/out.png")), size, QColor(30, 30, 200));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QStringList ids;
        for (const QString &file : {inside, outside}) {
            auto item = std::make_shared<jp::PictureItem>();
            item->rect = QRectF(50, 50 + 200 * ids.size(), 160, 120);
            item->imgRect = QRectF(0, 0, 160, 120);
            item->imageId = doc->addLinkedImage(linkFileBytes(file), QStringLiteral("png"), file, false);
            ids << item->imageId;
            doc->pages[0]->items.push_back(item);
        }
        const QString original = root.filePath(QStringLiteral("pub/book.jpub"));
        const QString lost = root.filePath(QStringLiteral("AutoRecover/lost"));
        QVERIFY(root.mkpath(QStringLiteral("AutoRecover/lost")));
        const QString copy = lost + QStringLiteral("/Book 0123abcd.jpub");
        QVERIFY(jp::saveRecoveryCopy(*doc, copy, original, QStringLiteral("Book")));
        for (const QString &f : {inside, outside}) linkPicture(f, size, QColor(250, 250, 10));   // changed since: what is read shows

        // The copy keeps the links relative to the original folder, as the file does
        // (opening it discards the copy, so it is read first).
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(linkFileBytes(copy), entries));
        QString relative;
        for (const auto &v : QJsonDocument::fromJson(entries["document.json"]).object()["images"].toArray())
            if (v.toObject()["id"].toString() == ids[0]) relative = v.toObject()["relative"].toString();
        QCOMPARE(relative, QStringLiteral("sub/in.png"));

        jp::MainWindow w;
        const jp::recovery::Recovered found{copy, original, QStringLiteral("Book"), QDateTime::currentDateTime()};
        QCOMPARE(w.openRecovered(found), &w);
        jp::Document *d = w.editor()->doc();
        QCOMPARE(d->linkStatus(ids[0]), jp::LinkStatus::Modified);   // in the original folder: read, found changed
        QCOMPARE(d->images[ids[0]].bytes, linkFileBytes(inside));
        QCOMPARE(d->images[ids[0]].sourcePath, QFileInfo(inside).absoluteFilePath());
        QCOMPARE(d->linkStatus(ids[1]), jp::LinkStatus::NotUpdated);
        QVERIFY(d->images[ids[1]].bytes != linkFileBytes(outside));
        QCOMPARE(d->folder, QFileInfo(original).absolutePath());

        // Work that was never saved has no original folder: no link is followed.
        auto fresh = jp::Document::blank(QSizeF(612, 792));
        auto item = std::make_shared<jp::PictureItem>();
        item->rect = QRectF(50, 50, 160, 120);
        item->imgRect = QRectF(0, 0, 160, 120);
        item->imageId = fresh->addLinkedImage(linkFileBytes(inside), QStringLiteral("png"), inside, false);
        fresh->pages[0]->items.push_back(item);
        QVERIFY(root.mkpath(QStringLiteral("AutoRecover/lost2")));
        const QString copy2 = root.filePath(QStringLiteral("AutoRecover/lost2/Publication2 4567abcd.jpub"));
        QVERIFY(jp::saveRecoveryCopy(*fresh, copy2, QString(), QStringLiteral("Publication2")));
        jp::MainWindow v;
        QCOMPARE(v.openRecovered(jp::recovery::Recovered{copy2, QString(), QStringLiteral("Publication2"), QDateTime::currentDateTime()}), &v);
        QCOMPARE(v.editor()->doc()->linkStatus(item->imageId), jp::LinkStatus::NotUpdated);
        jp::recovery::endSession();
        jp::recovery::setRoot(QString());
    }

    // Pack and Go for another computer says so when the ZIP can't be saved,
    // instead of reporting that it did.
    void packToZipReportsAFailedSave()
    {
        jp::MainWindow w;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString err;
        int fonts = -1;
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("taken.zip")));   // a folder where the file must go
        QVERIFY(!jp::packToZip(&w, dir.filePath(QStringLiteral("taken.zip")), &fonts, &err));
        QVERIFY2(err.contains(QStringLiteral("taken.zip")), qPrintable(err));
        err.clear();
        QVERIFY(!jp::packToZip(&w, dir.filePath(QStringLiteral("no/such/folder/pack.zip")), nullptr, &err));
        QVERIFY2(!err.isEmpty(), qPrintable(err));
        QVERIFY(!QFile::exists(dir.filePath(QStringLiteral("no"))));
        err.clear();
        const QString good = dir.filePath(QStringLiteral("pack.zip"));
        QVERIFY(jp::packToZip(&w, good, &fonts, &err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QVERIFY(fonts >= 0);
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(linkFileBytes(good), entries));
        QVERIFY(entries.contains(w.editor()->displayName() + QStringLiteral(".jpub")));
    }

    // A linked picture copied from one publication into one that came from a
    // file, whose file is outside that publication's folder, arrives without
    // the link: the file is not looked at.
    void pastedOutsideLinkComesWithoutTheLink()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir root(dir.path());
        QVERIFY(root.mkpath(QStringLiteral("pub")) && root.mkpath(QStringLiteral("outside")));
        const QString file = linkPicture(root.filePath(QStringLiteral("outside/photo.png")), QSize(300, 200), QColor(200, 30, 30));
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.insertFiles({file}, QPointF(-1, -1), jp::PictureInsert::Link);
        w.editor()->select(linkFirstPicture(w.editor()->doc())->id);
        w.editor()->copy();

        QString err;
        const QString path = root.filePath(QStringLiteral("pub/book.jpub"));
        QVERIFY(jp::savePublication(*jp::Document::blank(QSizeF(612, 792)), path, QImage(), &err));
        auto opened = jp::loadPublication(path, &err);
        QVERIFY2(opened, qPrintable(err));
        jp::MainWindow v;
        v.editor()->setDocument(std::move(opened), path);
        v.editor()->paste();
        jp::PictureItem *pic = linkFirstPicture(v.editor()->doc());
        QVERIFY(pic);
        QVERIFY(!v.editor()->doc()->images[pic->imageId].linked);
        QCOMPARE(v.editor()->doc()->image(pic->imageId).size(), QSize(300, 200));

        // In the publication that has the file in its folder, it stays a link.
        w.editor()->paste();
        int linked = 0;
        for (const auto &it : w.editor()->doc()->pages[0]->items)
            if (auto *p = dynamic_cast<jp::PictureItem *>(it.get())) linked += w.editor()->doc()->images[p->imageId].linked;
        QCOMPARE(linked, 2);
    }

    // Embedding for e-mail, Pack and Go, Save as Template, and .pub files
    // writes what the publication holds: a link not followed brings its
    // preview, and the outside file is not read for it.
    void embeddingNeverReadsAnOutsideLink()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir root(dir.path());
        QVERIFY(root.mkpath(QStringLiteral("pub")) && root.mkpath(QStringLiteral("outside")));
        const QString file = linkPicture(root.filePath(QStringLiteral("outside/photo.png")), QSize(1000, 800), QColor(200, 30, 30));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto item = std::make_shared<jp::PictureItem>();
        item->rect = QRectF(100, 100, 250, 200);
        item->imgRect = QRectF(0, 0, 250, 200);
        item->imageId = doc->addLinkedImage(linkFileBytes(file), QStringLiteral("png"), file, false);
        const QString id = item->imageId;
        doc->pages[0]->items.push_back(item);
        QString err;
        const QString path = root.filePath(QStringLiteral("pub/book.jpub"));
        QVERIFY(jp::savePublication(*doc, path, QImage(), &err));
        linkPicture(file, QSize(1000, 800), QColor(250, 250, 10));
        auto loaded = jp::loadPublication(path, &err);
        QVERIFY2(loaded, qPrintable(err));
        QCOMPARE(loaded->linkStatus(id), jp::LinkStatus::NotUpdated);

        auto there = jp::publicationFromBytes(jp::publicationBytes(*loaded, QImage(), QString(), true), &err);
        QVERIFY2(there, qPrintable(err));
        QCOMPARE(std::max(there->image(id).width(), there->image(id).height()), 512);   // the preview, not the 1000-pixel file
        QCOMPARE(there->imageSize(id), QSize(1000, 800));
        const QString pub = root.filePath(QStringLiteral("embedded.pub"));
        QVERIFY2(jp::exportPublisher(*loaded, pub, &err), qPrintable(err));
        auto back = jp::importPublisherFile(pub, &err);
        QVERIFY2(back, qPrintable(err));
        for (const auto &it : back->pages[0]->items)
            if (auto *p = dynamic_cast<jp::PictureItem *>(it.get())) QCOMPARE(std::max(back->image(p->imageId).width(), back->image(p->imageId).height()), 512);
    }

    // Extra Content in a damaged file: a text box with no story to hold its
    // text, a table whose cell has none, are dropped when the file loads; a
    // shape without text stays. (Found by a crafted file.)
    void danglingExtraContentIsDroppedOnLoad()
    {
        QJsonObject json = jp::Document::blank(QSizeF(612, 792))->toJson();
        json["extra"] = QJsonArray{
            QJsonObject{{"id", "x1"}, {"type", "text"}, {"rect", QJsonArray{0, 0, 100, 50}}, {"story", "no-such-story"}},
            QJsonObject{{"id", "x2"}, {"type", "shape"}, {"rect", QJsonArray{0, 0, 100, 50}}, {"shape", "rect"}},
            QJsonObject{{"id", "x3"}, {"type", "table"}, {"rect", QJsonArray{0, 0, 100, 50}}, {"rows", 1}, {"cols", 1}, {"colW", QJsonArray{100}},
                        {"rowH", QJsonArray{50}}, {"cells", QJsonArray{QJsonObject{{"story", "nope"}}}}},
            QJsonObject{{"id", "x4"}, {"type", "shape"}, {"rect", QJsonArray{0, 0, 100, 50}}, {"shape", "rect"}, {"story", "gone"}}};
        jp::Document doc;
        doc.fromJson(json);
        QCOMPARE(int(doc.extra.size()), 1);
        QCOMPARE(doc.extra[0]->id, QStringLiteral("x2"));

        // One that has its story is kept.
        auto good = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(0, 0, 100, 50);
        box->storyId = good->createStory(QStringLiteral("Kept"));
        good->extra.push_back(box);
        jp::Document back;
        back.fromJson(good->toJson());
        QCOMPARE(int(back.extra.size()), 1);
    }

    // The Extra Content pane lists whatever is there, with words from the
    // story when it has one: an object whose story is missing (it can't come
    // from a file, but nothing may crash on it) is listed by its kind.
    void extraContentPaneSurvivesMissingStories()
    {
        jp::MainWindow w;
        w.show();
        jp::Document *d = w.editor()->doc();
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(0, 0, 100, 50);
        box->storyId = QStringLiteral("ghost");
        auto shape = std::make_shared<jp::ShapeItem>();
        shape->rect = QRectF(0, 0, 100, 50);
        shape->storyId = QStringLiteral("ghost");
        auto table = std::make_shared<jp::TableItem>();
        table->rows = table->cols = 1;
        table->colW = {100};
        table->rowH = {50};
        table->cells.resize(1);
        table->cells[0].storyId = QStringLiteral("ghost");
        auto plain = std::make_shared<jp::ShapeItem>();
        plain->rect = QRectF(0, 0, 100, 50);
        d->extra = {box, shape, table, plain};
        w.showTaskPane(QStringLiteral("extra"));
        auto *list = w.findChild<jp::TaskPane *>()->findChild<QListWidget *>(QStringLiteral("extraContentList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 4);
    }

    // A pane that is not showing does nothing when the publication changes:
    // the Extra Content pane rescales every picture in it each time it
    // refreshes, so hidden it must not refresh at all.
    void hiddenExtraContentPaneDoesNotRefresh()
    {
        jp::MainWindow w;
        w.show();
        jp::Editor *ed = w.editor();
        QImage big(1600, 1200, QImage::Format_RGB32);
        big.fill(Qt::blue);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        big.save(&buffer, "PNG");
        for (int i = 0; i < 3; ++i) {
            auto pic = std::make_shared<jp::PictureItem>();
            pic->rect = QRectF(0, 0, 100, 80);
            pic->imageId = ed->doc()->addImage(png, QStringLiteral("png"));
            ed->doc()->extra.push_back(pic);
        }
        w.showTaskPane(QStringLiteral("extra"));
        auto *list = w.findChild<jp::TaskPane *>()->findChild<QListWidget *>(QStringLiteral("extraContentList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 3);
        w.showTaskPane(QStringLiteral("graphics"));   // another pane in front: this one is hidden
        QVERIFY(!list->isVisible());
        QSignalSpy rebuilt(list->model(), &QAbstractItemModel::rowsInserted);
        for (int i = 0; i < 4; ++i) {
            ed->change(QStringLiteral("Test"), [&] {
                auto box = ed->newTextBox(QRectF(50, 50 + 20 * i, 100, 40));
                ed->surfaceItems().push_back(box);
            });
            w.refreshUi();
        }
        QCOMPARE(rebuilt.count(), 0);
        // Showing it brings it up to date again, and a change while it shows refreshes it.
        ed->doc()->extra.pop_back();
        w.showTaskPane(QStringLiteral("extra"));
        QCOMPARE(list->count(), 2);
        ed->doc()->extra.pop_back();
        ed->change(QStringLiteral("Test"), [&] {});
        w.refreshUi();
        QCOMPARE(list->count(), 1);
    }

    // The year calendar's twelve pictures: made in parallel, compressed (they
    // were stored unpacked, 27 MB a calendar), and the same each time.
    void calendarPictures()
    {
        const jp::TemplateInfo *info = jp::findTemplate(QStringLiteral("calendar-year"));
        QVERIFY(info);
        jp::TemplateOptions o;
        o.colorScheme = QStringLiteral("Harbor");   // not the calendar's own colors, so likely made fresh here
        auto first = info->build(o);
        auto again = info->build(o);   // from the session's cache
        qint64 total = 0;
        QStringList a, b;
        for (const auto &img : first->images) {
            total += img.bytes.size();
            a << QString::fromLatin1(QCryptographicHash::hash(img.bytes, QCryptographicHash::Sha1).toHex());
            QVERIFY(!img.cache.isNull() && img.cache.size() == img.pixelSize);
            QCOMPARE(img.cache, img.image());   // as decoded from the file
        }
        for (const auto &img : again->images) b << QString::fromLatin1(QCryptographicHash::hash(img.bytes, QCryptographicHash::Sha1).toHex());
        QCOMPARE(int(first->images.size()), 12);
        QVERIFY2(total < 4 * 1024 * 1024, qPrintable(QString::number(total)));
        a.sort();
        b.sort();
        QCOMPARE(a, b);
        QImage decoded;
        {
            jp::ImageData copy = first->images.first();
            copy.cache = QImage();
            decoded = copy.image();
        }
        QCOMPARE(decoded, first->images.first().cache);
    }

    // A story through three linked boxes of the same height is drawn in
    // all three (paint clipped each paragraph in frame-local units while its
    // lines sit below the earlier boxes: the third box drew nothing).
    void linkedBoxesAllDrawn()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QVector<std::shared_ptr<jp::TextItem>> boxes;
        const QString story = doc->createStory(QStringLiteral("A line of text to fill the boxes.\n").repeated(24).trimmed());
        for (int i = 0; i < 3; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 72 + i * 220, 300, 160);
            t->storyId = story;
            if (i) boxes.last()->nextId = t->id;
            boxes << t;
            doc->pages[0]->items.push_back(t);
        }
        w.editor()->setDocument(std::move(doc));
        const QImage img = w.pageThumbnail(0, 792);
        jp::LayoutCache cache;
        for (int i = 0; i < 3; ++i) {
            const auto fl = cache.textFrame(*w.editor()->doc(), *boxes[i], 1, jp::RenderOptions());
            const auto lines = fl.layout->lineRects(i);
            QVERIFY(lines.size() >= 5);
            for (const QRectF &r : lines) {
                int ink = 0;
                for (int y = int(boxes[i]->rect.top() + r.top()); y < int(boxes[i]->rect.top() + r.bottom()); ++y)
                    for (int x = 75; x < 372; ++x) ink += qGray(img.pixel(x, y)) < 128;
                QVERIFY2(ink > 50, qPrintable(QStringLiteral("box %1, line at %2: no text drawn").arg(i + 1).arg(r.top())));
            }
        }
    }

    // Insert > Icons: the picker searches names and tags; an icon becomes
    // one editable artwork shape in the scheme's main color whose line
    // weight scales with it; an icon with filled parts is a group; inserting
    // several is one undo step; edited points follow any shape's resize; and
    // icons survive saving as .jpub and as .pub.
    void iconsInsert()
    {
        using namespace jp;
        QVERIFY(iconCatalog().size() > 1500);

        ItemPtr it = iconItem(QStringLiteral("heart"), QPointF(100, 100), 72, ColorRef::scheme(Main));
        QVERIFY(it && it->type() == ItemType::Shape);
        auto *heart = static_cast<ShapeItem *>(it.get());
        QVERIFY(heart->isArt());
        QCOMPARE(heart->rect, QRectF(64, 64, 72, 72));
        QCOMPARE(heart->stroke.width, 6.0);                       // 2 of 24 units, at an inch
        QCOMPARE(heart->stroke.color, ColorRef::scheme(Main));
        QCOMPARE(heart->stroke.cap, Qt::RoundCap);
        QCOMPARE(heart->stroke.join, Qt::RoundJoin);
        QVERIFY(heart->fill.type == Fill::NoFill);
        QCOMPARE(heart->altText, QStringLiteral("heart"));
        const QRectF pathBox = heart->customPath.boundingRect();
        QVERIFY(QRectF(0, 0, 72, 72).contains(pathBox) && pathBox.width() > 55);
        heart->scaleInto(heart->rect, QRectF(64, 64, 144, 144));
        QCOMPARE(heart->stroke.width, 12.0);
        QVERIFY(std::abs(heart->customPath.boundingRect().width() - 2 * pathBox.width()) < 1e-6);

        // A shape with edited points stretches with its frame, its line kept.
        ShapeItem edited;
        edited.rect = QRectF(0, 0, 100, 50);
        edited.customPath.addRect(QRectF(0, 0, 100, 50));
        edited.scaleInto(edited.rect, QRectF(0, 0, 200, 100));
        QCOMPARE(edited.customPath.boundingRect(), QRectF(0, 0, 200, 100));
        QCOMPARE(edited.stroke.width, 1.0);

        ItemPtr tag = iconItem(QStringLiteral("tag"), QPointF(300, 300), 72, ColorRef::scheme(Main));
        QVERIFY(tag && tag->type() == ItemType::Group);
        const auto &parts = static_cast<GroupItem *>(tag.get())->children;
        QCOMPARE(parts.size(), size_t(2));
        QVERIFY(static_cast<ShapeItem *>(parts[1].get())->fill.type == Fill::Solid);   // the dot is filled
        QCOMPARE(static_cast<ShapeItem *>(parts[1].get())->fill.color, ColorRef::scheme(Main));

        // The picker: words match the starts of names and tags.
        MainWindow w;
        w.editor()->setDocument(Document::blank(QSizeF(612, 792)));
        Editor *ed = w.editor();
        QStringList picked, love, arrows;
        QTimer::singleShot(0, &w, [&] {
            // Checked after the dialog closes, so a failure can't leave it open.
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            auto *search = dlg->findChild<QLineEdit *>();
            auto *list = dlg->findChild<QListWidget *>(QStringLiteral("iconList"));
            auto shown = [&] {
                QStringList v;
                for (int i = 0; i < list->count(); ++i)
                    if (!list->item(i)->isHidden()) v << list->item(i)->data(Qt::UserRole).toString();
                return v;
            };
            if (search && list) {
                search->setText(QStringLiteral("love"));             // a tag of "heart"
                love = shown();
                search->setText(QStringLiteral("arrow-big-down"));   // hyphens count as spaces
                arrows = shown();
                for (int i = 0, n = 0; i < list->count() && n < 2; ++i)
                    if (!list->item(i)->isHidden()) { list->item(i)->setSelected(true); ++n; }
            }
            dlg->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        picked = pickIcons(&w);
        QVERIFY(love.contains(QStringLiteral("heart")));
        QVERIFY(love.size() < 60);
        QVERIFY(arrows.contains(QStringLiteral("arrow-big-down")) && arrows.contains(QStringLiteral("arrow-big-down-dash")));
        QVERIFY(!arrows.contains(QStringLiteral("arrow-big-up")));
        QCOMPARE(picked, arrows.mid(0, 2));
        insertIcons(ed, picked);
        QCOMPARE(ed->doc()->pages[0]->items.size(), size_t(2));
        QCOMPARE(ed->selectedItems().size(), 2);
        const QRectF r0 = ed->doc()->pages[0]->items[0]->rect, r1 = ed->doc()->pages[0]->items[1]->rect;
        QCOMPARE(r0.size(), QSizeF(72, 72));
        QCOMPARE(r0.center().y(), 396.0);
        QCOMPARE(r0.center().x() + r1.center().x(), 612.0);         // side by side across the middle
        ed->undo();
        QCOMPARE(ed->doc()->pages[0]->items.size(), size_t(0));
        ed->redo();
        QCOMPARE(ed->doc()->pages[0]->items.size(), size_t(2));

        // Saving: .jpub keeps the artwork exactly; .pub keeps a drawn shape.
        ed->doc()->pages[0]->items.push_back(it);
        QTemporaryDir dir;
        QString err;
        const QString jpub = dir.filePath(QStringLiteral("icons.jpub"));
        QVERIFY2(savePublication(*ed->doc(), jpub, QImage(), &err), qPrintable(err));
        auto back = loadPublication(jpub, &err);
        QVERIFY2(back, qPrintable(err));
        auto *again = static_cast<ShapeItem *>(back->pages[0]->items[2].get());
        QVERIFY(again->isArt());
        QCOMPARE(again->stroke.width, 12.0);
        QCOMPARE(again->customPath.elementCount(), heart->customPath.elementCount());
        QCOMPARE(again->customPath.fillRule(), Qt::WindingFill);
        const QString pub = dir.filePath(QStringLiteral("icons.pub"));
        QVERIFY2(exportPublisher(*ed->doc(), pub, &err), qPrintable(err));
        auto fromPub = importPublisherFile(pub, &err);
        QVERIFY2(fromPub, qPrintable(err));
        QCOMPARE(fromPub->pages[0]->items.size(), size_t(3));
        auto *pubHeart = dynamic_cast<ShapeItem *>(fromPub->pages[0]->items[2].get());
        QVERIFY(pubHeart && !pubHeart->customPath.isEmpty());
        QVERIFY(std::abs(pubHeart->stroke.width - 12) < 0.1);
        QVERIFY(std::abs(pubHeart->customPath.boundingRect().width() - heart->customPath.boundingRect().width()) < 0.5);
    }

    // Linked boxes keep their order when the file lists them in another
    // (here the second box is in front of the first): the chain follows each
    // box's place in it (0x28), not the order the boxes are drawn.
    void linkedBoxOrderFromPub()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto first = std::make_shared<TextItem>(), second = std::make_shared<TextItem>(), third = std::make_shared<TextItem>();
        first->rect = QRectF(36, 72, 160, 300);
        second->rect = QRectF(216, 72, 160, 300);
        third->rect = QRectF(396, 72, 160, 300);
        QString text;
        for (int i = 1; i <= 120; ++i) text += QStringLiteral("Sentence %1 of a story that runs through three boxes. ").arg(i);
        first->storyId = doc->createStory(QStringLiteral("START ") + text);
        second->storyId = third->storyId = first->storyId;
        first->nextId = second->id;
        second->nextId = third->id;
        // Drawn back to front: third, first, second.
        doc->pages[0]->items = {third, first, second};
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("order.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QVector<TextItem *> boxes;
        for (const auto &it : back->pages[0]->items)
            if (auto *t = dynamic_cast<TextItem *>(it.get())) boxes << t;
        QCOMPARE(boxes.size(), 3);
        auto at = [&](double x) { for (TextItem *t : boxes) if (std::abs(t->rect.x() - x) < 1) return t; return static_cast<TextItem *>(nullptr); };
        TextItem *a = at(36), *b = at(216), *c = at(396);
        QVERIFY(a && b && c);
        QVERIFY(!back->prevFrame(a->id));          // the left box starts the story
        QCOMPARE(a->nextId, b->id);
        QCOMPARE(b->nextId, c->id);
        QVERIFY(c->nextId.isEmpty());
        LayoutCache cache;
        RenderOptions opt;
        const auto fl = cache.textFrame(*back, *a, 1, opt);
        QVERIFY(fl.layout);
        const auto lines = fl.layout->lineInfo(fl.frame);
        QVERIFY(!lines.isEmpty());
        QVERIFY(lines.first().text.startsWith(QStringLiteral("START")));
    }

    // The eyedropper picks the color under the click from the window.
    void eyedropperPicks()
    {
        QWidget win;
        win.resize(200, 100);
        win.setAutoFillBackground(true);
        QPalette pal = win.palette();
        pal.setColor(QPalette::Window, QColor(200, 30, 40));
        win.setPalette(pal);
        win.show();
        QVERIFY(QTest::qWaitForWindowExposed(&win));
        QColor got;
        jp::pickColorFromWindow(&win, [&](const QColor &c) { got = c; });
        QWidget *overlay = nullptr;
        for (QWidget *c : win.findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) overlay = c;
        QVERIFY(overlay);
        QTest::mouseClick(overlay, Qt::LeftButton, {}, QPoint(50, 50));
        QCOMPARE(got, QColor(200, 30, 40));
    }

    // Catalog merge: the first cell of the catalog area repeats for each
    // record, a pageful at a time; objects outside the area stay on every page.
    void catalogMerge()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->merge.fields = {QStringLiteral("Name"), QStringLiteral("Price")};
        const QStringList names = {"Apples", "Bread", "Cheese", "Dates", "Eggs"};
        for (int i = 0; i < names.size(); ++i) doc->merge.rows << QStringList{names[i], QString::number(i + 1)};
        doc->catalog.pageId = doc->pages[0]->id;
        doc->catalog.rect = QRectF(36, 72, 540, 684);
        doc->catalog.rows = 2;
        doc->catalog.cols = 2;
        QCOMPARE(doc->catalog.perPage(), 4);
        QCOMPARE(doc->catalog.cell(3), QRectF(306, 414, 270, 342));
        auto title = std::make_shared<jp::TextItem>();
        title->rect = QRectF(36, 20, 540, 40);
        title->storyId = doc->createStory(QStringLiteral("Our catalog"));
        doc->pages[0]->items.push_back(title);
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(48, 84, 200, 30);
        box->storyId = doc->createStory();
        {
            QTextCursor c(doc->storyDoc(box->storyId));
            QTextCharFormat cf;
            cf.setProperty(jp::tp::Field, QStringLiteral("merge:Name"));
            c.insertText(QString(QChar::ObjectReplacementCharacter), cf);
            c.insertText(QStringLiteral(" $"), QTextCharFormat());
            cf.setProperty(jp::tp::Field, QStringLiteral("merge:Price"));
            c.insertText(QString(QChar::ObjectReplacementCharacter), cf);
        }
        doc->pages[0]->items.push_back(box);
        QVERIFY(doc->catalog.inTemplate(box->bounds()));
        QVERIFY(!doc->catalog.inTemplate(title->bounds()));

        // The area survives saving.
        QString err;
        auto again = jp::publicationFromBytes(jp::publicationBytes(*doc, QImage()), &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(again->catalog.pageId, doc->catalog.pageId);
        QCOMPARE(again->catalog.rect, doc->catalog.rect);
        QCOMPARE(again->catalog.cols, 2);

        auto merged = jp::mergeToNewPublication(*doc);
        QCOMPARE(merged->pages.size(), 2);
        QVERIFY(!merged->catalog.isActive());
        auto texts = [&](int page) {
            QMap<QString, QPointF> out;
            for (const auto &it : merged->pages[page]->items)
                if (it->type() == jp::ItemType::Text)
                    out.insert(merged->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId)->toPlainText(), it->rect.topLeft());
            return out;
        };
        const auto p1 = texts(0), p2 = texts(1);
        QCOMPARE(p1.size(), 5);
        QCOMPARE(p2.size(), 2);
        QVERIFY(p1.contains("Our catalog") && p2.contains("Our catalog"));
        QCOMPARE(p1.value("Apples $1"), QPointF(48, 84));
        QCOMPARE(p1.value("Bread $2"), QPointF(318, 84));
        QCOMPARE(p1.value("Cheese $3"), QPointF(48, 426));
        QCOMPARE(p1.value("Dates $4"), QPointF(318, 426));
        QCOMPARE(p2.value("Eggs $5"), QPointF(48, 84));

        // Previewing a record paints the next records in the other cells.
        jp::PaintContext ctx;
        ctx.doc = doc.get();
        jp::LayoutCache cache;
        ctx.cache = &cache;
        ctx.opt.output = true;
        auto inkIn = [&](int record, const QRectF &r) {
            ctx.opt.mergeRecord = record;
            QImage img(612, 792, QImage::Format_RGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            jp::Renderer::paintPage(&p, ctx, 0);
            p.end();
            int dark = 0;
            for (int y = int(r.top()); y < int(r.bottom()); ++y)
                for (int x = int(r.left()); x < int(r.right()); ++x) dark += qGray(img.pixel(x, y)) < 128;
            return dark;
        };
        const QRectF cell4(306, 414, 270, 60);
        QVERIFY(inkIn(0, cell4) > 20);      // Dates in the fourth cell
        QCOMPARE(inkIn(-1, cell4), 0);       // unmerged output prints only the template
        QCOMPARE(inkIn(4, QRectF(306, 72, 270, 60)), 0);   // Eggs is the last: nothing after it

        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            jp::MainWindow w;
            w.resize(1400, 900);
            w.editor()->setDocument(std::move(doc));
            w.show();
            QVERIFY(QTest::qWaitForWindowExposed(&w));
            w.showTaskPane(QStringLiteral("catalog"));
            QTest::qWait(200);
            w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/catalog-design.png");
            w.editor()->setMergeRecord(0);
            QTest::qWait(200);
            w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/catalog-preview.png");
        }
    }
};

JP_TEST_CLASS(PubPictureTests)
#include "tst_pub_pictures.moc"
