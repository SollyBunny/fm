
#ifndef INCLUDE_H
#define INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

typedef unsigned char bool;
#define true 1
#define false 0

#define MAX_PATH 4096

struct winsize ws;
#define wx (ws.ws_col)
#define wy (ws.ws_row)

typedef struct {
	char name[256];
	unsigned char type;
} File;

typedef struct {
	char *path;
	File *files;
	size_t numfiles;
	size_t allocfiles;
	unsigned int cur;
	unsigned int off;
} Folder;

void __attribute__((noreturn)) die(const char msg[]) {
	printf("Error: %s\n", msg);
	exit(1);
}

extern inline void w_setup();
extern inline void w_reset();
extern void w_display(Folder *folder);

extern void f_readdir(Folder *folder);

#include "config.h"
#include "window.h"
#include "folder.h"

#endif
