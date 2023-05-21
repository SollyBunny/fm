
// How many items can be seen after the current selected when scrolling down (improves readability)
#define SEELIMIT 2
// If set, will create a FIFO, and exec the program with argument of the FIFO location (where the FIFO will output directories)
#define FIFOEXEC {"kitty", "@", "launch", "--no-response", "--location=vsplit", "--keep-focus", "/home/solly/Code/c/fm/preview", NULL, NULL}
// If set, will change dir of the parent process on exit
#define CHANGEDIR

#ifndef DEFS_ONLY
/* 
	The different preview programs to use.
	Note that the order is important, the first one which matches will be executed.
	The filename will be an absolute path starting with '/' and ending with '\0'
	Make sure the program you specify is in path or given as an absolute path
	The last element of args and args[filenum] must be NULL (and not the same index)
*/
#ifdef PREVIEW_C
char *cmd_text[] = {"micro", NULL, NULL};
char *cmd_image[] = {"kitty", "+kitten", "icat", "-z", "-1", "--scale-up", NULL, NULL};
struct {
	char *mime; // Will execute args if mime starts with this
	int filenum; // Where the file path will be inserted
	char **args;
} previewprograms[] = {
	{"text", 1, cmd_text},
	{"image", 5, cmd_image}
};
#endif

/*
	`tick()`
	Determine an action to be done from keyboard input
	Return an action code (see `common.h`)
	:)
*/
#ifdef MAIN_C
inline uint32_t tick(char *inp, int size) {
	switch (inp[0]) {
	case 3: // C-c
		return QUIT;
	case 13: // Enter
		return FORW;
	case '\b': // Backspace
		return BACK;
	case '\x1b': // Escaped key
		if (size == 1) return QUIT; // escape
		switch (inp[1]) {
		case '[': // Island keys
			if (size == 2) return NONE;
			switch (inp[2]) {
				case 'A': return UP; // Up
				case 'B': return DN; // Down
				case 'C': return FORW; // Right
				case 'D': return BACK; // Left
				case 'F': return END; // End
				case 'H': return HOME; // Home
				case '5': return PGUP; // Page Up
				case '6': return PGDN; // Page Down
			case 'M': // Mouse
				if (size == 3) return NONE;
				switch (inp[3]) {
					case 96: return UP; // Scroll Up
					case 97: return DN; // Scroll Down
					case 32: // Left click
						if (size < 5) return NONE;
						if (inp[5] == 34)
							return PATH | ((inp[4] - 33) << 8);
						else // Select file
							return GOTO | ((inp[5] - 34) << 8);
				}
				break;
			}
			break;
		}
	}
	return NONE;
}
#endif

#endif