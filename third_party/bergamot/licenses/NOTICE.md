# The translation engine and its parts

The Translate pane (JeffPub and JeffWrite) translates with Mozilla's Bergamot translator, built into the program as static libraries (`cmake/bergamot_step.cmake`, `third_party/bergamot`). Its source is not in this repository: the build fetches the commit named in `cmake/bergamot_step.cmake` of https://github.com/browsermt/bergamot-translator (with its submodules) and applies the patches in `third_party/bergamot/patches` to a copy. What each patch changes is written at the top of its file.

| Part | License | Text |
|---|---|---|
| bergamot-translator (the engine) | MPL-2.0 | `bergamot-translator-MPL-2.0.txt`, and MPL-2.0.txt in the licenses folder |
| Marian (the neural network code) | MIT | `marian-MIT.txt` |
| intgemm (8-bit matrix multiplication, x86) | MIT | `intgemm-MIT.txt` |
| SentencePiece (word pieces) | Apache-2.0 | Apache-2.0.txt in the licenses folder |
| ruy (matrix multiplication) | Apache-2.0 | Apache-2.0.txt in the licenses folder |
| cpuinfo (what the processor can do, used by ruy) | BSD-2-Clause | `cpuinfo-BSD-2-Clause.txt` |
| simd_utils (vector math on ARM) | BSD-2-Clause | `simd_utils-BSD-2-Clause.txt` |
| Inside SentencePiece, compiled in (the check of what is compiled is the list of its sources in `src/CMakeLists.txt` and the headers they include): protobuf-lite (protocol buffers, lite) | BSD-3-Clause | `sentencepiece-protobuf-lite.txt` |
| Inside SentencePiece: absl (a few of Abseil's header-only strings, containers, and flags) | Apache-2.0 | `sentencepiece-absl.txt` |
| Inside SentencePiece: darts_clone (a trie), esaxx (suffix arrays, for training, which is linked) | BSD-3-Clause, MIT | `sentencepiece-darts_clone.txt`, `sentencepiece-esaxx.txt` |
| ssplit-cpp (splits text into sentences) | Apache-2.0 | `ssplit-cpp.txt` |
| PCRE2 (regular expressions, used by ssplit-cpp) | BSD-3-Clause | `pcre2-BSD.txt` |
| Libraries inside Marian: CLI11, cnpy, faiss, mio, pathie-cpp, phf, spdlog, yaml-cpp, zstr | MIT, BSD, and similar | `marian-<name>.txt` |

The files of nonbreaking word prefixes that ssplit-cpp comes with (LGPL-2.1, read at run time, not compiled in) are not used and not shipped.

Parts are copyright their authors; the texts above name them. Bergamot and Marian are copyright the University of Edinburgh, Mozilla, and the other contributors named in their repositories, which hold the notices.

The translation models are not part of the program: they are downloaded when a person asks for them, from the addresses in `resources/translate/models.json`. They are Mozilla's Firefox Translations models, which Mozilla publishes under the MPL-2.0 (https://github.com/mozilla/translations). The list names each file's size and SHA-256, and a file that does not match is not used.
