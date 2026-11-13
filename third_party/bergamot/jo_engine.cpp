// The implementation of libs/translate/engine/engine.h: Mozilla's Bergamot translator, behind a standard-C++ interface. This file is compiled in the engine's build
// (third_party/bergamot/CMakeLists.txt, with the engine's flags), not in the program's.

#include "engine/engine.h"

#include "translator/parser.h"
#include "translator/response.h"
#include "translator/response_options.h"
#include "translator/service.h"
#include "translator/translation_model.h"

#include <cstring>
#include <future>
#include <map>
#include <mutex>

namespace jo {
namespace translate {
namespace engine {

namespace mb = marian::bergamot;

struct Engine::Impl {
    explicit Impl(int threads) : service(makeConfig(threads)), count(size_t(threads < 1 ? 1 : threads)) {}
    static mb::AsyncService::Config makeConfig(int threads)
    {
        mb::AsyncService::Config c;
        c.numWorkers = size_t(threads < 1 ? 1 : threads);
        c.logger.level = "off";
        return c;
    }
    mb::AsyncService service;
    std::mutex lock;
    std::map<int, std::shared_ptr<mb::TranslationModel>> models;
    int next = 1;
    size_t count;
    size_t workers() const { return count; }
};

static mb::AlignedMemory copyAligned(const std::string &from, size_t alignment)
{
    mb::AlignedMemory m(from.size(), alignment);
    if (!from.empty()) std::memcpy(m.begin(), from.data(), from.size());
    return m;
}

Engine::Engine(int threads) : m_impl(new Impl(threads)) {}

Engine::~Engine()
{
    m_impl->service.clear();
}

int Engine::load(ModelBytes &bytes, std::string *error)
{
    try {
        if (bytes.model.empty() || bytes.vocabs.empty() || bytes.vocabs.size() > 2) {
            if (error) *error = "the model files are incomplete";
            return 0;
        }
        // (the model's own settings are what Mozilla's Firefox runs them with; the file names here stand in, the bytes are in memory)
        std::string config =
            "models:\n  - model.bin\n"
            "vocabs:\n  - vocab.spm\n  - vocab.spm\n"
            "beam-size: 1\nnormalize: 1.0\nword-penalty: 0\nmax-length-break: 128\nmini-batch-words: 1024\nworkspace: 128\nmax-length-factor: 2.0\n"
            "skip-cost: true\ncpu-threads: 0\nquiet: true\nquiet-translation: true\ngemm-precision: int8shiftAlphaAll\nalignment: soft\n";
        if (!bytes.shortlist.empty()) config += "shortlist:\n  - lex.bin\n  - false\n";
        mb::MemoryBundle memory;
        memory.models.push_back(copyAligned(bytes.model, 256));
        if (!bytes.shortlist.empty()) memory.shortlist = copyAligned(bytes.shortlist, 64);
        auto first = std::make_shared<mb::AlignedMemory>(copyAligned(bytes.vocabs.front(), 64));
        memory.vocabs.push_back(first);
        memory.vocabs.push_back(bytes.vocabs.size() == 2 ? std::make_shared<mb::AlignedMemory>(copyAligned(bytes.vocabs.back(), 64)) : first);
        bytes = ModelBytes();
        auto model = std::make_shared<mb::TranslationModel>(config, std::move(memory), m_impl->workers());
        std::lock_guard<std::mutex> g(m_impl->lock);
        const int h = m_impl->next++;
        m_impl->models[h] = model;
        return h;
    } catch (const std::exception &e) {
        if (error) *error = e.what();
    } catch (...) {
        if (error) *error = "the model could not be loaded";
    }
    return 0;
}

void Engine::unload(int handle)
{
    std::lock_guard<std::mutex> g(m_impl->lock);
    m_impl->models.erase(handle);
}

bool Engine::translatePieces(int handle, const std::vector<std::string> &pieces, bool html, std::vector<std::string> *out, std::string *error, const std::atomic<bool> *cancel)
{
    std::shared_ptr<mb::TranslationModel> model;
    {
        std::lock_guard<std::mutex> g(m_impl->lock);
        auto it = m_impl->models.find(handle);
        if (it != m_impl->models.end()) model = it->second;
    }
    if (!model) {
        if (error) *error = "the model is not loaded";
        return false;
    }
    out->clear();
    try {
        mb::ResponseOptions options;
        options.HTML = html;
        // Several pieces at once, so the engine can batch short ones together (a piece alone is a poor batch); a cancel is looked at before each is handed over.
        const size_t window = m_impl->workers() * 8;
        // (the promises are shared with the callbacks: when this returns early, by an exception or a cancel, what is still queued finds them alive)
        std::vector<std::shared_ptr<std::promise<std::string>>> promises;
        std::vector<std::future<std::string>> futures;
        for (size_t i = 0; i < pieces.size(); ++i) {
            promises.push_back(std::make_shared<std::promise<std::string>>());
            futures.push_back(promises.back()->get_future());
        }
        size_t submitted = 0;
        for (size_t done = 0; done < pieces.size(); ++done) {
            while (submitted < pieces.size() && submitted < done + window && !(cancel && cancel->load())) {
                auto p = promises[submitted];
                std::string piece = pieces[submitted];
                if (piece.empty()) p->set_value(std::string());
                else m_impl->service.translate(model, std::move(piece), [p](mb::Response &&r) { p->set_value(std::move(r.target.text)); }, options);
                ++submitted;
            }
            if (done >= submitted) return false;   // canceled before this one was handed over
            out->push_back(futures[done].get());
        }
    } catch (const std::exception &e) {
        if (error) *error = e.what();
        return false;
    } catch (...) {
        if (error) *error = "the translation failed";
        return false;
    }
    return true;
}

} // namespace engine
} // namespace translate
} // namespace jo
