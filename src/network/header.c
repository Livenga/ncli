#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../include/util.h"
#include "../../include/network_type.h"

#define _BUF_SIZE 1024


char *
http_header_get_value(const struct http_response_header *p_head, const char *name);


const static struct enum_value transfer_values[] = {
  {CHUNKED,  "chunked"},
  {COMPRESS, "compress"},
  {DEFLATE,  "deflate"},
  {GZIP,     "gzip"},
  {IDENTITY, "identity"},
  {0,         NULL},
};

static void
_get_transfer_en(struct http_response_header *resp) {
  unsigned type = 0;
  char     *p_value, *p_sep;

  struct enum_value *p_ev = NULL;


  if((p_value = http_header_get_value(resp, "transfer-encoding")) == NULL)
    return;


  for(p_sep = p_value; p_sep != NULL && *p_sep != '\0'; p_sep = strchr(p_sep, ',')) {
    p_ev = (struct enum_value *)transfer_values;

    // 仕切り文字の識別と無駄な空白を除去
    if(*p_sep == ',') ++p_sep;
    skip_ch(p_sep, ' ');

    while(p_ev->type != 0) {
      if(strstr(p_sep, p_ev->name) != NULL)
        type |= p_ev->type;
      ++p_ev;
    }
  }

  resp->transfer_encoding_type = type;
}

static void
_get_content_length(struct http_response_header *resp) {
  const char *p;

  if((p = http_header_get_value(resp, "content-length")) != NULL) {
    resp->content_length = strtol(p, NULL, 10);
  } else {
    resp->content_length = 0L;
  }
}


struct http_response_header *
http_header_get(struct net_client *client) {
  size_t size;
  char buf[_BUF_SIZE] = { '\0' };
  char *p_sep_st, *p_sep_ed;

  struct http_response_header *header = NULL;


  header = (struct http_response_header *)calloc(1, sizeof(struct http_response_header));
  if(header == NULL) return NULL;

  // 
  size = client->readline(client, buf, _BUF_SIZE);
  if(size < 0) goto error;

  // HTTP 応答ステータス
  // レスポンスステータス一例: HTTP/1.1 200 OK\r\n
  if((p_sep_st = strchr(buf, '/')) != NULL &&
      (p_sep_ed = strchr(p_sep_st + 1, ' ')) != NULL) {
    *p_sep_ed = '\0';

    // HTTP バージョン
    header->version = strtod(p_sep_st + 1, NULL);


    // 応答ステータス
    p_sep_st = p_sep_ed + 1;
    if((p_sep_ed = strchr(p_sep_st, ' ')) != NULL) {
      *p_sep_ed = '\0';

      header->status = (int)strtol(p_sep_st, NULL, 10);
    } else {
      header->status = -1;
    }
  }

  // ヘッダ取得
  struct http_field *hf_cursor = NULL;

  while(1) {
    char *p = NULL, *p_key = NULL, *p_value = NULL;

    memset((void *)buf, '\0', sizeof(buf));
    size = client->readline(client, buf, _BUF_SIZE);

    // エラーもしくはヘッダの終点
    if(size <= 0 || strcmp(buf, "\r\n") == 0) break;

    // 改行を無視
    if((p = strstr(buf, "\r\n")) != NULL)
      *p = '\0';

    p_key   = buf;
    p_value = strchr(buf, ':');

    // ヘッダの追加
    if(p_value != NULL) {
      // 小文字変換
      int i;
      for(i = 0; i < strlen(p_key); ++i)
        p_key[i] = (char)tolower(p_key[i]);

      *p_value = '\0';
      ++p_value;

      // 先頭の空白文字を除去
      skip_ch(p_value, ' ');


      if(header->fields == NULL) {
        header->fields = (struct http_field *)calloc(1, sizeof(struct http_field));
        hf_cursor = header->fields;
      } else {
        hf_cursor->next = (struct http_field *)calloc(1, sizeof(struct http_field));
        hf_cursor = hf_cursor->next;
      }

      // NOTE: + 1 空文字用
      hf_cursor->name = (char *)calloc(strlen(p_key) + 1, sizeof(char));
      strncpy(hf_cursor->name, p_key, strlen(p_key));

      hf_cursor->value = (char *)calloc(strlen(p_value) + 1, sizeof(char));
      strncpy(hf_cursor->value, p_value, strlen(p_value));
    }
  }

  // ヘッダの取得完了後, struct http_response_header のパラメータを設定
  _get_transfer_en(header);
  _get_content_length(header);

  return header;

error:
  if(header != NULL)
    free((void *)header);
  header = NULL;

  return NULL;
}


void
http_header_free(struct http_response_header *p_header) {
  struct http_field *p_hf = p_header->fields, *p_next = NULL;

  memset((void *)p_header, '\0', sizeof(struct http_response_header));
  free((void *)p_header);
  p_header = NULL;

  if(p_hf == NULL) return;

  while(p_hf != NULL) {
    p_next = p_hf->next;

    if(p_hf->name != NULL) {
      memset((void *)p_hf->name, '\0', strlen(p_hf->name));
      free((void *)p_hf->name);
      p_hf->name = NULL;
    }

    if(p_hf->value != NULL) {
      memset((void *)p_hf->value, '\0', strlen(p_hf->value));
      free((void *)p_hf->value);
      p_hf->value = NULL;
    }
    p_hf->next = NULL;

    free((void *)p_hf);
    p_hf = p_next;
  }
}

char *
http_header_get_value(const struct http_response_header *p_head, const char *name) {
  if(p_head == NULL || p_head->fields == NULL)
    return NULL;

  struct http_field *p_field = p_head->fields;

  for(; p_field != NULL; p_field = p_field->next) {
    if(strcmp(p_field->name, name) == 0)
      return p_field->value;
  }

  return NULL;
}


size_t
http_header_field_count(const struct http_response_header *p_header) {
  size_t count = 0;
  struct http_field *p;

  for(p = p_header->fields; p != NULL; p = p->next)
    ++count;

  return count;
}


void
http_header_print(const struct http_response_header *p_header) {
  struct http_field *p_field = NULL;

  fprintf(stderr, "* status:  %d\n", p_header->status);
  fprintf(stderr, "* version: %.1f\n", p_header->version);

  for(p_field = p_header->fields;
      p_field != NULL;
      p_field = p_field->next) {
    printf("** \033[1;32m%s\033[0m: %s\n", p_field->name, p_field->value);
  }
}


