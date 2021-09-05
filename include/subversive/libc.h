#ifndef __LIBC_H
#define __LIBC_H

#include <subversive/ksyms.h>

void *subversive_memcpy(void *dst, const void *src, unsigned int n);
void *subversive_memset(void *s, int c, unsigned int n);
unsigned int subversive_strlen(const char *s);
int subversive_strcmp(const char *s1, const char *s2);

char *subversive_strndup_from_user(const char *ustr, unsigned int ulen);

static inline int subversive_strncmp(const char *s1, const char *s2, unsigned int n)
{
	return ksyms.strncmp(s1, s2, n);
}

static inline unsigned int subversive_strlcat(char *s1, const char *s2, unsigned int n)
{
	return ksyms.strlcat(s1, s2, n);
}

#define subversive_snprintf(str, size, format, ...) ksyms.snprintf(str, size, format, __VA_ARGS__)

static inline void *subversive_vmalloc(unsigned int size)
{
	return ksyms.vmalloc(size);
}

static inline void *subversive_vfree(void *ptr)
{
	return ksyms.vfree(ptr);
}

#endif
