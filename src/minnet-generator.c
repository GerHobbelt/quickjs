#include "minnet-generator.h"
#include "jsutils.h"
#include <quickjs.h>
#include <assert.h>
#include <libwebsockets.h>

THREAD_LOCAL JSClassID minnet_generator_class_id;
THREAD_LOCAL JSValue minnet_generator_proto, minnet_generator_ctor;

static JSValue
minnet_generator_next(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, void* opaque) {
  MinnetGenerator* gen = (MinnetGenerator*)opaque;
  JSValue ret, arg = argc > 0 ? argv[0] : JS_UNDEFINED;

  ret = generator_next(gen, arg);

  // js_async_then(ctx, ret, js_function_bind_return(ctx, ))

  return ret;
}

static JSValue
minnet_generator_push(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, void* opaque) {
  MinnetGenerator* gen = (MinnetGenerator*)opaque;
  JSValue ret = JS_UNDEFINED;

  if(argc < 1)
    return JS_ThrowInternalError(ctx, "argument required");

  // JSValue callback = argc > 1 ? JS_DupValue(ctx, argv[1]) : JS_NULL;

  ret = generator_push(gen, argv[0]);
  // JS_FreeValue(ctx, callback);
  return ret;
}

static JSValue
minnet_generator_stop(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, void* opaque) {
  MinnetGenerator* gen = (MinnetGenerator*)opaque;
  JSValue ret = JS_UNDEFINED;

  // JSValue callback = argc > 1 ? JS_DupValue(ctx, argv[1]) : JS_NULL;

  ret = JS_NewBool(ctx, generator_stop(gen, JS_NULL));

  // JS_FreeValue(ctx, callback);

  return ret;
}

JSValue
minnet_generator_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  MinnetGenerator* gen;
  JSValue args[2];

  if(!(gen = generator_new(ctx)))
    return JS_EXCEPTION;

  if(argc < 1 || !JS_IsFunction(ctx, argv[0]))
    return JS_ThrowInternalError(ctx, "MinnetGenerator needs a function parameter");

  args[0] = js_function_cclosure(ctx, minnet_generator_push, 0, 0, generator_dup(gen), (void (*)(void*)) & generator_free);
  args[1] = js_function_cclosure(ctx, minnet_generator_stop, 0, 0, generator_dup(gen), (void (*)(void*)) & generator_free);

  gen->executor = js_function_bind_argv(ctx, argv[0], 2, args);
  /*  ret = JS_Call(ctx, argv[0], JS_UNDEFINED, 2, args);

    JS_FreeValue(ctx, ret);*/
  JS_FreeValue(ctx, args[0]);
  JS_FreeValue(ctx, args[1]);

  return minnet_generator_iterator(ctx, gen);
}

static const JSCFunctionListEntry minnet_generator_funcs[] = {
    JS_CFUNC_DEF("[Symbol.asyncIterator]", 0, (JSCFunction*)&JS_DupValue),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "MinnetGenerator", JS_PROP_CONFIGURABLE),
};

JSValue
minnet_generator_iterator(JSContext* ctx, MinnetGenerator* gen) {
  JSValue ret = JS_NewObject(ctx);

  JS_SetPropertyStr(ctx, ret, "next", js_function_cclosure(ctx, minnet_generator_next, 0, 0, generator_dup(gen), (void*)&generator_free));
  JS_SetPropertyFunctionList(ctx, ret, minnet_generator_funcs, countof(minnet_generator_funcs));

  return ret;
}

JSValue
minnet_generator_create(JSContext* ctx, MinnetGenerator** gen_p) {
  if(!*gen_p)
    *gen_p = generator_new(ctx);
  else
    generator_dup(*gen_p);

  return minnet_generator_iterator(ctx, *gen_p);
}

static void
minnet_generator_finalizer(JSRuntime* rt, JSValue val) {
  MinnetGenerator* g;

  if((g = JS_GetOpaque(val, minnet_generator_class_id))) {
    generator_free(g);
  }
}

JSClassDef minnet_generator_class = {
    "MinnetGenerator",
    .finalizer = minnet_generator_finalizer,
};
