#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>

#include <sys/ioctl.h>

#define MAX_PATH 4096
#define MAX_NAME 256

extern struct winsize w_size;
#define wx (w_size.ws_col)
#define wy (w_size.ws_row)

typedef struct {
	char name[MAX_NAME];
	unsigned char type;
} File;

typedef struct {
	char *path;
	File *files;
	unsigned int numfiles;
	unsigned int allocfiles;
	int cur;
	int off;
} Folder;

void __attribute__((noreturn)) die(const char msg[]);

enum Action {
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
	PATH
};

void action(Folder *folder, enum Action action);
void actionarg(Folder *folder, enum Action action, void *arg);
void onresize();
int tick(Folder *folder);

void w_setup();
void w_reset();
void w_display(Folder *folder);

void f_readdir(Folder *folder, char *path);
void f_select(Folder *folder, char *path);
void f_scroll(Folder *folder);
#endif