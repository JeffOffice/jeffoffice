# One dialog style for JeffOffice (approved by Jeff, November 2026)

Every dialog, message box, File page, and side pane in all four apps follows this page. The shared
pieces live in `libs/shell/app/uistyle.h` and the application style sheet (`modernStyleSheet()` in
`libs/shell/app/icons.cpp`). `python3 -I tools/check_ui_style.py` reports what breaks the rules.

## Colors are palette roles, never literals
Ask `jp::` for them, so dark mode and high contrast work: `uiText()` (text), `uiMuted()` (grey detail
and headings), `uiAccent()` (the app's own color: default button, focus), `uiDanger()` (red: Warning and Critical signs), `uiLine()` (borders), `uiPanel()`
(soft grey panels, previews), `uiCard()` (field and card fill). In high contrast the system's colors
replace all of these.

## Spacing (pixels)
- Dialog margins 20 on every side; space between sections 16; between a heading and its rows 8.
- Rows: 8 apart, fields 28 tall (4 + 4 padding, 13 px text). Radius 6 on fields and buttons, 8 on panels.
- Label column: one per dialog, as wide as its longest label (at least 96); labels sit left of their
  field, left-aligned and centered on its height (`jp::styleForm`, applied to every form of a dialog
  when it is shown).
- Space between buttons 8; the button row sits 16 below the last section.
- Two fields on a row: 12 between them, a small (11 px, muted) label above each (`jp::twoUp`).

## Message boxes (`jp::plainBox`; no hand-built `QMessageBox`)
- A sign at the left, 32 px, by kind (the blue is a fixed blue, not the app's color): Warning is a red triangle with "!", Critical a red circle
  with an X, Information a blue circle with an "i", Question a blue circle with a "?".
- Text: the first sentence bold at 15 px, the rest as a muted detail line under it (13 px, wraps at
  about 360 px). A title with one sentence has no detail line.
- Buttons: no icons. One accent-filled default button. A destructive choice (Don't Save, Delete,
  Discard) sits alone at the left; every other button is at the right, the accept button last.
- Save on close, in all four apps: red triangle; "Save changes to "<name>"?"; muted line "If you
  don't save, your changes since the last save are lost."; Don't Save left; Cancel and Save
  (accent, default) right.

## Settings-type dialogs (Font, Paragraph, Format Cells, Options, Page Setup, and the rest)
- No `QGroupBox` with a title. A section is a heading (`jp::sectionHeading`, or `jp::Section` where a
  frame-less group with a layout is handier): 11 px, medium weight (500), muted, UPPERCASE, with
  16 above it (not the first one) and 8 below.
- Rounded fields from the style sheet; never set a field's own border or background.
- Labels have no trailing colon. A label that is a question or sentence keeps its mark.
- OK or the primary button is accent-filled (`primary=true`) and the default. Cancel is a plain
  button. Use `jp::ButtonBox`: it does the order, the accent, and the destructive place for you.
- Previews sit on a soft grey panel (`jp::previewPanel`: `uiPanel()`, radius 8, no border), not a
  sunken `QFrame`.
- Compact: no empty rows; checkboxes are one per row at the field column or two on a row with
  `jp::twoUp`; a note is a muted 12 px line under what it explains.
- Links such as "Page Setup" are `jp::linkButton` (accent text, no border, underline on hover).

## Button order
Left to right: [destructive alone] ... stretch ... [Help] [secondary] [Cancel] [primary]. The
primary is last and always the default. A dialog with one button (Close) has it at the right, as the
primary.

## Print page (File > Print, all four apps; `jp::PrintPageLayout`)
Print button and Copies on top, then four groups under headings: PRINTER (list, Settings), PAGES
(Print, Pages, Which pages, Sides, then Collated and Last page first on one line), LAYOUT (Pages on
a sheet, Scale to paper), PAPER ("(as each sheet says unless changed)": Turned, Paper, Margins,
Size, then Ignore the print area and Grayscale on one line). Two fields per row with a small label
above each. "Page Setup" is a link at the bottom. The preview sits to the right.

## File pages (Info, New, Open, Save As, Export, Options) and side panes
Same headings, labels, fields, and buttons as above. A pane's title is a 15 px bold line; its
sections use `jp::sectionHeading`; its buttons are plain, with one primary at most.

## Words
Titles of commands keep their capitals ("Page Setup"). No colons after labels; no "..." except on a
button that opens another dialog. Buttons name what they do: "Save", "Delete", not "OK", when the
dialog is a question.

## What does it for you
- Every `QDialogButtonBox` shown by the program gets the button order, the accent default, and no icons
  (`jp::installButtonBoxStyle`, installed with the theme); write `jp::ButtonBox` in new code.
- Every dialog of the program's own gets one label column and rows 8 apart in its forms when shown.
- `QGroupBox` frames are gone from the style sheet; use `jp::Section` (drop-in) for a titled group.
