# Move, align, group, and layer objects
<!-- keywords: selection pane, hide object, show object, rename object, object name, select, move, resize, rotate, flip, align, distribute, group, ungroup, layer, bring to front, send to back, nudge, lock, duplicate -->

Every picture, text box, shape, and line on a page is an object. This topic shows how to select, move, resize, and layer objects.

## Select objects

- Click an object. To select a text box, click inside it and then press **Esc**.
- Hold **Shift** and click to add an object to the selection, or to remove it.
- Drag a box around objects on an empty part of the page. Objects that fit completely inside the box are selected.
- Press **Ctrl+A** to select every object on the page. Press **Tab** to step to the next object.
- **Home > Editing > Select** can select all the text boxes, pictures, shapes, tables, or Text Art at once.

## Move and resize

- Drag an object to move it. Hold **Shift** to move it only straight across or straight up and down. Hold **Alt** to turn snapping off. Hold **Ctrl** while you drag to make a copy.
- Press the arrow keys to nudge the object by one eighth of an inch. **Alt** with an arrow key moves it one screen pixel. You can change the nudge distance in **File > Options > Advanced**.
- Drag a square handle to resize. A corner handle changes width and height together. A side handle changes only one.
- Text boxes and shapes: hold **Shift** while you drag a corner to keep the proportions.
- Pictures, Text Art, icons, and groups already keep their proportions at a corner. Hold **Shift** to stretch them freely.
- Hold **Ctrl** to resize from the centre.
- For exact numbers, type in the **Height** and **Width** boxes of the **Size** group (on tabs such as **Shape Format**), or right-click the object, choose **Format Object…**, and use the **Size** and **Layout** tabs. **View > Tools > Measurement** opens a small window that shows and sets position, size, and rotation.

## Rotate and flip

Drag the round green handle above a selected object. Hold **Shift** to turn in 15-degree steps. Or click **Home > Arrange > Rotate** and choose **Rotate Right 90°**, **Rotate Left 90°**, **Flip Vertical**, **Flip Horizontal**, or **More Rotation Options…** to type an angle.

## Align and distribute

1. Select the objects.
2. Click **Home > Arrange > Align**.
3. Choose Align Left, Centre, Right, Top, Middle, or Bottom.

With several objects selected, they line up with each other. With one object selected, it lines up with the page edges. Check **Relative to Margin Guides** in the same menu to use the margin guides instead.

**Distribute Horizontally** and **Distribute Vertically** space three or more objects evenly between the outer two.

## Make objects the same size

1. Select two or more objects. Select first the one whose size the others should take.
2. Click **Home > Arrange > Align**.
3. Choose **Equalise Width**, **Equalise Height**, or **Equalise Width and Height**.

Every other object takes the width and height of that one (a turned object its own width and height, not those of its turned outline) and keeps its top left corner where it was. An object that is locked keeps its own size. Each choice is one step of Undo.

## Corner radius and the point to resize from

**Format Object… > Size** has **Resize from**. It names the point of the object that stays where it is when you change the height or the width: a corner, the middle of a side, or the centre. The usual choice is **Top left**. On a turned object the point stays where you see it on the page. If you also type a new position, the position you typed wins. For a rounded rectangle (and the shapes with one or two rounded corners), **Corner radius** sets how round the corners are, in the unit you use. The most it can be is half of the shorter side, where the corners meet.

## Duplicate several times

Ctrl+D makes one copy, moved 12 points down and to the right. To make more, select the objects and click **Home > Arrange > Duplicate Several Times…** (or right-click the object and choose it). In the window, set:

- **Number of copies**, from 1 to 200. No more than 2,000 new objects are made at a time, and the copies stop before the first that would be more than 20 times the larger side of the page, smaller than a point, or moved further than that.
- **Horizontal offset** and **Vertical offset**: how far each copy is moved from the one before it. A negative number moves it left or up.
- **Size change for each copy**, in percent. For example, 10 % makes each copy 10 % bigger than the one before it, and -10 % makes each smaller. The top left corner of each copy is where its offset puts it.
- **Step the fill colour to**: pick a colour, and the copies change step by step from the object's own solid fill to that colour, so the last copy has exactly that colour. An object with no solid fill keeps its fill.

All the copies are one step of Undo, and the last copies are left selected.

## Send objects to the master page

1. Select the objects on a page.
2. Click **Home > Arrange > Send to Master Page**.

The objects move onto the master page that the page uses, in front of what is already there, and they look the same: they stay in the same place on the page. After that they show on every page that uses the master, and you change them on the master page. To bring them back, open the master page (**View > Views > Master Page**), select them, and click **Send to Page** (the same button), and they land on the page you were on. The objects leave the page, so they show on every page that uses that master, and not only on this one. Each is one step of Undo. A page with no master page, a locked object, an object inside a group (select the whole group instead), a text box linked to another one, and an object kept with its page (a chart or an embedded object) stay where they are, and the status bar says why.

## Group objects

A group moves, resizes, and turns as one object.

1. Select two or more objects.
2. Click **Home > Arrange > Group** (Ctrl+Shift+G).

To take a group apart, select it and click **Ungroup**. The arrow beside **Ungroup** offers **Regroup**, which puts the last group back together. To select one object inside a group, double-click it. Press **Esc** to select the whole group again.

## Change the stacking order

Objects stack like sheets of paper. Use **Bring Forward** and **Send Backward** to move an object one step. The arrow beside each button offers **Bring to Front** (Alt+F6) and **Send to Back** (Alt+Shift+F6). Right-click and choose **Order** for the same choices.

## List the objects in the Selection Pane

**Arrange > Selection Pane** opens a list of every object on the page, with the one in front first. It helps when objects hide each other.

- Click a name to select that object.
- Double-click a name, or press **F2**, to type a new name for the object. An object with no name of its own shows its kind and its number, such as Shape 2.
- Clear the box beside a name to hide the object. A hidden object is not drawn on the page, in a printout, or in a PDF, and a click on the page does not pick it. Check the box to show it again. **Show All** and **Hide All** set every box at once.
- Select a name and click **Bring Forward** or **Send Backward** to move the object one place in the list, or drag a name up or down.

Every one of these is a step you can undo. A name and a hidden mark are saved in a .jpub file. A .pub file has no place for them.

## Lock an object

Right-click an object and choose **Lock Position and Size**. It cannot be moved or resized, and its handles turn into red X marks. Choose it again to unlock.

## Related

- [Wrap text around objects](wrap)
- [Rulers, guides, and snapping](guides)
- [The Shape Format tab](tab-shape-format)
