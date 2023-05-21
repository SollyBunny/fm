#ifndef FOLDER_H
#define FOLDER_H

#include "common.h"

void updatefifo(Folder *folder) {
	static char path[MAX_PATH];
	sprintf(path, "%s/%s", folder->path, folder->files[folder->cur].name);
	write(folder->fifo.fd, path, strlen(path));
	fsync(folder->fifo.fd);	
}
int comparechar(char c) {
	switch (c) {
		case '\0':        return -1;
		case '.' :        return 0 ;
		case ' ' :        return 1 ;
		case '_' :        return 2 ;
		case ',' :        return 3 ;
		case '>' :        return 4 ;
		case '<' :        return 5 ;
		case '?' :        return 6 ;
		case '!' :        return 7 ;
		case '\'':        return 8 ;
		case '"' :        return 9 ;
		case '$' :        return 10;
		case '%' :        return 11;
		case '^' :        return 12;
		case '&' :        return 13;
		case '*' :        return 14;
		case '-' :        return 15;
		case '+' :        return 16;
		case '=' :        return 17;
		case '#' :        return 18;
		case '~' :        return 20;
		case '@' :        return 21;
		case '`' :        return 22;
		case '|' :        return 23;
		case '(' :        return 24;
		case ')' :        return 25;
		case '[' :        return 26;
		case ']' :        return 27;
		case '{' :        return 28;
		case '}' :        return 29;
		case '0' ... '9': return 30 + (c - '0');
		case 'A' ... 'Z': return 40 + (c - 'A');
		case 'a' ... 'z': return 60 + (c - 'a');
		default:          return c;
	}
}
int comparestr(const void *a, const void *b) {
	File *A = (File*)a;
	File *B = (File*)b;
	if (A->type > B->type) return 1;
	if (A->type < B->type) return 0;
	static char *Ac;
	static char *Bc;
	Ac = (char*)(A->name);
	Bc = (char*)(B->name);
	while (
		*Ac != '\0' &&
		*Bc != '\0' &&
		*Ac == *Bc 
	) {
		++(Ac);
		++(Bc);
	}
	if (*Ac == *Bc) return 0; // this should be impossible (files of same name)
	//if (*A == '\0') return -1; // A ended first
	//if (*B == '\0') return 1; // B ended first
	A = (File*)(long)comparechar(*Ac);
	B = (File*)(long)comparechar(*Bc);
	return A > B;
}

DIR *_f_readdir_dir;
struct stat *_f_readdir_stat;
struct dirent *_f_readdir_entry;
void f_readdir(Folder *folder, char *path) {
	if (path == NULL) {
		path = folder->path;
		if (path == NULL) return;
	}
	if (chdir(path) == -1) {
		folder->error = errno;
		return;
	}
	free(folder->path);
	folder->path = getcwd(NULL, 0);
	chdir(folder->path);
	if ((_f_readdir_dir = opendir(".")) == NULL) {
		die("Coudn't read dir");
	} else {
		folder->numfiles = 0;
		while ((_f_readdir_entry = readdir(_f_readdir_dir)) != NULL) {
			if (_f_readdir_entry->d_name[0] == '.') {
				if (_f_readdir_entry->d_name[1] == '\0') continue; // ignore "."
				if (_f_readdir_entry->d_name[1] == '.' && _f_readdir_entry->d_name[2] == '\0') continue; // ignore ".."
			}
			if (folder->numfiles + 2 > folder->allocfiles) {
				if (folder->allocfiles == 0) folder->allocfiles  = 8;
				else                         folder->allocfiles *= 2;
				folder->files = (File*)realloc(folder->files, folder->allocfiles * sizeof(folder->files[0]));
			}
			unsigned int i;
			for (i = 0; _f_readdir_entry->d_name[i] != '\0'; ++i) {
				folder->files[folder->numfiles].name[i] = _f_readdir_entry->d_name[i];
			}
			folder->files[folder->numfiles].name[i] = '\0';
			folder->files[folder->numfiles].type = _f_readdir_entry->d_type;
			++folder->numfiles;
		}
		closedir(_f_readdir_dir);
	}
	folder->cur = 0;
	folder->off = 0;
	qsort(folder->files, folder->numfiles, sizeof(folder->files[0]), comparestr);
}

int oldcurpos;
void f_scroll(Folder *folder) {
	if (folder->cur == -1) {
		folder->cur = folder->numfiles - 1;
	} else if (folder->cur == folder->numfiles) {
		folder->cur = 0;
	} else if (folder->cur < 0) {
		folder->cur = 0;
	} else if (folder->cur > folder->numfiles) {
		folder->cur = folder->numfiles - 1;
	}
	if (folder->cur + SEELIMIT > folder->off + wy - 1) {
		if (folder->cur + SEELIMIT + 1 > folder->numfiles)
			folder->off = folder->numfiles - wy + 1;
		else
			folder->off = folder->cur - wy + 1 + SEELIMIT;
	} else if (folder->cur < folder->off + SEELIMIT) {
		if (folder->cur < SEELIMIT)
			folder->off = 0;
		else
			folder->off = folder->cur - SEELIMIT;
	}
}
void f_scrollto(Folder *folder, int y) {
	oldcurpos = folder->cur;
	folder->cur = y;
	f_scroll(folder);
	if (folder->cur == oldcurpos) return;
	updatefifo(folder);
	f_display(folder);
}
void f_scrollby(Folder *folder, int dy) {
	oldcurpos = folder->cur;
	folder->cur += dy;
	f_scroll(folder);
	if (folder->cur == oldcurpos) return;
	updatefifo(folder);
	f_display(folder);
}
void f_select(Folder *folder, char *path) {
	for (unsigned int i = 0; i < folder->numfiles; ++i) {
		if (strcmp(folder->files[i].name, path) == 0) {
			f_scrollto(folder, i);
			break;
		}
	}
}
void f_fifo(Folder *folder) { 
	#ifdef FIFOEXEC
		struct timespec currenttime;
    	clock_gettime(CLOCK_REALTIME, &currenttime);
		sprintf(folder->fifo.path, "/tmp/fm_%ld%ld.fifo", currenttime.tv_sec, currenttime.tv_nsec);
		if ((mkfifo(folder->fifo.path, 0666) == -1))
			die("Failed to create FIFO");
		// fork and exec FIFOEXEC
		pid_t pid = fork();
		if (pid == -1) {
			die("Failed to fork FIFOEXEC");
		} else if (pid == 0) {
			char *args[] = FIFOEXEC;
			args[(sizeof(args) / sizeof(args[0])) - 2] = folder->fifo.path;
			execvp(args[0], args);
			exit(1);
		}
		// Open FIFO after someone starts reading
		if ((folder->fifo.fd = open(folder->fifo.path, O_WRONLY)) == -1)
			die("Failed to open FIFO");
	#endif
}

void f_close(Folder *folder) {
	#ifdef FIFOEXEC
		close(folder->fifo.fd);
		unlink(folder->fifo.path);
	#endif
	free(folder->files);
	free(folder);
}

void f_display(Folder *folder) {
	printf("\x1b[2J\x1b[H%s %d/%d\n", folder->path, folder->cur + 1, folder->numfiles);
	unsigned int end;
	if (folder->numfiles < wy + folder->off) end = folder->numfiles;
	else                                     end = wy + folder->off;
	for (unsigned int i = folder->off; i < end; ++i) {
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
		putchar('\n');
	}
	if (folder->error == NONE) return;
	
	if (folder->error == 0) {


	} else {
		printf("\x1b[%d;1H\x1b[31m%s\x1b[0m", wy + 2, strerror(folder->error));
	}
	
	fflush(stdout);
	folder->error = NONE;
	
}

#endif