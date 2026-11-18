#pragma once
// Equations between the math markup of ECMA-376 part 1, chapter 22 (m:oMath) and MathML 3 (presentation markup), the form an .odt or .odp keeps an equation in
// (the Object N/content.xml of a draw:object). Each function reads a part of a file, so each is bounded: at most kMostBytes of XML, kMostElements elements, and
// kMostDepth levels of nesting (and the output at most kMostOutputBytes and kMostOutputElements), the root in its own namespace (MathML may have none, if its root has none), and a DOCTYPE (so any entity, internal or external) is refused. Whatever a function cannot map is kept as a run of its text, not dropped.

#include "core/export.h"

#include <QByteArray>
#include <QString>

namespace jp::office::mathml {

constexpr int kMostBytes = 1 << 20;
constexpr int kMostElements = 4000;
constexpr int kMostDepth = 64;
// What a conversion may make: bytes and elements of its output (a long text becomes an element a character at the most), or it is refused.
constexpr int kMostOutputBytes = 4 << 20;
constexpr int kMostOutputElements = 20000;

// The MathML (a math element, declaring its namespace) of the markup (an m:oMath, or an m:oMathPara of several), or an empty array for markup that is refused or has nothing in it.
// `ok` (if given) is false when it is refused, and `error` says why.
JO_COMMON_EXPORT QByteArray mathmlFromOmml(const QByteArray &markup, bool *ok = nullptr, QString *error = nullptr);
// The markup (an m:oMath declaring its own namespace) of a MathML part (a math root, with or without a prefix on its names; a semantics element is read through to its first child
// and its annotations, as the StarMath one of an .odt, are left out), or an empty array for one that is refused. `ok` and `error` as above.
JO_COMMON_EXPORT QByteArray ommlFromMathml(const QByteArray &xml, bool *ok = nullptr, QString *error = nullptr);

} // namespace jp::office::mathml
