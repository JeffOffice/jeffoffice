# Animations
<!-- keywords: animation, animations, animation pane, effect, entrance, exit, emphasis, motion path, build, by paragraph, click step, start, on click, with previous, after previous, delay, duration, repeat, rewind, smooth, preview, effect options, add animation, animation painter, reorder, remove, custom, play -->

An **animation** moves or changes an object on a slide while the slide show plays: a bullet that fades in, a picture that flies in from the side, or a shape that travels along a path. Each animation is an **effect**. A slide can have as many effects as you like.

## Add an animation

1. Click the shape, picture, or text box on the slide. To animate several at once, hold Ctrl and click each one.
2. Click the **Animations** tab. In the **Animation** gallery, click an effect. Click the **Show all** button beside the gallery to see all of them.
3. Click **Preview** to watch the slide's animations play over the slide. Press Esc to stop.

Each effect is one of four kinds:

- **Entrance** effects bring an object onto the slide: **Appear**, **Fade**, **Fly In**, **Float In**, **Split**, **Wipe**, **Grow & Turn**, and **Zoom**. The object is hidden until its effect plays.
- **Emphasis** effects change an object that is already there: **Pulse**, **Teeter**, **Spin**, and **Grow/Shrink**.
- **Exit** effects take an object off the slide: **Disappear**, **Fade**, **Fly Out**, **Float Out**, **Split**, **Wipe**, **Shrink & Turn**, and **Zoom**.
- **Motion Paths** move an object along a path: **Lines**, **Arcs**, **Turns**, **Shapes**, **Loops**, and **Curves**. A small picture of the path is shown for each one you choose.

**None** takes the animation off the shape. If the shape already has an effect, the gallery changes that effect to the one you click. To give a shape a second effect, click **Add Animation** and choose from its gallery. The second effect starts on a click.

## Choose how an effect goes

Click **Effect Options** to see what the effect can do:

- A **direction**, such as **From Left** for **Fly In**, or **Clockwise** for **Spin**. For a motion path, the choices are the paths themselves.
- An **amount**. **Grow/Shrink** is **Tiny**, **Smaller**, **Larger**, or **Huge**, and goes **Both**, **Horizontal**, or **Vertical**. **Spin** is **Quarter Spin**, **Half Spin**, **Full Spin**, or **Two Spins**.
- **Sequence**, for a shape that has text: **As One Object** animates the whole shape at once, and **By Paragraph** gives each paragraph its own effect, so a list shows one paragraph at a time. A paragraph that is indented under another goes with the one above it.

**Show Additional Effect Options** opens a window with the same choices, and more. Its **Effect** tab has **Smooth start**, which makes the effect begin slowly, and **Smooth end**, which makes it slow down at the end. Its **Timing** tab has **Repeat**, which plays the effect again, and **Rewind when done playing**, which sets the object back the way it was when the effect ends. Its **Text Animation** tab chooses **All at once** or **By paragraph**.

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
