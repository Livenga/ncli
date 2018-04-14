#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int
write_content(const char *path,
    const size_t size,
    const char *cont) {
  int fd;
  size_t write_size = 0L,
         b_write, b_total;

  if((fd = open(path, O_WRONLY | O_CREAT, 0644)) < 0)
    return fd;

  write_size = (size == 0) ? strlen(cont) : size;


  b_total = 0L;
  while((b_write = write(fd, (const void *)cont + b_total, write_size - b_total)) > 0) {
    b_total += b_write;
  }

  close(fd);

  return 0;
}
