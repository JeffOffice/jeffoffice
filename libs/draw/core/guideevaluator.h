#pragma once
// Guides: named values that formulas make from a shape's size and its adjustments (ECMA-376 20.1.9). The preset shapes' connection sites (core/connsites.h)
// and the outlines of drawingml_geometry.h (libs/office) both evaluate them.

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

namespace jp {

// Evaluates guide formulas, such as "*/ w adj1 100000" (x * y / z). The
// operators are */ +- +/ ?: abs at2 cat2 cos max min mod pin sat2 sin sqrt
// tan and val; angles are in 60,000ths of a degree. The names a formula can
// use are those defined, the shape's w and h (in EMU), and the built-in
// ones that follow from them (l t r b hc vc wd2 hd2 ss ls cd4 3cd4 ...).
// Every result is finite: a division by zero, a name that was never
// defined, a guide that depends on itself, and a chain of guides nested more
// deeply than a drawing needs each give 0.
class GuideEvaluator {
public:
    GuideEvaluator(double width, double height);
    // Defines a guide by its formula; a name defined already keeps its value.
    void define(const QString &name, const QString &formula);
    // The same for a formula already split into its words (a list that is used again and again splits once).
    void define(const QString &name, const QStringList &words);
    // Defines a name as a number.
    void set(const QString &name, double value);
    // The value of a name or of a number written out.
    double value(const QString &token);

private:
    double evaluate(const QStringList &parts);

    QHash<QString, double> m_values;
    QHash<QString, QStringList> m_formulas;
    QSet<QString> m_active;
    int m_depth = 0;
};

} // namespace jp
