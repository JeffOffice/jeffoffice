/* Lists what FFmpeg libraries really contain, by loading them: every demuxer, muxer, decoder, encoder, protocol, parser,
 * bitstream filter, and hardware device type. tools/check_ffmpeg_components.py --probe compares the list with what the build
 * script allows, so that what ships is checked from the built libraries themselves, not from the text lists they were made from.
 *
 *   ffmpeg_probe LIBDIR      prints "kind name" lines
 *
 * It needs no FFmpeg headers, so it runs on any libraries (ours on every system, the snap's older ones): the libraries are
 * opened by file name from LIBDIR (libavformat, libavcodec, libavutil: majors 61, 59 or 60, 58) and the few structures read
 * start with the fields that have not moved in any release (a name; for a codec, its type and id; for a parser, its codec ids).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
typedef HMODULE Lib;
static Lib openLib(const char *dir, const char *name, int major)
{
    char path[2048];
    snprintf(path, sizeof path, "%s/%s-%d.dll", dir, name, major);
    return LoadLibraryA(path);
}
static void *sym(Lib l, const char *n) { return (void *)GetProcAddress(l, n); }
#else
#include <dlfcn.h>
typedef void *Lib;
static Lib openLib(const char *dir, const char *name, int major)
{
    char path[2048];
#ifdef __APPLE__
    snprintf(path, sizeof path, "%s/lib%s.%d.dylib", dir, name, major);
#else
    snprintf(path, sizeof path, "%s/lib%s.so.%d", dir, name, major);
#endif
    return dlopen(path, RTLD_NOW | RTLD_LOCAL);
}
static void *sym(Lib l, const char *n) { return dlsym(l, n); }
#endif

typedef struct { const char *name, *long_name; int type, id; } CodecHead;   /* AVCodec, as far as it is read */
typedef struct { int codec_ids[7]; } ParserHead;                            /* AVCodecParser */
typedef struct { const char *name; } NameHead;                              /* AVInputFormat, AVOutputFormat, AVBitStreamFilter */

static void *need(Lib l, const char *n)
{
    void *p = sym(l, n);
    if (!p) { fprintf(stderr, "ffmpeg_probe: no %s\n", n); exit(3); }
    return p;
}

int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "usage: ffmpeg_probe LIBDIR\n"); return 2; }
    const int majors[][3] = {{61, 61, 59}, {60, 60, 58}};   /* avformat, avcodec, avutil */
    Lib fmt = 0, cod = 0, utl = 0;
    for (int i = 0; i < 2 && !(fmt && cod && utl); i++) {
        utl = openLib(argv[1], "avutil", majors[i][2]);
        cod = utl ? openLib(argv[1], "avcodec", majors[i][1]) : 0;
        fmt = cod ? openLib(argv[1], "avformat", majors[i][0]) : 0;
    }
    if (!(fmt && cod && utl)) { fprintf(stderr, "ffmpeg_probe: cannot load the libraries in %s\n", argv[1]); return 3; }

    const NameHead *(*demuxer)(void **) = need(fmt, "av_demuxer_iterate");
    const NameHead *(*muxer)(void **) = need(fmt, "av_muxer_iterate");
    const char *(*protocol)(void **, int) = need(fmt, "avio_enum_protocols");
    const CodecHead *(*codec)(void **) = need(cod, "av_codec_iterate");
    int (*isDecoder)(const CodecHead *) = need(cod, "av_codec_is_decoder");
    const ParserHead *(*parser)(void **) = need(cod, "av_parser_iterate");
    const NameHead *(*bsf)(void **) = need(cod, "av_bsf_iterate");
    const char *(*codecName)(int) = need(cod, "avcodec_get_name");
    int (*hwNext)(int) = need(utl, "av_hwdevice_iterate_types");
    const char *(*hwName)(int) = need(utl, "av_hwdevice_get_type_name");
    const char *(*version)(void) = need(utl, "av_version_info");
    unsigned (*avcodecVersion)(void) = need(cod, "avcodec_version");

    printf("version %s avcodec-major %u\n", version(), avcodecVersion() >> 16);
    void *it = 0;
    const NameHead *n;
    while ((n = demuxer(&it))) printf("demuxer %s\n", n->name);
    it = 0;
    while ((n = muxer(&it))) printf("muxer %s\n", n->name);
    it = 0;
    const CodecHead *c;
    while ((c = codec(&it))) {
        printf("%s %s\n", isDecoder(c) ? "decoder" : "encoder", c->name);
        if (isDecoder(c)) printf("decoder-codec %s\n", codecName(c->id));
    }
    it = 0;
    const char *p;
    while ((p = protocol(&it, 0))) printf("protocol-in %s\n", p);
    it = 0;
    while ((p = protocol(&it, 1))) printf("protocol-out %s\n", p);
    it = 0;
    const ParserHead *pp;
    while ((pp = parser(&it))) {
        printf("parser");
        for (int i = 0; i < 7 && pp->codec_ids[i]; i++) printf(" %s", codecName(pp->codec_ids[i]));
        printf("\n");
    }
    it = 0;
    while ((n = bsf(&it))) printf("bsf %s\n", n->name);
    for (int t = hwNext(0); t; t = hwNext(t)) printf("hwdevice %s\n", hwName(t));
    return 0;
}
