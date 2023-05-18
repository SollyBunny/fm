
#ifndef FOLDER_H
#define FOLDER_H
#include "include.h"

int f_alphacompare(const void *a, const void *b) {
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
	int ctn(char c) {
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

	A = (void*)(long)ctn(*Ac);
	B = (void*)(long)ctn(*Bc);
	//printf("%s (%d) > %s (%d)\n", (char*)a, (int)(long)A, (char*)b, (int)(long)B);
	return A > B;

	#undef ctn
}

DIR *_f_readdir_dir;
struct stat *_f_readdir_stat;
struct dirent *_f_readdir_entry;
void f_readdir(Folder *folder) {
	chdir(folder->path);
	//if ((_f_readdir_dir = opendir(".")) == NULL) {
	if ((_f_readdir_dir = opendir(".")) == NULL) {
		die("Coudn't read dir");
	} else {
		folder->numfiles = 0;
		while ((_f_readdir_entry = readdir(_f_readdir_dir)) != NULL) {
			if (_f_readdir_entry->d_name[0] == '.' && _f_readdir_entry->d_name[1] == '\0') continue; // ignore "."
			if (folder->numfiles + 2 > folder->allocfiles) {
				if (folder->allocfiles == 0) folder->allocfiles  = 8;
				else                         folder->allocfiles *= 2;
				folder->files = realloc(folder->files, folder->allocfiles * sizeof(folder->files[0]));
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
	qsort(folder->files, folder->numfiles, sizeof(folder->files[0]), f_alphacompare);
}

#endif
