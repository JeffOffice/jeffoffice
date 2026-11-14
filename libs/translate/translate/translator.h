#pragma once
// Translation of pieces of text between two languages with the models in the store: one model for a pair that has one, two (through English) for one that has not.
// What does the translating is a Backend: the engine built in (BergamotBackend), or, in the tests, a stand-in that needs no model.

#include "core/export.h"
#include "catalog.h"
#include "modelstore.h"

#include <QObject>
#include <QStringList>
#include <atomic>
#include <memory>

namespace jp {
namespace translate {

// The most letters one translation takes in all (a request over it is refused with a message that says so).
constexpr int kMostRequestChars = 1000000;

class Backend {
public:
    virtual ~Backend();
    // Translates `pieces` (paragraphs: HTML when `html`) one language direction, with the models of `pair`, which are installed in `store`. False with `error` set when it cannot, or with
    // `error` empty when `cancel` was set (what is done is in `out`).
    virtual bool translatePieces(const ModelPair &pair, const ModelStore &store, const QStringList &pieces, bool html, QStringList *out, QString *error, const std::atomic<bool> *cancel) = 0;
};

// The engine built into the program, or null when the program was built without it.
std::shared_ptr<Backend> builtInBackend();
// The end of the program (it is connected to aboutToQuit, and a test may call it): every translation under way is canceled, none is waited for, and the engine is let go on a thread of its
// own and waited for a second at most. Nothing in the library waits for a thread when the program or the library ends.
void shutdownTranslation();
void releaseBuiltInBackend();

class Translator {
public:
    Translator(const Catalog &catalog, const ModelStore &store, std::shared_ptr<Backend> backend) : m_catalog(catalog), m_store(store), m_backend(std::move(backend)) {}

    bool hasEngine() const { return m_backend != nullptr; }
    // Whether the catalog has a way from one language to the other.
    bool canTranslate(const QString &from, const QString &to) const { return m_catalog.canTranslate(from, to); }
    // The pairs of the way that are not installed (empty when the translation can be done now, or when there is no way).
    QVector<ModelPair> missing(const QString &from, const QString &to) const { return m_store.missing(m_catalog.route(from, to)); }
    qint64 downloadBytes(const QString &from, const QString &to) const;

    // Translates the pieces; false, with `error` set (or empty after a cancel), when it did not complete.
    bool translatePieces(const QString &from, const QString &to, const QStringList &pieces, bool html, QStringList *out, QString *error, const std::atomic<bool> *cancel = nullptr) const;

    const Catalog &catalog() const { return m_catalog; }
    const ModelStore &store() const { return m_store; }

private:
    Catalog m_catalog;
    ModelStore m_store;
    std::shared_ptr<Backend> m_backend;
};

// One translation, run on a thread of the program's thread pool: `finished` comes on the thread of the one that made it. `cancel` stops it between pieces. Letting go
// of the job (closing the window it was for) never waits for the thread: the translation stops at the next piece, in the pool, which is joined only when the program ends, and
// nobody is told of its end.
class JO_COMMON_EXPORT TranslateJob : public QObject {
    Q_OBJECT
public:
    TranslateJob(const Translator &translator, const QString &from, const QString &to, const QStringList &pieces, bool html, QObject *parent = nullptr);
    ~TranslateJob() override;
    void start();
    void cancel() { m_state->cancel = true; }
    bool isRunning() const { return m_state->running; }

Q_SIGNALS:
    void finished(bool ok, bool canceled, const QStringList &out, const QString &error);

private:
    Translator m_translator;
    QString m_from, m_to;
    QStringList m_pieces;
    bool m_html;
    struct State {
        std::atomic<bool> cancel{false}, running{false};
    };
    std::shared_ptr<State> m_state;
};

} // namespace translate
} // namespace jp
