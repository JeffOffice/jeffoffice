Sample movies and sounds for JeffDeck's media tests (tests/deck/tst_deck_media.cpp).

All of them are made from nothing by make_samples.sh, which runs make_sample.c: a gradient that moves (64 by 48 pictures, 10 a second) and sine
tones, two seconds each, encoded and joined by a full FFmpeg 7.x (its libx264, libx265, libvpx, libmp3lame, libvorbis, libopus, libtheora, and libsvtav1
among them). No file of anyone else's is in this folder, and nothing here came from a presentation. The whole folder is under a megabyte.

  kept/          formats the FFmpeg that JeffDeck packages reads (tools/build_ffmpeg.sh): the tests play every one
  dropped/       movies in formats it leaves out (Flash video, Ogg video, AV1): the tests expect "can't be played"
  audio_dropped/ a movie whose sound (TrueHD) it leaves out: the tests expect "can't be played"

The dropped/ tests run only where JO_MINIMAL_FFMPEG is set, because a Qt with the full FFmpeg plays those files.
