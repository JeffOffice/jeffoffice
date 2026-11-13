#include "downloader.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace jp {
namespace translate {

Downloader::Downloader(const ModelStore &store, QNetworkAccessManager *net, QObject *parent) : QObject(parent), m_store(store), m_net(net) {}

Downloader::~Downloader()
{
    if (m_reply) {
        m_reply->disconnect(this);
        m_reply->abort();
        m_reply->deleteLater();
    }
    delete m_out;
    delete m_hash;
}

void Downloader::start(const QVector<ModelPair> &pairs)
{
    if (m_running) return;
    m_pairs.clear();
    m_total = 0;
    for (const ModelPair &p : pairs)
        if (!m_store.isInstalled(p)) {
            m_pairs.push_back(p);
            m_total += p.totalBytes();
        }
    m_pair = m_file = 0;
    m_doneBefore = 0;
    m_running = true;
    if (m_pairs.isEmpty()) {
        finish(true, false, QString());
        return;
    }
    for (const ModelPair &p : m_pairs) {
        m_store.remove(p.from, p.to);   // (a half-made folder of an earlier try)
        QDir().mkpath(m_store.partFolder(p.from, p.to));
    }
    Q_EMIT progress(0, m_total);
    next();
}

void Downloader::cancel()
{
    if (!m_running) return;
    if (m_reply) {
        m_reply->disconnect(this);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    finish(false, true, QString());
}

void Downloader::finish(bool ok, bool canceled, const QString &error)
{
    delete m_out;
    m_out = nullptr;
    delete m_hash;
    m_hash = nullptr;
    if (!ok)
        for (const ModelPair &p : m_pairs) QDir(m_store.partFolder(p.from, p.to)).removeRecursively();
    m_running = false;
    Q_EMIT finished(ok, canceled, error);
}

void Downloader::next()
{
    if (m_pair >= m_pairs.size()) {
        finish(true, false, QString());
        return;
    }
    const ModelPair &p = m_pairs[m_pair];
    if (m_file >= p.files.size()) {
        if (!m_store.commit(p)) {
            finish(false, false, QCoreApplication::translate("jp::translate", "The translation files could not be saved."));
            return;
        }
        ++m_pair;
        m_file = 0;
        next();
        return;
    }
    if (!m_net) {
        finish(false, false, QCoreApplication::translate("jp::translate", "The program cannot reach the network."));
        return;
    }
    const ModelFile &f = p.files[m_file];
    m_out = new QFile(m_store.partFolder(p.from, p.to) + QLatin1Char('/') + f.fileName);
    if (!m_out->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        finish(false, false, QCoreApplication::translate("jp::translate", "The translation files could not be saved."));
        return;
    }
    m_hash = new QCryptographicHash(QCryptographicHash::Sha256);
    m_got = 0;
    QNetworkRequest req(f.url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("JeffOffice"));
    req.setTransferTimeout(60000);
    m_reply = m_net->get(req);
    connect(m_reply, &QNetworkReply::readyRead, this, &Downloader::readChunk);
    connect(m_reply, &QNetworkReply::finished, this, &Downloader::replyDone);
}

void Downloader::readChunk()
{
    if (!m_reply || !m_out) return;
    const ModelFile &f = m_pairs[m_pair].files[m_file];
    for (;;) {
        const QByteArray chunk = m_reply->read(256 * 1024);
        if (chunk.isEmpty()) break;
        m_got += chunk.size();
        // (a server that sends more than the catalog says is not sending that file)
        if (m_got > f.size) {
            QNetworkReply *r = m_reply;
            m_reply = nullptr;
            r->disconnect(this);
            r->abort();
            r->deleteLater();
            finish(false, false, QCoreApplication::translate("jp::translate", "A translation file is larger than it should be, and was refused."));
            return;
        }
        m_out->write(chunk);
        m_hash->addData(chunk);
    }
    Q_EMIT progress(m_doneBefore + m_got, m_total);
}

void Downloader::replyDone()
{
    QNetworkReply *r = m_reply;
    if (!r) return;
    readChunk();
    if (!m_reply) return;   // (readChunk refused it)
    m_reply = nullptr;
    const QNetworkReply::NetworkError err = r->error();
    const QString text = r->errorString();
    r->deleteLater();
    const ModelFile f = m_pairs[m_pair].files[m_file];
    if (err != QNetworkReply::NoError) {
        finish(false, err == QNetworkReply::OperationCanceledError ? true : false, err == QNetworkReply::OperationCanceledError ? QString() : QCoreApplication::translate("jp::translate", "The download failed: %1").arg(text));
        return;
    }
    m_out->close();
    const QString got = QString::fromLatin1(m_hash->result().toHex());
    delete m_out;
    m_out = nullptr;
    delete m_hash;
    m_hash = nullptr;
    const ModelPair &p = m_pairs[m_pair];
    if (m_got != f.size || got != f.sha256) {
        QFile::remove(m_store.partFolder(p.from, p.to) + QLatin1Char('/') + f.fileName);
        finish(false, false, QCoreApplication::translate("jp::translate", "A translation file did not match its checksum, and was thrown away. Nothing was installed."));
        return;
    }
    m_doneBefore += f.size;
    ++m_file;
    Q_EMIT progress(m_doneBefore, m_total);
    next();
}

} // namespace translate
} // namespace jp
