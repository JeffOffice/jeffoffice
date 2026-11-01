#pragma once
// The question of Fill > Across Worksheets: which of the other sheets get the cells that are selected, and whether they get everything, just the contents, or just the formats.

#include "core/editsession.h"

#include <QDialog>

#include <vector>

class QComboBox;
class QListWidget;

namespace jp {

class FillAcrossDialog : public QDialog {
    Q_OBJECT
public:
    // `names` are the sheets' names by their places; `current` is the place of the sheet the cells are on, which is not offered.
    FillAcrossDialog(const QStringList &names, int current, QWidget *parent = nullptr);
    // The places of the sheets that are checked, and what is copied to them.
    std::vector<int> sheets() const;
    crunch::EditSession::FillWhat what() const;

private:
    QListWidget *m_list;
    QComboBox *m_what;
    std::vector<int> m_places;
};

} // namespace jp
