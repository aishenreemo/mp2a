#include "screen.h"
#include "video.h"
#include <libavcodec/avcodec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include "main.h"


AVFormatContext *format_ctx;
AVPacket *packet;
AVFrame *frame;


int main() {
	GOTO_IF_TRUE(
		dealloc_l,
		get_window_size(),
		"error: failed to get window size\n"
	);

	GOTO_IF_ALLOC_NULL(dealloc_l, format_ctx, avformat_alloc_context());

	GOTO_IF_FAILURE(
		dealloc_l,
		avformat_open_input(&format_ctx, FILE_INPUT, NULL, NULL),
		"error: couldn't open file %s.\n",
		FILE_INPUT
	);

	GOTO_IF_TRUE(
		dealloc_l,
		avformat_find_stream_info(format_ctx, NULL) < 0,
		"error: couldn't find stream info in file %s.\n",
		FILE_INPUT
	);

	GOTO_IF_FAILURE(
		dealloc_l,
		find_video_stream(),
		"error: file %s doesn't contain a video stream.\n",
		FILE_INPUT
	);

	GOTO_IF_TRUE(
		dealloc_l,
		avcodec_parameters_to_context(video_codec_ctx, video_codec_params) < 0,
		"error: failed parameters to context.\n"
	);

	GOTO_IF_TRUE(
		dealloc_l,
		avcodec_open2(video_codec_ctx, video_codec, NULL) < 0,
		"error: failed to initialize AVCodecContext.\n"
	);

	printf(CLEAR_TERMINAL);
	printf(MOVE_CURSOR(0, 0));

	GOTO_IF_ALLOC_NULL(dealloc_l, frame, av_frame_alloc());
	GOTO_IF_ALLOC_NULL(dealloc_l, packet, av_packet_alloc());

	while (av_read_frame(format_ctx, packet) >= 0) {
		if (packet->stream_index == video_stream_index) {
			GOTO_IF_FAILURE(dealloc_l, decode_video_packet(), "error: failed to decode video.\n");
		}

		av_packet_unref(packet);
	}

	dealloc_l: {
		av_frame_free(&frame);
		av_packet_free(&packet);
		avcodec_parameters_free(&video_codec_params);
		avcodec_close(video_codec_ctx);
		avcodec_free_context(&video_codec_ctx);
		avformat_close_input(&format_ctx);
		avformat_free_context(format_ctx);
	}

	return MP2A_SUCCESS;
}
