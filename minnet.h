#include "quickjs.h"
#include "cutils.h"
#include <libwebsockets.h>

#define countof(x) (sizeof(x) / sizeof((x)[0]))
#define JS_CGETSET_MAGIC_FLAGS_DEF(prop_name, fgetter, fsetter, magic_num, flags)                                              \
  {                                                                                                                            \
    .name = prop_name, .prop_flags = flags, .def_type = JS_DEF_CGETSET_MAGIC, .magic = magic_num, .u = {                       \
      .getset = {.get = {.getter_magic = fgetter}, .set = {.setter_magic = fsetter}}                                           \
    }                                                                                                                          \
  }

static JSValue minnet_log, minnet_log_this;
static JSContext* minnet_log_ctx;
static BOOL minnet_exception;

static JSValue
minnet_get_log(JSContext* ctx, JSValueConst this_val) {
  return JS_DupValue(ctx, minnet_log);
}

static JSValue
minnet_set_log(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst argv[]) {
  JSValue ret = minnet_log;

  minnet_log_ctx = ctx;
  minnet_log = JS_DupValue(ctx, argv[0]);
  if(argc > 1) {
    JS_FreeValue(ctx, minnet_log_this);
    minnet_log_this = JS_DupValue(ctx, argv[1]);
  }
  return ret;
}

typedef struct minnet_ws_callback {
  JSContext* ctx;
  JSValueConst* this_obj;
  JSValue* func_obj;
} minnet_ws_callback;

static struct minnet_ws_callback server_cb_message;
static struct minnet_ws_callback server_cb_connect;
static struct minnet_ws_callback server_cb_error;
static struct minnet_ws_callback server_cb_close;
static struct minnet_ws_callback server_cb_pong;
static struct minnet_ws_callback server_cb_fd;
static struct minnet_ws_callback server_cb_http;

static JSValue minnet_ws_server(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static struct minnet_ws_callback client_cb_message;
static struct minnet_ws_callback client_cb_connect;
static struct minnet_ws_callback client_cb_error;
static struct minnet_ws_callback client_cb_close;
static struct minnet_ws_callback client_cb_pong;
static struct minnet_ws_callback client_cb_fd;

static JSValue minnet_ws_client(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static JSValue minnet_fetch(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static const JSCFunctionListEntry minnet_funcs[] = {
    JS_CFUNC_DEF("server", 1, minnet_ws_server),
    JS_CFUNC_DEF("client", 1, minnet_ws_client),
    JS_CFUNC_DEF("fetch", 1, minnet_fetch),
    JS_CFUNC_DEF("setLog", 1, minnet_set_log),
};

static int
js_minnet_init(JSContext* ctx, JSModuleDef* m) {
  return JS_SetModuleExportList(ctx, m, minnet_funcs, countof(minnet_funcs));
}

/* class WebSocket */

struct http_header {
  unsigned char *start, *pos, *end;
};

typedef struct {
  struct lws* lwsi;
  size_t ref_count;
  struct http_header header;
} MinnetWebsocket;

static JSValue minnet_ws_send(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue minnet_ws_respond(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic);
static JSValue minnet_ws_ping(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue minnet_ws_pong(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue minnet_ws_close(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue minnet_ws_get(JSContext* ctx, JSValueConst this_val, int magic);

static void minnet_ws_finalizer(JSRuntime* rt, JSValue val);

static JSClassDef minnet_ws_class = {
    "MinnetWebSocket",
    .finalizer = minnet_ws_finalizer,
};

static const JSCFunctionListEntry minnet_ws_proto_funcs[] = {
    JS_CFUNC_DEF("send", 1, minnet_ws_send),
    JS_CFUNC_MAGIC_DEF("respond", 1, minnet_ws_respond, 0),
    JS_CFUNC_MAGIC_DEF("redirect", 2, minnet_ws_respond, 1),
    JS_CFUNC_MAGIC_DEF("header", 2, minnet_ws_respond, 2),
    JS_CFUNC_DEF("ping", 1, minnet_ws_ping),
    JS_CFUNC_DEF("pong", 1, minnet_ws_pong),
    JS_CFUNC_DEF("close", 1, minnet_ws_close),
    JS_CGETSET_MAGIC_FLAGS_DEF("fd", minnet_ws_get, 0, 0, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("address", minnet_ws_get, 0, 1, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("family", minnet_ws_get, 0, 2, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("port", minnet_ws_get, 0, 3, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("peer", minnet_ws_get, 0, 4, JS_PROP_ENUMERABLE),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "MinnetWebSocket", JS_PROP_CONFIGURABLE),
    JS_PROP_INT32_DEF("CLOSE_STATUS_NORMAL", LWS_CLOSE_STATUS_NORMAL, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_GOINGAWAY", LWS_CLOSE_STATUS_GOINGAWAY, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_PROTOCOL_ERR", LWS_CLOSE_STATUS_PROTOCOL_ERR, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_UNACCEPTABLE_OPCODE", LWS_CLOSE_STATUS_UNACCEPTABLE_OPCODE, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_RESERVED", LWS_CLOSE_STATUS_RESERVED, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_NO_STATUS", LWS_CLOSE_STATUS_NO_STATUS, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_ABNORMAL_CLOSE", LWS_CLOSE_STATUS_ABNORMAL_CLOSE, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_INVALID_PAYLOAD", LWS_CLOSE_STATUS_INVALID_PAYLOAD, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_POLICY_VIOLATION", LWS_CLOSE_STATUS_POLICY_VIOLATION, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_MESSAGE_TOO_LARGE", LWS_CLOSE_STATUS_MESSAGE_TOO_LARGE, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_EXTENSION_REQUIRED", LWS_CLOSE_STATUS_EXTENSION_REQUIRED, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_UNEXPECTED_CONDITION", LWS_CLOSE_STATUS_UNEXPECTED_CONDITION, 0),
    JS_PROP_INT32_DEF("CLOSE_STATUS_TLS_FAILURE", LWS_CLOSE_STATUS_TLS_FAILURE, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_CONTINUE", HTTP_STATUS_CONTINUE, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_OK", HTTP_STATUS_OK, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_NO_CONTENT", HTTP_STATUS_NO_CONTENT, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_PARTIAL_CONTENT", HTTP_STATUS_PARTIAL_CONTENT, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_MOVED_PERMANENTLY", HTTP_STATUS_MOVED_PERMANENTLY, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_FOUND", HTTP_STATUS_FOUND, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_SEE_OTHER", HTTP_STATUS_SEE_OTHER, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_NOT_MODIFIED", HTTP_STATUS_NOT_MODIFIED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_BAD_REQUEST", HTTP_STATUS_BAD_REQUEST, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_UNAUTHORIZED", HTTP_STATUS_UNAUTHORIZED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_PAYMENT_REQUIRED", HTTP_STATUS_PAYMENT_REQUIRED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_FORBIDDEN", HTTP_STATUS_FORBIDDEN, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_NOT_FOUND", HTTP_STATUS_NOT_FOUND, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_METHOD_NOT_ALLOWED", HTTP_STATUS_METHOD_NOT_ALLOWED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_NOT_ACCEPTABLE", HTTP_STATUS_NOT_ACCEPTABLE, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_PROXY_AUTH_REQUIRED", HTTP_STATUS_PROXY_AUTH_REQUIRED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_REQUEST_TIMEOUT", HTTP_STATUS_REQUEST_TIMEOUT, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_CONFLICT", HTTP_STATUS_CONFLICT, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_GONE", HTTP_STATUS_GONE, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_LENGTH_REQUIRED", HTTP_STATUS_LENGTH_REQUIRED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_PRECONDITION_FAILED", HTTP_STATUS_PRECONDITION_FAILED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_REQ_ENTITY_TOO_LARGE", HTTP_STATUS_REQ_ENTITY_TOO_LARGE, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_REQ_URI_TOO_LONG", HTTP_STATUS_REQ_URI_TOO_LONG, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE", HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_REQ_RANGE_NOT_SATISFIABLE", HTTP_STATUS_REQ_RANGE_NOT_SATISFIABLE, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_EXPECTATION_FAILED", HTTP_STATUS_EXPECTATION_FAILED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_INTERNAL_SERVER_ERROR", HTTP_STATUS_INTERNAL_SERVER_ERROR, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_NOT_IMPLEMENTED", HTTP_STATUS_NOT_IMPLEMENTED, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_BAD_GATEWAY", HTTP_STATUS_BAD_GATEWAY, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_SERVICE_UNAVAILABLE", HTTP_STATUS_SERVICE_UNAVAILABLE, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_GATEWAY_TIMEOUT", HTTP_STATUS_GATEWAY_TIMEOUT, 0),
    JS_PROP_INT32_DEF("HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED", HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED, 0),
};

JSClassID minnet_ws_class_id;

static JSValue create_websocket_obj(JSContext* ctx, struct lws* wsi);

/* class Response */

typedef struct {
  uint8_t* buffer;
  long size;
  JSValue status;
  JSValue ok;
  JSValue url;
  JSValue type;
} MinnetResponse;

static JSValue minnet_response_buffer(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue minnet_response_json(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue minnet_response_text(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue minnet_response_getter_ok(JSContext* ctx, JSValueConst this_val);
static JSValue minnet_response_getter_url(JSContext* ctx, JSValueConst this_val);
static JSValue minnet_response_getter_status(JSContext* ctx, JSValueConst this_val);
static JSValue minnet_response_getter_type(JSContext* ctx, JSValueConst this_val);
static void minnet_response_finalizer(JSRuntime* rt, JSValue val);

static JSClassDef minnet_response_class = {
    "MinnetResponse",
    .finalizer = minnet_response_finalizer,
};

static const JSCFunctionListEntry minnet_response_proto_funcs[] = {
    JS_CFUNC_DEF("arrayBuffer", 0, minnet_response_buffer),
    JS_CFUNC_DEF("json", 0, minnet_response_json),
    JS_CFUNC_DEF("text", 0, minnet_response_text),
    JS_CGETSET_DEF("ok", minnet_response_getter_ok, NULL),
    JS_CGETSET_DEF("url", minnet_response_getter_url, NULL),
    JS_CGETSET_DEF("status", minnet_response_getter_status, NULL),
    JS_CGETSET_DEF("type", minnet_response_getter_type, NULL),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "MinnetResponse", JS_PROP_CONFIGURABLE),
};

JSClassID minnet_response_class_id;
