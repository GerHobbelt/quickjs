#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>

#include "quickjs.h"

uint64_t
njs_time(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t) ts.tv_sec * 1000000000 + ts.tv_nsec;
}

int main(int argc, char **argv)
{
    JSRuntime *rt;
    JSContext *ctx;
    struct rusage  usage;

    uint64_t iters = 10000000;
    uint64_t ns = njs_time(), ms;

#if 1
    const char *str;

#define SRC "var COUNT= 5; function test() { return ['hello',  'world', ++COUNT].join(' '); }"

    rt = JS_NewRuntime();
    if (!rt) {
        fprintf(stderr, "qjs: cannot allocate JS runtime\n");
        exit(2);
    }

    ctx = JS_NewContext(rt);
    if (!ctx) {
        fprintf(stderr, "qjs: cannot allocate JS context\n");
        exit(2);
    }

    JSValue gcode = JS_Eval(ctx, (const char *) SRC, strlen(SRC), "<input>",
                  JS_EVAL_FLAG_COMPILE_ONLY  | JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(gcode)) {
        JSValue val = JS_GetException(ctx);
        str = JS_ToCString(ctx, val);
        if (!str) {
            fprintf(stderr, "qjs: cannot get exception after failed to compile\n");
            exit(2);
        }

        fprintf(stderr, "qjs: failed to compile: %s\n", str);
        JS_FreeCString(ctx, str);
        JS_FreeValue(ctx, val);
        exit(2);
    }

    gcode = JS_DupValue(ctx, gcode);
    JSValue ret = JS_EvalFunction(ctx, gcode);
    if (JS_IsException(ret)) {
        fprintf(stderr, "qjs: failed to run\n");
        exit(2);
    }

    JS_FreeValue(ctx, ret);

    JSValue global = JS_GetGlobalObject(ctx);
    JSAtom atom = JS_NewAtom(ctx, "test");
    JSValue func = JS_GetProperty(ctx, global, atom);
    JS_FreeAtom(ctx, atom);

    if (JS_IsException(func)) {
        fprintf(stderr, "qjs: failed to get 'test'\n");
        exit(2);
    }

    if (!JS_IsFunction(ctx, func)) {
        fprintf(stderr, "qjs: 'test' is not a function\n");
        exit(2);
    }

    for (int i = 0; i < iters; i++) {
        JSValue val = JS_Call(ctx, func, global, 0, NULL);

        str = JS_ToCString(ctx, val);
        if (!str) {
            fprintf(stderr, "qjs: cannot get value of the script\n");
            exit(2);
        }

//        fprintf(stderr, "qjs: %s\n", str);

        JS_FreeCString(ctx, str);
        JS_FreeValue(ctx, val);
    }

    JS_FreeValue(ctx, func);
    JS_FreeValue(ctx, global);
    JS_FreeValue(ctx, gcode);
    JS_FreeContext(ctx);

    JS_FreeRuntime(rt);
#else
    for (int i = 0; i < iters; i++) {
        rt = JS_NewRuntime();
        if (!rt) {
            fprintf(stderr, "qjs: cannot allocate JS runtime\n");
            exit(2);
        }

        ctx = JS_NewContext(rt);
        if (!ctx) {
            fprintf(stderr, "qjs: cannot allocate JS context\n");
            exit(2);
        }

        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
    }
#endif

    uint64_t total = njs_time() - ns;

    ms = total / 1000000;
    ns = total % 1000000;

    printf("total: %lu.%06lums\n", ms, ns);

    uint64_t periter = total / iters;

    ms = periter / 1000000;
    ns = periter % 1000000;

    printf("per req: %lu.%06lums\n", ms, ns);

    getrusage(RUSAGE_SELF, &usage);

    uint64_t us = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec
    + usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;

    printf("%.3fμs per VM, %d times/s\n",
      (double) us / iters, (int) ((uint64_t) iters * 1000000 / us));

    return 0;
}
