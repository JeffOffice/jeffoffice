#pragma once
// Fonts a document uses that this computer lacks (the same in all four programs): a quiet bar when a document opens ("This document uses fonts that aren't on this computer") with a
// Details button; the dialog lists each missing font, what is drawn in its place, and where the document uses it, and lets the person choose another stand-in (shown, and written to PDF,
// but the file keeps the font's name) or put one in the document for good ("Replace in Document", which the program does as one change that undoes). The choices are kept for each font name,
// for every document. A program supplies only where its fonts are used (a FontUsage) and how it changes them (a MissingFontsHost).

#include "core/export.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <functional>

class QMainWindow;
class QTextDocument;
class QWidget;

namespace jp {

// Where a document uses each font: the family, and the places the person knows it by ("Body text", "Slide 3", "Sheet1").
class JO_COMMON_EXPORT FontUsage {
public:
    void add(const QString &family, const QString &place);
    // The families of the text of `doc` (the document's own font for text that names none), all in the place `place`.
    void addDocument(const QTextDocument *doc, const QString &place);
    QStringList families() const { return m_order; }
    QStringList places(const QString &family) const { return m_places.value(family); }
    bool isEmpty() const { return m_order.isEmpty(); }

private:
    QStringList m_order;
    QHash<QString, QStringList> m_places;
};

struct MissingFont {
    QString family;
    QString inPlace;       // what is drawn in its place now: the person's choice, the table's stand-in, or the system's pick
    QString automatic;     // what would be drawn without a choice
    bool chosen = false;   // inPlace is the person's choice
    QStringList places;
};

// Whether this computer has a font for the family (a test gives its own answer).
using FontInstalled = std::function<bool(const QString &family)>;
JO_COMMON_EXPORT bool fontIsInstalled(const QString &family);

// The fonts of `usage` that are not installed, in the order the document first uses them.
JO_COMMON_EXPORT QList<MissingFont> missingFonts(const FontUsage &usage, const FontInstalled &installed = {});

// The person's choice of a stand-in for a font (kept for every document and every run; empty when there is none), and the making of one. An empty replacement takes the choice away.
JO_COMMON_EXPORT QString fontChoice(const QString &family);
JO_COMMON_EXPORT void setFontChoice(const QString &family, const QString &replacement);
// Puts the remembered choices of these families in force (what a document that has just opened needs). True when one was.
JO_COMMON_EXPORT bool applyFontChoices(const QStringList &families);

struct MissingFontsHost {
    std::function<FontUsage()> usage;
    // Puts `to` in the document for every use of `from`, as one change that undoes; how many places changed, or -1 when the program refused (a read-only document).
    std::function<int(const QString &from, const QString &to)> replace;
    // Draws the document again (a choice changes the widths of the text).
    std::function<void()> refresh;
};

// Looks at the fonts of the document the window holds (after it opened): puts the remembered choices in force, and shows the bar at the top of the window when a font is missing, or takes it away.
JO_COMMON_EXPORT void checkMissingFonts(QMainWindow *win, const MissingFontsHost &host, const FontInstalled &installed = {});
// The dialog the bar's Details button opens. The families it offers are the installed ones.
JO_COMMON_EXPORT void showMissingFontsDialog(QWidget *parent, const MissingFontsHost &host, const FontInstalled &installed = {});

} // namespace jp
