#pragma once
// Where a workbook of the XML form keeps the language of a cell's text and "do not check spelling". The form has no element for either, so they are kept in an extension of the cell
// format (an `extLst` of the `xf`, with an `ext` of this program's own address): a reader that does not know the extension leaves it alone, as the form says to.

#include "sheet/styles.h"
#include "sheet/xmlscan.h"

#include <QByteArray>

namespace jp::office::sheet {

inline const char *proofingExtensionUri() { return "{6D1F3A52-8B47-4C0E-9E21-5A7B3C94D0F8}"; }
inline const char *proofingExtensionNamespace() { return "http://jeffoffice.com/ns/spreadsheet-proofing/1"; }

// The `extLst` element that holds the format's language and mark (`prefix` is the main namespace's prefix with its colon, when the part uses one); empty when the format has neither.
inline QByteArray proofingExtensionXml(const CellFormat &f, const QByteArray &prefix = QByteArray())
{
    if (f.language.isEmpty() && !f.noProof) return QByteArray();
    QByteArray out = "<" + prefix + "extLst><" + prefix + "ext uri=\"" + proofingExtensionUri() + "\" xmlns:jo=\"" + proofingExtensionNamespace() + "\"><jo:proofing";
    if (!f.language.isEmpty()) out += " lang=\"" + xmlscan::escapeAttribute(f.language) + "\"";
    if (f.noProof) out += " noProof=\"1\"";
    return out + "/></" + prefix + "ext></" + prefix + "extLst>";
}

// `inner` (the children of an `xf` that are not its alignment and protection) without this program's extension, and with `extension` (from proofingExtensionXml) put in: in the
// `extLst` the file already has, else as one of its own at the end.
inline QByteArray withProofingExtension(QByteArray inner, const QByteArray &extension, const QByteArray &prefix = QByteArray())
{
    // (our extension is found by reading the tags: whatever space follows `<ext`, the attributes in any order, either quote)
    const QByteArray open = "<" + prefix + "ext";
    const QByteArray closeExt = "</" + prefix + "ext>";
    for (qsizetype from = 0;;) {
        qsizetype start = inner.indexOf(open, from);
        if (start < 0) break;
        const qsizetype after = start + open.size();
        from = after;
        if (after >= inner.size() || !(inner[after] == ' ' || inner[after] == '\n' || inner[after] == '\t' || inner[after] == '\r')) continue;
        const qsizetype tagEnd = inner.indexOf('>', after);
        if (tagEnd < 0) break;
        const QByteArray tag = inner.mid(after, tagEnd - after);
        if (!tag.contains(QByteArray("\"") + proofingExtensionUri() + "\"") && !tag.contains(QByteArray("'") + proofingExtensionUri() + "'")) continue;
        const qsizetype end = inner.indexOf(closeExt, tagEnd);
        if (end < 0) break;
        inner.remove(start, end + closeExt.size() - start);
        from = start;
    }
    // (an extension list with nothing left in it is left out)
    const QByteArray emptyList = "<" + prefix + "extLst></" + prefix + "extLst>";
    if (const qsizetype empty = inner.indexOf(emptyList); empty >= 0) inner.remove(empty, emptyList.size());
    if (extension.isEmpty()) return inner;
    const QByteArray closeList = "</" + prefix + "extLst>";
    const qsizetype list = inner.lastIndexOf(closeList);
    if (list >= 0) {
        const QByteArray ext = extension.mid(extension.indexOf("<" + prefix + "ext "));
        inner.insert(list, ext.left(ext.size() - closeList.size()));
        return inner;
    }
    return inner + extension;
}

} // namespace jp::office::sheet
