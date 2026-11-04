// JeffWrite's Layout and Design tabs, the parts beyond the first presets: mirrored margins, more columns, hyphenation, line numbers, the page's color, and the page's borders
// (the work on the document is core/layoutops.h and the document's own settings; the dialogs are app/layoutdialogs.cpp).

#include "app/writewindow.h"

#include "app/icons.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/widgets.h"
#include "app/writedialogs.h"
#include "core/designops.h"
#include "core/layoutops.h"
#include "core/watermarkops.h"

#include <QAction>
#include <QMenu>

namespace jp {

void WriteWindow::createLayoutExtraActions()
{
    auto sectionNow = [this] { return m_view->cursorSection(); };
    // Margins: Mirrored (the inside is wider than the outside, on facing pages), and the document's mirror setting itself.
    mk("layout.margins.mirrored", tr("Mirrored"), "", QKeySequence(), [this] {
        if (!m_doc) return;
        const write::WriteDoc::StepGroup group(*m_doc);
        changeSection([](flow::Section &s) { s.margins = QMarginsF(90, 72, 72, 72); });
        m_doc->setMirrorMargins(true);
        refreshUi();
    });
    mk("layout.mirrorMargins", tr("Mirror Margins"), "", QKeySequence(), [this] {
        if (m_doc) m_doc->setMirrorMargins(act("layout.mirrorMargins")->isChecked());
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("layout.customMargins", tr("Custom Margins…"), "settings-2", QKeySequence(), [this] { showPageSetupDialog(); });
    mk("layout.moreSizes", tr("More Paper Sizes…"), "settings-2", QKeySequence(), [this] { showPageSetupDialog(); });
    // Columns: the narrow one on the left or the right, and the dialog.
    for (const bool left : {true, false})
        mk(left ? QStringLiteral("layout.columns.left") : QStringLiteral("layout.columns.right"), left ? tr("Left") : tr("Right"), left ? "panel-left" : "panel-right", QKeySequence(), [this, left] {
            changeSection([left](flow::Section &s) {
                const double text = std::max(36.0, s.content().width() - 36);
                s.columns = 2;
                s.columnGap = 36;
                s.columnWidths = {text * (left ? 1.0 / 3 : 2.0 / 3), text * (left ? 2.0 / 3 : 1.0 / 3)};
                s.columnSpaces = {36, 0};
            });
        }, true);
    mk("layout.columns.more", tr("More Columns…"), "columns-3", QKeySequence(), [this, sectionNow] {
        if (!m_doc) return;
        write::columnsDialog(this, m_doc.get(), sectionNow());
        m_view->setFocus(Qt::OtherFocusReason);
    });
    // Hyphenation: none, or automatic (the document's setting; the words are hyphenated by the bundled dictionary of their language).
    mk("layout.hyphenation.none", tr("None"), "", QKeySequence(), [this] {
        if (m_doc) m_doc->setAutoHyphenation(false);
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("layout.hyphenation.auto", tr("Automatic"), "", QKeySequence(), [this] {
        if (m_doc) m_doc->setAutoHyphenation(true);
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    // Line numbers.
    struct Numbers {
        const char *id;
        const char *text;
        int mode;   // -1 none, else the restart
    };
    static const Numbers numbers[] = {{"none", QT_TR_NOOP("None"), -1}, {"continuous", QT_TR_NOOP("Continuous"), 2}, {"eachPage", QT_TR_NOOP("Restart Each Page"), 0}, {"eachSection", QT_TR_NOOP("Restart Each Section"), 1}};
    for (const Numbers &n : numbers) {
        const int mode = n.mode;
        mk(QStringLiteral("layout.lineNumbers.") + QLatin1String(n.id), QCoreApplication::translate("jp::WriteWindow", n.text), "", QKeySequence(), [this, mode, sectionNow] {
            if (!m_doc) return;
            write::applyToSections(*m_doc, sectionNow(), false, [mode](office::wml::SectionProps *p) {
                write::LineNumberSetup s = write::lineNumbersOf(*p);
                if (mode < 0) {
                    s.countBy = 0;
                } else {
                    if (s.countBy <= 0) s.countBy = 1;
                    s.restart = write::LineNumberSetup::Restart(mode);
                }
                write::setLineNumbers(p, s);
            });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    mk("layout.lineNumbers.options", tr("Line Numbering Options…"), "list-ordered", QKeySequence(), [this, sectionNow] {
        if (!m_doc) return;
        write::lineNumbersDialog(this, m_doc.get(), sectionNow());
        m_view->setFocus(Qt::OtherFocusReason);
    });
    // Design: the page's color and borders.
    mk("design.pageColor.none", tr("No Color"), "circle-slash", QKeySequence(), [this] {
        if (m_doc) m_doc->setPageColor(QColor());
        m_view->setFocus(Qt::OtherFocusReason);
    });
    // Paragraph Spacing: the ready-made spacings.
    for (const write::SpacingPreset &preset : write::spacingPresets()) {
        const QString id = preset.id;
        mk(QStringLiteral("design.spacing.") + id, write::spacingPresetName(id), "", QKeySequence(), [this, id] {
            if (!m_doc || refuseIfReadOnly()) return;
            write::applyParagraphSpacing(*m_doc, id);
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    // Watermark: the built-in words, custom words, and taking it out.
    {
        const QStringList words = write::builtInWatermarks();
        for (int i = 0; i < words.size(); ++i) {
            const QString text = words.at(i);
            mk(QStringLiteral("design.watermark.%1").arg(i), text, "", QKeySequence(), [this, text] {
                if (!m_doc || refuseIfReadOnly()) return;
                write::WatermarkSetup w;
                w.text = text;
                write::insertWatermark(*m_doc, w);
                m_view->setFocus(Qt::OtherFocusReason);
            });
        }
    }
    mk("design.watermark.custom", tr("Custom Watermark…"), "stamp", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        write::watermarkDialog(this, m_doc.get());
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("design.watermark.remove", tr("Remove Watermark"), "circle-slash", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        write::removeWatermark(*m_doc);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("design.pageBorders", tr("Page Borders…"), "square-dashed", QKeySequence(), [this, sectionNow] {
        if (!m_doc) return;
        write::pageBordersDialog(this, m_doc.get(), sectionNow());
        m_view->setFocus(Qt::OtherFocusReason);
    });
}

void WriteWindow::layoutRibbonParts(RibbonParts &parts)
{
    parts.widgets[QStringLiteral("pageColor")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("paint-bucket"), tr("Page Color"), true, tr("No Color"));
        b->setCurrent(ColorRef::none());
        connect(b, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
            if (!m_doc || refuseIfReadOnly()) return;
            m_doc->setPageColor(c.isNone() ? QColor() : c.resolve(m_doc->flow().env.colors));
            m_view->setFocus(Qt::OtherFocusReason);
        });
        m_pageColorButtons << b;
        return b;
    };
}

void WriteWindow::refreshLayoutUi()
{
    act("design.pageBorders")->setEnabled(m_doc != nullptr);
    for (int i = 0; i < write::builtInWatermarks().size(); ++i) act(QStringLiteral("design.watermark.%1").arg(i))->setEnabled(m_doc != nullptr);
    {
        const QString now = m_doc ? write::paragraphSpacingOf(*m_doc) : QString();
        for (const write::SpacingPreset &preset : write::spacingPresets()) {
            act(QStringLiteral("design.spacing.") + preset.id)->setEnabled(m_doc != nullptr);
            act(QStringLiteral("design.spacing.") + preset.id)->setChecked(m_doc && preset.id == now);
        }
    }
    act("design.watermark.custom")->setEnabled(m_doc != nullptr);
    act("design.watermark.remove")->setEnabled(m_doc != nullptr && write::watermarkOf(*m_doc).has_value());
    act("design.pageColor.none")->setEnabled(m_doc && m_doc->pageColor().isValid());
    if (!m_doc) {
        for (const char *id : {"layout.mirrorMargins", "layout.hyphenation.none", "layout.hyphenation.auto"}) act(QString::fromLatin1(id))->setChecked(false);
        return;
    }
    act("layout.mirrorMargins")->setChecked(m_doc->mirrorMargins());
    act("layout.hyphenation.auto")->setChecked(m_doc->autoHyphenation());
    act("layout.hyphenation.none")->setChecked(!m_doc->autoHyphenation());
    const int section = m_view->cursorSection();
    const office::wml::SectionProps props = m_doc->sectionProps(section);
    const write::LineNumberSetup numbers = write::lineNumbersOf(props);
    act("layout.lineNumbers.none")->setChecked(numbers.countBy <= 0);
    act("layout.lineNumbers.continuous")->setChecked(numbers.countBy > 0 && numbers.restart == write::LineNumberSetup::Restart::Continuous);
    act("layout.lineNumbers.eachPage")->setChecked(numbers.countBy > 0 && numbers.restart == write::LineNumberSetup::Restart::EachPage);
    act("layout.lineNumbers.eachSection")->setChecked(numbers.countBy > 0 && numbers.restart == write::LineNumberSetup::Restart::EachSection);
    const flow::Section s = m_doc->flow().section(section);
    const bool unequal = !s.columnWidths.isEmpty() && s.columns == 2;
    act("layout.columns.left")->setChecked(unequal && s.columnWidths[0] < s.columnWidths[1]);
    act("layout.columns.right")->setChecked(unequal && s.columnWidths[0] > s.columnWidths[1]);
    for (int n = 1; n <= 3; ++n) act(QStringLiteral("layout.columns.%1").arg(n))->setChecked(s.columns == n && s.columnWidths.isEmpty());
    for (ColorButton *b : m_pageColorButtons) {
        b->setScheme(m_doc->flow().env.colors);
        b->setCurrent(m_doc->pageColor().isValid() ? ColorRef::rgb(m_doc->pageColor()) : ColorRef::none());
    }
}

} // namespace jp
