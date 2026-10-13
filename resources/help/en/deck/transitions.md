# Transitions between slides
<!-- keywords: transition, transitions, morph, fade, push, wipe, split, reveal, cover, uncover, zoom, cut, effect options, objects, words, characters, match, pair, name, direction, duration, advance slide, on mouse click, after, automatic, timing, apply to all, preview, star -->

A **transition** is the effect that plays as the slide show moves from one slide to the next. A transition belongs to the slide it comes into. The slide show plays it when it moves forward onto that slide.

## Add a transition

1. Click the slide in the slides pane.
2. Click the **Transitions** tab. In the **Transition to This Slide** gallery, click an effect: **None**, **Morph**, **Fade**, **Push**, **Wipe**, **Split**, **Reveal**, **Cover**, **Uncover**, **Zoom**, or **Cut**. The slide's picture in the slides pane gets a small star to show that it has one.
3. Click **Transitions > Preview > Preview** to see it played into the slide, over the slide area. Press Esc to stop it.

Choose **None** to take a transition away.

## Choose how it goes

Click **Effect Options** for the effect's choices:

- **Push**, **Wipe**, **Reveal**, **Cover**, and **Uncover** have a direction: **From Right**, **From Left**, **From Bottom**, or **From Top**.
- **Split** and **Zoom** go **Out** or **In**.
- **Morph** moves **Objects**, **Words**, or **Characters**. See below.
- **Fade** and **Cut** have no choices. A cut goes through black.

## Morph

**Morph** moves what two slides have in common. An **object** is one thing on a slide: a shape, a picture, a text box, a line, or a table. The show goes from one slide to the next. Each object that is on both slides glides from where the first slide has it to where the second slide has it. On the way it grows or shrinks, turns, and changes color. An object that is only on the first slide fades out. An object that is only on the second slide fades in.

To make a morph:

1. Right-click a slide in the slides pane and choose **Duplicate Slide**. The copy has the same objects.
2. On the copy, move an object, make it bigger, turn it, or change its color. You can also change its text, or add and delete objects.
3. Click the copy, then click **Transitions > Transition to This Slide > Morph**. A morph takes 2 seconds. You can change that with **Duration**.
4. Click **Preview** to watch it, or start the slide show.

### Which objects go together

JeffDeck pairs an object on the first slide with an object on the second slide. It tries these ways, in this order. Each object goes in one pair at most.

1. **A name that starts with !!.** Two objects with the same name, and the name starts with two exclamation marks, are always a pair. It can be any two objects, even a circle and a star.
2. **The same name.** Two objects of the same kind with the same name are a pair. A text box and a shape are different kinds.
3. **The same text.** Two text boxes with the same words are a pair, whatever their names are. So are two shapes with the same words.
4. **The same picture.** Two pictures of the same image are a pair, even if they have another name or another size.
5. **The same kind of shape.** Two circles are a pair. So are two lines, or two tables.

When several objects could be a pair, the ones nearest each other go together first.

The names of objects come from the presentation file. A presentation made elsewhere keeps the names it gave its objects, and JeffDeck uses them.

### How a pair moves

- Two objects of the same shape are one object that moves. Its place, size, turn, fill color, line color, and see-through amount all change a little at a time. A picture's crop changes, too.
- Two objects that are not the same shape, such as a circle and a square, move to the same places while one fades into the other.
- If the text of a pair is not the same, the old text fades into the new text.
- Objects that an animation hides are left out. An object that an animation has moved is taken from where it ended up.

### What moves: Objects, Words, or Characters

Click **Effect Options** to choose:

- **Objects.** The text of a pair stays in its object. The old text fades into the new.
- **Words.** A word that both texts have moves from its old place to its new place. The other words fade out and in. A mark such as a comma counts as a word of its own.
- **Characters.** The same, one letter or number at a time.

**Words** and **Characters** work on text boxes and shapes. A text that is longer than 16,000 characters, text that runs up and down, and text in a flipped shape play as **Objects**. So does a pair that has no word or character in common.

Morph is also a transition in other presentation programs (the 2019 versions and newer). JeffDeck reads and saves it the way they do. Older programs play a morph as a fade.

## Set the timing

The **Timing** group sets how long it takes and how the show leaves the slide.

- **Duration** is how many seconds the effect takes. A new transition takes 0.7 seconds. You can type from 0.01 to 59.
- **On Mouse Click** is on by default. The show moves on from this slide when the presenter clicks. Turn it off to move on with a key such as Space or Right instead. The slide's animations still play at a click.
- **After** makes the show move on by itself after the number of seconds you type. You can use **After** together with **On Mouse Click**: the show moves on at whichever comes first.

**Apply To All** gives every slide the transition and the timing of the slide you are on.

Each of these is one step that Ctrl+Z takes back.

## Transitions from other files

A presentation made elsewhere can have a transition JeffDeck has no effect for. It plays as a fade, and it is saved as it was, so it still works where it came from.

## Related

- [Transitions tab](tab-transitions)
- [Animations](animations)
- [Play the slide show and use Presenter View](show)
