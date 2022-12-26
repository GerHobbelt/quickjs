#include "session.h"
#include "opaque.h"
#include "ringbuffer.h"
#include "jsutils.h"
#include "ws.h"
#include "context.h"
#include "lws-utils.h"
#include <assert.h>

struct socket* minnet_ws_data(JSValueConst);
struct http_request* minnet_request_data(JSValueConst);
struct http_response* minnet_response_data(JSValueConst);

struct http_response;
extern struct http_response* minnet_response_data(JSValueConst);

/*static THREAD_LOCAL uint32_t session_serial = 0;
THREAD_LOCAL struct list_head session_list = {0, 0};*/

void
session_zero(struct session_data* session) {
  session->ws_obj = JS_NULL;
  session->req_obj = JS_NULL;
  session->resp_obj = JS_NULL;
  session->mount = 0;
  session->proxy = 0;
  session->generator = JS_NULL;
  session->next = JS_NULL;
  session->in_body = FALSE;
  session->response_sent = FALSE;
  session->wait_resolve = FALSE;
  session->server = NULL;
  session->client = NULL;

  queue_zero(&session->sendq);
}

void
session_clear(struct session_data* session, JSContext* ctx) {
  session_clear_rt(session, JS_GetRuntime(ctx));
}

void
session_clear_rt(struct session_data* session, JSRuntime* rt) {

  JS_FreeValueRT(rt, session->ws_obj);
  session->ws_obj = JS_UNDEFINED;

  JS_FreeValueRT(rt, session->req_obj);
  session->req_obj = JS_UNDEFINED;
  JS_FreeValueRT(rt, session->resp_obj);
  session->resp_obj = JS_UNDEFINED;
  JS_FreeValueRT(rt, session->generator);
  session->generator = JS_UNDEFINED;
  JS_FreeValueRT(rt, session->next);
  session->next = JS_UNDEFINED;

  if(queue_size(&session->sendq))
    queue_clear_rt(&session->sendq, rt);
}

JSValue
session_object(struct session_data* session, JSContext* ctx) {
  JSValue ret;
  ret = JS_NewArray(ctx);

  JS_SetPropertyUint32(ctx, ret, 1, JS_DupValue(ctx, session->ws_obj));
  JS_SetPropertyUint32(ctx, ret, 2, JS_DupValue(ctx, session->req_obj));
  JS_SetPropertyUint32(ctx, ret, 3, JS_DupValue(ctx, session->resp_obj));

  return ret;
}

int
session_writable(struct session_data* session, BOOL binary, JSContext* ctx) {

  int ret = 0;
  size_t size;
  struct socket* ws = minnet_ws_data(session->ws_obj);

  if((size = queue_size(&session->sendq)) > 0) {
    ByteBlock chunk;
    BOOL done = FALSE;

    chunk = queue_next(&session->sendq, &done);

    ret = lws_write(ws->lwsi, block_BEGIN(&chunk), block_SIZE(&chunk), binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT);

    block_free(&chunk);
  }

  if(queue_size(&session->sendq) > 0)
    lws_callback_on_writable(ws->lwsi);

  return ret;
}

int
session_callback(struct session_data* session, JSCallback* cb, struct context* context) {
  int ret = 0;
  JSValue result = callback_emit_this(cb, session->ws_obj, 2, &session->req_obj);
  context_exception(context, result);

  if(JS_IsException(result)) {
    JS_FreeValue(cb->ctx, result);
    ret = -1;
  } else if(js_is_iterator(cb->ctx, result)) {
    assert(js_is_iterator(cb->ctx, result));
    session->generator = result;
    session->next = JS_UNDEFINED;
  } else {
    JS_FreeValue(cb->ctx, result);
  }

  return ret;
}

struct wsi_opaque_user_data*
session_opaque(struct session_data* sess) {
  struct socket* ws;

  if((ws = session_ws(sess))) {
    assert(ws->lwsi);
    return lws_get_opaque_user_data(ws->lwsi);
  }

  return 0;
}

struct context*
session_context(struct session_data* sess) {
  struct lws* wsi;

  if((wsi = session_wsi(sess)))
    return wsi_context(wsi);

  return 0;
}

struct http_request*
session_request(struct session_data* sess) {
  if(JS_IsObject(sess->req_obj))
    return minnet_request_data(sess->req_obj);
  return 0;
}

struct http_response*
session_response(struct session_data* sess) {
  if(JS_IsObject(sess->resp_obj))
    return minnet_response_data(sess->resp_obj);
  return 0;
}
