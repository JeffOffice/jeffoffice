// The Fill & Line page of the Format pane: the fill and the line of the selected objects.
#include "app/editor.h"
#include "app/formatpane_p.h"
#include "app/paneframe.h"
#include "app/propertyeditors.h"

#include <QVBoxLayout>

namespace jp::format {

void buildFillLinePage(FormatPane *pane, PanePage *page)
{
    FormatSession *s = pane->session();
    Editor *ed = pane->editor();
    PaneSection *fillSection = page->addSection(FormatPane::tr("Fill"));
    auto *fill = new FillEditor(fillSection->bodyWidget());
    fillSection->body()->addWidget(fill);
    PaneSection *lineSection = page->addSection(FormatPane::tr("Line"));
    auto *line = new LineEditor(lineSection->bodyWidget());
    lineSection->body()->addWidget(line);

    QObject::connect(fill, &FillEditor::changed, pane, [s](const FillChange &change) {
        s->apply(FormatPane::tr("Fill"), [change](Item &it) {
            if (takesFill(it)) change(it.fill);
        }, FormatSession::Scope::Leaves, false);
    });
    QObject::connect(fill, &FillEditor::finished, pane, [s] { s->finish(); });
    QObject::connect(fill, &FillEditor::pictureRequested, pane, [pane] {
        pane->chooseFillPicture();
    });
    QObject::connect(line, &LineEditor::changed, pane, [s](const StrokeChange &change) {
        s->apply(FormatPane::tr("Line"), [change](Item &it) {
            if (takesLine(it)) change(it.stroke);
        }, FormatSession::Scope::Leaves, false);
    });
    QObject::connect(line, &LineEditor::finished, pane, [s] { s->finish(); });

    s->enroll([=] {
        QVector<Fill> fills;
        QVector<Stroke> strokes;
        bool lines = false;
        for (const Item *it : s->leaves()) {
            if (takesFill(*it)) fills << it->fill;
            if (takesLine(*it)) strokes << it->stroke;
            lines = lines || it->type() == ItemType::Line;
        }
        const ColorScheme &colors = ed->doc()->colors;
        fill->setScheme(colors);
        fill->setImageProvider([ed](const QString &id) { return ed->doc()->image(id); });
        fill->setFills(fills);
        line->setScheme(colors);
        line->setArrowsShown(lines);
        line->setStrokes(strokes);
        fillSection->setVisible(!fills.isEmpty());
        lineSection->setVisible(!strokes.isEmpty());
    });
}

} // namespace jp::format
