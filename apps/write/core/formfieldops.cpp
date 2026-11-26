#include "core/formfieldops.h"

#include "core/editops.h"
#include "core/fieldops.h"
#include "core/writeprops.h"
#include "fields/fieldcode.h"

#include <QDate>
#include <QLocale>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QXmlStreamReader>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {

QTextCharFormat formatAt(QTextDocument *story, int position)
{
    QTextCursor c(story);
    c.setPosition(position + 1);
    return c.charFormat();
}

// The same atom with another body, put in the format of the character at the position (its shown text too).
void putAtom(QTextDocument *story, int position, const Inline &atom)
{
    QTextCharFormat f = formatAt(story, position);
    setStored(f, prop::Atom, packAtom(atom));
    f.setProperty(tp::Field, QStringLiteral("raw:") + atom.display);
    QTextCursor c(story);
    c.setPosition(position);
    c.setPosition(position + 1, QTextCursor::KeepAnchor);
    c.setCharFormat(f);
}

QString localName(QStringView qualified)
{
    const qsizetype colon = qualified.indexOf(QLatin1Char(':'));
    return (colon < 0 ? qualified : qualified.mid(colon + 1)).toString();
}

QString valueOf(const QXmlStreamReader &r, const QString &name = QStringLiteral("val"))
{
    for (const QXmlStreamAttribute &a : r.attributes())
        if (localName(a.qualifiedName()) == name) return a.value().toString();
    return QString();
}

bool hasValueOf(const QXmlStreamReader &r, const QString &name = QStringLiteral("val"))
{
    for (const QXmlStreamAttribute &a : r.attributes())
        if (localName(a.qualifiedName()) == name) return true;
    return false;
}

// An on/off element: on unless its value says off.
bool isOn(const QXmlStreamReader &r)
{
    if (!hasValueOf(r)) return true;
    const QString v = valueOf(r).toLower();
    return !(v == QLatin1String("0") || v == QLatin1String("false") || v == QLatin1String("off"));
}

} // namespace

bool FormField::fillable() const
{
    if (!enabled) return false;
    if (kind != FormKind::Text) return true;
    return textType == QLatin1String("regular") || textType == QLatin1String("number") || textType == QLatin1String("date");
}

FormField formFieldFromData(const QByteArray &beginRaw, bool *okOut)
{
    FormField f;
    bool found = false, haveChecked = false, defaultOn = false, haveResult = false;
    int result = 0, def = 0;
    QXmlStreamReader r(beginRaw.left(kMostFormDataBytes));
    r.setNamespaceProcessing(false);
    bool inData = false;
    while (!r.atEnd()) {
        if (r.readNext() != QXmlStreamReader::StartElement) continue;
        const QString n = localName(r.qualifiedName());
        if (n == QLatin1String("ffData")) {
            inData = true;
            continue;
        }
        if (!inData) continue;
        if (n == QLatin1String("name")) f.name = valueOf(r).left(kMostFormChars);
        else if (n == QLatin1String("enabled")) f.enabled = isOn(r);
        else if (n == QLatin1String("calcOnExit")) f.calcOnExit = isOn(r);
        else if (n == QLatin1String("helpText")) f.help = valueOf(r).left(kMostFormChars);
        else if (n == QLatin1String("statusText")) f.status = valueOf(r).left(kMostFormChars);
        else if (n == QLatin1String("checkBox")) {
            f.kind = FormKind::CheckBox;
            found = true;
        } else if (n == QLatin1String("ddList")) {
            f.kind = FormKind::DropDown;
            found = true;
        } else if (n == QLatin1String("textInput")) {
            f.kind = FormKind::Text;
            found = true;
        } else if (n == QLatin1String("size")) f.size = valueOf(r).toInt();
        else if (n == QLatin1String("sizeAuto")) f.size = 0;
        else if (n == QLatin1String("checked")) {
            haveChecked = true;
            f.checked = isOn(r);
        } else if (n == QLatin1String("default")) {
            if (f.kind == FormKind::Text && found) f.defaultText = valueOf(r).left(kMostFormChars);
            else if (f.kind == FormKind::CheckBox) defaultOn = isOn(r) && valueOf(r) != QLatin1String("0");
            else def = valueOf(r).toInt();
        } else if (n == QLatin1String("result")) {
            haveResult = true;
            result = valueOf(r).toInt();
        } else if (n == QLatin1String("listEntry")) {
            if (f.entries.size() < kMostFormEntries) f.entries << valueOf(r).left(kMostFormChars);
        } else if (n == QLatin1String("type") && f.kind == FormKind::Text) f.textType = valueOf(r);
        else if (n == QLatin1String("maxLength")) f.maxLength = std::max(0, valueOf(r).toInt());
        else if (n == QLatin1String("format")) f.format = valueOf(r).left(kMostFormChars);
    }
    if (f.kind == FormKind::CheckBox && !haveChecked) f.checked = defaultOn;
    if (f.kind == FormKind::DropDown && !f.entries.isEmpty()) f.chosen = std::clamp(haveResult ? result : std::max(def, 0), 0, int(f.entries.size()) - 1);
    if (okOut) *okOut = found;
    return f;
}

namespace {

// The form fields of a scan, with their results.
QVector<FormField> fieldsOf(const StoryScan &scan)
{
    QVector<FormField> out;
    for (const FieldPlace &p : scan.fields) {
        if (out.size() >= kMostFormFields) break;
        if (p.form != FieldPlace::Complex || p.begin < 0 || p.end < 0) continue;
        if (p.name != QLatin1String("FORMTEXT") && p.name != QLatin1String("FORMCHECKBOX") && p.name != QLatin1String("FORMDROPDOWN")) continue;
        out << FormField();
        out.last().begin = p.begin;
        out.last().separate = p.separate;
        out.last().end = p.end;
    }
    return out;
}

// Reads the data of each field from its begin mark, and what it holds and the bookmark round it; a field whose mark says nothing of what it is is left out.
void describe(QTextDocument *story, const StoryScan &scan, QVector<FormField> *fields)
{
    QVector<FormField> kept;
    for (const FormField &f : std::as_const(*fields)) {
        const Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
        bool ok = false;
        FormField data = formFieldFromData(atom.raw, &ok);
        if (!ok) continue;
        data.begin = f.begin;
        data.separate = f.separate;
        data.end = f.end;
        data.result = f.separate >= 0 ? scan.plain.mid(f.separate + 1, f.end - f.separate - 1).remove(QChar::ObjectReplacementCharacter).left(kMostFormChars) : QString();
        for (const BookmarkSpan &b : scan.bookmarks)
            if (b.start <= f.begin && b.end >= f.end && (!b.hidden || data.bookmark.isEmpty())) {
                data.bookmark = b.name;
                if (!b.hidden) break;
            }
        kept << data;
    }
    *fields = kept;
}

} // namespace

QVector<FormField> formFields(QTextDocument *story)
{
    const StoryScan scan = scanStory(story, true);
    QVector<FormField> found = fieldsOf(scan);
    describe(story, scan, &found);
    return found;
}

FormField formFieldAt(QTextDocument *story, int position)
{
    const QVector<FormField> all = formFields(story);
    FormField best;
    // (A caret before the begin mark, or after the end mark, is outside the field.)
    for (const FormField &f : all)
        if (position > f.begin && position <= f.end && (!best.valid() || f.begin >= best.begin)) best = f;
    return best;
}

FormField formFieldAtSelection(QTextDocument *story, int from, int to)
{
    FormField f = formFieldAt(story, from);
    // (The begin mark alone, selected, is the field: it is what shows a field with nothing in it.)
    if (!f.valid() && to == from + 1) f = formFieldBeginningAt(story, from);
    return f;
}

bool formStructureCut(QTextDocument *story, int from, int to)
{
    if (to <= from) return false;
    {
        // (A field's marks are objects: a range with none in it takes no part of a field's structure.)
        QTextCursor look(story);
        look.setPosition(from);
        look.setPosition(to, QTextCursor::KeepAnchor);
        if (!look.selectedText().contains(QChar::ObjectReplacementCharacter)) return false;
    }
    for (const FormField &f : formFields(story)) {
        const int head = (f.separate >= 0 ? f.separate : f.end - 1) + 1;   // the begin mark, the code, and the separator end here
        const bool touchesHead = from < head && to > f.begin, touchesEnd = from < f.end + 1 && to > f.end;
        if (!touchesHead && !touchesEnd) continue;
        if (!(from <= f.begin && to >= f.end + 1)) return true;   // (some of it and not all: the field's marks are never taken piece by piece)
    }
    return false;
}

namespace {

QString titleCased(const QString &s, bool everyWord)
{
    QString out = s.toLower();
    bool start = true;
    for (QChar &c : out) {
        if (start && c.isLetter()) {
            c = c.toUpper();
            start = false;
        } else if (!c.isLetterOrNumber() && everyWord) {
            start = true;
        } else if (c.isLetter()) {
            start = false;
        }
    }
    return out;
}

} // namespace

QString enteredFormText(const FormField &field, const QString &typedIn)
{
    QString text = typedIn.left(kMostFormChars);
    if (field.textType == QLatin1String("number")) {
        QString kept;
        for (const QChar c : std::as_const(text))
            if (c.isDigit() || QStringLiteral(".,-+%$() ").contains(c)) kept += c;
        text = kept;
    } else if (field.textType == QLatin1String("regular")) {
        const QString format = field.format.trimmed().toLower();
        if (format == QLatin1String("uppercase")) text = text.toUpper();
        else if (format == QLatin1String("lowercase")) text = text.toLower();
        else if (format == QLatin1String("first capital")) text = titleCased(text, false);
        else if (format == QLatin1String("title case")) text = titleCased(text, true);
    }
    if (field.maxLength > 0 && text.size() > field.maxLength) {
        int keep = field.maxLength;
        if (keep > 0 && text.at(keep - 1).isHighSurrogate()) --keep;   // (a pair is kept whole or not at all)
        text = text.left(keep);
    }
    return text;
}

QString finishedFormText(const FormField &field, const QString &typed, bool *ok)
{
    if (ok) *ok = true;
    const QString text = typed.trimmed();
    if (text.isEmpty() || field.format.trimmed().isEmpty()) return typed;
    if (field.textType == QLatin1String("number")) {
        QString digits = text;
        const bool negative = digits.startsWith(QLatin1Char('-')) || (digits.startsWith(QLatin1Char('(')) && digits.endsWith(QLatin1Char(')')));
        const bool percent = digits.endsWith(QLatin1Char('%'));
        digits.remove(QRegularExpression(QStringLiteral("[,$()%\\s+-]")));
        bool good = false;
        double value = digits.toDouble(&good);
        if (!good) {
            if (ok) *ok = false;
            return typed;
        }
        if (negative) value = -value;
        if (percent) value /= 100;
        return fields::numberInPicture(value, field.format);
    }
    if (field.textType == QLatin1String("date")) {
        static const char *const kinds[] = {"M/d/yyyy", "M/d/yy", "yyyy-MM-dd", "MMMM d, yyyy", "MMM d, yyyy", "d MMMM yyyy", "d MMM yyyy", "dddd, MMMM d, yyyy"};
        QDate date;
        for (const char *k : kinds)
            if (!date.isValid()) date = QLocale::c().toDate(text, QString::fromLatin1(k));
        if (!date.isValid()) {
            if (ok) *ok = false;
            return typed;
        }
        return fields::dateInPicture(QDateTime(date, QTime(0, 0)), field.format, QLocale::c());
    }
    return typed;
}

namespace {

// What a text field with this result shows: the result, or, where it has none, its default text.
QString shownOf(const FormField &f, const QString &result) { return result.isEmpty() ? f.defaultText.left(4000) : QString(); }

// The begin mark's data with a change: `change` is given the text of the element that holds the state and returns the new text (or none to refuse).
QByteArray withCheckedState(const QByteArray &raw, bool checked)
{
    static const QRegularExpression existing(QStringLiteral("<((?:[\\w.-]+:)?)checked\\b[^>]*?/>"));
    static const QRegularExpression existingPair(QStringLiteral("<((?:[\\w.-]+:)?)checked\\b[^>]*?>\\s*</(?:[\\w.-]+:)?checked>"));
    static const QRegularExpression close(QStringLiteral("</((?:[\\w.-]+:)?)checkBox>"));
    const QString text = QString::fromUtf8(raw);
    for (const QRegularExpression &re : {existing, existingPair}) {
        const QRegularExpressionMatch m = re.match(text);
        if (m.hasMatch()) {
            const QString prefix = m.captured(1);
            QString out = text;
            out.replace(m.capturedStart(), m.capturedLength(), checked ? QStringLiteral("<%1checked/>").arg(prefix) : QStringLiteral("<%1checked %1val=\"0\"/>").arg(prefix));
            return out.toUtf8();
        }
    }
    const QRegularExpressionMatch c = close.match(text);
    if (!c.hasMatch()) return QByteArray();
    const QString prefix = c.captured(1);
    QString out = text;
    out.insert(c.capturedStart(), checked ? QStringLiteral("<%1checked/>").arg(prefix) : QStringLiteral("<%1checked %1val=\"0\"/>").arg(prefix));
    return out.toUtf8();
}

QByteArray withChosenEntry(const QByteArray &raw, int index)
{
    static const QRegularExpression existing(QStringLiteral("<((?:[\\w.-]+:)?)result\\b[^>]*?/>"));
    static const QRegularExpression open(QStringLiteral("<((?:[\\w.-]+:)?)ddList\\b[^>]*?>"));
    const QString text = QString::fromUtf8(raw);
    const QRegularExpressionMatch m = existing.match(text);
    QString out = text;
    if (m.hasMatch()) {
        out.replace(m.capturedStart(), m.capturedLength(), QStringLiteral("<%1result %1val=\"%2\"/>").arg(m.captured(1)).arg(index));
        return out.toUtf8();
    }
    const QRegularExpressionMatch o = open.match(text);
    if (!o.hasMatch() || o.captured(0).endsWith(QLatin1String("/>"))) return QByteArray();
    out.insert(o.capturedEnd(), QStringLiteral("<%1result %1val=\"%2\"/>").arg(o.captured(1)).arg(index));
    return out.toUtf8();
}

// The field that begins at a place, described; none valid where there is none, or the field says nothing of what it is.
FormField fieldBeginningAt(QTextDocument *story, int begin, StoryScan *scanOut = nullptr)
{
    StoryScan scan = scanStory(story, true);
    QVector<FormField> found = fieldsOf(scan);
    describe(story, scan, &found);
    FormField out;
    for (const FormField &f : std::as_const(found))
        if (f.begin == begin) out = f;
    if (scanOut) *scanOut = scan;
    return out;
}

// The references to a field's bookmark in the text are worked out again (the other fields are left as they are).
void updateReferencesTo(WriteDoc &doc, QTextDocument *story, const StoryScan &scan, const FormField &field)
{
    if (story != doc.story() || field.bookmark.isEmpty()) return;
    bool any = false;
    for (int i = 0; i < scan.fields.size() && !any; ++i) {
        if (scan.fields[i].name != QLatin1String("REF")) continue;
        any = scan.instructionOf(i).contains(field.bookmark, Qt::CaseInsensitive);
    }
    if (!any) return;
    UpdateOptions options;
    options.skip = [](const QString &name) { return name != QLatin1String("REF"); };
    updateFields(doc, options);
}

// What replaces a result's characters: the look of its first character, else of its separator.
QTextCharFormat resultLook(QTextDocument *story, const FormField &f)
{
    QTextCharFormat look;
    bool found = false;
    for (int p = f.separate + 1; p < f.end && !found; ++p) {
        const QTextCharFormat cf = formatAt(story, p);
        if (!cf.hasProperty(prop::Atom)) {
            look = cf;
            found = true;
        }
    }
    if (!found) look = formatAt(story, f.separate);
    look.clearProperty(prop::Atom);
    look.clearProperty(tp::Field);
    look.clearProperty(tp::LinkResult);
    look.clearProperty(tp::BlankFieldResult);
    look.clearProperty(tp::FieldCodeText);
    look.clearProperty(tp::DisplayFamily);
    return look;
}

} // namespace

FormField formFieldBeginningAt(QTextDocument *story, int begin) { return fieldBeginningAt(story, begin); }

bool fillFormText(WriteDoc &doc, QTextDocument *story, int begin, const QString &text, bool leaving, QString *shown, bool joinPrevious)
{
    StoryScan scan;
    const FormField f = fieldBeginningAt(story, begin, &scan);
    if (!f.valid() || f.kind != FormKind::Text || !f.fillable() || f.separate < 0) return false;
    QString entered = enteredFormText(f, text);
    if (leaving) entered = finishedFormText(f, entered);
    if (shown) *shown = entered;
    if (entered == f.result) return false;
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    const QTextCharFormat look = resultLook(story, f);
    QTextCursor edit(story);
    if (joinPrevious) edit.joinPreviousEditBlock();
    else edit.beginEditBlock();
    QTextCursor c(story);
    c.setPosition(f.separate + 1);
    c.setPosition(f.end, QTextCursor::KeepAnchor);
    if (c.hasSelection()) c.removeSelectedText();
    // (A line break in what a field holds is a break in its line, as it was read; never a new paragraph.)
    if (!entered.isEmpty()) c.insertText(QString(entered).replace(QLatin1Char('\n'), QChar(QChar::LineSeparator)), look);
    // (What the begin mark shows: the default text where the result has none, else nothing.)
    Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
    atom.display = shownOf(f, entered);
    putAtom(story, f.begin, atom);
    edit.endEditBlock();
    updateReferencesTo(doc, story, scanStory(story, true), f);
    return true;
}

bool setFormChecked(WriteDoc &doc, QTextDocument *story, int begin, bool checked)
{
    const FormField f = fieldBeginningAt(story, begin);
    if (!f.valid() || f.kind != FormKind::CheckBox || !f.fillable() || f.checked == checked) return false;
    Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
    const QByteArray raw = withCheckedState(atom.raw, checked);
    if (raw.isEmpty()) return false;
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    atom.raw = raw;
    atom.display = QString(QChar(checked ? 0x2612 : 0x2610));
    putAtom(story, f.begin, atom);
    edit.endEditBlock();
    return true;
}

bool chooseFormEntry(WriteDoc &doc, QTextDocument *story, int begin, int index)
{
    StoryScan scan;
    const FormField f = fieldBeginningAt(story, begin, &scan);
    if (!f.valid() || f.kind != FormKind::DropDown || !f.fillable() || index < 0 || index >= f.entries.size() || f.chosen == index) return false;
    Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
    const QByteArray raw = withChosenEntry(atom.raw, index);
    if (raw.isEmpty()) return false;
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    atom.raw = raw;
    atom.display = f.entries.at(index);
    putAtom(story, f.begin, atom);
    edit.endEditBlock();
    updateReferencesTo(doc, story, scanStory(story, true), f);
    return true;
}

FormField nextFormField(QTextDocument *story, int from, bool backward)
{
    QVector<FormField> all;
    for (const FormField &f : formFields(story))
        if (f.fillable()) all << f;
    if (all.isEmpty()) return FormField();
    if (!backward) {
        for (const FormField &f : std::as_const(all))
            if (f.begin > from) return f;
        return all.first();
    }
    for (int i = int(all.size()) - 1; i >= 0; --i)
        if (all[i].begin < from) return all[i];
    return all.last();
}

void selectFormField(QTextCursor &c, const FormField &field)
{
    if (!field.valid()) return;
    if (field.kind == FormKind::Text && field.separate >= 0) {
        c.setPosition(field.separate + 1);
        c.setPosition(field.end, QTextCursor::KeepAnchor);
        // (A field with nothing in it, its default text showing: that is what is selected.)
        if (field.end == field.separate + 1) {
            c.setPosition(field.begin);
            c.setPosition(field.begin + 1, QTextCursor::KeepAnchor);
        }
    } else {
        c.setPosition(field.begin + 1);   // (inside the field: after its begin mark)
    }
}

QVector<bool> formsOpenSections(WriteDoc &doc)
{
    static const QRegularExpression off(QStringLiteral("^<(?:[\\w.-]+:)?formProt\\b[^>]*?[\\s:]val\\s*=\\s*\"(?:0|false|off)\""));
    QVector<bool> open;
    for (int i = 0; i < std::max(1, doc.sectionCount()); ++i) {
        bool left = false;
        for (const RawChild &child : doc.sectionProps(i).extra)
            if (off.match(QString::fromUtf8(child.xml.left(200))).hasMatch()) left = true;
        open << left;
    }
    return open;
}

bool formsRangeOpen(WriteDoc &doc, QTextDocument *story, int from, int to, const QVector<bool> *sections)
{
    if (story != doc.story()) return false;
    const bool given = sections && !sections->isEmpty();
    const QVector<bool> own = given ? QVector<bool>() : formsOpenSections(doc);
    const QVector<bool> &open = given ? *sections : own;
    if (to < from) std::swap(from, to);
    if (open.size() == 1) return open[0];
    const int first = doc.sectionAt(from), last = to > from ? doc.sectionAt(to) : first;
    for (int i = first; i <= last && i < open.size(); ++i)
        if (!open[i]) return false;
    return true;
}

bool formsSectionOpenAt(WriteDoc &doc, int position) { return formsRangeOpen(doc, doc.story(), position, position); }

bool formsEditableAt(WriteDoc &doc, QTextDocument *story, int position)
{
    if (story == doc.story() && formsSectionOpenAt(doc, position)) return true;
    const FormField f = formFieldAt(story, position);
    return f.valid() && f.fillable();
}

} // namespace jp::write
