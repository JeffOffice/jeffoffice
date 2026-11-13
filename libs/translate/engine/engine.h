#pragma once
// The translation engine's own interface, in standard C++ only: no Qt, and none of the engine's headers. The file that implements it (engine.cpp) is compiled
// inside the engine's build (third_party/bergamot), with the engine's own flags, so what the engine's headers mean is the same in the engine and in this file.
// Everything else in the program reaches the engine through this header (and the Qt layer over it, translate/enginehost.h).

#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace jo {
namespace translate {
namespace engine {

// One language direction's files, read into memory by the caller (so a folder name in any language opens, as the engine's own file opening is not ready for those on Windows).
struct ModelBytes {
    std::string model;                 // the weights (model.*.intgemm.alphas.bin)
    std::string shortlist;             // the lexical shortlist (lex.*.s2t.bin); may be empty
    std::vector<std::string> vocabs;   // the SentencePiece model: one for both languages, or the source's and then the target's
};

class Engine {
public:
    // `threads` workers translate at once (1 or 2 are used).
    explicit Engine(int threads);
    ~Engine();
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;

    // Builds a language direction from its bytes (which are left empty: the engine takes them); a handle above 0, or 0 and `error`.
    int load(ModelBytes &bytes, std::string *error);
    void unload(int handle);

    // Translates the pieces of text one after the other (a piece is a paragraph; the sentences in it are split by the engine). `html`: the pieces are HTML, whose tags stay where the
    // words they enclose went. `cancel` is looked at between pieces: when it is set, what is done so far is returned with false and `error` empty.
    bool translatePieces(int handle, const std::vector<std::string> &pieces, bool html, std::vector<std::string> *out, std::string *error, const std::atomic<bool> *cancel = nullptr);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace engine
} // namespace translate
} // namespace jo
