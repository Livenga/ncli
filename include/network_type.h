#ifndef _NETWORK_TYPE_H
#define _NETWORK_TYPE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <openssl/ssl.h>

#ifndef _TYPE_H
#include "type.h"
#endif



#define UA_GOOGLE_CHROME \
  "Mozilla/5.0 (X11; Linux x86_64) " \
  "AppleWebKit/537.36 (KHTML, like Gecko) "\
  "Chrome/65.0.3325.181 Safari/537.36"

struct net_client {
  // socket 通信
  int socket;
  struct sockaddr_in addr;

  // SSL 通信
  SSL *ssl;
  int is_ssl;

  /* 通常ソケットおよび SSL で暗号化されたソケットに対応するため,
   * 関数ポインタを使用して各々のラッパー関数を設定.
   */
  size_t     (*read)(const struct net_client *client, void *buf, size_t count);
  size_t    (*write)(const struct net_client *client, const void *buf, size_t count);
  size_t (*readline)(const struct net_client *client, char *buf, size_t count);
  int       (*close)(const struct net_client *client);
};

struct http_field {
  char *name;
  char *value;

  struct http_field *next;
};


struct http_response_header {
  int    status;
  double version;

  long     content_length;
  unsigned transfer_encoding_type;

  struct http_field *fields;
};

struct http_response_content {
  size_t size;
  char *content;
};

enum transfer_encoding_type {
  CHUNKED  = 0b00000001,
  COMPRESS = 0b00000010,
  DEFLATE  = 0b00000100,
  GZIP     = 0b00001000,
  IDENTITY = 0b00010000,
};


enum network_error_code {
  F_CTX_NEW = 127,
  F_SSL_NEW = 128,
};

#endif
