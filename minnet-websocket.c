#include "minnet.h"
#include "minnet-websocket.h"
#include "minnet-server.h"

JSClassID minnet_ws_class_id;

static JSValue
create_websocket_obj(JSContext* ctx, struct lws* wsi) {
  MinnetWebsocket* res;
  JSValue ws_obj = JS_NewObjectClass(ctx, minnet_ws_class_id);

  if(JS_IsException(ws_obj))
    return JS_EXCEPTION;

  if(!(res = js_mallocz(ctx, sizeof(*res)))) {
    JS_FreeValue(ctx, ws_obj);
    return JS_EXCEPTION;
  }

  res->lwsi = wsi;
  res->ref_count = 1;

  JS_SetOpaque(ws_obj, res);

  lws_set_wsi_user(wsi, JS_VALUE_GET_OBJ(JS_DupValue(ctx, ws_obj)));

  return ws_obj;
}

JSValue
minnet_ws_object(JSContext* ctx, struct lws* wsi) {
  JSObject* obj;

  if((obj = lws_wsi_user(wsi))) {
    JSValue ws_obj = JS_MKPTR(JS_TAG_OBJECT, obj);
    MinnetWebsocket* res = JS_GetOpaque2(ctx, ws_obj, minnet_ws_class_id);

    res->ref_count++;

    return JS_DupValue(ctx, ws_obj);
  }

  return create_websocket_obj(ctx, wsi);
}

JSValue
minnet_ws_emit(struct callback_ws* cb, int argc, JSValue* argv) {
  if(!cb->func_obj)
    return JS_UNDEFINED;
  return JS_Call(cb->ctx, *cb->func_obj, cb->this_obj ? *cb->this_obj : JS_NULL, argc, argv);
}

void
minnet_ws_sslcert(JSContext* ctx, struct lws_context_creation_info* info, JSValueConst options) {
  JSValue opt_ssl_cert = JS_GetPropertyStr(ctx, options, "sslCert");
  JSValue opt_ssl_private_key = JS_GetPropertyStr(ctx, options, "sslPrivateKey");
  JSValue opt_ssl_ca = JS_GetPropertyStr(ctx, options, "sslCA");

  if(JS_IsString(opt_ssl_cert))
    info->ssl_cert_filepath = JS_ToCString(ctx, opt_ssl_cert);
  if(JS_IsString(opt_ssl_private_key))
    info->ssl_private_key_filepath = JS_ToCString(ctx, opt_ssl_private_key);
  if(JS_IsString(opt_ssl_ca))
    info->client_ssl_ca_filepath = JS_ToCString(ctx, opt_ssl_ca);
}

static JSValue
minnet_ws_send(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  MinnetWebsocket* ws_obj;
  const char* msg;
  uint8_t* data;
  int  m, n;
  size_t len;

  if(!(ws_obj = JS_GetOpaque2(ctx, this_val, minnet_ws_class_id)))
    return JS_EXCEPTION;

  if(JS_IsString(argv[0])) {
    msg = JS_ToCString(ctx, argv[0]);
    len = strlen(msg);
    uint8_t buffer[LWS_PRE + len];

    n = lws_snprintf((char*)&buffer[LWS_PRE], len + 1, "%s", msg);
    m = lws_write(ws_obj->lwsi, &buffer[LWS_PRE], len, LWS_WRITE_TEXT);
    if(m < n) {
      // Sending message failed
      return JS_EXCEPTION;
    }
    return JS_UNDEFINED;
  }

  data = JS_GetArrayBuffer(ctx, &len, argv[0]);
  if(data) {
    uint8_t buffer[LWS_PRE + len];
    memcpy(&buffer[LWS_PRE], data, len);

    m = lws_write(ws_obj->lwsi, &buffer[LWS_PRE], len, LWS_WRITE_BINARY);
    if((size_t)m < len) {
      // Sending data failed
      return JS_EXCEPTION;
    }
  }
  return JS_UNDEFINED;
}

enum { RESPONSE_BODY, RESPONSE_HEADER, RESPONSE_REDIRECT };

static JSValue
minnet_ws_respond(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic) {
  MinnetWebsocket* ws_obj;
  JSValue ret = JS_UNDEFINED;

  if(!(ws_obj = JS_GetOpaque2(ctx, this_val, minnet_ws_class_id)))
    return JS_EXCEPTION;
  MinnetBuffer header = {0, 0, 0};

  switch(magic) {
    case RESPONSE_BODY: {
      const char* msg = 0;
      uint32_t status = 0;

      JS_ToUint32(ctx, &status, argv[0]);

      if(argc >= 2)
        msg = JS_ToCString(ctx, argv[1]);

      lws_return_http_status(ws_obj->lwsi, status, msg);
      if(msg)
        JS_FreeCString(ctx, msg);
      break;
    }
    case RESPONSE_REDIRECT: {

      const char* msg = 0;
      size_t len = 0;
      uint32_t status = 0;

      JS_ToUint32(ctx, &status, argv[0]);

      if(argc >= 2)
        msg = JS_ToCStringLen(ctx, &len, argv[1]);

      if(lws_http_redirect(ws_obj->lwsi, status, (unsigned char*)msg, len, &header.pos, header.end) < 0)
        ret = JS_NewInt32(ctx, -1);
      if(msg)
        JS_FreeCString(ctx, msg);
      break;
    }
    case RESPONSE_HEADER: {

      size_t namelen;
      const char* namestr = JS_ToCStringLen(ctx, &namelen, argv[0]);
      char* name = js_malloc(ctx, namelen + 2);
      size_t len;
      const char* value = JS_ToCStringLen(ctx, &len, argv[1]);

      memcpy(name, namestr, namelen);
      name[namelen] = ':';
      name[namelen + 1] = '\0';

      if(lws_add_http_header_by_name(ws_obj->lwsi, (const uint8_t*)name,  (const uint8_t*)value, len, &header.pos, header.end) < 0)
        ret = JS_NewInt32(ctx, -1);

      js_free(ctx, name);
      JS_FreeCString(ctx, namestr);
      JS_FreeCString(ctx, value);
      break;
    }
  }

  return ret;
}

static JSValue
minnet_ws_ping(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  MinnetWebsocket* ws_obj;
  uint8_t* data;
  size_t len;

  if(!(ws_obj = JS_GetOpaque2(ctx, this_val, minnet_ws_class_id)))
    return JS_EXCEPTION;

  data = JS_GetArrayBuffer(ctx, &len, argv[0]);
  if(data) {
    uint8_t buffer[len + LWS_PRE];
    memcpy(&buffer[LWS_PRE], data, len);

    int m = lws_write(ws_obj->lwsi, &buffer[LWS_PRE], len, LWS_WRITE_PING);
    if((size_t)m < len) {
      // Sending ping failed
      return JS_EXCEPTION;
    }
  } else {
    uint8_t buffer[LWS_PRE];
    lws_write(ws_obj->lwsi, &buffer[LWS_PRE], 0, LWS_WRITE_PING);
  }
  return JS_UNDEFINED;
}

static JSValue
minnet_ws_pong(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  MinnetWebsocket* ws_obj;
  uint8_t* data;
  size_t len;

  if(!(ws_obj = JS_GetOpaque2(ctx, this_val, minnet_ws_class_id)))
    return JS_EXCEPTION;

  data = JS_GetArrayBuffer(ctx, &len, argv[0]);
  if(data) {
    uint8_t buffer[len + LWS_PRE];
    memcpy(&buffer[LWS_PRE], data, len);

    int m = lws_write(ws_obj->lwsi, &buffer[LWS_PRE], len, LWS_WRITE_PONG);
    if((size_t)m < len) {
      // Sending pong failed
      return JS_EXCEPTION;
    }
  } else {
    uint8_t buffer[LWS_PRE];
    lws_write(ws_obj->lwsi, &buffer[LWS_PRE], 0, LWS_WRITE_PONG);
  }
  return JS_UNDEFINED;
}

static JSValue
minnet_ws_close(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  MinnetWebsocket* ws_obj;
  const char* reason = 0;
  size_t rlen = 0;

  if(!(ws_obj = JS_GetOpaque2(ctx, this_val, minnet_ws_class_id)))
    return JS_EXCEPTION;

  if(ws_obj->lwsi) {
    int optind = 0;
    int32_t status = LWS_CLOSE_STATUS_NORMAL;

    if(optind < argc && JS_IsNumber(argv[optind]))
      JS_ToInt32(ctx, &status, argv[optind++]);

    if(optind < argc) {
      reason = JS_ToCStringLen(ctx, &rlen, argv[optind++]);
      if(rlen > 124)
        rlen = 124;
    }

    if(reason)
      lws_close_reason(ws_obj->lwsi, status, (uint8_t*)reason, rlen);

    lws_close_free_wsi(ws_obj->lwsi, status, "minnet_ws_close");

    ws_obj->lwsi = 0;
    return JS_TRUE;
  }

  return JS_FALSE;
}

static JSValue
minnet_ws_get(JSContext* ctx, JSValueConst this_val, int magic) {
  MinnetWebsocket* ws_obj;
  JSValue ret = JS_UNDEFINED;
  if(!(ws_obj = JS_GetOpaque2(ctx, this_val, minnet_ws_class_id)))
    return JS_EXCEPTION;

  switch(magic) {
    case 0: {
      ret = JS_NewInt32(ctx, lws_get_socket_fd(ws_obj->lwsi));
      break;
    }
    case 1: {
      char address[1024];
      lws_get_peer_simple(ws_obj->lwsi, address, sizeof(address));

      ret = JS_NewString(ctx, address);
      break;
    }
    case 2:
    case 3: {
      struct sockaddr_in addr;
      socklen_t addrlen = sizeof(addr);
      int fd = lws_get_socket_fd(ws_obj->lwsi);

      if(getpeername(fd, (struct sockaddr*)&addr, &addrlen) != -1) {
        ret = JS_NewInt32(ctx, magic == 2 ? addr.sin_family : addr.sin_port);
      }
      break;
    }
    case 4: {
      struct sockaddr_in addr;
      socklen_t addrlen = sizeof(addr);
      int fd = lws_get_socket_fd(ws_obj->lwsi);

      if(getpeername(fd, (struct sockaddr*)&addr, &addrlen) != -1) {
        ret = JS_NewArrayBufferCopy(ctx, (const uint8_t*)&addr, addrlen);
      }
      break;
    }
  }
  return ret;
}

static void
minnet_ws_finalizer(JSRuntime* rt, JSValue val) {
  MinnetWebsocket* ws_obj = JS_GetOpaque(val, minnet_ws_class_id);
  if(ws_obj) {
    if(--ws_obj->ref_count == 0)
      js_free_rt(rt, ws_obj);
  }
}

JSClassDef minnet_ws_class = {
    "MinnetWebSocket",
    .finalizer = minnet_ws_finalizer,
};

const JSCFunctionListEntry minnet_ws_proto_funcs[] = {
    JS_CFUNC_DEF("send", 1, minnet_ws_send),
    JS_CFUNC_MAGIC_DEF("respond", 1, minnet_ws_respond, RESPONSE_BODY),
    JS_CFUNC_MAGIC_DEF("redirect", 2, minnet_ws_respond, RESPONSE_REDIRECT),
    JS_CFUNC_MAGIC_DEF("header", 2, minnet_ws_respond, RESPONSE_HEADER),
    JS_CFUNC_DEF("ping", 1, minnet_ws_ping),
    JS_CFUNC_DEF("pong", 1, minnet_ws_pong),
    JS_CFUNC_DEF("close", 1, minnet_ws_close),
    JS_CGETSET_MAGIC_FLAGS_DEF("fd", minnet_ws_get, 0, 0, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("address", minnet_ws_get, 0, 1, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("family", minnet_ws_get, 0, 2, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("port", minnet_ws_get, 0, 3, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("peer", minnet_ws_get, 0, 4, 0),
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

const size_t minnet_ws_proto_funcs_size = countof(minnet_ws_proto_funcs);
