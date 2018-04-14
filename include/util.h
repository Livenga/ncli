#ifndef _UTIL_H
#define _UTIL_H

#define skip_ch(ptr, ch) for(; *ptr== ch; ++ptr)

/* src/io.c */
extern int
write_content(const char *path,
    const size_t size,
    const char *cont);

/* src/util.c */
extern int
eprintf(FILE *strm, const char *fn, const char *param);
extern char *
str_datetime(void);

#endif
