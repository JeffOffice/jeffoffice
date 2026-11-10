/* Opens a file with the FFmpeg libraries it is linked to, decodes it all, and says how many pictures and sound frames came out. Used on the built libraries
 * (tools/ffmpeg_decode_check.sh), to tell a library that does not decode a format from a player that does not play it.
 *   ffmpeg_decode_check FILE      prints "ok FILE N frames", or "FAIL FILE: why"; exit 0 only when something was decoded */
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    AVFormatContext *f = NULL;
    int r = avformat_open_input(&f, argv[1], NULL, NULL);
    if (r < 0 || (r = avformat_find_stream_info(f, NULL)) < 0) { char e[128]; av_strerror(r, e, sizeof e); printf("FAIL %s: cannot open (%s)\n", argv[1], e); return 1; }
    AVCodecContext *cx[32] = {0};
    long frames[32] = {0};
    for (unsigned i = 0; i < f->nb_streams && i < 32; i++) {
        const AVCodec *c = avcodec_find_decoder(f->streams[i]->codecpar->codec_id);
        if (!c) { printf("  stream %u: no decoder for %s\n", i, avcodec_get_name(f->streams[i]->codecpar->codec_id)); continue; }
        cx[i] = avcodec_alloc_context3(c);
        avcodec_parameters_to_context(cx[i], f->streams[i]->codecpar);
        if (avcodec_open2(cx[i], c, NULL) < 0) { avcodec_free_context(&cx[i]); printf("  stream %u: cannot open %s\n", i, c->name); }
    }
    AVPacket *p = av_packet_alloc();
    AVFrame *fr = av_frame_alloc();
    while (av_read_frame(f, p) >= 0) {
        unsigned i = (unsigned)p->stream_index;
        if (i < 32 && cx[i] && avcodec_send_packet(cx[i], p) >= 0)
            while (avcodec_receive_frame(cx[i], fr) >= 0) frames[i]++;
        av_packet_unref(p);
    }
    long total = 0;
    for (int i = 0; i < 32; i++) total += frames[i];
    printf("%s %s %ld frames\n", total > 0 ? "ok  " : "FAIL", argv[1], total);
    return total > 0 ? 0 : 1;
}
