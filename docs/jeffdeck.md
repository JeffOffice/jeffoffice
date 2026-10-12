# JeffDeck

JeffDeck makes slide presentations and plays them full screen, with the
features of the presentation programs of the Office 2021 era. It is the second
JeffOffice app, built on JeffPub's shared engine. This file holds its feature
list (moved here from the JeffOffice spec when building started, October 2026)
and how it is built.

**Ready to ship when** the presentation program of that era opens every file
JeffDeck saves, and JeffDeck's slides match that program's own PDFs about as
closely as JeffPub's pages match Publisher's. JeffDeck ships in JeffOffice
0.7.0 as a preview: the first holds (the program opens all 64 reference
presentations JeffDeck saved), and the second is close but not yet measured
on the same yardstick as JeffPub's.

## Features

| Area | What it covers | Status |
| --- | --- | --- |
| Slides | Layouts, slide masters, themes and their variants, backgrounds, sections, the slide sorter, outline view, speaker notes, and notes and handout masters | Layouts, masters, notes, backgrounds, slide size, header and footer, sections (saved, and shown in the slides pane and Slide Sorter), Slide Sorter, Outline view: started |
| On a slide | Text, shapes, pictures, icons, Text Art, tables, charts, audio, and video. SmartArt diagrams and equations show as saved at first; editing them follows | Text, shapes, pictures, tables: shown and saved; charts: drawn from their stored values and saved whole; SmartArt diagrams: drawn from the drawing the file keeps of them (one picture on the slide: selected, moved, resized, copied, and deleted as a whole; printed as vectors) and saved whole with all their parts; equations: shown as the picture the file keeps beside them (in a text box with other words too) and saved whole; movies and sounds: kept and saved, not yet played; Text Art, old files' too: shown and saved |
| Motion | Slide transitions, including Morph, and animations (entrance, emphasis, exit, motion paths) with the animation pane and timings | Nine transitions with a Transitions tab, read from and saved to the file (an effect JeffDeck has no form for plays as a fade and is saved as it was); animations play in the show click by click (appear, fade, wipe, split, moves, turns, sizes, pulses, motion paths, builds by paragraph; other effects play as a fade) and are edited on an Animations tab and in an Animation Pane: 8 entrance, 8 exit, and 4 emphasis effects, and 64 motion paths in 6 groups, each with its directions, amounts, start (on click, with, or after the one before), delay, duration, smooth start and end, repeats, rewind, hiding after the effect, and a build by paragraph; Preview plays them; saved as the presentation program writes them, so it lists each by its own name; an effect JeffDeck has no kind for is kept as it was and listed as Custom (it can be moved, removed, and started another way); not yet: triggers (an effect that waits for a click on a shape), sounds, dimming after an effect, effects that change a color |
| Slide Show | Full screen, Presenter View on a second screen (notes, next slide, timer), pen and highlighter, rehearsed timings, custom shows, and looping for kiosks | Full screen, Presenter View, rehearsal (Alt+F5); pen, highlighter, laser pointer, and eraser, in the show and in Presenter View (kept ink becomes one freeform line for each stroke); Set Up Slide Show (by a speaker, in a window, or at a kiosk; loop; without animation; all slides, a range, or a custom show; by hand or by timings; pen and laser colors); custom shows; Rehearse Timings; Use Timings. The settings are saved in ppt/presProps.xml (p:showPr, with the laser color in its extension) and the custom shows in ppt/presentation.xml (p:custShowLst), and read back; a right click in the show opens its menu |
| Working together | Comments on slides, and reusing slides from another presentation | Comments: read, shown, edited, and saved (the presentation program shows them with their replies); Reuse Slides, keeping the source's design or taking this one's |
| Output | Printed slides, notes pages, and handouts, with print preview; PDF and pictures. Video export follows the first release | Print with preview, PDF (slides, notes pages, handouts, outline), slide pictures |
| Files | .pptx, .potx, and .ppsx; opens .ppt and .odp | Reads .pptx, .potx, .ppsx, .ppt, .odp, and .otp; saves .pptx, .potx, and .ppsx (an .odp, or an old .ppt, .pps, or .pot, is saved under a new name, never written over). The presentation program opens all 64 reference presentations JeffDeck saved (its 10 designs and 54 real ones, old .ppt files among them) |

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
- **Nothing in a file is lost.** What JeffDeck can't show yet (animations,
  movies, and sounds) is kept with the slide to be written back. A chart, a
  SmartArt diagram, or an equation is a picture on the slide, with what it is
  made of in the page's `props` ("charts", each with a "kind"); saving writes
  the real thing again. A duplicated slide keeps them; a copy of the picture
  pasted on a slide is only a picture.
- **Text is set as on a slide.** A presentation's `Document::slideText`
  switches the shared text engine to the presentation program's rules: a
  single line is 1.2 times the text size whatever the font, a wider line's
  extra space is above it, the paragraph at the top of a box takes no space
  before it, an empty paragraph shows no bullet, text doesn't wrap around
  objects, and what doesn't fit runs out of its box instead of hiding.
  Publications keep JeffPub's rules.
- **Tests.** `tests/deck` (program `jdtests`) and `tests/office`
  (`officetests`), run on every push on Windows, macOS, and Linux.

## Reference files

The presentation program's own renderings are the reference, made in the
Windows VM: its PDFs and slide pictures of 54 real presentations (1,028
slides) and of its 10 built-in designs (one slide per layout, 144 slides).
They stay on the build machine, never in this repository.
