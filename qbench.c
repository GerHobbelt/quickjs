#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#if !defined(_WIN32)
#include <unistd.h>
#else
#include "win/sys_time.h"
#include <sys/utime.h>
#endif

#include "quickjs.h"

#include "monolithic_examples.h"


static uint64_t
njs_time(void)
{
	struct timeval ts;
	gettimeofday(&ts, NULL);

    return (uint64_t) ts.tv_sec * 1000000 + ts.tv_usec;
}


#if defined(BUILD_MONOLITHIC)
#define main      qjs_benchmark_main
#endif

int main(int argc, const char **argv)
{
    JSRuntime *rt;
    JSContext *ctx;
#if defined(RUSAGE_SELF)
	struct rusage usage;
#endif
    uint64_t iters = 10000000;
    uint64_t ns = njs_time();
	uint64_t ms;

#if 1
    const char *str;

#define SRC "var COUNT= 5; function test() { return ['hello',  'world', ++COUNT].join(' '); }"

    rt = JS_NewRuntime();
    if (!rt) {
        fprintf(stderr, "qjs: cannot allocate JS runtime\n");
        return 2;
    }

    ctx = JS_NewContext(rt);
    if (!ctx) {
        fprintf(stderr, "qjs: cannot allocate JS context\n");
        return 2;
    }

    JSValue gcode = JS_Eval(ctx, (const char *) SRC, strlen(SRC), "<input>",
                  JS_EVAL_FLAG_COMPILE_ONLY  | JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(gcode)) {
        JSValue val = JS_GetException(ctx);
        str = JS_ToCString(ctx, val);
        if (!str) {
            fprintf(stderr, "qjs: cannot get exception after failed to compile\n");
            return 2;
        }

        fprintf(stderr, "qjs: failed to compile: %s\n", str);
        JS_FreeCString(ctx, str);
        JS_FreeValue(ctx, val);
        return 2;
    }

    gcode = JS_DupValue(ctx, gcode);
    JSValue ret = JS_EvalFunction(ctx, gcode);
    if (JS_IsException(ret)) {
        fprintf(stderr, "qjs: failed to run\n");
        return 2;
    }

    JS_FreeValue(ctx, ret);

    JSValue global = JS_GetGlobalObject(ctx);
    JSAtom atom = JS_NewAtom(ctx, "test");
    JSValue func = JS_GetProperty(ctx, global, atom);
    JS_FreeAtom(ctx, atom);

    if (JS_IsException(func)) {
        fprintf(stderr, "qjs: failed to get 'test'\n");
        return 2;
    }

    if (!JS_IsFunction(ctx, func)) {
        fprintf(stderr, "qjs: 'test' is not a function\n");
        return 2;
    }

    for (int i = 0; i < iters; i++) {
        JSValue val = JS_Call(ctx, func, global, 0, NULL);

        str = JS_ToCString(ctx, val);
        if (!str) {
            fprintf(stderr, "qjs: cannot get value of the script\n");
            return 2;
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
            return 2;
        }

        ctx = JS_NewContext(rt);
        if (!ctx) {
            fprintf(stderr, "qjs: cannot allocate JS context\n");
            return 2;
        }

        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
    }
#endif

    uint64_t total = njs_time() - ns;

    ms = total / 1000000;
    ns = total % 1000000;

    printf("total: %lu.%06lums\n", (long)ms, (long)ns);

    uint64_t periter = total / iters;

    ms = periter / 1000000;
    ns = periter % 1000000;

    printf("per req: %lu.%06lums\n", (long)ms, (long)ns);

#if defined(RUSAGE_SELF)
	getrusage(RUSAGE_SELF, &usage);

    uint64_t us = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec
    + usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;
#else
	uint64_t us = total;
#endif

    printf("%.3fμs per VM, %d times/s\n",
      (double) us / iters, (int) ((uint64_t) iters * 1000000 / us));

    return 0;
}
