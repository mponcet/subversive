#include <subversive/ksyms.h>
#include <subversive/libc.h>

void *subversive_memcpy(void *dst, const void *src, unsigned int n)
{
	char *_dst = dst;
	const char *_src = src;

	while (n--)
		*(_dst++) = *(_src++);

	return dst;
}

void *subversive_memset(void *s, int c, unsigned int n)
{
	char *_s = s;

	while (n--)
		*(_s++) = c;

	return s;
}

unsigned int subversive_strlen(const char *s)
{
	unsigned int ret = 0;

	while (*(s++))
		ret++;

	return ret;
}

int subversive_strcmp(const char *s1, const char *s2)
{
	for (; *s1 == *s2; s1++, s2++)
		if (!*s1)
			return 0;

	return *s1 - *s2;
}

char *subversive_strndup_from_user(const char *ustr, unsigned int ulen)
{
	char *kstr, *kstr_copy;
	unsigned int klen;

	kstr = subversive_vmalloc(ulen+1);
	if (!kstr)
		return NULL;

	if (ksyms._copy_from_user(kstr, ustr, ulen)) {
		subversive_vfree(kstr);
		return NULL;
	}

	kstr[ulen] = 0;
	klen = subversive_strlen(kstr);
	kstr[klen] = 0;
	if (klen == ulen)
		return kstr;

	/* copy inside a smaller block */
	kstr_copy = subversive_vmalloc(klen+1);
	if (!kstr_copy)
		return kstr;

	subversive_memcpy(kstr_copy, kstr, klen);
	kstr_copy[klen] = 0;
	subversive_vfree(kstr);

	return kstr_copy;
}
