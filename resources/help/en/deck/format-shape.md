# The Format Shape pane and the Shape Format tab
<!-- keywords: format shape, format picture, format pane, pane, shape format, ctrl+1, fill, solid fill, gradient, gradient stops, stop, picture fill, texture, tile, pattern, line, outline, width, compound type, dash type, cap type, join type, arrows, begin arrow, end arrow, effects, shadow, reflection, glow, soft edges, size, height, rotation, scale, lock aspect ratio, position, text box, margins, vertical alignment, text direction, autofit, shrink text on overflow, resize shape to fit text, wrap text, columns, alt text, title, description, decorative, picture corrections, brightness, contrast, recolor, black and white, transparency, crop, shape fill, shape outline, shape effects, drawing tools, change shape, shape styles, quick styles, style, theme colors, accent, text art styles, text fill, text outline, text effects, transform, letters -->

The **Format Shape pane** is a panel on the right of the window. It shows how the objects you selected look, and it lets you change that. A **shape**, a **text box**, a **line**, and a **picture** all use it. It has a page for the fill and the line, one for effects, one for size and place, and, for pictures, one for the picture's colors.

## Open and close it

- Select an object, then press **Ctrl+1**.
- Or right-click the object and choose **Format Shape…**. (For a picture it says **Format Picture…**.)
- Or click **Format Pane** on the **Shape Format** tab. A second click closes it.
- Or click the small button at the corner of a group on the **Shape Format** tab. The corner of **Shape Styles** opens the **Fill & Line** page, and the corner of **Size** opens the **Size & Properties** page.

To close it, click the **×** at its top right, or click **Format Pane** again. JeffDeck remembers whether the pane was open, so it opens the same way the next time you start it.

## How the pane works

- **It follows what you select.** Click another object and the pane shows that object. When nothing is selected, it says so.
- **Several objects at once.** Select more than one and a change goes to all of them. Where they differ, the box is blank. Change the box and every object takes your value. Each object keeps everything you didn't touch. For example, three shapes with three different colors keep their colors when you change their transparency.
- **Each change is one step.** **Ctrl+Z** takes back one change. When you drag a slider, the objects follow your pointer, and the whole drag is one step. It counts when you let go. A number you hold down with the arrow keys works the same way.
- **A group** takes a fill, a line, or an effect for everything inside it. Its size and place are its own.
- **Locked objects** keep their size and place.

The tabs are icons across the top: **Fill & Line**, **Effects**, **Size & Properties**, and, when you selected only pictures, **Picture**. Point at one to see its name. Each page is a column of **sections**. Click a section's name to fold it up or open it.

## Fill & Line

**Fill** is the color or picture inside a shape. Choose one kind:

- **No fill** leaves the inside clear.
- **Solid fill** is one color. **Color** opens the theme's colors. **Transparency** says how much shows through: 0% hides what is behind, and 100% shows all of it.
- **Gradient fill** blends colors. **Type** is **Linear** (along a line), **Radial** (out from the middle), **Rectangular**, or **Path** (along the outline). A linear gradient also has an **Angle**: 0° runs left to right, and 90° runs top to bottom.
  - The bar shows the colors. Each little marker below it is a **stop**, a place where the color is exactly the one you chose. Click a marker to choose it, then set its **Color**, its **Position** from 0% to 100%, and its **Transparency**.
  - Drag a marker, or press the **Left** and **Right** arrow keys, to move it. Press **Up** and **Down** to choose another one. **Add Stop** puts a new one halfway to the next. **Remove Stop**, or the **Delete** key, takes the chosen one away. A gradient keeps at least two. You can also double-click the bar to add a stop where you click.
- **Picture or texture fill** covers the inside with a picture from a file. Click **Insert…** and choose a file. Check **Tile picture as texture** to repeat the picture across the shape, at the **Scale** you set. Leave it clear to stretch the picture to fit. **Transparency** works here too.
- **Pattern fill** is a small repeating picture, such as stripes or dots. Click one of the 13 patterns, then choose its **Foreground** color (the lines) and its **Background** color (what is between them).

**Line** is the edge of a shape, or a line itself. Choose **No line**, **Solid line**, or **Gradient line**. A gradient line blends colors along its length. Then set:

- **Color** and **Transparency**, or the gradient's stops.
- **Width**, in points. A point is 1/72 of an inch.
- **Compound type**: a single line, or two or three lines side by side, thick and thin.
- **Dash type**: solid, dots, or dashes.
- **Cap type**: how the end of a line looks: **Flat** stops at the end, **Round** adds a half circle, and **Square** adds a half square.
- **Join type**: how a corner looks: **Round**, **Bevel** (cut off), or **Miter** (pointed).
- **Begin arrow type**, **Begin arrow size**, **End arrow type**, and **End arrow size**, for a line that has ends. A shape's edge has no ends, so these show only when a line is selected.

## Effects

An **effect** is a look added around an object. Each section has a box to turn it on, a list of **Presets** to start from, and boxes to change it. A preset only sets the boxes. You can change any of them after.

- **Shadow**: a dark copy behind the object. **Color** and **Transparency** are as for a fill. **Size** makes the shadow bigger or smaller than the object, from the object's middle. **Blur** softens its edge. **Angle** says where it falls (0° is right, 90° is down), and **Distance** says how far.
- **Reflection**: a faded, upside-down copy under the object. **Transparency** is how faint it starts. **Size** is how far down it fades out. **Distance** is the gap between the object and its reflection. **Blur** softens it.
- **Glow**: a soft ring of color around the object. **Color**, **Size**, and **Transparency**.
- **Soft Edges**: fades the edge of the object into what is behind it. **Size** is how wide the fade is.

## Size & Properties

**Size**:

- **Height** and **Width** are in the unit you chose in the options (inches, for example). You can type another unit, such as **2 cm**.
- **Rotation** turns the object, in degrees.
- **Scale height** and **Scale width** are percents. For a picture, 100% is the size the picture has in its file. For anything else they say 100% and make the object bigger or smaller by the percent you type.
- **Lock aspect ratio** keeps the proportions: a new width changes the height to match, and the other way round. A picture starts with it on. It also decides whether dragging a corner keeps the proportions. Hold **Shift** while you drag to do the opposite.

**Position**: **Horizontal position** and **Vertical position** say where the object's top left corner is, measured from the top left corner of the slide.

**Text Box** shows for text boxes and shapes:

- **Vertical alignment** puts the text at the top, middle, or bottom. The **Centered** choices also put the block of text in the middle across the box.
- **Text direction** turns all the text a quarter turn. It is for text boxes.
- **Autofit** says what happens when there is more text than room. **Do not Autofit** lets it run over. **Shrink text on overflow** makes the letters smaller to fit. **Resize shape to fit text** makes the box taller or shorter to fit. These two are for text boxes. The text in a shape keeps its size.
- **Left margin**, **Right margin**, **Top margin**, and **Bottom margin** are the room between the edge and the text.
- **Wrap text in shape**: when it is on, a line of text breaks at the edge. When it is off, the line goes on until you press Enter.
- **Columns** and **Spacing** set the number of columns of text, and the gap between them.

**Alt Text** is for people who can't see the slide. A screen reader says it aloud.

- **Title** is a short name.
- **Description** says what the object shows or why it is there. Press **Tab** or click away to keep what you typed.
- **Mark as decorative** is for an object that only adds looks, such as a line under a title. A screen reader skips it. JeffDeck doesn't ask for a description for it.

## Picture

When only pictures are selected, the **Picture** page shows:

- **Brightness** and **Contrast** make the picture lighter or darker, and make the light and dark parts differ more or less.
- **Recolor** can make the picture **Grayscale**, or **Black and White**. **Black and white level** says how light a spot must be to turn white.
- **Transparency** lets what is behind the picture show through.
- **Crop** shows where the whole picture is in its frame. **Picture width** and **Picture height** are the size of the whole picture. **Offset X** and **Offset Y** move it from the frame's middle. What is outside the frame is cut away, but it is kept. For handles you can drag, use **Crop** on the **Picture Format** tab.

These changes never touch the picture's data. They are saved beside it.

## The Shape Format tab

The **Shape Format** tab shows while a shape, a text box, a line, or a group of them is selected. It goes away when you select a picture, a table, or nothing.

- **Shapes** has **Change Shape**. It gives the shapes you selected another shape from the list in **Insert > Shapes**. Their text, fill, line, effects, place, and size stay as they are. A shape's yellow handles go back to the new shape's own, and points you moved by hand are replaced. It is for shapes only, not for text boxes, lines, or pictures. It is one step, so **Ctrl+Z** brings the old shape back.
- **Shape Styles** starts with a gallery of ready-made looks, and has **Shape Fill**, **Shape Outline**, and **Shape Effects**.
  - **The gallery** has four rows: **Outline Only** (a line, and no fill), **Subtle Effect**, **Moderate Effect**, and **Intense Effect** (each a fill with a line, and the last two with a shadow, stronger as you go down). Each row has six tiles, one for each of the six accent colors of your theme. Point at a tile to see its name, and click it to give every shape selected that look. It is one step.
  - A style comes from the theme: the fills, lines, and shadows are the theme's own, in the accent you chose. The same tile looks different in another theme, and a tile shows what it gives in the theme you have. The shape remembers which style it has. A style also sets the color of the words in the shape, except words you colored yourself.
  - **Shape Fill**, **Shape Outline**, and **Shape Effects** each open a menu of the choices above, so you don't need the pane for the common ones. They change one part of a style and leave the rest. The last choice of each, such as **More Fill Options…**, opens the pane on the right page.
  - **Shape Fill**: the theme's colors, **No Fill**, **More Fill Colors…**, **Picture…**, and **Gradient** (six starting looks).
  - **Shape Outline**: the colors, **No Outline**, **Weight**, **Dashes**, and **Arrows**.
  - **Shape Effects**: **Shadow**, **Reflection**, **Glow**, and **Soft Edges**, each with its presets.
- **Text Art Styles** changes the letters of the words in a shape or a text box, and Text Art. While you are typing in a box, it changes the words you selected. When you select the box itself, it changes all of its words. When you select Text Art, it changes the Text Art. It is off for an object that has no words.
  - **The gallery** has the same styles as **Insert > Text Art**. On a shape's words, a style gives the letters its fill, its outline, and its shadow, and leaves the font alone. On Text Art, it gives the whole style (the font and the bend too), and keeps the words and the size.
  - **Text Fill**: the theme's colors, **No Fill** (clear letters, so only an outline shows), **More Fill Colors…**, and **Gradient** (six starting looks, from the color the letters have).
  - **Text Outline**: the colors, **No Outline**, **More Outline Colors…**, and **Weight**. An outline starts one point wide.
  - **Text Effects**: **Shadow**, **Reflection**, and **Glow**, each with its presets, and **Transform**. Each effect you add stays beside the others. **Transform** bends Text Art into an arch, a circle, a wave, and more. The words in a shape are not bent, so it is off for them.
- **Accessibility** has **Alt Text**. It opens the pane on the **Alt Text** section, with the cursor in the description.
- **Size** has **Height** and **Width** boxes. Type a number to change the size of every object selected.
- **Format** has **Format Pane**, which opens and closes the pane.

Each choice in a menu is one step, like a change in the pane.

## Use it with the keyboard

- **Ctrl+1** opens the pane, and the keyboard goes to its tabs. Use the **Left** and **Right** arrow keys to go from tab to tab, and **Tab** to go on to the sections and boxes.
- **Space** or **Enter** on a section's name folds it up or opens it.
- In a number box, **Up** and **Down** change the number a little. Type a number and press **Enter** to set it.
- **Esc** takes the keyboard back to the slide.

## In the files

A .pptx file keeps every setting above. A .odp file keeps them too, but it has no place for a few of them, so JeffDeck writes those in a way only JeffDeck reads: a line with a gradient, a line of two or three strokes, a reflection, the size of a shadow, the aspect ratio lock, the columns, the mark for decoration, and the style a shape has. For letters, it is the gradient fill, the outline, the glow, the reflection, the blur of a shadow, and clear letters. Another program that opens that .odp file shows the line in its first color, as a single line, and without the reflection, and it shows the letters plain, in one color, with a shadow that has no blur. JeffDeck brings them all back. A pattern fill is saved as the nearest hatch or solid color, and a picture's black and white level is not saved in a .odp file.

A style is saved in a .pptx file the way the other programs save it. The shape says which of the theme's looks it uses (a line, a fill, a shadow, and a text color, each in an accent color), and its own settings say only what you changed after. So a shape you changed keeps your change, and a shape you didn't change looks the way the theme makes it, wherever the file is opened. A shape in a file you open keeps its style the same way. The letters' outline and gradient fill are saved in a .pptx file too.

## Related

- [Shapes, pictures, tables, and Text Art](objects)
- [Type and format text](text)
- [Home tab](tab-home)
- [Use JeffDeck with the keyboard](keyboard)
