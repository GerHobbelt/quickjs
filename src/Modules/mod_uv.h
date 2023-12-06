#ifndef MOD_UV_HPP
#define MOD_UV_HPP

#include "quickjs/quickjs-libc.h"
#include "quickjs/cutils.h"
#include "quickjs/config.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#define UV_INIT_MODULE js_init_module_uv
#else
#define UV_INIT_MODULE js_init_module
#endif // NDEBUG

JS_MODULE JSModuleDef *UV_INIT_MODULE(JSContext *ctx, const char *module_name);


#ifdef __cplusplus
}
#endif

#endif // MOD_UV_HPP
