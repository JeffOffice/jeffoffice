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
   with `--gc-sections` (`-dead_strip` on macOS). CI turns it off in the test-only builds.

## Before and after

Before = main 32dde24f4 (every layer a static library, all of Help and the translations in resources.qrc). After = this branch,
Release with `JO_SLIM_RELEASE=ON`. "Without LTO" is the same branch with it OFF.

| | Before | After, no LTO | After (slim) |
|---|---|---|---|
| JeffPub exe, stripped | 43,450,880 | 4,343,464 | 4,056,608 |
| JeffDeck exe, stripped | 45,756,496 | 6,536,096 | 5,671,712 |
| JeffWrite exe, stripped | 49,207,280 | 8,002,560 | 6,933,336 |
| JeffCrunch exe, stripped | 48,559,344 | 8,398,032 | 7,038,008 |
| libjeffoffice-common.so, stripped | | 28,587,496 | 23,396,144 |
| Data folder (help 15,132,971 + i18n 18,910,762) | inside the exes | 34,043,733 | 34,043,733 |
| **Install total, stripped** | **186,974,000** | **89,911,381** | **81,139,541** |
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
