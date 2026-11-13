// The engine built into the program (third_party/bergamot): loads the models a pair names from the store, keeps the last few in memory, and translates with them.
// Without the engine (the CMake option JO_TRANSLATE off), builtInBackend() is null.

#include "translator.h"

#ifdef JO_TRANSLATE_ENGINE
#include "engine/engine.h"

#include <QCoreApplication>
#include <QFile>
#include <chrono>
#include <QThread>
#include <list>
#include <map>
#include <mutex>
#endif

namespace jp {
namespace translate {

#ifdef JO_TRANSLATE_ENGINE

namespace {

class BergamotBackend : public Backend {
public:
    BergamotBackend() : m_engine(std::min(2, std::max(1, QThread::idealThreadCount() / 2))) {}
    ~BergamotBackend() override = default;

    bool translatePieces(const ModelPair &pair, const ModelStore &store, const QStringList &pieces, bool html, QStringList *out, QString *error, const std::atomic<bool> *cancel) override
    {
        // (another translation may be using the engine: wait for it, but a cancel is heard while waiting)
        std::unique_lock<std::timed_mutex> lock(m_lock, std::defer_lock);
        while (!lock.try_lock_for(std::chrono::milliseconds(10)))
            if (cancel && cancel->load()) return false;
        const int handle = load(pair, store, error, cancel);
        if (!handle) return false;
        std::vector<std::string> in, got;
        for (const QString &p : pieces) in.push_back(p.toUtf8().toStdString());
        std::string err;
        const bool ok = m_engine.translatePieces(handle, in, html, &got, &err, cancel);
        for (const std::string &g : got) out->push_back(QString::fromUtf8(g.data(), int(g.size())));
        if (!ok && !err.empty() && error) *error = QString::fromStdString(err);
        return ok;
    }

private:
    static bool read(const QString &path, std::string *into)
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return false;
        const QByteArray b = f.readAll();
        into->assign(b.constData(), size_t(b.size()));
        return !into->empty();
    }

    // 0 with `error` empty when `cancel` was set (between one file and the next).
    int load(const ModelPair &pair, const ModelStore &store, QString *error, const std::atomic<bool> *cancel)
    {
        const QString key = pair.key();
        auto it = m_loaded.find(key);
        if (it != m_loaded.end()) {
            m_order.remove(key);
            m_order.push_front(key);
            return it->second;
        }
        jo::translate::engine::ModelBytes bytes;
        bool ok = read(store.filePath(pair, QStringLiteral("model")), &bytes.model);
        if (cancel && cancel->load()) return 0;
        const QString shortlist = store.filePath(pair, QStringLiteral("shortlist"));
        if (!shortlist.isEmpty()) ok = ok && read(shortlist, &bytes.shortlist);
        for (const char *role : {"vocab", "srcvocab", "trgvocab"}) {
            const QString path = store.filePath(pair, QString::fromLatin1(role));
            if (path.isEmpty()) continue;
            std::string v;
            ok = ok && read(path, &v);
            if (cancel && cancel->load()) return 0;
            bytes.vocabs.push_back(std::move(v));
        }
        if (!ok) {
            if (error) *error = QCoreApplication::translate("jp::translate", "The translation files could not be read.");
            return 0;
        }
        std::string err;
        const int handle = m_engine.load(bytes, &err);
        if (!handle) {
            if (error) *error = QString::fromStdString(err);
            return 0;
        }
        m_loaded[key] = handle;
        m_order.push_front(key);
        // (a model takes room: the last four stay, which is two pairs' worth of a translation through English)
        while (m_order.size() > 4) {
            const QString old = m_order.back();
            m_order.pop_back();
            m_engine.unload(m_loaded[old]);
            m_loaded.erase(old);
        }
        return handle;
    }

    std::timed_mutex m_lock;
    jo::translate::engine::Engine m_engine;
    std::map<QString, int> m_loaded;
    std::list<QString> m_order;
};

} // namespace

std::shared_ptr<Backend> builtInBackend()
{
    static std::shared_ptr<Backend> backend = std::make_shared<BergamotBackend>();
    return backend;
}

#else

std::shared_ptr<Backend> builtInBackend() { return nullptr; }

#endif

} // namespace translate
} // namespace jp
