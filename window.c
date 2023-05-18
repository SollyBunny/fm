#include "common.h"
#include <stdio.h>

struct termios w_term, w_restore;
struct winsize w_size;

void w_resize() {
	ioctl(0, TIOCGWINSZ, &w_size);
	onresize();
}

void w_setup() {

	printf("\x1b[?1049h\x1b[?1003h\x1b[?25l\x1b[?1000h");

	ioctl(0, TIOCGWINSZ, &w_size);
	signal(SIGWINCH, w_resize);
	
	tcgetattr(0, &w_term);
	tcgetattr(0, &w_restore); // backup the original terminal state to restore later
	w_term.c_lflag &= ~(ICANON|ECHO|~ISIG);
	w_term.c_cflag &= ~(PARENB);
	w_term.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF);
	w_term.c_cflag = (w_term.c_cflag & ~CSIZE) | CS8;
	w_term.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &w_term);

}

void w_reset() {
	tcsetattr(0, TCSANOW, &w_restore); // restore terminal state
	printf("\x1b[?1000l\x1b[?1003l\x1b[?25h\x1b[?1049l"); // disable mouse echo
}

void w_display(Folder *folder) {
	printf("\x1b[2J\x1b[H%s %d/%d", folder->path, folder->cur + 1, folder->numfiles);
	unsigned int end;
	if (folder->numfiles < wy + folder->off - 1) end = folder->numfiles;
	else                                         end = wy + folder->off - 1;
	for (unsigned int i = folder->off; i < end; ++i) {
		putchar('\n');
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
		printf(" %s", folder->files[i].name);
		if (i == folder->cur)
			printf("\x1b[27m");
	}
	fflush(stdout);
}
