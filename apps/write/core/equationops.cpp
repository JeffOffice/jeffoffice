#include "core/equationops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/wml.h"
#include "wml/wmlmath.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QHash>
#include <QVector>

namespace jp::write {

using namespace office::wml;

namespace {

// The equation the character just after `position` holds: its markup and its atom; false for any other character.
bool equationAtomAt(const QTextDocument *story, int position, Inline *atom)
{
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return false;
    for (QTextBlock::iterator it = story->findBlock(position).begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (position < f.position() || position >= f.position() + f.length()) continue;
        const QTextCharFormat cf = f.charFormat();
        if (!cf.hasProperty(prop::Atom)) return false;
        const QByteArray bytes = storedBytes(cf, prop::Atom);
        if (!bytes.contains("oMath")) return false;
        *atom = unpackAtom(bytes);
        return atom->tag == QLatin1String("oMath");
    }
    return false;
}

} // namespace

QString equationAt(const WriteDoc &doc, int position)
{
    Inline atom;
    if (!equationAtomAt(doc.story(), position, &atom)) return QString();
    return equationLinear(atom.raw);
}

bool replaceEquation(WriteDoc &doc, int position, const QString &linear)
{
    Inline atom;
    if (!equationAtomAt(doc.story(), position, &atom) || isLocked(doc.story()->findBlock(position))) return false;
    bool ok = false;
    const QByteArray markup = equationMarkup(linear, &ok);
    if (!ok || markup == atom.raw) return false;
    QTextCursor c(doc.story());
    c.setPosition(position);
    c.setPosition(position + 1, QTextCursor::KeepAnchor);
    return insertEquation(doc, c, linear);
}

bool insertEquation(WriteDoc &doc, QTextCursor &c, const QString &linear)
{
    bool ok = false;
    const QByteArray markup = equationMarkup(linear, &ok);
    if (!ok) return false;
    XmlTree t;
    if (!t.parse("<x>" + markup + "</x>", nullptr)) return false;
    const int n = t.firstChild(t.root());
    if (n < 0) return false;
    const Equation e = equationText(t, n, 4000);
    if (e.text.isEmpty()) return false;
    Inline atom;
    atom.kind = Inline::ParaAtom;
    atom.role = Inline::Single;
    atom.tag = QStringLiteral("oMath");
    atom.raw = markup;
    atom.display = e.text;
    atom.key = e.style;
    // The character is put in with the line the pages show as its field text; the letters' look is set with it, in the same step.
    QTextCursor outer = c;
    outer.beginEditBlock();
    Inline field = atom;
    field.key.clear();
    const int before = std::min(c.selectionStart(), c.position());
    const bool put = insertFieldAtom(doc, c, field);
    if (put) {
        QTextCursor look(c.document());
        look.setPosition(before);
        look.setPosition(c.position(), QTextCursor::KeepAnchor);
        QTextCharFormat f;
        if (atom.key == QLatin1String("d")) f.setProperty(tp::DisplayFamily, QStringLiteral("DejaVu Sans"));
        else if (!atom.key.isEmpty()) f.setProperty(tp::DisplayStyle, atom.key);
        // (The atom itself keeps the key, as a file's equation does.)
        look.mergeCharFormat(f);
    }
    outer.endEditBlock();
    return put;
}

} // namespace jp::write
