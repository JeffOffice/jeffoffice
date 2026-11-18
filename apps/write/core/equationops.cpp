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

// The w:rPr every run of an equation made elsewhere carries (the face and weight of its letters): true with `props` set (empty when no run has one) when all the runs agree, false
// when they differ or only some have one, as an edit that rebuilds the runs could not keep them apart.
bool commonRunProps(const QByteArray &raw, QByteArray *props)
{
    props->clear();
    XmlTree t;
    if (!t.parse("<x>" + raw + "</x>", nullptr)) return false;
    const int root = t.firstChild(t.root());
    if (root < 0) return false;
    int runs = 0, withProps = 0;
    QByteArray first;
    bool same = true;
    QVector<int> todo{root};
    while (!todo.isEmpty()) {
        const int n = todo.takeLast();
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (t.ns(c) == NsW) continue;
            if (t.localName(c) != "r") {
                todo << c;
                continue;
            }
            ++runs;
            for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                if (t.ns(k) != NsW || t.localName(k) != "rPr") continue;
                const QByteArray one = t.raw(k);
                if (!withProps++) first = one;
                else if (one != first) same = false;
                break;
            }
        }
    }
    if (withProps == 0) return true;
    if (!same || withProps != runs) return false;
    *props = first;
    return true;
}

// The markup with `props` (a w:rPr) put in every run, after its m:rPr if it has one and before its text.
QByteArray withRunProps(QByteArray markup, const QByteArray &props)
{
    if (props.isEmpty()) return markup;
    markup.replace("</m:rPr><m:t", "</m:rPr>" + props + "<m:t");
    markup.replace("<m:r><m:t", "<m:r>" + props + "<m:t");
    return markup;
}

bool insertMarkup(WriteDoc &doc, QTextCursor &c, const QByteArray &markup)
{
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

} // namespace

QString equationAt(const WriteDoc &doc, int position)
{
    Inline atom;
    QByteArray props;
    if (!equationAtomAt(doc.story(), position, &atom) || !commonRunProps(atom.raw, &props)) return QString();   // (runs that differ in look can't be rebuilt)
    return equationLinear(atom.raw);
}

bool replaceEquation(WriteDoc &doc, int position, const QString &linear)
{
    Inline atom;
    if (!equationAtomAt(doc.story(), position, &atom) || isLocked(doc.story()->findBlock(position))) return false;
    QByteArray props;
    if (!commonRunProps(atom.raw, &props)) return false;
    bool ok = false;
    const QByteArray markup = withRunProps(equationMarkup(linear, &ok), props);
    if (!ok || markup == atom.raw) return false;
    if (!props.isEmpty() && markup.count(props) != markup.count("<m:r>")) return false;
    QTextCursor c(doc.story());
    c.setPosition(position);
    c.setPosition(position + 1, QTextCursor::KeepAnchor);
    return insertMarkup(doc, c, markup);
}

bool insertEquation(WriteDoc &doc, QTextCursor &c, const QString &linear)
{
    bool ok = false;
    const QByteArray markup = equationMarkup(linear, &ok);
    if (!ok) return false;
    return insertMarkup(doc, c, markup);
}

} // namespace jp::write
