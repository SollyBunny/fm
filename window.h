
#ifndef WINDOW_H
#define WINDOW_H
#include "include.h"

struct termios w_term, w_restore;

void w_resize() {
	ioctl(0, TIOCGWINSZ, &ws);
}

inline void w_setup() {

	ioctl(0, TIOCGWINSZ, &ws);
	signal(SIGWINCH, w_resize);
	
	tcgetattr(0, &w_term);
	tcgetattr(0, &w_restore); // backup the original terminal state to restore later
	w_term.c_lflag &= ~(ICANON|ECHO|~ISIG);
	w_term.c_cflag &= ~(PARENB);
	w_term.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF);
	w_term.c_cflag = (w_term.c_cflag & ~CSIZE) | CS8;
	w_term.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &w_term);

	printf("\x1b[?1003h\x1b[?25l");
	
}

inline void w_reset() {
	tcsetattr(0, TCSANOW, &w_restore); // restore terminal state
	printf("\x1b[?9l\x1b[?1003l\x1b[?25h"); // disable mouse echo
}

void w_display(Folder *folder) {
	printf("\x1b[2J\x1b[H%s\n", folder->path);
	for (unsigned int i = folder->off; i < folder->numfiles && i + folder->off < wy; ++i) {
		if (i == folder->cur)
			printf("\x1b[7m");
		switch (folder->files[i].type) {
			// see https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
			case DT_REG: // regular file
				printf("📃");
				break;
			case DT_DIR: // directory
				printf("📁");
				break;
			case DT_FIFO: // fifo pipe
				printf("➡️");
				break;
			case DT_SOCK: // network socket
				printf("🌐");
				break;
			case DT_CHR: // character devices (octet stream)
			case DT_BLK: // block device (data stream)
				printf("💾");
				break;
			case DT_LNK:
				printf("📎");
				break;
			default:
				putchar('?');
				break;
		
		}
		printf(" %s\n", folder->files[i].name);
		if (i == folder->cur)
			printf("\x1b[27m");
	}
}

#endif
