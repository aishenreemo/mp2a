#include <stdio.h>
#include <unistd.h>

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

	printf(MOVE_CURSOR(0, 0));

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

	for (int y = 0; y < window_size.ws_row; y++) {
		for (int x = 0; x < window_size.ws_col; x++) {
			int video_x = x * step_x;
			int video_y = y * step_y;
			int index = video_y * rgb_frame->linesize[0] + video_x * 3;
			uint8_t r = rgb_frame->data[0][index];
			uint8_t g = rgb_frame->data[0][index + 1];
			uint8_t b = rgb_frame->data[0][index + 2];

			float average = (r + g + b) / 3.0;
			float intensity = average / 255.0;

			char ascii_char = get_char_by_intensity(intensity);

			if (options.is_color) {
				printf(SET_COLOR(r, g, b));
			}

			putchar(ascii_char);
		}

		putchar('\n');
	}

	printf(RESET_COLOR);

	return MP2A_SUCCESS;
}


char get_char_by_intensity(float intensity) {
	int num_chars = sizeof(ASCII_CHARS) - 1;

	int index = (int)(intensity * num_chars);

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
