#pragma once
// The lines of a bundled text table (a file under resources/), whatever a checkout did to it.

#include <QByteArray>
#include <QList>

namespace jp {

// The lines of `data` without their endings: a byte order mark at the start is dropped, a line ends at LF, CRLF, or a lone CR, spaces at a line's end are
// dropped (and tabs too, unless `keepTabs`: in a table whose columns are split at tabs, a tab at the end is an empty last column), and the last line needs
// no ending. Empty lines stay (a caller skips them); the empty piece after the last ending is not a line.
QList<QByteArray> textResourceLines(const QByteArray &data, bool keepTabs = false);

}
