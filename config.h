#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

// How many items can be seen after the current selected when scrolling down (improves readability)
#define SEELIMIT 2

#ifdef MAIN_C
inline int tick(Folder *folder) {
	static char c;
	switch ((c = getchar())) {
	case 3: // C-c
	case 23: // escape
	case 'q':
		return -1;
	case 13: // Enter
		action(folder, FORW);
		break;
	case '\b': // Backspace
		action(folder, BACK);
		break;
	case '\x1b': // Escaped key
		switch (c = getchar()) {
		case '[': // Island keys
			switch (c = getchar()) {
				case 'A': action(folder, UP); break; // Up
				case 'B': action(folder, DN); break; // Down
				case 'C': action(folder, FORW); break; // Right
				case 'D': action(folder, BACK); break; // Left
				case 'F': action(folder, END); break; // End
				case 'H': action(folder, HOME); break; // Home
				case '5': c = getchar(); action(folder, PGUP); break; // Page Up
				case '6': c = getchar(); action(folder, PGDN); break; // Page Down
			case 'M': // Mouse
				switch (c = getchar()) {
					case 96: action(folder, UP); break; // Scroll Up
					case 97: action(folder, DN); break; // Scroll Down
					default:
						static unsigned int x; x = getchar() - 33;
						static unsigned int y; y = getchar() - 33;
					switch (c) {
					case 32: // Left click
						if (y == 0) {
							actionarg(folder, PATH, (void*)(long)(x));
						} else { // Select file
							actionarg(folder, GOTO, (void*)(long)(y + folder->off - 1));
						}
						break;
					}
				}
				c = getchar(); // x
				c = getchar(); // y
				break;
			}
			break;
		case 32: // Mouse down
			c = getchar();
			break;
		}
	}
	return 1;
}
#endif

#endif