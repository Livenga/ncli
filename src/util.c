#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>


/** エラー出力
 */
int
eprintf(FILE *strm, const char *fn, const char *param) {
  if(param != NULL)
    return fprintf(strm, "%s: %s %s\n", fn, param, strerror(errno));
  return fprintf(strm, "%s: %s\n", fn, strerror(errno));
}


static char
_s_datetime[20];

/** 関数実行時の日時を取得.
 * static char _s_datetime を使用するため,
 * free(void *ptr) の必要はない.
 *
 * @param void
 * @return _s_datetime アドレス
 */
char *
str_datetime(void) {
  const time_t t = time(NULL);
  struct tm *p_ltime = NULL;

  p_ltime = localtime(&t);

  memset((void *)_s_datetime, '\0', sizeof(_s_datetime));

  sprintf(_s_datetime, "%04d-%02d-%02dT%02d:%02d:%02d",
      (1900 + p_ltime->tm_year), (1 + p_ltime->tm_mon),
      p_ltime->tm_mday,          p_ltime->tm_hour,
      p_ltime->tm_min,           p_ltime->tm_sec);

  return (char *)_s_datetime;
}
