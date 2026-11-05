# Speed bench

`speedbench.py` compares the JeffOffice command-line tools and apps with LibreOffice (`/usr/bin/soffice`) on the same
files from the test collections (`/media/user/volume2/jo-corpora`, read only, nothing copied into the repo).

    python3 tools/bench/speedbench.py            # full run (about an hour on a busy machine)
    python3 tools/bench/speedbench.py --quick    # 10 files, one run each, a few minutes
    options: --build DIR (default ./build, a release or RelWithDebInfo build with writetool, jcrunchtool,
             jdecktool, JeffWrite, JeffCrunch, JeffDeck), --out DIR, --only convert,startup,recalc, --runs N, --no-x,
             --cpus LIST (pin both tools to the same cores with taskset, e.g. 0-7; use it with a quiet machine)

Results (CSV of every file, Markdown summary, the list of files) go to `/home/user/jo-scratch/speedbench/`
(`summary-latest.md`, `results-latest.csv`); the script refuses an output directory inside the repo.

What it does
- Sample: fixed seed (20261105); 40 .docx, 40 .xlsx, 25 .pptx from the collections outside `work/`, plus the three
  largest of each kind (under 30 MB; the converted copies in `work/` hold the biggest), plus up to six workbooks
  with the most formulas for the recalculation.
- Each tool gets a fresh HOME and config dir; LibreOffice a fresh `-env:UserInstallation` profile. The whole run is
  one `systemd-run --user --scope -p MemoryMax=8G`. Tools run one at a time. One warm-up run per tool and measure is
  discarded; each file is read once beforehand to warm the disk cache. Median of 3 runs (5 if the first three
  differ by over 30%); a failure or a timeout (120 s) is counted, not retried. Wall time, CPU time, peak RSS
  (of the largest process, from `wait4`), and the load average are recorded per run.
- convert: open and save in the same format, and open and save as PDF. Ours: `writetool docsave|pdf`,
  `xlsxsave_bench` (a helper built here from `xlsxsave_bench.cpp` with jcrunchtool's flags; it reads every sheet and
  writes the file), `jcrunchtool print-pdf`, `jdecktool save|pdf`. LibreOffice: `--headless --convert-to`.
- startup: ours offscreen with `--screenshot` against LibreOffice's headless first-page render (the nearest fair
  match; the empty case is not comparable), and, on a private X server (Xorg :97, started and stopped by the
  script), the time to the first mapped window and to a main-size window for both.
- recalc: ours `jcrunchtool recalc-full-one` (total, and the calculation phase it reports); LibreOffice with
  "always recalculate" set in the profile, minus the same convert without it.

Limits: the xlsx save of ours copies unchanged parts; the startup measures are not the same event for both programs;
the load average of a shared machine adds noise (CPU time is recorded for that reason).
