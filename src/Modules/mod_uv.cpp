#include "./mod_uv.h"
#include "uv.h"
#include "new"
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


static JSClassID  js_uv_file_class_id;
static JSValue js_uv_file_init      (JSContext *ctx, JSValueConst new_target,int argc,JSValueConst *argv);
static void    js_uv_file_finalizer (JSRuntime *rt, JSValueConst this_val);
static void    js_uv_file_gc_mark   (JSRuntime *rt,  JSValueConst this_val, JS_MarkFunc *mark_func);
static JSValue js_uv_file_close     (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_uv_file_read      (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_uv_file_write     (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static const JSCFunctionListEntry js_uv_file_proto[] = {
	JS_CFUNC_DEF("read",  3, js_uv_file_read),// offset:number,size:number,cb:function
	JS_CFUNC_DEF("write", 1, js_uv_file_write),// data:string{,offset:number,cb:function}
};
static const char js_uv_file_class_name[]="File";
static JSClassDef js_uv_file_class = {
    js_uv_file_class_name,
    .finalizer = js_uv_file_finalizer,
    .gc_mark=NULL,
    .call=NULL,
    .exotic=NULL
};





static JSClassID js_uv_dir_class_id;
static JSValue   js_uv_dir_init      (JSContext *ctx, JSValueConst new_target,int argc,JSValueConst *argv);
static void      js_uv_dir_finalizer (JSRuntime *rt, JSValueConst this_val);
static JSValue   js_uv_dir_read      (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static const JSCFunctionListEntry js_uv_dir_proto[] = {
	JS_CFUNC_DEF("read",  0, js_uv_dir_read),
};
static const char js_uv_dir_class_name[]="Dir";
static JSClassDef js_uv_dir_class = {
    js_uv_dir_class_name,
    .finalizer = js_uv_dir_finalizer,
    .gc_mark=NULL,
    .call=NULL,
    .exotic=NULL
};



static JSClassID  js_uv_sockaddr_class_id;
static JSValue    js_uv_sockaddr_init       (JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv, int magic);
static void       js_uv_sockaddr_finalizer  (JSRuntime *rt,  JSValueConst this_val);
static const char js_uv_sockaddr_class_name[]="sockaddr";
static JSClassDef js_uv_sockaddr_class = {
    js_uv_sockaddr_class_name,
    .finalizer = js_uv_sockaddr_finalizer,
    .gc_mark=NULL,
    // note: this is not constructor!
    .call=NULL,
    .exotic=NULL
};
static const char js_uv_sockaddr6_class_name[]="sockaddr6";



static JSClassID js_uv_udp_class_id;
static JSValue   js_uv_udp_init            (JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
static void      js_uv_udp_finalizer       (JSRuntime *rt,  JSValueConst this_val);
static void      js_uv_udp_gc_mark         (JSRuntime *rt,  JSValueConst this_val, JS_MarkFunc *mark_func);
static JSValue   js_uv_udp_bind            (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_connect         (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_start_recv      (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_srop_recv       (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_send            (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_try_send        (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_set             (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
static JSValue   js_uv_udp_get             (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
static const JSCFunctionListEntry js_uv_udp_proto[] = {
	JS_CFUNC_DEF("start_recv", 2, js_uv_udp_start_recv),
	JS_CFUNC_DEF("stop_recv", 2, js_uv_udp_srop_recv),
	JS_CFUNC_DEF("bind", 1, js_uv_udp_bind),
	JS_CFUNC_DEF("connect", 1, js_uv_udp_connect),
	JS_CFUNC_DEF("send", 1, js_uv_udp_send),
	JS_CFUNC_DEF("try_send", 1, js_uv_udp_try_send),
	JS_CFUNC_MAGIC_DEF("set_ttl",            1, js_uv_udp_set, 1),
	JS_CFUNC_MAGIC_DEF("set_multicast_ttl",  1, js_uv_udp_set, 2),
	JS_CFUNC_MAGIC_DEF("set_multicast_loop", 1, js_uv_udp_set, 4),
	JS_CFUNC_MAGIC_DEF("set_set_broadcast",  1, js_uv_udp_set, 8),
	JS_CFUNC_MAGIC_DEF("get_send_queue_size",  0, js_uv_udp_get, 1),
	JS_CFUNC_MAGIC_DEF("get_send_queue_count", 0, js_uv_udp_get, 2),
	JS_CFUNC_MAGIC_DEF("using_recvmmsg",       0, js_uv_udp_get, 4),
};
static const JSCFunctionListEntry js_uv_udp_funcs[] = {
};
static const char js_uv_udp_class_name[]="udp";
static JSClassDef js_uv_udp_class = {
    js_uv_udp_class_name,
    .finalizer = js_uv_udp_finalizer,
    .gc_mark = js_uv_udp_gc_mark,
    // note: this is not constructor!
    .call=NULL,
    .exotic=NULL
};

static JSValue js_uv_simulate_key(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static const JSCFunctionListEntry js_uv_funcs[] = {
	// openFile(Addr:string, Flags:string[, cb:function]):any ;
	//JS_CFUNC_DEF("openFile", 2, js_uv_file_init),
	//JS_CFUNC_DEF("openudp", 0, js_uv_udp_init),
	JS_CFUNC_DEF("sim", 1, js_uv_simulate_key),
};

static int js_uv_init(JSContext *ctx, JSModuleDef *m)
{
	/* File class */

	JS_NewClassID(&js_uv_file_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_uv_file_class_id, &js_uv_file_class);
    JSValue file_proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, file_proto, js_uv_file_proto, countof(js_uv_file_proto));
    JSValueConst file_func = JS_NewCFunction2(ctx, js_uv_file_init, js_uv_file_class_name, 2, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, file_func, file_proto);
    JS_SetClassProto(ctx, js_uv_file_class_id, file_proto);


    JS_NewClassID(&js_uv_dir_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_uv_dir_class_id, &js_uv_dir_class);
    JSValue dir_proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, dir_proto, js_uv_dir_proto, countof(js_uv_dir_proto));
    JSValueConst dir_func = JS_NewCFunction2(ctx, js_uv_dir_init, js_uv_dir_class_name, 2, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, dir_func, dir_proto);
    JS_SetClassProto(ctx, js_uv_dir_class_id, dir_proto);



//    JS_NewClassID(&js_uv_sockaddr_class_id);
//    JS_NewClass(JS_GetRuntime(ctx), js_uv_sockaddr_class_id, &js_uv_sockaddr_class);
//    JSValue sockaddr_proto = JS_NewObject(ctx);
//    JSValueConst sockaddr_func = JS_NewCFunction2(ctx, js_uv_sockaddr_init, js_uv_sockaddr_class_name, 2, JS_CFUNC_constructor_magic, 0);
//    JS_SetConstructor(ctx, sockaddr_func, sockaddr_proto);
//    JS_SetClassProto(ctx, js_uv_sockaddr_class_id, sockaddr_proto);

    JS_NewClassID(&js_uv_sockaddr_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_uv_sockaddr_class_id, &js_uv_sockaddr_class);
    JSValue sockaddr_proto = JS_NewObject(ctx);
    JS_SetClassProto(ctx, js_uv_sockaddr_class_id, sockaddr_proto);

    JSValueConst sockaddr_func = JS_NewCFunctionMagic(ctx, js_uv_sockaddr_init, js_uv_sockaddr_class_name, 2, JS_CFUNC_constructor_magic, 0);
    JSValueConst sockaddr6_func= JS_NewCFunctionMagic(ctx, js_uv_sockaddr_init, js_uv_sockaddr6_class_name,2, JS_CFUNC_constructor_magic, 1);



    // a class is identified by it unique class ID
	JS_NewClassID(&js_uv_udp_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_uv_udp_class_id, &js_uv_udp_class);
    // create, initialize and set a object as the class prototype
    JSValue udp_proto = JS_NewObject(ctx);
    // properties
	JS_SetPropertyFunctionList(ctx, udp_proto, js_uv_udp_proto, countof(js_uv_udp_proto));
    // constructorjs_uv_udp_file_name
    JSValueConst udp_func = JS_NewCFunction2(ctx, js_uv_udp_init, js_uv_udp_class_name, 0, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, udp_func, udp_proto);
    // static properties
    //JS_SetPropertyFunctionList(ctx, udp_func, js_uv_udp_funcs, countof(js_uv_udp_funcs));
    JS_SetClassProto(ctx, js_uv_udp_class_id, udp_proto);


    // constructors must be added to the module function list
    JS_SetModuleExport(ctx, m, js_uv_file_class_name,          file_func);
    JS_SetModuleExport(ctx, m, js_uv_dir_class_name,            dir_func);
    JS_SetModuleExport(ctx, m, js_uv_sockaddr_class_name,  sockaddr_func);
    JS_SetModuleExport(ctx, m, js_uv_sockaddr6_class_name,sockaddr6_func);
    JS_SetModuleExport(ctx, m, js_uv_udp_class_name,            udp_func);
	return JS_SetModuleExportList(ctx,m,js_uv_funcs,countof(js_uv_funcs));
}
JSModuleDef *UV_INIT_MODULE(JSContext *ctx, const char *module_name)
{
	JSModuleDef *m = JS_NewCModule(ctx, module_name, js_uv_init);
	if (!m)
		return NULL;
	// JS_AddModuleExportList(ctx, m, js_uv_proto_timer, countof(js_uv_proto_timer));
	JS_AddModuleExportList(ctx, m, js_uv_funcs, countof(js_uv_funcs));
	JS_AddModuleExport(ctx, m, js_uv_file_class_name);
	JS_AddModuleExport(ctx, m, js_uv_dir_class_name);
	JS_AddModuleExport(ctx, m, js_uv_sockaddr_class_name);
	JS_AddModuleExport(ctx, m, js_uv_sockaddr6_class_name);
	JS_AddModuleExport(ctx, m, js_uv_udp_class_name);
	return m;
}

















static void onFileOpen              (uv_fs_t *req);
static void onForceClose            (uv_fs_t* req);
static void onRead                  (uv_fs_t* req);
static void onWrite                 (uv_fs_t* req);

struct Event
{
    uv_fs_t request;
    JSValue onEvent;
};

struct IOEvent
{
    uv_fs_t request;
    JSValue onEvent;
    uv_buf_t bufferPtr[1];
    char bufferData[];
};

struct FileSystem
{
    uv_file F;
	JSContext *ctx;
    JSValue this_val;
    //ssize_t refrence_count; replaced by built-in variable refrence count
    //uv_buf_t buffer;
};





static JSValue js_uv_file_init(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv)
{
    JSValue proto,thiz;
    int res=1;
    FileSystem *stream_ctx;
    Event *event_ctx;
    const uint8_t *arg_temp;
    size_t len=0;
    int32_t flag=0,mode=0;

    if(argc!=3)  goto Invalid_arg;
    if(!JS_IsString(argv[0]))  goto Invalid_arg;
    if(!JS_IsFunction(ctx,argv[2])) goto Invalid_arg;

    //if(is_file)
    {
        int32_t flag_temp=0;
        if (!JS_IsString(argv[1]))
            goto Invalid_arg;
        if( (arg_temp = JS_ToCStringLenRaw(ctx, &len, argv[1])) == NULL)
            goto Invalid_arg;
        for(size_t i=0;i<len && arg_temp[i];i++){
            switch(arg_temp[i]){
                case 'r':flag_temp|=1;break;
                case 'w':flag_temp|=2;break;
                case '+':flag_temp|=4;break;
                case 'a':mode|=O_TRUNC;break;
                #if !IS_WINDOWS_OS
                case 't':mode|=O_TMPFILE;break;
                #endif
                default:
                    return JS_ThrowTypeError(ctx, "Invalid flag: %c",arg_temp[i]);
            }
        }
        if(flag_temp&4) flag|=O_CREAT;
        if((flag_temp&3)==3) flag|=O_RDWR;
        else if(flag_temp&2) flag|=O_WRONLY;
        else if(flag_temp&1) flag|=O_RDONLY;
        else return JS_ThrowTypeError(ctx, "bad flag");
    }


    stream_ctx=(FileSystem*)js_malloc(ctx, sizeof(FileSystem));
    if (!stream_ctx){
        return JS_ThrowOutOfMemory(ctx);
    }
    stream_ctx->ctx=ctx;
    //stream_ctx->this_val = JS_NewObjectClass(ctx, 1?js_uv_file_class_id:js_uv_DIR_class_id);
    //if(is_file)
        stream_ctx->F=-1;
    //else
    //    stream_ctx->system.D=NULL;



    if (JS_IsUndefined(new_target))
        proto = JS_GetClassProto(ctx, js_uv_file_class_id);
    else
        proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto)) {
        js_free(ctx,stream_ctx);
        return proto;
    }
    thiz = JS_NewObjectProtoClass(ctx, proto, js_uv_file_class_id);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(thiz))
    {
        js_free(ctx,stream_ctx);
        return thiz;
    }
    stream_ctx->this_val = thiz;
    JS_SetOpaque(stream_ctx->this_val, stream_ctx);




    event_ctx=(Event*)js_malloc(ctx,sizeof(Event));
    if (!event_ctx)
    {
        JS_FreeValue(ctx, thiz);
        return JS_ThrowOutOfMemory(ctx);
    }
    event_ctx->onEvent=JS_UNDEFINED;



    if((arg_temp = JS_ToCStringLenRaw(ctx, &len, argv[0])) == NULL)
        return JS_ThrowTypeError(ctx,"Invalid arguments");

    res=uv_fs_open(uv_default_loop(), &event_ctx->request, (const char *)arg_temp, flag, mode, onFileOpen);



    if(res)
    {
        js_free(ctx, event_ctx);
        JS_FreeValue(ctx, thiz);
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    }



    event_ctx->request.data=stream_ctx;
    event_ctx->onEvent = JS_DupValue(ctx, argv[argc-1]);
    JS_DupValue(ctx, stream_ctx->this_val);
    return thiz;

Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}


static void onFileOpen(uv_fs_s *req)
{
    JSValue ret;
    JSValue err[1]={JS_UNDEFINED};
    Event *event_ctx=(Event *)req;
    FileSystem *stream_ctx=(FileSystem *)req->data;

    if(req->result < 0)
        err[0]=JS_NewString(stream_ctx->ctx,uv_strerror(req->result));
    // 'result' contains file descriptor or negative error number

    stream_ctx->F=req->result;

    {
        ret = JS_Call(stream_ctx->ctx, event_ctx->onEvent, JS_UNDEFINED, 1, err);
        if (unlikely(JS_IsException(ret))) js_std_dump_error(stream_ctx->ctx);
        JS_FreeValue(stream_ctx->ctx, ret);
    }


    uv_fs_req_cleanup(req);
    JS_FreeValue(stream_ctx->ctx, err[0]);
    JS_FreeValue(stream_ctx->ctx, event_ctx->onEvent);
    js_free(stream_ctx->ctx,event_ctx);

    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}


//static void js_uv_file_gc_mark(JSRuntime *rt,  JSValueConst this_val, JS_MarkFunc *mark_func){
//    FileSystem *stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_file_class_id);
//}


static void js_uv_file_finalizer(JSRuntime *rt, JSValueConst this_val)
{
    int res;
    FileSystem *stream_ctx=NULL;
    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_file_class_id);
    if(stream_ctx->F<0)     goto end;

    {
        Event *event_ctx;
        event_ctx=(Event*)js_malloc_rt(rt,sizeof(Event));
        event_ctx->request.data=rt;
        event_ctx->onEvent=JS_UNDEFINED;
        //if(JS_GetClassID(stream_ctx->this_val) == js_uv_DIR_class_id)
        //    res=uv_fs_closedir(uv_default_loop(), &event_ctx->request, stream_ctx->system.D, onForceClose);
        //else
        res=uv_fs_close(uv_default_loop(), &event_ctx->request, stream_ctx->F, onForceClose);
        if(unlikely(res)) {
            js_free_rt(rt,event_ctx);
            Console(warn,"js_uv_file_finalizer","finalizer fs close failed\n");
        }
    }
end:
    js_free_rt(rt, stream_ctx);
}

static void onForceClose(uv_fs_t* req){
    //puts("LOG: onForceClose");
    //Event *event_ctx=(Event *)req;
    if(req->result<0){
        Console(note,"onForceClose","finalizer fs close cb failed\n");
    }
    uv_fs_req_cleanup(req);
    // must not causes problem
    js_free_rt((JSRuntime *)(req->data),req);
}

static JSValue js_uv_file_read(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    FileSystem *stream_ctx=NULL;
    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_file_class_id);


    int64_t offset=0,size=0;


    if(argc!=3)
		return JS_ThrowTypeError(ctx, "Invalid arguments");
    if(!JS_IsFunction(ctx,argv[2]))
		return JS_ThrowTypeError(ctx, "Invalid arguments");

    if(JS_ToInt64(ctx,&offset,argv[0]) || JS_ToInt64(ctx,&size,argv[1])){
        return JS_ThrowTypeError(ctx, "JS_ToInt64 failed");
    }

    if(size<1){
        return JS_ThrowRangeError(ctx, "invalid size %ld",size);
    }

    Console(log,"js_uv_file_read","read (%ld,%ld)\n",offset,size);
    if(stream_ctx->F>=0){
        int res;
        IOEvent *event_ctx=(IOEvent*)js_malloc(ctx,sizeof(IOEvent)+size);
        event_ctx->request.data=stream_ctx;
        event_ctx->bufferPtr[0]=uv_buf_init(event_ctx->bufferData, size);
        res=uv_fs_read(uv_default_loop(), &event_ctx->request, stream_ctx->F, event_ctx->bufferPtr, 1, offset, onRead);
        if(res){
            js_free(ctx,event_ctx);
            return JS_ThrowInternalError(ctx, uv_strerror(res));
        }else{
            event_ctx->onEvent = JS_DupValue(ctx, argv[2]);
            JS_DupValue(ctx, this_val);
        }
    }else{
        return JS_ThrowTypeError(ctx, "this is not initilized currectly");
    }

    return JS_UNDEFINED;
}



static void free_file_buffer(JSRuntime *rt, void *opaque, void *ptr)
{
    (void)rt;
    (void)opaque;
    (void)ptr;
    js_free_rt(rt,opaque);
}


static void onRead(uv_fs_t* req)
{
    IOEvent *event_ctx=NULL;
    FileSystem *stream_ctx=NULL;
    JSValue args[2];
    JSValue ret;

    event_ctx=(IOEvent *)req;
    stream_ctx=(FileSystem *)req->data;

    args[0]= req->result<0 ? JS_NewString(stream_ctx->ctx,uv_strerror(req->result)) : JS_UNDEFINED;
    //args[1]= req->result<0 ? JS_UNDEFINED : JS_NewStringLen(stream_ctx->ctx,event_ctx->bufferPtr->base,req->result);
    args[1]= req->result<0 ? JS_UNDEFINED : JS_NewArrayBuffer(stream_ctx->ctx, (uint8_t *)event_ctx->bufferPtr->base, req->result, &free_file_buffer, req,false);

    ret = JS_Call(stream_ctx->ctx, event_ctx->onEvent, stream_ctx->this_val, 2, args);
    if (JS_IsException(ret))
        js_std_dump_error(stream_ctx->ctx);

    JS_FreeValue(stream_ctx->ctx, ret);
    JS_FreeValue(stream_ctx->ctx, args[0]);
    JS_FreeValue(stream_ctx->ctx, args[1]);

    uv_fs_req_cleanup(req);
    JS_FreeValue(stream_ctx->ctx, event_ctx->onEvent);


    // WARN: JS_FreeValue may call js_uv_file_finalizer (if no refrence exists)
    // so there is no guaranty that 'stream_ctx' will be available
    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}

static JSValue js_uv_file_write(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    FileSystem *stream_ctx=NULL;
    int64_t offset=-1;

    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_file_class_id);

    if(argc!=3 && argc!=2 && argc!=1)
		return JS_ThrowTypeError(ctx, "Invalid arguments");
    if(!JS_IsString(argv[0]))
		return JS_ThrowTypeError(ctx, "Invalid arguments");

    if(argc>1){
        if(!JS_IsFunction(ctx,argv[argc-1]))
            return JS_ThrowTypeError(ctx, "Invalid arguments");
        if(argc>2)
            if(JS_ToInt64(ctx,&offset,argv[1]))
                return JS_ThrowTypeError(ctx, "Invalid arguments");
    }

    if(stream_ctx->F>=0)
    {
        size_t len;
        int res=1;
        IOEvent *event_ctx=(IOEvent*)js_malloc(ctx,sizeof(IOEvent));
        event_ctx->request.data=stream_ctx;
        event_ctx->bufferPtr[0].base=(char *)JS_ToCStringLen(ctx,&len,argv[1]);
        event_ctx->bufferPtr[0].len=len;
        if(!event_ctx->bufferPtr[0].base){
            js_free(ctx,event_ctx);
            return JS_ThrowTypeError(ctx, "Invalid arguments");
        }
        res=uv_fs_write(uv_default_loop(), &event_ctx->request, stream_ctx->F, event_ctx->bufferPtr, 1, offset, onWrite);
        if(res){
            js_free(ctx,event_ctx);
            return JS_ThrowInternalError(ctx, uv_strerror(res));
        }else{
            event_ctx->onEvent = argc>1 ? JS_DupValue(ctx, argv[argc-1]) : JS_UNDEFINED;
            JS_DupValue(ctx, this_val);
        }
    }else{
        return JS_ThrowTypeError(ctx, "this is not initilized currectly");
    }
    return JS_UNDEFINED;
}

static void onWrite(uv_fs_t* req)
{
    IOEvent *event_ctx=NULL;
    FileSystem *stream_ctx=NULL;

    event_ctx=(IOEvent *)req;
    stream_ctx=(FileSystem *)req->data;

    if(JS_IsFunction(stream_ctx->ctx,event_ctx->onEvent))
    {
        JSValue args[1];
        JSValue ret;
        args[0]= req->result<0 ? JS_NewString(stream_ctx->ctx,uv_strerror(req->result)) : JS_UNDEFINED;
        ret = JS_Call(stream_ctx->ctx, event_ctx->onEvent, stream_ctx->this_val, 1, args);
        if (JS_IsException(ret)) js_std_dump_error(stream_ctx->ctx);
        JS_FreeValue(stream_ctx->ctx, ret);
        JS_FreeValue(stream_ctx->ctx, args[0]);
    }

    uv_fs_req_cleanup(req);
    JS_FreeCString(stream_ctx->ctx,event_ctx->bufferPtr[0].base);
    JS_FreeValue(stream_ctx->ctx, event_ctx->onEvent);
    js_free(stream_ctx->ctx,event_ctx);

    // WARN: JS_FreeValue may call js_uv_file_finalizer (if no refrence exists)
    // so there is no guaranty that 'stream_ctx' will be available
    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}











struct Directory
{
    uv_dir_s *d;
	JSContext *ctx;
    JSValue this_val;
};



static void onDirOpen(uv_fs_s *req)
{
    JSValue ret;
    JSValue err[1]={JS_UNDEFINED};
    Event *event_ctx=(Event *)req;
    Directory *stream_ctx=(Directory *)req->data;

    if(req->result < 0)
        err[0]=JS_NewString(stream_ctx->ctx,uv_strerror(req->result));
    // 'result' contains file descriptor or negative error number
    stream_ctx->d=(uv_dir_s*)req->ptr;


    {
        ret = JS_Call(stream_ctx->ctx, event_ctx->onEvent, JS_UNDEFINED, 1, err);
        if (unlikely(JS_IsException(ret)))
            js_std_dump_error(stream_ctx->ctx);
        JS_FreeValue(stream_ctx->ctx, ret);
    }

    uv_fs_req_cleanup(req);
    JS_FreeValue(stream_ctx->ctx, err[0]);
    JS_FreeValue(stream_ctx->ctx, event_ctx->onEvent);
    js_free(stream_ctx->ctx,event_ctx);
    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}



static JSValue js_uv_dir_init(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv)
{
    const uint8_t *arg_temp;
    size_t len=0;
    Directory *stream_ctx;
    JSValue proto,thiz;
    Event *event_ctx;
    int res=1;

    if(argc!=2
        || !JS_IsString(argv[0])
        || !JS_IsFunction(ctx,argv[1]))
        goto invalid_arg;
    if((arg_temp = JS_ToCStringLenRaw(ctx, &len, argv[0])) == NULL)
        goto invalid_arg;

    stream_ctx=(Directory*)js_mallocz(ctx, sizeof(Directory));
    if (!stream_ctx)
        return JS_ThrowOutOfMemory(ctx);
    stream_ctx->ctx=ctx;
    stream_ctx->d=nullptr;

    if (JS_IsUndefined(new_target))
        proto = JS_GetClassProto(ctx, js_uv_dir_class_id);
    else
        proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto))
    {
        js_free(ctx,stream_ctx);
        return proto;
    }
    thiz = JS_NewObjectProtoClass(ctx, proto, js_uv_dir_class_id);
    JS_FreeValue(ctx, proto);
    if(JS_IsException(thiz))
    {
        js_free(ctx,stream_ctx);
        return thiz;
    }
    stream_ctx->this_val = thiz;
    JS_SetOpaque(stream_ctx->this_val, stream_ctx);


    {
        event_ctx=(Event*)js_malloc(ctx,sizeof(Event));
        if (!event_ctx)
        {
            JS_FreeValue(ctx, thiz);
            return JS_ThrowOutOfMemory(ctx);
        }
        event_ctx->onEvent=JS_UNDEFINED;

        res=uv_fs_opendir(uv_default_loop(), &event_ctx->request,(const char*)arg_temp, onDirOpen);

        if(res)
        {
            js_free(ctx,event_ctx);
            JS_FreeValue(ctx, thiz);
            return JS_ThrowInternalError(ctx, uv_strerror(res));
        }
        event_ctx->request.data=stream_ctx;
        event_ctx->onEvent = JS_DupValue(ctx, argv[1]);
    }

    JS_DupValue(ctx, stream_ctx->this_val);
    return thiz;

invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}


static void onForceCloseDir(uv_fs_t* req){
    if(req->result<0)
        Console(warn,"onForceCloseDir","finalizer fs close cb failed\n");
    uv_fs_req_cleanup(req);
    js_free_rt((JSRuntime *)(req->data),req);
}

static void js_uv_dir_finalizer (JSRuntime *rt, JSValueConst this_val)
{
    int res;
    Event *event_ctx;
    Directory *stream_ctx;
    stream_ctx = (Directory *)JS_GetOpaque(this_val,js_uv_dir_class_id);
    if(stream_ctx->d != nullptr)
    {
        event_ctx=(Event*)js_malloc_rt(rt,sizeof(Event));
        event_ctx->request.data=rt;
        event_ctx->onEvent=JS_UNDEFINED;
        res=uv_fs_closedir(uv_default_loop(), &event_ctx->request, stream_ctx->d, onForceCloseDir);
        if(unlikely(res)) {
            js_free_rt(rt,event_ctx);
            Console(warn,"js_uv_dir_finalizer","finalizer fs close failed\n");
        }
    }
end:
    js_free_rt(rt, stream_ctx);
}


static JSValue js_uv_dir_read(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    return JS_UNDEFINED;
}





















union sockAddr
{
    struct sockaddr_in addr;
    struct sockaddr_in6 addr6;
};

static JSValue js_uv_sockaddr_init (JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv, int magic)
{
    sockAddr *thiz_ctx=nullptr;
    int32_t port=0,res=0;
    size_t len=0;
    const char *arg_temp=nullptr;
    JSValue thiz;


    if(argc!=2)
        goto invalid_argument;
    if(JS_ToInt32(ctx,&port,argv[1]))
        goto invalid_argument;

    thiz_ctx=(sockAddr*)js_malloc(ctx, sizeof(sockAddr));
    if (!thiz_ctx)
        return JS_ThrowOutOfMemory(ctx);

    if((arg_temp = JS_ToCStringLen(ctx, &len, argv[0])) == NULL)
        goto invalid_argument;
    if(magic)
        res=uv_ip6_addr(arg_temp, port, &thiz_ctx->addr6);
    else
        res=uv_ip4_addr(arg_temp, port, &thiz_ctx->addr);
    JS_FreeCString(ctx, arg_temp);
    if(res)
    {
        js_free(ctx,thiz_ctx);
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    }




    JSValue proto;
    if (JS_IsUndefined(new_target))
        proto = JS_GetClassProto(ctx, js_uv_sockaddr_class_id);
    else
        proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto))
    {
        js_free(ctx,thiz_ctx);
        return proto;
    }
    thiz = JS_NewObjectProtoClass(ctx, proto, js_uv_sockaddr_class_id);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(thiz))
    {
        js_free(ctx,thiz_ctx);
        return thiz;
    }
    JS_SetOpaque(thiz, thiz_ctx);
    return thiz;
invalid_argument:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}
static void js_uv_sockaddr_finalizer (JSRuntime *rt, JSValueConst this_val)
{
    sockAddr *thiz_ctx;
    thiz_ctx = (sockAddr *)JS_GetOpaque(this_val,js_uv_sockaddr_class_id);
    js_free_rt(rt,thiz_ctx);
}



























struct UDPEvent
{
    void *data;
};
struct UDPSendEvent
{
    uv_udp_send_s request;
    JSValue callback;
    JSValue this_val;
    char data[];
};

struct UDP
{
    uv_udp_s s;
	JSContext *ctx;
    JSValue this_val;
    JSValue recvcb;
    //JSValue socket_address;
};

static JSValue js_uv_udp_init(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv) {
    UDP *stream_ctx;
    int32_t res;
    JSValue thiz;

    if(argc!=0)  goto Invalid_arg;

    stream_ctx=(UDP*)js_malloc(ctx, sizeof(UDP));
    if (!stream_ctx)
        return JS_ThrowOutOfMemory(ctx);

    res = uv_udp_init(uv_default_loop(),&stream_ctx->s);
    if(res)
    {
        js_free(ctx,stream_ctx);
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    }


    JSValue proto;
    if (JS_IsUndefined(new_target))
        //thiz=JS_NewObjectClass(ctx, js_uv_udp_class_id);
        proto = JS_GetClassProto(ctx, js_uv_udp_class_id);
    else
        proto = JS_GetPropertyStr(ctx, new_target, "prototype");

    if (JS_IsException(proto)) {
        js_free(ctx,stream_ctx);
        return proto;
    }
    thiz = JS_NewObjectProtoClass(ctx, proto, js_uv_udp_class_id);
    JS_FreeValue(ctx, proto);

    if (JS_IsException(thiz))
    {
        js_free(ctx,stream_ctx);
        return thiz;
    }

    stream_ctx->this_val = thiz;
    stream_ctx->ctx=ctx;
    stream_ctx->s.data=stream_ctx;
    stream_ctx->recvcb=JS_UNDEFINED;
    //stream_ctx->socket_address=JS_UNDEFINED;
    JS_SetOpaque(thiz, stream_ctx);
    return thiz;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}



static void js_uv_udp_gc_mark(JSRuntime *rt,  JSValueConst this_val, JS_MarkFunc *mark_func)
{
    UDP *stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);
    JS_MarkValue(rt, stream_ctx->recvcb, mark_func);
    //JS_MarkValue(rt, stream_ctx->socket_address, mark_func);
}

static void js_uv_udp_finalizer(JSRuntime *rt, JSValueConst this_val)
{
    UDP *stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);
    uv_close((uv_handle_t*) &stream_ctx->s, NULL);
    JS_FreeValueRT(rt,stream_ctx->recvcb);
    //JS_FreeValueRT(rt,stream_ctx->socket_address);
    js_free_rt(rt,stream_ctx);
}




static JSValue js_uv_udp_bind(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
    UDP *stream_ctx=nullptr;
    sockAddr *saddr;
    int32_t res=0;
    if(argc!=1)
        goto Invalid_arg;

    if((saddr = (sockAddr *)JS_GetOpaque(argv[0],js_uv_sockaddr_class_id)) == NULL)
        goto Invalid_arg;

    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);

    res = uv_udp_bind(&stream_ctx->s, (const struct sockaddr*)&saddr->addr, 0);
    // free previuse one
    //JS_FreeValue(ctx,stream_ctx->socket_address);
    if(res){
        //stream_ctx->socket_address=JS_UNDEFINED;
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    }
    //stream_ctx->socket_address=JS_DupValue(ctx,argv[0]);

    return JS_UNDEFINED;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}


static JSValue js_uv_udp_connect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
    UDP *stream_ctx=nullptr;
    sockAddr *saddr;
    int32_t res=0;
    if(argc!=1)
        goto Invalid_arg;

    if((saddr = (sockAddr *)JS_GetOpaque(argv[0],js_uv_sockaddr_class_id)) == NULL)
        goto Invalid_arg;

    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);

    res = uv_udp_connect(&stream_ctx->s, (sockaddr *)&saddr->addr);
    if(res){
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    }
    return JS_UNDEFINED;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}



static void free_udp_buffer(JSRuntime *rt, void *opaque, void *ptr)
{
    (void)rt;
    (void)opaque;
    free(ptr);
}

static void alloc_udp_buffer(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf)
{
    //UDP * const stream_ctx=(UDP *)handle->data;
    //JSValue val=JS_NewStringWLen(stream_ctx->ctx,suggested_size);
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_udp_read(uv_udp_t *handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr *addr, unsigned flags) {
    UDP *const stream_ctx=(UDP *)handle->data;
    uint8_t *str=NULL;
    JSValue ret;
    JSValue args[3];
    if (nread < 1) {
        if(nread==0){
            //it is an empty package
            if(!addr) goto end;
        }else{
            Console(note,"on_udp_read","read error\n");
        }
        //uv_close((uv_handle_t*) req, NULL);
        goto end;
    }
    Console(log,"on_udp_read", "Recv from %s %d bytes\n", sender, nread, buf->base);
    //fwrite(buf->base, nread, 1, stdout);
    if(!addr){
        args[1]=JS_UNDEFINED;
        args[2]=JS_UNDEFINED;
        goto call;
    }
    if(addr->sa_family==AF_INET){
        args[1]=JS_NewStringWLen(stream_ctx->ctx, 17);
        args[2]=JS_NewInt32(stream_ctx->ctx, ((sockaddr_in*)addr)->sin_port);
    }else if(addr->sa_family==AF_INET6){
        args[1]=JS_NewStringWLen(stream_ctx->ctx, 41);
        args[2]=JS_NewInt32(stream_ctx->ctx, ((sockaddr_in6*)addr)->sin6_port);
    }else{
        args[1]=JS_UNDEFINED;
        args[2]=JS_UNDEFINED;
        goto call;
    }
    str=JS_ToCStringLenRaw(stream_ctx->ctx,nullptr,args[1]);
    // it could be a low memory situation.
    if(str){
        if(addr->sa_family==AF_INET){
            uv_ip6_name((struct sockaddr_in6*) addr, (char *)str, 16);
        }else{
            uv_ip6_name((struct sockaddr_in6*) addr, (char *)str, 39);
        }
    }

call:
    args[0]=JS_NewArrayBuffer(stream_ctx->ctx, (uint8_t *)buf->base, nread, &free_udp_buffer, nullptr, false);
    // may uv_udp_recv_stop be called by JS which causes to JS_FreeValue(this_val)
    // be called which may causes to js_free_rt(stream_ctx) be called.
    JS_DupValue(stream_ctx->ctx,stream_ctx->this_val);
    ret = JS_Call(stream_ctx->ctx, stream_ctx->recvcb, stream_ctx->this_val, 3, args);
    if (JS_IsException(ret))
        js_std_dump_error(stream_ctx->ctx);
    JS_FreeValue(stream_ctx->ctx, ret);
    JS_FreeValue(stream_ctx->ctx, args[0]);
    JS_FreeValue(stream_ctx->ctx, args[1]);
    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
    return;
end:
    free(buf->base);
}




static JSValue js_uv_udp_start_recv(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    UDP *stream_ctx;
    int res;
    if(argc!=1 || !JS_IsFunction(ctx,argv[0])) goto Invalid_arg;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);
    res=uv_udp_recv_start(&stream_ctx->s,alloc_udp_buffer,on_udp_read);
    if(res){
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    }
    JS_FreeValue(ctx,stream_ctx->recvcb);
    stream_ctx->recvcb=JS_DupValue(ctx,argv[0]);
    JS_DupValue(ctx,this_val);
    return JS_UNDEFINED;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}

static JSValue js_uv_udp_srop_recv(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    UDP *stream_ctx;
    int res;
    if(argc!=0)  goto Invalid_arg;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);
    res=uv_udp_recv_stop(&stream_ctx->s);
    if(res){
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    }
    JS_FreeValue(ctx,this_val);
    return JS_UNDEFINED;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}




static void on_udp_send(uv_udp_send_s *req, int status) {
    //TODO: 'req' may be null!
    UDPSendEvent *send_req=(UDPSendEvent*)req;
    JSContext *ctx=(JSContext*)send_req->request.data;
    JSValue args[1];

    if(JS_IsFunction(ctx, send_req->callback))
    {
        JSValue args[1];
        JSValue ret;
        args[0]= status <0 ? JS_NewString(ctx,uv_strerror(status)) : JS_NewInt32(ctx,status);
        ret = JS_Call(ctx, send_req->callback, send_req->this_val, 1, args);
        if (JS_IsException(ret)) js_std_dump_error(ctx);
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, args[0]);
    }

    JS_FreeValue(ctx, send_req->callback);
    JS_FreeValue(ctx, send_req->this_val);
    js_free(ctx,send_req);
}



static JSValue js_uv_udp_send(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    UDP *stream_ctx;
    sockAddr *send_addr;
    //sockAddr *send_addr;
    JSValueConst callback;//=JS_UNDEFINED;
    int32_t res;
    uv_buf_t buf;
    UDPSendEvent *send_req;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);

    if(!JS_IsString(argv[0]))
        goto Invalid_arg;

    // set send_addr and callback
    if(argc==1){
        send_addr=NULL;
        callback=JS_UNDEFINED;
    }else if(argc==2){
        if(JS_IsFunction(ctx,argv[1])){
            send_addr=NULL;
            callback=argv[1];
        }else{
            send_addr = (sockAddr *)JS_GetOpaque(argv[1],js_uv_sockaddr_class_id);
            if(send_addr == NULL)
                goto Invalid_arg;
            callback=JS_UNDEFINED;
        }
    }else if(argc==3){
        send_addr = (sockAddr *)JS_GetOpaque(argv[1],js_uv_sockaddr_class_id);
        if(send_addr == NULL)
            goto Invalid_arg;
        if(!JS_IsFunction(ctx,argv[2]))
            goto Invalid_arg;
        callback=argv[2];
    }else{
        goto Invalid_arg;
    }


    buf.base=(char*)JS_ToCStringLenRaw(ctx,&buf.len,argv[0]);
    if(buf.len<1)
        goto Invalid_arg;

    send_req = (UDPSendEvent *)js_malloc(ctx, buf.len + sizeof(UDPSendEvent));
    if(!send_req)
        return JS_ThrowOutOfMemory(ctx);
    memcpy(send_req->data, buf.base, buf.len);
    buf.base = send_req->data;

    res=uv_udp_send(&send_req->request, &stream_ctx->s, &buf, 1, (sockaddr*)&send_addr->addr, on_udp_send);
    if(res)
    {
        js_free(ctx,send_req);
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    }

    send_req->request.data=ctx;
    send_req->callback = JS_DupValue(ctx, callback);
    send_req->this_val = JS_DupValue(ctx, this_val);

    return JS_UNDEFINED;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}



static JSValue js_uv_udp_try_send(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    UDP *stream_ctx;
    struct sockaddr_in *send_addr;
    int32_t res;
    uv_buf_t buf;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);

    if(!JS_IsString(argv[0]))
        goto Invalid_arg;

    if(argc==1){
        send_addr=NULL;
    }else if(argc==2){
        send_addr = (sockaddr_in *)JS_GetOpaque(argv[1],js_uv_sockaddr_class_id);
        if(send_addr == NULL)
            goto Invalid_arg;
    }else{
        goto Invalid_arg;
    }

    buf.base=(char*)JS_ToCStringLenRaw(ctx,&buf.len,argv[0]);
    if(buf.len<0)
        goto Invalid_arg;

    res=uv_udp_try_send(&stream_ctx->s, &buf, 1, (sockaddr*)send_addr);
    if(res<0)
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    return JS_NewInt32(ctx,res);
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}

static JSValue js_uv_udp_set(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic)
{
    UDP *stream_ctx;
    int res;
    if(argc!=1 || JS_ToInt32(ctx, &res, argv[0]))
        goto Invalid_arg;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);

    if(magic==1)
        res=uv_udp_set_ttl(&stream_ctx->s,res);
    else if(magic==2)
        res=uv_udp_set_multicast_ttl(&stream_ctx->s,res);
    else if(magic==4)
        res=uv_udp_set_multicast_loop(&stream_ctx->s,res);
    else if(magic==8)
        res=uv_udp_set_broadcast(&stream_ctx->s,res);
    else
        return JS_ThrowTypeError(ctx, "Invalid magic");
    if(res)
        return JS_ThrowInternalError(ctx, uv_strerror(res));
    return JS_UNDEFINED;
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}

static JSValue js_uv_udp_get(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic)
{
    UDP *stream_ctx;
    union unk1{
        size_t u64;
        int i32;
    } res;
    if(argc!=0)
        goto Invalid_arg;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);

    if(magic==1)
        res.u64=uv_udp_get_send_queue_size(&stream_ctx->s);
    else if(magic==2)
        res.u64=uv_udp_get_send_queue_count(&stream_ctx->s);
    else if(magic==4){
        res.i32=uv_udp_using_recvmmsg(&stream_ctx->s);
        return JS_NewInt32(ctx, res.i32);
    }else
        return JS_ThrowTypeError(ctx, "Invalid magic");
    return JS_NewInt64(ctx, res.u64);
Invalid_arg:
    return JS_ThrowTypeError(ctx, "Invalid arguments");
}






#if IS_LINUX_OS
static inline unsigned int
XStringToToKeycode (Display * display, const char * const string) {
  return XKeysymToKeycode(display, XStringToKeysym(string) );
}

static const char* KeyEvent (const char* const keyname) {
  Display * display = NULL;
  Window window = 0;
  int revert_to_ret = 0;
  XKeyEvent event;
  Status status;

  display = XOpenDisplay(NULL); /* localhost:0.0 */
  if (display == NULL) {
    return "Could not open display: localhost:0.0\n";
  }

  /**
  * Target window is the window currently focused.
  */
  XGetInputFocus(display, &window, &revert_to_ret);
  if (window == 0) {
    return "Could not detect the window which has gotten input focus.\n";
  }

  event.display = display;
  event.window = window;
  event.root = RootWindow(display, DefaultScreen(display));
  event.subwindow = None;
  event.time = CurrentTime;
  event.x = event.y = 1;
  event.x_root = event.y_root = 1;
  event.same_screen = 1;
  event.keycode = XStringToToKeycode(display, keyname);
  /* event.state = modifiers; */

  event.type = KeyPress;
  status = XSendEvent(display, window, 1, KeyPressMask, (XEvent *)&event);
  if (status == 0) {
    return "Failed to send evnet: type=KeyPress\n"; /* error */
  }

  event.type = KeyRelease;
  status = XSendEvent(display, window, 1, KeyReleaseMask, (XEvent *)&event);

  XSync(display, 1);
  XCloseDisplay(display);

  return status==0?"XSendEvent failed",0;
}
#endif



static JSValue js_uv_simulate_key(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#if IS_WINDOWS_OS
    int c;
    if(argc!=1 || !JS_IsNumber(argv[0]))
        return JS_ThrowTypeError(ctx, "Invalid arguments");

    if(JS_ToInt32(ctx,&c,argv[0]))
        return JS_ThrowTypeError(ctx, "Invalid arguments");

//    INPUT keystroke[ 2 ];
//    keystroke[0].type = INPUT_KEYBOARD;
//    keystroke[0].ki.wVk = 0;
//    keystroke[0].ki.wScan = (WORD)c;
//    keystroke[0].ki.dwFlags = KEYEVENTF_UNICODE;
//    keystroke[0].ki.time = 0;
//    keystroke[0].ki.dwExtraInfo = GetMessageExtraInfo();
//
//    keystroke[1].type = INPUT_KEYBOARD;
//    keystroke[1].ki.wVk = 0;
//    keystroke[1].ki.wScan = (WORD)c;
//    keystroke[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
//    keystroke[1].ki.time = 0;
//    keystroke[1].ki.dwExtraInfo = GetMessageExtraInfo();
//    //Send the keystrokes.
//    return JS_NewBool(ctx,SendInput(2, keystroke, sizeof(*keystroke)) == 2);

    //HWND w=GetForegroundWindow();
    keybd_event(c,0,KEYEVENTF_EXTENDEDKEY,0);
    keybd_event(c,0,KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,0);
    //SendMessage(w,WM_KEYDOWN,VK_RIGHT,21823489);
    //SendMessage(w,WM_KEYUP,VK_RIGHT,3243048961);
#endif
#if IS_LINUX_OS
    const char *res=KeyEvent("a");
    if(res)
        return JS_ThrowInternalError(ctx, res);
#endif
    return JS_UNDEFINED;
}
