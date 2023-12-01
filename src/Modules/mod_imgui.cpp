#include "./mod_imgui.h"
#include "quickjs/quickjs.h"
#include "quickjs/cutils.h"
#include "quickjs/quickjs-libc.h"
#include <string.h>
#include "Log.hpp"
#include "CrossPlatform.h"
#if IS_LINUX_OS
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif // IS_LINUX_OS


static JSClassID js_imgui_window_class_id;
static const char js_imgui_window_class_name[]="udp";

static JSValue   js_imgui_window_init            (JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
static void      js_imgui_window_finalizer       (JSRuntime *rt,  JSValueConst this_val);
static JSValue   js_imgui_window_bind            (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static const JSCFunctionListEntry js_imgui_window_proto[] = {
          JS_CFUNC_DEF("bind", 2, js_imgui_window_bind),
};

static const JSCFunctionListEntry js_imgui_window_funcs[] = {
};

static JSClassDef js_imgui_window_class = {
    js_imgui_window_class_name,
    .finalizer = js_imgui_window_finalizer,
    .gc_mark=NULL,
    .call=NULL,
    .exotic=NULL
};

static const JSCFunctionListEntry js_imgui_funcs[] = {
	//JS_CFUNC_DEF("openFile", 2, js_uv_file_init),
};

static int js_uv_init(JSContext *ctx, JSModuleDef *m)
{
	JS_NewClassID(&js_imgui_window_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_imgui_window_class_id, &js_imgui_window_class);
    // create, initialize and set a object as the class prototype
    JSValue udp_proto = JS_NewObject(ctx);
    // properties
	JS_SetPropertyFunctionList(ctx, udp_proto, js_uv_udp_proto, countof(js_uv_udp_proto));
    // constructorjs_uv_udp_file_name
    JSValueConst udp_func = JS_NewCFunction2(ctx, js_uv_udp_init, js_uv_udp_class_name, 1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, udp_func, udp_proto);
    // static properties
    //JS_SetPropertyFunctionList(ctx, udp_func, js_uv_udp_funcs, countof(js_uv_udp_funcs));
    JS_SetClassProto(ctx, js_imgui_window_class_id, udp_proto);


    // constructor must be added to module functions
    JS_SetModuleExport(ctx, m,  js_uv_udp_class_name,  udp_func);
	return JS_SetModuleExportList(ctx,m,js_imgui_funcs,countof(js_imgui_funcs));
}
//JS_MODULE
//JS_MODULE
JSModuleDef *js_init_module_uv(JSContext *ctx, const char *module_name)
{
	JSModuleDef *m = JS_NewCModule(ctx, module_name, js_imgui_init);
	if (!m)
		return NULL;
	// JS_AddModuleExportList(ctx, m, js_uv_proto_timer, countof(js_uv_proto_timer));
	JS_AddModuleExportList(ctx, m, js_imgui_funcs, countof(js_imgui_funcs));
	JS_AddModuleExport(ctx, m, js_imgui_windows_class_name);
	return m;
}
