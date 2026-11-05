#pragma once
// Goal Seek: finds the number to put in one cell so that a formula that reads it comes to a target. The search is the numbers' only: how a cell is changed and how the formula is worked out
// again is the caller's (`trial`).

#include <QString>

#include <functional>
#include <optional>

namespace jp::crunch {

struct GoalSeekResult {
    bool found = false;     // the formula came within `tolerance` of the target
    double input = 0;       // the best number found (the start if nothing better)
    double result = 0;      // what the formula came to for it
    int trials = 0;         // how many numbers were tried
};

constexpr int kGoalSeekMaxTrials = 200;
constexpr double kGoalSeekTolerance = 0.001;   // (the result is within this of the target)

// `trial(x)` puts x in the cell and gives what the formula comes to (nothing when it is not a number: an error, text, or empty). `start` is the number the cell holds.
GoalSeekResult goalSeek(const std::function<std::optional<double>(double)> &trial, double start, double target, double tolerance = kGoalSeekTolerance, int maxTrials = kGoalSeekMaxTrials);

} // namespace jp::crunch
