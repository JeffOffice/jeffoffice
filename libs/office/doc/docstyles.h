#pragma once
// The style sheet of an old binary document (STSH): its styles with their properties and the styles they are based on, written as the
// styles part of a .docx. Internal to libs/office/doc.

#include "doc/docprops.h"

namespace jp::office::doc {

struct StyleDef {
    bool valid = false;
    int sti = 0;              // the style's number among the built-in ones (0xFFE: made by the person)
    int sgc = 0;              // 1 paragraph, 2 character, 3 table, 4 list
    int base = -1, next = -1;
    QString name, id;
    Pap pap;
    Chp chp;
    Tap tap;                  // a table style's
    quint8 settled[TogCount] = {};   // the toggles the style comes to, with those of the styles it is based on
    bool settledDone = false;
};

class StyleSheet {
public:
    QVector<StyleDef> styles;
    int ftcAscii = -1, ftcFE = -1, ftcOther = -1, ftcBi = -1;   // the document's default fonts

    // Reads the style sheet from the table stream. False if there are none.
    bool read(const Span &stsh);
    // The id the XML refers to a style by; empty for a style that isn't there.
    QString idOf(int istd) const { return istd >= 0 && istd < styles.size() && styles[istd].valid ? styles[istd].id : QString(); }
    const StyleDef *style(int istd) const { return istd >= 0 && istd < styles.size() && styles[istd].valid ? &styles[istd] : nullptr; }
    // The toggles a run has from its paragraph style and its character style (to settle the changes stated against them).
    void runParent(int paragraphStyle, int characterStyle, quint8 *out) const;
    // The styles part.
    QByteArray xml(const FontNames &fonts, int listInstances) const;

private:
    void settle(int istd, int depth) const;
};

} // namespace jp::office::doc
