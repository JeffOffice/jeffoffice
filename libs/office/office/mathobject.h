#pragma once
// An equation as an OpenDocument file keeps it: a formula object (a draw:frame with a draw:object) whose `Object N/content.xml` is a MathML document, with a picture of the equation beside it
// for the programs that don't read formulas. The writers and readers of the text, presentation, and spreadsheet forms share this: the conversion (libs/office/mathml) behind a check of the
// sub-document, and the limits on how many a file holds and how much they come to. Internal to the readers and writers of those forms.

#include "core/export.h"

#include <QByteArray>
#include <QColor>

#include <atomic>

namespace jp::office {

class Package;

// The most formula objects a file has (more are left as their pictures, if they have any), and the most bytes of formula documents all of them come to, read or written.
constexpr int kMostMathObjects = 2000;
constexpr qint64 kMostMathObjectBytes = qint64(16) << 20;

// The equation markup (an m:oMath element, declaring its own namespace) of the content.xml of a formula object; empty for a document that is refused: too large, a root that is not MathML's
// math element, an element that runs or loads something (a script, a frame, an embedded object, an action), or a link or an event handler on any element.
JO_COMMON_EXPORT QByteArray mathObjectToOmml(const QByteArray &contentXml);
// What tests count: the documents of embedded objects (formulas, and the charts that sit beside them) the readers of the text and presentation forms inflate whole.
struct MathObjectTestHooks {
    std::atomic<qint64> objectReads{0};
};
JO_COMMON_EXPORT MathObjectTestHooks &mathObjectTestHooks();
// The first bytes of a part (4 KB, inflated no further) and the package's name for the part: empty for a part that is not there.
JO_COMMON_EXPORT QByteArray peekPart(const Package &package, const QString &part);

// True if the document, of which the first bytes are given (4 KB is enough), has MathML's math element as its root: a formula and not a chart, whatever namespaces the chart declares.
JO_COMMON_EXPORT bool isMathObjectDocument(const QByteArray &head);
// A formula document with the color its letters are drawn in written on its root (mathcolor="#rrggbb"), and the color a document's root names (invalid for none, or a form that is not #rrggbb).
JO_COMMON_EXPORT QByteArray mathObjectWithColor(const QByteArray &contentXml, const QColor &color);
JO_COMMON_EXPORT QColor mathObjectColor(const QByteArray &contentXml);
// The content.xml (MathML) of the equation markup (an m:oMath or an m:oMathPara); empty for markup that is refused.
JO_COMMON_EXPORT QByteArray mathObjectFromOmml(const QByteArray &markup);

} // namespace jp::office
