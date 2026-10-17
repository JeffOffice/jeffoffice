// The pages of the Format pane are built here, each in a file of its own as it is made.
#include "app/formatpane_p.h"

namespace jp::format {

void buildPages(FormatPane *pane, PanePage *fill, PanePage *effects, PanePage *size, PanePage *)
{
    buildFillLinePage(pane, fill);
    buildEffectsPage(pane, effects);
    buildSizePage(pane, size);
}

} // namespace jp::format
