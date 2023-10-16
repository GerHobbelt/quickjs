//
// Created by nemtudom345 on 3/25/23.
//
#include <stdio.h>
#include <stdlib.h>
#include "quickjs-libc.h"

void process_function(JSContext* ctx, JSValue func)
{
    if (JS_IsException(func)) {
        js_std_dump_error(ctx);
        exit(1);
    }
}

int main(int argc, char** argv)
{
    JSRuntime* rt = JS_NewRuntime();
    JSContext* ctx = JS_NewContext(rt);

    if (argc < 3) {
        printf("Usage: ./qjsd <input.bin> <output.js>\n");
        return 1;
    }

    const char* input = argv[1];
    const char* output = argv[2];

    uint8_t *buf;
    size_t buf_len;

    buf = js_load_file(ctx, &buf_len, input);
    if (!buf) {
        perror(input);
        exit(1);
    }

    JSValue obj = JS_ReadObject(ctx, buf, buf_len, JS_READ_OBJ_BYTECODE);
    process_function(ctx, obj);

    return 0;
}