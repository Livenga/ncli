#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../../include/network_type.h"

void
net_print_address(const char *domain, const struct sockaddr_in addr) {
  printf("[%s]: %s(%u)\n",
      domain, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}
