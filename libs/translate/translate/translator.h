#pragma once
// Translation of pieces of text between two languages with the models in the store: one model for a pair that has one, two (through English) for one that has not.
// What does the translating is a Backend: the engine built in (BergamotBackend), or, in the tests, a stand-in that needs no model.

#include "catalog.h"
#include "modelstore.h"

#include <QObject>
#include <QStringList>
#include <atomic>
#include <memory>

class QThread;

namespace jp {
namespace translate {

class Backend {
public:
    virtual ~Backend();
    // Translates `pieces` (paragraphs: HTML when `html`) one language direction, with the models of `pair`, which are installed in `store`. False with `error` set when it cannot, or with
    // `error` empty when `cancel` was set (what is done is in `out`).
    virtual bool translatePieces(const ModelPair &pair, const ModelStore &store, const QStringList &pieces, bool html, QStringList *out, QString *error, const std::atomic<bool> *cancel) = 0;
};

// The engine built into the program, or null when the program was built without it.
std::shared_ptr<Backend> builtInBackend();

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

// One translation, run on a thread of its own: `finished` comes on the thread of the one that made it. `cancel` stops it between pieces.
class TranslateJob : public QObject {
    Q_OBJECT
public:
    TranslateJob(const Translator &translator, const QString &from, const QString &to, const QStringList &pieces, bool html, QObject *parent = nullptr);
    ~TranslateJob() override;
    void start();
    void cancel() { m_cancel = true; }
    bool isRunning() const;

Q_SIGNALS:
    void finished(bool ok, bool canceled, const QStringList &out, const QString &error);

private:
    Translator m_translator;
    QString m_from, m_to;
    QStringList m_pieces;
    bool m_html;
    std::atomic<bool> m_cancel{false};
    QThread *m_thread = nullptr;
};

} // namespace translate
} // namespace jp
