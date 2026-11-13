#include "translator.h"
#include "languages.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QPointer>
#include <QThread>

namespace jp {
namespace translate {

Backend::~Backend() = default;

qint64 Translator::downloadBytes(const QString &from, const QString &to) const
{
    qint64 n = 0;
    for (const ModelPair &p : missing(from, to)) n += p.totalBytes();
    return n;
}

bool Translator::translatePieces(const QString &from, const QString &to, const QStringList &pieces, bool html, QStringList *out, QString *error, const std::atomic<bool> *cancel) const
{
    out->clear();
    if (error) error->clear();
    if (!m_backend) {
        if (error) *error = QCoreApplication::translate("jp::translate", "This copy of the program was built without the translator.");
        return false;
    }
    if (from == to) {
        *out = pieces;
        return true;
    }
    const QVector<ModelPair> route = m_catalog.route(from, to);
    if (route.isEmpty()) {
        if (error) *error = QCoreApplication::translate("jp::translate", "There is no model for translating from %1 to %2.").arg(languageName(from), languageName(to));
        return false;
    }
    const QVector<ModelPair> missing = m_store.missing(route);
    if (!missing.isEmpty()) {
        if (error) *error = QCoreApplication::translate("jp::translate", "The model for translating from %1 to %2 is not installed.").arg(languageName(missing.first().from), languageName(missing.first().to));
        return false;
    }
    QStringList current = pieces;
    for (const ModelPair &hop : route) {
        QStringList next;
        QString err;
        const bool ok = m_backend->translatePieces(hop, m_store, current, html, &next, &err, cancel);
        if (!ok) {
            if (error) *error = err;
            return false;
        }
        if (next.size() != current.size()) {
            if (error) *error = QCoreApplication::translate("jp::translate", "The translator gave back the wrong number of paragraphs.");
            return false;
        }
        current = next;
        if (cancel && cancel->load()) {
            return false;
        }
    }
    *out = current;
    return true;
}

TranslateJob::TranslateJob(const Translator &translator, const QString &from, const QString &to, const QStringList &pieces, bool html, QObject *parent)
    : QObject(parent), m_translator(translator), m_from(from), m_to(to), m_pieces(pieces), m_html(html)
{
}

TranslateJob::~TranslateJob()
{
    m_cancel = true;
    if (m_thread) {
        m_thread->wait();
        delete m_thread;
    }
}

bool TranslateJob::isRunning() const { return m_thread && m_thread->isRunning(); }

void TranslateJob::start()
{
    if (m_thread) return;
    QPointer<TranslateJob> self(this);
    m_thread = QThread::create([self, this] {
        QStringList out;
        QString error;
        const bool ok = m_translator.translatePieces(m_from, m_to, m_pieces, m_html, &out, &error, &m_cancel);
        const bool canceled = !ok && error.isEmpty();
        if (self) QMetaObject::invokeMethod(self.data(), [self, ok, canceled, out, error] { if (self) Q_EMIT self->finished(ok, canceled, out, error); }, Qt::QueuedConnection);
    });
    m_thread->start();
}

} // namespace translate
} // namespace jp
