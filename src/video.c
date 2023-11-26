#include <unistd.h>

#include "screen.h"
#include "video.h"
#include "main.h"


int video_stream_index;

AVCodecParameters *video_codec_params;
AVCodecContext *video_codec_ctx;
AVCodec *video_codec;


enum mp2a_result_t find_video_stream() {
	EXIT_IF_TRUE(
		format_ctx == NULL,
		"error(video): AVFormatContext is NULL\n"
	);

	for (int i = 0; i < format_ctx->nb_streams; i++) {
		AVStream *stream = format_ctx->streams[i];

		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;

			video_codec_params = stream->codecpar;
			video_codec = (AVCodec *) avcodec_find_decoder(video_codec_params->codec_id);
			video_codec_ctx = avcodec_alloc_context3(video_codec);

			return MP2A_SUCCESS;
		}
	}

	return MP2A_FAILURE;
}


enum mp2a_result_t decode_video_packet() {
	EXIT_IF_TRUE(
		video_codec_ctx == NULL,
		"error(video): AVCodecContext is NULL\n"
	);
	EXIT_IF_TRUE(
		packet == NULL,
		"error(video): AVPacket is NULL\n"
	);
	EXIT_IF_TRUE(
		frame == NULL,
		"error(video): AVFrame is NULL\n"
	);

	EXIT_IF_TRUE(
		avcodec_send_packet(video_codec_ctx, packet),
		"error: could not send packet to video codec\n"
	);

	while (true) {
		int result = avcodec_receive_frame(video_codec_ctx, frame);

		if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
			break;
		}

		EXIT_IF_TRUE(result < 0, "error: could not receive video frame from codec\n");

		display_frame();
		usleep(1000000 / 30);
	}

	return MP2A_SUCCESS;
}
