#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/file.h>


#define FILE_INPUT "bad_apple.mp4"

#define NO_VIDEO_STREAM -1

#define SUCCESS EXIT_SUCCESS
#define FAILURE EXIT_FAILURE

#define CLEAR_TERMINAL "\033[2J"
#define MOVE_CURSOR(X, Y) "\033[%d;%dH", X, Y

#define SET_COLOR(R, G, B) "\033[48;2;%d;%d;%dm", R, G, B
#define RESET_COLOR "\033[0m"

#define ASCII_CHARS "@%#*+=-:. "


int get_terminal_size();
void get_streams();

int decode_video_packet();

char get_ascii_char(float intensity);
void display_frame();


static struct winsize terminal_size;

static AVFormatContext *format_ctx;
static AVStream *video_stream;

static int video_stream_index = NO_VIDEO_STREAM;

static AVCodecParameters *video_codec_params;
static AVCodecContext *video_codec_ctx;
static AVCodec *video_codec;

static AVPacket *packet;
static AVFrame *frame;


int main(void) {
	if (get_terminal_size() == FAILURE) {
		return FAILURE;
	}

	format_ctx = avformat_alloc_context();

	if (format_ctx == NULL) {
		printf("error: could not allocate memory for AVFormatContext.");
		return FAILURE;
	}

	if (avformat_open_input(&format_ctx, FILE_INPUT, NULL, NULL) != SUCCESS) {
		printf("error: could not open the file.");
		return FAILURE;
	}


	if (avformat_find_stream_info(format_ctx, NULL) < 0) {
		fprintf(stderr, "Error finding stream information\n");
		return FAILURE;
	}


	get_streams();

	if (video_stream_index == NO_VIDEO_STREAM) {
		printf("error: file " FILE_INPUT " does not contain a video stream.");
		return FAILURE;
	}


	avcodec_parameters_to_context(video_codec_ctx, video_codec_params);
	avcodec_open2(video_codec_ctx, video_codec, NULL);

	if (video_codec_ctx == NULL) {
		printf("error: could not allocate memory for AVCodecContext.");
		return FAILURE;
	}


	printf(CLEAR_TERMINAL);
	printf(MOVE_CURSOR(0, 0));

	frame = av_frame_alloc();
	packet = av_packet_alloc();

	while (av_read_frame(format_ctx, packet) >= 0) {
		int result = SUCCESS;

		if (packet->stream_index == video_stream_index) {
			result = decode_video_packet();
		}

		if (result != SUCCESS) {
			return FAILURE;
		}

		av_packet_unref(packet);
	}


	av_frame_free(&frame);
	av_packet_free(&packet);

	avcodec_parameters_free(&video_codec_params);
	avcodec_free_context(&video_codec_ctx);

	avformat_close_input(&format_ctx);
	avformat_free_context(format_ctx);


	return SUCCESS;
}

int get_terminal_size() {
	if (ioctl(0, TIOCGWINSZ, &terminal_size) != -1) {
		return SUCCESS;
	} else {
		fprintf(stderr, "Error getting terminal size\n");
		return FAILURE;
	}
}

void get_streams() {
	for (int i = 0; i < format_ctx->nb_streams; i++) {
		AVStream *stream = format_ctx->streams[i];

		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream = stream;
			video_stream_index = i;

			video_codec_params = video_stream->codecpar;
			video_codec = (AVCodec *) avcodec_find_decoder(video_codec_params->codec_id);
			video_codec_ctx = avcodec_alloc_context3(video_codec);
		}
	}
}


int decode_video_packet() {
	int result = avcodec_send_packet(video_codec_ctx, packet);

	if (result < 0) {
		printf("error: could not send packet to video codec\n");
		return result;
	}

	while (result >= 0) {
		result = avcodec_receive_frame(video_codec_ctx, frame);

		if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
			break;
		} else if (result < 0) {
			printf("error: could not receive video frame from codec\n");
			return FAILURE;
		}

		display_frame();
		usleep(1000000 / 30);
	}

	return SUCCESS;
}


char get_ascii_char(float intensity) {
	char ascii_chars[] = "@%#*+=-:. ";
	int num_chars = sizeof(ascii_chars) - 1;

	int index = (int)(intensity * num_chars);
	index = (index < 0) ? 0 : (index >= num_chars) ? num_chars - 1 : index;

	return ascii_chars[index];
}


void display_frame() {
	printf(MOVE_CURSOR(0, 0));

	int step_x = video_codec_params->width / terminal_size.ws_col;
	int step_y = video_codec_params->height / terminal_size.ws_row;

	for (int y = 0; y < terminal_size.ws_row; y++) {
		for (int x = 0; x < terminal_size.ws_col; x++) {
			int video_x = x * step_x;
			int video_y = y * step_y;
			int index = video_y * frame->linesize[0] + video_x;
			uint8_t r = frame->data[0][index];
			uint8_t g = frame->data[0][index + 1];
			uint8_t b = frame->data[0][index + 2];

			float average = (r + g + b) / 3.0;
			float intensity = average / 255.0;

			char ascii_char = get_ascii_char(intensity);

			putchar(ascii_char);
		}

		putchar('\n');
	}

	printf(RESET_COLOR);
}
