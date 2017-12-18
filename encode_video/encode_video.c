#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

static void save_packet(AVPacket *pkt, char *filename)
{
    FILE *f = fopen(filename, "wb");
	if (f == NULL) {
		perror("fopen faild:");
		exit(1);
	}
	printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
	fwrite(pkt->data, 1, pkt->size, f);
    fclose(f);
}

static void save_all_packets_one_file(AVPacket *pkt)
{
	static FILE *f = NULL;

	if (f == NULL) {
		f = fopen("output.hevc", "wb");
		if (f == NULL) {
			perror("fopen failed:");
			exit(1);
		}
	}
	printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
	fwrite(pkt->data, 1, pkt->size, f);
}

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt)
{
    int ret;
	char name[128];
	static int index = 0;

    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3"PRId64"\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

#if 0
        snprintf(name, sizeof(name), "sep_hevc/output_%06d.hevc", index++);
		save_packet(pkt, name);
#else
		save_all_packets_one_file(pkt);
#endif
        av_packet_unref(pkt);
    }
}

int main(int argc, char **argv)
{
    const char *filename, *codec_name;
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, ret, x, y;
    FILE *f;
    AVFrame *frame;
    AVPacket *pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	if (argc < 2) {
		printf("Usage: %s <input raw yuv420p file>\n", argv[0]);
		return -1;
	}
	filename = argv[1];
    codec_name = "libx265";

    avcodec_register_all();

    /* find the mpeg1video encoder */
    codec = avcodec_find_encoder_by_name(codec_name);
    if (!codec) {
        fprintf(stderr, "Codec '%s' not found\n", codec_name);
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 1920;
    c->height = 1080;
    /* frames per second */
    c->time_base = (AVRational){1, 25};
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);

    /* open it */
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        exit(1);
    }

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

	int uv_plane_size = frame->width * frame->height / 2;
	int u_v_plane_size = frame->width / 2 * frame->height / 2;
	int y_plane_size = frame->width * frame->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }

    /* encode 1 second of video */
    for (i = 0; i < 100; i++) {
        fflush(stdout);

        /* make sure the frame data is writable */
        ret = av_frame_make_writable(frame);
        if (ret < 0)
            exit(1);

#if 0 // NV12
		fread(frame->data[0], y_plane_size, 1, f);  /* Y */
		fread(frame->data[1], uv_plane_size, 1, f); /* Cb and Cr */
#else // i420(yuv420p)
		fread(frame->data[0], y_plane_size, 1, f);  /* Y */
		fread(frame->data[1], u_v_plane_size, 1, f); /* U */
		fread(frame->data[2], u_v_plane_size, 1, f); /* V */
#endif

        frame->pts = i;

        /* encode the image */
        encode(c, frame, pkt);
    }

    /* flush the encoder */
    encode(c, NULL, pkt);

    /* add sequence end code to have a real MPEG file */
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);

    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;
}
