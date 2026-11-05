#pragma once
// The Scenario Manager's reading of what a person types: the changing cells of a scenario ("B1:B3, D5"), as a list of cell names, and the summary table a list of scenarios makes. The
// cells and the editing are the session's and the window's.

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::crunch {

// The most cells a scenario changes (as the dialogs of other spreadsheet programs have it).
constexpr int kMaxScenarioCells = 32;

// The cells named by text: cell names and ranges, set apart by commas, semicolons, or spaces. Each is a cell name in capitals, in the order given, none twice. False, with the reason in
// `problem`, for a name that is no cell, none at all, or more than kMaxScenarioCells.
bool parseScenarioCells(const QString &text, QStringList *cells, QString *problem = nullptr);

} // namespace jp::crunch
