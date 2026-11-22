#pragma once
// The lists a writer gives of what a kind of file leaves out (shown by Check Compatibility and by the question before a save) are bounded where they are built: a workbook with a
// thousand sheets or defined names must not make a thousand lines. A writer keeps the first kMaxLossNotes, counts the rest, and says how many were left off.

#include <QCoreApplication>
#include <QString>
#include <QStringList>

namespace jp {

constexpr int kMaxLossNotes = 100;

// Adds a note to the list unless the list is full, when it is counted in `dropped` instead.
inline void addLossNote(QStringList *notes, const QString &note, qint64 *dropped)
{
    if (notes->size() < kMaxLossNotes) notes->push_back(note);
    else ++*dropped;
}

// Cuts a list that is longer than the bound, counting what is cut in `dropped`.
inline void boundLossNotes(QStringList *notes, qint64 *dropped)
{
    if (notes->size() <= kMaxLossNotes) return;
    *dropped += notes->size() - kMaxLossNotes;
    notes->erase(notes->begin() + kMaxLossNotes, notes->end());
}

// The last line of a list that was cut, in the words the box uses.
inline QString moreLossNotes(qint64 dropped)
{
    return QCoreApplication::translate("jp::CompatibilityDialog", "And %n more.", nullptr, int(qMin<qint64>(dropped, 2000000000)));
}

} // namespace jp
