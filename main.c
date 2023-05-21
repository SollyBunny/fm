#define MAIN_C

#include "common.h"

void __attribute__((noreturn)) die(char *msg) {
	printf("\x1b[31;1m: %s", msg);
	if (errno) {
		putchar(':'); putchar(' ');
		printf("%s", strerror(errno));
	}
	printf("\x1b[0m\n");
	exit(errno || 1);
}

Folder *folder;

void onresize() {
	f_scroll(folder);
	f_display(folder);
}

int main(int argc, char *argv[]) {

	folder = (Folder *)malloc(sizeof(Folder));

	if (argc == 1) // no args use working directory
		folder->path = getcwd(NULL, 0);
	else
		folder->path = realpath(argv[1], NULL);
	if (errno) die("Codunt get path");

	f_fifo(folder);
	f_readdir(folder, NULL);

	w_setup();
	f_display(folder);

	uint32_t arg;
	char input[10];
	unsigned int inputsize;
	while (1) {
		// Get user input
		inputsize = read(STDIN_FILENO, input, sizeof(input));
		input[inputsize] = '\0';
		arg = tick(input, inputsize);
		switch (arg & 0x000000FF) {
		case QUIT:
			goto l_end;
		case GOTO:
			arg >>= 8;
			arg += folder->off;
			if (folder->cur == arg) {
				f_readdir(folder, folder->files[folder->cur].name);
				f_display(folder);
			} else {
				f_scrollto(folder, arg);
			}
			break;
		case PATH:
			arg >>= 8;
			if (arg == 0) {
				f_readdir(folder, "/");
				f_display(folder);
				break;
			}
			static int count;
			static char *p;
			count = -2;
			p = folder->path;
			while (*p != '\0') {
				if (p - folder->path > (unsigned int)(long)arg) {
					count = -1;
					break;
				}
				++p;
			}
			if (count == -2) break; // Out of bounds
			while (*p != '\0') {
				if (*p == '/') ++count;
				++p;
			}
			if (count == -1) break; // Out of bounds
			for (; count > 0; --count) chdir("..");
			f_readdir(folder, "..");
			f_display(folder);
			break;
		case FORW:
			if (folder->files[folder->cur].type == DT_DIR) {
				f_readdir(folder, folder->files[folder->cur].name);
				f_display(folder);
			}
			break;
		case BACK:
			if (folder->path[1] != '\0') { // "/"
				static char filename[MAX_NAME];
				static char *p1, *p2;
				p1 = folder->path;
				p2 = NULL;
				while (*p1 != '\0') { 
					if (*p1 == '/') p2 = p1 + 1;
					++p1;
				}
				if (p2) strcpy(filename, p2);
				f_readdir(folder, "..");
				if (p2) f_select(folder, p2);
			}
			f_display(folder); // doesn't get called
			break;
		case UP:
			f_scrollby(folder, -1);
			break;
		case DN:
			f_scrollby(folder, 1);
			break;
		case PGUP:
			f_scrollby(folder, -wy);
			break;
		case PGDN:
			f_scrollby(folder, wy);
			break;
		case HOME:
			f_scrollto(folder, 0);
			break;
		case END:
			f_scrollto(folder, folder->numfiles - 1);
			break;
		case TGLSEL:
			break;
		case RENAME:
			break;
		case DEL:
			break;
		}
	}

	l_end:

		f_close(folder);
		w_reset();
		
	return 0;
}