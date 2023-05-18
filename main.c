#define MAIN_C

#include "common.h"
#include "config.h"

void __attribute__((noreturn)) die(const char msg[]) {
	printf("Error: %s\n", msg);
	exit(1);
}

Folder folder = { NULL, NULL, 0, 0, 0, 0 };

void actionarg(Folder *folder, enum Action id, void *arg) {
	switch (id) {
	default: break;
	case GOTO:
		if (folder->cur == (unsigned int)(long)arg) {
			f_readdir(folder, folder->files[folder->cur].name);
		} else {
			folder->cur = (unsigned int)(long)arg;
			f_scroll(folder);
		}
		w_display(folder);
		break;
	case PATH:
		(void)arg; // I dont know why but gcc is complaing about the next line if i dont do anything here
		if ((unsigned int)(long)arg == 0) {
			f_readdir(folder, "/");
			w_display(folder);
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
		w_display(folder);
		break;
	}
}
void action(Folder *folder, enum Action id) {
	switch (id) {
	default: break;
	case FORW:
		if (folder->files[folder->cur].type == DT_DIR) {
			f_readdir(folder, folder->files[folder->cur].name);
		}      w_display(folder);
		break;
	case BACK:
		// get cur dir
		printf("hello\n");
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
		w_display(folder);
		break;
	case UP:
		folder->cur -= 1;
		f_scroll(folder);
		w_display(folder);
		break;
	case DN:
		folder->cur += 1;
		f_scroll(folder);
		w_display(folder);
		break;
	case PGUP:
		folder->cur -= wy;
		f_scroll(folder);
		w_display(folder);
		break;
	case PGDN:
		folder->cur += wy;
		f_scroll(folder);
		w_display(folder);
		break;
	case HOME:
		folder->cur = 0;
		f_scroll(folder);
		w_display(folder);
		break;
	case END:
		folder->cur = folder->numfiles - 1;
		f_scroll(folder);
		w_display(folder);
		break;
	case TGLSEL:
		break;
	case RENAME:
		break;
	case DEL:
		break;
	}
}

void onresize() {
	f_scroll(&folder);
	w_display(&folder);
}

int main(int argc, char *argv[]) {
	
	if (argc == 1) { // no args use working directory
		folder.path = getcwd(folder.path, 255);
	} else {
		folder.path = realpath(argv[1], folder.path);
	}
	if (folder.path == NULL) {
		die("Coudn't open directory");
	}
	printf("Opened: %s\n", folder.path);

	f_readdir(&folder, NULL);

	w_setup();
	w_display(&folder);

	while (tick(&folder) > 0) { ;; }

	w_reset();
	
	return 0;
	
}
