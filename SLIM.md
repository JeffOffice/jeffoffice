# Slimming the suite (round 39)

Linux x86-64, GCC, Qt 6.11.3, Release build (`-O3`), on the build machine. Sizes are bytes of the files on disk.
"Stripped" is `strip` of a copy. Startup is the median of 5 runs of a fresh process (the first, warm-up run is dropped), offscreen,
a new empty HOME each run, no file, `JP_STARTUP_TRACE=1` with `JP_STARTUP_T0` set to the launch time (so the program's own marks are
milliseconds since launch) and `JP_GRAB` to make it quit: "first paint" and "usable" are the program's own marks ("first paint",
"window usable"); "whole run" also includes the 200 ms the grab hook waits before it quits.

## Step 0: baseline (main 32dde24f4, every layer a static library, all of Help and the translations in resources.qrc)

| App | exe | exe stripped | first paint | usable | whole run |
|---|---|---|---|---|---|
| JeffPub | 47,500,416 | 43,450,880 | 80.1 ms | 178.6 ms | 472.3 ms |
| JeffDeck | 49,971,176 | 45,756,496 | 91.0 ms | 190.4 ms | 471.1 ms |
| JeffWrite | 53,896,336 | 49,207,280 | 75.2 ms | 154.3 ms | 418.1 ms |
| JeffCrunch | 53,308,400 | 48,559,344 | 74.1 ms | 153.4 ms | 415.8 ms |

Install size now (stripped exes only, since Help and translations are inside them): 187,000,000 bytes for the four.
