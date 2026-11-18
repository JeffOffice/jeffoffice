#include "core/guideevaluator.h"

#include <QStringList>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

constexpr int kMaxGuides = 2000;
constexpr int kMaxGuideDepth = 64;
constexpr double kMaxValue = 1e15;

double finite(double v)
{
    return std::isfinite(v) ? std::clamp(v, -kMaxValue, kMaxValue) : 0.0;
}

double toRadians(double angle60k)
{
    return angle60k / 60000.0 * M_PI / 180.0;
}

double toAngle60k(double radians)
{
    return radians * 180.0 / M_PI * 60000.0;
}

} // namespace

GuideEvaluator::GuideEvaluator(double width, double height)
{
    width = finite(width);
    height = finite(height);
    const double ss = std::min(width, height), ls = std::max(width, height);
    const struct { const char *name; double value; } names[] = {
        {"w", width}, {"h", height}, {"l", 0}, {"t", 0}, {"r", width}, {"b", height}, {"hc", width / 2}, {"vc", height / 2},
        {"wd2", width / 2}, {"wd3", width / 3}, {"wd4", width / 4}, {"wd5", width / 5}, {"wd6", width / 6}, {"wd8", width / 8},
        {"wd10", width / 10}, {"wd32", width / 32},
        {"hd2", height / 2}, {"hd3", height / 3}, {"hd4", height / 4}, {"hd5", height / 5}, {"hd6", height / 6}, {"hd8", height / 8}, {"hd10", height / 10},
        {"ss", ss}, {"ls", ls}, {"ssd2", ss / 2}, {"ssd4", ss / 4}, {"ssd6", ss / 6}, {"ssd8", ss / 8}, {"ssd16", ss / 16}, {"ssd32", ss / 32},
        {"cd2", 10800000}, {"cd4", 5400000}, {"cd8", 2700000}, {"3cd4", 16200000}, {"3cd8", 8100000}, {"5cd8", 13500000}, {"7cd8", 18900000}};
    for (const auto &n : names) m_values.insert(QString::fromLatin1(n.name), n.value);
}

void GuideEvaluator::define(const QString &name, const QString &formula)
{
    if (name.isEmpty() || m_values.contains(name) || m_formulas.contains(name) || m_formulas.size() >= kMaxGuides) return;
    m_formulas.insert(name, formula.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts));
}

void GuideEvaluator::define(const QString &name, const QStringList &words)
{
    if (name.isEmpty() || m_values.contains(name) || m_formulas.contains(name) || m_formulas.size() >= kMaxGuides) return;
    m_formulas.insert(name, words);
}

void GuideEvaluator::set(const QString &name, double value)
{
    if (name.isEmpty()) return;
    m_formulas.remove(name);
    m_values.insert(name, finite(value));
}

double GuideEvaluator::value(const QString &token)
{
    const auto known = m_values.constFind(token);
    if (known != m_values.constEnd()) return *known;
    const auto formula = m_formulas.constFind(token);
    if (formula != m_formulas.constEnd()) {
        if (m_depth >= kMaxGuideDepth || m_active.contains(token)) return 0;
        ++m_depth;
        m_active.insert(token);
        const double v = evaluate(*formula);
        --m_depth;
        m_active.remove(token);
        m_values.insert(token, v);
        return v;
    }
    bool ok = false;
    const double number = token.toDouble(&ok);
    return ok ? finite(number) : 0.0;
}

double GuideEvaluator::evaluate(const QStringList &parts)
{
    if (parts.isEmpty()) return 0;
    const QString &op = parts[0];
    auto arg = [&](int i) { return i < parts.size() ? value(parts[i]) : 0.0; };
    auto divide = [](double n, double d) { return d == 0 ? 0.0 : n / d; };
    double r = 0;
    if (op == QLatin1String("val")) r = arg(1);
    else if (op == QLatin1String("*/")) { const double x = arg(1), y = arg(2), z = arg(3); r = divide(x * y, z); }
    else if (op == QLatin1String("+-")) { const double x = arg(1), y = arg(2), z = arg(3); r = x + y - z; }
    else if (op == QLatin1String("+/")) { const double x = arg(1), y = arg(2), z = arg(3); r = divide(x + y, z); }
    else if (op == QLatin1String("?:")) { const double x = arg(1), y = arg(2), z = arg(3); r = x > 0 ? y : z; }
    else if (op == QLatin1String("abs")) r = std::fabs(arg(1));
    else if (op == QLatin1String("at2")) { const double x = arg(1), y = arg(2); r = toAngle60k(std::atan2(y, x)); }
    else if (op == QLatin1String("cat2")) { const double x = arg(1), y = arg(2), z = arg(3); r = x * std::cos(std::atan2(z, y)); }
    else if (op == QLatin1String("sat2")) { const double x = arg(1), y = arg(2), z = arg(3); r = x * std::sin(std::atan2(z, y)); }
    else if (op == QLatin1String("cos")) { const double x = arg(1), y = arg(2); r = x * std::cos(toRadians(y)); }
    else if (op == QLatin1String("sin")) { const double x = arg(1), y = arg(2); r = x * std::sin(toRadians(y)); }
    else if (op == QLatin1String("tan")) { const double x = arg(1), y = arg(2); r = x * std::tan(toRadians(y)); }
    else if (op == QLatin1String("max")) { const double x = arg(1), y = arg(2); r = std::max(x, y); }
    else if (op == QLatin1String("min")) { const double x = arg(1), y = arg(2); r = std::min(x, y); }
    else if (op == QLatin1String("mod")) { const double x = arg(1), y = arg(2), z = arg(3); r = std::sqrt(x * x + y * y + z * z); }
    else if (op == QLatin1String("pin")) { const double x = arg(1), y = arg(2), z = arg(3); r = y < x ? x : y > z ? z : y; }
    else if (op == QLatin1String("sqrt")) { const double x = arg(1); r = x > 0 ? std::sqrt(x) : 0.0; }
    return finite(r);
}

} // namespace jp
