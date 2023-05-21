#include "common.h"

struct termios w_term, w_restore;
struct winsize w_size;

void w_resize() {
	ioctl(0, TIOCGWINSZ, &w_size);
	wy -= 2;
	onresize();
}

void w_setup() {

	tcgetattr(0, &w_term);
	tcgetattr(0, &w_restore); // backup the original terminal state to restore later

	printf("\x1b[?1049h\x1b[?1003h\x1b[?25l\x1b[?1000h");
	fflush(stdout);

	ioctl(0, TIOCGWINSZ, &w_size);
	signal(SIGWINCH, w_resize);
	wy -= 2;
	
	w_term.c_lflag = 0;
	w_term.c_lflag &= ~ECHO;
	w_term.c_lflag &= ~ICANON;
	w_term.c_lflag &= ~ISIG;

	// Set the new interrupt character for Ctrl-C, Ctrl-S and Ctrl-Q
	w_term.c_cc[VINTR] = 3;
	w_term.c_cc[VSTOP] = 19;
	w_term.c_cc[VSTART] = 17;

	tcsetattr(0, TCSANOW, &w_term);

}

void w_reset() {
	printf("\x1b[?1049l\x1b[?25h\x1b[?1000l\x1b[?1003l\x1b[0m");
	fflush(stdout);
	tcsetattr(0, TCSANOW, &w_restore); // restore terminal state
}
