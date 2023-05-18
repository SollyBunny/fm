
#include "include.h"

void action(enum Action action) {
	switch (action) {
		
	}
}

int main(int argc, char *argv[]) {

	Folder folder = { NULL, NULL, 0, 0, 0, 0 };
	if (argc == 1) { // no args use working directory
		folder.path = getcwd(folder.path, 255);
	} else {
		folder.path = realpath(argv[1], folder.path);
	}
	if (folder.path == NULL) {
		die("Coudn't open directory");
	}
	printf("Opened: %s\n", folder.path);

	f_readdir(&folder);

	w_setup();
	w_display(&folder);

	char c, lc, llc;
	char ngetchar() {
		llc = lc;
		lc = c;
		c = getchar();
		return c;
	}
	
	while ((c = ngetchar()) != 3) {
		switch (c) {
			case 13: // Enter
				if (folder.files[folder.cur].type == DT_DIR) {
					folder.path = realpath(folder.files[folder.cur].name, folder.path);
					f_readdir(&folder);
				}
				break;
			case 27: // Arrow keys
				c = ngetchar();
				if (c == '[') {
					c = ngetchar();			
					switch (c) {
						case 68: // Left:
							break;
						case 67: // Right
							break;
						case 65: // Up
							if (folder.cur > 0) folder.cur -= 1;
							break;
						case 66: // Down
							if (folder.cur < folder.numfiles - 1) folder.cur += 1;
							break;
					}
				}
				break;
			case 32: // Mouse down
				c = ngetchar();
				c = ngetchar();
				
				break;
			default:
				goto l_continue;
		}
		w_display(&folder);
		l_continue:
		printf("\x1b[%d;1H'%c' (%d) '%c' (%d) '%c' (%d)", wy, llc, llc, lc, lc, c, c);
	}

	w_reset();
	
	return 0;
}
