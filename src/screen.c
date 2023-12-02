#include <stdint.h>
#include <stdio.h>

#include "screen.h"
#include "video.h"
#include "main.h"


struct winsize window_size;


enum mp2a_result_t get_window_size() {
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &window_size) == -1) {
		window_size.ws_col = 0;
		window_size.ws_row = 0;

		return MP2A_FAILURE;
	} else {
		window_size.ws_col -= 1;
		window_size.ws_row -= 1;
		return MP2A_SUCCESS;
	}
};


enum mp2a_result_t display_frame() {
	FAIL_IF_NULL(video_codec_params);

	sws_scale(
		video_sws_ctx,
		(const uint8_t *const *) frame->data,
		frame->linesize,
		0,
		video_codec_ctx->height,
		rgb_frame->data,
		rgb_frame->linesize
	);

	int step_x = video_codec_params->width / window_size.ws_col;
	int step_y = video_codec_params->height / window_size.ws_row;

	// color[0..3] will hold the current color and
	// color[3..6] will hold the previous color
	uint8_t color[6];
	memset(color, 0, 6);

	printf(MOVE_CURSOR(0, 0));
	for (int i = 0, j = 0; i < window_size.ws_row * window_size.ws_col; i++, j++) {
		int x = i % window_size.ws_col;
		int y = i / window_size.ws_col;

		int video_x = x * step_x;
		int video_y = y * step_y;

		int index = video_y * rgb_frame->linesize[0] + video_x * 3;

		color[0] = rgb_frame->data[0][index];
		color[1] = rgb_frame->data[0][index + 1];
		color[2] = rgb_frame->data[0][index + 2];

		float average = (color[0] + color[1] + color[2]) / 3.0;
		float intensity = average / 255.0;

		if (options.is_invert) {
			color[0] = UINT8_MAX - color[0];
			color[1] = UINT8_MAX - color[1];
			color[2] = UINT8_MAX - color[2];
		}

		bool same_color = color[0] == color[3] &&
			color[1] == color[4] &&
			color[2] == color[5];

		if (options.is_full_color && !same_color) {
			printf(SET_BG_COLOR(color[0], color[1], color[2]));
			printf(SET_COLOR(color[0], color[1], color[2]));
		} else if (options.is_color && !same_color) {
			printf(SET_COLOR(color[0], color[1], color[2]));
		}

		char ascii_char = get_char_by_intensity(intensity);
		putchar(ascii_char);

		if (x == window_size.ws_col - 1) {
			printf("\n");
		}

		color[3] = color[0];
		color[4] = color[1];
		color[5] = color[2];
	}

	printf(RESET_COLOR);

	return MP2A_SUCCESS;
}


char get_char_by_intensity(float intensity) {
	int num_chars = sizeof(ASCII_CHARS) - 1;

	int index = (int) (intensity * num_chars);

	if (index < 0) {
		index = 0;
	} else if (index >= num_chars) {
		index = num_chars - 1;
	}

	if (options.is_invert) {
		index += 1;
		index = num_chars - index;
	}

	return ASCII_CHARS[index];
}
