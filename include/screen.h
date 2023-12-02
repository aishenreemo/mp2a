#ifndef SCREEN_H
#define SCREEN_H


#include <sys/ioctl.h>
#include <unistd.h>


#define CLEAR_TERMINAL "\033[2J"
#define MOVE_CURSOR(X, Y) "\033[%d;%dH", X, Y
#define SHOW_CURSOR "\e[?25h"
#define HIDE_CURSOR "\e[?25l"

#define SET_BG_COLOR(R, G, B) "\033[48;2;%d;%d;%dm", R, G, B
#define SET_COLOR(R, G, B) "\033[38;2;%d;%d;%dm", R, G, B
#define RESET_COLOR "\033[0m"

#define ASCII_CHARS "   ...',;:clodxkO0KXNWM"


enum mp2a_result_t get_window_size();
enum mp2a_result_t display_frame();

char get_char_by_intensity(float intensity);


extern struct winsize window_size;


#endif // !SCREEN_H
