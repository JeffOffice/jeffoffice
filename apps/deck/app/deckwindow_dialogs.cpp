// JeffDeck's commands that open the dialog boxes it shares with the other
// apps (Font, Paragraph, Bullets and Numbering, Symbol, Link, Date and Time,
// Spelling, Thesaurus, Word Count, Options), and its own page of Options.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/optionsdialog.h"
#include "app/proofingdialogs.h"
#include "app/settings.h"
#include "app/textdialogs.h"

#include <QAction>
#include <QCheckBox>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QVBoxLayout>
#include <memory>
#include "app/plaintext.h"

namespace jp {

namespace {
// Word Count for a presentation: what is typed on the slides. The layouts'
// prompts and the speaker notes are text too, but not what the slides say.
void slideWordCount(QWidget *p, Editor *ed)
{
    Document *d = ed->doc();
    QSet<QString> seen;
    int words = 0, chars = 0, charsNoSpace = 0, paragraphs = 0;
    auto count = [&](const QString &storyId) {
        const QTextDocument *sd = storyId.isEmpty() || seen.contains(storyId) ? nullptr : d->storyDoc(storyId);
        seen.insert(storyId);
        if (!sd) return;
        const QString text = sd->toPlainText();
        words += text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
        chars += text.size();
        charsNoSpace += QString(text).remove(QRegularExpression("\\s")).size();
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next())
            if (!b.text().trimmed().isEmpty()) ++paragraphs;
    };
    for (const auto &slide : d->pages)
        walkItems(slide->items, [&](const ItemPtr &it) {
            if (it->type() == ItemType::Text) count(static_cast<TextItem *>(it.get())->storyId);
            else if (it->type() == ItemType::Shape) count(static_cast<ShapeItem *>(it.get())->storyId);
            else if (it->type() == ItemType::Table)
                for (const TableCell &c : static_cast<TableItem *>(it.get())->cells) {
                    count(c.storyId);
                    count(c.storyId2);
                }
        });
    QString sel;
    if (ed->isEditingText() && ed->cursor().hasSelection())
        sel = QCoreApplication::translate("jp::DeckWindow", "<br><br><b>Selection</b><br>Words: %1").arg(ed->cursor().selectedText().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size());
    jp::plainInformation(p, QCoreApplication::translate("jp::DeckWindow", "Word Count"),
                             QCoreApplication::translate("jp::DeckWindow", "<b>Presentation</b><br>Slides: %1<br>Words: %2<br>Characters (no spaces): %3<br>Characters (with spaces): %4"
                                                                       "<br>Paragraphs: %5%6")
                                 .arg(d->pages.size()).arg(words).arg(charsNoSpace).arg(chars).arg(paragraphs).arg(sel));
}
} // namespace

void DeckWindow::createDialogActions()
{
    Editor *ed = m_ed;
    mk("fmt.fontDialog", tr("Font…"), "type", QKeySequence(Qt::CTRL | Qt::Key_T), [this, ed] { fontDialog(this, ed); });
    mk("fmt.spacingDialog", tr("More Spacing…"), "", QKeySequence(), [this, ed] { characterSpacingDialog(this, ed); });
    mk("para.dialog", tr("Paragraph…"), "pilcrow", QKeySequence(), [this, ed] { paragraphDialog(this, ed, 0); });
    mk("para.bulletsDialog", tr("Bullets and Numbering…"), "list-plus", QKeySequence(), [this, ed] { bulletsDialog(this, ed, false); });
    mk("para.numberingDialog", tr("Bullets and Numbering…"), "list-plus", QKeySequence(), [this, ed] { bulletsDialog(this, ed, true); });
    mk("insert.symbol", tr("Symbol"), "omega", QKeySequence(), [this, ed] { symbolDialog(this, ed); });
    mk("insert.link", tr("Link"), "link", QKeySequence(Qt::CTRL | Qt::Key_K), [this, ed] { hyperlinkDialog(this, ed); });
    mk("insert.dateTime", tr("Date and Time"), "calendar-clock", QKeySequence(), [this, ed] { dateTimeDialog(this, ed); });
    mk("review.spelling", tr("Spelling"), "spell-check", QKeySequence(Qt::Key_F7), [this, ed] { spellingDialog(this, ed); });
    mk("review.thesaurus", tr("Thesaurus"), "book-a", QKeySequence(Qt::SHIFT | Qt::Key_F7), [this, ed] { thesaurusDialog(this, ed); });
    mk("review.wordCount", tr("Word Count"), "whole-word", QKeySequence(), [this, ed] { slideWordCount(this, ed); });

    // Options: the shared General and Proofing pages, then the View page, which
    // sets how every window opens (and the open ones, now).
    mk("file.options", tr("Options"), "settings", QKeySequence(), [this] {
        struct Fields {
            QCheckBox *ruler = nullptr, *snap = nullptr, *embedFonts = nullptr;
        };
        auto f = std::make_shared<Fields>();
        QVector<OptionsPage> pages;
        pages << OptionsPage{tr("View"), [this, f] {
            auto *page = new QWidget();
            auto *column = new QVBoxLayout(page);
            f->ruler = new QCheckBox(tr("Show the ruler"), page);
            f->ruler->setObjectName(QStringLiteral("showRuler"));
            f->ruler->setChecked(m_ed->view.rulers);
            f->snap = new QCheckBox(tr("Snap objects to other objects"), page);
            f->snap->setObjectName(QStringLiteral("snapObjects"));
            f->snap->setChecked(m_ed->view.snapObjects);
            column->addWidget(f->ruler);
            column->addWidget(f->snap);
            column->addStretch(1);
            return page;
        }, [f] {
            Settings::get().setValue(QStringLiteral("view/rulers"), f->ruler->isChecked());
            Settings::get().setValue(QStringLiteral("view/snapObjects"), f->snap->isChecked());
            for (AppWindow *w : AppWindow::windows())
                if (auto *deck = qobject_cast<DeckWindow *>(w)) {
                    deck->m_ed->view.rulers = f->ruler->isChecked();
                    deck->m_canvas->setRulersVisible(f->ruler->isChecked());
                    deck->m_ed->view.snapObjects = f->snap->isChecked();
                    deck->refreshUi();
                }
        }};
        // Save: what goes in the file besides the slides. The fonts are the ones that come with the program, which other computers lack;
        // the ones every computer has (Calibri, Arial) are never put in.
        pages << OptionsPage{tr("Save"), [f] {
            auto *page = new QWidget();
            auto *column = new QVBoxLayout(page);
            f->embedFonts = new QCheckBox(tr("Embed fonts in the file"), page);
            f->embedFonts->setObjectName(QStringLiteral("embedFonts"));
            const QString about = tr("Puts the fonts that come with the program, and that the slides use, in the file, so the slides look the same on a computer that doesn't have them. "
                                     "It makes the file larger. Fonts every computer has, such as Calibri and Arial, are never put in.");
            f->embedFonts->setToolTip(jp::plainToolTip(about));
            f->embedFonts->setWhatsThis(jp::plainToolTip(about));
            f->embedFonts->setAccessibleDescription(about);
            f->embedFonts->setChecked(Settings::get().value(QStringLiteral("save/embedFonts"), true).toBool());
            column->addWidget(f->embedFonts);
            column->addStretch(1);
            return page;
        }, [f] { Settings::get().setValue(QStringLiteral("save/embedFonts"), f->embedFonts->isChecked()); }};
        optionsDialog(this, m_ed, pages);
    });

    // A window opens as the View page left it.
    m_ed->view.rulers = Settings::get().value(QStringLiteral("view/rulers"), false).toBool();
    m_canvas->setRulersVisible(m_ed->view.rulers);
    m_ed->view.snapObjects = Settings::get().value(QStringLiteral("view/snapObjects"), true).toBool();
}

} // namespace jp
