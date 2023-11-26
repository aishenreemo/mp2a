#ifndef VIDEO_H
#define VIDEO_H


#include <libavcodec/avcodec.h>


enum mp2a_result_t find_video_stream();
enum mp2a_result_t decode_video_packet();


extern int video_stream_index;

extern AVCodecParameters *video_codec_params;
extern AVCodecContext *video_codec_ctx;
extern AVCodec *video_codec;


#endif // !VIDEO_H
