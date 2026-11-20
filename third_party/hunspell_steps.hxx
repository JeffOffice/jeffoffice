/* JeffOffice patch (third_party/README.md): the suggestion and compound-word searches of Hunspell 1.7.6 end by a wall-clock limit
   (250, 100, and 50 ms), so a slow or busy machine found fewer suggestions than a fast one, and the same word could give two lists.
   Here the clock is a counter: each time Hunspell reads "the time" (at the checks inside its loops) and each time it looks a word up in
   the dictionary, the counter goes up by one, and a limit is a number of those checks. The same word in the same dictionary then stops at the same place on every machine.
   The counter is per thread, and a search only ever compares two readings of it, so its starting value does not matter. */
#ifndef JEFFOFFICE_HUNSPELL_STEPS_HXX
#define JEFFOFFICE_HUNSPELL_STEPS_HXX

#include <chrono>
#include <ratio>

inline long long& jeffoffice_hunspell_steps() {
  static thread_local long long steps = 0;
  return steps;
}

/* A check costs more in some dictionaries (long affix tables) than in others. The program sets this, for the thread that is searching,
   to the percent of the limits its dictionary gets (100 when it never sets it); the same dictionary always gets the same limit. */
inline int& jeffoffice_hunspell_percent() {
  static thread_local int percent = 100;
  return percent;
}

struct jeffoffice_steady_clock {
  typedef long long rep;
  typedef std::ratio<1> period;
  typedef std::chrono::duration<rep, period> duration;
  typedef std::chrono::time_point<jeffoffice_steady_clock> time_point;
  static const bool is_steady = true;
  static time_point now() { return time_point(duration(++jeffoffice_hunspell_steps())); }
};

/* The limits, in checks (the old ones in milliseconds, at about 0.3 microsecond a check on the machine they were measured on, see
   third_party/README.md): the whole suggestion (250 ms before), one search for plain or compound-word suggestions (100 ms), and one
   long compound-word or REP-table search (50 ms). */
#define TIMELIMIT_GLOBAL_MS jeffoffice_steady_clock::duration(JEFFOFFICE_STEPS_GLOBAL * jeffoffice_hunspell_percent() / 100)
#define TIMELIMIT_SUGGESTION_MS jeffoffice_steady_clock::duration(JEFFOFFICE_STEPS_SUGGESTION * jeffoffice_hunspell_percent() / 100)
#define TIMELIMIT_MS jeffoffice_steady_clock::duration(JEFFOFFICE_STEPS_ONE * jeffoffice_hunspell_percent() / 100)
#define JEFFOFFICE_STEPS_GLOBAL 750000LL
#define JEFFOFFICE_STEPS_SUGGESTION 300000LL
#define JEFFOFFICE_STEPS_ONE 150000LL

#endif
