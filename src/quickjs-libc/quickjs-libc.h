/*
 * QuickJS C library
 *
 * Copyright (c) 2017-2018 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef QUICKJS_LIBC_H
#define QUICKJS_LIBC_H

#include <stdio.h>
#include <stdlib.h>

#include "quickjs.h"

#ifdef __cplusplus
extern "C"
{
#endif

  extern const uint8_t qjsc_inspect[];
  extern const uint32_t qjsc_inspect_size;

  extern const uint8_t qjsc_lib[];
  extern const uint32_t qjsc_lib_size;

  JSModuleDef *js_init_module_std(JSContext *ctx, const char *module_name);
  JSModuleDef *js_init_module_os(JSContext *ctx, const char *module_name);

  void js_std_add_helpers(JSContext *ctx, int argc, char **argv);

  void js_std_add_inspect(JSContext *ctx);
  void js_std_add_console(JSContext *ctx);
  void js_std_add_print(JSContext *ctx);
  void js_std_add_scriptArgs(JSContext *ctx, int argc, char **argv);
  void js_std_add_timeout(JSContext *ctx);
  void js_std_add_lib(JSContext *ctx);

  void js_std_loop(JSContext *ctx);
  void js_std_init_handlers(JSRuntime *rt);
  void js_std_free_handlers(JSRuntime *rt);
  void js_std_set_worker_new_context_func(JSContext *(*func)(JSRuntime *rt));

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* QUICKJS_LIBC_H */
