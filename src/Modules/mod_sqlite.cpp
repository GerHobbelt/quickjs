#include "./mod_sqlite.h"
#include "new"
#include "quickjs/quickjs.h"
#include "quickjs/cutils.h"
#include "quickjs/quickjs-libc.h"
#include <string.h>
#include "Log.hpp"
#include "CrossPlatform.h"
#include "sqlite3.h"


static JSClassID js_sqlite_sqlite3_class_id;
static JSValue   js_sqlite_sqlite3_init            (JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
static void      js_sqlite_sqlite3_finalizer       (JSRuntime *rt,  JSValueConst this_val);
static void      js_sqlite_sqlite3_gc_mark         (JSRuntime *rt,  JSValueConst this_val, JS_MarkFunc *mark_func);
static JSValue   js_sqlite_sqlite3_exec            (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static const JSCFunctionListEntry js_sqlite_sqlite3_proto[] = {
	JS_CFUNC_DEF("exec", 1, js_sqlite_sqlite3_exec),
};
static const JSCFunctionListEntry js_sqlite_sqlite3_funcs[] = {
};
static const char js_sqlite_sqlite3_class_name[]="sqlite3";
static JSClassDef js_sqlite_sqlite3_class = {
    js_sqlite_sqlite3_class_name,
    .finalizer=js_sqlite_sqlite3_finalizer,
    .gc_mark = js_sqlite_sqlite3_gc_mark,
    // note: this is not constructor!
    .call=NULL,
    .exotic=NULL
};



static JSValue js_sqlite3_key(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static const JSCFunctionListEntry js_sqlite3_funcs[] = {
	//JS_CFUNC_DEF("key", 1, js_sqlite3_key),
};

static int js_sqlite3_init(JSContext *ctx, JSModuleDef *m)
{
	JS_NewClassID(&js_sqlite_sqlite3_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_sqlite_sqlite3_class_id, &js_sqlite_sqlite3_class);
    JSValue sqlite3_proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, sqlite3_proto, js_sqlite_sqlite3_proto, countof(js_sqlite_sqlite3_proto));
    JSValueConst sqlite3_func = JS_NewCFunction2(ctx, js_sqlite_sqlite3_init, js_sqlite_sqlite3_class_name, 1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, sqlite3_func, sqlite3_proto);
    JS_SetClassProto(ctx, js_sqlite_sqlite3_class_id, sqlite3_proto);

    JS_SetModuleExport(ctx, m, js_sqlite_sqlite3_class_name,                      sqlite3_func  );
	return JS_SetModuleExportList(ctx,m,js_sqlite_sqlite3_funcs,countof(js_sqlite_sqlite3_funcs));
}
JSModuleDef *SQLITE_INIT_MODULE(JSContext *ctx, const char *module_name)
{
	JSModuleDef *m = JS_NewCModule(ctx, module_name, js_sqlite3_init);
	if (!m)
		return NULL;
	JS_AddModuleExportList(ctx, m, js_sqlite_sqlite3_funcs, countof(js_sqlite_sqlite3_funcs));
	JS_AddModuleExport    (ctx, m, js_sqlite_sqlite3_class_name);
	return m;
}
























struct js_sqlite_sqlite3
{
    sqlite3 *db;
	JSContext *ctx;
    JSValue this_val;
    //JSValue socket_address;
};

static JSValue js_sqlite_sqlite3_init(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv) {
    js_sqlite_sqlite3 *db_ctx;
    const char *arg_temp;
    size_t len=0;
    sqlite3 *db;
    int32_t res;
    JSValue thiz;

    if(argc!=1)
        goto Invalid_arg;

    if( (arg_temp = (const char *)JS_ToCStringLenRaw(ctx, &len, argv[0])) == NULL)
            goto Invalid_arg;

    db_ctx=(js_sqlite_sqlite3*)js_malloc(ctx, sizeof(js_sqlite_sqlite3));
    if (!db_ctx)
        return JS_ThrowOutOfMemory(ctx);

    res = sqlite3_open(arg_temp, &db);

    if (res != SQLITE_OK) {
        js_free(ctx,db_ctx);
        JSValue e=JS_ThrowInternalError(ctx,sqlite3_errmsg(db));
        sqlite3_close(db);
        return e;
    }

    JSValue proto;
    if (JS_IsUndefined(new_target))
        proto = JS_GetClassProto(ctx, js_sqlite_sqlite3_class_id);
    else
        proto = JS_GetPropertyStr(ctx, new_target, "prototype");

    if (JS_IsException(proto)) {
        sqlite3_close(db);
        js_free(ctx,db_ctx);
        return proto;
    }
    thiz = JS_NewObjectProtoClass(ctx, proto, js_sqlite_sqlite3_class_id);
    JS_FreeValue(ctx, proto);

    if (JS_IsException(thiz))
    {
        sqlite3_close(db);
        js_free(ctx,db_ctx);
        return thiz;
    }

    db_ctx->this_val = thiz;
    db_ctx->ctx=ctx;
    db_ctx->db=db;
    //stream_ctx->socket_address=JS_UNDEFINED;
    JS_SetOpaque(thiz, db_ctx);
    return thiz;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}



static void js_sqlite_sqlite3_gc_mark(JSRuntime *rt,  JSValueConst this_val, JS_MarkFunc *mark_func)
{
    //UDP *stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);
    //JS_MarkValue(rt, stream_ctx->recvcb, mark_func);
    //JS_MarkValue(rt, stream_ctx->socket_address, mark_func);
}

static void js_sqlite_sqlite3_finalizer(JSRuntime *rt, JSValueConst this_val)
{
    js_sqlite_sqlite3 *db_ctx=(js_sqlite_sqlite3 *)JS_GetOpaque(this_val,js_sqlite_sqlite3_class_id);
    sqlite3_close(db_ctx->db);
    js_free_rt(rt,db_ctx);
}



struct callback_data
{
    js_sqlite_sqlite3 *db_ctx;
    JSContext *ctx;
    JSValue this_val,cb;
};
static int sqlite_exec_callback(void *ctx, int argc, char **argv, char **azColName);

static JSValue js_sqlite_sqlite3_exec (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *sql;
    js_sqlite_sqlite3 *db_ctx;
    char *err_msg;
    size_t len;
    int res;

    if(argc!=1 && argc!=2)
        goto Invalid_arg;
    if( (sql = (const char *)JS_ToCStringLenRaw(ctx, &len, argv[0])) == NULL)
        goto Invalid_arg;
    if(argc==2 && !JS_IsFunction(ctx, argv[1])){
        goto Invalid_arg;
    }

    db_ctx=(js_sqlite_sqlite3 *)JS_GetOpaque(this_val,js_sqlite_sqlite3_class_id);

    if(argc==2){
        callback_data data={
            db_ctx,ctx,this_val,argv[1]
        };
        res = sqlite3_exec(db_ctx->db, sql, sqlite_exec_callback, &data, &err_msg);
    }else{
        res = sqlite3_exec(db_ctx->db, sql, 0, 0, &err_msg);
    }
    if (res != SQLITE_OK ) {
        JSValue e=JS_ThrowInternalError(ctx,err_msg);
        sqlite3_free(err_msg);
        return e;
    }
    return JS_UNDEFINED;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}


static int sqlite_exec_callback(void *parg, int argc, char **argv, char **azColName)
{
    callback_data *data=(callback_data *)parg;
    JSValue ret;
    JSValue res[1]={JS_UNDEFINED};

    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    puts("");

    ret = JS_Call(data->ctx, data->cb, data->this_val, 0, res);
    if (unlikely(JS_IsException(ret))) js_std_dump_error(data->ctx);
    JS_FreeValue(data->ctx, ret);

    return 0;
}


