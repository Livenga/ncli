#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../../include/util.h"
#include "../../include/network_type.h"


/* src/network/ssl.c */
/** client を SSL に対応させる.
 */
extern int
create_ssl_client(struct net_client *client);


/**
 * SSL を使用しないソケット通信で使用する
 * read, write, readline, close 関数
 * 外部から参照させないためすべて静的
 */

static size_t
_read(const struct net_client *client, void *buf, size_t count) {
  ssize_t b_total = 0, b_read;


  while((b_read = read(client->socket, buf + b_total, count - b_total)) > 0) {
    b_total += b_read;
    if(b_total == count) break;
  }

  return (size_t)b_total;
}

static size_t
_readline(const struct net_client *client, char *buf, size_t count) {
  size_t i, r_total, r_byte;

  r_byte = read(client->socket, (void *)&buf[0], 1);
  if(r_byte < 0) return EOF;

  r_total = 1;
  for(i = 1; i < count; ++i) {
    r_byte = read(client->socket, (void *)&buf[i], 1);

    if(r_byte < 0) return EOF;
    else if(r_byte == 0) break;
    else if(buf[i - 1] == '\r' && buf[i] == '\n') {
      ++r_total;
      break;
    }

    ++r_total;
  }

  return r_total;
}

static size_t
_write(const struct net_client *client, const void *buf, size_t count) {
  ssize_t b_write, b_total = 0;


  do {
    b_write = write(client->socket, buf + b_total, count - b_total);

    if(b_write == 0) break;
    else if(b_write < 0) return -1;

    b_total += b_write;
  } while(b_write != 0 || b_total != count);

  return b_total;
}

static int
_close(const struct net_client *client) {
  return close(client->socket);
}


static int
create_connected_socket(
    const char *domain, const char *service, struct sockaddr_in *addr) {
  int sock, status;
  struct addrinfo hints, *res;

  if(addr == NULL) return EOF;
  memset((void *)addr, '\0', sizeof(struct sockaddr_in));

  // struct addrinfo 初期化
  memset((void *)&hints, '\0', sizeof(struct addrinfo));

  hints.ai_flags     = 0;
  hints.ai_family    = AF_INET;
  hints.ai_socktype  = SOCK_STREAM;
  hints.ai_protocol  = 0;
  hints.ai_addr      = NULL;
  hints.ai_canonname = NULL;
  hints.ai_next      = NULL;

  //
  status = getaddrinfo(domain, service, (const struct addrinfo *)&hints, &res);
  if(status < 0) {
    fprintf(stderr,
        "getaddrinfo(3): %s:%s %s\n", domain, service, gai_strerror(status));
    return EOF;
  }


  struct addrinfo *ai = res;
  for(; ai != NULL; ai = ai->ai_next) {
    // struct sockadd_in データの複製
    if(ai->ai_addrlen == sizeof(struct sockaddr_in)) {
      memmove((void *)addr, (const void *)ai->ai_addr, ai->ai_addrlen);
      break;
    }
  }
  freeaddrinfo(res);

  // ソケットの作成と接続
  sock   = socket(AF_INET, SOCK_STREAM, 0);
  status = connect(sock, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
  if(status < 0) {
    eprintf(stderr, "connect(2)", NULL);
    close(sock);
    return EOF;
  }

  return sock;
}


struct net_client *
create_net_client(const char *domain, const char *service) {
  int status;
  struct net_client *client = NULL;


  client = (struct net_client *)calloc(1, sizeof(struct net_client));
  if(client == NULL) goto error;

  client->read  = NULL;
  client->write = NULL;
  client->close = NULL;



  client->socket = create_connected_socket(domain, service, &client->addr);
  if(client->socket < 0) goto error;

  // ポート番号が 443 の場合, SSL用に設定
  if(ntohs(client->addr.sin_port) == 443) {
    // この関数で read, write 等を設定.
    status = create_ssl_client(client);
  } else {
    client->read     = _read;
    client->readline = _readline;
    client->write    = _write;
    client->close    = _close;
  }

  return client;

error:
  if(client != NULL) {
    memset((void *)client, '\0', sizeof(struct net_client));
    free((void *)client);
    client = NULL;
  }

  return NULL;
}

void
write_to_file(struct net_client *client, const char *output_path) {
  int fd;
  char buf[512];

  size_t size;

  if((fd = open(output_path, O_WRONLY | O_CREAT, 0644)) < 0) return;

  while(1) {
    size = client->readline(client, buf, 512);

    if(size <= 0) break;
    else if(strcmp(buf, "\r\n") == 0 ||
        strcmp(buf, "0\r\n") == 0) break;

    write(fd, buf, size);
  }

  close(fd);
}
