#ifndef QJSNET_LIB_GENERATOR_H
#define QJSNET_LIB_GENERATOR_H

#include "buffer.h"
#include "asynciterator.h"
#include "queue.h"

typedef struct generator {
  union {
    AsyncIterator iterator;
    struct {
      int ref_count;
      struct list_head reads;
      BOOL closed, closing;
    };
  };
  JSContext* ctx;
  Queue* q;
  JSValue executor, callback;
  uint64_t bytes_written, bytes_read;
  uint32_t chunks_written, chunks_read;
  JSValue (*block_fn)(ByteBlock*, JSContext*);
} Generator;

void generator_free(Generator*);
Generator* generator_new(JSContext*);
JSValue generator_dequeue(Generator*, BOOL* done_p);
JSValue generator_next(Generator*, JSValueConst arg);
ssize_t generator_write(Generator*, const void* data, size_t len, JSValueConst callback);
JSValue generator_push(Generator*, JSValueConst value);
BOOL generator_yield(Generator*, JSValueConst value, JSValueConst callback);
BOOL generator_stop(Generator*, JSValueConst callback);
BOOL generator_continuous(Generator*, JSValueConst callback);
Queue* generator_queue(Generator*);
BOOL generator_finish(Generator* gen);

static inline Generator*
generator_dup(Generator* gen) {
  ++gen->ref_count;
  return gen;
}

#endif /* QJSNET_LIB_GENERATOR_H */
