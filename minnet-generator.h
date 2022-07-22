#ifndef MINNET_GENERATOR_H
#define MINNET_GENERATOR_H

#include "minnet.h"
#include "jsutils.h"
#include <libwebsockets.h>
#include <pthread.h>
#include "minnet-buffer.h"

typedef struct generator {
  MinnetBuffer buffer;
  union {
    AsyncIterator iterator;
    JSContext* ctx;
  };
  uint64_t bytes_written, bytes_read;
  int ref_count;
} MinnetGenerator;

void generator_zero(struct generator*);
void generator_destroy(struct generator**);
BOOL generator_free(struct generator*);
struct generator* generator_new(JSContext*);
JSValue generator_next(MinnetGenerator*, JSContext*);
ssize_t generator_queue(MinnetGenerator*, const void*, size_t);
ssize_t generator_write(MinnetGenerator*, const void*, size_t);
BOOL generator_close(MinnetGenerator*, JSContext* ctx);
JSValue minnet_generator_create(JSContext*, MinnetGenerator**);

static inline struct generator*
generator_dup(struct generator* gen) {
  ++gen->ref_count;
  return gen;
}

#endif /* MINNET_GENERATOR_H */
