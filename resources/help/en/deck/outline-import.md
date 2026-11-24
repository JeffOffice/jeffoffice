# Make slides from an outline
<!-- keywords: slides from outline, outline import, markdown, text outline, rtf, docx, headings to slides, bullet list to slides, start a presentation from text, save outline, export outline -->

You can write a presentation as plain text and let JeffDeck turn it into slides. Each slide has a title and a body, on the Title and Content layout, so Outline view, Reset Slide, and themes all work on it as usual.

## Add the slides to the presentation you have open

1. Click **Home > Slides**, then the small arrow under **New Slide**, and choose **Slides from Outline…**.
2. Choose a file. It can be a text file (.txt), a Markdown file (.md), a rich text file (.rtf), or a document file (.docx).
3. The slides are added after the slide you are on. They are one step, so one Ctrl+Z takes all of them back.

## Or start a new presentation from one

Choose **File > Open** and pick the outline file. JeffDeck makes a new presentation from it. The file is never changed, and **Save** asks for a name.

## How the file is read

**Text files.** A line that starts at the left edge is a slide's title. The lines under it are the slide's text. A line that is indented further than the one before it goes one level deeper (use tabs or spaces). A bullet (-, *, or +) or a number at the start of a line is dropped, because the slide has its own bullets.

**Markdown files.** The top heading level the file uses makes the slides: # if the file has one, otherwise ##. Smaller headings become lines of the slide's text, one level deeper for each step. A line of three dashes (---) starts a new slide. Lists become levels. **Bold**, *italic*, `code`, and links keep their look. A block of code between lines of three backticks becomes a text box in a typewriter font. A picture on a line of its own, written in Markdown's picture form (an exclamation mark, the description in square brackets, and the file name in parentheses), is placed on the slide, but only when the picture file is in the same folder as the outline, or in a folder inside it. Pictures from the internet or from other folders are never loaded.

**Rich text and .docx files.** Paragraphs in a Heading style make the slides: the top heading style the file uses is the title, smaller headings are lines of the text, and every other paragraph is a line under the heading above it.

## Limits

An outline can make up to 5,000 slides. A slide's text has up to 9 levels, and a line indented deeper is shown at the deepest one. A text file can be up to 10 MB, in UTF-8 or UTF-16. A file that is not text is refused. When something is left out, such as a picture that could not be used or text past a limit, JeffDeck tells you what.

## Save a copy as an outline

Choose **File > Save a Copy** and pick **Markdown Outline (*.md)**. The file holds each slide's title and text, with bold, italic, and links. It leaves out pictures, shapes, tables, charts, other text boxes, speaker notes, and the look of the slides. JeffDeck tells you this when it saves, and **File > Info > Check Compatibility** lists it too.

## Related

- [Add, change, and delete slides](slides)
- [Reuse slides from another presentation](reuse)
- [Views](views)
