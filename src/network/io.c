#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include "../../include/network.h"


struct content_buffer {
  size_t size;
  char *buf;

  struct content_buffer *next;
};

#define content_buffer_new() \
  (struct content_buffer *)calloc(1, sizeof(struct content_buffer))

static struct content_buffer *cb = NULL;

static void
content_buffer_free(struct content_buffer *p) {
  struct content_buffer *p_next = NULL;

  while(p != NULL) {
    p_next = p->next;

    if(p->buf != NULL) {
      memset((void *)p->buf, '\0', sizeof(char) * p->size);
      free((void *)p->buf);
      p->buf = NULL;
    }
    p->size = 0;

    free((void *)p);

    p = p_next;
  }
}


struct http_response_content *
http_read_content(const struct net_client *client,
    const struct http_response_header *resp) {
  extern int f_verbose;

  struct content_buffer *cb_cur = NULL;
  size_t size;
    
  char sep_buffer[512];


  cb = content_buffer_new();

  if(IS_CHUNKED(resp->transfer_encoding_type)) {  // Transfer-Encoding: chunked の場合
    long chunk_size = 0L;
    char *p_ln = NULL;

    if(f_verbose) {
      fprintf(stderr, "* 読み込みモード: chunked\n");
    }

    while(1) {
      size = client->readline(client, (char *)sep_buffer, 512);

      if(size <= 0) {
        break;
      }

      // 改行文字の除去
      // '\n' が残留するが問題ない.
      if((p_ln = strstr(sep_buffer, "\r\n")) != NULL) *p_ln = '\0';

      // チャンクの取得
      chunk_size = strtol((const char *)sep_buffer, NULL, 16);
      if(chunk_size == 0) break;

      // chunk_size + 改行文字の長さ(\r\n: 2)
      if(cb_cur == NULL) {
        cb_cur = cb;
      } else {
        cb_cur->next = content_buffer_new();
        cb_cur = cb_cur->next;
      }

      cb_cur->size = chunk_size + 2;
      cb_cur->buf = (char *)calloc(chunk_size + 2, sizeof(char));

      size = client->read((struct net_client *)client, cb_cur->buf, chunk_size + 2);
    }

  } else { // それ以外.
    if(resp->content_length > 0) {
      // Content-Length が指定されている
      if(f_verbose) {
        fprintf(stderr, "* 読み込みモード: Content-Length\n");
      }

      cb->size = resp->content_length;
      cb->buf = (char *)calloc(resp->content_length, sizeof(char));

      client->read(client, (void *)cb->buf, resp->content_length);
    } else {
      // Content-Length が指定されていない場合は,
      // 一行づつ読み込みを行う

      if(f_verbose) {
        fprintf(stderr, "* 読み込みモード: line\n");
      }

      if(cb_cur == NULL) {
        cb_cur = cb;
      } else {
        cb_cur->next = content_buffer_new();
        cb_cur = cb_cur->next;
      }

      cb_cur->size = client->readline(client, (char *)sep_buffer, 512);
      cb_cur->buf = (char *)calloc(cb_cur->size, sizeof(char));
      strncpy(cb_cur->buf, sep_buffer, cb_cur->size);
    }
  }


  //
  // バッファの結合
  struct http_response_content *resp_cont;
  long read_size = 0L;

  resp_cont =
    (struct http_response_content *)calloc(1, sizeof(struct http_response_content));

  // 読み取った合計サイズを取得
  for(cb_cur = cb; cb_cur != NULL; cb_cur = cb_cur->next) {
    read_size += cb_cur->size;
  }

  // 最終的な結合時に改行文字を除去する.
  resp_cont->size    = read_size;
  resp_cont->content = (char *)calloc(read_size, sizeof(char));

  for(cb_cur = cb; cb_cur != NULL; cb_cur = cb_cur->next) {
#if 0
    char *_p_ln = strstr(cb_cur->buf, "\r\n");
    strncat(resp_cont->content, (const char *)cb_cur->buf, cb_cur->size + (_p_ln != NULL ? 2 : 0));
#else
    strncat(resp_cont->content, (const char *)cb_cur->buf, cb_cur->size);
#endif
  }

  // 解放
  content_buffer_free(cb);
  cb = NULL;

  return resp_cont;
}

void
http_response_content_free(struct http_response_content *p) {
  if(p->content != NULL) {
    memset((void *)p->content, '\0', sizeof(char) * p->size);
    free((void *)p->content);
  }

  p->content = NULL;
  free((void *)p);
  p = NULL;
}
