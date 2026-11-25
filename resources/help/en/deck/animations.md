# Animations
<!-- keywords: animation, animations, animation pane, effect, entrance, exit, emphasis, motion path, build, by paragraph, click step, start, on click, with previous, after previous, delay, duration, repeat, rewind, smooth, preview, effect options, add animation, animation painter, reorder, remove, custom, play -->

An **animation** moves or changes an object on a slide while the slide show plays: a bullet that fades in, a picture that flies in from the side, or a shape that travels along a path. Each animation is an **effect**. A slide can have as many effects as you like.

## Add an animation

1. Click the shape, picture, or text box on the slide. To animate several at once, hold Ctrl and click each one.
2. Click the **Animations** tab. In the **Animation** gallery, click an effect. Click the **Show all** button beside the gallery to see all of them.
3. Click **Preview** to watch the slide's animations play over the slide. Press Esc to stop.

Each effect is one of four kinds:

- **Entrance** effects bring an object onto the slide: **Appear**, **Fade**, **Fly In**, **Float In**, **Split**, **Wipe**, **Grow & Turn**, **Zoom**, **Blinds**, **Box**, **Checkerboard**, **Shape**, **Diamond**, **Plus**, **Dissolve In**, **Random Bars**, **Wheel**, **Peek In**, **Flash Once**, **Stretch**, **Swivel**, **Expand**, **Spiral In**, and **Bounce**. The object is hidden until its effect plays. The ones that uncover the object in pieces (**Blinds**, **Checkerboard**, **Random Bars**, **Dissolve In**, and the forms) show it through a mask, so a picture, a chart, or text is cut the same way.
- **Emphasis** effects change an object that is already there: **Pulse**, **Teeter**, **Spin**, **Grow/Shrink**, and **Transparency**. Effect Options sets how clear a Transparency makes the object.
- **Exit** effects take an object off the slide: **Disappear**, **Fade**, **Fly Out**, **Float Out**, **Split**, **Wipe**, **Shrink & Turn**, **Zoom**, and the same effects as the entrances, going the other way (**Dissolve Out**, **Peek Out**, **Contract**, **Collapse**, and **Spiral Out** are named for it).
- **Motion Paths** move an object along a path: **Lines**, **Arcs**, **Turns**, **Shapes**, **Loops**, and **Curves**. A small picture of the path is shown for each one you choose.

**None** takes the animation off the shape. If the shape already has an effect, the gallery changes that effect to the one you click. To give a shape a second effect, click **Add Animation** and choose from its gallery. The second effect starts on a click.

## Choose how an effect goes

Click **Effect Options** to see what the effect can do:

- A **direction**, such as **From Left** for **Fly In**, or **Clockwise** for **Spin**. For a motion path, the choices are the paths themselves.
- An **amount**. **Grow/Shrink** is **Tiny**, **Smaller**, **Larger**, or **Huge**, and goes **Both**, **Horizontal**, or **Vertical**. **Spin** is **Quarter Spin**, **Half Spin**, **Full Spin**, or **Two Spins**.
- **Sequence**, for a shape that has text: **As One Object** animates the whole shape at once, and **By Paragraph** gives each paragraph its own effect, so a list shows one paragraph at a time. A paragraph that is indented under another goes with the one above it.

**Show Additional Effect Options** opens a window with the same choices, and more. Its **Effect** tab has **Smooth start**, which makes the effect begin slowly, and **Smooth end**, which makes it slow down at the end. **After animation** says what happens to the object when the effect is over: **Don't Dim** leaves it as it is, **Dim to a Color** turns it toward the color you pick in **Dim color** (a theme color or a color of your own) at the next click, so that it fades into the background, **Hide After Animation** hides it at once, and **Hide on Next Click** hides it at the next click. **Sound** plays a sound when the effect starts: **No Sound**, **Stop Previous Sound** (which stops the sounds the effects before it started), or **Other Sound…**, which keeps a sound file inside the presentation (up to 32 MB). The sound plays in the slide show and when you play the effects in the Animation pane, and it stops when the slide changes. At most eight effect sounds play at the same time. A file saved as OpenDocument does not keep it. Its **Timing** tab has **Repeat**, which plays the effect again, and **Rewind when done playing**, which sets the object back the way it was when the effect ends. Its **Text Animation** tab chooses **All at once** or **By paragraph**, and, under **Animate text**, **All at once**, **By word**, or **By letter**. By word and by letter play the effect on one word or letter after another; the delay between them is a percent of how long the effect takes, so 10 percent of a one-second effect starts each word a tenth of a second after the one before. Text with more than 300 words or letters is animated in groups of them, so that the slide show stays smooth. A letter is one whole character as you see it, so a letter with its accent, or an emoji made of several parts, comes in as one piece, and a bullet or number comes in with the first word or letter of its paragraph. Only the fading, appearing, and moving of each word or letter is played one by one; a turn, a change of size, or a wipe plays as a fade.

## Set when it plays

The **Timing** group sets the effect you picked:

- **Start** says when the effect begins. **On Click** waits for a click in the slide show. **With Previous** begins together with the effect before it. **After Previous** begins when the effect before it ends.
- **Duration** is how many seconds the effect takes. An effect such as **Appear** is instant, so it has none.
- **Delay** is how many seconds the effect waits before it begins.

Under **Reorder Animation**, **Move Earlier** and **Move Later** change the order the effects play in. Each effect you start **On Click** is one click in the slide show; the effects that start with it or after it play in the same click.

## The Animation Pane

Click **Animation Pane** to open a list of the slide's effects on the right of the window. Each row shows:

- A number in a box, which is the click the effect plays at. An effect that starts with or after the one before it has a small bracket instead.
- A small picture of the effect.
- The name of the shape, and the effect with how it starts. For a paragraph effect the name has the start of the paragraph.
- A bar for when the effect plays after its click: it starts at the delay and lasts for the duration.

Click a row to pick its effect. The shape is picked on the slide too, and the other way around. Hold Ctrl or Shift to pick several rows. The **Timing** group then changes all of them.

- Drag a row up or down, or click the **Re-Order** arrows, to change the order.
- **Play All** plays every effect. **Play From** plays from the picked one.
- Right-click a row for **On Click**, **With Previous**, **After Previous**, **Effect Options**, **Timing**, and **Remove**. Double-click a row to open its options. Press Delete to remove the picked effects.

While the **Animations** tab or the pane is showing, a small number sits at the top left of each animated shape on the slide. It is the click the effect plays at. The number of the effect you picked is filled in.

## Effects that wait for a click on a shape

A presentation made in another program can have effects that do not wait for the next click. They wait for a click on one shape instead. In a slide show or in Reading View, clicking that shape plays those effects at once. A click anywhere else still plays the next effect of the slide, as before. Click the shape again and its effects play again. The pointer turns into a hand over such a shape, and the Tab key reaches it in the same order as a link: press Tab until its ring shows, then press Enter. Going back one step takes back what the shape's click played. If a shape has several of these sets of effects, one click plays all of them. If the shape cannot be clicked in the show (it is hidden, off the slide, or has a link of its own), what waits for it is simply shown.

JeffDeck keeps these effects when you save a .pptx or .odp file.

The **Animation Pane** lists them after the effects of the slide. Each set has a heading, such as "When Box 2 is clicked", over its effects.

To make one, pick an effect in the pane, open the **Start** box on the **Animations** tab, and choose **When an Object Is Clicked**. Pick the object from the list. The effect is then in that object's set, at the end. An object that the effect makes appear is not in the list for that effect, because nobody can click it before it appears.

To change a set, work in the pane as you do for other effects. Drag an effect onto another row to put it there, even in another set or in the slide's own effects. The arrows put an effect earlier or later inside its set. **Delete** removes it. Each change is one step that **Undo** takes back. Choosing **On Click**, **With Previous**, or **After Previous** for an effect in a set moves it to the end of the slide's own effects.

A slide can have effects that wait for a click on up to 64 objects, up to 200 effects for each object, and up to 1,000 in all. JeffDeck tells you when a change would go past these.

If you delete or cut an object that effects wait for, those effects move to the end of the slide's own effects, and the status bar says so. **Undo** brings the object and its effects back.

To see a set play, pick one of its effects in the pane and click **Play From**. Only that object's effects play.

## Copy an animation

Click a shape that has an animation, click **Animation Painter**, and then click another shape. The second shape gets the same effects.

## Remove an animation

Pick the effect and choose **None** in the gallery, click **Remove** in the right-click menu, or press Delete in the pane. If you delete a shape, its effects go with it.

## Every change can be undone

Each of these is one step that Ctrl+Z takes back.

## Animations in the slide show

Each click in the slide show plays the next **step**. For example, a title can appear first, and then each bullet can appear with its own click. Some steps start by themselves when the slide appears, or right after the step before.

In the slide show, a click while a step plays finishes that step and does no more. Press Left, or Backspace, or P to take a step back. See [Play the slide show and use Presenter View](show).

## Animations from other files

A presentation made in another program can have effects JeffDeck has no form for. They are listed as **Custom**. JeffDeck keeps each one just as it was and saves it that way, so it still works where it came from. You can move a custom effect, remove it, and change how it starts, but not its options. The slide show plays the other effects as a fade, a move, a turn, or a change of size, whichever they do. Changes of color and sounds don't play.

## Related

- [Transitions between slides](transitions)
- [Play the slide show and use Presenter View](show)
- [Welcome to JeffDeck](start)
