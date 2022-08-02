#include <stdio.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>

static AVFrame *pAVFrame;
static AVRational timebase;
static int frame_cnt = 0;
static int chosen_frame_num = 1;

int get_sample_frame(const char *filename, int frame_num, AVFrame *frame);
static int open_codec_context(int *stream_idx, // comment to control clang-format line break, do not remove
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
static int decode_video(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *last_frm);
static int print_frame(AVFrame *frame);

static int print_frame(AVFrame *frame)
{
	// Examine and process frame in the decoding loop here
	return 0;
	printf("Video frame coded_n: %d\n", frame->coded_picture_number);
	printf("\ttime: %f\n", av_q2d(timebase) * (double)frame->best_effort_timestamp);
	printf("\twidth: %d, height: %d, format: %s\n", //
	       frame->width, frame->height, av_get_pix_fmt_name(frame->format));
}

static int decode_video(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *last_frm)
{
	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "send_packet: %s\n", av_err2str(ret));
		return ret;
	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(dec_ctx, pAVFrame);
		if (ret < 0) {
			if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) return 0;

			fprintf(stderr, "receive_frame: %s\n", av_err2str(ret));
			return ret;
		}

		print_frame(pAVFrame);

		frame_cnt++;
		if (frame_cnt >= chosen_frame_num) {
			// Decoded past chosen number of frames, copy and return
			av_frame_ref(last_frm, pAVFrame);
			frame_cnt = 0;
			return 1;
		}

		av_frame_unref(pAVFrame);
	}

	return 0;
}

static int open_codec_context(int *stream_idx, //
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
	int ret, stream_index;
	AVStream *st;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;

	if ((ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0)) < 0) {
		fprintf(stderr, "Cannot find %s stream!\n", av_get_media_type_string(type));
	}

	stream_index = ret;
	st = fmt_ctx->streams[stream_index];

	dec = avcodec_find_decoder(st->codecpar->codec_id);
	if (!dec) {
		fprintf(stderr, "Failed to find a codec\n");
		return AVERROR(EINVAL);
	}

	*dec_ctx = avcodec_alloc_context3(dec);
	if (!*dec_ctx) {
		fprintf(stderr, "Failed to allocate AVCodecContext\n");
		return AVERROR(ENOMEM);
	}

	if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
		fprintf(stderr, "Failed to set decoder parameter: %s\n", av_err2str(ret));
		return ret;
	}

	if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
		fprintf(stderr, "Failed to open codec: %s\n", av_err2str(ret));
		return ret;
	}

	*stream_idx = stream_index;
	return 0;
}

int get_sample_frame(const char *filename, int frame_num, AVFrame *frame)
{
	int ret, video_stream_idx;

	AVCodecContext *dec_ctx = NULL;

	AVStream *stream = NULL;
	AVPacket *pkt = NULL;

	chosen_frame_num = frame_num;

	// Open Sample File
	AVFormatContext *fmt_ctx = avformat_alloc_context();
	if (fmt_ctx == NULL) {
		ret = AVERROR(ENOMEM);
		fprintf(stderr, "alloc_context: ENOMEM\n");
		goto end;
	}

	ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL);
	if (ret < 0) {
		fprintf(stderr, "open_input: %s\n", av_err2str(ret));
		goto end;
	}

	if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
		fprintf(stderr, "find_stream_info: %s\n", av_err2str(ret));
		goto end;
	}

	if (open_codec_context(&video_stream_idx, &dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) < 0) {
		fprintf(stderr, "open_codec_context: %s\n", av_err2str(ret));
		goto end;
	}

	stream = fmt_ctx->streams[video_stream_idx];
	timebase = stream->time_base;
	//av_dump_format(fmt_ctx, 0, "yuv444.mkv", 0);

	pAVFrame = av_frame_alloc();
	if (!pAVFrame) {
		fprintf(stderr, "alloc_frame: ENOMEM\n");
		goto end;
	}

	pkt = av_packet_alloc();
	if (!pkt) {
		fprintf(stderr, "alloc_packet: ENOMEM\n");
		goto end;
	}

	while (av_read_frame(fmt_ctx, pkt) >= 0) {
		if (pkt->stream_index == video_stream_idx) {
			ret = decode_video(dec_ctx, pkt, frame);
		}
		av_packet_unref(pkt);
		if (ret != 0) goto end;
	}


end:
	av_frame_free(&pAVFrame);
	av_packet_free(&pkt);
	avcodec_free_context(&dec_ctx);
	avformat_close_input(&fmt_ctx);
	avformat_free_context(fmt_ctx);
	return ret;
}
