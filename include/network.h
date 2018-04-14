#ifndef _NETWORK_H
#define _NETWORK_H

#ifndef _NETWORK_TYPE_H
#include "network_type.h"
#endif


#define IS_CHUNKED(type)  (type & CHUNKED) > 0
#define IS_COMPRESS(type) (type & COMPRESS) > 0
#define IS_DEFLATE(type)  (type & DEFLATE) > 0
#define IS_GZIP(type)     (type & GZIP) > 0
#define IS_IDENTITY(type) (type & IDENTITY) > 0


/* src/network/init.c */
extern struct net_client *
create_net_client(const char *domain, const char *service);


/* src/network/header.c */
extern struct http_response_header*
http_header_get(struct net_client *client);
extern void
http_header_free(struct http_response_header *resp);
extern size_t
http_header_field_count(const struct http_response_header *resp);
extern char *
http_header_get_value(const struct http_response_header *p_head, const char *name);
extern void
http_header_print(const struct http_response_header *resp);

/* src/network/io.c */
extern struct http_response_content *
http_read_content(const struct net_client *client,
    const struct http_response_header *resp);
extern void
http_response_content_free(struct http_response_content *p);


/* src/network/util.c */
extern void
net_print_address(const char *domain, const struct sockaddr_in addr);
extern void
write_to_file(struct net_client *client, const char *output_path);

#endif
