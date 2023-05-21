#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#define DEFS_ONLY
#include "config.h"
#undef DEFS_ONLY

#define MAX_PATH 4096
#define MAX_NAME 256

extern struct winsize w_size;
#define wx (w_size.ws_col)
#define wy (w_size.ws_row)

typedef struct {
	char name[MAX_NAME];
	unsigned char type;
} File;

enum Enum {
	NONE = 0,
	QUIT,
	FORW,
	BACK,
	UP,
	DN,
	PGUP,
	PGDN,
	TGLSEL,
	DEL,
	RENAME,
	HOME,
	END,
	GOTO,
	PATH,
	PERMDIR
};

typedef struct {
	char *path;
	File *files;
	unsigned int numfiles;
	unsigned int allocfiles;
	int cur;
	int off;
	enum Enum error;
	#ifdef FIFOEXEC
		struct {
			char path[MAX_NAME];
			int fd;
		} fifo;
	#endif
} Folder;

void __attribute__((noreturn)) die(char *msg);

void onresize();

void w_setup();
void w_reset();

void f_readdir(Folder *folder, char *path);
void f_select(Folder *folder, char *path);
void f_scroll(Folder *folder);
void f_scrollto(Folder *folder, int y);
void f_scrollby(Folder *folder, int dy);
void f_close(Folder *folder);
void f_fifo(Folder *folder);
void f_display(Folder *folder);

#include "config.h"

#endif