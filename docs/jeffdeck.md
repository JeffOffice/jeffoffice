# JeffDeck

JeffDeck makes slide presentations and plays them full screen, with the
features of the presentation programs of the Office 2021 era. It is the second
JeffOffice app, built on JeffPub's shared engine. This file holds its feature
list (moved here from the JeffOffice spec when building started, October 2026)
and how it is built.

**Ready to ship when** the presentation program of that era opens every file
JeffDeck saves, and JeffDeck's slides match that program's own PDFs about as
closely as JeffPub's pages match Publisher's.

## Features

| Area | What it covers | Status |
| --- | --- | --- |
| Slides | Layouts, slide masters, themes and their variants, backgrounds, sections, the slide sorter, outline view, speaker notes, and notes and handout masters | Layouts, masters, notes, backgrounds: started |
| On a slide | Text, shapes, pictures, icons, Text Art, tables, charts, audio, and video. SmartArt diagrams and equations show as saved at first; editing them follows | Text, shapes, pictures: from the shared engine |
| Motion | Slide transitions, including Morph, and animations (entrance, emphasis, exit, motion paths) with the animation pane and timings | Transitions: started |
| Slide Show | Full screen, Presenter View on a second screen (notes, next slide, timer), pen and highlighter, rehearsed timings, custom shows, and looping for kiosks | Full screen, Presenter View: started |
| Working together | Comments on slides, and reusing slides from another presentation | Not started |
| Output | Printed slides, notes pages, and handouts, with print preview; PDF and pictures. Video export follows the first release | Not started |
| Files | .pptx, .potx, and .ppsx; opens .ppt and .odp | .pptx reading: started |

## How it is built

```
apps/deck            JeffDeck: the window, slides, notes, the slide show,
                     the .pptx reader and writer
libs/page            pages of placed objects: the document, its drawing,
                     the editor, the canvas (shared with JeffPub)
libs/shell           the window parts every app shares
libs/office          Office Open XML: packages, themes, DrawingML
libs/text            the text engine
libs/draw            shapes, fills, outlines, placed objects
libs/base            colors, fonts, files, PDF
```

- **The model.** A presentation is a shared `Document`: each slide is a
  `Page`, each slide layout a `MasterPage` whose `baseId` is its slide
  master, and the masters say which they are in their `props` ("kind":
  "master" or "layout", a layout's "type" as presentation files name it).
  Placeholders are text boxes whose `role` is `ph:<type>` or
  `ph:<type>:<index>`; a layout's and a master's are prompts that new slides
  copy and that never show on the slides themselves. A slide's speaker notes
  are a story (`notesStoryId`); hidden slides, transitions, and what the
  reader keeps for writing back are in the page's `props`; the theme is the
  document's `appData`. See `apps/deck/core/deck.h`.
- **Undo** is the shared editor's: every change is one step that restores the
  whole document.
- **Files.** `.pptx` is ECMA-376 PresentationML: `libs/office` reads the
  package, the theme, and the DrawingML shapes and text that slides share
  with documents and spreadsheets; `apps/deck/io` reads the presentation's
  own structure (masters, layouts, slides, notes) on top.
- **Nothing in a file is lost.** What JeffDeck can't show yet (tables,
  charts, SmartArt, animations) is kept with the slide to be written back.
- **Tests.** `tests/deck` (program `jdtests`) and `tests/office`
  (`officetests`), run on every push on Windows, macOS, and Linux.

## Reference files

The presentation program's own renderings are the reference, made in the
Windows VM: its PDFs and slide pictures of 54 real presentations (1,028
slides) and of its 10 built-in designs (one slide per layout, 144 slides).
They stay on the build machine, never in this repository.
