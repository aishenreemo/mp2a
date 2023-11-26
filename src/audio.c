#include <SDL2/SDL_audio.h>
#include <libswresample/swresample.h>
#include <unistd.h>

#include "screen.h"
#include "audio.h"
#include "main.h"


int audio_stream_index;

AVCodecParameters *audio_codec_params;
AVCodecContext *audio_codec_ctx;
AVCodec *audio_codec;

SDL_AudioDeviceID audio_device;
SDL_AudioSpec wanted_spec;


enum mp2a_result_t find_audio_stream() {
	FAIL_IF_TRUE(
		format_ctx == NULL,
		"error(audio): AVFormatContext is NULL\n"
	);

	for (int i = 0; i < format_ctx->nb_streams; i++) {
		AVStream *stream = format_ctx->streams[i];

		if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_stream_index = i;

			audio_codec_params = stream->codecpar;
			audio_codec = (AVCodec *) avcodec_find_decoder(audio_codec_params->codec_id);
			FAIL_IF_NULL(audio_codec);
			audio_codec_ctx = avcodec_alloc_context3(audio_codec);
			FAIL_IF_NULL(audio_codec_ctx);


			return MP2A_SUCCESS;
		}
	}

	return MP2A_FAILURE;
}


enum mp2a_result_t decode_audio_packet() {
	FAIL_IF_TRUE(
		avcodec_send_packet(audio_codec_ctx, packet) < 0,
		"error: couldn't send packet to the decoder\n"
	);

	while (true) {
		int result = avcodec_receive_frame(audio_codec_ctx, frame);

		if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
			break;
		}

		FAIL_IF_TRUE(result < 0, "error: couldn't decode audio.\n");

		for (int ch = 0; ch < audio_codec_ctx->ch_layout.nb_channels; ch++) {
			SDL_QueueAudio(audio_device, frame->data[ch], frame->linesize[ch]);
		}
	}

	return MP2A_SUCCESS;
}


enum mp2a_result_t init_audio_spec() {
	FAIL_IF_NULL(audio_codec_ctx);

	wanted_spec.freq = audio_codec_ctx->sample_rate;
	wanted_spec.channels = audio_codec_ctx->ch_layout.nb_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
	wanted_spec.callback = NULL;
	wanted_spec.userdata = audio_codec_ctx;
	wanted_spec.format = AUDIO_F32SYS;

	return MP2A_SUCCESS;
}

