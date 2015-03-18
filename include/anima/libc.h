#ifndef __LIBC_H
#define __LIBC_H

#include <anima/ksyms.h>

void *anima_memcpy(void *dst, const void *src, unsigned int n);
void *anima_memset(void *s, int c, unsigned int n);
unsigned int anima_strlen(const char *s);
int anima_strcmp(const char *s1, const char *s2);
int anima_strncmp(const char *s1, const char *s2, unsigned int n);
char *anima_strndup_from_user(const char *ustr, unsigned int ulen);

static inline void *anima_vmalloc(unsigned int size)
{
	return ksyms.vmalloc(size);
}

static inline void *anima_vfree(void *ptr)
{
	return ksyms.vfree(ptr);
}

#endif
