#ifndef SERVERWRAPPER_HPP
#define SERVERWRAPPER_HPP

#include "quickjs/quickjs-libc.h"
#include "quickjs/cutils.h"

JS_MODULE JSModuleDef *js_init_module_imgui(JSContext *ctx, const char *module_name);

#endif // SERVERWRAPPER_HPP