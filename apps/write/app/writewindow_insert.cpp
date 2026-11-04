// JeffWrite's Insert tab, the parts that are not pictures, tables, notes, or fields: symbols and special characters, and the pages and breaks (the work on the document is
// core/insertops.h and core/editops.h).

#include "app/writewindow.h"

#include "app/icons.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/textdialogs.h"
#include "core/editops.h"
#include "core/insertops.h"
#include "core/linkops.h"
#include "app/writedialogs.h"

#include <QAction>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStatusBar>
#include <QDesktopServices>
#include <QUrl>
#include <QGridLayout>
#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>

namespace jp {

void WriteWindow::createInsertActions()
{
    auto edit = [this](const std::function<void(write::WriteDoc &, QTextCursor &)> &fn) {
        if (!m_doc) return;
        fn(*m_doc, m_view->cursor());
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    };
    mk("insert.symbol", tr("Symbol…"), "omega", QKeySequence(), [this] {
        symbolDialog(this, [this](QWidget *, const QString &ch, const QString &font) {
            if (!m_doc || refuseIfReadOnly()) return false;
            write::insertSymbol(*m_doc, m_view->cursor(), ch, font);
            rememberSymbol(ch, font);
            return true;
        });
        m_view->setFocus(Qt::OtherFocusReason);
    });
    struct Special {
        const char *id;
        const char *text;
        ushort ch;
        const char *icon;
        Qt::Key key;
        int mods;
    };
    // (A tab and a paragraph mark are typed, not inserted from here.)
    static const Special specials[] = {
        {"nbsp", QT_TR_NOOP("Nonbreaking Space"), 0x00A0, "space", Qt::Key_Space, int(Qt::CTRL | Qt::SHIFT)},
        {"nbHyphen", QT_TR_NOOP("Nonbreaking Hyphen"), 0x2011, "minus", Qt::Key_Minus, int(Qt::CTRL | Qt::SHIFT)},
        {"optHyphen", QT_TR_NOOP("Optional Hyphen"), 0x00AD, "minus", Qt::Key_Minus, int(Qt::CTRL)},
        {"enDash", QT_TR_NOOP("En Dash"), 0x2013, "minus", Qt::Key(0), 0},
        {"emDash", QT_TR_NOOP("Em Dash"), 0x2014, "minus", Qt::Key(0), 0},
        {"enSpace", QT_TR_NOOP("En Space"), 0x2002, "space", Qt::Key(0), 0},
        {"emSpace", QT_TR_NOOP("Em Space"), 0x2003, "space", Qt::Key(0), 0},
        {"openDouble", QT_TR_NOOP("Opening Double Quotation Mark"), 0x201C, "quote", Qt::Key(0), 0},
        {"closeDouble", QT_TR_NOOP("Closing Double Quotation Mark"), 0x201D, "quote", Qt::Key(0), 0},
        {"openSingle", QT_TR_NOOP("Opening Single Quotation Mark"), 0x2018, "quote", Qt::Key(0), 0},
        {"closeSingle", QT_TR_NOOP("Closing Single Quotation Mark"), 0x2019, "quote", Qt::Key(0), 0},
        {"lineBreak", QT_TR_NOOP("Line Break"), 0x2028, "corner-down-left", Qt::Key(0), 0},
        {"noWidthBreak", QT_TR_NOOP("No-Width Optional Break"), 0x200B, "scissors", Qt::Key(0), 0},
        {"noWidthNoBreak", QT_TR_NOOP("No-Width Nonbreak"), 0x2060, "link", Qt::Key(0), 0},
    };
    for (const Special &s : specials) {
        const QString ch(QChar(s.ch));
        const QKeySequence key = s.key == Qt::Key(0) ? QKeySequence() : QKeySequence(s.mods | int(s.key));
        mk(QStringLiteral("insert.special.") + QLatin1String(s.id), QCoreApplication::translate("jp::WriteWindow", s.text), s.icon, key, [edit, ch] {
            edit([ch](write::WriteDoc &d, QTextCursor &c) { write::insertText(d, c, ch); });
        });
    }
    struct Cover {
        const char *id;
        const char *text;
        write::CoverDesign design;
    };
    static const Cover covers[] = {{"classic", QT_TR_NOOP("Classic"), write::CoverDesign::Classic}, {"band", QT_TR_NOOP("Band"), write::CoverDesign::Band},
                                   {"plain", QT_TR_NOOP("Plain"), write::CoverDesign::Plain}};
    for (const Cover &cover : covers) {
        const write::CoverDesign design = cover.design;
        mk(QStringLiteral("insert.cover.") + QLatin1String(cover.id), QCoreApplication::translate("jp::WriteWindow", cover.text), "file-text", QKeySequence(), [this, design] {
            if (!m_doc || refuseIfReadOnly()) return;
            QString why;
            m_view->leaveHeaderFooter();
            m_view->leaveNote();
            if (!write::insertCoverPage(*m_doc, design, &why)) statusBar()->showMessage(why, 8000);
            m_view->setFocus(Qt::OtherFocusReason);
        });
    }
    mk("insert.cover.remove", tr("Remove Current Cover Page"), "trash-2", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        write::removeCoverPage(*m_doc);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("insert.dateTime", tr("Date & Time…"), "calendar-clock", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        write::dateTimeDialog(this, m_doc.get(), m_view);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("insert.textFromFile", tr("Text from File…"), "file-input", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        QString path;
        if (m_textFileChooser) {
            path = m_textFileChooser();
        } else {
            const QString dir = Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
            path = QFileDialog::getOpenFileName(this, tr("Insert Text from File"), dir,
                                                tr("Documents and text (*.docx *.docm *.doc *.rtf *.odt *.txt)") + QStringLiteral(";;") + tr("All Files (*)"));
        }
        if (path.isEmpty()) return;
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly) || f.size() > write::kMostInsertedFile) {
            statusBar()->showMessage(tr("%1 can't be opened as text for the document.").arg(QFileInfo(path).fileName()), 8000);
            return;
        }
        QString why;
        if (!write::insertTextFromFile(*m_doc, m_view->cursor(), f.readAll(), &why)) statusBar()->showMessage(why, 8000);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("insert.hyperlink", tr("Link…"), "link", QKeySequence(Qt::CTRL | Qt::Key_K), [this] {
        if (!m_doc) return;
        write::linkDialog(this, m_doc.get(), m_view);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("insert.removeHyperlink", tr("Remove Link"), "unlink", QKeySequence(), [this] {
        if (!m_doc) return;
        write::removeHyperlink(*m_doc, m_view->cursor().position());
        m_view->setFocus(Qt::OtherFocusReason);
    });
    connect(m_view, &write::PageView::linkFollowed, this, [this](const QString &address, const QString &shown) {
        if (write::linkNeedsAsking(address, shown)) {
            const QString question = tr("Open %1?").arg(address);
            const bool yes = m_linkAsker ? m_linkAsker(question) : plainQuestion(this, tr("Open Link"), question) == QMessageBox::Yes;
            if (!yes) return;
        }
        if (m_linkOpener) m_linkOpener(address);
        else QDesktopServices::openUrl(QUrl(address, QUrl::TolerantMode));
    });
    mk("insert.blankPage", tr("Blank Page"), "file", QKeySequence(), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::insertBlankPage(d, c); }); });
    mk("layout.columnBreak", tr("Column"), "columns-2", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Return), [edit] {
        edit([](write::WriteDoc &d, QTextCursor &c) { write::insertText(d, c, QString(flow::kColumnBreak)); });
    });
    struct Kind {
        const char *id;
        const char *text;
        const char *type;
    };
    static const Kind kinds[] = {{"continuous", QT_TR_NOOP("Continuous"), "continuous"}, {"evenPage", QT_TR_NOOP("Even Page"), "evenPage"}, {"oddPage", QT_TR_NOOP("Odd Page"), "oddPage"}};
    for (const Kind &k : kinds) {
        const QString type = QString::fromLatin1(k.type);
        mk(QStringLiteral("layout.sectionBreak.") + QLatin1String(k.id), QCoreApplication::translate("jp::WriteWindow", k.text), "section", QKeySequence(),
           [edit, type] { edit([type](write::WriteDoc &d, QTextCursor &c) { write::insertSectionBreak(d, c, type); }); });
    }
}

void WriteWindow::insertRibbonParts(RibbonParts &parts)
{
    // Symbol: the symbols used lately to click, then More Symbols.
    parts.menus[QStringLiteral("symbolMenu")] = [this](QWidget *owner) {
        auto *sm = new QMenu(owner);
        connect(sm, &QMenu::aboutToShow, this, [this, sm] {
            sm->clear();
            const QStringList recents = recentSymbols();
            if (!recents.isEmpty()) {
                auto *panel = new QWidget(sm);
                auto *grid = new QGridLayout(panel);
                grid->setContentsMargins(6, 6, 6, 6);
                grid->setSpacing(2);
                for (int i = 0; i < recents.size() && i < 20; ++i) {
                    const QString ch = recents[i].section(QLatin1Char('\t'), 0, 0), family = recents[i].section(QLatin1Char('\t'), 1);
                    auto *b = new QToolButton(panel);
                    b->setText(ch);
                    QFont f = b->font();
                    if (!family.isEmpty()) f.setFamily(family);
                    f.setPointSize(13);
                    b->setFont(f);
                    b->setFixedSize(30, 30);
                    b->setAutoRaise(true);
                    connect(b, &QToolButton::clicked, this, [this, sm, ch, family] {
                        sm->close();
                        if (!m_doc || refuseIfReadOnly()) return;
                        write::insertSymbol(*m_doc, m_view->cursor(), ch, family);
                        rememberSymbol(ch, family);
                        m_view->setFocus(Qt::OtherFocusReason);
                    });
                    grid->addWidget(b, i / 5, i % 5);
                }
                auto *wa = new QWidgetAction(sm);
                wa->setDefaultWidget(panel);
                sm->addAction(wa);
                sm->addSeparator();
            }
            sm->addAction(act("insert.symbol"));
        });
        return sm;
    };
}

void WriteWindow::refreshInsertUi()
{
    // A header, a footer, or a note has no pages to break (nor a blank page); the characters go in any text.
    const bool pages = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    for (const char *id : {"insert.cover.classic", "insert.cover.band", "insert.cover.plain"}) act(QString::fromLatin1(id))->setEnabled(m_doc != nullptr);
    act("insert.cover.remove")->setEnabled(m_doc && write::hasCoverPage(*m_doc));
    act("insert.dateTime")->setEnabled(m_doc && !m_view->inHeaderFooter() && !m_view->inNote());
    act("insert.textFromFile")->setEnabled(m_doc && !m_view->inHeaderFooter() && !m_view->inNote());
    act("insert.hyperlink")->setEnabled(m_doc && !m_view->inHeaderFooter() && !m_view->inNote());
    act("insert.removeHyperlink")->setEnabled(m_doc && !m_view->inHeaderFooter() && !m_view->inNote() && write::hyperlinkAt(*m_doc, m_view->cursor().position()).valid());
    for (const char *id : {"insert.blankPage", "layout.columnBreak", "layout.sectionBreak.continuous", "layout.sectionBreak.evenPage", "layout.sectionBreak.oddPage"})
        act(QString::fromLatin1(id))->setEnabled(pages);
}

} // namespace jp
