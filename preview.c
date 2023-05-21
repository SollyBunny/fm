#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <magic.h>

#define PREVIEW_C
#include "config.h"

magic_t magic_ctx;

struct winsize ws;
#define W ws.ws_col
#define H ws.ws_row

const char* SIZES[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB" };

int fd;
char *fddir;

struct {
	char path[1024];
	enum {
		NONE = -1,
		DIR,
		LNK,
		IMG,
		AUD,
		VID,
		TXT
	} type;
	pid_t pid;
	struct stat stat;
	char *mime;
} preview;

void previewbar() {
	printf("\x1b[0m\x1b[2J\x1b[1;1H"); // clear
	if (preview.type == DIR) {
		printf("Dir ");
		goto l_size_start;
	} if (preview.type == LNK) {
		printf("Lnk ");
	}
	// File Size
	static float size;
	static unsigned int i;
	size = preview.stat.st_size;
	i = 0;
	goto l_size_start;
	while (size > 1024) {
		if (i == sizeof(SIZES) / sizeof(SIZES[0])) {
			printf(">1024 YiB ");
		}
		goto l_size_end;
		++i;
		l_size_start:
		size /= 1024;
	}
	printf("%.2f %s ", size, SIZES[i]);
	l_size_end:
	// Permissions
	printf(
		"Own: %c%c%c "
		"Grp: %c%c%c "
		"All: %c%c%c ",
		(preview.stat.st_mode & S_IRUSR) ? 'R' : '-', (preview.stat.st_mode & S_IWUSR) ? 'W' : '-', (preview.stat.st_mode & S_IXUSR) ? 'X' : '-',
		(preview.stat.st_mode & S_IRGRP) ? 'R' : '-', (preview.stat.st_mode & S_IWGRP) ? 'W' : '-', (preview.stat.st_mode & S_IXGRP) ? 'X' : '-',
		(preview.stat.st_mode & S_IROTH) ? 'R' : '-', (preview.stat.st_mode & S_IWOTH) ? 'W' : '-', (preview.stat.st_mode & S_IXOTH) ? 'X' : '-'
	);
	// Done Stat
	fflush(stdout);
}


void previewalarm(int sig) {
	if (sig != SIGALRM) return;
	if (preview.pid == -1) return;
	kill(preview.pid, SIGKILL);
	preview.pid = -1; // just to be sure
}

void previewkill() {
	if (preview.pid == -1) return;
	 // NOTE: here be jank, be wary
	kill(preview.pid, SIGTERM);
	alarm(3); // wait for SIGKILL (creates interupt previewalarm in 3s)
	waitpid(preview.pid, NULL, 0); // wait for exit
	preview.pid = -1;
	alarm(0); // cancel any ongoing alarm
}

void previewspawn(char *args[]) {
	pid_t pid = fork();
	if (pid == -1) {
		printf("Failed to fork\n");
		return;
	}
	if (pid == 0) { // new proc
        execvp(args[0], args);
		printf("Failed to exec\n");
		exit(0);
	}
	preview.pid = pid;	
}

void previewupdate() {
	previewkill();
	// Figure out type
	if (stat(preview.path, &preview.stat) != 0) {
		printf("Failed to stat %s\n", preview.path);
		return;
	}
	// File Type
	if (S_ISREG(preview.stat.st_mode)) {
		preview.mime = (char*)magic_file(magic_ctx, preview.path);
		if (strncmp(preview.mime, "text", 4) == 0) {
			preview.type = TXT;
			char* args[] = { "micro", preview.path, NULL };
			previewspawn(args);
			return;
		} else if (strncmp(preview.mime, "image", 5) == 0) {
			preview.type = IMG;
			previewbar();
			static char pos[4 + 1 + 4 + 1];
			sprintf(pos, "%ux%u@0x1", W, H - 1);
			char* args[] = { "kitty", "+kitten", "icat", "-z", "-1", "--scale-up", "--place", pos, preview.path, NULL };
			previewspawn(args);
		} else if (strncmp(preview.mime, "video", 5) == 0) {
			preview.type = VID;
			goto l_mime_audio;
		} else if (strncmp(preview.mime, "audio", 5) == 0) {
			preview.type = AUD;
			l_mime_audio:
			previewbar();
			char* args[] = { "mpv", "--vo=kitty", "--vo-kitty-cols=0000", "--vo-kitty-rows=0000", "--vo-kitty-top=0000", "--vo-kitty-use-shm=yes", "--profile=sw-fast", preview.path, NULL };
			sprintf(args[2], "--vo-kitty-cols=%u", W);
			sprintf(args[3], "--vo-kitty-rows=%u", H - 1);
			previewspawn(args);
		}
	} else if (S_ISDIR(preview.stat.st_mode)) {
		preview.mime = NULL;
		preview.type = DIR;
		char* args[] = { "nnn", "--list", preview.path, NULL };
		previewspawn(args);
		return;
	} else if (S_ISLNK(preview.stat.st_mode)) {
		preview.mime = NULL;
		preview.type = LNK;
		previewbar();
	}
}

void handleresize(int _) {
	(void)_;
	ioctl(0, TIOCGWINSZ, &ws);
	previewupdate();
}
void handleterm(int _) {
	(void)_;
	close(fd);
	previewkill();
	remove(fddir);
	printf("Bye bye!\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	// Init FIFO
	if (argc != 2) {
		printf("Usage %s [FIFO PIPE]\n", argv[0]);
		return 1;
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		printf("Failed to open FIFO at %s\n", argv[1]);
		return 2;
	}
	fddir = argv[1];
	// Init other stuff
	preview.pid = -1;
	signal(SIGALRM, previewalarm);
	signal(SIGWINCH, handleresize);
	signal(SIGTERM, handleterm);
	signal(SIGINT, handleterm);
	signal(SIGQUIT, handleterm);
	signal(SIGTSTP, handleterm);
	signal(SIGPIPE, handleterm);
	ioctl(0, TIOCGWINSZ, &ws);
	// Init libmagic
	magic_ctx = magic_open(MAGIC_MIME_TYPE);
    if (magic_ctx == NULL) {
        printf("Failed to initialize magic library\n");
        return 3;
    }
    if (magic_load(magic_ctx, NULL) != 0) {
        printf("Failed to load magic database\n");
        magic_close(magic_ctx);
        return 4;
    }
	// Read FIFO
	size_t readsize;
	char *p;
	while ((readsize = read(fd, preview.path, sizeof(preview.path))) > 0) {
		preview.path[readsize] = '\0';
		previewupdate();
	}
	handleterm(0);
	return 0;
}
