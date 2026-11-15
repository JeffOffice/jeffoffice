#include "text/fontreplace.h"


#include "text/textprops.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QVector>
#include <utility>

namespace jp {

QStringList fontFamiliesOf(const QTextCharFormat &format)
{
    if (format.hasProperty(QTextFormat::FontFamilies)) return format.fontFamilies().toStringList();
    if (format.hasProperty(QTextFormat::FontFamily)) return QStringList{format.property(QTextFormat::FontFamily).toString()};
    return {};
}

bool replaceFontFamilyIn(QTextCharFormat *format, const QString &from, const QString &to)
{
    bool changed = false;
    if (format->hasProperty(QTextFormat::FontFamilies)) {
        QStringList families = format->fontFamilies().toStringList();
        for (QString &family : families)
            if (family.compare(from, Qt::CaseInsensitive) == 0 && family != to) {
                family = to;
                changed = true;
            }
        if (changed) format->setFontFamilies(families);
    }
    if (format->hasProperty(QTextFormat::FontFamily)) {
        const QString family = format->property(QTextFormat::FontFamily).toString();
        if (family.compare(from, Qt::CaseInsensitive) == 0 && family != to) {
            format->setProperty(QTextFormat::FontFamily, to);
            changed = true;
        }
    }
    return changed;
}

void collectFontFamilies(const QTextDocument *doc, QMap<QString, int> *uses, int *major, int *minor)
{
    auto note = [&](const QTextCharFormat &f) {
        const QStringList families = fontFamiliesOf(f);
        if (!families.isEmpty() && !families.first().isEmpty()) {
            ++(*uses)[families.first()];
            return;
        }
        const QString theme = f.stringProperty(tp::ThemeFont);
        if (theme == QLatin1String("major") && major) ++*major;
        else if (theme == QLatin1String("minor") && minor) ++*minor;
    };
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        note(b.charFormat());
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) note(it.fragment().charFormat());
    }
}

int replaceFontFamily(QTextDocument *doc, const QString &from, const QString &to)
{
    struct Change {
        int begin, end;
        QTextCharFormat format;
        bool mark;
    };
    QVector<Change> changes;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        QTextCharFormat mark = b.charFormat();
        if (replaceFontFamilyIn(&mark, from, to)) changes.push_back({b.position(), b.position(), mark, true});
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fragment = it.fragment();
            QTextCharFormat f = fragment.charFormat();
            if (replaceFontFamilyIn(&f, from, to)) changes.push_back({fragment.position(), fragment.position() + fragment.length(), f, false});
        }
    }
    if (changes.isEmpty()) return 0;
    QTextCursor edit(doc);
    edit.beginEditBlock();
    for (const Change &c : std::as_const(changes)) {
        QTextCursor cursor(doc);
        cursor.setPosition(c.begin);
        if (c.mark) {
            cursor.mergeBlockCharFormat(c.format);
            continue;
        }
        cursor.setPosition(c.end, QTextCursor::KeepAnchor);
        // (Only the families are put in: whatever else the run's format says stays.)
        QTextCharFormat only;
        if (c.format.hasProperty(QTextFormat::FontFamilies)) only.setFontFamilies(c.format.fontFamilies().toStringList());
        if (c.format.hasProperty(QTextFormat::FontFamily)) only.setProperty(QTextFormat::FontFamily, c.format.property(QTextFormat::FontFamily));
        cursor.mergeCharFormat(only);
    }
    edit.endEditBlock();
    return int(changes.size());
}

} // namespace jp
