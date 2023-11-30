#include "screen.h"
#include "audio.h"
#include "video.h"
#include <libswscale/swscale.h>
#include "main.h"


struct mp2a_options_t options;

AVFormatContext *format_ctx;
AVPacket *packet;
AVFrame *frame;

SDL_Event event;
bool quit;


static void help() {
	printf(
		"MPEG-4 to ASCII video player\n"
		"usage: \e[1;31mmp2a\e[0m [FILE] [--help] [--version]\n\n"
		"\e[1;37m\t-v\t--version\e[0m:\tPrint version and exit.\n"
		"\e[1;37m\t-h\t   --help\e[0m:\tShow this message.\n\n"
		"\e[1;37m\t  \t --colors\e[0m:\tEnable ANSI colors.\n"
		"\e[1;37m\t  \t --invert\e[0m:\tInvert output.\n\n"
		"Homepage: <https://github.com/aishenreemo/mp2a>\n"
		"(C) 2023 Aishen Reemo <aish3n@pm.me>\n\n"
	);
}


int main(int argc, char *args[]) {
	GOTO_IF_TRUE(
		dealloc_l,
		get_window_size(),
		"error: failed to get window size\n"
	);

	GOTO_IF_ALLOC_NULL(dealloc_l, format_ctx, avformat_alloc_context());

	for (int i = 1; i < argc; i++) {
		if (EQ(args[i], "--help") || EQ(args[i], "-h")) {
			help();
			goto dealloc_l;
		} else if (EQ(args[i], "--version") || EQ(args[i], "-v")) {
			printf(VERSION "\n");
			goto dealloc_l;
		} else if (EQ(args[i], "--colors")) {
			options.is_color = true;
			continue;
		} else if (EQ(args[i], "--invert")) {
			options.is_invert = true;
			continue;
		}

		options.input_file = args[i];
		GOTO_IF_FAILURE(
			dealloc_l,
			avformat_open_input(&format_ctx, options.input_file, NULL, NULL),
			"error: couldn't open file %s.\n",
			options.input_file
		);
	}

	GOTO_IF_TRUE(
		dealloc_l,
		options.input_file == NULL,
		"error: no file specified.\n"
	);

	GOTO_IF_TRUE(
		dealloc_l,
		avformat_find_stream_info(format_ctx, NULL) < 0,
		"error: couldn't find stream info in file %s.\n",
		options.input_file
	);

	GOTO_IF_FAILURE(
		dealloc_l,
		find_video_stream(),
		"error: file %s doesn't contain a video stream.\n",
		options.input_file
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
		options.input_file
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

	GOTO_IF_ALLOC_NULL(dealloc_l, rgb_frame, av_frame_alloc());
	GOTO_IF_ALLOC_NULL(dealloc_l, video_sws_ctx, sws_getContext(
		    video_codec_ctx->width, video_codec_ctx->height, video_codec_ctx->pix_fmt,
		    video_codec_ctx->width, video_codec_ctx->height, AV_PIX_FMT_RGB24,
		    SWS_BILINEAR, NULL, NULL, NULL
	));

	av_opt_set_int(audio_swr_ctx, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(audio_swr_ctx, "in_sample_rate", audio_codec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(audio_swr_ctx, "in_sample_fmt", audio_codec_ctx->sample_fmt, 0);

	av_opt_set_int(audio_swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(audio_swr_ctx, "out_sample_rate", audio_codec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(audio_swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	swr_init(audio_swr_ctx);

	rgb_frame->format = AV_PIX_FMT_RGB24;
	rgb_frame->width = video_codec_ctx->width;
	rgb_frame->height = video_codec_ctx->height;
	GOTO_IF_TRUE(
		dealloc_l,
		av_frame_get_buffer(rgb_frame, 32) < 0,
		"error: could not allocate buffer for rgb frame\n"
	);

	float time_base = av_q2d(format_ctx->streams[video_stream_index]->time_base);
	int64_t video_pts = AV_NOPTS_VALUE;

	fprintf(stderr, "Pixel Format: %d\n", video_codec_ctx->pix_fmt);
	while (av_read_frame(format_ctx, packet) >= 0) {
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

			video_pts = packet->pts;
		} else if (packet->stream_index == audio_stream_index) {
			GOTO_IF_FAILURE(
				dealloc_l,
				decode_audio_packet(),
				"error: failed to decode audio.\n"
			);
		} else {
			av_packet_unref(packet);
		}

		int64_t video_duration = video_pts * time_base * 1000;
		int64_t diff = video_duration - SDL_GetTicks();

		if (diff > 0) {
			SDL_Delay(diff);
		}

		GOTO_IF_TRUE(dealloc_l, quit, "exiting...\n");
	}

	dealloc_l: {
		sws_freeContext(video_sws_ctx);
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
