# The .pub file format: notes for the writer

What JeffPub has learned about `.pub` files (versions 2003 and later), from
libmspub's reader and from comparing small sample files that differ in one
thing. `tools/pubdump.py FILE.pub` prints every structure described here.

Units: lengths are EMU (914400 per inch, 12700 per point) unless noted.

## Container

A compound file (`src/io/cfb.cpp`) with these streams:

| Stream | Contents |
|---|---|
| `Contents` | Document, pages, shapes, tables, palette, fonts: a block tree (below) |
| `Quill/QuillSub/CONTENTS` | All text, with character and paragraph formatting and style sheets |
| `Escher/EscherStm` | Drawing records: shape geometry, fill, line, anchors |
| `Escher/EscherDelayStm` | Picture data referenced by the drawing records (empty if none) |
| `\x01CompObj`, `\x03Internal`, `Envelope` | Fixed identification streams |
| `\x05SummaryInformation`, `\x05DocumentSummaryInformation` | Standard OLE property sets (title, author; a preview picture) |

The preview picture (SummaryInformation property 17, `VT_CF`): in all 282
reference files Publisher saved, clipboard format 3 (`CF_METAFILEPICT`:
mapping mode 8, then the picture's size in 0.01 mm at the screen's dpi,
2540/96 per pixel on a 96-dpi screen) around a Windows metafile of seven
records: `SetWindowOrg` 0,0; `SetWindowExt` to the bitmap; a white solid
brush (`CreateBrushIndirect`, `SelectObject` 0); `PatBlt` PATCOPY over the
window; `SelectObject` 0 again; `DIBBitBlt` SRCCOPY of a 24-bit bottom-up
bitmap of page 1 (about 141 to 161 pixels across: it follows the view's
zoom when saved); end of file. JeffPub writes the same, page 1 fitted in
160 pixels. (Not yet seen in File Explorer: the test computer showed no
previews for any office files, test35, Oct 6.)

Storage class ids: root `00021201-0000-0000-00C0-000000000046`,
`Quill/QuillSub` `08C8F6DA-969D-11D1-8E02-00C04FB6FECE`. Empty storages
`Objects` and `VBA` are present.

## Blocks (Contents stream)

A block is `id (u8)`, `type (u8)`, then data whose size the type decides:

| Type | Data |
|---|---|
| 0x00, 0x05, 0x08, 0x0A, 0x78 | none (0x08 is a true flag; 0x78 a placeholder) |
| 0x07, 0x10, 0x12, 0x18, 0x1A | 2 bytes |
| 0x20, 0x22, 0x58, 0x68, 0x70, 0xB8 | 4 bytes (0x68/0x70 hold sequence numbers) |
| 0x28 / 0x38 / 0x48 | 8 / 16 / 24 bytes |
| 0x80, 0x82, 0x88, 0x8A, 0x90, 0x98, 0xA0 | u32 length (counting itself), then nested blocks |
| 0xC0 | u32 length, then a NUL-terminated UTF-16 string |

0x88 is a record of fields; 0xA0, 0x90, and 0x98 are lists whose items are
blocks with id 0.

### Header and chunk directory

At 0x1A, a u32 is the offset of the trailer. The trailer is a u32 length and
three blocks; the one of type 0x90 is the chunk directory. Each item in the
directory has a sequence number in order, the document being 256 (items
before it are 0x78 placeholders). A chunk reference is an 0x88 record:
02 = chunk type, 04 = offset in the stream, 05 = parent sequence number. A
chunk is a u32 length followed by blocks.

The header region before the first chunk also holds a file-information
record: magic `e8ac`, the stream length, the trailer offset, and the path
the file was last saved to. Not fully decoded yet.

### Chunks in a blank one-page Letter publication

| Seq | Type | What |
|---|---|---|
| 256 | 0x44 document | 01 page count?, 02 list of page seqs, 12 page size (01 width, 02 height), links to the chunks below |
| 263 | 0x43 page | master page "A" ("Master Page A") |
| 266 | 0x43 page | the page: 01 shape count, 02 list of shape seqs, 0d its master (263) |
| 269, 272, 275, 279 | 0x43 page | special pages (different sizes; not shown as pages) |
| 264… | 0x60, 0x77 | per-page settings; 0x77 holds web-form defaults |
| 282 | 0x5B | text index: 01 → 0x61, 02 → 0x65, 04 → font chunk |
| 283 | 0x61 | text id → shape: records {01 text id, 03 shape seq} |
| 284 | 0x65 | stories: records {01 text id, 07, 08, 09} |
| 285 | 0x5C palette | 8 colors, scheme name |
| 287, 288 | 0x4B, 0x4F | print settings (paper size, printer name) |
| 289 | 0x4C | layout guides and margins |
| 291 | 0x4A | bullet symbols (Symbol font) |
| 292 | 0x8A | page setup: paper name ("Letter"), sizes, margins |
| 293… | 0x01 shape, 0x10 table, 0x63 cells | objects on pages |

A shape chunk (rectangle): 04 = 259, 0c/0d = 8 zero bytes, aa = width,
ab = height, b7 = 0; a text box (or a shape holding text) adds 27 = its
text id, and 35 = vertical alignment (1 middle, 2 bottom; left out for
top). Publisher doesn't use the drawing property 0x0087 for this.

## Drawing records (EscherStm)

Standard Escher-format drawing records: one `DggContainer`, then a `DgContainer` per
page (Dg instance = drawing id; data = shape count and last shape id). Each
shape is an `SpContainer` with `Sp` (instance = shape type: 1 rectangle,
202 text box; data = shape id, flags 0x0A00), `OPT` and `TertiaryOPT`
property tables, a `ClientAnchor` (blocks 01–04 = left, top, right, bottom,
relative to the page center), and `ClientData` (block 01 type 0x68 = the
shape's sequence number in Contents). Text boxes add `0x0080` = text id in
OPT and an empty ClientTextbox (0xF00D).

Table cell formats are shapes with a ClientAnchor naming the table (see
`third_party/libmspub/src/TableInfo.h`).

Turned shapes put the angle in OPT 0x0004 (clockwise degrees, 16.16). The
anchor is the unturned frame, except that between 45° and 135° (and 225°
and 315°) it holds the frame turned a quarter about its center; the
Contents size (aa, ab) matches the anchor. Sp flags 0x40 and 0x80 flip.
Publisher flips first, then turns; a shape flipped one way (not both) is
turned counterclockwise by the stored angle, so JeffPub (which always turns
clockwise) stores the negative angle for it. Checked in Publisher.

### Lines

A line is an `Sp` of type 32 (straight connector), flags 0x0B00 plus the
flips that say which corner it starts from; the anchor is the box it spans.
OPT holds the line props (0x01C0 color, 0x01CB width, 0x01FF 0x00080008,
0x01CE dashes, 0x01D0/0x01D1 start and end arrowheads with sizes in
0x01D2-0x01D5) and 0x0303 = 0. Its Contents chunk is type 0x20:
`02:08, 03:08, 04:10 = 256, 0c, 0d, b7 = 0`, directory version 0x0102.
Any line or outline can add 0x01D6, its corners: 0 bevel, 1 miter, or 2
round (Publisher's designs store 2 for many shapes). Without it Publisher
draws mitered corners, as its own new shapes do. Publisher opened a JeffPub
file with all three, drew each as such, and kept 0 and 2 when saving again.

### Pictures

Each image is one `BSE` in a `BStoreContainer` (after `Dgg`), instance =
type (5 JPEG, 6 PNG): two type bytes, a 16-byte id (MD4 of the data),
tag 0x00FF, record size, use count, offset into `EscherDelayStm`, and four
zero bytes. The delay stream holds the image records (0xF01E PNG with
instance 0x6E0, 0xF01D JPEG with 0x46A, or 0x6E2 for CMYK): the id, a 0xFF
byte, then the file. The shape is an `Sp` of type 1 whose OPT has 0x007F =
0x00800080, 0x4104 = the store entry (from 1), 0xC105 = the name without
extension (UTF-16, complex), 0x0106 = 1, 0x033F = 0x00100010, and crops in
0x0100-0x0103 (16.16 fractions). The Contents chunk (type 1, directory
version 0x0102, 0b = 1) is `02:08, 03:08, 0c, 0d, 34 = 0, 3a → name chunk,
aa, ab`; the name chunk (type 0x66, parent the picture) holds `03` = the
file name.

### Stories

Chunk 0x65 lists the stories: each record has `01` = the text id, `02` =
the number of boxes in a linked chain, `03` = 0 for a table, `04` (a flag
of type 0x00) when the story isn't hyphenated automatically, `05` (u16,
type 0x10) for AutoFit Text, `07` = its entry in the frame layout section
(MCLD), `08`/`09` (floats, -9999996 when unset), a `0c` flag (type 0x08)
for Grow Text Box to Fit, and `0a` (u32), the hyphenation zone in EMU, left
out at its default of 228600 (0.25 inch). A text box's zone is the same
field (a chain of linked boxes is one story, so it has one zone); files
JeffPub wrote with zones of 36, 30, and 27 points opened in Publisher's
Hyphenation dialog showing 0.5", 0.417", and 0.375". JeffPub reads a zone
only up to 720 points, the most it writes.

`05` is 1 for Best Fit and 3 for Shrink Text On Overflow; older files set
higher bits too (0x103, 0x203, 0x3003), so only the low two bits count.
With Shrink, Publisher also records the font size before shrinking in
`08`, and in both cases it writes the fitted size into the text's runs.
Do Not Autofit leaves `05` and `0c` out. Found by choosing each setting
through Publisher's AutoFit Text menu and saving. JeffPub writes `05` (or
`0c`) and its unfitted sizes: Publisher fits the text again when it opens
the file (10-point text in a box set to shrink opened at 5.3 points, and a
box set to grow opened 204 points tall instead of 60).

### Master pages

The document chunk lists every master page first, then the pages, then
the four special pages; its `2d` is the number of master pages. A master
page is a page chunk (0x43) with `03` = its margin guides (0x4C), `0e` =
its letter, and `0f` = its description ("Master Page A"); a page names its
master in `0d`. Older files show a two-page master as two master chunks
with the same letter, and Publisher 2002 files use L and R.

### Fills, transparency and shadows

These are OPT props on any shape, text box, or Text Art. A solid fill is
0x0181 (color) with 0x01BF = 0x00100010; 0x0182 is its opacity
(0x10000 = opaque). A gradient adds 0x0180 = 7 (linear), 5 (from the
center, with 0x018D-0x0190 = 0x8000), or 6 (along the outline); 0x0181 and
0x0183 are the end colors, 0x0182 and 0x0184 their opacities, 0x018B
is the angle (16.16 degrees, signed), and 0x018C is the focus. With focus 0
the gradient runs from 0x0181 to 0x0183; with focus 100 (as Publisher's
Fill Effects dialog saves it) the two colors trade places. Measured
clockwise from left-to-right on the page, the gradient runs toward 270 -
*a* for a stored angle *a* of 0 or more, and toward 90 - *a* for a negative
one: with focus 0, a stored 0 runs bottom to top, 270 left to right, and
-90 right to left. Publisher's built-in designs store only 0, -45, -90, and
-135; its dialog stores 0-359. Checked in Publisher's pictures of 47
gradients saved by both programs. A linear gradient turns and flips with
its shape only when the tertiary fill flags (0x01BF in 0xF122) carry 0x20
with its use bit (0x00600020, as Publisher's designs write it); otherwise
it keeps its direction on the page however the shape turns. JeffPub writes
focus 0, the angle 270 - *g* (0-359) for a gradient toward *g*, and the
tertiary flag, since its gradients turn with their shapes. More
than two colors go in 0xC197 (complex): count, count, 8, then each color
and its position (16.16). A picture fill is 0x0180 = 3 (stretched) or 2
(tiled) with 0x4186 = the store entry. Publisher draws a stretched picture
fill 2.5 points past its shape on every side, clipped to the shape (a
rectangle of 200 to 600 points with a picture of a red edge 5% wide showed
17 of its 20 pixels at 96 dpi, whatever its place; a page's background is
such a fill, and so is a picture saved see-through or cut to a shape),
where a picture in a picture frame (0x4104) is drawn exactly to its frame.
JeffPub saves a picture as a fill with the part showing and 2.5 points of
picture around it (edge pixels carried on where the picture stops), so the
frame shows what JeffPub drew; a background it opens is drawn the same 2.5
points past the page. Patterns are saved as a tiled 8 x 8
picture of the pattern in its colors. Line opacity is 0x01C1. A shadow is
0x0200 = 0 (offset), 0x0201 color, 0x0204 opacity, 0x0205/0x0206 offsets
(EMU), and 0x023F = 0x00020002. A color can also name another of the
shape's colors and change it: the top byte 0x10, the low byte 0xF0 (the
fill color) or 0xF2 (the line color), the next byte the change (1 darken,
2 lighten), and the third byte how much (out of 255). Publisher's default
text box shadow is 0x107F02F2: the line color lightened halfway, gray for
a black line.

Text and the line around its box: Publisher measures a text box's (or a
shape's) text margins from the inner edge of the box's line, so the text sits
half the line's weight further in when the line is centered on the edge, and
all of it when the line is an inset pen. Measured Oct 11 with a text box and
a rectangle holding text, lines of weight 1, 2, 4, 6, 8, 12, and 24 (the
ink of a letter 0.48 pt in per point of weight, left and top alike, 12 pt in
for an inset line of 12). The margins in the file are the plain margins
(0x0081-0x0084), whatever the line. JeffPub's boxes are laid out so, and a
rectangle drawn just before a text box at the same place, which JeffPub reads
as the box's border, takes the line's share off the box's margins, since the
text of the original sat inside no line.

### Picture settings and picture shapes

A picture's OPT can add 0x0109 brightness (signed; 0x8000 is all the way
to white), 0x0108 contrast (a 16.16 multiplier, 0x10000 unchanged),
0x013F = 0x00040004 for grayscale or 0x00020002 for black and white, and
0x0107 = the color shown clear. Washout is brightness 0x599A with contrast
0x4CCD. A recolor is 0x011A in the tertiary OPT (0xF122); there is no
sepia setting, so a sepia picture is recolored to brown 0x704214 and read
back as sepia. A picture cut to a shape is saved as that shape filled with
the picture (0x0180 = 3); the fill always covers the shape's box, so a
cropped or adjusted picture is saved as the part that shows, with the
settings applied. Publisher showed a picture saved with only the grayscale
or black and white setting in its own colors, so such pictures are saved
already converted, with the setting kept. A picture shape can keep a solid
fill of its own (0x0181, with 0x01BF = 0x001F001C marking it filled, as in
Publisher's designs); Publisher paints it behind the picture, where it
shows through a metafile's clear parts.

### Line spacing

Spacing given in lines (`34:22`, the "sp" value) adds the extra beyond one
line *below* each line: a column's first line sits one ascent below its top,
lines in a paragraph are the spacing apart, and a paragraph after a more
widely spaced one starts lower by that paragraph's extra. Measured on
reference PDFs: a 10 pt Times New Roman paragraph at 1 sp after one at
1.19 sp starts 2 pt lower than single spacing alone gives, and the title
lines of a cover after an empty 1.19 sp paragraph match within 0.2 pt.
Whether a line fits in a box counts its text, not the spacing below it.

Spacing given in points is one of two kinds, told apart by the `34:22` value
(see Paragraph runs below): the points as eighths of an EMU plus 1 is
Publisher's "Exactly" (`ParagraphFormat.LineSpacingRule` 4), and the same
with nothing added is "At least" (rule 3, which the object model reads but
cannot set: assigning 3 gives 4). Measured in Publisher 2021's PDFs of 10 and
24 point text in Times New Roman, Arial, and Calibri, at 1, 1.4, 5, 8, 10, 12,
20, and 30 points (Exactly made through the object model, At least from
JeffPub's files, which Publisher read back as rule 3):

- Exactly: the lines are the value apart whatever the text (the same for 10
  and 24 points and for the three fonts: 5.00, 8.00, 10.00, 12.00, 20.00, and
  30.00 points over twenty lines), and the extra is above the line. A first
  baseline is the line's top plus the value less the font's descent. Where the
  value is less than the font's own single spacing, the whole line shrinks in
  proportion, as with a spacing in lines under 1: the baseline is the top plus
  value x (single - descent) / single (10 point Times, single 10.59, descent
  2.16: 1 point gives 0.8, 5 points 4.0, 8 points 6.4, 10 points 7.97).
  JeffPub had placed such a line the value less the descent below the top,
  1.1 points too high at 5 points and 1.9 at 1.
- At least: the lines are the larger of the value and the font's single
  spacing apart, with the extra above the line, so a value under the font's
  own spacing is plain single spacing (1.4 points of 10 point Arial bold is
  10.9 points, 10 point Times 10.59 up to 10 points, 12.00 at 12 points, 20.00
  at 20 points; 24 point Times 25.42 up to 24 points). JeffPub read these as
  Exactly, so a paragraph with 1.4 points on 10 point text (the MIVDF welcome
  booklet's key points) put its lines 9.5 points too high.

With no space before or after, the first line of the next paragraph is the
same distance below the last line as the lines within a paragraph, for both
kinds.

### Hanging indents, bullets, and tabs

A paragraph whose first-line indent is below 0 hangs: its first line starts
at the left indent plus that, but never left of the text area or inside a
picture's text wrap, where it starts at the free edge instead. A list's
text comes one hang (the first-line indent's size) after its bullet,
wherever the bullet is, and the room a first line needs beside a picture
counts that hang (a gap of 20 pt beside a picture left an 18 pt hanging
bullet a line 2 pt wide; JeffPub now passes it over, as it does a gap too
narrow for a word). Checked against Publisher's own line positions: a
bullet list beside a picture (left indent 9 pt, first line -9) had its
bullet at the wrap's edge and its text 9 pt on; a list with the left indent
0 and the first line -18 had its bullet at the edge and its text 18 pt in,
and a plain paragraph like that started at the edge (a text box's line
breaks matched Publisher's in 20 of 33 lines, 2 before). Record `06` in
those paragraph styles equals the hang and adds nothing: lists with it
(0.15 in, 0.125 in) had their text at the left indent.

A tab after a hanging paragraph's first words ("b.<tab>text") goes to the
left indent (57.6 pt with the first line at 28.8), and the default stops
before it are gone. The default stops are 36 pt apart (or what the
publication says, below) from the text area's edge, not from where the first
line starts or from the left indent. Checked
in Publisher's PDFs: with the left indent at 13.5, a tab at the start of a
line puts the text at 36, 22.5 pt from the line's start (the four
"Name<spaces><tab>number" lines of a newsletter: their numbers stand in a
column there, and 36 pt from the line's start put them 13 pt too far right),
and the second line of a paragraph hanging 18 goes to 36 too. The lines of
a paragraph start in different places (a first-line indent, a hang, a
picture's text wrap, a tab that went on to the line) and the stops stay in
the same places on all of them. Qt counts a stop from each line's own start
and keeps one list for the paragraph's layout, which it reads again each
time it is asked where text is (the caret, the drawing), not once when the
line is laid out: JeffPub gives each line a list from its own start, and
the paragraph's layout hands each line out with its own (a first line
indented 20 put the tabs of a later line at 52 and 88, not 36 and 72, and a
hanging paragraph's second line had one at 172.8, not 144). A line that
starts on a stop gets no list, as Qt's own spacing is the same places, and
a list is slow: for every tab it answers Qt goes over the rest of the
paragraph's items (3,000 price list rows in one paragraph took 6.9 s
against 0.3 s without a list, 24,000 runs 21.8 s against 1.2).

A tab goes with the text after it. When that text does not fit after the
tab's stop, Publisher starts the next line with the tab (the spaces before
it stay at the end of the line above) and the tab goes to the first stop
past the line's start. The four "Name<spaces><tab>number" lines had their
stop (144) past the end of the line; a course list's "Title<tab>Management"
had its stop (180) on the line and the word (55 pt) too long for what was
left, and moved the same way. Of several tabs together only the last goes on
with the text: the ones before it stay while their stops are on the line and
move with it once past its end (a line ended with one tab, and the next
began with two, from three in a row). Qt leaves such a tab at the end of the
line, so JeffPub lays the line out as Qt does and starts the next one that
far in (the tab's width, and the spaces after it) instead of moving the
character, from where that line starts (the stops being the same places
whatever the line). When the line breaks inside the group, so that the next
line starts with the last of the tabs, the tab ending the line above stays,
unless its stop is past that line's end (a flyer's "Strategic<spaces><tab>
<2 spaces>" ending a line and "<tab><6 spaces>National" starting the next:
Publisher's PDF has "National" 28.1 from the edge, a stop and the spaces,
not a stop further, as when that tab had gone on too). That is done only when the text after the tab, its first
word, fits on the next line after the tab's stop; otherwise the tab stays at
the end of the line above and the text starts the next line at the left
indent, as Qt sets it. The tab had gone anyway and left what was after its
stop: in a box 36 pt wide "Qty<tab>125 units" came out "Qty", "1" at the
edge, and "25 units", and in one 60 wide the word of "Name Smith<tab>Management"
(46 pt) had 24 pt after its stop and broke (Mana-gement, or Man-agement when
hyphenating). Not checked against Publisher: what it does in these cases (the
assumption is that it doesn't break a word that fits whole on a line).
Qt shifts a centered or right-aligned line's text, which the stops don't
share, so where its tab ends is counted from the line's text, not from where
Qt draws it (a centered line's tab was taken for past its stop and went on,
36 pt too far in). Not checked: stops saved in the paragraph (no file in
the corpus has any), and a hanging paragraph's tabs on its second line when
the left indent is not 18.

A bullet in the list's own font (Symbol, Wingdings) is that font's regular
face, whatever the item's first run is. Publisher's PDFs hold one SymbolMT
(or Wingdings-Regular) for bold and plain items alike, and the asterisk
bullets of a bold booklet list stand thin in its own pictures; JeffPub drew
them in the text's font at the run's weight, bold.

The glyphs themselves, measured in those PDFs (ems of the bullet's size,
from the pen's origin, y up from the baseline): Symbol's round bullet (0xB7,
nearly every list) is a circle 0.355 across, x 0.052 to 0.407, y 0.103 to
0.458, its advance 0.458; its asterisk (0x2A) a five-petal asterisk, advance
0.5, x 0.070 to 0.430, y 0.129 to 0.534; Wingdings' heavy arrow (0xE8) has
advance 1.067, x 0.085 to 0.983, y 0 to 0.723, and its arrowhead (0xD8)
advance 0.795, x 0.084 to 0.711, y 0 to 0.724. The text font's own bullet,
which JeffPub drew for the round one, is 0.24 to 0.28 across (a dot with 41
to 59% of Publisher's ink); the stand-in drawn for the arrow filled 45% of
the real one's area, and the asterisk's box was 1.4 to 1.6 times too large.
When a symbol font is missing, the bullet is now drawn as a circle, or as a
stand-in glyph scaled to the real one's box (the round bullet is a circle
wherever Symbol is). The greater-than sign (Symbol 0x3E, 119 bullets in the
built-in designs) is estimated from Publisher's 96 dpi pictures: a box about
0.41 by 0.44 from x 0.09, y 0.03; it was drawn as a round bullet.

The text after a bullet that doesn't hang starts at the list's indent,
measured from the text area's edge, or at the bullet's end when that is
further, with no gap after the bullet: a table of contents (the bullet at
15.3 ends at 19.88: the text is at 462.34 = 457.78 + 4.58 in its PDF), a
flyer (563.83 = 538.39 + 25.44). The indent is property 0x06 of the
paragraph's style, in EMU (7.2, 9.4, and 10.8 pt in the built-in Web
designs, 0 in one; a quarter inch, 18 pt, where the style leaves it out),
and 0 on a right-to-left paragraph; the list's delimiter (0x58's high half,
-1, 0, or 3) makes no difference. Publisher's pictures of the 154
left-aligned Web designs with a Symbol bullet put the text where this does,
to the glyph's side bearing; JeffPub put it 4 pt past the bullet, whatever
the indent. The text of a centered list is not checked.

### Process colors

A fill given as process inks keeps, in the tertiary OPT, the color as shown
(0x019E, the same as 0x0181) and its inks in 0x019F and 0x01A6. Read the two
as one run of bits, 31 from each (0x019F's bits 0-30, then 0x01A6's): bits
0-4 are the bits per ink (8); bits 5-8 say which inks there are (0x100 cyan,
0x80 magenta, 0x40 yellow, 0x20 black); then come only those inks' values,
in C M Y K order, from bit 9. Three reference files hold all four inks
(0x1E8: C 98 M 76.1 Y 13.3 K 30.2 is 0x4585F5E8 with 0x01A6 308; C 60 M 40
Y 40 K 100 is 0x4CCD33E8 with 1021) or cyan and magenta (0x188: C 75.3
M 40 is 0x00CD8188). Checked in Publisher's color dialog: single inks
(C 100 0x0001FF08, M 100 0x0001FE88, Y 100 0x0001FE48, K 100 0x0001FE28)
and M 50 Y 100 K 20 (0x67FF00E8) all read back exactly; flags that don't
match the values read as no ink. Publisher shows the
0x0181 color on screen and writes the inks to CMYK PDFs.

## Text (Quill/QuillSub/CONTENTS)

Starts with a `CHNKINK` index of sections, each with a 4-letter name, an id,
an offset, and a length:

| Section | What |
|---|---|
| TEXT | All stories' text, UTF-16, paragraphs end with `\r` |
| STSH ×3 | Style sheets (the second holds the styles; ~16 KB in a blank file) |
| FDPC / FDPP | Character / paragraph formatting runs (512-byte pages) |
| SYID | Text ids of the stories, in order |
| BTEC / BTEP | Indexes into FDPC / FDPP |
| STRS | Story lengths |
| MCLD | Text frame layout per story (below); required once a table is present |
| FONT | Font table |
| SGP, INK, PL | Small fixed sections; PL is the text color list (below); SGP holds the publication's default tab stop (below) |
| TCD | Table cell text ends (tables only) |

A blank file has no TEXT, FDPC, FDPP, BTEC, BTEP, STRS, or MCLD sections.

**The index's blocks.** The header gives the total number of sections at
0x0C, the total size of the index's blocks at 0x10 (512 per block), and the
stream's size at 0x14. The index itself is a chain of blocks: each starts
with `0x01F8`, its count of entries (u16), and the next block's offset (u32,
`FFFFFFFF` on the last), then 24-byte entries. The first block is at 0x18
and holds 19 entries, ending before the text at 512; the others are 512-byte
blocks of 20 entries at the end of the stream (Publisher's 93-section
newsletter: 19 + 20 + 20 + 20 + 14). Publisher refuses a file whose
first block runs past 512 into the text: JeffPub wrote the whole index in
one block until October 2026, so its .pub files with more than 21 sections
(any story long enough to need three paragraph pages, about 170 paragraphs)
didn't open. Confirmed by opening the saved files in Publisher.

**Formatting pages.** Each 512-byte FDPC or FDPP page starts with its run
count (u16), 1 (u16), and where its text starts (u32): the last end on the
page before it, 0 on the first page. Then the runs' end offsets (u32, in
the stream), their property offsets in the page (u16), and the property
blocks packed from the page's end. BTEC/BTEP hold the page count, 4, 0, 0,
each page's last end, and each page's offset.

### Style sheets

Three STSH sections, each an offset table (u32 total, u32 count, three u32s,
then a u32 offset per entry). The first names the styles: per style a u16
length in characters, the name in UTF-16, then a u32 id. Publisher's
built-in styles have no name and a negative id (-1 Normal, -3, -6...); a
named style has id 0. The second has two entries per style, character then
paragraph properties: a u16 word count, a u32 length, the property blocks
(as in FDPC/FDPP), two zero bytes. Publisher's own styles carry a font for
each of the 35 writing-system slots. The third has two 16-byte entries per
style: u16 7, the style it is based on (-1 none), the style's index, u32 4,
two bytes that vary. A paragraph's style is FDPP `19:1a` (the index; absent
for Normal). Bold and italic in a run switch the style's on or off: a bold
run in a bold style shows plain.

### Text color

A run's color is two records that name the same entry in the PL list:
`44:8a {00:22 index}` (the color) and `58:8a {00:12 1, 01:22 index,
02:22 100000}` (the text fill: solid, that color, 100% opaque). Publisher
draws the fill, so a run with only 0x44 keeps the Normal style's fill
(index 0) and shows in the main color.

PL is a u32 count, 0x28, 0, then length-prefixed entries. Entry 0 is the
scheme's main color (`01:22 0x08000000`, 02/04/05 = 0xFFFFFFFF, 03 =
0x20000000); entry 1 is black (`01:22 0`). A custom color is
`01:22 BGR, 02:22 0xFFFFFFFF, 03:22 0x20000000`; a scheme color is
`01:22 BGR, 02:22 0x08000000 + slot` plus an 03 value not yet decoded.
Every entry ends with `06:82` holding two zero bytes. Files that use only
scheme colors may have no PL section at all.

## Tables

A table is a Contents chunk of type 0x10 (directory version 0x0102, 0b =
0x16): `02:08, 04:10 = 259, 0c, 0d, 27 = text id, 2a:08, 66 = rows, 67 =
columns, 68 = width, 69 = height, 6b → cell list, 6d = edges, 70 =
0xFFFFFFFD, b7 = 0`. The edges list (0x90) holds a record per column, then
per row: `01` = where it ends, `02` = its size. JeffPub's reader leaves out a
table of more than 256 rows or 256 columns (twice the largest Publisher
makes), since every cell of it would be made, and goes on with the rest of
the file. It does the same for a table whose row or column count doesn't
match its edges, whose cell list isn't in the file, or that lacks a count.

The cell list (type 0x63, parent the table, directory 08 and 0b = 1) is
`01:18 count, 02 list`; each cell has `01` first row, `02` last row, `03`
first column, `04` last column (zero fields left out), `06` Text Direction
(2 when the cell's text is turned a quarter turn, so its lines read from top
to bottom; left out when horizontal), `07` vertical alignment (a u16, type
0x10: 1 middle, 2 bottom; left out for top), `0a`-`0d` margins (left, right,
top, bottom) and `0e = 114300`. A merged cell is one entry spanning its
rows and columns; the cells under it are not listed. Cells can come in any
order; their text follows the list order. The count is 16 bits, so a list
holds at most 65,535 entries (a 256 x 256 table has 65,536 cells; divided cells
count twice): JeffPub writes the cells first, then as many second halves as
fit, and the save says what it left out.

Text Direction is the cell's `06` and nothing else in the cell: in Publisher
2021 `Cell.CellTextOrientation` is 1 for horizontal and 2 for turned, and
accepts no other value. Found by setting it on one cell through the object
model and through the Table Layout tab's Text Direction button (a toggle:
twice is horizontal again) and comparing the saved files; setting another
cell moved `06` to that cell's record. JeffPub reads only its low byte, as
for `05` and `07`. Publisher also records the cell's frame turned (see Frame
layout below).

Vertical alignment is the cell's `07` and nothing else in the file:
`Cell.VerticalTextAlignment` is 0 (top), 1 (middle), or 2 (bottom), and
accepts no other value. Setting it back to 0 removes the field. Found by
setting each value on cells through the object model and comparing the saved
files: only the cell list changed. On the first cell of a table the field has
stray bits above its low byte (0x6C01, 0x3101, 0x3102 for 1, 1, and 2), so
read only the low byte. In a turned cell, bottom puts the text at the left
edge. JeffPub's files opened in Publisher read back each cell's alignment
through the object model and drew the same positions as JeffPub, merged and
turned cells included, and a Publisher-made table opened in JeffPub drew its
cells at the same places.

A cell divided by a diagonal (Table Layout > Merge > Diagonals) is two
entries in the cell list, at the same row and column, each with its own text
and `05` (a u16, type 0x10; read only its low byte, as the first entry's has
stray high bits): bit 0 = divided, bit 1 = the line runs from top left to
bottom right (`Cell.Diagonal` 2; 1 is bottom left to top right), bit 2 = the
first half. So the first half has 5 or 7 and the second 1 or 3. The first
halves keep the cell's place in the list; the second halves are listed after
all the grid's cells, in the same order, with the same `01`-`04`, margins, and
other fields. The text follows the list order, so a divided cell adds a
paragraph after the grid's text and an end to the `TCD ` list. Each half's
frame in the frame layout is the whole cell's again, with `12` = 0 instead
of 219456000. In the object model `Cells(r, c, r, c)` then has two items, the
first half and the second (`Cell.Diagonal` 1 or 2 on both). Found by dividing
cells in both directions, with text in both halves, and comparing the saved
files. Besides the records above, only the story record's `07` and the frame
layout's first number changed, from 1 to 3 with two divided cells (JeffPub
writes them as for any table, and Publisher opened its files and read every
half's text).

Publisher draws the first half in the triangle with the cell's left edge (the
upper left for `/`, the lower left for `\`) and wraps its text inside it. The
dividing line itself is only a gridline: nothing prints, and `SaveAsPicture`
draws it dotted, unless the cell has a diagonal border (Table Design >
Borders; `Cell.BorderDiagonal`, with a `Weight` and a `Color.RGB` shared by
both halves). That border is a line shape (type 20) in the table-format
drawing, like a rule: its client anchor has `01 = 3`, `02` = the table, and
the cell's grid box in `04`-`07` as a rule's (first row, first column, last
row, last column), the shape's line color is `01c0` and width `01cb`, and its
flags have the flip-horizontal bit (0x40) for `/`. JeffPub keeps the line
along a cell's diagonal, or none, in this shape, and each half's text in
its own entry, so the two stay apart in both directions.

How Publisher lays out and edits the halves (found with pictures of both
directions, long text in both halves, and by typing in the application):

- A half is a triangle and its text wraps inside it, left aligned (or
  centered, or justified, as the paragraphs say). A line has the room that
  is left, inside the cell's margins, beside the diagonal over the height of
  its text: for the lower right half of `/` and the lower left of `\` that
  is where the diagonal is at the top of the line, for the other two where
  it is at the bottom, so the lines get shorter toward a triangle's point.
  Lines sit on the cell's usual grid from its top margin; a line where not
  even the first word (or the part of one before a hyphen) fits is left
  empty, so the text starts lower. Words are hyphenated in the narrow
  tip ("Al-" and "pha" in the tip of a half).
- Text a half can't hold is cut off at the cell's bottom, and the row does
  not grow (200 points before and after a long text in each half, set through
  the object model).
- Vertical alignment applies to each half: middle puts the text where its
  height is centered in the cell, and bottom as low as it fits, so one word
  of a half whose point is at the bottom sits just above where the triangle
  gets too narrow for it. For a long text the two depend on where the lines
  fall, which Publisher settles in a way JeffPub only approximates.
  (`SaveAsPicture` drew top alignment for alignment set through the object
  model until the file was saved and opened again.)
- Clicking puts the caret in the half the click is in, at the end of its text
  when the click is in empty space. Tab goes from the first half to the
  second and on to the next cell, selecting the text of the one it reaches
  (Shift+Tab was not tried). The Right and Down keys at the end of the first half go to the start of
  the second, and Left and Up at the start of the second go to the end of
  the first.
- Setting `Cell.Diagonal` to 0 joins the halves into one paragraph, the
  first half's text and then the second's with nothing between ("A1" and
  "A2" make "A1A2"); a half with no text adds nothing. Changing the direction
  keeps each half's text with its half.

All the cells' text is one story. Its `0x65` record adds `03:10 = 0`, and
it has no `0x61` entry. A `TCD ` section (kind `PLC `, id = the story's
index in SYID) holds: cells - 1, 0, 0xFF00, then each cell's end within
the story (the position of its last paragraph mark; the last value is the
story length).

Because the cells are one story, Hyphenation (Table Layout tab) belongs to
the whole table, not to a cell: its dialog says "Automatically hyphenate
this story", and turning it off with one cell selected turned it off for
every cell (the dialog, opened on another cell after saving and reopening,
was unchecked). The table's story record has the same `04` flag and `0a`
zone as any story; a zone of 0.5 inch gave `0a = 457200`. JeffPub writes the
table as not hyphenated when any cell is set not to hyphenate, with the
first zone that is not 18 points, and a file opens with every cell taking
the table's setting.

The drawing has an `Sp` of type 201 with OPT 0x0080 = text id, insets 0,
0x017F = 0x00300000, and a ClientTextbox. Each cell fill and each ruled
line is a shape of type 1 in the table-format drawing (drawing 3) whose
ClientAnchor names the table (`02:68`):

- Fill: `03` column, `04` row; OPT 0x0181 = color, 0x01BF = 0x001F001C.
- Line: `01` = 1 across or 2 down, then the grid box it runs along as grid
  lines: `04` first row, `05` first column, `06` last row, `07` last
  column. OPT 0x0181 = color, 0x01CB = width, 0x01FF = 0x001F0006.

These shapes take ids from 0x0C03 up, in one block of ids (0x0C00-0x0FFF).
JeffPub writes at most 1,000 of them for the whole publication: the fills,
then the diagonals, then the rules, and the save says how many it left out
(a 40 x 12 table with every border and a diagonal in each cell of its last
row has 1,024).

### Frame layout (MCLD)

Publisher won't open a file with a table unless this section is present.
It starts with the last entry number, the entry count, and the entry
numbers (1, 2, ...); each story's `0x65` record names its entry in field
`07`. Then, per story in order: a length-prefixed `{00:0a, 01:22 =
228600}`, a u32 frame count, and a length-prefixed record per frame (a
text box has one; a table has one per cell, in cell-list order):

| Field | Value |
|---|---|
| 00, 01, 02, 03 | Top, left, bottom, right in layout units: 147 per inch, absolute, with the page center at 110185200 EMU. Starts round up, ends round down. |
| 04, 05 | Width, height (EMU) |
| 06, 07, 08, 09 | Margins: top, left, bottom, right |
| 0a | 82676 for a text box, 0 for a cell |
| 12 | 0 for a text box, 219456000 for a cell |
| 16 | 9525 for a text box, 0 for a cell |
| 0b:1a 0, 11 0, 13:12 255, 14:0a, 18 0, 1a:02, 1d:8a {00:22 = -4} | Fixed |
| 0d, 15 | 0 and 1; a cell with Text Direction turned has 2700 and 2, and its frame is recorded turned a quarter turn about its center (width and height swapped, as a vertical text box's is) |

The column record's first field is a flag of type 0x0a, or 0x02 when the
story is not hyphenated automatically (in 284 Publisher files, all 1,021
hyphenated stories have 0x0a and 1,097 of the 1,100 others have 0x02); its
`01` is the story's hyphenation zone (228600 by default).

### Fonts per script

A run's font container (`24:8a`) holds a container per script, its id the
slot, each with the font's index: Publisher sets slots 0-5 and `2a`
together when you choose a font (checked by setting Courier New and Arial
through Publisher and saving). Slot 0 is the font for Latin text. Runs in
older files can list only other slots (3, 4, 6, 7, `0c`); their Latin text
is in the style's font (Publisher shows Gill Sans MT where those slots say
Courier New). JeffPub writes slots 0-4.

Text pasted from a web page can leave a font named `inherit` (the CSS word)
in the font table, and runs that point at it. No font has that name:
Publisher drew such a run in Calibri (the file's first font), breaking the
lines where Calibri's widths do (31 lines matched within a pixel), and
JeffPub takes the style's font for it, else the table's first. Judged from
Publisher's picture of one file, not tried with another font first.

### Vertical text

A text box whose text runs top to bottom (a book's spine) has `34:20 = 2`
in its shape chunk and text flow 1 (OPT `0x0088`) in its drawing, and its
MCLD frame is recorded turned a quarter turn (width = the box's height).
Every vertical box in 40 of Publisher's covers has the first two; without
`34` Publisher stacks the letters one per line in the tall box, which JeffPub's
files showed until October 2026. Checked by opening the saved file in
Publisher.

### Layout type

Page Setup's layout type is the DOCUMENT chunk's field `11` (u32): 1 for a
booklet, 3 for a folded card, 7 for an envelope; one page per sheet and
multiple pages per sheet leave it out. Found by choosing each in
Publisher's Page Setup dialog and saving.

A booklet (Publisher 2021, Oct 7) also sets DOCUMENT flags `06` and `0b`
and keeps its pages as two-page spreads. Each master is two page chunks
with the same name, a right-hand part (`10` = 1) and a left-hand one
(`10` = 0), each with its own margin guides, the left listed first in the
DOCUMENT page list; `2d` counts both. Page 1 and every odd page use the
right part, even pages the left. Canvases (`05`, and `11` = `05` +
87325200) are 25 inches around the page, a right-hand page's or part's
widened and a left-hand one's narrowed by half a page width; the first
special page is half a page shorter (one page per sheet: also half a page
narrower), the others a left-hand page's width. Only the first page's
field `06` starts with 2; the others are all zero (one page per sheet
numbers them 2 and their index), so a reader that takes a zero there for a
special page loses every page after the first. Objects keep their own
page's coordinates. The print settings (0x8A) add two pages to a sheet
(`09` = 2), the sheet turned to hold them, folded at the page width
(`0a`), and `15` = 4. JeffPub reads and writes all of this: Publisher read
JeffPub's copy of its booklet as its own (booklet layout, spreads, two-page
masters, the same left and right pages) and pictured it the same, and
showed a booklet made in JeffPub with its master's objects on every page.
JeffPub keeps a booklet's master as one two-page master (the right part a
page width over); a single master in a booklet is written to both parts.

A folded card is kept the same way (Publisher's own greeting cards, Oct 9):
DOCUMENT flags `06` and `0b`, `11` = 3, two-part masters (`2d` counts
both), and the pages as spreads; its print settings chunk (0x8A) is
empty, the layout giving the sheet. Publisher's envelope designs don't set
`11` at all (one page per sheet at an envelope's size); choosing Envelope
in Page Setup sets `11` = 7. JeffPub writes 3 and 7 for its folded cards
and envelopes, and Publisher kept both layouts when saving JeffPub's
files again. (Publisher's object model reports a publication's design
type instead, as `PublicationLayout`: 47 for a greeting card design, 23
for JeffPub's copy, "custom".)

### Page numbering

The first page's number and the number style live in a section list, chunk
type 0x75, named by the DOCUMENT chunk's field `2f` (a reference, type 0x70)
and present only when the pages aren't numbered 1, 2, 3. The chunk holds
`01` (type 0x18) = the number of sections and an array `02` (type 0xA0) with
one `00` record (type 0x88) per section: `01` (type 0x20) = the section's
first page index (1 for the first page), `02` (type 0x18) = its start number
(left out when the section continues the numbering), and `03` (type 0x18) =
the style, left out for 1 2 3: 1 = I II III, 2 = i ii iii, 3 = A B C, and
4 = a b c. Its directory entry is stamped like the font chunk's: version
0x0102, flag 08, `0b` = 1. Publisher refuses a start number above 1000.

Confirmed October 2026 by numbering sections in Publisher 2021 through its
object model (`Sections(1).PageNumberStart`, `.PageNumberFormat`), saving,
and comparing the streams; then by opening JeffPub's own files in Publisher,
which read back start 5 in lowercase roman numerals and drew v, vi, vii.
JeffPub numbers a publication one way, so it writes one section and, when
opening a file whose later sections number on their own, keeps the first.

### Text wrapping

How text in boxes behind an object wraps around it is the low byte of the
object's shape chunk field `04` (u16, type 0x10): 0 none, 2 tight, 3
through, 4 top and bottom; square, the default, leaves the field out. (The
high byte varies and isn't about wrapping.) The distances kept clear are
drawing properties `0x0384`-`0x0387` (left, top, right, bottom, EMU). Read
against the wrapping Publisher reports for its objects: 1,330 of 1,342 in
284 publications agree (the rest are groups and boxes the check couldn't
place). Publisher moves a line aside only for an object reaching above the
bottom of the line's text (its fonts' ascent and descent), not into the
spacing below it; and text centered or set at the bottom of its box wraps
where it ends up. Until October 2026 JeffPub opened every object as not
wrapping and saved every one as through (`04` = 0x103).

A text box pushes another's text aside only when its frame reaches into
that box's text area (the frame less its insets) by more than about half a
point: Publisher's designs butt boxes together, and in its pictures a frame
0.5 pt into a text area moved nothing, while one 0.6 pt in (a book cover's
spine) and one 0.72 pt in (a newsletter's columns) moved the lines beside
it to the frame's edge plus its wrap distance (the newsletter's PDF puts
the bullets at 433.87, 431.1 + 2.88 less 0.1; the cover's centered lines
stand 3.5 pt narrower, 1.7 pt off center). JeffPub let anything under a
point go.

### Connectors

Every line is a connector shape in the drawing: 32 straight, 33 an elbow
with one bend, 34 an elbow with two, 37 and 38 the curved ones. The anchor
is the box the line spans; the shape runs from the box's top left,
leaving level, and flips and a turn (OPT `0x0004`) put its start at the
line's start. A route that leaves level is turned 0, flipped across for
leftward and up for upward (both at once is written as a half turn); one
that leaves upright is turned 90 if it goes down, 270 if up, and flipped
across when it runs down and right or up and left. A two-bend route's
middle is OPT `0x0147`, in 21600ths of the way from start to end (10800,
halfway, is left out for elbows), and `0x0303` is 0 straight, 1 elbow, 2
curved.

The drawing's container ends with a solver container (`0xF005`, instance =
the rule count) holding one rule (`0xF012`, version 1) per line: rule
number, shape A, shape B, the line, and the connection site on A and on
B, by drawing shape id. The line starts on A and ends on B; a loose end has
shape 0 and site -1 (Publisher writes a rule even for a line with both
ends loose). A rectangle's sites count from 0: top, left, bottom, right.
Checked by attaching connectors through Publisher and saving, and by
opening JeffPub's files in Publisher and moving a shape: the connectors
followed it.

### Linked text boxes

A chain of linked boxes is one story. Every box has its own shape chunk
and drawing shape, all naming the same text id (`27`, OPT 0x0080, and the
ClientTextbox). The `0x61` map has a record per box in chain order, with
`02` = its place in the chain (left out for the first); the story's `0x65`
record has `02:18` = the number of boxes; its MCLD entry lists a frame per
box in chain order. The boxes can be on different pages. Each box's shape
chunk also links its neighbors: `28` = its place in the chain, `36:68` =
the box before, `37:68` = the box after, and `2d:08` on the last box.
Without these links Publisher hangs while laying out the text.

### Preset shapes and freeforms

Most shapes use the shape chunk of type 1 (as rectangles do); the drawing
`Sp` instance is the shape number (2 rounded rectangle, 5 triangle, 12
star, ...), with handle settings in OPT 0x0147 onward. JeffPub writes a
preset as Publisher's own shape only where the two look the same at every
proportion (`src/io/pubshapes.cpp`; `jpubtool shapecheck` measures it);
anything else is a freeform: `Sp` instance 0 with OPT 0x0142/0x0143 = the
coordinate space (the frame in EMU), 0x0144 = 4, 0xC145 = the points
(count, count, 8, then 32-bit x/y pairs), and 0xC146 = the segments (count,
count, 2, then: 0x4000 move, n lines, 0x2000 + n curves, 0x6001 close,
0x8000 end). The .pub reader takes a segment's count from its low byte, so
runs stay under 256. A freeform whose outline reaches past its frame gets
a frame grown evenly around the same center. Parts within one path (up to
a 0x8000) fill alternately, so holes cut through; a shape whose parts
merge on screen is written as its merged outline plus, when it has an
outline, each part twice more (an even number of layers leaves the fill
alone but draws the inner lines, like a smiley's eyes).

Publisher's own freeforms (shape type 100 in its designs) can take their
coordinates from formulas. The coordinate space runs from 0x0140/0x0141
(left, top) to 0x0142/0x0143 (right, bottom), often -32000 to 32000. A
point coordinate of 0x8000nnnn means formula *n* (other values, negative
ones included, are plain numbers). The formulas are 0xC156: count, count,
8, then per formula a 16-bit code (the low byte the operation, as in
MS-ODRAW's SG formulas; bits 13-15 mark arguments that refer to the
shape's adjust values, the coordinate space's edges, or earlier formulas)
and three signed 16-bit arguments. Operation 6 picks the second argument
when the first is greater than 0. An adjust value used as the angle of a
sine or cosine is in 16.16 degrees.

### Spot colors

A fill printed as a spot ink keeps, beside its process inks (`0x019E` shown
color, `0x019F`/`0x01A6` the ink's CMYK stand-in), a complex property in the
same extra-properties record (`0xF122`): `0x01A1` = UTF-16 text
`P2,#003d007e00db0000,PANTONE 2727 C` (the shown color as four 16-bit hex
values, then the ink's name). Ten book covers carry it, and each of their
PDFs prints that color as a Separation of the named ink with the CMYK
stand-in as its alternate; no file names a color without it being a spot
ink. Other tags than `P2`, and where line and text colors keep a spot name,
are not known yet. JeffPub reads and writes it; Publisher showed a box saved
by JeffPub this way as PANTONE 2727 C in its fill color list (test33, Oct 6).
Character scaling (paragraph run property `0x20`, tenths of a percent, as
tracking) was confirmed in the same file: 150% and 80% drawn as such.

### Text Art

A Text Art shape uses the same chunk type as lines (0x20: `02, 03, 04:10 = 259,
0c, 0d, b7 = 0`). Its `Sp` instance is the warp (136 plain, 144 arch up,
146 circle, 156 wave, 160 inflate, 172 slant up, ... up to 175), and OPT
holds `0xC0C0` = the words and `0xC0C5` = the font (UTF-16 with a closing
zero), `0x00C2` alignment (0 stretch, 1 center (default), 2 left, 3 right,
4 letter justify, 5 word justify), `0x00C3` size and `0x00C4` spacing
(16.16; defaults 36 pt and 1.0), and `0x00FF` settings: the high half marks
which are set, the low half has bit 4 italic, 5 bold, 7 even height, 8 best
fit, 10 stretch, 12 kern, 13 vertical, 14 Text Art.

How Publisher draws circle Text Art (146), measured on its PDFs of ten book
covers that all hold `0x00FF = 0xFFFF5720` and no adjust value: the line runs
clockwise round the ellipse the frame holds, from the shape's default angle
(-179°, just above 9 o'clock) to +179°, so the frame's rotation moves the
start (308° puts it at 129°, measured 128°). The whole text, trailing spaces
included, is scaled as one to fill the path whatever the alignment (where the
letters end shows it: 2019 cover 45°, 2026 cover 68°, both as computed). The
path runs about 0.31 cap heights above the baseline (0.305 and 0.32), so the
letters reach past the frame.

### Fields and hyperlinks

A page number is a "#" in the story text whose character run has `00:12`
with low byte 5 and `22:22` saying which page: -1 the current page, -7 the
next linked box's, -6 the previous box's ("#" shows when there's no such
box). Older files store `00:12` as 0xFF05 rather than 0x0005 (a 2006
newsletter), so only the low byte counts. A date or time keeps the text it
last showed, in plain runs.

Each story with fields or hyperlinks has a TOKN section (kind "PLC "),
worked out from Publisher's own files (Oct 7):

- a count n, 0x0C, 0x0001FFFF, n character positions, and the story's end;
- n property lists (each a length, then blocks), each holding only what
  differs from the entry before: `00:22` flags (0x8C0 for a hyperlink in
  2021, 0xC0 in 2006, else 0), `01:22` the length in characters, `02:22`
  the kind: -5 page number, 1 hyperlink, 6 date or time;
- n more lists, encoded the same way: `00:22` the entry's payload (-1 none);
- the payloads: their length (from the offsets on), their count, 12 bytes
  (0x0E 0xCA 0x59 0xD6, four zeros, then four bytes that differ from file
  to file but are shared by its fields), an offset for each from the start
  of the offsets, then each payload: a count of 16-bit units and the units.
  A hyperlink's is its address. A date's is its kind (1 date, 11 date and
  time, 12 time), Publisher's format number (1-17, its Date and Time list in
  order: M/d/yyyy, dddd MMMM d yyyy, ... HH:mm:ss), the language (0x0409),
  then the format in Windows' letters after a space (" M/d/yyyy h:mm am/pm").
  With no payloads (page numbers only) the header is still written, empty
  (length 0, count 0, the 12 bytes), just past the section and outside its
  stated length: without it Publisher refuses the file.

JeffPub reads all three, and writes them the same way: its TOKN sections
for Publisher's samples were byte for byte Publisher's (but for the 12
bytes), Publisher counted the same fields and hyperlinks in JeffPub's copies
as in its own files, and pictured them the same.

### Objects in text

An object set in a line of text (Publisher's "inline" position) is a U+FFFC
in the story whose character run has `00:12` = 2 (a page number has 5). Each
story with such objects has an EOBJ section in the text stream (kind
"PLC "): the count n, the entry size (4), 0xFF00, n character positions,
the story's length (twice it in a 2006 file), then each object's number.
The objects themselves sit on the last special page (seq 279 in 2021
files), which Publisher lists but never shows; each object's chunk has
`03:08`, `0f:20` = its number, and `34:20` = 0. A 0x70 chunk, field `05:70`
of the text index (0x5b), lists each number with its story's text id and
its object (`00:88 {01:20 number, 02:20 text id, 03:68 object}`). In the
drawing, an object in text is locked against grouping (`007f` =
0x00010001) and, in the tertiary properties, against ungrouping (0x02000200),
and its anchor is in its own box, from its left and top wrap distances.

Layout (Publisher 2021, Oct 7: boxes 6, 18, 36, and 72 pt tall in 12 and 24
pt Times New Roman and 12 pt Arial, measured at 300 dpi): the object sits
on the baseline inside its wrap distances, as a character as wide as the
object and its side distances; one taller than the text's ascent lowers
the baseline, and the line, by the difference, and line spacing stays the
text's. JeffPub matches every case to a pixel. Publisher showed a copy of
its sample saved by JeffPub identically to its own file (the 300 dpi
pictures were the same, pixel for pixel), but only once the run carried
`00:12` = 2: without it the objects floated above their text boxes.

### Paragraph and character settings

Right to left: a sample made in Publisher (Oct 6) holds two paragraph
properties on its right-to-left paragraph only, `06:22` = 0 and `3a:12` =
0xF3FF; `3a` appears in none of 247 left-to-right reference files (`06` = 0
appears in 53 of them, so it isn't the direction). JeffPub reads `3a` =
0xF3FF as right to left and writes both, as Publisher did; Publisher showed
a paragraph saved by JeffPub this way as Right-to-left in its Paragraph
dialog (test34, Oct 6). Alignment codes are the paragraph's start and end:
the same paragraph, with the default alignment, starts at the right, and
Publisher's dialog calls that alignment "Right".

Seen in Publisher's files and written by JeffPub (paragraph runs, FDPP):
`0c:22` first-line indent (EMU, negative for a hanging indent), `0d:22`
left and `0e:22` right indent, `12:22` space before, `13:22` space after,
`34:22` line spacing: in points as eighths of an EMU plus 1 (Exactly) or as
they are (At least; 12 pt is 1219200), or in lines as what would be EMU at 96
pt (a multiple of 4) plus 2 (1 line = 1219202; Normal's 1.19 lines =
1450850). JeffPub writes spacing and line spacing on
every paragraph so Normal's own values (6 pt after, 1.19 lines) don't apply.

Character runs (FDPC): `12:22` and `3e:22` the language as a Windows LCID
(1033 for US English in every run of the 56 reference files with
character pages; JeffPub writes the run's language in both, such as 2058
for Spanish (Mexico), from the table libmspub reads; Publisher showed
English (United States), Spanish (Mexico), French (France), and German
(Germany) for the four lines of test35, Oct 6),
`1e:12 = 1` underline, `10:0a` strikethrough,
`13:0a` small caps, `14:0a` all caps, `0f:12` 1 superscript, 2 subscript,
`1b:22` space added between letters in EMU (Publisher's "kerning"), `1f:1a`
tracking in tenths of a percent (1000 normal, 1250 very loose). A run can
have both. Tracking adds the font's average character width (OS/2
`xAvgCharWidth`) times the percentage beyond 100 after each letter: in a
reference PDF, bold Century Schoolbook at 112.5% got 0.063–0.067 em (0.0655
predicted), bold Times New Roman at 87.5% got -0.055 em (-0.053), and
CloisterBlack at 115% got 0.045 em (0.049). Small capitals get tracking at
their own (0.8×) size.

The default tab stop distance is the publication's, not a paragraph's
(Publisher's `Document.DefaultTabStop`, 36 pt unless set; its Paragraph
dialog's Tabs page shows it as "Default tab stops", 0.2" for 14.4): the text
stream's SGP section, a block list of one property, `00:22` in EMU, 182,880
for 14.4 pt. A publication left at 36 pt has an empty list (length 4 only).
Saved by Publisher with 14.4, 20, 72, and 100 pt (Oct 11): next to a file
at 36 they differ in that section, in what moves with its six bytes, and in
one style sheet byte (0x35 at 36, 0 in the others) that JeffPub doesn't
write and Publisher doesn't need. Publisher takes 1 to 1,584 pt. A MIVDF
flyer (2006) has 14.4 pt, and Publisher's PDF puts the text after its tabs
at multiples of 14.4 from the text area's edge (28.8 after two tabs, 28.0 to
28.1 after one and six spaces), where JeffPub used 36 for them all.
Publisher's PDFs of a publication at 14.4, 20, and 36 and of JeffPub's files
at 14.4, 20, 36, 100, and 1,584 agree with the layout to 0.1 pt, with the
paragraph's left indent 13.5 as well: the stops are the same places on every
line, the indent not moving them.

Tab stops (FDPP `32:82`): `27:1a` count, then `28:8a` with a record per
stop: `00:20` position (EMU), `01:10` alignment in the low byte (left left
out, 1 right, 2 center, 3 decimal), `02:18` the leader character, left out
for none: 46 dots, 45 dashes, 95 a line, 183 bullets (from a sample made in
Publisher with one of each).

Lists (FDPP): `57:8a {00:22 kind, 01:22 bullet character, 02:22 0}` where
kind 23 is a bulleted list (0xB7 = the Symbol font's round bullet) and
otherwise the numbering style (0 1 2 3, 1 I II, 2 i ii, 3 A B, 4 a b;
255 is numbering "(none)", no list);
numbered lists add `58:22` with the punctuation in the high half (2 "1.",
0 "1)", 1 "(1)"). Publisher also writes `02:22` (the bullet or number's
size, EMU; usually the text size), `03:1a` (the bullet's font, its place in
the FONT table: 31 is Symbol and 32 Wingdings in Publisher's own table), and
a ¼" hanging indent (`0c` -228600, `0d` 228600). A Wingdings bullet's
character is kept as 0xF0xx (0xF0E8 is the ➔ arrow). The bullet is drawn in
its font at that size, and
an item's first line is as tall as the taller of the text's line and the
bullet's (SymbolMT's hhea: 2059 + 450 over 2048 em, so 12.25 pt for a 10 pt
bullet; Wingdings' is 1841 + 432). Measured on reference PDFs: list items in 10 pt Times New Roman at
single spacing sit 12.2 pt apart, not 10.6.


### Bold on a font with no bold face

Franklin Gothic Demi and Forte have no bold face, and Arial Black and
Rockwell Extra Bold are too heavy for one; Publisher draws their letters
"bold by hand" and does it differently for each place its text goes. Measured
Oct 11 in Publisher with a test publication (the letters I and H in Arial
Black, Rockwell Extra Bold, Franklin Gothic Demi, and Arial, plain and bold,
at 12, 24, 48, 96, 120 to 300 pt) and with a book cover in Arial Black 48 pt
bold. The widths below are of an "I" stem, which grows by the stroke's width
(a stroke centered on the outline adds half to each side).

| Where | Arial Black, plain or bold | Rockwell Extra Bold, plain or bold | Franklin Gothic Demi, plain | Franklin Gothic Demi, bold |
|---|---|---|---|---|
| Plain letters (no growth), 48 pt | 10.64 pt (0.2217 em) | 12.63 pt | 7.4 pt (0.155 em) | |
| `Page.SaveAsPicture` at 96, 150, and 300 dpi, and the window | no growth (10.69, 10.62) | no growth (12.66, 12.60) | no growth | +0.84 pt (0.0175 em; 96 pt +1.68; 24 pt +0.4) |
| PDF export (`ExportAsFixedFormat`, all three intents) | +1.37 pt: fill and stroke (`2 Tr`), stroke 1/35 em (1.3714 w at 48 pt) | +1.40 pt (1.3721 w at 48.024 pt) | no stroke | +1.35 pt, stroke 1/35 em |
| Printed (XPS with `StyleSimulations="BoldSimulation"` on these runs, through the Microsoft Print to PDF driver) | +0.47 pt: `2 Tr`, stroke 0.01 em (0.12, 0.24, 0.48, 0.96 pt at 12, 24, 48, 96 pt) | +0.5 pt | none (`StyleSimulations="None"`) | +0.51 pt, stroke 0.01 em |

- Arial Black and Rockwell Extra Bold are stroked in a PDF and on paper
  whether or not bold is set (85 of 85 runs in the reference PDFs); Franklin
  Gothic Demi and Forte only when it is. A font with a bold face (Arial) is
  drawn in it, with no stroke, everywhere.
- On the screen and in pictures only the faces set bold grow, by about
  0.017 em (Windows' own synthetic bold), in proportion to the size; the
  Arial Black cover's title is as wide in Publisher's picture (stem 10.69 pt)
  as the plain font, 12.00 pt in its PDF, 11.1 to 11.2 pt printed.
- At 96 and 150 dpi the screen's growth is whole pixels for small letters
  (GDI's synthetic bold): Publisher's pictures of Franklin Gothic Book 8 to
  24 pt, bold against plain, have this much more ink: at 96 dpi 1.85, 1.78,
  1.70, 1.63, 1.40, and 1.23 times (8, 10, 12, 14, 18, 24 pt); at 150 dpi
  1.69, 1.56, 1.38, 1.37, 1.24, and 1.23; at 300 dpi 1.24 at every size, which
  is the 0.017 em growth (and is what the large sizes at 96 and 150 dpi
  come to). Counted in pixels, the stem grows about 1 pixel while the letters
  are under about 30 pixels to the em, and 0.017 em beyond; JeffPub does that
  into pixels at 96 to 216 dpi (a window at 100% and 150%, a picture saved
  at 96 or 150 dpi). The same widening shows in Franklin Gothic Book,
  Century, Impact, and Cooper Black set bold (0.75 to 0.9 pt at 48 pt), and
  they are stroked 1/35 em in a PDF only when bold (all four tried).
- Text from 216 pt up is set as outlines, with Franklin Gothic Demi bold
  widened as on the screen (a rectangle 41.48 pt wide for the "I" of 240 pt,
  plain 37.39 pt: 0.017 em) and Arial Black not at all; no stroke is added.
  In a PDF the change is between 215 pt (text, stroked) and 216 pt (checked
  at 212 to 218 pt); printed output is text at 202 pt and outlines at 240 pt
  (the same 216 pt is assumed, not checked between).
- The stroke has miter joins (the PDF default) in the text's own color.
  JeffPub lays the text out the same for every place and adds the stroke
  as it paints (`TextOutput` in `PaintOptions`, set by `RenderOptions::
  textOutput`: a PDF export is `Pdf`, the printer `Print`, the rest `Picture`).
  The printed 0.01 em is the Microsoft Print to PDF driver's reading of the
  simulation; another driver may embolden by its own amount.
