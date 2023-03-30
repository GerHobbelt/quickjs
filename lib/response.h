#ifndef QJSNET_LIB_RESPONSE_H
#define QJSNET_LIB_RESPONSE_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include "url.h"
#include "buffer.h"
#include "generator.h"

struct session_data;

typedef struct http_response {
  int ref_count;
  bool read_only : 1, headers_sent : 1, compress : 1;
  URL url;
  char* type;
  int status;
  char* status_text;
  ByteBuffer headers;
  Generator* generator;
} Response;

void response_init(Response*, URL url, int32_t status, char* status_text, BOOL headers_sent, char* type);
Response* response_dup(Response*);
void response_clear(Response*, JSRuntime* rt);
void response_free(Response*, JSRuntime* rt);
Response* response_new(JSContext*);

static inline Generator*
response_generator(struct http_response* resp, JSContext* ctx) {
  if(!resp->generator)
    resp->generator = generator_new(ctx);
  return resp->generator;
}

struct http_response* response_new(JSContext*);

#endif /* QJSNET_LIB_RESPONSE_H */
