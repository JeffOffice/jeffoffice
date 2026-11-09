// Makes one synthetic movie or sound file: a moving gradient picture and sine tones, encoded and joined by a full FFmpeg (7.x, its development files).
//   make_sample OUT VIDEO_ENCODER|- AUDIO_ENCODER|- SECONDS     (environment: W, H, FPS, CH, RATE, BR, STRICT)
// The kind of file comes from OUT's name. tests/data/media/make_samples.sh runs it for every sample in this folder.
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define ENVI(n, d) (getenv(n) ? atoi(getenv(n)) : (d))
#define W ENVI("W", 160)
#define H ENVI("H", 120)
#define FPS ENVI("FPS", 15)
#define CH ENVI("CH", 2)
typedef struct { AVStream *st; AVCodecContext *c; AVFrame *f; AVPacket *p; int64_t n; } S;
static int fl(AVFormatContext *o, S *s, AVFrame *f) {
    if (avcodec_send_frame(s->c, f) < 0) return -1;
    for (;;) { int r = avcodec_receive_packet(s->c, s->p); if (r < 0) break;
        av_packet_rescale_ts(s->p, s->c->time_base, s->st->time_base); s->p->stream_index = s->st->index;
        av_interleaved_write_frame(o, s->p); }
    return 0;
}
int main(int argc, char **argv) {
    const char *out = argv[1], *vn = argv[2], *an = argv[3]; double secs = atof(argv[4]);
    AVFormatContext *o = NULL; avformat_alloc_output_context2(&o, NULL, NULL, out);
    if (!o) { fprintf(stderr, "no muxer for %s\n", out); return 1; }
    S v = {0}, a = {0};
    if (strcmp(vn, "-")) {
        const AVCodec *c = avcodec_find_encoder_by_name(vn); if (!c) { fprintf(stderr, "no encoder %s\n", vn); return 1; }
        v.c = avcodec_alloc_context3(c); v.c->width = W; v.c->height = H; v.c->time_base = (AVRational){1, FPS}; v.c->framerate = (AVRational){FPS, 1};
        v.c->pix_fmt = c->pix_fmts ? c->pix_fmts[0] : AV_PIX_FMT_YUV420P; v.c->gop_size = 10; v.c->bit_rate = 400000;
        if (o->oformat->flags & AVFMT_GLOBALHEADER) v.c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        if (avcodec_open2(v.c, c, NULL) < 0) { fprintf(stderr, "open %s failed\n", vn); return 1; }
        v.st = avformat_new_stream(o, NULL); avcodec_parameters_from_context(v.st->codecpar, v.c); v.st->time_base = v.c->time_base;
        v.f = av_frame_alloc(); v.f->format = v.c->pix_fmt; v.f->width = W; v.f->height = H; av_frame_get_buffer(v.f, 0); v.p = av_packet_alloc();
    }
    if (strcmp(an, "-")) {
        const AVCodec *c = avcodec_find_encoder_by_name(an); if (!c) { fprintf(stderr, "no encoder %s\n", an); return 1; }
        a.c = avcodec_alloc_context3(c); if (getenv("STRICT")) a.c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL; a.c->sample_fmt = c->sample_fmts ? c->sample_fmts[0] : AV_SAMPLE_FMT_S16;
        a.c->sample_rate = getenv("RATE") ? atoi(getenv("RATE")) : 44100; av_channel_layout_default(&a.c->ch_layout, CH); a.c->bit_rate = ENVI("BR", 32000); a.c->time_base = (AVRational){1, a.c->sample_rate};
        if (o->oformat->flags & AVFMT_GLOBALHEADER) a.c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        if (avcodec_open2(a.c, c, NULL) < 0) { fprintf(stderr, "open %s failed\n", an); return 1; }
        a.st = avformat_new_stream(o, NULL); avcodec_parameters_from_context(a.st->codecpar, a.c); a.st->time_base = a.c->time_base;
        a.f = av_frame_alloc(); a.f->format = a.c->sample_fmt; a.f->sample_rate = a.c->sample_rate; av_channel_layout_copy(&a.f->ch_layout, &a.c->ch_layout);
        a.f->nb_samples = a.c->frame_size ? a.c->frame_size : 1024; av_frame_get_buffer(a.f, 0); a.p = av_packet_alloc();
    }
    if (!(o->oformat->flags & AVFMT_NOFILE)) avio_open(&o->pb, out, AVIO_FLAG_WRITE);
    if (avformat_write_header(o, NULL) < 0) { fprintf(stderr, "header failed\n"); return 1; }
    int64_t vframes = v.c ? (int64_t)(secs * FPS) : 0, asamples = a.c ? (int64_t)(secs * a.c->sample_rate) : 0;
    while (v.n < vframes || a.n < asamples) {
        int dov = v.c && v.n < vframes && (!a.c || a.n >= asamples || (double)v.n / FPS <= (double)a.n / (a.c ? a.c->sample_rate : 44100));
        if (dov) {
            av_frame_make_writable(v.f);
            if (av_pix_fmt_count_planes(v.f->format) == 3) {
                for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) v.f->data[0][y * v.f->linesize[0] + x] = (x + y + v.n * 6) & 255;
                for (int y = 0; y < H / 2; y++) for (int x = 0; x < W / 2; x++) { v.f->data[1][y * v.f->linesize[1] + x] = 128 + (v.n * 3 & 63); v.f->data[2][y * v.f->linesize[2] + x] = 128 - (x & 63); }
            } else {   // packed or grey pictures (an older codec's own formats): the same moving gradient over every byte
                for (int y = 0; y < H; y++) for (int x = 0; x < v.f->linesize[0]; x++) v.f->data[0][y * v.f->linesize[0] + x] = (x + y + v.n * 6) & 255;
            }
            v.f->pts = v.n++; fl(o, &v, v.f);
        } else {
            av_frame_make_writable(a.f);
            int ns = a.f->nb_samples, planar = av_sample_fmt_is_planar(a.f->format);
            for (int i = 0; i < ns; i++) for (int ch = 0; ch < CH; ch++) { double s = 0.4 * sin(2 * M_PI * (ch ? 660 : 440) * (a.n + i) / (double)a.c->sample_rate);
                uint8_t *p = planar ? a.f->data[ch] : a.f->data[0]; int idx = planar ? i : i * CH + ch;
                switch (a.f->format) { case AV_SAMPLE_FMT_S16: case AV_SAMPLE_FMT_S16P: ((int16_t *)p)[idx] = (int16_t)(s * 32767); break;
                    case AV_SAMPLE_FMT_FLT: case AV_SAMPLE_FMT_FLTP: ((float *)p)[idx] = (float)s; break;
                    case AV_SAMPLE_FMT_S32: case AV_SAMPLE_FMT_S32P: ((int32_t *)p)[idx] = (int32_t)(s * 2147483647.0); break;
                    case AV_SAMPLE_FMT_U8: ((uint8_t *)p)[idx] = (uint8_t)(128 + s * 127); break; default: break; } }
            a.f->pts = a.n; a.n += ns; fl(o, &a, a.f);
        }
    }
    if (v.c) fl(o, &v, NULL); if (a.c) fl(o, &a, NULL);
    av_write_trailer(o); if (!(o->oformat->flags & AVFMT_NOFILE)) avio_closep(&o->pb); return 0;
}
