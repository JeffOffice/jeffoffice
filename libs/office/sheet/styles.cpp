#include "sheet/styles.h"

#include "calc/numfmt.h"

namespace jp::office::sheet {

QString Styles::formatCode(quint32 xf) const
{
    const qint32 id = numFmtId(xf);
    const auto it = numFmts.constFind(id);
    if (it != numFmts.constEnd()) return it.value();
    const QString builtin = jp::calc::builtinFormatCode(id);
    return builtin.isEmpty() ? QStringLiteral("General") : builtin;
}

} // namespace jp::office::sheet
