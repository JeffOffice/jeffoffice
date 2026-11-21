# Draw shapes and lines
<!-- keywords: shape, line, arrow, rectangle, circle, oval, connector, curve, freeform, fill, outline, shadow, 3-D, edit points, flowchart -->

Shapes and lines are drawn objects. You can fill them with color, outline them, and type text inside a shape.

## Draw a shape

1. Click **Insert > Illustrations > Shapes**. (Also **Home > Objects > Shapes**.)
2. Pick a shape. The groups are Lines, Rectangles, Basic Shapes, Block Arrows, Equation Shapes, Flowchart, Stars and Banners, Callouts, and Action Buttons.
3. Drag on the page.

While you drag, hold **Shift** for an even shape: an Oval becomes a circle, and a Rectangle becomes a square. Hold **Ctrl** to draw from the center outward. If you click without dragging, you get a shape one inch across. After one shape, the pointer goes back to normal. Press **Esc** to cancel before you draw.

## Draw lines and arrows

Pick **Line**, **Arrow**, or **Double Arrow** from the gallery's Lines group, or from **Shape Format > Lines**. Drag on the page. Hold **Shift** to keep the line level, upright, or at a 45-degree angle. To change a line later, drag the handle at either end.

Three more tools draw by hand:

- **Curve**: click points. The line bends smoothly through them.
- **Freeform: Shape**: click points for straight sides, or hold the button and drag to draw freehand.
- **Freeform: Scribble**: hold the button and draw one stroke.

To finish a curve or freeform, double-click or press **Enter**. Click the first point to close it into a filled shape.

## Join objects with connectors

A line can stick to other objects. **Connector: Elbow** and **Connector: Curved** draw bent and curved ones. As you draw near a shape, text box, picture, or table, its attach points show. The end snaps to the middle of a side. When you move, resize, or turn the object, the line follows. Drag the yellow handle to move an elbow's bend. Drag a line away by itself to detach it.

## Change fill, outline, and effects

Select the shape and open the **Shape Format** tab.

- **Shape Styles**: the gallery gives quick looks.
- **Shape Fill**: pick a color, **No Fill**, **Picture Fill…**, or **Fill Effects…** for a gradient, texture, or pattern.
- **Shape Outline**: pick a color. The arrow beside it sets weight, dashes, and arrowheads.
- **Shape Effects**: **Shadow**, **Reflection**, **Glow**, **Soft Edges**, **Bevel**, and **3-D Rotation**. **Shadow Options…** sets the shadow's color, blur, distance, and angle. **3-D Rotation** tilts the shape in perspective.

For every setting at once, click the small button at the corner of the **Shape Styles** group to open the **Format Shape** window.

## Add text to a shape

Double-click the shape and type. You can also select it and press **Enter**, or right-click it and choose **Add Text**.

## Reshape a shape

- **Change Shape** (in **Insert Shapes**) swaps in another shape.
- A yellow diamond handle, on shapes that have one, adjusts the shape's form. On a rounded rectangle, it sets how round the corners are.
- **Edit Points** lets you move the corners. White squares mark each point. Drag a point to move it. **Ctrl**+click a point to delete it, or **Ctrl**+click an edge to add one. Press **Esc** when you are done.

## Glue points for connectors

A connector attaches to the connection points a shape has (the middle of each side, and more on some shapes). To add one of your own, select the shape, click **Edit Points**, and **Shift**+click where the point should be. A blue diamond marks it. Connectors can attach to it like to any other connection point, and follow the shape when it is moved or sized: the point keeps its place in the shape (a quarter of the way across stays a quarter of the way across). **Shift**+click a diamond to take it away; connectors attached to it are let go. A shape holds up to 16 glue points of its own. Each change is one step of Undo, and the points are kept when you save the file as a JeffPub file. A `.pub` file keeps a connector on the side its glue point is nearest.

## Merge shapes and convert outlines

Select two or more shapes on the same page, then click **Shape Format > Insert Shapes > Merge Shapes**. The new shape has the fill and line of the shape you selected first, and sits where the old ones were. The old shapes go, and so does any text in them. Each choice is one step of Undo, and the new shapes are left selected.

- **Union** makes one shape of everything the shapes cover.
- **Combine** keeps what only one of the shapes covers and cuts out the places where they overlap.
- **Subtract** keeps the first shape you selected without the others.
- **Intersect** keeps only what every shape covers.
- **Fragment** cuts the shapes into every piece they make with one another. Each piece is a shape of its own.

You can merge up to 64 shapes at once, and a result that would be cut into more than 256 pieces is refused. A shape that is locked, inside a group, or artwork stays as it is, and the status bar says so. A merged shape is a shape with points of its own, so **Edit Points** reshapes it, and it is kept in the file like any other.

**Shape Format > Insert Shapes > Convert** changes how the selected shapes are drawn:

- **Convert to Curve** gives the shape an outline of its own, so that its points can be moved (**Edit Points** does the same when you start to edit).
- **Convert to Polygon** makes every curve of the outline out of short straight lines.
- **Convert to Contour** makes the line of the shape a shape of its own, filled with the line's color. The shape needs a line.
- **Break Apart** makes a shape that has several parts (as a union of shapes that do not touch has) into one shape for each part. A ring breaks into its outer circle and its hole, each filled.

## Reroute connectors

A connector is a line whose ends are attached to shapes (draw it from one shape's connection point to another's). Select a shape, or the connector, and click **Shape Format > Insert Shapes > Reroute Connectors**. Each connector attached to the shape moves each attached end to the connection point of its shape that is nearest the other end, so the path is the shortest one. It is one step of Undo. If every connector is on its shortest path already, the status bar says so.

## Related

- [Move, align, group, and layer objects](arrange)
- [Add and adjust pictures](pictures)
- [Icons, barcodes, and other objects](insert-objects)
- [The Shape Format tab](tab-shape-format)
