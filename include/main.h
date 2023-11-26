#ifndef MAIN_H
#define MAIN_H


#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <sys/ioctl.h>
#include <stdbool.h>


#define FILE_INPUT "bad_apple.mp4"

#define EXIT_IF_FAILURE(STATEMENT, MESSAGE, ...) do {\
		if (STATEMENT != MP2A_SUCCESS) {\
			fprintf(stderr, #MESSAGE, ##__VA_ARGS__);\
			exit(MP2A_FAILURE);\
		}\
	} while (false)

#define EXIT_IF_TRUE(EXPRESSION, MESSAGE, ...) do {\
		if (EXPRESSION) {\
			fprintf(stderr, #MESSAGE, ##__VA_ARGS__);\
			exit(MP2A_FAILURE);\
		}\
	} while (false)

#define EXIT_IF_ALLOC_NULL(IDENTIFIER, ALLOC_STATEMENT) do {\
		IDENTIFIER = ALLOC_STATEMENT;\
		if (IDENTIFIER == NULL) {\
			fprintf(stderr, "error: couldn't alloc memory for " #IDENTIFIER ".\n");\
			exit(MP2A_FAILURE);\
		}\
	} while (false)


enum mp2a_result_t {
	MP2A_SUCCESS,
	MP2A_FAILURE,
};


extern AVFormatContext *format_ctx;
extern AVPacket *packet;
extern AVFrame *frame;


#endif // !MAIN_H
