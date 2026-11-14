// The word lists of the grammar rules are made on first use, which many threads can reach at once (tests/shell/tst_shell_grammar_rules.cpp,
// firstUseByManyThreadsIsSafe). A program of its own, whose first use it is, with none of the test program's start-up:
//   jpgrammarprobe
// Sixteen threads begin together and check the same text; exits 0 when all get the same, nonzero number of issues.

#include "grammar/grammarrules.h"

#include <QString>

#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

int main()
{
    const QString text = QStringLiteral("Your a fool. It was a very big the the mistake, and he could of gone. Its a nice day.Do not stop. Mr. Smith vs. Jones etc. a apple. "
                                        "She will loose weight, and I would of known. Your not wrong. There going to there house.");
    std::vector<std::thread> threads;
    std::vector<size_t> counts(16, 0);
    std::atomic<bool> go(false);
    for (int t = 0; t < 16; ++t)
        threads.emplace_back([&, t] {
            while (!go.load()) {}   // (all begin together)
            counts[size_t(t)] = jp::grammar::check(text).size();
        });
    go = true;
    for (std::thread &th : threads) th.join();
    for (size_t t = 1; t < counts.size(); ++t)
        if (counts[t] != counts[0]) {
            std::fprintf(stderr, "thread %zu found %zu issues, thread 0 found %zu\n", t, counts[t], counts[0]);
            return 1;
        }
    if (counts[0] == 0) {
        std::fprintf(stderr, "no issues found\n");
        return 2;
    }
    return 0;
}
