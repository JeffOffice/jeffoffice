// The Hyphenate command (Liang patterns, by language). Spelling and the
// thesaurus are in libs/page/app/proofingdialogs.cpp.

#include "app/appfuncs.h"
#include "app/spelling.h"
#include "text/hyphenation.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>

namespace jp {

// ---------------- hyphenation (Liang) ----------------
// Hyphenation patterns live in text/hyphenation so layout can use them too.

int hyphenateStory(QTextDocument *doc)
{
    int n = 0;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        const QString text = b.text();
        QVector<QPair<int, QString>> words;
        auto it = spellingWordPattern().globalMatch(text);
        while (it.hasNext()) {
            const auto m = it.next();
            if (m.captured().size() >= 6 && !m.captured().contains(QChar(0x00AD))) words << qMakePair(int(m.capturedStart()), m.captured());
        }
        for (int i = words.size() - 1; i >= 0; --i) {
            QTextCursor at(doc);
            at.setPosition(b.position() + words[i].first + 1);
            const QVector<int> pts = hyphenationPoints(words[i].second, at.charFormat().stringProperty(tp::Language));
            for (int k = pts.size() - 1; k >= 0; --k) {
                QTextCursor c(doc);
                c.setPosition(b.position() + words[i].first + pts[k]);
                c.insertText(QString(QChar(0x00AD)));
                ++n;
            }
        }
    }
    return n;
}

} // namespace jp
