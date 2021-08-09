#include "minnet.h"
#include "minnet-request.h"
#include "minnet-stream.h"
#include "jsutils.h"
#include <cutils.h>
#include <ctype.h>

JSClassID minnet_request_class_id;
JSValue minnet_request_proto, minnet_request_ctor;

enum { REQUEST_TYPE, REQUEST_METHOD, REQUEST_URI, REQUEST_PATH, REQUEST_HEADER, REQUEST_ARRAYBUFFER, REQUEST_TEXT, REQUEST_BODY };

static const char* const method_names[] = {"GET", "POST", "OPTIONS", "PUT", "PATCH", "DELETE", "CONNECT", "HEAD"};

static const char*
method2name(enum http_method m) {
  if(m >= 0 && m < countof(method_names))
    return method_names[m];
  return 0;
}

static enum http_method
name2method(const char* name) {
  unsigned long i;
  for(i = 0; i < countof(method_names); i++) {
    if(!strcasecmp(name, method_names[i]))
      return i;
  }
  return -1;
}

void
request_dump(struct http_request const* req) {
  printf("\nMinnetRequest {\n\turi = %s", req->url);
  printf("\n\tpath = %s", req->path);
  printf("\n\ttype = %s", method_name(req->method));

  buffer_dump("header", &req->header);
  fputs("\n\tresponse = ", stdout);
  fputs(" }", stdout);
  fflush(stdout);
}

void
request_init(struct http_request* req, const char* path, char* url, char* method) {
  memset(req, 0, sizeof(*req));

  req->ref_count = 0;

  pstrcpy(req->path, sizeof(req->path), path);

  req->url = url;
  // req->type = method;
}

struct http_request*
request_new(JSContext* ctx) {
  MinnetRequest* req;

  if((req = js_mallocz(ctx, sizeof(MinnetRequest)))) {
    req->ref_count = 1;
  }
  return req;
}

void
request_zero(struct http_request* req) {
  memset(req, 0, sizeof(MinnetRequest));
  req->header = BUFFER_0();
  req->body = BUFFER_0();
}

static const char*
header_get(JSContext* ctx, size_t* lenp, struct byte_buffer* buf, const char* name) {
  size_t len, namelen = strlen(name);
  uint8_t *x, *end;

  for(x = buf->start, end = buf->write; x < end; x += len + 1) {
    len = byte_chr(x, end - x, '\n');

    /*   if(namelen >= len)
         continue;*/
    if(byte_chr(x, len, ':') != namelen || strncasecmp(name, (const char*)x, namelen))
      continue;

    if(x[namelen] == ':')
      namelen++;
    if(isspace(x[namelen]))
      namelen++;

    if(lenp)
      *lenp = len - namelen;
    return (const char*)x + namelen;
  }
  return 0;
}

JSValue
minnet_request_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  JSValue proto, obj;
  MinnetRequest* req;

  if(!(req = js_mallocz(ctx, sizeof(MinnetRequest))))
    return JS_ThrowOutOfMemory(ctx);

  /* using new_target to get the prototype is necessary when the
     class is extended. */
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    proto = JS_DupValue(ctx, minnet_request_proto);

  obj = JS_NewObjectProtoClass(ctx, proto, minnet_request_class_id);
  JS_FreeValue(ctx, proto);
  if(JS_IsException(obj))
    goto fail;

  if(argc >= 1) {
    if(JS_IsString(argv[0])) {
      const char* str = JS_ToCString(ctx, argv[0]);
      req->url = js_strdup(ctx, str);
      JS_FreeCString(ctx, str);
    }
    argc--;
    argv++;
  }

  if(argc >= 1) {
    if(JS_IsObject(argv[0]) && !JS_IsNull(argv[0])) {
      js_copy_properties(ctx, obj, argv[0], JS_GPN_STRING_MASK);
      argc--;
      argv++;
    }
  }

  req->read_only = TRUE;

  JS_SetOpaque(obj, req);

  return obj;

fail:
  js_free(ctx, req);
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

JSValue
minnet_request_new(JSContext* ctx, const char* path, const char* url, enum http_method method) {
  struct http_request* req;

  if(!(req = request_new(ctx)))
    return JS_ThrowOutOfMemory(ctx);

  request_init(req, path, js_strdup(ctx, url), js_strdup(ctx, method == METHOD_POST ? "POST" : "GET"));

  req->method = method;

  return minnet_request_wrap(ctx, req);
}

JSValue
minnet_request_wrap(JSContext* ctx, struct http_request* req) {
  JSValue ret = JS_NewObjectProtoClass(ctx, minnet_request_proto, minnet_request_class_id);

  if(JS_IsException(ret))
    return JS_EXCEPTION;

  JS_SetOpaque(ret, req);

  ++req->ref_count;

  return ret;
}

static JSValue
minnet_request_get(JSContext* ctx, JSValueConst this_val, int magic) {
  MinnetRequest* req;
  if(!(req = JS_GetOpaque2(ctx, this_val, minnet_request_class_id)))
    return JS_EXCEPTION;

  JSValue ret = JS_UNDEFINED;
  switch(magic) {
    case REQUEST_METHOD: {
      ret = JS_NewString(ctx, method_name(req->method));
      break;
    }
    case REQUEST_TYPE: {
      ret = JS_NewInt32(ctx, req->method);
      break;
    }
    case REQUEST_URI: {
      if(req->url)
        ret = JS_NewString(ctx, req->url);
      break;
    }
    case REQUEST_PATH: {
      ret = JS_NewString(ctx, req->path);
      break;
    }
    case REQUEST_HEADER: {
      size_t len, namelen;
      uint8_t *x, *end;
      ret = JS_NewObject(ctx);

      for(x = req->header.start, end = req->header.write; x < end; x += len + 1) {
        len = byte_chr(x, end - x, '\n');
        namelen = byte_chr(x, len, ':');

        if(namelen >= len)
          continue;

        const char* prop = js_strndup(ctx, (const char*)x, namelen);

        if(x[namelen] == ':')
          namelen++;
        if(isspace(x[namelen]))
          namelen++;

        JS_SetPropertyStr(ctx, ret, prop, JS_NewStringLen(ctx, (const char*)&x[namelen], len - namelen));
        js_free(ctx, (void*)prop);
      }

      //      ret = buffer_tostring(&req->header, ctx);
      break;
    }
    case REQUEST_ARRAYBUFFER: {
      ret = buffer_OFFSET(&req->body) ? buffer_toarraybuffer(&req->body, ctx) : JS_NULL;
      break;
    }
    case REQUEST_TEXT: {
      ret = buffer_OFFSET(&req->body) ? buffer_tostring(&req->body, ctx) : JS_NULL;
      break;
    }
    case REQUEST_BODY: {
      if(buffer_OFFSET(&req->body)) {
        size_t typelen;
        const char* type = header_get(ctx, &typelen, &req->header, "content-type");

        ret = minnet_stream_new(ctx, type, typelen, buffer_START(&req->body), buffer_OFFSET(&req->body));
      } else {
        ret = JS_NULL;
      }
      break;
    }
  }
  return ret;
}

static JSValue
minnet_request_set(JSContext* ctx, JSValueConst this_val, JSValueConst value, int magic) {
  MinnetRequest* req;
  JSValue ret = JS_UNDEFINED;
  const char* str;
  size_t len;
  if(!(req = JS_GetOpaque2(ctx, this_val, minnet_request_class_id)))
    return JS_EXCEPTION;

  if(req->read_only)
    return JS_ThrowReferenceError(ctx, "Request object is read-only");

  str = JS_ToCStringLen(ctx, &len, value);

  switch(magic) {
    case REQUEST_METHOD:
    case REQUEST_TYPE: {
      int32_t m = -1;
      if(JS_IsNumber(value))
        JS_ToInt32(ctx, &m, value);
      else
        m = name2method(str);
      if(m >= 0 && method2name(m))
        req->method = m;
      break;
    }
    case REQUEST_URI: {
      if(req->url) {
        js_free(ctx, req->url);
        req->url = 0;
      }
      req->url = js_strdup(ctx, str);
      break;
    }
    case REQUEST_PATH: {
      pstrcpy(req->path, sizeof(req->path), str);
      break;
    }
    case REQUEST_HEADER: {
      ret = JS_ThrowReferenceError(ctx, "Cannot set headers");
      break;
    }
  }

  JS_FreeCString(ctx, str);

  return ret;
}

static void
minnet_request_finalizer(JSRuntime* rt, JSValue val) {
  MinnetRequest* req = JS_GetOpaque(val, minnet_request_class_id);
  if(req && --req->ref_count == 0) {
    if(req->url)
      js_free_rt(rt, req->url);

    js_free_rt(rt, req);
  }
}

JSClassDef minnet_request_class = {
    "MinnetRequest",
    .finalizer = minnet_request_finalizer,
};

const JSCFunctionListEntry minnet_request_proto_funcs[] = {
    JS_CGETSET_MAGIC_FLAGS_DEF("type", minnet_request_get, minnet_request_set, REQUEST_TYPE, 0),
    JS_CGETSET_MAGIC_FLAGS_DEF("method", minnet_request_get, minnet_request_set, REQUEST_METHOD, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("url", minnet_request_get, minnet_request_set, REQUEST_URI, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("path", minnet_request_get, minnet_request_set, REQUEST_PATH, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("headers", minnet_request_get, 0, REQUEST_HEADER, JS_PROP_ENUMERABLE),
    JS_CGETSET_MAGIC_FLAGS_DEF("arrayBuffer", minnet_request_get, 0, REQUEST_ARRAYBUFFER, 0),
    JS_CGETSET_MAGIC_FLAGS_DEF("text", minnet_request_get, 0, REQUEST_TEXT, 0),
    JS_CGETSET_MAGIC_FLAGS_DEF("body", minnet_request_get, 0, REQUEST_BODY, 0),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "MinnetRequest", JS_PROP_CONFIGURABLE),
};

const size_t minnet_request_proto_funcs_size = countof(minnet_request_proto_funcs);
