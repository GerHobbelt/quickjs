#include "queue.h"
#include <assert.h>

void
queue_zero(Queue* q) {
  init_list_head(&q->items);
  q->size = 0;
  q->continuous = FALSE;
}

void
queue_clear(Queue* q, JSRuntime* rt) {
  struct list_head *p, *p2;

  list_for_each_safe(p, p2, &q->items) {
    QueueItem* i = list_entry(p, QueueItem, link);

    list_del(p);
    block_free(&i->block);
    // JS_FreeValueRT(rt, i->value);

    free(p);
  }

  q->size = 0;
}

void
queue_free(Queue* q, JSRuntime* rt) {
  queue_clear(q, rt);
  js_free_rt(rt, q);
}

Queue*
queue_new(JSContext* ctx) {
  Queue* q;

  if((q = js_malloc(ctx, sizeof(Queue))))
    queue_zero(q);

  return q;
}

QueueItem*
queue_front(Queue* q) {
  return list_empty(&q->items) ? 0 : list_entry(q->items.next, QueueItem, link);
}

QueueItem*
queue_back(Queue* q) {
  return list_empty(&q->items) ? 0 : list_entry(q->items.prev, QueueItem, link);
}

QueueItem*
queue_last_chunk(Queue* q) {
  struct list_head* el;

  list_for_each_prev(el, &q->items) {
    QueueItem* i = list_entry(el, QueueItem, link);

    if(block_SIZE(&i->block) || !i->done)
      return i;
  }

  return 0;
}

ByteBlock
queue_next(Queue* q, BOOL* done_p) {
  ByteBlock ret = {0, 0};
  QueueItem* i;
  BOOL done = FALSE;

  if((i = queue_front(q))) {
    ret = i->block;
    done = i->done;

    if(i->unref) {
      JSContext* ctx = deferred_getctx(i->unref);
      JSValue fn = deferred_getjs(i->unref);

      JS_FreeValue(ctx, JS_Call(ctx, fn, JS_UNDEFINED, 0, 0));

      deferred_call(i->unref);
      deferred_free(i->unref);
    }

    if(!done) {
      list_del(&i->link);

      --q->size;
      free(i);
    }
  }

  if(done_p)
    *done_p = done;

  return ret;
}

QueueItem*
queue_add(Queue* q, ByteBlock chunk) {
  QueueItem* i;

  if(queue_complete(q))
    return 0;

  if(q->items.next == 0 && q->items.prev == 0)
    init_list_head(&q->items);

  if((i = malloc(sizeof(QueueItem)))) {
    i->block = chunk;
    i->done = FALSE;
    i->unref = 0;

    list_add_tail(&i->link, &q->items);
    ++q->size;
  }

  return i;
}

QueueItem*
queue_put(Queue* q, ByteBlock chunk, JSContext* ctx) {
  QueueItem* i;

  if(q->continuous && (i = queue_last_chunk(q))) {
    if(!block_SIZE(&i->block)) {
      //      i->block = chunk;
      i->block = block_copy(chunk.start, block_SIZE(&chunk));
    } else {
      if(block_append(&i->block, block_BEGIN(&chunk), block_SIZE(&chunk)) == -1)
        i = 0;
    }
    block_free(&chunk);
  } else {
    i = queue_add(q, chunk);
  }
  return i;
}

QueueItem*
queue_write(Queue* q, const void* data, size_t size, JSContext* ctx) {
  QueueItem* i;

  if(q->continuous && (i = queue_last_chunk(q))) {
    if(block_append(&i->block, data, size) == -1)
      i = 0;
  } else {
    ByteBlock chunk = block_copy(data, size);

    i = queue_add(q, chunk);
  }

  return i;
}

QueueItem*
queue_close(Queue* q) {
  QueueItem* i;

  /* if(q->items.next == 0 && q->items.prev == 0)
     init_list_head(&q->items);
 */
  if(queue_complete(q))
    return queue_back(q);

  assert(!queue_closed(q));

  if((i = malloc(sizeof(QueueItem)))) {
    i->block = (ByteBlock){0, 0};
    i->done = TRUE;
    i->unref = 0;

    list_add_tail(&i->link, &q->items);
    ++q->size;
  }

  return i;
}

size_t
queue_bytes(Queue* q) {
  QueueItem* i;
  struct list_head* el;
  size_t bytes = 0;

  if(queue_size(q) == 0)
    return 0;

  list_for_each(el, &q->items) {
    i = list_entry(el, QueueItem, link);

    bytes += block_SIZE(&i->block);
  }

  return bytes;
}

QueueItem*
queue_continuous(Queue* q) {
  QueueItem* i;

  q->continuous = TRUE;

  if(!(i = queue_last_chunk(q))) {
    if((i = malloc(sizeof(QueueItem)))) {
      i->block = (ByteBlock){0, 0};
      i->done = FALSE;
      i->unref = 0;

      list_add_tail(&i->link, &q->items);
      ++q->size;
    }
  }

  return i;
}
