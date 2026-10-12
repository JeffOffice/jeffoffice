// Writing DrawingML (drawingml_write.h). This first version writes nothing;
// shapes, pictures, and text follow.

#include "office/drawingml_write.h"

namespace jp::office {

void writeShape(QXmlStreamWriter &, const Item &item, DrawingWriteContext &, QStringList *skipped)
{
    if (skipped) skipped->append(item.id);
}

void writeParagraphs(QXmlStreamWriter &w, const QTextDocument *, DrawingWriteContext &)
{
    w.writeEmptyElement(QStringLiteral("a:p"));
}

void writeFill(QXmlStreamWriter &, const Fill &, DrawingWriteContext &) {}

void writeLine(QXmlStreamWriter &, const Stroke &, DrawingWriteContext &) {}

QString presetForShape(const QString &shapeId) { return shapeId; }

} // namespace jp::office
