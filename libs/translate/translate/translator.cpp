#include "translator.h"
#include "languages.h"
#include "runmap.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QPointer>
#include <QLocale>
#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

namespace jp {
namespace translate {

Backend::~Backend() = default;

qint64 Translator::downloadBytes(const QString &from, const QString &to) const
{
    qint64 n = 0;
    for (const ModelPair &p : missing(from, to)) n += p.totalBytes();
    return n;
}

namespace {

// The jobs under way, to cancel at the end of the program (they are on threads of their own that nothing joins).
std::mutex &jobsLock() { static std::mutex *m = new std::mutex; return *m; }
std::vector<std::weak_ptr<std::atomic<bool>>> &jobsRunning() { static auto *v = new std::vector<std::weak_ptr<std::atomic<bool>>>; return *v; }
std::atomic<bool> &shuttingDown() { static auto *a = new std::atomic<bool>(false); return *a; }

} // namespace

void shutdownTranslation()
{
    shuttingDown() = true;
    {
        std::lock_guard<std::mutex> g(jobsLock());
        for (const auto &w : jobsRunning())
            if (auto c = w.lock()) *c = true;
    }
    releaseBuiltInBackend();
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
    qint64 total = 0;
    for (const QString &p : pieces) total += p.size();
    if (total > kMostRequestChars) {
        if (error) *error = QCoreApplication::translate("jp::translate", "That is too much text to translate at once: %1 letters. The most is %2. Select less of it.").arg(QLocale().toString(total), QLocale().toString(kMostRequestChars));
        return false;
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
    // A paragraph goes to the engine in chunks of bounded size, so no one piece of work is long and a cancel is looked at between them.
    QStringList chunks, trailing;
    QVector<int> owner;
    if (html) {
        for (int i = 0; i < pieces.size(); ++i)
            for (const Chunk &c : splitForTranslation(pieces[i])) {
                chunks << c.html;
                trailing << c.trailing;
                owner << i;
            }
    } else {
        for (int i = 0; i < pieces.size(); ++i) {
            chunks << pieces[i];
            trailing << QString();
            owner << i;
        }
    }
    QStringList current = chunks;
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
        if (cancel && cancel->load()) return false;
    }
    QStringList joined;
    for (int i = 0; i < pieces.size(); ++i) joined << QString();
    for (int k = 0; k < current.size(); ++k) joined[owner[k]] += current[k] + (current[k].isEmpty() ? QString() : trailing[k]);
    *out = joined;
    return true;
}

TranslateJob::TranslateJob(const Translator &translator, const QString &from, const QString &to, const QStringList &pieces, bool html, QObject *parent)
    : QObject(parent), m_translator(translator), m_from(from), m_to(to), m_pieces(pieces), m_html(html), m_state(std::make_shared<State>())
{
}

TranslateJob::~TranslateJob() { m_state->cancel = true; }

void TranslateJob::start()
{
    if (m_state->running) return;
    m_state->running = true;
    QPointer<TranslateJob> self(this);
    // (the work holds what it needs by itself: the job may be gone before it ends)
    const std::shared_ptr<State> state = m_state;
    const Translator translator = m_translator;
    const QString from = m_from, to = m_to;
    const QStringList pieces = m_pieces;
    const bool html = m_html;
    if (shuttingDown()) {
        m_state->running = false;
        return;
    }
    {
        std::lock_guard<std::mutex> g(jobsLock());
        auto &v = jobsRunning();
        v.erase(std::remove_if(v.begin(), v.end(), [](const std::weak_ptr<std::atomic<bool>> &w) { return w.expired(); }), v.end());
        v.push_back(std::shared_ptr<std::atomic<bool>>(state, &state->cancel));
    }
    // A thread of its own that nothing joins (a pool would be waited for when the program ends, and the engine cannot be told to leave a sentence).
    std::thread([self, state, translator, from, to, pieces, html] {
        QStringList out;
        QString error;
        const bool ok = translator.translatePieces(from, to, pieces, html, &out, &error, &state->cancel);
        const bool canceled = !ok && error.isEmpty();
        state->running = false;
        // (told on the main thread, by way of the application, which outlives the job; a job that is gone, or a program that is ending, is not told)
        if (QCoreApplication *app = QCoreApplication::instance())
            QMetaObject::invokeMethod(app, [self, ok, canceled, out, error] { if (self) Q_EMIT self->finished(ok, canceled, out, error); }, Qt::QueuedConnection);
    }).detach();
}

} // namespace translate
} // namespace jp
