# Add and change pictures
<!-- keywords: picture, pictures, image, insert picture, photo, clipboard, paste a picture, screenshot, wrap text, text wrapping, in line with text, square, tight, top and bottom, behind text, in front of text, alt text, description, resize, size, handles, move a picture, drag a picture, drop, picture format, lock aspect ratio, crop, cut, brightness, contrast, corrections, recolor, grayscale, black and white, washout, border, frame, picture styles, change picture, reset picture -->

A **picture** can sit in a line of text, like a letter, or float on the page with the words going round it. This topic shows how to put a picture in a document, how to change its size, its place, and how the text goes round it, and how to describe it for people who can't see it.

## Put a picture in

- Click **Insert > Pictures**, then **Picture**, pick a file, and click **Open**. JeffWrite reads PNG, JPEG, GIF, BMP, TIFF, WebP, and SVG files. A picture that comes from a file is kept in the document as the file it was (a WebP or SVG picture is kept as PNG).
- Or click **Insert > Pictures > Picture from Clipboard**, or press **Ctrl+V**, when you have copied a picture (a screenshot, say). The picture is kept in the document, so it is still there when you save and open the file again.
- Or drag a picture file from your computer's file manager onto a page. The picture goes where you drop it.

The picture goes where the caret is, in place of anything you selected. It is made as large as it asks to be, but no wider than the column and no taller than the page's text, with its proportions kept. The picture is selected afterward, and the **Picture Format** tab appears at the right of the ribbon.

A picture that is too large (more than 40 million pixels, a side longer than 30,000 pixels, or a file of more than 48 MB) is not put in, and the status bar says so. So is a file that is not a picture.

## Change the size

Click the picture to select it. Eight small squares, the **handles**, appear on its corners and sides.

- Drag a **corner** handle to make the picture larger or smaller in the same proportions.
- Drag a **side** handle to change one side only.
- Or type a number in **Picture Format > Height** or **Width**. With **Lock Aspect Ratio** on, the other number follows. Pictures are held between 2 points and 2,000 points on a side.

Each change is one step to undo.

## Crop a picture

To **crop** a picture is to cut off part of its edges. Select the picture and click **Picture Format > Crop**. Type how much to cut off the left, top, right, and bottom, as a percent of the whole picture. Click **OK**.

The picture frame gets smaller by the same amount, so the part that is left keeps its size. Crop again later and you start from the whole picture, so you can always get back what you cut. A number below zero leaves empty room around the picture. The cut on two opposite sides can add up to no more than 98 percent.

## Change how a picture looks

These commands are on the **Picture Format** tab. Each one is a single step to undo, and none of them changes the picture file inside your document.

- **Corrections** makes the picture brighter or darker, and gives it more or less contrast. **Brightness** is how light the picture is. **Contrast** is how different the light parts are from the dark parts.
- **Color** turns the picture to shades of gray (**Grayscale**), to pure black and white (**Black and White**), or to a pale, faded look (**Washout**). **No Recolor** puts the colors back.
- **Picture Styles** and **Picture Border** put a frame round the picture. Pick a style, or pick a color, then a **Weight** (how thick the line is) and **Dashes** (a solid, dotted, or dashed line). Choose **No Border** to take the frame off.
- **Change Picture** puts a different picture in the same place. The new picture keeps the width of the old one, its place on the page, how the text goes round it, and its alt text. Its height follows its own shape.
- **Reset Picture** takes off the crop, the corrections, the color change, and the border. The picture comes back to the scale it has now.

## Wrap text round a picture

**Picture Format > Wrap Text** says how the words go round the picture. You can also right-click the picture and choose **Wrap Text**.

- **In Line with Text**: the picture is a character of its line, and moves with the words around it.
- **Square**: the words go round a rectangle that holds the picture.
- **Tight**: the words go as close to the picture as a rectangle allows (JeffWrite treats it as Square).
- **Top and Bottom**: the words stop above the picture and start again under it.
- **Behind Text**: the picture is under the words, which go straight over it.
- **In Front of Text**: the picture is over the words, which go straight under it.

## Move a picture

Drag the picture. A picture **in line with text** goes to the place in the words where you let go. A picture with any other wrap moves to where you let go on the page, and stays there while the words around it change (it stays with the paragraph it belongs to, on that paragraph's page).

Dragging a picture that is in line with text does not change its wrap; to let it float, choose a wrap such as **Square**.

## Describe a picture

A picture that only decorates the page, such as a line or a border, does not need a description. Select it and click **Picture Format > Mark as Decorative** (or right-click it). Screen readers then skip it. Click it again to take the mark off.

**Alt text** is a short description that screen readers read aloud, and that shows when a picture can't be. Select the picture and click **Picture Format > Alt Text** (or right-click the picture and choose **Alt Text**), type the description, and click **OK**.

## Pictures from other documents

JeffWrite shows and keeps the pictures of the documents it opens. It can change the size, wrap, place, and alt text of a picture that the document stores as a plain picture. A picture stored in a form JeffWrite doesn't change (a group of drawings, for instance) is selected and kept as it was; its commands are off.
