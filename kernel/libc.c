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

int anima_strcmp(const char *s1, const char *s2)
{
	for (; *s1 == *s2; s1++, s2++)
		if (!*s1)
			return 0;

	return *s1 - *s2;
}

int anima_strncmp(const char *s1, const char *s2, unsigned int n)
{
	while (n--)
		if (*(s1++) != *(s2++))
			return *(s1 - 1) - *(s2 - 1);
	return 0;
}
