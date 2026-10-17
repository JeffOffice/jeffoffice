#pragma once
// A shape's style reference (p:style: lnRef, fillRef, effectRef, and fontRef), read into the model's StyleRef and written back, and what a
// reference gives once the theme's format scheme (a:fmtScheme) says what each of its styles is. The reader (drawingml_shape.cpp) resolves a
// shape's look through drawingml_style.h; this is for the writers, and for an application that sets a reference itself (a Quick Style) and
// needs the look that goes with it.

#include "core/items.h"
#include "office/drawingml.h"

#include <QByteArray>
#include <QDomDocument>
#include <QXmlStreamWriter>

namespace jp::office {

// The reference in a p:style; one that is off if there is none, or if it names a color this can't keep (a system or preset color, or a change
// with no value), so that the shape's look stays the properties it was read with.
StyleRef readStyleRef(const QDomElement &style);
// <p:style> with its four references, as the presentation program writes a shape's style.
void writeStyleRef(QXmlStreamWriter &w, const StyleRef &ref);

// What a reference gives.
struct StyleLook {
    Fill fill;
    Stroke stroke;
    Effects fx;
    QColor text;   // the color of the text it asks for; invalid if it names none
};

// The theme's format scheme and colors, to see what a reference gives. `map` says which theme color "bg1", "tx1", and the others are (a dark
// slide master has its own).
class StyleResolver {
public:
    explicit StyleResolver(const QByteArray &themeXml, const ColorMap &map = ColorMap());
    // False if the part is not a theme (nothing is given then).
    bool valid() const { return m_valid; }
    // The look of a reference on a shape of this size (a gradient written as scaled follows it).
    StyleLook look(const StyleRef &ref, const QSizeF &frame = QSizeF()) const;
    // A color of a reference as the color it stands for; invalid for none.
    QColor color(const StyleColor &color) const;
    // The color a theme color is by its name ("accent1", "lt1", "tx1").
    QColor themeColor(const QString &name) const { return color(StyleColor{name, {}}); }

private:
    QDomDocument m_dom;
    DrawingContext m_ctx;
    bool m_valid = false;
};

} // namespace jp::office
