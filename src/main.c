#include "screen.h"
#include "video.h"
#include "main.h"


AVFormatContext *format_ctx;
AVPacket *packet;
AVFrame *frame;


int main() {
	EXIT_IF_FAILURE(get_window_size(), "error: failed to get window size\n");
	EXIT_IF_ALLOC_NULL(format_ctx, avformat_alloc_context());

	EXIT_IF_FAILURE(
		avformat_open_input(&format_ctx, FILE_INPUT, NULL, NULL),
		"error: couldn't open file %s.\n",
		FILE_INPUT
	);

	EXIT_IF_TRUE(
		avformat_find_stream_info(format_ctx, NULL) < 0,
		"error: couldn't find stream info in file %s.\n",
		FILE_INPUT
	);

	EXIT_IF_FAILURE(
		find_video_stream(),
		"error: file %s doesn't contain a video stream.\n",
		FILE_INPUT
	);

	EXIT_IF_TRUE(
		avcodec_parameters_to_context(video_codec_ctx, video_codec_params) < 0,
		"error: failed parameters to context.\n"
	);

	EXIT_IF_TRUE(
		avcodec_open2(video_codec_ctx, video_codec, NULL) < 0,
		"error: failed to initialize AVCodecContext.\n"
	);

	printf(CLEAR_TERMINAL);
	printf(MOVE_CURSOR(0, 0));

	EXIT_IF_ALLOC_NULL(frame, av_frame_alloc());
	EXIT_IF_ALLOC_NULL(packet, av_packet_alloc());

	while (av_read_frame(format_ctx, packet) >= 0) {
		if (packet->stream_index == video_stream_index) {
			EXIT_IF_FAILURE(decode_video_packet(), "error: failed to decode video.\n");
		}

		av_packet_unref(packet);
	}

	return MP2A_SUCCESS;
}
