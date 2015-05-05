#include <anima/ksyms.h>
#include <anima/libc.h>

void *anima_memcpy(void *dst, const void *src, unsigned int n)
{
	char *_dst = dst;
	const char *_src = src;

	while (n--)
		*(_dst++) = *(_src++);

	return dst;
}

void *anima_memset(void *s, int c, unsigned int n)
{
	char *_s = s;

	while (n--)
		*(_s++) = c;

	return s;
}

unsigned int anima_strlen(const char *s)
{
	unsigned int ret = 0;

	while (*(s++))
		ret++;

	return ret;
}

int anima_strcmp(const char *s1, const char *s2)
{
	for (; *s1 == *s2; s1++, s2++)
		if (!*s1)
			return 0;

	return *s1 - *s2;
}

char *anima_strndup_from_user(const char *ustr, unsigned int ulen)
{
	char *kstr, *kstr_copy;
	unsigned int klen;

	kstr = anima_vmalloc(ulen+1);
	if (!kstr)
		return NULL;

	if (ksyms._copy_from_user(kstr, ustr, ulen)) {
		anima_vfree(kstr);
		return NULL;
	}

	kstr[ulen] = 0;
	klen = anima_strlen(kstr);
	kstr[klen] = 0;
	if (klen == ulen)
		return kstr;

	/* copy inside a smaller block */
	kstr_copy = anima_vmalloc(klen+1);
	if (!kstr_copy)
		return kstr;

	anima_memcpy(kstr_copy, kstr, klen);
	kstr_copy[klen] = 0;
	anima_vfree(kstr);

	return kstr_copy;
}
