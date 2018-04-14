#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <getopt.h>

#include "../include/debug.h"
#include "../include/util.h"
#include "../include/network.h"

#define option_argument(index) \
  (optarg != NULL) ? optarg : argv[index]

int f_verbose = 0;


int
main(int argc, char *argv[]) {
  const struct option longopts[] = {
    {"domain",  required_argument, 0, 'd'},
    {"service", required_argument, 0, 's'},
    {"path",    required_argument, 0, 'p'},
    {"output",  optional_argument, 0, 'o'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0},
  };
  int opt, longindex;
  int f_output = 0;
  char *p_out_path;

  char domain[512] = { '\0' }, service[32] = { '\0' },
       path[1024] = { '\0' }, output[1024] = { '\0' };

  struct net_client *client;


  // サービス, パスの初期値を設定
  sprintf(service, "https");
  sprintf(path, "/");

  while((opt = getopt_long(argc, argv, "d:s:p:vo",
          longopts, &longindex)) > 0) {
    switch(opt) {
      case 'd': // ドメイン名
        strncpy(domain, option_argument(optind), sizeof(domain));
        break;

      case 's': // サービス名
        strncpy(service, option_argument(optind), sizeof(service));
        break;

      case 'p': // パス
        strncpy(path, option_argument(optind), sizeof(path));
        break;

      case 'o': // 出力先
        f_output    = 1;
        p_out_path = option_argument(optind);
        if(p_out_path != NULL && p_out_path[0] != '-') {
          strncpy(output, p_out_path, sizeof(output));
        } else {
          sprintf(output, "%s_output.txt", str_datetime());
        }
        break;

      case 'v': // 冗長
        f_verbose = 1;
        break;
    }
  }

  if(f_verbose) {
    fprintf(stderr, "* アクセス先: %s/%s:%s\n", domain, path, service);

    if(f_output)
      fprintf(stderr, "* ファイル出力先: %s\n", output);
  }

  client = create_net_client(domain, service);
  if(client == NULL) return EOF;

  net_print_address(domain, client->addr);

  struct http_response_header *resp;
  char buf[1024] = { '\0' };
  sprintf(buf,
      "GET %s HTTP/1.1\r\n"
      "Host: %s\r\n"
      "User-Agent: %s\r\n"
      "Accept: */*\r\n"
      //"Connection: close\r\n"
      "\r\n",
      path, domain, UA_GOOGLE_CHROME);

  client->write(client, buf, strlen(buf));
  resp = http_header_get(client);

  if(resp != NULL) {
    struct http_response_content *p_resp_cont;
    if(f_verbose) {
      http_header_print(resp);
    }

    p_resp_cont = http_read_content(client, resp);
    if(f_output) {
      write_content(output, 0, p_resp_cont->content);
    } else {
      write(1, (const void *)p_resp_cont->content, p_resp_cont->size);
    }

    http_response_content_free(p_resp_cont);
    http_header_free(resp);
    resp = NULL;
  }


  client->close(client);

  return 0;
}
