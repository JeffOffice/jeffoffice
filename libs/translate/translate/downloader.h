#pragma once
// Downloads the files of language pairs, one after the other, into the store: with progress, a cancel, and a check of each file's size and SHA-256 before it is kept.
// Nothing is used until the whole pair is there and every file has been checked; a file that does not match is thrown away and the download fails (the pair is not
// installed). Only what the catalog names is fetched, and only from the addresses in it.

#include "catalog.h"
#include "modelstore.h"

#include <QObject>
#include <QPointer>
#include <QVector>

class QNetworkAccessManager;
class QNetworkReply;
class QFile;
class QCryptographicHash;

namespace jp {
namespace translate {

class Downloader : public QObject {
    Q_OBJECT
public:
    // `net` is the program's network manager (or a test's); the downloader does not own it.
    Downloader(const ModelStore &store, QNetworkAccessManager *net, QObject *parent = nullptr);
    ~Downloader() override;

    // Fetches every one of `pairs` that is not installed. `finished` comes once, when all are installed, or the first failed, or `cancel` was called.
    void start(const QVector<ModelPair> &pairs);
    void cancel();
    bool isRunning() const { return m_running; }
    qint64 totalBytes() const { return m_total; }

Q_SIGNALS:
    // Bytes of the files kept and the one coming, of all of them.
    void progress(qint64 done, qint64 total);
    // `error` is empty when the pairs are installed; `canceled`: it ended because of cancel (the error is empty too).
    void finished(bool ok, bool canceled, const QString &error);

private:
    void next();
    void finish(bool ok, bool canceled, const QString &error);
    void readChunk();
    void replyDone();

    ModelStore m_store;
    QPointer<QNetworkAccessManager> m_net;
    QVector<ModelPair> m_pairs;
    int m_pair = 0, m_file = 0;
    qint64 m_total = 0, m_doneBefore = 0;
    bool m_running = false;
    QNetworkReply *m_reply = nullptr;
    QFile *m_out = nullptr;
    QCryptographicHash *m_hash = nullptr;
    qint64 m_got = 0;
};

} // namespace translate
} // namespace jp
