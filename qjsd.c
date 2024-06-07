//
// Created by nemtudom345 on 3/25/23.
//
#include <stdio.h>
#include <stdlib.h>
#include "quickjs-libc.h"

#include "monolithic_examples.h"

static int process_function(JSContext* ctx, JSValue func)
{
    if (JS_IsException(func)) {
        js_std_dump_error(ctx);
        return 1;
    }
	return 0;
}

#if defined(BUILD_MONOLITHIC)
#define main(cnt, arr)      qjsd_main(cnt, arr)
#endif

int main(int argc, const char** argv)
{
	int rv = 0;
	JS_Initialize();
	JSRuntime* rt = JS_NewRuntime();
    JSContext* ctx = JS_NewContext(rt);

    if (argc != 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "-?")) {
        printf("Usage: ./qjsd <input.bin>\n\nExecute compiled QuickJS function / binary code blob.\n");
		rv = 1;
        goto fail;
    }

    const char* input = argv[1];

    size_t buf_len;
	uint8_t *buf = js_load_file(ctx, &buf_len, input);
    if (!buf) {
        perror(input);
		rv = 2;
		goto fail;
    }

    JSValue obj = JS_ReadObject(ctx, buf, buf_len, JS_READ_OBJ_BYTECODE);
    rv = process_function(ctx, obj);

fail:
	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);
	JS_Finalize();

    return rv;
}
