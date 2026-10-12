// Text bodies (drawingml.h). A first, plain reading: the paragraphs' text
// and line breaks; formatting follows.

#include "office/drawingml.h"

#include <QTextCursor>
#include <QTextDocument>

namespace jp::office {

void readTextBody(const QDomElement &txBody, QTextDocument *doc, const DrawingContext &, const TextStyleStack &)
{
    QStringList paragraphs;
    for (QDomElement p = txBody.firstChildElement(); !p.isNull(); p = p.nextSiblingElement()) {
        if (p.localName() != QLatin1String("p")) continue;
        QString text;
        for (QDomElement r = p.firstChildElement(); !r.isNull(); r = r.nextSiblingElement()) {
            if (r.localName() == QLatin1String("r") || r.localName() == QLatin1String("fld")) {
                for (QDomElement t = r.firstChildElement(); !t.isNull(); t = t.nextSiblingElement())
                    if (t.localName() == QLatin1String("t")) text += t.text();
            } else if (r.localName() == QLatin1String("br")) {
                text += QChar(QChar::LineSeparator);
            }
        }
        paragraphs << text;
    }
    doc->setPlainText(paragraphs.join(QLatin1Char('\n')));
}

} // namespace jp::office
