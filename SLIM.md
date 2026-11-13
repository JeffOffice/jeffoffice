# Slimming the suite (round 39)

Linux x86-64, GCC, Qt 6.11.3, Release build (`-O3`) on the build machine. Sizes are bytes of the files on disk. "Stripped" is
`strip` of a copy. Startup is the median of 5 runs of a fresh process (a first, warm-up run dropped), offscreen, a new empty
HOME each run, no file: the program's own marks ("first paint", "window usable") in milliseconds since launch, with
`JP_STARTUP_TRACE=1`, `JP_STARTUP_T0` set to the launch time, and `JP_GRAB` to make it quit.

## What changed

1. `jeffoffice-common`: the seven layers (jo_base, jo_draw, jo_text, jo_office, jo_shell, jo_page, jo_flow) are OBJECT libraries
   whose objects are linked into ONE shared library, `libjeffoffice-common.so/.dll/.dylib`, that all four apps and every test
   program link. One library, not seven: the layers call each other freely downward, so splitting them would add six more files
   to every package and a load order to get right, for no saving. hunspell, zlib, qpdf, and the other third-party static
   libraries are linked into it privately (Linux hides their symbols with `--exclude-libs,ALL`); PDFium was already a shared
   library and stays one. jpcore/jdcore/jwcore/jccore stay static. Linux links with `--no-undefined` (so a call upward fails
   to link, as the per-layer link checks always did) and `-Bsymbolic-functions`/`-fno-semantic-interposition` (the library's
   own calls are direct). Windows exports every symbol (`-Wl,--export-all-symbols`); the Linux library has 11,409 exported
   symbols, far under the DLL limit of 65,535. RPATH: `$ORIGIN`, `$ORIGIN/../lib`, `$ORIGIN/../lib/jeffoffice` (Linux);
   `@executable_path/../Frameworks` (macOS); beside the program (Windows). The qrc stays inside the library (its
   initializer runs when the library loads; `Q_INIT_RESOURCE` in the apps still works). No `Q_GLOBAL_STATIC` exists.
2. Help and the translations are files, not qrc entries: `jp::dataFolder("help")` and `jp::dataFolder("i18n")`
   (libs/base/core/datafolder.h) search beside the program, `../share/jeffoffice`, `../Resources`, then the source tree
   (`resources/help`) and the build tree (`build/data/i18n`). Help, the language list, and the translators use it. With no
   data folder an app starts in English and Help shows "Help is not installed". Test: `dataFolderHoldsHelpAndTranslations`.
   `tools/help_files.py` is now a no-op (kept because the translation scripts call it). Icons, ribbon, and font metrics stay in the qrc.
3. Packaging: the library and the data folder go in once per install (Windows installer and zip, .deb, AppImages, snap), and
   inside each of the four macOS bundles (each stays whole by itself; `tools/link_duplicates.py` already stores identical files
   once in the disk image, so what the .dmg gains depends on how many copies are byte-identical after macdeployqt).
4. `JO_SLIM_RELEASE` (default ON, Release builds only): `-flto` (`check_ipo_supported`), `-ffunction-sections -fdata-sections`
   with `--gc-sections` (`-dead_strip` on macOS), applied to the shared library and its layers ONLY, not to the four programs (they are
   4 to 7 MB; and Qt's Windows entry point does not resolve `qMain` against an LTO program: the Windows VM link failed on it).
   Windows (MinGW) gets no LTO at all, only the section flags and `--gc-sections`: its ld did not run the LTO plugin on our objects and the DLL
   came out empty. CI turns it off in the test-only builds and in pull requests.

## Before and after

Before = main 32dde24f4 (every layer a static library, all of Help and the translations in resources.qrc). After = this branch,
Release with `JO_SLIM_RELEASE=ON`. "Without LTO" is the same branch with it OFF.

| | Before | After, no LTO | After (slim) |
|---|---|---|---|
| JeffPub exe, stripped | 43,450,880 | 4,343,464 | 4,298,408 |
| JeffDeck exe, stripped | 45,756,496 | 6,536,096 | 6,478,752 |
| JeffWrite exe, stripped | 49,207,280 | 8,002,560 | 7,932,928 |
| JeffCrunch exe, stripped | 48,559,344 | 8,398,032 | 8,316,112 |
| libjeffoffice-common.so, stripped | | 28,587,496 | 24,540,592 |
| Data folder (help 15,132,971 + i18n 18,910,762) | inside the exes | 34,043,733 | 34,043,733 |
| **Install total, stripped** | **186,974,000** | **89,911,381** | **85,671,733** (library-only LTO, after the review fixes; 81,139,541 with LTO on the programs too) |
| Unstripped exes (Pub, Deck, Write, Crunch) | 47,500,416 / 49,971,176 / 53,896,336 / 53,308,400 | 5,343,664 / 7,715,544 / 9,478,440 / 10,030,280 | 5,047,696 / 6,828,864 / 8,381,816 / 8,591,744 |
| Unstripped library | | 32,473,288 | 26,905,056 |

Install total without LTO: 27,280,152 (exes) + 28,587,496 + 34,043,733 = 89,911,381. The same files as zip -9 (a stand-in for the
Windows zip, which also holds Qt and the fonts): before 84,932,716, after 31,209,836 (the data is plain text and compresses
well, which the uncompressed install size hides). The Windows 0.27.1 zip was 353 MB, of which about 190 MB was four copies of
the code and Help; it should drop by about 50 MB zipped (all of Help and the translations were already compressed in the exes).
Fonts, dictionaries, PDFium, Qt, and FFmpeg are unchanged and not counted.

macOS: each of the four bundles holds its own copy of the library (about 23 MB stripped for x86-64, about double for a universal build) and of
the data (34 MB, which was compressed inside the executables before), plus its own executable (4 to 7 MB). That is about 62 to 68 MB per bundle
for x86-64 against 43 to 49 MB before, so the installed size on macOS goes UP by about a third (4 x 57 MB of shared files against 4 x about 35 MB
of the same code and compressed Help). The .dmg should shrink anyway, because `tools/link_duplicates.py` stores byte-identical files once and
the library and data are identical across bundles unless macdeployqt makes them differ (not verified). If you want macOS smaller on disk too, the
alternative is one shared folder with symlinks, which breaks "one app copied alone is whole" (the lead's call).

### Startup (median of 5, interleaved A/B on a machine at load average 50, so differences under 10% are noise)

| App | before: first paint / usable | after: first paint / usable |
|---|---|---|
| JeffPub | 104.0 / 222.5 ms | 101.0 / 215.1 ms |
| JeffDeck | 110.1 / 222.0 ms | 108.3 / 218.3 ms |
| JeffWrite | 88.2 / 177.7 ms | 85.8 / 208.8 ms (a noisy run: 183.6 in another) |
| JeffCrunch | 90.5 / 180.9 ms | 88.0 / 180.9 ms |

On the quiet machine at the start, the before numbers were 80 / 179 (Pub), 91 / 190 (Deck), 75 / 154 (Write), 74 / 153 ms (Crunch). Three
interleaved runs of before and after on the loaded machine agreed except for one noisy run each; the dynamic loader costs under 1 ms more (`LD_DEBUG=statistics`).
No startup regression found. (A first comparison looked 40 ms slower: the "before" build was missing its source folder, so it had loaded no bundled fonts.)

### Build time cost (4 apps, no compiler cache, `-j24`, machine busy)

| | Time | Link of libjeffoffice-common | Link of JeffCrunch |
|---|---|---|---|
| JO_SLIM_RELEASE=OFF | 373 s | 0.9 s | 0.6 s |
| ON (LTO) | 232 s | 37.6 s | 20.2 s |

LTO compiles faster (no code generation per file) and links much slower; the 80-processor machine hides that. On a
4-processor CI runner expect the library link to take several minutes, and Windows (MinGW, big objects) and macOS more.

The data folder is now also `data.stamp` (the version) and `manifest.txt` (every Help file and translation; the topic and language lists
come from it, so a stale file from an older version is ignored). A folder is the data folder only if its stamp matches this version, one
root serves both help and i18n, and the result is found once per process. A slim (packaged) build bakes in no path of this machine; a
development build also looks in its own build folder. `tools/check_packaged_layout.sh` (run by the gate) installs the build to a temporary
prefix and checks every library resolves there, each app starts from it, and Help and a German translation load.
The numbers in the size table above for the exes and library are from the last, library-only LTO build; the startup and build-time
tables are from the first (all-targets LTO) build and were not repeated.

## What to watch on the first CI run

Nothing here was run on Windows or macOS, and the .deb, AppImage, and snap scripts were not run either.

- **Windows**: (1) the DLL's link: `-Wl,--export-all-symbols` plus LTO (if GNU ld drops symbols or hits the 65,535-export limit, turn
  LTO off first with `-DJO_SLIM_RELEASE=OFF`; the Linux count is 11,409). (2) `libjeffoffice-common.dll` in dist/JeffOffice
  (built as `libjeffoffice-common.dll`, the MinGW prefix), with Qt6Svg/Xml/Network DLLs there: windeployqt is now given the DLL as
  well, because the programs no longer import those modules. (3) `help` and `i18n` folders in dist/JeffOffice, installed by the
  installer's existing `dist\JeffOffice\*` line; the uninstaller removes them (shared.isi). (4) `Developer tools` step copies the DLL and data.
  (5) The installer-smoke job's stand-ins include the new names. (6) Qt translations: the app now loads `qtbase_xx.qm` from `i18n`.
- **macOS** (workflow_dispatch only): macdeployqt with `-executable=` the dylib; the loop checks `otool -L` of the app and the dylib for
  any path into the build folder (`install_name_tool -delete_rpath` of the build dir and PDFium's). Watch for the dylib still pointing at
  an absolute path, `codesign --deep` of the dylib, and the "alone" check now also compares `help` and `i18n`. The disk image size is the
  open question above.
- **Linux .deb and AppImages**: linuxdeploy is given the library with `--library`; the checks added to the .deb job expect
  `opt/jeffoffice/lib/libjeffoffice-common.so` exactly once, `opt/jeffoffice/share/jeffoffice/help` and `i18n`, and `ldd` of the
  apps resolving the library inside the package. linuxdeploy-plugin-qt must still bring Qt's Svg, Xml, and Network plugins (the
  apps import those modules only through the library); the PDF export and screenshot runs from the extracted .deb test it.
- **Snap**: the library goes to `usr/lib/jeffoffice` beside PDFium (found through its own `$ORIGIN` run path), and help and i18n to
  `usr/share/jeffoffice`. The strict snap cannot be run here: start each app and open Help in it. The `--no-undefined` link
  against the KDE SDK's Qt is also new.
- **Everywhere**: the Linux, Windows, and macOS "tests" jobs build with `JO_SLIM_RELEASE=OFF`; the "checks" job that packages builds with
  it ON, so its link takes longer (this is where a toolchain would break). Tests that read Help or translations read them from the
  build folder (`build/data/i18n`, `resources/help`); no test depends on the installed layout, so the packaged layout is checked only
  by the package steps.

## Not done or uncertain

- Not measured: Windows and macOS sizes, and the snap, .deb, and AppImage sizes.
- A relocation hazard: a program run from a copy of the build folder that lacks the library fails to start (as any dynamic build).
- The macOS dedupe in the disk image depends on identical bytes per app after macdeployqt (not verified).

# Dictionaries and thesaurus (round 40)

The 54 MB of spelling dictionaries, hyphenation patterns, and the thesaurus (`resources/dict`) were copied as plain files into every install
(and four times on macOS). Now the install holds each as a zlib file that the program inflates into memory when it is first used.
The repository keeps the plain files (readable diffs, licenses unchanged); the compression is a build step. Measured on the same machine
as above, offscreen, warm file cache, `jo_dictbench` (tools/dictbench.cpp; Linux only for the memory column).

## What changed

1. **Format**: zlib (deflate, level 9), which we already link (`jp_zlib`). xz, bzip2, and zstd are not in the build, and nothing is added. On the
   biggest files the alternatives would have saved little: `th_en_US_v2.dat` 18.6 MB: deflate 3.98 MB, bzip2 3.32, xz 2.18 (xz would be the one
   worth having; it is a new library, so not taken); `uk_UA.dic` 8.9 MB: 1.53 / 1.29 / 1.18. Inflating is fast: 75 ms for the whole thesaurus,
   32 ms for `uk_UA.dic`.
2. **Spelling and hyphenation** (`libs/base/text/dictpack.{h,cpp}`, `dictionaries.cpp`, `hyphenation.cpp`): `name.dic.jz` is `"JZD1"`, the plain size
   (4 bytes), and the zlib stream. `dict::readFile(path)` inflates `path.jz` when it is there, else reads the plain file (the source tree and
   older installs keep working). Hunspell had no way to read from memory (`FileMgr` opens a path with `std::ifstream`), so there is **one patch**
   (`third_party/CMakeLists.txt`, listed in `third_party/README.md`): `filemgr.hxx/.cxx` get a hook, `jeffoffice_memory_file`, and `FileMgr`
   reads its lines from memory for a name the hook claims, as 1.7.6's `fin.getline` gives them (buffer limit included). The build copies Hunspell's sources to the build folder and patches the copies (the
   downloaded tree, shared by worktrees, is never edited; a source that is not 1.7.6 stops the build). The files are inflated, handed to
   Hunspell under made-up names, and freed when it has loaded them: the memory that stays is Hunspell's own tables, as before.
3. **Thesaurus**: measured both ways. Inflating the whole 18.6 MB at first use costs 75 ms and holds 18.6 MB (plus the old 16 MB hash index
   the lookup built from it, 34.8 MB in all, 92 ms). A block format costs 1 to 2 ms to open and 1 MB: the build sorts the words, packs
   entries in blocks of about 64 KB, each its own zlib stream, with a table of every block's first word, so a lookup binary-searches the table,
   inflates one block (about 0.2 ms; the last 1 MB of blocks are kept), and finds the entry. The file is 4.93 MB (a whole-file zlib would be
   3.98 MB: sorting the words apart from WordNet's own order costs 0.95 MB). Each word's entry is kept as the file has it, so the lookup code
   (`thesaurusLookup`: base forms, parts of speech, duplicates) is unchanged and reads the same lines as before.
4. **Build**: `jo_dictpack` (tools/dictpack.cpp, Qt Core and zlib only, so it builds before everything else) makes one `.jz` per `.dic`, `.aff`,
   and `.dat` in `build/data/dict` (one ninja job each, in parallel; the plain licenses and READMEs are copied beside them as they are).
   `jeffoffice_data` builds them, so every target that links the library has them, and tests read the packed files. Each is read back and
   compared before it is written.
5. **One copy per install** (as r39's data folder): Linux (.deb, AppImages, snap, `cmake --install`) `share/jeffoffice/dict`, Windows
   `dict` beside the programs (already one copy), both through the data folder that is copied whole (`build/data`), so `dict` in the
   packaging steps is no longer copied from `resources/dict`. `jp::dict::root()` looks in the data folder's `dict` first, then in the old
   places (beside the program, `../share/jeffoffice/dict`, `../Resources/dict`), then the source tree. **macOS**: every bundle stays whole and
   holds its own `Resources/dict` (4 copies), now 13.8 MB each, not 54.5 MB.
6. **Packaged-layout check** (`tools/check_packaged_layout.sh`, `jo_datacheck`): from the installed prefix, `en-US`, `de-DE`, and `uk-UA`
   spelling load and answer (a right word and a wrong one), hyphenation breaks a word, and the thesaurus answers; the dictionaries must
   be read from the prefix's data folder; no plain `.dic`, `.aff`, or `.dat` may be installed; all 69 `.jz` files must be.

## Before and after

| | Before | After |
|---|---|---|
| `resources/dict` / installed `dict` folder, bytes (111 files) | 55,841,886 | 14,137,085 |
| Linux and Windows install, one copy | 54.5 MB | 13.8 MB (-40.7 MB) |
| macOS, 4 bundles | 4 x 54.5 = 218 MB | 4 x 13.8 = 55 MB (-163 MB) |
| The same folder as zip -9 (a stand-in for the Windows zip and the .deb) | 12,927,217 | 13,850,055 (+0.9 MB: the files are compressed already, and the thesaurus is in sorted blocks) |
| Thesaurus first lookup (Thesaurus pane, Thesaurus command) | 92 ms | 1 to 2 ms |
| Thesaurus memory after it | +34.8 MB | +1.0 MB |
| `en-US` spelling, first use | 18 ms (+5.7 MB) | 21 ms (+5.8 MB) |
| `de-DE` | 81 ms | 102 ms |
| `pt-BR` | 280 ms | 299 ms |
| `ru-RU` | 65 ms | 77 ms |
| `uk-UA` | 246 ms (+41.6 MB) | 272 ms (+41.7 MB) |
| All 33 languages loaded one after the other (sum of first uses) | 1,579 ms | 1,770 ms |
| Memory with all 33 loaded, then the thesaurus | 402 MB | 371 MB |

The installed size of the apps falls by 40.7 MB on Linux and Windows and about 163 MB on macOS, where it matters most. First use of a
spelling dictionary costs 3 to 25 ms more (the inflate; the largest file is `uk_UA.dic`), and the thesaurus opens 90 ms sooner and holds 34 MB
less. The zipped download is about 0.9 MB larger, because zip already compressed these files (a .dmg stores identical files once, so the four
bundles' copies cost one; that part is unchanged).

## Correctness

`PubDictTests` (tests/pub/tst_pub_dict.cpp):
- **Spelling is the same for every language**: tests/data/dict_golden.txt holds, for each of the 33 languages, how many words of a word
  list taken from its plain `.dic` (every Nth word, the word in capitals, and a misspelling of it: 363 or more words) are right, and the SHA-1 of
  every answer. It was made with main's dictionary code (d1d647828: `libs/base/text/dictionaries.cpp`, `hyphenation.cpp`, and
  `proofingdialogs.cpp` checked out over this branch, `jo_dictbench --golden resources/dict`) reading the plain files through Hunspell's own file
  path, linked against the real, hash-pinned Hunspell 1.7.6 (twice, identical). The test points the dictionaries at the build's compressed
  folder (`RootGuard(builtRoot())`, root checked) so it cannot fall back to plain files. Suggestions are not in the digest: Hunspell cuts its
  search at a time limit, so they differ from run to run on a busy machine.
- **Thesaurus**: the same file holds the SHA-1 of the lookup results for 1,000 words (a third of them inflected, to use the base-form
  search), made before the change. In addition every one of the thesaurus's 145,866 words has the same entry as the plain file's.
- Every compressed file inflates to the plain file byte for byte, and the licenses and READMEs are the same files; no plain dictionary is installed.
- **Plain files still load** (a plain folder gives the same answers).
- **Damaged files fail cleanly**: truncated at every kind of place, a byte changed, trailing bytes, a wrong magic, an empty file: no output
  and no crash. A damaged dictionary turns spell check off for that language only (the rest, the hyphenation, and the thesaurus go on).
  A thesaurus cut at every third byte never opens, and every single byte changed in a small thesaurus is survived.
- **Bounded**: the size a file claims is checked against a cap (64 MB; 4 MB for a thesaurus block; 1 MB for an entry) before any memory is
  allocated, and the stream must end at exactly that size. An 8 MB stream of zeros claiming 1000 bytes, claiming 4 GB, claiming a byte more than
  it makes, or opened under a smaller cap is refused.
