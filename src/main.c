#include "screen.h"
#include "audio.h"
#include "video.h"
#include "main.h"


AVFormatContext *format_ctx;
AVPacket *packet;
AVFrame *frame;

SDL_Event event;
bool quit;


int main(int argc, char *argv[]) {
	GOTO_IF_TRUE(
		dealloc_l,
		argc < 2,
		"usage: %s <input_file>\n",
		argv[0]
	);

	GOTO_IF_TRUE(
		dealloc_l,
		get_window_size(),
		"error: failed to get window size\n"
	);

	GOTO_IF_ALLOC_NULL(dealloc_l, format_ctx, avformat_alloc_context());

	GOTO_IF_FAILURE(
		dealloc_l,
		avformat_open_input(&format_ctx, argv[1], NULL, NULL),
		"error: couldn't open file %s.\n",
		argv[1]
	);
	GOTO_IF_TRUE(
		dealloc_l,
		avformat_find_stream_info(format_ctx, NULL) < 0,
		"error: couldn't find stream info in file %s.\n",
		argv[1]
	);

	GOTO_IF_FAILURE(
		dealloc_l,
		find_video_stream(),
		"error: file %s doesn't contain a video stream.\n",
		argv[1]
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

	GOTO_IF_FAILURE(
		dealloc_l,
		find_audio_stream(),
		"error: file %s doesn't contain a video stream.\n",
		argv[1]
	);
	GOTO_IF_TRUE(
		dealloc_l,
		avcodec_parameters_to_context(audio_codec_ctx, audio_codec_params) < 0,
		"error: failed parameters to context.\n"
	);
	GOTO_IF_TRUE(
		dealloc_l,
		avcodec_open2(audio_codec_ctx, audio_codec, NULL) < 0,
		"error: failed to initialize AVCodecContext.\n"
	);

	GOTO_IF_TRUE(
		dealloc_l,
		SDL_Init(SDL_INIT_AUDIO) < 0,
		"error: could not initialize SDL audio subsystem.\n"
	);

	GOTO_IF_FAILURE(dealloc_l, init_audio_spec(), "error: failed to initialize audio spec\n");

	audio_device = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &obtained_spec, 0);
	GOTO_IF_TRUE(dealloc_l, audio_device == 0, "error: couldn't open audio device.\n");

	SDL_PauseAudioDevice(audio_device, 0);

	printf(CLEAR_TERMINAL);
	printf(MOVE_CURSOR(0, 0));

	GOTO_IF_ALLOC_NULL(dealloc_l, frame, av_frame_alloc());
	GOTO_IF_ALLOC_NULL(dealloc_l, packet, av_packet_alloc());
	GOTO_IF_ALLOC_NULL(dealloc_l, audio_swr_ctx, swr_alloc());

	av_opt_set_int(audio_swr_ctx, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(audio_swr_ctx, "in_sample_rate", audio_codec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(audio_swr_ctx, "in_sample_fmt", audio_codec_ctx->sample_fmt, 0);

	av_opt_set_int(audio_swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(audio_swr_ctx, "out_sample_rate", audio_codec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(audio_swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	swr_init(audio_swr_ctx);


	while (av_read_frame(format_ctx, packet) >= 0 || !quit) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
		}

		if (packet->stream_index == video_stream_index) {
			GOTO_IF_FAILURE(
				dealloc_l,
				decode_video_packet(),
				"error: failed to decode video.\n"
			);
		} else if (packet->stream_index == audio_stream_index) {
			GOTO_IF_FAILURE(
				dealloc_l,
				decode_audio_packet(),
				"error: failed to decode audio.\n"
			);
		} else {
			av_packet_unref(packet);
		}

		GOTO_IF_TRUE(dealloc_l, quit, "exiting...\n");
	}

	dealloc_l: {
		swr_free(&audio_swr_ctx);
		av_frame_free(&frame);
		av_packet_free(&packet);
		avcodec_parameters_free(&audio_codec_params);
		avcodec_close(audio_codec_ctx);
		avcodec_free_context(&audio_codec_ctx);
		avcodec_parameters_free(&video_codec_params);
		avcodec_close(video_codec_ctx);
		avcodec_free_context(&video_codec_ctx);
		avformat_close_input(&format_ctx);
		avformat_free_context(format_ctx);
		SDL_CloseAudioDevice(audio_device);
		SDL_Quit();
	}

	return MP2A_SUCCESS;
}
