#ifndef MOD_SQLITE_HPP
#define MOD_SQLITE_HPP

#include "quickjs/quickjs-libc.h"
#include "quickjs/cutils.h"
#include "quickjs/config.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#define SQLITE_INIT_MODULE js_init_module_sqlite
#else
#define SQLITE_INIT_MODULE js_init_module
#endif // NDEBUG

JS_MODULE JSModuleDef *SQLITE_INIT_MODULE(JSContext *ctx, const char *module_name);


#ifdef __cplusplus
}
#endif

#endif // MOD_SQLITE_HPP
