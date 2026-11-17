// JeffWrite's list setup: the galleries under Bullets, Numbering, and Multilevel List on the Home tab, Define New Bullet, Define New Number Format, Define New Multilevel List, Set Numbering Value, and
// Change List Level. Each choice goes into the document's numbering model through the list commands (core/editops.h), as one step of the history.

#include "app/plaintext.h"
#include "app/listdialogs.h"
#include "app/multileveldialog.h"
#include "app/settings.h"
#include "app/writewindow.h"
#include "core/editops.h"
#include "core/listpresets.h"
#include "core/styleops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"

#include <QApplication>
#include <QGridLayout>
#include <QMenu>
#include <QTextBlock>
#include <QToolButton>
#include <QWidgetAction>

#include "app/pageview.h"
#include "app/ribbonbuilder.h"
#include "app/textdialogs.h"
#include "app/widgets.h"

namespace jp {

namespace {

constexpr int kMostRecentBullets = 8;

// A grid of buttons in a menu: each cell's text, its tooltip, and what choosing it does (the menu closes first).
QWidgetAction *gridAction(QMenu *menu, const QVector<std::pair<QString, QString>> &cells, int columns, const std::function<void(int)> &choose)
{
    auto *host = new QWidget(menu);
    auto *grid = new QGridLayout(host);
    grid->setContentsMargins(6, 4, 6, 4);
    grid->setSpacing(2);
    for (int i = 0; i < cells.size(); ++i) {
        auto *b = new QToolButton(host);
        b->setText(cells[i].first);
        b->setToolTip(jp::plainToolTip(cells[i].second));
        b->setAccessibleName(cells[i].second);
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
        b->setMinimumSize(44, 34);
        QFont f = b->font();
        f.setPointSize(f.pointSize() + 3);
        b->setFont(f);
        QObject::connect(b, &QToolButton::clicked, menu, [menu, choose, i] {
            menu->close();
            choose(i);
        });
        grid->addWidget(b, i / columns, i % columns);
    }
    auto *action = new QWidgetAction(menu);
    action->setDefaultWidget(host);
    menu->addAction(action);
    return action;
}

QStringList recentBullets() { return Settings::get().value(QStringLiteral("write/recentBullets")).toStringList(); }

void rememberBullet(const QString &character, const QString &font)
{
    QStringList recent = recentBullets();
    const QString entry = character + QLatin1Char('|') + font;
    recent.removeAll(entry);
    recent.prepend(entry);
    while (recent.size() > kMostRecentBullets) recent.removeLast();
    Settings::get().setValue(QStringLiteral("write/recentBullets"), recent);
}

office::wml::Level levelOfLook(const ListLevelLook &look)
{
    office::wml::Level l = look.bullet ? write::bulletLevel(look.text, look.font) : write::numberLevel(look.numFmt, look.text, look.start);
    if (!look.bullet && !look.font.isEmpty()) {
        office::wml::Fonts f;
        f.ascii = f.hAnsi = look.font;
        l.rPr.fonts = f;
    }
    if (look.sizePoints > 0) l.rPr.size = int(std::lround(look.sizePoints * 2));
    if (look.color.isValid()) l.rPr.color = office::wml::Color{look.color.name().mid(1).toUpper(), {}, {}, {}};
    l.jc = look.align;
    return l;
}

} // namespace

void WriteWindow::createListActions()
{
    auto edit = [this](const std::function<void()> &fn) {
        if (!m_doc || refuseIfReadOnly()) return;
        fn();
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    };
    mk("list.bulletNone", tr("None"), "", QKeySequence(), [this, edit] { edit([this] { write::clearList(*m_doc, m_view->cursor()); }); });
    mk("list.numberNone", tr("None"), "", QKeySequence(), [this, edit] { edit([this] { write::clearList(*m_doc, m_view->cursor()); }); });
    mk("list.defineBullet", tr("Define New Bullet…"), "", QKeySequence(), [this, edit] {
        edit([this] {
            ListLevelLook look;
            const QTextBlock block = m_view->cursor().block();
            if (write::listKindOf(*m_doc, block) == write::ListKind::Bullets) {
                const office::wml::Level *l = m_doc->numbering().levelOf(block.blockFormat().intProperty(write::prop::NumId), block.blockFormat().intProperty(write::prop::NumLevel));
                if (l) look.text = l->text;
            }
            const SymbolPicker pick = [this](QWidget *parent, QString *ch, QString *font) {
                bool picked = false;
                symbolDialog(parent, [&](QWidget *, const QString &c, const QString &f) {
                    *ch = c;
                    *font = f;
                    picked = true;
                    return true;
                });
                return picked;
            };
            if (!defineBulletDialog(this, &look, pick)) return;
            if (look.picture.isEmpty()) rememberBullet(look.text, look.font);
            office::wml::Level level = levelOfLook(look);
            if (!look.picture.isEmpty()) {
                const int id = m_doc->addPictureBullet(look.picture);
                if (id >= 0) level.picBullet = id;
            }
            write::applyListLevel(*m_doc, m_view->cursor(), level);
        });
    });
    mk("list.defineNumber", tr("Define New Number Format…"), "", QKeySequence(), [this, edit] {
        edit([this] {
            ListLevelLook look;
            look.bullet = false;
            if (!defineNumberFormatDialog(this, &look)) return;
            write::applyListLevel(*m_doc, m_view->cursor(), levelOfLook(look));
        });
    });
    mk("list.setValue", tr("Set Numbering Value…"), "", QKeySequence(), [this, edit] {
        edit([this] {
            const QTextBlock block = m_view->cursor().block();
            if (write::listKindOf(*m_doc, block) != write::ListKind::Numbers) return;
            // A list of the same definition above is what can be continued.
            const int definition = m_doc->numbering().definitionOf(block.blockFormat().intProperty(write::prop::NumId));
            bool canContinue = false;
            for (QTextBlock p = block.previous(); p.isValid() && !canContinue; p = p.previous())
                canContinue = p.blockFormat().hasProperty(write::prop::NumId) && p.blockFormat().intProperty(write::prop::NumId) != block.blockFormat().intProperty(write::prop::NumId)
                              && m_doc->numbering().definitionOf(p.blockFormat().intProperty(write::prop::NumId)) == definition;
            NumberingValueChoice choice;
            if (!setNumberingValueDialog(this, canContinue, &choice)) return;
            write::setNumberingValue(*m_doc, m_view->cursor(), choice.continuePrevious, choice.setValue ? std::optional<int>(choice.value) : std::nullopt);
        });
    });
    mk("list.defineMultilevel", tr("Define New Multilevel List…"), "", QKeySequence(), [this, edit] {
        edit([this] {
            office::wml::AbstractNum def = write::listDefinitionAt(*m_doc, m_view->cursor().block(), true);
            QList<QPair<QString, QString>> styles;
            for (const write::StyleEntry &e : m_doc->styleEntries())
                if (!e.character) styles.push_back({e.id, e.name});
            write::ListStyleChoice choice;
            choice.check = [this](const QString &name) { return write::styleNameProblem(*m_doc, name); };
            if (!write::multilevelListDialog(this, &def, styles, &choice)) return;
            if (choice.save) {
                // The definition is kept as a list style, and the paragraphs have the style (so that changing it changes them).
                QString problem;
                const QString id = write::saveListStyle(*m_doc, choice.name, def, &problem);
                if (!id.isEmpty()) {
                    write::applyListStyle(*m_doc, m_view->cursor(), id);
                    return;
                }
                statusBar()->showMessage(problem, 8000);
            }
            write::applyMultilevelList(*m_doc, m_view->cursor(), def);
        });
    });
    for (int level = 0; level < 9; ++level)
        mk(QStringLiteral("list.level.%1").arg(level + 1), tr("Level %1").arg(level + 1), "", QKeySequence(), [this, edit, level] { edit([this, level] { write::setListLevel(*m_doc, m_view->cursor(), level); }); });
}

void WriteWindow::modifyListStyleDialog(const QString &id)
{
    if (!m_doc || refuseIfReadOnly()) return;
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    int abstractId = -1;
    QString name;
    for (const write::WriteDoc::ListStyleEntry &e : m_doc->listStyleEntries())
        if (e.id == id) {
            abstractId = e.abstractId;
            name = e.name;
        }
    if (abstractId < 0) return;
    office::wml::AbstractNum def;
    for (const office::wml::AbstractNum &a : m_doc->numbering().abstracts)
        if (a.id == abstractId) {
            def = a;
            break;
        }
    write::ListStyleChoice choice;
    choice.editing = true;
    choice.name = name;
    if (!write::multilevelListDialog(this, &def, {}, &choice)) return;
    write::modifyListStyle(*m_doc, id, def);
    m_view->setFocus(Qt::OtherFocusReason);
    m_view->viewport()->update();
    refreshUi();
}

void WriteWindow::listRibbonParts(RibbonParts &parts)
{
    parts.menus[QStringLiteral("bulletsMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, this, [this, m] {
            m->clear();
            auto apply = [this](const QString &character, const QString &font) {
                if (!m_doc || refuseIfReadOnly()) return;
                write::applyListLevel(*m_doc, m_view->cursor(), write::bulletLevel(character, font));
                m_view->setFocus(Qt::OtherFocusReason);
                m_view->viewport()->update();
            };
            QVector<std::pair<QString, QString>> recent;
            QVector<std::pair<QString, QString>> recentFont;
            for (const QString &entry : recentBullets()) {
                const int bar = entry.indexOf(QLatin1Char('|'));
                recent.push_back({entry.left(bar), tr("Recently used: %1").arg(entry.left(bar))});
                recentFont.push_back({entry.left(bar), entry.mid(bar + 1)});
            }
            if (!recent.isEmpty()) {
                m->addSection(tr("Recently Used Bullets"));
                gridAction(m, recent, 8, [apply, recentFont](int i) { apply(recentFont[i].first, recentFont[i].second); });
            }
            m->addSection(tr("Bullet Library"));
            const QVector<write::BulletPreset> presets = write::bulletPresets();
            QVector<std::pair<QString, QString>> cells;
            for (const write::BulletPreset &p : presets) cells.push_back({p.text, p.name});
            gridAction(m, cells, 7, [this, apply, presets](int i) {
                rememberBullet(presets[i].text, presets[i].font);
                apply(presets[i].text, presets[i].font);
            });
            m->addAction(act("list.bulletNone"));
            m->addSeparator();
            m->addAction(act("list.defineBullet"));
        });
        return m;
    };
    parts.menus[QStringLiteral("numberingMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        m->addSection(tr("Numbering Library"));
        const QVector<write::NumberPreset> presets = write::numberPresets();
        QVector<std::pair<QString, QString>> cells;
        for (const write::NumberPreset &p : presets) cells.push_back({p.id, p.example});
        gridAction(m, cells, 4, [this, presets](int i) {
            if (!m_doc || refuseIfReadOnly()) return;
            write::applyListLevel(*m_doc, m_view->cursor(), write::numberLevel(presets[i].numFmt, presets[i].text));
            m_view->setFocus(Qt::OtherFocusReason);
            m_view->viewport()->update();
        });
        m->addAction(act("list.numberNone"));
        m->addSeparator();
        m->addAction(act("list.defineNumber"));
        m->addAction(act("list.setValue"));
        return m;
    };
    parts.menus[QStringLiteral("multilevelMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, this, [this, m] {
            m->clear();
            m->addSection(tr("List Library"));
            const QVector<write::MultilevelPreset> presets = write::multilevelPresets();
            for (int i = 0; i < presets.size(); ++i) {
                QAction *a = m->addAction(presets[i].name);
                connect(a, &QAction::triggered, this, [this, presets, i] {
                    if (!m_doc || refuseIfReadOnly()) return;
                    write::applyMultilevelList(*m_doc, m_view->cursor(), presets[i].def);
                    m_view->setFocus(Qt::OtherFocusReason);
                    m_view->viewport()->update();
                });
            }
            const QVector<write::WriteDoc::ListStyleEntry> listStyles = m_doc ? m_doc->listStyleEntries() : QVector<write::WriteDoc::ListStyleEntry>();
            if (!listStyles.isEmpty()) {
                m->addSection(tr("List Styles"));
                for (const write::WriteDoc::ListStyleEntry &e : listStyles) {
                    QAction *a = m->addAction(e.name);
                    a->setObjectName(QStringLiteral("listStyle:") + e.id);
                    connect(a, &QAction::triggered, this, [this, id = e.id] {
                        if (!m_doc || refuseIfReadOnly()) return;
                        write::applyListStyle(*m_doc, m_view->cursor(), id);
                        m_view->setFocus(Qt::OtherFocusReason);
                        m_view->viewport()->update();
                    });
                }
                QMenu *modify = m->addMenu(tr("Modify List Style"));
                QMenu *remove = m->addMenu(tr("Delete List Style"));
                for (const write::WriteDoc::ListStyleEntry &e : listStyles) {
                    connect(modify->addAction(e.name), &QAction::triggered, this, [this, id = e.id] { modifyListStyleDialog(id); });
                    connect(remove->addAction(e.name), &QAction::triggered, this, [this, id = e.id] {
                        if (!m_doc || refuseIfReadOnly()) return;
                        write::deleteListStyle(*m_doc, id);
                        refreshUi();
                    });
                }
            }
            m->addSeparator();
            QMenu *levels = m->addMenu(tr("Change List Level"));
            for (int i = 1; i <= 9; ++i) levels->addAction(act(QStringLiteral("list.level.%1").arg(i)));
            m->addAction(act("list.defineMultilevel"));
        });
        return m;
    };
}

} // namespace jp
