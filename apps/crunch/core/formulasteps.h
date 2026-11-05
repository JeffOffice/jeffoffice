#pragma once
// Evaluate Formula: a formula worked out one part at a time. Each step takes the part that comes next (the leftmost cell, then the innermost function call, then the innermost brackets, then
// what is left) and puts its value in its place, so a person sees each part's value. The values come from `eval`, which works an expression out on the spot.

#include <QString>

#include <functional>
#include <optional>

namespace jp::crunch {

class FormulaSteps {
public:
    // What an expression (no leading =) comes to, as it is written in a formula (text in quotes, numbers as typed); nothing if it cannot be worked out (`problem` says why).
    using Eval = std::function<std::optional<QString>(const QString &expression, QString *problem)>;
    FormulaSteps(const QString &expression, Eval eval);
    // The formula as it is now, and the part the next step works out (length 0 when there is none).
    QString text() const { return m_text; }
    int underlineStart() const { return m_from; }
    int underlineLength() const { return m_length; }
    // True when the formula is one value (or the next step could not be taken).
    bool finished() const { return m_finished; }
    QString problem() const { return m_problem; }
    // Works out the underlined part. False when finished, or when the part could not be worked out (then `problem` is set and the formula is as it was).
    bool step();
    void restart();

private:
    void findNext();
    QString m_original, m_text, m_problem;
    Eval m_eval;
    int m_from = 0, m_length = 0;
    bool m_finished = false;
};

} // namespace jp::crunch
