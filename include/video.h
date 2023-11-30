#ifndef VIDEO_H
#define VIDEO_H

#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>


enum mp2a_result_t find_video_stream();
enum mp2a_result_t decode_video_packet();


extern int video_stream_index;

extern AVCodecParameters *video_codec_params;
extern AVCodecContext *video_codec_ctx;
extern AVCodec *video_codec;

extern struct SwsContext *video_sws_ctx;
extern AVFrame *rgb_frame;


#endif // !VIDEO_H
