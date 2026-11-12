# Playback tab
<!-- keywords: playback tab, ribbon, movie, video, sound, audio, play, pause, preview, trim, captions, insert captions, remove captions, subtitles, webvtt, vtt, volume, mute, start, in click sequence, automatically, when clicked on, play full screen, hide while not playing, loop until stopped, rewind after playing, media tools -->

The **Playback** tab sets how a movie or a sound plays, and adds captions to a movie. It shows only while one movie or sound is selected, and it goes away when you select something else.

- **Preview** has **Play**. It plays the movie or sound on the slide so you can see and hear it. While it plays, the button says **Pause**.
- **Editing** has **Trim**. It opens a box with a **Start Time** and an **End Time**, in seconds. The movie or sound plays only from the start time to the end time. The rest of the file stays inside the presentation. To play to the end of the file again, set the end time to the full length.
  - **Insert Captions** adds captions to a movie from a WebVTT file (a text file that ends in `.vtt`). It asks for the file and then for the language of the captions, as a short code such as `en`, `es`, or `pt-BR`. Captions go with a movie that is kept inside the presentation; for a movie that is only linked from outside, the button is dimmed. A movie keeps one set of captions for each language: inserting a second file for the same language replaces the first. A file that isn't WebVTT, holds no captions, or is too large (the most is 4 MB, 20,000 captions, and 2,000 characters in one caption) is refused with a message, and nothing changes.
  - **Remove Captions** takes captions off the movie. If the movie has captions in more than one language, it asks which to remove, or all of them.
- **Media Options** says how it plays in the slide show:
  - **Volume** is **Low**, **Medium**, **High**, or **Mute**.
  - **Start** says what starts it. **In Click Sequence** plays it at a click, with the slide's animations. **Automatically** plays it by itself when its turn comes. **When Clicked On** plays it when you click the movie or sound.
  - **Play Full Screen** makes a movie fill the screen while it plays. It works only for movies.
  - **Hide While Not Playing** hides the picture until the movie or sound plays, and again after it stops. A sound's speaker picture is hidden the whole time.
  - **Loop until Stopped** plays it again each time it ends, until it is stopped or the slide changes.
  - **Rewind after Playing** takes a movie back to its first picture when it ends. Without it, the movie stays on its last picture.

Captions show over the bottom of the movie, in white text on a dark band, while the movie plays in the slide show and in the editor. They are timed to the movie, and their size follows the size of the movie. If a movie has more than one language, the first one shows. Captions are saved with the presentation as part of the `.pptx` file, so they come back when you open it again. A caption file in another format (not WebVTT) is kept as it is and is saved again unchanged, but it doesn't show. Once a movie has captions, **Check Accessibility** no longer lists it.

Each change is one step. **Ctrl+Z** takes it back.

A sound or movie that plays across slides, which some files set, keeps playing for as many slides as the file says. JeffDeck plays it that way in the slide show, and it keeps the setting when you save.

See [Shapes, pictures, tables, and Text Art](objects) and [Play the slide show and use Presenter View](show).

## Related

- [Shapes, pictures, tables, and Text Art](objects)
- [Insert tab](tab-insert)
- [Play the slide show and use Presenter View](show)
