#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include "../../include/network_type.h"


static SSL_CTX *ctx = NULL;
static SSL     *ssl = NULL;


static int
_init(void) {
  SSL_load_error_strings();
  SSL_library_init();

  ctx = SSL_CTX_new(SSLv23_client_method());
  if(ctx == NULL) return F_CTX_NEW;

  // TODO: 証明書の設定が必要
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

  ssl = SSL_new(ctx);
  if(ssl == NULL) return F_SSL_NEW;

  return 0;
}

static size_t
_read(const struct net_client *client, void *buf, size_t count) {
  if(client->ssl == NULL) return -1;

  size_t b_total = 0, b_read = 0;

  // クライアントに対する入力がなくなるか,
  // エラーが発生するまで読取りを行う.
  while((b_read = SSL_read(client->ssl, buf + b_total, count - b_total)) > 0) {
    b_total += b_read;
    if(b_total == count) break;
  }

  return b_total;
}

static size_t
_readline(const struct net_client *client, char *buf, size_t count) {
  int    status;
  size_t i, b_read;

  memset((void *)buf, '\0', sizeof(char) * count);

  status = SSL_read(client->ssl, (void *)&buf[0], 1);
  if(status < 1) return status;

  b_read = 1;
  for(i = 1; i < count; ++i) {
    status = SSL_read(client->ssl, (void *)&buf[i], 1);

    if(status < 1) return status;
    else if(buf[i - 1] == '\r' && buf[i] == '\n') break;

    ++b_read;
  }

  return b_read;
}


static size_t
_write(const struct net_client *client, const void *buf, size_t count) {
  if(client->ssl == NULL) return -1;

  size_t b_total = 0, b_write = 0;

  // 書き込み
  do {
    b_write = SSL_write(client->ssl, buf + b_total, count - b_total);

    if(b_write < 0) return -1;
    else if(b_write > 0) b_total += b_write;
  } while(b_write != 0 || b_total != count);

  return b_total;
}

static int
_close(const struct net_client *client) {

  SSL_shutdown(client->ssl);
  SSL_free(client->ssl);

  SSL_CTX_free(ctx);

  close(client->socket);

  return 0;
}


const static struct enum_value ssl_errors[] = {
  {SSL_ERROR_NONE, "none"},
  {SSL_ERROR_WANT_READ, "want read"},
  {SSL_ERROR_WANT_WRITE, "want write"},
  {SSL_ERROR_WANT_CONNECT, "want connect"},
  {SSL_ERROR_WANT_ACCEPT, "want accept"},
  {SSL_ERROR_WANT_X509_LOOKUP, "want x509 lookup"},
  {SSL_ERROR_WANT_ASYNC, "want async"},
  {SSL_ERROR_WANT_ASYNC_JOB, "want async job"},
  {SSL_ERROR_SYSCALL, "syscall"},
  {SSL_ERROR_SSL, "ssl"},
  {0, NULL},
};


static void
ssl_print_error(const int err) {
  struct enum_value *e_detail = (struct enum_value *)ssl_errors;

  do {
    if(err == e_detail->type) {
      printf("* \033[1;31mError: %s(%d)\033[0m\n",
          e_detail->name, e_detail->type);
    }
  } while((++e_detail)->type != 0);
}


int
create_ssl_client(struct net_client *client) {
  int status;
  status = _init();

  if(status != 0) {
    return EOF;
  }

  /**
   * NOTE: SSL_* 系の関数のエラーコードは, 0 が成功ではない.
   */
  // ファイルディスクリプタを SSL に設定
  status = SSL_set_fd(ssl, client->socket);
  // エラー
  if(status == 0) {
    return EOF;
  }

  // SSL に接続
  status = SSL_connect(ssl);
  // エラー
  if(status != 1) {
    int ssl_err = SSL_get_error(ssl, status);
    ssl_print_error(ssl_err);
    return EOF;
  }

  client->is_ssl   = 1;
  client->ssl      = ssl;
  client->close    = _close;
  client->read     = _read;
  client->readline = _readline;
  client->write    = _write;

  return 0;
}

