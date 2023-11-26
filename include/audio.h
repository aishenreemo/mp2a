#ifndef AUDIO_H
#define AUDIO_H


#include <libavcodec/avcodec.h>
#include <SDL2/SDL.h>


#define SDL_AUDIO_BUFFER_SIZE 1024


enum mp2a_result_t find_audio_stream();
enum mp2a_result_t decode_audio_packet();
enum mp2a_result_t init_audio_spec();


extern int audio_stream_index;

extern AVCodecParameters *audio_codec_params;
extern AVCodecContext *audio_codec_ctx;
extern AVCodec *audio_codec;

extern SDL_AudioDeviceID audio_device;
extern SDL_AudioSpec wanted_spec;


#endif // !AUDIO_H
