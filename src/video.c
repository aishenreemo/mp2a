#include "screen.h"
#include "video.h"
#include "main.h"


int video_stream_index;

AVCodecParameters *video_codec_params;
AVCodecContext *video_codec_ctx;
AVCodec *video_codec;

struct SwsContext *video_sws_ctx;
AVFrame *rgb_frame;


enum mp2a_result_t find_video_stream() {
	FAIL_IF_TRUE(
		format_ctx == NULL,
		"error(video): AVFormatContext is NULL\n"
	);

	for (int i = 0; i < format_ctx->nb_streams; i++) {
		AVStream *stream = format_ctx->streams[i];

		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;

			video_codec_params = stream->codecpar;
			video_codec = (AVCodec *) avcodec_find_decoder(video_codec_params->codec_id);
			FAIL_IF_NULL(video_codec);
			video_codec_ctx = avcodec_alloc_context3(video_codec);
			FAIL_IF_NULL(video_codec_ctx);

			return MP2A_SUCCESS;
		}
	}

	return MP2A_FAILURE;
}


enum mp2a_result_t decode_video_packet() {
	FAIL_IF_NULL(video_codec_ctx);
	FAIL_IF_NULL(packet);
	FAIL_IF_NULL(frame);

	FAIL_IF_TRUE(
		avcodec_send_packet(video_codec_ctx, packet) < 0,
		"error: could not send packet to video codec\n"
	);


	while (true) {
		int result = avcodec_receive_frame(video_codec_ctx, frame);

		if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
			break;
		}

		FAIL_IF_TRUE(result < 0, "error: could not receive video frame from codec\n");

		display_frame();
	}

	return MP2A_SUCCESS;
}
