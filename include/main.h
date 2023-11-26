#ifndef MAIN_H
#define MAIN_H


#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <sys/ioctl.h>
#include <stdbool.h>


#define FILE_INPUT "bad_apple.mp4"

#define GOTO_IF_TRUE(LABEL, EXPRESSION, MESSAGE, ...) do {\
		if (EXPRESSION) {\
			fprintf(stderr, #MESSAGE, ##__VA_ARGS__);\
			goto LABEL;\
		}\
	} while (false)

#define GOTO_IF_FAILURE(LABEL, STATEMENT, MESSAGE, ...) \
	GOTO_IF_TRUE(LABEL, STATEMENT != MP2A_SUCCESS, #MESSAGE, ##__VA_ARGS__);

#define GOTO_IF_ALLOC_NULL(LABEL, IDENTIFIER, ALLOC_STATEMENT) do {\
		IDENTIFIER = ALLOC_STATEMENT;\
		GOTO_IF_TRUE(\
			LABEL,\
			IDENTIFIER == NULL,\
			"error: couldn't alloc memory for " #IDENTIFIER ".\n"\
		);\
	} while (false)

#define FAIL_IF_TRUE(EXPRESSION, MESSAGE, ...) do {\
		if (EXPRESSION) {\
			fprintf(stderr, #MESSAGE, ##__VA_ARGS__);\
			return MP2A_FAILURE;\
		}\
	} while (false)

#define FAIL_IF_FAILURE(STATEMENT, MESSAGE, ...) FAIL_IF_TRUE(STATEMENT != MP2A_SUCCESS, #MESSAGE, ##__VA_ARGS__);
#define FAIL_IF_NULL(IDENTIFIER) FAIL_IF_TRUE(IDENTIFIER == NULL, "error: " #IDENTIFIER " is NULL.\n");


enum mp2a_result_t {
	MP2A_SUCCESS,
	MP2A_FAILURE,
};


extern AVFormatContext *format_ctx;
extern AVPacket *packet;
extern AVFrame *frame;


#endif // !MAIN_H
